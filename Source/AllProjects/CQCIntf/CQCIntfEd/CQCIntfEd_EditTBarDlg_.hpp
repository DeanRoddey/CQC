//
// FILE NAME: CQCIntfEd_EditTBarDlg_.hpp
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
//  This is the header for the CQCIntfEd_EditTBarDlg.cpp file, which allows the user
//  to edit the type specific attributes of a tool bar widget.
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
//   CLASS: TEditTBarDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------
class TEditTBarDlg : public TDlgBox, public MMCLBIPEIntf
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TEditTBarDlg();

        ~TEditTBarDlg();


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIPETestCanEdit
        (
            const   tCIDCtrls::TWndId       widSrc
            , const tCIDLib::TCard4         c4ColInd
            ,       TAttrData&              adatFill
            ,       tCIDLib::TBoolean&      bValueSet
        )   override;

        tCIDLib::TBoolean bIPEValidate
        (
            const   TString&                strSrc
            ,       TAttrData&              adatSrc
            , const TString&                strNewVal
            ,       TString&                strErrMsg
            ,       tCIDLib::TCard8&        c8UserId
        )   const override;

        tCIDLib::TVoid IPEValChanged
        (
            const   tCIDCtrls::TWndId       widSrc
            , const TString&                strSrc
            , const TAttrData&              adatNew
            , const TAttrData&              adatOld
            , const tCIDLib::TCard8         c8UserId
        )   override;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bRun
        (
            const   TWindow&                wndOwner
            ,       TCQCIntfToolbar* const  piwdgEdit
            ,       TCQCIntfEdView* const   pcivOwner
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCreated() override;


    private :
        // -------------------------------------------------------------------
        //  Unimplemented
        // -------------------------------------------------------------------
        TEditTBarDlg(const TEditTBarDlg&);
        tCIDLib::TVoid operator=(const TEditTBarDlg&);


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

        tCIDLib::TVoid ShowAction
        (
            const   TCQCIntfToolbar::TTBButton& tbbSrc
        );

        tCIDLib::TVoid ShowPreview
        (
            const   TString&                strImgPath
            , const tCIDLib::TCard1         c1Opacity
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4NextId
        //      For in place editing purposes, we assign a unique id to each item we
        //      put into the list box.
        //
        //  m_iwdgOrg
        //      The toolbar widget whose attributes we are editing. We use this if
        //      they select the Revert button.
        //
        //  m_pcivOwner
        //      A pointer to the IV view (the editor specific one) that owns our
        //      widget. We need this at least for access to the security info.
        //
        //  m_piwdgEdit
        //      The widget we are editing. This is the actual widget so that changes
        //      show up immediately.
        //
        //  m_pwndXXX
        //      Some typed pointers we get to some of our child widgets we need to
        //      interact with on a typed basis.
        //
        //  m_wstateEdit
        //  m_wstate_Empty
        //      Window states for when we have no buttons in the list and when we
        //      have at least one, to make it convenient to enable/disable the
        //      controls appropriately.
        // -------------------------------------------------------------------
        tCIDLib::TCard4     m_c4NextId;
        TCQCIntfToolbar     m_iwdgOrg;
        TCQCIntfEdView*     m_pcivOwner;
        TCQCIntfToolbar*    m_piwdgEdit;
        TStaticMultiText*   m_pwndActText;
        TPushButton*        m_pwndAdd;
        TPushButton*        m_pwndCancel;
        TPushButton*        m_pwndClrImg;
        TPushButton*        m_pwndDel;
        TPushButton*        m_pwndDown;
        TPushButton*        m_pwndEdAct;
        TImgPreviewWnd*     m_pwndImg;
        TSlider*            m_pwndImgOp;
        TMultiColListBox*   m_pwndList;
        TPushButton*        m_pwndRevert;
        TPushButton*        m_pwndSave;
        TPushButton*        m_pwndSelImg;
        TPushButton*        m_pwndUp;
        TWndState           m_wstateEdit;
        TWndState           m_wstateEmpty;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TEditTBarDlg,TDlgBox)
};

#pragma CIDLIB_POPPACK


