//
// FILE NAME: CQCMedia_WMPMetaSrc.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/08/2006
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
//  This file implements a wrapper around the Windows Media Player SDK, for
//  getting media metadata via WMP.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


// ---------------------------------------------------------------------------
//  Facility specific includes
// ---------------------------------------------------------------------------
#include    "CQCMedia_.hpp"


// ---------------------------------------------------------------------------
//  Bring in the WMP SDK header and a platform specific helper header
// ---------------------------------------------------------------------------
#include    <wmp.h>
#include    "CIDKernel_InternalHelpers_.hpp"


// ---------------------------------------------------------------------------
//  Magic RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCWMPMetaSrc,TObject)


// ---------------------------------------------------------------------------
//  Local types and data
// ---------------------------------------------------------------------------
namespace CQCMedia_WMPMetaSrc
{
    namespace
    {
        // -----------------------------------------------------------------------
        //  The internal format of our instance data, which we hide from the
        //  outside world.
        // -----------------------------------------------------------------------
        struct TObjData
        {
            IWMPCore3*  pPlayer;
        };
    }
}



// ---------------------------------------------------------------------------
//  CLASS: TCQCWMPMetaSrc
// PREFIX: cwmp
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCWMPMetaSrc: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCWMPMetaSrc::TCQCWMPMetaSrc() :

    m_pData(0)
{
}

TCQCWMPMetaSrc::~TCQCWMPMetaSrc()
{
    // Clean up the data we created
    CQCMedia_WMPMetaSrc::TObjData* pData = static_cast<CQCMedia_WMPMetaSrc::TObjData*>(m_pData);
    if (pData)
    {
        // Close any existing media file
        if (pData->pPlayer)
            pData->pPlayer->close();
        SafeCOMFree(pData->pPlayer);
    }
}


// ---------------------------------------------------------------------------
//  TCQCWMPMetaSrc: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  After calling bStartLoad(), the caller should use a timer to periodically
//  call back into this method until it reports that the data is available,
//  or the caller has waited as long as it can wait and gives up.
//
tCIDLib::TBoolean
TCQCWMPMetaSrc::bCompleteLoad(  const   TString&                strPath
                                ,       TMediaCollect&          mcolToFill
                                ,       TJPEGImage&
                                ,       tCQCMedia::TItemList&   colItems
                                ,       TVector<TString>&       colCatNames
                                ,       TString&                strInfoURL)
{
    // If we've not faulted in our data yet, that's an error
    if (!m_pData)
    {
        facCQCMedia().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMedErrs::errcWMP_NotInitialized
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::CantDo
        );
    }

    // Clear the incoming to defaults
    colItems.RemoveAll();
    mcolToFill.Reset();

    // Get our instance data, cast to the real type
    CQCMedia_WMPMetaSrc::TObjData* pData
    (
        static_cast<CQCMedia_WMPMetaSrc::TObjData*>(m_pData)
    );

    // Get the playlist from the player
    IWMPPlaylist* pNPList = 0;
    HRESULT hRes = pData->pPlayer->get_currentPlaylist(&pNPList);
    if (FAILED(hRes))
    {
        facCQCMedia().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMedErrs::errcWMP_GetPlayList
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::CantDo
            , strPath
            , TCardinal(hRes, tCIDLib::ERadices::Hex)
        );
    }
    TCOMJanitor<IWMPPlaylist> janNPList(&pNPList);

    // Some local strings and BSTRs for our use
    TBSTRJanitor janTmpStr1(kCIDLib::pszEmptyZStr);
    TBSTRJanitor janTmpStr2(kCIDLib::pszEmptyZStr);
    TString strTmp;

    // A convenient flag for below if we have to distinguish between types
    const tCIDLib::TBoolean bIsMusic(mcolToFill.eType() == tCQCMedia::EMediaTypes::Music);

    // See if metadata has shown up. If not, return false
    pNPList->get_name(&janTmpStr1.bsData);

    if (bIsMusic)
    {
        if (TRawStr::bCompareStrNI(janTmpStr1.pszData(), L"Unknown Album", 13))
            return kCIDLib::False;
    }
     else
    {
        if (TRawStr::bCompareStrNI(janTmpStr1.pszData(), L"Unknown DVD", 11))
            return kCIDLib::False;
    }

    //
    //  Get the overall name and set that as the collection name. We'll look
    //  for any [WS] or [FS] or other common suffixes on it and remove it,
    //  else we won't get a good cover art lookup later. We just look for the
    //  first [ and cap there.
    //
    //  We also look for things like " Disc 1" and remove that, using a regular
    //  expression.
    //
    pNPList->get_name(&janTmpStr1.bsData);
    strTmp = janTmpStr1.pszData();

    tCIDLib::TCard4 c4Ofs;
    if (strTmp.bFirstOccurrence(L'[', c4Ofs))
        strTmp.CapAt(c4Ofs);

    TRegEx regxSuffix;
    regxSuffix.SetExpression(L".+ Disc *[0-9]+$");
    if (regxSuffix.bFullyMatches(strTmp, kCIDLib::False))
    {
        if (strTmp.bLastOccurrence(L" Disc", c4Ofs, kCIDLib::False, kCIDLib::False))
            strTmp.CapAt(c4Ofs);
    }

    strTmp.StripWhitespace();
    mcolToFill.strName(strTmp);

    //
    //  We need to build up the info required to create the collection.
    //  We need the name (title), duration, track/chapter count, and
    //  year. So get the number of attributes and iterate through them,
    //  pulling out the ones of interest.
    //
    //  For whatever reason, we seem to never get any duration info,
    //  though we'll look for it just in case.
    //
    tCIDLib::TCard4 c4Duration = 0;
    tCIDLib::TCard4 c4Year = 0;
    TString         strArtist;
    TString         strFName;
    TString         strLName;
    TString         strLabel;
    TString         strRating;
    TString         strName;

    long lAttrCnt;
    hRes = pNPList->get_attributeCount(&lAttrCnt);
    if (FAILED(hRes))
    {
        facCQCMedia().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMedErrs::errcWMP_GetCount
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::CantDo
            , TString(L"collection attribute")
            , TCardinal(hRes, tCIDLib::ERadices::Hex)
        );
    }

    for (long lIndex = 0; lIndex < lAttrCnt; lIndex++)
    {
        hRes = pNPList->get_attributeName(lIndex, &janTmpStr1.bsData);
        if (FAILED(hRes))
        {
            if (facCQCMedia().bLogWarnings())
            {
                facCQCMedia().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kMedErrs::errcWMP_GetMediaAttrName
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::CantDo
                    , TString(L"collection")
                    , TCardinal(hRes, tCIDLib::ERadices::Hex)
                    , TCardinal64(lIndex)
                );
            }
            continue;
        }

        hRes = pNPList->getItemInfo(janTmpStr1.bsData,  &janTmpStr2.bsData);
        if (FAILED(hRes))
        {
            if (facCQCMedia().bLogWarnings())
            {
                facCQCMedia().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kMedErrs::errcWMP_GetMediaAttr
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::CantDo
                    , TString(L"collection")
                    , TCardinal(hRes, tCIDLib::ERadices::Hex)
                    , TString(janTmpStr1.bsData)
                );
            }
            continue;
        }

        // See if this is a collection level item we care about
        if (TRawStr::bCompareStrI(janTmpStr1.pszData(), L"Artist")
        ||  TRawStr::bCompareStrI(janTmpStr1.pszData(), L"Director"))
        {
            strArtist = janTmpStr2.pszData();
            if ((strArtist == L"Unknown Director")
            ||  (strArtist == L"Unknown Artist"))
            {
                strArtist.Clear();
            }
        }
         else if (TRawStr::bCompareStrI(janTmpStr1.pszData(), L"Genre"))
        {
            //
            //  Translate some dumb ones into better ones. Else, take it
            //  as is.
            //
            const tCIDLib::TCh* pszGenre = janTmpStr2.pszData();
            if (TRawStr::bCompareStr(pszGenre, L"Comedy Drama"))
            {
                colCatNames.objAdd(TString(L"Comedy"));
                colCatNames.objAdd(TString(L"Drama"));
            }
             else if (TRawStr::bCompareStr(pszGenre, L"Music [nf]"))
            {
                colCatNames.objAdd(TString(L"Music"));
            }
             else
            {
                colCatNames.objAdd(TString(pszGenre));
            }
        }
         else if (TRawStr::bCompareStrI(janTmpStr1.pszData(), L"Label")
              ||  TRawStr::bCompareStrI(janTmpStr1.pszData(), L"Studio"))
        {
            strLabel = janTmpStr2.pszData();
        }
         else if (TRawStr::bCompareStrI(janTmpStr1.pszData(), L"MPAARating"))
        {
            strRating = janTmpStr2.pszData();
        }
         else if (TRawStr::bCompareStrI(janTmpStr1.pszData(), L"MoreInfo"))
        {
            strInfoURL = janTmpStr2.pszData();
        }
         else if (TRawStr::bCompareStrI(janTmpStr1.pszData(), L"Duration"))
        {
            tCIDLib::TBoolean bOk;
            const tCIDLib::TCard8 c8Dur = TRawStr::c8AsBinary
            (
                janTmpStr2.pszData(), bOk, tCIDLib::ERadices::Dec
            );

            if (c8Dur)
                c4Duration = tCIDLib::TCard4(c8Dur / kCIDLib::enctOneSecond);
            else
                c4Duration = 0;
        }
         else if (TRawStr::bCompareStrI(janTmpStr1.pszData(), L"ReleaseDate"))
        {
            // This is in the form YYYY-MM-DD, so just pull out the year
            janTmpStr2.pszData()[4] = kCIDLib::chNull;
            tCIDLib::TBoolean bOk;
            c4Year = TRawStr::c4AsBinary
            (
                janTmpStr2.pszData(), bOk, tCIDLib::ERadices::Dec
            );
        }
    }

    // Store the non-media type specific stuff
    mcolToFill.c4Year(c4Year);
    mcolToFill.strLabel(strLabel);

    //
    //  Unique ids are provided by the metadata source since they may need
    //  to map back to something in the source meta database. In our case,
    //  we just generate a random one.
    //
    if (bIsMusic)
    {
        // Get the number of media items (tracks)
        long lCount;
        hRes = pNPList->get_count(&lCount);
        if (FAILED(hRes))
        {
            if (facCQCMedia().bLogWarnings())
            {
                facCQCMedia().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kMedErrs::errcWMP_GetCount
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::CantDo
                    , TString(L"track")
                    , TCardinal(hRes, tCIDLib::ERadices::Hex)
                );
            }
            lCount = 0;
        }

        tCIDLib::TBoolean bSameArtist = kCIDLib::True;
        TString strPrevArtist;
        for (long lIndex = 0; lIndex < lCount; lIndex++)
        {
            // Look up the current media item (track or chapter)
            c4Year = 0;
            {
                IWMPMedia* pMedia = 0;
                pNPList->get_item(lIndex, &pMedia);
                TCOMJanitor<IWMPMedia> janMedia(&pMedia);

                // Get the number of attributes
                hRes = pNPList->get_count(&lAttrCnt);
                if (FAILED(hRes))
                {
                    if (facCQCMedia().bLogWarnings())
                    {
                        facCQCMedia().LogMsg
                            (
                            CID_FILE
                            , CID_LINE
                            , kMedErrs::errcWMP_GetCount
                            , tCIDLib::ESeverities::Failed
                            , tCIDLib::EErrClasses::CantDo
                            , TString(L"track attribute")
                            , TCardinal(hRes, tCIDLib::ERadices::Hex)
                            );
                    }
                    continue;
                }

                strArtist.Clear();
                strName.Clear();
                for (long lAttrInd = 0; lAttrInd < lAttrCnt; lAttrInd++)
                {
                    hRes = pMedia->getAttributeName(lAttrInd, &janTmpStr1.bsData);
                    if (FAILED(hRes))
                    {
                        if (facCQCMedia().bLogWarnings())
                        {
                            facCQCMedia().LogMsg
                                (
                                CID_FILE
                                , CID_LINE
                                , kMedErrs::errcWMP_GetMediaAttrName
                                , tCIDLib::ESeverities::Failed
                                , tCIDLib::EErrClasses::CantDo
                                , TString(L"track")
                                , TCardinal(hRes, tCIDLib::ERadices::Hex)
                                , TCardinal64(lAttrInd)
                                );
                        }
                        continue;
                    }

                    hRes = pMedia->getItemInfo(janTmpStr1.bsData, &janTmpStr2.bsData);
                    if (FAILED(hRes))
                    {
                        if (facCQCMedia().bLogWarnings())
                        {
                            facCQCMedia().LogMsg
                                (
                                CID_FILE
                                , CID_LINE
                                , kMedErrs::errcWMP_GetMediaAttr
                                , tCIDLib::ESeverities::Failed
                                , tCIDLib::EErrClasses::CantDo
                                , TString(L"track")
                                , TCardinal(hRes, tCIDLib::ERadices::Hex)
                                , TString(janTmpStr1.bsData)
                                );
                        }
                        continue;
                    }

                    if (TRawStr::bCompareStrI(janTmpStr1.pszData(), L"Artist"))
                    {
                        // If not set already (probably via AlbumArtist), then store
                        if (strArtist.bIsEmpty())
                            strArtist = janTmpStr2.pszData();
                    }
                     else if (TRawStr::bCompareStrI(janTmpStr1.pszData(), L"Caption"))
                    {
                        strName = janTmpStr2.pszData();
                    }
                     else if (TRawStr::bCompareStrI(janTmpStr1.pszData(), L"ReleaseDate"))
                    {
                        // This is in the form YYYY-MM-DD, so just pull out the year
                        janTmpStr2.pszData()[4] = kCIDLib::chNull;
                        tCIDLib::TBoolean bOk;
                        c4Year = TRawStr::c4AsBinary(janTmpStr2.pszData(), bOk, tCIDLib::ERadices::Dec);
                    }

                    //
                    //  If we get an album artist, override any regular artist we might have stored above,
                    //  if it's not empty anyway.
                    //
                    if (TRawStr::bCompareStrI(janTmpStr1.pszData(), L"AlbumArtist"))
                    {
                        if (TRawStr::c4StrLen(janTmpStr2.pszData()))
                            strArtist = janTmpStr2.pszData();
                    }
                }

                // If no name or artist, fake one so that we have something
                if (strName.bIsEmpty())
                {
                    strName = L"Track ";
                    strName.AppendFormatted(tCIDLib::TCard4(lIndex + 1));
                }

                if (strArtist.bIsEmpty())
                    strArtist = L"Unknown Artist";
            }

            // If the year is zero, take the collection year
            if (!c4Year)
                c4Year = mcolToFill.c4Year();

            //
            //  We don't know loc, or duration info. That will get filled
            //  in later.
            //
            TMediaItem mitemCur
            (
                strName
                , TUniqueId::strMakeId()
                , TString::strEmpty()
                , tCQCMedia::EMediaTypes::Music
            );
            mitemCur.strArtist(strArtist);
            colItems.objAdd(mitemCur);

            // Keep up with whether all the tracks are by the same artist
            if (lIndex && (strPrevArtist != strArtist))
                bSameArtist = kCIDLib::False;
            strPrevArtist = strArtist;
        }

        if (bSameArtist)
        {
            mcolToFill.strArtist(strArtist);
        }
         else
        {
            //
            //  To make the sorting come out right, set no first name and
            //  set the last name to Various Artists. Otherwise, it'll take
            //  Artists as the last name and sort on that first.
            //
            mcolToFill.strArtist(L"Various Artists");
        }
    }
     else
    {
        // Just add the single place holder item
        colItems.objAdd
        (
            TMediaItem
            (
                L"Movie"
                , TUniqueId::strMakeId()
                , TString::strEmpty()
                , tCQCMedia::EMediaTypes::Movie
            )
        );

        //
        //  Translate the rating into the standard form used by CQC, so
        //  that every repository uses the same rating (important for
        //  things like using the media image widget to show ratings images.)
        //
        if (strRating == L"PG13")
            strRating = L"PG-13";
        else if (strRating == L"NC17")
            strRating = L"NC-17";
        mcolToFill.strRating(strRating);

        // And store the artist if we got one
        if (!strArtist.bIsEmpty())
            mcolToFill.strArtist(strArtist);
    }

    return kCIDLib::True;
}


//
//  This method starts a metadata download. Once started successfully, the
//  caller should periodically call bCompleteLoad() until the data is
//  ready, or they decide they want to give up.
//
tCIDLib::TVoid TCQCWMPMetaSrc::StartLoad(const TString& strPath)
{
    // If we've not faulted in our data yet, then do it
    HRESULT hRes;
    if (!m_pData)
    {
        CQCMedia_WMPMetaSrc::TObjData* pData = new CQCMedia_WMPMetaSrc::TObjData;
        pData->pPlayer = 0;

        // The class id for the WMP player interfaces
        const CLSID CLSID_WMP = {0x6BF52A52,0x394a,0x11d3,{0xb1,0x53,0x00,0xc0,0x4f,0x79,0xfa,0xa6}};

        // The ref id for the core interface we use
        const IID IID_WMPCore3 = {0x7587C667,0x628F,0x499F,{0x88,0xE7,0x6A,0x6F,0x4E,0x88,0x84,0x64}};

        // Create the base player core interface
        hRes = ::CoCreateInstance
        (
            CLSID_WMP, NULL, CLSCTX_ALL, IID_WMPCore3, tCIDLib::pToVoidPP(&pData->pPlayer)
        );
        if (FAILED(hRes))
        {
            delete pData;
            facCQCMedia().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kMedErrs::errcWMP_CreateFailed
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::CantDo
                , TCardinal(hRes, tCIDLib::ERadices::Hex)
                , TString(L"player")
            );
        }

        // It worked so store it
        m_pData = pData;
    }

    TBSTRJanitor janTmpStr1(kCIDLib::pszEmptyZStr);

    // Get our instance data, cast to the real type
    CQCMedia_WMPMetaSrc::TObjData* pData
    (
        static_cast<CQCMedia_WMPMetaSrc::TObjData*>(m_pData)
    );

    // Get the CD drive collection and try to find our drive
    IWMPCdromCollection* pCDCol = 0;
    hRes = pData->pPlayer->get_cdromCollection(&pCDCol);
    if (FAILED(hRes))
    {
        facCQCMedia().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMedErrs::errcWMP_GetCDCol
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::CantDo
            , TCardinal(hRes, tCIDLib::ERadices::Hex)
        );
    }
    TCOMJanitor<IWMPCdromCollection> janCDCol(&pCDCol);

    // See if he has the driver we want
    tCIDLib::TCh szDrv[3];
    szDrv[0] = strPath[0];
    szDrv[1] = kCIDLib::chColon;
    szDrv[2] = kCIDLib::chNull;
    TBSTRJanitor janDrv(szDrv);

    long lDrvInd = 0;
    long lDrvCnt = 0;
    hRes = pCDCol->get_count(&lDrvCnt);
    IWMPCdrom* pCDDrv = 0;
    for (lDrvInd = 0; lDrvInd < lDrvCnt; lDrvInd++)
    {
        hRes = pCDCol->item(lDrvInd, &pCDDrv);
        if (FAILED(hRes))
        {
            facCQCMedia().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kMedErrs::errcWMP_DriveNotFound
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::CantDo
                , strPath
                , TCardinal(hRes, tCIDLib::ERadices::Hex)
            );
        }

        pCDDrv->get_driveSpecifier(&janTmpStr1.bsData);
        if (TRawStr::bCompareStr(szDrv, janTmpStr1.pszData()))
            break;
        SafeCOMFree(pCDDrv);
    }
    TCOMJanitor<IWMPCdrom> janCDDrv(&pCDDrv);

    // If we didnt' find the drive, then give up
    if (!pCDDrv)
    {
        facCQCMedia().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMedErrs::errcWMP_DriveNotFound
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::CantDo
            , strPath
            , TCardinal(hRes, tCIDLib::ERadices::Hex)
        );
    }

    // He does have it, so let's get the play list from this guy
    {
        IWMPPlaylist* pPList = 0;
        hRes = pCDDrv->get_playlist(&pPList);
        if (FAILED(hRes))
        {
            facCQCMedia().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kMedErrs::errcWMP_GetPlayList
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::CantDo
                , strPath
                , TCardinal(hRes, tCIDLib::ERadices::Hex)
            );
        }
        TCOMJanitor<IWMPPlaylist> janPList(&pPList);

        // Disable auto-play
        IWMPSettings* pSettings = 0;
        hRes = pData->pPlayer->get_settings(&pSettings);
        if (FAILED(hRes))
        {
        }
        TCOMJanitor<IWMPSettings> janSettings(&pSettings);
        VARIANT_BOOL fAutoStart(0);
        hRes = pSettings->put_autoStart(fAutoStart);
        if (FAILED(hRes))
        {
        }

        //
        //  Set this as the current playlist so that metadata will be available.
        //  If this is a DVD, then we need to actually pull the first item out
        //  (which represents the overall DVD) and set it.
        //
        hRes = pData->pPlayer->put_currentPlaylist(pPList);
        if (FAILED(hRes))
        {
            facCQCMedia().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kMedErrs::errcWMP_SetPlayList
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::CantDo
                , strPath
                , TCardinal(hRes, tCIDLib::ERadices::Hex)
            );
        }
    }
}


