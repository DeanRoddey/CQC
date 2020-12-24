//
// FILE NAME: CQCAdmin_ImgListDlg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/18/2015
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
//  This file implements a dialog that let's the user configure the image list
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
#include    "CQCIntfEd_.hpp"
#include    "CQCIntfEd_ImgListDlg_.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TImgListSelDlg, TDlgBox)



// ---------------------------------------------------------------------------
//   CLASS: TImgListSelDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TImgListSelDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TImgListSelDlg::TImgListSelDlg() :

    m_pcivOwner(nullptr)
    , m_pmiwdgEdit(nullptr)
    , m_pwndEnable(nullptr)
    , m_pwndCancelButton(nullptr)
    , m_pwndClrButton(nullptr)
    , m_pwndHOfs(nullptr)
    , m_pwndHOfsVal(nullptr)
    , m_pwndList(nullptr)
    , m_pwndOpacity(nullptr)
    , m_pwndOpVal(nullptr)
    , m_pwndPreview(nullptr)
    , m_pwndSaveButton(nullptr)
    , m_pwndSelButton(nullptr)
    , m_pwndVOfs(nullptr)
    , m_pwndVOfsVal(nullptr)
{
}

TImgListSelDlg::~TImgListSelDlg()
{
}


// ---------------------------------------------------------------------------
//  TImgListSelDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TImgListSelDlg::bRun(const  TWindow&                wndOwner
                    ,       MCQCIntfImgListIntf&    miwdgEdit
                    ,       TCQCIntfEdView* const   pcivOwner
                    , const TArea&                  areaWidget)
{
    //
    //  If it doesn't have any defined images, do nothing. This simplifies everything
    //  else below because it never has to deal with no selection.
    //
    if (!miwdgEdit.c4ImgKeyCount())
        return kCIDLib::False;

    // Store incoming stuff for later use
    m_areaWidget = areaWidget;
    m_pcivOwner = pcivOwner;
    m_pmiwdgEdit = &miwdgEdit;

    //
    //  Save the image list info so that we can restore it if they cancel, to insure
    //  that no changes are kept.
    //
    static_cast<MCQCIntfImgListIntf&>(m_iewSave) = miwdgEdit;

    const tCIDLib::TCard4 c4Res = c4RunDlg
    (
        wndOwner, facCQCIntfEd(), kCQCIntfEd::ridDlg_ImgList
    );

    if (c4Res == kCQCIntfEd::ridDlg_ImgList_Save)
        return kCIDLib::True;

    // Make sure we get rid of any changes
    miwdgEdit = m_iewSave;
    m_pcivOwner->Redraw(areaWidget);

    return kCIDLib::False;
}


// ---------------------------------------------------------------------------
//  TImgListSelDlg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TImgListSelDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRet = TParent::bCreated();

    // First get typed pointers to the widget we always keep
    CastChildWnd(*this, kCQCIntfEd::ridDlg_ImgList_Enable, m_pwndEnable);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_ImgList_Cancel, m_pwndCancelButton);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_ImgList_ClrImg, m_pwndClrButton);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_ImgList_HOfs, m_pwndHOfs);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_ImgList_HOfsVal, m_pwndHOfsVal);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_ImgList_List, m_pwndList);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_ImgList_Opacity, m_pwndOpacity);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_ImgList_OpVal, m_pwndOpVal);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_ImgList_Img, m_pwndPreview);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_ImgList_Save, m_pwndSaveButton);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_ImgList_SelImg, m_pwndSelButton);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_ImgList_VOfs, m_pwndVOfs);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_ImgList_VOfsVal, m_pwndVOfsVal);

    // Iterate the defined images and load up the list of image names
    const tCIDLib::TCard4 c4Count = m_pmiwdgEdit->c4ImgKeyCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        m_pwndList->c4AddItem(m_pmiwdgEdit->imgiAt(c4Index).m_strKey);

    //
    //  Set the ranges on the sliders
    //
    m_pwndHOfs->SetRange(-128, 128);
    m_pwndOpacity->SetRange(0, 255);
    m_pwndVOfs->SetRange(-128, 128);

    //
    //  The image preview is sitting over background fill, so set the clip siblings
    //  style.
    //
    m_pwndPreview->SetWndStyle(tCIDCtrls::EWndStyles::ClipSiblings, kCIDLib::True);

    // Register our handlers
    m_pwndCancelButton->pnothRegisterHandler(this, &TImgListSelDlg::eClickHandler);
    m_pwndClrButton->pnothRegisterHandler(this, &TImgListSelDlg::eClickHandler);
    m_pwndEnable->pnothRegisterHandler(this, &TImgListSelDlg::eClickHandler);
    m_pwndHOfs->pnothRegisterHandler(this, &TImgListSelDlg::eSliderHandler);
    m_pwndList->pnothRegisterHandler(this, &TImgListSelDlg::eListHandler);
    m_pwndOpacity->pnothRegisterHandler(this, &TImgListSelDlg::eSliderHandler);
    m_pwndSaveButton->pnothRegisterHandler(this, &TImgListSelDlg::eClickHandler);
    m_pwndSelButton->pnothRegisterHandler(this, &TImgListSelDlg::eClickHandler);
    m_pwndVOfs->pnothRegisterHandler(this, &TImgListSelDlg::eSliderHandler);

    // Select the first image in the list, if there are any
    if (m_pwndList->c4ItemCount())
        m_pwndList->SelectByIndex(0, kCIDLib::True);

    return bRet;
}


// ---------------------------------------------------------------------------
//  TImgListSelDlg: Private, non-virutal methods
// ---------------------------------------------------------------------------
tCIDCtrls::EEvResponses
TImgListSelDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCIntfEd::ridDlg_ImgList_Cancel)
    {
        EndDlg(kCQCIntfEd::ridDlg_ImgList_Cancel);
    }
     else if (wnotEvent.widSource() == kCQCIntfEd::ridDlg_ImgList_ClrImg)
    {
        MCQCIntfImgListIntf::TImgInfo& imgiCur = m_pmiwdgEdit->imgiAt(m_pwndList->c4CurItem());

        // Reset this image item and update our dialog display info
        imgiCur.Reset();
        ShowImg(imgiCur);

        // Force a redraw to get rid of the image in case it was visible
        m_pcivOwner->Redraw(m_areaWidget);
    }
     else if (wnotEvent.widSource() == kCQCIntfEd::ridDlg_ImgList_Enable)
    {
        // This involved enforcement of some rules so we call ahelper
        SetCurEnable(wnotEvent.bState());
    }
     else if (wnotEvent.widSource() == kCQCIntfEd::ridDlg_ImgList_Save)
    {
        // Make sure any required images are set
        // <TBD>

        EndDlg(kCQCIntfEd::ridDlg_ImgList_Save);
    }
     else if (wnotEvent.widSource() == kCQCIntfEd::ridDlg_ImgList_SelImg)
    {
        MCQCIntfImgListIntf::TImgInfo& imgiNew = m_pmiwdgEdit->imgiAt(m_pwndList->c4CurItem());

        TString strSelected;
        const tCIDLib::TBoolean bRes = facCQCTreeBrws().bSelectFile
        (
            *this
            , L"Select an Image"
            , tCQCRemBrws::EDTypes::Image
            , imgiNew.m_strImageName
            , m_pcivOwner->cuctxToUse()
            , tCQCTreeBrws::EFSelFlags::SelectItems
            , strSelected
        );

        if (bRes)
        {
            // Get the image for this guy
            try
            {
                TWndPtrJanitor janPtr(tCIDCtrls::ESysPtrs::Wait);
                TDataSrvClient dsclImg;

                TIntfImgCachePtr cptrImg = facCQCIntfEng().cptrGetImage
                (
                    strSelected, *m_pcivOwner, dsclImg
                );

                // Update the sample display with the thumb info
                m_pwndPreview->SetBitmap
                (
                    cptrImg->bmpImage()
                    , cptrImg->c4TransClr()
                    , cptrImg->bTransparent()
                    , imgiNew.m_c1Opacity
                );

                // Store the image info for this one
                imgiNew.m_strImageName = strSelected;

                // Make sure this guy is enabled
                imgiNew.m_bEnabled = kCIDLib::True;
                m_pwndEnable->SetCheckedState(kCIDLib::True);

                // Load the image in case it's new or changed
                imgiNew.m_cptrImg = facCQCIntfEng().cptrGetImage
                (
                    strSelected, *m_pcivOwner, dsclImg
                );

                // And update our display info in case we just enabled it
                ShowImg(imgiNew);

                // And force a redraw to redraw with the slider
                m_pcivOwner->Redraw(m_areaWidget);
            }

            catch(const TError& errToCatch)
            {
                facCQCGKit().ShowError
                (
                    *this, L"Could not aquire the selected image", errToCatch
                );
                return tCIDCtrls::EEvResponses::Handled;
            }
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}


tCIDCtrls::EEvResponses
TImgListSelDlg::eListHandler(TListChangeInfo& wnotEvent)
{
    // When a new image is selected, update the image info
    if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::SelChanged)
    {
        if (wnotEvent.widSource() == kCQCIntfEd::ridDlg_ImgList_List)
        {
            MCQCIntfImgListIntf::TImgInfo& imgiNew = m_pmiwdgEdit->imgiAt(wnotEvent.c4Index());
            ShowImg(imgiNew);
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}


//
//  Some helpers to handle enabling other images when the current one's state is
//  changed.
//
tCIDLib::TVoid TImgListSelDlg::EnableAllImages(const tCIDLib::TBoolean bState)
{
    const tCIDLib::TCard4 c4Count = m_pmiwdgEdit->c4ImgKeyCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        m_pmiwdgEdit->imgiAt(c4Index).m_bEnabled = bState;
}

tCIDLib::TVoid TImgListSelDlg::EnableRIAImages()
{
    const tCIDLib::TCard4 c4Count = m_pmiwdgEdit->c4ImgKeyCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        MCQCIntfImgListIntf::TImgInfo& imgiCur = m_pmiwdgEdit->imgiAt(c4Index);

        if (imgiCur.m_eReqFlag == tCQCIntfEng::EImgReqFlags::RequiredIfAny)
            imgiCur.m_bEnabled = kCIDLib::True;
    }
}


//
//  We need to keep update the current image's info on the fly so we don't have to
//  try to catch it during selection changes.
//
tCIDCtrls::EEvResponses
TImgListSelDlg::eSliderHandler(TSliderChangeInfo& wnotEvent)
{
    MCQCIntfImgListIntf::TImgInfo& imgiNew = m_pmiwdgEdit->imgiAt(m_pwndList->c4CurItem());

    if ((wnotEvent.eEvent() == tCIDCtrls::ESldrEvents::Change)
    ||  (wnotEvent.eEvent() == tCIDCtrls::ESldrEvents::EndTrack))
    {
        const tCIDLib::TCard4 c4ImgIndex = m_pwndList->c4CurItem();

        if (wnotEvent.widSource() == kCQCIntfEd::ridDlg_ImgList_HOfs)
        {
            imgiNew.m_i4HOffset = wnotEvent.i4Value();
            m_pcivOwner->Redraw(m_areaWidget);

            m_strFmt.SetFormatted(imgiNew.m_i4HOffset);
            m_pwndHOfsVal->strWndText(m_strFmt);
        }
         else if (wnotEvent.widSource() == kCQCIntfEd::ridDlg_ImgList_Opacity)
        {
            m_pmiwdgEdit->SetImgOpacity
            (
                imgiNew.m_strKey, tCIDLib::TCard1(wnotEvent.i4Value())
            );
            m_pcivOwner->Redraw(m_areaWidget);

            m_strFmt.SetFormatted(imgiNew.m_c1Opacity);
            m_pwndOpVal->strWndText(m_strFmt);

            // Update the preview as well for this one
            m_pwndPreview->c1Opacity(imgiNew.m_c1Opacity);
        }
         else if (wnotEvent.widSource() == kCQCIntfEd::ridDlg_ImgList_VOfs)
        {
            imgiNew.m_i4VOffset = wnotEvent.i4Value();
            m_pcivOwner->Redraw(m_areaWidget);

            m_strFmt.SetFormatted(imgiNew.m_i4VOffset);
            m_pwndVOfsVal->strWndText(m_strFmt);
        }
    }
     else if (wnotEvent.eEvent() == tCIDCtrls::ESldrEvents::Track)
    {
        //
        //  We update the display value but don't try to set the actual value. That'll
        //  happen on the EndTrack event.
        //
        if (wnotEvent.widSource() == kCQCIntfEd::ridDlg_ImgList_HOfs)
        {
            m_strFmt.SetFormatted(wnotEvent.i4Value());
            m_pwndHOfsVal->strWndText(m_strFmt);
        }
         else if (wnotEvent.widSource() == kCQCIntfEd::ridDlg_ImgList_Opacity)
        {
            m_strFmt.SetFormatted(tCIDLib::TCard1(wnotEvent.i4Value()));
            m_pwndOpVal->strWndText(m_strFmt);
        }
         else if (wnotEvent.widSource() == kCQCIntfEd::ridDlg_ImgList_VOfs)
        {
            m_strFmt.SetFormatted(wnotEvent.i4Value());
            m_pwndVOfsVal->strWndText(m_strFmt);
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}


tCIDLib::TVoid TImgListSelDlg::ShowImg(const MCQCIntfImgListIntf::TImgInfo& imgiShow)
{
    m_pwndEnable->SetCheckedState(imgiShow.m_bEnabled);

    //
    //  If the image is required, then disable the enable check box so it cannot be
    //  disabled.
    //
    m_pwndEnable->SetEnable(imgiShow.m_eReqFlag != tCQCIntfEng::EImgReqFlags::Required);

    // Enable or disable the settings controls based on whether this image is enabled
    m_pwndHOfsVal->SetEnable(imgiShow.m_bEnabled);
    m_pwndVOfsVal->SetEnable(imgiShow.m_bEnabled);
    m_pwndOpVal->SetEnable(imgiShow.m_bEnabled);

    m_pwndHOfs->SetEnable(imgiShow.m_bEnabled);
    m_pwndVOfs->SetEnable(imgiShow.m_bEnabled);
    m_pwndOpacity->SetEnable(imgiShow.m_bEnabled);

    if (imgiShow.m_bEnabled)
    {
        // It's enabled so show the content
        m_strFmt.SetFormatted(imgiShow.m_i4HOffset);
        m_pwndHOfsVal->strWndText(m_strFmt);

        m_strFmt.SetFormatted(imgiShow.m_i4VOffset);
        m_pwndVOfsVal->strWndText(m_strFmt);

        m_strFmt.SetFormatted(imgiShow.m_c1Opacity);
        m_pwndOpVal->strWndText(m_strFmt);

        m_pwndHOfs->SetValue(imgiShow.m_i4HOffset);
        m_pwndOpacity->SetValue(imgiShow.m_c1Opacity);
        m_pwndVOfs->SetValue(imgiShow.m_i4VOffset);

        if (imgiShow.m_strImageName.bIsEmpty())
        {
            // No actual image set yet, so reset the preview
            m_pwndPreview->Reset(tCIDGraphDev::EPlacement::Center);
        }
         else
        {
            // And image name is set, try to load it
            try
            {

                TWndPtrJanitor janPtr(tCIDCtrls::ESysPtrs::Wait);
                TDataSrvClient dsclImg;

                TIntfImgCachePtr cptrImg = facCQCIntfEng().cptrGetImage
                (
                    imgiShow.m_strImageName, *m_pcivOwner, dsclImg
                );

                // Update the sample display with the thumb info
                m_pwndPreview->SetBitmap
                (
                    cptrImg->bmpImage()
                    , cptrImg->c4TransClr()
                    , cptrImg->bTransparent()
                    , imgiShow.m_c1Opacity
                );

            }

            catch(TError& errToCatch)
            {
                // Reset the preview to get rid of any previous image
                m_pwndPreview->Reset(tCIDGraphDev::EPlacement::Center);

                facCQCGKit().ShowError
                (
                    *this
                    , strWndText()
                    , L"Could not set the image for preview. See the logs"
                    , errToCatch
                );
            }
        }
    }
     else
    {
        //
        //  This guy is not enabled, so reset the preview and set all of the
        //  controls to default values.
        //
        m_pwndPreview->Reset(tCIDGraphDev::EPlacement::Center);
        m_pwndHOfsVal->strWndText(L"0");
        m_pwndVOfsVal->strWndText(L"0");
        m_pwndOpVal->strWndText(L"255");

        m_pwndHOfs->SetValue(0);
        m_pwndOpacity->SetValue(0);
        m_pwndVOfs->SetValue(0);
    }
}


//
//  When the enable check box is clicked, we have to deal with issues such as required
//  images, and 'required if any' images.
//
tCIDLib::TVoid TImgListSelDlg::SetCurEnable(const tCIDLib::TBoolean bState)
{
    MCQCIntfImgListIntf::TImgInfo& imgiNew = m_pmiwdgEdit->imgiAt(m_pwndList->c4CurItem());
    m_pmiwdgEdit->EnableImgKey(imgiNew.m_strKey, bState);

    // Update our display info
    ShowImg(imgiNew);

    //
    //  If this one is a 'required if any' and we are turning it off,
    //  then we have to flip off all the others that are on. If it's
    //  optional and we are turning it on, look and see if there are
    //  any other 'required if any' ones and turn them on.
    //
    if ((imgiNew.m_eReqFlag == tCQCIntfEng::EImgReqFlags::RequiredIfAny) && !bState)
        EnableAllImages(kCIDLib::False);
    else if (bState)
        EnableRIAImages();
}


