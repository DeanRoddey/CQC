//
// FILE NAME: CQCGKit_ChangePWDlg_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/19/2002
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
//  This is the header for the CQCGKit_ChangePWDlg.cpp file, which implements
//  the TCQCChangePWDlg class. This dialog does a standard CQC password
//  change interaction with the CQC security server. It will only change the
//  password of the currently logged on user, and the current password must
//  be provided in order to protected against someone 'stealing' an account
//  via an unattended machine.
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
//   CLASS: TCQCChangePWDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------
class TCQCChangePWDlg : public TDlgBox
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCChangePWDlg();

        TCQCChangePWDlg(const TCQCChangePWDlg&) = delete;
        TCQCChangePWDlg(TCQCChangePWDlg&&) = delete;

        ~TCQCChangePWDlg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCChangePWDlg& operator=(const TCQCChangePWDlg&) = delete;
        TCQCChangePWDlg& operator=(TCQCChangePWDlg&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid Run
        (
            const   TWindow&                wndOwner
            , const TString&                strUserName
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
        tCIDLib::TBoolean bAttemptChangePW();

        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );

        tCIDCtrls::EEvResponses eEFHandler
        (
                    TEFChangeInfo&          wnotEvent
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_pwndXXXX
        //      Some typed pointers to our widgets. We don't own them, we just
        //      want to avoid lots of downcasting when we use them.
        //
        //  m_strUserName
        //      The caller provides us with the user name.
        // -------------------------------------------------------------------
        TPushButton*    m_pwndCancel;
        TPushButton*    m_pwndChange;
        TEntryField*    m_pwndNewPW1;
        TEntryField*    m_pwndNewPW2;
        TEntryField*    m_pwndOldPW;
        TRadioButton*   m_pwndCQCPW;
        TRadioButton*   m_pwndWebPW;
        TString         m_strUserName;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCChangePWDlg,TDlgBox)
};

#pragma CIDLIB_POPPACK


