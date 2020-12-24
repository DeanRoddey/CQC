//
// FILE NAME: CQCAdmin_ImgEditTab.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/09/2015
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
//  This file implements a tab for doing basic editing of images in the CQC image
//  repo. This is the read/write counterpart to the TImgViewTab.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCAdmin.hpp"
#include    "CQCAdmin_ImgEditTab.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TImgEditTab, TContentTab)



// ---------------------------------------------------------------------------
//  CLASS: TImgEditTab
// PREFIX: wnd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TImgEditTab: Constructors and destructor
// ---------------------------------------------------------------------------
TImgEditTab::TImgEditTab(const TString& strPath, const TString& strRelPath) :

    TContentTab(strPath, strRelPath, L"Image", kCIDLib::True)
    , m_bCaptureMode(kCIDLib::False)
    , m_bChanges(kCIDLib::False)
    , m_pwndChangeSz(nullptr)
    , m_pwndCurGamma(nullptr)
    , m_pwndDoTrans(nullptr)
    , m_pwndFlipHorz(nullptr)
    , m_pwndFlipVert(nullptr)
    , m_pwndImage(nullptr)
    , m_pwndScroll(nullptr)
    , m_pwndSetGamma(nullptr)
    , m_pwndTransClr(nullptr)
{
}

TImgEditTab::~TImgEditTab()
{
}


// ---------------------------------------------------------------------------
//  TImgEditTab: Public, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid
TImgEditTab::CreateTab(         TTabbedWnd& wndParent
                        , const TString&    strTabText
                        , const TPNGImage&  imgEdit)
{
    // Store away the image data then create us
    m_imgEdit = imgEdit;
    wndParent.c4CreateTab(this, strTabText, 0, kCIDLib::True);
}


// ---------------------------------------------------------------------------
//  TImgEditTab: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TImgEditTab::AreaChanged(const  TArea&                  areaPrev
                        , const TArea&                  areaNew
                        , const tCIDCtrls::EPosStates   ePosState
                        , const tCIDLib::TBoolean       bOrgChanged
                        , const tCIDLib::TBoolean       bSizeChanged
                        , const tCIDLib::TBoolean       bStateChanged)
{
    // Call our parent first
    TTabWindow::AreaChanged(areaPrev, areaNew, ePosState, bOrgChanged, bSizeChanged, bStateChanged);

    // Auto-size our contents to fit
    if ((ePosState != tCIDCtrls::EPosStates::Minimized) && bSizeChanged && m_pwndScroll)
        AutoAdjustChildren(areaPrev, areaNew);
}


tCIDLib::TBoolean
TImgEditTab::bClick(const   tCIDCtrls::EMouseButts  eButton
                    , const tCIDCtrls::EMouseClicks eClickType
                    , const TPoint&                 pntAt
                    , const tCIDLib::TBoolean       bCtrlShift
                    , const tCIDLib::TBoolean       bShift)
{
    // If this is a release after we captured, then lets process it
    if (m_bCaptureMode)
    {
        ReleaseMouse();
        m_bCaptureMode = kCIDLib::False;

        // If we turned off transparency during the drag, enable it again
        if (m_imgEdit.bTransClr())
            m_pwndImage->bTransparency(kCIDLib::True);

        if ((eButton == tCIDCtrls::EMouseButts::Left)
        &&  (eClickType == tCIDCtrls::EMouseClicks::Up))
        {
            // Get the release point relative to our image
            TPoint pntImgAt = pntAt;
            m_pwndImage->XlatCoordinates(pntImgAt, *this);
            if (m_pwndImage->bImagePnt(pntImgAt))
            {
                // Get the color at that point in the image
                TRGBClr rgbNew = m_imgEdit.rgbAt(pntImgAt);
                m_pwndTransClr->SetBgnColor(rgbNew, kCIDLib::True, kCIDLib::True);

                m_pwndImage->c4TransClr(rgbNew.c4Color());
                m_imgEdit.c4TransClr(rgbNew.c4Color());

                m_bChanges = kCIDLib::True;
            }
            return kCIDLib::True;
       }
    }

    return kCIDLib::False;
}


tCIDLib::TBoolean TImgEditTab::bCreated()
{
    TParent::bCreated();

    // Get the dialog description we'll use to create the controls
    TDlgDesc dlgdChildren;
    facCQCAdmin.bCreateDlgDesc(kCQCAdmin::ridTab_ImgEdit, dlgdChildren);

    // Create the children
    tCIDLib::TCard4 c4InitFocus;
    CreateDlgItems(dlgdChildren, c4InitFocus);

    // Do an initial auto-adjust to get them sized up to us
    AutoAdjustChildren(dlgdChildren.areaPos(), areaClient());

    // Get pointers to our important controls
    CastChildWnd(*this, kCQCAdmin::ridTab_ImgEdit_ChangeSz, m_pwndChangeSz);
    CastChildWnd(*this, kCQCAdmin::ridTab_ImgEdit_CurGamma, m_pwndCurGamma);
    CastChildWnd(*this, kCQCAdmin::ridTab_ImgEdit_DoTrans, m_pwndDoTrans);
    CastChildWnd(*this, kCQCAdmin::ridTab_ImgEdit_FlipHorz, m_pwndFlipHorz);
    CastChildWnd(*this, kCQCAdmin::ridTab_ImgEdit_FlipVert, m_pwndFlipVert);
    CastChildWnd(*this, kCQCAdmin::ridTab_ImgEdit_Scroll, m_pwndScroll);
    CastChildWnd(*this, kCQCAdmin::ridTab_ImgEdit_SetGamma, m_pwndSetGamma);
    CastChildWnd(*this, kCQCAdmin::ridTab_ImgEdit_TransClr, m_pwndTransClr);

    // Set a little internal margin on the scroll window to look nicer
    m_pwndScroll->c4Margin(8);

    //
    //  Create the static image window. The size of the incoming image and as a child
    //  of the scroll area.
    //
    TArea areaInit(areaClient());
    areaInit.SetSize(m_imgEdit.szImage());

    m_pwndImage = new TImgPreviewWnd();
    m_pwndImage->CreateImgPreview
    (
        *m_pwndScroll
        , kCIDCtrls::widFirstCtrl
        , areaInit
        , tCIDCtrls::EWndStyles::VisChild
        , tCIDWUtils::EImgPStyles::None
    );

    // Tell the scroll area to treat our image window as it's scrollable child
    m_pwndScroll->SetChild(m_pwndImage);

    // Set them both to the standard window background
    m_pwndScroll->SetBgnColor(facCIDCtrls().rgbSysClr(tCIDCtrls::ESysColors::Window));
    m_pwndImage->SetBgnColor(facCIDCtrls().rgbSysClr(tCIDCtrls::ESysColors::Window));

    // Show incoming info
    ShowImgInfo();

    //
    //  Load up the gamma setting slider. We load up from 1.0 to 3.0 in 0.1
    //  increments. Clip it to our valid range if needed.
    //
    tCIDLib::TFloat8 f8Gamma = m_imgEdit.f8Gamma();
    tCIDLib::TInt4 i4Init;
    if (f8Gamma <= 1.0)
    {
        i4Init = 0;
        m_imgEdit.f8Gamma(1.0);
    }
     else if (f8Gamma >= 3.0)
    {
        i4Init = 30;
        m_imgEdit.f8Gamma(3.0);
    }
     else
    {
        i4Init = tCIDLib::TInt4(f8Gamma / 0.1);
    }
    m_pwndSetGamma->SetRange(10, 30);
    m_pwndSetGamma->SetValue(i4Init);

    //
    //  Set the incoming image. The default centered placement is fine for our purposes.
    //  That will not scale it so we'll see it at its current size.
    //
    RefreshImage();

    //
    //  If the image has source alpha transparency, then let's disable the
    //  color transparency.
    //
    if (tCIDLib::bAllBitsOn(m_imgEdit.ePixFmt(), tCIDImage::EPixFmts::Alpha))
    {
        m_pwndDoTrans->SetEnable(kCIDLib::False);
        m_pwndTransClr->SetEnable(kCIDLib::False);
    }
     else
    {
        //
        //  No source alpha, so if the source image has a suggested
        //  transparency color, let's set that up.
        //
        if (m_imgEdit.bTransClr())
            m_pwndDoTrans->SetCheckedState(kCIDLib::True);

        m_pwndTransClr->SetBgnColor
        (
            m_imgEdit.rgbTransClr()
            , kCIDLib::True
            , kCIDLib::True
        );
    }

    // Make sure we have some initial focus
    m_pwndDoTrans->TakeFocus();

    // Set up event handlers
    m_pwndChangeSz->pnothRegisterHandler(this, &TImgEditTab::eClickHandler);
    m_pwndDoTrans->pnothRegisterHandler(this, &TImgEditTab::eClickHandler);
    m_pwndFlipHorz->pnothRegisterHandler(this, &TImgEditTab::eClickHandler);
    m_pwndFlipVert->pnothRegisterHandler(this, &TImgEditTab::eClickHandler);
    m_pwndSetGamma->pnothRegisterHandler(this, &TImgEditTab::eSliderHandler);

    return kCIDLib::True;
}


tCIDLib::TBoolean
TImgEditTab::bMouseMove(const   TPoint&             pntAt
                        , const tCIDLib::TBoolean   bControlDown
                        , const tCIDLib::TBoolean   bShiftDown)
{
    if (m_bCaptureMode)
    {
        TPoint pntImgAt = pntAt;
        m_pwndImage->XlatCoordinates(pntImgAt, *this);
        if (m_pwndImage->bImagePnt(pntImgAt))
        {
            m_pwndTransClr->SetBgnColor
            (
                m_imgEdit.rgbAt(pntImgAt)
                , kCIDLib::True
                , kCIDLib::True
            );
        }

        return kCIDLib::True;
    }
    return kCIDLib::False;
}


tCIDLib::ESaveRes
TImgEditTab::eDoSave(       TString&                strErr
                    , const tCQCAdmin::ESaveModes   eMode
                    ,       tCIDLib::TBoolean&      bChangesSaved)
{
    if (eMode == tCQCAdmin::ESaveModes::Test)
    {
        if (!m_bChanges)
            return tCIDLib::ESaveRes::NoChanges;
        return tCIDLib::ESaveRes::OK;
    }

    // There are changes, so write our current edit contents out if allowed
    try
    {
        //
        //  Get the image and thumb packaged up appropriately. Images can vary
        //  widely in size, so we used chunked streams for high efficiency. But
        //  that means we have to get the info out to contiguous chunks for use
        //  below.
        //
        tCIDLib::TCard4 c4ImgSz, c4ThumbSz;
        THeapBuf mbufImg(1, 32 * (1024 * 1024));
        THeapBuf mbufThumb(1, 4 * (1024 * 1024));
        TSize szThumb;
        {
            TChunkedBinOutStream strmImg(32 * (1024 * 1024));
            TChunkedBinOutStream strmThumb(4 * (1024 * 1024));
            facCQCKit().PackageImg(m_imgEdit, strmImg, strmThumb, szThumb);
            c4ImgSz = strmImg.c4CurSize();
            c4ThumbSz = strmThumb.c4CurSize();

            mbufImg.Reallocate(c4ImgSz, kCIDLib::False);
            mbufThumb.Reallocate(c4ThumbSz, kCIDLib::False);
            strmImg.c4CopyOutTo(mbufImg, 0);
            strmThumb.c4CopyOutTo(mbufThumb, 0);
        }

        TDataSrvClient dsclLoad;
        tCIDLib::TCard4 c4NewSerialNum = 0;
        tCIDLib::TEncodedTime enctLastChange;
        tCIDLib::TKVPFList colXMeta;
        dsclLoad.WriteImage
        (
            strRelPath()
            , c4NewSerialNum
            , enctLastChange
            , mbufImg
            , c4ImgSz
            , mbufThumb
            , c4ThumbSz
            , m_imgEdit.ePixFmt()
            , m_imgEdit.eBitDepth()
            , m_imgEdit.szImage()
            , szThumb
            , kCQCRemBrws::c4Flag_OverwriteOK
            , colXMeta
            , facCQCAdmin.sectUser()
        );

        // It all went well, so clear the changes flag
        m_bChanges = kCIDLib::False;
        bChangesSaved = kCIDLib::True;
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        TExceptDlg dlgErr
        (
            *this
            , strWndText()
            , L"An error occurred while packaging or uploading the image"
            , errToCatch
        );
        return tCIDLib::ESaveRes::Errors;
    }

    // No changes anymore
    return tCIDLib::ESaveRes::NoChanges;
}



tCIDLib::TVoid
TImgEditTab::StaticChildClick(  const   tCIDCtrls::TWndId       widChild
                                , const tCIDCtrls::EMouseButts  eButton
                                , const tCIDCtrls::EMouseClicks eClickType)
{
    // We react to a left click on our color selector
    if ((eButton == tCIDCtrls::EMouseButts::Left)
    &&  (widChild == kCQCAdmin::ridTab_ImgEdit_Selector))
    {
        // If it was a down click, and our image is not alpha based
        if ((eClickType == tCIDCtrls::EMouseClicks::Down)
        &&  !tCIDLib::bAllBitsOn(m_imgEdit.ePixFmt(), tCIDImage::EPixFmts::Alpha))
        {
            // Turn off transparency while we do this
            if (m_pwndImage->bTransparency())
                m_pwndImage->bTransparency(kCIDLib::False);

            // Go into capture mode
            CaptureMouse();
            facCIDCtrls().SetPointer(tCIDCtrls::ESysPtrs::Cross);
            m_bCaptureMode = kCIDLib::True;
        }
    }
}



// ---------------------------------------------------------------------------
//  TImgEditTab: Private, non-virtual methods
// ---------------------------------------------------------------------------

tCIDCtrls::EEvResponses TImgEditTab::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCAdmin::ridTab_ImgEdit_ChangeSz)
    {
        // Get the new desired change from them
        TSize szNew;
        if (facCIDWUtils().bGetSize(*this
                                    , L"Enter a New Image Size"
                                    , szNew
                                    , m_imgEdit.szImage()
                                    , TSize(4, 4)
                                    , TSize(4096, 4096)
                                    , kCIDLib::False))
        {
            // If the size changed, then do the scale
            if (szNew != m_imgEdit.szImage())
            {
                // Put up a wait pointer while we do this
                TWndPtrJanitor janPtr(tCIDCtrls::ESysPtrs::Wait);

                // Do the scaling
                m_imgEdit.Scale(szNew, 4);

                // Update the image display
                RefreshImage();

                // Resize the image window to fit the image
                m_pwndScroll->SetChildSize(m_imgEdit.szImage());

                // Show the new image info
                ShowImgInfo();

                m_bChanges = kCIDLib::True;
            }
        }
    }
     else if (wnotEvent.widSource() == kCQCAdmin::ridTab_ImgEdit_DoTrans)
    {
        // Enable or disable transparency based on the new state of the check box
        m_pwndImage->bTransparency(wnotEvent.bState());
        m_imgEdit.bTransClr(wnotEvent.bState());

        m_bChanges = kCIDLib::True;
    }
     else if (wnotEvent.widSource() == kCQCAdmin::ridTab_ImgEdit_FlipHorz)
    {
        // Ask the image to flip itself and reload it
        m_imgEdit.FlipHorizontally();
        RefreshImage();
        m_bChanges = kCIDLib::True;
    }
     else if (wnotEvent.widSource() == kCQCAdmin::ridTab_ImgEdit_FlipVert)
    {
        // Ask the image to flip itself and reload it
        m_imgEdit.FlipVertically();
        RefreshImage();
        m_bChanges = kCIDLib::True;
    }
    return tCIDCtrls::EEvResponses::Handled;
}


tCIDCtrls::EEvResponses TImgEditTab::eSliderHandler(TSliderChangeInfo& wnotEvent)
{
    //
    //  We only make a change when they drop it, not while dragging, because
    //  the work we have to do can be too much to do that fast. But we do
    //  update the displayed value during drag.
    //
    if (wnotEvent.eEvent() == tCIDCtrls::ESldrEvents::Change)
    {
        const tCIDLib::TFloat8 f8New
        (
            tCIDLib::TFloat8(wnotEvent.i4Value()) / 10.0
        );
        m_imgEdit.f8Gamma(f8New);

        // Update the gamma value display
        TString strVal(TFloat(f8New, 2));
        strChildText(kCQCAdmin::ridTab_ImgEdit_CurGamma,strVal);

        // Update the preview window with a new copy of the image
        RefreshImage();

        m_bChanges = kCIDLib::True;
    }
     else if (wnotEvent.eEvent() == tCIDCtrls::ESldrEvents::Track)
    {
        const tCIDLib::TFloat8 f8New
        (
            tCIDLib::TFloat8(wnotEvent.i4Value()) / 10.0
        );
        TString strVal(TFloat(f8New, 2));
        strChildText(kCQCAdmin::ridTab_ImgEdit_CurGamma, strVal);
    }
    return tCIDCtrls::EEvResponses::Handled;
}


// Load the image back into the preview window with the current setts
tCIDLib::TVoid TImgEditTab::RefreshImage()
{
    TGraphWndDev gdevCompat(*this);
    TBitmap bmpNew(gdevCompat, m_imgEdit);
    m_pwndImage->SetBitmap(bmpNew, m_imgEdit.c4TransClr(), m_imgEdit.bTransClr(), 0xFF);
}


tCIDLib::TVoid TImgEditTab::ShowImgInfo()
{
    TString strTmp;
    strTmp.AppendFormatted(m_imgEdit.c4Width());
    strTmp.Append(L" x ");
    strTmp.AppendFormatted(m_imgEdit.c4Height());
    strChildText(kCQCAdmin::ridTab_ImgEdit_Size, strTmp);

    facCIDImgFact().FormatImgFormat
    (
        strTmp
        , m_imgEdit.ePixFmt()
        , m_imgEdit.eBitDepth()
    );
    strChildText(kCQCAdmin::ridTab_ImgEdit_FmtInfo, strTmp);

    strTmp.SetFormatted(m_imgEdit.f8Gamma(), 2);
    strChildText(kCQCAdmin::ridTab_ImgEdit_CurGamma, strTmp);
}
