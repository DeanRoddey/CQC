//
// FILE NAME: CQCAct_StdActEngine.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/01/2008
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
//  This file implements
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
#include    "CQCAct_.hpp"


// ---------------------------------------------------------------------------
//  Magic Macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCStdActEngine,TCQCActEngine)


// ---------------------------------------------------------------------------
//   CLASS: TCQCStdActEngine
//  PREFIX: acte
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCStdActEngine: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCStdActEngine::TCQCStdActEngine(const TCQCUserCtx& cuctxToUse) :

    TCQCActEngine(cuctxToUse)
    , m_colExtra(tCIDLib::EAdoptOpts::NoAdopt, 64)
    , m_colTargets(tCIDLib::EAdoptOpts::NoAdopt, 64)
    , m_ctarLocals(tCIDLib::EMTStates::Unsafe, kCIDLib::True)
    , m_pcmdtDebug(nullptr)
{
}

TCQCStdActEngine::~TCQCStdActEngine()
{
}


// ---------------------------------------------------------------------------
//  TCQCStdActEngine: Public, inherited methods
// ---------------------------------------------------------------------------
tCQCKit::TCmdTarList& TCQCStdActEngine::colExtra()
{
    return m_colExtra;
}


const TStdVarsTar& TCQCStdActEngine::ctarLocals() const
{
    return m_ctarLocals;
}

TStdVarsTar& TCQCStdActEngine::ctarLocals()
{
    return m_ctarLocals;
}


MCQCCmdTracer* const TCQCStdActEngine::pcmdtDebug()
{
    return m_pcmdtDebug;
}


tCIDLib::TVoid TCQCStdActEngine::Reset()
{
    // Call our parent first
    TParent::Reset();
}


// ---------------------------------------------------------------------------
//  TCQCStdActEngine: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  This method is called when the caller wants the action to happen in the
//  background.
//
tCQCKit::ECmdRes TCQCStdActEngine::
eInvokeOpsInBgn(const   MCQCCmdSrcIntf&             mcsrcSender
                , const TString&                    strEventId
                ,       TStdVarsTar&                ctarGlobals
                ,       TRefVector<MCQCCmdTarIntf>& colExtraTars
                ,       MCQCCmdTracer* const        pcmdtDebug
                , const TString&                    strParms)
{
    //
    //  Set up an action info structure with the info and pass that to the
    //  thread.
    //
    TActInfo Info;
    Info.pmcsrcSender = &mcsrcSender;
    Info.pstrEventId  = &strEventId;
    Info.pctarGlobals = &ctarGlobals;
    Info.pcolExtraTars= &colExtraTars;
    Info.pcmdtDebug   = pcmdtDebug;
    Info.eRes         = tCQCKit::ECmdRes::Unknown;
    Info.c4ErrIndex   = 0;
    Info.bDoneFlag    = kCIDLib::False;
    Info.pstrParms    = &strParms;

    //
    //  We have to insure a unique name for each thread we invoke, so we have
    //  a local counter that we just bump for each one.
    //
    static tCIDLib::TCard4 c4InstId = 1;

    TString strThrName(L"StdCmdEngineThr");
    strThrName.AppendFormatted(c4InstId++);

    //
    //  This has to be done in the context of the calling thread, since
    //  this might be a GUI based derivative and this directly resets
    //  any output display.
    //
    if (pcmdtDebug)
        pcmdtDebug->ActStartNew(ctarGlobals);

    // And now spin up the thread
    TThread thrBgnProc
    (
        strThrName
        , TMemberFunc<TCQCStdActEngine>(this, &TCQCStdActEngine::eBgnProc)
    );
    thrBgnProc.Start(&Info);

    // And wait for it to complete
    WaitEnd(Info.bDoneFlag);

    // If an exception, call the error callback
    if (Info.eRes == tCQCKit::ECmdRes::Except)
        CmdInvokeErr(Info.errCaught, Info.c4ErrIndex);

    //
    //  If we have a debug tracer, issue an end callback. This one also
    //  should be done in the context of the calling thread.
    //
    if (pcmdtDebug)
        pcmdtDebug->ActEnd(Info.eRes == tCQCKit::ECmdRes::Ok);

    // Return whatever the processing thread returned
    return Info.eRes;
}


//
//  This method is called to invoke the operations. We get the command srce
//  that's sending the commands, the event id that indicates the specific
//  set of commands of the source's to run, and a global variables target
//  which the containing app has to provide because they have to exist
//  across invocations.
//
//  They can pass in other targets, but they must be ones that can be
//  processed without any user interaction.
//
//  They also can provide an optional tracer for debugging.
//
//  This method is called directly usually. If the app is a GUI app, such
//  as the Admin Intf, which needs to be able to invoke mapped key type
//  commands, then it should call the version above that wil spin up a
//  thread to run the commands. That thread will just turn around can
//  call us back here.
//
tCQCKit::ECmdRes
TCQCStdActEngine::eInvokeOps(const  MCQCCmdSrcIntf&             mcsrcSender
                            , const TString&                    strEventId
                            ,       TStdVarsTar&                ctarGlobals
                            ,       TRefVector<MCQCCmdTarIntf>& colExtraTars
                            ,       MCQCCmdTracer* const        pcmdtDebug
                            , const TString&                    strParms)
{
    //
    //  This has to be done in the context of the calling thread, since
    //  this might be a GUI based derivative and this directly resets
    //  any output display.
    //
    if (pcmdtDebug)
        pcmdtDebug->ActStartNew(ctarGlobals);

    // Pass it on the private helper that does the actual work
    tCIDLib::TCard4 c4ErrIndex;
    TError errCaught;
    const tCQCKit::ECmdRes eRes = eInvoke
    (
        mcsrcSender
        , strEventId
        , ctarGlobals
        , colExtraTars
        , pcmdtDebug
        , errCaught
        , c4ErrIndex
        , strParms
    );

    // If we got an exception, let the invoking app handle it
    if (eRes == tCQCKit::ECmdRes::Except)
        CmdInvokeErr(errCaught, c4ErrIndex);

    //
    //  If we have a debug tracer, issue an end callback. This one also
    //  should be done in the context of the calling thread.
    //
    if (pcmdtDebug)
        pcmdtDebug->ActEnd(eRes == tCQCKit::ECmdRes::Ok);

    return eRes;
}


// Provide access to our standard targets
const TEvSrvCmdTar& TCQCStdActEngine::ctarEventSrv() const
{
    return m_ctarEventSrv;
}

TEvSrvCmdTar& TCQCStdActEngine::ctarEventSrv()
{
    return m_ctarEventSrv;
}

const TStdFieldCmdTar& TCQCStdActEngine::ctarFlds() const
{
    return m_ctarFlds;
}

TStdFieldCmdTar& TCQCStdActEngine::ctarFlds()
{
    return m_ctarFlds;
}

const TStdMacroCmdTar& TCQCStdActEngine::ctarMacro() const
{
    return m_ctarMacro;
}

TStdMacroCmdTar& TCQCStdActEngine::ctarMacro()
{
    return m_ctarMacro;
}

const TStdSystemCmdTar& TCQCStdActEngine::ctarSystem() const
{
    return m_ctarSystem;
}

TStdSystemCmdTar& TCQCStdActEngine::ctarSystem()
{
    return m_ctarSystem;
}


// ---------------------------------------------------------------------------
//  TCQCStdActEngine: Protected, virtual methods
// ---------------------------------------------------------------------------

//
//  This is called on each config object, to expand any tokens in it. In our
//  case we just do the default command prep provided by the CQCAct facility.
//  Some derived classes might first pre-expand some tokens of its own and
//  then call us.
//
//  And there are some parameter types that will not be expanded, such as expressions,
//  because the information about fields and variables needs to still be there for
//  the command handler.
//
tCIDLib::TBoolean
TCQCStdActEngine::bDoCmdPrep(       TCQCCmdCfg&         ccfgToPrep
                            , const TCQCCmdRTVSrc&      crtsVals
                            , const TStdVarsTar&        ctarGVars
                            , const TStdVarsTar&        ctarLVars
                            ,       TString&            strTmp)
{
    return facCQCKit().bDoCmdPrep
    (
        ccfgToPrep, crtsVals, &ctarGVars, &ctarLVars, strTmp
    );
}


// If the derived class doesn't handle errors, we just throw
tCIDLib::TVoid
TCQCStdActEngine::CmdInvokeErr( const   TError&         errToCatch
                                , const tCIDLib::TCard4)
{
    throw errToCatch;
}


//
//  This is called in the background thread scenario, after the thread is
//  started. It's assumed the called derivative will enter a modal loop. We
//  pass them an end flag that the processing thread will set when it's
//  done. They can pass this to the modal loop to have it end when the thread
//  sets it.
//
tCIDLib::TVoid TCQCStdActEngine::WaitEnd(tCIDLib::TBoolean&)
{
    // Empty default implementation
}


//
//  This is called to allow the derived class to do anything it needs to get
//  ready for processing. We do nothing
//
tCIDLib::TVoid
TCQCStdActEngine::PreLoop(  const   MCQCCmdSrcIntf&
                            ,       tCQCKit::TCmdTarList&
                            , const TString&)
{
}


// ---------------------------------------------------------------------------
//  TCQCStdActEngine: Protected, virtual methods
// ---------------------------------------------------------------------------

//
//  Background invocations are pointed at this method on a new thread. We
//  get a pointer to the data we need.
//
tCIDLib::EExitCodes
TCQCStdActEngine::eBgnProc(TThread& thrThis, tCIDLib::TVoid* pData)
{
    TActInfo* pInfo = static_cast<TActInfo*>(pData);

    // Let the calling thread go
    thrThis.Sync();

    try
    {
        // Call the helper to do the actual work
        pInfo->eRes = eInvoke
        (
            *pInfo->pmcsrcSender
            , *pInfo->pstrEventId
            , *pInfo->pctarGlobals
            , *pInfo->pcolExtraTars
            , pInfo->pcmdtDebug
            , pInfo->errCaught
            , pInfo->c4ErrIndex
            , *pInfo->pstrParms
        );
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);

        pInfo->eRes = tCQCKit::ECmdRes::Except;
        pInfo->errCaught = errToCatch;
    }

    catch(...)
    {
        // Shouldn't happen, but we can't risk not catching it
        pInfo->eRes = tCQCKit::ECmdRes::Except;

        // Fall through
    }

    // Indicate we are done
    pInfo->bDoneFlag = kCIDLib::True;

    return tCIDLib::EExitCodes::Normal;
}


//
//  This is a private helper that does the actual work. We have multiple
//  ways of invoking the engine, and they ultimately get around to calling
//  this method to do the real work.
//
tCQCKit::ECmdRes
TCQCStdActEngine::eInvoke(  const   MCQCCmdSrcIntf&             mcsrcSender
                            , const TString&                    strEventId
                            ,       TStdVarsTar&                ctarGlobals
                            ,       TRefVector<MCQCCmdTarIntf>& colExtraTars
                            ,       MCQCCmdTracer* const        pcmdtDebug
                            ,       TError&                     errCaught
                            ,       tCIDLib::TCard4&            c4ErrIndex
                            , const TString&                    strParms)
{
    // Store the tracer pointer if provided, with a janitor to insure it's cleared
    TGFJanitor<MCQCCmdTracer*> janDebug(&m_pcmdtDebug, pcmdtDebug);

    //
    //  Set up the standard targets and load up a collection of them. We
    //  proivde a local vars. The caller must provide a global vars if
    //  desired. Local vars don't need to be thread safe since they are
    //  only accessed here locally.
    //
    m_colTargets.RemoveAll();
    m_colTargets.Add(&m_ctarEventSrv);
    m_colTargets.Add(&m_ctarFlds);
    m_colTargets.Add(&ctarGlobals);
    m_colTargets.Add(&m_ctarLocals);
    m_colTargets.Add(&m_ctarMacro);
    m_colTargets.Add(&m_ctarSystem);

    //
    //  If the user passed in any extra targets, add them, plus we also keep
    //  a separate list of those. Flush the extra list first or we'll just
    //  accumulate them. It's only a pointer, but still.
    //
    m_colExtra.RemoveAll();
    const tCIDLib::TCard4 c4ETCount = colExtraTars.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ETCount; c4Index++)
    {
        m_colExtra.Add(colExtraTars[c4Index]);
        m_colTargets.Add(colExtraTars[c4Index]);
    }

    // Initialize us, passing along any action parameters we got
    Init(strParms);

    const tCIDLib::TCard4 c4TarCount = m_colTargets.c4ElemCount();
    tCQCKit::ECmdRes eRes = tCQCKit::ECmdRes::Ok;
    tCIDLib::TCard4 c4Index = 0;
    try
    {
        //
        //  Go through all of the targets and initialize them. This MUST be
        //  done in the context of the thread that will be running the action,
        //  as must the cleanup at the end below.
        //
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4TarCount; c4Index++)
            m_colTargets[c4Index]->CmdTarInitialize(*this);

        //
        //  We have to make a copy of the opcodes, because we will modify
        //  them as we process them. If there aren't any, then we just
        //  return. This also resolves all the targets for us.
        //
        const tCIDLib::TCard4 c4CmdCount = mcsrcSender.c4GetResolvedOpList
        (
            m_colTargets, m_colOps, strEventId
        );

        //
        //  Ask the source object to gen up a runtime value source object.
        //  It's done this way because, though not here, in some cases the
        //  sending object can be destroyed by the processing of commands,
        //  and we wouldn't even be seeing it in those cases, we'd just
        //  get the runtime value object it has provided.
        //
        //  We just look at it via the base class interface.
        //
        TCQCCmdRTVSrc* pcrtsRTVs = mcsrcSender.pcrtsMakeNew(cuctxToUse());
        TJanitor<TCQCCmdRTVSrc> janRTVs(pcrtsRTVs);

        // Loop through all of the commands that the source is configured to send
        TString strPVal;
        while (c4Index < c4CmdCount)
        {
            // Get the current opcode out
            TActOpcode& aocCur = m_colOps.objAt(c4Index);

            // Ignore comments and disabled commands.
            if ((aocCur.m_eOpcode == tCQCKit::EActOps::Comment) || aocCur.m_bDisabled)
            {
                c4Index++;
                continue;
            }

            //
            //  For Ends of conditionals, for now we just treat them as no
            //  ops. But later, if we handle loops, we will have to deal with
            //  this potentially being the end of a loop.
            //
            if (aocCur.m_eOpcode == tCQCKit::EActOps::End)
            {
                // <TBD> Deal with loops here eventually
                c4Index++;
                continue;
            }

            //
            //  If it's an else, just change our index and go back to the top.
            //  Else is just a jump. It just marks the end of the If portion
            //  of an If/Else and jumps to the End.
            //
            if (aocCur.m_eOpcode == tCQCKit::EActOps::Else)
            {
                c4Index += aocCur.m_i4Ofs;
                continue;
            }

            //
            //  Looks like a keeper, so do the standard prep work on it,
            //  to do token replacement and such. We can't change the
            //  original, so we have to make a copy.
            //
            //  We give him the runtime value object (which is requird)
            //  and the optional local/global variables pointers.
            //
            TCQCCmdCfg ccfgCur = aocCur.m_ccfgStep;
            bDoCmdPrep(ccfgCur, *pcrtsRTVs, ctarGlobals, m_ctarLocals, strPVal);

            // Look up the target. If not found, that's bad, give up
            MCQCCmdTarIntf* pmctarSel = pctarFind(ccfgCur.c4TargetId());

            // If we didn't find it, that's bad
            if (!pmctarSel)
            {
                facCQCAct().ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kActErrs::errcCmd_TarNotFound
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::BadParms
                    , ccfgCur.strTargetName()
                    , ccfgCur.strTargetId()
                );
            }

            // If we have a tracer, pass it the expanded command
            if (m_pcmdtDebug)
                m_pcmdtDebug->ActCmd(aocCur, ccfgCur, c4Index + 1);

            //
            //  Invoke the command. We don't use the UserId stuff here, so
            //  pass an empty string. It's used in the interface system.
            //
            //  If this is a conditional type opcode, the command will
            //  set the passed boolean value. If it's not, then it won't
            //  change it.
            //
            tCIDLib::TBoolean bRes;
            tCQCKit::ECmdRes eRes = pmctarSel->eDoCmd
            (
                ccfgCur
                , c4Index
                , TString::strEmpty()
                , strEventId
                , ctarGlobals
                , bRes
                , *this
            );

            // If they asked to stop, then return the stop status
            if (eRes == tCQCKit::ECmdRes::Stop)
                return eRes;

            //
            //  If this is a conditional code, then we have to check the
            //  result and jump or not.
            //
            if (aocCur.m_eOpcode == tCQCKit::EActOps::If)
            {
                // If the opcode is negated, then negate it
                if (aocCur.m_bNegate)
                    bRes = !bRes;

                if (bRes)
                {
                    // It passed, so just fall through
                    c4Index++;
                }
                 else
                {
                    // It failed, so jump
                    c4Index += aocCur.m_i4Ofs;

                    //
                    //  If debugging, verify that we ended up on an Else
                    //  or End opcode. We are either going to the end of
                    //  an If or While, or jumping to the Else part of an
                    //  If.
                    //
                    #if CID_DEBUG_ON
                    const TActOpcode& aocJumpTo = m_colOps[c4Index];
                    if ((aocJumpTo.m_eOpcode != tCQCKit::EActOps::Else)
                    &&  (aocJumpTo.m_eOpcode != tCQCKit::EActOps::End))
                    {
                        facCQCAct().ThrowErr
                        (
                            CID_FILE
                            , CID_LINE
                            , kActErrs::errcCmd_BadIfTarget
                            , tCIDLib::ESeverities::Failed
                            , tCIDLib::EErrClasses::BadParms
                        );
                    }
                    #endif

                    //
                    //  And move forward one more, since the Else is a
                    //  jump to the End, and the End is a dummy so no need
                    //  to process it.
                    //
                    c4Index++;
                }
            }
             else
            {
                // Nothing special, so just move to the next step
                c4Index++;
            }
        }
    }

    catch(const TError& errToCatch)
    {
        c4ErrIndex = c4Index;
        errCaught = errToCatch;
        eRes = tCQCKit::ECmdRes::Except;
    }

    catch(...)
    {
        c4ErrIndex = c4Index;
        eRes = tCQCKit::ECmdRes::UnknownExcept;
    }

    // Do a cleanup pass on all the targets
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4TarCount; c4Index++)
    {
        try
        {
            m_colTargets[c4Index]->CmdTarCleanup();
        }

        catch(const TError& errToCatch)
        {
            if (facCQCAct().bShouldLog(errToCatch))
                TModule::LogEventObj(errToCatch);

            if (facCQCAct().bLogFailures())
            {
                facCQCAct().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kActErrs::errcCmd_TarCleanupErr
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Internal
                    , m_colTargets[c4Index]->strCmdTargetType()
                );
            }
        }

        catch(...)
        {
            if (facCQCAct().bLogFailures())
            {
                facCQCAct().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kActErrs::errcCmd_TarCleanupErr
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Internal
                    , m_colTargets[c4Index]->strCmdTargetType()
                );
            }
        }
    }

    return eRes;
}


// This is a helper to find a target in our list by numeric id
MCQCCmdTarIntf* TCQCStdActEngine::pctarFind(const tCIDLib::TCard4 c4TarId)
{
    const tCIDLib::TCard4 c4TarCnt = m_colTargets.c4ElemCount();
    for (tCIDLib::TCard4 c4TarInd = 0; c4TarInd < c4TarCnt; c4TarInd++)
    {
        if (m_colTargets[c4TarInd]->c4UniqueId() == c4TarId)
            return m_colTargets[c4TarInd];
    }
    return 0;
}

