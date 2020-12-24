//
// FILE NAME: CQCTreeBrws_DrvWizDlg_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/11/2015
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
//  This is the header for the CQCIGKit_DrvWizDlgDlg.cpp file, which walks the user
//  through the process of providing the information required to install a driver.
//  This is driven by the driver manifest file.
//
//  It consists of a set of pages, each of which we implement as a tab, and present
//  the wizard as a tabbed window. We just control the order of going through the
//  tabs, we don't let the user move randomly through them. The page windows are
//  internall to the cpp file since we don't need to expose them, even internally
//  here within this facility. They all derive from the base tab window class
//  defined by the tabbed window control.
//
//  We receive a driver object config object into which has already been parsed the
//  manifest info. So the base class is filled in the configuration description
//  that indicates the info we need to get. Upon successful return, we will have
//  filled in the derived class' stuff with the actual data.
//
//  We handle tab notifications, one of which allows us to decide whether we will
//  let the user move off of the current tab. We use this both to insure that they
//  go through the tabs in order and to insure that we don't let them go off of a
//  tab that has errors on it.
//
//  This is down here, insteadof in, say, CQCIGKit, because we want to have access
//  to it from the device browser object, so that we can handle driver installation
//  internally without having to ask client code to do it. We expose it for anyone
//  else who needs it (such as the driver IDEs.)
//
// CAVEATS/GOTCHAS:
//
//  1)  If the caller doesn't provide a list of ports to expose, we just query the
//      locally available ports. If a real driver is being installed, the caller
//      should provide us with a list of ports available on the actual target
//      machine where the driver is being installed.
//
// LOG:
//
//  $Log$
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TCQCDrvWizDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------
class TCQCDrvWizDlg : public TDlgBox
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCDrvWizDlg() = delete;

        TCQCDrvWizDlg
        (
            const   TString&                strHost
            , const tCIDLib::TBoolean       bReconfigMode
            , const TCQCSecToken&           sectUser
        );

        TCQCDrvWizDlg(const TCQCDrvWizDlg&) = delete;
        TCQCDrvWizDlg(TCQCDrvWizDlg&&) = delete;

        ~TCQCDrvWizDlg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCDrvWizDlg& operator=(const TCQCDrvWizDlg&) = delete;
        TCQCDrvWizDlg& operator=(TCQCDrvWizDlg&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bRun
        (
            const   TWindow&                wndOwner
            ,       TCQCDriverObjCfg&       cqcdcEdit
            , const tCIDLib::TStrList&      colMonikers
            , const tCIDLib::TStrList* const pcolPorts
        );

        const TString& strHost() const;


    protected :
        // -------------------------------------------------------------------
        //  Protected inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCreated() final;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bMoveForward();

        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );

        tCIDCtrls::EEvResponses eTabHandler
        (
                    TTabEventInfo&          wnotEvent
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bAllowTabMove
        //      We prevent the user from changing tabs. However, that same notification
        //      gets sent even if we programmatically ask to change them, which will
        //      try to prevent that from happening as well. So this is set while we
        //      move, to prevent our tab handler from denying the change.
        //
        //  m_bReconfigMode
        //      We can be used to do initial config for loading a driver, or
        //      reconfiguration. This tells us which mode. In the reconfig
        //      mode, we don't allow the moniker to be changed, and might
        //      prevent some other things potentially later.
        //
        //  m_c4FirstPrompt
        //  m_c4LastPrompt
        //      The indices of the first/last prompt panels, if any. If none,
        //      they will be set to kCIDLib::c4MaxCard.
        //
        //  m_c4Summary
        //      The index of the summary panel
        //
        //  m_colPorts
        //      The caller can provide us with a list of ports to make available.
        //      In the admin client, it will provide us with a list of ports not
        //      currently used on the target host. In the IDE, it can just pass a
        //      null pointer and we'll load it with the local available ports.
        //
        //  m_pcolMonikers
        //      We get a list of existing monikers, so that we can check for dups. In
        //      the IDE, it may not provide any since it doesn't have to register the
        //      driver in the name server anyway. In the admin client it should provide
        //      the monikers for the configured drivers.
        //
        //  m_pwndXXX
        //      Some typed pointers we get to some of our child widgets we need to
        //      interact with on a typed basis.
        //
        //  m_cqcdcManifest
        //      The original manifest information.
        //
        //  m_sectUser
        //      We need a security token, so the caller has to provide one.
        //
        //  m_strHost
        //      The host we were told to install to.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean           m_bAllowTabMode;
        tCIDLib::TBoolean           m_bReconfigMode;
        tCIDLib::TCard4             m_c4FirstPrompt;
        tCIDLib::TCard4             m_c4LastPrompt;
        tCIDLib::TStrList           m_colPorts;
        const tCIDLib::TStrList*    m_pcolMonikers;
        TPushButton*                m_pwndCancelButton;
        TPushButton*                m_pwndNextButton;
        TPushButton*                m_pwndPrevButton;
        TTabbedWnd*                 m_pwndTabs;
        TCQCDriverObjCfg            m_cqcdcEdit;
        TCQCSecToken                m_sectUser;
        TString                     m_strHost;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCDrvWizDlg,TDlgBox)
};

#pragma CIDLIB_POPPACK




