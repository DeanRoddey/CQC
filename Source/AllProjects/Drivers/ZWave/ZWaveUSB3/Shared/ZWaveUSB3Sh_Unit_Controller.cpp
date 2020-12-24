//
// FILE NAME: ZWaveUSB3Sh_Controller.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/02/2017
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
//  This is the basically do-nothing implementation for controllers.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "ZWaveUSB3Sh_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros.
// ---------------------------------------------------------------------------
RTTIDecls(TZWController, TZWaveUnit)



// ---------------------------------------------------------------------------
//   CLASS: TZWController
//  PREFIX: unit
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWController: Constructors and Destructor
// ---------------------------------------------------------------------------
TZWController::TZWController(TZWUnitInfo* const punitiOwner) :

    TZWaveUnit(punitiOwner)
{
}

TZWController::~TZWController()
{
}


// ---------------------------------------------------------------------------
//  TZWController: Public, inherited methods
// ---------------------------------------------------------------------------

// Nothing for us to do, just pass to parent for now
tCIDLib::TBoolean TZWController::bPrepare()
{
    return TParent::bPrepare();
}
