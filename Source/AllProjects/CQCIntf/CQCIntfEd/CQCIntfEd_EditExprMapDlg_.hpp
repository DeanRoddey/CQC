//
// FILE NAME: CQCIntfEd_EditExprMapDlg_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 5/8/2016
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
//  This is the header for the CQCIntfEd_EditExprMapDlg.cpp file, which lets the user
//  edit the non-default mappings for the mapped image widget, and potentially other
//  stuff later that require a list of named expressions.
//
//  We do this buy editing the stuff in place in the list. We use the standard IPE for
//  the name and value. We have to implement the MIPEIntf mixin but we don't have to
//  actually override anything since we never allow the user to enter anything invalid,
//  and the only restraint is that the name can't be empty, which we handle via a limit
//  on the attribute we pass to the IPE. But we still have to implement it since we have
//  to pass the in place editor a pointer to something that implements it. He still gets
//  the base mixin class functionality this way.
//
//  <TBD> Honestly we could have done less work to just use the actually mixin and let
//  the multi-column list control do most of the work. All we'd have to do is set up
//  the attribute appropriately. Oh well, for later.
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
//   CLASS: TEditExprMapDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------
class TEditExprMapDlg : public TDlgBox, public MIPEIntf
{
    public :
        // -------------------------------------------------------------------
        //  For brevity
        // -------------------------------------------------------------------
        typedef TCQCIntfMappedImgBase::TKeyList     TExprList;


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TEditExprMapDlg();

        TEditExprMapDlg(const TEditExprMapDlg&) = delete;

        ~TEditExprMapDlg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TEditExprMapDlg& operator=(const TEditExprMapDlg&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bRun
        (
            const   TWindow&                wndOwner
            , const TString&                strTitle
            , const TString&                strInstruct
            ,       TExprList&              colList
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
        tCIDLib::TBoolean bCollectResult
        (
                    TExprList&              colToFill
            , const tCIDLib::TBoolean       bShowErrs
        );

        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );

        tCQCKit::EExprTypes eGetExprType
        (
            const   tCIDLib::TCard4         c4Index
            ,       tCQCKit::EStatements&   eStmt
        )   const;

        tCIDCtrls::EEvResponses eListHandler
        (
                    TListChangeInfo&        wnotEvent
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4NextId
        //      For in place editing purposes, we assign a unique id to each item we
        //      put into the list box.
        //
        //  m_pcolEdit
        //      A pointer to the original data. We use it to load up the incoming and
        //      to store the content if the user saves. Since we don't put the data back
        //      until the user saves and we validate the data, we don't need a separate
        //      edit version.
        //
        //  m_pwndXXX
        //      Some typed pointers we get to some of our child widgets we need to
        //      interact with on a typed basis.
        //
        //  m_strBoolNo
        //  m_strBoolYes
        //      We preload these because we use them a lot.
        //
        //  m_strInstruct
        //  m_strTitle
        //      This is a potentially reusable dialog so we let the caller pass us title
        //      and instruction text.
        //
        //  m_strRegEx
        //      We pre-load the translation of the reg ex expression type, since we use
        //      it in comparisons a lot. If not this, then it's one of the statement
        //      types.
        // -------------------------------------------------------------------
        tCIDLib::TCard4     m_c4NextId;
        TExprList*          m_pcolEdit;
        TPushButton*        m_pwndAddButton;
        TPushButton*        m_pwndCancelButton;
        TPushButton*        m_pwndDnButton;
        TStaticMultiText*   m_pwndInstruct;
        TMultiColListBox*   m_pwndList;
        TPushButton*        m_pwndDeleteAllButton;
        TPushButton*        m_pwndDeleteButton;
        TPushButton*        m_pwndSaveButton;
        TPushButton*        m_pwndUpButton;
        const TString       m_strBoolNo;
        const TString       m_strBoolYes;
        TString             m_strInstruct;
        const TString       m_strRegEx;
        TString             m_strTitle;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TEditExprMapDlg,TDlgBox)
};

#pragma CIDLIB_POPPACK
