//
// FILE NAME: CQCIntfView_ExtCtrlServerImpl.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/01/2004
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
//  The
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
#include    "CQCIntfView.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TIntfCtrlServer,TIntfCtrlServerBase)
RTTIDecls(TIntfCtrlEv,TObject)
RTTIDecls(TIntfCtrlExtKeyEv,TIntfCtrlEv)
RTTIDecls(TIntfCtrlFrameOp,TIntfCtrlEv)
RTTIDecls(TIntfCtrlLoadEv,TIntfCtrlEv)
RTTIDecls(TIntfCtrlMiscEv,TIntfCtrlEv)
RTTIDecls(TIntfCtrlSetVarEv,TIntfCtrlEv)


// ---------------------------------------------------------------------------
//   CLASS: TIntfCtrlEv
//  PREFIX: aue
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TIntfCtrlEv: Destructor
// ---------------------------------------------------------------------------
TIntfCtrlEv::~TIntfCtrlEv()
{
}


// ---------------------------------------------------------------------------
//  TIntfCtrlEv: Hidden constructors
// ---------------------------------------------------------------------------
TIntfCtrlEv::TIntfCtrlEv()
{
}




// ---------------------------------------------------------------------------
//   CLASS: TIntfCtrlExtKeyEv
//  PREFIX: aue
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TIntfCtrlExtKeyEv: Constructors and Destructor
// ---------------------------------------------------------------------------
TIntfCtrlExtKeyEv::TIntfCtrlExtKeyEv(const  tCIDCtrls::EExtKeys eToSend
                                    , const tCIDLib::TBoolean   bAltShift
                                    , const tCIDLib::TBoolean   bCtrlShift
                                    , const tCIDLib::TBoolean   bShift) :
    TIntfCtrlEv()
    , m_bAltShift(bAltShift)
    , m_bCtrlShift(bCtrlShift)
    , m_bShift(bShift)
    , m_eToSend(eToSend)
{
}

TIntfCtrlExtKeyEv::~TIntfCtrlExtKeyEv()
{
}


// ---------------------------------------------------------------------------
//  TIntfCtrlExtKeyEv: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TIntfCtrlExtKeyEv::ProcessEv(TMainFrameWnd& wndMainFrame)
{
    // Make sure this guy is the foreground window
    wndMainFrame.ForceToFront();

    if (facCQCIntfView.bLogInfo())
    {
        facCQCIntfView.LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"Sending a remote ctrl key event"
            , tCIDLib::ESeverities::Info
            , tCIDLib::EErrClasses::AppStatus
        );
    }

    // And send the key stroke
    facCIDCtrls().SendExtKey(m_eToSend, m_bAltShift, m_bCtrlShift, m_bShift);
}




// ---------------------------------------------------------------------------
//   CLASS: TIntfCtrlFrameOp
//  PREFIX: ice
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TIntfCtrlFrameOp: Constructors and Destructor
// ---------------------------------------------------------------------------
TIntfCtrlFrameOp::TIntfCtrlFrameOp(const tCQCKit::EIVFrmOps eOp) :

    TIntfCtrlEv()
    , m_eOp(eOp)
{
}

TIntfCtrlFrameOp::~TIntfCtrlFrameOp()
{
}


// ---------------------------------------------------------------------------
//  TIntfCtrlFrameOp: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TIntfCtrlFrameOp::ProcessEv(TMainFrameWnd& wndMainFrame)
{
    switch(m_eOp)
    {
        case tCQCKit::EIVFrmOps::FSMode :
            // If not alreayd in full screen mode, then do it
            if (!facCQCIntfView.bFullScreen())
            {
                wndMainFrame.StartFSMode();
                wndMainFrame.ForceToFront();
            }
            break;

        case tCQCKit::EIVFrmOps::Hide :
            wndMainFrame.SetVisibility(kCIDLib::False);
            break;

        case tCQCKit::EIVFrmOps::Maximize :
            wndMainFrame.Maximize();
            break;

        case tCQCKit::EIVFrmOps::Minimize :
            wndMainFrame.Minimize();
            break;

        case tCQCKit::EIVFrmOps::Restore :
            // If in fulls creen, exit that, else do a regular restore
            if (facCQCIntfView.bFullScreen())
                wndMainFrame.EndFSMode();
            else
                wndMainFrame.Restore();
            break;

        case tCQCKit::EIVFrmOps::Show :
            wndMainFrame.SetVisibility(kCIDLib::True);
            break;

        case tCQCKit::EIVFrmOps::ToBack :
            wndMainFrame.ToBottom();
            break;

        case tCQCKit::EIVFrmOps::ToFront:
            wndMainFrame.ForceToFront();
            break;
    };
}




// ---------------------------------------------------------------------------
//   CLASS: TIntfCtrlLoadEv
//  PREFIX: ice
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TIntfCtrlLoadEv: Constructors and Destructor
// ---------------------------------------------------------------------------
TIntfCtrlLoadEv::TIntfCtrlLoadEv(const TString& strTemplate) :

    TIntfCtrlEv()
    , m_bOverlay(kCIDLib::False)
    , m_strTemplate(strTemplate)
{
}

TIntfCtrlLoadEv::TIntfCtrlLoadEv(const  TString& strOvrName
                                , const TString& strTemplate) :

    TIntfCtrlEv()
    , m_bOverlay(kCIDLib::True)
    , m_strOvrName(strOvrName)
    , m_strTemplate(strTemplate)
{
}

TIntfCtrlLoadEv::~TIntfCtrlLoadEv()
{
}


// ---------------------------------------------------------------------------
//  TIntfCtrlLoadEv: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TIntfCtrlLoadEv::ProcessEv(TMainFrameWnd& wndMainFrame)
{
    wndMainFrame.DoRemLoadOp(m_bOverlay, m_strOvrName, m_strTemplate);
}




// ---------------------------------------------------------------------------
//   CLASS: TIntfCtrlMiscEv
//  PREFIX: ice
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TIntfCtrlMiscEv: Constructors and Destructor
// ---------------------------------------------------------------------------
TIntfCtrlMiscEv::TIntfCtrlMiscEv(const tCQCKit::EIVMiscOps eOp) :

    TIntfCtrlEv()
    , m_eOp(eOp)
{
}

TIntfCtrlMiscEv::~TIntfCtrlMiscEv()
{
}


// ---------------------------------------------------------------------------
//  TIntfCtrlMiscEv: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TIntfCtrlMiscEv::ProcessEv(TMainFrameWnd& wndMainFrame)
{
    wndMainFrame.DoRemMiscOp(m_eOp);
}




// ---------------------------------------------------------------------------
//   CLASS: TIntfCtrlSetVarEv
//  PREFIX: ice
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TIntfCtrlSetVarEv: Constructors and Destructor
// ---------------------------------------------------------------------------
TIntfCtrlSetVarEv::TIntfCtrlSetVarEv(const  TString& strVarName
                                    , const TString& strValue) :

    TIntfCtrlEv()
    , m_strVarName(strVarName)
    , m_strValue(strValue)
{
}

TIntfCtrlSetVarEv::~TIntfCtrlSetVarEv()
{
}


// ---------------------------------------------------------------------------
//  TIntfCtrlSetVarEv: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TIntfCtrlSetVarEv::ProcessEv(TMainFrameWnd& wndMainFrame)
{
    wndMainFrame.SetVariable(m_strVarName, m_strValue);
}





// ---------------------------------------------------------------------------
//   CLASS: TIntfCtrlServer
//  PREFIX: orbs
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TIntfCtrlServer: Constructors and Destructor
// ---------------------------------------------------------------------------
TIntfCtrlServer::TIntfCtrlServer() :

    m_bMaximized(kCIDLib::False)
    , m_c4LayerCount(0)
{
}

TIntfCtrlServer::~TIntfCtrlServer()
{
}


// ---------------------------------------------------------------------------
//  TIntfCtrlServer: Public, inherited methods
// ---------------------------------------------------------------------------

// Pass it on to the facility class to queue up for processing
tCIDLib::TBoolean TIntfCtrlServer::bSpeakText(const TString& strToSpeak)
{
    return facCQCIntfView.bQueueTTS(strToSpeak);
}


//
//  We just give back the late status info that the main frame window stored
//  on us via the StoreStatus() method below.
//
tCIDLib::TCard4
TIntfCtrlServer::c4Poll(TString&                strBaseTmpl
                        , TString&              strTopTmpl
                        , tCIDLib::TBoolean&    bMaximized)
{
    // Synchronize and store the data
    TLocker lockrSync(&m_mtxSync);

    bMaximized = m_bMaximized;
    strBaseTmpl = m_strBaseTemplate;
    strTopTmpl = m_strTopTemplate;

    return m_c4LayerCount;
}



// Just add it to the viewer's queue of things to process
tCIDLib::TVoid
TIntfCtrlServer::DoFrameOp(const tCQCKit::EIVFrmOps eOp)
{
    facCQCIntfView.wndMain().AddCtrlEvent(new TIntfCtrlFrameOp(eOp));
}


// Just add it to the viewer's queue of things to process
tCIDLib::TVoid
TIntfCtrlServer::DoMiscOp(const tCQCKit::EIVMiscOps eOp)
{
    facCQCIntfView.wndMain().AddCtrlEvent(new TIntfCtrlMiscEv(eOp));
}


tCIDLib::TVoid TIntfCtrlServer::Invoke()
{
    // Queue up an enter key event
    facCQCIntfView.wndMain().AddCtrlEvent
    (
        new TIntfCtrlExtKeyEv
        (
            tCIDCtrls::EExtKeys::Enter
            , kCIDLib::False
            , kCIDLib::False
            , kCIDLib::False
        )
    );
}


tCIDLib::TVoid
TIntfCtrlServer::LoadOverlay(const  TString&    strOvrName
                            , const TString&    strToLoad)
{
    facCQCIntfView.wndMain().AddCtrlEvent
    (
        new TIntfCtrlLoadEv(strOvrName, strToLoad)
    );

}

tCIDLib::TVoid TIntfCtrlServer::LoadTemplate(const TString& strToLoad)
{
    facCQCIntfView.wndMain().AddCtrlEvent(new TIntfCtrlLoadEv(strToLoad));
}




tCIDLib::TVoid TIntfCtrlServer::Navigation(const tCQCKit::EScrNavOps eOp)
{
    // Convert from a direction to a key
    tCIDLib::TBoolean bShift = kCIDLib::False;
    tCIDCtrls::EExtKeys eKey;
    switch(eOp)
    {
        case tCQCKit::EScrNavOps::Down :
            eKey = tCIDCtrls::EExtKeys::Down;
            break;

        case tCQCKit::EScrNavOps::End :
            eKey = tCIDCtrls::EExtKeys::End;
            break;

        case tCQCKit::EScrNavOps::Home :
            eKey = tCIDCtrls::EExtKeys::Home;
            break;

        case tCQCKit::EScrNavOps::Left :
            eKey = tCIDCtrls::EExtKeys::Left;
            break;

        case tCQCKit::EScrNavOps::Next :
            eKey = tCIDCtrls::EExtKeys::Tab;
            break;

        case tCQCKit::EScrNavOps::NextPage :
            eKey = tCIDCtrls::EExtKeys::PageDown;
            break;

        case tCQCKit::EScrNavOps::Prev :
            eKey = tCIDCtrls::EExtKeys::Tab;
            bShift = kCIDLib::True;
            break;

        case tCQCKit::EScrNavOps::PrevPage :
            eKey = tCIDCtrls::EExtKeys::PageUp;
            break;

        case tCQCKit::EScrNavOps::Right :
            eKey = tCIDCtrls::EExtKeys::Right;
            break;

        case tCQCKit::EScrNavOps::Up :
            eKey = tCIDCtrls::EExtKeys::Up;
            break;

        default :
            return;
    };

    // And queue up a key event for it
    facCQCIntfView.wndMain().AddCtrlEvent
    (
        new TIntfCtrlExtKeyEv
        (
            eKey, kCIDLib::False, kCIDLib::False, bShift
        )
    );
}


//
//  This one we can just handle here. We just queue it up to be played
//  and return.
//
tCIDLib::TVoid TIntfCtrlServer::PlayWAV(const TString& strPath)
{
    TAudio::PlayWAVFile(strPath, tCIDLib::EWaitModes::NoWait);
}


// Just add it to the viewer's queue of things to process
tCIDLib::TVoid
TIntfCtrlServer::SetGlobalVar(  const   TString&    strVarName
                                , const TString&    strValue)
{
    facCQCIntfView.wndMain().AddCtrlEvent
    (
        new TIntfCtrlSetVarEv(strVarName, strValue)
    );
}



// ---------------------------------------------------------------------------
//  TIntfCtrlServer: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TIntfCtrlServer::StoreStatus(const  TString&            strBaseTmpl
                            , const TString&            strTopTmpl
                            , const tCIDLib::TCard4     c4LayerCnt
                            , const tCIDLib::TBoolean   bMaximized)
{
    // Synchronize and store the data
    TLocker lockrSync(&m_mtxSync);

    m_bMaximized = bMaximized;
    m_c4LayerCount = c4LayerCnt;
    m_strBaseTemplate = strBaseTmpl;
    m_strTopTemplate = strTopTmpl;
}


// ---------------------------------------------------------------------------
//  TIntfCtrlServer: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TIntfCtrlServer::Initialize()
{
    // Nothing to do at this time
}


tCIDLib::TVoid TIntfCtrlServer::Terminate()
{
    // Nothing to do at this time
}

