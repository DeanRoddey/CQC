//
// FILE NAME: CQCIntfEd_.hpp
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
//  This is our internal header, where we bring in stuff that we only need
//  internally, underlying and our own. We also bring in our public header so
//  that all of our stuff sees that.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


// ---------------------------------------------------------------------------
//  Include our own public header for internal use
// ---------------------------------------------------------------------------
#include    "CQCIntfEd.hpp"



// ---------------------------------------------------------------------------
//  Any underlying stuff we only need internallu
// ---------------------------------------------------------------------------
#include    "CIDPNG.hpp"
#include    "CQCKit.hpp"
#include    "CQCIGKit.hpp"
#include    "CQCIntfWEng.hpp"
#include    "CQLogicSh.hpp"

//
//  Some that aren't normally accessed outside of the interface engine but we need
//  to directly access them.
//
#include    "CQCIntfEng_LiveTile.hpp"
#include    "CQCIntfEng_Toolbar.hpp"



// ----------------------------------------------------------------------------
//  Internal facility constants
// ----------------------------------------------------------------------------
namespace kCQCIntfEd_
{
    // ------------------------------------------------------------------------
    //  Our local config store keys.  The last path ones are not full paths,
    //  just ids that are put into a standard last path scope by a helper
    //  method.
    // ------------------------------------------------------------------------
    const tCIDLib::TCh* const pszStoreKey           = L"IntfDes";
    const tCIDLib::TCh* const pszCfgKey_MainFrame   = L"/Wnd/MainFrameState";
    const tCIDLib::TCh* const pszCfgKey_CfgData     = L"/Wnd/CfgData";
    const tCIDLib::TCh* const pszCfgKey_Theme       = L"/Main/CfgData/Theme";
    const tCIDLib::TCh* const pszCfgKey_WndPal      = L"/Wnd/WndPalState";

    const tCIDLib::TCh* const pszLastPackage        = L"LastPackPath";


    // ------------------------------------------------------------------------
    //  Window related constants
    // ------------------------------------------------------------------------
    const tCIDLib::TCard4   c4ScrollUnitSz = 32;


    // ------------------------------------------------------------------------
    //  We wire up all the buttons in the tools tab to the editor window. Since it may
    //  get notifications from other windows as well, it doesn't use window ids as the
    //  identifier of the incoming notification. Instead we set a window name on all of
    //  the buttons and it uses that.
    // ------------------------------------------------------------------------
    const TString   strTool_BottomAlign(L"Tool_BottomAlign");
    const TString   strTool_FontBold(L"Tool_FontBold");
    const TString   strTool_FontDecrease(L"Tool_FontDecrease");
    const TString   strTool_FontFace(L"Tool_FontFace");
    const TString   strTool_FontIncrease(L"Tool_FontIncrase");
    const TString   strTool_FontItalic(L"Tool_FontItalic");
    const TString   strTool_HorzCenter(L"Tool_HorzCenter");
    const TString   strTool_HorzSpace(L"Tool_HorzSpace");
    const TString   strTool_LeftAlign(L"Tool_LeftAlign");
    const TString   strTool_RightAlign(L"Tool_RightAlign");
    const TString   strTool_TopAlign(L"Tool_TopAlign");
    const TString   strTool_VertCenter(L"Tool_VertCenter");
    const TString   strTool_VertSpace(L"Tool_VertSpace");


    // -----------------------------------------------------------------------
    //  We dynamically fill in the items under New submenu of the edit popup
    //  menu. We start them at this id. All the fixed menu items must be
    //  below the first id here.
    // -----------------------------------------------------------------------
    const tCIDLib::TCard4   c4FirstNewId    = 5000;


    // -----------------------------------------------------------------------
    //  The minimim size we'll allow a widget to get
    // -----------------------------------------------------------------------
    const tCIDLib::TCard4   c4MinSize       = 2;
}


// ----------------------------------------------------------------------------
//  Internal facility Types
// ----------------------------------------------------------------------------
namespace tCQCIntfEd_
{
}



// ---------------------------------------------------------------------------
//  Any of our own headers we only need internally
// ---------------------------------------------------------------------------
#include    "CQCIntfEd_ResourceIds.hpp"
#include    "CQCIntfEd_View_.hpp"
#include    "CQCIntfEd_WidgetCB_.hpp"
#include    "CQCIntfEd_ErrList_.hpp"
#include    "CQCIntfEd_Undo.hpp"
#include    "CQCIntfEd_AttrEditWnd_.hpp"
#include    "CQCIntfEd_EditWnd_.hpp"
#include    "CQCIntfEd_EdWrappers_.hpp"
