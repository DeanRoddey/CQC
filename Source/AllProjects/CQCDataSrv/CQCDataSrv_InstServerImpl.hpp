//
// FILE NAME: CQCDataSrv_InstServerImpl.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/16/2001
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
//  This is the header for the CQCDataSrv_Impl.cpp file, which provides an
//  implementation of the CQC installation server IDL interface. This interface
//  serves up manifest files, protocol files, DLLs and resource/msg files for
//  drivers.
//
//  It also coordinates access to configured drivers. We define some small classes
//  to maintain info on the configured drivers and the servers that host them.
//  CQCServer hosts come here to figure out what drivers to load, and admin clients
//  work through this interface to manage configured drivers.
//
//  The driver and server objects maintain a version that is bumped any time the
//  object is changed. The server objects bump their version any time that their
//  actual content changes, i.e. if drivers are added or removed.
//
//  Most of our stuff is local files under the CQCData directory, but the configured
//  driver stuff is stored on the config server, which we load to memory and just
//  write through to the config server.
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
//   CLASS: TCQCDrvCfgDrvInfo
//  PREFIX: dcdi
// ---------------------------------------------------------------------------
class TCQCDrvCfgDrvInfo
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCDrvCfgDrvInfo();

        TCQCDrvCfgDrvInfo
        (
            const   TCQCDriverObjCfg&       cqcdcDrv
        );

        TCQCDrvCfgDrvInfo(const TCQCDrvCfgDrvInfo&) = default;
        TCQCDrvCfgDrvInfo(TCQCDrvCfgDrvInfo&&) = delete;

        ~TCQCDrvCfgDrvInfo();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCDrvCfgDrvInfo& operator=(const TCQCDrvCfgDrvInfo&) = default;
        TCQCDrvCfgDrvInfo& operator=(TCQCDrvCfgDrvInfo&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bPaused() const;

        tCIDLib::TBoolean bSetPausedState
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TCard4 c4Version() const;

        const TCQCDriverObjCfg& cqcdcDrv() const;

        const TString& strMoniker() const;

        tCIDLib::TVoid SetConfig
        (
            const   TCQCDriverObjCfg&       cqcdcToSet
        );


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4Version
        //      A version number we bump any time this config is modified.
        //
        //  m_cqcdcDrv
        //      The configuration for this driver, which includes the moniker of
        //      course.
        // -------------------------------------------------------------------
        tCIDLib::TCard4     m_c4Version;
        TCQCDriverObjCfg    m_cqcdcDrv;
};


// ---------------------------------------------------------------------------
//   CLASS: TCQCDrvCfgSrvInfo
//  PREFIX: dcsi
// ---------------------------------------------------------------------------
class TCQCDrvCfgSrvInfo
{
    public :
        // -------------------------------------------------------------------
        //  Public data types
        // -------------------------------------------------------------------
        using TSrvList = TVector<TCQCDrvCfgDrvInfo>;


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCDrvCfgSrvInfo();

        TCQCDrvCfgSrvInfo
        (
            const   TString&                strHost
        );

        TCQCDrvCfgSrvInfo(const TCQCDrvCfgSrvInfo&) = default;
        TCQCDrvCfgSrvInfo(TCQCDrvCfgSrvInfo&&) = delete;

        ~TCQCDrvCfgSrvInfo();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCDrvCfgSrvInfo& operator=(const TCQCDrvCfgSrvInfo&) = default;

        TCQCDrvCfgSrvInfo& operator=(TCQCDrvCfgSrvInfo&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TCard4 c4AddDriver
        (
            const   TCQCDriverObjCfg&       cqcdcToAdd
        );

        tCIDLib::TCard4 c4UpdateDriver
        (
            const   TCQCDriverObjCfg&       cqcdcToSet
        );

        tCIDLib::TCard4 c4DriverCnt() const;

        tCIDLib::TCard4 c4Version() const;

        const TCQCDrvCfgDrvInfo& dcdiAt
        (
            const   tCIDLib::TCard4         c4At
        )   const;

        TCQCDrvCfgDrvInfo* pdcdiByMoniker
        (
            const   TString&                strToFind
            ,       tCIDLib::TCard4&        c4At
        );

        tCIDLib::TVoid QueryDrvConfigs
        (
                    tCQCKit::TDrvCfgList&   colToFill
            ,       tCIDLib::TCardList&     fcolCfgVers
            ,       tCIDLib::TCard4&        c4Version
        );

        tCIDLib::TVoid QueryHostMons
        (
                    tCIDLib::TKVPList&      colToFill
        );

        tCIDLib::TVoid QueryMons
        (
                    tCIDLib::TStrList&      colToFill
            ,       tCIDLib::TCardList&     fcolCfgVers
            ,       tCIDLib::TBoolList&     fcolClDrv
        );

        tCIDLib::TCard4 c4RemoveDriverAt
        (
            const   tCIDLib::TCard4         c4At
        );

        const TString& strHost() const;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4Version
        //      A version number we bump any time the config for any of the
        //      drivers associated with this server are modifed (or added or
        //      removed.)
        //
        //  m_colDrivers
        //      The driver info objects for the drivers configured for this
        //      server.
        //
        //  m_strHost
        //      The host that this object represents.
        // -------------------------------------------------------------------
        tCIDLib::TCard4 m_c4Version;
        TSrvList        m_colDrivers;
        TString         m_strHost;
};


// ---------------------------------------------------------------------------
//   CLASS: TCQCInstSrvImpl
//  PREFIX: orbs
// ---------------------------------------------------------------------------
class TCQCInstSrvImpl : public TCQCInstServerBase
{
    public :
        // -------------------------------------------------------------------
        //  Public class types
        // -------------------------------------------------------------------
        using TManifestList = TVector<TCQCDriverCfg>;
        using TDrvCfgCache = TVector<TCQCDrvCfgSrvInfo>;


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCInstSrvImpl();

        TCQCInstSrvImpl(const TCQCInstSrvImpl&) = delete;
        TCQCInstSrvImpl(TCQCInstSrvImpl&&) = delete;

        ~TCQCInstSrvImpl();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCInstSrvImpl& operator=(const TCQCInstSrvImpl&) = delete;
        TCQCInstSrvImpl& operator=(TCQCInstSrvImpl&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bAddOrUpdateCheatSheet
        (
            const   TString&                strUserName
            ,       tCIDLib::TCard4&        c4Ver
            , const TCQCActVarList&         cavlToSet
            , const TCQCSecToken&           sectUser
        )   final;

        tCIDLib::TBoolean bUpdateEmailAccount
        (
            const   TCQCEMailAccount&       emacctToAdd
            ,       tCIDLib::TCard4&        c4Ver
            , const TCQCSecToken&           sectUser
            , const tCIDLib::TBoolean       bMustBeNew
        )   final;

        tCIDLib::TBoolean bAddOrUpdateGC100Cfg
        (
            const   tCIDLib::TCard4         c4BufSz
            , const THeapBuf&               mbufToSet
            ,       tCIDLib::TCard4&        c4SerialNum
            , const TCQCSecToken&           sectUser
        )   final;

        tCIDLib::TBoolean bAddOrUpdateJAPCfg
        (
            const   tCIDLib::TCard4         c4BufSz
            , const THeapBuf&               mbufToFill
            ,       tCIDLib::TCard4&        c4SerialNum
            , const TCQCSecToken&           sectUser
        )   final;

        tCIDLib::TBoolean bDeleteEMailAccount
        (
            const   TString&                strAccount
            , const TCQCSecToken&           sectUser
        )   final;

        tCIDLib::TBoolean bDrvCfgDriverPresent
        (
            const   TString&                strMoniker
            ,       TString&                strHost
            , const TCQCSecToken&           sectUser
        )   final;

        tCIDLib::TBoolean bDrvCfgMoveDrivers
        (
            const   TString&                strOldHost
            , const TString&                strNewHost
            ,       tCIDLib::TStrList&      colMsgs
            , const TCQCSecToken&           sectUser
        );

        tCIDLib::TBoolean bDrvCfgQueryConfig
        (
                    tCIDLib::TCard4&        c4Version
            , const TString&                strMoniker
            ,       TString&                strHost
            ,       TCQCDriverObjCfg&       cqcdcToFill
            , const TCQCSecToken&           sectUser
        )   final;

        tCIDLib::TBoolean bDrvCfgQueryHostConfigs
        (
                    tCIDLib::TCard4&        c4Version
            , const TString&                strHost
            ,       tCQCKit::TDrvCfgList&   colCfgs
            ,       tCIDLib::TCardList&     fcolCfgVers
            , const TCQCSecToken&           sectUser
        )   final;

        tCIDLib::TBoolean bDrvCfgQueryHosts
        (
                    tCIDLib::TStrList&      colToFill
            , const TCQCSecToken&           sectUser
        )   final;

        tCIDLib::TBoolean bDrvCfgQueryMonList
        (
                    tCIDLib::TCard4&        c4Version
            , const TString&                strHost
            ,       tCIDLib::TStrList&      colToFill
            ,       tCIDLib::TCardList&     fcolVers
            ,       tCIDLib::TBoolList&     fcolClDrvs
            , const TCQCSecToken&           sectUser
        )   final;

        tCIDLib::TBoolean bDrvCfgQueryMonList
        (
                    tCIDLib::TKVPList&      colToFill
            , const TCQCSecToken&           sectUser
        )   final;

        tCIDLib::TBoolean bDrvCfgSetPause
        (
            const   TString&                strMoniker
            , const tCIDLib::TBoolean       bState
            ,       tCIDLib::TCard4&        c4NewVersion
            , const TCQCSecToken&           sectUser
        )   final;

        tCIDLib::TBoolean bQueryCheatSheet
        (
                    tCIDLib::TCard4&        c4SerialNum
            ,       tCIDLib::ELoadRes&      eLoadRes
            , const TString&                strUserName
            ,       TCQCActVarList&         cavlCheatSheet
            , const TCQCSecToken&           sectUser
        )   final;

        tCIDLib::TBoolean bQueryDrvManifest
        (
            const   TString&                strMake
            , const TString&                strModel
            ,       TCQCDriverCfg&          cqcdcToFill
        )   final;

        tCIDLib::TBoolean bQueryEmailAccount
        (
            const   TString&                strAccount
            ,       TCQCEMailAccount&       emacctToFill
            , const tCIDLib::TBoolean       bThrowIfNot
            , const TCQCSecToken&           sectUser
        )   final;

        tCIDLib::TBoolean bQueryEMailAccountNames
        (
                    tCIDLib::TStrList&      colToFill
            , const TCQCSecToken&           sectUser
        )   final;

        tCIDLib::TBoolean bQueryKeyMap
        (
                    tCIDLib::TCard4&        c4SerialNum
            ,       tCIDLib::ELoadRes&      eLoadRes
            , const TString&                strUserName
            ,       TCQCKeyMap&             kmToFill
            , const TCQCSecToken&           sectUser
        )   final;

        tCIDLib::TBoolean bQueryLocationInfo
        (
                    tCIDLib::TFloat8&       f8LatToFill
            ,       tCIDLib::TFloat8&       f8LongToFill
            , const TCQCSecToken&           sectUser
        )   final;

        tCIDLib::TBoolean bQueryProtoFile
        (
            const   TString&                strFileName
            ,       tCIDLib::TCard4&        c4BytesToFill
            ,       THeapBuf&               mbufToFill
        )   final;

        tCIDLib::TBoolean bQueryZWDIIndex
        (
                    tCIDLib::TCard4&        c4IndexSz
            ,       THeapBuf&               mbufIndex
            ,       tCIDLib::TCard4&        c4DTDSz
            ,       THeapBuf&               mbufDTD
        )   final;

        tCIDLib::TBoolean bQueryZWDIFile
        (
            const   TString&                strFileName
            ,       tCIDLib::TCard4&        c4BufSz
            ,       THeapBuf&               mbufData
        )   final;

        tCIDLib::TCard4 c4QueryDrvManifests
        (
                    TManifestList&          colToFill
            , const TCQCSecToken&           sectUser
        )   final;

        tCIDLib::TVoid DrvCfgQueryDrvsInfo
        (
                    tCIDLib::TStrList&      colMonikers
            ,       tCIDLib::TStrList&      colMakes
            ,       tCIDLib::TStrList&      colModels
            ,       tCQCKit::TDevCatList&   fcolCats
            , const TCQCSecToken&           sectUser
        )   final;

        tCIDLib::TVoid DrvCfgQueryReportInfo
        (
                    tCIDLib::TKValsList&    colToFill
            , const TCQCSecToken&           sectUser
        )   final;

        tCIDLib::TVoid DrvCfgRemoveDrv
        (
            const   TString&                strMoniker
            , const TString&                strHost
            ,       tCIDLib::TCard4&        c4NewVersion
            , const TCQCSecToken&           sectUser
        )   final;

        tCIDLib::TVoid DrvCfgSetConfig
        (
            const   TString&                strMoniker
            , const TString&                strHost
            , const TCQCDriverObjCfg&       cqcdcToSet
            , const tCIDLib::TBoolean       bNewDriver
            ,       tCIDLib::TCard4&        c4NewVersion
            , const TCQCSecToken&           sectUser
        )   final;

        tCIDLib::ELoadRes eQueryGC100Cfg
        (
                    tCIDLib::TCard4&        c4SerialNum
            ,       tCIDLib::TCard4&        c4BufSz
            ,       THeapBuf&               mbufToFill
            , const TCQCSecToken&           sectUser
        )   final;

        tCIDLib::ELoadRes eQueryJAPCfg
        (
                    tCIDLib::TCard4&        c4SerialNum
            ,       tCIDLib::TCard4&        c4BufSz
            ,       THeapBuf&               mbufToFill
            , const TCQCSecToken&           sectUser
        )   final;


        tCIDLib::TVoid Ping() final
        {
        }

        tCIDLib::TVoid QueryVoiceFiles
        (
                    tCIDLib::TCard4&        c4TreeBufSz
            ,       THeapBuf&               mbufTree
            ,       tCIDLib::TCard4&        c4GramBufSz
            ,       THeapBuf&               mbufGrammar
        )   final;

        tCIDLib::TVoid RenameEMailAccount
        (
            const   TString&                strOldName
            , const TString&                strNewName
            , const TCQCSecToken&           sectUser
        )   final;

        tCIDLib::TVoid SetKeyMap
        (
            const   TString&                strUserName
            , const TCQCKeyMap&             kmToSet
            , const TCQCSecToken&           sectUser
        )   final;

        tCIDLib::TBoolean SetLocationInfo
        (
            const   tCIDLib::TFloat8        f8Lat
            , const tCIDLib::TFloat8        f8Long
            , const TCQCSecToken&           sectUser
        )   final;

        tCIDLib::TVoid UploadProtocol
        (
            const   TString&                strFileName
            , const tCIDLib::TCard4         c4DataSize
            , const THeapBuf&               mbufData
            , const tCIDLib::TBoolean       bSystem
            , const TCQCSecToken&           sectUser
        )   final;

        tCIDLib::TVoid UploadManifest
        (
            const   TString&                strFileName
            , const tCIDLib::TCard4         c4DataSize
            , const THeapBuf&               mbufData
            , const tCIDLib::TBoolean       bSystem
            , const TCQCSecToken&           sectUser
        )   final;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        const TManifestList& colDrvManifests() const
        {
            return m_colDrvManifests;
        }

        TManifestList& colDrvManifests()
        {
            return m_colDrvManifests;
        }


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid Initialize() final;

        tCIDLib::TVoid Terminate() final;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bLoadUpFile
        (
            const   TString&                strName
            ,       TMemBuf&                mbufToLoad
            ,       tCIDLib::TCard4&        c4Size
            , const tCIDLib::TBoolean       bFailIfNotFound
        );

        tCIDLib::TVoid CheckUserRole
        (
            const   TCQCSecToken&           sectUser
            , const tCQCKit::EUserRoles     eMinRole
            , const tCIDLib::TCard4         c4Line
        )   const;

        tCIDLib::TVoid CheckUserRole
        (
            const   TCQCSecToken&           sectUser
            , const TString&                strAccount
            , const tCQCKit::EUserRoles     eMinRole
            , const tCIDLib::TCard4         c4Line
        )   const;

        TCQCDrvCfgSrvInfo* pdcsiFind
        (
            const   TString&                strToFind
        );

        TCQCDrvCfgSrvInfo* pdcsiFind
        (
            const   TString&                strToFind
            ,       tCIDLib::TCard4&        c4At
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_colDrvCfgCache
        //      In order to control the number of drivers configured, and coordinate
        //      management of that configuration, we wrap that data and provide an
        //      interface to get to it. The actual data is stored in the config
        //      server, so some of what we do is pass-throughs to him, but we maintain
        //      an in-memory cache for fast access and atomic checks for dups and
        //      capacity limits and such. On startup we load up all of the configured
        //      drivers from the config server. Everyone else gets this info from us.
        //
        //  m_colDrvManifests
        //      A list of driver manifest objects that we load from the XML manifest
        //      files.
        //
        //  m_f8Lat
        //  m_f8Long
        //      Our configured geographical location info, which is used in various
        //      places, but mostly for doing sunrise/sunset calculations. We load it
        //      from our private repo and make it available. We also provide an API to
        //      set it which CQCAdmin needs. If not set yet, they are zero.
        //
        //  m_mtxSync
        //      Provides synchronization of access to our data.
        //
        //  m_oseRepo
        //      We have a repository of our own for storing sensitive stuff that we can't
        //      put into the main configuration repo.
        //
        //  m_strIRDir
        //  m_strProtoDir
        //  m_strZWDIDir
        //      We pre-build the paths to our supported file types. If in the
        //      development environment, we special case them. Else they are all
        //      relative to the CQCData directory.
        // -------------------------------------------------------------------
        tCIDLib::TCard4         m_c4RmCfgSz;
        TDrvCfgCache            m_colDrvCfgCache;
        TManifestList           m_colDrvManifests;
        tCIDLib::TFloat8        m_f8Lat;
        tCIDLib::TFloat8        m_f8Long;
        TMutex                  m_mtxSync;
        TCIDObjStore            m_oseRepo;
        TString                 m_strIRDir;
        TString                 m_strManifestDir;
        TString                 m_strProtoDir;
        TString                 m_strZWDIDir;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCInstSrvImpl,TCQCInstServerBase)
};

#pragma CIDLIB_POPPACK

