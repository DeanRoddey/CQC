//
// FILE NAME: CQCTreeBrws_SystemBrws.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/06/2016
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
//  This file implements the /System derivative of the browser plugin interface.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCTreeBrws_.hpp"
#include    "CQCTreeBrws_SystemBrws_.hpp"



// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCSystemBrws,TCQCTreeBrwsIntf)



// ---------------------------------------------------------------------------
//   CLASS: TCQCSystemBrws
//  PREFIX: rbrws
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCSystemBrws: Constructors and Destructor
// ---------------------------------------------------------------------------

TCQCSystemBrws::TCQCSystemBrws() :

    TCQCTreeBrwsIntf
    (
        kCQCRemBrws::strPath_System
        , kCQCRemBrws::strItem_System
        , facCQCTreeBrws().strMsg(kTBrwsMsgs::midTitle_SystemBrowser)
    )
{
}

TCQCSystemBrws::~TCQCSystemBrws()
{
}


// ---------------------------------------------------------------------------
//  TCQCSystemBrws: Public, inherited methods
// ---------------------------------------------------------------------------

// Our browser object never accepts dropped files, so this won't get called
tCIDLib::TVoid
TCQCSystemBrws::AcceptFiles(const   TString&
                            , const tCIDLib::TStrList&
                            , const tCIDLib::TStrList&)
{
    CIDAssert2(L"The System browser should not be accepting files");
}


// We never accept dropped files in this section
tCIDLib::TBoolean
TCQCSystemBrws::bAcceptsNew(const TString&, const tCIDLib::TStrHashSet&) const
{
    return kCIDLib::False;
}


//
//  The browser window calls us here if the user invokes a menu operation on the
//  tree window. We have a very simple menu just as a formality, which just provides
//  a View option, and it's always enabled.
//
tCIDLib::TBoolean
TCQCSystemBrws::bDoMenuAction(  const   TString&            strPath
                                ,       TTreeBrowseInfo&    wnotToSend)
{
    // Get the area of this item, tell it to use just the text width
    TArea areaItem;
    wndBrowser().bQueryItemArea(strPath, areaItem, kCIDLib::True, kCIDLib::True);

    // Get the center point of it and convert to screen coordinates
    TPoint pntAt;
    wndBrowser().ToScreenCoordinates(areaItem.pntCenter(), pntAt);

    // Create the menu and load it up from the resource
    TPopupMenu menuAction(L"/System Action");

    tCIDLib::TCard4 c4MenuCmd = 0;
    menuAction.Create(facCQCTreeBrws(), kCQCTreeBrws::ridMenu_System);

    c4MenuCmd = menuAction.c4Process(wndBrowser(), pntAt, tCIDLib::EVJustify::Bottom);

    //
    //  If they made a choice, then we have to translate it to the standard action
    //  enum that the browser window will understand.
    //
    return bProcessMenuSelection(c4MenuCmd, strPath, wnotToSend);
}


// We don't have any connection, so we always say yes
tCIDLib::TBoolean TCQCSystemBrws::bIsConnected() const
{
    return kCIDLib::True;
}


//
//  Our own bDoMenuAction calls this if a selection is made. It is also called by the
//  browser window if an accelerator driven command is seen. That's why it's split out
//  so that we can avoid duplicating this code.
//
//  We just have a single data type (System) which we send for anything here. The path
//  indicates what type of system info it is.
//
tCIDLib::TBoolean
TCQCSystemBrws::bProcessMenuSelection(  const   tCIDLib::TCard4     c4CmdId
                                        , const TString&            strPath
                                        ,       TTreeBrowseInfo&    wnotToSend)
{
    tCIDLib::TBoolean bRet = kCIDLib::True;
    if (c4CmdId == kCQCTreeBrws::ridMenu_System_View)
    {
        wnotToSend = TTreeBrowseInfo
        (
            tCQCTreeBrws::EEvents::View, strPath, tCQCRemBrws::EDTypes::System, wndBrowser()
        );
    }
     else
    {
        bRet = kCIDLib::False;
    }
    return bRet;
}


//
//  For our stuff, as long as it's not our top level path or some other scope, it's fine
//  to report it all when double clicked. The client code will decide if it really
//  wants to deal with it and how. So just checking to see if it's a scope is a good
//  enough check. We indicate these are all view operations. The admin client knows what
//  to do with them base don path.
//
tCIDLib::TBoolean
TCQCSystemBrws::bReportInvocation(const TString& strPath, tCIDLib::TBoolean& bAsEdit) const
{
    bAsEdit = kCIDLib::False;
    return kCIDLib::True;
}


//
//  We add our top level scope and our child items. All of our hierarchy is fixed so we
//  can just put it in up front, non-virtual.
//
tCIDLib::TVoid TCQCSystemBrws::Initialize(const TCQCUserCtx& cuctxUser)
{
    TParent::Initialize(cuctxUser);

    TTreeView& wndTar = wndBrowser();

    //
    //  Add our top level scope. It's not marked as virtual in this case, since we
    //  preload all of the possible stuff.
    //
    wndTar.AddScope(kCQCRemBrws::strPath_Root, kCQCRemBrws::strItem_System, kCIDLib::False);

    // And the various other bits, all scopes being non-virtual
    wndTar.AddItem(kCQCRemBrws::strPath_System, L"Explore Logs");

    wndTar.AddScope(kCQCRemBrws::strPath_System, L"Monitor", kCIDLib::False);
    wndTar.AddItem(kCQCRemBrws::strPath_System_Monitor, L"Event Triggers");
}


// This should never be called for us
tCIDLib::TVoid TCQCSystemBrws::LoadScope(const TString& strPath)
{
    CIDAssert2(L"The System browser should not be adding scopes");
}


// This won't be called for us
tCIDLib::TVoid
TCQCSystemBrws::MakeDefName(const TString&, TString&, const tCIDLib::TBoolean) const
{
    CIDAssert2(L"The System browser should not be making default names");
}


//
//  If the browser window gets an accelerator key translation call, he will call us
//  here to load up an accelerator table for him which he will process. If it causes
//  him to get a menu call, he will pass it on to us.
//
tCIDLib::TVoid
TCQCSystemBrws::QueryAccelTable(const TString& strPath, TAccelTable& accelToFill) const
{
    //
    //  Just load it up from our menu. So we just create an instance of our menu but
    //  never show it.
    //
    TPopupMenu menuAction(L"/System Action");
    menuAction.Create(facCQCTreeBrws(), kCQCTreeBrws::ridMenu_System);

    // And now set up the accel table from the menu
    accelToFill.Create(menuAction);
}


// We don't use a persistent connection, so nothing really to do here
tCIDLib::TVoid TCQCSystemBrws::Terminate()
{
}

