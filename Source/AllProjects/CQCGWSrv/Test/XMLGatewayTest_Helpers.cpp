//
// FILE NAME: XMLGatewayTest_Helpers.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 12/21/2002
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
//  This file just handles some overflow private methods of the facility
//  class, mainly just gruntwork stuff.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ----------------------------------------------------------------------------
//  Includes
// ----------------------------------------------------------------------------
#include    "XMLGatewayTest.hpp"


// ----------------------------------------------------------------------------
//  TFacXMLGatewayTest: Private, non-virtual methods
// ----------------------------------------------------------------------------

tCIDLib::TBoolean TFacXMLGatewayTest::bParseParms()
{
    TString strCurParm;
    TSysInfo::TCmdLineCursor cursParms = TSysInfo::cursCmdLineParms();
    for (; cursParms; ++cursParms)
    {
        strCurParm = *cursParms;

        // We know what is by what it starts with
        if (strCurParm.bStartsWithI(L"/Addr="))
        {
            strCurParm.Cut(0, 6);
            m_strServerAddr = strCurParm;
        }
         else if (strCurParm.bCompareI(L"/Secure"))
        {
            m_bSecure = kCIDLib::True;
        }
         else if (strCurParm.bCompareI(L"/Logging"))
        {
//            m_bLogging = kCIDLib::True;
        }
         else if (strCurParm.bStartsWithI(L"/Password="))
        {
            strCurParm.Cut(0, 10);
            m_strPassword = strCurParm;
        }
         else if (strCurParm.bStartsWithI(L"/Port="))
        {
            strCurParm.Cut(0, 6);
            m_ippnServer = strCurParm.c4Val();
        }
         else if (strCurParm.bStartsWithI(L"/User="))
        {
            strCurParm.Cut(0, 6);
            m_strUserName = strCurParm;
        }
         else
        {
            m_conOut << L"'" << strCurParm
                     << L"' is not a known parameter" << kCIDLib::EndLn;
        }
    }

    // Default the server to our own host if none is provided explicitly
    if (m_strServerAddr.bIsEmpty())
        m_strServerAddr = L"localhost";

    // If either the user name or password is empty, we failed
    if (m_strUserName.bIsEmpty() || m_strPassword.bIsEmpty())
        return kCIDLib::False;

    return kCIDLib::True;
}


// Get a cardinal value from the user
tCIDLib::TBoolean
TFacXMLGatewayTest::bGetCardVal(const   TString&            strPrompt
                                ,       tCIDLib::TCard4&    c4ToFill
                                , const tCIDLib::TCard4     c4MinVal
                                , const tCIDLib::TCard4     c4MaxVal)
{
    TString strVal;
    m_conOut << strPrompt << L">" << kCIDLib::FlushIt;
    while(kCIDLib::True)
    {
        if (!m_conIn.c4GetLine(strVal, 0, kCIDLib::True) || strVal.bIsEmpty())
            return kCIDLib::False;

        if (strVal.bToCard4(c4ToFill, tCIDLib::ERadices::Dec))
        {
            if ((c4ToFill < c4MinVal) || (c4ToFill > c4MaxVal))
                m_conOut << L" Out of range, try again> " << kCIDLib::FlushIt;
            else
                break;
        }
         else
        {
            m_conOut << L" Not a valid number, try again> " << kCIDLib::FlushIt;
        }
    }
    return kCIDLib::True;
}


// Get an integer value from the user
tCIDLib::TBoolean
TFacXMLGatewayTest::bGetIntVal( const   TString&        strPrompt
                                ,       tCIDLib::TInt4& i4ToFill
                                , const tCIDLib::TInt4  i4MinVal
                                , const tCIDLib::TInt4  i4MaxVal)
{
    TString strVal;
    m_conOut << strPrompt << L">" << kCIDLib::FlushIt;
    while(kCIDLib::True)
    {
        if (!m_conIn.c4GetLine(strVal, 0, kCIDLib::True) || strVal.bIsEmpty())
            return kCIDLib::False;

        if (strVal.bToInt4(i4ToFill, tCIDLib::ERadices::Dec))
        {
            if ((i4ToFill < i4MinVal) || (i4ToFill > i4MaxVal))
                m_conOut << L" Out of range, try again> " << kCIDLib::FlushIt;
            else
                break;
        }
         else
        {
            m_conOut << L" Not a valid number, try again> " << kCIDLib::FlushIt;
        }
    }
    return kCIDLib::True;
}



// Show our supports commands to the output console
tCIDLib::TVoid TFacXMLGatewayTest::ShowCommands()
{
    m_conOut << L"Supported commands:\n\n"
                L"    ?\n"
                L"    Help\n\n"

                L"    DoGlobAct\n"
                L"    MWriteField\n"
                L"    PauseSchEv\n"
                L"    Poll\n"
                L"    QueryDrvs\n"
                L"    QueryDrvText\n"
                L"    QueryFlds\n"
                L"    QueryFldInfo\n"
                L"    QueryGlobActs\n"
                L"    QueryMacros\n"
                L"    QueryMediaArt\n"
                L"    QueryMediaDB\n"
                L"    QueryMediaRendArt\n"
                L"    QueryPerEvInfo\n"
                L"    QueryRendPL\n"
                L"    QueryRepoDrvs\n"
                L"    QueryRmCfg\n"
                L"    QueryRmCfgList\n"
                L"    QuerySchEv\n"
                L"    QuerySchEvInfo\n"
                L"    QuerySunEvInfo\n"
                L"    QuerySchEvs\n"
                L"    ReadField\n"
                L"    RunMacro\n"
                L"    SetPerEvTime\n"
                L"    SetPollList\n"
                L"    SetSchEvTime\n"
                L"    SetSunEvTime\n"
                L"    ShowPollList\n"
                L"    WriteField\n"
             << kCIDLib::NewLn << kCIDLib::FlushIt;
}


//
//  A helper to dump the hierarchical tree that a couple of queries
//  returns. This guy is recursive, but indirectly. The tree collection
//  cursor handles the recursion for us.
//
tCIDLib::TVoid
TFacXMLGatewayTest::ShowTree(const TTreeCol& colToShow)
{
    TTreeCol::TCursor cursTree(&colToShow);
    do
    {
        m_conOut << TTextOutStream::Spaces(cursTree.c4Depth() * 3);

        if (cursTree.eCurType() == tCIDLib::ETreeNodes::NonTerminal)
            m_conOut << L"<";
        m_conOut << cursTree.strCurName();
        if (cursTree.eCurType() == tCIDLib::ETreeNodes::NonTerminal)
            m_conOut << L">";
        m_conOut << kCIDLib::NewLn;
    }   while (cursTree.bNext());
}


tCIDLib::TVoid TFacXMLGatewayTest::ShowUsage()
{
    m_conOut << L"Usage:\n    XMLGatewayTest params\n\n"
                L"  Required:\n"
                L"    /User=uuu     - User name to log on as\n"
                L"    /Password=www - The password to log on with\n\n"
                L"  Optional:\n"
                L"    /Port=ppp     - The port the server is listening on. Defaults to "
             << kCQCKit::ippnCQCGWSrvPort << kCIDLib::NewLn
             << L"    /Addr=aaa     - The DNS name of the gateway server. Defaults to local host\n"
//             << L"    /Logging      - Log all XML traffic to a file\n"
             << kCIDLib::NewLn << kCIDLib::FlushIt;
}


