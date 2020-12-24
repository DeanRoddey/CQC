//
// FILE NAME: CQCIR_ActionsThread.hpp
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
//  This is the header file for the CQCIR_ActionsThread.cpp file. This file
//  implements a standard thread that is used by all of the IR server side
//  drivers that support receiver functionality. They all need a way to queue
//  up events and have them processed. This thread provides that service.
//  They can queue mapped events on us and we will process them sequentially,
//  and provide an API for them to control us appropriately.
//
//  This significantly reduces the redundant code in each server driver.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//  A very simple class we use to queue up actions to be processed. It's
//  only used internally, but we have to define it publically in order to
//  have a collection of them below in the thread class.
// ---------------------------------------------------------------------------
class TCQCIRActInfo
{
    public :
        TCQCIRActInfo() = delete;

        TCQCIRActInfo
        (
            const   TCQCStdKeyedCmdSrc&     csrcAct
            , const TString&                strActKey
            , const TString&                strActData
        );

        TCQCIRActInfo(const TCQCIRActInfo&) = delete;
        TCQCIRActInfo(TCQCIRActInfo&&) = delete;

        ~TCQCIRActInfo();

        TCQCIRActInfo& operator=(const TCQCIRActInfo&) = delete;
        TCQCIRActInfo& operator=(TCQCIRActInfo&&) = delete;

        TCQCStdKeyedCmdSrc  m_csrcAct;
        TString             m_strActData;
        TString             m_strActKey;
};



// ---------------------------------------------------------------------------
//   CLASS: TCQCIRActionsThread
//  PREFIX: irrc
// ---------------------------------------------------------------------------
class CQCIREXPORT TCQCIRActionsThread : public TThread
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCIRActionsThread() = delete;

        TCQCIRActionsThread
        (
            const   TString&                strMoniker
            , const TCQCUserCtx&            cuctxToUse
        );

        TCQCIRActionsThread(const TCQCIRActionsThread&) = delete;
        TCQCIRActionsThread(TCQCIRActionsThread&&) = delete;

        ~TCQCIRActionsThread();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCIRActionsThread& operator=(const TCQCIRActionsThread&) = delete;
        TCQCIRActionsThread& operator=(TCQCIRActionsThread&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIsEmpty() const;

        tCIDLib::TBoolean bVerboseMode() const;

        tCIDLib::TBoolean bVerboseMode
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TVoid FlushQueue();

        tCIDLib::TVoid QueueEvent
        (
            const   TCQCStdKeyedCmdSrc&     csrcToQueue
            , const TString&                strActKey
            , const TString&                strActData
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::EExitCodes eProcess() final;


    private :
        // -------------------------------------------------------------------
        //  Class types
        // -------------------------------------------------------------------
        using TEventList = TRefQueue<TCQCIRActInfo>;


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bVerbose
        //      We can be put into verbose mode, in which case we will log
        //      each event that we process.
        //
        //  m_colEvents
        //      This is a collection of mapped events. The clients call us
        //      to add events to the list, which we process. We make it thread
        //      safe, since it's the primary member that's accessed by us and
        //      outside code. If we need any other sync, we'll use this guy's
        //      locking for that too.
        //
        //      We have to make copies of the command sources, since their
        //      config could change at any time, so we can't just keep pointers
        //      to them around. We store pointers to our action info objects,
        //      but it stores a copy of the action itself.
        //
        //      We use a queue, since we need to do FIFO processing, and we
        //      need one that provides synchronized waiting for input.
        //
        //  m_cuctxToUse
        //      The user account context that this thread should use when
        //      user credentials are required.
        //
        //  m_strMoniker
        //      The owning driver gives us his moniker so we can use it in
        //      error messages.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bVerbose;
        TEventList              m_colEvents;
        const TCQCUserCtx&      m_cuctxToUse;
        TString                 m_strMoniker;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCIRActionsThread,TThread)
};

#pragma CIDLIB_POPPACK



