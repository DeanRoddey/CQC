//
// FILE NAME: CQCWBHelper_WBHelperCtrlImpl.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/16/2016
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
#include    "CQCWBHelper.hpp"



// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TWBHelperCtrlImpl, TCQCWBHelperServerBase)



// ---------------------------------------------------------------------------
//  CLASS: TWBHelperCtrlImpl
// PREFIX: orbs
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TWBHelperCtrlImpl: Constructors and Destructor
// ---------------------------------------------------------------------------
TWBHelperCtrlImpl::TWBHelperCtrlImpl(const TOrbObjId& ooidToUse) :

    TCQCWBHelperServerBase(ooidToUse)
{
}

TWBHelperCtrlImpl::~TWBHelperCtrlImpl()
{
}


// ---------------------------------------------------------------------------
//  TWBHelperCtrlImpl: Public, inherited methods
// ---------------------------------------------------------------------------

// Just pass these on to the facility class, usually by queuing up a message on it
tCIDLib::TVoid TWBHelperCtrlImpl::Close()
{
    facCQCWBHelper().Close();
}


tCIDLib::TVoid TWBHelperCtrlImpl::DoOp(const tWebBrowser::ENavOps eToDo)
{
    facCQCWBHelper().DoOp(eToDo);
}


tCIDLib::TVoid
TWBHelperCtrlImpl::Initialize(  const   TArea&              areaPar
                                , const TArea&              areaWidget
                                , const tCIDLib::TBoolean   bVisibility
                                , const TString&            strInitURL)
{
    facCQCWBHelper().Initialize(areaPar, areaWidget, bVisibility, strInitURL);
}


tCIDLib::TVoid TWBHelperCtrlImpl::Ping()
{
    facCQCWBHelper().Ping();
}

tCIDLib::TVoid TWBHelperCtrlImpl::SetArea(const TArea& areaPar, const TArea& areaWidget)
{
    facCQCWBHelper().SetArea(areaPar, areaWidget);
}


tCIDLib::TVoid TWBHelperCtrlImpl::SetURL(const TString& strToSet)
{
    facCQCWBHelper().SetURL(strToSet);
}


tCIDLib::TVoid TWBHelperCtrlImpl::SetVisibility(const tCIDLib::TBoolean bNewState)
{
    facCQCWBHelper().SetVisibility(bNewState);
}


// ---------------------------------------------------------------------------
//  TWBHelperCtrlImpl: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TWBHelperCtrlImpl::Initialize()
{
}


tCIDLib::TVoid TWBHelperCtrlImpl::Terminate()
{
}

