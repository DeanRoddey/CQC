//
// FILE NAME: CQCMedia_DBInfo.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/21/2005
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
//  This is the header file for the CQCMedia_DBInfo.cpp file, which provides the classes
//  that make up the in-memory media database of a media driver. It defines a set of classes
//  that are used to maintain a database in memory that can be browsed by clients to access
//  media metadata. In our own repository it also provides the persistent in/out streaming
//  of the data. For other repositories, we read in some other format and load it into a
//  database only in memory.
//
//  These classes represent a hierarchy that is used in all CQC repositories, of Title Sets
//  (multi-disc sets) which include Collections (DVDs/CDs) and Items (individual tracks.)
//  Pictures are also shoehorned into this with title sets representing groups of groups of
//  pictures, collections are groups of pictures, and items individual pictures.
//
//  We also include some classes that are not fundamental components of the database, but
//  are used in providing non-persistent alternative views of the database, such as a view
//  of title sets by associated artist. These just provide
//
// CAVEATS/GOTCHAS:
//
//  1)  At this time, title sets cannot be mixed media. That is something could be very nice
//  later on, but for now the complexities are too much to deal with. So for now all the
//      collections in a set will be of the same media type. Therefore the outside world
//      can ask for the media type of the set and know what it's dealing with without digging
//      deeper.
//
//  2)  We currently don't cache poster art. They can be quite large and they are never browsed.
//      They are only needed when a movie is playing. But, we don't wnat to special case it all
//      over the place. So we treat it the same but the media image class will not cache it and
//      will always return zero bytes for the size, so that it'll always need to be loaded. And
//      this leaves us open to cache it later if desired.
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

class TMediaDB;
class TMediaTitleSet;


// ---------------------------------------------------------------------------
//   CLASS: TMediaDBBase
//  PREFIX: mdbb
// ---------------------------------------------------------------------------
class CQCMEDIAEXPORT TMediaDBBase : public TObject, public MStreamable
{
    public :
        // -------------------------------------------------------------------
        //  Destructor
        // -------------------------------------------------------------------
        ~TMediaDBBase();


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        virtual tCIDLib::TVoid Reset();

        virtual tCIDLib::TVoid ToXML
        (
                    TTextOutStream&         strmTar
        )   const;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TCard2 c2ArtId() const
        {
            return m_c2ArtId;
        }

        tCIDLib::TCard2 c2ArtId
        (
            const   tCIDLib::TCard2         c2ToSet
        );

        tCIDLib::TCard2 c2Id() const
        {
            return m_c2Id;
        }

        tCIDLib::TCard2 c2Id
        (
            const   tCIDLib::TCard2         c2ToSet
        );

        tCIDLib::TCard4 c4AltId() const;

        tCIDLib::TCard4 c4AltId
        (
            const   tCIDLib::TCard4         c4ToSet
        );

        tCIDLib::TCard4 c4LoadOrder() const;

        tCIDLib::TCard4 c4LoadOrder
        (
            const   tCIDLib::TCard4         c4ToSet
        );

        tCQCMedia::EMediaTypes eType() const
        {
            return m_eType;
        }

        tCIDLib::TVoid ResetType
        (
            const   tCQCMedia::EMediaTypes  eType
        );

        tCIDLib::TEncodedTime enctStamp() const
        {
            return m_enctStamp;
        }

        tCIDLib::TEncodedTime enctStamp
        (
            const   tCIDLib::TEncodedTime   enctToSet
        );

        const TString& strAltId() const;

        const TString& strAltId
        (
            const   TString&                strToSet
        );

        const TString& strName() const
        {
            return m_strName;
        }

        const TString& strName
        (
            const   TString&                strToSet
        );

        const TString& strUniqueId() const
        {
            return m_strUniqueId;
        }

        const TString& strUniqueId
        (
            const   TString&                strToSet
        );


    protected :
        // -------------------------------------------------------------------
        //  Hidden constructors and operators
        // -------------------------------------------------------------------
        TMediaDBBase() = delete;
        TMediaDBBase
        (
            const   tCQCMedia::EMediaTypes  eType
        );

        TMediaDBBase
        (
            const   TString&                strName
            , const TString&                strUniqueId
            , const tCQCMedia::EMediaTypes  eType
        );

        TMediaDBBase(const TMediaDBBase&) = default;
        TMediaDBBase(TMediaDBBase&&) = default;

        TMediaDBBase& operator=(const TMediaDBBase&) = default;
        TMediaDBBase& operator=(TMediaDBBase&&) = default;


        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        )   override;

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const override;


        // -------------------------------------------------------------------
        //  Protected, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid CheckMagicVal
        (
            const   tCIDLib::TCard4         c4Line
            , const tCIDLib::TCard4         c4Exected
            ,       TBinInStream&           strmToReadFrom
            , const tCIDLib::TCh* const     pszType
        );


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c2ArtId
        //      The id of the cover art of this item, if any. If none, then it will
        //      be zero.
        //
        //  m_c2Id
        //      The id assigned to this database object (within whatever
        //      data type it belongs to, i.e. each data type has its own
        //      id space.)
        //
        //  m_c4AltId
        //      This is just a field that drivers can use for their own
        //      purposes. It's not persisted or used by this class or
        //      the in-memory database.
        //
        //  m_c4LoadOrder
        //      This is used during loading in some cases, where loaded data
        //      may be out of order, but it provides a 'track number' type
        //      of value to indicate what the natural order is. So the loader
        //      can store the natural order here and use that to correctly
        //      place each new one as it is loaded.
        //
        //  m_eType
        //      Indicates the type of media this object is for.
        //
        //  m_enctStamp
        //      Some databases will want to know if some external piece of
        //      data has changed since it was last added to the repository.
        //      This stamp can be used for that.
        //
        //  m_strAltId
        //      As with m_c4AltId, this is purely for the use of client code
        //      so that they can store some info to map back to something
        //      in the original metadata source. It's not persisted.
        //
        //  m_strName
        //      The name of the item, the meaning of which varyies with media
        //      type.
        //
        //  m_strUniqueId
        //      The unique identifier for this object. Might come from
        //      a loaded repository, or from a metadata source, or the
        //      driver might make one up. This is what is used to identify
        //      each item, collection or title set. According to the
        //      capabilities of the repo, they might be persistent across DB
        //      loads or not.
        // -------------------------------------------------------------------
        tCIDLib::TCard2         m_c2ArtId = 0;
        tCIDLib::TCard2         m_c2Id = 0;
        tCIDLib::TCard4         m_c4AltId = 0;
        tCIDLib::TCard4         m_c4LoadOrder = 0;
        tCQCMedia::EMediaTypes  m_eType;
        tCIDLib::TEncodedTime   m_enctStamp = 0;
        TString                 m_strAltId;
        TString                 m_strName;
        TString                 m_strUniqueId;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TMediaDBBase,TObject)
};


// ---------------------------------------------------------------------------
//   CLASS: TMediaCat
//  PREFIX: mcat
//
//  We don't allow media categories with the same name, so we just store the
//  name also as the unique id, which will prevent multiple cats with the same
//  name.
// ---------------------------------------------------------------------------
class CQCMEDIAEXPORT TMediaCat : public TMediaDBBase
{
    public :
        // -------------------------------------------------------------------
        //  Public static methods
        // -------------------------------------------------------------------
        static tCIDLib::ESortComps eCompById
        (
            const   TMediaCat&              mcat1
            , const TMediaCat&              mcat2
        );

        static tCIDLib::ESortComps eCompByName
        (
            const   TMediaCat&              mcat1
            , const TMediaCat&              mcat2

        );

        static tCIDLib::ESortComps eIdKeyComp
        (
            const   tCIDLib::TCard2&        c2Id
            , const TMediaCat&              mcatComp

        );

        static const TString& strKeyUID
        (
            const   TMediaCat&              mcatSrc
        );


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TMediaCat();

        TMediaCat
        (
            const   tCQCMedia::EMediaTypes  eType
            , const TString&                strName
            , const TString&                strUniqueId
        );

        TMediaCat(const TMediaCat&) = default;
        TMediaCat(TMediaCat&&) = default;

        ~TMediaCat();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TMediaCat& operator=(const TMediaCat&) = default;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid ToXML
        (
                    TTextOutStream&         strmTar
        )   const override;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        )   final;

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const final;


    private :
        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TMediaCat,TMediaDBBase)
};


// ---------------------------------------------------------------------------
//   CLASS: TMediaImg
//  PREFIX: mimg
// ---------------------------------------------------------------------------
class CQCMEDIAEXPORT TMediaImg : public TMediaDBBase
{
    public :
        // -------------------------------------------------------------------
        //  Public static methods
        // -------------------------------------------------------------------
        static tCIDLib::ESortComps eCompById
        (
            const   TMediaImg&              mimg1
            , const TMediaImg&              mimg2
        );

        static tCIDLib::ESortComps eCompBySmlPID
        (
            const   TMediaImg&              mimg1
            , const TMediaImg&              mimg2
        );

        static tCIDLib::ESortComps eCompByLrgPID
        (
            const   TMediaImg&              mimg1
            , const TMediaImg&              mimg2
        );

        static tCIDLib::ESortComps eIdKeyComp
        (
            const   tCIDLib::TCard2&        c2Id
            , const TMediaImg&              mimgComp
        );

        static tCIDLib::ESortComps eSmlPIDKeyComp
        (
            const   TString&                strPID
            , const TMediaImg&              mimgComp
        );

        static tCIDLib::ESortComps eLrgPIDKeyComp
        (
            const   TString&                strPID
            , const TMediaImg&              mimgComp
        );

        static const TString& strKeyLrgPID
        (
            const   TMediaImg&              mimgSrc
        );

        static const TString& strKeyUID
        (
            const   TMediaImg&              mimgSrc
        );


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TMediaImg();

        TMediaImg
        (
            const   tCQCMedia::EMediaTypes  eType
            , const TString&                strArtPath
            , const TString&                strThumbArtPath
        );

        TMediaImg(const TMediaImg&) = default;
        TMediaImg(TMediaImg&&) = default;

        ~TMediaImg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TMediaImg& operator=(const TMediaImg&) = default;
        TMediaImg& operator=(TMediaImg&&) = default;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid Reset() override;

        tCIDLib::TVoid ToXML
        (
                    TTextOutStream&         strmTar
        )   const override;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TCard4 c4QueryArt
        (
                    TMemBuf&                mbufToFill
            , const tCQCMedia::ERArtTypes   eType
            , const tCIDLib::TCard4         c4AtIndex = 0
        )   const;

        tCIDLib::TCard4 c4Size
        (
            const   tCQCMedia::ERArtTypes   eType
        )   const;

        tCIDLib::TVoid ClearAllArt();

        tCIDLib::TVoid ClearArt
        (
            const   tCQCMedia::ERArtTypes   eType
        );

        const THeapBuf& mbufArt
        (
            const   tCQCMedia::ERArtTypes   eType
        )   const;

        const TString& strArtPath
        (
            const   tCQCMedia::ERArtTypes   eType
        )   const;

        const TString& strPersistentId
        (
            const   tCQCMedia::ERArtTypes   eType
        )   const;

        tCIDLib::TVoid SetArt
        (
            const   tCIDLib::TCard4         c4BufSz
            , const TMemBuf&                mbufArt
            , const tCQCMedia::ERArtTypes   eType
        );

        tCIDLib::TVoid SetArt
        (
                    TBinInStream&           strmSrc
            , const tCIDLib::TCard4         c4BufSz
            , const tCQCMedia::ERArtTypes   eType
        );

        tCIDLib::TVoid SetArtPath
        (
            const   TString&                strToSet
            , const tCQCMedia::ERArtTypes   eType
        );

        tCIDLib::TVoid SetPersistentId
        (
            const   TString&                strToSet
            , const tCQCMedia::ERArtTypes   eType
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        )   final;

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const final;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4Size
        //  m_c4ThumbSize
        //      The size of the art. It's zero until this one is faulted in.
        //
        //  m_mbufArt
        //  m_mbufThumbArt
        //      The loaded art. If m_c4Size is zero, then the art hasn't been
        //      faulted in yet.
        //
        //  m_strArtPath
        //      The path to the image art. Might not be used by all
        //      repositories, since the id might be used as the mapping
        //      mechanism.
        //
        //  m_strPersistentIdLrg
        //  m_strPersistentIdPos
        //  m_strPersistentIdSml
        //      This is the unique, persistent id for the art in this image object.
        //      We need one per image type, since they are all potentially different
        //      images, in any given repo.
        //
        //      NOTE that this is different from the unique id that all of these
        //      classes have, via the base class above, in that the unique id is
        //      more for mapping back to the original repo if needed, i.e. to get
        //      info from the repo source. This id is for use by clients to cache
        //      cover art mostly.
        //
        //  m_strPosterArtPath
        //      An optional path to the poster art if the repo supports it.
        //      This is never cached. It's only loaded upon request.
        //
        //  m_strThumbArtPath
        //      An optional path to the thumb size art. If not provided, then
        //      generally the main art is scaled down on the fly to create
        //      the thumb.
        // -------------------------------------------------------------------
        tCIDLib::TCard4 m_c4Size;
        tCIDLib::TCard4 m_c4ThumbSize;
        THeapBuf        m_mbufArt;
        THeapBuf        m_mbufThumbArt;
        TString         m_strArtPath;
        TString         m_strPersistentIdLrg;
        TString         m_strPersistentIdPos;
        TString         m_strPersistentIdSml;
        TString         m_strPosterArtPath;
        TString         m_strThumbArtPath;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TMediaImg,TMediaDBBase)
};



// We need a list of media categories and media images
namespace tCQCMedia
{
    using TCatList = TRefVector<const TMediaCat>;
    using TNCCatList = TRefVector<TMediaCat>;
}



// ---------------------------------------------------------------------------
//   CLASS: TMediaItem
//  PREFIX: mitem
// ---------------------------------------------------------------------------
class CQCMEDIAEXPORT TMediaItem : public TMediaDBBase
{
    public :
        // -------------------------------------------------------------------
        //  Public static methods
        // -------------------------------------------------------------------
        static tCIDLib::ESortComps eCompById
        (
            const   TMediaItem&             mitem1
            , const TMediaItem&             mitem2
        );

        static tCIDLib::ESortComps eCompByName
        (
            const   TMediaItem&             mitem1
            , const TMediaItem&             mitem2
        );

        static tCIDLib::ESortComps eIdKeyComp
        (
            const   tCIDLib::TCard2&        c2Id
            , const TMediaItem&             mitemComp
        );

        static const TString& strUIDAccess
        (
            const   TMediaItem&             mitemSrc
        );


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TMediaItem
        (
            const   tCQCMedia::EMediaTypes  eType = tCQCMedia::EMediaTypes::Count
        );

        TMediaItem
        (
            const   TString&                strName
            , const TString&                strUniqueId
            , const TString&                strLocInfo
            , const tCQCMedia::EMediaTypes  eType
        );

        TMediaItem(const TMediaItem&) = default;
        TMediaItem(TMediaItem&&) = default;

        ~TMediaItem();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TMediaItem& operator=(const TMediaItem&) = default;
        TMediaItem& operator=(TMediaItem&&) = default;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid Reset() override;

        tCIDLib::TVoid ToXML
        (
                    TTextOutStream&         strmTar
        )   const override;


        // -------------------------------------------------------------------
        //  Public, non-virual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bAddCategory
        (
            const   tCIDLib::TCard2         c2Id
        );

        [[nodiscard]] tCIDLib::TCard4 c4BitDepth() const;

        tCIDLib::TCard4 c4BitDepth
        (
            const   tCIDLib::TCard4         c4ToSet
        );

        [[nodiscard]] tCIDLib::TCard4 c4BitRate() const;

        tCIDLib::TCard4 c4BitRate
        (
            const   tCIDLib::TCard4         c4ToSet
        );

        [[nodiscard]] tCIDLib::TCard4 c4Channels() const;

        tCIDLib::TCard4 c4Channels
        (
            const   tCIDLib::TCard4         c4ToSet
        );

        [[nodiscard]] tCIDLib::TCard4 c4Duration() const;

        tCIDLib::TCard4 c4Duration
        (
            const   tCIDLib::TCard4         c4ToSet
        );

        tCIDLib::TCard4 c4QueryCats
        (
                    tCQCMedia::TIdList&     fcolTar
            , const tCIDLib::TBoolean       bAccume = kCIDLib::True
        )   const;

        [[nodiscard]] tCIDLib::TCard4 c4Rating() const;

        tCIDLib::TCard4 c4Rating
        (
            const   tCIDLib::TCard4         c4ToSet
        );

        [[nodiscard]] tCIDLib::TCard4 c4SampleRate() const;

        tCIDLib::TCard4 c4SampleRate
        (
            const   tCIDLib::TCard4         c4ToSet
        );

        [[nodiscard]] tCIDLib::TCard4 c4Year() const;

        tCIDLib::TCard4 c4Year
        (
            const   tCIDLib::TCard4         c4ToSet
        );

        tCIDLib::TVoid ClearCats();

        [[nodiscard]] const TString& strArtist() const;

        const TString& strArtist
        (
            const   TString&                strToSet
        );

        [[nodiscard]] const TString& strLocInfo() const;

        const TString& strLocInfo
        (
            const   TString&                strToSet
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        )   final;

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const final;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4Rating
        //      This is not persisted at this level. It's just to help out
        //      item oriented repos. It will be aggregated up to higher
        //      layers.
        //
        //  m_c4BitDepth
        //  m_c4BitRate
        //      The bit depth and rate of the samples per channel. Depth is
        //      always a multiple of 8 so we only accept 8, 16, 24 or 32,
        //      and as a field it will be exposed as an enumerated field.
        //
        //  m_c4Channels
        //      The number of audio channels this track has, if applicable. If
        //      not known it's set to 2 on the assumption that that won't be
        //      dangerous at worst and probably will work. We don't set any
        //      real limit on it, since who knows what the future will bring.
        //
        //  m_c4Duration
        //      The length of the track/chapter in seconds, if applicable.
        //
        //  m_c4SampleRate
        //      The number of samples per second of the data, i.e. 44100,
        //      48000, 88200, 96000. Though there are in fact only a small
        //      number of rates in real use, it's difficult to actually be
        //      sure that a fixed set will be practical, so we don't try to
        //      do so. It's just a number.
        //
        //  m_c4Year
        //      This is not persisted at this level. It's just to help out
        //      item oriented repos. It will be aggregated up to higher
        //      layers.
        //
        //  m_fcolCatIds
        //      For those repos that get category info at the per-item
        //      level they can store it on the items and after the load
        //      the finalization phase will aggregate that info up to the
        //      collections. This is not persisted, it's just convenience
        //      for loading purposes.
        //
        //  m_strArtist
        //  m_strArtistFN
        //  m_strArtistLN
        //      It's possible to have a different artist for each track, in
        //      'best of' type CDs. We store one at the colleciton level for
        //      sorting, and this one, if present, is for when they ask for
        //      track info, so that the actual artist can be shown for each
        //      track. The caller provides us with first/last name, since
        //      we need that for sorting. And we create a full name and store
        //      that for display purposes.
        //
        //  m_strLocInfo
        //      In file based systems, there is a need to keep location info
        //      for each track.
        // -------------------------------------------------------------------
        tCIDLib::TCard4     m_c4BitDepth;
        tCIDLib::TCard4     m_c4BitRate;
        tCIDLib::TCard4     m_c4Channels;
        tCIDLib::TCard4     m_c4Duration;
        tCIDLib::TCard4     m_c4Rating;
        tCIDLib::TCard4     m_c4SampleRate;
        tCIDLib::TCard4     m_c4Year;
        tCQCMedia::TIdList  m_fcolCatIds;
        TString             m_strArtist;
        TString             m_strLocInfo;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TMediaItem,TMediaDBBase)
};



// ---------------------------------------------------------------------------
//   CLASS: TMediaCollect
//  PREFIX: mcol
// ---------------------------------------------------------------------------
class CQCMEDIAEXPORT TMediaCollect : public TMediaDBBase
{
    public :
        // -------------------------------------------------------------------
        //  Class types
        // -------------------------------------------------------------------
        using TItemList = TRefVector<TMediaItem>;


        // -------------------------------------------------------------------
        //  Public static methods
        // -------------------------------------------------------------------
        static tCIDLib::ESortComps eCompById
        (
            const   TMediaCollect&          mcol1
            , const TMediaCollect&          mcol2
        );

        static tCIDLib::ESortComps eCompByName
        (
            const   TMediaCollect&          mcol1
            , const TMediaCollect&          mcol2
        );

        static tCIDLib::ESortComps eIdKeyComp
        (
            const   tCIDLib::TCard2&        c2Id
            , const TMediaCollect&          mcolComp
        );

        static const TString& strUIDAccess
        (
            const   TMediaCollect&          mcolSrc
        );


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TMediaCollect
        (
            const   tCQCMedia::EMediaTypes  eType = tCQCMedia::EMediaTypes::Count
        );

        TMediaCollect
        (
            const   TString&                strName
            , const TString&                strUniqueId
            , const tCIDLib::TCard4         c4Year
            , const tCQCMedia::ELocTypes    eLocType
            , const tCQCMedia::EMediaTypes  eType
        );

        TMediaCollect(const TMediaCollect&) = default;
        TMediaCollect(TMediaCollect&&) = default;

        ~TMediaCollect();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TMediaCollect& operator=(const TMediaCollect&) = default;
        TMediaCollect& operator=(TMediaCollect&&) = default;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid Reset() override;

        tCIDLib::TVoid ToXML
        (
                    TTextOutStream&         strmTar
        )   const override;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid AddCatsToList
        (
                    tCQCMedia::TIdList&     fcolToFill
        )   const;

        tCIDLib::TBoolean bAddCategory
        (
            const   tCIDLib::TCard2         c2CatId
        );

        tCIDLib::TBoolean bAddCategory
        (
            const   TMediaCat&              mcatToAdd
        );

        tCIDLib::TBoolean bAddItem
        (
            const   TMediaItem&             mitemToAdd
            , const tCIDLib::TCard4         c4At = kCIDLib::c4MaxCard
        );

        [[nodiscard]] tCIDLib::TBoolean bAnamorphic() const;

        tCIDLib::TBoolean bAnamorphic
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TBoolean bContainsItem
        (
            const   tCIDLib::TCard2         c2ItemId
            ,       tCIDLib::TCard2&        c2ItemInd
        )   const;

        [[nodiscard]] tCIDLib::TBoolean bIsPlayList() const;

        tCIDLib::TBoolean bIsPlayList
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TBoolean bIsInCat
        (
            const   tCIDLib::TCard2         c2CatId
        )   const;

        tCIDLib::TCard2 c2CatIdAt
        (
            const   tCIDLib::TCard4         c4At
        )   const;

        tCIDLib::TCard2 c2ItemIdAt
        (
            const   tCIDLib::TCard4         c4At
        )   const;

        [[nodiscard]] tCIDLib::TCard4 c4Duration() const;

        tCIDLib::TCard4 c4Duration
        (
            const   tCIDLib::TCard4         c4ToSet
        );

        [[nodiscard]] tCIDLib::TCard4 c4CatCount() const;

        [[nodiscard]] tCIDLib::TCard4 c4ItemCount() const;

        [[nodiscard]] tCIDLib::TCard4 c4Year() const;

        tCIDLib::TCard4 c4Year
        (
            const   tCIDLib::TCard4         c4ToSet
        );

        tCIDLib::TVoid ClearCategories();

        tCIDLib::TVoid ClearItems();

        [[nodiscard]] tCQCMedia::ELocTypes eLocType() const;

        tCIDLib::TVoid Finalize
        (
                    TMediaDB&               mdbOwner
        );

        const TMediaItem& mitemAt
        (
            const   TMediaDB&               mdbSrc
            , const tCIDLib::TCard4         c4At
        )   const;

        tCIDLib::TVoid RemoveCategory
        (
            const   tCIDLib::TCard2         c2ToRemove
        );

        tCIDLib::TVoid RemoveItem
        (
            const   tCIDLib::TCard2         c2ToRemove
        );

        const TString& strAltInfo() const;

        const TString& strAltInfo
        (
            const   TString&                strToSet
        );

        const TString& strArtist() const;

        const TString& strArtist
        (
            const   TString&                strToSet
        );

        const TString& strAspectRatio() const;

        const TString& strAspectRatio
        (
            const   TString&                strToSet
        );

        const TString& strASIN() const;

        const TString& strASIN
        (
            const   TString&                strToSet
        );

        const TString& strCast() const;

        const TString& strCast
        (
            const   TString&                strToSet
        );

        const TString& strDescr() const;

        const TString& strDescr
        (
            const   TString&                strToSet
        );

        const TString& strLabel() const;

        const TString& strLabel
        (
            const   TString&                strToSet
        );

        const TString& strLocInfo() const;

        const TString& strLocInfo
        (
            const   TString&                strToSet
        );

        const TString& strLeadActor() const;

        const TString& strLeadActor
        (
            const   TString&                strToSet
        );

        const TString& strMediaFormat() const;

        const TString& strMediaFormat
        (
            const   TString&                strToSet
        );

        const TString& strPosterArt() const;

        const TString& strPosterArt
        (
            const   TString&                strToSet
        );

        const TString& strRating() const;

        const TString& strRating
        (
            const   TString&                strToSet
        );

        const TString& strUPC() const;

        const TString& strUPC
        (
            const   TString&                strToSet
        );

        tCIDLib::TVoid SetCatIds
        (
            const   tCQCMedia::TIdList&     fcolIds
        );

        tCIDLib::TVoid SetItemIds
        (
            const   tCQCMedia::TIdList&     fcolIds
        );

        tCIDLib::TVoid SetLocation
        (
            const   tCQCMedia::ELocTypes    eLocType
            , const TString&                strLocInfo
        );

        tCIDLib::TVoid SwapItems
        (
            const   tCIDLib::TCard4         c4At1
            , const tCIDLib::TCard4         c4At2
        );


    protected :
        // -------------------------------------------------------------------
        //  Declare our friends. We trust the database class and the title set
        //  class to directl manipulate our items, i.e. to call the NC item
        //  access method below.
        // -------------------------------------------------------------------
        friend class TMediaDB;
        friend class TMediaTitleSet;


        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        )   final;

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const final;


        // -------------------------------------------------------------------
        //  Protected, non-virtual methods
        // -------------------------------------------------------------------
        TMediaItem& mitemAtNC
        (
                    TMediaDB&               mdbSrc
            , const tCIDLib::TCard4         c4At
        );


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bAnamorphic
        //      Used in conjunction with the m_strAspectRatio field to correctly
        //      display movie content.
        //
        //  m_bIsPlayList
        //      Used to mark playlists as such, so that we don't depend on any
        //      user modifiable attribute to know which ones are play lists.
        //
        //  m_c4Duration
        //      The running time of the contained media in seconds
        //
        //  m_c4Year
        //      The year that this media was created.
        //
        //  m_eLocType
        //      Indicates how to interpret the location information for
        //      this collection. It either indicates that we have a collection
        //      level location (music or movie in a changer or file based
        //      movie), or we have per-track file based location info.
        //
        //  m_fcolCatIds
        //      The list of ids into the category list for any categories
        //      we are part of. This is persisted.
        //
        //  m_fcolItemIds
        //      The list of ids into the item list for the items that make
        //      up this collection. This is persisted.
        //
        //  m_strAltInfo
        //      Just an open ended storage string that a driver can use to
        //      store extra info, so that they don't need to create a derived
        //      class just to store an extra string.
        //
        //  m_strArtist
        //      The name of the artist (for music), director (for movies), or
        //      photographer (for pictures) of this title. This may be
        //      aggregated from underlying item info.
        //
        //  m_strAspectRatio
        //      The AR of the movie, if available.
        //
        //  m_strASIN
        //      If we get it, this is the ASIN code for the collection.
        //
        //  m_strCast
        //      The cast information, if available.
        //
        //  m_strDescr
        //      The descriptive text of this disc.
        //
        //  m_strLabel
        //      The name of the label or studio that released this set.
        //
        //  m_strLeadActor
        //      The lead actor, either from an explicit indication of such
        //      or by taking the first actor from the cast list.
        //
        //  m_strLocInfo
        //      Some location types are stored at the collection level, and
        //      some at the item level. If this one is a collection level
        //      style, the info is stored here, based on m_eLocType.
        //
        //  m_strMediaFmt
        //      This is an open ended field that users can store information
        //      about the format of the media, e.g. DVD, blu-ray and that
        //      sort of thing. They often need this to know how to play back
        //      a selected collection.
        //
        //  m_strPosterArt
        //      A path to poster art for this collection, if any.
        //
        //  m_strRating
        //      The rating description of the movie.
        //
        //  m_strUPC
        //      If we get it, this is the UPC code for the collection.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bAnamorphic;
        tCIDLib::TBoolean       m_bIsPlayList;
        tCIDLib::TCard4         m_c4Duration;
        tCIDLib::TCard4         m_c4Year;
        tCQCMedia::ELocTypes    m_eLocType;
        tCQCMedia::TIdList      m_fcolCatIds;
        tCQCMedia::TIdList      m_fcolItemIds;
        TString                 m_strAltInfo;
        TString                 m_strArtist;
        TString                 m_strAspectRatio;
        TString                 m_strASIN;
        TString                 m_strCast;
        TString                 m_strDescr;
        TString                 m_strLabel;
        TString                 m_strLeadActor;
        TString                 m_strLocInfo;
        TString                 m_strMediaFmt;
        TString                 m_strPosterArt;
        TString                 m_strRating;
        TString                 m_strUPC;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TMediaCollect,TMediaDBBase)
};



// ---------------------------------------------------------------------------
//   CLASS: TMediaTitleSet
//  PREFIX: mts
// ---------------------------------------------------------------------------
class CQCMEDIAEXPORT TMediaTitleSet : public TMediaDBBase
{
    public :
        // -------------------------------------------------------------------
        //  Public class types
        // -------------------------------------------------------------------
        using TColRefList = TRefVector<TMediaCollect>;


        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static tCIDLib::ESortComps eCompByArtist
        (
            const   TMediaTitleSet&         mts1
            , const TMediaTitleSet&         mts2
        );

        static tCIDLib::ESortComps eCompByBitDepth
        (
            const   TMediaTitleSet&         mts1
            , const TMediaTitleSet&         mts2
        );

        static tCIDLib::ESortComps eCompByBitRate
        (
            const   TMediaTitleSet&         mts1
            , const TMediaTitleSet&         mts2
        );

        static tCIDLib::ESortComps eCompByChannels
        (
            const   TMediaTitleSet&         mts1
            , const TMediaTitleSet&         mts2
        );

        static tCIDLib::ESortComps eCompByDateAdded
        (
            const   TMediaTitleSet&         mts1
            , const TMediaTitleSet&         mts2
        );

        static tCIDLib::ESortComps eCompById
        (
            const   TMediaTitleSet&         mts1
            , const TMediaTitleSet&         mts2
        );

        static tCIDLib::ESortComps eCompBySampleRate
        (
            const   TMediaTitleSet&         mts1
            , const TMediaTitleSet&         mts2
        );

        static tCIDLib::ESortComps eCompBySeqNum
        (
            const   TMediaTitleSet&         mts1
            , const TMediaTitleSet&         mts2
        );

        static tCIDLib::ESortComps eCompByTitle
        (
            const   TMediaTitleSet&         mts1
            , const TMediaTitleSet&         mts2
        );

        static tCIDLib::ESortComps eCompBySortTitle
        (
            const   TMediaTitleSet&         mts1
            , const TMediaTitleSet&         mts2

        );

        static tCIDLib::ESortComps eCompByUserRating
        (
            const   TMediaTitleSet&         mts1
            , const TMediaTitleSet&         mts2
        );

        static tCIDLib::ESortComps eCompByYear
        (
            const   TMediaTitleSet&         mts1
            , const TMediaTitleSet&         mts2
        );

                static tCIDLib::ESortComps eIdKeyComp
        (
            const   tCIDLib::TCard2&        c2Id
            , const TMediaTitleSet&         mts2
        );

        static const TString& strKeyAccess
        (
            const   TMediaTitleSet&         mtsSrc
        );

        static const TString& strUIDAccess
        (
            const   TMediaTitleSet&         mtsSrc
        );


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TMediaTitleSet
        (
            const   tCQCMedia::EMediaTypes  eType = tCQCMedia::EMediaTypes::Count
        );

        TMediaTitleSet
        (
            const   TString&                strName
            , const TString&                strUniqueId
            , const TString&                strSortTitle
            , const tCQCMedia::EMediaTypes  eType
        );

        TMediaTitleSet(const TMediaTitleSet&) = default;
        TMediaTitleSet(TMediaTitleSet&&) = default;

        ~TMediaTitleSet();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TMediaTitleSet& operator=(const TMediaTitleSet&) = default;
        TMediaTitleSet& operator=(TMediaTitleSet&&) = default;


        // -------------------------------------------------------------------
        //  Public, inhierted methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid Reset() override;

        tCIDLib::TVoid ToXML
        (
                    TTextOutStream&         strmTar
        )   const override;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bAddCollect
        (
            const   TMediaCollect&          mcolToAdd
        );

        tCIDLib::TBoolean bContainsCol
        (
            const   tCIDLib::TCard2         c2ColId
            ,       tCIDLib::TCard2&        c2Index
        )   const;

        [[nodiscard]] tCIDLib::TBoolean bIsPlayList() const;

        tCIDLib::TBoolean bRemoveColById
        (
            const   tCIDLib::TCard2         c2ColId
            , const tCIDLib::TBoolean       bThrowIfNot
        );

        tCIDLib::TCard2 c2ColIdAt
        (
            const   tCIDLib::TCard4         c4At
        )   const;

        [[nodiscard]] tCIDLib::TCard4 c4BitDepth() const noexcept
        {
            return m_c4BitDepth;
        }

        [[nodiscard]] tCIDLib::TCard4 c4BitRate() const noexcept
        {
            return m_c4BitRate;
        }

        [[nodiscard]] tCIDLib::TCard4 c4Channels() const noexcept
        {
            return m_c4Channels;
        }

        [[nodiscard]] tCIDLib::TCard4 c4ColCount() const;

        [[nodiscard]] tCIDLib::TCard4 c4SampleRate() const noexcept
        {
            return m_c4SampleRate;
        }

        [[nodiscard]] tCIDLib::TCard4 c4SeqNum() const noexcept
        {
            return m_c4SeqNum;
        }

        tCIDLib::TCard4 c4SeqNum
        (
            const   tCIDLib::TCard4         c4ToSet
        );

        [[nodiscard]] tCIDLib::TCard4 c4UserRating() const noexcept
        {
            return m_c4UserRating;
        }

        tCIDLib::TCard4 c4UserRating
        (
            const   tCIDLib::TCard4         c4ToSet
        );

        [[nodiscard]] tCIDLib::TCard4 c4Year() const noexcept
        {
            return m_c4Year;
        }

        tCIDLib::TCh chSortChar
        (
            const   tCQCMedia::ESortOrders  eOrder
        )   const;

        tCIDLib::TVoid ClearCollects();

        [[nodiscard]] tCIDLib::TEncodedTime enctAdded() const;

        tCIDLib::TEncodedTime enctAdded
        (
            const   tCIDLib::TEncodedTime   enctToSet
        );

        tCIDLib::TVoid Finalize
        (
                    TMediaDB&               mdbOwner
        );

        [[nodiscard]] const TMediaCollect& mcolAt
        (
            const   TMediaDB&               mdbSrc
            , const tCIDLib::TCard4         c4At
        )   const;

        tCIDLib::TVoid RemoveColAt
        (
            const   tCIDLib::TCard4         c4At
        );

        [[nodiscard]] const TString& strArtist() const;

        const TString& strArtist
        (
            const   TString&                strToSet
        );

        [[nodiscard]] const TString& strSortTitle() const;

        const TString& strSortTitle
        (
            const   TString&                strToSet
        );

        tCIDLib::TVoid SetColIds
        (
            const   tCQCMedia::TIdList&     fcolIds
        );

        tCIDLib::TVoid SwapCollects
        (
            const   tCIDLib::TCard4         c4At1
            , const tCIDLib::TCard4         c4At2
        );


    protected :
        // -------------------------------------------------------------------
        //  Declare our friends. We trust the database class to directly manipulate
        //  our collections, i.e. to call the NC collection access method below.
        // -------------------------------------------------------------------
        friend class TMediaDB;


        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        )   final;

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const final;


        // -------------------------------------------------------------------
        //  Protected, non-virtual methods
        // -------------------------------------------------------------------
        TMediaCollect& mcolAtNC
        (
                    TMediaDB&               mdbSrc
            , const tCIDLib::TCard4         c4At
        );


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bIsPlayList
        //  m_c4BitDepth
        //  m_c4BitRate
        //  m_c4Channels
        //  m_c4SampleRate
        //  m_c4Year
        //      For sorting purposes we aggregate some info from our contained
        //      collections (and their items in some cases) upon load (and in
        //      the case of our repository upon changing, removing or adding
        //      items or collections dynamically, in other repos the info never
        //      changes after load.)
        //
        //      So, in our initialize method we just find the largest/smallest
        //      etc... values we contain and set ourself to those values. So
        //      none of these are persisted.
        //
        //  m_c4SeqNum
        //      The sequence number for this collection, i.e. its sequential
        //      index as added to the repository. So the first collection is
        //      1, the next one is 2 and so on. This is not adjusted to deal
        //      with deletions, so it's an ever increasing value.
        //
        //      This is not persisted even in our own repository either since
        //      it can be deduced from the file names of the title set data files
        //      which are numbered.
        //
        //  m_c4UserRating
        //      The user can apply a rating from 1 to 10 to indicate how much
        //      they like this title. This may be aggregated from contained
        //      objects.
        //
        //  m_enctAdded
        //      The date on which this title set was added to the repository,
        //      if that info is available. Where not, it'll be set to the
        //      current date when we load.
        //
        //  m_fcolColIds
        //      The list of ids into the collection list. For each collection
        //      added to us, we store it's id, which let's us look it up
        //      again later as required.
        //
        //  m_strArtist
        //      The full artist name. This is aggregated frm contined objects
        //      and may be set to indicate various artists if they are not
        //      all the same.
        //
        //  m_strSortTitle
        //      If the metadata source provides a separate sort title (or the
        //      driver for that source creates one), it's stored here. If this
        //      is empty, then the name string (in our parent class) is used
        //      for both display and sorting.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bIsPlayList;
        tCIDLib::TCard4         m_c4BitDepth;
        tCIDLib::TCard4         m_c4BitRate;
        tCIDLib::TCard4         m_c4Channels;
        tCIDLib::TCard4         m_c4SampleRate;
        tCIDLib::TCard4         m_c4SeqNum;
        tCIDLib::TCard4         m_c4UserRating;
        tCIDLib::TEncodedTime   m_enctAdded;
        tCIDLib::TCard4         m_c4Year;
        tCQCMedia::TIdList      m_fcolColIds;
        TString                 m_strArtist;
        TString                 m_strSortTitle;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TMediaTitleSet,TMediaDBBase)
};



// ---------------------------------------------------------------------------
//   CLASS: TArtistMap
//  PREFIX: mamap
//
//  Used to create a list of artists and the title sets that they are associated with. This
//  is a dynamic list, created from existing (persistent) database info.
// ---------------------------------------------------------------------------
class CQCMEDIAEXPORT TArtistMap : public TObject
{
    public :
        // -------------------------------------------------------------------
        //  Public static types
        // -------------------------------------------------------------------
        using TSetList = TRefVector<const TMediaTitleSet>;


        // -------------------------------------------------------------------
        //  Public static methods
        // -------------------------------------------------------------------
        static tCIDLib::ESortComps eComp
        (
            const   TArtistMap&             mamap1
            , const TArtistMap&             mamap2
        );

        static tCIDLib::ESortComps eCompByKey
        (
            const   TString&                strKey
            , const TArtistMap&             mamap2
        );


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TArtistMap();

        TArtistMap
        (
            const   TString&                strArtistName
        );

        TArtistMap
        (
            const   TArtistMap&             mamapSrc
        );

        TArtistMap(TArtistMap&&) = delete;

        ~TArtistMap();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TArtistMap& operator=
        (
            const   TArtistMap&             mamapSrc
        );

        TArtistMap& operator=(TArtistMap&&) = delete;


        // -------------------------------------------------------------------
        //  Public methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid AddTitleSet
        (
            const   TMediaTitleSet* const   pmtsToAdd
        );

        const TSetList& colSets() const;

        tCIDLib::TVoid Complete();

        const TString& strArtistName() const;

        const TString& strArtistName
        (
            const   TString&                strToSet
        );


   private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_colTitleSets
        //      A collection references to title sets that this artist is assocaited
        //      with, sorted by title. It is non-adopting, since the objects pointed to
        //      belong to the database object. Absolutely worst case it needs to be
        //      64K max size, for a full set of titles all from one artist.
        //
        //  m_strArtistName
        //      THe name of the artist that this object is tracking.
        // -------------------------------------------------------------------
        TSetList    m_colTitleSets;
        TString     m_strArtistName;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TArtistMap,TObject)
};


#pragma CIDLIB_POPPACK

