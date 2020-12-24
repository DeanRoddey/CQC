//
// FILE NAME: ZWaveUSBC_Window.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/26/2005
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
//  This file implements the main window of the ZWave client driver
//
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "ZWaveUSBC_.hpp"


// ---------------------------------------------------------------------------
//  Magic macro
// ---------------------------------------------------------------------------
RTTIDecls(TZWaveUSBCWnd,TCQCDriverClient)



// ---------------------------------------------------------------------------
//   CLASS: TZWaveUSBCWnd
//  PREFIX: wnd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWaveUSBCWnd: Constructors and Destructor
// ---------------------------------------------------------------------------
TZWaveUSBCWnd::TZWaveUSBCWnd(const  TCQCDriverObjCfg&   cqcdcThis
                            , const TCQCUserCtx&        cuctxToUse) :

    TCQCDriverClient
    (
        cqcdcThis
        , L"TZWaveUSBCWnd"
        , tCQCKit::EActLevels::Low
        , cuctxToUse
    )
    , m_dplrClient(cqcdcThis.strMoniker())
    , m_pwndLearn(nullptr)
    , m_pwndReset(nullptr)
    , m_pwndUnitEdit(nullptr)
    , m_pwndUnits(nullptr)
{
}

TZWaveUSBCWnd::~TZWaveUSBCWnd()
{
}


// ---------------------------------------------------------------------------
//  TZWaveUSBCWnd: Protected, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TZWaveUSBCWnd::bChanges() const
{
    return (m_dcfgEdit != m_dcfgLast);
}


tCIDLib::TBoolean TZWaveUSBCWnd::bSaveChanges(TString& strErrMsg)
{
    // If no changes, then nothing to do
    if (m_dcfgEdit == m_dcfgLast)
        return kCIDLib::True;

    try
    {
        tCIDLib::TCard4 c4Size = 0;
        THeapBuf mbufData(16 * 1024);
        {
            TBinMBufOutStream strmTar(&mbufData);
            strmTar << m_dcfgEdit << kCIDLib::FlushIt;
            c4Size = strmTar.c4CurSize();
        }

        // The data name isn't used in this case, just the data type
        TString strDataName;
        orbcServer()->bSendData
        (
            strMoniker()
            , kZWaveUSBSh::strDrvCmd_SetConfig
            , strDataName
            , c4Size
            , mbufData
        );

        //
        //  It worked, or seems to have. To be save query back the config from the driver
        //  in case it changed anything.
        //
        orbcServer()->QueryConfig(strMoniker(), c4Size, mbufData);

        // Make an input stream over the data and stream in the config
        {
            TBinMBufInStream strmIn(&mbufData, c4Size);
            strmIn.CheckForStartMarker(CID_FILE, CID_LINE);
            strmIn >> m_dcfgLast;
            strmIn.CheckForEndMarker(CID_FILE, CID_LINE);
        }

        // Copy it to the edit version
        m_dcfgEdit = m_dcfgLast;

        // Now load the info to our controls
        LoadConfig();
    }

    catch(TError& errToCatch)
    {
        if (facZWaveUSBC().bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        TExceptDlg dlgbFail
        (
            *this
            , facZWaveUSBC().strMsg(kZWCMsgs::midGen_Title)
            , facZWaveUSBC().strMsg(kZWCMsgs::midStatus_SetConfigFailed)
            , errToCatch
        );
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


// ---------------------------------------------------------------------------
//  TZWaveUSBCWnd: Protected, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TZWaveUSBCWnd::bCreated()
{
    TParent::bCreated();

    // Get the dialog description we'll use to create the main controls
    TDlgDesc dlgdMain;
    facZWaveUSBC().bCreateDlgDesc(kZWaveUSBC::ridIntf_Main, dlgdMain);

    // And now create the controls
    tCIDCtrls::TWndId widInitFocus;
    CreateDlgItems(dlgdMain, widInitFocus);

    // Get typed pointers to the widgets we want to interact with a lot
    CastChildWnd(*this, kZWaveUSBC::ridIntf_Main_Learn, m_pwndLearn);
    CastChildWnd(*this, kZWaveUSBC::ridIntf_Main_Reset, m_pwndReset);
    CastChildWnd(*this, kZWaveUSBC::ridIntf_Main_UnitEdit, m_pwndUnitEdit);
    CastChildWnd(*this, kZWaveUSBC::ridIntf_Main_Units, m_pwndUnits);

    //
    //  Our list window is a two-columned one. So we use the multi-column
    //  list item class. We need to set up the column description info that
    //  we set on each of them as we add them to the list.
    //
    tCIDLib::TStrList colCols(2);
    colCols.objAdd(L"Unit Name");
    colCols.objAdd(L"Unit Id");
    m_pwndUnits->SetColumns(colCols);

    // Register event handlers
    m_pwndLearn->pnothRegisterHandler(this, &TZWaveUSBCWnd::eClickHandler);
    m_pwndReset->pnothRegisterHandler(this, &TZWaveUSBCWnd::eClickHandler);
    m_pwndUnitEdit->pnothRegisterHandler(this, &TZWaveUSBCWnd::eClickHandler);
    m_pwndUnits->pnothRegisterHandler(this, &TZWaveUSBCWnd::eLBHandler);

    return kCIDLib::True;
}


tCIDLib::TBoolean TZWaveUSBCWnd::bGetConnectData(tCQCKit::TCQCSrvProxy& orbcTarget)
{
    // Do any initial load of any config
    try
    {
        THeapBuf mbufCfg(8192UL);
        tCIDLib::TCard4 c4Sz;
        orbcServer()->QueryConfig(strMoniker(), c4Sz, mbufCfg);

        // Make an input stream over the data and stream in the config
        TBinMBufInStream strmIn(&mbufCfg, c4Sz);
        strmIn.CheckForStartMarker(CID_FILE, CID_LINE);
        strmIn >> m_dcfgLast;
        strmIn.CheckForEndMarker(CID_FILE, CID_LINE);

        // Copy it to the edit version
        m_dcfgEdit = m_dcfgLast;

        // Now load the info to our controls
        LoadConfig();
    }

    catch(...)
    {
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


tCIDLib::TBoolean TZWaveUSBCWnd::bDoPoll(tCQCKit::TCQCSrvProxy& orbcTarget)
{
    tCIDLib::TBoolean bRet;
    try
    {
        bRet = m_dplrClient.bPoll(orbcTarget);
    }

    catch(TError& errToCatch)
    {
        // If not a sync error, just throw
        if (errToCatch.eClass() != tCIDLib::EErrClasses::OutOfSync)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            throw;
        }

        // Just call our own get connect data. If it fails, we return false to give up
        bRet = bGetConnectData(orbcTarget);
    }
    return bRet;
}


tCIDLib::TVoid TZWaveUSBCWnd::Connected()
{
}


tCIDLib::TVoid TZWaveUSBCWnd::DoCleanup()
{
}


tCIDLib::TVoid TZWaveUSBCWnd::LostConnection()
{
}


tCIDLib::TVoid TZWaveUSBCWnd::UpdateDisplayData()
{
}


// ---------------------------------------------------------------------------
//  TZWaveUSBCWnd: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Asks he driver whether it is in replication mode, and if so, what it's
//  replication status is. It returns true if in replication mode.
//
tCIDLib::TBoolean TZWaveUSBCWnd::bDrvMode(tZWaveUSBSh::ERepRes& eToFill)
{
    tCIDLib::TBoolean bRet = kCIDLib::False;
    try
    {
        const tCIDLib::TCard4 c4Status = orbcServer()->c4QueryVal
        (
            strMoniker(), kZWaveUSBSh::strValId_DrvStatus
        );

        bRet = (c4Status & 0xFFFF) != 0;
        eToFill = tZWaveUSBSh::ERepRes(c4Status >> 16);
    }

    catch(TError& errToCatch)
    {
        if (facZWaveUSBC().bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        TExceptDlg dlgbFail
        (
            *this
            , facZWaveUSBC().strMsg(kZWCMsgs::midGen_Title)
            , facZWaveUSBC().strMsg(kZWCMsgs::midStatus_CantFindSrv)
            , errToCatch
        );
        return kCIDLib::False;
    }
    return bRet;
}


//
//  This is called on a button click. Mostly what we do is set up a string
//  to pass to the driver's Command field, but some will us the backdoor
//  c4SendCmd() method for things that no fields exist.
//
//  NOTE:   Groups and units can have spaces in their names so be sure to
//          quote them in the send strings.
//
tCIDCtrls::EEvResponses TZWaveUSBCWnd::eClickHandler(TButtClickInfo& wnotEvent)
{
    // If we are not connected, then do nothing
    if (eConnState() != tCQCGKit::EConnStates::Connected)
        return tCIDCtrls::EEvResponses::Handled;

    try
    {
        // Get the list index, if any
        const tCIDLib::TCard4 c4ListInd = m_pwndUnits->c4CurItem();

        // Use that to get the unit name, if one was selected, and find the unit
        TString strUnitName;
        TZWaveUnit* punitTar = nullptr;
        if (c4ListInd != kCIDLib::c4MaxCard)
        {
            m_pwndUnits->QueryColText(c4ListInd, 0, strUnitName);
            punitTar = m_dcfgEdit.punitFindByName(strUnitName);
        }

        // According to the button id, we do the right thing
        const tCIDCtrls::TWndId widSrc = wnotEvent.widSource();
        if (widSrc == kZWaveUSBC::ridIntf_Main_UnitEdit)
        {
            // Let the user edit this unit
            if (punitTar)
                EditUnit(c4ListInd);
        }
         else if (widSrc == kZWaveUSBC::ridIntf_Main_Learn)
        {
            tZWaveUSBSh::ERepRes eToFill;
            if (!bDrvMode(eToFill))
            {
                // Invoke a dialog box that does the replication operation
                TZWaveLearnDlg dlgLearn;
                if (dlgLearn.bRunDlg(*this, strMoniker()))
                {
                    // There may be new units and gruops so update again
                    TWndPtrJanitor janPtr(tCIDCtrls::ESysPtrs::Wait);
                    bGetConnectData(orbcServer());
                }
            }
             else
            {
                TOkBox msgbWarn
                (
                    facZWaveUSBC().strMsg(kZWCMsgs::midGen_Title)
                    , facZWaveUSBC().strMsg(kZWCMsgs::midStatus_AlreadyRepMode)
                );
                msgbWarn.ShowIt(*this);
            }
        }
         else if (widSrc == kZWaveUSBC::ridIntf_Main_Reset)
        {
            // Make sure that they want to do this
            TYesNoBox msgbAsk
            (
                facZWaveUSBC().strMsg(kZWCMsgs::midGen_Title)
                , facZWaveUSBC().strMsg(kZWCMsgs::midQ_Reset)
            );

            if (msgbAsk.bShowIt(*this))
            {
                // Copy the last config to the edit config and reload
                m_dcfgEdit = m_dcfgLast;
                LoadConfig();
            }
        }
    }

    catch(TError& errToCatch)
    {
        if (facZWaveUSBC().bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        TExceptDlg dlgbFail
        (
            *this
            , facZWaveUSBC().strMsg(kZWCMsgs::midGen_Title)
            , facZWaveUSBC().strMsg(kZWCMsgs::midStatus_RemOpFailed)
            , errToCatch
        );
    }
    return tCIDCtrls::EEvResponses::Handled;
}


//
//  We allow the user to double click on groups or units to edit them.
//
tCIDCtrls::EEvResponses TZWaveUSBCWnd::eLBHandler(TListChangeInfo& wnotEvent)
{
    if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::Invoked)
    {
        if (wnotEvent.widSource() == kZWaveUSBC::ridIntf_Main_Units)
            EditUnit(wnotEvent.c4Index());
    }
     else if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::DeleteReq)
    {
    }
    return tCIDCtrls::EEvResponses::Handled;
}


// We have to do this in two places, so split it out to avoid redundancy
tCIDLib::TVoid TZWaveUSBCWnd::EditUnit(const tCIDLib::TCard4 c4ListInd)
{
    TString strUnitName;
    m_pwndUnits->QueryColText(c4ListInd, 0, strUnitName);
    TZWaveUnit& unitTar = m_dcfgEdit.unitFindByName(strUnitName);

    TZWaveEdUnitDlg dlgEdUnit;
    if (dlgEdUnit.bRunDlg(*this, strMoniker(), unitTar))
    {
        // The saved changes, so update our window info
        m_pwndUnits->SetColText(c4ListInd, 0, unitTar.strName());

        TString strFmt;
        strFmt.SetFormatted(unitTar.enctPollPeriod() / kCIDLib::enctOneSecond);
        strFmt.Append(L" secs");
        m_pwndUnits->SetColText(c4ListInd, 1, strFmt);
    }
}



//
//  Asks the server side driver for the list of groups and units, which we
//  load into the list boxes. We sort the lists by name, and load them in
//  that order, so the index in the list is the index back into the lists.
//
tCIDLib::TVoid TZWaveUSBCWnd::LoadConfig()
{
    tZWaveUSBSh::TUnitList& colUnits = m_dcfgEdit.colUnits();
    const tCIDLib::TCard4 c4UnitCnt = colUnits.c4ElemCount();

    tCIDLib::TStrList colCols(2);
    colCols.objAdd(TString::strEmpty());
    colCols.objAdd(TString::strEmpty());

    m_pwndUnits->RemoveAll();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4UnitCnt; c4Index++)
    {
        TZWaveUnit* punitCur = colUnits[c4Index];

        colCols[0] = punitCur->strName();
        colCols[1].Clear();
        if (punitCur->enctPollPeriod())
        {
            colCols[1].SetFormatted
            (
                punitCur->enctPollPeriod() / kCIDLib::enctOneSecond
            );
            colCols[1].Append(L" secs");
        }
         else
        {
            colCols[1].Append(L" - -");
        }

        m_pwndUnits->c4AddItem(colCols, 0);
    }
}

