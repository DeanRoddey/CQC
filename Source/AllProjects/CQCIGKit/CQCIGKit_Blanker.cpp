//
// FILE NAME: CQCGKit_Blanker.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/13/2001
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
//  This module implements the screen blanker for CQC.
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
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCBlankerWnd,TFrameWnd)


// ---------------------------------------------------------------------------
//  Local types and constants
// ---------------------------------------------------------------------------
namespace
{
    namespace CQCIGKit_Blanker
    {
        // -----------------------------------------------------------------------
        //  How long between image slides if in slideshow mode
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TEncodedTime enctSlideInterval(kCIDLib::enctOneMinute * 2);


        // -----------------------------------------------------------------------
        //  How long between updating the clock display
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TEncodedTime enctClockInterval(kCIDLib::enctOneMinute);


        // -----------------------------------------------------------------------
        //  The code that the slide show image downloader thread posts to the
        //  window to indicate it has a new image to display.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TInt4    i4NewImgReady = 1000;


        // -----------------------------------------------------------------------
        //  The code that we post to ourself to dismiss the dialog on a click or
        //  keystroke. We do it this way so that we can eat the up stroke/click
        //  before we exit.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TInt4    i4ExitBlanker = -1;


        // -----------------------------------------------------------------------
        //  The image repository scope we expect the slide show image to be in.
        // -----------------------------------------------------------------------
        const TString    strSlideScope(L"/User/CQC/Blanker");
    }
}



// ---------------------------------------------------------------------------
//  CLASS: TCQCBlankerWnd
// PREFIX: wnd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCBlankerWnd: Constructors and destructor
// ---------------------------------------------------------------------------
TCQCBlankerWnd::TCQCBlankerWnd(const TCQCUserCtx& cuctxToUse) :

    m_bPtrHidden(kCIDLib::False)
    , m_c4LastSlideScopeId(0)
    , m_c4LastSlideIndex(0)
    , m_cuctxToUse(cuctxToUse)
    , m_eMode(tCQCIGKit::EBlankModes::Normal)
    , m_pimgSlide(nullptr)
    , m_tmLastUpdate()
    , m_tmidUpdate(kCIDCtrls::tmidInvalid)
    , m_thrSlideLoader
      (
        TString(L"CQCBlankerWndDnldThread")
        , TMemberFunc<TCQCBlankerWnd>(this, &TCQCBlankerWnd::eDownloadThread)
      )
{
}


TCQCBlankerWnd::~TCQCBlankerWnd()
{
    // Clean up the slide show image if we created it
    try
    {
        delete m_pimgSlide;
    }

    catch(...)
    {
    }
    m_pimgSlide = nullptr;
}


// ---------------------------------------------------------------------------
//  TCQCBlankerWnd: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Get/set the display mode
tCQCIGKit::EBlankModes TCQCBlankerWnd::eMode() const
{
    return m_eMode;
}

tCQCIGKit::EBlankModes TCQCBlankerWnd::eMode(const tCQCIGKit::EBlankModes eToSet)
{
    m_eMode = eToSet;
    return m_eMode;
}


//
//  Called to create the blanker window. Though we are a top level window, we get
//  a caller window, so that we can figure out which monitor he is on and set our
//  window up to cover that.
//
tCIDLib::TVoid TCQCBlankerWnd::Create(  const   TWindow& wndCaller
                                        , const TString& strTitle)
{
    // Store away the title text for later use
    m_strTitle = strTitle;

    // Get the monitor that the caller is on. The whole size, not just available
    facCIDCtrls().QueryMonArea(wndCaller, m_areaMonitor, kCIDLib::False);

    CreateFrame
    (
        nullptr
        , m_areaMonitor
        , strTitle
        , tCIDLib::eOREnumBits(tCIDCtrls::EWndStyles::Visible, tCIDCtrls::EWndStyles::Popup)
        , tCIDLib::eOREnumBits
          (
            tCIDCtrls::EExWndStyles::TopMost, tCIDCtrls::EExWndStyles::ToolBar
          )
        , tCIDCtrls::EFrameStyles::None
        , kCIDLib::False
    );

    TakeFocus();
}


// ---------------------------------------------------------------------------
//  TCQCBlankerWnd: Protected, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TCQCBlankerWnd::bCreated()
{
    TParent::bCreated();

    // Reset any content tracking stuff
    m_tmLastUpdate = 0;

    // If we aren't in normal mode, start a timer for updates
    if (m_eMode != tCQCIGKit::EBlankModes::Normal)
    {
        if (m_eMode == tCQCIGKit::EBlankModes::Clock)
        {
            // If clock mode, then set up our clock fonts
            TFontSelAttrs fselClock;
            fselClock.strFaceName(L"Yu Gothic UI Light");
            fselClock.c4Height(96);
            fselClock.eWeight(tCIDGraphDev::EFontWeights::Thin);
            m_gfontTime.SetSelAttrs(fselClock);

            fselClock.c4Height(48);
            m_gfontDate.SetSelAttrs(fselClock);

            //
            //  Force an initial update. Otherwise, we will initially draw the zeroed
            //  time because we will draw before the first timer.
            //
            UpdateContent(kCIDLib::True);
        }

        // Start our update timer
        m_tmidUpdate = tmidStartTimer(2000);
    }

    // Indicate we want to see keyboard input
    SetWantInput(tCIDCtrls::EWantInputs::AllKeys);

    return kCIDLib::True;
}


// Handle character input and invoke
tCIDLib::TBoolean
TCQCBlankerWnd::bChar(  const   tCIDLib::TCh        chChar
                        , const tCIDLib::TBoolean
                        , const tCIDLib::TBoolean
                        , const tCIDLib::TBoolean
                        , const TWindow* const)
{
    //  Destroy ourself on any character key
    SendCode(CQCIGKit_Blanker::i4ExitBlanker, 0);
    return kCIDLib::True;
}


tCIDLib::TBoolean
TCQCBlankerWnd::bClick( const   tCIDCtrls::EMouseButts  eButton
                        , const tCIDCtrls::EMouseClicks eClickType
                        , const TPoint&                 pntAt
                        , const tCIDLib::TBoolean       bCtrlShift
                        , const tCIDLib::TBoolean       bShift)
{
    //
    //  Destroy ourself on any click. We eat the down click and dismiss on the up, so that
    //  the up doesn't end up showing up to someone else.
    //
    if (eClickType == tCIDCtrls::EMouseClicks::Up)
        SendCode(CQCIGKit_Blanker::i4ExitBlanker, 0);
    return kCIDLib::True;
}


tCIDLib::TBoolean
TCQCBlankerWnd::bExtKey(const   tCIDCtrls::EExtKeys     eExtKey
                        , const tCIDLib::TBoolean       bAltShift
                        , const tCIDLib::TBoolean       bCtrlShift
                        , const tCIDLib::TBoolean       bShift
                        , const TWindow* const          pwndChild)
{
    #if 0
    //
    //  Check specially for Shift or Ctrl-Shift keys. We have to ignore those
    //  because our key mappings support shifted function keys. If we don't do
    //  this, we'll dismiss when they hit the shift key.
    //
    if ((eExtKey == tCIDCtrls::EExtKeys::Ctrl)
    ||  (eExtKey == tCIDCtrls::EExtKeys::Shift))
    {
        return tCIDCtrls::EEventResponse_Handled;
    }
    #endif

    //
    //  If its unshifted F12, pop up the key mapping dialog, in the 'cheat
    //  sheet' mode that lets them execute mappings from the dialog.
    //
    if ((eExtKey == tCIDCtrls::EExtKeys::F12) && !bAltShift && !bCtrlShift && !bShift)
    {
        TCQCKeyMap kmToUse;
        if (facCQCIGKit().bQueryKeyMapData(*this, kmToUse, m_cuctxToUse, m_cuctxToUse.strUserName()))
        {
            facCQCIGKit().bEditKeyMap
            (
                *this
                , m_cuctxToUse.strUserName()
                , tCQCIGKit::EKMActModes::Execute
                , m_cuctxToUse
                , kmToUse
            );
        }
        return kCIDLib::True;
    }

    // If not a key map key, then we dismiss ourself
    if (!facCQCIGKit().bInvokeKeyMap(eExtKey
                                    , *this
                                    , *this
                                    , m_strTitle
                                    , bAltShift
                                    , bCtrlShift
                                    , bShift
                                    , m_cuctxToUse))
    {
        // Do this asynchronously (the default)
        SendCode(CQCIGKit_Blanker::i4ExitBlanker, 0);
    }
    return kCIDLib::True;
}


//
//  We post an async to ourself to dismiss on keystroke or click, to insure
//  that we eat any remaining input messages before we destroy ourself.
//
tCIDLib::TVoid
TCQCBlankerWnd::CodeReceived(const  tCIDLib::TInt4  i4Code
                            , const tCIDLib::TCard4 c4Data)
{
    if (i4Code == CQCIGKit_Blanker::i4ExitBlanker)
    {
        // Undo our hiding of the pointer
        if (m_bPtrHidden)
        {
            m_bPtrHidden = kCIDLib::False;
            facCIDCtrls().ShowPointer();
        }

        //
        //  Turn the backlight back on, just in case. Normally that will
        //  happen without our intervention, but if this is invoked from
        //  a remote call, we have to make sure it happens.
        //
        facCIDCtrls().SetMonitorPowerState(*this, tCIDCtrls::EMonPowerSt::On);
        Destroy();
    }
     else if (i4Code == CQCIGKit_Blanker::i4NewImgReady)
    {
        //
        //  We must be in slide show mode because the downloader thread has posted us a
        //  msg that there is a new image to display. So let's grab it and display it.
        //
        //  We know that the loader thread is not running now, so we can access
        //  the image object just for this setup of the bitmap, which we'll use
        //  from here on out to draw it.
        //
        TGraphWndDev gdevDraw(*this);
        m_bmpSlide = TBitmap(gdevDraw, *m_pimgSlide);

        // Get the area and center it
        m_areaContent.Set(TPoint::pntOrigin, m_bmpSlide.szBitmap());
        m_areaContent.CenterIn(areaWndSize());

        //
        //  And force a redraw. The size relationship between this one and
        //  any previous one means that it's not worth trying to do some sort
        //  of minimal redraw. And it only happens ever couple minutes anyway.
        //
        ForceRepaint();
    }
}


tCIDLib::TVoid TCQCBlankerWnd::Destroyed()
{
    // If we enabled the update timer, then disable it now
    if (m_tmidUpdate != kCIDCtrls::tmidInvalid)
    {
        StopTimer(m_tmidUpdate);
        m_tmidUpdate = kCIDCtrls::tmidInvalid;
    }

    // Undo our hiding of the pointer
    if (m_bPtrHidden)
    {
        m_bPtrHidden = kCIDLib::False;
        facCIDCtrls().ShowPointer();
    }

    // Make sure that the thread is all the way down
    try
    {
        if (m_thrSlideLoader.bIsRunning())
        {
            TWndPtrJanitor janPtr(tCIDCtrls::ESysPtrs::Wait);
            m_thrSlideLoader.ReqShutdownSync(10000);
            m_thrSlideLoader.eWaitForDeath(5000);
        }
    }

    catch(const TError& errToCatch)
    {
        TExceptDlg dlgError
        (
            *this
            , m_strTitle
            , facCQCIGKit().strMsg(kIGKitErrs::errcBlank_StopImgLoaderThread)
            , errToCatch
        );
    }

    // And call our parent
    TParent::Destroyed();
}


tCIDLib::TVoid TCQCBlankerWnd::GettingFocus()
{
    // When we have the focus, turn off the pointer if we've not done it
    if (!m_bPtrHidden)
    {
        m_bPtrHidden = kCIDLib::True;
        facCIDCtrls().HidePointer();
    }
}


tCIDLib::TVoid TCQCBlankerWnd::LosingFocus()
{
    // If we should lose focus before being destroyed, turn the pointer back on
    if (m_bPtrHidden)
    {
        m_bPtrHidden = kCIDLib::False;
        facCIDCtrls().ShowPointer();
    }
}


tCIDLib::TBoolean
TCQCBlankerWnd::bPaint(TGraphDrawDev& gdevToUse, const TArea& areaUpdate)
{
    // Fill with black, which overrides any standard frame paiting
    gdevToUse.Fill(areaUpdate, facCIDGraphDev().rgbBlack);

    //
    //  If we aren't normal mode, and the update area intersects the content
    //  area, then redraw what needs redrawing.
    //
    if ((m_eMode != tCQCIGKit::EBlankModes::Normal)
    &&   areaUpdate.bIntersects(m_areaContent))
    {
        if (m_eMode == tCQCIGKit::EBlankModes::Clock)
            DrawClock(gdevToUse, kCIDLib::False);
        else if (m_eMode == tCQCIGKit::EBlankModes::SlideShow)
            DrawSlide(gdevToUse, kCIDLib::False);
    }
    return kCIDLib::True;
}


// If it's our timer, update our content if needed
tCIDLib::TVoid TCQCBlankerWnd::Timer(const tCIDCtrls::TTimerId tmidToUse)
{
    if (tmidToUse == m_tmidUpdate)
        UpdateContent(kCIDLib::False);
}



// ---------------------------------------------------------------------------
//  TCQCBlankerWnd: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  A helper for drawing the clock when in clock mode.
//
tCIDLib::TVoid
TCQCBlankerWnd::DrawClock(TGraphDrawDev& gdevToUse, const tCIDLib::TBoolean bFillBgn)
{
    // Format out the time and date in the format we display
    TString strDateFmt;
    TString strTimeFmt;
    m_tmLastUpdate.FormatToStr(strDateFmt, L"%(a) %(m) %(D), %(Y)");
    m_tmLastUpdate.FormatToStr(strTimeFmt, TTime::strHHMM());

    //
    //  Calculate the two areas given their fonts. We have to basically do set the font
    //  twice, once to get the areas, so that we can place them, then to draw them.
    //
    TArea areaDate;
    TArea areaTime;
    {
        TFontJanitor janFont(&gdevToUse, &m_gfontDate);
        areaDate = gdevToUse.areaString(strDateFmt);
    }

    {
        TFontJanitor janFont(&gdevToUse, &m_gfontTime);
        areaTime = gdevToUse.areaString(strTimeFmt);
    }

    //
    //  OK, place the date area at the bottom left of the monitor, with a little spacing
    //  outwards from the edge.
    //
    areaDate.JustifyIn(m_areaMonitor, tCIDLib::EHJustify::Left, tCIDLib::EVJustify::Bottom);
    areaDate.AdjustOrg(16, -16);

    // And place the time above that
    areaTime.SetOrg(areaDate.pntOrg());
    areaTime.Move(tCIDLib::EDirs::Up);
    areaTime.AdjustOrg(0, -8);

    // Before we update the content area, fill the previous one if we are doing fill
    if (bFillBgn && !m_areaContent.bIsEmpty())
        gdevToUse.Fill(m_areaContent, facCIDGraphDev().rgbBlack);

    // The combination of these two becomes the content area
    m_areaContent = areaDate;
    m_areaContent |= areaTime;

    // Now we can draw the two strings
    gdevToUse.eBackMixMode(tCIDGraphDev::EBackMixModes::Transparent);
    gdevToUse.SetTextColor(facCIDGraphDev().rgbWhite);
    {
        TFontJanitor janFont(&gdevToUse, &m_gfontTime);
        gdevToUse.DrawString
        (
            strTimeFmt, areaTime, tCIDLib::EHJustify::Center, tCIDLib::EVJustify::Center
        );
    }

    {
        TFontJanitor janFont(&gdevToUse, &m_gfontDate);
        gdevToUse.DrawString
        (
            strDateFmt, areaDate, tCIDLib::EHJustify::Center, tCIDLib::EVJustify::Center
        );
    }
}


//
//  A helper for drawing the drawing the current image when in slide show
//  mode. Right now it's just a single call, but we might make it fancier in
//  the future, maybe a wipe fade or some orbital repositioning to avoid
//  burn-in and such.
//
tCIDLib::TVoid
TCQCBlankerWnd::DrawSlide(TGraphDrawDev& gdevToUse, const tCIDLib::TBoolean)
{
    // Just draw the slide. Allow the system to clip as required
    gdevToUse.DrawBitmap(m_bmpSlide, m_areaContent);
}


//
//  The thread that downloads slide show images in the background for us. It's
//  just restarted each time we need to download an image, and then exits after
//  posting a code to this window to come pick up the new image.
//
tCIDLib::EExitCodes
TCQCBlankerWnd::eDownloadThread(TThread& thrThis, tCIDLib::TVoid*)
{
    try
    {
        // If we haven't created the image object yet, do that
        if (!m_pimgSlide)
            m_pimgSlide = new TPNGImage;

        //
        //  If we don't have any image names in the list, then we've not downloaded them
        //  (successfully) yet, so try that.
        //
        TDataSrvClient dsclLoad;
        if (m_colSlideImgs.bIsEmpty())
        {
            dsclLoad.bQueryScopeNames
            (
                m_c4LastSlideScopeId
                , CQCIGKit_Blanker::strSlideScope
                , tCQCRemBrws::EDTypes::Image
                , m_colSlideImgs
                , tCQCRemBrws::EQSFlags::Items
                , m_cuctxToUse.sectUser()
            );
        }

        // If still no images, then we are done
        if (m_colSlideImgs.bIsEmpty())
            return tCIDLib::EExitCodes::Normal;

        // Bump the image index to the next id, and wrap if needed
        m_c4LastSlideIndex++;
        if (m_c4LastSlideIndex >= m_colSlideImgs.c4ElemCount())
            m_c4LastSlideIndex = 0;

        //
        //  Download the image. We have to build up the full path to it since we just
        //  get names in the list.
        //
        TString strNewImg = CQCIGKit_Blanker::strSlideScope;
        facCQCRemBrws().AddPathComp(strNewImg, m_colSlideImgs[m_c4LastSlideIndex]);

        tCIDLib::TCard4         c4SerNum = 0;
        tCIDLib::TEncodedTime   enctLast;
        tCIDLib::TKVPFList      colMeta;
        dsclLoad.bReadImage
        (
            strNewImg, c4SerNum, enctLast, *m_pimgSlide, colMeta, m_cuctxToUse.sectUser()
        );

        // Post a code to the window indicating we have a new image
        SendCode(CQCIGKit_Blanker::i4NewImgReady, 0);
    }

    catch(TError& errToCatch)
    {
        if (facCQCIGKit().bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
    }
    return tCIDLib::EExitCodes::Normal;
}


//
//  Called to see if the content needs to be drawn. It can be forced, in order to get the
//  initial display done, else the content area will stay empty and we'll never do anything.
//
tCIDLib::TVoid TCQCBlankerWnd::UpdateContent(const tCIDLib::TBoolean bForce)
{
    // If in normal mode, nothing to do
    if (m_eMode == tCQCIGKit::EBlankModes::Normal)
        return;

    // Get the current time and see how much has elapsed since our last update
    TTime tmNow(tCIDLib::ESpecialTimes::CurrentTime);
    const tCIDLib::TEncodedTime enctDiff = tmNow.enctDifference(m_tmLastUpdate);

    //
    //  If enough time has elapsed to need to do an update for the mode we are
    //  in, then do it and store the new time as the last update time.
    //
    if (m_eMode == tCQCIGKit::EBlankModes::Clock)
    {
        if ((enctDiff > CQCIGKit_Blanker::enctClockInterval) || bForce)
        {
            m_tmLastUpdate = tmNow;
            TGraphWndDev gdevWnd(*this);
            DrawClock(gdevWnd, kCIDLib::True);
        }
    }
     else if (m_eMode == tCQCIGKit::EBlankModes::SlideShow)
    {
        if ((enctDiff > CQCIGKit_Blanker::enctSlideInterval) || bForce)
        {
            m_tmLastUpdate = tmNow;

            //
            //  Make sure the downloader thread isn't already running. If not, then
            //  start it up for another round. When it's done, it'll let us know and
            //  we'll redraw then.
            //
            if (!m_thrSlideLoader.bIsRunning())
                m_thrSlideLoader.Start();
        }
    }
}

