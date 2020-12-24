//
// FILE NAME: CQSLRepoImp_DVDPLoader.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 12/25/2006
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
//  This file implements the DVD Profiler import
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
RTTIDecls(TCQSLImpDVDPLoader,TObject)



// ---------------------------------------------------------------------------
//  Local data and types
// ---------------------------------------------------------------------------
namespace DVDPLoader
{
    // -----------------------------------------------------------------------
    //  The names of some things we look for in the XML
    // -----------------------------------------------------------------------
    const TString   strAttr_CreditSubtype(L"CreditSubtype");
    const TString   strAttr_Director(L"Director");
    const TString   strAttr_FirstName(L"FirstName");
    const TString   strAttr_LastName(L"LastName");
    const TString   strAttr_Name("Title");

    const TString   strElem_Actor(L"/Collection/DVD/Actors/Actor");
    const TString   strElem_Actors(L"/Collection/DVD/Actors");
    const TString   strElem_AspectRatio(L"/Collection/DVD/Format/FormatAspectRatio");
    const TString   strElem_Credit(L"/Collection/DVD/Credits/Credit");
    const TString   strElem_Credits(L"/Collection/DVD/Credits");
    const TString   strElem_Descr(L"/Collection/DVD/Discs/Disc/DescriptionSideA");
    const TString   strElem_Disc(L"/Collection/DVD/Discs/Disc");
    const TString   strElem_Discs(L"/Collection/DVD/Discs");
    const TString   strElem_DVD(L"/Collection/DVD");
    const TString   strElem_Genre(L"/Collection/DVD/Genre");
    const TString   strElem_Genres(L"/Collection/DVD/Genres");
    const TString   strElem_Database(L"/Collection");
    const TString   strElem_Format(L"/Collection/DVD/Formats/Format");
    const TString   strElem_Formats(L"/Collection/DVD/Formats");
    const TString   strElem_ImgID(L"/Collection/DVD/ID");
    const TString   strElem_Location(L"/Collection/DVD/Discs/Disc/Location");
    const TString   strElem_Notes(L"/Collection/DVD/Notes");
    const TString   strElem_Overview(L"/Collection/DVD/Overview");
    const TString   strElem_Rating(L"/Collection/DVD/Rating");
    const TString   strElem_RunningTime(L"/Collection/DVD/RunningTime");
    const TString   strElem_Slot(L"/Collection/DVD/Discs/Disc/Slot");
    const TString   strElem_SortTitle(L"/Collection/DVD/Discs/Disc/SortTitle");
    const TString   strElem_Title(L"/Collection/DVD/Title");
    const TString   strElem_Studio(L"/Collection/DVD/Studio");
    const TString   strElem_Studios(L"/Collection/DVD/Studios");
    const TString   strElem_UPC(L"/Collection/DVD/UPC");
    const TString   strElem_Year(L"/Collection/DVD/ProductionYear");
}



// ---------------------------------------------------------------------------
//   CLASS: TCQSLImpDVDPLoader
//  PREFIX: dbl
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQSLImpDVDPLoader: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQSLImpDVDPLoader::TCQSLImpDVDPLoader() :

    m_bFileBased(kCIDLib::False)
    , m_c4TitlesDone(0)
    , m_colDiscs(8)
    , m_colCatNames(128)
    , m_colUniqueIds(109, TStringKeyOps)
    , m_strAllMoviesCat(kMedMsgs::midCat_AllMovies, facCQCMedia())
    , m_strDoubleNL(L"\r\n\r\n")
    , m_strFileLocPref(L"[ONLINELOCATION=")
    , m_strFileLocPrefDx(L"[ONLINELOCATIOND")
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

TCQSLImpDVDPLoader::~TCQSLImpDVDPLoader()
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
TCQSLImpDVDPLoader::bLoadIt(        tCQSLRepoImp::TRepoClient&  orbcRepoMgr
                            , const TString&                    strXMLFile
                            , const TString&                    strImagePath
                            , const TString&                    strLeaseId
                            , const tCIDLib::TBoolean           bFileBased)
{
    // Store the incoming info for later use during callbacks
    m_bFileBased = bFileBased;
    m_orbcRepoMgr = orbcRepoMgr;
    m_strFilename = strXMLFile;
    m_strImagePath = strImagePath;
    m_strLeaseId = strLeaseId;

    // And now kick off the parsing process
    const tCIDXML::EParseOpts   eOpts  = tCIDXML::EParseOpts::None;
    const tCIDXML::EParseFlags  eFlags = tCIDXML::EParseFlags::TagsNText;

    // Kick off the parsing. Let exceptions propogate back to the caller
    m_xprsThis.ParseRootEntity(m_strFilename, eOpts, eFlags);

    return kCIDLib::True;
}



// ---------------------------------------------------------------------------
//  TCQSLImpDVDPLoader: Protected, virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TCQSLImpDVDPLoader::DocCharacters(  const   TString&            strChars
                                    , const tCIDLib::TBoolean   bIsCDATA
                                    , const tCIDLib::TBoolean   bIsIgnorable
                                    , const tCIDXML::ELocations eLocation)
{
    //
    //  Note that it is theoretically possible to get chars in more than
    //  one chunk, so we append here. The target strings were all cleared
    //  at the DVD start element, so it's safe to just append.
    //
    if (m_strCurXMLPath == DVDPLoader::strElem_AspectRatio)
    {
        m_strFld_AspectRatio.Append(strChars);
    }
     else if (m_strCurXMLPath == DVDPLoader::strElem_Descr)
    {
        TDiscInfo& diCur = m_colDiscs[m_colDiscs.c4ElemCount() - 1];
        diCur.m_strDescr.Append(strChars);
    }
     else if (m_strCurXMLPath == DVDPLoader::strElem_Genre)
    {
        m_strFld_Genre.Append(strChars);
    }
     else if (m_strCurXMLPath == DVDPLoader::strElem_ImgID)
    {
        m_strFld_ImgID.Append(strChars);
    }
     else if (m_strCurXMLPath == DVDPLoader::strElem_Location)
    {
        if (!m_bFileBased)
        {
            TDiscInfo& diCur = m_colDiscs[m_colDiscs.c4ElemCount() - 1];
            diCur.m_strLocInfo.Append(strChars);
        }
    }
     else if (m_strCurXMLPath == DVDPLoader::strElem_Rating)
    {
        m_strFld_Rating.Append(strChars);
    }
     else if (m_strCurXMLPath == DVDPLoader::strElem_Notes)
    {
        if (m_bFileBased)
            m_strFld_Notes.Append(strChars);
    }
     else if (m_strCurXMLPath == DVDPLoader::strElem_Overview)
    {
        m_strFld_Overview.Append(strChars);
    }
     else if (m_strCurXMLPath == DVDPLoader::strElem_RunningTime)
    {
        m_strFld_RunningTime.Append(strChars);
    }
     else if (m_strCurXMLPath == DVDPLoader::strElem_Slot)
    {
        if (!m_bFileBased)
        {
            TDiscInfo& diCur = m_colDiscs[m_colDiscs.c4ElemCount() - 1];
            if (!diCur.m_strLocInfo.bIsEmpty())
            {
                diCur.m_strLocInfo.Append(L'.');
                diCur.m_strLocInfo.Append(strChars);
                diCur.m_strLocInfo.Append(L"\n");
            }
        }
    }
     else if (m_strCurXMLPath == DVDPLoader::strElem_SortTitle)
    {
        m_strFld_SortTitle.Append(strChars);
    }
     else if (m_strCurXMLPath == DVDPLoader::strElem_Studio)
    {
        //
        //  There can be multiple studio entries. We store up chars for
        //  the current one and append it to the actual value when we
        //  see the end of the containing element and then flush this
        //  string to start accumulating the next one.
        //
        m_strFld_CurStudio.Append(strChars);
    }
     else if (m_strCurXMLPath == DVDPLoader::strElem_Title)
    {
        m_strFld_Title.Append(strChars);
    }
     else if (m_strCurXMLPath == DVDPLoader::strElem_UPC)
    {
        m_strFld_UPC.Append(strChars);
    }
     else if (m_strCurXMLPath == DVDPLoader::strElem_Year)
    {
        m_strFld_Year.Append(strChars);
    }
}


tCIDLib::TVoid TCQSLImpDVDPLoader::EndDocument(const TXMLEntitySrc& xsrcOfRoot)
{
}


tCIDLib::TVoid TCQSLImpDVDPLoader::EndTag(const TXMLElemDecl& xdeclElem)
{
    if (m_strCurXMLPath == DVDPLoader::strElem_DVD)
    {
        // We have a title's worth of stuff, so process it
        try
        {
            ProcessTitle();
        }

        catch(const TError& errToCatch)
        {
            c4ErrCnt++;
            *pconOut    << L"An error occured while processing title.\nERROR:\n"
                        << errToCatch
                        << kCIDLib::EndLn;
        }
    }
     else if (m_strCurXMLPath == DVDPLoader::strElem_Genre)
    {
        //
        //  If we are coming out of a Genre field, then add the current
        //  genre text to the list and clear the genre text accumulator.
        //
        //  If it's a slashed or hyphenated genre, replace the slash or
        //  hyphen with a space, so the genre name can wrap when viewed.
        //
        m_strFld_Genre.bReplaceChar(kCIDLib::chHyphenMinus, kCIDLib::chSpace);
        m_strFld_Genre.bReplaceChar(kCIDLib::chForwardSlash, kCIDLib::chSpace);
        m_colCatNames.objAdd(m_strFld_Genre);
        m_strFld_Genre.Clear();
    }
     else if (m_strCurXMLPath == DVDPLoader::strElem_Notes)
    {
        //
        //  If file based, parse out the disk location info, but only
        //  if we've found any disks.
        //
        if (m_bFileBased && !m_colDiscs.bIsEmpty())
        {
            // Pull out newline separated lines
            TTextStringInStream strmSrc(&m_strFld_Notes);
            while (!strmSrc.bEndOfStream())
            {
                strmSrc >> m_strTmp;
                m_strTmp.StripWhitespace();

                //
                //  Make sure that it's a reasonable length. It has to be
                //  at least the file smallest file location prefix plus
                //  a quote and closing bracket plus a minimal path length
                //  which is going to be at least 8 characters to be of
                //  any pratical use since it must be fully qualified.
                //
                if (m_strTmp.c4Length() < m_strFileLocPref.c4Length() + 8)
                    continue;

                if (m_strTmp.bStartsWith(m_strFileLocPrefDx))
                {
                    // It's for a specific disk, so find it
                    const tCIDLib::TCard4 c4Ind
                    (
                        m_strTmp[m_strFileLocPrefDx.c4Length()] - 0x31
                    );

                    if (c4Ind < m_colDiscs.c4ElemCount())
                    {
                        TDiscInfo& diTar = m_colDiscs[c4Ind];
                        diTar.m_strLocInfo.CopyInSubStr
                        (
                            m_strTmp
                            , m_strFileLocPrefDx.c4Length() + 3
                            , kCIDLib::c4MaxCard
                        );
                        diTar.m_strLocInfo.DeleteLast();
                        diTar.m_strLocInfo.DeleteLast();
                    }
                }
                 else if (m_strTmp.bStartsWith(m_strFileLocPref))
                {
                    TDiscInfo& diTar = m_colDiscs[0];
                    diTar.m_strLocInfo.CopyInSubStr
                    (
                        m_strTmp
                        , m_strFileLocPref.c4Length() + 1
                        , kCIDLib::c4MaxCard
                    );
                    diTar.m_strLocInfo.DeleteLast();
                    diTar.m_strLocInfo.DeleteLast();

                    // And break out
                    break;
                }
                 else
                {
                    // It's not something we understand, so break out
                    break;
                }
            }
        }
        m_strFld_Notes.Clear();
    }
     else if (m_strCurXMLPath == DVDPLoader::strElem_Studio)
    {
        // Append the current studio to the overall studio field
        if (!m_strFld_Studio.bIsEmpty() && !m_strFld_CurStudio.bIsEmpty())
            m_strFld_Studio.Append(L", ");
        m_strFld_Studio.Append(m_strFld_CurStudio);
        m_strFld_CurStudio.Clear();
    }

    // Pop our XML path (which verifies the nesting for us
    PopXMLPath(xdeclElem);
}


tCIDLib::TVoid
TCQSLImpDVDPLoader::HandleXMLError( const   tCIDLib::TErrCode   errcToPost
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


tCIDLib::TVoid TCQSLImpDVDPLoader::ResetDocument()
{
}


tCIDLib::TVoid TCQSLImpDVDPLoader::StartDocument(const TXMLEntitySrc&)
{
}


tCIDLib::TVoid
TCQSLImpDVDPLoader::StartTag(       TXMLParserCore&     xprsSrc
                            , const TXMLElemDecl&       xdeclElem
                            , const tCIDLib::TBoolean   bEmpty
                            , const TVector<TXMLAttr>&  colAttrList
                            , const tCIDLib::TCard4     c4AttrCnt)
{
    // Push the new element
    const TString& strQName = xdeclElem.strFullName();
    m_strCurXMLPath.Append(L'/');
    m_strCurXMLPath.Append(strQName);

    if (m_strCurXMLPath == DVDPLoader::strElem_DVD)
    {
        // It's a new disc, so reset any per-disc data
        m_colDiscs.RemoveAll();
        m_colCatNames.RemoveAll();

        m_strFld_AspectRatio.Clear();
        m_strFld_Cast.Clear();
        m_strFld_CreditSubtype.Clear();
        m_strFld_CurStudio.Clear();
        m_strFld_Director.Clear();
        m_strFld_Genre.Clear();
        m_strFld_ImgID.Clear();
        m_strFld_LeadActor.Clear();
        m_strFld_Notes.Clear();
        m_strFld_Overview.Clear();
        m_strFld_Rating.Clear();
        m_strFld_RunningTime.Clear();
        m_strFld_SortTitle.Clear();
        m_strFld_Studio.Clear();
        m_strFld_Title.Clear();
        m_strFld_UPC.Clear();
        m_strFld_Year.Clear();
    }
     else
    {
        // If a new disc, then add a disc info object
        if (m_strCurXMLPath == DVDPLoader::strElem_Disc)
            m_colDiscs.objAdd(TDiscInfo());

        //
        //  Watch for some stuff we need to pull from attributes on some
        //  elements.
        //
        if (m_strCurXMLPath == DVDPLoader::strElem_Actor)
        {
            bFindAttr(DVDPLoader::strAttr_FirstName, colAttrList, c4AttrCnt, m_strTmp);
            bFindAttr(DVDPLoader::strAttr_LastName, colAttrList, c4AttrCnt, m_strTmp2);

            //
            //  If this is the first actor, then store it as the name of the
            //  lead actor. Else, we want to put out a trailing comma to the
            //  cast string in preperartion for the new item.
            //
            if (m_strFld_Cast.bIsEmpty())
            {
                m_strFld_LeadActor = m_strTmp;
                if (!m_strTmp.bIsEmpty() && !m_strTmp2.bIsEmpty())
                    m_strFld_LeadActor.Append(L' ');
                m_strFld_LeadActor.Append(m_strTmp2);
            }
             else if (!m_strTmp.bIsEmpty() || !m_strTmp2.bIsEmpty())
            {
                //
                //  We have a name, and it's not the first one, so append a
                //  comma to the cast to make way for the next one.
                //
                m_strFld_Cast.Append(L", ");
            }

            m_strFld_Cast.Append(m_strTmp);
            if (!m_strTmp.bIsEmpty() && !m_strTmp2.bIsEmpty())
                m_strFld_Cast.Append(kCIDLib::chSpace);
            m_strFld_Cast.Append(m_strTmp2);
        }
         else if (m_strCurXMLPath == DVDPLoader::strElem_Credit)
        {
            // If there's a credit sub-type attr, pull that out
            if (bFindAttr(DVDPLoader::strAttr_CreditSubtype, colAttrList, c4AttrCnt, m_strTmp)
            &&  (m_strTmp == DVDPLoader::strAttr_Director))
            {
                bFindAttr(DVDPLoader::strAttr_FirstName, colAttrList, c4AttrCnt, m_strTmp);
                bFindAttr(DVDPLoader::strAttr_LastName, colAttrList, c4AttrCnt, m_strTmp2);

                m_strFld_Director = m_strTmp;
                if (!m_strTmp.bIsEmpty() && !m_strTmp2.bIsEmpty())
                    m_strFld_Director.Append(L' ');
                m_strFld_Director.Append(m_strTmp2);
            }
        }
    }
}



// ---------------------------------------------------------------------------
//  TCQSLImpDVDPLoader: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  We need to get some info out of the attributes of some start elements,
//  so we have this helper to search for the attribute value we want from
//  the list of attributes that the start element callback got.
//
tCIDLib::TBoolean
TCQSLImpDVDPLoader::bFindAttr(  const   TString&            strQName
                                , const TVector<TXMLAttr>&  colAttrList
                                , const tCIDLib::TCard4     c4ListSz
                                ,       TString&            strValue)
{
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ListSz; c4Index++)
    {
        if (colAttrList[c4Index].strQName() == strQName)
        {
            strValue = colAttrList[c4Index].strValue();
            return kCIDLib::True;
        }
    }
    return kCIDLib::False;
}


//
//  Pops the last element off of the current XML path and verifies that
//  that element is the one that we got that's ending. We consider any
//  errors here to be fatal and exit.
//
tCIDLib::TVoid TCQSLImpDVDPLoader::PopXMLPath(const TXMLElemDecl& xdeclEnding)
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
    m_strCurXMLPath.CopyOutSubStr(m_strTmp, c4Ofs + 1);
    if (m_strTmp != xdeclEnding.strFullName())
    {
        *pconOut    << L"XML nesting error. Got end of element "
                    << xdeclEnding.strFullName() << L"but expected the end "
                    << L"of element " << m_strTmp << L". Exiting..."
                    << kCIDLib::EndLn;
        TProcess::ExitProcess(tCIDLib::EExitCodes::RuntimeError);
    }

    // Looks ok, cap the path at the divider
    m_strCurXMLPath.CapAt(c4Ofs);
}


// We have all the info for a title, so upload it
tCIDLib::TVoid TCQSLImpDVDPLoader::ProcessTitle()
{
    // If we found no discs for this title, then nothing to do
    if (m_colDiscs.bIsEmpty())
    {
        c4NoDiscCnt++;
        return;
    }

    //
    //  There's some sort of problem, probably occuring in the import of
    //  the old 2.x database into the new 3.0 DVDP. Some of the UPC codes
    //  end up with a bogus bit of prefix text.
    //
    if (m_strFld_UPC.bStartsWith(L"Disc ID: "))
        m_strFld_UPC.Cut(0, 9);

    // If the image id is empty we can't do this one
    if (m_strFld_ImgID.bIsEmpty())
    {
        c4SkippedCnt++;
        *pconOut << L"The disc ID was empty, skipping title ended at line "
                 << m_xprsThis.c4CurLine()
                 << kCIDLib::EndLn;
        return;
    }

    //
    //  We put a prefix on it to make sure it's obvious later that it's a
    //  DVDP provided id, and not one of the CQSL repo's ids. But save the
    //  the original since we need that to access the cover art.
    //
    const TString strUniqueId(L"DVDP_", m_strFld_ImgID);

    //
    //  And now we need to process the data we've collected on the current
    //  title. If it already exists, then ignore this one since it's a dup.
    //
    if (m_colUniqueIds.bHasElement(strUniqueId))
    {
        c4SkippedCnt++;
        *pconOut  << L"DVD with ID "
                  << m_strFld_ImgID
                  << L" is already in the database. Skipping title ended at line"
                  << m_xprsThis.c4CurLine()
                  << kCIDLib::EndLn;
        return;
    }

    // Not there, so add it
    m_colUniqueIds.objAdd(strUniqueId);

    //
    //  DVDP uses a unique image for every disc, and it's based on the disc
    //  unique id, which we checked above is unique. So we can just add
    //  this guy's without checking for dup art paths.
    //
    tCIDLib::TCard4 c4ArtSz = 0;
    if (!m_strFld_ImgID.bIsEmpty())
    {
        m_pathTmp = m_strImagePath;
        m_pathTmp.AddLevel(m_strFld_ImgID);
        m_pathTmp.Append(L'f');
        m_pathTmp.AppendExt(L"jpg");

        // Load up this image to our buffer member
        if (TFileSys::bExists(m_pathTmp))
        {
            TBinaryFile flImg(m_pathTmp);
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
                *pconOut << L"Could not read image for: '" << m_strFld_Title
                         << L"'. No image will be uploaded."
                         << kCIDLib::NewLn;
                c4ArtSz = 0;
            }
        }
    }

    // If we got no art and not in test mode, then upload a default image
    if (!c4ArtSz)
    {
        c4MissingArtCnt++;
        if (!bTestMode)
        {
            TBinMBufOutStream strmOut(&m_mbufArtUpload);
            strmOut << m_imgDefArt << kCIDLib::FlushIt;
            c4ArtSz = strmOut.c4CurSize();
        }
    }

    //
    //  We don't get separate year/duration values for each disc, so just
    //  get them out now and use them for all the discs below. Not optimal
    //  but the best we can do.
    //
    tCIDLib::TBoolean   bOk;
    tCIDLib::TCard4     c4Duration = 0;
    tCIDLib::TCard4     c4Year = 0;
    if (!m_strFld_RunningTime.bIsEmpty())
    {
        c4Duration = TRawStr::c4AsBinary
        (
            m_strFld_RunningTime.pszBuffer(), bOk, tCIDLib::ERadices::Dec
        );
        if (!bOk)
            c4Year = 0;

        // It's in minutes, we want seconds
        c4Duration *= 60;
    }

    if (!m_strFld_Year.bIsEmpty())
    {
        c4Year = TRawStr::c4AsBinary
        (
            m_strFld_Year.pszBuffer(), bOk, tCIDLib::ERadices::Dec
        );
        if (!bOk)
            c4Year = 0;
    }

    //
    //  Set up the title set. Note that, if we have multiple discs,
    //  the first upload will update this guy to reflect it's stored
    //  contents, and so on the subsequent uploads it will will be
    //  correctly identified and all of the discs will end up in this
    //  same title set.
    //
    TMediaTitleSet mtsUpload
    (
        m_strFld_Title
        , strUniqueId
        , m_strFld_SortTitle
        , 0
        , tCQCMedia::EMediaTypes::Movie
    );

    // Create any categories that this set's collections are in
    tCQCMedia::TIdList fcolCatIds;
    m_orbcRepoMgr->CreateCategories
    (
        m_strLeaseId, m_colCatNames, tCQCMedia::EMediaTypes::Movie, fcolCatIds
    );

    //
    //  For each collection we find, set it up and upload it
    //
    tCIDLib::TCard4     c4DiscCnt = m_colDiscs.c4ElemCount();
    TVector<TMediaItem> colItemsUpload;

    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4DiscCnt; c4Index++)
    {
        TDiscInfo& diCur = m_colDiscs[c4Index];
        diCur.m_strLocInfo.StripWhitespace();
        if (diCur.m_strLocInfo.bIsEmpty())
            continue;

        // Just use the title set unique id plus a suffix for the collections
        m_strTmp = strUniqueId;
        m_strTmp.AppendFormatted(c4Index + 1);
        TMediaCollect mcolUpload
        (
            diCur.m_strDescr
            , m_strTmp
            , 0
            , c4Year
            , !m_bFileBased
              ? tCQCMedia::ELocTypes::Changer : tCQCMedia::ELocTypes::FileCol
            , tCQCMedia::EMediaTypes::Movie
        );

        // Store the other info on the collection
        mcolUpload.c4Duration(c4Duration);

        // Set the category ids on the collection
        mcolUpload.SetCatIds(fcolCatIds);

        //
        //  These guys use two new lines between paragraphs, which makes the
        //  text look bad when viewed, with big gaps. So we do a search for
        //  them and replace them with single new lines.
        //
        //  Note that they were expanded from entity references, they will
        //  remain as CR\LF pairs, and won't have been reduced to LFs by
        //  either the XML parser or our text streams.
        //
        tCIDLib::TCard4 c4Ofs;
        if (m_strFld_Overview.bFirstOccurrence(m_strDoubleNL, c4Ofs))
        {
            do
            {
                m_strFld_Overview[c4Ofs + 0] = kCIDLib::chSpace;
                m_strFld_Overview[c4Ofs + 1] = kCIDLib::chSpace;
                m_strFld_Overview[c4Ofs + 2] = kCIDLib::chLF;
                m_strFld_Overview[c4Ofs + 3] = kCIDLib::chLF;
            }
            while (m_strFld_Overview.bNextOccurrence(m_strDoubleNL, c4Ofs));
        }
        mcolUpload.strDescr(m_strFld_Overview);

        // Set all the other collection info we gathered
        mcolUpload.strArtist(m_strFld_Director);
        mcolUpload.strAspectRatio(m_strFld_AspectRatio);
        mcolUpload.strCast(m_strFld_Cast);
        mcolUpload.strLabel(m_strFld_Studio);
        mcolUpload.strRating(m_strFld_Rating);
        mcolUpload.strLeadActor(m_strFld_LeadActor);
        mcolUpload.strUPC(m_strFld_UPC);

        //
        //  Set the location info on the collection. This driver either
        //  uses per-disc file or changer location type, both of which
        //  imply a single location, stored at the collection level.
        //
        mcolUpload.strLocInfo(diCur.m_strLocInfo);

        // Make a unique id that's the collection id plus a suffix
        m_strTmp = mcolUpload.strUniqueId();
        m_strTmp.Append(L"-1");

        // Add a single chapter item that represents the whole movie contents
        colItemsUpload.RemoveAll();
        TMediaItem mitemUpload
        (
            mcolUpload.strName()
            , m_strTmp
            , 0
            , TString::strEmpty()
            , tCQCMedia::EMediaTypes::Movie
        );
        mitemUpload.strArtist(m_strFld_Director);
        colItemsUpload.objAdd(mitemUpload);

        // OK, let's upload this collection
        if (!bTestMode)
        {
            *pconOut << L"Uploading: " << mtsUpload.strName() << kCIDLib::EndLn;
            m_orbcRepoMgr->UploadNewMeta
            (
                m_strLeaseId
                , tCQCMedia::EMediaTypes::Movie
                , m_colCatNames
                , mtsUpload
                , mcolUpload
                , colItemsUpload
                , c4ArtSz
                , m_mbufArtUpload
            );

            // Don't beat the server to death. Pause a bit between each one
            TThread::Sleep(100);
        }
    }
}


