//
// FILE NAME: CQCWCHelper_CtrlImpl.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/05/2017
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
//  This file implements the server side of the control interface.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCWCHelper.hpp"



// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TWCHelperCtrlImpl, TCQCWCHelperServerBase)



// ---------------------------------------------------------------------------
//  CLASS: TWCHelperCtrlImpl
// PREFIX: orbs
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TWCHelperCtrlImpl: Constructors and Destructor
// ---------------------------------------------------------------------------
TWCHelperCtrlImpl::TWCHelperCtrlImpl(const TOrbObjId& ooidToUse) :

    TCQCWCHelperServerBase(ooidToUse)
{
}

TWCHelperCtrlImpl::~TWCHelperCtrlImpl()
{
}


// ---------------------------------------------------------------------------
//  TWCHelperCtrlImpl: Public, inherited methods
// ---------------------------------------------------------------------------

// Just pass these on to the facility class, usually by queuing up a message on it
tCIDLib::TVoid TWCHelperCtrlImpl::Close()
{
    facCQCWCHelper().Close();
}


tCIDLib::TVoid
TWCHelperCtrlImpl::Initialize(  const   TArea&              areaPar
                                , const TArea&              areaWidget
                                , const tCIDLib::TBoolean   bVisibility)
{
    facCQCWCHelper().Initialize(areaPar, areaWidget, bVisibility);
}


tCIDLib::TVoid TWCHelperCtrlImpl::Ping()
{
    facCQCWCHelper().Ping();
}

tCIDLib::TVoid TWCHelperCtrlImpl::SetArea(const TArea& areaPar, const TArea& areaWidget)
{
    facCQCWCHelper().SetArea(areaPar, areaWidget);
}


tCIDLib::TVoid TWCHelperCtrlImpl::SetMute(const tCIDLib::TBoolean bNewState)
{
    facCQCWCHelper().SetMute(bNewState);
}


tCIDLib::TVoid TWCHelperCtrlImpl::SetVisibility(const tCIDLib::TBoolean bNewState)
{
    facCQCWCHelper().SetVisibility(bNewState);
}


tCIDLib::TVoid TWCHelperCtrlImpl::SetVolume(const tCIDLib::TCard4 c4ToSet)
{
    facCQCWCHelper().SetVolume(c4ToSet);
}

// ---------------------------------------------------------------------------
//  TWCHelperCtrlImpl: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TWCHelperCtrlImpl::Initialize()
{
}


tCIDLib::TVoid TWCHelperCtrlImpl::Terminate()
{
}

