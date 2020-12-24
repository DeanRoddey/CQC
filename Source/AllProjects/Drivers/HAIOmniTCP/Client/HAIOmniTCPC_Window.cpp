//
// FILE NAME: HAIOmniTCPC_Window.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 05/24/2008
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
//  This file implements the main window of the HAI TCP/IP Omni client driver
//
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "HAIOmniTCPC_.hpp"


// ---------------------------------------------------------------------------
//  Magic macro
// ---------------------------------------------------------------------------
RTTIDecls(THAIOmniTCPCWnd,TCQCDriverClient)



// ---------------------------------------------------------------------------
//   CLASS: THAIOmniTCPCWnd
//  PREFIX: wnd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  THAIOmniTCPCWnd: Constructors and Destructor
// ---------------------------------------------------------------------------
THAIOmniTCPCWnd::THAIOmniTCPCWnd(const  TCQCDriverObjCfg&   cqcdcThis
                                , const TCQCUserCtx&        cuctxToUse) :

    TCQCDriverClient
    (
        cqcdcThis, L"THAIOmniTCPCWnd", tCQCKit::EActLevels::Low, cuctxToUse
    )
    , m_pwndAllTrigs(nullptr)
    , m_pwndAreas(nullptr)
    , m_pwndClear(nullptr)
    , m_pwndSend(nullptr)
    , m_pwndThermos(nullptr)
    , m_pwndUnits(nullptr)
    , m_pwndZones(nullptr)
    , m_strCfg_Added(L"+")
    , m_strCfg_Cfg(L"*")
    , m_strCfg_Modified(L"!")
    , m_strCfg_Removed(L"-")
{
}

THAIOmniTCPCWnd::~THAIOmniTCPCWnd()
{
}


// ---------------------------------------------------------------------------
//  THAIOmniTCPCWnd: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean THAIOmniTCPCWnd::bChanges() const
{
    return (m_dcfgCurrent != m_dcfgOrg);
}


tCIDLib::TBoolean THAIOmniTCPCWnd::bSaveChanges(TString& strErrMsg)
{
    // If no changes, then do nothing
    if (m_dcfgCurrent == m_dcfgOrg)
        return kCIDLib::True;

    try
    {
        THeapBuf mbufCfg(1024 * 16);
        TBinMBufOutStream strmOut(&mbufCfg);

        strmOut << m_dcfgCurrent << kCIDLib::FlushIt;
        orbcServer()->SetConfig
        (
            strMoniker()
            , strmOut.c4CurSize()
            , mbufCfg, cuctxToUse().sectUser()
            , tCQCKit::EDrvCmdWaits::Wait
        );

        // Get ourself back into sync
        LoadConfig(orbcServer());
        Connected();
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
//  THAIOmniTCPCWnd: Protected, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean THAIOmniTCPCWnd::bCreated()
{
    TParent::bCreated();

    // Load the dialog resource that defines our content and create our content
    TDlgDesc dlgdChildren;
    facHAIOmniTCPC().bCreateDlgDesc(kHAIOmniTCPC::ridIntf_Main, dlgdChildren);

    tCIDCtrls::TWndId widInitFocus;
    CreateDlgItems(dlgdChildren, widInitFocus);

    // Do an initial auto-size to get them in sync with our initial size
    AutoAdjustChildren(dlgdChildren.areaPos(), areaClient());

    // Get typed pointers to the widgets we want to interact with a lot
    CastChildWnd(*this, kHAIOmniTCPC::ridIntf_AllTrigs, m_pwndAllTrigs);
    CastChildWnd(*this, kHAIOmniTCPC::ridIntf_AreaList, m_pwndAreas);
    CastChildWnd(*this, kHAIOmniTCPC::ridIntf_ClearChanges, m_pwndClear);
    CastChildWnd(*this, kHAIOmniTCPC::ridIntf_ThermoList, m_pwndThermos);
    CastChildWnd(*this, kHAIOmniTCPC::ridIntf_UnitList, m_pwndUnits);
    CastChildWnd(*this, kHAIOmniTCPC::ridIntf_ZoneList, m_pwndZones);

    // Register event handlers
    m_pwndAllTrigs->pnothRegisterHandler(this, &THAIOmniTCPCWnd::eClickHandler);
    m_pwndAreas->pnothRegisterHandler(this, &THAIOmniTCPCWnd::eLBHandler);
    m_pwndClear->pnothRegisterHandler(this, &THAIOmniTCPCWnd::eClickHandler);
    m_pwndThermos->pnothRegisterHandler(this, &THAIOmniTCPCWnd::eLBHandler);
    m_pwndUnits->pnothRegisterHandler(this, &THAIOmniTCPCWnd::eLBHandler);
    m_pwndZones->pnothRegisterHandler(this, &THAIOmniTCPCWnd::eLBHandler);

    // Set the columns
    tCIDLib::TStrList colCols(2);
    colCols.objAdd(L"   ");
    colCols.objAdd(L"Item Name");
    m_pwndAreas->SetColumns(colCols);
    m_pwndAreas->AutoSizeCol(0, kCIDLib::True);
    m_pwndThermos->SetColumns(colCols);
    m_pwndThermos->AutoSizeCol(0, kCIDLib::True);
    m_pwndUnits->SetColumns(colCols);
    m_pwndUnits->AutoSizeCol(0, kCIDLib::True);
    m_pwndZones->SetColumns(colCols);
    m_pwndZones->AutoSizeCol(0, kCIDLib::True);

    //
    //  Fill in the lists with the values that they will hold. The lists
    //  won't change, we'll just mark/unmark items according to config.
    //
    TString strName;
    m_pwndAreas->RemoveAll();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < kHAIOmniTCPSh::c4MaxAreas; c4Index++)
    {
        THAIOmniArea* pitemCur = m_dcfgCurrent.pitemAreaAt(c4Index);
        if (pitemCur)
        {
            colCols[0] = m_strCfg_Cfg;
            colCols[1] = pitemCur->strName();
            m_pwndAreas->c4AddItem(colCols, c4Index);
        }
         else
        {
            THAIOmniItem::QueryDefName
            (
                tHAIOmniTCPSh::EItem_Area, c4Index + 1, strName, kCIDLib::True
            );
            colCols[0].Clear();
            colCols[1] = pitemCur->strName();
            m_pwndAreas->c4AddItem(colCols, c4Index);
        }
    }
    m_pwndAreas->SelectByIndex(0);

    m_pwndThermos->RemoveAll();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < kHAIOmniTCPSh::c4MaxThermos; c4Index++)
    {
        THAIOmniThermo* pitemCur = m_dcfgCurrent.pitemThermoAt(c4Index);
        if (pitemCur)
        {
            colCols[0] = m_strCfg_Cfg;
            colCols[1] = pitemCur->strName();
            m_pwndThermos->c4AddItem(colCols, c4Index);
        }
         else
        {
            THAIOmniItem::QueryDefName
            (
                tHAIOmniTCPSh::EItem_Thermo, c4Index + 1, strName, kCIDLib::True
            );
            colCols[0].Clear();
            colCols[1] = pitemCur->strName();
            m_pwndThermos->c4AddItem(colCols, c4Index);
        }
    }
    m_pwndThermos->SelectByIndex(0);

    m_pwndUnits->RemoveAll();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < kHAIOmniTCPSh::c4MaxUnits; c4Index++)
    {
        THAIOmniUnit* pitemCur = m_dcfgCurrent.pitemUnitAt(c4Index);
        if (pitemCur)
        {
            colCols[0] = m_strCfg_Cfg;
            colCols[1] = pitemCur->strName();
            m_pwndUnits->c4AddItem(colCols, c4Index);
        }
         else
        {
            THAIOmniItem::QueryDefName
            (
                tHAIOmniTCPSh::EItem_Unit, c4Index + 1, strName, kCIDLib::True
            );
            colCols[0].Clear();
            colCols[1] = pitemCur->strName();
            m_pwndUnits->c4AddItem(colCols, c4Index);
        }
    }
    m_pwndUnits->SelectByIndex(0);

    m_pwndZones->RemoveAll();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < kHAIOmniTCPSh::c4MaxZones; c4Index++)
    {
        THAIOmniZone* pitemCur = m_dcfgCurrent.pitemZoneAt(c4Index);
        if (pitemCur)
        {
            colCols[0] = m_strCfg_Cfg;
            colCols[1] = pitemCur->strName();
            m_pwndZones->c4AddItem(colCols, c4Index);
        }
         else
        {
            THAIOmniItem::QueryDefName
            (
                tHAIOmniTCPSh::EItem_Zone, c4Index + 1, strName, kCIDLib::True
            );
            colCols[0].Clear();
            colCols[1] = pitemCur->strName();
            m_pwndZones->c4AddItem(colCols, c4Index);
        }
    }
    m_pwndZones->SelectByIndex(0);

    return kCIDLib::True;
}



tCIDLib::TBoolean
THAIOmniTCPCWnd::bGetConnectData(tCQCKit::TCQCSrvProxy& orbcTarget)
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


tCIDLib::TBoolean THAIOmniTCPCWnd::bDoPoll(tCQCKit::TCQCSrvProxy& orbcTarget)
{
    //
    //  We don't really poll, but we do want to watch for it going off
    //  line, so we do a simple 'by name' field read periodically.
    //
    return kCIDLib::True;
}


tCIDLib::TVoid THAIOmniTCPCWnd::Connected()
{
    //Update all the lists
    UpdateCfgLists();
}


tCIDLib::TVoid THAIOmniTCPCWnd::DoCleanup()
{
}


tCIDLib::TVoid THAIOmniTCPCWnd::LostConnection()
{
}


tCIDLib::TVoid THAIOmniTCPCWnd::UpdateDisplayData()
{
}


// ---------------------------------------------------------------------------
//  THAIOmniTCPCWnd: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  We have to display which items are changed, i.e. that need to be uploaded.
//  There may be no item at a given slot in the new or old config, and if they
//  are both there they may or may not be equal. So this helper handles making
//  that decision. The item classes provide a polymorphic comparator so that
//  we can do an easy comparison here without regard to type.
//
//  We also need to figure out whta text to display for the slot. If we have
//  a new one, we take it, else, if we have an old one, we take that. Else,
//  we display nothing.
//
//  We return true if equal else false.
//
tCIDLib::TBoolean
THAIOmniTCPCWnd::bCompareItems( const   THAIOmniItem* const         pitemOrg
                                , const THAIOmniItem* const         pitemNew
                                ,       TString&                    strText
                                ,       tCIDLib::TBoolean&          bUsed
                                ,       EChangeTypes&               eChType
                                , const tHAIOmniTCPSh::EItemTypes   eType
                                , const tCIDLib::TCard4             c4ItemNum) const
{
    // Figure out the text to display.
    if (pitemNew)
        pitemNew->QueryDisplayText(strText);
    else if (pitemOrg)
        pitemOrg->QueryDisplayText(strText);
    else
        THAIOmniItem::QueryDefName(eType, c4ItemNum, strText, kCIDLib::True);

    //
    //  If we have one or the other, then at least for now this slot is in use.
    //  If the changes are committed, then this might change.
    //
    bUsed = (pitemOrg != 0) || (pitemNew != 0);

    // If not used, then obviously they are 'equal'
    if (!bUsed)
    {
        eChType = EChType_None;
        return kCIDLib::True;
    }

    // If one is set and the other isn't, obviously not the same
    if (!pitemOrg && pitemNew)
    {
        eChType = EChType_Added;
        return kCIDLib::False;
    }

    if (pitemOrg && !pitemNew)
    {
        eChType = EChType_Removed;
        return kCIDLib::False;
    }

    // Both are there, so see if they are equal
    const tCIDLib::TBoolean bRes = pitemOrg->bCompare(*pitemNew);

    if (bRes)
        eChType = EChType_None;
    else
        eChType = EChType_Modified;

    return bRes;
}



//
//  This is called on a button click. Mostly what we do is set up a string
//  to pass to the driver's Command field, but some will us the backdoor
//  c4SendCmd() method for things that no fields exist.
//
//  NOTE:   Groups and units can have spaces in their names so be sure to
//          quote them in the send strings.
//
tCIDCtrls::EEvResponses THAIOmniTCPCWnd::eClickHandler(TButtClickInfo& wnotEvent)
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
            case kHAIOmniTCPC::ridIntf_AllTrigs :
            {
                TAllTrigsDlg dlgAllTrigs;
                dlgAllTrigs.Run(*this, m_dcfgCurrent);
                UpdateCfgLists();
                break;
            }

            case kHAIOmniTCPC::ridIntf_ClearChanges :
            {
                // Go back to the original config
                m_dcfgCurrent = m_dcfgOrg;
                UpdateCfgLists();
                break;
            }

            default :
                break;
        };
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        TExceptDlg dlgbFail
        (
            *this
            , strWndText()
            , facHAIOmniTCPC().strMsg(kOmniTCPCMsgs::midStatus_RemOpFailed)
            , errToCatch
        );
    }
    return tCIDCtrls::EEvResponses::Handled;
}


tCIDCtrls::EEvResponses
THAIOmniTCPCWnd::eLBHandler(TListChangeInfo& wnotEvent)
{
    // On double clicks, we add/remove/change the item
    if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::Invoked)
    {
        const tCIDLib::TCard4       c4ListInd = wnotEvent.c4Index();
        tHAIOmniTCPSh::EItemTypes   eType;
        const TString&              strDef = TString::strEmpty();
        const tCIDCtrls::TWndId     widSrc = wnotEvent.widSource();

        TMultiColListBox* pwndList = pwndChildAs<TMultiColListBox>(widSrc);
        const tCIDLib::TCard4       c4ItemInd = pwndList->c4IndexToId(c4ListInd);

        //
        //  We need to edit the selected one. Since we don't want to
        //  change the config unless the user commits, we either want
        //  to dup the existing one, or create a new one.
        //
        THAIOmniItem* pitemCur = 0;
        THAIOmniItem* pitemNew = 0;
        if (widSrc == kHAIOmniTCPC::ridIntf_AreaList)
        {
            pitemCur = m_dcfgCurrent.pitemAreaAt(c4ItemInd);
            if (pitemCur)
                pitemNew = pitemCur->pitemDup();
            else
                pitemNew = new THAIOmniArea(strDef, c4ItemInd + 1);
            eType = tHAIOmniTCPSh::EItem_Area;
        }
         else if (widSrc == kHAIOmniTCPC::ridIntf_ThermoList)
        {
            pitemCur = m_dcfgCurrent.pitemThermoAt(c4ItemInd);
            if (pitemCur)
                pitemNew = pitemCur->pitemDup();
            else
                pitemNew = new THAIOmniThermo(strDef, c4ItemInd + 1);
            eType = tHAIOmniTCPSh::EItem_Thermo;
        }
         else if (widSrc == kHAIOmniTCPC::ridIntf_UnitList)
        {
            pitemCur = m_dcfgCurrent.pitemUnitAt(c4ItemInd);
            if (pitemCur)
                pitemNew = pitemCur->pitemDup();
            else
                pitemNew = new THAIOmniUnit(strDef, c4ItemInd + 1, tHAIOmniTCPSh::EUnit_Binary);
            eType = tHAIOmniTCPSh::EItem_Unit;
        }
         else if (widSrc == kHAIOmniTCPC::ridIntf_ZoneList)
        {
            pitemCur = m_dcfgCurrent.pitemZoneAt(c4ItemInd);
            if (pitemCur)
                pitemNew = pitemCur->pitemDup();
            else
                pitemNew = new THAIOmniZone(strDef, c4ItemInd + 1, tHAIOmniTCPSh::EZone_Alarm);
            eType = tHAIOmniTCPSh::EItem_Zone;
        }
         else
        {
            // Shouldn't happen, but just in case
            return tCIDCtrls::EEvResponses::Handled;
        }

        //
        //  Make sure the new one, which is really a temp that we won't ever
        //  actually store away itself, gets cleaned up.
        //
        TJanitor<THAIOmniItem> janNew(pitemNew);

        //
        //  Make a copy of the original so that we can check for changes and
        //  update the list slot if needed.
        //
        THAIOmniItem* pitemOrg = pitemNew->pitemDup();
        TJanitor<THAIOmniItem> janOrg(pitemOrg);

        //
        //  Invoke the edit dialog. If the current pointer isn't zero, then
        //  there was an original one, and so the delete button can be
        //  enabled.
        //
        TEdItemDlg dlgEdit;
        const tHAIOmniTCPC::EEditRes eRes = dlgEdit.eRun
        (
            *this, *pitemNew, m_dcfgCurrent, pitemCur != nullptr
        );

        if (eRes == tHAIOmniTCPC::EEditRes_Delete)
        {
            // Delete the selected item
            m_dcfgCurrent.RemoveAt(eType, c4ItemInd);

            //
            //  Zero out the new item pointer, so that we now see it as
            //  being removed. The delete button would only have been enabled
            //  if we had a previous one.
            //
            pitemNew = 0;
        }
         else if (eRes == tHAIOmniTCPC::EEditRes_Save)
        {
            // Store the info
            if (widSrc == kHAIOmniTCPC::ridIntf_AreaList)
                m_dcfgCurrent.SetAreaAt(static_cast<THAIOmniArea&>(*pitemNew), c4ItemInd);
            else if (widSrc == kHAIOmniTCPC::ridIntf_ThermoList)
                m_dcfgCurrent.SetThermoAt(static_cast<THAIOmniThermo&>(*pitemNew), c4ItemInd);
            else if (widSrc == kHAIOmniTCPC::ridIntf_UnitList)
                m_dcfgCurrent.SetUnitAt(static_cast<THAIOmniUnit&>(*pitemNew), c4ItemInd);
            else if (widSrc == kHAIOmniTCPC::ridIntf_ZoneList)
                m_dcfgCurrent.SetZoneAt(static_cast<THAIOmniZone&>(*pitemNew), c4ItemInd);
        }

        // And update the list slot for this guy if anything could have changed
        if (eRes != tHAIOmniTCPC::EEditRes_Cancel)
            UpdateCfgItem(*pwndList, c4ListInd, c4ItemInd, pitemOrg, pitemNew, eType);
    }
    return tCIDCtrls::EEvResponses::Handled;
}


//
//  Asks the server side driver for it's current configuration. Our server
//  side driver is a C++ driver, so we can get a binary blob back. We use
//  one of the backdoor driver calls to get it.
//
tCIDLib::TVoid THAIOmniTCPCWnd::LoadConfig(tCQCKit::TCQCSrvProxy& orbcSrv)
{
    try
    {
        THeapBuf mbufCfg(8192UL);
        tCIDLib::TCard4 c4Sz;
        orbcSrv->QueryConfig(strMoniker(), c4Sz, mbufCfg, sectUser());

        // Make an input stream over the data and stream in the config
        TBinMBufInStream strmIn(&mbufCfg, c4Sz);
        strmIn >> m_dcfgCurrent;

        // Remember this as the original config
        m_dcfgOrg = m_dcfgCurrent;
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        TExceptDlg dlgbFail
        (
            *this
            , strWndText()
            , facHAIOmniTCPC().strMsg(kOmniTCPCErrs::errcCfg_CantLoadConfig)
            , errToCatch
        );
    }
}


//
//  Given the original and new config for an item, updates the list window
//  slot that is displaying the info for that item.
//
tCIDLib::TVoid
THAIOmniTCPCWnd::UpdateCfgItem(         TMultiColListBox&           wndList
                                , const tCIDLib::TCard4             c4ListIndex
                                , const tCIDLib::TCard4             c4ItemIndex
                                ,       THAIOmniItem*               pitemOrg
                                ,       THAIOmniItem*               pitemNew
                                , const tHAIOmniTCPSh::EItemTypes   eType)
{
    tCIDLib::TBoolean   bDiff;
    tCIDLib::TBoolean   bUsed;
    EChangeTypes        eChType;
    TString             strCfgText;
    TString             strName;

    bDiff = !bCompareItems
    (
        pitemOrg, pitemNew, strName, bUsed, eChType, eType, c4ItemIndex + 1
    );

    // Set the appropriate image
    if (bUsed)
    {
        if ((eChType == EChType_Added) || (eChType == EChType_Modified))
            strCfgText = m_strCfg_Added;
        else if (eChType == EChType_Removed)
            strCfgText = m_strCfg_Removed;
        else
            strCfgText = m_strCfg_Cfg;
    }

    wndList.SetColText(c4ListIndex, 0, strCfgText);
    wndList.SetColText(c4ListIndex, 1, strName);
}


//
//  These methods handle updating the config lists to reflect the current
//  state of the state. The grunt work is done by a helper that handles
//  individual items.
//
tCIDLib::TVoid THAIOmniTCPCWnd::UpdateCfgLists()
{
    // Do the areas
    for (tCIDLib::TCard4 c4Index = 0; c4Index < kHAIOmniTCPSh::c4MaxAreas; c4Index++)
    {
        THAIOmniArea* pitemOrg = m_dcfgOrg.pitemAreaAt(c4Index);
        THAIOmniArea* pitemCur = m_dcfgCurrent.pitemAreaAt(c4Index);
        UpdateCfgItem
        (
            *m_pwndAreas
            , m_pwndAreas->c4IdToIndex(c4Index)
            , c4Index
            , pitemOrg
            , pitemCur
            , tHAIOmniTCPSh::EItem_Area
        );
    }
    m_pwndAreas->RedrawVisible();

    // Do the Thermos
    for (tCIDLib::TCard4 c4Index = 0; c4Index < kHAIOmniTCPSh::c4MaxThermos; c4Index++)
    {
        THAIOmniThermo* pitemOrg = m_dcfgOrg.pitemThermoAt(c4Index);
        THAIOmniThermo* pitemCur = m_dcfgCurrent.pitemThermoAt(c4Index);
        UpdateCfgItem
        (
            *m_pwndThermos
            , c4Index
            , m_pwndThermos->c4IdToIndex(c4Index)
            , pitemOrg
            , pitemCur
            , tHAIOmniTCPSh::EItem_Thermo
        );
    }
    m_pwndThermos->RedrawVisible();

    // Do the units
    for (tCIDLib::TCard4 c4Index = 0; c4Index < kHAIOmniTCPSh::c4MaxUnits; c4Index++)
    {
        THAIOmniUnit* pitemOrg = m_dcfgOrg.pitemUnitAt(c4Index);
        THAIOmniUnit* pitemCur = m_dcfgCurrent.pitemUnitAt(c4Index);
        UpdateCfgItem
        (
            *m_pwndUnits
            , c4Index
            , m_pwndUnits->c4IdToIndex(c4Index)
            , pitemOrg
            , pitemCur
            , tHAIOmniTCPSh::EItem_Unit
        );
    }
    m_pwndUnits->RedrawVisible();

    // Do the zones
    for (tCIDLib::TCard4 c4Index = 0; c4Index < kHAIOmniTCPSh::c4MaxZones; c4Index++)
    {
        THAIOmniZone* pitemOrg = m_dcfgOrg.pitemZoneAt(c4Index);
        THAIOmniZone* pitemCur = m_dcfgCurrent.pitemZoneAt(c4Index);
        UpdateCfgItem
        (
            *m_pwndZones
            , c4Index
            , m_pwndZones->c4IdToIndex(c4Index)
            , pitemOrg
            , pitemCur
            , tHAIOmniTCPSh::EItem_Zone
        );
    }
    m_pwndZones->RedrawVisible();
}

