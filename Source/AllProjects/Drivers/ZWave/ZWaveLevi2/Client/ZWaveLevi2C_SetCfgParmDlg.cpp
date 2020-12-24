//
// FILE NAME: ZWaveLevi2C_SetCfgParamDlg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/21/2014
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
//  This file implements a small dialog to get info for a set configuration
//  parameter call to a unit.
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
#include    "ZWaveLevi2C_.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TZWaveSetCfgParamDlg,TDlgBox)


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace ZWaveLevi2C_SetCfgParmDlg
{
    tCIDLib::TCard4 c4LastBytes = 1;
    tCIDLib::TCard4 c4LastNum = 1;
}



// ---------------------------------------------------------------------------
//   CLASS: TZWaveSetCfgParamDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWaveSetCfgParamDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TZWaveSetCfgParamDlg::TZWaveSetCfgParamDlg() :

    m_pdcfgUnits(nullptr)
    , m_pwndClose(nullptr)
    , m_pwndDriver(nullptr)
    , m_pwndPBytes(nullptr)
    , m_pwndPNum(nullptr)
    , m_pwndSet(nullptr)
    , m_pwndPVal(nullptr)
    , m_pwndTarUnit(nullptr)
{
}

TZWaveSetCfgParamDlg::~TZWaveSetCfgParamDlg()
{
}


// ---------------------------------------------------------------------------
//  TZWaveSetCfgParamDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TZWaveSetCfgParamDlg::RunDlg(       TZWaveLevi2CWnd&    wndOwner
                            , const TString&            strMoniker
                            , const TZWaveDrvConfig&    dcfgUnits)
{
    // Save a pointer to the configuration info for later
    m_pdcfgUnits = &dcfgUnits;
    m_pwndDriver = &wndOwner;
    m_strMoniker = strMoniker;

    c4RunDlg(wndOwner, facZWaveLevi2C(), kZWaveLevi2C::ridDlg_CfgParm);
}


// ---------------------------------------------------------------------------
//  TZWaveSetCfgParamDlg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TZWaveSetCfgParamDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRet = TParent::bCreated();

    // Store our pre-typed child widget pointers
    CastChildWnd(*this, kZWaveLevi2C::ridDlg_CfgParm_Set, m_pwndSet);
    CastChildWnd(*this, kZWaveLevi2C::ridDlg_CfgParm_Close, m_pwndClose);
    CastChildWnd(*this, kZWaveLevi2C::ridDlg_CfgParm_PBytes, m_pwndPBytes);
    CastChildWnd(*this, kZWaveLevi2C::ridDlg_CfgParm_PNum, m_pwndPNum);
    CastChildWnd(*this, kZWaveLevi2C::ridDlg_CfgParm_PVal, m_pwndPVal);
    CastChildWnd(*this, kZWaveLevi2C::ridDlg_CfgParm_Query, m_pwndQuery);
    CastChildWnd(*this, kZWaveLevi2C::ridDlg_CfgParm_TarUnit, m_pwndTarUnit);

    // And register our button event handlers
    m_pwndClose->pnothRegisterHandler(this, &TZWaveSetCfgParamDlg::eClickHandler);
    m_pwndQuery->pnothRegisterHandler(this, &TZWaveSetCfgParamDlg::eClickHandler);
    m_pwndSet->pnothRegisterHandler(this, &TZWaveSetCfgParamDlg::eClickHandler);

    // Set up the parameter # spin box and select the last one
    m_pwndPNum->SetNumRange(1, 255, ZWaveLevi2C_SetCfgParmDlg::c4LastNum);

    // Set up the parameter bytes spin box
    m_pwndPBytes->SetNumRange(1, 4, ZWaveLevi2C_SetCfgParmDlg::c4LastBytes);

    // Load up the available units list box
    const tCIDLib::TCard4 c4UnitCnt = m_pdcfgUnits->c4UnitCnt();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4UnitCnt; c4Index++)
        m_pwndTarUnit->c4AddItem(m_pdcfgUnits->unitAt(c4Index).strName(), c4Index);

    if (c4UnitCnt)
        m_pwndTarUnit->SelectByIndex(0);

    // Set an initial value into the value widget
    m_pwndPVal->strWndText(L"0");

    return bRet;
}



// ---------------------------------------------------------------------------
//  TZWaveSetCfgParamDlg: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDCtrls::EEvResponses TZWaveSetCfgParamDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kZWaveLevi2C::ridDlg_CfgParm_Close)
    {
        EndDlg(kZWaveLevi2C::ridDlg_CfgParm_Close);
    }
     else if (wnotEvent.widSource() == kZWaveLevi2C::ridDlg_CfgParm_Query)
    {
        // Get the target unit
        TString strUnit;
        if (!m_pwndTarUnit->bQueryCurVal(strUnit))
            return tCIDCtrls::EEvResponses::Handled;

        const TZWaveUnit& unitCur = m_pdcfgUnits->unitFindByName(strUnit);

        // If a wakeup type, then make sure they know it needs to be awake
        if (unitCur.bImplementsClass(tZWaveLevi2Sh::ECClasses::WakeUp))
        {
            TYesNoBox msgbWakeup
            (
                facZWaveLevi2C().strMsg(kZW2CMsgs::midGen_Title)
                , facZWaveLevi2C().strMsg(kZW2CMsgs::midQ_WakeupType)
            );

            if (msgbWakeup.bShowIt(*this))
                return tCIDCtrls::EEvResponses::Handled;
        }

        //
        //  Get the current value of the selected unit/parameter, and load the result
        //  into the value widget.
        //
        try
        {
            tCQCKit::TCQCSrvProxy orbcTarget;

            // Try to get a client proxy
            TWndPtrJanitor janPtr(tCIDCtrls::ESysPtrs::Wait);
            orbcTarget = facCQCKit().orbcCQCSrvAdminProxy(m_strMoniker);

            // We got it, so get out the values to send and format the command
            const tCIDLib::TCard4 c4PNum = tCIDLib::TCard4(m_pwndPNum->i4CurValue() + 1);

            // Store it as the last number
            ZWaveLevi2C_SetCfgParmDlg::c4LastNum = c4PNum;

            TString strValName;
            strValName.Append(unitCur.strName());
            strValName.Append(kCIDLib::chForwardSlash);
            strValName.AppendFormatted(c4PNum);

            tCIDLib::TCard4 c4Val = orbcTarget->c4QueryVal2
            (
                m_strMoniker, kZWaveLevi2Sh::strQCardV_CfgParam, strValName
            );

            TString strVal;
            strVal.AppendFormatted(c4Val);
            m_pwndPVal->strWndText(strVal);
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);

            TExceptDlg dlgbFail
            (
                *this
                , strWndText()
                , facZWaveLevi2C().strMsg(kZW2CMsgs::midStatus_RemOpFailed)
                , errToCatch
            );
        }
    }
     else if (wnotEvent.widSource() == kZWaveLevi2C::ridDlg_CfgParm_Set)
    {
        try
        {
            tCQCKit::TCQCSrvProxy orbcTarget;

            // Try to get a client proxy
            TWndPtrJanitor janPtr(tCIDCtrls::ESysPtrs::Wait);
            orbcTarget = facCQCKit().orbcCQCSrvAdminProxy(m_strMoniker);

            // We got it, so get out the values to send and format the command
            const tCIDLib::TCard4 c4PNum = tCIDLib::TCard4(m_pwndPNum->i4CurValue());
            const tCIDLib::TCard4 c4PBytes = tCIDLib::TCard4(m_pwndPBytes->i4CurValue());

            // Store them as the last values
            ZWaveLevi2C_SetCfgParmDlg::c4LastBytes = c4PBytes;
            ZWaveLevi2C_SetCfgParmDlg::c4LastNum = c4PNum;

            // Get the target unit
            const TZWaveUnit& unitCur = m_pdcfgUnits->unitAt(m_pwndTarUnit->c4CurItemId());

            TString strCmd = kZWaveLevi2Sh::strDrvCmd_SetCfgParam;
            strCmd.Append(kCIDLib::chSpace);
            strCmd.Append(unitCur.strName());
            strCmd.Append(kCIDLib::chSpace);
            strCmd.AppendFormatted(c4PNum);
            strCmd.Append(kCIDLib::chSpace);
            strCmd.Append(m_pwndPVal->strWndText());
            strCmd.Append(kCIDLib::chSpace);
            strCmd.AppendFormatted(c4PBytes);

            // Seems ok, so write it to the command field
            orbcTarget->WriteStringByName
            (
                m_strMoniker
                , kZWaveLevi2Sh::strFld_InvokeCmd
                , strCmd
                , m_pwndDriver->sectUser()
                , tCQCKit::EDrvCmdWaits::DontCare
            );

            // No exception, so report it appeared to work
            TOkBox msgbOK
            (
                facZWaveLevi2C().strMsg(kZW2CMsgs::midGen_Title)
                , facZWaveLevi2C().strMsg(kZW2CMsgs::midStatus_CmdOpOK)
            );
            msgbOK.ShowIt(*this);
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);

            TExceptDlg dlgbFail
            (
                *this
                , strWndText()
                , facZWaveLevi2C().strMsg(kZW2CMsgs::midStatus_RemOpFailed)
                , errToCatch
            );
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}


