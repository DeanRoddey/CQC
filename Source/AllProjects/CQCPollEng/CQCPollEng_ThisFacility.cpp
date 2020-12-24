//
// FILE NAME: CQCPollEng_ThisFacility.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/25/2008
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
#include    "CQCPollEng_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TFacCQCPollEng,TFacility)


// ---------------------------------------------------------------------------
//   CLASS: TFacCQCPollEng
//  PREFIX: fac
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TFacCQCPollEng: Constructors and Destructor
// ---------------------------------------------------------------------------
TFacCQCPollEng::TFacCQCPollEng() :

    TFacility
    (
        L"CQCPollEng"
        , tCIDLib::EModTypes::SharedLib
        , kCQCKit::c4MajVersion
        , kCQCKit::c4MinVersion
        , kCQCKit::c4Revision
        , tCIDLib::EModFlags::HasMsgFile
    )
{
}

TFacCQCPollEng::~TFacCQCPollEng()
{
}


