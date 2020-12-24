//
// FILE NAME: CQCDriver_ThisFacility.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/06/2000
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
#include    "CQCDriver_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TFacCQCDriver,TFacility)




// ---------------------------------------------------------------------------
//   CLASS: TFacCQCDriver
//  PREFIX: fac
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TFacCQCDriver: Constructors and Destructor
// ---------------------------------------------------------------------------
TFacCQCDriver::TFacCQCDriver() :

    TFacility
    (
        L"CQCDriver"
        , tCIDLib::EModTypes::SharedLib
        , kCQCKit::c4MajVersion
        , kCQCKit::c4MinVersion
        , kCQCKit::c4Revision
        , tCIDLib::EModFlags::HasMsgFile
    )
{
    // Register some stats cache items
    TStatsCache::RegisterItem
    (
        L"/Stats/SrvDrv/DrvCmdItemsFree"
        , tCIDLib::EStatItemTypes::Value
        , m_sciCmdItemsFree
    );

    TStatsCache::RegisterItem
    (
        L"/Stats/SrvDrv/DrvCmdItemsUsed"
        , tCIDLib::EStatItemTypes::Value
        , m_sciCmdItemsUsed
    );
}

TFacCQCDriver::~TFacCQCDriver()
{
}


// ---------------------------------------------------------------------------
//  TFacCQCDriver: Public, non-virtual methods
// ---------------------------------------------------------------------------


//
//  WE maintain a stats cache item for the number of driver command pool
//  items used. The driver calls in here periodically to udpate this. He
//  does call every time one is allocated or deleted, since that happens
//  a lot. We'll call every so many times through. That's good enough for
//  our purposes, since there are always happening.
//
tCIDLib::TVoid
TFacCQCDriver::UpdatePoolItemsStat( const   tCIDLib::TCard4 c4Used
                                    , const tCIDLib::TCard4 c4Free)
{
    TStatsCache::SetValue(m_sciCmdItemsFree, c4Free);
    TStatsCache::SetValue(m_sciCmdItemsUsed, c4Used);
}

