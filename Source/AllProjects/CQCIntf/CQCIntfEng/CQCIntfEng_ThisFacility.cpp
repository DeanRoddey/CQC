//
// FILE NAME: CQCIntfEng_ThisFacility.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/16/2002
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
//  This is the implementation file for the facility class.
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
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TFacCQCIntfEng,TFacility)


// ---------------------------------------------------------------------------
//  We need a key extraction function for our image item based cache items so
//  we can use them in a keyed hash collection.
// ---------------------------------------------------------------------------
static const TString& strExtractKey(const TIntfImgCachePtr& cptrSrc)
{
    return cptrSrc->strImageKey();
}



// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace
{
    namespace CQCIntfEng_ThisFacility
    {
        // -----------------------------------------------------------------------
        //  We fault in the widget type list, so we need a static flag to know
        //  if it's done yet.
        // -----------------------------------------------------------------------
        TAtomicFlag atomTypeListDone;


        // -----------------------------------------------------------------------
        //  Some constants for the image repo cache. The working size is the count of
        //  images in the cache at which point we start seeing if we can drop images
        //  from the cache when space for new ones are needed. This is only for the
        //  image repository cache, not for the web or art caches.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard4   c4WorkingSize   = 128;


        // -----------------------------------------------------------------------
        //  A cache of downloaded repo images. All image downloads by interfaces and done
        //  through us, which has huge advantages in a couple ways. The interface repo
        //  keeps up with changes to images and we don't have to re-download unless an
        //  image we have in the cache is modified in the repository.
        //
        //  We hand out counted pointers to our cache items, so that they will stay
        //  alive (and can know if they are being referenced) as long as a widget is
        //  still using them. So our cache is in terms of that counter pointer type. This
        //  also lets us use convenient by value semantics.
        //
        //  Note that, if the image is a color transparent one, what we store is the
        //  pre-masked image, which is what is required to draw the image masked, and we
        //  store the mask bitmap as well. This vastly speeds up drawing of images with
        //  a number of instances of the same image, which is very common.
        //
        //  Not many color transparency based images are used anymore and it would be
        //  nice at some to drop them, but for now we still support them.
        //
        //
        //  NOTE: To avoid possible case issues, we lower case the keys that are used
        //  in the cache. So they are the image path, with appropriate image/thumb
        //  prefix, lower cased.
        // -----------------------------------------------------------------------
        using TImgCache = TKeyedHashSet<TIntfImgCachePtr,TString,TStringKeyOps>;
        TImgCache& colImgCache()
        {
            static TImgCache colRet(109, TStringKeyOps(), &strExtractKey);
            return colRet;
        }
        constexpr tCIDLib::TEncodedTime enctImgCacheTime(15 * kCIDLib::enctOneSecond);
        constexpr tCIDLib::TCard4 c4MaxCachedImgs = 256;


        // -----------------------------------------------------------------------
        //  Another image cache, for web images this time.
        //
        //  The key is the URL of the image, in the fully encoded form, with the
        //  image (as apposed to thumb) prefix. The web widgets will only go out
        //  and get the image again if it's not in this cache or is in the cache
        //  but hasn't been downloaded in a certain amount of time.
        //
        //
        //  NOTE: To avoid possible case issues, we lower case the keys that are used
        //  in the cache. So they are the image path, with the image prefix, lower
        //  cased. No thumbs in this one.
        // -----------------------------------------------------------------------
        using TWebImgCache = TKeyedHashSet<TIntfImgCachePtr,TString,TStringKeyOps>;
        TWebImgCache& colWebImgCache()
        {
            static TWebImgCache colRet(109, TStringKeyOps(), &strExtractKey);
            return colRet;
        }
        constexpr tCIDLib::TCard4 c4MaxCachedWebImgs = 256;


        // -----------------------------------------------------------------------
        //  A template cache. We will keep templates for so many seconds before we
        //  check them again with the server. We do LRU on it and flush out a number
        //  of them if we get full, though that is pretty unlikely.
        //
        //  NOTE: To avoid possible case issues, we lower case the keys that are used
        //  in the cache. So they are the template path lower cased.
        // -----------------------------------------------------------------------
        using TTmplCache = TKeyedHashSet<TCIDGenCacheItem,TString,TStringKeyOps>;
        TTmplCache& colTmplCache()
        {
            static TTmplCache colRet(109, TStringKeyOps(), &TCIDGenCacheItem::strGenKey);
            return colRet;
        }
        constexpr tCIDLib::TCard4   c4TmplCacheSecs(240);
        constexpr tCIDLib::TCard4   c4MaxCachedTmpls = 64;


        // -----------------------------------------------------------------------
        //  A string that is displayed when a text widget goes into error state
        // -----------------------------------------------------------------------
        const TString   strErrText(L"????");
    }
}




// ---------------------------------------------------------------------------
//   CLASS: TFacCQCIntfEng
//  PREFIX: fac
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TFacCQCIntfEng: Public, static methods
// ---------------------------------------------------------------------------

// Set the designer mode flag. The getter is inlined for efficiency
tCIDLib::TBoolean TFacCQCIntfEng::bDesMode(const tCIDLib::TBoolean bToSet)
{
    s_bDesMode = bToSet;
    return s_bDesMode;
}


// Set the remote mode flag. The getter is inlined for efficiency
tCIDLib::TBoolean TFacCQCIntfEng::bRemoteMode(const tCIDLib::TBoolean bToSet)
{
    s_bRemoteMode = bToSet;
    return s_bRemoteMode;
}


// Set the WebRIVA specific remote mode flag.  The getter is inlined for efficiency
tCIDLib::TBoolean TFacCQCIntfEng::bWebRIVAMode(const tCIDLib::TBoolean bToSet)
{
    s_bWebRIVAMode = bToSet;
    return s_bWebRIVAMode;
}



//
//  These are element comparator functions that are used to sort collections
//  of widgets by their horziontal or vertical origins.
//
tCIDLib::TBoolean
TFacCQCIntfEng::bWidgetHPosComp(const   TCQCIntfWidget& iwdg1
                                , const TCQCIntfWidget& iwdg2)
{
    // If they have any commonality horizontally, then sort on the Y
    const TArea& area1 = iwdg1.areaActual();
    const TArea& area2 = iwdg2.areaActual();
    if (area1.bHorzOverlap(area2))
        return area1.i4Y() < area2.i4Y();
    return area1.i4X() < area2.i4X();
}

tCIDLib::TBoolean
TFacCQCIntfEng::bWidgetVPosComp(const   TCQCIntfWidget& iwdg1
                                , const TCQCIntfWidget& iwdg2)
{
    // If they have any commonality vertically, then sort on the X
    const TArea& area1 = iwdg1.areaActual();
    const TArea& area2 = iwdg2.areaActual();
    if (area1.bVertOverlap(area2))
        return area1.i4X() < area2.i4X();
    return area1.i4Y() < area2.i4Y();
}


tCIDLib::TVoid
TFacCQCIntfEng::MakeArtDBKey(const  TString&            strRepo
                            , const TString&            strCookie
                            , const tCIDLib::TBoolean   bLarge
                            ,       TString&            strKey)
{
    strKey = strRepo;
    strKey.Append(kCIDLib::chSpace);
    strKey.Append(strCookie);
    strKey.Append(kCIDLib::chSpace);
    strKey.Append(bLarge ? L"Lrg" : L"Sml");
}

tCIDLib::TVoid
TFacCQCIntfEng::MakeArtPIDKey(  const   TString&            strRepo
                                , const TString&            strPerId
                                , const tCIDLib::TBoolean   bLarge
                                ,       TString&            strKey)
{
    //
    //  This one gets passed to RIVA clients, so it needs the image type prefix
    //  on it.
    //
    strKey = kCQCKit::strWRIVA_ArtPref;
    strKey.Append(strRepo);
    strKey.Append(kCIDLib::chSpace);
    strKey.Append(strPerId);
    strKey.Append(kCIDLib::chSpace);
    strKey.Append(bLarge ? L"Lrg" : L"Sml");
}


const TString& TFacCQCIntfEng::strErrText()
{
    return CQCIntfEng_ThisFacility::strErrText;
}


// ---------------------------------------------------------------------------
//  TFacCQCIntfEng: Constructors and Destructor
// ---------------------------------------------------------------------------
TFacCQCIntfEng::TFacCQCIntfEng() :

    TFacility
    (
        L"CQCIntfEng"
        , tCIDLib::EModTypes::SharedLib
        , kCQCKit::c4MajVersion
        , kCQCKit::c4MinVersion
        , kCQCKit::c4Revision
        , tCIDLib::EModFlags::HasMsgsAndRes
    )
    , m_c4EmptyImgSz(0)
    , m_ppolleThis(nullptr)
    , m_strTitle(L"CQC Interface Engine")
    , m_widNext(1)
{
}

TFacCQCIntfEng::~TFacCQCIntfEng()
{
    // Flush the image cache
    try
    {
        CQCIntfEng_ThisFacility::colImgCache().RemoveAll();
    }

    catch(TError& errToCatch)
    {
        // Log but not much else we can do
        if (bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            LogEventObj(errToCatch);
        }
    }
}


// ---------------------------------------------------------------------------
//  TFacCQCIntfEng: Public, non-virtual methods
// ---------------------------------------------------------------------------


//
//  Checks the web image cache to see if there's a cached web image there
//  for the indicated URL. If we don't find it, we return false. If we do
//  find it, we give him back a deep copy of it. If the server provided an
//  expires stamp, we'll give him that back.
//
tCIDLib::TBoolean
TFacCQCIntfEng::bCachedWebImage(const   TString&            strURL
                                ,       TBitmap&            bmpToFill
                                ,       TString&            strServerStamp
                                ,       tCIDLib::TBoolean&  bExpired)
{
    // Build up the full path and lower case it
    TString strPath(kCQCIntfEng_::strImagePref);
    strPath.Append(strURL);
    strPath.ToLower();

    // Lock the queue while we do this
    TLocker lockrSync(&m_mtxSync);
    TIntfImgCachePtr cptrImg;
    if (CQCIntfEng_ThisFacility::colWebImgCache().bFindByKey(strPath, cptrImg))
    {
        //
        //  If no longer valid based on max age or expires, then we return
        //  true but we indicate it's expired.
        //
        if (!cptrImg->bStillGood(strServerStamp))
        {
            bExpired = kCIDLib::True;
            return kCIDLib::True;
        }

        // Update the last access
        cptrImg->SetLastAccess();

        //
        //  Get a deep copy of the image and a copy of the tag data,
        //  which will be the server last modified stamp if one was
        //  available.
        //
        bmpToFill = cptrImg->bmpImage();
        bmpToFill.DeepCopy();
        strServerStamp = cptrImg->strExpiresStamp();

        // Return true and indicate not expired
        bExpired = kCIDLib::False;
        return kCIDLib::True;
    }

    //
    //  Return not found and just in case set expired to true, though it's really
    //  meaningless in this case.
    //
    bExpired = kCIDLib::True;
    return kCIDLib::False;
}


//
//  See if the indicated image is in the cache. It's assumed the path is
//  the repository path.
//
tCIDLib::TBoolean
TFacCQCIntfEng::bFindImage( const   TString&            strPath
                            , const tCIDLib::TBoolean   bThumb
                            ,       TIntfImgCachePtr&   cptrToFill)
{
    // Build up the key
    TString strKey
    (
        bThumb ? kCQCIntfEng_::strThumbPref : kCQCIntfEng_::strImagePref
    );
    strKey.Append(strPath);
    strKey.ToLower();

    tCIDLib::TBoolean bRet;
    {
        // Lock the queue while we check
        TLocker lockrSync(&m_mtxSync);
        bRet = CQCIntfEng_ThisFacility::colImgCache().bFindByKey(strKey, cptrToFill);
    }
    return bRet;
}



//
//  These methods provide IV engine code with access to locally cached cover art.
//  It loads the local art and caches it. The first method is primarily for the RIVA
//  server, who will come back and look for art by way of the PID key that it is
//  given when an image is drawn.
//
//  The second one is for the IV itself, to find image by way of cookie. In their
//  case they want the bitmap. In the former case he wants the raw data because he
//  has to provide it to the RIVA clients.
//
tCIDLib::TBoolean
TFacCQCIntfEng::bFindLocalArt(  const   TString&            strPIDKey
                                ,       TMemBuf&            mbufToFill
                                ,       tCIDLib::TCard4&    c4Size)
{
    // Fault in the cache if not yet
    if (!TAtomic::pFencedGet(&s_pcolArtCache))
    {
        TBaseLock lockInit;
        if (!s_pcolArtCache)
            TAtomic::FencedSet(&s_pcolArtCache, new TMArtCache());
    }

    // Look for the art by the PID key. It handles synchronization
    TSize szOrg;
    return s_pcolArtCache->bByPIDKey(strPIDKey, mbufToFill, c4Size, szOrg);
}

tCIDLib::TBoolean
TFacCQCIntfEng::bLoadLocalArt(  const   TGraphDrawDev&      gdevCompat
                                , const TString&            strRepoMon
                                , const TString&            strCookie
                                , const TSize&              szTar
                                , const tCIDLib::TBoolean   bLarge
                                ,       TString&            strDBKey
                                ,       TBitmap&            bmpToSet
                                ,       TSize&              szOrg)
{
    // Fault in the cache if not yet
    if (!TAtomic::pFencedGet(&s_pcolArtCache))
    {
        TBaseLock lockInit;
        if (!TAtomic::pFencedGet(&s_pcolArtCache))
            TAtomic::FencedSet(&s_pcolArtCache, new TMArtCache());
    }

    //
    //  This will return already loaded or load it if needed (and it can. It handles
    //  synchronization.
    //
    return s_pcolArtCache->bByDBKey
    (
        gdevCompat
        , strRepoMon
        , strCookie
        , szTar
        , bLarge
        , bmpToSet
        , strDBKey
        , szOrg
    );
}


//
//  Given a starting container we will search for another container, following
//  the named nested containers by widget ids. Each component of the path is a
//  container widget id.
//
//  It has to be relative to the starting widget, so it must start with ./ or ../
//  So ./Info would be a child overlay of the starting container named Info. That
//  means that ../Preview would be a sibling of the starting widget. ../../Info
//  would be an overlay within the parent of the starting container named Info.
//
//  If the target is either the starting widget or one of its ancestors (i.e. if
//  the target were reloaded it would destroy the start widget), then the bIsAncestor
//  flag is true, else false.
//
//  If the path would go beyond the root template (of that layer in the interface
//  stack), then it will fail and return false. If any of the path components are
//  not found, it will also fail and return false.
//
//  Note that there is also the piwdgFindByNamePath() which will find a widget via
//  a path from the root. It's really only used internally. This one supports a
//  format that is used by some exposed action commands, which target relative
//  overlays.
//
tCIDLib::TBoolean
TFacCQCIntfEng::bFindTarContainer(          TCQCIntfContainer&      iwdgStart
                                    , const TString&                strPath
                                    ,       tCIDLib::TBoolean&      bIsAncestor
                                    ,       TCQCIntfContainer*&     piwdgTar)
{
    // Assume the worst
    piwdgTar = 0;
    bIsAncestor = kCIDLib::False;

    // It has to be a relative path
    if (!strPath.bStartsWith(L"./") && !strPath.bStartsWith(L"../"))
        return kCIDLib::False;

    // We start at the start container widget
    TCQCIntfContainer* piwdgCur = &iwdgStart;

    // And let's break it into path components
    tCIDLib::TStrList colComps;
    TStringTokenizer stokPath(&strPath, L"/");
    const tCIDLib::TCard4 c4CompCount = stokPath.c4BreakApart(colComps);

    //
    //  Now, for each component, we move up or down.
    //
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4CompCount; c4Index++)
    {
        const TString& strCurComp = colComps[c4Index];

        if (strCurComp == L".")
        {
            // Ignore it. It has no effect
        }
         else if (strCurComp == L"..")
        {
            // Move up to the parent container
            piwdgCur = piwdgCur->piwdgParent();
        }
         else
        {
            //
            //  Find a child of the current container that has the current name,
            //  no recursion.
            //
            TCQCIntfWidget* piwdgNext = piwdgCur->piwdgFindByName
            (
                strCurComp, kCIDLib::False
            );

            // If not found or not a container, then we failed
            if (!piwdgNext || !piwdgNext->bIsDescendantOf(TCQCIntfContainer::clsThis()))
                return kCIDLib::False;

            // Cast it to a container now that we know it is one
            piwdgCur = static_cast<TCQCIntfContainer*>(piwdgNext);
        }

        // If we have maxed out, then we failed
        if (!piwdgCur)
            return kCIDLib::False;
    }

    CIDAssert(piwdgCur != 0, L"The target container return value was not set");

    //
    //  See if the final target is an ancestor of the start widget. We indicate we
    //  want to recurse, not just check immediate children.
    //
    piwdgTar = piwdgCur;
    bIsAncestor = piwdgTar->bContainsWidget(&iwdgStart, kCIDLib::True);

    return kCIDLib::True;
}


//
//  Checks if the passed area is valid. There have been errors in the past that would
//  allow a widget's area to get set incorrectly and that would cause errors in OS
//  APIs because the coordinates were beyond those valid for the UI.
//
//  We check if the area is within the parent area (inclusive so it can touch the edges
//  of the parent.) If not, we force it within.
//
//  We have to though be careful of an error that would cause the offset plus size in
//  a direction to cause a coordinate wrap around. So we first check that the combination
//  will not overflow. If so, we see if the size is bigger than the parent and limit it to
//  that if so, else we adjust the offset just enough to avoid the wrap around.
//
tCIDLib::TBoolean
TFacCQCIntfEng::bIsValidWdgArea(const   TArea&  areaParent
                                , const TArea&  areaToCheck
                                ,       TArea&  areaFixed) const
{
    const tCIDLib::TInt8 i8Max(kCIDLib::i4MaxInt);
    tCIDLib::TInt8 i8XCheck
    (
        tCIDLib::TInt8(areaToCheck.i4X()) + tCIDLib::TInt8(areaToCheck.c4Width())
    );
    tCIDLib::TInt8 i8YCheck
    (
        tCIDLib::TInt8(areaToCheck.i4Y()) + tCIDLib::TInt8(areaToCheck.c4Height())
    );


    if ((i8XCheck > i8Max) || (i8YCheck > i8Max) || !areaParent.bContainsArea(areaToCheck))
    {
        // It ain't good so let's fix it
        areaFixed = areaToCheck;

        //
        //  Adjust any axis that would overflow. If the object size is larger than
        //  the parent, then obviously we need to size it down. Then adjust the origin
        //  if more is needed.
        //
        if (i8XCheck > i8Max)
        {
            if (areaFixed.c4Width() > areaParent.c4Width())
                areaFixed.c4Width(areaParent.c4Width());
            if (areaFixed.i4X() > areaParent.i4Right())
                areaFixed.i4X(areaParent.i4Right());
        }

        if (i8YCheck > i8Max)
        {
            if (areaFixed.c4Height() > areaParent.c4Height())
                areaFixed.c4Height(areaParent.c4Height());
            if (areaFixed.i4Y() > areaParent.i4Bottom())
                areaFixed.i4Y(areaParent.i4Bottom());
        }

        // Now force it fully within the template area
        areaFixed.ForceWithin(areaParent, kCIDLib::True);
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


//
//  A simple helper to read the value of a field as a string, for use by
//  the interface viewer engine.
//
tCIDLib::TBoolean
TFacCQCIntfEng::bReadField(         TCQCIntfView&   civCaller
                            , const TString&        strMoniker
                            , const TString&        strField
                            ,       TString&        strValue)
{
    // Pass it on to the engine
    try
    {
        if (!m_ppolleThis->bReadValue(strMoniker, strField, strValue))
            return kCIDLib::False;
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        civCaller.ShowErr
        (
            civCaller.strTitle()
            , facCQCIntfEng().strMsg
              (
                kIEngMsgs::midStatus_FldReadFailed
                , strMoniker + TString(L".") + strField
              )
            , errToCatch
        );
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


//
//  This is called by the interface designer after the user invokes the
//  image repository editing dialog, to deal with any potential changes
//  that occured. It returns true if any changes were seen, which means
//  the designer should ask all it's open templates to refresh.
//
//  This is only used at design time.
//
tCIDLib::TBoolean
TFacCQCIntfEng::bRefreshImgCache(TGraphDrawDev& gdevCompat, const TCQCSecToken& sectUser)
{
    // We need a data server client for this
    TDataSrvClient dsclLoad;

    // Lock the image cache while we do this
    TLocker lockrSync(&m_mtxSync);

    //
    //  Loop through all of the images in the cache, and check whether they
    //  have been modified. If so, then get the new info. If none, just return
    //  now.
    //
    CQCIntfEng_ThisFacility::TImgCache::TNCCursor cursImgs
    (
        &CQCIntfEng_ThisFacility::colImgCache()
    );

    // If empty, bail out now and don't create the big buffer
    if (!cursImgs.bReset())
        return kCIDLib::False;;

    //
    //  Create a buffer to read into. Make it one chunk size since that's
    //  all we'll get in the first round if there is more needed. And create
    //  a buffer for reading successive chunks.
    //
    THeapBuf mbufData(64 * 1024, 16 * 0x100000, 0x80000);
    tCIDLib::TBoolean bRet = kCIDLib::False;
    for (; cursImgs; ++cursImgs)
    {
        TIntfImgCachePtr& cptrCur = cursImgs.objWCur();

        try
        {
            tCIDLib::TCard4 c4Bytes;
            tCIDLib::TCard4 c4SerialNum = cptrCur->c4SerialNum();
            tCIDLib::TEncodedTime enctLastChange;
            tCIDLib::TBoolean bNewData;
            if (cptrCur->bThumb())
            {
                bNewData = dsclLoad.bReadThumb
                (
                    cptrCur->strImageName()
                    , c4SerialNum
                    , enctLastChange
                    , mbufData
                    , c4Bytes
                    , sectUser
                );
            }
             else
            {
                tCIDLib::TKVPFList colMeta;
                bNewData = dsclLoad.bReadImage
                (
                    cptrCur->strImageName()
                    , c4SerialNum
                    , enctLastChange
                    , mbufData
                    , c4Bytes
                    , colMeta
                    , sectUser
                );
            }

            //
            //  If we got new data, then stream it in. If didn't get new data,
            //  it had to have been in the cache and our cache was up to date,
            //  but we need to update the time stamp on the referenced image.
            //
            if (bNewData)
            {
                // Remember we saw changes
                bRet = kCIDLib::True;

                //
                //  And update the cache item with the new info. This will
                //  also update the last checked and last accessed time
                //  stamps.
                //
                cptrCur->Set
                (
                    cptrCur->bThumb()
                    , c4Bytes
                    , tCIDLib::ForceMove(mbufData)
                    , c4SerialNum
                    , gdevCompat
                );
            }

            // Update both the last access and last checked stamps
            cptrCur->SetLastCheck();
            cptrCur->SetLastAccess();
        }

        catch(TError& errToCatch)
        {
            if (!errToCatch.bLogged() && bLogWarnings())
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                LogEventObj(errToCatch);
            }

            //
            //  Delete this one from the cache. Deleting via the cursor
            //  will keep the cursor valid.
            //
            CQCIntfEng_ThisFacility::colImgCache().RemoveAt(cursImgs);

            // And it's a change
            bRet = kCIDLib::True;
        }
    }

    return bRet;
}


// Provide access to our two special empty images
const TIntfImgCachePtr& TFacCQCIntfEng::cptrEmptyImg() const
{
    return m_cptrEmptyImg;
}

const TIntfImgCachePtr& TFacCQCIntfEng::cptrEmptyThumbImg() const
{
    return m_cptrEmptyThumbImg;
}


//
//  These get an image by way of the image cache. If the image isn't in the
//  cache, or is out of date, then it's downloaded and put into the cache.
//  There are versions for getting color based transparency images with masks
//  or not. Mostly it'll be not, since not very many interfaces don't use
//  alpha based images.
//
//  If it's not found, a pointer to a default empty image is returned, so
//  that they have something to use.
//
TIntfImgCachePtr
TFacCQCIntfEng::cptrGetImage(const  TString&                strName
                            , const TCQCIntfView&           civOwner
                            ,       TDataSrvClient&         dsclToUse)
{
    //
    //  Build up the cache key. In this case use the image prefix, not the
    //  thumb prefix.
    //
    TString strKey(kCQCIntfEng_::strImagePref);
    strKey.Append(strName);
    strKey.ToLower();

    //
    //  See if it's in our cache. If so, then use that as the serial number
    //  for the download attempt. Do this in a faux block so that we unlock
    //  the cache again during the read of the data.
    //
    tCIDLib::TBoolean   bExists;
    TIntfImgCachePtr    cptrRet;
    tCIDLib::TCard4     c4SerialNum(0);
    {
        TLocker lockrSync(&m_mtxSync);
        bExists = CQCIntfEng_ThisFacility::colImgCache().bFindByKey(strKey, cptrRet);
        if (bExists)
            c4SerialNum = cptrRet->c4SerialNum();
    }

    //
    //  If we already have it and it's been less than the cache time since we last
    //  checked this one, then don't bother to check it again.
    //
    if (bExists
    &&  ((TTime::enctNow() - cptrRet->enctLastCheck()) < CQCIntfEng_ThisFacility::enctImgCacheTime))
    {
        return cptrRet;
    }

    // If it wasn't in the cache, add a new item for it, we'll set it below
    if (!bExists)
        cptrRet = cptrNewCacheItem(strName, 0, kCIDLib::False);

    try
    {
        //
        //  Create a buffer to read into and a chunk buffer to read the individual
        //  chunks.
        //
        tCIDLib::TCard4         c4Bytes;
        tCIDLib::TKVPFList      colMeta;
        tCIDLib::TEncodedTime   enctLastChange;
        THeapBuf                mbufData(64 * 1024, 16 * 0x100000, 0x80000);
        const tCIDLib::TBoolean bNewData = dsclToUse.bReadImage
        (
            strName
            , c4SerialNum
            , enctLastChange
            , mbufData
            , c4Bytes
            , colMeta
            , civOwner.cuctxToUse().sectUser()
        );

        //
        //  If we got new data, then stream it in. If didn't get new data, it
        //  had to have been in the cache and our cache was up to date, but
        //  we need to update the time stamp on the referenced image.
        //
        //  We need to lock again now that we are about to access the cache
        //
        TLocker lockrSync(&m_mtxSync);
        if (bNewData)
        {
            //
            //  And update the cache item with the new info. This will also
            //  update the last checked and last accessed time stamps.
            //
            cptrRet->Set
            (
                kCIDLib::False
                , c4Bytes
                , tCIDLib::ForceMove(mbufData)
                , c4SerialNum
                , civOwner.gdevCompat()
            );
        }
         else
        {
            // Bump the last check since this was still a check
            cptrRet->SetLastCheck();
        }
    }

    catch(TError& errToCatch)
    {
        if (bLogWarnings())
        {
            if (!errToCatch.bLogged())
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                LogEventObj(errToCatch);
            }

            LogMsg
            (
                CID_FILE
                , CID_LINE
                , kIEngErrs::errcImg_LoadFailedMS
                , tCIDLib::ESeverities::Warn
                , tCIDLib::EErrClasses::CantDo
                , strName
            );
        }

        //
        //  If it didn't already exist, then set the default blank image data on it for now,
        //  with a zero serial number. It will get checked again later and maybe we'll get
        //  the actual data then. Since we use a zero serial number we should get the real
        //  stuff no matter what serial number it's on.
        //
        //  If it did exist, then we keep what we have.
        //
        if (!bExists)
        {
            cptrRet->Set
            (
                kCIDLib::True, m_c4EmptyImgSz, m_mbufEmptyImg, 0, civOwner.gdevCompat()
            );
        }
    }
    return cptrRet;
}


//
//  A helper method for derived widgets to call to do the grunt work of loading
//  thumbs, and keeping the image cache updated. If the image is in the cache,
//  that is returned, else we load it and add it to the cache.
//
//  If it's not found, a pointer to a default empty image is returned, so
//  that they have something to use.
//
TIntfImgCachePtr
TFacCQCIntfEng::cptrGetThumb(const  TString&        strName
                            , const TCQCIntfView&   civOwner
                            ,       TDataSrvClient& dsclToUse)
{
    // Build up the key
    TString strKey(kCQCIntfEng_::strThumbPref);
    strKey.Append(strName);
    strKey.ToLower();

    //
    //  See if it's in our cache. If so, then use that as the serial number
    //  for the download attempt.
    //
    tCIDLib::TBoolean   bExists;
    TIntfImgCachePtr    cptrRet;
    tCIDLib::TCard4     c4SerialNum(0);
    {
        TLocker lockrSync(&m_mtxSync);
        bExists = CQCIntfEng_ThisFacility::colImgCache().bFindByKey(strKey, cptrRet);
        if (bExists)
            c4SerialNum = cptrRet->c4SerialNum();
    }

    //
    //  If we already have it and it's been less than 10 seconds since we last
    //  checked this one, then don't bother to check it again.
    //
    if (bExists
    &&  ((TTime::enctNow() - cptrRet->enctLastCheck()) < CQCIntfEng_ThisFacility::enctImgCacheTime))
    {
        return cptrRet;
    }

    //  If it wasn't in the cache, add a new item for it, we'll fill it in below
    if (!bExists)
        cptrRet = cptrNewCacheItem(strName, 0, kCIDLib::True);


    // We don't have it or it's been long enough to need to check again
    try
    {
        THeapBuf mbufData(kCQCRemBrws::c4DataBlockSz);
        tCIDLib::TCard4 c4Bytes;
        tCIDLib::TEncodedTime enctLastChange;
        const tCIDLib::TBoolean bNewData = dsclToUse.bReadThumb
        (
            strName
            , c4SerialNum
            , enctLastChange
            , mbufData
            , c4Bytes
            , civOwner.cuctxToUse().sectUser()
        );

        // It could be something already in the cache, so lock first then update
        TLocker lockrSync(&m_mtxSync);
        if (bNewData)
        {
            cptrRet->Set
            (
                kCIDLib::True
                , c4Bytes
                , tCIDLib::ForceMove(mbufData)
                , c4SerialNum
                , civOwner.gdevCompat()
            );
        }
    }

    catch(TError& errToCatch)
    {
        if (bLogWarnings())
        {
            if (bShouldLog(errToCatch))
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                LogEventObj(errToCatch);
            }

            LogMsg
            (
                CID_FILE
                , CID_LINE
                , kIEngErrs::errcImg_LoadFailedMS
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::CantDo
                , strName
            );
        }

        //
        //  If it didn't already exist, then set the default blank image data on it for now,
        //  with a zero serial number. It will get checked again later and maybe we'll get
        //  the actual data then. Since we use a zero serial number we should get the real
        //  stuff no matter what serial number it's on.
        //
        //  If it did exist, then we keep what we have.
        //
        if (!bExists)
        {
            cptrRet->Set
            (
                kCIDLib::True, m_c4EmptyImgSz, m_mbufEmptyImg, 0, civOwner.gdevCompat()
            );
        }
    }
    return cptrRet;
}


//
//  Return a cursor for our widget types list, faulting in the list if
//  not already done.
//
TFacCQCIntfEng::TTypeList::TCursor TFacCQCIntfEng::cursTypes() const
{
    FaultInTypeList();
    return TTypeList::TCursor(&m_colTypes);
}


//
//  Store/update an image in the web image cache. If the image is already
//  there it will update that existing image. Else it will put a new one
//  into the cache.
//
//  If the cache is full, we find the least recently used one and throw
//  it out of the cache.
//
//  We just bump the serial number each time a new one is stored.
//
//  The caller gets a deep copy of the bitmap, because he may modify it
//  and we don't want him to modify the cached version.
//
tCIDLib::TVoid
TFacCQCIntfEng::CacheWebImage(  const   TString&            strURL
                                , const tCIDLib::TCard4     c4Data
                                ,       THeapBuf&&          mbufData
                                , const TGraphDrawDev&      gdevCompat
                                ,       tCIDLib::TCard4&    c4NewSerialNum)
{

    // Build the cache style path (key), lower case it, and see if it exists
    TString strPath(kCQCIntfEng_::strImagePref);
    strPath.Append(strURL);
    strPath.ToLower();

    // Lock while we do this
    TLocker lockrSync(&m_mtxSync);

    TIntfImgCachePtr cptrRet;
    if (!CQCIntfEng_ThisFacility::colWebImgCache().bFindByKey(strPath, cptrRet))
    {
        //
        //  We have to add a new one. So first see if the cache is full and we
        //  want to throw out some older ones. We throw out 1/8th of the entries at
        //  a time, so as to avoid undue work doing it one at a time.
        //
        if (CQCIntfEng_ThisFacility::colWebImgCache().bIsFull(CQCIntfEng_ThisFacility::c4MaxCachedWebImgs))
        {
            CQCIntfEng_ThisFacility::TWebImgCache::TNCCursor cursImgs
            (
                &CQCIntfEng_ThisFacility::colWebImgCache()
            );

            //
            //  We iterate the list and get pointers to all of them. Then we sort
            //  them by time.
            //
            TRefVector<TIntfImgCachePtr> colSort(tCIDLib::EAdoptOpts::NoAdopt);

            cursImgs.bReset();
            do
            {
                colSort.Add(&cursImgs.objWCur());
            }   while (cursImgs.bNext());

            // Sort them so that the oldest access are earliest in the list
            colSort.Sort
            (
                [](const TIntfImgCachePtr& cptr1, const TIntfImgCachePtr& cptr2)
                {
                    const tCIDLib::TEncodedTime enct1 = cptr1->enctLastAccess();
                    const tCIDLib::TEncodedTime enct2 = cptr2->enctLastAccess();
                    if (enct1 < enct2)
                        return tCIDLib::ESortComps::FirstLess;
                    else if (enct1 > enct2)
                        return tCIDLib::ESortComps::FirstGreater;
                    return tCIDLib::ESortComps::Equal;
                }
            );

            //
            //  Now go through up to 1/4th of the cache items, and for each one we
            //  remove it from the cache collection. Since our sorted list is just
            //  pointers, we don't have to worry about invalidating them. But we need
            //  to copy the key string out and use that to remove it, so that we don't
            //  pass it the string inside the object it's about to destroy.
            //
            const tCIDLib::TCard4 c4Count(CQCIntfEng_ThisFacility::c4MaxCachedWebImgs / 4);
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            {
                strPath = (*colSort[c4Index])->strImageKey();
                CQCIntfEng_ThisFacility::colWebImgCache().bRemoveKey
                (
                    strPath, kCIDLib::False
                );
            }
        }

        //
        //  And add a new one, and assign the newly added one to our return pointer
        //  so that by the time we get below, one way or another the return cache
        //  item pointer is set up. Since it's a counted pointer, this assigment will
        //  leave the return pointing to the same cache item that we just added.
        //
        cptrRet = CQCIntfEng_ThisFacility::colWebImgCache().objAdd
        (
            TIntfImgCachePtr
            (
                new TIntfImgCacheItem(strURL, kCIDLib::False, tCQCIntfEng::EImgTypes::Web)
            )
        );
    }

    // Set or update the one we found or added, bumping the serial number
    c4NewSerialNum = cptrRet->c4SerialNum() + 1;
    cptrRet->Set
    (
        kCIDLib::False
        , c4Data
        , tCIDLib::ForceMove(mbufData)
        , c4NewSerialNum
        , gdevCompat
    );

    // And set this as our last check time
    cptrRet->SetLastCheck();
}


//
//  A helper method that can be used for drawing images from the image cache. Some
//  things don't call this because they want to pre-scale images for efficiency.
//  These do the placement calculation and do a (potentially) stretching blit of
//  the image to the calculated target, so they are a bit less efficient.
//
tCIDLib::TVoid
TFacCQCIntfEng::DrawImage(          TGraphDrawDev&      gdevTar
                            , const tCIDLib::TBoolean   bForceAlpha
                            , const TIntfImgCachePtr&   cptrToDraw
                            , const TArea&              areaDrawIn
                            , const TArea&              areaInvalid
                            , const tCIDGraphDev::EPlacement ePlacement
                            , const tCIDLib::TCard1     c1ConstOpacity
                            , const tCIDLib::TBoolean   bPressed)
{
    // Set up the source area and call the other version
    TArea areaSrc(TPoint::pntOrigin, cptrToDraw->szImage());
    DrawImage
    (
        gdevTar
        , bForceAlpha
        , cptrToDraw
        , areaDrawIn
        , areaInvalid
        , areaSrc
        , ePlacement
        , 0
        , 0
        , c1ConstOpacity
        , bPressed
    );
}

tCIDLib::TVoid
TFacCQCIntfEng::DrawImage(          TGraphDrawDev&      gdevTar
                            , const tCIDLib::TBoolean   bForceAlpha
                            , const TIntfImgCachePtr&   cptrToDraw
                            , const TArea&              areaDrawIn
                            , const TArea&              areaInvalid
                            , const tCIDLib::TInt4      i4HOffset
                            , const tCIDLib::TInt4      i4VOffset
                            , const tCIDLib::TCard1     c1ConstOpacity
                            , const tCIDLib::TBoolean   bPressed)
{
    // Set up the source area and call the other version
    TArea areaSrc(TPoint::pntOrigin, cptrToDraw->szImage());
    DrawImage
    (
        gdevTar
        , bForceAlpha
        , cptrToDraw
        , areaDrawIn
        , areaInvalid
        , areaSrc
        , tCIDGraphDev::EPlacement::Center
        , i4HOffset
        , i4VOffset
        , c1ConstOpacity
        , bPressed
    );
}

tCIDLib::TVoid
TFacCQCIntfEng::DrawImage(          TGraphDrawDev&      gdevTar
                            , const tCIDLib::TBoolean   bForceAlpha
                            , const TIntfImgCachePtr&   cptrToDraw
                            , const TArea&              areaDrawIn
                            , const TArea&              areaInvalid
                            , const tCIDGraphDev::EPlacement ePlacement
                            , const tCIDLib::TInt4      i4HOffset
                            , const tCIDLib::TInt4      i4VOffset
                            , const tCIDLib::TCard1     c1ConstOpacity
                            , const tCIDLib::TBoolean   bPressed)
{
    // Set up the source area and call the other version
    TArea areaSrc(TPoint::pntOrigin, cptrToDraw->szImage());
    DrawImage
    (
        gdevTar
        , bForceAlpha
        , cptrToDraw
        , areaDrawIn
        , areaInvalid
        , areaSrc
        , ePlacement
        , i4HOffset
        , i4VOffset
        , c1ConstOpacity
        , bPressed
    );
}

tCIDLib::TVoid
TFacCQCIntfEng::DrawImage(          TGraphDrawDev&      gdevTar
                            , const tCIDLib::TBoolean   bForceAlpha
                            , const TIntfImgCachePtr&   cptrToDraw
                            , const TArea&              areaDrawIn
                            , const TArea&              areaInvalid
                            ,       TArea&              areaSrc
                            , const tCIDGraphDev::EPlacement ePlacement
                            , const tCIDLib::TInt4      i4HOffset
                            , const tCIDLib::TInt4      i4VOffset
                            , const tCIDLib::TCard1     c1ConstOpacity
                            , const tCIDLib::TBoolean   bPressed)
{
    //
    //  Calcualte where in the drawing area the image will be placed and
    //  the sort of blit type required to fit the source into it.
    //
    TArea areaTarget;
    const tCIDGraphDev::EBltTypes eBlt = facCIDGraphDev().eCalcPlacement
    (
        areaDrawIn, areaSrc, areaTarget, ePlacement, bPressed
    );

    // If there are offsets, then offset the target area
    areaTarget.AdjustOrg(i4HOffset, i4VOffset);

    // If the target area intersects the invalid area, draw it
    if (areaTarget.bIntersects(areaInvalid))
    {
        cptrToDraw->bmpImage().AdvDrawOn
        (
            gdevTar
            , areaSrc
            , areaTarget
            , cptrToDraw->bTransparent()
            , cptrToDraw->c4TransClr()
            , cptrToDraw->bmpMask()
            , eBlt
            , c1ConstOpacity
            , bForceAlpha
        );
    }
}


//
//  When a widget wants to drop a reference to an image cache item, it can't just drop
//  the reference, since that would leave it with a null pointer, and he'd have to
//  special case every call to check for it and forth. So, instead he calls this and
//  we just point him at a blank image.
//
tCIDLib::TVoid TFacCQCIntfEng::DropImgRef(TIntfImgCachePtr& cptrToReset)
{
    // It may already not exist
    if (cptrToReset.pobjData())
    {
        if (cptrToReset->bThumb())
            cptrToReset = m_cptrEmptyThumbImg;
        else
            cptrToReset = m_cptrEmptyImg;
    }
}


//
//  This is for the RIVA server, to allow him to first check whether
//  we have an image in our cache that he needs before he bothers going to the
//  data server for it.
//
//  Don't call this otherwise, since the buffer data is only cached when
//  in RIVA mode.
//
tCQCIntfEng::EImgQRes
TFacCQCIntfEng::eGetImageData(  const   TString&                strPath
                                , const tCIDLib::TBoolean       bThumb
                                , const tCQCIntfEng::EImgTypes  eType
                                ,       tCIDLib::TCard4&        c4SerialNum
                                ,       TMemBuf&                mbufToFill
                                ,       tCIDLib::TCard4&        c4Size)
{
    // Build up the key
    TString strKey
    (
        bThumb ? kCQCIntfEng_::strThumbPref : kCQCIntfEng_::strImagePref
    );
    strKey.Append(strPath);
    strKey.ToLower();

    // Lock and see if we can find this guy
    TIntfImgCachePtr cptrFind;
    TLocker lockrSync(&m_mtxSync);

    // If not in the cache, then obviously nothing more to do
    if (eType == tCQCIntfEng::EImgTypes::Repo)
    {
        if (!CQCIntfEng_ThisFacility::colImgCache().bFindByKey(strKey, cptrFind))
            return tCQCIntfEng::EImgQRes::NotFound;
    }
     else if (eType == tCQCIntfEng::EImgTypes::Web)
    {
        if (!CQCIntfEng_ThisFacility::colWebImgCache().bFindByKey(strKey, cptrFind))
            return tCQCIntfEng::EImgQRes::NotFound;
    }
     else
    {
        return tCQCIntfEng::EImgQRes::NotFound;
    }

    // If our serial number is the same, say it's unchanged
    if (c4SerialNum == cptrFind->c4SerialNum())
        return tCQCIntfEng::EImgQRes::Unchanged;

    c4SerialNum = cptrFind->c4SerialNum();
    c4Size = cptrFind->c4Size();
    if (mbufToFill.c4Size() < c4Size)
        mbufToFill.Reallocate(c4Size);
    mbufToFill.CopyIn(cptrFind->mbufIVImgData(), c4Size);
    return tCQCIntfEng::EImgQRes::NewData;
}


//
//  The editor calls this to get us to fill in a sub-menu for him, which will
//  include all of the available widget types, seperated into sub-menus, and set
//  up such that we can correlate the menu ids back to widget classes later when
//  the user wants to create a new widget.
//
tCIDLib::TVoid
TFacCQCIntfEng::FillNewMenu(        TSubMenu&           menuTarget
                            , const tCIDLib::TCard4     c4FirstId)
{
    // Fault in the list if not already
    FaultInTypeList();

    tCIDLib::TBoolean   bInSub = kCIDLib::False;
    tCIDLib::TCard4     c4Id = c4FirstId;
    TSubMenu            menuSub;

    const tCIDLib::TCard4 c4Count = m_colTypes.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TKeyValuePair& kvalCur = m_colTypes[c4Index];

        //
        //  If the key is empty and the name is not, then it's the start of
        //  a sub-menu, so create that.
        //
        //  If the key and name are empty, then it's the end of the sub-menu,
        //  so null out the sub-menu handle because we are now adding to the
        //  main menu again.
        //
        //  Else, it's an item to go into the current sub-menu, or the
        //  top level menu if a sub-menu is not open.
        //
        const tCIDLib::TBoolean bNoKey = kvalCur.strKey().bIsEmpty();
        const tCIDLib::TBoolean bNoVal = kvalCur.strValue().bIsEmpty();
        if (bNoKey && bNoVal)
        {
            bInSub = kCIDLib::False;
        }
         else if (bNoKey)
        {
            // Create a new sub-menu and get the handle
            menuTarget.AddSubMenu(TString::strEmpty(), kvalCur.strValue(), c4Id);
            menuSub.Set(menuTarget, c4Id);
            c4Id++;
            bInSub = kCIDLib::True;
        }
         else
        {
            // Add the current item to either the sub or main menu
            if (bInSub)
            {
                menuSub.AddActionItem
                (
                    kCIDLib::c4MaxCard, TString::strEmpty(), kvalCur.strValue(), c4Id++
                );
            }
             else
            {
                menuTarget.AddActionItem
                (
                    kCIDLib::c4MaxCard, TString::strEmpty(), kvalCur.strValue(), c4Id++
                );
            }
        }
    }
}


//
//  Given a path in the form name/name/etc... we start at the passed starting
//  container and look for each name as we drill down into nested containers.
//  So every name except the last one must be a container widget.
//
//  This is only meaningful at viewing time, where the hierarchy is in place.
//
//  If not found, we return a null.
//
TCQCIntfWidget*
TFacCQCIntfEng::piwdgFindByNamePath(const   TString&            strPath
                                    ,       TCQCIntfContainer&  iwdgStart)
{
    TStringTokenizer stokPath(&strPath, L"/");
    TString strToken;
    TCQCIntfContainer* piwdgCur = &iwdgStart;
    while (stokPath.bGetNextToken(strToken))
    {
        // Find a child with this name, no recursion
        TCQCIntfWidget* piwdgChild = piwdgCur->piwdgFindByName(strToken, kCIDLib::False);
        if (!piwdgChild)
            return 0;

        // If not more tokens, the we are done
        if (!stokPath.bMoreTokens())
            return piwdgChild;

        // It has to be a container derivative at this point
        if (!piwdgChild->bIsDescendantOf(TCQCIntfContainer::clsThis()))
            return 0;

        // Cast it to a container and make it the new current one
        piwdgCur = static_cast<TCQCIntfContainer*>(piwdgChild);
    }

    // Never found it
    return 0;
}


//
//  Dynamically creates a widget of the indicated type. The type is the user
//  visible name of the type, not our internal class type.
//
TCQCIntfWidget*
TFacCQCIntfEng::piwdgMakeInstanceOf(const TString& strUserTypeName) const
{
    // Fault in the list if not already
    FaultInTypeList();

    // If the entry in the types list that has this user type name
    const tCIDLib::TCard4 c4Count = m_colTypes.c4ElemCount();
    tCIDLib::TCard4 c4Index = 0;
    for (; c4Index < c4Count; c4Index++)
    {
        if (m_colTypes[c4Index].strValue() == strUserTypeName)
            break;
    }

    TCQCIntfWidget* piwdgRet = 0;
    if (c4Index != c4Count)
        piwdgRet = ::pobjMakeNewOfClass<TCQCIntfWidget>(m_colTypes[c4Index].strKey());

    return piwdgRet;
}


// Provide access to the common polling engine
TCQCPollEngine& TFacCQCIntfEng::polleThis()
{
    CIDAssert(m_ppolleThis != nullptr, L"The polling engine has not been set yet");
    return *m_ppolleThis;
}


//
//  A thin helper/wrapper around the interface server client proxy, for querying a
//  template. They could easily enough do it themselves, but this guy does a little
//  grunt work for them, and it enables us to provide caching, which speeds things up
//  a lot.
//
//  We allocate a buffer, query the template, stream it in to the caller's widget
//  object, store the path of the template into the template object, and throw if
//  the server indicates insufficient rights.
//
//  We have a second one that will create the client proxy as well, for those clients
//  who don't already have one.
//
//  The caller indicates if he wants to make use of caching. For testing purposes,
//  clients typically want to disable it. And, in a RIVA situation, each session (view)
//  needs to be able to enable/disable caching separately.
//
//  Even if they don't want to use caching, we still cache, again for the RIVA
//  scenario since if we download new data, other (caching) sessions will still
//  benefit from it. Basically, if caching is enabled for the caller, we act like
//  the cache item is out of data, if we find one. So we'll go back to the server to
//  check. If we get new data, then we update the cache. Either way, the caller will
//  get new data if there is new data available,b ut still pay a fairly low price
//  if the template is cached and not out of date.
//
//
//  NOTE THAT, if any RIVA session has disabled caching, then any other sessions
//  will be blocked while the server is checked, so it does still affect the
//  performance of other sessions to that extent. We could make this a lot more
//  complex and avoid that, but it hasn't been done yet.
//
tCIDLib::TVoid
TFacCQCIntfEng::QueryTemplate(  const   TString&                strName
                                ,       TDataSrvClient&         dsclLoad
                                ,       TCQCIntfTemplate&       iwdgToFill
                                , const TCQCUserCtx&            cuctxToUse
                                , const tCIDLib::TBoolean       bCacheEnabled)
{
    TLocker lockrSync(&m_mtxSync);

    // For brevity get a ref to the cache
    tCIDLib::TGenCache& colCache = CQCIntfEng_ThisFacility::colTmplCache();

    //
    //  Check the cache, even if caching isn't enabled. This way, we can cache it
    //  below if it's a new one, even if the caller doesn't make use of caching.
    //
    TString strKey(strName);
    strKey.ToLower();
    TCIDGenCacheItem* pcciFind = colCache.pobjFindByKey(strKey);

    //
    //  If the caller wants caching and we have it and it's current enough, then we
    //  can give it back. If caching is disabled, even if we find it, we just go to
    //  the other block, which will essentially be like it was out of date.
    //
    if (bCacheEnabled && pcciFind && (pcciFind->enctTimeStamp() > TTime::enctNow()))
    {
        TBinMBufInStream strmIn(&pcciFind->mbufData(), pcciFind->c4Bytes());
        strmIn >> iwdgToFill;
    }
     else
    {
        //
        //  Not in the cache or we need to recheck it. If it was in the cache and caching
        //  is enabled, then pass in the serial number we have, since most likely we won't
        //  need to download new data.
        //
        tCIDLib::TCard4 c4SerialNum = 0;
        if (pcciFind && bCacheEnabled)
            c4SerialNum = pcciFind->c4SerialNum();

        tCIDLib::TCard4         c4Bytes;
        tCIDLib::TEncodedTime   enctLast;
        THeapBuf                mbufData;
        tCIDLib::TKVPFList      colMeta;
        const tCIDLib::TBoolean bNewData = dsclLoad.bReadTemplate
        (
             strName, c4SerialNum, enctLast, mbufData, c4Bytes, colMeta, cuctxToUse.sectUser()
        );

        //
        //  If we got new data, then let's stream in from the new data and update
        //  the cache with it. Else, let's just stream in from the cache item. We
        //  have to have a cache item if we didn't get new data, since otherwise
        //  we would have passed a zero serial number.
        //
        //  If not caching we'll always get new data, since we'll always pass in
        //  a zero serial number, so if not new data, the cache item must have been
        //  found above and must therefore be set.
        //
        if (bNewData)
        {
            TBinMBufInStream strmIn(&mbufData, c4Bytes);
            strmIn >> iwdgToFill;

            // If it wasn't in the cache, then add a cache item
            if (!pcciFind)
            {
                //
                //  Add it to the cache. If it's full, throw out some LRU items.
                //  It will be based on the time stamp which is really a next check
                //  time, but effectively that's still a measure of how recently
                //  it's been accessed, close enough.
                //
                if (colCache.bIsFull(CQCIntfEng_ThisFacility::c4MaxCachedTmpls))
                    TCIDGenCacheItem::c4LRUDiscard(colCache, 25);

                //
                //  Point the find object pointer at it so that either way we end up
                //  with pcciFind pointing at the cache item to update.
                //
                pcciFind = &colCache.objPlace(strKey, 0);
            }

            // Store the new data on the cache item
            pcciFind->SetData(c4Bytes, tCIDLib::ForceMove(mbufData));

            // And store the new serial number we got
            pcciFind->c4SerialNum(c4SerialNum);
        }
         else
        {
            CIDAssert(pcciFind != nullptr, L"Template cache pointer was null");

            // We already have the most recent stuff, so use the cache
            TBinMBufInStream strmIn(&pcciFind->mbufData(), pcciFind->c4Bytes());
            strmIn >> iwdgToFill;
        }

        //
        //  Whether new or updated, we have checked the server, so update this guy
        //  for the next time we'll check, if we found it in the cache.
        //
        if (pcciFind)
            pcciFind->UpdateTimeStamp(CQCIntfEng_ThisFacility::c4TmplCacheSecs);
    }

    //
    //  Store the template name into the template. This is important because it's
    //  used in error messages, in save operations when in the editor, to expand
    //  relative paths invoked by child widgets, and so forth.
    //
    iwdgToFill.strTemplateName(strName);
}

tCIDLib::TVoid
TFacCQCIntfEng::QueryTemplate(  const   TString&            strName
                                ,       TCQCIntfTemplate&   iwdgToFill
                                , const TCQCUserCtx&        cuctxToUse
                                , const tCIDLib::TBoolean   bCacheEnabled
                                , const tCIDLib::TCard4     c4WaitFor)
{
    // If not caching, just immediately call the other one
    if (!bCacheEnabled)
    {
        TDataSrvClient dsclLoad;
        QueryTemplate(strName, dsclLoad, iwdgToFill, cuctxToUse, bCacheEnabled);
        return;
    }

    // Let's check the cache then before we bother calling the other one
    TLocker lockrSync(&m_mtxSync);

    //
    //  Though it will be redundant if we end up calling the other version, check
    //  the cache first. If we can get it from there, we never have to create the
    //  client proxy. So it's worth it.
    //
    TString strKey(strName);
    strKey.ToLower();
    TCIDGenCacheItem* pcciFind
    (
        CQCIntfEng_ThisFacility::colTmplCache().pobjFindByKey(strKey)
    );

    // If cached and it's recent enough, take it
    if (bCacheEnabled && pcciFind && (pcciFind->enctTimeStamp() > TTime::enctNow()))
    {
        TBinMBufInStream strmIn(&pcciFind->mbufData(), pcciFind->c4Bytes());
        strmIn >> iwdgToFill;
    }
     else
    {
        // Oh well, let's call the other one
        TDataSrvClient dsclLoad;
        QueryTemplate(strName, dsclLoad, iwdgToFill, cuctxToUse, bCacheEnabled);
    }
}


//
//  Updates the last check stamp on an image in the web image cache. This
//  is done if a web image widget checks the cache and we tell it that we
//  have it but the last time it was checked is far enough back that he
//  web widget decides to check it again. If it can determine that it is
//  up to date without re-querying the data, it can call this to just update
//  the stamp.
//
tCIDLib::TVoid TFacCQCIntfEng::RefreshWebImage(const TString& strURL)
{
    TLocker lockrSync(&m_mtxSync);
    TIntfImgCachePtr cptrToSet;

    TString strPath(kCQCIntfEng_::strImagePref);
    strPath.Append(strURL);

    if (CQCIntfEng_ThisFacility::colWebImgCache().bFindByKey(strPath, cptrToSet))
        cptrToSet->SetLastCheck();
}


//
//  Sometimes the designer or themes need to automatically generate an id for a
//  widget. And, in some cases, when widgets are copied and pasted into the
//  same template, a new id will need to be generated for it until such time as
//  the user decides to change it.
//
//  If, by some bizarre chance, we cannot get a unique one after 1024
//  loops, we give up and throw.
//
TString
TFacCQCIntfEng::strGenWidgetId( const   TCQCIntfContainer&  iwdgCont
                                , const TString&            strBaseName)
{
    TString strNew = strBaseName;
    const tCIDLib::TCard4 c4OrgLen = strBaseName.c4Length();
    tCIDLib::TCard4 c4Suffix = 2;

    const TCQCIntfWidget* piwdgDup = iwdgCont.piwdgFindByName(strNew, kCIDLib::False);
    while (piwdgDup)
    {
        strNew.CapAt(c4OrgLen);
        strNew.AppendFormatted(c4Suffix);
        piwdgDup = iwdgCont.piwdgFindByName(strNew, kCIDLib::False);
        if (!piwdgDup)
            break;

        if (c4Suffix > 1024)
        {
            facCQCIntfEng().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kIEngErrs::errcDbg_MakeUniqueId
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Internal
                , strBaseName
            );
        }
        c4Suffix++;
    }
    return strNew;
}


//
//  We provide a common title to use as the popup error text for any
//  windows created for general interface viewing.
//
const TString& TFacCQCIntfEng::strTitle() const
{
    return m_strTitle;
}


//
//  This method provides the fundamental operation of template resizing. It's
//  used from a couple places where we support scaling templates from one resolution
//  to another.
//
//  The caller makes a copy of the source template and passes us that copy, along
//  with the horizontal and vertical scaling factors.
//
tCIDLib::TVoid
TFacCQCIntfEng::ScaleTemplate(          TGraphDrawDev&      gdevTmp
                                ,       TCQCIntfTemplate&   iwdgToScale
                                , const tCIDLib::TFloat8    f8XScale
                                , const tCIDLib::TFloat8    f8YScale)
{
    TArea areaCur;
    TArea areaNew;

    TPoint pntNew;
    TSize  szOrg;
    TSize  szNew;
    TFontMetrics fmtrCur;

    //
    //  Update the size. Tell it not to report the change to child widgets since
    //  we want them to stay unaffected for below and because they are not set up
    //  yet and could try to do something that would fail.
    //
    szNew = iwdgToScale.areaRelative().szArea();
    szNew.Scale(f8XScale, f8YScale);
    iwdgToScale.SetSize(szNew, kCIDLib::False);

    const tCIDLib::TCard4 c4Count = iwdgToScale.c4ChildCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TCQCIntfWidget& iwdgCur = iwdgToScale.iwdgAt(c4Index);

        // Get the area out and scale the size and position
        areaCur = iwdgCur.areaRelative();

        pntNew = areaCur.pntOrg();
        szNew = areaCur.szArea();
        szOrg = szNew;

        pntNew.Scale(f8XScale, f8YScale);
        szNew.Scale(f8XScale, f8YScale);

        //
        //  Set the area back on the widget. Because it's just a raw template not
        //  yet loaded into the viewer, the relative and absolute areas are the
        //  same, so we don't have to have access to the relative area setting
        //  method.
        //
        //  Tell it not to try to report area changes since that won't work in it's
        //  unloaded state.
        //
        areaNew.Set(pntNew, szNew);
        iwdgCur.SetArea(areaNew, kCIDLib::False);

        //
        //  Give it a chance to do any internal scaling. At the widget level, this
        //  will check if this widget implements the command source interface and
        //  swizzle any commands if needed.
        //
        iwdgCur.ScaleInternal(f8XScale, f8YScale);

        // Process the font if it implements the font interfce
        MCQCIntfFontIntf* pmiwdgFont = dynamic_cast<MCQCIntfFontIntf*>(&iwdgCur);
        if (pmiwdgFont)
            pmiwdgFont->ScaleFont(f8XScale, f8YScale);

        //
        //  If it implements the image list interface, let it scale its internal data,
        //  not the images which are handled separately, but some other info.
        //
        MCQCIntfImgListIntf* pmiwdgImgList = dynamic_cast<MCQCIntfImgListIntf*>(&iwdgCur);
        if (pmiwdgImgList)
            pmiwdgImgList->ScaleImgList(f8XScale, f8YScale);
    }
}


//
//  This method handles the standard background color fill that all widgets
//  support (in the Base attributes tab.)
//
tCIDLib::TVoid
TFacCQCIntfEng::StdBgnFill(         TGraphDrawDev&          gdevTarget
                            , const TArea&                  areaToFill
                            , const tCQCIntfEng::EBgnStyles eStyle
                            , const TRGBClr&                rgbBgn1
                            , const TRGBClr&                rgbBgn2)
{
    // Make sure we don't affect the drawing device
    TGraphicDevJanitor janDev(&gdevTarget);

    switch(eStyle)
    {
        case tCQCIntfEng::EBgnStyles::Fill :
        {
            gdevTarget.Fill(areaToFill, rgbBgn1);
            break;
        }

        case tCQCIntfEng::EBgnStyles::CGradient :
        {
            //
            //  We just fill the right/left sides with separate gradient
            //  fills that meet in the middle with the same color.
            //
            //  If the area is an odd number of pixels, this can leave a line
            //  in the center due to rounding, so just bump the left hand
            //  side size up by 1 pixel just in case. If it's not needed,
            //  it'll get overwritten.
            //
            TArea areaTmp(areaToFill);
            areaTmp.ScaleSize(0.5, 1);
            areaTmp.AdjustSize(1, 0);
            gdevTarget.GradientFill(areaTmp, rgbBgn1, rgbBgn2);

            areaTmp.RightJustifyIn(areaToFill);
            gdevTarget.GradientFill(areaTmp, rgbBgn2, rgbBgn1);
            break;
        }

        case tCQCIntfEng::EBgnStyles::LGradient :
        {
            //
            //  Do a gradient from the left out to 15% of the width, then
            //  complete the rest with the second color.
            //
            TArea areaTmp(areaToFill);
            areaTmp.ScaleSize(0.15, 1.0);
            gdevTarget.GradientFill(areaTmp, rgbBgn1, rgbBgn2);
            areaTmp.i4Left(areaTmp.i4Right());
            areaTmp.i4Right(areaToFill.i4Right());
            gdevTarget.Fill(areaTmp, rgbBgn2);
            break;
        }

        case tCQCIntfEng::EBgnStyles::HGradient :
        case tCQCIntfEng::EBgnStyles::VGradient :
        {
            gdevTarget.GradientFill
            (
                areaToFill
                , rgbBgn1
                , rgbBgn2
                , (eStyle == tCQCIntfEng::EBgnStyles::HGradient)
                   ? tCIDGraphDev::EGradDirs::Horizontal
                   : tCIDGraphDev::EGradDirs::Vertical
            );
            break;
        }

        default :
            CIDAssert2(L"Unknown standard background fill type")
            break;
    };
}


//
//  The containing code must call this before making any use of the interface engine. It
//  has to provide us with a polling engine to use. It should have already started it up
//  and are responsible for stopping it when everyone is done with it.
//
//  We also get a compatible graphics device so we can do any image bootstrapping we need.
//
//  No locking to deal with here since this is before everything gets cranked up.
//
tCIDLib::TVoid TFacCQCIntfEng::StartEngine(TCQCPollEngine* const ppolleToUse)
{
    m_ppolleThis = ppolleToUse;

    //
    //  We pre-create the image data for a 1x1 transparent image. Any time we can't download
    //  a repo image, this is set as the image data. If it later becomes available it will
    //  get picked up the next time that image is checked.
    //
    //  We map it to the real image /System/Blank, so that it's a path to a real image
    //  in the repo. This just insures it's available at all times.
    //
    TPixelArray pixaData
    (
        tCIDImage::EPixFmts::TrueAlpha
        , tCIDImage::EBitDepths::Eight
        , tCIDImage::ERowOrders::TopDown
        , TSize(1, 1)
    );
    pixaData.SetAllAlpha(0);

    // Create PNG image from that
    TPNGImage imgBlank(pixaData);

    // Stream it out to a memory buffer for later use and remember the size
    {
        TBinMBufOutStream strmTar(&m_mbufEmptyImg);
        strmTar << imgBlank;
        strmTar.Flush();
        m_c4EmptyImgSz = strmTar.c4CurPos();
    }

    // Now let's create our default empty images from it
    const TString strImgPath(L"/System/Blank");
    if (!CQCIntfEng_ThisFacility::colImgCache().bKeyExists(strImgPath))
    {
        TGraphMemDev gdevCompat
        (
            TSize(10, 10), tCIDImage::EBitDepths::Eight, tCIDImage::EPixFmts::TrueAlpha
        );

        m_cptrEmptyImg = cptrNewCacheItem(strImgPath, m_c4EmptyImgSz, kCIDLib::False);
        m_cptrEmptyImg->Set(kCIDLib::False, m_c4EmptyImgSz, m_mbufEmptyImg, 0, gdevCompat);

        m_cptrEmptyThumbImg = cptrNewCacheItem(strImgPath, m_c4EmptyImgSz, kCIDLib::True);
        m_cptrEmptyThumbImg->Set(kCIDLib::True, m_c4EmptyImgSz, m_mbufEmptyImg, 0, gdevCompat);
    }
}


//
//  Shuts down the engine if it was created. Currently there's nothing to do. We
//  used to have our own polling engine but now the containing client provides that
//  and handles starting/stopping it.
//
//  However we do clear our pointer so that any subsequent access (which shouldn't
//  happen) will get caught.
//
tCIDLib::TVoid TFacCQCIntfEng::StopEngine()
{
    m_ppolleThis = nullptr;
}


tCIDCtrls::TWndId TFacCQCIntfEng::widNext()
{
    m_widNext++;
    if (!m_widNext)
        m_widNext++;
    return m_widNext;
}


//
//  Most field writing by widgets is done through this method, which will
//  display an error on any failure. Some that handle the error themselves
//  will go straight to the polling engine.
//
tCIDLib::TVoid
TFacCQCIntfEng::WriteField(         TCQCIntfView&   civOwner
                            , const TString&        strMoniker
                            , const TString&        strFldName
                            , const TString&        strValue)
{
    try
    {
        m_ppolleThis->WriteField(strMoniker, strFldName, strValue, civOwner.sectUser());
    }

    catch(TError& errToCatch)
    {
        civOwner.ShowErr
        (
            civOwner.strTitle()
            , facCQCIntfEng().strMsg
              (
                kIEngMsgs::midStatus_FldWriteFailed
                , strMoniker
                , strFldName
              )
            , errToCatch
        );
    }
}


// ---------------------------------------------------------------------------
//  TFacCQCIntfEng: Public, static data members
// ---------------------------------------------------------------------------
tCIDLib::TBoolean   TFacCQCIntfEng::s_bDesMode(kCIDLib::False);
tCIDLib::TBoolean   TFacCQCIntfEng::s_bRemoteMode(kCIDLib::False);
tCIDLib::TBoolean   TFacCQCIntfEng::s_bSimulatorMode(kCIDLib::False);
tCIDLib::TBoolean   TFacCQCIntfEng::s_bWebRIVAMode(kCIDLib::False);
TMArtCache*         TFacCQCIntfEng::s_pcolArtCache;



// ---------------------------------------------------------------------------
//  TFacCQCIntfEng: Private, non-virtual methods
// ---------------------------------------------------------------------------


//
//  We assume that the caller has locked the synx mutex to protect us while
//  we putz around with the cache.
//
//  THIS ONLY creates cache items for repo based images.
//
TIntfImgCachePtr
TFacCQCIntfEng::cptrNewCacheItem(const  TString&            strName
                                , const tCIDLib::TCard4     c4ImageSz
                                , const tCIDLib::TBoolean   bThumb)
{
    const tCIDLib::TCard4 c4Count = CQCIntfEng_ThisFacility::colImgCache().c4ElemCount();

    //
    //  If the cache has reached the working size, we want to start seeing if we
    //  can toss one before we put another one in.
    //
    if (c4Count > CQCIntfEng_ThisFacility::c4WorkingSize)
    {
        CQCIntfEng_ThisFacility::TImgCache::TNCCursor cursImgs
        (
            &CQCIntfEng_ThisFacility::colImgCache()
        );
        CQCIntfEng_ThisFacility::TImgCache::TNCCursor cursOldest
        (
            &CQCIntfEng_ThisFacility::colImgCache()
        );

        for (; cursImgs; ++cursImgs)
        {
            // Get the current one and check it
            TIntfImgCachePtr& cptrCur = cursImgs.objWCur();

            //
            //  If it's unused, then it's a candidate. The count should be at
            //  least 1, since it's in the cache itself, but check 0 or 1 just
            //  to be safe.
            //
            if (cptrCur.c4StrongCount() <= 1)
            {
                if (!cursOldest.bIsValid()
                ||  (cptrCur->enctLastAccess() < cursOldest.objRCur()->enctLastAccess()))
                {
                    cursOldest = cursImgs;
                }
            }
        }

        // If we found one we can dump, then do so
        if (cursOldest.bIsValid())
            CQCIntfEng_ThisFacility::colImgCache().RemoveAt(cursOldest);
    }

    // And add another one now
    return CQCIntfEng_ThisFacility::colImgCache().objAdd
    (
        TIntfImgCachePtr
        (
            new TIntfImgCacheItem(strName, bThumb, tCQCIntfEng::EImgTypes::Repo)
        )
    );
}


//
//
//  Faults in the widget type list. The list member is mutable so that we
//  can be called from const methods that access the list.
//
tCIDLib::TVoid TFacCQCIntfEng::FaultInTypeList() const
{
    // Fault the list in as required
    if (!CQCIntfEng_ThisFacility::atomTypeListDone)
    {
        TBaseLock lockInit;
        if (!CQCIntfEng_ThisFacility::atomTypeListDone)
        {
            //
            //  Create a little map that we can use to load up the list of widget
            //  types. We support a set of prefix types, to add a prefix to the
            //  the human readable text. This is used to indicate field based,
            //  static, or variable based.
            //
            //  0 = No prefix
            //  1 = Field
            //  2 = Static
            //  3 = Variable
            //
            struct TTypeMap
            {
                const tCIDLib::TCh* pszTypeName;
                tCIDLib::TMsgId     midName;
                tCIDLib::TCard4     c4Prefix;
            };


            static const TTypeMap amapItems[] =
            {
                { L""                       , kIEngMsgs::midWidget_Animations          , 0 }
              , { L"TCQCIntfFldAnimatedImg" , kIEngMsgs::midWidget_AnimatedImg         , 1 }
              , { L"TCQCIntfStaticAnimatedImg" , kIEngMsgs::midWidget_AnimatedImg      , 2 }
              , { L"TCQCIntfVarAnimatedImg" , kIEngMsgs::midWidget_AnimatedImg         , 3 }
              , { L""                       , 0                                        , 0 }

              , { L""                       , kIEngMsgs::midWidget_Browsers            , 0 }
              , { L"TCQCIntfFldListBrowser" , kIEngMsgs::midWidget_ListBrowser         , 1 }
              , { L"TCQCIntfStaticListBrowser", kIEngMsgs::midWidget_ListBrowser       , 2 }
              , { L"TCQCIntfWebBrowser"     , kIEngMsgs::midWidget_WebBrowser          , 0 }
              , { L""                       , 0                                        , 0 }

              , { L""                       , kIEngMsgs::midWidget_Cameras             , 0 }
              , { L"TCQCIntfWebCamera"      , kIEngMsgs::midWidget_WebCamera           , 0 }

              , { L""                       , kIEngMsgs::midWidget_Buttons             , 0 }
              , { L"TCQCIntfCmdButton"      , kIEngMsgs::midWidget_CmdButton           , 0 }
              , { L"TCQCIntfFldCmdButton"   , kIEngMsgs::midWidget_CmdButton           , 1 }
              , { L"TCQCIntfFieldCheckBox"  , kIEngMsgs::midWidget_CheckBox            , 1 }
              , { L"TCQCIntfSActPushButton" , kIEngMsgs::midWidget_SpecialAction       , 0 }
              , { L"TCQCIntfStaticCheckBox" , kIEngMsgs::midWidget_CheckBox            , 2 }
              , { L"TCQCIntfVarCheckBox"    , kIEngMsgs::midWidget_CheckBox            , 3 }
              , { L"TCQCIntfVarCmdButton"   , kIEngMsgs::midWidget_CmdButton           , 3 }
              , { L""                       , 0                                        , 0 }

              , { L""                       , kIEngMsgs::midWidget_LinesFills          , 0 }
              , { L"TCQCIntfAreaFill"       , kIEngMsgs::midWidget_AreaFill            , 0 }
              , { L"TCQCIntfLine"           , kIEngMsgs::midWidget_Line                , 0 }
              , { L""                       , 0                                        , 0 }

              , { L""                       , kIEngMsgs::midWidget_Containers          , 0 }
              , { L"TCQCIntfLiveTile"       , kIEngMsgs::midWidget_LiveTile            , 0 }
              , { L"TCQCIntfOverlay"        , kIEngMsgs::midWidget_Overlay             , 0 }
              , { L""                       , 0                                        , 0 }

              , { L""                       , kIEngMsgs::midWidget_Times               , 0 }
              , { L"TCQCIntfStaticCalendar" , kIEngMsgs::midWidget_Calendar            , 2 }
              , { L"TCQCIntfFldTimeImg"     , kIEngMsgs::midWidget_TimeImg             , 1 }
              , { L"TCQCIntfFieldTimeText"  , kIEngMsgs::midWidget_TimeText            , 1 }
              , { L"TCQCIntfDigitalClock"   , kIEngMsgs::midWidget_DigitalClock        , 2 }
              , { L"TCQCIntfLocalTimeImg"   , kIEngMsgs::midWidget_TimeImg             , 2 }
              , { L"TCQCIntfLocalTimeText"  , kIEngMsgs::midWidget_TimeText            , 2 }
              , { L""                       , 0                                        , 0 }

              , { L""                       , kIEngMsgs::midWidget_Images              , 0 }
              , { L"TCQCIntfFldBooleanImg"  , kIEngMsgs::midWidget_BooleanImg          , 1 }
              , { L"TCQCIntfDrvBDImg"       , kIEngMsgs::midWidget_DrvBDImage          , 1 }
              , { L"TCQCIntfFldEnumImg"     , kIEngMsgs::midWidget_EnumImg             , 1 }
              , { L"TCQCIntfFldWebImg"      , kIEngMsgs::midWidget_WebImage            , 1 }
              , { L"TCQCIntfDTVLogoImg"     , kIEngMsgs::midWidget_LogoImg             , 1 }
              , { L"TCQCIntfFldMappedImg"   , kIEngMsgs::midWidget_MappedImg           , 1 }
              , { L"TCQCIntfLinMPatchImg"   , kIEngMsgs::midWidget_LinMPatchImg        , 0 }
              , { L"TCQCIntfStaticBooleanImg",kIEngMsgs::midWidget_BooleanImg          , 2 }
              , { L"TCQCIntfStaticImg"      , kIEngMsgs::midWidget_Image               , 2 }
              , { L"TCQCIntfStaticWebImg"   , kIEngMsgs::midWidget_WebImage            , 2 }
              , { L"TCQCIntfVarBooleanImg"  , kIEngMsgs::midWidget_BooleanImg          , 3 }
              , { L"TCQCIntfVarMappedImg"   , kIEngMsgs::midWidget_MappedImg           , 3 }
              , { L"TCQCIntfVarWebImg"      , kIEngMsgs::midWidget_WebImage            , 3 }
              , { L"TCQCIntfWeatherChImg"   , kIEngMsgs::midWidget_WeatherChImg        , 0 }
              , { L""                       , 0                                        , 0 }

              , { L""                       , kIEngMsgs::midWidget_Media               , 0 }
              , { L"TCQCIntfAdvMItemBrowser", kIEngMsgs::midWidget_AdvMItemBrowser     , 0 }
              , { L"TCQCIntfCvrArtBrowser"  , kIEngMsgs::midWidget_CvrArtBrowser       , 0 }
              , { L"TCQCIntfMediaCatBrowser", kIEngMsgs::midWidget_MCatBrowser         , 0 }
              , { L"TCQCIntfMediaImg"       , kIEngMsgs::midWidget_MediaImg            , 0 }
              , { L"TCQCIntfMediaListBrowser",kIEngMsgs::midWidget_MediaListBrowser    , 0 }
              , { L"TCQCIntfMediaRepoImg"   , kIEngMsgs::midWidget_MediaRepoImg        , 0 }
              , { L"TCQCIntfMediaRepoText"  , kIEngMsgs::midWidget_MediaRepoText       , 0 }
              , { L"TCQCIntfMItemBrowser"   , kIEngMsgs::midWidget_MItemBrowser        , 0 }
              , { L"TCQCIntfMRItemBrowser"  , kIEngMsgs::midWidget_MRItemBrowser       , 0 }
              , { L""                       , 0                                        , 0 }

              , { L""                       , kIEngMsgs::midWidget_MetersBarsGraphs    , 0 }
              , { L"TCQCIntfFldVUMeter"     , kIEngMsgs::midWidget_DigitalVUMeter      , 1 }
              , { L"TCQCIntfGraphLS"        , kIEngMsgs::midWidget_LSGraph             , 0 }
              , { L"TCQCIntfFieldProgBar"   , kIEngMsgs::midWidget_ProgressBar         , 1 }
              , { L"TCQCIntfStaticProgBar"  , kIEngMsgs::midWidget_ProgressBar         , 2 }
              , { L"TCQCIntfToolbar"        , kIEngMsgs::midWidget_Toolbar             , 0 }
              , { L"TCQCIntfVarToolbar"     , kIEngMsgs::midWidget_Toolbar             , 3 }
              , { L"TCQCIntfVarVUMeter"     , kIEngMsgs::midWidget_DigitalVUMeter      , 3 }
              , { L""                       , 0                                        , 0 }

              , { L""                       , kIEngMsgs::midWidget_Palettes            , 0 }
              , { L"TCQCIntfClrPalette"     , kIEngMsgs::midWidget_ClrPalette          , 0 }

              , { L""                       , kIEngMsgs::midWidget_SlidersKnobs        , 0 }
              , { L"TCQCIntfFieldSlider"    , kIEngMsgs::midWidget_Slider              , 1 }
              , { L"TCQCIntfFldVolKnob"     , kIEngMsgs::midWidget_VolKnob             , 1 }
              , { L"TCQCIntfStaticSlider"   , kIEngMsgs::midWidget_Slider              , 2 }
              , { L"TCQCIntfStaticVolKnob"  , kIEngMsgs::midWidget_VolKnob             , 2 }
              , { L"TCQCIntfVarSlider"      , kIEngMsgs::midWidget_Slider              , 3 }
              , { L"TCQCIntfVarVolKnob"     , kIEngMsgs::midWidget_VolKnob             , 3 }
              , { L""                       , 0                                        , 0 }

              , { L""                       , kIEngMsgs::midWidget_Textual             , 0 }
              , { L"TCQCIntfFldBooleanText" , kIEngMsgs::midWidget_BooleanText         , 1 }
              , { L"TCQCIntfNumericText"    , kIEngMsgs::midWidget_NumericText         , 1 }
              , { L"TCQCIntfFldMarquee"     , kIEngMsgs::midWidget_Marquee             , 1 }
              , { L"TCQCIntfFldText"        , kIEngMsgs::midWidget_Text                , 1 }
              , { L"TCQCIntfStaticMarquee"  , kIEngMsgs::midWidget_Marquee             , 2 }
              , { L"TCQCIntfStaticText"     , kIEngMsgs::midWidget_Text                , 2 }
              , { L"TCQCIntfTitleText"      , kIEngMsgs::midWidget_TitleText           , 0 }
              , { L"TCQCIntfVarBooleanText" , kIEngMsgs::midWidget_BooleanText         , 3 }
              , { L"TCQCIntfVarText"        , kIEngMsgs::midWidget_Text                , 3 }
              , { L"TCQCIntfVarMarquee"     , kIEngMsgs::midWidget_Marquee             , 3 }
              , { L""                       , 0                                        , 0 }
            };
            const tCIDLib::TCard4 c4Size = tCIDLib::c4ArrayElems(amapItems);

            // Fill in the type list
            TString strText;
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Size; c4Index++)
            {
                const TTypeMap itemCur = amapItems[c4Index];
                const tCIDLib::TCard4 c4CurId = itemCur.midName;

                if (c4CurId)
                {
                    tCIDLib::TCard4 c4PrefId = 0;
                    switch(itemCur.c4Prefix)
                    {
                        case 1 :
                            c4PrefId = kIEngMsgs::midWidget_PrefField;
                            break;

                        case 2 :
                            c4PrefId = kIEngMsgs::midWidget_PrefStatic;
                            break;

                        case 3 :
                            c4PrefId = kIEngMsgs::midWidget_PrefVariable;
                            break;

                        default :
                            break;
                    };

                    if (c4PrefId)
                        strText = facCQCIntfEng().strMsg(c4PrefId);
                    else
                        strText.Clear();
                    strText.Append(facCQCIntfEng().strMsg(c4CurId));
                }

                m_colTypes.objAdd
                (
                    TKeyValuePair
                    (
                        amapItems[c4Index].pszTypeName
                        , c4CurId ? strText : TString::strEmpty()
                    )
                );
            }

            CQCIntfEng_ThisFacility::atomTypeListDone.Set();
        }
    }
}


// Lock and clear out the template cache
tCIDLib::TVoid TFacCQCIntfEng::FlushTmplCache()
{
    TLocker lockrSync(&m_mtxSync);
    CQCIntfEng_ThisFacility::colTmplCache().RemoveAll();
}


