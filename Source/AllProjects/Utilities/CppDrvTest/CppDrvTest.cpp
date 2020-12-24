//
// FILE NAME: CppDrvTest.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/25/2018
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
//  This is the main module of the program. It just defines the facility object
//  and program entry point.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


// ----------------------------------------------------------------------------
//  Includes
// ----------------------------------------------------------------------------
#include    "CppDrvTest.hpp"


// ----------------------------------------------------------------------------
//  Global data declarations
// ----------------------------------------------------------------------------
TFacCppDrvTest  facCppDrvTest;


// ----------------------------------------------------------------------------
//  Magic program entry point macro
// ----------------------------------------------------------------------------
CIDLib_CtrlsModule
(
    TThread
    (
        L"MainThread"
        , TMemberFunc<TFacCppDrvTest>(&facCppDrvTest, &TFacCppDrvTest::eMainThread)
    )
)

