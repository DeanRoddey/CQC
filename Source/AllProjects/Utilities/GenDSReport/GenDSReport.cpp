//
// FILE NAME: GenDSReport.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 2/22/2019
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
//  This is a little utility program that can generate some report info on files
//  in the data server.
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
#include "CIDLib.hpp"
#include "CIDPNG.hpp"
#include "CQCRemBrws.hpp"



// ---------------------------------------------------------------------------
//  Forward references
// ---------------------------------------------------------------------------
tCIDLib::EExitCodes eMainThreadFunc(TThread&, tCIDLib::TVoid*);
TFacility facThis
(
    L"GenDSReport"
    , tCIDLib::EModTypes::Exe
    , 1
    , 0
    , 0
    , tCIDLib::EModFlags::None
);
TTextOutStream& strmOut = TSysInfo::strmOut();
TInConsole  conIn(kCIDLib::True, 32);


// ---------------------------------------------------------------------------
//  Do the magic main module code
// ---------------------------------------------------------------------------
CIDLib_MainModule(TThread(L"GenDSReportMainThread", eMainThreadFunc))


// ---------------------------------------------------------------------------
//  Local helpers
// ---------------------------------------------------------------------------


//
//  A generic debugger that queries meta info on a given file and display it so
//  that we can manually compare to expectations.
//
static tCIDLib::TVoid DumpMetaInfo(         TDataSrvClient& dsclTest
                                    , const TString&        strHPath
                                    , const TCQCSecToken&   sectUser)
{
    // Convert the HPath to get the relative path and type
    TString strRelPath;
    const tCQCRemBrws::EDTypes eType = facCQCRemBrws().eConvertPath
    (
        strHPath, kCIDLib::False, strRelPath, kCIDLib::False
    );

    tCIDLib::TCard4         c4SerialNum;
    tCIDLib::TKVPFList      colMetaVals;
    tCIDLib::TStrList       colChunkIds;
    tCIDLib::TEncodedTime   enctLastChange;
    tCIDLib::TCardList      fcolChunkSizes;
    tCIDLib::TBoolean bRes = dsclTest.bQueryItemMetaData
    (
        strRelPath
        , eType
        , colMetaVals
        , colChunkIds
        , fcolChunkSizes
        , c4SerialNum
        , enctLastChange
        , kCIDLib::True
        , sectUser
    );

    if (!bRes)
    {
        strmOut  << L"No meta data returned for: " << strRelPath
                << kCIDLib::NewEndLn;
        return;
    }

    if (colChunkIds.c4ElemCount() != fcolChunkSizes.c4ElemCount())
    {
        strmOut  << L"The chunk and chunk size list sizes differed ("
                << colChunkIds.c4ElemCount() << L" vs "
                << fcolChunkSizes.c4ElemCount()
                << kCIDLib::NewEndLn;
    }


    // Set up the last change for appropriate formatting
    TTime::UTCToLocal(enctLastChange, enctLastChange);
    TTime tmLast(enctLastChange);
    tmLast.strDefaultFormat(TTime::strCTime());

    strmOut  << L"\n----- General ----- \n"
            << L"   Serial Num=" << c4SerialNum << kCIDLib::NewLn
            << L"  Last Change=" << tmLast << kCIDLib::NewLn

            << L"\n----- Meta Values ----- \n";
    tCIDLib::TCard4 c4Count = colMetaVals.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TKeyValuePair& kvalCur = colMetaVals[c4Index];
        strmOut  << L"    " << kvalCur.strKey()
                << L"=" << kvalCur.strValue() << kCIDLib::NewLn;
    }
    strmOut << L"\n------- Chunks -------- \n";
    c4Count = colChunkIds.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        strmOut  << L"    " << colChunkIds[c4Index] << L"="
                << fcolChunkSizes[c4Index] << L" bytes"
                << kCIDLib::NewLn;
    }
    strmOut << kCIDLib::NewEndLn;
}



// ---------------------------------------------------------------------------
//  Entry point
// ---------------------------------------------------------------------------
tCIDLib::EExitCodes eMainThreadFunc(TThread& thrThis, tCIDLib::TVoid*)
{
    // We have to let our calling thread go first
    thrThis.Sync();


    try
    {
        //
        //  Ask CQCKit to load up environment info, which we'll need for
        //  almost anything.
        //
        TString strFailReason;
        if (!facCQCKit().bLoadEnvInfo(strFailReason, kCIDLib::True))
        {
            strmOut << strFailReason << kCIDLib::NewEndLn;
            return tCIDLib::EExitCodes::BadEnvironment;
        }


        if (TSysInfo::c4CmdLineParmCount() < 4)
        {
            strmOut << L"Usage: \n"
                      L"    GenDSReport [username] [password] [cmd] [parameters] \n\n"
                      L"        Commands:\n"
                      L"        ---------------------\n"
                      L"        DumpMeta [path]\n"
                      L"        FullReport\n"
                      L"        Report [path]\n\n"

                      L"        Examples:\n"
                      L"        ---------------------\n"
                      L"        GenDSReport Admin MyPW FullReport\n"
                      L"        GenDSReport Admin MyPW Report /Interfaces/User/MyTmpls\n"
                      L"        GenDSReport Admin MyPW DumpMeta /Interfaces/User/MyTmpls/MainBgn\n"
                   << kCIDLib::EndLn;
            return tCIDLib::EExitCodes::Normal;
        }

        // Get the required parameters
        TString strUserName = TSysInfo::strCmdLineParmAt(0);
        TString strPassword = TSysInfo::strCmdLineParmAt(1);
        TString strCmd = TSysInfo::strCmdLineParmAt(2);

        //
        //  Initialize the client side of the ORB. This program is run within
        //  the CQC command prompt, so we'll get the name server info from
        //  the environment.
        //
        facCIDOrb().InitClient();

        // Try to log in using the provided info
        TCQCSecToken        sectUser;
        TCQCUserAccount     uaccUser;
        tCQCKit::ELoginRes  eLoginRes = tCQCKit::ELoginRes::Ok;
        tCQCKit::TSecuritySrvProxy orbcSS = facCQCKit().orbcSecuritySrvProxy();
        TCQCSecChallenge seccLogon;
        if (orbcSS->bLoginReq(strUserName, seccLogon))
        {
            // Lets do a hash of the user's password
            TMD5Hash mhashPW;
            TMessageDigest5 mdigTmp;
            mdigTmp.StartNew();
            mdigTmp.DigestStr(strPassword);
            mdigTmp.Complete(mhashPW);

            // And use that to validate the challenge
            seccLogon.Validate(mhashPW);

            // And send that back to get a security token, assuming its legal
            orbcSS->bGetSToken(seccLogon, sectUser, uaccUser, eLoginRes);
        }
         else
        {
            eLoginRes = tCQCKit::ELoginRes::BadCred;
        }

        if (eLoginRes == tCQCKit::ELoginRes::BadCred)
        {
            strmOut << L"The provided username/password were rejected"
                    << kCIDLib::NewEndLn;
            return tCIDLib::EExitCodes::Normal;
        }

        if (strCmd.bCompareI(L"DumpMeta"))
        {
            TDataSrvClient  dsclTest;
            TString strHPath = TSysInfo::strCmdLineParmAt(3);
            strHPath.Prepend(L"/Customize");

            DumpMetaInfo(dsclTest, strHPath, sectUser);
        }
         else if (strCmd.bCompareI(L"FullReport"))
        {
            TDataSrvClient  dsclTest;
            dsclTest.GenerateFullReport(strmOut, L"CQC Full Report", sectUser);
        }
         else if (strCmd.bCompareI(L"Report"))
        {
            TDataSrvClient  dsclTest;
            TString strHPath = TSysInfo::strCmdLineParmAt(3);
            strHPath.Prepend(L"/Customize");

            TString strRelPath;
            const tCQCRemBrws::EDTypes eType = facCQCRemBrws().eConvertPath
            (
                strHPath, kCIDLib::False, strRelPath, kCIDLib::False
            );
            dsclTest.GenerateReport
            (
                strmOut, L"CQC Report", strRelPath, eType, kCIDLib::True, sectUser
             );
        }
    }

    // Catch any CIDLib runtime errors
    catch(TError& errToCatch)
    {
        strmOut  <<  L"A CIDLib runtime error occured during processing.\n"
                <<  L"Error: \n" << errToCatch << kCIDLib::NewEndLn;
        return tCIDLib::EExitCodes::RuntimeError;
    }

    //
    //  Kernel errors should never propogate out of CIDLib, but I test
    //  for them in my demo programs so I can catch them if they do
    //  and fix them.
    //
    catch(const TKrnlError& kerrToCatch)
    {
        strmOut  << L"A general exception occured during processing"
                << kCIDLib::NewEndLn;
        return tCIDLib::EExitCodes::FatalError;
    }

    // Catch a general exception
    catch(...)
    {
        strmOut  << L"A general exception occured during processing"
                << kCIDLib::NewEndLn;
        return tCIDLib::EExitCodes::SystemException;
    }

    return tCIDLib::EExitCodes::Normal;
}


