//
// FILE NAME: CQCWCHelper_MainFrame.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/05/2017
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
#include    "CQCWCHelper.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TMainFrameWnd,TFrameWindow)



// ---------------------------------------------------------------------------
//  CLASS: TGUIMsg
// PREFIX: qmsg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TGUIMsg: Constructors and Destructor
// ---------------------------------------------------------------------------

TGUIMsg::TGUIMsg() :

    m_bVal(kCIDLib::False)
    , m_eType(tCQCWCHelper::EGUIMsgTypes::None)
{
}

TGUIMsg::TGUIMsg(const tCQCWCHelper::EGUIMsgTypes eType) :

    m_bVal(kCIDLib::False)
    , m_eType(eType)
{
}

TGUIMsg::TGUIMsg(const  tCQCWCHelper::EGUIMsgTypes  eType
                , const tCIDLib::TBoolean           bVal) :

    m_bVal(bVal)
    , m_eType(eType)
{
}

TGUIMsg::TGUIMsg(const  tCQCWCHelper::EGUIMsgTypes  eType
                , const TString&                    strVal) :

    m_bVal(kCIDLib::False)
    , m_eType(eType)
    , m_strVal1(strVal)
{
}

TGUIMsg::TGUIMsg(const TArea& areaPar, const TArea& areaWidget) :

    m_areaVal(areaPar)
    , m_areaVal2(areaWidget)
    , m_bVal(kCIDLib::False)
    , m_eType(tCQCWCHelper::EGUIMsgTypes::SetArea)
{
}

TGUIMsg::TGUIMsg(const  TArea&              areaPar
                , const TArea&              areaWidget
                , const tCIDLib::TBoolean   bVisibility) :

    m_areaVal(areaPar)
    , m_areaVal2(areaWidget)
    , m_bVal(bVisibility)
    , m_eType(tCQCWCHelper::EGUIMsgTypes::Init)
{
}

TGUIMsg::TGUIMsg(const TGUIMsg& qmsgSrc) :

    m_areaVal(qmsgSrc.m_areaVal)
    , m_areaVal2(qmsgSrc.m_areaVal2)
    , m_bVal(qmsgSrc.m_bVal)
    , m_eType(qmsgSrc.m_eType)
    , m_strVal1(qmsgSrc.m_strVal1)
{
}

TGUIMsg::~TGUIMsg()
{
}


// ---------------------------------------------------------------------------
//  TGUIMsg: Public operators
// ---------------------------------------------------------------------------
TGUIMsg& TGUIMsg::operator=(const TGUIMsg& qmsgSrc)
{
    if (this != &qmsgSrc)
    {
        m_areaVal   = qmsgSrc.m_areaVal;
        m_areaVal2  = qmsgSrc.m_areaVal2;
        m_bVal      = qmsgSrc.m_bVal;
        m_eType     = qmsgSrc.m_eType;
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

    m_bDummyTest(kCIDLib::False)
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
                        , const tCIDLib::TBoolean       bDummyTest)
{
    m_bDummyTest = bDummyTest;

    //
    //  Just use a small default area. We aren't visible initially. In the widget's
    //  post init it'll send us the correct initial area, by which time it knows it.
    //
    TParent::CreateFrame
    (
        nullptr
        , bDummyTest ? TArea(100, 100, 1280, 720) : TArea(0, 0, 64, 64)
        , TString::strEmpty()
        , tCIDCtrls::EWndStyles::Popup
          | tCIDCtrls::EWndStyles::ClipSiblings
        , tCIDCtrls::EExWndStyles::ToolBar
        , tCIDCtrls::EFrameStyles::None
        , kCIDLib::False
    );

    // Make the other program's passed handle our owner, so we stay on top of him
    if (!bDummyTest)
        facCIDCtrls().SetExtOwner(hwndThis(), hwndOwner);
}


// ---------------------------------------------------------------------------
//  TMainFrameWnd: Protected, inherited methods
// ---------------------------------------------------------------------------

// We are only shut down via incoming remote command
tCIDLib::TBoolean TMainFrameWnd::bAllowShutdown()
{
    return m_bDummyTest;
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

    if ((ePosState != tCIDCtrls::EPosStates::Minimized) && m_wndVideo.bIsValid())
    {
        if (bSizeChanged)
        {
            try
            {
                m_wndVideo.SetSizePos(areaWndSize(), kCIDLib::False);
            }

            catch(TError& errToCatch)
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);

                facCQCWCHelper().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kWCHErrs::errcCam_Resize
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

    try
    {
        // Create our video window
        m_wndVideo.CreateGenWnd
        (
            *this
            , areaWndSize()
            , tCIDCtrls::EWndStyles::VisChild
            , tCIDCtrls::EExWndStyles::None
            , kCIDCtrls::widFirstCtrl
        );
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        facCQCWCHelper().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kWCHErrs::errcCam_Create
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::CantDo
        );
    }

    // Start our timeout monitor
    m_tmidMonitor = tmidStartTimer(1000);

    // If not playing video, we fill the background with black
    SetBgnColor(TRGBClr());

    return kCIDLib::True;
}


//
//  If we are in playing state, then the video is covering the whole window so we want
//  to suppress this. Else, we'll fill with black, and show current state info.
//
tCIDLib::TBoolean TMainFrameWnd::bEraseBgn(TGraphDrawDev& gdevToUse)
{
    if (facCQCWCHelper().eCurState() != tCQCWCHelper::EStates::Playing)
        gdevToUse.Fill(areaWndSize(), gbrBgn());
    return kCIDLib::True;
}


// Suppress this, we are fully covered by the video window
tCIDLib::TBoolean
TMainFrameWnd::bPaint(TGraphDrawDev& gdevToUse, const TArea& areaInvalid)
{
    // For debugging to see where it is
    // gdevToUse.Fill(areaInvalid, TRGBClr(0xFF, 0, 0));

    return kCIDLib::True;
}


// The facility object posts to us here when it queues up a new message
tCIDLib::TVoid
TMainFrameWnd::CodeReceived(const tCIDLib::TInt4 i4Code, const tCIDLib::TCard4)
{
    if (i4Code == kCQCWCHelper::i4NewMsg)
    {
        while (facCQCWCHelper().bGetNextGUIMsg(m_qmsgCur))
        {
            try
            {
                switch(m_qmsgCur.m_eType)
                {
                    case tCQCWCHelper::EGUIMsgTypes::Close :
                        //
                        //  Hide us immediate, and then exit the msg loop to clean up
                        //  which will happen async and might be delayed if the video
                        //  engine is slow to stop.
                        //
                        SetVisibility(kCIDLib::False);
                        facCIDCtrls().ExitLoop(1);
                        break;

                    case tCQCWCHelper::EGUIMsgTypes::Init :
                        SetNewArea(m_qmsgCur.m_areaVal, m_qmsgCur.m_areaVal2);
                        facCIDCtrls().MsgYield(50);
                        SetVisibility(m_qmsgCur.m_bVal, kCIDLib::False);
                        break;

                    case tCQCWCHelper::EGUIMsgTypes::SetArea :
                    {
                        if (facCQCWCHelper().bVerboseLogging())
                        {
                            facCQCWCHelper().LogMsg
                            (
                                CID_FILE
                                , CID_LINE
                                , L"Got set area msg for area %(1)/%(2))"
                                , tCIDLib::ESeverities::Failed
                                , tCIDLib::EErrClasses::CantDo
                                , m_qmsgCur.m_areaVal
                                , m_qmsgCur.m_areaVal2
                            );
                        }
                        SetNewArea(m_qmsgCur.m_areaVal, m_qmsgCur.m_areaVal2);
                        break;
                    }

                    case tCQCWCHelper::EGUIMsgTypes::SetVisibility :
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

                facCQCWCHelper().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kWCHErrs::errcCam_CameraOp
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::CantDo
                    , TCardinal(tCIDLib::c4EnumOrd(m_qmsgCur.m_eType))
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

    // We have to destroy the video window since it's a member, not a pointer
    try
    {
        m_wndVideo.Destroy();
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);
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
        if (!m_bDummyTest && facCQCWCHelper().bTimedOut())
            facCIDCtrls().ExitLoop(1);
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
