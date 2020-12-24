//
// FILE NAME: CQCAdmin_QueryLogsTab.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/12/2016
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
//  This file implements a tab for querying the log server.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCAdmin.hpp"
#include    "CQCAdmin_QueryLogsTab.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TQueryLogsTab, TContentTab)


// ---------------------------------------------------------------------------
//  CLASS: TQueryLogsTab
// PREFIX: wnd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TQueryLogsTab: Constructors and destructor
// ---------------------------------------------------------------------------
TQueryLogsTab::TQueryLogsTab(const TString& strPath, const TString& strRelPath) :

    TContentTab(strPath, strRelPath, L"System", kCIDLib::False)
    , m_pwndClasses(nullptr)
    , m_pwndClear(nullptr)
    , m_pwndCopy(nullptr)
    , m_pwndCurAux(nullptr)
    , m_pwndCurErrs(nullptr)
    , m_pwndCurFac(nullptr)
    , m_pwndCurFile(nullptr)
    , m_pwndCurThread(nullptr)
    , m_pwndFilter(nullptr)
    , m_pwndList(nullptr)
    , m_pwndMaxHits(nullptr)
    , m_pwndSearch(nullptr)
    , m_pwndSevs(nullptr)
    , m_pwndSnap(nullptr)
{
    //
    //  Set up our filter list. We use a key/value pair. The key is the displayed human
    //  readable text, and the value is a comma separated list of the real filter
    //  criteria, which are in the order below. Wild cards are often used.
    //
    //  1. Process expression
    //  2. Host expression
    //  3. Facility expression
    //
    TString strTmp;

    m_colFilters.objPlace(L"All Messages", L"*, *, *");
    m_colFilters.objPlace(L"Admin Intf Messages", L"CQCAdmin, *, *");
    m_colFilters.objPlace(L"CQC Media Engine Messages", L"*, *, CQCMedia");
    m_colFilters.objPlace(L"Driver Messages", L"CQCServer, *, *");
    m_colFilters.objPlace(L"Event Server Messages", L"CQCEventSrv, *, *");
    m_colFilters.objPlace(L"Interface Engine Messages", L"*, *, CQCIntf*");

    strTmp = L"*, ";
    strTmp.Append(TSysInfo::strIPHostName());
    strTmp.Append(L", *");
    m_colFilters.objPlace(L"Local Host Messages", strTmp);

    m_colFilters.objPlace(L"Logic Server Messages", L"CQLogic*, *, *");
    m_colFilters.objPlace(L"Macro Engine Messages", L"*, *, CIDMacroEng");

    // Set up our time formatting object with the desired foramt
    m_tmFmt.strDefaultFormat(TTime::strMMDD_24HHMMSS());
}

TQueryLogsTab::~TQueryLogsTab()
{
}


// ---------------------------------------------------------------------------
//  TQueryLogsTab: Public, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid TQueryLogsTab::CreateTab(TTabbedWnd& wndParent, const TString& strTabText)
{
    wndParent.c4CreateTab(this, strTabText, 0, kCIDLib::True);
}


// ---------------------------------------------------------------------------
//  TQueryLogsTab: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TQueryLogsTab::AreaChanged(const    TArea&                  areaPrev
                            , const TArea&                  areaNew
                            , const tCIDCtrls::EPosStates   ePosState
                            , const tCIDLib::TBoolean       bOrgChanged
                            , const tCIDLib::TBoolean       bSizeChanged
                            , const tCIDLib::TBoolean       bStateChanged)
{
    // Call our parent first
    TParent::AreaChanged(areaPrev, areaNew, ePosState, bOrgChanged, bSizeChanged, bStateChanged);

    // Resize our controls to fit
    if ((ePosState != tCIDCtrls::EPosStates::Minimized) && bSizeChanged && m_pwndList)
        AutoAdjustChildren(areaPrev, areaNew);
}


tCIDLib::TBoolean TQueryLogsTab::bCreated()
{
    TParent::bCreated();

    // Get the dialog description we'll use to create the controls.
    TDlgDesc dlgdChildren;
    facCQCAdmin.bCreateDlgDesc(kCQCAdmin::ridTab_QueryLogs, dlgdChildren);

    // Create the children
    tCIDLib::TCard4 c4InitFocus;
    CreateDlgItems(dlgdChildren, c4InitFocus);

    // Do an initial auto-adjust to get them sized to fit us
    AutoAdjustChildren(dlgdChildren.areaPos(), areaClient());

    // Store our pre-typed child widget pointers
    CastChildWnd(*this, kCQCAdmin::ridTab_QueryLogs_Classes, m_pwndClasses);
    CastChildWnd(*this, kCQCAdmin::ridTab_QueryLogs_Clear, m_pwndClear);
    CastChildWnd(*this, kCQCAdmin::ridTab_QueryLogs_Copy, m_pwndCopy);
    CastChildWnd(*this, kCQCAdmin::ridTab_QueryLogs_CurAux, m_pwndCurAux);
    CastChildWnd(*this, kCQCAdmin::ridTab_QueryLogs_CurErr, m_pwndCurErrs);
    CastChildWnd(*this, kCQCAdmin::ridTab_QueryLogs_CurFac, m_pwndCurFac);
    CastChildWnd(*this, kCQCAdmin::ridTab_QueryLogs_CurFile, m_pwndCurFile);
    CastChildWnd(*this, kCQCAdmin::ridTab_QueryLogs_CurThread, m_pwndCurThread);
    CastChildWnd(*this, kCQCAdmin::ridTab_QueryLogs_Filter, m_pwndFilter);
    CastChildWnd(*this, kCQCAdmin::ridTab_QueryLogs_List, m_pwndList);
    CastChildWnd(*this, kCQCAdmin::ridTab_QueryLogs_MaxHits, m_pwndMaxHits);
    CastChildWnd(*this, kCQCAdmin::ridTab_QueryLogs_Search, m_pwndSearch);
    CastChildWnd(*this, kCQCAdmin::ridTab_QueryLogs_Sevs, m_pwndSevs);
    CastChildWnd(*this, kCQCAdmin::ridTab_QueryLogs_Snap, m_pwndSnap);

    // Register our event handlers
    m_pwndClear->pnothRegisterHandler(this, &TQueryLogsTab::eClickHandler);
    m_pwndCopy->pnothRegisterHandler(this, &TQueryLogsTab::eClickHandler);
    m_pwndList->pnothRegisterHandler(this, &TQueryLogsTab::eLBHandler);
    m_pwndSearch->pnothRegisterHandler(this, &TQueryLogsTab::eClickHandler);
    m_pwndSnap->pnothRegisterHandler(this, &TQueryLogsTab::eClickHandler);

    // Set our list columns
    tCIDLib::TStrList colCols(6);
    colCols.objAdd(L"Sev");
    colCols.objAdd(L"Host");
    colCols.objAdd(L"Process");
    colCols.objAdd(L"Line");
    colCols.objAdd(L"Time");
    colCols.objAdd(L"Text");
    m_pwndList->SetColumns(colCols);

    // Set the initial sort column to the time column
    m_pwndList->SetSortColumn(c4Col_Time);

    // Load the list of filters and select the 0th one
    const tCIDLib::TCard4 c4FCnt = m_colFilters.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4FCnt; c4Index++)
        m_pwndFilter->c4AddItem(m_colFilters[c4Index].strKey(), c4Index);
    m_pwndFilter->SelectByIndex(0);

    // Load up the fixed lists
    InitLists();

    // Set some initial column sizes, except for the last, which we allow to auto-size
    m_pwndList->SetColOpts(c4Col_Severity, TGUIFacility::c4AdjustHDlgUnit(30));
    m_pwndList->SetColOpts(c4Col_Host, TGUIFacility::c4AdjustHDlgUnit(48));
    m_pwndList->SetColOpts(c4Col_Process, TGUIFacility::c4AdjustHDlgUnit(52));
    m_pwndList->SetColOpts(c4Col_Line, TGUIFacility::c4AdjustHDlgUnit(28));
    m_pwndList->SetColOpts(c4Col_Time, TGUIFacility::c4AdjustHDlgUnit(54));
    m_pwndList->SetColOpts(c4Col_Text, TGUIFacility::c4AdjustHDlgUnit(220));

    return kCIDLib::True;
}



// ---------------------------------------------------------------------------
//  TQueryLogsTab: Private, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TCard8 TQueryLogsTab::c8SevMask() const
{
    tCIDLib::TCard8 c8Ret = 0;

    const tCIDLib::TCard4 c4Count = m_pwndSevs->c4ItemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (m_pwndSevs->bIsMarked(c4Index))
            c8Ret |= tCIDLib::TCard8(1) << c4Index;
    }
    return c8Ret;
}


tCIDLib::TCard8 TQueryLogsTab::c8ClassMask() const
{
    tCIDLib::TCard8 c8Ret = 0;

    const tCIDLib::TCard4 c4Count = m_pwndClasses->c4ItemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (m_pwndClasses->bIsMarked(c4Index))
            c8Ret |= tCIDLib::TCard8(1) << c4Index;
    }
    return c8Ret;
}


// Formats the currently displayed stuff to a string the writes to the clipboard
tCIDLib::TVoid TQueryLogsTab::CopyToClipboard()
{
    try
    {
        TTextStringOutStream strmOut(64 * 1024);

        //
        //  Go through our list and write them all out. We set up default
        //  time format on a time object to use for the time stamp.
        //
        TTime tmLogged;
        tmLogged.strDefaultFormat(L"%(M,2,0)/%(D,2,0) %(H,2,0):%(u,2,0):%(s,2,0)");

        //
        //  Our list is in reverse time order, and we want it to the other
        //  way, so put them out from last to first.
        //
        tCIDLib::TInt4 i4Index = tCIDLib::TInt4(m_colMatches.c4ElemCount());
        i4Index--;
        while (i4Index >= 0)
        {
            const TLogEvent& logevCur = m_colMatches[i4Index];
            logevCur.AdvFormat(strmOut, tmLogged);
            i4Index--;
        }
        strmOut.Flush();

        // Write this content to the clipboard
        TGUIClipboard gclipCopy(*this);
        gclipCopy.Clear();
        gclipCopy.StoreText(strmOut.strData());
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TExceptDlg dlgFailed
        (
            *this
            , strWndText()
            , L"AN error occurred while copying the data to the clipboard"
            , errToCatch
        );
    }
}


//
//  We grab the filter info and do a query. If it works we load the matches up to our
//  list window.
//
tCIDLib::TVoid TQueryLogsTab::DoQuery()
{
    try
    {
        // Build up the bits for the severity and error classes
        const tCIDLib::TCard8 c8Levels = c8SevMask();
        const tCIDLib::TCard8 c8Classes = c8ClassMask();

        // Get the current filter and parse out the values
        const tCIDLib::TCard4 c4FiltInd = m_pwndFilter->c4CurItem();
        const TKeyValuePair& kvalFilt = m_colFilters[c4FiltInd];
        TString strHostExpr;
        TString strProcExpr;
        TString strFacExpr;
        TStringTokenizer stokFilt(&kvalFilt.strValue(), L",");
        if (!stokFilt.bGetNextToken(strProcExpr)
        ||  !stokFilt.bGetNextToken(strHostExpr)
        ||  !stokFilt.bGetNextToken(strFacExpr))
        {
            // Should never happen but just in case
            TErrBox msgbErr(L"The internal filter data was not correct");
            msgbErr.ShowIt(*this);
            return;
        }
        strFacExpr.StripWhitespace();
        strHostExpr.StripWhitespace();
        strProcExpr.StripWhitespace();

        // Get a log server client proxy and do the query
        TWndPtrJanitor janPtr(tCIDCtrls::ESysPtrs::Wait);
        tCIDOrbUC::TLSrvProxy orbcLS = facCIDOrbUC().orbcLogSrvProxy(4000);
        const tCIDLib::TCard4 c4Count = orbcLS->c4QueryEvents
        (
            m_colMatches
            , m_pwndMaxHits->c4CurItemId()
            , strHostExpr
            , strProcExpr
            , strFacExpr
            , L"*"
            , c8Levels
            , c8Classes
        );

        // It worked, so load up the list, even if we got none so that the list gets cleared
        LoadMatches();
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TExceptDlg dlgErr
        (
            *this
            , strWndText()
            , facCQCAdmin.strMsg(kCQCAMsgs::midStatus_LogQueryFailed)
            , errToCatch
        );
    }
}


// Handle our buttons
tCIDCtrls::EEvResponses TQueryLogsTab::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCAdmin::ridTab_QueryLogs_Clear)
    {
        m_pwndList->RemoveAll();
    }
     else if (wnotEvent.widSource() == kCQCAdmin::ridTab_QueryLogs_Copy)
    {
        // Format the content out to text and then the clip board
        CopyToClipboard();
    }
     else if (wnotEvent.widSource() == kCQCAdmin::ridTab_QueryLogs_Search)
    {
        // Do a query based on current settings
        DoQuery();
    }
     else if (wnotEvent.widSource() == kCQCAdmin::ridTab_QueryLogs_Snap)
    {
        // Just call a helper
        SnapToFile();
    }
    return tCIDCtrls::EEvResponses::Handled;
}


//
//  We just want to update the detail info for the selected item in the list, or clear
//  them if the list is cleared.
//
tCIDCtrls::EEvResponses TQueryLogsTab::eLBHandler(TListChangeInfo& wnotClick)
{
    if (wnotClick.eEvent() == tCIDCtrls::EListEvents::Cleared)
    {
        // Clear out the detail controls
        m_pwndCurAux->ClearText();
        m_pwndCurErrs->ClearText();
        m_pwndCurFac->ClearText();
        m_pwndCurFile->ClearText();
        m_pwndCurThread->ClearText();
    }
     else if (wnotClick.eEvent() == tCIDCtrls::EListEvents::SelChanged)
    {
        // Use the row id for this row, which has the original index
        const TLogEvent& logevCur = m_colMatches[wnotClick.c4Id()];

        m_pwndCurFac->strWndText(logevCur.strFacName());
        m_pwndCurFile->strWndText(logevCur.strFileName());
        m_pwndCurThread->strWndText(logevCur.strThread());

        // Format out the errors
        m_strFmt.SetFormatted(logevCur.errcId());
        m_strFmt.Append(kCIDLib::chForwardSlash);
        m_strFmt.AppendFormatted(logevCur.errcKrnlId());
        m_strFmt.Append(kCIDLib::chForwardSlash);
        m_strFmt.AppendFormatted(logevCur.errcHostId());
        m_pwndCurErrs->strWndText(m_strFmt);

        if (logevCur.bHasAuxText())
            m_pwndCurAux->strWndText(logevCur.strAuxText());
        else
            m_pwndCurAux->ClearText();
    }
    return tCIDCtrls::EEvResponses::Handled;
}


// Called on load to get our various static lists loaded
tCIDLib::TVoid TQueryLogsTab::InitLists()
{
    TTextStringOutStream strmTmp(64UL);

    tCIDLib::ESeverities eSev = tCIDLib::ESeverities::Min;
    for (; eSev < tCIDLib::ESeverities::Count; eSev++)
    {
        strmTmp << eSev << kCIDLib::FlushIt;
        m_pwndSevs->c4AddItem(strmTmp.strData(), tCIDLib::c4EnumOrd(eSev));
        strmTmp.Reset();
    }
    m_pwndSevs->MarkAll(kCIDLib::True);

    tCIDLib::ForEachE<tCIDLib::EErrClasses>
    (
        [this, &strmTmp](const tCIDLib::EErrClasses eClass)
        {
            strmTmp << eClass << kCIDLib::FlushIt;
            m_pwndClasses->c4AddItem(strmTmp.strData(), tCIDLib::c4EnumOrd(eClass));
            strmTmp.Reset();
        }
    );
    m_pwndClasses->MarkAll(kCIDLib::True);

    // Load the mx hits list. We do it in powers of 2
    m_pwndMaxHits->c4AddItem(L"16", 16);
    m_pwndMaxHits->c4AddItem(L"32", 32);
    m_pwndMaxHits->c4AddItem(L"64", 64);
    m_pwndMaxHits->c4AddItem(L"128", 128);
    m_pwndMaxHits->c4AddItem(L"256", 256);
    m_pwndMaxHits->c4AddItem(L"512", 512);

    // Select the 64 one
    m_pwndMaxHits->SelectByIndex(2);
}


// This loads up the list of log events we got in the last query.
tCIDLib::TVoid TQueryLogsTab::LoadMatches()
{
    TWndPaintJanitor janList(m_pwndList);

    tCIDLib::TStrList colCols(6);
    colCols.AddXCopies(TString::strEmpty(), 6);

    m_pwndList->RemoveAll();
    const tCIDLib::TCard4 c4Count = m_colMatches.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TLogEvent& logevCur = m_colMatches[c4Index];

        // Set up all of the columns
        colCols[c4Col_Host] = logevCur.strHostName();
        colCols[c4Col_Process] = logevCur.strProcess();
        colCols[c4Col_Text] = logevCur.strErrText();
        colCols[c4Col_Line].SetFormatted(logevCur.c4LineNum());
        colCols[c4Col_Severity] = tCIDLib::strLoadESeverities(logevCur.eSeverity());

        m_tmFmt.enctTime(logevCur.enctLogged());
        m_tmFmt.FormatToStr(colCols[c4Col_Time]);

        m_pwndList->c4AddItem(colCols, c4Index);
    }

    // If we got any, select the first one
    if (c4Count)
        m_pwndList->SelectByIndex(0);
}


// Lets the user save the currently displayed messages to a text file
tCIDLib::TVoid TQueryLogsTab::SnapToFile()
{
    try
    {
        // Let them pick a target directory
        TString strTitle(kCQCAMsgs::midPrompt_SelSnapPath, facCQCAdmin);

        tCIDLib::TKVPList colTypes(1);
        colTypes.objPlace(L"Text Files", L"*.Txt");

        tCIDLib::TStrList colSel;
        const tCIDLib::TBoolean bRes = facCIDCtrls().bOpenFileDlg
        (
            *this
            , strTitle
            , TString::strEmpty()
            , colSel
            , colTypes
            , tCIDLib::eOREnumBits
              (
                tCIDCtrls::EFOpenOpts::NoReadOnly
                , tCIDCtrls::EFOpenOpts::FileSystemOnly
              )
        );
        if (!bRes)
            return;

        //
        //  Open a text output stream for the target path. Use a native
        //  wide character transcoder so that they can read it on their
        //  machine before sending it.
        //
        TTextFileOutStream strmOut(new TUTF8Converter);
        strmOut.Open
        (
            colSel[0]
            , tCIDLib::ECreateActs::CreateAlways
            , tCIDLib::EFilePerms::AllAccess
            , tCIDLib::EFileFlags::SequentialScan
            , tCIDLib::EAccessModes::Excl_OutStream
        );
        strmOut << kCIDLib::chUniBOM;

        //
        //  Go through our list and writ them all out. We set up default
        //  time format on a time object to use for the time stamp.
        //
        TTime tmLogged;
        tmLogged.strDefaultFormat(L"%(M,2,0)/%(D,2,0) %(H,2,0):%(u,2,0):%(s,2,0)");

        //
        //  Our list is in reverse time order, and we want it to the other
        //  way, so put them out from last to first.
        //
        tCIDLib::TInt4 i4Index = tCIDLib::TInt4(m_colMatches.c4ElemCount());
        i4Index--;
        while (i4Index >= 0)
        {
            const TLogEvent& logevCur = m_colMatches[i4Index];
            logevCur.AdvFormat(strmOut, tmLogged);
            i4Index--;
        }
        strmOut.Flush();
        strmOut.Close();

        // Let them know it was saved
        TOkBox msgbSaved
        (
            facCQCAdmin.strMsg(kCQCAMsgs::midTitle_LogFileSnap)
            , facCQCAdmin.strMsg(kCQCAMsgs::midPrompt_SavedOK)
        );
        msgbSaved.ShowIt(*this);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TExceptDlg dlgFailed
        (
            *this
            , strWndText()
            , L"An error occurred while writing out the information"
            , errToCatch
        );
    }
}

