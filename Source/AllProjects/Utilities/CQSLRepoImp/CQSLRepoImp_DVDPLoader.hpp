//
// FILE NAME: CQSLRepoImp_DVDPLoader.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 12/25/2006
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
//  This is the header for the DVD Profiler loader class. This is a slightly
//  modified ripoff of the loader from the DVD Profiler driver.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: TCQSLImpDVDPLoader
//  PREFIX: dbl
// ---------------------------------------------------------------------------
class TCQSLImpDVDPLoader : public TObject, public MXMLDocEvents, public MXMLErrorEvents
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQSLImpDVDPLoader();

        ~TCQSLImpDVDPLoader();


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bLoadIt
        (
                    tCQSLRepoImp::TRepoClient& orbcRepoMgr
            , const TString&                strXMLFile
            , const TString&                strImagePath
            , const TString&                strLeaseId
            , const tCIDLib::TBoolean       bFileBased
        );


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

        //
        //  We just need a simple object to hold disk info till we are ready
        //  to process the title. There can be multiple discs per title. THis
        //  is all the per-disc info that is provided.
        //
        class TDiscInfo
        {
            public :
                TDiscInfo() {}
                ~TDiscInfo() {}
                tCIDLib::TVoid Reset()
                {
                    m_strDescr.Clear();
                    m_strLocInfo.Clear();
                }

                TString m_strDescr;
                TString m_strLocInfo;
        };

        typedef tCIDLib::TStrHashSet TUniqueIdList;


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bFindAttr
        (
            const   TString&                strQName
            , const TVector<TXMLAttr>&      colAttrList
            , const tCIDLib::TCard4         c4ListSz
            ,       TString&                strValue
        );

        tCIDLib::TVoid PopXMLPath
        (
            const   TXMLElemDecl&           xdeclEnding
        );

        tCIDLib::TVoid ProcessTitle();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bFileBased
        //      Indicates whether we should expect file based or changer based
        //      location info.
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
        //  m_colDiscs
        //      This is where we store the info on the discs of the current
        //      title until we are ready to process them.
        //
        //  m_colCatNames
        //      We store up the list of categories we see during processing of
        //      a disc, until we can get them stored away when we hit the
        //      end of the element.
        //
        //  m_colUniqueIds
        //      We use the DVDP generated ids as the unique ids for the
        //      repository. We have to insure that we don't send him the
        //      same one twice by any chance, so we keep up with all of
        //      them that we have sent so far.
        //
        //  m_imgDefArt
        //      A default image we will upload for any missing art
        //
        //  m_mbufArtUpload
        //      To avoid creating a new buffer for every title, we keep one
        //      around for loading the cover art into for upload.
        //
        //  m_orbcRepoMgr
        //      The main code that calls us will provide the repository
        //      manager interface to the repository driver we are uploading
        //      to.
        //
        //  m_pathTmp
        //      A temporary path string to use so that we can avoid creating
        //      them over and over in callbacks.
        //
        //  m_strAllMoviesCat
        //      We have to add each title to the all movies category, so we
        //      load the text for this category up front and keep it around.
        //
        //  m_strCurXMLPath
        //      In order to know where we are in the XML hierarchy during
        //      the callbacks, we add/remove element names as the start/end
        //      element callbacks are made. So at any time we can check to
        //      see, are we in the /X/Y/Z element right now. This is needed
        //      because the same element name is used in multiple places.
        //
        //  m_strDoubleNL
        //      The format used by DVD Profiler often puts two new lines
        //      between each paragraph, which looks bad when displayed. We
        //      get one of them. So we set up this search string that we can
        //      use to search for them in each description string.
        //
        //  m_strFilename
        //      The file name we were given to load from.
        //
        //  m_strFileLocPref
        //  m_strFileLocPrefDx
        //      The prefix strings used for file-based systems. We set these
        //      up for quick searching.
        //
        //  m_strFld_XXX
        //      As we see the field values, we store them here until we get
        //      to the end of the item element, and then create a new media
        //      track item to store (and possibly a new collection if this is
        //      the first track of a collection.)
        //
        //  m_strImagePath
        //      We are given this. It's the path to the images. The art paths
        //      are made up of this path, plus the ID of the title, plus the
        //      .jpg extension.
        //
        //  m_strLeaseId
        //      The main program gets a lease on the target repository and
        //      gives us the lease id so that we can upload.
        //
        //  m_strTmp
        //  m_strTmp2
        //      Temporary strings for various uses, to avoid creating them
        //      over and over in callbacks.
        //
        //  m_xprsThis
        //      Teh parser we use. We installer ourself as handlers for the
        //      stuff we are interested in.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean           m_bFileBased;
        tCIDLib::TCard4             m_c4ErrCount;
        tCIDLib::TCard4             m_c4TitlesDone;
        TVector<TDiscInfo>          m_colDiscs;
        tCQCMedia::TNameList        m_colCatNames;
        TUniqueIdList               m_colUniqueIds;
        TJPEGImage                  m_imgDefArt;
        THeapBuf                    m_mbufArtUpload;
        tCQSLRepoImp::TRepoClient   m_orbcRepoMgr;
        TPathStr                    m_pathTmp;
        TString                     m_strAllMoviesCat;
        TString                     m_strCurXMLPath;
        TString                     m_strDoubleNL;
        TString                     m_strFilename;
        TString                     m_strFileLocPref;
        TString                     m_strFileLocPrefDx;
        TString                     m_strFld_AspectRatio;
        TString                     m_strFld_Cast;
        TString                     m_strFld_CreditSubtype;
        TString                     m_strFld_Director;
        TString                     m_strFld_Genre;
        TString                     m_strFld_ImgID;
        TString                     m_strFld_LeadActor;
        TString                     m_strFld_Notes;
        TString                     m_strFld_Overview;
        TString                     m_strFld_Rating;
        TString                     m_strFld_RunningTime;
        TString                     m_strFld_SortTitle;
        TString                     m_strFld_CurStudio;
        TString                     m_strFld_Studio;
        TString                     m_strFld_Title;
        TString                     m_strFld_ImageId;
        TString                     m_strFld_UPC;
        TString                     m_strFld_Year;
        TString                     m_strImagePath;
        TString                     m_strLeaseId;
        TString                     m_strTmp;
        TString                     m_strTmp2;
        TXMLParserCore              m_xprsThis;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQSLImpDVDPLoader,TObject)
};

#pragma CIDLIB_POPPACK

