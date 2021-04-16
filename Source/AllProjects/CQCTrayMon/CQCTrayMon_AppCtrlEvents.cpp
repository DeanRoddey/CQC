//
// FILE NAME: CQCTrayMon_AppCtrlEvents.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 05/06/2004
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
//  This file implements the events that the app control related server
//  threads post to the app control tab window.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCTrayMon.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TAppUtilEv,TObject)
RTTIDecls(TAppFocusEv,TObject)
RTTIDecls(TAppInvFileEv,TObject)
RTTIDecls(TAppRemStartEv,TObject)
RTTIDecls(TAppSendExtKeyEv,TObject)
RTTIDecls(TAppSendKeyEv,TObject)
RTTIDecls(TAppSendMsgEv,TObject)
RTTIDecls(TAppStartEv,TObject)
RTTIDecls(TAppStdOpEv,TObject)




// ---------------------------------------------------------------------------
//   CLASS: TAppUtilEv
//  PREFIX: aue
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TAppUtilEv: Destructor
// ---------------------------------------------------------------------------
TAppUtilEv::~TAppUtilEv()
{
}


// ---------------------------------------------------------------------------
//  TAppUtilEv: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TCard4 TAppUtilEv::c4ChildId() const
{
    return m_c4ChildId;
}


tCIDLib::TCard4 TAppUtilEv::c4Id() const
{
    return m_c4Id;
}


const TString& TAppUtilEv::strMoniker() const
{
    return m_strMoniker;
}


// ---------------------------------------------------------------------------
//  TAppUtilEv: Hidden constructors
// ---------------------------------------------------------------------------
TAppUtilEv::TAppUtilEv( const   TString&                strMoniker
                        , const tCIDLib::TCard4         c4Id
                        , const tCIDLib::TCard4         c4ChildId) :

    m_c4ChildId(c4ChildId)
    , m_c4Id(c4Id)
    , m_strMoniker(strMoniker)
{
}

TAppUtilEv::TAppUtilEv(const TString& strMoniker) :

    m_c4ChildId(0)
    , m_c4Id(0)
    , m_strMoniker(strMoniker)
{
}



// ---------------------------------------------------------------------------
//   CLASS: TAppFocusEv
//  PREFIX: aue
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TAppSendExtKeyEv: Constructors and Destructor
// ---------------------------------------------------------------------------
TAppFocusEv::TAppFocusEv(const  TString&            strMoniker
                        , const tCIDLib::TCard4     c4Id
                        , const tCIDLib::TCard4     c4ChildId) :

    TAppUtilEv(strMoniker, c4Id, c4ChildId)
{
}

TAppFocusEv::~TAppFocusEv()
{
}


// ---------------------------------------------------------------------------
//  TAppFocusEv: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TAppFocusEv::ProcessEv(TCQCTrayAppCtrlTab& wndTab, TAppCtrlServerImpl& orbsAC)
{
    wndTab.ForceToFront();

    TLocker lockrSync(&orbsAC.mtxSync());
    TAppCtrlRec* pacrTar = orbsAC.pacrFind(strMoniker());
    if (pacrTar)
    {
        try
        {
            pacrTar->SetFocus(c4Id(), c4ChildId());
        }

        catch(TError& errToCatch)
        {
            if (facCQCTrayMon.bShouldLog(errToCatch))
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }

            // Put something into the event log window
            wndTab.AddStatusEvent
            (
                facCQCTrayMon.strMsg(kTrayMonMsgs::midStatus_Focus, strMoniker())
            );
        }

        catch(...)
        {
            wndTab.AddStatusEvent
            (
                facCQCTrayMon.strMsg(kTrayMonMsgs::midStatus_Focus, strMoniker())
            );
        }
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TAppInvFileEv
//  PREFIX: aue
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TAppInvFileEv: Constructors and Destructor
// ---------------------------------------------------------------------------
TAppInvFileEv::TAppInvFileEv(const  TString&                strPath
                            , const tCIDLib::EExtProcShows  eShow) :

    TAppUtilEv(TString::strEmpty())
    , m_eShow(eShow)
    , m_strPath(strPath)
{
}

TAppInvFileEv::TAppInvFileEv(const  TString&                strMoniker
                            , const TString&                strPath
                            , const tCIDLib::EExtProcShows  eShow) :

    TAppUtilEv(strMoniker)
    , m_eShow(eShow)
    , m_strPath(strPath)
{
}


TAppInvFileEv::~TAppInvFileEv()
{
}


// ---------------------------------------------------------------------------
//  TAppInvFileEv: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TAppInvFileEv::ProcessEv(TCQCTrayAppCtrlTab& wndTab, TAppCtrlServerImpl& orbsAC)
{
    try
    {
        //
        //  If no moniker, then just do a generic operation here. Else,
        //  look up the app and start it.
        //
        if (strMoniker().bIsEmpty())
        {
            //
            //  We need to translate the external app show stuff to the
            //  window pos form for this. We only support a subset of
            //  options.
            //
            tCIDCtrls::EPosStates ePosState = tCIDCtrls::EPosStates::Restored;
            switch(m_eShow)
            {
                case tCIDLib::EExtProcShows::Maximized :
                    ePosState = tCIDCtrls::EPosStates::Maximized;
                    break;

                case tCIDLib::EExtProcShows::Minimized :
                    ePosState = tCIDCtrls::EPosStates::Minimized;
                    break;

                case tCIDLib::EExtProcShows::Normal :
                default :
                    break;
            };

            wndTab.AddStatusEvent
            (
                facCQCTrayMon.strMsg(kTrayMonMsgs::midStatus_InvokeFile, m_strPath)
            );
            TGUIShell::InvokeDefFileAction(wndTab, m_strPath, ePosState);
        }
         else
        {
            wndTab.ForceToFront();

            // Lock while we do this
            TLocker lockrSync(&orbsAC.mtxSync());
            TAppCtrlRec* pacrStart = orbsAC.pacrFind(strMoniker());

            //
            //  If already running, then stop it and start it up again
            //  on the new guy.
            //
            if (pacrStart)
            {
                if (pacrStart->bIsRunning())
                {
                    pacrStart->bStop();
                    tCIDLib::EExitCodes eExit;
                    if (pacrStart->bWaitForDeath(eExit, 7000))
                    {
                        // Send the main window a status message
                        wndTab.AddStatusEvent
                        (
                            facCQCTrayMon.strMsg
                            (
                                kTrayMonMsgs::midStatus_StopFailed
                                , pacrStart->strTitle()
                            )
                        );
                        return;
                    }
                }

                tCIDLib::TBoolean bRes = pacrStart->bStartViaOpen(m_strPath, m_eShow);
                if (!bRes)
                {
                    // Send the main window a status message
                    wndTab.AddStatusEvent
                    (
                        facCQCTrayMon.strMsg
                        (
                            kTrayMonMsgs::midStatus_StartFailed
                            , pacrStart->strTitle()
                        )
                    );
                    return;
                }
            }
        }
    }

    catch(TError& errToCatch)
    {
        if (facCQCTrayMon.bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        // Put something into the event log window
        wndTab.AddStatusEvent
        (
            facCQCTrayMon.strMsg(kTrayMonMsgs::midStatus_FInvExcept, m_strPath)
        );
    }

    catch(...)
    {
        wndTab.AddStatusEvent
        (
            facCQCTrayMon.strMsg(kTrayMonMsgs::midStatus_FInvExcept, m_strPath)
        );
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TAppRemStartEv
//  PREFIX: aue
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TAppRemStartEv: Constructors and Destructor
// ---------------------------------------------------------------------------
TAppRemStartEv::TAppRemStartEv( const   TString&    strPath
                                , const TString&    strParams
                                , const TString&    strInitPath) :

    TAppUtilEv(TString::strEmpty())
    , m_strInitPath(strInitPath)
    , m_strPath(strPath)
    , m_strParams(strParams)
{
}

TAppRemStartEv::~TAppRemStartEv()
{
}


// ---------------------------------------------------------------------------
//  TAppRemStartEv: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TAppRemStartEv::ProcessEv(TCQCTrayAppCtrlTab& wndTab, TAppCtrlServerImpl& orbsAC)
{
    // Log a message to the status window if above low logging level
    if (facCQCTrayMon.eVerboseLvl() > tCQCKit::EVerboseLvls::Low)
        wndTab.AddStatusEvent(TStrCat(L"Running app: ", m_strPath));

    //
    //  No need to lock here. We aren't dealing with registred app control
    //  records. And we don't wait for it to start.
    //
    try
    {
        wndTab.ForceToFront();

        TString strFullCmdLine(L"\"", 256);
        strFullCmdLine += m_strPath;
        strFullCmdLine += L"\" ";
        strFullCmdLine += m_strParams;
        TExternalProcess extpStart;
        extpStart.Start(strFullCmdLine, m_strInitPath);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        // Put something into the event log window
        wndTab.AddStatusEvent
        (
            facCQCTrayMon.strMsg(kTrayMonMsgs::midStatus_StartFailed, m_strPath)
        );
    }

    catch(...)
    {
        wndTab.AddStatusEvent
        (
            facCQCTrayMon.strMsg(kTrayMonMsgs::midStatus_StartFailed, m_strPath)
        );
    }
}




// ---------------------------------------------------------------------------
//   CLASS: TAppSendMsgEv
//  PREFIX: aue
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TAppSendMsgEv: Constructors and Destructor
// ---------------------------------------------------------------------------
TAppSendMsgEv::TAppSendMsgEv(const  TString&            strMoniker
                            , const tCIDLib::TCard4     c4Id
                            , const tCIDLib::TCard4     c4ChildId
                            , const tCIDCtrls::TWndMsg  wmsgToSend
                            , const tCIDCtrls::TWParam  wParm
                            , const tCIDCtrls::TLParam  lParm) :

    TAppUtilEv(strMoniker, c4Id, c4ChildId)
    , m_lParm(lParm)
    , m_wParm(wParm)
    , m_wmsgToSend(wmsgToSend)
{
}

TAppSendMsgEv::~TAppSendMsgEv()
{
}


// ---------------------------------------------------------------------------
//  TAppSendMsgEv: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TAppSendMsgEv::ProcessEv(TCQCTrayAppCtrlTab& wndTab, TAppCtrlServerImpl& orbsAC)
{
    TLocker lockrSync(&orbsAC.mtxSync());
    TAppCtrlRec* pacrTar = orbsAC.pacrFind(strMoniker());
    if (pacrTar)
    {
        try
        {
            // Do the standard processing to be sure we can send the key
            facCIDCtrls().PrepForeignFocus
            (
                wndTab.hwndThis(), pacrTar->hwndFindWindow(c4Id(), c4ChildId())
            );

            pacrTar->i4SendMsg
            (
                c4Id(), c4ChildId(), m_wmsgToSend, m_wParm, m_lParm, kCIDLib::True
            );
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
            wndTab.AddStatusEvent
            (
                facCQCTrayMon.strMsg(kTrayMonMsgs::midStatus_SendMsg, strMoniker())
            );
        }

        catch(...)
        {
            wndTab.AddStatusEvent
            (
                facCQCTrayMon.strMsg(kTrayMonMsgs::midStatus_SendMsg, strMoniker())
            );
        }
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TAppSendExtKeyEv
//  PREFIX: aue
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TAppSendExtKeyEv: Constructors and Destructor
// ---------------------------------------------------------------------------
TAppSendExtKeyEv::TAppSendExtKeyEv( const   TString&            strMoniker
                                    , const tCIDLib::TCard4     c4Id
                                    , const tCIDLib::TCard4     c4ChildId
                                    , const tCIDCtrls::EExtKeys eToSend
                                    , const tCIDLib::TBoolean   bAltShift
                                    , const tCIDLib::TBoolean   bCtrlShift
                                    , const tCIDLib::TBoolean   bShift) :

    TAppUtilEv(strMoniker, c4Id, c4ChildId)
    , m_bAltShift(bAltShift)
    , m_bCtrlShift(bCtrlShift)
    , m_bShift(bShift)
    , m_eToSend(eToSend)
{
}

TAppSendExtKeyEv::~TAppSendExtKeyEv()
{
}


// ---------------------------------------------------------------------------
//  TAppSendExtKeyEv: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TAppSendExtKeyEv::ProcessEv(TCQCTrayAppCtrlTab& wndTab, TAppCtrlServerImpl& orbsAC)
{
    TLocker lockrSync(&orbsAC.mtxSync());
    TAppCtrlRec* pacrTar = orbsAC.pacrFind(strMoniker());
    if (pacrTar)
    {
        try
        {
            // Make sure this guy is the foreground window
            facCIDCtrls().PrepForeignFocus
            (
                wndTab.hwndThis(), pacrTar->hwndFindWindow(c4Id(), c4ChildId())
            );

            pacrTar->SendExtKey
            (
                c4Id()
                , c4ChildId()
                , m_eToSend
                , m_bAltShift
                , m_bCtrlShift
                , m_bShift
            );
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);

            wndTab.AddStatusEvent
            (
                facCQCTrayMon.strMsg(kTrayMonMsgs::midStatus_SendExtKey, strMoniker())
            );
        }

        catch(...)
        {
            wndTab.AddStatusEvent
            (
                facCQCTrayMon.strMsg(kTrayMonMsgs::midStatus_SendExtKey, strMoniker())
            );
        }
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TAppSendKeyEv
//  PREFIX: aue
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TAppSendKeyEv: Constructors and Destructor
// ---------------------------------------------------------------------------
TAppSendKeyEv::TAppSendKeyEv(const  TString&            strMoniker
                            , const tCIDLib::TCard4     c4Id
                            , const tCIDLib::TCard4     c4ChildId
                            , const tCIDLib::TCh        chToSend
                            , const tCIDLib::TBoolean   bAltShift
                            , const tCIDLib::TBoolean   bCtrlShift
                            , const tCIDLib::TBoolean   bShift) :

    TAppUtilEv(strMoniker, c4Id, c4ChildId)
    , m_bAltShift(bAltShift)
    , m_bCtrlShift(bCtrlShift)
    , m_bShift(bShift)
    , m_chToSend(chToSend)
{
}

TAppSendKeyEv::~TAppSendKeyEv()
{
}


// ---------------------------------------------------------------------------
//  TAppSendKeyEv: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TAppSendKeyEv::ProcessEv(TCQCTrayAppCtrlTab& wndTab, TAppCtrlServerImpl& orbsAC)
{
    TLocker lockrSync(&orbsAC.mtxSync());
    TAppCtrlRec* pacrTar = orbsAC.pacrFind(strMoniker());
    if (pacrTar)
    {
        try
        {
            // Make sure this guy is the foreground window
            facCIDCtrls().PrepForeignFocus
            (
                wndTab.hwndThis(), pacrTar->hwndFindWindow(c4Id(), c4ChildId())
            );

            pacrTar->SendKey
            (
                c4Id()
                , c4ChildId()
                , m_chToSend
                , m_bAltShift
                , m_bCtrlShift
                , m_bShift
            );
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
            wndTab.AddStatusEvent
            (
                facCQCTrayMon.strMsg(kTrayMonMsgs::midStatus_SendKey, strMoniker())
            );
        }

        catch(...)
        {
            wndTab.AddStatusEvent
            (
                facCQCTrayMon.strMsg(kTrayMonMsgs::midStatus_SendKey, strMoniker())
            );
        }
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TAppStdOpEv
//  PREFIX: aue
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TAppStdOpEv: Constructors and Destructor
// ---------------------------------------------------------------------------
TAppStdOpEv::TAppStdOpEv(const  TString&            strMoniker
                        , const tCIDLib::TCard4     c4Id
                        , const tCIDLib::TCard4     c4ChildId
                        , const tCQCKit::EStdACOps  eOp) :

    TAppUtilEv(strMoniker, c4Id, c4ChildId)
    , m_eOp(eOp)
{
}

TAppStdOpEv::~TAppStdOpEv()
{
}


// ---------------------------------------------------------------------------
//  TAppStdOpEv: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TAppStdOpEv::ProcessEv(TCQCTrayAppCtrlTab& wndTab, TAppCtrlServerImpl& orbsAC)
{
    //
    //  If we have to bring the other guy to the top, then we have come to
    //  the top first. We are off-screen, so the user won't notice this.
    //
    if (m_eOp == tCQCKit::EStdACOps::ToTop)
        wndTab.ForceToFront();

    TLocker lockrSync(&orbsAC.mtxSync());
    TAppCtrlRec* pacrTar = orbsAC.pacrFind(strMoniker());
    if (pacrTar)
    {
        try
        {
            pacrTar->StandardOp(c4Id(), c4ChildId(), m_eOp);
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
            wndTab.AddStatusEvent
            (
                facCQCTrayMon.strMsg(kTrayMonMsgs::midStatus_StdOp, strMoniker())
            );
        }

        catch(...)
        {
            wndTab.AddStatusEvent
            (
                facCQCTrayMon.strMsg(kTrayMonMsgs::midStatus_StdOp, strMoniker())
            );
        }
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TAppStartEv
//  PREFIX: aue
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TAppStartEv: Constructors and Destructor
// ---------------------------------------------------------------------------
TAppStartEv::TAppStartEv(const TString& strMoniker) :

    TAppUtilEv(strMoniker)
{
}

TAppStartEv::TAppStartEv(const  TString&    strMoniker
                        , const TString&    strParams) :

    TAppUtilEv(strMoniker)
    , m_strParams(strParams)
{
}

TAppStartEv::~TAppStartEv()
{
}


// ---------------------------------------------------------------------------
//  TAppStartEv: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TAppStartEv::ProcessEv(TCQCTrayAppCtrlTab& wndTab, TAppCtrlServerImpl& orbsAC)
{
    wndTab.ForceToFront();
    try
    {
        // Lock while we do this
        TLocker lockrSync(&orbsAC.mtxSync());
        TAppCtrlRec* pacrStart = orbsAC.pacrFind(strMoniker());

        // If already running, then bring it forward
        if (pacrStart)
        {
            if (pacrStart->bIsRunning())
            {
                pacrStart->Activate();
            }
             else
            {
                //
                //  Call one or other of the start methods, according to
                //  whether we got parameter overrides.
                //
                //  NOTE:   We don't have a single start and just pass an
                //          empty string if no overrides, since the override
                //          might be to pass no parameters.
                //
                tCIDLib::TBoolean bRes = kCIDLib::False;
                if (m_strParams.bIsEmpty())
                    bRes = pacrStart->bStart();
                else
                    bRes = pacrStart->bStart(m_strParams);

                if (!bRes)
                {
                    // Send the main window a status message
                    wndTab.AddStatusEvent
                    (
                        facCQCTrayMon.strMsg
                        (
                            kTrayMonMsgs::midStatus_StartFailed
                            , pacrStart->strTitle()
                        )
                    );
                }
            }
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);
        wndTab.AddStatusEvent
        (
            facCQCTrayMon.strMsg(kTrayMonMsgs::midStatus_StartExcept, strMoniker())
        );
    }

    catch(...)
    {
        wndTab.AddStatusEvent
        (
            facCQCTrayMon.strMsg(kTrayMonMsgs::midStatus_StartExcept, strMoniker())
        );
    }
}


