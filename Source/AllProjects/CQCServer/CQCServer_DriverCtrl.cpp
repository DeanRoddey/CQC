//
// FILE NAME: CQCServer_DriverCtrl.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 05/07/2001
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
//  This file implements the the parts of the facility class which are related
//  to loading and unloading drivers. This is just to keep the main facility
//  file from getting to big and messy.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCServer.hpp"


// ---------------------------------------------------------------------------
//  TFacCQCServer: Public, non-virtual methods
// ---------------------------------------------------------------------------


//
//  This method is called from our admin implementation object. It just delegates remote
//  driver add calls to us. Mostly all we do is to call the private method that does all
//  the work of adding drivers. We loop through the passed monikers and, for each one, we
//  query the config for it from the MS' config server, and we call the helper that loads
//  the driver.
//
//  The drivers will register themselves with the name server once they get loaded.
//
tCIDLib::TBoolean
TFacCQCServer::bRemoteLoadDrivers(  const   tCIDLib::TStrList&  colMonikers
                                    , const TCQCSecToken&       sectUser)
{
    // We require at least system admin rights
    facCQCKit().TestPrivileges(sectUser, tCQCKit::EUserRoles::SystemAdmin);

    // WE need an installation server proxy or can't do any of this
    tCQCKit::TInstSrvProxy orbcIS = facCQCKit().orbcInstSrvProxy();

    tCIDLib::TCard4 c4Failures = 0;
    TCQCDriverObjCfg cqcdcCur;
    TCQCDriverCfg cqcdcLatest;
    TString strMsg;
    const tCIDLib::TCard4 c4Count = colMonikers.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TString& strCurMon = colMonikers[c4Index];

        try
        {
            //
            //  Get the configuration for this driver, throwing if it's not found.
            //
            TString strHost;
            {
                tCIDLib::TCard4 c4Ver = 0;
                if (orbcIS->bDrvCfgQueryConfig(c4Ver, strCurMon, strHost, cqcdcCur, cuctxToUse().sectUser()))
                {
                    //
                    //  Query the original manifest info and see if there are any changes.
                    //  If so, then update the base part of our configuration.
                    //
                    if (orbcIS->bQueryDrvManifest(cqcdcCur.strMake(), cqcdcCur.strModel(), cqcdcLatest))
                    {
                        if (!cqcdcCur.bSameBase(cqcdcLatest))
                        {
                            // Delete any current driver files to make us download them again
                            DeleteDrvFiles(cqcdcCur);

                            // And now update the base content
                            cqcdcCur.UpdateBase(cqcdcLatest);
                        }
                    }
                     else
                    {
                        // Log that we couldn't refresh the driver config
                        LogMsg
                        (
                            CID_FILE
                            , CID_LINE
                            , kCQCSMsgs::midStatus_DrvCfgRefresh
                            , tCIDLib::ESeverities::Status
                            , tCIDLib::EErrClasses::AppStatus
                            , cqcdcCur.strMoniker()
                            , cqcdcCur.strMake()
                            , cqcdcCur.strModel()
                            , cqcdcCur.strVersion()
                        );
                    }
                }
            }

            // Just in case, make sure it came back configured for our host
            CIDAssert
            (
                strHost.bCompareI(TSysInfo::strIPHostName())
                , L"The driver is not configured for this CQC host"
            );

            // Just in case, make sure we don't already have a driver with this moniker
            {
                TLocker lockrSync(&m_mtxLock);
                tCIDLib::TCard4 c4DrvIndex;
                if (psdiFindDrv(strCurMon, c4DrvIndex, kCIDLib::False))
                {
                    facCQCServer.ThrowErr
                    (
                        CID_FILE
                        , CID_LINE
                        , kCQCSErrs::errcInst_AlreadyLoaded
                        , tCIDLib::ESeverities::Failed
                        , tCIDLib::EErrClasses::Already
                        , strCurMon
                    );
                }
            }

            // Load up this driver
            LoadADriver(cqcdcCur);

            //
            //  If in verbose logging mode, then log that a remote client is
            //  forcing a driver load.
            //
            if (bLogStatus())
            {
                LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kCQCSMsgs::midStatus_RemoteDrvLoad
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::AppStatus
                    , cqcdcCur.strMoniker()
                    , cqcdcCur.strMake()
                    , cqcdcCur.strModel()
                    , cqcdcCur.strVersion()
                );
            }
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);

            LogMsg
            (
                CID_FILE
                , CID_LINE
                , kCQCSErrs::errcDrv_DriverLoadErr
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
                , strCurMon
            );
            c4Failures++;
        }
    }
    return (c4Failures == 0);
}


//
//  Does a pause/resume operation on the indicated driver. This will put it into
//  paused state if pausing, which will leave it in place but inactive (and having
//  released its comm resource and such.) This command is driven by the client
//  side which will persist the configuration change first before telling us to
//  do it.
//
//  The driver will register/unregister itself from the name server once it processes
//  the command.
//
tCIDLib::TVoid
TFacCQCServer::PauseResumeDrv(  const   TString&                strMoniker
                                , const tCIDLib::TBoolean       bPause
                                , const TCQCSecToken&           sectUser
                                , const tCQCKit::EDrvCmdWaits   eWait)
{
    // We require at least power user rights
    facCQCKit().TestPrivileges(sectUser, tCQCKit::EUserRoles::PowerUser);

    LogMsg
    (
        CID_FILE
        , CID_LINE
        , kCQCSMsgs::midStatus_PauseDrv
        , tCIDLib::ESeverities::Status
        , tCIDLib::EErrClasses::AppStatus
        , bPause ? TString(L"pause") : TString(L"resume")
        , strMoniker
    );

    //
    //  Lock and get the command queued up. Actually we'll check the state of the
    //  driver and if it is already in the requested state, we won't do anything.
    //
    TCQCServerBase::TDrvCmd* pdcmdWait = nullptr;
    TCQCDriverObjCfg cqcdcTar;
    {
        TLocker lockrSync(&m_mtxLock);

        // Look up the driver, throw if not found
        tCIDLib::TCard4 c4Index;
        TServerDriverInfo* psdiTar = psdiFindDrv(strMoniker, c4Index);

        // Queue up the command if the state is changed
        if ((bPause && (psdiTar->sdrvDriver().eState() != tCQCKit::EDrvStates::Paused))
        ||  (!bPause && (psdiTar->sdrvDriver().eState() == tCQCKit::EDrvStates::Paused)))
        {
            pdcmdWait = psdiTar->sdrvDriver().pdcmdQPauseResumeCmd(bPause, eWait);
        }
    }

    // Wait for it to complete if needed. Tell it to release the cmd for us
    if (pdcmdWait)
    {
        TError errFail;
        if (!TCQCServerBase::bWaitCmd(pdcmdWait, errFail, kCIDLib::True))
            throw(errFail);
    }
}



//
//  This is called from our admin server implementation object, which it just
//  passes on to us. That is called by the admin client, after a user does a
//  'reconfigure driver' operation. The client will have already stored the
//  configuration change to the master server, and just passes us the new stuff
//  for us to update live.
//
//  This one is always wait mode, so the caller doesn't get to choose. We
//  need to wait till the driver accepts the config, so that we can update
//  our own configuration and write it back out.
//
tCIDLib::TVoid
TFacCQCServer::ReconfigDriver(const TString& strMoniker, const TCQCSecToken& sectUser)
{
    // We require at least power user rights
    facCQCKit().TestPrivileges(sectUser, tCQCKit::EUserRoles::PowerUser);

    //
    //  Get the (presumably updated) config for this driver, throwing if it's
    //  not found.
    //
    TString strHost;
    TCQCDriverObjCfg cqcdcNew;
    {
        tCQCKit::TInstSrvProxy orbcIS = facCQCKit().orbcInstSrvProxy();

        tCIDLib::TCard4 c4Ver = 0;
        orbcIS->bDrvCfgQueryConfig(c4Ver, strMoniker, strHost, cqcdcNew, cuctxToUse().sectUser());
    }

    // Just in case, make sure it came back configured for our host
    CIDAssert
    (
        strHost.bCompareI(TSysInfo::strIPHostName())
        , L"The driver is not configured for this CQC host"
    );

    // We found that, so see if we can send the changes to the driver
    TCQCServerBase::TDrvCmd* pdcmdWait = 0;
    {
        TLocker lockrSync(&m_mtxLock);

        // Make sure we have a driver with this moniker, throwing if not
        tCIDLib::TCard4 c4DrvIndex;
        TServerDriverInfo* psdiTar = psdiFindDrv(strMoniker, c4DrvIndex);

        pdcmdWait = psdiTar->sdrvDriver().pdcmdQReconfig
        (
            cqcdcNew, tCQCKit::EDrvCmdWaits::Wait
        );
    }

    // We always wait for this one, tell it to release for us when done
    TError errFail;
    if (!TCQCServerBase::bWaitCmd(pdcmdWait, errFail, kCIDLib::True))
        throw(errFail);

    // If in verbose logging mode, log that we completed it ok
    LogMsg
    (
        CID_FILE
        , CID_LINE
        , kCQCSMsgs::midStatus_ReconfigDrvDone
        , tCIDLib::ESeverities::Status
        , tCIDLib::EErrClasses::AppStatus
        , strMoniker
    );
}


//
//  This method allows a client to make us unload all our drivers. We just call the same
//  method that is called normally on shutdown. This is done async, he just unbinds the
//  drivers, then sets a flag on them to start them shutting down, then waits for them
//  to complete.
//
//  The drivers themselves also unbind from the name server, but the private helper will
//  force them all out up front, which is best for us, just to make sure, because likely
//  these drivers are being moved to a new machine, and will be starting up, so we want to
//  be sure there's no conflict.
//
tCIDLib::TVoid TFacCQCServer::RemoteUnloadAllDrivers(const TCQCSecToken& sectUser)
{
    // We require at least system admin rights
    facCQCKit().TestPrivileges(sectUser, tCQCKit::EUserRoles::SystemAdmin);

    LogMsg
    (
        CID_FILE
        , CID_LINE
        , kCQCSMsgs::midStatus_RemoteUnloadAll
        , tCIDLib::ESeverities::Status
        , tCIDLib::EErrClasses::AppStatus
    );

    //
    //  We have to lock in this case, so that no one else can get in while we the drivers
    //  are being unloaded.
    //
    try
    {
        TLocker lockrSync(&m_mtxLock);
        UnloadDrivers();
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);
    }

    LogMsg
    (
        CID_FILE
        , CID_LINE
        , kCQCSMsgs::midStatus_RemoteUnloadAllDone
        , tCIDLib::ESeverities::Status
        , tCIDLib::EErrClasses::AppStatus
    );

}


//
//  This method is called from our ORB admin interface, when a client asks to unload
//  a driver. All we do is check the passed security credentials to make sure that
//  this is legal.
//
//  If so, then we set the shutdown flag on the driver and return. The driver will
//  start the shutdown process. Later the driver will be removed when it hits
//  terminated state. It will undregister itself from the name server.
//
tCIDLib::TVoid
TFacCQCServer::RemoteUnloadDriver(const TString& strMoniker, const TCQCSecToken& sectUser)
{
    // We require at least system admin rights
    facCQCKit().TestPrivileges(sectUser, tCQCKit::EUserRoles::SystemAdmin);

    // Look and see if we have this driver. if so, start the shutdown process
    {
        TLocker lockrSync(&m_mtxLock);

        tCIDLib::TCard4 c4DrvIndex;
        TServerDriverInfo* psdiToRemove = psdiFindDrv
        (
            strMoniker, c4DrvIndex, kCIDLib::False
        );

        // If not find, throw that error
        if (!psdiToRemove)
        {
            facCQCKit().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kKitErrs::errcDrv_NotFound
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::NotFound
                , strMoniker
                , TSysInfo::strIPHostName()
            );
        }

        psdiToRemove->StartShutdown();

        // Log that the unload process is started
        if (bLogStatus())
        {
            LogMsg
            (
                CID_FILE
                , CID_LINE
                , kCQCSMsgs::midStatus_RemoteDrvUnload
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
                , strMoniker
            );
        }
    }
}


// ---------------------------------------------------------------------------
//  TFacCQCServer: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  The scavenger thread will call this periodically. We remove any drivers that
//  are dead. We go through the config and remove any that are down, and and clean
//  them up.
//
//  Keep in mind that juts because the driver state is terminated, that doesn't
//  mean the driver is actually all the way down. So we first check for terminated
//  state, and then check to see if the thread is all the way down, though we use
//  a zero timeout so that we won't block if not. If not, we just wait till the next
//  time to get that driver.
//
tCIDLib::TVoid TFacCQCServer::UnloadDeadDrivers()
{
    // Now scan for dead drivers, remember if we removed any
    tCIDLib::TBoolean bDriversUnloaded = kCIDLib::False;
    {
        TLocker lockrSync(&m_mtxLock);

        // If the driver list is already empty, give up now
        tCIDLib::TCard4 c4Count = m_colDriverList.c4ElemCount();
        if (!c4Count)
            return;

        // Else iterate through them
        tCIDLib::TCard4 c4Index = 0;
        TString strMoniker;
        while (c4Index < c4Count)
        {
            TServerDriverInfo& sdiCur = *m_colDriverList[c4Index];

            if (sdiCur.bIsDead(0))
            {
                // Remember we unloaded at least one
                bDriversUnloaded = kCIDLib::True;

                // Get a copy of the moniker before we toast this one
                strMoniker = sdiCur.sdrvDriver().strMoniker();

                //
                //  Tell the driver info object to clean up. This guy won't
                //  throw any errors.
                //
                sdiCur.DropDriver();

                // Remove it from our active driver list
                m_colDriverList.RemoveAt(c4Index);
                c4Count--;

                // Log that we've unloaded this one
                if (bLogInfo())
                {
                    LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , kCQCSMsgs::midStatus_RemoteDrvUnloaded
                        , tCIDLib::ESeverities::Info
                        , tCIDLib::EErrClasses::AppStatus
                        , strMoniker
                    );
                }
            }
             else
            {
                c4Index++;
            }
        }

        // If we removed any, bump the driver id list
        if (bDriversUnloaded)
            m_c4DriverListId++;
    }
}



tCIDLib::TVoid
TFacCQCServer::DeleteDrvFiles(const TCQCDriverObjCfg&)
{
    // <TBD> Get rid of the downloaded files for the passed driver
}


//
//  This handles the work of actually getting a driver basically ready, put into our
//  list, and starting the driver up to begin processing (which will also cause it to
//  register itself with the name server.)
//
//  If the caller tells us to, we update our in memory config data. This is only
//  when a remote client is forcing a load, not during the initial loading of
//  drivers on startup, which uses already set config.
//
tCIDLib::TVoid
TFacCQCServer::LoadADriver(const TCQCDriverObjCfg& cqcdcToLoad)
{
    //
    //  Build up the full path to this driver facility
    //
    //  If it's a macro based driver, we have to load the generic driver, else
    //  we use the name in the config. We also cannot use the version numbers, since
    //  those are for the macro driver, not the generic driver.
    //
    const tCIDLib::TCard4 c4MajVer(kCQCKit::c4MajVersion);
    const tCIDLib::TCard4 c4MinVer(kCQCKit::c4MinVersion);
    TString strBaseLibName;
    if (cqcdcToLoad.eType() == tCQCKit::EDrvTypes::CppLib)
        strBaseLibName = cqcdcToLoad.strServerLibName();
    else if (cqcdcToLoad.eType() == tCQCKit::EDrvTypes::GenProto)
        strBaseLibName = facCQCKit().strProtoDriverLibName();
    else
        strBaseLibName = facCQCKit().strGenDriverLibName();

    TPathStr pathPortName;
    TPathStr pathLoadName;
    TModule::BuildModName
    (
        strBaseLibName
        , c4MajVer
        , c4MinVer
        , tCIDLib::EModTypes::SharedLib
        , pathPortName
        , pathLoadName
    );

    //
    //  Since we don't know ahead of time here whether there is a loadable message text
    //  file, we have to just build the file name and see if it is there. If so, we set
    //  up the appropriate flags.
    //
    tCIDLib::EModFlags eModFlags = tCIDLib::EModFlags::None;
    TPathStr pathMsgs(facCQCKit().strBinDir());
    pathMsgs.AddLevel(pathPortName);
    pathMsgs.Append(L"_*");
    pathMsgs.AppendExt(L"CIDMsg");
    if (TFileSys::bExists(pathMsgs))
        eModFlags |= tCIDLib::EModFlags::HasMsgFile;

    //
    //  And try to load it as a facility object. This will do a load of it, which will bump
    //  the reference count, so that it doesn't go away if we have more than one driver
    //  that uses the same facility and we drop one of them.
    //
    //  We don't need a path, it'll be in the same directory as our exe, which means
    //  we don't have to do anything special for driver dev IDE vs production system.
    //
    TFacility* pfacDriver = new TFacility
    (
        strBaseLibName
        , tCIDLib::EModTypes::SharedLib
        , c4MajVer
        , c4MinVer
        , 0
        , eModFlags
        , kCIDLib::True
    );
    TJanitor<TFacility> janFac(pfacDriver);

    //
    //  Use the facility object to load up the exported driver factory
    //  function, and ask it to create an instance of its driver for us.
    //
    TCQCServerBase* psdrvNew = nullptr;
    try
    {
        tCQCKit::TSrvDriverFuncPtr pfnDriverFunc = tCQCKit::TSrvDriverFuncPtr
        (
            pfacDriver->pfnGetFuncPtr(kCQCKit::pszSrvFunc)
        );

        //
        //  IT IS IMPORTANT that we call the version that takes the configuration.
        //  Driver pause/resume is persistent and stored in the config. We want
        //  the driver to get to the correct initial state, and he will look at
        //  the paused flag to do that. Otherwise, we'd have to start the driver
        //  before he was see the config (in the init callback.)
        //
        psdrvNew = (pfnDriverFunc)(cqcdcToLoad);

        // Give it a copy of our user context/security credentials
        psdrvNew->SetUC(cuctxToUse());
    }

    catch(TError& errToCatch)
    {
        if (bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        if (bLogFailures())
        {
            LogMsg
            (
                CID_FILE
                , CID_LINE
                , kCQCSMsgs::midStatus_FactoryFuncErr
                , cqcdcToLoad.strServerLibName()
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::AppStatus
                , cqcdcToLoad.strMoniker()
            );
        }
        throw;
    }

    catch(...)
    {
        if (bLogFailures())
        {
            LogMsg
            (
                CID_FILE
                , CID_LINE
                , kCQCSMsgs::midStatus_FactoryFuncErr
                , cqcdcToLoad.strServerLibName()
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::AppStatus
                , cqcdcToLoad.strMoniker()
            );
        }
        throw;
    }

    {
        // Lock while we do this
        TLocker lockrSync(&m_mtxLock);

        //
        //  Tell the driver about our admin interface. It will register an 'alias'
        //  entry for itself that actually points back to us.
        //
        psdrvNew->SetSrvAdminOId(m_ooidAdmin);

        //
        //  Add this new driver to our list. We create a server driver info
        //  object with the driver object and the facility object, which is
        //  orphaned out of the janitor we temporarily put on it.
        //
        m_colDriverList.Add(new TServerDriverInfo(janFac.pobjOrphan(), psdrvNew));

        //
        //  At this point, the driver is ready to go, so start the driver processing
        //  now on its own thread. We want to put this off as long as possible, since
        //  once we do this we can't just delete it if anything happens, we'd have
        //  to do the whole shutdown sequence.
        //
        //  If it is marked as Paused, then it will just immediately go into paused
        //  state. If it's not paused, it will register itself with the name server.
        //
        psdrvNew->StartPoll();

        //
        //  Bump the driver list id, to invalidate any ids we've given back
        //  to clients.
        //
        m_c4DriverListId++;
    }

    if (bLogInfo())
    {
        LogMsg
        (
            CID_FILE
            , CID_LINE
            , kCQCSMsgs::midStatus_LoadedDriver
            , tCIDLib::ESeverities::Info
            , tCIDLib::EErrClasses::AppStatus
            , cqcdcToLoad.strMoniker()
            , pfacDriver->strName()
            , cqcdcToLoad.strMake()
            , cqcdcToLoad.strModel()
        );
    }
}


//
//  This is called at shutdown, to terminate and unload all of the drivers from this
//  server. It's also called if we are remotely forced to drop our drivers (such as when
//  a 'move all drivers to another host' type operation is invoked by a client.
//
//  If shutting down, at this point, we've dumped our admin interface from the ORB, so no
//  one can be talking to drivers unless they have their own private interfaces, which they
//  must clean up during shutdown.
//
//  What we do here is first is loop through the drivers and set their shutdown flags. Then
//  we just go through the list looking for them to reach terminated state.
//
//  If calling during shutdown, no locking is required because we are already hidden from
//  the outside work. If being called to remotely force an unload, the caller should lock.
//
tCIDLib::TVoid TFacCQCServer::UnloadDrivers()
{
    // Get a cursor over the driver list. If it's empty, we are done
    tCQCServer::TNCDrvCursor cursDrivers(&m_colDriverList);
    if (!cursDrivers.bReset())
        return;

    //
    //  Make one run through and unbind all of the name space bindings,
    //  so that no one else can see us as being available. If we cannot
    //  see the name server, then we can skip this part, since it's down
    //  or we can't get to it. If it's not down, these bindings will drop
    //  soon anyway because we won't be maintain our binding lease.
    //
    tCIDOrbUC::TNSrvProxy orbcNS;
    try
    {
        orbcNS = facCIDOrbUC().orbcNameSrvProxy(2500);
    }

    catch(const TError& errToCatch)
    {
        if (bShouldLog(errToCatch))
            LogEventObj(errToCatch);

        if (bLogFailures())
        {
            LogMsg
            (
                CID_FILE
                , CID_LINE
                , kCQCSMsgs::midStatus_NoNSNotUnbinding
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::NotFound
            );
        }
    }

    catch(...)
    {
        if (bLogFailures())
        {
            LogMsg
            (
                CID_FILE
                , CID_LINE
                , kCQCSMsgs::midStatus_NoNSNotUnbinding
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::NotFound
            );
        }
    }

    //
    //  Loop through all the drivers the first time and remove their
    //  bindings if we can and set their shutdown flags.
    //
    TString  strBinding;
    TString  strMoniker;
    do
    {
        TServerDriverInfo& sdiCur = cursDrivers.objWCur();
        strMoniker = sdiCur.sdrvDriver().strMoniker();

        // If we got the name server, attempt to rmove the binding
        if (orbcNS.pobjData())
        {
            try
            {
                strBinding = TCQCSrvAdminServerBase::strDrvScope;
                strBinding.Append(kCIDLib::chForwardSlash);
                strBinding.Append(strMoniker);
                orbcNS->RemoveBinding(strBinding, kCIDLib::False);
            }

            catch(const TError& errToCatch)
            {
                if (bShouldLog(errToCatch))
                    LogEventObj(errToCatch);

                if (bLogFailures())
                {
                    facCQCKit().LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , kKitErrs::errcDrv_CantUnbind
                        , tCIDLib::ESeverities::Failed
                        , tCIDLib::EErrClasses::NotFound
                        , strMoniker
                    );
                }
            }

            catch(...)
            {
                if (bLogFailures())
                {
                    facCQCKit().LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , kKitErrs::errcDrv_CantUnbind
                        , tCIDLib::ESeverities::Failed
                        , tCIDLib::EErrClasses::NotFound
                        , strMoniker
                    );
                }
            }
        }

        // Ask this guy to start his driver shutting down.
        sdiCur.StartShutdown();

    }   while (cursDrivers.bNext());

    //
    //  Now we just wait around waiting for them to go into terminated state.
    //  When we find terminated ones, we remove them. We keep going until we
    //  empty the list or have waited as long as we are willing to wait.
    //
    const tCIDLib::TEncodedTime enctEnd = TTime::enctNowPlusSecs(20);
    tCIDLib::TEncodedTime enctNow = TTime::enctNow();
    while (enctNow < enctEnd)
    {
        // If we've emptied the list, then break out
        if (!cursDrivers.bReset())
            break;

        // Sleep a bit before each round
        TThread::Sleep(250);
        do
        {
            TServerDriverInfo& sdiCur = *cursDrivers;
            if (sdiCur.bIsDead(500))
                m_colDriverList.RemoveAt(cursDrivers);
            else
                cursDrivers.bNext();

            // Loop until we hit the end or the list is empty
        }   while (cursDrivers.bIsValid());

        enctNow = TTime::enctNow();
    }

    if (enctNow >= enctEnd)
    {
        facCQCServer.LogMsg
        (
            CID_FILE
            , CID_LINE
            , kCQCSMsgs::midStatus_DrvStopTimeout
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::Timeout
        );
    }
}


//
//  Unregister a driver from the name server
//
tCIDLib::TVoid TFacCQCServer::UnregDriverNS(const TString& strMoniker)
{
    TString strBinding(TCQCSrvAdminServerBase::strDrvScope);
    strBinding.Append(kCIDLib::chForwardSlash);
    strBinding.Append(strMoniker);
    try
    {
        facCIDOrbUC().bDeregRebindObj(strBinding);

        tCIDOrbUC::TNSrvProxy orbcNS = facCIDOrbUC().orbcNameSrvProxy();
        orbcNS->RemoveBinding(strBinding, kCIDLib::False);
    }

    catch(const TError& errToCatch)
    {
        if (facCQCServer.bShouldLog(errToCatch))
            LogEventObj(errToCatch);

        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcDrv_CantUnbind
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , strMoniker
        );
    }

    catch(...)
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcDrv_CantUnbind
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , strMoniker
        );
    }
}


