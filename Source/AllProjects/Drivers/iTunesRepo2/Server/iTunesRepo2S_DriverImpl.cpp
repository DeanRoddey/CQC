//
// FILE NAME: iTunesRepo2S_DriverImpl.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/20/2009
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
#include    "iTunesRepo2S.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TiTunesRepo2SDriver,TCQCStdMediaRepoDrv)



// ---------------------------------------------------------------------------
//   CLASS: iTunesRepoSDriver
//  PREFIX: drv
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  iTunesRepoSDriver: Constructors and Destructor
// ---------------------------------------------------------------------------
TiTunesRepo2SDriver::TiTunesRepo2SDriver(const TCQCDriverObjCfg& cqcdcToLoad) :

    TCQCStdMediaRepoDrv(cqcdcToLoad, &m_srdbEngine)
    , m_c4FldId_Loaded(kCIDLib::c4MaxCard)
    , m_c4FldId_ReloadDB(kCIDLib::c4MaxCard)
    , m_c4FldId_Status(kCIDLib::c4MaxCard)
    , m_srdbEngine(cqcdcToLoad.strMoniker(), tCQCMedia::EMTFlags::Music)
{
}

TiTunesRepo2SDriver::~TiTunesRepo2SDriver()
{
}


// ---------------------------------------------------------------------------
//  TiTunesRepo2SDriver: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TiTunesRepo2SDriver::bGetCommResource(TThread&)
{
    // We have no persistent comm resource, so just say yes
    return kCIDLib::True;
}


tCIDLib::TBoolean TiTunesRepo2SDriver::bWaitConfig(TThread&)
{
    //
    //  We asynchronously load the database. So we will make multiple
    //  passes into here. The first time, we try to start the loading
    //  process, and on subsequent entries, we check the status of the
    //  load.
    //
    tCIDLib::TBoolean bRet = kCIDLib::False;
    if (m_pdblRepo)
    {
        try
        {
            bRet = bMonitorLoad();
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            LogError(errToCatch, tCQCKit::EVerboseLvls::Medium);
            if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
            {
                facCQCMedia().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , L"An exception proporated out of the load monitor"
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::AppStatus
                    , strMoniker()
                );
            }
        }

        catch(...)
        {
            if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
            {
                facCQCMedia().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , L"An unknown exception proporated out of the load monitor"
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::AppStatus
                    , strMoniker()
                );
            }
        }
    }
     else
    {
        // No outstanding load operation, so try one
        StartLoad();
    }
    return bRet;
}


tCQCKit::ECommResults
TiTunesRepo2SDriver::eBoolFldValChanged( const   TString&            strName
                                        , const tCIDLib::TCard4     c4FldId
                                        , const tCIDLib::TBoolean   bNewValue)
{
    if (c4FldId == m_c4FldId_ReloadDB)
    {
        // If not already loading, then do a load
        if (!m_pdblRepo)
            StartLoad();
    }
     else
    {
        IncUnknownWriteCounter();
        return tCQCKit::ECommResults::FieldNotFound;
    }
    return tCQCKit::ECommResults::Success;
}


tCQCKit::ECommResults TiTunesRepo2SDriver::eConnectToDevice(TThread& thrCaller)
{
    // We don't have a device to connect to
    return tCQCKit::ECommResults::Success;
}


tCQCKit::EDrvInitRes TiTunesRepo2SDriver::eInitializeImpl()
{
    //
    //  Make sure that we were configured for a socket connection. Otherwise,
    //  its a bad configuration.
    //
    const TCQCDriverObjCfg& cqcdcOurs = cqcdcThis();
    if (cqcdcOurs.conncfgReal().clsIsA() != TCQCIPConnCfg::clsThis())
    {
        // Note that we are throwing a CQCKit error here!
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcDrv_BadConnCfgType
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Config
            , cqcdcOurs.conncfgReal().clsIsA()
            , clsIsA()
            , TCQCIPConnCfg::clsThis()
        );
    }

    //
    //  Its the right type so do the down cast and get the data out of it
    //  that we need.
    //
    m_conncfgSock = static_cast<const TCQCIPConnCfg&>(cqcdcOurs.conncfgReal());

    //
    //  Set the reconnect and poll times to 1 second. We don't poll any
    //  device or have any device to connect to, but we do watch for the
    //  DB loader to finish and want to respond to that quickly.
    //
    SetPollTimes(1000, 1000);

    //
    //  And set up our device fields. The bulk of what this driver does is
    //  via the media interface, and this driver doesn't control any device
    //  or app, so there are just a few.
    //
    TVector<TCQCFldDef> colFlds(8);
    TCQCFldDef flddNew;

    const tCIDLib::TCh* const pszFldLoaded(L"Loaded");
    const tCIDLib::TCh* const pszFldReloadDB(L"ReloadDB");
    const tCIDLib::TCh* const pszFldStatus(L"Status");

    flddNew.Set
    (
        pszFldLoaded
        , tCQCKit::EFldTypes::Boolean
        , tCQCKit::EFldAccess::Read
    );
    colFlds.objAdd(flddNew);

    flddNew.Set
    (
        pszFldReloadDB
        , tCQCKit::EFldTypes::Boolean
        , tCQCKit::EFldAccess::Write
    );
    flddNew.bAlwaysWrite(kCIDLib::True);
    colFlds.objAdd(flddNew);

    flddNew.Set
    (
        pszFldStatus
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Read
    );
    colFlds.objAdd(flddNew);

    //
    //  Now register our fields with the base driver class, and go back
    //  and look up the ids.
    //
    SetFields(colFlds);
    m_c4FldId_Loaded = pflddFind(pszFldLoaded, kCIDLib::True)->c4Id();
    m_c4FldId_ReloadDB = pflddFind(pszFldReloadDB, kCIDLib::True)->c4Id();
    m_c4FldId_Status = pflddFind(pszFldStatus, kCIDLib::True)->c4Id();

    //
    //  Init the status field to indicate not loaded. The loaded field will
    //  have a default initial value of false, which is what we want.
    //
    bStoreStringFld
    (
        m_c4FldId_Status
        , faciTunesRepo2S().strMsg(kiTunes2Msgs::midStatus_NotLoaded)
        , kCIDLib::True
    );

    // We'll try to get a first load of the database before we come online
    return tCQCKit::EDrvInitRes::WaitConfig;
}


tCQCKit::ECommResults TiTunesRepo2SDriver::ePollDevice(TThread& thrCaller)
{
    //
    //  If a database loader is active, then we are loading right now, so
    //  call the load monitoring method.
    //
    if (m_pdblRepo)
        bMonitorLoad();
    return tCQCKit::ECommResults::Success;
}



tCIDLib::TVoid TiTunesRepo2SDriver::ReleaseCommResource()
{
    // Nothing to release
}


tCIDLib::TVoid TiTunesRepo2SDriver::TerminateImpl()
{
    // If a load is going on, then stop it
    if (m_pdblRepo)
        m_pdblRepo->Shutdown();

    try
    {
        delete m_pdblRepo;
        m_pdblRepo = 0;
    }

    catch(...)
    {
    }

    // Tell the database engine to flush the media database
    m_srdbEngine.ResetMediaDB();
}


// ---------------------------------------------------------------------------
//  TiTunesRepo2SDriver: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  This is called when a load is underway. We watch the load status and
//  handle with it finishes.
//
tCIDLib::TBoolean TiTunesRepo2SDriver::bMonitorLoad()
{
    // We have a load going, so let's check the status
    const tCQCMedia::ELoadStatus eStatus = m_pdblRepo->eStatus(m_strStatus);

    tCIDLib::TBoolean bRet = kCIDLib::False;
    if (eStatus > tCQCMedia::ELoadStatus::Loading)
    {
        try
        {
            // Make sure the repo thread is down and out
            m_pdblRepo->Shutdown();

            // If it failed, log that
            if (eStatus == tCQCMedia::ELoadStatus::Failed)
            {
                // Set the status field to not loaded and leave loaded field false
                {
                    bStoreStringFld
                    (
                        m_c4FldId_Status
                        , faciTunesRepo2S().strMsg(kiTunes2Msgs::midStatus_NotLoaded)
                        , kCIDLib::True
                    );
                }

                if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
                {
                    facCQCMedia().LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , kMedErrs::errcDB_LoadFailed
                        , tCIDLib::ESeverities::Status
                        , tCIDLib::EErrClasses::AppStatus
                        , strMoniker()
                    );
                }
                bRet = kCIDLib::False;
            }
             else
            {
                //
                //  Set the status field to loaded and the loaded field to
                //  true.
                //
                bStoreStringFld
                (
                    m_c4FldId_Status
                    , faciTunesRepo2S().strMsg(kiTunes2Msgs::midStatus_Loaded)
                    , kCIDLib::True
                );
                bStoreBoolFld(m_c4FldId_Loaded, kCIDLib::True, kCIDLib::True);

                // Give the loader a chance to wrap up
                m_pdblRepo->CompleteLoad();
                bRet = kCIDLib::True;
            }
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);

            // Tell the parent class we are done loading
            bLoading(kCIDLib::False);
            delete m_pdblRepo;
            m_pdblRepo = 0;
            throw;
        }

        // Tell the parent class we are done loading
        bLoading(kCIDLib::False);
        delete m_pdblRepo;
        m_pdblRepo = 0;
    }
     else
    {
        // Not done yet, so keep waiting, but update the status fields
        bStoreStringFld(m_c4FldId_Status, m_strStatus, kCIDLib::True);
    }
    return bRet;
}


tCIDLib::TVoid TiTunesRepo2SDriver::StartLoad()
{
    //
    //  Build up the path we'll tell it to write to. We create a file
    //  in the CQCData\Server\Data\ path. We'll add to that
    //
    //      \iTunesRepo2\TrayDump_[moniker]\
    //
    //  i.e. it'll include this driver's moniker so that if we have
    //  multiple instances of the driver they won't conflict.
    //
    TPathStr pathXML(facCQCKit().strServerDataDir());
    pathXML.AddLevel(L"iTunesRepo2");
    pathXML.AddLevel(L"TrayDump_");
    pathXML.Append(strMoniker());
    TFileSys::MakePath(pathXML);

    // The tray app wants a final separator
    pathXML.AddTrailingSeparator();

    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
    {
        facCQCMedia().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kMedMsgs::midStatus_StartingLoad
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
            , pathXML
        );
    }

    //
    //  Create a loader. Set our verbosity level on it so that it will
    //  do the appropriate logging. Once we get the new loader set,
    //  then that's the signal that we are in loading mode, so we can
    //  go ahead and set up other stuff before starting the loading
    //  process, sine we know we'll get back out of loading mode even
    //  if we were to fail before we finish this process, since the
    //  poll callback watchs this member, and the initial state is
    //  set to error status until we start it up.
    //
    m_pdblRepo = new TiTunesRepo2Loader(strMoniker(), pathXML, this);
    m_pdblRepo->eVerboseLevel(eVerboseLevel());

    //
    //  Set the loading flag on the parent class, so that he can
    //  reject incoming requests at this point, since the loading
    //  process will be updating the database.
    //
    bLoading(kCIDLib::True);

    //
    //  Make sure that our fields that represent the load state are
    //  set to their default values since if we are waiting for config,
    //  we can't have any titles loaded yet.
    //
    {
        bStoreBoolFld(m_c4FldId_Loaded, kCIDLib::True, kCIDLib::True);
        bStoreStringFld
        (
            m_c4FldId_Status
            , faciTunesRepo2S().strMsg(kiTunes2Msgs::midStatus_NotLoaded)
            , kCIDLib::True
        );
    }

    // And finally start the loading process
    m_pdblRepo->StartLoad(&m_srdbEngine.mdbInfo(), m_conncfgSock.ipepConn());
}


