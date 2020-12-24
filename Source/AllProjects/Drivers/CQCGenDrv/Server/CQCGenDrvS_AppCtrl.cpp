//
// FILE NAME: CQCMacroEngS_AppCtrl.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/13/2004
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
//
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


// ---------------------------------------------------------------------------
//  Facility specific includes
// ---------------------------------------------------------------------------
#include    "CQCGenDrvS_.hpp"


// ---------------------------------------------------------------------------
//  Magic RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCAppCtrlVal,TMEngClassVal)
RTTIDecls(TCQCAppCtrlInfo,TMEngClassInfo)



// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace CQCGenDrvS_AppCtrl
{
    namespace
    {
        // -----------------------------------------------------------------------
        //  The names for the types that we support here. Each derivative has to
        //  be able to return strings that contain its name and full name.
        // -----------------------------------------------------------------------
        const TString   strBasePath(L"MEng.System.CQC.Runtime");
        const TString   strClassName(L"AppCtrlClient");
        const TString   strClassPath(L"MEng.System.CQC.Runtime.AppCtrlClient");
    }
}



// ---------------------------------------------------------------------------
//  CLASS: TCQCAppCtrlVal
// PREFIX: mecv
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCAppCtrlVal: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCAppCtrlVal::TCQCAppCtrlVal( const   TString&                strName
                                , const tCIDLib::TCard2         c2ClassId
                                , const tCIDMacroEng::EConstTypes  eConst) :

    TMEngClassVal(strName, c2ClassId, eConst)
    , m_porbcCtrl(0)
{
}

TCQCAppCtrlVal::~TCQCAppCtrlVal()
{
    try
    {
        delete m_porbcCtrl;
    }

    catch(const TError& errToCatch)
    {
        if (facCQCGenDrvS().bShouldLog(errToCatch))
            TModule::LogEventObj(errToCatch);
    }

    catch(...)
    {
    }
}


// ---------------------------------------------------------------------------
//  TCQCAppCtrlVal: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TCQCAppCtrlVal::bDbgFormat(         TTextOutStream&     strmTarget
                            , const TMEngClassInfo&
                            , const tCIDMacroEng::EDbgFmts eFormat
                            , const tCIDLib::ERadices
                            , const TCIDMacroEngine&) const
{
    if (eFormat == tCIDMacroEng::EDbgFmts::Long)
    {
        strmTarget << L"Moniker = " << m_strMoniker << kCIDLib::NewLn
                   << L"Binding = " << m_strBinding;
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


// ---------------------------------------------------------------------------
//  TCQCAppCtrlVal: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TCQCAppCtrlVal::bConnect()
{
    //
    //  If we already have a client proxy, then make sure it's still good
    //  by pinging the other side. If it fails, then we'll toast this
    //  proxy and try to create another.
    //
    if (m_porbcCtrl)
    {
        try
        {
            m_porbcCtrl->Ping();
        }

        catch(...)
        {
            delete m_porbcCtrl;
            m_porbcCtrl = 0;
        }
    }

    // If we don't have the client proxy, then let's try to create it
    if (!m_porbcCtrl)
    {
        TString strBind(TAppCtrlClientProxy::strImplBinding);
        strBind.eReplaceToken(m_strBinding, kCIDLib::chLatin_b);
        m_porbcCtrl = facCIDOrbUC().porbcMakeClient<TAppCtrlClientProxy>(strBind);

        // If not, return failure
        if (!m_porbcCtrl)
            return kCIDLib::False;
    }

    // Looks OK, so try to register the app
    m_porbcCtrl->AddRecord
    (
        m_strMoniker
        , m_strAppTitle
        , m_strAppPath
        , m_strParams
        , m_strWorkingDir
    );
    return kCIDLib::True;
}


tCIDLib::TBoolean TCQCAppCtrlVal::bDisconnect(const tCIDLib::TBoolean bCleanup)
{
    //
    //  If we have a client proxy, then try to send a remove event for
    //  our application. If it fails, nothing we can really do.
    //
    if (m_porbcCtrl && bCleanup)
    {
        try
        {
            m_porbcCtrl->RemoveRecord(m_strMoniker);
        }

        catch(const TError& errToCatch)
        {
            if (facCQCGenDrvS().bShouldLog(errToCatch))
                TModule::LogEventObj(errToCatch);
        }
    }

    delete m_porbcCtrl;
    m_porbcCtrl = 0;
    return kCIDLib::True;
}


tCIDLib::TBoolean TCQCAppCtrlVal::bIsConnected()
{
    // Tell them if we have a client proxy
    return (m_porbcCtrl != 0);
}


tCIDLib::TBoolean TCQCAppCtrlVal::bIsRegistered()
{
    CheckConnected();
    return m_porbcCtrl->bAppRegistered(m_strMoniker, m_strAppTitle);
}



tCIDLib::TBoolean TCQCAppCtrlVal::bIsRunning()
{
    CheckConnected();
    return m_porbcCtrl->bIsRunning(m_strMoniker);
}


tCIDLib::TBoolean
TCQCAppCtrlVal::bIsVisible( const   tCIDLib::TCard4 c4Id
                            , const tCIDLib::TCard4 c4ChildId)
{
    CheckConnected();
    return m_porbcCtrl->bIsVisible(m_strMoniker, c4Id, c4ChildId);
}


tCIDLib::TBoolean TCQCAppCtrlVal::bStart()
{
    // Throw if not connected. If we are, then try the operation
    CheckConnected();
    return m_porbcCtrl->bStart(m_strMoniker);
}


tCIDLib::TBoolean TCQCAppCtrlVal::bStart(const TString& strParams)
{
    // Throw if not connected. If we are, then try the operation
    CheckConnected();
    return m_porbcCtrl->bStartWithParams(m_strMoniker, strParams);
}


tCIDLib::TBoolean TCQCAppCtrlVal::bStartViaOpen(const TString& strDocFile)
{
    // Throw if not connected. If we are, then try the operation
    CheckConnected();
    return m_porbcCtrl->bStartAppViaOpen
    (
        m_strMoniker
        , strDocFile
        , TString::strEmpty()
        , L"Normal"
    );
}


tCIDLib::TBoolean TCQCAppCtrlVal::bStop()
{
    // Throw if not connected. If we are, then try the operation
    CheckConnected();
    return m_porbcCtrl->bStop(m_strMoniker);
}


tCIDLib::TCard4 TCQCAppCtrlVal::c4AddWindow(const TString& strPath)
{
    // Throw if not connected. If we are, then try the operation
    CheckConnected();
    return m_porbcCtrl->c4AddWindow(m_strMoniker, strPath);
}


tCIDLib::TCard4 TCQCAppCtrlVal::c4AddWindowByClass(const TString& strClass)
{
    // Throw if not connected. If we are, then try the operation
    CheckConnected();
    return m_porbcCtrl->c4AddWindowByClass(m_strMoniker, strClass);
}


tCIDLib::TCard4
TCQCAppCtrlVal::c4QueryListSel( const   tCIDLib::TCard4 c4Id
                                , const tCIDLib::TCard4 c4ChildId)
{
    // Throw if not connected. If we are, then try the operation
    CheckConnected();
    return m_porbcCtrl->c4QueryListSel(m_strMoniker, c4Id, c4ChildId);
}


tCIDLib::TCard4
TCQCAppCtrlVal::c4QueryText(const   tCIDLib::TCard4 c4Id
                            , const tCIDLib::TCard4 c4ChildId
                            ,       TString&        strToFill)
{
    // Throw if not connected. If we are, then try the operation
    CheckConnected();
    return m_porbcCtrl->c4QueryText(m_strMoniker, c4Id, c4ChildId, strToFill);
}


tCIDLib::TCard4
TCQCAppCtrlVal::c4QueryWndStyles(const  tCIDLib::TCard4 c4Id
                                , const tCIDLib::TCard4 c4ChildId)
{
    // Throw if not connected. If we are, then try the operation
    CheckConnected();
    return m_porbcCtrl->c4QueryWndStyles(m_strMoniker, c4Id, c4ChildId);
}


tCIDLib::TInt4
TCQCAppCtrlVal::i4QueryTrackBar(const   tCIDLib::TCard4 c4Id
                                , const tCIDLib::TCard4 c4ChildId)
{
    // Throw if not connected. If we are, then try the operation
    CheckConnected();
    return m_porbcCtrl->i4QueryTrackBar(m_strMoniker, c4Id, c4ChildId);
}


tCIDLib::TInt4
TCQCAppCtrlVal::i4SendMsg(  const   tCIDLib::TCard4     c4Id
                            , const tCIDLib::TCard4     c4ChildId
                            , const tCIDLib::TCard4     c4ToSend
                            , const tCIDLib::TCard4     c4Param1
                            , const tCIDLib::TInt4      i4Param2
                            , const tCIDLib::TBoolean   bAsync)
{
    // Throw if not connected. If we are, then try the operation
    CheckConnected();
    return m_porbcCtrl->i4SendMsg
    (
        m_strMoniker
        , c4Id
        , c4ChildId
        , c4ToSend
        , c4Param1
        , i4Param2
        , bAsync
    );
}


tCIDLib::TVoid
TCQCAppCtrlVal::Initialize( const   TString&    strMoniker
                            , const TString&    strBinding
                            , const TString&    strAppTitle
                            , const TString&    strAppName
                            , const TString&    strAppPath
                            , const TString&    strParams
                            , const TString&    strWorkingDir)
{
    m_strAppTitle    = strAppTitle;
    m_strBinding     = strBinding;
    m_strMoniker     = strMoniker;
    m_strParams      = strParams;
    m_strWorkingDir  = strWorkingDir;

    // Build up the full path
    TPathStr pathTmp(strAppPath);
    pathTmp.AddLevel(strAppName);
    m_strAppPath = pathTmp;
}


TAppCtrlClientProxy* TCQCAppCtrlVal::porbcCtrl()
{
    return m_porbcCtrl;
}


tCIDLib::TVoid TCQCAppCtrlVal::Reset()
{
    m_strAppPath.Clear();
    m_strAppTitle.Clear();
    m_strBinding.Clear();
    m_strMoniker.Clear();
    m_strParams.Clear();
    m_strWorkingDir.Clear();
}


tCIDLib::TVoid
TCQCAppCtrlVal::SendExtKey( const   tCIDLib::TCard4     c4Id
                            , const tCIDLib::TCard4     c4ChildId
                            , const tCIDCtrls::EExtKeys   eKeyToSend
                            , const tCIDLib::TBoolean   bAltShifted
                            , const tCIDLib::TBoolean   bCtrlShifted
                            , const tCIDLib::TBoolean   bShifted)
{
    // Throw if not connected. If we are, then try the operation
    CheckConnected();
    m_porbcCtrl->SendExtKey
    (
        m_strMoniker
        , c4Id
        , c4ChildId
        , eKeyToSend
        , bAltShifted
        , bCtrlShifted
        , bShifted
    );
}


tCIDLib::TVoid
TCQCAppCtrlVal::SendKey(const   tCIDLib::TCard4     c4Id
                        , const tCIDLib::TCard4     c4ChildId
                        , const tCIDLib::TCh        chToSend
                        , const tCIDLib::TBoolean   bAltShifted
                        , const tCIDLib::TBoolean   bCtrlShifted
                        , const tCIDLib::TBoolean   bShifted)
{
    // Throw if not connected. If we are, then try the operation
    CheckConnected();
    m_porbcCtrl->SendKey
    (
        m_strMoniker
        , c4Id
        , c4ChildId
        , chToSend
        , bAltShifted
        , bCtrlShifted
        , bShifted
    );
}


tCIDLib::TVoid
TCQCAppCtrlVal::SetFocus(const  tCIDLib::TCard4 c4Id
                        , const tCIDLib::TCard4 c4ChildId)
{
    // Throw if not connected. If we are, then try the operation
    CheckConnected();
    m_porbcCtrl->SetFocus(m_strMoniker, c4Id, c4ChildId);
}


tCIDLib::TVoid
TCQCAppCtrlVal::SetListSel( const   tCIDLib::TCard4 c4Id
                            , const tCIDLib::TCard4 c4ChildId
                            , const tCIDLib::TCard4 c4Index)
{
    // Throw if not connected. If we are, then try the operation
    CheckConnected();
    m_porbcCtrl->SetListSel(m_strMoniker, c4Id, c4ChildId, c4Index);
}

tCIDLib::TVoid
TCQCAppCtrlVal::SetTrackBar(const   tCIDLib::TCard4 c4Id
                            , const tCIDLib::TCard4 c4ChildId
                            , const tCIDLib::TInt4  i4ToSet)
{
    // Throw if not connected. If we are, then try the operation
    CheckConnected();
    m_porbcCtrl->SetTrackBar(m_strMoniker, c4Id, c4ChildId, i4ToSet);
}


tCIDLib::TVoid
TCQCAppCtrlVal::SetWindowPos(const  tCIDLib::TCard4 c4Id
                            , const tCIDLib::TCard4 c4ChildId
                            , const tCIDLib::TInt4  i4X
                            , const tCIDLib::TInt4  i4Y)
{
    // Throw if not connected. If we are, then try the operation
    CheckConnected();
    m_porbcCtrl->SetWindowPos(m_strMoniker, c4Id, c4ChildId, i4X, i4Y);
}


tCIDLib::TVoid
TCQCAppCtrlVal::SetWindowSize(  const   tCIDLib::TCard4 c4Id
                                , const tCIDLib::TCard4 c4ChildId
                                , const tCIDLib::TCard4 c4Width
                                , const tCIDLib::TCard4 c4Height)
{
    // Throw if not connected. If we are, then try the operation
    CheckConnected();
    m_porbcCtrl->SetWindowSize(m_strMoniker, c4Id, c4ChildId, c4Width, c4Height);
}


tCIDLib::TVoid
TCQCAppCtrlVal::SetWindowText(  const   tCIDLib::TCard4 c4Id
                                , const tCIDLib::TCard4 c4ChildId
                                , const TString&        strText)
{
    // Throw if not connected. If we are, then try the operation
    CheckConnected();
    m_porbcCtrl->SetWindowText(m_strMoniker, c4Id, c4ChildId, strText);
}


tCIDLib::TVoid
TCQCAppCtrlVal::ShowWindow( const   tCIDLib::TCard4     c4Id
                            , const tCIDLib::TCard4     c4ChildId
                            , const tCIDLib::TBoolean   bState)
{
    // Throw if not connected. If we are, then try the operation
    CheckConnected();
    m_porbcCtrl->ShowWindow(m_strMoniker, c4Id, c4ChildId, bState);
}


tCIDLib::TVoid
TCQCAppCtrlVal::StandardOp( const   tCIDLib::TCard4     c4Id
                            , const tCIDLib::TCard4     c4ChildId
                            , const tCQCKit::EStdACOps eOp)
{
    // Throw if not connected. If we are, then try the operation
    CheckConnected();
    m_porbcCtrl->StandardOp(m_strMoniker, c4Id, c4ChildId, eOp);
}


// ---------------------------------------------------------------------------
//  TCQCAppCtrlVal: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCAppCtrlVal::CheckConnected()
{
    if (!m_porbcCtrl)
    {
        facCQCGenDrvS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kGDrvSErrs::errcAppCtrl_NotConnected
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::LostConnection
        );
    }
}



// ---------------------------------------------------------------------------
//  CLASS: TCQCAppCtrlInfo
// PREFIX: meci
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCAppCtrlInfo: Public, static methods
// ---------------------------------------------------------------------------
const TString& TCQCAppCtrlInfo::strPath()
{
    return CQCGenDrvS_AppCtrl::strClassPath;
}


// ---------------------------------------------------------------------------
//  TCQCAppCtrlInfo: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCAppCtrlInfo::TCQCAppCtrlInfo(TCIDMacroEngine& meOwner) :

    TMEngClassInfo
    (
        CQCGenDrvS_AppCtrl::strClassName
        , CQCGenDrvS_AppCtrl::strBasePath
        , meOwner
        , kCIDLib::False
        , tCIDMacroEng::EClassExt::Final
        , L"MEng.Object"
    )
    , m_c2EnumId_ButtStates(kCIDMacroEng::c2BadId)
    , m_c2EnumId_Errors(kCIDMacroEng::c2BadId)
    , m_c2EnumId_ExtKeys(kCIDMacroEng::c2BadId)
    , m_c2EnumId_StdOps(kCIDMacroEng::c2BadId)
    , m_c2MethId_AddWindow(kCIDMacroEng::c2BadId)
    , m_c2MethId_AddWindowByClass(kCIDMacroEng::c2BadId)
    , m_c2MethId_ClickButton(kCIDMacroEng::c2BadId)
    , m_c2MethId_Connect(kCIDMacroEng::c2BadId)
    , m_c2MethId_DefCtor(kCIDMacroEng::c2BadId)
    , m_c2MethId_Disconnect(kCIDMacroEng::c2BadId)
    , m_c2MethId_IsRegistered(kCIDMacroEng::c2BadId)
    , m_c2MethId_IsRunning(kCIDMacroEng::c2BadId)
    , m_c2MethId_IsVisible(kCIDMacroEng::c2BadId)
    , m_c2MethId_QueryButtState(kCIDMacroEng::c2BadId)
    , m_c2MethId_QueryListSel(kCIDMacroEng::c2BadId)
    , m_c2MethId_QueryText(kCIDMacroEng::c2BadId)
    , m_c2MethId_QueryTrackBar(kCIDMacroEng::c2BadId)
    , m_c2MethId_QueryWndStyles(kCIDMacroEng::c2BadId)
    , m_c2MethId_SendExtKey(kCIDMacroEng::c2BadId)
    , m_c2MethId_SendKey(kCIDMacroEng::c2BadId)
    , m_c2MethId_SendMenuItem(kCIDMacroEng::c2BadId)
    , m_c2MethId_SendMsg(kCIDMacroEng::c2BadId)
    , m_c2MethId_SetButtState(kCIDMacroEng::c2BadId)
    , m_c2MethId_SetFocus(kCIDMacroEng::c2BadId)
    , m_c2MethId_SetInfo(kCIDMacroEng::c2BadId)
    , m_c2MethId_SetListSel(kCIDMacroEng::c2BadId)
    , m_c2MethId_SetTrackBar(kCIDMacroEng::c2BadId)
    , m_c2MethId_SetWindowPos(kCIDMacroEng::c2BadId)
    , m_c2MethId_SetWindowSize(kCIDMacroEng::c2BadId)
    , m_c2MethId_SetWindowText(kCIDMacroEng::c2BadId)
    , m_c2MethId_ShowWindow(kCIDMacroEng::c2BadId)
    , m_c2MethId_StandardOp(kCIDMacroEng::c2BadId)
    , m_c2MethId_StartApp(kCIDMacroEng::c2BadId)
    , m_c2MethId_StartAppWithParams(kCIDMacroEng::c2BadId)
    , m_c2MethId_StartViaOpen(kCIDMacroEng::c2BadId)
    , m_c2MethId_StopApp(kCIDMacroEng::c2BadId)
    , m_c4ErrId_AddWindow(kCIDLib::c4MaxCard)
    , m_c4ErrId_Connect(kCIDLib::c4MaxCard)
    , m_c4ErrId_Disconnect(kCIDLib::c4MaxCard)
    , m_c4ErrId_NotConnected(kCIDLib::c4MaxCard)
    , m_c4ErrId_QueryText(kCIDLib::c4MaxCard)
    , m_c4ErrId_QueryVisibility(kCIDLib::c4MaxCard)
    , m_c4ErrId_Register(kCIDLib::c4MaxCard)
    , m_c4ErrId_SendKey(kCIDLib::c4MaxCard)
    , m_c4ErrId_SendMsg(kCIDLib::c4MaxCard)
    , m_c4ErrId_SetFocus(kCIDLib::c4MaxCard)
    , m_c4ErrId_SetSizePos(kCIDLib::c4MaxCard)
    , m_c4ErrId_SetText(kCIDLib::c4MaxCard)
    , m_c4ErrId_SetVisibility(kCIDLib::c4MaxCard)
    , m_c4ErrId_ShowWindow(kCIDLib::c4MaxCard)
    , m_c4ErrId_Start(kCIDLib::c4MaxCard)
    , m_c4ErrId_StdOp(kCIDLib::c4MaxCard)
    , m_c4ErrId_Stop(kCIDLib::c4MaxCard)
    , m_c4ErrId_Unknown(kCIDLib::c4MaxCard)
    , m_pmeciButtStates(0)
    , m_pmeciErrors(0)
    , m_pmeciExtKeys(0)
    , m_pmeciStdOps(0)
{
}

TCQCAppCtrlInfo::~TCQCAppCtrlInfo()
{
}


// ---------------------------------------------------------------------------
//  TCQCAppCtrlInfo: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCAppCtrlInfo::Init(TCIDMacroEngine& meOwner)
{
    // Add a few literals that will be of help to users of this class
    AddLiteral
    (
        new TMEngLiteralVal
        (
            L"kMsg_UserBase"
            , tCIDMacroEng::EIntrinsics::Card4
            , new TMEngCard4Val(L"UserBase", tCIDMacroEng::EConstTypes::Const, 0x400)
        )
    );

    AddLiteral
    (
        new TMEngLiteralVal
        (
            L"kMsg_Command"
            , tCIDMacroEng::EIntrinsics::Card4
            , new TMEngCard4Val(L"Command", tCIDMacroEng::EConstTypes::Const, 0x111)
        )
    );

    AddLiteral
    (
        new TMEngLiteralVal
        (
            L"kMsg_SysCommand"
            , tCIDMacroEng::EIntrinsics::Card4
            , new TMEngCard4Val(L"SysCommand", tCIDMacroEng::EConstTypes::Const, 0x112)
        )
    );

    AddLiteral
    (
        new TMEngLiteralVal
        (
            L"kWndId_Top"
            , tCIDMacroEng::EIntrinsics::Card4
            , new TMEngCard4Val(L"TopWindowId", tCIDMacroEng::EConstTypes::Const, 0)
        )
    );

    AddLiteral
    (
        new TMEngLiteralVal
        (
            L"kNoChild"
            , tCIDMacroEng::EIntrinsics::Card4
            , new TMEngCard4Val(L"kNoChild", tCIDMacroEng::EConstTypes::Const, kCIDLib::c4MaxCard)
        )
    );

    AddLiteral
    (
        new TMEngLiteralVal
        (
            L"kWndStyle_PopUp"
            , tCIDMacroEng::EIntrinsics::Card4
            , new TMEngCard4Val(L"PopUp", tCIDMacroEng::EConstTypes::Const, 0x80000000)
        )
    );

    AddLiteral
    (
        new TMEngLiteralVal
        (
            L"kWndStyle_Child"
            , tCIDMacroEng::EIntrinsics::Card4
            , new TMEngCard4Val(L"Child", tCIDMacroEng::EConstTypes::Const, 0x40000000)
        )
    );

    AddLiteral
    (
        new TMEngLiteralVal
        (
            L"kWndStyle_Visible"
            , tCIDMacroEng::EIntrinsics::Card4
            , new TMEngCard4Val(L"Visible", tCIDMacroEng::EConstTypes::Const, 0x10000000)
        )
    );

    AddLiteral
    (
        new TMEngLiteralVal
        (
            L"kWndStyle_Disabled"
            , tCIDMacroEng::EIntrinsics::Card4
            , new TMEngCard4Val(L"Disabled", tCIDMacroEng::EConstTypes::Const, 0x08000000)
        )
    );

    AddLiteral
    (
        new TMEngLiteralVal
        (
            L"kWndStyle_ClipSiblings"
            , tCIDMacroEng::EIntrinsics::Card4
            , new TMEngCard4Val(L"ClipSiblings", tCIDMacroEng::EConstTypes::Const, 0x04000000)
        )
    );

    AddLiteral
    (
        new TMEngLiteralVal
        (
            L"kWndStyle_ClipChildren"
            , tCIDMacroEng::EIntrinsics::Card4
            , new TMEngCard4Val(L"ClipChildren", tCIDMacroEng::EConstTypes::Const, 0x02000000)
        )
    );

    AddLiteral
    (
        new TMEngLiteralVal
        (
            L"kWndStyle_Border"
            , tCIDMacroEng::EIntrinsics::Card4
            , new TMEngCard4Val(L"Border", tCIDMacroEng::EConstTypes::Const, 0x00800000)
        )
    );

    AddLiteral
    (
        new TMEngLiteralVal
        (
            L"kWndStyle_DlgFrame"
            , tCIDMacroEng::EIntrinsics::Card4
            , new TMEngCard4Val(L"DlgFrame", tCIDMacroEng::EConstTypes::Const, 0x00400000)
        )
    );

    AddLiteral
    (
        new TMEngLiteralVal
        (
            L"kWndStyle_VScroll"
            , tCIDMacroEng::EIntrinsics::Card4
            , new TMEngCard4Val(L"VScroll", tCIDMacroEng::EConstTypes::Const, 0x00200000)
        )
    );

    AddLiteral
    (
        new TMEngLiteralVal
        (
            L"kWndStyle_HScroll"
            , tCIDMacroEng::EIntrinsics::Card4
            , new TMEngCard4Val(L"HScroll", tCIDMacroEng::EConstTypes::Const, 0x00100000)
        )
    );

    AddLiteral
    (
        new TMEngLiteralVal
        (
            L"kWndStyle_SysMenu"
            , tCIDMacroEng::EIntrinsics::Card4
            , new TMEngCard4Val(L"SysMenu", tCIDMacroEng::EConstTypes::Const, 0x00080000)
        )
    );

    AddLiteral
    (
        new TMEngLiteralVal
        (
            L"kWndStyle_ThickFrame"
            , tCIDMacroEng::EIntrinsics::Card4
            , new TMEngCard4Val(L"ThickFrame", tCIDMacroEng::EConstTypes::Const, 0x00040000)
        )
    );

    AddLiteral
    (
        new TMEngLiteralVal
        (
            L"kWndStyle_Group"
            , tCIDMacroEng::EIntrinsics::Card4
            , new TMEngCard4Val(L"Group", tCIDMacroEng::EConstTypes::Const, 0x00020000)
        )
    );

    AddLiteral
    (
        new TMEngLiteralVal
        (
            L"kWndStyle_TabStop"
            , tCIDMacroEng::EIntrinsics::Card4
            , new TMEngCard4Val(L"TabStop", tCIDMacroEng::EConstTypes::Const, 0x00010000)
        )
    );

    AddLiteral
    (
        new TMEngLiteralVal
        (
            L"kSysCmd_Minimize"
            , tCIDMacroEng::EIntrinsics::Card4
            , new TMEngCard4Val(L"Minimize", tCIDMacroEng::EConstTypes::Const, 0xF020)
        )
    );

    AddLiteral
    (
        new TMEngLiteralVal
        (
            L"kSysCmd_Maximize"
            , tCIDMacroEng::EIntrinsics::Card4
            , new TMEngCard4Val(L"Maximize", tCIDMacroEng::EConstTypes::Const, 0xF030)
        )
    );

    AddLiteral
    (
        new TMEngLiteralVal
        (
            L"kSysCmd_Close"
            , tCIDMacroEng::EIntrinsics::Card4
            , new TMEngCard4Val(L"Close", tCIDMacroEng::EConstTypes::Const, 0xF060)
        )
    );

    AddLiteral
    (
        new TMEngLiteralVal
        (
            L"kSysCmd_Restore"
            , tCIDMacroEng::EIntrinsics::Card4
            , new TMEngCard4Val(L"Restore", tCIDMacroEng::EConstTypes::Const, 0xF120)
        )
    );


    // Create an error enum for this class
    {
        m_pmeciErrors = new TMEngEnumInfo
        (
            meOwner
            , L"CQCAppCtrlErrs"
            , strClassPath()
            , L"MEng.Enum"
            , 20
        );
        m_c4ErrId_AddWindow = m_pmeciErrors->c4AddEnumItem(L"AddWindow", TString::strEmpty());
        m_c4ErrId_Connect = m_pmeciErrors->c4AddEnumItem(L"ConnectErr", TString::strEmpty());
        m_c4ErrId_Disconnect = m_pmeciErrors->c4AddEnumItem(L"DisconnectErr", TString::strEmpty());
        m_c4ErrId_NotConnected = m_pmeciErrors->c4AddEnumItem(L"NotConnected", L"Not connected to the app control server");
        m_c4ErrId_QueryText = m_pmeciErrors->c4AddEnumItem(L"QueryText", TString::strEmpty());
        m_c4ErrId_QueryVisibility = m_pmeciErrors->c4AddEnumItem(L"QueryVisibility", TString::strEmpty());
        m_c4ErrId_QueryWndStyles = m_pmeciErrors->c4AddEnumItem(L"QueryWndStyles", TString::strEmpty());
        m_c4ErrId_Register = m_pmeciErrors->c4AddEnumItem(L"RegisterFailed", TString::strEmpty());
        m_c4ErrId_SendKey = m_pmeciErrors->c4AddEnumItem(L"SendKey", TString::strEmpty());
        m_c4ErrId_SendMsg = m_pmeciErrors->c4AddEnumItem(L"SendMsg", TString::strEmpty());
        m_c4ErrId_SetFocus = m_pmeciErrors->c4AddEnumItem(L"SetFocus", TString::strEmpty());
        m_c4ErrId_SetSizePos = m_pmeciErrors->c4AddEnumItem(L"SetSizePos", TString::strEmpty());
        m_c4ErrId_SetText = m_pmeciErrors->c4AddEnumItem(L"SetText", TString::strEmpty());
        m_c4ErrId_SetVisibility = m_pmeciErrors->c4AddEnumItem(L"SetVisibility", TString::strEmpty());
        m_c4ErrId_ShowWindow = m_pmeciErrors->c4AddEnumItem(L"ShowWindow", TString::strEmpty());
        m_c4ErrId_Start = m_pmeciErrors->c4AddEnumItem(L"StartFailed", TString::strEmpty());
        m_c4ErrId_StdOp = m_pmeciErrors->c4AddEnumItem(L"StdOpFailed", TString::strEmpty());
        m_c4ErrId_Stop = m_pmeciErrors->c4AddEnumItem(L"StopFailed", TString::strEmpty());
        m_c4ErrId_Unknown = m_pmeciErrors->c4AddEnumItem(L"Unknown", TString::strEmpty());
        m_pmeciErrors->BaseClassInit(meOwner);
        m_c2EnumId_Errors = meOwner.c2AddClass(m_pmeciErrors);
        bAddNestedType(m_pmeciErrors->strClassPath());
    }

    // Create an enum for button states
    {
        m_pmeciButtStates = new TMEngEnumInfo
        (
            meOwner
            , L"ButtonStates"
            , strClassPath()
            , L"MEng.Enum"
            , 3
        );
        m_pmeciButtStates->c4AddEnumItem(L"Off", L"Not checked");
        m_pmeciButtStates->c4AddEnumItem(L"On", L"Checked");
        m_pmeciButtStates->c4AddEnumItem(L"Maybe", L"Indeterminate");
        m_pmeciButtStates->BaseClassInit(meOwner);
        m_c2EnumId_ButtStates = meOwner.c2AddClass(m_pmeciButtStates);
        bAddNestedType(m_pmeciButtStates->strClassPath());
    }

    // An enum for the standard window operations
    {
        m_pmeciStdOps = new TMEngEnumInfo
        (
            meOwner
            , L"AppCtrlOps"
            , strClassPath()
            , L"MEng.Enum"
            , 5
        );
        m_pmeciStdOps->c4AddEnumItem(L"Maximize", L"Maximize Window", tCQCKit::EStdACOps::Maximize);
        m_pmeciStdOps->c4AddEnumItem(L"Minimize", L"Minimize Window", tCQCKit::EStdACOps::Minimize);
        m_pmeciStdOps->c4AddEnumItem(L"Restore", L"Restore Window", tCQCKit::EStdACOps::Restore);
        m_pmeciStdOps->c4AddEnumItem(L"ToBottom", L"To Bottom", tCQCKit::EStdACOps::ToBack);
        m_pmeciStdOps->c4AddEnumItem(L"ToTop", L"To Top", tCQCKit::EStdACOps::ToTop);
        m_pmeciStdOps->BaseClassInit(meOwner);
        m_c2EnumId_StdOps = meOwner.c2AddClass(m_pmeciStdOps);
        bAddNestedType(m_pmeciStdOps->strClassPath());
    }

    //
    //  An enum for the extended keys that can be sent. This is mapped to
    //  the tCIDCtrls::EExtKeys enum that is used to represent these keys
    //  in CIDLib.
    //
    {
        m_pmeciExtKeys = new TMEngEnumInfo
        (
            meOwner
            , L"AppCtrlExtKeys"
            , strClassPath()
            , L"MEng.Enum"
            , tCIDLib::c4EnumOrd(tCIDCtrls::EExtKeys::Count)
        );
        m_pmeciExtKeys->c4AddEnumItem(L"BackSpace", L"BackSpace", tCIDCtrls::EExtKeys::BackSpace);
        m_pmeciExtKeys->c4AddEnumItem(L"Tab", L"Tab", tCIDCtrls::EExtKeys::Tab);
        m_pmeciExtKeys->c4AddEnumItem(L"BackTab", L"BackTab", tCIDCtrls::EExtKeys::BackTab);
        m_pmeciExtKeys->c4AddEnumItem(L"Esc", L"Esc", tCIDCtrls::EExtKeys::Esc);
        m_pmeciExtKeys->c4AddEnumItem(L"PageUp", L"PageUp", tCIDCtrls::EExtKeys::PageUp);
        m_pmeciExtKeys->c4AddEnumItem(L"PageDown", L"PageDown", tCIDCtrls::EExtKeys::PageDown);
        m_pmeciExtKeys->c4AddEnumItem(L"End", L"End", tCIDCtrls::EExtKeys::End);
        m_pmeciExtKeys->c4AddEnumItem(L"Home", L"Home", tCIDCtrls::EExtKeys::Home);
        m_pmeciExtKeys->c4AddEnumItem(L"Left", L"Left", tCIDCtrls::EExtKeys::Left);
        m_pmeciExtKeys->c4AddEnumItem(L"Up", L"Up", tCIDCtrls::EExtKeys::Up);
        m_pmeciExtKeys->c4AddEnumItem(L"Right", L"Right", tCIDCtrls::EExtKeys::Right);
        m_pmeciExtKeys->c4AddEnumItem(L"Down", L"Down", tCIDCtrls::EExtKeys::Down);
        m_pmeciExtKeys->c4AddEnumItem(L"Insert", L"Insert", tCIDCtrls::EExtKeys::Insert);
        m_pmeciExtKeys->c4AddEnumItem(L"Delete", L"Delete", tCIDCtrls::EExtKeys::Delete);
        m_pmeciExtKeys->c4AddEnumItem(L"Enter", L"Enter", tCIDCtrls::EExtKeys::Enter);
        m_pmeciExtKeys->c4AddEnumItem(L"F1", L"F1", tCIDCtrls::EExtKeys::F1);
        m_pmeciExtKeys->c4AddEnumItem(L"F2", L"F2", tCIDCtrls::EExtKeys::F2);
        m_pmeciExtKeys->c4AddEnumItem(L"F3", L"F3", tCIDCtrls::EExtKeys::F3);
        m_pmeciExtKeys->c4AddEnumItem(L"F4", L"F4", tCIDCtrls::EExtKeys::F4);
        m_pmeciExtKeys->c4AddEnumItem(L"F5", L"F5", tCIDCtrls::EExtKeys::F5);
        m_pmeciExtKeys->c4AddEnumItem(L"F6", L"F6", tCIDCtrls::EExtKeys::F6);
        m_pmeciExtKeys->c4AddEnumItem(L"F7", L"F7", tCIDCtrls::EExtKeys::F7);
        m_pmeciExtKeys->c4AddEnumItem(L"F8", L"F8", tCIDCtrls::EExtKeys::F8);
        m_pmeciExtKeys->c4AddEnumItem(L"F9", L"F9", tCIDCtrls::EExtKeys::F9);
        m_pmeciExtKeys->c4AddEnumItem(L"F10", L"F10", tCIDCtrls::EExtKeys::F10);
        m_pmeciExtKeys->c4AddEnumItem(L"F11", L"F11", tCIDCtrls::EExtKeys::F11);
        m_pmeciExtKeys->c4AddEnumItem(L"F12", L"F12", tCIDCtrls::EExtKeys::F12);
        m_pmeciExtKeys->c4AddEnumItem(L"Browser_Back", L"Browser Back", tCIDCtrls::EExtKeys::Browser_Back);
        m_pmeciExtKeys->c4AddEnumItem(L"Browser_Forward", L"Browser Forward", tCIDCtrls::EExtKeys::Browser_Forward);
        m_pmeciExtKeys->c4AddEnumItem(L"Browser_Stop", L"Browser Stop", tCIDCtrls::EExtKeys::Browser_Stop);
        m_pmeciExtKeys->c4AddEnumItem(L"Browser_Refresh", L"Browser Refresh", tCIDCtrls::EExtKeys::Browser_Refresh);
        m_pmeciExtKeys->c4AddEnumItem(L"Browser_Search", L"Browser Search", tCIDCtrls::EExtKeys::Browser_Search);
        m_pmeciExtKeys->c4AddEnumItem(L"Browser_Favorites", L"Browser Favorites", tCIDCtrls::EExtKeys::Browser_Favorites);
        m_pmeciExtKeys->c4AddEnumItem(L"Browser_Home", L"Browser Home", tCIDCtrls::EExtKeys::Browser_Home);
        m_pmeciExtKeys->c4AddEnumItem(L"Volume_Mute", L"Volume Mute", tCIDCtrls::EExtKeys::Volume_Mute);
        m_pmeciExtKeys->c4AddEnumItem(L"Volume_Down", L"Volume Down", tCIDCtrls::EExtKeys::Volume_Down);
        m_pmeciExtKeys->c4AddEnumItem(L"Volume_Up", L"Volume Up", tCIDCtrls::EExtKeys::Volume_Up);
        m_pmeciExtKeys->c4AddEnumItem(L"Media_Pause", L"Media Pause", tCIDCtrls::EExtKeys::Pause);
        m_pmeciExtKeys->c4AddEnumItem(L"Media_PlayPause", L"Media PlayPause", tCIDCtrls::EExtKeys::Media_PlayPause);
        m_pmeciExtKeys->c4AddEnumItem(L"Media_Stop", L"Media Stop", tCIDCtrls::EExtKeys::Media_Stop);
        m_pmeciExtKeys->c4AddEnumItem(L"Media_PrevTrack", L"Media PrevTrack", tCIDCtrls::EExtKeys::Media_PrevTrack);
        m_pmeciExtKeys->c4AddEnumItem(L"Media_NextTrack", L"Media NextTrack", tCIDCtrls::EExtKeys::Media_NextTrack);
        m_pmeciExtKeys->BaseClassInit(meOwner);
        m_c2EnumId_ExtKeys = meOwner.c2AddClass(m_pmeciExtKeys);
        bAddNestedType(m_pmeciExtKeys->strClassPath());
    }


    // Add a new window to the list of target windows, by path
    {
        TMEngMethodInfo methiNew
        (
            L"AddWindow"
            , tCIDMacroEng::EIntrinsics::Card4
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"PathStr", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_AddWindow = c2AddMethodInfo(methiNew);
    }

    // Add a new top level window to the list of target windows, by class
    {
        TMEngMethodInfo methiNew
        (
            L"AddWindowByClass"
            , tCIDMacroEng::EIntrinsics::Card4
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"ClassName", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_AddWindowByClass = c2AddMethodInfo(methiNew);
    }

    // The connect method
    {
        TMEngMethodInfo methiNew
        (
            L"ClickButton"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"WndId", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddInParm(L"ChildId", tCIDMacroEng::EIntrinsics::Card4);
        m_c2MethId_ClickButton = c2AddMethodInfo(methiNew);
    }

    // The connect method
    {
        TMEngMethodInfo methiNew
        (
            L"Connect"
            , tCIDMacroEng::EIntrinsics::Boolean
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        m_c2MethId_Connect = c2AddMethodInfo(methiNew);
    }

    // Add the default constructor
    {
        TMEngMethodInfo methiNew
        (
            L"ctor1_MEng.System.CQC.Runtime.AppCtrlClient"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.bIsCtor(kCIDLib::True);
        m_c2MethId_DefCtor = c2AddMethodInfo(methiNew);
    }

    // The disconnect method
    {
        TMEngMethodInfo methiNew
        (
            L"Disconnect"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        m_c2MethId_Disconnect = c2AddMethodInfo(methiNew);
    }

    // Check if the app is registered
    {
        TMEngMethodInfo methiNew
        (
            L"IsRegistered"
            , tCIDMacroEng::EIntrinsics::Boolean
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        m_c2MethId_IsRegistered = c2AddMethodInfo(methiNew);
    }

    // Check if the app is running
    {
        TMEngMethodInfo methiNew
        (
            L"IsRunning"
            , tCIDMacroEng::EIntrinsics::Boolean
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        m_c2MethId_IsRunning = c2AddMethodInfo(methiNew);
    }

    // Check if a window is visible
    {
        TMEngMethodInfo methiNew
        (
            L"IsVisible"
            , tCIDMacroEng::EIntrinsics::Boolean
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"WndId", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddInParm(L"ChildId", tCIDMacroEng::EIntrinsics::Card4);
        m_c2MethId_IsVisible = c2AddMethodInfo(methiNew);
    }

    // Query a button state
    {
        TMEngMethodInfo methiNew
        (
            L"QueryButtonState"
            , m_c2EnumId_ButtStates
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"WndId", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddInParm(L"ChildId", tCIDMacroEng::EIntrinsics::Card4);
        m_c2MethId_QueryButtState = c2AddMethodInfo(methiNew);
    }

    // Query the selection of a single selection list box
    {
        TMEngMethodInfo methiNew
        (
            L"QueryListSel"
            , tCIDMacroEng::EIntrinsics::Card4
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"WndId", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddInParm(L"ChildId", tCIDMacroEng::EIntrinsics::Card4);
        m_c2MethId_QueryListSel = c2AddMethodInfo(methiNew);
    }

    // Query the text of a child of a window
    {
        TMEngMethodInfo methiNew
        (
            L"QueryText"
            , tCIDMacroEng::EIntrinsics::Card4
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"WndId", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddInParm(L"ChildId", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddOutParm(L"ToFill", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_QueryText = c2AddMethodInfo(methiNew);
    }

    // Query the value of a track bar
    {
        TMEngMethodInfo methiNew
        (
            L"QueryTrackBar"
            , tCIDMacroEng::EIntrinsics::Int4
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"WndId", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddInParm(L"ChildId", tCIDMacroEng::EIntrinsics::Card4);
        m_c2MethId_QueryTrackBar = c2AddMethodInfo(methiNew);
    }

    // Query the window's base styles
    {
        TMEngMethodInfo methiNew
        (
            L"QueryWndStyles"
            , tCIDMacroEng::EIntrinsics::Card4
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"WndId", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddInParm(L"ChildId", tCIDMacroEng::EIntrinsics::Card4);
        m_c2MethId_QueryWndStyles = c2AddMethodInfo(methiNew);
    }

    // Send a key or extended key
    {
        TMEngMethodInfo methiNew
        (
            L"SendExtKey"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"WndId", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddInParm(L"ChildId", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddInParm(L"KeyToSend", m_c2EnumId_ExtKeys);
        methiNew.c2AddInParm(L"AltShifted", tCIDMacroEng::EIntrinsics::Boolean);
        methiNew.c2AddInParm(L"CtrlShifted", tCIDMacroEng::EIntrinsics::Boolean);
        methiNew.c2AddInParm(L"Shifted", tCIDMacroEng::EIntrinsics::Boolean);
        m_c2MethId_SendExtKey = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"SendKey"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"WndId", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddInParm(L"ChildId", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddInParm(L"KeyToSend", tCIDMacroEng::EIntrinsics::Char);
        methiNew.c2AddInParm(L"AltShifted", tCIDMacroEng::EIntrinsics::Boolean);
        methiNew.c2AddInParm(L"CtrlShifted", tCIDMacroEng::EIntrinsics::Boolean);
        methiNew.c2AddInParm(L"Shifted", tCIDMacroEng::EIntrinsics::Boolean);
        m_c2MethId_SendKey = c2AddMethodInfo(methiNew);
    }

    // Send a menu item
    {
        TMEngMethodInfo methiNew
        (
            L"SendMenuItem"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"WndId", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddInParm(L"ChildId", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddInParm(L"CmdId", tCIDMacroEng::EIntrinsics::Card2);
        m_c2MethId_SendMenuItem = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"SendMsg"
            , tCIDMacroEng::EIntrinsics::Int4
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"WndId", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddInParm(L"ChildId", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddInParm(L"MsgToSend", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddInParm(L"Param1", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddInParm(L"Param2", tCIDMacroEng::EIntrinsics::Int4);
        methiNew.c2AddInParm(L"Async", tCIDMacroEng::EIntrinsics::Boolean);
        m_c2MethId_SendMsg = c2AddMethodInfo(methiNew);
    }

    // Set a button state
    {
        TMEngMethodInfo methiNew
        (
            L"SetButtonState"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"WndId", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddInParm(L"ChildId", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddInParm(L"NewState", m_c2EnumId_ButtStates);
        m_c2MethId_SetButtState = c2AddMethodInfo(methiNew);
    }

    // Select an item in a single selection list box
    {
        TMEngMethodInfo methiNew
        (
            L"SetFocus"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"WndId", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddInParm(L"ChildId", tCIDMacroEng::EIntrinsics::Card4);
        m_c2MethId_SetFocus = c2AddMethodInfo(methiNew);
    }

    // Set the moniker and binding after default construction
    {
        TMEngMethodInfo methiNew
        (
            L"SetInfo"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"Moniker", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"Binding", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"AppTitle", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"AppName", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"AppPath", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"Params", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"WorkingDir", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_SetInfo = c2AddMethodInfo(methiNew);
    }

    // Select an item in a single selection list box
    {
        TMEngMethodInfo methiNew
        (
            L"SetListSel"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"WndId", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddInParm(L"ChildId", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddInParm(L"Index", tCIDMacroEng::EIntrinsics::Card4);
        m_c2MethId_SetListSel = c2AddMethodInfo(methiNew);
    }

    // Select the value of a track bar
    {
        TMEngMethodInfo methiNew
        (
            L"SetTrackBar"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"WndId", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddInParm(L"ChildId", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddInParm(L"ToSet", tCIDMacroEng::EIntrinsics::Int4);
        m_c2MethId_SetTrackBar = c2AddMethodInfo(methiNew);
    }

    // Set the position or size of a window
    {
        TMEngMethodInfo methiNew
        (
            L"SetWindowPos"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"WndId", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddInParm(L"ChildId", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddInParm(L"XPos", tCIDMacroEng::EIntrinsics::Int4);
        methiNew.c2AddInParm(L"YPos", tCIDMacroEng::EIntrinsics::Int4);
        m_c2MethId_SetWindowPos = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"SetWindowSize"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"WndId", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddInParm(L"ChildId", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddInParm(L"Width", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddInParm(L"Height", tCIDMacroEng::EIntrinsics::Card4);
        m_c2MethId_SetWindowSize = c2AddMethodInfo(methiNew);
    }

    // Set window text
    {
        TMEngMethodInfo methiNew
        (
            L"SetWindowText"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"WndId", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddInParm(L"ChildId", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddInParm(L"ToSet", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_SetWindowText = c2AddMethodInfo(methiNew);
    }

    // Show or hide a window
    {
        TMEngMethodInfo methiNew
        (
            L"ShowWindow"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"WndId", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddInParm(L"ChildId", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddInParm(L"State", tCIDMacroEng::EIntrinsics::Boolean);
        m_c2MethId_ShowWindow = c2AddMethodInfo(methiNew);
    }

    // Force the app to the top
    {
        TMEngMethodInfo methiNew
        (
            L"StandardOp"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"WndId", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddInParm(L"ChildId", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddInParm(L"OpToDo", m_c2EnumId_StdOps);
        m_c2MethId_StandardOp = c2AddMethodInfo(methiNew);
    }

    // Start the program if not already running
    {
        TMEngMethodInfo methiNew
        (
            L"StartApp"
            , tCIDMacroEng::EIntrinsics::Boolean
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        m_c2MethId_StartApp = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"StartAppWithParams"
            , tCIDMacroEng::EIntrinsics::Boolean
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"Params", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_StartAppWithParams = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"StartViaOpen"
            , tCIDMacroEng::EIntrinsics::Boolean
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"DocFile", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_StartViaOpen = c2AddMethodInfo(methiNew);
    }

    // Stop the program if running
    {
        TMEngMethodInfo methiNew
        (
            L"StopApp"
            , tCIDMacroEng::EIntrinsics::Boolean
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        m_c2MethId_StopApp = c2AddMethodInfo(methiNew);
    }
}


TMEngClassVal*
TCQCAppCtrlInfo::pmecvMakeStorage(  const   TString&               strName
                                    ,       TCIDMacroEngine&
                                    , const tCIDMacroEng::EConstTypes eConst) const
{
    return new TCQCAppCtrlVal(strName, c2Id(), eConst);
}


// ---------------------------------------------------------------------------
//  TCQCAppCtrlInfo: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TCQCAppCtrlInfo::bInvokeMethod(         TCIDMacroEngine&    meOwner
                                , const TMEngMethodInfo&    methiTarget
                                ,       TMEngClassVal&      mecvInstance)
{
    TCQCAppCtrlVal& mecvActual = static_cast<TCQCAppCtrlVal&>(mecvInstance);
    const tCIDLib::TCard2 c2MethId = methiTarget.c2Id();
    const tCIDLib::TCard4 c4FirstInd = meOwner.c4FirstParmInd(methiTarget);


    tCIDLib::TCard4 c4Res;
    tCIDLib::TInt4  i4Res;
    if ((c2MethId == m_c2MethId_AddWindow)
    ||  (c2MethId == m_c2MethId_AddWindowByClass))
    {
        try
        {
            TMEngCard4Val& mecvRet = meOwner.mecvStackAtAs<TMEngCard4Val>(c4FirstInd - 1);
            const TString& strVal = meOwner.strStackValAt(c4FirstInd);
            mecvRet.c4Value
            (
                (c2MethId == m_c2MethId_AddWindow)
                ? mecvActual.c4AddWindow(strVal)
                : mecvActual.c4AddWindowByClass(strVal)
            );
        }

        catch(const TError& errToCatch)
        {
            TestExcept(meOwner, errToCatch, m_c4ErrId_AddWindow, mecvActual);
        }
    }
     else if (c2MethId == m_c2MethId_ClickButton)
    {
        try
        {
            mecvActual.i4SendMsg
            (
                meOwner.c4StackValAt(c4FirstInd)
                , meOwner.c4StackValAt(c4FirstInd + 1)
                , 0xF5
                , 0
                , 0
                , kCIDLib::True
            );
        }

        catch(const TError& errToCatch)
        {
            TestExcept(meOwner, errToCatch, m_c4ErrId_SendMsg, mecvActual);
        }
    }
     else if (c2MethId == m_c2MethId_Connect)
    {
        try
        {
            TMEngBooleanVal& mecvRet = meOwner.mecvStackAtAs<TMEngBooleanVal>(c4FirstInd - 1);
            mecvRet.bValue(mecvActual.bConnect());
        }

        catch(const TError& errToCatch)
        {
            ThrowAnErr(meOwner, m_c4ErrId_Connect, errToCatch.strErrText());
        }
    }
     else if (c2MethId == m_c2MethId_DefCtor)
    {
        // Just reset the value object
        mecvActual.Reset();
    }
     else if (c2MethId == m_c2MethId_Disconnect)
    {
        try
        {
            mecvActual.bDisconnect(kCIDLib::True);
        }

        catch(const TError& errToCatch)
        {
            ThrowAnErr(meOwner, m_c4ErrId_Disconnect, errToCatch.strErrText());
        }
    }
     else if ((c2MethId == m_c2MethId_IsRegistered)
          ||  (c2MethId == m_c2MethId_IsRunning))
    {
        try
        {
            TMEngBooleanVal& mecvRet = meOwner.mecvStackAtAs<TMEngBooleanVal>(c4FirstInd - 1);

            if (c2MethId == m_c2MethId_IsRegistered)
                mecvRet.bValue(mecvActual.bIsRegistered());
            else
                mecvRet.bValue(mecvActual.bIsRunning());
        }

        catch(const TError& errToCatch)
        {
            TestExcept(meOwner, errToCatch, m_c4ErrId_Unknown, mecvActual);
        }
    }
     else if (c2MethId == m_c2MethId_IsVisible)
    {
        try
        {
            TMEngBooleanVal& mecvRet = meOwner.mecvStackAtAs<TMEngBooleanVal>(c4FirstInd - 1);
            mecvRet.bValue
            (
                mecvActual.bIsVisible
                (
                    meOwner.c4StackValAt(c4FirstInd)
                    , meOwner.c4StackValAt(c4FirstInd + 1)
                )
            );
        }

        catch(const TError& errToCatch)
        {
            TestExcept(meOwner, errToCatch, m_c4ErrId_QueryVisibility, mecvActual);
        }
    }
     else if (c2MethId == m_c2MethId_QueryButtState)
    {
        try
        {
            i4Res = mecvActual.i4SendMsg
            (
                meOwner.c4StackValAt(c4FirstInd)
                , meOwner.c4StackValAt(c4FirstInd + 1)
                , 0xF0
                , 0
                , 0
                , kCIDLib::False
            );
            TMEngEnumVal& mecvState = meOwner.mecvStackAtAs<TMEngEnumVal>(c4FirstInd - 1);
            mecvState.c4Ordinal(i4Res);
        }

        catch(const TError& errToCatch)
        {
            TestExcept(meOwner, errToCatch, m_c4ErrId_SendMsg, mecvActual);
        }
    }
     else if (c2MethId == m_c2MethId_QueryText)
    {
        try
        {
            TMEngStringVal& mecvFill = meOwner.mecvStackAtAs<TMEngStringVal>(c4FirstInd + 2);
            TString& strToFill = mecvFill.strValue();
            c4Res = mecvActual.c4QueryText
            (
                meOwner.c4StackValAt(c4FirstInd)
                , meOwner.c4StackValAt(c4FirstInd + 1)
                , strToFill
            );

            TMEngCard4Val& mecvRet = meOwner.mecvStackAtAs<TMEngCard4Val>(c4FirstInd - 1);
            mecvRet.c4Value(c4Res);
        }

        catch(const TError& errToCatch)
        {
            TestExcept(meOwner, errToCatch, m_c4ErrId_QueryText, mecvActual);
        }
    }
     else if (c2MethId == m_c2MethId_QueryListSel)
    {
        try
        {
            TMEngCard4Val& mecvRet = meOwner.mecvStackAtAs<TMEngCard4Val>(c4FirstInd - 1);
            mecvRet.c4Value
            (
                mecvActual.c4QueryListSel
                (
                    meOwner.c4StackValAt(c4FirstInd)
                    , meOwner.c4StackValAt(c4FirstInd + 1)
                )
            );
        }

        catch(const TError& errToCatch)
        {
            TestExcept(meOwner, errToCatch, m_c4ErrId_SendMsg, mecvActual);
        }
    }
     else if (c2MethId == m_c2MethId_QueryWndStyles)
    {
        try
        {
            // Get the window styles
            c4Res = mecvActual.c4QueryWndStyles
            (
                meOwner.c4StackValAt(c4FirstInd)
                , meOwner.c4StackValAt(c4FirstInd + 1)
            );

            TMEngCard4Val& mecvRet = meOwner.mecvStackAtAs<TMEngCard4Val>(c4FirstInd - 1);
            mecvRet.c4Value(c4Res);
        }

        catch(const TError& errToCatch)
        {
            TestExcept(meOwner, errToCatch, m_c4ErrId_QueryWndStyles, mecvActual);
        }
    }
     else if (c2MethId == m_c2MethId_QueryTrackBar)
    {
        try
        {
            TMEngInt4Val& mecvRet = meOwner.mecvStackAtAs<TMEngInt4Val>(c4FirstInd - 1);
            mecvRet.i4Value
            (
                mecvActual.i4QueryTrackBar
                (
                    meOwner.c4StackValAt(c4FirstInd)
                    , meOwner.c4StackValAt(c4FirstInd + 1)
                )
            );
        }

        catch(const TError& errToCatch)
        {
            TestExcept(meOwner, errToCatch, m_c4ErrId_SendMsg, mecvActual);
        }
    }
     else if (c2MethId == m_c2MethId_SendExtKey)
    {
        try
        {
            TMEngEnumVal& mecvState = meOwner.mecvStackAtAs<TMEngEnumVal>(c4FirstInd + 2);
            mecvActual.SendExtKey
            (
                meOwner.c4StackValAt(c4FirstInd)
                , meOwner.c4StackValAt(c4FirstInd + 1)
                , tCIDCtrls::EExtKeys(m_pmeciExtKeys->c4MapValue(mecvState))
                , meOwner.bStackValAt(c4FirstInd + 3)
                , meOwner.bStackValAt(c4FirstInd + 4)
                , meOwner.bStackValAt(c4FirstInd + 5)
            );
        }

        catch(const TError& errToCatch)
        {
            TestExcept(meOwner, errToCatch, m_c4ErrId_SendKey, mecvActual);
        }
    }
     else if (c2MethId == m_c2MethId_SendKey)
    {
        try
        {
            mecvActual.SendKey
            (
                meOwner.c4StackValAt(c4FirstInd)
                , meOwner.c4StackValAt(c4FirstInd + 1)
                , meOwner.chStackValAt(c4FirstInd + 2)
                , meOwner.bStackValAt(c4FirstInd + 3)
                , meOwner.bStackValAt(c4FirstInd + 4)
                , meOwner.bStackValAt(c4FirstInd + 5)
            );
        }

        catch(const TError& errToCatch)
        {
            TestExcept(meOwner, errToCatch, m_c4ErrId_SendKey, mecvActual);
        }
    }
     else if (c2MethId == m_c2MethId_SendMenuItem)
    {
        try
        {
            mecvActual.i4SendMsg
            (
                meOwner.c4StackValAt(c4FirstInd)
                , meOwner.c4StackValAt(c4FirstInd + 1)
                , 0x111
                , TRawBits::c4From16(0, meOwner.c2StackValAt(c4FirstInd + 2))
                , 0
                , kCIDLib::True
            );
        }

        catch(const TError& errToCatch)
        {
            TestExcept(meOwner, errToCatch, m_c4ErrId_SendMsg, mecvActual);
        }
    }
     else if (c2MethId == m_c2MethId_SendMsg)
    {
        try
        {
            i4Res = mecvActual.i4SendMsg
            (
                meOwner.c4StackValAt(c4FirstInd)
                , meOwner.c4StackValAt(c4FirstInd + 1)
                , meOwner.c4StackValAt(c4FirstInd + 2)
                , meOwner.c4StackValAt(c4FirstInd + 3)
                , meOwner.i4StackValAt(c4FirstInd + 4)
                , meOwner.bStackValAt(c4FirstInd + 5)
            );

            TMEngInt4Val& mecvRet = meOwner.mecvStackAtAs<TMEngInt4Val>(c4FirstInd - 1);
            mecvRet.i4Value(i4Res);
        }

        catch(const TError& errToCatch)
        {
            TestExcept(meOwner, errToCatch, m_c4ErrId_SendMsg, mecvActual);
        }
    }
     else if (c2MethId == m_c2MethId_SetButtState)
    {
        try
        {
            TMEngEnumVal& mecvState = meOwner.mecvStackAtAs<TMEngEnumVal>(c4FirstInd + 2);
            mecvActual.i4SendMsg
            (
                meOwner.c4StackValAt(c4FirstInd)
                , meOwner.c4StackValAt(c4FirstInd + 1)
                , 0xF1
                , mecvState.c4Ordinal()
                , 0
                , kCIDLib::False
            );
        }

        catch(const TError& errToCatch)
        {
            TestExcept(meOwner, errToCatch, m_c4ErrId_SendMsg, mecvActual);
        }
    }
     else if (c2MethId == m_c2MethId_SetFocus)
    {
        try
        {
            mecvActual.SetFocus
            (
                meOwner.c4StackValAt(c4FirstInd)
                , meOwner.c4StackValAt(c4FirstInd + 1)
            );
        }

        catch(const TError& errToCatch)
        {
            TestExcept(meOwner, errToCatch, m_c4ErrId_SetFocus, mecvActual);
        }
    }
     else if (c2MethId == m_c2MethId_SetListSel)
    {
        try
        {
            mecvActual.SetListSel
            (
                meOwner.c4StackValAt(c4FirstInd)
                , meOwner.c4StackValAt(c4FirstInd + 1)
                , meOwner.c4StackValAt(c4FirstInd + 2)
            );
        }

        catch(const TError& errToCatch)
        {
            TestExcept(meOwner, errToCatch, m_c4ErrId_SendMsg, mecvActual);
        }
    }
     else if (c2MethId == m_c2MethId_SetTrackBar)
    {
        try
        {
            mecvActual.SetTrackBar
            (
                meOwner.c4StackValAt(c4FirstInd)
                , meOwner.c4StackValAt(c4FirstInd + 1)
                , meOwner.i4StackValAt(c4FirstInd + 2)
            );
        }

        catch(const TError& errToCatch)
        {
            TestExcept(meOwner, errToCatch, m_c4ErrId_SendMsg, mecvActual);
        }
    }
     else if (c2MethId == m_c2MethId_SetWindowPos)
    {
        try
        {
            mecvActual.SetWindowPos
            (
                meOwner.c4StackValAt(c4FirstInd)
                , meOwner.c4StackValAt(c4FirstInd + 1)
                , meOwner.i4StackValAt(c4FirstInd + 2)
                , meOwner.i4StackValAt(c4FirstInd + 3)
            );
        }

        catch(const TError& errToCatch)
        {
            TestExcept(meOwner, errToCatch, m_c4ErrId_SetSizePos, mecvActual);
        }
    }
     else if (c2MethId == m_c2MethId_SetWindowSize)
    {
        try
        {
            mecvActual.SetWindowSize
            (
                meOwner.c4StackValAt(c4FirstInd)
                , meOwner.c4StackValAt(c4FirstInd + 1)
                , meOwner.c4StackValAt(c4FirstInd + 2)
                , meOwner.c4StackValAt(c4FirstInd + 3)
            );
        }

        catch(const TError& errToCatch)
        {
            TestExcept(meOwner, errToCatch, m_c4ErrId_SetSizePos, mecvActual);
        }
    }
     else if (c2MethId == m_c2MethId_SetWindowText)
    {
        try
        {
            mecvActual.SetWindowText
            (
                meOwner.c4StackValAt(c4FirstInd)
                , meOwner.c4StackValAt(c4FirstInd + 1)
                , meOwner.strStackValAt(c4FirstInd + 2)
            );
        }

        catch(const TError& errToCatch)
        {
            TestExcept(meOwner, errToCatch, m_c4ErrId_SetText, mecvActual);
        }
    }
     else if (c2MethId == m_c2MethId_ShowWindow)
    {
        try
        {
            mecvActual.ShowWindow
            (
                meOwner.c4StackValAt(c4FirstInd)
                , meOwner.c4StackValAt(c4FirstInd + 1)
                , meOwner.bStackValAt(c4FirstInd + 2)
            );
        }

        catch(const TError& errToCatch)
        {
            TestExcept(meOwner, errToCatch, m_c4ErrId_SetVisibility, mecvActual);
        }
    }
     else if (c2MethId == m_c2MethId_StandardOp)
    {
        try
        {
            TMEngEnumVal& mecvOp = meOwner.mecvStackAtAs<TMEngEnumVal>(c4FirstInd + 2);
            mecvActual.StandardOp
            (
                meOwner.c4StackValAt(c4FirstInd)
                , meOwner.c4StackValAt(c4FirstInd + 1)
                , tCQCKit::EStdACOps(m_pmeciStdOps->c4MapValue(mecvOp))
            );
        }

        catch(const TError& errToCatch)
        {
            TestExcept(meOwner, errToCatch, m_c4ErrId_StdOp, mecvActual);
        }
    }
     else if ((c2MethId == m_c2MethId_StartApp)
          ||  (c2MethId == m_c2MethId_StartAppWithParams)
          ||  (c2MethId == m_c2MethId_StartViaOpen)
          ||  (c2MethId == m_c2MethId_StopApp))
    {
        try
        {
            TMEngBooleanVal& mecvRet = meOwner.mecvStackAtAs<TMEngBooleanVal>(c4FirstInd - 1);

            // Call the start or stop method
            if (c2MethId == m_c2MethId_StartApp)
            {
                mecvRet.bValue(mecvActual.bStart());
            }
             else if (c2MethId == m_c2MethId_StartAppWithParams)
            {
                // For this one, get the parameters
                mecvRet.bValue
                (
                    mecvActual.bStart(meOwner.strStackValAt(c4FirstInd))
                );
            }
             else if (c2MethId == m_c2MethId_StartViaOpen)
            {
                // FOo this one, get the target file path
                mecvRet.bValue
                (
                    mecvActual.bStartViaOpen(meOwner.strStackValAt(c4FirstInd))
                );
            }
             else
            {
                mecvRet.bValue(mecvActual.bStop());
            }
        }

        catch(const TError& errToCatch)
        {
            TestExcept
            (
                meOwner
                , errToCatch
                , (c2MethId == m_c2MethId_StopApp) ? m_c4ErrId_Stop
                                                   : m_c4ErrId_Start
                , mecvActual
            );
        }
    }
     else if (c2MethId == m_c2MethId_SetInfo)
    {
        // Set the binding and moniker
        mecvActual.Initialize
        (
            meOwner.strStackValAt(c4FirstInd)
            , meOwner.strStackValAt(c4FirstInd + 1)
            , meOwner.strStackValAt(c4FirstInd + 2)
            , meOwner.strStackValAt(c4FirstInd + 3)
            , meOwner.strStackValAt(c4FirstInd + 4)
            , meOwner.strStackValAt(c4FirstInd + 5)
            , meOwner.strStackValAt(c4FirstInd + 6)
        );
    }
     else
    {
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


// ---------------------------------------------------------------------------
//  TCQCAppCtrlInfo: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TCQCAppCtrlInfo::TestExcept(        TCIDMacroEngine&    meOwner
                            , const TError&             errCaught
                            , const tCIDLib::TCard4     c4Default
                            ,       TCQCAppCtrlVal&     mecvTarget)
{
    if (errCaught.eClass() == tCIDLib::EErrClasses::LostConnection)
    {
        mecvTarget.bDisconnect(kCIDLib::False);
        ThrowAnErr(meOwner, m_c4ErrId_NotConnected);
    }
    ThrowAnErr(meOwner, c4Default, errCaught.strErrText());
}


tCIDLib::TVoid
TCQCAppCtrlInfo::ThrowAnErr(        TCIDMacroEngine&    meOwner
                            , const tCIDLib::TCard4     c4ToThrow
                            , const TString&            strText) const
{
    // Set the exception info on the engine
    meOwner.SetException
    (
        m_c2EnumId_Errors
        , strClassPath()
        , c4ToThrow
        , m_pmeciErrors->strPartialName(c4ToThrow)
        , strText
        , meOwner.c4CurLine()
    );

    // And throw the excpetion that represents a macro level exception
    throw TExceptException();
}


tCIDLib::TVoid
TCQCAppCtrlInfo::ThrowAnErr(        TCIDMacroEngine&    meOwner
                            , const tCIDLib::TCard4     c4ToThrow) const
{
    // Set the exception info on the engine
    meOwner.SetException
    (
        m_c2EnumId_Errors
        , strClassPath()
        , c4ToThrow
        , m_pmeciErrors->strPartialName(c4ToThrow)
        , m_pmeciErrors->strTextValue(c4ToThrow)
        , meOwner.c4CurLine()
    );

    // And throw the excpetion that represents a macro level exception
    throw TExceptException();
}


