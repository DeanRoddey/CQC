//
// FILE NAME: CQCGKit_EdExprDlg_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/01/2004
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
//  This is the header for the CQCGKit_EdExprDlg.cpp file. This guy implements two dialogs
//  for editing standard CQC expressions.
//
//  One lets them both select a source field and then set up the expression for that field.
//  We provide interactive value selection based on the field selected.
//
//  The other one works with some already selected value source. We can't assume it is a
//  field, it might be a variable or something else. TO allow for interactive value selection
//  we accept a data type and limits string. If it's a field, the caller passes us info from
//  the field. If it's a variable, he tells us based on that. If they don't know what to
//  limit it so, they can pass a string type and no limits.
//
//
//  <TBD> For not it appears the first one is not used anywhere, only the second one. We
//  should update the first to work like the second, where it disables the comparison value
//  entry field and value selection button if a comparison value using statement is not
//  selected. But, for now, no need since it's not in use.
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
//   CLASS: TCQCEdExprDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------
class TCQCEdExprDlg : public TDlgBox
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCEdExprDlg();

        TCQCEdExprDlg(const TCQCEdExprDlg&) = delete;
        TCQCEdExprDlg(TCQCEdExprDlg&&) = delete;

        ~TCQCEdExprDlg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCEdExprDlg& operator=(const TCQCEdExprDlg&) = delete;
        TCQCEdExprDlg& operator=(TCQCEdExprDlg&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bRunDlg
        (
            const   TWindow&                wndOwner
            , const TString&                strTitle
            ,       TString&                strDevice
            ,       TString&                strField
            ,       TCQCFldDef&             flddSelected
            ,       TCQCExpression&         exprEdit
            , const tCIDLib::TBoolean       bChangeDescr = kCIDLib::True
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCreated() final;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bValidate();

        tCIDLib::TBoolean bValidate
        (
                    TString&                strErrmsg
        );

        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );

        tCIDCtrls::EEvResponses eListHandler
        (
                    TListChangeInfo&        wnotEvent
        );

        tCIDLib::TVoid LoadLists();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bChangeDescr
        //      Indicates whether the description can be changed or not. In some cases,
        //      when editing an existing expression, it is not desirable to allow the
        //      description to change.
        //
        //  m_cfcData
        //      A field cache object, which provides us with the info we need to present
        //      the available readable fields, and to go back and look up the field info
        //      during validation.
        //
        //  m_exprEdit
        //      A temp expression that we copy the caller's into for editing purposes.
        //      The validation method fills it in with the new stuff. If we save, we copy
        //      it back to the caller's.
        //
        //  m_flddAssoc
        //      We store any incoming associated field here until we can get it loaded up,
        //      and we put any outgoing here for putting back into the caller's parm.
        //
        //  m_pexprOrg
        //      A pointer to the original data so we can check for changes if they hit Cancel.
        //
        //  m_pwndXXXX
        //      Some typed pointers to our widgets. We don't own them, we just want to
        //      avoid lots of downcasting when we use them.
        //
        //  m_strField
        //  m_strDevice
        //      We store any incoming moniker/field here until we can get it loaded up, and
        //      we put any outgoing here for putting back into the caller's parm.
        //
        //  m_strTitle
        //      This dialog is used from a number of place, so they provide the desired
        //      title bar.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bChangeDescr;
        TCQCFldCache        m_cfcData;
        TCQCExpression      m_exprEdit;
        TCQCFldDef          m_flddAssoc;
        TCQCExpression*     m_pexprOrg;
        TPushButton*        m_pwndCancelButton;
        TEntryField*        m_pwndCompVal;
        TEntryField*        m_pwndDescription;
        TComboBox*          m_pwndDevices;
        TComboBox*          m_pwndExprType;
        TListBox*           m_pwndFields;
        TCheckBox*          m_pwndNegate;
        TPushButton*        m_pwndSaveButton;
        TPushButton*        m_pwndSelValButton;
        TString             m_strDevice;
        TString             m_strField;
        TString             m_strTitle;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCEdExprDlg,TDlgBox)
};


// ---------------------------------------------------------------------------
//   CLASS: TCQCEdExprDlg2
//  PREFIX: dlg
// ---------------------------------------------------------------------------
class CQCGKITEXPORT TCQCEdExprDlg2 : public TDlgBox
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCEdExprDlg2();

        TCQCEdExprDlg2(const TCQCEdExprDlg2&) = delete;
        TCQCEdExprDlg2(TCQCEdExprDlg2&&) = delete;

        ~TCQCEdExprDlg2();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCEdExprDlg2& operator=(const TCQCEdExprDlg2&) = delete;
        TCQCEdExprDlg2& operator=(TCQCEdExprDlg2&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bRunDlg
        (
            const   TWindow&                wndOwner
            , const TString&                strTitle
            , const tCQCKit::EFldTypes      eType
            , const TString&                strLimits
            ,       TCQCExpression&         exprEdit
            , const tCIDLib::TBoolean       bChangeDescr = kCIDLib::True
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCreated() final;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bValidate();

        tCIDLib::TBoolean bValidate
        (
                    TString&                strErrmsg
        );

        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );

        tCIDCtrls::EEvResponses eListHandler
        (
                    TListChangeInfo&        wnotEvent
        );

        tCIDLib::TVoid GetSelectedType
        (
                    tCQCKit::EExprTypes&    eType
                    , tCQCKit::EStatements& eStatement
        )   const;

        tCIDLib::TVoid InitWidgets();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bChangeDescr
        //      Indicates whether the description can be changed or not. In some cases, when
        //      editing an existing expression, it is not desirable to allow the description
        //      to change.
        //
        //  m_exprEdit
        //      A temp expression that we copy the caller's into for editing purposes. The
        //      validation method fills it in with the new stuff. If we save, we copy it back
        //      to the caller's.
        //
        //  m_eType
        //      The data type of whatever it is is providing the source value.
        //
        //  m_pexprOrg
        //      A pointer to the original data to check for changes if they hit Cancel.
        //
        //  m_pwndXXXX
        //      Some typed pointers to our widgets. We don't own them, we just want to avoid
        //      lots of downcasting when we use them.
        //
        //  m_strLimits
        //      Any limits placed on the value, which is used to support interactive value
        //      selection.
        //
        //  m_strTitle
        //      This dialog is used from a number of places, so they provide the desired
        //      title bar.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bChangeDescr;
        TCQCExpression      m_exprEdit;
        tCQCKit::EFldTypes  m_eType;
        TCQCExpression*     m_pexprOrg;
        TPushButton*        m_pwndCancelButton;
        TEntryField*        m_pwndCompVal;
        TEntryField*        m_pwndDescription;
        TComboBox*          m_pwndExprType;
        TCheckBox*          m_pwndNegate;
        TPushButton*        m_pwndSaveButton;
        TPushButton*        m_pwndSelValButton;
        TString             m_strLimits;
        TString             m_strTitle;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCEdExprDlg2,TDlgBox)
};

#pragma CIDLIB_POPPACK



