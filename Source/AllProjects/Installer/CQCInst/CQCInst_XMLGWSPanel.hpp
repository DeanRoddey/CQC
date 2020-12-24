//
// FILE NAME: CQCInst_XMLGWSPanel.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/01/2004
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
//  This is the header for the info panel derived class that handles getting
//  the XML Gateway Server configuration info from the user.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TInstXMLGWSrvPanel
//  PREFIX: pan
// ---------------------------------------------------------------------------
class TInstXMLGWSrvPanel : public TInstInfoPanel
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TInstXMLGWSrvPanel() = delete;

        TInstXMLGWSrvPanel
        (
                    TCQCInstallInfo* const  pinfoCur
            ,       TMainFrameWnd* const    pwndParent
        );

        TInstXMLGWSrvPanel(const TInstXMLGWSrvPanel&) = delete;
        TInstXMLGWSrvPanel(TInstXMLGWSrvPanel&&) = delete;

        ~TInstXMLGWSrvPanel();


        // -------------------------------------------------------------------
        //  Public operator
        // -------------------------------------------------------------------
        TInstXMLGWSrvPanel& operator=(const TInstXMLGWSrvPanel&) = delete;
        TInstXMLGWSrvPanel& operator=(TInstXMLGWSrvPanel&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bPanelIsVisible() const final;

        tCIDLib::TBoolean bValidate
        (
                    tCIDCtrls::TWndId&      widFailed
            ,       TString&                strErrText
        )   final;

        tCIDLib::TVoid Entered() final;

        tCIDLib::TVoid ReactToChanges() final;

        tCIDLib::TVoid ReportPortUsage
        (
                    tCQCInst::TPortCntList& fcolPortCnts
        )   final;

        tCIDLib::TVoid SaveContents() final;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCreated() final;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_pwndXXX
        //      Typed pointers to some of our controls that we interact with
        //      enough not to want to cast them all the time. We don't own
        //      them.
        // -------------------------------------------------------------------
        TComboBox*      m_pwndAdminPort;
        TCheckBox*      m_pwndDoSecure;
        TCheckBox*      m_pwndDoInsecure;
        TCheckBox*      m_pwndInstall;
        TComboBox*      m_pwndInsecurePort;
        TComboBox*      m_pwndSecurePort;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TInstXMLGWSrvPanel,TInstInfoPanel)
};

#pragma CIDLIB_POPPACK

