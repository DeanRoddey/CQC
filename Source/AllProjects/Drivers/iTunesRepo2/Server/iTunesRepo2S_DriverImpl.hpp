//
// FILE NAME: iTunesRepo2S_DriverImpl.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/20/2009
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
//  This driver talks to a tray program (that must be running on the
//  same machine) that will talk to iTunes via a COM interface. That will
//  make that program generate an XML file that we will then parse.
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
class TiTunesRepo2SDriver : public TCQCStdMediaRepoDrv
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TiTunesRepo2SDriver
        (
            const   TCQCDriverObjCfg&       cqcdcToLoad
        );

        ~TiTunesRepo2SDriver();


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
        TiTunesRepo2SDriver();
        TiTunesRepo2SDriver(const TiTunesRepo2SDriver&);
        tCIDLib::TVoid operator=(const TiTunesRepo2SDriver&);


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
        //  m_conncfgSock
        //      We pull out our socket configuration info in the ctor for
        //      later use.
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
        //  m_srdbEngine
        //      We use a standard repo database engine to provide the database
        //      and serving up data to media data clients.
        //
        //  m_strStatus
        //      We regularly check the loader's progress and he gives us back
        //      a status string which we dump into the status field
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bUseAlbumArtist;
        tCIDLib::TCard4         m_c4FldId_Loaded;
        tCIDLib::TCard4         m_c4FldId_ReloadDB;
        tCIDLib::TCard4         m_c4FldId_Status;
        TCQCIPConnCfg           m_conncfgSock;
        TJPEGImage              m_imgLoad;
        TiTunesRepo2Loader*     m_pdblRepo;
        TCQCStdMediaRepoEng     m_srdbEngine;
        TString                 m_strStatus;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TiTunesRepo2SDriver,TCQCStdMediaRepoDrv)
};

#pragma CIDLIB_POPPACK


