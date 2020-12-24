//
// FILE NAME: CQCGKit_LogonDlg_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/02/2002
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
//  This is the header for the CQCGKit_LogonDlg.cpp file, which implements
//  the TCQCLogonDlg class. This dialog does a standard CQC logon, getting
//  a user name and password from the user and doing the logon sequence with
//  the security server. If it works, it will return a filled in CQC security
//  token to the client app, which can use it to access needed resources.
//
//  It also returns a copy of the user'a account info. This object will have
//  all password information removed, so it's just general information that
//  would not provide any benefit to a cracker. They already have to have the
//  user name and password in order to log in, and with that they can look at
//  their own account information via the GUI anyway.
//
//  If the user bails, it returns false, which means the client should
//  exit without any action.
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
//   CLASS: TCQCLogonDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------
class TCQCLogonDlg : public TDlgBox
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCLogonDlg();

        TCQCLogonDlg(const TCQCLogonDlg&) = delete;
        TCQCLogonDlg(TCQCLogonDlg&&) = delete;

        ~TCQCLogonDlg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCLogonDlg& operator=(const TCQCLogonDlg&);
        TCQCLogonDlg& operator=(TCQCLogonDlg&&);


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bRunDlg
        (
            const   TWindow&                wndOwner
            ,       TCQCSecToken&           sectToFill
            ,       TCQCUserAccount&        uaccToFill
            , const tCQCKit::EUserRoles     eMinRole
            , const TString&                strAppTitle
            , const tCIDLib::TBoolean       bNoEnvLogon
            , const TString&                strIconName
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
        tCIDLib::TBoolean bAttemptLogon();

        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );

        tCIDLib::TBoolean bDoRegDialog
        (
            const   TWindow&                wndOwner
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_eMinRole
        //      The minimum user account role we'll allow to log in.
        //
        //  m_eMode
        //      The logon mode that we are doing.
        //
        //  m_psectTarget
        //      This is the security token that we need to fill in if the
        //      login goes ok. We need a pointer to it so that we can see
        //      it from our other methods during event handling.
        //
        //  m_pwndXXXX
        //      Some typed pointers to our widgets. We don't own them, we just
        //      want to avoid lots of downcasting when we use them.
        //
        //  m_puaccTarget
        //      This is the user account object we fill in if the logon is
        //      successful.
        //
        //  m_strAppTitle
        //      The caller tells us what his title is, which we can use in
        //      any popups.
        //
        //  m_strCurLogon
        //      If this is a re-logon, then this is where we store the current
        //      logon account name until we can get it loaded.
        //
        //  m_strIconName
        //      The calling program tells us what icon to use.
        // -------------------------------------------------------------------
        tCQCKit::EUserRoles     m_eMinRole;
        tCQCGKit::ELogonModes   m_eMode;
        TCQCSecToken*           m_psectTarget;
        TPushButton*            m_pwndCancelButton;
        TPushButton*            m_pwndLoginButton;
        TEntryField*            m_pwndLoginName;
        TEntryField*            m_pwndPassword;
        TCQCUserAccount*        m_puaccTarget;
        TString                 m_strAppTitle;
        TString                 m_strCurLogon;
        TString                 m_strIconName;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCLogonDlg,TDlgBox)
};

#pragma CIDLIB_POPPACK


