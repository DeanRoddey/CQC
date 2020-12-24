//
// FILE NAME: CQSLRepoS_DriverImpl.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/02/2006
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
#include    "CQSLRepoS_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQSLMediaRepoEng,TCQCStdMediaRepoEng)
RTTIDecls(TCQSLRepoSDriver,TCQCStdMediaRepoDrv)


// ---------------------------------------------------------------------------
//  CLASS: TCQSLMediaRepoEng
// PREFIX: srdb
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQSLMediaRepoEng: Constructors and Destructor
// ---------------------------------------------------------------------------

//
//  Indicate the media types we support
//
TCQSLMediaRepoEng::TCQSLMediaRepoEng(const TString& strMoniker) :

    TCQCStdMediaRepoEng(strMoniker, tCQCMedia::EMTFlags::MultiMedia)
{
}

TCQSLMediaRepoEng::~TCQSLMediaRepoEng()
{
}


// ---------------------------------------------------------------------------
//  TCQSLMediaRepoEng: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  This is called if an image needs to be loaded. We only support large art directly
//  so we scale down the large art if they ask for a thumb. We should never get called
//  for anything we have already returned, since it will be cached.
//
//  Unfortunately we can't cache the large art if we load it to create the thumb. If
//  the large art is subsequently asked for, we have to load it again. We give up this
//  slight efficiency in order for the standard repo DB engine code that calls this
//  to be a lot simpler.
//
//  We let exceptions propogate back.
//
tCIDLib::TCard4
TCQSLMediaRepoEng::c4LoadRawCoverArt(const  TMediaImg&              mimgToLoad
                                    ,       TMemBuf&                mbufToFill
                                    , const tCQCMedia::ERArtTypes   eType)
{
    tCIDLib::TCard4 c4Ret = 0;
    try
    {
        //
        //  If the large art has already been loaded and they want small, then we
        //  don't have to load the file again.
        //
        if (eType == tCQCMedia::ERArtTypes::SmlCover)
            c4Ret = mimgToLoad.c4QueryArt(mbufToFill, tCQCMedia::ERArtTypes::LrgCover, 0);

        // Else we need to load it
        if (!c4Ret)
        {
            const TString& strPath = mimgToLoad.strArtPath(tCQCMedia::ERArtTypes::LrgCover);

            //
            //  Apparently not so load the file. Keep in mind that we have to use the
            //  large art path here, since we don't support the small. If the file
            //  exists, then load it.
            //
            if (!strPath.bIsEmpty() && TFileSys::bExists(strPath))
            {
                TBinFileInStream strmArt
                (
                    strPath
                    , tCIDLib::ECreateActs::OpenIfExists
                    , tCIDLib::EFilePerms::Default
                    , tCIDLib::EFileFlags::SequentialScan
                );

                // Read in the info
                tCIDLib::TCard2 c2FmtVer;
                tCIDLib::TCard4 c4XORSz;
                TString strPerId;
                strmArt >> c2FmtVer
                        >> strPerId
                        >> c4Ret
                        >> c4XORSz;
                CIDAssert
                (
                    c4Ret == (c4XORSz ^ kCIDLib::c4MaxCard)
                    , L"Invalid size info in image file"
                );

                CIDAssert
                (
                    strPerId == mimgToLoad.strPersistentId(tCQCMedia::ERArtTypes::LrgCover)
                    , L"The loaded img per id was not the same as the target img object"
                );

                // Read in the rest of the data into the caller's buffer
                strmArt.c4ReadBuffer(mbufToFill, c4Ret, tCIDLib::EAllData::FailIfNotAll);
            }
        }

        //
        //  If they asked for small art, let's scale it down since we don't support
        //  separate thumbs.
        //
        if ((eType == tCQCMedia::ERArtTypes::SmlCover) && c4Ret)
        {
            c4Ret = facCIDImgFact().c4CompactToJPEG
            (
                mbufToFill, c4Ret, TSize(kCQCMedia::c4DefThumbSz)
            );
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }

    return c4Ret;
}





// ---------------------------------------------------------------------------
//   CLASS: CQSLRepoSDriver
//  PREFIX: drv
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  CQSLRepoSDriver: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQSLRepoSDriver::TCQSLRepoSDriver(const TCQCDriverObjCfg& cqcdcToLoad) :

    TCQCStdMediaRepoDrv(cqcdcToLoad, &m_srdbEngine)
    , m_c4FldId_DBSerialNum(kCIDLib::c4MaxCard)
    , m_c4FldId_Loaded(kCIDLib::c4MaxCard)
    , m_c4FldId_LoadStatus(kCIDLib::c4MaxCard)
    , m_c4FldId_Status(kCIDLib::c4MaxCard)
    , m_c4FldId_ReloadDB(kCIDLib::c4MaxCard)
    , m_c4FldId_TitleCnt(kCIDLib::c4MaxCard)
    , m_enctLastClientLease(0)
    , m_eUplState(EUplStates::Idle)
    , m_ippnUpload(23482)
    , m_srdbEngine(cqcdcToLoad.strMoniker())
    , m_porbsMgrIntf(nullptr)
    , m_strDTCategory(L"category")
    , m_strDTCollect(L"collection")
    , m_strDTImage(L"image")
    , m_strDTItem(L"item")
    , m_strDTTitleSet(L"title set")
    , m_thrUploader
      (
        L"CQSLRepoUploaderThread" + cqcdcToLoad.strMoniker()
        , TMemberFunc<TCQSLRepoSDriver>(this, &TCQSLRepoSDriver::eUploadThread)
      )
{
    // Preload the 'loading' text for the status field
    m_strLoadMsg.LoadFromMsg(kCQSLRMsgs::midStatus_Loading, facCQSLRepoS());
}

TCQSLRepoSDriver::~TCQSLRepoSDriver()
{
}


// ---------------------------------------------------------------------------
//  TCQSLRepoSDriver, inherited methods
// ---------------------------------------------------------------------------

//
//  We support a number of backdoor commands to query data. These are overrides
//  of those methods.
//
//  We have to override all of these and just pass them to our parent (which will
//  in turn pass them on to the loaded repo engine) because we (unlike other repo
//  drivers) have a private ORB interface for the repo manager to use. So, that
//  means that all of the calls to use are not synchronized by the base driver
//  class.
//
//  Our private methods lock our m_mtxSync mutex, so we override these and lock
//  it as well before letting the base repo driver process them. This insure that
//  we get the needed locking. Normally this is not required.
//
tCIDLib::TBoolean
TCQSLRepoSDriver::bQueryData(const  TString&            strQType
                            , const TString&            strQData
                            ,       tCIDLib::TCard4&    c4OutBytes
                            ,       THeapBuf&           mbufToFill)
{
    TLocker lockrSync(&m_mtxSync);
    return TParent::bQueryData(strQType, strQData, c4OutBytes, mbufToFill);
}


tCIDLib::TBoolean
TCQSLRepoSDriver::bQueryData2(  const   TString&            strQType
                                ,       tCIDLib::TCard4&    c4IOBytes
                                ,       THeapBuf&           mbufIO)
{
    TLocker lockrSync(&m_mtxSync);
    return TParent::bQueryData2(strQType, c4IOBytes, mbufIO);
}


tCIDLib::TBoolean
TCQSLRepoSDriver::bQueryTextVal(const   TString&    strQType
                                , const TString&    strQData
                                ,       TString&    strToFill)
{
    TLocker lockrSync(&m_mtxSync);
    return TParent::bQueryTextVal(strQType, strQData, strToFill);
}


tCIDLib::TCard4 TCQSLRepoSDriver::c4QueryVal(const TString& strValId)
{
    TLocker lockrSync(&m_mtxSync);
    return TParent::c4QueryVal(strValId);
}


tCIDLib::TBoolean
TCQSLRepoSDriver::bSendData(const   TString&            strSendType
                            ,       TString&            strDataName
                            ,       tCIDLib::TCard4&    c4Bytes
                            ,       THeapBuf&           mbufToSend)
{
    TLocker lockrSync(&m_mtxSync);
    return TParent::bSendData(strSendType, strDataName, c4Bytes, mbufToSend);
}


//
//  We override this to support clients sending us some backdoor commands.
//  We check for our own commands, then pass through any others to our parent
//  class.
//
tCIDLib::TCard4
TCQSLRepoSDriver::c4SendCmd(const TString& strCmd, const TString& strParms)
{
    tCIDLib::TCard4 c4Ret = 0;

    if (strCmd == kCQCMedia::strCmd_SetUserRating)
    {
        //
        //  If a repo manager is currently connected, don't do this. Just return a
        //  1 instead of a 0, to indicate that this change would be lost if it was
        //  made, since the repo manager committing its changes would overwrite.
        //
        if (m_enctLastClientLease)
        {
            c4Ret = 1;
        }
         else
        {
            //
            //  It's our set user rating command. We have to look up the target title set,
            //  update its rating, and then write it back out. The parm has to be a title
            //  set cookie, a space, then a 1 to 10 value for the rating. If the
            //  conversion of the rating to a number fails or we don't find a title set
            //  with the passed cookie, it'll just throw back to the caller as an error.
            //
            TString strCookie(strParms);
            TString strRating;
            strCookie.bSplit(strRating, kCIDLib::chSpace);
            strCookie.StripWhitespace();
            strRating.StripWhitespace();
            tCIDLib::TCard4 c4Rating = strRating.c4Val();

            //
            //  Parse the cookie as a title cookie to make sure it is valid
            //  and to get the title set id and media type.
            //
            tCIDLib::TCard2 c2CatId;
            tCIDLib::TCard2 c2TitleId;
            tCQCMedia::EMediaTypes eMType = facCQCMedia().eParseTitleCookie
            (
                strCookie, c2CatId, c2TitleId
            );

            //
            //  We have to lock because we have a separate ORB interface that
            //  can call directly into the driver.
            //
            TLocker lockrSync(&m_mtxSync);

            // Update the title
            TMediaDB& mdbTar = m_srdbEngine.mdbInfo();
            mdbTar.pmtsSetTitleRating(c2TitleId, eMType, c4Rating);

            // And update the stored database and serial number
            StoreDBChanges();
        }
    }
     else
    {
        // Not one of ours, so pass it on, look first
        TLocker lockrSync(&m_mtxSync);
        c4Ret = TParent::c4SendCmd(strCmd, strParms);
    }
    return c4Ret;
}


// ---------------------------------------------------------------------------
//  TCQSLRepoSDriver: Public, non-virtual methods
// ---------------------------------------------------------------------------

// This is called by clients to get control over the repository
tCIDLib::TBoolean
TCQSLRepoSDriver::bGetLease(const   TString&            strLeaseId
                            , const tCIDLib::TBoolean   bOverride)
{
    //
    //  We have to lock since these separate ORB interfaces won't be synced against
    //  the regular driver ORB interface calls.
    //
    TLocker lockrSync(&m_mtxSync);

    //
    //  If there is a current id, then see if it's the same. If not, we
    //  either grant an override if requested, or deny.
    //
    const tCIDLib::TEncodedTime enctNow = TTime::enctNow();
    if (!m_strClientLeaseId.bIsEmpty() && (strLeaseId != m_strClientLeaseId))
    {
        // They are not equal. If not overriding, then deny
        if (!bOverride)
            return kCIDLib::False;

        //
        //  They want to override, but we only will do it if the
        //  current owner hasn't renewed in the last 2 minutes.
        //
        if (m_enctLastClientLease + (kCIDLib::enctOneMinute * 2) > enctNow)
            return kCIDLib::False;
    }

    // Grant the lease
    m_strClientLeaseId = strLeaseId;
    m_enctLastClientLease = enctNow;
    return kCIDLib::True;
}


//
//  This method will look for a cover art entry with the passed id and type. If the
//  actual image data isn't faulted in yet, it will fault it in.
//
tCIDLib::TBoolean
TCQSLRepoSDriver::bQueryArtById(const   tCQCMedia::EMediaTypes  eMediaType
                                , const tCQCMedia::ERArtTypes   eArtType
                                , const tCIDLib::TCard2         c2Id
                                ,       tCIDLib::TCard4&        c4ArtSz
                                ,       THeapBuf&               mbufArt)
{
    //
    //  Make sure the return size is zero if we don't return anything, else.
    //  it might copy back some random amount of data.
    //
    c4ArtSz = 0;

    //
    //  We have to lock since these separate ORB interfaces won't be synced against
    //  the regular driver ORB interface calls.
    //
    TLocker lockrSync(&m_mtxSync);

    // Make sure the media type is valid
    m_srdbEngine.CheckType(eMediaType, CID_LINE);

    // Look up the art and see if it's there
    const TMediaImg* pmimgRet = m_srdbEngine.mdbInfo().pmimgById
    (
        eMediaType, c2Id, kCIDLib::False
    );

    if (!pmimgRet)
        return kCIDLib::False;

    // See if we have the data already
    c4ArtSz = pmimgRet->c4QueryArt(mbufArt, eArtType);

    // If not, ssk the engine for the data
    if (!c4ArtSz)
        c4ArtSz = m_srdbEngine.c4LoadArtData(*pmimgRet, mbufArt, eArtType, 0);

    // If we have some image data, then return true
    return (c4ArtSz != 0);
}


//
//  Though this info is availble via the backdoor commands, it's convenient for
//  the repo manager to get it via a more structured API.
//
tCIDLib::TBoolean
TCQSLRepoSDriver::bQueryDB(         tCIDLib::TCard4&    c4CompSz
                            ,       THeapBuf&           mbufComp
                            , const TString&            strDBSerNum
                            ,       TString&            strRepoPath)
{
    //
    //  We have to lock since these separate ORB interfaces won't be synced against
    //  the regular driver ORB interface calls.
    //
    TLocker lockrSync(&m_mtxSync);

    //
    //  Call the backdoor query to get the info. They pass us a DB serial number
    //  so that they can also use this to see if they need to download changes.
    //  If the serial number hasn't changed, then we return nothing and false.
    //
    const tCIDLib::TBoolean bRet = TParent::bQueryData
    (
        kCQCMedia::strQuery_BinMediaDump, strDBSerNum, c4CompSz, mbufComp
    );

    // If no changes being returned, make sure the buffer size is zero
    if (!bRet)
        c4CompSz = 0;

    // Give back the repo path
    strRepoPath = m_pathStart;

    return bRet;
}



// This is called by the client with the lease, to drop the lease
tCIDLib::TVoid TCQSLRepoSDriver::DropLease(const TString& strLeaseId)
{
    //
    //  We have to lock since these separate ORB interfaces won't be synced against
    //  the regular driver ORB interface calls.
    //
    TLocker lockrSync(&m_mtxSync);

    // The must have the current lease id, else we do nothing
    if (strLeaseId != m_strClientLeaseId)
    {
        facCQCMedia().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"Could not drop lease for repo '%(1)'. Lease id was wrong"
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
            , strMoniker()
        );
        return;
    }

    // Clear the least fields
    m_strClientLeaseId.Clear();
    m_enctLastClientLease = 0;
}


//
//  Returns a list of referenced changer monikers. It's read only so we
//  don't require a lease id.
//
tCIDLib::TVoid
TCQSLRepoSDriver::QueryChangers(TVector<TString>& colMonikersToFill)
{
    //
    //  We have to lock since these separate ORB interfaces won't be synced against
    //  the regular driver ORB interface calls.
    //
    TLocker lockrSync(&m_mtxSync);

    // The database has a helper for this
    TMediaDB& mdbSrc = m_srdbEngine.mdbInfo();
    mdbSrc.QueryChangers(colMonikersToFill);
}



// ---------------------------------------------------------------------------
//  TCQSLRepoSDriver: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TCQSLRepoSDriver::bGetCommResource(TThread&)
{
    // We have no comm resource
    return kCIDLib::True;
}


tCIDLib::TBoolean TCQSLRepoSDriver::bWaitConfig(TThread&)
{
    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
    {
        facCQCMedia().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kMedMsgs::midStatus_StartingLoad
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
            , m_pathStart
        );
    }

    //
    //  Try to load any existing database file. Baring that, create an empty one
    //  and write it out. One way or another we then install that as the active
    //  database, get our status fields updated, and then we are ready.
    //
    try
    {
        TMediaDB mdbTmp;
        TString strDBSerNum;
        if (!bLoadDB(mdbTmp, strDBSerNum))
        {
            // It' a new database, so let's do some initialization and write it out
            SetInitDBContents(mdbTmp);
            StoreDBFile(mdbTmp, strDBSerNum);
        }
         else
        {
            //
            //  Do a check to make sure that all of the internal media files (the ones
            //  we ripped, currently music files) have location info paths that are
            //  relative to the repo base path. If they change the path we want to
            //  update these files so that they are exposed via same path. If any
            //  changes, write the file back out and get the new serial number.
            //
            if (mdbTmp.bAdjustIntMediaPaths(m_pathStart))
                StoreDBFile(mdbTmp, strDBSerNum);
        }

        //
        //  Lock while we do this. Normally it doesn't matter, but we also just
        //  call this method if the user forces a reload, and in that case we
        //  need to lock while we reload.
        //
        TLocker lockrSync(&m_mtxSync);

        InstallNewDB(mdbTmp, strDBSerNum);
    }

    catch(TError& errToCatch)
    {
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


tCQCKit::ECommResults
TCQSLRepoSDriver::eBoolFldValChanged(const  TString&            strName
                                    , const tCIDLib::TCard4     c4FldId
                                    , const tCIDLib::TBoolean   bNewValue)
{
    if (c4FldId == m_c4FldId_ReloadDB)
    {
        //
        //  Just call bWaitConfig() which will do what we want. Ignore it if there
        //  is an uploading going on, because the current data is going to be
        //  replaced anyway.
        //
        if (m_eUplState == EUplStates::Idle)
        {
            if (!bWaitConfig(*TThread::pthrCaller()))
                return tCQCKit::ECommResults::ValueRejected;
        }
    }
     else
    {
        IncUnknownWriteCounter();
        return tCQCKit::ECommResults::FieldNotFound;
    }
    return tCQCKit::ECommResults::Success;
}


// No device to connect to
tCQCKit::ECommResults TCQSLRepoSDriver::eConnectToDevice(TThread& thrCaller)
{
    return tCQCKit::ECommResults::Success;
}


tCQCKit::EDrvInitRes TCQSLRepoSDriver::eInitializeImpl()
{
    const TCQCDriverObjCfg& cqcdcOurs = cqcdcThis();

    // Pre-create the name server scope our mgmt interface goes into
    m_strMgrBindingScope = TCQSLRepoMgrServerBase::strBindingScope;
    m_strMgrBindingScope.eReplaceToken(cqcdcOurs.strMoniker(), L'm');

    m_strMgrBinding = m_strMgrBindingScope;
    m_strMgrBinding.Append(L"/");
    m_strMgrBinding.Append(TCQSLRepoMgrServerBase::strBindingName);

    // We should have gotten a prompt value with the start directory
    if (!cqcdcOurs.bFindPromptValue(L"StartPath", L"Path", m_pathStart))
    {
        facCQSLRepoS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"The repository directory prompt was not set"
            , tCIDLib::ESeverities::Status
        );
        return tCQCKit::EDrvInitRes::Failed;
    }

    // We may get user name and password prompts. Only store if we get both
    TString strUser, strPW;
    if (cqcdcOurs.bFindPromptValue(L"UserName", L"Text", strUser)
    &&  cqcdcOurs.bFindPromptValue(L"Password", L"Text", strPW))
    {
        m_strShareUserName = strUser;
        m_strSharePassword = strPW;
    }

    //
    //  We don't poll anything or connect to anything, so set both pretty high.
    //
    SetPollTimes(5000, 10000);

    //
    //  And set up our device fields. The bulk of what this driver does is
    //  via the media interface, and this driver doesn't control any device
    //  or app, so there are just a few.
    //
    //  We do different stuff depending on if we are in V1 or V2 mode.
    //
    TVector<TCQCFldDef> colFlds(8);
    TCQCFldDef flddNew;

    TString strFldDBSerialNum(L"DBSerialNum");
    TString strFldLoaded(L"Loaded");
    TString strFldReloadDB(L"ReloadDB");
    TString strFldLoadStatus(L"LoadStatus");
    TString strFldStatus(L"Status");
    TString strFldTitleCnt(L"TitleCount");

    const tCIDLib::TBoolean bV2(c4ArchVersion() > 1);

    //
    //  If V2, prepend the device class prefix to the fields used in V2. Else,
    //  set up any V1 only fields.
    //
    if (bV2)
    {
        const TString& strPref(L"MREPO#");
        strFldDBSerialNum.Prepend(strPref);
        strFldLoadStatus.Prepend(strPref);
        strFldReloadDB.Prepend(strPref);
        strFldStatus.Prepend(strPref);
        strFldTitleCnt.Prepend(strPref);
    }
     else
    {
        flddNew.Set
        (
            strFldLoaded
            , tCQCKit::EFldTypes::Boolean
            , tCQCKit::EFldAccess::Read
        );
        colFlds.objAdd(flddNew);
    }

    //
    //  These are the same, the V2 ones just have the device class prefix.
    //  The DBSerialNum and LoadStatus fields weren't there in V1 but it
    //  doesn't hurt anything to include them V1 drivers, and it makes things
    //  simpler since we don't have to special case them.
    //
    flddNew.Set
    (
        strFldDBSerialNum
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Read
    );
    colFlds.objAdd(flddNew);

    flddNew.Set
    (
        strFldLoadStatus
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Read
        , L"Enum: Initializing, Loading, Ready, Error"
    );
    colFlds.objAdd(flddNew);

    flddNew.Set
    (
        strFldReloadDB
        , tCQCKit::EFldTypes::Boolean
        , tCQCKit::EFldAccess::Write
    );
    colFlds.objAdd(flddNew);

    flddNew.Set
    (
        strFldStatus
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Read
    );
    colFlds.objAdd(flddNew);

    flddNew.Set
    (
        strFldTitleCnt
        , tCQCKit::EFldTypes::Card
        , tCQCKit::EFldAccess::Read
    );
    colFlds.objAdd(flddNew);

    //
    //  Now register our fields with the base driver class, and go back
    //  and look up the ids.
    //
    SetFields(colFlds);
    m_c4FldId_DBSerialNum = pflddFind(strFldDBSerialNum, kCIDLib::True)->c4Id();
    m_c4FldId_LoadStatus = pflddFind(strFldLoadStatus, kCIDLib::True)->c4Id();
    m_c4FldId_ReloadDB = pflddFind(strFldReloadDB, kCIDLib::True)->c4Id();
    m_c4FldId_Status = pflddFind(strFldStatus, kCIDLib::True)->c4Id();
    m_c4FldId_TitleCnt = pflddFind(strFldTitleCnt, kCIDLib::True)->c4Id();

    if (!bV2)
        m_c4FldId_Loaded = pflddFind(strFldLoaded, kCIDLib::True)->c4Id();

    //
    //  Init the status field to indicate not loaded. The loaded field will
    //  have a default initial value of false, which is what we want. And set
    //  the load status field to loading.
    //
    bStoreStringFld
    (
        m_c4FldId_Status
        , facCQSLRepoS().strMsg(kCQSLRMsgs::midStatus_NotLoaded)
        , kCIDLib::True
    );

    bStoreStringFld
    (
        m_c4FldId_LoadStatus, L"Loading", kCIDLib::True
    );

    //
    //  Do basic init on the repository, to get the header and category list
    //  stuff loaded, and to create all the directories if this is a new
    //  repository. We do that here synchronously. The other database
    //  loading stuff is done in the usual async way later during the
    //  WaitConfig phase.
    //
    if (!bInitRepo())
        return tCQCKit::EDrvInitRes::Failed;

    //
    //  If we've not created our management interface yet, then do that,
    //  and register it for auto-rebinding.
    //
    if (!m_porbsMgrIntf)
    {
        m_porbsMgrIntf = new TCQSLRepoMgrImpl(this);
        facCIDOrb().RegisterObject(m_porbsMgrIntf, tCIDLib::EAdoptOpts::Adopt);

        TString strDescr(L"CQSL Repo Mgr - ");
        strDescr.Append(strMoniker());

        facCIDOrbUC().RegRebindObj
        (
            m_porbsMgrIntf->ooidThis(), m_strMgrBinding, strDescr
        );
    }

    // We'll try to get a first load of the database before we come online
    return tCQCKit::EDrvInitRes::WaitConfig;
}


//
//  For now, nothing for us to do.
//
tCQCKit::ECommResults TCQSLRepoSDriver::ePollDevice(TThread& thrCaller)
{
    return tCQCKit::ECommResults::Success;
}


tCIDLib::TVoid TCQSLRepoSDriver::ReleaseCommResource()
{
    // Nothing to release
}


tCIDLib::TVoid TCQSLRepoSDriver::TerminateImpl()
{
    //
    //  Unbind our manager interface and unregister it from the server.
    //
    //  Remove it from the auto-rebinder first, or it could rebind it
    //  before we get it removed. It may already be removed, if we are
    //  being called because CQCServer is stopping. It would only still be
    //  registered if we are being unloaded remotely by a user.
    //
    if (!m_strMgrBinding.bIsEmpty())
    {
        try
        {
            facCIDOrbUC().bDeregRebindObj(m_strMgrBinding);

            tCIDOrbUC::TNSrvProxy orbcNS = facCIDOrbUC().orbcNameSrvProxy(4000);
            orbcNS->RemoveBinding(m_strMgrBinding, kCIDLib::False);
        }

        catch(TError& errToCatch)
        {
            if (!errToCatch.bLogged())
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }
        }
    }

    try
    {
        if (m_porbsMgrIntf)
            facCIDOrb().DeregisterObject(m_porbsMgrIntf);
    }

    catch(TError& errToCatch)
    {
        if (!errToCatch.bLogged())
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
    }
    m_porbsMgrIntf = 0;
}


// ---------------------------------------------------------------------------
//  TCQSLRepoSDriver: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Given a media type and the id of a media item or image, this will build
//  the path to the media file. We arrange the directories so that we store no
//  more than a max number of thing stitles per directory. So they will be in
//  the form:
//
//  [base-repodir]\datatype\mediatype\XXXX\
//
//  The bUpload parameter allows us to create paths for the real locations
//  or under the upload directory, for supporting the building up of data
//  there before moving it into the repository.
//
//
//  NOTE:
//  As of 4.4.902, we only store images and the actual media content as separate
//  files, but we still accept all data types in case we need to read in any old
//  format content at some point after the upgrade.
//
tCIDLib::TVoid
TCQSLRepoSDriver::BuildPath(const   tCQCMedia::EMediaTypes  eType
                            , const tCQCMedia::EDataTypes   eDType
                            , const tCIDLib::TCard2         c2Id
                            ,       TPathStr&               pathTarget
                            ,       TPathStr&               pathMetaFl
                            ,       TPathStr&               pathMediaFl
                            , const tCIDLib::TBoolean       bUpload)
{
    tCIDLib::TCard4 c4Tmp;
    TString strTmp;

    // Build up the basic path for this data type and type
    TMediaDB& mdbSrc = m_srdbEngine.mdbInfo();
    if (bUpload)
        pathTarget = m_pathUpload;
    else
        pathTarget = m_pathStart;

    // Add the data type level
    switch(eDType)
    {
        case tCQCMedia::EDataTypes::Cat :
            pathTarget.AddLevel(L"CtDat");
            break;

        case tCQCMedia::EDataTypes::Collect :
            pathTarget.AddLevel(L"ClDat");
            break;

        case tCQCMedia::EDataTypes::Image :
            pathTarget.AddLevel(L"ImDat");
            break;

        case tCQCMedia::EDataTypes::Item :
            pathTarget.AddLevel(L"ItDat");
            break;

        case tCQCMedia::EDataTypes::TitleSet :
            pathTarget.AddLevel(L"TsDat");
            break;

        default :
            CIDAssert2(L"Unknown media data type");
            break;
    };

    // Add the media type
    pathTarget.AddLevel(tCQCMedia::strXlatEMediaTypes(eType));

    // Format the directory number and append it
    TString strNum;
    c4Tmp = c2Id / kCQSLRepoS::c4MaxObjsPerDir;
    strTmp.SetFormatted(c4Tmp, tCIDLib::ERadices::Hex);
    strNum.FormatToFld(strTmp, 4, tCIDLib::EHJustify::Right, L'0');
    pathTarget.AddLevel(strNum);

    // Build the basic file name based on the path
    pathMetaFl = pathTarget;
    strTmp.SetFormatted(c2Id, tCIDLib::ERadices::Hex);
    strNum.FormatToFld(strTmp, 4, tCIDLib::EHJustify::Right, L'0');
    pathMetaFl.AddLevel(strNum);

    //
    //   Create the media file if it's one that can represent an uploaded media
    //  file.
    //
    if (eDType == tCQCMedia::EDataTypes::Item)
    {
        pathMediaFl = pathMetaFl;
        pathMediaFl.AppendExt(L"wma");
    }

    // And now add the extension
    switch(eDType)
    {
        case tCQCMedia::EDataTypes::Cat :
            pathMetaFl.AppendExt(L"mcat");
            break;

        case tCQCMedia::EDataTypes::Collect :
            pathMetaFl.AppendExt(L"mcol");
            break;

        case tCQCMedia::EDataTypes::Image :
            pathMetaFl.AppendExt(L"mimg2");
            break;

        case tCQCMedia::EDataTypes::Item :
            pathMetaFl.AppendExt(L"mitem");
            break;

        case tCQCMedia::EDataTypes::TitleSet :
            pathMetaFl.AppendExt(L"mts");
            break;

        default :
            CIDAssert2(L"Unknown media data type");
            break;
    };
}


//
//  Checks whether the passed lease id is the id of the current lease owner.
//  If not, it throws.
//
tCIDLib::TVoid
TCQSLRepoSDriver::CheckLease(const  tCIDLib::TCh* const pszFile
                            , const tCIDLib::TCard4     c4Line
                            , const TString&            strLease)
{
    if (strLease != m_strClientLeaseId)
    {
        facCQSLRepoS().ThrowErr
        (
            pszFile
            , c4Line
            , kCQSLRErrs::errcGen_NotLeaseHolder
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Locked
            , strMoniker()
        );
    }

    // It's good, so update the time stamp
    m_enctLastClientLease = TTime::enctNow();
}


