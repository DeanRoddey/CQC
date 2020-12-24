//
// FILE NAME: CQCIntfEd_TmplHotKeysDlg_.hpp
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
//  This is the header for the CQCIntfEd_TmplHotKeysDlg.cpp file, which allows the
//  user to edit the hot key driven actions associated with a template.
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
//   CLASS: TTmplHotKeysDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------
class TTmplHotKeysDlg : public TDlgBox
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TTmplHotKeysDlg();

        ~TTmplHotKeysDlg();


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bRun
        (
            const   TWindow&                wndOwner
            , const TString&                strTmplBasePath
            ,       tCQCIntfEng::THKActList& colActions
            ,       tCQCKit::TCmdTarList&   colExtraTars
            , const TCQCUserCtx&            cuctxToUse
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
        TTmplHotKeysDlg(const TTmplHotKeysDlg&);
        tCIDLib::TVoid operator=(const TTmplHotKeysDlg&);


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bValidate();

        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );

        tCIDCtrls::EEvResponses eListHandler
        (
                    TListChangeInfo&        wnotEvent
        );

        tCIDLib::TVoid EditAction();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_colEdit
        //      WE make a copy of the hot keys to edit, and copy them back if the
        //      user saves.
        //
        //  m_pcolExtraTars
        //      The caller passes us any extra command targets he wants us to present
        //      when editing the hot key actions.
        //
        //  m_pcolOrg
        //      A pointer to the original content to check for changes on cancel.
        //
        //  m_pcuctxToUse
        //      The caller has to provide us with a user context so that we can do
        //      various things when the user edits any actions.
        //
        //  m_pwndXXX
        //      Some typed pointers we get to some of our child widgets we need to
        //      interact with on a typed basis.
        //
        //  m_strTmplBasePath
        //      The caller gives us the path of the template that is being edited. We
        //      have to set them when editing the actions, to support relative template
        //      paths.
        // -------------------------------------------------------------------
        tCQCIntfEng::THKActList     m_colEdit;
        tCQCKit::TCmdTarList*       m_pcolExtraTars;
        tCQCIntfEng::THKActList*    m_pcolOrg;
        const TCQCUserCtx*          m_pcuctxToUse;
        TStaticMultiText*           m_pwndActText;
        TPushButton*                m_pwndCancelButton;
        TPushButton*                m_pwndDelButton;
        TPushButton*                m_pwndEditButton;
        TCheckedMCListWnd*          m_pwndList;
        TPushButton*                m_pwndSaveButton;
        TString                     m_strTmplBasePath;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TTmplHotKeysDlg,TDlgBox)
};

#pragma CIDLIB_POPPACK



