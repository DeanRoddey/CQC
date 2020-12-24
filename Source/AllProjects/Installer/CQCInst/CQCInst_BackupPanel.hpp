//
// FILE NAME: CQCInst_BackupPanel.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 05/12/2010
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
//  This is the header for the info panel derived class that handles getting
//  the path to do a backup of the existing system into and invoking that
//  backup operation.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TInstBackupPanel
//  PREFIX: pan
// ---------------------------------------------------------------------------
class TInstBackupPanel : public TInstInfoPanel
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TInstBackupPanel() = delete;

        TInstBackupPanel
        (
                    TCQCInstallInfo* const  pinfoCur
            ,       TMainFrameWnd* const    pwndParent
        );

        TInstBackupPanel(const TInstBackupPanel&) = delete;
        TInstBackupPanel(TInstBackupPanel&&) = delete;

        ~TInstBackupPanel();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TInstBackupPanel& operator=(const TInstBackupPanel&) = delete;
        TInstBackupPanel& operator=(TInstBackupPanel&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bPanelIsVisible() const final;

        tCIDLib::TVoid SaveContents() final;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCreated() final;


    private :
        // -------------------------------------------------------------------
        //  Private class types
        // -------------------------------------------------------------------
        enum class EResults
        {
            Idle
            , Waiting
            , Done
            , DoneWithError
        };


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bDoBackup();

        tCIDLib::EExitCodes eBackupThread
        (
                    TThread&                thrThis
            ,       tCIDLib::TVoid*         pData
        );

        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );

        tCIDLib::TVoid LoadFiles
        (
            const   TPathStr&               pathParent
            ,       tCIDLib::TStrList&      colFiles
            , const tCIDLib::TBoolean       bRecurse
            , const tCIDLib::TCard4         c4BasePathLen
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4Percent
        //      The background thread updates this for a rough percentage
        //      done indicator and we update a bar meter to reflect the
        //      current value.
        //
        //  m_eStatus
        //      The timer watches this and the background thread updates it
        //      when it either succeeds or fails.
        //
        //  m_pathFinalTarDir
        //  m_pathSrcDir
        //  m_pathTarDir
        //      We set up the final target directory (the version/time stamped
        //      one), the source directory, and the temporary output directory
        //      before we invoke the background thread.
        //
        //  m_pwndXXX
        //      We get typed pointers to some of our child widgets. We don't
        //      own them, we just want typed pointers to interact with them.
        //
        //  m_thrBackup
        //      We have to do the backup via a background thread. We start it
        //      up on the eBackupThread method above.
        // -------------------------------------------------------------------
        tCIDLib::TCard4 m_c4Percent;
        EResults        m_eStatus;
        TPathStr        m_pathFinalTarDir;
        TPathStr        m_pathSrcDir;
        TPathStr        m_pathTarDir;
        TPushButton*    m_pwndBrowse;
        TProgressBar*   m_pwndPercent;
        TPushButton*    m_pwndDoBackup;
        TEntryField*    m_pwndPath;
        TThread         m_thrBackup;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TInstBackupPanel,TInstInfoPanel)
};

#pragma CIDLIB_POPPACK


