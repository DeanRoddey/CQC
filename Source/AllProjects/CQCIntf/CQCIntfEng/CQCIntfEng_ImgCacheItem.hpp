//
// FILE NAME: CQCIntfEng_ImgCacheItem.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/26/2009
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
//  We need small derivative of the standard image cache item for our image
//  cache. We need to override a couple things. Widgets get a counted pointer
//  to one of these when they reference images from the engine's image
//  cache, so we also define that counted pointer type below.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TIntfImgCacheItem
//  PREFIX: ici
// ---------------------------------------------------------------------------
class TIntfImgCacheItem : public TImgCacheItem
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TIntfImgCacheItem
        (
            const   TString&                strImgName
            , const tCIDLib::TBoolean       bThumb
            , const tCQCIntfEng::EImgTypes  eType
        );

        TIntfImgCacheItem
        (
            const   TString&                strImgName
            , const tCIDLib::TBoolean       bThumb
            , const tCIDLib::TCard4         c4Size
        );

        TIntfImgCacheItem
        (
            const   TIntfImgCacheItem&      iciToCopy
        );

        ~TIntfImgCacheItem();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TIntfImgCacheItem& operator=
        (
            const   TIntfImgCacheItem&      iciToAssign
        );


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid Reset
        (
            const   TString&                strImgName
            , const tCIDLib::TBoolean       bThumb
        );

        tCIDLib::TVoid Set
        (
            const   tCIDLib::TBoolean       bThumb
            , const tCIDLib::TCard4         c4Size
            , const TMemBuf&                mbufData
            , const tCIDLib::TCard4         c4SerialNum
            , const TGraphDrawDev&          gdevCompat
        );


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        const TMemBuf& mbufIVImgData() const;

        const TString& strImageKey() const;

        // This is the same name as the virtual above, but a different signature
        tCIDLib::TVoid Set
        (
            const   tCIDLib::TBoolean       bThumb
            , const tCIDLib::TCard4         c4Size
            ,       THeapBuf&&              mbufData
            , const tCIDLib::TCard4         c4SerialNum
            , const TGraphDrawDev&          gdevCompat
        );



    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_eType
        //      We are told what type of image is being cached (there are multiple
        //      caches.) This is used to set the correct image type  prefix on the
        //      image user data that we send to the RIVA clients.
        //
        //  m_mbufData
        //      When in remote viewer mode (i.e. running inside the RIVA server)
        //      we will cache the raw image data as well as the bitmap that
        //      the parent class stores, so that we can serve up the images
        //      almost all the time directly from our cache.
        //
        //  m_strImageKey
        //      We create a key based on the persistent id of the image, the repo
        //      from which it was gotten
        //
        //      This is used as the key in the hash set that is used to implement
        //      the cache.
        // -------------------------------------------------------------------
        tCQCIntfEng::EImgTypes  m_eType;
        THeapBuf                m_mbufData;
        TString                 m_strImageKey;
};

#pragma CIDLIB_POPPACK


// ---------------------------------------------------------------------------
//  Define a counted pointer to an image cache object. This is what we hand
//  out to widgets when they want to use an image. This insures that the image
//  will not be dropped from the cache as long as someone is holding it.
// ---------------------------------------------------------------------------
using TIntfImgCachePtr = TCntPtr<TIntfImgCacheItem>;
