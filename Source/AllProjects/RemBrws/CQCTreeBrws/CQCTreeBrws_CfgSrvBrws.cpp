//
// FILE NAME: CQCTreeBrws_CfgSrvBrws.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 12/11/2015
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
//  This file implements the remote browser derivative that handles browsing the
//  config server.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCTreeBrws_.hpp"
#include    "CQCTreeBrws_CfgSrvBrws_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCCfgSrvBrws,TCQCTreeBrwsIntf)



// ---------------------------------------------------------------------------
//   CLASS: TCQCCfgSrvBrws
//  PREFIX: rbrws
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCCfgSrvBrws: Constructors and Destructor
// ---------------------------------------------------------------------------

TCQCCfgSrvBrws::TCQCCfgSrvBrws() :

    TCQCTreeBrwsIntf
    (
        kCQCRemBrws::strPath_Configure
        , kCQCRemBrws::strItem_Configure
        , facCQCTreeBrws().strMsg(kTBrwsMsgs::midTitle_ConfBrower)
    )
{
}

TCQCCfgSrvBrws::~TCQCCfgSrvBrws()
{
}


// ---------------------------------------------------------------------------
//  TCQCCfgSrvBrws: Public, inherited methods
// ---------------------------------------------------------------------------

// Our browser object never accepts dropped files, so this won't get called
tCIDLib::TVoid
TCQCCfgSrvBrws::AcceptFiles(const   TString&
                            , const tCIDLib::TStrList&
                            , const tCIDLib::TStrList&)
{
    CIDAssert2(L"The config server browser should not be accepting files");
}


// We never accept dropped files in this section
tCIDLib::TBoolean
TCQCCfgSrvBrws::bAcceptsNew(const TString&, const tCIDLib::TStrHashSet&) const
{
    return kCIDLib::False;
}


//
//  The browser window calls us here if the user invokes a menu operation on the
//  tree window.
//
tCIDLib::TBoolean
TCQCCfgSrvBrws::bDoMenuAction(  const   TString&            strPath
                                ,       TTreeBrowseInfo&    wnotToSend)
{
    // Get the area of this item, tell it to use just the text width
    TArea areaItem;
    wndBrowser().bQueryItemArea(strPath, areaItem, kCIDLib::True, kCIDLib::True);

    // Get the center point of it and convert to screen coordinates
    TPoint pntAt;
    wndBrowser().ToScreenCoordinates(areaItem.pntCenter(), pntAt);

    // Create the menu and load it up from the resource
    TPopupMenu menuAction(L"/Configure Action");

    // Depending on the type of thing selected, we do a different menu
    tCIDLib::TBoolean bAtTop;
    const tCQCRemBrws::EDTypes eDType = ePathToDType(strPath, bAtTop);
    const tCIDLib::TBoolean bNoType(eDType == tCQCRemBrws::EDTypes::Count);

    const tCIDLib::TBoolean bScope = wndBrowser().bIsScope(strPath);
    tCIDLib::TCard4 c4MenuCmd = 0;
    menuAction.Create(facCQCTreeBrws(), kCQCTreeBrws::ridMenu_GenFile);

    // If an item, or not e-mail or user, then disable new
    if (!bScope
    ||  ((eDType != tCQCRemBrws::EDTypes::EMailAccount)
    &&   (eDType != tCQCRemBrws::EDTypes::User)))
    {
        menuAction.SetItemEnable(kCQCTreeBrws::ridMenu_GenFile_New, kCIDLib::False);
    }

    // If a scope, or not an e-mail or user, then disable delete
    if (bScope
    ||  ((eDType != tCQCRemBrws::EDTypes::EMailAccount)
    &&   (eDType != tCQCRemBrws::EDTypes::User)))
    {
        menuAction.SetItemEnable(kCQCTreeBrws::ridMenu_GenFile_Delete, kCIDLib::False);
    }

    // If a scope, disable the Edit and Copy. If not, disable the Refresh and Paste
    if (bScope)
    {
        menuAction.SetItemEnable(kCQCTreeBrws::ridMenu_GenFile_Copy, kCIDLib::False);
        menuAction.SetItemEnable(kCQCTreeBrws::ridMenu_GenFile_Edit, kCIDLib::False);
    }
     else
    {
        menuAction.SetItemEnable(kCQCTreeBrws::ridMenu_GenFile_Paste, kCIDLib::False);
        menuAction.SetItemEnable(kCQCTreeBrws::ridMenu_GenFile_Refresh, kCIDLib::False);
    }

    // We only allow email accounts to be renamed
    if (bScope || (eDType != tCQCRemBrws::EDTypes::EMailAccount))
        menuAction.SetItemEnable(kCQCTreeBrws::ridMenu_GenFile_Rename, kCIDLib::False);

    c4MenuCmd = menuAction.c4Process(wndBrowser(), pntAt, tCIDLib::EVJustify::Bottom);

    //
    //  If they made a choice, then we have to translate it to the standard action
    //  enum that the browser window will understand.
    //
    return bProcessMenuSelection(c4MenuCmd, strPath, wnotToSend);
}


// We don't use a persistent connection, so we just say yes
tCIDLib::TBoolean TCQCCfgSrvBrws::bIsConnected() const
{
    return kCIDLib::True;
}


//
//  Our own bDoMenuAction calls this if a selection is made. It is also called by the
//  browser window if an accelerator driven command is seen. That's why it's split out
//  so that we can avoid duplicating this code.
//
tCIDLib::TBoolean
TCQCCfgSrvBrws::bProcessMenuSelection(  const   tCIDLib::TCard4     c4CmdId
                                        , const TString&            strPath
                                        ,       TTreeBrowseInfo&    wnotToSend)
{
    // See what the data type is, if any
    tCIDLib::TBoolean bAtTop = kCIDLib::False;
    const tCQCRemBrws::EDTypes eType = ePathToDType(strPath, bAtTop);


    tCIDLib::TBoolean bRet = kCIDLib::True;
    switch(c4CmdId)
    {
        case kCQCTreeBrws::ridMenu_GenFile_Edit :
        {
            // Just inform the containing application that the user wants to edit
            CIDAssert(eType != tCQCRemBrws::EDTypes::Count, L"Unknown data type for Edit");
            wnotToSend = TTreeBrowseInfo
            (
                tCQCTreeBrws::EEvents::Edit, strPath, eType, wndBrowser()
            );
            break;
        }

        case kCQCTreeBrws::ridMenu_GenFile_New :
        {
            CIDAssert(eType != tCQCRemBrws::EDTypes::Count, L"Unknown data type for New");
            bRet = bMakeNewFile(strPath, eType, wnotToSend);
            break;
        }

        case kCQCTreeBrws::ridMenu_GenFile_Delete :
        {
            CIDAssert(eType != tCQCRemBrws::EDTypes::Count, L"Unknown data type for Delete");

            // Delete the indicated file
            if (bDeleteFile(strPath))
            {
                // Let any listeners know we did this
                wnotToSend = TTreeBrowseInfo
                (
                    tCQCTreeBrws::EEvents::Deleted, strPath, eType, wndBrowser()
                );
            }
            break;
        }

        case kCQCTreeBrws::ridMenu_GenFile_Refresh :
        {
            // We just handle this one internally
            UpdateScope(strPath);
            break;
        }

        case kCQCTreeBrws::ridMenu_GenFile_Rename :
        {
            // Call our parent's rename method which does what we want
            bRet = bDoRename(strPath, wnotToSend);
            break;
        }

        default :
            bRet = kCIDLib::False;
            break;
    };
    return bRet;

}


//
//  For our stuff, as long as it's not our top level path or some other scope, it's fine
//  to report it all when double clicked. The client code will decide if it really
//  wants to deal with it and how. So just checking to see if it's a scope is a good
//  enough check. We indicate these are all edit operations.
//
tCIDLib::TBoolean
TCQCCfgSrvBrws::bReportInvocation(const TString& strPath, tCIDLib::TBoolean& bAsEdit) const
{
    bAsEdit = kCIDLib::True;
    return !wndBrowser().bIsScope(strPath);
}


//
//  We add our top level scope and all of the next level ones since it's a fixed set
//  that we always have.
//
tCIDLib::TVoid TCQCCfgSrvBrws::Initialize(const TCQCUserCtx& cuctxUser)
{
    TParent::Initialize(cuctxUser);

    TTreeView& wndTar = wndBrowser();

    //
    //  Add our top level scope. It's not marked as virtual in this case, since we
    //  preload all of the possible top level items and sub-scopes. Some of our
    //  sub-scopes will be virtual.
    //
    wndTar.AddScope
    (
        kCQCRemBrws::strPath_Root, kCQCRemBrws::strItem_Configure, kCIDLib::False
    );

    //
    //  Load our main scopes. Some are virtual and will be faulted in only if
    //  they are accessed. Others we know the content and will go ahead and load them.
    //
    wndTar.AddScope(kCQCRemBrws::strPath_Configure, L"Accounts", kCIDLib::True);
    wndTar.AddScope(kCQCRemBrws::strPath_Accounts, L"EMail", kCIDLib::True);
    wndTar.AddScope(kCQCRemBrws::strPath_Configure, L"Ports", kCIDLib::True);
    wndTar.AddScope(kCQCRemBrws::strPath_Configure, L"Users", kCIDLib::True);

    //
    //  These are items at the top level, not scopes. These are for known, fixed data
    //  and cannot be removed.
    //
    wndTar.AddItem(kCQCRemBrws::strPath_Ports, L"GC-100");
    wndTar.AddItem(kCQCRemBrws::strPath_Ports, L"JustAddPwr");
    wndTar.AddItem(kCQCRemBrws::strPath_Configure, L"LogicSrv");
    wndTar.AddItem(kCQCRemBrws::strPath_Configure, L"Location");
    wndTar.AddItem(kCQCRemBrws::strPath_Configure, L"SystemCfg");
}


tCIDLib::TVoid
TCQCCfgSrvBrws::LoadScope(const TString& strPath)
{
    TTreeView& wndTar = wndBrowser();

    try
    {
        //
        //  Depending on the path, let's load up the relevant info and put it into
        //  the tree.
        //
        if (strPath.bStartsWithI(kCQCRemBrws::strPath_Users))
            LoadUsers(wndTar);
        else if (strPath.bStartsWithI(kCQCRemBrws::strPath_EMailAccts))
            LoadEMails(wndTar);

        //
        //  Probably this happened, if we added any above. But, if the scope was
        //  empty, then we need to do this just in case, to clear set the child
        //  count info, which also turns off the virtual scope flag, so we won't
        //  try to fault this one back in.
        //
        wndTar.UpdateChildCnt(strPath);

        //
        //  At the end of an expansion, force the expanded once state on. It won't
        //  get done if this is a lazily faulted in tree and the expanded scope
        //  ended up being empty. That will cause lots of problems later.
        //
        wndTar.ForceExpandedOnce(strPath);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        facCQCGKit().ShowError
        (
            wndBrowser(), strTitle(), L"Could not expand item", errToCatch
        );
    }
}


//
//  Some of our stuff doesn't use this, like user accounts, because they have to have a
//  correct name up front. For other stuff we create a default name based on the parent
//  scope.
//
tCIDLib::TVoid
TCQCCfgSrvBrws::MakeDefName(const   TString&            strParScope
                            ,       TString&            strToFill
                            , const tCIDLib::TBoolean   bIsFile) const
{
    tCIDLib::TBoolean bAtTop = kCIDLib::False;
    const tCQCRemBrws::EDTypes eType = ePathToDType(strParScope, bAtTop);
    CIDAssert(bAtTop, L"The parent scope was not the top level scope for its type");

    // Build up the base name for the new guy
    TString strCurPath(strParScope);
    strCurPath.Append(kCIDLib::chForwardSlash);
    strCurPath.Append(tCQCRemBrws::strXlatEDTypes(eType));

    // Now we add numbers to it until we get a unique name not already in this scope.
    const tCIDLib::TCard4 c4BaseLen = strCurPath.c4Length();
    tCIDLib::TCard4 c4Num = 1;
    while (kCIDLib::True)
    {
        strCurPath.CapAt(c4BaseLen);
        strCurPath.AppendFormatted(c4Num);

        if (!wndBrowser().bPathExists(strCurPath))
        {
            strToFill = tCQCRemBrws::strXlatEDTypes(eType);
            strToFill.AppendFormatted(c4Num);
            break;
        }

        // Not unique, to try another
        c4Num++;
    }
}


//
//  If the browser window gets an accelerator key translation call, he will call us
//  here to load up an accelerator table for him which he will process. If it causes
//  him to get a menu call, he will pass it on to us.
//
tCIDLib::TVoid
TCQCCfgSrvBrws::QueryAccelTable(const  TString&        strPath
                                ,      TAccelTable&    accelToFill) const
{
    //
    //  Just load it up from our menu. So we just create an instance of our menu but
    //  never show it.
    //
    TPopupMenu menuAction(L"/Configure Action");

    // Depending on the type of thing selected, we do a different menu
    tCIDLib::TBoolean bAtTop;
    const tCQCRemBrws::EDTypes eDType = ePathToDType(strPath, bAtTop);

    if ((eDType == tCQCRemBrws::EDTypes::EMailAccount)
    ||  (eDType == tCQCRemBrws::EDTypes::User))
    {
        menuAction.Create(facCQCTreeBrws(), kCQCTreeBrws::ridMenu_GenFile);
        if (!bAtTop)
            menuAction.SetItemEnable(kCQCTreeBrws::ridMenu_GenFile_New, kCIDLib::False);
    }
     else
    {
        // Don't have a menu for this so nothing
        return;
    }

    // And now set up the accel table from the menu
    accelToFill.Create(menuAction);
}


// We don't use a persistent connection, so nothing really to do here
tCIDLib::TVoid TCQCCfgSrvBrws::Terminate()
{
}


// ---------------------------------------------------------------------------
//  TCQCCfgSrvBrws: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TCQCCfgSrvBrws::bCanRename(const TString& strPath) const
{
    //
    //  The only things we currently allow to be renamed are e-mail accounts. Everything
    //  else is all special stuff.
    //
    return
    (
        !wndBrowser().bIsScope(strPath)
        && strPath.bStartsWithI(kCQCRemBrws::strPath_EMailAccts)
    );
}



tCIDLib::TBoolean
TCQCCfgSrvBrws::bRenameItem(const   TString&            strParScope
                            , const TString&            strOldName
                            , const TString&            strNewName
                            , const tCIDLib::TBoolean   bIsScope)
{
    try
    {
        // Depending on the parent scope, we do the appropriate thing
        if (strParScope.bStartsWithI(kCQCRemBrws::strPath_EMailAccts))
        {
            TWndPtrJanitor janPtr(tCIDCtrls::ESysPtrs::Wait);
            tCQCKit::TInstSrvProxy orbcInst = facCQCKit().orbcInstSrvProxy();
            orbcInst->RenameEMailAccount(strOldName, strNewName, sectUser());

            // And now tell the tree to update the display text of the item
            wndBrowser().UpdateItem(strParScope, strOldName, strNewName);
        }
         else
        {
            TErrBox msgbErr(strTitle(), L"This data type cannot be renamed");
            msgbErr.ShowIt(wndBrowser());
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        facCQCGKit().ShowError
        (
            wndBrowser(), strTitle(), L"Could not expand item", errToCatch
        );
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


// ---------------------------------------------------------------------------
//  TCQCCfgSrvBrws: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  This is called if we get a delete command (via menu or hot key). We look at
//  the type and do the appropriate
//
tCIDLib::TBoolean TCQCCfgSrvBrws::bDeleteFile(const TString& strPath)
{
    // Make sure that they really want to do it
    TYesNoBox msgbConfirm
    (
        strTitle(), facCQCTreeBrws().strMsg(kTBrwsMsgs::midQ_DeleteItem, strPath)
    );
    if (!msgbConfirm.bShowIt(wndBrowser()))
        return kCIDLib::False;

    tCQCRemBrws::EDTypes eType = tCQCRemBrws::EDTypes::Count;
    try
    {
        if (strPath.bStartsWithI(kCQCRemBrws::strPath_Users))
        {
            // Set this first in case of error
            eType = tCQCRemBrws::EDTypes::User;

            //
            //  Let's try to delete the indicated user. The last part of the path is
            //  the login name. We can get an error, even if the login name is valid,
            //  if they try to delete the last admin account.
            //
            TString strLoginName;
            facCQCRemBrws().QueryNamePart(strPath, strLoginName);
            tCQCKit::TSecuritySrvProxy orbcSrc = facCQCKit().orbcSecuritySrvProxy();
            orbcSrc->DeleteAccount(strLoginName, sectUser());
        }
         else if (strPath.bStartsWithI(kCQCRemBrws::strPath_EMailAccts))
        {
            // Set this first in case of error
            eType = tCQCRemBrws::EDTypes::EMailAccount;

            TString strAcctName;
            facCQCRemBrws().QueryNamePart(strPath, strAcctName);

            TWndPtrJanitor janPtr(tCIDCtrls::ESysPtrs::Wait);
            tCQCKit::TInstSrvProxy orbcInst = facCQCKit().orbcInstSrvProxy();
            if (!orbcInst->bDeleteEMailAccount(strAcctName, sectUser()))
            {
                facCQCTreeBrws().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kTBrwsErrs::errcBrws_EMailDelete
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::NotFound
                    , strAcctName
                );
            }
        }
         else
        {
            //
            //  Shouldn't happen, since we don't allow it in menus and we don't create
            //  a delete accellerator unless it's a deletable item. But, just in case.
            //
            facCQCGKit().ShowError
            (
                wndBrowser()
                , strTitle()
                , L"This item cannot be deleted, and you shouldn't have been able to "
                  L" try to do so. Sorry about that. Please report this issue."
            );
            return kCIDLib::False;
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        TString strMsg = TString::strConcat
        (
            L"An error occurred while deleting the ", tCQCRemBrws::strLoadEDTypes(eType)
        );
        facCQCGKit().ShowError(wndBrowser(), strTitle(), strMsg, errToCatch);
        return kCIDLib::False;
    }

    // It didn't fail, so remove this item from the browser window
    wndBrowser().RemoveItem(strPath);

    return kCIDLib::True;
}


//
//  This is called when we get a New menu selection. We see if it's one of the types
//  of ours that we can create (it should be since the menu shouldn't have allowed
//  otherwise.) We try to create the new file and get it into the browser.
//
tCIDLib::TBoolean
TCQCCfgSrvBrws::bMakeNewFile(const  TString&                strParHPath
                            , const tCQCRemBrws::EDTypes    eDType
                            ,       TTreeBrowseInfo&        wnotToSend)
{
    // We need the browser a number of times so get a convenient ref
    TCQCTreeBrowser& wndTar = wndBrowser();

    //
    //  The first thing we need to do is to get a name from the user. We call
    //  a helper on the browser window do this since it's sort of messy. If they
    //  don't commit, we return and nothing has been done.
    //
    TString strNewName;
    if (!wndTar.bGetNewName(strParHPath, kCIDLib::False, eDType, strNewName, *this))
        return kCIDLib::False;

    // Build up the new full hierarchical path
    TString strNewHPath(strParHPath);
    facCQCRemBrws().AddPathComp(strNewHPath, strNewName);

    //
    //  At this point the item is in the tree. We need to make sure it gets removed
    //  if we don't explicitly decide to keep it.
    //
    TTreeItemJan janTree(&wndTar, strNewHPath);

    // Get the type relative version of the parent path
    TString strParRelPath;
    facCQCRemBrws().CreateRelPath(strParHPath, eDType, strParRelPath);

    tCIDLib::TBoolean bRet = kCIDLib::False;
    try
    {
        if (eDType == tCQCRemBrws::EDTypes::EMailAccount)
        {
            bRet = bMakeNewEmailAcct(strParHPath, strParRelPath, strNewName);
        }
         else if (eDType == tCQCRemBrws::EDTypes::User)
        {
            bRet = bMakeNewUserAcct(strParHPath, strParRelPath, strNewName);
        }
         else
        {
            // Not something we can edit
            TErrBox msgbView
            (
                strTitle()
                , facCQCTreeBrws().strMsg
                  (
                    kTBrwsMsgs::midStatus_CantNew
                    , tCQCRemBrws::strLoadEDTypes(eDType)
                    , strNewHPath
                  )
            );
            msgbView.ShowIt(wndBrowser());
            return kCIDLib::False;
        }

        // It appears to have worked, so prevent the janitor from removing it
        janTree.Orphan();
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        TErrBox msgbNew
        (
            strTitle()
            , facCQCRemBrws().strMsg(kTBrwsMsgs::midStatus_CantCreateFile, strNewHPath)
        );
        msgbNew.ShowIt(wndTar);
        return kCIDLib::False;
    }

    // Select this new guy
    wndTar.SelectPath(strNewHPath);

    // It workeed so fill in a notification to be sent to the containing app
    wnotToSend = TTreeBrowseInfo
    (
        tCQCTreeBrws::EEvents::NewFile, strNewHPath, eDType, wndBrowser()
    );
    return bRet;
}


//
//  This is called to create a e-mail account. We just generate an empty one with
//  a default name, save it which also adds it to the tree view, and select it. We don't
//  need the hierarchy path info here for this.
//
tCIDLib::TBoolean
TCQCCfgSrvBrws::bMakeNewEmailAcct(const TString&, const TString&, const TString& strNewName)
{
    // Set up a default account
    TCQCEMailAccount emacctNew;
    emacctNew.Set
    (
        tCQCKit::EEMailTypes::Unused
        , strNewName
        , L"me@myisp.com"
        , L"HappyWeaselParty"
        , L"myisp.com"
        , L"Bubba"
        , 25
    );

    TWndPtrJanitor janPtr(tCIDCtrls::ESysPtrs::Wait);
    tCQCKit::TInstSrvProxy orbcInst = facCQCKit().orbcInstSrvProxy();
    tCIDLib::TCard4 c4SerNum = 0;

    // Indicate it must be a new account, if not it will throw
    return orbcInst->bUpdateEmailAccount(emacctNew, c4SerNum, sectUser(), kCIDLib::True);
}


//
//  Called when the user asks to create a new user account. This one is a little special.
//  In this case, we don't get a default name. The name has to be set up front and
//  cannot be changed once it's created and stored away. So in this special case we
//  get a login name from the user up front and use that.
//
tCIDLib::TBoolean
TCQCCfgSrvBrws::bMakeNewUserAcct(const  TString&        strParHPath
                                , const TString&        strParRelPath
                                , const TString&        strNewName)
{
    // Set up a user account with basic info
    TCQCUserAccount uaccNew
    (
        tCQCKit::EUserRoles::NormalUser
        , L"New user account"
        , strNewName
        , L"Welcome"
        , L"John"
        , L"Smith"
    );

    // Create the hierarchical path for display purposes
    TString strNewHPath(strParHPath);
    strNewHPath.Append(kCIDLib::chForwardSlash);
    strNewHPath.Append(strNewName);

    tCQCKit::TSecuritySrvProxy orbcSrc = facCQCKit().orbcSecuritySrvProxy();

    //
    //  In order to provide a more targeted error message for a common possible
    //  error, we check to see if this login name already exists. We have to provide
    //  the current user's security token to get this info.
    //
    if (orbcSrc->bLoginExists(strNewName, cuctxUser().sectUser()))
    {
        TErrBox msgbNew
        (
            strTitle()
            , facCQCTreeBrws().strMsg(kTBrwsErrs::errcBrws_UAccExists, strNewName)
        );
        msgbNew.ShowIt(wndBrowser());
        return kCIDLib::False;
    }

    orbcSrc->CreateAccount(uaccNew, sectUser());
    return kCIDLib::True;
}


//
//  Check the path to see if starts with the top level path for any of the types we
//  deal with. If not, we return the _Count value.
//
//  We also indicate if it was the top level for that type, or is on some sub-scope of
//  it.
//
tCQCRemBrws::EDTypes
TCQCCfgSrvBrws::ePathToDType(const TString& strPath, tCIDLib::TBoolean& bAtTop) const
{
    bAtTop = kCIDLib::False;
    tCQCRemBrws::EDTypes eRet = tCQCRemBrws::EDTypes::Count;

    const TString* pstrRoot = nullptr;
    if (strPath.bStartsWithI(kCQCRemBrws::strPath_EMailAccts))
    {
        eRet = tCQCRemBrws::EDTypes::EMailAccount;
        pstrRoot = &kCQCRemBrws::strPath_EMailAccts;
    }
     else if (strPath.bStartsWithI(kCQCRemBrws::strPath_GC100Ports))
    {
        eRet = tCQCRemBrws::EDTypes::GC100Ports;
        pstrRoot = &kCQCRemBrws::strPath_GC100Ports;
    }
     else if (strPath.bStartsWithI(kCQCRemBrws::strPath_JAPwrPorts))
    {
        eRet = tCQCRemBrws::EDTypes::JAPwrPorts;
        pstrRoot = &kCQCRemBrws::strPath_JAPwrPorts;
    }
     else if (strPath.bStartsWithI(kCQCRemBrws::strPath_Location))
    {
        eRet = tCQCRemBrws::EDTypes::Location;
        pstrRoot = &kCQCRemBrws::strPath_Location;
    }
     else if (strPath.bStartsWithI(kCQCRemBrws::strPath_LogicSrv))
    {
        eRet = tCQCRemBrws::EDTypes::LogicSrv;
        pstrRoot = &kCQCRemBrws::strPath_LogicSrv;
    }
     else if (strPath.bStartsWithI(kCQCRemBrws::strPath_SystemCfg))
    {
        eRet = tCQCRemBrws::EDTypes::SystemCfg;
        pstrRoot = &kCQCRemBrws::strPath_SystemCfg;
    }
     else if (strPath.bStartsWithI(kCQCRemBrws::strPath_Users))
    {
        eRet = tCQCRemBrws::EDTypes::User;
        pstrRoot = &kCQCRemBrws::strPath_Users;
    }

    if (eRet != tCQCRemBrws::EDTypes::Count)
        bAtTop = (strPath == *pstrRoot);

    return eRet;
}


//
//  Called when it's time to fault in our email accounts scope or the user asks to reload
//  the scope. In our case we just need to interate the config server scope that contains
//  the email accounts and load an entry for each one.
//
//  We let exceptions propagate.
//
tCIDLib::TVoid TCQCCfgSrvBrws::LoadEMails(TTreeView& wndTar)
{
    TWndPtrJanitor janPtr(tCIDCtrls::ESysPtrs::Wait);
    tCQCKit::TInstSrvProxy orbcInst = facCQCKit().orbcInstSrvProxy();

    tCIDLib::TStrList colList;
    orbcInst->bQueryEMailAccountNames(colList, sectUser());

    colList.bForEach([&wndTar](const TString& strName)
    {
        wndTar.AddItem(kCQCRemBrws::strPath_EMailAccts, strName);
        return kCIDLib::True;
    });
}


//
//  Called when it's time to fault in our users scope or the user asks to reload the
//  scope.
//
//  We let exceptions propagate.
//
tCIDLib::TVoid TCQCCfgSrvBrws::LoadUsers(TTreeView& wndTar)
{
    tCQCKit::TSecuritySrvProxy orbcSrc = facCQCKit().orbcSecuritySrvProxy();

    // If we don't get any, then we are done
    TVector<TCQCUserAccount> colList;
    if (!orbcSrc->c4QueryAccounts(colList, sectUser()))
        return;

    //
    //  Loop though the list and load them into the tree. The text is the login
    //  name, which is also the key to upload changes, delete users and so forth.
    //
    TString strCurPath;
    const tCIDLib::TCard4 c4Count = colList.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TCQCUserAccount& uaccCur = colList[c4Index];
        wndTar.AddItem(kCQCRemBrws::strPath_Users, uaccCur.strLoginName());
    }
}


//
//  We query the contents of the indicated path, which must be a scope. We don't
//  try to be clever. We temporarily disable drawing, remove the contents of
//  the scope, then call our scope loader above to reload it. This also serves to
//  toss all of the nested scopes, so we go back to virtual scope mode on all of
//  them, to be subsequently reloaded as well. Not the most efficient, but the
//  safest.
//
tCIDLib::TVoid TCQCCfgSrvBrws::UpdateScope(const TString& strPath)
{
    TTreeView& wndTar = wndBrowser();

    try
    {
        // Has to be a scope
        CIDAssert(wndTar.bIsScope(strPath), L"Cannot update a non-scope item");

        // Stop updates while we do this
        TWndPaintJanitor janPaint(&wndTar);

        wndBrowser().RemoveChildrenOf(strPath);

        // Now call the same private helper used to do the initial load
        if (strPath.bStartsWithI(kCQCRemBrws::strPath_Users))
            LoadUsers(wndTar);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        facCQCGKit().ShowError
        (
            wndBrowser(), strTitle(), L"Could not update scope", errToCatch
        );
    }
}

