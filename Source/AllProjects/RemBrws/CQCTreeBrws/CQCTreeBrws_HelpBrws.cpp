//
// FILE NAME: CQCTreeBrws_HelpBrws.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 05/18/2016
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
//  This file implements the help derivative of the browser plugin interface.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCTreeBrws_.hpp"
#include    "CQCTreeBrws_HelpBrws_.hpp"



// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCHelpBrws,TCQCTreeBrwsIntf)



// ---------------------------------------------------------------------------
//   CLASS: TCQCHelpBrws
//  PREFIX: rbrws
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCHelpBrws: Constructors and Destructor
// ---------------------------------------------------------------------------

TCQCHelpBrws::TCQCHelpBrws() :

    TCQCTreeBrwsIntf
    (
        kCQCRemBrws::strPath_Help
        , kCQCRemBrws::strItem_Configure
        , facCQCTreeBrws().strMsg(kTBrwsMsgs::midTitle_HelpBrowser)
    )
{
}

TCQCHelpBrws::~TCQCHelpBrws()
{
}


// ---------------------------------------------------------------------------
//  TCQCHelpBrws: Public, inherited methods
// ---------------------------------------------------------------------------

// Our browser object never accepts dropped files, so this won't get called
tCIDLib::TVoid
TCQCHelpBrws::AcceptFiles(const TString&, const tCIDLib::TStrList&, const tCIDLib::TStrList&)
{
    CIDAssert2(L"The help browser should not be accepting files");
}


// We never accept dropped files in this section
tCIDLib::TBoolean
TCQCHelpBrws::bAcceptsNew(const TString&, const tCIDLib::TStrHashSet&) const
{
    return kCIDLib::False;
}


//
//  The browser window calls us here if the user invokes a menu operation on the
//  tree window.
//
tCIDLib::TBoolean
TCQCHelpBrws::bDoMenuAction(const   TString&            strPath
                            ,       TTreeBrowseInfo&    wnotToSend)
{
    // Get the area of this item, tell it to use just the text width
    TArea areaItem;
    wndBrowser().bQueryItemArea(strPath, areaItem, kCIDLib::True, kCIDLib::True);

    // Get the center point of it and convert to screen coordinates
    TPoint pntAt;
    wndBrowser().ToScreenCoordinates(areaItem.pntCenter(), pntAt);

    // Create the menu and load it up from the resource
    TPopupMenu menuAction(L"/Help Action");

    tCIDLib::TCard4 c4MenuCmd = 0;
    menuAction.Create(facCQCTreeBrws(), kCQCTreeBrws::ridMenu_Help);

    c4MenuCmd = menuAction.c4Process(wndBrowser(), pntAt, tCIDLib::EVJustify::Bottom);

    //
    //  If they made a choice, then we have to translate it to the standard action
    //  enum that the browser window will understand.
    //
    return bProcessMenuSelection(c4MenuCmd, strPath, wnotToSend);
}


// We don't have any connection, so we always say yes
tCIDLib::TBoolean TCQCHelpBrws::bIsConnected() const
{
    return kCIDLib::True;
}


//
//  Our own bDoMenuAction calls this if a selection is made. It is also called by the
//  browser window if an accelerator driven command is seen. That's why it's split out
//  so that we can avoid duplicating this code.
//
tCIDLib::TBoolean
TCQCHelpBrws::bProcessMenuSelection(const   tCIDLib::TCard4     c4CmdId
                                    , const TString&            strPath
                                    ,       TTreeBrowseInfo&    wnotToSend)
{
    tCIDLib::TBoolean bRet = kCIDLib::True;
    if (c4CmdId == kCQCTreeBrws::ridMenu_Help_View)
    {
        wnotToSend = TTreeBrowseInfo
        (
            tCQCTreeBrws::EEvents::View, strPath, tCQCRemBrws::EDTypes::Help, wndBrowser()
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
//  to report it all when double clicked. The client code will decide if it really wants to
//  deal with it and how. So just checking to see if it's a scope is a good enough check.
//  We indicate these are all view operations.
//
tCIDLib::TBoolean
TCQCHelpBrws::bReportInvocation(const TString& strPath, tCIDLib::TBoolean& bAsEdit) const
{
    bAsEdit = kCIDLib::False;

    // If not a scope, allow the invocation
    return !wndBrowser().bIsScope(strPath);
}


//
//  We add our top level scope and some child items for special interest help items.
//
tCIDLib::TVoid TCQCHelpBrws::Initialize(const TCQCUserCtx& cuctxUser)
{
    TParent::Initialize(cuctxUser);

    TTreeView& wndTar = wndBrowser();

    //
    //  Add our scopes. They are not marked as virtual in this case, since we preload all
    //  of the possible stuff.
    //
    wndTar.AddScope(kCQCRemBrws::strPath_Root, kCQCRemBrws::strItem_Help, kCIDLib::False);
    wndTar.AddScope(kCQCRemBrws::strPath_Help, kCQCRemBrws::strItem_Docs, kCIDLib::False);
    wndTar.AddScope(kCQCRemBrws::strPath_Help_Docs, kCQCRemBrws::strItem_Actions, kCIDLib::False);
    wndTar.AddScope(kCQCRemBrws::strPath_Help_Docs, kCQCRemBrws::strItem_CML, kCIDLib::False);
    wndTar.AddScope(kCQCRemBrws::strPath_Help_Docs, kCQCRemBrws::strItem_Interface, kCIDLib::False);
    wndTar.AddScope(kCQCRemBrws::strPath_Help_Docs, kCQCRemBrws::strItem_Drivers, kCIDLib::False);
    wndTar.AddScope(kCQCRemBrws::strPath_Help_Docs, kCQCRemBrws::strItem_VoiceControl, kCIDLib::False);

    wndTar.AddItem(kCQCRemBrws::strPath_Help_Actions, L"Big Picture");
    wndTar.AddItem(kCQCRemBrws::strPath_Help_Actions, L"Reference");

    wndTar.AddItem(kCQCRemBrws::strPath_Help_CML, L"Big Picture");
    wndTar.AddItem(kCQCRemBrws::strPath_Help_CML, L"Reference");

    wndTar.AddItem(kCQCRemBrws::strPath_Help_Interface, L"Big Picture");
    wndTar.AddItem(kCQCRemBrws::strPath_Help_Interface, L"Reference");

    wndTar.AddItem(kCQCRemBrws::strPath_Help_Docs, L"Help Home");
    wndTar.AddItem(kCQCRemBrws::strPath_Help_Docs, L"Introduction");
    wndTar.AddItem(kCQCRemBrws::strPath_Help_Docs, L"Installation");

    wndTar.AddItem(kCQCRemBrws::strPath_Help_Docs, L"Tools");
    wndTar.AddItem(kCQCRemBrws::strPath_Help_Docs, L"V2 Classes");
    wndTar.AddItem(kCQCRemBrws::strPath_Help_Docs, L"Videos");

    wndTar.AddItem(kCQCRemBrws::strPath_Help_Drivers, L"Big Picture");
    wndTar.AddItem(kCQCRemBrws::strPath_Help_Drivers, L"Reference");

    wndTar.AddItem(kCQCRemBrws::strPath_Help_VoiceControl, L"Overview");
    wndTar.AddItem(kCQCRemBrws::strPath_Help_VoiceControl, L"Echo");
    wndTar.AddItem(kCQCRemBrws::strPath_Help_VoiceControl, L"CQCVoice");

    wndTar.AddItem(kCQCRemBrws::strPath_Help, L"About");
}


// This should never be called for us
tCIDLib::TVoid TCQCHelpBrws::LoadScope(const TString& strPath)
{
    CIDAssert2(L"The help browser should not be adding scopes");
}


// This won't be called for us
tCIDLib::TVoid
TCQCHelpBrws::MakeDefName(const TString&, TString&, const tCIDLib::TBoolean) const
{
    CIDAssert2(L"The help browser should not be making default names");
}


//
//  If the browser window gets an accelerator key translation call, he will call us
//  here to load up an accelerator table for him which he will process. If it causes
//  him to get a menu call, he will pass it on to us.
//
tCIDLib::TVoid
TCQCHelpBrws::QueryAccelTable(  const   TString&        strPath
                                ,       TAccelTable&    accelToFill) const
{
    //
    //  Just load it up from our menu. So we just create an instance of our menu but
    //  never show it.
    //
    TPopupMenu menuAction(L"/Help Action");
    menuAction.Create(facCQCTreeBrws(), kCQCTreeBrws::ridMenu_Help);

    // And now set up the accel table from the menu
    accelToFill.Create(menuAction);
}


// We don't use a persistent connection, so nothing really to do here
tCIDLib::TVoid TCQCHelpBrws::Terminate()
{
}

