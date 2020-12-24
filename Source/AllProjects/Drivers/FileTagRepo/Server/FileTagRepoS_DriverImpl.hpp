//
// FILE NAME: FileTagRepoS_DriverImpl.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/28/2007
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
//  We have to do a simple derivative of the standrad database engine class and
//  override a method it will call back if it needs to load up cover art, since it
//  doesn't know how the derive classes access cover art.
//
//  The base class holds the in-memory database, so we just call an access method
//  any time we need to access it, which is mainly just during the loading process.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//  CLASS: TFTagMediaRepoEng
// PREFIX: srdb
// ---------------------------------------------------------------------------
class TFTagMediaRepoEng : public TCQCStdMediaRepoEng
{
    public  :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TFTagMediaRepoEng
        (
            const   TString&                strMoniker
        );

        TFTagMediaRepoEng(const TFTagMediaRepoEng&) = delete;

        ~TFTagMediaRepoEng();


        // -------------------------------------------------------------------
        //  Public, inheritedmethods
        // -------------------------------------------------------------------
        tCIDLib::TCard4 c4LoadRawCoverArt
        (
            const   TMediaImg&              mimgToLoad
            ,       TMemBuf&                mbufToFill
            , const tCQCMedia::ERArtTypes   eType
        );

    private :
        RTTIDefs(TFTagMediaRepoEng,TCQCStdMediaRepoEng)
};




// ---------------------------------------------------------------------------
//   CLASS: FileTagRepoS
//  PREFIX: drv
// ---------------------------------------------------------------------------
class TFileTagRepoSDriver : public TCQCStdMediaRepoDrv
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TFileTagRepoSDriver() = delete;

        TFileTagRepoSDriver
        (
            const   TCQCDriverObjCfg&       cqcdcToLoad
        );

        TFileTagRepoSDriver(const TFileTagRepoSDriver&) = delete;

        ~TFileTagRepoSDriver();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TFileTagRepoSDriver& operator=(const TFileTagRepoSDriver&) = delete;


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

        tCQCKit::ECommResults eCardFldValChanged
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4FldId
            , const tCIDLib::TCard4         c4NewValue
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


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bMonitorLoad();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bLoadPlaylists
        //      We have a driver prompt that can tell us whether to load or ignore
        //      playlist files.
        //
        //  m_bUseEmbeddedArt
        //      There is a driver prompt to allow the use of embedded art or not. If
        //      they have only embedded art they can enable it. But it can be quite
        //      piggy over the network, so it needs to be optional. If this is not
        //      set, then we will always look for external art.
        //
        //  m_c4FldId_XXX
        //      We look up the ids of our fields after registering them, so that we
        //      can do efficient 'by id' reads/writes.
        //
        //  m_mdbLoad
        //      A temp database to load into, so that we can leave the current content
        //      in place while it's going on. If th eload is successful, we give all
        //      of the content to our database engine.
        //
        //  m_pdblRepo
        //      This is our background repository loader object. It runs in a separate
        //      thread to load up the media database, while we go about our business.
        //      We allocate it when needed.
        //
        //  m_srdbEngine
        //      We use a standard repo database engine to manage the DB and serve up
        //      data to clients. Actually we have a small derivative of it.
        //
        //  m_strLoadMsg
        //      We set a field to a status message during the loading process and it
        //      must be translatable, so we pre-load it to avoid having to reload it
        //      all the time.
        //
        //  m_strRepoPath
        //      We get the path to the path during load. This is the path we scan for
        //      files to add to the database.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bLoadPlaylists;
        tCIDLib::TBoolean   m_bUseEmbeddedArt;
        tCIDLib::TCard4     m_c4FldId_DBSerialNum;
        tCIDLib::TCard4     m_c4FldId_Loaded;
        tCIDLib::TCard4     m_c4FldId_LoadStatus;
        tCIDLib::TCard4     m_c4FldId_Status;
        tCIDLib::TCard4     m_c4FldId_ReloadDB;
        tCIDLib::TCard4     m_c4FldId_TitleCnt;
        TMediaDB            m_mdbLoad;
        FileTagRepoLoader*  m_pdblRepo;
        TFTagMediaRepoEng   m_srdbEngine;
        TString             m_strLoadMsg;
        TString             m_strRepoPath;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TFileTagRepoSDriver,TCQCStdMediaRepoDrv)
};

#pragma CIDLIB_POPPACK


