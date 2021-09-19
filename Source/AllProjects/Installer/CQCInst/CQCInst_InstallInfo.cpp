//
// FILE NAME: CQCInst_InstallInfo.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/04/2001
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
//  This file implements the simple class that holds the user entered config
//  info.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCInst.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCInstallInfo,TObject)


// ---------------------------------------------------------------------------
//   CLASS: TCQCInstallInfo
//  PREFIX: info
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCInstallInfo: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCInstallInfo::TCQCInstallInfo() :

    m_bLicAgree(kCIDLib::False)
    , m_bUsePrevOpts(kCIDLib::False)
    , m_eInstStatus(tCQCInst::EInstStatus::Setup)
    , m_eTargetType(tCQCInst::ETargetTypes::Clean)
{
    TPathStr pathTmp;

    //
    //  Set up the source path, which is the directory where the installer is
    //  running.
    //
    m_strSrc = facCQCInst.strPath();

    //
    //  Default the install program path to a CQC directory underneath
    //  the local program files directory. If we can't find that, we just
    //  go with a more obvious default.
    //
    pathTmp = TSysInfo::strSpecialPath(tCIDLib::ESpecialPaths::ProgramFiles);
    pathTmp.AddLevel(L"CQC");
    m_strPath = pathTmp;

    //
    //  We set the source path ourself. Its the Image directory underneath
    //  the installer.
    //
    pathTmp = m_strSrc;
    pathTmp.AddLevel(L"Image");
    m_strSrcImage = pathTmp;

    //
    //  At a minimum, set the current version as the new version, and set
    //  a default install directory.
    //
    m_viiNewInfo.c4MajVersion(kCQCInst::c4MajVersion);
    m_viiNewInfo.c4MinVersion(kCQCInst::c4MinVersion);
    m_viiNewInfo.c4Revision(kCQCInst::c4Revision);
}

TCQCInstallInfo::~TCQCInstallInfo()
{
}


// ---------------------------------------------------------------------------
//  TCQCInstallInfo: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Indicates that SetOldInfo() was called and previous install info is available
tCIDLib::TBoolean TCQCInstallInfo::bFoundOldVer() const
{
    return !m_strOldPath.bIsEmpty();
}


// Set or get whether they agreed to the EULA
tCIDLib::TBoolean TCQCInstallInfo::bLicenseAgree() const
{
    return m_bLicAgree;
}

tCIDLib::TBoolean
TCQCInstallInfo::bLicenseAgree(const tCIDLib::TBoolean bToSet)
{
    m_bLicAgree = bToSet;
    return m_bLicAgree;
}


//
//  Returns whether the current installation info would require the CQCApp Shell
//  service to be installed. Basically, as long as no serversare being installed,
//  we don't need the app shell service.
//
tCIDLib::TBoolean TCQCInstallInfo::bNeedsAppShell() const
{
    return m_viiNewInfo.bAnyServer();
}


// Set or get whether to use previous options
tCIDLib::TBoolean TCQCInstallInfo::bUsePrevOpts() const
{
    return m_bUsePrevOpts;
}

tCIDLib::TBoolean TCQCInstallInfo::bUsePrevOpts(const tCIDLib::TBoolean bToSet)
{
    m_bUsePrevOpts = bToSet;
    return m_bUsePrevOpts;
}


// Get or set the overall install status
tCQCInst::EInstStatus TCQCInstallInfo::eInstStatus() const
{
    return m_eInstStatus;
}

tCQCInst::EInstStatus
TCQCInstallInfo::eInstStatus(const tCQCInst::EInstStatus eToSet)
{
    m_eInstStatus = eToSet;
    return m_eInstStatus;
}


// Get or set the target install type
tCQCInst::ETargetTypes TCQCInstallInfo::eTargetType() const
{
    return m_eTargetType;
}

tCQCInst::ETargetTypes
TCQCInstallInfo::eTargetType(const tCQCInst::ETargetTypes eToSet)
{
    m_eTargetType = eToSet;
    return m_eTargetType;
}


tCIDLib::TVoid TCQCInstallInfo::QuerySrvCreds(TString& strSrvUser, TString& strSrvPassword)
{
    m_strSrvUser = m_strSrvUser;
    m_strSrvPassword = m_strSrvPassword;
}


//
//  Sets the new stuff back to the old again. Note that the other
//  stuff that gets done in SetOldInfo below doesn't need to get done
//  again. This won't change any of that.
//
tCIDLib::TVoid TCQCInstallInfo::RevertToOld()
{
    // We have to override the version to the version we are installing
    m_viiNewInfo = m_viiOldInfo;
    m_viiNewInfo.c4MajVersion(kCQCInst::c4MajVersion);
    m_viiNewInfo.c4MinVersion(kCQCInst::c4MinVersion);
    m_viiNewInfo.c4Revision(kCQCInst::c4Revision);
}


// Sets previous install information and where it was found at
tCIDLib::TVoid
TCQCInstallInfo::SetOldInfo(const   TCQCVerInstallInfo& viiToSet
                            , const TString&            strOldPath)
{
    // Store the old version values
    m_strOldPath = strOldPath;
    m_viiOldInfo = viiToSet;

    // Make sure the path has the right slashes
    m_strOldPath.bReplaceChar(L'/', L'\\');

    //
    //  If the old info indicates that the master server is being installed here,
    //  the compare the host name to this machine. If not, the same, change it.
    //  This handles the case where they move the installation to another machine.
    //  There's no configuration mechanism to change this, so we need to force it.
    //
    if (m_viiOldInfo.bMasterServer())
    {
        if (m_viiOldInfo.strMSAddr() != facCIDSock().strIPHostName())
            m_viiOldInfo.strMSAddr(facCIDSock().strIPHostName());
    }

    //
    //  And init the new info to the old, since this is what we will edit. But
    //  we need to override the version info of the new info to be our current
    //  version that we are installing.
    //
    m_viiNewInfo = m_viiOldInfo;
    m_viiNewInfo.c4MajVersion(kCQCInst::c4MajVersion);
    m_viiNewInfo.c4MinVersion(kCQCInst::c4MinVersion);
    m_viiNewInfo.c4Revision(kCQCInst::c4Revision);

    //
    //  If the old version numbers aren't set, its a clean install, so we
    //  can return
    //
    if (!m_viiOldInfo.c4MajVersion() && !m_viiOldInfo.c4MinVersion())
    {
        m_eTargetType = tCQCInst::ETargetTypes::Clean;
        return;
    }

    //
    //  Check for the psycho scenario, of a new version older than the
    //  current version.
    //
    if (m_viiNewInfo.c8Version() < m_viiOldInfo.c8Version())
    {
        m_eTargetType = tCQCInst::ETargetTypes::RetroVersion;
        return;
    }

    //
    //  If the new maj/min version is greater than the existing version,
    //  then we are upgrading. If the new maj/min version is equal to the
    //  existing version, then we are either doing a straight refresh or
    //  a revision upgrade.
    //
    tCIDLib::TCard8 c8OldVer = m_viiOldInfo.c8Version();
    tCIDLib::TCard8 c8NewVer = m_viiNewInfo.c8Version();

    // If the old version is less than our min upgrade version, we can't do it
    if (c8OldVer < kCQCInst::c8MinUpgradeVer)
    {
        m_eTargetType = tCQCInst::ETargetTypes::BadOldVer;
    }
     else if (c8NewVer == c8NewVer)
    {
        // They are the same so has to be just be a refresh
        m_eTargetType = tCQCInst::ETargetTypes::Refresh;
    }
     else
    {
        // Zero out the revision of both
        c8OldVer &= 0xFFFFFFFF00000000;
        c8NewVer &= 0xFFFFFFFF00000000;

        if (c8OldVer == c8NewVer)
        {
            // If they are equal now, then it's a revision upgrade
            m_eTargetType = tCQCInst::ETargetTypes::Revision;
        }
         else
        {
            // Else it's some sort of major/minor upgrade
            m_eTargetType = tCQCInst::ETargetTypes::Upgrade;
        }
    }

    // If an upgrade or refresh, then default to using previous options
    if ((m_eTargetType == tCQCInst::ETargetTypes::Upgrade)
    ||  (m_eTargetType == tCQCInst::ETargetTypes::Revision)
    ||  (m_eTargetType == tCQCInst::ETargetTypes::Refresh))
    {
        m_bUsePrevOpts = kCIDLib::True;
    }
}


//
//  The mastser server check panel calls this to set the info it got from the MS,
//  which the install thread will grab and put into the registry on non-MS systems.
//
tCIDLib::TVoid
TCQCInstallInfo::SetSrvCreds(const TString& strName, const TString& strPassword)
{
    m_strSrvUser = strName;
    m_strSrvPassword = strPassword;
}


const TString& TCQCInstallInfo::strOldPath() const
{
    return m_strOldPath;
}


const TString& TCQCInstallInfo::strPath() const
{
    return m_strPath;
}

const TString& TCQCInstallInfo::strPath(const TString& strToSet)
{
    //
    //  Store the new path, strip any lead/trailing space and set the
    //  temp path based on that.
    //
    m_strPath = strToSet;
    m_strPath.StripWhitespace();

    // Make sure it has the right slashes
    m_strPath.bReplaceChar(L'/', L'\\');

    TPathStr pathTmp(m_strPath);
    pathTmp.AddLevel(L"TmpNewDir");
    m_strTmpPath = pathTmp;

    return m_strPath;
}


const TString& TCQCInstallInfo::strSrc() const
{
    return m_strSrc;
}


const TString& TCQCInstallInfo::strSrcImage() const
{
    return m_strSrcImage;
}


const TString& TCQCInstallInfo::strTmpPath() const
{
    return m_strTmpPath;
}


const TCQCVerInstallInfo& TCQCInstallInfo::viiOldInfo() const
{
    return m_viiOldInfo;
}


const TCQCVerInstallInfo& TCQCInstallInfo::viiNewInfo() const
{
    return m_viiNewInfo;
}

TCQCVerInstallInfo& TCQCInstallInfo::viiNewInfo()
{
    return m_viiNewInfo;
}



