//
// FILE NAME: CQCSrvDrvTI_ThisFacility.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/02/2018
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
#include    "CQCSrvDrvTI_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TFacCQCSrvDrvTI,TFacility)




// ---------------------------------------------------------------------------
//   CLASS: TFacCQCSrvDrvTI
//  PREFIX: fac
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TFacCQCSrvDrvTI: Constructors and Destructor
// ---------------------------------------------------------------------------
TFacCQCSrvDrvTI::TFacCQCSrvDrvTI() :

    TFacility
    (
        L"CQCSrvDrvTI"
        , tCIDLib::EModTypes::SharedLib
        , kCQCKit::c4MajVersion
        , kCQCKit::c4MinVersion
        , kCQCKit::c4Revision
        , tCIDLib::EModFlags::HasMsgFile
    )
{
}

TFacCQCSrvDrvTI::~TFacCQCSrvDrvTI()
{
}

