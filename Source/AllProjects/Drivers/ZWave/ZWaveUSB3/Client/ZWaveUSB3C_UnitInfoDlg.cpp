//
// FILE NAME: ZWaveUSB3C_UnitInfoDlg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/19/2018
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
//  This file implements a small dialog that lets the user see unit info details.
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
RTTIDecls(TZWUSB3UnitInfoDlg,TDlgBox)



// ---------------------------------------------------------------------------
//   CLASS: TZWUSB3UnitInfoDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWUSB3UnitInfoDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TZWUSB3UnitInfoDlg::TZWUSB3UnitInfoDlg() :

    m_bGotInfo(kCIDLib::False)
    , m_c4FanModes(0)
    , m_c4SetPoints(0)
    , m_c4ThermoModes(0)
    , m_c8ManIds(0)
    , m_punitiSrc(nullptr)
    , m_pwndClose(nullptr)
    , m_pwndDoIt(nullptr)
    , m_pwndDriver(nullptr)
    , m_pwndText(nullptr)
    , m_thrQuery
      (
        TString(L"ZWUSB3ClUnitInfo")
        , TMemberFunc<TZWUSB3UnitInfoDlg>(this, &TZWUSB3UnitInfoDlg::eQueryThread)
      )
    , m_tmidMonitor(kCIDCtrls::tmidInvalid)
{
}

TZWUSB3UnitInfoDlg::~TZWUSB3UnitInfoDlg()
{
}


// ---------------------------------------------------------------------------
//  TZWUSB3UnitInfoDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TZWUSB3UnitInfoDlg::RunDlg(TZWaveUSB3CWnd& wndDriver, const TZWUnitInfo& unitiSrc)
{
    // Save pointers to the incoming for later use
    m_pwndDriver = &wndDriver;
    m_punitiSrc = &unitiSrc;
    c4RunDlg(wndDriver, facZWaveUSB3C(), kZWaveUSB3C::ridDlg_UnitInfo);
}


// ---------------------------------------------------------------------------
//  TZWUSB3UnitInfoDlg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TZWUSB3UnitInfoDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRet = TParent::bCreated();

    // Store our pre-typed child widget pointers
    CastChildWnd(*this, kZWaveUSB3C::ridDlg_UnitInfo_Close, m_pwndClose);
    CastChildWnd(*this, kZWaveUSB3C::ridDlg_UnitInfo_DoIt, m_pwndDoIt);
    CastChildWnd(*this, kZWaveUSB3C::ridDlg_UnitInfo_Processing, m_pwndProcessing);
    CastChildWnd(*this, kZWaveUSB3C::ridDlg_UnitInfo_Text, m_pwndText);

    // Register our button event handlers
    m_pwndClose->pnothRegisterHandler(this, &TZWUSB3UnitInfoDlg::eClickHandler);
    m_pwndDoIt->pnothRegisterHandler(this, &TZWUSB3UnitInfoDlg::eClickHandler);

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


tCIDLib::TVoid TZWUSB3UnitInfoDlg::Destroyed()
{
    // Clean up our update timer if it got started and wasn't stopped normally
    if (m_tmidMonitor != kCIDCtrls::tmidInvalid)
    {
        StopTimer(m_tmidMonitor);
        m_tmidMonitor = kCIDCtrls::tmidInvalid;
    }

    TParent::Destroyed();
}


tCIDLib::TVoid TZWUSB3UnitInfoDlg::Timer(const tCIDCtrls::TTimerId tmidToDo)
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

        TTextStringOutStream strmFmt(2048UL);
        FormatText(strmFmt);
        m_pwndText->strWndText(strmFmt.strData());
    }

}



// ---------------------------------------------------------------------------
//  TZWUSB3UnitInfoDlg: Private, non-virtual methods
// ---------------------------------------------------------------------------

// Handle button presses
tCIDCtrls::EEvResponses TZWUSB3UnitInfoDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kZWaveUSB3C::ridDlg_UnitInfo_DoIt)
    {
        //
        //  If we have not yet gotten the info, we start the process. Else we
        //  are now doing the save, so we get the output file name.
        //
        if (m_bGotInfo)
            SaveInfo();
        else
            QueryInfo();
    }
     else if (wnotEvent.widSource() == kZWaveUSB3C::ridDlg_UnitInfo_Close)
    {
        EndDlg(kZWaveUSB3C::ridDlg_UnitInfo_Close);
    }
    return tCIDCtrls::EEvResponses::Handled;
}


//
//  The background thread is started here. We ask the the server side driver to do
//  the queries for us and just wait for the return. We set our status flags according
//  to the result.
//
tCIDLib::EExitCodes
TZWUSB3UnitInfoDlg::eQueryThread(TThread& thrThis, tCIDLib::TVoid* pData)
{
    // Let the parent thread go now that we have the info
    thrThis.Sync();

    try
    {
        TZWDriverXData zwdxdComm;
        zwdxdComm.m_strCmd = kZWaveUSB3Sh::strSendData_QueryUnitInfo;
        zwdxdComm.m_c1UnitId = m_punitiSrc->c1Id();
        THeapBuf mbufComm(2048, 8192);

        TWndPtrJanitor janWait(tCIDCtrls::ESysPtrs::Wait);
        if (m_pwndDriver->bSendSrvMsg(*this, zwdxdComm, mbufComm))
        {
            TBinMBufInStream strmSrc(&zwdxdComm.m_mbufData, zwdxdComm.m_c4BufSz);
            strmSrc >> m_c8ManIds
                    >> m_colCCs
                    >> m_colEPs
                    >> m_c4FanModes
                    >> m_c4ThermoModes
                    >> m_c4SetPoints;
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


// Format our gotten info to a text stream
tCIDLib::TVoid TZWUSB3UnitInfoDlg::FormatText(TTextOutStream& strmFmt)
{
    // Display the information
    strmFmt << L"Name: " << m_punitiSrc->strName()
            << L"\nId: " << m_punitiSrc->c1Id()
            << L"\nListener: " << m_punitiSrc->bListener()
            << L"\nFreq Listener: " << m_punitiSrc->bFreqListener()
            << L"\nMan Ids: "

            << TCardinal(tCIDLib::TCard2(m_c8ManIds >> 32), tCIDLib::ERadices::Hex)
            << kCIDLib::chSpace
            << TCardinal(tCIDLib::TCard2(m_c8ManIds >> 16), tCIDLib::ERadices::Hex)
            << kCIDLib::chSpace
            << TCardinal(tCIDLib::TCard2(m_c8ManIds), tCIDLib::ERadices::Hex);

    strmFmt << L"\n\nClasses: \n";
    tCIDLib::TCard4 c4Count = m_colCCs.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TZWClassInfo& zwciCur = m_colCCs[c4Index];
        strmFmt << L"    " << tZWaveUSB3Sh::strXlatECClasses(zwciCur.m_eClass)
                << L" (" << zwciCur.m_c4Version;

        if (zwciCur.m_bSecure)
            strmFmt << L", Secure";

        strmFmt << L")\n";
    }

    if (!m_colEPs.bIsEmpty())
    {
        strmFmt << L"\n\nMulti-Channel End Points: \n";
        c4Count = m_colEPs.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const TZWUSB3EndPntInfo& zwepiCur = m_colEPs[c4Index];
            strmFmt << L"    EP Id: " << zwepiCur.m_c4EndPntId
                    << L"\n        Dynamic: " << facCQCKit().strBoolYesNo(zwepiCur.m_bDynamic)
                    << L"\n        Generic: " << TCardinal(zwepiCur.m_c1Generic, tCIDLib::ERadices::Hex)
                    << L"\n        Specific: " << TCardinal(zwepiCur.m_c1Specific, tCIDLib::ERadices::Hex)
                    << L"\n        Classes: \n";

            const tCIDLib::TCard4 c4CCCnt = zwepiCur.m_fcolClasses.c4ElemCount();
            for (tCIDLib::TCard4 c4CCIndex = 0; c4CCIndex < c4CCCnt; c4CCIndex++)
            {
                strmFmt << L"            ";
                if (c4CCIndex)
                    strmFmt << L", ";

                strmFmt << tZWaveUSB3Sh::strXlatECClasses(zwepiCur.m_fcolClasses[c4CCIndex])
                        << kCIDLib::NewLn;
            }

            strmFmt << kCIDLib::NewLn;
        }
    }

    if (m_c4ThermoModes)
    {
        strmFmt << L"\n\nSupported Thermo Modes: (0x"
                << TCardinal(m_c4ThermoModes, tCIDLib::ERadices::Hex)
                << L")\n    ";

        if (m_c4ThermoModes & 0x0001)
            strmFmt << L"Off ";
        if (m_c4ThermoModes & 0x0002)
            strmFmt << L"Heat ";
        if (m_c4ThermoModes & 0x0004)
            strmFmt << L"Cool ";
        if (m_c4ThermoModes & 0x0008)
            strmFmt << L"Auto ";
        if (m_c4ThermoModes & 0x0010)
            strmFmt << L"Aux ";
        if (m_c4ThermoModes & 0x0020)
            strmFmt << L"Resume On ";
        if (m_c4ThermoModes & 0x0040)
            strmFmt << L"Fan ";
        if (m_c4ThermoModes & 0x0080)
            strmFmt << L"Furnace ";

        if (m_c4ThermoModes & 0x0100)
            strmFmt << L"Dry ";
        if (m_c4ThermoModes & 0x0200)
            strmFmt << L"Moist ";
        if (m_c4ThermoModes & 0x0400)
            strmFmt << L"AutoChange ";
        if (m_c4ThermoModes & 0x0800)
            strmFmt << L"EnergyHeat ";
        if (m_c4ThermoModes & 0x1000)
            strmFmt << L"EnergyCool ";
        if (m_c4ThermoModes & 0x2000)
            strmFmt << L"Away ";

        if (m_c4ThermoModes & 0x8000)
            strmFmt << L"FullPower ";
        strmFmt << kCIDLib::NewLn;
    }

    if (m_c4FanModes)
    {
        strmFmt << L"\n\nSupported Fan Modes: (0x"
                << TCardinal(m_c4FanModes, tCIDLib::ERadices::Hex) << L")\n    ";

        if (m_c4FanModes & 0x01)
            strmFmt << L"AutoLow ";
        if (m_c4FanModes & 0x02)
            strmFmt << L"Low ";
        if (m_c4FanModes & 0x04)
            strmFmt << L"AutoHigh ";
        if (m_c4FanModes & 0x08)
            strmFmt << L"High ";
        if (m_c4FanModes & 0x10)
            strmFmt << L"AutoMed ";
        if (m_c4FanModes & 0x20)
            strmFmt << L"Medium ";

        strmFmt << kCIDLib::NewLn;
    }

    if (m_c4SetPoints)
    {
        strmFmt << L"\n\nSupported Thermo Setpoints: (0x"
                << TCardinal(m_c4SetPoints, tCIDLib::ERadices::Hex) << L")\n    ";

        if (m_c4SetPoints & 0x0002)
            strmFmt << L"Heating ";
        if (m_c4SetPoints & 0x0004)
            strmFmt << L"Cooling ";
        if (m_c4SetPoints & 0x0080)
            strmFmt << L"Furnace ";
        if (m_c4SetPoints & 0x0100)
            strmFmt << L"DryAir ";
        if (m_c4SetPoints & 0x0200)
            strmFmt << L"MoistAir ";
        if (m_c4SetPoints & 0x0400)
            strmFmt << L"AutoChange ";
        if (m_c4SetPoints & 0x0800)
            strmFmt << L"SaveHeat ";
        if (m_c4SetPoints & 0x1000)
            strmFmt << L"SaveCool ";
        if (m_c4SetPoints & 0x2000)
            strmFmt << L"AwayHeat ";

        strmFmt << kCIDLib::NewLn;
    }

    strmFmt << kCIDLib::NewEndLn;
}


//
//  This method is called if the user hits the DoIt button (now showing Start) and
//  we have not yet gotten the info. We start up the thread that does the query
//  process and monitors it
//
tCIDLib::TVoid TZWUSB3UnitInfoDlg::QueryInfo()
{
    // Give them some instructions and a chance to back out
    TYesNoBox msgbDoIt
    (
        facZWaveUSB3C().strMsg(kZWU3CMsgs::midDlg_UnitInfo_StartInstruct)
    );
    if (!msgbDoIt.bShowIt(*this))
        return;

    // They say do it so let's disable the Close and DoIt buttons
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
tCIDLib::TVoid TZWUSB3UnitInfoDlg::SaveInfo()
{
    // Let the user select an output file
    tCIDLib::TBoolean bRes = kCIDLib::False;
    TString strSelected;
    {
        tCIDLib::TKVPList colFileTypes(1);
        colFileTypes.objAdd(TKeyValuePair(L"CQC Z-Wave Unit Dump", L"*.CQCZWUDmp"));

        // Create a default name
        TPathStr pathDefName(L"[yourname]_[make]_[model].CQCZWDump");
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
        // If it still has any of the replacement values, reject it
        if (strSelected.bContainsSubStr(L"[yourname]", kCIDLib::False)
        ||  strSelected.bContainsSubStr(L"[make]", kCIDLib::False)
        ||  strSelected.bContainsSubStr(L"[model]", kCIDLib::False))
        {
            TErrBox msgbErr
            (
                L"Please replace the values in the file name with accurate information "
                L"about the unit whose information is being saved. This is necessary for "
                L"us to provide support."
            );
            msgbErr.ShowIt(*this);
            return;
        }

        TTextFileOutStream strmTar
        (
            strSelected
            , tCIDLib::ECreateActs::CreateAlways
            , tCIDLib::EFilePerms::Default
            , tCIDLib::EFileFlags::SafeStream
            , tCIDLib::EAccessModes::Write
            , new TUTF8Converter
        );
        FormatText(strmTar);
    }
}
