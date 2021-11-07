//
// FILE NAME: DVDProfilerS_DriverImpl.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 12/15/2005
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
//
//  DVD Profiler doesn't have any formal way to support file based repositories. So
//  we follow a semi-standard used by the MCE My Movies plugin. The Notes field is
//  set to something like this:
//
//      [ONLINELOCATION="\\server\\share\path\to\FILE.ISO"]
//
//  If more than one disc is involved, then it's like this:
//
//      [ONLINELOCATIOND1="\\maestro\\movies\index\DONNIE_DARKO_D1.ISO"]
//      [ONLINELOCATIOND2="\\maestro\\movies\index\DONNIE_DARKO_D2.ISO"]
//
//  For as many files are are relevant. For changers, there is a formal slot field
//  in DVD Profiler.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//  CLASS: TDVDPMediaRepoEng
// PREFIX: srdb
// ---------------------------------------------------------------------------
class TDVDPMediaRepoEng : public TCQCStdMediaRepoEng
{
    public  :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TDVDPMediaRepoEng
        (
            const   TString&                strMoniker
        );

        TDVDPMediaRepoEng(const TDVDPMediaRepoEng&) = delete;
        TDVDPMediaRepoEng(TDVDPMediaRepoEng&&) = delete;

        ~TDVDPMediaRepoEng();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TDVDPMediaRepoEng& operator=(const TDVDPMediaRepoEng&) = delete;
        TDVDPMediaRepoEng& operator=(TDVDPMediaRepoEng&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TCard4 c4LoadRawCoverArt
        (
            const   TMediaImg&              mimgToLoad
            ,       TMemBuf&                mbufToFill
            , const tCQCMedia::ERArtTypes   eType
        )   override;

    private :
        RTTIDefs(TDVDPMediaRepoEng,TCQCStdMediaRepoEng)
};



// ---------------------------------------------------------------------------
//   CLASS: DVDProfilerS
//  PREFIX: drv
// ---------------------------------------------------------------------------
class TDVDProfilerSDriver : public TCQCStdMediaRepoDrv
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TDVDProfilerSDriver() = delete;

        TDVDProfilerSDriver
        (
            const   TCQCDriverObjCfg&       cqcdcToLoad
        );

        TDVDProfilerSDriver(const TDVDProfilerSDriver&) = delete;
        TDVDProfilerSDriver(TDVDProfilerSDriver&&) = delete;

        ~TDVDProfilerSDriver();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TDVDProfilerSDriver& operator=(const TDVDProfilerSDriver&) = delete;
        TDVDProfilerSDriver& operator=(TDVDProfilerSDriver&&) = delete;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bGetCommResource
        (
                    TThread&                thrThis
        )   final;

        tCIDLib::TBoolean bWaitConfig
        (
                    TThread&                thrThis
        )   final;

        tCQCKit::ECommResults eBoolFldValChanged
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4FldId
            , const tCIDLib::TBoolean       bNewValue
        )   final;

        tCQCKit::ECommResults eConnectToDevice
        (
                    TThread&                thrThis
        )   final;

        tCQCKit::EDrvInitRes eInitializeImpl() final;

        tCQCKit::ECommResults ePollDevice
        (
                    TThread&                thrThis
        )   final;

        tCIDLib::TVoid ReleaseCommResource() final;

        tCIDLib::TVoid TerminateImpl() final;


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
        //  m_mdbLoad
        //      We load into a temporary database and only after a successful load do
        //      we give the data to our database engine.
        //
        //  m_pdblRepo
        //      This is our bgn repo loader object. It runs in a separate thread to
        //      load up the media DB, while we go about our business. We allocate it
        //      when needed.
        //
        //  m_srdbEngine
        //      We use a simple derivative of the standard media database engine from
        //      CQCMedia. We just load it up and it does the rest.
        //
        //  m_strImagePath
        //      The path to the cover art images. They are all kept in a single
        //      directory. The user provides this via a driver prompt.
        //
        //  m_strLoadMsg
        //      We set a field to a status message during the loading process
        //      and it must be translatable, so we pre-load it to avoid having
        //      to reload it all the time.
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
        TDVDProfilerLoader* m_pdblRepo;
        TDVDPMediaRepoEng   m_srdbEngine;
        TString             m_strImagePath;
        TString             m_strLoadMsg;
        TString             m_strXMLFile;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TDVDProfilerSDriver,TCQCStdMediaRepoDrv)
};

#pragma CIDLIB_POPPACK


