//
// FILE NAME: CQCMedia_ThisFacility.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/13/2005
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
//  This file defines facility class for this facility. We provide a lot of
//  helper methods to deal with media renderer and repo drivers and associated
//  data like cookies. The renderer and repo drivers provide fairly extensive
//  backdoor command interfaces, and a lot of these methods are just wrappers
//  around those, to save the caller from the details of building up the
//  query or send data and parsing out the results.
//
//  We provide a media metadata client side cache as well. The client side
//  CQC service (loaded on all CQC enabled systems) downloads media database
//  info from any currently available repositories. We check it regularly to
//  see if new data is available.
//
//  For each repo available, we keep the most recent data available, keyed
//  by the repo moniker. We store them via a counted pointer, and client code
//  can ask for access to data for a given repo. Since it's stored in a cnt
//  ptr, it's good as long as they hold onto it. So we can dump previous data
//  when new data is available, and therefore we only keep one database per
//  repo in our list. Any new database will have a new serial number.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

class TXMLTreeParser;


// ---------------------------------------------------------------------------
//   CLASS: TMDBCacheItem
//  PREFIX: mdbci
// ---------------------------------------------------------------------------
class CQCMEDIAEXPORT TMDBCacheItem
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TMDBCacheItem() = delete;

        TMDBCacheItem
        (
            const   TString&                strRepo
        );

        TMDBCacheItem
        (
            const   TString&                strRepo
            ,       TBinInStream&           strmSrc
        );

        TMDBCacheItem(const TMDBCacheItem&) = delete;
        TMDBCacheItem(TMDBCacheItem&&) = delete;

        ~TMDBCacheItem();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TMDBCacheItem& operator=(const TMDBCacheItem&) = delete;
        TMDBCacheItem& operator=(TMDBCacheItem&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TEncodedTime enctNextCheck() const noexcept
        {
            return m_enctNextCheck;
        }

        tCQCMedia::EMTFlags eMTFlags() const noexcept
        {
            return m_eMTFlags;
        }

        const TMediaDB& mdbData() const noexcept
        {
            return m_mdbData;
        }

        const TString& strDBSerialNum() const noexcept
        {
            return m_strDBSerialNum;
        }

        const TString& strRepoMoniker() const noexcept
        {
            return m_strRepoMoniker;
        }

        tCIDLib::TVoid UpdateCheckTime() noexcept
        {
            m_enctNextCheck = TTime::enctNowPlusSecs(15);
        }


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_enctNextCheck
        //      The next time at which we need to check this one to see if
        //      the data has changed.
        //
        //  m_eMTFlags
        //      The media type flags are returned in the database query as
        //      well, so that clients don't have to do a query just to get
        //      this, which they will often want.
        //
        //  m_mdbData
        //      The media database for this item.
        //
        //  m_strDBSerialNum
        //      The database serial number reported the last time we queried
        //      the data. We init it to zero which is never used, so that
        //      ensures we get initial data.
        //
        //  m_strRepoMoniker
        //      The repo for which we are to get data. This is also the key
        //      for the cache.
        // -------------------------------------------------------------------
        tCIDLib::TEncodedTime   m_enctNextCheck;
        tCQCMedia::EMTFlags     m_eMTFlags;
        TMediaDB                m_mdbData;
        TString                 m_strDBSerialNum;
        TString                 m_strRepoMoniker;
};



// ---------------------------------------------------------------------------
//   CLASS: TFacCQCMedia
//  PREFIX: fac
// ---------------------------------------------------------------------------
class CQCMEDIAEXPORT TFacCQCMedia : public TFacility
{
    public :
        // -------------------------------------------------------------------
        //  Public class types
        // -------------------------------------------------------------------
        using TMDBPtr = TCntPtr<TMDBCacheItem>;


        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static const TString& strMDBKey
        (
            const   TMDBPtr&                cptrSrc
        );


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TFacCQCMedia();

        TFacCQCMedia(const TFacCQCMedia&) = delete;
        TFacCQCMedia(TFacCQCMedia&&) = delete;

        ~TFacCQCMedia();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TFacCQCMedia& operator=(const TFacCQCMedia&) = delete;
        TFacCQCMedia& operator=(TFacCQCMedia&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCheckCookie
        (
            const   TString&                strCookie
            ,       tCQCMedia::ECookieTypes& eType
        )   const;

        tCIDLib::TBoolean bDefCategory
        (
            const   tCQCMedia::EMediaTypes  eDesired
            , const tCQCMedia::EMTFlags     eSupported
            ,       TString&                strName
            ,       TString&                strCookie
            ,       tCQCMedia::EMediaTypes& eMType
            ,       tCIDLib::TCard2&        c2Id
        );

        tCIDLib::TBoolean bFirstCharMatch
        (
            const   TString&                strToCheck
            , const tCIDLib::TCh            chFirst
            ,       tCIDLib::TCh&           chActual
            , const tCIDLib::TBoolean       bNoTrivialWords
        )   const;

        tCIDLib::TBoolean bParsePLItemKey
        (
            const   TString&                strPLItemKey
            ,       TString&                strMoniker
            ,       TString&                strCookie
        );

        tCIDLib::TBoolean bIsAnAllCat
        (
            const   TString&                strCatCookie
            ,       tCQCMedia::EMediaTypes& eType
        )   const;

        tCIDLib::TBoolean bIsSpecialCat
        (
            const   tCIDLib::TCard2         c2CatId
            ,       tCQCMedia::EMediaTypes& eType
        )   const;

        tCIDLib::TBoolean bIsTrivialWord
        (
            const   TString&                strToCheck
        )   const;

        tCIDLib::TBoolean bMakeSortTitle
        (
            const   TString&                strOrgTitle
            ,       TString&                strSortTitle
        )   const;

        tCIDLib::TBoolean bParseChangerLoc
        (
            const   TString&                strLocInfo
            ,       TString&                strMoniker
            ,       tCIDLib::TCard4&        c4Slot
        )   const;

        tCIDLib::TBoolean bQueryAudioFmt
        (
                    tCQCKit::TCQCSrvProxy&  orbcSrv
            , const TString&                strMoniker
            , const TString&                strCookie
            ,       tCIDLib::TCard4&        c4BitDepth
            ,       tCIDLib::TCard4&        c4BitRate
            ,       tCIDLib::TCard4&        c4BitChannels
            ,       tCIDLib::TCard4&        c4SampleRate
        )   const;

        tCIDLib::TBoolean bQueryCurPLItem
        (
                    tCQCKit::TCQCSrvProxy&  orbcSrv
            , const TString&                strMoniker
            ,       tCIDLib::TCard4&        c4Index
            ,       TCQCMediaPLItem&        mpliToFill
        )   const;

        tCIDLib::TBoolean bQueryCurRendArt
        (
            const   TString&                strRendMoniker
            ,       tCIDLib::TCard4&        c4Bytes
            ,       TMemBuf&                mbufToFill
            , const tCIDLib::TBoolean       bLarge
        )   const;

        tCIDLib::TBoolean bQueryLocalCachedArt
        (
            const   TString&                strRepoMoniker
            , const TString&                strCookie
            , const tCIDLib::TBoolean       bLarge
            ,       tCIDLib::TCard4&        c4Bytes
            ,       TMemBuf&                mbufToFill
            ,       TString&                strPerId
        );

        tCIDLib::TBoolean bQueryLocalCachedArt
        (
            const   TString&                strRepoMoniker
            , const TString&                strCookie
            , const tCIDLib::TBoolean       bLarge
            ,       TBitmapImage&           imgToFill
            ,       TString&                strPerId
        );

        tCIDLib::TBoolean bQueryRepoDrvArt
        (
            const   TString&                strRepoMoniker
            , const TString&                strCookie
            , const tCQCMedia::ERArtTypes   eType
            ,       tCIDLib::TCard4&        c4Bytes
            ,       TMemBuf&                mbufToFill
        )   const;

        tCIDLib::TBoolean bQueryRepoDrvArt
        (
                    tCQCKit::TCQCSrvProxy&  orbcSrv
            , const TString&                strRepoMoniker
            , const TString&                strCookie
            , const tCQCMedia::ERArtTypes   eType
            ,       tCIDLib::TCard4&        c4Bytes
            ,       TMemBuf&                mbufToFill
        )   const;

        tCIDLib::TBoolean bQueryRepoDrvArt
        (
                    tCQCKit::TCQCSrvProxy&  orbcSrv
            , const TString&                strRepoMoniker
            , const TString&                strCookie
            , const tCQCMedia::ERArtTypes   eType
            ,       TBitmapImage&           imgToFill
        )   const;

        tCIDLib::TBoolean bQueryRepoDrvArt
        (
            const   TString&                strRepoMoniker
            , const TString&                strCookie
            , const tCQCMedia::ERArtTypes   eType
            ,       TBitmapImage&           imgToFill
        )   const;

        tCIDLib::TBoolean bTestMediaFlags
        (
            const   tCQCMedia::EMediaTypes  eTypeToCheck
            , const tCQCMedia::EMTFlags     eAvailFlags
        )   const;

        tCIDLib::TCard4 c4FormatPLItems
        (
            const   TCQCMediaPLMgr&         mplmSrc
            ,       TMemBuf&                mbufToFill
        );

        tCIDLib::TCard4 c4FormatCurPLItem
        (
            const   TCQCMediaPLMgr&         mplmSrc
            ,       TMemBuf&                mbufToFill
        );

        tCIDLib::TCard4 c4QueryPLItems
        (
                    tCQCKit::TCQCSrvProxy&  orbcSrv
            , const TString&                strMoniker
            ,       TVector<TCQCMediaPLItem>& colPLItems
        );

        tCIDLib::TCard4 c4QueryCatList
        (
                    tCQCKit::TCQCSrvProxy&  orbcSrv
            , const TString&                strRepoMoniker
            , const tCQCMedia::EMediaTypes  eType
            , const tCIDLib::TBoolean       bNoEmpty
            ,       tCQCMedia::TNameList&   colNames
            ,       tCQCMedia::TNameList&   colCatCookies
        )   const;

        tCIDLib::TCard4 c4QueryColItems
        (
                    tCQCKit::TCQCSrvProxy&  orbcSrv
            , const TString&                strRepoMoniker
            , const TString&                strColCookie
            ,       TVector<TMediaItem>&    colItems
        );

        tCIDLib::TCard4 c4QueryRandomCatItems
        (
                    tCQCKit::TCQCSrvProxy&  orbcSrv
            , const TString&                strRepoMoniker
            , const TString&                strCatCookie
            , const tCIDLib::TCard4         c4MaxItems
            ,       TVector<TCQCMediaPLItem>& colToFill
        )   const;

        tCIDLib::TCard4 c4QuerySpecialImg
        (
                    TMemBuf&                mbufToFill
            , const tCQCMedia::ESpcImages   eImage
            , const TCQCSecToken&           sectUser
        )   const;

        tCIDLib::TVoid CreatePersistentId
        (
            const   TMemBuf&                mbufData
            , const tCIDLib::TCard4         c4DataSz
            ,       TString&                strToFill
            , const tCIDLib::TCard2         c2Id = 0
            , const tCIDLib::TEncodedTime   enctStamp = 0
        );

        tCIDLib::TVoid CreatePersistentId
        (
                    TBinInStream&           strmSrc
            , const tCIDLib::TCard4         c4DataSz
            ,       TString&                strToFill
            , const tCIDLib::TCard2         c2Id = 0
            , const tCIDLib::TEncodedTime   enctStamp = 0
        );

        tCQCMedia::ECookieTypes eCheckCookie
        (
            const   TString&                strCookie
            ,       tCQCMedia::EMediaTypes& eType
            ,       tCIDLib::TCard2&        c2CatId
            ,       tCIDLib::TCard2&        c2TitleId
            ,       tCIDLib::TCard2&        c2ColId
            ,       tCIDLib::TCard2&        c2ItemId
        )   const;

        tCIDLib::ELoadRes eGetMediaDB
        (
            const   TString&                strRepo
            , const TString&                strCurSerialNum
            ,       TMDBPtr&                cptrToFill
        );

        tCQCMedia::EMediaTypes eParseCatCookie
        (
            const   TString&                strCookie
            ,       tCIDLib::TCard2&        c2CatId
        )   const;

        tCQCMedia::EMediaTypes eParseColCookie
        (
            const   TString&                strCookie
            ,       tCIDLib::TCard2&        c2CatId
            ,       tCIDLib::TCard2&        c2TitleId
            ,       tCIDLib::TCard2&        c2ColId
        )   const;

        tCQCMedia::EMediaTypes eParseItemCookie
        (
            const   TString&                strCookie
            ,       tCIDLib::TCard2&        c2CatId
            ,       tCIDLib::TCard2&        c2TitleId
            ,       tCIDLib::TCard2&        c2ColId
            ,       tCIDLib::TCard2&        c2ItemId
        )   const;

        tCQCMedia::EMediaTypes eParseTitleCookie
        (
            const   TString&                strCookie
            ,       tCIDLib::TCard2&        c2CatId
            ,       tCIDLib::TCard2&        c2TitleId
        )   const;

        tCQCMedia::EMediaTypes eQueryColDetails
        (
            const   TString&                strRepoMoniker
            , const TString&                strColCookie
            ,       TMediaCollect&          mcolToFill
            ,       TString&                strTitleName
            ,       tCIDLib::TCard4&        c4ColCount
        )   const;

        tCQCMedia::EMediaTypes eQueryColDetails
        (
                    tCQCKit::TCQCSrvProxy&  orbcSrv
            , const TString&                strRepoMoniker
            , const TString&                strColCookie
            ,       TMediaCollect&          mcolToFill
            ,       TString&                strTitleName
            ,       tCIDLib::TCard4&        c4ColCount
        )   const;

        tCQCMedia::ELocTypes eQueryColLoc
        (
                    tCQCKit::TCQCSrvProxy&  orbcSrv
            , const TString&                strRepoMoniker
            , const TString&                strColCookie
            ,       TString&                strLocInfo
        )   const;

        tCQCMedia::EMediaTypes eQueryDefCatInfo
        (
                    tCQCKit::TCQCSrvProxy&  orbcSrv
            , const TString&                strRepoMoniker
            , const tCQCMedia::EMediaTypes  eMType
            ,       TString&                strCatCookie
            ,       TString&                strCatName
        )   const;

        tCIDLib::ELoadRes eQueryLocalCachedArt
        (
            const   TString&                strRepoMoniker
            , const TString&                strUID
            , const TString&                strCurPerId
            , const tCQCMedia::EMediaTypes  eMType
            , const tCIDLib::TBoolean       bLarge
            ,       tCIDLib::TCard4&        c4Bytes
            ,       TMemBuf&                mbufToFill
            ,       TString&                strNewPerId
        );

        tCQCMedia::EMediaTypes eQueryTitleDetails
        (
            const   TString&                strRepoMoniker
            , const TString&                strTitleCookie
            ,       TString&                strName
            ,       TString&                strArtist
            ,       TString&                strMediaFmt
            ,       tCIDLib::TCard4&        c4ColCount
            ,       tCIDLib::TCard4&        c4Year
            ,       tCQCMedia::TNameList&   colNames
            ,       tCQCMedia::TNameList&   colCookies
            ,       tCQCMedia::TNameList&   colArtists
        )   const;

        tCQCMedia::EMediaTypes eQueryTitleDetails
        (
                    tCQCKit::TCQCSrvProxy&  orbcSrv
            , const TString&                strRepoMoniker
            , const TString&                strTitleCookie
            ,       TString&                strName
            ,       TString&                strArtist
            ,       TString&                strMediaFmt
            ,       tCIDLib::TCard4&        c4ColCount
            ,       tCIDLib::TCard4&        c4Year
            ,       tCQCMedia::TNameList&   colNames
            ,       tCQCMedia::TNameList&   colCookies
            ,       tCQCMedia::TNameList&   colArtists
        )   const;

        tCQCMedia::ECompTypes eXlatCmdCompType
        (
            const   TString&                strToXlat
        )   const;

        tCQCMedia::EMediaTypes eXlatCmdMediaType
        (
            const   TString&                strToXlat
        )   const;

        tCQCMedia::ESortOrders eXlatCmdSortOrder
        (
            const   TString&                strToXlat
        )   const;

        tCIDLib::TVoid FormatCatCookie
        (
            const   tCQCMedia::EMediaTypes  eType
            , const tCIDLib::TCard2         c2CatId
            ,       TString&                strCookie
        )   const;

        tCIDLib::TVoid FormatColCookie
        (
            const   tCQCMedia::EMediaTypes  eType
            , const tCIDLib::TCard2         c2CatId
            , const tCIDLib::TCard2         c2TitleId
            , const tCIDLib::TCard2         c2ColId
            ,       TString&                strCookie
        )   const;

        tCIDLib::TVoid FormatDuration
        (
            const   tCIDLib::TCard4         c4Seconds
            ,       TString&                strToFill
        )   const;

        tCIDLib::TVoid FormatItemCookie
        (
            const   tCQCMedia::EMediaTypes  eType
            , const tCIDLib::TCard2         c2CatId
            , const tCIDLib::TCard2         c2TitleId
            , const tCIDLib::TCard2         c2ColId
            , const tCIDLib::TCard2         c2ItemId
            ,       TString&                strCookie
        )   const;

        tCIDLib::TVoid FormatTitleCookie
        (
            const   tCQCMedia::EMediaTypes  eType
            , const tCIDLib::TCard2         c2CatId
            , const tCIDLib::TCard2         c2TitleId
            ,       TString&                strCookie
        )   const;

        tCIDLib::TVoid InitCMLRuntime();

        tCIDLib::TVoid QueryItemDetails
        (
                    tCQCKit::TCQCSrvProxy&  orbcSrv
            , const TString&                strRepoMoniker
            , const TString&                strItemCookie
            ,       TMediaItem&             mitemToFill
        )   const;

        tCIDLib::TVoid QueryRepoCachePath
        (
            const   TString&                strRepoMoniker
            ,       TString&                strToFill
        );

        const TString& strCompTypeOpts() const;

        const TString& strMTypeOpts() const;

        const TString& strSortOpts() const;

        tCIDLib::TVoid StartMDBCacher();

        tCIDLib::TVoid StopMDBCacher();


    private :
        // -------------------------------------------------------------------
        //  Private class types
        // -------------------------------------------------------------------
        using TTrivWordsList = tCIDLib::TStrHashSet;
        using TMediaDBCache = TKeyedHashSet<TMDBPtr, TString, TStringKeyOps>;


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::EExitCodes eMDBCacheThread
        (
                    TThread&                thrThis
            ,       tCIDLib::TVoid*         pData
        );

        tCQCMedia::EMediaTypes eParseCookie
        (
            const   TString&                strCookie
            , const tCQCMedia::ECookieTypes eType
            ,       tCIDLib::TCard2&        c2CatId
            ,       tCIDLib::TCard2&        c2TitleId
            ,       tCIDLib::TCard2&        c2ColId
            ,       tCIDLib::TCard2&        c2ItemId
        )   const;

        tCIDLib::TVoid LoadOpts() const;

        tCIDLib::TVoid ThrowBadMsgFmt
        (
            const   tCIDLib::TCh* const     pszFile
            , const tCIDLib::TCard4         c4Line
            , const TString&                strType
        )  const;



        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_atomOptsLoaded
        //      Used to fault in some option strings below.
        //
        //  m_colMediaDBCache
        //      We provide a caching mechanism for uses of media repository
        //      data. See the class notes above. We set it up as thread safe
        //      and use its mutex for syncing access to it, sincem multiple
        //      client threads may be accessing it.
        //
        //  m_colTrivWords
        //      A hash set that we load up with trivial words that are to
        //      be skipped in title sorting.
        //
        //  m_strCompTypeOpts
        //  m_strMTypeOpts
        //  m_strSortOpts
        //      In any action command that allows the user to select from a
        //      list of enumerated values, we have to provide a list of
        //      values to choose from when setting up the action command. To
        //      avoid a lot of issues we provide some pre-fab lists for some
        //      of our actively used types.
        //
        //      NOTE: These have to be faulted in because if we tried to do
        //            in the ctor we'd cause a circular freakout. So they have
        //            to be mutable. There's no sync required since they are
        //            instance members. m_atomOptsLoaded is our flag.
        //
        //  m_thrMDBCache
        //      The media repo DB cache thread which runs in the background
        //      and keeps our copies of media metadata updated.
        // -------------------------------------------------------------------
        mutable TAtomicFlag m_atomOptsLoaded;
        TMediaDBCache       m_colMediaDBCache;
        TTrivWordsList      m_colTrivWords;
        mutable TString     m_strCompTypeOpts;
        mutable TString     m_strMTypeOpts;
        mutable TString     m_strSortOpts;
        TThread             m_thrMDBCache;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TFacCQCMedia,TFacility)

};

#pragma CIDLIB_POPPACK


