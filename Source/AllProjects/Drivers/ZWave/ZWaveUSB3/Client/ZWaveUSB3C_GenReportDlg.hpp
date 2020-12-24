//
// FILE NAME: ZWaveUSB3C_GenReportDlg.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/15/2018
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
//  This is the header for a small dialog that displays the information from a
//  driver report.
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
//   CLASS: TZWUSB3GenReportDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------
class TZWUSB3GenReportDlg : public TDlgBox
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TZWUSB3GenReportDlg();

        TZWUSB3GenReportDlg(const TZWUSB3GenReportDlg&) = delete;

        ~TZWUSB3GenReportDlg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TZWUSB3GenReportDlg& operator=(const TZWUSB3GenReportDlg&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid RunDlg
        (
                    TZWaveUSB3CWnd&         wndDriver
            , const TZWUnitInfo&            unitiSrc
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCreated() override;

        tCIDLib::TVoid Destroyed() override;

        tCIDLib::TVoid Timer
        (
            const   tCIDCtrls::TTimerId     tmidToDo
        )   override;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );

        tCIDLib::EExitCodes eQueryThread
        (
                    TThread&                thrThis
            ,       tCIDLib::TVoid*         pData
        );

        tCIDLib::TVoid QueryReport();

        tCIDLib::TVoid SaveInfo();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bError
        //      Our timer watches this. It indicates an error occurred and m_strErrMsg
        //      is set. This means the thread has stopped
        //
        //  m_bGotInfo
        //      It's a two stage process, so we need to know which to do when the
        //      user hits either of the buttons. Starts off false. The timer watches
        //      this to be set to indicate success.
        //
        //  m_punitiSrc
        //      A pointer to the unit we were told to get info for.
        //
        //  m_pwndDriver
        //      We get a pointer to the driver window so that we can send msgs.
        //
        //  m_pwndXXX
        //      Some controls we need to interact with, so we get typed pointers
        //
        //  m_strErrMsg
        //      If the thread gets back an error from the server, it will set
        //      m_bError and place the error msg here for us to display.
        //
        //  m_strReportText
        //      The query thread stores the info here for the GUI thread to grab it and
        //      display it. By that time the query thread is gone, so no sync required.
        //
        //  m_strTitle
        //      The original title text. We put the unit name into the title text, so
        //      we don't want to let popups pick that up automatically.
        //
        //  m_tmidMonitor
        //      A timer to monitor the thread and know when he completes, so that
        //      we can re-enable the buttons and display the gotten info (or an error
        //      if we failed.)
        //
        //  m_thrQuery
        //      There's a lot going on to get all the info, so we use a background
        //      thread.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bError;
        tCIDLib::TBoolean       m_bGotInfo;
        const TZWUnitInfo*      m_punitiSrc;
        TPushButton*            m_pwndClose;
        TZWaveUSB3CWnd*         m_pwndDriver;
        TPushButton*            m_pwndDoIt;
        TStaticText*            m_pwndProcessing;
        TMultiEdit*             m_pwndText;
        TString                 m_strErrMsg;
        TString                 m_strReportText;
        TString                 m_strTitle;
        tCIDCtrls::TTimerId     m_tmidMonitor;
        TThread                 m_thrQuery;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWUSB3GenReportDlg,TDlgBox)
};

#pragma CIDLIB_POPPACK



