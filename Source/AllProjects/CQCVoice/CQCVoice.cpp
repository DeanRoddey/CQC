//
// FILE NAME: CQCVoice.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/02/2017
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
//  This is the main implementation file of the CQCVoice facility. It does
//  the usual things of declaring the program entry point and the facility
//  object.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCVoice.hpp"


// ----------------------------------------------------------------------------
//  Main module magic macro, the windowed version of it
// ----------------------------------------------------------------------------
CIDLib_CtrlsModule
(
    TThread
    (
        L"MainThread"
        , TMemberFunc<TFacCQCVoice>(&facCQCVoice, &TFacCQCVoice::eMainThread)
    )
)


// ----------------------------------------------------------------------------
//  Our one global object, the facility object
// ----------------------------------------------------------------------------
TFacCQCVoice facCQCVoice;
