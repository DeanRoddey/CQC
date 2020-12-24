//
// FILE NAME: CQCIR_ActionsThread.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/17/2003
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
//  This file implements a thread used by all the IR receiver server side
//  drivers.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCIR_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCIRActionsThread,TThread)


// ---------------------------------------------------------------------------
//  Our little queue class to store data for processing
// ---------------------------------------------------------------------------

TCQCIRActInfo::TCQCIRActInfo(const  TCQCStdKeyedCmdSrc& csrcAct
                            , const TString&            strActKey
                            , const TString&            strActData) :

    m_csrcAct(csrcAct)
    , m_strActData(strActData)
    , m_strActKey(strActKey)
{
}

TCQCIRActInfo::~TCQCIRActInfo()
{
}




// ---------------------------------------------------------------------------
//   CLASS: TCQCIRActionsThread
//  PREFIX: irrc
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIRActionsThread: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIRActionsThread::TCQCIRActionsThread(const  TString&        strMoniker
                                        , const TCQCUserCtx&    cuctxToUse) :

    TThread(TString(L"CQCIRActThread_") + strMoniker)
    , m_bVerbose(kCIDLib::False)
    , m_colEvents(tCIDLib::EAdoptOpts::Adopt, tCIDLib::EMTStates::Safe)
    , m_cuctxToUse(cuctxToUse)
    , m_strMoniker(strMoniker)
{
}


TCQCIRActionsThread::~TCQCIRActionsThread()
{
}


// ---------------------------------------------------------------------------
//  TCQCIRActionsThread: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Tells the caller if our queue is empty or not
tCIDLib::TBoolean TCQCIRActionsThread::bIsEmpty() const
{
    return m_colEvents.bIsEmpty();
}


// Get/set our verbose logging mode
tCIDLib::TBoolean TCQCIRActionsThread::bVerboseMode() const
{
    return m_bVerbose;
}

tCIDLib::TBoolean
TCQCIRActionsThread::bVerboseMode(const tCIDLib::TBoolean bToSet)
{
    m_bVerbose = bToSet;
    return m_bVerbose;
}


// Clears out the queue of events to process
tCIDLib::TVoid TCQCIRActionsThread::FlushQueue()
{
    m_colEvents.RemoveAll();
}


// Queues up a new event to process
tCIDLib::TVoid
TCQCIRActionsThread::QueueEvent(const   TCQCStdKeyedCmdSrc& csrcToQueue
                                , const TString&            strActKey
                                , const TString&            strActData)
{
    // If the list has gotten big, ignore it
    if (m_colEvents.bIsFull(32))
    {
        facCQCIR().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kIRErrs::errcEv_QueueFull
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::OutResource
            , m_strMoniker
            , csrcToQueue.strTitle()
        );
        return;
    }

    // Add it to the deque at the bottom
    try
    {
        m_colEvents.Put(new TCQCIRActInfo(csrcToQueue, strActKey, strActData));

        // If in verbose mode, log that we queued this event
        if (m_bVerbose)
        {
            facCQCIR().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kIRMsgs::midStatus_EvQueued
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
                , m_strMoniker
                , csrcToQueue.strTitle()
            );
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}



// ---------------------------------------------------------------------------
//  TCQCIRActionsThread: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::EExitCodes TCQCIRActionsThread::eProcess()
{
    // Let the calling thread go
    Sync();

    //
    //  Create a standard command processing engine. We don't need anything
    //  here but the standard targets.
    //
    TCQCStdActEngine acteIR(m_cuctxToUse);

    //
    //  We don't have any optional extra targets, so we just pass in
    //  an empty list.
    //
    TRefVector<MCQCCmdTarIntf> colExtraTars(tCIDLib::EAdoptOpts::NoAdopt, 1);

    //
    //  Loop until we are told to exit. On each round, wait a bit for an
    //  event to show up in the queue. The queue is thread safe so we don't
    //  have to do any sync.
    //
    tCIDLib::TBoolean   bDone = kCIDLib::False;
    TCQCIRActInfo*      piraiInvoke = nullptr;
    TString             strInvokeParms;
    TString             strTitle;
    while (!bDone)
    {
        try
        {
            // Check for a shutdown request and get out if so
            if (bCheckShutdownRequest())
                break;

            //
            //  Check for an event to have arrived. We push them onto the bottom,
            //  so we get them off the top. We wait for a bit for one to show up,
            //  which provides our throttling. Tell it not to throw if nothing
            //  available.
            //
            piraiInvoke = m_colEvents.pobjGetNext(100, kCIDLib::False);
            if (!piraiInvoke)
                continue;

            // Put a janitor on the object to insure it's cleaned up
            TJanitor<TCQCIRActInfo> janAct(piraiInvoke);

            //
            //  Get a ref to the action for convenience below, and a copy of the
            //  action title, for exceptions below to use in logging.
            //
            TCQCStdKeyedCmdSrc& csrcInvoke = piraiInvoke->m_csrcAct;
            strTitle = csrcInvoke.strTitle();

            // If in verbose mode, log that we are processing this event
            if (m_bVerbose)
            {
                facCQCIR().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kIRMsgs::midStatus_InvokingEv
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::AppStatus
                    , m_strMoniker
                    , piraiInvoke->m_csrcAct.strTitle()
                );
            }

            //
            //  Set up any action parameters for the action we are going to run.
            //  There will be at least one, possibly two. The first is the name
            //  of the action (the short descriptive text configured by the user),
            //  and the second is any part of the incoming key that was after a
            //  configured separator character. The part before that separator is
            //  used to match against the key, and the part after is stuff to be
            //  passed to the action. If there is no separator, or the separator
            //  is the last character, then the second one is empty.
            //
            //  The code that handles the matching and queing up of actions will
            //  break the trailing bit out and queue it up with the action.
            //
            strInvokeParms.Clear();
            TStringTokenizer::BuildQuotedCommaList(strTitle, strInvokeParms);
            TStringTokenizer::BuildQuotedCommaList(piraiInvoke->m_strActData, strInvokeParms);

            const tCQCKit::ECmdRes eRes = acteIR.eInvokeOps
            (
                csrcInvoke
                , kCQCKit::strEvId_OnPress
                , facCQCIR().ctarGVars()
                , colExtraTars
                , 0
                , strInvokeParms
            );

            //
            //  If it terminated early, then log that. Else, if in verbose
            //  mode, log that it ended normally.
            //
            if (eRes == tCQCKit::ECmdRes::Stop)
            {
                facCQCIR().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kIRErrs::errcEv_Exited
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::AppStatus
                    , csrcInvoke.strTitle()
                    , m_strMoniker
                );
            }
             else if (m_bVerbose)
            {
                facCQCIR().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kIRMsgs::midStatus_EvEnded
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::AppStatus
                    , m_strMoniker
                    , csrcInvoke.strTitle()
                );
            }
        }

        catch(TError& errToCatch)
        {
            if (!errToCatch.bLogged())
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }

            facCQCIR().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kIRMsgs::midStatus_ExceptInAction
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
                , strTitle
                , m_strMoniker
            );
        }

        catch(...)
        {
            facCQCIR().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kIRMsgs::midStatus_ExceptInAction
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
                , strTitle
                , m_strMoniker
            );
        }

        // Clear stuff out for the next round
        strTitle.Clear();
        piraiInvoke = nullptr;
    }
    return tCIDLib::EExitCodes::Normal;
}



