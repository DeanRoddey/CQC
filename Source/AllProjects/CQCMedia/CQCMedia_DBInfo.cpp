//
// FILE NAME: CQCMedia_DBInfo.cpp
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
//  This is the implementation file for the classes that make up the standard
//  in-memory database that all media drivers must maintain (well, they can
//  maintain their own but it still must model this scheme.)
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCMedia_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TMediaDBBase,TObject)
RTTIDecls(TMediaCat,TMediaDBBase)
RTTIDecls(TMediaImg,TMediaDBBase)
RTTIDecls(TMediaItem,TMediaDBBase)
RTTIDecls(TMediaCollect,TMediaDBBase)
RTTIDecls(TMediaTitleSet,TMediaDBBase)
RTTIDecls(TArtistMap,TObject)



// ---------------------------------------------------------------------------
//  Local types and constants
// ---------------------------------------------------------------------------
namespace
{
    namespace CQCMedia_DBInfo
    {

        // -----------------------------------------------------------------------
        //  The base class persistent foramt version
        //
        //  Version 2 -
        //      We added a time stamp field
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2BaseFmtVersion    = 2;


        // -----------------------------------------------------------------------
        //  The category persistent format version
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2CatFmtVersion     = 1;


        // -----------------------------------------------------------------------
        //  The image persistent format version
        //
        //  Version 2 -
        //      We added a poster art path and new persistent ids so that we can
        //      have one per image type.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2ImageFmtVersion   = 2;


        // -----------------------------------------------------------------------
        //  The item persistent format version
        //
        //  Version 2 -
        //      We dumped separate first/last name for the lead actor and artist
        //      fields.
        //
        //  Version 3 -
        //      Added bit depth and sample rate field, and removed the year, which
        //      we just do at the collection level now.
        //
        //  Version 4 -
        //      Added bit rate, which was supposed to have been in V3 above but
        //      now it's too late since data has been changed so we have to do
        //      V4.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2ItemFmtVersion    = 4;


        // -----------------------------------------------------------------------
        //  The collection persistent format version
        //
        //  Version 2 -
        //      We dumped separate first/last name for the lead actor and artist
        //      fields.
        //
        //  Version 3 -
        //      We added a string field to store an open ended 'media format'
        //
        //  Version 4 -
        //      Add a string field to hold a 'poster art path' for displaying
        //      poster art.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2ColFmtVersion     = 4;


        // -----------------------------------------------------------------------
        //  The title set persistent format vesrion
        //
        //  Version 2 -
        //      We dumped the sort title and the separate artist first/last name,
        //      so we need to convert over to the single artist name and drop the
        //      other stuff.
        //
        //  Version 3 -
        //      Oh well, we were forced to bring back the sort title.
        //
        //  Version 4 -
        //      We stopped storing the year and now just aggregate it from the
        //      contained collections/items, as we do with other data. And we
        //      added 'date added' and 'user rating' values.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2TitleSetFmtVersion = 4;


        // -----------------------------------------------------------------------
        //  The repo change persistent format vesrion
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2RepoChFmtVersion = 1;


        // -----------------------------------------------------------------------
        //  Some magic values we write at the start of our objects to be able to
        //  recognize later whether it's a valid file or not (when used with our
        //  own repository where these are persisted.)
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard4   c4MagicCatVal       = 0xABCD1234;
        constexpr tCIDLib::TCard4   c4MagicColVal       = 0xDF812AC8;
        constexpr tCIDLib::TCard4   c4MagicImgVal       = 0x8FA091FA;
        constexpr tCIDLib::TCard4   c4MagicItemVal      = 0x09F663DC;
        constexpr tCIDLib::TCard4   c4MagicTitleVal     = 0x884400AA;
    }
}




// ---------------------------------------------------------------------------
//   CLASS: TMediaDBBase
//  PREFIX: mddb
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TMediaDBBase: Destructor
// ---------------------------------------------------------------------------
TMediaDBBase::~TMediaDBBase()
{
}

// ---------------------------------------------------------------------------
//  TMediaDBBase: Public virtaul methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TMediaDBBase::Reset()
{
    m_c2ArtId = 0;
    m_c2Id = 0;
    m_c4AltId = 0;
    m_c4LoadOrder = 0;
    m_enctStamp = 0;
    m_strAltId.Clear();
    m_strName.Clear();
    m_strUniqueId.Clear();
}


//
//  We format out our info at this level. It's all done as attributes of the
//  the derived class' XML element. We care nothing about formatting here since
//  this stuff is for machine consumption and we want it as compressed as
//  possible.
//
tCIDLib::TVoid TMediaDBBase::ToXML(TTextOutStream& strmTar) const
{
    strmTar << L" Id=\"" << m_c2Id << L"\"";

    if (m_c2ArtId)
        strmTar << L" ArtId=\"" << m_c2ArtId << L"\"";

    if (m_c4AltId)
        strmTar << L" AltId=\"" << m_c4AltId << L"\"";

    if (m_c4LoadOrder)
        strmTar << L" LOrd=\"" << m_c4LoadOrder << L"\"";

    if (m_enctStamp)
        strmTar << L" Stamp=\"0x" << TCardinal64(m_enctStamp, tCIDLib::ERadices::Hex)
                << L"\"";

    // These require escapement since they could be any text
    if (!m_strAltId.bIsEmpty())
        facCIDXML().FormatAttr(strmTar, L"AltId", m_strAltId, kCIDLib::True);

    if (!m_strName.bIsEmpty())
        facCIDXML().FormatAttr(strmTar, L"Name", m_strName, kCIDLib::True);

    if (!m_strUniqueId.bIsEmpty())
        facCIDXML().FormatAttr(strmTar, L"UID", m_strUniqueId, kCIDLib::True);
}


// ---------------------------------------------------------------------------
//  TMediaDBBase: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TCard2 TMediaDBBase::c2ArtId(const tCIDLib::TCard2 c2ToSet)
{
    m_c2ArtId = c2ToSet;
    return m_c2ArtId;
}


tCIDLib::TCard2 TMediaDBBase::c2Id(const tCIDLib::TCard2 c2ToSet)
{
    m_c2Id = c2ToSet;
    return m_c2Id;
}


// Get/set the alt value field
tCIDLib::TCard4 TMediaDBBase::c4AltId() const
{
    return m_c4AltId;
}

tCIDLib::TCard4 TMediaDBBase::c4AltId(const tCIDLib::TCard4 c4ToSet)
{
    m_c4AltId = c4ToSet;
    return m_c4AltId;
}


// Get/set the load order field
tCIDLib::TCard4 TMediaDBBase::c4LoadOrder() const
{
    return m_c4LoadOrder;
}

tCIDLib::TCard4 TMediaDBBase::c4LoadOrder(const tCIDLib::TCard4 c4ToSet)
{
    m_c4LoadOrder = c4ToSet;
    return m_c4LoadOrder;
}


// Reset the type and then call the virtual reset to do other reset
tCIDLib::TVoid TMediaDBBase::ResetType(const tCQCMedia::EMediaTypes eType)
{
    m_eType = eType;
    Reset();
}


//
//  Set the stamp, which is just for use by repositories to stamp
//  items relative to some external data that might change.
//
tCIDLib::TEncodedTime
TMediaDBBase::enctStamp(const tCIDLib::TEncodedTime enctToSet)
{
    return m_enctStamp;
}


// Get/set the alt id for this object
const TString& TMediaDBBase::strAltId() const
{
    return m_strAltId;
}

const TString& TMediaDBBase::strAltId(const TString& strToSet)
{
    m_strAltId = strToSet;
    return m_strAltId;
}


// Set the name of this database object
const TString& TMediaDBBase::strName(const TString& strToSet)
{
    m_strName = strToSet;
    return m_strName;
}


// Set the unique id for this database object
const TString& TMediaDBBase::strUniqueId(const TString& strToSet)
{
    m_strUniqueId = strToSet;
    return m_strUniqueId;
}


// ---------------------------------------------------------------------------
//  TMediaDBBase: Hidden constructors and operators
// ---------------------------------------------------------------------------
TMediaDBBase::TMediaDBBase(const tCQCMedia::EMediaTypes eType) :

    m_eType(eType)
{
}

TMediaDBBase::TMediaDBBase( const   TString&                strName
                            , const TString&                strUniqueId
                            , const tCQCMedia::EMediaTypes  eType) :
    m_eType(eType)
    , m_strName(strName)
    , m_strUniqueId(strUniqueId)
{
}


// ---------------------------------------------------------------------------
//  TMediaDBBase: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TMediaDBBase::StreamFrom(TBinInStream& strmToReadFrom)
{
    strmToReadFrom.CheckForFrameMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCMedia_DBInfo::c2BaseFmtVersion))
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

    strmToReadFrom  >> m_eType
                    >> m_strName
                    >> m_c2Id
                    >> m_c2ArtId
                    >> m_strUniqueId;

    // If V1, then default the time stamp, else read it in
    if (c2FmtVersion == 1)
        m_enctStamp = 0;
    else
        strmToReadFrom >> m_enctStamp;

    strmToReadFrom.CheckForFrameMarker(CID_FILE, CID_LINE);

    // Reset any non-persistent stuff
    m_c4AltId = 0;
    m_c4LoadOrder = 0;
    m_strAltId.Clear();
}


tCIDLib::TVoid TMediaDBBase::StreamTo(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo   << tCIDLib::EStreamMarkers::Frame
                    << CQCMedia_DBInfo::c2BaseFmtVersion
                    << m_eType
                    << m_strName
                    << m_c2Id
                    << m_c2ArtId
                    << m_strUniqueId

                    // V2 stuff
                    << m_enctStamp

                    << tCIDLib::EStreamMarkers::Frame;
}


// ---------------------------------------------------------------------------
//  TMediaDBBase: Protected, non-virtual methods
// ---------------------------------------------------------------------------

//
//  A helper that all the derived classes call to check for whether an
//  expected magic value is next in the input stream.
//
tCIDLib::TVoid
TMediaDBBase::CheckMagicVal(const   tCIDLib::TCard4     c4Line
                            , const tCIDLib::TCard4     c4Exected
                            ,       TBinInStream&       strmToReadFrom
                            , const tCIDLib::TCh* const pszType)
{
    tCIDLib::TCard4 c4Val;
    strmToReadFrom >> c4Val;
    if (c4Val != c4Exected)
    {
        facCQCMedia().ThrowErr
        (
            CID_FILE
            , c4Line
            , kMedErrs::errcDB_BadMagicVal
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TString(pszType)
        );
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TMediaCat
//  PREFIX: mcat
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TMediaCat: Public, static methods
// ---------------------------------------------------------------------------

tCIDLib::ESortComps
TMediaCat::eCompById(const  TMediaCat&      mcat1
                    , const TMediaCat&      mcat2)
{
    const tCIDLib::TCard2 c2Id1 = mcat1.c2Id();
    const tCIDLib::TCard2 c2Id2 = mcat2.c2Id();
    if (c2Id1 < c2Id2)
        return tCIDLib::ESortComps::FirstLess;
    if (c2Id1 > c2Id2)
        return tCIDLib::ESortComps::FirstGreater;
    return tCIDLib::ESortComps::Equal;
}

tCIDLib::ESortComps
TMediaCat::eCompByName(const    TMediaCat&      mcat1
                        , const TMediaCat&      mcat2)
{
    return mcat1.strName().eCompareI(mcat2.strName());
}

tCIDLib::ESortComps
TMediaCat::eIdKeyComp(  const   tCIDLib::TCard2&    c2Id
                        , const TMediaCat&          mcatComp)
{
    const tCIDLib::TCard2 c2Id2 = mcatComp.c2Id();
    if (c2Id < c2Id2)
        return tCIDLib::ESortComps::FirstLess;
    if (c2Id > c2Id2)
        return tCIDLib::ESortComps::FirstGreater;
    return tCIDLib::ESortComps::Equal;
}


// Key access for us in hash lists by unique id
const TString& TMediaCat::strKeyUID(const TMediaCat& mcatSrc)
{
    return mcatSrc.strUniqueId();
}


// ---------------------------------------------------------------------------
//  TMediaCat: Constructors and Destructor
// ---------------------------------------------------------------------------
TMediaCat::TMediaCat() :

    TMediaDBBase(tCQCMedia::EMediaTypes::Count)
{
}

TMediaCat::TMediaCat(const  tCQCMedia::EMediaTypes  eType
                    , const TString&                strName
                    , const TString&                strUniqueId) :

    TMediaDBBase(strName, strUniqueId, eType)
{
}

TMediaCat::~TMediaCat()
{
}


// ---------------------------------------------------------------------------
//  TMediaCat: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TMediaCat::ToXML(TTextOutStream& strmTar) const
{
    // Start the element and then just call our parent, since have nothing else
    strmTar << L"<Cat ";

    // Call our parent to let him do his stuff
    TParent::ToXML(strmTar);

    // Close it off
    strmTar << L"/>\n";
}


// ---------------------------------------------------------------------------
//  TMediaCat: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TMediaCat::StreamFrom(TBinInStream& strmToReadFrom)
{
    //
    //  We store an extra magic value at the start of media objects because
    //  they are flattened into their own files in our own repository and
    //  this helps to quickly check whether they are valid or not.
    //
    CheckMagicVal
    (
        CID_LINE, CQCMedia_DBInfo::c4MagicCatVal, strmToReadFrom, L"category"
    );

    // That looks ok, so check for our start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCMedia_DBInfo::c2CatFmtVersion))
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

    // Call our parent
    TParent::StreamFrom(strmToReadFrom);

    // We don't have any of our own stuff right now

    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TMediaCat::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Stream our magic value, start marker, and format version
    strmToWriteTo   << CQCMedia_DBInfo::c4MagicCatVal
                    << tCIDLib::EStreamMarkers::StartObject
                    << CQCMedia_DBInfo::c2CatFmtVersion;

    // Do our parent
    TParent::StreamTo(strmToWriteTo);

    // We don't have any stuff of our own right now, so jut do the marker
    strmToWriteTo   << tCIDLib::EStreamMarkers::EndObject;
}





// ---------------------------------------------------------------------------
//   CLASS: TMediaImg
//  PREFIX: mimg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TMediaImg: Public, static methods
// ---------------------------------------------------------------------------

tCIDLib::ESortComps
TMediaImg::eCompById(const  TMediaImg&      mimg1
                    , const TMediaImg&      mimg2)
{
    const tCIDLib::TCard2 c2Id1 = mimg1.c2Id();
    const tCIDLib::TCard2 c2Id2 = mimg2.c2Id();
    if (c2Id1 < c2Id2)
        return tCIDLib::ESortComps::FirstLess;
    if (c2Id1 > c2Id2)
        return tCIDLib::ESortComps::FirstGreater;
    return tCIDLib::ESortComps::Equal;
}


tCIDLib::ESortComps
TMediaImg::eCompByLrgPID(const  TMediaImg&      mimg1
                        , const TMediaImg&      mimg2)
{
    return mimg1.m_strPersistentIdLrg.eCompare(mimg2.m_strPersistentIdLrg);
}


tCIDLib::ESortComps
TMediaImg::eCompBySmlPID(const  TMediaImg&      mimg1
                        , const TMediaImg&      mimg2)
{
    return mimg1.m_strPersistentIdSml.eCompare(mimg2.m_strPersistentIdSml);
}


tCIDLib::ESortComps
TMediaImg::eIdKeyComp(  const   tCIDLib::TCard2&    c2Id
                        , const TMediaImg&          mimg2)
{
    const tCIDLib::TCard2 c2Id2 = mimg2.c2Id();
    if (c2Id < c2Id2)
        return tCIDLib::ESortComps::FirstLess;
    if (c2Id > c2Id2)
        return tCIDLib::ESortComps::FirstGreater;
    return tCIDLib::ESortComps::Equal;
}


tCIDLib::ESortComps
TMediaImg::eSmlPIDKeyComp(  const   TString&        strPID
                            , const TMediaImg&      mimg2)
{
    return strPID.eCompare(mimg2.m_strPersistentIdSml);
}


tCIDLib::ESortComps
TMediaImg::eLrgPIDKeyComp(  const   TString&        strPID
                            , const TMediaImg&      mimg2)
{
    return strPID.eCompare(mimg2.m_strPersistentIdLrg);
}

const TString& TMediaImg::strKeyLrgPID(const TMediaImg& mimgSrc)
{
    return mimgSrc.m_strPersistentIdLrg;
}

const TString& TMediaImg::strKeyUID(const TMediaImg& mimgSrc)
{
    return mimgSrc.strUniqueId();
}


// ---------------------------------------------------------------------------
//  TMediaImg: Constructors and Destructor
// ---------------------------------------------------------------------------
TMediaImg::TMediaImg() :

    TMediaDBBase(tCQCMedia::EMediaTypes::Count)
    , m_c4Size(0)
    , m_c4ThumbSize(0)
    , m_mbufArt(8)
    , m_mbufThumbArt(8)
{
}

TMediaImg::TMediaImg(const  tCQCMedia::EMediaTypes  eType
                    , const TString&                strArtPath
                    , const TString&                strThumbArtPath) :

    TMediaDBBase(TString::strEmpty(), TString::strEmpty(), eType)
    , m_c4Size(0)
    , m_c4ThumbSize(0)
    , m_mbufArt(8)
    , m_mbufThumbArt(8)
    , m_strArtPath(strArtPath)
    , m_strThumbArtPath(strThumbArtPath)
{
}

TMediaImg::~TMediaImg()
{
}


// ---------------------------------------------------------------------------
//  TMediaImg: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TMediaImg::Reset()
{
    m_c4Size = 0;
    m_c4ThumbSize = 0;
    m_strArtPath.Clear();
    m_strPersistentIdLrg.Clear();
    m_strPersistentIdPos.Clear();
    m_strPersistentIdSml.Clear();
    m_strThumbArtPath.Clear();
    m_strPosterArtPath.Clear();
    TParent::Reset();
}


//
//  Note that we aren't interested in the actual image data or paths here. We are
//  only interested in the actual metadata.
//
tCIDLib::TVoid TMediaImg::ToXML(TTextOutStream& strmTar) const
{
    // Start the element for this guy and do the attributes at our level
    strmTar << L"<Img ";

    //
    //  For those persistent ids set, we put them out since some clients will need
    //  to use these to query art.
    //
    if (!m_strPersistentIdLrg.bIsEmpty())
        strmTar << L" LrgPId=\"" << m_strPersistentIdLrg << L"\"";

    if (!m_strPersistentIdPos.bIsEmpty())
        strmTar << L" PosPId=\"" << m_strPersistentIdPos << L"\"";

    if (!m_strPersistentIdSml.bIsEmpty())
        strmTar << L" SmlPId=\"" << m_strPersistentIdSml << L"\"";

    // Call our parent to let him do his stuff
    TParent::ToXML(strmTar);

    // Close it off
    strmTar << L"/>\n";
}


// ---------------------------------------------------------------------------
//  TMediaImg: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Query the art size and data. We currently don't cache poster art so just
//  return zero for that always.
//
tCIDLib::TCard4
TMediaImg::c4QueryArt(          TMemBuf&                mbufToFill
                        , const tCQCMedia::ERArtTypes   eType
                        , const tCIDLib::TCard4         c4AtIndex) const
{
    tCIDLib::TCard4 c4Ret = 0;

    if (eType == tCQCMedia::ERArtTypes::LrgCover)
    {
        c4Ret = m_c4Size;
        if (c4Ret)
            mbufToFill.CopyIn(m_mbufArt, c4Ret, c4AtIndex);
    }
     else if (eType == tCQCMedia::ERArtTypes::SmlCover)
    {
        c4Ret = m_c4ThumbSize;
        if (c4Ret)
            mbufToFill.CopyIn(m_mbufThumbArt, c4Ret, c4AtIndex);
    }
    return c4Ret;
}


//
//  Get the art size. We don't currently cache poster art, so always return
//  zero for that.
//
tCIDLib::TCard4 TMediaImg::c4Size(const tCQCMedia::ERArtTypes eType) const
{
    if (eType == tCQCMedia::ERArtTypes::LrgCover)
        return m_c4Size;
    else if (eType == tCQCMedia::ERArtTypes::SmlCover)
        return m_c4ThumbSize;

    return 0;
}


// Clear all the art and associated persistent ids
tCIDLib::TVoid TMediaImg::ClearAllArt()
{
    m_c4Size = 0;
    m_mbufArt.Reallocate(8, kCIDLib::False);

    m_c4ThumbSize = 0;
    m_mbufThumbArt.Reallocate(8, kCIDLib::False);

    m_strPersistentIdPos.Clear();
    m_strPersistentIdLrg.Clear();
    m_strPersistentIdSml.Clear();
}


//
//  We don't cache poster art so nothing to do in that case, but we do still have
//  to clear the persistent id.
//
tCIDLib::TVoid TMediaImg::ClearArt(const tCQCMedia::ERArtTypes eType)
{
    if (eType == tCQCMedia::ERArtTypes::LrgCover)
    {
        m_c4Size = 0;
        m_mbufArt.Reallocate(8, kCIDLib::False);
        m_strPersistentIdLrg.Clear();
    }
     else if (eType == tCQCMedia::ERArtTypes::Poster)
    {
        m_strPersistentIdPos.Clear();
    }
     else if (eType == tCQCMedia::ERArtTypes::SmlCover)
    {
        m_c4ThumbSize = 0;
        m_mbufThumbArt.Reallocate(8, kCIDLib::False);
        m_strPersistentIdSml.Clear();
    }
}


//
//  Get read access to the art buffers. Since we always return zero for
//  the size of poster art, we should not be called for that. If we are,
//  we'll just return the large art buffer.
//
const THeapBuf& TMediaImg::mbufArt(const tCQCMedia::ERArtTypes eType) const
{
    if (eType == tCQCMedia::ERArtTypes::SmlCover)
        return m_mbufThumbArt;

    return m_mbufArt;
}


// Get the indicated art path
const TString& TMediaImg::strArtPath(const tCQCMedia::ERArtTypes eType) const
{
    if (eType == tCQCMedia::ERArtTypes::Poster)
        return m_strPosterArtPath;
    else if (eType == tCQCMedia::ERArtTypes::LrgCover)
        return m_strArtPath;

    return m_strThumbArtPath;
}


// Get the persistent id for one of the images
const TString& TMediaImg::strPersistentId(const tCQCMedia::ERArtTypes eType) const
{
    if (eType == tCQCMedia::ERArtTypes::LrgCover)
        return m_strPersistentIdLrg;
    else if (eType == tCQCMedia::ERArtTypes::Poster)
        return m_strPersistentIdPos;

    return m_strPersistentIdSml;
}


//
//  Set the data for one of the art images. We currently don't cache poster
//  art, so just ignore attempts to do that. This lets the rest of the code
//  not have to special case poster art. But we do still store the persistent id
//  for later use if needed.
//
//  NOTE: The caller should make sure that the persistent id is updated if that
//  is appropriate. Often it's already set and the art work is just be stored after
//  the fact.
//
tCIDLib::TVoid TMediaImg::SetArt(const  tCIDLib::TCard4         c4BufSz
                                , const TMemBuf&                mbufArt
                                , const tCQCMedia::ERArtTypes   eType)
{
    if (eType == tCQCMedia::ERArtTypes::LrgCover)
    {
        m_c4Size = c4BufSz;
        m_mbufArt.CopyIn(mbufArt, c4BufSz);
    }
     else if (eType == tCQCMedia::ERArtTypes::Poster)
    {
    }
     else if (eType == tCQCMedia::ERArtTypes::SmlCover)
    {
        m_c4ThumbSize = c4BufSz;
        m_mbufThumbArt.CopyIn(mbufArt, c4BufSz);
    }
}

tCIDLib::TVoid TMediaImg::SetArt(       TBinInStream&           strmSrc
                                , const tCIDLib::TCard4         c4BufSz
                                , const tCQCMedia::ERArtTypes   eType)
{
    if (eType == tCQCMedia::ERArtTypes::LrgCover)
    {
        m_c4Size = c4BufSz;
        strmSrc.c4ReadBuffer(m_mbufArt, c4BufSz);
    }
     else if (eType == tCQCMedia::ERArtTypes::Poster)
    {
    }
     else if (eType == tCQCMedia::ERArtTypes::SmlCover)
    {
        m_c4ThumbSize = c4BufSz;
        strmSrc.c4ReadBuffer(m_mbufArt, c4BufSz);
    }
}


// Set the indicated art path
tCIDLib::TVoid TMediaImg::SetArtPath(const  TString&                strToSet
                                    , const tCQCMedia::ERArtTypes   eType)
{
    if (eType == tCQCMedia::ERArtTypes::Poster)
        m_strPosterArtPath = strToSet;
    else if (eType == tCQCMedia::ERArtTypes::LrgCover)
        m_strArtPath = strToSet;
    else
        m_strThumbArtPath = strToSet;
}


// Set the indicated persistent id
tCIDLib::TVoid TMediaImg::SetPersistentId(  const   TString&                strToSet
                                            , const tCQCMedia::ERArtTypes   eType)
{
    if (eType == tCQCMedia::ERArtTypes::Poster)
        m_strPersistentIdPos = strToSet;
    else if (eType == tCQCMedia::ERArtTypes::LrgCover)
        m_strPersistentIdLrg = strToSet;
    else
        m_strPersistentIdSml = strToSet;
}



// ---------------------------------------------------------------------------
//  TMediaCat: Protected, inherited methods
// ---------------------------------------------------------------------------

//
//  Note that we do not stream image data here. That's only for runtime, for
//  caching in memory. The database itself just needs to hold the info about
//  what id it is, and the paths to load when the image data needs to be loaded.
//
tCIDLib::TVoid TMediaImg::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Check for our start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCMedia_DBInfo::c2ImageFmtVersion))
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

    // Call our parent
    TParent::StreamFrom(strmToReadFrom);

    // And stream out our stuff
    strmToReadFrom  >> m_strArtPath
                    >> m_strPersistentIdLrg
                    >> m_strThumbArtPath;

    //
    //  If V2 or beyond, read in the poster art path and new persistent ids, else
    //  default them.
    //
    if (c2FmtVersion > 1)
    {
        strmToReadFrom  >> m_strPosterArtPath
                        >> m_strPersistentIdPos
                        >> m_strPersistentIdSml;
    }
     else
    {
        // Clear the post stuff
        m_strPosterArtPath.Clear();
        m_strPersistentIdPos.Clear();

        //
        //  Set the new small persistent id to the same as the large. The only
        //  persistent use of these classes are by the CQSL repo which uses the
        //  same image for both, just scaled down.
        //
        m_strPersistentIdSml = m_strPersistentIdLrg;
    }

    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);

    // Reset runtime only stuff
    m_c4Size = 0;
    m_c4ThumbSize = 0;
    m_mbufArt.Reallocate(8, kCIDLib::False);
    m_mbufThumbArt.Reallocate(8, kCIDLib::False);
}


tCIDLib::TVoid TMediaImg::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Stream our magic value, start marker, and format version
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCMedia_DBInfo::c2ImageFmtVersion;

    // Do our parent
    TParent::StreamTo(strmToWriteTo);

    // And stream out our stuff
    strmToWriteTo   << m_strArtPath
                    << m_strPersistentIdLrg
                    << m_strThumbArtPath

                    // V2 stuff
                    << m_strPosterArtPath
                    << m_strPersistentIdPos
                    << m_strPersistentIdSml;

    // We don't have any stuff of our own right now, so jut do the marker
    strmToWriteTo   << tCIDLib::EStreamMarkers::EndObject;
}






// ---------------------------------------------------------------------------
//   CLASS: TMediaItem
//  PREFIX: mitem
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TMediaItem: Public, static methods
// ---------------------------------------------------------------------------
tCIDLib::ESortComps
TMediaItem::eCompById(  const   TMediaItem&     mitem1
                        , const TMediaItem&     mitem2)
{
    const tCIDLib::TCard2 c2Id1 = mitem1.c2Id();
    const tCIDLib::TCard2 c2Id2 = mitem2.c2Id();
    if (c2Id1 < c2Id2)
        return tCIDLib::ESortComps::FirstLess;
    else if (c2Id1 > c2Id2)
        return tCIDLib::ESortComps::FirstGreater;
    return tCIDLib::ESortComps::Equal;
}

tCIDLib::ESortComps
TMediaItem::eCompByName(const   TMediaItem&     mitem1
                        , const TMediaItem&     mitem2)
{
    return mitem1.strName().eCompareI(mitem2.strName());
}


tCIDLib::ESortComps
TMediaItem::eIdKeyComp( const   tCIDLib::TCard2&    c2Id
                        , const TMediaItem&         mitem2)
{
    const tCIDLib::TCard2 c2Id2 = mitem2.c2Id();
    if (c2Id < c2Id2)
        return tCIDLib::ESortComps::FirstLess;
    if (c2Id > c2Id2)
        return tCIDLib::ESortComps::FirstGreater;
    return tCIDLib::ESortComps::Equal;
}

const TString& TMediaItem::strUIDAccess(const TMediaItem& mitemSrc)
{
    return mitemSrc.strUniqueId();
}


// ---------------------------------------------------------------------------
//  TMediaItem: Constructors and operators
// ---------------------------------------------------------------------------
TMediaItem::TMediaItem(const tCQCMedia::EMediaTypes eType) :

    TMediaDBBase(eType)
    , m_c4BitDepth(0)
    , m_c4BitRate(0)
    , m_c4Channels(0)
    , m_c4Duration(0)
    , m_c4Rating(0)
    , m_c4SampleRate(0)
    , m_c4Year(0)
    , m_fcolCatIds(1UL)
{
}

TMediaItem::TMediaItem( const   TString&                strName
                        , const TString&                strUniqueId
                        , const TString&                strLocInfo
                        , const tCQCMedia::EMediaTypes  eType) :

    TMediaDBBase(strName, strUniqueId, eType)
    , m_c4BitDepth(0)
    , m_c4BitRate(0)
    , m_c4Channels(0)
    , m_c4Duration(0)
    , m_c4Rating(0)
    , m_c4SampleRate(0)
    , m_c4Year(0)
    , m_fcolCatIds(1UL)
    , m_strLocInfo(strLocInfo)
{
}

TMediaItem::~TMediaItem()
{
}


// -------------------------------------------------------------------
//  TMediaItem: Public, inherited methods
// -------------------------------------------------------------------
tCIDLib::TVoid TMediaItem::Reset()
{
    m_c4BitDepth = 0;
    m_c4BitRate = 0;
    m_c4Channels = 0;
    m_c4Duration = 0;
    m_c4Rating = 0;
    m_c4SampleRate = 0;
    m_c4Year = 0;
    m_fcolCatIds.RemoveAll();
    m_strArtist.Clear();
    m_strLocInfo.Clear();

    TParent::Reset();
}


tCIDLib::TVoid TMediaItem::ToXML(TTextOutStream& strmTar) const
{
    // Start the element for this guy and do the attributes at our level
    strmTar << L"<Item";

    if (m_c4BitDepth)
        strmTar << L" BitD=\"" << m_c4BitDepth << L"\"";

    if (m_c4BitRate)
        strmTar << L" BitR=\"" << m_c4BitRate << L"\"";

    if (m_c4Channels)
        strmTar << L" Chs=\"" << m_c4Channels << L"\"";

    if (m_c4Rating)
        strmTar << L" URat=\"" << m_c4Rating << L"\"";

    if (m_c4SampleRate)
        strmTar << L" SRate=\"" << m_c4SampleRate << L"\"";

    if (m_c4Duration)
        strmTar << L" Secs=\"" << m_c4Duration << L"\"";

    if (m_c4Year)
        strmTar << L" Year=\"" << m_c4Year << L"\"";

    // These require escapement
    facCIDXML().FormatAttr(strmTar, L"Artist", m_strArtist, kCIDLib::True);

    if (!m_strLocInfo.bIsEmpty())
        facCIDXML().FormatAttr(strmTar, L"Loc", m_strLocInfo, kCIDLib::True);

    // Call our parent to let him do his stuff and close off the start element
    TParent::ToXML(strmTar);
    strmTar << L">";

    // The category ids we do in a child element
    strmTar << L"<InCats>";
    const tCIDLib::TCard4 c4Count = m_fcolCatIds.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        strmTar << m_fcolCatIds[c4Index] << L' ';
    strmTar << L"</InCats>";

    // And do the close element
    strmTar << L"</Item>\n";
}


// ---------------------------------------------------------------------------
//  TMediaItem: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Add a category at this level (to later be aggregated)
tCIDLib::TBoolean TMediaItem::bAddCategory(const tCIDLib::TCard2 c2Id)
{
    // It has to be non-zero
    #if CID_DEBUG_ON
    if (!c2Id)
    {
        facCQCMedia().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMedErrs::errcDB_ZeroId
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::BadParms
            , tCQCMedia::strXlatEDataTypes(tCQCMedia::EDataTypes::Cat)
            , tCQCMedia::strXlatEMediaTypes(eType())
        );
    }
    #endif

    if (!m_fcolCatIds.bElementPresent(c2Id))
    {
        m_fcolCatIds.c4AddElement(c2Id);
        return kCIDLib::True;
    }
    return kCIDLib::False;
}

// Get/set the bit depth
tCIDLib::TCard4 TMediaItem::c4BitDepth() const
{
    return m_c4BitDepth;
}

tCIDLib::TCard4 TMediaItem::c4BitDepth(const tCIDLib::TCard4 c4ToSet)
{
    m_c4BitDepth = c4ToSet;
    return m_c4BitDepth;
}


// Get/set the bit rate
tCIDLib::TCard4 TMediaItem::c4BitRate() const
{
    return m_c4BitRate;
}

tCIDLib::TCard4 TMediaItem::c4BitRate(const tCIDLib::TCard4 c4ToSet)
{
    m_c4BitRate = c4ToSet;
    return m_c4BitRate;
}


// Get/set the channel count
tCIDLib::TCard4 TMediaItem::c4Channels() const
{
    return m_c4Channels;
}

tCIDLib::TCard4 TMediaItem::c4Channels(const tCIDLib::TCard4 c4ToSet)
{
    m_c4Channels = c4ToSet;
    return m_c4Channels;
}


// Get/set the duration of this item
tCIDLib::TCard4 TMediaItem::c4Duration() const
{
    return m_c4Duration;
}

tCIDLib::TCard4 TMediaItem::c4Duration(const tCIDLib::TCard4 c4ToSet)
{
    m_c4Duration = c4ToSet;
    return m_c4Duration;
}


// Get/set the rating (used for later aggregation)
tCIDLib::TCard4 TMediaItem::c4Rating() const
{
    return m_c4Rating;
}

tCIDLib::TCard4 TMediaItem::c4Rating(const tCIDLib::TCard4 c4ToSet)
{
    m_c4Rating = c4ToSet;
    return m_c4Rating;
}


// Get/set the sample rate
tCIDLib::TCard4 TMediaItem::c4SampleRate() const
{
    return m_c4SampleRate;
}

tCIDLib::TCard4 TMediaItem::c4SampleRate(const tCIDLib::TCard4 c4ToSet)
{
    m_c4SampleRate = c4ToSet;
    return m_c4SampleRate;
}


// Get/set the year (used for later aggregation)
tCIDLib::TCard4 TMediaItem::c4Year() const
{
    return m_c4Year;
}

tCIDLib::TCard4 TMediaItem::c4Year(const tCIDLib::TCard4 c4ToSet)
{
    m_c4Year = c4ToSet;
    return m_c4Year;
}



// Clear our category aggregating list
tCIDLib::TVoid TMediaItem::ClearCats()
{
    m_fcolCatIds.RemoveAll();
}


//
//  Copy any aggregation category ids we have to the caller's list. We
//  only copy if they are unique, and can optionally accumulate to the
//  caller's list.
//
tCIDLib::TCard4
TMediaItem::c4QueryCats(        tCQCMedia::TIdList& fcolTar
                        , const tCIDLib::TBoolean   bAccum) const
{
    // If accumulating we have to check for dups, else not
    tCIDLib::TCard4 c4Added = 0;
    const tCIDLib::TCard4 c4Count = m_fcolCatIds.c4ElemCount();
    if (bAccum)
    {
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const tCIDLib::TCard2 c2Cur = m_fcolCatIds[c4Index];
            if (!fcolTar.bElementPresent(c2Cur))
            {
                fcolTar.c4AddElement(c2Cur);
                c4Added++;
            }
        }
    }
     else
    {
        // Just assign our list to the caller's
        fcolTar = m_fcolCatIds;
        c4Added = c4Count;
    }
    return c4Added;
}


// Get the artist of this item
const TString& TMediaItem::strArtist() const
{
    return m_strArtist;
}

const TString& TMediaItem::strArtist(const TString& strToSet)
{
    m_strArtist = strToSet;
    return m_strArtist;
}


// Get/set the location information
const TString& TMediaItem::strLocInfo() const
{
    return m_strLocInfo;
}

const TString& TMediaItem::strLocInfo(const TString& strToSet)
{
    m_strLocInfo = strToSet;
    return m_strLocInfo;
}


// ---------------------------------------------------------------------------
//  TMediaItem: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TMediaItem::StreamFrom(TBinInStream& strmToReadFrom)
{
    //
    //  We store an extra magic value at the start of media objects because
    //  they are flattened into their own files in our own repository and
    //  this helps to quickly check whether they are valid or not.
    //
    CheckMagicVal
    (
        CID_LINE, CQCMedia_DBInfo::c4MagicItemVal, strmToReadFrom, L"item"
    );

    // Looks ok so check for our start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCMedia_DBInfo::c2ItemFmtVersion))
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

    // Call our parent class then stream our stuff
    TParent::StreamFrom(strmToReadFrom);
    strmToReadFrom  >> m_c4Channels
                    >> m_c4Duration;

    // We removed the year in V3
    if (c2FmtVersion < 3)
    {
        tCIDLib::TCard4 c4Dummy;
        strmToReadFrom >> c4Dummy;
    }

    //
    //  If version 1, then we have to build up the full artist name from the
    //  old first/last name format, else just read it in.
    //
    if (c2FmtVersion == 1)
    {
        TString strFirstName;
        TString strLastName;
        strmToReadFrom >> strFirstName >> strLastName;

        m_strArtist = strFirstName;
        if (!strFirstName.bIsEmpty() && !strLastName.bIsEmpty())
            m_strArtist.Append(kCIDLib::chSpace);
        m_strArtist.Append(strLastName);
    }
     else
    {
        strmToReadFrom >> m_strArtist;
    }

    strmToReadFrom  >> m_strLocInfo;

    // If V3 or beyond, read in the sample rate/bit depth, else default
    if (c2FmtVersion >= 3)
    {
        strmToReadFrom  >> m_c4BitDepth
                        >> m_c4SampleRate;
    }
     else
    {
        m_c4BitDepth = 0;
        m_c4SampleRate = 0;
    }

    // If V4 or beyond do the same for bit rate
    if (c2FmtVersion >= 4)
        strmToReadFrom  >> m_c4BitRate;
    else
        m_c4BitRate = 0;

    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);

    // Reset non-streamed stuff
    m_c4Rating  = 0;
    m_c4Year    = 0;
    m_fcolCatIds.RemoveAll();
}


tCIDLib::TVoid TMediaItem::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Write out our magic value, marker, and version info
    strmToWriteTo << CQCMedia_DBInfo::c4MagicItemVal
                  << tCIDLib::EStreamMarkers::StartObject
                  << CQCMedia_DBInfo::c2ItemFmtVersion;

    // Let our parent class write his stuff
    TParent::StreamTo(strmToWriteTo);

    // And stream our stuff
    strmToWriteTo   << m_c4Channels
                    << m_c4Duration
                    << m_strArtist
                    << m_strLocInfo

                    << m_c4BitDepth
                    << m_c4SampleRate

                    << m_c4BitRate

                    << tCIDLib::EStreamMarkers::EndObject;
}




// ---------------------------------------------------------------------------
//   CLASS: TMediaCollect
//  PREFIX: mcol
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TMediaDBBase: Public, static methods
// ---------------------------------------------------------------------------
tCIDLib::ESortComps
TMediaCollect::eCompById(const  TMediaCollect&  mcol1
                        , const TMediaCollect&  mcol2)
{
    const tCIDLib::TCard2 c2Id1 = mcol1.c2Id();
    const tCIDLib::TCard2 c2Id2 = mcol2.c2Id();
    if (c2Id1 < c2Id2)
        return tCIDLib::ESortComps::FirstLess;
    else if (c2Id1 > c2Id2)
        return tCIDLib::ESortComps::FirstGreater;
    return tCIDLib::ESortComps::Equal;
}

tCIDLib::ESortComps
TMediaCollect::eCompByName( const   TMediaCollect&  mcol1
                            , const TMediaCollect&  mcol2)
{
    return mcol1.strName().eCompareI(mcol2.strName());
}


tCIDLib::ESortComps
TMediaCollect::eIdKeyComp(  const   tCIDLib::TCard2&    c2Id
                            , const TMediaCollect&      mcol2)
{
    const tCIDLib::TCard2 c2Id2 = mcol2.c2Id();
    if (c2Id < c2Id2)
        return tCIDLib::ESortComps::FirstLess;
    else if (c2Id > c2Id2)
        return tCIDLib::ESortComps::FirstGreater;
    return tCIDLib::ESortComps::Equal;
}

const TString& TMediaCollect::strUIDAccess(const TMediaCollect& mcolSrc)
{
    return mcolSrc.strUniqueId();
}


// ---------------------------------------------------------------------------
//  TMediaCollect: Constructors and Destructor
// ---------------------------------------------------------------------------
TMediaCollect::TMediaCollect(const tCQCMedia::EMediaTypes eType) :

    TMediaDBBase(eType)
    , m_bAnamorphic(kCIDLib::False)
    , m_bIsPlayList(kCIDLib::False)
    , m_c4Duration(0)
    , m_c4Year(0)
    , m_eLocType(tCQCMedia::ELocTypes::FileItem)
    , m_fcolItemIds(12UL)
{
}

TMediaCollect::TMediaCollect(const  TString&                strName
                            , const TString&                strUniqueId
                            , const tCIDLib::TCard4         c4Year
                            , const tCQCMedia::ELocTypes    eLocType
                            , const tCQCMedia::EMediaTypes  eType) :

    TMediaDBBase(strName, strUniqueId, eType)
    , m_bAnamorphic(kCIDLib::False)
    , m_bIsPlayList(kCIDLib::False)
    , m_c4Duration(0)
    , m_c4Year(c4Year)
    , m_eLocType(eLocType)
    , m_fcolItemIds(12UL)
{
}

TMediaCollect::~TMediaCollect()
{
}


// ---------------------------------------------------------------------------
//  TMediaCollect: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TMediaCollect::Reset()
{
    m_bAnamorphic = kCIDLib::False;
    m_bIsPlayList = kCIDLib::False;
    m_c4Duration = 0;
    m_c4Year = 0;
    m_eLocType = tCQCMedia::ELocTypes::FileItem;
    m_fcolCatIds.RemoveAll();
    m_fcolItemIds.RemoveAll();
    m_strAltInfo.Clear();
    m_strArtist.Clear();
    m_strAspectRatio.Clear();
    m_strASIN.Clear();
    m_strCast.Clear();
    m_strDescr.Clear();
    m_strLabel.Clear();
    m_strLeadActor.Clear();
    m_strLocInfo.Clear();
    m_strMediaFmt.Clear();
    m_strPosterArt.Clear();
    m_strRating.Clear();
    m_strUPC.Clear();

    TParent::Reset();
}


tCIDLib::TVoid TMediaCollect::ToXML(TTextOutStream& strmTar) const
{
    // Start the element for this guy and do the attributes at our level
    strmTar << L"<Col"
            << L" Ana=\"" << (m_bAnamorphic ? L'Y' : L'N')
            << L"\" IsPL=\"" << (m_bIsPlayList ? L'Y' : L'N')
            << L"\" Secs=\"" << m_c4Duration
            << L"\" Year=\"" << m_c4Year
            << L"\" LType=\"" << tCQCMedia::strAltXlatELocTypes(m_eLocType)
            << L"\" AR=\"" << m_strAspectRatio
            << L"\"";

    // These require escapement, some we'll only do if not empty
    facCIDXML().FormatAttr(strmTar, L"Artist", m_strArtist, kCIDLib::True);
    facCIDXML().FormatAttr(strmTar, L"Lab", m_strLabel, kCIDLib::True);
    facCIDXML().FormatAttr(strmTar, L"LAct", m_strLeadActor, kCIDLib::True);
    facCIDXML().FormatAttr(strmTar, L"Loc", m_strLocInfo, kCIDLib::True);
    facCIDXML().FormatAttr(strmTar, L"Fmt", m_strMediaFmt, kCIDLib::True);
    facCIDXML().FormatAttr(strmTar, L"Rat", m_strRating, kCIDLib::True);

    if (!m_strAltInfo.bIsEmpty())
        facCIDXML().FormatAttr(strmTar, L"AltInf", m_strAltInfo, kCIDLib::True);

    if (!m_strASIN.bIsEmpty())
        facCIDXML().FormatAttr(strmTar, L"ASIN", m_strASIN, kCIDLib::True);

    if (!m_strPosterArt.bIsEmpty())
        facCIDXML().FormatAttr(strmTar, L"PArt", m_strPosterArt, kCIDLib::True);

    if (!m_strUPC.bIsEmpty())
        facCIDXML().FormatAttr(strmTar, L"UPC", m_strUPC, kCIDLib::True);

    // Call our parent to let him do his stuff and close off the start element
    TParent::ToXML(strmTar);
    strmTar << L">";

    // These are done as child elements
    strmTar << L"<InCats>";
    tCIDLib::TCard4 c4Count = m_fcolCatIds.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        strmTar << m_fcolCatIds[c4Index] << L' ';
    strmTar << L"</InCats>";

    strmTar << L"<CItems>";
    c4Count = m_fcolItemIds.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        strmTar << m_fcolItemIds[c4Index] << L' ';
    strmTar << L"</CItems>";

    strmTar << L"<Cast>";
    facCIDXML().EscapeFor(m_strCast, strmTar, tCIDXML::EEscTypes::ElemText);
    strmTar << L"</Cast>";

    strmTar << L"<Desc>";
    facCIDXML().EscapeFor(m_strDescr, strmTar, tCIDXML::EEscTypes::ElemText);
    strmTar << L"</Desc>";

    // And do the close element
    strmTar << L"</Col>\n";
}



// ---------------------------------------------------------------------------
//  TMediaCollect: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Add any of our categories to the list if they are not already in it.
tCIDLib::TVoid TMediaCollect::AddCatsToList(tCQCMedia::TIdList& fcolToFill) const
{
    const tCIDLib::TCard4 c4CatCnt = m_fcolCatIds.c4ElemCount();
    for (tCIDLib::TCard4 c4CInd = 0; c4CInd < c4CatCnt; c4CInd++)
    {
        const tCIDLib::TCard2 c2CatId = m_fcolCatIds[c4CInd];
        if (!fcolToFill.bElementPresent(c2CatId))
            fcolToFill.c4AddElement(c2CatId);
    }
}


// Add a category id to our list of categories
tCIDLib::TBoolean TMediaCollect::bAddCategory(const tCIDLib::TCard2 c2CatId)
{
    // It has to be non-zero
    #if CID_DEBUG_ON
    if (!c2CatId)
    {
        facCQCMedia().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMedErrs::errcDB_ZeroId
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::BadParms
            , tCQCMedia::strXlatEDataTypes(tCQCMedia::EDataTypes::Cat)
            , tCQCMedia::strXlatEMediaTypes(eType())
        );
    }
    #endif


    // If not in the list already, then add it
    if (!m_fcolCatIds.bElementPresent(c2CatId))
    {
        m_fcolCatIds.c4AddElement(c2CatId);
        return kCIDLib::True;
    }
    return kCIDLib::False;
}

tCIDLib::TBoolean TMediaCollect::bAddCategory(const TMediaCat& mcatToAdd)
{
    // It has to be of our type and non-zero id
    #if CID_DEBUG_ON
    if (mcatToAdd.eType() != eType())
    {
        facCQCMedia().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMedErrs::errcDB_BadMediaType
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::BadParms
            , tCQCMedia::strXlatEDataTypes(tCQCMedia::EDataTypes::Collect)
            , strName()
            , tCQCMedia::strXlatEMediaTypes(mcatToAdd.eType())
        );
    }

    if (!mcatToAdd.c2Id())
    {
        facCQCMedia().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMedErrs::errcDB_ZeroId
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::BadParms
            , tCQCMedia::strXlatEDataTypes(tCQCMedia::EDataTypes::Cat)
            , tCQCMedia::strXlatEMediaTypes(eType())
        );
    }
    #endif

    // If not in the list already, then add it
    if (!m_fcolCatIds.bElementPresent(mcatToAdd.c2Id()))
    {
        m_fcolCatIds.c4AddElement(mcatToAdd.c2Id());
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


//
//  Adds the passed item to our list of items. We store the id in our list
//  of collection ids. They can indicate where in the list to add it, with
//  the default value being kCIDLib::c4MaxCard, which means just append.
//
tCIDLib::TBoolean
TMediaCollect::bAddItem(const   TMediaItem&     mitemToAdd
                        , const tCIDLib::TCard4 c4At)
{
    // It has to be of our type and have a non-zero id
    #if CID_DEBUG_ON
    if (mitemToAdd.eType() != eType())
    {
        facCQCMedia().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMedErrs::errcDB_BadMediaType
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::BadParms
            , tCQCMedia::strXlatEDataTypes(tCQCMedia::EDataTypes::Item)
            , strName()
            , tCQCMedia::strXlatEMediaTypes(mitemToAdd.eType())
        );
    }

    if (!mitemToAdd.c2Id())
    {
        facCQCMedia().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMedErrs::errcDB_ZeroId
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::BadParms
            , tCQCMedia::strXlatEDataTypes(tCQCMedia::EDataTypes::Item)
            , tCQCMedia::strXlatEMediaTypes(mitemToAdd.eType())
        );
    }
    #endif

    const tCIDLib::TCard2 c2NewId = mitemToAdd.c2Id();
    if (!m_fcolItemIds.bElementPresent(c2NewId))
    {
        if (c4At == kCIDLib::c4MaxCard)
            m_fcolItemIds.c4AddElement(c2NewId);
        else
            m_fcolItemIds.Insert(c2NewId, c4At);
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


// Get/set the anamorphic member
tCIDLib::TBoolean TMediaCollect::bAnamorphic() const
{
    return m_bAnamorphic;
}

tCIDLib::TBoolean TMediaCollect::bAnamorphic(const tCIDLib::TBoolean bToSet)
{
    m_bAnamorphic = bToSet;
    return m_bAnamorphic;
}


//
//  Returns true if this collection has an item with the passed item id. If so, it also
//  returns the index of that item within the collection.
//
tCIDLib::TBoolean
TMediaCollect::bContainsItem(const tCIDLib::TCard2 c2ItemId, tCIDLib::TCard2& c2ItemInd) const
{
    tCIDLib::TCard4 c4At;
    if (m_fcolItemIds.bElementPresent(c2ItemId, c4At))
    {
        // Has to be able to fit in a Card2
        if (c4At > kCIDLib::c2MaxCard)
        {
            facCQCMedia().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kMedErrs::errcDB_BadMediaIndex
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Internal
                , TCardinal(c4At)
                , tCQCMedia::strXlatEDataTypes(tCQCMedia::EDataTypes::Collect)
                , strName()
            );
        }
        c2ItemInd = tCIDLib::TCard2(c4At);
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


// Get/set the is playlist member
tCIDLib::TBoolean TMediaCollect::bIsPlayList() const
{
    return m_bIsPlayList;
}

tCIDLib::TBoolean TMediaCollect::bIsPlayList(const tCIDLib::TBoolean bToSet)
{
    m_bIsPlayList = bToSet;
    return m_bIsPlayList;
}


//
//  Returns true if this collection is in the indicated category. We assume
//  the caller only passes us cat ids of the same media type we are.
//
tCIDLib::TBoolean TMediaCollect::bIsInCat(const tCIDLib::TCard2 c2CatId) const
{
    return m_fcolCatIds.bElementPresent(c2CatId);
}


// Returns the category id at a given index
tCIDLib::TCard2 TMediaCollect::c2CatIdAt(const tCIDLib::TCard4 c4At) const
{
    if (c4At >= m_fcolCatIds.c4ElemCount())
    {
        facCQCMedia().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMedErrs::errcDB_BadMediaIndex
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::BadParms
            , TCardinal(c4At)
            , tCQCMedia::strXlatEDataTypes(tCQCMedia::EDataTypes::Collect)
            , strName()
        );
    }
    return m_fcolCatIds[c4At];
}


// Returns the item id at a given index
tCIDLib::TCard2 TMediaCollect::c2ItemIdAt(const tCIDLib::TCard4 c4At) const
{
    if (c4At >= m_fcolItemIds.c4ElemCount())
    {
        facCQCMedia().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMedErrs::errcDB_BadMediaIndex
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::BadParms
            , TCardinal(c4At)
            , tCQCMedia::strXlatEDataTypes(tCQCMedia::EDataTypes::Collect)
            , strName()
        );
    }
    return m_fcolItemIds[c4At];
}


// Get or set the duration of this collection
tCIDLib::TCard4 TMediaCollect::c4Duration() const
{
    return m_c4Duration;
}

tCIDLib::TCard4 TMediaCollect::c4Duration(const tCIDLib::TCard4 c4ToSet)
{
    m_c4Duration = c4ToSet;
    return m_c4Duration;
}


// The number of categories we have
tCIDLib::TCard4 TMediaCollect::c4CatCount() const
{
    return m_fcolCatIds.c4ElemCount();
}


// The number of items (tracks, chapters, pictures) we have
tCIDLib::TCard4 TMediaCollect::c4ItemCount() const
{
    return m_fcolItemIds.c4ElemCount();
}


// Get/set the year of this collection
tCIDLib::TCard4 TMediaCollect::c4Year() const
{
    return m_c4Year;
}

tCIDLib::TCard4 TMediaCollect::c4Year(const tCIDLib::TCard4 c4Year)
{
    m_c4Year = c4Year;
    return m_c4Year;
}


// Clear all of the category ids
tCIDLib::TVoid TMediaCollect::ClearCategories()
{
    m_fcolCatIds.RemoveAll();
}


// Clears our item id list
tCIDLib::TVoid TMediaCollect::ClearItems()
{
    m_fcolItemIds.RemoveAll();
}


// Return the location type for this collection
tCQCMedia::ELocTypes TMediaCollect::eLocType() const
{
    return m_eLocType;
}


//
//  We get called here after a reload of the database, or after underlying
//  items have been updated, to let us re-aggregate any info and so forth.
//  This much not do anything that can't be redone multiple times in response
//  to editing of underlying items!
//
tCIDLib::TVoid TMediaCollect::Finalize(TMediaDB& mdbInfo)
{
    //
    //  If music, then aggregate some info from the contained items.
    //
    if (eType() == tCQCMedia::EMediaTypes::Music)
    {
        tCIDLib::TBoolean   bVariousArtists = kCIDLib::False;
        tCIDLib::TCard4     c4Duration = kCIDLib::c4MaxCard;
        tCIDLib::TCard4     c4Year = kCIDLib::c4MaxCard;
        tCQCMedia::TIdList  fcolCats(8UL);

        const TMediaItem* pmitemPrev = 0;
        const tCIDLib::TCard4 c4Count = m_fcolItemIds.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const TMediaItem* pmitemCur = mdbInfo.pmitemById
            (
                eType(), m_fcolItemIds[c4Index], kCIDLib::False
            );

            if (pmitemCur)
            {
                //
                //  Take the artist of the first one. If we don't end up
                //  with the various artist's flag set, then they are all
                //  the same and we can keep this one.
                //
                if (!c4Index)
                    m_strArtist = pmitemCur->strArtist();

                c4Duration += pmitemCur->c4Duration();

                // If a non-zero year and lower than what we have so far, take it
                if (pmitemCur->c4Year() && (pmitemCur->c4Year() < c4Year))
                    c4Year = pmitemCur->c4Year();

                if (pmitemPrev
                &&  (pmitemPrev->strArtist() != pmitemCur->strArtist()))
                {
                    bVariousArtists = kCIDLib::True;
                }

                // If any categories stored at the unit level add them
                pmitemCur->c4QueryCats(m_fcolCatIds);
            }

            // Save this one as the previous one and go again
            pmitemPrev = pmitemCur;
        }

        // If we got a duration and ours is zero, store it
        if (!m_c4Duration && c4Duration && (c4Duration != kCIDLib::c4MaxCard))
            m_c4Duration = c4Duration;

        //
        //  If we got a year, i.e. it's not max card anymore, and it's either lower
        //  than ours at this level, or ours is zero, i.e. not set, then take it.
        //
        if ((c4Year != kCIDLib::c4MaxCard)
        &&  ((c4Year < m_c4Year) || (m_c4Year == 0)))
        {
            m_c4Year = c4Year;
        }

        // If the artists weren't all the same, set to various
        if (bVariousArtists)
            m_strArtist = facCQCMedia().strMsg(kMedMsgs::midRepo_VariousArtists);
    }
}


// Return the item at the given index
const TMediaItem&
TMediaCollect::mitemAt(const TMediaDB& mdbSrc, const tCIDLib::TCard4 c4At) const
{
    if (c4At >= m_fcolItemIds.c4ElemCount())
    {
        facCQCMedia().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMedErrs::errcDB_BadMediaIndex
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::BadParms
            , TCardinal(c4At)
            , tCQCMedia::strXlatEDataTypes(tCQCMedia::EDataTypes::Collect)
            , strName()
        );
    }
    return *mdbSrc.pmitemById(eType(), m_fcolItemIds[c4At], kCIDLib::True);
}


// Remove a category id from our cat list
tCIDLib::TVoid TMediaCollect::RemoveCategory(const tCIDLib::TCard2 c2Id)
{
    const tCIDLib::TCard4 c4Count = m_fcolCatIds.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (m_fcolCatIds[c4Index] == c2Id)
        {
            m_fcolCatIds.Delete(c4Index);
            break;
        }
    }
}


// Remove an item id from our item list
tCIDLib::TVoid TMediaCollect::RemoveItem(const tCIDLib::TCard2 c2Id)
{
    const tCIDLib::TCard4 c4Count = m_fcolItemIds.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (m_fcolItemIds[c4Index] == c2Id)
        {
            m_fcolItemIds.Delete(c4Index);
            break;
        }
    }
}


// Get or set the alt info string
const TString& TMediaCollect::strAltInfo() const
{
    return m_strAltInfo;
}

const TString& TMediaCollect::strAltInfo(const TString& strToSet)
{
    m_strAltInfo = strToSet;
    return m_strAltInfo;
}


// Get/set the artist for this collection
const TString& TMediaCollect::strArtist() const
{
    return m_strArtist;
}

const TString& TMediaCollect::strArtist(const TString& strToSet)
{
    m_strArtist = strToSet;
    return m_strArtist;
}


// Get/set the aspect ratio for this collection, if applicable
const TString& TMediaCollect::strAspectRatio() const
{
    return m_strAspectRatio;
}

const TString& TMediaCollect::strAspectRatio(const TString& strToSet)
{
    m_strAspectRatio = strToSet;
    return m_strAspectRatio;
}


// Get/set the ASIN code
const TString& TMediaCollect::strASIN() const
{
    return m_strASIN;
}

const TString& TMediaCollect::strASIN(const TString& strToSet)
{
    m_strASIN = strToSet;
    return m_strASIN;
}


// Get/set the cast info for this collection
const TString& TMediaCollect::strCast() const
{
    return m_strCast;
}

const TString& TMediaCollect::strCast(const TString& strToSet)
{
    m_strCast = strToSet;
    return m_strCast;
}


// Get/set the descriptive text
const TString& TMediaCollect::strDescr() const
{
    return m_strDescr;
}

const TString& TMediaCollect::strDescr(const TString& strToSet)
{
    m_strDescr = strToSet;
    return m_strDescr;
}


// Get/set the label for this collection
const TString& TMediaCollect::strLabel() const
{
    return m_strLabel;
}

const TString& TMediaCollect::strLabel(const TString& strToSet)
{
    m_strLabel = strToSet;
    return m_strLabel;
}


// Get/set the location info
const TString& TMediaCollect::strLocInfo() const
{
    return m_strLocInfo;
}

const TString& TMediaCollect::strLocInfo(const TString& strToSet)
{
    m_strLocInfo = strToSet;
    return m_strLocInfo;
}


// Get/set the lead actor info for this collection
const TString& TMediaCollect::strLeadActor() const
{
    return m_strLeadActor;
}

const TString& TMediaCollect::strLeadActor(const TString& strToSet)
{
    m_strLeadActor = strToSet;
    return m_strLeadActor;
}


// Get/set the media format info for this collection
const TString& TMediaCollect::strMediaFormat() const
{
    return m_strMediaFmt;
}

const TString& TMediaCollect::strMediaFormat(const TString& strToSet)
{
    m_strMediaFmt = strToSet;
    return m_strMediaFmt;
}


// Get/set the poster art path for this collection
const TString& TMediaCollect::strPosterArt() const
{
    return m_strPosterArt;
}

const TString& TMediaCollect::strPosterArt(const TString& strToSet)
{
    m_strPosterArt= strToSet;
    return m_strPosterArt;
}


// Get/set the rating info for this collection
const TString& TMediaCollect::strRating() const
{
    return m_strRating;
}

const TString& TMediaCollect::strRating(const TString& strToSet)
{
    m_strRating = strToSet;
    return m_strRating;
}


// Get/set the UPC code
const TString& TMediaCollect::strUPC() const
{
    return m_strUPC;
}

const TString& TMediaCollect::strUPC(const TString& strToSet)
{
    m_strUPC = strToSet;
    return m_strUPC;
}


// Set our category list to a pre-looked up list of ids
tCIDLib::TVoid TMediaCollect::SetCatIds(const tCQCMedia::TIdList& fcolIds)
{
    m_fcolCatIds = fcolIds;
}


// Set our item list to a pre-looked up list of ids
tCIDLib::TVoid TMediaCollect::SetItemIds(const tCQCMedia::TIdList& fcolIds)
{
    m_fcolItemIds = fcolIds;
}


// Set the location and location type of this collection
tCIDLib::TVoid
TMediaCollect::SetLocation( const   tCQCMedia::ELocTypes    eLocType
                            , const TString&                strLocInfo)
{
    m_eLocType = eLocType;
    m_strLocInfo = strLocInfo;
}


//
//  Swap the position of two items. Since items ids are indirect, i.e. we gives out
//  indices not actual ids, this is just a matter of swapping within our list of ids.
//
tCIDLib::TVoid
TMediaCollect::SwapItems(const   tCIDLib::TCard4 c4At1, const tCIDLib::TCard4 c4At2)
{
    tCIDLib::TCard2 c2Save = m_fcolItemIds[c4At1];
    m_fcolItemIds[c4At1] = m_fcolItemIds[c4At2];
    m_fcolItemIds[c4At2] = c2Save;
}


// ---------------------------------------------------------------------------
//  TMediaCollect: Protected, inherited methods
// ---------------------------------------------------------------------------

//
//  Note that we do not stream the collection items here. They are not members of
//  us, we only reference them.
//
tCIDLib::TVoid TMediaCollect::StreamFrom(TBinInStream& strmToReadFrom)
{
    //
    //  We store an extra magic value at the start of media objects because
    //  they are flattened into their own files in our own repository and
    //  this helps to quickly check whether they are valid or not.
    //
    CheckMagicVal
    (
        CID_LINE, CQCMedia_DBInfo::c4MagicColVal, strmToReadFrom, L"collect"
    );

    // Looks ok so check for our marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCMedia_DBInfo::c2ColFmtVersion))
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

    // Call our parent class
    TParent::StreamFrom(strmToReadFrom);

    // Stream in our stuff and check the end marker
    strmToReadFrom  >> m_bAnamorphic
                    >> m_bIsPlayList
                    >> m_c4Duration
                    >> m_c4Year
                    >> m_eLocType
                    >> m_fcolCatIds
                    >> m_fcolItemIds
                    >> m_strAltInfo;

    //
    //  If version 1, then we have to build up the full artist name from the
    //  old first/last name format, else just read it in.
    //
    if (c2FmtVersion == 1)
    {
        TString strFirstName;
        TString strLastName;
        strmToReadFrom >> strFirstName >> strLastName;

        m_strArtist = strFirstName;
        if (!strFirstName.bIsEmpty() && !strLastName.bIsEmpty())
            m_strArtist.Append(kCIDLib::chSpace);
        m_strArtist.Append(strLastName);
    }
     else
    {
        strmToReadFrom >> m_strArtist;
    }

    strmToReadFrom  >> m_strAspectRatio
                    >> m_strASIN
                    >> m_strCast
                    >> m_strDescr
                    >> m_strLabel;

    //
    //  If version 1, then we have to build up the full actor name from the
    //  old first/last name format, else just read it in.
    //
    if (c2FmtVersion == 1)
    {
        TString strFirstName;
        TString strLastName;
        strmToReadFrom >> strFirstName >> strLastName;

        m_strLeadActor = strFirstName;
        if (!strFirstName.bIsEmpty() && !strLastName.bIsEmpty())
            m_strLeadActor.Append(kCIDLib::chSpace);
        m_strLeadActor.Append(strLastName);
    }
     else
    {
        strmToReadFrom >> m_strLeadActor;
    }

    strmToReadFrom  >> m_strLocInfo;

    // If version 3 or later, read in the media format, else default it
    if (c2FmtVersion > 2)
        strmToReadFrom >> m_strMediaFmt;
    else
        m_strMediaFmt.Clear();

    strmToReadFrom  >> m_strRating
                    >> m_strUPC;

    // If V4 or later, read in the post art, else default it
    if (c2FmtVersion > 3)
        strmToReadFrom >> m_strPosterArt;
    else
        m_strPosterArt.Clear();

    // And it should end with a frame market
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TMediaCollect::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Write out our magic value, marker, and version info
    strmToWriteTo   << CQCMedia_DBInfo::c4MagicColVal
                    << tCIDLib::EStreamMarkers::StartObject
                    << CQCMedia_DBInfo::c2ColFmtVersion;

    // Let our parent class write his stuff
    TParent::StreamTo(strmToWriteTo);

    // Write our stuff and our end marker
    strmToWriteTo   << m_bAnamorphic
                    << m_bIsPlayList
                    << m_c4Duration
                    << m_c4Year
                    << m_eLocType
                    << m_fcolCatIds
                    << m_fcolItemIds
                    << m_strAltInfo
                    << m_strArtist
                    << m_strAspectRatio
                    << m_strASIN
                    << m_strCast
                    << m_strDescr
                    << m_strLabel
                    << m_strLeadActor
                    << m_strLocInfo
                    << m_strMediaFmt
                    << m_strRating
                    << m_strUPC

                    // V4 stuff
                    << m_strPosterArt

                    << tCIDLib::EStreamMarkers::EndObject;
}


// ---------------------------------------------------------------------------
//  TMediaCollect: Protected, non-virtual methods
// ---------------------------------------------------------------------------

// Provide direct non-const access to our referenced items
TMediaItem& TMediaCollect::mitemAtNC(TMediaDB& mdbSrc, const tCIDLib::TCard4 c4At)
{
    if (c4At >= m_fcolItemIds.c4ElemCount())
    {
        facCQCMedia().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMedErrs::errcDB_BadMediaIndex
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::BadParms
            , TCardinal(c4At)
            , tCQCMedia::strXlatEDataTypes(tCQCMedia::EDataTypes::Collect)
            , strName()
        );
    }
    return *mdbSrc.pmitemByIdNC(eType(), m_fcolItemIds[c4At], kCIDLib::True);
}





// ---------------------------------------------------------------------------
//   CLASS: TMediaTitleSet
//  PREFIX: mts
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TMediaTitleSet: Public, static methods
// ---------------------------------------------------------------------------
tCIDLib::ESortComps
TMediaTitleSet::eCompByArtist(  const   TMediaTitleSet& mts1
                                , const TMediaTitleSet& mts2)
{
    return mts1.m_strArtist.eCompareI(mts2.m_strArtist);
}


tCIDLib::ESortComps
TMediaTitleSet::eCompByBitDepth(const   TMediaTitleSet& mts1
                                , const TMediaTitleSet& mts2)
{
    if (mts1.m_c4BitDepth < mts2.m_c4BitDepth)
        return tCIDLib::ESortComps::FirstLess;
    else if (mts1.m_c4BitDepth > mts2.m_c4BitDepth)
        return tCIDLib::ESortComps::FirstGreater;
    return tCIDLib::ESortComps::Equal;
}


tCIDLib::ESortComps
TMediaTitleSet::eCompByBitRate( const   TMediaTitleSet& mts1
                                , const TMediaTitleSet& mts2)
{
    if (mts1.m_c4BitRate < mts2.m_c4BitRate)
        return tCIDLib::ESortComps::FirstLess;
    else if (mts1.m_c4BitRate > mts2.m_c4BitRate)
        return tCIDLib::ESortComps::FirstGreater;
    return tCIDLib::ESortComps::Equal;
}


tCIDLib::ESortComps
TMediaTitleSet::eCompByChannels(const   TMediaTitleSet& mts1
                                , const TMediaTitleSet& mts2)
{
    if (mts1.m_c4Channels < mts2.m_c4Channels)
        return tCIDLib::ESortComps::FirstLess;
    else if (mts1.m_c4Channels > mts2.m_c4Channels)
        return tCIDLib::ESortComps::FirstGreater;
    return tCIDLib::ESortComps::Equal;
}


tCIDLib::ESortComps
TMediaTitleSet::eCompByDateAdded(const  TMediaTitleSet& mts1
                                , const TMediaTitleSet& mts2)
{
    if (mts1.m_enctAdded < mts2.m_enctAdded)
        return tCIDLib::ESortComps::FirstLess;
    else if (mts1.m_enctAdded > mts2.m_enctAdded)
        return tCIDLib::ESortComps::FirstGreater;
    return tCIDLib::ESortComps::Equal;
}


tCIDLib::ESortComps
TMediaTitleSet::eCompById(  const   TMediaTitleSet& mts1
                            , const TMediaTitleSet& mts2)
{
    const tCIDLib::TCard2 c2Id1 = mts1.c2Id();
    const tCIDLib::TCard2 c2Id2 = mts2.c2Id();
    if (c2Id1 < c2Id2)
        return tCIDLib::ESortComps::FirstLess;
    if (c2Id1 > c2Id2)
        return tCIDLib::ESortComps::FirstGreater;
    return tCIDLib::ESortComps::Equal;
}


tCIDLib::ESortComps
TMediaTitleSet::eCompBySampleRate( const    TMediaTitleSet& mts1
                                    , const TMediaTitleSet& mts2)
{
    if (mts1.m_c4SampleRate < mts2.m_c4SampleRate)
        return tCIDLib::ESortComps::FirstLess;
    else if (mts1.m_c4SampleRate > mts2.m_c4SampleRate)
        return tCIDLib::ESortComps::FirstGreater;
    return tCIDLib::ESortComps::Equal;
}


tCIDLib::ESortComps
TMediaTitleSet::eCompByUserRating(  const   TMediaTitleSet& mts1
                                    , const TMediaTitleSet& mts2)
{
    if (mts1.m_c4UserRating < mts2.m_c4UserRating)
        return tCIDLib::ESortComps::FirstLess;
    else if (mts1.m_c4UserRating > mts2.m_c4UserRating)
        return tCIDLib::ESortComps::FirstGreater;
    return tCIDLib::ESortComps::Equal;
}


tCIDLib::ESortComps
TMediaTitleSet::eCompBySeqNum(  const   TMediaTitleSet& mts1
                                , const TMediaTitleSet& mts2)
{
    if (mts1.m_c4SeqNum < mts2.m_c4SeqNum)
        return tCIDLib::ESortComps::FirstLess;
    else if (mts1.m_c4SeqNum > mts2.m_c4SeqNum)
        return tCIDLib::ESortComps::FirstGreater;
    return tCIDLib::ESortComps::Equal;
}


tCIDLib::ESortComps
TMediaTitleSet::eCompByTitle(const  TMediaTitleSet& mts1
                            , const TMediaTitleSet& mts2)
{
    return mts1.strName().eCompareI(mts2.strName());
}


tCIDLib::ESortComps
TMediaTitleSet::eCompBySortTitle(const  TMediaTitleSet& mts1
                                , const TMediaTitleSet& mts2)
{
    return mts1.m_strSortTitle.eCompareI(mts2.m_strSortTitle);
}


tCIDLib::ESortComps
TMediaTitleSet::eCompByYear(const   TMediaTitleSet& mts1
                            , const TMediaTitleSet& mts2)
{
    if (mts1.m_c4Year < mts2.m_c4Year)
        return tCIDLib::ESortComps::FirstLess;
    if (mts1.m_c4Year > mts2.m_c4Year)
        return tCIDLib::ESortComps::FirstGreater;
    return tCIDLib::ESortComps::Equal;
}


tCIDLib::ESortComps
TMediaTitleSet::eIdKeyComp( const   tCIDLib::TCard2&    c2Id
                            , const TMediaTitleSet&     mts2)
{
    const tCIDLib::TCard2 c2Id2 = mts2.c2Id();
    if (c2Id < c2Id2)
        return tCIDLib::ESortComps::FirstLess;
    if (c2Id > c2Id2)
        return tCIDLib::ESortComps::FirstGreater;
    return tCIDLib::ESortComps::Equal;
}

const TString& TMediaTitleSet::strKeyAccess(const TMediaTitleSet& mtsSrc)
{
    return mtsSrc.strName();
}


const TString& TMediaTitleSet::strUIDAccess(const TMediaTitleSet& mtsSrc)
{
    return mtsSrc.strUniqueId();
}


// ---------------------------------------------------------------------------
//  TMediaTitleSet: Constructors and Destructor
// ---------------------------------------------------------------------------
TMediaTitleSet::TMediaTitleSet(const tCQCMedia::EMediaTypes eType) :

    TMediaDBBase(eType)
    , m_bIsPlayList(kCIDLib::False)
    , m_c4BitDepth(0)
    , m_c4BitRate(0)
    , m_c4Channels(0)
    , m_c4SampleRate(0)
    , m_c4SeqNum(0)
    , m_c4UserRating(kCQCMedia::c4MinUserRating)
    , m_c4Year(0)
    , m_enctAdded(0)
    , m_fcolColIds(8UL)
{
}

TMediaTitleSet::TMediaTitleSet( const   TString&                strName
                                , const TString&                strUniqueId
                                , const TString&                strSortTitle
                                , const tCQCMedia::EMediaTypes  eType) :

    TMediaDBBase(strName, strUniqueId, eType)
    , m_bIsPlayList(kCIDLib::False)
    , m_c4BitDepth(0)
    , m_c4BitRate(0)
    , m_c4Channels(0)
    , m_c4SampleRate(0)
    , m_c4SeqNum(0)
    , m_c4UserRating(kCQCMedia::c4MinUserRating)
    , m_c4Year(0)
    , m_enctAdded(0)
    , m_fcolColIds(8UL)
    , m_strSortTitle(strSortTitle)
{
}

TMediaTitleSet::~TMediaTitleSet()
{
}


// ---------------------------------------------------------------------------
//  TMediaTitleSet: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TMediaTitleSet::Reset()
{
    m_bIsPlayList = kCIDLib::False;
    m_c4BitDepth = 0;
    m_c4BitRate = 0;
    m_c4Channels = 0;
    m_c4SampleRate = 0;
    m_c4SeqNum = 0;
    m_c4UserRating = kCQCMedia::c4MinUserRating;
    m_c4Year = 0;
    m_enctAdded = 0;
    m_fcolColIds.RemoveAll();
    m_strArtist.Clear();
    m_strSortTitle.Clear();
    TParent::Reset();
}


tCIDLib::TVoid TMediaTitleSet::ToXML(TTextOutStream& strmTar) const
{
    // Start the element for this guy and do the attributes at our level
    strmTar << L"<Titl"
            << L" Ana=\"" << (m_bIsPlayList ? L'Y' : L'N') << L"\"";

    if (m_c4BitDepth)
        strmTar << L" BitD=\"" << m_c4BitDepth << L"\"";

    if (m_c4BitRate)
        strmTar << L" BitR=\"" << m_c4BitDepth << L"\"";

    if (m_c4Channels)
        strmTar << L" BitD=\"" << m_c4Channels << L"\"";

    if (m_enctAdded)
    {
        strmTar << L" Added=\""
                << TCardinal64(m_enctAdded, tCIDLib::ERadices::Hex) << L"\"";
    }

    if (m_c4SampleRate)
        strmTar << L" SRate=\"" << m_c4SampleRate << L"\"";

    if (m_c4SeqNum)
        strmTar << L" SeqN=\"" << m_c4SeqNum << L"\"";

    if (m_c4UserRating != kCQCMedia::c4MinUserRating)
        strmTar << L" URat=\"" << m_c4UserRating << L"\"";

    if (m_c4Year)
        strmTar << L" Year=\"" << m_c4Year << L"\"";


    // These require escapement
    facCIDXML().FormatAttr(strmTar, L"Artist", m_strArtist, kCIDLib::True);
    facCIDXML().FormatAttr(strmTar, L"STitle", m_strSortTitle, kCIDLib::True);

    // Call our parent to let him do his stuff and close off the start element
    TParent::ToXML(strmTar);
    strmTar << L">";

    // These are done as child elements
    strmTar << L"<CCols>";
    const tCIDLib::TCard4 c4Count = m_fcolColIds.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        strmTar << m_fcolColIds[c4Index] << L' ';
    strmTar << L"</CCols>";

    // And do the close element
    strmTar << L"</Titl>\n";
}


// ---------------------------------------------------------------------------
//  TMediaTitleSet: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Adds the passed collection to our list of collections
tCIDLib::TBoolean TMediaTitleSet::bAddCollect(const TMediaCollect& mcolToAdd)
{
    // Has to be of our type, and non-zero id
    #if CID_DEBUG_ON
    if (mcolToAdd.eType() != eType())
    {
        facCQCMedia().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMedErrs::errcDB_BadMediaType
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::BadParms
            , tCQCMedia::strXlatEDataTypes(tCQCMedia::EDataTypes::TitleSet)
            , strName()
            , tCQCMedia::strXlatEMediaTypes(mcolToAdd.eType())
        );
    }

    if (!mcolToAdd.c2Id())
    {
        facCQCMedia().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMedErrs::errcDB_ZeroId
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::BadParms
            , tCQCMedia::strXlatEDataTypes(tCQCMedia::EDataTypes::TitleSet)
            , tCQCMedia::strXlatEMediaTypes(mcolToAdd.eType())
        );
    }
    #endif

    if (!m_fcolColIds.bElementPresent(mcolToAdd.c2Id()))
    {
        m_fcolColIds.c4AddElement(mcolToAdd.c2Id());
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


//
//  Indicate whether or not we own the passed collection. If so, we return the index
//  of it within us. That's what's used to create a cookie, not the actual id.
//
tCIDLib::TBoolean
TMediaTitleSet::bContainsCol(const tCIDLib::TCard2 c2ColId, tCIDLib::TCard2& c2ColInd) const
{
    tCIDLib::TCard4 c4At;
    if (m_fcolColIds.bElementPresent(c2ColId, c4At))
    {
        // Has to be able to fit in a Card2
        if (c4At > kCIDLib::c2MaxCard)
        {
            facCQCMedia().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kMedErrs::errcDB_BadMediaIndex
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Internal
                , TCardinal(c4At)
                , tCQCMedia::strXlatEDataTypes(tCQCMedia::EDataTypes::TitleSet)
                , strName()
            );
        }
        c2ColInd = tCIDLib::TCard2(c4At);
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


// Our playlist marking is based on whether any of our collections is one
tCIDLib::TBoolean TMediaTitleSet::bIsPlayList() const
{
    return m_bIsPlayList;
}


// Finds the indicated id and removes it from our list
tCIDLib::TBoolean
TMediaTitleSet::bRemoveColById( const   tCIDLib::TCard2     c2Id
                                , const tCIDLib::TBoolean   bThrowIfNot)
{
    const tCIDLib::TCard4 c4ColCount = m_fcolColIds.c4ElemCount();
    tCIDLib::TCard4 c4Index = 0;
    for (; c4Index < c4ColCount; c4Index++)
    {
        if (m_fcolColIds[c4Index] == c2Id)
        {
            m_fcolColIds.Delete(c4Index);
            break;
        }
    }

    if (c4Index == c4ColCount)
    {
        if (bThrowIfNot)
        {
            facCQCMedia().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kMedErrs::errcDB_UnknownColId
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::BadParms
                , TCardinal(c2Id, tCIDLib::ERadices::Hex)
                , strName()
            );
        }
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


// Returns the collection id at a given index
tCIDLib::TCard2 TMediaTitleSet::c2ColIdAt(const tCIDLib::TCard4 c4At) const
{
    if (c4At >= m_fcolColIds.c4ElemCount())
    {
        facCQCMedia().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMedErrs::errcDB_BadMediaIndex
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::BadParms
            , TCardinal(c4At)
            , tCQCMedia::strXlatEDataTypes(tCQCMedia::EDataTypes::TitleSet)
            , strName()
        );
    }
    return m_fcolColIds[c4At];
}


// Returns the number of collections in this title set
tCIDLib::TCard4 TMediaTitleSet::c4ColCount() const
{
    return m_fcolColIds.c4ElemCount();
}


tCIDLib::TCard4 TMediaTitleSet::c4SeqNum(const tCIDLib::TCard4 c4ToSet)
{
    m_c4SeqNum = c4ToSet;
    return m_c4SeqNum;
}


tCIDLib::TCard4 TMediaTitleSet::c4UserRating(const tCIDLib::TCard4 c4ToSet)
{
    CIDAssert(c4ToSet <= 10, L"Invalid user rating set");

    m_c4UserRating = c4ToSet;
    return m_c4UserRating;
}


// Get or set the added date
tCIDLib::TEncodedTime TMediaTitleSet::enctAdded() const
{
    return m_enctAdded;
}

tCIDLib::TEncodedTime
TMediaTitleSet::enctAdded(const tCIDLib::TEncodedTime enctAdded)
{
    m_enctAdded = enctAdded;
    return m_enctAdded;
}


//
//  Returns the first character to sort by, based on the passed sort
//  order value.
//
tCIDLib::TCh
TMediaTitleSet::chSortChar(const tCQCMedia::ESortOrders eOrder) const
{
    if (eOrder == tCQCMedia::ESortOrders::Artist)
    {
        if (!m_strArtist.bIsEmpty())
            return m_strArtist[0];
    }
     else if (eOrder == tCQCMedia::ESortOrders::Title)
    {
        //
        //  If we have a sort title, use that. Else, if we have a title, then
        //  use that.
        //
        if (!m_strSortTitle.bIsEmpty())
            return m_strSortTitle[0];
        else if (!strName().bIsEmpty())
            return strName()[0];
    }

    // Don't know how or have nothing to give, so return null
    return kCIDLib::chNull;
}


// Clears our collection id list
tCIDLib::TVoid TMediaTitleSet::ClearCollects()
{
    m_fcolColIds.RemoveAll();
}


//
//  This is called to give us a chance do any finalization we need to do
//  after all the data is loaded, or after underlying items or collections
//  have been modified. So it can be called more than once, and we have to
//  be prepared to deal with that, i.e. don't just add to values or accumulate,
//  but reset each time as necessary.
//
//  We create a sort title if one hasn't been set yet. And we aggregate some
//  info from our contained collections and items, stuff that we don't persist
//  ourself but we need it for sorting purposes. So we take the highest, lowest,
//  bestest, whatever of our contained content.
//
//  We take the earliest (non-zero) year, and the largest bit depth, sample
//  rate, and channels.
//
//  If the repo aggregates user rating stuff, then that will be now aggregated
//  at the collection level (else it'll be zero.) So, if the col level value
//  is larger than ours, take it. Else we'll stick with what was set at
//  the set level.
//
tCIDLib::TVoid TMediaTitleSet::Finalize(TMediaDB& mdbOwner)
{
    // This stuff is always aggregated so clear them initially
    m_bIsPlayList = kCIDLib::False;
    m_c4BitDepth = 0;
    m_c4Channels = 0;
    m_c4SampleRate = 0;

    // Some stuff that we may aggregate
    tCIDLib::TBoolean   bVariousArtists = kCIDLib::False;
    tCIDLib::TCard4     c4Year = kCIDLib::c4MaxCard;
    tCIDLib::TCard4     c4UserRating = 0;

    const TMediaCollect* pmcolPrev = 0;
    const tCIDLib::TCard4 c4ColCount = m_fcolColIds.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ColCount; c4Index++)
    {
        const tCIDLib::TCard2 c2CurId = m_fcolColIds[c4Index];
        const TMediaCollect* pmcolCur = mdbOwner.pmcolById(eType(), c2CurId, kCIDLib::False);
        if (pmcolCur)
        {
            // If any collection is a playlist, then we are
            if (pmcolCur->bIsPlayList())
                m_bIsPlayList = kCIDLib::True;

            // If the year is non-zero and lower than what we have so far, take it
            if (pmcolCur->c4Year() && (pmcolCur->c4Year() < c4Year))
                c4Year = pmcolCur->c4Year();

            //
            //  Take the artist of the first one. If we don't end up
            //  with the various artist's flag set, then they are all
            //  the same and we can keep this one.
            //
            if (!c4Index)
                m_strArtist = pmcolCur->strArtist();

            // See if all of the artists names are the same
            if (pmcolPrev
            &&  (pmcolPrev->strArtist() != pmcolCur->strArtist()))
            {
                bVariousArtists = kCIDLib::True;
            }

            // The rest we have to iterate the items
            const tCIDLib::TCard4 c4ItemCnt = pmcolCur->c4ItemCount();
            for (tCIDLib::TCard4 c4ItemInd = 0; c4ItemInd < c4ItemCnt; c4ItemInd++)
            {
                // If we find the item, process its info
                const tCIDLib::TCard2 c2Id = pmcolCur->c2ItemIdAt(c4ItemInd);
                const TMediaItem* pmitemCur = mdbOwner.pmitemById(eType(), c2Id, kCIDLib::False);
                if (!pmitemCur)
                    continue;

                if (pmitemCur->c4Rating() > c4UserRating)
                    c4UserRating = pmitemCur->c4Rating();

                if (pmitemCur->c4BitDepth() > m_c4BitDepth)
                    m_c4BitDepth = pmitemCur->c4BitDepth();

                if (pmitemCur->c4BitRate () > m_c4BitRate)
                    m_c4BitRate = pmitemCur->c4BitRate();

                if (pmitemCur->c4Channels() > m_c4Channels)
                    m_c4Channels = pmitemCur->c4Channels();

                if (pmitemCur->c4SampleRate() > m_c4SampleRate)
                    m_c4SampleRate = pmitemCur->c4SampleRate();
            }

            pmcolPrev = pmcolCur;
        }
    }

    //
    //  If we got a user rating, and it's higher than what we have set
    //  at this level, then take it.
    //
    if (c4UserRating && (c4UserRating > m_c4UserRating))
        m_c4UserRating = c4UserRating;

    //
    //  If we got a year, i.e. it's not max card anymore, and either it's lower
    //  than our year at this level, our or year is zero (never got set), then
    //  take it.
    //
    if ((c4Year != kCIDLib::c4MaxCard)
    &&  (!m_c4Year || (c4Year < m_c4Year)))
    {
        m_c4Year = c4Year;
    }

    // If the artists weren't all the same, set to various
    if (bVariousArtists)
        m_strArtist = facCQCMedia().strMsg(kMedMsgs::midRepo_VariousArtists);

    //
    //  If no sort title was set, then create one ourself. We also upper case
    //  it so that case differences are ignored for sorting purposes.
    //
    if (m_strSortTitle.bIsEmpty())
        facCQCMedia().bMakeSortTitle(strName(), m_strSortTitle);
    m_strSortTitle.ToUpper();
}


// Returns the collection at a given index within us
const TMediaCollect&
TMediaTitleSet::mcolAt( const   TMediaDB&       mdbSrc
                        , const tCIDLib::TCard4 c4At) const
{
    if (c4At >= m_fcolColIds.c4ElemCount())
    {
        facCQCMedia().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMedErrs::errcDB_BadMediaIndex
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::BadParms
            , TCardinal(c4At)
            , tCQCMedia::strXlatEDataTypes(tCQCMedia::EDataTypes::TitleSet)
            , strName()
        );
    }
    return *mdbSrc.pmcolById(eType(), m_fcolColIds[c4At], kCIDLib::True);
}


// Removes the collection at the indicated index
tCIDLib::TVoid TMediaTitleSet::RemoveColAt(const tCIDLib::TCard4 c4At)
{
    if (c4At >= m_fcolColIds.c4ElemCount())
    {
        facCQCMedia().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMedErrs::errcDB_BadMediaIndex
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::BadParms
            , TCardinal(c4At)
            , tCQCMedia::strXlatEDataTypes(tCQCMedia::EDataTypes::TitleSet)
            , strName()
        );
    }
    m_fcolColIds.Delete(c4At);
}


// Get or set the artist for this title set
const TString& TMediaTitleSet::strArtist() const
{
    return m_strArtist;
}

const TString& TMediaTitleSet::strArtist(const TString& strToSet)
{
    m_strArtist = strToSet;
    return m_strArtist;
}


// Get/set the sort title text
const TString& TMediaTitleSet::strSortTitle() const
{
    return m_strSortTitle;
}

const TString& TMediaTitleSet::strSortTitle(const TString& strToSet)
{
    m_strSortTitle = strToSet;
    return m_strSortTitle;
}


//
//  Replaces the current list of collection ids with the passed list.
//
tCIDLib::TVoid TMediaTitleSet::SetColIds(const tCQCMedia::TIdList& fcolIds)
{
    m_fcolColIds = fcolIds;
}


//
//  Swap the position of two collections. Since collection ids are indirect, i.e. we
//  gives out indices not actual ids, this is just a matter of swapping within our
//  list of ids.
//
tCIDLib::TVoid
TMediaTitleSet::SwapCollects(const  tCIDLib::TCard4 c4At1
                            , const tCIDLib::TCard4 c4At2)
{
    tCIDLib::TCard2 c2Save = m_fcolColIds[c4At1];
    m_fcolColIds[c4At1] = m_fcolColIds[c4At2];
    m_fcolColIds[c4At2] = c2Save;
}


// ---------------------------------------------------------------------------
//  TMediaTitleSet: Protected, inherited methods
// ---------------------------------------------------------------------------

//
//  Note that we do not stream the collection items here. They are not members of
//  us, we only reference them.
//
tCIDLib::TVoid TMediaTitleSet::StreamFrom(TBinInStream& strmToReadFrom)
{
    //
    //  We store an extra magic value at the start of media objects because
    //  they are flattened into their own files in our own repository and
    //  this helps to quickly check whether they are valid or not.
    //
    CheckMagicVal
    (
        CID_LINE, CQCMedia_DBInfo::c4MagicTitleVal, strmToReadFrom, L"title set"
    );

    // Looks ok so check for our start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCMedia_DBInfo::c2TitleSetFmtVersion))
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

    // Call our parent class
    TParent::StreamFrom(strmToReadFrom);

    //
    //  Stream in our stuff and check the end marker. We have kind of a wierd
    //  one here because we removed the first/last artist names and the sort
    //  title in V2, but we had to bring back the sort title for V3. So, if
    //  someone hasn't upgraded to V2 yet, we can get back the original sort
    //  title since it was never removed. Else we have to build up if we are
    //  on V2. If on V3, then it's up to date data.
    //
    //  We stopped persisting the year in V4, just creating it via aggregation
    //  from our contained info upon load/edits, so jus read it into a dummy
    //  value. We can reuse it at some point later perhaps.
    //
    if (c2FmtVersion < 4)
    {
        tCIDLib::TCard4 c4DummyYear;
        strmToReadFrom  >> c4DummyYear;
    }

    if (c2FmtVersion < 2)
    {
        TString strArtistFN;
        TString strArtistLN;
        strmToReadFrom  >> m_strSortTitle
                        >> m_fcolColIds
                        >> strArtistFN
                        >> strArtistLN;

        // Build up the new single artist field from the old f/l names
        m_strArtist = strArtistFN;
        if (!strArtistFN.bIsEmpty() && !strArtistLN.bIsEmpty())
            m_strArtist.Append(kCIDLib::chSpace);
        m_strArtist.Append(strArtistLN);
    }
     else if (c2FmtVersion == 2)
    {
        strmToReadFrom  >> m_fcolColIds
                        >> m_strArtist;

        // We don't have a sort title in this version, so make one
        facCQCMedia().bMakeSortTitle(strName(), m_strSortTitle);
        m_strSortTitle.ToUpper();
    }
     else
    {
        strmToReadFrom  >> m_strSortTitle
                        >> m_fcolColIds
                        >> m_strArtist;
    }

    //
    //  If V4 or greater, read in the date added and user rating, else default
    //  them.
    //
    if (c2FmtVersion >= 4)
    {
        strmToReadFrom  >> m_c4UserRating
                        >> m_enctAdded;
    }
     else
    {
        m_c4UserRating = kCQCMedia::c4MinUserRating;
        m_enctAdded = 0;
    }

    // And our stuff should end with an end marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);

    // Initialize any non-persisted stuff
    m_bIsPlayList = kCIDLib::False;
    m_c4BitDepth = 0;
    m_c4Channels = 0;
    m_c4SampleRate = 0;
    m_c4Year = 0;
}


tCIDLib::TVoid TMediaTitleSet::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Write out our magic value, marker, and version info
    strmToWriteTo   << CQCMedia_DBInfo::c4MagicTitleVal
                    << tCIDLib::EStreamMarkers::StartObject
                    << CQCMedia_DBInfo::c2TitleSetFmtVersion;

    // Let our parent class write his stuff
    TParent::StreamTo(strmToWriteTo);

    //  Write our stuff and our end marker
    strmToWriteTo   << m_strSortTitle
                    << m_fcolColIds
                    << m_strArtist

                    << m_c4UserRating
                    << m_enctAdded

                    << tCIDLib::EStreamMarkers::EndObject;
}


// ---------------------------------------------------------------------------
//  TMediaTitleSet: Protected, non-virtual methods
// ---------------------------------------------------------------------------

// Provide non-const direct access to our referenced collections
TMediaCollect&
TMediaTitleSet::mcolAtNC(TMediaDB& mdbSrc, const tCIDLib::TCard4 c4At)
{
    if (c4At >= m_fcolColIds.c4ElemCount())
    {
        facCQCMedia().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMedErrs::errcDB_BadMediaIndex
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::BadParms
            , TCardinal(c4At)
            , tCQCMedia::strXlatEDataTypes(tCQCMedia::EDataTypes::TitleSet)
            , strName()
        );
    }
    return *mdbSrc.pmcolByIdNC(eType(), m_fcolColIds[c4At], kCIDLib::True);
}




// ---------------------------------------------------------------------------
//   CLASS: TArtistMap
//  PREFIX: mamap
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TArtistMap: Public static methods
// ---------------------------------------------------------------------------
tCIDLib::ESortComps
TArtistMap::eComp(  const   TArtistMap&     mamap1
                    , const TArtistMap&     mamap2)
{
    return mamap1.m_strArtistName.eCompare(mamap2.m_strArtistName);
}

tCIDLib::ESortComps
TArtistMap::eCompByKey( const   TString&        strArtistName
                        , const TArtistMap&     mamap2)
{
    return strArtistName.eCompare(mamap2.m_strArtistName);
}


// -------------------------------------------------------------------
//  TArtistMap: Constructors and Destructor
// -------------------------------------------------------------------
TArtistMap::TArtistMap() :

    m_colTitleSets(tCIDLib::EAdoptOpts::NoAdopt, 32)
{
}

TArtistMap::TArtistMap(const TString& strArtistName) :

    m_colTitleSets(tCIDLib::EAdoptOpts::NoAdopt, 32)
    , m_strArtistName(strArtistName)
{
}

TArtistMap::TArtistMap(const TArtistMap& mamapSrc) :

    m_colTitleSets(tCIDLib::EAdoptOpts::NoAdopt, mamapSrc.m_colTitleSets.c4ElemCount())
{
    mamapSrc.m_colTitleSets.DupPointers(m_colTitleSets);
}

TArtistMap::~TArtistMap()
{
}


// -------------------------------------------------------------------
//  TArtistMap: Public operators
// -------------------------------------------------------------------
TArtistMap& TArtistMap::operator=(const TArtistMap& mamapSrc)
{
    if (this != &mamapSrc)
    {
        mamapSrc.m_colTitleSets.DupPointers(m_colTitleSets);
        m_strArtistName = mamapSrc.m_strArtistName;
    }
    return *this;
}



// -------------------------------------------------------------------
//  TArtistMap: Public methods
// -------------------------------------------------------------------
tCIDLib::TVoid TArtistMap::AddTitleSet(const TMediaTitleSet* const pmtsToAdd)
{
    m_colTitleSets.Add(pmtsToAdd);
}


const TArtistMap::TSetList& TArtistMap::colSets() const
{
    return m_colTitleSets;
}


// We have all the title sets, so we can sort now
tCIDLib::TVoid TArtistMap::Complete()
{
    m_colTitleSets.Sort(TMediaTitleSet::eCompBySortTitle);
}


const TString& TArtistMap::strArtistName() const
{
    return m_strArtistName;
}

const TString& TArtistMap::strArtistName(const TString& strToSet)
{
    m_strArtistName = strToSet;
    return m_strArtistName;
}

