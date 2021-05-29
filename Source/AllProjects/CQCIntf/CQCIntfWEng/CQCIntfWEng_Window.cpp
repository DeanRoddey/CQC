//
// FILE NAME: CQCIntfWEng_Window.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/06/2009
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
//  This file implements the TCQCIntfWView class, which provides a display
//  mechanism for user drawn interfaces onto a standard window.
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
#include    "CQCIntfWEng_.hpp"


// ---------------------------------------------------------------------------
//  Magic Macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCIntfViewWnd,TouchWnd)


// ---------------------------------------------------------------------------
//  Local constants and types
// ---------------------------------------------------------------------------
namespace
{
    namespace CQCIntfWEng_Window
    {
        // -----------------------------------------------------------------------
        //  A code we post to ourself when we receive a request to process an
        //  action command, so that the processing can occur in the context of
        //  the GUI thread.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard4   c4CmdAsyncId = 1000;
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfViewWnd
//  PREFIX: wnd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfViewWnd: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfViewWnd::TCQCIntfViewWnd() :

    m_c4PauseTimers(0)
    , m_pcivTarget(0)
    , m_strTmplId(TUniqueId::strMakeId())
    , m_tmidActiveUp(kCIDLib::c4MaxCard)
    , m_tmidEvents(kCIDLib::c4MaxCard)
    , m_tmidValueUp(kCIDLib::c4MaxCard)
{
}

TCQCIntfViewWnd::~TCQCIntfViewWnd()
{
    try
    {
        delete m_pcivTarget;
    }

    catch(const TError& errToCatch)
    {
        if (facCQCIntfWEng().bShouldLog(errToCatch))
            TModule::LogEventObj(errToCatch);
    }
}


// ---------------------------------------------------------------------------
//  TCQCIntfViewWnd: Public, inherited methods
// ---------------------------------------------------------------------------
//
//  If the view is set, we pass this gesture event on to it, who will pass it
//  to the correct widget, if any.
//
tCIDLib::TBoolean
TCQCIntfViewWnd::bProcessGestEv(const   tCIDCtrls::EGestEvs eEv
                                , const TPoint&             pntStart
                                , const TPoint&             pntAt
                                , const TPoint&             pntDelta
                                , const tCIDLib::TBoolean   bTwoFinger)
{
    tCIDLib::TBoolean bContinue = kCIDLib::False;

    if (m_pcivTarget && ! m_pcivTarget->bActiveAction())
    {
        try
        {
            bContinue = m_pcivTarget->bProcessGestEv
            (
                eEv, pntStart, pntAt, pntDelta, bTwoFinger
            );
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);

            facCQCGKit().ShowError
            (
                *this
                , L"IV Gesture handler"
                , L"An error occurred during gesture handling."
                , errToCatch
            );
        }

        catch(...)
        {
            TErrBox msgbErr
            (
                L"IV Gesture handler"
                , TString("A system error occurred during gesture handling.")
            );
            msgbErr.ShowIt(*this);
        }
    }
    return bContinue;
}



//
//  We override this to suppress background erasing. Our display is purely
//  double buffered, and the bitmap we draw into covers the whole window
//  area. So this is a waste of time and increases blinkies.
//
tCIDLib::TBoolean TCQCIntfViewWnd::bEraseBgn(TGraphDrawDev& gdevTarget)
{
    return kCIDLib::True;
}


//
//  Ask the view to buffer all the content under the update area, then
//  we blit that to the screen.
//
tCIDLib::TBoolean
TCQCIntfViewWnd::bPaint(TGraphDrawDev& gdevToUse, const TArea& areaUpdate)
{
    if (m_pcivTarget)
    {
        //
        //  Get all the stuff under the update area and blit to the screen. Howevre
        //  we have to deal with the fact that the content may be smaller than the
        //  window as we are sized up. If so, then the bitmap will not provide all
        //  of the area fill.
        //
        TBitmap& bmpUpdate = m_pcivTarget->bmpQueryContentUnder(areaUpdate);
        gdevToUse.DrawBitmap(bmpUpdate, areaUpdate, areaUpdate);

        //
        //  Get our size and see if the bitmap falls short on the left/right. If
        //  so fill those areas with black.
        //
        TArea areaBmp(TPoint::pntOrigin, bmpUpdate.szBitmap());

        if ((areaBmp.i4Right() < areaUpdate.i4Right())
        ||  (areaBmp.i4Bottom() < areaUpdate.i4Bottom()))
        {
            TGUIRegion grgnUpdate(areaUpdate);
            grgnUpdate.CombineWith(areaBmp, tCIDGraphDev::EClipModes::Diff);

            gdevToUse.Fill(grgnUpdate, facCIDGraphDev().rgbBlack);
        }
    }

    return kCIDLib::True;
}




//
//  In order to support GUI based programs invoking actions (which run in
//  a background thread), we have to allow a special GUI enabled action engine
//  to send us commands that are targetd towards interface widget targets.
//  He does a synchronous send and blocks till we process the command and
//  return. The data is passed in and out via a structure he provides.
//
//
//  !!!!!!
//
//  We definitely do NOT want to let any exception propogate out of here. If
//  we throw, it propogates back on the GUI thread, and not back to the caller
//  that would otherwise be expected to handle it, and he will remain blocked
//  forever waiting for us to return. We catch the exceptions and return it
//  via the provided I/O structure.
//
tCIDLib::TVoid
TCQCIntfViewWnd::DataReceived(  const   tCIDLib::TCard4         c4DataId
                                ,       tCIDLib::TVoid* const   pData)
{
    if (c4DataId == CQCIntfWEng_Window::c4CmdAsyncId)
    {
        // Cast the data to its actual type
        tCQCIntfEng::TIntfCmdEv* piceToDo
        (
            static_cast<tCQCIntfEng::TIntfCmdEv*>(pData)
        );

        // Default the result to OK until some failure occurs
        piceToDo->eRes = tCQCKit::ECmdRes::Ok;

        try
        {
            CIDAssert(m_pcivTarget != nullptr, L"The target view has not been initialized");

            CIDAssert(m_pcivTarget != nullptr, L"The target interface view was not set");
            CIDAssert(piceToDo->pstrUserId != nullptr, L"The user id was not set");
            CIDAssert(piceToDo->pstrEventId != nullptr, L"The event id was not set");
            CIDAssert(piceToDo->pctarGlobals != nullptr, L"The global vars was not set");
            CIDAssert(piceToDo->pctarLocals != nullptr, L"The local vars was not set");

            //
            //  Get the template out that has to contain source and target for this
            //  action, though it may be either one of those as well.
            //
            TCQCIntfTemplate& iwdgTmpl = m_pcivTarget->iwdgAt(piceToDo->c4HandlerId);

            // Get a ref tot he command we are to do
            const TCQCCmdCfg& ccfgToDo = *piceToDo->pccfgToDo;

            // Find the target id. If we find one, then tell him to do the command
            TCQCIntfWidget* piwdgTar = iwdgTmpl.piwdgFindByUID(ccfgToDo.c4TargetId(), kCIDLib::True);

            // If we didn't find it, it could be the view itself, so check that.
            MCQCCmdTarIntf* pmctarCur = nullptr;
            if (piwdgTar)
                pmctarCur = dynamic_cast<MCQCCmdTarIntf*>(piwdgTar);
             else if (m_pcivTarget->c4UniqueId() == ccfgToDo.c4TargetId())
                pmctarCur = m_pcivTarget;

            if (pmctarCur)
            {
                piceToDo->eRes = pmctarCur->eDoCmd
                (
                    ccfgToDo
                    , piceToDo->c4StepInd
                    , *piceToDo->pstrUserId
                    , *piceToDo->pstrEventId
                    , *piceToDo->pctarGlobals
                    , piceToDo->bResult
                    , *piceToDo->pacteOwner
                );
            }
             else
            {
                piceToDo->bTarNotFound = kCIDLib::True;
            }
        }

        //  We send the exception info back to the caller via his provided
        //  event structure.
        //
        catch(const TError& errToCatch)
        {
            piceToDo->eRes = tCQCKit::ECmdRes::Except;
            piceToDo->errFailure = errToCatch;
        }

        catch(...)
        {
            piceToDo->eRes = tCQCKit::ECmdRes::UnknownExcept;
        }
    }
     else
    {
        // Don't throw, just ignore. See method comments above
    }
}


// We need to clean up our timers and release any tracking we are doing
tCIDLib::TVoid TCQCIntfViewWnd::Destroyed()
{
    try
    {
        // Close down the timers
        StopTimers();

        // Clean up the view
        if (m_pcivTarget)
            m_pcivTarget->Destroyed();

        // And finally pass it on to our parent to complete the process
        TParent::Destroyed();
    }

    catch(TError& errToCatch)
    {
        if (!errToCatch.bLogged() && facCQCIntfWEng().bLogInfo())
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
    }
}


//
//  If the view is set, we pass this to him. He will find the appropriate
//  widget and let it know its been clicked.
//
//  We have to make sure we aren't already busy doing an action though,
//  and that we aren't in no-input mode.
//
tCIDLib::TVoid TCQCIntfViewWnd::GestClick(const TPoint& pntAt)
{
    if (m_pcivTarget && !facCQCIntfWEng().bNoInputMode())
    {
        if (m_pcivTarget->bActiveAction())
        {
            const TSize szHalo(98, 86);
            TPoint pntOrg(pntAt);
            pntOrg.Adjust(-49, -43);

            const TArea areaTar(pntOrg, szHalo);
            TArea areaSrc(TPoint::pntOrigin, szHalo);
            TGraphWndDev gdevThis(*this);
            gdevThis.AlphaBlit
            (
                m_bmpBusyHalo, areaSrc, areaTar, 0x60, kCIDLib::True
            );
            TThread::Sleep(200);
            Redraw(areaTar, tCIDCtrls::ERedrawFlags::ImmedErase);
        }
         else
        {
            try
            {
                m_pcivTarget->Clicked(pntAt);
            }

            catch(TError& errToCatch)
            {
                m_pcivTarget->ShowErr
                (
                    L"IV Click handler"
                    , errToCatch.strErrText()
                    , errToCatch
                );
            }

            catch(...)
            {
                TErrBox msgbErr
                (
                    L"IV Click handler"
                    , TString("A system error occurred during click handling.")
                );
                msgbErr.ShowIt(*this);
            }
        }
    }
}


//
//  If there's a view set, pass this on to him. He will use it to let any current
//  gesture target widget do drawing ahead in between drag movement, so that it can
//  stay ahead of the user, laying down the tracks in front of the train.
//
tCIDLib::TVoid
TCQCIntfViewWnd::GestInertiaIdle(const  tCIDLib::TEncodedTime   enctEnd
                                , const tCIDLib::TCard4         c4Millis
                                , const tCIDLib::EDirs          eDir)
{
    if (m_pcivTarget)
    {
        try
        {
            m_pcivTarget->GestInertiaIdle(enctEnd, c4Millis, eDir);
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);

            TErrBox msgbErr
            (
                L"IV Gesture handler"
                , TString("An error occurred during gesture idle mode.")
            );
            msgbErr.ShowIt(*this);
        }

        catch(...)
        {
            TErrBox msgbErr
            (
                L"IV Gesture handler"
                , TString("A system error occurred during gesture idle mode.")
            );
            msgbErr.ShowIt(*this);
        }
    }
}


//
//  The gest was reset, either at the end or potentially cancelled. Currently we
//  don't need to do anything with this. We still get the end gesture event and
//  that's enough for us.
//
tCIDLib::TVoid TCQCIntfViewWnd::GestReset(const tCIDCtrls::EGestReset eType)
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfViewWnd: Public, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TCQCIntfViewWnd::bInitialize(const  TString&                    strTitle
                            ,       MCQCIntfAppHandler* const   pmiahToUse
                            , const TCQCUserCtx&                cuctxToUse)
{
    CIDAssert(m_pcivTarget == nullptr, L"Multiple calls to view window init");

    try
    {
        //
        //  We have to provide the view with the lat/long info for later use in
        //  event processing.
        //
        tCIDLib::TFloat8 f8Lat = 0.0;
        tCIDLib::TFloat8 f8Long = 0.0;
        {
            if (!facCQCKit().bGetLocationInfo(tCIDLib::ECSSides::Server, f8Lat, f8Long, kCIDLib::False, cuctxToUse.sectUser()))
            {
                TErrBox msgbErr
                (
                    L"Viewer Initializiation Failed"
                    , facCQCKit().strMsg(kKitErrs::errcCfg_NoLocInfo, TString(L"Server"))
                );
                msgbErr.ShowIt(*this);
                return kCIDLib::False;
            }
        }

        // Just allocate up our interface view, and pass him a pointer to us
        m_pcivTarget = new TCQCIntfWView
        (
            this, strTitle, pmiahToUse, f8Lat, f8Long, cuctxToUse
        );
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }

    // Start up the timers
    StartTimers();

    return kCIDLib::True;
}

tCIDLib::TBoolean
TCQCIntfViewWnd::bInitialize(       TWindow&                    wndParent
                            ,       TDataSrvClient&             dsclToUse
                            ,       tCQCIntfEng::TErrList&      colErrs
                            , const TString&                    strTitle
                            ,       MCQCIntfAppHandler* const   pmiahToUse
                            , const TCQCIntfTemplate&           iwdgTemplate
                            , const TCQCUserCtx&                cuctxToUse)
{
    CIDAssert(m_pcivTarget == nullptr, L"Multiple calls to view window init");

    //
    //  We have to provide the view with the lat/long info for later use in
    //  event processing.
    //
    tCIDLib::TFloat8 f8Lat = 0.0;
    tCIDLib::TFloat8 f8Long = 0.0;
    {
        if (!facCQCKit().bGetLocationInfo(tCIDLib::ECSSides::Server, f8Lat, f8Long, kCIDLib::False, cuctxToUse.sectUser()))
        {
            TErrBox msgbErr
            (
                L"Viewer Initializiation Failed"
                , facCQCKit().strMsg(kKitErrs::errcCfg_NoLocInfo, TString(L"Server"))
            );
            msgbErr.ShowIt(wndParent);
            return kCIDLib::False;
        }
    }

    //
    //  Aallocate up our interface view, and pass him a pointer to us, and
    //  the incoming template.
    //
    m_pcivTarget = new TCQCIntfWView
    (
        iwdgTemplate, this, strTitle, pmiahToUse, f8Lat, f8Long, cuctxToUse
    );

    // Initialize it so it'll load up this initial template
    const tCIDLib::TBoolean bRet =  m_pcivTarget->bInitialize(dsclToUse, colErrs);

    // Start up the timers
    StartTimers();
    return bRet;
}


//
//  This is called to get us to reload with new template contents. We just
//  pass it on to the view as though we are loading initially.
//
tCIDLib::TBoolean
TCQCIntfViewWnd::bReload(       TWindow&                wndParent
                        ,       TDataSrvClient&         dsclToUse
                        ,       tCQCIntfEng::TErrList&  colErrs
                        , const TCQCIntfTemplate&       iwdgTemplate)
{
    if (!m_pcivTarget)
        return kCIDLib::True;

    const tCIDLib::TBoolean bRes = m_pcivTarget->bLoadNewTemplate
    (
        dsclToUse, iwdgTemplate, colErrs
    );

    // If it works, but there are errors, then display them
    if (bRes && !colErrs.bIsEmpty())
    {
    }

    return kCIDLib::True;
}


// Provide access to our view
const TCQCIntfWView& TCQCIntfViewWnd::civTarget() const
{
    CIDAssert(m_pcivTarget != nullptr, L"The target view has not be initilized");
    return *m_pcivTarget;
}

TCQCIntfWView& TCQCIntfViewWnd::civTarget()
{
    CIDAssert(m_pcivTarget != nullptr, L"The target view has not be initilized");
    return *m_pcivTarget;
}


tCIDLib::TVoid
TCQCIntfViewWnd::Create(        TWindow&            wndParent
                        , const tCIDCtrls::TWndId   widToUse
                        , const TArea&              areaInit)
{
    // Pass it on to our parent class, providing the remaining info
    CreateWnd
    (
        wndParent.hwndSafe()
        , kCIDCtrls::pszCustClass
        , L""
        , areaInit
        , tCIDCtrls::EWndStyles::ClippingChild
        , tCIDCtrls::EExWndStyles::ControlParent
        , widToUse
    );

    // Take the focus so that the frame remembers us as his focus window
    TakeFocus();
}


//
//  We have to tell the view if the display resolution changes, so that he
//  and resize his double buffering bitmap. The containing app calls this,
//  when he gets a notification from the system in his main window.
//
tCIDLib::TVoid TCQCIntfViewWnd::DisplayResChanged(const TSize& szNew)
{
    // Just pass it on the view
    if (m_pcivTarget)
        m_pcivTarget->DisplayResChanged(szNew);
}


//
//  The display area size changed, so tell the view. We don't do this ourself,
//  this is called when the containing frame is resized, actually just once
//  when it's completed the resize opreation.
//
tCIDLib::TVoid TCQCIntfViewWnd::NewSize(const TSize& szNew)
{
    // Resize ourself, and if we have a view pass it on to him
    if (m_pcivTarget)
        m_pcivTarget->NewSize(szNew);
    SetSize(szNew, kCIDLib::True);
}


// Pause/unpause our timer callbacks to the view
tCIDLib::TVoid TCQCIntfViewWnd::PauseTimers(const tCIDLib::TBoolean bPauseState)
{
    if (bPauseState)
    {
        m_c4PauseTimers++;
    }
     else
    {
        CIDAssert(m_c4PauseTimers != 0, L"View window timer pause underflow");
        m_c4PauseTimers--;
    }
}


//
//  This is called from our related window based view class. This is called
//  when it looks like the target widget is not one of the standard ones, so
//  it's probably a widget. So the action engine passes it on to the view, who
//  passes it to us. We are being called on the action engine thread so we
//  need to let the window thread process this guy.
//
//  So we do a synchronous post to ourself. The DataReceived() method below
//  will see that and process the command in the GUI thread context.
//
tCIDLib::TVoid
TCQCIntfViewWnd::ProcessActEvent(tCQCIntfEng::TIntfCmdEv& iceToDo)
{
    // Do a sync send to ourself
    SendData(CQCIntfWEng_Window::c4CmdAsyncId, &iceToDo, tCIDLib::ESyncTypes::Synchronous);
}


// Provides access to the generated template id that identifies us at runtime
const TString& TCQCIntfViewWnd::strTmplId() const
{
    return m_strTmplId;
}


tCIDLib::TVoid TCQCIntfViewWnd::ScrPosChanged()
{
    if (m_pcivTarget)
    {
        // Get our own window area in screen coordinates
        TArea areaScr;
        wndParent().ToScreenCoordinates(areaWnd(), areaScr);

        m_pcivTarget->ScrPosChanged(areaScr);
    }

}


//
//  Let's the viewer update the activity stamp on our view. Mostly this is for when
//  the blanker is dismissed by the user touching the screen. That touch doesn't come
//  to us, it goes to the blanker, so the view will call this to make sure the stamp
//  gets updated. Else we could just immediatley blank again.
//
tCIDLib::TVoid TCQCIntfViewWnd::UpdateLastActivityStamp()
{
    if (m_pcivTarget)
        m_pcivTarget->UpdateLastActivityStamp();
}



// ---------------------------------------------------------------------------
//  TCQCIntfViewWnd: Protected, inherited methods
// ---------------------------------------------------------------------------

//
//  We do minimal redraw here in the size operation. So we'll re-validate the area that
//  gets invalidated (if sizing up) and therefore no paint will occur.
//
//  Note that we don't tell the view about this size change, other than to cause
//  a redraw. We only tell it about a size change at the end when it is complete,
//  via NewSize() above.
//
//  We pass along origin changes, mostly to support those widgets that actually control
//  an external helper process.
//
tCIDLib::TVoid
TCQCIntfViewWnd::AreaChanged(const  TArea&                  areaPrev
                            , const TArea&                  areaNew
                            , const tCIDCtrls::EPosStates   ePosState
                            , const tCIDLib::TBoolean       bOrgChanged
                            , const tCIDLib::TBoolean       bSizeChanged
                            , const tCIDLib::TBoolean       bStateChanged)
{
    TParent::AreaChanged(areaPrev, areaNew, ePosState, bOrgChanged, bSizeChanged, bStateChanged);

    if (bIsVisible() && (ePosState != tCIDCtrls::EPosStates::Restored))
    {
        if (bSizeChanged)
        {
            MinimalRedraw(areaNew.szArea(), m_szLast);
            m_szLast = areaNew.szArea();
        }

        if (bOrgChanged)
        {
            if (m_pcivTarget)
                m_pcivTarget->ScrPosChanged(areaNew);
        }
    }
}


// Called when our window is created
tCIDLib::TBoolean TCQCIntfViewWnd::bCreated()
{
    // Pass it on to our parent window first
    TParent::bCreated();

    // We want the background color to be black
    SetBgnColor(facCIDGraphDev().rgbBlack);

    // Load the busy halow image
    m_bmpBusyHalo = TBitmap(facCQCIntfWEng(), kCQCIntfWEng::ridPNG_BusyHalo);

    return kCIDLib::True;
}


tCIDLib::TBoolean
TCQCIntfViewWnd::bExtKey(const  tCIDCtrls::EExtKeys     eExtKey
                        , const tCIDLib::TBoolean       bAltShift
                        , const tCIDLib::TBoolean       bCtrlShift
                        , const tCIDLib::TBoolean       bShift
                        , const TWindow* const          )
{
    // Pass this on to the view
    tCIDLib::TBoolean bHandled = kCIDLib::False;
    if (m_pcivTarget)
        bHandled = m_pcivTarget->bSpecialKey(eExtKey, bAltShift, bCtrlShift, bShift);

    return bHandled;
}


// If we get focus and the view is set, pass that on
tCIDLib::TVoid TCQCIntfViewWnd::GettingFocus()
{
    // Pass it on to the view
    if (m_pcivTarget)
        m_pcivTarget->GettingFocus();

    // And call our parent
    TParent::GettingFocus();
}


// If we lose focus and the view is set, pass that on
tCIDLib::TVoid TCQCIntfViewWnd::LosingFocus()
{
    // Pass it to our parent first
    TParent::LosingFocus();

    // Pass it on to the view
    if (m_pcivTarget)
        m_pcivTarget->LosingFocus();
}


//
//  If the view is set, we pass this on to him. He'll find the appropriate target
//  widget, if any, and pass it in turn to him, to let him indicate what type of
//  gesture he'll accept, if any.
//
tCIDLib::TVoid
TCQCIntfViewWnd::PerGestOpts(const  TPoint&                 pntAt
                            , const tCIDLib::EDirs          eDir
                            , const tCIDLib::TBoolean       bTwoFingers
                            ,       tCIDCtrls::EGestOpts&     eToSet
                            ,       tCIDLib::TFloat4&       f4VScale)
{
    //
    //  If the view is set, and we are not in no-input mode, then pass this
    //  on to the view. We also though have to check to see if we are already
    //  busy.
    //
    if (m_pcivTarget && !facCQCIntfWEng().bNoInputMode())
    {
        if (m_pcivTarget->bActiveAction())
        {
            const TSize szHalo(98, 86);
            TPoint pntOrg(pntAt);
            pntOrg.Adjust(-49, -43);

            const TArea areaTar(pntOrg, szHalo);
            TArea areaSrc(TPoint::pntOrigin, szHalo);
            TGraphWndDev gdevThis(*this);
            gdevThis.AlphaBlit
            (
                m_bmpBusyHalo, areaSrc, areaTar, 0x60, kCIDLib::True
            );
            TThread::Sleep(200);
            Redraw(areaTar, tCIDCtrls::ERedrawFlags::ImmedErase);
        }
         else
        {
            m_pcivTarget->PerGestOpts(pntAt, eDir, bTwoFingers, eToSet, f4VScale);
        }
    }
}


//
//  This is called if we get a flick. We just pass it on the view if set. He will find
//  the appropriate widget, if any, and tell him about the flick.
//
tCIDLib::TVoid
TCQCIntfViewWnd::GestFlick(const tCIDLib::EDirs eDir, const TPoint& pntStartPos)
{
    if (m_pcivTarget)
    {
        try
        {
            m_pcivTarget->ProcessFlick(eDir, pntStartPos);
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);

            m_pcivTarget->ShowErr
            (
                L"IV Click handler"
                , errToCatch.strErrText()
                , errToCatch
            );
        }

        catch(...)
        {
            TErrBox msgbErr
            (
                L"IV Gesture handler"
                , TString("A system error occurred during flick handling.")
            );
            msgbErr.ShowIt(*this);
        }
    }
}


tCIDLib::TVoid TCQCIntfViewWnd::Timer(const tCIDCtrls::TTimerId tmidToDo)
{
    // If timers are paused, do nothing
    if (m_c4PauseTimers)
        return;

    if (tmidToDo == m_tmidActiveUp)
    {
        //
        //  Ask the view to do an active update pass, and we also use this
        //  to make him watch for timeout events.
        //
        if (m_pcivTarget)
        {
            m_pcivTarget->DoActiveUpdatePass();
            m_pcivTarget->bCheckTimeout();
        }
    }
     else if (tmidToDo == m_tmidValueUp)
    {
        // Ask the view to do a value update pass
        if (m_pcivTarget)
            m_pcivTarget->DoUpdatePass();
    }
     else if (tmidToDo == m_tmidEvents)
    {
        // Ask the view to do an event processing pass
        if (m_pcivTarget)
            m_pcivTarget->DoEventProcessingPass();
    }
}



// ---------------------------------------------------------------------------
//  TCQCIntfViewWnd: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCIntfViewWnd::StartTimers()
{
    m_tmidActiveUp  = tmidStartTimer(kCQCIntfEng::c4ActiveUpdatePeriod);
    m_tmidEvents    = tmidStartTimer(kCQCIntfEng::c4EventUpdatePeriod);
    m_tmidValueUp   = tmidStartTimer(kCQCIntfEng::c4ValueUpdatePeriod);
}


tCIDLib::TVoid TCQCIntfViewWnd::StopTimers()
{
    StopTimer(m_tmidActiveUp);
    StopTimer(m_tmidEvents);
    StopTimer(m_tmidValueUp);

    m_tmidActiveUp  = kCIDLib::c4MaxCard;
    m_tmidEvents    = kCIDLib::c4MaxCard;
    m_tmidValueUp   = kCIDLib::c4MaxCard;
}


