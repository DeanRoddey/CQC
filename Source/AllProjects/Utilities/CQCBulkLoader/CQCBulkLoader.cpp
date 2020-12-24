//
// FILE NAME: CQCBulkLoader.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/15/2010
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
//  This is the main module of the program. It just defines the facility
//  object and provides a little other stuff that has no other specific
//  home.
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
#include    "CQCBulkLoader.hpp"


// ----------------------------------------------------------------------------
//  Global data declarations
// ----------------------------------------------------------------------------
TFacCQCBulkLoader  facCQCBulkLoader;


// ----------------------------------------------------------------------------
//  Magic program entry point macro
// ----------------------------------------------------------------------------
CIDLib_MainModule
(
    TThread
    (
        L"MainThread"
        , TMemberFunc<TFacCQCBulkLoader>(&facCQCBulkLoader, &TFacCQCBulkLoader::eMainThread)
    )
)


