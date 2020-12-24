//
// FILE NAME: AutonomicS_DriverImpl2.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/07/2007
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
//  This just contains some grunt work methods from the main implementation
//  file, to keep it from getting too big. These are related to media meta
//  data retrieval.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "AutonomicS.hpp"



// ---------------------------------------------------------------------------
//  TAutonomicSDriver: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  This is called from the loader thread, to query the genres from the
//  MCE server and load them into our database.
//
tCIDLib::TBoolean TAutonomicSDriver::bLoadGenres()
{
    //
    //  Now we loop until we get the EndGenres message, processing any
    //  asyncs that come along while we wait.
    //
    tCIDLib::TEncodedTime enctCur = TTime::enctNow();
    tCIDLib::TEncodedTime enctEnd = enctCur + (kCIDLib::enctOneSecond * 5);

    tCIDLib::TCard4 c4GIndex = 1;
    TString strMsg;
    TString strName;
    while (enctCur < enctEnd)
    {
        // Send a message asking for up to 20 genres from the last point
        strMsg = m_strMsg_BrowseGenres;
        strMsg.Append(L' ');
        strMsg.AppendFormatted(c4GIndex);
        strMsg.Append(L" 20");
        if (!bSendAndWaitForRep(strMsg, m_strMsg_BeginGenres, 2500, strMsg))
            return kCIDLib::False;

        //
        //  And pull in any that are returned. We need to pull the count
        //  off the end of the reply.
        //
        strMsg.Cut(0, m_strMsg_BeginGenres.c4Length());
        const tCIDLib::TCard4 c4Count = strMsg.c4Val(tCIDLib::ERadices::Dec);
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            if (bGetMsg(strMsg, enctEnd))
            {
                if (!strMsg.bStartsWith(m_strMsg_Genre))
                {
                    bProcessMsg(strMsg);
                    continue;
                }

                // Strip off the prefix and following space
                strMsg.Cut(0, m_strMsg_Genre.c4Length() + 1);

                //
                //  Now we have a GUID and a quoted genre name. Split it
                //  on the space between them, then remove the quotes from
                //  the name.
                //
                if (strMsg.bSplit(strName, kCIDLib::chSpace))
                {
                    if (!strName.bIsEmpty())
                        strName.Cut(0, 1);
                    if (!strName.bIsEmpty())
                        strName.DeleteLast();
                }

                //
                //  Ok, add this genre to our database. We store the GUI
                //  id as the unique id of the genre.
                //
                TMediaCat* pmcatNew = new TMediaCat
                (
                    tCQCMedia::EMediaTypes::Music
                    , strName
                    , m_c2NextId_Cat++
                );
                pmcatNew->strUniqueId(strMsg);
                m_mdbLoad.AddCategory(pmcatNew);
            }
        }

        // Add the count to our running index
        c4GIndex += c4Count;

        // Wait for the terminator
        if (!bWaitForReply(m_strMsg_EndGenres, 2500, strMsg))
        {
            if (eVerboseLevel() > tCQCKit::EVerboseLvls::Medium)
            {
                facAutonomicS().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , L"No end genre list response. Start index=%(1)"
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::AppStatus
                    , TCardinal(c4GIndex)
                );
            }
            return kCIDLib::False;
        }

        // Get the more/nomore indicator and break out if no more
        strMsg.Cut(0, m_strMsg_EndGenres.c4Length());
        if (strMsg != m_strMsg_More)
            break;
    }
    return kCIDLib::True;
}


//
//  This is called from the loader thread, to query the titles from the
//  MCE server and load them into our database. This is pathetically
//  inefficient, because we cannot just ask for the list of titles and
//  what genres each one is in. We have to set a filter for each genre
//  and query the titles in that genre. If a title is in 5 genres, we'll
//  get it five times.
//
tCIDLib::TBoolean TAutonomicSDriver::bLoadTitles()
{
    //
    //  Let's iterate the genres that we previous asked for, so that we
    //  can set each one as a filter.
    //
    const tCQCMedia::TCatList& colCats = m_mdbLoad.colCatsForType
    (
        tCQCMedia::EMediaTypes::Music
    );

    tCIDLib::TBoolean bRes = kCIDLib::True;
    TString strMsg;
    const tCIDLib::TCard4 c4CatCnt = colCats.c4ElemCount();
    for (tCIDLib::TCard4 c4CatIndex = 0; c4CatIndex < c4CatCnt; c4CatIndex++)
    {
        const TMediaCat& mcatCur = *colCats[c4CatIndex];

        // Set a set filter message for the current genre
        strMsg = m_strMsg_SetMediaFilter;
        strMsg.Append(L" Clear");
        if (!bSendAndWaitForRep(strMsg, m_strMsg_ClearMusicFilter, 2500, strMsg))
        {
            bRes = kCIDLib::False;
            break;
        }

        strMsg = m_strMsg_SetMediaFilter;
        strMsg.Append(L" Genre=");
        strMsg.Append(mcatCur.strUniqueId());
        if (!bSendAndWaitForRep(strMsg, m_strMsg_MusicFilter, 2500, strMsg))
        {
            bRes = kCIDLib::False;
            break;
        }

        //
        //  Now we query albums, which should get us only those titles
        //  that are in the current genre.
        //
        tCIDLib::TCard4 c4AIndex = 1;

        TString strArtistFN;
        TString strArtistLN;
        TString strGUID;
        TString strTitle;
        TString strSortTitle;
        tCIDLib::TEncodedTime enctCur = TTime::enctNow();
        tCIDLib::TEncodedTime enctEnd = enctCur + (kCIDLib::enctOneSecond * 10);
        while (enctCur < enctEnd)
        {
            // Send a message asking for up to 50 albums from the last point
            strMsg = m_strMsg_BrowseAlbums;
            strMsg.Append(L' ');
            strMsg.AppendFormatted(c4AIndex);
            strMsg.Append(L" 50");
            if (!bSendAndWaitForRep(strMsg, m_strMsg_BeginAlbums, 3000, strMsg))
                return kCIDLib::False;

            strMsg.Cut(0, m_strMsg_BeginAlbums.c4Length());
            const tCIDLib::TCard4 c4Count = strMsg.c4Val(tCIDLib::ERadices::Dec);
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            {
                if (bGetMsg(strMsg, enctEnd))
                {
                    if (!strMsg.bStartsWith(m_strMsg_Album))
                    {
                        bProcessMsg(strMsg);
                        continue;
                    }

                    bParseAlbumText
                    (
                        strMsg
                        , strGUID
                        , strTitle
                        , strSortTitle
                        , strArtistFN
                        , strArtistLN
                    );

                    //
                    //  If this title isn't already in our database, then
                    //  we need to add it.
                    //
                    TMediaTitleSet* pmtsNew = m_mdbLoad.pmtsByUniqueId
                    (
                        tCQCMedia::EMediaTypes::Music, strMsg, kCIDLib::False
                    );

                    if (!pmtsNew)
                    {
                        //
                        //  Create a media image object for this guy. We
                        //  set the art path to be the GUID. We give it
                        //  the same id as the title id since they are
                        //  one to one.
                        //
                        TMediaImg* pmimgNew = new TMediaImg
                        (
                            tCQCMedia::EMediaTypes::Music
                            , m_c2NextId_Title
                            , strGUID
                        );
                        m_mdbLoad.AddImage(pmimgNew);

                        // Create a sort tile for this guy
                        pmtsNew = new TMediaTitleSet
                        (
                            strTitle
                            , strGUID
                            , strSortTitle
                            , m_c2NextId_Title
                            , tCQCMedia::EMediaTypes::Music
                        );
                        pmtsNew->SetArtist(strArtistFN, strArtistLN);
                        m_mdbLoad.AddTitle(pmtsNew);

                        //
                        //  And we need to create a single collection to
                        //  it. We don't get the info required to do multi-
                        //  collection sets, so they are always one to one.
                        //  We use the same unique id for the collection as
                        //  we got for the title set, and the same numeric
                        //  id since they are one to one.
                        //
                        //  We do a faux location as collection level file
                        //  with the GUID being the 'path'.
                        //
                        TMediaCollect* pmcolNew = new TMediaCollect
                        (
                            strTitle
                            , strGUID
                            , m_c2NextId_Title
                            , 0
                            , tCQCMedia::ELocTypes::FileCol
                            , tCQCMedia::EMediaTypes::Music
                        );
                        TJanitor<TMediaCollect> janCol(pmcolNew);

                        pmcolNew->SetArtist(strArtistFN, strArtistLN);
                        pmcolNew->c2ArtId(pmimgNew->c2Id());
                        pmcolNew->SetLocation
                        (
                            tCQCMedia::ELocTypes::FileCol, strGUID
                        );
                        pmcolNew->bAddCategory(mcatCur);
                        m_mdbLoad.AddCollect(pmcolNew);

                        // Add a ref to this collection to the parent set
                        pmtsNew->bAddCollect(*janCol.pobjOrphan());

                        // And move up to the next title/col id
                        m_c2NextId_Title++;
                    }
                     else
                    {
                        //
                        //  Just add the current category as one of the
                        //  cats this title's one collection belongs to.
                        //
                        TMediaCollect& mcolCur = m_mdbLoad.mcolAt
                        (
                            tCQCMedia::EMediaTypes::Music, 0
                        );
                        mcolCur.bAddCategory(mcatCur);
                    }
                }
            }

            // Add the count to our running index
            c4AIndex += c4Count;

            // Wait for the terminator
            if (!bWaitForReply(m_strMsg_EndAlbums, 2500, strMsg))
            {
                if (eVerboseLevel() > tCQCKit::EVerboseLvls::Medium)
                {
                    facAutonomicS().LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , L"No end title list response. Start index=%(1)"
                        , tCIDLib::ESeverities::Status
                        , tCIDLib::EErrClasses::AppStatus
                        , TCardinal(c4AIndex)
                    );
                }
                return kCIDLib::False;
            }

            // Get the more/nomore indicator and break out if no more
            strMsg.Cut(0, m_strMsg_EndGenres.c4Length());
            if (strMsg != m_strMsg_More)
                break;
        }
    }

    // Clear the final filter if any
    strMsg = m_strMsg_SetMediaFilter;
    strMsg.Append(L" Clear");
    if (!bSendAndWaitForRep(strMsg, m_strMsg_ClearMusicFilter, 2500, strMsg))
        bRes = kCIDLib::False;

    return bRes;
}


//
//  Given the string we get for each album, this guy parses out the info
//  to keep the main title loading code simpler.
//
tCIDLib::TBoolean
TAutonomicSDriver::bParseAlbumText( const   TString&    strToParse
                                    ,       TString&    strGUID
                                    ,       TString&    strTitle
                                    ,       TString&    strSortTitle
                                    ,       TString&    strArtistFN
                                    ,       TString&    strArtistLN)
{
    // Use a tokenizer with space as the separator
    TStringTokenizer stokParse(&strToParse, L" ");

    // Skip the first token, which is the prefix
    if (!stokParse.bEatNextToken())
        return kCIDLib::False;

    // The next one should be the GUID
    if (!stokParse.bGetNextToken(strGUID))
        return kCIDLib::False;

    //
    //  And the remainder is the title text, with quotes around it, so
    //  just take the remainder and remove the quotes.
    //
    if (!stokParse.bGetRestOfLine(strTitle))
        return kCIDLib::False;

    if (strTitle[0] == kCIDLib::chQuotation)
        strTitle.Cut(0, 1);
    if (strTitle.chLast() == kCIDLib::chQuotation)
        strTitle.DeleteLast();

    //
    //  And it may have the artist name in parens at the end. So check for
    //  a trailing paren. If so, then find the opening paren, and let's pull
    //  that out
    //
    if (strTitle.chLast() == kCIDLib::chCloseParen)
    {
        // Find the last opening paren
        tCIDLib::TCard4 c4Ofs;
        if (strTitle.bLastOccurrence(kCIDLib::chOpenParen, c4Ofs))
        {
            TString strTmp;
            strTitle.CopyOutSubStr(strTmp, c4Ofs + 1);
            strTmp.DeleteLast();

            // Break this out into the standard f/l name form
            facCQCMedia().BreakOutName(strTmp, strArtistFN, strArtistLN);

            // And cut the artist name out of the title
            strTitle.CapAt(c4Ofs);
        }
    }
     else
    {
        strArtistFN.Clear();
        strArtistLN.Clear();
    }

    // Create the sort title from the title
    facCQCMedia().bMakeSortTitle(strTitle, strSortTitle);

    return kCIDLib::True;
}


//
//  This method is started up as a thread, to do our loading of the media
//  metadata asynchronously.
//
tCIDLib::EExitCodes
TAutonomicSDriver::eLoaderThread(TThread& thrThis, tCIDLib::TVoid*)
{
    // Update the status before we let the caller go
    m_eConnStatus = tAutonomicS::EConnStatus_Loading;

    // Let the calling thread go
    thrThis.Sync();

    try
    {
        // Flush our temp database in preperation for reloading
        m_mdbLoad.Reset();

        // Set up our next id values so we can assign ids
        m_c2NextId_Cat = 1;
        m_c2NextId_Title = 1;

        // First we need to get the genres
        if (!bLoadGenres())
        {
            m_eConnStatus = tAutonomicS::EConnStatus_Failed;
            return tCIDLib::EExitCodes::Normal;
        }

        // And load the titles for each genre
        if (!bLoadTitles())
        {
            m_eConnStatus = tAutonomicS::EConnStatus_Failed;
            return tCIDLib::EExitCodes::Normal;
        }

        // Complete the load on our temp database now
        m_mdbLoad.LoadComplete();

        // Ok, it worked
        m_eConnStatus = tAutonomicS::EConnStatus_Loaded;
    }

    catch(const tAutonomicS::EConnStatus)
    {
        // We were asked to shutdown, so just fail it and return
        m_eConnStatus = tAutonomicS::EConnStatus_Failed;
    }

    catch(...)
    {
        m_eConnStatus = tAutonomicS::EConnStatus_Failed;
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
        {
            facCQCMedia().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kMedMsgs::midStatus_ExceptInLoad
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );
        }
    }

    //
    //  Make sure that the status didn't end up stuck on loading somehow,
    //  since that would make the driver wait forever.
    //
    if ((m_eConnStatus != tAutonomicS::EConnStatus_Failed)
    &&  (m_eConnStatus != tAutonomicS::EConnStatus_Loaded))
    {
        facAutonomicS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kAnomicErrs::errcDB_LoadStatusFailed
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::Internal
        );
        m_eConnStatus = tAutonomicS::EConnStatus_Failed;
    }

    return tCIDLib::EExitCodes::Normal;
}


