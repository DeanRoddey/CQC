//
// FILE NAME: CQCMedia_Database.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/23/2005
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
//  This is the implementation file for the in-memory database class.
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
RTTIDecls(TMediaDB,TObject)


// ---------------------------------------------------------------------------
//  Local types and constants
// ---------------------------------------------------------------------------
namespace CQCMedia_Database
{
    namespace
    {
        // -----------------------------------------------------------------------
        //  Our persistent format version
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2FmtVersion = 1;
    }
}





// ---------------------------------------------------------------------------
//   CLASS: TMediaCatFilt
//  PREFIX: mdbf
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TMediaCatFilt: Constructors and Destructor
// ---------------------------------------------------------------------------
TMediaCatFilt::TMediaCatFilt() noexcept
{
}

TMediaCatFilt::~TMediaCatFilt()
{
}

// ---------------------------------------------------------------------------
//  TMediaCatFilt: Public, virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TMediaCatFilt::bTest(const TMediaCat&) const
{
    // We are justa null filter
    return kCIDLib::True;
}




// ---------------------------------------------------------------------------
//   CLASS: TMediaColFilt
//  PREFIX: mdbf
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TMediaColFilt: Constructors and Destructor
// ---------------------------------------------------------------------------
TMediaColFilt::TMediaColFilt() noexcept
{
}

TMediaColFilt::~TMediaColFilt()
{
}

// ---------------------------------------------------------------------------
//  TMediaColFilt: Public, virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TMediaColFilt::bTest(const TMediaCollect&) const
{
    // We are justa null filter
    return kCIDLib::True;
}



// ---------------------------------------------------------------------------
//   CLASS: TMediaItemFilt
//  PREFIX: mdbf
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TMediaItemFilt: Constructors and Destructor
// ---------------------------------------------------------------------------
TMediaItemFilt::TMediaItemFilt() noexcept
{
}

TMediaItemFilt::~TMediaItemFilt()
{
}

// ---------------------------------------------------------------------------
//  TMediaItemFilt: Public, virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TMediaItemFilt::bTest(const TMediaItem&) const
{
    // We are justa null filter
    return kCIDLib::True;
}




// ---------------------------------------------------------------------------
//   CLASS: TMediaSetFilt
//  PREFIX: mdbf
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TMediaSetFilt: Constructors and Destructor
// ---------------------------------------------------------------------------
TMediaSetFilt::TMediaSetFilt() noexcept
{
}

TMediaSetFilt::~TMediaSetFilt()
{
}

// ---------------------------------------------------------------------------
//  TMediaSetFilt: Public, virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TMediaSetFilt::bTest(const TMediaTitleSet&) const
{
    // We are a null filter
    return kCIDLib::True;
}




// ---------------------------------------------------------------------------
//   CLASS: TMediaDB
//  PREFIX: mdb
// ---------------------------------------------------------------------------


// ---------------------------------------------------------------------------
//  TMediaDB: Public, static methods
// ---------------------------------------------------------------------------

//
//  In order to keep the binary dump format stuff hidden away, we provide a parsing
//  method to format out or parse binary dumps. The format writes it out uncompressed
//  to the target stream.
//
//  The parser takes a buffer, which can be the compressed or uncompressed format
//  and parses the info out.
//
//  We also have a special case one where they want to just decompress it but not
//  expand it out. THe client service needs this, and we'd have to expose the details
//  of our the compressed binary dump is stored.
//
tCIDLib::TCard4
TMediaDB::c4FormatBinDump(          TBinOutStream&          strmTar
                            , const tCQCMedia::EMTFlags&    eMTFlags
                            , const TString&                strSerNum
                            , const TMediaDB&               mdbSrc)
{
    // Make sure the stream is reset
    strmTar.Reset();

    // And format out the content and flush it
    strmTar << kCQCMedia::c2BinMDBDumpFmtVer
            << strSerNum
            << eMTFlags
            << mdbSrc
            << kCIDLib::FlushIt;

    // Give back the resulting byte count
    return strmTar.c4CurPos();
}


//
//  Decompress the passed binary dump (assumed to be comrpessed of course) to the
//  target stream. The caller must reset the stream if that is intended. Else we
//  just output to where it is now.
//
tCIDLib::TCard4
TMediaDB::c4DecompBinDump(  const   TMemBuf&            mbufSrc
                            , const tCIDLib::TCard4&    c4SrcSz
                            ,       TBinOutStream&      strmTar)
{
    // Set up the source stream
    TBinMBufInStream strmSrc(&mbufSrc, c4SrcSz);

    //
    //  There are two card4 values at the start with the raw size and the XOR'd
    //  raw size. Get themout and check them.
    //
    tCIDLib::TCard4 c4RawBytes, c4XORBytes;
    strmSrc >> c4RawBytes >> c4XORBytes;
    if (c4RawBytes != (c4XORBytes ^ kCIDLib::c4MaxCard))
    {
        facCQCMedia().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMedErrs::errcMDBC_BadRawDBSize
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
        );
    }

    TZLibCompressor zlibComp;
    return zlibComp.c4Decompress(strmSrc, strmTar);
}


//
//  Compress the passed binary dump (assumed to be uncompressed of course) to the
//  target stream. The caller must reset the stream if that is intended. Else we
//  just output to where it is now.
//
tCIDLib::TCard4
TMediaDB::c4CompBinDump(const   TMemBuf&            mbufSrc
                        , const tCIDLib::TCard4&    c4SrcSz
                        ,       TBinOutStream&      strmTar)
{
    // Set up the source stream
    TBinMBufInStream strmSrc(&mbufSrc, c4SrcSz);

    // Put out the decompressed size and XOR'd version of it to the target first
    strmTar << c4SrcSz << (c4SrcSz ^ kCIDLib::c4MaxCard);

    //
    //  And now compress into the buffer after that. Be sure to add the two exta values
    //  to the compressed data size.
    //
    TZLibCompressor zlibComp;
    return zlibComp.c4Compress(strmSrc, strmTar) + (sizeof(c4SrcSz) * 2);
}


tCIDLib::TVoid
TMediaDB::ParseBinDump( const   TMemBuf&                mbufData
                        , const tCIDLib::TCard4         c4DataSz
                        ,       tCQCMedia::EMTFlags&    eMTFlags
                        ,       TString&                strSerNum
                        ,       TMediaDB&               mdbToFill
                        , const tCIDLib::TBoolean       bCompressed)
{
    tCIDLib::TCard2 c2FmtVer;
    if (bCompressed)
    {
        TBinMBufInStream strmRaw(&mbufData, c4DataSz);

        //
        //  There are two card4 values at the start with the raw size and the XOR'd
        //  raw size. Get themout and check them. This also moves the source stream
        //  past them and leaves it ready for decompression.
        //
        tCIDLib::TCard4 c4RawBytes, c4XORBytes;
        strmRaw >> c4RawBytes >> c4XORBytes;
        if (c4RawBytes != (c4XORBytes ^ kCIDLib::c4MaxCard))
        {
            facCQCMedia().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kMedErrs::errcMDBC_BadRawDBSize
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
            );
        }

        //
        //  Use that to allocate our decompression buffer, with a little spare
        //  to be safe.
        //
        THeapBuf mbufDecomp(c4RawBytes + 1024, c4RawBytes + 1024);
        TZLibCompressor zlibComp;
        TBinMBufOutStream strmTar(&mbufDecomp);

        // And now decompress, and stream out the data from that
        const tCIDLib::TCard4 c4DecompSz = zlibComp.c4Decompress(strmRaw, strmTar);
        TBinMBufInStream strmSrc(&mbufDecomp, c4DecompSz);
        strmSrc >> c2FmtVer
                >> strSerNum
                >> eMTFlags
                >> mdbToFill;

    }
     else
    {
        // Stream the data directly from the incoming buffer
        TBinMBufInStream strmSrc(&mbufData, c4DataSz);
        strmSrc >> c2FmtVer
                >> strSerNum
                >> eMTFlags
                >> mdbToFill;
    }
}


// ---------------------------------------------------------------------------
//  TMediaDB: Constructors and Destructor
// ---------------------------------------------------------------------------
TMediaDB::TMediaDB() :

    m_bComplete(kCIDLib::False)
    , m_colByArtist(tCIDLib::EAdoptOpts::Adopt, tCQCMedia::EMediaTypes::Count)
    , m_colCats(tCIDLib::EAdoptOpts::Adopt, tCQCMedia::EMediaTypes::Count)
    , m_colColsById(tCIDLib::EAdoptOpts::Adopt, tCQCMedia::EMediaTypes::Count)
    , m_colColsByUID(tCIDLib::EAdoptOpts::Adopt, tCQCMedia::EMediaTypes::Count)
    , m_colImgsById(tCIDLib::EAdoptOpts::Adopt, tCQCMedia::EMediaTypes::Count)
    , m_colImgsByUID(tCIDLib::EAdoptOpts::Adopt, tCQCMedia::EMediaTypes::Count)
    , m_colItemsById(tCIDLib::EAdoptOpts::Adopt, tCQCMedia::EMediaTypes::Count)
    , m_colItemsByUID(tCIDLib::EAdoptOpts::Adopt, tCQCMedia::EMediaTypes::Count)
    , m_colSetsById(tCIDLib::EAdoptOpts::Adopt, tCQCMedia::EMediaTypes::Count)
    , m_colSetsByUID(tCIDLib::EAdoptOpts::Adopt, tCQCMedia::EMediaTypes::Count)
    , m_objaUnknownCols(tCQCMedia::EMediaTypes::Count)
    , m_objaUnknownItems(tCQCMedia::EMediaTypes::Count)
    , m_objaAllCatCookies(tCQCMedia::EMediaTypes::Count)
{
    // Do Common Setup
    DoCommonInit();
}


TMediaDB::TMediaDB(const TMediaDB& mdbSrc) :

    m_bComplete(mdbSrc.m_bComplete)
    , m_colByArtist(tCIDLib::EAdoptOpts::Adopt, tCQCMedia::EMediaTypes::Count)
    , m_colCats(tCIDLib::EAdoptOpts::Adopt, tCQCMedia::EMediaTypes::Count)
    , m_colColsById(tCIDLib::EAdoptOpts::Adopt, tCQCMedia::EMediaTypes::Count)
    , m_colColsByUID(tCIDLib::EAdoptOpts::Adopt, tCQCMedia::EMediaTypes::Count)
    , m_colImgsById(tCIDLib::EAdoptOpts::Adopt, tCQCMedia::EMediaTypes::Count)
    , m_colImgsByUID(tCIDLib::EAdoptOpts::Adopt, tCQCMedia::EMediaTypes::Count)
    , m_colItemsById(tCIDLib::EAdoptOpts::Adopt, tCQCMedia::EMediaTypes::Count)
    , m_colItemsByUID(tCIDLib::EAdoptOpts::Adopt, tCQCMedia::EMediaTypes::Count)
    , m_colSetsById(tCIDLib::EAdoptOpts::Adopt, tCQCMedia::EMediaTypes::Count)
    , m_colSetsByUID(tCIDLib::EAdoptOpts::Adopt, tCQCMedia::EMediaTypes::Count)
    , m_objaUnknownCols(tCQCMedia::EMediaTypes::Count)
    , m_objaUnknownItems(tCQCMedia::EMediaTypes::Count)
    , m_objaAllCatCookies(tCQCMedia::EMediaTypes::Count)
{
    // Do Common Setup
    DoCommonInit();

    // And duplicate the source contents
    DupDB(mdbSrc);
}


TMediaDB::~TMediaDB()
{
}


// ---------------------------------------------------------------------------
//  TMediaDB: Public operators
// ---------------------------------------------------------------------------

// We just duplicate the contents of teh source database.
TMediaDB& TMediaDB::operator=(const TMediaDB& mdbSrc)
{
    if (this != &mdbSrc)
    {
        m_bComplete = mdbSrc.m_bComplete;

        // Call the dup helper to copy over the data
        DupDB(mdbSrc);
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TMediaDB: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  For those repos that really just get a list of tracks, and have to figure out
//  artist names based on the tracks that ended up being grouped together somehow,
//  we provide this helper. We'll go through the items of each collection and see if
//  they are the same. If so, we set the collection artist name to that name. Else
//  we set it to Various Artists.
//
//  Then we do the same for titles relative collections.
//
tCIDLib::TVoid TMediaDB::AccumArtistNames(const tCQCMedia::EMediaTypes eMType)
{
    const tCIDLib::TCard4 c4ColCnt = c4CollectCnt(eMType);
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ColCnt; c4Index++)
    {
        //
        //  If all of the artist names are the same, then we just use
        //  that. Else, set it to "Various Artists"
        //
        TMediaCollect& mcolCur = mcolAtNC(eMType, c4Index);
        const tCIDLib::TCard4 c4ItemCnt = mcolCur.c4ItemCount();
        TMediaItem* pmitemPrev = pmitemByIdNC(eMType, mcolCur.c2ItemIdAt(0), kCIDLib::True);

        TMediaItem* pmitemCur = 0;
        tCIDLib::TCard4 c4IIndex = 1;
        for (; c4IIndex < c4ItemCnt; c4IIndex++)
        {
            pmitemCur = pmitemByIdNC(eMType, mcolCur.c2ItemIdAt(c4IIndex), kCIDLib::True);
            if (pmitemCur->strArtist() != pmitemPrev->strArtist())
                break;
            pmitemPrev = pmitemCur;
        }

        if (c4IIndex == c4ItemCnt)
            mcolCur.strArtist(pmitemPrev->strArtist());
        else
            mcolCur.strArtist(L"Various Artists");
    }

    const tCIDLib::TCard4 c4SetCnt = c4TitleSetCnt(eMType);
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4SetCnt; c4Index++)
    {
        TMediaTitleSet& mtsCur = mtsAtNC(eMType, c4Index);
        const tCIDLib::TCard4 c4ColCnt = mtsCur.c4ColCount();

        TMediaCollect* pmcolPrev = pmcolByIdNC(eMType, mtsCur.c2ColIdAt(0), kCIDLib::True);
        TMediaCollect* pmcolCur = 0;
        tCIDLib::TCard4 c4IIndex = 1;
        for (; c4IIndex < c4ColCnt; c4IIndex++)
        {
            pmcolCur = pmcolByIdNC(eMType, mtsCur.c2ColIdAt(c4IIndex), kCIDLib::True);
            if (pmcolCur->strArtist() != pmcolPrev->strArtist())
                break;
            pmcolPrev = pmcolCur;
        }

        if (c4IIndex == c4ColCnt)
            mtsCur.strArtist(pmcolPrev->strArtist());
        else
            mtsCur.strArtist(L"Various Artists");
    }
}



//
//  These are helpers to allow some common operations that would otherwise require
//  getting objects out, updating them, and putting them back via UpdateXXX()
//  methods. They can tell us where to insert the item, so as to keep the track
//  order correct. Or just pass max card to have it added at the end.
//
tCIDLib::TVoid
TMediaDB::AddColItem(const  tCIDLib::TCard2         c2ColId
                    , const tCQCMedia::EMediaTypes  eMType
                    , const TMediaItem&             mitemToAdd
                    , const tCIDLib::TCard4         c4At)
{
    TMediaCollect* pmcolTar = pmcolByIdNC(eMType, c2ColId, kCIDLib::True);
    pmcolTar->bAddItem(mitemToAdd, c4At);
}


tCIDLib::TVoid
TMediaDB::AddColToCat(  const   tCIDLib::TCard2         c2ColId
                        , const tCIDLib::TCard2         c2CatId
                        , const tCQCMedia::EMediaTypes  eMType)
{
    // Make sure the category exists
    pmcatById(eMType, c2CatId, kCIDLib::True);

    // And get the collection and update it
    TMediaCollect* pmcolTar = pmcolByIdNC(eMType, c2ColId, kCIDLib::True);
    pmcolTar->bAddCategory(c2CatId);
}


tCIDLib::TVoid
TMediaDB::AddTitleCol(  const   tCIDLib::TCard2         c2SetId
                        , const tCQCMedia::EMediaTypes  eMType
                        , const TMediaCollect&          mcolToAdd)
{
    TMediaTitleSet* pmtsTar = pmtsByIdNC(eMType, c2SetId, kCIDLib::True);
    pmtsTar->bAddCollect(mcolToAdd);
}


//
//  Add an item to a playlist collection. Playlists don't own items, they just
//  reference them.
//
//  The collection must be marked as a playlist or we will reject it. And the item
//  must already be present in the database.
//
//  Return whether we added it or not. If so, we need to re-finalize the playlist
//  collection and its parent, which we have to look up.
//
tCIDLib::TBoolean
TMediaDB::bAddItemToPL( const   tCIDLib::TCard2         c2PLColId
                        , const tCQCMedia::EMediaTypes  eMType
                        , const tCIDLib::TCard2         c2ItemId)
{
    // Only music is supported right now
    if (eMType != tCQCMedia::EMediaTypes::Music)
        return kCIDLib::False;

    // Get the collection, whic has to exist, and it has to be a playlist
    TMediaCollect* pmcolTar = pmcolByIdNC(eMType, c2PLColId, kCIDLib::True);
    if (!pmcolTar->bIsPlayList())
    {
        facCQCMedia().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMedErrs::errcDB_NotAPlayList
            , pmcolTar->strName()
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Internal
        );
    }

    //
    //  Make sure the passed item is in the database. If so, and it doens't already
    //  contain this item, add it.
    //
    TMediaItem* pmitemAdd = pmitemByIdNC(eMType, c2ItemId, kCIDLib::True);
    tCIDLib::TCard2 c2Index;
    if (!pmcolTar->bContainsItem(pmitemAdd->c2Id(), c2Index))
    {
        // It doesn't so add it
        pmcolTar->bAddItem(*pmitemAdd);

        // Allow the collection and its parent set to re-finalize
        pmcolTar->Finalize(*this);

        pmtsFindColParentNC(*pmcolTar, c2Index, kCIDLib::True)->Finalize(*this);

        return kCIDLib::True;
    }
    return kCIDLib::False;
}


//
//  The same as above, but in this case we are adding all of the items of a collection
//  to the target playlist.
//
tCIDLib::TBoolean
TMediaDB::bAddColToPL(  const   tCIDLib::TCard2         c2PLColId
                        , const tCQCMedia::EMediaTypes  eMType
                        , const tCIDLib::TCard2         c2SrcColId)
{
    // Only music is supported right now
    if (eMType != tCQCMedia::EMediaTypes::Music)
        return kCIDLib::False;

    // Make sure the source collection is in the database
    const TMediaCollect* pmcolSrc = pmcolById(eMType, c2SrcColId, kCIDLib::True);

    // If this guys is himself a playlist, ignore this. Caller should check for this
    if (pmcolSrc->bIsPlayList())
        return kCIDLib::False;

    //
    //  Get the target collection, which has to exist, and has to be music type and
    //  it has to be a playlist.
    //
    TMediaCollect* pmcolTar = pmcolByIdNC(eMType, c2PLColId, kCIDLib::True);
    if (!pmcolTar->bIsPlayList())
    {
        facCQCMedia().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMedErrs::errcDB_NotAPlayList
            , pmcolTar->strName()
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Internal
        );
    }

    //
    //  Loop through all the items and add if not already present. Remember if
    //  we added any.
    //
    tCIDLib::TBoolean bRet = kCIDLib::False;
    const tCIDLib::TCard4 c4Count = pmcolSrc->c4ItemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const tCIDLib::TCard2 c2CurItem = pmcolSrc->c2ItemIdAt(c4Index);

        // Make sure the list doesn't already contains this item
        tCIDLib::TCard2 c2ItemInd;
        if (!pmcolTar->bContainsItem(c2CurItem, c2ItemInd))
        {
            const TMediaItem* pmitemCur = pmitemById(eMType, c2CurItem, kCIDLib::True);
            pmcolTar->bAddItem(*pmitemCur);
            bRet = kCIDLib::True;
        }
    }

    // Allow the collection and set to refinalize if we added any
    if (bRet)
    {
        pmcolTar->Finalize(*this);

        tCIDLib::TCard2 c2Index;
        pmtsFindColParentNC(*pmcolTar, c2Index, kCIDLib::True)->Finalize(*this);
    }
    return bRet;
}


//
//  When a database is newlly loaded, this is usually called to make sure that all
//  of the internal media file paths (the files we ripped and therefore are always
//  relative to our repo path) are relative to the repository base path. In our case
//  the stuff we want to update are the music files, which currently have to be ones
//  that we ripped, and the files are item level.
//
//  We have to figure out the base part of the current path, and then replace it
//  with the passed base path.
//
tCIDLib::TBoolean TMediaDB::bAdjustIntMediaPaths(const TString& strBasePath)
{
    //
    //  Create the actual base path we will use here, which is the music item sub
    //  dir under the passed base. Any files that don't start with this will we
    //  remove the current pathup to this same point, then insert this before the
    //  remainder.
    //
    const TString strFindSub(L"\\ItDat\\Music");
    TPathStr pathOurBase = strBasePath;
    pathOurBase.AddLevel(strFindSub);

    tCIDLib::TCard4 c4ModCnt = 0;
    tCIDLib::TCard4 c4At;
    TString         strCur;

    const tCIDLib::TCard4 c4Count = c4ItemCnt(tCQCMedia::EMediaTypes::Music);
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TMediaItem& mitemCur = mitemAtNC(tCQCMedia::EMediaTypes::Music, c4Index);
        const TString& strLocInfo = mitemCur.strLocInfo();

        // If this one starts with our base path, it's ok as is
        if (strLocInfo.bStartsWithI(pathOurBase))
            continue;

        // This one doesn't start with our base path, so update
        c4At = 0;
        strCur = strLocInfo;

        //
        //  Do a search for the find substring. If found, then then the end of that
        //  sub-string is the point we want to cut up to.
        //
        if (strCur.bFirstOccurrence(strFindSub, c4At, kCIDLib::False))
        {
            // Cut out up to the end of this
            strCur.Cut(0, c4At + strFindSub.c4Length());
            strCur.Insert(pathOurBase, 0);

            mitemCur.strLocInfo(strCur);
            c4ModCnt++;
        }
    }
    return (c4ModCnt != 0);
}


//
//  Checks to see if the passed category has at least one title set
//  assigned to it.
//
tCIDLib::TBoolean TMediaDB::bCatHasSomeSets(const TMediaCat& mcatToCheck) const
{
    const tCQCMedia::TNCSetIdList& colById = *m_colSetsById[mcatToCheck.eType()];
    tCIDLib::TCard2 c2CatId = mcatToCheck.c2Id();
    tCIDLib::TCard4 c4Count = colById.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (bTitleIsInCat(*colById[c4Index], c2CatId))
            return kCIDLib::True;
    }
    return kCIDLib::False;
}


//
//  Checks to see if the data type object of the indicated media type with
//  the passed unique id exists.
//
tCIDLib::TBoolean TMediaDB::
bCheckUIDExists(const   tCQCMedia::EMediaTypes  eMType
                , const tCQCMedia::EDataTypes   eDType
                , const TString&                strUniqueID
                ,       tCIDLib::TCard2&        c2Id
                ,       TString&                strName)
{
    const TMediaDBBase* pmddbTar = 0;
    switch(eDType)
    {
        case tCQCMedia::EDataTypes::Cat :
            pmddbTar = pmcatByUniqueIdNC(eMType, strUniqueID, kCIDLib::False);
            break;

        case tCQCMedia::EDataTypes::Collect :
            pmddbTar = pmcolByUniqueIdNC(eMType, strUniqueID, kCIDLib::False);
            break;

        case tCQCMedia::EDataTypes::TitleSet :
            pmddbTar = pmtsByUniqueIdNC(eMType, strUniqueID, kCIDLib::False);
            break;

        default :
            break;
    };

    if (pmddbTar)
    {
        c2Id = pmddbTar->c2Id();
        strName = pmddbTar->strName();
    }
     else
    {
        strName.Clear();
    }
    return (pmddbTar != 0);
}


//
//  Called to look up a cookie by its text
//
tCIDLib::TBoolean
TMediaDB::bCookieLookup(const   tCQCMedia::ECookieTypes eCType
                        , const tCQCMedia::EMediaTypes  eMType
                        , const TString&                strCatName
                        , const TString&                strTitleName
                        , const TString&                strColName
                        , const TString&                strItemName
                        ,       TString&                strToFill) const
{
    CIDAssert(eCType <= tCQCMedia::ECookieTypes::Max, L"Invalid cookie type");
    CIDAssert(eMType <= tCQCMedia::EMediaTypes::Max, L"Invalid media type");

    // The category is always at least used
    const TMediaCat* pmcatTar = 0;
    {
        const tCQCMedia::TNCCatList& colCats = *m_colCats[eMType];
        const tCIDLib::TCard4 c4Count = colCats.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const TMediaCat* pmcatCur = colCats.pobjAt(c4Index);
            if (pmcatCur->strName().bCompareI(strCatName))
            {
                pmcatTar = pmcatCur;
                break;
            }
        }
    }

    // If not found, then we failed. If looking for a category, we worked
    if (!pmcatTar)
    {
        return kCIDLib::False;
    }
     else if (eCType == tCQCMedia::ECookieTypes::Cat)
    {
        facCQCMedia().FormatCatCookie(eMType, pmcatTar->c2Id(), strToFill);
        return kCIDLib::True;
    }

    //
    //  Ok, let's do the title set. There's no real relationship here
    //  between the title set and the category above. If the title set
    //  actually isn't in that category, we aren't going to complain.
    //
    const TMediaTitleSet* pmtsTar = 0;
    {
        const tCQCMedia::TNCSetIdList& colSets = *m_colSetsById[eMType];
        const tCIDLib::TCard4 c4Count = colSets.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const TMediaTitleSet* pmtsCur = colSets[c4Index];
            if (pmtsCur->strName().bCompareI(strTitleName))
            {
                pmtsTar = pmtsCur;
                break;
            }
        }
    }

    // If not found, then we failed. If looking for a category, we worked
    if (!pmtsTar)
    {
        return kCIDLib::False;
    }
     else if (eCType == tCQCMedia::ECookieTypes::Title)
    {
        facCQCMedia().FormatTitleCookie
        (
            eMType, pmcatTar->c2Id(), pmtsTar->c2Id(), strToFill
        );
        return kCIDLib::True;
    }

    //
    //  And now the collection, which must be a collection within the
    //  title set we found above.
    //
    tCIDLib::TCard4 c4ColIndex = 0;
    const TMediaCollect* pmcolTar = 0;
    {
        const tCIDLib::TCard4 c4ColCount = pmtsTar->c4ColCount();
        for (; c4ColIndex < c4ColCount; c4ColIndex++)
        {
            const TMediaCollect& mcolCur = pmtsTar->mcolAt(*this, c4ColIndex);
            if (mcolCur.strName().bCompareI(strColName))
            {
                pmcolTar = &mcolCur;
                break;
            }
        }
    }

    // If not found, then we failed. If looking for a category, we worked
    if (!pmcolTar)
    {
        return kCIDLib::False;
    }
     else if (eCType == tCQCMedia::ECookieTypes::Collect)
    {
        //
        //  Remember that collection ids are faux ones relative to the
        //  title set, not their actual id, so we use the index we found it
        //  at above, plus one.
        //
        facCQCMedia().FormatColCookie
        (
            eMType
            , pmcatTar->c2Id()
            , pmtsTar->c2Id()
            , tCIDLib::TCard2(c4ColIndex + 1)
            , strToFill
        );
        return kCIDLib::True;
    }

    // OK, let's try to find the item inside the collection
    tCIDLib::TCard4 c4ItemIndex = 0;
    const TMediaItem* pmitemTar = nullptr;
    {
        const tCIDLib::TCard4 c4Count = pmcolTar->c4ItemCount();
        for (; c4ItemIndex < c4Count; c4ItemIndex++)
        {
            const TMediaItem& mitemCur = pmcolTar->mitemAt(*this, c4ItemIndex);
            if (mitemCur.strName().bCompareI(strItemName))
            {
                pmitemTar = &mitemCur;
                break;
            }
        }
    }

    // If not found, then we failed
    if (!pmitemTar)
        return kCIDLib::False;

    //
    //  We found it, so format out the cookie. Here again, the 'ids' of
    //  items within their collections are indirect via the one based
    //  index.
    //
    facCQCMedia().FormatItemCookie
    (
        eMType
        , pmcatTar->c2Id()
        , pmtsTar->c2Id()
        , tCIDLib::TCard2(c4ColIndex + 1)
        , tCIDLib::TCard2(c4ItemIndex + 1)
        , strToFill
    );
    return kCIDLib::True;
}


// Find the objects associated with a cookie
tCIDLib::TBoolean
TMediaDB::bFindByCookie(const   TString&                strCookie
                        ,       tCQCMedia::EMediaTypes& eMType
                        , const TMediaTitleSet*&        pmtsFound
                        , const TMediaCollect*&         pmcolFound
                        , const TMediaItem*&            pmitemFound) const
{
    // Parse the cookie to get the media type and cookie values
    tCIDLib::TCard2 c2CatId = 0;
    tCIDLib::TCard2 c2TiId = 0;
    tCIDLib::TCard2 c2ColId = 0;
    tCIDLib::TCard2 c2ItId = 0;
    const tCQCMedia::ECookieTypes eType = facCQCMedia().eCheckCookie
    (
        strCookie
        , eMType
        , c2CatId
        , c2TiId
        , c2ColId
        , c2ItId
    );

    pmtsFound = nullptr;
    pmcolFound = nullptr;
    pmitemFound = nullptr;

    if (eType >= tCQCMedia::ECookieTypes::Title)
    {
        pmtsFound = pmtsById(eMType, c2TiId, kCIDLib::False);
        if (!pmtsFound)
            return kCIDLib::False;
    }

    if (eType >= tCQCMedia::ECookieTypes::Collect)
    {
        //
        //  Get the real collection id, from the 1 based index used by
        //  cookies.
        //
        const tCIDLib::TCard2 c2RealId(pmtsFound->c2ColIdAt(c2ColId - 1));
        pmcolFound = pmcolById(eMType, c2RealId, kCIDLib::False);
        if (!pmcolFound)
            return kCIDLib::False;
    }

    if (eType >= tCQCMedia::ECookieTypes::Item)
    {
        // pmcolFound cannot be null here but the analyzer cannot figure that out
        CIDAssert(pmcolFound != nullptr, L"Collection was not found");

        //
        //  Get the real item id, from the 1 based index used by
        //  cookies.
        //
        const tCIDLib::TCard2 c2RealId(pmcolFound->c2ItemIdAt(c2ItId - 1));
        pmitemFound = pmitemById(eMType, c2RealId, kCIDLib::False);
        if (!pmitemFound)
            return kCIDLib::False;
    }

    // We found everything represented by this cookie
    return kCIDLib::True;
}


// Finds the set that owns a given collection
tCIDLib::TBoolean
TMediaDB::bFindColContainer(const   tCQCMedia::EMediaTypes  eType
                            , const tCIDLib::TCard2         c2Id
                            ,       tCIDLib::TCard2&        c2ColInd
                            , const TMediaTitleSet*&        pmtsCont) const
{
    pmtsCont = nullptr;

    // Make sure it exists. If not let it throw
    const TMediaCollect* pmcolTar = pmcolById(eType, c2Id, kCIDLib::True);

    // See if we can find its containing set
    const tCQCMedia::TNCSetIdList& colSets = *m_colSetsById[eType];
    const tCIDLib::TCard4 c4SetCnt = colSets.c4ElemCount();
    for (tCIDLib::TCard4 c4SetInd = 0; c4SetInd < c4SetCnt; c4SetInd++)
    {
        const TMediaTitleSet* pmtsCur = colSets[c4SetInd];
        if (pmtsCur->bContainsCol(pmcolTar->c2Id(), c2ColInd))
        {
            pmtsCont = pmtsCur;
            break;
        }
    }

    // Return true if we got the title set, which means we got the col, too
    return (pmtsCont != nullptr);
}

//
//  Finds the primary collection/set that own an item. Only one collection and
//  set actually own an item. Playlists only reference them.
//
tCIDLib::TBoolean
TMediaDB::bFindItemContainer(const  tCQCMedia::EMediaTypes  eType
                            , const tCIDLib::TCard2         c2Id
                            ,       tCIDLib::TCard2&        c2ColInd
                            ,       tCIDLib::TCard2&        c2ItemInd
                            , const TMediaTitleSet*&        pmtsCont
                            , const TMediaCollect*&         pmcolCont) const
{
    pmtsCont = nullptr;
    pmcolCont = nullptr;

    // Make sure it exists. If not let it throw
    const TMediaItem* pmitemTar = pmitemById(eType, c2Id, kCIDLib::True);

    const tCQCMedia::TNCColIdList& colCols = *m_colColsById[eType];
    const tCIDLib::TCard4 c4Count = colCols.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        // If not a playlist, and it contains the item, it's our guy
        const TMediaCollect* pmcolCur = colCols[c4Index];
        if (!pmcolCur->bIsPlayList() && pmcolCur->bContainsItem(c2Id, c2ItemInd))
        {
            pmcolCont = pmcolCur;
            break;
        }
    }

    // If we got the collection find the title set that contains it
    if (pmcolCont)
    {
        const tCQCMedia::TNCSetIdList& colSets = *m_colSetsById[eType];
        const tCIDLib::TCard4 c4SetCnt = colSets.c4ElemCount();

        for (tCIDLib::TCard4 c4SetInd = 0; c4SetInd < c4SetCnt; c4SetInd++)
        {
            const TMediaTitleSet* pmtsCur = colSets[c4SetInd];

            if (pmtsCur->bContainsCol(pmcolCont->c2Id(), c2ColInd))
            {
                pmtsCont = pmtsCur;
                break;
            }
        }
    }

    // Return true if we got the title set, which means we got the col, too
    return (pmtsCont != nullptr);
}



//
//  Find all the unassigned things of a given type. We return a list of
//  the ids of the unassigned ones. The return indicates whether there
//  were any unassigned ones found.
//
tCIDLib::TBoolean
TMediaDB::bFindUnassigned(          tCQCMedia::TIdList&     fcolToFill
                            , const tCQCMedia::EMediaTypes  eMType
                            , const tCQCMedia::EDataTypes   eDType)
{
    // Set up a bit set with 64K bits
    TBitset btsFlags(0x10000);

    // Flush the output list in preperation for filling in
    fcolToFill.RemoveAll();

    if (eDType == tCQCMedia::EDataTypes::Collect)
    {
        const tCQCMedia::TNCSetIdList& colSets = *m_colSetsById[eMType];
        tCIDLib::TCard4 c4Count = colSets.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const TMediaTitleSet& mtsCur = *colSets[c4Index];
            const tCIDLib::TCard4 c4ColCount = mtsCur.c4ColCount();
            for (tCIDLib::TCard4 c4CIndex = 0; c4CIndex < c4ColCount; c4CIndex++)
                btsFlags.bSetBitState(mtsCur.c2ColIdAt(c4CIndex), kCIDLib::True);
        }

        //
        //  Now got through the collection list. For each id we find, see
        //  if it's in the bit set. If not, add this id to the outgoing
        //  list.
        //
        const tCQCMedia::TNCColIdList& colCols = *m_colColsById[eMType];
        c4Count = colCols.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const TMediaCollect& mcolCur = *colCols[c4Index];
            if (!btsFlags[mcolCur.c2Id()])
                fcolToFill.c4AddElement(mcolCur.c2Id());
        }
    }
     else if (eDType == tCQCMedia::EDataTypes::Item)
    {
        const tCQCMedia::TNCColIdList& colCols = *m_colColsById[eMType];
        tCIDLib::TCard4 c4Count = colCols.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const TMediaCollect& mcolCur = *colCols[c4Index];
            const tCIDLib::TCard4 c4ItemCount = mcolCur.c4ItemCount();
            for (tCIDLib::TCard4 c4IIndex = 0; c4IIndex < c4ItemCount; c4IIndex++)
                btsFlags.bSetBitState(mcolCur.c2ItemIdAt(c4IIndex), kCIDLib::True);
        }

        //
        //  Now got through the item list. For each id we find, see
        //  if it's in the bit set. If not, add this id to the outgoing
        //  list.
        //
        const tCQCMedia::TNCItemIdList& colItems = *m_colItemsById[eMType];
        c4Count = colItems.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const TMediaItem& mitemCur = *colItems[c4Index];
            if (!btsFlags[mitemCur.c2Id()])
                fcolToFill.c4AddElement(mitemCur.c2Id());
        }
    }
    return !fcolToFill.bIsEmpty();
}


//
//  Returns true if we have no title sets, collections, or items.
//
tCIDLib::TBoolean TMediaDB::bIsEmpty() const
{
    tCIDLib::TBoolean bRet = kCIDLib::True;
    tCQCMedia::EMediaTypes eType = tCQCMedia::EMediaTypes::Min;
    for (; eType < tCQCMedia::EMediaTypes::Count; eType++)
    {
        if (!m_colSetsById[eType]->bIsEmpty()
        ||  !m_colColsById[eType]->bIsEmpty()
        ||  !m_colItemsById[eType]->bIsEmpty())
        {
            bRet = kCIDLib::False;
            break;
        }
    }
    return bRet;
}


//
//  Removes the indicated item from our list, if found. Returns true if the object
//  was found and removed. We do the lookups ourself here, so that we can find the
//  index and use that to remove the object.
//
//  If it's a track, we find the parent collection and set and any playlists that
//  reference it and remove it from them, and update them to re-compute info based on
//  contents. If it's a collection, we find the parent title set and do the sort of
//  thing.
//
tCIDLib::TBoolean
TMediaDB::bRemoveObject(const   tCQCMedia::EMediaTypes  eType
                        , const tCQCMedia::EDataTypes   eDataType
                        , const tCIDLib::TCard2         c2Id)
{
    // Find our guy, based on data type, and remove it
    tCIDLib::TBoolean   bRemoved = kCIDLib::False;
    tCIDLib::TCard4     c4At;
    switch(eDataType)
    {
        case tCQCMedia::EDataTypes::Cat :
        {
            tCQCMedia::TNCCatList& colTar = *m_colCats[eType];
            if (colTar.pobjKeyedBinarySearch(c2Id, TMediaCat::eIdKeyComp, c4At))
            {
                colTar.RemoveAt(c4At);
                bRemoved = kCIDLib::True;
            }
            break;
        }

        case tCQCMedia::EDataTypes::Collect :
        {
            tCQCMedia::TNCColIdList& colTar = *m_colColsById[eType];
            if (colTar.pobjKeyedBinarySearch(c2Id, TMediaCollect::eIdKeyComp, c4At))
            {
                TMediaCollect* pmcolDel = colTar[c4At];

                // First remove it from any containing sets
                RemoveFromSets(*pmcolDel);

                // And now remove from the lists
                m_colColsByUID[eType]->bRemoveKeyIfExists(pmcolDel->strUniqueId());
                m_colColsById[eType]->RemoveAt(c4At);
                bRemoved = kCIDLib::True;
            }
            break;
        }

        case tCQCMedia::EDataTypes::Image :
        {
            tCQCMedia::TNCImgIdList& colTar = *m_colImgsById[eType];
            if (colTar.pobjKeyedBinarySearch(c2Id, TMediaImg::eIdKeyComp, c4At))
            {
                TMediaImg* pmimgDel = colTar[c4At];
                m_colImgsByUID[eType]->bRemoveKeyIfExists(TMediaImg::strKeyUID(*pmimgDel));

                // And now remove from the owning collection
                colTar.RemoveAt(c4At);
                bRemoved = kCIDLib::True;
            }
            break;
        }

        case tCQCMedia::EDataTypes::Item :
        {
            tCQCMedia::TNCItemIdList& colTar = *m_colItemsById[eType];
            if (colTar.pobjKeyedBinarySearch(c2Id, TMediaItem::eIdKeyComp, c4At))
            {
                TMediaItem* pmitemDel = colTar[c4At];

                // First remove it from any collections.
                RemoveFromCols(*pmitemDel);

                // And now remove the track itself
                m_colItemsByUID[eType]->bRemoveKeyIfExists(pmitemDel->strUniqueId());
                m_colItemsById[eType]->RemoveAt(c4At);
                bRemoved = kCIDLib::True;
            }
            break;
        }

        case tCQCMedia::EDataTypes::TitleSet :
        {
            tCQCMedia::TNCSetIdList& colTar = *m_colSetsById[eType];
            if (colTar.pobjKeyedBinarySearch(c2Id, TMediaTitleSet::eIdKeyComp, c4At))
            {
                m_colSetsByUID[eType]->bRemoveKeyIfExists(colTar[c4At]->strUniqueId());
                m_colSetsById[eType]->RemoveAt(c4At);
                bRemoved = kCIDLib::True;
            }
            break;
        }

        default :
            #if CID_DEBUG_ON
            facCQCMedia().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kMedErrs::errcDB_UnknownDataType
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Unknown
                , TInteger(tCIDLib::i4EnumOrd(eDataType))
            );
            #endif
            break;
    };

    // If anything was removed, reset transient views
    if (bRemoved)
        ResetTransientViews();

    return bRemoved;
}


//
//  Checks to see if any of the indicated title's collections are in the
//  indicate category.
//
tCIDLib::TBoolean
TMediaDB::bTitleIsInCat(const   TMediaTitleSet& mtsToCheck
                        , const tCIDLib::TCard2 c2CatId) const
{
    const tCIDLib::TCard4 c4ColCount = mtsToCheck.c4ColCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ColCount; c4Index++)
    {
        const TMediaCollect* pmcolCur = pmcolById
        (
            mtsToCheck.eType(),  mtsToCheck.c2ColIdAt(c4Index), kCIDLib::False
        );
        if (pmcolCur && pmcolCur->bIsInCat(c2CatId))
            return kCIDLib::True;
    }
    return kCIDLib::False;
}


//
//  These methods all add various data types to the database. Typically these are
//  expected to assign ids to the objects being added, since they won't have one when
//  first being added. However, in the case where we load pre-existing database, they
//  will have existing ids, so the bTakeId parameter indicates we should take the
//  incoming id. It must be non-zero and cannot be a dup in that case.
//
//  Either way we return the id used.
//
//  They are required to have a unique id that is unique within that data type and
//  media type, since these are fundamental to how data is tracked and mapped back to
//  the orginal source and such.
//
tCIDLib::TCard2
TMediaDB::c2AddCategory(TMediaCat* const pmcatToAdd, const tCIDLib::TBoolean bTakeId)
{
    // Put a janitor on it until we store it away
    TJanitor<TMediaCat> janCat(pmcatToAdd);

    const tCQCMedia::EMediaTypes eType = pmcatToAdd->eType();

    #if CID_DEBUG_ON
    if (eType >= tCQCMedia::EMediaTypes::Count)
        ThrowBadType(CID_LINE, eType);
    #endif

    // Check for a dup of the unique id. Throw if so
    if (pmcatByUniqueId(eType, pmcatToAdd->strUniqueId(), kCIDLib::False))
    {
        facCQCMedia().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMedErrs::errcDB_DupMediaId
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Already
            , pmcatToAdd->strUniqueId()
            , tCQCMedia::strXlatEDataTypes(tCQCMedia::EDataTypes::Cat)
        );
    }

    // Check or assign the id
    tCIDLib::TCard2 c2Ret = 0;
    if (bTakeId)
    {
        c2Ret = c2CheckTakeId(*pmcatToAdd, tCQCMedia::EDataTypes::Cat, eType);
    }
     else
    {
        c2Ret = c2NextId(eType, tCQCMedia::EDataTypes::Cat);
        pmcatToAdd->c2Id(c2Ret);
    }

    // Looks reasonable, so let's add it
    tCQCMedia::TNCCatList& colTar = *m_colCats[eType];

    //
    //  Not been added yet, so add it to the owning collection, sorted. We
    //  do bother with a separate 'by UID' collection for these. There are
    //  never very many, so we just do a linear search of the by id list
    //  if we need to do a buy UID lookup.
    //
    tCIDLib::TCard4 c4At;
    colTar.InsertSorted(janCat.pobjOrphan(), TMediaCat::eCompById, c4At);
    return c2Ret;
}


tCIDLib::TCard2
TMediaDB::c2AddCollect(TMediaCollect* const pmcolToAdd, const tCIDLib::TBoolean bTakeId)
{
    // Put a janitor on it until we store it away
    TJanitor<TMediaCollect> janCollect(pmcolToAdd);

    const tCQCMedia::EMediaTypes eType = pmcolToAdd->eType();

    #if CID_DEBUG_ON
    if (eType >= tCQCMedia::EMediaTypes::Count)
        ThrowBadType(CID_LINE, eType);
    #endif

    // Make sure the unique id is unique
    if (pmcolByUniqueId(eType, pmcolToAdd->strUniqueId(), kCIDLib::False))
    {
        facCQCMedia().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMedErrs::errcDB_DupMediaId
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Already
            , pmcolToAdd->strUniqueId()
            , tCQCMedia::strXlatEDataTypes(tCQCMedia::EDataTypes::Collect)
        );
    }

    // Check or assign the id
    tCIDLib::TCard2 c2Ret = 0;
    if (bTakeId)
    {
        c2Ret = c2CheckTakeId(*pmcolToAdd, tCQCMedia::EDataTypes::Collect, eType);
    }
     else
    {
        c2Ret = c2NextId(eType, tCQCMedia::EDataTypes::Collect);
        pmcolToAdd->c2Id(c2Ret);
    }

    //
    //  Make sure it's in the 'All XXX' category for its media type. If it's a
    //  playlist, then add it ot the playlsits category.
    //
    switch(eType)
    {
        case tCQCMedia::EMediaTypes::Music :
            pmcolToAdd->bAddCategory(kCQCMedia::c2CatId_AllMusic);

            // If a play list, add it to the playlists category
            if (pmcolToAdd->bIsPlayList())
                pmcolToAdd->bAddCategory(kCQCMedia::c2CatId_PlayLists);
            break;

        case tCQCMedia::EMediaTypes::Movie :
            pmcolToAdd->bAddCategory(kCQCMedia::c2CatId_AllMovies);
            break;

        case tCQCMedia::EMediaTypes::Pic :
            pmcolToAdd->bAddCategory(kCQCMedia::c2CatId_AllPictures);
            break;

        default :
            break;
    };

    //
    //  Not been added yet, so add it to the owning collection, sorted,
    //  and the referencing 'by UID' list.
    //
    tCIDLib::TCard4 c4At;
    m_colColsById[eType]->InsertSorted(janCollect.pobjOrphan(), TMediaCollect::eCompById, c4At);
    m_colColsByUID[eType]->Add(pmcolToAdd);

    // Reset any transient views just in case
    ResetTransientViews();

    return c2Ret;
}


tCIDLib::TCard2
TMediaDB::c2AddImage(TMediaImg* const pmimgToAdd, const tCIDLib::TBoolean bTakeId)
{
    // Put a janitor on it until we store it away
    TJanitor<TMediaImg> janImg(pmimgToAdd);

    const tCQCMedia::EMediaTypes eType = pmimgToAdd->eType();

    #if CID_DEBUG_ON
    if (eType >= tCQCMedia::EMediaTypes::Count)
        ThrowBadType(CID_LINE, eType);
    #endif

    // Make sure the unique id is unique
    if (pmimgByUniqueId(eType, pmimgToAdd->strUniqueId(), kCIDLib::False))
    {
        facCQCMedia().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMedErrs::errcDB_DupMediaId
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Already
            , pmimgToAdd->strUniqueId()
            , tCQCMedia::strXlatEDataTypes(tCQCMedia::EDataTypes::Image)
        );
    }

    // Check or assign the id
    tCIDLib::TCard2 c2Ret = 0;
    if (bTakeId)
    {
        c2Ret = c2CheckTakeId(*pmimgToAdd, tCQCMedia::EDataTypes::Image, eType);
    }
     else
    {
        c2Ret = c2NextId(eType, tCQCMedia::EDataTypes::Image);
        pmimgToAdd->c2Id(c2Ret);
    }

    //
    //  Not been added yet, so add it to the appropriate lists. Orphan it into the
    //  by id list.
    //
    tCIDLib::TCard4 c4At;
    m_colImgsById[eType]->InsertSorted(janImg.pobjOrphan(), TMediaImg::eCompById, c4At);
    m_colImgsByUID[eType]->Add(pmimgToAdd);

    return c2Ret;
}


tCIDLib::TCard2
TMediaDB::c2AddItem(TMediaItem* const pmitemToAdd, const tCIDLib::TBoolean bTakeId)
{
    // Put a janitor on it until we store it away
    TJanitor<TMediaItem> janItem(pmitemToAdd);

    const tCQCMedia::EMediaTypes eType = pmitemToAdd->eType();

    #if CID_DEBUG_ON
    if (eType >= tCQCMedia::EMediaTypes::Count)
        ThrowBadType(CID_LINE, eType);
    #endif

    // Make sure the unique id is unique
    if (pmitemByUniqueId(eType, pmitemToAdd->strUniqueId(), kCIDLib::False))
    {
        facCQCMedia().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMedErrs::errcDB_DupMediaId
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Already
            , pmitemToAdd->strUniqueId()
            , tCQCMedia::strXlatEDataTypes(tCQCMedia::EDataTypes::Item)
        );
    }

    // Check or assign the id
    tCIDLib::TCard2 c2Ret = 0;
    if (bTakeId)
    {
        c2Ret = c2CheckTakeId(*pmitemToAdd, tCQCMedia::EDataTypes::Item, eType);
    }
     else
    {
        c2Ret = c2NextId(eType, tCQCMedia::EDataTypes::Item);
        pmitemToAdd->c2Id(c2Ret);
    }

    // Not been added yet, so add it to the owning collection, sorted
    tCIDLib::TCard4 c4At;
    m_colItemsById[eType]->InsertSorted(janItem.pobjOrphan(), TMediaItem::eCompById, c4At);
    m_colItemsByUID[eType]->Add(pmitemToAdd);

    // Reset any transient views just in case
    ResetTransientViews();

    return c2Ret;
}


// Adds a new title to the database
tCIDLib::TCard2
TMediaDB::c2AddTitle(TMediaTitleSet* const pmtsToAdd, const tCIDLib::TBoolean bTakeId)
{
    // Put a janitor on it until we store it away
    TJanitor<TMediaTitleSet> janTitle(pmtsToAdd);

    const tCQCMedia::EMediaTypes eType = pmtsToAdd->eType();

    #if CID_DEBUG_ON
    if (eType >= tCQCMedia::EMediaTypes::Count)
        ThrowBadType(CID_LINE, eType);
    #endif

    // Make sure the unique id is unique
    if (pmtsByUniqueId(eType, pmtsToAdd->strUniqueId(), kCIDLib::False))
    {
        facCQCMedia().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMedErrs::errcDB_DupMediaId
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Already
            , pmtsToAdd->strUniqueId()
            , tCQCMedia::strXlatEDataTypes(tCQCMedia::EDataTypes::TitleSet)
        );
    }

    // Check or assign the id
    tCIDLib::TCard2 c2Ret = 0;
    if (bTakeId)
    {
        c2Ret = c2CheckTakeId(*pmtsToAdd, tCQCMedia::EDataTypes::TitleSet, eType);
    }
     else
    {
        c2Ret = c2NextId(eType, tCQCMedia::EDataTypes::TitleSet);
        pmtsToAdd->c2Id(c2Ret);
    }

    //
    //  Not been added yet, so add it to the owning collection, sorted,
    //  and the referencing 'by UID' list.
    //
    tCIDLib::TCard4 c4At;
    m_colSetsById[eType]->InsertSorted
    (
        janTitle.pobjOrphan(), TMediaTitleSet::eCompById, c4At
    );
    m_colSetsByUID[eType]->Add(pmtsToAdd);

    // Reset any transient views just in case
    ResetTransientViews();

    return c2Ret;
}



//
//  This method is used to build a compressed binary dump of the whole database
//  which can be downloaded by CQC cliens to get access to the data.
//
tCIDLib::TCard4
TMediaDB::c4BuildBinDump(       TMemBuf&            mbufToFill
                        , const TString&            strRepoMon
                        , const TString&            strSerialNum
                        , const tCQCMedia::EMTFlags eMTFlags) const
{
    TChunkedBinOutStream strmRawOut(32 * (1024 * 1024));

    // Stream out the data to a chunked buffer stream
    tCIDLib::TCard4 c4RawBytes = 0;
    c4RawBytes = c4FormatBinDump(strmRawOut, eMTFlags, strSerialNum, *this);

    const tCIDLib::TCard4 c4Extra = sizeof(tCIDLib::TCard4) * 2;
    tCIDLib::TCard4 c4RetSz = 0;
    {
        TZLibCompressor zlibComp;

        //
        //  Create a linked stream so we can stream from the data we put out above,
        //  and a new out put for the compressed data.
        //
        TChunkedBinInStream strmRawIn(strmRawOut);
        TChunkedBinOutStream strmCompOut(32 * (1024 * 1024));

        // Now compress to the temp buffer
        c4RetSz = zlibComp.c4Compress(strmRawIn, strmCompOut);

        //
        //  Make sure the caller's buffer is sized up hold the compressed data
        //  plus the extra two bytes. The copying out of the stream contents would
        //  size it up, but won't contain the extra bytes.
        //
        if (mbufToFill.c4Size() < c4RetSz + c4Extra)
            mbufToFill.Reallocate(c4RetSz + c4Extra, kCIDLib::False);

        // Copy out the compressed data to the target buf, after the extra values
        strmCompOut.c4CopyOutTo(mbufToFill, c4Extra);
    }

    //  And put in the size values at the start now where we left space
    mbufToFill.PutCard4(c4RawBytes, 0);
    mbufToFill.PutCard4(c4RawBytes ^ kCIDLib::c4MaxCard, sizeof(c4RawBytes));

    // And return the compressed data size plus the extra stuff
    return c4RetSz + c4Extra;
}


//
//  This method is used to build up a compressed XML dump of the whole database
//  which can be downloaded by non-CQC clients to get access to the data.
//
tCIDLib::TCard4
TMediaDB::c4BuildXMLDump(       TMemBuf&            mbufToFill
                        , const TString&            strRepoMon
                        , const TString&            strSerialNum
                        , const tCQCMedia::EMTFlags eMTFlags) const
{
    TChunkedTextOutStream strmRawOut(48 * (1024 * 1024),  new TUTF8Converter);

    tCIDLib::TCard4 c4RawBytes = 0;

    // Output the opening XML document info
    strmRawOut  << L"<?xml version=\"1\" encoding=\"UTF-8\"?>\n"
                << L"<Meta Repo='" << strRepoMon
                << L"' FmtVer='" << kCQCMedia::c2XMLMDBDumpFmtVer
                << L"' SerNum='" << strSerialNum
                << L"' MTFlags='";

    if (tCIDLib::bAllBitsOn(eMTFlags, tCQCMedia::EMTFlags::Movie))
        strmRawOut << L"Movie ";
    if (tCIDLib::bAllBitsOn(eMTFlags, tCQCMedia::EMTFlags::Music))
        strmRawOut << L"Music";

    strmRawOut << L"'>\n";

    // If there's any movie content, then format that out
    strmRawOut << L"<Movies>\n";
    FormatXML(tCQCMedia::EMediaTypes::Movie, strmRawOut);
    strmRawOut << L"</Movies>\n";

    // If there's any music content, then format that out
    strmRawOut << L"<Music>\n";
    FormatXML(tCQCMedia::EMediaTypes::Music, strmRawOut);
    strmRawOut << L"</Music>\n";

    // Close off the root element and flush the stream
    strmRawOut << L"</Meta>\n";
    strmRawOut.Flush();

    // Now let's compress it
    tCIDLib::TCard4 c4RetSz = 0;
    TZLibCompressor zlibComp;

    //
    //  Create a linked stream to read in the raw data from. Note that here we
    //  want to use the binary stream that underlies the text stream. Because
    //  we need to feed a binary stream to the compressor.
    //
    TChunkedBinInStream strmRawIn(strmRawOut.strmOut());

    // And another chunked stream to compress to
    TChunkedBinOutStream strmCompOut(32 * (1024 * 1024));

    c4RetSz = zlibComp.c4Compress(strmRawIn, strmCompOut);

    // Make sure the caller's buffer is sized up to take the comp bytes
    if (mbufToFill.c4Size() < c4RetSz)
        mbufToFill.Reallocate(c4RetSz, kCIDLib::False);

    // And copy out to to the target buffer
    const tCIDLib::TCard4 c4OutBytes = strmCompOut.c4CopyOutTo(mbufToFill, 0);
    CIDAssert(c4OutBytes == c4RetSz, L"Compressed bytes != streamed bytes");

    return c4RetSz;
}


// Returns the count of categories for a given media type
tCIDLib::TCard4 TMediaDB::c4CatCnt(const tCQCMedia::EMediaTypes eType) const
{
    #if CID_DEBUG_ON
    if (eType >= tCQCMedia::EMediaTypes::Count)
        ThrowBadType(CID_LINE, eType);
    #endif
    return m_colCats[eType]->c4ElemCount();
}


// Returns the count of collections of the indicated type
tCIDLib::TCard4 TMediaDB::c4CollectCnt(const tCQCMedia::EMediaTypes eType) const
{
    #if CID_DEBUG_ON
    if (eType >= tCQCMedia::EMediaTypes::Count)
        ThrowBadType(CID_LINE, eType);
    #endif
    return m_colColsById[eType]->c4ElemCount();
}


// Returns the count of images for the indicated media type
tCIDLib::TCard4 TMediaDB::c4ImageCnt(const tCQCMedia::EMediaTypes eType) const
{
    #if CID_DEBUG_ON
    if (eType >= tCQCMedia::EMediaTypes::Count)
        ThrowBadType(CID_LINE, eType);
    #endif
    return m_colImgsById[eType]->c4ElemCount();
}


// Returns the count of items of the indicated type
tCIDLib::TCard4 TMediaDB::c4ItemCnt(const tCQCMedia::EMediaTypes eType) const
{
    #if CID_DEBUG_ON
    if (eType >= tCQCMedia::EMediaTypes::Count)
        ThrowBadType(CID_LINE, eType);
    #endif
    return m_colItemsById[eType]->c4ElemCount();
}


//
//  Find all the collections that are changer based, and build up a map
//  of taken and free slots. We set the bits for the taken ones. We can also
//  return a list of names.
//
tCIDLib::TCard4
TMediaDB::c4QueryAvailSlots(const   TString&                strChangerMoniker
                            ,       TBitset&                btsToFill
                            , const tCIDLib::TCard4         c4MaxSlots
                            ,       TVector<TString>* const pcolColNames) const
{
    //
    //  Size up the bit set to the max slots, which will zero it out so
    //  we only have to set the bits for the taken ones. And flush the
    //  name collection in preperation for loading it back up, and make
    //  sure it's large enough for the slot count
    //
    btsToFill.ChangeBitCount(c4MaxSlots);
    if (pcolColNames)
    {
        pcolColNames->RemoveAll();
        pcolColNames->CheckExpansion(c4MaxSlots);

        //
        //  Put in max slots worth of empty strings, so that we can just
        //  assign titles to the indices we find.
        //
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4MaxSlots; c4Index++)
            pcolColNames->objAdd(TString::strEmpty());
    }

    //
    //  We'll be looking for location info that starts with the passed
    //  changer moniker plus a period.
    //
    TString strFindStr(strChangerMoniker);
    strFindStr.Append(kCIDLib::chPeriod);

    // For each media type, we loop through each collection
    tCIDLib::TCard4 c4RetCnt = 0;
    TString strSlot;
    tCQCMedia::EMediaTypes eType = tCQCMedia::EMediaTypes::Min;
    for (; eType < tCQCMedia::EMediaTypes::Count; eType++)
    {
        const tCQCMedia::TNCColIdList& colTar = *m_colColsById[eType];
        const tCIDLib::TCard4 c4Count = colTar.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const TMediaCollect* pmcolCur = colTar[c4Index];

            // If not a changer based, skip it
            if (pmcolCur->eLocType() != tCQCMedia::ELocTypes::Changer)
                continue;

            // If it doesn't start with our find string, skip it
            const TString& strLoc = pmcolCur->strLocInfo();
            if (!strLoc.bStartsWith(strFindStr))
                continue;

            //
            //  It's one we care about, so break out the slot number
            //  and set that bit on the bitset, if it's not larger than
            //  the max slots we got.
            //
            strLoc.CopyOutSubStr(strSlot, strFindStr.c4Length());

            const tCIDLib::TCard4 c4SlotNum = strSlot.c4Val(tCIDLib::ERadices::Dec);
            if (c4SlotNum && (c4SlotNum <= c4MaxSlots))
            {
                btsToFill.bSetBitState(c4SlotNum - 1, kCIDLib::True);
                if (pcolColNames)
                    (*pcolColNames)[c4SlotNum - 1] = pmcolCur->strName();
                c4RetCnt++;
            }
        }
    }
    return c4RetCnt;
}


//
//  Find all of the music collections that are marked as play lists, and
//  return their names and ids.
//
tCIDLib::TCard4
TMediaDB::c4QueryPlayLists( TVector<TString>&               colNames
                            , TFundVector<tCIDLib::TCard2>& fcolIds) const
{
    colNames.RemoveAll();
    fcolIds.RemoveAll();

    const tCQCMedia::TNCColIdList& colTar = *m_colColsById[tCQCMedia::EMediaTypes::Music];
    const tCIDLib::TCard4 c4Count = colTar.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TMediaCollect* pmcolCur = colTar[c4Index];
        if (!pmcolCur->bIsPlayList())
            continue;

        colNames.objAdd(pmcolCur->strName());
        fcolIds.c4AddElement(pmcolCur->c2Id());
    }
    return colNames.c4ElemCount();
}


//
//  These are helpers that will return non-adopting ref vectors of the various
//  data types, sorted by whatever the caller wants them sorted by. The vector
//  has to be non-adopting. This is just for the callers to get temporary lists
//  for display or evaluation and whatnot.
//
tCIDLib::TCard4
TMediaDB::c4QueryCatList(const  tCQCMedia::EMediaTypes  eType
                        ,       tCQCMedia::TCatList&    colToFill
                        ,       TCatCompFunc            pfnSort
                        ,       TMediaCatFilt* const    pmdbfSel) const
{
    CIDAssert
    (
        colToFill.eAdopt() == tCIDLib::EAdoptOpts::NoAdopt
        , L"The output vector must be non-adopting"
    )

    // Flush the list to start filling in new content
    colToFill.RemoveAll();

    const tCQCMedia::TNCCatList& colCats = *m_colCats[eType];
    const tCIDLib::TCard4 c4Count = colCats.c4ElemCount();
    colToFill.CheckExpansion(c4Count);
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        // If the current category is empty, then skip it
        const TMediaCat* pmcatCur = colCats[c4Index];
        if (!bCatHasSomeSets(*pmcatCur))
            continue;

        // If no filter, or the filter says keep it, then add it
        if (!pmdbfSel || pmdbfSel->bTest(*pmcatCur))
            colToFill.Add(pmcatCur);
    }

    // If the list isn't empty, sort it
    const tCIDLib::TCard4 c4Ret = colToFill.c4ElemCount();
    if (c4Ret)
        colToFill.Sort(pfnSort);

    return c4Ret;
}

tCIDLib::TCard4
TMediaDB::c4QueryColList(const  tCQCMedia::EMediaTypes  eType
                        ,       tCQCMedia::TColIdList&  colToFill
                        ,       TColCompFunc            pfnSort
                        ,       TMediaColFilt* const    pmdbfSel) const
{
    CIDAssert
    (
        colToFill.eAdopt() == tCIDLib::EAdoptOpts::NoAdopt
        , L"The output vector must be non-adopting"
    )

    // Flush the list to start filling in new content
    colToFill.RemoveAll();

    const tCQCMedia::TNCColIdList& colCols = *m_colColsById[eType];
    const tCIDLib::TCard4 c4Count = colCols.c4ElemCount();
    colToFill.CheckExpansion(c4Count);
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        // If no filter or the filter says keep it, then add it
        const TMediaCollect* pmcolCur = colCols[c4Index];
        if (!pmdbfSel || pmdbfSel->bTest(*pmcolCur))
            colToFill.Add(pmcolCur);
    }

    // If the list isn't empty, sort it
    const tCIDLib::TCard4 c4Ret = colToFill.c4ElemCount();
    if (c4Ret)
        colToFill.Sort(pfnSort);

    return c4Ret;
}

tCIDLib::TCard4
TMediaDB::c4QueryItemList(  const   tCQCMedia::EMediaTypes  eType
                            ,       tCQCMedia::TItemIdList& colToFill
                            ,       TItemCompFunc           pfnSort
                            ,       TMediaItemFilt* const   pmdbfSel) const
{
    CIDAssert
    (
        colToFill.eAdopt() == tCIDLib::EAdoptOpts::NoAdopt
        , L"The output vector must be non-adopting"
    )

    // Flush the list to start filling in new content
    colToFill.RemoveAll();

    const tCQCMedia::TNCItemIdList& colItems = *m_colItemsById[eType];
    const tCIDLib::TCard4 c4Count = colItems.c4ElemCount();
    colToFill.CheckExpansion(c4Count);
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        // If no filter or the filter says keep it, then add it
        const TMediaItem* pmitemCur = colItems[c4Index];
        if (!pmdbfSel || pmdbfSel->bTest(*pmitemCur))
            colToFill.Add(pmitemCur);
    }

    // If the list isn't empty, sort it
    const tCIDLib::TCard4 c4Ret = colToFill.c4ElemCount();
    if (c4Ret)
        colToFill.Sort(pfnSort);

    return c4Ret;
}

tCIDLib::TCard4
TMediaDB::c4QuerySetList(const  tCQCMedia::EMediaTypes  eType
                        ,       tCQCMedia::TSetIdList&  colToFill
                        ,       TSetCompFunc            pfnSort
                        ,       TMediaSetFilt* const    pmdbfSel) const
{
    CIDAssert
    (
        colToFill.eAdopt() == tCIDLib::EAdoptOpts::NoAdopt
        , L"The output vector must be non-adopting"
    )

    // Flush the list to start filling in new content
    colToFill.RemoveAll();

    const tCQCMedia::TNCSetIdList& colSets = *m_colSetsById[eType];
    const tCIDLib::TCard4 c4Count = colSets.c4ElemCount();
    colToFill.CheckExpansion(c4Count);

    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        // If no filter, or the filter says keep it, then add it
        const TMediaTitleSet* pmtsCur = colSets[c4Index];
        if (!pmdbfSel || pmdbfSel->bTest(*pmtsCur))
            colToFill.Add(pmtsCur);
    }

    // If the list isn't empty, sort it
    const tCIDLib::TCard4 c4Ret = colToFill.c4ElemCount();
    if (c4Ret)
        colToFill.Sort(pfnSort);

    return c4Ret;
}


// Returns the count of titles for a given media type
tCIDLib::TCard4 TMediaDB::c4TitleSetCnt(const tCQCMedia::EMediaTypes eType) const
{
    #if CID_DEBUG_ON
    if (eType >= tCQCMedia::EMediaTypes::Count)
        ThrowBadType(CID_LINE, eType);
    #endif
    return m_colSetsById[eType]->c4ElemCount();
}


//
//  Return the total count of title sets, as a convenince since it's needed
//  by repo drivers./
//
tCIDLib::TCard4 TMediaDB::c4TotalTitleCnt() const
{
    tCIDLib::TCard4 c4Ret = 0;

    c4Ret += m_colSetsById[tCQCMedia::EMediaTypes::Music]->c4ElemCount();
    c4Ret += m_colSetsById[tCQCMedia::EMediaTypes::Movie]->c4ElemCount();
    c4Ret += m_colSetsById[tCQCMedia::EMediaTypes::Pic]->c4ElemCount();

    return c4Ret;
}


//
//  Returns the by artist mapping for a given media type.
//
const tCQCMedia::TByArtistList&
TMediaDB::colByArtistList(const tCQCMedia::EMediaTypes eMType) const
{
    #if CID_DEBUG_ON
    if (eMType >= tCQCMedia::EMediaTypes::Count)
        ThrowBadType(CID_LINE, eMType);
    #endif

    // Return the by artist list for the indicated media type
    return *m_colByArtist[eMType];
}


// A convenience to clear the art of a specific type on an image
tCIDLib::TVoid TMediaDB::ClearArt(  const   tCIDLib::TCard2         c2ImgId
                                    , const tCQCMedia::EMediaTypes  eMType
                                    , const tCQCMedia::ERArtTypes   eArtType)
{
    TMediaImg* pimgTar = pmimgByIdNC(eMType, c2ImgId, kCIDLib::True);
    pimgTar->ClearArt(eArtType);
}


// A convenience to clear all the art paths of an image
tCIDLib::TVoid TMediaDB::ClearArtPaths( const   tCIDLib::TCard2         c2ImgId
                                        , const tCQCMedia::EMediaTypes  eMType)
{
    TMediaImg* pimgTar = pmimgByIdNC(eMType, c2ImgId, kCIDLib::True);
    pimgTar->SetArtPath(TString::strEmpty(), tCQCMedia::ERArtTypes::LrgCover);
    pimgTar->SetArtPath(TString::strEmpty(), tCQCMedia::ERArtTypes::SmlCover);
    pimgTar->SetArtPath(TString::strEmpty(), tCQCMedia::ERArtTypes::Poster);
}


tCIDLib::TVoid
TMediaDB::ClearColCats( const   tCIDLib::TCard2         c2ColId
                        , const tCQCMedia::EMediaTypes  eMType)
{
    pmcolByIdNC(eMType, c2ColId, kCIDLib::True)->ClearCategories();
}


//
//  A helper to make it easier to finalize individual objects after their referenced
//  objects have been modified. They are all done at the end of a DB load, but
//  sometimes after single changes we need to do anything that contains the changed
//  stuff.
//
tCIDLib::TVoid
TMediaDB::FinalizeObjects(  const   tCIDLib::TCard2         c2Id
                            , const tCQCMedia::EMediaTypes  eMType
                            , const tCQCMedia::EDataTypes   eDType
                            , const tCIDLib::TBoolean       bIncludeContained)
{
    //
    //  If a title set, and we are doing included stuff, then get its
    //  collections first and do them, an then the title set. Else, get the
    //  collection and do that.
    //
    if (eDType == tCQCMedia::EDataTypes::TitleSet)
    {
        TMediaTitleSet* pmtsTar = pmtsByIdNC(eMType, c2Id, kCIDLib::True);

        // Go through the collections and do them if asked
        if (bIncludeContained)
        {
            const tCIDLib::TCard4 c4Count = pmtsTar->c4ColCount();
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            {
                const tCIDLib::TCard2 c2ColId = pmtsTar->c2ColIdAt(c4Index);

                TMediaCollect* pmcolTar = pmcolByIdNC(eMType, c2ColId, kCIDLib::True);
                pmcolTar->Finalize(*this);
            }
        }

        // Do the title set itself
        pmtsTar->Finalize(*this);
    }
     else if (eDType == tCQCMedia::EDataTypes::Collect)
    {
        TMediaCollect* pmcolTar = pmcolByIdNC(eMType, c2Id, kCIDLib::True);
        pmcolTar->Finalize(*this);
    }
}


//
//  If a repository allows editing, or reporting of changes from the repo source side,
//  then the repo driver has to update any containers that reference the affected
//  object.
//
//  It would be awfully nice if it could all be done here, but unfortunately, due to
//  the way that data is made available in variuos repos, it can't. Some of it can be
//  done by calling the Finalize() method of any containers, but other stuff has to be
//  done by the driver because it's not always just a matter of re-aggregating info
//  from underlying objects.
//
//  So we provide a means to find all collections and sets that reference an affected
//  object. Obviously if it's just a title set, then caller can handle it specially
//  since it would be the only object. But we handle that scenario here as well, in
//  which case the collections list would be empty and the sets list would have one
//  entry. This way the driver can handle the situation fairly generically.
//
//  There can be more than one collection if the object is an item, since it can be
//  referenced by playlists, and in that case there could be more than one title set
//  if those playlists are in more than one set, though typically playlists are put
//  into single set/collections each.
//
//  The two provided lists must be non-adopting or we will throw.
//
//  If the indicated object is not found, we'll throw.
//
tCIDLib::TVoid
TMediaDB::FindContainers(const  tCQCMedia::EMediaTypes  eMType
                        , const tCQCMedia::EDataTypes   eDataType
                        , const tCIDLib::TCard2         c2Id
                        ,       tCQCMedia::TColIdList&  colCols
                        ,       tCQCMedia::TSetIdList&  colSets) const
{
    #if CID_DEBUG_ON
    if ((colCols.eAdopt() != tCIDLib::EAdoptOpts::NoAdopt)
    ||  (colSets.eAdopt() != tCIDLib::EAdoptOpts::NoAdopt))
    {
        facCIDLib().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCIDErrs::errcCol_CantAdopt
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Internal
        );
    }
    #endif

    if (eDataType == tCQCMedia::EDataTypes::Item)
    {
        // Make sure it exists. If not let it throw
        const TMediaItem* pmitemTar = pmitemById(eMType, c2Id, kCIDLib::True);

        // Find all collections that referenced this item
        const tCQCMedia::TNCColIdList& colSrcCols = *m_colColsById[eMType];
        const tCIDLib::TCard4 c4Count = colSrcCols.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            // If this one doesn't reference the indicated track, do nothing
            const TMediaCollect& mcolCur = *colSrcCols[c4Index];

            tCIDLib::TCard2 c2ItemInd;
            if (!mcolCur.bContainsItem(c2Id, c2ItemInd))
                continue;

            colCols.Add(&mcolCur);
        }
    }
     else if (eDataType == tCQCMedia::EDataTypes::Collect)
    {
        // Just add the one collection to the collection list
        colCols.Add(pmcolById(eMType, c2Id, kCIDLib::True));
    }
     else if (eDataType == tCQCMedia::EDataTypes::TitleSet)
    {
        // Just add the one title set to the title set list
        colSets.Add(pmtsById(eMType, c2Id, kCIDLib::True));
    }

    //
    //  If we got any collections, then find the sets that contain them. In this
    //  case, it's a one to one relationship, so we can break out of the inner loop
    //  once we find each containing set.
    //
    const tCIDLib::TCard4 c4ColCnt = colCols.c4ElemCount();
    if (c4ColCnt)
    {
        const tCQCMedia::TNCSetIdList& colSrcSets = *m_colSetsById[eMType];
        const tCIDLib::TCard4 c4SetCnt = colSrcSets.c4ElemCount();

        for (tCIDLib::TCard4 c4ColInd = 0; c4ColInd < c4ColCnt; c4ColInd++)
        {
            const TMediaCollect* pmcolCur = colCols[c4ColInd];
            for (tCIDLib::TCard4 c4SetInd = 0; c4SetInd < c4SetCnt; c4SetInd++)
            {
                const TMediaTitleSet* pmtsCur = colSrcSets[c4SetInd];

                tCIDLib::TCard2 c2Index;
                if (pmtsCur->bContainsCol(pmcolCur->c2Id(), c2Index))
                {
                    colSets.Add(pmtsCur);
                    break;
                }
            }
        }
    }
}


//
//  When streaming the database out, we have to write out the same stuff for each media type,
//  so we split this out to call separately. And some folks may just want to use these
//  directly to deal with a single media type.
//
//  Similar for formatting out to XML as well.
//
tCIDLib::TVoid
TMediaDB::FormatBin(const   tCQCMedia::EMediaTypes  eMType
                    ,       TBinOutStream&          strmTar) const
{
    // Stream out the music type for double checking
    strmTar << tCIDLib::EStreamMarkers::Frame << eMType;

    // Do the images
    tCIDLib::TCard4 c4Count;
    {
        const tCQCMedia::TNCImgIdList& colImgs = *m_colImgsById[eMType];
        c4Count = colImgs.c4ElemCount();
        strmTar << tCQCMedia::EDataTypes::Image << c4Count;
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            strmTar << *colImgs[c4Index]
                    << tCIDLib::EStreamMarkers::Frame;
        }
    }

    // Do the categories
    {
        const tCQCMedia::TNCCatList& colCats = *m_colCats[eMType];
        c4Count = colCats.c4ElemCount();
        strmTar << tCQCMedia::EDataTypes::Cat << c4Count;
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            strmTar << *colCats[c4Index]
                    << tCIDLib::EStreamMarkers::Frame;
        }
    }

    // Do the items
    {
        const tCQCMedia::TNCItemIdList& colItems = *m_colItemsById[eMType];
        c4Count = colItems.c4ElemCount();
        strmTar << tCQCMedia::EDataTypes::Item << c4Count;
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            strmTar << *colItems[c4Index]
                    << tCIDLib::EStreamMarkers::Frame;
        }
    }

    // Do the collections
    {
        const tCQCMedia::TNCColIdList& colCols = *m_colColsById[eMType];
        c4Count = colCols.c4ElemCount();
        strmTar << tCQCMedia::EDataTypes::Collect << c4Count;
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            strmTar << *colCols[c4Index]
                    << tCIDLib::EStreamMarkers::Frame;
        }
    }

    // Do the title sets
    {
        const tCQCMedia::TNCSetIdList& colSets = *m_colSetsById[eMType];
        c4Count = colSets.c4ElemCount();
        strmTar << tCQCMedia::EDataTypes::TitleSet << c4Count;
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            strmTar << *colSets[c4Index]
                    << tCIDLib::EStreamMarkers::Frame;
        }
    }
}

tCIDLib::TVoid
TMediaDB::FormatXML(const   tCQCMedia::EMediaTypes  eMType
                    ,       TTextOutStream&         strmTar) const
{
    tCIDLib::TCard4 c4Count;

    // Do all of the images
    {
        const tCQCMedia::TNCImgIdList& colImgs = *m_colImgsById[eMType];
        c4Count = colImgs.c4ElemCount();
        strmTar << L"<Imgs Cnt='" << c4Count << L"'>\n";
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            colImgs[c4Index]->ToXML(strmTar);
        strmTar << L"</Imgs>\n";
    }

    // Do all of the categories
    {
        const tCQCMedia::TNCCatList& colCats = *m_colCats[eMType];
        c4Count = colCats.c4ElemCount();
        strmTar << L"<Cats Cnt='" << c4Count << L"'>\n";
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            colCats[c4Index]->ToXML(strmTar);
        strmTar << L"</Cats>\n";
    }

    // Do all of the items
    {
        const tCQCMedia::TNCItemIdList& colItems = *m_colItemsById[eMType];
        c4Count = colItems.c4ElemCount();
        strmTar << L"<Items Cnt='" << c4Count << L"'>\n";
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            colItems[c4Index]->ToXML(strmTar);
        strmTar << L"</Items>\n";
    }

    // Do all of the collections
    {
        const tCQCMedia::TNCColIdList& colCols = *m_colColsById[eMType];
        c4Count = colCols.c4ElemCount();
        strmTar << L"<Cols Cnt='" << c4Count << L"'>\n";
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            colCols[c4Index]->ToXML(strmTar);
        strmTar << L"</Cols>\n";
    }

    // Do all of the titles
    {
        const tCQCMedia::TNCSetIdList& colSets = *m_colSetsById[eMType];
        c4Count = colSets.c4ElemCount();
        strmTar << L"<Sets Cnt='" << c4Count << L"'>\n";
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            colSets[c4Index]->ToXML(strmTar);
        strmTar << L"</Sets>\n";
    }
}


//
//  This can be called for force the loading of artist to title set mappings, for
//  hose folks who want this functionality.
//
tCIDLib::TVoid TMediaDB::LoadByArtistMap()
{
    tCQCMedia::EMediaTypes eMType = tCQCMedia::EMediaTypes::Min;
    for (; eMType <= tCQCMedia::EMediaTypes::Max; eMType++)
    {
        // Get the artist map for this data type
        tCQCMedia::TByArtistList& colCurByArtist = *m_colByArtist[eMType];

        // Clear it out in case it has previous contents
        colCurByArtist.RemoveAll();

        //
        //  Let's loop through all of the title sets and, for each one, we get the artist
        //  name for each of the contained collections. For each name, we see if we have
        //  that name in our list. If not, we add it and add this title set to the map for
        //  that artist.
        //
        tCIDLib::TCard4 c4At;
        const tCQCMedia::TNCSetIdList& colSets = *m_colSetsById[eMType];
        const tCIDLib::TCard4 c4SetCnt = colSets.c4ElemCount();
        for (tCIDLib::TCard4 c4SetInd = 0; c4SetInd < c4SetCnt; c4SetInd++)
        {
            const TMediaTitleSet* pmsetCur = colSets[c4SetInd];

            const tCIDLib::TCard4 c4ColCnt = pmsetCur->c4ColCount();
            for (tCIDLib::TCard4 c4ColInd = 0; c4ColInd < c4ColCnt; c4ColInd++)
            {
                const TMediaCollect& mcolCur = pmsetCur->mcolAt(*this, c4ColInd);
                const TString& strArtist = mcolCur.strArtist();

                //
                //  See if we have this one yet. We need to use the private helper that
                //  does this lookup, since we need a non-const pointer.
                //
                TArtistMap* pcolArtistMap = pcolArtistToMap(eMType, strArtist);
                if (!pcolArtistMap)
                {
                    //
                    //  Not there yet, so add it. We have to add them sorted, not sort after
                    //  the fact because the lookup above depends on them being sorted.
                    //
                    pcolArtistMap = new TArtistMap(strArtist);
                    colCurByArtist.InsertSorted(pcolArtistMap, TArtistMap::eComp, c4At);
                }

                // OK, it's in there one way or another now
                pcolArtistMap->AddTitleSet(pmsetCur);
            }
        }
    }
}


//
//  This is called by the client code after it has completed the load of the DB. We do any
//  post-processing operations.
//
//  Note that this be called more than once, since if the database info is incrementally
//  edited (without a full reload), this has to be called to make sure that everything gets
//  back right again. So this method has to be prepared to be called repeatedly.
//
tCIDLib::TVoid TMediaDB::LoadComplete()
{
    //
    //  Sort our item, collection and title lists by their ids, so that
    //  we can use a binary search for by id lookups.
    //
    tCQCMedia::EMediaTypes eMType = tCQCMedia::EMediaTypes::Min;
    while (eMType <= tCQCMedia::EMediaTypes::Max)
    {
        m_colColsById[eMType]->Sort(TMediaCollect::eCompById);
        m_colImgsById[eMType]->Sort(TMediaImg::eCompById);
        m_colItemsById[eMType]->Sort(TMediaItem::eCompById);
        m_colSetsById[eMType]->Sort(TMediaTitleSet::eCompById);
        eMType++;
    }

    //
    //  Now allow each of the collections and title sets to finalize
    //  themselves and look up/aggregate any info they need.
    //
    while (eMType <= tCQCMedia::EMediaTypes::Max)
    {
        {
            tCQCMedia::TNCColIdList& colCols = *m_colColsById[eMType];
            const tCIDLib::TCard4 c4ColCnt = colCols.c4ElemCount();
            for (tCIDLib::TCard4 c4ColInd = 0; c4ColInd < c4ColCnt; c4ColInd++)
                colCols[c4ColInd]->Finalize(*this);
        }

        {
            tCQCMedia::TNCSetIdList& colSets = *m_colSetsById[eMType];
            const tCIDLib::TCard4 c4SetCnt = colSets.c4ElemCount();
            for (tCIDLib::TCard4 c4SetInd = 0; c4SetInd < c4SetCnt; c4SetInd++)
                colSets[c4SetInd]->Finalize(*this);
        }
    }

    // Indicate that complete has been called
    m_bComplete = kCIDLib::True;
}


//
//  In order to keep the binary dump format stuff hidden away, we provide a load
//  method that allows us to be loaded from a previously generated binary dump.
//  We can take compressed or uncompressed data.
//
tCIDLib::TVoid
TMediaDB::LoadFromBinDump(const     TMemBuf&                mbufData
                          , const   tCIDLib::TCard4         c4DataSz
                            ,       tCQCMedia::EMTFlags&    eMTFlags
                            ,       TString&                strSerNum
                            , const tCIDLib::TBoolean       bCompressed)
{
    // We just call the public, static parsing method and pass ourself as the target
    ParseBinDump
    (
        mbufData, c4DataSz, eMTFlags, strSerNum, *this, bCompressed
    );
}


//
//  These provide for iteration of the various database objects we hold.
//  Normally by id access is used, but in some cases there's just a need
//  to iterate them all.
//
const TMediaCat&
TMediaDB::mcatAt(const  tCQCMedia::EMediaTypes  eType
                , const tCIDLib::TCard4         c4At) const
{
    #if CID_DEBUG_ON
    if (eType >= tCQCMedia::EMediaTypes::Count)
        ThrowBadType(CID_LINE, eType);
    #endif
    return *m_colCats[eType]->pobjAt(c4At);
}

const TMediaCollect&
TMediaDB::mcolAt(const  tCQCMedia::EMediaTypes  eType
                , const tCIDLib::TCard4         c4At) const
{
    #if CID_DEBUG_ON
    if (eType >= tCQCMedia::EMediaTypes::Count)
        ThrowBadType(CID_LINE, eType);
    #endif
    return *m_colColsById[eType]->pobjAt(c4At);
}

const TMediaImg&
TMediaDB::mimgAt(const  tCQCMedia::EMediaTypes  eType
                , const tCIDLib::TCard4         c4At) const
{
    #if CID_DEBUG_ON
    if (eType >= tCQCMedia::EMediaTypes::Count)
        ThrowBadType(CID_LINE, eType);
    #endif
    return *m_colImgsById[eType]->pobjAt(c4At);
}

const TMediaItem&
TMediaDB::mitemAt(  const   tCQCMedia::EMediaTypes  eType
                    , const tCIDLib::TCard4         c4At) const
{
    #if CID_DEBUG_ON
    if (eType >= tCQCMedia::EMediaTypes::Count)
        ThrowBadType(CID_LINE, eType);
    #endif
    return *m_colItemsById[eType]->pobjAt(c4At);
}

const TMediaTitleSet&
TMediaDB::mtsAt(const   tCQCMedia::EMediaTypes  eType
                , const tCIDLib::TCard4         c4At) const
{
    #if CID_DEBUG_ON
    if (eType >= tCQCMedia::EMediaTypes::Count)
        ThrowBadType(CID_LINE, eType);
    #endif
    return *m_colSetsById[eType]->pobjAt(c4At);
}


//
//  A helper to handle the movement of a collection from one title set to another,
//  which isn't uncommon and would otherwise require a lot of fiddling to do from
//  the outside.
//
tCIDLib::TVoid
TMediaDB::MoveItem( const   tCQCMedia::EMediaTypes  eMType
                    , const tCIDLib::TCard2         c2SrcColId
                    , const tCIDLib::TCard2         c2TarColId
                    , const tCIDLib::TCard2         c2ItemId)
{
    TMediaCollect* pmcolSrc = pmcolByIdNC(eMType, c2SrcColId, kCIDLib::True);
    TMediaCollect* pmcolTar = pmcolByIdNC(eMType, c2TarColId, kCIDLib::True);
    TMediaItem* pmitemTar = pmitemByIdNC(eMType, c2ItemId, kCIDLib::True);

    pmcolSrc->RemoveItem(c2ItemId);
    pmcolTar->bAddItem(*pmitemTar);

    // Re-finalize the target and source
    pmcolTar->Finalize(*this);
    pmcolSrc->Finalize(*this);
}


//
//  Sees if we have a by artist map for the indicated artist name. If not, it returns a null
//  pointer. If we have a list for the indicated media type, it is sorted by artist name,
//  so we can do a binary search.
//
const TArtistMap*
TMediaDB::pcolFindArtistMap(const tCQCMedia::EMediaTypes eMType, const TString& strArtist) const
{
    tCQCMedia::TByArtistList& colMap = *m_colByArtist[eMType];

    // If empty, obviously not
    if (colMap.bIsEmpty())
        return nullptr;

    tCIDLib::TCard4 c4At;
    return colMap.pobjKeyedBinarySearch(strArtist, TArtistMap::eCompByKey, c4At);
}



//
//  Find a category by id, UID, or name. For by id search we can do a binary
//  search on the category list object. For the UID or name, since there are never
//  that many categories, we just do a linear search.
//
const TMediaCat*
TMediaDB::pmcatById(const   tCQCMedia::EMediaTypes  eType
                    , const tCIDLib::TCard2         c2IdToFind
                    , const tCIDLib::TBoolean       bThrowIfNot) const
{
    #if CID_DEBUG_ON
    if (eType >= tCQCMedia::EMediaTypes::Count)
        ThrowBadType(CID_LINE, eType);
    #endif

    const tCQCMedia::TNCCatList& colTar = *m_colCats[eType];
    tCIDLib::TCard4 c4At;
    const TMediaCat* pmcatRet = colTar.pobjKeyedBinarySearch
    (
        c2IdToFind, TMediaCat::eIdKeyComp, c4At
    );

    if (!pmcatRet && bThrowIfNot)
    {
        facCQCMedia().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMedErrs::errcDB_UnknownObjId
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , TCardinal(c2IdToFind, tCIDLib::ERadices::Hex)
            , tCQCMedia::strXlatEDataTypes(tCQCMedia::EDataTypes::Cat)
            , tCQCMedia::strXlatEMediaTypes(eType)
        );
    }
    return pmcatRet;
}

const TMediaCat*
TMediaDB::pmcatByName(  const   tCQCMedia::EMediaTypes  eType
                        , const TString&                strToFind
                        , const tCIDLib::TBoolean       bThrowIfNot) const
{
    #if CID_DEBUG_ON
    if (eType >= tCQCMedia::EMediaTypes::Count)
        ThrowBadType(CID_LINE, eType);
    #endif

    const tCQCMedia::TNCCatList& colTar = *m_colCats[eType];
    const tCIDLib::TCard4 c4Count = colTar.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TMediaCat* pmcatCur = colTar[c4Index];
        if (pmcatCur->strName().bCompareI(strToFind))
            return pmcatCur;
    }

    if (bThrowIfNot)
    {
        facCQCMedia().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMedErrs::errcDB_UnknownObjName
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , strToFind
            , tCQCMedia::strXlatEDataTypes(tCQCMedia::EDataTypes::Cat)
            , tCQCMedia::strXlatEMediaTypes(eType)
        );
    }
    return 0;
}

const TMediaCat*
TMediaDB::pmcatByUniqueId(  const   tCQCMedia::EMediaTypes  eType
                            , const TString&                strToFind
                            , const tCIDLib::TBoolean       bThrowIfNot) const
{
    #if CID_DEBUG_ON
    if (eType >= tCQCMedia::EMediaTypes::Count)
        ThrowBadType(CID_LINE, eType);
    #endif

    const tCQCMedia::TNCCatList& colTar = *m_colCats[eType];
    const tCIDLib::TCard4 c4Count = colTar.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TMediaCat* pmcatCur = colTar[c4Index];
        if (pmcatCur->strUniqueId() == strToFind)
            return pmcatCur;
    }

    if (bThrowIfNot)
    {
        facCQCMedia().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMedErrs::errcDB_UnknownObjUID
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , strToFind
            , tCQCMedia::strXlatEDataTypes(tCQCMedia::EDataTypes::Cat)
            , tCQCMedia::strXlatEMediaTypes(eType)
        );
    }
    return 0;
}


//
//  Finds the media collection with the indicated type in the collection
//  list. If it's not of the indicated type, an exception is thrown.
//
const TMediaCollect*
TMediaDB::pmcolById(const   tCQCMedia::EMediaTypes  eType
                    , const tCIDLib::TCard2         c2IdToFind
                    , const tCIDLib::TBoolean       bThrowIfNot) const
{
    #if CID_DEBUG_ON
    if (eType >= tCQCMedia::EMediaTypes::Count)
        ThrowBadType(CID_LINE, eType);
    #endif

    const tCQCMedia::TNCColIdList& colTar = *m_colColsById[eType];
    tCIDLib::TCard4 c4At;
    const TMediaCollect* pmcolRet = colTar.pobjKeyedBinarySearch
    (
        c2IdToFind, TMediaCollect::eIdKeyComp, c4At
    );

    if (!pmcolRet && bThrowIfNot)
    {
        facCQCMedia().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMedErrs::errcDB_UnknownObjId
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , TCardinal(c2IdToFind, tCIDLib::ERadices::Hex)
            , tCQCMedia::strXlatEDataTypes(tCQCMedia::EDataTypes::Collect)
            , tCQCMedia::strXlatEMediaTypes(eType)
        );
    }
    return pmcolRet;
}


// Looks up a collection with the indicated media type and unique id
const TMediaCollect*
TMediaDB::pmcolByUniqueId(  const   tCQCMedia::EMediaTypes  eType
                            , const TString&                strToFind
                            , const tCIDLib::TBoolean       bThrowIfNot) const
{
    #if CID_DEBUG_ON
    if (eType >= tCQCMedia::EMediaTypes::Count)
        ThrowBadType(CID_LINE, eType);
    #endif

    const tCQCMedia::TColNameList& colByUID = *m_colColsByUID[eType];
    const TMediaCollect* pmcolRet = colByUID.pobjFindByKey(strToFind, kCIDLib::False);
    if (!pmcolRet && bThrowIfNot)
    {
        facCQCMedia().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMedErrs::errcDB_UnknownObjUID
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , strToFind
            , tCQCMedia::strXlatEDataTypes(tCQCMedia::EDataTypes::Collect)
            , tCQCMedia::strXlatEMediaTypes(eType)
        );
    }
    return pmcolRet;
}


// Returns the special 'unknown collection' placeholder for the media type
const TMediaCollect* TMediaDB::pmcolUnknown(const tCQCMedia::EMediaTypes eType) const
{
    #if CID_DEBUG_ON
    if (eType >= tCQCMedia::EMediaTypes::Count)
        ThrowBadType(CID_LINE, eType);
    #endif
    return &m_objaUnknownCols[eType];
}


//
//  Finds the art image with the indicated id and of the indicated media
//  type.
//
const TMediaImg*
TMediaDB::pmimgById(const   tCQCMedia::EMediaTypes  eType
                    , const tCIDLib::TCard2         c2IdToFind
                    , const tCIDLib::TBoolean       bThrowIfNot) const
{
    #if CID_DEBUG_ON
    if (eType >= tCQCMedia::EMediaTypes::Count)
        ThrowBadType(CID_LINE, eType);
    #endif

    const tCQCMedia::TNCImgIdList& colTar = *m_colImgsById[eType];
    tCIDLib::TCard4 c4At;
    const TMediaImg* pmimgRet = colTar.pobjKeyedBinarySearch
    (
        c2IdToFind, TMediaImg::eIdKeyComp, c4At
    );

    if (!pmimgRet && bThrowIfNot)
    {
        TString strCookie;
        facCQCMedia().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMedErrs::errcDB_UnknownObjId
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , TCardinal(c2IdToFind, tCIDLib::ERadices::Hex)
            , tCQCMedia::strXlatEDataTypes(tCQCMedia::EDataTypes::Image)
            , tCQCMedia::strXlatEMediaTypes(eType)
        );
    }
    return pmimgRet;
}


// Looks up an image with the indicated media type and unique id
const TMediaImg*
TMediaDB::pmimgByUniqueId(  const   tCQCMedia::EMediaTypes  eType
                            , const TString&                strToFind
                            , const tCIDLib::TBoolean       bThrowIfNot) const
{
    #if CID_DEBUG_ON
    if (eType >= tCQCMedia::EMediaTypes::Count)
        ThrowBadType(CID_LINE, eType);
    #endif

    const TMediaImg* pmimgRet = m_colImgsByUID[eType]->pobjFindByKey(strToFind, kCIDLib::False);
    if (!pmimgRet && bThrowIfNot)
    {
        facCQCMedia().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMedErrs::errcDB_UnknownObjUID
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , strToFind
            , tCQCMedia::strXlatEDataTypes(tCQCMedia::EDataTypes::Image)
            , tCQCMedia::strXlatEMediaTypes(eType)
        );
    }
    return pmimgRet;
}


//
//  Finds the media item with the indicated type in the item list.
//
const TMediaItem*
TMediaDB::pmitemById(const  tCQCMedia::EMediaTypes  eType
                    , const tCIDLib::TCard2         c2IdToFind
                    , const tCIDLib::TBoolean       bThrowIfNot) const
{
    #if CID_DEBUG_ON
    if (eType >= tCQCMedia::EMediaTypes::Count)
        ThrowBadType(CID_LINE, eType);
    #endif

    const tCQCMedia::TNCItemIdList& colTar = *m_colItemsById[eType];
    tCIDLib::TCard4 c4At;
    const TMediaItem* pmitemRet = colTar.pobjKeyedBinarySearch
    (
        c2IdToFind, TMediaItem::eIdKeyComp, c4At
    );

    if (!pmitemRet && bThrowIfNot)
    {
        TString strCookie;
        facCQCMedia().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMedErrs::errcDB_UnknownObjId
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , TCardinal(c2IdToFind, tCIDLib::ERadices::Hex)
            , tCQCMedia::strXlatEDataTypes(tCQCMedia::EDataTypes::Item)
            , tCQCMedia::strXlatEMediaTypes(eType)
        );
    }
    return pmitemRet;
}


//
//  Looks up an item with the indicated media type and unique id
//
const TMediaItem*
TMediaDB::pmitemByUniqueId( const   tCQCMedia::EMediaTypes  eType
                            , const TString&                strToFind
                            , const tCIDLib::TBoolean       bThrowIfNot) const
{
    #if CID_DEBUG_ON
    if (eType >= tCQCMedia::EMediaTypes::Count)
        ThrowBadType(CID_LINE, eType);
    #endif

    const tCQCMedia::TItemNameList& colByUID = *m_colItemsByUID[eType];
    const TMediaItem* pmitemRet = colByUID.pobjFindByKey(strToFind, kCIDLib::False);
    if (!pmitemRet && bThrowIfNot)
    {
        facCQCMedia().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMedErrs::errcDB_UnknownObjUID
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , strToFind
            , tCQCMedia::strXlatEDataTypes(tCQCMedia::EDataTypes::Item)
            , tCQCMedia::strXlatEMediaTypes(eType)
        );
    }
    return pmitemRet;
}


//
//  Find an item by location info, returning either 0 or throwing if not
//  found, according to the bThrowIfNot param. There's no lookup for this,
//  so unfortunately it has just to be a straight search.
//
const TMediaItem*
TMediaDB::pmitemByLocInfo(  const   tCQCMedia::EMediaTypes  eType
                            , const TString&                strToFind
                            , const tCIDLib::TBoolean       bThrowIfNot) const
{
    const tCQCMedia::TNCItemIdList& colTar = *m_colItemsById[eType];
    const tCIDLib::TCard4 c4Count = colTar.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TMediaItem* pmitemCur = colTar[c4Index];
        if (pmitemCur->strLocInfo().bCompareI(strToFind))
            return pmitemCur;
    }

    if (bThrowIfNot)
    {
        facCQCMedia().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMedErrs::errcDB_LocInfoNotFound
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , tCQCMedia::strXlatEDataTypes(tCQCMedia::EDataTypes::Item)
        );
    }
    return 0;
}


// Returns the special 'unknown item' placeholder for the media type
const TMediaItem* TMediaDB::pmitemUnknown(const tCQCMedia::EMediaTypes eType) const
{
    #if CID_DEBUG_ON
    if (eType >= tCQCMedia::EMediaTypes::Count)
        ThrowBadType(CID_LINE, eType);
    #endif
    return &m_objaUnknownItems[eType];
}


// Find a title set with the indicated type and title id.
const TMediaTitleSet*
TMediaDB::pmtsById( const   tCQCMedia::EMediaTypes  eType
                    , const tCIDLib::TCard2         c2IdToFind
                    , const tCIDLib::TBoolean       bThrowIfNot) const
{
    #if CID_DEBUG_ON
    if (eType >= tCQCMedia::EMediaTypes::Count)
        ThrowBadType(CID_LINE, eType);
    #endif

    // Do a binary search of the appropriate by id list
    tCIDLib::TCard4 c4At;
    const tCQCMedia::TNCSetIdList& colById = *m_colSetsById[eType];
    const TMediaTitleSet* pmtsRet = colById.pobjKeyedBinarySearch
    (
        c2IdToFind, TMediaTitleSet::eIdKeyComp, c4At
    );

    if (!pmtsRet && bThrowIfNot)
    {
        facCQCMedia().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMedErrs::errcDB_UnknownObjId
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , TCardinal(c2IdToFind, tCIDLib::ERadices::Hex)
            , tCQCMedia::strXlatEDataTypes(tCQCMedia::EDataTypes::TitleSet)
            , tCQCMedia::strXlatEMediaTypes(eType)
        );
    }
    return pmtsRet;
}


// Looks up a title with the indicated media type and unique id
const TMediaTitleSet*
TMediaDB::pmtsByUniqueId(const  tCQCMedia::EMediaTypes  eType
                        , const TString&                strToFind
                        , const tCIDLib::TBoolean       bThrowIfNot) const
{
    #if CID_DEBUG_ON
    if (eType >= tCQCMedia::EMediaTypes::Count)
        ThrowBadType(CID_LINE, eType);
    #endif

    const tCQCMedia::TSetNameList& colByUID = *m_colSetsByUID[eType];
    const TMediaTitleSet* pmtsRet = colByUID .pobjFindByKey(strToFind, kCIDLib::False);
    if (!pmtsRet && bThrowIfNot)
    {
        facCQCMedia().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMedErrs::errcDB_UnknownObjUID
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , strToFind
            , tCQCMedia::strXlatEDataTypes(tCQCMedia::EDataTypes::TitleSet)
            , tCQCMedia::strXlatEMediaTypes(eType)
        );
    }
    return pmtsRet;
}


//
//  Find the title set that contains the passed collection. They always
//  have a one to one relationship, so there's only one.
//
const TMediaTitleSet*
TMediaDB::pmtsFindContainingSet(const   tCQCMedia::EMediaTypes  eMType
                                , const TMediaCollect&          mcolSrc
                                ,       tCIDLib::TCard2&        c2Index
                                , const tCIDLib::TBoolean       bThrowIfNot) const
{
    const tCQCMedia::TNCSetIdList& colSets = *m_colSetsById[eMType];
    const tCIDLib::TCard4 c4SetCnt = colSets.c4ElemCount();

    const tCIDLib::TCard2 c2ColId = mcolSrc.c2Id();
    for (tCIDLib::TCard4 c4SetInd = 0; c4SetInd < c4SetCnt; c4SetInd++)
    {
        const TMediaTitleSet* pmtsCur = colSets[c4SetInd];
        if (pmtsCur->bContainsCol(c2ColId, c2Index))
            return pmtsCur;
    }

    if (bThrowIfNot)
    {
        facCQCMedia().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMedErrs::errcDB_FindContainer
            , mcolSrc.strName()
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , tCQCMedia::strXlatEDataTypes(tCQCMedia::EDataTypes::TitleSet)
            , tCQCMedia::strXlatEDataTypes(tCQCMedia::EDataTypes::Collect)
        );
    }
    return 0;
}


//
//  The user may be given the option to remove all containers (sets/collects) that
//  have become empty due to changes in the database. This is called to do that work.
//
tCIDLib::TVoid TMediaDB::PruneEmptyContainers()
{
    // <TBD> Implement me
}


//
//  This is called to prune the hierarchical contents under a given level. The caller
//  passes us a media type and the id of that piece of data, which is either a title
//  set or collection.
//
//  We work our way down through all the contained content and remove it. So, for
//  instance, if we get a title set, we remove all the collections from it, and all of
//  the items from those collections, and then the title set.
//
//  The exception is if the target collection is a playlist, in which case we don't
//  remove any items since the playlist just references them.
//
//  We shouldn't be called for items.
//
tCIDLib::TVoid
TMediaDB::PruneHierarchy(const  tCQCMedia::EMediaTypes  eType
                        , const tCIDLib::TBoolean       bIsTitle
                        , const tCIDLib::TCard2         c2Id)
{
    //
    //  If we are at the title set level, we have to iterate the collections, and
    //  just recurse back on ourself for each collection. If at the collection level
    //  we just remove the items and then the collection.
    //
    if (bIsTitle)
    {
        // Look up the title set
        TMediaTitleSet* pmtsTar = pmtsByIdNC(eType, c2Id, kCIDLib::False);
        if (!pmtsTar)
            return;

        // Recurse on each of the collections
        tCIDLib::TCard4 c4Count = pmtsTar->c4ColCount();
        tCIDLib::TCard4 c4Index = 0;
        while (c4Index < c4Count)
        {
            // Look up this collection
            TMediaCollect* pmcolCur = pmcolByIdNC
            (
                eType, pmtsTar->c2ColIdAt(0), kCIDLib::False
            );

            if (pmcolCur)
            {
                PruneHierarchy(eType, kCIDLib::False, pmcolCur->c2Id());
                c4Count--;
            }
             else
            {
                c4Index++;
            }
        }

        // Now remove the title set
        bRemoveObject(eType, tCQCMedia::EDataTypes::TitleSet, pmtsTar->c2Id());
    }
     else
    {
        // Look up the collection
        TMediaCollect* pmcolTar = pmcolByIdNC(eType, c2Id, kCIDLib::False);
        if (!pmcolTar)
            return;

        // If not a playlist, then remove items
        if (!pmcolTar->bIsPlayList())
        {
            tCIDLib::TCard4 c4Count = pmcolTar->c4ItemCount();
            tCIDLib::TCard4 c4Index = 0;
            while (c4Index < c4Count)
            {
                // Look up this item. If found remove it
                TMediaItem* pmitemCur = pmitemByIdNC
                (
                    eType, pmcolTar->c2ItemIdAt(c4Index), kCIDLib::False
                );

                if (pmitemCur)
                {
                    bRemoveObject(eType, tCQCMedia::EDataTypes::Item, pmitemCur->c2Id());
                    c4Count--;
                }
                else
                {
                    c4Index++;
                }
            }
        }

        // And now remove the collection
        bRemoveObject(eType, tCQCMedia::EDataTypes::Collect, pmcolTar->c2Id());
    }
}


// Query the available changers
tCIDLib::TVoid TMediaDB::QueryChangers(TVector<TString>& colMonikersToFill)
{
    colMonikersToFill.RemoveAll();

    //
    //  For movies and music, we'll look for any changer based collections
    //  and store a unique list.
    //
    tCIDLib::TCard4 c4RetCnt = 0;
    tCQCMedia::EMediaTypes eType = tCQCMedia::EMediaTypes::Min;
    TString strMoniker;
    for (; eType < tCQCMedia::EMediaTypes::Count; eType++)
    {
        // Skip pictures on this one
        if (eType == tCQCMedia::EMediaTypes::Pic)
            continue;

        const tCQCMedia::TNCColIdList& colTar = *m_colColsById[eType];
        const tCIDLib::TCard4 c4Count = colTar.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const TMediaCollect* pmcolCur = colTar[c4Index];

            // If not a changer based, skip it
            if (pmcolCur->eLocType() != tCQCMedia::ELocTypes::Changer)
                continue;

            //
            //  See if it starts with one of the changers we've already
            //  got. If so, then skip it. Else add the changer to our
            //  list.
            //
            const TString& strLoc = pmcolCur->strLocInfo();
            const tCIDLib::TCard4 c4MCnt = colMonikersToFill.c4ElemCount();
            tCIDLib::TCard4 c4MInd = 0;
            for (; c4MInd < c4MCnt; c4MInd++)
            {
                if (strLoc.bStartsWith(colMonikersToFill[c4MInd]))
                    break;
            }

            if (c4MInd == c4MCnt)
            {
                tCIDLib::TCard4 c4Ofs;
                if (strLoc.bFirstOccurrence(kCIDLib::chPeriod, c4Ofs))
                {
                    strMoniker = strLoc;
                    strMoniker.CapAt(c4Ofs);
                    colMonikersToFill.objAdd(strMoniker);
                }
            }
        }
    }
}


// Just returns some basic stats about the database
tCIDLib::TVoid
TMediaDB::QueryStats(tCIDLib::TCard4&   c4MovieSetCnt
                    , tCIDLib::TCard4&  c4MovieColCnt
                    , tCIDLib::TCard4&  c4MovieItemCnt
                    , tCIDLib::TCard4&  c4MovieArtCnt
                    , tCIDLib::TCard4&  c4MusicSetCnt
                    , tCIDLib::TCard4&  c4MusicColCnt
                    , tCIDLib::TCard4&  c4MusicItemCnt
                    , tCIDLib::TCard4&  c4MusicArtCnt) const
{
    c4MovieSetCnt = m_colSetsById[tCQCMedia::EMediaTypes::Movie]->c4ElemCount();
    c4MusicSetCnt = m_colSetsById[tCQCMedia::EMediaTypes::Music]->c4ElemCount();

    c4MovieColCnt = m_colColsById[tCQCMedia::EMediaTypes::Movie]->c4ElemCount();
    c4MusicColCnt = m_colColsById[tCQCMedia::EMediaTypes::Music]->c4ElemCount();

    c4MovieItemCnt = m_colItemsById[tCQCMedia::EMediaTypes::Movie]->c4ElemCount();
    c4MusicItemCnt = m_colItemsById[tCQCMedia::EMediaTypes::Music]->c4ElemCount();

    c4MovieArtCnt = m_colImgsById[tCQCMedia::EMediaTypes::Movie]->c4ElemCount();
    c4MusicArtCnt = m_colImgsById[tCQCMedia::EMediaTypes::Music]->c4ElemCount();
}


//
//  Looks up the combination of all of the categories of the collections owned
//  by the passed title set. We return all of their ids of those categories.
//
tCIDLib::TVoid
TMediaDB::QueryTitleCats(const  tCQCMedia::EMediaTypes  eType
                        , const tCIDLib::TCard2         c2SetId
                        ,       tCQCMedia::TIdList&     fcolToFill) const
{
    fcolToFill.RemoveAll();
    const TMediaTitleSet* pmtsSrc = pmtsById(eType, c2SetId, kCIDLib::True);

    const tCIDLib::TCard4 c4ColCount = pmtsSrc->c4ColCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ColCount; c4Index++)
    {
        const TMediaCollect* pmcolCur = pmcolById
        (
            eType, pmtsSrc->c2ColIdAt(c4Index), kCIDLib::False
        );

        if (pmcolCur)
            pmcolCur->AddCatsToList(fcolToFill);
    }
}


//
//  This method will return up to the indicated number of media items of the
//  indicated category, randomly selected. It's used to support a randomcategory
//  based play mode in media players.
//
//  We can't set the playlist item ids here, since only the caller knows what they
//  should be (since they are unique to a given playlist manager, and these may be
//  getting added to a list with some unknown current sequend of ids.) So we just set
//  them to zero for now. He'll set them when he gets this list back.
//
tCIDLib::TVoid
TMediaDB::RandomCatQuery(const  TString&                    strRepoMoniker
                        , const tCIDLib::TCard2             c2CatId
                        , const tCIDLib::TCard4             c4MaxItems
                        ,       TVector<TCQCMediaPLItem>&   colToFill) const
{
    colToFill.RemoveAll();

    // Find the media category for the indicated id
    const TMediaCat* pmcatSrc = pmcatById(tCQCMedia::EMediaTypes::Music, c2CatId, kCIDLib::True);

    // And let's go through the titles and find ones in this category
    const tCQCMedia::TNCSetIdList& colById = *m_colSetsById[tCQCMedia::EMediaTypes::Music];
    const tCIDLib::TCard4 c4Count = colById.c4ElemCount();

    //
    //  Clip back the max if too large or larger than the actual count of
    //  items available in total for this category.
    //
    tCIDLib::TCard4 c4RealMax = c4MaxItems;
    if (c4RealMax > 50)
        c4RealMax = 50;

    // Build up a list of all the titles in the indicated category
    tCQCMedia::TSetIdList  colSets(tCIDLib::EAdoptOpts::NoAdopt, c4RealMax);
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TMediaTitleSet* pmtsCur = colById[c4Index];
        if (bTitleIsInCat(*pmtsCur, c2CatId))
            colSets.Add(pmtsCur);
    }

    // Create a random number generator
    TRandomNum randSel;
    randSel.Seed(TTime::c4Millis());

    // We'll us a hash set to avoid selecting the same item more than one
    tCIDLib::TStrHashSet colDupList(109, TStringKeyOps());

    //
    //  Now we will randomly select a title set, then randomly select a
    //  collection within it, then randomly select an item within that.
    //
    //  There is a possibility that we will go on for a long time without
    //  really finding what we want. So we'll limit the number of rounds to
    //  five times the real max.
    //
    TCQCMediaPLItem mpliNew;
    TString strColCookie;
    TString strTitleCookie;
    TString strItemCookie;
    tCIDLib::TCard4 c4SoFar = 0;
    tCIDLib::TCard4 c4Rounds = 0;
    while ((c4SoFar < c4RealMax) && (c4Rounds < (c4RealMax * 5)))
    {
        // Bump our round count each time
        c4Rounds++;

        // Get a random title
        const tCIDLib::TCard4 c4TitleInd = randSel.c4GetNextNum() % colSets.c4ElemCount();
        const TMediaTitleSet* pmtsCur = colSets[c4TitleInd];

        // Get a collection within that
        const tCIDLib::TCard4 c4ColCnt = pmtsCur->c4ColCount();
        tCIDLib::TCard4 c4ColInd = 0;
        if (c4ColCnt > 1)
            c4ColInd = randSel.c4GetNextNum() % c4ColCnt;
        const TMediaCollect* pmcolCur = pmcolById
        (
            tCQCMedia::EMediaTypes::Music, pmtsCur->c2ColIdAt(c4ColInd), kCIDLib::False
        );

        // And select an item from the collection
        const tCIDLib::TCard4 c4ItemCnt = pmcolCur->c4ItemCount();
        tCIDLib::TCard4 c4ItemInd = 0;
        if (c4ItemCnt > 1)
            c4ItemInd = randSel.c4GetNextNum() % c4ItemCnt;
        const TMediaItem* pmitemCur = pmitemById
        (
            tCQCMedia::EMediaTypes::Music, pmcolCur->c2ItemIdAt(c4ItemInd), kCIDLib::False
        );

        // Build up an item cookie for this guy and see if it's already used
        facCQCMedia().FormatItemCookie
        (
            tCQCMedia::EMediaTypes::Music
            , c2CatId
            , pmtsCur->c2Id()
            , tCIDLib::TCard2(c4ColInd + 1)
            , tCIDLib::TCard2(c4ItemInd + 1)
            , strItemCookie
        );

        // If already in the list, do nothing
        if (colDupList.bHasElement(strItemCookie))
            continue;

        //
        //  Ok, let's take this one. Build up the rest of the cookies that
        //  we have to provide.
        //
        facCQCMedia().FormatColCookie
        (
            tCQCMedia::EMediaTypes::Music
            , c2CatId
            , pmtsCur->c2Id()
            , tCIDLib::TCard2(c4ColInd + 1)
            , strColCookie
        );

        facCQCMedia().FormatTitleCookie
        (
            tCQCMedia::EMediaTypes::Music
            , c2CatId
            , pmtsCur->c2Id()
            , strTitleCookie
        );

        mpliNew.Set
        (
            pmitemCur->c4BitDepth()
            , pmitemCur->c4BitRate()
            , pmitemCur->c4Channels()
            , pmitemCur->c4Duration()
            , pmcolCur->c4Year()
            , pmcolCur->eLocType()
            , tCQCMedia::EMediaTypes::Music
            , pmcolCur->strArtist()
            , pmcolCur->strAspectRatio()
            , pmcolCur->strCast()
            , strColCookie
            , pmcolCur->strName()
            , pmcolCur->strRating()
            , pmcolCur->strDescr()
            , pmcolCur->strLabel()
            , pmcolCur->strLeadActor()
            , pmitemCur->strLocInfo()
            , pmitemCur->strArtist()
            , strItemCookie
            , pmitemCur->strName()
            , pmcolCur->strMediaFormat()
            , strRepoMoniker
            , strTitleCookie
            , pmtsCur->strName()
            , 0
        );
        colToFill.objAdd(mpliNew);

        // Bump the count done so far
        c4SoFar++;
    }
}


//
//  A helper that will re-finalize all of the containers that reference
//  an item. It's called after an item has been modified in some way.
//  It will also re-sort the item related sorted lists to make sure they
//  stay correct.
//
tCIDLib::TVoid
TMediaDB::RefinalizeItemConts(const TMediaItem& mitemSrc)
{
    tCQCMedia::TNCColIdList colCols(tCIDLib::EAdoptOpts::NoAdopt, 1024);
    tCQCMedia::TNCSetIdList colSets(tCIDLib::EAdoptOpts::NoAdopt, 1024);

    FindContainersNC
    (
        mitemSrc.eType(), tCQCMedia::EDataTypes::Item, mitemSrc.c2Id(), colCols, colSets
    );

    // Do any collections first, then sets
    const tCIDLib::TCard4 c4ColCnt = colCols.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ColCnt; c4Index++)
        colCols[c4Index]->Finalize(*this);

    const tCIDLib::TCard4 c4SetCnt = colSets.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4SetCnt; c4Index++)
        colSets[c4Index]->Finalize(*this);

    // Re-sort the item related lists
    m_colItemsById[mitemSrc.eType()]->Sort(TMediaItem::eCompById);
}


//
//  Called when a category is removed, to remove all references to this category
//  so that there aren't stranded ids.
//
//  Categories are stored at the collection level, so we go through all the
//  collections of this media type and remove any references to this category id.
//
//  No need to reset transient views here since they don't use categories.
//
tCIDLib::TVoid TMediaDB::RemoveAllCatRefs(  const   tCQCMedia::EMediaTypes  eType
                                            , const tCIDLib::TCard2         c2CatId)
{
    #if CID_DEBUG_ON
    if (eType >= tCQCMedia::EMediaTypes::Count)
        ThrowBadType(CID_LINE, eType);
    #endif

    tCQCMedia::TNCColIdList& colCols = *m_colColsById[eType];
    const tCIDLib::TCard4 c4Count = colCols.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        colCols[c4Index]->RemoveCategory(c2CatId);
}


//
//  Called when we want to remove an item from a collection, but not remove the
//  item itself. We refinalize the collection if we remove.
//
tCIDLib::TVoid
TMediaDB::RemoveFromCol(const   TMediaCollect&      mcolSrc
                        , const tCIDLib::TCard2     c2ItemId)
{
    const tCQCMedia::EMediaTypes eType = mcolSrc.eType();

    TMediaCollect* pmcolCur = pmcolByIdNC(eType, mcolSrc.c2Id(), kCIDLib::False);

    // If we found the collection and it contains the item, remove it
    tCIDLib::TCard2 c2ItemInd;
    if (pmcolCur && pmcolCur->bContainsItem(c2ItemId, c2ItemInd))
    {
        pmcolCur->RemoveItem(c2ItemId);
        pmcolCur->Finalize(*this);

        // Reset any transient views just in case, though likely they were not affected
        ResetTransientViews();
    }
}


//
//  Called when we want to remove a collection a title set, but not remove the
//  col itself. This typically isn't needed since collections are never multiply
//  referenced. We refinalize the set if we remove.
//
tCIDLib::TVoid
TMediaDB::RemoveFromSet(const   TMediaTitleSet&     mtsSrc
                        , const tCIDLib::TCard2     c2ColId)
{
    const tCQCMedia::EMediaTypes eType = mtsSrc.eType();

    // Look up the set by its id to get a writeable version
    TMediaTitleSet* pmtsSrc = pmtsByIdNC(eType, mtsSrc.c2Id(), kCIDLib::True);

    // Remove the collection from it and refinalize
    tCIDLib::TCard2 c2Index;
    if (pmtsSrc->bContainsCol(c2ColId, c2Index))
    {
        pmtsSrc->RemoveColAt(c2Index);
        pmtsSrc->Finalize(*this);
    }
}



//
//  Called to rename any type of media database object. We do all the
//  work necessary to make the change, if it's required, and keep everything
//  up to date.
//
tCIDLib::TVoid
TMediaDB::RenameObject( const   tCQCMedia::EMediaTypes  eType
                        , const tCQCMedia::EDataTypes   eDataType
                        , const tCIDLib::TCard2         c2Id
                        , const TString&                strNew)
{
    if (eType >= tCQCMedia::EMediaTypes::Count)
        ThrowBadType(CID_LINE, eType);

    //
    //  Special case categories, since the UID is just the upper cased
    //  name of the category. Therefore we can't just rename it. We also
    //  have to change its unique id, and so we have to check for possible
    //  dups.
    //
    //
    //  !!!!!!!!
    //  We currently don't have a sorted unique id list for categories,
    //  so we don't have to re-sort anything to do this. If one were to
    //  added, we'd need to deal with that here.
    //
    if (eDataType == tCQCMedia::EDataTypes::Cat)
    {
        TString strUID(strNew);
        TMediaCat* pmcatNew = pmcatByUniqueIdNC(eType, strUID, kCIDLib::False);
        if (pmcatNew)
        {
            facCQCMedia().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kMedErrs::errcDB_DupMediaId
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Duplicate
                , strNew
                , tCQCMedia::strXlatEDataTypes(eDataType)
            );
        }

        // And just ago ahead and handle the rest of it here
        strUID = strNew;
        strUID.ToUpper();
        pmcatNew->strName(strNew);
        pmcatNew->strUniqueId(strUID);
        return;
    }


    //
    //  Handle the rest of the types. None of these changes any sort order
    //  stuff, since we are changing the name, not the unique id.
    //
    switch(eDataType)
    {
        case tCQCMedia::EDataTypes::Cat :
            // Already done above
            break;

        case tCQCMedia::EDataTypes::Collect :
        {
            TMediaCollect* pmcolRen = pmcolByIdNC(eType, c2Id, kCIDLib::True);
            if (pmcolRen->strName() != strNew)
                pmcolRen->strName(strNew);
            break;
        }

        case tCQCMedia::EDataTypes::Image :
        {
            //
            //  Images aren't named, so do nothing. And their unique id is
            //  the art path.
            //
            break;
        }

        case tCQCMedia::EDataTypes::Item :
        {
            TMediaItem* pmitemRen = pmitemByIdNC(eType, c2Id, kCIDLib::True);
            if (pmitemRen->strName() != strNew)
                pmitemRen->strName(strNew);
            break;
        }

        case tCQCMedia::EDataTypes::TitleSet :
        {
            TMediaTitleSet* pmtsRen = pmtsByIdNC(eType, c2Id, kCIDLib::True);
            if (pmtsRen->strName() != strNew)
                pmtsRen->strName(strNew);
            break;
        }

        default :
            facCQCMedia().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kMedErrs::errcDB_UnknownDataType
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Unknown
                , TInteger(tCIDLib::i4EnumOrd(eDataType))
            );
            break;
    };
}


//
//  Resets the database back to default values. It removes all the hierarchy and resets the
//  category list back to just any prefab categories.
//
tCIDLib::TVoid TMediaDB::Reset()
{
    // Indicate no longer complete
    m_bComplete = kCIDLib::False;

    // Flush the main collection lists
    tCQCMedia::EMediaTypes eMType = tCQCMedia::EMediaTypes::Min;
    while (eMType <= tCQCMedia::EMediaTypes::Max)
    {
        m_colCats[eMType]->RemoveAll();
        m_colColsById[eMType]->RemoveAll();
        m_colColsByUID[eMType]->RemoveAll();
        m_colImgsById[eMType]->RemoveAll();
        m_colImgsByUID[eMType]->RemoveAll();
        m_colItemsById[eMType]->RemoveAll();
        m_colItemsByUID[eMType]->RemoveAll();
        m_colSetsById[eMType]->RemoveAll();
        m_colSetsByUID[eMType]->RemoveAll();

        eMType++;
    }

    // Reset any transient views so they will fault back in if needed
    ResetTransientViews();
}


//
//  We have the 'all xxx' category cookies already built, so we make them
//  available to client code for convenience.
//
const TString&
TMediaDB::strAllCatCookieFor(const tCQCMedia::EMediaTypes eForType) const
{
    if (eForType >= tCQCMedia::EMediaTypes::Count)
        ThrowBadType(CID_LINE, eForType);

    return m_objaAllCatCookies[eForType];
}


//
//  This is used to change the external location of movies, so that the user can
//  change the location that the database exposes them via.
//
tCIDLib::TBoolean
TMediaDB::bChangeExtMediaPaths( const   TString&            strFind
                                , const TString&            strRepWith
                                ,       tCIDLib::TCard4&    c4ModCnt)
{
    tCIDLib::TCard4 c4At;
    TString         strCur;

    c4ModCnt = 0;
    tCIDLib::TBoolean bRet = kCIDLib::False;

    const tCIDLib::TCard4 c4Count = c4CollectCnt(tCQCMedia::EMediaTypes::Movie);
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TMediaCollect& mcolCur = mcolAtNC(tCQCMedia::EMediaTypes::Movie, c4Index);

        // If it's a collection level location, let's update it
        if (mcolCur.eLocType() == tCQCMedia::ELocTypes::FileCol)
        {
            c4At = 0;
            strCur = mcolCur.strLocInfo();

            // Do the replacement if needed
            const tCIDLib::TBoolean bChanged = strCur.bReplaceSubStr
            (
                strFind, strRepWith, c4At, kCIDLib::True, kCIDLib::False
            );

            // If we changed it, put it back
            if (bChanged)
            {
                bRet = kCIDLib::True;
                mcolCur.strLocInfo(strCur);
                c4ModCnt++;
            }
        }
    }
    return bRet;
}


//
//  These are helpers for stuff that would commonly otherwise require that an
//  object be copied out, updated and then passed back to an UpdateXXX() method.
//  These help handle those common scenarios more efficiently. They also provide
//  the persistent id, which has to change if new art is being set.
//

//
//  Note that we assume the caller will update the persistent id if needed to match
//  the new art data.
//
tCIDLib::TVoid
TMediaDB::SetArt(const  tCIDLib::TCard2         c2ImgId
                , const tCQCMedia::EMediaTypes  eMType
                , const tCQCMedia::ERArtTypes   eArtType
                , const TMemBuf&                mbufToSet
                , const tCIDLib::TCard4         c4Bytes)
{
    TMediaImg* pmimgTar = pmimgByIdNC(eMType, c2ImgId, kCIDLib::True);
    pmimgTar->SetArt(c4Bytes, mbufToSet, eArtType);
}

tCIDLib::TVoid
TMediaDB::SetArt(const  tCIDLib::TCard2         c2ImgId
                , const tCQCMedia::EMediaTypes  eMType
                , const tCQCMedia::ERArtTypes   eArtType
                ,       TBinInStream&           strmSrc
                , const tCIDLib::TCard4         c4Bytes)
{
    TMediaImg* pmimgTar = pmimgByIdNC(eMType, c2ImgId, kCIDLib::True);
    pmimgTar->SetArt(strmSrc, c4Bytes, eArtType);
}

tCIDLib::TVoid
TMediaDB::SetArt(const  tCIDLib::TCard2         c2ImgId
                , const tCQCMedia::EMediaTypes  eMType
                , const tCQCMedia::ERArtTypes   eArtType
                , const TMemBuf&                mbufToSet
                , const tCIDLib::TCard4         c4Bytes
                , const TString&                strPath)
{
    TMediaImg* pmimgTar = pmimgByIdNC(eMType, c2ImgId, kCIDLib::True);
    pmimgTar->SetArt(c4Bytes, mbufToSet, eArtType);
    pmimgTar->SetArtPath(strPath, eArtType);
}

tCIDLib::TVoid
TMediaDB::SetArt(const  tCIDLib::TCard2         c2ImgId
                , const tCQCMedia::EMediaTypes  eMType
                , const tCQCMedia::ERArtTypes   eArtType
                ,       TBinInStream&           strmSrc
                , const tCIDLib::TCard4         c4Bytes
                , const TString&                strPath)
{
    TMediaImg* pmimgTar = pmimgByIdNC(eMType, c2ImgId, kCIDLib::True);
    pmimgTar->SetArt(strmSrc, c4Bytes, eArtType);
    pmimgTar->SetArtPath(strPath, eArtType);
}


tCIDLib::TVoid
TMediaDB::SetArtId( const   tCIDLib::TCard2         c2Id
                    , const tCQCMedia::EMediaTypes  eMType
                    , const tCIDLib::TBoolean       bTitle
                    , const tCIDLib::TCard2         c2ArtId)
{
    if (bTitle)
    {
        TMediaTitleSet* pmtsTar = pmtsByIdNC(eMType, c2Id, kCIDLib::True);
        pmtsTar->c2ArtId(c2ArtId);
    }
     else
    {
        TMediaCollect* pmcolTar = pmcolByIdNC(eMType, c2Id, kCIDLib::True);
        pmcolTar->c2ArtId(c2ArtId);
    }
}

tCIDLib::TVoid
TMediaDB::SetArtPath(const  tCIDLib::TCard2         c2ImgId
                    , const tCQCMedia::EMediaTypes  eMType
                    , const tCQCMedia::ERArtTypes   eArtType
                    , const TString&                strToSet)
{
    TMediaImg* pmimgTar = pmimgByIdNC(eMType, c2ImgId, kCIDLib::True);
    pmimgTar->SetArtPath(strToSet, eArtType);
}

tCIDLib::TVoid
TMediaDB::SetDescription(const  tCIDLib::TCard2         c2Id
                        , const tCQCMedia::EMediaTypes  eMType
                        , const TString&                strToSet)
{
    TMediaCollect* pmcolTar = pmcolByIdNC(eMType, c2Id, kCIDLib::True);
    pmcolTar->strDescr(strToSet);
}

tCIDLib::TVoid
TMediaDB::SetLocInfo(const  tCIDLib::TCard2         c2Id
                    , const tCQCMedia::EMediaTypes  eMType
                    , const tCQCMedia::EDataTypes   eDType
                    , const TString&                strToSet)
{
    CIDAssert
    (
        (eDType == tCQCMedia::EDataTypes::Item) || (eDType == tCQCMedia::EDataTypes::Collect)
        , L"Can only set loc info for items or collections"
    );

    if (eDType == tCQCMedia::EDataTypes::Item)
    {
        TMediaItem* pmitemTar = pmitemByIdNC(eMType, c2Id, kCIDLib::True);
        pmitemTar->strLocInfo(strToSet);
    }
     else
    {
        TMediaCollect* pmcolTar = pmcolByIdNC(eMType, c2Id, kCIDLib::True);
        pmcolTar->strLocInfo(strToSet);
    }
}

tCIDLib::TVoid
TMediaDB::SetSeqNum(const   tCIDLib::TCard2         c2SetId
                    , const tCQCMedia::EMediaTypes  eMType
                    , const tCIDLib::TCard4         c4ToSet)
{
    TMediaTitleSet* pmtsTar = pmtsByIdNC(eMType, c2SetId, kCIDLib::True);
    pmtsTar->c4SeqNum(c4ToSet);

}

const TMediaTitleSet*
TMediaDB::pmtsSetTitleRating(const  tCIDLib::TCard2         c2SetId
                            , const tCQCMedia::EMediaTypes  eMType
                            , const tCIDLib::TCard4         c4ToSet)
{
    TMediaTitleSet* pmtsTar = pmtsByIdNC(eMType, c2SetId, kCIDLib::True);

    // Ignore it if it is not within range
    if (pmtsTar && (c4ToSet > 0) && (c4ToSet < 11))
        pmtsTar->c4UserRating(c4ToSet);

    return pmtsTar;
}


//
//  This is an efficient means to allow the drivers to load into an offline
//  database and then copy the contents to their own database. So they pass
//  us that offline database, and we adopt the contents out of it. leaving
//  it empty.
//
tCIDLib::TVoid TMediaDB::TakeFrom(TMediaDB& mdbSrc)
{
    // Clear ourself out, also resets any transient views
    Reset();

    // Copy over all the data for each media type
    tCQCMedia::EMediaTypes eType = tCQCMedia::EMediaTypes::Min;
    for (; eType < tCQCMedia::EMediaTypes::Count; eType++)
    {
        //
        //  For the by id lists (the adopting ones), we just move the contents of the/
        //  sources list to our lists.
        //
        *m_colCats[eType] = tCIDLib::ForceMove(*mdbSrc.m_colCats[eType]);
        *m_colColsById[eType] = tCIDLib::ForceMove(*mdbSrc.m_colColsById[eType]);
        *m_colImgsById[eType] = tCIDLib::ForceMove(*mdbSrc.m_colImgsById[eType]);
        *m_colItemsById[eType] = tCIDLib::ForceMove(*mdbSrc.m_colItemsById[eType]);
        *m_colSetsById[eType] = tCIDLib::ForceMove(*mdbSrc.m_colSetsById[eType]);

        //
        //  For the by hash set collections, just run through the items we
        //  just put into the by id collection and add each of them
        //  to the hash collection. These are  non-adopting so it's fine
        //  to just put copies into them.
        //
        {
            tCQCMedia::TNCColIdList& colSrc = *m_colColsById[eType];
            const tCIDLib::TCard4 c4CCount = colSrc.c4ElemCount();
            for (tCIDLib::TCard4 c4CInd = 0; c4CInd < c4CCount; c4CInd++)
                m_colColsByUID[eType]->Add(colSrc[c4CInd]);
        }

        {
            tCQCMedia::TNCImgIdList& colSrc = *m_colImgsById[eType];
            const tCIDLib::TCard4 c4ICount = colSrc.c4ElemCount();
            for (tCIDLib::TCard4 c4IInd = 0; c4IInd < c4ICount; c4IInd++)
            {
                TMediaImg* pmimgCur = colSrc[c4IInd];
                m_colImgsByUID[eType]->Add(pmimgCur);
            }
        }

        {
            tCQCMedia::TNCItemIdList& colSrc = *m_colItemsById[eType];
            const tCIDLib::TCard4 c4ICount = colSrc.c4ElemCount();
            for (tCIDLib::TCard4 c4IInd = 0; c4IInd < c4ICount; c4IInd++)
                m_colItemsByUID[eType]->Add(colSrc[c4IInd]);
        }

        {
            tCQCMedia::TNCSetIdList& colSrc = *m_colSetsById[eType];
            const tCIDLib::TCard4 c4SCount = colSrc.c4ElemCount();
            for (tCIDLib::TCard4 c4SInd = 0; c4SInd < c4SCount; c4SInd++)
                m_colSetsByUID[eType]->Add(colSrc[c4SInd]);
        }
    }

    // And mark us as complete if the source was
    m_bComplete  = mdbSrc.m_bComplete;
}



tCIDLib::TVoid
TMediaDB::ThrowBadType( const   tCIDLib::TCard4         c4Line
                        , const tCQCMedia::EMediaTypes  eType) const
{
    facCIDLib().ThrowErr
    (
        CID_FILE
        , c4Line
        , kCIDErrs::errcGen_BadEnumValue
        , tCIDLib::ESeverities::Failed
        , tCIDLib::EErrClasses::AppError
        , TInteger(tCIDLib::i4EnumOrd(eType))
        , TString(L"tCQCMedia::EMediaTypes")
    );
}


//
//  These are called to update the objects in the database. These must be fully set
//  up objects that have been gotten from the database previously, so they have to
//  have id and unique id set up, for images the persistent id must be there, and
//  so forth.
//
tCIDLib::TVoid TMediaDB::UpdateCategory(const TMediaCat& mcatNew)
{
    // Verify it's decent
    TMediaCat* pmcatTar = static_cast<TMediaCat*>
    (
        pmddbCheckCanUpdate(mcatNew, tCQCMedia::EDataTypes::Cat)
    );

    // Looks ok, so update it
    *pmcatTar = mcatNew;
}

tCIDLib::TVoid TMediaDB::UpdateCollect(const TMediaCollect& mcolNew)
{
    // Verify it's decent
    TMediaCollect* pmcolTar = static_cast<TMediaCollect*>
    (
        pmddbCheckCanUpdate(mcolNew, tCQCMedia::EDataTypes::Collect)
    );

    // Looks ok, so update it
    *pmcolTar = mcolNew;
}

tCIDLib::TVoid TMediaDB::UpdateImage(const TMediaImg& mimgNew)
{
    // Verify it's decent
    TMediaImg* pmimgTar = static_cast<TMediaImg*>
    (
        pmddbCheckCanUpdate(mimgNew, tCQCMedia::EDataTypes::Image)
    );

    // Looks ok, so update it
    *pmimgTar = mimgNew;
}

tCIDLib::TVoid TMediaDB::UpdateItem(const TMediaItem &mitemNew)
{
    // Verify it's decent
    TMediaItem* pmitemTar = static_cast<TMediaItem*>
    (
        pmddbCheckCanUpdate(mitemNew, tCQCMedia::EDataTypes::Item)
    );

    // Looks ok, so update it
    *pmitemTar = mitemNew;
}

tCIDLib::TVoid TMediaDB::UpdateTitle(const TMediaTitleSet& mtsNew)
{
    // Verify it's decent
    TMediaTitleSet* pmtsTar = static_cast<TMediaTitleSet*>
    (
        pmddbCheckCanUpdate(mtsNew, tCQCMedia::EDataTypes::TitleSet)
    );

    // Looks ok, so update it
    *pmtsTar = mtsNew;
}


//
//  This is just for the CQSL repo driver to call when upgrading a repo from the
//  pre-4.4.902 format. We do the following:
//
//  1. We used to use the upper cased name of categories as the unique id. This meant
//     that it was awkward to ever change the name of a category. It has to be removed
//     re-added, which could break references. So we will run through them all and
//     generate real unique ids.
//
tCIDLib::TVoid TMediaDB::CQSLUpdate902()
{
    tCQCMedia::EMediaTypes eType = tCQCMedia::EMediaTypes::Min;
    for (; eType <= tCQCMedia::EMediaTypes::Max; eType++)
    {
        tCQCMedia::TNCCatList& colCur = *m_colCats[eType];
        const tCIDLib::TCard4 c4Count = colCur.c4ElemCount();

        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            colCur[c4Index]->strUniqueId(TUniqueId::strMakeId());
        eType++;
    }

    //
    //  Create a 64K array of flags. We will use this to figure out if any
    //  collection is referenced by more than one title set. Any time we see one
    //  referenced and its bit is already on, we remove it from the newly found
    //  title set, insuring that it is only referenced by a single title.
    //
    TFundArray<tCIDLib::TCard1> fcolRefs(0x10000UL);

    for (eType = tCQCMedia::EMediaTypes::Min; eType <= tCQCMedia::EMediaTypes::Max; eType++)
    {
        // Reset them all on each round
        fcolRefs.SetAll(0);

        tCQCMedia::TNCSetIdList& colCur = *m_colSetsById[eType];
        const tCIDLib::TCard4 c4Count = colCur.c4ElemCount();

        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            TMediaTitleSet* pmtsCur = colCur[c4Index];

            // Go through all of this guy's collections
            const tCIDLib::TCard4 c4ColCnt = pmtsCur->c4ColCount();
            for (tCIDLib::TCard4 c4CInd = 0; c4CInd < c4ColCnt; c4CInd++)
            {
                const tCIDLib::TCard2 c2ColId = pmtsCur->c2ColIdAt(c4CInd);

                // If already referenced, remove it, else set it as referenced
                if (fcolRefs[c2ColId])
                    pmtsCur->bRemoveColById(c2ColId, kCIDLib::False);
                else
                    fcolRefs[c2ColId] = 1;
            }
        }
        eType++;
    }
}


// ---------------------------------------------------------------------------
//  TMediaDB: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TMediaDB::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Check for our start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCMedia_Database::c2FmtVersion))
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

    // Flush our lists out to prepare for new stuff
    Reset();

    // Stream in the music stuff
    StreamInMType(tCQCMedia::EMediaTypes::Music, strmToReadFrom);

    // Do the movie stuff
    StreamInMType(tCQCMedia::EMediaTypes::Movie, strmToReadFrom);

    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);

    // Finalize the data
    LoadComplete();
}


tCIDLib::TVoid TMediaDB::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Stream our start marker and format version
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCMedia_Database::c2FmtVersion;

    // Do the music stuff
    FormatBin(tCQCMedia::EMediaTypes::Music, strmToWriteTo);

    // Do the movie stuff
    FormatBin(tCQCMedia::EMediaTypes::Movie, strmToWriteTo);

    // We don't have any stuff of our own right now, so jut do the marker
    strmToWriteTo   << tCIDLib::EStreamMarkers::EndObject;
}



// ---------------------------------------------------------------------------
//  TMediaDB: Protected, non-virtual methods
// ---------------------------------------------------------------------------

//
//  These are non-const versions of all of the public methods that provide access
//  to our sets, collections, etc... These are for internal use and by a couple of
//  friend classes we trust to make direct changes. Everyone has to get copies out
//  modify them, then use the UpdateXXX() methods to put the changes back.
//
//  These mostly just call the const versions and cast off the const-ness, but some
//  can't do that.
//

tCIDLib::TVoid
TMediaDB::FindContainersNC( const   tCQCMedia::EMediaTypes      eMType
                            , const tCQCMedia::EDataTypes       eDataType
                            , const tCIDLib::TCard2             c2Id
                            ,       tCQCMedia::TNCColIdList&    colCols
                            ,       tCQCMedia::TNCSetIdList&    colSets)
{
    #if CID_DEBUG_ON
    if ((colCols.eAdopt() != tCIDLib::EAdoptOpts::NoAdopt)
    ||  (colSets.eAdopt() != tCIDLib::EAdoptOpts::NoAdopt))
    {
        facCIDLib().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCIDErrs::errcCol_CantAdopt
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Internal
        );
    }
    #endif

    if (eDataType == tCQCMedia::EDataTypes::Item)
    {
        // Make sure it exists. If not let it throw
        TMediaItem* pmitemTar = pmitemByIdNC(eMType, c2Id, kCIDLib::True);

        tCQCMedia::TNCColIdList& colCols = *m_colColsById[eMType];
        const tCIDLib::TCard4 c4Count = colCols.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            // If this one doesn't reference the indicated track, do nothing
            TMediaCollect& mcolCur = *colCols[c4Index];

            tCIDLib::TCard2 c2ItemInd;
            if (!mcolCur.bContainsItem(c2Id, c2ItemInd))
                continue;

            colCols.Add(&mcolCur);
        }
    }
     else if (eDataType == tCQCMedia::EDataTypes::Collect)
    {
        // Just add the one collection to the collection list
        colCols.Add(pmcolByIdNC(eMType, c2Id, kCIDLib::True));
    }
     else if (eDataType == tCQCMedia::EDataTypes::TitleSet)
    {
        // Just add the one title set to the title set list
        colSets.Add(pmtsByIdNC(eMType, c2Id, kCIDLib::True));
    }

    //
    //  If we got any collections, then find the sets that contain them.
    //  In this case, it's a one to one relationship, so we can just stop
    //  once we find each containing set.
    //
    const tCIDLib::TCard4 c4ColCnt = colCols.c4ElemCount();
    if (c4ColCnt)
    {
        tCQCMedia::TNCSetIdList& colSets = *m_colSetsById[eMType];
        const tCIDLib::TCard4 c4SetCnt = colSets.c4ElemCount();

        for (tCIDLib::TCard4 c4ColInd = 0; c4ColInd < c4ColCnt; c4ColInd++)
        {
            TMediaCollect* pmcolCur = colCols[c4ColInd];
            for (tCIDLib::TCard4 c4SetInd = 0; c4SetInd < c4SetCnt; c4SetInd++)
            {
                TMediaTitleSet* pmtsCur = colSets[c4SetInd];

                tCIDLib::TCard2 c2Index;
                if (pmtsCur->bContainsCol(pmcolCur->c2Id(), c2Index))
                    colSets.Add(pmtsCur);
            }
        }
    }
}


TMediaCat&
TMediaDB::mcatAtNC( const   tCQCMedia::EMediaTypes  eType
                    , const tCIDLib::TCard4         c4At)
{
    #if CID_DEBUG_ON
    if (eType >= tCQCMedia::EMediaTypes::Count)
        ThrowBadType(CID_LINE, eType);
    #endif
    return *m_colCats[eType]->pobjAt(c4At);
}


TMediaCollect&
TMediaDB::mcolAtNC( const   tCQCMedia::EMediaTypes  eType
                    , const tCIDLib::TCard4         c4At)
{
    #if CID_DEBUG_ON
    if (eType >= tCQCMedia::EMediaTypes::Count)
        ThrowBadType(CID_LINE, eType);
    #endif
    return *m_colColsById[eType]->pobjAt(c4At);
}

TMediaImg&
TMediaDB::mimgAtNC( const   tCQCMedia::EMediaTypes  eType
                    , const tCIDLib::TCard4         c4At)
{
    #if CID_DEBUG_ON
    if (eType >= tCQCMedia::EMediaTypes::Count)
        ThrowBadType(CID_LINE, eType);
    #endif
    return *m_colImgsById[eType]->pobjAt(c4At);
}

TMediaItem&
TMediaDB::mitemAtNC(const   tCQCMedia::EMediaTypes  eType
                    , const tCIDLib::TCard4         c4At)
{
    #if CID_DEBUG_ON
    if (eType >= tCQCMedia::EMediaTypes::Count)
        ThrowBadType(CID_LINE, eType);
    #endif
    return *m_colItemsById[eType]->pobjAt(c4At);
}

TMediaTitleSet&
TMediaDB::mtsAtNC(  const   tCQCMedia::EMediaTypes  eType
                    , const tCIDLib::TCard4         c4At)
{
    #if CID_DEBUG_ON
    if (eType >= tCQCMedia::EMediaTypes::Count)
        ThrowBadType(CID_LINE, eType);
    #endif
    return *m_colSetsById[eType]->pobjAt(c4At);
}


TMediaCat*
TMediaDB::pmcatByUniqueIdNC(const   tCQCMedia::EMediaTypes  eType
                            , const TString&                strToFind
                            , const tCIDLib::TBoolean       bThrowIfNot)
{
    // Avoid duplication, call the const version
    return const_cast<TMediaCat*>
    (
        static_cast<const TMediaDB&>(*this).pmcatByUniqueId(eType, strToFind, bThrowIfNot)
    );
}


TMediaCollect*
TMediaDB::pmcolByIdNC(  const   tCQCMedia::EMediaTypes  eType
                        , const tCIDLib::TCard2         c2IdToFind
                        , const tCIDLib::TBoolean       bThrowIfNot)
{
    // Avoid duplication, call the const version
    return const_cast<TMediaCollect*>
    (
        static_cast<const TMediaDB&>(*this).pmcolById(eType, c2IdToFind, bThrowIfNot)
    );
}


TMediaCollect*
TMediaDB::pmcolByUniqueIdNC(const   tCQCMedia::EMediaTypes  eType
                            , const TString&                strToFind
                            , const tCIDLib::TBoolean       bThrowIfNot)
{
    // Avoid duplication, call the const version
    return const_cast<TMediaCollect*>
    (
        static_cast<const TMediaDB&>(*this).pmcolByUniqueId(eType, strToFind, bThrowIfNot)
    );
}


TMediaImg*
TMediaDB::pmimgByIdNC(  const   tCQCMedia::EMediaTypes  eType
                        , const tCIDLib::TCard2         c2IdToFind
                        , const tCIDLib::TBoolean       bThrowIfNot)
{
    // Avoid duplication, call the const version
    return const_cast<TMediaImg*>
    (
        static_cast<const TMediaDB&>(*this).pmimgById(eType, c2IdToFind, bThrowIfNot)
    );
}


TMediaImg*
TMediaDB::pmimgByUniqueIdNC(const   tCQCMedia::EMediaTypes  eType
                            , const TString&                strToFind
                            , const tCIDLib::TBoolean       bThrowIfNot)
{
    // Avoid duplication, call the const version
    return const_cast<TMediaImg*>
    (
        static_cast<const TMediaDB&>(*this).pmimgByUniqueId(eType, strToFind, bThrowIfNot)
    );
}


TMediaItem*
TMediaDB::pmitemByIdNC( const   tCQCMedia::EMediaTypes  eType
                        , const tCIDLib::TCard2         c2IdToFind
                        , const tCIDLib::TBoolean       bThrowIfNot)
{
    // Avoid duplication, call the const version
    return const_cast<TMediaItem*>
    (
        static_cast<const TMediaDB&>(*this).pmitemById(eType, c2IdToFind, bThrowIfNot)
    );
}


TMediaItem*
TMediaDB::pmitemByUniqueIdNC(const  tCQCMedia::EMediaTypes  eType
                            , const TString&                strToFind
                            , const tCIDLib::TBoolean       bThrowIfNot)
{
    // Avoid duplication, call the const version
    return const_cast<TMediaItem*>
    (
        static_cast<const TMediaDB&>(*this).pmitemByUniqueId(eType, strToFind, bThrowIfNot)
    );
}


//
//  Find teh title set that contains the indicated collection. There should only ever
//  be one. There's no smart way to do this, we have to just search the titles.
//
TMediaTitleSet*
TMediaDB::pmtsFindColParentNC(  const   TMediaCollect&      mcolTar
                                ,       tCIDLib::TCard2&    c2Index
                                , const tCIDLib::TBoolean   bThrowIfNot)
{
    // Get the list of sets for the passed collection's media type
    tCQCMedia::TNCSetIdList& colSets = *m_colSetsById[mcolTar.eType()];
    const tCIDLib::TCard4 c4SetCnt = colSets.c4ElemCount();

    // And search it for one that references the collection's id
    TMediaTitleSet* pmtsRet = 0;
    for (tCIDLib::TCard4 c4SetInd = 0; c4SetInd < c4SetCnt; c4SetInd++)
    {
        TMediaTitleSet* pmtsCur = colSets[c4SetInd];
        if (pmtsCur->bContainsCol(mcolTar.c2Id(), c2Index))
        {
            pmtsRet = pmtsCur;
            break;
        }
    }
    return pmtsRet;
}


TMediaTitleSet*
TMediaDB::pmtsByIdNC(const  tCQCMedia::EMediaTypes  eType
                    , const tCIDLib::TCard2         c2IdToFind
                    , const tCIDLib::TBoolean       bThrowIfNot)
{
    // Avoid duplication, call the const version
    return const_cast<TMediaTitleSet*>
    (
        static_cast<const TMediaDB&>(*this).pmtsById(eType, c2IdToFind, bThrowIfNot)
    );
}


TMediaTitleSet*
TMediaDB::pmtsByUniqueIdNC( const   tCQCMedia::EMediaTypes  eType
                            , const TString&                strToFind
                            , const tCIDLib::TBoolean       bThrowIfNot)
{
    // Avoid duplication, call the const version
    return const_cast<TMediaTitleSet*>
    (
        static_cast<const TMediaDB&>(*this).pmtsByUniqueId(eType, strToFind, bThrowIfNot)
    );
}


// ---------------------------------------------------------------------------
//  TMediaDB: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  In the case where we reload existing databases, we have to accept the already
//  set ids. This is called to check them.
//
tCIDLib::TCard2
TMediaDB::c2CheckTakeId(const   TMediaDBBase&           mddbTest
                        , const tCQCMedia::EDataTypes   eDType
                        , const tCQCMedia::EMediaTypes  eMType) const
{
    const tCIDLib::TCard2 c2Ret = mddbTest.c2Id();

    if (!mddbTest.c2Id())
    {
        facCQCMedia().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMedErrs::errcDB_ZeroId
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::BadParms
            , tCQCMedia::strXlatEDataTypes(eDType)
            , tCQCMedia::strXlatEMediaTypes(eMType)
        );
    }

    tCIDLib::TBoolean   bFound = kCIDLib::False;
    switch(eDType)
    {
        case tCQCMedia::EDataTypes::Cat :
            bFound = pmcatById(eMType, c2Ret, kCIDLib::False) != 0;
            break;

        case tCQCMedia::EDataTypes::Collect :
            bFound = pmcolById(eMType, c2Ret, kCIDLib::False) != 0;
            break;

        case tCQCMedia::EDataTypes::Image :
            bFound = pmimgById(eMType, c2Ret, kCIDLib::False) != 0;
            break;

        case tCQCMedia::EDataTypes::Item :
            bFound = pmitemById(eMType, c2Ret, kCIDLib::False) != 0;
            break;

        case tCQCMedia::EDataTypes::TitleSet :
            bFound = pmtsById(eMType, c2Ret, kCIDLib::False) != 0;
            break;

        default :
            CIDAssert2(L"Unknown data type");
            break;
    };

    // If we found it, it's a dup
    if (bFound)
    {
        facCQCMedia().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMedErrs::errcDB_DupMediaId
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Already
            , TCardinal(c2Ret)
            , tCQCMedia::strXlatEDataTypes(eDType)
        );
    }
    return c2Ret;
}


//
//  Return the next available id for a particular media type and data type. We look
//  at the last entry in the by id list, which is sorted by id so whatever the last
//  one is is the next one available.
//
//  Note that, in the case of categories, we have some special ones with fixed ids,
//  and the rest start after that. If we get called here, it's not a special one, so
//  we make sure that we return at least the minimum user id. Once one of them is
//  assigned, after that we'll always come up with a higher number.
//
//  <TBD>
//  If we max out, then we go back and try to find an empty one to use, since some
//  may have been deleted.
//
tCIDLib::TCard2
TMediaDB::c2NextId( const   tCQCMedia::EMediaTypes  eMType
                    , const tCQCMedia::EDataTypes   eDType) const
{
    tCIDLib::TCard2 c2Ret = 1;
    switch(eDType)
    {
        case tCQCMedia::EDataTypes::Cat :
        {
            //
            //  We don't have an id sorted list for these, so we have to just
            //  run through them and keep the highest number.
            //
            const tCQCMedia::TNCCatList& colCats = *m_colCats[eMType];
            const tCIDLib::TCard4 c4Count = colCats.c4ElemCount();
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            {
                const tCIDLib::TCard2 c2Cur = colCats[c4Index]->c2Id();
                if (c2Cur > c2Ret)
                    c2Ret = c2Cur;
            }

            // If it's less than the start of user categories, move to that
            if (c2Ret < kCQCMedia::c2CatId_UserStart)
                c2Ret = kCQCMedia::c2CatId_UserStart;
            break;
        }

        case tCQCMedia::EDataTypes::Collect :
        {
            const tCQCMedia::TNCColIdList& colCols = *m_colColsById[eMType];
            if (!colCols.bIsEmpty())
                c2Ret = colCols.pobjLast()->c2Id();
            break;
        }

        case tCQCMedia::EDataTypes::Image :
        {
            const tCQCMedia::TNCImgIdList& colArt = *m_colImgsById[eMType];
            if (!colArt.bIsEmpty())
                c2Ret = colArt.pobjLast()->c2Id();
            break;
        }

        case tCQCMedia::EDataTypes::Item :
        {
            const tCQCMedia::TNCItemIdList& colItems = *m_colItemsById[eMType];
            if (!colItems.bIsEmpty())
                c2Ret = colItems.pobjLast()->c2Id();
            break;
        }

        case tCQCMedia::EDataTypes::TitleSet :
        {
            const tCQCMedia::TNCSetIdList& colSets = *m_colSetsById[eMType];
            if (!colSets.bIsEmpty())
                c2Ret = colSets.pobjLast()->c2Id();
            break;
        }

        default :
            #if CID_DEBUG_ON
            facCQCMedia().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kMedErrs::errcDB_UnknownDataType
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Unknown
                , TInteger(tCIDLib::i4EnumOrd(eDType))
            );
            #endif
            break;
    };

    // Not likely but check for maxing out
    if (c2Ret == kCIDLib::c2MaxCard)
    {
        facCQCMedia().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMedErrs::errcDB_NorMoreIds
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::OutResource
            , tCQCMedia::strXlatEMediaTypes(eMType)
            , tCQCMedia::strXlatEDataTypes(eDType)
        );
    }

    // And return the next value up
    return c2Ret + 1;
}


//
//  Called from the UpdateXXX() methods to see if the passed object is valid to
//  update. It has to be something already in the database before we can do it,
//  which also means it'll have all the required stuff. The caller passed the media
//  type he knows it is supposed to be, so we can verify that.
//
TMediaDBBase*
TMediaDB::pmddbCheckCanUpdate(  const   TMediaDBBase&           mddbTest
                                , const tCQCMedia::EDataTypes   eDType)
{
    // The unique id must not be empty
    if (mddbTest.strUniqueId().bIsEmpty())
    {
        facCQCMedia().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMedErrs::errcDB_EmptyUID
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Unknown
            , tCQCMedia::strXlatEDataTypes(eDType)
        );
    }

    // The id must exist
    const tCQCMedia::EMediaTypes eMType = mddbTest.eType();
    TMediaDBBase* pmddbId = 0;
    TMediaDBBase* pmddbUID = 0;
    switch(eDType)
    {
        case tCQCMedia::EDataTypes::Cat :
        {
            pmddbId = pmcatByIdNC(eMType, mddbTest.c2Id(), kCIDLib::False);
            pmddbUID = pmcatByUniqueIdNC(eMType, mddbTest.strUniqueId(), kCIDLib::False);
            break;
        }

        case tCQCMedia::EDataTypes::Collect :
        {
            pmddbId = pmcolByIdNC(eMType, mddbTest.c2Id(), kCIDLib::False);
            pmddbUID = pmcolByUniqueIdNC(eMType, mddbTest.strUniqueId(), kCIDLib::False);
            break;
        }

        case tCQCMedia::EDataTypes::Image :
        {
            pmddbId = pmimgByIdNC(eMType, mddbTest.c2Id(), kCIDLib::False);
            pmddbUID = pmimgByUniqueIdNC(eMType, mddbTest.strUniqueId(), kCIDLib::False);
            break;
        }

        case tCQCMedia::EDataTypes::Item :
        {
            pmddbId = pmitemByIdNC(eMType, mddbTest.c2Id(), kCIDLib::False);
            pmddbUID = pmitemByUniqueIdNC(eMType, mddbTest.strUniqueId(), kCIDLib::False);
            break;
        }

        case tCQCMedia::EDataTypes::TitleSet :
        {
            pmddbId = pmtsByIdNC(eMType, mddbTest.c2Id(), kCIDLib::False);
            pmddbUID = pmtsByUniqueIdNC(eMType, mddbTest.strUniqueId(), kCIDLib::False);
            break;
        }

        default :
            #if CID_DEBUG_ON
            facCQCMedia().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kMedErrs::errcDB_UnknownDataType
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Unknown
                , TInteger(tCIDLib::i4EnumOrd(eDType))
            );
            #endif
            break;
    };

    //
    //  If either of them wasn't found, that's bad. We also verify that we found
    //  the same object via both ids.
    //
    if (!pmddbId || !pmddbUID || (pmddbId->strUniqueId() != pmddbUID->strUniqueId()))
    {
        facCQCMedia().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMedErrs::errcDB_UpdateError
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Unknown
            , tCQCMedia::strXlatEDataTypes(eDType)
        );
    }

    return pmddbId;
}


//
//  This will check whether the media database is complete. If not, it will
//  throw an exception.
//
tCIDLib::TVoid TMediaDB::CheckComplete()
{
    if (!m_bComplete)
    {
        facCQCMedia().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMedErrs::errcDB_NotComplete
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotReady
        );
    }
}


// Verify a data type and throw if not the same
tCIDLib::TVoid
TMediaDB::CheckDataType(const   tCQCMedia::EDataTypes   eExpected
                        , const tCQCMedia::EDataTypes   eCheck) const
{
    if (eExpected != eCheck)
    {
        facCQCMedia().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMedErrs::errcDB_WrongDType
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::TypeMatch
            , tCQCMedia::strXlatEDataTypes(eExpected)
            , tCQCMedia::strXlatEDataTypes(eCheck)
        );
    }
}


// Verify a media type and throw if not the same
tCIDLib::TVoid
TMediaDB::CheckMediaType(const  tCQCMedia::EMediaTypes  eExpected
                        , const tCQCMedia::EMediaTypes  eCheck) const
{
    if (eExpected != eCheck)
    {
        facCQCMedia().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMedErrs::errcDB_WrongMType
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::TypeMatch
            , tCQCMedia::strXlatEMediaTypes(eExpected)
            , tCQCMedia::strXlatEMediaTypes(eCheck)
        );
    }
}


// To avoid duplicated code in multiple ctors
tCIDLib::TVoid TMediaDB::DoCommonInit()
{
    //
    //  And now create all the collections except any that are going to be faulted in.
    //  For those we use the null pointer as a means to know if we need to fault it in
    //  or not.
    //
    tCQCMedia::EMediaTypes eType = tCQCMedia::EMediaTypes::Min;
    for (; eType < tCQCMedia::EMediaTypes::Count; eType++)
    {
        m_colByArtist.Add
        (
            new tCQCMedia::TByArtistList(tCIDLib::EAdoptOpts::Adopt, 128)
        );
        m_colCats.Add
        (
            new tCQCMedia::TNCCatList(tCIDLib::EAdoptOpts::Adopt, 32)
        );
        m_colColsById.Add
        (
            new tCQCMedia::TNCColIdList(tCIDLib::EAdoptOpts::Adopt, 512)
        );

        m_colColsByUID.Add
        (
            new tCQCMedia::TColNameList
            (
                tCIDLib::EAdoptOpts::NoAdopt
                , 107
                , TStringKeyOps()
                , &TMediaCollect::strUIDAccess
            )
        );
        m_colImgsById.Add
        (
            new tCQCMedia::TNCImgIdList(tCIDLib::EAdoptOpts::Adopt, 8192)
        );
        m_colImgsByUID.Add
        (
            new tCQCMedia::TImgNameList
            (
                tCIDLib::EAdoptOpts::NoAdopt
                , 107
                , TStringKeyOps()
                , &TMediaImg::strKeyUID
            )
        );
        m_colItemsById.Add
        (
            new tCQCMedia::TNCItemIdList(tCIDLib::EAdoptOpts::Adopt, 8192)
        );
        m_colItemsByUID.Add
        (
            new tCQCMedia::TItemNameList
            (
                tCIDLib::EAdoptOpts::NoAdopt
                , 107
                , TStringKeyOps()
                , &TMediaItem::strUIDAccess
            )
        );
        m_colSetsById.Add
        (
            new tCQCMedia::TNCSetIdList(tCIDLib::EAdoptOpts::Adopt, 64)
        );
        m_colSetsByUID.Add
        (
            new tCQCMedia::TSetNameList
            (
                tCIDLib::EAdoptOpts::NoAdopt
                , 107
                , TStringKeyOps()
                , &TMediaTitleSet::strUIDAccess
            )
        );
    }

    // Set up the category cookie strings for the 'All xxx' categories
    facCQCMedia().FormatCatCookie
    (
        tCQCMedia::EMediaTypes::Movie
        , kCQCMedia::c2CatId_AllMovies
        , m_objaAllCatCookies[tCQCMedia::EMediaTypes::Movie]
    );

    facCQCMedia().FormatCatCookie
    (
        tCQCMedia::EMediaTypes::Music
        , kCQCMedia::c2CatId_AllMusic
        , m_objaAllCatCookies[tCQCMedia::EMediaTypes::Music]
    );

    facCQCMedia().FormatCatCookie
    (
        tCQCMedia::EMediaTypes::Pic
        , kCQCMedia::c2CatId_AllPictures
        , m_objaAllCatCookies[tCQCMedia::EMediaTypes::Pic]
    );

    // Create the unknown collections and items
    m_objaUnknownCols[tCQCMedia::EMediaTypes::Movie] = TMediaCollect
    (
      L"[Unknown Movie Collection]"
      , L"[Unknown Movie]"
      , 0
      , tCQCMedia::ELocTypes::Changer
      , tCQCMedia::EMediaTypes::Movie
    );
    m_objaUnknownCols[tCQCMedia::EMediaTypes::Movie].c2Id(kCIDLib::c2MaxCard);

    m_objaUnknownCols[tCQCMedia::EMediaTypes::Music] = TMediaCollect
    (
        L"[Unknown Music Collection]"
        , L"[Unknown Music]"
        , 0
        , tCQCMedia::ELocTypes::Changer
        , tCQCMedia::EMediaTypes::Music
    );
    m_objaUnknownCols[tCQCMedia::EMediaTypes::Music].c2Id(kCIDLib::c2MaxCard);

    m_objaUnknownCols[tCQCMedia::EMediaTypes::Pic] = TMediaCollect
    (
        L"[Unknown Picture Collection]"
        , L"[Unknown Picture Set]"
        , 0
        , tCQCMedia::ELocTypes::Changer
        , tCQCMedia::EMediaTypes::Pic
    );
    m_objaUnknownCols[tCQCMedia::EMediaTypes::Pic].c2Id(kCIDLib::c2MaxCard);

    m_objaUnknownItems[tCQCMedia::EMediaTypes::Movie] = TMediaItem
    (
        L"[Unknown Chapter]"
        , L"[Unknown Chapter]"
        , TString::strEmpty()
        , tCQCMedia::EMediaTypes::Movie
    );
    m_objaUnknownItems[tCQCMedia::EMediaTypes::Movie].c2Id(kCIDLib::c2MaxCard);

    m_objaUnknownItems[tCQCMedia::EMediaTypes::Music] = TMediaItem
    (
        L"[Unknown Track]"
        , L"[Unknown Track]"
        , TString::strEmpty()
        , tCQCMedia::EMediaTypes::Music
    );
    m_objaUnknownItems[tCQCMedia::EMediaTypes::Music].c2Id(kCIDLib::c2MaxCard);

    m_objaUnknownItems[tCQCMedia::EMediaTypes::Pic] = TMediaItem
    (
        L"[Unknown Picture]"
        , L"[Unknown Picture]"
        , TString::strEmpty()
        , tCQCMedia::EMediaTypes::Pic
    );
    m_objaUnknownItems[tCQCMedia::EMediaTypes::Pic].c2Id(kCIDLib::c2MaxCard);

    // Do a reset to set up some defaults
    Reset();
}


//
//  This will duplicate the media contents of the passed database. This requires
//  copying all of the lists and whatnot. This is not unlike TakeFrom() above, but
//  doesn't strip the source database, and this one is just for use internally in
//  the copy ctor and assignment operator, so it's private.
//
tCIDLib::TVoid TMediaDB::DupDB(const TMediaDB& mdbSrc)
{
    // Make sure we are cleaned out, also resets any transient views
    Reset();

    // Copy over all the data for each media type
    tCQCMedia::EMediaTypes eType = tCQCMedia::EMediaTypes::Min;
    for (; eType < tCQCMedia::EMediaTypes::Count; eType++)
    {
        //
        //  For each source object, duplicate it into our adopting list, then add it
        //  to the non-adopting by UID list if we have one for that type.
        //
        {
            TMediaCat* pmcatNew = nullptr;
            const tCIDLib::TCard4 c4CCount = mdbSrc.m_colCats[eType]->c4ElemCount();
            for (tCIDLib::TCard4 c4CInd = 0; c4CInd < c4CCount; c4CInd++)
            {
                pmcatNew = new TMediaCat(*mdbSrc.m_colCats[eType]->pobjAt(c4CInd));
                m_colCats[eType]->Add(pmcatNew);
            }
        }

        {
            TMediaImg* pmimgNew = nullptr;
            const tCIDLib::TCard4 c4ICount = mdbSrc.m_colImgsById[eType]->c4ElemCount();
            for (tCIDLib::TCard4 c4IInd = 0; c4IInd < c4ICount; c4IInd++)
            {
                pmimgNew = new TMediaImg(*mdbSrc.m_colImgsById[eType]->pobjAt(c4IInd));
                m_colImgsById[eType]->Add(pmimgNew);
                m_colImgsByUID[eType]->Add(pmimgNew);
            }
        }

        {
            TMediaItem* pmitemNew = nullptr;
            const tCIDLib::TCard4 c4ICount = mdbSrc.m_colItemsById[eType]->c4ElemCount();
            for (tCIDLib::TCard4 c4IInd = 0; c4IInd < c4ICount; c4IInd++)
            {
                pmitemNew = new TMediaItem(*mdbSrc.m_colItemsById[eType]->pobjAt(c4IInd));
                m_colItemsById[eType]->Add(pmitemNew);
                m_colItemsByUID[eType]->Add(pmitemNew);
            }
        }


        {
            TMediaCollect* pmcolNew = nullptr;
            const tCIDLib::TCard4 c4CCount = mdbSrc.m_colColsById[eType]->c4ElemCount();
            for (tCIDLib::TCard4 c4CInd = 0; c4CInd < c4CCount; c4CInd++)
            {
                pmcolNew = new TMediaCollect(*mdbSrc.m_colColsById[eType]->pobjAt(c4CInd));
                m_colColsById[eType]->Add(pmcolNew);
                m_colColsByUID[eType]->Add(pmcolNew);
            }
        }

        {
            TMediaTitleSet* pmtsNew = nullptr;
            const tCIDLib::TCard4 c4SCount = mdbSrc.m_colSetsById[eType]->c4ElemCount();
            for (tCIDLib::TCard4 c4SInd = 0; c4SInd < c4SCount; c4SInd++)
            {
                pmtsNew = new TMediaTitleSet(*mdbSrc.m_colSetsById[eType]->pobjAt(c4SInd));
                m_colSetsById[eType]->Add(pmtsNew);
                m_colSetsByUID[eType]->Add(pmtsNew);
            }
        }
    }
}


//
//  See if we have the passed artist in our artist map. This is for internal use and returns
//  a non-const pointer so that it can be modified if needed.
//
const TArtistMap*
TMediaDB::pcolArtistToMap(const tCQCMedia::EMediaTypes eMType, const TString& strArtist) const
{
    // Get the artist map for this media type
    const tCQCMedia::TByArtistList& colMap = *m_colByArtist[eMType];

    // If empty, obviously not
    if (colMap.bIsEmpty())
        return nullptr;

    // The list is sorted, so we can do a binary lookup
    tCIDLib::TCard4 c4At = kCIDLib::c4MaxCard;
    return colMap.pobjKeyedBinarySearch(strArtist, TArtistMap::eCompByKey, c4At);
}

TArtistMap*
TMediaDB::pcolArtistToMap(const tCQCMedia::EMediaTypes eMType, const TString& strArtist)
{
    // Get the artist map for this media type
    tCQCMedia::TByArtistList& colMap = *m_colByArtist[eMType];

    // If empty, obviously not
    if (colMap.bIsEmpty())
        return nullptr;

    // The list is sorted, so we can do a binary lookup
    tCIDLib::TCard4 c4At = kCIDLib::c4MaxCard;
    return colMap.pobjKeyedBinarySearch(strArtist, TArtistMap::eCompByKey, c4At);
}



// Get a non-constant pinter to a category by id
TMediaCat*
TMediaDB::pmcatByIdNC(  const   tCQCMedia::EMediaTypes  eType
                        , const tCIDLib::TCard2         c2IdToFind
                        , const tCIDLib::TBoolean       bThrowIfNot)
{
    // Avoid duplication, call the const version
    return const_cast<TMediaCat*>
    (
        static_cast<const TMediaDB&>(*this).pmcatById(eType, c2IdToFind, bThrowIfNot)
    );
}


//
//  Find any collections that reference the passed track and remove it from them.
//  This doesn't require any re-sorting since we are just removing things.
//
//  The affected collections are refinalized. The item is not deleted itself, unless
//  the caller does so.
//
tCIDLib::TVoid TMediaDB::RemoveFromCols(const TMediaItem& mitemRem)
{
    const tCQCMedia::EMediaTypes eType = mitemRem.eType();

    //
    //  Iterate the collections and find any that have this guy as a member. In
    //  this case, it could be more than one collection since its original
    //  collection plus any playlists could reference it.
    //
    //  For each one we find, we remove the item.
    //
    const tCIDLib::TCard4 c4CCnt = m_colColsById[eType]->c4ElemCount();
    for (tCIDLib::TCard4 c4CIndex = 0; c4CIndex < c4CCnt; c4CIndex++)
    {
        TMediaCollect* pmcolCur = m_colColsById[eType]->pobjAt(c4CIndex);

        tCIDLib::TCard2 c2ItemInd = kCIDLib::c2MaxCard;
        if (pmcolCur->bContainsItem(mitemRem.c2Id(), c2ItemInd))
        {
            pmcolCur->RemoveItem(mitemRem.c2Id());
            pmcolCur->Finalize(*this);
        }
    }

    // Reset any transient views just in case
    ResetTransientViews();
}


//
//  Find the title set that includes the passed collection and removes the col. Note
//  that this won't require any re-sorting since we just removed stuff.
//
//  The affected sets are refinalized. The collection itself is not removed unless
//  the caller does so.
//
tCIDLib::TVoid TMediaDB::RemoveFromSets(const TMediaCollect& mcolRem)
{
    const tCQCMedia::EMediaTypes eType = mcolRem.eType();

    // Iterate the titles and find the one that contains this guy
    const tCIDLib::TCard4 c4TCnt = m_colSetsById[eType]->c4ElemCount();
    for (tCIDLib::TCard4 c4TIndex = 0; c4TIndex < c4TCnt; c4TIndex++)
    {
        TMediaTitleSet* pmtsCur = m_colSetsById[eType]->pobjAt(c4TIndex);

        tCIDLib::TCard2 c2Index = kCIDLib::c2MaxCard;
        if (pmtsCur->bContainsCol(mcolRem.c2Id(), c2Index))
        {
            pmtsCur->RemoveColAt(c2Index);
            pmtsCur->Finalize(*this);
        }
    }
}


//
//  This will clean up any transient views of the data, which needs to be done after changes
//  are made to the database.
//
tCIDLib::TVoid TMediaDB::ResetTransientViews()
{
    // Clear the by artist lists
    tCQCMedia::EMediaTypes eMType = tCQCMedia::EMediaTypes::Min;
    while (eMType <= tCQCMedia::EMediaTypes::Max)
    {
        m_colByArtist[eMType]->RemoveAll();
        eMType++;
    }
}



//
//  Once we've accumulated all the titles that match a given search
//  criteria, we need to sort the list by the requested sort order.
//
tCIDLib::TVoid
TMediaDB::SortList(tCQCMedia::TSetIdList& colList, const tCQCMedia::ESortOrders  eOrder)
{
    switch(eOrder)
    {
        case tCQCMedia::ESortOrders::Artist :
        {
            colList.Sort(TMediaTitleSet::eCompByArtist);
            break;
        }

        case tCQCMedia::ESortOrders::BitDepth :
        {
            colList.Sort(TMediaTitleSet::eCompByBitDepth);
            break;
        }

        case tCQCMedia::ESortOrders::BitRate :
        {
            colList.Sort(TMediaTitleSet::eCompByBitDepth);
            break;
        }

        case tCQCMedia::ESortOrders::Channels :
        {
            colList.Sort(TMediaTitleSet::eCompByChannels);
            break;
        }

        case tCQCMedia::ESortOrders::DateAdded :
        {
            colList.Sort(TMediaTitleSet::eCompByDateAdded);
            break;
        }

        case tCQCMedia::ESortOrders::DateAddedDesc :
        {
            colList.Sort
            (
                tCIDLib::TRevCompWrap(TMediaTitleSet::eCompByDateAdded)
            );
            break;
        }

        case tCQCMedia::ESortOrders::SampleRate :
        {
            colList.Sort(TMediaTitleSet::eCompBySampleRate);
            break;
        }

        case tCQCMedia::ESortOrders::SeqNum :
        {
            colList.Sort(TMediaTitleSet::eCompBySeqNum);
            break;
        }

        case tCQCMedia::ESortOrders::SeqNumDesc :
        {
            colList.Sort(tCIDLib::TRevCompWrap(TMediaTitleSet::eCompBySeqNum));
            break;
        }

        case tCQCMedia::ESortOrders::Title :
        {
            // We use the sort title here, not the actual title
            colList.Sort(TMediaTitleSet::eCompBySortTitle);
            break;
        }

        case tCQCMedia::ESortOrders::UserRating :
        {
            colList.Sort(TMediaTitleSet::eCompByUserRating);
            break;
        }

        case tCQCMedia::ESortOrders::UserRatingDesc :
        {
            colList.Sort(tCIDLib::TRevCompWrap(TMediaTitleSet::eCompByUserRating));
            break;
        }

        case tCQCMedia::ESortOrders::Year :
        {
            colList.Sort(TMediaTitleSet::eCompByYear);
            break;
        }

        case tCQCMedia::ESortOrders::YearDesc :
        {
            colList.Sort(tCIDLib::TRevCompWrap(TMediaTitleSet::eCompByYear));
            break;
        }
    };
}


//
//  A helper for the streaming in method, to handle each of the media types,
//  which is all the same except the media type being streamed.
//
tCIDLib::TVoid
TMediaDB::StreamInMType(const   tCQCMedia::EMediaTypes  eMType
                        ,       TBinInStream&           strmSrc)
{
    // SHould start with a frame marker and the same media type as passed
    strmSrc.CheckForFrameMarker(CID_FILE, CID_LINE);
    tCQCMedia::EMediaTypes eTypeTest;
    strmSrc >> eTypeTest;
    CheckMediaType(eMType, eTypeTest);

    // Do the images
    tCIDLib::TCard4         c4Count;
    tCQCMedia::EDataTypes   eDType;
    {
        strmSrc >> eDType >> c4Count;
        CheckDataType(tCQCMedia::EDataTypes::Image, eDType);

        TMediaImg mimgCur;
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            strmSrc >> mimgCur;
            strmSrc.CheckForFrameMarker(CID_FILE, CID_LINE);
            c2AddImage(new TMediaImg(mimgCur), kCIDLib::True);
        }
    }

    // Do the categories
    {
        strmSrc >> eDType >> c4Count;
        CheckDataType(tCQCMedia::EDataTypes::Cat, eDType);

        TMediaCat mcatCur;
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            strmSrc >> mcatCur;
            strmSrc.CheckForFrameMarker(CID_FILE, CID_LINE);
            c2AddCategory(new TMediaCat(mcatCur), kCIDLib::True);
        }
    }

    // Do the items
    {
        strmSrc >> eDType >> c4Count;
        CheckDataType(tCQCMedia::EDataTypes::Item, eDType);

        TMediaItem mitemCur;
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            strmSrc >> mitemCur;
            strmSrc.CheckForFrameMarker(CID_FILE, CID_LINE);
            c2AddItem(new TMediaItem(mitemCur), kCIDLib::True);
        }
    }

    // Do the collections
    {
        strmSrc >> eDType >> c4Count;
        CheckDataType(tCQCMedia::EDataTypes::Collect, eDType);

        TMediaCollect mcolCur;
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            strmSrc >> mcolCur;
            strmSrc.CheckForFrameMarker(CID_FILE, CID_LINE);
            c2AddCollect(new TMediaCollect(mcolCur), kCIDLib::True);
        }
    }

    // Do the title sets
    {
        strmSrc >> eDType >> c4Count;
        CheckDataType(tCQCMedia::EDataTypes::TitleSet, eDType);

        TMediaTitleSet mtsCur;
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            strmSrc >> mtsCur;
            strmSrc.CheckForFrameMarker(CID_FILE, CID_LINE);
            c2AddTitle(new TMediaTitleSet(mtsCur), kCIDLib::True);
        }
    }
}



