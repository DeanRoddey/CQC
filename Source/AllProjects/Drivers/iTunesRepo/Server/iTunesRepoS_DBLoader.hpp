//
// FILE NAME: iTunesRepoS_DBLoader.hpp
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
//  This is the header file for the database loader class. We are a thread
//  class and the main driver just starts us up and we load into a temp
//  database until we have the whole thing loaded. Once we are done and
//  set our status to indicate this, the main driver will see this and
//  update his database from our temp one. We also implement some XML
//  parser callbacks, since we use the SAX style XML parser interface.
//
// CAVEATS/GOTCHAS:
//
//  1)  Though in theory we could look in the XML and use that to find the
//      cover art in the "Album Artwork" directory. But there is no real
//      discernable relationship between them anymore, though there used
//      to be. So we can really only use embedded artwork.
//
//      The code has been removed but and old copy of the driver is saved
//      in the .\WithITC directory in case it should be figured out later.
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: TiTunesRepoLoader
//  PREFIX: dbl
// ---------------------------------------------------------------------------
class TiTunesRepoLoader

    : public TThread
    , public MXMLDocEvents
    , public MXMLErrorEvents
{
    public :
        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid BuildImgPath
        (
            const   TString&                strLibraryUniqueID
            , const TString&                strTrackUniqueId
            ,       TPathStr&               pathToFill
        );


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TiTunesRepoLoader
        (
            const   TString&                strMoniker
            , const TString&                strDataPath
            , const tCIDLib::TBoolean       bUseAlbumArtist
        );

        ~TiTunesRepoLoader();


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid CompleteLoad();

        tCQCMedia::ELoadStatus eStatus
        (
                    tCIDLib::TCard4&        c4TitlesDone
            ,       tCIDLib::TCard4&        c4TracksIgnored
        )   const;

        tCQCKit::EVerboseLvls eVerboseLevel
        (
            const   tCQCKit::EVerboseLvls   eToSet
        );

        tCIDLib::TVoid StartLoad
        (
                    TMediaDB* const         pmdbToLoad
            , const TPathStr&               pathXMLFile
        );

        tCIDLib::TVoid Shutdown();


    protected :
        // -------------------------------------------------------------------
        //  Protected, virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid DocCharacters
        (
            const   TString&                strChars
            , const tCIDLib::TBoolean       bIsCDATA
            , const tCIDLib::TBoolean       bIsIgnorable
            , const tCIDXML::ELocations     eLocation
        );

        tCIDLib::EExitCodes eProcess();

        tCIDLib::TVoid EndDocument
        (
            const   TXMLEntitySrc&          xsrcOfRoot
        );

        tCIDLib::TVoid EndTag
        (
            const   TXMLElemDecl&           xdeclElem
        );

        tCIDLib::TVoid HandleXMLError
        (
            const   tCIDLib::TErrCode       errcToPost
            , const tCIDXML::EErrTypes      eType
            , const TString&                strText
            , const tCIDLib::TCard4         c4CurColumn
            , const tCIDLib::TCard4         c4CurLine
            , const TString&                strSystemId
        );

        tCIDLib::TVoid ResetDocument();

        tCIDLib::TVoid StartDocument
        (
            const   TXMLEntitySrc&          xsrcOfRoot
        );

        tCIDLib::TVoid StartTag
        (
                    TXMLParserCore&         xprsSrc
            , const TXMLElemDecl&           xdeclElem
            , const tCIDLib::TBoolean       bEmpty
            , const TVector<TXMLAttr>&      colAttrList
            , const tCIDLib::TCard4         c4AttrListSize
        );


    private :
        // -------------------------------------------------------------------
        //  Private class types
        //
        //  The XML is completely retarded, and doesn't use nesting very well
        //  at all.  It just has some separate sections mostly that are just
        //  separated by the presence of an element. So we have to keep up
        //  with where we are in order to know how to interpret the data.
        //
        //  For the playlists, we need a vector of vectors of cardinals. This
        //  stores the list of lists of track ids that define the playlists.
        //  At the end, we'll go through those lists and build up the actual
        //  faux collections by going back to the database to find the
        //  tracks by id. We'll store the playlists names in a parallel vector
        //  of strings.
        //
        //  And we need a hash set of key/value pairs to store the image id
        //  to path translation table.
        // -------------------------------------------------------------------
        enum EFilePos
        {
            EFilePos_None
            , EFilePos_Library
            , EFilePos_Tracks
            , EFilePos_PlayLists
        };

        typedef TFundVector<tCIDLib::TCard4>    TPLIds;
        typedef TVector<TPLIds>                 TPListList;
        typedef TVector<TString>                TPLNames;
        typedef tCIDLib::TKVHashSet             TImageMap;


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid PopXMLPath
        (
            const   TXMLElemDecl&           xdeclEnding
        );

        tCIDLib::TVoid PostProcessing();

        tCIDLib::TVoid ProcessKey
        (
            const   TXMLElemDecl&           xdeclElem
        );

        tCIDLib::TVoid ProcessTrack();



        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bUseAlbumArtist
        //      The user sets a driver prompt that indicates if we should
        //      require use of the Album Artist field. If so, we drop any
        //      that don't provide it and we use album name and album artist
        //      as the unique key. Else we just use the album name as the
        //      key, so any dup album names will end up all together.
        //
        //  m_c2NextCatId
        //  m_c2NextColId
        //  m_c2NextItemId
        //  m_c2NextTitleId
        //      We are responsible for assigning ids to the things we add
        //      to the database. We run these up as we add items. The cat
        //      id starts actually at the first user defined category id
        //      since there are some special ones at the low numbers. The
        //      others start at 1.
        //
        //  m_c2MissingArtId
        //  m_c2PListArtId
        //      The user can put some images into the image repo at specific
        //      paths and we will use them for missing art or for playlists.
        //      When a load starts, we grab these if they have been set and
        //      store the ids here for later use.
        //
        //  m_c4ErrCount
        //      Used to track any errors that aren't XML errors (which we
        //      cannot recover from) but data errors at our semantic level.
        //      We'll keep going, but we want to log an overall error count
        //      at the end.
        //
        //  m_c4TitlesDone
        //  m_c4TracksIgnored
        //      During the load process we count the titles done and tracks
        //      ignored so that we can return that value in the status call.
        //      The main driver thread uses this to update some fields.
        //
        //  m_colCatNanes
        //      We store up the list of categories we see during processing of
        //      a disc, until we can get them stored away when we hit the
        //      end of the element.
        //
        //  m_colImageMap
        //      We need to keep up with the mappings between the unique id
        //      and .itc files, so that we can go back and look them up
        //      as we are processing tracks. We scan the album artwork
        //      directory first and build this list up.
        //
        //  m_colPLNames
        //  m_colPListList
        //      These are used to hold the playlist info. They are parallel
        //      vectors. So index X holds the name and list of track ids,
        //      respectively, of each playlist we find.
        //
        //  m_eFilePos
        //      We keep track of what area of the file we are parsing so that
        //      we know how to interpret things.
        //
        //  m_eStatus
        //      The current load status that the main thread looks at to
        //      know what we are doing.
        //
        //  m_eVerbose
        //      The driver passes us his current verbose logging level, so
        //      that we can report stuff based on that.
        //
        //  m_mbufArt
        //      If any files have embedded images instead of externals, then
        //      we preload them and need a buffer for that.
        //
        //  m_mbufTmp
        //      A temporary buffer for things like expanding URLs and such.
        //
        //  m_mdigId
        //  m_mhashId
        //      We have to generate unique ids for title sets, collections,
        //      and items in this repository. We just do an MD5 hash of the
        //      file name.
        //
        //  m_pathTmp
        //      A temporary path string to use so that we can avoid creating
        //      them over and over in callbacks.
        //
        //  m_pathXMLFile
        //      For convenience, we go ahead and build up the path tot he
        //      XML file, based on the incoming data path.
        //
        //  m_pmdbLoading
        //      A pointer to the media database we are loading. We don't own
        //      it, we are given a pointer to an external one and we load it
        //      up from the passed file.
        //
        //  m_metexAAC
        //  m_metexWMA
        //      We may have to extract an embedded image for files that were
        //      ripped. We create one for each file type we support. The WMA
        //      one also handles MP3.
        //
        //  m_strAllMusicCat
        //      We have to add each title to the all music category, so we
        //      load the text for this category up front and keep it around.
        //
        //  m_strCurXMLPath
        //      In order to know where we are in the XML hierarchy during
        //      the callbacks, we add/remove element names as the start/end
        //      element callbacks are made. So at any time we can check to
        //      see, are we in the /X/Y/Z element right now. This is needed
        //      because the same element name is used in multiple places.
        //
        //  m_strDataPath
        //      The iTunes data path we were given to load from. The XML file
        //      should be directly in this path.
        //
        //  m_strLastKey
        //  m_strLastValue
        //      We basically get almost all the information in the form of
        //      element text, with a key element first and then some value
        //      element. So we store the last key and value we saw here,
        //      and we then process it when we see the end of a value
        //      element.
        //
        //  m_strFld_XXX
        //      As we see the field values, we store them here until we get
        //      to the end of the track element, and then create a new media
        //      track item to store (and possibly a new collection if this is
        //      the first track of a collection.)
        //
        //  m_strMoniker
        //      The moniker of the driver instance who invoked us, to use in
        //      the creation of our thread name (to make it unique) and for
        //      error messages.
        //
        //  m_strTmp
        //  m_strTmp2
        //  m_strTmpID
        //      Temporary strings for various uses, to avoid creating them
        //      over and over in callbacks.
        //
        //  m_tcvtExpand
        //      We have to do a lot of URL expansion, so we pre-allocate a
        //      UTF-8 converter to pass in, avoiding the overhead of the
        //      expansion method creating one over and over.
        //
        //  m_urlLoc
        //      We get the location info as a URL, generally a local file
        //      URL, which we need to parse and expand.
        //
        //  m_xprsThis
        //      The parser we use. We installer ourself as handlers for the
        //      stuff we are interested in.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bUseAlbumArtist;
        tCIDLib::TCard2         m_c2MissingArtId;
        tCIDLib::TCard2         m_c2PListArtId;
        tCIDLib::TCard2         m_c2NextColId;
        tCIDLib::TCard2         m_c2NextCatId;
        tCIDLib::TCard2         m_c2NextImgId;
        tCIDLib::TCard2         m_c2NextItemId;
        tCIDLib::TCard2         m_c2NextTitleId;
        tCIDLib::TCard4         m_c4ErrCount;
        tCIDLib::TCard4         m_c4TitlesDone;
        tCIDLib::TCard4         m_c4TracksIgnored;
        tCQCMedia::TNameList    m_colCatNames;
        TImageMap               m_colImageMap;
        TPLNames                m_colPLNames;
        TPListList              m_colPListList;
        EFilePos                m_eFilePos;
        tCQCMedia::ELoadStatus  m_eStatus;
        tCQCKit::EVerboseLvls   m_eVerbose;
        THeapBuf                m_mbufArt;
        THeapBuf                m_mbufTmp;
        TMessageDigest5         m_mdigId;
        TCIDMetaExtrAAC         m_metexAAC;
        TCIDMetaExtrWMA         m_metexWMA;
        TMD5Hash                m_mhashId;
        TPathStr                m_pathTmp;
        TPathStr                m_pathXMLFile;
        TMediaDB*               m_pmdbLoading;
        TString                 m_strAllMusicCat;
        TString                 m_strCurXMLPath;
        TString                 m_strDataPath;
        TString                 m_strFld_Album;
        TString                 m_strFld_AlbumArtist;
        TString                 m_strFld_Artist;
        TString                 m_strFld_DiscCnt;
        TString                 m_strFld_DiscNum;
        TString                 m_strFld_Genre;
        TString                 m_strFld_LibraryUniqueID;
        TString                 m_strFld_LocInfo;
        TString                 m_strFld_Name;
        TString                 m_strFld_TotalTime;
        TString                 m_strFld_TrackID;
        TString                 m_strFld_TrackCnt;
        TString                 m_strFld_TrackNum;
        TString                 m_strFld_TrackUniqueID;
        TString                 m_strFld_Year;
        TString                 m_strLastKey;
        TString                 m_strLastValue;
        TString                 m_strMoniker;
        TString                 m_strTmp;
        TString                 m_strTmp2;
        TString                 m_strTmpID;
        TUTF8Converter          m_tcvtExpand;
        TURL                    m_urlLoc;
        TXMLParserCore          m_xprsThis;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TiTunesRepoLoader,TThread)
};

#pragma CIDLIB_POPPACK


