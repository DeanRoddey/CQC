//
// FILE NAME: iTunesRepo2S_DBLoader.hpp
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
//  This is the header file for the database loader class. We are a thread
//  class and the main driver just starts us up and we load into a temp
//  database until we have the whole thing loaded. Once we are done and
//  set our status to indicate this, the main driver will see this and
//  update his database from our temp one. We also implement some XML
//  parser callbacks, since we use the SAX style XML parser interface.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)


// Forward references
class TiTunesRepo2SDriver;


// ---------------------------------------------------------------------------
//   CLASS: TiTunesRepo2Loader
//  PREFIX: dbl
// ---------------------------------------------------------------------------
class TiTunesRepo2Loader

    : public TThread
    , public MXMLDocEvents
    , public MXMLErrorEvents
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TiTunesRepo2Loader
        (
            const   TString&                strMoniker
            , const TString&                strXMLPath
            ,       TiTunesRepo2SDriver* const psdrvOwner
        );

        ~TiTunesRepo2Loader();


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid CompleteLoad();

        tCQCMedia::ELoadStatus eStatus
        (
                    TString&                strStatus
        )   const;

        tCQCKit::EVerboseLvls eVerboseLevel
        (
            const   tCQCKit::EVerboseLvls   eToSet
        );

        tCIDLib::TVoid StartLoad
        (
                    TMediaDB* const         pmdbToLoad
            , const TIPEndPoint&            ipepTrayApp
        );

        tCIDLib::TVoid Shutdown();


    protected :
        // -------------------------------------------------------------------
        //  Protected, virtual methods
        // -------------------------------------------------------------------
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
        //  An enum to track where we are in the file, so that we know what
        //  to look for and how to react.
        //
        //  And we need some stuff to hold the playlist info until we've got
        //  everything parsed, then we can go back and add them to the data
        //  base. We store the name of each playlist in one vector. And the
        //  other is a vector of vectors of card4 values, which hold the ids
        //  of the tracks that makeup the playlist. So they are parallel
        //  vectors.
        // -------------------------------------------------------------------
        enum EFileParts
        {
            EFilePart_None
            , EFilePart_Repository
            , EFilePart_Library
            , EFilePart_PlayLists
            , EFilePart_PlayList
        };

        typedef TFundVector<tCIDLib::TCard4>    TPLIds;
        typedef TVector<TPLIds>                 TPListList;
        typedef TVector<TString>                TPLNames;
        typedef TRefVector<TMediaItem>          TItemIDList;


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid AddNewPlayList
        (
            const  TXMLElemDecl&            xdeclElem
            , const TVector<TXMLAttr>&      colAttrList
            , const tCIDLib::TCard4         c4AttrCnt
        );

        tCIDLib::TBoolean bGetMsg
        (
                    TClientStreamSocket&    sockTray
            ,       TString&                strCmd
            ,       TString&                strValue
        );

        tCIDLib::TVoid PostProcessing();

        tCIDLib::TVoid ProcessPLTrack
        (
                    TXMLParserCore&         xprsSrc
            , const TXMLElemDecl&           xdeclElem
            , const TVector<TXMLAttr>&      colAttrList
            , const tCIDLib::TCard4         c4AttrCnt
        );

        tCIDLib::TVoid ProcessTrack
        (
                    TXMLParserCore&         xprsSrc
            , const TXMLElemDecl&           xdeclElem
            , const TVector<TXMLAttr>&      colAttrList
            , const tCIDLib::TCard4         c4AttrCnt
        );

        tCIDLib::TVoid SendMsg
        (
                    TClientStreamSocket&    sockTray
            , const TString&                strCmd
            , const TString&                strAttrName
            , const TString&                strAttrVal
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c2NextCatId
        //  m_c2NextColId
        //  m_c2NextItemId
        //      We are responsible for assigning ids to the things we add
        //      to the database. We run these up as we add items. The cat
        //      id starts actually at the first user defined category id
        //      since there are some special ones at the low numbers. The
        //      others start at 1. There is a one to one title set/collection
        //      relationship, so we just have one for collections and give
        //      the title sets the same id as the collection it holds.
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
        //  m_colItemIdList
        //      Though in theory the track ids in the XML would match ours, in
        //      reality they will not, because if we skip any track due to
        //      missing info we'll get out of sync. So, we keep a non-adopting
        //      list of pointers to tracks by id. We store a null pointer if
        //      we don't use an id, so that later we can map quickly from the
        //      play list ids we store while parsing the playlist back to a
        //      track (or a track that can't be used since it was ignored.)
        //
        //  m_colPLNames
        //  m_colPListList
        //      These are used to hold the playlist info. They are parallel
        //      vectors. So index X holds the name and list of track ids,
        //      respectively, of each playlist we find.
        //
        //  m_eStatus
        //      The current load status that the main thread looks at to
        //      know what we are doing.
        //
        //  m_eFilePart
        //      Used to track where we are in the file, based on start/end
        //      tags.
        //
        //  m_eVerbose
        //      The driver passes us his current verbose logging level, so
        //      that we can report stuff based on that.
        //
        //  m_ipepTrayApp
        //      The IP end point of the tray app, given to us by the driver
        //      from the driver configuration.
        //
        //  m_mbufArt
        //      Used to load up image files into if needed.
        //
        //  m_mbufTmp
        //      A temp buffer to use for local use. Don't assume it'll remain
        //      unchanged over any call within this facility.
        //
        //  m_mtxSync
        //      For the most part we need no sync since the only values
        //      exchanged between the loader and driver are fundamental
        //      values. But, we do have to give him a copy of the status
        //      string, so we have to sync that.
        //
        //  m_pathXMLFile
        //      The path of the XML file we were told to tell the tray app
        //      to create.
        //
        //  m_pmdbLoading
        //      A pointer to the media database we are loading. We don't own
        //      it, we are given a pointer to an external one and we load it
        //      up from the passed file.
        //
        //  m_psdrvOwner
        //      A pointer to our owning driver so that we can use some of the
        //      helper methods for reading messages.
        //
        //  m_strAllMusicCat
        //      We have to add each title to the all music category, so we
        //      load the text for this category up front and keep it around.
        //
        //  m_strMoniker
        //      The moniker of the driver instance who invoked us, to use in
        //      the creation of our thread name (to make it unique) and for
        //      error messages.
        //
        //  m_strStatus
        //      We store the incoming status strings during load and give
        //      that info back to the driver. We have to sync access to this
        //      string since both threads access it
        //
        //  m_strTmp
        //  m_strTmp2
        //      Temporary strings for various uses, to avoid creating them
        //      over and over in callbacks.
        //
        //  m_strTr_Album
        //  m_strTr_AlbumID
        //  m_strTr_Artist
        //  m_strTr_ArtPath
        //  m_strTr_Genre
        //  m_strTr_LocInfo
        //  m_strTr_Name
        //      These are used during track parsing, to prevent having to
        //      constantly create and destroy them on each track.
        //
        //  m_xtprsMsg
        //      We use a tree parser for the simple messages that the tray
        //      app sends us. This keeps us from having to parse it.
        // -------------------------------------------------------------------
        tCIDLib::TCard2         m_c2MissingArtId;
        tCIDLib::TCard2         m_c2PListArtId;
        tCIDLib::TCard2         m_c2NextColId;
        tCIDLib::TCard2         m_c2NextCatId;
        tCIDLib::TCard2         m_c2NextImgId;
        tCIDLib::TCard2         m_c2NextItemId;
        tCIDLib::TCard4         m_c4ErrCount;
        tCIDLib::TCard4         m_c4TitlesDone;
        tCIDLib::TCard4         m_c4TracksIgnored;
        tCQCMedia::TNameList    m_colCatNames;
        TItemIDList             m_colItemIDList;
        TPLNames                m_colPLNames;
        TPListList              m_colPListList;
        tCQCMedia::ELoadStatus  m_eStatus;
        EFileParts              m_eFilePart;
        tCQCKit::EVerboseLvls   m_eVerbose;
        TIPEndPoint             m_ipepTrayApp;
        THeapBuf                m_mbufArt;
        THeapBuf                m_mbufTmp;
        TMutex                  m_mtxSync;
        TPathStr                m_pathXMLFile;
        TMediaDB*               m_pmdbLoading;
        TiTunesRepo2SDriver*    m_psdrvOwner;
        TString                 m_strAllMusicCat;
        TString                 m_strMoniker;
        TString                 m_strStatus;
        TString                 m_strTmp;
        TString                 m_strTmp2;
        TString                 m_strTr_AlbumID;
        TString                 m_strTr_AlbumName;
        TString                 m_strTr_Artist;
        TPathStr                m_strTr_ArtPath;
        TString                 m_strTr_Genre;
        TString                 m_strTr_LocInfo;
        TString                 m_strTr_Name;
        TUTF8Converter          m_tcvtMsg;
        TXMLTreeParser          m_xtprsMsg;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TiTunesRepo2Loader,TThread)
};

#pragma CIDLIB_POPPACK


