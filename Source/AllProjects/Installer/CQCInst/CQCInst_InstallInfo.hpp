//
// FILE NAME: CQCInst_InstallInfo.hpp
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
//  This file defines a simple class that holds the installation information
//  provided by the user. It is then passed into the code that actually does
//  the installation work.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TCQCInstallInfo
//  PREFIX: info
// ---------------------------------------------------------------------------
class TCQCInstallInfo : public TObject
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCInstallInfo();

        TCQCInstallInfo(const TCQCInstallInfo&) = delete;
        TCQCInstallInfo(TCQCInstallInfo&&) = delete;

        ~TCQCInstallInfo();


        // -------------------------------------------------------------------
        //  Public oeprators
        // -------------------------------------------------------------------
        TCQCInstallInfo& operator=(const TCQCInstallInfo&) = delete;
        TCQCInstallInfo& operator=(TCQCInstallInfo&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bFoundOldVer() const;

        tCIDLib::TBoolean bLicenseAgree() const;

        tCIDLib::TBoolean bLicenseAgree
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TBoolean bNeedsAppShell() const;

        tCIDLib::TBoolean bUsePrevOpts() const;

        tCIDLib::TBoolean bUsePrevOpts
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCQCInst::EInstStatus eInstStatus() const;

        tCQCInst::EInstStatus eInstStatus
        (
            const   tCQCInst::EInstStatus   eToSet
        );

        tCQCInst::ETargetTypes eTargetType() const;

        tCQCInst::ETargetTypes eTargetType
        (
            const   tCQCInst::ETargetTypes  eToSet
        );

        tCIDLib::TVoid QuerySrvCreds
        (
                    TString&                strSrvUser
            ,       TString&                strSrvPassword
        );

        tCIDLib::TVoid RevertToOld();

        const TString& strBackupPath() const;

        const TString& strOldPath() const;

        const TString& strPath() const;

        const TString& strPath
        (
            const   TString&                strToSet
        );

        const TString& strSrc() const;

        const TString& strSrcImage() const;

        const TString& strSrvUser() const
        {
            return m_strSrvUser;
        }

        const TString& strSrvPassword() const
        {
            return m_strSrvPassword;
        }

        const TString& strTmpPath() const;

        tCIDLib::TVoid SetOldInfo
        (
            const   TCQCVerInstallInfo&     viiToSet
            , const TString&                strOldPath
        );

        tCIDLib::TVoid SetSrvCreds
        (
            const   TString&                strUser
            , const TString&                strPassword
        );

        const TCQCVerInstallInfo& viiOldInfo() const;

        const TCQCVerInstallInfo& viiNewInfo() const;

        TCQCVerInstallInfo& viiNewInfo();


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bLicAgree
        //      Set to the agree/disagree button the user checked in the
        //      license agreement.
        //
        //  m_bUsePrevOpts
        //      On one of the first panels, where they select what components to
        //      install, if this is an upgrade, they can check a check box to
        //      say just use previous options. That will set this flag here which
        //      will cause most panels not to be displayed.
        //
        //  m_eInstStatus
        //      The install status, which is set by the install status panel
        //      as the install ends good or bad, so that downstream panels
        //      can know what happened.
        //
        //  m_eTargetType
        //      Indicates whether we found a previous version, i.e. we are
        //      doing an upgrade, else we are doing a fresh install. It will
        //      be a sub-directory under the actual target path, and named
        //      "NewInst".
        //
        //  m_strBackupPath
        //      We offer to backup the CQC system for the user. So that they
        //      don't have to select the path every time, we remember the
        //      last one provided and make that the default selection.
        //
        //  m_strOldPath
        //      If we found a previous version, this is where we found it.
        //
        //  m_strPath
        //      The target path to install into. This is gotten from the user
        //      but defaults to C:/CQC initially.
        //
        //  m_strSrc
        //      The source path, which is set to the executable directory of
        //      the installer.
        //
        //  m_strSrcImage
        //      THe source path plus the Image subdirectory.
        //
        //  m_strSrvUser
        //  m_strSrvPassword
        //      If this is not the MS, the check master server panel will make sure
        //      the user provides admin credentials and will use this to ask the MS
        //      for the secondary server username/password, which we need to store
        //      for the installer thread to use.
        //
        //  m_strTmpPath
        //      This is the temp path that we are installing into during
        //      the install. We get the whole install done and then rename
        //      the directory at the end. The m_strPath member holds the
        //      actual path that we will eventually get into.
        //
        //  m_viiOldInfo
        //  m_viiNewInfo
        //      These are the old and new installation info objects. We get
        //      the old info by reading in the info from an object streamed
        //      to a file in the installation directory.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bLicAgree;
        tCIDLib::TBoolean       m_bUsePrevOpts;
        tCQCInst::EInstStatus   m_eInstStatus;
        tCQCInst::ETargetTypes  m_eTargetType;
        TString                 m_strBackupPath;
        TString                 m_strOldPath;
        TString                 m_strPath;
        TString                 m_strSrc;
        TString                 m_strSrcImage;
        TString                 m_strSrvUser;
        TString                 m_strSrvPassword;
        TString                 m_strTmpPath;
        TCQCVerInstallInfo      m_viiOldInfo;
        TCQCVerInstallInfo      m_viiNewInfo;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCInstallInfo,TObject)
};

#pragma CIDLIB_POPPACK



