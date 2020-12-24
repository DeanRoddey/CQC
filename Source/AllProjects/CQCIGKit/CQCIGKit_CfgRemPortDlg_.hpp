//
// FILE NAME: CQCIGKit_CfgRemPortDlg_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/15/2005
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
//  This is the header for the CQCGKit_CfgRemPortDlg.cpp file, which implements a dialog
//  that lets an admin enable access to a CQC remote port server in one of the development
//  tools.
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
//   CLASS: TCfgRemPortDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------
class TCfgRemPortDlg : public TDlgBox
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCfgRemPortDlg();

        TCfgRemPortDlg(const TCfgRemPortDlg&) = delete;

        ~TCfgRemPortDlg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCfgRemPortDlg& operator=(const TCfgRemPortDlg&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bRunDlg
        (
            const   TWindow&                wndOwner
            ,       tCIDLib::TBoolean&      bEnable
            ,       TIPEndPoint&            ipepInOut
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
        tCIDLib::TBoolean bValidate
        (
            const   tCIDLib::TBoolean       bTest
        );

        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bEnable
        //      The incoming enable flag is put here.
        //
        //  m_ipepCur
        //      The incoming endpoint info is stored here, and we update it
        //      during the validation.
        //
        //  m_pwndXXX
        //      Various widgets we get typed pointers to since we interact
        //      with them a lot. We don't own them.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bEnable;
        TIPEndPoint         m_ipepCur;
        TEntryField*        m_pwndAddr;
        TPushButton*        m_pwndCancel;
        TPushButton*        m_pwndCloseAll;
        TCheckBox*          m_pwndEnable;
        TEntryField*        m_pwndPort;
        TPushButton*        m_pwndSave;
        TPushButton*        m_pwndTest;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCfgRemPortDlg,TDlgBox)
};

#pragma CIDLIB_POPPACK

