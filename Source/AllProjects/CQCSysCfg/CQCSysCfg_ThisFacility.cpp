//
// FILE NAME: CQCSysCfg_ThisFacility.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/06/2011
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
#include    "CQCSysCfg_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TFacCQCSysCfg,TFacility)


// ---------------------------------------------------------------------------
//   CLASS: TFacCQCSysCfg
//  PREFIX: fac
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TFacCQCSysCfg: Constructors and Destructor
// ---------------------------------------------------------------------------
TFacCQCSysCfg::TFacCQCSysCfg() :

    TFacility
    (
        L"CQCSysCfg"
        , tCIDLib::EModTypes::SharedLib
        , kCQCKit::c4MajVersion
        , kCQCKit::c4MinVersion
        , kCQCKit::c4Revision
        , tCIDLib::EModFlags::HasMsgFile
    )
{
}

TFacCQCSysCfg::~TFacCQCSysCfg()
{
}

