//
// FILE NAME: CQCIntfView.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/15/2002
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
//  This is the main header of the CQC standalone interface viewer. This is
//  a simple program that just allows users to view user drawn interfaces
//  and nothing else.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCIntfView.hpp"


// ----------------------------------------------------------------------------
//  Main module magic macro
// ----------------------------------------------------------------------------
CIDLib_CtrlsModule
(
    TThread
    (
        L"MainThread"
        , TMemberFunc<TFacCQCIntfView>(&facCQCIntfView, &TFacCQCIntfView::eMainThread)
    )
)


// ----------------------------------------------------------------------------
//  Declare our global data, which is exported via the main header
// ----------------------------------------------------------------------------
TFacCQCIntfView   facCQCIntfView;


