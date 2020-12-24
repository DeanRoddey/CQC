//
// FILE NAME: CQCTrayMon.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/14/2004
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
//  This is the main header of the CQC tray monitor.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCTrayMon.hpp"


// ----------------------------------------------------------------------------
//  Main module magic macro
// ----------------------------------------------------------------------------
CIDLib_CtrlsModule
(
    TThread
    (
        L"MainThread"
        , TMemberFunc<TFacCQCTrayMon>(&facCQCTrayMon, &TFacCQCTrayMon::eMainThread)
    )
)


// ----------------------------------------------------------------------------
//  Declare our global data, which is exported via the main header
// ----------------------------------------------------------------------------
TFacCQCTrayMon   facCQCTrayMon;


