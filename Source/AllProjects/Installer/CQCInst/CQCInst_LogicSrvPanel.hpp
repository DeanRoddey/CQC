//
// FILE NAME: CQCInst_LogicSrvPanel.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/06/2009
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
//  the Logic Server configuration info from the user.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TInstLogicSrvPanel
//  PREFIX: pan
// ---------------------------------------------------------------------------
class TInstLogicSrvPanel : public TInstInfoPanel
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TInstLogicSrvPanel() = delete;

        TInstLogicSrvPanel
        (
                    TCQCInstallInfo* const  pinfoCur
            ,       TMainFrameWnd* const    pwndParent
        );

        TInstLogicSrvPanel(const TInstLogicSrvPanel&) = delete;
        TInstLogicSrvPanel(TInstLogicSrvPanel&&) = delete;

        ~TInstLogicSrvPanel();


        // -------------------------------------------------------------------
        //  Public operator
        // -------------------------------------------------------------------
        TInstLogicSrvPanel& operator=(const TInstLogicSrvPanel&) = delete;
        TInstLogicSrvPanel& operator=(TInstLogicSrvPanel&&) = delete;


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
        TCheckBox*      m_pwndInstall;
        TComboBox*      m_pwndPort;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TInstLogicSrvPanel,TInstInfoPanel)
};

#pragma CIDLIB_POPPACK


