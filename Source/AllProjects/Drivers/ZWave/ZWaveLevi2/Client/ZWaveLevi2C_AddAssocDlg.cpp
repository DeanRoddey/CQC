//
// FILE NAME: ZWaveLevi2C_AddAssocDlg.cpp
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
//  This file implements a small dialog that lets the user set inter-unit
//  associations.
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
RTTIDecls(TZWaveAddAssocDlg,TDlgBox)



// ---------------------------------------------------------------------------
//   CLASS: TZWaveAddAssocDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWaveAddAssocDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TZWaveAddAssocDlg::TZWaveAddAssocDlg() :

    m_c4VRCOPId(0)
    , m_punitSrc(nullptr)
    , m_pwndClose(nullptr)
    , m_pwndDriver(nullptr)
    , m_pwndGrpNum(nullptr)
{
}

TZWaveAddAssocDlg::~TZWaveAddAssocDlg()
{
}


// ---------------------------------------------------------------------------
//  TZWaveAddAssocDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TZWaveAddAssocDlg::RunDlg(          TZWaveLevi2CWnd&    wndOwner
                            , const TString&            strMoniker
                            , const TZWaveUnit&         unitSrc
                            , const tCIDLib::TCard4     c4VRCOPId)
{
    m_c4VRCOPId = c4VRCOPId;
    m_punitSrc = &unitSrc;
    m_pwndDriver = &wndOwner;
    m_strMoniker = strMoniker;

    c4RunDlg(wndOwner, facZWaveLevi2C(), kZWaveLevi2C::ridDlg_AddAssoc);
}


// ---------------------------------------------------------------------------
//  TZWaveAddAssocDlg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TZWaveAddAssocDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRet = TParent::bCreated();

    // Store our pre-typed child widget pointers
    CastChildWnd(*this, kZWaveLevi2C::ridDlg_AddAssoc_Set, m_pwndSet);
    CastChildWnd(*this, kZWaveLevi2C::ridDlg_AddAssoc_Close, m_pwndClose);
    CastChildWnd(*this, kZWaveLevi2C::ridDlg_AddAssoc_GrpNum, m_pwndGrpNum);

    // And register our button event handlers
    m_pwndSet->pnothRegisterHandler(this, &TZWaveAddAssocDlg::eClickHandler);
    m_pwndClose->pnothRegisterHandler(this, &TZWaveAddAssocDlg::eClickHandler);

    // Set up the group # spin box
    m_pwndGrpNum->SetNumRange(1, 255, 1);

    return bRet;
}



// ---------------------------------------------------------------------------
//  TZWaveAddAssocDlg: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDCtrls::EEvResponses
TZWaveAddAssocDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kZWaveLevi2C::ridDlg_AddAssoc_Close)
    {
        EndDlg(kZWaveLevi2C::ridDlg_AddAssoc_Close);
    }
     else if (wnotEvent.widSource() == kZWaveLevi2C::ridDlg_AddAssoc_Set)
    {
        try
        {
            tCQCKit::TCQCSrvProxy orbcTarget;

            // Try to get a client proxy
            TWndPtrJanitor janPtr(tCIDCtrls::ESysPtrs::Wait);
            orbcTarget = facCQCKit().orbcCQCSrvAdminProxy(m_strMoniker);

            // We got it, so get out the values to send and format the command
            const tCIDLib::TCard4 c4GrpNum = tCIDLib::TCard4(m_pwndGrpNum->i4CurValue() + 1);

            TString strCmd = kZWaveLevi2Sh::strDrvCmd_AssocToVRCOP;
            strCmd.Append(kCIDLib::chSpace);
            strCmd.Append(m_punitSrc->strName());
            strCmd.Append(kCIDLib::chSpace);
            strCmd.AppendFormatted(c4GrpNum);

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


