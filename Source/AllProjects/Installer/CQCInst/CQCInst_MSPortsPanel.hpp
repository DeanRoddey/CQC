//
// FILE NAME: CQCInst_MSPortsPanel.hpp
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
//  This is the header for the info panel derived class that lets the user
//  select the ports to use for master server processes.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TInstMSPortsPanel
//  PREFIX: pan
// ---------------------------------------------------------------------------
class TInstMSPortsPanel : public TInstInfoPanel
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TInstMSPortsPanel() = delete;

        TInstMSPortsPanel
        (
                    TCQCInstallInfo* const  pinfoCur
            ,       TMainFrameWnd* const    pwndParent
        );

        TInstMSPortsPanel(const TInstMSPortsPanel&) = delete;
        TInstMSPortsPanel(TInstMSPortsPanel&&) = delete;

        ~TInstMSPortsPanel();


        // -------------------------------------------------------------------
        //  Public operator
        // -------------------------------------------------------------------
        TInstMSPortsPanel& operator=(const TInstMSPortsPanel&) = delete;
        TInstMSPortsPanel& operator=(TInstMSPortsPanel&&) = delete;


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
        //  Private data members
        //
        //  m_pwndXXX
        //      Typed pointers to some of our controls that we interact with
        //      enough not to want to cast them all the time. We don't own
        //      them.
        // -------------------------------------------------------------------
        TComboBox*  m_pwndClS;
        TComboBox*  m_pwndCS;
        TComboBox*  m_pwndDS;
        TComboBox*  m_pwndEvents;
        TComboBox*  m_pwndNS;
        TComboBox*  m_pwndLS;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TInstMSPortsPanel,TInstInfoPanel)
};

#pragma CIDLIB_POPPACK



