//
// FILE NAME: CQCInst_ClientPanel.hpp
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
//  the client configuration info from the user.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TInstClientPanel
//  PREFIX: pan
// ---------------------------------------------------------------------------
class TInstClientPanel : public TInstInfoPanel
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TInstClientPanel() = delete;

        TInstClientPanel
        (
                    TCQCInstallInfo* const  pinfoCur
            ,       TMainFrameWnd* const    pwndParent
        );

        TInstClientPanel(const TInstClientPanel&) = delete;
        TInstClientPanel(TInstClientPanel&&) = delete;

        ~TInstClientPanel();


        // -------------------------------------------------------------------
        //  Public operator
        // -------------------------------------------------------------------
        TInstClientPanel& operator=(const TInstClientPanel&) = delete;
        TInstClientPanel& operator=(TInstClientPanel&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bPanelIsVisible() const final;

        tCIDLib::TVoid Entered() final;

        tCIDLib::TVoid SaveContents() final;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCreated() final;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtuao methods
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
        TCheckBox*      m_pwndAdmin;
        TPushButton*    m_pwndAllOff;
        TPushButton*    m_pwndAllOn;
        TCheckBox*      m_pwndCmdPrompt;
        TCheckBox*      m_pwndDrvHarness;
        TCheckBox*      m_pwndIntfDes;
        TCheckBox*      m_pwndIntfView;
        TCheckBox*      m_pwndMacroWS;
        TCheckBox*      m_pwndMediaRepoMgr;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TInstClientPanel,TInstInfoPanel)
};

#pragma CIDLIB_POPPACK



