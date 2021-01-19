// ----------------------------------------------------------------------------
//  FILE: CQCWCHelper_CtrlServerBase.cpp
//  DATE: Tue, Jan 19 17:39:55 2021 -0500
//    ID: D2CE20750A902C56-3E26CBF9499D7A90
//
//  This file was generated by the Charmed Quark CIDIDL compiler. Do not make
//  changes by hand, because they will be lost if the file is regenerated.
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Includes
// ----------------------------------------------------------------------------
#include "CQCWCHelper.hpp"

// ----------------------------------------------------------------------------
// Magic macros
// ----------------------------------------------------------------------------
RTTIDecls(TCQCWCHelperServerBase,TOrbServerBase)


// ----------------------------------------------------------------------------
// Public, static data
// ----------------------------------------------------------------------------
const TString TCQCWCHelperServerBase::strInterfaceId(L"D2CE20750A902C56-3E26CBF9499D7A90");
const TString TCQCWCHelperServerBase::strParam_CustAR(L"CustAR");


// ----------------------------------------------------------------------------
// TCQCWCHelperServerBase: Destructor
// ----------------------------------------------------------------------------
TCQCWCHelperServerBase::~TCQCWCHelperServerBase()
{
}

// ----------------------------------------------------------------------------
// TCQCWCHelperServerBase: Protected Constructors
// ----------------------------------------------------------------------------
TCQCWCHelperServerBase::TCQCWCHelperServerBase() :
    TOrbServerBase(strInterfaceId, L"TCQCWCHelperClientProxy" )
{
}

TCQCWCHelperServerBase::TCQCWCHelperServerBase(const TOrbObjId& ooidThis) :
    TOrbServerBase(ooidThis)
{
}

// ----------------------------------------------------------------------------
// TCQCWCHelperServerBase: Public, non-virtual methods
// ----------------------------------------------------------------------------
tCIDLib::TVoid TCQCWCHelperServerBase::Dispatch(const TString& strMethodName, TOrbCmd& orbcToDispatch)
{
    if (strMethodName == L"Close")
    {
        Close
        (
        );
        orbcToDispatch.strmOut().Reset();
    }
     else if (strMethodName == L"Initialize")
    {
        TArea areaPar;
        orbcToDispatch.strmIn() >> areaPar;
        TArea areaWidget;
        orbcToDispatch.strmIn() >> areaWidget;
        tCIDLib::TBoolean bVisibility;
        orbcToDispatch.strmIn() >> bVisibility;
        Initialize
        (
            areaPar
          , areaWidget
          , bVisibility
        );
        orbcToDispatch.strmOut().Reset();
    }
     else if (strMethodName == L"Ping")
    {
        Ping
        (
        );
        orbcToDispatch.strmOut().Reset();
    }
     else if (strMethodName == L"SetArea")
    {
        TArea areaPar;
        orbcToDispatch.strmIn() >> areaPar;
        TArea areaWidget;
        orbcToDispatch.strmIn() >> areaWidget;
        SetArea
        (
            areaPar
          , areaWidget
        );
        orbcToDispatch.strmOut().Reset();
    }
     else if (strMethodName == L"SetMute")
    {
        tCIDLib::TBoolean bNewState;
        orbcToDispatch.strmIn() >> bNewState;
        SetMute
        (
            bNewState
        );
        orbcToDispatch.strmOut().Reset();
    }
     else if (strMethodName == L"SetVisibility")
    {
        tCIDLib::TBoolean bNewState;
        orbcToDispatch.strmIn() >> bNewState;
        SetVisibility
        (
            bNewState
        );
        orbcToDispatch.strmOut().Reset();
    }
     else if (strMethodName == L"SetVolume")
    {
        tCIDLib::TCard4 c4ToSet;
        orbcToDispatch.strmIn() >> c4ToSet;
        SetVolume
        (
            c4ToSet
        );
        orbcToDispatch.strmOut().Reset();
    }
     else
    {
         TParent::Dispatch(strMethodName, orbcToDispatch);
    }
}

