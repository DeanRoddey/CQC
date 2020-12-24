//
// FILE NAME: TestCQCKit_ConnCfg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/11/2008
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
//  This file implements the tests for the connection configuration classes,
//  which are used to pass in driver connection configuration to drivers.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


// ---------------------------------------------------------------------------
//  Include underlying headers
// ---------------------------------------------------------------------------
#include    "TestCQCKit.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TTest_IPConnCfg,TTestFWTest)
RTTIDecls(TTest_SerialConnCfg,TTestFWTest)
RTTIDecls(TTest_UPnPConnCfg,TTestFWTest)



// ---------------------------------------------------------------------------
//  CLASS: TTest_IPConnCfg
// PREFIX: tfwt
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TTest_IPConnCfg: Constructor and Destructor
// ---------------------------------------------------------------------------
TTest_IPConnCfg::TTest_IPConnCfg() :

    TTestFWTest
    (
        L"IP Conn Cfg", L"Tests the IP connection configuration class", 2
    )
{
}

TTest_IPConnCfg::~TTest_IPConnCfg()
{
}


// ---------------------------------------------------------------------------
//  TTest_IPConnCfg: Public, inherited methods
// ---------------------------------------------------------------------------
tTestFWLib::ETestRes
TTest_IPConnCfg::eRunTest(   TTextStringOutStream&   strmOut
                            , tCIDLib::TBoolean&    bWarning)
{
    tTestFWLib::ETestRes eRes = tTestFWLib::ETestRes::Success;

    //
    //  It should default to the IP protocol on port zero. Set it up for
    //  IPV4 addresses in this case.
    //
    TCQCIPConnCfg conncfgTest;
    if (conncfgTest.eProto() != tCIDSock::ESockProtos::IP)
    {
        strmOut << TFWCurLn << L"Default socket protocol is wrong\n\n";
        return tTestFWLib::ETestRes::Failed;
    }

    if (conncfgTest.ippnTarget() != 0)
    {
        strmOut << TFWCurLn << L"Default port number is wrong\n\n";
        return tTestFWLib::ETestRes::Failed;
    }


    // Now set it and make sure the values were correctly set
    conncfgTest.Set(L"www.charmedquark.com", 80);
    conncfgTest.eProto(tCIDSock::ESockProtos::UDP);

    if (conncfgTest.eProto() != tCIDSock::ESockProtos::UDP)
    {
        strmOut << TFWCurLn << L"Setting protocol failed\n\n";
        return tTestFWLib::ETestRes::Failed;
    }

    if (conncfgTest.ippnTarget() != 80)
    {
        strmOut << TFWCurLn << L"Setting port number failed\n\n";
        return tTestFWLib::ETestRes::Failed;
    }

    if (conncfgTest.strAddr() != L"www.charmedquark.com")
    {
        strmOut << TFWCurLn << L"Setting address failed\n\n";
        return tTestFWLib::ETestRes::Failed;
    }

    // Copy it and test for equality
    TCQCIPConnCfg conncfgCopy(conncfgTest);

    if (conncfgCopy != conncfgTest)
    {
        strmOut << TFWCurLn << L"Copy did not create identical objects\n\n";
        return tTestFWLib::ETestRes::Failed;
    }

    // And the same check as well (this is used for polymorphic comparisons)
    if (!conncfgCopy.bIsSame(conncfgTest))
    {
        strmOut << TFWCurLn << L"Polymorphic equivalence failed\n\n";
        return tTestFWLib::ETestRes::Failed;
    }

    // Let it parse a few different forms of the text encoded connection
    struct TEncTestInfo
    {
        const   tCIDLib::TCh*   pszText;
        const   tCIDLib::TCh*   pszAddr;
        tCIDLib::TIPPortNum     ippnTarget;
        tCIDSock::EAddrTypes    eAddrType;
    };
    static const TEncTestInfo aEncodedTests[] =
    {
        { L"localhost:9999", L"127.0.0.1", 9999, tCIDSock::EAddrTypes::IPV4 }
        , { L"192.168.100.50:9441", L"192.168.100.50", 9441, tCIDSock::EAddrTypes::IPV4 }
        , { L"localhost:9999", L"::1", 9999, tCIDSock::EAddrTypes::IPV6 }
        , {
            L"2001:0:4137:9e76:2413:199:bc4b:a874:15300"
            , L"2001:0:4137:9e76:2413:199:bc4b:a874"
            , 15300
            , tCIDSock::EAddrTypes::IPV6
          }
    };
    const tCIDLib::TCard4 c4EncTestCnt = tCIDLib::c4ArrayElems(aEncodedTests);

    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4EncTestCnt; c4Index++)
    {
        // Set up the address type for this round
        conncfgTest.eAddrType(aEncodedTests[c4Index].eAddrType);

        if (!conncfgTest.bParseConnInfo(aEncodedTests[c4Index].pszText))
        {
            strmOut << TFWCurLn << L"Parse of encoded conn info failed. URL=\""
                    << aEncodedTests[c4Index].pszText << L"\"\n\n";
            return tTestFWLib::ETestRes::Failed;
        }

        if (conncfgTest.strAddr() != aEncodedTests[c4Index].pszAddr)
        {
            strmOut << TFWCurLn << L"Parsed cfg's addr was wrong. Round="
                    << (c4Index + 1) << L". Expected '" << aEncodedTests[c4Index].pszAddr
                    << L"' but got '" << conncfgTest.strAddr()
                    << L"'\n\n";
            return tTestFWLib::ETestRes::Failed;
        }

        if (conncfgTest.ippnTarget() != aEncodedTests[c4Index].ippnTarget)
        {
            strmOut << TFWCurLn << L"Parsed cfg's port was wrong. Round="
                    << (c4Index + 1) << L". Expected " << aEncodedTests[c4Index].ippnTarget
                    << L" but got " << conncfgTest.ippnTarget()
                    << L"\n\n";
            return tTestFWLib::ETestRes::Failed;
        }
    }

    // Do a round trip stream and make sure it comes back the same
    {
        TBinMBufOutStream strmWrite(8192UL);
        TBinMBufInStream strmRead(strmWrite);
        strmWrite << conncfgTest << kCIDLib::FlushIt;
        strmRead.Reset();
        strmRead >> conncfgCopy;
    }

    if (conncfgTest != conncfgCopy)
    {
        strmOut << TFWCurLn << L"Round trip streaming failed\n\n";
        return tTestFWLib::ETestRes::Failed;
    }

    return eRes;
}




// ---------------------------------------------------------------------------
//  CLASS: TTest_SerialConnCfg
// PREFIX: tfwt
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TTest_SerialConnCfg: Constructor and Destructor
// ---------------------------------------------------------------------------
TTest_SerialConnCfg::TTest_SerialConnCfg() :

    TTestFWTest
    (
        L"Serial Conn Cfg", L"Tests the serial connection configuration class", 2
    )
{
}

TTest_SerialConnCfg::~TTest_SerialConnCfg()
{
}


// ---------------------------------------------------------------------------
//  TTest_SerialConnCfg: Public, inherited methods
// ---------------------------------------------------------------------------
tTestFWLib::ETestRes
TTest_SerialConnCfg::eRunTest(   TTextStringOutStream&   strmOut
                            , tCIDLib::TBoolean&    bWarning)
{
    tTestFWLib::ETestRes eRes = tTestFWLib::ETestRes::Success;

    // It should default to no port and RS232 port type
    TCQCSerialConnCfg conncfgTest;
    if (conncfgTest.strPortPath() != TString::strEmpty())
    {
        strmOut << TFWCurLn << L"Default port path is wrong\n\n";
        return tTestFWLib::ETestRes::Failed;
    }

    if (conncfgTest.ePortType() != tCIDComm::EPortTypes::RS232)
    {
        strmOut << TFWCurLn << L"Default port type is wrong\n\n";
        return tTestFWLib::ETestRes::Failed;
    }

    //
    //  Check the default port parameters. We can't just default construct
    //  a serial port config object, since we want to catch any changes, so
    //  we constuct one with the settings we know that currently they default
    //  to.
    //
    TCommPortCfg cpcfgDefault
    (
        9600
        , tCIDComm::EFlags::None
        , tCIDComm::EParities::None
        , tCIDComm::EDataBits::Eight
        , tCIDComm::EStopBits::One
        , tCIDComm::EPortDTR::Disable
        , tCIDComm::EPortRTS::Disable
    );

    if (conncfgTest.cpcfgSerial() != cpcfgDefault)
    {
        strmOut << TFWCurLn << L"Default comm config is wrong\n\n";
        return tTestFWLib::ETestRes::Failed;
    }


    //
    //  Now set it and make sure the values were correctly set. We cahnge
    //   all of the port config settings first, just in case.
    //
    TCommPortCfg cpcfgTest
    (
        2400
        , tCIDComm::EFlags::AllBits
        , tCIDComm::EParities::Odd
        , tCIDComm::EDataBits::Seven
        , tCIDComm::EStopBits::Two
        , tCIDComm::EPortDTR::Enable
        , tCIDComm::EPortRTS::Handshake
    );

    conncfgTest.cpcfgSerial(cpcfgTest);
    conncfgTest.strPortPath(L"/Local/COM1");

    if (conncfgTest.strPortPath() != L"/Local/COM1")
    {
        strmOut << TFWCurLn << L"Setting port path failed\n\n";
        return tTestFWLib::ETestRes::Failed;
    }

    if (conncfgTest.cpcfgSerial() != cpcfgTest)
    {
        strmOut << TFWCurLn << L"Setting port configuration failed\n\n";
        return tTestFWLib::ETestRes::Failed;
    }

    // Test the non-default constructor
    TCQCSerialConnCfg conncfgTest2
    (
        cpcfgTest
        , L"/Local/COM2"
        , tCIDComm::EPortTypes::RS422
    );

    if (conncfgTest2.strPortPath() != L"/Local/COM2")
    {
        strmOut << TFWCurLn << L"Constructor failed on port path\n\n";
        return tTestFWLib::ETestRes::Failed;
    }

    if (conncfgTest2.cpcfgSerial() != cpcfgTest)
    {
        strmOut << TFWCurLn << L"Constructor failed on port config\n\n";
        return tTestFWLib::ETestRes::Failed;
    }

    // Copy it and test for equality
    TCQCSerialConnCfg conncfgCopy(conncfgTest);

    if (conncfgCopy != conncfgTest)
    {
        strmOut << TFWCurLn << L"Copy did not create identical objects\n\n";
        return tTestFWLib::ETestRes::Failed;
    }

    // And the same check as well (this is used for polymorphic comparisons)
    if (!conncfgCopy.bIsSame(conncfgTest))
    {
        strmOut << TFWCurLn << L"Polymorphic equivalence failed\n\n";
        return tTestFWLib::ETestRes::Failed;
    }

    //
    //  Parsing config in this case is just the port path, so it's pretty
    //  simple. But, it tests that it's a valid port path, so it's hard
    //  to test in a way that will work on any machine, so we don't do it.
    //

    // Do a round trip stream and make sure it comes back the same
    {
        TBinMBufOutStream strmOut(8192UL);
        TBinMBufInStream strmIn(strmOut);
        strmOut << conncfgTest << kCIDLib::FlushIt;
        strmIn.Reset();
        strmIn >> conncfgCopy;
    }

    if (conncfgTest != conncfgCopy)
    {
        strmOut << TFWCurLn << L"Round trip streaming failed\n\n";
        return tTestFWLib::ETestRes::Failed;
    }

    return eRes;
}





// ---------------------------------------------------------------------------
//  CLASS: TTest_UPnPConnCfg
// PREFIX: tfwt
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TTest_UPnPConnCfg: Constructor and Destructor
// ---------------------------------------------------------------------------
TTest_UPnPConnCfg::TTest_UPnPConnCfg() :

    TTestFWTest
    (
        L"UPnP Conn Cfg", L"Tests the UPnP connection configuration class", 2
    )
{
}

TTest_UPnPConnCfg::~TTest_UPnPConnCfg()
{
}


// ---------------------------------------------------------------------------
//  TTest_UPnPConnCfg: Public, inherited methods
// ---------------------------------------------------------------------------
tTestFWLib::ETestRes
TTest_UPnPConnCfg::eRunTest(TTextStringOutStream&   strmOut
                            , tCIDLib::TBoolean&    bWarning)
{
    tTestFWLib::ETestRes eRes = tTestFWLib::ETestRes::Success;

    //
    //  Set up an object and test the initial state, then do some standard
    //  sorts of grunt work tests on the resulting object.
    //
    {
        TCQCUPnPConnCfg conncfgTest;

        // All the values should be empty
        if (!conncfgTest.strDevName().bIsEmpty()
        ||  !conncfgTest.strDevUID().bIsEmpty()
        ||  !conncfgTest.strDispSvcType().bIsEmpty()
        ||  !conncfgTest.strDispVarName().bIsEmpty()
        ||  !conncfgTest.strTypeDescr().bIsEmpty())
        {
            strmOut << TFWCurLn << L"Wrong state after def ctor\n\n";
            return tTestFWLib::ETestRes::Failed;
        }

        // Copy it and test for equality
        TCQCUPnPConnCfg conncfgCopy(conncfgTest);

        if (conncfgCopy != conncfgTest)
        {
            strmOut << TFWCurLn << L"Copy did not create identical objects\n\n";
            return tTestFWLib::ETestRes::Failed;
        }

        // And the same check as well (this is used for polymorphic comparisons)
        if (!conncfgCopy.bIsSame(conncfgTest))
        {
            strmOut << TFWCurLn << L"Polymorphic equivalence failed\n\n";
            return tTestFWLib::ETestRes::Failed;
        }

        // Do a round trip stream and make sure it comes back the same
        {
            TBinMBufOutStream strmOut(8192UL);
            TBinMBufInStream strmIn(strmOut);
            strmOut << conncfgTest << kCIDLib::FlushIt;
            strmIn.Reset();
            strmIn >> conncfgCopy;
        }

        if (conncfgTest != conncfgCopy)
        {
            strmOut << TFWCurLn << L"Round trip streaming failed\n\n";
            return tTestFWLib::ETestRes::Failed;
        }

        // Make sure that changing something causes inequality
        conncfgTest.strDevName(L"Bubba");
        if (conncfgTest == conncfgCopy)
        {
            strmOut << TFWCurLn << L"Unequal objects come out equal\n\n";
            return tTestFWLib::ETestRes::Failed;
        }
    }

    // Test the scheme of parsing form a string
    {
        TCQCUPnPConnCfg conncfgTest;
        const tCIDLib::TCh* pszFmtInfo = L"urn:schemas-upnp-org:device:ZonePlayer:1 "
                                         L"RINCON_000E582D558E01400 Zone Player";
        if (!conncfgTest.bParseConnInfo(pszFmtInfo))
        {
            strmOut << TFWCurLn << L"Could not parse formatted conn info\n\n";
            return tTestFWLib::ETestRes::Failed;
        }

        //
        //  Make sure it got parsed right. In this case, the service type and
        //  variable name are not used, and the device name is set to unknown.
        //
        if ((conncfgTest.strDevUID() != L"RINCON_000E582D558E01400")
        ||  (conncfgTest.strDevType() != L"urn:schemas-upnp-org:device:ZonePlayer:1")
        ||  (conncfgTest.strTypeDescr() != L"Zone Player"))
        {
            strmOut << TFWCurLn << L"Parse of formatted info resulted in wrong state\n\n";
            return tTestFWLib::ETestRes::Failed;
        }

        if (!conncfgTest.strDispSvcType().bIsEmpty()
        ||  !conncfgTest.strDispVarName().bIsEmpty())
        {
            strmOut << TFWCurLn << L"SvcType/VarName should have remained empty\n\n";
            return tTestFWLib::ETestRes::Failed;
        }
    }

    return eRes;
}

