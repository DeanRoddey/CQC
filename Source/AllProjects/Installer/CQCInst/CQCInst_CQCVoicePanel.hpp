//
// FILE NAME: CQCInst_CQCVoicePanel.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/30/2017
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
//  CQCVoice tray app configuration info from the user.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TInstCQCVoicePanel
//  PREFIX: pan
// ---------------------------------------------------------------------------
class TInstCQCVoicePanel : public TInstInfoPanel
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TInstCQCVoicePanel() = delete;

        TInstCQCVoicePanel
        (
                    TCQCInstallInfo* const  pinfoCur
            ,       TMainFrameWnd* const    pwndParent
        );

        TInstCQCVoicePanel(const TInstCQCVoicePanel&) = delete;
        TInstCQCVoicePanel(TInstCQCVoicePanel&&) = delete;

        ~TInstCQCVoicePanel();


        // -------------------------------------------------------------------
        //  Public operator
        // -------------------------------------------------------------------
        TInstCQCVoicePanel& operator=(const TInstCQCVoicePanel&) = delete;
        TInstCQCVoicePanel& operator=(TInstCQCVoicePanel&&) = delete;


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
        TCheckBox*          m_pwndAutoStart;
        TCheckBox*          m_pwndInstall;
        TComboBox*          m_pwndPort;
        TEntryField*        m_pwndPassword;
        TComboBox*          m_pwndReplyStyle;
        TEntryField*        m_pwndUserName;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TInstCQCVoicePanel,TInstInfoPanel)
};

#pragma CIDLIB_POPPACK


