//
// FILE NAME: ZWaveUSB3C_GenReportDlg.cpp
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
//  This file implements a small dialog that lets the user get a report from the
//  driver and display it.
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
#include    "ZWaveUSB3C_.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TZWUSB3GenReportDlg,TDlgBox)



// ---------------------------------------------------------------------------
//   CLASS: TZWUSB3GenReportDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWUSB3GenReportDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TZWUSB3GenReportDlg::TZWUSB3GenReportDlg() :

    m_bGotInfo(kCIDLib::False)
    , m_pwndClose(nullptr)
    , m_pwndDoIt(nullptr)
    , m_pwndDriver(nullptr)
    , m_pwndText(nullptr)
    , m_thrQuery
      (
        TString(L"ZWUSB3ClGenReport")
        , TMemberFunc<TZWUSB3GenReportDlg>(this, &TZWUSB3GenReportDlg::eQueryThread)
      )
    , m_tmidMonitor(kCIDCtrls::tmidInvalid)
{
}

TZWUSB3GenReportDlg::~TZWUSB3GenReportDlg()
{
}


// ---------------------------------------------------------------------------
//  TZWUSB3GenReportDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TZWUSB3GenReportDlg::RunDlg(TZWaveUSB3CWnd& wndDriver, const TZWUnitInfo& unitiSrc)
{
    // Save pointers to the incoming for later use
    m_pwndDriver = &wndDriver;
    m_punitiSrc = &unitiSrc;
    c4RunDlg(wndDriver, facZWaveUSB3C(), kZWaveUSB3C::ridDlg_GenReport);
}


// ---------------------------------------------------------------------------
//  TZWUSB3GenReportDlg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TZWUSB3GenReportDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRet = TParent::bCreated();

    // Store our pre-typed child widget pointers
    CastChildWnd(*this, kZWaveUSB3C::ridDlg_GenReport_Close, m_pwndClose);
    CastChildWnd(*this, kZWaveUSB3C::ridDlg_GenReport_DoIt, m_pwndDoIt);
    CastChildWnd(*this, kZWaveUSB3C::ridDlg_GenReport_Processing, m_pwndProcessing);
    CastChildWnd(*this, kZWaveUSB3C::ridDlg_GenReport_Text, m_pwndText);

    // Register our button event handlers
    m_pwndClose->pnothRegisterHandler(this, &TZWUSB3GenReportDlg::eClickHandler);
    m_pwndDoIt->pnothRegisterHandler(this, &TZWUSB3GenReportDlg::eClickHandler);

    // Set a fixed pitch font on the text display
    m_pwndText->SetNewFont(TGUIFacility::gfontDefFixed().fselCurrent());

    //
    //  Update the title text to include the unit name. Save the original text for
    //  use in popups.
    //
    m_strTitle = strWndText();
    TString strTitle = m_strTitle;
    strTitle.Append(L" [");
    strTitle.Append(m_punitiSrc->strName());
    strTitle.Append(L"]");
    strWndText(strTitle);

    return bRet;
}


tCIDLib::TVoid TZWUSB3GenReportDlg::Destroyed()
{
    // Clean up our update timer if it got started and wasn't stopped normally
    if (m_tmidMonitor != kCIDCtrls::tmidInvalid)
    {
        StopTimer(m_tmidMonitor);
        m_tmidMonitor = kCIDCtrls::tmidInvalid;
    }

    TParent::Destroyed();
}


tCIDLib::TVoid TZWUSB3GenReportDlg::Timer(const tCIDCtrls::TTimerId tmidToDo)
{
    // IF not our guy, pass to our parent
    if (tmidToDo != m_tmidMonitor)
    {
        TParent::Timer(tmidToDo);
        return;
    }

    //
    //  Check the status flags. If either is set, then stop ourself, and update
    //  the buttons.
    //
    if (m_bError || m_bGotInfo)
    {
        StopTimer(m_tmidMonitor);
        m_tmidMonitor = kCIDCtrls::tmidInvalid;

        m_pwndDoIt->SetEnable(kCIDLib::True);
        m_pwndClose->SetEnable(kCIDLib::True);

        // Clear out the status text value
        m_pwndProcessing->ClearText();
    }

    // And tell the user what happened
    if (m_bError)
    {
        TString strMsg = facZWaveUSB3C().strMsg(kZWU3CMsgs::midDlg_Gen_Failed);
        strMsg.Append(m_strErrMsg);
        TErrBox msgbFailed
        (
            facZWaveUSB3C().strMsg(kZWU3CMsgs::midDlg_Gen_FailedTitle), strMsg
        );
        msgbFailed.ShowIt(*this);
    }
     else if (m_bGotInfo)
    {
        // Update the text of the DoIt button to save now
        m_pwndDoIt->strWndText(facZWaveUSB3C().strMsg(kZWU3CMsgs::midDlg_Gen_Save));
        m_pwndText->strWndText(m_strReportText);
    }

}



// ---------------------------------------------------------------------------
//  TZWUSB3GenReportDlg: Private, non-virtual methods
// ---------------------------------------------------------------------------

// Handle button presses
tCIDCtrls::EEvResponses TZWUSB3GenReportDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kZWaveUSB3C::ridDlg_GenReport_DoIt)
    {
        //
        //  If we have not yet gotten the info, we start the process. Else we
        //  are now doing the save, so we get the output file name.
        //
        if (m_bGotInfo)
            SaveInfo();
        else
            QueryReport();
    }
     else if (wnotEvent.widSource() == kZWaveUSB3C::ridDlg_GenReport_Close)
    {
        EndDlg(kZWaveUSB3C::ridDlg_GenReport_Close);
    }
    return tCIDCtrls::EEvResponses::Handled;
}


//
//  The background thread is started here. We ask the the server side driver to do
//  the queries for us and just wait for the return. We set our status flags according
//  to the result.
//
tCIDLib::EExitCodes
TZWUSB3GenReportDlg::eQueryThread(TThread& thrThis, tCIDLib::TVoid* pData)
{
    // Let the parent thread go now that we have the info
    thrThis.Sync();

    try
    {
        TZWDriverXData zwdxdComm;
        zwdxdComm.m_strCmd = kZWaveUSB3Sh::strSendData_QueryUnitReport;
        zwdxdComm.m_c1UnitId = m_punitiSrc->c1Id();
        THeapBuf mbufComm(8192, 0x100000);
        if (m_pwndDriver->bSendSrvMsg(*this, zwdxdComm, mbufComm))
        {
            TZLibCompressor zlibReport;
            TBinMBufInStream strmSrc(&zwdxdComm.m_mbufData, zwdxdComm.m_c4BufSz);

            tCIDLib::TCard4 c4TextBytes = 0;
            THeapBuf mbufDecomp(zwdxdComm.m_c4BufSz * 2);
            {
                TBinMBufOutStream strmTar(&mbufDecomp);
                c4TextBytes = zlibReport.c4Decompress(strmSrc, strmTar, zwdxdComm.m_c4BufSz);
            }

            // Create a UTF8 converter and convert to string format
            TUTF8Converter tcvtUTF8;
            tcvtUTF8.c4ConvertFrom(mbufDecomp.pc1Data(), c4TextBytes, m_strReportText);
            m_bGotInfo = kCIDLib::True;
        }
         else
        {
            m_strErrMsg = zwdxdComm.m_strMsg;
            m_bError = kCIDLib::True;
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);
    }

    catch(...)
    {
    }

    return tCIDLib::EExitCodes::Normal;
}

//
//  This method is called if the user hits the DoIt button (now showing Start) and
//  we have not yet gotten the info. We start up the thread that does the query
//  process and monitors it
//
tCIDLib::TVoid TZWUSB3GenReportDlg::QueryReport()
{
    // Let's disable the Close and DoIt buttons
    m_pwndClose->SetEnable(kCIDLib::False);
    m_pwndDoIt->SetEnable(kCIDLib::False);

    // Set the text of the processing indicator
    m_pwndProcessing->strWndText
    (
        facZWaveUSB3C().strMsg(kZWU3CMsgs::midDlg_Gen_Processing)
    );

    //
    //  Make sure the error flag is cleared. The got info flag we know is cleared
    //  also or we wouldn't get here, but do it anyway.
    //
    m_bError = kCIDLib::False;
    m_bGotInfo = kCIDLib::False;

    // And now we can start the thread
    m_thrQuery.Start();

    // Start up the timer that will monitor the operation for completion
    m_tmidMonitor = tmidStartTimer(500);
}


//
//  This method is called if the user hits the DoIt button (now showing Save) and
//  we have already gotten the info. This lets him choose a file to save it to.
//
tCIDLib::TVoid TZWUSB3GenReportDlg::SaveInfo()
{
    // Let the user select an output file
    tCIDLib::TBoolean bRes = kCIDLib::False;
    TString strSelected;
    {
        tCIDLib::TKVPList colFileTypes(1);
        colFileTypes.objAdd(TKeyValuePair(L"CQC Z-Wave Report", L"*.CQCZWReport"));

        // Create a default name
        TPathStr pathDefName(m_pwndDriver->strMoniker(), L".CQCZWReport");

        bRes = facCIDCtrls().bSaveFileDlg
        (
            *this
            , facZWaveUSB3C().strMsg(kZWU3CMsgs::midMsg_SelectTargetFl)
            , TString::strEmpty()
            , pathDefName
            , strSelected
            , colFileTypes
            , tCIDCtrls::EFSaveOpts::FilesOnly
        );
    }

    // If they made a selection, then let's do it
    if (bRes)
    {
        TTextFileOutStream strmTar
        (
            strSelected
            , tCIDLib::ECreateActs::CreateAlways
            , tCIDLib::EFilePerms::Default
            , tCIDLib::EFileFlags::SafeStream
            , tCIDLib::EAccessModes::Write
            , new TUTF8Converter
        );
        strmTar << m_strReportText << kCIDLib::FlushIt;
    }
}
