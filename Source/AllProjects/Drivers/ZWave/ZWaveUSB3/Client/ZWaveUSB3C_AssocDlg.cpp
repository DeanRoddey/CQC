//
// FILE NAME: ZWaveUSB3C_AssocDlg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/13/2018
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
//  This file implements a small dialog that lets the user manage unit associations
//  manually.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "ZWaveUSB3C_.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TZWUSB3AssocDlg,TDlgBox)



// ---------------------------------------------------------------------------
//   CLASS: TZWUSB3AssocDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWUSB3AssocDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TZWUSB3AssocDlg::TZWUSB3AssocDlg() :

    m_pdcfgEdit(nullptr)
    , m_punitiTar(nullptr)
    , m_pwndAddAssoc(nullptr)
    , m_pwndAssocList(nullptr)
    , m_pwndClose(nullptr)
    , m_pwndDelAssoc(nullptr)
    , m_pwndDriver(nullptr)
    , m_pwndEndPnt(nullptr)
    , m_pwndGroupId(nullptr)
    , m_pwndIsAwake(nullptr)
    , m_pwndQueryGrp(nullptr)
    , m_pwndToAdd(nullptr)
{
}

TZWUSB3AssocDlg::~TZWUSB3AssocDlg()
{
}


// ---------------------------------------------------------------------------
//  TZWUSB3AssocDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TZWUSB3AssocDlg::RunDlg(        TZWaveUSB3CWnd& wndDriver
                        , const TZWaveDrvCfg&   dcfgEdit
                        , const TZWUnitInfo&    unitiTar)
{
    // Save pointers to the incoming for later use
    m_pdcfgEdit = &dcfgEdit;
    m_punitiTar = &unitiTar;
    m_pwndDriver = &wndDriver;

    c4RunDlg(wndDriver, facZWaveUSB3C(), kZWaveUSB3C::ridDlg_Assoc);
}


// ---------------------------------------------------------------------------
//  TZWUSB3AssocDlg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TZWUSB3AssocDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRet = TParent::bCreated();

    // Store our pre-typed child widget pointers
    CastChildWnd(*this, kZWaveUSB3C::ridDlg_Assoc_AddAssoc, m_pwndAddAssoc);
    CastChildWnd(*this, kZWaveUSB3C::ridDlg_Assoc_AssocList, m_pwndAssocList);
    CastChildWnd(*this, kZWaveUSB3C::ridDlg_Assoc_Close, m_pwndClose);
    CastChildWnd(*this, kZWaveUSB3C::ridDlg_Assoc_DelAssoc, m_pwndDelAssoc);
    CastChildWnd(*this, kZWaveUSB3C::ridDlg_Assoc_EndPnt, m_pwndEndPnt);
    CastChildWnd(*this, kZWaveUSB3C::ridDlg_Assoc_GroupId, m_pwndGroupId);
    CastChildWnd(*this, kZWaveUSB3C::ridDlg_Assoc_IsAwake, m_pwndIsAwake);
    CastChildWnd(*this, kZWaveUSB3C::ridDlg_Assoc_QueryGrp, m_pwndQueryGrp);
    CastChildWnd(*this, kZWaveUSB3C::ridDlg_Assoc_ToAdd, m_pwndToAdd);

    // Load up the potential group ids into the combo box and select 1 initially
    m_pwndGroupId->LoadNumRange(1, 127);
    m_pwndGroupId->SelectByIndex(0);

    // Do the same for end points, but we need a special 'None' value at the start
    m_pwndEndPnt->LoadNumRange(1, 127);
    m_pwndEndPnt->InsertItem(0, L"None");
    m_pwndEndPnt->SelectByIndex(0);

    //
    //  Load up the units available to add to this one, skipping the target so that
    //  we don't have to check for adding to itself. We also add a special one for
    //  our driver.
    //
    m_pwndToAdd->c4AddItem(L"[Driver]", m_pdcfgEdit->c1CtrlId());
    const tZWaveUSB3Sh::TUnitInfoList& colLoad = m_pdcfgEdit->colUnits();
    const tCIDLib::TCard4 c4Count = colLoad.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TZWUnitInfo& unitiCur = colLoad[c4Index];
        if (unitiCur.c1Id() != m_punitiTar->c1Id())
            m_pwndToAdd->c4AddItem(unitiCur.strName(), unitiCur.c1Id());
    }
    m_pwndToAdd->SelectByIndex(0);

    // Set the columns on the association list
    tCIDLib::TStrList colCols(2);
    colCols.objAdd(L"Target Id");
    colCols.objAdd(L"End Point");
    m_pwndAssocList->SetColumns(colCols);

    // And register our button event handlers
    m_pwndAddAssoc->pnothRegisterHandler(this, &TZWUSB3AssocDlg::eClickHandler);
    m_pwndClose->pnothRegisterHandler(this, &TZWUSB3AssocDlg::eClickHandler);
    m_pwndDelAssoc->pnothRegisterHandler(this, &TZWUSB3AssocDlg::eClickHandler);
    m_pwndQueryGrp->pnothRegisterHandler(this, &TZWUSB3AssocDlg::eClickHandler);

    // If the unit is a live listener, then check and disable the awake check box
    if (m_punitiTar->bAlwaysOn())
    {
        m_pwndIsAwake->SetCheckedState(kCIDLib::True);
        m_pwndIsAwake->SetEnable(kCIDLib::False);
    }

    //
    //  Update the title text to include the unit name. Save the original text for
    //  use in popups.
    //
    m_strTitle = strWndText();
    TString strTitle = m_strTitle;
    strTitle.Append(L" [");
    strTitle.Append(m_punitiTar->strName());
    strTitle.Append(L"]");
    strWndText(strTitle);

    return bRet;
}


// ---------------------------------------------------------------------------
//  TZWUSB3AssocDlg: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Called when the user uses the add assocation button.
tCIDLib::TVoid TZWUSB3AssocDlg::AddAssoc()
{
    // Get the selected info that we need
    tCIDLib::TCard1 c1ToAdd;
    if (!bGetToAdd(c1ToAdd))
        return;

    tCIDLib::TCard1 c1TarGroup;
    if (!bGetTarGroup(c1TarGroup))
        return;

    tCIDLib::TCard1 c1EPId;
    if (!bGetEndPoint(c1EPId))
        return;

    TZWDriverXData zwdxdComm;
    zwdxdComm.m_strCmd = kZWaveUSB3Sh::strSendData_SetAssoc;
    zwdxdComm.m_c1UnitId = m_punitiTar->c1Id();
    zwdxdComm.m_c1Val1 = c1TarGroup;
    zwdxdComm.m_c1Val2 = c1ToAdd;
    zwdxdComm.m_c1EPId = c1EPId;
    zwdxdComm.m_bAwake = m_pwndIsAwake->bCheckedState();
    THeapBuf mbufComm(512, 512);

    // Whether it works or not we clear the list, if only to reload it
    TWndPtrJanitor janWait(tCIDCtrls::ESysPtrs::Wait);
    if (m_pwndDriver->bSendSrvMsg(*this, zwdxdComm, mbufComm))
    {
        // If they were queued, warn about that
        if (!m_punitiTar->bAlwaysOn() && !m_pwndIsAwake->bCheckedState())
        {
            TOkBox msgbWarn(m_strTitle, facZWaveUSB3C().strMsg(kZWU3CMsgs::midWarn_OpPending));
            msgbWarn.ShowIt(*this);
        }
         else
        {
            // Assume it worked and add it to the list
            tCIDLib::TStrList colCols;
            colCols.AddXCopies(TString::strEmpty(), 2);
            colCols[0].SetFormatted(c1ToAdd);

            if (c1EPId != kCIDLib::c1MaxCard)
                colCols[1].SetFormatted(c1EPId);

            // Store the combined values in the row id
            tCIDLib::TCard4 c4RowId = c1ToAdd;
            c4RowId <<= 16;
            c4RowId |= c1EPId;
            m_pwndAssocList->c4AddItem(colCols, c4RowId);
        }
    }
     else
    {
        TErrBox msgbErr(m_strTitle, zwdxdComm.m_strMsg);
        msgbErr.ShowIt(*this);
    }
}


//
//  Called when the user invokes any action. We make sure that the unit is a listener
//  or they have told it is awake. If not, we warn them.
//
tCIDLib::TBoolean TZWUSB3AssocDlg::bCheckAwake() const
{
    if (!m_punitiTar->bAlwaysOn() && !m_pwndIsAwake->bCheckedState())
    {
        TYesNoBox msgbAck(m_strTitle, facZWaveUSB3C().strMsg(kZWU3CMsgs::midQ_MsgsWillBeQd));
        return msgbAck.bShowIt(*this);
    }
    return kCIDLib::True;
}


//
//  Some helpers to get user selected values from the combo boxes
//
tCIDLib::TBoolean TZWUSB3AssocDlg::bGetEndPoint(tCIDLib::TCard1& c1ToFill) const
{
    //
    //  It's not read only so they can type in a value. So we need to get the text
    //  value and test it. And it can also be the special value 'None'.
    //
    TString strVal;
    if (!m_pwndEndPnt->bQueryCurVal(strVal))
    {
        TErrBox msgbErr(L"The target end point must be 'None' or 0 to 127");
        return kCIDLib::False;
    }

    if (strVal.bCompareI(L"None"))
    {
        c1ToFill = 0xFF;
    }
     else if (!strVal.bToCard1(c1ToFill, tCIDLib::ERadices::Dec) || (c1ToFill > 127))
    {
        TErrBox msgbErr(L"The target end point must be 'None' or 0 to 127");
        return kCIDLib::False;
    }

    return kCIDLib::True;
}

tCIDLib::TBoolean TZWUSB3AssocDlg::bGetTarGroup(tCIDLib::TCard1& c1ToFill) const
{
    //
    //  It's not read only so they can type in a value. So we need to get the text
    //  value and test it.
    //
    TString strVal;
    if (!m_pwndGroupId->bQueryCurVal(strVal)
    ||  !strVal.bToCard1(c1ToFill, tCIDLib::ERadices::Dec))
    {
        TErrBox msgbErr(L"The target group id must be a number from 0 to 255");
        return kCIDLib::False;
    }
    return kCIDLib::True;
}

tCIDLib::TBoolean TZWUSB3AssocDlg::bGetToAdd(tCIDLib::TCard1& c1ToFill) const
{
    // This one is read only so we just need to get the value, if any
    const tCIDLib::TCard4 c4Index = m_pwndToAdd->c4CurItem();
    if (c4Index == kCIDLib::c4MaxCard)
    {
        TErrBox msgbErr(L"No units are available to add to the target unit");
        return kCIDLib::False;
    }

    // Get the row id, which is the unit id for this unit
    c1ToFill = tCIDLib::TCard1(m_pwndToAdd->c4IndexToId(c4Index));
    return kCIDLib::True;
}


//
//  This is called when the user hits the delete button. If we have loaded any
//  associations, then we will try to delete the selected one. We stored the target
//  id and end point as the row id, so we can easily get the info we need.
//
tCIDLib::TVoid TZWUSB3AssocDlg::DeleteAssoc()
{
    // If nothing selected do nothing
    const tCIDLib::TCard4 c4ListInd = m_pwndAssocList->c4CurItem();
    if (c4ListInd == kCIDLib::c4MaxCard)
        return;

    // Get the selected target group. If it fails, do nothing
    tCIDLib::TCard1 c1TarGroup;
    if (!bGetTarGroup(c1TarGroup))
        return;

    //
    //  Check the awake situation. They may not mean to do this if the unit is
    //  not currently awake.
    //
    if (!bCheckAwake())
        return;

    // Get the row id which has the target info
    const tCIDLib::TCard4 c4RowId = m_pwndAssocList->c4IndexToId(c4ListInd);
    const tCIDLib::TCard1 c1ToRemove = tCIDLib::TCard1(c4RowId >> 16);
    const tCIDLib::TCard1 c1EPId = tCIDLib::TCard1(c4RowId);

    // Make sure they really want to do it
    {
        // If we have an end point, insert it, else empty string
        TString strEP;
        if (c1EPId != 0xFF)
        {
            strEP = L" (End point=";
            strEP.AppendFormatted(c1EPId);
            strEP.Append(L") ");
        }

        TYesNoBox msgbConfirm
        (
            m_strTitle
            , facZWaveUSB3C().strMsg
              (
                kZWU3CMsgs::midQ_RemoveAssoc
                , TCardinal(c1ToRemove)
                , strEP
                , TCardinal(c1TarGroup)
                , m_punitiTar->strName()
              )
        );
        if (!msgbConfirm.bShowIt(*this))
            return;
    }

    // Let's give it a try
    TZWDriverXData zwdxdComm;
    zwdxdComm.m_strCmd = kZWaveUSB3Sh::strSendData_RemoveAssoc;
    zwdxdComm.m_c1UnitId = m_punitiTar->c1Id();
    zwdxdComm.m_c1Val1 = c1TarGroup;
    zwdxdComm.m_c1Val2 = c1ToRemove;
    zwdxdComm.m_c1EPId = c1EPId;
    zwdxdComm.m_bAwake = m_pwndIsAwake->bCheckedState();
    THeapBuf mbufComm(512, 512);

    // Whether it works or not we clear the list, if only to reload it
    TWndPtrJanitor janWait(tCIDCtrls::ESysPtrs::Wait);
    if (m_pwndDriver->bSendSrvMsg(*this, zwdxdComm, mbufComm))
    {
        //
        //  If they were queued, warn about that, else assume it worked and remove it
        //  from our list.
        //
        if (!m_punitiTar->bAlwaysOn() && !m_pwndIsAwake->bCheckedState())
        {
            TOkBox msgbWarn(m_strTitle, facZWaveUSB3C().strMsg(kZWU3CMsgs::midWarn_OpPending));
            msgbWarn.ShowIt(*this);
        }
         else
        {
            m_pwndAssocList->RemoveAt(c4ListInd);
        }
    }
     else
    {
        TErrBox msgbErr(m_strTitle, zwdxdComm.m_strMsg);
        msgbErr.ShowIt(*this);
    }
}


// Handle button presses
tCIDCtrls::EEvResponses TZWUSB3AssocDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kZWaveUSB3C::ridDlg_Assoc_AddAssoc)
    {
        AddAssoc();
    }
     else if (wnotEvent.widSource() == kZWaveUSB3C::ridDlg_Assoc_Close)
    {
        EndDlg(kZWaveUSB3C::ridDlg_Assoc_Close);
    }
     else if (wnotEvent.widSource() == kZWaveUSB3C::ridDlg_Assoc_DelAssoc)
    {
        DeleteAssoc();
    }
     else if (wnotEvent.widSource() == kZWaveUSB3C::ridDlg_Assoc_QueryGrp)
    {
        // Load up the selected group's associations to the list
        QueryAssocs();
    }
    return tCIDCtrls::EEvResponses::Handled;
}


//
//  We need to watch the group combo box. If the selection changes, we clear the
//  current association query list since it doesn't apply to the selected group
//  anymore.
//
tCIDCtrls::EEvResponses TZWUSB3AssocDlg::eLBHandler(TListChangeInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kZWaveUSB3C::ridDlg_Assoc_GroupId)
    {
         if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::SelChanged)
            m_pwndAssocList->RemoveAll();
    }

    return tCIDCtrls::EEvResponses::Handled;
}


//
//  When the user hits the query group button this is called. We empty the list,
//  query the selected group's associations and loads them to the list, if any.
//
tCIDLib::TVoid TZWUSB3AssocDlg::QueryAssocs()
{
    //
    //  Check the awake situation. They may not mean to do this if the unit is
    //  not currently awake.
    //
    if (!bCheckAwake())
        return;

    // Get the target group
    tCIDLib::TCard1 c1Group;
    if (!bGetTarGroup(c1Group))
        return;

    // And let's ask for the associations for this group
    TZWDriverXData zwdxdComm;
    zwdxdComm.m_strCmd = kZWaveUSB3Sh::strSendData_QueryAssoc;
    zwdxdComm.m_c1UnitId = m_punitiTar->c1Id();
    zwdxdComm.m_c1Val1 = c1Group;
    zwdxdComm.m_bAwake = m_pwndIsAwake->bCheckedState();
    THeapBuf mbufComm(2048, 8192);


    // Whether it works or not we clear the list, if only to reload it
    TWndPtrJanitor janWait(tCIDCtrls::ESysPtrs::Wait);
    m_pwndAssocList->RemoveAll();
    if (m_pwndDriver->bSendSrvMsg(*this, zwdxdComm, mbufComm))
    {
        // It's a set of byte pairs in the memory buffer
        const tCIDLib::TCard4 c4Count = zwdxdComm.m_c4BufSz;
        if (c4Count)
        {
            tCIDLib::TStrList colCols;
            colCols.AddXCopies(TString::strEmpty(), 2);
            tCIDLib::TCard4 c4Index = 0;
            while (c4Index < c4Count)
            {
                const tCIDLib::TCard1 c1TarId = zwdxdComm.m_mbufData[c4Index++];
                colCols[0].SetFormatted(c1TarId);

                const tCIDLib::TCard1 c1EP = zwdxdComm.m_mbufData[c4Index++];
                if (c1EP != 0xFF)
                    colCols[1].SetFormatted(c1EP);
                else
                    colCols[1].Clear();

                //
                //  Set the combined values as the row id for convenience later.
                //  The target id goes into the top word and the end point into
                //  the bottom.
                //
                tCIDLib::TCard4 c4Id = c1TarId;
                c4Id <<= 16;
                c4Id |= c1EP;
                m_pwndAssocList->c4AddItem(colCols, c4Id);
            }

            m_pwndAssocList->SelectByIndex(0);
        }
    }
     else
    {
        TErrBox msgbErr(m_strTitle, zwdxdComm.m_strMsg);
        msgbErr.ShowIt(*this);
    }
}
