//
// FILE NAME: CQCAdmin.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/19/2015
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
//  This is the main header of the CQC Admin client. It does the usual things that
//  a CIDLib based facility main file does, which is to provide the magic main entry
//  pointer macro, and the facility object
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCAdmin.hpp"


// ----------------------------------------------------------------------------
//  Main module magic macro
// ----------------------------------------------------------------------------
CIDLib_CtrlsModule
(
    TThread
    (
        L"MainThread"
        , TMemberFunc<TFacCQCAdmin>(&facCQCAdmin, &TFacCQCAdmin::eMainThread)
    )
)


// ----------------------------------------------------------------------------
//  Declare our global data, which is exported via the main header
// ----------------------------------------------------------------------------
TFacCQCAdmin   facCQCAdmin;

