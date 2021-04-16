//
// FILE NAME: CQCIGKit_ActTraceWnd.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/19/2006
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
//  This file implements a debug trace output for the CQC action system.
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
RTTIDecls(TActTraceWnd,TFrameWnd)


// ---------------------------------------------------------------------------
//  Local types and contants
// ---------------------------------------------------------------------------
namespace CQCIGKit_ActTraceWnd
{
    namespace
    {
        // -----------------------------------------------------------------------
        //  We come back to the previous position each time. Initially it'll be empty,
        //  which will make us set to the system default size. We set it initially to
        //  the count value to let us know we haven't stored one yet.
        // -----------------------------------------------------------------------
        tCIDCtrls::EPosStates   ePosState = tCIDCtrls::EPosStates::Count;
        TArea                   areaPrevious;
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TActTraceWnd
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TActTraceWnd: Constructors and Destructor
// ---------------------------------------------------------------------------
TActTraceWnd::TActTraceWnd() :

    m_bBreakFlag(kCIDLib::False)
    , m_bGetParms(kCIDLib::False)
    , m_bModal(kCIDLib::False)
    , m_c4InAction(0)
    , m_c4Locked(0)
    , m_colQueue(tCIDLib::EMTStates::Safe)
    , m_enctLast(0)
    , m_pacteBgn(nullptr)
    , m_pcsrcToRun(nullptr)
    , m_pctarGlobals(nullptr)
    , m_strDivider(L"----------------------------------------------------------\n")
    , m_strDivider2(L"TRC: Start Action\n")
    , m_pwndCloseButton(nullptr)
    , m_pwndOwner(nullptr)
    , m_pwndRunButton(nullptr)
    , m_pwndOutput(nullptr)
    , m_tmidUpdate(kCIDCtrls::tmidInvalid)
{
}

TActTraceWnd::~TActTraceWnd()
{
    // If we created an action engine, clean it up now
    if (m_pacteBgn)
    {
        delete m_pacteBgn;
        m_pacteBgn = nullptr;
    }

    // Same for the global variables
    if (m_pctarGlobals)
    {
        delete m_pctarGlobals;
        m_pctarGlobals = nullptr;
    }
}


// ---------------------------------------------------------------------------
//  TActTraceWnd: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TActTraceWnd::ActCmd(const  TActOpcode&     aocOriginal
                    , const TCQCCmdCfg&     ccfgExpanded
                    , const tCIDLib::TCard4 c4Step)
{
    // Format a string and post to the queue
    m_strLine = L"CMD: ";

    // Put a step number on it
    if (c4Step < 10)
        m_strLine.Append(L"00");
    else if (c4Step < 100)
        m_strLine.Append(L"0");
    m_strLine.AppendFormatted(c4Step);
    m_strLine.Append(L". ");

    // Display the elapsed millis and update the last time
    const tCIDLib::TEncodedTime enctCur = TTime::enctNow();
    const tCIDLib::TEncodedTime enctDiff(enctCur - m_enctLast);
    m_enctLast = enctCur;

    m_strLine.Append(L" (");
    if (enctDiff >= kCIDLib::enctOneMilliSec)
    {
        m_strLine.AppendFormatted(tCIDLib::TCard4(enctDiff / kCIDLib::enctOneMilliSec));
        m_strLine.Append(L"ms) - ");
    }
     else
    {
        // Too small for millis, so do microseconds
        m_strLine.AppendFormatted(tCIDLib::TCard4(enctDiff / 10));
        m_strLine.Append(L"us) - ");
    }

    // Ask the opcode to do the long format
    aocOriginal.FormatPrefixTo(m_strLine, kCIDLib::True);
    ccfgExpanded.FormatLongText(m_strLine, kCIDLib::True);
    m_strLine.Append(L"\n");

    // And post it
    QueueLine(m_strLine);
}


tCIDLib::TVoid TActTraceWnd::ActDebug(const TString& strMsg)
{
    m_strLine = L"DBG: ";
    m_strLine.Append(strMsg);
    m_strLine.Append(L"\n\n");
    QueueLine(m_strLine);
}


tCIDLib::TVoid TActTraceWnd::ActEnd(const tCIDLib::TBoolean bStatus)
{
    if (m_c4InAction)
        m_c4InAction--;

    m_strLine = L"TRC: End Action [Result=";
    if (bStatus)
        m_strLine.Append(L"Success]\n");
    else
        m_strLine.Append(L"Failure]\n");
    QueueLine(m_strLine);
}


//
//  Output the data for a new action starting. This must be called from the
//  GUI thread since it directly accesses the window if being called for a
//  top level (initial) action.
//
tCIDLib::TVoid TActTraceWnd::ActStartNew(const TStdVarsTar& ctarGlobals)
{
    //
    //  If starting a new top level action, then clear the window. We also
    //  reset the last command time member.
    //
    if (!m_c4InAction)
    {
        m_pwndOutput->Clear();
        m_enctLast = TTime::enctNow();
    }

    // And now bump up the action nesting counter
    m_c4InAction++;

    QueueLine(m_strDivider);
    QueueLine(m_strDivider2);
    QueueLine(L"\n");
    QueueLine(L"Global Vars=\n");

    // Dump out all the global variables
    tCQCKit::TVarList::TCursor cursDump(ctarGlobals.cursVars());
    if (cursDump.bReset())
    {
        TString strVar;
        do
        {
            const TCQCActVar& varCur = cursDump.objRCur();
            strVar = L"    ";
            strVar.Append(varCur.strName());
            strVar.Append(L"=");
            strVar.Append(varCur.strValue());
            strVar.Append(L"\n");

            QueueLine(strVar);
        }   while (cursDump.bNext());
    }

    QueueLine(L"\n");
}


tCIDLib::TVoid TActTraceWnd::ActVarRemoved(const TString& strKey)
{
    m_strLine = L"VAR: Removed variable '";
    m_strLine.Append(strKey);
    m_strLine.Append(L"'\n\n");
    QueueLine(m_strLine);
}


tCIDLib::TVoid
TActTraceWnd::ActVarSet(const   TString&    strKey
                        , const TString&    strValue)
{
    m_strLine = L"VAR: Set variable '";
    m_strLine.Append(strKey);
    m_strLine.Append(L"'\n       =");
    m_strLine.Append(strValue);
    m_strLine.Append(L"\n\n");
    QueueLine(m_strLine);
}


// Get/set the locked value
tCIDLib::TBoolean TActTraceWnd::bLocked() const
{
    return (m_c4Locked != 0);
}

tCIDLib::TBoolean TActTraceWnd::bLocked(const tCIDLib::TBoolean bToSet)
{
    if (bToSet)
    {
        m_c4Locked++;
    }
     else
    {
        if (m_c4Locked)
        {
            m_c4Locked--;
        }
         else
        {
            // Can't do much, queue up an underflow error message
            QueueLine(TString(L"ERR: Tracer lock underflow!"));
        }
    }
    return (m_c4Locked != 0);
}


// ---------------------------------------------------------------------------
//  TActTraceWnd: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Just returns our in action flag if the outside world wants to know
tCIDLib::TBoolean TActTraceWnd::bInAction() const
{
    return (m_c4InAction > 0);
}


//
//  Clear the window. We flush the queue just in case, and then clear the windwo
//  and move to the end.
//
tCIDLib::TVoid TActTraceWnd::ClearOutput()
{
    m_colQueue.RemoveAll();
    m_pwndOutput->Clear();
}


//
//  Invoke us as a top level secondary frame, which is plugged into other
//  actions engines by the caller. In this case we are a passive receiver
//  of action output.
//
tCIDLib::TVoid TActTraceWnd::CreateWindow(const TString& strIconName)
{
    CIDAssert(!bIsValid(), L"The action trace window already has been created");

    // Remember we are not being invoked modally
    m_bModal = kCIDLib::False;

    //
    //  Make sure the modal invocation stuff is all zeroed, out, especially
    //  the owner window since that is used as a flag in the rest of the
    //  code to indicate whether we are in modal or top level secondary
    //  frame mode.
    //
    m_pwndOwner = nullptr;
    m_pcsrcToRun = nullptr;
    m_pcuctxToUse = nullptr;
    m_strActName.Clear();
    m_strEventId.Clear();

    // Load our dialog resource so we can get the size
    TDlgDesc dlgdChildren;
    facCQCIGKit().bCreateDlgDesc(kCQCIGKit::ridDlg_ActTrace, dlgdChildren);

    // If we have no previous position, take a default origin, but the defined size
    if (CQCIGKit_ActTraceWnd::ePosState == tCIDCtrls::EPosStates::Count)
    {
        CQCIGKit_ActTraceWnd::areaPrevious = facCIDCtrls().areaDefWnd();
        CQCIGKit_ActTraceWnd::areaPrevious.SetSize(dlgdChildren.areaPos().szArea());
    }

    // Call our parent to create the window
    CreateFrame
    (
        m_pwndOwner
        , CQCIGKit_ActTraceWnd::areaPrevious
        , L"CQC Action Tracer"
        , tCIDCtrls::EWndStyles::HelperFrameNV
        , tCIDCtrls::EExWndStyles::None
        , tCIDCtrls::EFrameStyles::StdFrame
        , kCIDLib::False
    );

    // Restore any previous position state if we have one
    if (CQCIGKit_ActTraceWnd::ePosState != tCIDCtrls::EPosStates::Count)
    {
        RestorePosState
        (
            CQCIGKit_ActTraceWnd::ePosState, CQCIGKit_ActTraceWnd::areaPrevious, kCIDLib::True
        );
    }

    //
    //  Set our icon. Normally this is done using the executable's facility since the
    //  icon is in the same directory as it. But, in our case, we have to do it for our
    //  self, but we know this facility is in the same directory as the executable that
    //  is using us. The application tells us which icon to us.
    //
    facCQCIGKit().SetFrameIcon(*this, strIconName);

    // Now make it visible
    SetVisibility(kCIDLib::True);
}


//
//  Invoke us as a modal popup. In this case, we get a command source to
//  invoke the actions of and we create a Run button that lets the user
//  invoke the action. We create our own internal action engine and point
//  it as us as the debugger interface so that we can display the
//  trace output.
//
//  In this case we run as a modal window and won't come back until the
//  the user closes the window.
//
tCIDLib::TVoid
TActTraceWnd::RunModally(const  TWindow&            wndOwner
                        , const MCQCCmdSrcIntf&     csrcToRun
                        , const TCQCUserCtx&        cuctxToUse
                        , const TString&            strActionName
                        , const TString&            strEventId
                        , const tCIDLib::TBoolean   bGetParms
                        , const TString&            strIconName)
{
    CIDAssert(!bIsValid(), L"The action trace window already has been created");

    // Remember we are being invoked modally and if we need to get parameters
    m_bModal = kCIDLib::True;
    m_bGetParms = bGetParms;

    //
    //  Store the incoming info since we are being invokved as a modal
    //  popup and we need to store this for later use in invoking the
    //  action and such.
    //
    m_pcsrcToRun = &csrcToRun;
    m_pcuctxToUse = &cuctxToUse;
    m_pwndOwner = &wndOwner;
    m_strActName = strActionName;
    m_strEventId = strEventId;

    // Load our dialog resource so we can get the size
    TDlgDesc dlgdChildren;
    facCQCIGKit().bCreateDlgDesc(kCQCIGKit::ridDlg_ActTrace, dlgdChildren);

    // If we have no previous position, take a default origin, but the defined size
    if (CQCIGKit_ActTraceWnd::areaPrevious.bIsEmpty())
    {
        CQCIGKit_ActTraceWnd::areaPrevious = facCIDCtrls().areaDefWnd();
        CQCIGKit_ActTraceWnd::areaPrevious.SetSize(dlgdChildren.areaPos().szArea());
    }

    // Create a title string
    const TString strTitle(L"Testing Action: " + m_strActName);

    // Call our parent to create the window
    CreateFrame
    (
        &wndOwner
        , CQCIGKit_ActTraceWnd::areaPrevious
        , strTitle
        , tCIDCtrls::EWndStyles::HelperFrameNV
        , tCIDCtrls::EExWndStyles::None
        , tCIDCtrls::EFrameStyles::StdFrame
        , kCIDLib::False
    );

    //
    //  Set our icon. Normally this is done using the executable's facility since the
    //  icon is in the same directory as it. But, in our case, we have to do it for our
    //  self, but we know this facility is in the same directory as the executable that
    //  is using us. The application tells us which icon to us.
    //
    facCQCIGKit().SetFrameIcon(*this, strIconName);

    // Restore any previous position state if we have one, else just show it
    if (CQCIGKit_ActTraceWnd::ePosState != tCIDCtrls::EPosStates::Count)
    {
        RestorePosState
        (
            CQCIGKit_ActTraceWnd::ePosState, CQCIGKit_ActTraceWnd::areaPrevious, kCIDLib::True
        );
    }
    else
    {
        SetVisibility(kCIDLib::True);
    }

    Activate();

    //
    //  And let's do a modal loop now. We clear the break flag befoe we
    //  enter. The Close button when we are on this mode will set this
    //  flag to make us exit.
    //
    m_bBreakFlag = kCIDLib::False;
    eDoModalLoop(m_bBreakFlag, tCIDCtrls::EModalFlags::None);

    //
    //  Destroy ourself, since each modal invocation is a new one that
    //  creates the window again.
    //
    Destroy();
}


// ---------------------------------------------------------------------------
//  TActTraceWnd: Protected, inherited methods
// ---------------------------------------------------------------------------

// We need to make ourself the active top window if getting activated
tCIDLib::TVoid
TActTraceWnd::ActivationChange( const   tCIDLib::TBoolean       bState
                                , const tCIDCtrls::TWndHandle   hwndOther)
{
    TParent::ActivationChange(bState, hwndOther);
    if (bState)
        facCIDCtrls().SetActiveTopWnd(this);
}


tCIDLib::TVoid
TActTraceWnd::AreaChanged(  const   TArea&                  areaPrev
                            , const TArea&                  areaNew
                            , const tCIDCtrls::EPosStates   ePosState
                            , const tCIDLib::TBoolean       bOrgChanged
                            , const tCIDLib::TBoolean       bSizeChanged
                            , const tCIDLib::TBoolean       bStateChanged)
{
    // Call our parent first
    TParent::AreaChanged(areaPrev, areaNew, ePosState, bOrgChanged, bSizeChanged, bStateChanged);

    // If the size changed, and we have created children, and aren't minimized
    if (bSizeChanged && m_pwndOutput && (ePosState != tCIDCtrls::EPosStates::Minimized))
        AutoAdjustChildren(areaPrev, areaNew);
}


//
//  This is a top level window. If we don't override this, we will be forced out of
//  the message loop. So we just destroy ourself and return false. However, we have to
//  be sure the action has ended in this case, since the X is not disabled while it
//  runs.
//
//  But, if we are modal, we just set the modal break flag. We will be destroyed upon
//  exiting the modal loop.
//
tCIDLib::TBoolean TActTraceWnd::bAllowShutdown()
{
    if (m_bModal)
    {
        m_bBreakFlag = kCIDLib::True;
    }
     else
    {
        if (!m_c4Locked)
            Destroy();
    }
    return kCIDLib::False;
}


tCIDLib::TBoolean TActTraceWnd::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRet = TParent::bCreated();

    // Load our dialog resource
    TDlgDesc dlgdChildren;
    facCQCIGKit().bCreateDlgDesc(kCQCIGKit::ridDlg_ActTrace, dlgdChildren);

    //
    //  If we are not modal then we don't want to have a run button, so remove it. Get
    //  its position first and move the close button there.
    //
    if (!m_bModal)
    {
        const TArea areaSave = dlgdChildren.dlgiFindById(kCQCIGKit::ridDlg_ActTrace_Run).areaPos();
        dlgdChildren.RemoveItem(kCQCIGKit::ridDlg_ActTrace_Run);
        TDlgItem& dlgiCancel = dlgdChildren.dlgiFindById(kCQCIGKit::ridDlg_ActTrace_Close);
        dlgiCancel.areaPos(areaSave);
    }

    // OK, create the controls now
    tCIDLib::TCard4 c4InitFocus;
    CreateDlgItems(dlgdChildren, c4InitFocus);

    // Set our initial size as the minimim size
    SetMinMaxSize(dlgdChildren.areaPos().szArea(), TSize());

    // Do an initial auto-adjust to fit them to us
    AutoAdjustChildren(dlgdChildren.areaPos(), areaClient());

    // Get pointers to our children
    CastChildWnd(*this, kCQCIGKit::ridDlg_ActTrace_Close, m_pwndCloseButton);
    CastChildWnd(*this, kCQCIGKit::ridDlg_ActTrace_Output, m_pwndOutput);

    // Register our button handlers. Only do run if it still exists
    if (m_bModal)
    {
        CastChildWnd(*this, kCQCIGKit::ridDlg_ActTrace_Run, m_pwndRunButton);
        m_pwndRunButton->pnothRegisterHandler(this, &TActTraceWnd::eClickHandler);
    }
    m_pwndCloseButton->pnothRegisterHandler(this, &TActTraceWnd::eClickHandler);

    // Start up the timer
    m_tmidUpdate = tmidStartTimer(100);

    return bRet;
}


// We need to clean up our child controls if they got created
tCIDLib::TVoid TActTraceWnd::Destroyed()
{
    // Diable our timer in case it got started
    if (m_tmidUpdate != kCIDCtrls::tmidInvalid)
        StopTimer(m_tmidUpdate);

    // Clean up the queue
    m_colQueue.RemoveAll();

    //
    //  Update our previous area value and we also store the pos state so we
    //  can go back to that. This will get us the restore position if we are in min
    //  or max state, which is what we want.
    //
    CQCIGKit_ActTraceWnd::ePosState = ePosState(CQCIGKit_ActTraceWnd::areaPrevious);

    // We cannot any longer be the active top level window
    if (this == facCIDCtrls().pwndActiveTopWnd())
        facCIDCtrls().SetActiveTopWnd(nullptr);
}



// If it's our timer, we pull out items from the queue till it's empty
tCIDLib::TVoid TActTraceWnd::Timer(const tCIDCtrls::TTimerId tmidToDo)
{
    if (tmidToDo == m_tmidUpdate)
    {
        //
        //  Default is not to throw on timeout, which is what we want. Use a
        //  small timeout so that we basically just get what's there already.
        //
        while (m_colQueue.bGetNextMv(m_strLine, 2))
            m_pwndOutput->AddString(m_strLine);
    }
}


// ---------------------------------------------------------------------------
//  TActTraceWnd: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  We just have to respond to the close button and destroy ourself. But we refuse to do
//  it if we are curently locked.
//
tCIDCtrls::EEvResponses TActTraceWnd::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCIGKit::ridDlg_ActTrace_Close)
    {
        if (m_bModal)
        {
            // We are in the modal loop mode, so just force the loop to exit
            m_bBreakFlag = kCIDLib::True;
        }
         else
        {
            //
            //  We are a separate top level window. If the action has ended, then just
            //  destroy ourself.
            //
            if (!m_c4Locked)
                Destroy();
        }
    }
     else if (wnotEvent.widSource() == kCQCIGKit::ridDlg_ActTrace_Run)
    {
        //
        //  We need to invoke the action. First, if we've not created our
        //  action engine yet, do that.
        //
        if (!m_pacteBgn)
        {
            const TString strTitle(L"Test Action - " + m_strActName);
            m_pacteBgn = new TCQCStdGUIActEngine(this, this, strTitle, *m_pcuctxToUse);
        }

        // If we haven't created the global actions target, then do that
        if (!m_pctarGlobals)
            m_pctarGlobals = new TStdVarsTar(tCIDLib::EMTStates::Safe, kCIDLib::False);

        // Get any parameters that the action might require
        TString strNewParams;
        if (m_bGetParms)
        {
            const TString strInstruct(kIGKitMsgs::midDlg_ActTrace_TestInstruct, facCQCIGKit());
            const TString strTitle(kIGKitMsgs::midDlg_ActTrace_EnterTestParams, facCQCIGKit());

            const tCIDLib::TBoolean bRes = facCIDWUtils().bGetText2
            (
                *this
                , strTitle
                , strInstruct
                , m_strPrevTestParams
                , strNewParams
                , TString::strEmpty()
            );

            // If they cancel, then do nothing
            if (!bRes)
                return tCIDCtrls::EEvResponses::Handled;

            // Remember these parameters as new previous ones
            m_strPrevTestParams = strNewParams;
        }

        //
        //  Call the new action prep method, which is normally called by the outside
        //  world, but we do it in this case since we are running the action.
        //
        ActStartNew(*m_pctarGlobals);

        //
        //  And now let's start it processing. We create a temporary global variables
        //  object for this run. We don't need any extra targets, so pass in an empty
        //  list.
        //
        //  This starts off the background thread processing. When we see the end of
        //  the action notification, we'll re-enable the buttons.
        //
        tCQCKit::TCmdTarList colExtraTars(tCIDLib::EAdoptOpts::NoAdopt);
        m_pacteBgn->eInvokeOpsInBgn
        (
            *m_pcsrcToRun
            , m_strEventId
            , *m_pctarGlobals
            , colExtraTars
            , this
            , strNewParams
        );
    }
    return tCIDCtrls::EEvResponses::Handled;
}


//
//  Handles queuing up a line and handleing any exception.
//
tCIDLib::TVoid TActTraceWnd::QueueLine(const TString& strToQ)
{
    //
    //  Start throttling if the queue gets really big, and drop them if
    //  nothing gets removed from the queue while we wait.
    //
    if (m_colQueue.bWaitForSpace(100, 2048))
    {
        try
        {
            m_colQueue.objAdd(strToQ);
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
    }
}
