//
// FILE NAME: JAPwrSer_ThisFacility.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/12/2013
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
//  This is the implementation file for the facility class.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "JAPwrSer_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TFacJAPwrSer,TFacility)



// ---------------------------------------------------------------------------
//  Local const data
// ---------------------------------------------------------------------------
namespace JAPwrSer_ThisFacility
{
}


// ---------------------------------------------------------------------------
//   CLASS: TFacJAPwrSer
//  PREFIX: fac
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TFacJAPwrSer: Constructors and Destructor
// ---------------------------------------------------------------------------
TFacJAPwrSer::TFacJAPwrSer() :

    TFacility
    (
        L"JAPwrSer"
        , tCIDLib::EModTypes::SharedLib
        , kCQCKit::c4MajVersion
        , kCQCKit::c4MinVersion
        , kCQCKit::c4Revision
        , tCIDLib::EModFlags::HasMsgFile
    )
{
}

TFacJAPwrSer::~TFacJAPwrSer()
{
}


// ---------------------------------------------------------------------------
//  TFacJAPwrSer: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Parse one of our port paths and return the device name and the port number
//  (1, 2, 3) that we find in the path. Return true if syntacically valid,
//  else false.
//
//  The format is like this:
//
//      /JAPwr/xxx/COMn
//
tCIDLib::TBoolean
TFacJAPwrSer::bParsePath(const  TString&            strPath
                        ,       TString&            strName
                        ,       tCIDLib::TCard4&    c4PortNum)
{
    // Set up a string tokenizer so we can pull out the tokens
    TStringTokenizer stokPath(&strPath, L"/");

    TString strPrefix;
    TString strCOM;

    if (!stokPath.bGetNextToken(strPrefix)
    ||  !stokPath.bGetNextToken(strName)
    ||  !stokPath.bGetNextToken(strCOM))
    {
        return kCIDLib::False;
    }

    // Check some obvious stuff
    if (!strPrefix.bCompareI(L"JAPwr") || !strCOM.bStartsWithI(L"COM"))
        return kCIDLib::False;

    // Get the port number
    strCOM.Cut(0, 3);

    if (!strCOM.bToCard4(c4PortNum, tCIDLib::ERadices::Dec))
        return kCIDLib::False;

    //
    //  Make sure it's something valid, keeping in mind it is 1 based
    //
    if (c4PortNum > kJAPwrSer::c4MaxPortsPer)
        return kCIDLib::False;

    return kCIDLib::True;
}


//
//  The containing application can periodically call this to see if new
//  info is available and to update the factory if new data.
//
tCIDLib::TBoolean
TFacJAPwrSer::bRefreshFactory(          TJAPwrCfgList&      japlToFill
                                ,       tCIDLib::TCard4&    c4SerialNum
                                , const TCQCSecToken&       sectUser)
{
    // Do have to lock here, we are just reading into the caller's object
    tCIDLib::ELoadRes eRes = tCIDLib::ELoadRes::NotFound;
    tCIDLib::TCard4 c4SerNum = c4SerialNum;
    try
    {
        eRes = eQueryConfig(japlToFill, c4SerNum, sectUser);
    }

    catch(TError& errToCatch)
    {
        if (bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            LogEventObj(errToCatch);
        }
        return kCIDLib::False;
    }

    //
    //  Only update factory and return true if we got new data. He will lock when
    //  he updates the data.
    //
    if (eRes == tCIDLib::ELoadRes::NewData)
    {
        c4SerialNum = c4SerNum;
        bUpdateFactory(japlToFill);
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


//
//  A helper to register our factory with CIDComm, if not already registered.
//
//  We have one that queries the config for the caller and just registers the factory
//  with that info by calling the other version which just takes new info to register with.
//  The containing program can call the second version directly if it already has config
//  to set.
//
//  This guy is thread safe.
//
tCIDLib::TBoolean TFacJAPwrSer::bRegisterFactory(const TCQCSecToken& sectUser)
{
    // Create discardable locals and call the other version
    tCIDLib::TCard4 c4SerialNum = 0;
    TJAPwrCfgList japlToSet;

    // If we can't get the config, then don't do anything. They can try again later
    tCIDLib::ELoadRes eRes;
    try
    {
        eRes = eQueryConfig(japlToSet, c4SerialNum, sectUser);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);
        return kCIDLib::False;
    }

    // If we didn't get any config data, then it just doesn't exist
    if (eRes != tCIDLib::ELoadRes::NewData)
        return kCIDLib::False;

    // And now call theo ther version.
    return bRegisterFactory(japlToSet);
}

tCIDLib::TBoolean
TFacJAPwrSer::bRegisterFactory(const TJAPwrCfgList& japlToSet)
{
    try
    {
        // Lock the CIDComm facility while we do this
        TLocker lockrSync(&facCIDComm());

        // If not already registered, then register it
        if (!facCIDComm().pfactById(kJAPwrSer::pszFactoryId))
            facCIDComm().RegisterFactory(new TJAPwrPortFactory(japlToSet));
    }

    catch(TError& errToCatch)
    {
        if (bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            LogEventObj(errToCatch);
        }
        return kCIDLib::False;
    }
    return kCIDLib::True;
}



//
//  A helper to store the configuration to the MS config server. If successful,
//  we return the new serial number for the configuration data.
//
tCIDLib::TBoolean
TFacJAPwrSer::bSaveConfig(  const   TJAPwrCfgList&      japlNew
                            ,       tCIDLib::TCard4&    c4SerialNum
                            , const TCQCSecToken&       sectUser)
{
    try
    {
        THeapBuf mbufData(4096UL);
        TBinMBufOutStream strmOut(&mbufData);
        strmOut << japlNew << kCIDLib::FlushIt;

        tCQCKit::TInstSrvProxy orbcInst = facCQCKit().orbcInstSrvProxy();
        orbcInst->bAddOrUpdateJAPCfg
        (
            strmOut.c4CurPos(), mbufData, c4SerialNum, sectUser
        );

        //
        //  It worked. Go ahead and update the factory as well. This guy
        //  will lock while it does it's work.
        //
        bUpdateFactory(japlNew);
    }

    catch(TError& errToCatch)
    {
        if (bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            LogEventObj(errToCatch);
        }
        return kCIDLib::False;
    }
    return kCIDLib::True;
}



//
//  A helper method to update the registered com port factory, if it is in fact registered.
//  We have one that will get the config from the server and then call the other version
//  that takes a config object. The second can also be called by the containing program to
//  set some config it already has.
//
tCIDLib::TBoolean TFacJAPwrSer::bUpdateFactory(const TCQCSecToken& sectUser)
{
    //
    //  Try to read the config. Regardless we all the other version. If we got no data then
    //  it could have been deleted so we want to set the empty content now. If it outright
    //  fails it will throw and we won't set anything, we'll just return false.
    //
    tCIDLib::TCard4 c4SerialNum = 0;
    TJAPwrCfgList japlToSet;
    try
    {
        eQueryConfig(japlToSet, c4SerialNum, sectUser);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);
        return kCIDLib::False;
    }

    return bUpdateFactory(japlToSet);
}


// This guy is thread safe
tCIDLib::TBoolean TFacJAPwrSer::bUpdateFactory(const TJAPwrCfgList& japlNew)
{
    // Now update the factory if registered. If not register it with this new config
    try
    {
        // Lock the CIDComm facility while we do this
        TLocker lockrSync(&facCIDComm());

        // See if the factory is registered
        TComPortFactory* pfactUpd = facCIDComm().pfactById(kJAPwrSer::pszFactoryId);
        if (pfactUpd)
        {
            // It's there, so cast to the right type and update it
            TJAPwrPortFactory* pfactJAP = static_cast<TJAPwrPortFactory*>(pfactUpd);
            pfactJAP->UpdateConfig(japlNew);
        }
         else
        {
            // Not found, so register one with this config
            facCIDComm().RegisterFactory(new TJAPwrPortFactory(japlNew));
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        LogEventObj(errToCatch);
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


//
//  Query the JAPconfiguration from the MS config server. We return true only if
//  we get new configuration.
//
//  Don't need any locking here. We are just reading into the caller's buffer.
//
//  The caller should either set serial number to zero, or the last serial number it got.
//
tCIDLib::ELoadRes
TFacJAPwrSer::eQueryConfig(         TJAPwrCfgList&      japlFill
                            ,       tCIDLib::TCard4&    c4SerialNum
                            , const TCQCSecToken&       sectUser)
{
    // Assume the worst
    tCIDLib::ELoadRes eRes = tCIDLib::ELoadRes::NotFound;
    try
    {
        tCQCKit::TInstSrvProxy orbcInst = facCQCKit().orbcInstSrvProxy();

        tCIDLib::TCard4 c4Size = 0;
        THeapBuf mbufData(4096);
        eRes = orbcInst->eQueryJAPCfg(c4SerialNum, c4Size, mbufData, sectUser);
        if (eRes == tCIDLib::ELoadRes::NewData)
        {
            TBinMBufInStream strmSrc(&mbufData, c4Size);
            strmSrc >> japlFill;
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
    return eRes;
}
