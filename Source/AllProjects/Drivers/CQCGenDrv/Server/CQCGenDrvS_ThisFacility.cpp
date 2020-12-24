//
// FILE NAME: CQCGenDrvS_ThisFacility.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/09/2003
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
#include    "CQCGenDrvS_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TFacCQCGenDrvS,TFacility)



// ---------------------------------------------------------------------------
//   CLASS: TFacCQCGenDrvS
//  PREFIX: fac
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TFacCQCGenDrv: Constructors and Destructor
// ---------------------------------------------------------------------------
TFacCQCGenDrvS::TFacCQCGenDrvS() :

    TFacility
    (
        L"CQCGenDrvS"
        , tCIDLib::EModTypes::SharedLib
        , kCQCKit::c4MajVersion
        , kCQCKit::c4MinVersion
        , kCQCKit::c4Revision
        , tCIDLib::EModFlags::HasMsgFile
    )
{
}


TFacCQCGenDrvS::~TFacCQCGenDrvS()
{
}


// ---------------------------------------------------------------------------
//  TFacCQCGenDrv: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TFacCQCGenDrvS::InitCMLRuntime()
{
    // Register one of our own CML class loaders with the macro engine
    facCIDMacroEng().AddClassLoader(new TCQCGenDrvClassLoader);
}

