//
// FILE NAME: ElkM1V2C_Window.cpp
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
#include    "ElkM1V2C_.hpp"


// ---------------------------------------------------------------------------
//  Magic macro
// ---------------------------------------------------------------------------
RTTIDecls(TElkM1V2CWnd,TCQCDriverClient)



// ---------------------------------------------------------------------------
//   CLASS: TElkM1V2CWnd
//  PREFIX: wnd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TElkM1V2CWnd: Public, static methods
// ---------------------------------------------------------------------------

//
//  This provides common checking for the id/name parts that apply to all of the
//  info classes. This way they don't all have to replicate this code. We display
//  an error to the user. We return the status so that the dialog can put the focus
//  on the offending item if an error.
//
tElkM1V2C::EBaseValRes
TElkM1V2CWnd::eCheckBaseVals(const  TWindow&                wndOwner
                            ,       tElkM1V2C::TItemList&   colList
                            , const TString&                strName
                            , const TString&                strElkId
                            , const tCIDLib::TCard4         c4ListInd
                            , const tElkM1V2C::EItemTypes   eType
                            ,       tCIDLib::TCard4&        c4ElkId)
{
    if (strName.bIsEmpty())
    {
        TErrBox msgbErr(facElkM1V2C().strMsg(kElkM1V2CErrs::errcCfg_NoName));
        msgbErr.ShowIt(wndOwner);
        return tElkM1V2C::EBaseValRes::Name;
    }

    // Make sure the name is valid for a field
    if (!facCQCKit().bIsValidFldName(strName, kCIDLib::True))
    {
        TErrBox msgbErr(facElkM1V2C().strMsg(kElkM1V2CErrs::errcEdit_BadFldName));
        msgbErr.ShowIt(wndOwner);
        return tElkM1V2C::EBaseValRes::Name;
    }

    // We have to have an id
    if (strElkId.bIsEmpty() || !strElkId.bToCard4(c4ElkId))
    {
        TErrBox msgbErr(facElkM1V2C().strMsg(kElkM1V2CErrs::errcEdit_NoId));
        msgbErr.ShowIt(wndOwner);
        return tElkM1V2C::EBaseValRes::Id;
    }

    // Make sure it's legal for the type
    if (c4ElkId > kElkM1V2C::ac4MaxIds[eType])
    {
        TErrBox msgbErr
        (
            facElkM1V2C().strMsg
            (
                kElkM1V2CErrs::errcEdit_IdRange
                , TString(kElkM1V2C::apszTypes[eType])
                , TCardinal(kElkM1V2C::ac4MaxIds[eType])
            )
        );
        msgbErr.ShowIt(wndOwner);
        return tElkM1V2C::EBaseValRes::Id;
    }

    //
    //  Make sure it's not a dup name or id, (unless it's the original one). If
    //  this is a new one, the incoming index is max card, so we will check them
    //  all.
    //
    const tCIDLib::TCard4 c4Count = colList.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        // If not the same one as we are checking
        if (c4Index != c4ListInd)
        {
            if (colList.pobjAt(c4Index)->m_strName.bCompareI(strName))
            {
                TErrBox msgbErr
                (
                    facElkM1V2C().strMsg(kElkM1V2CErrs::errcEdit_DupName, strName)
                );
                msgbErr.ShowIt(wndOwner);
                return tElkM1V2C::EBaseValRes::Name;
            }

            if (colList.pobjAt(c4Index)->m_c4ElkId == c4ElkId)
            {
                TErrBox msgbErr
                (
                    facElkM1V2C().strMsg(kElkM1V2CErrs::errcEdit_DupId, TCardinal(c4ElkId))
                );
                msgbErr.ShowIt(wndOwner);
                return tElkM1V2C::EBaseValRes::Id;
            }
        }
    }
    return tElkM1V2C::EBaseValRes::OK;
}


// ---------------------------------------------------------------------------
//  TElkM1V2CWnd: Constructors and Destructor
// ---------------------------------------------------------------------------
TElkM1V2CWnd::TElkM1V2CWnd( const   TCQCDriverObjCfg&   cqcdcThis
                            , const TCQCUserCtx&        cuctxToUse) :

    TCQCDriverClient
    (
        cqcdcThis
        , L"TElkM1V2CWnd"
        , tCQCKit::EActLevels::Low
        , cuctxToUse
    )
    , m_colLists(tCIDLib::EAdoptOpts::Adopt, tElkM1V2C::EItemTypes::Count)
    , m_colModLists(tCIDLib::EAdoptOpts::Adopt, tElkM1V2C::EItemTypes::Count)
    , m_pwndAddButton(nullptr)
    , m_pwndClearButton(nullptr)
    , m_pwndEditButton(nullptr)
    , m_pwndItemList(nullptr)
    , m_pwndSettings(nullptr)
    , m_pwndTypeList(nullptr)
    , m_sfmtSettingsL(14, 0, tCIDLib::EHJustify::Right)
    , m_sfmtSettingsR(0, 2, tCIDLib::EHJustify::Left)
{
    //
    //  Go through the individual lists and allocate each list. THESE MUST be in the
    //  same order as the EItemTypes enum!
    //
    m_colLists.Add(new tElkM1V2C::TItemList(tCIDLib::EAdoptOpts::Adopt, kElkM1V2C::c4MaxAreas));
    m_colLists.Add(new tElkM1V2C::TItemList(tCIDLib::EAdoptOpts::Adopt, kElkM1V2C::c4MaxCounters));
    m_colLists.Add(new tElkM1V2C::TItemList(tCIDLib::EAdoptOpts::Adopt, kElkM1V2C::c4MaxLoads));
    m_colLists.Add(new tElkM1V2C::TItemList(tCIDLib::EAdoptOpts::Adopt, kElkM1V2C::c4MaxOutputs));
    m_colLists.Add(new tElkM1V2C::TItemList(tCIDLib::EAdoptOpts::Adopt, kElkM1V2C::c4MaxThermos));
    m_colLists.Add(new tElkM1V2C::TItemList(tCIDLib::EAdoptOpts::Adopt, kElkM1V2C::c4MaxThermoCpls));
    m_colLists.Add(new tElkM1V2C::TItemList(tCIDLib::EAdoptOpts::Adopt, kElkM1V2C::c4MaxVolts));
    m_colLists.Add(new tElkM1V2C::TItemList(tCIDLib::EAdoptOpts::Adopt, kElkM1V2C::c4MaxZones));


    // Do the same for the mod versions
    m_colModLists.Add(new tElkM1V2C::TItemList(tCIDLib::EAdoptOpts::Adopt, kElkM1V2C::c4MaxAreas));
    m_colModLists.Add(new tElkM1V2C::TItemList(tCIDLib::EAdoptOpts::Adopt, kElkM1V2C::c4MaxCounters));
    m_colModLists.Add(new tElkM1V2C::TItemList(tCIDLib::EAdoptOpts::Adopt, kElkM1V2C::c4MaxLoads));
    m_colModLists.Add(new tElkM1V2C::TItemList(tCIDLib::EAdoptOpts::Adopt, kElkM1V2C::c4MaxOutputs));
    m_colModLists.Add(new tElkM1V2C::TItemList(tCIDLib::EAdoptOpts::Adopt, kElkM1V2C::c4MaxThermos));
    m_colModLists.Add(new tElkM1V2C::TItemList(tCIDLib::EAdoptOpts::Adopt, kElkM1V2C::c4MaxThermoCpls));
    m_colModLists.Add(new tElkM1V2C::TItemList(tCIDLib::EAdoptOpts::Adopt, kElkM1V2C::c4MaxVolts));
    m_colModLists.Add(new tElkM1V2C::TItemList(tCIDLib::EAdoptOpts::Adopt, kElkM1V2C::c4MaxZones));
}


TElkM1V2CWnd::~TElkM1V2CWnd()
{
}


// ---------------------------------------------------------------------------
//  TElkM1V2CWnd: Public, inherited methods
// ---------------------------------------------------------------------------

// See if we have any changes
tCIDLib::TBoolean TElkM1V2CWnd::bChanges() const
{
    //
    //  The first quick check is just if any of the lists are of different sizes.
    //  If so, has to be changes.
    //
    tElkM1V2C::EItemTypes eType = tElkM1V2C::EItemTypes::Min;
    while (eType <= tElkM1V2C::EItemTypes::Max)
    {
        if (m_colModLists[eType]->c4ElemCount() != m_colLists[eType]->c4ElemCount())
            return kCIDLib::True;
        eType++;
    }

    //
    //  Let's compare the individual items them. The item classes provide a poly-
    //  morphic comparison method, so we can do this completely generically.
    //
    eType = tElkM1V2C::EItemTypes::Min;
    while (eType <= tElkM1V2C::EItemTypes::Max)
    {
        const tElkM1V2C::TItemList& colOrg = *m_colLists[eType];
        const tElkM1V2C::TItemList& colMod = *m_colModLists[eType];

        const tCIDLib::TCard4 c4Count = colMod.c4ElemCount();
        CIDAssert(c4Count == colOrg.c4ElemCount(), L"Lists should be the same size");

        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            if (!colMod[c4Index]->bComp(*colOrg.pobjAt(c4Index)))
                return kCIDLib::True;
        }
        eType++;
    }

    return kCIDLib::False;
}


tCIDLib::TBoolean TElkM1V2CWnd::bSaveChanges(TString& strErrMsg)
{
    try
    {
        // Format out the config in the exchange format
        TTextStringOutStream strmConfig(0x1000UL);
        TString strFmt;

        // Put out the version first
        strmConfig << L"Version=" << kElkM1V2C::c4ConfigVer << kCIDLib::NewLn;

        // Then all of the individual blocks
        tCIDLib::ForEachE<tElkM1V2C::EItemTypes>
        (
            [this, &strmConfig](const tElkM1V2C::EItemTypes eType)
            {
                const tElkM1V2C::TItemList& colMod = *m_colModLists[eType];

                // Output the block start
                strmConfig  << kElkM1V2C::apszBlocks[eType] << kCIDLib::chEquals
                            << kCIDLib::NewLn;

                const tCIDLib::TCard4 c4Count = colMod.c4ElemCount();
                for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
                {
                    colMod[c4Index]->FormatCfg(strmConfig);
                    strmConfig << kCIDLib::NewLn;
                }

                // Output the end block line
                strmConfig  << L"End" << kElkM1V2C::apszBlocks[eType] << kCIDLib::NewLn;
            }
        );

        // Send the changes, flush the stream first to get it all out
        strmConfig.Flush();
        orbcServer()->c4SendCmd(strMoniker(), L"TakeChanges", strmConfig.strData(), sectUser());

        // Copy all of the mod lists to the regular lists to clear changes
        StoreMods();
    }

    catch(TError& errToCatch)
    {
        if (facElkM1V2C().bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        strErrMsg = facElkM1V2C().strMsg(kElkM1V2CErrs::errcCfg_CantSendCfg);
        return kCIDLib::False;
    }
    return kCIDLib::True;
}



// ---------------------------------------------------------------------------
//  TElkM1V2CWnd: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TElkM1V2CWnd::bCreated()
{
    TParent::bCreated();

    // Load the dialog resource that defines our content and create our content
    TDlgDesc dlgdChildren;
    facElkM1V2C().bCreateDlgDesc(kElkM1V2C::ridIntf_Main, dlgdChildren);

    tCIDCtrls::TWndId widInitFocus;
    CreateDlgItems(dlgdChildren, widInitFocus);

    // Do an initial auto-size to get them in sync with our initial size
    AutoAdjustChildren(dlgdChildren.areaPos(), areaClient());

    // Get typed pointers to the widgets we want to interact with a lot
    CastChildWnd(*this, kElkM1V2C::ridIntf_Main_Add, m_pwndAddButton);
    CastChildWnd(*this, kElkM1V2C::ridIntf_Main_ClearChanges, m_pwndClearButton);
    CastChildWnd(*this, kElkM1V2C::ridIntf_Main_Delete, m_pwndDelButton);
    CastChildWnd(*this, kElkM1V2C::ridIntf_Main_Edit, m_pwndEditButton);
    CastChildWnd(*this, kElkM1V2C::ridIntf_Main_ItemList, m_pwndItemList);
    CastChildWnd(*this, kElkM1V2C::ridIntf_Main_Settings, m_pwndSettings);
    CastChildWnd(*this, kElkM1V2C::ridIntf_Main_TypeList, m_pwndTypeList);

    // Set up the settings window with a non-proportioal font
    m_pwndSettings->SetFont(TGUIFacility::gfontDefFixed());

    // Register event handlers
    m_pwndAddButton->pnothRegisterHandler(this, &TElkM1V2CWnd::eClickHandler);
    m_pwndClearButton->pnothRegisterHandler(this, &TElkM1V2CWnd::eClickHandler);
    m_pwndDelButton->pnothRegisterHandler(this, &TElkM1V2CWnd::eClickHandler);
    m_pwndEditButton->pnothRegisterHandler(this, &TElkM1V2CWnd::eClickHandler);
    m_pwndItemList->pnothRegisterHandler(this, &TElkM1V2CWnd::eLBHandler);
    m_pwndTypeList->pnothRegisterHandler(this, &TElkM1V2CWnd::eLBHandler);


    // Set the column titles on the lists
    tCIDLib::TStrList colVals(2);

    // The type list has one column
    colVals.objAdd(facElkM1V2C().strMsg(kElkM1V2CMsgs::midIntf_Main_TypesPref));
    m_pwndTypeList->SetColumns(colVals);

    // The item list has two columns
    colVals[0] = L"Elk Id";
    colVals.objAdd(facElkM1V2C().strMsg(kElkM1V2CMsgs::midIntf_Main_TypesPref));
    m_pwndItemList->SetColumns(colVals);

    // Auto-size the first column to fit the header. The other is auto-sized in the res file
    m_pwndItemList->AutoSizeCol(0, kCIDLib::True);

    //
    //  Load up the list of item types, which will kick off the other loading. It only
    //  has one column, so we have to remove the other value we added above.
    //
    colVals.RemoveAt(1);
    tElkM1V2C::EItemTypes eTypes = tElkM1V2C::EItemTypes::Min;
    while (eTypes <= tElkM1V2C::EItemTypes::Max)
    {
        colVals[0] = kElkM1V2C::apszTypes[eTypes];
        m_pwndTypeList->c4AddItem(colVals, tCIDLib::c4EnumOrd(eTypes));
        eTypes++;
    }

    return kCIDLib::True;
}


tCIDLib::TBoolean TElkM1V2CWnd::bGetConnectData(tCQCKit::TCQCSrvProxy& orbcTarget)
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


tCIDLib::TBoolean TElkM1V2CWnd::bDoPoll(tCQCKit::TCQCSrvProxy& orbcTarget)
{
    // We just display config data, so nothing to do here
    return kCIDLib::True;
}


tCIDLib::TVoid TElkM1V2CWnd::Connected()
{
    // Get initial data displayed by forcing a reselect of the 0th item type
    m_pwndTypeList->SelectByIndex(0, kCIDLib::True);
}



tCIDLib::TVoid TElkM1V2CWnd::DoCleanup()
{
    // Nothing for us to do currently
}


tCIDLib::TVoid TElkM1V2CWnd::LostConnection()
{
    // Nothing for us to do currently
}


tCIDLib::TVoid TElkM1V2CWnd::UpdateDisplayData()
{
    // We don't display any live data, just config
}



// ---------------------------------------------------------------------------
//  TElkM1V2CWnd: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Add a new item of the currently selected type
//
tCIDLib::TVoid TElkM1V2CWnd::AddNew()
{
    //  Get the type index and convert it to an item type
    tCIDLib::TCard4 c4TypeInd = m_pwndTypeList->c4CurItem();
    if (c4TypeInd == kCIDLib::c4MaxCard)
        return;
    const tElkM1V2C::EItemTypes eCurType = tElkM1V2C::EItemTypes(c4TypeInd);

    // Get the selected item of that type and invoke the edit dialog
    tCIDLib::TBoolean bChanges = kCIDLib::False;
    tCIDLib::TCard4 c4ElkId = kCIDLib::c4MaxCard;
    tElkM1V2C::TItemList& colList = *m_colModLists[eCurType];
    TItemInfo* piiNew = 0;
    switch(eCurType)
    {
        case tElkM1V2C::EItemTypes::Area :
        case tElkM1V2C::EItemTypes::Output :
        {
            //
            //  These all use a generic dialog. But in this case we have to
            //  allocate a temp to use. Put a janitor on it until we get it
            //  stored away.
            //
            TItemInfo* piiEdit;
            if (eCurType == tElkM1V2C::EItemTypes::Area)
                piiEdit = new TAreaInfo;
            else
                piiEdit = new TOutputInfo;
            TJanitor<TItemInfo> janEdit(piiEdit);

            // Load the appropriate instruction text
            tCIDLib::TMsgId midInstruct;
            if (eCurType == tElkM1V2C::EItemTypes::Area)
                midInstruct = kElkM1V2CMsgs::midDlg_EdArea_Instruct;
            else
                midInstruct = kElkM1V2CMsgs::midDlg_EdOutput_Instruct;

            // The changes are stored by the dialog if committed
            TEdM1GenDlg dlgEd;
            bChanges = dlgEd.bRunDlg
            (
                *this
                , kCIDLib::c4MaxCard
                , *m_colModLists[eCurType]
                , *piiEdit
                , facElkM1V2C().strMsg(midInstruct)
                , eCurType
            );

            // If committed, give the pointer over to be handled below
            if (bChanges)
                piiNew = janEdit.pobjOrphan();
            break;
        }

        case tElkM1V2C::EItemTypes::Counter :
        case tElkM1V2C::EItemTypes::Thermo :
        case tElkM1V2C::EItemTypes::ThermoCpl :
        {
            //
            //  These all share the same dialog. They all are name, id, and low/high
            //  limits.
            //
            TLimInfo* piiEdit;
            if (eCurType == tElkM1V2C::EItemTypes::Counter)
                piiEdit = new TCounterInfo;
            else if (eCurType == tElkM1V2C::EItemTypes::Thermo)
                piiEdit = new TThermoInfo;
            else
                piiEdit = new TThermoCplInfo;

            TJanitor<TItemInfo> janEdit(piiEdit);

            // Load the appropriate instruction text
            tCIDLib::TMsgId midInstruct;
            if (eCurType == tElkM1V2C::EItemTypes::Counter)
                midInstruct = kElkM1V2CMsgs::midDlg_EdCnt_Instruct;
            else if (eCurType == tElkM1V2C::EItemTypes::Thermo)
                midInstruct = kElkM1V2CMsgs::midDlg_EdThermo_Instruct;
            else
                midInstruct = kElkM1V2CMsgs::midDlg_EdThermoCpl_Instruct;

            // The changes are stored by the dialog if committed
            TEdM1GenLimDlg dlgEd;
            bChanges = dlgEd.bRunDlg
            (
                *this
                , kCIDLib::c4MaxCard
                , *m_colModLists[eCurType]
                , *piiEdit
                , facElkM1V2C().strMsg(midInstruct)
                , eCurType
            );

            // If committed, give the pointer over to be handled below
            if (bChanges)
                piiNew = janEdit.pobjOrphan();
            break;
        }


        case tElkM1V2C::EItemTypes::Load :
        {
            TLoadInfo iiEdit;
            TEdM1LoadDlg dlgEd;
            bChanges = dlgEd.bRunDlg(*this, kCIDLib::c4MaxCard, colList, iiEdit);

            if (bChanges)
                piiNew = new TLoadInfo(iiEdit);

            break;
        }

        case tElkM1V2C::EItemTypes::Volt :
        {
            TVoltInfo iiEdit;
            TEdM1VoltDlg dlgEd;
            bChanges = dlgEd.bRunDlg(*this, kCIDLib::c4MaxCard, colList, iiEdit);
            if (bChanges)
                piiNew = new TVoltInfo(iiEdit);
            break;
        }

        case tElkM1V2C::EItemTypes::Zone :
        {
            TZoneInfo iiEdit;
            TEdM1ZoneDlg dlgEd;
            bChanges = dlgEd.bRunDlg(*this, kCIDLib::c4MaxCard, colList, iiEdit);
            if (bChanges)
                piiNew = new TZoneInfo(iiEdit);
            break;
        }

        default :
            return;
    };


    // If changes, then add this item
    if (bChanges)
    {
        //
        //  Find the insert point, in Elk id order. We need to insert it into the
        //  mod list and the list box.
        //
        const tCIDLib::TCard4 c4Count = colList.c4ElemCount();
        tCIDLib::TCard4 c4Index = 0;
        for (; c4Index < c4Count; c4Index++)
        {
            if (colList[c4Index]->m_c4ElkId > piiNew->m_c4ElkId)
                break;
        }

        if (c4Index == c4Count)
            colList.Add(piiNew);
        else
            colList.InsertAt(piiNew, c4Index);

        // Insert the list item at the same spot
        TString strFmt;
        strFmt.SetFormatted(piiNew->m_c4ElkId);

        tCIDLib::TStrList colCols(2);
        colCols.objAdd(strFmt);
        colCols.objAdd(piiNew->m_strName);
        m_pwndItemList->c4AddItem(colCols, piiNew->m_c4ElkId, c4Index);
    }
}


//
//  Checks the source config data stream for an xxx= type starting line for a block.
//  End of stream is never expected, so we just let it throw if it happens.
//
tCIDLib::TVoid
TElkM1V2CWnd::CheckStart(       TTextInStream&      strmSrc
                        , const TString&            strExp)
{
    GetNextLine(strmSrc, m_strTmp);

    // If not what we are expecting, then throw
    if (!m_strTmp.bCompareI(strExp))
    {
        facElkM1V2C().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kElkM1V2CErrs::errcCfg_ExpBlock
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , strExp
            , m_strTmp
            , TCardinal(m_c4LineNum)
        );
    }
}


// We remove all of the items from all of the lists
tCIDLib::TVoid TElkM1V2CWnd::ClearAllData()
{
    tElkM1V2C::EItemTypes eType = tElkM1V2C::EItemTypes::Min;
    while (eType <= tElkM1V2C::EItemTypes::Max)
    {
        m_colLists[eType]->RemoveAll();
        m_colModLists[eType]->RemoveAll();
        eType++;
    }

    // Can't be any changes now
    m_pwndItemList->TakeFocus();
}


//
//  Copy over the contents of the original lists back to the mod lists, which
//  undoes any changes since the last save (or initial load.)
//
tCIDLib::TVoid TElkM1V2CWnd::ClearMods()
{
    tElkM1V2C::EItemTypes eType = tElkM1V2C::EItemTypes::Min;
    while (eType <= tElkM1V2C::EItemTypes::Max)
    {
        // Get the right lists for this type
        tElkM1V2C::TItemList& colTar = *m_colModLists[eType];
        tElkM1V2C::TItemList& colSrc = *m_colLists[eType];

        // Clear out the target list
        colTar.RemoveAll();

        // And now dup the mod list
        const tCIDLib::TCard4 c4Count = colSrc.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            colTar.Add
            (
                static_cast<TItemInfo*>(colSrc[c4Index]->pobjDuplicate())
            );
        }
        eType++;
    }

    // Force reload the data
    m_pwndTypeList->SelectByIndex(0, kCIDLib::True);
}



//
//  Called when the user selects the Delete button. IF there's a selected item, we
//  remove it.
//
tCIDLib::TVoid TElkM1V2CWnd::DeleteCurrent()
{
    //  Get the type index and convert it to an item type
    tCIDLib::TCard4 c4TypeInd = m_pwndTypeList->c4CurItem();
    if (c4TypeInd == kCIDLib::c4MaxCard)
        return;

    // Something has to be selected in the item list
    tCIDLib::TCard4 c4ItemInd = m_pwndItemList->c4CurItem();
    if (c4ItemInd == kCIDLib::c4MaxCard)
        return;

    const tElkM1V2C::EItemTypes eCurType = tElkM1V2C::EItemTypes(c4TypeInd);
    tElkM1V2C::TItemList& colList = *m_colModLists[eCurType];

    colList.RemoveAt(c4ItemInd);
    m_pwndItemList->RemoveAt(c4ItemInd);
}


//
//  This is called on a button click.
//
tCIDCtrls::EEvResponses TElkM1V2CWnd::eClickHandler(TButtClickInfo& wnotEvent)
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
            case kElkM1V2C::ridIntf_Main_Add :
            {
                // Add a new item of the current type
                AddNew();
                break;
            }

            case kElkM1V2C::ridIntf_Main_ClearChanges :
            {
                // Clear any local modifications and reload the lists
                ClearMods();
                m_pwndTypeList->SelectByIndex(0, kCIDLib::True);
                break;
            }

            case kElkM1V2C::ridIntf_Main_Delete :
            {
                DeleteCurrent();
                break;
            }

            case kElkM1V2C::ridIntf_Main_Edit :
            {
                // Edit the currently selected item, if any
                EditCurrent();
                break;
            }
        };
    }

    catch(TError& errToCatch)
    {
        if (facElkM1V2C().bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        TExceptDlg dlgbFail
        (
            *this
            , strWndText()
            , facElkM1V2C().strMsg(kElkM1V2CMsgs::midStatus_RemOpFailed)
            , errToCatch
        );
    }
    return tCIDCtrls::EEvResponses::Handled;
}


tCIDCtrls::EEvResponses TElkM1V2CWnd::eLBHandler(TListChangeInfo& wnotEvent)
{
    const tCIDCtrls::TWndId widSrc = wnotEvent.widSource();

    // On double clicks, we add/remove/change the item
    if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::Cleared)
    {
        if (widSrc == kElkM1V2C::ridIntf_Main_ItemList)
        {
            m_pwndSettings->ClearText();
        }
         else if (widSrc == kElkM1V2C::ridIntf_Main_TypeList)
        {
            // Clear out the item list since there's nothing now to display here
            m_pwndItemList->RemoveAll();
        }
    }
     else if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::SelChanged)
    {
        if (widSrc == kElkM1V2C::ridIntf_Main_ItemList)
        {
            // Get the currently selected type in the type list
            const tElkM1V2C::EItemTypes eType
            (
                tElkM1V2C::EItemTypes(m_pwndTypeList->c4CurItem())
            );

            // Use that to get the item for the selected item within that type
            const TItemInfo& iiCur = *(*m_colModLists[eType])[wnotEvent.c4Index()];

            // Update the settings window with the current item's settings
            TTextStringOutStream strmTar(8192UL);
            iiCur.FormatSettings(strmTar, m_sfmtSettingsL, m_sfmtSettingsR);
            strmTar.Flush();
            m_pwndSettings->strWndText(strmTar.strData());
        }
         else if (widSrc == kElkM1V2C::ridIntf_Main_TypeList)
        {
            //
            //  Update the item list with the items for the newly selected
            //  type. The indices of the types list box map directly to the
            //  types enum.
            //
            const tElkM1V2C::EItemTypes eType(tElkM1V2C::EItemTypes(wnotEvent.c4Index()));
            LoadItemList(eType);

            // Update the title of the name column to indicate the new type
            m_pwndItemList->SetColTitle(1, kElkM1V2C::apszTypes[eType]);
        }
    }
     else if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::Invoked)
    {
    }
    return tCIDCtrls::EEvResponses::Handled;
}


//
//  This will look at what tpe of thing is currently selected, and will invoke
//  the correct dialog box to let the user edit it.
//
tCIDLib::TVoid TElkM1V2CWnd::EditCurrent()
{
    //  Get the type index and convert it to an item type
    tCIDLib::TCard4 c4TypeInd = m_pwndTypeList->c4CurItem();
    if (c4TypeInd == kCIDLib::c4MaxCard)
        return;
    const tElkM1V2C::EItemTypes eCurType = tElkM1V2C::EItemTypes(c4TypeInd);

    // If there's nothing selected in the item list, then do nothing
    tCIDLib::TCard4 c4ItemInd = m_pwndItemList->c4CurItem();
    if (c4ItemInd == kCIDLib::c4MaxCard)
        return;

    // Get the selected item of that type and invoke the edit dialog
    tCIDLib::TBoolean bChanges = kCIDLib::False;
    switch(eCurType)
    {
        case tElkM1V2C::EItemTypes::Area :
        case tElkM1V2C::EItemTypes::Output :
        {
            // These all use a generic dialog
            TItemInfo* piiEdit = m_colModLists[eCurType]->pobjAt(c4ItemInd);

            // Load the appropriate instruction text
            tCIDLib::TMsgId midInstruct;
            if (eCurType == tElkM1V2C::EItemTypes::Area)
                midInstruct = kElkM1V2CMsgs::midDlg_EdArea_Instruct;
            else
                midInstruct = kElkM1V2CMsgs::midDlg_EdOutput_Instruct;

            // The changes are stored by the dialog if committed
            TEdM1GenDlg dlgEd;
            bChanges = dlgEd.bRunDlg
            (
                *this
                , c4ItemInd
                , *m_colModLists[eCurType]
                , *piiEdit
                , facElkM1V2C().strMsg(midInstruct)
                , eCurType
            );
            break;
        }

        case tElkM1V2C::EItemTypes::Counter :
        case tElkM1V2C::EItemTypes::Thermo :
        case tElkM1V2C::EItemTypes::ThermoCpl :
        {
            // These all use a generic dialog
            TLimInfo* piiEdit = static_cast<TLimInfo*>
            (
                m_colModLists[eCurType]->pobjAt(c4ItemInd)
            );

            // Load the appropriate instruction text
            tCIDLib::TMsgId midInstruct;
            if (eCurType == tElkM1V2C::EItemTypes::Counter)
                midInstruct = kElkM1V2CMsgs::midDlg_EdCnt_Instruct;
            else if (eCurType == tElkM1V2C::EItemTypes::Thermo)
                midInstruct = kElkM1V2CMsgs::midDlg_EdThermo_Instruct;
            else
                midInstruct = kElkM1V2CMsgs::midDlg_EdThermoCpl_Instruct;

            TEdM1GenLimDlg dlgEd;
            bChanges = dlgEd.bRunDlg
            (
                *this
                , c4ItemInd
                , *m_colModLists[eCurType]
                , *piiEdit
                , facElkM1V2C().strMsg(midInstruct)
                , eCurType
            );
            break;
        }

        case tElkM1V2C::EItemTypes::Load :
        {
            TLoadInfo iiEdit = *static_cast<TLoadInfo*>
            (
                m_colModLists[eCurType]->pobjAt(c4ItemInd)
            );

            TEdM1LoadDlg dlgEd;
            bChanges = dlgEd.bRunDlg(*this, c4ItemInd, *m_colModLists[eCurType], iiEdit);

            // Store the changes back if made
            if (bChanges)
                *static_cast<TLoadInfo*>(m_colModLists[eCurType]->pobjAt(c4ItemInd)) = iiEdit;
            break;
        }

        case tElkM1V2C::EItemTypes::Volt :
        {
            TVoltInfo iiEdit = *static_cast<TVoltInfo*>
            (
                m_colModLists[eCurType]->pobjAt(c4ItemInd)
            );

            TEdM1VoltDlg dlgEd;
            bChanges = dlgEd.bRunDlg(*this, c4ItemInd, *m_colModLists[eCurType], iiEdit);

            // Store the changes back if made
            if (bChanges)
                *static_cast<TVoltInfo*>(m_colModLists[eCurType]->pobjAt(c4ItemInd)) = iiEdit;
            break;
        }

        case tElkM1V2C::EItemTypes::Zone :
        {
            TZoneInfo iiEdit = *static_cast<TZoneInfo*>
            (
                m_colModLists[eCurType]->pobjAt(c4ItemInd)
            );

            TEdM1ZoneDlg dlgEd;
            bChanges = dlgEd.bRunDlg(*this, c4ItemInd, *m_colModLists[eCurType], iiEdit);

            // Store the changes back if made
            if (bChanges)
                *static_cast<TZoneInfo*>(m_colModLists[eCurType]->pobjAt(c4ItemInd)) = iiEdit;
            break;
        }

        default :
            return;
    };


    // If changes, then update this item
    if (bChanges)
    {
        TItemInfo* piiCur = m_colModLists[eCurType]->pobjAt(c4ItemInd);

        // Update the item list entry
        tCIDLib::TStrList colVals(2);
        TString strFmt;
        strFmt.SetFormatted(piiCur->m_c4ElkId);
        colVals.objAdd(strFmt);
        colVals.objAdd(piiCur->m_strName);
        m_pwndItemList->UpdateRowAt(colVals, c4ItemInd, piiCur->m_c4ElkId);

        // And force a reselect so that the currently selected item's info is udpated
        m_pwndItemList->SelectByIndex(c4ItemInd, kCIDLib::True);
    }
}


//
//  Get the next non-empty, non-comment line from the source stream, from which
//  we are streaming in config data.
//
tCIDLib::TVoid
TElkM1V2CWnd::GetNextLine(  TTextInStream&      strmSrc
                            , TString&          strToFill)
{
    while (kCIDLib::True)
    {
        strmSrc >> strToFill;
        strToFill.StripWhitespace();
        m_c4LineNum++;

        // If empty or a comment, try again
        if (strToFill.bIsEmpty() || (strToFill[0] == kCIDLib::chSemiColon))
            continue;

        // Break out
        break;
    }
}


//
//  Asks the server side driver for it's current configuration. Our server
//  side driver is a CML driver, so we use the text value query to get it as
//  as formatted string. The more common config set/query methods use binary
//  flattened C++ data, and we want to avoid having to try to understand the
//  format of CML flattened data.
//
tCIDLib::TVoid TElkM1V2CWnd::LoadConfig(tCQCKit::TCQCSrvProxy& orbcSrv)
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

        m_c4LineNum = 1;

        // Parse the version line
        GetNextLine(strmIn, m_strTmp);
        if (m_strTmp.bStartsWith(L"Version="))
        {
            m_strTmp.Cut(0, 8);
            const tCIDLib::TCard4 c4Version = m_strTmp.c4Val(tCIDLib::ERadices::Dec);

            if (!c4Version || (c4Version > kElkM1V2C::c4ConfigVer))
            {
            }
        }

        //
        //  Now we have to read in all of the configured items
        //
        {
            TAreaInfo iiArea;
            ParseBlock(strmIn, tElkM1V2C::EItemTypes::Area, L"Areas", iiArea);
        }

        {
            TCounterInfo iiCnt;
            ParseBlock(strmIn, tElkM1V2C::EItemTypes::Counter, L"Counters", iiCnt);
        }

        {
            TLoadInfo iiLoad;
            ParseBlock(strmIn, tElkM1V2C::EItemTypes::Load, L"Loads", iiLoad);
        }

        {
            TOutputInfo iiOutput;
            ParseBlock(strmIn, tElkM1V2C::EItemTypes::Output, L"Outputs", iiOutput);
        }

        {
            TThermoInfo iiThermo;
            ParseBlock(strmIn, tElkM1V2C::EItemTypes::Thermo, L"Thermos", iiThermo);
        }

        {
            TThermoCplInfo iiTCpl;
            ParseBlock(strmIn, tElkM1V2C::EItemTypes::ThermoCpl, L"ThermoCpls", iiTCpl);
        }

        {
            TVoltInfo iiVolt;
            ParseBlock(strmIn, tElkM1V2C::EItemTypes::Volt, L"Volts", iiVolt);
        }

        {
            TZoneInfo iiZone;
            ParseBlock(strmIn, tElkM1V2C::EItemTypes::Zone, L"Zones", iiZone);
        }

        // Sort all the lists by elk number
        tElkM1V2C::EItemTypes eType = tElkM1V2C::EItemTypes::Min;
        while (eType <= tElkM1V2C::EItemTypes::Max)
        {
            m_colModLists[eType]->Sort(TItemInfo::eSortByNum);
            eType++;
        }

        //
        //  OK, it worked, so let's copy the Mod versions tht we loaded into the
        //  regular lists.
        //
        StoreMods();
    }

    catch(const TError& errToCatch)
    {
        if (facElkM1V2C().bShouldLog(errToCatch))
            TModule::LogEventObj(errToCatch);

        TExceptDlg dlgbFail
        (
            *this
            , strWndText()
            , facElkM1V2C().strMsg(kElkM1V2CErrs::errcCfg_CantParseCfg)
            , errToCatch
        );
    }
}


//
//  Load up the item list with all of the configured items of the indicated type.
//  we'll just put them in in the order that they are in the list.
//
//  We store the elk ids in the row id storage of the list window item, in case we
//  need to get back to a list window item by id.
//
tCIDLib::TVoid TElkM1V2CWnd::LoadItemList(const tElkM1V2C::EItemTypes eType)
{
    // Get the correct list
    tElkM1V2C::TItemList& colTar = *m_colModLists[eType];

    // Stop painting while we update, then remove all and load up the new ones
    TWndPaintJanitor janPaint(this);
    m_pwndItemList->RemoveAll();

    const tCIDLib::TCard4 c4Count = colTar.c4ElemCount();
    if (c4Count)
    {
        tCIDLib::TStrList colVals(2);
        TString& strId = colVals.objAdd(TString::strEmpty());
        colVals.objAdd(TString::strEmpty());
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const TItemInfo& iiCur = *colTar[c4Index];

            strId.SetFormatted(iiCur.m_c4ElkId);
            colVals[1] = iiCur.m_strName;

            m_pwndItemList->c4AddItem(colVals, iiCur.m_c4ElkId);
        }

        m_pwndItemList->SelectByIndex(0, kCIDLib::True);
    }
}


//
//  Generically parse one of the config file blocks and fill in the appropriate
//  Mod list with the parsed in items of the indicated type.
//
tCIDLib::TVoid
TElkM1V2CWnd::ParseBlock(       TTextInStream&          strmSrc
                        , const tElkM1V2C::EItemTypes   eType
                        , const TString&                strType
                        ,       TItemInfo&              iiTmp)
{
    //
    //  Get the target list. We parse into the mod lists, and we'll later copy
    //  these to the other lists.
    //
    tElkM1V2C::TItemList& colTar = *m_colModLists[eType];

    // Check for the expected start line
    TString strCheck(strType);
    strCheck.Append(kCIDLib::chEquals);
    CheckStart(strmSrc, strCheck);

    // Build up the end line we'll watch for
    strCheck = L"End";
    strCheck.Append(strType);

    // And now loop until we hit the end line, processing lines until we do
    while (kCIDLib::True)
    {
        GetNextLine(strmSrc, m_strTmp);

        // Watch for the end of block indicator
        if (m_strTmp.bCompareI(strCheck))
            break;

        // Break out the id on the left, tokens on the right
        tCIDLib::TCard4 c4Id;
        if (!m_strTmp.bSplit(m_strTmp2, kCIDLib::chEquals)
        ||  !m_strTmp.bToCard4(c4Id))
        {
            facElkM1V2C().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kElkM1V2CErrs::errcCfg_LineId
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
                , TCardinal(m_c4LineNum)
            );
        }

        // Tokenize the right side
        m_stokTmp.Reset(&m_strTmp2, L",");
        m_stokTmp.c4BreakApart(m_colTokens);

        iiTmp.ParseCfg(c4Id, m_colTokens);
        colTar.Add(static_cast<TItemInfo*>(iiTmp.pobjDuplicate()));
    }
}


//
//  Copy over the contents of the Mod lists to the regular lists. This is done
//  before saving any changes to the server or after downloading and parsing new
//  config.
//
tCIDLib::TVoid TElkM1V2CWnd::StoreMods()
{
    tElkM1V2C::EItemTypes eType = tElkM1V2C::EItemTypes::Min;
    while (eType <= tElkM1V2C::EItemTypes::Max)
    {
        // Get the right lists for this type
        tElkM1V2C::TItemList& colSrc = *m_colModLists[eType];
        tElkM1V2C::TItemList& colTar = *m_colLists[eType];

        // Clear out the target list
        colTar.RemoveAll();

        // And now dup the mod list
        const tCIDLib::TCard4 c4Count = colSrc.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            colTar.Add
            (
                static_cast<TItemInfo*>(colSrc[c4Index]->pobjDuplicate())
            );
        }
        eType++;
    }
}


