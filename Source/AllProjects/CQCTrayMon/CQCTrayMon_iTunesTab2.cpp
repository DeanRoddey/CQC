//
// FILE NAME: CQCTrayMon_iTunesTab2.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/23/2012
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
//  large. This one handles the stuff that does the repository oriented
//  parts of interfacing to iTunes.
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
#undef      DeleteFile
#include    <iTunesCOMInterface.h>
#include    "CIDKernel_InternalHelpers_.hpp"



// ---------------------------------------------------------------------------
//  A derivative of the iTunes event handler class. We have to override all
//  of the callbacks in order to react to the changes. The tab creates it and
//  passes it a pointer to itself. This guy will call back on it to queue up
//  events for later processing by the background thread.
// ---------------------------------------------------------------------------
class TITEventHandler : public _IiTunesEvents
{
    public:
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TITEventHandler(TCQCTrayiTunesTab* pwndTab) :

            m_lRefCount(1)
            , m_pwndTab(pwndTab)
        {
        }

        ~TITEventHandler()
        {
        }

        STDMETHODIMP_(ULONG) AddRef()
        {
            return ::InterlockedIncrement(&m_lRefCount);
        }

        STDMETHODIMP QueryInterface(REFIID iid, LPVOID* ppvObject)
        {
            HRESULT hRes = S_OK;

            if(NULL == ppvObject)
                hRes = E_POINTER;
            else
                *ppvObject = NULL;

            if(SUCCEEDED(hRes))
            {
                if (IsEqualIID(iid, IID_IUnknown)
                ||  IsEqualIID(iid, IID_IDispatch)
                ||  IsEqualIID(iid, DIID__IiTunesEvents))
                {
                    *ppvObject = this;
                    AddRef();
                }
                 else
                {
                    *ppvObject = 0;
                    hRes = E_NOINTERFACE;
                }
            }
            return hRes;
        }

        STDMETHODIMP_(ULONG) Release()
        {
            LONG lRefCount = ::InterlockedDecrement(&m_lRefCount);
            if(0 == lRefCount)
                delete this;
            return lRefCount;
        }

        virtual HRESULT STDMETHODCALLTYPE GetTypeInfoCount(UINT* uCnt)
        {
            *uCnt = 0;
            return S_OK;
        }

        virtual HRESULT STDMETHODCALLTYPE
        GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo** ppTInfo){return E_NOTIMPL;}

        virtual HRESULT STDMETHODCALLTYPE
        GetIDsOfNames(REFIID,LPOLESTR*,UINT,LCID lcid,DISPID*){return E_NOTIMPL;}


        // These we handle out of line below
        virtual HRESULT STDMETHODCALLTYPE OnDatabaseChangedEvent
        (
            VARIANT     deletedObjIDs
            , VARIANT   changedObjIDs
        );

        virtual HRESULT STDMETHODCALLTYPE OnPlayerPlayEvent(VARIANT iTrack);

        virtual HRESULT STDMETHODCALLTYPE OnVolumeChange
        (
            VARIANT     newVolume
        );


        //
        //  Do standard dispatching
        //
        virtual HRESULT STDMETHODCALLTYPE
        Invoke( DISPID          dispidMember
                , REFIID
                , LCID
                , WORD          wFlags
                , DISPPARAMS*   pdispparams
                , VARIANT*      pResult
                , EXCEPINFO*    pExcepInfo
                , UINT*         puArgErr)
        {
            HRESULT hRes = S_OK;
            switch (dispidMember)
            {
                case ITEventCOMCallsDisabled :
                    break;

                case ITEventCOMCallsEnabled :
                    break;

                case ITEventDatabaseChanged :
                    OnDatabaseChangedEvent
                    (
                        pdispparams->rgvarg[1], pdispparams->rgvarg[0]
                    );
                    break;

                case ITEventQuitting :
                    // Let the tab know we saw this
                    m_pwndTab->OnQuitNotification();
                    break;

                case ITEventPlayerPlay :
                    OnPlayerPlayEvent(pdispparams->rgvarg[0]);
                    break;

                case ITEventPlayerStop :
                    m_pwndTab->OnStop();
                    break;

                case ITEventSoundVolumeChanged :
                    OnVolumeChange(pdispparams->rgvarg[0]);
                    break;

                default:
                    break;
            }
            return hRes;
        }

    private :
        LONG                m_lRefCount;
        TCQCTrayiTunesTab*  m_pwndTab;
};


//
//  Called when changes are made to the database. We just queue up the info
//  on the tab window, and its background thread will process the info. We
//  get lists of permanent object ids that we can use to find the affected
//  objects.
//
HRESULT
TITEventHandler::OnDatabaseChangedEvent(VARIANT     deletedObjIDs
                                        , VARIANT   changedObjIDs)
{
    if ((deletedObjIDs.vt != (VT_ARRAY | VT_VARIANT))
    ||  (changedObjIDs.vt != (VT_ARRAY | VT_VARIANT)))
    {
        return E_INVALIDARG;
    }

    // Update any that have been deleted
    VARIANT vSrc;
    VARIANT vPL;
    VARIANT vTrack;
    VARIANT vDB;

    ::VariantInit(&vSrc);
    ::VariantInit(&vPL);
    ::VariantInit(&vTrack);
    ::VariantInit(&vDB);
    {
        SAFEARRAY* pArray = V_ARRAY(&deletedObjIDs);
        long lLBound, lUBound;
        if (FAILED(::SafeArrayGetLBound(pArray, 1, &lLBound))
        ||  FAILED(::SafeArrayGetUBound(pArray, 1, &lUBound)))
        {
            return E_INVALIDARG;
        }

        long alIndices[2];
        for (long lIndex = lLBound; lIndex <= lUBound; lIndex++)
        {
            alIndices[0] = lIndex;

            //
            //  Normally we'd use tCIDLib::pToVoidPP, but these are particularly messy casts,
            //  so we just use reinterpret and suppress the warnings.
            //
            alIndices[1] = 0;
            ::SafeArrayGetElement(pArray, alIndices, reinterpret_cast<void**>(&vSrc));

            alIndices[1] = 1;
            ::SafeArrayGetElement(pArray, alIndices, reinterpret_cast<void**>(&vPL));

            alIndices[1] = 2;
            ::SafeArrayGetElement(pArray, alIndices, reinterpret_cast<void**>(&vTrack));

            alIndices[1] = 3;
            ::SafeArrayGetElement(pArray, alIndices, reinterpret_cast<void**>(&vDB));

            m_pwndTab->OnDBChange
            (
                V_I4(&vSrc), V_I4(&vPL), V_I4(&vTrack), V_I4(&vDB), kCIDLib::True
            );

            ::VariantClear(&vSrc);
            ::VariantClear(&vPL);
            ::VariantClear(&vTrack);
            ::VariantClear(&vDB);
        }
    }

    // And any that have been changed
    {
        SAFEARRAY* pArray = V_ARRAY(&changedObjIDs);
        long lLBound, lUBound;
        if (FAILED(::SafeArrayGetLBound(pArray, 1, &lLBound))
        ||  FAILED(::SafeArrayGetUBound(pArray, 1, &lUBound)))
        {
            return E_INVALIDARG;
        }

        long alIndices[2];
        for (long lIndex = lLBound; lIndex <= lUBound; lIndex++)
        {
            alIndices[0] = lIndex;

            //
            //  Normally we'd use tCIDLib::pToVoidPP, but these are particularly messy casts,
            //  so we just use reinterpret and suppress the warnings.
            //
            alIndices[1] = 0;
            ::SafeArrayGetElement(pArray, alIndices, reinterpret_cast<void**>(&vSrc));

            alIndices[1] = 1;
            ::SafeArrayGetElement(pArray, alIndices, reinterpret_cast<void**>(&vPL));

            alIndices[1] = 2;
            ::SafeArrayGetElement(pArray, alIndices, reinterpret_cast<void**>(&vTrack));

            alIndices[1] = 3;
            ::SafeArrayGetElement(pArray, alIndices, reinterpret_cast<void**>(&vDB));

            m_pwndTab->OnDBChange
            (
                V_I4(&vSrc), V_I4(&vPL), V_I4(&vTrack), V_I4(&vDB), kCIDLib::False
            );
        }

        ::VariantClear(&vSrc);
        ::VariantClear(&vPL);
        ::VariantClear(&vTrack);
        ::VariantClear(&vDB);
    }

    return S_OK;
}

HRESULT TITEventHandler::OnPlayerPlayEvent(VARIANT iTrack)
{
    IITTrack* pTrack = 0;

    VARIANT vtDisp;
    ::VariantInit(&vtDisp);
    if SUCCEEDED(::VariantChangeType(&vtDisp, &iTrack, 0, VT_DISPATCH))
    {
        vtDisp.pdispVal->QueryInterface(__uuidof(pTrack), tCIDLib::pToVoidPP(&pTrack));
        if (pTrack)
        {
            BSTR bsTmp;
            tCIDLib::TCard4 c4Duration = 0;
            TString         strAlbum;
            TString         strArtist;
            TString         strTrack;

            // Get the albm name
            {
                pTrack->get_Album(&bsTmp);
                if (bsTmp)
                {
                    strAlbum = bsTmp;
                    ::SysFreeString(bsTmp);
                }
            }

            // Get the artist name
            {
                pTrack->get_Artist(&bsTmp);
                if (bsTmp)
                {
                    strArtist = bsTmp;
                    ::SysFreeString(bsTmp);
                }
            }

            // Get the track name
            {
                pTrack->get_Name(&bsTmp);
                if (bsTmp)
                {
                    strTrack = bsTmp;
                    ::SysFreeString(bsTmp);
                }
            }

            // Get the duration info
            long iDuration;
            if (SUCCEEDED(pTrack->get_Duration(&iDuration)))
                c4Duration = tCIDLib::TCard4(iDuration);

            ::VariantClear(&vtDisp);
            pTrack->Release();

            // Pass this info on to the tab window
            m_pwndTab->OnTrackChange(strAlbum, strArtist, strTrack, c4Duration);
        }
    }
    return S_OK;
}


//
//  There really isn't a mute in iTunes. It just sets the volume to zero.
//  But we expose one for convenience. If the volume goes to zero the tab
//  window sets the mute value true, else false.
//
HRESULT TITEventHandler::OnVolumeChange(VARIANT newVolume)
{
    VARIANT vtVolume;
    ::VariantInit(&vtVolume);
    if SUCCEEDED(::VariantChangeType(&vtVolume, &newVolume, 0, VT_I4))
    {
        tCIDLib::TCard4 c4Volume = tCIDLib::TCard4(vtVolume.lVal);
        if (c4Volume > 100)
            c4Volume = 100;
        m_pwndTab->OnVolumeChange(c4Volume);

        ::VariantClear(&vtVolume);
    }
    return S_OK;
}



// ---------------------------------------------------------------------------
//  TCQCTrayiTunesTab: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Called by the database event handler when there are changes to process.
//  We just queue them up and the bgn thread will handle them.
//
tCIDLib::TVoid
TCQCTrayiTunesTab::OnDBChange(  const   tCIDLib::TInt4      i4Src
                                , const tCIDLib::TInt4      i4PL
                                , const tCIDLib::TInt4      i4Track
                                , const tCIDLib::TInt4      i4DB
                                , const tCIDLib::TBoolean   bDelete)
{
    TLocker lockrSync(&m_mtxSync);

    if (m_colDBChanges.bIsFull(2048))
        QLogMsg(L"iTunes DB change queue is full, dropped events");
    else
        m_colDBChanges.objAdd(TDBQChangeInfo(i4Src, i4PL, i4Track, i4DB, bDelete));
}


// Called by the event handler when playback has stopped
tCIDLib::TVoid TCQCTrayiTunesTab::OnStop()
{
    TLocker lockrSync(&m_mtxSync);

    m_enctCurPos = 0;
    m_enctCurTotal = 0;
    m_ePlState = TiTPlayerIntfServerBase::EPlStates::Stopped;
    m_strCurAlbum.Clear();
    m_strCurArtist.Clear();
    m_strCurTrack.Clear();

    // Bump the player serial number
    m_c4PlSerialNum++;
    if (!m_c4PlSerialNum)
        m_c4PlSerialNum++;
}



// Our event handler calls this if an iTunes quit notification is recieved
tCIDLib::TVoid TCQCTrayiTunesTab::OnQuitNotification()
{
    m_bQuitNotification = kCIDLib::True;
    QLogMsg(L"Got iTunes quit notification");
}


//
//  Called by the event handler when a new track starts playing. We lock
//  and store the info away.
//
tCIDLib::TVoid
TCQCTrayiTunesTab::OnTrackChange(const  TString&        strAlbum
                                , const TString&        strArtist
                                , const TString&        strTrack
                                , const tCIDLib::TCard4 c4Duration)
{
    TLocker lockrSync(&m_mtxSync);

    // Init the current position to zero
    m_enctCurPos = 0;

    m_enctCurTotal = c4Duration * kCIDLib::enctOneSecond;
    m_ePlState = TiTPlayerIntfServerBase::EPlStates::Playing;
    m_strCurAlbum = strAlbum;
    m_strCurArtist = strArtist;
    m_strCurTrack = strTrack;

    // Bump the player serial number
    m_c4PlSerialNum++;
    if (!m_c4PlSerialNum)
        m_c4PlSerialNum++;
}


// Called by the event handler when the volume changes
tCIDLib::TVoid
TCQCTrayiTunesTab::OnVolumeChange(const tCIDLib::TCard4 c4Volume)
{
    // Make absolutely sure it's good
    if (c4Volume > 100)
    {
        return;
    }

    TLocker lockrSync(&m_mtxSync);

    //
    //  If it's changed, then store it and bump the player serial number.
    //  We also set the mute state based on volume. The player provides a
    //  mute command, but it really just sets the volume to zero.
    //
    if (m_c4CurVolume != c4Volume)
    {
        m_bCurMute = (c4Volume == 0);
        m_c4CurVolume = c4Volume;
        m_c4PlSerialNum++;
        if (!m_c4PlSerialNum)
            m_c4PlSerialNum++;
    }
}



// ---------------------------------------------------------------------------
//  TCQCTrayiTunesTab: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  A helper to avoid a lot of redundant code. After we try to get an object
//  of some sort, this is called to check if it worked. If not, an error is
//  logged.
//
tCIDLib::TBoolean
TCQCTrayiTunesTab::bCheckGetObj(const   tCIDLib::TCard4     c4Res
                                , const tCIDLib::TCh* const pszType
                                , const tCIDLib::TCard4     c4Line)
{
    if (FAILED(c4Res))
    {
        facCQCTrayMon.LogMsg
        (
            CID_FILE
            , c4Line
            , kTrayMonErrs::errcSrv_GetObject
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , TString(pszType)
            , TCardinal(c4Res, tCIDLib::ERadices::Hex)
        );
        return kCIDLib::False;
    }
    return kCIDLib::True;
}



//
//  A helper to pull the info out of a track and return it. We may need
//  to do this from more than one place and it's way too much to replicate.
//
//  We return true if we processed the track successfully, in which case the
//  caller should deal with the returned info.
//
tCIDLib::TBoolean TCQCTrayiTunesTab::
bExtractTrackInfo(  IITTrack* const             pTrack
                    , TString&                  strAlbumTitle
                    , TString&                  strAlbumArtist
                    , TString&                  strArtist
                    , TString&                  strGenre
                    , TString&                  strLocInfo
                    , TString&                  strSortTitle
                    , TString&                  strTrackName
                    , TString&                  strTrackUID
                    , TString&                  strTrackAltUID
                    , tCIDLib::TEncodedTime&    enctAdded
                    , tCIDLib::TCard4&          c4BitRate
                    , tCIDLib::TCard4&          c4Duration
                    , tCIDLib::TCard4&          c4Rating
                    , tCIDLib::TCard4&          c4SampleRate
                    , tCIDLib::TCard4&          c4TrackNum
                    , tCIDLib::TCard4&          c4Year)
{
    HRESULT hRes;

    // See what kind it is. We only care about CD/file based tracks
    ITTrackKind Kind;
    if (FAILED(pTrack->get_Kind(&Kind)))
    {
        m_c4TracksFailed++;
        return kCIDLib::False;
    }

    // We can only do file or ripped CD type tracks
    if ((Kind != ITTrackKindFile) && (Kind != ITTrackKindCD))
    {
        m_c4TracksIgnored++;
        return kCIDLib::False;
    }

    // Get the file/CD track version of this one
    IITFileOrCDTrack* pFileTrack = 0;
    hRes = pTrack->QueryInterface(__uuidof(pFileTrack),(void **)&pFileTrack);
    if (FAILED(hRes))
    {
        m_c4TracksFailed++;
        return kCIDLib::False;
    }
    TCOMJanitor<IITFileOrCDTrack> janFileTrack(&pFileTrack);

    // We can only handle non-video tracks
    ITVideoKind VideoKind;
    if (FAILED(pFileTrack->get_VideoKind(&VideoKind)))
    {
        m_c4TracksIgnored++;
        return kCIDLib::False;
    }
    if (VideoKind != ITVideoKindNone)
    {
        m_c4TracksIgnored++;
        return kCIDLib::False;
    }

    // Looks like we will keep it
    c4BitRate = 0;
    c4Duration = 0;
    c4Rating = 0;
    c4SampleRate = 0;
    c4TrackNum = 0;
    c4Year = 0;
    enctAdded = 0;
    strAlbumTitle.Clear();
    strAlbumArtist.Clear();
    strArtist.Clear();
    strGenre.Clear();
    strLocInfo.Clear();
    strSortTitle.Clear();
    strTrackName.Clear();
    strTrackUID.Clear();
    strTrackAltUID.Clear();

    // Get all the info out that we need
    BSTR bsAlbumTitle = 0;
    BSTR bsArtist = 0;
    BSTR bsGenre = 0;
    BSTR bsLocInfo = 0;
    BSTR bsTrackName = 0;
    DATE dtAdded = 0;
    long iBitRate = 0;
    long iDuration = 0;
    long iRating = 0;
    long iSampleRate = 0;
    long iTrackNum = 0;
    long iYear = 0;

    pTrack->get_Name(&bsTrackName);
    if (bsTrackName)
    {
        strTrackName = bsTrackName;
        ::SysFreeString(bsTrackName);
    }

    pTrack->get_Album(&bsAlbumTitle);
    if (bsAlbumTitle)
    {
        strAlbumTitle = bsAlbumTitle;
        ::SysFreeString(bsAlbumTitle);
    }

    pTrack->get_Artist(&bsArtist);
    if (bsArtist)
    {
        strArtist = bsArtist;
        ::SysFreeString(bsArtist);
    }

    if (SUCCEEDED(pTrack->get_BitRate(&iBitRate)))
        c4BitRate = tCIDLib::TCard4(iBitRate) * 1000;

    if (SUCCEEDED(pTrack->get_Duration(&iDuration)))
        c4Duration = tCIDLib::TCard4(iDuration);

    pTrack->get_Genre(&bsGenre);
    if (bsGenre)
    {
        strGenre = bsGenre;
        ::SysFreeString(bsGenre);
    }

    if (SUCCEEDED(pTrack->get_Rating(&iRating)))
    {
        c4Rating = tCIDLib::TCard4(iRating) / 10;
        if (c4Rating > 10)
            c4Rating = 0;
    }

    if (SUCCEEDED(pTrack->get_SampleRate(&iSampleRate)))
        c4SampleRate = tCIDLib::TCard4(iSampleRate);

    if (SUCCEEDED(pTrack->get_TrackNumber(&iTrackNum)))
        c4TrackNum = tCIDLib::TCard4(iTrackNum);

    if (SUCCEEDED(pTrack->get_Year(&iYear)))
        c4Year = tCIDLib::TCard4(iYear);

    //
    //  If we are missing anything we have to have so far, then return false
    //  now before we bother doing a bunch of other stuff.
    //
    //  Note that, when getting the object id, we reverse the values from
    //  what you'd think. For us, the first one, which has all the values, is
    //  the alt id, and the track one is the actual track UID.
    //
    if (strAlbumTitle.bIsEmpty()
    ||  strTrackName.bIsEmpty()
    ||  strArtist.bIsEmpty()
    ||  !c4TrackNum
    ||  !bGetObjectId(pTrack, strTrackUID, strTrackAltUID))
    {
        if (facCQCTrayMon.eVerboseLvl() >= tCQCKit::EVerboseLvls::Medium)
        {
            if (!strTrackName.bIsEmpty())
                QLogMsg(L"Ignored track missing required info. Track='%(1)'", strTrackName);
            else if (!strAlbumTitle.bIsEmpty())
                QLogMsg(L"Ignored track with no name. From title '%(1)'", strAlbumTitle);
            else
                QLogMsg(L"Ignored track is missing both album and track name info");
        }

        m_c4TracksIgnored++;
        return kCIDLib::False;
    }

    // Get the added date
    if (SUCCEEDED(pTrack->get_DateAdded(&dtAdded)))
    {
        //
        //  Convert the date to our time format. Trash the hours part of
        //  it first, since all we care about is the date. The hours are
        //  the fractional part.
        //
        tCIDLib::TFloat8    f8Days;
        FILETIME            FlTime;
        SYSTEMTIME          SysTime;
        TMathLib::f8Split(dtAdded, f8Days);
        ::VariantTimeToSystemTime(f8Days, &SysTime);
        ::SystemTimeToFileTime(&SysTime, &FlTime);
        enctAdded = TRawBits::c8From32
        (
            FlTime.dwLowDateTime, FlTime.dwHighDateTime
        ) - TKrnlTimeStamp::enctBaseOfs;
    }

    // The rest we get through the file/CD track interface
    pFileTrack->get_Location(&bsLocInfo);
    if (bsLocInfo)
    {
        strLocInfo = bsLocInfo;
        ::SysFreeString(bsLocInfo);
    }

    BSTR bsSortTitle = 0;
    pFileTrack->get_SortAlbum(&bsSortTitle);
    if (bsSortTitle)
    {
        strSortTitle = bsSortTitle;
        ::SysFreeString(bsSortTitle);
    }

    //
    //  If we didn't get a track level rating, then let's get the
    //  album level rating.
    //
    if (!c4Rating)
    {
        if (SUCCEEDED(pFileTrack->get_AlbumRating(&iRating)))
        {
            c4Rating = tCIDLib::TCard4(iRating) / 10;
            if (c4Rating > 10)
                c4Rating = 10;
        }
    }


    // If we didn't get location info, we ignore this one
    if (strLocInfo.bIsEmpty())
        return kCIDLib::False;

    // If no sort title, make one up
    if (strSortTitle.bIsEmpty())
        facCQCMedia().bMakeSortTitle(strAlbumTitle, strSortTitle);

    //
    //  See if we have an album artist. If not, then just set the
    //  regular artist as the album artist.
    //
    BSTR bsAlbumArtist = 0;
    pFileTrack->get_AlbumArtist(&bsAlbumArtist);
    if (bsAlbumArtist)
    {
        strAlbumArtist = bsAlbumArtist;
        ::SysFreeString(bsAlbumArtist);
    }
     else
    {
        strAlbumArtist = strArtist;
    }
    return kCIDLib::True;
}



//
//  A helper to get id info for a given object and format it out into the
//  form that we store in our database to link back to the object.
//
//  For tracks we have two, one for our own purposes to map back to our
//  items from incoming events. The other is to map back from our side
//  to tracks in the iTunes database.
//
//  For non-tracks, the track one is not filled in.
//
tCIDLib::TBoolean
TCQCTrayiTunesTab::bGetObjectId(IITObject* const    pObject
                                , TString&          strUID
                                , TString&          strTrackUID)
{
    tCIDLib::TInt4 i4Src, i4PL, i4Track, i4DB;
    if (SUCCEEDED(pObject->GetITObjectIDs(&i4Src, &i4PL, &i4Track, &i4DB)))
    {
        FormatObjUID(i4Src, i4PL, i4Track, i4DB, strUID, strTrackUID);
        return kCIDLib::True;
    }

    strUID.Clear();
    return kCIDLib::False;
}



//
//  Called during reload or incremental update when we see a new playlist.
//  We check out it's attributes and, if it's something we want to keep and
//  we don't already have it in the list, we add it and return true and set
//  the passed title set and collection. If we already have it, we return the
//  title set and collection and return false.
//
//  If we can't add it at all, we return false and the new pointers are null.
//
//  Note that we don't react to folders being added to the main library list.
//  So we really are only dealing with playlists here (in our definition of
//  a playlist.) So this never affects the title count and never gets called
//  for the addition of a new collection. We drive that off of the addition
//  of the individual items.
//
tCIDLib::TBoolean
TCQCTrayiTunesTab::bProcessPlayList(TMediaDB&               mdbTar
                                    , IITPlaylist* const    pPList
                                    , tCIDLib::TCard2&      c2TitleId
                                    , tCIDLib::TCard2&      c2ColId)
{
    c2TitleId = kCIDLib::c2MaxCard;
    c2ColId = kCIDLib::c2MaxCard;

    // Get the kind
    ITPlaylistKind PLKind;
    pPList->get_Kind(&PLKind);

    // We only do user created playlists
    if ((PLKind != ITPlaylistKindUser)
    &&  (PLKind != ITPlaylistKindUnknown))
    {
        return kCIDLib::False;
    }

    // It's of the right type so look at it as a user playlist
    IITUserPlaylist* pUserPL = 0;
    if (FAILED(pPList->QueryInterface(__uuidof(pUserPL), tCIDLib::pToVoidPP(&pUserPL))))
        return kCIDLib::False;
    TCOMJanitor<IITUserPlaylist> janUsrPL(&pUserPL);

    // Looks like something we care about, so get the playlist name
    BSTR bsPLName = 0;
    pPList->get_Name(&bsPLName);
    if (!bsPLName)
        return kCIDLib::False;
    TString strPLName = bsPLName;
    ::SysFreeString(bsPLName);

    // Create a sort title for this playlist name
    TString strSortTitle;
    facCQCMedia().bMakeSortTitle(strPLName, strSortTitle);

    // And now get the unique id of it. Ignore the track id in this case
    TString strPLUID;
    if (!bGetObjectId(pPList, strPLUID, m_strThreadTmp))
        return kCIDLib::False;

    //
    //  It looks ok, so let's handle it. If there's no title set
    //  with this id, add it, else we'll update the existing one.
    //
    {
        const TMediaTitleSet* pmtsPL = mdbTar.pmtsByUniqueId
        (
            m_eMediaType, strPLUID, kCIDLib::False
        );

        //
        //  If we already have it, then just get the collection and return false
        //  to say it's not a new one.
        //
        if (pmtsPL)
        {
            c2TitleId = pmtsPL->c2Id();
            c2ColId = pmtsPL->c2ColIdAt(0);
            return kCIDLib::False;
        }
    }

    // We need to add a new playlist and collection
    TMediaTitleSet* pmtsNew = new TMediaTitleSet
    (
        strPLName, strPLUID, strSortTitle, m_eMediaType
    );
    TJanitor<TMediaTitleSet> janSet(pmtsNew);

    if (facCQCTrayMon.eVerboseLvl() >= tCQCKit::EVerboseLvls::High)
        QLogMsg(L"Added new PlayList: %(1)", strPLName);

    // Bump the title count
    m_c4TitleCnt++;

    //
    //  Add a single collection. Use the title set unique id for the id, since it
    //  doesn't really matter in this repo.
    //
    //  Zero the year, we'll aggregate it later from item level info.
    //
    TMediaCollect* pmcolNew = new TMediaCollect
    (
        strPLName
        , pmtsNew->strUniqueId()
        , 0
        , tCQCMedia::ELocTypes::FileItem
        , m_eMediaType
    );
    TJanitor<TMediaCollect> janCol(pmcolNew);

    // Mark it as a playlist since these always are
    pmcolNew->bIsPlayList(kCIDLib::True);

    // Add to the playlists category
    pmcolNew->bAddCategory(kCQCMedia::c2CatId_PlayLists);

    // Add the collection to the database, and then to the set once the id is set
    mdbTar.c2AddCollect(janCol.pobjOrphan());
    pmtsNew->bAddCollect(*pmcolNew);

    // Add the title set now
    mdbTar.c2AddTitle(janSet.pobjOrphan());

    c2TitleId = pmtsNew->c2Id();
    c2ColId = pmcolNew->c2Id();

    QLogMsg(L"Added new playlist '%(1)'", pmtsNew->strName());

    // And indicate that we added a new one
    return kCIDLib::True;
}



//
//  This is called during load or incremental updates, to add a new
//  track to the database, or to update the cottents of an existing
//  item.
//
//  If we have to create a new title set and collection we do, and return
//  true, and the new objects. If they are already there we return false but
//  return the existing objects. If we can't process this track for some
//  reason, we return false and zeroed objects.
//
//  The caller tells us if this is being called from a change event handler,
//  for logging reasons. We only log the creation of new stuff for event
//  driven stuff.
//
//
//  NOTE THAT THIS should only be called for tracks within the main list,
//  not within playlists. This guy creates new items. Playlists only need
//  to reference existing items. Other code handles that.
//
//
//  NOTE THAT we can be called either for a previously unseen item (during
//  a full reload), or for an update of a track during incremental updates
//  driven by change events. In the later case, we may need to move the
//  item to a newly created set/collection if the artist/album name was
//  changed (since that's how we create the unique id for collections.)
//
//  We return true if this caused the creation of a new set, else false,
//  in case anyone cares.
//
tCIDLib::TBoolean
TCQCTrayiTunesTab::bProcessTrack(       TMediaDB&           mdbTar
                                ,       IITTrack* const     pTrack
                                ,       tCIDLib::TCard2&    c2TitleId
                                ,       tCIDLib::TCard2&    c2ColId
                                ,       tCIDLib::TCard2&    c2ItemId
                                , const tCIDLib::TBoolean   bFromEvent)
{
    c2TitleId = 0;
    c2ColId = 0;
    c2ItemId = 0;

    // Get the info out of this track
    tCIDLib::TCard4         c4BitRate;
    tCIDLib::TCard4         c4Duration;
    tCIDLib::TCard4         c4Rating;
    tCIDLib::TCard4         c4SampleRate;
    tCIDLib::TCard4         c4TrackNum;
    tCIDLib::TCard4         c4Year;;
    tCIDLib::TEncodedTime   enctAdded;
    TString                 strAlbumArtist;
    TString                 strAlbumTitle;
    TString                 strArtist;
    TString                 strGenre;
    TString                 strLocInfo;
    TString                 strPLName;
    TString                 strSortTitle;
    TString                 strTmp;
    TString                 strTrackName;
    TString                 strTrackUID;
    TString                 strUID;
    const tCIDLib::TBoolean bSkip = !bExtractTrackInfo
    (
        pTrack
        , strAlbumTitle
        , strAlbumArtist
        , strArtist
        , strGenre
        , strLocInfo
        , strSortTitle
        , strTrackName
        , strUID
        , strTrackUID
        , enctAdded
        , c4BitRate
        , c4Duration
        , c4Rating
        , c4SampleRate
        , c4TrackNum
        , c4Year
    );

    if (bSkip)
        return kCIDLib::False;

    // Create the unique id from this guy's album artist/album combo
    strTmp = strAlbumArtist;
    strTmp.Append(L'|');
    strTmp.Append(strAlbumTitle);

    strTmp.ToUpper();
    m_mdigMD5.StartNew();
    m_mdigMD5.DigestStr(strTmp);
    m_mdigMD5.Complete(m_mhashUID);

    TString strAlbumUID;
    m_mhashUID.FormatToStr(strAlbumUID);

    // Store the art for this guy, if any
    const TMediaImg* pmimgArt = pmimgStoreArt(mdbTar, pTrack, strAlbumUID);

    //
    //  It looks ok, so let's handle it. If there's no title set with this id, add
    //  it, else we'll update the existing one.
    //
    const TMediaTitleSet* pmtsTrack = mdbTar.pmtsByUniqueId
    (
        m_eMediaType, strAlbumUID, kCIDLib::False
    );

    //
    //  If we haven't created the collection yet, then do so. If we have do a little
    //  updating of the collection.
    //
    tCIDLib::TBoolean bNewSet = kCIDLib::False;
    const TMediaCollect* pmcolTrack = 0;
    if (pmtsTrack)
    {
        //
        //  It already exists, so get the collection out. We don' have
        //  the info to figure out multi-collection title sets so it'll
        //  always be the 0th collection.
        //
        pmcolTrack = &pmtsTrack->mcolAt(mdbTar, 0);
    }
     else
    {
        bNewSet = kCIDLib::True;

        TMediaTitleSet* pmtsNew = new TMediaTitleSet
        (
            strAlbumTitle, strAlbumUID, strSortTitle, m_eMediaType
        );
        TJanitor<TMediaTitleSet> janSet(pmtsNew);

        if (facCQCTrayMon.eVerboseLvl() >= tCQCKit::EVerboseLvls::High)
            QLogMsg(L"Added new Title: %(1)", strAlbumTitle);

        //
        //  Add a single collection. Use the title set unique id for
        //  the disc id, since it doesn't really matter in this repo.
        //
        //  Zero the year, we'll aggregate it later from item level
        //  info.
        //
        TMediaCollect* pmcolNew = new TMediaCollect
        (
            strAlbumTitle
            , pmtsNew->strUniqueId()
            , 0
            , tCQCMedia::ELocTypes::FileItem
            , m_eMediaType
        );
        TJanitor<TMediaCollect> janCol(pmcolNew);

        // Add it to the all music category
        pmcolNew->bAddCategory(kCQCMedia::c2CatId_AllMusic);

        // We added a new (real, not playlist) title set
        m_c4TitleCnt++;

        if (bFromEvent)
            QLogMsg(L"Added new title '%(1)'", pmtsNew->strName());

        // Add it to the database and the title set
        mdbTar.c2AddCollect(janCol.pobjOrphan());
        pmtsNew->bAddCollect(*pmcolNew);

        // Add the set now
        mdbTar.c2AddTitle(janSet.pobjOrphan());

        // And now set them into other pointers for subsequent use below
        pmcolTrack = pmcolNew;
        pmtsTrack = pmtsNew;
    }

    // We can set up the title and collection ids for the caller now
    c2TitleId = pmtsTrack->c2Id();
    c2ColId = pmcolTrack->c2Id();

    //
    //  Create the non-case sensitive version of the category name, and
    //  see if it exists. If not, add it.
    //
    strTmp = strGenre;
    strTmp.ToUpper();
    const TMediaCat* pmcatCur = mdbTar.pmcatByUniqueId(m_eMediaType, strTmp, kCIDLib::False);
    if (!pmcatCur)
    {
        TMediaCat* pmcatNew = new TMediaCat(m_eMediaType, strGenre, strTmp);
        mdbTar.c2AddCategory(pmcatNew);
        pmcatCur = pmcatNew;
    }

    // If we haven't set the current cateogry on the collection, do so
    mdbTar.AddColToCat(pmcolTrack->c2Id(), pmcatCur->c2Id(), m_eMediaType);

    // If art was available, set that
    if (pmimgArt)
    {
        mdbTar.SetArtId
        (
            pmcolTrack->c2Id()
            , m_eMediaType
            , kCIDLib::False
            , pmimgArt->c2Id()
        );
    }

    //
    //  See if we already have this item in the list. If so, then we are being called
    //  for an update. Else it's a new one.
    //
    const TMediaItem* pmitemTrack = mdbTar.pmitemByUniqueId
    (
        m_eMediaType, strTrackUID, kCIDLib::False
    );

    //
    //  If it's already present, then see if we are moving it to a new collection.
    //  Also clear any categories, so that the code below will add just the one new
    //  category back. If we are moving it, we fake the new item flag back on, so
    //  that it's added back in below the same whether it already existed or not.
    //
    //  Else, it's a new one so create it and add it to the database.
    //
    tCIDLib::TBoolean bNewItem = kCIDLib::False;
    if (pmitemTrack)
    {
        // Clear any existing category, we'll add back below
        mdbTar.ClearColCats(pmcolTrack->c2Id(), m_eMediaType);

        // Find any existing owning containers for this item
        const TMediaTitleSet* pmtsOld = 0;
        const TMediaCollect*  pmcolOld = 0;
        tCIDLib::TCard2 c2ColInd, c2ItemInd;
        const tCIDLib::TBoolean bFoundOld = mdbTar.bFindItemContainer
        (
            pmitemTrack->eType()
            , pmitemTrack->c2Id()
            , c2ColInd
            , c2ItemInd
            , pmtsOld
            , pmcolOld
        );

        //
        //  See if the new set has the same unique id as the old one. If not, then
        //  this is obviously a case of an item being moved to a new
        //  collection, so move it.
        //
        //  If that leaves the old collection empty, then remove it and its parent
        //  set. Otherwise, let them both re-finalize.
        //
        //
        //  BE SURE to move the item to the new collection first, else we will trash
        //  it when we delete the old one.
        //
        if (bFoundOld && (pmtsTrack->strUniqueId() != pmtsOld->strUniqueId()))
        {
            if (bFromEvent)
            {
                QLogMsg
                (
                    L"Removing track '%(1)' from title '%(2)'"
                    , pmitemTrack->strName()
                    , pmtsOld->strName()
                );
            }

            mdbTar.RemoveFromCol(*pmcolOld, pmitemTrack->c2Id());
            if (!pmcolOld->c4ItemCount())
            {
                mdbTar.PruneHierarchy
                (
                    m_eMediaType, kCIDLib::True, pmtsOld->c2Id()
                );

                // Update the title count
                m_c4TitleCnt = mdbTar.c4TitleSetCnt(m_eMediaType);
            }
             else
            {
                // Refinalize the old title set and its collections if from an event
                if (bFromEvent)
                {
                    mdbTar.FinalizeObjects
                    (
                        pmtsOld->c2Id()
                        , m_eMediaType
                        , tCQCMedia::EDataTypes::TitleSet
                        , kCIDLib::True
                    );
                }
            }

            if (bFromEvent)
            {
                QLogMsg
                (
                    L"Moving track '%(1)' to title '%(2)'"
                    , pmitemTrack->strName()
                    , pmtsTrack->strName()
                );
            }

            //
            //  Now act like it's a new item, so that it will get added
            //  back to the new collection below at the right place.
            //
            bNewItem = kCIDLib::True;
        }
         else
        {
            //
            //  See if we already have this guy in the target collection. If not,
            //  then this is an existing item being added to a playlist, so force
            //  new item on so it'll get inserted below.
            //
            tCIDLib::TCard2 c2ItemInd;
            if (!pmcolTrack->bContainsItem(pmitemTrack->c2Id(), c2ItemInd))
                bNewItem = kCIDLib::True;
        }
    }
     else
    {
        bNewItem = kCIDLib::True;
        TMediaItem* pmitemNew = new TMediaItem
        (
            strTrackName, strTrackUID, strLocInfo, m_eMediaType
        );
        mdbTar.c2AddItem(pmitemNew);

        if (bFromEvent)
        {
            QLogMsg
            (
                L"Added new track '%(1)' to title '%(2)'"
                , pmitemNew->strName()
                , pmtsTrack->strName()
            );
        }
        pmitemTrack = pmitemNew;
    }

    // And we cna set up the caller's item id now
    c2ItemId = pmitemTrack->c2Id();

    //
    //  We need to update the track data now. To do this, we ahve to get a copy,
    //  which we can update and then put back. That's sort of redundant if we just
    //  created it above, but we may not have created it, so this is the cleanest
    //  way to do it.
    //
    TMediaItem mitemUpdate(*pmitemTrack);

    //
    //  Store the full id that will let us map back to iTunes. We want to
    //  do this whether it's a new one or an existing one since, if it was
    //  moved to a new collection, this id will change. The main unique id
    //  will not since it's based on the database wide track id.
    //
    mitemUpdate.strAltId(strUID);

    // Since we could get them out of order, set the track number
    mitemUpdate.c4LoadOrder(c4TrackNum);

    //
    //  Store the art id of the collection at the item level as well, so that
    //  they can be displayed when being accessed via a playlist.
    //
    mitemUpdate.c2ArtId(pmcolTrack->c2ArtId());

    //
    //  Store the other info now. Some of this is just stored for
    //  later aggregation at the collection/set level when we are
    //  done loading.
    //
    mitemUpdate.c4BitRate(c4BitRate);
    mitemUpdate.c4Duration(c4Duration);
    mitemUpdate.c4SampleRate(c4SampleRate);
    mitemUpdate.c4Rating(c4Rating);
    mitemUpdate.c4Year(c4Year);
    mitemUpdate.strArtist(strArtist);
    mitemUpdate.strName(strTrackName);
    mitemUpdate.strLocInfo(strLocInfo);

    // Put the updated item contents back now
    mdbTar.UpdateItem(mitemUpdate);

    //
    //  If we created a new set or new item, then figure out where to add
    //  the track in the collection. They are not necessarliy in order.
    //
    if (bNewSet || bNewItem)
    {
        const tCIDLib::TCard4 c4ItemCnt = pmcolTrack->c4ItemCount();
        tCIDLib::TCard4 c4Index = 0;
        for (; c4Index < c4ItemCnt; c4Index++)
        {
            //
            //  Break out when we see an existing track with a larger
            //  track number. Since we are adding them in order, this
            //  will keep the newly added ones sorted.
            //
            const TMediaItem* pmitemCur = mdbTar.pmitemById
            (
                m_eMediaType, pmcolTrack->c2ItemIdAt(c4Index), kCIDLib::False
            );
            if (pmitemCur && (pmitemCur->c4LoadOrder() > c4TrackNum))
                break;
        }

        // Add it to the collection at the appropriate position
        if (c4Index == c4ItemCnt)
            c4Index = kCIDLib::c4MaxCard;
        mdbTar.AddColItem(pmcolTrack->c2Id(), m_eMediaType, mitemUpdate, c4Index);
    }

    //
    //  If this is from an event, allow any owners of the item to refinalize, which
    //  may be more than just the direct owners. If not an event, then this is an
    //  initial load, so we don't want to do this.
    //
    if (bFromEvent)
        mdbTar.RefinalizeItemConts(mitemUpdate);

    // Indicate we created a new title set/collection or not
    return bNewSet;
}


//
//  After a load or change to the database, this is called to generate a new DB
//  serial number. In order to avoid 'churn', since the client service will re-
//  download the database when this changes, we set a flag any time changes are
//  made, and the poll method will check this flag every so often and call this
//  method to generate the new id.
//
//  The flag is only set in response to incoming change events, since those operate
//  on the current database. Reloads load into a separate DB and only update after
//  the load succeeds, and it just calls this method unconditionally.
//
tCIDLib::TVoid
TCQCTrayiTunesTab::CalcDBSerialNum(const TMediaDB& mdbSrc, TString& strToFill)
{
    TChunkedBinOutStream strmTar(32 * (1024 * 1024));
    strmTar << mdbSrc << kCIDLib::FlushIt;

    TChunkedBinInStream strmSrc(strmTar);
    facCQCMedia().CreatePersistentId
    (
        strmSrc, strmTar.c4CurSize(), strToFill
    );
}



//
//  This is called by the iTunes thread to try to get connected to iTunes.
//  We create the main app object, which causes iTunes to load if it is not
//  already.
//
tCIDLib::TVoid TCQCTrayiTunesTab::ConnectToiTunes()
{
    // Make sure any per-connect stuff is reset
    m_bQuitNotification = kCIDLib::False;
    m_bForceReload = kCIDLib::False;
    m_c4CurVolume = 0;
    m_ePlState = TiTPlayerIntfServerBase::EPlStates::None;
    m_enctCurPos = 0;
    m_enctCurTotal = 0;
    m_strCurAlbum.Clear();
    m_strCurArtist.Clear();
    m_strCurTrack.Clear();

    // Just for sanity's sake
    if (m_piTunes != 0)
        QLogMsg(L"The iTunes interface pointer is not zero at connect");

    m_piTunes = 0;
    HRESULT hRes = ::CoCreateInstance
    (
        CLSID_iTunesApp
        , NULL
        , CLSCTX_LOCAL_SERVER
        , IID_IiTunes
        , tCIDLib::pToVoidPP(&m_piTunes)
    );

    if (FAILED(hRes))
    {
        if (m_piTunes)
            QLogMsg(L"The iTunes pointer was set, though an error was returned");

        if (facCQCTrayMon.eVerboseLvl() >= tCQCKit::EVerboseLvls::Medium)
        {
            facCQCTrayMon.LogMsg
            (
                CID_FILE
                , CID_LINE
                , kTrayMonErrs::errciTunes_ConnFailed
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::LostConnection
                , TCardinal(hRes, tCIDLib::ERadices::Hex)
            );
        }
        return;
    }

    if (facCQCTrayMon.eVerboseLvl() >= tCQCKit::EVerboseLvls::Medium)
    {
        facCQCTrayMon.LogMsg
        (
            CID_FILE
            , CID_LINE
            , kTrayMonMsgs::midStatus_iT_Connected
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
    }

    //
    //  Wire ourself up for notifications from iTunes. For whatever reason, if we don't
    //  pumps some messages here, this will not work, though it will appear to.
    //
    m_piTunes->Stop();
    tCIDLib::TBoolean bNotOK = kCIDLib::False;
    IConnectionPoint* pCP = 0;
    IConnectionPointContainer* pCPC = 0;
    if (SUCCEEDED(m_piTunes->QueryInterface(IID_IConnectionPointContainer, tCIDLib::pToVoidPP(&pCPC))))
    {
        if (SUCCEEDED(pCPC->FindConnectionPoint(DIID__IiTunesEvents, &pCP)))
        {
            m_piTunesEv = new TITEventHandler(this);
            if (SUCCEEDED(pCP->Advise((IUnknown*)m_piTunesEv, &m_c4EvAdviseId)))
                bNotOK = kCIDLib::True;
            pCP->Release();
        }
        pCPC->Release();
    }

    if (!bNotOK)
    {
        facCQCTrayMon.LogMsg
        (
            CID_FILE
            , CID_LINE
            , kTrayMonErrs::errciTunes_NotifyReg
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
        return;
    }

    // Query some initial info to get the player state stuff ready
    long lVolume;
    m_piTunes->get_SoundVolume(&lVolume);
    if (lVolume < 0)
        lVolume = 0;
    else if (lVolume > 100)
        lVolume = 100;
    m_c4CurVolume = tCIDLib::TCard4(lVolume);

    ITPlayerState PlState;
    m_piTunes->get_PlayerState(&PlState);
    if (PlState == ITPlayerStatePlaying)
        m_ePlState = TiTPlayerIntfServerBase::EPlStates::Playing;
    else if (PlState == ITPlayerStateStopped)
        m_ePlState = TiTPlayerIntfServerBase::EPlStates::Stopped;
    else
        m_ePlState = TiTPlayerIntfServerBase::EPlStates::None;

    VARIANT_BOOL vBool;
    m_piTunes->get_Mute(&vBool);
    m_bCurMute = (vBool == VARIANT_TRUE);

    // Set the status to loading
    StoreNewStatus(tCQCMedia::ELoadStatus::Loading);
}


//
//  Called when the bgn thread is exiting, or when it feels that something has gone
//  wrong, to force a recycle of the connection and load.
//
tCIDLib::TVoid TCQCTrayiTunesTab::Disconnect()
{
    // Clean up stuff if we have it. First remove event handler if set
    if (m_piTunes && m_c4EvAdviseId)
    {
        try
        {
            IConnectionPointContainer* pCPC = 0;
            if (SUCCEEDED(m_piTunes->QueryInterface(IID_IConnectionPointContainer
                                                    , tCIDLib::pToVoidPP(&pCPC))))
            {
                IConnectionPoint* pCP = 0;
                if (SUCCEEDED(pCPC->FindConnectionPoint(DIID__IiTunesEvents, &pCP)))
                {
                    pCP->Unadvise(m_c4EvAdviseId);
                    pCP->Release();
                }
                pCPC->Release();
            }
            m_c4EvAdviseId = 0;

            //
            //  If we created teh event listener, then release it. iTunes should ahve
            //  released it above when we unadvised.
            //
            if (m_piTunesEv)
                SafeCOMFree(m_piTunesEv);
        }

        catch(TError& errToCatch)
        {
            if ((facCQCTrayMon.eVerboseLvl() >= tCQCKit::EVerboseLvls::Medium)
            &&  !errToCatch.bLogged())
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }
        }

        catch(...)
        {
        }
    }

    // Release the iTunes app object if we got it
    if (m_piTunes)
        SafeCOMFree(m_piTunes);

    // Clean up everything
    m_bQuitNotification = kCIDLib::False;
    m_bForceReload = kCIDLib::False;
    m_c4CurVolume = 0;
    m_ePlState = TiTPlayerIntfServerBase::EPlStates::None;
    m_enctCurPos = 0;
    m_enctCurTotal = 0;
    m_strCurAlbum.Clear();
    m_strCurArtist.Clear();
    m_strCurTrack.Clear();

    // Be sure to go back to INIT state, so we'll start trying to connect again
    StoreNewStatus(tCQCMedia::ELoadStatus::Init);
}



//
//  This will get the playlist and source id from the passed object. This
//  let's us get the unique list ids for any object.
//
tCIDLib::TInt4
TCQCTrayiTunesTab::i4GetPLId(IITObject* const pPList, tCIDLib::TInt4& i4SrcId)
{
    tCIDLib::TInt4 i4Src, i4PL, i4Track, i4DB;
    if (SUCCEEDED(pPList->GetITObjectIDs(&i4Src, &i4PL, &i4Track, &i4DB)))
    {
        i4SrcId = i4Src;
        return i4PL;
    }

    i4SrcId = 0;
    return 0;
}



//
//  This is called to attempt to get the database loaded up. What we do is first go
//  through the main library list and create all of the real tracks, collections,
//  and sets.
//
//  Then we go back and do the playlists. For those, all we need to do is look up
//  existing tracks and add them to playlist set/collections.
//
//  We do all of this into the temporary m_mdbLoad database, and only swap it over
//  at the end if all goes well.
//
tCIDLib::TVoid TCQCTrayiTunesTab::LoadDB(TThread& thrLoader)
{
    HRESULT     hRes;

    //
    //  Get the source for the main library. If we can't get that, don't
    //  bother even trying to do anything else, and particularly don't
    //  reset the database so that, if we can't connect to COM we won't
    //  trash any existing content and end up with nothing.
    //
    IITSource* pMainLib;
    hRes = m_piTunes->get_LibrarySource(&pMainLib);
    if (!bCheckGetObj(hRes, L"main library source", CID_LINE))
        return;
    TCOMJanitor<IITSource> janMainLib(&pMainLib);

    // Create a directory for our cover art data if it's not there yet
    TFileSys::MakePath(m_pathArtDir);

    // Flush any outstanding change requests or commands
    m_colPlCmdQ.RemoveAll();
    m_colDBChanges.RemoveAll();

    //
    //  Reset our counts visible to the timer. The displayed values will change
    //  if these new values aren't equal to the last seen values from the
    //  previous load.
    //
    m_c4TracksFailed = 0;
    m_c4TracksIgnored = 0;
    m_c4TitleCnt = 0;

    // Flush the database for a new load
    m_mdbLoad.Reset();

    // Let's try to enumerate the main library's collection lists
    IITPlaylistCollection* pPLCol;
    hRes = pMainLib->get_Playlists(&pPLCol);
    if (!bCheckGetObj(hRes, L"main library PL collection", CID_LINE))
        return;
    TCOMJanitor<IITPlaylistCollection> janMLPCol(&pPLCol);

    // Set up the stuff we need to extra track info
    tCIDLib::TCard2 c2PListArtId(0);
    tCIDLib::TCard2 c2MissingArtId(0);
    THeapBuf        mbufTmp(2048, 0x20000);
    TPathStr        pathCoverArt;
    TString         strAllMusicCat(kMedMsgs::midCat_AllMusic, facCQCMedia());

    // Add some special categories, which have fixed ids and text
    TString strUID(m_strAllMusicCat);
    strUID.ToUpper();
    TMediaCat* pmcatNew = new TMediaCat(m_eMediaType, m_strAllMusicCat, strUID);
    pmcatNew->c2Id(kCQCMedia::c2CatId_AllMusic);
    m_mdbLoad.c2AddCategory(pmcatNew, kCIDLib::True);

    strUID = m_strPlayListCat;
    strUID.ToUpper();
    pmcatNew = new TMediaCat(m_eMediaType, m_strPlayListCat, strUID);
    pmcatNew->c2Id(kCQCMedia::c2CatId_PlayLists);
    m_mdbLoad.c2AddCategory(pmcatNew, kCIDLib::True);

    // Make sure we have at least one playlist, else nothing to do
    long iPLCount;
    if (FAILED(pPLCol->get_Count(&iPLCount)) || !iPLCount)
        return;

    //
    //  Now we do the main list, which is were we create all the items
    //  and their main owning (non-playlist) sets and collections.
    //
    {
        IITPlaylist* pPList;
        hRes = pPLCol->get_Item(1, &pPList);
        if (!bCheckGetObj(hRes, L"main library list", CID_LINE))
            return;
        TCOMJanitor<IITPlaylist> janMainList(&pPList);

        // Get the list of tracks in this list. If none, nothing to do
        IITTrackCollection* pTrList;
        hRes = pPList->get_Tracks(&pTrList);
        if (!bCheckGetObj(hRes, L"playlist", CID_LINE))
            return;
        TCOMJanitor<IITTrackCollection> janTracks(&pTrList);

        // Get the count of tracks in this list. If none, nothing to do
        long iTrCount;
        hRes = pTrList->get_Count(&iTrCount);
        if (!bCheckGetObj(hRes, L"playlist count", CID_LINE) || !iTrCount)
            return;

        // Save the list id of this main list
        m_i4MainListId = i4GetPLId(pPList, m_i4MainSrcId);

        // Iterate the tracks in this list
        for (long iIndex = 1; iIndex <= iTrCount; iIndex++)
        {
            // Watch for shutdown requests
            if (thrLoader.bCheckShutdownRequest())
                return;

            IITTrack* pTrack;
            hRes = pTrList->get_Item(iIndex, &pTrack);
            if (FAILED(hRes))
            {
                m_c4TracksFailed++;
                continue;
            }
            TCOMJanitor<IITTrack> janTrack(&pTrack);

            // Process this track
            tCIDLib::TCard2 c2TitleId, c2ColId, c2ItemId;
            if (bProcessTrack(m_mdbLoad, pTrack, c2TitleId, c2ColId, c2ItemId, kCIDLib::False))
            {
                //
                //  If we hit the maximum item id, then log this. We are going to
                //  ignore everything from here on out. We are guaranteed to hit
                //  this one first, unless we have 64K title sets which have one
                //  collection and one item. That's not going to happen.
                //
                if (c2ItemId == kCIDLib::c2MaxCard)
                {
                    if (facCQCTrayMon.eVerboseLvl() >= tCQCKit::EVerboseLvls::Medium)
                    {
                        facCQCTrayMon.LogMsg
                        (
                            CID_FILE
                            , CID_LINE
                            , kTrayMonErrs::errciTunes_MaxItems
                            , tCIDLib::ESeverities::Status
                            , tCIDLib::EErrClasses::Overflow
                        );
                    }
                    break;
                }
            }
        }
    }

    //
    //  Now go back and iterate all the playlists and set those up.
    //  Here we don't create any new items. We just add existing items
    //  to playlist sets/collections.
    //
    //  SO WE START at index 2, past the main list we already did.
    //
    TString strTrackUID;
    for (long iPLIndex = 2; iPLIndex <= iPLCount; iPLIndex++)
    {
        // Watch for shutdown requests
        if (thrLoader.bCheckShutdownRequest())
            break;

        // Get the current playlist
        IITPlaylist* pPList;
        hRes = pPLCol->get_Item(iPLIndex, &pPList);
        if (!bCheckGetObj(hRes, L"library PL", CID_LINE))
            return;
        TCOMJanitor<IITPlaylist> janMainList(&pPList);

        // Get the list of tracks in this list. If not, try again
        IITTrackCollection* pTrList;
        hRes = pPList->get_Tracks(&pTrList);
        if (!bCheckGetObj(hRes, L"PL track collection", CID_LINE))
            continue;;
        TCOMJanitor<IITTrackCollection> janTracks(&pTrList);

        // Get the count of tracks in this list. If not, or none, try next
        long iTrCount;
        hRes = pTrList->get_Count(&iTrCount);
        if (!bCheckGetObj(hRes, L"PL col count", CID_LINE) || !iTrCount)
            continue;

        // Not empty so try to add it. If it's not a keeper, skip it
        tCIDLib::TCard2 c2TitleId, c2ColId;
        if (!bProcessPlayList(m_mdbLoad, pPList, c2TitleId, c2ColId))
            continue;

        // Iterate the tracks in this list
        tCIDLib::TCard4 c4Keepers = 0;
        for (long iIndex = 1; iIndex <= iTrCount; iIndex++)
        {
            // Watch for shutdown requests
            if (thrLoader.bCheckShutdownRequest())
                return;

            IITTrack* pTrack;
            hRes = pTrList->get_Item(iIndex, &pTrack);
            if (FAILED(hRes))
            {
                m_c4TracksFailed++;
                continue;
            }

            // Get the track UID for this track
            if (bGetObjectId(pTrack, m_strThreadTmp, strTrackUID))
            {
                const TMediaItem* pmitemCur = m_mdbLoad.pmitemByUniqueId
                (
                    m_eMediaType, strTrackUID, kCIDLib::False
                );

                // If we found it, add it to the new playlist
                if (pmitemCur)
                {
                    m_mdbLoad.AddColItem
                    (
                        c2ColId, m_eMediaType, *pmitemCur, kCIDLib::c4MaxCard
                    );
                    c4Keepers++;
                }
            }
        }

        //
        //  If we never found any tracks that were valid, i.e. the playlist
        //  is empty, then just remove the new title set and collection.
        //
        if (!c4Keepers)
        {
            m_mdbLoad.bRemoveObject
            (
                m_eMediaType, tCQCMedia::EDataTypes::TitleSet, c2TitleId
            );

            // Update the title count
            m_c4TitleCnt = m_mdbLoad.c4TitleSetCnt(m_eMediaType);
        }
    }

    // Complete the loading process on the temporary database
    m_mdbLoad.LoadComplete();

    //
    //  If they want to delete images, then we need to find all of the
    //  image files that are not referenced and delete them.
    //
    if (m_bDelImages)
        CleanupArt(m_mdbLoad);

    //
    //  We have to generate a database serial number. So we just flatten out the
    //  database and hash the buffer. A bit piggy but the only way to do it that
    //  insures it only changes if the data we actually care about changes.
    //
    TString strSerNum;
    CalcDBSerialNum(m_mdbLoad, strSerNum);

    //
    //  And let's move the data to the real database and we have to update the
    //  database serial number. We have to lock while doing these things.
    //
    {
        TLocker lockrSync(&m_mtxSync);
        m_strDBSerialNum = strSerNum;
        m_srdbEng.ResetMediaDB(m_mdbLoad);

        // Set the status to ready status
        StoreNewStatus(tCQCMedia::ELoadStatus::Ready);
    }

    QLogMsg(L"Database loaded");
}


//
//  This is called for each track of a given collection until we finally
//  find one that has cover art. We force the image out to a path that
//  will then be stored in the media image item for later faulting in.
//
const TMediaImg*
TCQCTrayiTunesTab::pmimgStoreArt(       TMediaDB&       mdbTar
                                ,       IITTrack* const pTrack
                                , const TString&        strAlbumUID)
{
    // Assume it will fail until proven otherwise
    const TMediaImg* pmimgRet = 0;

    //
    //  Get the artwork collection for this track. If we can't just return
    //  without loading any art.
    //
    IITArtworkCollection* pArtCol = 0;
    if (FAILED(pTrack->get_Artwork(&pArtCol)) || !pArtCol)
        return 0;
    TCOMJanitor<IITArtworkCollection> janCol(&pArtCol);

    //
    //  Get the first artwork object. We can't really get any info about
    //  the image to try to select the best one, so we just take the first
    //  one.
    //
    IITArtwork* pArtwork = 0;
    if (FAILED(pArtCol->get_Item(1, &pArtwork)) || !pArtwork)
        return 0;
    TCOMJanitor<IITArtwork> janArtwork(&pArtwork);

    //
    //  Get the type so we can build up the full path. If we can't get it,
    //  or it's of a type we don't comprehend, then skip it.
    //
    ITArtworkFormat ArtFmt;
    if (FAILED(pArtwork->get_Format(&ArtFmt)))
        return 0;

    TPathStr pathCoverArt = m_pathArtDir;
    pathCoverArt.AddLevel(strAlbumUID);

    if (ArtFmt == ITArtworkFormatJPEG)
        pathCoverArt.AppendExt(L"jpg");
    else if (ArtFmt == ITArtworkFormatPNG)
        pathCoverArt.AppendExt(L"png");
    else if (ArtFmt == ITArtworkFormatBMP)
        pathCoverArt.AppendExt(L"bmp");
    else
        return 0;

    //
    //  See if the file already exists. If so, then get the actual path out to use,
    //  to insure that we have the same case exactly as what it was stored as, since
    //  we may later need to do an art cleanup pass and this means we'll be able
    //  to use a hash set since the case will be the same as what we find during a
    //  file search.
    //
    TFindBuf fndbSearch;
    if (TFileSys::bExists(pathCoverArt, fndbSearch))
        pathCoverArt = fndbSearch.pathFileName();

    // See if we already have this one
    pmimgRet = mdbTar.pmimgByUniqueId(m_eMediaType, strAlbumUID, kCIDLib::False);
    if (!pmimgRet)
    {
        // If we can't force it to disk, then we failed
        TBSTRJanitor janPath(pathCoverArt.pszBuffer());
        if (FAILED(pArtwork->SaveArtworkToFile(janPath.bsData)))
            return 0;

        // If it's a BMP or PNG, convert to JPEG to save space
        TString strPerId;
        if ((ArtFmt == ITArtworkFormatPNG)
        ||  (ArtFmt == ITArtworkFormatBMP))
        {
            const TString strOrg = pathCoverArt;

            pathCoverArt = m_pathArtDir;
            pathCoverArt.AddLevel(strAlbumUID);
            pathCoverArt.AppendExt(L"jpg");

            //
            //  Since we have to generate a persistent id, we read the file into
            //  memory and stream from that. We can hash that buffer to create the
            //  persistent id.
            //
            try
            {
                TPNGImage imgOut;
                {
                    TBinFileInStream strmSrc
                    (
                        strOrg
                        , tCIDLib::ECreateActs::OpenIfExists
                        , tCIDLib::EFilePerms::Default
                        , tCIDLib::EFileFlags::SequentialScan
                    );

                    // Read it into the type it is and assign to a PNG
                    if (ArtFmt == ITArtworkFormatPNG)
                    {
                        TPNGImage imgPNG;
                        strmSrc >> imgPNG;
                        imgOut = imgPNG;
                    }
                     else if (ArtFmt == ITArtworkFormatBMP)
                    {
                        TBitmapImage imgBMP;
                        strmSrc >> imgBMP;
                        imgOut = imgBMP;
                    }
                }

                // Write it back out to the new path
                {
                    TBinFileOutStream strmTar
                    (
                        pathCoverArt
                        , tCIDLib::ECreateActs::CreateAlways
                        , tCIDLib::EFilePerms::Default
                        , tCIDLib::EFileFlags::SequentialScan
                    );
                    strmTar << imgOut << kCIDLib::FlushIt;
                }

                // Delete the original
                TFileSys::DeleteFile(strOrg);
            }

            catch(TError& errToCatch)
            {
                if (facCQCTrayMon.eVerboseLvl() >= tCQCKit::EVerboseLvls::Medium)
                {
                    errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                    TModule::LogEventObj(errToCatch);
                    QLogMsg
                    (
                        L"Failed to load cover art file. Path=%(1)", pathCoverArt
                    );
                }
                return 0;
            }
        }

        //
        //  It either exists already or we converted it. So let's generate the
        //  persistent id.
        //
        {
            TBinaryFile flSrc(pathCoverArt);
            flSrc.Open
            (
                tCIDLib::EAccessModes::Read
                , tCIDLib::ECreateActs::OpenIfExists
                , tCIDLib::EFilePerms::Default
                , tCIDLib::EFileFlags::SequentialScan
            );

            // Read up to the max id bytes
            const tCIDLib::TCard4 c4Bytes = flSrc.c4ReadBuffer
            (
                m_mbufArtLoad
                , kCQCTrayMon::c4MaxImgPerIdBytes
                , tCIDLib::EAllData::OkIfNotAll
            );
            facCQCMedia().CreatePersistentId(m_mbufArtLoad, c4Bytes, strPerId);
        }

        TMediaImg* pmimgNew = new TMediaImg
        (
            m_eMediaType, pathCoverArt, TString::strEmpty()
        );
        pmimgNew->strUniqueId(strAlbumUID);

        //
        //  Note we have to store it on both large and small, since we generate the
        //  small from the large.
        //
        pmimgNew->SetPersistentId(strPerId, tCQCMedia::ERArtTypes::LrgCover);
        pmimgNew->SetPersistentId(strPerId, tCQCMedia::ERArtTypes::SmlCover);
        mdbTar.c2AddImage(pmimgNew);

        pmimgRet = pmimgNew;
    }

    // We either found or created it
    return pmimgRet;
}


//
//  Called to handle database change/delete events. There's a fair bit of work
//  involved with this. We have to get the database updated to reflect the reported
//  changes.
//
//  See the class header for comments on how we structure the database vis a vis
//  iTunes ids in order to make things easier for us.
//
//  Note that we are always working on the real database here, since this only
//  happens when changes are made to the iTunes data, not during our loading process.
//
tCIDLib::TVoid TCQCTrayiTunesTab::ProcessEvent(const TDBQChangeInfo& dbciEv)
{
    // See what type of object is being referenced
    const tCQCTrayMon::EiTunesObjs eType = dbciEv.eObjType();

    //
    //  Format out the unique id of the object. The track id is what we use
    //  for individual items. Everything else uses the full id.
    //
    TString strTrackId;
    TString strUniqueId;
    FormatObjUID(dbciEv, strUniqueId, strTrackId);

    // If not a delete event, see if we can find the object. If not, do nothing
    IITObject* pObject = 0;
    if (!dbciEv.m_bDelete)
    {
        m_piTunes->GetITObjectByID
        (
            dbciEv.m_i4Src
            , dbciEv.m_i4PlayList
            , dbciEv.m_i4Track
            , dbciEv.m_i4Database
            , &pObject
        );

        if (!pObject)
            return;
    }

    // In this case, we are always operating on the real database
    TMediaDB& mdbInfo = m_srdbEng.mdbInfo();

    //
    //  Put a janitor on it. If we didn't get it, because we are doing a
    //  delete operation, it is null so this won't hurt anything.
    //
    TCOMJanitor<IITObject> janTrack(&pObject);

    if (eType == tCQCTrayMon::EiTunesObjs::Track)
    {
        //
        //  It's a track, so see if we can find it in our database. Use the
        //  track id, which is how we store individual items.
        //
        const TMediaItem* pmitemTar = mdbInfo.pmitemByUniqueId
        (
            m_eMediaType, strTrackId, kCIDLib::False
        );

        if (pmitemTar)
        {
            if (dbciEv.m_bDelete)
            {
                //
                //  If the containing playlist is the main list, then we need to
                //  delete he actual object. Otherwise, we just want to remove it
                //  from the referenced playlist.
                //
                //  If the main list, and this leaves the collect empty, we remove
                //  the collection and title, since it doesn't exist anymore.
                //  These are only pseudo entities to iTunes and go away when
                //  the contained items go away.
                //
                //  If a playlist, we just remove the item from the PL. PLs are
                //  real entities to iTunes and we will get an explicit delete
                //  if one of those is removed.
                //
                if ((dbciEv.m_i4Src == m_i4MainSrcId)
                &&  (dbciEv.m_i4PlayList == m_i4MainListId))
                {
                    QLogMsg
                    (
                        L"Removing track '%(1)'. TrackID=%(2)"
                        , pmitemTar->strName()
                        , TCardinal(pmitemTar->c2Id())
                    );

                    // Find the owning containers before we remove it
                    const tCIDLib::TCard2 c2TrackId = pmitemTar->c2Id();
                    const TMediaCollect*  pmcolPar = 0;
                    const TMediaTitleSet* pmtsPar = 0;
                    tCIDLib::TCard2 c2ColInd, c2ItemInd;
                    const tCIDLib::TBoolean bFound = mdbInfo.bFindItemContainer
                    (
                        m_eMediaType, c2TrackId, c2ColInd, c2ItemInd, pmtsPar, pmcolPar
                    );

                    // Now remove the track from the DB
                    if (mdbInfo.bRemoveObject(m_eMediaType, tCQCMedia::EDataTypes::Item, c2TrackId))
                        m_bDBChanges = kCIDLib::True;

                    // If we found a parent container, then see if it's empty now
                    if (bFound)
                    {
                        //
                        //  If this left the owning collection empty, then remove the
                        //  overall parent set.
                        //
                        if (!pmcolPar->c4ItemCount())
                        {
                            QLogMsg(L"Removing empty title '%(1)'", pmtsPar->strName());
                            mdbInfo.PruneHierarchy
                            (
                                m_eMediaType, kCIDLib::True, pmtsPar->c2Id()
                            );

                            // Update the title count
                            m_c4TitleCnt = mdbInfo.c4TitleSetCnt(m_eMediaType);
                            m_bDBChanges = kCIDLib::True;
                        }
                    }
                }
                 else
                {
                    //
                    //  Not the main list, so it's a playlist, so we can look that
                    //  up from the passed id info.
                    //
                    //  Format the id of the list its in and look it up.
                    //
                    FormatListUID(dbciEv, m_strThreadTmp);
                    const TMediaTitleSet* pmtsTar = mdbInfo.pmtsByUniqueId
                    (
                        m_eMediaType, m_strThreadTmp, kCIDLib::False
                    );

                    //
                    //  If we found it, and it has any collections, then let's process
                    //  this event.
                    //
                    if (pmtsTar && pmtsTar->c4ColCount())
                    {
                        QLogMsg
                        (
                            L"Removing track '%(1)' from title '%(2)'"
                            , pmitemTar->strName()
                            , pmtsTar->strName()
                        );

                        // Get the first (only) collection
                        const TMediaCollect& mcolTar = pmtsTar->mcolAt(mdbInfo, 0);
                        mdbInfo.RemoveFromCol(mcolTar, pmitemTar->c2Id());

                        m_bDBChanges = kCIDLib::True;
                    }
                }
            }
             else
            {
                //
                //  If we are seeing this item via the main list, thenprocess it.
                //  This will either just update the item in place, or potentialy
                //  move it to a new set/collection if the artist/album names were
                //  changed.
                //
                //  Else, we need to add this item to the indicated playlist if it
                //  is not already there.
                //
                if ((dbciEv.m_i4Src == m_i4MainSrcId)
                &&  (dbciEv.m_i4PlayList == m_i4MainListId))
                {
                    IITTrack* pTrack = 0;
                    if (SUCCEEDED(pObject->QueryInterface(__uuidof(pTrack), tCIDLib::pToVoidPP(&pTrack))))
                    {
                        TCOMJanitor<IITTrack> janTrack(&pTrack);

                        tCIDLib::TCard2 c2TitleId;
                        tCIDLib::TCard2 c2ColId;
                        tCIDLib::TCard2 c2ItemId;
                        bProcessTrack
                        (
                            mdbInfo
                            , pTrack
                            , c2TitleId
                            , c2ColId
                            , c2ItemId
                            , kCIDLib::True
                        );

                        mdbInfo.RefinalizeItemConts(*pmitemTar);
                        m_bDBChanges = kCIDLib::True;
                    }
                }
                 else
                {
                    // Look up the playlist that it's in
                    FormatListUID(dbciEv, m_strThreadTmp);
                    const TMediaTitleSet* pmtsTar = mdbInfo.pmtsByUniqueId
                    (
                        m_eMediaType, m_strThreadTmp, kCIDLib::False
                    );

                    if (pmtsTar
                    &&  mdbInfo.bAddItemToPL(pmtsTar->c2ColIdAt(0)
                                            , m_eMediaType
                                            , pmitemTar->c2Id()))
                    {
                        m_bDBChanges = kCIDLib::True;
                        QLogMsg
                        (
                            L"Adding track '%(1)' to list '%(2)'"
                            , pmitemTar->strName()
                            , pmtsTar->strName()
                        );
                    }
                }
            }
        }
         else if (!dbciEv.m_bDelete)
        {
            //
            //  It's not a delete, and it's for a track we don't know about. If we
            //  are dealing with the main list, then we need to create a new item
            //  (and possibly containing col/set for it.) Else, there's not much we
            //  can do.
            //
            IITTrack* pTrack = 0;
            if (SUCCEEDED(pObject->QueryInterface(__uuidof(pTrack), tCIDLib::pToVoidPP(&pTrack))))
            {
                TCOMJanitor<IITTrack> janTrack(&pTrack);

                if ((dbciEv.m_i4Src == m_i4MainSrcId)
                &&  (dbciEv.m_i4PlayList == m_i4MainListId))
                {
                    tCIDLib::TCard2 c2TitleId;
                    tCIDLib::TCard2 c2ColId;
                    tCIDLib::TCard2 c2ItemId;
                    bProcessTrack
                    (
                        mdbInfo
                        , pTrack
                        , c2TitleId
                        , c2ColId
                        , c2ItemId
                        , kCIDLib::True
                    );

                    // Let any containers that reference this item update
                    if (c2ItemId != kCIDLib::c4MaxCard)
                    {
                        pmitemTar = mdbInfo.pmitemById(m_eMediaType, c2ItemId, kCIDLib::True);

                        // Let any containers that reference this item update
                        mdbInfo.RefinalizeItemConts(*pmitemTar);
                        m_bDBChanges = kCIDLib::True;
                    }
                }
            }
        }
    }
     else if (eType == tCQCTrayMon::EiTunesObjs::PlayList)
    {
        // It's a track, so see if we can find it in our database
        FormatListUID(dbciEv, m_strThreadTmp);
        const TMediaTitleSet* pmtsTar = mdbInfo.pmtsByUniqueId
        (
            m_eMediaType, m_strThreadTmp, kCIDLib::False
        );

        if (pmtsTar)
        {
            if (dbciEv.m_bDelete)
            {
                //
                //  Note that, in this case, there's no need to do any
                //  updating of the database other than the removal.
                //  Since we only have a flat structure in this repo, a
                //  single title set holds a single collection, so we can
                //  just delete the title set.
                //
                QLogMsg(L"Removing title '%(1)'", pmtsTar->strName());
                mdbInfo.PruneHierarchy
                (
                    m_eMediaType, kCIDLib::True, pmtsTar->c2Id()
                );

                // Update the title count
                m_c4TitleCnt = mdbInfo.c4TitleSetCnt(m_eMediaType);

                m_bDBChanges = kCIDLib::True;
            }
             else
            {
                //
                //  The only ting we really care about here is maybe the
                //  name of the playlist having changed. In terms of
                //  playlist contents, we will update that based on item
                //  level changes reported above.
                //
                BSTR bsName;
                pObject->get_Name(&bsName);
                m_strThreadTmp = bsName;
                ::SysFreeString(bsName);
                if (pmtsTar->strName() != m_strThreadTmp)
                {
                    QLogMsg(L"Renaming title '%(1)'", pmtsTar->strName());
                    mdbInfo.RenameObject
                    (
                        m_eMediaType
                        , tCQCMedia::EDataTypes::TitleSet
                        , pmtsTar->c2Id()
                        , m_strThreadTmp
                    );
                    m_bDBChanges = kCIDLib::True;
                }
            }
        }
         else if (!dbciEv.m_bDelete)
        {
            // Presumably we are creating a new playlist
            IITPlaylist* pPList = 0;
            if (SUCCEEDED(pObject->QueryInterface(__uuidof(pPList), tCIDLib::pToVoidPP(&pPList))))
            {
                TCOMJanitor<IITPlaylist> janTrack(&pPList);

                tCIDLib::TCard2 c2TitleId;
                tCIDLib::TCard2 c2ColId;
                if (bProcessPlayList(mdbInfo, pPList, c2TitleId, c2ColId))
                    m_bDBChanges = kCIDLib::True;
            }
        }
    }
}


//
//  We need to update the content of the indicated title set with thepassed rating
//  (in our 0 to 10 format.) Since iTunes only supports rating on tracks, we just
//  apply to each item of each collection.
//
tCIDLib::TVoid
TCQCTrayiTunesTab::SetUserRating(const  TMediaTitleSet& mtsTar
                                , const tCIDLib::TCard4 c4Rating)
{
    tCIDLib::TInt4  i4Src;
    tCIDLib::TInt4  i4PL;
    tCIDLib::TInt4  i4Track;
    tCIDLib::TInt4  i4DB;

    // Convert to iTunes rating format
    tCIDLib::TInt4 i4Rating = tCIDLib::TInt4(c4Rating * 10);
    if (i4Rating > 100)
        i4Rating = 100;
    else if (i4Rating < 0)
        i4Rating = 0;

    // Iterate the collections in the passed target
    TMediaDB& mdbInfo = m_srdbEng.mdbInfo();
    const tCIDLib::TCard4 c4CCount = mtsTar.c4ColCount();
    for (tCIDLib::TCard4 c4CIndex = 0; c4CIndex < c4CCount; c4CIndex++)
    {
        const TMediaCollect& mcolCur = mtsTar.mcolAt(mdbInfo, c4CIndex);

        // Iterate the items of this collection
        const tCIDLib::TCard4 c4ICount = mcolCur.c4ItemCount();
        for (tCIDLib::TCard4 c4IIndex = 0; c4IIndex < c4ICount; c4IIndex++)
        {
            const TMediaItem& mitemCur = mcolCur.mitemAt(mdbInfo, c4IIndex);

            //
            //  Find the track object for this guy. We have to parse
            //  out the values that we stored in the alt id value of
            //  the item.
            //
            if (!bParseObjUID(mitemCur.strAltId(), i4Src, i4PL, i4Track, i4DB))
                continue;

            IITObject* pObject = 0;
            m_piTunes->GetITObjectByID(i4Src, i4PL, i4Track, i4DB, &pObject);
            if (!pObject)
                continue;
            TCOMJanitor<IITObject> janObject(&pObject);

            // We have to get it as a track
            IITTrack* pTrack = 0;
            if (FAILED(pObject->QueryInterface(__uuidof(pTrack), tCIDLib::pToVoidPP(&pTrack))))
                continue;
            TCOMJanitor<IITTrack> janTrack(&pTrack);

            pTrack->put_Rating(i4Rating);
        }
    }

    m_bDBChanges = kCIDLib::True;
}


//
//  This is called if we are in playback state and there is a current track duration
//  stored. We query the position within the playing track and store it for later query
//  by the driver.
//
//  The caller already locked.
//
tCIDLib::TVoid TCQCTrayiTunesTab::StoreCurPos()
{
    //
    //  Make sure the current track length value isn't zero. We shouldn't
    //  get called if so, but just in case.
    //
    tCIDLib::TEncodedTime enctNewCur = 0;
    if (m_enctCurTotal)
    {
        // We have a duration so do it
        long iPos;
        HRESULT hRes = m_piTunes->get_PlayerPosition(&iPos);

        // If no track, then it may have ended in the meantime
        if (FAILED(hRes))
        {
            m_enctCurPos = 0;
            m_enctCurTotal = 0;
            return;
        }

        // Convert to our standard time stamp format
        enctNewCur = tCIDLib::TEncodedTime(iPos) * kCIDLib::enctOneSecond;

        // Make sure it remains validly less than the total
        if (enctNewCur > m_enctCurTotal)
            enctNewCur = m_enctCurTotal;
    }

    //
    //  If it's changed more than half a second since last time, store it and bump the
    //  serial number.
    //
    if (enctNewCur != m_enctCurPos)
    {
        tCIDLib::TInt8 i8Diff(tCIDLib::TInt8(enctNewCur) - tCIDLib::TInt8(m_enctCurPos));
        if (i8Diff < 0)
            i8Diff *= -1;

        if (i8Diff > tCIDLib::TInt8(kCIDLib::enctHalfSecond))
        {
            m_enctCurPos = enctNewCur;

            #if CID_DEBUG_ON
            TAudio::Beep(50, 880);
            #endif

            m_c4PlSerialNum++;
            if (!m_c4PlSerialNum)
                m_c4PlSerialNum++;
        }
    }
}

