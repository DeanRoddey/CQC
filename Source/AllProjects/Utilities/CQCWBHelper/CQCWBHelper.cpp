//
// FILE NAME: CQCWBHelper.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/16/2016
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
//  This is the main implementation file of the web browser helper program. Just does
//  the basic setup and kicks off the main window, then waits for a shutdown request.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCWBHelper.hpp"


// ---------------------------------------------------------------------------
//  Do the magic main module code
// ---------------------------------------------------------------------------
CIDLib_CtrlsModule
(
    TThread
    (
        L"CQCWBHelperMainThread"
        , TMemberFunc<TFacCQCWBHelper>(&facCQCWBHelper(), &TFacCQCWBHelper::eMainThread)
    )
)


// ---------------------------------------------------------------------------
//  Global functions
// ---------------------------------------------------------------------------
TFacCQCWBHelper& facCQCWBHelper()
{
    static TFacCQCWBHelper* pfacThis = new TFacCQCWBHelper();
    return *pfacThis;
}
