//
// FILE NAME: PlexRepoS_DBLoader.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/29/2015
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
//   CLASS: TPlexRepoLoader
//  PREFIX: dbl
// ---------------------------------------------------------------------------
class TPlexRepoLoader

    : public TThread
    , public MXMLDocEvents
    , public MXMLErrorEvents
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TPlexRepoLoader() = delete;

        TPlexRepoLoader
        (
            const   TString&                strMoniker
            , const TIPEndPoint&            ipepServer
        );

        TPlexRepoLoader(const TPlexRepoLoader&) = delete;

        ~TPlexRepoLoader();


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
        //  Private data types
        // -------------------------------------------------------------------
        using TValStack = TFundStack<tPlexRepoS::EElems>  ;
        using TAttrArray = TObjArray<TString, tPlexRepoS::EAttrs>;


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tPlexRepoS::EElems ePopValStack
        (
            const   TString&                strQName
        );

        tCIDLib::TVoid FindSections
        (
                    tCIDLib::TCardList&     fcolSections
        );

        tCIDLib::TVoid ImportDatabase();

        TMediaImg* pmimgDownloadArt
        (
            const   TString&                strImgPath
            , const TString&                strThumbPath
        );

        tCIDLib::TVoid ProcessSection
        (
            const   tCIDLib::TCard4         c4Index
        );

        const TString& strGetAttr
        (
            const   TVector<TXMLAttr>&      colAttrList
            , const TString&                strToFind
        );

        tCIDLib::TVoid StoreAttrs
        (
            const   TVector<TXMLAttr>&      colAttrList
        );

        tCIDLib::TVoid StoreTitleSet();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4TitlesDone
        //      During the load process we count the titles done so that we can
        //      return that value in the status call. The main driver thread uses
        //      this to update a status field. We don't use the value in the DB
        //      being loaded so that we don't have provide any sync.
        //
        //  m_colAttrVals
        //      A vector, indexed by the EAttrs enum, which we use to store the
        //      attribute values until we are ready to store the data at the end
        //      of the Video element.
        //
        //  m_colCastList
        //      A list of actors names we found in the current movie.
        //
        //  m_colCatList
        //      Categories are loaded into here during parsing.
        //
        //  m_colInHdrLines
        //  m_colOutHdrLines
        //  m_colHTTPQVals
        //      Temp header line lists for the HTTP operations. And a dummy one
        //      for query values, which we don't use.
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
        //  m_httpcImgs
        //      We have to get the images via HTTP, so we need an HTTP client for
        //      that.
        //
        //  m_ipepServer
        //      The IP end point of the server, provided to us by the driver. We'll
        //      use this to set up the m_pcdsSrv data source. We use this to maintain
        //      a persistent connection to the server during the whole loading
        //      process.
        //
        //  m_mbufTmp
        //      A temp buffer for downloading images or other data required.
        //
        //  m_pathTmpX
        //        Some temp path strings for using during loading.
        //
        //  m_pcdsSrv
        //      We create a data source and store it here for everyone to use who
        //      needs to do an HTTP operation. It's really just alive within the
        //      the eProcess() method.
        //
        //  m_pmdbLoading
        //      A pointer to the media database we are loading. We don't own it, we
        //      are given a pointer to an external one and we load it up from the
        //      passed file.
        //
        //  m_strAllMoviesCatName
        //      We have to add each title set to the all movies category, so we store
        //      the name during init for subsequent reuse.
        //
        //  m_strDBSerialNum
        //      At the end we generate a serial number, by flattening the database
        //      and hashing the resulting buffer.
        //
        //  m_strMoniker
        //      The moniker of the driver instance who invoked us, to use in the
        //      creation of our thread name (to make it unique) and for error msgs.
        //
        //  m_strTmp2
        //      Temporary strings for formatting, to avoid creating/dstroying them
        //      over and over again.
        //
        //  m_xprsDB
        //      The XML parser we use. We install ourself as the handler for the
        //      stuff we are interested in. The potential size of the data means we
        //      can't afford the tree browser, so we use the core parser and
        //      handle data in a streaming manner.
        //
        //  m_xptrsTree
        //      We do use a tree browser for a couple smaller preliminary docs we
        //      have to parse to find the final target, so we create one of those
        //      as well.
        // -------------------------------------------------------------------
        tCIDLib::TCard4             m_c4TitlesDone;
        tCIDLib::TStrList           m_colCastList;
        TAttrArray                  m_colAttrVals;
        tCIDLib::TStrList           m_colCatList;
        tCIDLib::TKVPList           m_colInHdrLines;
        tCIDLib::TKVPList           m_colOutHdrLines;
        tCIDLib::TKVPList           m_colHTTPQVals;
        const tCQCMedia::EMediaTypes m_eMediaType;
        tCQCMedia::ELoadStatus      m_eStatus;
        tCQCKit::EVerboseLvls       m_eVerbose;
        tCIDLib::TCardList          m_fcolCatIds;
        TValStack                   m_fcolValStack;
        THTTPClient                 m_httpcImgs;
        TIPEndPoint                 m_ipepServer;
        THeapBuf                    m_mbufTmp;
        TPathStr                    m_pathTmp1;
        TPathStr                    m_pathTmp2;
        TCIDSockStreamDataSrc*      m_pcdsSrv;
        TMediaDB*                   m_pmdbLoading;
        TString                     m_strAllMoviesCatName;
        TString                     m_strDBSerialNum;
        const TString               m_strMoniker;
        TString                     m_strTmp1;
        TString                     m_strTmp2;
        TString                     m_strTmp3;
        TXMLParserCore              m_xprsDB;
        TXMLTreeParser              m_xptrsTree;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TPlexRepoLoader,TThread)
};

#pragma CIDLIB_POPPACK


