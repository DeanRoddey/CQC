//
// FILE NAME: CQCAdmin_SchedEvTab.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/22/2016
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
//  This file implements a tab for editing scheduled events
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCAdmin.hpp"
#include    "CQCAdmin_SchedEvTab.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TSchedEvTab, TContentTab)



// ---------------------------------------------------------------------------
//  CLASS: TSchedEvTab
// PREFIX: wnd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TSchedEvTab: Constructors and destructor
// ---------------------------------------------------------------------------

//
//  Indicate edit mode in our call to our parent class, since we only support edit
//  mode in this tab type.
//
TSchedEvTab::TSchedEvTab(const TString& strPath, const TString& strRelPath) :

    TContentTab(strPath, strRelPath, L"Sched Event", kCIDLib::True)
    , m_pwndEditAct(nullptr)
    , m_pwndEvType(nullptr)
    , m_pwndLogRuns(nullptr)
    , m_pwndTest(nullptr)
    , m_pwndTitle(nullptr)
{
}

TSchedEvTab::~TSchedEvTab()
{
}


// ---------------------------------------------------------------------------
//  TSchedEvTab: Public, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid
TSchedEvTab::CreateTab(         TTabbedWnd&         wndParent
                        , const TString&            strTabText
                        , const TCQCSchEvent&       csrcEdit
                        , const tCIDLib::TFloat8    f8Lat
                        , const tCIDLib::TFloat8    f8Long)
{
    // Store away the data, and a second copy for comparisons later
    m_csrcEdit = csrcEdit;
    m_csrcOrg = csrcEdit;
    m_f8Lat = f8Lat;
    m_f8Long = f8Long;

    // Force a recalc of the next event time for initial display purposes
    m_csrcEdit.CalcNextTime(m_f8Lat, m_f8Long);

    // And now create the tab
    wndParent.c4CreateTab(this, strTabText, 0, kCIDLib::True);
}


tCIDLib::TVoid TSchedEvTab::UpdatePauseState(const tCIDLib::TBoolean bToSet)
{
    m_csrcEdit.bPaused(bToSet);
}


// ---------------------------------------------------------------------------
//  TSchedEvTab: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TSchedEvTab::AreaChanged(const  TArea&                  areaPrev
                        , const TArea&                  areaNew
                        , const tCIDCtrls::EPosStates   ePosState
                        , const tCIDLib::TBoolean       bOrgChanged
                        , const tCIDLib::TBoolean       bSizeChanged
                        , const tCIDLib::TBoolean       bStateChanged)
{
    // Call our parent first
    TParent::AreaChanged(areaPrev, areaNew, ePosState, bOrgChanged, bSizeChanged, bStateChanged);

    // If the size changed, and we children and not minimized, then adjust children
    if (bSizeChanged && m_pwndEvType && (ePosState != tCIDCtrls::EPosStates::Minimized))
        AutoAdjustChildren(areaPrev, areaNew);
}


tCIDLib::TBoolean TSchedEvTab::bCreated()
{
    TParent::bCreated();

    // Get the dialog description we'll use to create the controls
    TDlgDesc dlgdChildren;
    facCQCAdmin.bCreateDlgDesc(kCQCAdmin::ridTab_EdSchEv, dlgdChildren);

    // Create the children as children of the client
    tCIDLib::TCard4 c4InitFocus;
    CreateDlgItems(dlgdChildren, c4InitFocus);

    // Do an initial auto-adjust to get them sized to fit us
    AutoAdjustChildren(dlgdChildren.areaPos(), areaClient());

    //
    //  Set us to use the standard window background. The dialog description we
    //  used to create the content was set to the main window theme, so all of the
    //  widgets should be fine already.
    //
    SetBgnColor(facCIDCtrls().rgbSysClr(tCIDCtrls::ESysColors::Window));

    // Get pointers to our controls we need to interact with
    CastChildWnd(*this, kCQCAdmin::ridTab_EdSchEv_Title, m_pwndTitle);
    CastChildWnd(*this, kCQCAdmin::ridTab_EdSchEv_EditAct, m_pwndEditAct);
    CastChildWnd(*this, kCQCAdmin::ridTab_EdSchEv_LogRuns, m_pwndLogRuns);
    CastChildWnd(*this, kCQCAdmin::ridTab_EdSchEv_Test, m_pwndTest);
    CastChildWnd(*this, kCQCAdmin::ridTab_EdSchEv_Type, m_pwndEvType);

    // Load the title text
    m_pwndTitle->strWndText(m_csrcEdit.strTitle());

    // Load up the event types list
    tCQCKit::ESchTypes eType = tCQCKit::ESchTypes::Min;
    while (eType <= tCQCKit::ESchTypes::Max)
    {
        m_pwndEvType->c4AddItem(tCQCKit::strLoadESchTypes(eType));
        eType++;
    }

    // Register our event handlers
    m_pwndEditAct->pnothRegisterHandler(this, &TSchedEvTab::eClickHandler);
    m_pwndLogRuns->pnothRegisterHandler(this, &TSchedEvTab::eClickHandler);
    m_pwndTest->pnothRegisterHandler(this, &TSchedEvTab::eClickHandler);
    m_pwndEvType->pnothRegisterHandler(this, &TSchedEvTab::eLBHandler);

    //
    //  And select the incoming event type, which will cause the right controls
    //  for this type to be loaded, since we force an event.
    //
    m_pwndEvType->SelectByIndex(tCIDLib::c4EnumOrd(m_csrcEdit.eType()), kCIDLib::True);

    //
    //  Iniitalize the log runs check box, which isn't per type and so won't have been
    //  set by the selection above.
    //
    m_pwndLogRuns->SetCheckedState(m_csrcEdit.bLoggable());

    return kCIDLib::True;
}


tCIDLib::ESaveRes
TSchedEvTab::eDoSave(       TString&                strErrMsg
                    , const tCQCAdmin::ESaveModes   eMode
                    ,       tCIDLib::TBoolean&      bChangesSaved)
{
    //
    //  Make sure that we have a description field, and that the action
    //  has at least one sub-action defined.
    //
    m_csrcEdit.strTitle(m_pwndTitle->strWndText());
    if (m_csrcEdit.strTitle().bIsEmpty())
    {
        strErrMsg = facCQCIGKit().strMsg(kIGKitErrs::errcEv_NoTitle);
        return tCIDLib::ESaveRes::Errors;
    }

    if (!m_csrcEdit.bHasAnyOps())
    {
        strErrMsg = facCQCKit().strMsg(kKitErrs::errcCmd_NoActCommands);
        return tCIDLib::ESaveRes::Errors;
    }

    // Pull the info out and store it away
    tCQCKit::ESchTypes eType = tCQCKit::ESchTypes(m_pwndEvType->c4CurItem());
    TTime tmInit(tCIDLib::ESpecialTimes::CurrentTime);
    switch(eType)
    {
        case tCQCKit::ESchTypes::Once :
        {
            TCalendar* pwndCal;
            TComboBox* pwndHour;
            TComboBox* pwndMin;

            CastChildWnd(*this, kCQCAdmin::ridTab_EdSchEv_OneSh_Date, pwndCal);
            CastChildWnd(*this, kCQCAdmin::ridTab_EdSchEv_OneSh_Hour, pwndHour);
            CastChildWnd(*this, kCQCAdmin::ridTab_EdSchEv_OneSh_Min, pwndMin);

            //
            //  Get the time stamp from the calendar, break out the date info, then
            //  add to that the hour/minute info to create the final time stamp.
            //
            TTime tmDate;
            pwndCal->GetSelection(tmDate);

            tCIDLib::TCard4     c4Hour;
            tCIDLib::TCard4     c4Min;
            tCIDLib::TCard4     c4Year;
            tCIDLib::TCard4     c4Day;
            tCIDLib::EMonths    eMonth;
            tmDate.eAsDateInfo(c4Year, eMonth, c4Day);
            c4Hour = pwndHour->c4CurItem();
            c4Min = pwndMin->c4CurItem();

            tmDate.FromDetails(c4Year, eMonth, c4Day, c4Hour, c4Min, 0, 0);
            m_csrcEdit.SetOneShot(tmDate.enctTime());
            break;
        }

        case tCQCKit::ESchTypes::Daily :
        {
            TComboBox*      pwndHour;
            TComboBox*      pwndMin;
            CastChildWnd(*this, kCQCAdmin::ridTab_EdSchEv_Daily_Hour, pwndHour);
            CastChildWnd(*this, kCQCAdmin::ridTab_EdSchEv_Daily_Min, pwndMin);

            // Get the current date info from the initial time
            tCIDLib::TCard4     c4Year;
            tCIDLib::TCard4     c4Day;
            tCIDLib::EMonths    eMonth;
            tmInit.eAsDateInfo(c4Year, eMonth, c4Day);

            // Now set our hour/minute and set the time stamp back from those details
            tCIDLib::TCard4     c4Hour = pwndHour->c4CurItem();
            tCIDLib::TCard4     c4Min = pwndMin->c4CurItem();
            tmInit.FromDetails(c4Year, eMonth, c4Day, c4Hour, c4Min);

            //
            //  We'll pass that calculated date as the starting time, minus a bit so that
            //  there's no way it can see the current time as beyond this. That insures
            //  that, if we aren't already past this hour/minute for today, that the enxt
            //  time will be for today.
            //
            m_csrcEdit.SetScheduled
            (
                eType, 0, c4Hour, c4Min, 0, tmInit.enctTime() - (kCIDLib::enctOneMinute / 2)
            );
            break;
        }

        case tCQCKit::ESchTypes::Weekly :
        {
            TMultiColListBox*   pwndDays;
            TComboBox*          pwndHour;
            TComboBox*          pwndMin;
            CastChildWnd(*this, kCQCAdmin::ridTab_EdSchEv_Weekly_Days, pwndDays);
            CastChildWnd(*this, kCQCAdmin::ridTab_EdSchEv_Weekly_Hour, pwndHour);
            CastChildWnd(*this, kCQCAdmin::ridTab_EdSchEv_Weekly_Min, pwndMin);

            // Set up the mask
            tCIDLib::TCard4 c4Mask = 0;
            const tCIDLib::TCard4 c4Count = pwndDays->c4ItemCount();
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            {
                if (pwndDays->bIsMarked(c4Index))
                    c4Mask |= (0x1UL << c4Index);
            }

            if (!c4Mask)
            {
                strErrMsg = facCQCAdmin.strMsg(kCQCAErrs::errcEv_NoDays);
                return tCIDLib::ESaveRes::Errors;
            }

            m_csrcEdit.SetScheduled
            (
                eType
                , 0
                , pwndHour->c4CurItem()
                , pwndMin->c4CurItem()
                , c4Mask
                , tmInit.enctTime()
            );
            break;
        }

        case tCQCKit::ESchTypes::Monthly :
        {
            TComboBox*          pwndDay;
            TComboBox*          pwndHour;
            TComboBox*          pwndMin;
            TMultiColListBox*   pwndMonths;
            CastChildWnd(*this, kCQCAdmin::ridTab_EdSchEv_Monthly_Day, pwndDay);
            CastChildWnd(*this, kCQCAdmin::ridTab_EdSchEv_Monthly_Hour, pwndHour);
            CastChildWnd(*this, kCQCAdmin::ridTab_EdSchEv_Monthly_Min, pwndMin);
            CastChildWnd(*this, kCQCAdmin::ridTab_EdSchEv_Monthly_Months, pwndMonths);

            // Set up the mask
            tCIDLib::TCard4 c4Mask = 0;
            const tCIDLib::TCard4 c4Count = pwndMonths->c4ItemCount();
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            {
                if (pwndMonths->bIsMarked(c4Index))
                    c4Mask |= (0x1UL << c4Index);
            }

            if (!c4Mask)
            {
                strErrMsg = facCQCAdmin.strMsg(kCQCAErrs::errcEv_NoMonths);
                return tCIDLib::ESaveRes::Errors;
            }

            // Add one for the day since it's one based
            m_csrcEdit.SetScheduled
            (
                eType
                , pwndDay->c4CurItem() + 1
                , pwndHour->c4CurItem()
                , pwndMin->c4CurItem()
                , c4Mask
                , tmInit.enctTime()
            );
            break;
        }

        case tCQCKit::ESchTypes::MinPeriod :
        case tCQCKit::ESchTypes::HrPeriod :
        case tCQCKit::ESchTypes::DayPeriod :
        {
            TComboBox*      pwndInterval;
            TStaticText*    pwndIntName;
            CastChildWnd(*this, kCQCAdmin::ridTab_EdSchEv_Per_Interval, pwndInterval);
            CastChildWnd(*this, kCQCAdmin::ridTab_EdSchEv_Per_IntName, pwndIntName);

            //
            //  If it's a hours or days, then we allow them to select a minute or hour
            //  at which to run it, so we need to get some more info and adjust the
            //  initial time.
            //
            tCIDLib::TCard4 c4SubPeriod = 0;
            if ((eType == tCQCKit::ESchTypes::HrPeriod)
            ||  (eType == tCQCKit::ESchTypes::DayPeriod))
            {
                TComboBox* pwndStartOn;
                CastChildWnd(*this, kCQCAdmin::ridTab_EdSchEv_Per_On, pwndStartOn);
                c4SubPeriod = pwndStartOn->c4CurItem();
            }

            m_csrcEdit.SetPeriodic
            (
                eType, pwndInterval->c4CurItem() + 1, c4SubPeriod, tmInit.enctTime()
            );
            break;
        }

        case tCQCKit::ESchTypes::Sunrise :
        case tCQCKit::ESchTypes::Sunset :
        {
            TComboBox*      pwndOffset;
            CastChildWnd(*this, kCQCAdmin::ridTab_EdSchEv_Sun_Offset, pwndOffset);

            const tCIDLib::TInt4 i4Ofs(tCIDLib::TInt4(pwndOffset->c4CurItem()) - 59);
            if (eType == tCQCKit::ESchTypes::Sunrise)
                m_csrcEdit.SetSunrise(m_f8Lat, m_f8Long, i4Ofs);
            else
                m_csrcEdit.SetSunset(m_f8Lat, m_f8Long, i4Ofs);
            break;
        }

        default:
            CIDAssert2(L"Unknown scheduled event type");
            break;
    };

    if (eMode == tCQCAdmin::ESaveModes::Test)
    {
        if (m_csrcEdit == m_csrcOrg)
            return tCIDLib::ESaveRes::NoChanges;
        return tCIDLib::ESaveRes::OK;
    }

    // There are changes, so write our current edit contents out if allowed
    try
    {
        TDataSrvClient dsclToUse;

        //
        //  Before we store it, update the next possible time. This minimizes the
        //  overhead involved in finding the next run time for long lived events
        //  that we are modifying (the event server only has to look forward from
        //  this point instead of from the original point.)
        //
        m_csrcEdit.CalcNextTime(m_f8Lat, m_f8Long);

        tCIDLib::TCard4 c4SerialNum = 0;
        tCIDLib::TEncodedTime enctLastChange;
        tCIDLib::TKVPFList  colXMeta;
        dsclToUse.WriteScheduledEvent
        (
            strRelPath()
            , c4SerialNum
            , enctLastChange
            , m_csrcEdit
            , kCQCRemBrws::c4Flag_OverwriteOK
            , colXMeta
            , facCQCAdmin.sectUser()
        );

        //
        //  Now copy our current edit stuff to the original, since we want to show
        //  no changes again.
        //
        m_csrcOrg = m_csrcEdit;

        switch(eType)
        {
            case tCQCKit::ESchTypes::Daily :
                UpdateNextTime(kCQCAdmin::ridTab_EdSchEv_Daily_NextTime);
                break;

            case tCQCKit::ESchTypes::Weekly :
                UpdateNextTime(kCQCAdmin::ridTab_EdSchEv_Weekly_NextTime);
                break;

            case tCQCKit::ESchTypes::Monthly :
                UpdateNextTime(kCQCAdmin::ridTab_EdSchEv_Monthly_NextTime);
                break;

            case tCQCKit::ESchTypes::MinPeriod :
            case tCQCKit::ESchTypes::HrPeriod :
            case tCQCKit::ESchTypes::DayPeriod :
                UpdateNextTime(kCQCAdmin::ridTab_EdSchEv_Per_NextTime);
                break;

            case tCQCKit::ESchTypes::Sunrise :
            case tCQCKit::ESchTypes::Sunset :
                UpdateNextTime(kCQCAdmin::ridTab_EdSchEv_SunBased_NextTime);
                break;

            default :
                CIDAssert2(L"Unknonwn scheduled event type");
                break;
        };
        bChangesSaved = kCIDLib::True;
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        strErrMsg = errToCatch.strErrText();
        return tCIDLib::ESaveRes::Errors;
    }

    // No changes anymore
    return tCIDLib::ESaveRes::NoChanges;
}


// ---------------------------------------------------------------------------
//  TSchedEvTab: Private, non-virtual methods
// ---------------------------------------------------------------------------

tCIDCtrls::EEvResponses TSchedEvTab::eClickHandler(TButtClickInfo& wnotClick)
{
    if (wnotClick.widSource() == kCQCAdmin::ridTab_EdSchEv_EditAct)
    {
        // The user wants to edit this events action
        facCQCIGKit().bEditAction
        (
            *this
            , m_csrcEdit
            , kCIDLib::True
            , facCQCAdmin.cuctxToUse()
            , nullptr
        );
    }
     else if (wnotClick.widSource() == kCQCAdmin::ridTab_EdSchEv_LogRuns)
    {
        m_csrcEdit.bLoggable(wnotClick.bState());
    }
     else if (wnotClick.widSource() == kCQCAdmin::ridTab_EdSchEv_Test)
    {
        TWndPtrJanitor janPtr(tCIDCtrls::ESysPtrs::Wait);
        TActTraceWnd wndTrace;

        wndTrace.RunModally
        (
            *this
            , m_csrcEdit
            , facCQCAdmin.cuctxToUse()
            , m_pwndTitle->strWndText()
            , kCQCKit::strEvId_OnScheduledTime
            , kCIDLib::False
            , L"AdminIntf"
        );
    }
    return tCIDCtrls::EEvResponses::Handled;
}


tCIDCtrls::EEvResponses TSchedEvTab::eLBHandler(TListChangeInfo& wnotEvent)
{
    if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::SelChanged)
    {
        //
        //  We only see the event type here, so we know that's what it is. We need
        //  to remove any controls for the previous type and load up the ones for
        //  the new type and initialize them.
        //
        //  So first delete any old ones
        //
        for (tCIDCtrls::TWndId widCur = 1010; widCur < 1020; widCur++)
        {
            if (pwndChildById(widCur, kCIDLib::False))
                DestroyChild(widCur);
        }

        TTime tmNow(tCIDLib::ESpecialTimes::CurrentTime);
        tCIDLib::TCard4         c4Year;
        tCIDLib::EMonths        eMonth;
        tCIDLib::TCard4         c4Day;
        tCIDLib::TCard4         c4Hour;
        tCIDLib::TCard4         c4Min;
        tCIDLib::TCard4         c4Sec;
        tCIDLib::TCard4         c4Millis;
        tCIDLib::TEncodedTime   enctStamp;
        tmNow.eExpandDetails(c4Year, eMonth, c4Day, c4Hour, c4Min, c4Sec, c4Millis, enctStamp);


        //
        //  Figure out which new one we want to load. And, if the type has changed, we
        //  need to reset the event.
        //
        tCIDLib::TResId ridToLoad = 0;
        const tCQCKit::ESchTypes eType(tCQCKit::ESchTypes(wnotEvent.c4Index()));
        switch(eType)
        {
            case tCQCKit::ESchTypes::Once :
                ridToLoad = kCQCAdmin::ridTab_EdSchEv_OneSh;

                if (eType != m_csrcEdit.eType())
                    m_csrcEdit.SetOneShot(enctStamp + kCIDLib::enctOneHour);
                break;

            case tCQCKit::ESchTypes::Daily :
            case tCQCKit::ESchTypes::Weekly :
            case tCQCKit::ESchTypes::Monthly :
                if (eType == tCQCKit::ESchTypes::Daily)
                    ridToLoad = kCQCAdmin::ridTab_EdSchEv_Daily;
                else if (eType == tCQCKit::ESchTypes::Weekly)
                    ridToLoad = kCQCAdmin::ridTab_EdSchEv_Weekly;
                else
                    ridToLoad = kCQCAdmin::ridTab_EdSchEv_Monthly;

                if (eType != m_csrcEdit.eType())
                    m_csrcEdit.SetScheduled(eType, c4Day, c4Hour, c4Min, 1, enctStamp);
                break;

            case tCQCKit::ESchTypes::MinPeriod :
            case tCQCKit::ESchTypes::HrPeriod :
            case tCQCKit::ESchTypes::DayPeriod :
                ridToLoad = kCQCAdmin::ridTab_EdSchEv_Periodic;
                if (eType != m_csrcEdit.eType())
                    m_csrcEdit.SetPeriodic(eType, 1, 0, enctStamp);
                break;

            case tCQCKit::ESchTypes::Sunrise :
            case tCQCKit::ESchTypes::Sunset :
                ridToLoad = kCQCAdmin::ridTab_EdSchEv_SunBased;
                if (eType != m_csrcEdit.eType())
                {
                    if (eType == tCQCKit::ESchTypes::Sunrise)
                        m_csrcEdit.SetSunrise(m_f8Lat, m_f8Long, 0);
                    else
                        m_csrcEdit.SetSunset(m_f8Lat, m_f8Long, 0);
                }
                break;

            default:
                break;
        };

        // Load this new stuff up
        tCIDLib::TCard4 c4InitFocus;
        TDlgDesc dlgdChildren;
        facCQCAdmin.bCreateDlgDesc(ridToLoad, dlgdChildren);
        CreateDlgItems(dlgdChildren, c4InitFocus);

        // Load the contents up
        LoadIncoming();

        //
        //  Now we need to push the edit action button back to the top since the
        //  ones we created above will end up after it and that will throw off the
        //  tab order.
        //
        m_pwndEditAct->ToTop();
    }
    return tCIDCtrls::EEvResponses::Handled;
}


//
//  Called to get the incoming values loaded when we start up. We assume the controls
//  already are present. That happens the event type changes and it is done on startup
//  to select the incoming type.
//
tCIDLib::TVoid TSchedEvTab::LoadIncoming()
{
    // Get the dialog description we'll use to create the controls
    tCIDLib::TCard4 c4Hour, c4Minute, c4Second;
    const TTime tmAt(m_csrcEdit.enctAt());
    tmAt.c4AsTimeInfo(c4Hour, c4Minute, c4Second);

    const tCQCKit::ESchTypes eType(m_csrcEdit.eType());
    switch(eType)
    {
        case tCQCKit::ESchTypes::Once :
        {
            //
            //  We have to set the calendar to the date and load the hour/min combos.
            //  So get the run time and break it out to its constituent parts.
            //
            TCalendar* pwndCal;
            CastChildWnd(*this, kCQCAdmin::ridTab_EdSchEv_OneSh_Date, pwndCal);
            pwndCal->SetSelection(tmAt);

            TComboBox* pwndHour;
            TComboBox* pwndMin;
            CastChildWnd(*this, kCQCAdmin::ridTab_EdSchEv_OneSh_Hour, pwndHour);
            CastChildWnd(*this, kCQCAdmin::ridTab_EdSchEv_OneSh_Min, pwndMin);

            LoadNumCombo(0, 23, *pwndHour);
            LoadNumCombo(0, 59, *pwndMin);

            pwndHour->SelectByIndex(c4Hour);
            pwndMin->SelectByIndex(c4Minute);
            break;
        }

        case tCQCKit::ESchTypes::Daily :
        {
            TComboBox* pwndHour;
            TComboBox* pwndMin;
            CastChildWnd(*this, kCQCAdmin::ridTab_EdSchEv_Daily_Hour, pwndHour);
            CastChildWnd(*this, kCQCAdmin::ridTab_EdSchEv_Daily_Min, pwndMin);

            LoadNumCombo(0, 23, *pwndHour);
            LoadNumCombo(0, 59, *pwndMin);

            pwndHour->SelectByIndex(c4Hour);
            pwndMin->SelectByIndex(c4Minute);

            UpdateNextTime(kCQCAdmin::ridTab_EdSchEv_Daily_NextTime);
            break;
        }

        case tCQCKit::ESchTypes::Weekly :
        {
            TComboBox* pwndHour;
            TComboBox* pwndMin;
            TMultiColListBox* pwndDays;
            CastChildWnd(*this, kCQCAdmin::ridTab_EdSchEv_Weekly_Hour, pwndHour);
            CastChildWnd(*this, kCQCAdmin::ridTab_EdSchEv_Weekly_Min, pwndMin);
            CastChildWnd(*this, kCQCAdmin::ridTab_EdSchEv_Weekly_Days, pwndDays);

            // We aren't showing the columns on the days list but we have to set one
            tCIDLib::TStrList colCols(1);
            colCols.objAdd(L"Available Days");
            pwndDays->SetColumns(colCols);

            LoadNumCombo(0, 23, *pwndHour);
            LoadNumCombo(0, 59, *pwndMin);

            // Load up the week names
            TString strColVal;
            tCIDLib::TStrList colDays;
            tCIDLib::EWeekDays eDay = tCIDLib::EWeekDays::Min;
            while (eDay <= tCIDLib::EWeekDays::Max)
            {
                strColVal = L"\"";
                strColVal.Append(TLocale::strDay(eDay));
                strColVal.Append(L"\"");
                colDays.objAdd(strColVal);
                eDay++;
            }
            pwndDays->LoadList(colDays);

            pwndHour->SelectByIndex(c4Hour);
            pwndMin->SelectByIndex(c4Minute);

            // Select the days based on the mask
            const tCIDLib::TCard4 c4Mask = m_csrcEdit.c4Mask();
            for (tCIDLib::TCard4 c4Index = 0; c4Index < 7; c4Index++)
            {
                if (c4Mask & (0x1UL << c4Index))
                    pwndDays->MarkAt(c4Index, kCIDLib::True);
            }

            UpdateNextTime(kCQCAdmin::ridTab_EdSchEv_Weekly_NextTime);
            break;
        }

        case tCQCKit::ESchTypes::Monthly :
        {
            TComboBox* pwndDay;
            TComboBox* pwndHour;
            TComboBox* pwndMin;
            TMultiColListBox* pwndMonths;
            CastChildWnd(*this, kCQCAdmin::ridTab_EdSchEv_Monthly_Day, pwndDay);
            CastChildWnd(*this, kCQCAdmin::ridTab_EdSchEv_Monthly_Hour, pwndHour);
            CastChildWnd(*this, kCQCAdmin::ridTab_EdSchEv_Monthly_Min, pwndMin);
            CastChildWnd(*this, kCQCAdmin::ridTab_EdSchEv_Monthly_Months, pwndMonths);

            // We aren't showing the columns on the months list but we have to set one
            tCIDLib::TStrList colCols(1);
            colCols.objAdd(L"Available Months");
            pwndMonths->SetColumns(colCols);

            LoadNumCombo(1, 31, *pwndDay);
            LoadNumCombo(0, 23, *pwndHour);
            LoadNumCombo(0, 59, *pwndMin);

            // Load up the month names
            TString strColVal;
            tCIDLib::TStrList colMonths;
            tCIDLib::EMonths eMonth = tCIDLib::EMonths::Min;
            while (eMonth <= tCIDLib::EMonths::Max)
            {
                strColVal = L"\"";
                strColVal.Append(TLocale::strMonth(eMonth));
                strColVal.Append(L"\"");
                colMonths.objAdd(strColVal);
                eMonth++;
            }
            pwndMonths->LoadList(colMonths);

            // The day is one based, so sub one for an index
            pwndDay->SelectByIndex(m_csrcEdit.c4Day() - 1);
            pwndHour->SelectByIndex(c4Hour);
            pwndMin->SelectByIndex(c4Minute);

            // Select the months based on the mask
            const tCIDLib::TCard4 c4Mask = m_csrcEdit.c4Mask();
            for (tCIDLib::TCard4 c4Index = 0; c4Index < 12; c4Index++)
            {
                if (c4Mask & (0x1UL << c4Index))
                    pwndMonths->MarkAt(c4Index, kCIDLib::True);
            }

            UpdateNextTime(kCQCAdmin::ridTab_EdSchEv_Monthly_NextTime);
            break;
        }

        case tCQCKit::ESchTypes::MinPeriod :
        case tCQCKit::ESchTypes::HrPeriod :
        case tCQCKit::ESchTypes::DayPeriod :
        {
            TComboBox*      pwndInterval;
            TStaticText*    pwndIntName;
            CastChildWnd(*this, kCQCAdmin::ridTab_EdSchEv_Per_Interval, pwndInterval);
            CastChildWnd(*this, kCQCAdmin::ridTab_EdSchEv_Per_IntName, pwndIntName);

            // Indicate the interval type
            tCIDLib::TCard4 c4InitSel = 0;
            if (eType == tCQCKit::ESchTypes::MinPeriod)
            {
                pwndIntName->strWndText(L"Minutes");
                c4InitSel = m_csrcEdit.c4Minute();
            }
             else if (eType == tCQCKit::ESchTypes::HrPeriod)
            {
                pwndIntName->strWndText(L"Hours");
                c4InitSel = m_csrcEdit.c4Hour();
            }
             else
            {
                pwndIntName->strWndText(L"Days");
                c4InitSel = m_csrcEdit.c4Day();
            }

            //
            //  Load up the interval combo box and select the current value. They are
            //  1 based, so sub one for an index.
            //
            LoadNumCombo(1, 100, *pwndInterval);
            pwndInterval->SelectByIndex(c4InitSel - 1);

            //
            //  If it's hours or days, then we
            //
            if ((eType == tCQCKit::ESchTypes::HrPeriod)
            ||  (eType == tCQCKit::ESchTypes::DayPeriod))
            {
                TComboBox* pwndStartOn;
                TStaticText* pwndOnPref;
                CastChildWnd(*this, kCQCAdmin::ridTab_EdSchEv_Per_On, pwndStartOn);
                CastChildWnd(*this, kCQCAdmin::ridTab_EdSchEv_Per_OnPref, pwndOnPref);

                tCIDLib::TMsgId midPref;
                if (eType == tCQCKit::ESchTypes::HrPeriod)
                {
                    midPref = kCQCAMsgs::midTab_EdSchEv_Per_OnPrefM;
                    LoadNumCombo(0, 59, *pwndStartOn);
                }
                else
                {
                    midPref = kCQCAMsgs::midTab_EdSchEv_Per_OnPrefH;
                    LoadNumCombo(0, 23, *pwndStartOn);
                }
                pwndStartOn->SelectByIndex(m_csrcEdit.c4SubPeriod());
                pwndOnPref->strWndText(facCQCAdmin.strMsg(midPref));

                UpdateNextTime(kCQCAdmin::ridTab_EdSchEv_Per_NextTime);
            }
             else
            {
                // Hide the 'on' stuff and next time
                SetVisibility(kCQCAdmin::ridTab_EdSchEv_Per_On, kCIDLib::False);
                SetVisibility(kCQCAdmin::ridTab_EdSchEv_Per_OnPref, kCIDLib::False);
                SetVisibility(kCQCAdmin::ridTab_EdSchEv_Per_NextTime, kCIDLib::False);
            }
            break;
        }

        case tCQCKit::ESchTypes::Sunrise :
        case tCQCKit::ESchTypes::Sunset :
        {
            TComboBox* pwndOffset;
            CastChildWnd(*this, kCQCAdmin::ridTab_EdSchEv_Sun_Offset, pwndOffset);

            LoadNumCombo(-59, 59, *pwndOffset);
            pwndOffset->SelectByIndex(m_csrcEdit.i4Offset() + 59);

            UpdateNextTime(kCQCAdmin::ridTab_EdSchEv_SunBased_NextTime);
            break;
        }

        default:
            break;
    };
}


tCIDLib::TVoid
TSchedEvTab::LoadNumCombo(  const   tCIDLib::TInt4  i4Min
                            , const tCIDLib::TInt4  i4Max
                            ,       TComboBox&      wndTar)
{
    // If not already loaded, then do it
    if (!wndTar.c4ItemCount())
    {
        TString strTmp;
        for (tCIDLib::TInt4 i4Index = i4Min; i4Index <= i4Max; i4Index++)
        {
            strTmp.SetFormatted(i4Index);
            wndTar.c4AddItem(strTmp);
        }
    }
}


//
//  Called to update the next run time display on those that support it.
//
tCIDLib::TVoid TSchedEvTab::UpdateNextTime(const tCIDLib::TResId ridTarget)
{
    const TTime tmAt(m_csrcEdit.enctAt());

    TStaticText* pwndNext;
    CastChildWnd(*this, ridTarget, pwndNext);
    TString strNext(kCQCAMsgs::midTab_EdSchEv_Next, facCQCAdmin);
    TString strTime;
    tmAt.FormatToStr(strTime, TTime::strCTime());
    strNext.eReplaceToken(strTime, kCIDLib::chDigit1);
    pwndNext->strWndText(strNext);
}
