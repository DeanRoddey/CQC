//
// FILE NAME: ZWaveLeviC_SetCfgParamDlg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/14/2012
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
#include    "ZWaveLeviC_.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TZWaveSetCfgParamDlg,TDlgBox)


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace ZWaveLeviC_SetCfgParmDlg
{
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
    , m_pwndPNum(nullptr)
    , m_pwndSet(nullptr)
    , m_pwndPVals(nullptr)
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
TZWaveSetCfgParamDlg::Run(  const   TWindow&            wndOwner
                            , const TString&            strMoniker
                            , const TZWaveDrvConfig&    dcfgUnits)
{
    // Save a pointer to the configuration info for later
    m_pdcfgUnits = &dcfgUnits;
    m_strMoniker = strMoniker;

    c4RunDlg(wndOwner, facZWaveLeviC(), kZWaveLeviC::ridDlg_CfgParm);
}


// ---------------------------------------------------------------------------
//  TZWaveSetCfgParamDlg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TZWaveSetCfgParamDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRet = TParent::bCreated();

    // Store our pre-typed child widget pointers
    CastChildWnd(*this, kZWaveLeviC::ridDlg_CfgParm_Set, m_pwndSet);
    CastChildWnd(*this, kZWaveLeviC::ridDlg_CfgParm_Close, m_pwndClose);
    CastChildWnd(*this, kZWaveLeviC::ridDlg_CfgParm_PNum, m_pwndPNum);
    CastChildWnd(*this, kZWaveLeviC::ridDlg_CfgParm_PVals, m_pwndPVals);
    CastChildWnd(*this, kZWaveLeviC::ridDlg_CfgParm_TarUnit, m_pwndTarUnit);

    // And register our button event handlers
    m_pwndSet->pnothRegisterHandler(this, &TZWaveSetCfgParamDlg::eClickHandler);
    m_pwndClose->pnothRegisterHandler(this, &TZWaveSetCfgParamDlg::eClickHandler);

    // Set up the parameter # spin box
    m_pwndPNum->SetNumRange(0, 254, ZWaveLeviC_SetCfgParmDlg::c4LastNum);

    // Load up the available units list box
    const tCIDLib::TCard4 c4UnitCnt = m_pdcfgUnits->c4UnitCnt();
    if (c4UnitCnt)
    {
        tCIDLib::TStrList colVals;
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4UnitCnt; c4Index++)
            colVals.objAdd(m_pdcfgUnits->unitAt(c4Index).strName());
        m_pwndTarUnit->SetEnumList(colVals);
    }

    return kCIDLib::True;
}



// ---------------------------------------------------------------------------
//  TZWaveSetCfgParamDlg: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDCtrls::EEvResponses TZWaveSetCfgParamDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kZWaveLeviC::ridDlg_CfgParm_Close)
    {
        EndDlg(kZWaveLeviC::ridDlg_CfgParm_Close);
    }
     else if (wnotEvent.widSource() == kZWaveLeviC::ridDlg_CfgParm_Set)
    {
        try
        {
            tCQCKit::TCQCSrvProxy orbcTarget;

            // Try to get a client proxy
            TWndPtrJanitor janPtr(tCIDCtrls::ESysPtrs::Wait);
            orbcTarget = facCQCKit().orbcCQCSrvAdminProxy(m_strMoniker);

            // We got it, so get out the values to send and format the command
            const tCIDLib::TCard4 c4PNum = tCIDLib::TCard4(m_pwndPNum->i4CurValue());

            TString strTar;
            m_pwndTarUnit->QueryCurText(strTar);

            TString strCmd = kZWaveLeviSh::strDrvCmd_SetCfgParam;
            strCmd.Append(kCIDLib::chSpace);
            strCmd.Append(strTar);
            strCmd.Append(kCIDLib::chSpace);
            strCmd.AppendFormatted(c4PNum);

            //
            //  Break out the values and verify them. Each one must be a byte.
            //  They are space separated, so we just use a tokenizer, which
            //  will remove white space for us as we go as well.
            //
            TString strVals = m_pwndPVals->strWndText();
            TStringTokenizer stokVals(&strVals, L" \t\r\n");
            TString strVal;
            tCIDLib::TCard4 c4TokenCnt = 0;
            while (stokVals.bGetNextToken(strVal))
            {
                tCIDLib::TCard4 c4Val;
                if (!strVal.bToCard4(c4Val, tCIDLib::ERadices::Auto))
                {
                    TErrBox msgbErr
                    (
                        facZWaveLeviC().strMsg(kZWCMsgs::midGen_Title)
                        , facZWaveLeviC().strMsg(kZWCErrs::errcCfg_BadCfgParmVal)
                    );
                    msgbErr.ShowIt(*this);
                    return tCIDCtrls::EEvResponses::Handled;
                }
                strCmd.Append(kCIDLib::chSpace);
                strCmd.Append(strVal);
                c4TokenCnt++;
            }

            // We have to have gotten one value at least
            if (!c4TokenCnt)
            {
                TErrBox msgbErr
                (
                    facZWaveLeviC().strMsg(kZWCMsgs::midGen_Title)
                    , facZWaveLeviC().strMsg(kZWCErrs::errcCfg_MinCfgParmVal)
                );
                msgbErr.ShowIt(*this);
                return tCIDCtrls::EEvResponses::Handled;
            }

            // Seems ok, so write it to the command field
            orbcTarget->WriteStringByName
            (
                m_strMoniker
                , kZWaveLeviSh::strFld_Command
                , strCmd
                , tCQCKit::EDrvCmdWaits::DontCare
            );

            // No exception, so report it appeared to work
            TOkBox msgbOK
            (
                facZWaveLeviC().strMsg(kZWCMsgs::midGen_Title)
                , facZWaveLeviC().strMsg(kZWCMsgs::midStatus_CmdOpOK)
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
                , facZWaveLeviC().strMsg(kZWCMsgs::midStatus_RemOpFailed)
                , errToCatch
            );
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}


