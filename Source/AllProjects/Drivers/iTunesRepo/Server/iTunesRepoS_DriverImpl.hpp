//
// FILE NAME: iTunesRepoS_DriverImpl.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/06/2007
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
//  This is the header file for the actual driver class. Since we use the
//  standard in-memory media database, we derive from a base class in the
//  CQCMedia facility that provides support for all the standard queries we
//  get from clients. It knows how to fulfill those out of the database so
//  we don't have to deal with that. We have to provide a virtual method that
//  it will call back if it needs to load up cover art, since it doesn't
//  know how the derived classes access cover art (art is faulted in as
//  needed.)
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
//   CLASS: iTunesRepoS
//  PREFIX: drv
// ---------------------------------------------------------------------------
class TiTunesRepoSDriver : public TCQCStdMediaRepoDrv
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TiTunesRepoSDriver
        (
            const   TCQCDriverObjCfg&       cqcdcToLoad
        );

        ~TiTunesRepoSDriver();


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

        tCIDLib::TCard4 c4LoadCoverArt
        (
            const   TMediaImg&              mimgToLoad
            ,       TMemBuf&                mbufToFill
            , const tCIDLib::TBoolean       bLarge
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
        TiTunesRepoSDriver();
        TiTunesRepoSDriver(const TiTunesRepoSDriver&);
        tCIDLib::TVoid operator=(const TiTunesRepoSDriver&);


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bMonitorLoad();

        tCIDLib::TVoid StartLoad();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bUseAlbumArtist
        //      The user sets a driver prompt that indicates if we should
        //      require use of the Album Artist field. If so, we drop any
        //      that don't provide it and we use album name and album artist
        //      as the unique key. Else we just use the album name as the
        //      key, so any dup album names will end up all together. We pass
        //      this on to the loader.
        //
        //  m_c4FldId_XXX
        //      We look up the ids of our fields after registering them, so
        //      that we can do efficient 'by id' reads/writes.
        //
        //  m_imgLoad
        //      We need to load up and convert images upon demand by our
        //      base class. So to avoid having to constantly create and
        //      destroy an object, we keep this one around.
        //
        //  m_pdblRepo
        //      This is our background repository loader object. It runs
        //      in a separate thread to load up the media database, while
        //      we go about our business. We allocate it when needed.
        //
        //  m_strITunesPath
        //      The path to the iTunes data directory. Under this are the
        //      XML file and images directories that we scan.
        //
        //  m_strLoadMsg
        //      We set a field to a status message during the loading process
        //      and it must be translatable, so we pre-load it to avoid having
        //      to reload it all the time.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bUseAlbumArtist;
        tCIDLib::TCard4         m_c4FldId_Loaded;
        tCIDLib::TCard4         m_c4FldId_ReloadDB;
        tCIDLib::TCard4         m_c4FldId_Status;
        tCIDLib::TCard4         m_c4FldId_TitlesDone;
        tCIDLib::TCard4         m_c4FldId_TracksIgnored;
        TJPEGImage              m_imgLoad;
        TiTunesRepoLoader*      m_pdblRepo;
        TString                 m_strITunesPath;
        TString                 m_strLoadMsg;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TiTunesRepoSDriver,TCQCStdMediaRepoDrv)
};

#pragma CIDLIB_POPPACK


