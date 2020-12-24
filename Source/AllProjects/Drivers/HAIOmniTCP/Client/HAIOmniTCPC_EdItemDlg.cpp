//
// FILE NAME: HAIOmniTCPC_EdItemDlg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/13/2013
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
#include    "HAIOmniTCPC_.hpp"


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
tHAIOmniTCPC::EEditRes
TEdItemDlg::eRun(   const   TWindow&            wndOwner
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
        wndOwner, facHAIOmniTCPC(), kHAIOmniTCPC::ridDlg_EdItem
    );

    if (c4Res == kHAIOmniTCPC::ridDlg_EdItem_Save)
        return tHAIOmniTCPC::EEditRes_Save;
    else if (c4Res == kHAIOmniTCPC::ridDlg_EdItem_Delete)
        return tHAIOmniTCPC::EEditRes_Delete;

    return tHAIOmniTCPC::EEditRes_Cancel;
}


// ---------------------------------------------------------------------------
//  TEdItemDlg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TEdItemDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRet = TParent::bCreated();

    // Get some typed pointers to some of our widgets
    CastChildWnd(*this, kHAIOmniTCPC::ridDlg_EdItem_Cancel, m_pwndCancelButton);
    CastChildWnd(*this, kHAIOmniTCPC::ridDlg_EdItem_Delete, m_pwndDelButton);
    CastChildWnd(*this, kHAIOmniTCPC::ridDlg_EdItem_Name, m_pwndName);
    CastChildWnd(*this, kHAIOmniTCPC::ridDlg_EdItem_Save, m_pwndSaveButton);
    CastChildWnd(*this, kHAIOmniTCPC::ridDlg_EdItem_SendTrigs, m_pwndSendTrig);
    CastChildWnd(*this, kHAIOmniTCPC::ridDlg_EdItem_Types, m_pwndTypes);

    //
    //  Get the list of available subtypes from the item, and load them
    //  into the types list. Else, disable it.
    //
    tCIDLib::TCard4     c4CurType;
    tCIDLib::TStrList   colTypes;
    const tCIDLib::TCard4 c4TypesCnt = m_pitemEdit->c4QueryTypes(colTypes, c4CurType);
    if (c4TypesCnt)
    {
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4TypesCnt; c4Index++)
            m_pwndTypes->c4AddItem(colTypes[c4Index]);
        m_pwndTypes->SelectByIndex(0);
    }
     else
    {
        // Disable the prefix and the list
        pwndChildById(kHAIOmniTCPC::ridDlg_EdItem_TypesPref)->SetEnable(kCIDLib::False);
        m_pwndTypes->SetEnable(kCIDLib::False);
    }

    //
    //  Set or disable the send triggers check box as appropriate for the
    //  item's current configuration.
    //
    if (m_pitemEdit->bCanSendTrig())
        m_pwndSendTrig->SetCheckedState(m_pitemEdit->bSendTrig());
    else
        m_pwndSendTrig->SetEnable(kCIDLib::False);

    // Load up the item name
    m_pwndName->strWndText(m_pitemEdit->strName());

    // Disable the delete button if we aren't editing an existing item
    if (!m_bCanDelete)
        m_pwndDelButton->SetEnable(kCIDLib::False);

    // Register handlers now that we've set up initial values
    m_pwndCancelButton->pnothRegisterHandler(this, &TEdItemDlg::eClickHandler);
    m_pwndDelButton->pnothRegisterHandler(this, &TEdItemDlg::eClickHandler);
    m_pwndSaveButton->pnothRegisterHandler(this, &TEdItemDlg::eClickHandler);
    m_pwndTypes->pnothRegisterHandler(this, &TEdItemDlg::eLBHandler);

    return bRet;
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
        TErrBox msgbFailed(facHAIOmniTCPC().strMsg(kOmniTCPCErrs::errcCfg_EmptyName));
        msgbFailed.ShowIt(*this);
        m_pwndName->TakeFocus();
        return kCIDLib::False;
    }

    if ((strName != m_strOrgName) && m_pdcfgCurrent->bNameUsed(strName))
    {
        TErrBox msgbFailed(facHAIOmniTCPSh().strMsg(kOmniTCPShErrs::errcCfg_DupName, strName));
        msgbFailed.ShowIt(*this);
        m_pwndName->TakeFocus();
        return kCIDLib::False;
    }

    // Make sure it's a valid field name
    if (!facCQCKit().bIsValidBaseFldName(strName))
    {
        TErrBox msgbFailed(facHAIOmniTCPSh().strMsg(kOmniTCPShErrs::errcCfg_BadName));
        msgbFailed.ShowIt(*this);
        m_pwndName->TakeFocus();
        return kCIDLib::False;
    }


    //
    //  If we have any sub-types, then get the selected one and set it back
    //  on the item.
    //
    tCIDLib::TCard4 c4TypeInd = m_pwndTypes->c4CurItem();
    if (c4TypeInd != kCIDLib::c4MaxCard)
    {
        TString strVal;
        m_pwndTypes->ValueAt(c4TypeInd, strVal);
        m_pitemEdit->SetType(strVal);
    }

    // If the send trigs check box is enabled, store the value
    if (m_pwndSendTrig->bIsEnabled())
        m_pitemEdit->bSendTrig(m_pwndSendTrig->bCheckedState());

    // And store the name
    m_pitemEdit->strName(strName);
    return kCIDLib::True;
}


tCIDCtrls::EEvResponses TEdItemDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kHAIOmniTCPC::ridDlg_EdItem_Delete)
    {
        EndDlg(kHAIOmniTCPC::ridDlg_EdItem_Delete);
    }
     else if (wnotEvent.widSource() == kHAIOmniTCPC::ridDlg_EdItem_Cancel)
    {
        EndDlg(kHAIOmniTCPC::ridDlg_EdItem_Cancel);
    }
     else if (wnotEvent.widSource() == kHAIOmniTCPC::ridDlg_EdItem_Save)
    {
        //
        //  Validate the data. If good, then exit. Else, the user was told
        //  what was wrong and we stay in.
        //
        if (bValidate())
            EndDlg(kHAIOmniTCPC::ridDlg_EdItem_Save);
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
    if ((wnotEvent.widSource() == kHAIOmniTCPC::ridDlg_EdItem_Types)
    &&  (wnotEvent.eEvent() == tCIDCtrls::EListEvents::SelChanged))
    {
        // Set the new type on the item
        TString strType;
        m_pwndTypes->ValueAt(wnotEvent.c4Index(), strType);
        m_pitemEdit->SetType(strType);

        // Now ask again if it can send triggers
        if (m_pitemEdit->bCanSendTrig())
        {
            m_pwndSendTrig->SetEnable(kCIDLib::True);
            m_pwndSendTrig->SetCheckedState(m_pitemEdit->bSendTrig());
        }
         else
        {
            m_pwndSendTrig->SetCheckedState(kCIDLib::False);
            m_pwndSendTrig->SetEnable(kCIDLib::False);
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}
