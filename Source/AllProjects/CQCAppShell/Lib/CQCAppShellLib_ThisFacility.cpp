//
// FILE NAME: CQCAppShellLib_ThisFacility.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/30/2001
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
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCAppShellLib_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TFacCQCAppShellLib,TFacility)


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace
{
    namespace CQCAppShellLib_ThisFacility
    {
        // -----------------------------------------------------------------------
        //  The DTD text for the config file
        // -----------------------------------------------------------------------
        constexpr const tCIDLib::TCh s_pszEmbeddedDTDText[] =
        (
            L"<?xml encoding='$NativeWideChar$'?>\n"
            L"<!ENTITY % Levels '0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9'>\n"
            L"<!ELEMENT CQCSh:ProcRec EMPTY>\n"
            L"<!ATTLIST CQCSh:ProcRec\n"
            L"          CQCSh:Title CDATA #REQUIRED\n"
            L"          CQCSh:Name CDATA #REQUIRED\n"
            L"          CQCSh:Path CDATA #REQUIRED\n"
            L"          CQCSh:Parms CDATA #REQUIRED\n"
            L"          CQCSh:AdminPath CDATA #REQUIRED\n"
            L"          CQCSh:Level (%Levels;) #REQUIRED>\n\n"
            L"<!ELEMENT CQCSh:ProcRecs (CQCSh:ProcRec+)>\n"
        );
    }
}


// ---------------------------------------------------------------------------
//   CLASS: TFacCQCAppShellLib
//  PREFIX: fac
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TFacCQCAppShellLib: Constructors and Destructor
// ---------------------------------------------------------------------------

//
//  Note that we use the same version constants as the toolkit facility. Since
//  they must be used together, this makes sense.
//
TFacCQCAppShellLib::TFacCQCAppShellLib() :

    TFacility
    (
        L"CQCAppShellLib"
        , tCIDLib::EModTypes::SharedLib
        , kCQCKit::c4MajVersion
        , kCQCKit::c4MinVersion
        , kCQCKit::c4Revision
        , tCIDLib::EModFlags::HasMsgFile
    )
    , m_strmOut(kCQCAppShellLib_::c4MaxQElems, tCIDLib::EReadModes::NonDestructive)
{
}

TFacCQCAppShellLib::~TFacCQCAppShellLib()
{
}


// ---------------------------------------------------------------------------
//  TFacCQCAppShellLib: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TFacCQCAppShellLib::bQueryNewMsgs(          tCIDLib::TCard4&    c4MsgIndex
                                    ,       TVector<TString>&   colMsgs
                                    , const tCIDLib::TBoolean   bAddNewLine)
{
    // Just delegate to the stream
    return m_strmOut.bQueryNewMsgs(c4MsgIndex, colMsgs, bAddNewLine);
}


tCIDLib::TCard4 TFacCQCAppShellLib::c4QueryApps(tCIDLib::TKVPList& colApps)
{
    // Just delegate to the app engine
    return m_asengCtrl.c4QueryAppsInfo(colApps);
}


tCIDLib::TVoid TFacCQCAppShellLib::CycleApps()
{
    // Just delegate to the app engine
    m_asengCtrl.Cycle(15000);
}


tCIDLib::TVoid TFacCQCAppShellLib::Initialize(const TString& strPath)
{
    try
    {
        // Find our input file
        TPathStr pathSrc(strPath);
        pathSrc.AddLevel(L"CQCAppShell.List");

        if (TFileSys::bExists(pathSrc, tCIDLib::EDirSearchFlags::AllFiles))
        {
            //
            //  Parse the file. If its bad, then we won't be able to start
            //  any processes, but errors will have been written so the user
            //  can see the issues.
            //
            ParseListFile(pathSrc);
        }
         else
        {
            // Just put out a message that we couldn't find the file
            m_strmOut   << strMsg(kCQCShErrs::errcFl_NotFound, pathSrc)
                        << kCIDLib::FlushIt;

            // Log it also
            if (bLogFailures())
            {
                LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kCQCShErrs::errcFl_NotFound
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::AppStatus
                    , pathSrc
                );
            }
        }
    }

    catch(const TError& errToCatch)
    {
        if (bShouldLog(errToCatch))
            LogEventObj(errToCatch);

        // <TBD> log something
    }

    catch(...)
    {
        // <TBD> log something
    }
}


TTextOutStream& TFacCQCAppShellLib::strmOut()
{
    return m_strmOut;
}


tCIDLib::TVoid TFacCQCAppShellLib::StartAllApps()
{
    // Tell the app shell engine to activate all the apps
    m_asengCtrl.ActivateAll();
}


tCIDLib::TVoid TFacCQCAppShellLib::StopAllApps()
{
    // Tell the app shell engine to stop all the apps
    m_asengCtrl.StopAll(30000);
}


tCIDLib::TVoid TFacCQCAppShellLib::StopEngine()
{
    StopAllApps();
}



// ---------------------------------------------------------------------------
//  TFacCQCAppShellLib: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TFacCQCAppShellLib::ParseListFile(const TString& strFile)
{
    TXMLTreeParser xtprsConfig;

    //
    //  Add an entity mapping so that the public URN that is used in the
    //  manifest files get mapped to our hard coded internal DTD text.
    //
    xtprsConfig.AddMapping
    (
        new TMemBufEntitySrc
        (
            L"http://www.charmedquark.com/CQCShell/CQCProcList.DTD"
            , L"urn:charmedquark.com:CQC-ShellProcList.DTD"
            , TString(CQCAppShellLib_ThisFacility::s_pszEmbeddedDTDText)
        )
    );

    //
    //  Ok, we have a file, so lets try to use the XML tree parser to get
    //  a representation of the file into memory. Tell it that we only
    //  want to see tag information. That will save us a lot of filtering
    //  out comments and whitspace and such.
    //
    const tCIDLib::TBoolean bOk = xtprsConfig.bParseRootEntity
    (
        strFile, tCIDXML::EParseOpts::Validate, tCIDXML::EParseFlags::Tags
    );

    if (!bOk)
    {
        //
        //  Format the first error in the error list, into a string that
        //  we can pass as a replacement parameter to the real error we
        //  log.
        //
        const TXMLTreeParser::TErrInfo& erriFirst
                                        = xtprsConfig.colErrors()[0];

        TTextStringOutStream strmOut(512);
        strmOut << L"[" << erriFirst.strSystemId() << L"/"
                << erriFirst.c4Line() << L"." << erriFirst.c4Column()
                << L"] " << erriFirst.strText() << kCIDLib::EndLn;

        //
        //  Throw an exception that indicates that the file was not valid
        //  XML. Pass along the first error that occured.
        //
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCQCShErrs::errcFl_BadFormat
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , strmOut.strData()
        );
    }

    //
    //  Ok, we now have the document, so get the root out. All of our
    //  config records are children of the root.
    //
    const TXMLTreeElement& xtnodeRoot = xtprsConfig.xtdocThis().xtnodeRoot();

    //
    //  We will print out the configuration as we read them in, so we
    //  put out a header for that section of the output.
    //
    m_strmOut   << kCIDLib::NewLn << strMsg(kCQCShMsgs::midGen_ConfiguredApps)
                << L"\n------------------------" << kCIDLib::NewLn;

    //
    //  Loop through the children of the root element, each of which is a
    //  CQCSh:ProcRec element, which contains the info for one element.
    //
    const tCIDLib::TCard4 c4ChildCount = xtnodeRoot.c4ChildCount();
    for (tCIDLib::TCard4 c4ChildInd = 0; c4ChildInd < c4ChildCount; c4ChildInd++)
    {
        // Get the next available element
        const TXMLTreeElement& xtnodeCur
                            = xtnodeRoot.xtnodeChildAtAsElement(c4ChildInd);

        // Pull out the other string attributes that hold our info
        const TString& strTitle = xtnodeCur.xtattrNamed(L"CQCSh:Title").strValue();
        const TString& strName = xtnodeCur.xtattrNamed(L"CQCSh:Name").strValue();
        const TString& strParms = xtnodeCur.xtattrNamed(L"CQCSh:Parms").strValue();
        const TString& strAdminPath = xtnodeCur.xtattrNamed(L"CQCSh:AdminPath").strValue();
        const TString& strLevel = xtnodeCur.xtattrNamed(L"CQCSh:Level").strValue();

        TPathStr pathFull(xtnodeCur.xtattrNamed(L"CQCSh:Path").strValue());
        pathFull.AddLevel(strName);
        pathFull.AppendExt(L"exe");

        // Convert the level to the enum value
        tCIDAppSh::EAppLevels eLevel = tCIDAppSh::EAppLevels(strLevel.c4Val());

        // Looks ok, so store in the collection
        TCQCAppShRecord* pasrNew = new TCQCAppShRecord
        (
            strTitle
            , strName
            , pathFull
            , strParms
            , strAdminPath
            , eLevel
        );
//        pasrNew->eFlags(tCIDLib::EExtProcFlags::NewProcGroup);
        pasrNew->eFlags(tCIDLib::EExtProcFlags::None);

        pasrNew->eStatus(tCIDAppSh::EStatus::Inactive);

        m_asengCtrl.AddApp(pasrNew);

        m_strmOut   << strMsg(kCQCShMsgs::midGen_Title)
                    << strTitle << kCIDLib::NewLn
                    << strMsg(kCQCShMsgs::midGen_Process) << pathFull << kCIDLib::NewLn
                    << strMsg(kCQCShMsgs::midGen_Parms) << strParms << kCIDLib::NewLn
                    << strMsg(kCQCShMsgs::midGen_Level) << strLevel << kCIDLib::NewLn
                    << kCIDLib::FlushIt;
    }
    m_strmOut << kCIDLib::NewLn << kCIDLib::FlushIt;

    // They are all in and sorted, so activate them all now
    m_asengCtrl.ActivateAll();
}


