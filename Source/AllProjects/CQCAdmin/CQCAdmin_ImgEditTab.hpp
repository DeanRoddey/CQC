//
// FILE NAME: CQCAdmin_ImgEditTab.hpp
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
//  This is the header for the CQCClient_ImgEditTab.cpp file, which implements a
//  simple editor for images that have been imported into CQC. It allows the user to
//  resize the image, adjust gamma, and control color based transparency.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


// ---------------------------------------------------------------------------
//  CLASS: TImgEditTab
// PREFIX: wnd
// ---------------------------------------------------------------------------
class TImgEditTab : public TContentTab
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TImgEditTab
        (
            const   TString&                strPath
            , const TString&                strRelPath
        );

        TImgEditTab(const TImgEditTab&) = delete;
        TImgEditTab(TImgEditTab&&) = delete;

        ~TImgEditTab();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TImgEditTab& operator=(const TImgEditTab&) = delete;
        TImgEditTab& operator=(TImgEditTab&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid CreateTab
        (
                    TTabbedWnd&             wndParent
            , const TString&                strTabText
            , const TPNGImage&              imgToEdit
        );


    protected  :
        // -------------------------------------------------------------------
        //  Protected, inherited method
        // -------------------------------------------------------------------
        tCIDLib::TVoid AreaChanged
        (
            const   TArea&                  areaPrev
            , const TArea&                  areaNew
            , const tCIDCtrls::EPosStates   ePosState
            , const tCIDLib::TBoolean       bOrgChanged
            , const tCIDLib::TBoolean       bSizeChanged
            , const tCIDLib::TBoolean       bStateChanged
        )   final;

        tCIDLib::TBoolean bClick
        (
            const   tCIDCtrls::EMouseButts  eButton
            , const tCIDCtrls::EMouseClicks eClickType
            , const TPoint&                 pntAt
            , const tCIDLib::TBoolean       bCtrlShift
            , const tCIDLib::TBoolean       bShift
        )   final;

        tCIDLib::TBoolean bCreated() final;

        tCIDLib::TBoolean bMouseMove
        (
            const   TPoint&                 pntAt
            , const tCIDLib::TBoolean       bControlDown
            , const tCIDLib::TBoolean       bShiftDown
        )   final;

        tCIDLib::ESaveRes eDoSave
        (
                    TString&                strErr
            , const tCQCAdmin::ESaveModes   eMode
            ,       tCIDLib::TBoolean&      bChangedSaved
        )   final;

        tCIDLib::TVoid StaticChildClick
        (
            const   tCIDCtrls::TWndId       widChild
            , const tCIDCtrls::EMouseButts  eButton
            , const tCIDCtrls::EMouseClicks eClickType
        )   final;



    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotClick
        );

        tCIDCtrls::EEvResponses eSliderHandler
        (
                    TSliderChangeInfo&      wnotClick
        );

        tCIDLib::TVoid SetGamma
        (
            const   tCIDLib::TFloat8        f8NewGamma
        );

        tCIDLib::TVoid RefreshImage();

        tCIDLib::TVoid ShowImgInfo();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bCaptureMode
        //      Used to indicate mouse capture mode when the color selector is being
        //      dragged.
        //
        //  m_bChanges
        //      We don't have an image pixel content comparitor or anything, so we just
        //      have to remember if we made changes.
        //
        //  m_imgEdit
        //      The image info for the image we are editng. The caller has to convert
        //      it to PNG image format if it wasn't already. Any images from our
        //      image repo will be already.
        //
        //  m_pwndXXX
        //      We get typed pointers to the important controls. m_pwndScroll is a
        //      scroll area. We create m_pwndImage as a child of it and size the
        //      image window to the image. The scroll window allows the user to
        //      scroll it around if needed. The image window is not part of the
        //      dialog description we use to create our controls. We have to create
        //      that one by hand.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bCaptureMode;
        tCIDLib::TBoolean   m_bChanges;
        TPNGImage           m_imgEdit;
        TPushButton*        m_pwndChangeSz;
        TStaticText*        m_pwndCurGamma;
        TCheckBox*          m_pwndDoTrans;
        TPushButton*        m_pwndFlipHorz;
        TPushButton*        m_pwndFlipVert;
        TSlider*            m_pwndSetGamma;
        TImgPreviewWnd*     m_pwndImage;
        TScrollArea*        m_pwndScroll;
        TStaticColor*       m_pwndTransClr;


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TImgEditTab, TContentTab)
};

