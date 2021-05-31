//
// FILE NAME: SonosZPPropS_DriverImpl.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 05/14/2012
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
#include    "SonosZPPropS.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TSonosZPPropSDriver,TCQCServerBase)


namespace SonosZPS_DriverImpl
{
    // -----------------------------------------------------------------------
    //  All instances of this driver share an async finder object. It is
    //  faulted in upon first use.
    //
    //  !! NOTE:
    //
    //  Since the finder's methods for checking the list are already
    //  synchronized (because it has to sync with a background thread that
    //  updates the list), we can call them from multiple drivers and not
    //  cause issues. Since the object is never destroyed, all we have to do
    //  is synchronize the creation of the finder.
    //
    //  This way we don't have to have a separate one per driver.
    // -----------------------------------------------------------------------
    TUPnPAsyncFinder*       pupnpafRend = nullptr;


    // -----------------------------------------------------------------------
    //  Some fields that we don't go back and look up since they are only
    //  set during the connect. So we need the names in the connect and the
    //  field registration methods.
    // -----------------------------------------------------------------------
    const tCIDLib::TCh* const pszFld_DevUID(L"DevUID");
    const tCIDLib::TCh* const pszFld_IPAddress(L"IPAddress");
    const tCIDLib::TCh* const pszFld_MACAddress(L"MACAddress");
    const tCIDLib::TCh* const pszFld_SerialNum(L"SerialNum");
    const tCIDLib::TCh* const pszFld_SoftwareVer(L"SofwareVer");


    // -----------------------------------------------------------------------
    //  Some Sonos speicfic values we use
    // -----------------------------------------------------------------------
    const tCIDLib::TCh* const pszCont_Tracks(L"A:TRACKS");
    const tCIDLib::TCh* const pszCont_StoredQ(L"SQ:");
    const tCIDLib::TCh* const pszCont_CurQ(L"Q:0");

    const tCIDLib::TCh* const pszURI_PLFile(L"file:///jffs/settings/savedqueues.rsq#");
    const tCIDLib::TCh* const pszPref_XInCon(L"x-rincon:");
    const tCIDLib::TCh* const pszPref_StreamInCon(L"x-rincon-stream:");
    const tCIDLib::TCh* const pszPref_Container(L"x-rincon-cpcontainer:");
    const tCIDLib::TCh* const pszPref_Playlist(L"x-rincon-playlist:");


    // -----------------------------------------------------------------------
    //  Some state variables we look for
    // -----------------------------------------------------------------------
    const TString   strVar_FavoritesUpdateID(L"FavoritesUpdateID");


    // -----------------------------------------------------------------------
    //  Backdoor commands we support.
    // -----------------------------------------------------------------------

    // Used with the bQueryTextVal method
    const tCIDLib::TCh* const pszCmdType_ListQuery(L"ListQuery");
    const tCIDLib::TCh* const pszCmdType_FavsList(L"FavsList");
    const tCIDLib::TCh* const pszCmdType_PLList(L"PLList");
}




// ---------------------------------------------------------------------------
//   CLASS: SonosZPPropSDriver
//  PREFIX: drv
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  SonosZPPropSDriver: Constructors and Destructor
// ---------------------------------------------------------------------------
TSonosZPPropSDriver::TSonosZPPropSDriver(const TCQCDriverObjCfg& cqcdcToLoad) :

    TCQCServerBase(cqcdcToLoad)
    , m_c4FinderSerialNum(0)

    , m_c4FldId_ArtURL(kCIDLib::c4MaxCard)
    , m_c4FldId_AVTransURI(kCIDLib::c4MaxCard)
    , m_c4FldId_ClearQ(kCIDLib::c4MaxCard)
    , m_c4FldId_CrossfadeMode(kCIDLib::c4MaxCard)
    , m_c4FldId_CurAlbum(kCIDLib::c4MaxCard)
    , m_c4FldId_CurArtist(kCIDLib::c4MaxCard)
    , m_c4FldId_CurTrack(kCIDLib::c4MaxCard)
    , m_c4FldId_CurTrackDur(kCIDLib::c4MaxCard)
    , m_c4FldId_CurTrackNum(kCIDLib::c4MaxCard)
    , m_c4FldId_CurTrackTime(kCIDLib::c4MaxCard)
    , m_c4FldId_CurTrackURI(kCIDLib::c4MaxCard)
//    , m_c4FldId_DevName(kCIDLib::c4MaxCard)
//    , m_c4FldId_Icon(kCIDLib::c4MaxCard)
    , m_c4FldId_InvokeCmd(kCIDLib::c4MaxCard)
//    , m_c4FldId_IsBridge(kCIDLib::c4MaxCard)
    , m_c4FldId_IsLocalGroup(kCIDLib::c4MaxCard)
    , m_c4FldId_Mute(kCIDLib::c4MaxCard)
    , m_c4FldId_PBPercent(kCIDLib::c4MaxCard)
    , m_c4FldId_PlayMode(kCIDLib::c4MaxCard)
//    , m_c4FldId_SetLED(kCIDLib::c4MaxCard)
    , m_c4FldId_ShuffleMode(kCIDLib::c4MaxCard)
    , m_c4FldId_Transport(kCIDLib::c4MaxCard)
    , m_c4FldId_TransState(kCIDLib::c4MaxCard)
    , m_c4FldId_Volume(kCIDLib::c4MaxCard)
    , m_c4SerialNum_AVTrans(0)
    , m_c4SerialNum_ContDir(0)
    , m_c4SerialNum_DevProps(0)
    , m_c4SerialNum_RendCtrl(0)
    , m_c4FldId_GroupCoord(kCIDLib::c4MaxCard)
    , m_c4SerialNum_GrpMgmt(0)
    , m_enctNextAVQ(0)
    , m_i4ActivePLLUpdateID(0)
    , m_i4LastQUpdateID(0)
    , m_i4LastFavsUpdateID(-1)
    , m_i4LastPLLUpdateID(0)
{
}

TSonosZPPropSDriver::~TSonosZPPropSDriver()
{
}


// ---------------------------------------------------------------------------
//  TSonosZPPropSDriver: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  We support some queries for text based data.
//
tCIDLib::TBoolean
TSonosZPPropSDriver::bQueryTextVal( const   TString&    strQueryType
                                    , const TString&    strDataName
                                    ,       TString&    strToFill)
{
    tCIDLib::TBoolean bRet = kCIDLib::False;

    // Clear it in case we fail, so that we don't return bogus info
    strToFill.Clear();

    // Handle the query based on the query type
    if ((strQueryType == SonosZPS_DriverImpl::pszCmdType_ListQuery)
    &&  (strDataName == SonosZPS_DriverImpl::pszCmdType_PLList))
    {
        // See if we need to refresh the list of playlists
        CheckPLListReload();

        //
        //  They want a list of playlists. This is a double list, with
        //  the names in the first list and the ids in the second. We
        //  have to do them in separate strings then combine, else we
        //  wil get a comma between them.
        //
        const tCIDLib::TCard4 c4Count = m_colPLList.c4ElemCount();
        if (c4Count)
        {
            // Do the first list
            TString strList1;
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            {
                TStringTokenizer::BuildQuotedCommaList
                (
                    m_colPLList[c4Index].strValue(), strList1
                );
            }

            TString strList2;
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            {
                TStringTokenizer::BuildQuotedCommaList
                (
                    m_colPLList[c4Index].strKey(), strList2
                );
            }

            strToFill = strList1;
            strToFill.Append(L"\r\n");
            strToFill.Append(strList2);

            bRet = kCIDLib::True;
        }
    }
     else if ((strQueryType == SonosZPS_DriverImpl::pszCmdType_ListQuery)
          &&  (strDataName == SonosZPS_DriverImpl::pszCmdType_FavsList))
    {
        // Reload the list if we are out of date
        CheckFavsListReload();

        const tCIDLib::TCard4 c4Count = m_colFavsList.c4ElemCount();
        if (c4Count)
        {
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            {
                TStringTokenizer::BuildQuotedCommaList
                (
                    m_colFavsList[c4Index].strKey(), strToFill
                );
            }
            bRet = kCIDLib::True;
        }
    }
     else
    {
        // Don't know what it is, so throw
        facSonosZPPropS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kSonosZPErrs::errcBDCall_UnknownBDQuery
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::BadParms
            , strQueryType
            , strMoniker()
        );
    }

    return bRet;
}


// ---------------------------------------------------------------------------
//  TSonosZPPropSDriver: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TSonosZPPropSDriver::bGetCommResource(TThread&)
{
    try
    {
        // Do the UPnP setup to get all the devices and services initialized
        UPnPSetup();
    }

    catch(TError& errToCatch)
    {
        LogError(errToCatch, tCQCKit::EVerboseLvls::Medium, CID_FILE, CID_LINE);
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


// Try to load configuration data
tCIDLib::TBoolean TSonosZPPropSDriver::bWaitConfig(TThread&)
{
    //
    //  We have no config to get other than some info we may get from the
    //  device itself, so just say we are happy.
    //
    return kCIDLib::True;
}


// A Boolean field changed
tCQCKit::ECommResults
TSonosZPPropSDriver::eBoolFldValChanged(const   TString&
                                        , const tCIDLib::TCard4     c4FldId
                                        , const tCIDLib::TBoolean   bNewValue)
{
    try
    {
        if (c4FldId == m_c4FldId_ClearQ)
        {
            if (bCheckIsLocalGrp())
                m_upnpsAVTrans.ClearQueue();
            else
                return tCQCKit::ECommResults::ResNotAvail;
        }
         else if (c4FldId == m_c4FldId_CrossfadeMode)
        {
            m_upnpsAVTrans.bCrossfadeMode(bNewValue);
        }
         else if (c4FldId == m_c4FldId_Mute)
        {
            m_upnpsRendCtrl.SetMute(bNewValue);
        }
/*
         else if (c4FldId == m_c4FldId_SetLED)
        {
            m_upnpsDevProps.bLEDState(bNewValue);
        }
*/
         else if (c4FldId == m_c4FldId_ShuffleMode)
        {
            if (bNewValue)
                m_upnpsAVTrans.SetPlayMode(tCIDUPnP::EPlayModes::SHUFFLE);
            else
                m_upnpsAVTrans.SetPlayMode(tCIDUPnP::EPlayModes::NORMAL);
        }
         else
        {
            IncUnknownWriteCounter();
            return tCQCKit::ECommResults::FieldNotFound;
        }
    }

    catch(TError& errToCatch)
    {
        LogError(errToCatch, tCQCKit::EVerboseLvls::Medium, CID_FILE, CID_LINE);
        return tCQCKit::ECommResults::LostCommRes;
    }
    return tCQCKit::ECommResults::Success;
}


// A Cardinal field changed
tCQCKit::ECommResults
TSonosZPPropSDriver::eCardFldValChanged(const   TString&
                                        , const tCIDLib::TCard4 c4FldId
                                        , const tCIDLib::TCard4 c4NewValue)
{
    try
    {
        if (c4FldId == m_c4FldId_Volume)
        {
            if (c4NewValue > 100)
                return tCQCKit::ECommResults::BadValue;
            m_upnpsRendCtrl.SetVolume(c4NewValue);
        }
         else
        {
            IncUnknownWriteCounter();
            return tCQCKit::ECommResults::FieldNotFound;
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
//  We set up our device and service objects in the GetCommRes phase. Here
//  we just get our fields initialized. No need to lock since we've not connected
//  yet.
//
//  NOTE: We enable eventing on services here AFTER we do the initial query,
//  so that we are sure to get initial info. The arrival of the initial hit
//  of evented data is not guaranteed to be within any particularly short
//  time.
//
tCQCKit::ECommResults TSonosZPPropSDriver::eConnectToDevice(TThread&)
{
    try
    {
        // Store AVTrans info. Tell it to get current time info this time
        StoreAVTrans(kCIDLib::True);

        // Store content directoryinfo
        StoreContDir();

        // Store device property info
        // StoreDevProps();

        // Store group management info
        StoreGrpMgmt();

        // Store render control info
        StoreRendCtrl();

        //
        //  We can get some one time stuff now and store some fields by name
        //  since we don't bother to look these up given that they won't
        //  change unless we reconnect again.
        //
        /*
        TString strIPAddress;
        TString strMacAddress;
        TString strSerialNum;
        TString strSoftwareVer;
        m_upnpsDevProps.QueryZoneInfo
        (
            strSerialNum, strIPAddress, strMacAddress, strSoftwareVer
        );
        bStoreStringFld
        (
            SonosZPS_DriverImpl::pszFld_IPAddress, strIPAddress, kCIDLib::True
        );

        bStoreStringFld
        (
            SonosZPS_DriverImpl::pszFld_MACAddress, strMacAddress, kCIDLib::True
        );

        bStoreStringFld
        (
            SonosZPS_DriverImpl::pszFld_SerialNum, strSerialNum, kCIDLib::True
        );

        bStoreStringFld
        (
            SonosZPS_DriverImpl::pszFld_SoftwareVer, strSoftwareVer, kCIDLib::True
        );
        */

        // This one we get from the user and we know it matches whatever device we found
        bStoreStringFld
        (
            SonosZPS_DriverImpl::pszFld_DevUID, m_strDevUID, kCIDLib::True
        );

        //
        //  Do an initial load of available playlists. Then initialize the
        //  last update id to equal the active update id now that we are in
        //  sync.
        //
        LoadPlaylistsList();
        m_i4LastPLLUpdateID = m_i4ActivePLLUpdateID;

        // Reset any per-connection stuff
        m_enctNextAVQ = TTime::enctNowPlusSecs(2);

        // Initialize our serial numbers for our evented services
        m_c4SerialNum_AVTrans  = m_upnpsAVTrans.c4SerialNum();
        m_c4SerialNum_ContDir = m_upnpsContDir.c4SerialNum();
//        m_c4SerialNum_DevProps = m_upnpsDevProps.c4SerialNum();
        m_c4SerialNum_RendCtrl = m_upnpsRendCtrl.c4SerialNum();

        // Enable eventing on services where that would be beneficial
        m_upnpsAVTrans.EnableEvents();
        m_upnpsContDir.EnableEvents();
//        m_upnpsDevProps.EnableEvents();
        m_upnpsRendCtrl.EnableEvents();

        m_c4SerialNum_GrpMgmt  = m_upnpsGrpMgmt.c4SerialNum();
        m_upnpsGrpMgmt.EnableEvents();
    }

    catch(TError& errToCatch)
    {
        LogError(errToCatch, tCQCKit::EVerboseLvls::Medium, CID_FILE, CID_LINE);
        return tCQCKit::ECommResults::LostCommRes;
    }

    return tCQCKit::ECommResults::Success;
}


// A floating point field changed
tCQCKit::ECommResults
TSonosZPPropSDriver::eFloatFldValChanged( const   TString&
                                            , const tCIDLib::TCard4     c4FldId
                                            , const tCIDLib::TFloat8    f8NewValue)
{
    IncUnknownWriteCounter();
    return tCQCKit::ECommResults::FieldNotFound;
}


//
// Initialize ourself and register fields
//
tCQCKit::EDrvInitRes TSonosZPPropSDriver::eInitializeImpl()
{
    // If we've not initialized the aysync finder yet, then do so
    if (!TAtomic::pFencedGet(&SonosZPS_DriverImpl::pupnpafRend))
    {
        TBaseLock lockInit;
        if (!TAtomic::pFencedGet(&SonosZPS_DriverImpl::pupnpafRend))
        {
            try
            {
                TUPnPAsyncFinder* pupnpafNew = new TUPnPAsyncFinder;
                TJanitor<TUPnPAsyncFinder> janFinder(pupnpafNew);

                TString strName(L"SonosZPP_Drv_");
                strName.Append(strMoniker());
                pupnpafNew->strName(strName);
                pupnpafNew->StartSearch(kCIDUPnP::strDevType_ZonePlayer, L"ZPSearch");

                TAtomic::pFencedSet
                (
                    &SonosZPS_DriverImpl::pupnpafRend, janFinder.pobjOrphan()
                );
            }

            catch(TError& errToCatch)
            {
                if (eVerboseLevel() > tCQCKit::EVerboseLvls::Medium)
                {
                    errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                    TModule::LogEventObj(errToCatch);
                }
                return tCQCKit::EDrvInitRes::Failed;
            }
        }
    }

    //
    //  Make sure that we were configured for a UPnP connection. Otherwise,
    //  its a bad configuration.
    //
    const TCQCDriverObjCfg& cqcdcOurs = cqcdcThis();
    if (cqcdcOurs.conncfgReal().clsIsA() != TCQCUPnPConnCfg::clsThis())
    {
        if (eVerboseLevel() > tCQCKit::EVerboseLvls::Medium)
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
        }
        return tCQCKit::EDrvInitRes::Failed;
    }

    //
    //  Its the right type so do the down cast and get the data out of it
    //  that we need.
    //
    m_conncfgUPnP = static_cast<const TCQCUPnPConnCfg&>(cqcdcOurs.conncfgReal());
    m_strDevUID = m_conncfgUPnP.strDevUID();

    //
    //  Set appropriate poll/reconnect times. We do the fastest possible poll
    //  time because we get async messages so we can react quickly with low
    //  overhead. Time stamps are used to limit any active polling.
    //
    SetPollTimes(100, 5000);

    // Register our fields
    RegisterFields();

    return tCQCKit::EDrvInitRes::WaitCommRes;
}


//
//  We are mainly just watching for event driven changes. If we see the
//  serial number of one of the evented services go up, we store away the
//  info from that service.
//
//  The exception is the AV Transport, where we will do an active query
//  every so often, to get the current media playback time, which is not
//  an evented value.
//
tCQCKit::ECommResults TSonosZPPropSDriver::ePollDevice(TThread&)
{
    try
    {
        tCIDLib::TEncodedTime enctCur = TTime::enctNow();

        //
        //  If it's been more than two seconds since the last time, then
        //  do a full AV transport info storage. Else, only do it if it
        //  has changed.
        //
        if (m_enctNextAVQ < enctCur)
        {
            // Reset the next time first in case of an exception
            m_enctNextAVQ = TTime::enctNowPlusSecs(2);
            StoreAVTrans(kCIDLib::True);

            // And now set it again in case the above took some time
            m_enctNextAVQ = TTime::enctNowPlusSecs(2);
        }
         else if (m_upnpsAVTrans.c4SerialNum() > m_c4SerialNum_AVTrans)
        {
            StoreAVTrans(kCIDLib::False);
        }

        // If any of the other event enabled services have changed, store them
        if (m_upnpsContDir.c4SerialNum() > m_c4SerialNum_ContDir)
            StoreContDir();

        if (m_upnpsRendCtrl.c4SerialNum() > m_c4SerialNum_RendCtrl)
            StoreRendCtrl();

        if (m_upnpsGrpMgmt.c4SerialNum() > m_c4SerialNum_GrpMgmt)
            StoreGrpMgmt();

//        if (m_upnpsDevProps.c4SerialNum() > m_c4SerialNum_DevProps)
//            StoreDevProps();
    }

    catch(ESvcErrs)
    {
        // A service has died, which means that want to recycle
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
        {
            facSonosZPPropS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kSonosZPErrs::errcProto_ServiceDied
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::LostConnection
            );
        }
        return tCQCKit::ECommResults::LostCommRes;
    }

    catch(TError& errToCatch)
    {
        LogError(errToCatch, tCQCKit::EVerboseLvls::High, CID_FILE, CID_LINE);
        return tCQCKit::ECommResults::LostCommRes;
    }
    return tCQCKit::ECommResults::Success;
}


// A string field changed
tCQCKit::ECommResults
TSonosZPPropSDriver::eStringFldValChanged(  const   TString&
                                            , const tCIDLib::TCard4 c4FldId
                                            , const TString&        strNewValue)
{
    tCQCKit::ECommResults eRes = tCQCKit::ECommResults::Success;
    try
    {
        if (c4FldId == m_c4FldId_InvokeCmd)
        {
            // Call a private helper to parse the command info and invoke it
            eRes = eInvokeCmd(strNewValue);
        }
         else if (c4FldId == m_c4FldId_Transport)
        {
            // This is only valid for the group coordinator
            if (!bCheckIsLocalGrp())
                return tCQCKit::ECommResults::ResNotAvail;

            try
            {
                if (strNewValue == L"Next")
                    m_upnpsAVTrans.Next();
                else if (strNewValue == L"Pause")
                    m_upnpsAVTrans.Pause();
                else if (strNewValue == L"Play")
                    m_upnpsAVTrans.Play();
                else if (strNewValue == L"Prev")
                    m_upnpsAVTrans.Previous();
                else if (strNewValue == L"Stop")
                    m_upnpsAVTrans.Stop();
                else
                {
                    IncFailedWriteCounter();
                    eRes = tCQCKit::ECommResults::BadValue;
                }
            }

            catch(TError& errToCatch)
            {
                //
                //  These can all fail for stupid reasons like doing next or
                //  previous when there's only one file. It returns an error
                //  for those types of things. So we don't let them propogate
                //  and knock us off line.
                //
                IncFailedWriteCounter();
                LogError(errToCatch, tCQCKit::EVerboseLvls::Medium, CID_FILE, CID_LINE);
            }
        }
          else
        {
            IncUnknownWriteCounter();
            eRes = tCQCKit::ECommResults::FieldNotFound;
        }
    }

    catch(TError& errToCatch)
    {
        LogError(errToCatch, tCQCKit::EVerboseLvls::Medium, CID_FILE, CID_LINE);
        return tCQCKit::ECommResults::LostCommRes;
    }
    return eRes;
}



// Release all our services and devices
tCIDLib::TVoid TSonosZPPropSDriver::ReleaseCommResource()
{
    //
    //  Release the service objects and the devices. This will not cause an
    //  error if they are already released or never got set.
    //
    ReleaseService(m_upnpsGrpMgmt, L"Group Management");

//    ReleaseService(m_upnpsDevProps, L"Device Properties");
    ReleaseService(m_upnpsAVTrans, L"AV Transport");
    ReleaseService(m_upnpsRendCtrl, L"Rend Ctrl");
    ReleaseService(m_upnpsContDir, L"Content Directory");

    ReleaseDevice(m_upnpdMedRend, L"Media Renderer");
    ReleaseDevice(m_upnpdMedSrv, L"Media Server");
    ReleaseDevice(m_upnpdZP, L"Zone Player");
}


tCIDLib::TVoid TSonosZPPropSDriver::TerminateImpl()
{
}


// ---------------------------------------------------------------------------
//  TSonosZPPropSDriver: Private, non-virtual methods
// ---------------------------------------------------------------------------


//
//  Sees if we are a local group. If not it will return false and log a
//  message if in verbose omde.
//
tCIDLib::TBoolean TSonosZPPropSDriver::bCheckIsLocalGrp()
{
    tCIDLib::TBoolean bLocal = bFldValue(m_c4FldId_IsLocalGroup);

    // If not local and verbose, log a message
    if (!bLocal && (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium))
    {
        facSonosZPPropS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kSonosZPErrs::errcProto_NotCoord
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Authority
            , strMoniker()
        );
    }
    return bLocal;
}


// A helperfor the eInvokeCmd method, to check parameter counts
tCIDLib::TBoolean
TSonosZPPropSDriver::bCheckParms(const tCIDLib::TStrCollect&    colParms
                                , const tCIDLib::TCard4         c4ExpCnt
                                , const TString&                strCmd)
{
    if (colParms.c4ElemCount() != c4ExpCnt)
    {
        facSonosZPPropS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kSonosZPErrs::errcFld_BadInvokeCmdParmCnt
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , strCmd
            , TCardinal(c4ExpCnt)
        );
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


//
//  We search our list of available playlists. An initial one is gottne
//  during connect, and probably it'll remain valid, But we also watch for
//  update id changes and if it's changed, we'll reload it here before we
//  search.
//
//  DUE TO a bug in Windows UPNP, apparently, we don't get container update
//  id events. So, instead we have to do a query of a single item here to
//  see if the list has changed.
//
tCIDLib::TBoolean
TSonosZPPropSDriver::bFindNamedPL(  const   TString&            strToFind
                                    ,       tCIDLib::TCard4&    c4PLNum)
{
    CheckPLListReload();

    // Now go through the list and see if we can find this guy
    const tCIDLib::TCard4 c4Count = m_colPLList.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TKeyValuePair& kvalCur = m_colPLList[c4Index];
        if (kvalCur.strValue().bCompareI(strToFind))
        {
            bValidatePLID(kvalCur.strKey(), c4PLNum);
            return kCIDLib::True;
        }
    }
    return kCIDLib::False;
}


//
//  Checks a service to see if it's been marked dead, which will happen
//  asynchronously for those that are evented.
//
tCIDLib::TVoid TSonosZPPropSDriver::CheckIsDead(TUPnPService& upnpsToCheck)
{
    if (upnpsToCheck.bIsDead())
    {
        throw(ESvcErr_Died);
    }
}


//
//  Due to a Windows bug, apparently, we don't get reports of container update
//  ids, so we have to do a manual check by reading one item in order to get
//  the most recent update id, which we can check against the last one we
//  stored. If it's changed, we read the whole list.
//
tCIDLib::TVoid TSonosZPPropSDriver::CheckFavsListReload()
{
    tCIDLib::TCard4 c4Total;
    tCIDLib::TInt4  i4TestID;
    tCIDLib::TKValsList colTmp(1);
    m_upnpsContDir.c4BrowseFavorites(0, 1, c4Total, colTmp, i4TestID, kCIDLib::False);

    // If it's not the same as the last one we stored, get the whole list
    if (i4TestID != m_i4LastFavsUpdateID)
        LoadFavoritesList();
}


//
//  Due to a Windows bug, apparently, we don't get reports of container update
//  ids, so we have to do a manual check by reading one item in order to get
//  the most recent update id, which we can check against the last one we
//  stored.
//
tCIDLib::TVoid TSonosZPPropSDriver::CheckPLListReload()
{
    // Do a reload if needed
    tCIDLib::TCard4 c4Total;
    tCIDLib::TKVPList colTest;
    tCIDLib::TCard4 c4CurCnt = m_upnpsContDir.c4BrowseLists
    (
        SonosZPS_DriverImpl::pszCont_StoredQ
        , 0
        , 1
        , c4Total
        , colTest
        , m_i4LastPLLUpdateID
        , kCIDLib::False
    );

    // If not the same, then load the list
    if (m_i4LastPLLUpdateID != m_i4ActivePLLUpdateID)
        LoadPlaylistsList();
}


//
//  This is called when the InvokeCmd field is written to. We parse out the
//  passed command data and invoke the action.
//
//  Commands are:
//
//      AddPLToQ        : [ByName | ByIId], [name or id]
//      DeletePL        : [ByName | ById], [name or id]
//      GroupTo         : [<self> | coordmoniker]
//      MonAnalogIn     : srcmoniker
//      PlayFavorite    : favoriteid
//      SaveQAsPL       : playlistname
//      SetPLToQ        : [ByName | ByIId], [name or id]
//      SetTransURI     : "uri", "artist", "title", "track", "tracknum", "year"
//
tCQCKit::ECommResults TSonosZPPropSDriver::eInvokeCmd(const TString& strCmdData)
{
    // Get the command out first, which is followed by a colon
    TString strCmd(strCmdData);
    TString strParms;
    if (!strCmd.bSplit(strParms, kCIDLib::chColon, kCIDLib::True))
    {
        facSonosZPPropS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kSonosZPErrs::errcFld_BadInvokeCmdFmt
            , L"No colon separator found"
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
        );
        return tCQCKit::ECommResults::BadValue;
    }
    strCmd.StripWhitespace();

    // We'll break out the parameter bits into this list
    tCIDLib::TStrList colParms(4);

    //
    //  Special case the SetTransURI command, which has special formatting
    //  requirements on its parameters. We treat it as one quoted comma list
    //  because it can have spaces and potentially commas in the values.
    //
    tCIDLib::TBoolean bKnownCmd = kCIDLib::True;

    if (strCmd.bCompareI(L"SetTransURI"))
    {
        // Break out the values
        tCIDLib::TCard4 c4ErrIndex;
        if (TStringTokenizer::bParseQuotedCommaList(strParms, colParms, c4ErrIndex)
        &&  (colParms.c4ElemCount() == 6))
        {
            // Pass it on to the player
            m_upnpsAVTrans.SetAVTransportURI
            (
                colParms[0]
                , colParms[1]
                , colParms[2]
                , colParms[3]
                , colParms[4].c4Val(tCIDLib::ERadices::Dec)
                , colParms[5].c4Val(tCIDLib::ERadices::Dec)
            );
        }
         else
        {
            if (colParms.c4ElemCount() == 6)
            {
                strParms = L"Error in quoted comma list at index ";
                strParms.AppendFormatted(c4ErrIndex);
            }
             else
            {
                strParms = L"SetTransURI requires 6 parameters";
            }

            facSonosZPPropS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kSonosZPErrs::errcFld_BadInvokeCmdFmt
                , strParms
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
            );

            // Invalid, so log something and reject
            return tCQCKit::ECommResults::BadValue;
        }
    }
     else
    {
        // Not a special so split out the parameters, which are comma separated
        tCIDLib::TCard4 c4ParmCnt = 0;
        TString strToken;
        TStringTokenizer stokCmd(&strParms, L",");
        while (stokCmd.bGetNextToken(strToken))
        {
            strToken.StripWhitespace();
            colParms.objAdd(strToken);
        }

        // And now process the specific command we got
        if (strCmd.bCompareI(L"GroupTo"))
        {
            // We need one parameter
            if (!bCheckParms(colParms, 1, strCmd))
                return tCQCKit::ECommResults::BadValue;

            //
            //  If the parameter is [self], then we create a standalone group
            //  else it has to be the moniker of the desired group coordinator.
            //
            if (colParms[0].bCompareI(L"<self>"))
            {
                m_upnpsAVTrans.BecomeStandaloneGrp();
            }
             else
            {
                // We have to read the unique id of the target coordinator
                TString strUID;
                MonikerToUID(colParms[0], strUID);

                // Use that to build the appropriate URI
                m_upnpsAVTrans.SetAVTransportURI
                (
                    TString::strConcat(SonosZPS_DriverImpl::pszPref_XInCon, strUID)
                    , TString::strEmpty()
                );
            }
        }
         else if (strCmd.bCompareI(L"AddPLToQ")
              ||  strCmd.bCompareI(L"DeletePL")
              ||  strCmd.bCompareI(L"SetPLToQ"))
        {
            if (!bCheckParms(colParms, 2, strCmd))
                return tCQCKit::ECommResults::BadValue;

            if (!bCheckIsLocalGrp())
                return tCQCKit::ECommResults::ValueRejected;

            tCIDLib::TBoolean bByName;
            if (colParms[0].bCompareI(L"ByName"))
                bByName = kCIDLib::True;
            else if (colParms[0].bCompareI(L"ById"))
                bByName = kCIDLib::False;
            else
                return tCQCKit::ECommResults::BadValue;

            // If setting the queue, clear the queue first
            if (strCmd.bCompareI(L"SetPLToQ"))
                m_upnpsAVTrans.ClearQueue();

            //
            //  If by name, we have to search the list of playlists and find the
            //  named one.
            //
            tCIDLib::TCard4 c4PLNum;
            if (bByName)
            {
                if (!bFindNamedPL(colParms[1], c4PLNum))
                    return tCQCKit::ECommResults::ResNotFound;
            }
             else
            {
                if (!bValidatePLID(colParms[1], c4PLNum))
                    return tCQCKit::ECommResults::ResNotFound;
            }

            if (strCmd.bCompareI(L"DeletePL"))
            {
                // We need to delete a playlist
                m_strTmp1 = SonosZPS_DriverImpl::pszCont_StoredQ;
                m_strTmp1.AppendFormatted(c4PLNum);
                m_upnpsContDir.DeleteObject(m_strTmp1);
            }
             else
            {
                m_strTmp1 = SonosZPS_DriverImpl::pszURI_PLFile;
                m_strTmp1.AppendFormatted(c4PLNum);

                if (strCmd.bCompareI(L"SetPLToQ"))
                {
                    m_upnpsAVTrans.StartAutoplay
                    (
                        m_strTmp1, kCIDLib::True, kCIDLib::False, 0xFFFF
                    );
                }
                 else
                {
                    tCIDLib::TCard4 c4QdAt, c4NewCnt;
                    m_upnpsAVTrans.AddURIToQueue
                    (
                        m_strTmp1, TString::strEmpty(), c4QdAt, c4NewCnt
                    );
                }
            }
        }
         else if (strCmd.bCompareI(L"MonAnalogIn"))
        {
            // We need one parameter
            if (!bCheckParms(colParms, 1, strCmd))
                return tCQCKit::ECommResults::BadValue;

            //
            //  See if we are local. If not, then this is not legal for us,
            //  it should be sent to the group coordinator, so reject it.
            //
            if (!bCheckIsLocalGrp())
                return tCQCKit::ECommResults::ValueRejected;

            //
            //  Build up a stream from input URI for the UID of the target
            //  player, which could be us.
            //
            TString strURI = SonosZPS_DriverImpl::pszPref_StreamInCon;
            if (!colParms[0].bCompareI(strMoniker()))
            {
                //
                //  We are being asked to set up for someone else's analog
                //  input, so read his id.
                //
                TString strTargetUID;
                MonikerToUID(colParms[0], strTargetUID);
                strURI.Append(strTargetUID);
            }
             else
            {
                //
                //  It's us so just use our own device id, which we have
                //  in a member, so we don't have to read the field.
                //
                strURI.Append(m_strDevUID);
            }
            m_upnpsAVTrans.SetAVTransportURI(strURI, TString::strEmpty());
        }
         else if (strCmd.bCompareI(L"PlayFavorite"))
        {
            //
            //  Call a helper to do this since there's a good bit of work involved.
            //  We get the name of the favorites channel as the one parameter,
            //  and pass it in. He will use this to look up the info for the
            //  channel.
            //
            return ePlayFavorite(colParms[0]);
        }
         else if (strCmd.bCompareI(L"SaveQAsPL"))
        {
            // We need one parameter
            if (!bCheckParms(colParms, 1, strCmd))
                return tCQCKit::ECommResults::BadValue;

            TString strAssignedID;
            m_upnpsAVTrans.SaveQAsPlaylist(colParms[0], strAssignedID);
        }
         else
        {
            bKnownCmd = kCIDLib::False;
        }
    }

    if (!bKnownCmd)
    {
        facSonosZPPropS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kSonosZPErrs::errcFld_BadInvokeCmd
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , strCmd
        );
        return tCQCKit::ECommResults::BadValue;
    }
    return tCQCKit::ECommResults::Success;
}


//
//  Playing a favorite is sort of complicated. We first do a query of the favorites
//  container, asking for children. It will return us some XML that has info
//  on all of the favorites. We parse that, looking for the item that has the
//  indicated name.
//
//  Then we pull out the URI and the DIDL-Lite metadata content (reformatting
//  the latter back to text) and pass that back in to the SetAVTransport command.
//
tCQCKit::ECommResults TSonosZPPropSDriver::ePlayFavorite(const TString& strName)
{
    // Reload if we are out of date
    CheckFavsListReload();

    //
    //  Let's loop through our list and find one that has the indicated name. If
    //  we find it, we will try to start it.
    //
    const tCIDLib::TCard4 c4Count = m_colFavsList.c4ElemCount();
    tCIDLib::TCard4 c4Index = 0;
    while (c4Index < c4Count)
    {
        if (m_colFavsList[c4Index].strKey().bCompareI(strName))
            break;

        c4Index++;
    }

    if (c4Index < c4Count)
    {
        const TKeyValues& kvalsTar = m_colFavsList[c4Index];
        TString strMeta = kvalsTar.strVal2();

        //
        //  We need to un-encode the metadata. DO AMP FIRST, since it may be
        //  being used to doubly deref some charactres. If we do this one first
        //  then we correctly match the subsequent ones.
        //
        tCIDLib::TCard4 c4At = 0;
        strMeta.bReplaceSubStr(L"&amp;", L"&", c4At, kCIDLib::True, kCIDLib::False);

        c4At = 0;
        strMeta.bReplaceSubStr(L"&apos;", L"'", c4At, kCIDLib::True, kCIDLib::False);

        c4At = 0;
        strMeta.bReplaceSubStr(L"&quot;", L"\"", c4At, kCIDLib::True, kCIDLib::False);

        c4At = 0;
        strMeta.bReplaceSubStr(L"&lt;", L"<", c4At, kCIDLib::True, kCIDLib::False);

        c4At = 0;
        strMeta.bReplaceSubStr(L"&gt", L">", c4At, kCIDLib::True, kCIDLib::False);

        //
        //  If the URI indicates it's a container or playlist, then we need to do
        //  an add URI to queue, to queue up the things it contains. Else, we just
        //  set the AV transport URI directly.
        //
        const TString& strURI = kvalsTar.strVal1();
        if (strURI.bStartsWithI(SonosZPS_DriverImpl::pszPref_Container)
        ||  strURI.bStartsWithI(SonosZPS_DriverImpl::pszPref_Playlist))
        {
            tCIDLib::TCard4 c4QueuedAt;
            tCIDLib::TCard4 c4QueuedCnt;
            m_upnpsAVTrans.AddURIToQueue(strURI, strMeta, c4QueuedAt, c4QueuedCnt);
        }
         else
        {
            m_upnpsAVTrans.SetAVTransportURI(strURI, strMeta);
        }
    }

    return tCQCKit::ECommResults::Success;
}



//
//  We iterate through all the availble playlists or favorites and store them in our
//  local copy and store the lastest update id if we get a new one.
//
tCIDLib::TVoid TSonosZPPropSDriver::LoadFavoritesList()
{
    //
    //  Do an initial query, which will get us the count we need to do,
    //  and in most cases get them all.
    //
    tCIDLib::TCard4 c4CurInd = 0;
    tCIDLib::TCard4 c4Total;
    tCIDLib::TInt4  i4UpdateID;
    tCIDLib::TCard4 c4CurCnt = m_upnpsContDir.c4BrowseFavorites
    (
        c4CurInd
        , 20
        , c4Total
        , m_colFavsList
        , i4UpdateID
        , kCIDLib::False
    );

    // Add what we got to the index to start there next time
    c4CurInd += c4CurCnt;

    // Loop until we get the total reported
    while (c4CurInd < c4Total)
    {
        // Get another chunk
        c4CurCnt = m_upnpsContDir.c4BrowseFavorites
        (
            c4CurInd
            , 20
            , c4Total
            , m_colFavsList
            , i4UpdateID
            , kCIDLib::True
        );

        // And move the index up
        c4CurInd += c4CurCnt;

        // If we didn't get any this time, break out just to be safe
        if (!c4CurCnt)
            break;
    }

    // Store the last update id got for this guy
    m_i4LastFavsUpdateID = i4UpdateID;
}


tCIDLib::TVoid TSonosZPPropSDriver::LoadPlaylistsList()
{
    //
    //  Do an initial query, which will get us the count we need to do,
    //  and in most cases get them all.
    //
    tCIDLib::TCard4 c4CurInd = 0;
    tCIDLib::TCard4 c4Total;
    tCIDLib::TInt4  i4UpdateID;
    tCIDLib::TCard4 c4CurCnt = m_upnpsContDir.c4BrowseLists
    (
        SonosZPS_DriverImpl::pszCont_StoredQ
        , c4CurInd
        , 20
        , c4Total
        , m_colPLList
        , i4UpdateID
        , kCIDLib::False
    );

    // Add what we got to the index to start there next time
    c4CurInd += c4CurCnt;

    // Loop until we get the total reported
    while (c4CurInd < c4Total)
    {
        // Get another chunk
        c4CurCnt = m_upnpsContDir.c4BrowseLists
        (
            SonosZPS_DriverImpl::pszCont_StoredQ
            , c4CurInd
            , 20
            , c4Total
            , m_colPLList
            , i4UpdateID
            , kCIDLib::True
        );

        // And move the index up
        c4CurInd += c4CurCnt;

        // If we didn't get any this time, break out just to be safe
        if (!c4CurCnt)
            break;
    }

    // Store the last update id we got to both the active and last
    m_i4ActivePLLUpdateID = i4UpdateID;
    m_i4LastPLLUpdateID = i4UpdateID;
}



//
//  Goes to the indicated driver and reads the DevUID field from it and returns
//  that value.
//
tCIDLib::TVoid
TSonosZPPropSDriver::MonikerToUID(  const   TString&    strMoniker
                                    ,       TString&    strToFill) const
{
    tCQCKit::TCQCSrvProxy orbcSrv = facCQCKit().orbcCQCSrvAdminProxy(strMoniker);
    tCIDLib::TCard4     c4SerialNum = 0;
    tCQCKit::EFldTypes  eType;
    orbcSrv->bReadFieldByName(c4SerialNum, strMoniker, L"DevUID", strToFill, eType);
}


//
//  Register our fields and go back and look up the ids of those we need
//  to access often.
//
tCIDLib::TVoid TSonosZPPropSDriver::RegisterFields()
{
    //
    //  And set up our device fields. Most of our fields are those that
    //  a CQC media renderer driver has to provide, so we are using field
    //  names from the media facility.
    //
    TString strLimits;
    TVector<TCQCFldDef> colFlds(64);
    TCQCFldDef flddNew;

    const tCIDLib::TCh* const pszArtURL(L"ArtURL");
    const tCIDLib::TCh* const pszAVTransURI(L"AVTransportURI");
    const tCIDLib::TCh* const pszClearQ(L"ClearQ");
    const tCIDLib::TCh* const pszCrossfade(L"CrossfadeMode");
    const tCIDLib::TCh* const pszCurAlbum(L"CurAlbum");
    const tCIDLib::TCh* const pszCurArtist(L"CurArtist");
    const tCIDLib::TCh* const pszCurTrack(L"CurTrack");
    const tCIDLib::TCh* const pszCurTrackDur(L"CurTrackDur");
    const tCIDLib::TCh* const pszCurTrackNum(L"CurTrackNum");
    const tCIDLib::TCh* const pszCurTrackTime(L"CurTrackTime");
    const tCIDLib::TCh* const pszCurTrackURI(L"CurTrackURI");
//    const tCIDLib::TCh* const pszDevName(L"DevName");
    const tCIDLib::TCh* const pszDevUID(L"DevUID");
//    const tCIDLib::TCh* const pszIcon(L"Icon");
    const tCIDLib::TCh* const pszInvokeCmd(L"InvokeCmd");
//    const tCIDLib::TCh* const pszIsBridge(L"IsBridge");
    const tCIDLib::TCh* const pszMute(L"Mute");
    const tCIDLib::TCh* const pszPBPercent(L"PBPercent");
    const tCIDLib::TCh* const pszPlayMode(L"PlayMode");
//    const tCIDLib::TCh* const pszSetLED(L"SetLED");
    const tCIDLib::TCh* const pszShuffleMode(L"ShuffleMode");
    const tCIDLib::TCh* const pszTransport(L"Transport");
    const tCIDLib::TCh* const pszTransState(L"TransState");
    const tCIDLib::TCh* const pszVolume(L"Volume");
    const tCIDLib::TCh* const pszGroupCoord(L"GroupCoord");
    const tCIDLib::TCh* const pszIsLocalGrp(L"IsLocalGroup");

    flddNew.Set(pszArtURL, tCQCKit::EFldTypes::String, tCQCKit::EFldAccess::Read);
    colFlds.objAdd(flddNew);

    flddNew.Set(pszAVTransURI, tCQCKit::EFldTypes::String, tCQCKit::EFldAccess::Read);
    colFlds.objAdd(flddNew);

    flddNew.Set(pszClearQ, tCQCKit::EFldTypes::Boolean, tCQCKit::EFldAccess::Write);
    colFlds.objAdd(flddNew);

    flddNew.Set(pszCrossfade, tCQCKit::EFldTypes::Boolean, tCQCKit::EFldAccess::ReadWrite);
    colFlds.objAdd(flddNew);

    flddNew.Set(pszCurAlbum, tCQCKit::EFldTypes::String, tCQCKit::EFldAccess::Read);
    colFlds.objAdd(flddNew);

    flddNew.Set(pszCurArtist, tCQCKit::EFldTypes::String, tCQCKit::EFldAccess::Read);
    colFlds.objAdd(flddNew);

    flddNew.Set(pszCurTrack, tCQCKit::EFldTypes::String, tCQCKit::EFldAccess::Read);
    colFlds.objAdd(flddNew);

    flddNew.Set(pszCurTrackDur, tCQCKit::EFldTypes::Time, tCQCKit::EFldAccess::Read);
    colFlds.objAdd(flddNew);

    flddNew.Set(pszCurTrackNum, tCQCKit::EFldTypes::Card, tCQCKit::EFldAccess::Read);
    colFlds.objAdd(flddNew);

    flddNew.Set(pszCurTrackTime, tCQCKit::EFldTypes::Time, tCQCKit::EFldAccess::Read);
    colFlds.objAdd(flddNew);

    flddNew.Set(pszCurTrackURI, tCQCKit::EFldTypes::String, tCQCKit::EFldAccess::Read);
    colFlds.objAdd(flddNew);

//    flddNew.Set(pszDevName, tCQCKit::EFldTypes::String, tCQCKit::EFldAccess::Read);
//    colFlds.objAdd(flddNew);

    flddNew.Set(SonosZPS_DriverImpl::pszFld_DevUID, tCQCKit::EFldTypes::String, tCQCKit::EFldAccess::Read);
    colFlds.objAdd(flddNew);

//    flddNew.Set(SonosZPS_DriverImpl::pszFld_IPAddress, tCQCKit::EFldTypes::String, tCQCKit::EFldAccess::Read);
//    colFlds.objAdd(flddNew);

//    flddNew.Set(pszIcon, tCQCKit::EFldTypes::String, tCQCKit::EFldAccess::Read);
//    colFlds.objAdd(flddNew);

    flddNew.Set(pszInvokeCmd, tCQCKit::EFldTypes::String, tCQCKit::EFldAccess::Write);
    colFlds.objAdd(flddNew);

//    flddNew.Set(pszIsBridge, tCQCKit::EFldTypes::Boolean, tCQCKit::EFldAccess::Read);
//    colFlds.objAdd(flddNew);

//    flddNew.Set(SonosZPS_DriverImpl::pszFld_MACAddress, tCQCKit::EFldTypes::String, tCQCKit::EFldAccess::Read);
//    colFlds.objAdd(flddNew);

    flddNew.Set(pszGroupCoord, tCQCKit::EFldTypes::String, tCQCKit::EFldAccess::Read);
    colFlds.objAdd(flddNew);

    flddNew.Set(pszIsLocalGrp, tCQCKit::EFldTypes::Boolean, tCQCKit::EFldAccess::Read);
    colFlds.objAdd(flddNew);

    flddNew.Set
    (
        pszMute, tCQCKit::EFldTypes::Boolean, tCQCKit::EFldAccess::ReadWrite, tCQCKit::EFldSTypes::Mute
    );
    colFlds.objAdd(flddNew);

    flddNew.Set
    (
        pszPBPercent
        , tCQCKit::EFldTypes::Card
        , tCQCKit::EFldAccess::Read
        , L"Range:0, 100"
    );
    colFlds.objAdd(flddNew);

    tCIDUPnP::FormatEPlayModes(strLimits, L"Enum: ", kCIDLib::chComma);
    flddNew.Set
    (
        pszPlayMode, tCQCKit::EFldTypes::String, tCQCKit::EFldAccess::Read, strLimits
    );
    colFlds.objAdd(flddNew);

//    flddNew.Set(SonosZPS_DriverImpl::pszFld_SerialNum, tCQCKit::EFldTypes::String, tCQCKit::EFldAccess::Read);
//    colFlds.objAdd(flddNew);

//    flddNew.Set(pszSetLED, tCQCKit::EFldTypes::Boolean, tCQCKit::EFldAccess::Write);
//    colFlds.objAdd(flddNew);

//    flddNew.Set(SonosZPS_DriverImpl::pszFld_SoftwareVer, tCQCKit::EFldTypes::String, tCQCKit::EFldAccess::Read);
//    colFlds.objAdd(flddNew);


    flddNew.Set(pszShuffleMode, tCQCKit::EFldTypes::Boolean, tCQCKit::EFldAccess::Write);
    colFlds.objAdd(flddNew);

    // These require enum based limit strings. Do the transport field
    TString strLims = L"Enum: Pause, Play, Stop, Next, Prev";
    flddNew.Set
    (
        pszTransport
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Write
        , tCQCKit::EFldSTypes::MediaTransport
        , strLims
    );
    colFlds.objAdd(flddNew);

    // The current transport state
    tCIDUPnP::FormatETransStates(strLimits, L"Enum: ", kCIDLib::chComma);
    flddNew.Set
    (
        pszTransState, tCQCKit::EFldTypes::String, tCQCKit::EFldAccess::Read, strLimits
    );
    colFlds.objAdd(flddNew);

    flddNew.Set
    (
        pszVolume
        , tCQCKit::EFldTypes::Card
        , tCQCKit::EFldAccess::ReadWrite
        , tCQCKit::EFldSTypes::Volume
        , L"Range:0, 100"
    );
    colFlds.objAdd(flddNew);

    //
    //  Now register our fields with the base driver class, and go back
    //  and look up the ids.
    //
    SetAllErrStates();
    SetFields(colFlds);

    m_c4FldId_ArtURL = pflddFind(pszArtURL, kCIDLib::True)->c4Id();
    m_c4FldId_AVTransURI = pflddFind(pszAVTransURI, kCIDLib::True)->c4Id();
    m_c4FldId_ClearQ = pflddFind(pszClearQ, kCIDLib::True)->c4Id();
    m_c4FldId_CrossfadeMode = pflddFind(pszCrossfade, kCIDLib::True)->c4Id();
    m_c4FldId_CurAlbum = pflddFind(pszCurAlbum, kCIDLib::True)->c4Id();
    m_c4FldId_CurArtist = pflddFind(pszCurArtist, kCIDLib::True)->c4Id();
    m_c4FldId_CurTrack = pflddFind(pszCurTrack, kCIDLib::True)->c4Id();
    m_c4FldId_CurTrackDur = pflddFind(pszCurTrackDur, kCIDLib::True)->c4Id();
    m_c4FldId_CurTrackNum = pflddFind(pszCurTrackNum, kCIDLib::True)->c4Id();
    m_c4FldId_CurTrackTime = pflddFind(pszCurTrackTime, kCIDLib::True)->c4Id();
    m_c4FldId_CurTrackURI = pflddFind(pszCurTrackURI, kCIDLib::True)->c4Id();
//    m_c4FldId_DevName = pflddFind(pszDevName, kCIDLib::True)->c4Id();
//    m_c4FldId_Icon = pflddFind(pszIcon, kCIDLib::True)->c4Id();
    m_c4FldId_InvokeCmd = pflddFind(pszInvokeCmd, kCIDLib::True)->c4Id();
//    m_c4FldId_IsBridge = pflddFind(pszIsBridge, kCIDLib::True)->c4Id();
    m_c4FldId_Mute = pflddFind(pszMute, kCIDLib::True)->c4Id();
    m_c4FldId_PBPercent = pflddFind(pszPBPercent, kCIDLib::True)->c4Id();
    m_c4FldId_PlayMode = pflddFind(pszPlayMode, kCIDLib::True)->c4Id();
//    m_c4FldId_SetLED = pflddFind(pszSetLED, kCIDLib::True)->c4Id();
    m_c4FldId_ShuffleMode = pflddFind(pszShuffleMode, kCIDLib::True)->c4Id();
    m_c4FldId_Transport = pflddFind(pszTransport, kCIDLib::True)->c4Id();
    m_c4FldId_TransState = pflddFind(pszTransState, kCIDLib::True)->c4Id();
    m_c4FldId_Volume = pflddFind(pszVolume, kCIDLib::True)->c4Id();

    m_c4FldId_GroupCoord = pflddFind(pszGroupCoord, kCIDLib::True)->c4Id();
    m_c4FldId_IsLocalGroup = pflddFind(pszIsLocalGrp, kCIDLib::True)->c4Id();
}


//
//  A helper to wrap the release of a device, catch any exception and log
//  it.
//
tCIDLib::TVoid
TSonosZPPropSDriver::ReleaseDevice(         TUPnPDevice&        upnpdToRelease
                                    , const tCIDLib::TCh* const pszName)
{
    try
    {
        upnpdToRelease.Release();
    }

    catch(TError& errToCatch)
    {
        if (!errToCatch.bLogged() && (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium))
        {
            facSonosZPPropS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kSonosZPErrs::errcProto_ReleaseDev
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::CantDo
                , TString(pszName)
            );
        }
        LogError(errToCatch, tCQCKit::EVerboseLvls::Medium, CID_FILE, CID_LINE);
    }
}


//
//  A helper to wrap the release of a service, catch any exception and log
//  it.
//
tCIDLib::TVoid
TSonosZPPropSDriver::ReleaseService(        TUPnPService&       upnpsToRelease
                                    , const tCIDLib::TCh* const pszName)
{
    try
    {
        upnpsToRelease.Release();
    }

    catch(TError& errToCatch)
    {
        if (!errToCatch.bLogged() && (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium))
        {
            facSonosZPPropS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kSonosZPErrs::errcProto_ReleaseSrv
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::CantDo
                , TString(pszName)
            );
        }
        LogError(errToCatch, tCQCKit::EVerboseLvls::Medium, CID_FILE, CID_LINE);
    }
}


//
//  Store all the field info from the A/V Transport service. We can do two
//  different versions. One that gets the current media position info, and
//  one that doesn't. The former is called periodically as a poll every couple
//  seconds. The latter is called when we see that event variables have
//  changed.
//
tCIDLib::TVoid
TSonosZPPropSDriver::StoreAVTrans(const tCIDLib::TBoolean bGetCurTime)
{
    // Check if it's been marked dead, in which case we need to recycle
    CheckIsDead(m_upnpsAVTrans);

    //
    //  Store the serial number first, so that the info we get below must
    //  be for this serial number of beyond. Else we could miss something,
    //  at the risk of course of doing a redundant query if it changed right
    //  after we get it, but before we get the data values out below.
    //
    m_c4SerialNum_AVTrans = m_upnpsAVTrans.c4SerialNum();

    // Get the current transport state info
    tCIDUPnP::ETransStates  eTransState;
    tCIDLib::TCard4         c4Speed;
    m_upnpsAVTrans.QueryTransportInfo(eTransState, m_strTmp1, c4Speed);

    // Get current media position info
    tCIDLib::TEncodedTime   enctDuration = 0;
    tCIDLib::TCard4         c4TrackNum;
    tCIDLib::TEncodedTime   enctCurTime = 0;
    tCIDLib::TCard4         c4PerComplete = 0;
    if (bGetCurTime)
    {
        m_upnpsAVTrans.QueryPositionInfo
        (
            m_strTmp2, enctDuration, c4TrackNum, enctCurTime, c4PerComplete, &m_upnptiMeta
        );
    }
     else
    {
        m_upnpsAVTrans.QueryPositionInfo
        (
            m_strTmp2, enctDuration, c4TrackNum, &m_upnptiMeta
        );
    }

    // Get other info
    const tCIDLib::TBoolean bCrossfade = m_upnpsAVTrans.bCrossfadeMode();
    m_upnpsAVTrans.QueryAVTransportURI(m_strTmp1);

    bStoreStringFld(m_c4FldId_AVTransURI, m_strTmp1, kCIDLib::True);
    bStoreBoolFld(m_c4FldId_CrossfadeMode, bCrossfade, kCIDLib::True);
    bStoreStringFld(m_c4FldId_CurAlbum, m_upnptiMeta.strAlbum(), kCIDLib::True);
    bStoreStringFld(m_c4FldId_CurArtist, m_upnptiMeta.strArtist(), kCIDLib::True);
    bStoreStringFld(m_c4FldId_CurTrack, m_upnptiMeta.strTitle(), kCIDLib::True);
    bStoreTimeFld(m_c4FldId_CurTrackDur, enctDuration, kCIDLib::True);
    bStoreCardFld(m_c4FldId_CurTrackNum, c4TrackNum, kCIDLib::True);
    bStoreStringFld(m_c4FldId_CurTrackURI, m_strTmp2, kCIDLib::True);

    // Load the human readable text format of the play mode and transport state
    bStoreStringFld
    (
        m_c4FldId_PlayMode
        , tCIDUPnP::strXlatEPlayModes(m_upnpsAVTrans.ePlayMode())
        , kCIDLib::True
    );

    bStoreStringFld
    (
        m_c4FldId_TransState, tCIDUPnP::strXlatETransStates(eTransState), kCIDLib::True
    );


    // Store the current time stuff if we got it
    if (bGetCurTime)
    {
        bStoreTimeFld(m_c4FldId_CurTrackTime, enctCurTime, kCIDLib::True);
        bStoreCardFld(m_c4FldId_PBPercent, c4PerComplete, kCIDLib::True);
    }

    // If the art URL is different from what we last saw, then try to load it
    if (m_upnptiMeta.strArtURL() != m_strLastArtURL)
    {
        //
        //  If it's relative, then we have to expand it out to be relative to the
        //  the device URL.
        //
        if (!m_upnptiMeta.strArtURL().bIsEmpty()
        &&  (m_upnptiMeta.strArtURL().chFirst() == kCIDLib::chForwardSlash))
        {
            m_upnpdZP.QueryDeviceURL(m_strLastArtURL);
            m_strLastArtURL.Append(m_upnptiMeta.strArtURL());
        }
         else
        {
            // It's fully qualified to just take it as is
            m_strLastArtURL = m_upnptiMeta.strArtURL();
        }

        // Write the URL to the artwork URL field
        bStoreStringFld(m_c4FldId_ArtURL, m_strLastArtURL, kCIDLib::True);
    }
}


//
//  Store away any content directory info we keep up with. We are mainly
//  just watching for container update id changes, which we store to keep
//  up with whether any contaainers we are tracking have changed.
//
//  UNFORTUNATELY this doesn't wokr because the Windows UPNP framework does
//  not seem to report container update ids even though we turn on eventing
//  for the servivce.
//
//  This code is left here since it doesn't hurt. We just won't ever run
//  it because we won't ever see changes.
//
tCIDLib::TVoid TSonosZPPropSDriver::StoreContDir()
{
    // Check if it's been marked dead, in which case we need to recycle
    CheckIsDead(m_upnpsContDir);

    //
    //  Store the serial number first, so that the info we get below must
    //  be for this serial number of beyond. Else we could miss something,
    //  at the risk of course of doing a redundant query if it changed right
    //  after we get it, but before we get the data values out below.
    //
    m_c4SerialNum_ContDir = m_upnpsContDir.c4SerialNum();

    //
    //  This guy will be a comma separate list of containers and their
    //  latests update ids. This will clear it when we read it, so that
    //  we won't see these changes again. It will accumulate changes
    //  until our next read.
    //
    m_upnpsContDir.QueryContUpdateID(m_strTmp1);

    TStringTokenizer stokIDs(&m_strTmp1, L",");
    while (stokIDs.bGetNextToken(m_strTmp2))
    {
        // Break it half on the period
        if (m_strTmp2.bSplit(m_strTmp3, kCIDLib::chPeriod, kCIDLib::True))
        {
            //
            //  If the first part is something we care about, store the
            //  update id.
            //
            tCIDLib::TInt4 i4ID;
            if (m_strTmp3.bToInt4(i4ID, tCIDLib::ERadices::Dec))
            {
                if (m_strTmp2.bCompareI(L"SQ:"))
                    m_i4LastPLLUpdateID = i4ID;
                else if (m_strTmp2.bCompareI(L"Q:0"))
                    m_i4LastQUpdateID = i4ID;
            }
        }
    }
}


//
//  Store away device properties info. LED state is not evented so we make
//  that a write only field.
//
tCIDLib::TVoid TSonosZPPropSDriver::StoreDevProps()
{
    /*
    // Check if it's been marked dead, in which case we need to recycle
    CheckIsDead(m_upnpsDevProps);

    //
    //  Store the serial number first, so that the info we get below must
    //  be for this serial number of beyond. Else we could miss something,
    //  at the risk of course of doing a redundant query if it changed right
    //  after we get it, but before we get the data values out below.
    //
    m_c4SerialNum_DevProps = m_upnpsDevProps.c4SerialNum();

    // Query up all our data
    tCIDLib::TBoolean bIsBridge = m_upnpsDevProps.bIsBridge();
    m_upnpsDevProps.QueryZoneAttrs(m_strTmp1, m_strTmp2);

    bStoreBoolFld(m_c4FldId_IsBridge, bIsBridge, kCIDLib::True);
    bStoreStringFld(m_c4FldId_DevName, m_strTmp1, kCIDLib::True);
    bStoreStringFld(m_c4FldId_Icon, m_strTmp2, kCIDLib::True);
    */
}


// Store away group management info
tCIDLib::TVoid TSonosZPPropSDriver::StoreGrpMgmt()
{
    // Check if it's been marked dead, in which case we need to recycle
    CheckIsDead(m_upnpsGrpMgmt);

    //
    //  Store the serial number first, so that the info we get below must
    //  be for this serial number of beyond. Else we could miss something,
    //  at the risk of course of doing a redundant query if it changed right
    //  after we get it, but before we get the data values out below.
    //
    m_c4SerialNum_GrpMgmt = m_upnpsGrpMgmt.c4SerialNum();

    // Pull the values out to locals
    const tCIDLib::TBoolean bCoordLocal = m_upnpsGrpMgmt.bCoordIsLocal();
    m_upnpsGrpMgmt.QueryGrpCoordinator(m_strTmp1, m_strTmp2);

    bStoreBoolFld(m_c4FldId_IsLocalGroup, bCoordLocal, kCIDLib::True);
    bStoreStringFld(m_c4FldId_GroupCoord, m_strTmp1, kCIDLib::True);
}


// Store away render control info
tCIDLib::TVoid TSonosZPPropSDriver::StoreRendCtrl()
{
    // Check if it's been marked dead, in which case we need to recycle
    CheckIsDead(m_upnpsRendCtrl);


    //
    //  Store the serial number first, so that the info we get below must
    //  be for this serial number of beyond. Else we could miss something,
    //  at the risk of course of doing a redundant query if it changed right
    //  after we get it, but before we get the data values out below.
    //
    m_c4SerialNum_RendCtrl = m_upnpsRendCtrl.c4SerialNum();

    const tCIDLib::TBoolean bMute = m_upnpsRendCtrl.bQueryMute();
    const tCIDLib::TCard4 c4Vol = m_upnpsRendCtrl.c4QueryVolume();

    bStoreBoolFld(m_c4FldId_Mute, bMute, kCIDLib::True);
    bStoreCardFld(m_c4FldId_Volume, c4Vol, kCIDLib::True);
}


//
//  This is called during Connect and any time that we see that we have lost
//  one of our services. So find the devices and services we need.
//
tCIDLib::TVoid TSonosZPPropSDriver::UPnPSetup()
{
    const tCIDLib::ELoadRes eRes = SonosZPS_DriverImpl::pupnpafRend->eSetupDevice
    (
        m_strDevUID, m_upnpdZP, m_c4FinderSerialNum
    );

    if (eRes == tCIDLib::ELoadRes::NotFound)
    {
        facSonosZPPropS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kSonosZPErrs::errcProto_SetDevID
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::CantDo
        );
    }

    // Query the services of the main sone player device
    tCIDLib::TKVPList colServices;
    m_upnpdZP.QueryServices(colServices);

    /*
    facCIDUPnP().bSetServiceByType
    (
        colServices
        , m_upnpdZP
        , m_upnpsDevProps
        , kCIDUPnP::strSvcType_DeviceProperties
        , kCIDLib::True
    );
    */

    facCIDUPnP().bSetServiceByType
    (
        colServices
        , m_upnpdZP
        , m_upnpsGrpMgmt
        , kCIDUPnP::strSvcType_GroupManagement
        , kCIDLib::True
    );

    // Look up the zone player's child devices
    tCIDLib::TKValsList colChildDevs;
    m_upnpdZP.QueryChildDevices(colChildDevs, TString::strEmpty());

    // Find the media renderer child
    facCIDUPnP().bSetDeviceByType
    (
        colChildDevs
        , m_upnpdMedRend
        , kCIDUPnP::strDevType_MediaRenderer
        , kCIDLib::True
    );

    // Query his services
    m_upnpdMedRend.QueryServices(colServices);

    // Set up the services that are associted with the media renderer
    facCIDUPnP().bSetServiceByType
    (
        colServices
        , m_upnpdMedRend
        , m_upnpsAVTrans
        , kCIDUPnP::strSvcType_AVTransport
        , kCIDLib::True
    );

    facCIDUPnP().bSetServiceByType
    (
        colServices
        , m_upnpdMedRend
        , m_upnpsRendCtrl
        , kCIDUPnP::strSvcType_RendControl
        , kCIDLib::True
    );

    // Find the media server child
    facCIDUPnP().bSetDeviceByType
    (
        colChildDevs
        , m_upnpdMedSrv
        , kCIDUPnP::strDevType_MediaServer
        , kCIDLib::True
    );

    // Query his services
    m_upnpdMedSrv.QueryServices(colServices);

    // And set up the services related to the media server
    facCIDUPnP().bSetServiceByType
    (
        colServices
        , m_upnpdMedSrv
        , m_upnpsContDir
        , kCIDUPnP::strSvcType_ContentDirectory
        , kCIDLib::True
    );
}



//
//  Validates a playlist id (in the form SQ:xx, where xx is a number. If it
//  is valid, it returns the number value.
//
tCIDLib::TBoolean
TSonosZPPropSDriver::bValidatePLID( const   TString&            strPLID
                                    ,       tCIDLib::TCard4&    c4PLNum) const
{
    // Make sure it's a valid playlist id and extract the playlist number
    TString strPrefix(strPLID);
    TString strPLNum;
    if (!strPrefix.bSplit(strPLNum, L':')
    ||  (strPrefix.eCompareI(L"SQ") != tCIDLib::ESortComps::Equal)
    ||  !strPLNum.bToCard4(c4PLNum, tCIDLib::ERadices::Dec))
    {
        return kCIDLib::False;
    }
    return kCIDLib::True;
}

