//
// FILE NAME: TestCQCKit_Facility.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/23/2009
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
//  This file implements the tests for the facility class.
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
#include    "TestCQCKit.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TTest_Facility1,TTestFWTest)
RTTIDecls(TTest_Facility2,TTestFWTest)




// ---------------------------------------------------------------------------
//  CLASS: TTest_Facility1
// PREFIX: tfwt
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TTest_Facility1: Constructor and Destructor
// ---------------------------------------------------------------------------
TTest_Facility1::TTest_Facility1() :

    TTestFWTest
    (
        L"Facility 1", L"Basic CQCKit facility class tests 1", 1
    )
{
}

TTest_Facility1::~TTest_Facility1()
{
}


// ---------------------------------------------------------------------------
//  TTest_Facility1: Public, inherited methods
// ---------------------------------------------------------------------------
tTestFWLib::ETestRes
TTest_Facility1::eRunTest(TTextStringOutStream&  strmOut
                        , tCIDLib::TBoolean&    bWarning)
{
    tTestFWLib::ETestRes eRes = tTestFWLib::ETestRes::Success;

    //
    //  Do some parsing of field names and make sure that it catches any
    //  bad stuff and passes good stuff.
    //
    {
        struct TTestItem
        {
            const tCIDLib::TCh*     pszValue;
            const tCIDLib::TCh*     pszMoniker;
            const tCIDLib::TCh*     pszField;
            tCIDLib::TCard1         c1ShouldFail;
            tCQCKit::EDevClasses    eDevClass;


            //
            //  Only for when using the parsing method that splits out the sub-unit
            //  and the suffix part of the field name. In this case the pszField
            //  value isn't used. If it's a V1 field, then the whole field name ends
            //  up in pszSuffix.
            //
            const tCIDLib::TCh*     pszSubUnit;
            const tCIDLib::TCh*     pszSuffix;
        };

        const TTestItem aTests[] =
        {
            // Positive tests
            { L"One.Two"        , L"One"    , L"Two"        , 0 , tCQCKit::EDevClasses::None, 0, 0 }
          , { L"One.$Two"       , L"One"    , L"$Two"       , 0 , tCQCKit::EDevClasses::None, 0, 0 }
          , { L"A.B"            , L"A"      , L"B"          , 0 , tCQCKit::EDevClasses::None, 0, 0 }
          , { L"A_B.C_D"        , L"A_B"    , L"C_D"        , 0 , tCQCKit::EDevClasses::None, 0, 0 }
          , { L"A-B.C-D"        , L"A-B"    , L"C-D"        , 0 , tCQCKit::EDevClasses::None, 0, 0 }
          , { L"AB_CD.EF_GH"    , L"AB_CD"  , L"EF_GH"      , 0 , tCQCKit::EDevClasses::None, 0, 0 }
          , { L"AB-CD.EF-GH"    , L"AB-CD"  , L"EF-GH"      , 0 , tCQCKit::EDevClasses::None, 0, 0 }
          , { L"A1.B2"          , L"A1"     , L"B2"         , 0 , tCQCKit::EDevClasses::None, 0, 0 }
          , { L"A.1A"           , L"A"      , L"1A"         , 0 , tCQCKit::EDevClasses::None, 0, 0 }

            // V2 architecture positive tests
          , { L"Pan.AUD#Test"   , L"Pan"    , L"AUD#Test"   , 0 , tCQCKit::EDevClasses::Audio, L"", L"Test" }
          , { L"Pan.AUD#123"    , L"Pan"    , L"AUD#123"    , 0 , tCQCKit::EDevClasses::Audio, L"", L"123" }
          , { L"Pan.AUD#Z1~A"   , L"Pan"    , L"AUD#Z1~A"   , 0 , tCQCKit::EDevClasses::Audio, L"Z1", L"A" }

            // Negative tests
          , { L".B"             , L""       , L""           , 1 , tCQCKit::EDevClasses::None, 0, 0 }
          , { L"One.Two$"       , L""       , L""           , 1 , tCQCKit::EDevClasses::None, 0, 0 }
          , { L"_One.Two"       , L""       , L""           , 1 , tCQCKit::EDevClasses::None, 0, 0 }
          , { L"-One.Two"       , L""       , L""           , 1 , tCQCKit::EDevClasses::None, 0, 0 }
          , { L".One.Two"       , L""       , L""           , 1 , tCQCKit::EDevClasses::None, 0, 0 }
          , { L"One..Two"       , L""       , L""           , 1 , tCQCKit::EDevClasses::None, 0, 0 }
          , { L"One.Tw.o"       , L""       , L""           , 1 , tCQCKit::EDevClasses::None, 0, 0 }
          , { L"One.Tw.o"       , L""       , L""           , 1 , tCQCKit::EDevClasses::None, 0, 0 }
          , { L"One.Two."       , L""       , L""           , 1 , tCQCKit::EDevClasses::None, 0, 0 }
          , { L"One.Two "       , L""       , L""           , 1 , tCQCKit::EDevClasses::None, 0, 0 }
          , { L" One.Two"       , L""       , L""           , 1 , tCQCKit::EDevClasses::None, 0, 0 }
          , { L"One._Two"       , L""       , L""           , 1 , tCQCKit::EDevClasses::None, 0, 0 }
          , { L"1.A"            , L""       , L""           , 1 , tCQCKit::EDevClasses::None, 0, 0 }
          , { L"1B.C"           , L""       , L""           , 1 , tCQCKit::EDevClasses::None, 0, 0 }

            // Negative V2 architecture tests
          , { L"Pan.SAB#Test"   , L""       , L""           , 1 , tCQCKit::EDevClasses::None, 0, 0 }
          , { L"Pan.SEC#Test"   , L""       , L""           , 1 , tCQCKit::EDevClasses::AIO, 0, 0 }
          , { L"1C.SEC#Test"    , L""       , L""           , 1 , tCQCKit::EDevClasses::None, 0, 0 }
          , { L"1C.SEC#"        , L""       , L""           , 1 , tCQCKit::EDevClasses::Security, 0, 0 }
          , { L"1C.#Test"       , L""       , L""           , 1 , tCQCKit::EDevClasses::None, 0, 0 }
        };
        const tCIDLib::TCard4 c4Count = tCIDLib::c4ArrayElems(aTests);

        tCQCKit::EDevClasses    eDevClass;
        TString                 strField;
        TString                 strMoniker;
        TString                 strSub;
        TString                 strSuffix;
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const TTestItem& itemCur = aTests[c4Index];

            tCIDLib::TBoolean bRes = facCQCKit().bParseFldName
            (
                itemCur.pszValue, strMoniker, eDevClass, strField
            );

            //
            //  The device class not being what we expact isn't part of the
            //  parsing method, so check that and override if needed.
            //
            if (bRes)
            {
                if ((itemCur.eDevClass != tCQCKit::EDevClasses::None)
                &&  (eDevClass != itemCur.eDevClass))
                {
                    bRes = kCIDLib::False;
                }
            }

            if ((itemCur.c1ShouldFail == 1) == bRes)
            {
                strmOut << TFWCurLn << L"Parse of '"
                        << itemCur.pszValue << L"' produced wrong result\n\n";
                eRes = tTestFWLib::ETestRes::Failed;
            }

            // If not a negative test, check the returned values
            if (itemCur.c1ShouldFail == 0)
            {
                if (strMoniker != itemCur.pszMoniker)
                {
                    strmOut << TFWCurLn << L"Field name parse of '"
                            << itemCur.pszValue << L"' got wrong moniker\n\n";
                    eRes = tTestFWLib::ETestRes::Failed;

                }
                 else if (strField != itemCur.pszField)
                {
                    strmOut << TFWCurLn << L"Field name parse of '"
                            << itemCur.pszValue << L"' got wrong field\n\n";
                    eRes = tTestFWLib::ETestRes::Failed;
                }
            }

            // Test the more advanced (mostly for V2) parsing method
            if (itemCur.c1ShouldFail == 0)
            {
                bRes = facCQCKit().bParseFldName
                (
                    itemCur.pszValue
                    , strMoniker
                    , eDevClass
                    , strSub
                    , strSuffix
                );

                if (bRes)
                {
                    if (eDevClass != itemCur.eDevClass)
                    {
                        strmOut << TFWCurLn << L"Adv field name parse of '"
                                << itemCur.pszValue << L"' got wrong device class\n\n";
                        eRes = tTestFWLib::ETestRes::Failed;
                    }

                    // If we expected a subunit, then check it
                    if (itemCur.pszSubUnit && (strSub != itemCur.pszSubUnit))
                    {
                        strmOut << TFWCurLn << L"Adv field name parse of '"
                                << itemCur.pszValue << L"' got wrong subunit\n\n";
                        eRes = tTestFWLib::ETestRes::Failed;
                    }

                    if (itemCur.pszSuffix && (strSuffix != itemCur.pszSuffix))
                    {
                        strmOut << TFWCurLn << L"Adv field name parse of '"
                                << itemCur.pszValue << L"' got wrong suffix\n\n";
                        eRes = tTestFWLib::ETestRes::Failed;
                    }
                }
                 else
                {
                    strmOut << TFWCurLn << L"Adv field name parse of '"
                            << itemCur.pszValue << L"' failed\n\n";
                    eRes = tTestFWLib::ETestRes::Failed;
                }
            }
        }
    }


    //
    //  Test the 'is valid field name' method.
    //
    {
        struct TTestItem
        {
            const tCIDLib::TCh*     pszValue;
            tCIDLib::TCard1         bShouldFail;
            tCIDLib::TCard1         bInt;
            tCQCKit::EDevClasses    eDevClass;
        };

        const TTestItem aTests[] =
        {
            // Positive tests
            { L"Two"        , 0 , 0 , tCQCKit::EDevClasses::None }
          , { L"DIO#Two"    , 0 , 0 , tCQCKit::EDevClasses::DIO  }
          , { L"AVPRC#Two"  , 0 , 0 , tCQCKit::EDevClasses::AVProcessor }
          , { L"DIO#Z1~AB"  , 0 , 0 , tCQCKit::EDevClasses::DIO }
          , { L"AUD#1~Abc"  , 0 , 0 , tCQCKit::EDevClasses::Audio }

          , { L"B"          , 0 , 0 , tCQCKit::EDevClasses::None }
          , { L"$B"         , 0 , 1 , tCQCKit::EDevClasses::None }
          , { L"B12"        , 0 , 0 , tCQCKit::EDevClasses::None }
          , { L"2wo"        , 0 , 0 , tCQCKit::EDevClasses::None }
          , { L"24"         , 0 , 0 , tCQCKit::EDevClasses::None }
          , { L"$Two"       , 0 , 0 , tCQCKit::EDevClasses::None }

          , { L".B"         , 1 , 0 , tCQCKit::EDevClasses::None }
          , { L"Two$"       , 1 , 0 , tCQCKit::EDevClasses::None }
          , { L"..Two"      , 1 , 0 , tCQCKit::EDevClasses::None }
          , { L"Tw.o"       , 1 , 0 , tCQCKit::EDevClasses::None }
          , { L"Two."       , 1 , 0 , tCQCKit::EDevClasses::None }
          , { L"Two "       , 1 , 0 , tCQCKit::EDevClasses::None }
          , { L"_Two"       , 1 , 0 , tCQCKit::EDevClasses::None }
        };
        const tCIDLib::TCard4 c4Count = tCIDLib::c4ArrayElems(aTests);

        tCQCKit::EDevClasses    eDevClass;
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const TTestItem& itemCur = aTests[c4Index];

            const tCIDLib::TBoolean bRes = facCQCKit().bIsValidFldName
            (
                itemCur.pszValue
                , itemCur.bInt == 1
                , eDevClass
            );

            if (bRes == (itemCur.bShouldFail == 1))
            {
                strmOut << TFWCurLn << L"Field name parse of '"
                        << itemCur.pszValue << L"' produced wrong result\n\n";
                eRes = tTestFWLib::ETestRes::Failed;
            }
             else
            {
                // We got the right result. Check the device class if needed
                if ((itemCur.eDevClass != tCQCKit::EDevClasses::None)
                &&  (eDevClass != itemCur.eDevClass))
                {
                    strmOut << TFWCurLn << L"Field name parse of '"
                            << itemCur.pszValue << L"' produced wrong device class\n\n";
                    eRes = tTestFWLib::ETestRes::Failed;
                }
            }
        }
    }


    // Test the general purpose bulk token replacement method.
    {
        tCIDLib::TKVHashSet colList(17, TStringKeyOps(), &TKeyValuePair::strExtractKey);

        colList.objAdd(TKeyValuePair(L"%Test1%", L"1"));
        colList.objAdd(TKeyValuePair(L"%Test2%", L"2"));
        colList.objAdd(TKeyValuePair(L"%Test3%", L"3"));

        TString strTest(L"%Test1%%Test2%%Test3%");
        if (!facCQCKit().bReplaceTokens(colList, strTest) || (strTest != L"123"))
        {
            strmOut << TFWCurLn << L"Bulk token replacement failed\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }
    }


    // Test the boolean value checker
    {
        tCIDLib::TBoolean abVals[4];
        if (!facCQCKit().bCheckBoolVal(L"0", abVals[0])
        ||  !facCQCKit().bCheckBoolVal(L"1", abVals[1])
        ||  !facCQCKit().bCheckBoolVal(L"False", abVals[2])
        ||  !facCQCKit().bCheckBoolVal(L"True", abVals[3]))
        {
            strmOut << TFWCurLn << L"Valid boolean value was rejected\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }

        if ((abVals[0] || !abVals[1] || abVals[2] || !abVals[3]))
        {
            strmOut << TFWCurLn << L"Incorrect boolean value was returned\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }

        if (facCQCKit().bCheckBoolVal(L"x", abVals[0])
        ||  facCQCKit().bCheckBoolVal(L"3", abVals[1])
        ||  facCQCKit().bCheckBoolVal(L"Flse", abVals[2])
        ||  facCQCKit().bCheckBoolVal(L"Tru", abVals[3]))
        {
            strmOut << TFWCurLn << L"Invalid boolean value were not rejected\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }
    }

    return eRes;
}



// ---------------------------------------------------------------------------
//  CLASS: TTest_Facility2
// PREFIX: tfwt
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TTest_Facility2: Constructor and Destructor
// ---------------------------------------------------------------------------
TTest_Facility2::TTest_Facility2() :

    TTestFWTest
    (
        L"Facility 2", L"Basic CQCKit facility class tests 2", 4
    )
{
}

TTest_Facility2::~TTest_Facility2()
{
}


// ---------------------------------------------------------------------------
//  TTest_Facility2: Public, inherited methods
// ---------------------------------------------------------------------------
tTestFWLib::ETestRes
TTest_Facility2::eRunTest(TTextStringOutStream&  strmOut
                        , tCIDLib::TBoolean&    bWarning)
{
    tTestFWLib::ETestRes eRet = tTestFWLib::ETestRes::Success;

    {
        struct TTestItem
        {
            const tCIDLib::TCh* pszSrc;
            const tCIDLib::TCh* pszResult;
            tCIDLib::TCard1     c1ShouldChange;
            tCIDLib::TCard1     c1ShouldFail;
        };

        const TTestItem aTests[] =
        {
            // Positive tests
            { L"Should not change"          , 0                     , 0, 0 }
          , { L"Var=%(GVar:Var1)"           , L"Var=Val1"           , 1, 0 }
          , { L"Val=%(StdRTV:Env01)"        , L"Val=Env1"           , 1, 0 }
          , { L"User=%(StdRTV:CurUserName)" , L"User=BubbaJones"    , 1, 0 }
          , { L"Val=%(StdRTV:Env07)"        , L"Val=TestVal7"       , 1, 0 }

            // Negative tests
          , { L"Untermed %(LVar:Test"       , 0                     , 0, 1 }
        };
        const tCIDLib::TCard4 c4Count = tCIDLib::c4ArrayElems(aTests);

        //
        //  Create a standard RTV object and global variables object and
        //  add a couple variables to it. Set up a bogus user context
        //  which the RTV object needs. Also tests the ability to set
        //  environment variables on RTV objects. We'll first ask it to
        //  load from the host (where 7, 8 and 9 are set), then we'll set
        //  some directly.
        //
        TCQCUserCtx     cuctxToUse
        (
            L"BubbaJones", tCQCKit::EUserRoles::NormalUser, L"\\User\\MyDefTemplate"
        );
        cuctxToUse.LoadEnvRTVsFromHost();
        cuctxToUse.SetEnvRTVAt(0, L"Env1");
        cuctxToUse.SetEnvRTVAt(5, L"Env6");

        TCQCCmdRTVSrc   crtsToUse(cuctxToUse);
        TStdVarsTar     ctarGlobals(tCIDLib::EMTStates::Safe, kCIDLib::False);
        ctarGlobals.bAddOrUpdateVar
        (
            L"GVar:Var1", L"Val1", tCQCKit::EFldTypes::String, TString::strEmpty()
        );

        // And now process all of the tests
        TString         strResult;
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const TTestItem& itemCur = aTests[c4Index];

            const tCQCKit::ECmdPrepRes eRes = facCQCKit().eStdTokenReplace
            (
                itemCur.pszSrc
                , &crtsToUse
                , &ctarGlobals
                , 0
                , strResult
                , tCQCKit::ETokRepFlags::None
            );

            if (itemCur.c1ShouldFail && (eRes != tCQCKit::ECmdPrepRes::Failed))
            {
                strmOut << TFWCurLn << L"Cmd parm parse should have failed '"
                        << itemCur.pszSrc << L"'\n\n";
                eRet = tTestFWLib::ETestRes::Failed;
            }

            // If not a negative test, check the returned values
            if (!itemCur.c1ShouldFail)
            {
                if (itemCur.c1ShouldChange)
                {
                    if (eRes != tCQCKit::ECmdPrepRes::Changed)
                    {
                        strmOut << TFWCurLn << L"Cmd parm should have changed '"
                                << itemCur.pszSrc << L"'\n\n";
                        eRet = tTestFWLib::ETestRes::Failed;
                    }
                     else
                    {
                        if (strResult != itemCur.pszResult)
                        {
                            strmOut << TFWCurLn
                                    << L"Got incorrect result on '"
                                    << itemCur.pszSrc << L"'\n\n";
                            eRet = tTestFWLib::ETestRes::Failed;
                        }
                    }
                }
            }
        }
    }
    return eRet;
}


