//
// FILE NAME: CQCWCHelper.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/05/2017
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
//  This is the main implementation file of the web camera helper program. It just provides
//  the entry point macro and the lazy eval method for the facility object which does all
//  of the real work.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  We need to force the use of the libvlc library.
// ---------------------------------------------------------------------------
#pragma comment(lib, "libvlc.lib")


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCWCHelper.hpp"


// ---------------------------------------------------------------------------
//  Do the magic main module code
// ---------------------------------------------------------------------------
CIDLib_CtrlsModule
(
    TThread
    (
        L"CQCWCHelperMainThread"
        , TMemberFunc<TFacCQCWCHelper>(&facCQCWCHelper(), &TFacCQCWCHelper::eMainThread)
    )
)


// ---------------------------------------------------------------------------
//  Global functions
// ---------------------------------------------------------------------------
TFacCQCWCHelper& facCQCWCHelper()
{
    static TFacCQCWCHelper* pfacThis = new TFacCQCWCHelper();
    return *pfacThis;
}
