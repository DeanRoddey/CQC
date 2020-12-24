//
// FILE NAME: CQCMedia_ThisFacility.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/13/2005
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
#include    "CQCMedia_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TFacCQCMedia,TFacility)



// ---------------------------------------------------------------------------
//   CLASS: TMDBCacheItem
//  PREFIX: mdbci
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TMDBCacheItem: Constructors and Destructor
// ---------------------------------------------------------------------------

// Zero the next check so that newly added ones will be updated immediately
TMDBCacheItem::TMDBCacheItem(const TString& strRepo) :

    m_enctNextCheck(0)
    , m_eMTFlags(tCQCMedia::EMTFlags::None)
    , m_strRepoMoniker(strRepo)
{
}

TMDBCacheItem::TMDBCacheItem(const  TString&        strRepo
                            ,       TBinInStream&   strmSrc) :

    m_enctNextCheck(TTime::enctNowPlusSecs(15))
    , m_strRepoMoniker(strRepo)
{
    tCIDLib::TCard2 c2FmtVer;
    strmSrc >> c2FmtVer
            >> m_strDBSerialNum
            >> m_eMTFlags
            >> m_mdbData;

    //
    //  We want by artist views of the data to be available as well, so tell the
    //  database to generate those.
    //
    m_mdbData.LoadByArtistMap();
}

TMDBCacheItem::~TMDBCacheItem()
{
}


// ---------------------------------------------------------------------------
//   CLASS: TFacCQCMedia
//  PREFIX: fac
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TFacCQCMedia: Public, static methods
// ---------------------------------------------------------------------------

//
//  Just a key extractor so we can store media DB objects in a keyed hash set.
//  We store them in a counted pointer, so we just access the cache item within
//  the counter pointer and return the key.
//
const TString& TFacCQCMedia::strMDBKey(const TMDBPtr& cptrSrc)
{
    return cptrSrc->strRepoMoniker();
}


// ---------------------------------------------------------------------------
//  TFacCQCMedia: Constructors and Destructor
// ---------------------------------------------------------------------------
TFacCQCMedia::TFacCQCMedia() :

    TFacility
    (
        L"CQCMedia"
        , tCIDLib::EModTypes::SharedLib
        , kCQCKit::c4MajVersion
        , kCQCKit::c4MinVersion
        , kCQCKit::c4Revision
        , tCIDLib::EModFlags::HasMsgFile
    )
    , m_colMediaDBCache(29, TStringKeyOps(), TFacCQCMedia::strMDBKey, tCIDLib::EMTStates::Safe)
    , m_thrMDBCache
      (
        L"CQCMediaMDBCacher", TMemberFunc<TFacCQCMedia>(this, &TFacCQCMedia::eMDBCacheThread)
      )
    , m_colTrivWords(11, TStringKeyOps())
{
    //
    //  WE CANNOT do any stuff that loads text from our loadable resources, since
    //  that would call us back here, because the lazy eval method won't store the
    //  pointer to us until we return. We'll fault in any that are required.
    //


    // Load our trivial words list
    m_colTrivWords.objPlace(L"A");
    m_colTrivWords.objPlace(L"AN");
    m_colTrivWords.objPlace(L"AND");
    m_colTrivWords.objPlace(L"BY");
    m_colTrivWords.objPlace(L"THE");
    m_colTrivWords.objPlace(L"OF");
    m_colTrivWords.objPlace(L"WITH");
}

TFacCQCMedia::~TFacCQCMedia()
{
}


// ---------------------------------------------------------------------------
//  TFacCQCMedia: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  A convenience to wrap the general cookie parsing method, which just returns
//  a success or failure and the cookie type. So basically a quick check of
//  validity and the type if valid. It eats the other parameters.
//
tCIDLib::TBoolean
TFacCQCMedia::bCheckCookie( const   TString&                    strCookie
                            ,       tCQCMedia::ECookieTypes&    eType) const
{
    tCQCMedia::EMediaTypes eMType;
    tCIDLib::TCard2        c2CatId;
    tCIDLib::TCard2        c2TitleId;
    tCIDLib::TCard2        c2ColId;
    tCIDLib::TCard2        c2ItemId;

    try
    {
        eType = eCheckCookie
        (
            strCookie, eMType, c2CatId, c2TitleId, c2ColId, c2ItemId
        );
    }

    catch(...)
    {
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


//
//  Given a desired media type, and the actual supported media types that
//  were discovered, figure out a default category and give back the cookie
//  and media type for that category.
//
tCIDLib::TBoolean
TFacCQCMedia::bDefCategory( const   tCQCMedia::EMediaTypes  eDesired
                            , const tCQCMedia::EMTFlags     eSupported
                            ,       TString&                strName
                            ,       TString&                strCookie
                            ,       tCQCMedia::EMediaTypes& eMType
                            ,       tCIDLib::TCard2&        c2Id)
{
    // First see if the requested type is supported
    if (facCQCMedia().bTestMediaFlags(eDesired, eSupported))
    {
        // It was, so take that one
        if (eDesired == tCQCMedia::EMediaTypes::Music)
        {
            c2Id = kCQCMedia::c2CatId_AllMusic;
            eMType = tCQCMedia::EMediaTypes::Music;
            strName = facCQCMedia().strMsg(kMedMsgs::midCat_AllMusic);
        }
         else
        {
            c2Id = kCQCMedia::c2CatId_AllMovies;
            eMType = tCQCMedia::EMediaTypes::Movie;
            strName = facCQCMedia().strMsg(kMedMsgs::midCat_AllMovies);
        }
    }
     else if (tCIDLib::bAllBitsOn(eSupported, tCQCMedia::EMTFlags::Music))
    {
        c2Id = kCQCMedia::c2CatId_AllMusic;
        eMType = tCQCMedia::EMediaTypes::Music;
        strName = facCQCMedia().strMsg(kMedMsgs::midCat_AllMusic);
    }
     else if (tCIDLib::bAllBitsOn(eSupported, tCQCMedia::EMTFlags::Movie))
    {
        c2Id = kCQCMedia::c2CatId_AllMovies;
        eMType = tCQCMedia::EMediaTypes::Movie;
        strName = facCQCMedia().strMsg(kMedMsgs::midCat_AllMovies);
    }
     else
    {
        return kCIDLib::False;
    }

    // Format up the category cookie
    facCQCMedia().FormatCatCookie(eMType, c2Id, strCookie);
    return kCIDLib::True;
}


//
//  Given the value of a playlist item key field from a renderer driver, parse
//  it out if it is a standard one. If so, and it's well formatted, return trye.
//
tCIDLib::TBoolean
TFacCQCMedia::bParsePLItemKey(  const   TString&            strPLItemKey
                                ,       TString&            strMoniker
                                ,       TString&            strCookie)
{
    // First check the key. If not a standard one, return false now
    if (!strPLItemKey.bStartsWith(kCQCMedia::strRend_StdPLItemKeyPref))
        return kCIDLib::False;

    //
    //  Cut out the prefix and parse out the other two bits. Make sure we
    //  get two. If not, return false.
    //
    strMoniker = strPLItemKey;
    strMoniker.CutUpTo(kCIDLib::chColon);

    return strMoniker.bSplit(strCookie, kCIDLib::chSpace);
}


//
//  This guy implements the standard first character check algorithm. We can
//  optionally skip leading pronouns and such, i.e. 'The' 'A', and so forth,
//  and match on the first significant word.
//
//  We give back the actual character that we ended up comparing for the
//  match/no match test, so that the caller will know what we tried to compare
//  the incoming character to. If trivial words are not being skipped, then we
//  assume that the passed string to test has no leading whitespace, of it
//  does then it is important and should be tested against.
//
tCIDLib::TBoolean
TFacCQCMedia::bFirstCharMatch(  const   TString&            strToCheck
                                , const tCIDLib::TCh        chFirst
                                ,       tCIDLib::TCh&       chActual
                                , const tCIDLib::TBoolean   bNoTrivialWords) const
{
    //
    //  If trivial words are to be ignored, then we have to scan the string
    //  for the first non-trivial word. Else we can just look at it as is.
    //
    const tCIDLib::TCh chUpFirst = TRawStr::chUpper(chFirst);

    if (bNoTrivialWords)
    {
        TStringTokenizer stokCheck(&strToCheck, L" ,.~`-&/+!@#$%^*()][{}|\\/?><");
        TString strWord;
        while (stokCheck.bGetNextToken(strWord))
        {
            //
            //  If it isn't one of the trivials, then we are going to break
            //  out one way or another since this one has to either pass or
            //  fail.
            //
            strWord.ToUpper();
            if (!bIsTrivialWord(strWord))
            {
                // Give back the actual first char of the word we tried
                chActual = strWord.chFirst();
                return (strWord.chFirst() == chUpFirst);
            }
        }
        chActual = kCIDLib::chNull;
        return kCIDLib::False;
    }

    // We are doing all words, so just take the first char
    if (strToCheck.bIsEmpty())
        chActual = kCIDLib::chNull;
    else
        chActual = TRawStr::chUpper(strToCheck.chFirst());
    return (chUpFirst == chActual);
}


//
//  Sees if the cookie is an 'all' type of category cookie, i.e. one of the
//  special 'All Movies', 'All Music', etc...
//
tCIDLib::TBoolean
TFacCQCMedia::bIsAnAllCat(  const   TString&                strCatCookie
                            ,       tCQCMedia::EMediaTypes& eType) const
{
    tCIDLib::TCard2 c2CatId;
    eType = eParseCatCookie(strCatCookie, c2CatId);
    return (c2CatId == kCQCMedia::c2CatId_AllMovies)
           ||  (c2CatId == kCQCMedia::c2CatId_AllMusic)
           ||  (c2CatId == kCQCMedia::c2CatId_AllPictures);
}


//
//  See's if the passed media category id is a special one. Currently it's an
//  easy decision. Anything that is below the first available user catagory id
//  is special.
//
tCIDLib::TBoolean
TFacCQCMedia::bIsSpecialCat(const   tCIDLib::TCard2         c2CatId
                            ,       tCQCMedia::EMediaTypes& eType) const
{
    return (c2CatId < kCQCMedia::c2CatId_UserStart);
}


// Returns true if the passed word is in our list of trivial words
tCIDLib::TBoolean TFacCQCMedia::bIsTrivialWord(const TString& strToCheck) const
{
    TString strWord(strToCheck);
    strWord.ToUpper();
    return m_colTrivWords.bHasElement(strWord);
}


//
//  Creates a sort title by removing any leading trivial word from the
//  title. If the sort title is the same as the original title, it
//  returns false and sets sort title to the original title. Else it
//  sets sort title to the adjusted title text and returns true.
//
tCIDLib::TBoolean
TFacCQCMedia::bMakeSortTitle(const  TString&    strOrgTitle
                            ,       TString&    strSortTitle) const
{
    //
    //  Get the first word out of the original title, by looking for
    //  the first space.
    //
    strSortTitle.Clear();
    tCIDLib::TCard4 c4Ofs;
    if (strOrgTitle.bFirstOccurrence(kCIDLib::chSpace, c4Ofs))
    {
        TString strFirstWord;
        strOrgTitle.CopyOutSubStr(strFirstWord, 0, c4Ofs);
        strFirstWord.ToUpper();

        // See if it's in our list of trivial words
        if (m_colTrivWords.bHasElement(strFirstWord))
        {
            //
            //  There is a leading trivial word. So search forward to skip
            //  any other spaces that might be there, and any punctuation.
            //  Then we'll copy the remainder into the sort title.
            //
            const tCIDLib::TCard4 c4Len = strOrgTitle.c4Length();
            while (c4Ofs < c4Len)
            {
                const tCIDLib::TCh chCur = strOrgTitle.chAt(c4Ofs);
                if (!TRawStr::bIsPunct(chCur) && !TRawStr::bIsSpace(chCur))
                    break;
                c4Ofs++;
            }

            // If we went to the end, then don't do anything, it was the only word
            if (c4Ofs < c4Len)
                strOrgTitle.CopyOutSubStr(strSortTitle, c4Ofs);
        }
    }

    // If the sort title is still empty, just use the original title
    if (strSortTitle.bIsEmpty())
    {
        strSortTitle = strOrgTitle;
        return kCIDLib::False;
    }
    return kCIDLib::True;
}



//
//  Parses out a standard changer based media location info value, which
//  is in the form "moniker.slot". Returns false is badly formatted, else
//  returns true and the broken out values.
//
tCIDLib::TBoolean
TFacCQCMedia::bParseChangerLoc( const   TString&            strLocInfo
                                ,       TString&            strMoniker
                                ,       tCIDLib::TCard4&    c4Slot) const
{
    // There should be two tokens, period separated
    TStringTokenizer stokLoc(&strLocInfo, L".");

    TString strSlot;
    if (!stokLoc.bGetNextToken(strMoniker) || !stokLoc.bGetNextToken(strSlot))
        return kCIDLib::False;

    // If there are still tokens, then it's bad
    if (stokLoc.bMoreTokens())
        return kCIDLib::False;

    // Try to convert the slot
    tCIDLib::TBoolean bOk;
    c4Slot = TRawStr::c4AsBinary(strSlot.pszBuffer(), bOk, tCIDLib::ERadices::Dec);
    return bOk;
}


//
//  Query the audio format info for a specific title or item cookie. If a
//  collection cookie is passed, the returned info will be for the containing
//  title set (i.e. the aggregated values for all contained items.)
//
tCIDLib::TBoolean
TFacCQCMedia::bQueryAudioFmt(       tCQCKit::TCQCSrvProxy&  orbcSrv
                            , const TString&                strMoniker
                            , const TString&                strCookie
                            ,       tCIDLib::TCard4&        c4BitDepth
                            ,       tCIDLib::TCard4&        c4BitRate
                            ,       tCIDLib::TCard4&        c4Channels
                            ,       tCIDLib::TCard4&        c4SampleRate) const
{
    // Make the call which should get us four space separate values
    TString strVals;
    const tCIDLib::TBoolean bRet = orbcSrv->bQueryTextVal
    (
        strMoniker
        , kCQCMedia::strQuery_QueryAudioFmt
        , strCookie
        , strVals
    );

    // If he reports he found the data, then decode it
    if (bRet)
    {
        TString strBD;
        TString strBR;
        TString strCh;
        TString strSR;

        TStringTokenizer stokRet(&strVals, L" ");
        if (!stokRet.bGetNextToken(strBD)
        ||  !stokRet.bGetNextToken(strBR)
        ||  !stokRet.bGetNextToken(strCh)
        ||  !stokRet.bGetNextToken(strSR)
        ||  !strBD.bToCard4(c4BitDepth)
        ||  !strBR.bToCard4(c4BitRate)
        ||  !strCh.bToCard4(c4Channels)
        ||  !strSR.bToCard4(c4SampleRate))
        {
            facCQCMedia().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kMedErrs::errcRepoQ_BadQueryRepl
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
                , TString(L"audio format")
                , strMoniker
            );
        }
    }
     else
    {
        c4BitDepth = 0;
        c4BitRate = 0;
        c4Channels = 0;
        c4SampleRate = 0;
    }
    return bRet;
}


//
//  Ask a renderer driver for the current playlist, if it has one.
//
tCIDLib::TBoolean
TFacCQCMedia::bQueryCurPLItem(          tCQCKit::TCQCSrvProxy&  orbcSrv
                                , const TString&                strMoniker
                                ,       tCIDLib::TCard4&        c4Index
                                ,       TCQCMediaPLItem&        mpliToFill) const
{
    tCIDLib::TCard4 c4Bytes = 0;
    THeapBuf mbufData;
    const tCIDLib::TBoolean bRet = orbcSrv->bQueryData
    (
        strMoniker
        , kCQCMedia::strQuery_CurPLItem
        , TString::strEmpty()
        , c4Bytes
        , mbufData
    );

    if (bRet && (c4Bytes != 0))
    {
        TBinMBufInStream strmReply(&mbufData, c4Bytes);
        strmReply >> c4Index;
        strmReply.CheckForFrameMarker(CID_FILE, CID_LINE);
        strmReply >> mpliToFill;
        strmReply.CheckForEndMarker(CID_FILE, CID_LINE);
    }
    return bRet;
}


//
//  This will do a query of current cover art from a renderer driver.
//
tCIDLib::TBoolean
TFacCQCMedia::bQueryCurRendArt( const   TString&            strRendMoniker
                                ,       tCIDLib::TCard4&    c4Bytes
                                ,       TMemBuf&            mbufTar
                                , const tCIDLib::TBoolean   bLarge) const
{
    // Pass the right size value
    const TString strSize(bLarge ? L"Lrg" : L"Sml");

    // Get a proxy for the renderer and ask it for the art
    try
    {
        tCQCKit::TCQCSrvProxy orbcSrv
        (
            facCQCKit().orbcCQCSrvAdminProxy(strRendMoniker)
        );

        //
        //  Ask the rendeer driver associated with this playlist item for the
        //  cover art.
        //
        THeapBuf mbufArt;
        const tCIDLib::TBoolean bRes = orbcSrv->bQueryData
        (
            strRendMoniker
            , kCQCMedia::strQuery_QueryArt
            , strSize
            , c4Bytes
            , mbufArt
        );

        if (!bRes || !c4Bytes)
        {
            c4Bytes = 0;
            return kCIDLib::False;
        }

        mbufTar.CopyIn(mbufArt, c4Bytes);
    }

    catch(...)
    {
        c4Bytes = 0;
        return kCIDLib::False;
    }

    // Probe the buffer and see it's a valid image
    return facCIDImgFact().eProbeImg(mbufTar, c4Bytes) != tCIDImage::EImgTypes::Unknown;
}


//
//  For most access of cover art, clients want to get it from the locally cached data
//  downloaded by the client service. This method simplifies that operation by hiding
//  the details.
//
//  We have a separate one that returns an image object, not just the raw data.
//  Having a separate one means we don't have to do an extra read of the data into
//  memory just to stream it into the image object.
//
//  We only deal with large/small cover art here. Poster art is not cached by the
//  client service. So we just get a boolean large parm, which weturn into the
//  appropriate repo art type.
//
//  There is another version further down that is for things like XMLGW clients who
//  are requesting art via the unique/persistent ids, not via cookies. It is the
//  eQueryLocalCachedArt() method.
//
tCIDLib::TBoolean TFacCQCMedia::
bQueryLocalCachedArt(const  TString&            strRepoMoniker
                    , const TString&            strCookie
                    , const tCIDLib::TBoolean   bLarge
                    ,       tCIDLib::TCard4&    c4Bytes
                    ,       TMemBuf&            mbufToFill
                    ,       TString&            strPerId)
{
    // Until proven otherwise, return zero bytes
    c4Bytes = 0;

    // See if we have data for this repo
    TFacCQCMedia::TMDBPtr cptrInfo;
    TString strSerialNum;
    tCIDLib::ELoadRes eRes = eGetMediaDB(strRepoMoniker, strSerialNum, cptrInfo);

    // If not, there's nothing to do
    if (eRes != tCIDLib::ELoadRes::NewData)
    {
        if ((eRes == tCIDLib::ELoadRes::NotFound) && bLogWarnings())
        {
            LogMsg
            (
                CID_FILE
                , CID_LINE
                , kMedErrs::errcMDBC_NoRepo
                , tCIDLib::ESeverities::Warn
                , tCIDLib::EErrClasses::CantDo
                , strRepoMoniker
            );
        }
        return kCIDLib::False;
    }

    const TMediaDB& mdbInfo = cptrInfo->mdbData();

    // Look up the media objects for this cookie, if not found give up
    tCQCMedia::EMediaTypes  eMType;
    const TMediaTitleSet*   pmtsImg;
    const TMediaCollect*    pmcolImg;
    const TMediaItem*       pmitemImg;
    tCIDLib::TBoolean bRes = mdbInfo.bFindByCookie
    (
        strCookie, eMType, pmtsImg, pmcolImg, pmitemImg
    );

    // If not found, give up
    if (!bRes)
        return kCIDLib::False;

    // Get the art id. If top level, check the title first
    tCIDLib::TCard2 c2ArtId = 0;
    if (pmcolImg)
    {
        // Take the collection's id
        c2ArtId = pmcolImg->c2ArtId();
    }
     else if (pmtsImg)
    {
        // Check the title set first
        c2ArtId = pmtsImg->c2ArtId();

        // If none, let's get the first collection and check it
        if (!c2ArtId)
        {
            if (pmtsImg->c4ColCount())
            {
                pmcolImg = &pmtsImg->mcolAt(mdbInfo, 0);
                c2ArtId = pmcolImg->c2ArtId();
            }
        }
    }

    // If we got no art id in all of that, given up
    if (!c2ArtId)
        return kCIDLib::False;

    // Look up this art. If not found, give up
    const TMediaImg* pmimgArt = mdbInfo.pmimgById(eMType, c2ArtId, kCIDLib::False);
    if (!pmimgArt)
        return kCIDLib::False;

    // If no path for the type we want, or the file isn't there, then give up
    tCQCMedia::ERArtTypes eArtType
    (
        bLarge ? tCQCMedia::ERArtTypes::LrgCover : tCQCMedia::ERArtTypes::SmlCover
    );
    const TString& strArtPath = pmimgArt->strArtPath(eArtType);
    if (strArtPath.bIsEmpty() || !TFileSys::bExists(strArtPath))
        return kCIDLib::False;

    // Read in the data
    try
    {
        TBinaryFile flArt(strArtPath);
        flArt.Open
        (
            tCIDLib::EAccessModes::Read
            , tCIDLib::ECreateActs::OpenIfExists
            , tCIDLib::EFilePerms::Default
            , tCIDLib::EFileFlags::SequentialScan
        );

        c4Bytes = tCIDLib::TCard4(flArt.c8CurSize());
        flArt.c4ReadBuffer
        (
            mbufToFill, c4Bytes, tCIDLib::EAllData::FailIfNotAll
        );
    }

    catch(TError& errToCatch)
    {
        if (bLogWarnings())
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            LogEventObj(errToCatch);

            LogMsg
            (
                CID_FILE
                , CID_LINE
                , kMedErrs::errcMDBC_LoadFailed
                , tCIDLib::ESeverities::Warn
                , tCIDLib::EErrClasses::CantDo
                , strArtPath
            );
        }

        c4Bytes = 0;
        return kCIDLib::False;
    }

    strPerId = pmimgArt->strPersistentId(eArtType);
    return kCIDLib::True;
}

tCIDLib::TBoolean TFacCQCMedia::
bQueryLocalCachedArt(const  TString&            strRepoMoniker
                    , const TString&            strCookie
                    , const tCIDLib::TBoolean   bLarge
                    ,       TBitmapImage&       imgToFill
                    ,       TString&            strPerId)
{
    // See if we have cached info for this repo
    TFacCQCMedia::TMDBPtr cptrInfo;
    TString strSerialNum;
    const tCIDLib::ELoadRes eRes = eGetMediaDB(strRepoMoniker, strSerialNum, cptrInfo);

    // If not, there's nothing to do
    if (eRes != tCIDLib::ELoadRes::NewData)
    {
        if ((eRes == tCIDLib::ELoadRes::NotFound) && bLogWarnings())
        {
            LogMsg
            (
                CID_FILE
                , CID_LINE
                , kMedErrs::errcMDBC_NoRepo
                , tCIDLib::ESeverities::Warn
                , tCIDLib::EErrClasses::CantDo
                , strRepoMoniker
            );
        }
        return kCIDLib::False;
    }

    const TMediaDB& mdbInfo = cptrInfo->mdbData();

    // Look up the media objects for this cookie, if not found give up
    tCQCMedia::EMediaTypes  eMType;
    const TMediaTitleSet*   pmtsImg;
    const TMediaCollect*    pmcolImg;
    const TMediaItem*       pmitemImg;
    tCIDLib::TBoolean bRes = mdbInfo.bFindByCookie
    (
        strCookie, eMType, pmtsImg, pmcolImg, pmitemImg
    );

    // If the cookie is bad or not at least a collection cookie then give up.
    if (!bRes || !pmcolImg)
        return kCIDLib::False;

    // Get the art id. If top level, check the title first
    tCIDLib::TCard2 c2ArtId = 0;
    if (pmcolImg)
    {
        // Take the collection's id
        c2ArtId = pmcolImg->c2ArtId();
    }
     else if (pmtsImg)
    {
        // Check the title set to see if it has its own art
        c2ArtId = pmtsImg->c2ArtId();

        // If none, let's get the first collection and check it
        if (!c2ArtId)
        {
            if (pmtsImg->c4ColCount())
            {
                pmcolImg = &pmtsImg->mcolAt(mdbInfo, 0);
                c2ArtId = pmcolImg->c2ArtId();
            }
        }
    }

    // If we got no art id in all of that, given up
    if (!c2ArtId)
        return kCIDLib::False;

    // Look up this art. If not found, give up
    const TMediaImg* pmimgArt = mdbInfo.pmimgById(eMType, c2ArtId, kCIDLib::False);
    if (!pmimgArt)
        return kCIDLib::False;

    // If not found or no path for the type we want, then give up
    tCQCMedia::ERArtTypes eArtType
    (
        bLarge ? tCQCMedia::ERArtTypes::LrgCover : tCQCMedia::ERArtTypes::SmlCover
    );
    const TString& strArtPath = pmimgArt->strArtPath(eArtType);

    if (strArtPath.bIsEmpty() || !TFileSys::bExists(strArtPath))
        return kCIDLib::False;

    //
    //  Read in the data. If it's the small art, we can just stream it directly in.
    //  Else, we need to decode it to a bitmap.
    //
    try
    {
        if (bLarge)
        {
            // It's a jpeg so load it to a bitmap
            facCIDImgFact().bLoadToBitmap(strArtPath, imgToFill, kCIDLib::True);
        }
         else
        {
            TBinFileInStream strmArt
            (
                strArtPath
                , tCIDLib::ECreateActs::OpenIfExists
                , tCIDLib::EFilePerms::Default
                , tCIDLib::EFileFlags::SequentialScan
            );
            strmArt >> imgToFill;
        }
    }

    catch(TError& errToCatch)
    {
        if (bLogWarnings())
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            LogEventObj(errToCatch);

            LogMsg
            (
                CID_FILE
                , CID_LINE
                , kMedErrs::errcMDBC_LoadFailed
                , tCIDLib::ESeverities::Warn
                , tCIDLib::EErrClasses::CantDo
                , strArtPath
            );
        }
        return kCIDLib::False;
    }

    strPerId = pmimgArt->strPersistentId(eArtType);
    return kCIDLib::True;
}


//
//  This is a helper method that will ask a media repository driver for
//  the cover art associated with a particular cookie. The caller indicates
//  large or small art, and provides a cookie (title, col, or item) and the
//  moniker of the repo to go to.
//
//  We have one that just returns a buffer of image data, and another that
//  calls that first and then expands to an image. We have versions of both
//  of those that take an existing CQC Server proxy and others that create
//  them locally and call the other version, as a convenience.
//
//
//  NOTE:
//
//  These are not so much used anymore because as of 4.4.902 we have
//  the client service that pre-caches media metadata and art locally and
//  most programs that need repo art get it from there (using the
//  bQueryLocalCachedArt() method) because it's vastly more efficient both
//  on the client side and in terms of load on the back end.
//
//  However sometimes they are still needed, and they are still used to
//  query poster art, which is not cached locally, because it's pretty large
//  and seldom needed.
//
tCIDLib::TBoolean
TFacCQCMedia::bQueryRepoDrvArt( const   TString&                strMoniker
                                , const TString&                strCookie
                                , const tCQCMedia::ERArtTypes   eType
                                ,       tCIDLib::TCard4&        c4Bytes
                                ,       TMemBuf&                mbufToFill) const
{
    tCIDLib::TBoolean bRet;
    try
    {
        // Create a proxy and call the other version
        tCQCKit::TCQCSrvProxy orbcSrv = facCQCKit().orbcCQCSrvAdminProxy
        (
            strMoniker
        );

        bRet = bQueryRepoDrvArt
        (
            orbcSrv, strMoniker, strCookie, eType, c4Bytes, mbufToFill
        );
    }

    catch(const TError& errToCatch)
    {
        if (bShouldLog(errToCatch))
            LogEventObj(errToCatch);
        bRet = kCIDLib::False;
    }
    return bRet;
}

tCIDLib::TBoolean
TFacCQCMedia::bQueryRepoDrvArt(         tCQCKit::TCQCSrvProxy&  orbcSrv
                                , const TString&                strMoniker
                                , const TString&                strCookie
                                , const tCQCMedia::ERArtTypes   eType
                                ,       tCIDLib::TCard4&        c4Bytes
                                ,       TMemBuf&                mbufToFill) const
{
    tCIDLib::TBoolean bRet = kCIDLib::False;
    c4Bytes = 0;
    try
    {
        // See what kind of cookie we got
        tCIDLib::TCard2         c2CatId;
        tCIDLib::TCard2         c2ColId;
        tCIDLib::TCard2         c2ItemId;
        tCIDLib::TCard2         c2TitleId;
        tCQCMedia::EMediaTypes  eMType;
        tCQCMedia::ECookieTypes eCookieType = eCheckCookie
        (
            strCookie, eMType, c2CatId, c2TitleId, c2ColId, c2ItemId
        );

        const TString* pstrCmd = 0;
        switch(eType)
        {
            case tCQCMedia::ERArtTypes::Poster :
                pstrCmd = &kCQCMedia::strQuery_QueryPosterArt;
                break;

            case tCQCMedia::ERArtTypes::LrgCover :
                if (eCookieType == tCQCMedia::ECookieTypes::Title)
                    pstrCmd = &kCQCMedia::strQuery_QueryTitleArt;
                else if (eCookieType == tCQCMedia::ECookieTypes::Collect)
                    pstrCmd = &kCQCMedia::strQuery_QueryArt;
                else if (eCookieType == tCQCMedia::ECookieTypes::Item)
                    pstrCmd = &kCQCMedia::strQuery_QueryItemArt;
                break;

            case tCQCMedia::ERArtTypes::SmlCover :
                if (eCookieType == tCQCMedia::ECookieTypes::Title)
                    pstrCmd = &kCQCMedia::strQuery_QueryTitleThumbArt;
                else if (eCookieType == tCQCMedia::ECookieTypes::Collect)
                    pstrCmd = &kCQCMedia::strQuery_QueryThumbArt;
                else if (eCookieType == tCQCMedia::ECookieTypes::Item)
                    pstrCmd = &kCQCMedia::strQuery_QueryItemThumbArt;
                break;

            default :
                break;
        };

        if (!pstrCmd)
        {
            // It's not a valid combination
            ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kMedErrs::errcProto_BadRepoArtQuery
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
            );

            // Won't happen but makes analzyer happy
            return kCIDLib::False;
        }

        // Ask the driver for the image data
        THeapBuf mbufData;
        if (orbcSrv->bQueryData(strMoniker, *pstrCmd, strCookie, c4Bytes, mbufData)
        &&  (c4Bytes != 0))
        {
            mbufToFill.CopyIn(mbufData, c4Bytes, 0);
            bRet = kCIDLib::True;
        }
    }

    catch(const TError& errToCatch)
    {
        if (bShouldLog(errToCatch))
            LogEventObj(errToCatch);
        bRet = kCIDLib::False;
    }
    return bRet;
}

tCIDLib::TBoolean
TFacCQCMedia::bQueryRepoDrvArt(         tCQCKit::TCQCSrvProxy&  orbcSrv
                                , const TString&                strMoniker
                                , const TString&                strCookie
                                , const tCQCMedia::ERArtTypes   eType
                                ,       TBitmapImage&           imgToFill) const
{
    tCIDLib::TBoolean bRet;

    try
    {
        // Call the version that gets the raw data. If that works..
        tCIDLib::TCard4 c4Bytes;
        THeapBuf        mbufImg;
        bRet = bQueryRepoDrvArt(orbcSrv, strMoniker, strCookie, eType, c4Bytes, mbufImg);

        if (bRet)
        {
            //
            //  Ask the image factory to load it. If he can't, just return
            //  false.
            //
            TCIDImage* pimgNew;
            bRet = facCIDImgFact().bDecodeImg
            (
                mbufImg, c4Bytes, pimgNew, kCIDLib::False
            );

            if (bRet)
            {
                // Make sure it gets cleaned up
                TJanitor<TCIDImage> janImg(pimgNew);

                // And convert to the target type via assignment
                imgToFill = *pimgNew;
            }
        }
    }

    catch(const TError& errToCatch)
    {
        if (bShouldLog(errToCatch))
            LogEventObj(errToCatch);
        bRet = kCIDLib::False;
    }
    return bRet;
}

tCIDLib::TBoolean
TFacCQCMedia::bQueryRepoDrvArt( const   TString&                strMoniker
                                , const TString&                strCookie
                                , const tCQCMedia::ERArtTypes   eType
                                ,       TBitmapImage&           imgToFill) const
{
    tCIDLib::TBoolean bRet;

    try
    {
        // Call the version that gets the raw data. If that works..
        tCIDLib::TCard4 c4Bytes;
        THeapBuf        mbufImg;
        bRet = bQueryRepoDrvArt(strMoniker, strCookie, eType, c4Bytes, mbufImg);

        if (bRet)
        {
            //
            //  Ask the image factory to load it. If he can't, just return
            //  false.
            ///
            TCIDImage* pimgNew;
            bRet = facCIDImgFact().bDecodeImg
            (
                mbufImg, c4Bytes, pimgNew, kCIDLib::False
            );
            if (bRet)
            {
                // Make sure it gets cleaned up
                TJanitor<TCIDImage> janImg(pimgNew);

                // And convert to the target type via assignment
                imgToFill = *pimgNew;
            }
        }
    }

    catch(const TError& errToCatch)
    {
        if (bShouldLog(errToCatch))
            LogEventObj(errToCatch);
        bRet = kCIDLib::False;
    }
    return bRet;
}


// Test whether the passed media type is in the passed media type flags
tCIDLib::TBoolean
TFacCQCMedia::bTestMediaFlags(  const   tCQCMedia::EMediaTypes  eType
                                , const tCQCMedia::EMTFlags     eFlags) const
{
    return
    (
        ((0x1UL << tCIDLib::c4EnumOrd(eType)) & tCIDLib::c4EnumOrd(eFlags)) != 0
    );
}


//
//  This method wil format out a renderer's playlist. This is used by
//  some interface widgets, and also by the XML GW to return playlist
//  info. This is the server side of c4QueryPlayList().
//
tCIDLib::TCard4
TFacCQCMedia::c4FormatPLItems(  const   TCQCMediaPLMgr& mplmSrc
                                ,       TMemBuf&        mbufToFill)
{
    const tCIDLib::TCard2 c2Count = tCIDLib::TCard2(mplmSrc.c4ItemCount());

    // Put out the count first, followed by a XOR'd count for sanity
    TBinMBufOutStream strmOut(&mbufToFill);
    strmOut << mplmSrc.c4ItemCount()
            << (mplmSrc.c4ItemCount() ^ kCIDLib::c4MaxCard);

    // Now each of the items spearated by a frame marker
    for (tCIDLib::TCard2 c2Index = 0; c2Index < c2Count; c2Index++)
    {
        // Separate them by a frame market for sanity checking
        strmOut << mplmSrc.mpliAt(c2Index)
                << tCIDLib::EStreamMarkers::Frame;
    }

    // End it with an end marker and flush it
    strmOut << tCIDLib::EStreamMarkers::EndObject << kCIDLib::FlushIt;
    return strmOut.c4CurSize();
}


//
//  Formats out the current playlist item from the passed playlist manager.
//  For now, it's basically just the playlist item flattened, but preceeded
//  by the playlist index. The caller should insure it doesn't change the
//  playlist while we are in here (since they may be using a separate thread
//  to do playback and it may update the list.
//
//  This MUST stay in synce with bQueryCurCurPLItem() below, which is the
//  client side of this exchange.
//
tCIDLib::TCard4
TFacCQCMedia::c4FormatCurPLItem(    const   TCQCMediaPLMgr& mplmSrc
                                    ,       TMemBuf&        mbufToFill)
{
    // If it's empty, return zero bytes
    if (mplmSrc.bIsEmpty())
        return 0;

    const TCQCMediaPLItem&  mpliCur = mplmSrc.mpliCurrent();
    const tCIDLib::TCard4   c4Index = mplmSrc.c4CurIndex();

    TBinMBufOutStream strmOut(&mbufToFill);
    strmOut << c4Index
            << tCIDLib::EStreamMarkers::Frame
            << mpliCur
            << tCIDLib::EStreamMarkers::EndObject
            << kCIDLib::FlushIt;

    return strmOut.c4CurSize();
}


//
//  This will query a renderer driver for it's playlist items. The driver has to
//  ask the playlist manager to format out its playlsit to a buffer and return it
//  to us.
//
tCIDLib::TCard4
TFacCQCMedia::c4QueryPLItems(       tCQCKit::TCQCSrvProxy&      orbcSrv
                            , const TString&                    strMoniker
                            ,       TVector<TCQCMediaPLItem>&   colPLItems)
{
    tCIDLib::TCard4 c4Size = 0;
    THeapBuf mbufData(8);

    // Ask the driver for the data
    orbcSrv->bQueryData
    (
        strMoniker
        , kCQCMedia::strQuery_PLItems
        , TString::strEmpty()
        , c4Size
        , mbufData
    );

    // Get the count and XOR'd count and make sure it hasn't gotten scrambled
    tCIDLib::TCard4 c4Count;
    tCIDLib::TCard4 c4XORCount;
    TBinMBufInStream strmReply(&mbufData, c4Size);
    strmReply >> c4Count >> c4XORCount;

    if (c4Count != (c4XORCount ^ kCIDLib::c4MaxCard))
    {
        facCQCMedia().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMedErrs::errcDrv_InvalidItemCnt
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
        );
    }

    // Read in the items and give them to the caller
    colPLItems.RemoveAll();
    TCQCMediaPLItem mpliCur;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        strmReply >> mpliCur;

        // Each one should be separated by a frame marker
        strmReply.CheckForFrameMarker(CID_FILE, CID_LINE);

        colPLItems.objAdd(mpliCur);
    }

    // And it should all end with an end marker
    strmReply.CheckForEndMarker(CID_FILE, CID_LINE);

    return c4Count;
}



//
//  A helper method that does a category query transation, so that the caller
//  doesn't have to deal with the details.
//
tCIDLib::TCard4
TFacCQCMedia::c4QueryCatList(       tCQCKit::TCQCSrvProxy&  orbcSrv
                            , const TString&                strMoniker
                            , const tCQCMedia::EMediaTypes  eType
                            , const tCIDLib::TBoolean       bNoEmpty
                            ,       tCQCMedia::TNameList&   colNames
                            ,       tCQCMedia::TNameList&   colCookies) const
{
    tCIDLib::TCard4 c4Size = 0;
    THeapBuf mbufData(32 * 1024);

    // Pack up the outgoing data
    {
        TBinMBufOutStream strmOut(&mbufData);
        strmOut << bNoEmpty
                << eType;
        strmOut.Flush();
        c4Size = strmOut.c4CurSize();
    }

    // Ask the driver for the data
    const tCIDLib::TBoolean bRes = orbcSrv->bQueryData2
    (
        strMoniker, kCQCMedia::strQuery_CatList, c4Size, mbufData
    );
    if (!bRes)
        return 0;

    // Get the info out
    tCIDLib::TCard4 c4Count;
    TBinMBufInStream strmReply(&mbufData, c4Size);
    strmReply >> c4Count
              >> colNames
              >> colCookies;
    return c4Count;
}



//
//  This one is a wrapper around a query to a standard media repository
//  driver, to query the raw media item objects that are part of a
//  collection.
//
tCIDLib::TCard4
TFacCQCMedia::c4QueryColItems(          tCQCKit::TCQCSrvProxy&  orbcSrv
                                , const TString&                strRepoMoniker
                                , const TString&                strColCookie
                                ,       TVector<TMediaItem>&    colItems)
{
    tCIDLib::TCard4 c4Size = 0;
    THeapBuf mbufData(32 * 1024);

    //
    //  Ask the driver for the data. There's none to send. The collection cookie
    //  is all that's needed and it's passed as the query data value.
    //
    const tCIDLib::TBoolean bRes = orbcSrv->bQueryData
    (
        strRepoMoniker
        , kCQCMedia::strQuery_ColItems
        , strColCookie
        , c4Size
        , mbufData
    );

    if (!bRes)
        return 0;

    // We should get a count and a XOR'd count for sanity checking
    tCIDLib::TCard4 c4Count, c4XORCount;
    TBinMBufInStream strmReply(&mbufData, c4Size);
    strmReply >> c4Count >> c4XORCount;

    if (c4Count != (c4XORCount ^ kCIDLib::c4MaxCard))
    {
        facCQCMedia().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMedErrs::errcDrv_InvalidItemCnt
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
        );
    }

    // OK, read in the items. They are separated by frame markers
    colItems.RemoveAll();
    TMediaItem mitemCur;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        strmReply >> mitemCur;
        strmReply.CheckForFrameMarker(CID_FILE, CID_LINE);

        colItems.objAdd(mitemCur);
    }

    // And there shoudl be an end marker
    strmReply.CheckForEndMarker(CID_FILE, CID_LINE);

    return c4Count;
}


//
//  Returns playlist item objects for up to X items from the indicated
//  genre. This is used by players to implement a random play from genre
//  type mode.
//
//  Note that the caller, if these are to be put into a media playlist,
//  needs to assign unique list item ids to these PL items, since that
//  cannot be done by the server. They will be zero as returned from
//  there server.
//
tCIDLib::TCard4 TFacCQCMedia
::c4QueryRandomCatItems(        tCQCKit::TCQCSrvProxy&      orbcSrv
                        , const TString&                    strMoniker
                        , const TString&                    strCatCookie
                        , const tCIDLib::TCard4             c4MaxItems
                        ,       TVector<TCQCMediaPLItem>&   colToFill) const
{
    // Insure that the cat cookie is for a music category
    tCIDLib::TCard2 c2CatId;
    if (eParseCatCookie(strCatCookie, c2CatId) != tCQCMedia::EMediaTypes::Music)
    {
        facCQCMedia().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMedErrs::errcDB_RandCatPlayType
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::TypeMatch
        );
    }

    // Pack up the outgoing data
    tCIDLib::TCard4 c4Size = 0;
    THeapBuf mbufData(32 * 1024);
    {
        TBinMBufOutStream strmOut(&mbufData);
        strmOut << strCatCookie
                << c4MaxItems;
        strmOut.Flush();
        c4Size = strmOut.c4CurSize();
    }

    // Ask the driver for the data
    orbcSrv->bQueryData2
    (
        strMoniker, kCQCMedia::strQuery_RandomGenreItems, c4Size, mbufData
    );

    // Get the info out
    tCIDLib::TCard4 c4Count;
    TBinMBufInStream strmReply(&mbufData, c4Size);
    strmReply >> c4Count
              >> colToFill;
    return c4Count;
}



//
//  These are used to access some cover art images that we let the user
//  define by putting them into the image repository.
//
tCIDLib::TCard4
TFacCQCMedia::c4QuerySpecialImg(        TMemBuf&                mbufToFill
                                , const tCQCMedia::ESpcImages   eImage
                                , const TCQCSecToken&           sectUser) const
{
    tCIDLib::TCard4 c4GotBytes = 0;
    try
    {
        TDataSrvClient dsclQuery;

        // Figure out which image path we want
        TString strPath;
        switch(eImage)
        {
            case tCQCMedia::ESpcImages::PlayList :
                strPath = L"\\User\\CQC\\Media\\PListArt";
                break;

            case tCQCMedia::ESpcImages::Missing :
                strPath = L"\\User\\CQC\\Media\\MissingArt";
                break;

            default :
                return 0;
        };

        // Read the image using the memory buffer based method
        tCIDLib::TCard4         c4SerNum = 0;
        tCIDLib::TEncodedTime   enctLast;
        tCIDLib::TKVPFList      colMeta;
        dsclQuery.bReadImage
        (
            strPath, c4SerNum, enctLast, mbufToFill, c4GotBytes, colMeta, sectUser
        );
    }

    catch(TError& errToCach)
    {
        errToCach.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCach);
    }

    catch(...)
    {
        c4GotBytes = 0;
    }
    return c4GotBytes;
}


//
//  Creates persistent ids by hashing data buffers. It will has the passed buffer
//  and format out the hash to text format and return it. This is used in a number
//  of places to create persistent ids, mostly for images, but potentially for other
//  files.
//
//  Generally we just use the actual buffer data, but we can also add an id and/or
//  time stamp to the data if the caller wants to create an id that is specific to
//  some data object's id or time specific. If either is is zero we ignore it.
//
//  There's another version that takes an input stream for the sourc data. In some
//  cases they have the data in a stream and copying it out to a buffer just to create
//  the persistent id is wasteful.
//
tCIDLib::TVoid
TFacCQCMedia::CreatePersistentId(const  TMemBuf&                mbufData
                                , const tCIDLib::TCard4         c4DataSz
                                ,       TString&                strToFill
                                , const tCIDLib::TCard2         c2Id
                                , const tCIDLib::TEncodedTime   enctStamp)
{
    // Create a hasher and and hash and call the other version
    TSHA1Hasher mdigToUse;
    TSHA1Hash   mhashToUse;

    // Hash the buffer and the size itself
    mdigToUse.StartNew();
    mdigToUse.DigestBuf(mbufData, c4DataSz);
    mdigToUse.DigestRaw((tCIDLib::TCard1*)&c4DataSz, sizeof(c4DataSz));

    // And the optional bits
    if (c2Id)
        mdigToUse.DigestRaw((tCIDLib::TCard1*)&c2Id, sizeof(c2Id));
    if (enctStamp)
        mdigToUse.DigestRaw((tCIDLib::TCard1*)&enctStamp, sizeof(enctStamp));

    // Finish it off and format it to a string
    mdigToUse.Complete(mhashToUse);
    mhashToUse.FormatToStr(strToFill);

    // Replace the spaces with a dash
    strToFill.bReplaceChar(kCIDLib::chSpace, kCIDLib::chHyphenMinus);
}

tCIDLib::TVoid
TFacCQCMedia::CreatePersistentId(       TBinInStream&           strmSrc
                                , const tCIDLib::TCard4         c4DataSz
                                ,       TString&                strToFill
                                , const tCIDLib::TCard2         c2Id
                                , const tCIDLib::TEncodedTime   enctStamp)
{
    // Create a hasher and and hash and call the other version
    TSHA1Hasher mdigToUse;
    TSHA1Hash   mhashToUse;

    // Hash the stream and the size itself
    mdigToUse.StartNew();
    mdigToUse.DigestSrc(strmSrc, c4DataSz);
    mdigToUse.DigestRaw((tCIDLib::TCard1*)&c4DataSz, sizeof(c4DataSz));

    // And the optional bits
    if (c2Id)
        mdigToUse.DigestRaw((tCIDLib::TCard1*)&c2Id, sizeof(c2Id));
    if (enctStamp)
        mdigToUse.DigestRaw((tCIDLib::TCard1*)&enctStamp, sizeof(enctStamp));

    // Finish it off and format it to a string
    mdigToUse.Complete(mhashToUse);
    mhashToUse.FormatToStr(strToFill);

    // Replace the spaces with a dash
    strToFill.bReplaceChar(kCIDLib::chSpace, kCIDLib::chHyphenMinus);
}



//
//  If the caller doesn't know what type of cookie he has, and wants to get
//  it parsed and be told the type and cookie info, this method will do that
//  for him.
//
tCQCMedia::ECookieTypes
TFacCQCMedia::eCheckCookie( const   TString&                strCookie
                            ,       tCQCMedia::EMediaTypes& eType
                            ,       tCIDLib::TCard2&        c2CatId
                            ,       tCIDLib::TCard2&        c2TitleId
                            ,       tCIDLib::TCard2&        c2ColId
                            ,       tCIDLib::TCard2&        c2ItemId) const
{
    //
    //  The absolute longest a cookie can be is 4 hex chars per possible id,
    //  plus possible ids minus one commas. So we set up a local char buffer
    //  of that size and copy into it.
    //
    const tCIDLib::TCard4 c4MaxLen = (kCQCMedia::c4MaxCookieTokens * 4)
                                     + (kCQCMedia::c4MaxCookieTokens - 1);
    const tCIDLib::TCard4 c4Len = strCookie.c4Length();

    // Make sure it's not longer than the max legal
    if (c4Len > c4MaxLen)
    {
        ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMedErrs::errcProto_BadCookie
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TString(L"[unknown]")
        );
    }

    // Looks ok, so create a local buffer and copy into it
    tCIDLib::TCh achBuf[c4MaxLen + 1];
    TRawStr::CopyStr(achBuf, strCookie.pszBuffer(), c4Len);

    //
    //  For up to the maximum+1 number of tokens, find that string and convert
    //  to a TCard2 value. By going one extra, we catch a cookie with too
    //  many tokens.
    //
    tCIDLib::TCard2     ac2Ids[kCQCMedia::c4MaxCookieTokens + 1];
    TRawMem::SetMemBuf
    (
        ac2Ids, tCIDLib::TCard2(0), kCQCMedia::c4MaxCookieTokens + 1
    );
    tCIDLib::TBoolean   bOk;
    tCIDLib::TCard4     c4CurId;
    tCIDLib::TCard4     c4Count = 0;
    tCIDLib::TCh*       pszCur = achBuf;
    tCIDLib::TCh*       pszEnd = achBuf;
    while (c4Count < kCQCMedia::c4MaxCookieTokens + 1)
    {
        // Run forward to a comma or null
        tCIDLib::TCh chCur = *pszEnd;
        while((chCur != kCIDLib::chComma) && (chCur != kCIDLib::chNull))
        {
            pszEnd++;
            chCur = *pszEnd;
        }

        // Put a null in this position
        *pszEnd = kCIDLib::chNull;

        // Convert this one and store the id
        c4CurId = TRawStr::c4AsBinary(pszCur, bOk, tCIDLib::ERadices::Hex);

        // If bad, or too big, it's a bad cookie
        if (!bOk || (c4CurId > kCIDLib::c2MaxCard))
        {
            ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kMedErrs::errcProto_BadCookie
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
                , TString(L"[unknown]")
            );
        }

        // Looks ok, so store this id
        ac2Ids[c4Count++] = tCIDLib::TCard2(c4CurId);

        // If we hit a null, then we are done
        if (chCur == kCIDLib::chNull)
            break;

        // Else, move forward for the next round
        pszEnd++;
        pszCur = pszEnd;
    }

    //
    //  If we have less than 2 values, it can be valid. Of if the 0th one is
    //  larger than the maximum legal media type, it's bad. Or, if we got
    //  more than the legal number it's bad.
    //
    if ((c4Count < 2)
    ||  (c4Count > kCQCMedia::c4MaxCookieTokens)
    ||  (ac2Ids[0] > tCIDLib::c4EnumOrd(tCQCMedia::EMediaTypes::Max)))
    {
        ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMedErrs::errcProto_BadCookie
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TString(L"[unknown]")
        );
    }

    // Store the type
    eType = tCQCMedia::EMediaTypes(ac2Ids[0]);

    //
    //  Looks ok, so store them. Just store them all. If they never got
    //  set they'll be bogus values but the caller won't look at the ones
    //  he doesn't expect.
    //
    c2CatId   = ac2Ids[1];
    c2TitleId = ac2Ids[2];
    c2ColId  = ac2Ids[3];
    c2ItemId = ac2Ids[4];

    // Based on the actual number of ids we got, that tells us the type
    tCQCMedia::ECookieTypes eRet;
    switch(c4Count)
    {
        case 2 :
            eRet = tCQCMedia::ECookieTypes::Cat;
            break;

        case 3 :
            eRet = tCQCMedia::ECookieTypes::Title;
            break;

        case 4 :
            eRet = tCQCMedia::ECookieTypes::Collect;
            break;

        case 5 :
            eRet = tCQCMedia::ECookieTypes::Item;
            break;

        default :
            facCQCMedia().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kMedErrs::errcProto_BadCookie2
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
            );

            // Won't happen but makes analyzer happy
            return tCQCMedia::ECookieTypes::Count;
    };
    return eRet;
}


//
//  This method is used by client code to get access to downloaded media DBs. They
//  give us the repo moniker they are interested in. They can also provide the
//  serial number if they already have gotten a version and just want to see if
//  there's something newer. If not, set the serial number to empty. If the serial
//  is the same, then the pointer is left alone. We return a standard load result
//  to indicate if we returned nothing, returned new data, or if they caller is
//  already up to date.
//
tCIDLib::ELoadRes
TFacCQCMedia::eGetMediaDB(  const   TString&    strRepo
                            , const TString&    strCurSerialNum
                            ,       TMDBPtr&    cptrToFill)
{
    // See if we have it first. If not, just return no data found
    if (!m_colMediaDBCache.bFindByKey(strRepo, cptrToFill))
        return tCIDLib::ELoadRes::NotFound;

    // Set the return based on whether the serial number has changed
    tCIDLib::ELoadRes eRet;
    if (cptrToFill->strDBSerialNum() == strCurSerialNum)
        eRet = tCIDLib::ELoadRes::NoNewData;
    else
        eRet = tCIDLib::ELoadRes::NewData;

    return eRet;
}


//
//  These methods parse category and title cookies. These are always the
//  same format as of version 2.0.
//
//      Category cookies: mt,id
//      Title cookies: mt,cid,tid
//      Col cookies: mt,cid,tid,lid
//      Item cookies: mt,cid,tid,lid,iid
//
//  mt  - The media type as raw integer number
//  cid - The 16 bit category id
//  tid - The 16 bit title set id
//  lid - The 16 bit collection id
//  iid - The 16 bit item id
//
//  They are all formatted as hex to minimize size. These all call a common
//  helper method that does the work, efficiently since these are parsed
//  very often.
//
//  Note that the category and title set ids refer to ids within the name
//  database tables, since these are both top level entities (well, title
//  sets are in categories but that's not a nesting relationship really.)
//  Collection and item ids, OTOH, refer to the position within within the
//  containing title set or collection. So an item id of 1 means the 0th
//  item within the collection, which is used to look up the actual id of
//  the item within the overall item list.
//
tCQCMedia::EMediaTypes
TFacCQCMedia::eParseCatCookie(  const   TString&            strCookie
                                ,       tCIDLib::TCard2&    c2CatId) const
{
    tCIDLib::TCard2 c2Dummy1;
    tCIDLib::TCard2 c2Dummy2;
    tCIDLib::TCard2 c2Dummy3;
    return eParseCookie
    (
        strCookie, tCQCMedia::ECookieTypes::Cat, c2CatId, c2Dummy1, c2Dummy2, c2Dummy3
    );
}

tCQCMedia::EMediaTypes
TFacCQCMedia::eParseColCookie(  const   TString&            strCookie
                                ,       tCIDLib::TCard2&    c2CatId
                                ,       tCIDLib::TCard2&    c2TitleId
                                ,       tCIDLib::TCard2&    c2ColId) const
{
    tCIDLib::TCard2 c2Dummy1;
    return eParseCookie
    (
        strCookie, tCQCMedia::ECookieTypes::Collect, c2CatId, c2TitleId, c2ColId, c2Dummy1
    );
}

tCQCMedia::EMediaTypes
TFacCQCMedia::eParseItemCookie( const   TString&            strCookie
                                ,       tCIDLib::TCard2&    c2CatId
                                ,       tCIDLib::TCard2&    c2TitleId
                                ,       tCIDLib::TCard2&    c2ColId
                                ,       tCIDLib::TCard2&    c2ItemId) const
{
    return eParseCookie
    (
        strCookie, tCQCMedia::ECookieTypes::Item, c2CatId, c2TitleId, c2ColId, c2ItemId
    );
}

tCQCMedia::EMediaTypes
TFacCQCMedia::eParseTitleCookie(const   TString&            strCookie
                                ,       tCIDLib::TCard2&    c2CatId
                                ,       tCIDLib::TCard2&    c2TitleId) const
{
    tCIDLib::TCard2 c2Dummy1;
    tCIDLib::TCard2 c2Dummy2;
    return eParseCookie
    (
        strCookie, tCQCMedia::ECookieTypes::Title, c2CatId, c2TitleId, c2Dummy1, c2Dummy2
    );
}


//
//  These are helper methods that allow the caller to call us and we do all
//  the work internally to  build the request, send it, get the results, and
//  parse out the returned data.
//
tCQCMedia::EMediaTypes
TFacCQCMedia::eQueryColDetails( const   TString&            strRepoMoniker
                                , const TString&            strColCookie
                                ,       TMediaCollect&      mcolToFill
                                ,       TString&            strTitleName
                                ,       tCIDLib::TCard4&    c4ColCount) const
{
    //
    //  We need to get an admin proxy for the CQCServer that is hosting
    //  the repository driver.
    //
    tCQCKit::TCQCSrvProxy orbcSrv = facCQCKit().orbcCQCSrvAdminProxy(strRepoMoniker);

    // And call the other version
    return eQueryColDetails
    (
        orbcSrv
        , strRepoMoniker
        , strColCookie
        , mcolToFill
        , strTitleName
        , c4ColCount
    );
}

tCQCMedia::EMediaTypes
TFacCQCMedia::eQueryColDetails(         tCQCKit::TCQCSrvProxy&  orbcSrv
                                , const TString&                strMoniker
                                , const TString&                strColCookie
                                ,       TMediaCollect&          mcolToFill
                                ,       TString&                strTitleName
                                ,       tCIDLib::TCard4&        c4ColCount) const
{
    tCIDLib::TCard4 c4Size = 0;
    THeapBuf mbufData(8 * 1024UL);

    // Ask the server for the info
    orbcSrv->bQueryData
    (
        strMoniker, kCQCMedia::strQuery_ColDetails, strColCookie, c4Size, mbufData
    );

    // Stream the data out
    TBinMBufInStream strmReply(&mbufData, c4Size);
    strmReply   >> mcolToFill
                >> strTitleName
                >> c4ColCount;
    return mcolToFill.eType();
}


// Query the location type for the indicated collection
tCQCMedia::ELocTypes
TFacCQCMedia::eQueryColLoc(         tCQCKit::TCQCSrvProxy&  orbcSrv
                            , const TString&                strMoniker
                            , const TString&                strColCookie
                            ,       TString&                strColLoc) const
{
    tCIDLib::TCard4 c4Size = 0;
    THeapBuf mbufData(1024UL);

    // Ask the server for the info
    orbcSrv->bQueryData
    (
        strMoniker, kCQCMedia::strQuery_ColLoc, strColCookie, c4Size, mbufData
    );

    // Get the info back out
    tCQCMedia::ELocTypes eLocType;
    TBinMBufInStream strmReply(&mbufData, c4Size);
    strmReply   >> strColLoc
                >> eLocType;
    return eLocType;
}


//
//  A helper method that does a default category info transaction, so that
//  the caller doesn'thave tod eal with the details.
//
tCQCMedia::EMediaTypes
TFacCQCMedia::eQueryDefCatInfo(         tCQCKit::TCQCSrvProxy&  orbcSrv
                                , const TString&                strMoniker
                                , const tCQCMedia::EMediaTypes  ePrefType
                                ,       TString&                strCatCookie
                                ,       TString&                strCatName) const
{
    tCIDLib::TCard4 c4Size;
    THeapBuf mbufData(32 * 1024);

    // Pack up the outgoing data
    {
        TBinMBufOutStream strmOut(&mbufData);
        strmOut << ePrefType;
        strmOut.Flush();
        c4Size = strmOut.c4CurSize();
    }

    // Ask the driver for the data
    orbcSrv->bQueryData2
    (
        strMoniker, kCQCMedia::strQuery_DefCatInfo, c4Size, mbufData
    );

    // Get the info out
    tCQCMedia::EMediaTypes eMType;
    TBinMBufInStream strmReply(&mbufData, c4Size);
    strmReply   >> strCatCookie
                >> strCatName
                >> eMType;

    return eMType;
}


//
//  This is similar to the bQueryLocalCachedArt() methods above, but this one
//  is for things like the XML GW, whose clients don't deal in cookies. They download
//  the raw media databases and just use the UID/perids to get the art info.
//
//  THey give us the UID which we use to look it up, and then the persistent id
//  for the particular art size, empty if this is the first query for it. We tell
//  them if didn't find it, found it but they are up to date, or we are giveing
//  back new data (and a new persistent id, for that size.)
//
tCIDLib::ELoadRes
TFacCQCMedia::eQueryLocalCachedArt( const   TString&                strRepoMoniker
                                    , const TString&                strUID
                                    , const TString&                strCurPerId
                                    , const tCQCMedia::EMediaTypes  eMType
                                    , const tCIDLib::TBoolean       bLarge
                                    ,       tCIDLib::TCard4&        c4Bytes
                                    ,       TMemBuf&                mbufToFill
                                    ,       TString&                strNewPerId)
{
    // Until proven otherwise, return zero bytes
    c4Bytes = 0;

    // See if we have data for this repo
    TFacCQCMedia::TMDBPtr cptrInfo;
    TString strSerialNum;
    tCIDLib::ELoadRes eRes = eGetMediaDB(strRepoMoniker, strSerialNum, cptrInfo);

    // If not, there's nothing to do
    if (eRes != tCIDLib::ELoadRes::NewData)
    {
        if ((eRes == tCIDLib::ELoadRes::NotFound) && bLogWarnings())
        {
            LogMsg
            (
                CID_FILE
                , CID_LINE
                , kMedErrs::errcMDBC_NoRepo
                , tCIDLib::ESeverities::Warn
                , tCIDLib::EErrClasses::CantDo
                , strRepoMoniker
            );
        }
        return tCIDLib::ELoadRes::NotFound;
    }

    // We got it, so let's look up the image by its UID
    const TMediaDB& mdbInfo = cptrInfo->mdbData();
    const TMediaImg* pmimgLoad = mdbInfo.pmimgByUniqueId(eMType, strUID, kCIDLib::False);

    // If not found, give up
    if (!pmimgLoad)
        return tCIDLib::ELoadRes::NotFound;

    // Set u the art type for the requested size
    const tCQCMedia::ERArtTypes eArtType
    (
        bLarge ? tCQCMedia::ERArtTypes::LrgCover : tCQCMedia::ERArtTypes::SmlCover
    );

    //
    //  If no persistent id or path for the image they want, then it doesn't exist,
    //  so give up now.
    //
    const TString& strArtPath = pmimgLoad->strArtPath(eArtType);
    const TString& strPerId = pmimgLoad->strPersistentId(eArtType);
    if (strPerId.bIsEmpty() || strArtPath.bIsEmpty())
        return tCIDLib::ELoadRes::NotFound;

    //
    //  If they provided an incoming persistent id, and it's the same as the current
    //  one, then just tell them that they are up to date.
    //
    if (!strCurPerId.bIsEmpty() && (strCurPerId == strPerId))
        return tCIDLib::ELoadRes::NoNewData;

    // Read in the data
    try
    {
        TBinaryFile flArt(strArtPath);
        flArt.Open
        (
            tCIDLib::EAccessModes::Read
            , tCIDLib::ECreateActs::OpenIfExists
            , tCIDLib::EFilePerms::Default
            , tCIDLib::EFileFlags::SequentialScan
        );

        c4Bytes = tCIDLib::TCard4(flArt.c8CurSize());
        flArt.c4ReadBuffer
        (
            mbufToFill, c4Bytes, tCIDLib::EAllData::FailIfNotAll
        );
    }

    catch(TError& errToCatch)
    {
        if (bLogWarnings())
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            LogEventObj(errToCatch);

            LogMsg
            (
                CID_FILE
                , CID_LINE
                , kMedErrs::errcMDBC_LoadFailed
                , tCIDLib::ESeverities::Warn
                , tCIDLib::EErrClasses::CantDo
                , strArtPath
            );
        }

        // Indicate we failed and make sure bytes are cleared
        c4Bytes = 0;
        return tCIDLib::ELoadRes::NotFound;
    }

    // Give back the new persistent id and return new data result
    strNewPerId = strPerId;
    return tCIDLib::ELoadRes::NewData;
}



//
//  Query the overall details for a title. It returns the count of collections,
//  and for each one the title, collection cookie, and collection level artist.
//
tCQCMedia::EMediaTypes
TFacCQCMedia::eQueryTitleDetails(const  TString&                strRepoMoniker
                                , const TString&                strTitleCookie
                                ,       TString&                strName
                                ,       TString&                strArtist
                                ,       TString&                strMediaFmt
                                ,       tCIDLib::TCard4&        c4ColCnt
                                ,       tCIDLib::TCard4&        c4Year
                                ,       tCQCMedia::TNameList&   colNames
                                ,       tCQCMedia::TNameList&   colCookies
                                ,       tCQCMedia::TNameList&   colArtists) const
{
    //
    //  We need to get an admin proxy for the CQCServer that is hosting
    //  the repository driver.
    //
    tCQCKit::TCQCSrvProxy orbcSrv = facCQCKit().orbcCQCSrvAdminProxy(strRepoMoniker);
    return eQueryTitleDetails
    (
        orbcSrv
        , strRepoMoniker
        , strTitleCookie
        , strName
        , strArtist
        , strMediaFmt
        , c4ColCnt
        , c4Year
        , colNames
        , colCookies
        , colArtists
    );
}


tCQCMedia::EMediaTypes
TFacCQCMedia::eQueryTitleDetails(       tCQCKit::TCQCSrvProxy&  orbcSrv
                                , const TString&                strMoniker
                                , const TString&                strTitleCookie
                                ,       TString&                strName
                                ,       TString&                strArtist
                                ,       TString&                strMediaFmt
                                ,       tCIDLib::TCard4&        c4ColCnt
                                ,       tCIDLib::TCard4&        c4Year
                                ,       tCQCMedia::TNameList&   colNames
                                ,       tCQCMedia::TNameList&   colCookies
                                ,       tCQCMedia::TNameList&   colArtists) const
{
    tCIDLib::TCard4 c4Size = 0;
    THeapBuf mbufData(16 * 1024UL);

    // Ask the server for the info
    orbcSrv->bQueryData
    (
        strMoniker
        , kCQCMedia::strQuery_TitleDetails
        , strTitleCookie
        , c4Size
        , mbufData
    );

    // Get the info back out
    tCQCMedia::EMediaTypes eType;
    TBinMBufInStream strmReply(&mbufData, c4Size);
    strmReply   >> strName
                >> strArtist
                >> c4Year
                >> strMediaFmt
                >> c4ColCnt
                >> colNames
                >> colCookies
                >> colArtists
                >> eType;

    return eType;
}


//
//  In action commands used by some IV widets, they accept media types
//  and by bad historical decision that text is not the same as either
//  our loadable text for the media types or the trailing part of the
//  enum values. So we provdie a special translation method to keep it
//  all here in one place.
//
//
//  In a couple other cases what is presented to the user is the loadable
//  text (though fixed in the IDL in these cases), but we still want to
//  provide a helper translator because the IDL doesn't generate one to
//  translate back from the loadable text.
//
tCQCMedia::ECompTypes
TFacCQCMedia::eXlatCmdCompType(const TString& strToXlat) const
{
    tCQCMedia::ECompTypes eType = tCQCMedia::ECompTypes::Min;
    for (; eType < tCQCMedia::ECompTypes::Count; eType++)
    {
        if (strToXlat == tCQCMedia::strXlatECompTypes(eType))
            break;
    }
    return eType;
}

tCQCMedia::EMediaTypes
TFacCQCMedia::eXlatCmdMediaType(const TString& strToXlat) const
{
    tCQCMedia::EMediaTypes eMType = tCQCMedia::EMediaTypes::Min;
    for (; eMType < tCQCMedia::EMediaTypes::Count; eMType++)
    {
        if (strToXlat == tCQCMedia::strXlatEMediaTypes(eMType))
            break;
    }
    return eMType;
}

tCQCMedia::ESortOrders
TFacCQCMedia::eXlatCmdSortOrder(const TString& strToXlat) const
{
    tCQCMedia::ESortOrders eOrder = tCQCMedia::ESortOrders::Min;
    for (; eOrder < tCQCMedia::ESortOrders::Count; eOrder++)
    {
        if (strToXlat == tCQCMedia::strXlatESortOrders(eOrder))
            break;
    }
    return eOrder;
}


// Formats a standard category cookie using the passed info
tCIDLib::TVoid
TFacCQCMedia::FormatCatCookie(  const   tCQCMedia::EMediaTypes  eType
                                , const tCIDLib::TCard2         c2CatId
                                ,       TString&                strCookie) const
{
    strCookie.SetFormatted(tCIDLib::TInt4(eType));
    strCookie.Append(kCIDLib::chComma);
    strCookie.AppendFormatted(c2CatId, tCIDLib::ERadices::Hex);
}


//
//  Formats a standard collection cookie
//
tCIDLib::TVoid
TFacCQCMedia::FormatColCookie(  const   tCQCMedia::EMediaTypes  eType
                                , const tCIDLib::TCard2         c2CatId
                                , const tCIDLib::TCard2         c2TitleId
                                , const tCIDLib::TCard2         c2ColId
                                ,       TString&                strCookie) const
{
    strCookie.SetFormatted(tCIDLib::TInt4(eType));
    strCookie.Append(kCIDLib::chComma);
    strCookie.AppendFormatted(c2CatId, tCIDLib::ERadices::Hex);
    strCookie.Append(kCIDLib::chComma);
    strCookie.AppendFormatted(c2TitleId, tCIDLib::ERadices::Hex);
    strCookie.Append(kCIDLib::chComma);
    strCookie.AppendFormatted(c2ColId, tCIDLib::ERadices::Hex);
}



//
//  Formats a duration in seconds into a form like '01h, 02m, 03h'. The hours
//  are only done if we have a non-zero our amount. The minutes and seconds
//  are always done.
//
tCIDLib::TVoid
TFacCQCMedia::FormatDuration(const  tCIDLib::TCard4 c4Seconds
                            ,       TString&        strToFill) const
{
    strToFill.Clear();

    // Get a copy of the seconds we can modify
    tCIDLib::TCard4 c4Secs = c4Seconds;
    const tCIDLib::TCard4 c4Hours = (c4Secs / 3600);
    if (c4Hours)
        c4Secs -= (c4Hours * 3600);
    const tCIDLib::TCard4 c4Mins = (c4Secs / 60);
    c4Secs -= (c4Mins * 60);

    if (c4Hours)
    {
        //
        //  We only do the hours if present, since they would normally never
        //  be. This is a single item time, i.e. track.
        //
        if (c4Hours < 10)
            strToFill.Append(kCIDLib::chDigit0);
        strToFill.AppendFormatted(c4Hours);
        strToFill.Append(L"h, ");
    }

    if (c4Mins < 10)
        strToFill.Append(kCIDLib::chDigit0);
    strToFill.AppendFormatted(c4Mins);
        strToFill.Append(L"m, ");

    if (c4Secs < 10)
        strToFill.Append(kCIDLib::chDigit0);
    strToFill.AppendFormatted(c4Secs);
        strToFill.Append(L"s");
}


//
//  Formats a standard item cookie
//
tCIDLib::TVoid
TFacCQCMedia::FormatItemCookie( const   tCQCMedia::EMediaTypes  eType
                                , const tCIDLib::TCard2         c2CatId
                                , const tCIDLib::TCard2         c2TitleId
                                , const tCIDLib::TCard2         c2ColId
                                , const tCIDLib::TCard2         c2ItemId
                                ,       TString&                strCookie) const
{
    strCookie.SetFormatted(tCIDLib::TInt4(eType));
    strCookie.Append(kCIDLib::chComma);
    strCookie.AppendFormatted(c2CatId, tCIDLib::ERadices::Hex);
    strCookie.Append(kCIDLib::chComma);
    strCookie.AppendFormatted(c2TitleId, tCIDLib::ERadices::Hex);
    strCookie.Append(kCIDLib::chComma);
    strCookie.AppendFormatted(c2ColId, tCIDLib::ERadices::Hex);
    strCookie.Append(kCIDLib::chComma);
    strCookie.AppendFormatted(c2ItemId, tCIDLib::ERadices::Hex);
}


//
//  Formats a standard title cookie
//
tCIDLib::TVoid
TFacCQCMedia::FormatTitleCookie(const   tCQCMedia::EMediaTypes  eType
                                , const tCIDLib::TCard2         c2CatId
                                , const tCIDLib::TCard2         c2TitleId
                                ,       TString&                strCookie) const
{
    strCookie.SetFormatted(tCIDLib::TInt4(eType));
    strCookie.Append(kCIDLib::chComma);
    strCookie.AppendFormatted(c2CatId, tCIDLib::ERadices::Hex);
    strCookie.Append(kCIDLib::chComma);
    strCookie.AppendFormatted(c2TitleId, tCIDLib::ERadices::Hex);
}


//
//  Client apps have to call this to get our media related CML classes
//  registered with the macro engine facility, so that these classes are
//  available to any macros they might load. We just allocate one of our
//  standard runtime class loader objects and install it on the macro
//  engine.
//
//  Unfortunately there is no way to have this fault in, so containing apps
//  must call this during init, before they start up other threads.
//
tCIDLib::TVoid TFacCQCMedia::InitCMLRuntime()
{
    facCIDMacroEng().AddClassLoader(new TCQCMediaRTClassLoader);
}


//
//  Given a repo moniker, return the path that the client service will leave
//  downloaded media data for the indicated repo.
//
tCIDLib::TVoid
TFacCQCMedia::QueryRepoCachePath(const  TString&    strRepoMoniker
                                ,       TString&    strToFill)
{
    TPathStr pathTmp = facCQCKit().strClientDataDir();
    pathTmp.AddLevel(L"MediaCache");
    pathTmp.AddLevel(strRepoMoniker);

    strToFill = pathTmp;
}


// Get details on a single item
tCIDLib::TVoid
TFacCQCMedia::QueryItemDetails(         tCQCKit::TCQCSrvProxy&  orbcSrv
                                , const TString&                strMoniker
                                , const TString&                strItemCookie
                                ,       TMediaItem&             mitemToFill) const
{
    tCIDLib::TCard4 c4Size = 0;
    THeapBuf mbufData(16 * 1024UL);

    // Ask the server for the info
    orbcSrv->bQueryData
    (
        strMoniker
        , kCQCMedia::strQuery_ItemDetails
        , strItemCookie
        , c4Size
        , mbufData
    );

    // Get the info back out
    TBinMBufInStream strmReply(&mbufData, c4Size);
    strmReply >> mitemToFill;
}


//
//  Convenience methods for anywhere an action command needs to provide a list
//  of media types, comparison types, or sort orders for an enum parameter.
//
const TString& TFacCQCMedia::strCompTypeOpts() const
{
    if (!m_atomOptsLoaded)
        LoadOpts();

    return m_strCompTypeOpts;
}

const TString& TFacCQCMedia::strMTypeOpts() const
{
    if (!m_atomOptsLoaded)
        LoadOpts();

    return m_strMTypeOpts;
}

const TString& TFacCQCMedia::strSortOpts() const
{
    if (!m_atomOptsLoaded)
        LoadOpts();

    return m_strSortOpts;
}


//  Clients that want to use the media data base must call this to start up the
//  cacher thread, or they will never get any data. And they shuld stop it before
//  they exit.
//
tCIDLib::TVoid TFacCQCMedia::StartMDBCacher()
{
    if (!m_thrMDBCache.bIsRunning())
    {
        try
        {
            m_thrMDBCache.Start();
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        catch(...)
        {
        }
    }
}

tCIDLib::TVoid TFacCQCMedia::StopMDBCacher()
{
    try
    {
        if (m_thrMDBCache.bIsRunning())
        {
            m_thrMDBCache.ReqShutdownSync(10000);
            m_thrMDBCache.eWaitForDeath(3000);
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);
    }

    catch(...)
    {
    }
}



// ---------------------------------------------------------------------------
//  TFacCQCMedia: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  This thread manages the media repo database caching thread. We just
//  periodically connect to the client service and check to see if there's
//  any new data available. If so we pull it down and replace the current
//  stuff for that repo.
//
tCIDLib::EExitCodes
TFacCQCMedia::eMDBCacheThread(TThread& thrThis, tCIDLib::TVoid* pData)
{
    static const tCIDLib::TCard4 c4MaxBufSz(16 * (1024 * 1024));

    // Let the calling thread go now since we initialized ok
    thrThis.Sync();

    // Build up the name server binding of the service we want
    TString strORBBinding = kCQCKit::pszCQCSlSrvIntf;
    strORBBinding.eReplaceToken(TSysInfo::strIPHostName(), kCIDLib::chLatin_h);

    // And now process until asked to stop
    tCIDLib::TBoolean bDone = kCIDLib::False;
    while (!bDone)
    {
        try
        {
            //
            //  See if the service is available and connect to it if so.
            //  Tell it not to cache the object since we only connect ever
            //  minute and it would always end up just being pushed out of
            //  the cache before we used it again.
            //
            TCQCClServiceClientProxy* porbcProxy
            (
                facCIDOrbUC().porbcMakeClient<TCQCClServiceClientProxy>
                (
                    strORBBinding, 2000
                )
            );

            //
            //  If we didn't get it, just sleep a bit and try again. We
            //  sleep a shorter time in this case than if we get to the
            //  end successfully.
            //
            if (!porbcProxy)
            {
                // If asked to shutdown while sleeping, then exit
                if (!thrThis.bSleep(5000))
                    break;

                // Go back to the top and try again
                continue;
            }

            // Put a janitor on the proxy so it gets cleaned up
            TJanitor<TCQCClServiceClientProxy> janProxy(porbcProxy);

            // Get a list of repos that the service is managing
            tCIDLib::TStrList colRepos;
            const tCIDLib::TCard4 c4RepoCnt = porbcProxy->c4QueryRepoList(colRepos);
            if (c4RepoCnt)
            {
                // Go through and add any we don't have
                TLocker lockrSync(&m_colMediaDBCache);
                for (tCIDLib::TCard4 c4Index = 0; c4Index < c4RepoCnt; c4Index++)
                {
                    const TString& strCur = colRepos[c4Index];
                    if (!m_colMediaDBCache.bKeyExists(strCur))
                        m_colMediaDBCache.objPlace(new TMDBCacheItem(strCur));
                }
            }
             else
            {
                //
                //  There aren't any, so remove any we currently have.
                //  Don't have to lock here since it's a single operation
                //  and the list is thread safe.
                //
                m_colMediaDBCache.RemoveAll();
            }

            // Now go through our list and update any that need it
            TMediaDBCache::TNCCursor cursCache(&m_colMediaDBCache);
            if (cursCache.bReset())
            {
                tCIDLib::TCard4         c4RawBytes;
                THeapBuf                mbufRaw(1024, c4MaxBufSz, 0x10000);
                TChunkedBinOutStream    strmDecomp(c4MaxBufSz);
                TZLibCompressor         zlibComp;

                do
                {
                    TMDBPtr& cptrCur = cursCache.objWCur();

                    //
                    //  If this one has been checked recently skip it.
                    //  Note this also prevents us from re-checking the
                    //  ones that we replace if new data shows up.
                    //
                    if (TTime::enctNow() < cptrCur->enctNextCheck())
                        continue;

                    // See if this guy has new data
                    tCIDLib::TBoolean bNewData = porbcProxy->bQueryRepoDB
                    (
                        cptrCur->strRepoMoniker()
                        , cptrCur->strDBSerialNum()
                        , c4RawBytes
                        , mbufRaw
                    );

                    if (bNewData)
                    {
                        if (facCQCMedia().bLogWarnings())
                        {
                            facCQCMedia().LogMsg
                            (
                                CID_FILE
                                , CID_LINE
                                , kMedMsgs::midStatus_NewData
                                , tCIDLib::ESeverities::Status
                                , tCIDLib::EErrClasses::AppStatus
                                , cptrCur->strRepoMoniker()
                            );
                        }

                        // Decompress the data
                        {
                            strmDecomp.Reset();
                            TMediaDB::c4DecompBinDump(mbufRaw, c4RawBytes, strmDecomp);
                        }

                        //
                        //  We have to drop the old cache item for this entry
                        //  and put in a new one.
                        //
                        //  !! Lock around this so that we don't leave a gap
                        //  where a client might come in and think the repo
                        //  isn't available. The list is thread safe, but we
                        //  are doing multiple operations here.
                        //
                        //  When we assign a new value, the old one will drop
                        //  reference it has. And oding it this way doesn't
                        //  invalidate our cursor.
                        //
                        {
                            TLocker lockrSync(&m_colMediaDBCache);

                            //
                            //  Get a copy of the moniker to pass in, don't
                            //  in a ref within the object we are about to
                            //  destroy!
                            //
                            TChunkedBinInStream strmSrc(strmDecomp);
                            TString strMoniker = cptrCur->strRepoMoniker();
                            TMDBPtr cptrNew
                            (
                                new TMDBCacheItem(strMoniker, strmSrc)
                            );
                            m_colMediaDBCache.ReplaceValue(cptrNew);
                        }
                    }
                     else
                    {
                        // Nothing new, just update the next check time
                        cptrCur->UpdateCheckTime();
                    }

                }   while (cursCache.bNext());
            }
        }

        catch(TError& errToCatch)
        {
            facCQCMedia().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kMedErrs::errcMCache_Except
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Internal
            );

            if (facCQCMedia().bShouldLog(errToCatch))
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }
        }

        catch(...)
        {
        }

        // Sleep for a while, break out if asked to stop.
        if (!thrThis.bSleep(8000))
            break;
    }

    return tCIDLib::EExitCodes::Normal;
}


//
//  This method handles generalized parsing of cookies. THe caller provides the
//  cookie and the expected number of components for the type of cookie he's
//  passing us. We do a very efficient parse (these are parsed a lot), and
//  return the values. If the number of values isn't as expected, we throw.
//
tCQCMedia::EMediaTypes
TFacCQCMedia::eParseCookie( const   TString&                strCookie
                            , const tCQCMedia::ECookieTypes eType
                            ,       tCIDLib::TCard2&        c2CatId
                            ,       tCIDLib::TCard2&        c2TitleId
                            ,       tCIDLib::TCard2&        c2ColId
                            ,       tCIDLib::TCard2&        c2ItemId) const
{
    //
    //  The absolute longest a cookie can be is 4 hex chars per possible id,
    //  plus possible ids minus one commas. So we set up a local char buffer
    //  of that size and copy into it.
    //
    const tCIDLib::TCard4 c4MaxLen = (kCQCMedia::c4MaxCookieTokens * 4)
                                     + (kCQCMedia::c4MaxCookieTokens - 1);
    const tCIDLib::TCard4 c4Len = strCookie.c4Length();

    // Make sure it's not longer than the max legal
    if (c4Len > c4MaxLen)
    {
        ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMedErrs::errcProto_BadCookie
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TString(tCQCMedia::strXlatECookieTypes(eType))
        );
    }

    // Looks ok, so create a local buffer and copy into it
    tCIDLib::TCh achBuf[c4MaxLen + 1];
    TRawStr::CopyStr(achBuf, strCookie.pszBuffer(), c4Len);

    //
    //  For up to the maximum+1 number of tokens, find that string and convert
    //  to a TCard2 value. By going one extra, we catch a cookie with too
    //  many tokens.
    //
    tCIDLib::TCard2     ac2Ids[kCQCMedia::c4MaxCookieTokens + 1];
    tCIDLib::TBoolean   bOk;
    tCIDLib::TCard4     c4CurId;
    tCIDLib::TCard4     c4Count = 0;
    tCIDLib::TCh*       pszCur = achBuf;
    tCIDLib::TCh*       pszEnd = achBuf;
    for (; c4Count < kCQCMedia::c4MaxCookieTokens + 1; c4Count++)
    {
        // Run forward to a comma or null
        tCIDLib::TCh chCur = *pszEnd;
        while((chCur != kCIDLib::chComma) && (chCur != kCIDLib::chNull))
        {
            pszEnd++;
            chCur = *pszEnd;
        }

        // Put a null in this position
        *pszEnd = kCIDLib::chNull;

        // Convert this one and store the id
        c4CurId = TRawStr::c4AsBinary(pszCur, bOk, tCIDLib::ERadices::Hex);

        // If bad, or too big, it's a bad cookie
        if (!bOk || (c4CurId > kCIDLib::c2MaxCard))
        {
            ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kMedErrs::errcProto_BadCookie
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
                , TString(tCQCMedia::strXlatECookieTypes(eType))
            );
        }

        // Looks ok, so store this id
        ac2Ids[c4Count] = tCIDLib::TCard2(c4CurId);

        // If we hit a null, then we are done
        if (chCur == kCIDLib::chNull)
            break;

        // Else, move forward for the next round
        pszEnd++;
        pszCur = pszEnd;
    }

    //
    //  If wrong count of cookie components, or if the 0th one is larger
    //  than the maximum legal media type, it's bad. The count should be
    //  the enum plus 1, since there's always a media type value first.
    //
    if ((c4Count != tCIDLib::TCard4(eType) + 1)
    ||  (ac2Ids[0] > tCIDLib::c4EnumOrd(tCQCMedia::EMediaTypes::Max)))
    {
        ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMedErrs::errcProto_BadCookie
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TString(tCQCMedia::strXlatECookieTypes(eType))
        );
    }

    //
    //  Looks ok, so store them. Just store them all. If they never got
    //  set they'll be bogus values but the caller won't look at the ones
    //  he doesn't expect.
    //
    c2CatId   = ac2Ids[1];
    c2TitleId = ac2Ids[2];
    c2ColId  = ac2Ids[3];
    c2ItemId = ac2Ids[4];

    // The 0th one is the type
    return tCQCMedia::EMediaTypes(ac2Ids[0]);
}


// Faults in some option strings which are formatted out versions of some enums
tCIDLib::TVoid TFacCQCMedia::LoadOpts() const
{
    TBaseLock lockInit;
    if (!m_atomOptsLoaded)
    {
        tCIDLib::ForEachE<tCQCMedia::ECompTypes>
        (
            [this](const tCQCMedia::ECompTypes eType)
            {
                if (eType != tCQCMedia::ECompTypes::Min)
                    m_strCompTypeOpts.Append(L", ");
                m_strCompTypeOpts.Append(tCQCMedia::strXlatECompTypes(eType));
                return kCIDLib::True;
            }
        );
        tCIDLib::ForEachE<tCQCMedia::EMediaTypes>
        (
            [this](const tCQCMedia::EMediaTypes eType)
            {
                if (eType != tCQCMedia::EMediaTypes::Min)
                    m_strMTypeOpts.Append(L", ");
                m_strMTypeOpts.Append(tCQCMedia::strXlatEMediaTypes(eType));
                return kCIDLib::True;
            }
        );
        tCIDLib::ForEachE<tCQCMedia::ESortOrders>
        (
            [this](const tCQCMedia::ESortOrders eOrder)
            {
                if (eOrder != tCQCMedia::ESortOrders::Min)
                    m_strSortOpts.Append(L", ");
                m_strSortOpts.Append(tCQCMedia::strXlatESortOrders(eOrder));
                return kCIDLib::True;
            }
        );

        m_atomOptsLoaded.Set();
    }
}


tCIDLib::TVoid
TFacCQCMedia::ThrowBadMsgFmt(const  tCIDLib::TCh* const pszFile
                            , const tCIDLib::TCard4     c4Line
                            , const TString&            strType) const
{
    ThrowErr
    (
        pszFile
        , c4Line
        , kMedErrs::errcProto_BadMsgFmt
        , tCIDLib::ESeverities::Failed
        , tCIDLib::EErrClasses::Protocol
        , strType
    );
}


