//
// FILE NAME: ZWaveLeviC_Window.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/14/2012
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
//  This file implements the main window of the Leviton ZWave client driver
//
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "ZWaveLeviC_.hpp"


// ---------------------------------------------------------------------------
//  Magic macro
// ---------------------------------------------------------------------------
RTTIDecls(TZWaveLeviCWnd,TCQCDriverClient)



// ---------------------------------------------------------------------------
//   CLASS: TZWaveLeviCWnd
//  PREFIX: wnd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWaveLeviCWnd: Constructors and Destructor
// ---------------------------------------------------------------------------
TZWaveLeviCWnd::TZWaveLeviCWnd( const   TCQCDriverObjCfg&   cqcdcThis
                                , const TCQCUserCtx&        cuctxToUse) :

    TCQCDriverClient
    (
        cqcdcThis, L"TZWaveLeviCWnd", tCQCKit::EActLevels::Low, cuctxToUse
    )
    , m_bNewConfig(kCIDLib::False)
    , m_c4LastSerialNum(0)
    , m_pwndAddAssoc(nullptr)
    , m_pwndAsyncNot(nullptr)
    , m_pwndBattPwr(nullptr)
    , m_pwndDelete(nullptr)
    , m_pwndDisabled(nullptr)
    , m_pwndEdGroups(nullptr)
    , m_pwndInfoGrp(nullptr)
    , m_pwndOptInfo(nullptr)
    , m_pwndPollInt(nullptr)
    , m_pwndReadable(nullptr)
    , m_pwndRename(nullptr)
    , m_pwndRescan(nullptr)
    , m_pwndSecure(nullptr)
    , m_pwndSetCfgParm(nullptr)
    , m_pwndUnits(nullptr)
    , m_pwndUnitId(nullptr)
    , m_pwndWriteable(nullptr)
{
}

TZWaveLeviCWnd::~TZWaveLeviCWnd()
{
}


// ---------------------------------------------------------------------------
//  TZWaveLeviCWnd: Public, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TZWaveLeviCWnd::bChanges() const
{
    return (m_dcfgCur != m_dcfgPrev);
}

tCIDLib::TBoolean TZWaveLeviCWnd::bSaveChanges(TString& strErrMsg)
{
    // If no changes, then nothing to do
    if (m_dcfgPrev == m_dcfgCur)
        return kCIDLib::True;

    // Let's upload the configuration to the server side driver
    try
    {
        // Flatten it down to a buffer and upload that
        tCIDLib::TCard4 c4Bytes;
        THeapBuf mbufCfg(8192UL);
        {
            TBinMBufOutStream strmCfg(&mbufCfg);
            strmCfg << m_dcfgCur << kCIDLib::FlushIt;
            c4Bytes = strmCfg.c4CurSize();
        }

        orbcServer()->SetConfig
        (
            strMoniker()
            , c4Bytes
            , mbufCfg
            , cuctxToUse().sectUser()
            , tCQCKit::EDrvCmdWaits::NoWait
        );

        // It is queued up, so copy the current stuff to the previous config copy.
        m_dcfgPrev = m_dcfgCur;
        m_pwndUnits->TakeFocus();

        // And we can re-enable the config and assoc buttons
        m_pwndAddAssoc->SetEnable(kCIDLib::True);
        m_pwndSetCfgParm->SetEnable(kCIDLib::True);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        strErrMsg = errToCatch.strErrText();
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


// ---------------------------------------------------------------------------
//  TZWaveLeviCWnd: Protected, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TZWaveLeviCWnd::bCreated()
{
    TParent::bCreated();

    // Get the dialog description we'll use to create the main controls
    TDlgDesc dlgdMain;
    facZWaveLeviC().bCreateDlgDesc(kZWaveLeviC::ridMain_Intf, dlgdMain);

    // And now create the controls
    tCIDCtrls::TWndId widInitFocus;
    CreateDlgItems(dlgdMain, widInitFocus);

    // Get typed pointers to the widgets we want to interact with a lot
    CastChildWnd(*this, kZWaveLeviC::ridMain_AddAssoc, m_pwndAddAssoc);
    CastChildWnd(*this, kZWaveLeviC::ridMain_AsyncNot, m_pwndAsyncNot);
    CastChildWnd(*this, kZWaveLeviC::ridMain_BattPwr, m_pwndBattPwr);
    CastChildWnd(*this, kZWaveLeviC::ridMain_Delete, m_pwndDelete);
    CastChildWnd(*this, kZWaveLeviC::ridMain_Disabled, m_pwndDisabled);
    CastChildWnd(*this, kZWaveLeviC::ridMain_Groups, m_pwndEdGroups);
    CastChildWnd(*this, kZWaveLeviC::ridMain_UnitInfo, m_pwndInfoGrp);
    CastChildWnd(*this, kZWaveLeviC::ridMain_OptInfo, m_pwndOptInfo);
    CastChildWnd(*this, kZWaveLeviC::ridMain_PollInt, m_pwndPollInt);
    CastChildWnd(*this, kZWaveLeviC::ridMain_Readable, m_pwndReadable);
    CastChildWnd(*this, kZWaveLeviC::ridMain_Rename, m_pwndRename);
    CastChildWnd(*this, kZWaveLeviC::ridMain_Rescan, m_pwndRescan);
    CastChildWnd(*this, kZWaveLeviC::ridMain_Secure, m_pwndSecure);
    CastChildWnd(*this, kZWaveLeviC::ridMain_SetCfgParm, m_pwndSetCfgParm);
    CastChildWnd(*this, kZWaveLeviC::ridMain_Units, m_pwndUnits);
    CastChildWnd(*this, kZWaveLeviC::ridMain_UnitId, m_pwndUnitId);
    CastChildWnd(*this, kZWaveLeviC::ridMain_Writeable, m_pwndWriteable);

    // Set up our columns and marked colors for the unit list
    tCIDLib::TStrList colCols(1);
    colCols.objAdd(facZWaveLeviC().strMsg(kZWCMsgs::midMain_ListPref));
    m_pwndUnits->SetColumns(colCols);
    m_pwndUnits->SetMarkedColors(TRGBClr(0xFF, 0, 0), TRGBClr(0xFF, 0xFF, 0xFF));

    //
    //  Load the poll interval list with number of seconds starting at 3
    //  Put in a disabled entry first though.
    //
    tCIDLib::TStrList colVals(kZWaveLeviSh::c4MaxPollSecs);
    TString strFmt;
    colVals.objAdd(L"Disabled");
    for (tCIDLib::TCard4 c4Secs = 3; c4Secs <= kZWaveLeviSh::c4MaxPollSecs; c4Secs++)
    {
        strFmt.SetFormatted(c4Secs);
        colVals.objAdd(strFmt);
    }
    m_pwndPollInt->SetEnumList(colVals);

    // Register event handlers
    m_pwndAddAssoc->pnothRegisterHandler(this, &TZWaveLeviCWnd::eClickHandler);
    m_pwndAsyncNot->pnothRegisterHandler(this, &TZWaveLeviCWnd::eClickHandler);
    m_pwndBattPwr->pnothRegisterHandler(this, &TZWaveLeviCWnd::eClickHandler);
    m_pwndDelete->pnothRegisterHandler(this, &TZWaveLeviCWnd::eClickHandler);
    m_pwndDisabled->pnothRegisterHandler(this, &TZWaveLeviCWnd::eClickHandler);
    m_pwndEdGroups->pnothRegisterHandler(this, &TZWaveLeviCWnd::eClickHandler);
    m_pwndReadable->pnothRegisterHandler(this, &TZWaveLeviCWnd::eClickHandler);
    m_pwndRename->pnothRegisterHandler(this, &TZWaveLeviCWnd::eClickHandler);
    m_pwndRename->pnothRegisterHandler(this, &TZWaveLeviCWnd::eClickHandler);
    m_pwndRescan->pnothRegisterHandler(this, &TZWaveLeviCWnd::eClickHandler);
    m_pwndSecure->pnothRegisterHandler(this, &TZWaveLeviCWnd::eClickHandler);
    m_pwndSetCfgParm->pnothRegisterHandler(this, &TZWaveLeviCWnd::eClickHandler);
    m_pwndWriteable->pnothRegisterHandler(this, &TZWaveLeviCWnd::eClickHandler);

    m_pwndOptInfo->pnothRegisterHandler(this, &TZWaveLeviCWnd::eLBHandler);
    m_pwndPollInt->pnothRegisterHandler(this, &TZWaveLeviCWnd::eSpinHandler);
    m_pwndUnits->pnothRegisterHandler(this, &TZWaveLeviCWnd::eLBHandler);

    return kCIDLib::True;
}



tCIDLib::TBoolean
TZWaveLeviCWnd::bGetConnectData(tCQCKit::TCQCSrvProxy& orbcTarget)
{
    m_bNewConfig = kCIDLib::False;
    m_c4LastSerialNum = 0;

    // Do any initial load of any config
    try
    {
        //
        //  Query the driver configuration. If we get it, then store it
        //  away.
        //
        QueryConfig(orbcTarget, m_dcfgNew, m_c4NewSerialNum);
        LoadConfig(orbcTarget, m_dcfgNew, m_c4NewSerialNum);

        //
        //  Initially the buttons that are related to either changes to save, or
        //  a selected unit to affect, are disabled.
        //
        CheckForChanges();
    }

    catch(...)
    {
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


//
//  We just check to see if the driver is there there, and if so what his
//  current configuration serial number is.
//
tCIDLib::TBoolean TZWaveLeviCWnd::bDoPoll(tCQCKit::TCQCSrvProxy& orbcTarget)
{
    tCIDLib::TBoolean bRet = kCIDLib::False;
    try
    {
        const tCIDLib::TCard4 c4SerialNum = orbcServer()->c4QueryVal
        (
            strMoniker(), kZWaveLeviSh::strDrvQ_ConfigSerialNum
        );

        //
        //  If the serial number has changed, then try to query the
        //  current configuration data. If we get it stored away, then
        //  set the new config flag.
        //
        if (c4SerialNum != m_c4LastSerialNum)
        {
            QueryConfig(orbcTarget, m_dcfgNew, m_c4NewSerialNum);
            m_bNewConfig = kCIDLib::True;

            // Return true so UpdateDisplayData will be called
            bRet = kCIDLib::True;
        }
    }

    catch(TError& errToCatch)
    {
        if (facZWaveLeviC().bLogWarnings() && !errToCatch.bLogged())
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
        throw;
    }
    return bRet;
}


tCIDLib::TVoid TZWaveLeviCWnd::Connected()
{
}



tCIDLib::TVoid TZWaveLeviCWnd::DoCleanup()
{
}


tCIDLib::TVoid TZWaveLeviCWnd::LostConnection()
{
}


//
//  If the poll thread indicates we have new data, then let's load it
//  up. Clear the new data flag first, so that if something goes wrong we
//  don't get stuck in an endless loop.
//
tCIDLib::TVoid TZWaveLeviCWnd::UpdateDisplayData()
{
    if (m_bNewConfig)
    {
        m_bNewConfig = kCIDLib::False;
        LoadConfig(orbcServer(), m_dcfgNew, m_c4NewSerialNum);
    }
}


// ---------------------------------------------------------------------------
//  TZWaveLeviCWnd: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  After any change or suspected change even, this is called to see if need enable or
//  disable any controls. Even a change might have undone a previous change, meaning
//  we don't have any changes to save again.
//
//  We enable/disable the set config and set assoc buttons when changes have been made
//  since the info we have might not match the server's in that case.
//
//  If we have no units in the list, we also diable the delete and rename buttons.
//
tCIDLib::TVoid TZWaveLeviCWnd::CheckForChanges()
{
    if (m_dcfgCur == m_dcfgPrev)
    {
        m_pwndAddAssoc->SetEnable(kCIDLib::True);
        m_pwndSetCfgParm->SetEnable(kCIDLib::True);
    }
     else
    {
        m_pwndAddAssoc->SetEnable(kCIDLib::False);
        m_pwndSetCfgParm->SetEnable(kCIDLib::False);
    }

    //
    //  Enable/disable the rename and delete buttons depending on whether
    //  we have units or not
    //
    m_pwndDelete->SetEnable(m_dcfgCur.c4UnitCnt() != 0);
    m_pwndRename->SetEnable(m_dcfgCur.c4UnitCnt() != 0);
}



//
//  This is called on a button click
//
tCIDCtrls::EEvResponses
TZWaveLeviCWnd::eClickHandler(TButtClickInfo& wnotEvent)
{
    //
    //  If we are not connected, then do nothing.
    //
    if (eConnState() != tCQCGKit::EConnStates::Connected)
        return tCIDCtrls::EEvResponses::Handled;

    //
    //  Don't allow timers while we are in here, so that the configuration
    //  cannot change behind our backs. If we allowed timers, then the
    //  poll callback could happen as soon as we enter a dialog and start
    //  processing messages again.
    //
    TTimerSusJanitor janTimers(this);

    // Some require a unit be selected, so get the current selection
    const tCIDLib::TCard4 c4ListInd = m_pwndUnits->c4CurItem();
    try
    {
        //
        //  Some don't require a currently selected unit, while the rest
        //  of them do, so split them out separately.
        //
        if (wnotEvent.widSource() == kZWaveLeviC::ridMain_AddAssoc)
        {
            // Invoke the association setting dialog if the unit list isn't emty
            if (m_dcfgCur.c4UnitCnt())
            {
                TZWaveAddAssocDlg dlgAddAssoc;
                dlgAddAssoc.Run(*this, strMoniker(), m_dcfgCur);
            }
        }
         else if (wnotEvent.widSource() == kZWaveLeviC::ridMain_Groups)
        {
            // Call the group editor dialog
            TZWaveEdGroupsDlg dlgGroups;
            if (dlgGroups.bRun(*this, strMoniker(), m_dcfgCur))
                CheckForChanges();
        }
         else if (wnotEvent.widSource() == kZWaveLeviC::ridMain_Rescan)
        {
            // Just send the driver a command to start a new scan
            try
            {
                orbcServer()->c4SendCmd
                (
                    strMoniker()
                    , kZWaveLeviSh::strDrvCmd_StartScan
                    , TString::strEmpty()
                );

                TOkBox msgbOk
                (
                    facZWaveLeviC().strMsg(kZWCMsgs::midGen_Title)
                    , facZWaveLeviC().strMsg(kZWCMsgs::midStatus_ScanStarted)
                );
                msgbOk.ShowIt(*this);
            }

            catch(TError& errToCatch)
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);

                TExceptDlg dlgbFail
                (
                    *this
                    , facZWaveLeviC().strMsg(kZWCMsgs::midGen_Title)
                    , facZWaveLeviC().strMsg(kZWCMsgs::midStatus_Rescan)
                    , errToCatch
                );
            }
        }
         else if (wnotEvent.widSource() == kZWaveLeviC::ridMain_SetCfgParm)
        {
            // Invoke the configuration parameter setting dialog, if any units
            if (m_dcfgCur.c4UnitCnt())
            {
                TZWaveSetCfgParamDlg dlgSetCP;
                dlgSetCP.Run(*this, strMoniker(), m_dcfgCur);
            }
        }
         else if (c4ListInd != kCIDLib::c4MaxCard)
        {
            // All these require a unit be selected
            const tCIDLib::TCard4 c4UnitId = m_pwndUnits->c4IndexToId(c4ListInd);
            TZWaveUnit& unitTar = m_dcfgCur.unitFindById(c4UnitId);

            if (wnotEvent.widSource() == kZWaveLeviC::ridMain_AsyncNot)
            {
                if (m_pwndAsyncNot->bCheckedState())
                {
                    unitTar.SetCapFlags
                    (
                        tZWaveLeviSh::EUnitCap_AsyncNotify, kCIDLib::True
                    );

                    //
                    //  Set the poll interval to the maximum, which will be
                    //  the actual maximum minus two (we start with three
                    //  and one is the disabled entry.)
                    //
                    m_pwndPollInt->SetSpinIndex(kZWaveLeviSh::c4MaxPollSecs - 2);
                    m_pwndPollInt->SetEnable(kCIDLib::False);
                    unitTar.enctPollPeriod(kZWaveLeviSh::c4MaxPollSecs * kCIDLib::enctOneSecond);
                }
                 else
                {
                    unitTar.SetCapFlags
                    (
                        tZWaveLeviSh::EUnitCap_AsyncNotify, kCIDLib::False
                    );

                    //
                    //  See if this unit is there in the previous config.
                    //  If so, put the poll time back to that, since it will
                    //  have been forced to max time when the async was
                    //  enabled.
                    //
                    TZWaveUnit* punitPrev = m_dcfgPrev.punitFindByName(unitTar.strName());

                    if (punitPrev)
                    {
                        unitTar.enctPollPeriod(punitPrev->enctPollPeriod());
                        SetPollInt(unitTar);
                    }
                }
            }
             else if (wnotEvent.widSource() == kZWaveLeviC::ridMain_BattPwr)
            {
                unitTar.SetCapFlags
                (
                    tZWaveLeviSh::EUnitCap_Battery, m_pwndBattPwr->bCheckedState()
                );
            }
             else if (wnotEvent.widSource() == kZWaveLeviC::ridMain_Delete)
            {
                // Make sure they want to do it
                TYesNoBox msgbDel
                (
                    facZWaveLeviC().strMsg(kZWCMsgs::midQ_DelUnit, unitTar.strName())
                );

                if (msgbDel.bShowIt(*this))
                {
                    m_dcfgCur.DeleteUnit(&unitTar);
                    m_pwndUnits->RemoveAt(c4ListInd);
                    CheckForChanges();
                }
            }
             else if (wnotEvent.widSource() == kZWaveLeviC::ridMain_Disabled)
            {
                unitTar.bDisabled(m_pwndDisabled->bCheckedState());
            }
             else if (wnotEvent.widSource() == kZWaveLeviC::ridMain_Readable)
            {
                unitTar.SetCapFlags
                (
                    tZWaveLeviSh::EUnitCap_Readable, m_pwndReadable->bCheckedState()
                );
            }
             else if (wnotEvent.widSource() == kZWaveLeviC::ridMain_Rename)
            {
                EditUnit(unitTar);
            }
             else if (wnotEvent.widSource() == kZWaveLeviC::ridMain_Secure)
            {
                unitTar.SetCapFlags
                (
                    tZWaveLeviSh::EUnitCap_Secure, m_pwndSecure->bCheckedState()
                );
            }
             else if (wnotEvent.widSource() == kZWaveLeviC::ridMain_Writeable)
            {
                unitTar.SetCapFlags
                (
                    tZWaveLeviSh::EUnitCap_Writeable, m_pwndWriteable->bCheckedState()
                );
            }
        }

        //
        //  Check for changes, or lack of them if we did a save, and get
        //  the Save buton correct enabled/disabled.
        //
        CheckForChanges();
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        TExceptDlg dlgbFail
        (
            *this
            , facZWaveLeviC().strMsg(kZWCMsgs::midGen_Title)
            , facZWaveLeviC().strMsg(kZWCMsgs::midStatus_RemOpFailed)
            , errToCatch
        );
    }
    return tCIDCtrls::EEvResponses::Handled;
}


//
//  When the selection changes, we update the displayed unit info.
//
tCIDCtrls::EEvResponses TZWaveLeviCWnd::eLBHandler(TListChangeInfo& wnotEvent)
{
    //
    //  Don't allow timers while we are in here, so that the configuration
    //  cannot change behind our backs. If we allowed timers, then the
    //  poll callback could happen as soon as we enter a dialog and start
    //  processing messages again.
    //
    TTimerSusJanitor janTimers(this);

    const tCIDLib::TCard4 c4ListInd = m_pwndUnits->c4CurItem();

    if (wnotEvent.widSource() == kZWaveLeviC::ridMain_Units)
    {
        if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::Cleared)
        {
            SetUnitSelState(kCIDLib::False);
        }
         else
        {
            const tCIDLib::TCard4 c4UnitId = m_pwndUnits->c4IndexToId(c4ListInd);
            TZWaveUnit& unitTar = m_dcfgCur.unitFindById(c4UnitId);

            if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::SelChanged)
            {
                // Show the newly selected unit configuration
                ShowUnit(unitTar.strName());
            }
             else if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::Invoked)
            {
                // Let the user edit the name
                EditUnit(unitTar);

                // Check for changes
                CheckForChanges();
            }
        }
    }
     else if (wnotEvent.widSource() == kZWaveLeviC::ridMain_OptInfo)
    {
        // If a selection is made in the optional info combo, update the selected unit.
        if ((c4ListInd != kCIDLib::c4MaxCard)
        &&  (wnotEvent.eEvent() == tCIDCtrls::EListEvents::SelChanged))
        {
            const tCIDLib::TCard4 c4UnitId = m_pwndUnits->c4IndexToId(c4ListInd);
            TZWaveUnit& unitTar = m_dcfgCur.unitFindById(c4UnitId);
            unitTar.c4OptInfo(wnotEvent.c4Index());

            // Check for changes
            CheckForChanges();
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}


// We update the poll interval when the spin box changes
tCIDCtrls::EEvResponses TZWaveLeviCWnd::eSpinHandler(TSpinChangeInfo& wnotEvent)
{
    //
    //  Don't allow timers while we are in here, so that the configuration
    //  cannot change behind our backs. If we allowed timers, then the
    //  poll callback could happen as soon as we enter a dialog and start
    //  processing messages again.
    //
    TTimerSusJanitor janTimers(this);

    // If there's a unit selected update it appropriately
    const tCIDLib::TCard4 c4ListInd = m_pwndUnits->c4CurItem();
    if (c4ListInd != kCIDLib::c4MaxCard)
    {
        const tCIDLib::TCard4 c4UnitId = m_pwndUnits->c4IndexToId(c4ListInd);
        TZWaveUnit& unitTar = m_dcfgCur.unitFindById(c4UnitId);
        if (wnotEvent.widSource() == kZWaveLeviC::ridMain_PollInt)
        {
            tCIDLib::TEncodedTime enctNew = 0;
            if (wnotEvent.i4Value())
                 enctNew = (2 + wnotEvent.i4Value()) * kCIDLib::enctOneSecond;

            //
            //  Store the new value and check to see if we have changes
            //  to save.
            //
            unitTar.enctPollPeriod(enctNew);
            CheckForChanges();
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}


//
//  Called from a double click on a unit or if they press the Rename button
//  whena  unit is selected. It just lets them rename it.
//
tCIDLib::TVoid TZWaveLeviCWnd::EditUnit(TZWaveUnit& unitTar)
{
    // We can use a standard dialog here
    TString strNew;
    const tCIDLib::TBoolean bRes = facCIDWUtils().bGetText2
    (
        *this
        , facZWaveLeviC().strMsg(kZWCMsgs::midPrompt_UnitName)
        , facZWaveLeviC().strMsg(kZWCMsgs::midPrompt_UnitNameInstruct)
        , unitTar.strName()
        , strNew
        , L"[a-zA-Z][a-zA-Z0-9-_]*"
    );

    if (bRes && !strNew.bIsEmpty() && (strNew != unitTar.strName()))
    {
        //
        //  Make sure it's not an existing name. We know it's not the same as the
        //  original.
        //
        if (m_dcfgCur.bUnitExists(strNew))
        {
            TErrBox msgbDup(facZWaveLeviSh().strMsg(kZWShErrs::errcCfg_DupUnit, strNew));
            msgbDup.ShowIt(*this);
        }
         else
        {
            // Looks OK, so update the unit, and the list box item.
            unitTar.strName(strNew);
            m_pwndUnits->SetColText(m_pwndUnits->c4CurItem(), 0, strNew);
        }
    }
}


//
//  Asks the server side driver for the list of groups and units, which we
//  load into the list boxes. We sort the lists by name, and load them in
//  that order, so the index in the list is the index back into the lists.
//
tCIDLib::TVoid
TZWaveLeviCWnd::LoadConfig(         tCQCKit::TCQCSrvProxy&  orbcSrv
                            , const TZWaveDrvConfig&        dcfgNew
                            , const tCIDLib::TCard4         c4SerialNum)
{
    CIDAssert(&dcfgNew != &m_dcfgCur, L"Load config with same object");

    // Store the new data
    m_dcfgCur = dcfgNew;
    m_c4LastSerialNum = c4SerialNum;

    // Sort the unit list before we store it as the previous config
    m_dcfgCur.SortUnitList();

    //
    //  Save the current configuration as the previous config now. This
    //  is used to watch for changed.
    //
    m_dcfgPrev = m_dcfgCur;

    // And load it up
    tCIDLib::TStrList colCols(1);
    colCols.objAdd(TString::strEmpty());
    m_pwndUnits->RemoveAll();
    const tCIDLib::TCard4 c4UnitCnt = m_dcfgCur.c4UnitCnt();
    if (c4UnitCnt)
    {
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4UnitCnt; c4Index++)
        {
            const TZWaveUnit& unitCur = m_dcfgCur.unitAt(c4Index);

            // Store the unit is as the per-row id
            colCols[0] = unitCur.strName();
            const tCIDLib::TCard4 c4At = m_pwndUnits->c4AddItem(colCols, unitCur.c4Id());

            // If the unit is marked as missing, mark it
            if (unitCur.eStatus() == tZWaveLeviSh::EUnitStatus_Missing)
                m_pwndUnits->SetFlagAt(c4At, kCIDLib::True);
        }
        m_pwndUnits->SelectByIndex(0, kCIDLib::True);
    }
}


//
//  Query the configuration from the server and store it away in the passed
//  parameters.
//
tCIDLib::TVoid
TZWaveLeviCWnd::QueryConfig(tCQCKit::TCQCSrvProxy&  orbcSrv
                            , TZWaveDrvConfig&      dcfgToFill
                            , tCIDLib::TCard4&      c4NewSerialNum)
{
    THeapBuf mbufCfg(8192UL);
    tCIDLib::TCard4 c4Sz;
    orbcSrv->QueryConfig(strMoniker(), c4Sz, mbufCfg);

    // Make an input stream over the data and stream in the config
    TBinMBufInStream strmIn(&mbufCfg, c4Sz);
    strmIn.CheckForStartMarker(CID_FILE, CID_LINE);
    strmIn  >> dcfgToFill
            >> c4NewSerialNum;
    strmIn.CheckForEndMarker(CID_FILE, CID_LINE);

}


//
//  Given a unit, set up the poll interval spin box appropriately.
//
tCIDLib::TVoid TZWaveLeviCWnd::SetPollInt(const TZWaveUnit& unitSrc)
{
    //
    //  If async is enabled, then disable the poll interval spin box,
    //  and set it to the max poll period.
    //
    if (unitSrc.bHasCap(tZWaveLeviSh::EUnitCap_AsyncNotify))
    {
        m_pwndPollInt->SetSpinIndex(0);
        m_pwndPollInt->SetEnable(kCIDLib::False);
    }
     else
    {
        m_pwndPollInt->SetEnable(kCIDLib::True);

        if (unitSrc.enctPollPeriod())
        {
            tCIDLib::TCard4 c4Secs = tCIDLib::TCard4
            (
                unitSrc.enctPollPeriod() / kCIDLib::enctOneSecond
            );

            if (c4Secs < 3)
                c4Secs = 3;
            else if (c4Secs > kZWaveLeviSh::c4MaxPollSecs)
                c4Secs = kZWaveLeviSh::c4MaxPollSecs;

            //
            //  The number of seconds minus two is the right one (0th is the
            //  disabled setting and we started at 3.)
            //
            m_pwndPollInt->SetSpinIndex(c4Secs - 2);
        }
         else
        {
            // We can just select the 'disabled' item which always at 0
            m_pwndPollInt->SetSpinIndex(0);
        }
    }
}


//
//  There is some stuff we have to do based on whether there's a unit selected
//  or not (or it's missing.)
//
tCIDLib::TVoid
TZWaveLeviCWnd::SetUnitSelState(const tCIDLib::TBoolean bState)
{
    m_pwndInfoGrp->strWndText(facZWaveLeviC().strMsg(kZWCMsgs::midMain_UnitInfo));

    if (bState)
    {
        m_pwndDisabled->SetEnable(kCIDLib::True);
    }
     else
    {
        m_pwndUnitId->ClearText();

        m_pwndAsyncNot->SetCheckedState(kCIDLib::False);
        m_pwndBattPwr->SetCheckedState(kCIDLib::False);
        m_pwndDisabled->SetCheckedState(kCIDLib::False);
        m_pwndPollInt->SetSpinIndex(0);
        m_pwndReadable->SetCheckedState(kCIDLib::False);
        m_pwndSecure->SetCheckedState(kCIDLib::False);
        m_pwndWriteable->SetCheckedState(kCIDLib::False);

        m_pwndAsyncNot->SetEnable(kCIDLib::False);
        m_pwndBattPwr->SetEnable(kCIDLib::False);
        m_pwndDisabled->SetEnable(kCIDLib::False);
        m_pwndOptInfo->SetVisibility(kCIDLib::False);
        m_pwndPollInt->SetEnable(kCIDLib::False);
        m_pwndReadable->SetEnable(kCIDLib::False);
        m_pwndSecure->SetEnable(kCIDLib::False);
        m_pwndWriteable->SetEnable(kCIDLib::False);
    }
}



// When a unit is selected, this is called to load up its info for editing
tCIDLib::TVoid TZWaveLeviCWnd::ShowUnit(const TString& strName)
{
    TString strFmt;

    // Find the unit in the list
    TZWaveUnit& unitShow = m_dcfgCur.unitFindByName(strName);

    //
    //  If this unit is missing, we act like there's nothing selected on
    //  the right side.
    //
    SetUnitSelState(unitShow.eStatus() != tZWaveLeviSh::EUnitStatus_Missing);

    // Update the unit id display
    strFmt.SetFormatted(unitShow.c4Id());
    strFmt.Append(L" (0x");
    strFmt.AppendFormatted(unitShow.c4Id(), tCIDLib::ERadices::Hex);
    strFmt.Append(L")");
    m_pwndUnitId->strWndText(strFmt);

    if (unitShow.eStatus() == tZWaveLeviSh::EUnitStatus_Missing)
    {
        // Update the group box text to indicate it's missing
        m_pwndInfoGrp->strWndText
        (
            facZWaveLeviC().strMsg(kZWCMsgs::midStatus_MissingUnit)
        );
    }
     else
    {
        // Make sure the group box text is back to normal
        m_pwndInfoGrp->strWndText(facZWaveLeviC().strMsg(kZWCMsgs::midMain_UnitInfo));

        m_pwndAsyncNot->SetCheckedState(unitShow.bHasCap(tZWaveLeviSh::EUnitCap_AsyncNotify));
        m_pwndBattPwr->SetCheckedState(unitShow.bHasCap(tZWaveLeviSh::EUnitCap_Battery));
        m_pwndDisabled->SetCheckedState(unitShow.bDisabled());
        m_pwndReadable->SetCheckedState(unitShow.bHasCap(tZWaveLeviSh::EUnitCap_Readable));
        m_pwndSecure->SetCheckedState(unitShow.bHasCap(tZWaveLeviSh::EUnitCap_Secure));
        m_pwndWriteable->SetCheckedState(unitShow.bHasCap(tZWaveLeviSh::EUnitCap_Writeable));

        //
        //  Any bits that are in the fixed caps need to be disabled, since we don't
        //  allow them to be modified. So whatever the initial value set by the
        //  unit will be retained.
        //
        m_pwndAsyncNot->SetEnable(!unitShow.bHasFixedCap(tZWaveLeviSh::EUnitCap_AsyncNotify));
        m_pwndBattPwr->SetEnable(!unitShow.bHasFixedCap(tZWaveLeviSh::EUnitCap_Battery));
        m_pwndReadable->SetEnable(!unitShow.bHasFixedCap(tZWaveLeviSh::EUnitCap_Readable));
        m_pwndSecure->SetEnable(!unitShow.bHasFixedCap(tZWaveLeviSh::EUnitCap_Secure));
        m_pwndWriteable->SetEnable(!unitShow.bHasFixedCap(tZWaveLeviSh::EUnitCap_Writeable));

        // Set up the poll interval spin box. This one is a bit more work
        SetPollInt(unitShow);

        // If it has optional info, show that, else hide the combo
        tCIDLib::TStrList colOptVals;
        if (unitShow.bWantsOptInfo(colOptVals))
        {
            m_pwndOptInfo->SetVisibility(kCIDLib::True);

            m_pwndOptInfo->RemoveAll();
            const tCIDLib::TCard4 c4Count= colOptVals.c4ElemCount();
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
                m_pwndOptInfo->c4AddItem(colOptVals[c4Index]);

            // Initially select the previous value if valid
            tCIDLib::TCard4 c4InitSel = 0;
            if (unitShow.c4OptInfo() < c4Count)
                c4InitSel = unitShow.c4OptInfo();
            m_pwndOptInfo->SelectByIndex(c4InitSel);
        }
         else
        {
            m_pwndOptInfo->SetVisibility(kCIDLib::False);
            m_pwndOptInfo->RemoveAll();
        }
    }
}

