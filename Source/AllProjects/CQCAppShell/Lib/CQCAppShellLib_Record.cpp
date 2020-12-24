//
// FILE NAME: CQCAppShellLib_Record.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/01/2001
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
//  This file implements our app shell record derivative
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCAppShellLib_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCAppShRecord,TAppShRecord)


// ---------------------------------------------------------------------------
//   CLASS: TCQCAppShRecord
//  PREFIX: asr
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCAppShRecord: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCAppShRecord::TCQCAppShRecord(const  TString&                strTitle
                                , const TString&                strAppName
                                , const TString&                strAppPath
                                , const TString&                strParms
                                , const TString&                strAdminPath
                                , const tCIDAppSh::EAppLevels   eLevel) :

    TAppShRecord(strTitle, strAppPath, strParms, eLevel)
    , m_c4LastCycleTime(0)
    , m_c4LastLogTime(0)
    , m_c4LastULogTime(0)
    , m_eStopStatus(EStopStatus::None)
    , m_rsnWait
      (
        kCIDLib::pszResCompany
        , strAppName
        , kCIDOrbUC::pszEvResource
      )
    , m_strAdminPath(strAdminPath)
{
    // Do token replacement on the admin path string
    ReplaceTokens(m_strAdminPath);
}

TCQCAppShRecord::~TCQCAppShRecord()
{
}


// ---------------------------------------------------------------------------
//  TCQCAppShRecord: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TCard4 TCQCAppShRecord::c4LastCycleTime() const
{
    return m_c4LastCycleTime;
}

tCIDLib::TCard4 TCQCAppShRecord::c4LastCycleTime(const tCIDLib::TCard4 c4ToSet)
{
    m_c4LastCycleTime = c4ToSet;
    return m_c4LastCycleTime;
}


tCIDLib::TCard4 TCQCAppShRecord::c4LastLogTime() const
{
    return m_c4LastLogTime;
}

tCIDLib::TCard4 TCQCAppShRecord::c4LastLogTime(const tCIDLib::TCard4 c4ToSet)
{
    m_c4LastLogTime = c4ToSet;
    return m_c4LastLogTime;
}


tCIDLib::TCard4 TCQCAppShRecord::c4LastULogTime() const
{
    return m_c4LastULogTime;
}

tCIDLib::TCard4 TCQCAppShRecord::c4LastULogTime(const tCIDLib::TCard4 c4ToSet)
{
    m_c4LastULogTime = c4ToSet;
    return m_c4LastULogTime;
}


TCQCAppShRecord::EStopStatus TCQCAppShRecord::eStopStatus() const
{
    return m_eStopStatus;
}

TCQCAppShRecord::EStopStatus
TCQCAppShRecord::eStopStatus(const TCQCAppShRecord::EStopStatus eToSet)
{
    m_eStopStatus = eToSet;
    return m_eStopStatus;
}


const TError& TCQCAppShRecord::errLast() const
{
    return m_errLast;
}

const TError& TCQCAppShRecord::errLast(const TError& errToSet)
{
    m_errLast = errToSet;
    return m_errLast;
}


const TResourceName& TCQCAppShRecord::rsnWait() const
{
    return m_rsnWait;
}


const TString& TCQCAppShRecord::strAdminPath() const
{
    return m_strAdminPath;
}


