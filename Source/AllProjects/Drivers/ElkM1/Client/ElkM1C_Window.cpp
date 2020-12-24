//
// FILE NAME: ElkM1C_Window.cpp
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
//  This file implements the main window of the Elk M1 client driver
//
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "ElkM1C_.hpp"


// ---------------------------------------------------------------------------
//  Magic macro
// ---------------------------------------------------------------------------
RTTIDecls(TElkM1CWnd,TCQCDriverClient)



// ---------------------------------------------------------------------------
//   CLASS: TElkM1CWnd
//  PREFIX: wnd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TElkM1CWnd: Constructors and Destructor
// ---------------------------------------------------------------------------
TElkM1CWnd::TElkM1CWnd( const   TCQCDriverObjCfg&   cqcdcThis
                        , const TCQCUserCtx&        cuctxToUse) :

    TCQCDriverClient
    (
        cqcdcThis
        , L"TElkM1CWnd"
        , tCQCKit::EActLevels::Low
        , cuctxToUse
    )
    , m_colModVoltages(kElkM1C::c4MaxVoltages)
    , m_colVoltages(kElkM1C::c4MaxVoltages)
    , m_fcolOutputs(kElkM1C::c4MaxOutputs)
    , m_fcolThermostats(kElkM1C::c4MaxThermostats)
    , m_fcolThermoCpls(kElkM1C::c4MaxThermoCpls)
    , m_fcolPLCUnits(kElkM1C::c4MaxPLCUnits)
    , m_fcolZones(kElkM1C::c4MaxZones)
    , m_fcolModOutputs(kElkM1C::c4MaxOutputs)
    , m_fcolModThermostats(kElkM1C::c4MaxThermostats)
    , m_fcolModThermoCpls(kElkM1C::c4MaxThermoCpls)
    , m_fcolModPLCUnits(kElkM1C::c4MaxPLCUnits)
    , m_fcolModZones(kElkM1C::c4MaxZones)
    , m_pwndClearButton(nullptr)
    , m_pwndOutputs(nullptr)
    , m_pwndPLCUnits(nullptr)
    , m_pwndSendButton(nullptr)
    , m_pwndThermostats(nullptr)
    , m_pwndThermoCpls(nullptr)
    , m_pwndVoltEQ(nullptr)
    , m_pwndVoltLims(nullptr)
    , m_pwndVoltages(nullptr)
    , m_pwndZones(nullptr)
    , m_strState_Added(L"+")
    , m_strState_Cfg(L"*")
    , m_strState_Removed(L"-")
    , m_strState_Unused(TString::strEmpty())
{
}

TElkM1CWnd::~TElkM1CWnd()
{
}



// ---------------------------------------------------------------------------
//  Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TElkM1CWnd::bChanges() const
{
    // Compare all of the current vs. mod lists
    for (tCIDLib::TCard4 c4Index = 0; c4Index < kElkM1C::c4MaxOutputs; c4Index++)
    {
        if (m_fcolOutputs[c4Index] != m_fcolModOutputs[c4Index])
            return kCIDLib::True;
    }

    for (tCIDLib::TCard4 c4Index = 0; c4Index < kElkM1C::c4MaxThermostats; c4Index++)
    {
        if (m_fcolThermostats[c4Index] != m_fcolModThermostats[c4Index])
            return kCIDLib::True;
    }

    for (tCIDLib::TCard4 c4Index = 0; c4Index < kElkM1C::c4MaxThermoCpls; c4Index++)
    {
        if (m_fcolThermoCpls[c4Index] != m_fcolModThermoCpls[c4Index])
            return kCIDLib::True;
    }

    for (tCIDLib::TCard4 c4Index = 0; c4Index < kElkM1C::c4MaxPLCUnits; c4Index++)
    {
        if (m_fcolPLCUnits[c4Index] != m_fcolModPLCUnits[c4Index])
            return kCIDLib::True;
    }

    for (tCIDLib::TCard4 c4Index = 0; c4Index < kElkM1C::c4MaxZones; c4Index++)
    {
        if (m_fcolZones[c4Index] != m_fcolModZones[c4Index])
            return kCIDLib::True;
    }

    for (tCIDLib::TCard4 c4Index = 0; c4Index < kElkM1C::c4MaxVoltages; c4Index++)
    {
        if (m_colVoltages[c4Index] != m_colModVoltages[c4Index])
            return kCIDLib::True;
    }
    return kCIDLib::False;
}


tCIDLib::TBoolean TElkM1CWnd::bSaveChanges(TString& strErrMsg)
{
    try
    {
        SendChanges();
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
//  TElkM1CWnd: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TElkM1CWnd::bCreated()
{
    TParent::bCreated();

    // Load the dialog resource that defines our content and create our content
    TDlgDesc dlgdChildren;
    facElkM1C().bCreateDlgDesc(kElkM1C::ridIntf_Main, dlgdChildren);

    tCIDCtrls::TWndId widInitFocus;
    CreateDlgItems(dlgdChildren, widInitFocus);

    // Do an initial auto-size to get them in sync with our initial size
    AutoAdjustChildren(dlgdChildren.areaPos(), areaClient());

    // Get typed pointers to the widgets we want to interact with a lot
    CastChildWnd(*this, kElkM1C::ridIntf_ClearChanges, m_pwndClearButton);
    CastChildWnd(*this, kElkM1C::ridIntf_OutputList, m_pwndOutputs);
    CastChildWnd(*this, kElkM1C::ridIntf_PLCUnitList, m_pwndPLCUnits);
    CastChildWnd(*this, kElkM1C::ridIntf_SendChanges, m_pwndSendButton);
    CastChildWnd(*this, kElkM1C::ridIntf_ThermostatList, m_pwndThermostats);
    CastChildWnd(*this, kElkM1C::ridIntf_ThermoCplList, m_pwndThermoCpls);
    CastChildWnd(*this, kElkM1C::ridIntf_VoltEQ, m_pwndVoltEQ);
    CastChildWnd(*this, kElkM1C::ridIntf_VoltLims, m_pwndVoltLims);
    CastChildWnd(*this, kElkM1C::ridIntf_VoltList, m_pwndVoltages);
    CastChildWnd(*this, kElkM1C::ridIntf_ZoneList, m_pwndZones);

    // Set up the columns for all of the list boxes
    tCIDLib::TStrList colCols(2);
    colCols.objAdd(L"  ");
    colCols.objAdd(L"Name");

    m_pwndOutputs->SetColumns(colCols);
    m_pwndOutputs->AutoSizeCol(0, kCIDLib::True);
    m_pwndPLCUnits->SetColumns(colCols);
    m_pwndPLCUnits->AutoSizeCol(0, kCIDLib::True);
    m_pwndThermostats->SetColumns(colCols);
    m_pwndThermostats->AutoSizeCol(0, kCIDLib::True);
    m_pwndThermoCpls->SetColumns(colCols);
    m_pwndThermoCpls->AutoSizeCol(0, kCIDLib::True);
    m_pwndVoltages->SetColumns(colCols);
    m_pwndVoltages->AutoSizeCol(0, kCIDLib::True);
    m_pwndZones->SetColumns(colCols);
    m_pwndZones->AutoSizeCol(0, kCIDLib::True);

    // Register event handlers
    m_pwndClearButton->pnothRegisterHandler(this, &TElkM1CWnd::eClickHandler);
    m_pwndOutputs->pnothRegisterHandler(this, &TElkM1CWnd::eLBHandler);
    m_pwndPLCUnits->pnothRegisterHandler(this, &TElkM1CWnd::eLBHandler);
    m_pwndSendButton->pnothRegisterHandler(this, &TElkM1CWnd::eClickHandler);
    m_pwndThermostats->pnothRegisterHandler(this, &TElkM1CWnd::eLBHandler);
    m_pwndThermoCpls->pnothRegisterHandler(this, &TElkM1CWnd::eLBHandler);
    m_pwndVoltages->pnothRegisterHandler(this, &TElkM1CWnd::eLBHandler);
    m_pwndZones->pnothRegisterHandler(this, &TElkM1CWnd::eLBHandler);

    //
    //  Fill in the lists with the values that they will hold. The lists
    //  won't change mostly, we'll just mark/unmark items according to config.
    //  But we do show the values configured for voltages.
    //
    colCols[0].Clear();
    colCols[1] = L"Output";
    m_pwndOutputs->RemoveAll();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < kElkM1C::c4MaxOutputs; c4Index++)
    {
        colCols[1].CapAt(6);
        colCols[1].AppendFormatted(tCIDLib::TCard4(c4Index + 1));
        m_pwndOutputs->c4AddItem(colCols, c4Index + 1);
    }
    m_pwndOutputs->SelectByIndex(0);

    for (tCIDLib::TCard4 c4Index = 0; c4Index < kElkM1C::c4MaxPLCUnits; c4Index++)
    {
        FormatPLC(c4Index, colCols[1]);
        m_pwndPLCUnits->c4AddItem(colCols, c4Index + 1);
    }
    m_pwndPLCUnits->SelectByIndex(0);

    colCols[1] = L"T-Stat";
    for (tCIDLib::TCard4 c4Index = 0; c4Index < kElkM1C::c4MaxThermostats; c4Index++)
    {
        colCols[1].CapAt(6);
        colCols[1].AppendFormatted(tCIDLib::TCard4(c4Index + 1));
        m_pwndThermostats->c4AddItem(colCols, c4Index + 1);
    }
    m_pwndThermostats->SelectByIndex(0);

    for (tCIDLib::TCard4 c4Index = 0; c4Index < kElkM1C::c4MaxThermoCpls; c4Index++)
    {
        //
        //  This one is a little more complex. There are three sets of 16
        //  and each are named differently.
        //
        tCIDLib::TCard4 c4From;
        if (c4Index < 16)
        {
            c4From = 0;
            colCols[1] = L"Probe";
        }
         else if (c4Index < 32)
        {
            c4From = 16;
            colCols[1] = L"Keypad";
        }
         else
        {
            colCols[1] = L"Thermo";
            c4From = 32;
        }
        colCols[1].AppendFormatted((c4Index - c4From) + 1);
        m_pwndThermoCpls->c4AddItem(colCols, c4Index + 1);
    }
    m_pwndThermoCpls->SelectByIndex(0);

    colCols[1] = L"Voltage";
    for (tCIDLib::TCard4 c4Index = 0; c4Index < kElkM1C::c4MaxVoltages; c4Index++)
    {
        const TVoltInfo& CurVolt = m_colVoltages[c4Index];
        colCols[1].CapAt(7);
        colCols[1].AppendFormatted(c4Index + 1);
        m_pwndVoltages->c4AddItem(colCols, c4Index + 1);
    }
    m_pwndVoltages->SelectByIndex(0);

    colCols[1] = L"Zone";
    for (tCIDLib::TCard4 c4Index = 0; c4Index < kElkM1C::c4MaxZones; c4Index++)
    {
        colCols[1].CapAt(4);
        colCols[1].AppendFormatted(tCIDLib::TCard4(c4Index + 1));
        m_pwndZones->c4AddItem(colCols, c4Index + 1);
    }
    m_pwndZones->SelectByIndex(0);

    return kCIDLib::True;
}


tCIDLib::TBoolean TElkM1CWnd::bGetConnectData(tCQCKit::TCQCSrvProxy& orbcTarget)
{
    // Do any initial load of any config
    try
    {
        LoadConfig(orbcTarget);
    }

    catch(...)
    {
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


tCIDLib::TBoolean TElkM1CWnd::bDoPoll(tCQCKit::TCQCSrvProxy& orbcTarget)
{
    //
    //  We don't really poll, but we do want to watch for it going off
    //  line, so we do a simple 'by name' field read periodically.
    //
    //  <TBD>
    //
    return kCIDLib::True;
}


tCIDLib::TVoid TElkM1CWnd::Connected()
{
    // Update all the lists
    UpdateCfgLists();
}


tCIDLib::TVoid TElkM1CWnd::DoCleanup()
{
}


tCIDLib::TVoid TElkM1CWnd::LostConnection()
{
    // Nothing for us to do currently
}


tCIDLib::TVoid TElkM1CWnd::UpdateDisplayData()
{
}


// ---------------------------------------------------------------------------
//  TElkM1CWnd: Private, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid TElkM1CWnd::ClearAllFlags()
{
    // Clear out all the flag lists
    m_fcolOutputs.SetAll(kCIDLib::False);
    m_fcolPLCUnits.SetAll(tElkM1C::EPLCTypes::Unused);
    m_fcolThermostats.SetAll(kCIDLib::False);
    m_fcolThermoCpls.SetAll(kCIDLib::False);
    m_fcolZones.SetAll(kCIDLib::False);

    m_fcolModOutputs.SetAll(kCIDLib::False);
    m_fcolModPLCUnits.SetAll(tElkM1C::EPLCTypes::Unused);
    m_fcolModThermostats.SetAll(kCIDLib::False);
    m_fcolModThermoCpls.SetAll(kCIDLib::False);
    m_fcolModZones.SetAll(kCIDLib::False);

    // The voltages we have to do differently
    for (tCIDLib::TCard4 c4Index = 0; c4Index < kElkM1C::c4MaxVoltages; c4Index++)
    {
        m_colVoltages[c4Index].Reset();
        m_colModVoltages[c4Index].Reset();
    }
}


tCIDCtrls::EEvResponses TElkM1CWnd::eClickHandler(TButtClickInfo& wnotEvent)
{
    // If we are not connected, then do nothing
    if (eConnState() != tCQCGKit::EConnStates::Connected)
        return tCIDCtrls::EEvResponses::Handled;

    try
    {
        TWndPtrJanitor janPtr(tCIDCtrls::ESysPtrs::Wait);

        // According to the button id, we do the right thing
        const tCIDCtrls::TWndId widSrc = wnotEvent.widSource();

        switch(widSrc)
        {
            case kElkM1C::ridIntf_ClearChanges :
            {
                // Clear any local modifications and update the lists
                m_colModVoltages     = m_colVoltages;
                m_fcolModOutputs     = m_fcolOutputs;
                m_fcolModPLCUnits    = m_fcolPLCUnits;
                m_fcolModThermostats = m_fcolThermostats;
                m_fcolModThermoCpls  = m_fcolThermoCpls;
                m_fcolModZones       = m_fcolZones;
                UpdateCfgLists();
                break;
            }
        };
    }

    catch(TError& errToCatch)
    {
        if (facElkM1C().bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        TExceptDlg dlgbFail
        (
            *this
            , strWndText()
            , facElkM1C().strMsg(kElkM1CMsgs::midStatus_RemOpFailed)
            , errToCatch
        );
    }
    return tCIDCtrls::EEvResponses::Handled;
}


tCIDCtrls::EEvResponses TElkM1CWnd::eLBHandler(TListChangeInfo& wnotEvent)
{
    const tCIDCtrls::TWndId widSrc = wnotEvent.widSource();

    // On double clicks, we add/remove/change the item
    if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::Cleared)
    {
        if (widSrc == kElkM1C::ridIntf_VoltList)
        {
            m_pwndVoltEQ->ClearText();
            m_pwndVoltLims->ClearText();
        }
    }
     else if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::SelChanged)
    {
        if (widSrc == kElkM1C::ridIntf_VoltList)
        {
            const tCIDLib::TCard4 c4Index = wnotEvent.c4Index();

            if (c4Index == kCIDLib::c4MaxCard)
            {
                m_pwndVoltEQ->ClearText();
                m_pwndVoltLims->ClearText();
            }
             else
            {
                const TVoltInfo& viCur = m_colModVoltages[c4Index];
                TString strFmt;
                viCur.FormatEQ(strFmt);
                m_pwndVoltEQ->strWndText(strFmt);
                viCur.FormatLims(strFmt);
                m_pwndVoltLims->strWndText(strFmt);
            }
        }
    }
     else if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::Invoked)
    {
        //
        //  The PLC units we handle differently from the others. The others
        //  are just in or out. These are out, or they are in and a binary
        //  switch, or they are in and a dimmer. So we cycle through those
        //  three options. The voltages we pop up an editing dialog.
        //
        const tCIDLib::TCard4 c4Index = wnotEvent.c4Index();

        if (widSrc == kElkM1C::ridIntf_PLCUnitList)
        {
            const tElkM1C::EPLCTypes eCfg = m_fcolPLCUnits[c4Index];
            tElkM1C::EPLCTypes eMod = m_fcolModPLCUnits[c4Index];
            if (eMod == tElkM1C::EPLCTypes::Unused)
                eMod = tElkM1C::EPLCTypes::Binary;
            else if (eMod == tElkM1C::EPLCTypes::Binary)
                eMod = tElkM1C::EPLCTypes::Dimmer;
            else
                eMod = tElkM1C::EPLCTypes::Unused;
            m_fcolModPLCUnits[c4Index] = eMod;

            if ((eMod == tElkM1C::EPLCTypes::Unused) && (eCfg == tElkM1C::EPLCTypes::Unused))
                m_pwndPLCUnits->SetColText(c4Index, 0, m_strState_Unused);
            else if ((eMod != tElkM1C::EPLCTypes::Unused) && (eCfg == tElkM1C::EPLCTypes::Unused))
                m_pwndPLCUnits->SetColText(c4Index, 0, m_strState_Added);
            else if ((eMod == tElkM1C::EPLCTypes::Unused) && (eCfg != tElkM1C::EPLCTypes::Unused))
                m_pwndPLCUnits->SetColText(c4Index, 0, m_strState_Removed);
            else
                m_pwndPLCUnits->SetColText(c4Index, 0, m_strState_Cfg);

            TString strUnit;
            FormatPLC(c4Index, strUnit);
            m_pwndPLCUnits->SetColText(c4Index, 1, strUnit);
        }
         else if (widSrc == kElkM1C::ridIntf_VoltList)
        {
            TVoltInfo& viMod = m_colModVoltages[c4Index];

            TEdM1VoltDlg dlgEdVolt;
            if (dlgEdVolt.bRunDlg(*this, c4Index, viMod))
            {
                const TVoltInfo& viCfg = m_colVoltages[c4Index];

                if (!viCfg.bUsed() && viMod.bUsed())
                    m_pwndVoltages->SetColText(c4Index, 0, m_strState_Added);
                else if (viCfg.bUsed() && !viMod.bUsed())
                    m_pwndVoltages->SetColText(c4Index, 0, m_strState_Removed);
                else if (viCfg.bUsed() && viMod.bUsed())
                    m_pwndVoltages->SetColText(c4Index, 0, m_strState_Cfg);
                else
                    m_pwndVoltages->SetColText(c4Index, 0, m_strState_Unused);

                // Update the two display values
                TString strFmt;
                viMod.FormatEQ(strFmt);
                m_pwndVoltEQ->strWndText(strFmt);
                viMod.FormatLims(strFmt);
                m_pwndVoltLims->strWndText(strFmt);
            }
        }
         else
        {
            TMultiColListBox* pwndList;
            tElkM1C::TFlagArray* pfcolCfg;
            tElkM1C::TFlagArray* pfcolMod;

            if (widSrc == kElkM1C::ridIntf_OutputList)
            {
                pwndList = m_pwndOutputs;
                pfcolCfg = &m_fcolOutputs;
                pfcolMod = &m_fcolModOutputs;
            }
             else if (widSrc == kElkM1C::ridIntf_ThermostatList)
            {
                pwndList = m_pwndThermostats;
                pfcolCfg = &m_fcolThermostats;
                pfcolMod = &m_fcolModThermostats;
            }
             else if (widSrc == kElkM1C::ridIntf_ThermoCplList)
            {
                pwndList = m_pwndThermoCpls;
                pfcolCfg = &m_fcolThermoCpls;
                pfcolMod = &m_fcolModThermoCpls;
            }
             else if (widSrc == kElkM1C::ridIntf_ZoneList)
            {
                pwndList = m_pwndZones;
                pfcolCfg = &m_fcolZones;
                pfcolMod = &m_fcolModZones;
            }

            const tCIDLib::TBoolean bCfg = pfcolCfg->tAt(c4Index);
            const tCIDLib::TBoolean bMod = !pfcolMod->tAt(c4Index);
            pfcolMod->SetAt(c4Index, bMod);

            if (bCfg && !bMod)
                pwndList->SetColText(c4Index, 0, m_strState_Removed);
            else if (!bCfg && bMod)
                pwndList->SetColText(c4Index, 0, m_strState_Added);
            else if (bCfg && bMod)
                pwndList->SetColText(c4Index, 0, m_strState_Cfg);
            else
                pwndList->SetColText(c4Index, 0, m_strState_Unused);
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}


//
//  These methods format out the stuff that we show for those that don't
//  just show a name and enabled/disabled. We have to do this during initial
//  load, during updating of the list after config and when they double
//  click to change config, so we put it here in one place.
//
//  We use the mod list, since it's either the same as the real config, or
//  or it's different and we need to reflect that local change.
//
tCIDLib::TVoid
TElkM1CWnd::FormatPLC(const tCIDLib::TCard4 c4Index, TString& strToFill)
{
    const tCIDLib::TCh chHouse = tCIDLib::TCh((c4Index / 16) + L'A');
    const tCIDLib::TCard4 c4Unit = (c4Index % 16) + 1;

    strToFill.Clear();
    strToFill.Append(chHouse);
    strToFill.Append(L'.');
    strToFill.AppendFormatted(c4Unit);
    if (m_fcolModPLCUnits[c4Index] == tElkM1C::EPLCTypes::Binary)
        strToFill.Append(L" (Binary)");
    else if (m_fcolModPLCUnits[c4Index] == tElkM1C::EPLCTypes::Dimmer)
        strToFill.Append(L" (Dimmer)");
    else
        strToFill.Append(L" (Unused)");
}


//
//  Asks the server side driver for it's current configuration. Our server
//  side driver is a CML driver, so we use the text value query to get it as
//  as formatted string. The more common config set/query methods use binary
//  flattened C++ data, and we want to avoid having to try to understand the
//  format of CML flattened data.
//
//  The version that this driver expects is version 3, and is in this format:
//
//      Version=2
//      Outputs=10011001111...
//      ThermocCpls=0011101101...
//      Zones=10101101.....
//      Thermostats=10101011...
//      PLCBlocks=BDUDDUU....
//      Voltages=
//          X=A B C L H
//      EndVoltages
//
//  Each line is a set of flags of the max number of that type of resource,
//  which indicates the configuration of each of the possible resources of
//  that type.
//
//  The PLCBlocks is a single string of 256 characters which is either D=dimmer,
//  B=binary, or U=unused. The others are just 1 or 0, for configured or not.
//
//  For voltages, we just format the ones that are in use. X is the number
//  of the voltage and A, B, and C are the quadratic equation values that
//  map the raw voltage to a useful number. L and H are the low and high
//  field limits.
//
//  For each set, we get a list of numbers of the items of that type that
//  are being polled.
//
tCIDLib::TVoid TElkM1CWnd::LoadConfig(tCQCKit::TCQCSrvProxy& orbcSrv)
{
    try
    {
        //
        //  Ask the driver for the config data. It takes a 'command id'
        //  but we only use this method for one thing, so we just pass
        //  L"Driver". It's ignored on the other side.
        //
        TString strConfig(0x10000UL);
        orbcSrv->bQueryTextVal(strMoniker(), L"Driver", L"Config", strConfig);

        // Make an input stream over the data and parse it in
        TTextStringInStream strmIn(&strConfig);
        TString strCurLn;

        // Figure out the version
        tCIDLib::TCard4 c4Version = 1;
        if (strConfig.bStartsWith(L"Version="))
        {
            strmIn >> strCurLn;
            strCurLn.Cut(0, 8);
            c4Version = strCurLn.c4Val(tCIDLib::ERadices::Dec);
        }

        //
        //  If we are at the end, then there's no config, so just clear
        //  all the flags so that nothing is configured. If it should some
        //  how be a version 1, then also clear it. Else, we need to
        //  parse it. We'll parse into temps first so that, if something
        //  goes awry, we won't have screwed up our current config.
        //
        if (strmIn.bEndOfStream() || (c4Version < 2))
        {
            ClearAllFlags();
        }
         else
        {
            // Use some temps until we know the config is good
            tElkM1C::TVoltArray colVoltages(kElkM1C::c4MaxVoltages);
            tElkM1C::TFlagArray fcolOutputs(kElkM1C::c4MaxOutputs);
            tElkM1C::TPLCArray  fcolPLCUnits(kElkM1C::c4MaxPLCUnits);
            tElkM1C::TFlagArray fcolThermostats(kElkM1C::c4MaxThermostats);
            tElkM1C::TFlagArray fcolThermoCpls(kElkM1C::c4MaxThermoCpls);
            tElkM1C::TFlagArray fcolZones(kElkM1C::c4MaxZones);

            // And pull in the config data
            strmIn >> strCurLn;
            ParseFlagList(fcolOutputs, strCurLn, L"Outputs=");
            strmIn >> strCurLn;
            ParseFlagList(fcolThermoCpls, strCurLn, L"ThermoCpls=");
            strmIn >> strCurLn;
            ParseFlagList(fcolZones, strCurLn, L"Zones=");
            strmIn >> strCurLn;
            ParseFlagList(fcolThermostats, strCurLn, L"Thermostats=");
            strmIn >> strCurLn;
            ParsePLCList(fcolPLCUnits, strCurLn, L"PLCUnits=");

            // If version 3 or greater, parse the voltages
            if (c4Version >= 3)
                ParseVoltages(colVoltages, strmIn);

            // It went ok, so store the temps into our real arrays
            m_colVoltages     = colVoltages;
            m_fcolOutputs     = fcolOutputs;
            m_fcolPLCUnits    = fcolPLCUnits;
            m_fcolThermostats = fcolThermostats;
            m_fcolThermoCpls  = fcolThermoCpls;
            m_fcolZones       = fcolZones;

            // And set the mod versions to the same
            m_colModVoltages     = colVoltages;
            m_fcolModOutputs     = fcolOutputs;
            m_fcolModPLCUnits    = fcolPLCUnits;
            m_fcolModThermostats = fcolThermostats;
            m_fcolModThermoCpls  = fcolThermoCpls;
            m_fcolModZones       = fcolZones;
        }
    }

    catch(TError& errToCatch)
    {
        if (facElkM1C().bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        TExceptDlg dlgbFail
        (
            *this
            , strWndText()
            , facElkM1C().strMsg(kElkM1CErrs::errcCfg_CantParseCfg)
            , errToCatch
        );
    }
}


//
//  Helpers used by the LoadConfig method above. Some parse a single line of
//  config values and set the flags in the passed flag array to represent the
//  configuration values passed. Others get the stream and parse out more
//  complex stuff.
//
//  The passed config line will be modified.
//
tCIDLib::TVoid
TElkM1CWnd::ParseFlagList(          tElkM1C::TFlagArray&    fcolToFill
                            ,       TString&                strValues
                            , const TString&                strExpectedPrefix)
{
    // Make sure it starts with the expected prefix
    if (!strValues.bStartsWith(strExpectedPrefix))
    {
        facElkM1C().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kElkM1CErrs::errcCfg_InvalidCfgLine
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , strExpectedPrefix
        );
    }

    // It does, so strip the prefix off, so we can directly index the values
    strValues.Cut(0, strExpectedPrefix.c4Length());

    // Clear the flags
    fcolToFill.SetAll(kCIDLib::False);

    // Get the count of elements this one can take
    const tCIDLib::TCard4 c4Count = fcolToFill.c4ElemCount();

    // Make sure it has the expected number of values
    if (strValues.c4Length() != c4Count)
    {
        facElkM1C().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kElkM1CErrs::errcCfg_InvalidCfgLine
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , strExpectedPrefix
        );
    }

    // Do the expected number of values
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (strValues[c4Index] == L'1')
            fcolToFill[c4Index] = kCIDLib::True;
        else if (strValues[c4Index] == L'0')
            fcolToFill[c4Index] = kCIDLib::False;
        else
        {
            facElkM1C().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kElkM1CErrs::errcCfg_InvalidCfgLine
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
                , strExpectedPrefix
            );
        }
    }
}

tCIDLib::TVoid
TElkM1CWnd::ParsePLCList(       tElkM1C::TPLCArray& fcolToFill
                        ,       TString&            strValues
                        , const TString&            strExpectedPrefix)
{
    // Make sure it starts with the expected prefix
    if (!strValues.bStartsWith(strExpectedPrefix))
    {
        facElkM1C().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kElkM1CErrs::errcCfg_InvalidCfgLine
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , strExpectedPrefix
        );
    }

    // It does, so strip the prefix off, so we can directly index the values
    strValues.Cut(0, strExpectedPrefix.c4Length());

    // Clear the flags
    fcolToFill.SetAll(tElkM1C::EPLCTypes::Unused);

    // Get the count of elements this one can take
    const tCIDLib::TCard4 c4Count = fcolToFill.c4ElemCount();

    // Make sure it has the expected number of values
    if (strValues.c4Length() != c4Count)
    {
        facElkM1C().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kElkM1CErrs::errcCfg_InvalidCfgLine
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , strExpectedPrefix
        );
    }

    // Do the expected number of values
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (strValues[c4Index] == L'U')
            fcolToFill[c4Index] = tElkM1C::EPLCTypes::Unused;
        else if (strValues[c4Index] == L'B')
            fcolToFill[c4Index] = tElkM1C::EPLCTypes::Binary;
        else if (strValues[c4Index] == L'D')
            fcolToFill[c4Index] = tElkM1C::EPLCTypes::Dimmer;
        else
        {
            facElkM1C().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kElkM1CErrs::errcCfg_InvalidCfgLine
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
                , strExpectedPrefix
            );
        }
    }
}


//
//  We assume here that the passed array is all initialized to mark them
//  all as unused and with default equation values. We only fill in the
//  ones that are parsed from the configuration.
//
tCIDLib::TVoid
TElkM1CWnd::ParseVoltages(  tElkM1C::TVoltArray&    colToFill
                            , TTextInStream&        strmIn)
{
    static const TString strStart(L"Voltages=");
    static const TString strEnd(L"EndVoltages");
    TString strToken;
    TString strIndex;
    TString strAVal;
    TString strBVal;
    TString strCVal;
    TString strHighLim;
    TString strLowLim;

    // First we should see Voltages=
    strmIn >> strToken;
    strToken.StripWhitespace();
    if (strToken != strToken)
    {
        facElkM1C().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kElkM1CErrs::errcCfg_InvalidCfgLine
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , strStart
        );
    }

    // Now loop till we see the end
    TStringTokenizer stokLine;
    while(kCIDLib::True)
    {
        strmIn >> strToken;
        strToken.StripWhitespace();

        // Break out at the end marker
        if (strToken == strEnd)
            break;

        // We got one, so parse it out
        stokLine.Reset(&strToken, L"= ");
        if (!stokLine.bGetNextToken(strIndex)
        ||  !stokLine.bGetNextToken(strAVal)
        ||  !stokLine.bGetNextToken(strBVal)
        ||  !stokLine.bGetNextToken(strCVal)
        ||  !stokLine.bGetNextToken(strLowLim)
        ||  !stokLine.bGetNextToken(strHighLim))
        {
            facElkM1C().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kElkM1CErrs::errcCfg_InvalidCfgLine
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
                , TString(L"voltage")
            );
        }

        tCIDLib::TCard4 c4Index;
        tCIDLib::TFloat8 f8AVal, f8BVal, f8CVal, f8LowLim, f8HighLim;
        if (!strIndex.bToCard4(c4Index, tCIDLib::ERadices::Dec)
        ||  !strAVal.bToFloat8(f8AVal)
        ||  !strBVal.bToFloat8(f8BVal)
        ||  !strCVal.bToFloat8(f8CVal)
        ||  !strLowLim.bToFloat8(f8LowLim)
        ||  !strHighLim.bToFloat8(f8HighLim))
        {
            facElkM1C().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kElkM1CErrs::errcCfg_InvalidCfgLine
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
                , TString(L"voltage")
            );
        }

        colToFill[c4Index].Set
        (
            kCIDLib::True, f8AVal, f8BVal, f8CVal, f8LowLim, f8HighLim
        );
    }
}


//
//  Sends current configuration changes to the server side for storage. If it
//  goes well, all the config changes are cleared and we are back in sync with
//  the server.
//
tCIDLib::TVoid
TElkM1CWnd::SendChangesFmtFlags(const   TString&                strPrefix
                                , const tElkM1C::TFlagArray&    fcolCfg
                                ,       TTextOutStream&         strmTar)
{
    strmTar << strPrefix << L"=";
    const tCIDLib::TCard4 c4Count = fcolCfg.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (fcolCfg[c4Index])
            strmTar << L'1';
        else
            strmTar << L'0';
    }
    strmTar << kCIDLib::NewLn;
}


tCIDLib::TVoid TElkM1CWnd::SendChanges()
{
    // Format out the config in the exchange format
    TTextStringOutStream strmConfig(0x1000UL);
    TString strFmt;

    // Put out the version first
    strmConfig << L"Version=" << kElkM1C::c4ConfigVer << kCIDLib::NewLn;

    // Do the binary flag based lists first
    SendChangesFmtFlags(L"Outputs", m_fcolModOutputs, strmConfig);
    SendChangesFmtFlags(L"ThermoCpls", m_fcolModThermoCpls, strmConfig);
    SendChangesFmtFlags(L"Zones", m_fcolModZones, strmConfig);
    SendChangesFmtFlags(L"Thermostats", m_fcolModThermostats, strmConfig);

    // Do the PLC Units here since they aren't just boolean falgs
    strmConfig << L"PLCUnits=";
    for (tCIDLib::TCard4 c4Index = 0; c4Index < kElkM1C::c4MaxPLCUnits; c4Index++)
    {
        if (m_fcolModPLCUnits[c4Index] == tElkM1C::EPLCTypes::Unused)
            strmConfig << L'U';
        else if (m_fcolModPLCUnits[c4Index] == tElkM1C::EPLCTypes::Binary)
            strmConfig << L'B';
        else
            strmConfig << L'D';
    }
    strmConfig << kCIDLib::NewLn;

    // Do the voltages
    strmConfig << L"Voltages=\n";
    for (tCIDLib::TCard4 c4Index = 0; c4Index < kElkM1C::c4MaxVoltages; c4Index++)
    {
        const TVoltInfo& viCur = m_colModVoltages[c4Index];
        if (!viCur.bUsed())
            continue;

        //
        //  Note we do a zero based index here to send to the Elk, whereas
        //  we do a 1 based one elsewhere here for display.
        //
        viCur.FormatCfg(strFmt, c4Index);
        strmConfig << strFmt << kCIDLib::NewLn;
    }
    strmConfig << L"EndVoltages\n";

    // Ok, we can now send it to the driver
    strmConfig.Flush();
    orbcServer()->c4SendCmd(strMoniker(), L"TakeChanges", strmConfig.strData(), sectUser());

#if CID_DEBUG_ON
//TTextFileOutStream strmTest
//(
//    L"I:\\Test.Txt"
//    , tCIDLib::ECreateActs::CreateAlways
//    , tCIDLib::EFilePerms::Default
//    , tCIDLib::EFileFlags::SequentialScan
//    , tCIDLib::EAccessModes::Write
//);
//strmTest << strmConfig.strData() << kCIDLib::FlushIt;
#endif

    // Now reload the config, to get our window back in sync
    LoadConfig(orbcServer());
    Connected();
}


//
//  These methods handle updating the config lists to reflect the current state
//  of the state.
//
tCIDLib::TVoid TElkM1CWnd::UpdateCfgLists()
{
    // The boolean flag oriented ones we can do via a generic helper
    UpdateFlagCfgList(*m_pwndOutputs, m_fcolOutputs, m_fcolModOutputs);
    UpdateFlagCfgList(*m_pwndThermostats, m_fcolThermostats, m_fcolModThermostats);
    UpdateFlagCfgList(*m_pwndThermoCpls, m_fcolThermoCpls, m_fcolModThermoCpls);
    UpdateFlagCfgList(*m_pwndZones, m_fcolZones, m_fcolModZones);

    TWndPaintJanitor janUnits(m_pwndPLCUnits);
    TString strFmt;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < kElkM1C::c4MaxPLCUnits; c4Index++)
    {
        const tElkM1C::EPLCTypes eCfg = m_fcolPLCUnits[c4Index];
        const tElkM1C::EPLCTypes eMod = m_fcolModPLCUnits[c4Index];

        if (eMod == tElkM1C::EPLCTypes::Unused)
        {
            m_pwndPLCUnits->SetColText(c4Index, 0, L" ");
        }
         else if (eCfg == eMod)
        {
            m_pwndPLCUnits->SetColText(c4Index, 0, m_strState_Cfg);
        }
         else
        {
            if (eCfg == tElkM1C::EPLCTypes::Unused)
                m_pwndPLCUnits->SetColText(c4Index, 0, m_strState_Added);
            else
                m_pwndPLCUnits->SetColText(c4Index, 0, m_strState_Removed);
        }
        FormatPLC(c4Index, strFmt);
        m_pwndPLCUnits->SetColText(c4Index, 1, strFmt);
    }

    TWndPaintJanitor janVolts(m_pwndVoltages);
    for (tCIDLib::TCard4 c4Index = 0; c4Index < kElkM1C::c4MaxVoltages; c4Index++)
    {
        const TVoltInfo& viCfg = m_colVoltages[c4Index];
        const TVoltInfo& viMod = m_colModVoltages[c4Index];

        if (viCfg == viMod)
        {
            if (viMod.bUsed())
                m_pwndVoltages->SetColText(c4Index, 0, m_strState_Cfg);
            else
                m_pwndVoltages->SetColText(c4Index, 0, m_strState_Unused);
        }
         else
        {
            if (viCfg.bUsed() == viMod.bUsed())
                m_pwndVoltages->SetColText(c4Index, 0, m_strState_Added);
            else if (viCfg.bUsed())
                m_pwndVoltages->SetColText(c4Index, 0, m_strState_Removed);
            else
                m_pwndVoltages->SetColText(c4Index, 0, m_strState_Added);
        }
    }
    // Force an update so the two voltage display values get updated
    m_pwndVoltages->SelectByIndex(m_pwndVoltages->c4CurItem(), kCIDLib::True);
}


tCIDLib::TVoid
TElkM1CWnd::UpdateFlagCfgList(          TMultiColListBox&       wndTar
                                , const tElkM1C::TFlagArray&    fcolCfg
                                , const tElkM1C::TFlagArray&    fcolMod)
{
    TWndPaintJanitor janPaint(&wndTar);

    const tCIDLib::TCard4 c4Count = fcolCfg.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const tCIDLib::TBoolean bCfg = fcolCfg[c4Index];
        const tCIDLib::TBoolean bMod = fcolMod[c4Index];

        if (bMod)
        {
            if (bCfg == bMod)
            {
                // There are on local mods, so it's either configured or not
                wndTar.SetColText(c4Index, 0, m_strState_Cfg);
            }
             else
            {
                // We have local mods, so show mod state
                if (bCfg)
                    wndTar.SetColText(c4Index, 0, m_strState_Removed);
                else
                    wndTar.SetColText(c4Index, 0, m_strState_Added);
            }
        }
         else
        {
            wndTar.SetColText(c4Index, 0, m_strState_Unused);
        }
    }
}

