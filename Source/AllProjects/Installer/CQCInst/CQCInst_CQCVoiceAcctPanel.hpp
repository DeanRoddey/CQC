//
// FILE NAME: CQCInst_CQCVoiceAcctPanel.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/02/2017
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
//  This is the header for the info panel derived class that allows the user to create
//  the account selected for use with the CQCVoice tray app. It's only shown if the
//  selected account doesn't exist. The account was set during the earlier CQCVoice config
//  panel.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TInstCQCVoiceAcctPanel
//  PREFIX: pan
// ---------------------------------------------------------------------------
class TInstCQCVoiceAcctPanel : public TInstInfoPanel
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TInstCQCVoiceAcctPanel() = delete;

        TInstCQCVoiceAcctPanel
        (
                    TCQCInstallInfo* const  pinfoCur
            ,       TMainFrameWnd* const    pwndParent
        );

        TInstCQCVoiceAcctPanel(const TInstCQCVoiceAcctPanel&) = delete;
        TInstCQCVoiceAcctPanel(TInstCQCVoiceAcctPanel&&) = delete;

        ~TInstCQCVoiceAcctPanel();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TInstCQCVoiceAcctPanel& operator=(const TInstCQCVoiceAcctPanel&) = delete;
        TInstCQCVoiceAcctPanel& operator=(TInstCQCVoiceAcctPanel&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bPanelIsVisible() const final;

        tCIDLib::TBoolean bValidate
        (
                    tCIDCtrls::TWndId&      widFailed
            ,       TString&                strErrText
        )   override;

        tCQCInst::ENavFlags eNavFlags() const final;


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
        //  m_bAcctDone
        //      Once they have successfully created the account, this is set. This
        //      let's us skip if they subsequently go back and forward again. It's mutable
        //      so that we can set it initially in bPanelIsVisible(), so that if it
        //      already exists, we never have to see the panel.
        //
        //  m_pwndXXX
        //      Some typed pointers to important widgets that we have to interact with
        //      enough to want to avoid type casting. We don't own them.
        // -------------------------------------------------------------------
        mutable tCIDLib::TBoolean   m_bAcctDone;
        TPushButton*                m_pwndCreate;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TInstCQCVoiceAcctPanel,TInstInfoPanel)
};

#pragma CIDLIB_POPPACK

