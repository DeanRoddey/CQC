//
// FILE NAME: XMLGatewayTest.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 12/01/2002
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
//  This is the main implementation file of the gateway server test program.
//  This program simulates a client of the CQC XML Gateway server.
//
// CAVEATS/GOTCHAS:
//
//  1)  Note that, in the way of getting some regression testing slipped in
//      here, we use a number of variations on the XML encoding name. So it
//      is purposefully spelled differently in each case.
//
//  2)  We need to update this test to also support secure connections.
//
// LOG:
//


// ----------------------------------------------------------------------------
//  Includes
// ----------------------------------------------------------------------------
#include    "XMLGatewayTest.hpp"


// ----------------------------------------------------------------------------
//  Magic macros
// ----------------------------------------------------------------------------
RTTIDecls(TFacXMLGatewayTest,TFacility)


// ----------------------------------------------------------------------------
//  The facility object
// ----------------------------------------------------------------------------
TFacXMLGatewayTest facXMLGatewayTest;



// ----------------------------------------------------------------------------
//  Define the entry point to a method of the facility class
// ----------------------------------------------------------------------------
CIDLib_MainModule
(
    TThread
    (
        L"CQCXMLGatewayTestThread"
        , TMemberFunc<TFacXMLGatewayTest>
          (
            &facXMLGatewayTest
            , &TFacXMLGatewayTest::eMainThread
          )
    )
)


// ---------------------------------------------------------------------------
//   CLASS: TFacXMLGatewayTest
//  PREFIX: fac
// ---------------------------------------------------------------------------
TFacXMLGatewayTest::TFacXMLGatewayTest() :

    TFacility
    (
        L"XMLGatewayTest"
        , tCIDLib::EModTypes::Exe
        , kCQCKit::c4MajVersion
        , kCQCKit::c4MinVersion
        , kCQCKit::c4Revision
        , tCIDLib::EModFlags::HasMsgFile
    )
    , m_bSecure(kCIDLib::False)
    , m_bShutdown(kCIDLib::False)
    , m_c4CursorSize(0)
    , m_c4SchEvSerialNum(0)
    , m_conIn(kCIDLib::True, 50)
    , m_conOut()
    , m_gwscTest()
    , m_ippnServer(kCQCKit::ippnCQCGWSrvPort)
    , m_strServerAddr(L"localhost")
{
}

TFacXMLGatewayTest::~TFacXMLGatewayTest()
{
}


tCIDLib::TBoolean TFacXMLGatewayTest::bHandleSignal(const tCIDLib::ESignals eSig)
{
    //
    //  If it's a Ctrl-C or Ctrl-Break, just trigger the wait event. Ignore
    //  the others. This way, we can be stopped manually when running in
    //  debug mode, but don't get blown away by logoff signals when running
    //  under a service based app shell.
    //
    if ((eSig == tCIDLib::ESignals::CtrlC) || (eSig == tCIDLib::ESignals::Break))
        m_bShutdown = kCIDLib::True;

    // And return that we handled it
    return kCIDLib::True;
}


// ---------------------------------------------------------------------------
//  TFacXMLGatewayTest: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::EExitCodes
TFacXMLGatewayTest::eMainThread(TThread& thrThis, tCIDLib::TVoid*)
{
    // We have to let our calling thread go first
    thrThis.Sync();

    // Register ourself as a signal handler
    TSignals::c4AddHandler(this);

    // Get the parms and check that we got the ones we need
    if (!bParseParms())
    {
        ShowUsage();
        return tCIDLib::EExitCodes::BadParameters;
    }

    //
    //  Get a client stream socket for the indicated address and port and
    //  see if we can get connected to the gateway.
    //
    try
    {
        m_gwscTest.Connect
        (
            TIPEndPoint(m_strServerAddr, m_ippnServer, tCIDSock::EAddrTypes::Unspec)
            , m_strUserName
            , m_strPassword
            , m_bSecure
        );
    }

    catch(const TError& errToCatch)
    {
        m_conOut << L"Failed to connect/login to the server\n\n"
                 << errToCatch << kCIDLib::EndLn;
        return tCIDLib::EExitCodes::RuntimeError;
    }

    m_conOut << L"Login Succeeded. User Role=" << m_gwscTest.eUserRole()
             << kCIDLib::EndLn;

    //
    //  Loop, waiting for user input. We support a set of commands that
    //  allow the user to interact with the server.
    //
    TString strInput;
    while (kCIDLib::True)
    {
        m_conOut << L"\n> " << kCIDLib::FlushIt;
        if (!m_conIn.c4GetLine(strInput, 0, kCIDLib::True))
            continue;

        // Just break out on an exit command
        if (strInput.bCompareI(L"exit"))
            break;

        // Else, it's got to be a legal command
        try
        {
            if (strInput.bCompareI(L"help") || strInput.bCompareI(L"?"))
                ShowCommands();
            else if (strInput.bCompareI(L"doglobalact"))
                DoGlobalAct();
            else if (strInput.bCompareI(L"mwritefield"))
                MWriteField();
            else if (strInput.bCompareI(L"pauseschev"))
                PauseEvent(kCIDLib::True);
            else if (strInput.bCompareI(L"poll"))
                PollFields();
            else if (strInput.bCompareI(L"querydrvs"))
                QueryDrivers();
            else if (strInput.bCompareI(L"querydrvtext"))
                QueryDriverText();
            else if (strInput.bCompareI(L"queryflds"))
                QueryFlds();
            else if (strInput.bCompareI(L"queryfldInfo"))
                QueryFldInfo();
            else if (strInput.bCompareI(L"queryglobacts"))
                QueryGlobActs();
            else if (strInput.bCompareI(L"querymacros"))
                QueryMacros();
            else if (strInput.bCompareI(L"querymediaart"))
                QueryMediaArt();
            else if (strInput.bCompareI(L"querymediadb"))
                QueryMediaDB();
            else if (strInput.bCompareI(L"querymediarendart"))
                QueryMediaRendArt();
            else if (strInput.bCompareI(L"queryperev"))
                QueryPerEv();
            else if (strInput.bCompareI(L"queryrendpl"))
                QueryRendPL();
            else if (strInput.bCompareI(L"queryrepodrvs"))
                QueryRepoDrvs();
            else if (strInput.bCompareI(L"queryrmcfg"))
                QueryRmCfg();
            else if (strInput.bCompareI(L"queryrmcfglist"))
                QueryRmCfgList();
            else if (strInput.bCompareI(L"queryschev"))
                QuerySchEv();
            else if (strInput.bCompareI(L"queryschevinfo"))
                QuerySchEvInfo();
            else if (strInput.bCompareI(L"queryschevs"))
                QuerySchEvs();
            else if (strInput.bCompareI(L"querysunev"))
                QuerySunEv();
            else if (strInput.bCompareI(L"readfield"))
                ReadField();
            else if (strInput.bCompareI(L"runmacro"))
                RunMacro();
            else if (strInput.bCompareI(L"setpolllist"))
                SetPollList();
            else if (strInput.bCompareI(L"setperevtime"))
                SetPerEvTime();
            else if (strInput.bCompareI(L"setschevtime"))
                SetSchEvTime();
            else if (strInput.bCompareI(L"setsunevtime"))
                SetSunEvTime();
            else if (strInput.bCompareI(L"showpolllist"))
                ShowPollList();
            else if (strInput.bCompareI(L"writefield"))
                WriteField();
            else
                m_conOut << strInput << L" is not a known command" << kCIDLib::EndLn;
        }

        catch(const TError& errToCatch)
        {
            m_conOut << errToCatch << kCIDLib::EndLn;
        }

        catch(...)
        {
            m_conOut << L"Unknown exception occured" << kCIDLib::EndLn;
        }
    }

    try
    {
        m_gwscTest.Disconnect();
    }

    catch(const TError& errToCatch)
    {
        m_conOut << L"Failed to shutdown client\n\n"
                 << errToCatch << kCIDLib::EndLn;
        return tCIDLib::EExitCodes::RuntimeError;
    }
    return tCIDLib::EExitCodes::Normal;
}


// ----------------------------------------------------------------------------
//  TFacXMLGatewayTest: Private, non-virtual methods
// ----------------------------------------------------------------------------

//
//  We get a global actiion path and any parameters from the user, and then
//  invoke the action on the server.
//
tCIDLib::TVoid TFacXMLGatewayTest::DoGlobalAct()
{
    TString strPath;
    m_conOut << L"Enter an action path to run> " << kCIDLib::FlushIt;
    if (!m_conIn.c4GetLine(strPath, 0, kCIDLib::True) || strPath.bIsEmpty())
        return;

    m_conOut << L"Enter a list of parameters. Use an empty line to end the\n"
                L"list. To abort, enter an 'x' by itself. \n\nParam> "
             << kCIDLib::FlushIt;

    tCIDLib::TStrList    colParms;
    TString             strInput;
    while (kCIDLib::True)
    {
        if (!m_conIn.c4GetLine(strInput, 0, kCIDLib::True))
            break;

        if (strInput == L"x")
            break;

        colParms.objAdd(strInput);
        m_conOut << L"Param> " << kCIDLib::FlushIt;
    }

    try
    {
        TString strFailReason;

        if (m_gwscTest.bDoGlobalAct(strPath, colParms, strFailReason))
        {
            m_conOut << L"  Successfully ran the action\n";
        }
         else
        {
            m_conOut << L"  Failed the action invocation. Reason="
                     << strFailReason << kCIDLib::NewLn;
        }
        m_conOut.Flush();
    }

    catch(const TError& errToCatch)
    {
        m_conOut << L"Failed to run action. \n\n"
                 << errToCatch << kCIDLib::EndLn;
    }
}


// Pause/resume an event
tCIDLib::TVoid TFacXMLGatewayTest::PauseEvent(const tCIDLib::TBoolean bScheduled)
{
    // Get an id
    TString strId;
    m_conOut << L"Enter an even id> " << kCIDLib::FlushIt;
    if (!m_conIn.c4GetLine(strId, 0, kCIDLib::True) || strId.bIsEmpty())
        return;

    TString strState;
    m_conOut << L"Enter Pause or Resume> " << kCIDLib::FlushIt;
    if (!m_conIn.c4GetLine(strState, 0, kCIDLib::True) || strState.bIsEmpty())
        return;

    tCIDLib::TBoolean bState;
    if (strState == L"Pause")
        bState = kCIDLib::True;
    else if (strState == L"Resume")
        bState = kCIDLib::False;
    else
    {
        m_conOut << L"The new state value is not valid" << kCIDLib::NewLn;
        return;
    }

    if (bScheduled)
        m_gwscTest.PauseSchEv(strId, bState);
}


// Write multiple fields at one
tCIDLib::TVoid TFacXMLGatewayTest::MWriteField()
{
    // Just some quicky test stuff until we can implement this correctly
    tCIDLib::TStrList colMons(3);
    tCIDLib::TStrList colFlds(3);
    tCIDLib::TStrList colVals(3);

    colMons.objAdd(L"Vars");
    colMons.objAdd(L"DigIO");
    colMons.objAdd(L"Vars");

    colFlds.objAdd(L"BoolTest");
    colFlds.objAdd(L"Contact_03");
    colFlds.objAdd(L"CardTest");

    colVals.objAdd(L"True");
    colVals.objAdd(L"True");
    colVals.objAdd(L"15");


    // OK, we have the values so send them on
    m_gwscTest.MWriteField(colMons, colFlds, colVals);
}


//
//  Once the poll list is set, we can call this to poll the fields.
//
tCIDLib::TVoid TFacXMLGatewayTest::PollFields()
{
    if (m_gwscTest.bPollFields())
    {
        tCIDLib::TBoolean   bState;
        TString             strValue;

        // Some have changed, so let's iterate them
        const tCIDLib::TCard4 c4Count = m_gwscTest.c4PollListCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            if (m_gwscTest.bCheckField(c4Index, bState, strValue))
            {
                const TCQCGWSrvClient::TFieldInfo& fldiCur = m_gwscTest.fldiAt(c4Index);
                m_conOut << L"   " << fldiCur.m_strName;

                if (bState)
                    m_conOut << L"[Online]  Value=" << strValue;
                else
                    m_conOut << L"[InError]";

                m_conOut << kCIDLib::NewLn;
            }
        }
        m_conOut << kCIDLib::EndLn;
    }
     else
    {
        m_conOut << L"  No fields have changed\n" << kCIDLib::EndLn;
    }
}


//
//  Get information about all of the available drivers.
//
tCIDLib::TVoid TFacXMLGatewayTest::QueryDrivers()
{
    tCIDLib::TStrList       colMonikers;
    tCQCKit::TDevCatList    fcolCats;

    m_gwscTest.QueryDrivers(colMonikers, fcolCats);

    m_conOut << L"Available Devices\n------------------------------------\n";

    TStreamJanitor janStrmFmt(&m_conOut);
    TStreamFmt strmfMon(24, 0, tCIDLib::EHJustify::Left);
    TStreamFmt strmfCat(0, 0, tCIDLib::EHJustify::Left);

    const tCIDLib::TCard4 c4Count = colMonikers.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        m_conOut << L"  Device: "
                 << strmfMon << colMonikers[c4Index]
                 << janStrmFmt.strmfSaved() << L"    (Cat="
                 << strmfCat << fcolCats[c4Index]
                 << janStrmFmt.strmfSaved() << L")\n";
    }
}


//
//  Do a backdoor driver text query and display the text.
//
tCIDLib::TVoid TFacXMLGatewayTest::QueryDriverText()
{
    TString strMoniker;
    m_conOut << L"Enter an repo moniker> " << kCIDLib::FlushIt;
    if (!m_conIn.c4GetLine(strMoniker, 0, kCIDLib::True) || strMoniker.bIsEmpty())
        return;

    TString strQType;
    m_conOut << L"Enter the query type> " << kCIDLib::FlushIt;
    if (!m_conIn.c4GetLine(strQType, 0, kCIDLib::True) || strQType.bIsEmpty())
        return;

    TString strDType;
    m_conOut << L"Enter the data name> " << kCIDLib::FlushIt;
    if (!m_conIn.c4GetLine(strDType, 0, kCIDLib::True))
        return;

    TString strOutput;
    const tCIDLib::TBoolean bRes = m_gwscTest.bQueryDrvText
    (
        strMoniker, strQType, strDType, strOutput
    );

    m_conOut << L"Status: " << bRes
             << L"\nText:\n" << strOutput << kCIDLib::NewEndLn;
}


//
//  Get's us a named value map of all the drivers and all of their fields
//
tCIDLib::TVoid TFacXMLGatewayTest::QueryFlds()
{
    typedef TNamedValMap<TCQCFldDef> TList;

    TList colFlds;
    m_gwscTest.QueryFields(colFlds);

    m_conOut << L"Available Devices/Fields\n------------------------------------\n";

    // Get a cursor for the top level items
    TList::TItemCursor cursItems = colFlds.cursItems();
    if (cursItems.bReset())
    {
        do
        {
            m_conOut << L"  Device: " << cursItems.objRCur().strKey() << kCIDLib::NewLn;

            // Get a cursor for this guy's fields and put them out
            TList::TValCursor cursFlds(&cursItems.objRCur().colPairs());
            if (cursFlds.bReset())
            {
                do
                {
                    m_conOut << L"      Field: " << cursFlds.objRCur().objKey()
                             << L"  (" << cursFlds.objRCur().objValue().eType()
                             << L", " << cursFlds.objRCur().objValue().eSemType()
                             << L", " << cursFlds.objRCur().objValue().eAccess()
                             << L")"
                             << kCIDLib::NewLn;
                }   while(cursFlds.bNext());
            }

            m_conOut << kCIDLib::NewLn;
        }   while(cursItems.bNext());
    }
}


// Get's us info on a specific field
tCIDLib::TVoid TFacXMLGatewayTest::QueryFldInfo()
{
    TString strFld;
    m_conOut << L"Enter an mon.field> " << kCIDLib::FlushIt;
    if (!m_conIn.c4GetLine(strFld, 0, kCIDLib::True) || strFld.bIsEmpty())
        return;

    TCQCFldDef flddInfo;
    m_gwscTest.QueryFieldInfo(strFld, flddInfo);
    m_conOut << L"Field Info\n" << L"--------------------\n"
             << L"  Access: " << flddInfo.eAccess() << kCIDLib::NewLn
             << L"    Type: " << flddInfo.eType() << kCIDLib::NewLn
             << L" SemType: " << flddInfo.eSemType() << kCIDLib::NewLn
             << L"  Limits: " << flddInfo.strLimits() << kCIDLib::NewLn;
}


//
//  Query and display the available global actions
//
tCIDLib::TVoid TFacXMLGatewayTest::QueryGlobActs()
{
    // Ask for the heirarchy
    TCQCGWSrvClient::TNameTreeCol colTree;
    m_gwscTest.QueryGlobActs(colTree);

    // We have a helper to show these types of hierarchies
    ShowTree(colTree);
}


//
//  Query and display the available macros
//
tCIDLib::TVoid TFacXMLGatewayTest::QueryMacros()
{
    // Ask for the heirarchy
    TCQCGWSrvClient::TNameTreeCol colTree;
    m_gwscTest.QueryMacros(colTree);

    // We have a helper to show these types of hierarchies
    ShowTree(colTree);
}


//
//  This one goes to the client service cached metadata on the machine where the
//  GW server is running. This one is done by ids.
//
tCIDLib::TVoid TFacXMLGatewayTest::QueryMediaArt()
{
    // Get the repo driver moniker
    TString strMoniker;
    m_conOut << L"Enter an repo moniker> " << kCIDLib::FlushIt;
    if (!m_conIn.c4GetLine(strMoniker, 0, kCIDLib::True) || strMoniker.bIsEmpty())
        return;

    // Get large or small
    TString strImgSz;
    m_conOut << L"Image type (L/S)> " << kCIDLib::FlushIt;
    if (!m_conIn.c4GetLine(strImgSz, 0, kCIDLib::True) || strImgSz.bIsEmpty())
        return;
    const tCIDLib::TBoolean bLarge = strImgSz.bCompareI(L"L");

    // Get the UID
    TString strUID;
    m_conOut << L"Enter UID> " << kCIDLib::FlushIt;
    if (!m_conIn.c4GetLine(strUID, 0, kCIDLib::True) || strUID.bIsEmpty())
        return;

    // Get the SID, can be empty
    TString strSID;
    m_conOut << L"Enter SID, Empty if new> " << kCIDLib::FlushIt;
    m_conIn.c4GetLine(strSID, 0, kCIDLib::True);

    // Get media type
    TString strMType;
    m_conOut << L"Media Type (Movie/Music) " << kCIDLib::FlushIt;
    if (!m_conIn.c4GetLine(strMType, 0, kCIDLib::True) || strMType.bIsEmpty())
        return;
    const tCQCMedia::EMediaTypes eMType = tCQCMedia::eXlatEMediaTypes(strMType);


    tCIDLib::TCard4 c4ImgSz;
    THeapBuf        mbufImg(64);
    TString         strNewPerId;
    const tCIDLib::ELoadRes eRes = m_gwscTest.eQueryMediaArt
    (
        strMoniker
        , strUID
        , strSID
        , eMType
        , bLarge
        , c4ImgSz
        , mbufImg
        , strNewPerId
    );

    if (eRes == tCIDLib::ELoadRes::NewData)
    {
        //
        //  Probe the image to see what type it is. If not something we
        //  understand, give up.
        //
        const tCIDImage::EImgTypes eType = facCIDImgFact().eProbeImg(mbufImg, c4ImgSz);
        if (eType == tCIDImage::EImgTypes::Unknown)
        {
            m_conOut << L"Unknown image type was returned" << kCIDLib::EndLn;
            return;
        }

        // Set up a local directory name to store it to
        TString strFlName(L"CoverArt.");
        strFlName.Append(facCIDImgFact().strImgTypeExt(eType));

        // Write it out to the name we set up
        {
            TBinaryFile flImg(strFlName);
            flImg.Open
            (
                tCIDLib::EAccessModes::Write
                , tCIDLib::ECreateActs::CreateAlways
                , tCIDLib::EFilePerms::Default
                , tCIDLib::EFileFlags::SequentialScan
            );
            flImg.c4WriteBuffer(mbufImg, c4ImgSz);
        }

        // Invoke the local system viewer for this type of file
        TExternalProcess extpView;
        extpView.SystemEscape(strFlName, tCIDLib::ESysEscFlags::None);

        m_conOut << L"   New serial number: " << strNewPerId;
    }
     else if (eRes == tCIDLib::ELoadRes::NoNewData)
    {
        m_conOut << L"The image has not changed since the last query";
    }
     else if (eRes == tCIDLib::ELoadRes::NotFound)
    {
        m_conOut << L"The query failed";
    }
    m_conOut << kCIDLib::EndLn;
}


//
//  Query the media database from a repo.
//
tCIDLib::TVoid TFacXMLGatewayTest::QueryMediaDB()
{
    // Get the repo driver moniker
    TString strMoniker;
    m_conOut << L"Enter an repo/rend moniker> " << kCIDLib::FlushIt;
    if (!m_conIn.c4GetLine(strMoniker, 0, kCIDLib::True) || strMoniker.bIsEmpty())
        return;

    // Get the serial number
    TString strSerNum;
    m_conOut << L"Enter a serial number> " << kCIDLib::FlushIt;
    m_conIn.c4GetLine(strSerNum, 0, kCIDLib::True);


    tCIDLib::TCard4 c4DataSz;
    THeapBuf        mbufData(64);
    TString         strErrMsg;
    TString         strNewSerialNum;
    const tCIDLib::ELoadRes eRes = m_gwscTest.eQueryMediaDB
    (
        strMoniker
        , strSerNum
        , strNewSerialNum
        , mbufData
        , c4DataSz
        , strErrMsg
    );

    // If we got new data, parse it and dump it to a file
    if (eRes == tCIDLib::ELoadRes::NewData)
    {
        m_conOut    << L"  New Data. SerialNum=" << strNewSerialNum
                    << kCIDLib::NewEndLn;

        {
            const tCIDXML::EParseFlags eFlags = tCIDXML::EParseFlags::TagsNText;
            const tCIDXML::EParseOpts eOpts = tCIDXML::EParseOpts::IgnoreBadChars;

            // Givea away the buffer contents since we don't need it anymore
            tCIDXML::TEntitySrcRef esrMsg
            (
                new TMemBufEntitySrc(L"XMLGWOp", tCIDLib::ForceMove(mbufData), c4DataSz)
            );

            TXMLTreeParser xtprsMsgs;
            if (xtprsMsgs.bParseRootEntity(esrMsg, tCIDXML::EParseOpts::None, eFlags))
            {
                // Create an output stream and let it format to the file
                TTextFileOutStream strmOut
                (
                    L"MediaDB.xml"
                    , tCIDLib::ECreateActs::CreateAlways
                    , tCIDLib::EFilePerms::Default
                    , tCIDLib::EFileFlags::SequentialScan
                    , tCIDLib::EAccessModes::Write
                );
                xtprsMsgs.xtdocThis().PrintTo(strmOut, kCIDLib::True);
                strmOut.Flush();
            }
             else
            {
                // Display the first error
                const TXMLTreeParser::TErrInfo& erriToShow = xtprsMsgs.erriFirst();

                TString strErr(erriToShow.strText().c4Length() + 32UL);
                strErr.Append(L'[');
                strErr.AppendFormatted(erriToShow.c4Line());
                strErr.Append(L',');
                strErr.AppendFormatted(erriToShow.c4Column());
                strErr.Append(L"] - ");
                strErr.Append(erriToShow.strText());

                facCQCKit().ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kKitErrs::errcXGWC_ParseFailed
                    , strErr
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Protocol
                );
            }
        }
    }
     else if (eRes == tCIDLib::ELoadRes::NoNewData)
    {
        m_conOut    << L"  No changes since last query" << kCIDLib::NewEndLn;
    }
     else
    {
        m_conOut    << L"  The operation failed. Error=" << strErrMsg
                    << kCIDLib::NewEndLn;
    }
}



//
//  Get the current artwork for a renderer driver.
//
tCIDLib::TVoid TFacXMLGatewayTest::QueryMediaRendArt()
{
    // Get the repo driver moniker
    TString strMoniker;
    m_conOut << L"Enter an renderer moniker> " << kCIDLib::FlushIt;
    if (!m_conIn.c4GetLine(strMoniker, 0, kCIDLib::True) || strMoniker.bIsEmpty())
        return;

    tCIDLib::TCard4 c4ImgSz;
    THeapBuf        mbufImg(64);
    const tCIDLib::TBoolean bRes = m_gwscTest.bQueryMediaRendArt
    (
        strMoniker, mbufImg, c4ImgSz
    );

    if (bRes && c4ImgSz)
    {
        //
        //  Probe the image to see what type it is. If not something we
        //  understand, give up.
        //
        const tCIDImage::EImgTypes eType = facCIDImgFact().eProbeImg(mbufImg, c4ImgSz);
        if (eType == tCIDImage::EImgTypes::Unknown)
        {
            m_conOut << L"Unknown image type was returned" << kCIDLib::EndLn;
            return;
        }

        // Set up a local directory name to store it to
        TString strFlName(L"CoverArt.");
        strFlName.Append(facCIDImgFact().strImgTypeExt(eType));

        // Write it out to the name we set up
        {
            TBinaryFile flImg(strFlName);
            flImg.Open
            (
                tCIDLib::EAccessModes::Write
                , tCIDLib::ECreateActs::CreateAlways
                , tCIDLib::EFilePerms::Default
                , tCIDLib::EFileFlags::SequentialScan
            );
            flImg.c4WriteBuffer(mbufImg, c4ImgSz);
        }

        // Invoke the local system viewer for this type of file
        TExternalProcess extpView;
        extpView.SystemEscape(strFlName, tCIDLib::ESysEscFlags::None);
    }
     else
    {
        m_conOut << L"No image data was returned";
    }
    m_conOut << kCIDLib::EndLn;
}


// Query a periodic scheduled event by id
tCIDLib::TVoid TFacXMLGatewayTest::QueryPerEv()
{
    TString strId;
    m_conOut << L"Enter an event id> " << kCIDLib::FlushIt;
    if (!m_conIn.c4GetLine(strId, 0, kCIDLib::True) || strId.bIsEmpty())
        return;

    tCIDLib::TBoolean   bPaused;
    tCIDLib::TCard4     c4Period;
    TString             strName;
    TString             strType;
    m_gwscTest.QueryPerEv(strId, strName, bPaused, strType, c4Period);

    m_conOut << L"Name=" << strName << kCIDLib::NewLn
             << L"  Type=" << strType
             << L"  Paused=" << (bPaused ? L"Yes" : L"No")
             << L"  Period=" << c4Period
             << kCIDLib::EndLn;
}


// Query the playlist items from a media renderer driver
tCIDLib::TVoid TFacXMLGatewayTest::QueryRendPL()
{
    TString strMon;
    m_conOut << L"Enter a rnderer moniker> " << kCIDLib::FlushIt;
    if (!m_conIn.c4GetLine(strMon, 0, kCIDLib::True) || strMon.bIsEmpty())
        return;

    tCIDLib::TStrList   colArtists;
    tCIDLib::TStrList   colCookies;
    tCIDLib::TStrList   colLocs;
    tCIDLib::TStrList   colNames;
    tCIDLib::TCardList  fcolIds;
    m_gwscTest.QueryRendPL
    (
        strMon, colArtists, colNames, colCookies, colLocs, fcolIds
    );

    const tCIDLib::TCard4 c4Count = colArtists.c4ElemCount();
    if (c4Count)
    {
        m_conOut    << L"\nCount=" << colArtists.c4ElemCount() << kCIDLib::NewLn
                    << L"--------------------------------------------\n";

        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            m_conOut    << colNames[c4Index]
                        << L"\n    Cookie: " << colCookies[c4Index]
                        << L"\n    Artist: " << colArtists[c4Index]
                        << L"\n        Id: " << fcolIds[c4Index]
                        << L"\n       Loc: " << colLocs[c4Index]
                        << kCIDLib::DNewLn;
        }

        m_conOut << kCIDLib::NewEndLn;
    }
     else
    {
        m_conOut << L"\nThe playist was empty" << kCIDLib::NewEndLn;
    }
}


// Query a list of media repository drivers currently loaded
tCIDLib::TVoid TFacXMLGatewayTest::QueryRepoDrvs()
{
    tCIDLib::TStrList colDrvs;
    m_gwscTest.QueryMediaRepoDrvs(colDrvs);

    m_conOut << L"Available Repo Drivers\n------------------------\n";

    const tCIDLib::TCard4 c4Count = colDrvs.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        m_conOut << L"  " << colDrvs[c4Index] << kCIDLib::NewLn;
    m_conOut << kCIDLib::NewLn;
}


// Query the config for a specific room
tCIDLib::TVoid TFacXMLGatewayTest::QueryRmCfg()
{
    TString strName;
    m_conOut << L"Enter a room name> " << kCIDLib::FlushIt;
    if (!m_conIn.c4GetLine(strName, 0, kCIDLib::True) || strName.bIsEmpty())
        return;

    m_gwscTest.QueryRmCfg(strName);
}


// Query a list of configured rooms
tCIDLib::TVoid TFacXMLGatewayTest::QueryRmCfgList()
{
    tCIDLib::TStrList colRooms;
    m_gwscTest.QueryRmCfgList(colRooms);

    m_conOut << L"Available Configured Rooms\n------------------------\n";

    const tCIDLib::TCard4 c4Count = colRooms.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        m_conOut << L"  " << colRooms[c4Index] << kCIDLib::NewLn;
    m_conOut << kCIDLib::NewLn;
}



//
//  We query a specific scheduled event by id and dump out the info. This is
//  a generic level query, so it doesn't show the type specific info.
//
tCIDLib::TVoid TFacXMLGatewayTest::QuerySchEvInfo()
{
    TString strId;
    m_conOut << L"Enter an event id> " << kCIDLib::FlushIt;
    if (!m_conIn.c4GetLine(strId, 0, kCIDLib::True) || strId.bIsEmpty())
        return;

    tCIDLib::TBoolean   bPaused;
    TString             strName;
    TString             strType;
    m_gwscTest.QuerySchEvInfo(strId, strName, bPaused, strType);

    m_conOut << L"Name=" << strName << kCIDLib::NewLn
             << L"  Type=" << strType
             << L"  Paused=" << (bPaused ? L"Yes" : L"No")
             << kCIDLib::EndLn;
}


// Query a scheduled type scheduled event by id
tCIDLib::TVoid TFacXMLGatewayTest::QuerySchEv()
{
    TString strId;
    m_conOut << L"Enter an event id> " << kCIDLib::FlushIt;
    if (!m_conIn.c4GetLine(strId, 0, kCIDLib::True) || strId.bIsEmpty())
        return;

    tCIDLib::TBoolean   bPaused;
    tCIDLib::TCard4     c4Day;
    tCIDLib::TCard4     c4Hour;
    tCIDLib::TCard4     c4Min;
    TString             strMask;
    TString             strName;
    TString             strType;
    m_gwscTest.QuerySchEv(strId, strName, bPaused, strType, strMask, c4Day, c4Hour, c4Min);

    m_conOut << L"Name=" << strName << kCIDLib::NewLn
             << L"  Type=" << strType
             << L"  Paused=" << (bPaused ? L"Yes" : L"No")
             << L"  Mask=" << strMask
             << L"  DHM=" << c4Day << L'/' << c4Hour << L'/' << c4Min
             << kCIDLib::EndLn;
}


//
//  We get some lists of values that represent the available scheduled events,
//  and just dump them out.
//
tCIDLib::TVoid TFacXMLGatewayTest::QuerySchEvs()
{
    tCIDLib::TStrList               colNames;
    tCIDLib::TStrList               colIds;
    tCIDLib::TStrList               colTypes;
    TFundVector<tCIDLib::TBoolean>  fcolStates;

    // Do a query wtih the serial number we got last (or zero the first time.)
    tCIDLib::TBoolean bRes = m_gwscTest.bQuerySchEvs
    (
        m_c4SchEvSerialNum, colNames, colIds, colTypes, fcolStates
    );

    if (!bRes)
    {
        m_conOut << L"There have been no changes\n" << kCIDLib::FlushIt;
        return;
    }

    // We got changes, so display them
    m_conOut << L"Available Scheduled Events\n-------------------------------------\n";
    const tCIDLib::TCard4 c4Count = colNames.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        m_conOut << L"Name=" << colNames[c4Index] << kCIDLib::NewLn
                 << L"   Id=" << colIds[c4Index]
                 << L"  Type=" << colTypes[c4Index]
                 << L"  Paused=" << (fcolStates[c4Index] ? L"Yes" : L"No")
                 << kCIDLib::NewLn;
    }
    m_conOut << kCIDLib::EndLn;
}


// Query a sun based scheduled event by id
tCIDLib::TVoid TFacXMLGatewayTest::QuerySunEv()
{
    TString strId;
    m_conOut << L"Enter an event id> " << kCIDLib::FlushIt;
    if (!m_conIn.c4GetLine(strId, 0, kCIDLib::True) || strId.bIsEmpty())
        return;

    tCIDLib::TBoolean   bPaused;
    tCIDLib::TInt4      i4Offset;
    TString             strMask;
    TString             strName;
    TString             strType;
    m_gwscTest.QuerySunEv(strId, strName, bPaused, strType, i4Offset);

    m_conOut << L"Name=" << strName << kCIDLib::NewLn
             << L"  Type=" << strType
             << L"  Paused=" << (bPaused ? L"Yes" : L"No")
             << L"  Offset=" << i4Offset
             << kCIDLib::EndLn;
}


// Read a single field's value. Doesn't have to be in the poll list
tCIDLib::TVoid TFacXMLGatewayTest::ReadField()
{
    m_conOut << L"\n\nEnter a Moniker> "
             << kCIDLib::FlushIt;
    TString strMoniker;
    if (!m_conIn.c4GetLine(strMoniker, 0, kCIDLib::True))
        return;

    m_conOut << L"\n\nEnter a Field> "
             << kCIDLib::FlushIt;
    TString strField;
    if (!m_conIn.c4GetLine(strField, 0, kCIDLib::True))
        return;

    try
    {
        TString strValue;
        if (m_gwscTest.bReadField(strMoniker, strField, strValue))
            m_conOut << L"  The value is: " << strValue << kCIDLib::EndLn;
        else
            m_conOut << L"  <Error Status>" << kCIDLib::EndLn;
    }

    catch(const TError& errToCatch)
    {
        m_conOut << L"Failed to write value. \n\n"
                 << errToCatch << kCIDLib::EndLn;
    }
}


//
//  We get a macro name and any parameters from the user, and then invoke the
//  macro on the server.
//
tCIDLib::TVoid TFacXMLGatewayTest::RunMacro()
{
    TString strMacro;
    m_conOut << L"Enter a macro to run> " << kCIDLib::FlushIt;
    if (!m_conIn.c4GetLine(strMacro, 0, kCIDLib::True) || strMacro.bIsEmpty())
        return;

    m_conOut << L"Enter a list of parameters. Enter || \n"
                L"to end the list. To abort, enter ^^. \n\nParam> "
             << kCIDLib::FlushIt;

    tCIDLib::TStrList    colParms;
    TString             strInput;
    while (kCIDLib::True)
    {
        m_conIn.c4GetLine(strInput, 0, kCIDLib::True);
        if (strInput == L"^^")
            return;
        if (strInput == L"||")
            break;

        colParms.objAdd(strInput);
        m_conOut << L"Param> " << kCIDLib::FlushIt;
    }

    // OK we have the parms, so invoke the macro
    try
    {
        tCIDLib::TInt4  i4Res;
        TString         strOutput;
        if (m_gwscTest.bRunMacro(strMacro, colParms, strOutput, i4Res))
            m_conOut << L"  Successfully ran the macro. Result Code=";
        else
            m_conOut << L"  Failed the macro invocation. Result Code=";
        m_conOut << i4Res << kCIDLib::NewLn;

        if (!strOutput.bIsEmpty())
            m_conOut << L"   Output=" << strOutput;
        m_conOut << kCIDLib::DNewLn << kCIDLib::FlushIt;
    }

    catch(const TError& errToCatch)
    {
        m_conOut << L"Failed to invoke macro. \n\n"
                 << errToCatch << kCIDLib::EndLn;
    }
}


// Set up a period scheduled event
tCIDLib::TVoid TFacXMLGatewayTest::SetPerEvTime()
{
    TString strId;
    m_conOut << L"Enter an event id> " << kCIDLib::FlushIt;
    if (!m_conIn.c4GetLine(strId, 0, kCIDLib::True) || strId.bIsEmpty())
        return;

    // Get the period value
    tCIDLib::TCard4 c4Period;
    if (!bGetCardVal(L"Enter Period", c4Period, 0, 60))
        return;

    try
    {
        m_gwscTest.SetPerEv(strId, c4Period, TTime::enctNow());
    }

    catch(const TError& errToCatch)
    {
        m_conOut << L"Failed to set event time. \n\n"
                 << errToCatch << kCIDLib::EndLn;
    }
}

// Set up scheduled scheduled event
tCIDLib::TVoid TFacXMLGatewayTest::SetSchEvTime()
{
    TString strId;
    m_conOut << L"Enter an event id> " << kCIDLib::FlushIt;
    if (!m_conIn.c4GetLine(strId, 0, kCIDLib::True) || strId.bIsEmpty())
        return;

    // Get the day value
    tCIDLib::TCard4 c4Day;
    if (!bGetCardVal(L"Enter the Day", c4Day, 0, 31))
        return;

    // Get the hour value
    tCIDLib::TCard4 c4Hour;
    if (!bGetCardVal(L"Enter the Hour", c4Hour, 0, 24))
        return;

    // Get the minute value
    tCIDLib::TCard4 c4Minute;
    if (!bGetCardVal(L"Enter the Minute", c4Minute, 0, 60))
        return;

    TString strMask;
    m_conOut << L"Enter the Mask> " << kCIDLib::FlushIt;
    if (!m_conIn.c4GetLine(strMask, 0, kCIDLib::True))
        return;

    try
    {
        m_gwscTest.SetSchEv(strId, strMask, c4Day, c4Hour, c4Minute);
    }

    catch(const TError& errToCatch)
    {
        m_conOut << L"Failed to set event time. \n\n"
                 << errToCatch << kCIDLib::EndLn;
    }
}


// Set up a sun-based scheduled event
tCIDLib::TVoid TFacXMLGatewayTest::SetSunEvTime()
{
    TString strId;
    m_conOut << L"Enter an event id> " << kCIDLib::FlushIt;
    if (!m_conIn.c4GetLine(strId, 0, kCIDLib::True) || strId.bIsEmpty())
        return;

    // Get the offset
    tCIDLib::TInt4 i4Ofs;
    if (!bGetIntVal(L"Enter the hour offset", i4Ofs, -60, 60))
        return;

    try
    {
        m_gwscTest.SetSunEv(strId, i4Ofs);
    }

    catch(const TError& errToCatch)
    {
        m_conOut << L"Failed to set event time. \n\n"
                 << errToCatch << kCIDLib::EndLn;
    }
}



//
//  We get a list of fields from the user and set those on the client as the
//  new poll list. We let them enter lines until they enter an empty list as
//  the end of list. Of they can enter 'x' to abandon.
//
tCIDLib::TVoid TFacXMLGatewayTest::SetPollList()
{
    m_conOut << L"Enter a list of field names. Use an empty line to end the\n"
                L"list. To abort, enter an 'x' by itself. \n\nField> "
             << kCIDLib::FlushIt;

    tCIDLib::TStrList    colFlds;
    TString             strFld;
    TString             strMon;
    TString             strInput;
    while (kCIDLib::True)
    {
        if (!m_conIn.c4GetLine(strInput, 0, kCIDLib::True))
            break;

        if (strInput == L"x")
            return;

        // It's got to be a field, so check it
        if (facCQCKit().bParseFldName(strInput, strMon, strFld))
        {
            // Add it to the list
            colFlds.objAdd(strInput);
        }
         else
        {
            // Can't be valid, so reject it
            m_conOut << L"  The field name was not correctly formed"
                     << kCIDLib::EndLn;
        }

        m_conOut << L"Field> " << kCIDLib::FlushIt;
    }

    // OK we have a list of fields, so let's set them
    try
    {
        m_gwscTest.SetPollList(colFlds);
    }

    catch(const TError& errToCatch)
    {
        m_conOut << L"Failed to set poll list. \n\n"
                 << errToCatch << kCIDLib::EndLn;
    }
}



//
//  Just dumps out the current poll list info
//
tCIDLib::TVoid TFacXMLGatewayTest::ShowPollList()
{
    const tCIDLib::TCard4 c4Count = m_gwscTest.c4PollListCount();

    if (c4Count)
    {
        m_conOut << L"\nCurrent Poll List\n-------------------------\n";

        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const TCQCGWSrvClient::TFieldInfo& fldiCur = m_gwscTest.fldiAt(c4Index);
            m_conOut << L"  " << fldiCur.m_strName << L"[";

            if (fldiCur.m_bState)
            {
                m_conOut << L"Value=" << fldiCur.m_strValue;
            }
             else
            {
                m_conOut << L"InError";
            }

            m_conOut << L"]\n";
        }
    }
     else
    {
        m_conOut << L"\nThe poll list is empty\n";
    }

    m_conOut << kCIDLib::EndLn;
}


tCIDLib::TVoid TFacXMLGatewayTest::WriteField()
{
    m_conOut << L"\n\nEnter a Moniker> "
             << kCIDLib::FlushIt;
    TString strMoniker;
    if (!m_conIn.c4GetLine(strMoniker, 0, kCIDLib::True))
        return;

    m_conOut << L"\n\nEnter a Field> "
             << kCIDLib::FlushIt;
    TString strField;
    if (!m_conIn.c4GetLine(strField, 0, kCIDLib::True))
        return;

    m_conOut << L"\n\nEnter a Value> "
             << kCIDLib::FlushIt;
    TString strValue;
    if (!m_conIn.c4GetLine(strValue, 0, kCIDLib::True))
        return;

    try
    {
        m_gwscTest.WriteField(strMoniker, strField, strValue);
    }

    catch(const TError& errToCatch)
    {
        m_conOut << L"Failed to write value. \n\n"
                 << errToCatch << kCIDLib::EndLn;
    }
}

