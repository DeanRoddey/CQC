//
// FILE NAME: CQCEventDump.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/21/2005
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
//  This guy just initializes the event receiving side of the CQCKit
//  facility, then just waits for any events to come in and dumps them to
//  the console until Ctrl-Break is pressed.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


// ----------------------------------------------------------------------------
//  Includes
// ----------------------------------------------------------------------------
#include    "CQCEventDump.hpp"


// ---------------------------------------------------------------------------
//  Forward references
// ---------------------------------------------------------------------------
tCIDLib::EExitCodes eMainThreadFunc
(
        TThread&            thrThis
    ,   tCIDLib::TVoid*     pData
);



// ---------------------------------------------------------------------------
//  Do the magic main module code
// ---------------------------------------------------------------------------
CIDLib_MainModule(TThread(L"CQCEventDump", eMainThreadFunc))


// ----------------------------------------------------------------------------
//  Functions for local use
// ----------------------------------------------------------------------------

tCIDLib::EExitCodes eMainThreadFunc(TThread& thrThis, tCIDLib::TVoid*)
{
    // We have to let our calling thread go first
    thrThis.Sync();

    TTextOutStream& strmOut = TSysInfo::strmOut();
    TInConsole      conIn;
    try
    {
        strmOut << L"CQC Event Dumper\n"
                << L"Copyright (c) Charmed Quark Systems\n\n"
                << kCIDLib::FlushIt;

        //
        //  Ask CQCKit to load up environment info, which we'll need for
        //  almost anything.
        //
        TString strFailReason;
        if (!facCQCKit().bLoadEnvInfo(strFailReason, kCIDLib::True))
            return tCIDLib::EExitCodes::BadEnvironment;

        // We have to get a username/password
        if (TSysInfo::c4CmdLineParmCount() != 2)
        {
            strmOut << L"Usage:\n"
                    << L"    CQCEventDump username password"
                    << kCIDLib::NewEndLn;
            return tCIDLib::EExitCodes::BadParameters;
        }

        //
        //  Initialize the client side of the ORB. This program is run within
        //  the CQC command prompt, so we'll get the name server info from
        //  the environment.
        //
        facCIDOrb().InitClient();

        // We have to log in
        TCQCSecToken sectUser;
        {
            // Get a reference to the security server
            tCQCKit::TSecuritySrvProxy orbcSS = facCQCKit().orbcSecuritySrvProxy();

            // And ask it to give us a challenge
            TCQCSecChallenge seccLogon;
            tCIDLib::TBoolean bRet = kCIDLib::False;
            TCQCUserAccount uaccToFill;
            if (orbcSS->bLoginReq(TSysInfo::strCmdLineParmAt(0), seccLogon))
            {
                // Lets do a hash of the user's password
                TMD5Hash mhashPW;
                TMessageDigest5 mdigTmp;
                mdigTmp.StartNew();
                mdigTmp.DigestStr(TSysInfo::strCmdLineParmAt(1));
                mdigTmp.Complete(mhashPW);

                // And use that to validate the challenge
                seccLogon.Validate(mhashPW);

                // And send that back to get a security token, assuming its legal
                tCQCKit::ELoginRes eRes;
                bRet = orbcSS->bGetSToken(seccLogon, sectUser, uaccToFill, eRes);
            }

            if (!bRet)
            {
                strmOut  << L"Login failed, please check your user name/password\n"
                        << kCIDLib::EndLn;
            }
        }

        // Start up the receiving side of the event system
        facCQCKit().StartEventProcessing(tCQCKit::EEvProcTypes::Receive, sectUser);

        // Subscribe to CQCKit's event trigger topic, async
        TPubSubAsyncSub psasubEvTrigs(kCIDLib::False);
        psasubEvTrigs.SubscribeTo(kCQCKit::strPubTopic_EvTriggers, kCIDLib::True);

        TPubSubMsg psmsgCur;
        while(kCIDLib::True)
        {
            if (psasubEvTrigs.bGetNextMsg(psmsgCur, 1000))
            {
                const TCQCEvent& cevCur = psmsgCur.objMsgAs<TCQCEvent>();
                strmOut << cevCur << kCIDLib::DNewLn << kCIDLib::FlushIt;
            }

            // Break out on any key
            tCIDLib::EConKeys eKeyType;
            if (conIn.chReadChar(eKeyType, 0))
                break;
        }

        // Stop the event processing
        facCQCKit().StopEventProcessing();

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


