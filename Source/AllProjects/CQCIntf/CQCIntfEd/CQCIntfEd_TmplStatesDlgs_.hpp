//
// FILE NAME: CQCIntfEd_TmplStatesDlgs_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 12/23/2015
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
//  This is the header for the CQCIntfEd_TmplStatesDlg.cpp file, which allows the
//  user to edit the states for a template. There is one dialog for listing and
//  managing the states, and others for the editing of states and the editing of
//  the expressions that make up the states.
//
//  In both of our dialogs, we implement the multi-column list boxes in place
//  editor interfce, so that we can allow for some ad hoc IPE of values in our
//  lists of states and state expressions.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

class TTStateAttrEd;

// ---------------------------------------------------------------------------
//   CLASS: TTmplStateDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------
class TTmplStateDlg : public TDlgBox, public MMCLBIPEIntf
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TTmplStateDlg();

        TTmplStateDlg(const TTmplStateDlg&) = delete;

        ~TTmplStateDlg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TTmplStateDlg& operator=(const TTmplStateDlg&) = delete;


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
            ,       TCQCIntfState&          istEdit
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCreated() override;

        tCIDLib::TVoid ModDlgItem
        (
                    TDlgItem&               dlgiToMod
        )   override;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDCtrls::EEvResponses eAttrEditHandler
        (
                    TAttrEditInfo&          wnotInfo
        );

        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );

        tCIDCtrls::EEvResponses eListHandler
        (
                    TListChangeInfo&        wnotEvent
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  istEdit
        //      A copy of the state info, to edit. If the user commits, we copy it
        //      back to the caller's.
        //
        //  pistOrg
        //      A pointer to the original state content, so we can check for changes.
        //
        //  m_pwndXXX
        //      Some typed pointers we get to some of our child widgets we need to
        //      interact with on a typed basis.
        //
        //      The attribute editoris actually our own internal derivative, so we
        //      forward ref it above.
        // -------------------------------------------------------------------
        TCQCIntfState       m_istEdit;
        TCQCIntfState*      m_pistOrg;
        TPushButton*        m_pwndAddButton;
        TPushButton*        m_pwndCancelButton;
        TPushButton*        m_pwndDelButton;
        TTStateAttrEd*      m_pwndEditor;
        TMultiColListBox*   m_pwndList;
        TComboBox*          m_pwndLogOp;
        TPushButton*        m_pwndSaveButton;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TTmplStateDlg,TDlgBox)
};



// ---------------------------------------------------------------------------
//   CLASS: TTmplStatesDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------
class TTmplStatesDlg : public TDlgBox, public MMCLBIPEIntf
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TTmplStatesDlg();

        TTmplStatesDlg(const TTmplStatesDlg&) = delete;

        ~TTmplStatesDlg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TTmplStatesDlg& operator=(const TTmplStatesDlg&) = delete;


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
            ,       TCQCIntfStateList&      istlEdit
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
        tCIDLib::TBoolean bMakeDefName
        (
                    TString&                strToFill
        );

        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );

        tCIDCtrls::EEvResponses eListHandler
        (
                    TListChangeInfo&        wnotEvent
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  istlEdit
        //      A copy of the state info, to edit. If the user commits, we copy it
        //      back to the caller's.
        //
        //  pistlOrg
        //      A pointer to the original state content, so we can check for changes.
        //
        //  m_pwndXXX
        //      Some typed pointers we get to some of our child widgets we need to
        //      interact with on a typed basis.
        // -------------------------------------------------------------------
        TCQCIntfStateList   m_istlEdit;
        TCQCIntfStateList*  m_pistlOrg;
        TPushButton*        m_pwndAddButton;
        TPushButton*        m_pwndCancelButton;
        TPushButton*        m_pwndDelButton;
        TPushButton*        m_pwndEditButton;
        TMultiColListBox*   m_pwndList;
        TPushButton*        m_pwndSaveButton;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TTmplStatesDlg,TDlgBox)
};

#pragma CIDLIB_POPPACK




