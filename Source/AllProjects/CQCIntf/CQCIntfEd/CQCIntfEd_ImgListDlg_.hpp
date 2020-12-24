//
// FILE NAME: CQCIntfEd_ImgListSelDlg_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 12/18/2015
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
//  This is the header for the CQCIntfEd_ImgListSelDlg.cpp file, which allows the
//  user to edit the contents of the image list mixin, which is used by those widgets
//  that support multiple, named images.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TImgListSelDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------
class TImgListSelDlg : public TDlgBox
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TImgListSelDlg();

        TImgListSelDlg(const TImgListSelDlg&) = delete;

        ~TImgListSelDlg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TImgListSelDlg& operator=(const TImgListSelDlg&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bRun
        (
            const   TWindow&                wndOwner
            ,       MCQCIntfImgListIntf&    miwdgEdit
            ,       TCQCIntfEdView* const   pcivOwner
            , const TArea&                  areaWidget
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCreated() override;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );

        tCIDCtrls::EEvResponses eListHandler
        (
                    TListChangeInfo&        wnotEvent
        );

        tCIDCtrls::EEvResponses eSliderHandler
        (
                    TSliderChangeInfo&      wnotEvent
        );

        tCIDLib::TVoid EnableAllImages
        (
            const   tCIDLib::TBoolean       bState
        );

        tCIDLib::TVoid EnableRIAImages();

        tCIDLib::TVoid ShowImg
        (
            const   MCQCIntfImgListIntf::TImgInfo& imgiShow
        );

        tCIDLib::TVoid SetCurEnable
        (
            const   tCIDLib::TBoolean       bState
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_areaWidget
        //      We save the incoming area, which we need to be able to force a redraw
        //      of the area that contains the widget, after we make any changes.
        //
        //  m_iewSave
        //      An image list wrapper to save the image list content on entry. IF the
        //      user cancels, we copy it back in to insure any changes are tossed.
        //
        //  m_pwndXXX
        //      Some typed pointers we get to some of our child widgets we need to
        //      interact with on a typed basis.
        //
        //  m_strFmt
        //      We have to do a lot of value formatting, so we keep a string around
        //      for that.
        // -------------------------------------------------------------------
        TArea                   m_areaWidget;
        TIEImgListWrap          m_iewSave;
        TCQCIntfEdView*         m_pcivOwner;
        MCQCIntfImgListIntf*    m_pmiwdgEdit;
        TCheckBox*              m_pwndEnable;
        TPushButton*            m_pwndCancelButton;
        TPushButton*            m_pwndClrButton;
        TSlider*                m_pwndHOfs;
        TStaticText*            m_pwndHOfsVal;
        TListBox*               m_pwndList;
        TSlider*                m_pwndOpacity;
        TStaticText*            m_pwndOpVal;
        TImgPreviewWnd*         m_pwndPreview;
        TPushButton*            m_pwndSaveButton;
        TPushButton*            m_pwndSelButton;
        TSlider*                m_pwndVOfs;
        TStaticText*            m_pwndVOfsVal;
        TString                 m_strFmt;



        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TImgListSelDlg,TDlgBox)
};

#pragma CIDLIB_POPPACK


