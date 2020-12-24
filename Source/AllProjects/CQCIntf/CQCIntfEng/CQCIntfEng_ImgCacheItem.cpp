//
// FILE NAME: CQCIntfEng_ImgCacheItem.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/25/2009
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
//  This file implements a derivate of the base image cache item provided
//  by the CIDWnd facility. It just adds a little functionality for our
//  own needs and intercepts a couple methods so we can do something in
//  addition to the base class functionality.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCIntfEng_.hpp"




// ---------------------------------------------------------------------------
//   CLASS: TIntfImgCacheItem
//  PREFIX: ici
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TIntfImgCacheItem: Constructors and Destructor
// ---------------------------------------------------------------------------
TIntfImgCacheItem::TIntfImgCacheItem(const  TString&                strImgName
                                    , const tCIDLib::TBoolean       bThumb
                                    , const tCQCIntfEng::EImgTypes  eType) :

    TImgCacheItem(strImgName, bThumb)
    , m_eType(eType)
{
    // Set up the key
    if (bThumb)
        m_strImageKey = kCQCIntfEng_::strThumbPref;
    else
        m_strImageKey = kCQCIntfEng_::strImagePref;
    m_strImageKey.Append(strImgName);
    m_strImageKey.ToLower();
}

TIntfImgCacheItem::TIntfImgCacheItem(const TIntfImgCacheItem& iciSrc) :

    TImgCacheItem(iciSrc)
    , m_eType(iciSrc.m_eType)
    , m_mbufData(iciSrc.m_mbufData.c4Size())
    , m_strImageKey(iciSrc.m_strImageKey)
{
    //
    //  Copy over only the actual bytes of the image. The source buffer could
    //  be larger than that.
    //
    m_mbufData.CopyIn(iciSrc.m_mbufData, iciSrc.c4Size());
}

TIntfImgCacheItem::~TIntfImgCacheItem()
{
}


// ---------------------------------------------------------------------------
//  TIntfImgCacheItem: Public operators
// ---------------------------------------------------------------------------
TIntfImgCacheItem&
TIntfImgCacheItem::operator=(const TIntfImgCacheItem& iciSrc)
{
    if (this != &iciSrc)
    {
        TImgCacheItem::operator=(iciSrc);
        m_eType       = iciSrc.m_eType;
        m_strImageKey = iciSrc.m_strImageKey;

        //
        //  Copy over only the actual bytes of the image. The source buffer
        //  could be larger than that.
        //
        if (m_mbufData.c4Size() < iciSrc.c4Size())
            m_mbufData.Reallocate(iciSrc.c4Size(), kCIDLib::False);
        m_mbufData.CopyIn(iciSrc.m_mbufData, iciSrc.c4Size());
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TIntfImgCacheItem: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  Resets the cache item back to default state. If the derived class
//  overrides, it should call us first.
//
tCIDLib::TVoid TIntfImgCacheItem::Reset(const   TString&            strImgName
                                        , const tCIDLib::TBoolean   bThumb)
{
    TImgCacheItem::Reset(strImgName, bThumb);

    // Build up our key
    if (bThumb)
        m_strImageKey = kCQCIntfEng_::strThumbPref;
    else
        m_strImageKey = kCQCIntfEng_::strImagePref;
    m_strImageKey.Append(strImgName);
    m_strImageKey.ToLower();
}


//
//  We override this so we can force the user data on the newly added image. Most of
//  the time the non-inherited version of this below is called, since it allows for
//  a move of the data.
//
tCIDLib::TVoid
TIntfImgCacheItem::Set( const   tCIDLib::TBoolean   bThumb
                        , const tCIDLib::TCard4     c4Size
                        , const TMemBuf&            mbufData
                        , const tCIDLib::TCard4     c4SerialNum
                        , const TGraphDrawDev&      gdevCompat)
{
    TImgCacheItem::Set(bThumb, c4Size, mbufData, c4SerialNum, gdevCompat);

    // Set the path on the image as the user data on the image
    TString strUserData;
    if (m_eType == tCQCIntfEng::EImgTypes::Repo)
        strUserData = kCQCKit::strWRIVA_RepoPref;
    else
        strUserData = kCQCKit::strWRIVA_WebPref;
    strUserData.Append(strImageName());
    bmpWriteable().strUserData(strUserData);

    // If in remote mode, store the raw data
    if (TFacCQCIntfEng::bRemoteMode())
    {
        if (m_mbufData.c4Size() < c4Size)
            m_mbufData.Reallocate(c4Size, kCIDLib::False);
        m_mbufData.CopyIn(mbufData, c4Size);
    }
}


// ---------------------------------------------------------------------------
//  TIntfImgCacheItem: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Provide access to the raw data, only valid in remote mode
const TMemBuf& TIntfImgCacheItem::mbufIVImgData() const
{
    CIDAssert(TFacCQCIntfEng::bRemoteMode(), L"No image cache data except in remote mode");
    return m_mbufData;
}


// Provide access to the image key
const TString& TIntfImgCacheItem::strImageKey() const
{
    return m_strImageKey;
}


//
//  We create a new version of Set() from the inherited one above, which allows for
//  a move of the buffer data into our buffer.
//
tCIDLib::TVoid
TIntfImgCacheItem::Set( const   tCIDLib::TBoolean   bThumb
                        , const tCIDLib::TCard4     c4Size
                        ,       THeapBuf&&          mbufData
                        , const tCIDLib::TCard4     c4SerialNum
                        , const TGraphDrawDev&      gdevCompat)
{
    // Call the parent version of the virtual
    TImgCacheItem::Set(bThumb, c4Size, mbufData, c4SerialNum, gdevCompat);

    // Set the path on the image as the user data on the image
    TString strUserData;
    if (m_eType == tCQCIntfEng::EImgTypes::Repo)
        strUserData = kCQCKit::strWRIVA_RepoPref;
    else
        strUserData = kCQCKit::strWRIVA_WebPref;
    strUserData.Append(strImageName());
    bmpWriteable().strUserData(strUserData);

    // If in remote mode, store the raw data
    if (TFacCQCIntfEng::bRemoteMode())
        m_mbufData = tCIDLib::ForceMove(mbufData);
}
