//
// FILE NAME: CQCIntfEd_Tools.cpp
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
//  This file implements the simple tools windows that go into the right side tabbed
//  window of the editor.
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
#include    "CQCIntfEd_Tools_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TIntfAttrEdTab,TTabWindow)
RTTIDecls(TIntfToolsTab,TTabWindow)
RTTIDecls(TIntfFindWdgTab,TTabWindow)



// ---------------------------------------------------------------------------
//  CLASS: TIntfAttrEdTab
// PREFIX: wnd
// ---------------------------------------------------------------------------
TIntfAttrEdTab::TIntfAttrEdTab() :

    TTabWindow(L"IntfEdAttrEd", TString::strEmpty(), kCIDLib::False, kCIDLib::True)
    , m_pwndAttrEd(nullptr)
{
}

TIntfAttrEdTab::~TIntfAttrEdTab()
{
}


// ---------------------------------------------------------------------------
//  TIntfAttrEdTab: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TIntfAttrEdTab::AreaChanged(const   TArea&                  areaPrev
                            , const TArea&                  areaNew
                            , const tCIDCtrls::EPosStates   ePosState
                            , const tCIDLib::TBoolean       bOrgChanged
                            , const tCIDLib::TBoolean       bSizeChanged
                            , const tCIDLib::TBoolean       bStateChanged)
{
    TParent::AreaChanged(areaPrev, areaNew, ePosState, bOrgChanged, bSizeChanged, bStateChanged);

    // Keep the editor sized to our client area
    if (bSizeChanged && (ePosState != tCIDCtrls::EPosStates::Minimized) && m_pwndAttrEd)
        m_pwndAttrEd->SetSize(areaClient().szArea(), kCIDLib::True);
}

tCIDLib::TBoolean TIntfAttrEdTab::bCreated()
{
    TParent::bCreated();

    return kCIDLib::True;
}




// ---------------------------------------------------------------------------
//  CLASS: TIntfFindWdgTab
// PREFIX: wnd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TIntfFindWdgTab: Constructors and Destructor
// ---------------------------------------------------------------------------
TIntfFindWdgTab::TIntfFindWdgTab(TCQCIntfEdView* const pcivEdit) :

    TTabWindow(L"IntfFindWdgTab", TString::strEmpty(), kCIDLib::False, kCIDLib::True)
    , m_pcivEdit(pcivEdit)
    , m_pwndClient(nullptr)
    , m_pwndFind(nullptr)
    , m_pwndId(nullptr)
    , m_pwndIdREx(nullptr)
    , m_pwndImage(nullptr)
    , m_pwndImageREx(nullptr)
    , m_pwndMatches(nullptr)
    , m_pwndSelImage(nullptr)
    , m_pwndText(nullptr)
    , m_pwndTextREx(nullptr)
    , m_pwndTypes(nullptr)
{
}

TIntfFindWdgTab::~TIntfFindWdgTab()
{
}

// ---------------------------------------------------------------------------
//  TIntfFindWdgTab: Protected, inherited methods
// ---------------------------------------------------------------------------

// We just keep our generic client window sized to fit us
tCIDLib::TVoid
TIntfFindWdgTab::AreaChanged(const  TArea&                  areaPrev
                            , const TArea&                  areaNew
                            , const tCIDCtrls::EPosStates   ePosState
                            , const tCIDLib::TBoolean       bOrgChanged
                            , const tCIDLib::TBoolean       bSizeChanged
                            , const tCIDLib::TBoolean       bStateChanged)
{
    TParent::AreaChanged(areaPrev, areaNew, ePosState, bOrgChanged, bSizeChanged, bStateChanged);

    if (bSizeChanged && (ePosState != tCIDCtrls::EPosStates::Minimized) && m_pwndClient)
        m_pwndClient->SetSize(areaNew.szArea(), kCIDLib::True);
}


tCIDLib::TBoolean TIntfFindWdgTab::bCreated()
{
    TParent::bCreated();

    // Load the dialog description
    TDlgDesc dlgdChildren;
    facCQCIntfEd().bCreateDlgDesc(kCQCIntfEd::ridDlg_FindWidget, dlgdChildren);

    //
    //  Create our generic window which will be sized to fit the tab. But initially set
    //  it to the size of the dialog content. So, when we create teh controls below
    //  they will have the correct initial relationship to this window.
    //
    m_pwndClient = new TGenericWnd();
    m_pwndClient->CreateGenWnd
    (
        *this
        , dlgdChildren.areaPos()
        , tCIDCtrls::EWndStyles::ClippingVisChild
        , tCIDCtrls::EExWndStyles::ControlParent
        , kCIDCtrls::widFirstCtrl
    );

    //
    //  Now create the children. No need to do an initial sizing since we sized our
    //  own window initially to fit. Tell it to limit minimum size to the original
    //  dialog content size.
    //
    tCIDLib::TCard4 c4InitFocus;
    m_pwndClient->PopulateFromDlg
    (
        dlgdChildren, c4InitFocus, kCIDLib::False, kCIDLib::True
    );

    // Get refs to the ones we need
    CastChildWnd(*m_pwndClient, kCQCIntfEd::ridDlg_FindWidget_Find, m_pwndFind);
    CastChildWnd(*m_pwndClient, kCQCIntfEd::ridDlg_FindWidget_Id, m_pwndId);
    CastChildWnd(*m_pwndClient, kCQCIntfEd::ridDlg_FindWidget_IdREx, m_pwndIdREx);
    CastChildWnd(*m_pwndClient, kCQCIntfEd::ridDlg_FindWidget_Image, m_pwndImage);
    CastChildWnd(*m_pwndClient, kCQCIntfEd::ridDlg_FindWidget_ImageREx, m_pwndImageREx);
    CastChildWnd(*m_pwndClient, kCQCIntfEd::ridDlg_FindWidget_Matches, m_pwndMatches);
    CastChildWnd(*m_pwndClient, kCQCIntfEd::ridDlg_FindWidget_SelImage, m_pwndSelImage);
    CastChildWnd(*m_pwndClient, kCQCIntfEd::ridDlg_FindWidget_Text, m_pwndText);
    CastChildWnd(*m_pwndClient, kCQCIntfEd::ridDlg_FindWidget_TextREx, m_pwndTextREx);
    CastChildWnd(*m_pwndClient, kCQCIntfEd::ridDlg_FindWidget_Types, m_pwndTypes);

    //
    //  Store the original area of the matches list, and the original amounts it was
    //  offset from the bottom and right.
    //
    const TArea& areaOrgCont = dlgdChildren.areaPos();
    m_areaOrgList = m_pwndMatches->areaWnd();
    m_pntOrgListOfs.Set
    (
        areaOrgCont.i4Right() - m_areaOrgList.i4Right()
        , areaOrgCont.i4Bottom() - m_areaOrgList.i4Bottom()
    );


    //
    //  Load up the types list. We load up a list of them, sort it, then load that
    //  into the list window.
    //
    TFacCQCIntfEng::TTypeList::TCursor cursList = facCQCIntfEng().cursTypes();
    tCIDLib::TStrList colToLoad(128);
    if (cursList.bReset())
    {
        do
        {
            const TKeyValuePair& kvalCur = cursList.objRCur();

            // It includes spacers and category headers, so skip those
            if (!kvalCur.strValue().bIsEmpty() && !kvalCur.strKey().bIsEmpty())
                colToLoad.objAdd(kvalCur.strValue());
        }   while (cursList.bNext());
    }

    colToLoad.Sort(TString::eCompI);

    // Insert an Any entry at the start
    colToLoad.InsertAt(L"Any", 0);

    // And load them to the list
    m_pwndTypes->LoadList(colToLoad);

    // Select the initial any value
    m_pwndTypes->SelectByIndex(0);

    // Set up our two columns on the list window
    tCIDLib::TStrList colCols(2);
    colCols.objAdd(L"Widget Id");
    colCols.objAdd(L"Widget Type");
    m_pwndMatches->SetColumns(colCols);

    // Set up notification handlers
    m_pwndFind->pnothRegisterHandler(this, &TIntfFindWdgTab::eClickHandler);
    m_pwndSelImage->pnothRegisterHandler(this, &TIntfFindWdgTab::eClickHandler);
    m_pwndMatches->pnothRegisterHandler(this, &TIntfFindWdgTab::eLBHandler);

    return kCIDLib::True;
}


//
//  Because we have a main client window and the controls inside that, we override this
//  in order to make sure we get focus set upon the initial activation of this tab. The
//  code that calls us (from the tabbed window control) doesn't recursively look for a
//  focus candidate.
//
tCIDLib::TVoid TIntfFindWdgTab::TabActivationChange(const tCIDLib::TBoolean bGetting)
{
    //
    //  If getting the focus and we don't contain the focus (or the focus is us, which is
    //  where it goes if the calling code can't find anywhere else to put it), then set
    //  it to our id field.
    //
    if (bGetting && (!bContainsFocus() || bHasFocus()))
        m_pwndId->TakeFocus();
}


// ---------------------------------------------------------------------------
//  TIntfFindWdgTab: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDCtrls::EEvResponses TIntfFindWdgTab::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCIntfEd::ridDlg_FindWidget_Find)
    {
        // The view pointer must have been set on us
        if (!m_pcivEdit)
        {
            TErrBox msgbErr(L"The editor window view has not been set");
            msgbErr.ShowIt(*this);
            return tCIDCtrls::EEvResponses::Handled;
        }

        // Clear the list first
        m_pwndMatches->RemoveAll();

        //
        //  Get the criteria out. We may have to set up regular expressions. For any
        //  that are empty we assume that means don't use this criteria and force the
        //  regular expression flag to be off no matter what.
        //
        const TString& strId = m_pwndId->strWndText();
        const TString& strImage = m_pwndImage->strWndText();
        const TString& strText = m_pwndText->strWndText();

        tCIDLib::TBoolean bIdREx = m_pwndIdREx->bCheckedState() && !strId.bIsEmpty();
        tCIDLib::TBoolean bImageREx = m_pwndImageREx->bCheckedState() && !strImage.bIsEmpty();
        tCIDLib::TBoolean bTextREx = m_pwndTextREx->bCheckedState() && !strText.bIsEmpty();

        // Set up the register expressions as needed
        TRegEx regxId;
        TRegEx regxImage;
        TRegEx regxText;

        if (bIdREx)
        {
            try
            {
                regxId.SetExpression(strId);
            }

            catch(TError& errToCatch)
            {
                TString strMsg(L"Invalid id regular expression. Reason=", 128UL);
                strMsg.Append(errToCatch.strErrText());
                TErrBox msgbErr(strMsg);
                msgbErr.ShowIt(*m_pwndClient);
                return tCIDCtrls::EEvResponses::Handled;
            }
        }

        if (bImageREx)
        {
            try
            {
                regxImage.SetExpression(strImage);
            }

            catch(TError& errToCatch)
            {
                TString strMsg(L"Invalid image regular expression. Reason=", 128UL);
                strMsg.Append(errToCatch.strErrText());
                TErrBox msgbErr(strMsg);
                msgbErr.ShowIt(*m_pwndClient);
                return tCIDCtrls::EEvResponses::Handled;
            }
        }

        if (bTextREx)
        {
            try
            {
                regxText.SetExpression(strText);
            }

            catch(TError& errToCatch)
            {
                TString strMsg(L"Invalid text regular expression. Reason=", 128UL);
                strMsg.Append(errToCatch.strErrText());
                TErrBox msgbErr(strMsg);
                msgbErr.ShowIt(*m_pwndClient);
                return tCIDCtrls::EEvResponses::Handled;
            }
        }

        //
        //  Get the class. We have to get the user visible type name out and use that
        //  to search the list and translate to the actual class type. If it's the 0th
        //  entry, set it to empty since we'll match any.
        //
        tCIDLib::TStrList       colCols(2);
        colCols.objAdd(TString::strEmpty());
        colCols.objAdd(TString::strEmpty());
        tCIDLib::TStrHashSet    colImages(109, TStringKeyOps(kCIDLib::False));
        tCIDLib::TStrHashSet    colScopes(109, TStringKeyOps(kCIDLib::False));
        TString                 strType;
        if (m_pwndTypes->c4CurItem())
        {
            TString strUserType;
            m_pwndTypes->bQueryCurVal(strUserType);

            TFacCQCIntfEng::TTypeList::TCursor cursList = facCQCIntfEng().cursTypes();
            if (cursList.bReset())
            {
                do
                {
                    const TKeyValuePair& kvalCur = cursList.objRCur();
                    if (kvalCur.strValue().bCompareI(strUserType))
                    {
                        strType = kvalCur.strKey();
                        break;
                    }
                }   while (cursList.bNext());
            }
        }

        // Now loop through the widgets and check each one
        const TCQCIntfTemplate& iwdgSearch = m_pcivEdit->iwdgBaseTmpl();
        const tCIDLib::TCard4 c4Count = iwdgSearch.c4ChildCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const TCQCIntfWidget& iwdgCur = iwdgSearch.iwdgAt(c4Index);

            if (!strType.bIsEmpty())
            {
                // If no match, skip it
                if (!iwdgCur.clsIsA().strClassName().bCompareI(strType))
                    continue;
            }

            if (!strId.bIsEmpty())
            {
                if (bIdREx)
                {
                    if (!regxId.bFullyMatches(iwdgCur.strWidgetId()))
                        continue;
                }
                 else
                {
                    if (!iwdgCur.strWidgetId().bCompareI(strId))
                        continue;
                }
            }

            if (!strText.bIsEmpty())
            {
                if (bTextREx)
                {
                    if (!regxText.bFullyMatches(iwdgCur.strCaption()))
                        continue;
                }
                 else
                {
                    if (!iwdgCur.strCaption().bCompareI(strText))
                        continue;
                }
            }

            //
            //  This one is bit heavier. We have to ask the widget for a list of the
            //  images it references and check them all. We also get image scopes which
            //  is not used on a lot of them, but some. We check the images first, then
            //  the scopes so as to get the longest possible match first, though at this
            //  time no widget would likely return something in both lists.
            //
            if (!strImage.bIsEmpty())
            {
                //
                //  This is a recursive call so it cannot clean out the lists. We have to
                //  to do that first of they will just accumulate.
                //
                colImages.RemoveAll();
                colScopes.RemoveAll();
                iwdgCur.QueryReferencedImgs(colImages, colScopes, kCIDLib::True, kCIDLib::True);
                tCIDLib::TBoolean bMatched = kCIDLib::False;

                tCIDLib::TStrHashSet::TCursor cursImgs(&colImages);
                if(cursImgs.bReset())
                {
                    do
                    {
                        if (bImageREx)
                        {
                            if (regxImage.bFullyMatches(cursImgs.objRCur()))
                            {
                                bMatched = kCIDLib::True;
                                break;
                            }
                        }
                         else
                        {
                            if (cursImgs.objRCur().bCompareI(strImage))
                            {
                                bMatched = kCIDLib::True;
                                break;
                            }
                        }
                    }   while (cursImgs.bNext());
                }

                // If no match yet, see if any scopes match
                if (!bMatched)
                {
                    tCIDLib::TStrHashSet::TCursor cursScopes(&colScopes);
                    if(cursScopes.bReset())
                    {
                        do
                        {
                            if (bImageREx)
                            {
                                if (regxImage.bFullyMatches(cursScopes.objRCur()))
                                {
                                    bMatched = kCIDLib::True;
                                    break;
                                }
                            }
                             else
                            {
                                if (cursScopes.objRCur().bCompareI(strImage))
                                {
                                    bMatched = kCIDLib::True;
                                    break;
                                }
                            }
                        }   while (cursScopes.bNext());
                    }
                }

                // If no match, skip this one
                if (!bMatched)
                    continue;
            }

            //
            //  OK, this is a keeper, so add it to the list. We want to store the design
            //  time id as well, so that we can get back to this widget when the user selects
            //  it. But it's a Card8 and the item ids we can store on a list window item are
            //  Card4. So we store the low word. The high word is always the same for us since
            //  it is the template part of the id, and we are always working with the same
            //  template.
            //
            colCols[0] = iwdgCur.strWidgetId();
            colCols[1] = iwdgCur.strTypeName();
            m_pwndMatches->c4AddItem(colCols, tCIDLib::TCard4(iwdgCur.c8DesId()));
        }

        // Remember the template specific part of the ids if we stored any
        if (c4Count)
            m_c8TmplId = iwdgSearch.iwdgAt(0).c8DesId() & 0xFFFFFFFF00000000;

        //
        //  Note we don't select one, since we don't want to affect the current selection
        //  in the editor. We'll only do that if they make a selection manually.
        //
    }
     else if (wnotEvent.widSource() == kCQCIntfEd::ridDlg_FindWidget_SelImage)
    {
        // Invoke the image remote selection dialog for images
        TString strSelected;
        const tCIDLib::TBoolean bRes = facCQCTreeBrws().bSelectFile
        (
            *m_pwndClient
            , L"Select an Image"
            , tCQCRemBrws::EDTypes::Image
            , TString::strEmpty()
            , m_pcivEdit->cuctxToUse()
            , tCIDLib::eOREnumBits
              (
                tCQCTreeBrws::EFSelFlags::SelectItems
                , tCQCTreeBrws::EFSelFlags::SelectScopes
              )
            , strSelected
        );

        if (bRes)
            m_pwndImage->strWndText(strSelected);
    }
    return tCIDCtrls::EEvResponses::Handled;
}


//
//  If we get a selection in the list, we will tell the editor window to select that
//  widget. If it's no longer a valid designer id, he will just ignore that.
//
tCIDCtrls::EEvResponses TIntfFindWdgTab::eLBHandler(TListChangeInfo& wnotEvent)
{
    if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::SelChanged)
    {
        // Build up the full id
        tCIDLib::TCard8 c8DesId = m_c8TmplId;
        c8DesId |= m_pwndMatches->c4IndexToId(m_pwndMatches->c4CurItem());
        m_pcivEdit->pwndEditor()->SelectByDesId(c8DesId);
    }

    return tCIDCtrls::EEvResponses::Handled;
}





// ---------------------------------------------------------------------------
//  CLASS: TIntfToolsTab
// PREFIX: wnd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TIntfToolsTab: Constructors and Destructor
// ---------------------------------------------------------------------------
TIntfToolsTab::TIntfToolsTab() :

    TTabWindow(L"IntfEdTools", TString::strEmpty(), kCIDLib::False, kCIDLib::True)
    , m_pwndScroll(nullptr)
    , m_pwndTool_BottomAlign(nullptr)
    , m_pwndTool_FontBold(nullptr)
    , m_pwndTool_FontDecrease(nullptr)
    , m_pwndTool_FontFace(nullptr)
    , m_pwndTool_FontIncrease(nullptr)
    , m_pwndTool_FontItalic(nullptr)
    , m_pwndTool_LeftAlign(nullptr)
    , m_pwndTool_RightAlign(nullptr)
    , m_pwndTool_TopAlign(nullptr)
    , m_pwndTool_VertCenter(nullptr)
    , m_pwndTool_VertSpace(nullptr)
    , m_pwndTools(nullptr)
{
}

TIntfToolsTab::~TIntfToolsTab()
{
}


// ---------------------------------------------------------------------------
//  TIntfToolsTab: Public, inherited methods
// ---------------------------------------------------------------------------

// We set up the passed window to receive notifications from all our buttons
tCIDLib::TVoid TIntfToolsTab::WireTo(TIntfEditWnd& wndTarget)
{
    m_pwndTool_BottomAlign->pnothRegisterHandler(&wndTarget, &TIntfEditWnd::eClickHandler);
    m_pwndTool_FontBold->pnothRegisterHandler(&wndTarget, &TIntfEditWnd::eClickHandler);
    m_pwndTool_FontDecrease->pnothRegisterHandler(&wndTarget, &TIntfEditWnd::eClickHandler);
    m_pwndTool_FontFace->pnothRegisterHandler(&wndTarget, &TIntfEditWnd::eClickHandler);
    m_pwndTool_FontIncrease->pnothRegisterHandler(&wndTarget, &TIntfEditWnd::eClickHandler);
    m_pwndTool_FontItalic->pnothRegisterHandler(&wndTarget, &TIntfEditWnd::eClickHandler);
    m_pwndTool_LeftAlign->pnothRegisterHandler(&wndTarget, &TIntfEditWnd::eClickHandler);
    m_pwndTool_HorzCenter->pnothRegisterHandler(&wndTarget, &TIntfEditWnd::eClickHandler);
    m_pwndTool_HorzSpace->pnothRegisterHandler(&wndTarget, &TIntfEditWnd::eClickHandler);
    m_pwndTool_RightAlign->pnothRegisterHandler(&wndTarget, &TIntfEditWnd::eClickHandler);
    m_pwndTool_TopAlign->pnothRegisterHandler(&wndTarget, &TIntfEditWnd::eClickHandler);
    m_pwndTool_VertCenter->pnothRegisterHandler(&wndTarget, &TIntfEditWnd::eClickHandler);
    m_pwndTool_VertSpace->pnothRegisterHandler(&wndTarget, &TIntfEditWnd::eClickHandler);
}



// ---------------------------------------------------------------------------
//  TIntfToolsTab: Protected, inherited methods
// ---------------------------------------------------------------------------
// Adjust the scroll window to keep it sized to fit us
tCIDLib::TVoid
TIntfToolsTab::AreaChanged( const   TArea&                  areaPrev
                            , const TArea&                  areaNew
                            , const tCIDCtrls::EPosStates   ePosState
                            , const tCIDLib::TBoolean       bOrgChanged
                            , const tCIDLib::TBoolean       bSizeChanged
                            , const tCIDLib::TBoolean       bStateChanged)
{
    TParent::AreaChanged(areaPrev, areaNew, ePosState, bOrgChanged, bSizeChanged, bStateChanged);

    // Keep the scroll area to our client area
    if (bSizeChanged && (ePosState != tCIDCtrls::EPosStates::Minimized) && m_pwndScroll)
    {
        m_pwndScroll->SetSize(areaClient().szArea(), kCIDLib::True);

        // Let the
    }
}


// Creates our scroll window and then the button window inside that
tCIDLib::TBoolean TIntfToolsTab::bCreated()
{
    TParent::bCreated();

    // Create the scroll window
    m_pwndScroll = new TScrollArea();
    m_pwndScroll->Create
    (
        *this
        , kCIDCtrls::widFirstCtrl
        , areaClient()
        , tCIDCtrls::EWndStyles::ClippingVisChild
        , tCIDCtrls::EScrAStyles::None
        , tCIDCtrls::EExWndStyles::ControlParent
    );

    //
    //  Create the button container window. Set up all the magic numbers to make them
    //  easy to change. We put a static multi-text at the bottom with some instructions.
    //
    //  We arrange them like this:
    //
    //      LAlign      RAlign      FBold
    //
    //      BAlign      TAlign      FItalic
    //
    //      HCenter     VCenter     FInc
    //
    //      HSpace      VSpace      FDec
    //
    //                              FFace
    //
    //      ------ instruction ----------
    //
    const tCIDLib::TInt4 i4Space = 16;
    const tCIDLib::TInt4 i4ButtonSize = 40;
    const tCIDLib::TInt4 i4Offset(i4Space + i4ButtonSize);
    const tCIDCtrls::EWndStyles eToolStyles(tCIDCtrls::EWndStyles::VisTabChild);

    // Calculate the area required to hold the tools, with spacing
    TArea areaTools
    (
        0, 0, (4 * i4Space) + (3 * i4ButtonSize), (5 * i4Space) + (5 * i4ButtonSize)
    );

    // Add a little more for the instruction text so it can be a bit wider
    areaTools.AdjustSize(TGUIFacility::c4AdjustVDlgUnit(48), 0);

    //
    //  Set up the area of the instruction text. It can be a bit wider
    //  than the area of the buttons as well.
    //
    TArea areaInstruct(areaTools);
    areaInstruct.i4Y(areaTools.i4Bottom() + i4Space * 3);
    areaInstruct.c4Height(TGUIFacility::c4AdjustVDlgUnit(48));
    areaInstruct.Deflate(i4Space, 0);
    areaInstruct.AdjustSize
    (
        -tCIDLib::TInt4(TGUIFacility::c4AdjustVDlgUnit(8)), 0
    );

    // Add this to the overall tools area now
    areaTools.i4Bottom(areaInstruct.i4Bottom() + i4Space);

    m_pwndTools = new TGenericWnd();
    m_pwndTools->CreateGenWnd
    (
        *m_pwndScroll
        , areaTools
        , tCIDCtrls::EWndStyles::ClippingVisChild
        , tCIDCtrls::EExWndStyles::ControlParent
        , kCIDCtrls::widFirstCtrl
    );

    // Let's create the buttons now. Currently we have three columns of 4 each so far
    tCIDCtrls::TWndId widCur = kCIDCtrls::widFirstCtrl;

    // Do column 1
    TArea areaCur(i4Space, i4Space, i4ButtonSize, i4ButtonSize);

    m_pwndTool_LeftAlign = new TImgPushButton();
    m_pwndTool_LeftAlign->CreateButton
    (
        *m_pwndTools
        , widCur++
        , areaCur
        , TString::strEmpty()
        , facCQCIntfEd()
        , kCQCIntfEd::ridPNG_LeftAlign
        , eToolStyles
    );
    m_pwndTool_LeftAlign->strName(kCQCIntfEd_::strTool_LeftAlign);
    areaCur.AdjustOrg(0, i4Offset);

    m_pwndTool_BottomAlign = new TImgPushButton();
    m_pwndTool_BottomAlign->CreateButton
    (
        *m_pwndTools
        , widCur++
        , areaCur
        , TString::strEmpty()
        , facCQCIntfEd()
        , kCQCIntfEd::ridPNG_BottomAlign
        , eToolStyles
    );
    m_pwndTool_BottomAlign->strName(kCQCIntfEd_::strTool_BottomAlign);
    areaCur.AdjustOrg(0, i4Offset);

    m_pwndTool_HorzCenter = new TImgPushButton();
    m_pwndTool_HorzCenter->CreateButton
    (
        *m_pwndTools
        , widCur++
        , areaCur
        , TString::strEmpty()
        , facCQCIntfEd()
        , kCQCIntfEd::ridPNG_HorzCenter
        , eToolStyles
    );
    m_pwndTool_HorzCenter->strName(kCQCIntfEd_::strTool_HorzCenter);
    areaCur.AdjustOrg(0, i4Offset);

    m_pwndTool_HorzSpace = new TImgPushButton();
    m_pwndTool_HorzSpace->CreateButton
    (
        *m_pwndTools
        , widCur++
        , areaCur
        , TString::strEmpty()
        , facCQCIntfEd()
        , kCQCIntfEd::ridPNG_HorzSpace
        , eToolStyles
    );
    m_pwndTool_HorzSpace->strName(kCQCIntfEd_::strTool_HorzSpace);
    areaCur.AdjustOrg(0, i4Offset);


    // Do column 2
    areaCur.i4Y(i4Space);
    areaCur.AdjustOrg(i4Offset, 0);

    m_pwndTool_RightAlign = new TImgPushButton();
    m_pwndTool_RightAlign->CreateButton
    (
        *m_pwndTools
        , widCur++
        , areaCur
        , TString::strEmpty()
        , facCQCIntfEd()
        , kCQCIntfEd::ridPNG_RightAlign
        , eToolStyles
    );
    m_pwndTool_RightAlign->strName(kCQCIntfEd_::strTool_RightAlign);
    areaCur.AdjustOrg(0, i4Offset);

    m_pwndTool_TopAlign = new TImgPushButton();
    m_pwndTool_TopAlign->CreateButton
    (
        *m_pwndTools
        , widCur++
        , areaCur
        , TString::strEmpty()
        , facCQCIntfEd()
        , kCQCIntfEd::ridPNG_TopAlign
        , eToolStyles
    );
    m_pwndTool_TopAlign->strName(kCQCIntfEd_::strTool_TopAlign);
    areaCur.AdjustOrg(0, i4Offset);

    m_pwndTool_VertCenter = new TImgPushButton();
    m_pwndTool_VertCenter->CreateButton
    (
        *m_pwndTools
        , widCur++
        , areaCur
        , TString::strEmpty()
        , facCQCIntfEd()
        , kCQCIntfEd::ridPNG_VertCenter
        , eToolStyles
    );
    m_pwndTool_VertCenter->strName(kCQCIntfEd_::strTool_VertCenter);
    areaCur.AdjustOrg(0, i4Offset);

    m_pwndTool_VertSpace = new TImgPushButton();
    m_pwndTool_VertSpace->CreateButton
    (
        *m_pwndTools
        , widCur++
        , areaCur
        , TString::strEmpty()
        , facCQCIntfEd()
        , kCQCIntfEd::ridPNG_VertSpace
        , eToolStyles
    );
    m_pwndTool_VertSpace->strName(kCQCIntfEd_::strTool_VertSpace);
    areaCur.AdjustOrg(0, i4Offset);


    // Do column 3
    areaCur.i4Y(i4Space);
    areaCur.AdjustOrg(i4Offset, 0);

    m_pwndTool_FontBold = new TImgPushButton();
    m_pwndTool_FontBold->CreateButton
    (
        *m_pwndTools
        , widCur++
        , areaCur
        , TString::strEmpty()
        , facCQCIntfEd()
        , kCQCIntfEd::ridPNG_FontBold
        , eToolStyles
    );
    m_pwndTool_FontBold->strName(kCQCIntfEd_::strTool_FontBold);
    areaCur.AdjustOrg(0, i4Offset);

    m_pwndTool_FontItalic= new TImgPushButton();
    m_pwndTool_FontItalic->CreateButton
    (
        *m_pwndTools
        , widCur++
        , areaCur
        , TString::strEmpty()
        , facCQCIntfEd()
        , kCQCIntfEd::ridPNG_FontItalic
        , eToolStyles
    );
    m_pwndTool_FontItalic->strName(kCQCIntfEd_::strTool_FontItalic);
    areaCur.AdjustOrg(0, i4Offset);

    m_pwndTool_FontDecrease = new TImgPushButton();
    m_pwndTool_FontDecrease->CreateButton
    (
        *m_pwndTools
        , widCur++
        , areaCur
        , TString::strEmpty()
        , facCQCIntfEd()
        , kCQCIntfEd::ridPNG_FontDecrease
        , eToolStyles
    );
    m_pwndTool_FontDecrease->strName(kCQCIntfEd_::strTool_FontDecrease);
    areaCur.AdjustOrg(0, i4Offset);

    m_pwndTool_FontIncrease = new TImgPushButton();
    m_pwndTool_FontIncrease->CreateButton
    (
        *m_pwndTools
        , widCur++
        , areaCur
        , TString::strEmpty()
        , facCQCIntfEd()
        , kCQCIntfEd::ridPNG_FontIncrease
        , eToolStyles
    );
    m_pwndTool_FontIncrease->strName(kCQCIntfEd_::strTool_FontIncrease);
    areaCur.AdjustOrg(0, i4Offset);

    m_pwndTool_FontFace = new TImgPushButton();
    m_pwndTool_FontFace->CreateButton
    (
        *m_pwndTools
        , widCur++
        , areaCur
        , TString::strEmpty()
        , facCQCIntfEd()
        , kCQCIntfEd::ridPNG_FontFace
        , eToolStyles
    );
    m_pwndTool_FontFace->strName(kCQCIntfEd_::strTool_FontFace);
    areaCur.AdjustOrg(0, i4Offset);


    //
    //  Create the instruction text. Set it to use the small font, so that it's not
    //  overly obtrusive.
    //
    TStaticMultiText* pwndInstruct = new TStaticMultiText();
    pwndInstruct->Create
    (
        *m_pwndTools
        , widCur++
        , areaInstruct
        , facCQCIntfEd().strMsg(kIEdMsgs::midEdit_AlignInstruct)
        , tCIDCtrls::EWndStyles::VisChild
        , tCIDCtrls::EMLSStyles::None
        , tCIDLib::EHJustify::Center
    );
    pwndInstruct->SetFont(TGUIFacility::gfontDefSmall());

    // Tell the scroll window about the tools window it should manage
    m_pwndScroll->SetChild(m_pwndTools);

    return kCIDLib::True;
}
