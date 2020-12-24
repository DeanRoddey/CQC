//
// FILE NAME: MyMoviesRepoS_DriverImpl.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/24/2008
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
//  This is the header file for the actual driver class. Since we use the std in-
//  memory media database, we derive from a base class in the CQCMedia facility that
//  provides support for all the standard queries we get from clients. It knows how
//  to fulfill those out of the database so we don't have to deal with that.
//
//  We have to provide a virtual method that it will call back if it needs to load
//  up cover art, so we implement a very simple derivative of the standard engine,
//  since it doesn't know how the derive classes access cover art. MM provides both
//  large and thumb art, so we can support loading them separately in this repo.
//
//  So we are mainly responsible for just loading the contents from MyMovies into the
//  media database, and keeping the handful of fields updated.
//
//  Loading is done via a separate class that does it in the background. This way,
//  we can keep any current contents up and available until the new data is available
//  and then we just quickly switch over.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//  CLASS: TPlexMediaRepoEng
// PREFIX: srdb
// ---------------------------------------------------------------------------
class TPlexMediaRepoEng : public TCQCStdMediaRepoEng
{
    public  :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TPlexMediaRepoEng
        (
            const   TString&                strMoniker
        );

        ~TPlexMediaRepoEng();


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TCard4 c4LoadRawCoverArt
        (
            const   TMediaImg&              mimgToLoad
            ,       TMemBuf&                mbufToFill
            , const tCQCMedia::ERArtTypes   eType
        );


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid SetServerEP
        (
            const   TIPEndPoint&            ipepPlexSrv
        );

        tCIDLib::TVoid SetVerbosity
        (
            const   tCQCKit::EVerboseLvls   eToSet
        );


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TCard4 c4DownloadArt
        (
            const   TString&                strPath
            ,       TMemBuf&                mbufToFill
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_eVerbose
        //      The driver keeps us updated with his verbosity level for logging
        //      purposes.
        //
        //  m_ipepSrv
        //      The IP end point of the plex server, which we need in order to
        //      download images.
        // -------------------------------------------------------------------
        tCQCKit::EVerboseLvls   m_eVerbose;
        TIPEndPoint             m_ipepSrv;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TPlexMediaRepoEng,TCQCStdMediaRepoEng)
};



// ---------------------------------------------------------------------------
//   CLASS: PlexRepoS
//  PREFIX: drv
// ---------------------------------------------------------------------------
class TPlexRepoSDriver : public TCQCStdMediaRepoDrv
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TPlexRepoSDriver() = delete;

        TPlexRepoSDriver
        (
            const   TCQCDriverObjCfg&       cqcdcToLoad
        );

        TPlexRepoSDriver(const TPlexRepoSDriver&) = delete;
        TPlexRepoSDriver(TPlexRepoSDriver&&) = delete;

        ~TPlexRepoSDriver();


        // -------------------------------------------------------------------
        //  Public operators=
        // -------------------------------------------------------------------
        TPlexRepoSDriver& operator=(const TPlexRepoSDriver&) = delete;
        TPlexRepoSDriver& operator=(TPlexRepoSDriver&&) = delete;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bGetCommResource
        (
                    TThread&                thrThis
        );

        tCIDLib::TBoolean bWaitConfig
        (
                    TThread&                thrThis
        );

        tCQCKit::ECommResults eBoolFldValChanged
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4FldId
            , const tCIDLib::TBoolean       bNewValue
        );

        tCQCKit::ECommResults eConnectToDevice
        (
                    TThread&                thrThis
        );

        tCQCKit::EDrvInitRes eInitializeImpl();

        tCQCKit::ECommResults ePollDevice
        (
                    TThread&                thrThis
        );

        tCIDLib::TVoid ReleaseCommResource();

        tCIDLib::TVoid TerminateImpl();

        tCIDLib::TVoid VerboseModeChanged
        (
            const   tCQCKit::EVerboseLvls   eNewState
        );


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bMonitorLoad();



        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4FldId_XXX
        //      We look up the ids of our fields after registering them, so that we
        //      can do efficient 'by id' reads/writes.
        //
        //  m_ipepServer
        //      We get the server end point out during init and store it here for
        //      later use.
        //
        //  m_mdbLoad
        //      When doing loads, we load first into this database. If it goes well,
        //      we let our DB engine steal all the contents.
        //
        //  m_pdblRepo
        //      This is our background repository loader object. It runs in a separate
        //      thread to load up the media database, while we go about our business.
        //      We allocate it when needed. The poll callback watches for it to either
        //      complete the load or fail to.
        //
        //  m_srdbEngine
        //      We use a standard repo database engine to provide the database and
        //      serving up data to media data clients.
        //
        //  m_strLoadMsg
        //      We set a field to a status message during the loading process and it
        //      must be translatable, so we pre-load it to avoid having to reload it
        //      all the time.
        // -------------------------------------------------------------------
        tCIDLib::TCard4     m_c4FldId_DBSerialNum;
        tCIDLib::TCard4     m_c4FldId_LoadStatus;
        tCIDLib::TCard4     m_c4FldId_Status;
        tCIDLib::TCard4     m_c4FldId_ReloadDB;
        tCIDLib::TCard4     m_c4FldId_TitleCnt;
        TIPEndPoint         m_ipepServer;
        TMediaDB            m_mdbLoad;
        TPlexRepoLoader*    m_pdblRepo;
        TPlexMediaRepoEng   m_srdbEngine;
        TString             m_strLoadMsg;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TPlexRepoSDriver,TCQCStdMediaRepoDrv)
};

#pragma CIDLIB_POPPACK


