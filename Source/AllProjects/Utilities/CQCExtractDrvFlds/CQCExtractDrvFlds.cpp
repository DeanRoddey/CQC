//
// FILE NAME: CQCExtractDrvFlds.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/04/2007
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
//  This file just provides the program implementation, which isn't very
//  complex. We just need to get a server admin proxy for the desired
//  driver (the user provides the moniker name) and then we ask for the
//  fields and spit out the formatted XML.
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
#include "CQCExtractDrvFlds.hpp"


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
CIDLib_MainModule(TThread(L"CQCExtractDrvFlds", eMainThreadFunc))


// ---------------------------------------------------------------------------
//  Local types
// ---------------------------------------------------------------------------
enum EFmtTypes
{
    EFmtType_FldDump
    , EFmtType_DevClass
};


// ---------------------------------------------------------------------------
//  File local data
// ---------------------------------------------------------------------------
namespace
{
    tCIDLib::TBoolean bDumpFldValues = kCIDLib::False;
    TInConsole conIn(kCIDLib::True, 20);
    TOutConsole conOut;
    EFmtTypes eOutFmt = EFmtType_FldDump;
    TString strMonList;
    TCQCSecToken sectUser;
}


static tCIDLib::TVoid ShowUsage()
{
    conOut  << L"Usage:\n"
                L"  CQCExtractDrvFlds /User=xxx /Password=yyy\n"
                L"     Allows you to select drivers to dump interactively\n\n"
                L"  CQCExtractDrfFlds /User=xxx /Password=yyy /List=Mon1,Mon2[,...]\n"
                L"     Indicate a list of monikers to dump, non-interactive\n\n"
                L"  In either case /DumpValues will cause the field values to be\n"
                L"  included in the dumps\n\n"
                L"  You must provide a valid CQC user name and password of power \n"
                L"  user or higher\n"
            << kCIDLib::DNewLn << kCIDLib::FlushIt;
}

//
//  Give the moniker of a selected driver, this will dump out the fields
//  to an XML file named after the driver moniker.
//
static tCIDLib::TVoid
DumpFieldList(          tCIDOrbUC::TNSrvProxy&  orbcNS
                , const TString&                strMoniker)
{
    conOut << L"Dumping fields for moniker: " << strMoniker << kCIDLib::EndLn;

    // Get an admin client prox for the CQCServer hosting this moniker
    tCQCKit::TCQCSrvProxy orbcCQCSrv(facCQCKit().orbcCQCSrvAdminProxy(strMoniker));

    // And query all the fields
    tCIDLib::TCard4         c4FldListId;
    tCIDLib::TCard4         c4DriverId;
    tCIDLib::TCard4         c4DriverListId;
    TVector<TCQCFldDef>     colDrvCfgs;
    tCQCKit::EDrvStates     eState;
    tCIDLib::TCard4 c4FldCnt = orbcCQCSrv->c4QueryFields
    (
        strMoniker, eState, colDrvCfgs, c4FldListId, c4DriverId, c4DriverListId
    );


    //
    //  Throw out any that start with a dollar sign. Those are magic ones
    //  and they will be created automatically by all drivers.
    //
    tCIDLib::TCard4 c4Index = 0;
    while (c4Index < c4FldCnt)
    {
        const TCQCFldDef& flddCur = colDrvCfgs[c4Index];
        if (flddCur.strName().chFirst() == kCIDLib::chDollarSign)
        {
            colDrvCfgs.RemoveAt(c4Index);
            c4FldCnt--;
        }
         else
        {
            c4Index++;
        }
    }


    // And get the driver info so we can dump the make/model
    TCQCDriverObjCfg cqcdcDump;
    if (!orbcCQCSrv->bQueryDriverInfo(strMoniker, cqcdcDump))
    {
        conOut  << L"Could not query driver info for driver " << strMoniker
                << L". No output created" << kCIDLib::EndLn;
        return;
    }

    // That worked, so let's dump the info
    TPathStr pathOut(L".\\");
    pathOut.AddLevel(strMoniker);
    pathOut.AppendExt(L".CQCFldDump");

    TTextFileOutStream strmOutput
    (
        pathOut
        , tCIDLib::ECreateActs::CreateAlways
        , tCIDLib::EFilePerms::Default
        , tCIDLib::EFileFlags::SequentialScan
        , tCIDLib::EAccessModes::Excl_Write
        , new TUTF8Converter
    );

    // Based on the format requested, we do different output
    if (eOutFmt == EFmtType_FldDump)
    {
        // Do the preamble and the start of the main element
        strmOutput  << L"<?xml version='1.0' encoding='UTF-8' ?>\n"
                    << L"<!DOCTYPE CQCFldDump PUBLIC 'urn:charmedquark.com:CQC-CQCFldDumpV1.DTD' 'CQCFldDump.DTD'>\n"
                    << L"<CQCFldDump Moniker='" << strMoniker << L"' FldCnt='"
                    << c4FldCnt << L"' Make='" << cqcdcDump.strMake()
                    << L"' Model='" << cqcdcDump.strModel()
                    << L"'>\n";

        TString strValue;
        for (c4Index = 0; c4Index < c4FldCnt; c4Index++)
        {
            const TCQCFldDef& flddCur = colDrvCfgs[c4Index];
            const TString& strFldName = flddCur.strName();

            strmOutput  << L"    <CQCFldDef Name='" << strFldName
                        << L"' Type='" << tCQCKit::strXlatEFldTypes(flddCur.eType())
                        << L"' SType='" << tCQCKit::strXlatEFldSTypes(flddCur.eSemType())
                        << L"' Access='" << tCQCKit::strXlatEFldAccess(flddCur.eAccess())
                        << L"'\n            Private='"
                        << (flddCur.bPrivate() ? L"Yes" : L"No")
                        << L"' AlwaysWrite='" << (flddCur.bAlwaysWrite() ? L"Yes" : L"No")
                        << L"' QueuedWrite='" << (flddCur.bQueuedWrite() ? L"Yes" : L"No")
                        << L"'";

            // If they asked for the field values and this one is readable, get that
            if (bDumpFldValues && tCIDLib::bAllBitsOn(flddCur.eAccess(), tCQCKit::EFldAccess::Read))
            {
                tCIDLib::TCard4 c4SerialNum = 0;
                tCQCKit::EFldTypes eType;
                orbcCQCSrv->bReadFieldByName
                (
                    c4SerialNum, strMoniker, strFldName, strValue, eType
                );

                strmOutput << L" FldValue='";
                facCIDXML().EscapeFor(strValue, strmOutput, tCIDXML::EEscTypes::Attribute, TString::strEmpty());
                strmOutput << L"'";
            }

            strmOutput  << L">\n";

            //
            //  We put the limits info into the element body. It has to be
            //  escaped since it's free form info.
            //
            const TString& strLimits = flddCur.strLimits();
            if (!strLimits.bIsEmpty())
            {
                strmOutput << L"        ";
                facCIDXML().EscapeFor
                (
                    flddCur.strLimits(), strmOutput, tCIDXML::EEscTypes::ElemText, TString::strEmpty()
                );
                strmOutput << kCIDLib::NewLn;
            }

            // And close this one off
            strmOutput  << L"    </CQCFldDef>\n";
        }

        // Wrap up the main element and flush and we are done
        strmOutput  << L"</CQCFldDump>\n\n" << kCIDLib::FlushIt;
    }
     else
    {
        strmOutput << L"    <CQCDevC:Fields>\n";

        // We just want to dump the fields in the device class format
        TString strAccess;
        TString strType;
        for (c4Index = 0; c4Index < c4FldCnt; c4Index++)
        {
            if (c4Index)
                strmOutput << kCIDLib::NewLn;

            const TCQCFldDef& flddCur = colDrvCfgs[c4Index];
            const TString& strFldName = flddCur.strName();

            // Cut the type down to just Boolean, String, etc...
            strType = tCQCKit::strXlatEFldTypes(flddCur.eType());
            strType.Cut(0, 9);

            // Set up an access string of the type we want
            switch(flddCur.eAccess())
            {
                case tCQCKit::EFldAccess::Read :
                strAccess = L"R";
                break;

                case tCQCKit::EFldAccess::Write :
                strAccess = L"W";
                break;

                case tCQCKit::EFldAccess::ReadWrite :
                strAccess = L"RW";
                break;

                default :
                    conOut  << L">>> Bad access type on field " << strFldName
                            << L"\n" << kCIDLib::FlushIt;
                    strType = L"??";
                    break;
            };

            // Put a comment before the element that they can fill in
            strmOutput << L"        <!--  -->\n";

            // Output the start of the field element
            strmOutput  << L"        <CQCDevC:Field CQCDevC:Name=\""
                        << strFldName
                        << L"\"\n                       CQCDevC:Type=\""
                        << strType << L"\" CQCDevC:Access=\""
                        << strAccess << L"\"";

            // If we have a limit, then format that out, correctly escaped

            const TString& strLimits = flddCur.strLimits();
            if (!strLimits.bIsEmpty())
            {
                strmOutput << L"\n                       CQCDevC:Limits=\"";
                facCIDXML().EscapeFor
                (
                    flddCur.strLimits(), strmOutput, tCIDXML::EEscTypes::Attribute, TString::strEmpty()
                );
                strmOutput << L"\"";
            }

            //
            //  Cap it off and put the description stuff inside it. We can't
            //  fill that in here, but leave it there for the user.
            //
            strmOutput << L">\n            <CQCDevC:Descr>\n"
                          L"            </CQCDevC:Descr>\n"
                          L"        </CQCDevC:Field>\n";
        }

        strmOutput << L"    </CQCDevC:Fields>\n";
    }
}


//
//  This will enumerate the available drivers, from the name server, and
//  let the user pick the ones they want. As they pick them, we do the dump.
//
static tCIDLib::TVoid GetDriverSelection(tCIDOrbUC::TNSrvProxy& orbcNS)
{
    //
    //  Get a list of the bindings in the the CQC server admin scope,
    //  which is effectively a list of all the CQC Servers.
    //
    TVector<TNameServerInfo> colCQCSrvs;
    try
    {
        const tCIDLib::TCard4 c4Count = orbcNS->c4EnumObjects
        (
            TCQCSrvAdminClientProxy::strAdminScope
            , colCQCSrvs
            , kCIDLib::False
        );
    }

    catch(const TError& errToCatch)
    {
        conOut << L"An error occured while enumerating the loaded drivers\n"
                  L"Error=\n\n"
               << errToCatch << kCIDLib::DNewLn << kCIDLib::FlushIt;
        return;
    }

    // If there aren't any loaded, complain and give up
    const tCIDLib::TCard4 c4Count = colCQCSrvs.c4ElemCount();
    if (!c4Count)
    {
        conOut  << L"No CQCServers seem to be running at this time\n"
                << kCIDLib::EndLn;
        return;
    }

    //
    //  Collect a list of all of the monikers, nad we need a collection
    //  getting each servers's list of drivers.
    //
    TVector<TString>            colMonikers(64UL);
    TVector<TCQCDriverObjCfg>   colDriverList(64UL);

    // We need some stream formatting so set those up
    TStreamFmt strmfDef(0, 0, tCIDLib::EHJustify::Left, kCIDLib::chSpace);
    TStreamFmt strmfHost(45, 2, tCIDLib::EHJustify::Left, kCIDLib::chSpace);
    TStreamFmt strmfIndex(3, 2, tCIDLib::EHJustify::Right, kCIDLib::chSpace);
    TStreamFmt strmfMoniker(24, 2, tCIDLib::EHJustify::Left, kCIDLib::chSpace);

    // Spit out the header lines
    conOut << strmfIndex << L"#" << TTextOutStream::Spaces(2)
           << strmfMoniker << L"Driver Moniker" << TTextOutStream::Spaces(2)
           << strmfHost << L"On This Host"
           << strmfIndex << kCIDLib::NewLn
           << TTextOutStream::RepChars(kCIDLib::chHyphenMinus, 3)
           << TTextOutStream::Spaces(2)
           << TTextOutStream::RepChars(kCIDLib::chHyphenMinus, 24)
           << TTextOutStream::Spaces(2)
           << TTextOutStream::RepChars(kCIDLib::chHyphenMinus, 45)
           << kCIDLib::NewLn;

    // Now for each CQCServer, query the list of drivers and display them
    tCIDLib::TCard4 c4DriverListId;
    TOrbObjId       ooidTmp;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TNameServerInfo& nsiCur = colCQCSrvs[c4Index];
        const TString& strHostName = nsiCur.strNodeName();

        // Look up the CQCServer for this host name in the name server
        TString strBinding = TCQCSrvAdminClientProxy::strAdminScope;
        strBinding.Append(kCIDLib::pszTreeSepChar);
        strBinding.Append(strHostName);
        if (!facCIDOrbUC().bGetNSObject(orbcNS, strBinding, ooidTmp))
        {
            conOut  << L"Could not access CQCServer: " << strHostName
                    << kCIDLib::EndLn;
            continue;
        }

        // And create an admin proxxy for it
        tCQCKit::TCQCSrvProxy orbcCQCSrv(new TCQCSrvAdminClientProxy(ooidTmp, strBinding));

        // And ask it for the list of driver it has
        const tCIDLib::TCard4 c4DrvCnt = orbcCQCSrv->c4QueryDriverConfigObjs
        (
            colDriverList, c4DriverListId, sectUser
        );

        CIDAssert
        (
            c4DrvCnt == colDriverList.c4ElemCount()
            , L"The returned driver count does not equal the count of elements"
        );

        // Now for each of these, spit out a line
        for (tCIDLib::TCard4 c4DrvInf = 0; c4DrvInf < c4DrvCnt; c4DrvInf++)
        {
            const TCQCDriverObjCfg& cqcdcCur = colDriverList[c4DrvInf];
            const TString& strMoniker = cqcdcCur.strMoniker();

            // Add this one to the overall list of drivers
            colMonikers.objAdd(strMoniker);

            // And dump this out for the user to see
            conOut  << strmfIndex << colMonikers.c4ElemCount()
                    << TTextOutStream::Spaces(2)
                    << strmfMoniker << strMoniker
                    << TTextOutStream::Spaces(2)
                    << strmfHost << strHostName << kCIDLib::NewLn;
        }
    }

    // Set the output console format back to defaults
    conOut << TTextOutStream::strmfDefault();

    // Let them make a choice
    conOut  << kCIDLib::NewLn
            << L"Select a driver number to dump it (x to exit): ";
    conOut.Flush();

    tCIDLib::TBoolean   bRet = kCIDLib::False;
    tCIDLib::TCard4     c4Number = 0;
    TString             strNumber;
    TStringTokenizer    stokParse;

    while (kCIDLib::True)
    {
        conIn >> strNumber;
        conOut << kCIDLib::EndLn;
        strNumber.StripWhitespace();
        strNumber.ToUpper();

        // If they entered an x, then just return false
        if (strNumber == L"X")
            break;

        if (!strNumber.bToCard4(c4Number, tCIDLib::ERadices::Dec))
        {
            conOut << L"Could not convert the value to a number"
                   << kCIDLib::EndLn;
        }
         else if (!c4Number || (c4Number > colMonikers.c4ElemCount()))
        {
            conOut << L"The number must be from 1 to "
                   << colMonikers.c4ElemCount() << kCIDLib::EndLn;
        }
         else
        {
            // Looks ok, so dump this one
            DumpFieldList(orbcNS, colMonikers[c4Number - 1]);
        }

        // Ask them for the next one
        conOut << L"Please enter a number or x: " << kCIDLib::FlushIt;
    }
}


// ----------------------------------------------------------------------------
//  Functions for local use
// ----------------------------------------------------------------------------
tCIDLib::EExitCodes eMainThreadFunc(TThread& thrThis, tCIDLib::TVoid*)
{
    // We have to let our calling thread go first
    thrThis.Sync();

    TTextOutStream& strmOut = TSysInfo::strmOut();
    try
    {
        conOut  << L"\nCQC Driver Field Extractor\n"
                << L"Copyright (c) Charmed Quark Systems\n\n\n"
                << kCIDLib::FlushIt;

        //
        //  Ask CQCKit to load up environment info, which we'll need for
        //  almost anything.
        //
        TString strFailReason;
        if (!facCQCKit().bLoadEnvInfo(strFailReason, kCIDLib::True))
            return tCIDLib::EExitCodes::BadEnvironment;

        //
        //  If they provided us a command line list of monikers, then parse
        //  them out and dump them. Else, let them interactively select them.
        //
        TString strUser;
        TString strPassword;
        TString strParm;
        TSysInfo::TCmdLineCursor cursParms = TSysInfo::cursCmdLineParms();
        for (; cursParms; ++cursParms)
        {
            strParm = *cursParms;

            if (strParm.bStartsWithI(L"/List="))
            {
                strMonList.CopyInSubStr(strParm, 6);
            }
             else if (strParm.bStartsWithI(L"/Password="))
            {
                strPassword.CopyInSubStr(strParm, 10);
            }
             else if (strParm.bStartsWithI(L"/User="))
            {
                strUser.CopyInSubStr(strParm, 6);
            }
             else if (strParm.bCompareI(L"/DumpValues"))
            {
                bDumpFldValues = kCIDLib::True;
            }
             else if (strParm.bCompareI(L"/DevClassFmt"))
            {
                //
                //  They can ask us to dump out the fields in the form needed
                //  by the device class XML format. Otherwise it's done in
                //  the field dump format.
                //
                eOutFmt = EFmtType_DevClass;
            }
             else
            {
                ShowUsage();
                return tCIDLib::EExitCodes::Normal;
            }
        }

        if (strUser.bIsEmpty() || strPassword.bIsEmpty())
        {
            ShowUsage();
            return tCIDLib::EExitCodes::Normal;
        }

        //
        //  Initialize the client side of the ORB. This program is run within
        //  the CQC command prompt, so we'll get the name server info from
        //  the environment.
        //
        facCIDOrb().InitClient();

        // See if we can get to the name server
        tCIDOrbUC::TNSrvProxy orbcNS;
        try
        {
            TOrbObjId ooidTmp;
            conOut << L"Trying to connect to CQC name server..." << kCIDLib::EndLn;
            orbcNS = facCIDOrbUC().orbcNameSrvProxy(4000);
            conOut << L"Connected to name server successfully\n" << kCIDLib::EndLn;
        }

        catch(const TError& errToCatch)
        {
            conOut  << L"Could not contact name server!\nError=\n\n"
                    << errToCatch << kCIDLib::DNewLn << kCIDLib::FlushIt;
            return tCIDLib::EExitCodes::Normal;
        }

        // Now we need to get a security server proxy and try to log in
        try
        {
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
                conOut  << L"Login failed, please check your user name/password\n"
                        << kCIDLib::EndLn;
            }
        }

        catch(const TError& errToCatch)
        {
            conOut  << L"Could not log into CQC!\nError=\n\n"
                    << errToCatch << kCIDLib::DNewLn << kCIDLib::FlushIt;
            return tCIDLib::EExitCodes::Normal;
        }

        // If we got a moniker list, do that, else go interactive
        if (!strMonList.bIsEmpty())
        {
            // Now start pulling out monikers and dumping
            TStringTokenizer stokList(&strMonList, L",");
            TString strMoniker;
            while (stokList.bGetNextToken(strMoniker))
                DumpFieldList(orbcNS, strMoniker);
        }
         else
        {
            GetDriverSelection(orbcNS);
        }

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


