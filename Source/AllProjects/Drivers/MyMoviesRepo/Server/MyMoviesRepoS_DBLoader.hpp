//
// FILE NAME: MyMoviesRepoS_DBLoader.hpp
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
//  This is the header file for the database loader class. We load it in a separate
//  thread because it can take a while to load. So this class derives from the thread
//  class, and just works in the background, with a slightly lowered priority, to
//  build up the media database. We provide a status method so that the main driver
//  class can keep up with where we are. It provides a status field that it keeps
//  up to date with the status.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: TMyMoviesRepoLoader
//  PREFIX: dbl
// ---------------------------------------------------------------------------
class TMyMoviesRepoLoader : public TThread
{
    public :
        // -------------------------------------------------------------------
        //  We need a little class to use for lookup tables
        // -------------------------------------------------------------------
        class TTblItem
        {
            public :
                TTblItem(const TString& strValue, const tCIDLib::TCard4 c4Num) :

                    m_c4Num(c4Num)
                    , m_strValue(strValue)
                {
                }

                TTblItem(const tCIDLib::TInt4 i4Val, const tCIDLib::TCard4 c4Num) :

                    m_c4Num(c4Num)
                {
                    m_strValue.AppendFormatted(i4Val);
                }

                ~TTblItem()
                {
                }

                tCIDLib::TCard4 m_c4Num;
                TString         m_strValue;
        };
        typedef TVector<TTblItem>   TItemTable;

        // This allows us to do a fast binary search
        static inline
        tCIDLib::ESortComps eComp(  const   tCIDLib::TCard4&    c4Target
                                    , const TTblItem&           itemComp)
        {
            if (c4Target < itemComp.m_c4Num)
                return tCIDLib::ESortComps::FirstLess;
            else if (c4Target > itemComp.m_c4Num)
                return tCIDLib::ESortComps::FirstGreater;
            return tCIDLib::ESortComps::Equal;
        }

        // Just to avoid verbiage we inline this lookup on the collection
        static inline TTblItem* pitemFind(TItemTable& colTable, const tCIDLib::TCard4 c4Id)
        {
            tCIDLib::TCard4 c4Index;
            return colTable.pobjKeyedBinarySearch<tCIDLib::TCard4>(c4Id, &eComp, c4Index);
        }


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TMyMoviesRepoLoader
        (
            const   TString&                strMoniker
            , const TString&                strODBCSrc
            , const TString&                strUserName
            , const TString&                strPassword
            , const TString&                strImagePath
        );

        ~TMyMoviesRepoLoader();


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid CompleteLoad();

        tCQCMedia::ELoadStatus eStatus
        (
                    tCIDLib::TCard4&        c4TitlesDone
        )   const;

        tCQCKit::EVerboseLvls eVerboseLevel
        (
            const   tCQCKit::EVerboseLvls   eToSet
        );

        const TString& strDBSerialNum() const;

        tCIDLib::TVoid StartLoad
        (
                    TMediaDB* const         pmdbToLoad
        );

        tCIDLib::TVoid Shutdown();


    protected :
        // -------------------------------------------------------------------
        //  Protected, virtual methods
        // -------------------------------------------------------------------
        tCIDLib::EExitCodes eProcess();


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid ImportDatabase();

        const TMediaImg* pmimgLoadArt
        (
            const   TTblItem&               itemImg
        );

        const TMediaImg* pmimgLoadDiscArt
        (
            const   TString&                strTitleId
            , const TString&                strPath
            , const tCIDLib::TBoolean       bFileType
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4TitlesDone
        //      During the load process we count the titles done so that we can return
        //      that value in the status call. The main driver thread uses this to
        //      update a status field. We don't use the value in the DB being loaded
        //      so that we don't have provide any sync.
        //
        //  m_eMediaType
        //      We use the media type a good number of times, so just to avoid a lot
        //      of verbiage we store it on ctor and keep it around.
        //
        //  m_eStatus
        //      The current load status that the main thread looks at toknow what we
        //      are doing.
        //
        //  m_eVerbose
        //      The driver passes us his current verbose logging level, so that we
        //      can report stuff based on that.
        //
        //  m_mdigId
        //  m_mhashId
        //      We have to generate unique ids for title sets, collections, and items
        //      in this repository. We just do an MD5 hash of the file name.
        //
        //  m_pathTmpX
        //        Some temp path strings for using during loading.
        //
        //  m_pmdbLoading
        //      A pointer to the media database we are loading. We don't own it, we are
        //      given a pointer to an external one and we load it up from the passed
        //      file.
        //
        //  m_strAllMoviesCatName
        //      We have to add each title set to the all movies category, so we store
        //      the name during init for subsequent reuse.
        //
        //  m_strDBSerialNum
        //      At the end we generate a serial number, by flattening the database
        //      and hashing the resulting buffer.
        //
        //  m_strImagePath
        //      The driver provides us with the path to the cover art, so that we can
        //      create the art paths.
        //
        //  m_strMoniker
        //      The moniker of the driver instance who invoked us, to use in the
        //      creation of our thread name (to make it unique) and for error msgs.
        //
        //  m_strODBCSrc
        //      The ODBC source name we were given to load from.
        //
        //  m_strPassword
        //      The driver provides us with a password to use when accessing the DB.
        //
        //  m_strTmp1
        //  m_strTmp2
        //      Temporary strings for formatting, to avoid creating/dstroying them
        //      over and over again.
        //
        //  m_strUserName
        //      The driver provides us with a user name to use when accessing the DB.
        // -------------------------------------------------------------------
        tCIDLib::TCard4             m_c4TitlesDone;
        const tCQCMedia::EMediaTypes m_eMediaType;
        tCQCMedia::ELoadStatus      m_eStatus;
        tCQCKit::EVerboseLvls       m_eVerbose;
        THeapBuf                    m_mbufTmp;
        TMessageDigest5             m_mdigId;
        TMD5Hash                    m_mhashId;
        TPathStr                    m_pathTmp1;
        TPathStr                    m_pathTmp2;
        TMediaDB*                   m_pmdbLoading;
        TString                     m_strAllMoviesCatName;
        TString                     m_strDBSerialNum;
        const TString               m_strImagePath;
        const TString               m_strMoniker;
        const TString               m_strODBCSrc;
        const TString               m_strPassword;
        TString                     m_strTmp1;
        TString                     m_strTmp2;
        TString                     m_strTmp3;
        const TString               m_strUserName;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TMyMoviesRepoLoader,TThread)
};

#pragma CIDLIB_POPPACK


