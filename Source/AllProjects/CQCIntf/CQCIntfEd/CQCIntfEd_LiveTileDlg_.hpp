//
// FILE NAME: CQCIntfEd_TLiveTileDlg_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/19/2014
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
//  This is the header for a configuration dialog for live tile widgets. Everything
//  but the two source fields is done here.
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
//  CLASS: TLiveTileDlg
// PREFIX: wnd
// ---------------------------------------------------------------------------
class TLiveTileDlg : public TDlgBox
{
    public  :
        // -------------------------------------------------------------------
        //  Class types
        // -------------------------------------------------------------------
        typedef TCQCIntfLiveTile::TTmplList TTileInfo;


        // -------------------------------------------------------------------
        // Constructors and destructor
        // -------------------------------------------------------------------
        TLiveTileDlg();

        TLiveTileDlg(const TLiveTileDlg&) = delete;

        ~TLiveTileDlg();


        // -------------------------------------------------------------------
        //  Public opreators
        // -------------------------------------------------------------------
        TLiveTileDlg& operator=(const TLiveTileDlg&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bRun
        (
                    TWindow&                wndOwner
            ,       TTileInfo&              colEdit
            , const TString&                strFld1
            , const TString&                strFld2
            , const TCQCUserCtx&            cuctxToUse
            , const TString&                strParentTmplPath
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCreated() override;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bValidate(TString& strReason);

        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );

        tCIDCtrls::EEvResponses eComboHandler
        (
                    TListChangeInfo&        wnotEvent
        );

        tCIDCtrls::EEvResponses eEFHandler
        (
                    TEFChangeInfo&          wnotEvent
        );

        tCIDCtrls::EEvResponses eLBHandler
        (
                    TListChangeInfo&        wnotEvent
        );

        tCIDLib::TVoid LoadTmplInfo();

        tCIDLib::TVoid LoadWidgets();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_colEdit
        //      A copy fo the data that we edit.
        //
        //  m_pcolOrg
        //      The original data we got, so that we can check for changes or revert to
        //      the original content. And we put back changes here if the user commits.
        //
        //  m_pcuctxToUse
        //      The caller has to provide us with a user context so that we can do
        //      various things when the user edits any actions.
        //
        //  m_pwndXXX
        //      We get some typed pointers to some of our widgets, to avoid
        //      downcasting all over the place. We don't own them, we just
        //      reference them.
        //
        //  m_strFld1
        //  m_strFld2
        //      We don't edit these, but we get them so that we can do appropriate
        //      validation. We want to make sure that they don't configure an expr
        //      for a field that hasn't been set.
        //
        //  m_strParPath
        //      The caller provides us with the parent template path so that we can let
        //      the user select relative template paths.
        // -------------------------------------------------------------------
        TTileInfo           m_colEdit;
        TTileInfo*          m_pcolOrg;
        const TCQCUserCtx*  m_pcuctxToUse;
        TPushButton*        m_pwndAddTmpl;
        TPushButton*        m_pwndCancel;
        TPushButton*        m_pwndDelTmpl;
        TComboBox*          m_pwndFld1Comp;
        TComboBox*          m_pwndFld2Comp;
        TEntryField*        m_pwndFld1Val;
        TEntryField*        m_pwndFld2Val;
        TComboBox*          m_pwndLogOp;
        TPushButton*        m_pwndSave;
        TListBox*           m_pwndTmpls;
        TPushButton*        m_pwndTmplDn;
        TPushButton*        m_pwndTmplUp;
        TEntryField*        m_pwndVarName;
        TEntryField*        m_pwndVarVal;
        TString             m_strFld1;
        TString             m_strFld2;
        TString             m_strParPath;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TLiveTileDlg, TDlgBox)
};

#pragma CIDLIB_POPPACK



