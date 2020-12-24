//
// FILE NAME: XMLGatewayTest.hpp
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
//  This is the main header file of the gateway server test program. This
//  program simulates a client of the CQC XML Gateway server. We use the
//  canned XML GW client facility to talk to the server, so that we test it
//  at the same time.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


// ----------------------------------------------------------------------------
//  Includes
//
//  We have a special case one for the gateway structures and contants and
//  such. This one is a standalone header because clients won't usually be
//  CIDLib based programs, so they cannot depend on CIDLib or CQC stuff.
// ----------------------------------------------------------------------------
#include    "CIDLib.hpp"
#include    "CIDSock.hpp"
#include    "CIDImgFact.hpp"
#include    "CIDXML.hpp"
#include    "CQCKit.hpp"
#include    "CQCGWCl.hpp"




// ---------------------------------------------------------------------------
//  Include our own headers
// ---------------------------------------------------------------------------
#include    "XMLGateWayTest_ErrorIds.hpp"
#include    "XMLGateWayTest_MessageIds.hpp"


// ---------------------------------------------------------------------------
//   CLASS: TFacXMLGatewayTest
//  PREFIX: fac
// ---------------------------------------------------------------------------
class TFacXMLGatewayTest : public TFacility, public MSignalHandler
{
    public :
        // -------------------------------------------------------------------
        //  Class types
        // -------------------------------------------------------------------
        typedef TCQCGWSrvClient::TNameTreeCol   TTreeCol;


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TFacXMLGatewayTest();

        TFacXMLGatewayTest(const TFacXMLGatewayTest&) = delete;
        TFacXMLGatewayTest(TFacXMLGatewayTest&&) = delete;

        ~TFacXMLGatewayTest();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TFacXMLGatewayTest& operator=(const TFacXMLGatewayTest&) = delete;
        TFacXMLGatewayTest& operator=(TFacXMLGatewayTest&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bHandleSignal
        (
            const   tCIDLib::ESignals       eSignal
        )   final;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::EExitCodes eMainThread
        (
                    TThread&                thrThis
            ,       tCIDLib::TVoid*         pData
        );


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bGetCardVal
        (
            const   TString&                strPrompt
            ,       tCIDLib::TCard4&        c4ToFill
            , const tCIDLib::TCard4         c4MinVal = 0
            , const tCIDLib::TCard4         c4MaxVal = kCIDLib::c4MaxCard
        );

        tCIDLib::TBoolean bGetIntVal
        (
            const   TString&                strPrompt
            ,       tCIDLib::TInt4&         i4ToFill
            , const tCIDLib::TInt4          i4MinVal = kCIDLib::i4MinInt
            , const tCIDLib::TInt4          i4MaxVal = kCIDLib::i4MaxInt
        );

        tCIDLib::TBoolean bParseParms();

        tCIDLib::TVoid DoGlobalAct();

        tCIDLib::TVoid PauseEvent
        (
            const   tCIDLib::TBoolean       bScheduled
        );

        tCIDLib::TVoid QueryDrivers();

        tCIDLib::TVoid QueryDriverText();

        tCIDLib::TVoid QueryFlds();

        tCIDLib::TVoid QueryFldInfo();

        tCIDLib::TVoid QueryGlobActs();

        tCIDLib::TVoid QueryMacros();

        tCIDLib::TVoid QueryMediaArt();

        tCIDLib::TVoid QueryMediaDB();

        tCIDLib::TVoid QueryMediaRendArt();

        tCIDLib::TVoid QueryPerEv();

        tCIDLib::TVoid QueryRendPL();

        tCIDLib::TVoid QueryRepoDrvs();

        tCIDLib::TVoid QueryRmCfg();

        tCIDLib::TVoid QueryRmCfgList();

        tCIDLib::TVoid QuerySchEv();

        tCIDLib::TVoid QuerySchEvInfo();

        tCIDLib::TVoid QuerySunEv();

        tCIDLib::TVoid QuerySchEvs();

        tCIDLib::TVoid MWriteField();

        tCIDLib::TVoid PollFields();

        tCIDLib::TVoid ReadField();

        tCIDLib::TVoid RunMacro();

        tCIDLib::TVoid SetPollList();

        tCIDLib::TVoid SetPerEvTime();

        tCIDLib::TVoid SetSchEvTime();

        tCIDLib::TVoid SetSunEvTime();

        tCIDLib::TVoid ShowCommands();

        tCIDLib::TVoid ShowPollList();

        tCIDLib::TVoid ShowTree
        (
            const   TTreeCol&               colToShow
        );

        tCIDLib::TVoid ShowUsage();

        tCIDLib::TVoid WriteField();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bEncrypted
        //      Set via the /Secure parameter. We store this here until we
        //      initialize the gateway client.
        //
        //  m_bShutdown
        //      To support a signal handler for shutdown purposes. The handler
        //      sets this flag.
        //
        //  m_c4CursorSize
        //      If we have a media cursor, we store the reported number of
        //      items here when we get the cursor. So that we can tell the
        //      user what the max index is when he does a browse.
        //
        //  m_c4SchEvSerialNum
        //      We store the last scheduled event list serial number we got and
        //      pass it back in later.
        //
        //  m_conIn
        //  m_conOut
        //      Our input and output consoles.
        //
        //  m_gwscTest
        //      The gateware server client object we use to do our comm with
        //      the server.
        //
        //  m_ippnServer
        //      The port that the server is listening on, and which we need
        //      to connect.
        //
        //  m_strCursorId
        //      If we have a media browsing cursor, we store it here. When we
        //      have none (or have closed the one we had before), this will be
        //      empty.
        //
        //  m_strPassword
        //      The password given on the command line, which we use to log
        //      on to the server.
        //
        //  m_strServerAddr
        //      The address of the server we should connect to.
        //
        //  m_strUserName
        //      The user name given on the command line, which use to log on
        //      to the server.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bSecure;
        tCIDLib::TBoolean       m_bShutdown;
        tCIDLib::TCard4         m_c4CursorSize;
        tCIDLib::TCard4         m_c4SchEvSerialNum;
        TInConsole              m_conIn;
        TOutConsole             m_conOut;
        TCQCGWSrvClient         m_gwscTest;
        tCIDLib::TIPPortNum     m_ippnServer;
        TString                 m_strCursorId;
        TString                 m_strPassword;
        TString                 m_strServerAddr;
        TString                 m_strUserName;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TFacXMLGatewayTest,TFacility)
};


// ---------------------------------------------------------------------------
//  Export the global facility object to all cpp files
// ---------------------------------------------------------------------------
extern TFacXMLGatewayTest facXMLGatewayTest;
