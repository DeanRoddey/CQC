//
// FILE NAME: CQCProtoDev.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/27/2003
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
//  Just provides the entry point macro and any global data values.
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
#include    "CQCProtoDev.hpp"


// ----------------------------------------------------------------------------
//  Main module magic macro
// ----------------------------------------------------------------------------
CIDLib_CtrlsModule
(
    TThread
    (
        L"MainThread"
        , TMemberFunc<TFacCQCProtoDev>(&facCQCProtoDev, &TFacCQCProtoDev::eMainThread)
    )
)



// ----------------------------------------------------------------------------
//  Our one global object, which is the facility object for this facility
// ----------------------------------------------------------------------------
TFacCQCProtoDev facCQCProtoDev;

