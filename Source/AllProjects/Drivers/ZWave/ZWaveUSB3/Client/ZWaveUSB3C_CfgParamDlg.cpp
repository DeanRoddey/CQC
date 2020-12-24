//
// FILE NAME: ZWaveUSB3C_CfgParamDlg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/17/2018
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
//  This file implements a small dialog that lets the user manage unit configuration
//  parameters manually.
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
RTTIDecls(TZWUSB3CfgParamDlg,TDlgBox)



// ---------------------------------------------------------------------------
//   CLASS: TZWUSB3CfgParamDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWUSB3CfgParamDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TZWUSB3CfgParamDlg::TZWUSB3CfgParamDlg() :

    m_eRadix(tCIDLib::ERadices::Dec)
    , m_punitiTar(nullptr)
    , m_pwndClose(nullptr)
    , m_pwndDriver(nullptr)
    , m_pwndBytes(nullptr)
    , m_pwndIsAwake(nullptr)
    , m_pwndParamNum(nullptr)
    , m_pwndQueryValue(nullptr)
    , m_pwndSetValue(nullptr)
    , m_pwndRadix(nullptr)
    , m_pwndValue(nullptr)
{
}

TZWUSB3CfgParamDlg::~TZWUSB3CfgParamDlg()
{
}


// ---------------------------------------------------------------------------
//  TZWUSB3CfgParamDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TZWUSB3CfgParamDlg::RunDlg(TZWaveUSB3CWnd& wndDriver, const TZWUnitInfo& unitiTar)
{
    // Save pointers to the incoming for later use
    m_punitiTar = &unitiTar;
    m_pwndDriver = &wndDriver;

    c4RunDlg(wndDriver, facZWaveUSB3C(), kZWaveUSB3C::ridDlg_CfgParams);
}


// ---------------------------------------------------------------------------
//  TZWUSB3CfgParamDlg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TZWUSB3CfgParamDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRet = TParent::bCreated();

    // Store our pre-typed child widget pointers
    CastChildWnd(*this, kZWaveUSB3C::ridDlg_CfgParams_Close, m_pwndClose);
    CastChildWnd(*this, kZWaveUSB3C::ridDlg_CfgParams_Bytes, m_pwndBytes);
    CastChildWnd(*this, kZWaveUSB3C::ridDlg_CfgParams_IsAwake, m_pwndIsAwake);
    CastChildWnd(*this, kZWaveUSB3C::ridDlg_CfgParams_ParamNum, m_pwndParamNum);
    CastChildWnd(*this, kZWaveUSB3C::ridDlg_CfgParams_QueryValue, m_pwndQueryValue);
    CastChildWnd(*this, kZWaveUSB3C::ridDlg_CfgParams_Radix, m_pwndRadix);
    CastChildWnd(*this, kZWaveUSB3C::ridDlg_CfgParams_SetValue, m_pwndSetValue);
    CastChildWnd(*this, kZWaveUSB3C::ridDlg_CfgParams_Value, m_pwndValue);

    //
    //  Load up the availble parameters combo. We only load up 64 values, but they
    //  can type in others.
    //
    m_pwndParamNum->LoadNumRange(1, 64);
    m_pwndParamNum->SelectByIndex(0);

    // Load up the valid byte sizes
    m_pwndBytes->c4AddItem(L"1");
    m_pwndBytes->c4AddItem(L"2");
    m_pwndBytes->c4AddItem(L"4");
    m_pwndBytes->SelectByIndex(0);

    //
    //  Load up the radices we can show the value in. Initially select decimal since
    //  that is what we default m_eRadix to.
    //
    m_pwndRadix->c4AddItem(L"Bin");
    m_pwndRadix->c4AddItem(L"Dec");
    m_pwndRadix->c4AddItem(L"Hex");
    m_eRadix = tCIDLib::ERadices::Dec;
    m_pwndRadix->SelectByIndex(1);

    // And register our button event handlers
    m_pwndClose->pnothRegisterHandler(this, &TZWUSB3CfgParamDlg::eClickHandler);
    m_pwndQueryValue->pnothRegisterHandler(this, &TZWUSB3CfgParamDlg::eClickHandler);
    m_pwndRadix->pnothRegisterHandler(this, &TZWUSB3CfgParamDlg::eLBHandler);
    m_pwndSetValue->pnothRegisterHandler(this, &TZWUSB3CfgParamDlg::eClickHandler);

    // If the unit is always on, then check and disable the awake check box
    if (m_punitiTar->bAlwaysOn())
    {
        m_pwndIsAwake->SetCheckedState(kCIDLib::True);
        m_pwndIsAwake->SetEnable(kCIDLib::False);
    }

    //
    //  Update the title text to include the unit name. Save the original text for
    //  use in popups.
    //
    m_strTitleText = strWndText();
    TString strTitle = m_strTitleText;
    strTitle.Append(L" [");
    strTitle.Append(m_punitiTar->strName());
    strTitle.Append(L"]");
    strWndText(strTitle);

    return bRet;
}


// ---------------------------------------------------------------------------
//  TZWUSB3CfgParamDlg: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Called when the user invokes any action. We make sure that the unit is a listener
//  or they have told it is awake. If not, we warn them.
//
tCIDLib::TBoolean TZWUSB3CfgParamDlg::bCheckAwake() const
{
    if (!m_punitiTar->bAlwaysOn() && !m_pwndIsAwake->bCheckedState())
    {
        TYesNoBox msgbAck
        (
            m_strTitleText, facZWaveUSB3C().strMsg(kZWU3CMsgs::midQ_MsgsWillBeQd)
        );
        return msgbAck.bShowIt(*this);
    }
    return kCIDLib::True;
}


tCIDLib::TBoolean TZWUSB3CfgParamDlg::bGetParamNum(tCIDLib::TCard1& c1ToFill) const
{
    //
    //  It's not read only so they can type in a value. So we need to get the text
    //  value and test it.
    //
    TString strVal;
    if (!m_pwndParamNum->bQueryCurVal(strVal)
    ||  !strVal.bToCard1(c1ToFill, tCIDLib::ERadices::Dec))
    {
        TErrBox msgbErr(L"The paramber number must be a number from 0 to 255");
        return kCIDLib::False;
    }
    return kCIDLib::True;
}

tCIDLib::TBoolean TZWUSB3CfgParamDlg::bGetValue(tCIDLib::TInt4& i4ToFill) const
{
    // This guy is an entry field, so we have to get the text and convert to a number
    TString strVal = m_pwndValue->strWndText();
    if (strVal.bIsEmpty())
    {
        TErrBox msgbErr(m_strTitleText, L"The value to set must be provided");
        msgbErr.ShowIt(*this);
        return kCIDLib::False;
    }

    tCIDLib::ERadices eRadix = eGetRadix();
    if (!strVal.bToInt4(i4ToFill, eRadix))
    {
        TErrBox msgbErr
        (
            m_strTitleText, L"The value must be a number legal for the selected radix"
        );
        msgbErr.ShowIt(*this);
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


//
//  A helper to get the selected parameter bytes size combo box value.
//
tCIDLib::TCard4 TZWUSB3CfgParamDlg::c4GetBytes() const
{
    // This one is read only so we just get the index and go from there
    const tCIDLib::TCard4 c4Index = m_pwndBytes->c4CurItem();

    tCIDLib::TCard4 c4Ret;
    if (c4Index == 0)
        c4Ret = 1;
    else if (c4Index == 1)
        c4Ret = 2;
    else if (c4Index == 2)
        c4Ret = 4;
    else
    {
        TErrBox msgbErr(m_strTitleText, L"Invalid parameter bytes combo box index");
        msgbErr.ShowIt(*this);
        return 0;
    }
    return c4Ret;
}

// Handle button presses
tCIDCtrls::EEvResponses TZWUSB3CfgParamDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kZWaveUSB3C::ridDlg_CfgParams_QueryValue)
    {
        QueryValue();
    }
     else if (wnotEvent.widSource() == kZWaveUSB3C::ridDlg_CfgParams_Close)
    {
        EndDlg(kZWaveUSB3C::ridDlg_CfgParams_Close);
    }
     else if (wnotEvent.widSource() == kZWaveUSB3C::ridDlg_CfgParams_SetValue)
    {
        SetValue();
    }
    return tCIDCtrls::EEvResponses::Handled;
}


// A helper to get the selected value of the radix combo
tCIDLib::ERadices TZWUSB3CfgParamDlg::eGetRadix() const
{
    tCIDLib::TCard4 c4Index = m_pwndRadix->c4CurItem();
    tCIDLib::ERadices eRet = tCIDLib::ERadices::Dec;
    if (c4Index == 0)
        eRet = tCIDLib::ERadices::Bin;
    else if (c4Index == 1)
        eRet = tCIDLib::ERadices::Dec;
    else if (c4Index == 2)
        eRet = tCIDLib::ERadices::Hex;
    else
    {
        TErrBox msgbErr(m_strTitleText, L"Invalid radix combo box index");
        msgbErr.ShowIt(*this);
    }
    return eRet;
}


// We update the displayed value if we see a change in the selected radix
tCIDCtrls::EEvResponses TZWUSB3CfgParamDlg::eLBHandler(TListChangeInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kZWaveUSB3C::ridDlg_CfgParams_Radix)
    {
         if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::SelChanged)
         {
             // Figure out the new radix
            tCIDLib::ERadices eOldRadix = m_eRadix;
             if (wnotEvent.c4Index() == 0)
                m_eRadix = tCIDLib::ERadices::Bin;
             else if (wnotEvent.c4Index() == 1)
                m_eRadix = tCIDLib::ERadices::Dec;
            else if (wnotEvent.c4Index() == 2)
                m_eRadix = tCIDLib::ERadices::Hex;
            else
            {
                TErrBox msgbErr(m_strTitleText, L"Unknown radix value");
                msgbErr.ShowIt(*this);
                return tCIDCtrls::EEvResponses::Handled;
            }

            // Get the current value out
            TString strVal = m_pwndValue->strWndText();
            if (!strVal.bIsEmpty())
            {
                // Convert back to a number using the old radix
                tCIDLib::TInt4 i4NewVal;
                if (strVal.bToInt4(i4NewVal, eOldRadix))
                {
                    // Convert it to the new radix and put it back
                    TString strVal;
                    strVal.SetFormatted(i4NewVal, m_eRadix);
                    m_pwndValue->strWndText(strVal);
                }
                 else
                {
                    TErrBox msgbErr
                    (
                        m_strTitleText
                        , L"The current value cannot be convered to a number in this new radix"
                    );
                    msgbErr.ShowIt(*this);
                }
            }
        }
    }

    return tCIDCtrls::EEvResponses::Handled;
}


//
//  When the user hits the query button this is called. If there is a valid parameter
//  number set, we query the value.
//
tCIDLib::TVoid TZWUSB3CfgParamDlg::QueryValue()
{
    //
    //  Check the awake situation. They may not mean to do this if the unit is
    //  not currently awake.
    //
    if (!bCheckAwake())
        return;

    // Get the parameter number
    tCIDLib::TCard1 c1PNum;
    if (!bGetParamNum(c1PNum))
        return;

    // And let's ask for the value
    TZWDriverXData zwdxdComm;
    zwdxdComm.m_strCmd = kZWaveUSB3Sh::strSendData_QueryCfgParam;
    zwdxdComm.m_c1UnitId = m_punitiTar->c1Id();
    zwdxdComm.m_c1Val1 = c1PNum;
    zwdxdComm.m_bAwake = m_pwndIsAwake->bCheckedState();
    THeapBuf mbufComm(64, 128);

    // Whether it works or not we clear the list, if only to reload it
    TWndPtrJanitor janWait(tCIDCtrls::ESysPtrs::Wait);
    if (m_pwndDriver->bSendSrvMsg(*this, zwdxdComm, mbufComm))
    {
        TString strVal;
        strVal.AppendFormatted(zwdxdComm.m_i4Val1);
        m_pwndValue->strWndText(strVal);
    }
     else
    {
        TErrBox msgbErr(m_strTitleText, zwdxdComm.m_strMsg);
        msgbErr.ShowIt(*this);
    }
}


//
//  When the user hits the set value button this is called. If there is a valid
//  parameter number, bytes and value set, we try to set the value.
//
tCIDLib::TVoid TZWUSB3CfgParamDlg::SetValue()
{
    //
    //  Check the awake situation. They may not mean to do this if the unit is
    //  not currently awake.
    //
    if (!bCheckAwake())
        return;

    // Get the parameter number
    tCIDLib::TCard1 c1PNum;
    if (!bGetParamNum(c1PNum))
        return;

    // Get the bytes
    tCIDLib::TCard4 c4Bytes = c4GetBytes();

    // And get the value
    tCIDLib::TInt4 i4Val;
    if (!bGetValue(i4Val))
        return;

    TZWDriverXData zwdxdComm;
    zwdxdComm.m_strCmd = kZWaveUSB3Sh::strSendData_SetCfgParam;
    zwdxdComm.m_c1UnitId = m_punitiTar->c1Id();
    zwdxdComm.m_c1Val1 = c1PNum;
    zwdxdComm.m_c1Val2 = tCIDLib::TCard1(c4Bytes);
    zwdxdComm.m_i4Val1 = i4Val;
    zwdxdComm.m_bAwake = m_pwndIsAwake->bCheckedState();
    THeapBuf mbufComm(64, 128);

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
            TOkBox msgbWarn
            (
                m_strTitleText
                , facZWaveUSB3C().strMsg(kZWU3CMsgs::midWarn_OpPending)
            );
            msgbWarn.ShowIt(*this);
        }
         else
        {
            TOkBox msgbDone
            (
                m_strTitleText
                , L"The operation appears to have completed successfully. You can query "
                  L"the values back to verify if you want to be sure."
            );
            msgbDone.ShowIt(*this);
        }
    }
     else
    {
        TErrBox msgbErr(m_strTitleText, zwdxdComm.m_strMsg);
        msgbErr.ShowIt(*this);
    }

}
