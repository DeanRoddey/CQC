//
// FILE NAME: HAIOmniTCP2C_Window.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/09/2014
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
#include    "HAIOmniTCP2C_.hpp"


// ---------------------------------------------------------------------------
//  Magic macro
// ---------------------------------------------------------------------------
RTTIDecls(THAIOmniTCP2CWnd,TCQCDriverClient)



// ---------------------------------------------------------------------------
//   CLASS: THAIOmniTCP2CWnd
//  PREFIX: wnd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  THAIOmniTCP2CWnd: Constructors and Destructor
// ---------------------------------------------------------------------------
THAIOmniTCP2CWnd::THAIOmniTCP2CWnd( const   TCQCDriverObjCfg&   cqcdcThis
                                    , const TCQCUserCtx&        cuctxToUse) :

    TCQCDriverClient(cqcdcThis, L"THAIOmniTCP2CWnd", tCQCKit::EActLevels::Low, cuctxToUse)
    , m_pwndAllTrigs(nullptr)
    , m_pwndAreas(nullptr)
    , m_pwndClear(nullptr)
    , m_pwndThermos(nullptr)
    , m_pwndUnits(nullptr)
    , m_pwndZones(nullptr)
    , m_strCfg_Added(L">")
    , m_strCfg_Modified(L"o")
    , m_strCfg_Removed(L"x")
{
}

THAIOmniTCP2CWnd::~THAIOmniTCP2CWnd()
{
}


// ---------------------------------------------------------------------------
//  THAIOmniTCP2CWnd: Public, non-virtual methods
// ---------------------------------------------------------------------------

// See if we have any changes
tCIDLib::TBoolean THAIOmniTCP2CWnd::bChanges() const
{
    return (m_dcfgOrg != m_dcfgCurrent);
}


tCIDLib::TBoolean THAIOmniTCP2CWnd::bSaveChanges(TString& strErrMsg)
{
    // See if there not any changes, then we are done
    if (m_dcfgOrg == m_dcfgCurrent)
        return kCIDLib::False;

    // Format out our current config and send it
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

        //
        //  It worked so save the current stuff as the original since it's the new baseline
        //  for changes.
        //
        m_dcfgOrg = m_dcfgCurrent;

        //
        //  And update to get rid of any change markers. We have to force it to redraw
        //  changed lists since we just got rid of the changes above. If we redraw before
        //  doing that, the changes would still be there and we'd still be screwed.
        //
        bUpdateCfgLists(kCIDLib::True);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TExceptDlg dlgErr
        (
            *this
            , strWndText()
            , L"The changes could not be saved back to the driver"
            , errToCatch
        );
        return kCIDLib::False;
    }
    return kCIDLib::True;
}



// ---------------------------------------------------------------------------
//  THAIOmniTCP2CWnd: Protected, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean THAIOmniTCP2CWnd::bCreated()
{
    TParent::bCreated();

    // Load the dialog resource that defines our content and create our content
    TDlgDesc dlgdChildren;
    facHAIOmniTCP2C().bCreateDlgDesc(kHAIOmniTCP2C::ridIntf_Main, dlgdChildren);

    tCIDCtrls::TWndId widInitFocus;
    CreateDlgItems(dlgdChildren, widInitFocus);

    // Do an initial auto-size to get them in sync with our initial size
    AutoAdjustChildren(dlgdChildren.areaPos(), areaClient());

    // Get typed pointers to the widgets we want to interact with a lot
    CastChildWnd(*this, kHAIOmniTCP2C::ridIntf_AllTrigs, m_pwndAllTrigs);
    CastChildWnd(*this, kHAIOmniTCP2C::ridIntf_AreaList, m_pwndAreas);
    CastChildWnd(*this, kHAIOmniTCP2C::ridIntf_ClearChanges, m_pwndClear);
    CastChildWnd(*this, kHAIOmniTCP2C::ridIntf_ThermoList, m_pwndThermos);
    CastChildWnd(*this, kHAIOmniTCP2C::ridIntf_UnitList, m_pwndUnits);
    CastChildWnd(*this, kHAIOmniTCP2C::ridIntf_ZoneList, m_pwndZones);

    // Register event handlers
    m_pwndAllTrigs->pnothRegisterHandler(this, &THAIOmniTCP2CWnd::eClickHandler);
    m_pwndAreas->pnothRegisterHandler(this, &THAIOmniTCP2CWnd::eLBHandler);
    m_pwndClear->pnothRegisterHandler(this, &THAIOmniTCP2CWnd::eClickHandler);
    m_pwndThermos->pnothRegisterHandler(this, &THAIOmniTCP2CWnd::eLBHandler);
    m_pwndUnits->pnothRegisterHandler(this, &THAIOmniTCP2CWnd::eLBHandler);
    m_pwndZones->pnothRegisterHandler(this, &THAIOmniTCP2CWnd::eLBHandler);

    //
    //  Set the column titles for the lists.
    //
    //  For the area and thermos we have only a configuration change indicator and a
    //  name, and the last column is auto-sized. For the others we also have a sub-type
    //  column and a 'send triggers' column.
    //
    tCIDLib::TStrList colCols(4);
    colCols.objAdd(L"   ");
    colCols.objAdd(TString::strEmpty());

    colCols[1] = facHAIOmniTCP2C().strMsg(kOmniTCP2CMsgs::midIntf_AreaCol);
    m_pwndAreas->SetColumns(colCols);
    m_pwndAreas->AutoSizeCol(0, kCIDLib::True);

    colCols[1] = facHAIOmniTCP2C().strMsg(kOmniTCP2CMsgs::midIntf_ThermoCol);
    m_pwndThermos->SetColumns(colCols);
    m_pwndThermos->AutoSizeCol(0, kCIDLib::True);

    //
    //  Do the lists that don't have the sub-type column
    //
    tCIDLib::TCard2 c2Id;
    tCIDLib::TCard4 c4At;

    // Load the areas
    m_pwndAreas->RemoveAll();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < kHAIOmniTCP2Sh::c4MaxAreas; c4Index++)
    {
        c2Id = tCIDLib::TCard2(c4Index + 1);
        THAIOmniArea* pitemCur = m_dcfgCurrent.pitemAreaById(c2Id, c4At);

        if (pitemCur)
        {
            colCols[1] = pitemCur->strName();
        }
         else
        {
            THAIOmniItem::QueryDefName
            (
                tHAIOmniTCP2Sh::EItemTypes::Area, c2Id, colCols[1], kCIDLib::True
            );
        }
        m_pwndAreas->c4AddItem(colCols, c2Id);
    }
    m_pwndAreas->SelectByIndex(0);

    // Load the thermos
    m_pwndThermos->RemoveAll();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < kHAIOmniTCP2Sh::c4MaxThermos; c4Index++)
    {
        c2Id = tCIDLib::TCard2(c4Index + 1);

        THAIOmniThermo* pitemCur = m_dcfgCurrent.pitemThermoById(c2Id, c4At);
        if (pitemCur)
        {
            colCols[1] = pitemCur->strName();
        }
         else
        {
            THAIOmniItem::QueryDefName
            (
                tHAIOmniTCP2Sh::EItemTypes::Thermo, c2Id, colCols[1], kCIDLib::True
            );
        }
        m_pwndThermos->c4AddItem(colCols, c2Id);
    }
    m_pwndThermos->SelectByIndex(0);

    // Now add the extra columns and do the ones that support that
    colCols.objAdd(facHAIOmniTCP2C().strMsg(kOmniTCP2CMsgs::midIntf_SubTypeCol));
    colCols.objAdd(facHAIOmniTCP2C().strMsg(kOmniTCP2CMsgs::midIntf_TriggersCol));

    colCols[1] = facHAIOmniTCP2C().strMsg(kOmniTCP2CMsgs::midIntf_UnitCol);
    m_pwndUnits->SetColumns(colCols);
    m_pwndUnits->AutoSizeCol(0, kCIDLib::True);
    m_pwndUnits->AutoSizeCol(1, kCIDLib::True);
    m_pwndUnits->AutoSizeCol(2, kCIDLib::True);
    m_pwndUnits->AutoSizeCol(3, kCIDLib::True);

    colCols[1] = facHAIOmniTCP2C().strMsg(kOmniTCP2CMsgs::midIntf_ZoneCol);
    m_pwndZones->SetColumns(colCols);
    m_pwndZones->AutoSizeCol(0, kCIDLib::True);
    m_pwndZones->AutoSizeCol(1, kCIDLib::True);
    m_pwndZones->AutoSizeCol(2, kCIDLib::True);
    m_pwndZones->AutoSizeCol(3, kCIDLib::True);

    m_pwndUnits->RemoveAll();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < kHAIOmniTCP2Sh::c4MaxUnits; c4Index++)
    {
        c2Id = tCIDLib::TCard2(c4Index + 1);

        THAIOmniUnit* pitemCur = m_dcfgCurrent.pitemUnitById(c2Id, c4At);
        if (pitemCur)
        {
            colCols[1] = pitemCur->strName();
            pitemCur->bHasSubType(colCols[2]);
            if (pitemCur->bSendTrig())
                colCols[3] = L"Yes";
            else
                colCols[3] = L"No";
        }
         else
        {
            THAIOmniItem::QueryDefName
            (
                tHAIOmniTCP2Sh::EItemTypes::Unit, c2Id, colCols[1], kCIDLib::True
            );
            colCols[2].Clear();
            colCols[3].Clear();
        }

        m_pwndUnits->c4AddItem(colCols, c2Id);
    }
    m_pwndUnits->SelectByIndex(0);

    // Load the zones
    m_pwndZones->RemoveAll();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < kHAIOmniTCP2Sh::c4MaxZones; c4Index++)
    {
        c2Id = tCIDLib::TCard2(c4Index + 1);

        THAIOmniZone* pitemCur = m_dcfgCurrent.pitemZoneById(c2Id, c4At);
        if (pitemCur)
        {
            colCols[1] = pitemCur->strName();
            pitemCur->bHasSubType(colCols[2]);
            if (pitemCur->bSendTrig())
                colCols[3] = L"Yes";
            else
                colCols[3] = L"No";
        }
         else
        {
            THAIOmniItem::QueryDefName
            (
                tHAIOmniTCP2Sh::EItemTypes::Zone, c2Id, colCols[1], kCIDLib::True
            );
            colCols[2].Clear();
            colCols[3].Clear();
        }

        m_pwndZones->c4AddItem(colCols, c2Id);
    }
    m_pwndZones->SelectByIndex(0);

    m_pwndAreas->Redraw(tCIDCtrls::ERedrawFlags::Full);
    m_pwndThermos->Redraw(tCIDCtrls::ERedrawFlags::Full);
    m_pwndUnits->Redraw(tCIDCtrls::ERedrawFlags::Full);
    m_pwndZones->Redraw(tCIDCtrls::ERedrawFlags::Full);

    return kCIDLib::True;
}


tCIDLib::TBoolean THAIOmniTCP2CWnd::bGetConnectData(tCQCKit::TCQCSrvProxy& orbcTarget)
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


tCIDLib::TBoolean THAIOmniTCP2CWnd::bDoPoll(tCQCKit::TCQCSrvProxy& orbcTarget)
{
    // We don't display any live data
    return kCIDLib::True;
}


tCIDLib::TVoid THAIOmniTCP2CWnd::Connected()
{
    // Update all the lists
    bUpdateCfgLists(kCIDLib::True);
}


tCIDLib::TVoid THAIOmniTCP2CWnd::DoCleanup()
{
}


tCIDLib::TVoid THAIOmniTCP2CWnd::LostConnection()
{
    // Nothing for us to do currently
}


tCIDLib::TVoid THAIOmniTCP2CWnd::UpdateDisplayData()
{
    // We don't display any live data, just config
}


// ---------------------------------------------------------------------------
//  THAIOmniTCP2CWnd: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  We need to run through all of the lists and see if we have any changes
//  We return true if equal else false.
//
tCIDLib::TBoolean
THAIOmniTCP2CWnd::bCompareItems(const   THAIOmniItem* const         pitemOrg
                                , const THAIOmniItem* const         pitemNew
                                ,       tCIDLib::TStrList&          colVals
                                ,       tCIDLib::TBoolean&          bUsed
                                ,       EChangeTypes&               eChType
                                , const tHAIOmniTCP2Sh::EItemTypes  eType
                                , const tCIDLib::TCard2             c2ItemNum) const
{
    // Clear the values that don't always get set
    colVals[0].Clear();
    colVals[2].Clear();
    colVals[3].Clear();

    //
    //  If we have one or the other, then at least for now this slot is in use.
    //  If the changes are committed, then this might change.
    //
    bUsed = (pitemOrg != 0) || (pitemNew != 0);

    // If not used, then obviously they are 'equal'
    if (!bUsed)
    {
        // Show the default name
        THAIOmniItem::QueryDefName(eType, c2ItemNum, colVals[1], kCIDLib::True);
        eChType = EChangeTypes::None;
        return kCIDLib::True;
    }

    // We have at least one, then let's load the data
    const THAIOmniItem* pitemTar = pitemNew;
    if (!pitemTar)
        pitemTar = pitemOrg;

    // Get the info out
    colVals[1] = pitemTar->strName();
    pitemTar->bHasSubType(colVals[2]);
    if ((eType == tHAIOmniTCP2Sh::EItemTypes::Unit)
    ||  (eType == tHAIOmniTCP2Sh::EItemTypes::Zone))
    {
        if (pitemTar->bSendTrig())
            colVals[3] = L"Yes";
        else
            colVals[3] = L"No";
    }

    // If one is set and the other isn't, obviously not the same
    if (!pitemOrg && pitemNew)
    {
        colVals[0] = m_strCfg_Added;
        eChType = EChangeTypes::Added;
        return kCIDLib::False;
    }

    if (pitemOrg && !pitemNew)
    {
        colVals[0] = m_strCfg_Removed;
        eChType = EChangeTypes::Removed;
        return kCIDLib::False;
    }

    // Both are there, so see if they are equal
    const tCIDLib::TBoolean bRes = pitemOrg->bCompare(*pitemNew);
    if (bRes)
    {
        eChType = EChangeTypes::None;
    }
     else
    {
        colVals[0] = m_strCfg_Modified;
        eChType = EChangeTypes::Modified;
    }

    return bRes;
}



//
//  Given the original and new config for an item, updates the list window slot that is
//  displaying the info for that item. We also return whether there are any changes or
//  not.
//
tCIDLib::TBoolean
THAIOmniTCP2CWnd::bUpdateCfgItem(       TMultiColListBox&           wndList
                                ,       tCIDLib::TStrList&          colVals
                                , const THAIOmniItem*               pitemOrg
                                , const THAIOmniItem*               pitemNew
                                , const tCIDLib::TCard4             c4Index
                                , const tCIDLib::TCard2             c2Id
                                , const tHAIOmniTCP2Sh::EItemTypes  eType) const
{
    tCIDLib::TBoolean   bDiff;
    tCIDLib::TBoolean   bUsed;
    EChangeTypes        eChType;

    // Get comparison info and the name info
    bDiff = !bCompareItems(pitemOrg, pitemNew, colVals, bUsed, eChType, eType, c2Id);

    // Set the values it gaves us back, tell it not to redraw
    wndList.UpdateRowAt(colVals, c4Index, c2Id, kCIDLib::False);
    return bDiff;
}


//
//  These methods handle updating the config lists to reflect the current
//  state of the state. The grunt work is done by a helper that handles
//  individual items.
//
tCIDLib::TBoolean
THAIOmniTCP2CWnd::bUpdateACfgList(          TMultiColListBox&           wndList
                                    ,       tCIDLib::TStrList&          colVals
                                    , const tHAIOmniTCP2Sh::EItemTypes  eType
                                    , const tCIDLib::TCard4             c4Max) const
{
    tCIDLib::TBoolean   bDiffs = kCIDLib::False;
    tCIDLib::TCard2     c2Id;
    tCIDLib::TCard4     c4OrgAt;
    tCIDLib::TCard4     c4CurAt;

    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Max; c4Index++)
    {
        c2Id = tCIDLib::TCard2(c4Index + 1);

        const THAIOmniItem* pitemOrg = m_dcfgOrg.pitemById(eType, c2Id, c4OrgAt);
        const THAIOmniItem* pitemCur = m_dcfgCurrent.pitemById(eType, c2Id, c4CurAt);

        bDiffs |= bUpdateCfgItem
        (
            wndList, colVals, pitemOrg, pitemCur, c4Index, c2Id, eType
        );
    }
    return bDiffs;
}


tCIDLib::TBoolean
THAIOmniTCP2CWnd::bUpdateCfgLists(const tCIDLib::TBoolean bForceRedraw) const
{
    tCIDLib::TBoolean bRet = kCIDLib::False;

    //
    //  Do each of the lists by calling the helper above. We provide them with a column
    //  values list that has the max possible number of strings in it.
    //
    tCIDLib::TStrList colVals(4);
    colVals.AddXCopies(TString::strEmpty(), 4);

    const tCIDLib::TBoolean bAreaChanges = bUpdateACfgList
    (
        *m_pwndAreas, colVals, tHAIOmniTCP2Sh::EItemTypes::Area, kHAIOmniTCP2Sh::c4MaxAreas
    );

    const tCIDLib::TBoolean bThermoChanges = bUpdateACfgList
    (
        *m_pwndThermos, colVals, tHAIOmniTCP2Sh::EItemTypes::Thermo, kHAIOmniTCP2Sh::c4MaxThermos
    );

    const tCIDLib::TBoolean bUnitChanges = bUpdateACfgList
    (
        *m_pwndUnits, colVals, tHAIOmniTCP2Sh::EItemTypes::Unit, kHAIOmniTCP2Sh::c4MaxUnits
    );

    const tCIDLib::TBoolean bZoneChanges = bUpdateACfgList
    (
        *m_pwndZones, colVals, tHAIOmniTCP2Sh::EItemTypes::Zone, kHAIOmniTCP2Sh::c4MaxZones
    );

    if (bAreaChanges || bForceRedraw)
        m_pwndAreas->Redraw(tCIDCtrls::ERedrawFlags::Full);
    if (bThermoChanges || bForceRedraw)
        m_pwndThermos->Redraw(tCIDCtrls::ERedrawFlags::Full);
    if (bUnitChanges || bForceRedraw)
        m_pwndUnits->Redraw(tCIDCtrls::ERedrawFlags::Full);
    if (bZoneChanges || bForceRedraw)
        m_pwndZones->Redraw(tCIDCtrls::ERedrawFlags::Full);

    return (bAreaChanges | bThermoChanges | bUnitChanges | bZoneChanges);
}


//
//  This is called on a button click. Mostly what we do is set up a string
//  to pass to the driver's Command field, but some will us the backdoor
//  c4SendCmd() method for things that no fields exist.
//
//  NOTE:   Groups and units can have spaces in their names so be sure to
//          quote them in the send strings.
//
tCIDCtrls::EEvResponses THAIOmniTCP2CWnd::eClickHandler(TButtClickInfo& wnotEvent)
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
            case kHAIOmniTCP2C::ridIntf_AllTrigs :
            {
                TAllTrigsDlg dlgAllTrigs;
                dlgAllTrigs.RunDlg(*this, m_dcfgCurrent);
                bUpdateCfgLists(kCIDLib::True);
                break;
            }

            case kHAIOmniTCP2C::ridIntf_ClearChanges :
            {
                // Go back to the original config
                m_dcfgCurrent = m_dcfgOrg;
                bUpdateCfgLists(kCIDLib::True);
                break;
            }

            default :
                break;
        };
    }

    catch(TError& errToCatch)
    {
        if (facHAIOmniTCP2C().bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        TExceptDlg dlgbFail
        (
            *this
            , strWndText()
            , facHAIOmniTCP2C().strMsg(kOmniTCP2CMsgs::midStatus_RemOpFailed)
            , errToCatch
        );
    }
    return tCIDCtrls::EEvResponses::Handled;
}


tCIDCtrls::EEvResponses THAIOmniTCP2CWnd::eLBHandler(TListChangeInfo& wnotEvent)
{
    // On double clicks, we add/remove/change the item
    if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::Invoked)
    {
        const tCIDCtrls::TWndId widSrc = wnotEvent.widSource();
        tHAIOmniTCP2Sh::EItemTypes eType;
        const TString& strDef = TString::strEmpty();
        const tCIDLib::TCard2 c2SelId = tCIDLib::TCard2(wnotEvent.c4Index() + 1);
        TMultiColListBox* pwndList = nullptr;

        //
        //  We need to edit the selected one. Since we don't want to
        //  change the config unless the user commits, we either want
        //  to dup the existing one, or create a new one.
        //
        THAIOmniItem* pitemCur = 0;
        THAIOmniItem* pitemNew = 0;
        THAIOmniItem* pitemOrg = 0;
        tCIDLib::TCard4 c4At;
        if (widSrc == kHAIOmniTCP2C::ridIntf_AreaList)
        {
            pitemCur = m_dcfgCurrent.pitemAreaById(c2SelId, c4At);
            if (pitemCur)
                pitemNew = pitemCur->pitemDup();
            else
                pitemNew = new THAIOmniArea(strDef, c2SelId);

            eType = tHAIOmniTCP2Sh::EItemTypes::Area;
            pwndList = m_pwndAreas;
            pitemOrg = m_dcfgOrg.pitemAreaById(c2SelId, c4At);
        }
         else if (widSrc == kHAIOmniTCP2C::ridIntf_ThermoList)
        {
            pitemCur = m_dcfgCurrent.pitemThermoById(c2SelId, c4At);
            if (pitemCur)
                pitemNew = pitemCur->pitemDup();
            else
                pitemNew = new THAIOmniThermo(strDef, c2SelId, tHAIOmniTCP2Sh::EThermoTypes::AutoHC);

            eType = tHAIOmniTCP2Sh::EItemTypes::Thermo;
            pwndList = m_pwndThermos;
            pitemOrg = m_dcfgOrg.pitemThermoById(c2SelId, c4At);
        }
         else if (widSrc == kHAIOmniTCP2C::ridIntf_UnitList)
        {
            pitemCur = m_dcfgCurrent.pitemUnitById(c2SelId, c4At);
            if (pitemCur)
                pitemNew = pitemCur->pitemDup();
            else
                pitemNew = new THAIOmniUnit(strDef, c2SelId, tHAIOmniTCP2Sh::EUnitTypes::Switch);

            eType = tHAIOmniTCP2Sh::EItemTypes::Unit;
            pwndList = m_pwndUnits;
            pitemOrg = m_dcfgOrg.pitemUnitById(c2SelId, c4At);

        }
         else if (widSrc == kHAIOmniTCP2C::ridIntf_ZoneList)
        {
            pitemCur = m_dcfgCurrent.pitemZoneById(c2SelId, c4At);
            if (pitemCur)
                pitemNew = pitemCur->pitemDup();
            else
                pitemNew = new THAIOmniZone(strDef, c2SelId, tHAIOmniTCP2Sh::EZoneTypes::Alarm, 1);

            eType = tHAIOmniTCP2Sh::EItemTypes::Zone;
            pwndList = m_pwndZones;
            pitemOrg = m_dcfgOrg.pitemZoneById(c2SelId, c4At);
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
        //  Invoke the edit dialog. If the current pointer isn't zero, then there was
        //  an original one, and so the delete button can be enabled.
        //
        TEdItemDlg dlgEdit;
        const tHAIOmniTCP2C::EEditRes eRes = dlgEdit.eRunDlg
        (
            *this, *pitemNew, m_dcfgCurrent, pitemCur != 0
        );

        if (eRes == tHAIOmniTCP2C::EEditRes::Delete)
        {
            // Delete the selected item
            m_dcfgCurrent.RemoveItem(eType, c2SelId);

            //
            //  Zero out the new item pointer, so that we now see it as
            //  being removed. The delete button would only have been enabled
            //  if we had a previous one.
            //
            pitemNew = 0;
        }
         else if (eRes == tHAIOmniTCP2C::EEditRes::Save)
        {
            //
            //  Store the info. If it was an existing one, just copy it back in. Else,
            //  we need to add it.
            //
            if (pitemOrg)
            {
                if (widSrc == kHAIOmniTCP2C::ridIntf_AreaList)
                    m_dcfgCurrent.SetAreaAt(static_cast<THAIOmniArea&>(*pitemNew), c4At);
                else if (widSrc == kHAIOmniTCP2C::ridIntf_ThermoList)
                    m_dcfgCurrent.SetThermoAt(static_cast<THAIOmniThermo&>(*pitemNew), c4At);
                else if (widSrc == kHAIOmniTCP2C::ridIntf_UnitList)
                    m_dcfgCurrent.SetUnitAt(static_cast<THAIOmniUnit&>(*pitemNew), c4At);
                else if (widSrc == kHAIOmniTCP2C::ridIntf_ZoneList)
                    m_dcfgCurrent.SetZoneAt(static_cast<THAIOmniZone&>(*pitemNew), c4At);
            }
             else
            {
                if (widSrc == kHAIOmniTCP2C::ridIntf_AreaList)
                    m_dcfgCurrent.pitemAddArea(static_cast<THAIOmniArea&>(*pitemNew));
                else if (widSrc == kHAIOmniTCP2C::ridIntf_ThermoList)
                    m_dcfgCurrent.pitemAddThermo(static_cast<THAIOmniThermo&>(*pitemNew));
                else if (widSrc == kHAIOmniTCP2C::ridIntf_UnitList)
                    m_dcfgCurrent.pitemAddUnit(static_cast<THAIOmniUnit&>(*pitemNew));
                else if (widSrc == kHAIOmniTCP2C::ridIntf_ZoneList)
                    m_dcfgCurrent.pitemAddZone(static_cast<THAIOmniZone&>(*pitemNew));
            }
        }

        // And update the list slot for this guy if anything could have changed
        if (eRes != tHAIOmniTCP2C::EEditRes::Cancel)
        {
            // Update this list item to correctly show changes or not.
            tCIDLib::TStrList colVals(4);
            colVals.AddXCopies(TString::strEmpty(), 4);
            bUpdateCfgItem
            (
                *pwndList, colVals, pitemOrg, pitemNew, wnotEvent.c4Index(), c2SelId, eType
            );

            // Update this row
            pwndList->UpdateRowAt(colVals, wnotEvent.c4Index(), c2SelId, kCIDLib::True);
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}


//
//  Asks the server side driver for it's current configuration. Our server
//  side driver is a C++ driver, so we can get a binary blob back. We use
//  one of the backdoor driver calls to get it.
//
tCIDLib::TVoid THAIOmniTCP2CWnd::LoadConfig(tCQCKit::TCQCSrvProxy& orbcSrv)
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
        if (facHAIOmniTCP2C().bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        TExceptDlg dlgbFail
        (
            *this
            , strWndText()
            , facHAIOmniTCP2C().strMsg(kOmniTCP2CErrs::errcCfg_CantLoadConfig)
            , errToCatch
        );
    }
}

