//
// FILE NAME: MediaRepoMgr_MetaSrc.hpp
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
//  This is the header for MediaRepoMgr_MetaSrc.cpp, which implements some
//  basic classes used by media metadata sources that we might make use of.
//  Each source will be a derived class so that we can use them generically
//  via a base class. We also define small classes for some info that these
//  sources can return.
//
//  Since a lot of the sources will be XML formatted, the base meta data
//  source class provides a number of helpers to get data out of XML formatted
//  data in a useful way.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: TRepoMetaMatch
//  PREFIX: rmm
// ---------------------------------------------------------------------------
class TRepoMetaMatch : public TObject
{
    public :
        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static tCIDLib::ESortComps eSortByQuality
        (
            const   TRepoMetaMatch&         rmm1
            , const TRepoMetaMatch&         rmm2
        );


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TRepoMetaMatch();

        ~TRepoMetaMatch();


        // -------------------------------------------------------------------
        //  Public methods
        // -------------------------------------------------------------------
        tCIDLib::TCard4 c4Quality() const;

        tCIDLib::TCard4 c4Year() const;

        const TString& strBarcode() const;

        const TString& strEdition() const;

        const TString& strLoadID() const;

        const TString& strMediaType() const;

        const TString& strName() const;

        tCIDLib::TVoid Set
        (
            const   TString&                strLoadID
            , const TString&                strName
            , const TString&                strEdition
            , const tCIDLib::TCard4         c4Year
            , const TString&                strBarcode
            , const TString&                strMediaType
            , const tCIDLib::TCard4         c4Quality
        );


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4Quality
        //      The reported quality of the data. Each source should translate
        //      this to some numeric quality that can be sorted by. The actual
        //      values don't matter, just the relative magnitude for sorting.
        //
        //  m_c4Year
        //      The year of the match
        //
        //  m_strBarcode
        //      The UPC, ASIN, etc... of the match. Each derived class will
        //      provide a text label for the type of code it returns so that
        //      the correct prefix can be displayed to the user.
        //
        //  m_strEdition
        //      Source can sometimes provide extra info about the match,
        //      which we'll add in brackets after the name (for display) if
        //      provided.
        //
        //  m_strLoadID
        //      Not displayed, this is the internal id that we'll use to go
        //      back to the source and load it after the user makes a choice.
        //
        //  m_strMediaType
        //      The media type of the match.
        //
        //  m_strName
        //      The name of the match
        // -------------------------------------------------------------------
        tCIDLib::TCard4     m_c4Quality;
        tCIDLib::TCard4     m_c4Year;
        TString             m_strBarcode;
        TString             m_strEdition;
        TString             m_strLoadID;
        TString             m_strName;
        TString             m_strMediaType;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TRepoMetaMatch,TObject)
};


namespace tMediaRepoMgr
{
    typedef TVector<TRepoMetaMatch>     TMetaList;
}


// ---------------------------------------------------------------------------
//   CLASS: TRepoMetaSrc
//  PREFIX: rms
// ---------------------------------------------------------------------------
class TRepoMetaSrc
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TRepoMetaSrc() = delete;

        TRepoMetaSrc(const TRepoMetaSrc&) = delete;

        ~TRepoMetaSrc();


        // -------------------------------------------------------------------
        //  Unimplemented methods
        // -------------------------------------------------------------------
        TRepoMetaSrc& operator=(const TRepoMetaSrc&) = delete;


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        virtual tCIDLib::TBoolean bBarcodeSearch
        (
            const   TString&                strBarcode
            ,       tMediaRepoMgr::TMetaList& colToFill
            ,       TString&                strFailReason
        ) = 0;

        virtual tCIDLib::TBoolean bDiscIdSearch
        (
            const   TString&                strVolume
            ,       tMediaRepoMgr::TMetaList& colToFill
            , const TKrnlRemMedia::EMediaTypes eMType
            ,       TString&                strFailReason
        ) = 0;

        virtual tCIDLib::TBoolean bLoadTitleById
        (
            const   TString&                strID
            ,       TMediaCollect&          mcolToFill
            ,       TJPEGImage&             imgArt
            ,       tCIDLib::TStrList&      colCatNames
            ,       TString&                strFailReason
        ) = 0;

        virtual tCIDLib::TBoolean bTestConnection
        (
                    tCIDLib::TCard4&        c4QueriesLeft
            ,       TString&                strFailReason
        ) = 0;

        virtual tCIDLib::TBoolean bTitleSearch
        (
            const   TString&                strSearchFor
            ,       tMediaRepoMgr::TMetaList& colToFill
            ,       TString&                strFailReason
        ) = 0;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        const TString& strBarcodeType() const;

        const TString& strSrcName() const;


    protected :
        // -------------------------------------------------------------------
        //  Hidden Constructors
        // -------------------------------------------------------------------
        TRepoMetaSrc
        (
            const   TString&                strSrcName
            , const TString&                strBarcodeType
        );


        // -------------------------------------------------------------------
        //  Protected, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bGetAttr
        (
            const   TXMLTreeElement&        xtnodeSrc
            , const TString&                strAttrName
            ,       TString&                strToFill
        );

        tCIDLib::TCard4 c4GetAttrAsCard
        (
            const   TXMLTreeElement&        xtnodeSrc
            , const TString&                strAttrName
            , const tCIDLib::TCard4         c4Default = 0
        );

        tCIDLib::TCard4 c4GetChildTextAsCard
        (
            const   TXMLTreeElement&        xtnodeSrc
            , const TString&                strChildName
            , const tCIDLib::TCard4         c4Default = 0
        );

        tCIDLib::TCard4 c4GetTextAsCard
        (
            const   TXMLTreeElement&        xtnodeSrc
            , const tCIDLib::TCard4         c4Default = 0
        );

        tCIDLib::TVoid CheckElement
        (
            const   TXMLTreeElement&        xtnodeSrc
            , const TString&                strExpName
        );

        tCIDLib::TVoid GetAttr
        (
            const   TXMLTreeElement&        xtnodeSrc
            , const TString&                strAttrName
            ,       TString&                strToFill
        );

        tCIDLib::TVoid GetAttr
        (
            const   TXMLTreeElement&        xtnodeSrc
            , const TString&                strElemName
            , const TString&                strAttrName
            ,       TString&                strToFill
        );

        tCIDLib::TVoid GetChildText
        (
            const   TXMLTreeElement&        xtnodeSrc
            , const TString&                strChildName
            ,       TString&                strToFill
            , const tCIDLib::TBoolean       bThrowIfNot = kCIDLib::False
        );

        tCIDLib::TVoid GetText
        (
            const   TXMLTreeElement&        xtnodeSrc
            ,       TString&                strToFill
            , const tCIDLib::TBoolean       bThrowIfNot = kCIDLib::False
        );


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_strBarcodeType
        //      When we do a search that can give back more than one match,
        //      we dipslay them. They usually provide some sort of bar code,
        //      but it could be different types (UPC, ASIN, IMDB, etc...) so
        //      the derived class gives us a name that we can use to prefix
        //      the display of the value to the user.
        //
        //  m_strSrcName
        //      The derived class gives us a source name for display purposes
        //      and use in error messages and such.
        // -------------------------------------------------------------------
        TString m_strBarcodeType;
        TString m_strSrcName;
};

#pragma CIDLIB_POPPACK

