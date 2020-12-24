//
// FILE NAME: CQCIntfEng_ActEngine.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/16/2005
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
//  This file implements a specialized action engine for the user interface
//  system, which has various special concerns.
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
#include    "CQCIntfEng_.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCIntfAction,TObject)
RTTIDecls(TCQCIntfActCtx,TObject)




// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfAction
//  PREFIX: cia
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfAction: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfAction::TCQCIntfAction( const   TString&                strUserId
                                , const TString&                strEvent
                                , const MCQCCmdSrcIntf&         mcsrcToDo
                                ,       tCQCKit::TCmdTarList&   colStdTars
                                ,       tCQCKit::TCmdTarList&   colWdgTars
                                , const tCIDLib::TCard4         c4HandlerId
                                , const tCIDLib::TBoolean       bEventBased
                                , const TCQCUserCtx&            cuctxToUse
                                , const TCQCActEngine&          acteTar
                                , const TArea&                  areaSrcWidget) :

    m_bEventBased(bEventBased)
    , m_c4HandlerId(c4HandlerId)
    , m_c4Index(0)
    , m_ctarLocalVars(tCIDLib::EMTStates::Unsafe, kCIDLib::True)
    , m_pcrtsToUse(0)
    , m_strEventId(strEvent)
    , m_strUserId(strUserId)
{
    //
    //  We need to build up a comprehensive list of all the available targets
    //  that our commands could reference, so that we can get a list of the
    //  command opcodes, with all the target ids looked up. So we include the
    //  two passed lists, plus our local variables target.
    //
    tCQCKit::TCmdTarList colAllTars
    (
        tCIDLib::EAdoptOpts::NoAdopt
        , colStdTars.c4ElemCount() + colWdgTars.c4ElemCount() + 1
    );

    tCIDLib::TCard4 c4Count = colStdTars.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        colAllTars.Add(colStdTars[c4Index]);

    c4Count = colWdgTars.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        colAllTars.Add(colWdgTars[c4Index]);

    //
    //  Load any action parameters from our engine into this level's local vars,
    //  then add it to the target list.
    //
    acteTar.QueryActParms(m_ctarLocalVars);
    colAllTars.Add(&m_ctarLocalVars);

    //
    //  Now we can steal copies of all the source's opcodes for this event.
    //  This also resolves all the target ids as well, which is why we
    //  needed the full list of targets.
    //
    mcsrcToDo.c4GetResolvedOpList(colAllTars, m_colOps, strEvent);

    // Get a runtime value object from the source
    m_pcrtsToUse = mcsrcToDo.pcrtsMakeNew(cuctxToUse);

    //
    //  Force in some special local variables. We provide the relative points of
    //  the corners of the area of the invoking widget.
    //
    TString strVal;
    areaSrcWidget.pntOrg().FormatToText(strVal, tCIDLib::ERadices::Dec, kCIDLib::chComma);
    m_ctarLocalVars.bAddOrUpdateVar
    (
        L"LVar:CQCSrcWidgetUL"
        , strVal
        , tCQCKit::EFldTypes::String
        , TString::strEmpty()
    );

    areaSrcWidget.pntUR().FormatToText(strVal, tCIDLib::ERadices::Dec, kCIDLib::chComma);
    m_ctarLocalVars.bAddOrUpdateVar
    (
        L"LVar:CQCSrcWidgetUR"
        , strVal
        , tCQCKit::EFldTypes::String
        , TString::strEmpty()
    );

    areaSrcWidget.pntLR().FormatToText(strVal, tCIDLib::ERadices::Dec, kCIDLib::chComma);
    m_ctarLocalVars.bAddOrUpdateVar
    (
        L"LVar:CQCSrcWidgetLR"
        , strVal
        , tCQCKit::EFldTypes::String
        , TString::strEmpty()
    );

    areaSrcWidget.pntLL().FormatToText(strVal, tCIDLib::ERadices::Dec, kCIDLib::chComma);
    m_ctarLocalVars.bAddOrUpdateVar
    (
        L"LVar:CQCSrcWidgetLL"
        , strVal
        , tCQCKit::EFldTypes::String
        , TString::strEmpty()
    );

    areaSrcWidget.pntCenter().FormatToText(strVal, tCIDLib::ERadices::Dec, kCIDLib::chComma);
    m_ctarLocalVars.bAddOrUpdateVar
    (
        L"LVar:CQCSrcWidgetCenter"
        , strVal
        , tCQCKit::EFldTypes::String
        , TString::strEmpty()
    );
}

TCQCIntfAction::~TCQCIntfAction()
{
    // Clean up the runtime value source object
    delete m_pcrtsToUse;
}


// ---------------------------------------------------------------------------
//  TCQCIntfAction: Public, non-virtual methods
// ---------------------------------------------------------------------------
const TActOpcode& TCQCIntfAction::aocCurrent() const
{
    return m_colOps[m_c4Index];
}


tCIDLib::TBoolean TCQCIntfAction::bAtEnd() const
{
    return (m_c4Index >= m_colOps.c4ElemCount());
}


tCIDLib::TBoolean TCQCIntfAction::bEventBased() const
{
    return m_bEventBased;
}


tCIDLib::TCard4 TCQCIntfAction::c4HandlerId() const
{
    return m_c4HandlerId;
}


tCIDLib::TCard4 TCQCIntfAction::c4Index() const
{
    return m_c4Index;
}

tCIDLib::TCard4 TCQCIntfAction::c4Index(const tCIDLib::TCard4 c4ToSet)
{
    m_c4Index = c4ToSet;
    return m_c4Index;
}


tCIDLib::TCard4 TCQCIntfAction::c4OpCount() const
{
    return m_colOps.c4ElemCount();
}


const TStdVarsTar& TCQCIntfAction::ctarLocalVars() const
{
    return m_ctarLocalVars;
}

TStdVarsTar& TCQCIntfAction::ctarLocalVars()
{
    return m_ctarLocalVars;
}


TCQCCmdRTVSrc& TCQCIntfAction::crtsToUse() const
{
    return *m_pcrtsToUse;
}


tCIDLib::TVoid
TCQCIntfAction::FormatOpAt(const   tCIDLib::TCard4 c4At
                           ,       TString&        strToFill) const
{
    // Format in the non-nested mode
    tCIDLib::TCard4 c4Nesting = 0;
    m_colOps[c4At].FormatTo(strToFill, c4Nesting, kCIDLib::False);
}


tCIDLib::TVoid TCQCIntfAction::FormatCurOp(TString& strToFill) const
{
    // Format in the non-nested mode
    tCIDLib::TCard4 c4Nesting = 0;
    m_colOps[m_c4Index].FormatTo(strToFill, c4Nesting, kCIDLib::False);
}


tCIDLib::TVoid TCQCIntfAction::OffsetIndex(const tCIDLib::TInt4 i4OfsBy)
{
    m_c4Index += i4OfsBy;
}


const TString& TCQCIntfAction::strEventId() const
{
    return m_strEventId;
}


const TString& TCQCIntfAction::strUserId() const
{
    return m_strUserId;
}




// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfActCtx
//  PREFIX: ciac
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfActCtx: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfActCtx::TCQCIntfActCtx(const TCQCUserCtx& cuctxToUse) :

    TCQCActEngine(cuctxToUse)
    , m_bGotTrailingAction(kCIDLib::False)
    , m_bPopupRes(kCIDLib::False)
    , m_colActStack(tCIDLib::EAdoptOpts::Adopt)
    , m_colExtra(tCIDLib::EAdoptOpts::NoAdopt, 128)
    , m_colTargets(tCIDLib::EAdoptOpts::NoAdopt, 64)
    , m_eCurState(EStates::Idle)
    , m_pcmdtDebug(nullptr)
    , m_pctarGlobals(nullptr)
    , m_pcivOwner(nullptr)
    , m_thrRunner
      (
        facCIDLib().strNextThreadName(L"CQCRunAction")
        , TMemberFunc<TCQCIntfActCtx>(this, &TCQCIntfActCtx::eRunThread)
      )
{
}

TCQCIntfActCtx::~TCQCIntfActCtx()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfActCtx: Public, inherited methods
// ---------------------------------------------------------------------------
tCQCKit::TCmdTarList& TCQCIntfActCtx::colExtra()
{
    return m_colExtra;
}

const TStdVarsTar& TCQCIntfActCtx::ctarLocals() const
{
    CIDAssert(!m_colActStack.bIsEmpty(), L"The action stack is empty, can't return local vars");
    return m_colActStack.pobjPeekTop()->ctarLocalVars();
}

TStdVarsTar& TCQCIntfActCtx::ctarLocals()
{
    CIDAssert(!m_colActStack.bIsEmpty(), L"The action stack is empty, can't return local vars");
    return m_colActStack.pobjPeekTop()->ctarLocalVars();
}


// We override this so that we can do some init then call the base engine
tCIDLib::TVoid TCQCIntfActCtx::Init(const TString& strActParms)
{
    TParent::Init(strActParms);

    // If the popup result isn't explicitly set, we want to return false
    m_bPopupRes = kCIDLib::False;

    m_colRetVals.RemoveAll();
    m_colTmplVals.RemoveAll();
}


// Provide access to the action tracer plugged in on us
MCQCCmdTracer* const TCQCIntfActCtx::pcmdtDebug()
{
    return m_pcmdtDebug;
}


//
//  Resets the context so that old data can't be accidentally used. Note that
//  return values and template values are not reset, since they are only
//  reset when a new popup layer is created.
//
tCIDLib::TVoid TCQCIntfActCtx::Reset()
{
    CIDAssert(!m_thrRunner.bIsRunning(), L"The action thread is still running");

    // Call our parent first
    TParent::Reset();

    m_bGotTrailingAction    = kCIDLib::False;
    m_eCurState             = EStates::Idle;
    m_pcmdtDebug            = 0;
    m_pctarGlobals          = 0;
    m_pcivOwner             = 0;

    m_colActStack.RemoveAll();
    m_colTargets.RemoveAll();
}


// ---------------------------------------------------------------------------
//  TCQCIntfActCtx: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Get/set the trailing action flag. See the header comments for the
//  significance of this flag.
//
tCIDLib::TBoolean TCQCIntfActCtx::bGotTrailingAction() const
{
    return m_bGotTrailingAction;
}

tCIDLib::TBoolean
TCQCIntfActCtx::bGotTrailingAction(const tCIDLib::TBoolean bToSet)
{
    m_bGotTrailingAction = bToSet;
    return m_bGotTrailingAction;
}


// Get the last stored popup result
tCIDLib::TBoolean TCQCIntfActCtx::bPopupRes() const
{
    return m_bPopupRes;
}

tCIDLib::TBoolean TCQCIntfActCtx::bPopupRes(const tCIDLib::TBoolean bToSet)
{
    m_bPopupRes = bToSet;
    return m_bPopupRes;
}


// Query a return value by name
tCIDLib::TBoolean
TCQCIntfActCtx::bQueryRetValue(const TString& strName, TString& strToFill) const
{
    const tCIDLib::TCard4 c4Count = m_colRetVals.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TKeyValuePair& kvalCur = m_colRetVals[c4Index];
        if (kvalCur.strKey() == strName)
        {
            strToFill = kvalCur.strValue();
            return kCIDLib::True;
        }
    }
    return kCIDLib::False;
}


// Query a template value by name
tCIDLib::TBoolean
TCQCIntfActCtx::bQueryTmplValue(const TString& strName, TString& strToFill) const
{
    const tCIDLib::TCard4 c4Count = m_colTmplVals.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TKeyValuePair& kvalCur = m_colTmplVals[c4Index];
        if (kvalCur.strKey() == strName)
        {
            strToFill = kvalCur.strValue();
            return kCIDLib::True;
        }
    }
    return kCIDLib::False;
}


//
//  We go through our list of active actions and see if any of them are
//  marked as event based. If so, we return true. The client code may care
//  about this and want to prevent more than one event based action at once.
//
tCIDLib::TBoolean TCQCIntfActCtx::bPendingEventAct() const
{
    // Lock while do this since the processing thread is banging on stack
    TLocker lockrSync(&m_mtxSync);

    TRefStack<TCQCIntfAction>::TCursor cursStack(&m_colActStack);
    if (cursStack.bReset())
    {
        do
        {
            if (cursStack.objRCur().bEventBased())
                return kCIDLib::True;
        }   while (cursStack.bNext());
    }
    return kCIDLib::False;
}


//
//  Let's the outside world see if we are processing commands currently
//  or not, but generally they don't really need to check this because the
//  ePostOps() method does most of the required checking for them.
//
tCIDLib::TBoolean TCQCIntfActCtx::bProcessing() const
{
    tCIDLib::TBoolean bRet;
    {
        TLocker lockrSync(&m_mtxSync);
        bRet = (m_eCurState > EStates::Idle);
    }
    return bRet;
}


//
//  Client code calls this to post a new action to be processed. Since
//  actions can recurse, and therefore we can get called back into here
//  while previous actions are already running, we get two sets of info.
//  The first bunch of parameters are only used if this is a new action,
//  i.e. we aren't already processing. In that case, we store that info
//  as the action context info. Else, it's ignored and assumed to be the
//  same info as previously provided.
//
//  The reason we don't have a separate method to set the first bunch is
//  for synchronization reasons. We need to have a single (atomic from
//  the caller's perspective) method to see if the action is active and
//  if so, to get the new action posted in such a way that the thread
//  cannot exit in the meantime. Otherwise, if the caller first checked
//  via another method, the thread could end before it got here and
//  we couldn't return a valid status to him to tell him if he needs to
//  process the commands or just continue because we are already processing.
//
tCQCKit::EActPostRes
TCQCIntfActCtx::ePostOps(       TCQCIntfView* const     pcivOwner
                        ,       TStdVarsTar* const      pctarGlobals
                        ,       tCQCKit::TCmdTarList&   colExtraStdTars
                        ,       MCQCCmdTracer* const    pcmdtDebug
                        , const MCQCCmdSrcIntf&         mcsrcOps
                        ,       tCQCKit::TCmdTarList&   colWdgTars
                        , const TString&                strEvent
                        , const TString&                strUserId
                        , const tCIDLib::TCard4         c4HandlerId
                        , const tCIDLib::TBoolean       bEventBased
                        , const tCQCKit::EActOrders     eOrder
                        , const TArea&                  areaSrcWidget)
{
    // Some of them cannot be null
    CIDAssert(pcivOwner != 0, L"The owner view cannot be null");
    CIDAssert(pctarGlobals != 0, L"The globals target cannot be null");

    //
    //  Do a quick check to see if the source has any opcodes for the passed
    //  event. If not, no need to go through the trouble of even getting
    //  started.
    //
    if (!mcsrcOps.c4OpCount(strEvent))
        return tCQCKit::EActPostRes::NoAction;

    // Lock while we do the rest
    TLocker lockrSync(&m_mtxSync);

    //
    //  If we are not already processing, then we need to store the context
    //  info. Otherwise it's already set.
    //
    if (m_eCurState == EStates::Idle)
    {
        // Reset the trailing action flag just to be safe
        m_bGotTrailingAction = kCIDLib::False;

        // Store away the incoming info
        m_pcmdtDebug = pcmdtDebug;
        m_pcivOwner = pcivOwner;

        //
        //  Remove any actions that might have gotten left on the stack.
        //  That should not happen, but don't take any chances.
        //
        m_colActStack.RemoveAll();

        //
        //  Clean out any previous targets and add all the standard ones,
        //  except local variables which are action specific.
        //
        m_colTargets.RemoveAll();
        m_colTargets.Add(&m_ctarEventSrv);
        m_colTargets.Add(&m_ctarFlds);
        m_colTargets.Add(pctarGlobals);
        m_colTargets.Add(&m_ctarSystem);
        m_colTargets.Add(&m_ctarMacros);

        //
        //  We keep a special pointer to the global vars target, since
        //  we have to provide it to a number of interfaces directly.
        //
        m_pctarGlobals = pctarGlobals;

        //
        //  If the user passed in any extra targets, add them, to both our
        //  main list and the extra list we have to maintain.
        //
        const tCIDLib::TCard4 c4ETCount = colExtraStdTars.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ETCount; c4Index++)
        {
            m_colTargets.Add(colExtraStdTars[c4Index]);
            m_colExtra.Add(colExtraStdTars[c4Index]);
        }

        // And any others that we have to put into the extra list
        m_colExtra.Add(&m_ctarEventSrv);
    }

    //
    //  We've got an action to post, so create an action object. He'll load
    //  up copies of all the opcodes, get an RTV object, and so forth.
    //
    //  NOTE:   The sending source object is not stored anywhere. It may be
    //          destroyed by this action. So it's important that we don't
    //          keep any reference to it.
    //
    TCQCIntfAction* pciaToSend = new TCQCIntfAction
    (
        strUserId
        , strEvent
        , mcsrcOps
        , m_colTargets
        , colWdgTars
        , c4HandlerId
        , bEventBased
        , cuctxToUse()
        , *this
        , areaSrcWidget
    );

    // Put a janitor on it until we successfully get it set up
    TJanitor<TCQCIntfAction> janAction(pciaToSend);

    //
    //  Ok, we can push it on the stack now. If it works, then orphan it
    //  from the janitor. We remember if processing is already going on,
    //  so we can tell the caller how to react.
    //
    tCQCKit::EActPostRes eRes = tCQCKit::EActPostRes::Process;
    {
        if (m_eCurState != EStates::Idle)
            eRes = tCQCKit::EActPostRes::Already;

        //
        //  If we are past the running state and this is an onlreease, then
        //  this is a trailing action, so set the trailing action flag.
        //
        if (m_eCurState > EStates::Running)
            m_bGotTrailingAction = kCIDLib::True;

        // Push to the top or bottom, based on the order indicated
        if (eOrder == tCQCKit::EActOrders::Nested)
            m_colActStack.PushTop(janAction.pobjThis());
        else
            m_colActStack.PushBottom(janAction.pobjThis());
    }
    janAction.Orphan();

    // And tell the caller what to do
    return eRes;
}


//
//  This is where the action gets invoked. They've already been pushed onto
//  our stack by ePostOps above. The client code should only call this if they
//  pushed a new action (i.e. this context is not currently processing
//  commands.) If we are already processing, it should just continue on after
//  the post since our thread is already going.
//
//  Any failure will cause a throw. Otherwise, it'll return a status value
//  that indicates whether the action ended normally or if the user asked
//  to stop it.
//
tCQCKit::ECmdRes TCQCIntfActCtx::eInvokeCmds()
{
    // We must be in idle state if we are getting called here
    CIDAssert
    (
        m_eCurState == EStates::Idle
        , L"The intf action context was not in idle state on eInvokeCmds()"
    );

    //
    //  And owner and global vars can't be null. This won't actually return false,
    //  it'll throw. This is for the benefit of the code analyzer.
    //
    if (!bCIDAssert(m_pcivOwner != nullptr, L"Owning view was not set")
    ||  !bCIDAssert(m_pctarGlobals != nullptr, L"Global vars target was not set"))
    {
        return tCQCKit::ECmdRes::Stop;
    }

    //
    //  If we have a debug tracer, issue a start callback. This callback must
    //  be called within the GUI thread context. The others can be called form
    //  the background thread and just queue up stuff to be displayed.
    //
    if (m_pcmdtDebug)
        m_pcmdtDebug->ActStartNew(*m_pctarGlobals);

    //
    //  We have something to do, so let's crank up the background thread
    //  that does the actual work. We do this before we create ourself,
    //  since if it runs quickly, we won't even create the window.
    //
    //  We have to make sure we get the state back to idle whether we
    //  get out of here successfully or not, so we have to use a try
    //  around everything else.
    //
    m_strStepInErr.Clear();
    m_eCurState = EStates::Running;

    tCIDLib::TBoolean bDone = kCIDLib::False;
    try
    {
        //
        //  We can start up the background thread now. We pass him our local
        //  done thread, which he'll set when he exits.
        //
        m_thrRunner.Start(&bDone);

        //
        //  Now ask the viewer to do a modal loop until we see the action
        //  has completed. We pass it the done flag, so it'll see if the
        //  flag gets set and break out.
        //
        //  If the reason we get back is not because the done flag was set,
        //  then it probably was a cancel, but either way we want to insure
        //  that the thread knows he needs to exit now.
        //
        //  If by pathological chance the thread fell out and between the
        //  start above and this call, then the done flag will already be
        //  set and well'll just fall right back out.
        //
        m_pcivOwner->DoModalLoop(bDone, kCIDLib::False);

        if (m_thrRunner.bIsRunning())
            m_thrRunner.ReqShutdownSync(10000);

        // If we have a debug tracer, issue an end callback.
        if (m_pcmdtDebug)
            m_pcmdtDebug->ActEnd(m_eCurState < EStates::DoneErr);

        //
        //  The thread has completed or has been asked to exit, so let's wait
        //  for the thread to die.
        //
        m_thrRunner.eWaitForDeath(5000);

        //
        //  NOTE:   Do not clean up the action stack here. The thread will
        //          do any clean up required. And if a trailing mouse
        //          release event occurs, it can get posted as the thread
        //          is winding down. If so, the trailing action flag will
        //          be set and the view will see that when we get back and
        //          crank back up to do the new commands.
        //

        if (m_eCurState == EStates::DoneUnknownExcept)
        {
            facCQCKit().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kKitErrs::errcCmd_CmdUnkownExcept
                , m_strStepInErr
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::CantDo
            );
        }
         else if (m_eCurState == EStates::DoneExcept)
        {
            // Just thro the exception that the thread caught
            throw m_errThrown;
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        if (m_thrRunner.bIsRunning())
        {
            try
            {
                m_thrRunner.ReqShutdownSync(2000);
                m_thrRunner.eWaitForDeath(8000);
            }

            catch(...)
            {
            }
        }
        m_eCurState = EStates::Idle;
        throw;
    }

    catch(...)
    {
        if (m_thrRunner.bIsRunning())
        {
            try
            {
                m_thrRunner.ReqShutdownSync(2000);
                m_thrRunner.eWaitForDeath(8000);
            }

            catch(...)
            {
            }
        }
        m_eCurState = EStates::Idle;

        facCQCIntfEng().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kIEngMsgs::midStatus_SysExcept
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppError
            , TString(L"action command")
        );
    }

    // The return indicates whether they asked to exit or stop or not
    tCQCKit::ECmdRes eRes = tCQCKit::ECmdRes::Ok;
    if (m_eCurState ==  EStates::DoneExit)
        eRes = tCQCKit::ECmdRes::Exit;
    else if (m_eCurState == EStates::DoneCancel)
        eRes = tCQCKit::ECmdRes::Stop;

    // Set the state back to idle now and return the result
    m_eCurState = EStates::Idle;
    return eRes;
}


//
//  Returns the stored user id (which is just a string value that we pass
//  through from the invoker of a command to the targets), for the top action
//  on the stack. If the stack is empty, we return an empty string.
//
//  It's returned by value because the context stack may be removed.
//
TString TCQCIntfActCtx::strCurUserId()
{
    TLocker lockrSync(&m_mtxSync);

    // Get the top item on the stack, and return it's user id
    if (m_colActStack.bIsEmpty())
        return TString::strEmpty();
    return m_colActStack.pobjPeekTop()->strUserId();
}


// Look up a return value, throw if not found
const TString& TCQCIntfActCtx::strRetValue(const TString& strName) const
{
    const tCIDLib::TCard4 c4Count = m_colRetVals.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TKeyValuePair& kvalCur = m_colRetVals[c4Index];
        if (kvalCur.strKey() == strName)
            return kvalCur.strValue();
    }

    facCQCIntfEng().ThrowErr
    (
        CID_FILE
        , CID_LINE
        , kIEngErrs::errcActEng_RetVal
        , tCIDLib::ESeverities::Failed
        , tCIDLib::EErrClasses::NotFound
        , strName
    );

    // Make the compiler happy
    return TString::strEmpty();
}


// Look up a template value, throw if not found
const TString& TCQCIntfActCtx::strTmplValue(const TString& strName) const
{
    const tCIDLib::TCard4 c4Count = m_colTmplVals.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TKeyValuePair& kvalCur = m_colTmplVals[c4Index];
        if (kvalCur.strKey() == strName)
            return kvalCur.strValue();
    }

    facCQCIntfEng().ThrowErr
    (
        CID_FILE
        , CID_LINE
        , kIEngErrs::errcActEng_TmplVal
        , tCIDLib::ESeverities::Failed
        , tCIDLib::EErrClasses::NotFound
        , strName
    );

    // Make the compiler happy
    return TString::strEmpty();
}



// Set a return value. We update if found, else add it
tCIDLib::TVoid
TCQCIntfActCtx::SetRetValue(const   TString&    strName
                            , const TString&    strToSet)
{
    const tCIDLib::TCard4 c4Count = m_colRetVals.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TKeyValuePair& kvalCur = m_colRetVals[c4Index];
        if (kvalCur.strKey() == strName)
        {
            kvalCur.strValue(strToSet);
            return;
        }
    }

    // Not found, so add it
    m_colRetVals.objAdd(TKeyValuePair(strName, strToSet));
}



// Set a template value. We update if found, else add it
tCIDLib::TVoid
TCQCIntfActCtx::SetTmplValue(const  TString&    strName
                            , const TString&    strToSet)
{
    const tCIDLib::TCard4 c4Count = m_colTmplVals.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TKeyValuePair& kvalCur = m_colTmplVals[c4Index];
        if (kvalCur.strKey() == strName)
        {
            kvalCur.strValue(strToSet);
            return;
        }
    }

    // Not found, so add it
    m_colTmplVals.objAdd(TKeyValuePair(strName, strToSet));
}


//
//  Copy all return info from the passed context object. This is called when
//  a popup is being closed, to let us copy down return info from invoked
//  context that is being closed. Currently it's just the return values
//  list.
//
tCIDLib::TVoid TCQCIntfActCtx::TakeRetInfoFrom(const TCQCIntfActCtx& ciacSrc)
{
    m_colRetVals = ciacSrc.m_colRetVals;
}


// ---------------------------------------------------------------------------
//  TCQCIntfActCtx: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  This is started up as a thread that runs the actual action steps. The
//  calling thread just waits around for an indication that the action is
//  completed or failed.
//
//  When we do an action, that can cause a call back to ePostOps but it'll
//  just push another action and return again, and then we come back here and
//  we have a new action on the top of the stack to start processing. We do
//  them FIFO, i.e. as a stack, because newly pushed actions are nested
//  actions invoked by something in the underlying action, and it needs to
//  be processed first before we return to the underlying one.
//
//  But, nonetheless, we do the right thing and synchronize access to all
//  data members.
//
tCIDLib::EExitCodes
TCQCIntfActCtx::eRunThread(TThread& thrThis, tCIDLib::TVoid* pData)
{
    // Get the incoming done flag pointer first
    tCIDLib::TBoolean* pbDoneFlag = static_cast<tCIDLib::TBoolean*>(pData);

    // Then we can let the calling thread go
    thrThis.Sync();

    tCIDLib::TBoolean   bDone = kCIDLib::False;
    TCQCIntfAction*     pciaCur = 0;
    TString             strTmp;

    // Remember if we are in remote viewer mode
    const tCIDLib::TBoolean bRemMode(facCQCIntfEng().bRemoteMode());

    //
    //  Check first to see if the action stack is empty. This should not
    //  happen, but don't assume.
    {
        TLocker lockrSync(&m_mtxSync);
        bDone = m_colActStack.bIsEmpty();
    }

    //
    //  We need a GUI cmd event structure to call back to GUI targets. And
    //  we need a temp config object, into which we will put an expanded
    //  version of it. We point the callback object at it as well and use
    //  the same one for commands posted back to the GUI thread.
    //
    tCQCIntfEng::TIntfCmdEv iceCallback;
    TCQCCmdCfg              ccfgTmp;
    iceCallback.pccfgToDo = &ccfgTmp;

    // Remember the count of targets for later use
    const tCIDLib::TCard4 c4TarCount = m_colTargets.c4ElemCount();

    try
    {
        // These won't actually return false, they will throw. This is for the code analyzer
        if (!bCIDAssert(m_pcivOwner != nullptr, L"The owning view is not set")
        ||  !bCIDAssert(m_pctarGlobals != nullptr, L"The global variables target is not set"))
        {
            return tCIDLib::EExitCodes::InternalError;
        }

        //
        //  Initialize all of the targets. This is done here because it has to
        //  be done in the context of the thread that runs the action, as does
        //  the cleanup at the end below.
        //
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4TarCount; c4Index++)
            m_colTargets[c4Index]->CmdTarInitialize(*this);

        // Loop until the action stack is empty
        while (!bDone)
        {
            // Watch for a cancel
            if (thrThis.bCheckShutdownRequest())
            {
                m_eCurState = EStates::DoneCancel;
                break;
            }

            //
            //  Get the object on top of the stack. Be aware that it could be
            //  done now, so loop, popping items off that are done until we
            //  get one that still has commands to do, or we empty the stack.
            //
            //  We have to lock while doing this, but once we get our pointer
            //  out, we are ok. No one else is going to remove any objects
            //  from the stack but us.
            //
            {
                TLocker lockrSync(&m_mtxSync);
                while (!m_colActStack.bIsEmpty())
                {
                    pciaCur = m_colActStack.pobjPeekTop();

                    // If not done, then let's work on this one
                    if (!pciaCur->bAtEnd())
                        break;

                    // It's done, so delete this object
                    m_colActStack.PopTop();
                }

                // If we emptied it, then break out with a success status
                if (m_colActStack.bIsEmpty())
                {
                    m_eCurState = EStates::DoneOk;
                    break;
                }
            }

            // Won't actually ever return false, it will throw, this is for the analyzer
            if (!bCIDAssert(pciaCur != nullptr, L"The next interface action was not set"))
            {
                return tCIDLib::EExitCodes::InternalError;
            }

            //
            //  Get the current opcode for this action. This gets us
            //  a ref to the opcode and it's index.
            //
            const TActOpcode& aocCur = pciaCur->aocCurrent();

            // Ignore comments and disabled commands
            if ((aocCur.m_eOpcode == tCQCKit::EActOps::Comment) || aocCur.m_bDisabled)
            {
                pciaCur->OffsetIndex(1);
                continue;
            }

            //
            //  For the end, we ignore them for now. If we support loops at
            //  some point we have to deal with this potentially being the
            //  end of a loop.
            //
            if (aocCur.m_eOpcode == tCQCKit::EActOps::End)
            {
                // <TBD> Deal with loops here eventually
                pciaCur->OffsetIndex(1);
                continue;
            }

            //
            //  If it's an else, just change our index and go back to the top.
            //  Else is just a jump. It just marks the end of the If portion
            //  of an If/Else and jumps to the End.
            //
            if (aocCur.m_eOpcode == tCQCKit::EActOps::Else)
            {
                pciaCur->OffsetIndex(aocCur.m_i4Ofs);
                continue;
            }

            //
            //  Looks like a keeper, so do the standard prep work on it,
            //  to do token replacement and such.
            //
            //  We give him the runtime value object (which is requird)
            //  and the optional local/global variables pointers.
            //
            //  We get a copy even though we are already using a copy
            //  of the original commands, because we may support looping
            //  later and that would require we do this prep work on
            //  each round (since variables or fields could have changed.)
            //
            ccfgTmp = aocCur.m_ccfgStep;
            facCQCKit().bDoCmdPrep
            (
                ccfgTmp
                , pciaCur->crtsToUse()
                , m_pctarGlobals
                , &pciaCur->ctarLocalVars()
                , strTmp
            );

            // If we have a tracer, pass it the expanded command
            if (m_pcmdtDebug)
                m_pcmdtDebug->ActCmd(aocCur, ccfgTmp, pciaCur->c4Index() + 1);

            const TString& strEventId = pciaCur->strEventId();
            const TString& strUserId = pciaCur->strUserId();
            const TString& strTargetId = ccfgTmp.strTargetId();
            const tCIDLib::TCard4 c4TargetId = ccfgTmp.c4TargetId();

            // Look up the target based on the target id
            MCQCCmdTarIntf* pmctarToCall = pmctarFind(c4TargetId, *pciaCur);

            //
            //  In some applications the view wants to intercept commands
            //  even if they are not normally dispatched in the GUI sort of
            //  way. So ask him and save that flag. Only bother if the
            //  target wasn't found. Otherwise, it's already a GUI based
            //  command.
            //
            //  He may also update the command config as well, so that he can
            //  change info but still let us process it.
            //
            const tCIDLib::TBoolean bIntercepted
            (
                (pmctarToCall != 0)
                && m_pcivOwner->bInterceptCmd(strEventId, strTargetId, ccfgTmp)
            );

            tCIDLib::TBoolean bRes;
            tCQCKit::ECmdRes eCmdRes;
            if (pmctarToCall && !pmctarToCall->bGUIDispatch() && !bIntercepted)
            {
                //
                //  It was not a GUI target and the view didn't want to
                //  intercept it, so we dispatch directly.
                //
                eCmdRes = pmctarToCall->eDoCmd
                (
                    ccfgTmp
                    , pciaCur->c4Index()
                    , strUserId
                    , strEventId
                    , *m_pctarGlobals
                    , bRes
                    , *this
                );
            }
             else
            {
                //
                //  Set up the callback structure and do the send. The
                //  expanded config object is already pointed to. Just in
                //  case it's an intercepted command pass along the target
                //  and intercepted flag.
                //
                iceCallback.bInterceptedCmd = bIntercepted;
                iceCallback.pmctarIntercept = pmctarToCall;

                iceCallback.pacteOwner = this;
                iceCallback.bResult = kCIDLib::False;
                iceCallback.bTarNotFound = kCIDLib::False;
                iceCallback.c4HandlerId = pciaCur->c4HandlerId();
                iceCallback.c4StepInd = pciaCur->c4Index();
                iceCallback.eRes = tCQCKit::ECmdRes::Unknown;
                iceCallback.pstrEventId = &strEventId;
                iceCallback.pstrUserId = &strUserId;
                iceCallback.pctarGlobals = m_pctarGlobals;
                iceCallback.pctarLocals = &pciaCur->ctarLocalVars();
                iceCallback.pcmdtDebug = m_pcmdtDebug;

                //
                //  And now dispatch it. We won't come back until it's
                //  completed.
                //
                m_pcivOwner->DoActEvent(iceCallback);

                // Watch for a target not found
                if (iceCallback.bTarNotFound)
                {
                    facCQCAct().ThrowErr
                    (
                        CID_FILE
                        , CID_LINE
                        , kActErrs::errcCmd_TarNotFound
                        , tCIDLib::ESeverities::Failed
                        , tCIDLib::EErrClasses::NotFound
                        , strUserId
                        , strTargetId
                    );
                }

                // If one of the exception returns, then throw it
                if (iceCallback.eRes != tCQCKit::ECmdRes::Ok)
                {
                    if (iceCallback.eRes == tCQCKit::ECmdRes::Unknown)
                    {
                        // The target never set the status
                        facCQCKit().ThrowErr
                        (
                            CID_FILE
                            , CID_LINE
                            , kKitErrs::errcCmd_CmdResNotSet
                            , tCIDLib::ESeverities::Failed
                            , tCIDLib::EErrClasses::Internal
                        );
                    }
                     else if (iceCallback.eRes == tCQCKit::ECmdRes::Except)
                    {
                        throw iceCallback.errFailure;
                    }
                     else if (iceCallback.eRes == tCQCKit::ECmdRes::UnknownExcept)
                    {
                        facCQCKit().ThrowErr
                        (
                            CID_FILE
                            , CID_LINE
                            , kKitErrs::errcCmd_CmdException
                            , tCIDLib::ESeverities::Failed
                            , tCIDLib::EErrClasses::Unknown
                        );
                    }
                }

                // Not an exception to deal with it normally
                eCmdRes = iceCallback.eRes;
                bRes = iceCallback.bResult;
            }

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
                    pciaCur->OffsetIndex(1);
                }
                 else
                {
                    // It failed, so jump
                    pciaCur->OffsetIndex(aocCur.m_i4Ofs);

                    //
                    //  If debugging, verify that we ended up on an Else
                    //  or End opcode. We are either going to the end of
                    //  an If or While, or jumping to the Else part of an
                    //  If.
                    //
                    #if CID_DEBUG_ON
                    const TActOpcode& aocJumpTo = pciaCur->aocCurrent();
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
                    pciaCur->OffsetIndex(1);
                }
            }
             else
            {
                // Nothing special, so just move to the next step
                pciaCur->OffsetIndex(1);
            }

            // See if the stack is empty or if they asked to stop or exit
            {
                TLocker lockrSync(&m_mtxSync);
                if (eCmdRes == tCQCKit::ECmdRes::Stop)
                {
                    m_eCurState = EStates::DoneCancel;
                    m_colActStack.RemoveAll();
                }
                 else if (eCmdRes == tCQCKit::ECmdRes::Exit)
                {
                    m_eCurState = EStates::DoneExit;
                    m_colActStack.RemoveAll();
                }

                // Break out if so. The state is already set
                if (m_colActStack.bIsEmpty())
                    break;
            }
        }
    }

    catch(TError& errToCatch)
    {
        //
        //  Save the formatted text of the offending step
        //
        //  DO IT BEFORE we flush the stack, which will destroy the
        //  current action object.
        //
        if (pciaCur && !pciaCur->bAtEnd())
            pciaCur->FormatCurOp(m_strStepInErr);

        // Log this error if not already logged and logging warnings
        if (facCQCIntfEng().bLogWarnings() && !errToCatch.bLogged())
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        TLocker lockrSync(&m_mtxSync);

        // Clear the stack, since we are going to give up
        m_colActStack.RemoveAll();

        // Store the exception and return an exception code
        m_errThrown = errToCatch;
        m_eCurState = EStates::DoneExcept;
    }

    catch(...)
    {
        //
        //  Save the formatted text of the offending step
        //
        //  DO IT BEFORE we flush the stack, which will destroy the
        //  current action object.
        //
        if (pciaCur)
            pciaCur->FormatCurOp(m_strStepInErr);

        TLocker lockrSync(&m_mtxSync);

        // Clear the stack, since we are going to give up
        m_colActStack.RemoveAll();

        // Indicate we exited due to an unknown exception
        m_eCurState = EStates::DoneUnknownExcept;
    }

    // Clean up of the targets
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4TarCount; c4Index++)
    {
        try
        {
            m_colTargets[c4Index]->CmdTarCleanup();
        }

        catch(TError& errToCatch)
        {
            if (facCQCIntfEng().bShouldLog(errToCatch))
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }

            if (facCQCIntfEng().bLogFailures())
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
            if (facCQCIntfEng().bLogFailures())
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

    //
    //  Sync with the main thread, and then set our done flag
    //
    TLocker lockrSync(&m_mtxSync);
    *pbDoneFlag = kCIDLib::True;

    return tCIDLib::EExitCodes::Normal;
}


//
//  A helper method to find a particular target by its id, by searching the
//  list of targets. This will only find the standard and 'extra standard'
//  ones, since we don't keep a list of the GUI targets around.
//
MCQCCmdTarIntf*
TCQCIntfActCtx::pmctarFind(const tCIDLib::TCard4 c4Id, TCQCIntfAction& ciaiCur)
{
    const tCIDLib::TCard4 c4Count = m_colTargets.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        MCQCCmdTarIntf* pmctarCur = m_colTargets[c4Index];
        if (pmctarCur->c4UniqueId() == c4Id)
            return pmctarCur;
    }

    // If it's not one of those, see if it's the local variables in the action
    if (ciaiCur.ctarLocalVars().c4UniqueId() == c4Id)
        return &ciaiCur.ctarLocalVars();
    return 0;
}


