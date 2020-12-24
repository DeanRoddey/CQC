//
// FILE NAME: CQCKit_PowerStateMon.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/03/2014
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
//  Implements the power state mnonitor class.
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
#include    "CQCKit_.hpp"


// ---------------------------------------------------------------------------
//  Magic Macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCPwrStateMon,TObject)


// ---------------------------------------------------------------------------
//   CLASS: TCQCPwrStateMon
//  PREFIX: cpsm
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCPwrStateMon: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCPwrStateMon::TCQCPwrStateMon(const TString& strTitle) :

    m_eCurState(EStates::Idle)
    , m_eTarPwrState(tCQCKit::EPowerStatus::Count)
    , m_thrPoll
      (
          facCIDLib().strNextThreadName(TString(L"PwrStateMon") + strTitle)
          , TMemberFunc<TCQCPwrStateMon>(this, &TCQCPwrStateMon::ePollThread)
      )
    , m_strName(strTitle)
{
}

TCQCPwrStateMon::~TCQCPwrStateMon()
{
}


// ---------------------------------------------------------------------------
//  TCQCPwrStateMon: Public, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::EAsyncWaitRes TCQCPwrStateMon::eCheckResult()
{
    // If idle, then this shouldn't be called
    if (m_eCurState == EStates::Idle)
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcPwrStMon_NotReady
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotReady
            , m_strName
        );
    }

    // If complete, then
    if (m_eCurState == EStates::Complete)
        return tCIDLib::EAsyncWaitRes::Complete;

    if (m_eCurState == EStates::Timeout)
        return tCIDLib::EAsyncWaitRes::Timeout;

    // Still waiting
    return tCIDLib::EAsyncWaitRes::Waiting;
}


tCIDLib::TVoid TCQCPwrStateMon::Cleanup()
{
    // If we are idle, nothing to do
    if (m_eCurState == EStates::Idle)
        return;

    // Stop the thread if running
    if (m_thrPoll.bIsRunning())
    {
        try
        {
            m_thrPoll.ReqShutdownSync(5000);
            m_thrPoll.eWaitForDeath(2500);
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
    }

    // Clean up the driver list
    m_colDrivers.RemoveAll();
}


tCIDLib::TVoid
TCQCPwrStateMon::Start( const   tCIDLib::TKVPCollect&   colDrivers
                        , const tCIDLib::TBoolean       bNewState)
{
    if (m_eCurState != EStates::Idle)
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcPwrStMon_AlreadyStarted
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Already
            , m_strName
        );
    }

    // Set us up and start the thread
    m_colDrivers.RemoveAll();
    TColCursor<TKeyValuePair>* pcursDrvs = colDrivers.pcursNew();
    TJanitor<TColCursor<TKeyValuePair> > janCursor(pcursDrvs);

    if (pcursDrvs->bReset())
    {
        do
        {
            m_colDrivers.objAdd(pcursDrvs->objRCur());
        }   while(pcursDrvs->bNext());
    }
    m_thrPoll.Start();
}


// ---------------------------------------------------------------------------
//  TCQCPwrStateMon: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::EExitCodes
TCQCPwrStateMon::ePollThread(TThread& thrThis, tCIDLib::TVoid*)
{
    // Update the state before we let the caller go
    m_eCurState = EStates::Waiting;
    thrThis.Sync();


    return tCIDLib::EExitCodes::Normal;
}

