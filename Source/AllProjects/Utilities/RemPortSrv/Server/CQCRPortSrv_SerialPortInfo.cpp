//
// FILE NAME: CQCRPortSrv_SerialPortInfo.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/14/2005
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
//  This is the header for the CQCRPortSrv_SerialPortInfo.cpp file, which
//  implements a class that we use to track each of the local ports that
//  could possibly serve up.
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
#include    "CQCRPortSrv.hpp"



// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCSerPortInfo,TObject)


// ---------------------------------------------------------------------------
//   CLASS: TCQCSerPortInfo
//  PREFIX: spi
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCSerPortInfo: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCSerPortInfo::TCQCSerPortInfo(const TString& strLocalPath) :

    m_bAllow(kCIDLib::False)
    , m_c4Cookie(0)
    , m_enctLast(0)
    , m_pcommPort(0)
    , m_strLocalPath(strLocalPath)
    , m_strRemotePath()
{
    // Double check it starts with the local port path
    if (!strLocalPath.bStartsWithI(TFacCIDComm::strLocalPortPrefix))
    {

    }

    // Pull out the numerical indicator, and build up the remote version
    strLocalPath.CopyOutSubStr(m_strRemotePath, TFacCIDComm::strLocalPortPrefix.c4Length());
    m_strRemotePath.Prepend(TRemSerialSrvServerBase::strSerialPathPrefix);

    // Create the port object
    m_pcommPort = facCIDComm().pcommMakeNew(strLocalPath);

    //
    //  Generate a cookie for us. It is just our own address, which is
    //  guaranteed to be unique and which isn't likely to be a value that
    //  might be accidentally passed by the client.
    //
    m_c4Cookie = tCIDLib::TCard4(this);
}


TCQCSerPortInfo::~TCQCSerPortInfo()
{
    // Clean up the port if not already
    if (m_pcommPort)
    {
        try
        {
            delete m_pcommPort;
            m_pcommPort = 0;
        }

        catch(const TError& errToCatch)
        {
            if (facCQCRPortSrv().bShouldLog(errToCatch))
                TModule::LogEventObj(errToCatch);
        }
    }
}


// ---------------------------------------------------------------------------
//  TCQCSerPortInfo: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TCQCSerPortInfo::bAllow() const
{
    return m_bAllow;
}

tCIDLib::TBoolean TCQCSerPortInfo::bAllow(const tCIDLib::TBoolean bToSet)
{
    m_bAllow = bToSet;
    return m_bAllow;
}


tCIDLib::TCard4 TCQCSerPortInfo::c4Cookie() const
{
    return m_c4Cookie;
}

tCIDLib::TCard4 TCQCSerPortInfo::c4Cookie(const tCIDLib::TCard4 c4ToSet)
{
    m_c4Cookie = c4ToSet;
    return m_c4Cookie;
}


tCIDLib::TEncodedTime TCQCSerPortInfo::enctLast() const
{
    return m_enctLast;
}

tCIDLib::TEncodedTime
TCQCSerPortInfo::enctLast(const tCIDLib::TEncodedTime enctToSet)
{
    m_enctLast = enctToSet;
    return m_enctLast;
}


TCommPortBase* TCQCSerPortInfo::pcommPort() const
{
    return m_pcommPort;
}

TCommPortBase* TCQCSerPortInfo::pcommPort()
{
    return m_pcommPort;
}


const TString& TCQCSerPortInfo::strLocalPath() const
{
    return m_strLocalPath;
}


const TString& TCQCSerPortInfo::strRemotePath() const
{
    return m_strRemotePath;
}


