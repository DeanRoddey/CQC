//
// FILE NAME: MediaRepoMgr_MyMoviesMetaSrc.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/01/2006
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
//  This is the header for MediaRepoMgr_MyMoviesMetaSrc.cpp, which implements
//  a derivative of the base metadata source class for the My Movies web service.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: TMyMoviesMetaSrc
//  PREFIX: rms
// ---------------------------------------------------------------------------
class TMyMoviesMetaSrc: public TRepoMetaSrc
{
    public :
        // -------------------------------------------------------------------
        //  Destructor
        // -------------------------------------------------------------------
        TMyMoviesMetaSrc() = delete;

        TMyMoviesMetaSrc
        (
            const   TMediaRepoSettings&     mrsOpts
        );

        TMyMoviesMetaSrc(const TMyMoviesMetaSrc&) = delete;

        ~TMyMoviesMetaSrc();


        // -------------------------------------------------------------------
        //  Unimplemented methods
        // -------------------------------------------------------------------
        TMyMoviesMetaSrc& operator=(const TMyMoviesMetaSrc&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bBarcodeSearch
        (
            const   TString&                strBarcode
            ,       tMediaRepoMgr::TMetaList& colToFill
            ,       TString&                strFailReason
        );

        tCIDLib::TBoolean bDiscIdSearch
        (
            const   TString&                strVolume
            ,       tMediaRepoMgr::TMetaList& colToFill
            , const TKrnlRemMedia::EMediaTypes eMType
            ,       TString&                strFailReason
        );

        tCIDLib::TBoolean bLoadTitleById
        (
            const   TString&                strID
            ,       TMediaCollect&          mcolToFill
            ,       TJPEGImage&             imgArt
            ,       tCIDLib::TStrList&      colCatNames
            ,       TString&                strFailReason
        );

        tCIDLib::TBoolean bTestConnection
        (
                    tCIDLib::TCard4&        c4QueriesLeft
            ,       TString&                strFailReason
        );

        tCIDLib::TBoolean bTitleSearch
        (
            const   TString&                strSearchFor
            ,       tMediaRepoMgr::TMetaList& colToFill
            ,       TString&                strFailReason
        );


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bParseMatch
        (
            const   TXMLTreeElement&        xtnodeTitle
            ,       TRepoMetaMatch&         rmmToFill
            ,       TString&                strFailReason
        );

        tCIDLib::TBoolean bParseSearchResult
        (
            const   TXMLTreeElement&        xtnodeRoot
            ,       tMediaRepoMgr::TMetaList& colToFill
            ,       TString&                strFailReason
        );

        tCIDLib::TBoolean bParseTitleResult
        (
            const   TXMLTreeElement&        xtnodeRoot
            ,       TMediaCollect&          mcolToFill
            ,       TJPEGImage&             imgArt
            ,       tCIDLib::TStrList&      colCatNames
            ,       TString&                strFailReason
        );

        tMediaRepoMgr::EMMStatus eGetServerRep
        (
            const   TString&                strCmd
            , const tCIDLib::TKVPList&      colOpts
            , const TString&                strUserName
            , const TString&                strPassword
            ,       TString&                strFailMsg
        );

        const tCIDLib::TCh* pszBoolOptVal
        (
            const   tCIDLib::TBoolean       bVal
        )   const;


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_mbufData
        //      A buffer for any querying of images or XML text we need.
        //
        //  m_mrsOpts
        //      A ref to the repo options, so that we can get our related option
        //      values out.
        //
        //  m_strPassword
        //  m_strUserName
        //      The My Movies user name and password we should use. The user
        //      provides this in the repository configuration.
        //
        //  m_strTmp
        //      A string for temp use internally. Don't assume it stays the
        //      same across any call to another method of this class.
        // -------------------------------------------------------------------
        THeapBuf                    m_mbufData;
        const   TMediaRepoSettings& m_mrsOpts;
        TString                     m_strPassword;
        TString                     m_strUserName;
        TString                     m_strTmp;
        TXMLTreeParser              m_xtprsMeta;


        // -------------------------------------------------------------------
        //  Private static data members
        // -------------------------------------------------------------------
        static const TString    s_strAttr_Barcode;
        static const TString    s_strAttr_Edition;
        static const TString    s_strAttr_ID;
        static const TString    s_strAttr_MediaType;
        static const TString    s_strAttr_Quality;
        static const TString    s_strAttr_Status;
        static const TString    s_strAttr_Title;
        static const TString    s_strAttr_Year;
        static const TString    s_strCmd_BarcodeLookup;
        static const TString    s_strCmd_DiscIDLookup;
        static const TString    s_strCmd_GetRemainingLookups;
        static const TString    s_strCmd_LoadByID;
        static const TString    s_strCmd_TitleLookup;
        static const TString    s_strElem_Reponse;
        static const TString    s_strElem_Title;
        static const TString    s_strElem_Titles;
};

#pragma CIDLIB_POPPACK

