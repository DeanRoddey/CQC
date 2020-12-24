//
// FILE NAME: HAIOmniC_Window.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/12/2006
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
//  This file implements the main window of the HAI Omni client driver
//
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "HAIOmniC_.hpp"


// ---------------------------------------------------------------------------
//  Magic macro
// ---------------------------------------------------------------------------
RTTIDecls(THAIOmniCWnd,TCQCDriverClient)



// ---------------------------------------------------------------------------
//   CLASS: THAIOmniCWnd
//  PREFIX: wnd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  THAIOmniCWnd: Constructors and Destructor
// ---------------------------------------------------------------------------
THAIOmniCWnd::THAIOmniCWnd( const   TCQCDriverObjCfg&   cqcdcThis
                            , const TCQCUserCtx&        cuctxToUse) :

    TCQCDriverClient
    (
        cqcdcThis
        , L"THAIOmniCWnd"
        , tCQCKit::EActLevels::Low
        , cuctxToUse
    )
    , m_fcolModAreas(kHAIOmniSh::c4MaxAreas)
    , m_fcolModThermos(kHAIOmniSh::c4MaxThermos)
    , m_fcolModUnits(kHAIOmniSh::c4MaxUnits)
    , m_fcolModZones(kHAIOmniSh::c4MaxZones)
    , m_pwndAreas(0)
    , m_pwndClearButton(0)
    , m_pwndSendButton(0)
    , m_pwndThermos(0)
    , m_pwndUnits(0)
    , m_pwndZones(0)
{
    // Override the default background color
    wclrBgn().eSysColor(tCIDWnd::ESysClr_DialogBgn);
}

THAIOmniCWnd::~THAIOmniCWnd()
{
}


// ---------------------------------------------------------------------------
//  THAIOmniCWnd: Public, non-virtual methods
// ---------------------------------------------------------------------------


// ---------------------------------------------------------------------------
//  THAIOmniCWnd: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
THAIOmniCWnd::bGetConnectData(tCQCKit::TCQCSrvProxy& orbcTarget)
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


tCIDLib::TBoolean THAIOmniCWnd::bDoPoll(tCQCKit::TCQCSrvProxy& orbcTarget)
{
    //
    //  We don't really poll, but we do want to watch for it going off
    //  line, so we do a simple 'by name' field read periodically.
    //
    return kCIDLib::True;
}


tCIDLib::TVoid THAIOmniCWnd::Connected()
{
    // Update all the lists
    UpdateCfgLists();
}


tCIDLib::TVoid THAIOmniCWnd::Created(const TArea& areaInit)
{
    //
    //  We store our widgets as a dialog resource. The dialog box class
    //  provides a convenient method for non-dialogs to load up their
    //  children from a dialog description resource.
    //
    tCIDWnd::TWindowId widInitFocus;
    TDlgBox::CreateChildrenFromDD
    (
        *this, facHAIOmniC(), kHAIOmniC::ridIntf_Main, widInitFocus
    );

    //
    //  Set our frame icon.
    //
//    TIcon icoDrv(facCQCGKit(), kCQCGKit::ridIcon_SmallIR);
//    pwndContainingFrame()->SetFrameIcons(icoDrv, icoDrv);

    // And now pass on to our parent
    TParent::Created(areaInit);

    // Get typed pointers to the widgets we want to interact with a lot
    CastChildWnd(*this, kHAIOmniC::ridIntf_AreaList, m_pwndAreas);
    CastChildWnd(*this, kHAIOmniC::ridIntf_ClearChanges, m_pwndClearButton);
    CastChildWnd(*this, kHAIOmniC::ridIntf_SendChanges, m_pwndSendButton);
    CastChildWnd(*this, kHAIOmniC::ridIntf_ThermoList, m_pwndThermos);
    CastChildWnd(*this, kHAIOmniC::ridIntf_UnitList, m_pwndUnits);
    CastChildWnd(*this, kHAIOmniC::ridIntf_ZoneList, m_pwndZones);

    // Register event handlers
    m_pwndAreas->pnothRegisterHandler(this, &THAIOmniCWnd::eLBHandler);
    m_pwndClearButton->pnothRegisterHandler(this, &THAIOmniCWnd::eClickHandler);
    m_pwndSendButton->pnothRegisterHandler(this, &THAIOmniCWnd::eClickHandler);
    m_pwndThermos->pnothRegisterHandler(this, &THAIOmniCWnd::eLBHandler);
    m_pwndUnits->pnothRegisterHandler(this, &THAIOmniCWnd::eLBHandler);
    m_pwndZones->pnothRegisterHandler(this, &THAIOmniCWnd::eLBHandler);

    //
    //  Load up the images we use. We have one for an item that is currently
    //  configured, one for an item that's been selected but not uploaded
    //  yet, and one for an item that's been removed but not uploaded yet.
    //
    m_bmpAdded = facCIDWnd().bmpLoadSmallAppImg(L"Right Arrow");
    m_bmpCfg = facCIDWnd().bmpLoadSmallAppImg(L"Red Check");
    m_bmpRemoved = facCIDWnd().bmpLoadSmallAppImg(L"Left Arrow");

    //
    //  Fill in the lists with the values that they will hold. The lists
    //  won't change, we'll just mark/unmark items according to config.
    //
    TString strName;
    m_pwndAreas->RemoveAll();
    for (tCIDLib::TCard4 c4Index = 1; c4Index <= kHAIOmniSh::c4MaxAreas; c4Index++)
    {
        TOmniDrvCfg::c4MakeItemName(tHAIOmniSh::EItem_Area, c4Index, strName);
        m_pwndAreas->AddItem(new TListWndImageItem(strName, m_bmpCfg));
    }

    m_pwndThermos->RemoveAll();
    for (tCIDLib::TCard4 c4Index = 1; c4Index <= kHAIOmniSh::c4MaxThermos; c4Index++)
    {
        TOmniDrvCfg::c4MakeItemName(tHAIOmniSh::EItem_Thermo, c4Index, strName);
        m_pwndThermos->AddItem(new TListWndImageItem(strName, m_bmpCfg));
    }

    m_pwndUnits->RemoveAll();
    for (tCIDLib::TCard4 c4Index = 1; c4Index <= kHAIOmniSh::c4MaxUnits; c4Index++)
    {
        TOmniDrvCfg::c4MakeItemName(tHAIOmniSh::EItem_Unit, c4Index, strName);
        m_pwndUnits->AddItem(new TListWndImageItem(strName, m_bmpCfg));
    }

    m_pwndZones->RemoveAll();
    for (tCIDLib::TCard4 c4Index = 1; c4Index <= kHAIOmniSh::c4MaxZones; c4Index++)
    {
        TOmniDrvCfg::c4MakeItemName(tHAIOmniSh::EItem_Zone, c4Index, strName);
        m_pwndZones->AddItem(new TListWndImageItem(strName, m_bmpCfg));
    }
}


tCIDLib::TVoid THAIOmniCWnd::DoCleanup()
{
}


tCIDLib::TVoid THAIOmniCWnd::UpdateDisplayData()
{
}


// ---------------------------------------------------------------------------
//  THAIOmniCWnd: Private, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid THAIOmniCWnd::ClearAllFlags()
{
    // Clear out all the flag lists
    m_fcolModAreas.SetAll(kCIDLib::False);
    m_fcolModThermos.SetAll(kCIDLib::False);
    m_fcolModUnits.SetAll(tHAIOmniSh::EUnit_Unused);
    m_fcolModZones.SetAll(tHAIOmniSh::EZone_Unused);

    m_dcfgCurrent.Reset();
}


//
//  This is called on a button click. Mostly what we do is set up a string
//  to pass to the driver's Command field, but some will us the backdoor
//  c4SendCmd() method for things that no fields exist.
//
//  NOTE:   Groups and units can have spaces in their names so be sure to
//          quote them in the send strings.
//
tCIDWnd::EEventResponses
THAIOmniCWnd::eClickHandler(TButtClickInfo& wnotEvent)
{
    // If we are not connected, then do nothing
    if (eConnState() != tCQCGKit::EConnStates::Connected)
        return tCIDWnd::EEventResponse_Handled;

    try
    {
        TWndPtrJanitor janPtr(tCIDWnd::ESysPointer_Wait);

        // According to the button id, we do the right thing
        const tCIDWnd::TWindowId widSrc = wnotEvent.widSource();

        switch(widSrc)
        {
            case kHAIOmniC::ridIntf_ClearChanges :
            {
                // Clear any local modifications and update the lists
                m_fcolModAreas      = m_dcfgCurrent.fcolAreas();
                m_fcolModThermos    = m_dcfgCurrent.fcolThermos();
                m_fcolModUnits      = m_dcfgCurrent.fcolPLCUnits();
                m_fcolModZones      = m_dcfgCurrent.fcolZones();
                UpdateCfgLists();
                break;
            }

            case kHAIOmniC::ridIntf_SendChanges :
            {
                THeapBuf mbufCfg(1024 * 16);
                TBinMBufOutStream strmOut(&mbufCfg);

                // Create a temp config from the modified lists
                TOmniDrvCfg dcfgTmp
                (
                    m_fcolModAreas
                    , m_fcolModUnits
                    , m_fcolModThermos
                    , m_fcolModZones
                );
                strmOut << dcfgTmp << kCIDLib::FlushIt;
                orbcServer()->SetConfig
                (
                    strMoniker()
                    , strmOut.c4CurSize()
                    , mbufCfg
                    , cuctxToUse().sectUser()
                    , tCQCKit::EDrvCmdWaits::Wait
                );

                // Get ourself back into sync
                LoadConfig(orbcServer());
                Connected();
                break;
            }
        };
    }

    catch(const TError& errToCatch)
    {
        if (facHAIOmniC().bShouldLog(errToCatch))
            TModule::LogEventObj(errToCatch);

        TExceptDlg dlgbFail
        (
            wndContainingFrame()
            , strText()
            , facHAIOmniC().strMsg(kOmniCMsgs::midStatus_RemOpFailed)
            , errToCatch
        );
    }
    return tCIDWnd::EEventResponse_Handled;
}


tCIDWnd::EEventResponses
THAIOmniCWnd::eLBHandler(TListChangeInfo& wnotEvent)
{
    // On double clicks, we add/remove/change the item
    if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::Invoked)
    {
        const tCIDWnd::TWindowId widSrc = wnotEvent.widSource();

        //
        //  The PLC/Zones units we handle differently from the others. The
        //  others are just in or out. The others must be cycled through
        //  the options.
        //
        tCIDLib::TCard4 c4Index = wnotEvent.c4NewIndex();
        TListWndImageItem* plwiCur;

        if (widSrc == kHAIOmniC::ridIntf_UnitList)
        {
            const tHAIOmniSh::EUnitTypes eCfg = m_dcfgCurrent.fcolPLCUnits()[c4Index];
            tHAIOmniSh::EUnitTypes eMod = m_fcolModUnits[c4Index];
            if (c4Index < 256)
            {
                // Can be binary or dimmer or unused
                if (eMod == tHAIOmniSh::EUnit_Unused)
                    eMod = tHAIOmniSh::EUnit_Binary;
                else if (eMod == tHAIOmniSh::EUnit_Binary)
                    eMod = tHAIOmniSh::EUnit_Dimmer;
                else
                    eMod = tHAIOmniSh::EUnit_Unused;
            }
             else if (c4Index < 392)
            {
                // Can be binary or unused
                if (eMod == tHAIOmniSh::EUnit_Unused)
                    eMod = tHAIOmniSh::EUnit_Binary;
                else
                    eMod = tHAIOmniSh::EUnit_Unused;
            }
             else
            {
                // Can be flag or unused
                if (eMod == tHAIOmniSh::EUnit_Unused)
                    eMod = tHAIOmniSh::EUnit_Flag;
                else
                    eMod = tHAIOmniSh::EUnit_Unused;
            }

            m_fcolModUnits[c4Index] = eMod;
            plwiCur = m_pwndUnits->plwiCurrentAs<TListWndImageItem>();
            if (eCfg == eMod)
            {
                plwiCur->bmpImage(m_bmpCfg);
                plwiCur->bSuppressImage(eCfg == tHAIOmniSh::EUnit_Unused);
            }
             else
            {
                if (eCfg == tHAIOmniSh::EUnit_Unused)
                    plwiCur->bmpImage(m_bmpAdded);
                else
                    plwiCur->bmpImage(m_bmpRemoved);
                plwiCur->bSuppressImage(kCIDLib::False);
            }

            TString strName;
            MakeUnitText(c4Index + 1, eMod, strName);
            plwiCur->strText(strName);
            m_pwndUnits->RefreshCurrent();
        }
         else if (widSrc == kHAIOmniC::ridIntf_ZoneList)
        {
            const tHAIOmniSh::EZoneTypes eCfg = m_dcfgCurrent.fcolZones()[c4Index];
            tHAIOmniSh::EZoneTypes eMod = m_fcolModZones[c4Index];
            if (eMod == tHAIOmniSh::EZone_Unused)
                eMod = tHAIOmniSh::EZone_Alarm;
            else if (eMod == tHAIOmniSh::EZone_Alarm)
                eMod = tHAIOmniSh::EZone_Humidity;
            else if (eMod == tHAIOmniSh::EZone_Humidity)
                eMod = tHAIOmniSh::EZone_Temp;
            else
                eMod = tHAIOmniSh::EZone_Unused;

            m_fcolModZones[c4Index] = eMod;
            plwiCur = m_pwndZones->plwiCurrentAs<TListWndImageItem>();
            if (eCfg == eMod)
            {
                plwiCur->bmpImage(m_bmpCfg);
                plwiCur->bSuppressImage(eCfg == tHAIOmniSh::EZone_Unused);
            }
             else
            {
                if (eCfg == tHAIOmniSh::EZone_Unused)
                    plwiCur->bmpImage(m_bmpAdded);
                else
                    plwiCur->bmpImage(m_bmpRemoved);
                plwiCur->bSuppressImage(kCIDLib::False);
            }

            TString strName;
            MakeZoneText(c4Index + 1, eMod, strName);
            plwiCur->strText(strName);
            m_pwndZones->RefreshCurrent();
        }
         else
        {
            TListBox* pwndList;
            tHAIOmniSh::TFlagArray* pfcolCfg;
            tHAIOmniSh::TFlagArray* pfcolMod;

            if (widSrc == kHAIOmniC::ridIntf_AreaList)
            {
                pwndList = m_pwndAreas;
                pfcolCfg = &m_dcfgCurrent.fcolAreas();
                pfcolMod = &m_fcolModAreas;
            }
             else if (widSrc == kHAIOmniC::ridIntf_ThermoList)
            {
                pwndList = m_pwndThermos;
                pfcolCfg = &m_dcfgCurrent.fcolThermos();
                pfcolMod = &m_fcolModThermos;
            }

            const tCIDLib::TBoolean bCfg = pfcolCfg->tAt(c4Index);
            const tCIDLib::TBoolean bMod = !pfcolMod->tAt(c4Index);
            pfcolMod->SetAt(c4Index, bMod);

            plwiCur = pwndList->plwiCurrentAs<TListWndImageItem>();
            if (bCfg == bMod)
            {
                plwiCur->bmpImage(m_bmpCfg);
                plwiCur->bSuppressImage(!bCfg);
            }
             else
            {
                if (bCfg)
                    plwiCur->bmpImage(m_bmpRemoved);
                else
                    plwiCur->bmpImage(m_bmpAdded);
                plwiCur->bSuppressImage(kCIDLib::False);
            }
            pwndList->RefreshCurrent();
        }
    }
    return tCIDWnd::EEventResponse_Handled;
}


//
//  Asks the server side driver for it's current configuration. Our server
//  side driver is a C++ driver, so we can get a binary blob back. We use
//  one of the backdoor driver calls to get it.
//
tCIDLib::TVoid THAIOmniCWnd::LoadConfig(tCQCKit::TCQCSrvProxy& orbcSrv)
{
    try
    {
        THeapBuf mbufCfg(8192UL);
        tCIDLib::TCard4 c4Sz;
        orbcSrv->QueryConfig(strMoniker(), c4Sz, mbufCfg);

        // Make an input stream over the data and stream in the config
        TBinMBufInStream strmIn(&mbufCfg, c4Sz);
        strmIn >> m_dcfgCurrent;

        //
        //  Set all the modified lists to the same values as the current
        //  lists, so that we start off with no changes.
        //
        m_fcolModAreas = m_dcfgCurrent.fcolAreas();
        m_fcolModUnits = m_dcfgCurrent.fcolPLCUnits();
        m_fcolModThermos = m_dcfgCurrent.fcolThermos();
        m_fcolModZones = m_dcfgCurrent.fcolZones();
    }

    catch(const TError& errToCatch)
    {
        if (facHAIOmniC().bShouldLog(errToCatch))
            TModule::LogEventObj(errToCatch);

        TExceptDlg dlgbFail
        (
            wndContainingFrame()
            , strText()
            , facHAIOmniC().strMsg(kOmniCErrs::errcCfg_CantLoadConfig)
            , errToCatch
        );
    }
}


// Builds up the text that we show for unit configuration lines
tCIDLib::TVoid
THAIOmniCWnd::MakeUnitText( const   tCIDLib::TCard4         c4Index
                            , const tHAIOmniSh::EUnitTypes  eCfg
                            ,       TString&                strToFill)
{
    TOmniDrvCfg::c4MakeItemName(tHAIOmniSh::EItem_Unit, c4Index, strToFill);
    if (eCfg == tHAIOmniSh::EUnit_Binary)
        strToFill.Append(L" (Binary)");
    else if (eCfg == tHAIOmniSh::EUnit_Dimmer)
        strToFill.Append(L" (Dimmer)");
    else if (eCfg == tHAIOmniSh::EUnit_Flag)
        strToFill.Append(L" (Counter)");
}


// Builds up the text that we show for zone configuration lines
tCIDLib::TVoid
THAIOmniCWnd::MakeZoneText( const   tCIDLib::TCard4         c4Index
                            , const tHAIOmniSh::EZoneTypes  eCfg
                            ,       TString&                strToFill)
{
    TOmniDrvCfg::c4MakeItemName(tHAIOmniSh::EItem_Zone, c4Index, strToFill);
    if (eCfg == tHAIOmniSh::EZone_Alarm)
        strToFill.Append(L" (Alarm)");
    else if (eCfg == tHAIOmniSh::EZone_Humidity)
        strToFill.Append(L" (Humidity)");
    else if (eCfg == tHAIOmniSh::EZone_Temp)
        strToFill.Append(L" (Temp)");
}


//
//  These methods handle updating the config lists to reflect the current state
//  of the state.
//
tCIDLib::TVoid THAIOmniCWnd::UpdateCfgLists()
{
    // The boolean flag oriented ones we can do via a generic helper
    UpdateFlagCfgList(*m_pwndAreas, m_dcfgCurrent.fcolAreas(), m_fcolModAreas);
    UpdateFlagCfgList(*m_pwndThermos, m_dcfgCurrent.fcolThermos(), m_fcolModThermos);

    // The others we have to do separately, so do the units
    TString strUnit;
    tHAIOmniSh::TPLCArray& fcolUnits = m_dcfgCurrent.fcolPLCUnits();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < kHAIOmniSh::c4MaxUnits; c4Index++)
    {
        TListWndImageItem* plwiCur = m_pwndUnits->plwiAtAs<TListWndImageItem>(c4Index);
        const tHAIOmniSh::EUnitTypes eCfg = fcolUnits[c4Index];
        const tHAIOmniSh::EUnitTypes eMod = m_fcolModUnits[c4Index];

        MakeUnitText(c4Index + 1, eCfg, strUnit);
        if (eCfg == eMod)
        {
            plwiCur->bmpImage(m_bmpCfg);
            plwiCur->bSuppressImage(eCfg == tHAIOmniSh::EUnit_Unused);
        }
         else
        {
            if (eCfg == tHAIOmniSh::EUnit_Unused)
                plwiCur->bmpImage(m_bmpAdded);
            else
                plwiCur->bmpImage(m_bmpRemoved);
            plwiCur->bSuppressImage(kCIDLib::False);
        }
        plwiCur->strText(strUnit);
    }
    m_pwndUnits->Refresh();

    // And the zones
    TString strZone;
    tHAIOmniSh::TZoneArray& fcolZones = m_dcfgCurrent.fcolZones();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < kHAIOmniSh::c4MaxZones; c4Index++)
    {
        TListWndImageItem* plwiCur = m_pwndZones->plwiAtAs<TListWndImageItem>(c4Index);
        const tHAIOmniSh::EZoneTypes eCfg = fcolZones[c4Index];
        const tHAIOmniSh::EZoneTypes eMod = m_fcolModZones[c4Index];

        MakeZoneText(c4Index + 1, eCfg, strZone);
        if (eCfg == eMod)
        {
            plwiCur->bmpImage(m_bmpCfg);
            plwiCur->bSuppressImage(eCfg == tHAIOmniSh::EZone_Unused);
        }
         else
        {
            if (eCfg == tHAIOmniSh::EZone_Unused)
                plwiCur->bmpImage(m_bmpAdded);
            else
                plwiCur->bmpImage(m_bmpRemoved);
            plwiCur->bSuppressImage(kCIDLib::False);
        }
        plwiCur->strText(strZone);
    }
    m_pwndZones->Refresh();
}

tCIDLib::TVoid
THAIOmniCWnd::UpdateFlagCfgList(        TListBox&               wndTar
                                , const tHAIOmniSh::TFlagArray& fcolCfg
                                , const tHAIOmniSh::TFlagArray& fcolMod)
{
    const tCIDLib::TCard4 c4Count = fcolCfg.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TListWndImageItem* plwiCur = wndTar.plwiAtAs<TListWndImageItem>(c4Index);

        const tCIDLib::TBoolean bCfg = fcolCfg[c4Index];
        const tCIDLib::TBoolean bMod = fcolMod[c4Index];

        if (bCfg == bMod)
        {
            // There are on local mods, so it's either configured or not
            plwiCur->bmpImage(m_bmpCfg);
            plwiCur->bSuppressImage(!bCfg);
        }
         else
        {
            // We have local mods, so show mod state
            if (bCfg)
                plwiCur->bmpImage(m_bmpRemoved);
            else
                plwiCur->bmpImage(m_bmpAdded);
            plwiCur->bSuppressImage(kCIDLib::False);
        }
    }
    wndTar.Refresh();
}

