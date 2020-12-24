//
// FILE NAME: CQCInstUnp_MainWindow.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/24/2015
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
//  This is the header for the CQCInstUnp_MainWindow.Cpp module. This is a
//  single window program, so basically it's just this plus msg boxes where
//  needed.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $_CIDLib_Log_$
//
#pragma once


// ---------------------------------------------------------------------------
//  CLASS: TMainWindow
// PREFIX: wnd
// ---------------------------------------------------------------------------
class TMainWindow : public TFrameWnd
{
    public  :
        // -------------------------------------------------------------------
        // Constructors and destructor
        // -------------------------------------------------------------------
        TMainWindow();

        ~TMainWindow();


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCreateMainWnd();


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bAllowShutdown() override;

        tCIDLib::TBoolean bCreated() override;

        tCIDLib::TVoid Timer
        (
            const   tCIDCtrls::TTimerId     tmidToDo
        )   override;


    private :
        // -------------------------------------------------------------------
        //  Private types
        // -------------------------------------------------------------------
        enum EStates
        {
            EState_WaitInput
            , EState_Unpacking
            , EState_Success
            , EState_Failure
            , EState_Exception
        };


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid CheckShutdown();

        tCIDLib::TVoid CopyDir
        (
            const   TString&                strTarPath
            , const tCIDLib::TCh* const     pszSrcSub
            , const tCIDLib::TCh* const     pszTarSub
        );

        tCIDLib::TVoid CopyDLL
        (
            const   TString&                strTarPath
            , const tCIDLib::TCh* const     pszSub
            , const tCIDLib::TCh* const     pszBaseName
        );

        tCIDLib::TVoid DoExtraction
        (
            const   TString&                strTarPath
        );

        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );

        tCIDLib::EExitCodes eWorkThread
        (
                    TThread&                thrThis
            ,       tCIDLib::TVoid*         pData
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4FilesSoFar
        //      We pass this to the extraction engine, which updates it as it is
        //      extracting files, and we display this info.
        //
        //  m_c4TotalFiles
        //  m_c8Version
        //  m_enctTimeStamp
        //      We get this info from the CIDPack file upon startup, so that we
        //      can display it.
        //
        //  m_ePrevState
        //  m_eState
        //      The bgn thread updates m_eState and our timer handler uses prev
        //      state to know when the state has changed by storing the last state
        //      he saw. When the timer sees a final state (good or bad) he knows
        //      that the worker thread is done.
        //
        //  m_errReason
        //      If we get an exception, it's stored here for display once the timer
        //      sees we've gone into an error state.
        //
        //  m_pwndXXX
        //      Some typed pointers to child windows we need to interact with
        //      in a typed way. We don't own them.
        //
        //  m_strFmtVer
        //      This is the m_c8Version value formatted out for display.
        //
        //  m_strErrReason
        //      The bgn thread leaves an error reason string here if it fails. If
        //      an exception it also sets m_errReason.
        //
        //  m_strNotes
        //      Gotten from the pack file upon startup for display.
        //
        //  m_strSrcPackFile
        //      The path to the source package to extract. It's assumed to be in
        //      our own executable's directory so we look for it there.
        //
        //  m_strTarPath
        //      The target path. We store that here so taht the background thread
        //      knows where to put stuff.
        //
        //  m_strTimerFmt
        //      A temp string for the timer to format out status data for display.
        //
        //  m_tmidUpdate
        //      We start a timer to monitor the bgn thread.
        //
        //  m_thrWorker
        //      We do the work in a background thread, which will update the
        //      m_c4SoFar counter as it goes, and the m_eState member to indicate
        //      its status.
        // -------------------------------------------------------------------
        tCIDLib::TCard4         m_c4FilesSoFar;
        tCIDLib::TCard4         m_c4TotalFiles;
        tCIDLib::TCard8         m_c8Version;
        tCIDLib::TEncodedTime   m_enctTimeStamp;
        EStates                 m_ePrevState;
        EStates                 m_eState;
        TError                  m_errReason;
        TPushButton*            m_pwndBrowseButton;
        TPushButton*            m_pwndCancelButton;
        TPushButton*            m_pwndContButton;
        TStaticMultiText*       m_pwndInstruct;
        TStaticMultiText*       m_pwndNotes;
        TStaticText*            m_pwndStatus;
        TEntryField*            m_pwndTarDir;
        TStaticText*            m_pwndTarDirPref;
        TCheckBox*              m_pwndUseTempDir;
        TString                 m_strErrReason;
        TString                 m_strFmtVer;
        TString                 m_strNotes;
        TString                 m_strSrcPackFile;
        TString                 m_strTarPath;
        TString                 m_strTimerFmt;
        tCIDCtrls::TTimerId     m_tmidUpdate;
        TThread                 m_thrWorker;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TMainWindow,TFrameWnd)
};

