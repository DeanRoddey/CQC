//
// FILE NAME: CQCAppCtrl_ProtoServerImpl.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/15/2004
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
//  This implements the IDL generated server stub for the protocol interface
//  that the app control driver uses to talk to this server.
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
#include    "CQCTrayMon.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TAppCtrlServerImpl,TAppCtrlServerBase)



// ---------------------------------------------------------------------------
//   CLASS: TAppCtrlServerImpl
//  PREFIX: acr
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TAppCtrlServerImpl: Constructors and Destructor
// ---------------------------------------------------------------------------
TAppCtrlServerImpl::TAppCtrlServerImpl(TCQCTrayAppCtrlTab* const pwndACTab) :

    m_colApps(tCIDLib::EAdoptOpts::Adopt, 109, TStringKeyOps(), &TAppCtrlRec::strKey)
    , m_pwndTab(pwndACTab)
{
}

TAppCtrlServerImpl::~TAppCtrlServerImpl()
{
}


// ---------------------------------------------------------------------------
//  TAppCtrlServerImpl: Public, inherited methods
// ---------------------------------------------------------------------------
TAppCtrlRec& TAppCtrlServerImpl::acrFind(const TString& strMoniker)
{
    TAppCtrlRec* pacrRet = m_colApps.pobjFindByKey(strMoniker, kCIDLib::False);
    if (!pacrRet)
    {
        facCQCTrayMon.ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kTrayMonErrs::errcApp_NotFound
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , strMoniker
        );
    }
    return *pacrRet;
}


tCIDLib::TVoid
TAppCtrlServerImpl::AddRecord(  const   TString&    strMoniker
                                , const TString&    strTitle
                                , const TString&    strAppName
                                , const TString&    strParams
                                , const TString&    strWorkingDir)
{
    //
    //  Lock our sync mutex, and see if we have this record already. If not,
    //  then we need to add it. If we do, then update the parameters in case
    //  they've changed.
    //
    TLocker lockrSync(&m_mtxSync);

    // See if already have a record under this moniker
    TAppCtrlRec* pacrFind = m_colApps.pobjFindByKey(strMoniker, kCIDLib::False);

    // If so, just update the info, else creaet a new one
    if (pacrFind)
    {
        pacrFind->Update(strTitle, strAppName, strParams, strWorkingDir);

        // Let the notify window that we've changed this guy's config
        m_pwndTab->AddStatusEvent
        (
            strMoniker, strTitle, tCQCTrayMon::EListEvs::RecConfig
        );
    }
     else
    {
        m_colApps.Add
        (
            new TAppCtrlRec
            (
                strMoniker, strTitle, strAppName, strParams, strWorkingDir
            )
        );

        // Let the notify window that we've changed the list
        m_pwndTab->AddStatusEvent
        (
            strMoniker, strTitle, tCQCTrayMon::EListEvs::Add
        );
    }
}


tCIDLib::TBoolean
TAppCtrlServerImpl::bAppRegistered( const   TString&    strMoniker
                                    , const TString&    strAppTitle)
{
    TLocker lockrSync(&m_mtxSync);
    TAppCtrlRec* pacrRet = m_colApps.pobjFindByKey(strMoniker, kCIDLib::False);
    if (!pacrRet)
        return kCIDLib::False;
    return (pacrRet->strTitle() == strAppTitle);
}


tCIDLib::TBoolean TAppCtrlServerImpl::bIsRunning(const TString& strMoniker)
{
    // Lock while we do this
    TLocker lockrSync(&m_mtxSync);
    TAppCtrlRec& acrCheck = acrFind(strMoniker);
    return acrCheck.bIsRunning();
}


tCIDLib::TBoolean
TAppCtrlServerImpl::bInvokeFile(const   TString&    strPath
                                , const TString&    strShowType)
{
    //
    //  We support a few replacement tokens to refer to standard system
    //  directories. So check for the path starting with those and replace
    //  them.
    //
    tCIDLib::TCard4 c4RepLen = 0;
    TString strActual(strPath);
    TString strRepVal;

    if (strPath.bStartsWithI(L"%allstartmenu%"))
    {
        strRepVal = TSysInfo::strSpecialPath(tCIDLib::ESpecialPaths::CommonStartMenu);
        c4RepLen = 14;
    }
     else if (strPath.bStartsWithI(L"%userstartmenu%"))
    {
        strRepVal = TSysInfo::strSpecialPath(tCIDLib::ESpecialPaths::UserStartMenu);
        c4RepLen = 15;
    }
     else if (strPath.bStartsWithI(L"%mymusic%"))
    {
        strRepVal = TSysInfo::strSpecialPath(tCIDLib::ESpecialPaths::MyMusic);
        c4RepLen = 9;
    }
     else if (strPath.bStartsWithI(L"%appdata%"))
    {
        strRepVal = TSysInfo::strSpecialPath(tCIDLib::ESpecialPaths::UserAppData);
        c4RepLen = 9;
    }

    // If we got a replacement value do that
    if (c4RepLen)
    {
        strActual.Cut(0, c4RepLen);

        // If we'd end up with two path separators, remove one
        if ((strActual.chFirst() == kCIDLib::chPathSep)
        &&  (strRepVal.chLast() == kCIDLib::chPathSep))
        {
            strActual.DeleteLast();
        }
        strActual.Prepend(strRepVal);
    }

    tCIDLib::EExtProcShows eShow= tCIDLib::EExtProcShows::Normal;
    if (strShowType == L"Max")
        eShow = tCIDLib::EExtProcShows::Maximized;
    else if (strShowType == L"Min")
        eShow = tCIDLib::EExtProcShows::Minimized;

    m_pwndTab->AddACEvent(new TAppInvFileEv(strActual, eShow));
    return kCIDLib::True;
}


tCIDLib::TBoolean
TAppCtrlServerImpl::bIsVisible( const   TString&        strMoniker
                                , const tCIDLib::TCard4 c4Id
                                , const tCIDLib::TCard4 c4ChildId)
{
    // Lock while we do this
    TLocker lockrSync(&m_mtxSync);
    TAppCtrlRec& acrCheck = acrFind(strMoniker);
    return acrCheck.bIsVisible(c4Id, c4ChildId);
}


tCIDLib::TBoolean TAppCtrlServerImpl::bStart(const TString& strMoniker)
{
    m_pwndTab->AddACEvent(new TAppStartEv(strMoniker));
    return kCIDLib::True;
}


//
//  This one just asks us to start up some application. It's not one that
//  we will control or have a driver for, they just are using us a remote
//  app starter.
//
tCIDLib::TBoolean
TAppCtrlServerImpl::bStartApp(  const   TString&    strAppPath
                                , const TString&    strParams
                                , const TString&    strInitPath)
{
    m_pwndTab->AddACEvent
    (
        new TAppRemStartEv(strAppPath, strParams, strInitPath)
    );

    return kCIDLib::True;
}


tCIDLib::TBoolean
TAppCtrlServerImpl::bStartWithParams(const  TString&    strMoniker
                                    , const TString&    strParams)
{
    m_pwndTab->AddACEvent
    (
        new TAppStartEv(strMoniker, strParams)
    );
    return kCIDLib::True;
}


tCIDLib::TBoolean
TAppCtrlServerImpl::bStartAppViaOpen(const  TString&    strMoniker
                                    , const TString&    strDocPath
                                    , const TString&    strInitPath
                                    , const TString&    strShow)
{
    tCIDLib::EExtProcShows eShow= tCIDLib::EExtProcShows::Normal;
    if (strShow == L"Max")
        eShow = tCIDLib::EExtProcShows::Maximized;
    else if (strShow == L"Min")
        eShow = tCIDLib::EExtProcShows::Minimized;

    m_pwndTab->AddACEvent(new TAppInvFileEv(strMoniker, strDocPath, eShow));
    return kCIDLib::True;
}


tCIDLib::TBoolean TAppCtrlServerImpl::bStop(const TString& strMoniker)
{
    TLocker lockrSync(&m_mtxSync);
    TAppCtrlRec& acrStop = acrFind(strMoniker);

    // If not running, just return true
    if (!acrStop.bIsRunning())
        return kCIDLib::True;

    tCIDLib::TBoolean bRet;
    try
    {
        bRet = acrStop.bStop();
    }

    catch(const TError& errToCatch)
    {
        if (facCQCTrayMon.bShouldLog(errToCatch))
            TModule::LogEventObj(errToCatch);
        bRet = kCIDLib::False;
    }
    return bRet;
}


tCIDLib::TCard4
TAppCtrlServerImpl::c4AddWindow(const   TString& strMoniker
                                , const TString& strPath)
{
    TLocker lockrSync(&m_mtxSync);
    TAppCtrlRec& acrStop = acrFind(strMoniker);
    return acrStop.c4AddWindow(strPath);
}


tCIDLib::TCard4
TAppCtrlServerImpl::c4AddWindowByClass( const   TString& strMoniker
                                        , const TString& strClass)
{
    TLocker lockrSync(&m_mtxSync);
    TAppCtrlRec& acrStop = acrFind(strMoniker);
    return acrStop.c4AddWindowByClass(strClass);
}


tCIDLib::TCard4
TAppCtrlServerImpl::c4QueryListSel( const   TString&        strMoniker
                                    , const tCIDLib::TCard4 c4Id
                                    , const tCIDLib::TCard4 c4ChildId)
{
    TLocker lockrSync(&m_mtxSync);
    TAppCtrlRec& acrStop = acrFind(strMoniker);
    return acrStop.c4QueryListSel(c4Id, c4ChildId);
}


tCIDLib::TCard4
TAppCtrlServerImpl::c4QueryText(const   TString&        strMoniker
                                , const tCIDLib::TCard4 c4Id
                                , const tCIDLib::TCard4 c4ChildId
                                ,       TString&        strToFill)
{
    TLocker lockrSync(&m_mtxSync);
    TAppCtrlRec& acrStop = acrFind(strMoniker);
    return acrStop.c4QueryText(c4Id, c4ChildId, strToFill);
}


tCIDLib::TCard4
TAppCtrlServerImpl::c4QueryWndStyles(const  TString&        strMoniker
                                    , const tCIDLib::TCard4 c4Id
                                    , const tCIDLib::TCard4 c4ChildId)
{
    TLocker lockrSync(&m_mtxSync);
    TAppCtrlRec& acrStop = acrFind(strMoniker);
    return acrStop.c4QueryWndStyles(c4Id, c4ChildId);
}


tCIDLib::TInt4
TAppCtrlServerImpl::i4QueryTrackBar(const   TString&        strMoniker
                                    , const tCIDLib::TCard4 c4Id
                                    , const tCIDLib::TCard4 c4ChildId)
{
    TLocker lockrSync(&m_mtxSync);
    TAppCtrlRec& acrStop = acrFind(strMoniker);
    return acrStop.i4QueryTrackBar(c4Id, c4ChildId);
}


tCIDLib::TInt4
TAppCtrlServerImpl::i4SendMsg(  const   TString&            strMoniker
                                , const tCIDLib::TCard4     c4Id
                                , const tCIDLib::TCard4     c4ChildId
                                , const tCIDLib::TCard4     c4ToSend
                                , const tCIDLib::TCard4     c4Param1
                                , const tCIDLib::TInt4      i4Param2
                                , const tCIDLib::TBoolean   bAsync)
{
    TLocker lockrSync(&m_mtxSync);
    TAppCtrlRec& acrStop = acrFind(strMoniker);
    return acrStop.i4SendMsg
    (
        c4Id, c4ChildId, c4ToSend, c4Param1, i4Param2, bAsync
    );
}


TAppCtrlRec* TAppCtrlServerImpl::pacrFind(const TString& strMoniker)
{
    return m_colApps.pobjFindByKey(strMoniker, kCIDLib::False);
}


tCIDLib::TVoid TAppCtrlServerImpl::Ping()
{
    // Nothing to do, just being here is all that matters for this one
}


tCIDLib::TVoid
TAppCtrlServerImpl::RemoveRecord(const TString& strMoniker)
{
    TLocker lockrSync(&m_mtxSync);

    //
    //  If there is a record with this key, then remove it, then send the
    //  appropriate notification to the main window.
    //
    if (m_colApps.bRemoveKey(strMoniker))
    {
        m_pwndTab->AddStatusEvent
        (
            strMoniker, tCQCTrayMon::EListEvs::Remove
        );
    }
}


tCIDLib::TVoid
TAppCtrlServerImpl::SendExtKey( const   TString&            strMoniker
                                , const tCIDLib::TCard4     c4Id
                                , const tCIDLib::TCard4     c4ChildId
                                , const tCIDCtrls::EExtKeys eKeyToSend
                                , const tCIDLib::TBoolean   bAltShifted
                                , const tCIDLib::TBoolean   bCtrlShifted
                                , const tCIDLib::TBoolean   bShifted)
{
    m_pwndTab->AddACEvent
    (
        new TAppSendExtKeyEv
        (
            strMoniker
            , c4Id
            , c4ChildId
            , eKeyToSend
            , bAltShifted
            , bCtrlShifted
            , bShifted
        )
    );
}

tCIDLib::TVoid
TAppCtrlServerImpl::SendKey(const   TString&            strMoniker
                            , const tCIDLib::TCard4     c4Id
                            , const tCIDLib::TCard4     c4ChildId
                            , const tCIDLib::TCh        chToSend
                            , const tCIDLib::TBoolean   bAltShifted
                            , const tCIDLib::TBoolean   bCtrlShifted
                            , const tCIDLib::TBoolean   bShifted)
{
    m_pwndTab->AddACEvent
    (
        new TAppSendKeyEv
        (
            strMoniker
            , c4Id
            , c4ChildId
            , chToSend
            , bAltShifted
            , bCtrlShifted
            , bShifted
        )
    );
}


tCIDLib::TVoid
TAppCtrlServerImpl::SetFocus(const  TString&        strMoniker
                            , const tCIDLib::TCard4 c4Id
                            , const tCIDLib::TCard4 c4ChildId)
{
    // We have to do this one through the utility window
    m_pwndTab->AddACEvent
    (
        new TAppFocusEv(strMoniker, c4Id, c4ChildId)
    );
}


tCIDLib::TVoid
TAppCtrlServerImpl::SetListSel( const   TString&        strMoniker
                                , const tCIDLib::TCard4 c4Id
                                , const tCIDLib::TCard4 c4ChildId
                                , const tCIDLib::TCard4 c4Index)
{
    TLocker lockrSync(&m_mtxSync);
    TAppCtrlRec& acrStop = acrFind(strMoniker);
    acrStop.SetListSel(c4Id, c4ChildId, c4Index);
}


tCIDLib::TVoid
TAppCtrlServerImpl::SetTrackBar(const   TString&        strMoniker
                                , const tCIDLib::TCard4 c4Id
                                , const tCIDLib::TCard4 c4ChildId
                                , const tCIDLib::TInt4  i4ToSet)
{
    TLocker lockrSync(&m_mtxSync);
    TAppCtrlRec& acrStop = acrFind(strMoniker);
    acrStop.SetTrackBar(c4Id, c4ChildId, i4ToSet);
}


tCIDLib::TVoid
TAppCtrlServerImpl::SetWindowPos(const  TString&        strMoniker
                                , const tCIDLib::TCard4 c4Id
                                , const tCIDLib::TCard4 c4ChildId
                                , const tCIDLib::TInt4  i4X
                                , const tCIDLib::TInt4  i4Y)
{
    TLocker lockrSync(&m_mtxSync);
    TAppCtrlRec& acrStop = acrFind(strMoniker);
    acrStop.SetWindowPos(c4Id, c4ChildId, i4X, i4Y);
}

tCIDLib::TVoid
TAppCtrlServerImpl::SetWindowSize(  const   TString&        strMoniker
                                    , const tCIDLib::TCard4 c4Id
                                    , const tCIDLib::TCard4 c4ChildId
                                    , const tCIDLib::TCard4 c4Width
                                    , const tCIDLib::TCard4 c4Height)
{
    TLocker lockrSync(&m_mtxSync);
    TAppCtrlRec& acrStop = acrFind(strMoniker);
    acrStop.SetWindowSize(c4Id, c4ChildId, c4Width, c4Height);
}


tCIDLib::TVoid
TAppCtrlServerImpl::SetWindowText(  const   TString&        strMoniker
                                    , const tCIDLib::TCard4 c4Id
                                    , const tCIDLib::TCard4 c4ChildId
                                    , const TString&        strText)
{
    TLocker lockrSync(&m_mtxSync);
    TAppCtrlRec& acrStop = acrFind(strMoniker);
    acrStop.SetWindowText(c4Id, c4ChildId, strText);
}


tCIDLib::TVoid
TAppCtrlServerImpl::ShowWindow( const   TString&            strMoniker
                                , const tCIDLib::TCard4     c4Id
                                , const tCIDLib::TCard4     c4ChildId
                                , const tCIDLib::TBoolean   bState)
{
    TLocker lockrSync(&m_mtxSync);
    TAppCtrlRec& acrStop = acrFind(strMoniker);
    acrStop.ShowWindow(c4Id, c4ChildId, bState);
}


tCIDLib::TVoid
TAppCtrlServerImpl::StandardOp( const   TString&            strMoniker
                                , const tCIDLib::TCard4     c4Id
                                , const tCIDLib::TCard4     c4ChildId
                                , const tCQCKit::EStdACOps  eOp)
{
    m_pwndTab->AddACEvent
    (
        new TAppStdOpEv(strMoniker, c4Id, c4ChildId, eOp)
    );
}


// ---------------------------------------------------------------------------
//  TAppCtrlServerImpl: Public, non-virtual methods
// ---------------------------------------------------------------------------
const tCQCTrayMon::TAppList& TAppCtrlServerImpl::colAppList() const
{
    return m_colApps;
}


TMutex& TAppCtrlServerImpl::mtxSync()
{
    return m_mtxSync;
}


// ---------------------------------------------------------------------------
//  TAppCtrlServerImpl: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TAppCtrlServerImpl::Initialize()
{
    // Nothing to do at this time
}


tCIDLib::TVoid TAppCtrlServerImpl::Terminate()
{
    // Nothing to do at this time
}


