//
// FILE NAME: TestIntfEng_Template.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/26/2020
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
//  This file implements tests for the template class as a whole. Lower level
//  tests of the invidiual bits should be done before these, so they are set
//  to a failrly high level.
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
#include    "TestIntfEng.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TTest_Template1,TTestFWTest)
RTTIDecls(TTest_TemplateMove,TTestFWTest)




// ---------------------------------------------------------------------------
//  CLASS: TTest_Template1
// PREFIX: tfwt
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TTest_Template1: Constructor and Destructor
// ---------------------------------------------------------------------------
TTest_Template1::TTest_Template1() :

    TTestFWTest
    (
        L"Template 1", L"Basic tests of the template class", 4
    )
{
}

TTest_Template1::~TTest_Template1()
{
}


// ---------------------------------------------------------------------------
//  TTest_Template1: Public, inherited methods
// ---------------------------------------------------------------------------
tTestFWLib::ETestRes
TTest_Template1::eRunTest(    TTextStringOutStream&   strmOut
                            , tCIDLib::TBoolean&    bWarning)
{
    tTestFWLib::ETestRes eRes = tTestFWLib::ETestRes::Success;


    return eRes;
}



// ---------------------------------------------------------------------------
//  CLASS: TTest_TemplateMove
// PREFIX: tfwt
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TTest_TemplateMove: Constructor and Destructor
// ---------------------------------------------------------------------------
TTest_TemplateMove::TTest_TemplateMove() :

    TTestFWTest
    (
        L"Template Move", L"Tests template level move semantics", 5
    )
{
}

TTest_TemplateMove::~TTest_TemplateMove()
{
}


// ---------------------------------------------------------------------------
//  TTest_TemplateMove: Public, inherited methods
// ---------------------------------------------------------------------------
tTestFWLib::ETestRes
TTest_TemplateMove::eRunTest(TTextStringOutStream&  strmOut
                            , tCIDLib::TBoolean&    bWarning)
{
    tTestFWLib::ETestRes    eRes = tTestFWLib::ETestRes::Success;


    try
    {
        // Read one of the system templates that we can depend on being there
        TDataSrvClient          dsclTest;
        tCIDLib::TCard4         c4SerialNum;
        tCIDLib::TEncodedTime   enctLastChange;
        tCIDLib::TKVPFList      colMeta;
        tCIDLib::TCard4         c4Bytes;
        THeapBuf                mbufData;
        tCIDLib::TBoolean bRes = dsclTest.bReadTemplate
        (
            L"/System/Themes/BlueGlass/Main"
            , c4SerialNum
            , enctLastChange
            , mbufData
            , c4Bytes
            , colMeta
            , tfwappIntfEng.m_sectToUse
            , kCIDLib::True
        );

        if (!bRes)
        {
            strmOut << TFWCurLn << L"The test template was not found\n\n";
            return tTestFWLib::ETestRes::Failed;;
        }

        // Stream it in
        TCQCIntfTemplate iwdgOrg;
        {
            TBinMBufInStream strmSrc(&mbufData, c4Bytes);
            strmSrc >> iwdgOrg;
        }

        // Make a copy we can compare against after the move
        const TCQCIntfTemplate iwdgCopy(iwdgOrg);

        // Make sure it's equal, which is tested elsewhere, but just to be safe
        if (!iwdgCopy.bIsSame(iwdgOrg))
        {
            strmOut << TFWCurLn << L"Copy ctor didn't create equal copy\n\n";
            return tTestFWLib::ETestRes::Failed;;
        }
    }

    catch(TError& errToCatch)
    {
        strmOut << TFWCurLn << L"Error reading test template\n"
                << errToCatch << L"\n\n";
        return tTestFWLib::ETestRes::Failed;;
    }

    return eRes;
}

