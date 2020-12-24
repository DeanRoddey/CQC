//
// FILE NAME: CQCBulkLoader_ThisFacility.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/15/2010
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
//  This file implements the facility class of the program, which has the
//  entry point and most of the work, though some grunt work is split out to
//  a second file.
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
#include "CQCBulkLoader.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TFacCQCBulkLoader,TGUIFacility)


// ---------------------------------------------------------------------------
//   CLASS: TFacCQCBulkLoader
//  PREFIX: fac
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TFacCQCBulkLoader: Constructors and Destructor
// ---------------------------------------------------------------------------
TFacCQCBulkLoader::TFacCQCBulkLoader() :

    TFacility
    (
        L"CQCBulkLoader"
        , tCIDLib::EModTypes::Exe
        , kCIDLib::c4MajVersion
        , kCIDLib::c4MinVersion
        , kCIDLib::c4Revision
        , tCIDLib::EModFlags::None
    )
    , m_bLoadFldExtracts(kCIDLib::False)
    , m_c4LineNum(0)
    , m_c4VersionNum(0)
    , m_colDrivers()
    , m_colDupDrivers(109, TStringKeyOps())
    , m_colDupKeys(109, TStringKeyOps())
    , m_colHosts(109, TStringKeyOps())
    , m_strBlk_CfgInfo(L"CFGINFO=")
    , m_strBlk_CfgInfoEnd(L"END CFGINFO")
    , m_strBlk_DriverEnd(L"END DRIVER")
    , m_strBlk_Drivers(L"DRIVERS=")
    , m_strBlk_DriversEnd(L"END DRIVERS")
    , m_strBlk_Events(L"EVENTS=")
    , m_strBlk_EventsEnd(L"END EVENTS")
    , m_strBlk_EventScope(L"EVSCOPE=")
    , m_strBlk_EventScopeEnd(L"END EVSCOPE")
    , m_strBlk_Images(L"IMAGES=")
    , m_strBlk_ImagesEnd(L"END IMAGES")
    , m_strBlk_ImageScope(L"IMGSCOPE=")
    , m_strBlk_ImageScopeEnd(L"END IMGSCOPE")
    , m_strBlk_Ignore(L"IGNORE=")
    , m_strBlk_IgnoreEnd(L"END IGNORE")
    , m_strBlk_Prompts(L"PROMPTS=")
    , m_strBlk_PromptEnd(L"END PROMPT")
    , m_strBlk_PromptsEnd(L"END PROMPTS")
    , m_strBlk_SubKeyEnd(L"END SUBKEY")
    , m_strBlk_Unload(L"UNLOAD=")
    , m_strBlk_UnloadEnd(L"END UNLOAD")

    , m_strKey_ConnInfo(L"CONNINFO")
    , m_strKey_Descr(L"DESCRIPTION")
    , m_strKey_Driver(L"DRIVER")
    , m_strKey_Event(L"EVENT")
    , m_strKey_EventEnd(L"END EVENT")
    , m_strKey_FldExport(L"FLDEXPORT")
    , m_strKey_Host(L"HOST")
    , m_strKey_Image(L"IMAGE")
    , m_strKey_Key(L"KEY")
    , m_strKey_Make(L"MAKE")
    , m_strKey_Model(L"MODEL")
    , m_strKey_Name(L"NAME")
    , m_strKey_Pattern(L"PATTERN")
    , m_strKey_Prompt(L"PROMPT")
    , m_strKey_SrcDir(L"SRCDIR")
    , m_strKey_SubKey(L"SUBKEY")
    , m_strKey_TarName(L"TARNAME")
    , m_strKey_TarScope(L"TARSCOPE")
    , m_strKey_Type(L"TYPE")
    , m_strKey_Value(L"VALUE")
    , m_strKey_Version(L"VERSION")
    , m_strUserPref(L"/User/")

    , m_strmOut(TSysInfo::strmOut())
{
}

TFacCQCBulkLoader::~TFacCQCBulkLoader()
{
}


// ---------------------------------------------------------------------------
//  TFacCQCBulkLoader: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::EExitCodes
TFacCQCBulkLoader::eMainThread(TThread& thrThis, tCIDLib::TVoid*)
{
    // Let our caller go
    thrThis.Sync();

    try
    {
        m_strmOut   << L"\nCQC Bulk Loader\n"
                    << L"Copyright (c) Charmed Quark Systems\n\n"
                    << kCIDLib::FlushIt;


        // Make sure we got reasonable input
        if (!bInitialize())
        {
            ShowUsage();
            return tCIDLib::EExitCodes::Normal;
        }

        //
        //  Try to parse the source file. All the code for this work is in
        //  the secondary facility cpp file, to keep this file cleaner.
        //
        if (!bParseCfgFile())
            return tCIDLib::EExitCodes::Normal;

        m_strmOut << L"\nConfiguration successfully parsed. Starting "
                     L"validation phase..." << kCIDLib::EndLn;

        //
        //  Validate everything. We make sure that we can download the
        //  manifests for drivers, validate the prompts using that info,
        //  check that the image files exist, etc...
        //
        //  There's a fair bit of code involved, so this is in a third
        //  facilty cpp file.
        //
        if (!bValidateCfg())
            return tCIDLib::EExitCodes::Normal;
        m_strmOut << L"Configuration validation is complete\n" << kCIDLib::EndLn;

        //
        //  If any drivers are defined, we may need to unload any that are
        //  going to be reloaded. And we may have some that are defined just
        //  for unload.

        // If any drivers, deal with that
        if (!m_colDrivers.bIsEmpty() || !m_colUnloadDrvs.bIsEmpty())
        {
            m_strmOut << L"Processing drivers..."
                      << kCIDLib::EndLn;
            UpdateDrivers();
            m_strmOut << L"Driver processing has completed.\n" << kCIDLib::EndLn;
        }

        // If there were any field extracts, then load those
        if (m_bLoadFldExtracts)
        {
            m_strmOut << L"\nLoading field extracts..." << kCIDLib::EndLn;
            LoadFldExtracts();
            m_strmOut << L"Field loading completed\n" << kCIDLib::EndLn;
        }

        // If any events to laod, then do that
        if (!m_colEvents.bIsEmpty())
        {
            m_strmOut << L"Loading scheduled/triggered events" << kCIDLib::EndLn;
            LoadEvents();
            m_strmOut << L"Event loading completed\n" << kCIDLib::EndLn;
        }

        // If any images to laod, then do that
        if (!m_colImages.bIsEmpty())
        {
            m_strmOut << L"Loading Images" << kCIDLib::EndLn;
            LoadImages();
            m_strmOut << L"Image loading completed\n" << kCIDLib::EndLn;
        }

        m_strmOut << L"\nThe process completed successfully\n" << kCIDLib::EndLn;
    }

    catch(const EErrors)
    {
        m_strmOut << L"  An error occured, the load has been aborted.\n"
                  << kCIDLib::EndLn;
    }

    catch(const TError& errToCatch)
    {
        m_strmOut << L"  An error occured, the load has been aborted\n    "
                  << errToCatch << kCIDLib::EndLn;
    }
    return tCIDLib::EExitCodes::Normal;
}


// ---------------------------------------------------------------------------
//  TFacCQCBulkLoader: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Called on startup to get the parms and do initialization. We return true
//  if all is well, else false and we'll just exit.
//
tCIDLib::TBoolean TFacCQCBulkLoader::bInitialize()
{
    //
    //  First get the standard CQC environmental info. That's required to do
    //  most everything.
    //
    TString strFailReason;
    if (!facCQCKit().bLoadEnvInfo(strFailReason, kCIDLib::True))
    {
        m_strmOut << strFailReason << kCIDLib::EndLn;
        return kCIDLib::False;
    }

    // And now look for our program specific info
    const tCIDLib::TCard4 c4ArgCnt = TSysInfo::c4CmdLineParmCount();
    if (c4ArgCnt != 3)
        return kCIDLib::False;

    m_pathCfgFile = TSysInfo::strCmdLineParmAt(0);
    m_strUserName = TSysInfo::strCmdLineParmAt(1);
    m_strPassword = TSysInfo::strCmdLineParmAt(2);

    // Make sure we got required info
    if (m_pathCfgFile.bIsEmpty())
    {
        m_strmOut   << L"The source configuration file was not provided"
                    << kCIDLib::EndLn;
        return kCIDLib::False;
    }

    if (m_strPassword.bIsEmpty() || m_strUserName.bIsEmpty())
    {
        m_strmOut   << L"You must provide a CQC administrative user name and password"
                    << kCIDLib::EndLn;
        return kCIDLib::False;
    }

    // Make sure that the source file is found
    if (!TFileSys::bExists(m_pathCfgFile))
    {
        m_strmOut   << L"The source configuration file was not found"
                    << kCIDLib::EndLn;
        return kCIDLib::False;
    }

    // Crank up the client side ORB
    facCIDOrb().InitClient();

    // See if we can log in. If not, no need to do anything else
    if (!bLogin())
        return kCIDLib::False;

    return kCIDLib::True;
}


//
//  Called on startup to log into the system. We'll check that the account
//  has admin level rights, else we can't go forward.
//
tCIDLib::TBoolean TFacCQCBulkLoader::bLogin()
{
    // Get a reference to the security server
    tCQCKit::TSecuritySrvProxy orbcSS = facCQCKit().orbcSecuritySrvProxy();

    TCQCSecToken    sectToFill;
    TCQCUserAccount uaccToFill;

    // And ask it to give us a challenge
    TCQCSecChallenge seccLogon;
    tCIDLib::TBoolean bRet = kCIDLib::False;
    if (orbcSS->bLoginReq(m_strUserName, seccLogon))
    {
        // Lets do a hash of the user's password
        TMD5Hash mhashPW;
        TMessageDigest5 mdigTmp;
        mdigTmp.StartNew();
        mdigTmp.DigestStr(m_strPassword);
        mdigTmp.Complete(mhashPW);

        // And use that to validate the challenge
        seccLogon.Validate(mhashPW);

        // And send that back to get a security token, assuming its legal
        tCQCKit::ELoginRes eRes;
        bRet = orbcSS->bGetSToken(seccLogon, sectToFill, uaccToFill, eRes);

        // If it worked, store the security token for our process
        if (bRet)
            m_cuctxToUse.Set(uaccToFill, sectToFill);
    }

    if (!bRet)
    {
        m_strmOut << L"Login failed, please check your user name/password\n"
                  << kCIDLib::EndLn;
    }
    return bRet;
}



//
//  This guy loads up any defined event trees. We have a main method that loops through
//  all of the trees, and a helper that recurses through each tree.
//
tCIDLib::TVoid TFacCQCBulkLoader::LoadEvents()
{
    const tCIDLib::TCard4 c4BlkCount = m_colEvents.c4ElemCount();

    // If there aren't any images, then nothing to do
    if (!c4BlkCount)
        return;

    // We need a data server client for this
    TDataSrvClient dsclTar;

    //
    //  OK, let's go through each tree and load them up. For each one, we get the
    //  starting source dir and target scope, then we call a recursive helper to
    //  iterate through that tree.
    //
    tCIDLib::TBoolean   bReloadSched = kCIDLib::False;
    tCIDLib::TBoolean   bReloadTrig = kCIDLib::False;
    TPathStr            pathSrcDir;
    TString             strTarScope;
    for (tCIDLib::TCard4 c4BlkIndex = 0; c4BlkIndex < c4BlkCount; c4BlkIndex++)
    {
        const TBLEvScopeInfo& scpiRoot = m_colEvents[c4BlkIndex];

        // Remember which types we saw so we can only reload those we affected
        if (scpiRoot.eType() == tCQCRemBrws::EDTypes::SchEvent)
            bReloadSched = kCIDLib::True;
        else
            bReloadTrig = kCIDLib::True;

        //
        //  We have special issues here. We have to make sure that the full starting
        //  target scope exists. After this, it's just one sub-scope at a time.
        //
        dsclTar.MakePath(scpiRoot.strTarScope(), scpiRoot.eType(), m_cuctxToUse.sectUser());

        pathSrcDir = scpiRoot.strName();
        strTarScope = scpiRoot.strTarScope();
        LoadEventTree(dsclTar, scpiRoot, pathSrcDir, strTarScope);
    }

    // Ask the event server to reload his list now to get into sync
    tCQCEvCl::TEventSrvProxy orbcES = facCQCEvCl().orbcEventSrvProxy();
    if (bReloadTrig)
    {
        m_strmOut   << L"Asking event server to reload triggered events..."
                    << kCIDLib::FlushIt;
        orbcES->ReloadList(tCQCKit::EEvSrvTypes::TrgEvent);
        TThread::Sleep(5000);
        m_strmOut << L"Done" << kCIDLib::EndLn;
    }

    if (bReloadSched)
    {
        m_strmOut   << L"Asking event server to reload scheduled events..."
                    << kCIDLib::FlushIt;
        orbcES->ReloadList(tCQCKit::EEvSrvTypes::SchEvent);
        TThread::Sleep(5000);
        m_strmOut << L"Done" << kCIDLib::EndLn;
    }
}


tCIDLib::TVoid
TFacCQCBulkLoader::LoadEventTree(       TDataSrvClient& dsclTar
                                , const TBLEvScopeInfo& scpiParent
                                ,       TPathStr&       pathSrcDir
                                ,       TString&        strTarScope)
{
    //
    //  OK, let's start enumerating this scope level. Save the current
    //  path lengths, so that we can cap them back on each round.
    //
    const tCIDLib::TCard4 c4SrcLen = pathSrcDir.c4Length();
    const tCIDLib::TCard4 c4TarLen = strTarScope.c4Length();

    // Do any sub-directories first
    TPathStr pathTmp;
    const tCIDLib::TCard4 c4SubDirCnt = scpiParent.c4SubDirCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4SubDirCnt; c4Index++)
    {
        const TBLEvScopeInfo& scpiCur = scpiParent.scpiAt(c4Index);

        //
        //  Cap the paths back to the original length each time and build
        //  up the nested paths.
        //
        pathSrcDir.CapAt(c4SrcLen);
        strTarScope.CapAt(c4TarLen);

        // Make sure this scope exists
        if (!dsclTar.bScopeExists(strTarScope, scpiCur.strName(), scpiParent.eType()))
        {
            dsclTar.MakeNewScope
            (
                strTarScope, scpiCur.strName(), scpiParent.eType(), m_cuctxToUse.sectUser()
            );
        }

        pathSrcDir.AddLevel(scpiCur.strName());
        facCQCRemBrws().AddPathComp(strTarScope, scpiCur.strName());

        // And now recurse on this guy
        LoadEventTree(dsclTar, scpiCur, pathSrcDir, strTarScope);
    }

    tCIDLib::TCard4         c4SerNum;
    tCIDLib::TEncodedTime   enctLastChange;
    TString                 strExt;
    const tCIDLib::TCard4   c4EvCnt = scpiParent.c4EvCount();
    tCIDLib::TKVPFList      colXMeta;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4EvCnt; c4Index++)
    {
        const TBLEventInfo& eviCur = scpiParent.eviAt(c4Index);

        //  Cap the paths back to the original length each time and build up the paths.
        pathSrcDir.CapAt(c4SrcLen);
        pathSrcDir.AddLevel(eviCur.strName());
        strTarScope.CapAt(c4TarLen);
        facCQCRemBrws().AddPathComp(strTarScope, eviCur.strName());

        m_strmOut << L"Loading ";
        if (scpiParent.eType() == tCQCRemBrws::EDTypes::SchEvent)
            m_strmOut << L"scheduled";
        else
            m_strmOut << L"triggered";
        m_strmOut << L" event '" << strTarScope << L"..." << kCIDLib::FlushIt;

        // Based on the type, we'll add the correct extension to the source path
        pathSrcDir.AppendExt(facCQCRemBrws().strFlTypeExt(scpiParent.eType()));

        TBinFileInStream strmSrc;
        strmSrc.Open
        (
            pathSrcDir
            , tCIDLib::ECreateActs::OpenIfExists
            , tCIDLib::EFilePerms::Default
            , tCIDLib::EFileFlags::SequentialScan
        );


        if (scpiParent.bIsSchedEv())
        {
            TCQCSchEvent csrcSchEv;
            strmSrc >> csrcSchEv;

            dsclTar.WriteScheduledEvent
            (
                strTarScope
                , c4SerNum
                , enctLastChange
                , csrcSchEv
                , kCQCRemBrws::c4Flag_OverwriteOK | kCQCRemBrws::c4Flag_NoDataCache
                , colXMeta
                , m_cuctxToUse.sectUser()
            );
        }
         else
        {
            TCQCTrgEvent csrcTrgEv;
            strmSrc >> csrcTrgEv;
            dsclTar.WriteTriggeredEvent
            (
                strTarScope
                , c4SerNum
                , enctLastChange
                , csrcTrgEv
                , kCQCRemBrws::c4Flag_OverwriteOK | kCQCRemBrws::c4Flag_NoDataCache
                , colXMeta
                , m_cuctxToUse.sectUser()
            );
        }

        m_strmOut << L" (Done)" << kCIDLib::EndLn;

        // Pause a bit
        TThread::Sleep(250);
    }
}



//
//  If there were any field extracts referenced, this is called after the
//  loading is completed. We run through and load them up.
//
tCIDLib::TVoid TFacCQCBulkLoader::LoadFldExtracts()
{
    // Loop through all the defined drivers
    const tCIDLib::TCard4   c4Count = m_colDrivers.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TBLDrvInfo& drviCur = m_colDrivers[c4Index];
        if (drviCur.strFldExport().bIsEmpty())
            continue;

        m_strmOut << L"Loading fields to driver '" << drviCur.strMoniker()
                  << L"' ..." << kCIDLib::FlushIt;

        // Ok, this guy has one, so let's load it. So read in the file contents
        TVarDrvCfg cfgToUpload;
        try
        {
            // If the path is relative, make it relative to where the config file was
            TPathStr pathExp(drviCur.strFldExport());
            if (!pathExp.bIsFullyQualified())
                pathExp.AddToBasePath(m_pathCfgRel);

            cfgToUpload.ParseFromXML(pathExp);

            // Format it out to a memory buffer
            THeapBuf mbufCfg(8192UL);
            TBinMBufOutStream strmCfg(&mbufCfg);
            strmCfg << TString(L"FullCfg") << cfgToUpload << kCIDLib::FlushIt;

            // Now send this to the driver
            tCQCKit::TCQCSrvProxy orbcTar = facCQCKit().orbcCQCSrvAdminProxy(drviCur.strMoniker());
            orbcTar->SetConfig
            (
                drviCur.strMoniker()
                , strmCfg.c4CurSize()
                , mbufCfg
                , m_cuctxToUse.sectUser()
                , tCQCKit::EDrvCmdWaits::Wait
            );

            // Sleep a little
            TThread::Sleep(2000);

            m_strmOut << L" (Done)" << kCIDLib::EndLn;
        }

        catch(const TError& errToCatch)
        {
            m_strmOut << L"\n!!An error occured loading field extract:\n"
                      << errToCatch << kCIDLib::DNewLn << kCIDLib::EndLn;
        }
    }
}


//
//  This guy loads up any defined image lists. We have a main method that
//  loops through all of the trees, and a helper that recurses through each
//  tree.
//
tCIDLib::TVoid TFacCQCBulkLoader::LoadImages()
{
    const tCIDLib::TCard4 c4BlkCount = m_colImages.c4ElemCount();

    // If there aren't any images, then nothing to do
    if (!c4BlkCount)
        return;

    // We need a data server client for this
    TDataSrvClient dsclTar;

    // We provide some buffers for temp use
    TBinMBufOutStream   mbufImg(0x200000);
    TBinMBufOutStream   mbufThumb(0x20000);

    //
    //  OK, let's go through each tree and load them up. For each one,
    //  we get the starting source dir and target scope, then we call a
    //  recursive helper to iterate through that tree.
    //
    TPathStr pathSrc;
    TPathStr pathTar;
    for (tCIDLib::TCard4 c4BlkIndex = 0; c4BlkIndex < c4BlkCount; c4BlkIndex++)
    {
        // Get the root scope and the starting src path
        const TBLImgScopeInfo& scpiRoot = m_colImages[c4BlkIndex];
        pathSrc = scpiRoot.strName();
        pathTar = scpiRoot.strTarScope();

        //
        //  We have special issues here. We have to make sure that the full starting
        //  target scope exists. After this, it's just one sub-scope at a time.
        //
        dsclTar.MakePath
        (
            scpiRoot.strTarScope(), tCQCRemBrws::EDTypes::Image, m_cuctxToUse.sectUser()
        );
        LoadImageTree(dsclTar, scpiRoot, pathSrc, pathTar, mbufImg, mbufThumb);
    }
}


tCIDLib::TVoid
TFacCQCBulkLoader::LoadImageTree(       TDataSrvClient&     dsclTar
                                , const TBLImgScopeInfo&    scpiParent
                                ,       TPathStr&           pathSrcDir
                                ,       TString&            strTarScope
                                ,       TBinMBufOutStream&  strmImg
                                ,       TBinMBufOutStream&  strmThumb)
{
    //
    //  OK, let's start enumerating this scope level. Save the current
    //  path lengths, so that we can cap them back on each round.
    //
    const tCIDLib::TCard4 c4SrcLen = pathSrcDir.c4Length();
    const tCIDLib::TCard4 c4TarLen = strTarScope.c4Length();

    // Do any sub-directories first
    const tCIDLib::TCard4 c4SubDirCnt = scpiParent.c4SubDirCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4SubDirCnt; c4Index++)
    {
        const TBLImgScopeInfo& scpiCur = scpiParent.scpiAt(c4Index);

        //
        //  Cap the paths back to the original length each time and build
        //  up the nested paths.
        //
        pathSrcDir.CapAt(c4SrcLen);
        strTarScope.CapAt(c4TarLen);

        // Make sure this scope exists
        if (!dsclTar.bScopeExists(strTarScope, scpiCur.strName(), tCQCRemBrws::EDTypes::Image))
        {
            dsclTar.MakeNewScope
            (
                strTarScope, scpiCur.strName()
                , tCQCRemBrws::EDTypes::Image
                , m_cuctxToUse.sectUser()
            );
        }

        pathSrcDir.AddLevel(scpiCur.strName());
        facCQCRemBrws().AddPathComp(strTarScope, scpiCur.strName());

        // And now recurse on this guy
        LoadImageTree(dsclTar, scpiCur, pathSrcDir, strTarScope, strmImg, strmThumb);
    }

    TString strTarImg;
    TString strExt;
    strTarScope.CapAt(c4TarLen);
    tCIDLib::TKVPFList colXMeta;
    const tCIDLib::TCard4 c4ImgCnt = scpiParent.c4ImgCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ImgCnt; c4Index++)
    {
        const TBLImageInfo& imgiCur = scpiParent.imgiAt(c4Index);

        pathSrcDir.CapAt(c4SrcLen);
        pathSrcDir.AddLevel(imgiCur.strName());

        // We have to remove the extension for the target
        strTarImg = strTarScope;
        facCQCRemBrws().AddPathComp(strTarImg, imgiCur.strName());
        strTarImg.bCapAtChar(kCIDLib::chPeriod);

        //
        //  Based on the file extension we'll load up the image data. If it's
        //  not a PNG, we convert it to a PNG. We already checked the
        //  extensions during the validation phase.
        //
        TBinFileInStream strmSrc;
        strmSrc.Open
        (
            pathSrcDir
            , tCIDLib::ECreateActs::OpenIfExists
            , tCIDLib::EFilePerms::Default
            , tCIDLib::EFileFlags::SequentialScan
        );

        if (!pathSrcDir.bQueryExt(strExt))
        {
            // This shouldn't happen, but be careful
            m_strmOut << L">>Image '" << pathSrcDir << L"' doesn't have a valid "
                         L"extension" << kCIDLib::EndLn;
            throw(EError_Internal);
        }

        if (strExt.bCompareI(L"JPEG") || strExt.bCompareI(L"JPG"))
        {
            strmSrc >> m_imgJPEG;
            m_imgPNG = m_imgJPEG;
        }
         else if (strExt.bCompareI(L"BMP"))
        {
            strmSrc >> m_imgBitmap;
            m_imgPNG = m_imgBitmap;
        }
         else if (strExt.bCompareI(L"PNG"))
        {
            strmSrc >> m_imgPNG;
        }
         else
        {
            // This shouldn't happen, but be careful
            m_strmOut << L">>Image '" << pathSrcDir << L"' doesn't have a valid "
                         L"extension" << kCIDLib::EndLn;
            throw(EError_Internal);
        }

        //
        //  OK, let's upload it. First we package it up and get the image and
        //  thumbs sizes back, this takes a lot of burden off of us to replicate that
        //  functionality of creating the appropriate thumb.
        //
        TSize szThumb;
        facCQCKit().PackageImg(m_imgPNG, strmImg, strmThumb, szThumb);

        // And now let's send it to the server
        m_strmOut << L"Uploading image '" << pathSrcDir << L"'..." << kCIDLib::FlushIt;

        // Tell the server not to add to memory cache if it's not already cached
        tCIDLib::TCard4 c4NewSerNum;
        tCIDLib::TEncodedTime enctLastChange;
        dsclTar.WriteImage
        (
            strTarImg
            , c4NewSerNum
            , enctLastChange
            , strmImg.mbufData()
            , strmImg.c4CurSize()
            , strmThumb.mbufData()
            , strmThumb.c4CurSize()
            , m_imgPNG.ePixFmt()
            , m_imgPNG.eBitDepth()
            , m_imgPNG.szImage()
            , szThumb
            , kCQCRemBrws::c4Flag_OverwriteOK | kCQCRemBrws::c4Flag_NoDataCache
            , colXMeta
            , m_cuctxToUse.sectUser()
        );

        m_strmOut << L" (Done)" << kCIDLib::EndLn;

        // Pause a bit
        TThread::Sleep(250);
    }
}


// Just shows the correct cmd line parms
tCIDLib::TVoid TFacCQCBulkLoader::ShowUsage()
{
    m_strmOut << L"    \n    Usage:\n"
                 L"      CQCBulkLoader cfgfile username password\n\n"
                 L"      The config file describes what to load\n"
                 L"      The username/password must be for an administrative\n"
                 L"          CQC account\n"
              << kCIDLib::EndLn;
}


//
//  Here we go through and unload an existing drivers that have the same
//  moniker as the drivers we are going to unload. We unload them all before
//  we bother starting to load new ones. We also can have drivers that are
//  just marked for unload.
//
tCIDLib::TVoid TFacCQCBulkLoader::UpdateDrivers()
{
    tCIDLib::TCard4 c4VerNum;
    tCIDLib::TCard4 c4Count;
    TCQCDriverCfg   cqcdcCur;
    TString         strHost;

    tCQCKit::TInstSrvProxy orbcIS = facCQCKit().orbcInstSrvProxy();

    //
    //  Loop through the drivers to unload. If they are loaded, unload them. Either way
    //  we remove their configuration from the MS's driver config list
    //
    c4Count = m_colUnloadDrvs.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TString& strDrv = m_colUnloadDrvs[c4Index];

        m_strmOut << L"Removing driver '" << strDrv << L"'..." << kCIDLib::FlushIt;

        // See if the driver is loaded and what host is hosting it
        tCQCKit::TCQCSrvProxy orbcTar;
        if (facCQCKit().bDrvIsLoaded(strDrv, strHost))
        {
            m_strmOut << L" unloading from host '" << strHost << L"'..." << kCIDLib::FlushIt;

            // Get a proxy for the CQCServer that's hosting this guy and ask to unload it
            orbcTar.SetPointer(facCQCKit().porbcCQCSrvAdminProxy(strHost));
            orbcTar->UnloadDriver(strDrv, m_cuctxToUse.sectUser());
        }

        //
        //  And remove the config from the master server. Even if not loaded it still may
        //  be in the MS config. The host that loads it may just not be running.
        //
        //  But it may not be configured as well, so we want to first see if it is configured
        //  so that any failure to remove the config is a known real error.
        //
        if (orbcIS->bDrvCfgDriverPresent(strDrv, strHost, m_cuctxToUse.sectUser()))
        {
            m_strmOut << L" removing from MS..." << kCIDLib::FlushIt;
            orbcIS->DrvCfgRemoveDrv(strDrv, strHost, c4VerNum, m_cuctxToUse.sectUser());
        }

        // If the driver was loaded, wait a while for it to complete unloading.
        if (orbcTar.pobjData())
        {
            while (facCQCKit().bDrvIsLoaded(strDrv, strHost))
                TThread::Sleep(2000);
        }
        TThread::Sleep(2000);

        m_strmOut << L"  (Done)" << kCIDLib::EndLn;
    }

    //
    //  Do the same for any that we are going to load. This way, we are sure we are starting
    //  from scratch.
    //
    c4Count = m_colDrivers.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TBLDrvInfo& drviCur = m_colDrivers[c4Index];
        const TString& strMoniker = drviCur.strMoniker();

        m_strmOut << L"Removing driver '" << strMoniker << L"'..." << kCIDLib::FlushIt;

        tCQCKit::TCQCSrvProxy orbcTar;
        if (facCQCKit().bDrvIsLoaded(strMoniker))
        {
            m_strmOut   << L" unloading from host '"
                        << L"'..." << drviCur.strHost() << L"'..." << kCIDLib::FlushIt;

            orbcTar.SetPointer(facCQCKit().porbcCQCSrvAdminProxy(drviCur.strHost()));
            orbcTar->UnloadDriver(strMoniker, m_cuctxToUse.sectUser());
            orbcIS->DrvCfgRemoveDrv
            (
                strMoniker, drviCur.strHost(), c4VerNum, m_cuctxToUse.sectUser()
            );
        }

        // In this case we were told the host but this gives it back to us so we can validate
        if (orbcIS->bDrvCfgDriverPresent(strMoniker, strHost, m_cuctxToUse.sectUser()))
        {
            // Sanity check it shows up the same as what we were told it is
            if (!strHost.bCompareI(drviCur.strHost()))
            {
                m_strmOut   << L"Driver " << strMoniker
                            << L" was not configured for the indicated host '"
                            << drviCur.strHost() << L"'" << kCIDLib::EndLn;
                throw(EError_Data);
            }

            m_strmOut << L" removing from MS..." << kCIDLib::FlushIt;
            orbcIS->DrvCfgRemoveDrv(strMoniker, strHost, c4VerNum, m_cuctxToUse.sectUser());
        }

        if (orbcTar.pobjData())
        {
            while (facCQCKit().bDrvIsLoaded(strMoniker, strHost))
                TThread::Sleep(2000);
        }
        TThread::Sleep(2000);

        m_strmOut << L"  (Done)" << kCIDLib::EndLn;
    }

    //
    //  Now let's go through the drivers that we are to load, and upload their configuration
    //  to the master server. As we do each one, tell the affected host to load the driver
    //  with the uploaded configuration.
    //
    c4Count = m_colDrivers.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TBLDrvInfo& drviCur = m_colDrivers[c4Index];

        m_strmOut   << L"Loading driver '" << drviCur.strMoniker()
                    << L"' to host '" << drviCur.strHost() << L"'..." << kCIDLib::FlushIt;

        tCQCKit::TCQCSrvProxy orbcTar(facCQCKit().porbcCQCSrvAdminProxy(drviCur.strHost()));

        orbcIS->DrvCfgSetConfig
        (
            drviCur.strMoniker()
            , drviCur.strHost()
            , drviCur.cqcdcInfo()
            , kCIDLib::True
            , c4VerNum
            , m_cuctxToUse.sectUser()
        );

        // Ask it to load this driver
        tCIDLib::TStrList colList(1);
        colList.objAdd(drviCur.strMoniker());
        orbcTar->bLoadDrivers(colList, m_cuctxToUse.sectUser());

        // Wait a bit for it to come online
        tCIDLib::TEncodedTime enctNow = TTime::enctNow();
        const tCIDLib::TEncodedTime enctEnd(enctNow + (5 * kCIDLib::enctOneSecond));
        while (enctNow < enctEnd)
        {
            // Get the driver status
            tCIDLib::TCard4     c4ThreadId;
            tCQCKit::EDrvStates eState;
            tCQCKit::EVerboseLvls eVerbose;
            try
            {
                orbcTar->QueryDriverState(drviCur.strMoniker(), eState, eVerbose, c4ThreadId);

                // If it's online, then we are done
                if (eState == tCQCKit::EDrvStates::Connected)
                    break;

                // Pause a bit
                TThread::Sleep(500);
            }

            catch(...)
            {
                // Just eat it and keep waiting
            }
            enctNow = TTime::enctNow();
        }

        // Sleep a little bit between them to allow for settle time
        TThread::Sleep(2000);

        if (enctNow >= enctEnd)
            m_strmOut << L"  (Did not come online)" << kCIDLib::EndLn;
        else
            m_strmOut << L"  (Done)" << kCIDLib::EndLn;
    }
}
