//
// FILE NAME: CQCKit_ImageStoreItem.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/27/2003
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
//  Implements the pre-1.3.4 storage format for uploaded images. This is
//  only now used by the installer during upgrades. All functionality not
//  related to just reading in and accessing the data has been removed.
//
//  We have to bring our header in directly here, since it's not exposed
//  via the main facility header.
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
#include    "CQCKit_.hpp"
#include    "CQCKit_ImageStoreItemOld.hpp"



// ---------------------------------------------------------------------------
//  Magic Macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCImgStoreBaseOld,TObject)
RTTIDecls(TCQCImgStoreItemOld,TCQCImgStoreBaseOld)


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace
{
    namespace CQCKit_ImageStoreItem
    {
        // -----------------------------------------------------------------------
        //  Our current and previous format versions
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2FmtVersion     = 1;
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCImgStoreBaseOld
//  PREFIX: isi
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCImgStoreBaseOld: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCImgStoreBaseOld::TCQCImgStoreBaseOld() :

    m_bTransparent(kCIDLib::False)
    , m_c4ImageSz(0)
    , m_c4ThumbSz(0)
    , m_eType(tCQCKit::EImgTypes::None)
    , m_mbufThumb(8)
{
}

TCQCImgStoreBaseOld::TCQCImgStoreBaseOld(const TCQCImgStoreBaseOld& isiToCopy) :

    m_bTransparent(isiToCopy.m_bTransparent)
    , m_c4ImageSz(isiToCopy.m_c4ImageSz)
    , m_c4ThumbSz(isiToCopy.m_c4ThumbSz)
    , m_eType(isiToCopy.m_eType)
    , m_mbufThumb(isiToCopy.m_c4ThumbSz, isiToCopy.m_c4ThumbSz)
    , m_rgbTransClr(isiToCopy.m_rgbTransClr)
    , m_strName(isiToCopy.m_strName)
{
    m_mbufThumb.CopyIn(isiToCopy.m_mbufThumb, isiToCopy.m_c4ThumbSz);
}

TCQCImgStoreBaseOld::~TCQCImgStoreBaseOld()
{
}


// ---------------------------------------------------------------------------
//  TCQCImgStoreBaseOld: Public operators
// ---------------------------------------------------------------------------
TCQCImgStoreBaseOld&
TCQCImgStoreBaseOld::operator=(const TCQCImgStoreBaseOld& isiToAssign)
{
    if (this != &isiToAssign)
    {
        m_bTransparent  = isiToAssign.m_bTransparent;
        m_c4ImageSz     = isiToAssign.m_c4ImageSz;
        m_c4ThumbSz     = isiToAssign.m_c4ThumbSz;
        m_eType         = isiToAssign.m_eType;
        m_mbufThumb     = isiToAssign.m_mbufThumb;
        m_rgbTransClr   = isiToAssign.m_rgbTransClr;
        m_strName       = isiToAssign.m_strName;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCImgStoreBaseOld: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TCQCImgStoreBaseOld::bTransparent() const
{
    return m_bTransparent;
}

tCIDLib::TCard4 TCQCImgStoreBaseOld::c4ImageSz() const
{
    return m_c4ImageSz;
}


tCIDLib::TCard4 TCQCImgStoreBaseOld::c4ThumbSz() const
{
    return m_c4ThumbSz;
}


tCQCKit::EImgTypes TCQCImgStoreBaseOld::eType() const
{
    return m_eType;
}


const THeapBuf& TCQCImgStoreBaseOld::mbufThumb() const
{
    return m_mbufThumb;
}


const TRGBClr& TCQCImgStoreBaseOld::rgbTransClr() const
{
    return m_rgbTransClr;
}

const TString& TCQCImgStoreBaseOld::strName() const
{
    return m_strName;
}


// ---------------------------------------------------------------------------
//  TCQCImgStoreBaseOld: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCImgStoreBaseOld::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Make sure we get the start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Stream in the format version and make sure its good
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (c2FmtVersion != CQCKit_ImageStoreItem::c2FmtVersion)
    {
        facCIDLib().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCIDErrs::errcGen_UnknownFmtVersion
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(c2FmtVersion)
            , clsThis()
        );
    }

    // Looks ok, so lets read in the header stuff
    strmToReadFrom  >> m_bTransparent
                    >> m_c4ImageSz
                    >> m_c4ThumbSz
                    >> m_eType
                    >> m_rgbTransClr
                    >> m_strName;

    // There should be a frame before the thumb data
    strmToReadFrom.CheckForFrameMarker(CID_FILE, CID_LINE);
    strmToReadFrom.c4ReadBuffer(m_mbufThumb, m_c4ThumbSz);

    // And make sure we get the end marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TCQCImgStoreBaseOld::StreamTo(TBinOutStream& strmToWriteTo) const
{
    //
    //  Stream out the easy stuff first, with a start object marker at the
    //  start and a frame marker at the end, to begin the non-easy bits.
    //  We also stream out a format version to allow for future automatic
    //  upgrade of data.
    //
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCKit_ImageStoreItem::c2FmtVersion
                    << m_bTransparent
                    << m_c4ImageSz
                    << m_c4ThumbSz
                    << m_eType
                    << m_rgbTransClr
                    << m_strName
                    << tCIDLib::EStreamMarkers::Frame;

    //
    //  Write a frame marker, then dump the thumb data. We do a raw buffer
    //  write here so we can control how many bytes of the buffer get stored,
    //  since it could have been reset a number of times and the buffer could
    //  be a lot bigger than the actual image.
    //
    strmToWriteTo.c4WriteBuffer(m_mbufThumb, m_c4ThumbSz);

    // And finish off with an end marker
    strmToWriteTo   << tCIDLib::EStreamMarkers::EndObject;
}


// ---------------------------------------------------------------------------
//  TCQCImgStoreBaseOld: Protected, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TCard4 TCQCImgStoreBaseOld::c4ImageSz(const tCIDLib::TCard4 c4ToSet)
{
    m_c4ImageSz = c4ToSet;
    return m_c4ImageSz;
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCImgStoreItemOld
//  PREFIX: isi
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCImgStoreItemOld: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCImgStoreItemOld::TCQCImgStoreItemOld() :

    m_mbufImage(8)
{
}

//
//  Since the image sizes can vary substantially, we don't just copy ctor the
//  image buffer. We manually allocate to the image size, and then copy over
//  the used part of the data.
//
TCQCImgStoreItemOld::TCQCImgStoreItemOld(const TCQCImgStoreItemOld& isiToCopy) :

    TCQCImgStoreBaseOld(isiToCopy)
    , m_mbufImage(isiToCopy.c4ImageSz(), isiToCopy.c4ImageSz())
{
    // Copy over the just the bytes needed for the image
    m_mbufImage.CopyIn(isiToCopy.m_mbufImage, c4ImageSz());
}

TCQCImgStoreItemOld::~TCQCImgStoreItemOld()
{
}


// ---------------------------------------------------------------------------
//  TCQCImgStoreItemOld: Public operators
// ---------------------------------------------------------------------------
TCQCImgStoreItemOld&
TCQCImgStoreItemOld::operator=(const TCQCImgStoreItemOld& isiToAssign)
{
    if (this != &isiToAssign)
    {
        TCQCImgStoreBaseOld::operator=(isiToAssign);
        m_mbufImage.CopyIn(isiToAssign.m_mbufImage, isiToAssign.c4ImageSz());
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCImgStoreItemOld: Public, non-virtual methods
// ---------------------------------------------------------------------------
const THeapBuf& TCQCImgStoreItemOld::mbufImage() const
{
    return m_mbufImage;
}

THeapBuf& TCQCImgStoreItemOld::mbufImage()
{
    return m_mbufImage;
}


// ---------------------------------------------------------------------------
//  TCQCImgStoreItemOld: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCImgStoreItemOld::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Call our parent first
    TParent::StreamFrom(strmToReadFrom);

    // And our stuff should start with a start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    //
    //  Stream in the format version and make sure its good. This lets us change
    //  our format separately from the base, and vice versa.
    //
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (c2FmtVersion != CQCKit_ImageStoreItem::c2FmtVersion)
    {
        facCIDLib().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCIDErrs::errcGen_UnknownFmtVersion
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(c2FmtVersion)
            , clsThis()
        );
    }

    // Then there should be a frame marker and our image data
    strmToReadFrom.CheckForFrameMarker(CID_FILE, CID_LINE);
    strmToReadFrom.c4ReadBuffer(m_mbufImage, c4ImageSz());

    // And make sure we get the end marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TCQCImgStoreItemOld::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Call our parent first
    TParent::StreamTo(strmToWriteTo);

    //
    //  Stream out the easy stuff first, with a start object marker at the
    //  start and a frame marker at the end, to begin the non-easy bits.
    //  We also stream out a format version to allow for future automatic
    //  upgrade of data.
    //
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCKit_ImageStoreItem::c2FmtVersion
                    // Nothing yet
                    << tCIDLib::EStreamMarkers::Frame;

    // We write manually to avoid writing allocated but unused bytes
    strmToWriteTo.c4WriteBuffer(m_mbufImage, c4ImageSz());

    // And finish off with an end marker
    strmToWriteTo   << tCIDLib::EStreamMarkers::EndObject;
}


