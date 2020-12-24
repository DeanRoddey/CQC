//
// FILE NAME: CQCNetTest.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/11/2005
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
//  $Log$
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include "CQCNetTest.hpp"


// ---------------------------------------------------------------------------
//  Forward references
// ---------------------------------------------------------------------------
tCIDLib::EExitCodes eMainThreadFunc
(
        TThread&            thrThis
    ,   tCIDLib::TVoid*     pData
);


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
static TStreamFmt   strmfLegend(18, 2, tCIDLib::EHJustify::Right, kCIDLib::chSpace);
static TStreamFmt   strmfData(0, 2, tCIDLib::EHJustify::Left, kCIDLib::chSpace);
static TString      strMSHost;


// ---------------------------------------------------------------------------
//  Do the magic main module code
// ---------------------------------------------------------------------------
CIDLib_MainModule(TThread(L"CQCNetTest", eMainThreadFunc))


// -----------------------------------------------------------------------------
//  Local functions
// -----------------------------------------------------------------------------


// Shows all the basic local host info
static tCIDLib::TVoid ShowInfo(TTextOutStream& strmOut)
{
    strmOut << L"System Information\n" << TTextOutStream::RepChars(L'-', 32)
            << kCIDLib::EndLn;

    // Show operating system info
    tCIDLib::TCard4 c4OSMajVer, c4OSMinVer, c4OSRev, c4BuildNum;
    TSysInfo::QueryOSInfo(c4OSMajVer, c4OSMinVer, c4OSRev, c4BuildNum);
    strmOut << strmfLegend << L"OS Version: "
            << strmfData << c4OSMajVer << L"." << c4OSMinVer << L"." << c4OSRev
            << L", Build: " << c4BuildNum << kCIDLib::NewLn;

    // Show local IP address info
    strmOut << strmfLegend << L"IP Name: "
            << strmfData << TSysInfo::strIPHostName() << kCIDLib::NewLn
            << strmOut << strmfLegend << L"Host Name: "
            << strmfData << TSysInfo::strOSHostName() << kCIDLib::NewLn;

    // And local date/time
    {
        TTime tmCur(tCIDLib::ESpecialTimes::CurrentTime);
        tmCur.strDefaultFormat(TTime::strCTime());
        strmOut << strmfLegend << L"Local Date/Time: "
                << tmCur << kCIDLib::NewLn;
    }

    tCIDLib::TCard4 c4TCPMajVer;
    tCIDLib::TCard4 c4TCPMinVer;
    facCIDSock().QueryTCPVersion(c4TCPMajVer, c4TCPMinVer);
    TIPHostInfo iphiLocal(facCIDSock().strIPHostName(), kCIDLib::False);
    strmOut << strmfLegend << L"TCP Version: "
            << strmfData << c4TCPMajVer << L"." << c4TCPMinVer << kCIDLib::NewLn
            << kCIDLib::NewLn;

    // Show local adaptor info
    strmOut << kCIDLib::NewLn << L"Network Adaptor Information\n"
            << TTextOutStream::RepChars(L'-', 32)
            << kCIDLib::NewLn;

    TVector<THostAdapter> colAdapters;
    const tCIDLib::TCard4 c4Count = facCIDSock().c4QueryAdapterList(colAdapters);
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const THostAdapter& hadpCur = colAdapters[c4Index];
        strmOut << L"  " << hadpCur.strName();
        if (hadpCur.bIsTunnel() || !hadpCur.bIsDedicated())
            strmOut << L"  (Ignored)";

        strmOut << L"\n  " << hadpCur.strDescr()
                << L"\n           Ready: " << hadpCur.bIsReady()
                << L"\n    DHCP Enabled: " << hadpCur.bDHCPEnabled()
                << L"\n    IPV4 Enabled: " << hadpCur.bIPV4Enabled()
                << L"\n    IPV6 Enabled: " << hadpCur.bIPV6Enabled()
                << L"\n     Is Loopback: " << hadpCur.bIsLoopback()
                << L"\n       Is Tunnel: " << hadpCur.bIsTunnel()
                << L"\n       Dedicated: " << hadpCur.bIsDedicated()
                << L"\n   Hardware Addr: ";

        strmOut << TTextOutStream::Width(2);
        strmOut << TTextOutStream::Justify(tCIDLib::EHJustify::Right);
        strmOut << TTextOutStream::Fill(kCIDLib::chDigit0);
        for (tCIDLib::TCard4 c4HWInd = 0; c4HWInd < hadpCur.c4HWAddrSz(); c4HWInd++)
        {
            strmOut  << TCardinal(hadpCur.c1HWAddrByteAt(c4HWInd), tCIDLib::ERadices::Hex)
                    << TTextOutStream::Spaces(1);
        }
        strmOut.SetDefaultFormat();
        strmOut << kCIDLib::NewLn;

        // Dump all the addresses
        for (tCIDLib::TCard4 c4AInd = 0; c4AInd < hadpCur.c4AddrCnt(); c4AInd++)
        {
            strmOut  << L"    Address #" << c4AInd << L": "
                    << hadpCur.ipaAddrAt(c4AInd) << kCIDLib::EndLn;
        }
        strmOut << kCIDLib::NewLn;
    }

    // Show some environment info
    strmOut << kCIDLib::NewLn << L"Environment Information\n"
            << TTextOutStream::RepChars(L'-', 32) << kCIDLib::EndLn;

    TString strValue;
    strmOut << strmfLegend << L"CID_NSADDR: ";
    if (TProcEnvironment::bFind(L"CID_NSADDR", strValue))
    {
        strMSHost = strValue;
        tCIDLib::TCard4 c4Index;
        if (strMSHost.bFirstOccurrence(L':', c4Index))
            strMSHost.CapAt(c4Index);
    }
     else
    {
        strMSHost = TSysInfo::strIPHostName();
    }
    strmOut << strmfData << strMSHost << kCIDLib::NewLn;

    strmOut << strmfLegend << L"CQC_DATADIR (client): "
            << strmfData << facCQCKit().strDataDir()
            << kCIDLib::NewLn;

    strmOut << strmfLegend << L"CQC_DATADIR (server): ";
    if (TProcEnvironment::bFind(kCQCKit::pszDataDirVarName, strValue))
        strmOut << strmfData << strValue;
    strmOut << kCIDLib::DNewLn;

    strmOut << kCIDLib::EndLn;
}


//
//  We get the name server binding for the core admin interface of a
//  server, and query its core statistics and print them out.
//
static tCIDLib::TVoid
ShowSrvStats(       TTextOutStream&         strmOut
            , const TString&                strAdminBinding
            ,       tCIDOrbUC::TNSrvProxy&  orbcNS
            , const TString&                strProcTitle
            , const tCIDLib::TBoolean       bMustExist)
{
    TOrbObjId       ooidSrv;
    TString         strHostName;
    TString         strProcName;

    // Check the binding to see if it exists
    const tCIDLib::TBoolean bNotFound = !facCIDOrbUC().bGetNSObjectMS
    (
        orbcNS, strAdminBinding, ooidSrv, 1000
    );

    // If not found and it doesn't have to exist, just return now
    if (bNotFound && !bMustExist)
        return;

    TStreamFmt  strmfName(30, 0, tCIDLib::EHJustify::Right, kCIDLib::chSpace);
    TStreamFmt  strmfValue(0, 0, tCIDLib::EHJustify::Left, kCIDLib::chSpace);

    strmOut  << strmfName << L"Stats For Process: "
             << strmfValue << strProcTitle << kCIDLib::NewLn
             << L"    " << TTextOutStream::RepChars(L'-', 28) << kCIDLib::EndLn;

    if (bNotFound)
    {
        //
        //  We show some info from the object id whether we find the server or
        //  not. In this case it won't have any resolved address info, so we
        //  show the text formatted address info.
        //
        strmOut << strmfName << L"Object Id: "
                << strmfValue << ooidSrv.strAddrText() << kCIDLib::NewLn
                << L"    The program is not registered in the name server\n"
                << kCIDLib::EndLn;
        return;
    }

    // It seems to be there, so try it
    tCIDOrbUC::TCoreAdminProxy orbcAdmin
    (
        facCIDOrbUC().orbcCoreSrvAdminProxy(orbcNS, strAdminBinding)
    );

    // Show the object id. Set the resolved address on it first
    ooidSrv.SetCachedAddr(orbcAdmin->ipepServer());
    strmOut << strmfName << L"Object Id: "
            << strmfValue << ooidSrv << kCIDLib::NewLn;


    // Get all the stats related stuff
    tCIDLib::TCard8 c8Stamp = 0;
    TVector<TStatsCacheItemInfo> colValues;
    tCIDLib::TCard4 c4ValueCnt = orbcAdmin->c4QueryStats
    (
        kCIDLib::pszStat_Scope_Stats, colValues, kCIDLib::False, c8Stamp
    );

    // Get the info related stuff
    c8Stamp = 0;
    TVector<TStatsCacheItemInfo> colInfo;
    const tCIDLib::TCard4 c4InfoCnt = orbcAdmin->c4QueryStats
    (
        kCIDLib::pszStat_Scope_Info, colInfo, kCIDLib::False, c8Stamp
    );

    // Add the info list ot the stats list so we can have one list to sort
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4InfoCnt; c4Index++)
        colValues.objAdd(colInfo[c4Index]);
    c4ValueCnt += c4InfoCnt;

    // Sort the list
    colValues.Sort(TStatsCacheItemInfo::eCompNames);

    // And display them all
    TTime tmVal;
    tmVal.strDefaultFormat(TTime::strCTime());
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ValueCnt; c4Index++)
    {
        const TStatsCacheItemInfo& sciiCur = colValues[c4Index];

        strmOut << strmfName << sciiCur.strName()
                << strmfValue << L": ";

        // We display the various types differently
        if (sciiCur.eType() == tCIDLib::EStatItemTypes::Time)
        {
            tmVal.enctTime(sciiCur.c8Value());
            strmOut << tmVal;
        }
         else if (sciiCur.eType() == tCIDLib::EStatItemTypes::Flag)
        {
            if (sciiCur.c8Value())
                strmOut << L"True";
            else
                strmOut << L"False";
        }
         else
        {
            strmOut << sciiCur.c8Value();

            // If a percent, put the percent sign out
            if (sciiCur.eType() == tCIDLib::EStatItemTypes::Percent)
                strmOut << kCIDLib::chPercentSign;
        }
        strmOut << kCIDLib::NewLn;
    }
    strmOut << kCIDLib::EndLn;
}


static tCIDLib::TVoid TestNameRes(TTextOutStream& strmOut)
{
    strmOut.SetDefaultFormat();
    strmOut << L"Name Resolution\n" << TTextOutStream::RepChars(L'-', 32)
            << kCIDLib::EndLn;

    // Display the reported available address families
    strmOut << L"    Reported Addr Families: ";
    tCIDSock::EAddrTypes eTypes = facCIDSock().eAvailAddrType();
    if (eTypes == tCIDSock::EAddrTypes::Unspec)
        strmOut << L"IPV4 & IPV6";
    else if (eTypes == tCIDSock::EAddrTypes::IPV4)
        strmOut << L"IPV4";
    else if (eTypes == tCIDSock::EAddrTypes::IPV6)
        strmOut << L"IPV6";
    else
        strmOut << L"[Unknown]";
    strmOut << kCIDLib::DNewLn;


    // Try to resolve the local host name
    strmOut << L"    Looking up host name (default address family): "
            << TSysInfo::strIPHostName() << L" ...\n" << kCIDLib::FlushIt;
    try
    {
        TIPAddress ipaTest(TSysInfo::strIPHostName(), tCIDSock::EAddrTypes::Unspec);
        strmOut << L"    Resolved to: " << ipaTest.strAsText() << kCIDLib::NewLn
                << L"        Reverse: " << ipaTest.strHostName() << kCIDLib::NewEndLn;

        // Do a forced address family lookup for the supported families
        if (TKrnlIP::bIPV4Avail())
        {
            TIPAddress ipaTest(TSysInfo::strIPHostName(), tCIDSock::EAddrTypes::IPV4);
            strmOut << L"   IPV4 Lookup: " << ipaTest.strAsText()
                    << kCIDLib::EndLn;
        }

        if (TKrnlIP::bIPV6Avail())
        {
            TIPAddress ipaTest(TSysInfo::strIPHostName(), tCIDSock::EAddrTypes::IPV6);
            strmOut << L"   IPV6 Lookup: " << ipaTest.strAsText()
                    << kCIDLib::EndLn;
        }

        strmOut << kCIDLib::EndLn;
    }

    catch(const TError& errToCatch)
    {
        strmOut << L"\nResolution failed! Error=\n"
                << errToCatch << kCIDLib::NewEndLn;
        return;
    }

    // Try to resolve the name server host
    strmOut << L"    Trying to resolve name server host ["
            << facCIDOrbUC().strNSHost() << L"."
            << facCIDOrbUC().ippnNS()
            << L"]" << kCIDLib::FlushIt;
    try
    {
        TIPEndPoint ipepNS
        (
            facCIDOrbUC().strNSHost()
            , facCIDOrbUC().ippnNS()
            , tCIDSock::EAddrTypes::Unspec
        );
        strmOut << L"\n    Resolved to: " << ipepNS << kCIDLib::NewEndLn;
    }

    catch(const TError& errToCatch)
    {
        strmOut << L"\nResolution failed! Error=\n"
                << errToCatch << kCIDLib::NewEndLn;
        return;
    }

    strmOut << kCIDLib::EndLn;
}


//
//  This will do various tests on a single server, to verify that everything
//  seems ok and provide plenty of info if not. It's called by TestCQCSrvs
//  below for each of the MS servers (other than the NS which it does itself.)
//
static tCIDLib::TBoolean
TestOneSrv(         TTextOutStream&         strmOut
            ,       tCIDOrbUC::TNSrvProxy&  orbcNS
            , const TString&                strBinding
            , const TString&                strDescr)
{
    strmOut << L"    Testing " << strDescr << L"...\n";

    try
    {
        // Get the object id from the name server. If not found, given up
        TOrbObjId ooidNS;
        if (!facCIDOrbUC().bGetNSObjectMS(orbcNS, strBinding, ooidNS, 1000))
        {
            strmOut << L"       The " << strDescr << L" is not available"
                    << kCIDLib::EndLn;
            return kCIDLib::False;
        }

        // Display the name server binding info, formatted out nicely
        strmOut << L"        Server is registered at "
                << ooidNS.strAddrText() << L'.' << ooidNS.ippnHost();

        // Get the cached object id if it's cached
        TOrbObjId ooidCache;
        if (facCIDOrb().bCheckOOIDCache(strBinding, ooidCache))
        {
            if (ooidCache != ooidNS)
            {
                strmOut << L"       The cached object id for the " << strDescr
                        << L" is not the same as that from the name server!"
                        << kCIDLib::EndLn;
            }

            // If there's a cached address, display that
            if (ooidCache.bHasCachedAddr())
                strmOut << L" [" << ooidCache.ipaCached() << L']';
        }
        strmOut << kCIDLib::EndLn;
    }

    catch(const TError& errToCatch)
    {
        strmOut << L"        Error contacting the " << strDescr << L" ERROR=\n"
                << errToCatch << kCIDLib::DNewLn << kCIDLib::EndLn;

        return kCIDLib::False;
    }

    return kCIDLib::True;
}


static tCIDLib::TVoid TestCQCSrvs(TTextOutStream& strmOut)
{
    strmOut << L"\nCQC Connection Tests\n" << TTextOutStream::RepChars(L'-', 32)
            << kCIDLib::EndLn;

    try
    {
        // Try the name server
        strmOut << L"    Trying to connect to CQC name server" << kCIDLib::EndLn;

        TString strBinding;
        tCIDOrbUC::TNSrvProxy orbcNS;
        try
        {
            orbcNS = facCIDOrbUC().orbcNameSrvProxy(4000);
            strmOut << L"    Connected to name server successfully at: "
                    << orbcNS->ooidThis().strAddrText() << L"."
                    << orbcNS->ooidThis().ippnHost()
                    << L"\n       ["
                    << orbcNS->ipepServer()
                    << L"]"
                    << kCIDLib::NewEndLn;
        }

        catch(...)
        {
            strmOut << L"Failed to contact name server.\n"
                    << kCIDLib::NewEndLn;
            throw;
        }

        // Try the log server
        {
            if (TestOneSrv(strmOut, orbcNS, TCIDLogSrvClientProxy::strBinding, L"Log Server"))
            {
                try
                {
                    // It worked so connect via the helper method and do a simple call
                    tCIDOrbUC::TLSrvProxy orbcLS = facCIDOrbUC().orbcLogSrvProxy(2000);

                    // Try to just query one log event
                    TVector<TLogEvent> colToFill;
                    orbcLS->c4QueryEvents(colToFill, 1);

                    strmOut << L"        Log Server tests completed successfully.\n"
                            << kCIDLib::EndLn;
                }

                catch(const TError& errToCatch)
                {
                    strmOut << L"        Log Server tests failed. ERROR=\n"
                            << errToCatch << kCIDLib::DNewLn << kCIDLib::EndLn;
                }
            }
        }

        // Do the client service
        {
            strBinding = kCQCKit::pszCQCSlSrvIntf;
            strBinding.eReplaceToken(TSysInfo::strIPHostName(), L'h');
            if (TestOneSrv(strmOut, orbcNS, strBinding, L"Client Service"))
            {
                TOrbObjId ooidClSrv;
                facCIDOrbUC().bGetNSObjectMS(orbcNS, strBinding, ooidClSrv, 1000);
                TCQCClServiceClientProxy orbcClSrv(ooidClSrv, strBinding);

                try
                {
                    tCIDLib::TStrList colRepos;
                    orbcClSrv.c4QueryRepoList(colRepos);

                    strmOut << L"        Client service tests completed successfully.\n"
                            << kCIDLib::EndLn;

                }

                catch(const TError& errToCatch)
                {
                    strmOut << L"        Client Service tests failed. ERROR=\n"
                            << errToCatch << kCIDLib::DNewLn << kCIDLib::EndLn;
                }
            }
             else
            {
                strmOut << L"    \n!!!!Client service should be running on all machines"
                        << kCIDLib::DNewLn;
            }
        }

        //
        //  Try the local config server if it is registered. Leave this binding
        //  string visible since we'll use it below.
        //
        TString strLCfgBinding = TCIDCfgSrvClientProxy::strAdminBinding;
        strLCfgBinding.eReplaceToken(TSysInfo::strIPHostName(), L'h');
        {
            if (TestOneSrv(strmOut, orbcNS, strLCfgBinding, L"Local Cfg Server"))
            {
                // It worked so contact it the usual way and do a simple test
                try
                {
                    TCfgServerClient cfgcRepo;

                    // Do a simple check just to make sure it responds
                    tCIDLib::TStrList colChildren;
                    cfgcRepo.c4QuerySubScopes(L"/", colChildren);

                    strmOut << L"        Cfg Server tests completed successfully.\n"
                            << kCIDLib::EndLn;
                }

                catch(const TError& errToCatch)
                {
                    strmOut << L"        Local Cfg Server tests failed. ERROR=\n"
                            << errToCatch << kCIDLib::DNewLn << kCIDLib::EndLn;
                }
            }
        }

        // Try the installation server interface of the data server
        {
            if (TestOneSrv(strmOut, orbcNS, TCQCInstClientProxy::strBinding, L"Installation Server"))
            {
                try
                {
                    tCQCKit::TInstSrvProxy orbcIS = facCQCKit().orbcInstSrvProxy(4000);

                    orbcIS->Ping();
                    strmOut << L"        Installation Server tests completed successfully.\n"
                            << kCIDLib::EndLn;
                }

                catch(const TError& errToCatch)
                {
                    strmOut << L"        Installation Server tests failed. ERROR=\n"
                            << errToCatch << kCIDLib::DNewLn << kCIDLib::EndLn;
                }
            }
        }

        // Try the security server
        {
            if (TestOneSrv(strmOut, orbcNS, TCQCSecureClientProxy::strBinding, L"Security Server"))
            {
                try
                {
                    tCQCKit::TSecuritySrvProxy orbcSS = facCQCKit().orbcSecuritySrvProxy(4000);

                    // Just query the version, which doesn't require we be logged in
                    orbcSS->c8QueryVersion();
                    strmOut << L"        Security Server tests completed successfully.\n"
                            << kCIDLib::EndLn;
                }

                catch(const TError& errToCatch)
                {
                    strmOut << L"        Security Server tests failed. ERROR=\n"
                            << errToCatch << kCIDLib::DNewLn << kCIDLib::EndLn;
                }
            }
        }

        // Try the data server interface
        {
            if (TestOneSrv(strmOut, orbcNS, TDataSrvAccClientProxy::strBinding, L"Data Server"))
            {
                try
                {
                    TDataSrvClient dsclTest;

                    // Do a few calls
                    dsclTest.bFileExists(L"//MEng/User/MyMacro", tCQCRemBrws::EDTypes::Macro);
                    dsclTest.bIsScope(L"//MEng/User", tCQCRemBrws::EDTypes::Template);
                    dsclTest.Ping();

                    strmOut << L"        Data Server tests completed successfully.\n"
                            << kCIDLib::EndLn;
                }

                catch(const TError& errToCatch)
                {
                    strmOut << L"        Data Server tests failed. ERROR=\n"
                            << errToCatch << kCIDLib::DNewLn << kCIDLib::EndLn;
                }
            }
        }

        // Do any registered CQCServers
        TVector<TNameServerInfo> colSrvList;
        const tCIDLib::TCard4 c4CQCSrvCnt = orbcNS->c4EnumObjects
        (
            TCQCSrvAdminClientProxy::strAdminScope
            , colSrvList
            , kCIDLib::False
        );

        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4CQCSrvCnt; c4Index++)
        {
            strmOut << L"    Trying to connect to CQCServer on host "
                    << colSrvList[c4Index].strNodeName()
                    << kCIDLib::EndLn;

            try
            {
                TCQCSrvAdminClientProxy orbcAdmin
                (
                    colSrvList[c4Index].ooidSrvObject(), TString::strEmpty()
                );
                orbcAdmin.SendORBPing();
                strmOut << L"        Connected CQCServer successfully \n"
                        << kCIDLib::EndLn;
            }

            catch(const TError& errToCatch)
            {
                strmOut << L"        Failed to connect. ERROR=\n"
                        << errToCatch << kCIDLib::DNewLn << kCIDLib::EndLn;
            }
        }


        // Show server stats
        strmOut << L"\nServer Statistics\n" << TTextOutStream::RepChars(L'-', 32)
                << kCIDLib::NewEndLn;

        // Do the name server
        ShowSrvStats
        (
            strmOut
            , TCIDNameSrvClientProxy::strAdminBinding
            , orbcNS
            , L"Name Server"
            , kCIDLib::True
        );

        // Do the log server
        ShowSrvStats
        (
            strmOut
            , TCIDLogSrvClientProxy::strAdminBinding
            , orbcNS
            , L"Log Server"
            , kCIDLib::True
        );

        // Do the master server config server
        TString strMSCfgBinding = TCIDCfgSrvClientProxy::strAdminBinding;
        strMSCfgBinding.eReplaceToken(strMSHost, L'h');
        ShowSrvStats
        (
            strmOut
            , strMSCfgBinding
            , orbcNS
            , L"Master Cfg Server"
            , kCIDLib::True
        );

        // Do the data server
        ShowSrvStats
        (
            strmOut
            , kCQCKit::pszCQCDataSrvAdmin
            , orbcNS
            , L"Data Server"
            , kCIDLib::True
        );

        //
        //  Do the local config server if there is one and it's not
        //  the same as the MS config server binding we did above (in
        //  which case we are the MS.)
        //
        if (strLCfgBinding != strMSCfgBinding)
        {
            ShowSrvStats
            (
                strmOut
                , strLCfgBinding
                , orbcNS
                , L"Local Cfg Server"
                , kCIDLib::True
            );
        }

        //
        //  Show stats for any installed CQCServers. We already got the
        //  list above, so we don't have to do it again.
        //
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4CQCSrvCnt; c4Index++)
        {
            strBinding = TCQCSrvAdminClientProxy::strCoreAdminBinding;
            strBinding.eReplaceToken(colSrvList[c4Index].strNodeName(), L'h');

            TString strName(L"CQCServer on ");
            strName.Append(colSrvList[c4Index].strNodeName());
            ShowSrvStats
            (
                strmOut
                , strBinding
                , orbcNS
                , strName
                , kCIDLib::True
            );
        }

        // Show the event server
        ShowSrvStats
        (
            strmOut
            , kCQCKit::pszCQCEventSrvAdmin
            , orbcNS
            , L"Event Server"
            , kCIDLib::False
        );

        // If there's a logic server, show its stats
        ShowSrvStats
        (
            strmOut
            , kCQCKit::pszCQCLogicSrvAdmin
            , orbcNS
            , L"Logic Server"
            , kCIDLib::False
        );


        // Show any Web servers
        {
            const tCIDLib::TCard4 c4WebSrvCnt = orbcNS->c4EnumObjects
            (
                L"/CQC/CQCWebServer/CoreAdmins"
                , colSrvList
                , kCIDLib::False
            );

            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4WebSrvCnt; c4Index++)
            {
                strBinding = kCQCKit::pszCQCWebSrvCoreAdmin;
                strBinding.eReplaceToken(colSrvList[c4Index].strNodeName(), L'h');

                TString strName(L"Web Server on ");
                strName.Append(colSrvList[c4Index].strNodeName());
                ShowSrvStats
                (
                    strmOut
                    , strBinding
                    , orbcNS
                    , strName
                    , kCIDLib::True
                );
            }
        }


        // Show any XMLGW servers
        {
            const tCIDLib::TCard4 c4WebSrvCnt = orbcNS->c4EnumObjects
            (
                L"/CQC/CQCGWServer/CoreAdmins"
                , colSrvList
                , kCIDLib::False
            );

            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4WebSrvCnt; c4Index++)
            {
                strBinding = kCQCKit::pszCQCGWSrvCoreAdmin;
                strBinding.eReplaceToken(colSrvList[c4Index].strNodeName(), L'h');

                TString strName(L"XML GW Server on ");
                strName.Append(colSrvList[c4Index].strNodeName());
                ShowSrvStats
                (
                    strmOut
                    , strBinding
                    , orbcNS
                    , strName
                    , kCIDLib::True
                );
            }
        }

        strmOut.Flush();
    }

    catch(const TError& errToCatch)
    {
        strmOut << L"Test failed! Error=\n"
                << errToCatch << kCIDLib::NewLn << kCIDLib::EndLn;
    }
}



// ----------------------------------------------------------------------------
//  Program entry point
// ----------------------------------------------------------------------------
tCIDLib::EExitCodes eMainThreadFunc(TThread& thrThis, tCIDLib::TVoid*)
{
    // We have to let our calling thread go first
    thrThis.Sync();

    TTextOutStream& strmOut = TSysInfo::strmOut();
    try
    {
        strmOut << L"\nCQC Network Info Test\n"
                << L"Copyright (c) Charmed Quark Systems\n\n\n"
                << kCIDLib::FlushIt;

        //
        //  Ask CQCKit to load up environment info, which we'll need for
        //  almost anything.
        //
        TString strFailReason;
        if (!facCQCKit().bLoadEnvInfo(strFailReason, kCIDLib::True))
        {
            strmOut << L"You must run this program in the CQC Command Prompt"
                    << kCIDLib::EndLn;
            return tCIDLib::EExitCodes::BadEnvironment;
        }

        //
        //  First we will get some general network information and display
        //  it for validation purposes by the user.
        //
        ShowInfo(strmOut);

        //  We force a manual initialization of the ORB utilities client,
        //  so that it will force the name server address info to be faulted
        //  in and processed before we try to create a name server client
        //  proxy. Usually that's just allowed to happen implicitly in the
        //  context of creating the first name server client.
        //
        facCIDOrbUC().Initialize();

        // Try to do the name resolution stuff
        TestNameRes(strmOut);

        //
        //  Initialize the client side of the ORB. This program is run within
        //  the CQC command prompt, so we'll get the name server info from
        //  the environment.
        //
        facCIDOrb().InitClient();

        //
        //  Now try to connect to the CQC servers, which has to be done
        //  after ORB init.
        //
        TestCQCSrvs(strmOut);

        // Clean up the ORB
        facCIDOrb().Terminate();
    }

    // Catch any CIDLib runtime errors
    catch(const TError& errToCatch)
    {
        strmOut << L"A CIDLib runtime error occured during processing.\n"
                << L"Error: " << errToCatch << kCIDLib::NewLn << kCIDLib::EndLn;
        return tCIDLib::EExitCodes::RuntimeError;
    }

    //
    //  Kernel errors should never propogate out of CIDLib, but test for
    //  them here just in case.
    //
    catch(const TKrnlError& kerrToCatch)
    {
        strmOut << L"A kernel error occured during processing.\nError="
                << kerrToCatch.errcId() << kCIDLib::NewLn << kCIDLib::EndLn;
        return tCIDLib::EExitCodes::FatalError;
    }

    // Catch a general exception
    catch(...)
    {
        strmOut << L"A general exception occured during processing"
                << kCIDLib::NewLn << kCIDLib::EndLn;
        return tCIDLib::EExitCodes::SystemException;
    }
    return tCIDLib::EExitCodes::Normal;
}


