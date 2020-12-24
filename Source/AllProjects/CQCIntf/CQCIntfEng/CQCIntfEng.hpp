//
// FILE NAME: CQCIntfEng.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/01/2015
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
//  This is the main header of the graphics layer of the CQC Interface Viewer engine
//  facility.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


// ---------------------------------------------------------------------------
//  Set up our import/export attributes
// ---------------------------------------------------------------------------
#if     defined(PROJ_CQCINTFENG)
#define CQCINTFENGEXPORT   CID_DLLEXPORT
#else
#define CQCINTFENGEXPORT   CID_DLLIMPORT
#endif


// ---------------------------------------------------------------------------
//  Include our underlying headers
// ---------------------------------------------------------------------------
#include    "CIDRegX.hpp"
#include    "CIDNet.hpp"
#include    "CIDCtrls.hpp"
#include    "CIDGraphDev.hpp"
#include    "CQCKit.hpp"
#include    "CQCPollEng.hpp"
#include    "CQCMedia.hpp"
#include    "CQCEvCl.hpp"
#include    "CQCRemBrws.hpp"
#include    "CQCAct.hpp"

#include    "CQCIntfEng_EaseScroller.hpp"



// ---------------------------------------------------------------------------
//  Facility constants
// ---------------------------------------------------------------------------
namespace kCQCIntfEng
{
    // -----------------------------------------------------------------------
    //  The border size we use for hit testing of the mouse around widgets.
    // -----------------------------------------------------------------------
    constexpr tCIDLib::TCard4       c4SizeBorderWidth = 2;


    // -----------------------------------------------------------------------
    //  Maximum number of base + popup interfaces that can be nested.
    // -----------------------------------------------------------------------
    constexpr tCIDLib::TCard4       c4MaxIntfDepth = 8;


    // -----------------------------------------------------------------------
    //  Used in template import/export
    // -----------------------------------------------------------------------
    constexpr const tCIDLib::TCh* const pszExpMonMapFld = L"Intf_MapData";


    // -----------------------------------------------------------------------
    //  The recommended timings for the three types of callbacks that the
    //  view implementation (provided by various hosting applications) must
    //  make to us.
    // -----------------------------------------------------------------------
    constexpr tCIDLib::TCard4       c4ActiveUpdatePeriod    = 25;
    constexpr tCIDLib::TCard4       c4ValueUpdatePeriod     = 150;
    constexpr tCIDLib::TCard4       c4EventUpdatePeriod     = 1000;


    // -----------------------------------------------------------------------
    //  The font point range we support.
    // -----------------------------------------------------------------------
    constexpr tCIDLib::TInt4        i4MinFontSize = 5;
    constexpr tCIDLib::TInt4        i4MaxFontSize = 96;


    // -----------------------------------------------------------------------
    //  The min and max range we allow for some controls that are driven by the range
    //  of their associated field/variable, such as the slider or volume knob.
    // -----------------------------------------------------------------------
    constexpr tCIDLib::TFloat8      f8MinRange          = 8;
    constexpr tCIDLib::TFloat8      f8MaxRange          = 512;
}



// ---------------------------------------------------------------------------
//  Forward references
// ---------------------------------------------------------------------------
class TAttrData;
class TCQCIntfWidget;
class TCQCIntfContainer;
class TCQCIntfTemplate;
class TCQCIntfStateList;
class TCQCIntfOverlay;
class MCQCIntfImgListIntf;
class TCQCIntfView;
class TAttrEditWnd;


// Some types that we need to be declared before the headers
namespace tCQCIntfEng
{
    // -----------------------------------------------------------------------
    //  A list of command sources, which are used to store lists of actions
    //  that are not part of command source widgets, such as template level
    //  actions.
    // -----------------------------------------------------------------------
    using TActList = TVector<TCQCStdCmdSrc>;


    // -----------------------------------------------------------------------
    //  A counted pointer to a graphics drawing device. This is now the views
    //  return a graphics device when the engine asks for one, since graphics
    //  devices don't support copy semantics.
    // -----------------------------------------------------------------------
    using TGraphIntfDev = TCntPtr<TGraphDrawDev>;


    // -----------------------------------------------------------------------
    //  A vector of TCard8 values to hold lists of design time widget ids. It
    //  is mostly used by the designer but we have some assistance methods that
    //  have to fill them in.
    // -----------------------------------------------------------------------
    using TDesIdList = TFundVector<tCIDLib::TCard8>;


    // -----------------------------------------------------------------------
    //  A vector of user role based value range limits, used in things like
    //  sliders and volume knobs.
    // -----------------------------------------------------------------------
    using TUserLims = TFundVector<tCIDLib::TCard4, tCQCKit::EUserRoles>;
}


// ---------------------------------------------------------------------------
//  Include our headers, in the needed order.
// ---------------------------------------------------------------------------
#include    "CQCIntfEng_ErrorIds.hpp"
#include    "CQCIntfEng_MessageIds.hpp"
#include    "CQCIntfEng_Shared.hpp"
#include    "CQCIntfEng_ValErrInfo.hpp"
#include    "CQCIntfEng_Type.hpp"
#include    "CQCIntfEng_ImgCacheItem.hpp"

// We need to slip in a variation of the action list indexed by hot key enum
namespace tCQCIntfEng
{
    using THKActList    = TVector<TCQCStdCmdSrc, tCQCIntfEng::EHotKeys>;
    using TWidgetList   = TRefBag<TCQCIntfWidget>;
    using TChildList    = TRefVector<TCQCIntfWidget>;
    using TErrList      = TVector<TCQCIntfValErrInfo>;
}

#include    "CQCIntfEng_Widget.hpp"
#include    "CQCIntfEng_States.hpp"

#include    "CQCIntfEng_BooleanIntf.hpp"
#include    "CQCIntfEng_FontIntf.hpp"
#include    "CQCIntfEng_ImgIntf.hpp"
#include    "CQCIntfEng_ImgListIntf.hpp"
#include    "CQCIntfEng_MultiFldIntf.hpp"
#include    "CQCIntfEng_SingleFldIntf.hpp"
#include    "CQCIntfEng_StaticIntf.hpp"
#include    "CQCIntfEng_VarIntf.hpp"
#include    "CQCIntfEng_XlatIntf.hpp"

#include    "CQCIntfEng_ButtonBase.hpp"

#include    "CQCIntfEng_HVBrowserBase.hpp"
#include    "CQCIntfEng_ListBrowsers.hpp"
#include    "CQCIntfEng_MappedImg.hpp"

#include    "CQCIntfEng_Container.hpp"
#include    "CQCIntfEng_Overlay.hpp"
#include    "CQCIntfEng_Template.hpp"
#include    "CQCIntfEng_WebCamera.hpp"

#include    "CQCIntfEng_ActEngine.hpp"
#include    "CQCIntfEng_View.hpp"

#include    "CQCIntfEng_ThisFacility.hpp"


// ---------------------------------------------------------------------------
//  Export the facility object lazy eval method
// ---------------------------------------------------------------------------
extern CQCINTFENGEXPORT TFacCQCIntfEng& facCQCIntfEng();



