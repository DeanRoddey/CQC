//
// FILE NAME: JRDiskRepoS_DBLoader.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/10/2005
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
//  This is the header file for the database loader class. Because the
//  XML file is very badly designed for efficient loading and can be very
//  large, we load it in a separate thread. So this class derives from the
//  thread class, and just works in the background, with a slightly lowered
//  priority, to build up the media database. We provide a status method
//  so that the main driver class can keep up with where we are. It provides
//  a status field that it keeps up to date with the status.
//
//  Note that we also implement some XML parser mixins, so that we can plug
//  ourself into an XML parser to get callbacks. We don't use the tree
//  parser here because the file could be huge and we cannot afford to have
//  the whole thing in memory at once. So we just do a callback based scheme.
//
//  Since the file is a flat list of elements, we don't have to use any
//  sort of stack. We just use a simple enumerated type that indicates what
//  state we are in, so that we know how to react to element and character
//  data.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: TJRDiskRepoLoader
//  PREFIX: dbl
// ---------------------------------------------------------------------------
class TJRDiskRepoLoader

    : public TThread
    , public MXMLDocEvents
    , public MXMLErrorEvents
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TJRDiskRepoLoader
        (
            const   TString&                strMoniker
        );

        ~TJRDiskRepoLoader();


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
            ,       TString&                strFilename
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
        //  Class types
        // -------------------------------------------------------------------
        enum EFields
        {
            EField_None
            , EField_Album
            , EField_Artist
            , EField_ArtPath
            , EField_BitDepth
            , EField_BitRate
            , EField_Channels
            , EField_DateCreated
            , EField_Description
            , EField_Duration
            , EField_Filename
            , EField_Genre
            , EField_MediaType
            , EField_SampleRate
            , EField_TrackName
            , EField_TrackNum
            , EField_UserRating
            , EField_Year
        };


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bLoadPlaylist
        (
            const   TPathStr&               pathPLFile
            , const TString&                strBaseName
            ,       tCIDLib::TStrList&      colToLoad
        );

        tCIDLib::TCard4 c4XlatCardVal
        (
            const   TString&                strText
            , const tCIDLib::TCard4         c4Def
        );

        tCIDLib::TCard8 c8XlatCardVal
        (
            const   TString&                strText
            , const tCIDLib::TCard8         c8Def
        );

        const TMediaImg* pmimgProcessArtFile
        (
            const   TString&                strPath
        );

        tCIDLib::TVoid ProcessItem();

        tCIDLib::TVoid ProcessPlaylist
        (
            const   TPathStr&               pathFile
            ,       tCIDLib::TStrList&      colTmp
        );

        tCIDLib::TVoid PostProcessing();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bInItem
        //      Just as a simple sanity check, we set this flag when we enter
        //      a track item element, and clear it at the end of the element.
        //      If we see an item start and this flag is set, then we have a
        //      bad file.
        //
        //  m_c2NextXXXId
        //      We have to assign ids for the things that we add into the
        //      in-memory database. So we have to keep up with them as
        //      we load them. For categories, we start at the first user
        //      category id. The others start at 1.
        //
        //      Note that we don't need a next title set id. Because this
        //      repository doesn't provide the info to group collections
        //      into tite sets, each title set just has one collection in
        //      it and therefore the title sets and collections will have
        //      the same ids.
        //
        //  m_c4ErrCount
        //      Used to track any errors that aren't XML errors (which we
        //      cannot recover from) but data errors at our semantic level.
        //      We'll keep going, but we want to log an overall error count
        //      at the end.
        //
        //  m_c4TitlesDone
        //      During the load process we count the titles done so that we
        //      can return that value in the status call. The main driver
        //      thread uses this to update a status field.
        //
        //  m_colPlayLists
        //      After we have processed the main XML file, we look for .m3u file or
        //      .m3u8 files in the same directory. We will loop through those and
        //      add them into the database if valid.
        //
        //      We expect all media paths to be fully qualified in this case.
        //
        //  m_eField
        //      We need to know, in the character data callback, what element
        //      type we are inside. So in the start element, we set this to
        //      indicate what the upcoming element character data is for.
        //
        //  m_eMediaType
        //      We only do music, so we set this in the ctor and use it wherever the
        //      media type is needed, to cut down on verbiage.
        //
        //  m_eStatus
        //      The current load status that the main thread looks at to
        //      know what we are doing.
        //
        //  m_eVerbose
        //      The driver passes us his current verbose logging level, so
        //      that we can report stuff based on that.
        //
        //  m_mbufPerId
        //      For generating persistent ids for images. We pre-allocate it to the
        //      max number of bytes we'll read from an image to create the id, and
        //      read up to that many.
        //
        //  m_mdigId
        //  m_mhashId
        //      We have to generate unique ids for title sets, collections,
        //      and items in this repository. We just do an MD5 hash of the
        //      file name.
        //
        //  m_pathTmp
        //      Just a temp path for local use within methods. Don't expect it to
        //      remain unchanged across any call to another method in this class.
        //
        //  m_pathFld_XXX
        //  m_strFld_XXX
        //      As we see the field values, we store them here until we get
        //      to the end of the item element, and then create a new media
        //      item to store (and possibly a new collection if this is
        //      the first track of a collection.)
        //
        //  m_pmdbLoading
        //      A pointer to the media database we are loading. We don't own
        //      it, we are given a pointer to an external one and we load it
        //      up from the passed file.
        //
        //  m_strAllMusicCatName
        //  m_strPlayListsName
        //      We have to add each title set to the all music category and all
        //      playlists to the playlists category, so we store the names during
        //      init for subsequent reuse.
        //
        //  m_strDBSerialNum
        //      We have to generate a DB serial number for each load, so we store
        //      it here to provide to the driver after loading.
        //
        //  m_strFld_XXX
        //      These are for us to accumulate text content for the various
        //      fields as we get the callbacks. There's no guarantee that the
        //      text content will be reported in single callbacks.
        //
        //  m_strMoniker
        //      The moniker of the driver instance who invoked us, to use in
        //      the creation of our thread name (to make it unique) and for
        //      error messages.
        //
        //  m_strTmp1
        //  m_strTmp2
        //      Temporary strings for formatting, to avoid creating/destroying them over
        //      and over again.
        //
        //  m_strUniqueId
        //      For temporary use in the various places we need to generate unique ids.
        //
        //  m_strXMLBaseName
        //  m_strXMLFile
        //  m_strXMLPath
        //      There are various places where we need to build up new paths based on the
        //      the path and the base name of the original XML file. So we pull these out
        //      up front and keep them around.
        //
        //  m_xprsThis
        //      Teh parser we use. We installer ourself as handlers for the
        //      stuff we are interested in.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean               m_bInItem;
        tCIDLib::TCard4                 m_c4ErrCount;
        tCIDLib::TCard4                 m_c4TitlesDone;
        EFields                         m_eField;
        const tCQCMedia::EMediaTypes    m_eMediaType;
        tCQCMedia::ELoadStatus          m_eStatus;
        tCQCKit::EVerboseLvls           m_eVerbose;
        THeapBuf                        m_mbufPerId;
        TMessageDigest5                 m_mdigId;
        TMD5Hash                        m_mhashId;
        TPathStr                        m_pathFld_ArtPath;
        TPathStr                        m_pathFld_FileName;
        TPathStr                        m_pathTmp;
        TMediaDB*                       m_pmdbLoading;
        TString                         m_strAllMusicCatName;
        TString                         m_strDBSerialNum;
        TString                         m_strFld_Album;
        TString                         m_strFld_Artist;
        TString                         m_strFld_BitDepth;
        TString                         m_strFld_BitRate;
        TString                         m_strFld_Channels;
        TString                         m_strFld_DateCreated;
        TString                         m_strFld_Description;
        TString                         m_strFld_Duration;
        TString                         m_strFld_CatNames;
        TString                         m_strFld_MediaType;
        TString                         m_strFld_SampleRate;
        TString                         m_strFld_TrackName;
        TString                         m_strFld_TrackNum;
        TString                         m_strFld_UserRating;
        TString                         m_strFld_Year;
        TString                         m_strMoniker;
        TString                         m_strPlayListsCatName;
        TString                         m_strTmp1;
        TString                         m_strTmp2;
        TString                         m_strUniqueId;
        TString                         m_strXMLBaseName;
        TString                         m_strXMLFile;
        TString                         m_strXMLPath;
        TXMLParserCore                  m_xprsThis;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TJRDiskRepoLoader,TThread)
};

#pragma CIDLIB_POPPACK


