//
// FILE NAME: CQCDocComp.Cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/31/2015
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
//  This is the main module for a utility used to generate documentation. We
//  write our CQC documentation in an XML marked up format, and this program
//  reads all of those files resolves links, creates indices, and spits out HTML
//  documentation that we can package up as part of the installation and invoke
//  from the client applications based on topic.
//
//  The files are in CQC_Dev\Source\Docs\DocSrc. There are some top level files,
//  including any CSS and Javascript we might want to include. Under that is a
//  Src directory that includes any top level actual content, and below that is
//  a single level set of major category directories. Those are used to categorize
//  all topics in the category index.
//
//  So we read the whole tree of files in DocSrc\Src and load them up into a class
//  that holds all of the info. Once they are all loaded successfully, we then
//  process them, then go back and write out the output, to the output directory.
//
//  The output is sent to Web2/CQCDocs under our own HTMLRoot directory, so that it
//  will have the same path during testing as it will on the real web site.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $_CIDLib_Log_$
//


// ----------------------------------------------------------------------------
//  Includes
// ----------------------------------------------------------------------------
#include    "CQCDocComp.hpp"


// ----------------------------------------------------------------------------
//  Magic macros
// ----------------------------------------------------------------------------
RTTIDecls(TFacCQCDocComp,TFacility)



// ----------------------------------------------------------------------------
//  Forward references
// ----------------------------------------------------------------------------
tCIDLib::EExitCodes eMainThread(TThread&, tCIDLib::TVoid*);



// ----------------------------------------------------------------------------
//  Some local helpers for sorting things
// ----------------------------------------------------------------------------

//
//  This one sorts by driver make. The page pointers in these page info objects should
//  point to driver pages, so we can cast it and do the comparison based on the makes
//  inside them. If the makes are equal, we sort by the model, so sub-sort models within
//  each make.
//
static tCIDLib::ESortComps
eCompDriverByMake(const TPageInfo& pinfo1, const TPageInfo& pinfo2)
{
    const TDevDriverPage* ppg1 = static_cast<const TDevDriverPage*>(pinfo1.m_ppgThis);
    const TDevDriverPage* ppg2 = static_cast<const TDevDriverPage*>(pinfo2.m_ppgThis);

    //
    //  Figure out the make and model, which may be overrides in case of aliased drivers
    //  (where the same driver is used under the hood for multiple driver makes/models.)
    //  Else they are in the driver file.
    //
    const TString& strMake1
    (
        pinfo1.m_strExtra1.bIsEmpty() ? ppg1->m_strMake : pinfo1.m_strExtra1
    );
    const TString& strMake2
    (
        pinfo2.m_strExtra1.bIsEmpty() ? ppg2->m_strMake : pinfo2.m_strExtra1
    );

    tCIDLib::ESortComps eComp = strMake1.eCompare(strMake2);
    if (eComp == tCIDLib::ESortComps::Equal)
    {
        // Do the same as above, for th emodel this time
        const TString& strModel1
        (
            pinfo1.m_strExtra2.bIsEmpty() ? ppg1->m_strModel : pinfo1.m_strExtra2
        );
        const TString& strModel2
        (
            pinfo2.m_strExtra2.bIsEmpty() ? ppg2->m_strModel : pinfo2.m_strExtra2
        );
        eComp = strModel1.eCompare(strModel2);
    }
    return eComp;
}



// ----------------------------------------------------------------------------
//  Include magic main module code
// ----------------------------------------------------------------------------
CIDLib_MainModule
(
    TThread
    (
        L"CQCDocCompThread"
        , TMemberFunc<TFacCQCDocComp>(&facCQCDocComp, &TFacCQCDocComp::eMainThread)
    )
)



// ----------------------------------------------------------------------------
//  Global data
// ----------------------------------------------------------------------------
TFacCQCDocComp facCQCDocComp;



// ---------------------------------------------------------------------------
//   CLASS: TFacCQCDocComp
//  PREFIX: fac
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TFacCQCDocComp: Public, static data members
// ---------------------------------------------------------------------------
const TString   TFacCQCDocComp::s_strExt_ActionTar(L"cqcact");
const TString   TFacCQCDocComp::s_strExt_CMLClass(L"cqccml");
const TString   TFacCQCDocComp::s_strExt_DevClass(L"cqcdevcls");
const TString   TFacCQCDocComp::s_strExt_Driver(L"cqcdrv");
const TString   TFacCQCDocComp::s_strExt_EventFilter(L"cqcevf");
const TString   TFacCQCDocComp::s_strExt_HelpPage(L"cqchpage");
const TString   TFacCQCDocComp::s_strExt_IntfWidget(L"cqciwdg");

const TString   TFacCQCDocComp::s_strColSpan(L"ColSpan");
const TString   TFacCQCDocComp::s_strExtra(L"Extra");
const TString   TFacCQCDocComp::s_strExtra1(L"Extra1");
const TString   TFacCQCDocComp::s_strExtra2(L"Extra2");
const TString   TFacCQCDocComp::s_strFileExt(L"FileExt");
const TString   TFacCQCDocComp::s_strFileName(L"FileName");
const TString   TFacCQCDocComp::s_strId(L"Id");
const TString   TFacCQCDocComp::s_strPageLink(L"PageLink");
const TString   TFacCQCDocComp::s_strPageMap(L"PageMap");
const TString   TFacCQCDocComp::s_strRef(L"Ref");
const TString   TFacCQCDocComp::s_strRoot(L"/");
const TString   TFacCQCDocComp::s_strSubDir(L"SubDir");
const TString   TFacCQCDocComp::s_strSubTopicMap(L"SubTopicMap");
const TString   TFacCQCDocComp::s_strTitle(L"Title");
const TString   TFacCQCDocComp::s_strTopicPage(L"TopicPage");
const TString   TFacCQCDocComp::s_strType(L"Type");
const TString   TFacCQCDocComp::s_strV2Compat(L"  <Superscript>[V2]</Superscript>");
const TString   TFacCQCDocComp::s_strVirtual(L"Virtual");



// ---------------------------------------------------------------------------
//  TFacCQCDocComp: Constructors and Destructor
// ---------------------------------------------------------------------------
TFacCQCDocComp::TFacCQCDocComp() :

    TFacility
    (
        L"CQCDocComp"
        , tCIDLib::EModTypes::Exe
        , kCQCKit::c4MajVersion
        , kCQCKit::c4MinVersion
        , kCQCKit::c4Revision
        , tCIDLib::EModFlags::HasMsgFile
    )
    , m_bVerbose(kCIDLib::False)
    , m_colDriverPages(tCIDLib::EAdoptOpts::NoAdopt)
    , m_strmErr(tCIDLib::EStdFiles::StdErr)
    , m_pstrmOut(nullptr)
{
}

TFacCQCDocComp::~TFacCQCDocComp()
{
}


// ----------------------------------------------------------------------------
//  TFacCQCDocComp: Private, non-virtual methods
// ----------------------------------------------------------------------------

//
//  This is called after all loading is done, but before output generation. We spit out
//  some dynamic source files that provide alternative indices for the available drivers.
//
tCIDLib::TBoolean TFacCQCDocComp::bGenerateDriverIndices()
{
    //
    //  We need to go through the page info objects we stored in the drivers list and
    //  create a new list that we can sort by make. It's not stored in the page info objects,
    //  and the actual pages don't represents all of the
    //
    {
        // First let's sort the drivers list by make then model
        m_colDriverPages.Sort(eCompDriverByMake);

        // We will format this out to a string and parse from there
        TTextStringOutStream strmTar(32 * 1024UL);

        // Spit out the standard help page stuff at the top
        OutputStdHPHeader(strmTar, L"By Make Index");

        //
        //  Now let's spit them out. We do it as a doubly nested list. The outer
        //  list is the makes, and the inner list is the page links.
        //
        strmTar << L"<List Type=\"Unordered\">\n";

        TString strLastMake;
        const tCIDLib::TCard4 c4Count = m_colDriverPages.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const TPageInfo& pinfoCur = *m_colDriverPages[c4Index];
            const TDevDriverPage* ppgCur = static_cast<const TDevDriverPage*>(pinfoCur.m_ppgThis);

            // Figure out the actual make/model values
            const TString& strMake
            (
                pinfoCur.m_strExtra1.bIsEmpty() ? ppgCur->m_strMake : pinfoCur.m_strExtra1
            );

            const TString& strModel
            (
                pinfoCur.m_strExtra2.bIsEmpty() ? ppgCur->m_strModel : pinfoCur.m_strExtra2
            );

            //
            // If we have changed makes, then start another list and close off the
            //  previous one if there was a previous. If the last key is empty,
            //  this is the first one.
            //
            if (!strLastMake.bCompareI(strMake))
            {
                // If an outstanding list, close it off
                if (!strLastMake.bIsEmpty())
                    strmTar << L"</List></LItem>\n";

                // And start a new one
                strLastMake = strMake ;
                strmTar << L"<LItem><Bold>";
                facCIDXML().EscapeFor(strMake, strmTar, tCIDXML::EEscTypes::ElemText);
                strmTar << L"</Bold>\n<List Type=\"Unordered\">\n";
            }

            // And output an item for this one, which is the link to the page
            strmTar << L"<LItem><Link Type=\"Page\" Ref=\"" << pinfoCur.m_strTopicPath
                    << L"\" Id=\"" << pinfoCur.m_strFile << L"\">";
            facCIDXML().EscapeFor(strModel, strmTar, tCIDXML::EEscTypes::ElemText);
            strmTar << L"</Link>";
            if (ppgCur->m_bV2Compat)
                strmTar << s_strV2Compat;
            strmTar << L"</LItem>\n";
        }


        // And close it all off
        strmTar << L"    </List></LItem></List>\n</HelpText></HelpPage>\n\n"
                << kCIDLib::FlushIt;

        //
        //  Now let's create a page for it and add it to the main drivers section topic
        //  which is where it were it should show up. We don't need to add the page info
        //  object to the topic's page map, it's already there from the topic file (marked
        //  virtual so that it was skipped during the initial parsing pass). We need to
        //  look up the top level drivers topic to add this page to though.
        //
        TTopic& topicDrivers = topicFindByPath(L"/Reference/Drivers");
        THelpPage* ppgIndex = new THelpPage();
        topicDrivers.m_colParsedPages.Add(ppgIndex);

        // Now parse the content and let the page fill itself in
        TPageInfo pinfoFake;
        pinfoFake.m_strExt = s_strExt_HelpPage;
        pinfoFake.m_strFile = L"ByMakeIndex";
        pinfoFake.m_strTitle = L"By Make Index";
        const tCIDLib::TBoolean bRes = bParsePageFile
        (
            strmTar.strData()
            , pinfoFake
            , *ppgIndex
            , topicDrivers
            , kCIDLib::False
        );

        if (!bRes)
            return kCIDLib::False;
    }

    return kCIDLib::True;
}


//
//  This is called after all the loading is done, but before output generation. We read
//  in the special IR models file and generate dynamic source files for each supported
//  IR blaster device, which provides a list of the IR model files available for download
//  and provides links for them.
//
//  This file is very simple. It's plain text, no empty lines. Each line is:
//
//      make, model, filename
//
//  So we can just read them in, break the values out, and spit out the data. We can just
//  do it all as we are generating the file.
//
//  All of the device models currently support all of the files, because we generate them
//  all from a common list using UIRT data that we can convert to the formats used by the
//  other devices. SO, for now we don't need any indication of what devices a given model
//  can support.
//
//  The format of the output is a table, with one row per model.
//
tCIDLib::TBoolean TFacCQCDocComp::bGenerateIRModelIndex()
{
    TPathStr pathIRMs(m_strSrcPath);
    pathIRMs.AddLevel(L"Special");
    pathIRMs.AddLevel(L"IRModels");
    pathIRMs.AppendExt(L"Txt");

    try
    {
        TTextFileInStream strmIRMs
        (
            pathIRMs
            , tCIDLib::ECreateActs::OpenIfExists
            , tCIDLib::EFilePerms::Default
            , tCIDLib::EFileFlags::SequentialScan
            , tCIDLib::EAccessModes::Excl_Read
            , facCIDEncode().ptcvtMake(L"Latin1")
        );

        //
        //  We got the file, so let's start generating the output file. We will actually
        //  generate the output into a string, and then parse from that.
        //
        TTextStringOutStream strmTar(64 * 1024UL);

        // Spit out the standard help page stuff at the top
        OutputStdHPHeader(strmTar, L"IR Model Downloads");

        // Some introductory remarks
        strmTar << L"<P>This list is sorted by make, and then model. So find the make/model "
                   L"you want, and then select the download link for the appropriate blaster "
                   L"device.</P>";

        // Create the opening part of the table and the header row
        strmTar << L"<Table Class=\"GenHdrTable\">\n"
                   L"<TableRow><TableCol>Make</TableCol><TableCol>Model</TableCol>"
                   L"<TableCol>Global Cache</TableCol><TableCol>R2DI</TableCol>"
                   L"<TableCol>USB-UIRT</TableCol></TableRow>\n";

        // Now for each line in the source file, we generate the row content
        const TString strBaseURL(L"http://www.charmedquark.com/Web2/Downloads/IRModels/");
        TString strMake;
        TString strModel;
        TString strFile;
        tCIDLib::TCard4 c4CurLine(0);
        while (!strmIRMs.bEndOfStream())
        {
            strmIRMs >> strMake;
            c4CurLine++;

            if (!strMake.bSplit(strModel, kCIDLib::chComma)
            ||  !strModel.bSplit(strFile, kCIDLib::chComma))
            {
                facCQCDocComp.ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kDocCompErrs::errcSrc_BadIRModelLine
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::NotFound
                    , TCardinal(c4CurLine)
                );
            }

            strMake.StripWhitespace();
            strModel.StripWhitespace();
            strFile.StripWhitespace();

            strmTar << L"<TableRow><TableCol>"
                    << strMake << L"</TableCol><TableCol>"
                    << strModel << L"</TableCol>";

            // Do download links for each type
            strmTar << L"<TableCol><Link Type=\"Extern\" Ref=\"" << strBaseURL
                    << L"GlobalCache/" << strMake << L"/" << strFile
                    << L".IRExport\">Download</Link></TableCol>\n";

            strmTar << L"<TableCol><Link Type=\"Extern\" Ref=\"" << strBaseURL
                    << L"R2DI/" << strMake << L"/" << strFile
                    << L".IRExport\">Download</Link></TableCol>\n";

            strmTar << L"<TableCol><Link Type=\"Extern\" Ref=\"" << strBaseURL
                    << L"USB-UIRT/" << strMake << L"/" << strFile
                    << L".IRExport\">Download</Link></TableCol>\n";

            strmTar << L"</TableRow>\n";
        }

        // And now close everything off
        strmTar << L"        </Table>\n"
                << L"    </HelpText>\n"
                <<L"</HelpPage>\n\n"
                << kCIDLib::FlushIt;


        //
        //  Now let's create a page for it and add it to the IR Models section topic
        //  which is where it were it should show up.
        //
        TTopic& topicIRModels = topicFindByPath(L"/Downloads/IRModels");
        THelpPage* ppgIndex = new THelpPage();
        topicIRModels .m_colParsedPages.Add(ppgIndex);

        // Now parse the content and let the page fill itself in
        TPageInfo pinfoFake;
        pinfoFake.m_strExt = s_strExt_HelpPage;
        pinfoFake.m_strFile = L"IRModelFiles";
        pinfoFake.m_strTitle = L"IR Model Downloads";
        const tCIDLib::TBoolean bRes = bParsePageFile
        (
            strmTar.strData()
            , pinfoFake
            , *ppgIndex
            , topicIRModels
            , kCIDLib::False
        );

        if (!bRes)
            return kCIDLib::False;
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }

    return kCIDLib::True;
}


//
//  This is called to iterate the topics. It is recursive and we are started off
//  on the root topic member, and it goes fro there.
//
//  We get the directory name separately for convenience, so we don't have to parse
//  it back out of the source path. We want to have it later when we start doing the
//  output, and need to build up the output directories.
//
//  For the top level when we start, the source and help path are / and the directory
//  name is empty.
//
//  Since we reuse the parser for pages and we recurse, we get out all of the topic
//  file info and store it in TPageInfo and TSubTopicInfo objects before we start
//  parsing page files. Then we can go back and work through those stored lists.
//
tCIDLib::TBoolean
TFacCQCDocComp::bIterateTopics( const   TString&    strSrcPath
                                , const TString&    strDirName
                                , const TString&    strHelpPath
                                ,       TTopic&     topicToFill)
{
    TDirIter    diterSrc;
    TFindBuf    fndbCur;
    TPathStr    pathTmp;

    // Output the category we are doing
    strmOut() << L"\n   Key: " << strHelpPath << kCIDLib::EndLn;

    // Look for the topic file at this level
    pathTmp = strSrcPath;
    pathTmp.AddLevel(L"CQCTopic");
    pathTmp.AppendExt(L"xml");
    if (!TFileSys::bExists(pathTmp))
    {
        facCQCDocComp.ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kDocCompErrs::errcSrc_NoTopicFile
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , pathTmp
        );
    }

    // Let's parse the topic file
    const tCIDLib::TBoolean bRes = m_xtprsToUse.bParseRootEntity
    (
        pathTmp
        , tCIDXML::EParseOpts::Validate
        , tCIDXML::EParseFlags::TagsNText
    );

    if (!bRes)
    {
        const TXMLTreeParser::TErrInfo& errCur = m_xtprsToUse.colErrors()[0];
        m_strmErr   << L"The CQCTopic parse failed\n"
                    << errCur.strText()
                    << kCIDLib::NewLn << L"(" << errCur.c4Line()
                    << kCIDLib::chPeriod << errCur.c4Column()
                    << L") " << pathTmp << kCIDLib::NewEndLn;
        return kCIDLib::False;
    }

    const TXMLTreeElement& xtnodeRoot = m_xtprsToUse.xtdocThis().xtnodeRoot();

    // Store our path info
    topicToFill.m_strTopicPath = strHelpPath;
    topicToFill.m_strSrcPath = strSrcPath;
    topicToFill.m_strDirName = strDirName;

    // Get the topic page attribute
    topicToFill.m_strTopicPage = xtnodeRoot.xtattrNamed(L"TopicPage").strValue();

    //
    //  We now go through and get out all of the page and sub-top info from the
    //  XML, then we can start reusing the XML parser.
    //
    tCIDLib::TCard4 c4Count;
    TTopicList colSubTopics;
    TPageInfo pinfoCur;
    {
        tCIDLib::TCard4 c4At = 0;
        const TXMLTreeElement& xtnodePageMap = xtnodeRoot.xtnodeFindElement
        (
            TFacCQCDocComp::s_strPageMap, 0, c4At
        );
        c4Count = xtnodePageMap.c4ChildCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const TXMLTreeNode& xtnodeCur = xtnodePageMap.xtnodeChildAt(c4Index);

            if (xtnodeCur.eType() != tCIDXML::ENodeTypes::Element)
                continue;

            // Reset our page info object and let's load up the stuff for this round
            pinfoCur.Reset();
            const TXMLTreeElement& xtnodeElem = xtnodePageMap.xtnodeChildAtAsElement(c4Index);
            if (!xtnodeElem.bAttrExists(s_strFileName, pinfoCur.m_strFile)
            ||  !xtnodeElem.bAttrExists(s_strFileExt, pinfoCur.m_strExt)
            ||  !xtnodeElem.bAttrExists(s_strTitle, pinfoCur.m_strTitle))
            {
                // We are missing required info
                facCQCDocComp.ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kDocCompErrs::errcSrc_PageMapInfo
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Format
                );
            }

            // See if it's a virtual one
            pinfoCur.m_bVirtual = xtnodeElem.xtattrNamed(TFacCQCDocComp::s_strVirtual).bValueAs();

            // Store the extra values if present
            xtnodeElem.bAttrExists(s_strExtra1, pinfoCur.m_strExtra1);
            xtnodeElem.bAttrExists(s_strExtra2, pinfoCur.m_strExtra2);

            //
            //  If the link text is empty, set it to the same as the file name. In a good
            //  number of cases it is, and that saves work in the XML.
            //
            if (pinfoCur.m_strTitle.bIsEmpty())
                pinfoCur.m_strTitle = pinfoCur.m_strFile;

            pinfoCur.Complete(strHelpPath);

            // And add this page info to the main page list and this topic's page map
            topicToFill.m_colPageMap.Add(&m_colPages.objAdd(pinfoCur));
        }

        const TXMLTreeElement& xtnodeSubTopicMap = xtnodeRoot.xtnodeFindElement
        (
            TFacCQCDocComp::s_strSubTopicMap, 0, c4At
        );
        c4Count = xtnodeSubTopicMap.c4ChildCount();
        TSubTopicInfo stinfoCur;
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const TXMLTreeNode& xtnodeCur = xtnodeSubTopicMap.xtnodeChildAt(c4Index);

            if (xtnodeCur.eType() != tCIDXML::ENodeTypes::Element)
                continue;

            const TXMLTreeElement& xtnodeElem = xtnodeSubTopicMap.xtnodeChildAtAsElement(c4Index);

            if (!xtnodeElem.bAttrExists(TFacCQCDocComp::s_strSubDir, stinfoCur.m_strSubDir)
            ||  !xtnodeElem.bAttrExists(TFacCQCDocComp::s_strTitle, stinfoCur.m_strTitle))
            {
                // We are missing required info
                facCQCDocComp.ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kDocCompErrs::errcSrc_TopicMapInfo
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Format
                );
            }

            //
            //  If the link text is empty, set it to the same as the dir name.
            //
            if (stinfoCur.m_strTitle.bIsEmpty())
                stinfoCur.m_strTitle = stinfoCur.m_strSubDir;

            stinfoCur.Complete(strHelpPath);

            // Add this subtopic info to our local list and the overall list
            colSubTopics.objAdd(stinfoCur);
            m_colTopics.objAdd(stinfoCur);
        }
    }


    // OK, now we can go back and start processing the page files
    c4Count = topicToFill.m_colPageMap.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TPageInfo& pinfoCur = *topicToFill.m_colPageMap[c4Index];

        // If this one is virtual, skip it. It will be generated later
        if (pinfoCur.m_bVirtual)
            continue;

        // See if this one has been compiled yet
        TBasePage* ppgNew = topicToFill.m_colParsedPages.pobjFindByKey
        (
            pinfoCur.m_strPath, kCIDLib::False
        );

        // If not, let's let it parse its content
        if (!ppgNew)
        {
            if (pinfoCur.m_strExt.bCompareI(s_strExt_HelpPage))
                ppgNew = new THelpPage();
            else if (pinfoCur.m_strExt.bCompareI(s_strExt_CMLClass))
                ppgNew = new TCMLClassPage();
            else if (pinfoCur.m_strExt.bCompareI(s_strExt_DevClass))
                ppgNew = new TDevClassPage();
            else if (pinfoCur.m_strExt.bCompareI(s_strExt_Driver))
                ppgNew = new TDevDriverPage();
            else if (pinfoCur.m_strExt.bCompareI(s_strExt_ActionTar))
                ppgNew = new TDocActionTar();
            else if (pinfoCur.m_strExt.bCompareI(s_strExt_IntfWidget))
                ppgNew = new TDocIntfWidget();
            else if (pinfoCur.m_strExt.bCompareI(s_strExt_EventFilter))
                ppgNew = new TDocEvFilterRef();

            CIDAssert(ppgNew != nullptr, L"Unknown page extension")

            pathTmp = strSrcPath;
            pathTmp.AddLevel(pinfoCur.m_strFile);
            pathTmp.AppendExt(pinfoCur.m_strExt);

            const tCIDLib::TBoolean bRes = bParsePageFile
            (
                pathTmp, pinfoCur, *ppgNew, topicToFill, kCIDLib::True
            );

            if (!bRes)
                return kCIDLib::False;

            // And add this one to the list of parsed pages for this topic
            topicToFill.m_colParsedPages.Add(ppgNew);
        }

        //
        //  We keep some lists of specific types of pages, so do those. We can't
        //  put this above where we check for specific types, because this could be
        //  an aliased file (multiple page links reference it) and we'd only go into
        //  the above block the first time that page was referenced.
        //
        if (pinfoCur.m_strExt.bCompareI(s_strExt_Driver))
            m_colDriverPages.Add(&pinfoCur);

        // Store the page in the page info object
        pinfoCur.m_ppgThis = ppgNew;
    }

    //
    //  And go through the child topics map and recurse on them.
    //
    c4Count = colSubTopics.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TSubTopicInfo& stinfoCur = colSubTopics[c4Index];

        // Add a new topic object for this topic and fill in it
        TTopic* ptopicSub = new TTopic();
        topicToFill.m_colChildTopicMap.Add(ptopicSub);

        pathTmp = strSrcPath;
        pathTmp.AddLevel(stinfoCur.m_strSubDir);

        // The title we set at this level since it is gotten from the map items
        ptopicSub->m_strTitle = stinfoCur.m_strTitle;
        if (!bIterateTopics(pathTmp, stinfoCur.m_strSubDir, stinfoCur.m_strPath, *ptopicSub))
            return kCIDLib::False;
    }
    return kCIDLib::True;
}


//
//  We have to parse a help page from more than one place, so we break it out here
//  separately. It returns true if successful. Sometimes we are parsing a file and
//  sometimes we are parsing actual XML content generated out into a string, so we
//  have a parameter to tell us. If it's XML content, then the strSysId is a system
//  id we can pass to the parse for error reporting. Else it's just the base name to
//  show as what we are parsing.
//
tCIDLib::TBoolean
TFacCQCDocComp::bParsePageFile( const   TString&            strSrc
                                , const TPageInfo&          pinfoSrc
                                ,       TBasePage&          pgTar
                                ,       TTopic&             topicPar
                                , const tCIDLib::TBoolean   bFileSrc)
{
    // Try to parse the passed file as XML
    tCIDLib::TBoolean bRes;

    if (bFileSrc)
    {
        // It's a file, so we have a method for that directly
        bRes =  m_xtprsToUse.bParseRootEntity
        (
            strSrc, tCIDXML::EParseOpts::Validate, tCIDXML::EParseFlags::TagsNText
        );
    }
     else
    {
        // We need to set up a memory buffer entity source for this one
        tCIDXML::TEntitySrcRef esrSrc(new TMemBufEntitySrc(pinfoSrc.m_strPath, strSrc));
        bRes = m_xtprsToUse.bParseRootEntity
        (
            esrSrc, tCIDXML::EParseOpts::Validate, tCIDXML::EParseFlags::TagsNText
        );
    }

    // If an XML level errors, display the first one and return
    if (!bRes)
    {
        const TXMLTreeParser::TErrInfo& errCur = m_xtprsToUse.colErrors()[0];
        m_strmErr   << L"The help page parse failed\n"
                    << errCur.strText()
                    << kCIDLib::NewLn << L"(" << errCur.c4Line()
                    << kCIDLib::chPeriod << errCur.c4Column()
                    << L") "
                    << pinfoSrc.m_strPath << kCIDLib::NewEndLn;
        return kCIDLib::False;
    }

    // Output the name of the content we are going to parse
    strmOut() << L"        -> " << pinfoSrc.m_strPath << kCIDLib::EndLn;

    //
    //  Get the root now out and kick off and let the target page object try to pull
    //  his data from it.
    //
    const TXMLTreeElement& xtnodeRoot = m_xtprsToUse.xtdocThis().xtnodeRoot();
    if (!pgTar.bParseXML(xtnodeRoot, pinfoSrc.m_strFile, topicPar))
    {
        m_strmErr << L"Page could not internalize parsed XML content\n"
                  << pinfoSrc.m_strPath << kCIDLib::NewEndLn;
        return kCIDLib::False;
    }
    return kCIDLib::True;
}



tCIDLib::EExitCodes
TFacCQCDocComp::eMainThread(TThread& thrThis, tCIDLib::TVoid* pData)
{
    // We have to let our calling thread go first
    thrThis.Sync();

    // Check for a /Verbose command line parameter
    tCIDLib::TCard4 c4Args = TSysInfo::c4CmdLineParmCount();
    if (c4Args > 0)
        m_bVerbose = TSysInfo::strCmdLineParmAt(0).bCompareI(L"/verbose");

    // Figure out which output stream to use
    if (m_bVerbose)
        m_pstrmOut = new TTextFileOutStream(tCIDLib::EStdFiles::StdOut);
    else
        m_pstrmOut = new TTextStringOutStream(32 * 1024UL);

    strmOut() << L"CQC Documentation Compiler" << kCIDLib::NewEndLn;


    // We output into the HTMLRoot directory so we can test with the web server
    TPathStr pathTmp;
    if (!TProcEnvironment::bFind(L"CQC_DATADIR", pathTmp))
    {
        m_strmErr << L"   Could not find CQC Data path" << kCIDLib::NewEndLn;
        return tCIDLib::EExitCodes::BadEnvironment;
    }

    // Create the target path if it doesn't exist
    pathTmp.AddLevel(L"HTMLRoot");
    pathTmp.AddLevel(L"Web2");
    pathTmp.AddLevel(L"CQCDocs");
    TFileSys::MakePath(pathTmp);

    // Store it away now that it's good
    m_strTarPath = pathTmp;


    // Create the top level source directory
    if (!TProcEnvironment::bFind(L"CQC_SRCTREE", pathTmp))
    {
        m_strmErr << L"   Could not find CQC Source path" << kCIDLib::NewEndLn;
        return tCIDLib::EExitCodes::BadEnvironment;
    }
    pathTmp.AddLevel(L"Source");
    pathTmp.AddLevel(L"Docs");
    pathTmp.AddLevel(L"CQCDocs");

    // Make sure it exists
    if (!TFileSys::bIsDirectory(pathTmp))
    {
        m_strmErr   << L"   Source path doesn't exists Path="
                    << pathTmp
                    << kCIDLib::NewEndLn;
        return tCIDLib::EExitCodes::BadParameters;
    }

    // Store it away now
    m_strSrcPath = pathTmp;

    // Keep a path to the Src directory under that for later use
    pathTmp.AddLevel(L"Src");
    m_strSrcSrcPath = pathTmp;


    // And now let's do the work
    try
    {
        //
        //  Create the starting directory for the tree of source XML files that
        //  we process.
        //
        TPathStr pathXMLSrc(m_strSrcPath);
        pathXMLSrc.AddLevel(L"Src");

        //
        //  We'll use a single parser. In order to avoid a lot of complexity with
        //  maintaining DTD paths in the XML files, just do a mapping of the DTD
        //  via the URN, so that the parser won't try to load the file. We have
        //  to load the DTD content to a string, which we pass in as the mapped
        //  entity content.
        //
        LoadDTD();

        //
        //  We start with a root topic, and a root path of /. We'll then start parsing
        //  at the XML source directory. So the index file there will fill in the root
        //  and we'll go from there.
        //
        m_topicRoot.m_strTitle = L"CQC Documentation";
        if (bIterateTopics(pathXMLSrc, TString::strEmpty(), s_strRoot, m_topicRoot))
        {
            if (!m_topicRoot.m_colChildTopicMap.bIsEmpty()
            &&  m_topicRoot.m_colPageMap.bIsEmpty())
            {
                m_strmErr   << L"  !There is no content to process"
                            << kCIDLib::NewEndLn;
                return tCIDLib::EExitCodes::Normal;
            }

            //
            //  Copy any files to the top level target directory that we just use
            //  as is. They will be in the top level source directory.
            //
            //  1.  The main HTML file which is hand done and includes the main
            //      header with menu and such. It includes a DIV into which all
            //      of the actual content is dynamically loaded.
            //  2.  The CSS file that contains the formatting info
            //  3.  Our own javascript file
            //  4.  The jquery javascript file
            //
            TPathStr pathCopySrc;
            TPathStr pathCopyTar;

            pathCopySrc = m_strSrcPath;
            pathCopySrc.AddLevel(L"CQCDocs.html");
            pathCopyTar = m_strTarPath;
            pathCopyTar.AddLevel(L"CQCDocs.html");
            TFileSys::CopyFile(pathCopySrc, pathCopyTar);

            pathCopySrc = m_strSrcPath;
            pathCopySrc.AddLevel(L"CQCDocs.css");
            pathCopyTar = m_strTarPath;
            pathCopyTar.AddLevel(L"CQCDocs.css");
            TFileSys::CopyFile(pathCopySrc, pathCopyTar);

            pathCopySrc = m_strSrcPath;
            pathCopySrc.AddLevel(L"CQCDocs.js");
            pathCopyTar = m_strTarPath;
            pathCopyTar.AddLevel(L"CQCDocs.js");
            TFileSys::CopyFile(pathCopySrc, pathCopyTar);

            pathCopySrc = m_strSrcPath;
            pathCopySrc.AddLevel(L"jquery.js");
            pathCopyTar = m_strTarPath;
            pathCopyTar.AddLevel(L"jquery.js");
            TFileSys::CopyFile(pathCopySrc, pathCopyTar);


            // Copy over all of the images
            pathCopySrc = m_strSrcPath;
            pathCopySrc.AddLevel(L"Images");
            pathCopyTar = m_strTarPath;
            pathCopyTar.AddLevel(L"Images");
            if (!TFileSys::bIsDirectory(pathCopyTar))
                TFileSys::MakeSubDirectory(L"Images", m_strTarPath);
            CopyDir(pathCopySrc, pathCopyTar);

            //
            //  Do any post-loading, pre-generation work...
            //
            //  First we will generate some alternative indices for device drivers. We spit
            //  these out into source files that will be treated like any other source file
            //  by the output generation below. The top level drivers scope topic list already
            //  references them, so we just have to spit out the content.
            //
            if (!bGenerateDriverIndices())
            {
                m_strmErr   << L"Could not generate driver indices" << kCIDLib::NewEndLn;
                return tCIDLib::EExitCodes::RuntimeError;
            }

            //
            //  Generate the IR model download files. These are generated from a special
            //  file (in the Special directory) that is not part of the source. So we just
            //  read it in and spit out the source files (one per blaster device supported)
            //  and then parse them into the IR models topic, for later output.
            //
            if (!bGenerateIRModelIndex())
            {
                m_strmErr   << L"Could not generate IR model index" << kCIDLib::NewEndLn;
                return tCIDLib::EExitCodes::RuntimeError;
            }

            //
            //  All the loading succeeded, so let's process the data and output the
            //  HTML. We start it with the root topic and the top level target path
            //  and it will recurse from there.
            //
            //  We have to pass a dummy parent topic because each one looks to
            //  create a back link. This guy will have no path and this method
            //  will check that and not do anything on this first round.
            //
            TTopic topicDummy;
            m_topicRoot.GenerateOutput(topicDummy, m_strTarPath);
        }
    }

    catch(const TError& errToCatch)
    {
        m_strmErr   << L"   An exception occurred. Error=\n"
                    << errToCatch
                    << kCIDLib::NewEndLn;
        m_strmErr.Flush();
        return tCIDLib::EExitCodes::RuntimeError;
    }

    strmOut() << kCIDLib::FlushIt;
    return tCIDLib::EExitCodes::Normal;
}


//
//  Copy the contents of a source directory to a target one, recursively.
//
tCIDLib::TVoid
TFacCQCDocComp::CopyDir(const TString& strSrc, const TString& strTar)
{
    TDirIter    diterSrc;
    TFindBuf    fndbCur;

    // Copy any files first
    TPathStr pathTar;
    TString  strNameExt;
    if (diterSrc.bFindFirst(strSrc, L"*.*", fndbCur, tCIDLib::EDirSearchFlags::NormalFiles))
    {
        do
        {
            // Build up the target path for this file
            fndbCur.pathFileName().bQueryNameExt(strNameExt);
            pathTar = strTar;
            pathTar.AddLevel(strNameExt);

            TFileSys::CopyFile(fndbCur.pathFileName(), pathTar);
        }   while (diterSrc.bFindNext(fndbCur));
    }

    // And now do any child directories
    if (diterSrc.bFindFirst(strSrc, L"*", fndbCur, tCIDLib::EDirSearchFlags::NormalDirs))
    {
        do
        {
            //
            //  Build up the target path for this file. And make sure it
            //  exists.
            //
            fndbCur.pathFileName().bQueryNameExt(strNameExt);
            pathTar = strTar;
            pathTar.AddLevel(strNameExt);

            if (!TFileSys::bIsDirectory(pathTar))
                TFileSys::MakeSubDirectory(strNameExt, strTar);

            CopyDir(fndbCur.pathFileName(), pathTar);

        }   while (diterSrc.bFindNext(fndbCur));
    }
}



//
//  Called to load our DTD text and add it to our parser as a preloaded entity,
//  so that the individual XML files don't have to deal with pointing themselves
//  at an actual DTD file (with all of the nesting going on it's a pain.) And it
//  will speed things up as well.
//
//  But it keeps the DTD external so we can quickly make changes and test them.
//
tCIDLib::TVoid TFacCQCDocComp::LoadDTD()
{
    // Build up the full path name
    TPathStr pathDTD(m_strSrcPath);
    pathDTD.AddLevel(L"CQCDocs.DTD");

    TTextFileInStream strmDTD
    (
        pathDTD
        , tCIDLib::ECreateActs::OpenIfExists
        , tCIDLib::EFilePerms::Default
        , tCIDLib::EFileFlags::SequentialScan
        , tCIDLib::EAccessModes::Excl_Read
        , facCIDEncode().ptcvtMake(L"Latin1")
    );

    //
    //  Read in the lines and add them to a string. The file doesn't have the
    //  entity header line, so we add that, indicating our internal string
    //  format as the encoding.
    //
    TString strDTDText(32 * 1024UL);
    strDTDText = L"<?xml encoding='$NativeWideChar$'?>\n";

    TString strLine(128UL);
    while (!strmDTD.bEndOfStream())
    {
        strmDTD >> strLine;
        strDTDText.Append(strLine);
        strDTDText.Append(L"\n");
    }

    m_xtprsToUse.AddMapping
    (
        new TMemBufEntitySrc
        (
            L"http://www.charmedquark.com/CQC/CQCHelpDocs.DTD"
            , L"urn:charmedquark.com:CQC-Documentation.DTD"
            , strDTDText
        )
    );
}


//
//  Spits out the standard header content for a regular help page, with title, to avoid
//  any duplication since we may do this in a number of places where we generate dynamic
//  content. We output whatever the encoding name is that the target stream says that he
//  is creating, so that this will work for files or string based streams or whatever.
//
tCIDLib::TVoid
TFacCQCDocComp::OutputStdHPHeader(TTextOutStream& strmTar, const TString& strTitle)
{
    strmTar << L"<?xml version=\"1.0\" encoding=\""
            << strmTar.strEncodingName()
            << L"\"?>\n"
            << L"<!DOCTYPE HelpPage PUBLIC "
            << L"\"urn:charmedquark.com:CQC-Documentation.DTD\" \"CQCDocs.DTD\">\n\n"
            << L"<HelpPage>\n\n"
            << L"<Title>" << strTitle << L"</Title>\n"
            << L"<HelpText>\n";
}


//
//  This will locate a topic by its path, e.g. /Reference/Drivers. We pull out each
//  component of the path at a time, and look for a sub-topic with that name, and so on.
//
TTopic& TFacCQCDocComp::topicFindByPath(const TString& strToFind)
{
    // Start at the top root topic
    TTopic* ptopicCur = &m_topicRoot;
    TStringTokenizer stokCur(&strToFind, L"/");

    TString strCurName;
    TString strCurPath(strToFind.c4Length());
    while (stokCur.bGetNextToken(strCurName))
    {
        strCurPath.Append(L"/");
        strCurPath.Append(strCurName);

        const tCIDLib::TCard4 c4Count = ptopicCur->m_colChildTopicMap.c4ElemCount();
        tCIDLib::TCard4 c4Index = 0;
        while (c4Index < c4Count)
        {
            // If we find this one, make it the new current topic and go again
            if (ptopicCur->m_colChildTopicMap[c4Index]->m_strTopicPath.bCompareI(strCurPath))
            {
                ptopicCur = ptopicCur->m_colChildTopicMap[c4Index];
                break;
            }
            c4Index++;
        }

        // Never found this one
        if (c4Index == c4Count)
        {
            ptopicCur = nullptr;
            break;
        }
    }

    // If not found, then throw
    if (!ptopicCur)
    {
        facCQCDocComp.ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kDocCompErrs::errcSrc_TopicNotFound
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , strToFind
        );
    }

    return *ptopicCur;
}
