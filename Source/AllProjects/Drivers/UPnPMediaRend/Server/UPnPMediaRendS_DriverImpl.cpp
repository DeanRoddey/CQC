//
// FILE NAME: UPnPMediaRendS_DriverImpl.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/18/2011
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
//  This is the main implementation file of the driver.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "UPnPMediaRendS.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TUPnPMediaRendSDriver,TCQCStdMediaRendDrv)


namespace UPnPMediaRendS_DriverImpl
{
    // -----------------------------------------------------------------------
    //  If any UPnP media renderer drivers get started, then we need to create
    //  an async finder that they can use to watch for their configured devices.
    //
    //  We use the pointer itself as a lazy fault in flag. We don't try to
    //  keep up with the unload of drivers and reference count this object.
    //  Once created it'll just live until the CQCServer process dies.
    //
    //
    //  !! NOTE:
    //
    //  Since the finder's methods for checking the list are already
    //  synchronized (because it has to sync with a background thread that
    //  updates the list), we can call them from multiple drivers and not
    //  cause issues. Since the object is never destroyed, all we have to do
    //  is synchronize the creation of the finder.
    //
    //  This way we don't have to have a separate one per driver, each with
    //  it's own thread and behind the scenes overhead.
    // -----------------------------------------------------------------------
    TUPnPAsyncFinder*       pupnpafRend = 0;


    // -----------------------------------------------------------------------
    //  The names of our own fields
    // -----------------------------------------------------------------------
    const tCIDLib::TCh* const pszFldSetLocInfoPat(L"SetLocInfoPattern");
}


// ---------------------------------------------------------------------------
//   CLASS: UPnPMediaRendSDriver
//  PREFIX: drv
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  UPnPMediaRendSDriver: Constructors and Destructor
// ---------------------------------------------------------------------------
TUPnPMediaRendSDriver::TUPnPMediaRendSDriver(const TCQCDriverObjCfg& cqcdcToLoad) :

    TCQCStdMediaRendDrv
    (
        cqcdcToLoad
        , tCQCMedia::EPLModes::Jukebox
        , kCQCMedia::c4RendFlag_None
        , tCQCMedia::EMTFlags::MultiMedia
    )
    , m_bDoDBVolume(kCIDLib::False)
    , m_bFlipSeps(kCIDLib::False)
    , m_bMute(kCIDLib::False)
    , m_bRemoveUNCSlashes(kCIDLib::False)
    , m_c4FldId_SetLocInfoPat(kCIDLib::c4MaxCard)
    , m_c4Volume(0)
    , m_enctCurPos(0)
    , m_enctCurTotal(0)
    , m_enctLastPosQ(0)
    , m_enctLastTransQ(0)
    , m_enctWaitStart(0)
    , m_eEndState(tCQCMedia::EEndStates::Ended)
    , m_eState(tCQCMedia::EMediaStates::Stopped)
    , m_i4MaxVol(1)
    , m_i4MinVol(0)
{
}

TUPnPMediaRendSDriver::~TUPnPMediaRendSDriver()
{
}

// ---------------------------------------------------------------------------
//  TUPnPMediaRendSDriver: Protected, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TUPnPMediaRendSDriver::bGetCommResource(TThread&)
{
    try
    {
        //
        //  See if our device is available. This avoids kicking off a long wait that
        //  would happen if we just blindly tried to create our device and it wasn't
        //  availble, because it would do a synchronous
        //  search.
        //
        //  We don't bother waiting for the list to be complete. If our device shows
        //  up in the list early, then we get going quicker. If not, then then this
        //  will just return false.
        //
        if (!UPnPMediaRendS_DriverImpl::pupnpafRend->bDevIsAvailable(m_strDevUID))
        {
            if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
            {
                facUPnPMediaRendS().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kUPnPMRErrs::errcProto_DevNotFound
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::NotFound
                    , m_strDevUID
                );
            }
            return kCIDLib::False;
        }

        //
        //  We should be able to set up our device object now. If not, for some
        //  reason, then give up.
        //
        m_upnpsRendCtrl.Release();
        m_upnpsAVTrans.Release();
        m_upnpdUPnPMediaRend.Release();
        if (!facCIDUPnP().bSetDeviceFromUID(m_strDevUID, m_upnpdUPnPMediaRend))
        {
            if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
            {
                facUPnPMediaRendS().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kUPnPMRErrs::errcProto_SetDevID
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::CantDo
                );
            }
            return kCIDLib::False;
        }

        //
        //  Get a list of the services of the types we need. If it doesn't provide
        //  one of them, then we give up.
        //
        tCIDLib::TKVPList colServices;
        m_upnpdUPnPMediaRend.QueryServices(colServices);

        if (!facCIDUPnP().bSetServiceByType(colServices
                                            , m_upnpdUPnPMediaRend
                                            , m_upnpsAVTrans
                                            , kCIDUPnP::strSvcType_AVTransport
                                            , kCIDLib::False))
        {
            if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
            {
                facUPnPMediaRendS().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kUPnPMRErrs::errcProto_NoAVTrans
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::NotFound
                );
            }
            return kCIDLib::False;
        }

        if (!facCIDUPnP().bSetServiceByType(colServices
                                            , m_upnpdUPnPMediaRend
                                            , m_upnpsRendCtrl
                                            , kCIDUPnP::strSvcType_RendControl
                                            , kCIDLib::False))
        {
            if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
            {
                facUPnPMediaRendS().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kUPnPMRErrs::errcProto_NoRendCtrl
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::NotFound
                );
            }
            return kCIDLib::False;
        }

        //
        //  Enable eventing on our services, both of which support that. This will
        //  greatly reduce traffic.
        //
        m_upnpsAVTrans.EnableEvents();
        m_upnpsRendCtrl.EnableEvents();
    }

    catch(TError& errToCatch)
    {
        LogError(errToCatch, tCQCKit::EVerboseLvls::Medium, CID_FILE, CID_LINE);
        return kCIDLib::False;
    }
    return kCIDLib::True;
}



//
//  Called by our parent class to ak us to start up the player on the
//  pass media item. We return true or false to indicate if we id ok or not.
//
tCIDLib::TBoolean
TUPnPMediaRendSDriver::bPlayNewItem(const TCQCMediaPLItem& mpliNew)
{
    try
    {
        // Adjust the location info according to our driver prompt values
        TString strMassagedLoc = mpliNew.strLocInfo();
        if (m_bRemoveUNCSlashes && strMassagedLoc.bStartsWith(L"\\\\"))
            strMassagedLoc.Cut(0, 2);

        if (m_bFlipSeps)
            strMassagedLoc.bReplaceChar(L'\\', L'/');

        TString strLocInfo;
        if (m_strLocInfoPattern.bIsEmpty())
        {
            // No pattern so take it as we massaged it
            strLocInfo = strMassagedLoc;
        }
         else
        {
            // Format it into the pattern
            strLocInfo = m_strLocInfoPattern;
            strLocInfo.eReplaceToken(strMassagedLoc, kCIDLib::chLatin_L);
        }

        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Low)
        {
            facUPnPMediaRendS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , L"Setting new AV Transport URI"
                , strLocInfo
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );
        }

        //
        //  To avoid a race condition, set our state flags now to indicate we are
        //  playing. Else, the base class may poll us before we get the initial state
        //  change info, and he could see us as stopped still and kill another song.
        //
        //  If it fails, we'll see that once we get the status, or if we get an
        //  exception and catch it below.
        //
        m_eState = tCQCMedia::EMediaStates::Loading;
        m_eEndState = tCQCMedia::EEndStates::Running;

        // Tell the A/V transport to try to play this guy
        m_upnpsAVTrans.SetAVTransportURI
        (
            strLocInfo
            , mpliNew.strItemArtist()
            , mpliNew.strTitleName()
            , mpliNew.strItemName()
            , 1
            , mpliNew.c4ColYear()
        );

        // Send a play to start it playing
        m_upnpsAVTrans.Play();

        //
        //  Set a time stamp for how long we'll wait for it to start playing. Our
        //  player status callback will continue to return loading until at least
        //  this long or it sees something besides stopped.
        //
        m_enctWaitStart = TTime::enctNowPlusSecs(8);
    }

    catch(TError& errToCatch)
    {
        // Something went awry, so set our states
        m_eState = tCQCMedia::EMediaStates::Stopped;
        m_eEndState = tCQCMedia::EEndStates::Failed;

        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Low)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
        return kCIDLib::False;
    }

    return kCIDLib::True;
}



// Try to load configuration data
tCIDLib::TBoolean TUPnPMediaRendSDriver::bWaitConfig(TThread&)
{
    //
    //  We have no config to get (or we get it from the device), so just
    //  say we are happy.
    //
    return kCIDLib::True;
}



tCQCKit::ECommResults
TUPnPMediaRendSDriver::eAdjustVolume(const tCIDLib::TBoolean bUp)
{
    // Not implemented yet, just say it worked
    return tCQCKit::ECommResults::Success;
}



tCQCKit::ECommResults
TUPnPMediaRendSDriver::eDoTransportCmd(const tCQCMedia::ETransCmds eToSet)
{
    //
    //  The next/prev ones we handle ourselves since we control the
    //  playlist. The others we pass on to UPnP.
    //
    if ((eToSet == tCQCMedia::ETransCmds::Next)
    ||  (eToSet == tCQCMedia::ETransCmds::Previous))
    {
        // Call our parent class
        SelectNextPrev(eToSet == tCQCMedia::ETransCmds::Next);
    }
     else
    {
        if (eToSet == tCQCMedia::ETransCmds::Pause)
            m_upnpsAVTrans.Pause();
        else if (eToSet == tCQCMedia::ETransCmds::Play)
            m_upnpsAVTrans.Play();
        else if (eToSet == tCQCMedia::ETransCmds::Stop)
            m_upnpsAVTrans.Stop();
    }

    return tCQCKit::ECommResults::Success;
}


//
//  This is called when the parent class wants us to get the player status.
//  We do periodic queries of various bits and store them away. We return
//  the
//
tCQCKit::ECommResults
TUPnPMediaRendSDriver::eCheckPlayerStatus(  tCIDLib::TEncodedTime&      enctCurPos
                                            , tCIDLib::TEncodedTime&    enctTotal
                                            , tCQCMedia::EMediaStates&  eState
                                            , tCQCMedia::EEndStates&    eEndState
                                            , tCIDLib::TBoolean&        bMute
                                            , tCIDLib::TCard4&          c4Volume)
{
    // Do any polling we need to do this round
    try
    {
        tCIDLib::TEncodedTime enctCur = TTime::enctNow();

        // See if we need to query position info
        if ((enctCur - m_enctLastPosQ) > kCIDLib::enctOneSecond)
        {
            QueryPosInfo();
            enctCur = TTime::enctNow();
        }

        // See if we need to query transport info
        if ((enctCur - m_enctLastTransQ) > kCIDLib::enctHalfSecond)
        {
            QueryTransInfo();
            enctCur = TTime::enctNow();
        }

        // See if we need to query renderer control info
        if ((enctCur - m_enctLastRCQ) > kCIDLib::enctHalfSecond)
        {
            QueryRendCtrlInfo();
            enctCur = TTime::enctNow();
        }

        // Give back the latest info we've stored
        bMute = m_bMute;
        c4Volume = m_c4Volume;
        enctCurPos = m_enctCurPos;
        enctTotal = m_enctCurTotal;

        //
        //  If we have a wait start time stamp, then we will return loading status
        //  until such time as we playing status.
        //
        if (m_enctWaitStart)
        {
            if ((m_enctWaitStart < enctCur)
            ||  (m_eState == tCQCMedia::EMediaStates::Playing))
            {
                // Return the real status and clear the wait stamp
                m_enctWaitStart = 0;
                eEndState = m_eEndState;
                eState = m_eState;
            }
             else
            {
                eState = tCQCMedia::EMediaStates::Loading;
                eEndState = tCQCMedia::EEndStates::Running;
            }
        }
         else
        {
            // Not waiting for start, so return the real statatus
            eEndState = m_eEndState;
            eState = m_eState;
        }
    }

    catch(TError& errToCatch)
    {
        LogError(errToCatch, tCQCKit::EVerboseLvls::High, CID_FILE, CID_LINE);
        return tCQCKit::ECommResults::LostCommRes;
    }
    return tCQCKit::ECommResults::Success;
}


// Try to connect to the player
tCQCKit::ECommResults TUPnPMediaRendSDriver::eConnectToDevice(TThread&)
{
    // Get our service objects
    try
    {
        //
        //  Query his min/max dB volume values if configured for dB type
        //  volume contorl.
        //
        if (m_bDoDBVolume)
            m_upnpsRendCtrl.QueryVolDBRange(m_i4MinVol, m_i4MaxVol);

        // And query the render control info to get our volume/mute set
        QueryRendCtrlInfo();

        //
        //  Since we are just letting the base renderer class do faux power
        //  management for us, put us initially into 'on' mode for convenience.
        //  We'll just stay that way unless a client explicitly powers us 'off'.
        //
        StorePowerStatus(tCQCKit::EPowerStatus::Ready);
    }

    catch(TError& errToCatch)
    {
        LogError(errToCatch, tCQCKit::EVerboseLvls::Medium, CID_FILE, CID_LINE);
        return tCQCKit::ECommResults::LostCommRes;
    }

    // Reset some data related to polling
    m_enctCurPos = 0;
    m_enctCurTotal = 0;
    m_enctLastPosQ = 0;
    m_enctLastTransQ = 0;
    m_eEndState = tCQCMedia::EEndStates::Ended;
    m_eState = tCQCMedia::EMediaStates::Stopped;

    return tCQCKit::ECommResults::Success;
}


//
// Initialize ourself and register fields
//
tCQCKit::EDrvInitRes TUPnPMediaRendSDriver::eInitializeImpl()
{
    // If we've not initialized the aysync finder yet, then do so
    if (!TAtomic::pFencedGet(&UPnPMediaRendS_DriverImpl::pupnpafRend))
    {
        TBaseLock lockInit;
        if (!TAtomic::pFencedGet(&UPnPMediaRendS_DriverImpl::pupnpafRend))
        {
            facUPnPMediaRendS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , L"Faulting in UPnP async finder. Driver=%(1)"
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
                , strMoniker()
            );

            TUPnPAsyncFinder* pupnpafNew = new TUPnPAsyncFinder;
            TJanitor<TUPnPAsyncFinder> janNew(pupnpafNew);

            TString strName(L"UPnPFnd_Drv_");
            strName.Append(strMoniker());
            pupnpafNew->strName(strName);
            pupnpafNew->StartSearch(kCIDUPnP::strDevType_MediaRenderer, L"MedRendSearch");

            // And now store it LAST, since it's the fault in flag
            TAtomic::FencedSet(&UPnPMediaRendS_DriverImpl::pupnpafRend, janNew.pobjOrphan());
        }
    }

    //
    //  Make sure that we were configured for a UPnP connection. Otherwise,
    //  its a bad configuration.
    //
    const TCQCDriverObjCfg& cqcdcOurs = cqcdcThis();
    if (cqcdcOurs.conncfgReal().clsIsA() != TCQCUPnPConnCfg::clsThis())
    {
        facCQCKit().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcDrv_BadConnCfgType
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Config
            , cqcdcOurs.conncfgReal().clsIsA()
            , clsIsA()
            , TCQCUPnPConnCfg::clsThis()
        );
        return tCQCKit::EDrvInitRes::Failed;
    }

    //
    //  Its the right type so do the down cast and get the data out of it
    //  that we need.
    //
    m_conncfgUPnP = static_cast<const TCQCUPnPConnCfg&>(cqcdcOurs.conncfgReal());
    m_strDevUID = m_conncfgUPnP.strDevUID();

    //
    //  Set appropriate poll/reconnect times. We want a fast poll time, though
    //  we don't really poll. We just want to react quickly to async messages
    //  from the player.
    //
    SetPollTimes(200, 5000);

    //
    //  We have some fixed prompts for the location pattern and whether or not
    //  we should flip back slashes to forward ones and whether we should remove
    //  the leading \\ slashes from UNC names. Set it to the raw loc
    //  info if not found. And we get one to indicate what kinds of media the
    //  target device can handle.
    //
    if (!cqcdcOurs.bFindFixedVal(L"LocFormat", L"LocPattern", m_strLocInfoPattern))
        m_strLocInfoPattern = L"%(L)";

    TString strPromptVal;
    if (!cqcdcOurs.bFindFixedVal(L"LocFormat", L"FlipSeps", strPromptVal))
        m_bFlipSeps = kCIDLib::False;
    else
        m_bFlipSeps = (strPromptVal.bCompareI(L"Yes"));

    if (!cqcdcOurs.bFindFixedVal(L"LocFormat", L"RemoveUNCSlashes", strPromptVal))
        m_bRemoveUNCSlashes = kCIDLib::False;
    else
        m_bRemoveUNCSlashes = (strPromptVal.bCompareI(L"Yes"));

    tCQCMedia::EMTFlags eMTTypes = tCQCMedia::EMTFlags::MultiMedia;
    if (cqcdcOurs.bFindFixedVal(L"MediaSupport", L"MediaTypes", strPromptVal))
    {
        if (strPromptVal.bCompareI(L"Music"))
        {
           eMTTypes = tCQCMedia::EMTFlags::Music;
        }
         else if (strPromptVal.bCompareI(L"Movie"))
        {
           eMTTypes = tCQCMedia::EMTFlags::Movie;
        }
         else
        {
            facUPnPMediaRendS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , L"Invalid media types fixed prompt value. Defaulting to all types!. Driver=%(1)"
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
                , strMoniker()
            );
        }

        // Set this on our parent. Has to be done before we call our parent's init below
        SetMTFlags(eMTTypes);
    }
     else
    {
        facUPnPMediaRendS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"Missing media types fixed prompt value. Defaulting to all types!. Driver=%(1)"
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
            , strMoniker()
        );
    }

    //
    //  Get whether we should do dB or vendor specific volume. Default to
    //  dB if not provided.
    //
    m_bDoDBVolume = kCIDLib::True;
    if (cqcdcOurs.bFindFixedVal(L"VolCtrl", L"Type", strPromptVal))
    {
        if (strPromptVal.bCompareI(L"Vendor"))
        {
            m_bDoDBVolume = kCIDLib::False;

            // We have to have the range values
            TString strMinVal;
            TString strMaxVal;
            if (cqcdcOurs.bFindFixedVal(L"VolCtrl", L"Min", strMinVal)
            &&  cqcdcOurs.bFindFixedVal(L"VolCtrl", L"Max", strMaxVal))
            {
                //
                //  They are really unsigned in this case, but we have to support
                //  signed for dB style and these will fit in the int range.
                //
                if (!strMinVal.bToInt4(m_i4MinVol))
                    m_i4MinVol = 0;

                if (!strMaxVal.bToInt4(m_i4MaxVol))
                    m_i4MaxVol = m_i4MinVol + 100;
            }
             else
            {
                facUPnPMediaRendS().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , L"Missing vendor vol control min or max value. Defaulting 0 to 100!. Driver=%(1)"
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::AppStatus
                    , strMoniker()
                );
            }
        }
         else if (strPromptVal.bCompareI(L"dB"))
        {
            // We'll query the range values later
            m_bDoDBVolume = kCIDLib::True;
        }
         else
        {
            facUPnPMediaRendS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , L"Invalid volume control fixed prompt value. Defaulting to dB! Driver=%(1)"
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
                , strMoniker()
            );
        }
    }

    //
    //  And now call our parent and return his return, to completely init.
    //  He'll call us back on some virtuals to do setup.
    //
    return TParent::eInitializeImpl();
}



tCQCKit::ECommResults
TUPnPMediaRendSDriver::eSetMute(const tCIDLib::TBoolean bToSet)
{
    m_upnpsRendCtrl.SetMute(bToSet);
    return tCQCKit::ECommResults::Success;
}


tCQCKit::ECommResults
TUPnPMediaRendSDriver::eSetVolume(const tCIDLib::TCard4 c4ToSet)
{
    // Convert our volume percentage to the device's range
    tCIDLib::TFloat8 f8Per = tCIDLib::TFloat8(c4ToSet) / 100.0;

    // Calculate the new value as a percentage of the range
    tCIDLib::TInt4 i4NewVol = tCIDLib::TInt4
    (
        f8Per * (tCIDLib::TFloat8(m_i4MaxVol) - tCIDLib::TFloat8(m_i4MinVol))
    );

    // And bias for the min volume position
    i4NewVol += m_i4MinVol;

    // Clip it in case of rounding
    if (i4NewVol < m_i4MinVol)
        i4NewVol = m_i4MinVol;
    else if (i4NewVol > m_i4MaxVol)
        i4NewVol = m_i4MaxVol;

    if (m_bDoDBVolume)
        m_upnpsRendCtrl.SetVolumeDB(i4NewVol);
    else
        m_upnpsRendCtrl.SetVolume(tCIDLib::TCard4(i4NewVol));

    return tCQCKit::ECommResults::Success;
}


// A string field changed
tCQCKit::ECommResults
TUPnPMediaRendSDriver::eStringFldValChanged(const   TString&        strFldName
                                            , const tCIDLib::TCard4 c4FldId
                                            , const TString&        strNewValue)
{
    try
    {
        if (c4FldId == m_c4FldId_SetLocInfoPat)
        {
            // Make sure it has the require token if not empty
            if (!strNewValue.bIsEmpty() && !strNewValue.bTokenExists(kCIDLib::chLatin_L))
                return tCQCKit::ECommResults::BadValue;

            // Looks ok so store it for later use
            m_strLocInfoPattern = strNewValue;
        }
         else
        {
            // Pass it to our parent
            return TParent::eStringFldValChanged(strFldName, c4FldId, strNewValue);
        }
    }

    catch(TError& errToCatch)
    {
        LogError(errToCatch, tCQCKit::EVerboseLvls::Medium, CID_FILE, CID_LINE);
        return tCQCKit::ECommResults::LostCommRes;
    }
    return tCQCKit::ECommResults::Success;
}


//
//  Our parent calls this to give us a chance to load up any fields of
//  our own into the list.
//
tCIDLib::TVoid
TUPnPMediaRendSDriver::LoadDrvSpecificFlds(TVector<TCQCFldDef>& colFlds)
{
    TCQCFldDef flddNew;

    flddNew.Set
    (
        UPnPMediaRendS_DriverImpl::pszFldSetLocInfoPat
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Write
    );
    colFlds.objAdd(flddNew);
}



// Called by our parent after fields are registered, so we can look up ours
tCIDLib::TVoid
TUPnPMediaRendSDriver::LookupDrvSpecificFlds(const tCIDLib::TCard4 c4ArchVersion)
{
    m_c4FldId_SetLocInfoPat = pflddFind(UPnPMediaRendS_DriverImpl::pszFldSetLocInfoPat, kCIDLib::True)->c4Id();
}


// Release our UPnP objects
tCIDLib::TVoid TUPnPMediaRendSDriver::ReleaseCommResource()
{
    //
    //  Release the service objects and the device. This will not cause an
    //  error if they are already released or never got set.
    //
    m_upnpsRendCtrl.Release();
    m_upnpsAVTrans.Release();
    m_upnpdUPnPMediaRend.Release();
}


//
//  Our parent class will call this any time it wants us to stop playing
//  and unload any current media.
//
tCIDLib::TVoid TUPnPMediaRendSDriver::ResetPlayer()
{
    m_upnpsAVTrans.Stop();

    // Go ahead and update our states now
    m_eEndState = tCQCMedia::EEndStates::Ended;
    m_eState = tCQCMedia::EMediaStates::Stopped;
}


tCIDLib::TVoid TUPnPMediaRendSDriver::TerminateImpl()
{
}


// ---------------------------------------------------------------------------
//  TUPnPMediaRendSDriver: Private, non-virtual methods
// ---------------------------------------------------------------------------

// Called to poll current position info
tCIDLib::TVoid TUPnPMediaRendSDriver::QueryPosInfo()
{
    // Reset our last pos info query time
    m_enctLastPosQ = TTime::enctNow();

    tCIDLib::TCard4         c4PerComplete;
    tCIDLib::TCard4         c4TrackNum;
    m_upnpsAVTrans.QueryPositionInfo
    (
        m_strTmpPoll, m_enctCurTotal, c4TrackNum, m_enctCurPos, c4PerComplete
    );
}


// Called to poll current transport info
tCIDLib::TVoid TUPnPMediaRendSDriver::QueryRendCtrlInfo()
{
    // Reset our last transport info query time
    m_enctLastRCQ = TTime::enctNow();

    //
    //  Get the volume, dealing with the type of volume control we were
    //  told by the manifest to use. Vendor volume is really a cardinal
    //  but we are ok using ints so that we can treat them the same here
    //  for calculation purposes. We stored the min/max values as ints
    //  in either case.
    //
    tCIDLib::TInt4 i4DevVol;
    if (m_bDoDBVolume)
        i4DevVol = m_upnpsRendCtrl.i4QueryVolumeDB();
    else
        i4DevVol = tCIDLib::TInt4(m_upnpsRendCtrl.c4QueryVolume());

    //
    //  Convert the volume to our percentage format. We have to bias
    //  the current volume up to a positive number, then we can divide
    //  it by the full range.
    //
    if (m_i4MinVol < 0)
        i4DevVol -= m_i4MinVol;

    tCIDLib::TFloat8 f8Per
    (
        tCIDLib::TFloat8(i4DevVol)
        / (tCIDLib::TFloat8(m_i4MaxVol) - tCIDLib::TFloat8(m_i4MinVol))
    );

    // Get back to a card and clip if needed
    tCIDLib::TCard4 c4Per = tCIDLib::TCard4(f8Per * 100.0);
    if (c4Per > 100)
        c4Per = 100;

    m_bMute = m_upnpsRendCtrl.bQueryMute();
    m_c4Volume = c4Per;
}


// Called to poll current transport info
tCIDLib::TVoid TUPnPMediaRendSDriver::QueryTransInfo()
{
    // Reset our last transport info query time
    m_enctLastTransQ = TTime::enctNow();

    // Query our info from the player
    tCIDLib::TCard4         c4Speed;
    tCIDUPnP::ETransStates  eState;
    m_upnpsAVTrans.QueryTransportInfo(eState, m_strTmpPoll, c4Speed);

    switch(eState)
    {
        case tCIDUPnP::ETransStates::STOPPED :
        case tCIDUPnP::ETransStates::NO_MEDIA_PRESENT :
            m_eState = tCQCMedia::EMediaStates::Stopped;
            break;

        case tCIDUPnP::ETransStates::PAUSED_PLAYBACK :
        case tCIDUPnP::ETransStates::PAUSED_RECORDING :
            m_eState = tCQCMedia::EMediaStates::Paused;
            break;

        case tCIDUPnP::ETransStates::PLAYING :
        case tCIDUPnP::ETransStates::RECORDING :
            m_eState = tCQCMedia::EMediaStates::Playing;
            break;

        case tCIDUPnP::ETransStates::TRANSITIONING :
            m_eState = tCQCMedia::EMediaStates::Loading;
            break;

        case tCIDUPnP::ETransStates::UNKNOWN :
        default :
            m_eState = tCQCMedia::EMediaStates::Undefined;
            break;
    };

    // Update the end state based on the media state
    if (m_eState == tCQCMedia::EMediaStates::Stopped)
        m_eEndState = tCQCMedia::EEndStates::Ended;
    else
        m_eEndState = tCQCMedia::EEndStates::Running;
}


