//
// FILE NAME: CQCMedia_StdRepoDrvEng.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/14/2012
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
//  This is the implementation file for the standard repository driver
//  engine class.
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
RTTIDecls(TCQCStdMediaRepoEng,TObject)



// ---------------------------------------------------------------------------
//  CLASS: TCQCStdMediaRepoEng
// PREFIX: sdrv
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCStdMediaRepoEng: Constructors and Destructor
// ---------------------------------------------------------------------------

// The driver tells us what types of media we are to support
TCQCStdMediaRepoEng::
TCQCStdMediaRepoEng(const   TString&            strMoniker
                    , const tCQCMedia::EMTFlags eTypes) :

    m_eTypes(eTypes)
    , m_mdbInfo()
    , m_mbufTmp(32 * 1024, 8 * (1024 * 1024), 64 * 1024)
    , m_mbufTmp2(64 * 1024, 8 * (1024 * 1024), 16 * 1024)
    , m_strMissingCol(kMedMsgs::midMiss_Collect)
    , m_strMissingItem(kMedMsgs::midMiss_Item)
    , m_strMoniker(strMoniker)
{
}

TCQCStdMediaRepoEng::~TCQCStdMediaRepoEng()
{
}


// ---------------------------------------------------------------------------
//  TCQCStdMediaRepoEng: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Looks up a cookie by cookie type and name
tCIDLib::TBoolean
TCQCStdMediaRepoEng::bCookieLookup( const   TString&    strQData
                                    ,       TString&    strToFill)
{
    //
    //  The incoming data should be a quoted comma list that contains the
    //  following info:
    //
    //      media type, cookie type, cat name [, title name, col name, item name]
    //
    //  So, if looking up a category cookie, only the category name is
    //  requied. If looking up an item name, then the category, title,
    //  collection, and item name is required, since we have to search down
    //  through the layers to find the item.
    //
    tCIDLib::TCard4  c4ErrIndex;
    TVector<TString> colValues(6);
    if (!TStringTokenizer::bParseQuotedCommaList(strQData, colValues, c4ErrIndex)
    ||  (colValues.c4ElemCount() < 3))
    {
        facCQCMedia().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMedErrs::errcDrv_BadQueryData
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::BadParms
            , m_strMoniker
        );
    }

    // Translate the media type
    const tCQCMedia::EMediaTypes eMType = tCQCMedia::eXlatEMediaTypes(colValues[0]);

    // Translate the cookie type
    const tCQCMedia::ECookieTypes eCType = tCQCMedia::eXlatECookieTypes(colValues[1]);

    // Make sure that hte values make sense
    const tCIDLib::TCard4 c4ValCnt = colValues.c4ElemCount();
    if ((eMType == tCQCMedia::EMediaTypes::Count)
    ||  (eCType == tCQCMedia::ECookieTypes::Count)
    ||  ((eCType == tCQCMedia::ECookieTypes::Cat) && (c4ValCnt != 3))
    ||  ((eCType == tCQCMedia::ECookieTypes::Title) && (c4ValCnt != 4))
    ||  ((eCType == tCQCMedia::ECookieTypes::Collect) && (c4ValCnt != 5))
    ||  ((eCType == tCQCMedia::ECookieTypes::Item) && (c4ValCnt != 6)))
    {
        facCQCMedia().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMedErrs::errcDrv_BadQueryData
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::BadParms
            , m_strMoniker
        );
    }

    // If we have fewer than 6 values, then pad out the rest
    tCIDLib::TCard4 c4CurCnt = colValues.c4ElemCount();
    for (; c4CurCnt < 6; c4CurCnt++)
        colValues.objAdd(TString::strEmpty());

    //
    //  And do the lookup and return its return. We pass all the potential
    //  values in. It'll use as many as are required.
    //
    return m_mdbInfo.bCookieLookup
    (
        eCType
        , eMType
        , colValues[2]
        , colValues[3]
        , colValues[4]
        , colValues[5]
        , strToFill
    );
}


//
//  Returns info about the audio format of either a music title or music item.
//  We get a cookie to indicate the thing to query. If it's a collection we
//  just look at the first item of the collection.
//
//  We return bit depth, bit rate, channels, and sample rate formatted out into
//  a string, space separated.
//
//  If the cookie is not found we just return false. If it is found, we
//  return true. If it's bad, or not for music, or not an item, collection
//  or title cookie, we throw.
//
tCIDLib::TBoolean
TCQCStdMediaRepoEng::bQueryAudioFmt(const   TString&    strCookie
                                    ,       TString&    strToFill)
{
    // Parse the cookie
    tCIDLib::TCard2         c2CatId;
    tCIDLib::TCard2         c2ColId;
    tCIDLib::TCard2         c2ItemId;
    tCIDLib::TCard2         c2TitleId;
    tCQCMedia::EMediaTypes  eMType;

    tCQCMedia::ECookieTypes eCType = facCQCMedia().eCheckCookie
    (
        strCookie
        , eMType
        , c2CatId
        , c2TitleId
        , c2ColId
        , c2ItemId
    );

    // We expect a music oriented cookie, which is not a category type
    if ((eMType != tCQCMedia::EMediaTypes::Music)
    ||  (eCType == tCQCMedia::ECookieTypes::Cat))
    {
        facCQCMedia().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMedErrs::errcDB_BadCookieType
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::BadParms
            , m_strMoniker
        );
    }

    // Make sure we support music
    CheckType(eMType, CID_LINE);


    // Find the indicated title set. If not found return false
    const TMediaTitleSet* pmtsSrc = m_mdbInfo.pmtsById(eMType, c2TitleId, kCIDLib::False);
    if (!pmtsSrc)
        return kCIDLib::False;

    tCIDLib::TCard4 c4BitDepth;
    tCIDLib::TCard4 c4BitRate;
    tCIDLib::TCard4 c4Channels;
    tCIDLib::TCard4 c4SampleRate;

    if ((eCType == tCQCMedia::ECookieTypes::Title)
    ||  (eCType == tCQCMedia::ECookieTypes::Collect))
    {
        c4BitDepth   = pmtsSrc->c4BitDepth();
        c4BitRate    = pmtsSrc->c4BitRate();
        c4Channels   = pmtsSrc->c4Channels();
        c4SampleRate = pmtsSrc->c4SampleRate();
    }
     else
    {
        const TMediaItem* pmitemSrc = m_mdbInfo.pmitemById
        (
            eMType, c2ItemId, kCIDLib::False
        );

        // If we don't find it, return false
        if (!pmitemSrc)
            return kCIDLib::False;

        c4BitDepth   = pmitemSrc->c4BitDepth();
        c4BitRate    = pmitemSrc->c4BitRate();
        c4Channels   = pmitemSrc->c4Channels();
        c4SampleRate = pmitemSrc->c4SampleRate();
    }

    // Format out the info for return
    strToFill.SetFormatted(c4BitDepth);
    strToFill.Append(kCIDLib::chSpace);
    strToFill.AppendFormatted(c4BitRate);
    strToFill.Append(kCIDLib::chSpace);
    strToFill.AppendFormatted(c4Channels);
    strToFill.Append(kCIDLib::chSpace);
    strToFill.AppendFormatted(c4SampleRate);

    // We found it so return true
    return kCIDLib::True;
}


//
//  This guy calls a virtual method if the cover art isn't loaded, to allow
//  the derived class to load it up. We don't have any idea how the actual
//  image data gets loaded. It might be file based, an HTTP query, or some
//  call out to a device other other network service.
//
//  THe caller indicates which level the art should be at. If title level we
//  first check the title set, and then fall back to the first collection, then
//  the second, and so on until we find something with an art path or give up.
//  In this case, we get a title cookie.
//
//  If it's a collection cookie and they want the art for a collection or
//  nothing. If it's for item level art, we check the item. If not found,
//  we'll return the collection level art.
//
//  He also indicates if he wants large or thumb art.
//
tCIDLib::TBoolean
TCQCStdMediaRepoEng::bQueryCoverArt(const   TString&                strCookie
                                    ,       tCIDLib::TCard4&        c4OutBytes
                                    ,       THeapBuf&               mbufToFill
                                    , const tCQCMedia::EArtLevels   eLevel
                                    , const tCQCMedia::ERArtTypes   eArtType)
{
    // Parse the passed title cookie
    tCIDLib::TCard2 c2CatId = 0;
    tCIDLib::TCard2 c2TitleId = 0;
    tCIDLib::TCard2 c2ColId = 0;
    tCIDLib::TCard2 c2ItemId = 0;
    tCQCMedia::EMediaTypes eMType;

    if (eLevel == tCQCMedia::EArtLevels::TitleSet)
        eMType = facCQCMedia().eParseTitleCookie(strCookie, c2CatId, c2TitleId);
     else if (eLevel == tCQCMedia::EArtLevels::Collect)
        eMType = facCQCMedia().eParseColCookie(strCookie, c2CatId, c2TitleId, c2ColId);
    else
        eMType = facCQCMedia().eParseItemCookie(strCookie, c2CatId, c2TitleId, c2ColId, c2ItemId);
    CheckType(eMType, CID_LINE);

    // Find the title set. If not found, then obviously we cannot return any
    const TMediaTitleSet* pmtsSrc = m_mdbInfo.pmtsById(eMType, c2TitleId, kCIDLib::False);
    if (!pmtsSrc)
        return kCIDLib::False;

    // If they want title art, check the title first
    const TMediaImg* pmimgFound = 0;
    if (eLevel == tCQCMedia::EArtLevels::TitleSet)
    {
        if (pmtsSrc->c2ArtId())
        {
            pmimgFound = m_mdbInfo.pmimgByIdNC(eMType, pmtsSrc->c2ArtId(), kCIDLib::False);
        }

        if (!pmimgFound)
        {
            // Work through the collections to see if anybody has any data
            const tCIDLib::TCard4 c4ColCnt = pmtsSrc->c4ColCount();
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ColCnt; c4Index++)
            {
                const TMediaCollect* pmcolCur = m_mdbInfo.pmcolById
                (
                    eMType, pmtsSrc->c2ColIdAt(c4Index), kCIDLib::False
                );

                if (pmcolCur && pmcolCur->c2ArtId())
                {
                    pmimgFound = m_mdbInfo.pmimgByIdNC
                    (
                        eMType, pmcolCur->c2ArtId(), kCIDLib::False
                    );

                    if (pmimgFound)
                        break;
                }
            }
        }
    }
     else if (eLevel == tCQCMedia::EArtLevels::Collect)
    {
        // Check the specific collection they want
        const TMediaCollect* pmcolCur = m_mdbInfo.pmcolById
        (
            eMType, pmtsSrc->c2ColIdAt(c2ColId - 1), kCIDLib::False
        );
        if (pmcolCur && pmcolCur->c2ArtId())
            pmimgFound = m_mdbInfo.pmimgByIdNC(eMType, pmcolCur->c2ArtId(), kCIDLib::False);
    }
     else if (eLevel == tCQCMedia::EArtLevels::Item)
    {
        const TMediaCollect* pmcolCur = m_mdbInfo.pmcolById
        (
            eMType, pmtsSrc->c2ColIdAt(c2ColId - 1), kCIDLib::False
        );

        // If we got the collection, get out the item
        const TMediaItem* pmitemCur = 0;
        if (pmcolCur)
        {
            pmitemCur = m_mdbInfo.pmitemByIdNC
            (
                eMType, pmcolCur->c2ItemIdAt(c2ItemId - 1), kCIDLib::False
            );
        }

        //
        //  If we got the item, but it has has no item specific art, see
        //  if the collection is a playlist. If so, let's go back and update
        //  the collection to reflect the original (real) collection this
        //  item is part of. That way we'll get the right cover art.
        //
        if (pmitemCur && !pmitemCur->c2ArtId() && pmcolCur->bIsPlayList())
        {
            tCIDLib::TCard2 c2ColInd, c2ItemInd;
            const TMediaTitleSet* pmtsOrg;
            const TMediaCollect*  pmcolOrg;
            if (m_mdbInfo.bFindItemContainer(pmitemCur->eType()
                                            , pmitemCur->c2Id()
                                            , c2ColInd
                                            , c2ItemInd
                                            , pmtsOrg
                                            , pmcolOrg))
            {
                pmcolCur = pmcolOrg;
            }
        }

        //
        //  If we got the item and it has art, get that. Else, if we got the
        //  collection and it has art, get that.
        //
        if (pmitemCur && pmitemCur->c2ArtId())
            pmimgFound = m_mdbInfo.pmimgByIdNC(eMType, pmitemCur->c2ArtId(), kCIDLib::False);
        else if (pmcolCur && pmcolCur->c2ArtId())
            pmimgFound = m_mdbInfo.pmimgByIdNC(eMType, pmcolCur->c2ArtId(), kCIDLib::False);
    }

    c4OutBytes = 0;
    if (pmimgFound)
    {
        //
        //  If we have the art cached already that they want, then just
        //  return that. Else we have to get it loaded.
        //
        c4OutBytes = pmimgFound->c4QueryArt(mbufToFill, eArtType);
        if (!c4OutBytes)
        {
            try
            {
                c4OutBytes = c4LoadArtData(*pmimgFound, mbufToFill, eArtType, 0);
            }

            catch(const TError& errToCatch)
            {
                if (facCQCMedia().bShouldLog(errToCatch))
                    TModule::LogEventObj(errToCatch);

                if (facCQCMedia().bLogFailures())
                {
                    facCQCMedia().LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , kMedErrs::errcDB_CantLoadArt
                        , tCIDLib::ESeverities::Failed
                        , tCIDLib::EErrClasses::CantDo
                        , pmimgFound->strArtPath(eArtType)
                    );
                }
                c4OutBytes = 0;
            }
        }
    }
    return (c4OutBytes != 0);
}


//
//  We support a number of backdoor commands to query data. These are overrides
//  of those methods. Derived repo drivers can override further still to
//  provide more queries or to intercept incoming queries or the returned
//  information and modify them.
//
tCIDLib::TBoolean
TCQCStdMediaRepoEng::bQueryData(const   TString&            strQType
                                , const TString&            strQData
                                ,       tCIDLib::TCard4&    c4OutBytes
                                ,       THeapBuf&           mbufToFill)
{
    //
    //  Make sure the out bytes get initalized to zero just in case, so
    //  that the ORB doesn't try to stream back some crazy random data.
    //
    c4OutBytes = 0;

    tCIDLib::TBoolean bRet = kCIDLib::True;
    if (strQType == kCQCMedia::strQuery_ColDetails)
    {
        QueryColDetails(strQData, c4OutBytes, mbufToFill);
    }
     else if (strQType == kCQCMedia::strQuery_ColItems)
    {
        QueryColItems(strQData, c4OutBytes, mbufToFill);
    }
     else if (strQType == kCQCMedia::strQuery_ColLoc)
    {
        QueryColLoc(strQData, c4OutBytes, mbufToFill);
    }
     else if (strQType == kCQCMedia::strQuery_ItemDetails)
    {
        QueryItemDetails(strQData, c4OutBytes, mbufToFill);
    }
     else if (strQType == kCQCMedia::strQuery_TitleDetails)
    {
        QueryTitleDetails(strQData, c4OutBytes, mbufToFill);
    }
     else if ((strQType == kCQCMedia::strQuery_QueryArt)
          ||  (strQType == kCQCMedia::strQuery_QueryItemArt)
          ||  (strQType == kCQCMedia::strQuery_QueryTitleArt)
          ||  (strQType == kCQCMedia::strQuery_QueryThumbArt)
          ||  (strQType == kCQCMedia::strQuery_QueryItemThumbArt)
          ||  (strQType == kCQCMedia::strQuery_QueryTitleThumbArt)
          ||  (strQType == kCQCMedia::strQuery_QueryPosterArt))
    {
        tCQCMedia::ERArtTypes eType = tCQCMedia::ERArtTypes::LrgCover;
        tCQCMedia::EArtLevels eLevel = tCQCMedia::EArtLevels::TitleSet;

        if (strQType == kCQCMedia::strQuery_QueryArt)
        {
            eLevel = tCQCMedia::EArtLevels::Collect;
        }
         else if (strQType == kCQCMedia::strQuery_QueryItemArt)
        {
            eLevel = tCQCMedia::EArtLevels::Item;
        }
         else if (strQType == kCQCMedia::strQuery_QueryTitleArt)
        {
            eLevel = tCQCMedia::EArtLevels::TitleSet;
        }
         else if (strQType == kCQCMedia::strQuery_QueryThumbArt)
        {
            eType = tCQCMedia::ERArtTypes::SmlCover;
            eLevel = tCQCMedia::EArtLevels::Collect;
        }
         else if (strQType == kCQCMedia::strQuery_QueryItemThumbArt)
        {
            eType = tCQCMedia::ERArtTypes::SmlCover;
            eLevel = tCQCMedia::EArtLevels::Item;
        }
         else if (strQType == kCQCMedia::strQuery_QueryTitleThumbArt)
        {
            eType = tCQCMedia::ERArtTypes::SmlCover;
            eLevel = tCQCMedia::EArtLevels::TitleSet;
        }
         else if (strQType == kCQCMedia::strQuery_QueryPosterArt)
        {
            eType = tCQCMedia::ERArtTypes::Poster;
            eLevel = tCQCMedia::EArtLevels::Collect;
        }

        bRet = bQueryCoverArt
        (
            strQData, c4OutBytes, mbufToFill, eLevel, eType
        );
    }
     else if (strQType == kCQCMedia::strQuery_QueryImgById)
    {
        // The QData is media type, space, and the id of the image
        TString strMType = strQData;
        TString strId;
        tCIDLib::TCard4 c4Id;
        if (!strMType.bSplit(strId, kCIDLib::chSpace)
        ||  !strId.bToCard4(c4Id, tCIDLib::ERadices::Dec))
        {
            return kCIDLib::False;
        }

        // Find the image by id
        tCQCMedia::EMediaTypes eMType;
        if (strMType == L"Music")
            eMType = tCQCMedia::EMediaTypes::Music;
        else if (strMType == L"Movie")
            eMType = tCQCMedia::EMediaTypes::Movie;
        else
            return kCIDLib::False;

        //
        //  We have to get a non-const pointer here in order to set the img data
        //  should we need to fault it in.
        //
        TMediaImg* pmimgRet = m_mdbInfo.pmimgByIdNC
        (
            eMType, tCIDLib::TCard2(c4Id), kCIDLib::False
        );
        if (!pmimgRet)
            return kCIDLib::False;

        //
        //  See if we have large art cached yet. We do this first since, if the repo
        //  doesn't support separate thumbs, this will insure the large art gets
        //  loaded first, and he'll cache it. Then he can scale the data he already
        //  has loaded when we subsequently ask for the thumb data, instead of having
        //  to load the file again.
        //
        c4OutBytes = 0;
        mbufToFill.PutCard4(kCIDLib::chLatin_L, c4OutBytes);
        c4OutBytes += 4;

        tCIDLib::TCard4 c4ImgSz = pmimgRet->c4Size(tCQCMedia::ERArtTypes::LrgCover);
        if (c4ImgSz)
        {
            c4ImgSz = pmimgRet->c4QueryArt
            (
                mbufToFill, tCQCMedia::ERArtTypes::LrgCover, c4OutBytes + 4
            );
        }
         else
        {
            // Load new data, leave room for the size
            c4ImgSz = c4LoadArtData
            (
                *pmimgRet
                , mbufToFill
                , tCQCMedia::ERArtTypes::LrgCover
                , c4OutBytes + 4
            );
        }

        //
        //  Go back and put in the size and then move past the image data,
        //  and put out the size again as a sanity check.
        //
        mbufToFill.PutCard4(c4ImgSz, c4OutBytes);
        c4OutBytes += c4ImgSz + 4;
        mbufToFill.PutCard4(c4ImgSz, c4OutBytes);
        c4OutBytes += 4;


        // Now do the thumb. The same steps as above
        mbufToFill.PutCard4(kCIDLib::chLatin_T, c4OutBytes);
        c4OutBytes += 4;

        c4ImgSz = pmimgRet->c4Size(tCQCMedia::ERArtTypes::SmlCover);
        if (c4ImgSz)
        {
            c4ImgSz = pmimgRet->c4QueryArt
            (
                mbufToFill, tCQCMedia::ERArtTypes::SmlCover, c4OutBytes + 4
            );
        }
         else
        {
            // Load new data, leave room for the size
            c4ImgSz = c4LoadArtData
            (
                *pmimgRet
                , mbufToFill
                , tCQCMedia::ERArtTypes::SmlCover
                , c4OutBytes + 4
            );
        }

        // Go back and put in the size and then move past the iamge
        mbufToFill.PutCard4(c4ImgSz, c4OutBytes);
        c4OutBytes += c4ImgSz + 4;

        // And the second sanity check copy of the image size
        mbufToFill.PutCard4(c4ImgSz, c4OutBytes);
        c4OutBytes += 4;
    }
     else
    {
        // Don't know what it is, so throw
        facCQCMedia().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMedErrs::errcDrv_UnknownDataQuery
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::BadParms
            , strQType
            , m_strMoniker
        );
    }
    return bRet;
}


tCIDLib::TBoolean
TCQCStdMediaRepoEng::bQueryData2(const  TString&            strQType
                                ,       tCIDLib::TCard4&    c4IOBytes
                                ,       THeapBuf&           mbufIO)
{
    tCIDLib::TBoolean bRet = kCIDLib::True;

    if (strQType == kCQCMedia::strQuery_CatList)
        QueryCatList(c4IOBytes, mbufIO);
    else if (strQType == kCQCMedia::strQuery_DefCatInfo)
        QueryDefCatInfo(c4IOBytes, mbufIO);
    else if (strQType == kCQCMedia::strQuery_RandomGenreItems)
        QueryRandomCatItems(c4IOBytes, mbufIO);
    else
    {
        // Don't know what it is, so throw
        facCQCMedia().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMedErrs::errcDrv_UnknownDataQuery
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::BadParms
            , strQType
            , m_strMoniker
        );
    }
    return bRet;
}


tCIDLib::TBoolean
TCQCStdMediaRepoEng::bQueryTextVal( const   TString&    strQType
                                    , const TString&    strQData
                                    ,       TString&    strToFill)
{
    tCIDLib::TBoolean bRet = kCIDLib::True;
    strToFill.Clear();

    if (strQType == kCQCMedia::strQuery_QueryAudioFmt)
        bRet = bQueryAudioFmt(strQData, strToFill);
    else if (strQType == kCQCMedia::strQuery_CookieLookup)
        bRet = bCookieLookup(strQData, strToFill);
    else if (strQType == kCQCMedia::strQuery_MediaFmt)
        QueryMediaFormat(strQData, strToFill);
    else if (strQType.bStartsWithI(L"Fld_"))
        QuerySingleFld(strQType, strQData, strToFill);
     else
    {
        facCQCMedia().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMedErrs::errcDrv_UnknownDataQuery
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::BadParms
            , strQType
            , m_strMoniker
        );
    }

    // We don't really use the return so just return true
    return bRet;
}


//
//  We don't currently support any outgoing setting/sending backdoor commands,
//  but some of our derived classes might. So we provide some dummy
//  implementations so that they can call through to us in case we should
//  add any later.
//
tCIDLib::TBoolean
TCQCStdMediaRepoEng::bSendData( const   TString&            strSendType
                                ,       TString&            strDataName
                                ,       tCIDLib::TCard4&    c4Bytes
                                ,       THeapBuf&           mbufToSend)
{
    //
    //  BE SURE to clear the bytes so that it doesn't try to stream back over
    //  some random number of bytes using whatever value happens to be in
    //  the output parameter.
    //
    c4Bytes = 0;
    return kCIDLib::False;
}


//
//  This is called when the driver wants to get cover art that is not already cached.
//  We work in terms of a public virtual method, that drivers must override and do the
//  actual data loading themselves.
//
tCIDLib::TCard4
TCQCStdMediaRepoEng::c4LoadArtData( const   TMediaImg&              mimgToLoad
                                    ,       TMemBuf&                mbufToFill
                                    , const tCQCMedia::ERArtTypes   eType
                                    , const tCIDLib::TCard4         c4AtIndex)
{
    //
    //  See if there is art of the type they want already cached. If so, then let's
    //  just return it directly.
    //
    if (mimgToLoad.c4Size(eType))
        return mimgToLoad.c4QueryArt(mbufToFill, eType, c4AtIndex);

    tCIDLib::TCard4 c4ImgSz = 0;
    try
    {
        // It's not there already, so let's give the driver a chance to load it
        c4ImgSz = c4LoadRawCoverArt(mimgToLoad, m_mbufTmp, eType);
        if (c4ImgSz)
        {
            // Copy it to the caller's buffer at the right place
            mbufToFill.CopyIn(m_mbufTmp, c4ImgSz, c4AtIndex);

            // And cache it
            m_mdbInfo.SetArt
            (
                mimgToLoad.c2Id(), mimgToLoad.eType(), eType, m_mbufTmp, c4ImgSz
            );
        }
    }

    catch(const TError& errToCatch)
    {
        if (facCQCMedia().bLogWarnings())
        {
            if (!errToCatch.bLogged())
                TModule::LogEventObj(errToCatch);

            //
            //  If they asked for small, and that path is empty, then what really
            //  failed was the large art or the conversion thereof. So use that path
            //  if needed.
            //
            TString strPath;
            if (eType == tCQCMedia::ERArtTypes::SmlCover)
                strPath = mimgToLoad.strArtPath(tCQCMedia::ERArtTypes::SmlCover);
            if (!strPath.bIsEmpty())
                strPath = mimgToLoad.strArtPath(tCQCMedia::ERArtTypes::LrgCover);

            facCQCMedia().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kMedErrs::errcDB_CantLoadArt
                , tCIDLib::ESeverities::Warn
                , tCIDLib::EErrClasses::CantDo
                , strPath
            );
        }
        c4ImgSz = 0;
    }
    return c4ImgSz;
}


// Handle the card value query driver backdoor method
tCIDLib::TCard4 TCQCStdMediaRepoEng::c4QueryVal(const TString& strValId)
{
    tCIDLib::TCard4 c4Ret = 0;

    if (strValId == kCQCMedia::strQuery_MediaTypes)
        c4Ret = tCIDLib::TCard4(m_eTypes);
    else if (strValId == kCQCMedia::strQuery_ProtoVer)
        c4Ret = kCQCMedia::c4ProtoVer;
    else
    {
        // Don't know what this value is
        facCQCMedia().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMedErrs::errcDrv_UnknownValQuery
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::BadParms
            , strValId
            , m_strMoniker
        );
    }
    return c4Ret;
}


//
//  We currently don't support any outgoing backdoor driver commands. The
//  repo drivers may handle some up stream from us, but there aren't any
//  standard ones to handle yet.
//
tCIDLib::TCard4
TCQCStdMediaRepoEng::c4SendCmd(const TString&, const TString&)
{
    return 0;
}


//
//  Verifies that the passed media type is one that the derived class said
//  it supports. If not, it throws an unsupportd media type exception.
//
tCIDLib::TVoid
TCQCStdMediaRepoEng::CheckType( const   tCQCMedia::EMediaTypes  eType
                                ,       tCIDLib::TCard4         c4Line)
{
    if (!facCQCMedia().bTestMediaFlags(eType, m_eTypes))
    {
        facCQCMedia().ThrowErr
        (
            CID_FILE
            , c4Line
            , kMedErrs::errcDB_UnsupportedMType
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotSupported
            , tCQCMedia::strXlatEMediaTypes(eType)
            , m_strMoniker
        );
    }
}


// Proivde access to the media types that the repo driver set on us
tCQCMedia::EMTFlags TCQCStdMediaRepoEng::eMediaTypes() const
{
    return m_eTypes;
}


//
//  Provides access to the in memory database to the derive class, mostly for
//  when it is being loaded.
//
const TMediaDB& TCQCStdMediaRepoEng::mdbInfo() const
{
    return m_mdbInfo;
}

TMediaDB& TCQCStdMediaRepoEng::mdbInfo()
{
    return m_mdbInfo;
}


//
//  Returns info about the categories of the requested media type. We can
//  optionally return only categories that have at least one title set in
//  them.
//
tCIDLib::TVoid
TCQCStdMediaRepoEng::QueryCatList(tCIDLib::TCard4& c4IOBytes, THeapBuf& mbufIO)
{
    // Pull out the incoming info
    tCIDLib::TBoolean bNoEmpty;
    tCQCMedia::EMediaTypes eType;
    {
        TBinMBufInStream strmIn(&mbufIO, c4IOBytes);
        strmIn  >> bNoEmpty
                >> eType;
    }

    // Just to be safe, zero out the return bytes now
    c4IOBytes = 0;

    //
    //  Get the category list for the indicated media type. WE get it sorted by
    //  name. We just get pointers to the objects, so this is fairly light, and
    //  we can ask for it to be sorted as well, so we sort by name.
    //
    tCQCMedia::TCatList colCats(tCIDLib::EAdoptOpts::NoAdopt);
    const tCIDLib::TCard4 c4Count = m_mdbInfo.c4QueryCatList
    (
        eType, colCats, &TMediaCat::eCompByName
    );

    // Loop through them and load up name and cookie lists
    tCIDLib::TStrList   colCookies(c4Count);
    tCIDLib::TStrList   colNames(c4Count);
    TString             strCookie;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TMediaCat* pmcatCur = colCats[c4Index];

        //
        //  If they only want non-empty categories, then see if this guy
        //  has at least one title set in it.
        //
        if (bNoEmpty && !m_mdbInfo.bCatHasSomeSets(*pmcatCur))
            continue;

        // Format the cookie for this guy
        facCQCMedia().FormatCatCookie(eType, pmcatCur->c2Id(), strCookie);
        colNames.objAdd(pmcatCur->strName());
        colCookies.objAdd(strCookie);
    }

    // And format the return info
    TBinMBufOutStream strmOut(&mbufIO);
    strmOut << colNames.c4ElemCount()
            << colNames
            << colCookies;
    strmOut.Flush();
    c4IOBytes = strmOut.c4CurSize();
}


// Queies collection detail information
tCIDLib::TVoid
TCQCStdMediaRepoEng::QueryColDetails(const  TString&            strColCookie
                                    ,       tCIDLib::TCard4&    c4OutBytes
                                    ,       THeapBuf&           mbufToFill)
{
    // Parse the collection cookie
    tCIDLib::TCard2         c2CatId;
    tCIDLib::TCard2         c2TitleId;
    tCIDLib::TCard2         c2ColId;
    tCQCMedia::EMediaTypes  eType;
    eType = facCQCMedia().eParseColCookie
    (
        strColCookie, c2CatId, c2TitleId, c2ColId
    );
    CheckType(eType, CID_LINE);

    // Find the indicated title set
    const TMediaTitleSet* pmtsSrc = m_mdbInfo.pmtsById(eType, c2TitleId, kCIDLib::True);

    //
    //  And find the indicated collection. The collection ids are the 1 based
    //  collection number, so sub one for the offset.
    //
    const TMediaCollect& mcolDet = pmtsSrc->mcolAt(m_mdbInfo, c2ColId - 1);

    //
    //  And format the return info. We just return the collection object,
    //  since that's cheaper than doing all the individual fields and
    //  almost all of it would be wanted anyway. We also give back the
    //  containing title's name for convenience.
    //
    TBinMBufOutStream strmOut(&mbufToFill);
    strmOut << mcolDet
            << pmtsSrc->strName()
            << pmtsSrc->c4ColCount();
    strmOut.Flush();
    c4OutBytes = strmOut.c4CurSize();
}


//
//  Returns the media items that are in the indicated collection.
//
tCIDLib::TVoid
TCQCStdMediaRepoEng::QueryColItems( const   TString&            strColCookie
                                    ,       tCIDLib::TCard4&    c4OutBytes
                                    ,       THeapBuf&           mbufToFill)
{
    // Parse the collection cookie
    tCIDLib::TCard2         c2CatId;
    tCIDLib::TCard2         c2TitleId;
    tCIDLib::TCard2         c2ColId;
    tCQCMedia::EMediaTypes  eType;
    eType = facCQCMedia().eParseColCookie(strColCookie, c2CatId, c2TitleId, c2ColId);
    CheckType(eType, CID_LINE);

    // Find the indicated title set
    const TMediaTitleSet* pmtsSrc = m_mdbInfo.pmtsById(eType, c2TitleId, kCIDLib::True);

    //
    //  And find the indicated collection. The collection ids are the 1 based
    //  collection number, so sub one for the offset.
    //
    const TMediaCollect& mcolDet = pmtsSrc->mcolAt(m_mdbInfo, c2ColId - 1);

    //
    //  And format the return info. We put out the count of items in the
    //  collection, and then all of the items in that collection, each separated
    //  by a frame marker.
    //
    TBinMBufOutStream strmOut(&mbufToFill);
    const tCIDLib::TCard4 c4Count = mcolDet.c4ItemCount();
    strmOut << c4Count << (c4Count ^ kCIDLib::c4MaxCard);

    for (tCIDLib::TCard4 c4ItemInd = 0; c4ItemInd < c4Count; c4ItemInd++)
    {
        const TMediaItem& mitemCur = mcolDet.mitemAt(m_mdbInfo, c4ItemInd);
        strmOut << mitemCur
                << tCIDLib::EStreamMarkers::Frame;
    }

    // Cap it off with an end marker
    strmOut << tCIDLib::EStreamMarkers::EndObject;

    // And now flush it and give back the byte count
    strmOut.Flush();
    c4OutBytes = strmOut.c4CurSize();
}


// Returns the location type for a given collection
tCIDLib::TVoid
TCQCStdMediaRepoEng::QueryColLoc(const  TString&            strColCookie
                                ,       tCIDLib::TCard4&    c4OutBytes
                                ,       THeapBuf&           mbufToFill)
{
    // Parse the collection cookie and check the type
    tCIDLib::TCard2         c2CatId;
    tCIDLib::TCard2         c2ColId;
    tCIDLib::TCard2         c2TitleId;
    tCQCMedia::EMediaTypes  eMType;
    eMType = facCQCMedia().eParseColCookie
    (
        strColCookie, c2CatId, c2TitleId, c2ColId
    );
    CheckType(eMType, CID_LINE);

    // Look up the title
    const TMediaTitleSet* pmtsSrc = m_mdbInfo.pmtsById(eMType, c2TitleId, kCIDLib::True);

    // Get the indicated collection out
    const TMediaCollect& mcolSrc = pmtsSrc->mcolAt(m_mdbInfo, c2ColId - 1);

    // And load up the return data
    TBinMBufOutStream strmOut(&mbufToFill);
    strmOut << mcolSrc.strLocInfo()
            << mcolSrc.eLocType();
    strmOut.Flush();
    c4OutBytes = strmOut.c4CurSize();
}


tCIDLib::TVoid
TCQCStdMediaRepoEng::QueryDefCatInfo(tCIDLib::TCard4&   c4IOBytes
                                    , THeapBuf&         mbufIO)
{
    // The incoming info is the media type
    tCQCMedia::EMediaTypes eMType;
    {
        TBinMBufInStream strmIn(&mbufIO, c4IOBytes);
        strmIn >> eMType;
    }

    // Just to be safe, go ahead and zero out the return bytes
    c4IOBytes = 0;

    tCIDLib::TCard2 c2Id;
    TString strCookie;
    TString strName;
    const tCIDLib::TBoolean bRes = facCQCMedia().bDefCategory
    (
        eMType, m_eTypes, strName, strCookie, eMType, c2Id
    );

    // Send back the info
    TBinMBufOutStream strmOut(&mbufIO);
    strmOut << strCookie
            << strName
            << eMType;
    strmOut.Flush();
    c4IOBytes = strmOut.c4CurSize();
}


// Queries info about a specific item
tCIDLib::TVoid
TCQCStdMediaRepoEng::QueryItemDetails(  const   TString&            strItemCookie
                                        ,       tCIDLib::TCard4&    c4OutBytes
                                        ,       THeapBuf&           mbufToFill)
{
    // Parse the cookie
    tCIDLib::TCard2 c2CatId;
    tCIDLib::TCard2 c2ColId;
    tCIDLib::TCard2 c2ItemId;
    tCIDLib::TCard2 c2TitleId;
    const tCQCMedia::EMediaTypes eType = facCQCMedia().eParseItemCookie
    (
        strItemCookie, c2CatId, c2TitleId, c2ColId, c2ItemId
    );
    CheckType(eType, CID_LINE);

    // Find the title set, then the collection and then the item
    const TMediaTitleSet* pmtsSrc = m_mdbInfo.pmtsById(eType, c2TitleId, kCIDLib::True);
    const TMediaCollect& mcolSrc = pmtsSrc->mcolAt(m_mdbInfo, c2ColId - 1);
    const TMediaItem& mitemDet = mcolSrc.mitemAt(m_mdbInfo, c2ItemId - 1);

    // Send back the item object
    TBinMBufOutStream strmOut(&mbufToFill);
    strmOut << mitemDet;
    strmOut.Flush();
    c4OutBytes = strmOut.c4CurSize();
}


//
//  Queries the media format field for a title or collection cookie. This
//  is very simple. Just a cookie in and a string out.
//
tCIDLib::TVoid
TCQCStdMediaRepoEng::QueryMediaFormat(const TString& strCookie, TString& strToFill)
{
    // Parse out the cookie and see what format it is
    tCIDLib::TCard2         c2CatId;
    tCIDLib::TCard2         c2ColId;
    tCIDLib::TCard2         c2ItemId;
    tCIDLib::TCard2         c2TitleId;
    tCQCMedia::EMediaTypes  eMType;

    const tCQCMedia::ECookieTypes eType = facCQCMedia().eCheckCookie
    (
        strCookie, eMType, c2CatId, c2TitleId, c2ColId, c2ItemId
    );

    //
    //  If a title set cookie, take the 0th collection. We assume all
    //  collections of a title set have the same media format.
    //
    tCIDLib::TCard4 c4ColInd;
    if (eType == tCQCMedia::ECookieTypes::Title)
    {
        c4ColInd = 0;
    }
     else
    {
        CIDAssert(c2ColId != 0, L"The collection id was zero");
        c4ColInd = c2ColId - 1;
    }

    const TMediaTitleSet* pmtsSrc = m_mdbInfo.pmtsById
    (
        eMType, c2TitleId, kCIDLib::True
    );

    const TMediaCollect* pmcolCur = m_mdbInfo.pmcolById
    (
        eMType, pmtsSrc->c2ColIdAt(c4ColInd), kCIDLib::False
    );

    strToFill = pmcolCur->strMediaFormat();
}


//
//  We give back up to a given number of randomly selected items from a
//  particular category.
//
tCIDLib::TVoid
TCQCStdMediaRepoEng::QueryRandomCatItems(tCIDLib::TCard4&   c4IOBytes
                                        , THeapBuf&         mbufIO)
{
    // Get the incoming info out
    tCIDLib::TCard4     c4MaxItems;
    TString             strCatCookie;
    {
        TBinMBufInStream strmIn(&mbufIO, c4IOBytes);
        strmIn >> strCatCookie
               >> c4MaxItems;
    }

    // Just to be safe, go ahead and zero out the return bytes
    c4IOBytes = 0;

    // Check the cat cookie
    tCIDLib::TCard2         c2CatId;
    tCIDLib::TCard2         c2ColId;
    tCIDLib::TCard2         c2ItemId;
    tCIDLib::TCard2         c2TitleId;
    tCQCMedia::EMediaTypes  eMType;
    const tCQCMedia::ECookieTypes eType = facCQCMedia().eCheckCookie
    (
        strCatCookie, eMType, c2CatId, c2TitleId, c2ColId, c2ItemId
    );
    CIDAssert(eType == tCQCMedia::ECookieTypes::Cat, L"Expected a category cookie");
    CIDAssert
    (
        eMType == tCQCMedia::EMediaTypes::Music
        , L"Only music categories are legal for random play from category"
    );

    TVector<TCQCMediaPLItem> colOut;
    m_mdbInfo.RandomCatQuery(m_strMoniker, c2CatId, c4MaxItems, colOut);

    // Pack up the return info
    TBinMBufOutStream strmOut(&mbufIO);
    strmOut << colOut.c4ElemCount()
            << colOut;
    strmOut.Flush();
    c4IOBytes = strmOut.c4CurSize();
}


//
//  They want to get a single field of metadata info. The QData indicates which
//  cookie. The QType indicates the field they want.
//
tCIDLib::TVoid
TCQCStdMediaRepoEng::QuerySingleFld(const   TString&    strToGet
                                    , const TString&    strCookie
                                    ,       TString&    strToFill)
{
    // First, let's figure out what type of cookie
    tCIDLib::TCard2         c2CatId;
    tCIDLib::TCard2         c2ColId;
    tCIDLib::TCard2         c2ItemId;
    tCIDLib::TCard2         c2TitleId;
    tCQCMedia::EMediaTypes  eMType;

    const tCQCMedia::ECookieTypes eCType = facCQCMedia().eCheckCookie
    (
        strCookie, eMType, c2CatId, c2TitleId, c2ColId, c2ItemId
    );

    //
    //  If a title cookie, then we need to just use the first collection.
    //  Else it has to be a collection cookie.
    //
    tCIDLib::TCard4 c4ColInd;
    if (eCType == tCQCMedia::ECookieTypes::Title)
    {
        c4ColInd = 0;
    }
     else if (eCType == tCQCMedia::ECookieTypes::Collect)
    {
        CIDAssert(c2ColId != 0, L"The collection id was zero");
        c4ColInd = c2ColId - 1;
    }
     else
    {
        facCQCMedia().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMedErrs::errcDB_BadCookieType
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::BadParms
            , m_strMoniker
        );

        // Won't happen but makes analyzer happy
        return;
    }

    // Ok, let's look up this collection
    const TMediaTitleSet* pmtsSrc = m_mdbInfo.pmtsById(eMType, c2TitleId, kCIDLib::True);
    const TMediaCollect* pmcolCur = m_mdbInfo.pmcolById
    (
        eMType, pmtsSrc->c2ColIdAt(c4ColInd), kCIDLib::True
    );

    // Now, based on the requested info, fill in the caller's parm
    if (strToGet == kCQCMedia::strQuery_Artist)
    {
        strToFill = pmcolCur->strArtist();
    }
     else if (strToGet == kCQCMedia::strQuery_AspectRatio)
    {
        strToFill = pmcolCur->strAspectRatio();
    }
     else if (strToGet == kCQCMedia::strQuery_BitDepth)
    {
        strToFill.AppendFormatted(pmtsSrc->c4BitDepth());
    }
     else if (strToGet == kCQCMedia::strQuery_BitRate)
    {
        strToFill.AppendFormatted(pmtsSrc->c4BitRate());
    }
     else if (strToGet == kCQCMedia::strQuery_Cast)
    {
        strToFill = pmcolCur->strCast();
    }
     else if (strToGet == kCQCMedia::strQuery_Channels)
    {
        strToFill.AppendFormatted(pmtsSrc->c4Channels());
    }
     else if (strToGet == kCQCMedia::strQuery_DateAdded)
    {
        TTime tmFmt(pmtsSrc->enctAdded());
        tmFmt.FormatToStr(strToFill, L"%(m) %(D), %(Y)");
    }
     else if (strToGet == kCQCMedia::strQuery_Description)
    {
        strToFill = pmcolCur->strDescr();
    }
     else if (strToGet == kCQCMedia::strQuery_Label)
    {
        strToFill = pmcolCur->strLabel();
    }
     else if (strToGet == kCQCMedia::strQuery_LeadActor)
    {
        strToFill = pmcolCur->strLeadActor();
    }
     else if (strToGet == kCQCMedia::strQuery_Name)
    {
        strToFill = pmcolCur->strName();
    }
     else if (strToGet == kCQCMedia::strQuery_Rating)
    {
        strToFill = pmcolCur->strRating();
    }
     else if (strToGet == kCQCMedia::strQuery_RawRunTime)
    {
        strToFill.SetFormatted(pmcolCur->c4Duration());
    }
     else if (strToGet == kCQCMedia::strQuery_RunTime)
    {
        const tCIDLib::TCard4 c4Hours = (pmcolCur->c4Duration() / 3600);
        const tCIDLib::TCard4 c4Mins
        (
            (pmcolCur->c4Duration() - (c4Hours * 3600)) / 60
        );

        if (!c4Hours)
        {
            // There's no hours, so format it as xx mins
            strToFill.SetFormatted(c4Mins);
            strToFill.Append(L" mins");
        }
         else
        {
            // We have at least one hours so do H:MM format
            strToFill.SetFormatted(c4Hours);
            strToFill.Append(TLocale::chTimeSeparator());
            if (c4Mins < 10)
                strToFill.Append(kCIDLib::chDigit0);
            strToFill.AppendFormatted(c4Mins);
        }
    }
     else if (strToGet == kCQCMedia::strQuery_SampleRate)
    {
        strToFill.AppendFormatted(pmtsSrc->c4SampleRate());
    }
     else if (strToGet == kCQCMedia::strQuery_SeqNum)
    {
        strToFill.AppendFormatted(pmtsSrc->c4SeqNum());
    }
     else if (strToGet == kCQCMedia::strQuery_Title)
    {
        strToFill = pmtsSrc->strName();

        // If the collection name isn't the same, then append it
        if (pmtsSrc->strName() != pmcolCur->strName())
        {
            strToFill.Append(L" - ");
            strToFill.Append(pmcolCur->strName());
        }
    }
     else if (strToGet == kCQCMedia::strQuery_UserRating)
    {
        strToFill.AppendFormatted(pmtsSrc->c4UserRating());
    }
     else if (strToGet == kCQCMedia::strQuery_Year)
    {
        strToFill.SetFormatted(pmcolCur->c4Year());
    }
     else
    {
        // It's not valid
        facCQCMedia().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMedErrs::errcDB_BadMetaField
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::BadParms
            , strToGet
        );
    }
}


// Queries title details for a given title cookie
tCIDLib::TVoid TCQCStdMediaRepoEng::
QueryTitleDetails(  const   TString&            strTitleCookie
                    ,       tCIDLib::TCard4&    c4OutBytes
                    ,       THeapBuf&           mbufToFill)
{
    // Parse the title cookie
    tCIDLib::TCard2         c2CatId;
    tCIDLib::TCard2         c2TitleId;
    tCQCMedia::EMediaTypes  eType;
    eType = facCQCMedia().eParseTitleCookie(strTitleCookie, c2CatId, c2TitleId);
    CheckType(eType, CID_LINE);

    // And look this title set up
    const TMediaTitleSet* pmtsSrc = m_mdbInfo.pmtsById
    (
        eType, c2TitleId, kCIDLib::True
    );

    // Build up a lists of info we return for each collection
    const tCIDLib::TCard4 c4Count = pmtsSrc->c4ColCount();
    tCQCMedia::TNameList colArtists(c4Count);
    tCQCMedia::TNameList colCookies(c4Count);
    tCQCMedia::TNameList colNames(c4Count);
    TString strCurCookie;
    TString strMediaFmt;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TMediaCollect* pmcolCur = m_mdbInfo.pmcolById
        (
            eType, pmtsSrc->c2ColIdAt(c4Index), kCIDLib::False
        );

        // Get the media format from the first one that's set
        if (strMediaFmt.bIsEmpty())
            strMediaFmt = pmcolCur->strMediaFormat();

        strCurCookie = strTitleCookie;
        strCurCookie.Append(L',');
        strCurCookie.AppendFormatted(c4Index + 1, tCIDLib::ERadices::Hex);
        colCookies.objAdd(strCurCookie);

        if (pmcolCur)
        {
            colArtists.objAdd(pmcolCur->strArtist());
            colNames.objAdd(pmcolCur->strName());
        }
         else
        {
            colArtists.objAdd(m_strMissingCol);
            colNames.objAdd(m_strMissingCol);
        }
    }

    // Pack up the return info
    TBinMBufOutStream strmOut(&mbufToFill);
    strmOut << pmtsSrc->strName()
            << pmtsSrc->strArtist()
            << pmtsSrc->c4Year()
            << strMediaFmt
            << c4Count
            << colNames
            << colCookies
            << colArtists
            << eType;
    strmOut.Flush();
    c4OutBytes = strmOut.c4CurSize();
}


//
//  So that the derived driver can call us to reset the DB for another load,
//  or give us a new database to steal the contents of. Most drivers will
//  do a load asynchronously and then lock and swap in the new database,
//  so that we don't keep the driver locked up during a load.
//
tCIDLib::TVoid TCQCStdMediaRepoEng::ResetMediaDB()
{
    m_mdbInfo.Reset();
}

tCIDLib::TVoid TCQCStdMediaRepoEng::ResetMediaDB(TMediaDB& mdbToLoad)
{
    //
    //  Tell our database to adopt the contents of the passed database. This
    //  make it efficient since we don't copy them we just steal them.
    //
    m_mdbInfo.TakeFrom(mdbToLoad);

    // Now initialize all the objects that require it
    m_mdbInfo.LoadComplete();
}

