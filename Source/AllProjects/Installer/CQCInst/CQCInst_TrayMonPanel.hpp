//
// FILE NAME: CQCInst_ACSrvPanel.hpp
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
//  This is the header for the info panel derived class that handles getting the
//  Tray Monitor configuration info from the user. It used to be for a dedicated
//  app control program, which was later incorporated into the more general tray
//  monitor, hence the name.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TInstACSrvPanel
//  PREFIX: pan
// ---------------------------------------------------------------------------
class TInstACSrvPanel : public TInstInfoPanel
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TInstACSrvPanel() = delete;

        TInstACSrvPanel
        (
                    TCQCInstallInfo* const  pinfoCur
            ,       TMainFrameWnd* const    pwndParent
        );

        TInstACSrvPanel(const TInstACSrvPanel&) = delete;
        TInstACSrvPanel(TInstACSrvPanel&&) = delete;

        ~TInstACSrvPanel();


        // -------------------------------------------------------------------
        //  Public operator
        // -------------------------------------------------------------------
        TInstACSrvPanel& operator=(const TInstACSrvPanel&) = delete;
        TInstACSrvPanel& operator=(TInstACSrvPanel&&) = delete;


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
        //  m_bAutoStartAvail
        //      Task scheduling may not be available on some OS versions. So
        //      we test for it and set this on init, so that we don't have to
        //      keep checking.
        //
        //  m_pwndXXX
        //      Typed pointers to some of our controls that we interact with
        //      enough not to want to cast them all the time. We don't own
        //      them.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bAutoStartAvail;
        TCheckBox*          m_pwndAppCtrl;
        TCheckBox*          m_pwndAutoStart;
        TEntryField*        m_pwndBinding;
        TCheckBox*          m_pwndiTunes;
        TCheckBox*          m_pwndInstall;
        TComboBox*          m_pwndPort;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TInstACSrvPanel,TInstInfoPanel)
};

#pragma CIDLIB_POPPACK


