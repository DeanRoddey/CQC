//
// FILE NAME: CQCAct_ThisFacility.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/27/2015
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
//  This is the implementation file for the facility class.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCAct_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TFacCQCAct,TFacility)



// ---------------------------------------------------------------------------
//   CLASS: TFacCQCAct
//  PREFIX: fac
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TFacCQCAct: Constructors and Destructor
// ---------------------------------------------------------------------------
TFacCQCAct::TFacCQCAct() :

    TFacility
    (
        L"CQCAct"
        , tCIDLib::EModTypes::SharedLib
        , kCQCKit::c4MajVersion
        , kCQCKit::c4MinVersion
        , kCQCKit::c4Revision
        , tCIDLib::EModFlags::HasMsgFile
    )
{
}

TFacCQCAct::~TFacCQCAct()
{
}


// ---------------------------------------------------------------------------
//  TFacCQCAct: Public, non-virtual methods
// ---------------------------------------------------------------------------

