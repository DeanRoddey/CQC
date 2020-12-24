//
// FILE NAME: CQCIGKit_CfgRemPortDlg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/15/2005
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
//  This file implements the dialog that allows the user to configure access
//  to a CQC remote port server.
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
#include    "CQCIGKit_.hpp"
#include    "CQCIGKit_CfgRemPortDlg_.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TCfgRemPortDlg,TDlgBox)



// ---------------------------------------------------------------------------
//   CLASS: TCfgRemPortDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCfgRemPortDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TCfgRemPortDlg::TCfgRemPortDlg() :

    m_bEnable(kCIDLib::False)
    , m_pwndAddr(nullptr)
    , m_pwndCancel(nullptr)
    , m_pwndEnable(nullptr)
    , m_pwndPort(nullptr)
    , m_pwndSave(nullptr)
    , m_pwndTest(nullptr)
{
}

TCfgRemPortDlg::~TCfgRemPortDlg()
{
}


// ---------------------------------------------------------------------------
//  TCfgRemPortDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TCfgRemPortDlg::bRunDlg(const   TWindow&            wndOwner
                        ,       tCIDLib::TBoolean&  bEnable
                        ,       TIPEndPoint&        ipepInOut)
{
    // Save the incoming stuff for later use
    m_bEnable = bEnable;
    m_ipepCur = ipepInOut;

    const tCIDLib::TCard4 c4Res = c4RunDlg
    (
        wndOwner, facCQCIGKit(), kCQCIGKit::ridDlg_CfgRemPort
    );

    if (c4Res == kCQCIGKit::ridDlg_CfgRemPort_Save)
    {
        bEnable = m_bEnable;
        ipepInOut = m_ipepCur;
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


// ---------------------------------------------------------------------------
//  TCfgRemPortDlg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TCfgRemPortDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRes = TParent::bCreated();

    // First get typed pointers to some of the widgets
    CastChildWnd(*this, kCQCIGKit::ridDlg_CfgRemPort_Addr, m_pwndAddr);
    CastChildWnd(*this, kCQCIGKit::ridDlg_CfgRemPort_Cancel, m_pwndCancel);
    CastChildWnd(*this, kCQCIGKit::ridDlg_CfgRemPort_CloseAll, m_pwndCloseAll);
    CastChildWnd(*this, kCQCIGKit::ridDlg_CfgRemPort_Enable, m_pwndEnable);
    CastChildWnd(*this, kCQCIGKit::ridDlg_CfgRemPort_Port, m_pwndPort);
    CastChildWnd(*this, kCQCIGKit::ridDlg_CfgRemPort_Save, m_pwndSave);
    CastChildWnd(*this, kCQCIGKit::ridDlg_CfgRemPort_Test, m_pwndTest);

    // Register event handlers
    m_pwndCancel->pnothRegisterHandler(this, &TCfgRemPortDlg::eClickHandler);
    m_pwndCloseAll->pnothRegisterHandler(this, &TCfgRemPortDlg::eClickHandler);
    m_pwndEnable->pnothRegisterHandler(this, &TCfgRemPortDlg::eClickHandler);
    m_pwndSave->pnothRegisterHandler(this, &TCfgRemPortDlg::eClickHandler);
    m_pwndTest->pnothRegisterHandler(this, &TCfgRemPortDlg::eClickHandler);

    // Load up the incoming stuff
    TString strFmt;
    strFmt.SetFormatted(m_ipepCur.ippnThis());
    m_pwndPort->strWndText(strFmt);
    m_pwndAddr->strWndText(m_ipepCur.strAsText());

    m_pwndEnable->SetCheckedState(m_bEnable);

    return bRes;
}


// ---------------------------------------------------------------------------
//  TCfgRemPortDlg: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  This is called on save, to validate the current command info, so that we
//  can warn the user of potential problems. It's also called from teh Test button,
//  and we get a parameter indicating which of these scenarios. If testing, we
//  do a positive dialog box. Else, we only do a failure one.
//
tCIDLib::TBoolean TCfgRemPortDlg::bValidate(const tCIDLib::TBoolean bTest)
{
    const TString& strAddr = m_pwndAddr->strWndText();
    const TString& strPort = m_pwndPort->strWndText();

    // Try to convert the port to a number
    tCIDLib::TIPPortNum ippnNew;
    try
    {
        ippnNew = strPort.c4Val(tCIDLib::ERadices::Dec);
    }

    catch(...)
    {
        ippnNew = 0;
    }

    if (!ippnNew || (ippnNew > 65535))
    {
        TErrBox msbgTest(facCQCIGKit().strMsg(kIGKitErrs::errcRemPort_BadPort));
        msbgTest.ShowIt(*this);
        return kCIDLib::False;
    }

    // See if we can do a lookup on the address
    TIPAddress ipaNew;
    if (!facCIDSock().bResolveAddr(strAddr, ipaNew, tCIDSock::EAddrTypes::Unspec))
    {
        TErrBox msbgTest(facCQCIGKit().strMsg(kIGKitErrs::errcRemPort_BadAddr));
        msbgTest.ShowIt(*this);
        return kCIDLib::False;
    }

    try
    {
        TIPEndPoint ipepNew(ipaNew, ippnNew);

        tRPortClient::TSerialPortClient orbcTest = facCQCRPortClient().orbcMakePortProxy(ipepNew);

        //
        //  Just do some operation to make sure it's happy. We'll ask
        //  it for the port list.
        //
        TVector<TString> colPorts;
        orbcTest->bQueryCommPorts(colPorts);

        // If testing, positively acknowledge it worked
        if (bTest)
        {
            TOkBox msgbPassed(facCQCIGKit().strMsg(kIGKitMsgs::midDlg_CfgRemPort_Passed));
            msgbPassed.ShowIt(*this);
        }

        m_ipepCur = ipepNew;
    }

    catch(const TError& errToCatch)
    {
        TExceptDlg dlgErr
        (
            *this
            , strWndText()
            , facCQCIGKit().strMsg(kIGKitErrs::errcRemPort_NoServer)
            , errToCatch
        );
        return kCIDLib::False;
    }

    // Store the new end point
    return kCIDLib::True;
}


tCIDCtrls::EEvResponses TCfgRemPortDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCIGKit::ridDlg_CfgRemPort_Cancel)
    {
        EndDlg(kCQCIGKit::ridDlg_CfgRemPort_Cancel);
    }
     else if (wnotEvent.widSource() == kCQCIGKit::ridDlg_CfgRemPort_CloseAll)
    {
        if (bValidate(kCIDLib::False))
        {
            try
            {
                tRPortClient::TSerialPortClient orbcTest
                                    = facCQCRPortClient().orbcMakePortProxy(m_ipepCur);
                orbcTest->CloseAllPorts();
                TOkBox msgbPassed(facCQCIGKit().strMsg(kIGKitMsgs::midDlg_CfgRemPort_Closed));
                msgbPassed.ShowIt(*this);
            }

            catch(...)
            {
                TErrBox msgbTest(facCQCIGKit().strMsg(kIGKitErrs::errcRemPort_AllClose));
                msgbTest.ShowIt(*this);
            }
        }
    }
     else if (wnotEvent.widSource() == kCQCIGKit::ridDlg_CfgRemPort_Enable)
    {
        m_bEnable = m_pwndEnable->bCheckedState();
    }
     else if (wnotEvent.widSource() == kCQCIGKit::ridDlg_CfgRemPort_Save)
    {
        if (bValidate(kCIDLib::False))
            EndDlg(kCQCIGKit::ridDlg_CfgRemPort_Save);
    }
     else if (wnotEvent.widSource() == kCQCIGKit::ridDlg_CfgRemPort_Test)
    {
        bValidate(kCIDLib::True);
    }
    return tCIDCtrls::EEvResponses::Handled;
}


