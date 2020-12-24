//
// FILE NAME: CppDrvTest_MainFrame.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/25/2018
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
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TCppDrvTestMainFrame
//  PREFIX: wnd
// ---------------------------------------------------------------------------
class TCppDrvTestMainFrame : public TFrameWnd
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCppDrvTestMainFrame();

        TCppDrvTestMainFrame(const TCppDrvTestMainFrame&) = delete;

        ~TCppDrvTestMainFrame();


        // -------------------------------------------------------------------
        //  Unimplemented constructors and operators
        // -------------------------------------------------------------------
        TCppDrvTestMainFrame& operator=(const TCppDrvTestMainFrame&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bProcessAccel
        (
            const   tCIDLib::TVoid* const   pMsgData
        )   const override;


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid CreateFrame
        (
            const   TCQCFrameState&         fstMain
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bAllowShutdown() override;

        tCIDLib::TVoid AreaChanged
        (
            const   TArea&                  areaPrev
            , const TArea&                  areaNew
            , const tCIDCtrls::EPosStates   ePosState
            , const tCIDLib::TBoolean       bOrgChanged
            , const tCIDLib::TBoolean       bSizeChanged
            , const tCIDLib::TBoolean       bStateChanged
        )   override;

        tCIDLib::TBoolean bCreated() override;

        tCIDLib::TVoid Destroyed();

        tCIDLib::TVoid MenuCommand
        (
            const   tCIDLib::TResId         ridItem
            , const tCIDLib::TBoolean       bChecked
            , const tCIDLib::TBoolean       bEnabled
        )   override;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid SaveState();

        tCIDLib::TVoid SetupRemotePort();

        tCIDLib::TVoid StartClientIntf();

        tCIDLib::TVoid StopClientIntf();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_accelMain
        //      We need an accellerator table to handle a couple things at our level. All
        //      others are passed down to the active tab.
        //
        //  m_c4ClDrvTabId
        //      We remember the tab id of the client driver tab, if any, so that we can
        //      close that tab when the driver stops.
        //
        //  m_pfactRemote
        //      If the user has set us up for a remote port, then we register a remote port
        //      factory with CIDComm. We keep a pointer around for our own use and store it
        //      here, but we don't own it. If we aren't configured for a remote port, then
        //      this is null.
        //
        //  m_pwndClientTabs
        //      We create a tabbed window as a main client window and just keep it
        //      sized to fit us.
        //
        //  m_pwndClDrvTab
        //      If the driver has a client tab, when it is started up to run, we will
        //      create a client driver tab for it.
        //
        //  m_pwndDrvInfoTab
        //      A standard tab that we laod up and use to display the driver config
        //      and manifest data.
        //
        //  m_pwndSrvDrvTab
        //      A pointer to our server driver tab. We always create this one.
        //
        //  m_pwndTrigMonTab
        //      We insert a tab into the main tabbed window and it just displays any
        //      event triggers that get spit out by our driver.
        //
        //  m_strTitleText
        //      The title text to use in popups. We don't let them use our text since
        //      we put the manifest file into it.
        //
        //  m_wndChat
        //      The chat window. It's only accessible if the remote port server interface is
        //      enabled. It's a secondary frame so that it can be separately managed. We
        //      call it to let it know about changes in the remote port enable status.
        //
        //  m_wstateXXX
        //      Some window states that we use to efficiently set up everything for our
        //      our major states. We can have no session. We can have a session but
        //      the driver is not running. And we can have a session with the driver
        //      running.
        // -------------------------------------------------------------------
        TAccelTable             m_accelMain;
        tCIDLib::TCard4         m_c4ClDrvTabId;
        TCQCRemSrvPortFactory*  m_pfactRemote;
        TTabbedWnd*             m_pwndClientTabs;
        TClDrvTab*              m_pwndClDrvTab;
        TCQCDrvInfoTab*         m_pwndDrvInfoTab;
        TSrvDrvTab*             m_pwndSrvDrvTab;
        TTrigMonTab*            m_pwndTrigMonTab;
        TString                 m_strTitleText;
        TCQCChatWnd             m_wndChat;
        TWndState               m_wstateNoSession;
        TWndState               m_wstateSessionNR;
        TWndState               m_wstateSessionR;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCppDrvTestMainFrame,TFrameWnd)
};

#pragma CIDLIB_POPPACK

