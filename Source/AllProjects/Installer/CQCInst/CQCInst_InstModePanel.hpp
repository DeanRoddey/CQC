//
// FILE NAME: CQCInst_InstModePanel.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/30/2004
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
//  This is the header for the info panel derived class that let's the user select
//  the components he wants to install. The name is a bit misleading because this
//  used to be a list of install modes, instead of individual component selection.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TInstModePanel
//  PREFIX: pan
// ---------------------------------------------------------------------------
class TInstModePanel : public TInstInfoPanel
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TInstModePanel() = delete;

        TInstModePanel
        (
                    TCQCInstallInfo* const  pinfoCur
            ,       TMainFrameWnd* const    pwndParent
        );

        TInstModePanel(const TInstModePanel&) = delete;
        TInstModePanel(TInstModePanel&&) = delete;

        ~TInstModePanel();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TInstModePanel& operator=(const TInstModePanel&) = delete;
        TInstModePanel& operator=(TInstModePanel&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bValidate
        (
                    tCIDCtrls::TWndId&      widFailed
            ,       TString&                strErrText
        )   final;

        tCQCInst::ENavFlags eNavFlags() const final;

        tCIDLib::TVoid Entered() final;

        tCIDLib::TVoid SaveContents() final;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCreated() final;

        tCIDLib::TVoid ChildFocusChange
        (
            const   TWindow&                wndParent
            , const tCIDCtrls::TWndId       widChild
            , const tCIDLib::TBoolean       bGotFocus
        )   final;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCompState
        (
            const   tCQCInst::EInstComps    eSrcComp
            , const tCIDLib::TBoolean       bNew
        );

        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );

        tCIDCtrls::EEvResponses eListHandler
        (
                    TListChangeInfo&        wnotEvent
        );

        tCIDLib::TVoid LoadCompIndicators();

        const TString& strStatusText
        (
            const   tCIDLib::TBoolean       bOldState
            , const tCIDLib::TBoolean       bNewState
        );

        tCIDLib::TVoid SetCompState
        (
            const   tCQCInst::EInstComps    eTarComp
            , const tCIDLib::TBoolean       bNewState
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_pwndXXX
        //      Typed pointers to some of our controls that we interact with
        //      enough not to want to cast them all the time. We don't own
        //      them.
        //
        //  m_strXXX
        //      Some strings we use a lot so we preload them.
        // -------------------------------------------------------------------
        TMultiColListBox*   m_pwndComps;
        TStaticMultiText*   m_pwndCompDescr;
        TCheckBox*          m_pwndUsePrevOpts;
        const TString       m_strInstall;
        const TString       m_strNotInstalled;
        const TString       m_strUninstall;
        const TString       m_strUpgrade;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TInstModePanel,TInstInfoPanel)
};

#pragma CIDLIB_POPPACK


