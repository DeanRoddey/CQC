//
// FILE NAME: CQCDataSrv_InstSrvImpl.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/21/2001
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
//  This method implements the TCQCInstSrvImpl class, which is the implementation of
//  the name server IDL generated interface.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCDataSrv.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCInstSrvImpl,TCQCInstServerBase)



// ---------------------------------------------------------------------------
//   CLASS: TCQCDrvCfgDrvInfo
//  PREFIX: dcdi
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCDrvCfgDrvInfo: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCDrvCfgDrvInfo::TCQCDrvCfgDrvInfo() :

    m_c4Version(1)
{
}

TCQCDrvCfgDrvInfo::TCQCDrvCfgDrvInfo(const TCQCDriverObjCfg& cqcdcDrv) :

    m_c4Version(1)
    , m_cqcdcDrv(cqcdcDrv)
{
}

TCQCDrvCfgDrvInfo::~TCQCDrvCfgDrvInfo()
{
}


// ---------------------------------------------------------------------------
//  TCQCDrvCfgDrvInfo: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TCQCDrvCfgDrvInfo::bPaused() const
{
    return m_cqcdcDrv.bPaused();
}


//
//  Sets the paused state of our configuration. We return whether the state was
//  actually changed, i.e. the incoming state is different from what we had before.
//
tCIDLib::TBoolean TCQCDrvCfgDrvInfo::bSetPausedState(const tCIDLib::TBoolean bToSet)
{
    const tCIDLib::TBoolean bRet(bToSet != m_cqcdcDrv.bPaused());
    m_cqcdcDrv.bPaused(bToSet);

    // If it changed, bump the version
    if (bRet)
        m_c4Version++;
    return bRet;
}


tCIDLib::TCard4 TCQCDrvCfgDrvInfo::c4Version() const
{
    return m_c4Version;
}


const TCQCDriverObjCfg& TCQCDrvCfgDrvInfo::cqcdcDrv() const
{
    return m_cqcdcDrv;
}


const TString& TCQCDrvCfgDrvInfo::strMoniker() const
{
    return m_cqcdcDrv.strMoniker();
}


// Updates our configuration and bumps our version number
tCIDLib::TVoid TCQCDrvCfgDrvInfo::SetConfig(const TCQCDriverObjCfg& cqcdcToSet)
{
    m_cqcdcDrv = cqcdcToSet;
    m_c4Version++;
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCDrvCfgSrvInfo
//  PREFIX: dcsi
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCDrvCfgSrvInfo: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCDrvCfgSrvInfo::TCQCDrvCfgSrvInfo() :

    m_c4Version(1)
{
}

TCQCDrvCfgSrvInfo::TCQCDrvCfgSrvInfo(const TString& strHost) :

    m_c4Version(1)
    , m_strHost(strHost)
{
}

TCQCDrvCfgSrvInfo::~TCQCDrvCfgSrvInfo()
{
}


// ---------------------------------------------------------------------------
//  TCQCDrvCfgSrvInfo: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  We add a new driver info object to our list, after confirming that we don't
//  already have one with the incoming moniker. We bump our version number if the
//  driver is added.
//
tCIDLib::TCard4 TCQCDrvCfgSrvInfo::c4AddDriver(const TCQCDriverObjCfg& cqcdcToAdd)
{
    // Make sure we don't have this moniker already
    tCIDLib::TCard4 c4At;
    if (pdcdiByMoniker(cqcdcToAdd.strMoniker(), c4At))
    {
        facCQCDataSrv.ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kDSrvErrs::errcInst_AlreadyDrv
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Already
            , cqcdcToAdd.strMoniker()
        );
    }

    // Looks ok so add it and bump our version
    m_colDrivers.objPlace(cqcdcToAdd);
    m_c4Version++;

    return m_c4Version;
}


//
//  We remove the driver at the indicated index and bump our version, returning the
//  new version.
//
tCIDLib::TCard4 TCQCDrvCfgSrvInfo::c4RemoveDriverAt(const tCIDLib::TCard4 c4At)
{
    m_colDrivers.RemoveAt(c4At);

    // If we are now empty, return a max card version number
    if (m_colDrivers.bIsEmpty())
        return kCIDLib::c4MaxCard;

    // Else, bump our version and give it back
    m_c4Version++;
    return m_c4Version;
}


//
//  We update the configuration of the indicated driver and return its new version
//  number. We don't bump our own version since we are just changing the config of
//  a driver.
//
tCIDLib::TCard4 TCQCDrvCfgSrvInfo::c4UpdateDriver(const TCQCDriverObjCfg& cqcdcToSet)
{
    tCIDLib::TCard4 c4At;
    TCQCDrvCfgDrvInfo* pdcdiCur = pdcdiByMoniker(cqcdcToSet.strMoniker(), c4At);
    if (!pdcdiCur)
    {
        facCQCDataSrv.ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kDSrvErrs::errcInst_DrvNotFnd
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , cqcdcToSet.strMoniker()
        );
    }

    // We have it, so update it
    pdcdiCur->SetConfig(cqcdcToSet);
    return pdcdiCur->c4Version();
}


tCIDLib::TCard4 TCQCDrvCfgSrvInfo::c4DriverCnt() const
{
    return m_colDrivers.c4ElemCount();
}


tCIDLib::TCard4 TCQCDrvCfgSrvInfo::c4Version() const
{
    return m_c4Version;
}


const TCQCDrvCfgDrvInfo& TCQCDrvCfgSrvInfo::dcdiAt(const tCIDLib::TCard4 c4At) const
{
    return m_colDrivers[c4At];
}


//
//  Look up the indicated driver by moniker, returns null if not found. Returns a
//  pointer to the driver info and the index at which it was found, if found.
//
TCQCDrvCfgDrvInfo*
TCQCDrvCfgSrvInfo::pdcdiByMoniker(const TString& strToFind, tCIDLib::TCard4& c4At)
{
    const tCIDLib::TCard4 c4Count = m_colDrivers.c4ElemCount();
    for (c4At = 0; c4At < c4Count; c4At++)
    {
        TCQCDrvCfgDrvInfo& dcdiCur = m_colDrivers[c4At];
        if (dcdiCur.strMoniker().bCompareI(strToFind))
            return &dcdiCur;
    }
    return nullptr;
}


//
//  Load up the configuration for all of our drivers into the passed collection. Return
//  our current version as well.
//
tCIDLib::TVoid
TCQCDrvCfgSrvInfo::QueryDrvConfigs( tCQCKit::TDrvCfgList&   colToFill
                                    , tCIDLib::TCardList&   fcolCfgVers
                                    , tCIDLib::TCard4&      c4Version)
{
    const tCIDLib::TCard4 c4Count = m_colDrivers.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        colToFill.objAdd(m_colDrivers[c4Index].cqcdcDrv());
        fcolCfgVers.c4AddElement(m_colDrivers[c4Index].c4Version());
    }

    c4Version = m_c4Version;
}


//
//  Return a list of key value pairs with the monikers of our drivers as the key and
//  our host name as the value. We don't flush the list becasue this is often used
//  to collect this info on all drivers.
//
tCIDLib::TVoid TCQCDrvCfgSrvInfo::QueryHostMons(tCIDLib::TKVPList& colToFill)
{
    const tCIDLib::TCard4 c4Count = m_colDrivers.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        colToFill.objPlace(m_colDrivers[c4Index].strMoniker(), m_strHost);
}


//
//  Return a list of our monikers and the configuration versions for them.
//
tCIDLib::TVoid
TCQCDrvCfgSrvInfo::QueryMons(tCIDLib::TStrList&     colToFill
                            , tCIDLib::TCardList&   fcolVers
                            , tCIDLib::TBoolList&   fcolClDrvs)
{
    const tCIDLib::TCard4 c4Count = m_colDrivers.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        colToFill.objAdd(m_colDrivers[c4Index].strMoniker());
        fcolVers.c4AddElement(m_colDrivers[c4Index].c4Version());
        fcolClDrvs.c4AddElement
        (
            !m_colDrivers[c4Index].cqcdcDrv().strClientLibName().bIsEmpty()
        );
    }
}


const TString& TCQCDrvCfgSrvInfo::strHost() const
{
    return m_strHost;
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCInstSrvImpl
//  PREFIX: orbs
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCInstSrvImpl: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCInstSrvImpl::TCQCInstSrvImpl() :

    m_c4RmCfgSz(0)
    , m_f8Lat(0)
    , m_f8Long(0)
{
    //
    //  Build up the root paths to our various file types, special casing if we
    //  are in the development environment.
    //
    TPathStr pathTmp;
    if (facCQCKit().bDevMode())
    {
        // Get the CQC source tree and result directory variabls
        TString strSrcPath;
        TString strResPath;
        if (!TProcEnvironment::bFind(L"CQC_SRCTREE", strSrcPath)
        ||  !TProcEnvironment::bFind(L"CQC_RESDIR", strResPath))
        {
            facCQCDataSrv.ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kDSrvErrs::errcInst_DevEnvNotFnd
                , tCIDLib::ESeverities::ProcFatal
                , tCIDLib::EErrClasses::NotFound
            );
        }

        // The IR models dir is in the miscellaneous files area
        pathTmp = strSrcPath;
        pathTmp.AddLevel(L"Miscellaneous\\IRLibraries");
        m_strIRDir = pathTmp;

        // The manifests directory is the output sub-dir for manifests.
        pathTmp = strResPath;
        pathTmp.AddLevel(L"Manifests.Out");
        m_strManifestDir = pathTmp;

        // Make sure the user sub-dir exists
        if (!TFileSys::bExists(m_strManifestDir, L"User", tCIDLib::EDirSearchFlags::AllDirs))
            TFileSys::MakeSubDirectory(L"User", m_strManifestDir);

        // The protocol dir is the output sub-dir for protocols
        pathTmp = strResPath;
        pathTmp.AddLevel(L"Protocols.Out");
        m_strProtoDir = pathTmp;

        // And make sure the user sub-dir is there
        if (!TFileSys::bExists(m_strProtoDir, L"User", tCIDLib::EDirSearchFlags::AllDirs))
            TFileSys::MakeSubDirectory(L"User", m_strProtoDir);
    }
     else
    {
        // They are in the standard, server side places
        pathTmp = facCQCKit().strDataDir();
        pathTmp.AddLevel(L"DataServer\\IRModels");
        m_strIRDir = pathTmp;

        pathTmp = facCQCKit().strDataDir();
        pathTmp.AddLevel(L"DataServer\\Manifests");
        m_strManifestDir = pathTmp;

        pathTmp = facCQCKit().strDataDir();
        pathTmp.AddLevel(L"DataServer\\Protocols");
        m_strProtoDir = pathTmp;
    }

    //
    //  These are always in right place, but we still create a path for it just
    //  for consistency and flexibility.
    //
    pathTmp = facCQCKit().strDataDir();
    pathTmp.AddLevel(L"DataServer\\ZWU3Info");
    m_strZWDIDir = pathTmp;
}

TCQCInstSrvImpl::~TCQCInstSrvImpl()
{
}


// ---------------------------------------------------------------------------
//  TCQCInstSrvImpl: Public, inherited methods
// ---------------------------------------------------------------------------

// The token must be either for the account being updated or for a power user or up
tCIDLib::TBoolean
TCQCInstSrvImpl::bAddOrUpdateCheatSheet(const   TString&            strUserAccount
                                        ,       tCIDLib::TCard4&    c4SerialNum
                                        , const TCQCActVarList&     cavlToSet
                                        , const TCQCSecToken&       sectUser)
{
    CheckUserRole(sectUser, strUserAccount, tCQCKit::EUserRoles::PowerUser, CID_LINE);

    // Build up the path to the account and write it to the repo
    TString strPath = kCQCKit::pszInstKey_UserVarList;
    strPath.eReplaceToken(strUserAccount, kCIDLib::chDigit1);

    TLocker lockrSync(&m_mtxSync);
    return m_oseRepo.bAddOrUpdate(strPath, c4SerialNum, cavlToSet, 2048UL);
}


//
//  Returns true if it added a new one. If bMustBeNew is set, then it just not be an
//  existing account. Else, it must be an existing account.
//
tCIDLib::TBoolean
TCQCInstSrvImpl::bUpdateEmailAccount(const  TCQCEMailAccount&   emacctToAdd
                                    ,       tCIDLib::TCard4&    c4SerialNum
                                    , const TCQCSecToken&       sectUser
                                    , const tCIDLib::TBoolean   bMustBeNew)
{
    CheckUserRole(sectUser, tCQCKit::EUserRoles::PowerUser, CID_LINE);

    // Build up the path to the account and write it to the repo
    TString strPath = kCQCKit::pszInstKey_EMailAcctScope;
    strPath.Append(emacctToAdd.strAcctName());

    TLocker lockrSync(&m_mtxSync);

    // We have two scenarios
    const tCIDLib::TBoolean bExists = m_oseRepo.bKeyExists(strPath);
    if (bMustBeNew && bExists)
    {
        // We are adding a new one, but it's already there
        facCQCDataSrv.ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kDSrvErrs::errInst_DupEMail
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , emacctToAdd.strAcctName()
        );
    }
    else if (!bMustBeNew && !bExists)
    {
        // We are updating an existing one, but it's not there
        facCQCDataSrv.ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kDSrvErrs::errInst_EMailNotInList
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , emacctToAdd.strAcctName()
        );
    }

    // Do an add or update, since we now know whichever it does is fine
    return m_oseRepo.bAddOrUpdate(strPath, c4SerialNum, emacctToAdd, 256);
}

tCIDLib::TBoolean
TCQCInstSrvImpl::bAddOrUpdateGC100Cfg(  const   tCIDLib::TCard4         c4BufSz
                                        , const THeapBuf&               mbufToSet
                                        ,       tCIDLib::TCard4&        c4SerialNum
                                        , const TCQCSecToken&           sectUser)
{
    CheckUserRole(sectUser, tCQCKit::EUserRoles::PowerUser, CID_LINE);

    TLocker lockrSync(&m_mtxSync);
    return m_oseRepo.bAddOrUpdateDirect(kCQCKit::pszInstKey_GC100Cfg, c4SerialNum, mbufToSet, c4BufSz, 512UL);
}

tCIDLib::TBoolean
TCQCInstSrvImpl::bAddOrUpdateJAPCfg(const   tCIDLib::TCard4     c4BufSz
                                    , const THeapBuf&           mbufToSet
                                    ,       tCIDLib::TCard4&    c4SerialNum
                                    , const TCQCSecToken&       sectUser)
{
    CheckUserRole(sectUser, tCQCKit::EUserRoles::PowerUser, CID_LINE);
    TLocker lockrSync(&m_mtxSync);
    return m_oseRepo.bAddOrUpdateDirect(kCQCKit::pszInstKey_JAPwrCfg, c4SerialNum, mbufToSet, c4BufSz, 512UL);
}


//
//  Indicates if we have the indicated moniker in our config. If so, we give back the
//  host it is loaded on.
//
tCIDLib::TBoolean
TCQCInstSrvImpl::bDrvCfgDriverPresent(  const   TString&        strMoniker
                                        ,       TString&        strHost
                                        , const TCQCSecToken&   sectUser)
{
    // Don't stream back useless data if not found
    strHost.Clear();

    CheckUserRole(sectUser, tCQCKit::EUserRoles::LimitedUser, CID_LINE);
    TLocker lockrSync(&m_mtxSync);

    // Loop through the hosts and see if one has this guy
    const tCIDLib::TCard4 c4SCount  = m_colDrvCfgCache.c4ElemCount();
    for (tCIDLib::TCard4 c4SIndex = 0; c4SIndex < c4SCount; c4SIndex++)
    {
        TCQCDrvCfgSrvInfo& dcsiCur = m_colDrvCfgCache[c4SIndex];

        // See if this host has the moniker
        tCIDLib::TCard4 c4At;
        TCQCDrvCfgDrvInfo* pdcdiCur = dcsiCur.pdcdiByMoniker(strMoniker, c4At);
        if (pdcdiCur)
        {
            // It does so give back the host name and we succeeded
            strHost = dcsiCur.strHost();
            return kCIDLib::True;
        }
    }

    // Never found it
    return kCIDLib::False;
}


//
//  This will move all of the drivers configured for the old host to the new host, as long
//  as there are no naming conflicts (which shouldn't be possible.) The old server will remove
//  from the list since it now has no configured drivers.
//
tCIDLib::TBoolean
TCQCInstSrvImpl::bDrvCfgMoveDrivers(const   TString&            strOldHost
                                    , const TString&            strNewHost
                                    ,       tCIDLib::TStrList&  colMsgs
                                    , const TCQCSecToken&       sectUser)
{
    CheckUserRole(sectUser, tCQCKit::EUserRoles::PowerUser, CID_LINE);

    // Start with an empty messages list
    colMsgs.RemoveAll();

    //
    //  We have to lock while we are doing the bits related to our own data. Then we have
    //  to unlock to do the driver unloading and loading.
    //
    tCIDLib::TStrList colMovedMons;
    {
        TLocker lockrSync(&m_mtxSync);

        facCQCDataSrv.LogMsg
        (
            CID_FILE
            , CID_LINE
            , kDSrvMsgs::midStatus_DriverMoveStart
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
            , strOldHost
            , strNewHost
        );

        // Let's first try to find the old host. It has to exist
        TString strErrMsg;
        tCIDLib::TCard4 c4At;
        TCQCDrvCfgSrvInfo* pdcsiSrc = pdcsiFind(strOldHost, c4At);
        if (!pdcsiSrc)
        {
            facCQCDataSrv.LogMsg
            (
                CID_FILE
                , CID_LINE
                , kDSrvErrs::errcInst_DrvMoveOldHost
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
                , strOldHost
            );

            strErrMsg.LoadFromMsg(kDSrvErrs::errcInst_HostNotFnd, facCQCDataSrv, strOldHost);
            colMsgs.objAddMove(tCIDLib::ForceMove(strErrMsg));
            return kCIDLib::False;
        }

        //
        //  The new one doesn't have to exist. If it does, then we need to make sure that there
        //  cannot be any dups. It shouldn't be possible, but just in case because it causes such
        //  a mess if it happens.
        //
        tCIDLib::TBoolean bNew = kCIDLib::False;
        const tCIDLib::TCard4 c4SrcCnt = pdcsiSrc->c4DriverCnt();
        TCQCDrvCfgSrvInfo* pdcsiTar = pdcsiFind(strNewHost);
        if (pdcsiTar)
        {
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4SrcCnt; c4Index++)
            {
                const TCQCDrvCfgDrvInfo& dcdiCur = pdcsiSrc->dcdiAt(c4Index);

                tCIDLib::TCard4 c4At;
                if (pdcsiTar->pdcdiByMoniker(dcdiCur.strMoniker(), c4At))
                {
                    // It's a clash, can't do it
                    strErrMsg.LoadFromMsg
                    (
                        kDSrvErrs::errcInst_DrvMoveDup
                        , facCQCDataSrv
                        , dcdiCur.strMoniker()
                        , strOldHost
                        , strNewHost
                    );
                    colMsgs.objAddMove(tCIDLib::ForceMove(strErrMsg));
                    return kCIDLib::False;
                }
            }
        }
         else
        {
            facCQCDataSrv.LogMsg
            (
                CID_FILE
                , CID_LINE
                , kDSrvMsgs::midStatus_AddingNewHost
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
                , strNewHost
            );

            // Doesn't exist, so add it
            pdcsiTar = &m_colDrvCfgCache.objPlace(strNewHost);
            bNew = kCIDLib::True;
        }

        //
        //  Let's move all of the drivers to the new guy. Keep a list of the monikers so that
        //  can use that in a load command to the new host at the end.
        //
        colMovedMons.CheckExpansion(c4SrcCnt);
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4SrcCnt; c4Index++)
        {
            const TCQCDrvCfgDrvInfo& dcdiCur = pdcsiSrc->dcdiAt(c4Index);
            pdcsiTar->c4AddDriver(dcdiCur.cqcdcDrv());
            colMovedMons.objAdd(dcdiCur.strMoniker());
        }

        //
        //  And now we need to update the repo. This is sort of painful because there's
        //  no way to just rename things in place, and all of the drivers are separate config
        //  server items. So basically we have to go through and remove them all, and then add
        //  them back again.
        //
        //  We can at least remove them all from the old host scope with a single operation, by
        //  just deleting that scope. Then we have to write them all back out. The path will probably
        //  be slightly different which will probably cause a fair bit of churn in the config
        //  server if there are a lot of drivers when we put them all back.
        //
        //  We also take the risk here that something fails after we removed the old stuff but have
        //  not got the new stuff in place yet. We could write the new stuff first, but that would
        //  be even worse if something failed before we removed the old stuff since we'd have dup
        //  monikers for every driver. Oh well, damn the torpedos.
        //
        try
        {
            // Build up the config path for the old host
            TString strCfgPath(kCQCKit::pszInstKey_Drivers);
            facCQCRemBrws().AddPathComp(strCfgPath, strOldHost);

            // And let's remove that scope
            m_oseRepo.DeleteScope(strCfgPath);

            // Now loop through the drivers and add them back under their new names
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4SrcCnt; c4Index++)
            {
                const TCQCDrvCfgDrvInfo& dcdiCur = pdcsiSrc->dcdiAt(c4Index);
                strCfgPath = kCQCKit::pszInstKey_Drivers;
                facCQCRemBrws().AddPathComp(strCfgPath, strNewHost);
                facCQCRemBrws().AddPathComp(strCfgPath, dcdiCur.strMoniker());

                m_oseRepo.AddObject(strCfgPath, dcdiCur.cqcdcDrv(), 128);
            }

            // Now remove the old host from our host list
            pdcsiFind(strOldHost, c4At);
            m_colDrvCfgCache.RemoveAt(c4At);
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);

            facCQCDataSrv.LogMsg
            (
                CID_FILE
                , CID_LINE
                , kDSrvErrs::errcInst_DrvMoveCfgData
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
                , strOldHost
                , strNewHost
            );

            colMsgs.objPlace
            (
                L"The driver configuration could not be set to the new server. See the logs"
            );
            return kCIDLib::False;
        }
    }



    //
    //  NOTE we are unlocked now. we are just talking to CQCServers at this point. Our own
    //  data is already updated. And, if we stayed locked, the driver loads below would fail
    //  because CQCServer wouldn't be able to call back to us to download the driver config
    //  data.
    //
    //  So now let's ask the old server to unload all his drivers.
    //
    try
    {
        TCQCSrvAdminClientProxy* porbcOld = facCQCKit().porbcCQCSrvAdminProxy(strOldHost);
        if (porbcOld)
        {
            TJanitor<TCQCSrvAdminClientProxy> janProxy(porbcOld);
            porbcOld->UnloadAllDrivers(sectUser);

            //
            //  Now pause a bit while we wait for that to happen. We don't want to have two
            //  drivers with the same moniker running if possible, but we can't block here
            //  for a really long time either.
            //
            TThread::Sleep(3000);
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        facCQCDataSrv.LogMsg
        (
            CID_FILE
            , CID_LINE
            , kDSrvErrs::errcInst_DrvMoveUnloadOld
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
            , strOldHost
        );

        colMsgs.objPlace(L"An exception occurred while unload drivers from old host");
        colMsgs.objPlace(TString(L"   - ") + L"It would be safest to cycle the service on that host if it exists");
        colMsgs.objPlace(TString(L"   - ") + errToCatch.strErrText());

        // Fall through and continue
    }

    // And now let's ask the new server to load the new drivers
    try
    {
        TCQCSrvAdminClientProxy* porbcNew = facCQCKit().porbcCQCSrvAdminProxy(strNewHost);
        if (porbcNew)
        {
            TJanitor<TCQCSrvAdminClientProxy> janProxy(porbcNew);
            if (!porbcNew->bLoadDrivers(colMovedMons, sectUser))
            {
                colMsgs.objPlace
                (
                    L"Some errors occurred while loading drivers on new host, so all drivers "
                    L"may not have loaded successfully. See the logs"
                );
            }
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        colMsgs.objPlace(TString(L"   - ") + errToCatch.strErrText());

        facCQCDataSrv.LogMsg
        (
            CID_FILE
            , CID_LINE
            , kDSrvErrs::errcInst_DrvMoveLoadNew
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
            , strNewHost
        );

        colMsgs.objPlace(L"An exception occurred while loading drivers on new host");
        colMsgs.objPlace(TString(L"   - ") + errToCatch.strErrText());

        // Fall through and consider it a qualified success
    }

    facCQCDataSrv.LogMsg
    (
        CID_FILE
        , CID_LINE
        , kDSrvMsgs::midStatus_DriverMoveEnd
        , tCIDLib::ESeverities::Status
        , tCIDLib::EErrClasses::AppStatus
        , strOldHost
        , strNewHost
    );

    return kCIDLib::True;
}


//
//  Returns the configuration for the indicated driver. The caller passes in a
//  version, either zero or the value it got the last time it asked for this driver's
//  config. If the version hasn't changed we just return false to indicate it has
//  the latest stuff. Else, we return the current config and the current version.
//
//  The caller can pass in a host name that he believes the driver to be configured
//  for and we will validate that. If not, we just return the host that it was found
//  on.
//
tCIDLib::TBoolean
TCQCInstSrvImpl::bDrvCfgQueryConfig(        tCIDLib::TCard4&    c4Version
                                    , const TString&            strMoniker
                                    ,       TString&            strHost
                                    ,       TCQCDriverObjCfg&   cqcdcToFill
                                    , const TCQCSecToken&       sectUser)
{
    CheckUserRole(sectUser, tCQCKit::EUserRoles::LimitedUser, CID_LINE);

    TLocker lockrSync(&m_mtxSync);

    // Search the list of servers for one that has this moniker
    const tCIDLib::TCard4 c4SCount  = m_colDrvCfgCache.c4ElemCount();
    for (tCIDLib::TCard4 c4SIndex = 0; c4SIndex < c4SCount; c4SIndex++)
    {
        TCQCDrvCfgSrvInfo& dcsiCur = m_colDrvCfgCache[c4SIndex];

        // See if this host has the moniker
        tCIDLib::TCard4 c4At;
        TCQCDrvCfgDrvInfo* pdcdiCur = dcsiCur.pdcdiByMoniker(strMoniker, c4At);
        if (pdcdiCur)
        {
            // If they provided a host, then validate it
            if (!strHost.bIsEmpty() && !strHost.bCompareI(dcsiCur.strHost()))
            {
                facCQCDataSrv.ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kDSrvErrs::errcInst_BadDrvHost
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::AppError
                    , strMoniker
                    , dcsiCur.strHost()
                    , strHost
                );
            }

            // If the version is the same, return false, they have the right stuff
            if (pdcdiCur->c4Version() == c4Version)
                return kCIDLib::False;

            // Return the info and the current version
            cqcdcToFill = pdcdiCur->cqcdcDrv();
            strHost = dcsiCur.strHost();
            c4Version = pdcdiCur->c4Version();
            return kCIDLib::True;
        }
    }

    // Never found it so throw
    facCQCDataSrv.ThrowErr
    (
        CID_FILE
        , CID_LINE
        , kDSrvErrs::errcInst_DrvNotFnd
        , tCIDLib::ESeverities::Failed
        , tCIDLib::EErrClasses::NotFound
        , strMoniker
    );

    // Make the compiler happy
    return kCIDLib::False;
}


//
//  Returns the driver config objects for all of the drivers configured for the
//  indicated host. The caller passes in a version, either zero or the last time
//  it called this method for the indicated host. If the version hasn't changed, then
//  we just return false to indicate he has the latest stuff. Else we return the
//  current list and the current version, and return true.
//
//  If we don't find the host in our list, we just return false, not an error, since
//  that wouldn't be unusual, and always the case initially a new system without any
//  drivers installed yet.
//
tCIDLib::TBoolean
TCQCInstSrvImpl::bDrvCfgQueryHostConfigs(       tCIDLib::TCard4&        c4Version
                                        , const TString&                strHost
                                        ,       tCQCKit::TDrvCfgList&   colCfgs
                                        ,       tCIDLib::TCardList&     fcolCfgVers
                                        , const TCQCSecToken&           sectUser)
{
    CheckUserRole(sectUser, tCQCKit::EUserRoles::LimitedUser, CID_LINE);

    colCfgs.RemoveAll();
    fcolCfgVers.RemoveAll();

    TLocker lockrSync(&m_mtxSync);

    // See if we have this host. If not, just say no configs available
    TCQCDrvCfgSrvInfo* pdcsiSrc = pdcsiFind(strHost);
    if (!pdcsiSrc)
        return kCIDLib::False;

    // If the version hasn't changed then return false
    if (pdcsiSrc->c4Version() == c4Version)
        return kCIDLib::False;

    // Give back all of this guy's driver configs and the new version
    pdcsiSrc->QueryDrvConfigs(colCfgs, fcolCfgVers, c4Version);
    return kCIDLib::True;
}


// Returns a list of hosts that have configured drivers
tCIDLib::TBoolean
TCQCInstSrvImpl::bDrvCfgQueryHosts(tCIDLib::TStrList& colToFill, const TCQCSecToken& sectUser)
{
    CheckUserRole(sectUser, tCQCKit::EUserRoles::LimitedUser, CID_LINE);
    colToFill.RemoveAll();

    TLocker lockrSync(&m_mtxSync);

    const tCIDLib::TCard4 c4SCount  = m_colDrvCfgCache.c4ElemCount();
    for (tCIDLib::TCard4 c4SIndex = 0; c4SIndex < c4SCount; c4SIndex++)
        colToFill.objAdd(m_colDrvCfgCache[c4SIndex].strHost());

    return !colToFill.bIsEmpty();
}


//
//  We return a list of monikers for the drivers configured for the indicated
//  host.
//
tCIDLib::TBoolean
TCQCInstSrvImpl::bDrvCfgQueryMonList(       tCIDLib::TCard4&    c4Version
                                    , const TString&            strHost
                                    ,       tCIDLib::TStrList&  colToFill
                                    ,       tCIDLib::TCardList& fcolCfgVers
                                    ,       tCIDLib::TBoolList& fcolClDrvs
                                    , const TCQCSecToken&       sectUser)
{
    CheckUserRole(sectUser, tCQCKit::EUserRoles::LimitedUser, CID_LINE);
    colToFill.RemoveAll();
    fcolCfgVers.RemoveAll();

    TLocker lockrSync(&m_mtxSync);

    const tCIDLib::TCard4 c4SCount  = m_colDrvCfgCache.c4ElemCount();
    for (tCIDLib::TCard4 c4SIndex = 0; c4SIndex < c4SCount; c4SIndex++)
    {
        TCQCDrvCfgSrvInfo& dcsiCur = m_colDrvCfgCache[c4SIndex];
        if (dcsiCur.strHost().bCompareI(strHost))
        {
            // Ask this guy for his monikers and break out
            dcsiCur.QueryMons(colToFill, fcolCfgVers, fcolClDrvs);
            break;
        }
    }
    return !colToFill.bIsEmpty();
}


//
//  We return a list of key/value pairs for all the configured drivers. The keys are
//  the driver monikers and the value being the host on which each driver is
//  configured.
//
tCIDLib::TBoolean
TCQCInstSrvImpl::bDrvCfgQueryMonList(tCIDLib::TKVPList& colToFill, const TCQCSecToken& sectUser)
{
    CheckUserRole(sectUser, tCQCKit::EUserRoles::LimitedUser, CID_LINE);
    colToFill.RemoveAll();

    TLocker lockrSync(&m_mtxSync);

    const tCIDLib::TCard4 c4SCount  = m_colDrvCfgCache.c4ElemCount();
    for (tCIDLib::TCard4 c4SIndex = 0; c4SIndex < c4SCount; c4SIndex++)
    {
        TCQCDrvCfgSrvInfo& dcsiCur = m_colDrvCfgCache[c4SIndex];
        dcsiCur.QueryHostMons(colToFill);
    }
    return !colToFill.bIsEmpty();
}


//
//  Sets the paused state of the indicated driver. If the incoming state is different
//  from the current one, then we return true and the new version for the driver. Else
//  we return false to indicate we didn't do anything.
//
//  If we change the state then we update the config server with the new config.
//
//  The caller has to handle actually telling the driver to pause if that is relevant.
//
tCIDLib::TBoolean
TCQCInstSrvImpl::bDrvCfgSetPause(const  TString&            strMoniker
                                , const tCIDLib::TBoolean   bState
                                ,       tCIDLib::TCard4&    c4NewVersion
                                , const TCQCSecToken&       sectUser)
{
    CheckUserRole(sectUser, tCQCKit::EUserRoles::LimitedUser, CID_LINE);
    TLocker lockrSync(&m_mtxSync);

    // Search the list of servers for one that has this moniker
    const tCIDLib::TCard4 c4SCount  = m_colDrvCfgCache.c4ElemCount();
    for (tCIDLib::TCard4 c4SIndex = 0; c4SIndex < c4SCount; c4SIndex++)
    {
        TCQCDrvCfgSrvInfo& dcsiCur = m_colDrvCfgCache[c4SIndex];

        // See if this host has the moniker
        tCIDLib::TCard4 c4At;
        TCQCDrvCfgDrvInfo* pdcdiCur = dcsiCur.pdcdiByMoniker(strMoniker, c4At);
        if (pdcdiCur)
        {
            // We found it. If the state is the same, return false
            if (pdcdiCur->bPaused() == bState)
                return kCIDLib::False;

            // It has changed, so let's update the config server first
            try
            {
                // Get a copy and update it to write out
                TCQCDriverObjCfg cqcdcNew(pdcdiCur->cqcdcDrv());
                cqcdcNew.bPaused(bState);

                // Build up the appropriate repo path
                TString strCfgPath(kCQCKit::pszInstKey_Drivers);
                facCQCRemBrws().AddPathComp(strCfgPath, dcsiCur.strHost());
                facCQCRemBrws().AddPathComp(strCfgPath, strMoniker);

                m_oseRepo.c4UpdateObject(strCfgPath, cqcdcNew);
            }

            catch(TError& errToCatch)
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                throw;
            }

            // Update our own copy and return the new version
            pdcdiCur->bSetPausedState(bState);
            c4NewVersion = pdcdiCur->c4Version();

            return kCIDLib::True;
        }
    }

    // We never found the driver
    facCQCDataSrv.ThrowErr
    (
        CID_FILE
        , CID_LINE
        , kDSrvErrs::errcInst_DrvNotFnd
        , tCIDLib::ESeverities::Failed
        , tCIDLib::EErrClasses::NotFound
        , strMoniker
    );

    // Make the compiler happy
    return kCIDLib::False;
}


//
//  Query the cheat sheet for
tCIDLib::TBoolean
TCQCInstSrvImpl::bQueryCheatSheet(          tCIDLib::TCard4&    c4SerialNum
                                    ,       tCIDLib::ELoadRes&  eLoadRes
                                    , const TString&            strUserName
                                    ,       TCQCActVarList&     cavlCheatSheet
                                    , const TCQCSecToken&       sectUser)
{
    CheckUserRole(sectUser, strUserName, tCQCKit::EUserRoles::PowerUser, CID_LINE);

    // Build up the path to the account and write it to the repo
    TString strKey = kCQCKit::pszInstKey_UserVarList;
    strKey.eReplaceToken(strUserName, kCIDLib::chDigit1);

    // Lock while we access the config info memory
    TLocker lockrSync(&m_mtxSync);
    try
    {
        eLoadRes = m_oseRepo.eReadObject
        (
            strKey, c4SerialNum, cavlCheatSheet, kCIDLib::False
        );
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
    return (eLoadRes == tCIDLib::ELoadRes::NewData);
}


// Query the driver manifest info for the indicated driver type
tCIDLib::TBoolean
TCQCInstSrvImpl::bQueryDrvManifest(const    TString&        strMake
                                    , const TString&        strModel
                                    ,       TCQCDriverCfg&  cqcdcToFill)
{
    // Lock while we access the config info memory
    TLocker lockrSync(&m_mtxSync);

    // Find this one in the list and give back the info requested
    const tCIDLib::TCard4 c4Count = m_colDrvManifests.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TCQCDriverCfg& cqcdcCur = m_colDrvManifests[c4Index];

        // If its the same make and model, its our guy
        if ((cqcdcCur.strModel() == strModel)
        &&  (cqcdcCur.strMake() == strMake))
        {
            cqcdcToFill = m_colDrvManifests[c4Index];
            return kCIDLib::True;
        }
    }
    return kCIDLib::False;
}


//
//  Returns the email account info for the indicated account. The caller must pass a valid login.
//  It can't be forced higher than limited user because any user needs to be able to send an
//  e-mail.
//
tCIDLib::TBoolean
TCQCInstSrvImpl::bQueryEmailAccount(const   TString&            strAccount
                                    ,       TCQCEMailAccount&   emacctToFill
                                    , const tCIDLib::TBoolean   bThrowIfNot
                                    , const TCQCSecToken&       sectUser)
{
    CheckUserRole(sectUser, strAccount, tCQCKit::EUserRoles::LimitedUser, CID_LINE);

    // Build up the path to the account and write it to the repo
    TString strPath = kCQCKit::pszInstKey_EMailAcctScope;
    strPath.Append(strAccount);

    TLocker lockrSync(&m_mtxSync);

    tCIDLib::TCard4 c4Version = 0;
    return m_oseRepo.eReadObject(strPath, c4Version, emacctToFill, bThrowIfNot) == tCIDLib::ELoadRes::NewData;
}


//
//  This is to support the AI browser, which needs to get the names of all of the available
//  e-mail accounts. we can just ask for the all the key names under the email account scope of
//  our repo.
//
tCIDLib::TBoolean
TCQCInstSrvImpl::bQueryEMailAccountNames(tCIDLib::TStrList& colToFill, const TCQCSecToken& sectUser)
{
    CheckUserRole(sectUser, tCQCKit::EUserRoles::PowerUser, CID_LINE);
    TLocker lockrSync(&m_mtxSync);

    colToFill.RemoveAll();
    const tCIDLib::TCard4 c4Count = m_oseRepo.c4QueryKeysInScope(kCQCKit::pszInstKey_EMailAcctScope, colToFill);
    return colToFill.bIsEmpty();
}


// Returns the configuration for GC-100 serial ports
tCIDLib::ELoadRes
TCQCInstSrvImpl::eQueryGC100Cfg(        tCIDLib::TCard4&    c4SerialNum
                                ,       tCIDLib::TCard4&    c4BufSz
                                ,       THeapBuf&           mbufToFill
                                , const TCQCSecToken&       sectUser)
{
    CheckUserRole(sectUser, tCQCKit::EUserRoles::LimitedUser, CID_LINE);
    TLocker lockrSync(&m_mtxSync);

    // Make sure we don't stream any bytes back if not found or not new
    c4BufSz = 0;
    tCIDLib::ELoadRes eLoadRes = tCIDLib::ELoadRes::NotFound;
    try
    {
        eLoadRes = m_oseRepo.eReadObjectDirect
        (
            kCQCKit::pszInstKey_GC100Cfg, c4SerialNum, mbufToFill, c4BufSz, kCIDLib::False
        );
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
    return eLoadRes;
}


// Returns the configuration for Just Add Power serial ports
tCIDLib::ELoadRes
TCQCInstSrvImpl::eQueryJAPCfg(          tCIDLib::TCard4&    c4SerialNum
                                ,       tCIDLib::TCard4&    c4BufSz
                                ,       THeapBuf&           mbufToFill
                                , const TCQCSecToken&       sectUser)
{
    CheckUserRole(sectUser, tCQCKit::EUserRoles::LimitedUser, CID_LINE);
    TLocker lockrSync(&m_mtxSync);

    // Make sure we don't stream any bytes back if not found or not new
    c4BufSz = 0;
    tCIDLib::ELoadRes eLoadRes = tCIDLib::ELoadRes::NotFound;
    try
    {
        eLoadRes = m_oseRepo.eReadObjectDirect
        (
            kCQCKit::pszInstKey_JAPwrCfg, c4SerialNum, mbufToFill, c4BufSz, kCIDLib::False
        );
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
    return eLoadRes;
}


//
//  In this case the security token has to either be for the indicated user account
//  or for a role of power user or up. We return true if we returned new data.
//
tCIDLib::TBoolean
TCQCInstSrvImpl::bQueryKeyMap(          tCIDLib::TCard4&    c4SerialNum
                                ,       tCIDLib::ELoadRes&  eLoadRes
                                , const TString&            strUserName
                                ,       TCQCKeyMap&         kmToFill
                                , const TCQCSecToken&       sectUser)
{
    // Set up the repository key
    TString strCfgKey(kCQCKit::pszInstKey_KeyMaps, strUserName.c4Length() + 4);
    strCfgKey.eReplaceToken(strUserName, L'1');

    CheckUserRole(sectUser, strUserName, tCQCKit::EUserRoles::LimitedUser, CID_LINE);
    TLocker lockrSync(&m_mtxSync);

    try
    {
        eLoadRes = m_oseRepo.eReadObject(strCfgKey, c4SerialNum, kmToFill, kCIDLib::False);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
    return (eLoadRes == tCIDLib::ELoadRes::NewData);
}


// Returns our current lat/long info. If it's not been set, they will be zero, Null Island.
tCIDLib::TBoolean
TCQCInstSrvImpl::bQueryLocationInfo(        tCIDLib::TFloat8&   f8LatToFill
                                    ,       tCIDLib::TFloat8&   f8LongToFill
                                    , const TCQCSecToken&       sectUser)
{
    CheckUserRole(sectUser, tCQCKit::EUserRoles::LimitedUser, CID_LINE);
    TLocker lockrSync(&m_mtxSync);

    f8LatToFill = m_f8Lat;
    f8LongToFill = m_f8Long;

    return (f8LatToFill != 0) && (f8LongToFill != 0);
}


// Query the driver protocol file for the indicated driver
tCIDLib::TBoolean
TCQCInstSrvImpl::bQueryProtoFile(const  TString&            strFileName
                                ,       tCIDLib::TCard4&    c4BytesToFill
                                ,       THeapBuf&           mbufToFill)
{
    // Just in case, we don't want to stream back a bunch of bogus data
    c4BytesToFill = 0;

    //
    //  Build up the path to the file in question. We first check in the
    //  user path, then the system path.
    //
    TPathStr pathToLoad(m_strProtoDir);
    pathToLoad.AddLevel(L"User");
    pathToLoad.AddLevel(strFileName);
    pathToLoad.AppendExt(L"CQCProto");

    //
    //  If not found, then try the system protocol directory. If not found
    //  there, then give up.
    //
    if (!TFileSys::bExists(pathToLoad))
    {
        pathToLoad = m_strProtoDir;
        pathToLoad.AddLevel(strFileName);
        pathToLoad.AppendExt(L"CQCProto");
        if (!TFileSys::bExists(pathToLoad))
        {
            // Be sure we don't try to stream back a huge amount of junk!
            c4BytesToFill = 0;
            return kCIDLib::False;
        }
    }

    //  Read this guy in. Tell it not to throw if not found, we just return false
    TLocker lockrSync(&m_mtxSync);
    return bLoadUpFile(pathToLoad, mbufToFill, c4BytesToFill, kCIDLib::False);
}



//
//  Serves up Z-Wave device info files. For the individual files, the index includes
//  the base file name, so they pass that back in to query the contents of the
//  selected file, no extension. For the index we return the index file and the DTD
//  that will be used to validate the individual files later.
//
tCIDLib::TBoolean
TCQCInstSrvImpl::bQueryZWDIIndex(tCIDLib::TCard4&   c4IndexSz
                                , THeapBuf&         mbufIndex
                                , tCIDLib::TCard4&  c4DTDSz
                                , THeapBuf&         mbufDTD)
{
    // Just in case, so we don't stream back bogus data if we fail
    c4IndexSz = 0;
    c4DTDSz = 0;

    // Build up the path to the known names of the index and DTD files
    TPathStr pathIndex(m_strZWDIDir);
    pathIndex.AddLevel(L"ZWU3DevInfoIndex");
    pathIndex.AppendExt(L"xml");

    TPathStr pathDTD(m_strZWDIDir);
    pathDTD.AddLevel(L"ZWaveDevInfo");
    pathDTD.AppendExt(L"dtd");

    // Lock and load up both, return false if either fails to load
    TLocker lockrSync(&m_mtxSync);
    if (!bLoadUpFile(pathIndex, mbufIndex, c4IndexSz, kCIDLib::False))
        return kCIDLib::False;

    if (!bLoadUpFile(pathDTD, mbufDTD, c4DTDSz, kCIDLib::False))
    {
        c4IndexSz = 0;
        return kCIDLib::False;
    }
    return kCIDLib::True;
}

tCIDLib::TBoolean
TCQCInstSrvImpl::bQueryZWDIFile(const   TString&            strFileName
                                ,       tCIDLib::TCard4&    c4BufSz
                                ,       THeapBuf&           mbufData)
{
    // Just in case, so we don't stream back bogus data if we fail
    c4BufSz = 0;

    // Build up the path to this guy
    TPathStr pathToLoad(m_strZWDIDir);
    pathToLoad.AddLevel(strFileName);
    pathToLoad.AppendExt(L"zwdi");

    TLocker lockrSync(&m_mtxSync);
    return bLoadUpFile(pathToLoad, mbufData, c4BufSz, kCIDLib::False);
}


// Return our whole list of driver manifests available
tCIDLib::TCard4
TCQCInstSrvImpl::c4QueryDrvManifests(TVector<TCQCDriverCfg>& colToFill, const TCQCSecToken& sectUser)
{
    CheckUserRole(sectUser, tCQCKit::EUserRoles::LimitedUser, CID_LINE);

    // Flush the caller's collection first, just in case
    colToFill.RemoveAll();

    // And we have to sync during this operation
    TLocker lockrSync(&m_mtxSync);

    // We also store in a vector, so we can just assign to the caller's
    colToFill = m_colDrvManifests;
    return colToFill.c4ElemCount();
}


// Delete the indicated e-mail account. If it's not there we don't complain
tCIDLib::TBoolean
TCQCInstSrvImpl::bDeleteEMailAccount(const TString& strAccount, const TCQCSecToken& sectUser)
{
    CheckUserRole(sectUser, tCQCKit::EUserRoles::PowerUser, CID_LINE);

    TString strPath = kCQCKit::pszInstKey_EMailAcctScope;
    strPath.Append(strAccount);

    TLocker lockrSync(&m_mtxSync);
    return m_oseRepo.bDeleteObjectIfExists(strPath);
}


// Query some info on all of the loaded drivers
tCIDLib::TVoid
TCQCInstSrvImpl::DrvCfgQueryDrvsInfo(       tCIDLib::TStrList&     colMonikers
                                    ,       tCIDLib::TStrList&      colMakes
                                    ,       tCIDLib::TStrList&      colModels
                                    ,       tCQCKit::TDevCatList&   fcolCats
                                    , const TCQCSecToken&           sectUser)
{
    CheckUserRole(sectUser, tCQCKit::EUserRoles::LimitedUser, CID_LINE);
    colMonikers.RemoveAll();
    colMakes.RemoveAll();
    colModels.RemoveAll();
    fcolCats.RemoveAll();

    TLocker lockrSync(&m_mtxSync);

    const tCIDLib::TCard4 c4SCount  = m_colDrvCfgCache.c4ElemCount();
    for (tCIDLib::TCard4 c4SIndex = 0; c4SIndex < c4SCount; c4SIndex++)
    {
        // Get the current server info object
        TCQCDrvCfgSrvInfo& dcsiCur = m_colDrvCfgCache[c4SIndex];

        // Iterate its drivers
        const tCIDLib::TCard4 c4DCount = dcsiCur.c4DriverCnt();
        for (tCIDLib::TCard4 c4DIndex = 0; c4DIndex < c4DCount; c4DIndex++)
        {
            const TCQCDrvCfgDrvInfo& dcdiCur = dcsiCur.dcdiAt(c4DIndex);

            colMonikers.objAdd(dcdiCur.strMoniker());
            colMakes.objAdd(dcdiCur.cqcdcDrv().strMake());
            colModels.objAdd(dcdiCur.cqcdcDrv().strModel());
            fcolCats.c4AddElement(dcdiCur.cqcdcDrv().eCategory());
        }
    }
}


//
//  Returns the info needed by the reporting system, though it could be useful to
//  someone else.
//
//      Key  = Moniker
//      Val1 = Make/Model
//      Val2 = Host
//      Val3 = Version
//
tCIDLib::TVoid
TCQCInstSrvImpl::DrvCfgQueryReportInfo(tCIDLib::TKValsList& colToFill, const TCQCSecToken& sectUser)
{
    CheckUserRole(sectUser, tCQCKit::EUserRoles::PowerUser, CID_LINE);
    colToFill.RemoveAll();

    TLocker lockrSync(&m_mtxSync);

    TString strMM;
    TKeyValues kvalsCur;
    const tCIDLib::TCard4 c4SCount  = m_colDrvCfgCache.c4ElemCount();
    for (tCIDLib::TCard4 c4SIndex = 0; c4SIndex < c4SCount; c4SIndex++)
    {
        // Get the current server info object
        TCQCDrvCfgSrvInfo& dcsiCur = m_colDrvCfgCache[c4SIndex];

        // Iterate its drivers
        const tCIDLib::TCard4 c4DCount = dcsiCur.c4DriverCnt();
        for (tCIDLib::TCard4 c4DIndex = 0; c4DIndex < c4DCount; c4DIndex++)
        {
            const TCQCDrvCfgDrvInfo& dcdiCur = dcsiCur.dcdiAt(c4DIndex);

            strMM = dcdiCur.cqcdcDrv().strMake();
            strMM.Append(kCIDLib::chForwardSlash);
            strMM.Append(dcdiCur.cqcdcDrv().strModel());
            kvalsCur.Set
            (
                dcdiCur.strMoniker()
                , strMM
                , dcsiCur.strHost()
                , dcdiCur.cqcdcDrv().strVersion()
            );
            colToFill.objAdd(kvalsCur);
        }
    }
}


//
//  If we find the indicated moniker, remove it. We have to remove it from the config server
//  as well. The client has to unload it from the CQCServer if that is necessary. We throw if
//  the host or driver isn't found. We give back the new version for the target server, or
//  max card if we removed it because it's now empty.
//
tCIDLib::TVoid
TCQCInstSrvImpl::DrvCfgRemoveDrv(const  TString&            strMoniker
                                , const TString&            strHost
                                ,       tCIDLib::TCard4&    c4NewVersion
                                , const TCQCSecToken&       sectUser)
{
    CheckUserRole(sectUser, tCQCKit::EUserRoles::PowerUser, CID_LINE);
    TLocker lockrSync(&m_mtxSync);

    // See if we have this host
    tCIDLib::TCard4 c4HostInd;
    TCQCDrvCfgSrvInfo* pdcsiSrc = pdcsiFind(strHost, c4HostInd);
    if (!pdcsiSrc)
    {
        facCQCDataSrv.ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kDSrvErrs::errcInst_HostNotFnd
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , strHost
        );
    }

    //
    //  See if this host has the moniker. If not we throw since they said
    //  it was on this list.
    //
    tCIDLib::TCard4 c4At;
    TCQCDrvCfgDrvInfo* pdcdiCur = pdcsiSrc->pdcdiByMoniker(strMoniker, c4At);
    if (!pdcdiCur)
    {
        // The driver doesn't exist
        facCQCDataSrv.ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kDSrvErrs::errcInst_DrvNotOnHost
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , strMoniker
            , strHost
        );
    }

    // Remove it from the repo
    {
        // Build up the appropriate repo path
        TString strCfgPath(kCQCKit::pszInstKey_Drivers);
        facCQCRemBrws().AddPathComp(strCfgPath, strHost);
        facCQCRemBrws().AddPathComp(strCfgPath, strMoniker);
        m_oseRepo.DeleteObject(strCfgPath);
    }

    //
    //  Let the server remove it from its list. If it comes out empty, then just remove
    //  it from our hsot list.
    //
    c4NewVersion = pdcsiSrc->c4RemoveDriverAt(c4At);
    if (c4NewVersion == kCIDLib::c4MaxCard)
        m_colDrvCfgCache.RemoveAt(c4HostInd);
}


//
//  We first try to write the config to the config server. We call the appropriate
//  method based on the new driver flag, so that we can catch a dup or trying to
//  update something that's not there.
//
//  If that works, then we add/update our data. We give back the new version for the
//  affected driver.
//
//  We throw if either the host or moniker is not found.
//
tCIDLib::TVoid
TCQCInstSrvImpl::DrvCfgSetConfig(const  TString&            strMoniker
                                , const TString&            strHost
                                , const TCQCDriverObjCfg&   cqcdcToSet
                                , const tCIDLib::TBoolean   bNewDriver
                                ,       tCIDLib::TCard4&    c4NewVersion
                                , const TCQCSecToken&       sectUser)
{
    CheckUserRole(sectUser, tCQCKit::EUserRoles::PowerUser, CID_LINE);
    TLocker lockrSync(&m_mtxSync);

    // See if we have this host
    TCQCDrvCfgSrvInfo* pdcsiTar = pdcsiFind(strHost);

    //
    //  If not, and this is not a new driver, then clearly that's bad. Else, let's
    //  add a new host entry to our info.
    //
    if (!pdcsiTar)
    {
        if (!bNewDriver)
        {
            facCQCDataSrv.ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kDSrvErrs::errcInst_HostNotFnd
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::NotFound
                , strHost
            );
        }

        pdcsiTar = &m_colDrvCfgCache.objPlace(strHost);
    }

    // See if the host has the indicated driver
    tCIDLib::TCard4 c4At;
    TCQCDrvCfgDrvInfo* pdcdiTar = pdcsiTar->pdcdiByMoniker(strMoniker, c4At);

    //
    //  If we have it and this is supposed to be a new one, that's bad. If we don't
    //  have it and this is not a new driver, then that's bad.
    //
    if (pdcdiTar && bNewDriver)
    {
        // We already have it, but they said it's new
        facCQCDataSrv.ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kDSrvErrs::errcInst_AlreadyDrv
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Already
            , strMoniker
        );
    }
     else if (!pdcdiTar && !bNewDriver)
    {
        // We don't have it, but they said it's not new
        facCQCDataSrv.ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kDSrvErrs::errcInst_DrvNotFnd
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , strMoniker
        );
    }

    //
    //  Update it on the config server. This will naturally add the host if this is
    //  the first driver on this host.
    //
    try
    {
        // Build up the appropriate repo path
        TString strCfgPath(kCQCKit::pszInstKey_Drivers);
        facCQCRemBrws().AddPathComp(strCfgPath, strHost);
        facCQCRemBrws().AddPathComp(strCfgPath, strMoniker);

        if (bNewDriver)
            m_oseRepo.AddObject(strCfgPath, cqcdcToSet, 256);
        else
            m_oseRepo.c4UpdateObject(strCfgPath, cqcdcToSet);

        // Update or add to our list now
        if (bNewDriver)
            c4NewVersion = pdcsiTar->c4AddDriver(cqcdcToSet);
        else
            c4NewVersion = pdcsiTar->c4UpdateDriver(cqcdcToSet);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}


//
//  Query the contents of the two files used by the CQCVoice clients. These are shipped
//  files that they just need to download.
//
//  This is called rarely, when these programs start up or are asked to reload their
//  configuration.
//
tCIDLib::TVoid
TCQCInstSrvImpl::QueryVoiceFiles(tCIDLib::TCard4&   c4TreeBufSz
                                , THeapBuf&         mbufTree
                                , tCIDLib::TCard4&  c4GramBufSz
                                , THeapBuf&         mbufGrammar)
{
    // Make sure we don'st tstraem anything back if we don't have the data
    c4GramBufSz = 0;
    c4TreeBufSz = 0;

    // Build up the two paths and make sure the files exist
    TPathStr pathTree(facCQCKit().strDataDir());
    pathTree.AddLevel(L"CQCVoice");
    TPathStr pathGram(pathTree);

    pathTree.AddLevel(L"CQCVoice_BTree");
    pathTree.AppendExt(L"xml");
    pathGram.AddLevel(L"CQCVoice");
    pathGram.AppendExt(L"Grammar");

    if (!TFileSys::bExists(pathTree) || !TFileSys::bExists(pathGram))
    {
        facCQCDataSrv.ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kDSrvErrs::errcInst_VoiceFiles
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
        );
    }

    // Looks like we are ok, so try to read them in
    {
        TBinaryFile flSrc(pathTree);
        flSrc.Open
        (
            tCIDLib::EAccessModes::Multi_Read
            , tCIDLib::ECreateActs::OpenIfExists
            , tCIDLib::EFilePerms::Default
            , tCIDLib::EFileFlags::SequentialScan
        );

        // Give back the size and read the buffer, failing if not all bytes
        c4TreeBufSz = tCIDLib::TCard4(flSrc.c8CurSize());
        flSrc.c4ReadBuffer(mbufTree, c4TreeBufSz, tCIDLib::EAllData::FailIfNotAll);
    }

    {
        TBinaryFile flSrc(pathGram);
        flSrc.Open
        (
            tCIDLib::EAccessModes::Multi_Read
            , tCIDLib::ECreateActs::OpenIfExists
            , tCIDLib::EFilePerms::Default
            , tCIDLib::EFileFlags::SequentialScan
        );

        // Give back the size and read the buffer, failing if not all bytes
        c4GramBufSz = tCIDLib::TCard4(flSrc.c8CurSize());
        flSrc.c4ReadBuffer(mbufGrammar, c4GramBufSz, tCIDLib::EAllData::FailIfNotAll);
    }
}


tCIDLib::TVoid
TCQCInstSrvImpl::RenameEMailAccount(const   TString&        strOldName
                                    , const TString&        strNewName
                                    , const TCQCSecToken&   sectUser)
{
    CheckUserRole(sectUser, tCQCKit::EUserRoles::PowerUser, CID_LINE);

    TString strOldKey = kCQCKit::pszInstKey_EMailAcctScope;
    strOldKey.Append(strOldName);

    TString strNewKey = kCQCKit::pszInstKey_EMailAcctScope;
    strNewKey.Append(strNewName);

    TLocker lockrSync(&m_mtxSync);

    // Make sure we don't already have one with the new name
    if (m_oseRepo.bKeyExists(strNewKey))
    {
        facCQCDataSrv.ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kDSrvErrs::errInst_DupEMail
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Duplicate
            , strNewName
        );
    }

    // Make sure we have one with the old name
    if (!m_oseRepo.bKeyExists(strOldKey))
    {
        facCQCDataSrv.ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kDSrvErrs::errInst_EMailNotInList
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , strOldName
        );
    }

    // Read in the old onem, and remove it, say to throw if not found since we checked above
    TCQCEMailAccount emacctOld;
    tCIDLib::TCard4 c4Ver = 0;
    m_oseRepo.eReadObject(strOldKey, c4Ver, emacctOld, kCIDLib::True);
    m_oseRepo.DeleteObject(strOldKey);

    // Create a new one with the same info but new account name and write it out
    TCQCEMailAccount emacctNew(strNewName, emacctOld);
    m_oseRepo.AddObject(strNewKey, emacctNew, 256);
}


//
//  In these cases, the file name includes the extension, since it is just
//  uploading a file that was read in, not from selecting a protocol name
//  from a list (which is the case in the query methods above.)
//
tCIDLib::TVoid
TCQCInstSrvImpl::UploadProtocol(const   TString&            strFileName
                                , const tCIDLib::TCard4     c4DataSize
                                , const THeapBuf&           mbufData
                                , const tCIDLib::TBoolean   bSystem
                                , const TCQCSecToken&       sectUser)
{
    CheckUserRole(sectUser, tCQCKit::EUserRoles::PowerUser, CID_LINE);

    //
    //  Build up the full path to the file. Then build up a temp file path,
    //  since we will first write to a temp file. The final target path
    //  will go into the User subdirectory if not a system file.
    //
    TPathStr pathTar(m_strProtoDir);
    if (!bSystem)
        pathTar.AddLevel(L"User");
    pathTar.AddLevel(strFileName);
    pathTar.AppendExt(L"CQCProto");

    TPathStr pathTmp(m_strProtoDir);
    pathTmp.AddLevel(L"TmpProtocFile.Data");

    //
    //  Now we need to lock so that we control the repository during
    //  the rest of the operation.
    //
    TLocker lockrSync(&m_mtxSync);

    // And write the temp file contents, overwriting any previous temp file
    {
        TBinaryFile flTar(pathTmp);
        flTar.Open
        (
            tCIDLib::EAccessModes::Excl_Write
            , tCIDLib::ECreateActs::CreateAlways
            , tCIDLib::EFilePerms::Default
            , tCIDLib::EFileFlags::SequentialScan
        );

        //
        //  Write it out as a binary blob. It was uploaded via a raw binary
        //  read of the file, so this just recreates the original contents.
        //
        const tCIDLib::TCard4 c4Written = flTar.c4WriteBuffer(mbufData, c4DataSize);

        // If not all written, then freak out
        if (c4Written != c4DataSize)
        {
            // We are throwing a CIDLib error here!
            facCIDLib().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kCIDErrs::errcFile_NotAllData
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::NotAllWritten
                , TCardinal(c4DataSize)
                , TCardinal(c4Written)
            );
        }

        flTar.Close();
    }

    //
    //  Ok, the temp file is out there. If there is a current file with
    //  the target name, we need to rename it to a backup name.
    //
    if (TFileSys::bExists(pathTar))
    {
        // Build up the back up file name
        TPathStr pathBak(pathTar);
        pathBak.Append(L"_Bak");

        // If that exists, then delete it
        if (TFileSys::bExists(pathBak))
            TFileSys::DeleteFile(pathBak);

        // Now rename the original to the backup
        TFileSys::Rename(pathTar, pathBak);
    }

    // Now we can rename the temp file to the target file
    TFileSys::Rename(pathTmp, pathTar);

    // Log the fact that we stored the new file
    if (facCQCDataSrv.bLogInfo())
    {
        facCQCDataSrv.LogMsg
        (
            CID_FILE
            , CID_LINE
            , kDSrvMsgs::midStatus_Updated
            , tCIDLib::ESeverities::Info
            , tCIDLib::EErrClasses::AppStatus
            , TString(L"PDL File")
            , strFileName
        );
    }
}


//
//  This one is a special case for access rights. The security token either needs to
//  be for the indicated user account, or it has to be a power user. The facility class
//  provides a helper for this.
//
tCIDLib::TVoid
TCQCInstSrvImpl::SetKeyMap( const   TString&        strUserName
                            , const TCQCKeyMap&     kmToSet
                            , const TCQCSecToken&   sectUser)
{
    CheckUserRole(sectUser, strUserName, tCQCKit::EUserRoles::PowerUser, CID_LINE);

    // Set up the repository key
    TString strKey(kCQCKit::pszInstKey_KeyMaps, strUserName.c4Length() + 4);
    strKey.eReplaceToken(strUserName, L'1');

    TLocker lockrSync(&m_mtxSync);

    tCIDLib::TCard4 c4KeyMapVer = 0;
    m_oseRepo.bAddOrUpdate(strKey, c4KeyMapVer, kmToSet);
}


tCIDLib::TBoolean
TCQCInstSrvImpl::SetLocationInfo(const  tCIDLib::TFloat8    f8Lat
                                , const tCIDLib::TFloat8    f8Long
                                , const TCQCSecToken&       sectUser)
{
    CheckUserRole(sectUser, tCQCKit::EUserRoles::PowerUser, CID_LINE);

    // Verify that it doesn't cause an obvious conflict with the system time zone
    try
    {
        TTime tmCur(tCIDLib::ESpecialTimes::CurrentDate);
        tmCur.SetToSunrise(f8Lat, f8Long);

        m_f8Lat = f8Lat;
        m_f8Long = f8Long;
    }

    catch(...)
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcCfg_BadLocInfo
            , tCIDLib::ESeverities::Warn
            , tCIDLib::EErrClasses::Config
        );
    }

    // Format it out and write to the repo
    TString strData;
    strData.AppendFormatted(f8Lat, 2);
    strData.Append(kCIDLib::chComma);
    strData.AppendFormatted(f8Long, 2);

    TLocker lockrSync(&m_mtxSync);
    tCIDLib::TCard4 c4Ver = 0;
    m_oseRepo.bAddOrUpdate(kCQCKit::pszInstKey_LatLong, c4Ver, strData, 16);

    // Store the values in memory
    m_f8Lat = f8Lat;
    m_f8Long = f8Long;

    return kCIDLib::True;
}


tCIDLib::TVoid
TCQCInstSrvImpl::UploadManifest(const   TString&            strFileName
                                , const tCIDLib::TCard4     c4DataSize
                                , const THeapBuf&           mbufData
                                , const tCIDLib::TBoolean   bSystem
                                , const TCQCSecToken&       sectUser)
{
    CheckUserRole(sectUser, tCQCKit::EUserRoles::PowerUser, CID_LINE);

    //
    //  First of all, let's try to parse the contents as a manifest file.
    //  If we can't do that, it can't be valid.
    //
    TCQCDriverCfg cqcdcNew(strFileName, mbufData, c4DataSize);

    //
    //  It parsed, so we will first write the file to a temp file, and then
    //  rename it to the real file, so build up the file names. The final
    //  target path will be in the User subdirectory if not a system file.
    //
    TPathStr pathTmp(m_strManifestDir);
    pathTmp.AddLevel(L"Temp_Manifest.Data");

    TPathStr pathTar(m_strManifestDir);
    if (!bSystem)
        pathTar.AddLevel(L"User");
    pathTar.AddLevel(strFileName);

    // We have to lock at this point
    TLocker lockrSync(&m_mtxSync);

    // And write the temp file contents, overwriting any previous temp file
    {
        TBinaryFile flTmp(pathTmp);
        flTmp.Open
        (
            tCIDLib::EAccessModes::Excl_Write
            , tCIDLib::ECreateActs::CreateAlways
            , tCIDLib::EFilePerms::Default
            , tCIDLib::EFileFlags::SequentialScan
        );

        //
        //  Write it out as a binary blob. It was uploaded via a raw binary
        //  read of the file, so this just recreates the original contents.
        //
        const tCIDLib::TCard4 c4Written = flTmp.c4WriteBuffer(mbufData, c4DataSize);

        // If not all written, then freak out
        if (c4Written != c4DataSize)
        {
            // We are throwing a CIDLib error here!
            facCIDLib().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kCIDErrs::errcFile_NotAllData
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::NotAllWritten
                , TCardinal(c4DataSize)
                , TCardinal(c4Written)
            );
        }

        flTmp.Close();
    }

    //
    //  Ok, we have the temp file out.
    //
    if (TFileSys::bExists(pathTar))
    {
        // Build up the back up file name
        TPathStr pathBak(m_strManifestDir);
        pathBak.AddLevel(strFileName);
        pathBak.Append(L"_Bak");

        // If that exists, then delete it
        if (TFileSys::bExists(pathBak))
            TFileSys::DeleteFile(pathBak);

        // Now rename the original to the backup
        TFileSys::Rename(pathTar, pathBak);
    }

    // Now we can rename the temp file to the target file
    TFileSys::Rename(pathTmp, pathTar);

    // Log the fact that we stored the new file
    if (facCQCDataSrv.bLogInfo())
    {
        facCQCDataSrv.LogMsg
        (
            CID_FILE
            , CID_LINE
            , kDSrvMsgs::midStatus_Updated
            , tCIDLib::ESeverities::Info
            , tCIDLib::EErrClasses::AppStatus
            , TString(L"manifest")
            , strFileName
        );
    }

    //
    //  That worked, so we need to also update our in memory cache of
    //  driver config objects. So search the list for the make and model
    //  of the source file, if we find it, then update it. If we never
    //  find it, then add one.
    //
    const tCIDLib::TCard4 c4Count = m_colDrvManifests.c4ElemCount();
    tCIDLib::TCard4 c4Index;
    for (c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TCQCDriverCfg& cqcdcCur = m_colDrvManifests[c4Index];

        // If its the same make and model, its our guy
        if (cqcdcCur.bSameDriver(cqcdcNew))
        {
            cqcdcCur = cqcdcNew;
            break;
        }
    }

    // We don't have it, so we have to add it
    if (c4Index == c4Count)
        m_colDrvManifests.objAdd(cqcdcNew);
}


// ---------------------------------------------------------------------------
//  TCQCInstSrvImpl: Protected, inherited methods
// ---------------------------------------------------------------------------

// Not publically visible yet, so no locking needed
tCIDLib::TVoid TCQCInstSrvImpl::Initialize()
{
    m_c4RmCfgSz = 0;

    if (facCQCDataSrv.bLogInfo())
    {
        facCQCDataSrv.LogMsg
        (
            CID_FILE
            , CID_LINE
            , kDSrvMsgs::midStatus_StartManifestLoad
            , tCIDLib::ESeverities::Info
            , tCIDLib::EErrClasses::AppStatus
            , TString(L"System")
            , m_strManifestDir
        );
    }

    //
    //  Ok, lets search the configuration directory for .Manifest files, which
    //  are XML files that describe drivers available for install.
    //
    TDirIter    diterManifests;
    TFindBuf    fndbCur;

    if (diterManifests.bFindFirst(  m_strManifestDir
                                    , L"*.Manifest"
                                    , fndbCur
                                    , tCIDLib::EDirSearchFlags::AllFiles))
    {
        do
        {
            try
            {
                // Create a driver config object and let it parse the file
                m_colDrvManifests.objPlace(fndbCur.pathFileName());
            }

            catch(const TError& errToCatch)
            {
                // This one went awry, so skip it and keep going
                if (facCQCDataSrv.bShouldLog(errToCatch))
                    TModule::LogEventObj(errToCatch);
            }

        }   while (diterManifests.bFindNext(fndbCur));
    }

    //
    //  And now do the user manifests, if any. These are in a separate
    //  User directory under the main manifest directory.
    //
    TPathStr pathUser(m_strManifestDir);
    pathUser.AddLevel(L"User");
    if (TFileSys::bIsDirectory(pathUser))
    {
        if (facCQCDataSrv.bLogInfo())
        {
            facCQCDataSrv.LogMsg
            (
                CID_FILE
                , CID_LINE
                , kDSrvMsgs::midStatus_StartManifestLoad
                , tCIDLib::ESeverities::Info
                , tCIDLib::EErrClasses::AppStatus
                , TString(L"User")
                , pathUser
            );
        }

        if (diterManifests.bFindFirst(  pathUser
                                        , L"*.Manifest"
                                        , fndbCur
                                        , tCIDLib::EDirSearchFlags::AllFiles))
        {
            do
            {
                try
                {
                    m_colDrvManifests.objPlace(fndbCur.pathFileName());
                }

                catch(const TError& errToCatch)
                {
                    if (facCQCDataSrv.bShouldLog(errToCatch))
                        TModule::LogEventObj(errToCatch);
                }

            }   while (diterManifests.bFindNext(fndbCur));
        }
    }

    // If verbose logging, then log that we completed the manifest load
    if (facCQCDataSrv.bLogInfo())
    {
        facCQCDataSrv.LogMsg
        (
            CID_FILE
            , CID_LINE
            , kDSrvMsgs::midStatus_ManifestLoadDone
            , tCIDLib::ESeverities::Info
            , tCIDLib::EErrClasses::AppStatus
        );
    }


    //
    //  If the repo path doesn't exist, let's try to create it. Tell it not
    //  to throw if the path already exists, but it will if the path cannot
    //  be created.
    //
    TFileSys::MakePath(facCQCKit().strRepositoryDir());

    //
    //  Let's initialize the object store. This will create if if needed or open it
    //  if it exists.
    //
    try
    {
        m_oseRepo.bInitialize(facCQCKit().strRepositoryDir(), L"InstSrvData");
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);
        throw;
    }

    //
    //  Now we need to load the actual configured drivers info. We get this from
    //  the repo, which we have already waited for to get up and ready.
    //
    try
    {
        tCIDLib::TStrList colDrvPaths;
        m_oseRepo.bAllObjectsUnder(kCQCKit::pszInstKey_Drivers, colDrvPaths);

        //
        //  Now go through those and query the driver configuration data for
        //  each of them.
        //
        const TString strOSKey(kCQCKit::pszInstKey_Drivers);
        TCQCDriverObjCfg cqcdcCur;
        TString strHost;
        TString strMoniker;
        const tCIDLib::TCard4 c4Count = colDrvPaths.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const TString& strCurPath = colDrvPaths[c4Index];

            //
            //  Pull out the host and moniker. So first copy everything past the
            //  base object store path for drivers, which will be where the host name
            //  starts. Then split that on the slash to get the moniker.
            //
            strHost.CopyInSubStr(strCurPath, strOSKey.c4Length() + 1);
            strHost.bSplit(strMoniker, kCIDLib::chForwardSlash);

            // See if we have this server in our list yet. If not add it
            TCQCDrvCfgSrvInfo* pdcsiCur = pdcsiFind(strHost);
            if (!pdcsiCur)
                pdcsiCur = &m_colDrvCfgCache.objPlace(strHost);

            //
            //  Just in case of a failure of an individual driver, don't stop trying to
            //  load the rest.
            //
            try
            {
                // Read in the config for this driver
                tCIDLib::TCard4 c4SerNum = 0;
                const tCIDLib::ELoadRes eRes = m_oseRepo.eReadObject
                (
                    strCurPath, c4SerNum, cqcdcCur, kCIDLib::False
                );

                if (eRes == tCIDLib::ELoadRes::NewData)
                {
                    pdcsiCur->c4AddDriver(cqcdcCur);
                }
                 else
                {
                    facCQCDataSrv.LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , kDSrvErrs::errcInst_ReadDrvCfg
                        , tCIDLib::ESeverities::Failed
                        , tCIDLib::EErrClasses::InitFailed
                        , strMoniker
                    );
                }
            }

            catch(TError& errToCatch)
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);

                facCQCDataSrv.LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kDSrvErrs::errcInst_DrvLoadFailed
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::InitFailed
                    , strMoniker
                );
            }
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        // Log that we were unable to load driver configuration
        facCQCDataSrv.LogMsg
        (
            CID_FILE
            , CID_LINE
            , kDSrvErrs::errcInst_LoadDrvs
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::InitFailed
        );
    }


    // Load the lat/long info if it has been set
    try
    {
        m_f8Lat = 0;
        m_f8Long = 0;
        TString strLocInfo;
        tCIDLib::TCard4 c4SerNum = 0;
        const tCIDLib::ELoadRes eRes = m_oseRepo.eReadObject
        (
            kCQCKit::pszInstKey_LatLong, c4SerNum, strLocInfo, kCIDLib::False
        );

        tCIDLib::TBoolean bGotData = (eRes == tCIDLib::ELoadRes::NewData);
        if (bGotData)
        {
            tCIDLib::TFloat8 f8Lat, f8Long;
            TString strLat;
            TString strLong;
            TStringTokenizer stokInfo(&strLocInfo, L", ");
            if (!stokInfo.bGetNextToken(strLat) || !stokInfo.bGetNextToken(strLong))
                bGotData = kCIDLib::False;

            // The two tokens are there, so try to convert them
            if (bGotData)
            {
                f8Lat = TRawStr::f8AsBinary(strLat.pszBuffer(), bGotData);
                if (bGotData)
                    f8Long = TRawStr::f8AsBinary(strLong.pszBuffer(), bGotData);
            }

            //
            //  It appears to be ok. Just to be sure, try a conversion to see if
            //  it causes an error because of bad lat/long info. If that works, store
            //  the values.
            //
            if (bGotData)
            {
                try
                {
                    TTime tmCur(tCIDLib::ESpecialTimes::CurrentDate);
                    tmCur.SetToSunrise(f8Lat, f8Long);

                    m_f8Lat = f8Lat;
                    m_f8Long = f8Long;
                }

                catch(...)
                {
                    // Oh well, doesn't look valid, so say we don't have any
                    bGotData = kCIDLib::False;
                    facCQCKit().LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , kKitErrs::errcCfg_BadLocInfo
                        , tCIDLib::ESeverities::Warn
                        , tCIDLib::EErrClasses::Config
                    );

                    // Try to rmove the bad data
                    m_oseRepo.bDeleteObjectIfExists(kCQCKit::pszInstKey_LatLong);
                }
            }
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        // Log that we were unable to load driver configuration
        facCQCKit().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcCfg_BadLocInfo
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::InitFailed
        );

        m_f8Lat = 0;
        m_f8Long = 0;
    }
}


tCIDLib::TVoid TCQCInstSrvImpl::Terminate()
{
    // Close the object store if we got it open
    try
    {
        m_oseRepo.Close();
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);
    }
}


// ---------------------------------------------------------------------------
//  TCQCInstSrvImpl: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Most of the calls above need to read a file's contents into a memory buffer,
//  so this avoids a lot of redundant code.
//
//  WE ASSUME the caller has locked!
//
tCIDLib::TBoolean
TCQCInstSrvImpl::bLoadUpFile(const  TString&            strName
                            ,       TMemBuf&            mbufToLoad
                            ,       tCIDLib::TCard4&    c4Size
                            , const tCIDLib::TBoolean   bFailIfNotFound)
{
    TFindBuf    fndbSrc;

    // Try to find the first file
    c4Size = 0;
    if (!TFileSys::bExists(strName, fndbSrc, tCIDLib::EDirSearchFlags::AllFiles))
    {
        //
        //  If not found, and were told to fail, then throw now. Else just
        //  return the failure.
        //
        if (bFailIfNotFound)
        {
            facCQCDataSrv.ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kDSrvErrs::errcInst_FileNotFound
                , strName
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::NotFound
            );
        }
        return kCIDLib::False;
    }

    // Get the size out to the caller's parm, which requires a castw
    c4Size = tCIDLib::TCard4(fndbSrc.c8Size());

    //
    //  Lets make sure that the memory buffer is expanded out enough to hold
    //  the ne wcontents. Tell it not to bother preserving the previous contents.
    //
    if (mbufToLoad.c4Size() < c4Size)
        mbufToLoad.Reallocate(c4Size, kCIDLib::False);

    // And read the file into the memory buffer
    TBinaryFile flTmp(strName);
    flTmp.Open
    (
        tCIDLib::EAccessModes::Read
        , tCIDLib::ECreateActs::OpenIfExists
        , tCIDLib::EFilePerms::Default
        , tCIDLib::EFileFlags::SequentialScan
    );

    if (flTmp.c4ReadBuffer(mbufToLoad, c4Size) != c4Size)
    {
        facCQCDataSrv.ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kDSrvErrs::errcInst_FileReadError
            , strName
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::CantDo
        );
    }

    return kCIDLib::True;
}


//
//  Check to see if the passed security token is both valid and that it represents
//  a user account of at least the indicated role or greater.
//
//  Or, if the token is ether for the indicated user or it is at least the indicated
//  role or better.
//
tCIDLib::TVoid
TCQCInstSrvImpl::CheckUserRole( const   TCQCSecToken&       sectUser
                                , const tCQCKit::EUserRoles eMinRole
                                , const tCIDLib::TCard4     c4Line) const
{
    if (!facCQCDataSrv.bCheckMinRole(sectUser, eMinRole))
    {
        facCQCDataSrv.ThrowErr
        (
            CID_FILE
            , c4Line
            , kDSrvErrs::errcFAcc_UserRights
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Authority
        );
    }
}

tCIDLib::TVoid
TCQCInstSrvImpl::CheckUserRole( const   TCQCSecToken&       sectUser
                                , const TString&            strAccount
                                , const tCQCKit::EUserRoles eMinRole
                                , const tCIDLib::TCard4     c4Line) const
{
    if (!facCQCDataSrv.bCheckAccess(sectUser, strAccount, eMinRole))
    {
        facCQCDataSrv.ThrowErr
        (
            CID_FILE
            , c4Line
            , kDSrvErrs::errcFAcc_UserRights
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Authority
        );
    }
}


// Look for a server in our configured drivers list. We return null if not found
TCQCDrvCfgSrvInfo* TCQCInstSrvImpl::pdcsiFind(const TString& strToFind)
{
    tCIDLib::TCard4 c4Dummy;
    return pdcsiFind(strToFind, c4Dummy);
}


TCQCDrvCfgSrvInfo*
TCQCInstSrvImpl::pdcsiFind(const TString& strToFind, tCIDLib::TCard4& c4At)
{
    // Search the list of servers for one that has this moniker
    const tCIDLib::TCard4 c4SCount  = m_colDrvCfgCache.c4ElemCount();
    for (c4At = 0; c4At < c4SCount; c4At++)
    {
        TCQCDrvCfgSrvInfo& dcsiCur = m_colDrvCfgCache[c4At];
        if (dcsiCur.strHost().bCompareI(strToFind))
            return &dcsiCur;
    }
    return nullptr;
}


