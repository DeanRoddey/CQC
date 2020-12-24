//
// FILE NAME: CQCIntfEng_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/16/2002
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
//  This is the internal facility header
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


// ---------------------------------------------------------------------------
//  Include our own header for internal use
// ---------------------------------------------------------------------------
#include "CQCIntfEng.hpp"


// ---------------------------------------------------------------------------
//  Bring any underlying stuff we only need internally
// ---------------------------------------------------------------------------
#include    "CIDWebBrowser.hpp"
#include    "CIDPNG.hpp"
#include    "CIDJPEG.hpp"
#include    "CIDImgFact.hpp"
#include    "CIDCtrls.hpp"
#include    "CIDWUtils.hpp"



// ---------------------------------------------------------------------------
//  Internal types
// ---------------------------------------------------------------------------
namespace tCQCIntfEng_
{
}


// ---------------------------------------------------------------------------
//  And any internal headers
// ---------------------------------------------------------------------------
#include    "CQCIntfEng_ResourceIds.hpp"
#include    "CQCIntfEng_WBHCtrlClientProxy.hpp"
#include    "CQCIntfEng_WCHCtrlClientProxy.hpp"

// ---------------------------------------------------------------------------
//  Internal constants
// ---------------------------------------------------------------------------
namespace kCQCIntfEng_
{
    // -----------------------------------------------------------------------
    //  Some commonly used image keys used in the image list interface
    // -----------------------------------------------------------------------
    const tCIDLib::TCh* const pszImgKey_Error = L"Error";


    // -----------------------------------------------------------------------
    //  The prefixes we add to image names to distinguish between the image
    //  and the thumb, which both have the same name. This is what is used as
    //  the key for the hash set.
    // -----------------------------------------------------------------------
    const TString   strImagePref(L"Image::");
    const TString   strThumbPref(L"Thumb::");


    // -----------------------------------------------------------------------
    //  The number of milliseconds we use for our non-drag, intertial scrolling,
    //  and the easing algorithm order we use generally, though some widgets may
    //  choose to use something else if necessary.
    // -----------------------------------------------------------------------
    const tCIDLib::TCard4   c4EaseScrollMSs = 1000;
    const tCIDLib::TCard4   c4EaseOrder     = 4;


    // -----------------------------------------------------------------------
    //  The number of milliseconds that we use when doing a press emphasis effect
    //  in those widgets that support such a thing. Some, like buttons, may actually
    //  divide this in half.
    // -----------------------------------------------------------------------
    const tCIDLib::TCard4   c4PressEmphMSs  = 200;


    // -----------------------------------------------------------------------
    //  Some values we use as defaults when creating new widgets, in order to give
    //  them a basic appearance as a starting point.
    //
    //  rgbDef_Bgn        - The background of a template, and of any that need to
    //                      be scrollable.
    //  rgbDef_EmphXXX    - Emphasis versions of some of the other colors, used for
    //                      pressed emphasis, current selection, etc...
    //  rgbDef_Text
    //  rgbDef_TextShadow - Default text and text shadow colors.
    // -----------------------------------------------------------------------
    const TRGBClr   rgbDef_Bgn(0x50, 0x50, 0x50);
    const TRGBClr   rgbDef_Bgn2(0, 0, 0);
    const TRGBClr   rgbDef_EmphText(0xF0, 0xF0, 0xF0);
    const TRGBClr   rgbDef_EmphTextShadow(0x20, 0x20, 0x20);
    const TRGBClr   rgbDef_Text(0xC6, 0xC6, 0xC6);
    const TRGBClr   rgbDef_TextShadow(0x38, 0x38, 0x38);
    const TRGBClr   rgbDef_Border(0, 0, 0);
}

