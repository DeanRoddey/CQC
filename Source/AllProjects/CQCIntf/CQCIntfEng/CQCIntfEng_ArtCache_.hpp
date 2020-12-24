//
// FILE NAME: CQCIntfEng_ArtCache_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/17/2013
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
//  This is the header file for the media art cache classes that are used by the
//  facility class to cache art loaded via the local art cache (which is managed
//  by the client service.) This header is private and only used by facilty class
//  cpp file.
//
//  We define a cache entry and the cache itself. The cache has to maintain two
//  list, one providing access via the database key (repo plus cookie) and the
//  other via the persistent id key (repo plus persistent id.) We need fast access
//  via either scheme.
//
//  Note that we ultimately can't guarantee that two persistnt ids won't be used,
//  So for the persistent id list we just use a sorted list and insert things sorted.
//  That's more overhead, but we can't use a hash table because we couldn't then have
//  two with the same persistent id. There should be very little redundancy in the
//  hashed based persistent ids, so sorting/binary searching should be pretty
//  efficient.
//
//  The facility class provdies a helper to load data from the local cache, and it
//  writes that image data through the cache in the process, which greatly speeds
//  things up, particularly in the RIVA server scenario, where there can be multiple
//  clients accessing the image data, so we don't end up loading it multiple times
//  for each client.
//
//  If we are in remote mode, then we will hold onto the raw image data. Later, if
//  a RIVA client asks for the data, and we have not done so yet, we will create a
//  compressed version of the image and re-store that image data and cough it up to
//  RIVA clients thereafter. If we aren't in RIVA mode, we don't keep the data around,
//  see Caveat #1 below.
//
//  Since this class is only used by the facility class, and only in a couple of
//  methods, we just make the members public for the item class, to avoid more
//  code. The cache class itself is really teh only one that updates the items
//  anyway, so it's pretty safe.
//
//  That does mean that this cache has to be thread safe.
//
// CAVEATS/GOTCHAS:
//
//  1)  We don't keep the raw image data around unless we are in remote mode. So,
//      if not in remote mode, the Finalize() method will dump the memory and set
//      the size back to zero. SO DO NOT depend on the size being set, except in
//      remote mode, where it should only be used in the bByPIDKey() method that
//      serves up images to RIVA clients by persistent id.
//
//      Internal use with the IV should always be via the bByDBKey() method, since
//      internally we are working with cookies.
//
// LOG:
//
//  $Log$
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TMArtCacheItem
//  PREFIX: maci
// ---------------------------------------------------------------------------
class TMArtCacheItem
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TMArtCacheItem() = delete ;

        TMArtCacheItem
        (
            const   TString&                strRepo
            , const TString&                strCookie
            , const TString&                strDBKey
            , const tCIDLib::TBoolean       bLarge
        );

        TMArtCacheItem(const TMArtCacheItem&) = delete ;

        ~TMArtCacheItem();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TMArtCacheItem& operator=(const TMArtCacheItem&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid Finalize
        (
            const   TGraphDrawDev&          gdevCompat
            , const TString&                strPerId
            , const TString&                strPIDKey
        );

        tCIDLib::TVoid Reduce();


        // -------------------------------------------------------------------
        //  Public data members
        //
        //  m_bLarge
        //      Whether this is large or thumb art.
        //
        //  m_bReduced
        //      We reduce the bitmap format of the data to JPEG for RIVA clients, but
        //      we don't want to pay the price of that conversion unless it's really
        //      necessary. So we use this flag to remember if we have don it already
        //      or not.
        //
        //  m_bmpArt
        //      The bitmap we create from the provided data.
        //
        //  m_c4ArtSz
        //      The number of bytes in m_mbufArt.
        //
        //  m_enctLast
        //      The last time accessed, for doing LRU discarding of items when the
        //      cache is full.
        //
        //  m_mbufArt
        //      The raw art data. We keep this around for the RIVA server. Initially
        //      it's the raw data originally loaded. If the RIVA client query ever
        //      happens, we do the reduction.
        //
        //  m_strCookie
        //      The cookie, with the indicated repo, for the title or collection that
        //      uses this image.
        //
        //  m_strDBKey
        //      The database key, which is the repo, plus the cookie, plus Lrg/Sml.
        //
        //  m_strPerId
        //      The persistent id of this image.
        //
        //  m_strPIDKey
        //      The persistent id key, which is used by the RIVA server so look up
        //      images by persistent id when RIVA clients ask for them. This is the
        //      repo plus the persistent id plus Lrg/Sml.
        //
        //  m_strRepoMoniker
        //      The repo that this image came from.
        //
        //  m_szOrg
        //      The original image size, which clients may want to know, in case we
        //      scale it.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bLarge;
        tCIDLib::TBoolean       m_bReduced;
        TBitmap                 m_bmpArt;
        tCIDLib::TCard4         m_c4ArtSz;
        tCIDLib::TEncodedTime   m_enctLast;
        THeapBuf                m_mbufArt;
        TString                 m_strCookie;
        TString                 m_strDBKey;
        TString                 m_strPerId;
        TString                 m_strPIDKey;
        TString                 m_strRepoMoniker;
        TSize                   m_szOrg;
};



// ---------------------------------------------------------------------------
//   CLASS: TMArtCache
//  PREFIX: mac
// ---------------------------------------------------------------------------
class TMArtCache
{
    public :
        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static tCIDLib::ESortComps eCompByDBKey
        (
            const   TMArtCacheItem&         maci1
            , const TMArtCacheItem&         maci2
        );

        static tCIDLib::ESortComps eCompByPIDKey
        (
            const   TMArtCacheItem&         maci1
            , const TMArtCacheItem&         maci2
        );

        static tCIDLib::ESortComps eCompByPIDKey2
        (
            const   TString&                strKey
            , const TMArtCacheItem&         maci2
        );

        static tCIDLib::ESortComps eCompByTime
        (
            const   TMArtCacheItem&         maci1
            , const TMArtCacheItem&         maci2
        );

        static const TString& strDBKey
        (
            const   TMArtCacheItem&         maciSrc
        );

        static const TString& strPIDKey
        (
            const   TMArtCacheItem&         maciSrc
        );


        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TMArtCache();

        TMArtCache(const TMArtCache&) = delete;

        ~TMArtCache();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TMArtCache& operator=(const TMArtCache&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bByPIDKey
        (
            const   TString&                strKey
            ,       TMemBuf&                mbufData
            ,       tCIDLib::TCard4&        c4Bytes
            ,       TSize&                  szOrg
        );

        tCIDLib::TBoolean bByDBKey
        (
            const   TGraphDrawDev&          gdevCompat
            , const TString&                strRepo
            , const TString&                strCookie
            , const TSize&                  szTar
            , const tCIDLib::TBoolean       bLarge
            ,       TBitmap&                bmpToFill
            ,       TString&                strKey
            ,       TSize&                  szOrg
        );


    private :
        // -------------------------------------------------------------------
        //  Private class types and constants
        // -------------------------------------------------------------------
        using TDBKeyList = TRefKeyedHashSet<TMArtCacheItem, TString, TStringKeyOps>;
        using TPIDKeyList = TRefVector<TMArtCacheItem>;
        const tCIDLib::TCard4 c4MaxCacheItems = 8192;


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        TMArtCacheItem* pmaciAddNew
        (
            const   TGraphDrawDev&          gdevCompat
            , const TString&                strRepo
            , const TString&                strCookie
            , const TString&                strDBKey
            , const tCIDLib::TBoolean       bLarge
        );

        tCIDLib::TVoid MakeCacheSpace();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_colByDBKey
        //      A hash set of DB keys. These have to be unique since every title in
        //      the database has a unique repo/cookie. This one adopts the items.
        //
        //  m_colByPIDKey
        //      A non-adopting vector that provides our by persistent id lookup
        //      table. This one is a sorted vector.
        //
        //  m_mtxSync
        //      This cache has to be thread safe, due to the RIVA server using it
        //      in the context of multiple clients.
        // -------------------------------------------------------------------
        TDBKeyList  m_colByDBKey;
        TPIDKeyList m_colByPIDKey;
        TMutex      m_mtxSync;
};

#pragma CIDLIB_POPPACK

