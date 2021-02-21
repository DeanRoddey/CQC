//
// FILE NAME: CQCTreeBrws_RemBrowseDlg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/24/2015
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
//  This file implements a single resource type file selector for the data server
//  interface.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCTreeBrws_.hpp"
#include    "CQCTreeBrws_DataSrvBrws_.hpp"
#include    "CQCTreeBrws_RemBrowseDlg_.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TRemBrwsDlg, TDlgBox)



// ---------------------------------------------------------------------------
//   CLASS: TRemBrwsDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TRemBrwsDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TRemBrwsDlg::TRemBrwsDlg() :

    m_bOrgRelative(kCIDLib::False)
    , m_eFlags(tCQCTreeBrws::EFSelFlags::None)
    , m_eDType(tCQCRemBrws::EDTypes::Count)
    , m_prbbcEvents(nullptr)
    , m_pstrRelToPath(nullptr)
    , m_pwndAccept(nullptr)
    , m_pwndCancel(nullptr)
    , m_pwndCopyPath(nullptr)
    , m_pwndCurPath(nullptr)
    , m_pwndCurPreview(nullptr)
    , m_pwndImgPreview(nullptr)
    , m_pwndRelPath(nullptr)
    , m_pwndShowPreview(nullptr)
    , m_pwndTree(nullptr)
{
    //
    //  If we haven't sized up the last paths vector, do that. We just add empty strings
    //  initially.
    //
    if (s_colLastTypePaths.bIsEmpty())
    {
        tCQCRemBrws::EDTypes eType = tCQCRemBrws::EDTypes::Min;
        while (eType <= tCQCRemBrws::EDTypes::Max)
        {
            s_colLastTypePaths.objAdd(TString::strEmpty());
            eType++;
        }
    }
}

TRemBrwsDlg::~TRemBrwsDlg()
{
}


// ---------------------------------------------------------------------------
//  TRemBrwsDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TRemBrwsDlg::bRunDlg(const  TWindow&                    wndOwner
                    , const TString&                    strTitle
                    , const TString&                    strPath
                    , const TCQCUserCtx&                cuctxToUse
                    , const tCQCRemBrws::EDTypes        eDType
                    , const tCQCTreeBrws::EFSelFlags    eFlags
                    ,       TString&                    strSelected
                    ,       MRemBrwsBrwsCB* const       prbbcEvents
                    , const TString* const              pstrRelToPath)
{
    // Store parms so that we can use them later
    m_cuctxToUse = cuctxToUse;
    m_eFlags = eFlags;
    m_eDType = eDType;
    m_prbbcEvents = prbbcEvents;
    m_pstrRelToPath = pstrRelToPath;
    m_strPath = strPath;
    m_strTitle = strTitle;

    //
    //  Do some checks where we we will ignore the incoming if it doesn't look right.
    //  We'll clear it out and that will cause us to use a default below.
    //
    if (!m_strPath.bIsEmpty())
    {
        const tCIDLib::TBoolean bRelOK(eDType == tCQCRemBrws::EDTypes::Template);
        TString strErr;
        if (!facCQCKit().bIsValidRemTypeRelPath(m_strPath, strErr, bRelOK))
        {
            // It's not even syntactically valid, so start over
            m_strPath.Clear();
        }
         else if (!strPath.bStartsWith(L"/"))
        {
            //
            //  It's valid and doesn't start with a slash, so it has to be a relative
            //  path that's basically valid. So we have to have a relative to path and
            //  we have to be able to expand it out to a full hierarchical path. If not
            //  clear it out.
            //
            //  Use the original parameter here since we are expanding into m_strPath! If
            //  we get it expanded remember that it was relative, for later use.
            //
            if (pstrRelToPath && facCQCKit().bExpandResPath(*pstrRelToPath, strPath, m_strPath))
                m_bOrgRelative = kCIDLib::True;
            else
                m_strPath.Clear();
        }
    }

    // If we got nothing or it wasn't good, take a default
    if (m_strPath.bIsEmpty())
    {
        //
        //  First see if we have a previously stored 'last path' for this data type. If
        //  not, create a default by starting at the top of the type-specific part of
        //  the hierarchy either in /System or /User.
        //
        m_strPath = s_colLastTypePaths[eDType];
        if (m_strPath.bIsEmpty())
        {
            m_strPath = facCQCRemBrws().strFlTypePref(eDType);
            if (facCQCKit().bSysEdit())
                facCQCRemBrws().AddPathComp(m_strPath, L"System");
            else
                facCQCRemBrws().AddPathComp(m_strPath, L"User");
        }
    }

    //
    //  Now we need to expand it out a full hierarchical path which is what the browser
    //  window wants.
    //
    {
        TString strTmp(m_strPath);
        facCQCRemBrws().ToHPath(strTmp, eDType, m_strPath);
    }

    //
    //  Store the top limit, if any. We need to convert it to full data server
    //  hierarchical path so that we can efficiently compare it to the full hierachical
    //  paths we get from the browser window. If none was passed in, take the top level
    //  path for the data type.
    //
    const TString& strBase = facCQCRemBrws().strFlTypePref(m_eDType);
    TString strUserBase(TStrCat(strBase, L"/User"));

    // If edit mode, restrict to the user section, else just the base path
    if (tCIDLib::bAllBitsOn(eFlags, tCQCTreeBrws::EFSelFlags::Edit) && !facCQCKit().bSysEdit())
        m_strTopLimit = strUserBase;
    else
        m_strTopLimit = strBase;

    //
    //  Load up the dialog description. We'll modify it based on whether we are doing
    //  images or not, so that everything just gets created right to begin with.
    //
    TDlgDesc dlgdLoad;
    if (!facCQCTreeBrws().bCreateDlgDesc(kCQCTreeBrws::ridDlg_Browse, dlgdLoad))
    {
        CIDAssert2(L"Could not load the remote browser dialog's dlg resource");
    }

    //
    //  If we have no relative path then delete that check box and adjust our height
    //  down to get rid of that extra space.
    //
    if (!m_pstrRelToPath)
    {
        const TDlgItem& dlgiRelPath = dlgdLoad.dlgiFindById(kCQCTreeBrws::ridDlg_Browse_RelPath);
        const TArea areaRelPath = dlgiRelPath.areaPos();

        TArea areaNew = dlgdLoad.areaPos();
        areaNew.i4Bottom(dlgiRelPath.areaPos().i4Y());
        dlgdLoad.areaPos(areaNew);
    }

    // If not doing images, then adjust the content
    if (m_eDType != tCQCRemBrws::EDTypes::Image)
    {
        //
        // Get the area of the preview window. That will become the new right side of our
        //  overall area.
        //
        const TDlgItem& dlgiPreview = dlgdLoad.dlgiFindById(kCQCTreeBrws::ridDlg_Browse_ImgPreview);
        const TArea areaPreview = dlgiPreview.areaPos();
        const TDlgItem& dlgiBrowser = dlgdLoad.dlgiFindById(kCQCTreeBrws::ridDlg_Browse_Tree);
        const TArea areaBrowser = dlgiBrowser.areaPos();

        // And now we can remove it and the preview button and preview path
        dlgdLoad.RemoveItem(kCQCTreeBrws::ridDlg_Browse_CurPreview);
        dlgdLoad.RemoveItem(kCQCTreeBrws::ridDlg_Browse_ImgPreview);
        dlgdLoad.RemoveItem(kCQCTreeBrws::ridDlg_Browse_ShowPreview);

        //
        //  Size ourself down to fit, making our right side as far from the right side of
        //  the tree as our left side is from the left side of the tree.
        //
        TArea areaNew = dlgdLoad.areaPos();
        areaNew.i4Right(areaBrowser.i4Right() + areaBrowser.i4Left());
        dlgdLoad.areaPos(areaNew);

        // Now adjust the area of the buttons we are going to keep
        TDlgItem& dlgiAccept = dlgdLoad.dlgiFindById(kCQCTreeBrws::ridDlg_Browse_Accept);
        TDlgItem& dlgiCancel = dlgdLoad.dlgiFindById(kCQCTreeBrws::ridDlg_Browse_Cancel);
        TDlgItem& dlgiCopyPath = dlgdLoad.dlgiFindById(kCQCTreeBrws::ridDlg_Browse_CopyPath);

        TArea areaAccept = dlgiAccept.areaPos();
        TArea areaCancel = dlgiCancel.areaPos();
        TArea areaCopyPath = dlgiCopyPath.areaPos();

        // Remember the spacing between them
        const tCIDLib::TCard4 c4Space = tCIDLib::TCard4
        (
            areaCopyPath.i4Left() - areaCancel.i4Right()
        );

        // Put the copypath button right justified relative to the browser window
        areaCopyPath.RightJustifyIn(areaBrowser, kCIDLib::False);
        dlgiCopyPath.areaPos(areaCopyPath);

        // Now we want each successive one to be to the left of the previous with spacing
        areaCancel.i4Left(areaCopyPath.i4Left());
        areaCancel.AdjustOrg(-tCIDLib::TInt4(areaCancel.c4Width() + c4Space), 0);
        dlgiCancel.areaPos(areaCancel);

        areaAccept.i4Left(areaCancel.i4Left());
        areaAccept.AdjustOrg(-tCIDLib::TInt4(areaAccept.c4Width() + c4Space), 0);
        dlgiAccept.areaPos(areaAccept);

        // If we kept the relative path check box, left align it with the accept button
        if (m_pstrRelToPath)
        {
            TDlgItem& dlgiRelPath = dlgdLoad.dlgiFindById(kCQCTreeBrws::ridDlg_Browse_RelPath);
            TArea areaRelPath = dlgiRelPath.areaPos();
            areaRelPath.i4X(areaAccept.i4X() + 8);
            dlgiRelPath.areaPos(areaRelPath);
        }
    }

    //
    //  And now we can run it. If we ended with an accept, the store back the selected
    //  file.
    //
    if (c4RunDlg(wndOwner, dlgdLoad) == kCQCTreeBrws::ridDlg_Browse_Accept)
    {
        strSelected = m_strSelected;
        return kCIDLib::True;
    }

    return kCIDLib::False;
}


// ---------------------------------------------------------------------------
//  TRemBrwsDlg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TRemBrwsDlg::bCreated()
{
    //
    //  Call our parent first. We ignore the return in this case, because we know
    //  that we are going set a focus ourself below and will return False to indicate
    //  that we don't want any default focus setting.
    //
    TParent::bCreated();

    //
    //  The tree browsers aren't types supported by the resource editor, so a simple static
    //  color control was used. We'll get it's area, then destroy it and create a browser
    //  tree in its place.
    //
    {
        TCQCDataSrvBrws* prbrwsDlg = new TCQCDataSrvBrws(m_eDType);
        TJanitor<TCQCDataSrvBrws> janBrws(prbrwsDlg);

        TWindow* pwndClr = pwndChildById(kCQCTreeBrws::ridDlg_Browse_Tree);
        TArea areaTree = pwndClr->areaWnd();
        pwndClr->Destroy();
        delete pwndClr;

        // Set the underlying browser styles based on the flags we got
        tCQCTreeBrws::EBrwsStyles eBrwsStyles = tCQCTreeBrws::EBrwsStyles::None;
        if (tCIDLib::bAllBitsOn(m_eFlags, tCQCTreeBrws::EFSelFlags::ReadOnly))
            eBrwsStyles = tCQCTreeBrws::EBrwsStyles::ReadOnly;

        try
        {
            m_pwndTree = new TCQCTreeTypeBrowser;
            m_pwndTree->CreateTreeTypeBrowser
            (
                *this
                , m_eDType
                , kCQCTreeBrws::ridDlg_Browse_Tree
                , areaTree
                , tCIDLib::eOREnumBits
                  (
                    tCIDCtrls::EWndStyles::VisTabGroupChild, tCIDCtrls::EWndStyles::Border
                  )
                , eBrwsStyles
                , m_cuctxToUse
            );
            m_pwndTree->TakeFocus();

            //
            //  If image, then the tree just sizes to the bottom, else it's fills the whole
            //  area.
            //
            if (m_eDType == tCQCRemBrws::EDTypes::Image)
                m_pwndTree->eEdgeAnchor(tCIDCtrls::EEdgeAnchors::SizeBottom);
            else
                m_pwndTree->eEdgeAnchor(tCIDCtrls::EEdgeAnchors::SizeBottomRight);

            //
            //  Expand out the incoming path (or as much of it as is valid) and
            //  select it.
            //
            if (!m_strPath.bIsEmpty())
            {
                TString strGotTo;
                m_pwndTree->bExpandPath(m_strPath, strGotTo);
            }
        }

        catch(TError& errToCatch)
        {
            TExceptDlg dlgErr
            (
                *this
                , strWndText()
                , errToCatch.strErrText()
                , errToCatch
            );
            return kCIDLib::False;
        }
    }

    // Get typed pointers to the other widgets we always keep
    CastChildWnd(*this, kCQCTreeBrws::ridDlg_Browse_Accept, m_pwndAccept);
    CastChildWnd(*this, kCQCTreeBrws::ridDlg_Browse_Cancel, m_pwndCancel);
    CastChildWnd(*this, kCQCTreeBrws::ridDlg_Browse_CopyPath, m_pwndCopyPath);
    CastChildWnd(*this, kCQCTreeBrws::ridDlg_Browse_CurPath, m_pwndCurPath);

    // If not doing images, update the anchor for the current path
    if (m_eDType != tCQCRemBrws::EDTypes::Image)
        m_pwndCurPath->eEdgeAnchor(tCIDCtrls::EEdgeAnchors::MoveBottomSizeRight);

    // Register our handlers
    m_pwndAccept->pnothRegisterHandler(this, &TRemBrwsDlg::eClickHandler);
    m_pwndCancel->pnothRegisterHandler(this, &TRemBrwsDlg::eClickHandler);
    m_pwndCopyPath->pnothRegisterHandler(this, &TRemBrwsDlg::eClickHandler);
    m_pwndTree->pnothRegisterBrowserHandler(this, &TRemBrwsDlg::eBrwsHandler);

    //
    //  If we kept the relative path check box, then get it and initialize it. Don't
    //  need a click handler. We just look at it at the end.
    //
    if (m_pstrRelToPath)
    {
        CastChildWnd(*this, kCQCTreeBrws::ridDlg_Browse_RelPath, m_pwndRelPath);
        m_pwndRelPath->SetCheckedState(m_bOrgRelative);
    }

    //
    //  If doing images, then handle those controls.
    //
    if (m_eDType == tCQCRemBrws::EDTypes::Image)
    {
        CastChildWnd(*this, kCQCTreeBrws::ridDlg_Browse_CurPreview, m_pwndCurPreview);
        CastChildWnd(*this, kCQCTreeBrws::ridDlg_Browse_ImgPreview, m_pwndImgPreview);
        CastChildWnd(*this, kCQCTreeBrws::ridDlg_Browse_ShowPreview, m_pwndShowPreview);
        m_pwndShowPreview->pnothRegisterHandler(this, &TRemBrwsDlg::eClickHandler);
    }


    // Set the title we were given
    strWndText(m_strTitle);

    // Show the initially selected path
    TString strPath;
    if (m_pwndTree->bSelectedItem(strPath))
        m_pwndCurPath->strWndText(strPath);

    // Return false to indicate we set the focus ourself
    return kCIDLib::False;
}


// ---------------------------------------------------------------------------
//  TRemBrwsDlg: Private, non-virutal methods
// ---------------------------------------------------------------------------

//
//  Gets the current path, checks that it's valid and, if so stores it in m_strSelected.
//  If the caller wants a relative path that is what is stored.
//
tCIDLib::TBoolean TRemBrwsDlg::bStorePath()
{
    TString strPath;

    // If nothing is selected, then obviously not
    if (!m_pwndTree->bSelectedItem(strPath))
        return kCIDLib::False;

    // Get the base full hierarchical path for our data type
    const TString& strBasePath = facCQCRemBrws().strFlTypePref(m_eDType);

    // It has to at least be within the top limit
    if (!strPath.bStartsWithI(m_strTopLimit))
    {
        TString strMsg(TStrCat(L"Your selection must be beneath ", m_strTopLimit));
        TOkBox msgbLimit(strMsg);
        msgbLimit.ShowIt(*this);
        return kCIDLib::False;
    }

    //
    //  It has to start with the base path or can't be good. Though we only display
    //  stuff of the caller's requested type, they could be up above that in the
    //  hierarchy in the browser window.
    //
    if (!strPath.bStartsWithI(strBasePath))
        return kCIDLib::False;

    // If a scope and the caller only wants items, or vice versa, then bad
    if (m_pwndTree->bIsScope(strPath))
    {
        if (!tCIDLib::bAllBitsOn(m_eFlags, tCQCTreeBrws::EFSelFlags::SelectScopes))
            return kCIDLib::False;
    }
     else
    {
        if (!tCIDLib::bAllBitsOn(m_eFlags, tCQCTreeBrws::EFSelFlags::SelectItems))
            return kCIDLib::False;
    }

    //
    //  Get the type relative path from the full browser path. Remember the type relative
    //  path is what the caller considers a fully qualified path, so something that
    //  starts with /User or /System.
    //
    TString strRelPath;
    facCQCRemBrws().CreateRelPath(strPath, m_eDType, strRelPath);

    //
    //  Store the type relative path as the last path for this type, removing any file
    //  name part if a name was selected.
    //
    TString strLast = strRelPath;
    if (m_pwndTree->bIsScope(strPath) || facCQCRemBrws().bRemoveLastPathItem(strLast))
        s_colLastTypePaths[m_eDType] = strLast;

    //
    //  If they want further make it a path relative to a passed path, then get that, else
    //  keep the type relative one
    //
    if (m_pwndRelPath && m_pwndRelPath->bCheckedState())
        facCQCKit().bMakeRelResPath(*m_pstrRelToPath, strRelPath, m_strSelected);
    else
        m_strSelected = strRelPath;

    return kCIDLib::True;
}


tCIDCtrls::EEvResponses
TRemBrwsDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCTreeBrws::ridDlg_Browse_Accept)
    {
        // Get the path of the selected item
        if (bStorePath())
            EndDlg(kCQCTreeBrws::ridDlg_Browse_Accept);
    }
     else if (wnotEvent.widSource() == kCQCTreeBrws::ridDlg_Browse_Cancel)
    {
        EndDlg(kCQCTreeBrws::ridDlg_Browse_Cancel);
    }
     else if (wnotEvent.widSource() == kCQCTreeBrws::ridDlg_Browse_CopyPath)
    {
        // Store the appropriate path. If it's good, then copy to the clipboard
        if (bStorePath())
        {
            // Copy the text of the path to the clipboard
            TGUIClipboard gclipTmp(*this);
            gclipTmp.Clear();
            gclipTmp.StoreText(m_strSelected);
        }
    }
     else if (wnotEvent.widSource() == kCQCTreeBrws::ridDlg_Browse_ShowPreview)
    {
        //
        //  This button is only enabled if the selection is an item, which means it must
        //  be an image, since the only non-scopes would be images if we get this event.
        //  So we know the current selection is good.
        //
        TString strPath;
        if (!m_pwndTree->bSelectedItem(strPath))
        {
            CIDAssert2(L"There is no selected item in the tree");
        }

        // Get the relative path
        TString strRelPath;
        facCQCRemBrws().CreateRelPath(strPath, m_eDType, strRelPath);

        // And let's try to retrieve the image
        try
        {
            TDataSrvClient dsclImg;
            tCIDLib::TCard4 c4SerialNum = 0;
            tCIDLib::TEncodedTime enctLastChange;
            TPNGImage imgPreview;
            tCIDLib::TKVPFList colMeta;
            dsclImg.bReadImage
            (
                strRelPath
                , c4SerialNum
                , enctLastChange
                , imgPreview
                , colMeta
                , m_cuctxToUse.sectUser()
            );

            // And set it on the preview window
            TGraphWndDev gdevCompat(*this);
            TBitmap bmpPreview(gdevCompat, imgPreview);
            m_pwndImgPreview->SetBitmap
            (
                bmpPreview, imgPreview.c4TransClr(), imgPreview.bTransClr(), 0xFF
            );

            // Update current preview path
            m_pwndCurPreview->strWndText(strRelPath);
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);

            TExceptDlg dlgError
            (
                *this
                , strWndText()
                , L"The image could not be retrieved"
                , errToCatch
            );
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}


// We handle changes in the tree selection so we can display the path
tCIDCtrls::EEvResponses TRemBrwsDlg::eBrwsHandler(TTreeBrowseInfo& wnotEvent)
{
    // If the selection changed, update our current path display
    if (wnotEvent.eEvent() == tCQCTreeBrws::EEvents::SelChanged)
    {
        m_pwndCurPath->strWndText(wnotEvent.strPath());

        //
        //  If doing images, and they are on a scope, then disable the show preview button
        //  else enable it.
        //
        if (m_eDType == tCQCRemBrws::EDTypes::Image)
            m_pwndShowPreview->SetEnable(!m_pwndTree->bIsScope(wnotEvent.strPath()));
    }
     else if ((wnotEvent.eEvent() == tCQCTreeBrws::EEvents::View)
          ||  (wnotEvent.eEvent() == tCQCTreeBrws::EEvents::Edit))
    {
        //
        //  This means that an item was clicked on. All of them will support either viewing
        //  or editing. We just use these as an 'invoked' type event. If they invoked a
        //  valid path, then we are done.
        //
        if (bStorePath())
            EndDlg(kCQCTreeBrws::ridDlg_Browse_Accept);
    }
     else if (wnotEvent.eEvent() == tCQCTreeBrws::EEvents::CanRename)
    {
        // If the user has rename rights, allow it, else not
        wnotEvent.bFlag(m_cuctxToUse.eUserRole() >= tCQCKit::EUserRoles::PowerUser);
    }

    // Pass them on to the calling application if they provided a handler
    if (m_prbbcEvents)
        m_prbbcEvents->TreeBrowserEvent(wnotEvent);

    return tCIDCtrls::EEvResponses::Handled;
}



// ---------------------------------------------------------------------------
//  TRemBrwsDlg: Private, static data members
// ---------------------------------------------------------------------------
TVector<TString, tCQCRemBrws::EDTypes>  TRemBrwsDlg::s_colLastTypePaths;
