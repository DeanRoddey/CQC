//
// FILE NAME: CQCDrvDev_FldMonWnd.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/23/2003
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
//  This file implements the TCQCFldMonWnd class, which provides a means
//  for users of the driver test harness to see the current fields defined
//  by the driver, to see their values and attributes, and to modify the
//  values in order to simulate client writes operations for testing.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


// ---------------------------------------------------------------------------
//  Facility specific includes
// ---------------------------------------------------------------------------
#include    "CQCDrvDev.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCFldMonWnd,TFrameWnd)


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace
{
    namespace CQCDrvDev_FldMonWnd
    {
        constexpr   tCIDLib::TCard4         c4TimerPeriod = 50;
    }
}



// ---------------------------------------------------------------------------
//  CLASS: TCQCFldMonWnd
// PREFIX: wnd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCFldMonWnd: Constructors and destructor
// ---------------------------------------------------------------------------
TCQCFldMonWnd::TCQCFldMonWnd() :

    m_bLastTime(kCIDLib::False)
    , m_c4DrvSerialNum(0)
    , m_c4FldListId(0)
    , m_c4FldSerialNum(0)
    , m_eCurState(tCQCKit::EDrvStates::Count)
    , m_pdcmdWait(nullptr)
    , m_pwndOwner(nullptr)
    , m_pwndAccess(nullptr)
    , m_pwndChange(nullptr)
    , m_pwndLimits(nullptr)
    , m_pwndList(nullptr)
    , m_pwndName(nullptr)
    , m_pwndQText(nullptr)
    , m_pwndSendCmd(nullptr)
    , m_pwndStatus(nullptr)
    , m_pwndType(nullptr)
    , m_pwndValue(nullptr)
{
    // Set up or time that we use for formatting out time fields
    m_tmFmt.strDefaultFormat(TTime::strCTime());
}

TCQCFldMonWnd::~TCQCFldMonWnd()
{
    // Just in case we didn't get the command cleaned up, do that now
    if (m_pdcmdWait)
        TCQCServerBase::ReleaseCmd(m_pdcmdWait, kCIDLib::False);
}


// ---------------------------------------------------------------------------
//  TCQCFldMonWnd: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCFldMonWnd::Create(TCQCDrvDevFrameWnd* const pwndOwner)
{
    // Store the owner window for later use
    m_pwndOwner = pwndOwner;

    // Make sure our command pointer is initialized null
    m_pdcmdWait = nullptr;

    //
    //  Load the dialog resource we use, so that we can get the position out and use it
    //  as the minimize size when we read in any previously saved frame state.
    //
    TDlgDesc dlgdChildren;
    facCQCDrvDev.bCreateDlgDesc(kCQCDrvDev::ridDlg_FldMon, dlgdChildren);

    // Try to load any previous state
    TCQCFrameState fstFldMon;
    facCQCGKit().bReadFrameState
    (
        kCQCDrvDev::strCfgKey_FldMonState, fstFldMon, dlgdChildren.areaPos().szArea()
    );

    //
    //  We create ourself as a top level, unowned window so that we are not always on top
    //  of the main driver development frame window.
    //
    CreateFrame
    (
        nullptr
        , fstFldMon.areaFrame()
        , L"CQC Driver Test Harness Field Monitor"
        , tCIDCtrls::EWndStyles::HelperFrame
        , tCIDCtrls::EExWndStyles::None
        , tCIDCtrls::EFrameStyles::NoCloseStdFrame
        , kCIDLib::False
    );

    // Get any previuos pos state stored, passing in our current area as the restore state
    RestorePosState(fstFldMon.ePosState(), areaWnd(), kCIDLib::True);
}


// ---------------------------------------------------------------------------
//  TCQCFldMonWnd: Protected, inherited methods
// ---------------------------------------------------------------------------

//
//  We have multiple top level windows so we need to keep the active one set so that it
//  gets accelerator keys and such.
//
tCIDLib::TVoid
TCQCFldMonWnd::ActivationChange(const   tCIDLib::TBoolean       bState
                                , const tCIDCtrls::TWndHandle   hwndOther)
{
    TParent::ActivationChange(bState, hwndOther);
    if (bState)
        facCIDCtrls().SetActiveTopWnd(this);
}



tCIDLib::TVoid
TCQCFldMonWnd::AreaChanged( const   TArea&                  areaPrev
                            , const TArea&                  areaNew
                            , const tCIDCtrls::EPosStates   ePosState
                            , const tCIDLib::TBoolean       bOrgChanged
                            , const tCIDLib::TBoolean       bSizeChanged
                            , const tCIDLib::TBoolean       bStateChanged)
{
    TParent::AreaChanged(areaPrev, areaNew, ePosState, bOrgChanged, bSizeChanged, bStateChanged);

    if (bSizeChanged && (ePosState != tCIDCtrls::EPosStates::Minimized) && m_pwndValue)
        AutoAdjustChildren(areaPrev, areaNew);
}


tCIDLib::TBoolean TCQCFldMonWnd::bCreated()
{
    // Call our parent first
    TParent::bCreated();

    // Set our icon
    facCQCDrvDev.SetFrameIcon(*this, L"DriverIDE");

    // We use a dialog resource to create our widgets
    TDlgDesc dlgdChildren;
    facCQCDrvDev.bCreateDlgDesc(kCQCDrvDev::ridDlg_FldMon, dlgdChildren);

    // Create the children
    tCIDCtrls::TWndId widInitFocus;
    CreateDlgItems(dlgdChildren, widInitFocus);

    // Set the min size as the size that would fit the original dialog content
    TArea areaMin;
    TFrameWnd::AreaForClient
    (
        dlgdChildren.areaPos()
        , areaMin
        , eWndStyles()
        , eExWndStyles()
        , kCIDLib::False
    );
    SetMinMaxSize(areaMin.szArea(), TSize());

    // Size ourself to fit this content
    AutoAdjustChildren(dlgdChildren.areaPos(), areaClient());

    // Get some typed pointers to the relevant widgets
    CastChildWnd(*this, kCQCDrvDev::ridDlg_FldMon_Access, m_pwndAccess);
    CastChildWnd(*this, kCQCDrvDev::ridDlg_FldMon_Change, m_pwndChange);
    CastChildWnd(*this, kCQCDrvDev::ridDlg_FldMon_Limits, m_pwndLimits);
    CastChildWnd(*this, kCQCDrvDev::ridDlg_FldMon_List, m_pwndList);
    CastChildWnd(*this, kCQCDrvDev::ridDlg_FldMon_Name, m_pwndName);
    CastChildWnd(*this, kCQCDrvDev::ridDlg_FldMon_QueryText, m_pwndQText);
    CastChildWnd(*this, kCQCDrvDev::ridDlg_FldMon_SendCmd, m_pwndSendCmd);
    CastChildWnd(*this, kCQCDrvDev::ridDlg_FldMon_Status, m_pwndStatus);
    CastChildWnd(*this, kCQCDrvDev::ridDlg_FldMon_Type, m_pwndType);
    CastChildWnd(*this, kCQCDrvDev::ridDlg_FldMon_Value, m_pwndValue);

    // Register any needed handlers
    m_pwndChange->pnothRegisterHandler(this, &TCQCFldMonWnd::eClickHandler);
    m_pwndList->pnothRegisterHandler(this, &TCQCFldMonWnd::eLBHandler);
    m_pwndQText->pnothRegisterHandler(this, &TCQCFldMonWnd::eClickHandler);
    m_pwndSendCmd->pnothRegisterHandler(this, &TCQCFldMonWnd::eClickHandler);

    // Initially disable the interaction buttons
    m_bLastTime = TCQCGenDrvGlue::s_psdrvCurrent != nullptr;
    SetButtonStates();

    // Kick off our timer
    m_tmidUpdate = tmidStartTimer(CQCDrvDev_FldMonWnd::c4TimerPeriod);

    return kCIDLib::True;
}


tCIDLib::TVoid TCQCFldMonWnd::Destroyed()
{
    // Stop our timer just in case
    if (m_tmidUpdate != kCIDCtrls::tmidInvalid)
    {
        StopTimer(m_tmidUpdate);
        m_tmidUpdate = kCIDCtrls::tmidInvalid;
    }

    // And be sure to call our parent last
    TParent::Destroyed();
}


tCIDLib::TVoid TCQCFldMonWnd::Timer(const tCIDCtrls::TTimerId tmidUpdate)
{
    // FIrst see if the driver is available or not
    {
        TLocker lockrSync(TCQCGenDrvGlue::s_pmtxSync);

        // if the driver is no longer loaded, clear our display
        if (!TCQCGenDrvGlue::s_psdrvCurrent)
        {
            if (m_bLastTime)
            {
                // Clear the last time state to represent the new current state
                m_bLastTime = kCIDLib::False;

                // Set the state to an invalid value to insure it's different
                m_eCurState = tCQCKit::EDrvStates::Count;

                //
                //  It's gone away so clear our list out and the status. Clearing
                //  the list will clear the other fields.
                //
                m_pwndList->RemoveAll();
                m_pwndStatus->ClearText();

                m_pwndChange->SetEnable(kCIDLib::False);
                m_pwndQText->SetEnable(kCIDLib::False);
                m_pwndSendCmd->SetEnable(kCIDLib::False);

                // Clear our driver/field ids
                m_c4DrvSerialNum = 0;
                m_c4FldSerialNum = 0;
                m_c4FldListId = 0;

                // Clear the field serial number for the next connection
                m_c4FldSerialNum = 0;
            }
            return;
        }

        //
        //  We haev a driver. Remember if we are just seeing it, then
        //  store our new last seen state.
        //
        tCIDLib::TBoolean bNewDrv = !m_bLastTime;
        m_bLastTime = kCIDLib::True;

        //
        //  Get the current state of the driver. Save the last one
        //  we saw so we can tell if it changed or not below.
        //
        tCQCKit::EDrvStates eOldState = m_eCurState;
        m_eCurState = TCQCGenDrvGlue::s_psdrvCurrent->eState();

        //
        //  If the field id has changed, or we just now are seeing a new
        //  driver, then we want to force a new load of the fields.
        //
        if (bNewDrv
        ||  (TCQCGenDrvGlue::s_psdrvCurrent->c4FieldListId() != m_c4FldListId))
        {
            try
            {
                m_c4DrvSerialNum = 0;
                m_c4FldSerialNum = 0;
                m_c4FldListId = 0;
                LoadFields();
            }

            catch(TError& errToCatch)
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                throw;
            }
        }

        //
        //  If the state has changed, then update button states and curernt
        //  driver state display.
        //
        if (m_eCurState != eOldState)
        {
            SetButtonStates();
            m_pwndStatus->strWndText(tCQCKit::strXlatEDrvStates(m_eCurState));
        }

        // Update the current field value display
        ShowCurFldVal();
    }

    //
    //  See if we have an outstanding command queued up. If so, then see
    //  if it is done. If so, we can re-enable some of the buttons
    //
    //  We don't need the driver lock here, so we don't put this in the
    //  block above.
    //
    if (m_pdcmdWait && m_pdcmdWait->m_evWait.bWaitFor(5))
    {
        // Put a janitor on the command to insure it gets cleaned up
        TCQCSrvCmdJan janCmd(m_pdcmdWait);

        //
        //  And now clear our pointer just in case since we need to make sure
        //  it's zeroed when there's not a command. Use the pointer in the
        //  janitor for the rest below.
        //
        m_pdcmdWait = nullptr;

        //
        //  See if it failed. If so, display an error. Stop timers while
        //  we are in the popup, or we'll just keep reentering here once
        //  the dialog starts processing messages.
        //
        if (janCmd.pdcmdWait()->m_bError)
        {
            TTimerJanitor janTimer(this, m_tmidUpdate);
            TExceptDlg dlgErr
            (
                *this
                , strWndText()
                , L"The field write operation failed"
                , janCmd.pdcmdWait()->m_errReturn
            );
        }

        // We can enable driver interaction buttons again
        SetButtonStates();
    }
}


// ---------------------------------------------------------------------------
//  TCQCFldMonWnd: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Handle button clicks. We shouldn't be able to get one of these if there's
//  any outstanding command, since the buttons would be disabled.
//
tCIDCtrls::EEvResponses TCQCFldMonWnd::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCDrvDev::ridDlg_FldMon_Change)
    {
        //
        //  Get the selected item in the list box and invoke the field
        //  value change dialog for it.
        //
        WriteToField();
    }
     else if (wnotEvent.widSource() == kCQCDrvDev::ridDlg_FldMon_SendCmd)
    {
        // Invoke the dialog that does the command sending stuff
        TCQCSendDrvCmdDlg dlgSendCmd;
        dlgSendCmd.RunDlg(*this);
    }
     else if (wnotEvent.widSource() == kCQCDrvDev::ridDlg_FldMon_QueryText)
    {
        // Invoke the dialog that does the text query stuff
        TCQCQDrvTextDlg dlgQText;
        dlgQText.RunDlg(*this);
    }
    return tCIDCtrls::EEvResponses::Handled;
}


tCIDCtrls::EEvResponses TCQCFldMonWnd::eLBHandler(TListChangeInfo& wnotEvent)
{
    //
    //  Enable or disable our remove/info buttons according to whether
    //  a valid list box item is cursored or not.
    //
    if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::SelChanged)
    {
        // And update the display widgets with the selected field's info
        const tCIDLib::TCard4 c4ListInd = m_pwndList->c4CurItem();
        const tCIDLib::TCard4 c4Index = m_pwndList->c4IndexToId(c4ListInd);
        const TCQCFldDef& flddCur = m_colFields[c4Index];

        m_pwndName->strWndText(flddCur.strName());
        m_pwndType->strWndText(tCQCKit::strXlatEFldTypes(flddCur.eType()));
        m_pwndLimits->strWndText(flddCur.strLimits());
        m_pwndAccess->strWndText(tCQCKit::strXlatEFldAccess(flddCur.eAccess()));

        //
        //  If this field is write only, then just set the field to indicate
        //  this. Else, we just clear out the value field. We let the timer,
        //  which already has to do the logic to keep this widget up to
        //  date, just update it the next time through. This way we don't
        //  have to lock here.
        //
        if (!tCIDLib::bAllBitsOn(flddCur.eAccess(), tCQCKit::EFldAccess::Read))
            m_pwndValue->strWndText(L"<Field is write-only>");
        else
            m_pwndValue->ClearText();

        // Update button states
        SetButtonStates();

        //
        //  Zero the field serial number since we are on a new field now
        //  and need the first read to always get the current value.
        //
        m_c4FldSerialNum = 0;
    }
     else if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::Invoked)
    {
        // If there is an outstanding queued command, ignore this
        if (!m_pdcmdWait)
        {
            //
            //  Get the selected item in the list box and invoke the field
            //  value change dialog for it if it's writeable.
            //
            const tCIDLib::TCard4 c4ListInd = m_pwndList->c4CurItem();
            const tCIDLib::TCard4 c4Index = m_pwndList->c4IndexToId(c4ListInd);

            const TCQCFldDef& flddCur = m_colFields[c4Index];
            if (tCIDLib::bAllBitsOn(flddCur.eAccess(), tCQCKit::EFldAccess::Write))
                WriteToField();
        }
    }
     else if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::Cleared)
    {
        // Disable the change button and clear the field info widgets
        m_pwndChange->SetEnable(kCIDLib::False);
        m_pwndAccess->ClearText();
        m_pwndLimits->ClearText();
        m_pwndName->ClearText();
        m_pwndType->ClearText();
        m_pwndValue->ClearText();
        m_c4FldSerialNum = 0;
    }
    return tCIDCtrls::EEvResponses::Handled;
}


tCIDLib::TVoid TCQCFldMonWnd::LoadFields()
{
    TWndPaintJanitor janPaint(m_pwndList);

    m_pwndList->RemoveAll();

    // Get the list of fields and use that to fill in the list box
    const tCIDLib::TCard4 c4Count = TCQCGenDrvGlue::s_psdrvCurrent->c4QueryFields
    (
        m_colFields, m_c4FldListId
    );

    if (!c4Count)
        return;

    //
    //  We do a bulk load so we fill in a non-adopting collection, and then
    //  load them all at the end.
    //
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        m_pwndList->c4AddItem(m_colFields[c4Index].strName(), c4Index);

    if (c4Count)
        m_pwndList->SelectByIndex(0);
}


//
//  Enables/disables the buttons based on current conditions
//
tCIDLib::TVoid TCQCFldMonWnd::SetButtonStates()
{
    //
    //  If the driver isn't set, or there is an outstanding command, then
    //  everthing is disabled.
    //
    if (!TCQCGenDrvGlue::s_psdrvCurrent || m_pdcmdWait)
    {
        m_pwndChange->SetEnable(kCIDLib::False);
        m_pwndQText->SetEnable(kCIDLib::False);
        m_pwndSendCmd->SetEnable(kCIDLib::False);
        return;
    }

    // See what the driver state is
    switch(m_eCurState)
    {
        case tCQCKit::EDrvStates::Terminated :
        case tCQCKit::EDrvStates::WaitCommRes :
        case tCQCKit::EDrvStates::WaitConfig :
        case tCQCKit::EDrvStates::WaitConnect :
        case tCQCKit::EDrvStates::WaitInit :
            // Disable everything
            m_pwndValue->ClearText();

            m_pwndChange->SetEnable(kCIDLib::False);
            m_pwndQText->SetEnable(kCIDLib::False);
            m_pwndSendCmd->SetEnable(kCIDLib::False);
            break;

        case tCQCKit::EDrvStates::Connected :
        {
            // We can enable the backdoor commands
            m_pwndQText->SetEnable(kCIDLib::True);
            m_pwndSendCmd->SetEnable(kCIDLib::True);

            // See if there's a field selected
            const tCIDLib::TCard4 c4ListInd = m_pwndList->c4CurItem();
            if (c4ListInd == kCIDLib::c4MaxCard)
            {
                m_pwndChange->SetEnable(kCIDLib::False);
            }
             else
            {
                // There is, enable if writeable, else disable
                const tCIDLib::TCard4 c4Index = m_pwndList->c4IndexToId(c4ListInd);
                const TCQCFldDef& flddCur = m_colFields[c4Index];
                if (!tCIDLib::bAllBitsOn(flddCur.eAccess(), tCQCKit::EFldAccess::Write))
                    m_pwndChange->SetEnable(kCIDLib::False);
                else
                    m_pwndChange->SetEnable(kCIDLib::True);
            }
            break;
        }

        default :
            break;
    };
}


//
//  Update the current field value if appropriate. The caller has locked
//  and knows the driver is currently present. So we cannot do any popups
//  or we'll hang up.
//
tCIDLib::TVoid TCQCFldMonWnd::ShowCurFldVal()
{
    // If no field selected, nothing to do
    const tCIDLib::TCard4 c4ListInd = m_pwndList->c4CurItem();
    if (c4ListInd == kCIDLib::c4MaxCard)
        return;

    const tCIDLib::TCard4 c4Index = m_pwndList->c4IndexToId(c4ListInd);

    // If not readable, nothing to do
    const TCQCFldDef& flddCur = m_colFields[c4Index];
    if (tCIDLib::bAllBitsOn(flddCur.eAccess(), tCQCKit::EFldAccess::Read))
    {
        TString strVal;
        tCQCKit::EFldTypes eType;

        CIDAssert(TCQCGenDrvGlue::s_psdrvCurrent != nullptr, L"Drier pointer is null");
        TCQCServerBase& sdrvCur = *TCQCGenDrvGlue::s_psdrvCurrent;

        //
        //  Check the value. We'll get true back if it changed. We could get
        //  an out of sync exceptio here if changes have occurred on the driver
        //  since we got these ids. If so, just return. The timer will see the
        //  change next round.
        //
        tCIDLib::TBoolean bRes;
        try
        {
            bRes = sdrvCur.bReadField
            (
                m_c4FldListId
                , flddCur.c4Id()
                , m_c4FldSerialNum
                , strVal
                , eType
                , kCIDLib::False
            );
        }

        catch(...)
        {
            return;
        }

        if (bRes)
        {
            // Something has changed so update
            if ((sdrvCur.bFieldInErr(flddCur.c4Id()))
            ||  (m_eCurState != tCQCKit::EDrvStates::Connected))
            {
                // It's gone into error state or the driver is
                strVal = L"[field is in error]";
            }
             else
            {
                //
                //  If it's a time field, then we want to show the human
                //  readable version as well.
                //
                if (eType == tCQCKit::EFldTypes::Time)
                {
                    tCIDLib::TEncodedTime enctTemp;
                    if (strVal.bToCard8(enctTemp, tCIDLib::ERadices::Hex))
                    {
                        m_tmFmt = enctTemp;
                        strVal.Append(L" (");
                        m_tmFmt.FormatToStr(strVal, tCIDLib::EAppendOver::Append);
                        strVal.Append(L")");
                    }
                }
            }
            m_pwndValue->strWndText(strVal);
        }
    }
}


//
//  Called when the user asks to change the selected field's value. In theory
//  this won't get called unless there is a writeable field selected, but
//  we still check just to be sure.
//
tCIDLib::TVoid TCQCFldMonWnd::WriteToField()
{
    const tCIDLib::TCard4 c4ListInd = m_pwndList->c4CurItem();
    if (c4ListInd == kCIDLib::c4MaxCard)
        return;

    const tCIDLib::TCard4 c4Index = m_pwndList->c4IndexToId(c4ListInd);

    // Get the field info for the selected field
    const TCQCFldDef& flddCur = m_colFields[c4Index];

    //
    //  Pop up the dialog box that just get's a text string from
    //  them. We'll use the field write method that generic clients
    //  would use, which takes a string and leaves it up to the
    //  field storage type to validate it.
    //
    TString  strValue;
    {
        const TString strTitle(kDrvDevMsgs::midDlg_GetValue_Title, facCQCDrvDev);
        const tCIDLib::TBoolean bRes = facCIDWUtils().bGetText
        (
            *this, strTitle, TString::strEmpty(), strValue
        );

        if (!bRes)
            return;
    }

    // Disable the buttons that we don't want available while active commands
    m_pwndChange->SetEnable(kCIDLib::False);
    m_pwndQText->SetEnable(kCIDLib::False);
    m_pwndSendCmd->SetEnable(kCIDLib::False);

    //
    //  Allow us to get totally back to being the active window by processing
    //  messages for a bit. This way, we won't confuse things if the target
    //  'device' is an app and this write is going to change the active
    //  window. Otherwise, we'll see our reactivation messages (from the
    //  dialog we just ran) after we come back from here.
    //
    facCIDCtrls().MsgYield(50);

    //
    //  And queue up the command. Our timer will start monitoring it. Make
    //  sure the driver is there.
    //
    TLocker lockrSync(TCQCGenDrvGlue::s_pmtxSync);
    if (TCQCGenDrvGlue::s_psdrvCurrent)
    {
        m_pdcmdWait = TCQCGenDrvGlue::s_psdrvCurrent->pdcmdQWriteFld
        (
             TString::strEmpty()
             , m_c4FldListId
             , flddCur.c4Id()
             , strValue
             , tCQCKit::EDrvCmdWaits::Wait
        );
    }
}

