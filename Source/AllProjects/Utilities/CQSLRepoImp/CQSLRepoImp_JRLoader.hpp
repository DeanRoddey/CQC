//
// FILE NAME: CQSLRepoImp_JRLoader.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/02/2007
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
//  This is the header for the JRiver Profiler loader class. This is a
//  slightly modified ripoff of the loader from the J.River driver.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: TCQSLImpJRLoader
//  PREFIX: dbl
// ---------------------------------------------------------------------------
class TCQSLImpJRLoader : public TObject, public MXMLDocEvents, public MXMLErrorEvents
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQSLImpJRLoader();

        TCQSLImpJRLoader(const TCQSLImpJRLoader&) = delete;
        TCQSLImpJRLoader(TCQSLImpJRLoader&&) = delete;

        ~TCQSLImpJRLoader();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQSLImpJRLoader& operator=(const TCQSLImpJRLoader&) = delete;
        TCQSLImpJRLoader& operator=(TCQSLImpJRLoader&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bLoadIt
        (
                    tCQSLRepoImp::TRepoClient& orbcRepoMgr
            , const TString&                strXMLFile
            , const TString&                strLeaseId
        );


        tCIDLib::EExitCodes eProcess();


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
            , const tCIDLib::TBoolean       bAllSpaces
        )   final;

        tCIDLib::TVoid EndDocument
        (
            const   TXMLEntitySrc&          xsrcOfRoot
        )   final;

        tCIDLib::TVoid EndTag
        (
            const   TXMLElemDecl&           xdeclElem
        )   final;

        tCIDLib::TVoid HandleXMLError
        (
            const   tCIDLib::TErrCode       errcToPost
            , const tCIDXML::EErrTypes      eType
            , const TString&                strText
            , const tCIDLib::TCard4         c4CurColumn
            , const tCIDLib::TCard4         c4CurLine
            , const TString&                strSystemId
        )   final;

        tCIDLib::TVoid ResetDocument() final;

        tCIDLib::TVoid StartDocument
        (
            const   TXMLEntitySrc&          xsrcOfRoot
        )   final;

        tCIDLib::TVoid StartTag
        (
                    TXMLParserCore&         xprsSrc
            , const TXMLElemDecl&           xdeclElem
            , const tCIDLib::TBoolean       bEmpty
            , const TVector<TXMLAttr>&      colAttrList
            , const tCIDLib::TCard4         c4AttrListSize
        )   final;


    private :
        // -------------------------------------------------------------------
        //  Class types
        // -------------------------------------------------------------------
        enum class EFields
        {
            None
            , Album
            , Artist
            , ArtPath
            , Channels
            , Duration
            , Filename
            , Genre
            , MediaType
            , TrackName
            , TrackNum
            , Year
        };


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid PopXMLPath
        (
            const   TXMLElemDecl&           xdeclEnding
        );

        tCIDLib::TVoid ProcessItem();

        tCIDLib::TVoid UploadDB();


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
        //  m_eField
        //      We need to know, in the character data callback, what element
        //      type we are inside. So in the start element, we set this to
        //      indicate what the upcoming element character data is for.
        //
        //  m_imgDefArt
        //      A default image we will upload for any missing art
        //
        //  m_mbufArtUpload
        //      To avoid creating a new buffer for every title, we keep one
        //      around for loading the cover art into for upload.
        //
        //  m_mdbLoading
        //      Because the XML has no nesting to indicate hierarchy, we
        //      have to load the whole thing first in order to know what
        //      is what, and then we do the upload at the end. So we use
        //      the standard in-memory media database to hold the content
        //      until we get it all parsed.
        //
        //  m_mdigId
        //  m_mhashId
        //      We have to generate unique ids for title sets, collections,
        //      and items in this repository. We just do an MD5 hash of the
        //      file name.
        //
        //  m_orbcRepoMgr
        //      The main code that calls us will provide the repository
        //      manager interface to the repository driver we are uploading
        //      to.
        //
        //  m_pathFld_XXX
        //  m_strFld_XXX
        //      As we see the field values, we store them here until we get
        //      to the end of the item element, and then create a new media
        //      item to store (and possibly a new collection if this is
        //      the first track of a collection.)
        //
        //  m_strAllMusicCatName
        //      We have to add each title set to the all music category, so
        //      we store the name during init for subsequent reuse.
        //
        //  m_strCurXMLPath
        //      We use this as a kind of simple push/pop stack so that it
        //      always contains the nesting path of the current element we
        //      are processing. We can use this to know what element we are
        //      on at any time.
        //
        //  m_strFilename
        //      The file name we were given to load from.
        //
        //  m_strLeaseId
        //      The main program gets a lease on the target repository and
        //      gives us the lease id so that we can upload.
        //
        //  m_strTmp1
        //  m_strTmp2
        //      Temporary strings for formatting, to avoid creating/dstroyin
        //      them over and over again.
        //
        //  m_xprsThis
        //      The parser we use. We installer ourself as handlers for the
        //      stuff we are interested in.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean           m_bInItem;
        tCIDLib::TCard2             m_c2NextCatId;
        tCIDLib::TCard2             m_c2NextColId;
        tCIDLib::TCard2             m_c2NextImgId;
        tCIDLib::TCard2             m_c2NextItemId;
        tCIDLib::TCard4             m_c4TitlesDone;
        EFields                     m_eField;
        TJPEGImage                  m_imgDefArt;
        THeapBuf                    m_mbufArtUpload;
        TMediaDB                    m_mdbLoading;
        TMessageDigest5             m_mdigId;
        TMD5Hash                    m_mhashId;
        tCQSLRepoImp::TRepoClient   m_orbcRepoMgr;
        TPathStr                    m_pathFld_ArtPath;
        TPathStr                    m_pathFld_Filename;
        TString                     m_strAllMusicCatName;
        TString                     m_strCurXMLPath;
        TString                     m_strFilename;
        TString                     m_strFld_Album;
        TString                     m_strFld_Artist;
        TString                     m_strFld_Channels;
        TString                     m_strFld_Duration;
        TString                     m_strFld_CatNames;
        TString                     m_strFld_MediaType;
        TString                     m_strFld_TrackName;
        TString                     m_strFld_TrackNum;
        TString                     m_strFld_Year;
        TString                     m_strLeaseId;
        TString                     m_strTmp1;
        TString                     m_strTmp2;
        TString                     m_strUniqueId;
        TXMLParserCore              m_xprsThis;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQSLImpJRLoader,TObject)
};

#pragma CIDLIB_POPPACK


