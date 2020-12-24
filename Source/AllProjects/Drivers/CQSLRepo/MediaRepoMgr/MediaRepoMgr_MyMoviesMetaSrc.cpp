//
// FILE NAME: MediaRepoMgr_MyMoviesMetaSrc.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/01/2011
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
//  This file implements a derivative of the metadata source class for the
//  My Movies online database.
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
#include    "MediaRepoMgr.hpp"


// ---------------------------------------------------------------------------
//  TMyMoviesMetaSrc: Constructors Destructor
// ---------------------------------------------------------------------------
TMyMoviesMetaSrc::TMyMoviesMetaSrc(const TMediaRepoSettings& mrsOpts) :

    TRepoMetaSrc(L"My Movies", L"UPC")
    , m_mbufData(0x10000, 0x100000, 0x10000)
    , m_mrsOpts(mrsOpts)
{
}

TMyMoviesMetaSrc::~TMyMoviesMetaSrc()
{
}


// ---------------------------------------------------------------------------
//  TMyMoviesMetaSrc: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  Get a list of potential matches for a UPC barcode.
//
tCIDLib::TBoolean
TMyMoviesMetaSrc::bBarcodeSearch(const  TString&                    strBarcode
                                ,       tMediaRepoMgr::TMetaList&   colToFill
                                ,       TString&                    strFailReason)
{
    colToFill.RemoveAll();

    tMediaRepoMgr::EMMStatus eRet;
    try
    {
        // Set up the options for the search.

        tCIDLib::TKVPList colOpts(3);
        colOpts.objAdd(TKeyValuePair(L"barcode", strBarcode));
        colOpts.objAdd
        (
            TKeyValuePair(L"includeenglish", pszBoolOptVal(m_mrsOpts.bShowEnglish()))
        );
        colOpts.objAdd
        (
            TKeyValuePair(L"includeadult", pszBoolOptVal(m_mrsOpts.bShowAdult()))
        );

        eRet = eGetServerRep
        (
            s_strCmd_BarcodeLookup
            , colOpts
            , m_mrsOpts.strMMName()
            , m_mrsOpts.strMMPassword()
            , strFailReason
        );
    }

    catch(TError& errToCatch)
    {
        if (facMediaRepoMgr.bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
        strFailReason = errToCatch.strErrText();
        return kCIDLib::False;
    }

    //
    //  If it's a failure status, then show an error. Otherwise, has to be
    //  OK status or doesn't make any sense.
    //
    if (eRet == tMediaRepoMgr::EMMStatus::Failed)
    {
        return kCIDLib::False;
    }
     else if (eRet == tMediaRepoMgr::EMMStatus::None)
    {
        // It worked, but nothing matched
        strFailReason = L"No match was found in the My Movies database for this barcode.";
        return kCIDLib::False;
    }
     else if (eRet != tMediaRepoMgr::EMMStatus::OK)
    {
        // This response doesn't make sense in this case
        strFailReason = L"The received status reply makes no sense in this context";
        return kCIDLib::False;
    }

    // Parse the list of possible matches
    tCIDLib::TCard4 c4AtInd;
    const TXMLTreeElement& xtnodeList = m_xtprsMeta.xtdocThis().xtnodeRoot().xtnodeFindElement
    (
        s_strElem_Titles, 0, c4AtInd
    );
    return bParseSearchResult(xtnodeList, colToFill, strFailReason);
}


//
//  Given an explicit title id, which should only match one title, we query the
//  data and fill in the metadata for it if it works.
//
tCIDLib::TBoolean
TMyMoviesMetaSrc::bLoadTitleById(const  TString&            strID
                                ,       TMediaCollect&      mcolToFill
                                ,       TJPEGImage&         imgArt
                                ,       tCIDLib::TStrList&  colCatNames
                                ,       TString&            strFailReason)
{
    tMediaRepoMgr::EMMStatus eRet;
    try
    {
        //
        //  Set up the options for the search. We need to get the repo manager
        //  configuration to set some of them.
        //
        tCIDLib::TKVPList colOpts(4);
        colOpts.objAdd(TKeyValuePair(L"titleid", strID));
        colOpts.objAdd(TKeyValuePair(L"actorlimit", L"true"));
        colOpts.objAdd(TKeyValuePair(L"client", L"CQC Media Repo"));
        colOpts.objAdd(TKeyValuePair(L"version", kCQCKit::pszVersion));
        eRet = eGetServerRep
        (
            s_strCmd_LoadByID
            , colOpts
            , m_mrsOpts.strMMName()
            , m_mrsOpts.strMMPassword()
            , strFailReason
        );
    }

    catch(TError& errToCatch)
    {
        if (facMediaRepoMgr.bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
        strFailReason = errToCatch.strErrText();
        return kCIDLib::False;
    }

    //
    //  If it's a failure status, then show an error. Otherwise, has to be
    //  OK status or doesn't make any sense.
    //
    if (eRet == tMediaRepoMgr::EMMStatus::Failed)
    {
        // The failure reason is already set by the reply getter above
        return kCIDLib::False;
    }
     else if (eRet != tMediaRepoMgr::EMMStatus::OK)
    {
        strFailReason = L"The received status reply makes no sense in this context";
        return kCIDLib::False;
    }

    // Get the title element out, which is the only child of the root
    tCIDLib::TCard4 c4AtInd;
    const TXMLTreeElement& xtnodeTitle = m_xtprsMeta.xtdocThis().xtnodeRoot().xtnodeFindElement
    (
        s_strElem_Title, 0, c4AtInd
    );

    // And parse the info
    return bParseTitleResult(xtnodeTitle, mcolToFill, imgArt, colCatNames, strFailReason);
}


//
//  This method will test the connction to the server, and teh validity of the
//  user name and password. If it works, we get back the number of queries left
//  which can be displayed to the user.
//
tCIDLib::TBoolean
TMyMoviesMetaSrc::bTestConnection(tCIDLib::TCard4& c4Left, TString& strFailReason)

{
    tMediaRepoMgr::EMMStatus eRet;
    try
    {
        tCIDLib::TKVPList colEmpty;
        eRet = eGetServerRep
        (
            s_strCmd_GetRemainingLookups
            , colEmpty
            , m_mrsOpts.strMMName()
            , m_mrsOpts.strMMPassword()
            , strFailReason
        );
    }

    catch(TError& errToCatch)
    {
        if (facMediaRepoMgr.bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
        strFailReason = errToCatch.strErrText();
        return kCIDLib::False;
    }

    //
    //  If it's a failure status, then show an error. Otherwise, has to be
    //  OK status or doesn't make any sense.
    //
    if (eRet == tMediaRepoMgr::EMMStatus::Failed)
    {
        // The failure reason is already set by the reply getter above
        return kCIDLib::False;
    }
     else if (eRet != tMediaRepoMgr::EMMStatus::OK)
    {
        strFailReason = L"The received status reply makes no sense in this context";
        return kCIDLib::False;
    }

    //
    //  Get the text of the root element and convert to a number. In this case,
    //  don't let it return a default if not convertable.
    //
    try
    {
        c4Left = c4GetTextAsCard
        (
            m_xtprsMeta.xtdocThis().xtnodeRoot(), kCIDLib::c4MaxCard
        );
    }

    catch(...)
    {
        strFailReason = L"The available queries remaining value was not a valid number";
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


//
//  We do a search by disc id and return all possible matches. The passed volume
//  is the root of the disc file space. It's assumed that the caller already
//  probed the driver and got the removable media type of it, which we use to know
//  which type of id to generate.
//
tCIDLib::TBoolean
TMyMoviesMetaSrc::bDiscIdSearch(const   TString&                    strVolume
                                ,       tMediaRepoMgr::TMetaList&   colToFill
                                , const TKrnlRemMedia::EMediaTypes  eMType
                                ,       TString&                    strFailReason)
{
    colToFill.RemoveAll();

    tMediaRepoMgr::EMMStatus eRet;
    try
    {
        // First let's try to generate the id
        TString strDiscID;

        if (TKrnlRemMedia::bIsDVDType(eMType))
        {
            TCIDDVDID dvdidSrc;
            tCIDLib::TCard8 c8ID;
            dvdidSrc.GenDVDID(strVolume, c8ID, strDiscID);
        }
         else if (TKrnlRemMedia::bIsBDType(eMType))
        {

            TCIDBlurayID dvdidSrc;
            tCIDLib::TCard8 c8ID;
            dvdidSrc.GenBlurayID(strVolume, c8ID, strDiscID);
        }
         else
        {
            // We shouldn't get any types but DVD and BD
            strFailReason = L"This method only accepts DVD or BD media types";
            return 0;
        }

        // Set up the options for the search
        tCIDLib::TKVPList colOpts(4);
        colOpts.objAdd(TKeyValuePair(L"discid", strDiscID));
        colOpts.objAdd
        (
            TKeyValuePair(L"includeenglish", pszBoolOptVal(m_mrsOpts.bShowEnglish()))
        );
        colOpts.objAdd
        (
            TKeyValuePair(L"includeadult", pszBoolOptVal(m_mrsOpts.bShowAdult()))
        );

        eRet = eGetServerRep
        (
            s_strCmd_DiscIDLookup
            , colOpts
            , m_mrsOpts.strMMName()
            , m_mrsOpts.strMMPassword()
            , strFailReason
        );
    }

    catch(TError& errToCatch)
    {
        if (facMediaRepoMgr.bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
        strFailReason = errToCatch.strErrText();
        return kCIDLib::False;
    }

    //
    //  If it's a failure status, then show an error. Otherwise, has to be
    //  OK status or doesn't make any sense.
    //
    if (eRet == tMediaRepoMgr::EMMStatus::Failed)
    {
        return kCIDLib::False;
    }
     else if (eRet == tMediaRepoMgr::EMMStatus::None)
    {
        //
        //  It worked, but nothing matched. We return true, but with an empty
        //  list, so that the user can try doing a manual lookup.
        //
        return kCIDLib::True;
    }
     else if (eRet != tMediaRepoMgr::EMMStatus::OK)
    {
        // This response doesn't make sense in this case
        strFailReason = L"The received status reply makes no sense in this context";
        return kCIDLib::False;
    }

    // Parse the list of possible matches
    tCIDLib::TCard4 c4AtInd;
    const TXMLTreeElement& xtnodeList = m_xtprsMeta.xtdocThis().xtnodeRoot().xtnodeFindElement
    (
        s_strElem_Titles, 0, c4AtInd
    );
    return bParseSearchResult(xtnodeList, colToFill, strFailReason);
}


// Do a search by title id, and return any potential matches
tCIDLib::TBoolean
TMyMoviesMetaSrc::bTitleSearch( const   TString&                    strSearchFor
                                ,       tMediaRepoMgr::TMetaList&   colToFill
                                ,       TString&                    strFailReason)
{
    colToFill.RemoveAll();

    tMediaRepoMgr::EMMStatus eRet;
    try
    {
        //
        //  Set up the options for the search. We need to get the repo manager
        //  configuration to set some of them.
        //
        tCIDLib::TKVPList colOpts(4);
        colOpts.objAdd(TKeyValuePair(L"title", strSearchFor));
        colOpts.objAdd
        (
            TKeyValuePair(L"includeenglish", pszBoolOptVal(m_mrsOpts.bShowEnglish()))
        );
        colOpts.objAdd
        (
            TKeyValuePair(L"includeadult", pszBoolOptVal(m_mrsOpts.bShowAdult()))
        );

        eRet = eGetServerRep
        (
            s_strCmd_TitleLookup
            , colOpts
            , m_mrsOpts.strMMName()
            , m_mrsOpts.strMMPassword()
            , strFailReason
        );
    }

    catch(TError& errToCatch)
    {
        if (facMediaRepoMgr.bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
        strFailReason = errToCatch.strErrText();
        return kCIDLib::False;
    }

    //
    //  If it's a failure status, then show an error. Otherwise, has to be
    //  OK status or doesn't make any sense.
    //
    if (eRet == tMediaRepoMgr::EMMStatus::Failed)
    {
        return kCIDLib::False;
    }
     else if (eRet == tMediaRepoMgr::EMMStatus::None)
    {
        // It worked, but nothing matched
        strFailReason = L"No match was found in the My Movies database for this disc.";
        return kCIDLib::False;
    }
     else if (eRet != tMediaRepoMgr::EMMStatus::OK)
    {
        // This response doesn't make sense in this case
        strFailReason = L"The received status reply makes no sense in this context";
        return kCIDLib::False;
    }

    // Parse the list of possible matches
    tCIDLib::TCard4 c4AtInd;
    const TXMLTreeElement& xtnodeList = m_xtprsMeta.xtdocThis().xtnodeRoot().xtnodeFindElement
    (
        s_strElem_Titles, 0, c4AtInd
    );
    return bParseSearchResult(xtnodeList, colToFill, strFailReason);
}



// ---------------------------------------------------------------------------
//  TMyMoviesMetaSrc: Private, static data members
// ---------------------------------------------------------------------------
const TString TMyMoviesMetaSrc::s_strAttr_Barcode(L"barcode");
const TString TMyMoviesMetaSrc::s_strAttr_Edition(L"edition");
const TString TMyMoviesMetaSrc::s_strAttr_ID(L"id");
const TString TMyMoviesMetaSrc::s_strAttr_MediaType(L"type");
const TString TMyMoviesMetaSrc::s_strAttr_Quality(L"completepercentage");
const TString TMyMoviesMetaSrc::s_strAttr_Status(L"status");
const TString TMyMoviesMetaSrc::s_strAttr_Title(L"title");
const TString TMyMoviesMetaSrc::s_strAttr_Year(L"year");
const TString TMyMoviesMetaSrc::s_strCmd_BarcodeLookup(L"SearchDiscTitleByBarcode");
const TString TMyMoviesMetaSrc::s_strCmd_LoadByID(L"LoadDiscTitleById");
const TString TMyMoviesMetaSrc::s_strCmd_DiscIDLookup(L"SearchDiscTitleByDiscIds");
const TString TMyMoviesMetaSrc::s_strCmd_GetRemainingLookups(L"GetRemainingLookups");
const TString TMyMoviesMetaSrc::s_strCmd_TitleLookup(L"SearchDiscTitleByTitle");
const TString TMyMoviesMetaSrc::s_strElem_Reponse(L"Response");
const TString TMyMoviesMetaSrc::s_strElem_Title(L"Title");
const TString TMyMoviesMetaSrc::s_strElem_Titles(L"Titles");



// ---------------------------------------------------------------------------
//  TMyMoviesMetaSrc: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Parses out a single disc or title search result and puts the result into the
//  provided match object.
//
tCIDLib::TBoolean
TMyMoviesMetaSrc::bParseMatch(  const   TXMLTreeElement&    xtnodeTitle
                                ,       TRepoMetaMatch&     rmmToFill
                                ,       TString&            strFailReason)
{
    tCIDLib::TCard4 c4Quality;
    tCIDLib::TCard4 c4Year;
    TString strBarcode;
    TString strEdition;
    TString strID;
    TString strMediaType;
    TString strTitle;

    try
    {
        // These may not be there
        bGetAttr(xtnodeTitle, s_strAttr_Barcode, strBarcode);
        bGetAttr(xtnodeTitle, s_strAttr_Edition, strEdition);
        bGetAttr(xtnodeTitle, s_strAttr_MediaType, strMediaType);

        // These have to be there
        GetAttr(xtnodeTitle, s_strAttr_ID, strID);
        GetAttr(xtnodeTitle, s_strAttr_Title, strTitle);

        c4Quality = c4GetAttrAsCard(xtnodeTitle, s_strAttr_Quality, 0);
        c4Year = c4GetAttrAsCard(xtnodeTitle, s_strAttr_Year);
    }

    catch(TError& errToCatch)
    {
        strFailReason = errToCatch.strErrText();
        return kCIDLib::False;
    }

    rmmToFill.Set
    (
        strID, strTitle, strEdition, c4Year, strBarcode, strMediaType, c4Quality
    );
    return kCIDLib::True;
}


//
//  When we do an id or title search, we get back a list of one or more possible
//  matches. This will parse that info and fill in the list of match items.
//
tCIDLib::TBoolean
TMyMoviesMetaSrc::bParseSearchResult(const  TXMLTreeElement&            xtnodeTitles
                                    ,       tMediaRepoMgr::TMetaList&   colToFill
                                    ,       TString&                    strFailReason)
{
    TRepoMetaMatch rmmCur;

    // Get the number of child elements and let's just iterate through them
    tCIDLib::TCard4 c4Keepers = 0;
    const tCIDLib::TCard4 c4Count = xtnodeTitles.c4ChildCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        // Get the title element at the current index
        const TXMLTreeElement& xtnodeTitle = xtnodeTitles.xtnodeChildAtAsElement(c4Index);

        // Parse out the contents to a match object
        if (bParseMatch(xtnodeTitle, rmmCur, strFailReason))
        {
            // Add it sorted by quality
            tCIDLib::TCard4 c4At;
            colToFill.InsertSorted(rmmCur, TRepoMetaMatch::eSortByQuality, c4At);

            // Bump the count of keepers
            c4Keepers++;
        }
    }

    //
    //  Return true if we had any we could keep. If not, then the last one that
    //  failed should have left its fail reason there in the output parm.
    //
    return (c4Keepers != 0);
}


//
//  Given a title details response for a title, however it's gotten, this will
//  parse out the data and fill in the collection and get the cover art image.
//
tCIDLib::TBoolean
TMyMoviesMetaSrc::bParseTitleResult(const   TXMLTreeElement&    xtnodeTitle
                                    ,       TMediaCollect&      mcolToFill
                                    ,       TJPEGImage&         imgArt
                                    ,       tCIDLib::TStrList&  colCatNames
                                    ,       TString&            strFailReason)
{
    // Elements that we get back and look for in the XML
    const TString   strElem_Aspect(L"AspectRatio");
    const TString   strElem_Covers(L"Covers");
    const TString   strElem_Descr(L"Description");
    const TString   strElem_OrgTitle(L"OriginalTitle");
    const TString   strElem_LocalTitle(L"LocalTitle");
    const TString   strElem_Front(L"Front");
    const TString   strElem_Genre(L"Genre");
    const TString   strElem_Genres(L"Genres");
    const TString   strElem_Persons(L"Persons");
    const TString   strElem_Name(L"Name");
    const TString   strElem_RunTime(L"RunningTime");
    const TString   strElem_Studio(L"Studio");
    const TString   strElem_Studios(L"Studios");
    const TString   strElem_Type(L"Type");
    const TString   strElem_UPC(L"Barcode");
    const TString   strElem_Year(L"ProductionYear");

    // Attribute or text values we look for specifically
    const TString   strAttr_BigThumb(L"BigThumb");
    const TString   strAttr_Medium(L"Medium");

    const TString   strVal_Actor(L"Actor");
    const TString   strVal_Director(L"Director");

    //
    //  OK, now we have to parse the results and get it out and into the
    //  collection object.
    //
    try
    {
        tCIDLib::TCard4 c4AtInd;
        TString         strVal;

        // Get the aspect ratio. Remove any :1 at the end
        GetChildText(xtnodeTitle, strElem_Aspect, strVal);
        if (strVal.bEndsWith(L":1"))
            strVal.CapAt(strVal.c4Length() - 2);
        mcolToFill.strAspectRatio(strVal);

        // Get the description
        GetChildText(xtnodeTitle, strElem_Descr, strVal);
        mcolToFill.strDescr(strVal);

        //
        //  Get the title. We check for for original title, then local title. One
        //  of them should work.
        //
        strVal.Clear();
        GetChildText(xtnodeTitle, strElem_OrgTitle, strVal, kCIDLib::False);
        if (strVal.bIsEmpty())
            GetChildText(xtnodeTitle, strElem_LocalTitle, strVal, kCIDLib::False);
        mcolToFill.strName(strVal);

        // Get the media format
        GetChildText(xtnodeTitle, strElem_Type, strVal);
        mcolToFill.strMediaFormat(strVal);

        // Get the running time, adjust to seconds for us
        mcolToFill.c4Duration(c4GetChildTextAsCard(xtnodeTitle, strElem_RunTime) * 60);

        // Get the UPC code
        GetChildText(xtnodeTitle, strElem_UPC, strVal);
        mcolToFill.strUPC(strVal);

        // Get the year
        mcolToFill.c4Year(c4GetChildTextAsCard(xtnodeTitle, strElem_Year));

        //
        //  Now we have to do some more complicated nested ones.
        //
        const TXMLTreeElement* pxtnodeTmp1;
        const TXMLTreeElement* pxtnodeTmp2;

        //
        //  Look for the Covers element and let's scan through it to find the Front
        //  cover image, and then get the Medium image and download that. If there's
        //  no Medium, we look for the big thumb.
        //
        pxtnodeTmp1 = xtnodeTitle.pxtnodeFindElement(strElem_Covers, 0, c4AtInd);
        if (pxtnodeTmp1)
        {
            pxtnodeTmp2 = pxtnodeTmp1->pxtnodeFindElement(strElem_Front, 0, c4AtInd);
            if (pxtnodeTmp2)
            {
                strVal.Clear();
                if (pxtnodeTmp2->bAttrExists(strAttr_Medium, c4AtInd))
                    strVal = pxtnodeTmp2->xtattrAt(c4AtInd).strValue();
                else if (pxtnodeTmp2->bAttrExists(strAttr_BigThumb, c4AtInd))
                    strVal = pxtnodeTmp2->xtattrAt(c4AtInd).strValue();

                // If we got an image URL, then download it if we can
                if (!strVal.bIsEmpty())
                {
                    THTTPClient httpcImg;
                    facMediaRepoMgr.bExtractImg(httpcImg, strVal, imgArt, m_mbufData);
                }
            }
        }

        //
        //  Look for the Studios element, then get the text of the first child of
        //  it, if any.
        //
        pxtnodeTmp1 = xtnodeTitle.pxtnodeFindElement(strElem_Studios, 0, c4AtInd);
        if (pxtnodeTmp1)
        {
            GetChildText(*pxtnodeTmp1, strElem_Studio, strVal);
            mcolToFill.strLabel(strVal);
        }

        //
        //  Look for the Persons element. We scan through that look for actor and
        //  director entries.
        //
        pxtnodeTmp1 = xtnodeTitle.pxtnodeFindElement(strElem_Persons, 0, c4AtInd);
        if (pxtnodeTmp1)
        {
            TString strCast;
            const tCIDLib::TCard4 c4Count = pxtnodeTmp1->c4ChildCount();
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            {
                const TXMLTreeElement& xtnodePerson
                                = pxtnodeTmp1->xtnodeChildAtAsElement(c4Index);

                // See what type it is
                GetChildText(xtnodePerson, strElem_Type, strVal);
                if (strVal == strVal_Actor)
                {
                    // Add this one as an actor
                    GetChildText(xtnodePerson, strElem_Name, strVal);
                    if (!strVal.bIsEmpty())
                    {
                        if (strCast.bIsEmpty())
                        {
                            // For the first one, make that the lead actor
                            mcolToFill.strLeadActor(strVal);
                        }
                         else
                        {
                            // Not the first one, appeand a separator
                            strCast.Append(L",");
                        }
                        strCast.Append(strVal);
                    }
                }
                 else if (strVal == strVal_Director)
                {
                    // If we don't have a director yet, take it
                    if (mcolToFill.strArtist().bIsEmpty())
                    {
                        GetChildText(xtnodePerson, strElem_Name, strVal);
                        mcolToFill.strArtist(strVal);
                    }
                }
            }

            // Store the cast info
            mcolToFill.strCast(strCast);
        }

        //
        //  Look for the Genres element. We go through all the children of that
        //  element (Genre elements) and grab the text of them as our categories.
        //
        pxtnodeTmp1 = xtnodeTitle.pxtnodeFindElement(strElem_Genres, 0, c4AtInd);
        if (pxtnodeTmp1)
        {
            const tCIDLib::TCard4 c4Count = pxtnodeTmp1->c4ChildCount();
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            {
                const TXMLTreeElement& xtnodeGenre
                                = pxtnodeTmp1->xtnodeChildAtAsElement(c4Index);

                GetText(xtnodeGenre, strVal);
                if (!strVal.bIsEmpty())
                    colCatNames.objAdd(strVal);
            }
        }
    }

    catch(TError& errToCatch)
    {
        if (facMediaRepoMgr.bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
        strFailReason = errToCatch.strErrText();
        return kCIDLib::False;
    }
    return kCIDLib::True;
}



//
//  This is the base helper class that will send a command to the My Movies
//  server, get the reply back, and parse it into the XML tree parser member.
//  The caller can then iterate through the tree to get the info out.
//
//  We will check the response just enough to get the top level status code
//  and return that, so that each caller doesn't have to replicate this.
//
//  We accept up to 2 key/value pairs to put into the URL query. If the key
//  part is empty, it's assumed that that one isn't used. They must be used
//  sequentially, since the first empty one seen is where it stops.
//
tMediaRepoMgr::EMMStatus
TMyMoviesMetaSrc::eGetServerRep(const   TString&            strCmd
                                , const tCIDLib::TKVPList&  colOpts
                                , const TString&            strUserName
                                , const TString&            strPassword
                                ,       TString&            strFailMsg)
{
    // Let's get the URL formatted out, taking care for proper encoding of query values
    TString strURL(1024UL);

    strURL = L"https://api.mymovies.dk/?command=";
    strURL.Append(strCmd);

    // Always add the name and password
    strURL.Append(L"&username=");
    TURL::EncodeTo(strUserName, strURL, TURL::EExpTypes::Query, tCIDLib::EAppendOver::Append);
    strURL.Append(L"&password=");
    TURL::EncodeTo(strPassword, strURL, TURL::EExpTypes::Query, tCIDLib::EAppendOver::Append);

    // Add the optional values if provided
    const tCIDLib::TCard4 c4OptCnt = colOpts.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4OptCnt; c4Index++)
    {
        const TKeyValuePair& kvalCur = colOpts[c4Index];

        strURL.Append(kCIDLib::chAmpersand);
        TURL::EncodeTo(kvalCur.strKey(), strURL, TURL::EExpTypes::Query, tCIDLib::EAppendOver::Append);
        strURL.Append(kCIDLib::chEquals);
        TURL::EncodeTo(kvalCur.strValue(), strURL, TURL::EExpTypes::Query, tCIDLib::EAppendOver::Append);
    }

    // Get the URL into an URL object now
    TURL urlSrc(strURL, tCIDSock::EQualified::Full);

    // And now we can do the SSL HTTP query
    tCIDLib::TCard4 c4ResCode;
    TString strContType;
    tCIDLib::TCard4 c4Bytes = THTTPClient::c4DoSSLPageOp
    (
        tCIDNet::ESSLOps::GET
        , urlSrc
        , L"CQC MyMovies Client"
        , L"text/html"
        , strContType
        , c4ResCode
        , m_mbufData
    );

    if (c4ResCode != kCIDNet::c4HTTPStatus_OK)
    {
        facMediaRepoMgr.ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMRMgrErrs::errcMeta_GotHTTPErr
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::ReceiveFailed
            , TCardinal(c4ResCode)
        );
    }

    if (!c4Bytes)
    {
        facMediaRepoMgr.ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMRMgrErrs::errcMeta_NoData
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::ReceiveFailed
        );
    }

    //
    //  It seems to have worked, so create an XML entity source over the
    //  buffer so that we can parse it.
    //
    tCIDXML::TEntitySrcRef esrData
    (
        new TMemBufEntitySrc(L"MyMoviesReply.xml", m_mbufData, c4Bytes)
    );

    tCIDLib::TBoolean bRes = m_xtprsMeta.bParseRootEntity
    (
        esrData
        , tCIDXML::EParseOpts::None
        , tCIDXML::EParseFlags::Standard
    );

    // If it failed, thrown an exception
    if (!bRes)
    {
        const TXMLTreeParser::TErrInfo& erriFirst
                                        = m_xtprsMeta.colErrors()[0];

        TTextStringOutStream strmOut(512);
        strmOut << L"[" << erriFirst.strSystemId() << L"/"
                << erriFirst.c4Line() << L"." << erriFirst.c4Column()
                << L"] " << erriFirst.strText() << kCIDLib::FlushIt;

        //
        //  Throw an exception that indicates that the file was not valid
        //  XML. Pass along the first error that occured.
        //
        facMediaRepoMgr.ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMRMgrErrs::errcMM_CantParse
            , strmOut.strData()
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
        );
    }

    //
    //  Look at the status and return that. The root element should be a Response
    //  element, with a 'status' attribute.
    //
    const TXMLTreeElement& xtnodeRoot = m_xtprsMeta.xtdocThis().xtnodeRoot();

    TString strTmp;
    GetAttr(xtnodeRoot, s_strElem_Reponse, s_strAttr_Status, strTmp);

    tMediaRepoMgr::EMMStatus eRet;
    if (strTmp == L"ok")
        eRet = tMediaRepoMgr::EMMStatus::OK;
    else if (strTmp == L"failed")
        eRet = tMediaRepoMgr::EMMStatus::Failed;
    else if (strTmp == L"none")
        eRet = tMediaRepoMgr::EMMStatus::None;
    else
    {
        facMediaRepoMgr.ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMRMgrErrs::errcMM_UnknownStatus
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , strTmp
        );
    }

    // If it's a failed status, get the message text
    if (eRet == tMediaRepoMgr::EMMStatus::Failed)
        GetText(xtnodeRoot, strFailMsg);

    return eRet;
}


// A helper to convert a boolean to the form required for our URL options
const tCIDLib::TCh*
TMyMoviesMetaSrc::pszBoolOptVal(const tCIDLib::TBoolean bVal) const
{
    return bVal ? L"true" : L"false";
}


