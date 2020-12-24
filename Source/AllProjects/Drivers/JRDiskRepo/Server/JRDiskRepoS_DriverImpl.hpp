//
// FILE NAME: JRDiskRepoS_DriverImpl.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/04/2005
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
//  memory media DB, we derive from a base class in the CQCMedia facility that
//  provides support for all the standard queries we get from clients. It knows how
//  to fulfill those out of the database so we don't have to deal with that.
//
//  We do have to provide a simple derivative of the standard database engine so
//  that we can override the cover art loading method. Only we know how to load
//  our cover art.
//
//  The base class holds the in-memory database, so we just call an access
//  method any time we need to access it, which is mainly just during the
//  loading process.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//  CLASS: TJRMCMediaRepoEng
// PREFIX: srdb
// ---------------------------------------------------------------------------
class TJRMCMediaRepoEng : public TCQCStdMediaRepoEng
{
    public  :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TJRMCMediaRepoEng
        (
            const   TString&                strMoniker
        );

        ~TJRMCMediaRepoEng();


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
        RTTIDefs(TJRMCMediaRepoEng,TCQCStdMediaRepoEng)
};



// ---------------------------------------------------------------------------
//   CLASS: JRDiskRepoS
//  PREFIX: drv
// ---------------------------------------------------------------------------
class TJRDiskRepoSDriver : public TCQCStdMediaRepoDrv
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TJRDiskRepoSDriver
        (
            const   TCQCDriverObjCfg&       cqcdcToLoad
        );

        ~TJRDiskRepoSDriver();



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


    private :
        // -------------------------------------------------------------------
        //  Unimplemented constructors and operators
        // -------------------------------------------------------------------
        TJRDiskRepoSDriver();
        TJRDiskRepoSDriver(const TJRDiskRepoSDriver&);
        tCIDLib::TVoid operator=(const TJRDiskRepoSDriver&);


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
        //  m_mdbLoad
        //      To avoid having to keep clients out the whole time while we are loading
        //      a DB, we load into this temp first, then let our engine steal all of
        //      the contents at the end.
        //
        //  m_pdblRepo
        //      This is our background repository loader object. It runs in a separate
        //      thread to load up the media database, while we go about our business.
        //      We allocate it when needed.
        //
        //  m_srdbEngine
        //      We use a standard repo database engine to provide the database and
        //      serving up data to media data clients. We have to provide our own small
        //      derivive to handle art image loading.
        //
        //  m_strLoadMsg
        //      We set a field to a status message during the loading process and it
        //      must be translatable, so we pre-load it to avoid having to reload it
        //      all the time.
        //
        //  m_strXMLFile
        //      We get the J.River library export file as a driver prompt value during
        //      init.
        // -------------------------------------------------------------------
        tCIDLib::TCard4     m_c4FldId_DBSerialNum;
        tCIDLib::TCard4     m_c4FldId_Loaded;
        tCIDLib::TCard4     m_c4FldId_LoadStatus;
        tCIDLib::TCard4     m_c4FldId_ReloadDB;
        tCIDLib::TCard4     m_c4FldId_Status;
        tCIDLib::TCard4     m_c4FldId_TitleCnt;
        TMediaDB            m_mdbLoad;
        TJRDiskRepoLoader*  m_pdblRepo;
        TJRMCMediaRepoEng   m_srdbEngine;
        TString             m_strLoadMsg;
        TString             m_strXMLFile;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TJRDiskRepoSDriver,TCQCStdMediaRepoDrv)
};

#pragma CIDLIB_POPPACK


