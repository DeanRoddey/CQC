//
// FILE NAME: CQCIGKit_DriverClient.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/15/2015
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
//  This is the header for the CQCGKit_Client.cpp file, which implements a base class
//  from which all client side drivers derive. These are hosted inside a generic client
//  driver tab in the admin client. So these are windows which the client driver tab
//  creates and sizes to fit itself. These guys cannot provide the admin client tab
//  functionality, since it's only available within the admin client. But this cannot
//  be defined in the admin client since it has to be available for client side drivers
//  to derive from.
//
//  So we have to sort of split the two things out. For the most part all the tab has
//  to do is keep us sized.

//  This class does a lot of the grunt work for all drivers, and insures a certain amount
//  of consistency of implementation as well.
//
//  The things handled at this level are:
//
//  1)  Creates the poll thread and update timer, which call protected virtual methods
//      to allow the derive class to get processing time. It checks whether they need to
//      be called or not, i.e. if the server is offline, there is no need to call the
//      driver's update method. Or, if no new data changes have come in from the server
//      here is no need to call the drivers update method, and so forth. This takes a lot
//      of grunt work checking off the shoulders of all the drivers.
//
//  2)  Our poll thread logic keeps up with the connection state and does what is
//      appropriate to get reconnected, by calling methods on the derived class.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TCQCDriverClient
//  PREFIX: wnd
// ---------------------------------------------------------------------------
class CQCIGKITEXPORT TCQCDriverClient : public TGenericWnd
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCDriverClient() = delete;

        TCQCDriverClient
        (
            const   TCQCDriverObjCfg&       cqcdcThis
            , const TString&                strDriverClass
            , const tCQCKit::EActLevels     eActivityLevel
            , const TCQCUserCtx&            cuctxToUse
        );

        TCQCDriverClient(const TCQCDriverClient&) = delete;
        TCQCDriverClient(TCQCDriverClient&&) = delete;

        ~TCQCDriverClient();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCDriverClient& operator=(const TCQCDriverClient&) = delete;
        TCQCDriverClient& operator=(TCQCDriverClient&&) = delete;


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        virtual tCIDLib::TBoolean bChanges() const = 0;

        virtual tCIDLib::TBoolean bSaveChanges
        (
                    TString&                strErrMsg
        ) = 0;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        const TCQCUserCtx& cuctxToUse() const;

        tCIDLib::TVoid CreateClDrvWnd
        (
            const   TWindow&                wndParent
            , const TArea&                  areaInit
            , const tCIDCtrls::TWndId       widToUse
        );

        tCQCGKit::EConnStates eConnState() const;

        tCQCKit::EUserRoles eUserRole() const;

        tCQCKit::TCQCSrvProxy& orbcServer();

        TMutex* pmtxSync() const;

        const TCQCDriverObjCfg& cqcdcThis() const;

        const TCQCSecToken& sectUser() const;

        const TString& strMoniker() const;

        tCIDLib::TVoid StartDriver();

        tCIDLib::TVoid StopDriver();


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bEraseBgn
        (
                    TGraphDrawDev&          gdevToUse
        )   override;

        tCIDLib::TBoolean bPaint
        (
                    TGraphDrawDev&          gdevToUse
            , const TArea&                  areaUpdate
        )   override;

        tCIDLib::TVoid Destroyed() override;

        tCIDLib::TVoid Timer
        (
            const   tCIDCtrls::TTimerId     tmidToDo
        )   override;


        // -------------------------------------------------------------------
        //  Protected, virtual methods
        //
        //  The derived driver provides implementations of these methods, and
        //  this class calls them to do driver specific stuff.
        // -------------------------------------------------------------------
        virtual tCIDLib::TBoolean bGetConnectData
        (
                    tCQCKit::TCQCSrvProxy&  orbcTarget
        ) = 0;

        virtual tCIDLib::TBoolean bDoPoll
        (
                    tCQCKit::TCQCSrvProxy&  orbcTarget
        ) = 0;

        virtual tCIDLib::TVoid Connected() = 0;

        virtual tCIDLib::TVoid DoCleanup() = 0;

        virtual tCIDLib::TVoid LostConnection() = 0;

        virtual tCIDLib::TVoid UpdateDisplayData() = 0;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid DoPollCycle();

        tCIDLib::EExitCodes ePollThread
        (
                    TThread&                thrThis
            ,       tCIDLib::TVoid*         pData
        );

        tCIDLib::TVoid TestConnection();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bCleanupDone
        //      Just in case our StopDriver() method gets called more than once, we
        //      don't want to force all derivatives to be safe against multiple calls
        //      to the cleanup virtual method. So we remember if it has been called.
        //
        //  m_bFirstTimer
        //      We want to do an initial relatively long timer period, to give the
        //      client time to get up and the windows displayed before we start trying
        //      to update the data. Otherwise, the client comes up too slowly. We use
        //      this to reset the timer period once we get the first timer call.
        //
        //  m_c4PollChanges
        //      The poll thread bumps this up any time it stores changes. The update
        //      timer will watch for this and, if it is non-zero, update the window
        //      and zero this counter for the next time.
        //
        //  m_cuctxToUse
        //      The containing client gives us the user context of the user that's
        //      logged in, so that any client drivers can access it if needed.
        //
        //  m_eActivityLevel
        //      This is the activity level that the derived class claims to belong to.
        //      It is a measure of how fast their data can change, so we can use it to
        //      adjust our poll and reconnect periods. At this time, its not being used.
        //      But, in order to be able to turn it on later without requiring changes
        //      to drivers, we make them pass it in.
        //
        //  m_eConnState
        //      The current connection state, which tells us whether were are connected
        //      to our server and whether he is connected to his device.
        //
        //  m_ePrevState
        //      The previou connection state, so that our timer knows when it has
        //      changed. It sets the current here after seeing that it changed.
        //
        //  m_mtxSync
        //      This is used to sync between the poll thread and the window timer. We
        //      have a conversion operator so that the derived class can lock us via
        //      a standard locker as well, which it needs to do when it gets user
        //      input and needs to invoke remote ops and such. It has to be mutable since
        //      const methods have to be able to lock in order to read data.
        //
        //  m_orbcServer
        //      The client proxy for the CQCServer admin interface of the server that
        //      is running our server side driver. This is how we read/write data,
        //      config, etc... This is passed through to any of the virtual callbacks
        //      that have a need to interact with the server driver.
        //
        //  m_cqcdcThis
        //      The driver instance config data, which is passed to us by the derived
        //      class' ctor, which gets it from the CQCClient framework who loads the
        //      class.
        //
        //  m_thrPoll
        //      This is the poll thread. It handles the background work of (re)
        //      connecting to the server, and polling the server for new data and
        //      storing it away.
        //
        //  m_tmidUpdate
        //      The id of our update timer.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bCleanupDone;
        tCIDLib::TBoolean       m_bFirstTimer;
        tCIDLib::TCard4         m_c4PollChanges;
        const TCQCUserCtx&      m_cuctxToUse;
        tCQCKit::EActLevels     m_eActivityLevel;
        tCQCGKit::EConnStates   m_eConnState;
        tCQCGKit::EConnStates   m_ePrevState;
        mutable TMutex          m_mtxSync;
        tCQCKit::TCQCSrvProxy   m_orbcServer;
        TCQCDriverObjCfg        m_cqcdcThis;
        TString                 m_strTestProg;
        TThread                 m_thrPoll;
        tCIDCtrls::TTimerId     m_tmidUpdate;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCDriverClient,TGenericWnd)
};

#pragma CIDLIB_POPPACK

