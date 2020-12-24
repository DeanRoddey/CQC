//
// FILE NAME: CQCTrayMon_iTunesTab3.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/09/2012
//
// COPYRIGHT: Charmed Quark Systems, Ltd @ 2020
//
//  This software is copyrighted by 'Charmed Quark Systems, Ltd' and 
//  the author (Dean Roddey.) It is licensed under the MIT Open Source 
//  license:
//
//  https://opensource.org/licenses/MIT
//
// DESCRIPTION:
//
//  This file handles overflow from the main file, sine it's getting a little
//  large. This one handles the stuff that handles the player control parts
//  of the driver. Incoming calls to perform commands are queued up, and
//  the bgn thread that does the iTunes interfacing processes them.
//
//  For returned status, we maintain that information by way of notifications
//  from iTunes or by the bgn thread polling stuff and leaving it around,
//  so all we have to do is to just lock and copy that info to the caller's
//  output parameters.
//
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


// ---------------------------------------------------------------------------
//  Facility specific includes
// ---------------------------------------------------------------------------
#include    "CQCTrayMon.hpp"


// ---------------------------------------------------------------------------
//  Bring in the iTunes SDK header and a platform specific helper header
// ---------------------------------------------------------------------------
#include    <windows.h>
#include    <iTunesCOMInterface.h>
#include    "CIDKernel_InternalHelpers_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TPlayerCmd, TObject);




// ---------------------------------------------------------------------------
//  CLASS: TPlayerCmd
// PREFIX: plcmd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TPlayerCmd: Constructors and destructor
// ---------------------------------------------------------------------------
TPlayerCmd::TPlayerCmd() :

    m_bValue(kCIDLib::False)
    , m_c4Value(0)
    , m_eCmd(TiTPlayerIntfServerBase::EPlCmds::None)
    , m_eType(tCQCTrayMon::EPlCmdTypes::None)
{
}

TPlayerCmd::TPlayerCmd(const TiTPlayerIntfServerBase::EPlCmds eCmd) :

    m_bValue(kCIDLib::False)
    , m_c4Value(0)
    , m_eCmd(eCmd)
    , m_eType(tCQCTrayMon::EPlCmdTypes::PlCmd)
{
}

TPlayerCmd::TPlayerCmd( const   tCQCTrayMon::EPlCmdTypes    eType
                        , const tCIDLib::TCard4             c4Value) :

    m_bValue(kCIDLib::False)
    , m_c4Value(c4Value)
    , m_eCmd(TiTPlayerIntfServerBase::EPlCmds::None)
    , m_eType(eType)
{
}

TPlayerCmd::TPlayerCmd( const   tCQCTrayMon::EPlCmdTypes    eType
                        , const tCIDLib::TBoolean           bValue) :

    m_bValue(bValue)
    , m_c4Value(0)
    , m_eCmd(TiTPlayerIntfServerBase::EPlCmds::None)
    , m_eType(tCQCTrayMon::EPlCmdTypes::SetVolume)
{
}

TPlayerCmd::TPlayerCmd( const   tCQCTrayMon::EPlCmdTypes    eType
                        , const TString&                    strValue) :

    m_bValue(kCIDLib::False)
    , m_c4Value(0)
    , m_eCmd(TiTPlayerIntfServerBase::EPlCmds::None)
    , m_eType(eType)
    , m_strValue(strValue)
{
}

TPlayerCmd::TPlayerCmd(const TPlayerCmd& plcmdSrc) :

    m_bValue(plcmdSrc.m_bValue)
    , m_c4Value(plcmdSrc.m_c4Value)
    , m_eCmd(plcmdSrc.m_eCmd)
    , m_eType(plcmdSrc.m_eType)
    , m_strValue(plcmdSrc.m_strValue)
{
}

TPlayerCmd::~TPlayerCmd()
{
}


// ---------------------------------------------------------------------------
//  TPlayerCmd: Public operators
// ---------------------------------------------------------------------------
TPlayerCmd& TPlayerCmd::operator=(const TPlayerCmd& plcmdSrc)
{
    if (this != &plcmdSrc)
    {
        m_bValue    = plcmdSrc.m_bValue;
        m_c4Value   = plcmdSrc.m_c4Value;
        m_eCmd      = plcmdSrc.m_eCmd;
        m_eType     = plcmdSrc.m_eType;
        m_strValue  = plcmdSrc.m_strValue;
    }
    return *this;
}







// ---------------------------------------------------------------------------
//  TCQCTrayiTunesTab: Public, inherited methods
//
//  These are the player oriented public commands that handle player driver
//  requests.
// ---------------------------------------------------------------------------

//
//  This is connected to a poll method from the player IDL interface, so
//  the return is whether we have new data or not. If not, then nothing
//  is streamed back.
//
tCIDLib::TBoolean
TCQCTrayiTunesTab::bGetPlayerState( tCIDLib::TCard4&                    c4SerialNum
                                    , tCIDLib::TBoolean&                bConnected
                                    , tCIDLib::TCard4&                  c4Volume
                                    , TiTPlayerIntfServerBase::EPlStates& ePlState
                                    , tCIDLib::TBoolean&                bMute
                                    , tCIDLib::TEncodedTime&            enctTotal
                                    , tCIDLib::TEncodedTime&            enctCur
                                    , TString&                          strArtist
                                    , TString&                          strAlbum
                                    , TString&                          strTrack)
{
    TLocker lockrSync(&m_mtxSync);

    // If nothing new, return false
    if (m_c4PlSerialNum == c4SerialNum)
        return kCIDLib::False;

    // Give back the new serial number
    c4SerialNum = m_c4PlSerialNum;

    // Get the current status
    bConnected = (m_eStatus == tCQCMedia::ELoadStatus::Ready);

    // If not connected, return bogus info for most of it
    if (!bConnected)
    {
        bMute = kCIDLib::False;
        enctCur = 0;
        enctTotal = 0;
        c4Volume = 0;
        strArtist.Clear();
        strAlbum.Clear();
        strTrack.Clear();
        return kCIDLib::False;
    }

    // We are ready, so return info
    bMute = m_bCurMute;
    c4Volume = m_c4CurVolume;
    enctCur = m_enctCurPos;
    enctTotal = m_enctCurTotal;
    ePlState = m_ePlState;

    strAlbum = m_strCurAlbum;
    strArtist = m_strCurArtist;
    strTrack = m_strCurTrack;

    return kCIDLib::True;
}



//
//  The player driver passes through this request from clients, to select
//  a playlist by the repo title set cookie.
//
tCIDLib::TVoid TCQCTrayiTunesTab::SelPLByCookie(const TString& strCookie)
{
    // Queue it up for processing by the bgn thread
    TLocker lockrSync(&m_mtxSync);
    if (m_eStatus == tCQCMedia::ELoadStatus::Ready)
        m_colPlCmdQ.objAdd(TPlayerCmd(tCQCTrayMon::EPlCmdTypes::SelPL, strCookie));
}


//
//  The player driver passes through this request from clients, to select
//  a song by the repo item cookie.
//
tCIDLib::TVoid TCQCTrayiTunesTab::SelTrackByCookie(const TString& strCookie)
{

    // Queue it up for processing by the bgn thread
    TLocker lockrSync(&m_mtxSync);
    if (m_eStatus == tCQCMedia::ELoadStatus::Ready)
        m_colPlCmdQ.objAdd(TPlayerCmd(tCQCTrayMon::EPlCmdTypes::SelTrack, strCookie));
}


//
//  We just queue up a command to be processed by the bgn thread. If we
//  are not up and connected, this is ignored.
//
tCIDLib::TVoid
TCQCTrayiTunesTab::DoPlayerCmd(const TiTPlayerIntfServerBase::EPlCmds eCmd)
{
    // Queue up the command for processing by the bgn thread
    TLocker lockrSync(&m_mtxSync);
    if (m_eStatus == tCQCMedia::ELoadStatus::Ready)
        m_colPlCmdQ.objAdd(TPlayerCmd(eCmd));
}


//
//  We queue up the mute command for later processing by the bgn thrad. If
//  we are not up and connected, this is ignored.
//
tCIDLib::TVoid TCQCTrayiTunesTab::SetMute(const tCIDLib::TBoolean bToSet)
{
    // Queue up the command for processing by the bgn thread
    TLocker lockrSync(&m_mtxSync);

    if (m_eStatus == tCQCMedia::ELoadStatus::Ready)
    {
        m_colPlCmdQ.objAdd
        (
            TPlayerCmd(tCQCTrayMon::EPlCmdTypes::SetMute, bToSet)
        );
    }
}


//
//  We queue up the volume command for later processing by the bgn thrad. If
//  we are not up and connected, this is ignored.
//
tCIDLib::TVoid TCQCTrayiTunesTab::SetVolume(const tCIDLib::TCard4 c4ToSet)
{
    // Queue up the command for processing by the bgn thread
    TLocker lockrSync(&m_mtxSync);

    if (m_eStatus == tCQCMedia::ELoadStatus::Ready)
    {
        m_colPlCmdQ.objAdd
        (
            TPlayerCmd(tCQCTrayMon::EPlCmdTypes::SetVolume, c4ToSet)
        );
    }
}


// ---------------------------------------------------------------------------
//  TCQCTrayiTunesTab: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  The bgn thread calls us here to process player commands as he pulls them
//  off the queue. On incoming driver calls to DoPlayerCmd() above, we queue
//  them up. They come back to us here (in the right thread context) to handle.
//  The bgn thread will have locked before calling this.
//
tCIDLib::TVoid
TCQCTrayiTunesTab::ProcessPlCmd(const TPlayerCmd& plcmdToDo)
{
    // If we aren't connected, ignore it
    if (m_eStatus != tCQCMedia::ELoadStatus::Ready)
        return;

    // And just in case, if the player isn't set
    if (!m_piTunes)
        return;

    if (plcmdToDo.m_eType == tCQCTrayMon::EPlCmdTypes::PlCmd)
    {
        // It's a generic player command
        switch(plcmdToDo.m_eCmd)
        {
            case TiTPlayerIntfServerBase::EPlCmds::FF :
                m_piTunes->FastForward();
                break;

            case TiTPlayerIntfServerBase::EPlCmds::FullScrOff :
                m_piTunes->put_FullScreenVisuals(VARIANT_FALSE);
                break;

            case TiTPlayerIntfServerBase::EPlCmds::FullScrOn :
                m_piTunes->put_FullScreenVisuals(VARIANT_TRUE);
                break;

            case TiTPlayerIntfServerBase::EPlCmds::Next :
                m_piTunes->NextTrack();
                break;

            case TiTPlayerIntfServerBase::EPlCmds::Pause :
                m_piTunes->Pause();
                break;

            case TiTPlayerIntfServerBase::EPlCmds::Play :
            {
                //
                //  See if we are in rewind or FF mode. If so, then we want
                //  to do a resume, instead of a play.
                //
                ITPlayerState PlState;
                HRESULT hRes = m_piTunes->get_PlayerState(&PlState);
                if (FAILED(hRes)
                ||  (PlState == ITPlayerStateStopped)
                ||  (PlState == ITPlayerStatePlaying))
                {
                    m_piTunes->Play();
                }
                 else
                {
                    m_piTunes->Resume();
                }
                break;
            }

            case TiTPlayerIntfServerBase::EPlCmds::Previous :
                m_piTunes->PreviousTrack();
                break;

            case TiTPlayerIntfServerBase::EPlCmds::Rewind :
                m_piTunes->Rewind();
                break;

            case TiTPlayerIntfServerBase::EPlCmds::Stop :
                m_piTunes->Stop();
                break;

            case TiTPlayerIntfServerBase::EPlCmds::VisualsOff :
                m_piTunes->put_VisualsEnabled(VARIANT_FALSE);
                break;

            case TiTPlayerIntfServerBase::EPlCmds::VisualsOn :
                m_piTunes->put_VisualsEnabled(VARIANT_TRUE);
                break;

            default :
                QLogMsg
                (
                    L"Got unknown player command=%(1)"
                    , TCardinal(tCIDLib::c4EnumOrd(plcmdToDo.m_eCmd))
                );
                break;
        };
    }
     else if (plcmdToDo.m_eType == tCQCTrayMon::EPlCmdTypes::SelPL)
    {
        //
        //  See if can find a playlist represented by the passed title set
        //  cookie. If so, we start its first track playing.
        //
        tCIDLib::TCard2 c2CatId;
        tCIDLib::TCard2 c2TitleId;
        tCQCMedia::EMediaTypes eMType;

        try
        {
            eMType = facCQCMedia().eParseTitleCookie(plcmdToDo.m_strValue, c2CatId, c2TitleId);

            // If not a music cookie, ignore it
            if (eMType != tCQCMedia::EMediaTypes::Music)
            {
                QLogMsg(L"The received cookie was not an item cookie");
                return;
            }

            //
            //  Look up the title set that has the indicated id.
            //
            TMediaDB& mdbInfo = m_srdbEng.mdbInfo();
            const TMediaTitleSet* pmtsSrc = mdbInfo.pmtsById(eMType, c2TitleId, kCIDLib::True);

            //
            //  OK, we got it, so get the id out of it. That's the set of iTunes
            //  ids that represent this track. We parse that and that gives us
            //  the ids we need to select it. The last two will be zero in this
            //  case.
            //
            tCIDLib::TInt4 i4Src;
            tCIDLib::TInt4 i4PL;
            tCIDLib::TInt4 i4Track;
            tCIDLib::TInt4 i4DB;
            tCIDLib::TBoolean bRes = bParseObjUID
            (
                pmtsSrc->strUniqueId(), i4Src, i4PL, i4Track, i4DB
            );

            // If we couldn't parse it, that's bad, give up
            if (!bRes)
            {
                QLogMsg
                (
                    L"Couldn't parse title set's id. Value=%(1)", pmtsSrc->strUniqueId()
                );
                return;
            }

            // OK, let's try to find a playlist with this id
            IITObject* pObject = 0;
            HRESULT hRes = m_piTunes->GetITObjectByID(i4Src, i4PL, i4Track, i4DB, &pObject);
            if (FAILED(hRes))
            {
                QLogMsg(L"Couldn't find playlist in iTunes database");
                return;
            }
            TCOMJanitor<IITObject> janObject(&pObject);

            // Make sure it's a user playlist
            IITUserPlaylist* pPList = 0;
            hRes = pObject->QueryInterface(__uuidof(pPList),(void **)&pPList);
            if (FAILED(hRes))
            {
                QLogMsg(L"Selected object was not a user playlist");
                return;
            }
            TCOMJanitor<IITUserPlaylist> janList(&pPList);

            // We found it, so start it playing the first track of the list
            pPList->PlayFirstTrack();
        }

        catch(TError& errToCatch)
        {
            if (facCQCTrayMon.eVerboseLvl() >= tCQCKit::EVerboseLvls::Medium)
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }
            QLogMsg(L"Failed to select playlist by title cookie");
        }
    }
     else if (plcmdToDo.m_eType == tCQCTrayMon::EPlCmdTypes::SelTrack)
    {
        //
        //  See if we can find the indicated track by its item cookie. Parse
        //  the item cookie. If it's bad, we just do nothing.
        //
        tCIDLib::TCard2 c2CatId;
        tCIDLib::TCard2 c2TitleId;
        tCIDLib::TCard2 c2ColId;
        tCIDLib::TCard2 c2ItemId;
        tCQCMedia::EMediaTypes eMType;

        try
        {
            eMType = facCQCMedia().eParseItemCookie
            (
                plcmdToDo.m_strValue, c2CatId, c2TitleId, c2ColId, c2ItemId
            );

            // If not a music cookie, ignore it
            if (eMType != tCQCMedia::EMediaTypes::Music)
            {
                QLogMsg(L"The received cookie was not an item cookie");
                return;
            }

            //
            //  So we now need to go through the hierarchy and find the item.
            //  We don't really have to look up the title set, but we do it
            //  just as a safety check.
            //
            TMediaDB& mdbInfo = m_srdbEng.mdbInfo();
            const TMediaTitleSet* pmtsSrc = mdbInfo.pmtsById(eMType, c2TitleId, kCIDLib::True);
            const TMediaCollect& mcolSrc = pmtsSrc->mcolAt(mdbInfo, c2ColId - 1);
            const TMediaItem& mitemSrc = mcolSrc.mitemAt(mdbInfo, c2ItemId - 1);

            //
            //  OK, we got it, so get the alt id out of it. That's the set
            //  of iTunes ids that represent this track. We parse that and
            //  that gives us the ids we need to select it.
            //
            tCIDLib::TInt4 i4Src;
            tCIDLib::TInt4 i4PL;
            tCIDLib::TInt4 i4Track;
            tCIDLib::TInt4 i4DB;
            tCIDLib::TBoolean bRes = bParseObjUID
            (
                mitemSrc.strAltId(), i4Src, i4PL, i4Track, i4DB
            );

            // If we couldn't parse it, that's bad, give up
            if (!bRes)
            {
                QLogMsg
                (
                    L"Couldn't parse item's alt id. Value=%(1)", mitemSrc.strAltId()
                );
                return;
            }

            // OK, let's try to select this track
            IITObject* pObject = 0;
            HRESULT hRes = m_piTunes->GetITObjectByID(i4Src, i4PL, i4Track, i4DB, &pObject);
            if (FAILED(hRes))
            {
                QLogMsg(L"Couldn't find item in iTunes database");
                return;
            }
            TCOMJanitor<IITObject> janObject(&pObject);

            // Make sure it's a track
            IITTrack* pTrack = 0;
            hRes = pObject->QueryInterface(__uuidof(pTrack),(void **)&pTrack);
            if (FAILED(hRes))
            {
                QLogMsg(L"Selected object was not a track");
                return;
            }
            TCOMJanitor<IITTrack> janTrack(&pTrack);

            // And finally ask it to play
            pTrack->Play();
        }

        catch(TError& errToCatch)
        {
            if (facCQCTrayMon.eVerboseLvl() >= tCQCKit::EVerboseLvls::Medium)
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }
            QLogMsg(L"Failed to select track by cookie");
        }
    }
     else if (plcmdToDo.m_eType == tCQCTrayMon::EPlCmdTypes::SetMute)
    {
        if (plcmdToDo.m_bValue)
            m_piTunes->put_Mute(VARIANT_TRUE);
        else
            m_piTunes->put_Mute(VARIANT_FALSE);
    }
     else if (plcmdToDo.m_eType == tCQCTrayMon::EPlCmdTypes::SetVolume)
    {
        m_piTunes->put_SoundVolume((long)plcmdToDo.m_c4Value);
    }
     else
    {
        // We got an unknown type
        QLogMsg
        (
            L"Got unknown player command type=%(1)"
            , TCardinal(tCIDLib::c4EnumOrd(plcmdToDo.m_eType))
        );
    }
}



