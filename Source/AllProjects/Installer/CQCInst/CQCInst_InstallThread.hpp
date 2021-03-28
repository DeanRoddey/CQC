//
// FILE NAME: CQCInst_InstallThread.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/02/2004
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
//  This is the header for the thread class that does the actual installation
//  operation. We get a pointer to the main window, to whom we post status
//  messages, and we get a copy of the install info that use to drive the
//  installation process.
//
//  We have more than one implementation file, since there is a fair amount
//  of code involved in this process.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

class TInstStatusPanel;


// ---------------------------------------------------------------------------
//   CLASS: TInstallThread
//  PREFIX: thr
// ---------------------------------------------------------------------------
class TInstallThread : public TThread
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TInstallThread();

        TInstallThread(const TInstallThread&) = delete;
        TInstallThread(TInstallThread&&) = delete;

        ~TInstallThread();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TInstallThread& operator=(const TInstallThread&) = delete;
        TInstallThread& operator=(TInstallThread&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCQCInst::EInstSteps eStep() const;

        tCIDLib::TVoid StartInstall
        (
                    TMainFrameWnd* const    pwndOwner
            ,       TInstStatusPanel* const ppanStatus
            , const TCQCInstallInfo* const  pinfoToUse
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::EExitCodes eProcess() final;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bRemoveInstPath
        (
            const   TString&                strToRemove
        );

        tCIDLib::TBoolean bRenameInstPath
        (
            const   TString&                strSrc
            , const TString&                strTar
        );

        tCIDLib::TBoolean bTestDirLock
        (
            const   TString&                strSubDir
        );

        tCIDLib::TVoid CopyLangRT();

        tCIDLib::TVoid CopyImageDir
        (
            const   TString&                strSrcDir
            , const TString&                strDestDir
        );

        tCIDLib::TVoid DupRelPath
        (
            const   TString&                strSrc
            , const TString&                strTarget
            , const tCIDLib::TBoolean       bRecurse
        );

        tCIDLib::TVoid DupChildPaths
        (
            const   TString&                strSrc
            , const TString&                strTarget
            , const TCollection<TString>&   colChildren
            , const tCIDLib::TBoolean       bRecurse
        );

        tCIDLib::TVoid GenCmdFiles();

        tCIDLib::TVoid GenConfig();

        tCIDLib::TVoid GenLinks();

        tCIDKernel::TWSvcHandle hsvcInstallAppShell
        (
                    tCIDLib::TStrList&      colParms
        );

        tCIDLib::TVoid InstallClService
        (
                    tCIDLib::TStrList&      colParms
        );

        tCIDLib::TVoid MakeDirectories();

        tCIDLib::TVoid MakeDirSet
        (
            const   TString&                strParent
            , const tCQCInst::TInstDirInfo* pdiriToCreate
            , const tCIDLib::TCard4         c4Count
        );

        tCIDLib::TVoid Recover();

        tCIDLib::TVoid RewriteTree();

        tCIDLib::TVoid SetNonMSRegVars();

        tCIDLib::TVoid SetStep
        (
            const   tCQCInst::EInstSteps    eCurStep
        );

        tCIDLib::TVoid ShowError
        (
            const   TError&                 errToCatch
        );

        tCIDLib::TVoid UpgradeClientData();

        tCIDLib::TVoid UpdateCommonData();

        tCIDLib::TVoid UpgradeData();

        tCIDLib::TVoid UpgradeInstSrvInfo();

        tCIDLib::TVoid UpgradeLogicSrvInfo();

        tCIDLib::TVoid UpgradeMasterData();

        tCIDLib::TVoid UpgradeSecSrvInfo();

        tCIDLib::TVoid UpgradeServerData();

        tCIDLib::TVoid UpgradeSharedData();

        tCIDLib::TVoid UpdateFiles
        (
            const   TString&                strSrc
            , const TString&                strTarget
            , const tCQCRemBrws::EDTypes    eType
            ,       TChunkedFile&           chflTmp
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_colParms
        //      The parameters we pass to the service. These have to be
        //      passed in twice, once for the create and once for the start,
        //      so we set them up once and reuse them.
        //
        //  m_eCurStep
        //      The current step that we are on
        //
        //  m_ppanStatus
        //      We get a pointer to the status panel which is to whom we
        //      report status info, and who started us and will wait for us
        //      to finish.
        //
        //  m_pinfoToUse
        //      A poitner to the installation information to use to drive the
        //      installation. We don't own it.
        //
        //  m_pwndOwner
        //      A pointer to the main frame window that we use to do synchronous
        //      user interaction stuff through.
        //
        //  m_strClientDir
        //      The client specific directory is not under the CQC data
        //      directory. It's in the common app data directory. So we build
        //      this up early on and keep it around for any time we need to
        //      access that stuff.
        //
        //  m_strSrvSecName
        //  m_strSrvSecPW
        //      These are values that will need to get put into the registry. They
        //      will either come from the install info (if this is not the MS) or
        //      we'll set them ourself when we update the security server repo.
        // -------------------------------------------------------------------
        tCIDLib::TStrList       m_colParms;
        tCQCInst::EInstSteps    m_eCurStep;
        TInstStatusPanel*       m_ppanStatus;
        const TCQCInstallInfo*  m_pinfoToUse;
        TMainFrameWnd*          m_pwndOwner;
        TString                 m_strClientDir;
        TString                 m_strSrvSecName;
        TString                 m_strSrvSecPW;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TInstallThread,TThread)
};

#pragma CIDLIB_POPPACK


