//
// FILE NAME: CQCInst_NewCheckDlg.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/04/2002
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
//  This dialog is popped up if no previous installation is found. It lets
//  the user either confirm no previous install, or to find it for us, or
//  cancel.
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
//   CLASS: TCQCNewInstCheckDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------
class TCQCNewInstCheckDlg : public TDlgBox
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCNewInstCheckDlg();

        TCQCNewInstCheckDlg(const TCQCNewInstCheckDlg&) = delete;
        TCQCNewInstCheckDlg(TCQCNewInstCheckDlg&&) = delete;

        ~TCQCNewInstCheckDlg();


        // -------------------------------------------------------------------
        //  Unimplemented constructors and operators
        // -------------------------------------------------------------------
        TCQCNewInstCheckDlg& operator=(const TCQCNewInstCheckDlg&) = delete;
        TCQCNewInstCheckDlg& operator=(TCQCNewInstCheckDlg&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCQCInst::ENewInstRes eRunDlg
        (
                    TWindow&                wndOwner
            ,       TString&                strToFill
            ,       TCQCVerInstallInfo&     viiToFill
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
        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_pwndXXX
        //      Some typed pointers to important widgets that we have to interact with
        //      enough to want to avoid type casting. These are the ones that are part
        //      of the base dialog. The ones that come and go as pages are swapped in
        //      and out are dealt with on a temporary basis.
        //
        //  m_strPath
        //      We need a place to store the selected path until we can get into the
        //      caller's return parm.
        //
        //  m_viiToFill
        //      The version install info to fill, if the user steers us towards any.
        //      We store it here until we get it back to the caller's parm.
        // -------------------------------------------------------------------
        TPushButton*        m_pwndCancelButton;
        TPushButton*        m_pwndFindButton;
        TPushButton*        m_pwndNewButton;
        TString             m_strPathFound;
        TCQCVerInstallInfo  m_viiFound;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCNewInstCheckDlg,TDlgBox)
};

#pragma CIDLIB_POPPACK

