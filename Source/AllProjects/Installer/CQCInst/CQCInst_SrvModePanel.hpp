//
// FILE NAME: CQCInst_SrvModePanel.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/221/2009
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
//  This is the header for the info panel derived class that lets the user select the
//  startup mode for the app shell service.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TInstSrvModePanel
//  PREFIX: pan
// ---------------------------------------------------------------------------
class TInstSrvModePanel : public TInstInfoPanel
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TInstSrvModePanel() = delete;

        TInstSrvModePanel
        (
                    TCQCInstallInfo* const  pinfoCur
            ,       TMainFrameWnd* const    pwndParent
        );

        TInstSrvModePanel(const TInstSrvModePanel&) = delete;
        TInstSrvModePanel(TInstSrvModePanel&&) = delete;

        ~TInstSrvModePanel();


        // -------------------------------------------------------------------
        //  Public operator
        // -------------------------------------------------------------------
        TInstSrvModePanel& operator=(const TInstSrvModePanel&) = delete;
        TInstSrvModePanel& operator=(TInstSrvModePanel&&) = delete;


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
        //  Private data members
        //
        //  m_pwndXXX
        //      Typed pointers to some of our controls that we interact with
        //      enough not to want to cast them all the time. We don't own
        //      them.
        // -------------------------------------------------------------------
        TRadioButton*   m_pwndAutomatic;
        TRadioButton*   m_pwndAutoDelayed;
        TRadioButton*   m_pwndDisabled;
        TRadioButton*   m_pwndOnDemand;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TInstSrvModePanel,TInstInfoPanel)
};

#pragma CIDLIB_POPPACK


