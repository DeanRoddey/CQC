//
// FILE NAME: CQCIGKit_WaitCQCDlg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/05/2004
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
//  This file implements a dialog that is used by client apps to wait for
//  the back end services to come up.
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


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TWaitCQCDlg, TDlgBox)



// ---------------------------------------------------------------------------
//   CLASS: TWaitCQCDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TWaitCQCDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TWaitCQCDlg::TWaitCQCDlg() :

    m_enctEnd(0)
    , m_pwndCancelButton(nullptr)
    , m_thrWaiter
      (
        TString(L"CQCGIKitWaitCQCThread")
        , TMemberFunc<TWaitCQCDlg>(this, &TWaitCQCDlg::eRunThread)
      )
{
}

TWaitCQCDlg::~TWaitCQCDlg()
{
}


// ---------------------------------------------------------------------------
//  TWaitCQCDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TWaitCQCDlg::bRunDlg(const  TWindow&        wndOwner
                    , const TString&        strTitle
                    , const TString&        strBinding
                    , const tCIDLib::TCard4 c4WaitFor)
{
    // Store parms so that we can use them later
    m_enctEnd = TTime::enctNowPlusMSs(c4WaitFor);
    m_strBinding = strBinding;
    m_strTitle = strTitle;

    const tCIDLib::TCard4 c4Res = c4RunDlg
    (
        wndOwner, facCQCIGKit(), kCQCIGKit::ridDlg_WaitCQC
    );

    // Make sure that the thread is all the way down
    try
    {
        if (m_thrWaiter.bIsRunning())
        {
            TWndPtrJanitor janPtr(tCIDCtrls::ESysPtrs::Wait);
            m_thrWaiter.ReqShutdownSync(10000);
            m_thrWaiter.eWaitForDeath(5000);
        }
    }

    catch(const TError& errToCatch)
    {
        TExceptDlg dlgErr
        (
            *this
            , m_strTitle
            , facCQCIGKit().strMsg(kIGKitMsgs::midDlg_WaitCQC_ThreadStop)
            , errToCatch
        );
    }
    return (c4Res && (c4Res != kCIDLib::c4MaxCard));
}


// ---------------------------------------------------------------------------
//  TWaitCQCDlg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TWaitCQCDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRet = TParent::bCreated();

    // First get typed pointers to the widget we always keep
    CastChildWnd(*this, kCQCIGKit::ridDlg_WaitCQC_Cancel, m_pwndCancelButton);

    // Register our handlers
    m_pwndCancelButton->pnothRegisterHandler(this, &TWaitCQCDlg::eClickHandler);

    // Set the title we were given
    strWndText(m_strTitle);

    // Start up the waiter thread
    m_thrWaiter.Start();

    //
    //  Make sure we come to the top. There is a wait before the dialog pops
    //  up and they might run something else.
    //
    ForceToFront();

    return bRet;
}


tCIDLib::TVoid
TWaitCQCDlg::CodeReceived(const tCIDLib::TInt4 i4Code, const tCIDLib::TCard4)
{
    if (i4Code)
        EndDlg(1);
    else
        EndDlg(0);
}


// ---------------------------------------------------------------------------
//  TWaitCQCDlg: Private, non-virutal methods
// ---------------------------------------------------------------------------
tCIDCtrls::EEvResponses
TWaitCQCDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCIGKit::ridDlg_WaitCQC_Cancel)
    {
        try
        {
            if (m_thrWaiter.bIsRunning())
            {
                TWndPtrJanitor janPtr(tCIDCtrls::ESysPtrs::Wait);
                m_thrWaiter.ReqShutdownSync(10000);
                m_thrWaiter.eWaitForDeath(5000);
            }
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TExceptDlg dlgErr
            (
                *this
                , m_strTitle
                , facCQCIGKit().strMsg(kIGKitMsgs::midDlg_WaitCQC_ThreadStop)
                , errToCatch
            );
        }
        EndDlg(kCQCIGKit::ridDlg_WaitCQC_Cancel);
    }
    return tCIDCtrls::EEvResponses::Handled;
}


tCIDLib::EExitCodes TWaitCQCDlg::eRunThread(TThread& thrThis, tCIDLib::TVoid*)
{
    // Let the parent thread go
    thrThis.Sync();

    TThread* pthrCaller = nullptr;
    tCIDLib::TEncodedTime enctCur = TTime::enctNow();
    while (enctCur < m_enctEnd)
    {
        // Check for a shutdown request. If so, shutdown without any post
        if (thrThis.bCheckShutdownRequest())
            return tCIDLib::EExitCodes::Normal;

        TString strName;
        TString strDescr;
        try
        {
            tCIDOrbUC::TNSrvProxy orbcNS = facCIDOrbUC().orbcNameSrvProxy(2000);
            orbcNS->QueryNameInfo(m_strBinding, strName, strDescr);
            SendCode(1, 0);
            return tCIDLib::EExitCodes::Normal;
        }

        catch(...)
        {
            //
            //  If it fails, sleep a bit since we won't have necessarily gotten the
            //  required throttling above in the wait to connect.
            //
            if (!pthrCaller)
                pthrCaller = TThread::pthrCaller();

            if (!pthrCaller->bSleep(2000))
                    break;
        }
        enctCur = TTime::enctNow();
    }
    SendCode(0, 0);
    return tCIDLib::EExitCodes::Normal;
}



