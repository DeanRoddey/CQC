//
// FILE NAME: CQCIGKit_EditKeyMapDlg_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/08/2001
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
//  This is the header for the CQCGKit_KeyMapsDlg.cpp file, which implements the dialog
//  box that allows the user to add, remove, or edit key mappings. Key mappings allow the
//  user to assign an action (either a device field read/write or macro invocation) to a
//  function key. We support a fixed set of keys (F2..F9 and some of the media keys)
//  either unshifted or with combinations of Shift and Ctrl-Shift.
//
//  We display a list box with one set of key entries, and swap the contents in and out
//  as the user checks off shift state check boxes.
//
//  This dailog supports two modes. There is an edit mode, which allows the user to edit
//  the entries, and an execute mode which just offers Run and Close buttons and allows
//  the user to execute the mappings. This latter mode is used as a 'cheat sheet' from
//  the blanker window or within the IV, to help them if they forget a mapping. The edit
//  mode is only allowed within the admin client.
//
//  In the edit mode, there is a static multi-line at the bottom that shows the cmd steps
//  that make up the action. In the run mode, the dialog box is sized down to cover that,
//  since it doesn't need to be seen.
//
//  The caller indicates the user account whose key mappings we are getting if in editing
//  mode, so that we can display it. Otherwise, it's only for the account of the current
//  user.
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
//   CLASS: TCQCKeyMapDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------
class TCQCKeyMapDlg : public TDlgBox
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCKeyMapDlg() = delete;

        TCQCKeyMapDlg
        (
            const   tCQCIGKit::EKMActModes  eMode
            , const TCQCUserCtx&            cuctxToUse
        );

        TCQCKeyMapDlg(const TCQCKeyMapDlg&) = delete;

        ~TCQCKeyMapDlg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCKeyMapDlg& operator=(const TCQCKeyMapDlg&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bRun
        (
            const   TWindow&                wndOwner
            , const TString&                strAccount
            ,       TCQCKeyMap&             kmToEdit
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

        tCIDCtrls::EEvResponses eLBHandler
        (
                    TListChangeInfo&        wnotEvent
        );

        tCIDLib::TVoid EditItem();

        tCIDLib::TVoid ExecItem();

        tCIDLib::TVoid FillList();

        tCIDLib::TVoid ShowDetails();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_crtsStd
        //      A standard runtime values object that we'll need any time we have to
        //      format a command config object. To avoid just creating and destroying
        //      it over and over we create one. We know that key maps only use the std
        //      runtime values.
        //
        //  m_cuctxToUse
        //      The caller has to provide us with a user context for us to use in various
        //      operations.
        //
        //  m_eMode
        //      Indicates the mode that the dialog will run in. See the file comments above.
        //
        //  m_kmToEdit
        //      We make a copy of the caller's key map data which we edit. If the user
        //      saves, then we copy our edited version back.
        //
        //  m_pkmOrg
        //      A pointer to the original incoming stuff, so we can check for changes.
        //
        //  m_pwndXXX
        //      Type pointers to some of our widgets, for convenience.
        //
        //  m_strAccount
        //      We get passed the name of the user account whose mappings we are getting.
        //      Only used if in editing mode, when we put this in the title bar. If not in
        //      editing mode, the caller will generally just pass an empty string.
        // -------------------------------------------------------------------
        TCQCCmdRTVSrc           m_crtsStd;
        const TCQCUserCtx&      m_cuctxToUse;
        tCQCIGKit::EKMActModes  m_eMode;
        TCQCKeyMap              m_kmToEdit;
        const TCQCKeyMap*       m_pkmOrg;
        TPushButton*            m_pwndCloseButton;
        TCheckBox*              m_pwndCtrlShift;
        TPushButton*            m_pwndDeleteButton;
        TMultiEdit*             m_pwndDetails;
        TPushButton*            m_pwndEditButton;
        TCheckedMCListWnd*      m_pwndList;
        TPushButton*            m_pwndSaveButton;
        TCheckBox*              m_pwndShift;
        TString                 m_strAccount;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCKeyMapDlg,TDlgBox)
};

#pragma CIDLIB_POPPACK

