//
// FILE NAME: CQCIntfEng_ArtCache.cpp
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
//  This is the implementation file for the classes that make up our cover art
//  cache, maintained by the facility class.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCIntfEng_.hpp"
#include    "CQCIntfEng_ArtCache_.hpp"


// ---------------------------------------------------------------------------
//   CLASS: TMArtCacheItem
//  PREFIX: maci
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TMArtCacheItem: Constructors and Destructor
// ---------------------------------------------------------------------------

//
//  The cache class will set the two keys. It has to gen them up in order to check
//  for dups, so no need to do it again.
//
TMArtCacheItem::TMArtCacheItem( const   TString&            strRepo
                                , const TString&            strCookie
                                , const TString&            strDBKey
                                , const tCIDLib::TBoolean   bLarge) :
    m_bLarge(bLarge)
    , m_bReduced(kCIDLib::False)
    , m_c4ArtSz(0)
    , m_enctLast(TTime::enctNow())
    , m_strCookie(strCookie)
    , m_strDBKey(strDBKey)
    , m_strRepoMoniker(strRepo)
{
}

TMArtCacheItem::~TMArtCacheItem()
{
}


// ---------------------------------------------------------------------------
//  TMArtCacheItem: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  This is called after a load on us is done successfully by the cache. We create
//  the bitmap, if we can, and store away the ids. He already had to create the
//  ids so we he just passes us the PID key he already has.
//
//  If we are not in remote mode, then no RIVA clients are ever going to come
//  calling for the data. So if not, we dump the memory buffer contents
//
tCIDLib::TVoid
TMArtCacheItem::Finalize(const  TGraphDrawDev&  gdevCompat
                        , const TString&        strPerId
                        , const TString&        strPIDKey)
{
    // Store the persistent id stuff
    m_strPerId = strPerId;
    m_strPIDKey = strPIDKey;

    // Create the bitmap from the art data, if we got any
    if (m_c4ArtSz)
    {
        try
        {
            TCIDImage* pimgNew = 0;
            if (facCIDImgFact().bDecodeImg(m_mbufArt, m_c4ArtSz, pimgNew))
            {
                TJanitor<TCIDImage> janImage(pimgNew);
                m_bmpArt = TBitmap(gdevCompat, *pimgNew);
                m_szOrg = pimgNew->szImage();
            }
             else
            {
                m_bmpArt.Reset();
                m_szOrg.Zero();
            }
        }

        catch(...)
        {
            m_bmpArt.Reset();
            m_szOrg.Zero();
        }

        //
        //  If not in remote mode, dump the data now, since it'll never be used
        //  again. We keep the size value though, so we know we loaded an image.
        //
        if (!TFacCQCIntfEng::bRemoteMode())
            m_mbufArt.Reallocate(1);
    }

    // Set the PID key in the user data on the bitmap for the RIVA system
    m_bmpArt.strUserData(m_strPIDKey);
}


tCIDLib::TVoid TMArtCacheItem::Reduce()
{
    if (!m_bReduced && m_c4ArtSz)
    {
        m_bReduced = kCIDLib::True;

        //
        //  Create the compressed version for the RIVA client. this will
        //  update the data in place and we store the new size.
        //
        m_c4ArtSz = facCIDImgFact().c4CompactToJPEG
        (
            m_mbufArt, m_c4ArtSz, m_bmpArt.szBitmap()
        );
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TMArtCache
//  PREFIX: mac
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TMArtCache: Public, static methods
// ---------------------------------------------------------------------------

tCIDLib::ESortComps
TMArtCache::eCompByDBKey(const  TMArtCacheItem& maci1
                        , const TMArtCacheItem& maci2)
{
    return maci1.m_strDBKey.eCompare(maci2.m_strDBKey);
}

tCIDLib::ESortComps
TMArtCache::eCompByPIDKey(  const   TMArtCacheItem& maci1
                            , const TMArtCacheItem& maci2)
{
    return maci1.m_strPIDKey.eCompare(maci2.m_strPIDKey);
}

tCIDLib::ESortComps
TMArtCache::eCompByPIDKey2( const   TString&        strKey
                            , const TMArtCacheItem& maci2)
{
    return strKey.eCompare(maci2.m_strPIDKey);
}

tCIDLib::ESortComps
TMArtCache::eCompByTime(const   TMArtCacheItem& maci1
                        , const TMArtCacheItem& maci2)
{
    if (maci1.m_enctLast < maci2.m_enctLast)
        return tCIDLib::ESortComps::FirstLess;
    else if (maci1.m_enctLast > maci2.m_enctLast)
        return tCIDLib::ESortComps::FirstGreater;
    return tCIDLib::ESortComps::Equal;
}


const TString& TMArtCache::strDBKey(const TMArtCacheItem& maciSrc)
{
    return maciSrc.m_strDBKey;
}


const TString& TMArtCache::strPIDKey(const TMArtCacheItem& maciSrc)
{
    return maciSrc.m_strPIDKey;
}



// ---------------------------------------------------------------------------
//  TMArtCache: Constructors and destructor
// ---------------------------------------------------------------------------
TMArtCache::TMArtCache() :

    m_colByDBKey(tCIDLib::EAdoptOpts::Adopt, 491, TStringKeyOps(), &strDBKey)
    , m_colByPIDKey(tCIDLib::EAdoptOpts::NoAdopt)
{
}

TMArtCache::~TMArtCache()
{
}


// ---------------------------------------------------------------------------
//  TMArtCache: Public, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TMArtCache::bByPIDKey(  const   TString&            strPIDKey
                        ,       TMemBuf&            mbufData
                        ,       tCIDLib::TCard4&    c4Bytes
                        ,       TSize&              szOrg)
{
    TLocker lockrSync(&m_mtxSync);

    // We have to do a binary search for this one
    tCIDLib::TCard4 c4At;
    TMArtCacheItem* pmaciArt = m_colByPIDKey.pobjKeyedBinarySearch
    (
        strPIDKey, &eCompByPIDKey2, c4At
    );

    // If not found or no data, return false
    if (!pmaciArt || !pmaciArt->m_c4ArtSz)
    {
        c4Bytes = 0;
        return kCIDLib::False;
    }

    // Update this one's last access stamp
    pmaciArt->m_enctLast = TTime::enctNow();

    // If not reduced yet, do that
    if (!pmaciArt->m_bReduced)
        pmaciArt->Reduce();

    // Else give back the bytes if we have any
    c4Bytes = pmaciArt->m_c4ArtSz;
    if (c4Bytes)
    {
        if (mbufData.c4Size() < c4Bytes)
            mbufData.Reallocate(c4Bytes, kCIDLib::False);
        mbufData.CopyIn(pmaciArt->m_mbufArt, c4Bytes);
    }

    // Give back the original size
    szOrg = pmaciArt->m_szOrg;

    // If we returned bytes, indicate that
    return (c4Bytes != 0);
}


tCIDLib::TBoolean
TMArtCache::bByDBKey(const  TGraphDrawDev&      gdevCompat
                    , const TString&            strRepo
                    , const TString&            strCookie
                    , const TSize&              szTar
                    , const tCIDLib::TBoolean   bLarge
                    ,       TBitmap&            bmpToSet
                    ,       TString&            strKey
                    ,       TSize&              szOrg)
{
    // Assume no data until proven otherwise
    szOrg.Zero();

    // Build up the DB key
    TFacCQCIntfEng::MakeArtDBKey(strRepo, strCookie, bLarge, strKey);

    // See if this guy already exists
    TLocker lockrSync(&m_mtxSync);
    TMArtCacheItem* pmaciArt = m_colByDBKey.pobjFindByKey(strKey, kCIDLib::False);

    //
    //  If not loaded yet, then try to load it. We need to release the lock while
    //  doing this, so as not to block other users. The called method will lock
    //  before actually storing and will deal with the fact that another thread
    //  may have beat him to the punch.
    //
    if (!pmaciArt)
    {
        lockrSync.Release();
        pmaciArt = pmaciAddNew(gdevCompat, strRepo, strCookie, strKey, bLarge);

        // Gain control again
        lockrSync.Lock();
    }

    // If still nothing, then we failed
    if (!pmaciArt)
        return kCIDLib::False;

    // Update this one's last access stamp
    pmaciArt->m_enctLast = TTime::enctNow();

    // If we got any art, then let's set up the caller's bitmap
    if (pmaciArt->m_c4ArtSz)
    {
        //
        //  If the passed target size is zero, then we give back the natural size of
        //  the image. Else we scale the image to fit the target, keeping AR.
        //
        TSize szImg = pmaciArt->m_bmpArt.szBitmap();
        if (szTar.bAllZero())
        {
            // No scaling involved, so just do an assigment and deep copy
            bmpToSet = pmaciArt->m_bmpArt;
            bmpToSet.DeepCopy();
        }
         else
        {
            TSize szNew = szImg;
            szNew.ScaleAR(szTar);

            //
            //  Give the caller a copy of our image in the size he wants. We have to
            //  allocate the bitmap of the correct size first, since we are just
            //  drawing into it.
            //
            bmpToSet = TBitmap(szNew, tCIDImage::EPixFmts::TrueClr, tCIDImage::EBitDepths::Eight);
            {
                TGraphMemDev gdevTar(gdevCompat, bmpToSet);
                TArea areaSrc(TPoint::pntOrigin, szImg);
                TArea areaTar(TPoint::pntOrigin, szNew);
                gdevTar.DrawBitmap(pmaciArt->m_bmpArt, areaSrc, areaTar);
            }
        }

        // Give back the original size
        szOrg = pmaciArt->m_szOrg;
    }

    // Set the persistent id key on the new image so RIVA clients can find it
    bmpToSet.strUserData(pmaciArt->m_strPIDKey);

    return (pmaciArt->m_c4ArtSz != 0);
}


// ---------------------------------------------------------------------------
//  TMArtCache: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  The caller unlocks before calling us here so that we don't hold up other threads
//  while loading art. However, this means that there's a small but possible chance
//  that another thread will load it at the same time and beat us. So we check first
//  before adding, after we get the lock.
//
TMArtCacheItem*
TMArtCache::pmaciAddNew(const   TGraphDrawDev&      gdevCompat
                        , const TString&            strRepo
                        , const TString&            strCookie
                        , const TString&            strDBKey
                        , const tCIDLib::TBoolean   bLarge)
{
    //
    //  We are unlocked now, so we can make a query for the art. Temporarily create a
    //  new item which we will fill in if we can get the image data.
    //
    TMArtCacheItem* pmaciNew = new TMArtCacheItem(strRepo, strCookie, strDBKey, bLarge);
    TJanitor<TMArtCacheItem> janItem(pmaciNew);

    //
    //  Ask the media facility for the cached data. We read it straight into the
    //  cache item's memory buffer to avoid a copy.
    //
    TString strPerId;
    tCIDLib::TBoolean bRes = facCQCMedia().bQueryLocalCachedArt
    (
        strRepo
        , strCookie
        , bLarge
        , pmaciNew->m_c4ArtSz
        , pmaciNew->m_mbufArt
        , strPerId
    );

    // Make the PID key now
    TString strPIDKey;
    TFacCQCIntfEng::MakeArtPIDKey(strRepo, strPerId, bLarge, strPIDKey);

    //
    //  If not available, we still add an item, so that we won't continue trying
    //  to load this image. We'll just return an empty image. Else, finalize the
    //  item which will create the image and store some more info.
    //
    if (bRes)
    {
        pmaciNew->Finalize(gdevCompat, strPerId, strPIDKey);
    }
     else
    {
        // Mkae sure there is no art
        pmaciNew->m_c4ArtSz = 0;
        pmaciNew->m_szOrg.Zero();
        pmaciNew->m_bmpArt.Reset();
    }

    //
    //  And now add this guy to the lists. It's possible another thread already did
    //  it, though unlikely. But we have to check after we get the lock.
    //
    try
    {
        TLocker lockrSync(&m_mtxSync);
        if (!m_colByDBKey.pobjFindByKey(strDBKey, kCIDLib::False))
        {
            // Make sure the cache isn't full. If so, make space
            if (m_colByDBKey.bIsFull(this->c4MaxCacheItems))
                MakeCacheSpace();

            // Add it to the adopting hash table, orphaning from the janitor
            m_colByDBKey.Add(janItem.pobjOrphan());

            // And do a sorted insert on the PID list
            tCIDLib::TCard4 c4At;
            m_colByPIDKey.InsertSorted(pmaciNew, &eCompByPIDKey, c4At);
        }
    }

    catch(...)
    {
    }

    return pmaciNew;
}


// Clear out some space in the cache, throwing out the least recently used ones
tCIDLib::TVoid TMArtCache::MakeCacheSpace()
{
    //
    //  Shouldn't be called unless we are full, certainly not when we have less items
    //  than the number we are going to remove.
    //
    const tCIDLib::TCard4 c4RemoveCnt = 128;
    if (m_colByDBKey.c4ElemCount() <= c4RemoveCnt)
        return;

    //
    //  We use the PID list since we can directly index it, and it doesn't matter
    //  which. We go through and create a new non-adopting list, and sort it by
    //  the last access time.
    //
    tCIDLib::TCard4 c4Count = m_colByPIDKey.c4ElemCount();

    // Get a non-adopting ref vector of the image cache items
    TPIDKeyList colSort(tCIDLib::EAdoptOpts::NoAdopt, m_colByPIDKey.c4ElemCount());
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        colSort.Add(m_colByPIDKey[c4Index]);

    // Sort the list by time
    colSort.Sort(&eCompByTime);

    //
    //  Get the first remove count keys out. Doing it this way we don't have any
    //  issues of removing the objects that contains these keys.
    //
    tCIDLib::TStrList colKeys(c4RemoveCnt);
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4RemoveCnt; c4Index++)
        colKeys.objAdd(colSort[c4Index]->m_strPIDKey);

    // And now delete all of the keys we got out
    TString strDBKey;
    c4Count = colKeys.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TString& strKey = colKeys[c4Index];

        //
        //  Find it in the PID list and remove it. This one doesn't own it so we
        //  want to do this one first. Get the DB key out before we do so that we
        //  can remove it from the other list.
        //
        tCIDLib::TCard4 c4At;
        TMArtCacheItem* pmaciCur = m_colByPIDKey.pobjKeyedBinarySearch
        (
            strKey, &eCompByPIDKey2, c4At
        );
        strDBKey = pmaciCur->m_strDBKey;
        m_colByPIDKey.RemoveAt(c4At);

        // And now remove from the hash set, which does own it
        m_colByDBKey.bRemoveKeyIfExists(strDBKey);
    }
}


