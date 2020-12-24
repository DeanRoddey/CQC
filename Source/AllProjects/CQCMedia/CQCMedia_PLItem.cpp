//
// FILE NAME: CQCMedia_PLItem.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 12/20/2006
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
//  This is the implementation file for the playlist item class.
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
RTTIDecls(TCQCMediaPLItem,TObject)
RTTIDecls(TCQCMediaPLMgr,TObject)


// ---------------------------------------------------------------------------
//  Local types and constants
// ---------------------------------------------------------------------------
namespace CQCMedia_PLItem
{
    // -----------------------------------------------------------------------
    //  Max items we'll allow in a playlist
    // -----------------------------------------------------------------------
    constexpr tCIDLib::TCard4   c4MaxPLItems = 8192;


    // -----------------------------------------------------------------------
    //  The max number of items we'll keep in the list for random play from
    //  category mode.
    // -----------------------------------------------------------------------
    #if CID_DEBUG_ON
    constexpr tCIDLib::TCard4   c4MaxRandCatItems = 10;
    #else
    constexpr tCIDLib::TCard4   c4MaxRandCatItems = 40;
    #endif


    // -----------------------------------------------------------------------
    //  Our per item streaming format version
    //
    //  Version 2 -
    //      Added a few more media format values (bit rate, bit depth, and
    //      channels.)
    //
    //  Version 3 -
    //      Added the repository moniker and unique item id (4.3.915)
    //
    //  Version 4 -
    //      Added a user data value for client code's purposes (4.3.917)
    // -----------------------------------------------------------------------
    constexpr tCIDLib::TCard2   c2ItemFmtVersion = 4;


    // -----------------------------------------------------------------------
    //  Our list streaming format version
    // -----------------------------------------------------------------------
    constexpr tCIDLib::TCard2   c2ListFmtVersion = 1;
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCMediaPLItem
//  PREFIX: mpli
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCMediaPLItem: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCMediaPLItem::TCQCMediaPLItem() :

    m_bPlayed(kCIDLib::False)
    , m_c4BitDepth(0)
    , m_c4BitRate(0)
    , m_c4Channels(2)
    , m_c4ColYear(0)
    , m_c4ItemSeconds(0)
    , m_c4ListItemId(0)
    , m_eLocType(tCQCMedia::ELocTypes::Count)
    , m_eMediaType(tCQCMedia::EMediaTypes::Count)
    , m_strFmtTime(L"0m, 0s")
{
}

TCQCMediaPLItem::~TCQCMediaPLItem()
{
}


// ---------------------------------------------------------------------------
//  TCQCMediaPLItem: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Get or set the already played marker
tCIDLib::TBoolean TCQCMediaPLItem::bPlayed() const
{
    return m_bPlayed;
}

tCIDLib::TBoolean
TCQCMediaPLItem::bPlayed(const tCIDLib::TBoolean bToSet)
{
    m_bPlayed = bToSet;
    return m_bPlayed;
}


// Get or set the bit depth
tCIDLib::TCard4 TCQCMediaPLItem::c4BitDepth() const
{
    return m_c4BitDepth;
}

tCIDLib::TCard4 TCQCMediaPLItem::c4BitDepth(const tCIDLib::TCard4 c4ToSet)
{
    m_c4BitDepth = c4ToSet;
    return m_c4BitDepth;
}


// Get or set the bit rate
tCIDLib::TCard4 TCQCMediaPLItem::c4BitRate() const
{
    return m_c4BitRate;
}

tCIDLib::TCard4
TCQCMediaPLItem::c4BitRate(const tCIDLib::TCard4 c4ToSet)
{
    m_c4BitRate= c4ToSet;
    return m_c4BitRate;
}


// Get or set the channel count
tCIDLib::TCard4 TCQCMediaPLItem::c4Channels() const
{
    return m_c4Channels;
}

tCIDLib::TCard4
TCQCMediaPLItem::c4Channels(const tCIDLib::TCard4 c4ToSet)
{
    m_c4Channels = c4ToSet;
    return m_c4Channels;
}


// Get or set the collection year
tCIDLib::TCard4 TCQCMediaPLItem::c4ColYear() const
{
    return m_c4ColYear;
}

tCIDLib::TCard4
TCQCMediaPLItem::c4ColYear(const tCIDLib::TCard4 c4ToSet)
{
    m_c4ColYear = c4ToSet;
    return m_c4ColYear;
}


// Return the unique list item id
tCIDLib::TCard4 TCQCMediaPLItem::c4ListItemId() const
{
    return m_c4ListItemId;
}


//
//  Get or set the item duration in seconds. If we set it, then we also
//  reformat the formatted version.
//
tCIDLib::TCard4 TCQCMediaPLItem::c4ItemSeconds() const
{
    return m_c4ItemSeconds;
}

tCIDLib::TCard4
TCQCMediaPLItem::c4ItemSeconds(const tCIDLib::TCard4 c4ToSet)
{
    m_c4ItemSeconds = c4ToSet;
    facCQCMedia().FormatDuration(m_c4ItemSeconds, m_strFmtTime);
    return m_c4ItemSeconds;
}


// Get or set the collection location type
tCQCMedia::ELocTypes TCQCMediaPLItem::eLocType() const
{
    return m_eLocType;
}

tCQCMedia::ELocTypes
TCQCMediaPLItem::eLocType(const tCQCMedia::ELocTypes eToSet)
{
    m_eLocType = eToSet;
    return m_eLocType;
}


// Get or set the media type
tCQCMedia::EMediaTypes TCQCMediaPLItem::eMediaType() const
{
    return m_eMediaType;
}

tCQCMedia::EMediaTypes
TCQCMediaPLItem::eMediaType(const tCQCMedia::EMediaTypes eToSet)
{
    m_eMediaType = eToSet;
    return m_eMediaType;
}


// Just a convenience method to set it back to defaults
tCIDLib::TVoid TCQCMediaPLItem::Reset()
{
    m_bPlayed = kCIDLib::False;
    m_c4BitDepth = 0;
    m_c4BitRate = 0;
    m_c4Channels = 2;
    m_c4ColYear = 0;
    m_c4ItemSeconds = 0;
    m_c4ListItemId = 0;
    m_eLocType = tCQCMedia::ELocTypes::Count;
    m_eMediaType = tCQCMedia::EMediaTypes::Count;
    m_strColArtist.Clear();
    m_strColAspectRatio.Clear();
    m_strColCast.Clear();
    m_strColCookie.Clear();
    m_strColDescription.Clear();
    m_strColLabel.Clear();
    m_strColLeadActor.Clear();
    m_strColName.Clear();
    m_strColRating.Clear();
    m_strFmtTime = L"0m, 0s";
    m_strItemArtist.Clear();
    m_strItemCookie.Clear();
    m_strItemName.Clear();
    m_strLocInfo.Clear();
    m_strMediaFmt.Clear();
    m_strRepoMoniker.Clear();
    m_strTitleCookie.Clear();
    m_strTitleName.Clear();
    m_strUserData.Clear();
}


// Get or set the collection artist
const TString& TCQCMediaPLItem::strColArtist() const
{
    return m_strColArtist;
}

const TString& TCQCMediaPLItem::strColArtist(const TString& strToSet)
{
    m_strColArtist = strToSet;
    return m_strColArtist;
}


// Get or set the collection aspect ratio
const TString& TCQCMediaPLItem::strColAspectRatio() const
{
    return m_strColAspectRatio;
}

const TString& TCQCMediaPLItem::strColAspectRatio(const TString& strToSet)
{
    m_strColAspectRatio = strToSet;
    return m_strColAspectRatio;
}


// Get or set the collection cast
const TString& TCQCMediaPLItem::strColCast() const
{
    return m_strColCast;
}

const TString& TCQCMediaPLItem::strColCast(const TString& strToSet)
{
    m_strColCast = strToSet;
    return m_strColCast;
}


// Get or set the collection cookie
const TString& TCQCMediaPLItem::strColCookie() const
{
    return m_strColCookie;
}

const TString& TCQCMediaPLItem::strColCookie(const TString& strToSet)
{
    m_strColCookie = strToSet;
    return m_strColCookie;
}


// Get or set the collection description
const TString& TCQCMediaPLItem::strColDescription() const
{
    return m_strColDescription;
}

const TString& TCQCMediaPLItem::strColDescription(const TString& strToSet)
{
    m_strColDescription = strToSet;
    return m_strColDescription;
}


// Get or set the collection label
const TString& TCQCMediaPLItem::strColLabel() const
{
    return m_strColLabel;
}

const TString& TCQCMediaPLItem::strColLabel(const TString& strToSet)
{
    m_strColLabel = strToSet;
    return m_strColLabel;
}


// Get or set the collection lead actor
const TString& TCQCMediaPLItem::strColLeadActor() const
{
    return m_strColLeadActor;
}

const TString& TCQCMediaPLItem::strColLeadActor(const TString& strToSet)
{
    m_strColLeadActor = strToSet;
    return m_strColLeadActor;
}


// Get or set the collection name
const TString& TCQCMediaPLItem::strColName() const
{
    return m_strColName;
}

const TString& TCQCMediaPLItem::strColName(const TString& strToSet)
{
    m_strColName = strToSet;
    return m_strColName;
}


// Get or set the collection rating
const TString& TCQCMediaPLItem::strColRating() const
{
    return m_strColRating;
}

const TString& TCQCMediaPLItem::strColRating(const TString& strToSet)
{
    m_strColRating = strToSet;
    return m_strColRating;
}


// Get the formatted time
const TString& TCQCMediaPLItem::strFmtTime() const
{
    return m_strFmtTime;
}


// Get or set the item artist
const TString& TCQCMediaPLItem::strItemArtist() const
{
    return m_strItemArtist;
}

const TString& TCQCMediaPLItem::strItemArtist(const TString& strToSet)
{
    m_strItemArtist = strToSet;
    return m_strItemArtist;
}


// Get or set the item cookie
const TString& TCQCMediaPLItem::strItemCookie() const
{
    return m_strItemCookie;
}

const TString& TCQCMediaPLItem::strItemCookie(const TString& strToSet)
{
    m_strItemCookie = strToSet;
    return m_strItemCookie;
}


// Get or set the item name
const TString& TCQCMediaPLItem::strItemName() const
{
    return m_strItemName;
}

const TString& TCQCMediaPLItem::strItemName(const TString& strToSet)
{
    m_strItemName = strToSet;
    return m_strItemName;
}


// Get or set the location info
const TString& TCQCMediaPLItem::strLocInfo() const
{
    return m_strLocInfo;
}

const TString& TCQCMediaPLItem::strLocInfo(const TString& strToSet)
{
    m_strLocInfo = strToSet;
    return m_strLocInfo;
}


// Get or set the media format
const TString& TCQCMediaPLItem::strMediaFormat() const
{
    return m_strMediaFmt;
}

const TString& TCQCMediaPLItem::strMediaFormat(const TString& strToSet)
{
    m_strMediaFmt = strToSet;
    return m_strMediaFmt;
}


// Get or set the repo moniker
const TString& TCQCMediaPLItem::strRepoMoniker() const
{
    return m_strRepoMoniker;
}

const TString& TCQCMediaPLItem::strRepoMoniker(const TString& strToSet)
{
    m_strRepoMoniker = strToSet;
    return m_strRepoMoniker;
}


// Get or set the title cookie
const TString& TCQCMediaPLItem::strTitleCookie() const
{
    return m_strTitleCookie;
}

const TString& TCQCMediaPLItem::strTitleCookie(const TString& strToSet)
{
    m_strTitleCookie = strToSet;
    return m_strTitleCookie;
}


// Get or set the title name
const TString& TCQCMediaPLItem::strTitleName() const
{
    return m_strTitleName;
}

const TString& TCQCMediaPLItem::strTitleName(const TString& strToSet)
{
    m_strTitleName = strToSet;
    return m_strTitleName;
}


// Get or set the user data
const TString& TCQCMediaPLItem::strUserData() const
{
    return m_strUserData;
}

const TString& TCQCMediaPLItem::strUserData(const TString& strToSet)
{
    m_strUserData = strToSet;
    return m_strUserData;
}



//
//  Set up everything in one whack. We have one that's used for when a
//  collection level one is set up and another for item level.
//
tCIDLib::TVoid
TCQCMediaPLItem::Set(const  tCIDLib::TCard4         c4BitDepth
                    , const tCIDLib::TCard4         c4BitRate
                    , const tCIDLib::TCard4         c4Channels
                    , const tCIDLib::TCard4         c4Duration
                    , const tCIDLib::TCard4         c4Year
                    , const tCQCMedia::ELocTypes    eLocType
                    , const tCQCMedia::EMediaTypes  eMediaType
                    , const TString&                strArtist
                    , const TString&                strAspectRatio
                    , const TString&                strCast
                    , const TString&                strColCookie
                    , const TString&                strColName
                    , const TString&                strColRating
                    , const TString&                strDescription
                    , const TString&                strLabel
                    , const TString&                strLeadActor
                    , const TString&                strLocInfo
                    , const TString&                strMediaFmt
                    , const TString&                strRepoMoniker
                    , const TString&                strTitleCookie
                    , const TString&                strTitleName
                    , const tCIDLib::TCard4         c4ListItemId)
{
    m_c4BitDepth        = c4BitDepth;
    m_c4BitRate         = c4BitRate;
    m_c4Channels        = c4Channels;
    m_c4ListItemId      = c4ListItemId;
    m_c4ItemSeconds     = c4Duration;
    m_c4ColYear         = c4Year;
    m_eLocType          = eLocType;
    m_eMediaType        = eMediaType;
    m_strColArtist      = strArtist;
    m_strColAspectRatio = strAspectRatio;
    m_strColCast        = strCast;
    m_strColCookie      = strColCookie;
    m_strColName        = strColName;
    m_strColRating      = strColRating;
    m_strColDescription = strDescription;
    m_strColLabel       = strLabel;
    m_strColLeadActor   = strLeadActor;
    m_strLocInfo        = strLocInfo;
    m_strMediaFmt       = strMediaFmt;
    m_strRepoMoniker    = strRepoMoniker;
    m_strTitleCookie    = strTitleCookie;
    m_strTitleName      = strTitleName;

    // Clear the item level stuff we don't get in this case
    m_strItemArtist.Clear();
    m_strItemCookie.Clear();
    m_strItemName.Clear();

    // Mark us unplayed and clear any other non-set fields
    m_bPlayed = kCIDLib::False;
    m_strUserData.Clear();
}

tCIDLib::TVoid
TCQCMediaPLItem::Set(const  tCIDLib::TCard4         c4BitDepth
                    , const tCIDLib::TCard4         c4BitRate
                    , const tCIDLib::TCard4         c4Channels
                    , const tCIDLib::TCard4         c4Duration
                    , const tCIDLib::TCard4         c4Year
                    , const tCQCMedia::ELocTypes    eLocType
                    , const tCQCMedia::EMediaTypes  eMediaType
                    , const TString&                strArtist
                    , const TString&                strAspectRatio
                    , const TString&                strCast
                    , const TString&                strColCookie
                    , const TString&                strColName
                    , const TString&                strColRating
                    , const TString&                strDescription
                    , const TString&                strLabel
                    , const TString&                strLeadActor
                    , const TString&                strLocInfo
                    , const TString&                strItemArtist
                    , const TString&                strItemCookie
                    , const TString&                strItemName
                    , const TString&                strMediaFmt
                    , const TString&                strRepoMoniker
                    , const TString&                strTitleCookie
                    , const TString&                strTitleName
                    , const tCIDLib::TCard4         c4ListItemId)
{
    m_c4BitDepth        = c4BitDepth;
    m_c4BitRate         = c4BitRate;
    m_c4Channels        = c4Channels;
    m_c4ItemSeconds     = c4Duration;
    m_c4ListItemId      = c4ListItemId;
    m_c4ColYear         = c4Year;
    m_eLocType          = eLocType;
    m_eMediaType        = eMediaType;
    m_strColArtist      = strArtist;
    m_strColAspectRatio = strAspectRatio;
    m_strColCast        = strCast;
    m_strColCookie      = strColCookie;
    m_strColName        = strColName;
    m_strColRating      = strColRating;
    m_strColDescription = strDescription;
    m_strColLabel       = strLabel;
    m_strColLeadActor   = strLeadActor;
    m_strItemArtist     = strItemArtist;
    m_strItemCookie     = strItemCookie;
    m_strItemName       = strItemName;
    m_strLocInfo        = strLocInfo;
    m_strMediaFmt       = strMediaFmt;
    m_strRepoMoniker    = strRepoMoniker;
    m_strTitleCookie    = strTitleCookie;
    m_strTitleName      = strTitleName;

    // Mark us unplayed and clear any other non-set stuff
    m_bPlayed = kCIDLib::False;
    m_strUserData.Clear();
}


//
//  In some cases we have to set the unique list item id after the fact, such
//  as when we get a random category playlist back from the server.
//
tCIDLib::TVoid TCQCMediaPLItem::SetListItemId(const tCIDLib::TCard4 c4ToSet)
{
    m_c4ListItemId = c4ToSet;
}


// ---------------------------------------------------------------------------
//  TCQCMediaPLItem: Protected, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid TCQCMediaPLItem::StreamFrom(TBinInStream& strmToReadFrom)
{
    // It should start with a start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // And check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCMedia_PLItem::c2ItemFmtVersion))
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

    // If we have any nested or parent class to do do it here

    // Stream in our stuff
    strmToReadFrom  >> m_c4ItemSeconds
                    >> m_c4ColYear
                    >> m_eLocType
                    >> m_eMediaType
                    >> m_strColArtist
                    >> m_strColAspectRatio
                    >> m_strColCast
                    >> m_strColCookie
                    >> m_strColName
                    >> m_strColRating
                    >> m_strColDescription
                    >> m_strColLabel
                    >> m_strColLeadActor
                    >> m_strItemArtist
                    >> m_strItemCookie
                    >> m_strItemName
                    >> m_strLocInfo
                    >> m_strMediaFmt
                    >> m_strTitleCookie
                    >> m_strTitleName;

    // If V2 or later read in the added music fomat values, else default them
    if (c2FmtVersion > 1)
    {
        strmToReadFrom  >> m_c4BitDepth
                        >> m_c4BitRate
                        >> m_c4Channels;
    }
     else
    {
        m_c4BitDepth = 0;
        m_c4BitRate = 0;
        m_c4Channels = 2;
    }

    // If V3 or later, read in the moniker and id, else default them
    if (c2FmtVersion > 2)
    {
        strmToReadFrom  >> m_c4ListItemId
                        >> m_strRepoMoniker;
    }
     else
    {
        m_c4ListItemId = 0;
        m_strRepoMoniker.Clear();
    }

    // If V4 or later, read in the user data, else default it
    if (c2FmtVersion > 3)
        strmToReadFrom >> m_strUserData;
    else
        m_strUserData.Clear();

    // And it should end with a frame market
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TCQCMediaPLItem::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Start with a start object and our format version
    strmToWriteTo << tCIDLib::EStreamMarkers::StartObject
                  << CQCMedia_PLItem::c2ItemFmtVersion;

    // If we have any nested or parent class to do do it here

    // Write our stuff and our end marker
    strmToWriteTo   << m_c4ItemSeconds
                    << m_c4ColYear
                    << m_eLocType
                    << m_eMediaType
                    << m_strColArtist
                    << m_strColAspectRatio
                    << m_strColCast
                    << m_strColCookie
                    << m_strColName
                    << m_strColRating
                    << m_strColDescription
                    << m_strColLabel
                    << m_strColLeadActor
                    << m_strItemArtist
                    << m_strItemCookie
                    << m_strItemName
                    << m_strLocInfo
                    << m_strMediaFmt
                    << m_strTitleCookie
                    << m_strTitleName

                    // V2 stuff
                    << m_c4BitDepth
                    << m_c4BitRate
                    << m_c4Channels

                    // V3 stuff
                    << m_c4ListItemId
                    << m_strRepoMoniker

                    // V4 stuff
                    << m_strUserData

                    << tCIDLib::EStreamMarkers::EndObject;
}




// ---------------------------------------------------------------------------
//   CLASS: TCQCMediaPLMgr
//  PREFIX: mpli
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQSLAudPlSDriver: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCMediaPLMgr::TCQCMediaPLMgr() :

    m_c4CurIndex(kCIDLib::c4MaxCard)
    , m_c4NextItemId(1)
    , m_c4SerialNum(0)
    , m_colPlayList(512)
    , m_ePlayListMode(tCQCMedia::EPLModes::Normal)
    , m_eVerboseLvl(tCQCKit::EVerboseLvls::Off)
{
    // Seed the random number generator with the current millis
    m_randShuffle.Seed(TTime::c4Millis());
}

TCQCMediaPLMgr::TCQCMediaPLMgr(const TString& strMoniker) :

    m_c4CurIndex(kCIDLib::c4MaxCard)
    , m_c4NextItemId(1)
    , m_c4SerialNum(0)
    , m_colPlayList(512)
    , m_ePlayListMode(tCQCMedia::EPLModes::Normal)
    , m_eVerboseLvl(tCQCKit::EVerboseLvls::Off)
    , m_strMoniker(strMoniker)
{
    // Seed the random number generator with the current millis
    m_randShuffle.Seed(TTime::c4Millis());
}

TCQCMediaPLMgr::~TCQCMediaPLMgr()
{
}


// ---------------------------------------------------------------------------
//  TCQCMediaPLMgr: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Get the played flag for the item as the indicated index
tCIDLib::TBoolean
TCQCMediaPLMgr::bHasItemPlayed(const tCIDLib::TCard4 c4At) const
{
    return m_colPlayList[c4At].bPlayed();
}


tCIDLib::TBoolean TCQCMediaPLMgr::bIsFull() const
{
    return m_colPlayList.bIsFull(CQCMedia_PLItem::c4MaxPLItems);
}


//
//  Get the repo and cookie for the current item, which is used by
//  renderers to query art info. If this one has an item cookie, it
//  will return that, else collection.
//
tCIDLib::TBoolean
TCQCMediaPLMgr::bQueryCurCookie(TString&                strRepo
                                , TString&              strCookie
                                , tCIDLib::TBoolean&    bItem) const
{
    if (m_c4CurIndex <= m_colPlayList.c4ElemCount())
    {
        const TCQCMediaPLItem& mpliCur = m_colPlayList[m_c4CurIndex];

        strRepo = mpliCur.strRepoMoniker();

        bItem = !mpliCur.strItemCookie().bIsEmpty();
        if (bItem)
            strCookie = mpliCur.strItemCookie();
        else
            strCookie = mpliCur.strColCookie();

        return kCIDLib::True;
    }
    return kCIDLib::False;
}


// Return the playlist id of the current item, if there is one
tCIDLib::TBoolean TCQCMediaPLMgr::bQueryCurItemId(tCIDLib::TCard4& c4ToFill) const
{
    if (!m_colPlayList.c4ElemCount())
    {
        c4ToFill = kCIDLib::c4MaxCard;
        return kCIDLib::False;
    }

    CIDAssert
    (
        m_c4CurIndex <= m_colPlayList.c4ElemCount()
        , L"Current play list index is not legal"
    );
    c4ToFill = m_colPlayList[m_c4CurIndex].c4ListItemId();
    return kCIDLib::True;
}


//
//  This will search the list and return the index of the first item whose
//  location info matches that passed. We return c4MaxCard if it's not
//  found.
//
tCIDLib::TCard4 TCQCMediaPLMgr::c4FindByLocInfo(const TString& strToFind)
{
    const tCIDLib::TCard4 c4Count = m_colPlayList.c4ElemCount();
    tCIDLib::TCard4 c4Index = 0;
    for (; c4Index < c4Count; c4Index++)
    {
        TCQCMediaPLItem& mpliCur = m_colPlayList[c4Index];
        if (mpliCur.strLocInfo() == strToFind)
            break;
    }
    if (c4Index == c4Count)
        c4Index = kCIDLib::c4MaxCard;
    return c4Index;
}


//
//  This is called when the player driver has been asked to add new items
//  to the playlist (which may or may not replace any existing content.)
//  They give us the acceptable types of media that we should accept. If it's
//  not one of those, we return rejected.
//
//  The 'to add' info can just be a cookie, or a cookie, space then a repo
//  moniker. If it's just a cookie, we use the passed default repo moniker.
//
tCQCMedia::EPLMgrActs
TCQCMediaPLMgr::eAddMedia(  const   TString&            strDefRepo
                            , const TString&            strToAdd
                            , const tCIDLib::TBoolean   bEnqueue
                            , const tCQCMedia::EMTFlags eAcceptable
                            , const TString&            strUserData)
{
    TString strTmp;
    TString strCookie;
    TString strRepoMoniker;

    //
    //  Get the cookie and possibly repo out. If no repo, use the default
    //  one we were given.
    //
    strCookie = strToAdd;
    if (!strCookie.bSplit(strRepoMoniker, kCIDLib::chSpace))
        strRepoMoniker = strDefRepo;

    strRepoMoniker.StripWhitespace();
    strCookie.StripWhitespace();


    // Parse this cookie using the version that figures out the type for us
    tCIDLib::TCard2        c2CatId;
    tCIDLib::TCard2        c2ColId;
    tCIDLib::TCard2        c2ItemId;
    tCIDLib::TCard2        c2TitleId;
    tCQCMedia::EMediaTypes eType;
    const tCQCMedia::ECookieTypes eCookieType = facCQCMedia().eCheckCookie
    (
        strCookie, eType, c2CatId, c2TitleId, c2ColId, c2ItemId
    );

    // See if it's an acceptable type of media
    if (!facCQCMedia().bTestMediaFlags(eType, eAcceptable))
    {
        if (m_eVerboseLvl >= tCQCKit::EVerboseLvls::Low)
        {
            facCQCMedia().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kMedErrs::errcRend_UnsupportedMType
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
                , tCQCMedia::strXlatEMediaTypes(eType)
            );
        }
        return tCQCMedia::EPLMgrActs::Rejected;
    }

    //
    //  We need a title and collection cookie, no matter what we got, so
    //  build those up as required.
    //
    TString strColCookie;
    TString strTitleCookie;
    if (eCookieType == tCQCMedia::ECookieTypes::Title)
    {
        strColCookie = strCookie;
        strColCookie.Append(L",1");
        strTitleCookie = strCookie;
    }
     else if (eCookieType == tCQCMedia::ECookieTypes::Collect)
    {
        strColCookie = strCookie;
        facCQCMedia().FormatTitleCookie(eType, c2CatId, c2TitleId, strTitleCookie);
    }
     else if (eCookieType == tCQCMedia::ECookieTypes::Item)
    {
        facCQCMedia().FormatColCookie(eType, c2CatId, c2TitleId, c2ColId, strColCookie);
        facCQCMedia().FormatTitleCookie(eType, c2CatId, c2TitleId, strTitleCookie);
    }
     else
    {
        if (m_eVerboseLvl >= tCQCKit::EVerboseLvls::Low)
        {
            facCQCMedia().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kMedErrs::errcRend_BadCookie
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::BadParms
            );
        }
        return tCQCMedia::EPLMgrActs::Rejected;
    }

    // Get a proxy for the repository driver
    tCQCKit::TCQCSrvProxy orbcSrv
    (
        facCQCKit().orbcCQCSrvAdminProxy(strRepoMoniker)
    );

    // Get the details for this title set and collection
    tCIDLib::TCard4 c4ColCnt;
    TMediaCollect   mcolQuery;
    TString         strTitleName;
    facCQCMedia().eQueryColDetails
    (
        orbcSrv
        , strRepoMoniker
        , strColCookie
        , mcolQuery
        , strTitleName
        , c4ColCnt
    );

    // Get the item count out for convenience below
    const tCIDLib::TCard4 c4ItemCnt = mcolQuery.c4ItemCount();

    //
    //  If it's a PlayMedia, then clear any current play list. Otherwise,
    //  it's an enqueue so we just add to any exiting stuff. Only do it
    //  if it's not already empty, since that's just another possible
    //  serial number change that we don't need to do otherwise.
    //
    if (!bEnqueue && !m_colPlayList.bIsEmpty())
    {
        m_colPlayList.RemoveAll();
        m_c4SerialNum++;
    }

    // Remember if the list is empty before we add a new one
    const tCIDLib::TBoolean bEmptyList = m_colPlayList.bIsEmpty();

    TCQCMediaPLItem mpliNew;
    if ((eCookieType == tCQCMedia::ECookieTypes::Collect)
    ||  (eCookieType == tCQCMedia::ECookieTypes::Title))
    {
        //
        //  We only support file based location types.
        //
        if (mcolQuery.eLocType() == tCQCMedia::ELocTypes::FileCol)
        {
            // If full, reject it
            if (m_colPlayList.bIsFull(CQCMedia_PLItem::c4MaxPLItems))
                return tCQCMedia::EPLMgrActs::Rejected;

            //
            //  Set up a single play list item and add it to the playlist.
            //  We have no item level stuff here. We don't have access to
            //  bit depth, bit rate, or channels here.
            //
            mpliNew.Set
            (
                0
                , 0
                , 0
                , mcolQuery.c4Duration()
                , mcolQuery.c4Year()
                , mcolQuery.eLocType()
                , eType
                , mcolQuery.strArtist()
                , mcolQuery.strAspectRatio()
                , mcolQuery.strCast()
                , strColCookie
                , mcolQuery.strName()
                , mcolQuery.strRating()
                , mcolQuery.strDescr()
                , mcolQuery.strLabel()
                , mcolQuery.strLeadActor()
                , mcolQuery.strLocInfo()
                , mcolQuery.strMediaFormat()
                , strRepoMoniker
                , strTitleCookie
                , strTitleName
                , m_c4NextItemId++
            );
            TCQCMediaPLItem& mpliAdded = m_colPlayList.objAdd(mpliNew);
            mpliAdded.strUserData(strUserData);
            m_c4SerialNum++;
        }
         else if (mcolQuery.eLocType() == tCQCMedia::ELocTypes::FileItem)
        {
            //
            //  Get the item list for this collection, and add a playlist
            //  item for each item.
            //
            TVector<TMediaItem> colItems(c4ItemCnt);
            facCQCMedia().c4QueryColItems
            (
                orbcSrv, strRepoMoniker, strColCookie, colItems
            );

            // We have to build up the item cookies on each round
            TString strItemCookie;
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ItemCnt; c4Index++)
            {
                // If we fill up, then just stop
                if (m_colPlayList.bIsFull(CQCMedia_PLItem::c4MaxPLItems))
                    break;

                const TMediaItem& mitemCur = colItems[c4Index];
                strItemCookie = strColCookie;
                strItemCookie.Append(L",");
                strItemCookie.AppendFormatted(c4Index + 1, tCIDLib::ERadices::Hex);

                mpliNew.Set
                (
                    mitemCur.c4BitDepth()
                    , mitemCur.c4BitRate()
                    , mitemCur.c4Channels()
                    , mitemCur.c4Duration()
                    , mcolQuery.c4Year()
                    , mcolQuery.eLocType()
                    , eType
                    , mcolQuery.strArtist()
                    , mcolQuery.strAspectRatio()
                    , mcolQuery.strCast()
                    , strColCookie
                    , mcolQuery.strName()
                    , mcolQuery.strRating()
                    , mcolQuery.strDescr()
                    , mcolQuery.strLabel()
                    , mcolQuery.strLeadActor()
                    , mitemCur.strLocInfo()
                    , mitemCur.strArtist()
                    , strItemCookie
                    , mitemCur.strName()
                    , mcolQuery.strMediaFormat()
                    , strRepoMoniker
                    , strTitleCookie
                    , strTitleName
                    , m_c4NextItemId++
                );

                // Now add it to our list and set any user data
                TCQCMediaPLItem& mpliAdded = m_colPlayList.objAdd(mpliNew);
                mpliAdded.strUserData(strUserData);
            }

            // Bump the serial number for the changes
            m_c4SerialNum++;
        }
         else
        {
            // It's not a location type we support
            if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Low)
            {
                facCQCMedia().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kMedErrs::errcDB_WrongLocType
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::BadParms
                    , tCQCMedia::strXlatELocTypes(mcolQuery.eLocType())
                );
            }
            return tCQCMedia::EPLMgrActs::Rejected;
        }
    }
     else
    {
        // If full, reject it
        if (m_colPlayList.bIsFull(CQCMedia_PLItem::c4MaxPLItems))
            return tCQCMedia::EPLMgrActs::Rejected;

        //
        //  It's just the addition of a single item to the list, so
        //  get the item details for this one item and set up a new
        //  item.
        //
        TMediaItem mitemQuery;
        facCQCMedia().QueryItemDetails
        (
            orbcSrv, strRepoMoniker, strCookie, mitemQuery
        );

        mpliNew.Set
        (
            mitemQuery.c4BitDepth()
            , mitemQuery.c4BitRate()
            , mitemQuery.c4Channels()
            , mitemQuery.c4Duration()
            , mcolQuery.c4Year()
            , mcolQuery.eLocType()
            , eType
            , mcolQuery.strArtist()
            , mcolQuery.strAspectRatio()
            , mcolQuery.strCast()
            , strColCookie
            , mcolQuery.strName()
            , mcolQuery.strRating()
            , mcolQuery.strDescr()
            , mcolQuery.strLabel()
            , mcolQuery.strLeadActor()
            , mitemQuery.strLocInfo()
            , mitemQuery.strArtist()
            , strCookie
            , mitemQuery.strName()
            , mcolQuery.strMediaFormat()
            , strRepoMoniker
            , strTitleCookie
            , strTitleName
            , m_c4NextItemId++
        );

        TCQCMediaPLItem& mpliAdded = m_colPlayList.objAdd(mpliNew);
        mpliAdded.strUserData(strUserData);

        m_c4SerialNum++;
    }

    // If the next item id rolled over, skip zero
    if (!m_c4NextItemId)
        m_c4NextItemId++;

    //
    //  If the list was empty and isn't now, then tell the caller they need
    //  to select a new item.
    //
    if (bEmptyList && !m_colPlayList.bIsEmpty())
    {
        m_c4CurIndex = kCIDLib::c4MaxCard;
        return tCQCMedia::EPLMgrActs::NewSelection;
    }
    return tCQCMedia::EPLMgrActs::NoAction;
}



//
//  If the player is in random category play mode, he'll call us when the
//  list gets down a bit from the size it wants to maintain. We'll ask the
//  repository for some random selections from the indicated category and
//  add them to our playlist.
//
tCQCMedia::EPLMgrActs
TCQCMediaPLMgr::eRandomCatUpdate(const  TString&    strRepository
                                , const TString&    strCatCookie)
{
    // Remember if we were empty
    const tCIDLib::TBoolean bWasEmpty = m_colPlayList.bIsEmpty();

    try
    {
        // Get a proxy for the repository driver we wer told
        tCQCKit::TCQCSrvProxy orbcSrv
        (
            facCQCKit().orbcCQCSrvAdminProxy(strRepository)
        );

        // We'll only keep up to c4MaxRandCatItems in this mode
        const tCIDLib::TCard4 c4MaxItems
        (
            (m_colPlayList.c4ElemCount() < CQCMedia_PLItem::c4MaxRandCatItems)
                ? (CQCMedia_PLItem::c4MaxRandCatItems - m_colPlayList.c4ElemCount())
                : 0
        );

        // If already more than max, do nothing
        if (!c4MaxItems)
            return tCQCMedia::EPLMgrActs::NoAction;

        // Ask for as many as we can add
        TVector<TCQCMediaPLItem> colNewItems;
        const tCIDLib::TCard4 c4Count = facCQCMedia().c4QueryRandomCatItems
        (
            orbcSrv
            , strRepository
            , strCatCookie
            , c4MaxItems
            , colNewItems
        );

        // If we got any back, add them to our playlist
        if (c4Count)
        {
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            {
                // If we fill up, then just stop
                if (m_colPlayList.bIsFull(CQCMedia_PLItem::c4MaxPLItems))
                    break;

                //
                //  We have to assign unique playlist item ids to these guys
                //  as we add them.
                //
                TCQCMediaPLItem& mpliCur = m_colPlayList.objAdd(colNewItems[c4Index]);
                mpliCur.SetListItemId(m_c4NextItemId++);

                // If the next item id rolled over, skip zero
                if (!m_c4NextItemId)
                    m_c4NextItemId++;
            }
            m_c4SerialNum++;
        }
    }

    catch(const TError& errToCatch)
    {
        if (facCQCMedia().bShouldLog(errToCatch))
            TModule::LogEventObj(errToCatch);
    }

    //
    //  If we were empty before and not now, then we need to tell the
    //  calling player to start playing something. Else, nothing to do since
    //  something is already playing.
    //
    if (bWasEmpty && !m_colPlayList.bIsEmpty())
    {
        m_c4CurIndex = kCIDLib::c4MaxCard;
        return tCQCMedia::EPLMgrActs::NewSelection;
    }
    return tCQCMedia::EPLMgrActs::NoAction;
}



//
//  Remove the item at the indicated index.
//
tCQCMedia::EPLMgrActs TCQCMediaPLMgr::eRemoveItemAt(const tCIDLib::TCard4 c4At)
{
    tCQCMedia::EPLMgrActs eRet = tCQCMedia::EPLMgrActs::NoAction;

    // If it's beyond our playlist index, then do nothing
    if (c4At >= m_colPlayList.c4ElemCount())
        return eRet;

    // Looks legal so remove it and bump the serial number
    m_colPlayList.RemoveAt(c4At);
    m_c4SerialNum++;

    if (m_colPlayList.bIsEmpty())
    {
        // If the list is now empty, then tell them to just stop
        m_c4CurIndex = kCIDLib::c4MaxCard;
        eRet = tCQCMedia::EPLMgrActs::Stop;
    }
     else if (c4At == m_c4CurIndex)
    {
        // If it's the current item, then they need to select a new one
        m_c4CurIndex = kCIDLib::c4MaxCard;
        eRet = tCQCMedia::EPLMgrActs::NewSelection;
    }
     else if (m_c4CurIndex > c4At)
    {
        //
        //  If the current index is beyond the delete index, then
        //  we need to bring it down since the list has been moved
        //  down.
        //
        m_c4CurIndex--;
    }
    return eRet;
}


//
//  If the passed unique id is in the list, that item is removed. The return
//  value indicates whether we found it or not.
//
tCQCMedia::EPLMgrActs
TCQCMediaPLMgr::eRemoveItemById(const tCIDLib::TCard4 c4Id)
{
    // Assume we are removing something besides the current item
    tCQCMedia::EPLMgrActs eRet = tCQCMedia::EPLMgrActs::NoAction;

    tCIDLib::TCard4 c4Count = m_colPlayList.c4ElemCount();
    tCIDLib::TCard4 c4Index = 0;
    for (; c4Index < c4Count; c4Index++)
    {
        TCQCMediaPLItem& mpliCur = m_colPlayList[c4Index];
        if (mpliCur.c4ListItemId() == c4Id)
            break;
    }

    // If we found it, then let's remove it
    if (c4Index < c4Count)
    {
        m_colPlayList.RemoveAt(c4Index);
        m_c4SerialNum++;
        c4Count--;

        if (m_colPlayList.bIsEmpty())
        {
            // If that was the last one, tell them to stop
            m_c4CurIndex = kCIDLib::c4MaxCard;
            eRet = tCQCMedia::EPLMgrActs::Stop;
        }
         else if (c4Index == m_c4CurIndex)
        {
            //
            //  They deleted the current one. We just let the index stay
            //  where it was, which and indicate we selected a new one,
            //  which will make the next one (now in the same index) play.
            //
            //  But, if we deleted the last one, we have to go backwards.
            //
            if (m_c4CurIndex >= c4Count)
                m_c4CurIndex--;
            eRet = tCQCMedia::EPLMgrActs::NewItem;
        }
         else if (m_c4CurIndex > c4Index)
        {
            //
            //  If the current index is beyond the delete index, then
            //  we need to bring it down since the list has been moved
            //  down, and we need to keep ourself pointing to the same
            //  item.
            //
            m_c4CurIndex--;
        }
    }
     else
    {
        // It was never found, so reject it
        eRet = tCQCMedia::EPLMgrActs::Rejected;
    }

    // Never found it
    return eRet;
}


//
//  If the passed unique id is in the list, make that the active item.
//
tCQCMedia::EPLMgrActs
TCQCMediaPLMgr::eSelectItemById(const tCIDLib::TCard4 c4Id)
{
    // Assume we are removing something besides the current item
    tCQCMedia::EPLMgrActs eRet = tCQCMedia::EPLMgrActs::NoAction;

    const tCIDLib::TCard4 c4Count = m_colPlayList.c4ElemCount();
    tCIDLib::TCard4 c4Index = 0;
    for (; c4Index < c4Count; c4Index++)
    {
        TCQCMediaPLItem& mpliCur = m_colPlayList[c4Index];
        if (mpliCur.c4ListItemId() == c4Id)
            break;
    }

    if (c4Index < c4Count)
    {
        //
        //  We found it. If it' sthe current item, we do nothing. Else
        //  we select the new one.
        //
        if (c4Index != m_c4CurIndex)
        {
            m_c4CurIndex = c4Index;
            m_colPlayList[m_c4CurIndex].bPlayed(kCIDLib::True);

            // And indicate a new item has been selected
            eRet = tCQCMedia::EPLMgrActs::NewItem;
        }
    }
     else
    {
        // It was never found, so reject it
        eRet = tCQCMedia::EPLMgrActs::Rejected;
    }

    // Never found it
    return eRet;
}


//
//  This is called by the client code when it wants us to make a new
//  selection. If we are in jukebox mode, then the current item will be
//  removed before select a new one.
//
tCQCMedia::EPLMgrSelRes
TCQCMediaPLMgr::eSelectNewItem( const   tCIDLib::TBoolean   bNext
                                ,       tCIDLib::TCard4&    c4NewIndex)
{
    // Most likely scenario is a new item being selected
    tCQCMedia::EPLMgrSelRes eRes = tCQCMedia::EPLMgrSelRes::NewItem;

    //
    //  If we are in jukebox or random cat type mode, then we need to
    //  remove the item we just finished (if any.) If there's only one
    //  left, then call the playlist clear method to clean us up and
    //  put us back to inactive mode.
    //
    if (((m_ePlayListMode == tCQCMedia::EPLModes::Jukebox)
    ||   (m_ePlayListMode == tCQCMedia::EPLModes::RandomCat))
    &&  (m_c4CurIndex != kCIDLib::c4MaxCard))
    {
        m_colPlayList.RemoveAt(m_c4CurIndex);
        m_c4SerialNum++;

        // The play list has changed, so indicate that
        eRes = tCQCMedia::EPLMgrSelRes::NewAndChanged;
    }

    if (m_colPlayList.bIsEmpty())
    {
        // Nothing left to play
        m_c4CurIndex = kCIDLib::c4MaxCard;
        eRes = tCQCMedia::EPLMgrSelRes::Empty;
    }
     else
    {
        if (m_ePlayListMode == tCQCMedia::EPLModes::Shuffle)
        {
            //
            //  We are in suffle mode, so just suffle select a new
            //  index to play.
            //
            m_c4CurIndex = c4ShuffleSelect();
        }
         else
        {
            // Else just move back or forward as asked
            const tCIDLib::TCard4 c4Count = m_colPlayList.c4ElemCount();
            if (bNext)
            {
                //
                //  If in jukebox mode, we deleted the current item, so the
                //  current index already points to the next item so we do
                //  not need to increment. But deal with the special case
                //  of moving to the first song in a newly loaded list.
                //
                if (m_c4CurIndex == kCIDLib::c4MaxCard)
                {
                    m_c4CurIndex = 0;
                }
                 else if ((m_ePlayListMode != tCQCMedia::EPLModes::Jukebox)
                      &&  (m_ePlayListMode != tCQCMedia::EPLModes::RandomCat))
                {
                    m_c4CurIndex++;
                }

                // Watch for wrap around
                if (m_c4CurIndex >= c4Count)
                    m_c4CurIndex = 0;
            }
             else
            {
                if (m_c4CurIndex)
                    m_c4CurIndex--;
                else
                    m_c4CurIndex = c4Count - 1;
            }
        }
    }

    // Mark this one played if we got one
    if (m_c4CurIndex != kCIDLib::c4MaxCard)
        m_colPlayList[m_c4CurIndex].bPlayed(kCIDLib::True);

    // Return the result
    c4NewIndex = m_c4CurIndex;
    return eRes;
}


// Get or set the verbose mode member
tCQCKit::EVerboseLvls TCQCMediaPLMgr::eVerboseLevel() const
{
    return m_eVerboseLvl;
}

tCQCKit::EVerboseLvls
TCQCMediaPLMgr::eVerboseLevel(const tCQCKit::EVerboseLvls eToSet)
{
    m_eVerboseLvl = eToSet;
    return m_eVerboseLvl;
}


// Get or set the play list mode
tCQCMedia::EPLModes TCQCMediaPLMgr::ePlayListMode() const
{
    return m_ePlayListMode;
}

tCQCMedia::EPLModes
TCQCMediaPLMgr::ePlayListMode(const tCQCMedia::EPLModes eToSet)
{
    m_ePlayListMode = eToSet;
    return m_ePlayListMode;
}


// Provides access by index to our play list items
const TCQCMediaPLItem& TCQCMediaPLMgr::mpliAt(const tCIDLib::TCard4 c4At) const
{
    return m_colPlayList[c4At];
}


// A convenience method to access the current item
const TCQCMediaPLItem& TCQCMediaPLMgr::mpliCurrent() const
{
    CIDAssert
    (
        m_c4CurIndex <= m_colPlayList.c4ElemCount()
        , L"Current play list index is not legal"
    );
    return m_colPlayList[m_c4CurIndex];
}


// Just puts us back to empty state
tCIDLib::TVoid TCQCMediaPLMgr::Reset()
{
    m_colPlayList.RemoveAll();
    m_c4SerialNum++;
    m_c4CurIndex = kCIDLib::c4MaxCard;
}


//
//  Returns the item cookie for the item at the indicated index. This is
//  a field that is often accessed so we provide a convenience method to
//  get it directly.
//
const TString&
TCQCMediaPLMgr::strItemCookieAt(const tCIDLib::TCard4 c4At) const
{
    return m_colPlayList[c4At].strItemCookie();
}


// Get or set the moniker associated with this playlist
const TString& TCQCMediaPLMgr::strMoniker() const
{
    return m_strMoniker;
}

const TString& TCQCMediaPLMgr::strMoniker(const TString& strToSet)
{
    m_strMoniker = strToSet;
    return m_strMoniker;
}


//
//  Set an arbitrary list of items on us. We will assign our own playlist
//  ids to them if asked to.
//
tCIDLib::TVoid
TCQCMediaPLMgr::SetItemList(const   TVector<TCQCMediaPLItem>&   colToSet
                            , const tCIDLib::TBoolean           bSetIds)
{
    m_colPlayList = colToSet;
    m_c4SerialNum++;
    m_c4CurIndex = kCIDLib::c4MaxCard;

    // Update them to have list ids based on our sequence if asked
    if (bSetIds)
    {
        const tCIDLib::TCard4 c4Count = m_colPlayList.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            m_colPlayList[c4Index].SetListItemId(m_c4NextItemId++);
            if (!m_c4NextItemId)
                m_c4NextItemId++;
        }
    }
}


// ---------------------------------------------------------------------------
//  TCQCMediaPLMgr: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TCard4 TCQCMediaPLMgr::c4ShuffleSelect()
{
    const tCIDLib::TCard4 c4Count = m_colPlayList.c4ElemCount();
    tCIDLib::TCard4 c4New = 0;
    if (c4Count == 1)
    {
        // We can only return zero
        c4New = 0;
    }
     else if (c4Count == 2)
    {
        // We can only flip back and forth
        if (m_c4CurIndex)
            c4New = 0;
        else
            c4New = 1;
    }
     else
    {
        //
        //  We have enough items that we want to use the recently played
        //  rejection scheme. So we allocate a card4 vector of the size
        //  of the play list and then go through the play list and for each
        //  one marked not played, we add that guy's index to our vector.
        //  This will give us the pool of available yet unplayed items to
        //  play.
        //
        TFundVector<tCIDLib::TCard4> fcolAvail(c4Count);
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const TCQCMediaPLItem& mpliCur = m_colPlayList[c4Index];
            if (!mpliCur.bPlayed())
                fcolAvail.c4AddElement(c4Index);
        }

        //
        //  If we have none that haven't been played yet, then go through
        //  clear the played mark on all of them and select a new one
        //  randomly from the overall list. Else selection one from the
        //  list that hasn't been played yet.
        //
        tCIDLib::TCard4 c4AvailCnt = fcolAvail.c4ElemCount();
        if (c4AvailCnt == 0)
        {
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            {
                m_colPlayList[c4Index].bPlayed(kCIDLib::False);
                fcolAvail.c4AddElement(c4Index);
            }
            c4AvailCnt = c4Count;
        }

        if (c4AvailCnt == 1)
        {
            // We can only return this last unplayed one
            c4New = fcolAvail[0];
        }
         else
        {
            //
            //  Randomly select from the available list. We XOR and shift
            //  a value a few times, then do a modulus div by the number
            //  of items we have available. This should do a relatively
            //  reasonable random selection.
            //
            c4New = m_randShuffle.c4GetNextNum();
            c4New >>= 3;
            c4New ^= m_randShuffle.c4GetNextNum();
            c4New >>= 7;
            c4New ^= m_randShuffle.c4GetNextNum();
            c4New >>= 1;
            c4New ^= m_randShuffle.c4GetNextNum();
            c4New %= c4AvailCnt;

            // Now pull out this one from the list of available ones
            c4New = fcolAvail[c4New];
        }
    }
    return c4New;
}



// ---------------------------------------------------------------------------
//  TCQCMediaPLItem: Protected, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid TCQCMediaPLMgr::StreamFrom(TBinInStream& strmToReadFrom)
{
    // It should start with a start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // And check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCMedia_PLItem::c2ListFmtVersion))
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

    // Stream in our stuff
    strmToReadFrom  >> m_c4SerialNum
                    >> m_ePlayListMode
                    >> m_colPlayList;

    // And it should end with a frame market
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);

    // Reset runtime stuff
    m_strMoniker.Clear();
}


tCIDLib::TVoid TCQCMediaPLMgr::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Start with a start object and our format version
    strmToWriteTo << tCIDLib::EStreamMarkers::StartObject
                  << CQCMedia_PLItem::c2ListFmtVersion;

    // If we have any nested or parent class to do do it here

    // Do our stuff and the end market
    strmToWriteTo   << m_c4SerialNum
                    << m_ePlayListMode
                    << m_colPlayList
                    << tCIDLib::EStreamMarkers::EndObject;
}


