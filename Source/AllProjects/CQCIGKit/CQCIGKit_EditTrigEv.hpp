//
// FILE NAME: CQCIGKit_EditTrigEv.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/27/2016
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
//  This is the header for the CQCGKit_EditTrigEv.cpp file. There is more than one place where
//  we need to be able to edit a triggered event. The admin client does them via one his
//  tabbed pages. But we also have triggered events in templates and we need to be able to
//  edit those via a dialog box.
//
//  So we break this functionality out into a specialized window that can be used in either
//  situation. The tab creates one of these as his client area contents and the dialog loads
//  this content into himself. In the case of the client, the Save/Close buttons have to be
//  removed since they are not used there.
//
//  We define the wrapper dialog as well, to keep it all hidden away here. The caller
//  tells us which way he's using the window.
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
//  CLASS: TEditTrigEvWnd
// PREFIX: wnd
// ---------------------------------------------------------------------------
class CQCIGKITEXPORT  TEditTrigEvWnd : public TWindow
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TEditTrigEvWnd() = delete;

        TEditTrigEvWnd
        (
            const   tCIDLib::TBoolean       bDlgMode
        );

        TEditTrigEvWnd(const TEditTrigEvWnd&) = delete;

        ~TEditTrigEvWnd();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        tCIDLib::TVoid operator=(const TEditTrigEvWnd&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid CreateTEvEd
        (
            const   TWindow&                wndParent
            , const tCIDCtrls::TWndId       widTab
            ,       TCQCTrgEvent&           csrcEdit
            ,       tCQCKit::TCmdTarList* const pcolExtraTars
            , const tCIDCtrls::EWndThemes   eTheme
            , const TCQCUserCtx&            cuctxToUse
        );

        tCIDLib::TVoid CommitChanges();

        tCIDLib::ESaveRes eValidate
        (
                    TString&                strErr
        );


    protected  :
        // -------------------------------------------------------------------
        //  Protected, inherited method
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCreated() override;

        tCIDLib::TVoid ChildFocusChange
        (
            const   TWindow&                wndParent
            , const tCIDCtrls::TWndId       widChild
            , const tCIDLib::TBoolean       bGotFocus
        )   override;

        tCIDCtrls::ECtxMenuOpts eShowContextMenu
        (
            const   TPoint&                 pntAt
            , const tCIDCtrls::TWndId       widSrc
        )   override;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TCard4 c4FindFilterIndex
        (
            const   tCIDCtrls::TWndId       widChild
        )   const;

        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotClick
        );

        tCIDCtrls::EEvResponses eLBHandler
        (
                    TListChangeInfo&        wnotEvent
        );

        tCIDLib::TVoid NewFilterType
        (
            const   tCIDCtrls::TWndId       widCombo
        );

        tCIDLib::TVoid ShowFilterHelp
        (
            const   tCQCKit::ETEvFilters    eType
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_apwndXXX
        //      The child controls for the filters are all redundant so we put them into
        //      arrays so that we can process filter stuff by just knowing the filter index.
        //
        //  m_bDlgMode
        //      The caller tells us if we are being used in the dialog scenario or admin
        //      interface scenario.
        //
        //  m_cuctxToUse
        //      The caller has to provide us with a security context so that we can edit
        //      the action if needed.
        //
        //  m_eTheme
        //      We can be embedded in a dialog or a regular window. The caller tells us
        //      what theme to apply to the dialog description that defines our controls.
        //
        //  m_pcolExtraTars
        //      The caller provides a list of extra targets to make available in the action
        //      editor. Generally it's widgets when this is being used to edit a template's
        //      triggered events. This can be null if they have no extras.
        //
        //  m_pcsrcEdit
        //  m_csrcOrg
        //      We keep a pointer to the caller's data and we make a copy so that we can
        //      check for changes.
        //
        //  m_pwndXXX
        //      Some pointers to widgets that we want to deal with via their real types.
        //      We don't own them. Some are in the m_apwndXXX members.
        //
        //  m_strPrefTxt_XXX
        //      Some strings that we pre-load and swap into the field value prefix control
        //      depending on what type of filter is selected.
        // -------------------------------------------------------------------
        TStaticText*            m_apwndCompPrefs[kCQCKit::c4MaxTEvFilters];
        TCheckBox*              m_apwndCompRXs[kCQCKit::c4MaxTEvFilters];
        TEntryField*            m_apwndCompVals[kCQCKit::c4MaxTEvFilters];
        TStaticText*            m_apwndFldPrefs[kCQCKit::c4MaxTEvFilters];
        TCheckBox*              m_apwndFldRXs[kCQCKit::c4MaxTEvFilters];
        TEntryField*            m_apwndFldVals[kCQCKit::c4MaxTEvFilters];
        TCheckBox*              m_apwndNegates[kCQCKit::c4MaxTEvFilters];
        TComboBox*              m_apwndTypes[kCQCKit::c4MaxTEvFilters];
        tCIDLib::TBoolean       m_bDlgMode;
        tCQCKit::TCmdTarList*   m_pcolExtraTars;
        TCQCTrgEvent            m_csrcOrg;
        TCQCUserCtx             m_cuctxToUse;
        tCIDCtrls::EWndThemes   m_eTheme;
        TCQCTrgEvent*           m_pcsrcEdit;
        TPushButton*            m_pwndEditButton;
        TStaticMultiText*       m_pwndFilterInstr;
        TComboBox*              m_pwndLogOp;
        TCheckBox*              m_pwndLogRuns;
        TCheckBox*              m_pwndSerialize;
        TEntryField*            m_pwndTitle;


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TEditTrigEvWnd, TWindow)
};


// ---------------------------------------------------------------------------
//   CLASS: TEditTrigEvDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------
class CQCIGKITEXPORT TEditTrigEvDlg : public TDlgBox
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TEditTrigEvDlg();

        TEditTrigEvDlg(const TEditTrigEvDlg&) = delete;

        ~TEditTrigEvDlg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TEditTrigEvDlg& operator=(const TEditTrigEvDlg&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bRunDlg
        (
            const   TWindow&                wndOwner
            ,       TCQCTrgEvent&           csrcEdit
            ,       tCQCKit::TCmdTarList* const pcolExtraTars
            , const TCQCUserCtx&            cuctxToUse
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


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_csrcEdit
        //      The command source (triggered event) we are editing.
        //
        //  m_cuctxToUse
        //      The caller has to provide us with a user context so that we can do what we
        //      need to do.
        //
        //  m_pcolExtraTars
        //      The caller provides a list of extra targets to make available in the action
        //      editor. Generally it's widgets when this is being used to edit a template's
        //      triggered events. This can be null if they have no extras.
        //
        //  m_pwndXXXX
        //      Some typed pointers to our widgets. We don't own them, we just
        //      want to avoid lots of downcasting when we use them.
        //
        //  m_pwndEditor
        //      We create one of the above editors and position at the top.
        // -------------------------------------------------------------------
        TCQCTrgEvent            m_csrcEdit;
        TCQCUserCtx             m_cuctxToUse;
        tCQCKit::TCmdTarList*   m_pcolExtraTars;
        TPushButton*            m_pwndCancel;
        TEditTrigEvWnd*         m_pwndEditor;
        TPushButton*            m_pwndSave;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TEditTrigEvDlg,TDlgBox)
};
#pragma CIDLIB_POPPACK
