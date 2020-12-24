//
// FILE NAME: CQCRemBrws_ImageStoreItem.cpp
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
//  Just for conversion purpose now, see the header comments.
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
#include    "CQCRemBrws_.hpp"



// ---------------------------------------------------------------------------
//  Magic Macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCImgStoreBase,TObject)
RTTIDecls(TCQCImgStoreItem,TCQCImgStoreBase)


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace CQCKit_ImageStoreItem
{
    namespace
    {
        // -----------------------------------------------------------------------
        //  The format version for the base class
        //
        //  Version 2 -
        //      We move the serial number from being a runtime thing to being a
        //      persistent thing, so we have to fault it in for older versions.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2BaseFmtVersion     = 2;

        // -----------------------------------------------------------------------
        //  The format version for the full class
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2FullFmtVersion     = 1;


        // -----------------------------------------------------------------------
        //  The maximum image size we'll store (92MB). This is grotesqely large for
        //  a program like CQC, even if uncompressed.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard4   c4MaxImgSize        = 92 * (1024 * 1024);
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCImgStoreBase
//  PREFIX: isi
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCImgStoreBase: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCImgStoreBase::TCQCImgStoreBase() :

    m_c4SerialNum(facCIDCrypto().c4GetRandom(kCIDLib::True))
    , m_eOrgDepth(tCIDImage::EBitDepths::One)
    , m_eOrgFmt(tCIDImage::EPixFmts::GrayScale)
    , m_c4ImageSz(0)
    , m_c4ThumbSz(0)
    , m_mbufThumb(8)
{
}

TCQCImgStoreBase::~TCQCImgStoreBase()
{
}


// ---------------------------------------------------------------------------
//  TCQCImgStoreBase: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCImgStoreBase::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Make sure we get the start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Stream in the format version and make sure its good
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion
    ||  (c2FmtVersion > CQCKit_ImageStoreItem::c2BaseFmtVersion))
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
    strmToReadFrom  >> m_eOrgDepth
                    >> m_eOrgFmt
                    >> m_c4ImageSz
                    >> m_c4ThumbSz
                    >> m_strName
                    >> m_szOrgImg;

    // If a V1 file, we have to fault in the serial number, else read it in
    if (c2FmtVersion == 1)
        m_c4SerialNum = facCIDCrypto().c4GetRandom(kCIDLib::True);
    else
        strmToReadFrom >> m_c4SerialNum;

    // There should be a frame before the thumb data
    strmToReadFrom.CheckForFrameMarker(CID_FILE, CID_LINE);
    strmToReadFrom.c4ReadBuffer(m_mbufThumb, m_c4ThumbSz);

    // And make sure we get the end marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TCQCImgStoreBase::StreamTo(TBinOutStream& strmToWriteTo) const
{
    //
    //  Stream out the easy stuff first, with a start object marker at the
    //  start and a frame marker at the end, to begin the non-easy bits.
    //  We also stream out a format version to allow for future automatic
    //  upgrade of data.
    //
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCKit_ImageStoreItem::c2BaseFmtVersion
                    << m_eOrgDepth
                    << m_eOrgFmt
                    << m_c4ImageSz
                    << m_c4ThumbSz
                    << m_strName
                    << m_szOrgImg

                    // V2 stuff
                    << m_c4SerialNum
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
//   CLASS: TCQCImgStoreItem
//  PREFIX: isi
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCImgStoreItem: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCImgStoreItem::TCQCImgStoreItem() :

    m_mbufImage(8, 8 * (1024 * 1024))
{
}

TCQCImgStoreItem::~TCQCImgStoreItem()
{
}


// ---------------------------------------------------------------------------
//  TCQCImgStoreItem: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCImgStoreItem::StreamFrom(TBinInStream& strmToReadFrom)
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
    if (!c2FmtVersion || (c2FmtVersion != CQCKit_ImageStoreItem::c2FullFmtVersion))
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
    strmToReadFrom.c4ReadBuffer(m_mbufImage, m_c4ImageSz);

    // And make sure we get the end marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TCQCImgStoreItem::StreamTo(TBinOutStream& strmToWriteTo) const
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
                    << CQCKit_ImageStoreItem::c2FullFmtVersion
                    // Nothing yet
                    << tCIDLib::EStreamMarkers::Frame;

    // We write manually to avoid writing allocated but unused bytes
    strmToWriteTo.c4WriteBuffer(m_mbufImage, m_c4ImageSz);

    // And finish off with an end marker
    strmToWriteTo   << tCIDLib::EStreamMarkers::EndObject;
}


