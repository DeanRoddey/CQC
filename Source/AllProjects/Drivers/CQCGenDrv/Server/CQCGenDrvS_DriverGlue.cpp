//
// FILE NAME: CQCGenDrvS_DriverGlue.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/09/2003
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
//  This is the implementation file for the generic driver glue class, which
//  connects the CQCServer driver interfaces to the macro level classes that
//  allow development of drivers in the macro language.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCGenDrvS_.hpp"


// ---------------------------------------------------------------------------
//  We have to kind of cheat here and bring in a few headers from CQCKit and
//  from CIDMacroEng. This is done to avoid force all of the macro engine
//  headers onto every users of CQCKit (which is everyone.)
// ---------------------------------------------------------------------------
#include    "CIDMacroEng_CommClasses_.hpp"
#include    "CIDMacroEng_SockClasses_.hpp"



// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCGenDrvGlue,TCQCServerBase)



// ---------------------------------------------------------------------------
//   CLASS: TCQCGenDrvGlue
//  PREFIX: sdrv
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCGenDrvGlue: Public, static data
// ---------------------------------------------------------------------------
tCQCKit::EVerboseLvls   TCQCGenDrvGlue::s_eDefVerbosity = tCQCKit::EVerboseLvls::Off;
TMutex*                 TCQCGenDrvGlue::s_pmtxSync;
TCQCGenDrvGlue*         TCQCGenDrvGlue::s_psdrvCurrent;
TCQCDriverObjCfg        TCQCGenDrvGlue::s_cqcdcToLoad;


// ---------------------------------------------------------------------------
//  TCQCGenDrvGlue: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCGenDrvGlue::TCQCGenDrvGlue() :

    TCQCServerBase()
    , m_bSimMode(kCIDLib::False)
    , m_c2MethId_ClientCmd(kCIDMacroEng::c2BadId)
    , m_c2MethId_Conn(kCIDMacroEng::c2BadId)
    , m_c2MethId_FldChBool(kCIDMacroEng::c2BadId)
    , m_c2MethId_FldChCard(kCIDMacroEng::c2BadId)
    , m_c2MethId_FldChFloat(kCIDMacroEng::c2BadId)
    , m_c2MethId_FldChInt(kCIDMacroEng::c2BadId)
    , m_c2MethId_FldChString(kCIDMacroEng::c2BadId)
    , m_c2MethId_FldChStrList(kCIDMacroEng::c2BadId)
    , m_c2MethId_FldChTime(kCIDMacroEng::c2BadId)
    , m_c2MethId_FreeCommRes(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetCommRes(kCIDMacroEng::c2BadId)
    , m_c2MethId_Poll(kCIDMacroEng::c2BadId)
    , m_c2MethId_QueryBoolVal(kCIDMacroEng::c2BadId)
    , m_c2MethId_QueryBufVal(kCIDMacroEng::c2BadId)
    , m_c2MethId_QueryCardVal(kCIDMacroEng::c2BadId)
    , m_c2MethId_QueryIntVal(kCIDMacroEng::c2BadId)
    , m_c2MethId_QueryTextVal(kCIDMacroEng::c2BadId)
    , m_c2MethId_WaitConfig(kCIDMacroEng::c2BadId)
    , m_pmeciCommResEnum(nullptr)
    , m_pmeciMemBuf(nullptr)
    , m_pmeciDriver(nullptr)
    , m_pmeciVerbLevelEnum(nullptr)
    , m_pmecvDriver(nullptr)
    , m_pmeehLogger(nullptr)
    , m_pmefrData(nullptr)
    , m_pmeTarget(nullptr)
{
}

TCQCGenDrvGlue::TCQCGenDrvGlue(const TCQCDriverObjCfg& cqcdcToLoad) :

    TCQCServerBase(cqcdcToLoad)
    , m_bSimMode(kCIDLib::False)
    , m_c2MethId_ClientCmd(kCIDMacroEng::c2BadId)
    , m_c2MethId_Conn(kCIDMacroEng::c2BadId)
    , m_c2MethId_FldChBool(kCIDMacroEng::c2BadId)
    , m_c2MethId_FldChCard(kCIDMacroEng::c2BadId)
    , m_c2MethId_FldChFloat(kCIDMacroEng::c2BadId)
    , m_c2MethId_FldChInt(kCIDMacroEng::c2BadId)
    , m_c2MethId_FldChString(kCIDMacroEng::c2BadId)
    , m_c2MethId_FldChStrList(kCIDMacroEng::c2BadId)
    , m_c2MethId_FldChTime(kCIDMacroEng::c2BadId)
    , m_c2MethId_FreeCommRes(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetCommRes(kCIDMacroEng::c2BadId)
    , m_c2MethId_Poll(kCIDMacroEng::c2BadId)
    , m_c2MethId_QueryBoolVal(kCIDMacroEng::c2BadId)
    , m_c2MethId_QueryBufVal(kCIDMacroEng::c2BadId)
    , m_c2MethId_QueryCardVal(kCIDMacroEng::c2BadId)
    , m_c2MethId_QueryIntVal(kCIDMacroEng::c2BadId)
    , m_c2MethId_QueryTextVal(kCIDMacroEng::c2BadId)
    , m_c2MethId_WaitConfig(kCIDMacroEng::c2BadId)
    , m_pmeciCommResEnum(nullptr)
    , m_pmeciMemBuf(nullptr)
    , m_pmeciDriver(nullptr)
    , m_pmeciVerbLevelEnum(nullptr)
    , m_pmecvDriver(nullptr)
    , m_pmeehLogger(nullptr)
    , m_pmefrData(nullptr)
    , m_pmeTarget(nullptr)
{
}

TCQCGenDrvGlue::~TCQCGenDrvGlue()
{
    //
    //  If not in sim mode, then we own the driver and engine stuff and need
    //  to clean them up. If in sim mode, then drop the pointer to ourself
    //  so that the harness will know that we have stopped running.
    //
    if (m_bSimMode)
    {
        TLocker mtxSync(s_pmtxSync);
        s_psdrvCurrent = nullptr;
    }
     else
    {
        delete m_pmeTarget;
        delete m_pmefrData;
        delete m_pmeehLogger;
        delete m_pmecvDriver;
    }
}



// ---------------------------------------------------------------------------
//  TCQCGenDrvGlue: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TCQCGenDrvGlue::bQueryData( const   TString&            strQueryType
                            , const TString&            strDataName
                            ,       tCIDLib::TCard4&    c4OutBytes
                            ,       THeapBuf&           mbufToFill)
{
    tCIDLib::TBoolean bRet = kCIDLib::False;
    try
    {
        // Make sure we get the stack cleaned back up
        TMEngCallStackJan janStack(m_pmeTarget);

        // Forward to the macro class
        m_pmeTarget->PushPoolValue(tCIDMacroEng::EIntrinsics::Boolean, tCIDMacroEng::EConstTypes::NonConst);
        m_pmeTarget->PushString(strQueryType, tCIDMacroEng::EConstTypes::Const);
        m_pmeTarget->PushString(strDataName, tCIDMacroEng::EConstTypes::Const);

        //
        //  For the out's we have to create value objects. We push them as parms,
        //  which they are, so that the engine won't try to delete them when it
        //  pops them.
        //
        TMEngCard4Val mecvBytes(TString::strEmpty(), tCIDMacroEng::EConstTypes::NonConst);
        m_pmeTarget->PushValue(&mecvBytes, tCIDMacroEng::EStackItems::Parm);

        TMEngMemBufVal mecvToFill
        (
            TString::strEmpty()
            , m_pmeciMemBuf->c2Id()
            , tCIDMacroEng::EConstTypes::NonConst
            , &mbufToFill
        );
        m_pmeTarget->PushValue(&mecvToFill, tCIDMacroEng::EStackItems::Parm);

        m_pmeTarget->meciPushMethodCall(m_pmeciDriver->c2Id(), m_c2MethId_QueryBufVal);
        m_pmeciDriver->Invoke
        (
            *m_pmeTarget
            , *m_pmecvDriver
            , m_c2MethId_QueryBufVal
            , tCIDMacroEng::EDispatch::Poly
        );

        // Get the boolean return value out before we clean up the stack
        bRet = m_pmeTarget->bStackValAt
        (
            m_pmeTarget->c4StackTop() - 6, kCIDLib::True
        );

        // Get the out bytes value off the stack
        c4OutBytes = m_pmeTarget->c4StackValAt(m_pmeTarget->c4StackTop() - 3);
    }

    catch(const TExceptException&)
    {
        // If we get a macro level exception, just log it
        m_pmeTarget->LogLastExcept();

        TString strClass;
        TString strText;
        tCIDLib::TCard4 c4Line;
        m_pmeTarget->bGetLastExceptInfo(strClass, strText, c4Line);

        facCQCGenDrvS().ThrowErr
        (
            strClass
            , c4Line
            , kGDrvSErrs::errcDrv_ExceptInCall
            , strText
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::AppStatus
            , TString(L"QueryBufVal")
        );
    }

    catch(...)
    {
        TString strClass;
        TString strText;
        tCIDLib::TCard4 c4Line;
        m_pmeTarget->bGetLastExceptInfo(strClass, strText, c4Line);

        facCQCGenDrvS().ThrowErr
        (
            strClass
            , c4Line
            , kGDrvSErrs::errcDrv_SysExceptInCall
            , strText
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::AppStatus
            , TString(L"QueryBufVal")
        );
        bRet = kCIDLib::False;
    }
    return bRet;
}


tCIDLib::TBoolean TCQCGenDrvGlue::bQueryVal(const TString& strValId)
{
    tCIDLib::TBoolean bRet = kCIDLib::False;
    try
    {
        // Make sure we get the stack cleaned back up
        TMEngCallStackJan janStack(m_pmeTarget);

        // Forward to the macro class
        m_pmeTarget->PushPoolValue(tCIDMacroEng::EIntrinsics::Boolean, tCIDMacroEng::EConstTypes::NonConst);
        m_pmeTarget->PushString(strValId, tCIDMacroEng::EConstTypes::Const);
        m_pmeTarget->meciPushMethodCall(m_pmeciDriver->c2Id(), m_c2MethId_QueryBoolVal);
        m_pmeciDriver->Invoke
        (
            *m_pmeTarget
            , *m_pmecvDriver
            , m_c2MethId_QueryBoolVal
            , tCIDMacroEng::EDispatch::Poly
        );

        // Get the return out before we clean up the stack
        bRet = m_pmeTarget->bStackValAt
        (
            m_pmeTarget->c4StackTop() - 3, kCIDLib::True
        );
    }

    catch(const TExceptException&)
    {
        // Log the CML level exception and throw a C++ exception
        m_pmeTarget->LogLastExcept();

        TString strClass;
        TString strText;
        tCIDLib::TCard4 c4Line;
        m_pmeTarget->bGetLastExceptInfo(strClass, strText, c4Line);

        facCQCGenDrvS().ThrowErr
        (
            strClass
            , c4Line
            , kGDrvSErrs::errcDrv_ExceptInCall
            , strText
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::AppStatus
            , TString(L"QueryBoolVal")
        );
    }

    catch(...)
    {
        TString strClass;
        TString strText;
        tCIDLib::TCard4 c4Line;
        m_pmeTarget->bGetLastExceptInfo(strClass, strText, c4Line);

        facCQCGenDrvS().ThrowErr
        (
            strClass
            , c4Line
            , kGDrvSErrs::errcDrv_SysExceptInCall
            , strText
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::AppStatus
            , TString(L"QueryBoolVal")
        );
        bRet = kCIDLib::False;
    }
    return bRet;
}


tCIDLib::TBoolean
TCQCGenDrvGlue::bQueryTextVal(  const   TString&    strQueryType
                                , const TString&    strDataName
                                ,       TString&    strToFill)
{
    tCIDLib::TBoolean bRet = kCIDLib::False;
    try
    {
        // Make sure we get the stack cleaned back up
        TMEngCallStackJan janStack(m_pmeTarget);

        //
        //  Forward to the macro class. Note that we cannot create a string
        //  value object to push, in order to avoid copying the text data
        //  because the CML level string class uses a TPathStr internally, so
        //  we'd still have to create a temp TPathStr object anyway. So we
        //  just push a pool value and get the value out again later.
        //
        m_pmeTarget->PushPoolValue(tCIDMacroEng::EIntrinsics::Boolean, tCIDMacroEng::EConstTypes::NonConst);
        m_pmeTarget->PushString(strQueryType, tCIDMacroEng::EConstTypes::Const);
        m_pmeTarget->PushString(strDataName, tCIDMacroEng::EConstTypes::Const);
        m_pmeTarget->PushPoolValue(tCIDMacroEng::EIntrinsics::String, tCIDMacroEng::EConstTypes::NonConst);

        m_pmeTarget->meciPushMethodCall(m_pmeciDriver->c2Id(), m_c2MethId_QueryTextVal);
        m_pmeciDriver->Invoke
        (
            *m_pmeTarget
            , *m_pmecvDriver
            , m_c2MethId_QueryTextVal
            , tCIDMacroEng::EDispatch::Poly
        );

        // Get the return and output param out before we clean up the stack
        bRet = m_pmeTarget->bStackValAt
        (
            m_pmeTarget->c4StackTop() - 5, kCIDLib::True
        );

        strToFill = m_pmeTarget->strStackValAt
        (
            m_pmeTarget->c4StackTop() - 2, kCIDLib::True
        );
    }

    catch(const TExceptException&)
    {
        // If we get a macro level exception, just log it
        m_pmeTarget->LogLastExcept();

        TString strClass;
        TString strText;
        tCIDLib::TCard4 c4Line;
        m_pmeTarget->bGetLastExceptInfo(strClass, strText, c4Line);

        facCQCGenDrvS().ThrowErr
        (
            strClass
            , c4Line
            , kGDrvSErrs::errcDrv_ExceptInCall
            , strText
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::AppStatus
            , TString(L"QueryTextVal")
        );

        // Assume a failure
        bRet = kCIDLib::False;
    }

    catch(...)
    {
        TString strClass;
        TString strText;
        tCIDLib::TCard4 c4Line;
        m_pmeTarget->bGetLastExceptInfo(strClass, strText, c4Line);

        facCQCGenDrvS().ThrowErr
        (
            strClass
            , c4Line
            , kGDrvSErrs::errcDrv_SysExceptInCall
            , strText
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::AppStatus
            , TString(L"QueryTextVal")
        );
    }
    return bRet;
}


tCIDLib::TCard4 TCQCGenDrvGlue::c4QueryVal(const TString& strValId)
{
    tCIDLib::TCard4 c4Ret = 0;
    try
    {
        // Make sure we get the stack cleaned back up
        TMEngCallStackJan janStack(m_pmeTarget);

        // Forward to the macro class
        m_pmeTarget->PushPoolValue(tCIDMacroEng::EIntrinsics::Card4, tCIDMacroEng::EConstTypes::NonConst);
        m_pmeTarget->PushString(strValId, tCIDMacroEng::EConstTypes::Const);
        m_pmeTarget->meciPushMethodCall(m_pmeciDriver->c2Id(), m_c2MethId_QueryCardVal);
        m_pmeciDriver->Invoke
        (
            *m_pmeTarget
            , *m_pmecvDriver
            , m_c2MethId_QueryCardVal
            , tCIDMacroEng::EDispatch::Poly
        );

        // Get the return out before we clean up the stack
        c4Ret = m_pmeTarget->c4StackValAt
        (
            m_pmeTarget->c4StackTop() - 3, kCIDLib::True
        );
    }

    catch(const TExceptException&)
    {
        // Log the CML level exception and throw a C++ exception
        m_pmeTarget->LogLastExcept();

        TString strClass;
        TString strText;
        tCIDLib::TCard4 c4Line;
        m_pmeTarget->bGetLastExceptInfo(strClass, strText, c4Line);

        facCQCGenDrvS().ThrowErr
        (
            strClass
            , c4Line
            , kGDrvSErrs::errcDrv_ExceptInCall
            , strText
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::AppStatus
            , TString(L"QueryCardVal")
        );
    }

    catch(...)
    {
        TString strClass;
        TString strText;
        tCIDLib::TCard4 c4Line;
        m_pmeTarget->bGetLastExceptInfo(strClass, strText, c4Line);

        facCQCGenDrvS().ThrowErr
        (
            strClass
            , c4Line
            , kGDrvSErrs::errcDrv_SysExceptInCall
            , strText
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::AppStatus
            , TString(L"QueryCardVal")
        );
    }
    return c4Ret;
}


tCIDLib::TCard4
TCQCGenDrvGlue::c4SendCmd(const TString& strCmdId, const TString& strData)
{
    tCIDLib::TCard4 c4Ret = 0;
    try
    {
        // Make sure we get the stack cleaned back up
        TMEngCallStackJan janStack(m_pmeTarget);

        //
        //  Forward to the macro class. Note that we cannot create a string
        //  value object to push, in order to avoid copying the text data
        //  because the CML level string class uses a TPathStr internally, so
        //  we'd still have to create a temp TPathStr object anyway. So we
        //  just push a pool value and get the value out again later.
        //
        m_pmeTarget->PushPoolValue(tCIDMacroEng::EIntrinsics::Card4, tCIDMacroEng::EConstTypes::NonConst);
        m_pmeTarget->PushString(strCmdId, tCIDMacroEng::EConstTypes::Const);
        m_pmeTarget->PushString(strData, tCIDMacroEng::EConstTypes::Const);
        m_pmeTarget->meciPushMethodCall(m_pmeciDriver->c2Id(), m_c2MethId_ClientCmd);
        m_pmeciDriver->Invoke
        (
            *m_pmeTarget
            , *m_pmecvDriver
            , m_c2MethId_ClientCmd
            , tCIDMacroEng::EDispatch::Poly
        );

        // Get the return out before we clean up the stack
        c4Ret = m_pmeTarget->c4StackValAt
        (
            m_pmeTarget->c4StackTop() - 4, kCIDLib::True
        );
    }

    catch(const TExceptException&)
    {
        // If we get a macro level exception, just log it
        m_pmeTarget->LogLastExcept();

        TString strClass;
        TString strText;
        tCIDLib::TCard4 c4Line;
        m_pmeTarget->bGetLastExceptInfo(strClass, strText, c4Line);
        facCQCGenDrvS().ThrowErr
        (
            strClass
            , c4Line
            , kGDrvSErrs::errcDrv_ExceptInCall
            , strText
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::AppStatus
            , TString(L"SendCmd")
        );
    }

    catch(...)
    {
        TString strClass;
        TString strText;
        tCIDLib::TCard4 c4Line;
        m_pmeTarget->bGetLastExceptInfo(strClass, strText, c4Line);
        facCQCGenDrvS().ThrowErr
        (
            strClass
            , c4Line
            , kGDrvSErrs::errcDrv_SysExceptInCall
            , strText
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::AppStatus
            , TString(L"SendCmd")
        );
    }
    return c4Ret;
}


tCIDLib::TInt4 TCQCGenDrvGlue::i4QueryVal(const TString& strValId)
{
    tCIDLib::TInt4 i4Ret = 0;
    try
    {
        // Make sure we get the stack cleaned back up
        TMEngCallStackJan janStack(m_pmeTarget);

        // Forward to the macro class
        m_pmeTarget->PushPoolValue(tCIDMacroEng::EIntrinsics::Int4, tCIDMacroEng::EConstTypes::NonConst);
        m_pmeTarget->PushString(strValId, tCIDMacroEng::EConstTypes::Const);

        m_pmeTarget->meciPushMethodCall(m_pmeciDriver->c2Id(), m_c2MethId_QueryIntVal);
        m_pmeciDriver->Invoke
        (
            *m_pmeTarget
            , *m_pmecvDriver
            , m_c2MethId_QueryIntVal
            , tCIDMacroEng::EDispatch::Poly
        );

        // Get the return out before we clean up the stack
        i4Ret = m_pmeTarget->i4StackValAt
        (
            m_pmeTarget->c4StackTop() - 3, kCIDLib::True
        );
    }

    catch(const TExceptException&)
    {
        // Log the CML level exception and throw a C++ exception
        m_pmeTarget->LogLastExcept();

        TString strClass;
        TString strText;
        tCIDLib::TCard4 c4Line;
        m_pmeTarget->bGetLastExceptInfo(strClass, strText, c4Line);
        facCQCGenDrvS().ThrowErr
        (
            strClass
            , c4Line
            , kGDrvSErrs::errcDrv_ExceptInCall
            , strText
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::AppStatus
            , TString(L"QueryIntVal")
        );
    }

    catch(...)
    {
        TString strClass;
        TString strText;
        tCIDLib::TCard4 c4Line;
        m_pmeTarget->bGetLastExceptInfo(strClass, strText, c4Line);
        facCQCGenDrvS().ThrowErr
        (
            strClass
            , c4Line
            , kGDrvSErrs::errcDrv_SysExceptInCall
            , strText
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::AppStatus
            , TString(L"QueryIntVal")
        );
    }
    return i4Ret;
}



// ---------------------------------------------------------------------------
//  TCQCGenDrvGlue: Public, non-virtual methods
// ---------------------------------------------------------------------------

// A pass through to expose this to the CML driver class
tCIDLib::TVoid TCQCGenDrvGlue::SetDrvFields(TSetFieldList& colToSet)
{
    SetFields(colToSet);
}


tCIDLib::TVoid
TCQCGenDrvGlue::Simulate(       TCIDMacroEngine&        meTarget
                        ,       TMEngClassVal&          mecvDriver
                        , const TCQCDriverInfo&         meciDriver)
{
    // Remember we are in sim mode and store the pointers
    m_bSimMode      = kCIDLib::True;
    m_pmecvDriver   = &mecvDriver;
    m_pmeciDriver   = &meciDriver;
    m_pmeTarget     = &meTarget;

    //
    //  In this case, our config and moniker in sim mode are stored by
    //  the test harness in statics so that we can pick them up. So set
    //  them on our parent
    //
    SetConfigObj(s_cqcdcToLoad);

    // Update the pointer that lets the field monitor window see us
    {
        TLocker mtxSync(s_pmtxSync);
        s_psdrvCurrent = this;
    }

    //
    //  Set the default verbosity. We have special considerations here. We want
    //  the verbosity to be set for the init below, but we also want to be sure
    //  it gets set up correctly once the driver starts running. Normally we'd
    //  just call SetNewVerbosity, and set the default verbosity (last thing set
    //  via the menu.) But, tht won't get picked up until the driver loop starts
    //  running. So we then also set the verbosity directly, so that's alreayd
    //  set for init.
    //
    s_psdrvCurrent->SetNewVerbosity(s_eDefVerbosity);
    s_psdrvCurrent->SetVerboseLevel(s_eDefVerbosity);

    //
    //  Fake an init call to initialize it. Normally this happens in the
    //  context of the driver thread, but for development purposes we want
    //  to only do it once and give up if it fails.
    //
    try
    {
        tCQCKit::EDrvInitRes eRes = eInitialize();
        if (eRes == tCQCKit::EDrvInitRes::Failed)
        {
            TError errFailed
            (
                facCQCGenDrvS().strName()
                , CID_FILE
                , CID_LINE
                , L"The driver reported initialization failure"
            );
            meTarget.Exception(errFailed);
            return;
        }
    }

    catch(const TDbgExitException&)
    {
        // The thread hasn't been started yet, so nothing to do
        return;
    }

    catch(const TExceptException&)
    {
        // If we get a macro level exception, just log it
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
            m_pmeTarget->LogLastExcept();

        //
        //  If in sim mode, and exceptions aren't being reported at the point
        //  of throw, then our seeing it here means it wasn't handled and
        //  we need to report it because we don't let it propogate.
        //
        if (m_bSimMode
        &&  (m_pmeTarget->eExceptReport() == tCIDMacroEng::EExceptReps::NotHandled))
        {
            m_pmeTarget->ReportLastExcept();
        }
        return;
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        meTarget.Exception(errToCatch);
        return;
    }

    //
    //  And now call the method that normally is invoked on the separately
    //  started poll thread. In this case, we have to have it happen on
    //  this thread.
    //
    try
    {
        eDevPollThread(*TThread::pthrCaller(), 0);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}


// ---------------------------------------------------------------------------
//  TCQCGenDrvGlue: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TCQCGenDrvGlue::bGetCommResource(TThread&)
{
    tCIDLib::TBoolean bRet = kCIDLib::False;
    try
    {
        // Make sure we get the stack cleaned back up
        TMEngCallStackJan janStack(m_pmeTarget);

        // Forward to the macro class
        m_pmeTarget->PushPoolValue(tCIDMacroEng::EIntrinsics::Boolean, tCIDMacroEng::EConstTypes::NonConst);
        m_pmeTarget->meciPushMethodCall(m_pmeciDriver->c2Id(), m_c2MethId_GetCommRes);
        m_pmeciDriver->Invoke
        (
            *m_pmeTarget
            , *m_pmecvDriver
            , m_c2MethId_GetCommRes
            , tCIDMacroEng::EDispatch::Poly
        );

        // Get the boolean return value out before we clean up the stack
        bRet = m_pmeTarget->bStackValAt
        (
            m_pmeTarget->c4StackTop() - 2, kCIDLib::True
        );
    }

    catch(const TDbgExitException&)
    {
        // Just set the thread shutdown flag if not already
        TThread::pthrCaller()->ReqShutdownNoSync();
    }

    catch(const TExceptException&)
    {
        // If we get a macro level exception, just log it
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
            m_pmeTarget->LogLastExcept();

        //
        //  If in sim mode, and exceptions aren't being reported at the point
        //  of throw, then our seeing it here means it wasn't handled and
        //  we need to report it because we don't let it propogate.
        //
        if (m_bSimMode
        &&  (m_pmeTarget->eExceptReport() == tCIDMacroEng::EExceptReps::NotHandled))
        {
            m_pmeTarget->ReportLastExcept();
        }

        // Assume a failure
        bRet = kCIDLib::False;
    }

    catch(...)
    {
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
        {
            facCQCGenDrvS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kGDrvSErrs::errcDrv_SysExceptInCall
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::AppStatus
                , TString(L"GetCommResource")
            );
        }
        bRet = kCIDLib::False;
    }
    return bRet;
}


tCIDLib::TBoolean TCQCGenDrvGlue::bWaitConfig(TThread&)
{
    tCIDLib::TBoolean bRet = kCIDLib::False;
    try
    {
        // Make sure we get the stack cleaned back up
        TMEngCallStackJan janStack(m_pmeTarget);

        // Forward to the macro class
        m_pmeTarget->PushPoolValue(tCIDMacroEng::EIntrinsics::Boolean, tCIDMacroEng::EConstTypes::NonConst);
        m_pmeTarget->meciPushMethodCall(m_pmeciDriver->c2Id(), m_c2MethId_WaitConfig);
        m_pmeciDriver->Invoke
        (
            *m_pmeTarget
            , *m_pmecvDriver
            , m_c2MethId_WaitConfig
            , tCIDMacroEng::EDispatch::Poly
        );

        // Get the boolean return value out before we clean up the stack
        bRet = m_pmeTarget->bStackValAt
        (
            m_pmeTarget->c4StackTop() - 2, kCIDLib::True
        );
    }

    catch(const TDbgExitException&)
    {
        // Just set the thread shutdown flag if not already
        TThread::pthrCaller()->ReqShutdownNoSync();
    }

    catch(const TExceptException&)
    {
        // If we get a macro level exception, just log it
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
            m_pmeTarget->LogLastExcept();

        //
        //  If in sim mode, and exceptions aren't being reported at the point
        //  of throw, then our seeing it here means it wasn't handled and
        //  we need to report it because we don't let it propogate.
        //
        if (m_bSimMode
        &&  (m_pmeTarget->eExceptReport() == tCIDMacroEng::EExceptReps::NotHandled))
        {
            m_pmeTarget->ReportLastExcept();
        }

        // Assume a failure
        bRet = kCIDLib::False;
    }

    catch(...)
    {
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
        {
            facCQCGenDrvS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kGDrvSErrs::errcDrv_SysExceptInCall
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::AppStatus
                , TString(L"WaitConfig")
            );
        }
        bRet = kCIDLib::False;
    }
    return bRet;
}


tCQCKit::ECommResults
TCQCGenDrvGlue::eBoolFldValChanged( const   TString&
                                    , const tCIDLib::TCard4     c4FldId
                                    , const tCIDLib::TBoolean   bNewValue)
{
    // Dispatch it polymorphically
    tCQCKit::ECommResults eRes = tCQCKit::ECommResults::LostCommRes;
    try
    {
        // Make sure we get the stack cleaned back up
        TMEngCallStackJan janStack(m_pmeTarget);

        //
        //  We have to call the field changed callback on the macro class. We
        //  have to pass it the id of the fields that changed and the value.
        //  The return is a comm result enum.
        //
        m_pmeTarget->PushPoolValue(m_pmeciCommResEnum->c2Id(), tCIDMacroEng::EConstTypes::NonConst);
        m_pmeTarget->PushCard4(c4FldId, tCIDMacroEng::EConstTypes::Const);
        m_pmeTarget->PushBoolean(bNewValue, tCIDMacroEng::EConstTypes::Const);

        m_pmeTarget->meciPushMethodCall(m_pmeciDriver->c2Id(), m_c2MethId_FldChBool);
        m_pmeciDriver->Invoke
        (
            *m_pmeTarget
            , *m_pmecvDriver
            , m_c2MethId_FldChBool
            , tCIDMacroEng::EDispatch::Poly
        );

        eRes = tCQCKit::ECommResults
        (
            m_pmeciCommResEnum->c4MapValue
            (
                m_pmeTarget->mecvStackAtAs<TMEngEnumVal>(m_pmeTarget->c4StackTop() - 4)
            )
        );
    }

    catch(const TDbgExitException&)
    {
        // Just set the thread shutdown flag if not already
        TThread::pthrCaller()->ReqShutdownNoSync();
    }

    catch(const TExceptException&)
    {
        // If we get a macro level exception, just log it
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Low)
            m_pmeTarget->LogLastExcept();

        // Assume the worst
        eRes = tCQCKit::ECommResults::LostCommRes;

        //
        //  If in sim mode, and exceptions aren't being reported at the point
        //  of throw, then our seeing it here means it wasn't handled and
        //  we need to report it because we don't let it propogate.
        //
        if (m_bSimMode
        &&  (m_pmeTarget->eExceptReport() == tCIDMacroEng::EExceptReps::NotHandled))
        {
            m_pmeTarget->ReportLastExcept();
        }
    }

    catch(...)
    {
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Low)
        {
            facCQCGenDrvS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kGDrvSErrs::errcDrv_SysExceptInCall
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::AppStatus
                , TString(L"BoolFldChanged")
            );
        }
        eRes = tCQCKit::ECommResults::LostCommRes;
    }
    return eRes;
}


tCQCKit::ECommResults
TCQCGenDrvGlue::eCardFldValChanged( const   TString&
                                    , const tCIDLib::TCard4 c4FldId
                                    , const tCIDLib::TCard4 c4NewValue)
{
    // Dispatch it polymorphically
    tCQCKit::ECommResults eRes = tCQCKit::ECommResults::LostCommRes;
    try
    {
        // Make sure we get the stack cleaned back up
        TMEngCallStackJan janStack(m_pmeTarget);

        //
        //  We have to call the field changed callback on the macro class. We
        //  have to pass it the id of the fields that changed and the value.
        //  The return is a comm result enum.
        //
        m_pmeTarget->PushPoolValue(m_pmeciCommResEnum->c2Id(), tCIDMacroEng::EConstTypes::NonConst);
        m_pmeTarget->PushCard4(c4FldId, tCIDMacroEng::EConstTypes::Const);
        m_pmeTarget->PushCard4(c4NewValue, tCIDMacroEng::EConstTypes::Const);

        m_pmeTarget->meciPushMethodCall(m_pmeciDriver->c2Id(), m_c2MethId_FldChCard);
        m_pmeciDriver->Invoke
        (
            *m_pmeTarget
            , *m_pmecvDriver
            , m_c2MethId_FldChCard
            , tCIDMacroEng::EDispatch::Poly
        );

        eRes = tCQCKit::ECommResults
        (
            m_pmeciCommResEnum->c4MapValue
            (
                m_pmeTarget->mecvStackAtAs<TMEngEnumVal>(m_pmeTarget->c4StackTop() - 4)
            )
        );
    }

    catch(const TDbgExitException&)
    {
        // Just set the thread shutdown flag if not already
        TThread::pthrCaller()->ReqShutdownNoSync();
    }

    catch(const TExceptException&)
    {
        // If we get a macro level exception, just log it
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Low)
            m_pmeTarget->LogLastExcept();

        // Assume the worst
        eRes = tCQCKit::ECommResults::LostCommRes;

        //
        //  If in sim mode, and exceptions aren't being reported at the point
        //  of throw, then our seeing it here means it wasn't handled and
        //  we need to report it because we don't let it propogate.
        //
        if (m_bSimMode
        &&  (m_pmeTarget->eExceptReport() == tCIDMacroEng::EExceptReps::NotHandled))
        {
            m_pmeTarget->ReportLastExcept();
        }
    }

    catch(...)
    {
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Low)
        {
            facCQCGenDrvS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kGDrvSErrs::errcDrv_SysExceptInCall
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::AppStatus
                , TString(L"CardFldChanged")
            );
        }
        eRes = tCQCKit::ECommResults::LostCommRes;
    }
    return eRes;
}


tCQCKit::ECommResults TCQCGenDrvGlue::eConnectToDevice(TThread&)
{
    //
    //  Call the method on the driver that has to try to get the connection
    //  to the device up and going. It's return indicates whether it got
    //  connected or not, which we just return. This should be a polymorphic
    //  dispatch, since there might be multiple derived layers of the macro
    //  class we created.
    //
    tCQCKit::ECommResults eRes = tCQCKit::ECommResults::LostCommRes;
    try
    {
        // Make sure we get the stack cleaned back up
        TMEngCallStackJan janStack(m_pmeTarget);

        m_pmeTarget->PushPoolValue
        (
            m_pmeciCommResEnum->c2Id(), tCIDMacroEng::EConstTypes::NonConst
        );
        m_pmeTarget->meciPushMethodCall(m_pmeciDriver->c2Id(), m_c2MethId_Conn);
        m_pmeciDriver->Invoke
        (
            *m_pmeTarget
            , *m_pmecvDriver
            , m_c2MethId_Conn
            , tCIDMacroEng::EDispatch::Poly
        );

        // Get the boolean return value out
        eRes = tCQCKit::ECommResults
        (
            m_pmeciCommResEnum->c4MapValue
            (
                m_pmeTarget->mecvStackAtAs<TMEngEnumVal>(m_pmeTarget->c4StackTop() - 2)
            )
        );
    }

    catch(const TDbgExitException&)
    {
        // Just set the thread shutdown flag if not already
        TThread::pthrCaller()->ReqShutdownNoSync();
    }

    catch(const TExceptException&)
    {
        // If we get a macro level exception, just log it
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
            m_pmeTarget->LogLastExcept();

        // Assume the worst
        eRes = tCQCKit::ECommResults::LostCommRes;

        //
        //  If in sim mode, and exceptions aren't being reported at the point
        //  of throw, then our seeing it here means it wasn't handled and
        //  we need to report it because we don't let it propogate.
        //
        if (m_bSimMode
        &&  (m_pmeTarget->eExceptReport() == tCIDMacroEng::EExceptReps::NotHandled))
        {
            m_pmeTarget->ReportLastExcept();
        }
    }

    catch(TError& errToCatch)
    {
        // Assume the worst
        eRes = tCQCKit::ECommResults::LostCommRes;

        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::High)
        {
            if (!errToCatch.bLogged())
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }

            facCQCGenDrvS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kGDrvSErrs::errcDrv_ExceptInCall
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::AppStatus
                , TString(L"Connect")
            );
        }
    }

    catch(...)
    {
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
        {
            facCQCGenDrvS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kGDrvSErrs::errcDrv_SysExceptInCall
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::AppStatus
                , TString(L"Connect")
            );
        }
        eRes = tCQCKit::ECommResults::LostCommRes;
    }
    return eRes;
}


tCQCKit::ECommResults
TCQCGenDrvGlue::eFloatFldValChanged(const   TString&
                                    , const tCIDLib::TCard4     c4FldId
                                    , const tCIDLib::TFloat8    f8NewValue)
{
    tCQCKit::ECommResults eRes = tCQCKit::ECommResults::LostCommRes;
    try
    {
        // Make sure we get the stack cleaned back up
        TMEngCallStackJan janStack(m_pmeTarget);

        //
        //  We have to call the field changed callback on the macro class. We
        //  have to pass it the id of the fields that changed and the value.
        //  The return is a comm result enum.
        //
        m_pmeTarget->PushPoolValue(m_pmeciCommResEnum->c2Id(), tCIDMacroEng::EConstTypes::NonConst);
        m_pmeTarget->PushCard4(c4FldId, tCIDMacroEng::EConstTypes::Const);
        m_pmeTarget->PushFloat8(f8NewValue, tCIDMacroEng::EConstTypes::Const);

        m_pmeTarget->meciPushMethodCall(m_pmeciDriver->c2Id(), m_c2MethId_FldChFloat);
        m_pmeciDriver->Invoke
        (
            *m_pmeTarget
            , *m_pmecvDriver
            , m_c2MethId_FldChFloat
            , tCIDMacroEng::EDispatch::Poly
        );

        eRes = tCQCKit::ECommResults
        (
            m_pmeciCommResEnum->c4MapValue
            (
                m_pmeTarget->mecvStackAtAs<TMEngEnumVal>(m_pmeTarget->c4StackTop() - 4)
            )
        );
    }

    catch(const TDbgExitException&)
    {
        // Just set the thread shutdown flag if not already
        TThread::pthrCaller()->ReqShutdownNoSync();
    }

    catch(const TExceptException&)
    {
        // If we get a macro level exception, just log it
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Low)
            m_pmeTarget->LogLastExcept();

        // Assume the worst
        eRes = tCQCKit::ECommResults::LostCommRes;

        //
        //  If in sim mode, and exceptions aren't being reported at the point
        //  of throw, then our seeing it here means it wasn't handled and
        //  we need to report it because we don't let it propogate.
        //
        if (m_bSimMode
        &&  (m_pmeTarget->eExceptReport() == tCIDMacroEng::EExceptReps::NotHandled))
        {
            m_pmeTarget->ReportLastExcept();
        }
    }

    catch(...)
    {
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Low)
        {
            facCQCGenDrvS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kGDrvSErrs::errcDrv_SysExceptInCall
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::AppStatus
                , TString(L"FloatFldChanged")
            );
        }
        eRes = tCQCKit::ECommResults::LostCommRes;
    }
    return eRes;
}


//
//  We have to get all the macro engine stuff set up and then invoke the
//  CML level initialization method. Note that, we can get called more
//  than once if the initialization should fail for some reason, such as
//  the data server not being there or the CML not compiling correctly.
//  So we have to be careful not to re-create things that have already
//  been created.
//
tCQCKit::EDrvInitRes TCQCGenDrvGlue::eInitializeImpl()
{
    //
    //  If not in sim mode, then we have to create the macro engine and
    //  load the driver class and build it.
    //
    if (!m_bSimMode)
    {
        // First of all, create the macro engine if we need to, else reset it
        if (!m_pmeTarget)
        {
            m_pmeTarget = new TCIDMacroEngine;
        }
         else
        {
            //
            //  Delete any previous value object before we reset the engine.
            //  The engine doesn't own this top level value object, but the
            //  object references the engine in various ways, so we need to
            //  delete it first while the previous contents of the engine
            //  are still there.
            //
            if (m_pmecvDriver)
            {
                delete m_pmecvDriver;
                m_pmecvDriver = nullptr;
            }
            m_pmeTarget->Reset();
        }

        // Set any dynamic type ref from the manifest on the engine
        m_pmeTarget->strSpecialDynRef(cqcdcThis().strDynRef());

        // Set a standard CQC error handler, which logs to the log server
        if (!m_pmeehLogger)
            m_pmeehLogger = new TCQCMEngErrHandler;
        m_pmeTarget->SetErrHandler(m_pmeehLogger);

        //
        //  Create a parser and install a standard CQC class manager on it,
        //  which reads/writes the CQC macro repository.
        //
        TCQCMEngClassMgr    mecmParse(cuctxDrv().sectUser());
        TCQCPrsErrHandler   meehParser;
        TMacroEngParser     meprsDriver;
        TMEngClassInfo*     pmeciNew;
        if (!meprsDriver.bParse(cqcdcThis().strServerLibName()
                                , pmeciNew
                                , m_pmeTarget
                                , &meehParser
                                , &mecmParse))
        {
            if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
            {
                facCQCGenDrvS().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kGDrvSErrs::errcDrv_CompileFailed
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::AppStatus
                    , strMoniker()
                );
            }
            return tCQCKit::EDrvInitRes::Failed;
        }

        //
        //  Store the new driver. Note that it belongs to the macro engine
        //  so we don't have to clean it up if we fail during init. It'll
        //  get dumped when we reset the engine on the next round.
        //
        m_pmeciDriver = pmeciNew;

        //
        //  Install a basic directory based file resolver, setting the base
        //  directory to the correct place in the local CQCData directory.
        //
        if (!m_pmefrData)
            m_pmefrData = new TMEngFixedBaseFileResolver(facCQCKit().strMacroRootPath());
        m_pmeTarget->SetFileResolver(m_pmefrData);

        //
        //  Looks like it worked, so create the value object, initialize it,
        //  and add it to the engine, and invoke the default ctor.
        //
        //  Note that, in the CQCServer environment, this main driver object
        //  will last as long as the driver is loaded and so it is not
        //  destroyed until our destructor is called. In the simulation
        //  scenario, the driver is actually managed by the IDE and we just
        //  get a pointer to it.
        //
        m_pmecvDriver = m_pmeciDriver->pmecvMakeStorage
        (
            L"$Main$", *m_pmeTarget, tCIDMacroEng::EConstTypes::NonConst
        );

        //
        //  Tell the object about us by setting it's glue member. We have
        //  to look it up and cast it to it's actual type.
        //
        TCQCDriverGlueVal* pmecvGlue = static_cast<TCQCDriverGlueVal*>
        (
            m_pmecvDriver->pmecvFind(L"m_GlueInfo")
        );
        pmecvGlue->Initialize(this);

        // Call the driver's default constructor
        if (!m_pmeTarget->bInvokeDefCtor(*m_pmecvDriver, 0))
            return tCQCKit::EDrvInitRes::Failed;
    }

    // Pre-lookup the ids of the methods we'll call, for efficiency
    m_c2MethId_ClientCmd = m_pmeciDriver->c2FindMethod(L"ClientCmd");
    m_c2MethId_Conn = m_pmeciDriver->c2FindMethod(L"Connect");
    m_c2MethId_FldChBool = m_pmeciDriver->c2FindMethod(L"BoolFldChanged");
    m_c2MethId_FldChCard = m_pmeciDriver->c2FindMethod(L"CardFldChanged");
    m_c2MethId_FldChFloat = m_pmeciDriver->c2FindMethod(L"FloatFldChanged");
    m_c2MethId_FldChInt = m_pmeciDriver->c2FindMethod(L"IntFldChanged");
    m_c2MethId_FldChString = m_pmeciDriver->c2FindMethod(L"StringFldChanged");
    m_c2MethId_FldChStrList = m_pmeciDriver->c2FindMethod(L"StrListFldChanged");
    m_c2MethId_FldChTime = m_pmeciDriver->c2FindMethod(L"TimeFldChanged");
    m_c2MethId_FreeCommRes = m_pmeciDriver->c2FindMethod(L"FreeCommResource");
    m_c2MethId_GetCommRes = m_pmeciDriver->c2FindMethod(L"GetCommResource");
    m_c2MethId_Poll = m_pmeciDriver->c2FindMethod(L"Poll");
    m_c2MethId_QueryBoolVal = m_pmeciDriver->c2FindMethod(L"QueryBoolVal");
    m_c2MethId_QueryBufVal = m_pmeciDriver->c2FindMethod(L"QueryBufVal");
    m_c2MethId_QueryCardVal = m_pmeciDriver->c2FindMethod(L"QueryCardVal");
    m_c2MethId_QueryIntVal = m_pmeciDriver->c2FindMethod(L"QueryIntVal");
    m_c2MethId_QueryTextVal = m_pmeciDriver->c2FindMethod(L"QueryTextVal");
    m_c2MethId_WaitConfig = m_pmeciDriver->c2FindMethod(L"WaitConfig");

    // And pre-look up some types
    m_pmeciCommResEnum = m_pmeTarget->pmeciFindAs<TMEngEnumInfo>
    (
        TCQCDriverInfo::strCommResPath()
    );

    m_pmeciMemBuf = m_pmeTarget->pmeciFindAs<TMEngMemBufInfo>
    (
        TMEngMemBufInfo::strPath()
    );

    m_pmeciVerbLevelEnum = m_pmeTarget->pmeciFindAs<TMEngEnumInfo>
    (
        TCQCDriverInfo::strVerbLevelPath()
    );



    // Make sure we get the stack cleaned back up
    TMEngCallStackJan janStack(m_pmeTarget);

    //
    //  Let's call the driver's initialization method. He must set the field
    //  list during this, and can do other init if required. This needs to be
    //  a polymorphic dispatch, since there could be multiple derived layers
    //  in the driver class we created.
    //
    //  We have to call a different one according to what kind of resource
    //  connection they use.
    //
    tCQCKit::EDrvInitRes eRes;
    try
    {
        //
        //  First, look up the driver init result type id so that we can push
        //  a return value holder.
        //
        m_pmeTarget->PushPoolValue
        (
            m_pmeTarget->c2FindClassId(TCQCDriverInfo::strInitResPath())
            , tCIDMacroEng::EConstTypes::NonConst
        );

        //
        //  Push the appropriate configuration parameters. For the ones that
        //  aren't intrinsics we have to create the value objects ourself and
        //  push them. Mark them a locals, so that they will get deleted for
        //  us when the CML stack is popped.
        //
        //  Note taht we set the parm count one higher since we will push
        //  another parm at the end for all of them.
        //
        const TCQCDriverObjCfg& cqcdcCur = cqcdcThis();
        tCIDLib::TCard2 c2InitMethId = 0;
        tCIDLib::TCard4 c4ParmCount = 0;
        if (cqcdcCur.conncfgReal().clsIsA() == TCQCSerialConnCfg::clsThis())
        {
            c2InitMethId = m_pmeciDriver->c2FindMethod(L"InitializeSerial");
            const TCQCSerialConnCfg& conncfgSer
                = static_cast<const TCQCSerialConnCfg&>(cqcdcCur.conncfgReal());

            // Push the port path
            m_pmeTarget->PushString(conncfgSer.strPortPath(), tCIDMacroEng::EConstTypes::Const);

            // Push the port configuration data
            TMEngCommCfgVal* pmecvPush = new TMEngCommCfgVal
            (
                L"CfgData"
                , m_pmeTarget->c2FindClassId(TMEngCommCfgInfo::strPath())
                , tCIDMacroEng::EConstTypes::Const
            );
            pmecvPush->cpcfgValue(conncfgSer.cpcfgSerial());
            m_pmeTarget->PushValue(pmecvPush, tCIDMacroEng::EStackItems::Local);

            c4ParmCount = 3;
        }
         else if (cqcdcCur.conncfgReal().clsIsA() == TCQCIPConnCfg::clsThis())
        {
            c2InitMethId = m_pmeciDriver->c2FindMethod(L"InitializeSocket");
            const TCQCIPConnCfg& conncfgIP
                = static_cast<const TCQCIPConnCfg&>(cqcdcCur.conncfgReal());

            // Push the target IP end point
            TMEngIPEPVal* pmecvPush = new TMEngIPEPVal
            (
                L"EndPoint"
                , m_pmeTarget->c2FindClassId(TMEngIPEPInfo::strPath())
                , tCIDMacroEng::EConstTypes::Const
            );
            pmecvPush->ipepValue(conncfgIP.ipepConn());
            m_pmeTarget->PushValue(pmecvPush, tCIDMacroEng::EStackItems::Local);

            c4ParmCount = 2;
        }
         else if (cqcdcCur.conncfgReal().clsIsA() == TCQCOtherConnCfg::clsThis())
        {
            c2InitMethId = m_pmeciDriver->c2FindMethod(L"InitializeOther");
            const TCQCOtherConnCfg& conncfgOther
                = static_cast<const TCQCOtherConnCfg&>(cqcdcCur.conncfgReal());

            // We just have the generic connection info string to push
            m_pmeTarget->PushString
            (
                conncfgOther.strConnInfo(), tCIDMacroEng::EConstTypes::Const
            );
            c4ParmCount = 2;
        }
         else
        {
            facCQCGenDrvS().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kGDrvSErrs::errcConn_UnknownType
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Unknown
                , cqcdcCur.conncfgReal().clsIsA()
            );
        }

        //
        //  Set up the list of dynamic and fixed values. We pass in a named
        //  value map. The CML version of this class just wraps the C++ vresion
        //  so we can just use the value constructor that will make a copy
        //  of our existing one, and then append the fixed item values to it.
        //
        const tCIDLib::TCard2 c2NVMId
        (
            m_pmeTarget->c2FindClassId(TMEngNamedValMapInfo::strPath())
        );
        TMEngNamedValMapVal* pmecvPrompts = new TMEngNamedValMapVal
        (
            L"PromptVals", c2NVMId, tCIDMacroEng::EConstTypes::Const, cqcdcCur.nvmFixed()
        );
        pmecvPrompts->AppendFrom(cqcdcCur.nvmPromptVals());
        m_pmeTarget->PushValue(pmecvPrompts, tCIDMacroEng::EStackItems::Local);

        // Now push the method call item, and invoke the method
        m_pmeTarget->meciPushMethodCall(m_pmeciDriver->c2Id(), c2InitMethId);
        m_pmeciDriver->Invoke
        (
            *m_pmeTarget, *m_pmecvDriver, c2InitMethId, tCIDMacroEng::EDispatch::Poly
        );

        // Get the return out before we clean the stack
        TMEngEnumVal& mecvRes = m_pmeTarget->mecvStackAtAs<TMEngEnumVal>
        (
            m_pmeTarget->c4StackTop() - (2 + c4ParmCount)
        );
        eRes = tCQCKit::EDrvInitRes(mecvRes.c4Ordinal());
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
            LogError(errToCatch, tCQCKit::EVerboseLvls::Medium);

        if (m_bSimMode)
            throw;
        eRes = tCQCKit::EDrvInitRes::Failed;
    }

    catch(const TExceptException&)
    {
        // If we get a macro level exception, just log it
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
            m_pmeTarget->LogLastExcept();

        if (m_bSimMode)
            throw;
        eRes = tCQCKit::EDrvInitRes::Failed;
    }

    catch(...)
    {
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
        {
            facCQCGenDrvS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kGDrvSErrs::errcDrv_SysExceptInCall
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::AppStatus
                , TString(L"InitializeXXX")
            );
        }
        eRes = tCQCKit::EDrvInitRes::Failed;
    }
    return eRes;
}


tCQCKit::ECommResults
TCQCGenDrvGlue::eIntFldValChanged(  const   TString&
                                    , const tCIDLib::TCard4 c4FldId
                                    , const tCIDLib::TInt4  i4NewValue)
{
    tCQCKit::ECommResults eRes = tCQCKit::ECommResults::LostCommRes;
    try
    {
        // Make sure we get the stack cleaned back up
        TMEngCallStackJan janStack(m_pmeTarget);

        //
        //  We have to call the field changed callback on the macro class. We
        //  have to pass it the id of the fields that changed and the value.
        //  The return is a comm result enum.
        //
        m_pmeTarget->PushPoolValue(m_pmeciCommResEnum->c2Id(), tCIDMacroEng::EConstTypes::NonConst);
        m_pmeTarget->PushCard4(c4FldId, tCIDMacroEng::EConstTypes::Const);
        m_pmeTarget->PushInt4(i4NewValue, tCIDMacroEng::EConstTypes::Const);

        m_pmeTarget->meciPushMethodCall(m_pmeciDriver->c2Id(), m_c2MethId_FldChInt);
        m_pmeciDriver->Invoke
        (
            *m_pmeTarget
            , *m_pmecvDriver
            , m_c2MethId_FldChInt
            , tCIDMacroEng::EDispatch::Poly
        );

        eRes = tCQCKit::ECommResults
        (
            m_pmeciCommResEnum->c4MapValue
            (
                m_pmeTarget->mecvStackAtAs<TMEngEnumVal>(m_pmeTarget->c4StackTop() - 4)
            )
        );
    }

    catch(const TDbgExitException&)
    {
        // Just set the thread shutdown flag if not already
        TThread::pthrCaller()->ReqShutdownNoSync();
    }

    catch(const TExceptException&)
    {
        // If we get a macro level exception, just log it
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Low)
            m_pmeTarget->LogLastExcept();

        // Assume the worst
        eRes = tCQCKit::ECommResults::LostCommRes;

        //
        //  If in sim mode, and exceptions aren't being reported at the point
        //  of throw, then our seeing it here means it wasn't handled and
        //  we need to report it because we don't let it propogate.
        //
        if (m_bSimMode
        &&  (m_pmeTarget->eExceptReport() == tCIDMacroEng::EExceptReps::NotHandled))
        {
            m_pmeTarget->ReportLastExcept();
        }
    }

    catch(...)
    {
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Low)
        {
            facCQCGenDrvS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kGDrvSErrs::errcDrv_SysExceptInCall
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::AppStatus
                , TString(L"IntFldChanged")
            );
        }
        eRes = tCQCKit::ECommResults::LostCommRes;
    }
    return eRes;
}


tCQCKit::ECommResults TCQCGenDrvGlue::ePollDevice(TThread&)
{
    //
    //  Call the method on the driver that has to try to get the connection
    //  to the device up and going. It's return indicates whether it got
    //  connected or not, which we just return. This needs to be a polymorphic
    //  dispatch, since there could be multiple derived layers in the driver
    //  class we created.
    //
    tCQCKit::ECommResults eRes = tCQCKit::ECommResults::LostCommRes;
    try
    {
        // Make sure we get the stack cleaned back up
        TMEngCallStackJan janStack(m_pmeTarget);

        m_pmeTarget->PushPoolValue(m_pmeciCommResEnum->c2Id(), tCIDMacroEng::EConstTypes::NonConst);
        m_pmeTarget->meciPushMethodCall(m_pmeciDriver->c2Id(), m_c2MethId_Poll);
        m_pmeciDriver->Invoke
        (
            *m_pmeTarget
            , *m_pmecvDriver
            , m_c2MethId_Poll
            , tCIDMacroEng::EDispatch::Poly
        );

        // Get the boolean return value out
        eRes = tCQCKit::ECommResults
        (
            m_pmeciCommResEnum->c4MapValue
            (
                m_pmeTarget->mecvStackAtAs<TMEngEnumVal>(m_pmeTarget->c4StackTop() - 2)
            )
        );
    }

    catch(const TDbgExitException&)
    {
        // Just set the thread shutdown flag if not already
        TThread::pthrCaller()->ReqShutdownNoSync();
    }

    catch(const TExceptException&)
    {
        // If we get a macro level exception, just log it
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::High)
            m_pmeTarget->LogLastExcept();

        // Assume the worst
        eRes = tCQCKit::ECommResults::LostCommRes;

        //
        //  If in sim mode, and exceptions aren't being reported at the point
        //  of throw, then our seeing it here means it wasn't handled and
        //  we need to report it because we don't let it propogate.
        //
        if (m_bSimMode
        &&  (m_pmeTarget->eExceptReport() == tCIDMacroEng::EExceptReps::NotHandled))
        {
            m_pmeTarget->ReportLastExcept();
        }
    }

    catch(TError& errToCatch)
    {
        // Assume the worst
        eRes = tCQCKit::ECommResults::LostCommRes;

        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        LogError(errToCatch, tCQCKit::EVerboseLvls::High);
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::High)
        {
            facCQCGenDrvS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kGDrvSErrs::errcDrv_ExceptInCall
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::AppStatus
                , TString(L"Poll")
            );
        }
    }

    catch(...)
    {
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::High)
        {
            facCQCGenDrvS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kGDrvSErrs::errcDrv_SysExceptInCall
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::AppStatus
                , TString(L"Poll")
            );
        }
        eRes = tCQCKit::ECommResults::LostCommRes;
    }
    return eRes;
}


tCQCKit::ECommResults
TCQCGenDrvGlue::eStringFldValChanged(const  TString&
                                    , const tCIDLib::TCard4 c4FldId
                                    , const TString&        strNewValue)
{
    tCQCKit::ECommResults eRes = tCQCKit::ECommResults::LostCommRes;
    try
    {
        // Make sure we get the stack cleaned back up
        TMEngCallStackJan janStack(m_pmeTarget);

        //
        //  We have to call the field changed callback on the macro class. We
        //  have to pass it the id of the fields that changed and the value.
        //  The return is a comm result enum.
        //
        m_pmeTarget->PushPoolValue(m_pmeciCommResEnum->c2Id(), tCIDMacroEng::EConstTypes::NonConst);
        m_pmeTarget->PushCard4(c4FldId, tCIDMacroEng::EConstTypes::Const);
        m_pmeTarget->PushString(strNewValue, tCIDMacroEng::EConstTypes::Const);

        m_pmeTarget->meciPushMethodCall(m_pmeciDriver->c2Id(), m_c2MethId_FldChString);
        m_pmeciDriver->Invoke
        (
            *m_pmeTarget
            , *m_pmecvDriver
            , m_c2MethId_FldChString
            , tCIDMacroEng::EDispatch::Poly
        );

        eRes = tCQCKit::ECommResults
        (
            m_pmeciCommResEnum->c4MapValue
            (
                m_pmeTarget->mecvStackAtAs<TMEngEnumVal>(m_pmeTarget->c4StackTop() - 4)
            )
        );
    }

    catch(const TDbgExitException&)
    {
        // Just set the thread shutdown flag if not already
        TThread::pthrCaller()->ReqShutdownNoSync();
    }

    catch(const TExceptException&)
    {
        // If we get a macro level exception, just log it
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Low)
            m_pmeTarget->LogLastExcept();

        // Assume the worst
        eRes = tCQCKit::ECommResults::LostCommRes;

        //
        //  If in sim mode, and exceptions aren't being reported at the point
        //  of throw, then our seeing it here means it wasn't handled and
        //  we need to report it because we don't let it propogate.
        //
        if (m_bSimMode
        &&  (m_pmeTarget->eExceptReport() == tCIDMacroEng::EExceptReps::NotHandled))
        {
            m_pmeTarget->ReportLastExcept();
        }
    }

    catch(...)
    {
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Low)
        {
            facCQCGenDrvS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kGDrvSErrs::errcDrv_SysExceptInCall
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::AppStatus
                , TString(L"StringFldChanged")
            );
        }
        eRes = tCQCKit::ECommResults::LostCommRes;
    }
    return eRes;
}


tCQCKit::ECommResults
TCQCGenDrvGlue::eSListFldValChanged(const   TString&
                                    , const tCIDLib::TCard4     c4FldId
                                    , const TVector<TString>&   colNewValue)
{
    tCQCKit::ECommResults eRes = tCQCKit::ECommResults::LostCommRes;
    try
    {
        // Make sure we get the stack cleaned back up
        TMEngCallStackJan janStack(m_pmeTarget);

        //
        //  We have to call the field changed callback on the macro class. We
        //  have to pass it the id of the fields that changed and the value.
        //  The return is a comm result enum.
        //
        m_pmeTarget->PushPoolValue(m_pmeciCommResEnum->c2Id(), tCIDMacroEng::EConstTypes::NonConst);
        m_pmeTarget->PushCard4(c4FldId, tCIDMacroEng::EConstTypes::Const);
        m_pmeTarget->PushStringList(colNewValue, tCIDMacroEng::EConstTypes::Const);

        m_pmeTarget->meciPushMethodCall(m_pmeciDriver->c2Id(), m_c2MethId_FldChStrList);
        m_pmeciDriver->Invoke
        (
            *m_pmeTarget
            , *m_pmecvDriver
            , m_c2MethId_FldChStrList
            , tCIDMacroEng::EDispatch::Poly
        );

        eRes = tCQCKit::ECommResults
        (
            m_pmeciCommResEnum->c4MapValue
            (
                m_pmeTarget->mecvStackAtAs<TMEngEnumVal>(m_pmeTarget->c4StackTop() - 4)
            )
        );
    }

    catch(const TDbgExitException&)
    {
        // Just set the thread shutdown flag if not already
        TThread::pthrCaller()->ReqShutdownNoSync();
    }

    catch(const TExceptException&)
    {
        // If we get a macro level exception, just log it
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
            m_pmeTarget->LogLastExcept();

        // Assume the worst
        eRes = tCQCKit::ECommResults::LostCommRes;

        //
        //  If in sim mode, and exceptions aren't being reported at the point
        //  of throw, then our seeing it here means it wasn't handled and
        //  we need to report it because we don't let it propogate.
        //
        if (m_bSimMode
        &&  (m_pmeTarget->eExceptReport() == tCIDMacroEng::EExceptReps::NotHandled))
        {
            m_pmeTarget->ReportLastExcept();
        }
    }

    catch(...)
    {
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Low)
        {
            facCQCGenDrvS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kGDrvSErrs::errcDrv_SysExceptInCall
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::AppStatus
                , TString(L"StrListFldChanged")
            );
        }
        eRes = tCQCKit::ECommResults::LostCommRes;
    }
    return eRes;
}


tCQCKit::ECommResults
TCQCGenDrvGlue::eTimeFldValChanged( const   TString&
                                    , const tCIDLib::TCard4     c4FldId
                                    , const tCIDLib::TCard8&    c8NewValue)
{
    tCQCKit::ECommResults eRes = tCQCKit::ECommResults::LostCommRes;
    try
    {
        // Make sure we get the stack cleaned back up
        TMEngCallStackJan janStack(m_pmeTarget);

        //
        //  We have to call the field changed callback on the macro class. We
        //  have to pass it the id of the fields that changed and the value.
        //  The return is a comm result enum.
        //
        m_pmeTarget->PushPoolValue(m_pmeciCommResEnum->c2Id(), tCIDMacroEng::EConstTypes::NonConst);
        m_pmeTarget->PushCard4(c4FldId, tCIDMacroEng::EConstTypes::Const);
        m_pmeTarget->PushCard8(c8NewValue, tCIDMacroEng::EConstTypes::Const);

        m_pmeTarget->meciPushMethodCall(m_pmeciDriver->c2Id(), m_c2MethId_FldChTime);
        m_pmeciDriver->Invoke
        (
            *m_pmeTarget
            , *m_pmecvDriver
            , m_c2MethId_FldChTime
            , tCIDMacroEng::EDispatch::Poly
        );

        eRes = tCQCKit::ECommResults
        (
            m_pmeciCommResEnum->c4MapValue
            (
                m_pmeTarget->mecvStackAtAs<TMEngEnumVal>(m_pmeTarget->c4StackTop() - 4)
            )
        );
    }

    catch(const TDbgExitException&)
    {
        // Just set the thread shutdown flag if not already
        TThread::pthrCaller()->ReqShutdownNoSync();
    }

    catch(const TExceptException&)
    {
        // If we get a macro level exception, just log it
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
            m_pmeTarget->LogLastExcept();

        // Assume the worst
        eRes = tCQCKit::ECommResults::LostCommRes;

        //
        //  If in sim mode, and exceptions aren't being reported at the point
        //  of throw, then our seeing it here means it wasn't handled and
        //  we need to report it because we don't let it propogate.
        //
        if (m_bSimMode
        &&  (m_pmeTarget->eExceptReport() == tCIDMacroEng::EExceptReps::NotHandled))
        {
            m_pmeTarget->ReportLastExcept();
        }
    }

    catch(...)
    {
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Low)
        {
            facCQCGenDrvS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kGDrvSErrs::errcDrv_SysExceptInCall
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::AppStatus
                , TString(L"TimeFldChanged")
            );
        }
        eRes = tCQCKit::ECommResults::LostCommRes;
    }
    return eRes;
}


//
//  The base driver logic will call this once a second when it's waiting to
//  do another operation. If we are in simulator mode, we just call a method
//  on the macro engine that let's the IDE break or stop us.
//
tCIDLib::TVoid TCQCGenDrvGlue::Idle()
{
    if (m_bSimMode)
    {
        // Let the IDE break/stop us if he wants
        m_pmeTarget->CheckIDEReq();
    }
}


tCIDLib::TVoid TCQCGenDrvGlue::ReleaseCommResource()
{
    try
    {
        // Make sure we get the stack cleaned back up
        TMEngCallStackJan janStack(m_pmeTarget);

        // Forward to the macro class
        m_pmeTarget->PushPoolValue(tCIDMacroEng::EIntrinsics::Boolean, tCIDMacroEng::EConstTypes::NonConst);
        m_pmeTarget->meciPushMethodCall(m_pmeciDriver->c2Id(), m_c2MethId_FreeCommRes);
        m_pmeciDriver->Invoke
        (
            *m_pmeTarget
            , *m_pmecvDriver
            , m_c2MethId_FreeCommRes
            , tCIDMacroEng::EDispatch::Poly
        );
    }

    catch(const TDbgExitException&)
    {
        // Just set the thread shutdown flag if not already
        TThread::pthrCaller()->ReqShutdownNoSync();
    }

    catch(const TExceptException&)
    {
        // If we get a macro level exception, just log it
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
            m_pmeTarget->LogLastExcept();

        //
        //  If in sim mode, and exceptions aren't being reported at the point
        //  of throw, then our seeing it here means it wasn't handled and
        //  we need to report it because we don't let it propogate.
        //
        if (m_bSimMode
        &&  (m_pmeTarget->eExceptReport() == tCIDMacroEng::EExceptReps::NotHandled))
        {
            m_pmeTarget->ReportLastExcept();
        }
    }

    catch(...)
    {
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Low)
        {
            facCQCGenDrvS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kGDrvSErrs::errcDrv_SysExceptInCall
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::AppStatus
                , TString(L"FreeCommResource")
            );
        }
    }
}


tCIDLib::TVoid TCQCGenDrvGlue::TerminateImpl()
{
    try
    {
        // Make sure we get the stack cleaned back up
        TMEngCallStackJan janStack(m_pmeTarget);

        // We have to call the terminate callback on the macro class.
        m_pmeTarget->PushPoolValue(tCIDMacroEng::EIntrinsics::Void, tCIDMacroEng::EConstTypes::NonConst);

        tCIDLib::TCard2 c2MethId = m_pmeciDriver->c2FindMethod(L"Terminate");
        m_pmeTarget->meciPushMethodCall(m_pmeciDriver->c2Id(), c2MethId);
        m_pmeciDriver->Invoke
        (
            *m_pmeTarget
            , *m_pmecvDriver
            , c2MethId
            , tCIDMacroEng::EDispatch::Poly
        );
    }

    catch(const TExceptException&)
    {
        // If we get a macro level exception, just log it
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Low)
            m_pmeTarget->LogLastExcept();

        //
        //  If in sim mode, and exceptions aren't being reported at the point
        //  of throw, then our seeing it here means it wasn't handled and
        //  we need to report it because we don't let it propogate.
        //
        if (m_bSimMode
        &&  (m_pmeTarget->eExceptReport() == tCIDMacroEng::EExceptReps::NotHandled))
        {
            m_pmeTarget->ReportLastExcept();
        }
    }

    catch(...)
    {
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Low)
        {
            facCQCGenDrvS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kGDrvSErrs::errcDrv_SysExceptInCall
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::AppStatus
                , TString(L"Terminate")
            );
        }
    }
}


// Let the CML driver know about changes in verbosity level
tCIDLib::TVoid
TCQCGenDrvGlue::VerboseModeChanged(const tCQCKit::EVerboseLvls eNewState)
{
    //
    //  The initial call will be before the driver is initialized, when running
    //  in the IDE. So we don't want to do anything.
    //
    if (!m_pmeciVerbLevelEnum)
        return;

    try
    {
        // Make sure we get the stack cleaned back up
        TMEngCallStackJan janStack(m_pmeTarget);

        // No return value so push a void
        m_pmeTarget->PushPoolValue(tCIDMacroEng::EIntrinsics::Void, tCIDMacroEng::EConstTypes::NonConst);

        //
        //  Push an enum for the incoming new state. Our CML verbosity level
        //  enum maps directly to the C++ one, so we can just pass the incoming
        //  value as the ordinal.
        //
        m_pmeTarget->PushEnum
        (
            m_pmeciVerbLevelEnum->c2Id()
            , tCIDLib::TCard2(eNewState)
            , tCIDMacroEng::EConstTypes::Const
        );

        tCIDLib::TCard2 c2MethId = m_pmeciDriver->c2FindMethod(L"VerbosityChanged");
        m_pmeTarget->meciPushMethodCall(m_pmeciDriver->c2Id(), c2MethId);
        m_pmeciDriver->Invoke
        (
            *m_pmeTarget, *m_pmecvDriver, c2MethId, tCIDMacroEng::EDispatch::Poly
        );
    }

    catch(const TDbgExitException&)
    {
        // Just set the thread shutdown flag if not already
        TThread::pthrCaller()->ReqShutdownNoSync();
    }

    catch(const TExceptException&)
    {
        // If we get a macro level exception, just log it
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Low)
            m_pmeTarget->LogLastExcept();

        //
        //  If in sim mode, and exceptions aren't being reported at the point
        //  of throw, then our seeing it here means it wasn't handled and
        //  we need to report it because we don't let it propogate.
        //
        if (m_bSimMode
        &&  (m_pmeTarget->eExceptReport() == tCIDMacroEng::EExceptReps::NotHandled))
        {
            m_pmeTarget->ReportLastExcept();
        }
    }

    catch(...)
    {
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Low)
        {
            facCQCGenDrvS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kGDrvSErrs::errcDrv_SysExceptInCall
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::AppStatus
                , TString(L"VerbosityChanged")
            );
        }
    }

}


