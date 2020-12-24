//
// FILE NAME: CQCWBHelper.hpp
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
//  This is the main header of the web browser helper facility.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Include our underlying headers
// ---------------------------------------------------------------------------
#include    "CIDWebBrowser.hpp"
#include    "CIDOrbUC.hpp"
#include    "CIDWUtils.hpp"
#include    "CQCKit.hpp"

#include    "CQCGKit.hpp"


// ---------------------------------------------------------------------------
//  Facility types
// ---------------------------------------------------------------------------
namespace tCQCWBHelper
{
    // The types of incoming messages we can queue up
    enum EQMsgTypes
    {
        None

        , Close
        , DoOp
        , Init
        , SetArea
        , SetURL
        , SetVisibility

        , Count
    };
}


// ---------------------------------------------------------------------------
//  Facility constants
// ---------------------------------------------------------------------------
namespace kCQCWBHelper
{

    // -----------------------------------------------------------------------
    //  How long we will wait for activity from the invoking client before we give
    //  up and shut down.
    // -----------------------------------------------------------------------
    const tCIDLib::TEncodedTime enctTimeout = kCIDLib::enctOneSecond * 10;


    // -----------------------------------------------------------------------
    //  An id we post the main frame when new messages arrive and are queued up. He
    //  will grab them and process them.
    // -----------------------------------------------------------------------
    const tCIDLib::TInt4        i4NewMsg = 1000;
}


// ---------------------------------------------------------------------------
//  And sub-include our own headers in the correct order
// ---------------------------------------------------------------------------
#include    "CQCWBHelper_ErrorIds.hpp"
#include    "CQCWBHelper_MessageIds.hpp"
#include    "CQCWBHelper_CtrlServerBase.hpp"
#include    "CQCWBHelper_CtrlImpl.hpp"
#include    "CQCWBHelper_MainFrame.hpp"
#include    "CQCWBHelper_ThisFacility.hpp"



// ---------------------------------------------------------------------------
//  Some stuff that needs to be seen throughout this facility
// ---------------------------------------------------------------------------
extern TFacCQCWBHelper& facCQCWBHelper();
