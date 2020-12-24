//
// FILE NAME: FileTagRepoS_DBLoader.hpp
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
//  This is the header file for the database loader class. This guy will
//  scan the whole directory tree under the user provided repository path
//  and find any media files that it understands and add them to the
//  repository.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: FileTagRepoLoader
//  PREFIX: dbl
// ---------------------------------------------------------------------------
class FileTagRepoLoader : public TThread
{
    public :
        // -------------------------------------------------------------------
        //  Class types
        // -------------------------------------------------------------------
        typedef TFundVector<tCIDLib::TCard4>                    TCard4List;
        typedef TVector<TPathStr>                               TPathList;
        typedef TVector<TString>                                TStrList;
        typedef TFundVector<tCIDLib::TCard4>                    TTimeList;
        typedef TKeyedHashSet<TPathList,TString,TStringKeyOps>  TPlayLists;


        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static const TString& strPLKeyExtract
        (
            const   TPathList&              colPL
        );


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        FileTagRepoLoader
        (
            const   TString&                strMoniker
            , const tCIDLib::TBoolean       bLoadPlaylists
            , const tCIDLib::TBoolean       bUseEmbeddedArt
        );

        ~FileTagRepoLoader();


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
            ,       TString&                strRepoPath
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
        tCIDLib::TVoid PostProcessing();

        tCIDLib::TVoid ProcessFile
        (
            const   TFindBuf&               fndbSearch
            ,       tCIDLib::TCard4&        c4TrackCnt
            ,       TCIDMetaExtractor&      metexToUse
        );

        tCIDLib::TVoid ProcessDir
        (
            const   TString&                strPath
        );

        tCIDLib::TVoid ProcessPlaylist
        (
            const   TPathStr&               pathPLFile
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bLoadPlaylists
        //      We can be told to ignore playlist files or to load them, as the user
        //      wants. This is set in the ctor.
        //
        //  m_bUseEmbeddedArt
        //      We can be told whether or not to use embedded artwork. If so, we check
        //      that first and then look for an external file if not found. If not,
        //      we always look for an ext file.
        //
        //  m_c4ArtSize
        //      On a new directory, we zero this. The first file that has any art,
        //      we take that one and set this to the size. Zero at the end of the
        //      scan means we never found any art in any of the files.
        //
        //  m_c4ErrCount
        //      Used to track any non-fatal errors. We'll keep going, but we want to
        //      log an overall error count at the end.
        //
        //  m_c4TitlesDone
        //      During the load process we count the titles done so that we can return
        //      that value in the status call. The main driver thread uses this to
        //      update a status field.
        //
        //  m_colPlayLists
        //      Any time we find an .m3u file or .m3u8 file, we assume this represents
        //      a playlist. Each one is added to this list, and the media files it
        //      refs (fully expanded) are added to that entry. If m_bLoadPlaylists is
        //      False, we don't load this one at all during directory scanning.
        //
        //      At the end after the whole database is loaded, we go back and create
        //      the new playlist collections based on what we stored here.
        //
        //      We store the name of the playlist in the first slot of each playlist.
        //      We use a keyed hash set to avoid dups, and our key extraction function
        //      just pulls out the 0th element.
        //
        //  m_colXXX
        //  m_fcolXXX
        //      On entry to a new directory, we scan each candidate file that we find.
        //      For title/collection level data we just take the first values that we
        //      find, but we need to also collect any item level info for each file
        //      which we put into these lists
        //
        //  m_eMediaType
        //      To avoid using the full spelled out media type over and over we just
        //      set it here and use this. We only support music so it never changes.
        //
        //  m_eStatus
        //      The current load status that the main thread looks at to know what
        //      we are doing.
        //
        //  m_eVerbose
        //      The driver passes us his current verbose logging level, so that we
        //      can report stuff based on that.
        //
        //  m_mbufTmp
        //      A temporary buffer used for streaming in data where needed.
        //
        //  m_mbufArt
        //      Used to pull the cover art so that we don't have to keep creating
        //      and destroying a buffer for each file. m_c4ArtSize indicates how
        //      much data is in the buffer.
        //
        //  metexXXX
        //      Metadata extractors for each of the types we support. They are
        //      reusable and just process a file in a single call, so we just pass
        //      each file to the appropriate one.
        //
        //  m_mdigId
        //  m_mhashId
        //      We have to generate unique ids for title sets, collections, and items
        //      in this repository. We just do an MD5 hash of the file name.
        //
        //  m_pathTmp1
        //  m_pathTmp2
        //      Some temp paths, mostly used during playlist file processing.
        //
        //  m_pmdbLoading
        //      A pointer to the media database we are loading. We don't own
        //      it, we are given a pointer to an external one and we load it
        //      up from the passed file.
        //
        //  m_strAllMusicCatName
        //      We have to add each title set to the all music category, so
        //      we store the name during init for subsequent reuse.
        //
        //  m_strAlbum
        //  m_strAlbumArtist
        //  m_strDescr
        //      These are used to store the title/collection level info. We take this
        //      info out of the first file that contains it. The info is read into
        //      tmp strings and copied to these if they haven't been set yet.
        //
        //  m_strDBSerialNum
        //      Once the load completes, we generate a serial number by flattening
        //      the database and hashing the buffer.
        //
        //  m_strRepoPath
        //      The path to the repository, i.e. the path that we scan for files.
        //
        //  m_strMoniker
        //      The moniker of the driver instance who invoked us, to use in the
        //      creation of our thread name (to make it unique) and for error msgs.
        //
        //  m_strTmpX
        //      Temporary strings for formatting and for extracting metadata before
        //      we store it, to avoid creating/dstroying them over and over again.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bLoadPlaylists;
        tCIDLib::TBoolean       m_bUseEmbeddedArt;
        tCIDLib::TCard4         m_c4ErrCount;
        tCIDLib::TCard4         m_c4ArtSize;
        tCIDLib::TCard4         m_c4TitlesDone;
        TStrList                m_colArtists;
        TPathList               m_colFiles;
        TStrList                m_colGenres;
        TPlayLists              m_colPlayLists;
        TStrList                m_colTracks;
        tCQCMedia::EMediaTypes  m_eMediaType;
        tCQCMedia::ELoadStatus  m_eStatus;
        tCQCKit::EVerboseLvls   m_eVerbose;
        TCard4List              m_fcolBitRates;
        TTimeList               m_fcolTimes;
        TCard4List              m_fcolTrackNums;
        TCard4List              m_fcolYears;
        THeapBuf                m_mbufArt;
        THeapBuf                m_mbufTmp;
        TMessageDigest5         m_mdigId;
        TCIDMetaExtrAAC         m_metexAAC;
        TCIDMetaExtrWMA         m_metexWMA;
        TMD5Hash                m_mhashId;
        TPathStr                m_pathTmp1;
        TPathStr                m_pathTmp2;
        TMediaDB*               m_pmdbLoading;
        TString                 m_strAlbum;
        TString                 m_strAlbumArtist;
        TString                 m_strDBSerialNum;
        TString                 m_strDescr;
        TString                 m_strMoniker;
        TString                 m_strRepoPath;
        TString                 m_strTmp1;
        TString                 m_strTmp2;
        TString                 m_strTmp3;
        TString                 m_strTmp4;
        TString                 m_strTmp5;
        TString                 m_strTmp6;
        TString                 m_strTmp7;
        TString                 m_strUniqueId;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(FileTagRepoLoader,TThread)
};

#pragma CIDLIB_POPPACK


