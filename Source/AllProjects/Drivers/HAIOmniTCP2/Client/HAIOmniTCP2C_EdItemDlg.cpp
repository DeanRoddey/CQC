//
// FILE NAME: HAIOmniTCP2C_EdItemDlg.cpp
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
//  This file implements the item configuration editing dialog.
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
RTTIDecls(TEdItemDlg,TDlgBox)



// ---------------------------------------------------------------------------
//   CLASS: TEdItemDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TEdItemDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TEdItemDlg::TEdItemDlg() :

    m_pdcfgCurrent(nullptr)
    , m_pitemEdit(nullptr)
    , m_pwndAreas(nullptr)
    , m_pwndCancelButton(nullptr)
    , m_pwndDelButton(nullptr)
    , m_pwndName(nullptr)
    , m_pwndSaveButton(nullptr)
    , m_pwndSendTrig(nullptr)
    , m_pwndTypes(nullptr)
{
}

TEdItemDlg::~TEdItemDlg()
{
}


// ---------------------------------------------------------------------------
//  TEdItemDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tHAIOmniTCP2C::EEditRes
TEdItemDlg::eRunDlg(const   TWindow&            wndOwner
                    ,       THAIOmniItem&       itemEdit
                    , const TOmniTCPDrvCfg&     dcfgCurrent
                    , const tCIDLib::TBoolean   bCanDelete)
{
    // Save the incoming stuff for later use
    m_bCanDelete = bCanDelete;
    m_pdcfgCurrent = &dcfgCurrent;
    m_pitemEdit = &itemEdit;

    // Remember the original name
    m_strOrgName = m_pitemEdit->strName();

    const tCIDLib::TCard4 c4Res = c4RunDlg
    (
        wndOwner, facHAIOmniTCP2C(), kHAIOmniTCP2C::ridDlg_EdItem
    );

    if (c4Res == kHAIOmniTCP2C::ridDlg_EdItem_Save)
        return tHAIOmniTCP2C::EEditRes::Save;

    if (c4Res == kHAIOmniTCP2C::ridDlg_EdItem_Delete)
        return tHAIOmniTCP2C::EEditRes::Delete;

    return tHAIOmniTCP2C::EEditRes::Cancel;
}


// ---------------------------------------------------------------------------
//  TEdItemDlg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TEdItemDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRes = TParent::bCreated();

    // Get some typed pointers to some of our widgets
    CastChildWnd(*this, kHAIOmniTCP2C::ridDlg_EdItem_Areas, m_pwndAreas);
    CastChildWnd(*this, kHAIOmniTCP2C::ridDlg_EdItem_Cancel, m_pwndCancelButton);
    CastChildWnd(*this, kHAIOmniTCP2C::ridDlg_EdItem_Delete, m_pwndDelButton);
    CastChildWnd(*this, kHAIOmniTCP2C::ridDlg_EdItem_Name, m_pwndName);
    CastChildWnd(*this, kHAIOmniTCP2C::ridDlg_EdItem_Save, m_pwndSaveButton);
    CastChildWnd(*this, kHAIOmniTCP2C::ridDlg_EdItem_SendTrigs, m_pwndSendTrig);
    CastChildWnd(*this, kHAIOmniTCP2C::ridDlg_EdItem_Types, m_pwndTypes);

    //
    //  Get the list of available subtypes from the item, and load them into the types
    //  list. Else, disable it.
    //
    tCIDLib::TCard4     c4CurType;
    tCIDLib::TStrList   colTypes;
    const tCIDLib::TCard4 c4TypesCnt = m_pitemEdit->c4QueryTypes(colTypes, c4CurType);
    m_pwndTypes->SetEnable(c4TypesCnt != 0);
    if (c4TypesCnt)
    {
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4TypesCnt; c4Index++)
            m_pwndTypes->c4AddItem(colTypes[c4Index], c4Index);
        m_pwndTypes->SelectByIndex(c4CurType);
    }

    //
    //  Set or disable the send triggers check box as appropriate for the
    //  item's current configuration.
    //
    m_pwndSendTrig->SetEnable(m_pitemEdit->bCanSendTrig());
    if (m_pitemEdit->bCanSendTrig())
        m_pwndSendTrig->SetCheckedState(m_pitemEdit->bSendTrig());

    m_pwndName->strWndText(m_pitemEdit->strName());

    // Disable the delete button if we aren't editing an existing item
    m_pwndDelButton->SetEnable(m_bCanDelete);

    //
    //  If it's a zone, then load the areas list since they have to assign the
    //  zones to areas. Else disable it.
    //
    const tCIDLib::TCard4 c4AreaCnt = m_pdcfgCurrent->c4AreaCnt();
    if ((m_pitemEdit->eType() == tHAIOmniTCP2Sh::EItemTypes::Zone) && c4AreaCnt)
    {
        m_pwndAreas->SetEnable(kCIDLib::True);
        THAIOmniZone* pitemZone = static_cast<THAIOmniZone*>(m_pitemEdit);
        tCIDLib::TCard2 c2InitSelId = kCIDLib::c2MaxCard;
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4AreaCnt; c4Index++)
        {
            const THAIOmniArea& itemCur = m_pdcfgCurrent->itemAreaAt(c4Index);
            if (itemCur.c2ItemNum() == pitemZone->c4AreaNum())
                c2InitSelId = itemCur.c2ItemNum();
            m_pwndAreas->c4AddItem(itemCur.strName(), itemCur.c2ItemNum());
        }

        if (c2InitSelId == kCIDLib::c2MaxCard)
            m_pwndAreas->SelectByIndex(0);
        else
            m_pwndAreas->c4SelectById(c2InitSelId);
    }
     else
    {
        m_pwndAreas->SetEnable(kCIDLib::False);
    }

    // Register handlers now that we've set up initial values
    m_pwndCancelButton->pnothRegisterHandler(this, &TEdItemDlg::eClickHandler);
    m_pwndDelButton->pnothRegisterHandler(this, &TEdItemDlg::eClickHandler);
    m_pwndSaveButton->pnothRegisterHandler(this, &TEdItemDlg::eClickHandler);
    m_pwndTypes->pnothRegisterHandler(this, &TEdItemDlg::eLBHandler);

    return bRes;
}


// ---------------------------------------------------------------------------
//  TEdItemDlg: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Validates all of the data to make sure we are storing valid field data.
//  The only thing that can be wrong is the limit value. So we look through
//  and for each one that has a limit, we try to parse the limits. If it fails,
//  we tell the user and put the selection on that item.
//
tCIDLib::TBoolean TEdItemDlg::bValidate()
{
    // The name can't be empty or a dup
    TString strName = m_pwndName->strWndText();
    strName.StripWhitespace();
    if (strName.bIsEmpty())
    {
        TErrBox msgbFailed(facHAIOmniTCP2C().strMsg(kOmniTCP2CErrs::errcCfg_EmptyName));
        msgbFailed.ShowIt(*this);
        m_pwndName->TakeFocus();
        return kCIDLib::False;
    }

    if ((strName != m_strOrgName)
    &&  m_pdcfgCurrent->bNameUsed(strName, m_pitemEdit->eType()))
    {
        TErrBox msgbFailed(facHAIOmniTCP2Sh().strMsg(kOmniTCP2ShErrs::errcCfg_DupName, strName));
        msgbFailed.ShowIt(*this);
        m_pwndName->TakeFocus();
        return kCIDLib::False;
    }

    // The name has to be valid for a field name
    if (!facCQCKit().bIsValidFldName(strName, kCIDLib::True))
    {
        TErrBox msgbFailed(facHAIOmniTCP2C().strMsg(kOmniTCP2CErrs::errcCfg_FldName));
        msgbFailed.ShowIt(*this);
        m_pwndName->TakeFocus();
        return kCIDLib::False;
    }

    //
    //  If we have any sub-types, then get the selected one and set it back
    //  on the item.
    //
    const tCIDLib::TCard4 c4TypeInd = m_pwndTypes->c4CurItem();
    if (c4TypeInd != kCIDLib::c4MaxCard)
    {
        TString strType;
        m_pwndTypes->ValueAt(c4TypeInd, strType);
        m_pitemEdit->SetSubType(strType);
    }

    // If the send trigs check box is enabled, store the value
    if (m_pwndSendTrig->bIsEnabled())
        m_pitemEdit->bSendTrig(m_pwndSendTrig->bCheckedState());

    // And store the name
    m_pitemEdit->strName(strName);

    //
    //  If it's a zone, get the area and set that. If, by some chance, there are
    //  none, which shouldn't happen, assign it to area 1.
    //
    if (m_pitemEdit->eType() == tHAIOmniTCP2Sh::EItemTypes::Zone)
    {
        THAIOmniZone* pitemZone = static_cast<THAIOmniZone*>(m_pitemEdit);
        if (m_pwndAreas->c4ItemCount())
        {
            TString strArea;
            m_pwndAreas->ValueAt(m_pwndAreas->c4CurItem(), strArea);
            const THAIOmniArea* pitemArea = m_pdcfgCurrent->pitemFindArea(strArea);
            if (pitemArea)
                pitemZone->c4AreaNum(pitemArea->c2ItemNum());
            else
                pitemZone->c4AreaNum(1);
        }
         else
        {
            pitemZone->c4AreaNum(1);
        }
    }
    return kCIDLib::True;
}


tCIDCtrls::EEvResponses TEdItemDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kHAIOmniTCP2C::ridDlg_EdItem_Delete)
    {
        EndDlg(kHAIOmniTCP2C::ridDlg_EdItem_Delete);
    }
     else if (wnotEvent.widSource() == kHAIOmniTCP2C::ridDlg_EdItem_Cancel)
    {
        EndDlg(kHAIOmniTCP2C::ridDlg_EdItem_Cancel);
    }
     else if (wnotEvent.widSource() == kHAIOmniTCP2C::ridDlg_EdItem_Save)
    {
        //
        //  Validate the data. If good, then exit. Else, the user was told
        //  what was wrong and we stay in.
        //
        if (bValidate())
            EndDlg(kHAIOmniTCP2C::ridDlg_EdItem_Save);
    }
    return tCIDCtrls::EEvResponses::Handled;
}


//
//  We watch for selections in the sub-type list box. Sometimes, depending
//  on which sub-type is selected, the ability to send triggers may change,
//  so we want to update base don that.
//
tCIDCtrls::EEvResponses TEdItemDlg::eLBHandler(TListChangeInfo& wnotEvent)
{
    if ((wnotEvent.widSource() == kHAIOmniTCP2C::ridDlg_EdItem_Types)
    &&  (wnotEvent.eEvent() == tCIDCtrls::EListEvents::SelChanged))
    {
        // Set the new subtype on the item
        TString strType;
        m_pwndTypes->ValueAt(m_pwndTypes->c4CurItem(), strType);
        m_pitemEdit->SetSubType(strType);

        // Now ask again if it can send triggers
        m_pwndSendTrig->SetEnable(m_pitemEdit->bCanSendTrig());
        if (m_pitemEdit->bCanSendTrig())
            m_pwndSendTrig->SetCheckedState(m_pitemEdit->bSendTrig());
        else
            m_pwndSendTrig->SetEnable(kCIDLib::False);
    }
    return tCIDCtrls::EEvResponses::Handled;
}
