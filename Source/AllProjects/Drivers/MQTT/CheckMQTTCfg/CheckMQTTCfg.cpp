//
// FILE NAME: CheckMQTTCfg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/10/2019
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
//  We need a little test program that will parse the MQTT configuration file.
//  It would be brutal for customers to have to get the file right by loading the
//  service and looking at the logs, around and around many times. This will let
//  them quick make sure it's basically correct.
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
#include    "CIDSChan.hpp"
#include    "CQCMQTT.hpp"
#include    "MQTTSh.hpp"


// ---------------------------------------------------------------------------
//  Do the magic main module code
// ---------------------------------------------------------------------------
tCIDLib::EExitCodes eMainThreadFunc(TThread&, tCIDLib::TVoid*);
CIDLib_MainModule(TThread(L"CheckMQTTCfgMainThread", eMainThreadFunc))


// -----------------------------------------------------------------------------
//  Local helpers
// -----------------------------------------------------------------------------
static tCIDLib::TVoid ShowUsage(TTextOutStream& strmTar)
{
    strmTar << L"Usage:\n"
            << L"   CheckMQTTCfg /Moniker=xxx [/TestConn]\n\n"
            << L"   Where Moniker is the moniker of the MQTT driver to check\n"
            << L"   And /TestConn is an option to do a basic connection test\n"
            << kCIDLib::EndLn;
}


// -----------------------------------------------------------------------------
//  Program entry point
// -----------------------------------------------------------------------------
tCIDLib::EExitCodes eMainThreadFunc(TThread& thrThis, tCIDLib::TVoid*)
{
    // We have to let our calling thread go first
    thrThis.Sync();

    // Get a convenience ref to the standard output stream
    TTextOutStream& strmOut = TSysInfo::strmOut();

    // A little identification line
    strmOut << L"\nCQC MQTT Config Checker" << kCIDLib::NewEndLn;

    // Get the standard CQC environmental info
    TString strFailReason;
    if (!facCQCKit().bLoadEnvInfo(strFailReason, kCIDLib::False))
    {
        strmOut << L"The CQC environment was not found"
                            << kCIDLib::NewEndLn;
        return tCIDLib::EExitCodes::BadEnvironment;
    }

    // We have to at least get a moniker for the driver's config we are to check
    tCIDLib::TBoolean bTestConn = kCIDLib::False;
    TString strMoniker;
    TSysInfo::TCmdLineCursor cursParms = TSysInfo::cursCmdLineParms();
    if (!cursParms.bIsValid())
    {
        ShowUsage(strmOut);
        return tCIDLib::EExitCodes::BadParameters;
    }

    // Run through the parameters and check them
    for (; cursParms; ++cursParms)
    {
        if (cursParms->bStartsWithI(L"/Moniker="))
        {
            strMoniker = *cursParms;
            strMoniker.Cut(0, 9);
        }
         else if (cursParms->bCompareI(L"/TestConn"))
        {
            bTestConn = kCIDLib::True;
        }
         else
        {
            ShowUsage(strmOut);
            return tCIDLib::EExitCodes::BadParameters;
        }
    }

    if (strMoniker.bIsEmpty())
    {
        ShowUsage(strmOut);
        return tCIDLib::EExitCodes::BadParameters;
    }

    // Build up the path to the config file on this machine
    TPathStr pathCfg = facCQCKit().strServerDataDir();
    pathCfg.AddLevel(L"MQTT");
    pathCfg.AddLevel(strMoniker);
    pathCfg.AppendExt(L".cfg");

    // First just see if it's there
    if (!TFileSys::bExists(pathCfg, tCIDLib::EDirSearchFlags::NormalFiles))
    {
        strmOut << L"The configuration file was not found. It should be at:\n"
                            << L"    " << pathCfg
                            << kCIDLib::NewEndLn;
        return tCIDLib::EExitCodes::Normal;
    }

    // It's there so try to parse it
    tCIDLib::TStrList colErrs;
    TMQTTCfg mqcfgTest;
    try
    {
        mqcfgTest.bParseFrom(pathCfg, strMoniker, colErrs);

        // Check a few global config things
        if (!mqcfgTest.strCertInfo().bIsEmpty() && !mqcfgTest.bSecureConn())
            colErrs.objAdd(L"If certificate information is set, you should set Secure=\"Yes\"");

        // Do some checks on the fields
        tCIDLib::TStrHashSet colDups(29, TStringKeyOps(kCIDLib::False));
        TMQTTCfg::TFldCursor cursFlds = mqcfgTest.cursFldList();
        TString strErr;
        for (; cursFlds; ++cursFlds)
        {
            const TMQTTFldCfg& mqfldcCur = *cursFlds;

            // Make sure the base name meets basic field name requirements
            if (!facCQCKit().bIsValidBaseFldName(mqfldcCur.strBaseName()))
            {
                colErrs.objAdd
                (
                    TString(L"'%(1)' is not a valid base field name", mqfldcCur.strBaseName())
                );
            }

            if (colDups.bHasElement(mqfldcCur.strFldName()))
            {
                colErrs.objAdd
                (
                    TString
                    (
                        L"Field name %(1) is a duplicate", mqfldcCur.strFldName()
                    )
                );
            }
             else
            {
                colDups.objAdd(mqfldcCur.strFldName());
            }
        }

        if (colErrs.bIsEmpty())
        {
           strmOut << L" - The MQTT GW Server configuration parsed successfully."
                                << kCIDLib::NewLn;
        }
         else
        {
            strmOut << L"\n    The following errors occurred:\n"
                                << L"    --------------------------------------------\n";

            TStreamIndentJan janIndent(&strmOut, 8);
            colErrs.bForEach
            (
                [&strmOut](const TString& strCur) -> tCIDLib::TBoolean
                {
                    strmOut << strCur << kCIDLib::NewLn;
                    return kCIDLib::True;
                }
            );
        }
        strmOut << kCIDLib::EndLn;


        // If there were no errors, and we were asked, try a basic connection to the server
        if (colErrs.bIsEmpty() && bTestConn)
        {
            strmOut << L"Attempting server connection..." << kCIDLib::EndLn;
            TIPEndPoint ipepTar
            (
                mqcfgTest.strMQTTAddr(), mqcfgTest.ippnMQTT(), tCIDSock::EAddrTypes::Unspec
            );

            // Set up the ALPN list and add their preferred protocol if set
            tCIDLib::TStrList colALPN;
            if (!mqcfgTest.strPrefProtocol().bIsEmpty())
                colALPN.objAdd(mqcfgTest.strPrefProtocol());

            TCIDSockStreamBasedDataSrc* pcdsMQTT= nullptr;
            if (mqcfgTest.bSecureConn())
            {
                pcdsMQTT = new TCIDSChanClDataSrc
                (
                    L"MQTT Driver"
                    , ipepTar
                    , tCIDSock::ESockProtos::TCP
                    , mqcfgTest.strCertInfo()
                    , colALPN
                    , tCIDSChan::EConnOpts::None
                    , mqcfgTest.strSecPrincipal()
                );
            }
             else
            {
                pcdsMQTT = new TCIDSockStreamDataSrc(ipepTar);
            }

            TCIDDataSrcJan janSrc
            (
                pcdsMQTT
                , tCIDLib::EAdoptOpts::Adopt
                , kCIDLib::True
                , TTime::enctNowPlusSecs(4)
            );

            // Create a connect msg and send it over our data source
            strmOut << L"Contact established, sending Connect message" << kCIDLib::EndLn;
            TMQTTOutMsg msgOut(256);
            msgOut.BuildConnectMsg
            (
                L"CQCMQTTCfgCheck"
                , kCIDLib::True
                , 15
                , mqcfgTest.strUserName()
                , mqcfgTest.strPassword()
            );
            msgOut.SendOn(*pcdsMQTT);

            // And we should get a connection ack
            tCIDLib::TBoolean bGotAck = kCIDLib::False;
            strmOut << L"Waiting for response..." << kCIDLib::EndLn;
            while (pcdsMQTT->bDataAvail(TTime::enctNowPlusSecs(8)))
            {
                // There is data, so read it in
                TMQTTInMsg msgRep(2048);
                msgRep.ParseFrom(*pcdsMQTT);
                if (msgRep.ePacketType() == tCQCMQTT::EPacketTypes::ConnAck)
                {
                    strmOut << L"Got Connect Ack, connection info should be OK"
                            << kCIDLib::EndLn;

                    // Send a disconnect to be polite
                    msgOut.BuildDisconnectMsg();
                    msgOut.SendOn(*pcdsMQTT);
                    bGotAck = kCIDLib::True;
                }
                else
                {
                    strmOut << L"Expected Connect Ack reply but got: "
                            << tCQCMQTT::strXlatEPacketTypes(msgRep.ePacketType())
                            << kCIDLib::EndLn;
                }
            }

            if (!bGotAck)
                strmOut << L"No reply to MQTT connect message" << kCIDLib::EndLn;
        }
    }

    catch(const TError& errToCatch)
    {
        strmOut << L"An error occurred: ";
        TStreamIndentJan janIndent(&strmOut, 8);
        strmOut << errToCatch << kCIDLib::NewEndLn;
    }
    return tCIDLib::EExitCodes::Normal;
}


