//
// FILE NAME: DVDProfilerS_DBLoader.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 12/16/2005
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
//   CLASS: TDVDProfilerLoader
//  PREFIX: dbl
// ---------------------------------------------------------------------------
class TDVDProfilerLoader

    : public TThread
    , public MXMLDocEvents
    , public MXMLErrorEvents
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TDVDProfilerLoader
        (
            const   TString&                strMoniker
        );

        ~TDVDProfilerLoader();


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
            , const TString&                strFilename
            , const TString&                strImagePath
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
            , const tCIDLib::TBoolean       bAllSpaces
        )   final;

        tCIDLib::EExitCodes eProcess() final;

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
        //
        //  We need a simple object to hold disk info till we are ready to
        //  process the title. There can be multiple discs per title. This
        //  is all the per-disc info that is provided.
        //
        class TDiscInfo
        {
            public :
                tCIDLib::TVoid Reset()
                {
                    m_strDescr.Clear();
                    m_strLocInfo.Clear();
                }

                tCIDLib::TBoolean   m_bChanger;
                TString             m_strDescr;
                TString             m_strLocInfo;
        };


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
        //  m_colCatNanes
        //      We store up the list of categories we see during processing of
        //      a disc, until we can get them stored away when we hit the
        //      end of the element.
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
        //      A buffer we us to load up up to a certain amount of the img in order
        //      to generate a persistent id.
        //
        //  m_pathTmp
        //      A temporary path string to use so that we can avoid creating
        //      them over and over in callbacks.
        //
        //  m_pmdbLoading
        //      A pointer to the media database we are loading. We don't own
        //      it, we are given a pointer to an external one and we load it
        //      up from the passed file.
        //
        //  m_strCurXMLPath
        //      In order to know where we are in the XML hierarchy during
        //      the callbacks, we add/remove element names as the start/end
        //      element callbacks are made. So at any time we can check to
        //      see, are we in the /X/Y/Z element right now. This is needed
        //      because the same element name is used in multiple places.
        //
        //  m_strDBSerialNum
        //      We generate a database serial number by flattening the database out
        //      at the end and doing a hash of it. This should do effectively always
        //      generate the same value as long as any of the data that we use has
        //      not changed.
        //
        //  m_strFilename
        //      The file name we were given to load from.
        //
        //  m_strDoubleNL
        //      The format used by DVD Profiler often puts two new lines
        //      between each paragraph, which looks bad when displayed. We
        //      get one of them. So we set up this search string that we can
        //      use to search for them in each description string.
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
        //  m_strThumbsPath
        //      We are given this by the driver. It's the path to the images.
        //      The art paths are made up of this path, plus the ID of the
        //      title, plus the .jpg extension. We also build a thumbs path
        //      from that so that we can check for a pre-existing thumbnail
        //      image and not have to scale one ourself. DVDP has a thumbnails
        //      directory underneath the main image directory.
        //
        //  m_strMoniker
        //      The moniker of the driver instance who invoked us, to use in
        //      the creation of our thread name (to make it unique) and for
        //      error messages.
        //
        //  m_strTmp
        //  m_strTmp2
        //      Temporary strings for various uses, to avoid creating them
        //      over and over in callbacks.
        //
        //  m_tmStamp
        //      We keep this around to parse time stamps.
        //
        //  m_xprsThis
        //      Teh parser we use. We installer ourself as handlers for the
        //      stuff we are interested in.
        // -------------------------------------------------------------------
        tCIDLib::TCard4         m_c4ErrCount;
        tCIDLib::TCard4         m_c4TitlesDone;
        TVector<TDiscInfo>      m_colDiscs;
        tCQCMedia::TNameList    m_colCatNames;
        tCQCMedia::ELoadStatus  m_eStatus;
        tCQCKit::EVerboseLvls   m_eVerbose;
        TTimeCompList           m_fcolStamp;
        THeapBuf                m_mbufPerId;
        TPathStr                m_pathTmp;
        TPathStr                m_pathTmp2;
        TMediaDB*               m_pmdbLoading;
        TString                 m_strCurXMLPath;
        TString                 m_strDBSerialNum;
        TString                 m_strDoubleNL;
        TString                 m_strFilename;
        TString                 m_strFileLocPref;
        TString                 m_strFileLocPrefDx;
        TString                 m_strFld_AspectRatio;
        TString                 m_strFld_Cast;
        TString                 m_strFld_ColNumber;
        TString                 m_strFld_CreditSubtype;
        TString                 m_strFld_Director;
        TString                 m_strFld_FirstName;
        TString                 m_strFld_Genre;
        TString                 m_strFld_LastName;
        TString                 m_strFld_LeadActor;
        TString                 m_strFld_MFmt_BluRay;
        TString                 m_strFld_MFmt_DVD;
        TString                 m_strFld_MFmt_HDDVD;
        TString                 m_strFld_Notes;
        TString                 m_strFld_Overview;
        TString                 m_strFld_Rating;
        TString                 m_strFld_Review;
        TString                 m_strFld_RunningTime;
        TString                 m_strFld_SortTitle;
        TString                 m_strFld_Studio;
        TString                 m_strFld_Timestamp;
        TString                 m_strFld_Title;
        TString                 m_strFld_UniqueID;
        TString                 m_strFld_UPC;
        TString                 m_strFld_Year;
        TString                 m_strImagePath;
        TString                 m_strMediaFmt;
        TString                 m_strMoniker;
        TString                 m_strThumbsPath;
        TString                 m_strTrue;
        TString                 m_strTmp;
        TString                 m_strTmp2;
        TTime                   m_tmStamp;
        TXMLParserCore          m_xprsThis;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TDVDProfilerLoader,TThread)
};

#pragma CIDLIB_POPPACK


