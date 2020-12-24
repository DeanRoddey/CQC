//
// FILE NAME: CmpDrivers.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/07/2004
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
#include "CmpDrivers.hpp"


// ---------------------------------------------------------------------------
//  Do the magic main module code
// ---------------------------------------------------------------------------
tCIDLib::EExitCodes eMainThreadFunc(TThread&, tCIDLib::TVoid*);
CIDLib_MainModule(TThread(L"CmpDrivers", eMainThreadFunc))


// ----------------------------------------------------------------------------
//  Local data
// ----------------------------------------------------------------------------
const TString               strTargetName(L"DriverImpl");
tCIDLib::TCard4             c4ErrCount = 0;
TOutConsole                 conOut;
TMEngFixedBaseFileResolver  mefrToUse;
TMEngStrmErrHandler         meehEngine;
TMEngStrmPrsErrHandler      meehParser;
TCIDMacroEngine             meTest;
TCQCSecToken                sectUser;


// -----------------------------------------------------------------------------
//  Local functions
// -----------------------------------------------------------------------------

static tCIDLib::TBoolean bLogin()
{
    // Get a reference to the security server
    tCQCKit::TSecuritySrvProxy orbcSS = facCQCKit().orbcSecuritySrvProxy();

    TCQCUserAccount uaccToFill;

    // And ask it to give us a challenge
    TCQCSecChallenge seccLogon;
    tCIDLib::TBoolean bRet = kCIDLib::False;
    if (orbcSS->bLoginReq(TSysInfo::strCmdLineParmAt(0), seccLogon))
    {
        // Lets do a hash of the user's password
        TMD5Hash mhashPW;
        TMessageDigest5 mdigTmp;
        mdigTmp.StartNew();
        mdigTmp.DigestStr(TSysInfo::strCmdLineParmAt(1));
        mdigTmp.Complete(mhashPW);

        // And use that to validate the challenge
        seccLogon.Validate(mhashPW);

        // And send that back to get a security token, assuming its legal
        tCQCKit::ELoginRes eRes;
        bRet = orbcSS->bGetSToken(seccLogon, sectUser, uaccToFill, eRes);
    }

    if (!bRet)
    {
        conOut  << L"Login failed, please check your user name/password\n"
                 << kCIDLib::EndLn;
    }
    return bRet;
}


//
//  This one is called for CML drivers. It just uses a global macro engine to
//  compile the passed class path. The CQC aware class loader we set on the
//  macro engine knows how to get macros from the data server.
//
static tCIDLib::TVoid CompileCMLDriver(const TString& strClassPath)
{
    // Create a parser for this parse
    TMacroEngParser meprsToUse;

    //
    //  Create a class manager, which we can do now since we have the user security
    //  info.
    //
    TCQCMEngClassMgr mecmToUse(sectUser);

    conOut << L"Compiling CML: " << strClassPath << kCIDLib::EndLn;
    TMEngClassInfo*     pmeciMain;
    if (!meprsToUse.bParse(strClassPath, pmeciMain, &meTest, &meehParser, &mecmToUse))
    {
        conOut << L"   Driver '" << strClassPath << L"' failed" << kCIDLib::EndLn;
        c4ErrCount++;
    }
}


//
//  This one is called for PDL drivers.
//
static tCIDLib::TVoid CompilePDLDriver(const TCQCDriverCfg& cqcdcTest)
{
    conOut << L"Compiling PDL: " << cqcdcTest.strServerLibName() << kCIDLib::EndLn;

    // Load up the file contents
    TPathStr pathProto(TProcEnvironment::strFind(L"CID_RESDIR"));
    pathProto.AddLevel(L"Protocols.Out");
    pathProto.AddLevel(cqcdcTest.strServerLibName());
    pathProto.AppendExt(L"CQCProto");
    TBinaryFile flSrc(pathProto);
    flSrc.Open
    (
        tCIDLib::EAccessModes::Excl_Read
        , tCIDLib::ECreateActs::OpenIfExists
        , tCIDLib::EFilePerms::Default
        , tCIDLib::EFileFlags::SequentialScan
    );
    const tCIDLib::TCard4 c4SrcBytes = tCIDLib::TCard4(flSrc.c8CurSize());

    THeapBuf mbufProto(c4SrcBytes, c4SrcBytes);
    if (flSrc.c4ReadBuffer(mbufProto, c4SrcBytes) != c4SrcBytes)
    {
        conOut  << L"Could not load protocol file " << cqcdcTest.strServerLibName()
                << kCIDLib::EndLn;
        return;
    }

    // Create the driver and put it into test mode
    TGenProtoSDriver sdrvTest(cqcdcTest);
    sdrvTest.bTestMode(kCIDLib::True);

    // Ask it to try to parse the source
    try
    {
        sdrvTest.ParseProtocol(mbufProto, c4SrcBytes);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}


static tCIDLib::TVoid
IterateDrivers(tCIDLib::TCard4& c4CMLCnt, tCIDLib::TCard4& c4PDLCnt)
{
    c4CMLCnt = 0;
    c4PDLCnt = 0;

    // All of the manifests that are shipping will be copied here by the build
    TPathStr pathResDir = TProcEnvironment::strFind(L"CID_RESDIR");
    pathResDir.AddLevel(L"Manifests.Out");

    TDirIter    diterManifests;
    TFindBuf    fndbCur;
    if (!diterManifests.bFindFirst(pathResDir, L"*.Manifest", fndbCur))
    {
        conOut << L"No manifest files were found" << kCIDLib::EndLn;
        return;
    }

    //
    //  Keep a list of makes/models and make sure we don't see the same one
    //  twice, since that isn't legal. We'll just build a single string in
    //  the form "make,model" without space, and use that as the key to a
    //  hash set.
    //
    tCIDLib::TStrHashSet colDups(29, TStringKeyOps());

    TCQCDriverCfg   cqcdcCur;
    TString         strDescr;
    TString         strKey;
    TString         strName;
    TString         strType;
    do
    {
        try
        {
            // Get out the name part for display purposes
            fndbCur.pathFileName().bQueryNameExt(strName);

            // Let our driver config object parse from this XML file
            cqcdcCur.ParseFrom(new TFileEntitySrc(fndbCur.pathFileName()));

            // Build its key and make sure no dups
            strKey = cqcdcCur.strMake();
            strKey.Append(L",");
            strKey.Append(cqcdcCur.strModel());
            if (colDups.bHasElement(strKey))
            {
                conOut  << L"The make/model '" << strKey
                        << L"' used by manifest '" << strName
                        << L"' has been used already!!" << kCIDLib::EndLn;
                        break;
            }

            // It's not a dup so add it to the list
            colDups.objAdd(strKey);

            if (cqcdcCur.eType() == tCQCKit::EDrvTypes::Macro)
            {
                //
                //  It's a CML driver, so call our local helper that does
                //  CML compiles, passing on the class path.
                //
                CompileCMLDriver(cqcdcCur.strServerLibName());
                c4CMLCnt++;
            }
             else if (cqcdcCur.eType() == tCQCKit::EDrvTypes::GenProto)
            {
                //
                //  It's a PDL driver, so call our local helper that does
                //  PDL compiles, passing on the protocol name.
                //
                CompilePDLDriver(cqcdcCur);
                c4PDLCnt++;
            }

            // If it's an IR type, test the IR device info extraction
            if ((cqcdcCur.eCategory() == tCQCKit::EDevCats::IRBlaster)
            ||  (cqcdcCur.eCategory() == tCQCKit::EDevCats::IRCombo)
            ||  (cqcdcCur.eCategory() == tCQCKit::EDevCats::IRReceiver))
            {

                facCQCIR().eExtractDevInfo(cqcdcCur, strType, strDescr);
            }
        }

        catch(const TError& errToCatch)
        {
            c4ErrCount++;
            conOut  << L"Failure processing driver '"
                    << strName << L". Error=\n"
                    << errToCatch
                    << kCIDLib::EndLn << kCIDLib::FlushIt;
        }

    }   while (diterManifests.bFindNext(fndbCur));
}


// ----------------------------------------------------------------------------
//  Functions for local use
// ----------------------------------------------------------------------------
tCIDLib::EExitCodes eMainThreadFunc(TThread& thrThis, tCIDLib::TVoid*)
{
    // We have to let our calling thread go first
    thrThis.Sync();

    //
    //  Ask CQCKit to load core environment/parm stuff. If we can't do this,
    //  then we are doomed and just have to exit.
    //
    TString strFailReason;
    if (!facCQCKit().bLoadEnvInfo(strFailReason, kCIDLib::True))
        return tCIDLib::EExitCodes::BadEnvironment;

    try
    {
        // We have to get a username/password
        if (TSysInfo::c4CmdLineParmCount() != 2)
        {
            conOut  << L"\nUsage:\n"
                    << L"    CmpDrivers username password"
                    << kCIDLib::NewEndLn;
            return tCIDLib::EExitCodes::BadParameters;
        }

        //
        //  Initialize the client side of the ORB since our class manager
        //  works via the CQC macro repository on the master server.
        //
        facCIDOrb().InitClient();

        // Try to log in
        if (!bLogin())
            return tCIDLib::EExitCodes::Normal;

        //
        //  This is a global from the generic protocol driver that forces
        //  it to do the init that normally would be forced by CQCServer
        //  loading it and calling it's factory method. Mainly it makes
        //  it create its facility object, which drivers don't fault in
        //  since they normally have an init method that can do it up front.
        //
        MakeGenProtoSFac();

        //
        //  Set up the file resolver. It's basically not of much interest
        //  since we are only going to do compiles, but we have to set one.
        //
        TPathStr pathFiles;
        TFileSys::QueryCurrentDir(pathFiles);
        pathFiles.AddLevel(L"Files");
        mefrToUse.strBasePath(pathFiles);
        meTest.SetFileResolver(&mefrToUse);

        //
        //  Point our engine and parser error handlers, which are stream
        //  based handlers, to our output console. Register the engine
        //  error handler with the engine, and set our output console as
        //  the console output target of the engine.
        //
        meehEngine.SetStream(&conOut);
        meehParser.SetStream(&conOut);
        meTest.SetErrHandler(&meehEngine);
        meTest.SetConsole(&conOut);

        //
        //  We have to install some class loaders on the macro engine
        //  facility, so that the standard CQC runtime classes and driver
        //  classes are available.
        //
        facCQCMEng().InitCMLRuntime(sectUser);
        facCQCMedia().InitCMLRuntime();
        facCQCGenDrvS().InitCMLRuntime();

        // Turn on validation, for extra sanity checking
        meTest.bValidation(kCIDLib::True);

        //
        //  And now call a local recursive function that go through all the
        //  manifest files, compile each one, and for the ones that are for
        //  CML drivers, it will get compile the indicated implementation
        //  class.
        //
        tCIDLib::TCard4 c4CMLCnt;
        tCIDLib::TCard4 c4PDLCnt;
        IterateDrivers(c4CMLCnt, c4PDLCnt);

        conOut  << kCIDLib::DNewLn
                << L"Totals\n------------------\n"
                << L"  PDL Drivers: " << c4PDLCnt << kCIDLib::NewLn
                << L"  CML Drivers: " << c4CMLCnt << kCIDLib::NewLn
                << L"        Total: " << (c4PDLCnt + c4CMLCnt) << kCIDLib::NewLn
                << L"     Failures: " << c4ErrCount << kCIDLib::NewLn
                << kCIDLib::EndLn;
    }

    // Catch any CIDLib runtime errors
    catch(const TError& errToCatch)
    {
        conOut.SetDefaultFormat();
        conOut  << L"A CIDLib runtime error occured during processing.\n"
                << L"Error: " << errToCatch << kCIDLib::NewLn << kCIDLib::EndLn;
        return tCIDLib::EExitCodes::RuntimeError;
    }

    //
    //  Kernel errors should never propogate out of CIDLib, but test for
    //  them here just in case.
    //
    catch(const TKrnlError& kerrToCatch)
    {
        conOut.SetDefaultFormat();
        conOut  << L"A kernel error occured during processing.\nError="
                << kerrToCatch.errcId() << kCIDLib::NewLn << kCIDLib::EndLn;
        return tCIDLib::EExitCodes::FatalError;
    }

    // Catch a general exception
    catch(...)
    {
        conOut.SetDefaultFormat();
        conOut  << L"A general exception occured during processing"
                << kCIDLib::NewLn << kCIDLib::EndLn;
        return tCIDLib::EExitCodes::SystemException;
    }
    return tCIDLib::EExitCodes::Normal;
}


