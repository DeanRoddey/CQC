//
// FILE NAME: MediaChMgrS_DriverImpl.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/02/2006
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
//  This is a test driver which implements driver interface for the variable
//  driver..
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "MediaChMgrS.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TMediaChMgrSDriver,TCQCServerBase)


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace MediaChMgrS_DriverImpl
{
}



// ---------------------------------------------------------------------------
//   CLASS: TMediaChMgrSDriver
//  PREFIX: drv
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TMediaChMgrSDriver: Constructors and Destructor
// ---------------------------------------------------------------------------
TMediaChMgrSDriver::TMediaChMgrSDriver(const TCQCDriverObjCfg& cqcdcToLoad) :

    TCQCServerBase(cqcdcToLoad)
    , m_c4FldId_CurAspectRatio(kCIDLib::c4MaxCard)
    , m_c4FldId_CurCast(kCIDLib::c4MaxCard)
    , m_c4FldId_CurColArtist(kCIDLib::c4MaxCard)
    , m_c4FldId_CurColCookie(kCIDLib::c4MaxCard)
    , m_c4FldId_CurColName(kCIDLib::c4MaxCard)
    , m_c4FldId_CurDescr(kCIDLib::c4MaxCard)
    , m_c4FldId_CurItemArtist(kCIDLib::c4MaxCard)
    , m_c4FldId_CurItemCookie(kCIDLib::c4MaxCard)
    , m_c4FldId_CurItemName(kCIDLib::c4MaxCard)
    , m_c4FldId_CurLabel(kCIDLib::c4MaxCard)
    , m_c4FldId_CurLeadActor(kCIDLib::c4MaxCard)
    , m_c4FldId_CurRating(kCIDLib::c4MaxCard)
    , m_c4FldId_CurTitleCookie(kCIDLib::c4MaxCard)
    , m_c4FldId_CurTitleName(kCIDLib::c4MaxCard)
    , m_c4FldId_CurYear(kCIDLib::c4MaxCard)
    , m_c4FldId_PlayMedia(kCIDLib::c4MaxCard)
    , m_c4FldId_Transport(kCIDLib::c4MaxCard)
    , m_c4LastTrackNum(0)
    , m_mbufPoll(4096UL)
{
}

TMediaChMgrSDriver::~TMediaChMgrSDriver()
{
}


// ---------------------------------------------------------------------------
//  TMediaChMgrSDriver: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  We let clients get the image data for the title we are currently playing.
//  They could get it themselves by asking for our art path and the repository
//  we are associate with and going directly to the repository, but it's
//  more convenient and more flexible to just let them ask us.
//
tCIDLib::TBoolean
TMediaChMgrSDriver::bQueryData( const   TString&            strQType
                                , const TString&            strDataName
                                ,       tCIDLib::TCard4&    c4OutBytes
                                ,       THeapBuf&           mbufToFill)
{
    // Handle the query based on the query type
    tCIDLib::TBoolean bRet = kCIDLib::False;

    if ((strQType == kCQCMedia::strQuery_QueryArt)
    ||  (strQType == kCQCMedia::strQuery_QueryItemArt)
    ||  (strQType == kCQCMedia::strQuery_QueryTitleArt)
    ||  (strQType == kCQCMedia::strQuery_QueryThumbArt)
    ||  (strQType == kCQCMedia::strQuery_QueryItemThumbArt)
    ||  (strQType == kCQCMedia::strQuery_QueryTitleThumbArt))
    {
        // Get a proxy for our repository driver's CQCServer and pass it on
        tCQCKit::TCQCSrvProxy orbcAdmin(facCQCKit().orbcCQCSrvAdminProxy(m_strRepoMoniker));
        bRet = orbcAdmin->bQueryData
        (
            m_strRepoMoniker, strQType, strDataName, c4OutBytes, mbufToFill
        );
    }
     else
    {
        // Don't know what it is, so throw
        facCQCMedia().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMedErrs::errcDrv_UnknownDataQuery
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::BadParms
            , strQType
            , strMoniker()
        );
    }
    return bRet;
}


// ---------------------------------------------------------------------------
//  TMediaChMgrSDriver: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TMediaChMgrSDriver::bGetCommResource(TThread&)
{
    // We have no comm resource, so just say we are good
    return kCIDLib::True;
}


tCIDLib::TBoolean TMediaChMgrSDriver::bWaitConfig(TThread&)
{
    // We don't have any, just say it worked
    return kCIDLib::True;
}


tCQCKit::ECommResults
TMediaChMgrSDriver::eBoolFldValChanged( const   TString&            strMoniker
                                        , const tCIDLib::TCard4     c4FldId
                                        , const tCIDLib::TBoolean   bValWritten)
{
    // If no active changer, reject
    if (m_strChanger.bIsEmpty())
        return tCQCKit::ECommResults::NotReady;

    if (c4FldId == m_aFldIdMap[EMapFld_Power].c4OurId)
    {
        // Just pass it on to the active changer
        m_orbcDrv->WriteBoolByName
        (
            m_strChanger, L"Power", bValWritten, tCQCKit::EDrvCmdWaits::DontCare
        );
    }
     else
    {
        IncUnknownWriteCounter();
        return tCQCKit::ECommResults::FieldNotFound;
    }
    return tCQCKit::ECommResults::Success;
}


tCQCKit::ECommResults
TMediaChMgrSDriver::eCardFldValChanged( const   TString&        strMoniker
                                        , const tCIDLib::TCard4 c4FldId
                                        , const tCIDLib::TCard4 c4ValWritten)
{
    // If no active changer, reject
    if (m_strChanger.bIsEmpty())
        return tCQCKit::ECommResults::NotReady;

    //
    //  If we get writes to our current track or disc field, we turn around
    //  and send it to the active changer.
    //
    tCQCKit::ECommResults eRet = tCQCKit::ECommResults::Success;
    if (c4FldId == m_aFldIdMap[EMapFld_CurrentDisc].c4OurId)
    {
        m_orbcDrv->WriteCardByName
        (
            m_strChanger, L"CurrentDisc", c4ValWritten, tCQCKit::EDrvCmdWaits::DontCare
        );
    }
     else if (c4FldId == m_aFldIdMap[EMapFld_CurrentTrack].c4OurId)
    {
        m_orbcDrv->WriteCardByName
        (
            m_strChanger, L"CurrentTrack", c4ValWritten, tCQCKit::EDrvCmdWaits::DontCare
        );
    }
     else
    {
        eRet = tCQCKit::ECommResults::FieldNotFound;
        IncUnknownWriteCounter();
    }
    return eRet;
}


tCQCKit::ECommResults TMediaChMgrSDriver::eConnectToDevice(TThread&)
{
    //
    //  We are always 'connected', because we don't have a real device.
    //  So we just return success.
    //
    return tCQCKit::ECommResults::Success;
}


//
//  We know what our fields are up front, so we can create them now. We are
//  a renderer driver, so we have to provide the standard renderer driver
//  fields, plus some of our own.
//
//  We set a reasonable poll time, which we use to get status info from the
//  active changer driver. We don't really care about the connect time since
//  we never disconnect.
//
tCQCKit::EDrvInitRes TMediaChMgrSDriver::eInitializeImpl()
{
    //
    //  We should have gotten a prompt value with the repository driver
    //  we are to work with.
    //
    const TCQCDriverObjCfg& cqcdcTmp = cqcdcThis();
    if (!cqcdcTmp.bFindPromptValue(L"RepoDriver", L"Moniker", m_strRepoMoniker))
    {
        facMediaChMgrS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"The repository driver prompt was not set"
            , tCIDLib::ESeverities::Status
        );
        return tCQCKit::EDrvInitRes::Failed;
    }

    RegisterFields();
    SetPollTimes(1000, 10000);
    m_c4LastTrackNum = 0;
    return tCQCKit::EDrvInitRes::WaitCommRes;
}


tCQCKit::ECommResults TMediaChMgrSDriver::ePollDevice(TThread&)
{
    //
    //  Just poll the changer. If we fail, it will just set our fields that
    //  are relfecting active changer state into error state. If it returns
    //  true, then the track value changed, and we need to update our
    //  item level metadata fields.
    //
    if (bPollChanger())
    {
        if (m_c4LastTrackNum
        &&  (m_c4LastTrackNum <= m_colItemNames.c4ElemCount()))
        {
            // The track numbers are 1 based, so dec for an index
            const tCIDLib::TCard4 c4Ind = m_c4LastTrackNum - 1;
            bStoreStringFld(m_c4FldId_CurItemArtist, m_colItemArtists[c4Ind], kCIDLib::True);
            bStoreStringFld(m_c4FldId_CurItemCookie, m_colItemCookies[c4Ind], kCIDLib::True);
            bStoreStringFld(m_c4FldId_CurItemName, m_colItemNames[c4Ind], kCIDLib::True);
        }
         else
        {
            // There's no active track, so clear out the per-item fields
            bStoreStringFld(m_c4FldId_CurItemArtist, TString::strEmpty(), kCIDLib::True);
            bStoreStringFld(m_c4FldId_CurItemCookie, TString::strEmpty(), kCIDLib::True);
            bStoreStringFld(m_c4FldId_CurItemName, TString::strEmpty(), kCIDLib::True);
        }
    }
    return tCQCKit::ECommResults::Success;
}


tCQCKit::ECommResults
TMediaChMgrSDriver::eStringFldValChanged(const  TString&        strMoniker
                                        , const tCIDLib::TCard4 c4FldId
                                        , const TString&        strValWritten)
{
    if (c4FldId == m_c4FldId_PlayMedia)
    {
        // Parse this cookie using the version that figures out the type for us
        tCIDLib::TCard2        c2CatId;
        tCIDLib::TCard2        c2ColId;
        tCIDLib::TCard2        c2ItemId;
        tCIDLib::TCard2        c2TitleId;
        tCQCMedia::EMediaTypes eType;
        const tCQCMedia::ECookieTypes eCookieType = facCQCMedia().eCheckCookie
        (
            strValWritten, eType, c2CatId, c2TitleId, c2ColId, c2ItemId
        );

        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
        {
            facMediaChMgrS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , L"Got a request to play media. Cookie=%(1)"
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
                , strValWritten
            );
        }

        if (eCookieType == tCQCMedia::ECookieTypes::Item)
        {
            //
            //  See if any item in our current collection matches the
            //  passed item cookie.
            //
            const tCIDLib::TCard4 c4Count = m_colItemCookies.c4ElemCount();
            tCIDLib::TCard4 c4Index = 0;
            for (; c4Index < c4Count; c4Index++)
            {
                if (m_colItemCookies[c4Index] == strValWritten)
                    break;
            }

            //
            //  If not found, or found but cant' be a legal track, then
            //  reject. Check for >= because we are 0 based and the number
            //  we'll send is 1 based.
            //
            if ((c4Index == c4Count) || (c4Index >= 99))
            {
                if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Low)
                {
                    facMediaChMgrS().LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , L"%(1) is not a valid track index"
                        , tCIDLib::ESeverities::Status
                        , tCIDLib::EErrClasses::AppStatus
                        , TCardinal(c4Index)
                    );
                }
                IncFailedWriteCounter();
                return tCQCKit::ECommResults::FieldNotFound;
            }

            //
            //  The changers are always 1 based, so we send the index plus
            //  one.
            //
            m_orbcDrv->WriteCardByName
            (
                m_strChanger
                , L"CurrentTrack"
                , c4Index + 1
                , tCQCKit::EDrvCmdWaits::DontCare
            );
        }
         else
        {
            // Get a proxy for our repository driver's CQCServer
            tCQCKit::TCQCSrvProxy orbcRepo
            (
                facCQCKit().orbcCQCSrvAdminProxy(m_strRepoMoniker)
            );

            //
            //  If it's a title cookie, then convert it to a collection cookie
            //  for the first collection, else it's already a collection
            //  cookie, so create the associated title cookie.
            //
            TString strColCookie;
            TString strTitleCookie;
            if (eCookieType == tCQCMedia::ECookieTypes::Title)
            {
                strTitleCookie = strValWritten;
                strColCookie = strTitleCookie;
                strColCookie.Append(L",1");
            }
             else
            {
                strColCookie = strValWritten;
                facCQCMedia().FormatTitleCookie
                (
                    eType, c2CatId, c2TitleId, strTitleCookie
                );
            }

            if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
            {
                facMediaChMgrS().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , L"Querying location data"
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::AppStatus
                );
            }

            tCIDLib::TCard4 c4Slot;
            TString         strNewChanger;
            if (!bGetLocData(orbcRepo, strColCookie, strNewChanger, c4Slot))
            {
                if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Low)
                {
                    facMediaChMgrS().LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , L"Could not get changer location info from collection cookie '%(1)'"
                        , tCIDLib::ESeverities::Status
                        , tCIDLib::EErrClasses::AppStatus
                        , strColCookie
                    );
                }
                IncFailedWriteCounter();
                return tCQCKit::ECommResults::ValueRejected;
            }

            //
            //  If we have a current changer, and it's not the same as the new
            //  one, and we curerntly have a good client proxy, then tell the old
            //  changer to stop, just in case it's playing. We ignore any failure
            //  and just keep going to the new changer.
            //
            if ((m_strChanger != strNewChanger) && m_orbcDrv.pobjData())
            {
                if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
                {
                    facMediaChMgrS().LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , L"Stopping previously selected changer (%(1))"
                        , tCIDLib::ESeverities::Status
                        , tCIDLib::EErrClasses::AppStatus
                        , m_strChanger
                    );
                }
                bDoTransportOp(L"Stop");
            }

            if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
            {
                facMediaChMgrS().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , L"Querying driver info for new changer (%(1))"
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::AppStatus
                    , strNewChanger
                );
            }

            //
            //  We don't attempt to optimize here and see if the current changer
            //  is the same as the new one. We just get new info, which lets us
            //  adapt immediately to changes in driver configurations.
            //
            TCQCDriverObjCfg cqcdcInfo;
            tCQCKit::TCQCSrvProxy orbcNewSrv = facCQCKit().orbcCQCSrvAdminProxy(strNewChanger);
            if (!orbcNewSrv->bQueryDriverInfo(strNewChanger, cqcdcInfo))
            {
                if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Low)
                {
                    facMediaChMgrS().LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , L"Could not access changer %(1)"
                        , tCIDLib::ESeverities::Status
                        , tCIDLib::EErrClasses::AppStatus
                        , strNewChanger
                    );
                }
                IncFailedWriteCounter();
                return tCQCKit::ECommResults::ValueRejected;
            }

            //
            //  Now set up this new guy as our guy. After this, the client
            //  proxy is set in our m_orbcDrv member, so any methods that
            //  work on the currently active changer can be called.
            //
            m_orbcDrv = orbcNewSrv;
            bStoreStringFld(m_c4FldId_ActiveChanger, strNewChanger, kCIDLib::True);

            // Store the moniker in a member for faster access
            m_strChanger = strNewChanger;

            // If anything fails from here on out, we go back to ground zero
            try
            {
                if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
                {
                    facMediaChMgrS().LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , L"Loading media data from repository %(1)"
                        , tCIDLib::ESeverities::Status
                        , tCIDLib::EErrClasses::AppStatus
                        , m_strRepoMoniker
                    );
                }

                //
                //  Go to the repository and get the media info for this new
                //  stuff. Zero the last track number so that the next poll
                //  will see the initially loaded track as a change and
                //  update the per-item info.
                //
                m_c4LastTrackNum = 0;
                LoadMediaData(orbcRepo, strTitleCookie, strColCookie);

                // Make sure the new changer is powered up
                if (!bSetChangerPower(kCIDLib::True))
                {
                    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Low)
                    {
                        facMediaChMgrS().LogMsg
                        (
                            CID_FILE
                            , CID_LINE
                            , L"Could not power up changer %(1)"
                            , tCIDLib::ESeverities::Status
                            , tCIDLib::EErrClasses::AppStatus
                            , strNewChanger
                        );
                    }
                    IncFailedWriteCounter();
                    return tCQCKit::ECommResults::ValueRejected;
                }

                if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
                {
                    facMediaChMgrS().LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , L"Selecting new slot %(1)"
                        , tCIDLib::ESeverities::Status
                        , tCIDLib::EErrClasses::AppStatus
                        , TCardinal(c4Slot)
                    );
                }

                //
                //  Now, let's try to make it select the indicated slot. If that
                //  doesn't fail, then we assume we are happy tunes and make this
                //  guy our new changer.
                //
                if (!bSelectSlot(c4Slot))
                {
                    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Low)
                    {
                        facMediaChMgrS().LogMsg
                        (
                            CID_FILE
                            , CID_LINE
                            , L"Could not select slot %(1) on changer %(2)"
                            , tCIDLib::ESeverities::Status
                            , tCIDLib::EErrClasses::AppStatus
                            , TCardinal(c4Slot)
                            , strNewChanger
                        );
                    }
                    IncFailedWriteCounter();
                    return tCQCKit::ECommResults::ValueRejected;
                }
            }

            catch(TError& errToCatch)
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);

                LogError(errToCatch, tCQCKit::EVerboseLvls::Low);
                if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Low)
                {
                    facMediaChMgrS().LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , L"An exception occured setting up change. Resetting..."
                        , tCIDLib::ESeverities::Status
                        , tCIDLib::EErrClasses::AppStatus
                    );
                }

                Reset();
                throw;
            }

            // Do an initial poll cycle to get everything set up
            bPollChanger();
        }
    }
     else if (c4FldId == m_c4FldId_Transport)
    {
        // If not active changer, then rject
        if (m_strChanger.bIsEmpty())
            return tCQCKit::ECommResults::ValueRejected;

        // Do the transport op if we have an active changer. If not, reject
        if (!bDoTransportOp(strValWritten))
            return tCQCKit::ECommResults::ValueRejected;
    }
     else
    {
        IncUnknownWriteCounter();
        return tCQCKit::ECommResults::ValueRejected;
    }
    return tCQCKit::ECommResults::Success;
}


tCIDLib::TVoid TMediaChMgrSDriver::ReleaseCommResource()
{
    // Nothing to do for this one
}


tCIDLib::TVoid TMediaChMgrSDriver::TerminateImpl()
{
    // Drop any proxy we have
    m_orbcDrv.DropRef();
}


// ---------------------------------------------------------------------------
//  TMediaChMgrSDriver: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  This is called when we get a transport operation field write. We get the
//  moniker of the changer we are supposed to send the operation to.
//
tCIDLib::TBoolean TMediaChMgrSDriver::bDoTransportOp(const TString& strOp)
{
    // If no active changer, then reject it
    if (m_strChanger.bIsEmpty())
        return kCIDLib::False;

    try
    {
        m_orbcDrv->WriteFieldByName
        (
            m_strChanger, L"Transport", strOp, tCQCKit::EDrvCmdWaits::DontCare
        );
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        LogError(errToCatch, tCQCKit::EVerboseLvls::Medium);
        return kCIDLib::False;
    }
    return kCIDLib::True;
}



//
//  We use the passed collection cookie to go back to our associated media
//  repo driver and get the location info for the indicated collection. It
//  must be a changer based location type or we throw.
//
tCIDLib::TBoolean
TMediaChMgrSDriver::bGetLocData(        tCQCKit::TCQCSrvProxy&  orbcSrv
                                , const TString&                strColCookie
                                ,       TString&                strMon
                                ,       tCIDLib::TCard4&        c4Slot)
{
    TString strLocInfo;
    try
    {
        const tCQCMedia::ELocTypes eLocType = facCQCMedia().eQueryColLoc
        (
            orbcSrv, m_strRepoMoniker, strColCookie, strLocInfo
        );

        if (eLocType != tCQCMedia::ELocTypes::Changer)
        {
            facCQCMedia().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kMedErrs::errcDB_WrongLocType
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::TypeMatch
                , tCQCMedia::strXlatELocTypes(eLocType)
            );
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        LogError(errToCatch, tCQCKit::EVerboseLvls::Medium);
        return kCIDLib::False;
    }

    // Break it out into moniker and slot, if valid
    return facCQCMedia().bParseChangerLoc(strLocInfo, strMon, c4Slot);
}


//
//  This is called to get our field I/O object loaded up with the fields that
//  we poll from the active changer. So it's called any time a new changer
//  is set, or when we are trying to connect, or when we get an out of sync
//  error from the changer we are polling and need to resync with him.
//
tCIDLib::TBoolean TMediaChMgrSDriver::bLoadPollList()
{
    try
    {
        TVector<TCQCFldDef> colFields(1);
        tCIDLib::TCard4 c4FldListId;
        tCIDLib::TCard4 c4DriverId;
        tCIDLib::TCard4 c4DriverListId;

        const tCIDLib::TCard4 c4Count = m_orbcDrv->c4QueryFields
        (
            m_strChanger
            , colFields
            , c4FldListId
            , c4DriverId
            , c4DriverListId
        );

        // Reset the I/O list to prep it for the new fields
        m_fiopPoll.Reset(c4DriverListId);
        m_fiopPoll.c4AddDriver(c4DriverId, c4FldListId);

        // Run through the fields and find the ones we want
        tCIDLib::TCard4 c4HisId;
        EMapFlds eFld;
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const TCQCFldDef& flddCur = colFields[c4Index];
            c4HisId = flddCur.c4Id();

            if (flddCur.strName() == L"CurrentDisc")
                eFld = EMapFld_CurrentDisc;
            else if (flddCur.strName() == L"CurrentTitle")
                eFld = EMapFld_CurrentTitle;
            else if (flddCur.strName() == L"CurrentTrack")
                eFld = EMapFld_CurrentTrack;
            else if (flddCur.strName() == L"PlayMode")
                eFld = EMapFld_PlayMode;
            else if (flddCur.strName() == L"Power")
                eFld = EMapFld_Power;
            else
                eFld = EMapFlds_Count;

            if (eFld != EMapFlds_Count)
            {
                TMappedFld& mfCur = m_aFldIdMap[eFld];
                mfCur.c4HisId = c4HisId;
                m_fiopPoll.c4AddField(c4DriverId, c4HisId, flddCur.eType());
            }
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        LogError(errToCatch, tCQCKit::EVerboseLvls::Medium);

        //
        //  Set the fields that reflect the state of the active changer to
        //  be in error state.
        //
        SetErrState();
        return kCIDLib::False;
    }
    return kCIDLib::True;
}




//
//  This is called to do a poll of the active changer, during our poll callback
//  and on the setting of a new changer to get our fields updated. We return
//  true if we stored a new value for the current rack, since this is something
//  we want to react to.
//
tCIDLib::TBoolean TMediaChMgrSDriver::bPollChanger()
{
    // Get the active changer. If none, then nothing to do
    if (m_strChanger.bIsEmpty())
        return kCIDLib::False;

    //
    //  First see if we have a client proxy. If don't have one, or if we do
    //  have one but it is offline, then we have to try to get another.
    //
    tCIDLib::TBoolean bRet = kCIDLib::False;
    tCIDLib::TBoolean bNewProxy = kCIDLib::False;
    if (!m_orbcDrv.pobjData() || m_orbcDrv->bCheckForLostConnection())
    {
        m_orbcDrv.DropRef();
        bNewProxy = kCIDLib::True;
        try
        {
            m_orbcDrv = facCQCKit().orbcCQCSrvAdminProxy(m_strChanger);
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            LogError(errToCatch, tCQCKit::EVerboseLvls::High);

            //
            //  Set the fields that reflect the state of the active changer to
            //  be in error state.
            //
            SetErrState();
            return bRet;
        }
    }

    // If we got a new proxy or the I/O packet is not setup, then do that
    if (bNewProxy || m_fiopPoll.bIsEmpty())
    {
        //
        //  If we can't load the list, then not much we can do. THis will
        //  put our fields that are driven by polling the active changer into
        //  error state if it fails.
        //
        if (!bLoadPollList())
            return bRet;
    }

    // We seem to be ready to poll data, so do it
    try
    {
        tCIDLib::TCard4 c4Bytes;
        m_orbcDrv->ReadFields(m_fiopPoll, c4Bytes, m_mbufPoll);

        // Read the information out of the flattened buffer of data
        TBinMBufInStream strmSrc(&m_mbufPoll, c4Bytes);
        strmSrc.CheckForStartMarker(CID_FILE, CID_LINE);

        // Make sure it's returning data in a form we understand
        tCIDLib::TCard1 c1FmtVersion;
        strmSrc >> c1FmtVersion;
        if (c1FmtVersion != kCQCKit::c1FldFmtVersion)
        {
            if (eVerboseLevel() >= tCQCKit::EVerboseLvls::High)
            {
                facCQCKit().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kKitErrs::errcFIOP_BadFmtVersion
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Format
                    , TCardinal(c1FmtVersion)
                    , clsIsA()
                );
            }
            SetErrState();
            return bRet;
        }

        // Looks ok, so loop through the data
        tCIDLib::TCard1 c1Marker;
        tCIDLib::TCard1 c1State;
        tCIDLib::TCard2 c2SrcFldId;
        tCIDLib::TCard4 c4SerialNum;
        tCIDLib::TCard4 c4Count;
        TString strVal;

        // First check the driver status. Use the src field id as a temp
        strmSrc.CheckForStartMarker(CID_FILE, CID_LINE);
        strmSrc >> c1Marker >> c2SrcFldId;

        if (c1Marker == kCQCKit::c1FldType_DriverOffline)
        {
            // There is also a driver state, though we don't use it
            strmSrc >> c1State;

            // Offline, so mark all our mapped fields offline
            for (tCIDLib::TInt4 i4Index = EMapFlds_Min; i4Index < EMapFlds_Max; i4Index++)
                SetFieldErr(m_aFldIdMap[i4Index].c4OurId, kCIDLib::True);
        }
         else
        {
            // Get the count of fields for the driver
            c4Count = m_fiopPoll.c4FieldCount(c2SrcFldId);
            tCIDLib::TCard4 c4FldId;
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            {
                //
                //  We get a start marker, then a market that indicates a
                //  field item, which we can ignore, then the field id and
                //  a field status marker.
                //
                strmSrc.CheckForStartMarker(CID_FILE, CID_LINE);
                strmSrc >> c1Marker >> c2SrcFldId >> c1Marker;

                if (c1Marker == kCQCKit::c1FldData_InError)
                {
                    // The field is in error, so set ours in error
                    c4FldId = c4MapFldId(c2SrcFldId);
                    if (c4FldId)
                        SetFieldErr(c4FldId, kCIDLib::True);
                }
                else if (c1Marker == kCQCKit::c1FldData_Changed)
                {
                    // Eat the serial number which we don't use
                    strmSrc >> c4SerialNum;

                    //
                    //  Now get out the value and write it to our field which
                    //  is mirroring that value.
                    //
                    if (c2SrcFldId == m_aFldIdMap[EMapFld_CurrentDisc].c4HisId)
                    {
                        tCIDLib::TCard4 c4Col;
                        strmSrc >> c4Col;
                        c4FldId = m_aFldIdMap[EMapFld_CurrentDisc].c4OurId;
                        bStoreCardFld(c4FldId, c4Col, kCIDLib::True);
                    }
                     else if (c2SrcFldId == m_aFldIdMap[EMapFld_CurrentTitle].c4HisId)
                    {
                        tCIDLib::TCard4 c4Num;
                        strmSrc >> c4Num;
                        c4FldId = m_aFldIdMap[EMapFld_CurrentTitle].c4OurId;
                        bStoreCardFld(c4FldId, c4Num, kCIDLib::True);
                    }
                     else if (c2SrcFldId == m_aFldIdMap[EMapFld_CurrentTrack].c4HisId)
                    {
                        tCIDLib::TCard4 c4Num;
                        strmSrc >> c4Num;
                        c4FldId = m_aFldIdMap[EMapFld_CurrentTrack].c4OurId;
                        bStoreCardFld(c4FldId, c4Num, kCIDLib::True);
                        if (c4Num != m_c4LastTrackNum)
                        {
                            bRet = kCIDLib::True;
                            m_c4LastTrackNum = c4Num;
                        }
                    }
                     else if (c2SrcFldId == m_aFldIdMap[EMapFld_PlayMode].c4HisId)
                    {
                        strmSrc >> strVal;
                        c4FldId = m_aFldIdMap[EMapFld_PlayMode].c4OurId;
                        bStoreStringFld(c4FldId, strVal, kCIDLib::True);
                    }
                     else if (c2SrcFldId == m_aFldIdMap[EMapFld_Power].c4HisId)
                    {
                        tCIDLib::TBoolean bVal;
                        strmSrc >> bVal;
                        c4FldId = m_aFldIdMap[EMapFld_Power].c4OurId;
                        bStoreBoolFld(c4FldId, bVal, kCIDLib::True);
                    }
                }
            }
        }
    }

    catch(TError& errToCatch)
    {
        //
        //  If it's just an out of sync error, then empty the field I/O list.
        //  Next time we come back in, we'll see that our list is empty and
        //  reset it and start again.
        //
        if (errToCatch.eClass() == tCIDLib::EErrClasses::OutOfSync)
        {
            m_fiopPoll.Reset(0);
        }
         else
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            LogError(errToCatch, tCQCKit::EVerboseLvls::High);
            SetErrState();
        }
        return bRet;
    }
    return bRet;
}


//
//  This is called when we get a disk slot selection field write. We pass it on
//  to the active changer, the moniker of which we get passed to us.
//
tCIDLib::TBoolean TMediaChMgrSDriver::bSelectSlot(const tCIDLib::TCard4 c4Slot)
{
    // If no active changer, then reject it
    if (m_strChanger.bIsEmpty())
        return kCIDLib::False;

    try
    {
        m_orbcDrv->WriteCardByName
        (
            m_strChanger, L"CurrentDisc", c4Slot, tCQCKit::EDrvCmdWaits::DontCare
        );
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        LogError(errToCatch, tCQCKit::EVerboseLvls::Medium);
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


//
//  This method is called to change the power state of the active changer.
//
tCIDLib::TBoolean
TMediaChMgrSDriver::bSetChangerPower(const tCIDLib::TBoolean bToSet)
{
    // If no active changer, then reject it
    if (m_strChanger.bIsEmpty())
        return kCIDLib::False;

    TThread* pthrCaller = 0;
    try
    {
        // Get the current state
        tCIDLib::TBoolean bPower;
        tCIDLib::TCard4 c4SerNum = 0;
        m_orbcDrv->bReadBoolByName(c4SerNum, m_strChanger, L"Power", bPower);

        // If not the same as the state being set, then set the new state
        const TString strField(L"CurrentDisc");
        TString strVal;
        if (bPower != bToSet)
        {
            if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
            {
                facMediaChMgrS().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , L"Changing power state of changer %(1) to %(2)"
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::AppStatus
                    , m_strChanger
                    , (bPower ? TString(L"On") : TString(L"Off"))
                );
            }

            m_orbcDrv->WriteBoolByName
            (
                m_strChanger, L"Power", bToSet, tCQCKit::EDrvCmdWaits::DontCare
            );

            //
            //  If the new state was power on, then wait for a while for it to
            //  some up. We'll watch the current disc value to come online and
            //  to have a non-zero value.
            //
            if (bToSet)
            {
                tCIDLib::TCard4 c4Count = 0;
                while (c4Count < 10)
                {
                    //
                    //  Read the value of the field. If it changed since the
                    //  last check, then evaluate the new value.
                    //
                    try
                    {
                        c4SerNum = 0;
                        tCQCKit::EFldTypes eType;
                        if (m_orbcDrv->bReadFieldByName(c4SerNum
                                                        , m_strChanger
                                                        , strField
                                                        , strVal
                                                        , eType))
                        {
                            if (strVal != L"0")
                                break;
                        }
                    }

                    catch(TError& errToCatch)
                    {
                        // If not a field in error exception, then give up
                        if (!errToCatch.bCheckEvent(facCQCKit().strName()
                                                    , kKitErrs::errcFld_FieldInErr))
                        {
                            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                            throw;
                        }
                    }

                    // Watch for shutdown requests
                    if (!pthrCaller)
                        pthrCaller = TThread::pthrCaller();
                    if (pthrCaller->bCheckShutdownRequest())
                        break;

                    TThread::Sleep(1000);
                    c4Count++;
                }
            }
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        LogError(errToCatch, tCQCKit::EVerboseLvls::Medium);
        return kCIDLib::False;
    }
    return kCIDLib::True;
}



//
//  We look up the ids of the fields in the active changer that we are going
//  to echo through our own fields, and put them into a map. This method will
//  map from one of those field ids, that we get during a poll, back to our
//  field where the value should be stored.
//
tCIDLib::TCard4 TMediaChMgrSDriver::c4MapFldId(const tCIDLib::TCard4 c4SrcFldId)
{
    for (tCIDLib::TCard4 c4Index = 0; c4Index < EMapFlds_Count; c4Index++)
    {
        const TMappedFld& mfCur = m_aFldIdMap[c4Index];
        if (mfCur.c4HisId == c4SrcFldId)
            return mfCur.c4OurId;
    }
    return 0;
}



//
//  This is called when a new title is passed to PlayMedia. Once we've got
//  it playing and got ourself updated to the new changer, we then try to
//  go get the media info from our associated repository.
//
//  This only loads the title/collection level stuff. The item level stuff
//  is kept up to date as we see the track changes during polling.
//
tCIDLib::TVoid TMediaChMgrSDriver::
LoadMediaData(          tCQCKit::TCQCSrvProxy&  orbcSrv
                , const TString&                strTitleCookie
                , const TString&                strColCookie)
{
    // Query the disc details from the repository driver
    tCIDLib::TCard4 c4ColCnt;
    TMediaCollect   mcolInfo;
    TString         strTitleName;
    const tCQCMedia::EMediaTypes eType = facCQCMedia().eQueryColDetails
    (
        orbcSrv
        , m_strRepoMoniker
        , strColCookie
        , mcolInfo
        , strTitleName
        , c4ColCnt
    );

    {
        // Lock for field access
        bStoreStringFld(m_c4FldId_CurAspectRatio, mcolInfo.strAspectRatio(), kCIDLib::True);
        bStoreStringFld(m_c4FldId_CurCast, mcolInfo.strCast(), kCIDLib::True);
        bStoreStringFld(m_c4FldId_CurColArtist, mcolInfo.strArtist(), kCIDLib::True);
        bStoreStringFld(m_c4FldId_CurColCookie, strColCookie, kCIDLib::True);
        bStoreStringFld(m_c4FldId_CurColName, mcolInfo.strName(), kCIDLib::True);
        bStoreStringFld(m_c4FldId_CurDescr, mcolInfo.strDescr(), kCIDLib::True);
        bStoreStringFld(m_c4FldId_CurLabel, mcolInfo.strLabel(), kCIDLib::True);
        bStoreStringFld(m_c4FldId_CurLeadActor, mcolInfo.strLeadActor(), kCIDLib::True);
        bStoreStringFld(m_c4FldId_CurRating, mcolInfo.strRating(), kCIDLib::True);
        bStoreStringFld(m_c4FldId_CurTitleCookie, strTitleCookie, kCIDLib::True);
        bStoreStringFld(m_c4FldId_CurTitleName, strTitleName, kCIDLib::True);
        bStoreCardFld(m_c4FldId_CurYear, mcolInfo.c4Year(), kCIDLib::True);
    }

    //
    //  If it's music oriented, then query the item level metadata for
    //  this collection. Else, clear all that stuff out. We don't store
    //  the item level stuff here. The polling logic does that as it
    //  sees the track number change.
    //
    if (eType == tCQCMedia::EMediaTypes::Music)
    {
        //
        //  No need to store the item location info, since we know they
        //  are all blank because this is changer based collection and
        //  therefore the location info is collection level.
        //
        tCQCMedia::TNameList    colItemLocs;
        tCQCMedia::TNameList    colRepos;
        tCQCMedia::TCard4List   fcolDurations;
        tCQCMedia::TCard4List   fcolListItemIds;
        const tCIDLib::TCard4 c4Count = facCQCMedia().c4QueryItemList
        (
            orbcSrv
            , m_strRepoMoniker
            , strColCookie
            , m_colItemNames
            , m_colItemCookies
            , colItemLocs
            , m_colItemArtists
            , fcolDurations
            , fcolListItemIds
            , colRepos
        );
    }
     else
    {
        // Clear the stored item metadata info and fields
        m_colItemArtists.RemoveAll();
        m_colItemCookies.RemoveAll();
        m_colItemNames.RemoveAll();

        bStoreStringFld(m_c4FldId_CurItemArtist, TString::strEmpty(), kCIDLib::True);
        bStoreStringFld(m_c4FldId_CurItemCookie, TString::strEmpty(), kCIDLib::True);
        bStoreStringFld(m_c4FldId_CurItemName, TString::strEmpty(), kCIDLib::True);
    }
}


//
//  Just gets all this field registration stuff out of the main driver logic
//  above.
//
tCIDLib::TVoid TMediaChMgrSDriver::RegisterFields()
{
    TVector<TCQCFldDef> colFlds;
    TCQCFldDef flddNew;

    // Do our driver specific fields
    flddNew.Set(L"ActiveChanger", tCQCKit::EFldTypes::String, tCQCKit::EFldAccess::Read);
    colFlds.objAdd(flddNew);

    flddNew.Set
    (
        L"Transport"
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Write
    );
    flddNew.bAlwaysWrite(kCIDLib::True);
    colFlds.objAdd(flddNew);

    // Do the fields that we echo through from the active changer
    flddNew.Set(L"CurrentDisc", tCQCKit::EFldTypes::Card, tCQCKit::EFldAccess::ReadWrite);
    colFlds.objAdd(flddNew);

    flddNew.Set(L"CurrentTitle", tCQCKit::EFldTypes::Card, tCQCKit::EFldAccess::Read);
    colFlds.objAdd(flddNew);

    flddNew.Set(L"CurrentTrack", tCQCKit::EFldTypes::Card, tCQCKit::EFldAccess::ReadWrite);
    colFlds.objAdd(flddNew);

    flddNew.Set
    (
        L"PlayMode"
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Read
        , L"Enum: Stop, Play, Pause"
    );
    colFlds.objAdd(flddNew);

    flddNew.Set(L"Power", tCQCKit::EFldTypes::Boolean, tCQCKit::EFldAccess::ReadWrite);
    colFlds.objAdd(flddNew);

    // Do the standard media renderer fields that we can support
    flddNew.Set
    (
        kCQCMedia::strMediaFld_CurAspectRatio
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Read
    );
    colFlds.objAdd(flddNew);

    flddNew.Set
    (
        kCQCMedia::strMediaFld_CurCast
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Read
    );
    colFlds.objAdd(flddNew);

    flddNew.Set
    (
        kCQCMedia::strMediaFld_CurColArtist
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Read
    );
    colFlds.objAdd(flddNew);

    flddNew.Set
    (
        kCQCMedia::strMediaFld_CurColCookie
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Read
    );
    colFlds.objAdd(flddNew);

    flddNew.Set
    (
        kCQCMedia::strMediaFld_CurColName
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Read
    );
    colFlds.objAdd(flddNew);

    flddNew.Set
    (
        kCQCMedia::strMediaFld_CurDescr
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Read
    );
    colFlds.objAdd(flddNew);

    flddNew.Set
    (
        kCQCMedia::strMediaFld_CurItemArtist
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Read
    );
    colFlds.objAdd(flddNew);

    flddNew.Set
    (
        kCQCMedia::strMediaFld_CurItemCookie
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Read
    );
    colFlds.objAdd(flddNew);

    flddNew.Set
    (
        kCQCMedia::strMediaFld_CurItemName
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Read
    );
    colFlds.objAdd(flddNew);

    flddNew.Set
    (
        kCQCMedia::strMediaFld_CurLabel
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Read
    );
    colFlds.objAdd(flddNew);

    flddNew.Set
    (
        kCQCMedia::strMediaFld_CurLeadActor
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Read
    );
    colFlds.objAdd(flddNew);

    flddNew.Set
    (
        kCQCMedia::strMediaFld_CurRating
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Read
    );
    colFlds.objAdd(flddNew);

    flddNew.Set
    (
        kCQCMedia::strMediaFld_CurTitleName
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Read
    );
    colFlds.objAdd(flddNew);

    flddNew.Set
    (
        kCQCMedia::strMediaFld_CurTitleCookie
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Read
    );
    colFlds.objAdd(flddNew);

    flddNew.Set
    (
        kCQCMedia::strMediaFld_CurYear
        , tCQCKit::EFldTypes::Card
        , tCQCKit::EFldAccess::Read
    );
    colFlds.objAdd(flddNew);

    flddNew.Set
    (
        kCQCMedia::strMediaFld_PlayMedia
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Write
        , kCIDLib::True
    );
    colFlds.objAdd(flddNew);

    //
    //  Now register our fields with the base driver class, and go back
    //  and look up the ids.
    //
    SetFields(colFlds);

    m_c4FldId_ActiveChanger = pflddFind(L"ActiveChanger", kCIDLib::True)->c4Id();
    m_c4FldId_PlayMedia = pflddFind(kCQCMedia::strMediaFld_PlayMedia, kCIDLib::True)->c4Id();
    m_c4FldId_Transport = pflddFind(L"Transport", kCIDLib::True)->c4Id();

    m_aFldIdMap[EMapFld_CurrentDisc].c4OurId = pflddFind(L"CurrentDisc", kCIDLib::True)->c4Id();
    m_aFldIdMap[EMapFld_CurrentTitle].c4OurId = pflddFind(L"CurrentTitle", kCIDLib::True)->c4Id();
    m_aFldIdMap[EMapFld_CurrentTrack].c4OurId = pflddFind(L"CurrentTrack", kCIDLib::True)->c4Id();
    m_aFldIdMap[EMapFld_PlayMode].c4OurId = pflddFind(L"PlayMode", kCIDLib::True)->c4Id();
    m_aFldIdMap[EMapFld_Power].c4OurId = pflddFind(L"Power", kCIDLib::True)->c4Id();

    m_c4FldId_CurAspectRatio = pflddFind(kCQCMedia::strMediaFld_CurAspectRatio, kCIDLib::True)->c4Id();
    m_c4FldId_CurCast = pflddFind(kCQCMedia::strMediaFld_CurCast, kCIDLib::True)->c4Id();
    m_c4FldId_CurColArtist = pflddFind(kCQCMedia::strMediaFld_CurColArtist, kCIDLib::True)->c4Id();
    m_c4FldId_CurColCookie = pflddFind(kCQCMedia::strMediaFld_CurColCookie, kCIDLib::True)->c4Id();
    m_c4FldId_CurColName = pflddFind(kCQCMedia::strMediaFld_CurColName, kCIDLib::True)->c4Id();
    m_c4FldId_CurDescr = pflddFind(kCQCMedia::strMediaFld_CurDescr, kCIDLib::True)->c4Id();
    m_c4FldId_CurItemArtist = pflddFind(kCQCMedia::strMediaFld_CurItemArtist, kCIDLib::True)->c4Id();
    m_c4FldId_CurItemCookie = pflddFind(kCQCMedia::strMediaFld_CurItemCookie, kCIDLib::True)->c4Id();
    m_c4FldId_CurItemName = pflddFind(kCQCMedia::strMediaFld_CurItemName, kCIDLib::True)->c4Id();
    m_c4FldId_CurLabel = pflddFind(kCQCMedia::strMediaFld_CurLabel, kCIDLib::True)->c4Id();
    m_c4FldId_CurLeadActor = pflddFind(kCQCMedia::strMediaFld_CurLeadActor, kCIDLib::True)->c4Id();
    m_c4FldId_CurRating = pflddFind(kCQCMedia::strMediaFld_CurRating, kCIDLib::True)->c4Id();
    m_c4FldId_CurTitleCookie = pflddFind(kCQCMedia::strMediaFld_CurTitleCookie, kCIDLib::True)->c4Id();
    m_c4FldId_CurTitleName = pflddFind(kCQCMedia::strMediaFld_CurTitleName, kCIDLib::True)->c4Id();
    m_c4FldId_CurYear = pflddFind(kCQCMedia::strMediaFld_CurYear, kCIDLib::True)->c4Id();
}


//
//  If we get confused, this is called to put us back to a starting point
//  again, so that we don't provide bogus info.
//
tCIDLib::TVoid TMediaChMgrSDriver::Reset()
{
    // Let any changer ref go
    m_strChanger.Clear();
    try
    {
        m_orbcDrv.DropRef();
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        LogError(errToCatch, tCQCKit::EVerboseLvls::Medium);
    }

    // Clear our field I/O list
    m_fiopPoll.Reset(0);

    // Clear out the metadata and active changer fields
    SetDefFldVals();

    // Put our echoed changer fields into error state
    SetErrState();
}


// Get's all our state related fields to default values
tCIDLib::TVoid TMediaChMgrSDriver::SetDefFldVals()
{
    // Clear all the media fields out
    bStoreStringFld(m_c4FldId_ActiveChanger, TString::strEmpty(), kCIDLib::True);
    bStoreStringFld(m_c4FldId_CurTitleCookie, TString::strEmpty(), kCIDLib::True);
    bStoreStringFld(m_c4FldId_CurAspectRatio, TString::strEmpty(), kCIDLib::True);
    bStoreStringFld(m_c4FldId_CurCast, TString::strEmpty(), kCIDLib::True);
    bStoreStringFld(m_c4FldId_CurColArtist, TString::strEmpty(), kCIDLib::True);
    bStoreStringFld(m_c4FldId_CurColCookie, TString::strEmpty(), kCIDLib::True);
    bStoreStringFld(m_c4FldId_CurColName, TString::strEmpty(), kCIDLib::True);
    bStoreStringFld(m_c4FldId_CurDescr, TString::strEmpty(), kCIDLib::True);
    bStoreStringFld(m_c4FldId_CurColArtist, TString::strEmpty(), kCIDLib::True);
    bStoreStringFld(m_c4FldId_CurItemArtist, TString::strEmpty(), kCIDLib::True);
    bStoreStringFld(m_c4FldId_CurItemCookie, TString::strEmpty(), kCIDLib::True);
    bStoreStringFld(m_c4FldId_CurItemName, TString::strEmpty(), kCIDLib::True);
    bStoreStringFld(m_c4FldId_CurLabel, TString::strEmpty(), kCIDLib::True);
    bStoreStringFld(m_c4FldId_CurLeadActor, TString::strEmpty(), kCIDLib::True);
    bStoreStringFld(m_c4FldId_CurRating, TString::strEmpty(), kCIDLib::True);
    bStoreStringFld(m_c4FldId_CurTitleCookie, TString::strEmpty(), kCIDLib::True);
    bStoreStringFld(m_c4FldId_CurTitleName, TString::strEmpty(), kCIDLib::True);
    bStoreCardFld(m_c4FldId_CurYear, 0, kCIDLib::True);
}


//  If we cannot get info from the active changer, we set the fields that are
//  reflecting active changer state to error status.
//
tCIDLib::TVoid TMediaChMgrSDriver::SetErrState()
{
    for (tCIDLib::TCard4 c4Index = 0; c4Index < EMapFlds_Count; c4Index++)
        SetFieldErr(m_aFldIdMap[c4Index].c4OurId, kCIDLib::True);
}

