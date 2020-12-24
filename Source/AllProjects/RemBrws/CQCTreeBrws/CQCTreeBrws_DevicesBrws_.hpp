//
// FILE NAME: CQCTreeBrws_DevicesBrws.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 12/27/2015
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
//  This file implements a browswer window plugin for the /Devices section of the
//  browser window, all of which works via CQCServer. This one is only somewhat
//  driven by user interaction, in that they can add new drivers. But mostly it's
//  driven by configuration. As of 5.0, all of the driver configuration is stored
//  on the master server, so that we can enforce driver limits.
//
//  But we don't really care about the configuration itself, only the CQCServers
//  that the drivers are configured for, and the monikers of the drivers themselves.
//  That's all we need to do what we need to do. If the user asks to reconfigure
//  a driver or see the configuration, we go back and query it, so that we don't
//  have to worry about whether it's up to date or not.
//
//  What we really care about here is showing the CQCServers, showing the drivers
//  under each CQCServer, and indicating their status via the color of the text.
//  We do have to query the driver config in order to get the info we need, but
//  we discard it once we have the host/driver lists.
//
//  Once we have that list, we then talk periodically to each CQCServer to get
//  the status of the drivers. That drives the display info. If, in the process of
//  doing that, we see the driver id list of a CQCServer change, then apparently
//  some other admin client has changed the list since the CQCServer itself never
//  changes it (it only changes the status of the drivers themselves.) If we see
//  such a change, we go back and requery the config info for that server.
//
//  We don't show all CQCServers, only those that have drivers configured, so we
//  don't care if there are some that we aren't displaying. When the user asks to
//  add a driver, we get the full list from the name server and present those as
//  the possible targets to. This way we don't have to do a bunch of constant checking
//  for something that won't make any difference to our display.
//
//  So our dislay info is driven purely by configuration, not by what CQCServers are
//  currently running. Some of them may be down, in which case we'll show all its
//  drivers as offline. WE do have to periodically check the config server as well,
//  just in case the config was changed by another client in a way that didn't
//  affect any current running CQCServer.
//
//  All of the monitoring is done by a background thread. It stores the data away
//  and posts to the GUI thread about changes it should update to deal with. We
//  use a mutex to sync access. Changes made by the user are just updated directly
//  by adding or removing items. The bgn thread will pick them up on its subsequent
//  rounds.
//
//  We keep some fairly simple info. We have a per-driver class, which is stored in
//  a keyed hash set keyed on the browser path for that driver. And we have a simple
//  class that holds just the servers and the admin client proxy for that server, also
//  keyed on its key.
//
//
// CAVEATS/GOTCHAS:
//
//  1)  NOTE that, unlike some other browsers, we don't have to send notifications
//      about user requested operations for everything, except for a request to view
//      details on a driver, or to load its client side interface. That's the only
//      thing that would open a tab. For adding or removing drivers, pause/resume
//      and such, we can just handle it ourself internally.
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

class TCQCSrvAdminClientProxy;


// ---------------------------------------------------------------------------
//   CLASS: TDevBrwsDrvInfo
//  PREFIX: dbdi
// ---------------------------------------------------------------------------
class TDevBrwsDrvInfo
{
    public :
        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static const TString& strMonKey
        (
            const   TDevBrwsDrvInfo&        dbdiSrc
        );

        static const TString& strPathKey
        (
            const   TDevBrwsDrvInfo&        dbdiSrc
        );


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TDevBrwsDrvInfo() = delete;

        TDevBrwsDrvInfo
        (
            const   TString&                strHost
            , const TString&                strMoniker
            , const tCIDLib::TBoolean       bClientDriver
        );

        TDevBrwsDrvInfo(const TDevBrwsDrvInfo&) = default;
        TDevBrwsDrvInfo(TDevBrwsDrvInfo&&) = default;

        ~TDevBrwsDrvInfo();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TDevBrwsDrvInfo& operator=(const TDevBrwsDrvInfo&) = default;
        TDevBrwsDrvInfo& operator=(TDevBrwsDrvInfo&&) = default;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bClientDriver() const;

        tCIDLib::TBoolean bSetState
        (
            const   tCQCKit::EDrvStates     eToSet
        );

        tCIDLib::TCard4 c4DriverId() const;

        tCIDLib::TCard4 c4DriverId
        (
            const   tCIDLib::TCard4         c4ToSet
        );

        tCQCKit::EDrvStates eState() const;

        const TString& strBrwsPath() const;

        const TString& strMoniker() const;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bClientDriver
        //      We store whether we have a client driver. This controls whether we
        //      present the client driver option in the popup menu.
        //
        //  m_c4DriverId
        //      The server side id for this driver, which we use to map back from
        //      poll data from the server to this driver.
        //
        //  m_c4Version
        //      The last version we we got when we got the config for this
        //      driver.
        //
        //  m_eState
        //      The last driver state that the bgn ground thread saw. We start it
        //      at NotLoaded, which is the lowest state it can be.
        //
        //  m_eVerbosity
        //      The last driver verbosity that the bgn thread saw.
        //
        //  m_strBrwsPath
        //      This driver's path in the browser, which makes it easier to
        //      correlate between drivers and the browser.
        //
        //  m_strMoniker
        //      The moniker of this driver.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bClientDriver;
        tCIDLib::TCard4         m_c4DriverId;
        tCIDLib::TCard4         m_c4Version;
        tCQCKit::EDrvStates     m_eState;
        tCQCKit::EVerboseLvls   m_eVerbosity;
        TString                 m_strBrwsPath;
        TString                 m_strMoniker;
};


// ---------------------------------------------------------------------------
//   CLASS: TDevBrwsSrvInfo
//  PREFIX: dbsi
// ---------------------------------------------------------------------------
class TDevBrwsSrvInfo
{
    public :
        // -------------------------------------------------------------------
        //  Public class types
        // -------------------------------------------------------------------
        using TDrvList = TRefKeyedHashSet<TDevBrwsDrvInfo, TString, TStringKeyOps>;


        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static const TString& strKey
        (
            const   TDevBrwsSrvInfo&        dbsiSrc
        );


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TDevBrwsSrvInfo() = delete;

        TDevBrwsSrvInfo
        (
            const   TString&                strHost
            ,       TCQCTreeBrowser* const  pwndTree
            , const TCQCUserCtx&            cuctxUser
        );

        TDevBrwsSrvInfo(const TDevBrwsSrvInfo&) = delete;
        TDevBrwsSrvInfo(TDevBrwsSrvInfo&&) = delete;

        ~TDevBrwsSrvInfo();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TDevBrwsSrvInfo& operator=(const TDevBrwsSrvInfo&) = delete;
        TDevBrwsSrvInfo& operator=(TDevBrwsSrvInfo&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TCard4 c4DriverListId() const;

        tCIDLib::TVoid ForceRefresh();

        TDevBrwsDrvInfo* pdbdiFindById
        (
            const   tCIDLib::TCard4         c4ToFind
        );

        TDevBrwsDrvInfo* pdbdiFindByMoniker
        (
            const   TString&                strToFind
        );

        TCQCSrvAdminClientProxy* porbcSrv();

        tCIDLib::TVoid QueryMonikers
        (
                    tCIDLib::TStrList&      colToFill
            , const tCIDLib::TBoolean       bAppend
        )   const;

        const TString& strBrwsPath() const;

        const TString& strHost() const;

        tCIDLib::TVoid Start();

        tCIDLib::TVoid StartShutdown();

        tCIDLib::TVoid WaitForDeath();


    private :
        // -------------------------------------------------------------------
        //  Class types
        // -------------------------------------------------------------------
        enum class EPollStates
        {
            SyncCfg
            , SrvConn
            , InitData
            , Poll
        };
        using TStatesList = TFundVector<tCQCKit::EDrvStates>;
        using TVerboseList = TFundVector<tCQCKit::EVerboseLvls>;


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCheckConnection();

        tCIDLib::TBoolean bRefreshCfg();

        tCIDLib::TVoid DoPoll
        (
            const   tCIDLib::TBoolean       bFirstTime
        );

        tCIDLib::EExitCodes eMonThread
        (
                    TThread&                thrThis
            ,       tCIDLib::TVoid*         pData
        );

        tCIDLib::TVoid QueueDrvStateChange
        (
            const  TString&                 strPath
            , const tCQCKit::EDrvStates     eState
        );

        tCIDLib::TVoid SetAllStates
        (
            const   tCQCKit::EDrvStates     eNew
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bForceRefresh
        //      If the GUI thread knows it has made a change that will cause us to
        //      need to resync, he'll call ForceRefresh() and we'll just set this
        //      flag. The bgn thread will see it and reset our polling state, at a
        //      time where it's known to be valid (instead of us just trying to
        //      reset it directly in the ForceRefresh() call.)
        //
        //  m_c4DriverListId
        //      The last driver list id we got during polling. This is only accessed
        //      by the bgn thread and passed back in for each poll.
        //
        //  m_c4Version
        //      The last version we we got when we got the configured moniker list
        //      for this server. This is only accessed by the bgn thread.
        //
        //  m_colDrvList
        //  m_colDrvListMon
        //      A list of driver objects for the drivers loaded on this host. This is
        //      only accessed by the bgn thread. The main one is keyed by the hiearchical
        //      path of the driver. And we have another, non-adopting, that uses the
        //      moniker, so that we can find the drivers by either quickly.
        //
        //  m_cuctxUser
        //      We need a user security token to do some things we do, so the browser
        //      object passes it along to us.
        //
        //  m_ePollState
        //      The current state of the polling thread. This is only accessed by the
        //      bgn thread.
        //
        //  m_fcolStateId
        //  m_fcolStatePoll
        //      Used in the DoPoll() method, which gets back the ids and states
        //      of any offline drivers. So only used by the bgn thread.
        //
        //  m_mtxLock
        //      The only time we need to use this is when we actually modify the
        //      list of drivers, and when the outside world accesses it, so as to
        //      make sure we don't change it while they are using it.
        //
        //  m_porbcSrv
        //      A pointer to the CQCServer client proxy for this server. This is only
        //      used by the bgn thread.
        //
        //  m_pwndTree
        //      We get a pointer to the device browser object so that we can post
        //      change notifications to it.
        //
        //  m_strBrwsPath
        //      This server's path in the browser, which makes it easier to
        //      correlate between servers and the browser.
        //
        //  m_strHost
        //      The host that this object represents.
        //
        //  m_thrMon
        //      This is our monitor thread, which watches for changes in driver
        //      status for this browser.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean           m_bForceRefresh;
        tCIDLib::TCard4             m_c4DriverListId;
        tCIDLib::TCard4             m_c4Version;
        TCQCUserCtx                 m_cuctxUser;
        EPollStates                 m_ePollState;
        tCIDLib::TCardList          m_fcolIdPoll;
        TStatesList                 m_fcolStatePoll;
        TDrvList                    m_colDrvList;
        TDrvList                    m_colDrvListMon;
        TCQCSrvAdminClientProxy*    m_porbcSrv;
        TMutex                      m_mtxLock;
        TCQCTreeBrowser*            m_pwndTree;
        TString                     m_strBrwsPath;
        TString                     m_strHost;
        TThread                     m_thrMon;
};


// ---------------------------------------------------------------------------
//   CLASS: TCQCDevicesBrws
//  PREFIX: rbrws
// ---------------------------------------------------------------------------
class TCQCDevicesBrws : public TCQCTreeBrwsIntf
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCDevicesBrws();

        TCQCDevicesBrws(const TCQCDevicesBrws&) = delete;
        TCQCDevicesBrws(TCQCDevicesBrws&&) = delete;

        ~TCQCDevicesBrws();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCDevicesBrws& operator=(const TCQCDevicesBrws&) = delete;
        TCQCDevicesBrws& operator=(TCQCDevicesBrws&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid AcceptFiles
        (
            const   TString&                strParScope
            , const tCIDLib::TStrList&      colTarNames
            , const tCIDLib::TStrList&      colSrcPaths
        )   final;

        tCIDLib::TBoolean bAcceptsNew
        (
            const   TString&                strPath
            , const tCIDLib::TStrHashSet&   colExts
        )   const final;

        tCIDLib::TBoolean bDoMenuAction
        (
            const   TString&                strPath
            ,       TTreeBrowseInfo&        wnotToSend
        )   final;

        tCIDLib::TBoolean bIsConnected() const final;

        tCIDLib::TBoolean bProcessMenuSelection
        (
            const   tCIDLib::TCard4         c4CmdId
            , const TString&                strPath
            ,       TTreeBrowseInfo&        wnotToSend
        )   final;

        tCIDLib::TBoolean bReportInvocation
        (
            const   TString&                strPath
            ,       tCIDLib::TBoolean&      bAsEdit
        )   const final;

        tCIDLib::TVoid Initialize
        (
            const   TCQCUserCtx&            cuctxUser
        )   final;

        tCIDLib::TVoid LoadScope
        (
            const   TString&                strPath
        )   final;

        tCIDLib::TVoid MakeDefName
        (
            const   TString&                strParScope
            ,       TString&                strToFill
            , const tCIDLib::TBoolean       bIsFile
        )   const final;

        tCIDLib::TVoid ProcessChange
        (
            const   TString&                strPath
            , const TString&                strOpName
            , const TString&                strData
        )   final;

        tCIDLib::TVoid QueryAccelTable
        (
            const   TString&                strPath
            ,       TAccelTable&            accelToFill
        )   const final;

        tCIDLib::TVoid Terminate() final;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCanRename
        (
            const   TString&                strPath
        )   const final;

        tCIDLib::TBoolean bRenameItem
        (
            const   TString&                strParPath
            , const TString&                strOldName
            , const TString&                strNewName
            , const tCIDLib::TBoolean       bIsScope
        )   final;


    private :
        // -------------------------------------------------------------------
        //  Private data types
        // -------------------------------------------------------------------
        using TSrvList = TRefKeyedHashSet<TDevBrwsSrvInfo, TString, TStringKeyOps>;


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid AddDriver
        (
            const   TString&                strPath
        );

        tCIDLib::TBoolean bReadDrvConfig
        (
            const   TString&                strHost
            , const TString                 strMoniker
            ,       TCQCDriverObjCfg&       cqcdcToFill
        );

        tCIDLib::TBoolean bWriteDrvConfig
        (
            const   TString&                strHost
            , const TString                 strMoniker
            , const TCQCDriverObjCfg&       cqcdcToWrite
            , const tCIDLib::TBoolean       bNew
        );

        tCIDLib::TVoid ChangeServer
        (
            const   TString&                strPath
        );

        tCIDLib::TVoid ExportDrv
        (
                    tCQCKit::TCQCSrvProxy&  orbcSrv
            , const TPathStr&               pathTar
            , const tCIDLib::TBoolean       bIncludeVals
            , const TString&                strMoniker
        );

        tCIDLib::TVoid ExportFlds
        (
            const   TString&                strPath
            , const tCIDLib::TBoolean       bIncludeVals
        );

        TDevBrwsDrvInfo* pdbdiFindDriver
        (
            const   TString&                strHost
            , const TString&                strMoniker
        );

        TDevBrwsSrvInfo* pdbsiFindSrv
        (
            const   TString&                strHost
        );

        tCIDLib::TVoid ParsePath
        (
            const   TString&                strPath
            ,       TString&                strHost
            ,       TString&                strMonitor
        );

        tCIDLib::TVoid PauseResumeDriver
        (
            const   TString&                strPath
            , const tCIDLib::TBoolean       bPause
        );

        tCIDLib::TVoid ReconfigureDriver
        (
            const   TString&                strPath
        );

        tCIDLib::TVoid RefreshAll();

        tCIDLib::TVoid SetupMenu
        (
            const   TString&                strPath
            ,       TMenu&                  menuTar
        )   const;

        tCIDLib::TVoid UnloadDriver
        (
            const   TString&                strPath
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4NSVersionNum
        //      When we query the list of CQCServers we found out there, we store
        //      the version number (of the containing scope) here, so as to reduce
        //      overhead when we go back to check it again, which we do each time
        //      the user wants to add a driver.
        //
        //  m_colLastNSList
        //      This is what we got the last time we queried the name server for
        //      the registered CQCServers. We keep the last version number, in the
        //      m_NSVersionNum member above, so if no changes we don't have to
        //      download the list again. This is only used by the GUI thread during
        //      driver addition, so no need for sync in this case.
        //
        //  m_colSrvList
        //      This is our list of servers we are tracking, each of which contains
        //      info on the drivers it is configured for.
        //
        //  m_strChange_xxx
        //      Strings for the change notifications that
        // -------------------------------------------------------------------
        tCIDLib::TCard4     m_c4NSVersionNum;
        tCIDLib::TStrList   m_colLastNSList;
        TSrvList            m_colSrvList;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCDevicesBrws,TCQCTreeBrwsIntf)
};

#pragma CIDLIB_POPPACK


