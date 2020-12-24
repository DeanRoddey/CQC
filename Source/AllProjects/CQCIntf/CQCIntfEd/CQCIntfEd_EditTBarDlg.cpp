//
// FILE NAME: CQCIntfEd_EditTBarDlg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/09/2016
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
//  This file implements a dialog that let's the user edit the buttons of a tool
//  bar widget.
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
#include    "CQCIntfEd_.hpp"
#include    "CQCIntfEd_EditTBarDlg_.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TEditTBarDlg, TDlgBox)



// ---------------------------------------------------------------------------
//   CLASS: TEditTBarDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TEditTBarDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TEditTBarDlg::TEditTBarDlg() :

    m_c4NextId(1)
    , m_pcivOwner(nullptr)
    , m_piwdgEdit(nullptr)
    , m_pwndActText(nullptr)
    , m_pwndAdd(nullptr)
    , m_pwndCancel(nullptr)
    , m_pwndClrImg(nullptr)
    , m_pwndDel(nullptr)
    , m_pwndDown(nullptr)
    , m_pwndEdAct(nullptr)
    , m_pwndList(nullptr)
    , m_pwndImg(nullptr)
    , m_pwndImgOp(nullptr)
    , m_pwndRevert(nullptr)
    , m_pwndSave(nullptr)
    , m_pwndSelImg(nullptr)
    , m_pwndUp(nullptr)
    , m_wstateEdit(L"Edit")
    , m_wstateEmpty(L"Empty")
{
}

TEditTBarDlg::~TEditTBarDlg()
{
}


// ---------------------------------------------------------------------------
//  TEditTBarDlg: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TEditTBarDlg::bIPETestCanEdit(  const   tCIDCtrls::TWndId
                                , const tCIDLib::TCard4     c4ColInd
                                ,       TAttrData&          adatFill
                                ,       tCIDLib::TBoolean&  bValueSet)
{
    // Set up the passed attribute data object with the info to edit
    if (c4ColInd == 0)
    {
        adatFill.Set(L"Name", L"Name", tCIDMData::EAttrTypes::String);

        // Set the current value as the initial contents to edit
        TString strVal;
        m_pwndList->QueryColText(c4IPERow(), c4ColInd, strVal);
        adatFill.SetString(strVal);
        bValueSet = kCIDLib::True;
    }
    return kCIDLib::True;
}


tCIDLib::TBoolean
TEditTBarDlg::bIPEValidate( const   TString&            strSrc
                            ,       TAttrData&          adatTar
                            , const TString&            strNewVal
                            ,       TString&            strErrMsg
                            ,       tCIDLib::TCard8&    c8UserId) const
{
    //
    //  Call the base mixin class to do basic validation. If that fails, then
    //  no need to look further.
    //
    if (!MIPEIntf::bIPEValidate(strSrc, adatTar, strNewVal, strErrMsg, c8UserId))
        return kCIDLib::False;

    if (adatTar.strId() == L"Name")
    {
        // We don't really need to do any validation per se.
        adatTar.SetString(strNewVal);
        return kCIDLib::True;
    }

    return kCIDLib::False;
}


tCIDLib::TVoid
TEditTBarDlg::IPEValChanged(const   tCIDCtrls::TWndId   widSrc
                            , const TString&            strSrc
                            , const TAttrData&          adatNew
                            , const TAttrData&          adatOld
                            , const tCIDLib::TCard8     )
{
    // Store away the new value as the text of the current button
    if (adatNew.strId() == L"Name")
    {
        m_piwdgEdit->tbbAt(c4IPERow()).strText(adatNew.strValue());
        m_piwdgEdit->Invalidate();
    }
}


// ---------------------------------------------------------------------------
//  TEditTBarDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TEditTBarDlg::bRun( const   TWindow&                wndOwner
                    ,       TCQCIntfToolbar* const  piwdgEdit
                    ,       TCQCIntfEdView* const   pcivOwner)
{
    // Store incoming stuff for later use
    m_pcivOwner = pcivOwner;
    m_piwdgEdit = piwdgEdit;
    m_iwdgOrg = *piwdgEdit;

    const tCIDLib::TCard4 c4Res = c4RunDlg
    (
        wndOwner, facCQCIntfEd(), kCQCIntfEd::ridDlg_EditTBar
    );

    // If they saved we are done
    if (c4Res == kCQCIntfEd::ridDlg_EditTBar_Save)
        return kCIDLib::True;

    // Else, copy back the original to overwrite any changes
    *piwdgEdit = m_iwdgOrg;
    piwdgEdit->Invalidate();

    return kCIDLib::False;
}


// ---------------------------------------------------------------------------
//  TEditTBarDlg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TEditTBarDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRet = TParent::bCreated();

    // First get typed pointers to the widget we always keep
    CastChildWnd(*this, kCQCIntfEd::ridDlg_EditTBar_ActText, m_pwndActText);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_EditTBar_Add, m_pwndAdd);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_EditTBar_Cancel, m_pwndCancel);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_EditTBar_ClrImage, m_pwndClrImg);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_EditTBar_Del, m_pwndDel);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_EditTBar_MoveDn, m_pwndDown);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_EditTBar_EditAct, m_pwndEdAct);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_EditTBar_List, m_pwndList);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_EditTBar_Img, m_pwndImg);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_EditTBar_Opacity, m_pwndImgOp);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_EditTBar_Revert, m_pwndRevert);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_EditTBar_Save, m_pwndSave);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_EditTBar_SelImg, m_pwndSelImg);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_EditTBar_MoveUp, m_pwndUp);

    // Register our handlers
    m_pwndAdd->pnothRegisterHandler(this, &TEditTBarDlg::eClickHandler);
    m_pwndCancel->pnothRegisterHandler(this, &TEditTBarDlg::eClickHandler);
    m_pwndClrImg->pnothRegisterHandler(this, &TEditTBarDlg::eClickHandler);
    m_pwndDel->pnothRegisterHandler(this, &TEditTBarDlg::eClickHandler);
    m_pwndDown->pnothRegisterHandler(this, &TEditTBarDlg::eClickHandler);
    m_pwndEdAct->pnothRegisterHandler(this, &TEditTBarDlg::eClickHandler);
    m_pwndImgOp->pnothRegisterHandler(this, &TEditTBarDlg::eSliderHandler);
    m_pwndList->pnothRegisterHandler(this, &TEditTBarDlg::eListHandler);
    m_pwndRevert->pnothRegisterHandler(this, &TEditTBarDlg::eClickHandler);
    m_pwndSave->pnothRegisterHandler(this, &TEditTBarDlg::eClickHandler);
    m_pwndSelImg->pnothRegisterHandler(this, &TEditTBarDlg::eClickHandler);
    m_pwndUp->pnothRegisterHandler(this, &TEditTBarDlg::eClickHandler);

    // Set up our window states
    m_wstateEdit.AddWndEnable(kCQCIntfEd::ridDlg_EditTBar_ClrImage);
    m_wstateEdit.AddWndEnable(kCQCIntfEd::ridDlg_EditTBar_Del);
    m_wstateEdit.AddWndEnable(kCQCIntfEd::ridDlg_EditTBar_Opacity);
    m_wstateEdit.AddWndEnable(kCQCIntfEd::ridDlg_EditTBar_MoveDn);
    m_wstateEdit.AddWndEnable(kCQCIntfEd::ridDlg_EditTBar_EditAct);
    m_wstateEdit.AddWndEnable(kCQCIntfEd::ridDlg_EditTBar_SelImg);
    m_wstateEdit.AddWndEnable(kCQCIntfEd::ridDlg_EditTBar_MoveUp);

    m_wstateEmpty.AddWndDisable(kCQCIntfEd::ridDlg_EditTBar_ClrImage);
    m_wstateEmpty.AddWndDisable(kCQCIntfEd::ridDlg_EditTBar_Del);
    m_wstateEmpty.AddWndDisable(kCQCIntfEd::ridDlg_EditTBar_Opacity);
    m_wstateEmpty.AddWndDisable(kCQCIntfEd::ridDlg_EditTBar_MoveDn);
    m_wstateEmpty.AddWndDisable(kCQCIntfEd::ridDlg_EditTBar_EditAct);
    m_wstateEmpty.AddWndDisable(kCQCIntfEd::ridDlg_EditTBar_SelImg);
    m_wstateEmpty.AddWndDisable(kCQCIntfEd::ridDlg_EditTBar_MoveUp);
    m_wstateEmpty.SetFocus(kCQCIntfEd::ridDlg_EditTBar_Add, kCIDLib::True);

    // The column titles aren't shown, but we need to set a placeholder
    tCIDLib::TStrList colVals(1);
    colVals.objAdd(L"Name");
    m_pwndList->SetColumns(colVals);

    // Set the range on the opacity slider
    m_pwndImgOp->SetRange(0, 255);

    // Load up any incoming buttons
    const tCIDLib::TCard4 c4Count = m_piwdgEdit->c4ButtonCnt();;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TCQCIntfToolbar::TTBButton& tbbCur = m_piwdgEdit->tbbAt(c4Index);
        colVals[0] = tbbCur.strText();
        m_pwndList->c4AddItem(colVals, m_c4NextId++);
    }

    // Install us as the auto-in place editor on the list window
    m_pwndList->SetAutoIPE(this);

    //
    //  Select the 0th one (f available) to get its data loaded and set the
    //  appropriate window state.
    //
    if (c4Count)
    {
        m_wstateEdit.ApplyState(*this);
        m_pwndList->SelectByIndex(0);
    }
     else
    {
        m_wstateEmpty.ApplyState(*this);
    }

    return bRet;
}


// ---------------------------------------------------------------------------
//  TEditTBarDlg: Private, non-virutal methods
// ---------------------------------------------------------------------------
tCIDCtrls::EEvResponses TEditTBarDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCIntfEd::ridDlg_EditTBar_Add)
    {
        // If the list is currently empty, then we need to move to the edit state
        if (!m_piwdgEdit->c4ButtonCnt())
            m_wstateEdit.ApplyState(*this);

        // Add the button to our list first so it's there when the list index changes
        const TString& strDefText(L"New Button");
        const tCIDLib::TCard4 c4At = m_piwdgEdit->c4ButtonCnt();
        m_piwdgEdit->AddButton(TCQCIntfToolbar::TTBButton(strDefText, m_piwdgEdit));
        m_piwdgEdit->Invalidate();

        tCIDLib::TStrList colVals(1);
        colVals.objAdd(strDefText);
        m_pwndList->c4AddItem(colVals, m_c4NextId++, c4At);

        // Select this new one
        m_pwndList->SelectByIndex(c4At);
    }
     else if (wnotEvent.widSource() == kCQCIntfEd::ridDlg_EditTBar_Cancel)
    {
        //
        //  Note we don't check for changes. We were editing the widget directly. If they
        //  cancel, the dialog entry point method will put back the original content
        //  again.
        //
        EndDlg(kCQCIntfEd::ridDlg_EditTBar_Cancel);
    }
     else if (wnotEvent.widSource() == kCQCIntfEd::ridDlg_EditTBar_ClrImage)
    {
        const tCIDLib::TCard4 c4Index = m_pwndList->c4CurItem();
        if (c4Index != kCIDLib::c4MaxCard)
        {
            m_piwdgEdit->tbbAt(c4Index).ClearImage();
            m_piwdgEdit->Invalidate();

            // Clear the preview window as well
            m_pwndImg->Reset();
        }
    }
     else if (wnotEvent.widSource() == kCQCIntfEd::ridDlg_EditTBar_Del)
    {
        // Delete the selected button
        const tCIDLib::TCard4 c4Index = m_pwndList->c4CurItem();
        if (c4Index != kCIDLib::c4MaxCard)
        {
            // Delete it first from the widget
            m_piwdgEdit->RemoveButtonAt(c4Index);

            // And now from the list
            m_pwndList->RemoveAt(c4Index);

            // Redraw the widget area to pick up this change
            m_piwdgEdit->Invalidate();
        }
    }
     else if (wnotEvent.widSource() == kCQCIntfEd::ridDlg_EditTBar_EditAct)
    {
        //
        //  Invoke the command editing dialog on the selected button.
        //
        const tCIDLib::TCard4 c4Index = m_pwndList->c4CurItem();
        if (c4Index != kCIDLib::c4MaxCard)
        {
            TCQCIntfToolbar::TTBButton& tbbCur = m_piwdgEdit->tbbAt(c4Index);

            //
            //  We need to find all of the targetable widgets to pass in as action targets.
            //  We also add our owning view.
            //
            tCQCKit::TCmdTarList colExtra(tCIDLib::EAdoptOpts::NoAdopt);
            m_pcivOwner->iwdgBaseTmpl().FindAllTargets(colExtra, kCIDLib::False);
            colExtra.Add(m_pcivOwner);

            const tCIDLib::TBoolean bRes = facCQCIGKit().bEditAction
            (
                *this
                , tbbCur
                , kCIDLib::True
                , m_pcivOwner->cuctxToUse()
                , &colExtra
                , &m_pcivOwner->iwdgBaseTmpl().strTemplateName()
            );

            if (bRes)
                ShowAction(tbbCur);
        }
    }
     else if ((wnotEvent.widSource() == kCQCIntfEd::ridDlg_EditTBar_MoveDn)
          ||  (wnotEvent.widSource() == kCQCIntfEd::ridDlg_EditTBar_MoveUp))
    {
        //
        //  If it's not already at the end, then swap it with the one after
        //  it. The list window swap will redraw the visible items so it will
        //  udpate if a visible one was one of the swapped ones.
        //
        const tCIDLib::TCard4 c4Index = m_pwndList->c4CurItem();
        const tCIDLib::TCard4 c4Count = m_pwndList->c4ItemCount();

        // Note that this is backwards, up means lower indices below!
        const tCIDLib::TBoolean bUp
        (
            wnotEvent.widSource() == kCQCIntfEd::ridDlg_EditTBar_MoveUp
        );

        // Make sure there's room in the selected direction
        if ((!bUp && (c4Index + 1 < c4Count)) || (bUp && c4Index))
        {
            // There is so figure out the other index
            tCIDLib::TCard4 c4Other = c4Index;
            if (bUp)
                c4Other--;
            else
                c4Other++;

            //
            //  And swap first the buttons themselves then the list window items. Tell the
            //  window to follow the selected item, so it keeps the moved one selected.
            //
            m_piwdgEdit->SwapButtons(c4Index, c4Other);
            m_pwndList->SwapRows(c4Index, c4Other, kCIDLib::True);

            // Force a redraw to show the new order
            m_piwdgEdit->Invalidate();
        }
    }
     else if (wnotEvent.widSource() == kCQCIntfEd::ridDlg_EditTBar_Revert)
    {
        // Copy back over the original content
        *m_piwdgEdit = m_iwdgOrg;

        // And reload the buttons
        tCIDLib::TStrList colVals(1);
        colVals.objAdd(TString::strEmpty());
        m_pwndList->RemoveAll();
        const tCIDLib::TCard4 c4Count = m_piwdgEdit->c4ButtonCnt();;
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const TCQCIntfToolbar::TTBButton& tbbCur = m_piwdgEdit->tbbAt(c4Index);
            colVals[0] = tbbCur.strText();
            m_pwndList->c4AddItem(colVals, m_c4NextId++);
        }

        // Force a redraw of the area under the widget
        m_piwdgEdit->Invalidate();

        // If any buttons select the 0th one
        if (m_pwndList->c4ItemCount())
        {
            m_pwndList->SelectByIndex(0);
            m_wstateEdit.ApplyState(*this);
        }
    }
     else if (wnotEvent.widSource() == kCQCIntfEd::ridDlg_EditTBar_Save)
    {
        EndDlg(kCQCIntfEd::ridDlg_EditTBar_Save);
    }
     else if (wnotEvent.widSource() == kCQCIntfEd::ridDlg_EditTBar_SelImg)
    {
        TCQCIntfToolbar::TTBButton& tbbCur = m_piwdgEdit->tbbAt(m_pwndList->c4CurItem());

        TString strNewSel;
        const tCIDLib::TBoolean bSel = facCQCTreeBrws().bSelectFile
        (
            *this
            , L"Select a Button Image"
            , tCQCRemBrws::EDTypes::Image
            , tbbCur.strImagePath()
            , m_pcivOwner->cuctxToUse()
            , tCQCTreeBrws::EFSelFlags::SelectItems
            , strNewSel
        );

        if (bSel)
        {
            // They selected something, so let's update the button
            ShowPreview(strNewSel, tbbCur.c1Opacity());

            //
            //  Set the path as the new image on the target button, and
            //  ask him to load it up.
            //
            tbbCur.strImagePath(strNewSel);
            tbbCur.LoadImage(*m_pcivOwner);
            m_piwdgEdit->Invalidate();
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}


//
//  We have to watch for changes in selection in the button list box and update the
//  displayed info.
//
tCIDCtrls::EEvResponses TEditTBarDlg::eListHandler(TListChangeInfo& wnotEvent)
{
    if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::SelChanged)
    {
        const TCQCIntfToolbar::TTBButton& tbbNew = m_piwdgEdit->tbbAt(wnotEvent.c4Index());

        // Update the image preview
        if (tbbNew.strImagePath().bIsEmpty())
            m_pwndImg->Reset();
        else
            ShowPreview(tbbNew.strImagePath(), tbbNew.c1Opacity());

        // If we have an action, format it out and display it
        ShowAction(tbbNew);

        // Set the opacity slider
        m_pwndImgOp->SetValue(tCIDLib::TInt4(tbbNew.c1Opacity()));
    }
     else if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::Cleared)
    {
        //
        //  They've deleted the last one, so clear out all of the controls and
        //  disable them.
        //
        m_pwndActText->ClearText();
        m_pwndImg->Reset();
        m_pwndImgOp->SetValue(0xFF);
        m_wstateEmpty.ApplyState(*this);
    }

    return tCIDCtrls::EEvResponses::Handled;
}


tCIDCtrls::EEvResponses TEditTBarDlg::eSliderHandler(TSliderChangeInfo& wnotEvent)
{
    //
    //  We only have one, which is the image opacity. It should only be enabled if
    //  there is actually an image for the current button. So we just update the
    //  opacity of the current button and force a redraw.
    //
    if ((wnotEvent.eEvent() == tCIDCtrls::ESldrEvents::Change)
    ||  (wnotEvent.eEvent() == tCIDCtrls::ESldrEvents::EndTrack))
    {
        TCQCIntfToolbar::TTBButton& tbbCur = m_piwdgEdit->tbbAt(m_pwndList->c4CurItem());
        tbbCur.c1Opacity(tCIDLib::TCard1(m_pwndImgOp->i4CurValue()));
        m_piwdgEdit->Invalidate();
    }
    return tCIDCtrls::EEvResponses::Handled;
}


//
//  If the passed button has an action, format it out and display it in the
//  action text control.
//
tCIDLib::TVoid TEditTBarDlg::ShowAction(const TCQCIntfToolbar::TTBButton& tbbSrc)
{
    const tCIDLib::TCard4 c4EvCount = tbbSrc.c4EventCount();
    if (c4EvCount)
    {
        TTextStringOutStream strmFmt(2048UL);
        for (tCIDLib::TCard4 c4EvIndex = 0; c4EvIndex < c4EvCount; c4EvIndex++)
        {
            if (c4EvIndex)
                strmFmt << kCIDLib::NewLn;

            // Put out an item for the event name
            const TCQCActEvInfo& caeiCur = tbbSrc.caeiEventAt(c4EvIndex);
            strmFmt << L"[" << caeiCur.m_strName
                    << L"]" << kCIDLib::NewLn;

            //
            //  Ask the source to format this event's opcodes. We tell it to
            //  start at indent level 1, so that the opcodes will be indented
            //  inwards under each event label.
            //
            tbbSrc.FormatOpsForEvent(caeiCur.m_strEventId, strmFmt, 1);
        }

        // Output the accumulated text
        strmFmt.Flush();
        m_pwndActText->strWndText(strmFmt.strData());
    }
     else
    {
        m_pwndActText->ClearText();
    }
}


//
//  Called when a new button is selected, to show that button's image (if any) in
//  the preview control.
//
tCIDLib::TVoid
TEditTBarDlg::ShowPreview(  const   TString&        strImgPath
                            , const tCIDLib::TCard1 c1Opacity)
{
    if (strImgPath.bIsEmpty())
    {
        m_pwndImg->Reset();
        return;
    }

    try
    {
        // We need a data server client to get the image
        TDataSrvClient dsclSrv;

        //
        //  Make sure the image exists. If not, then clear out the image
        //  settings so that we know we don't have an image anymore. Else,
        //  we can get the thumb and set it on our preview windwo.
        //
        if (dsclSrv.bFileExists(strImgPath, tCQCRemBrws::EDTypes::Image))
        {
            TPNGImage               imgLoad;
            tCIDLib::TCard4         c4SerNum = 0;
            tCIDLib::TEncodedTime   enctLastChange;
            tCIDLib::TKVPFList      colXMeta;
            const tCIDLib::TBoolean bGotIt = dsclSrv.bReadImage
            (
                strImgPath
                , c4SerNum
                , enctLastChange
                , imgLoad
                , colXMeta
                , m_pcivOwner->cuctxToUse().sectUser()
            );

            // Create a bitmap from the loaded content and set it on the preview
            if (bGotIt)
            {
                TBitmap bmpImg(m_pcivOwner->gdevCompat(), imgLoad);
                m_pwndImg->SetBitmap(bmpImg, c1Opacity);
            }
             else
            {
                m_pwndImg->Reset();
            }
        }
         else
        {
            m_pwndImg->Reset();
        }
    }

    catch(TError& errToCatch)
    {
        if (facCQCIntfEd().bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
        m_pwndImg->Reset();
    }
}

