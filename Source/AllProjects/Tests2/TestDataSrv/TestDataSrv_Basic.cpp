//
// FILE NAME: TestDataSrv_Basic.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/28/2018
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
//  This file implements the basic dataserver I/O tests
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


// ---------------------------------------------------------------------------
//  Include underlying headers
// ---------------------------------------------------------------------------
#include    "TestDataSrv.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TTest_BasicIO,TTestFWTest)
RTTIDecls(TTest_FileSys,TTestFWTest)
RTTIDecls(TTest_MetaInfo,TTestFWTest)


// ---------------------------------------------------------------------------
//  CLASS: TTest_BasicIO
// PREFIX: tfwt
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TTest_BasicIO: Constructor and Destructor
// ---------------------------------------------------------------------------
TTest_BasicIO::TTest_BasicIO() :

    TTestFWTest
    (
        L"Basic I/O", L"Basic data server I/O tests", 4
    )
{
}

TTest_BasicIO::~TTest_BasicIO()
{
}


// ---------------------------------------------------------------------------
//  TTest_BasicIO: Public, inherited methods
// ---------------------------------------------------------------------------
tTestFWLib::ETestRes
TTest_BasicIO::eRunTest(TTextStringOutStream&   strmOut
                        , tCIDLib::TBoolean&    bWarning)
{
    tTestFWLib::ETestRes    eRes = tTestFWLib::ETestRes::Success;
    TDataSrvClient          dsclTest;
    tCIDLib::TBoolean       bRes;
    tCIDLib::TCard4         c4SerNum;
    tCIDLib::TEncodedTime   enctLastChange;
    tCIDLib::TKVPFList      colMeta;

    // Do some macro testing
    try
    {
        const TString strSampleFile(L"/User/DSTestsSample");

        // We create our own test macro, so delete it first so we start from scratch
        dsclTest.DeleteFile
        (
            strSampleFile, tCQCRemBrws::EDTypes::Macro, tfwappCQCKit.m_sectToUse
        );

        // Make sure it doesn't exist now
        if (dsclTest.bFileExists(strSampleFile, tCQCRemBrws::EDTypes::Macro))
        {
            strmOut << TFWCurLn << L"Sample macro exists after delete\n\n";

            // This is fundamentally bad so just give up
            return tTestFWLib::ETestRes::Failed;;
        }

        // Write out our new macro contents
        TString strMacro
        (
            L"Class=[NonFinal]\n"
            L"    ClassPath MEng.User.SendUserAct;\n"
            L"    ParentClass MEng.Object;\n"
            L"EndClass;\n"
            L"\n"
            L"Methods=[Public,Final]\n"
            L"\n"
            L"    Constructor()\n"
            L"    Begin\n"
            L"    EndConstructor;\n"
            L"\n"
            L"    // A comment\n"
            L"    Method Start() Returns Int4\n"
            L"    Begin\n"
            L"        Return 0;\n"
            L"    EndMethod;\n"
            L"\n"
            L"EndMethods;\n"
        );

        tCIDLib::TCard4 c4Flags = kCQCRemBrws::c4Flag_OverwriteOK;
        c4SerNum = 0;
        dsclTest.WriteMacro
        (
            strSampleFile
            , c4SerNum
            , enctLastChange
            , strMacro
            , c4Flags
            , colMeta
            , tfwappCQCKit.m_sectToUse
        );

        // And now it clearly should exist
        if (!dsclTest.bFileExists(strSampleFile, tCQCRemBrws::EDTypes::Macro))
        {
            strmOut << TFWCurLn << L"Sample macro not found exists after write\n\n";
            return tTestFWLib::ETestRes::Failed;;
        }

        // Remember the serial number post-creation
        const tCIDLib::TCard4 c4CreateSerNum = c4SerNum;

        //
        //  Now try to read the file back with that same serial number. We should not
        //  get new data.
        //
        TString strText;
        colMeta.RemoveAll();
        bRes = dsclTest.bReadMacro
        (
            strSampleFile
            , c4SerNum
            , enctLastChange
            , strText
            , kCIDLib::False
            , colMeta
            , tfwappCQCKit.m_sectToUse
        );

        if (bRes)
        {
            strmOut << TFWCurLn << L"Got new data after write\n\n";
            return tTestFWLib::ETestRes::Failed;;
        }

        // Zero the serial number and we should get data
        c4SerNum = 0;
        bRes = dsclTest.bReadMacro
        (
            strSampleFile
            , c4SerNum
            , enctLastChange
            , strText
            , kCIDLib::False
            , colMeta
            , tfwappCQCKit.m_sectToUse
        );

        if (!bRes)
        {
            strmOut << TFWCurLn << L"Got now data with zero serial number\n\n";
            return tTestFWLib::ETestRes::Failed;;
        }

        // The serial number should be the same as the post-creation one
        if (c4SerNum != c4CreateSerNum)
        {
            strmOut << TFWCurLn << L"Serial number different before any changes\n\n";
            return tTestFWLib::ETestRes::Failed;;
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        strmOut << TFWCurLn << L"Basic macro I/O tests failed\n\n";
        throw;
    }

    return eRes;
}




// ---------------------------------------------------------------------------
//  CLASS: TTest_FileSys
// PREFIX: tfwt
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TTest_FileSys: Constructor and Destructor
// ---------------------------------------------------------------------------
TTest_FileSys::TTest_FileSys() :

    TTestFWTest
    (
        L"File System", L"Test basic file system remote calls", 3
    )
{
}

TTest_FileSys::~TTest_FileSys()
{
}


// ---------------------------------------------------------------------------
//  TTest_FileSys: Public, inherited methods
// ---------------------------------------------------------------------------
tTestFWLib::ETestRes
TTest_FileSys::eRunTest(TTextStringOutStream&   strmOut
                        , tCIDLib::TBoolean&    bWarning)
{
    tTestFWLib::ETestRes eRes = tTestFWLib::ETestRes::Success;

    const tCQCRemBrws::EDTypes eImageType = tCQCRemBrws::EDTypes::Image;
    TFindBuf        fndbTest;
    TDataSrvClient  dsclTest;
    try
    {
        //
        //  First just test whether a system file we know is there exists. We test
        //  each of the file existence testing variations.
        //
        if (!dsclTest.bFileExists(L"/System/Blank", eImageType))
        {
            strmOut << TFWCurLn << L"System blank image was not found (1)\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }

        if (!dsclTest.bFindFile(L"/System/Blank", eImageType, fndbTest))
        {
            strmOut << TFWCurLn << L"System blank image was not found (2)\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }

        if (!dsclTest.bFileExists(L"/System", L"Blank", eImageType))
        {
            strmOut << TFWCurLn << L"System blank image was not found (3)\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }

        if (!dsclTest.bFindFile(L"/System", L"Blank", eImageType, fndbTest))
        {
            strmOut << TFWCurLn << L"System blank image was not found (4)\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        strmOut << TFWCurLn << L"File existence tests failed\n\n";
        throw;
    }


    // Do the same for some scopes
    try
    {
        if (!dsclTest.bFindScope(L"/System/Buttons", eImageType, fndbTest))
        {
            strmOut << TFWCurLn << L"/System/Buttons scope was not found\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }

        if (!dsclTest.bFindScope(L"/System/Buttons/", L"Alpha", eImageType, fndbTest))
        {
            strmOut << TFWCurLn << L"/System/Buttons/Alpha scope was not found\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        strmOut << TFWCurLn << L"Scope existence tests failed\n\n";
        throw;
    }



    // Some file type calls
    try
    {
        if (!dsclTest.bIsScope(L"/System/Buttons", eImageType))
        {
            strmOut << TFWCurLn << L"Said /System/Buttons is not a scope\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }

        if (dsclTest.bIsScope(L"/System/Black", eImageType))
        {
            strmOut << TFWCurLn << L"Said /System/Blackis a scope\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        strmOut << TFWCurLn << L"File/scope type tests failed\n\n";
        throw;
    }

    return eRes;
}



// ---------------------------------------------------------------------------
//  CLASS: TTest_MetaInfo
// PREFIX: tfwt
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TTest_MetaInfo: Constructor and Destructor
// ---------------------------------------------------------------------------
TTest_MetaInfo::TTest_MetaInfo() :

    TTestFWTest
    (
        L"Meta Info", L"Test item metadata queries", 3
    )
{
}

TTest_MetaInfo::~TTest_MetaInfo()
{
}


// ---------------------------------------------------------------------------
//  TTest_MetaInfo: Public, inherited methods
// ---------------------------------------------------------------------------
tTestFWLib::ETestRes
TTest_MetaInfo::eRunTest(TTextStringOutStream&  strmOut
                        , tCIDLib::TBoolean&    bWarning)
{
    tTestFWLib::ETestRes eRes = tTestFWLib::ETestRes::Success;

    const tCQCRemBrws::EDTypes eImageType = tCQCRemBrws::EDTypes::Image;
    TFindBuf        fndbTest;
    TDataSrvClient  dsclTest;
    try
    {
        tCIDLib::TKVPFList      colMeta;
        tCIDLib::TStrList       colChunkIds;
        tCIDLib::TCardList      fcolChunkSizes;
        tCIDLib::TCard4         c4SerialNum;
        tCIDLib::TEncodedTime   enctLastChange;

        const tCIDLib::TBoolean bRes = dsclTest.bQueryItemMetaData
        (
            L"/System/Blank"
            , tCQCRemBrws::EDTypes::Image
            , colMeta
            , colChunkIds
            , fcolChunkSizes
            , c4SerialNum
            , enctLastChange
            , kCIDLib::True
            , tfwappCQCKit.m_sectToUse
        );

        // We shouldn't be able to get back here with false, since we said to throw
        if (!bRes)
        {
            strmOut << TFWCurLn << L"Got false result but said to throw if not found\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }

        // We should get back three chunks
        if (colChunkIds.c4ElemCount() != fcolChunkSizes.c4ElemCount())
        {
            // This is going to cause other errors, so give up
            strmOut << TFWCurLn << L"The two chunk info lists were different sizes\n\n";
            return tTestFWLib::ETestRes::Failed;
        }

        // For an image we get four meta values
        if (colMeta.c4ElemCount() != 4)
        {
            strmOut << TFWCurLn << L"Expected 4 meta values for an image\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }

        // Make sure they are all there as expected
        if (!TChunkedFile::bFindMetaKey(colMeta, kCQCRemBrws::strMetaKey_ImgFormat)
        ||  !TChunkedFile::bFindMetaKey(colMeta, kCQCRemBrws::strMetaKey_ImgDepth)
        ||  !TChunkedFile::bFindMetaKey(colMeta, kCQCRemBrws::strMetaKey_ImgSize)
        ||  !TChunkedFile::bFindMetaKey(colMeta, kCQCRemBrws::strMetaKey_ThumbSize))
        {
            strmOut << TFWCurLn << L"Some image meta values were not found\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        strmOut << TFWCurLn << L"Metainfo query tests failed\n\n";
        throw;
    }

    return eRes;
}

