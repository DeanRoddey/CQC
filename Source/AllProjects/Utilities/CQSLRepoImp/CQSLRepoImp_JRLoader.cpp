//
// FILE NAME: CQSLRepoImp_JRLoader.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/02/2007
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
//  This file implements the J.River import
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQSLRepoImp.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQSLImpJRLoader,TObject)


namespace JRLoader
{
    // -----------------------------------------------------------------------
    //  The names of some things we look for in the XML
    // -----------------------------------------------------------------------
    const TString   strAttr_Name("Name");

    const TString   strElem_Field(L"/MPL/Item/Field");
    const TString   strElem_Item(L"/MPL/Item");
    const TString   strElem_Database(L"/MPL");

    const TString   strFld_Album(L"Album");
    const TString   strFld_Artist(L"Artist");
    const TString   strFld_ArtPath(L"Image File");
    const TString   strFld_Channels(L"Channels");
    const TString   strFld_Duration(L"Duration");
    const TString   strFld_Filename(L"Filename");
    const TString   strFld_Genre(L"Genre");
    const TString   strFld_MediaType(L"Media Type");
    const TString   strFld_TrackName(L"Name");
    const TString   strFld_TrackNum(L"Track #");
    const TString   strFld_Year(L"Date");

    const TString   strVal_Audio("Audio");
}



// ---------------------------------------------------------------------------
//   CLASS: TTCQSLImpJRLoader
//  PREFIX: dbl
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TTCQSLImpJRLoader: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQSLImpJRLoader::TCQSLImpJRLoader() :

    m_bInItem(kCIDLib::False)
    , m_c2NextCatId(kCQCMedia::c2CatId_UserStart)
    , m_c2NextColId(1)
    , m_c2NextImgId(1)
    , m_c2NextItemId(1)
    , m_eField(EField_None)
{
    //
    //  Create a DTD validator and plug it into our parser. We have to tell
    //  the validator about the parser as well.
    //
    TDTDValidator* pvalNew = new TDTDValidator(&m_xprsThis);
    m_xprsThis.pxvalValidator(pvalNew);

    // Set ourself as handlers on the parser
    m_xprsThis.pmxevDocEvents(this);
    m_xprsThis.pmxevErrorEvents(this);
}

TCQSLImpJRLoader::~TCQSLImpJRLoader()
{
}


// ---------------------------------------------------------------------------
//  Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Starts the database load process by storing the passed database to laod
//  and the file name to load from, and then starting ourself up as a separate
//  thread that does the loading.
//
tCIDLib::TBoolean
TCQSLImpJRLoader::bLoadIt(          tCQSLRepoImp::TRepoClient&  orbcRepoMgr
                            , const TString&                    strXMLFile
                            , const TString&                    strLeaseId)
{
    // Store the incoming info for later use during callbacks
    m_orbcRepoMgr = orbcRepoMgr;
    m_strFilename = strXMLFile;
    m_strLeaseId = strLeaseId;

    // Add an 'All Music' category that all stuff can be put into
    m_strAllMusicCatName = facCQCMedia().strMsg(kMedMsgs::midCat_AllMusic);
    TMediaCat* pmcatAll = new TMediaCat
    (
        tCQCMedia::EMediaTypes::Music
        , m_strAllMusicCatName
        , kCQCMedia::c2CatId_AllMusic
    );
    m_mdbLoading.AddCategory(pmcatAll);

    // And now kick off the parsing process. Let exceptions propogate
    const tCIDXML::EParseOpts   eOpts  = tCIDXML::EParseOpts::None;
    const tCIDXML::EParseFlags  eFlags = tCIDXML::EParseFlags::TagsNText;

    // Kick off the parsing
    m_xprsThis.ParseRootEntity(m_strFilename, eOpts, eFlags);

    //
    //  We are back again, so it must have worked.
    //
    //  Deal with the collection level artist name. If all of the
    //  item artist names are the same, we set the collection level
    //  artist to that. Else we set it to Various Artists.
    //
    const tCQCMedia::EMediaTypes eType = tCQCMedia::EMediaTypes::Music;
    const tCIDLib::TCard4 c4ColCnt = m_mdbLoading.c4CollectCnt(eType);
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ColCnt; c4Index++)
    {
        //
        //  If all of the artist names are the same, then we just use
        //  that. Else, set it to "Various Artists"
        //
        TMediaCollect& mcolCur = m_mdbLoading.mcolAt(eType, c4Index);
        const tCIDLib::TCard4 c4ItemCnt = mcolCur.c4ItemCount();
        TMediaItem* pmitemPrev = &mcolCur.mitemAt(m_mdbLoading, 0);
        TMediaItem* pmitemCur = 0;
        tCIDLib::TCard4 c4IIndex = 1;
        for (; c4IIndex < c4ItemCnt; c4IIndex++)
        {
            pmitemCur = &mcolCur.mitemAt(m_mdbLoading, c4IIndex);
            if (pmitemCur->strArtist() != pmitemPrev->strArtist())
                break;
            pmitemPrev = pmitemCur;
        }

        if (c4IIndex == c4ItemCnt)
        {
            // They are all the same
            pmitemCur = &mcolCur.mitemAt(m_mdbLoading, 0);
            mcolCur.strArtist(pmitemCur->strArtist());
        }
         else
        {
            mcolCur.strArtist(L"Various Artists");
        }
    }

    // And do the same for title sets wrt to their collections
    const tCIDLib::TCard4 c4SetCnt = m_mdbLoading.c4TitleSetCnt(eType);
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4SetCnt; c4Index++)
    {
        TMediaTitleSet& mtsCur = m_mdbLoading.mtsAt(eType, c4Index);
        const tCIDLib::TCard4 c4ColCnt = mtsCur.c4ColCount();
        TMediaCollect* pmcolPrev = &mtsCur.mcolAt(m_mdbLoading, 0);
        TMediaCollect* pmcolCur = 0;
        tCIDLib::TCard4 c4IIndex = 1;
        for (; c4IIndex < c4ColCnt; c4IIndex++)
        {
            pmcolCur = &mtsCur.mcolAt(m_mdbLoading, c4IIndex);
            if (pmcolCur->strArtist() != pmcolPrev->strArtist())
                break;
            pmcolPrev = pmcolCur;
        }

        if (c4IIndex == c4ColCnt)
        {
            // They are all the same
            pmcolCur = &mtsCur.mcolAt(m_mdbLoading, 0);
            mtsCur.strArtist(pmcolCur->strArtist());
        }
         else
        {
            mtsCur.strArtist(L"Various Artists");
        }
    }


    // And let's upload everything now if not in test mode
    if (!bTestMode)
        UploadDB();

    return kCIDLib::True;
}



// ---------------------------------------------------------------------------
//  JRDiskRepoLoader: Protected, virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TCQSLImpJRLoader::DocCharacters(const   TString&            strChars
                                , const tCIDLib::TBoolean   bIsCDATA
                                , const tCIDLib::TBoolean   bIsIgnorable
                                , const tCIDXML::ELocations eLocation)
{
    //
    //  Note that it is theoretically possible to get chars in more than
    //  one chunk, so we append here. The target strings were all cleared
    //  at the start element, so it's safe to do this.
    //
    switch(m_eField)
    {
        case EField_Album :
            m_strFld_Album.Append(strChars);
            break;

        case EField_Artist :
            m_strFld_Artist.Append(strChars);
            break;

        case EField_ArtPath :
            m_pathFld_ArtPath.Append(strChars);
            break;

        case EField_Channels :
            m_strFld_Channels.Append(strChars);
            break;

        case EField_Duration :
            m_strFld_Duration.Append(strChars);
            break;

        case EField_Filename :
            m_pathFld_Filename.Append(strChars);
            m_pathFld_Filename.Normalize();
            break;

        case EField_Genre :
            m_strFld_CatNames.Append(strChars);
            break;

        case EField_MediaType :
            m_strFld_MediaType.Append(strChars);
            break;

        case EField_TrackName :
            m_strFld_TrackName.Append(strChars);
            break;

        case EField_TrackNum :
            m_strFld_TrackNum.Append(strChars);
            break;

        case EField_Year :
            m_strFld_Year.Append(strChars);
            break;
    };
}


tCIDLib::TVoid TCQSLImpJRLoader::EndDocument(const TXMLEntitySrc& xsrcOfRoot)
{
}


tCIDLib::TVoid TCQSLImpJRLoader::EndTag(const TXMLElemDecl& xdeclElem)
{
    if (m_strCurXMLPath == JRLoader::strElem_Item)
    {
        // If not in an item right now, then it's bad
        if (!m_bInItem)
        {
            c4ErrCnt++;
            *pconOut << L"NEsting error at line " << m_xprsThis.c4CurLine()
                     << L". The file is invalid, exiting...."
                     << kCIDLib::EndLn;
            TProcess::ExitProcess(tCIDLib::EExitCodes::RuntimeError);
        }

        // Clear the flag now that we've finished this guy
        m_bInItem = kCIDLib::False;

        // If it's not an audio file type, we don't want it
        if (m_strFld_MediaType == JRLoader::strVal_Audio)
            ProcessItem();
    }
     else if (m_strCurXMLPath == JRLoader::strElem_Database)
    {
        // Nothing to do
    }
     else
    {
        //
        //  Probably end of a field, in which case we don't need to take any
        //  more text for this field. If not end of a field, this doesn't
        //  hurt since it should be none in that case anyway.
        //
        m_eField = EField_None;
    }

    // Pop the XML path now
    PopXMLPath(xdeclElem);
}


tCIDLib::TVoid
TCQSLImpJRLoader::HandleXMLError(const  tCIDLib::TErrCode   errcToPost
                                , const tCIDXML::EErrTypes  eType
                                , const TString&            strText
                                , const tCIDLib::TCard4     c4CurColumn
                                , const tCIDLib::TCard4     c4CurLine
                                , const TString&            strSystemId)
{
    c4ErrCnt++;
    *pconOut << L"An XML parse error occured, exiting... Error:\n"
             << L" (Line/Col: " << c4CurLine << L"." << c4CurColumn
             << L") - " << strText << kCIDLib::EndLn;
    TProcess::ExitProcess(tCIDLib::EExitCodes::RuntimeError);
}


tCIDLib::TVoid TCQSLImpJRLoader::ResetDocument()
{
}



tCIDLib::TVoid TCQSLImpJRLoader::StartDocument(const TXMLEntitySrc&)
{
}


tCIDLib::TVoid
TCQSLImpJRLoader::StartTag(         TXMLParserCore&     xprsSrc
                            , const TXMLElemDecl&       xdeclElem
                            , const tCIDLib::TBoolean   bEmpty
                            , const TVector<TXMLAttr>&  colAttrList
                            , const tCIDLib::TCard4     c4AttrListSize)
{
    // Push the new element
    m_strCurXMLPath.Append(L'/');
    m_strCurXMLPath.Append(xdeclElem.strFullName());

    //
    //  If it's an Item element, then we have to do a little preperatory
    //  work. Else, it's got to be a field so we set up for the field data
    //  to come.
    //
    if (m_strCurXMLPath == JRLoader::strElem_Item)
    {
        // If we are already in an item, then the file is bad
        if (m_bInItem)
        {
            c4ErrCnt++;
            *pconOut << L"Encountered nested Item elements at line "
                     << m_xprsThis.c4CurLine()
                     << L". The file is invalid, exiting...."
                     << kCIDLib::EndLn;
            TProcess::ExitProcess(tCIDLib::EExitCodes::RuntimeError);
        }

        // Set the flag and reset any per-item data
        m_bInItem = kCIDLib::True;

        m_pathFld_Filename.Clear();

        m_strFld_Album.Clear();
        m_strFld_Artist.Clear();
        m_pathFld_ArtPath.Clear();
        m_strFld_Channels.Clear();
        m_strFld_Duration.Clear();
        m_strFld_CatNames.Clear();
        m_strFld_MediaType.Clear();
        m_strFld_TrackName.Clear();
        m_strFld_TrackNum.Clear();
        m_strFld_Year.Clear();
    }
     else if (m_strCurXMLPath == JRLoader::strElem_Field)
    {
        //
        //  It's a field value, so look at the Name attribute and set
        //  the m_eField enum to indicate what field we expect to see the
        //  data for in the next characters callback.
        //
        const TXMLAttr* pxattrName = 0;
        if (c4AttrListSize > 1)
        {
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4AttrListSize; c4Index++)
            {
                const TXMLAttr& xattrCur = colAttrList[c4Index];
                if (xattrCur.strQName() == JRLoader::strAttr_Name)
                {
                    pxattrName = &xattrCur;
                    break;
                }
            }

            // If it doesn't have a name attr, the file is bad
            if (!pxattrName)
            {
                c4ErrCnt++;
                *pconOut << L"No name attribute was found for the Item element"
                            L" at line " << m_xprsThis.c4CurLine()
                         << L". The file is invalid, exiting...."
                         << kCIDLib::EndLn;
                TProcess::ExitProcess(tCIDLib::EExitCodes::RuntimeError);
            }
        }
         else
        {
            // Just one attr, assume it's the name
            pxattrName = &colAttrList[0];
        }

        //
        //  Get the value and set the field enum. There are fields we don't
        //  care about, in which we set it to unknown.
        //
        const TString& strFldName = pxattrName->strValue();

        //
        //  To avoid a lot of usless comparisons, we use pre-determined
        //  length values to only look at strings that are of the same
        //  length as ones we care about.
        //
        const tCIDLib::TCard4 c4NewLen = strFldName.c4Length();
        m_eField = EField_None;
        switch(c4NewLen)
        {
            case 4 :
                if (strFldName == JRLoader::strFld_Year)
                    m_eField = EField_Year;
                else if (strFldName == JRLoader::strFld_TrackName)
                    m_eField = EField_TrackName;
                break;

            case 5 :
                if (strFldName == JRLoader::strFld_Album)
                    m_eField = EField_Album;
                else if (strFldName == JRLoader::strFld_Genre)
                    m_eField = EField_Genre;
                break;

            case 6 :
                if (strFldName == JRLoader::strFld_Artist)
                    m_eField = EField_Artist;
                break;

            case 7 :
                if (strFldName == JRLoader::strFld_TrackNum)
                    m_eField = EField_TrackNum;
                break;

            case 8 :
                if (strFldName == JRLoader::strFld_Channels)
                    m_eField = EField_Channels;
                else if (strFldName == JRLoader::strFld_Filename)
                    m_eField = EField_Filename;
                else if (strFldName == JRLoader::strFld_Duration)
                    m_eField = EField_Duration;
                break;

            case 10 :
                if (strFldName == JRLoader::strFld_ArtPath)
                    m_eField = EField_ArtPath;
                else if (strFldName == JRLoader::strFld_MediaType)
                    m_eField = EField_MediaType;
                break;
        };
    }
}



// ---------------------------------------------------------------------------
//  TCQSLImpJRLoader: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Pops the last element off of the current XML path and verifies that
//  that element is the one that we got that's ending. We consider any
//  errors here to be fatal and exit.
//
tCIDLib::TVoid TCQSLImpJRLoader::PopXMLPath(const TXMLElemDecl& xdeclEnding)
{
    // If it's empty, then we've underflowed
    if (m_strCurXMLPath.bIsEmpty())
    {
        *pconOut    << L"The XML nesting stack has underflowed, probably "
                       L"due to invalid nesting. Exiting..."
                    << kCIDLib::EndLn;
        TProcess::ExitProcess(tCIDLib::EExitCodes::RuntimeError);
    }

    // Find the last separator
    tCIDLib::TCard4 c4Ofs;
    if (!m_strCurXMLPath.bLastOccurrence(L'/', c4Ofs))
    {
        *pconOut    << L"The XML nesting stack is invalid, probably "
                       L"due to internal error. Exiting..."
                    << kCIDLib::EndLn;
        TProcess::ExitProcess(tCIDLib::EExitCodes::RuntimeError);
    }

    // Get the bit after the separate to a tmp and compare to the passed element
    m_strCurXMLPath.CopyOutSubStr(m_strTmp1, c4Ofs + 1);
    if (m_strTmp1 != xdeclEnding.strFullName())
    {
        *pconOut    << L"XML nesting error. Got end of element "
                    << xdeclEnding.strFullName() << L"but expected the end "
                    << L"of element " << m_strTmp1 << L". Exiting..."
                    << kCIDLib::EndLn;
        TProcess::ExitProcess(tCIDLib::EExitCodes::RuntimeError);
    }

    // Looks ok, cap the path at the divider
    m_strCurXMLPath.CapAt(c4Ofs);
}


tCIDLib::TVoid TCQSLImpJRLoader::ProcessItem()
{
    // If the title or file path is empty, then we can't do this one
    if (m_strFld_Album.bIsEmpty() || m_pathFld_Filename.bIsEmpty())
    {
        *pconOut << L"CD name/loc info was empty, skipping title ended at line"
                 << m_xprsThis.c4CurLine() << kCIDLib::EndLn;
        c4SkippedCnt++;
        return;
    }

    //
    //  If we got an art path, see if this image is already in the database,
    //  else add it.
    //
    TMediaImg* pmimgArt = 0;
    if (m_pathFld_ArtPath.bIsEmpty())
    {
        c4MissingArtCnt++;
    }
     else
    {
        m_pathFld_ArtPath.Normalize();
        if (!m_pathFld_ArtPath.bIsFullyQualified())
        {
            TPathStr pathTmp(m_pathFld_Filename);
            pathTmp.bRemoveNameExt();
            m_pathFld_ArtPath.AddToBasePath(pathTmp);
        }

        if (TFileSys::bExists(m_pathFld_ArtPath))
        {
            pmimgArt = m_mdbLoading.pmimgByPath
            (
                tCQCMedia::EMediaTypes::Music, m_pathFld_ArtPath, kCIDLib::False
            );

            if (!pmimgArt)
            {
                pmimgArt = new TMediaImg
                (
                    tCQCMedia::EMediaTypes::Music
                    , m_c2NextImgId++
                    , m_pathFld_ArtPath
                    , TString::strEmpty()
                    , TString::strEmpty()
                );
                m_mdbLoading.AddImage(pmimgArt);
            }
        }
         else
        {
            c4MissingArtCnt++;
        }
    }

    //
    //  They don't provide unique ids, so we generate one ourself by
    //  hashing the album name.
    //
    m_mdigId.Digest(m_mhashId, m_strFld_Album);
    m_mhashId.FormatToStr(m_strUniqueId);
    m_strUniqueId.Prepend(L"JRMC_");

    //
    //  See if this title is already in the database. If not, we need to
    //  create it. Unfortunately, we have nothing in the data to allow us
    //  to deal correctly with individual discs and create per-disc
    //  collections within the title set. So we just put all the tracks
    //  into a single collection.
    //
    tCIDLib::TBoolean bNewTitle = kCIDLib::False;
    TMediaTitleSet* pmtsCur = 0;
    pmtsCur = m_mdbLoading.pmtsByUniqueId
    (
        tCQCMedia::EMediaTypes::Music, m_strUniqueId, kCIDLib::False
    );

    tCIDLib::TBoolean bValid;
    tCIDLib::TCard4 c4Seconds = TRawStr::c4AsBinary
    (
        m_strFld_Duration.pszBuffer(), bValid, tCIDLib::ERadices::Dec
    );
    if (!bValid)
        c4Seconds = 0;

    TMediaCollect* pmcolCur;
    if (pmtsCur)
    {
        // It already exists, so get the collection out
        pmcolCur = &pmtsCur->mcolAt(m_mdbLoading, 0);
    }
     else
    {
        // A new one, so create a new title set and add it to the database
        bNewTitle = kCIDLib::True;
        facCQCMedia().bMakeSortTitle(m_strFld_Album, m_strTmp1);
        pmtsCur = new TMediaTitleSet
        (
            m_strFld_Album
            , m_strUniqueId
            , m_strTmp1
            , m_c2NextColId++
            , tCQCMedia::EMediaTypes::Music
        );
        m_mdbLoading.AddTitle(pmtsCur);

        // They don't provide unique ids, so we generate one ourself
        pmtsCur->strUniqueId(m_strUniqueId);

        //
        //  The date is a Julian date, based on year 1900, so we have
        //  to do a bit of conversion. So convert to a number and add
        //  the base Julian date value for 1/1/1900, then convert to
        //  year/month/day values.
        //
        tCIDLib::TCard4 c4Year = 0;
        if (!m_strFld_Year.bIsEmpty())
        {
            const tCIDLib::TFloat8 f8Julian
            (
                m_strFld_Year.f8Val() + kCIDLib::c4Julian1900
            );
            tCIDLib::TCard4  c4Day;
            tCIDLib::EMonths eMonth;
            TTime::CvtFromJulian(f8Julian, c4Year, eMonth, c4Day);
        }

        //
        //  Add a single collection. Use the title set unique id for
        //  the disc id, since it doesn't really matter in this repository.
        //  Give it the same numeric id as the title set we just added,
        //  since there is a one to one relationship between them in this
        //  repository.
        //
        pmcolCur = new TMediaCollect
        (
            m_strFld_Album
            , pmtsCur->strUniqueId()
            , pmtsCur->c2Id()
            , c4Year
            , tCQCMedia::ELocTypes::FileItem
            , tCQCMedia::EMediaTypes::Music
        );
        m_mdbLoading.AddCollect(pmcolCur);
        pmtsCur->bAddCollect(*pmcolCur);

        // Add it to the all music category
        pmcolCur->bAddCategory(kCQCMedia::c2CatId_AllMusic);

        // Set the image id for the cover art if any
        if (pmimgArt)
            pmcolCur->c2ArtId(pmimgArt->c2Id());
    }

    //
    //  Scan through the genres in the genre list we found for this round
    //  and see if they exist yet. Create them if not, and add them to the
    //  list of categories or this collection.
    //
    //  The unique id for the categories just gets set to the same value
    //  as the name, so by checking for whether the UID exists already
    //  we keep only unique names in the category list.
    //
    if (!m_strFld_CatNames.bIsEmpty())
    {
        TMediaCat*       pmcatCur;
        TStringTokenizer stokCats(&m_strFld_CatNames, L",");
        TString          strCurCat;
        while (stokCats.bGetNextToken(strCurCat))
        {
            strCurCat.StripWhitespace();
            m_strTmp1 = strCurCat;
            m_strTmp1.ToUpper();
            pmcatCur = m_mdbLoading.pmcatByUniqueId
            (
                tCQCMedia::EMediaTypes::Music, m_strTmp1, kCIDLib::False
            );

            if (!pmcatCur)
            {
                pmcatCur = new TMediaCat
                (
                    tCQCMedia::EMediaTypes::Music, strCurCat, m_c2NextCatId++
                );
                m_mdbLoading.AddCategory(pmcatCur);
            }

            // Add this one to the collection if not already
            pmcolCur->bAddCategory(*pmcatCur);
        }
    }

    // And add a track item for the current track info
    tCIDLib::TCard4 c4Channels;
    tCIDLib::TCard4 c4TrackNum;
    c4Channels = TRawStr::c4AsBinary
    (
        m_strFld_Channels.pszBuffer(), bValid, tCIDLib::ERadices::Dec
    );
    if (!bValid)
        c4Channels = 0;

    //
    //  Set the unique id to the album id, plus an underscore and then the
    //  track number.
    //
    c4TrackNum = TRawStr::c4AsBinary
    (
        m_strFld_TrackNum.pszBuffer(), bValid, tCIDLib::ERadices::Dec
    );
    if (!bValid)
        c4TrackNum = 0;

    m_strUniqueId.Append(kCIDLib::chUnderscore);
    m_strUniqueId.AppendFormatted(c4TrackNum);
    TMediaItem* pmitemNew = new TMediaItem
    (
        m_strFld_TrackName
        , m_strUniqueId
        , m_c2NextItemId++
        , m_pathFld_Filename
        , tCQCMedia::EMediaTypes::Music
    );
    m_mdbLoading.AddItem(pmitemNew);

    // Replace any ^ characters with spaces for historical purposes
    m_strFld_Artist.bReplaceChar(kCIDLib::chCircumflex, kCIDLib::chSpace);
    pmitemNew->strArtist(m_strFld_Artist);

    // And store the other item stuff
    pmitemNew->c4Channels(c4Channels);
    pmitemNew->c4Duration(c4Seconds);
    pmitemNew->c4AltId(c4TrackNum);

    //
    //  Figure out where to add the track in the collection. They are not
    //  necessarliy in order.
    //
    const tCIDLib::TCard4 c4ItemCnt = pmcolCur->c4ItemCount();
    tCIDLib::TCard4 c4Index = 0;
    for (; c4Index < c4ItemCnt; c4Index++)
    {
        TMediaItem* pmitemCur = m_mdbLoading.pmitemById
        (
            tCQCMedia::EMediaTypes::Music
            , pmcolCur->c2ItemIdAt(c4Index)
            , kCIDLib::False
        );
        if (c4TrackNum < pmitemCur->c4AltId())
            break;
    }

    if (c4Index == c4ItemCnt)
        c4Index = kCIDLib::c4MaxCard;
    pmcolCur->bAddItem(*pmitemNew, c4Index);

    // Add this track's time to our total
    pmcolCur->c4Duration(pmcolCur->c4Duration() + c4Seconds);
}


//
//  Once we have the whole database parsed and stored away, this is
//  called at the end (in the non-test pass) to do the upload.
//
//  NOTE:   We have to be careful here about things like ids, because they
//          should not be set when we are uploading new content. But they
//          got set since we created our own local database during the
//          parse. So we need to turn those all off, which we do by just
//          filling in locals from the database info and uploading those.
//
tCIDLib::TVoid TCQSLImpJRLoader::UploadDB()
{
    //
    //  Iterate through the title sets and upload each one. We know each
    //  title set has only one collection, so we only have to do one
    //  upload per title set.
    //
    TVector<TString>    colCatNames(32UL);
    TVector<TMediaItem> colItems(32UL);
    tCQCMedia::TIdList  fcolCatIds;

    const tCIDLib::TCard4 c4TitleCnt = m_mdbLoading.c4TitleSetCnt
    (
        tCQCMedia::EMediaTypes::Music
    );
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4TitleCnt; c4Index++)
    {
        const TMediaTitleSet& mtsCurSrc = m_mdbLoading.mtsAt
        (
            tCQCMedia::EMediaTypes::Music, c4Index
        );
        const TMediaCollect& mcolCurSrc = mtsCurSrc.mcolAt(m_mdbLoading, 0);

        // Set up the title set, with a zeroed id
        TMediaTitleSet mtsUpload
        (
            mtsCurSrc.strName()
            , mtsCurSrc.strUniqueId()
            , mtsCurSrc.strSortTitle()
            , 0
            , tCQCMedia::EMediaTypes::Music
        );

        mtsUpload.strArtist(mtsCurSrc.strArtist());

        //
        //  Create a unique id for the collection which is the title set
        //  id plus _1.
        //
        m_strTmp1 = mtsCurSrc.strUniqueId();
        m_strTmp1.Append(L"_1");
        TMediaCollect mcolUpload
        (
            mcolCurSrc.strName()
            , m_strTmp1
            , 0
            , mcolCurSrc.c4Year()
            , tCQCMedia::ELocTypes::FileItem
            , tCQCMedia::EMediaTypes::Music
        );

        // Store the other info on the collection
        mcolUpload.c4Duration(mcolCurSrc.c4Duration());

        // Set the category ids on the collection
        colCatNames.RemoveAll();
        fcolCatIds.RemoveAll();
        const tCIDLib::TCard4 c4CatCnt = mcolCurSrc.c4CatCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4CatCnt; c4Index++)
        {
            colCatNames.objAdd
            (
                m_mdbLoading.pmcatById
                (
                    tCQCMedia::EMediaTypes::Music
                    , mcolCurSrc.c2CatIdAt(c4Index)
                    , kCIDLib::True
                )->strName()
            );
        }
        m_orbcRepoMgr->CreateCategories
        (
            m_strLeaseId, colCatNames, tCQCMedia::EMediaTypes::Music, fcolCatIds
        );

        // And set them on the collection
        mcolUpload.SetCatIds(fcolCatIds);

        // Set all the other collection info we gathered
        mcolUpload.strLabel(mcolCurSrc.strLabel());
        mcolUpload.strRating(mcolCurSrc.strRating());
        mcolUpload.strUPC(mcolCurSrc.strUPC());

        // Load the cover art
        tCIDLib::TCard4 c4ArtSz = 0;
        if (mcolCurSrc.c2ArtId())
        {
            const TMediaImg* pimgCur = m_mdbLoading.pmimgById
            (
                tCQCMedia::EMediaTypes::Music
                , mcolCurSrc.c2ArtId()
                , kCIDLib::True
            );

            if (!pimgCur->strArtPath().bIsEmpty()
            &&  TFileSys::bExists(pimgCur->strArtPath()))
            {
                TBinaryFile flImg(pimgCur->strArtPath());
                flImg.Open
                (
                    tCIDLib::EAccessModes::Excl_Read
                    , tCIDLib::ECreateActs::OpenIfExists
                    , tCIDLib::EFilePerms::Default
                    , tCIDLib::EFileFlags::SequentialScan
                );
                c4ArtSz = tCIDLib::TCard4(flImg.c8CurSize());
                const tCIDLib::TCard4 c4Read = flImg.c4ReadBuffer
                (
                    m_mbufArtUpload, c4ArtSz
                );

                if (c4ArtSz != c4Read)
                {
                    c4ErrCnt++;
                    *pconOut << L"Could not read image for: "
                             << mtsCurSrc.strName()
                             << L". No image will be uploaded."
                             << kCIDLib::NewLn;
                    c4ArtSz = 0;
                }
            }
        }

        // If we got no art and not in test mode, then upload a default image
        if (!c4ArtSz)
        {
            TBinMBufOutStream strmOut(&m_mbufArtUpload);
            strmOut << m_imgDefArt << kCIDLib::FlushIt;
            c4ArtSz = strmOut.c4CurSize();
        }

        //
        //  And now let's set up the items for this collection. The
        //
        colItems.RemoveAll();
        const tCIDLib::TCard4 c4ItemCnt = mcolCurSrc.c4ItemCount();
        for (tCIDLib::TCard4 c4ItemIndex = 0; c4ItemIndex < c4ItemCnt; c4ItemIndex++)
        {
            const TMediaItem& mitemCurSrc = mcolCurSrc.mitemAt(m_mdbLoading, c4ItemIndex);

            // Create a unique id based on the collection id plus our index
            m_strTmp1 = mcolCurSrc.strUniqueId();
            m_strTmp1.AppendFormatted(c4ItemIndex + 1);

            TMediaItem mitemUpload
            (
                mitemCurSrc.strName()
                , m_strTmp1
                , 0
                , mitemCurSrc.strLocInfo()
                , tCQCMedia::EMediaTypes::Music
            );
            mitemUpload.strArtist(mitemCurSrc.strArtist());
            colItems.objAdd(mitemUpload);
        }

        // Ok upload it
        *pconOut << L"Uploading: " << mtsCurSrc.strName() << kCIDLib::EndLn;
        m_orbcRepoMgr->UploadNewMeta
        (
            m_strLeaseId
            , tCQCMedia::EMediaTypes::Music
            , colCatNames
            , mtsUpload
            , mcolUpload
            , colItems
            , c4ArtSz
            , m_mbufArtUpload
        );


        // Don't beat the server to death. Pause a bit between each one
        TThread::Sleep(100);
    }
}

