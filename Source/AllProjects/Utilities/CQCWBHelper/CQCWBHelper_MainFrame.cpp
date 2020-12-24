//
// FILE NAME: CQCWBHelper_MainFrame.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/16/2016
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
//  This file implements the main frame window.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCWBHelper.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TMainFrameWnd,TFrameWindow)



// ---------------------------------------------------------------------------
//  CLASS: TQMsg
// PREFIX: qmsg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TQMsg: Constructors and Destructor
// ---------------------------------------------------------------------------

TQMsg::TQMsg() :

    m_bVal(kCIDLib::False)
    , m_eType(tCQCWBHelper::EQMsgTypes::None)
    , m_eWebOp(tWebBrowser::ENavOps::Back)
{
}

TQMsg::TQMsg(const tCQCWBHelper::EQMsgTypes eType) :

    m_bVal(kCIDLib::False)
    , m_eType(eType)
    , m_eWebOp(tWebBrowser::ENavOps::Back)
{
}

TQMsg::TQMsg(const  tCQCWBHelper::EQMsgTypes    eType
            , const tCIDLib::TBoolean           bVal) :

    m_bVal(bVal)
    , m_eType(eType)
    , m_eWebOp(tWebBrowser::ENavOps::Back)
{
}

TQMsg::TQMsg(const  tCQCWBHelper::EQMsgTypes    eType
            , const TString&                    strVal) :

    m_bVal(kCIDLib::False)
    , m_eType(eType)
    , m_eWebOp(tWebBrowser::ENavOps::Back)
    , m_strVal1(strVal)
{
}

TQMsg::TQMsg(const tWebBrowser::ENavOps eToDo) :

    m_bVal(kCIDLib::False)
    , m_eType(tCQCWBHelper::EQMsgTypes::DoOp)
    , m_eWebOp(eToDo)
{
}

TQMsg::TQMsg(const TArea& areaPar, const TArea& areaWidget) :

    m_areaVal(areaPar)
    , m_areaVal2(areaWidget)
    , m_bVal(kCIDLib::False)
    , m_eType(tCQCWBHelper::EQMsgTypes::SetArea)
    , m_eWebOp(tWebBrowser::ENavOps::Back)
{
}

TQMsg::TQMsg(const  TArea&              areaPar
            , const TArea&              areaWidget
            , const tCIDLib::TBoolean   bVisibility
            , const TString&            strInitURL) :

    m_areaVal(areaPar)
    , m_areaVal2(areaWidget)
    , m_bVal(bVisibility)
    , m_eType(tCQCWBHelper::EQMsgTypes::Init)
    , m_eWebOp(tWebBrowser::ENavOps::Back)
    , m_strVal1(strInitURL)
{
}

TQMsg::TQMsg(const TQMsg& qmsgSrc) :

    m_areaVal(qmsgSrc.m_areaVal)
    , m_areaVal2(qmsgSrc.m_areaVal2)
    , m_bVal(qmsgSrc.m_bVal)
    , m_eType(qmsgSrc.m_eType)
    , m_eWebOp(qmsgSrc.m_eWebOp)
    , m_strVal1(qmsgSrc.m_strVal1)
{
}

TQMsg::~TQMsg()
{
}


// ---------------------------------------------------------------------------
//  TQMsg: Public operators
// ---------------------------------------------------------------------------
TQMsg& TQMsg::operator=(const TQMsg& qmsgSrc)
{
    if (this != &qmsgSrc)
    {
        m_areaVal   = qmsgSrc.m_areaVal;
        m_areaVal2  = qmsgSrc.m_areaVal2;
        m_bVal      = qmsgSrc.m_bVal;
        m_eType     = qmsgSrc.m_eType;
        m_eWebOp    = qmsgSrc.m_eWebOp;
        m_strVal1   = qmsgSrc.m_strVal1;
    }
    return *this;
}




// ---------------------------------------------------------------------------
//  CLASS: TMainFrameWnd
// PREFIX: wnd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TMainFrameWnd: Constructors and destructor
// ---------------------------------------------------------------------------
TMainFrameWnd::TMainFrameWnd() :

    m_c4RefreshMins(0)
    , m_enctNextRefresh(0)
    , m_tmidMonitor(kCIDCtrls::tmidInvalid)
{
}

TMainFrameWnd::~TMainFrameWnd()
{
}


// ---------------------------------------------------------------------------
//  TMainFrameWnd: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TMainFrameWnd::Create(  const   tCIDCtrls::TWndHandle   hwndOwner
                        , const tCIDLib::TBoolean       bSilentMode
                        , const tCIDLib::TCard4         c4RefreshMins)
{
    //
    //  Just use a small default area. We aren't visible initially. In the widget's
    //  post init it'll send us the correct initial area, by which time it knows it.
    //
    TParent::CreateFrame
    (
        nullptr
        , TArea(0, 0, 128, 128)
        , TString::strEmpty()
        , tCIDLib::eOREnumBits
          (
            tCIDCtrls::EWndStyles::Popup, tCIDCtrls::EWndStyles::ClipSiblings
          )
        , tCIDCtrls::EExWndStyles::ToolBar
        , tCIDCtrls::EFrameStyles::None
        , kCIDLib::False
    );

    // Store the incoming refresh minutes
    m_c4RefreshMins = c4RefreshMins;

    // Make the other program's passed handle our owner, so we stay on top of him
    facCIDCtrls().SetExtOwner(hwndThis(), hwndOwner);

    // Set the silent mode if set
    if (bSilentMode)
        m_wndBrowser.SetOpt(tWebBrowser::EBoolOpts::SilentMode, kCIDLib::True);

    // If refresh is enabled, set the next refresh time
    if (m_c4RefreshMins)
        m_enctNextRefresh = TTime::enctNowPlusMins(m_c4RefreshMins);
}


// ---------------------------------------------------------------------------
//  TMainFrameWnd: Protected, inherited methods
// ---------------------------------------------------------------------------

// We are only shut down via incoming remote command
tCIDLib::TBoolean TMainFrameWnd::bAllowShutdown()
{
    return kCIDLib::False;
}


tCIDLib::TVoid
TMainFrameWnd::AreaChanged( const   TArea&                  areaPrev
                            , const TArea&                  areaNew
                            , const tCIDCtrls::EPosStates   ePosState
                            , const tCIDLib::TBoolean       bOrgChanged
                            , const tCIDLib::TBoolean       bSizeChanged
                            , const tCIDLib::TBoolean       bStateChanged)
{
    // Pass on to our parent first
    TParent::AreaChanged(areaPrev, areaNew, ePosState, bOrgChanged, bSizeChanged, bStateChanged);

    //
    //  If the size changed and we are maximized or restored, then we need to update the
    //  browser window for our new size.
    //
    if ((ePosState != tCIDCtrls::EPosStates::Minimized) && m_wndBrowser.bIsValid())
    {
        if (bSizeChanged)
        {
            try
            {
                m_wndBrowser.SetSize(areaNew.szArea(), kCIDLib::False);
            }

            catch(TError& errToCatch)
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);

                facCQCWBHelper().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kWBHErrs::errcBrws_Resize
                    , m_strLastURL
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::CantDo
                );
            }
        }
    }
}


tCIDLib::TBoolean TMainFrameWnd::bCreated()
{
    // Call our parent class first
    TParent::bCreated();

    // Create the browser window
    try
    {
        m_wndBrowser.CreateBrwsWindow
        (
            *this
            , tCIDCtrls::EWndStyles
              (
                tCIDCtrls::EWndStyles::VisChild
              )
            , areaClient()
            , kCIDCtrls::widFirstCtrl
        );
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        facCQCWBHelper().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kWBHErrs::errcBrws_Create
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::CantDo
        );
    }

    // Start our timeout monitor
    m_tmidMonitor = tmidStartTimer(1000);

    return kCIDLib::True;
}


// Suppress this, we are fully covered by the browser
tCIDLib::TBoolean TMainFrameWnd::bEraseBgn(TGraphDrawDev&)
{
    return kCIDLib::True;
}


// Suppress this, we are fully covered by the browser
tCIDLib::TBoolean
TMainFrameWnd::bPaint(TGraphDrawDev& gdevToUse, const TArea& areaInvalid)
{
    // For debugging to see where it is
    //gdevToUse.Fill(areaInvalid, TRGBClr(0xFF, 0, 0));

    return kCIDLib::True;
}


// The facility object posts to us here when it queues up a new message
tCIDLib::TVoid
TMainFrameWnd::CodeReceived(const tCIDLib::TInt4 i4Code, const tCIDLib::TCard4)
{
    if (i4Code == kCQCWBHelper::i4NewMsg)
    {
        TFacCQCWBHelper& facUs = facCQCWBHelper();
        while (facUs.bGetNextMsg(m_qmsgCur))
        {
            try
            {
                switch(m_qmsgCur.m_eType)
                {
                    case tCQCWBHelper::EQMsgTypes::Close :
                        facCIDCtrls().ExitLoop(1);
                        break;

                    case tCQCWBHelper::EQMsgTypes::DoOp :
                        m_wndBrowser.DoNavOp(m_qmsgCur.m_eWebOp);
                        break;

                    case tCQCWBHelper::EQMsgTypes::Init :
                        // This is a combination of SetArea, SetVisibility, and set URL
                        SetNewArea(m_qmsgCur.m_areaVal, m_qmsgCur.m_areaVal2);
                        facCIDCtrls().MsgYield(50);
                        m_wndBrowser.SetURL(m_qmsgCur.m_strVal1);
                        facCIDCtrls().MsgYield(50);
                        SetVisibility(m_qmsgCur.m_bVal);
                        break;

                    case tCQCWBHelper::EQMsgTypes::SetArea :
                        SetNewArea(m_qmsgCur.m_areaVal, m_qmsgCur.m_areaVal2);
                        break;

                    case tCQCWBHelper::EQMsgTypes::SetURL :
                        m_wndBrowser.SetURL(m_qmsgCur.m_strVal1);
                        break;

                    case tCQCWBHelper::EQMsgTypes::SetVisibility :
                        SetVisibility(m_qmsgCur.m_bVal);
                        break;

                    default :
                        break;
                };
            }

            catch(TError& errToCatch)
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);

                facCQCWBHelper().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kWBHErrs::errcBrws_BrowserOp
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::CantDo
                    , TInteger(m_qmsgCur.m_eType)
                );
            }
        }
    }
}


tCIDLib::TVoid TMainFrameWnd::Destroyed()
{
    if (m_tmidMonitor != kCIDCtrls::tmidInvalid)
    {
        StopTimer(m_tmidMonitor);
        m_tmidMonitor = kCIDCtrls::tmidInvalid;
    }

    //
    //  Destroy the browser window. We have to do this for multiple reasons, to insure the
    //  COM stuff really gets cleaned up, and because it's not a pointer. If we didn't do
    //  this, the base window class would try to destroy it and delete it.
    //
    if (m_wndBrowser.bIsValid())
    {
        try
        {
            m_wndBrowser.Terminate();
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        m_wndBrowser.Destroy();
    }

    TParent::Destroyed();
}


//
//  We enable a timer and use as a watchdog monitor. If we haven't received at least a ping
//  from the invoking program in a given period, we will exit. We also use it to do auto-
//  refresh if that's enabled.
//
tCIDLib::TVoid TMainFrameWnd::Timer(const tCIDCtrls::TTimerId tmidSrc)
{
    if (m_tmidMonitor == tmidSrc)
    {
        if (facCQCWBHelper().bTimedOut())
        {
            facCIDCtrls().ExitLoop(1);
        }
         else if (m_c4RefreshMins)
        {
            // Refresh is enabled, so see if we need to do one
            const tCIDLib::TEncodedTime enctCur = TTime::enctNow();
            if (enctCur > m_enctNextRefresh)
            {
                m_enctNextRefresh = enctCur + (kCIDLib::enctOneMinute * m_c4RefreshMins);

                try
                {
                    m_wndBrowser.DoNavOp(tWebBrowser::ENavOps::Reload);
                }

                catch(TError& errToCatch)
                {
                    errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                    TModule::LogEventObj(errToCatch);

                    facCQCWBHelper().LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , kWBHErrs::errcBrws_Refresh
                        , m_strLastURL
                        , tCIDLib::ESeverities::Failed
                        , tCIDLib::EErrClasses::CantDo
                    );
                }
            }
        }
    }
}


// ---------------------------------------------------------------------------
//  TMainFrameWnd: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  We have to move ourself to stay on top of the controlling widget's area. But we also have
//  to make sure we remain clipped to the area of the window that contains the widget.
//
tCIDLib::TVoid TMainFrameWnd::SetNewArea(const TArea& areaPar, const TArea& areaWidget)
{
    // If the new widget area is not completely within that, then we need to clip it.
    TArea areaActual = areaPar & areaWidget;

    // Set our new position
    SetSizePos(areaActual, kCIDLib::False);
}
