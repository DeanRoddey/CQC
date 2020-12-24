//
// FILE NAME: CQCInst_IntroPanel.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/30/2016
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
//  This is the header for the info panel derived class that handles showing
//  the EULA to the user and getting a positive response.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TInstIntroPanel
//  PREFIX: pan
// ---------------------------------------------------------------------------
class TInstIntroPanel : public TInstInfoPanel
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TInstIntroPanel() = delete;

        TInstIntroPanel
        (
                    TCQCInstallInfo* const  pinfoCur
            ,       TMainFrameWnd* const    pwndParent
        );

        TInstIntroPanel(const TInstIntroPanel&) = delete;
        TInstIntroPanel(TInstIntroPanel&&) = delete;

        ~TInstIntroPanel();


        // -------------------------------------------------------------------
        //  Public operator
        // -------------------------------------------------------------------
        TInstIntroPanel& operator=(const TInstIntroPanel&) = delete;
        TInstIntroPanel& operator=(TInstIntroPanel&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bPanelIsVisible() const final;

        tCIDLib::TBoolean bValidate
        (
                    tCIDCtrls::TWndId&      widFailed
            ,       TString&                strErrText
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
        TCheckBox*          m_pwndDevices;
        TRadioButton*       m_pwndMaster;
        TRadioButton*       m_pwndNonMaster;
        TStaticMultiText*   m_pwndInstruct;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TInstIntroPanel,TInstInfoPanel)
};

#pragma CIDLIB_POPPACK


