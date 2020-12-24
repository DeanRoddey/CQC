//
// FILE NAME: CQCInst_WebSrvPanel.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/31/2004
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
//  the web server configuration info from the user.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TInstWebSrvPanel
//  PREFIX: pan
// ---------------------------------------------------------------------------
class TInstWebSrvPanel : public TInstInfoPanel
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TInstWebSrvPanel() = delete;

        TInstWebSrvPanel
        (
                    TCQCInstallInfo* const  pinfoCur
            ,       TMainFrameWnd* const    pwndParent
        );

        TInstWebSrvPanel(const TInstWebSrvPanel&) = delete;
        TInstWebSrvPanel(TInstWebSrvPanel&&) = delete;

        ~TInstWebSrvPanel();


        // -------------------------------------------------------------------
        //  Public operator
        // -------------------------------------------------------------------
        TInstWebSrvPanel& operator=(const TInstWebSrvPanel&) = delete;
        TInstWebSrvPanel& operator=(TInstWebSrvPanel&&) = delete;


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
        //      Typed pointers to some of our controls that we interact with
        //      enough not to want to cast them all the time. We don't own
        //      them.
        // -------------------------------------------------------------------
        TComboBox*      m_pwndAdminPort;
        TCheckBox*      m_pwndDoInsecure;
        TCheckBox*      m_pwndDoSecure;
        TComboBox*      m_pwndInsecurePort;
        TCheckBox*      m_pwndInstall;
        TComboBox*      m_pwndSecurePort;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TInstWebSrvPanel,TInstInfoPanel)
};

#pragma CIDLIB_POPPACK


