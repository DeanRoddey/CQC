//
// FILE NAME: CQCTreeBrws.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/24/2015
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
//  This facility sits between CQCGKit and CQCIGKit basically, and provides lots
//  of stuff for browsing remote CQC servers. So it can use GUI stuff from CQCGKit,
//  but CQCIGKit uses stuff from here. So we have to be careful about how we layer
//  things.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


// ---------------------------------------------------------------------------
//  Include our underlying headers
// ---------------------------------------------------------------------------
#include    "CIDLib.hpp"
#include    "CIDCtrls.hpp"
#include    "CIDWUtils.hpp"

#include    "CQCKit.hpp"
#include    "CQCRemBrws.hpp"
#include    "CQCEvCl.hpp"



// ---------------------------------------------------------------------------
//  Set up our import/export attributes
// ---------------------------------------------------------------------------
#if     defined(PROJ_CQCTREEBRWS)
#define CQCTREEBRWSEXP CID_DLLEXPORT
#else
#define CQCTREEBRWSEXP CID_DLLIMPORT
#endif


// ---------------------------------------------------------------------------
//  Facility types
// ---------------------------------------------------------------------------
namespace tCQCTreeBrws
{
    // -----------------------------------------------------------------------
    //  These are used in a callback that the containing app can register with the
    //  browser to be informed of changes, or to provide information.
    // -----------------------------------------------------------------------
    enum class EEvents
    {
        // Cause things to happen
        ClientDrv
        , Edit
        , Package
        , Paste
        , View

        // Inform of changes made
        , Deleted
        , NewFile
        , NewScope
        , PausedResumed
        , Renamed
        , SelChanged

        // Get info
        , CanRename
        , IsOpen

        , Count
    };


    // -----------------------------------------------------------------------
    //  The browser dialog takes this to indicate some limitations on the selection
    //
    //  Edit
    //      Means that they are selecting to do an edit, and so we should only show
    //      them ones they have the right to edit.
    //
    //  ReadOnly
    //      Means that we don't allow them to create, rename, delete, etc... They can
    //      only select something.
    //
    //  SelectItems
    //  SelectScopes
    //      Controls whether we show items and/or scopes. They have to provide at least
    //      one of these.
    // -----------------------------------------------------------------------
    enum class EFSelFlags : tCIDLib::TCard4
    {
        None                = 0x00000000
        , Edit              = 0x80000000
        , ReadOnly          = 0x40000000
        , SelectItems       = 0x00000001
        , SelectScopes      = 0x00000002
    };


    // -----------------------------------------------------------------------
    //  Styles for the browser window
    // -----------------------------------------------------------------------
    enum class EBrwsStyles : tCIDLib::TCard4
    {
        None                = 0x00000000
        , InvokeOnEnter     = 0x00000001
        , ReadOnly          = 0x00000002
        , StdBrowsers       = 0x00000004
    };
}


// ---------------------------------------------------------------------------
//  Include our headers, in the needed order.
// ---------------------------------------------------------------------------
#include    "CQCTreeBrws_ErrorIds.hpp"
#include    "CQCTreeBrws_MessageIds.hpp"

#include    "CQCTreeBrws_Shared.hpp"

#include    "CQCTreeBrws_BrowserIntf.hpp"
#include    "CQCTreeBrws_BrowseWnd.hpp"

#include    "CQCTreeBrws_ThisFacility.hpp"


// ---------------------------------------------------------------------------
//  Export the facility object lazy eval method
// ---------------------------------------------------------------------------
extern CQCTREEBRWSEXP TFacCQCTreeBrws& facCQCTreeBrws();



