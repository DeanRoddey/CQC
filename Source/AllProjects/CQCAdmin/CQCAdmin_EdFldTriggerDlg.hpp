//
// FILE NAME: CQCAdmin_EdFldTriggerDlg.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/20/2005
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
//  This is the header for the CQCClient_EdFldTriggerDlg.cpp file, which implements
//  the TEdFldTriggerDlg class. This dialog allows the user to configure a field to
//  send a trigger when the value changes in certain ways.
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
//   CLASS: TEdFldTriggerDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------
class TEdFldTriggerDlg : public TDlgBox
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TEdFldTriggerDlg();

        TEdFldTriggerDlg(const TEdFldTriggerDlg&) = delete;
        TEdFldTriggerDlg(TEdFldTriggerDlg&&) = delete;

        ~TEdFldTriggerDlg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TEdFldTriggerDlg& operator=(const TEdFldTriggerDlg&) = delete;
        TEdFldTriggerDlg& operator=(TEdFldTriggerDlg&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bRunDlg
        (
            const   TWindow&                wndOwner
            , const TString&                strMoniker
            , const TCQCFldDef&             flddInfo
            ,       TCQCFldEvTrigger&       fetToEdit
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

        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );

        tCIDCtrls::EEvResponses eComboHandler
        (
                    TListChangeInfo&        wnotEvent
        );

        tCIDLib::TVoid ExprTypeChanged();

        tCIDLib::TVoid GetExprInfo
        (
                    tCQCKit::EExprTypes&    eExprType
            ,       tCQCKit::EStatements&   eStatement
        )   const;


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_fetToEdit
        //      The field event trigger to edit. We fill this with the incoming
        //      value, to provide initial contents to edit, and copy back to
        //      the caller's buffer if the user commits.
        //
        //  m_flddInfo
        //      We also get the field def info, which we need to verify that
        //      any selected expression is legal for the field.
        //
        //  m_pwndXXX
        //      We get some typed pointers to widgets we need to interact with
        //      a lot.
        //
        //  m_strMoniker
        //      The moniker of the driver that the field is owned by, so that we
        //      can display it or use it where necessary. It's not in the field
        //      definition object.
        // -------------------------------------------------------------------
        TCQCFldEvTrigger    m_fetToEdit;
        TCQCFldDef          m_flddInfo;
        TPushButton*        m_pwndCancel;
        TEntryField*        m_pwndCompVal;
        TRadioButton*       m_pwndDisable;
        TComboBox*          m_pwndExprType;
        TComboBox*          m_pwndLatching;
        TRadioButton*       m_pwndOnChange;
        TRadioButton*       m_pwndOnExpr;
        TPushButton*        m_pwndSave;
        TPushButton*        m_pwndSelCompVal;
        TString             m_strMoniker;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TEdFldTriggerDlg,TDlgBox)
};

#pragma CIDLIB_POPPACK



