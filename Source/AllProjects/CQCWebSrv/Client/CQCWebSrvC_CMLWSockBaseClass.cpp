//
// FILE NAME: CQCKit_CMLWSockBaseClass.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/17/2014
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
//  This file implements the info and value classes to create a CML level class that
//  is used as the base class for all Websocket handlers in the CQC web server.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include "CQCWebSrvC_.hpp"
#include "CIDMacroEng_JSONClasses_.hpp"


// ---------------------------------------------------------------------------
//  Magic RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TCMLWSockBaseInfo,TMEngClassInfo)



// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace CQCWebSrvC_CMLWSockBase
{
    // -----------------------------------------------------------------------
    //  The names for the types that we support here. Each derivative has to
    //  be able to return strings that contain its name and full name.
    // -----------------------------------------------------------------------
    static const TString   strCMLWSockBase(L"CMLWSockBase");
    static const TString   strCMLWSockClassPath(L"MEng.System.CQC.Runtime.CMLWSockBase");
    static const TString   strCMLWSockBasePath(L"MEng.System.CQC.Runtime");
}



// ---------------------------------------------------------------------------
//  CLASS: TCMLWSockBaseInfo
// PREFIX: meci
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCMLWSockBaseInfo: Public, static methods
// ---------------------------------------------------------------------------
const TString& TCMLWSockBaseInfo::strPath()
{
    return CQCWebSrvC_CMLWSockBase::strCMLWSockClassPath;
}

// ---------------------------------------------------------------------------
//  TCMLWSockBaseInfo: Constructors and Destructor
// ---------------------------------------------------------------------------
TCMLWSockBaseInfo::TCMLWSockBaseInfo(TCIDMacroEngine& meOwner) :

    TMEngClassInfo
    (
        CQCWebSrvC_CMLWSockBase::strCMLWSockBase
        , CQCWebSrvC_CMLWSockBase::strCMLWSockBasePath
        , meOwner
        , kCIDLib::False
        , tCIDMacroEng::EClassExt::Abstract
        , L"MEng.Object"
    )
    , m_c2EnumId_Errors(kCIDMacroEng::c2BadId)
    , m_c2MethId_Connected(kCIDMacroEng::c2BadId)
    , m_c2MethId_DefCtor(kCIDMacroEng::c2BadId)
    , m_c2MethId_Disconnected(kCIDMacroEng::c2BadId)
    , m_c2MethId_EnableMsgLog(kCIDMacroEng::c2BadId)
    , m_c2MethId_FieldChanged(kCIDMacroEng::c2BadId)
    , m_c2MethId_LogMsg(kCIDMacroEng::c2BadId)
    , m_c2MethId_LogMsg1(kCIDMacroEng::c2BadId)
    , m_c2MethId_LogMsg2(kCIDMacroEng::c2BadId)
    , m_c2MethId_ProcessMsg(kCIDMacroEng::c2BadId)
    , m_c2MethId_SendMsg(kCIDMacroEng::c2BadId)
    , m_c2MethId_SetFieldList(kCIDMacroEng::c2BadId)
    , m_c2MethId_Simulate(kCIDMacroEng::c2BadId)
    , m_c2MethId_StartShutdown(kCIDMacroEng::c2BadId)
    , m_c2MethId_ValidateCreds(kCIDMacroEng::c2BadId)
    , m_c2MethId_WriteField(kCIDMacroEng::c2BadId)
    , m_c2MethId_WriteField2(kCIDMacroEng::c2BadId)
    , m_c4ErrId_FieldWrite(kCIDLib::c4MaxCard)
    , m_c4ErrId_Simulate(kCIDLib::c4MaxCard)
    , m_pmeciErrors(0)
    , m_pmeciStrList(0)
{
}

TCMLWSockBaseInfo::~TCMLWSockBaseInfo()
{
}


tCIDLib::TVoid TCMLWSockBaseInfo::Init(TCIDMacroEngine& meOwner)
{
    // We need a vector of strings for our user interface and some members
    {
        m_pmeciStrList = new TMEngVectorInfo
        (
            meOwner
            , L"WSStrList"
            , CQCWebSrvC_CMLWSockBase::strCMLWSockClassPath
            , TMEngVectorInfo::strPath()
            , tCIDLib::TCard2(tCIDMacroEng::EIntrinsics::String)
        );
        m_pmeciStrList->BaseClassInit(meOwner);
        meOwner.c2AddClass(m_pmeciStrList);
        bAddNestedType(m_pmeciStrList->strClassPath());
    }

    //
    //  Create an enumerated type for our errors that we throw back to the derived
    //  class.
    //
    {
        m_pmeciErrors = new TMEngEnumInfo
        (
            meOwner
            , L"CMLWSockErrs"
            , strClassPath()
            , L"MEng.Enum"
            , 3
        );
        m_c4ErrId_FieldWrite = m_pmeciErrors->c4AddEnumItem(L"FieldWrite", TString::strEmpty());
        m_c4ErrId_Simulate = m_pmeciErrors->c4AddEnumItem(L"Simulate", TString::strEmpty());
        m_pmeciErrors->BaseClassInit(meOwner);
        m_c2EnumId_Errors = meOwner.c2AddClass(m_pmeciErrors);
        bAddNestedType(m_pmeciErrors->strClassPath());
    }


    // Constructor
    {
        TMEngMethodInfo methiNew
        (
            L"ctor1_MEng.System.CQC.Runtime.CMLWSockBase"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.bIsCtor(kCIDLib::True);
        m_c2MethId_DefCtor = c2AddMethodInfo(methiNew);
    }

    //
    //  A client has connected. This is the first call made
    //
    {
        TMEngMethodInfo methiNew
        (
            L"Connected"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Required
        );
        m_c2MethId_Connected = c2AddMethodInfo(methiNew);
    }

    // The client has disconnected. This is the last call made
    {
        TMEngMethodInfo methiNew
        (
            L"Disconnected"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Required
        );
        m_c2MethId_Disconnected = c2AddMethodInfo(methiNew);
    }

    // Enable/disable msg logging
    {
        TMEngMethodInfo methiNew
        (
            L"EnableMsgLog"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"State", tCIDMacroEng::EIntrinsics::Boolean);
        m_c2MethId_EnableMsgLog = c2AddMethodInfo(methiNew);
    }

    // One of the fields of interest has changed value or state
    {
        TMEngMethodInfo methiNew
        (
            L"FieldChanged"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Required
        );
        methiNew.c2AddInParm(L"Moniker", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"Field", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"Value", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"GoodVal", tCIDMacroEng::EIntrinsics::Boolean);
        m_c2MethId_FieldChanged = c2AddMethodInfo(methiNew);
    }

    // Log msgs with varying numbers of replacement parms
    {
        TMEngMethodInfo methiNew
        (
            L"LogMsg"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInParm(L"Msg", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_LogMsg = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"LogMsg1"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInParm(L"Msg", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"Parm1", tCIDMacroEng::EIntrinsics::Formattable);
        m_c2MethId_LogMsg1 = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"LogMsg2"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInParm(L"Msg", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"Parm1", tCIDMacroEng::EIntrinsics::Formattable);
        methiNew.c2AddInParm(L"Parm2", tCIDMacroEng::EIntrinsics::Formattable);
        m_c2MethId_LogMsg2 = c2AddMethodInfo(methiNew);
    }


    //
    //  A message has been received. We only support text messages.
    //
    {
        TMEngMethodInfo methiNew
        (
            L"ProcessMsg"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Required
        );
        methiNew.c2AddInParm(L"MsgTex", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_ProcessMsg = c2AddMethodInfo(methiNew);
    }

    // Send a message
    {
        TMEngMethodInfo methiNew
        (
            L"SendMsg"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"MsgText", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_SendMsg = c2AddMethodInfo(methiNew);
    }

    //
    //  Set a list of fields that the derived class wants the web server to
    //  monitor for it.
    //
    {
        TMEngMethodInfo methiNew
        (
            L"SetFieldList"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"List", m_pmeciStrList->c2Id());
        m_c2MethId_SetFieldList = c2AddMethodInfo(methiNew);
    }

    // Used within the IDE to test in a simulated web server environment
    {
        TMEngMethodInfo methiNew
        (
            L"Simulate"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"PortNum", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddInParm(L"ClassPath", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"CertInfo", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_Simulate = c2AddMethodInfo(methiNew);
    }

    //
    //  Terminate. Not usually called, but lets the handler terminate the connection
    //  from the server side. The close message will be sent. No more queued messages
    //  will be sent.
    //
    {
        TMEngMethodInfo methiNew
        (
            L"StartShutdown"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"Code", tCIDMacroEng::EIntrinsics::Card2);
        m_c2MethId_StartShutdown = c2AddMethodInfo(methiNew);
    }

    // Validate a user's credentials
    {
        TMEngMethodInfo methiNew
        (
            L"ValidateCreds"
            , tCIDMacroEng::EIntrinsics::Boolean
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );

        methiNew.c2AddInParm(L"Name", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"Password", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddOutParm(L"UserType", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_ValidateCreds = c2AddMethodInfo(methiNew);
    }

    // Write the indicated value to the indicated field
    {
        TMEngMethodInfo methiNew
        (
            L"WriteField"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );

        methiNew.c2AddInParm(L"Field", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"Value", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_WriteField = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"WriteField2"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );

        methiNew.c2AddInParm(L"Moniker", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"Field", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"Value", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_WriteField2 = c2AddMethodInfo(methiNew);
    }

}


//
//  This will be called when we are run within the CML IDE. In this case, we get
//  created first, and the derived class must call the Simulate() method to start
//  the actual process, which will cause a web socket thread to be created and he
//  will be told about us and us told about him. When run normally in the web server,
//  the thread is created first, and he creates us manually, so this is not called.
//
TMEngClassVal*
TCMLWSockBaseInfo::pmecvMakeStorage(const   TString&                strName
                                    ,       TCIDMacroEngine&        meOwner
                                    , const tCIDMacroEng::EConstTypes  eConst) const
{
    // We just create a standard value object. We don't need one of our own
    return new TMEngStdClassVal(strName, c2Id(), eConst);
}


// ---------------------------------------------------------------------------
//  TCMLWSockBaseInfo: Protected, inherited methods
// ---------------------------------------------------------------------------

//
//  Many of our methods are virtuals that are called by the web server, not called
//  by this class.
//
tCIDLib::TBoolean
TCMLWSockBaseInfo::bInvokeMethod(       TCIDMacroEngine&    meOwner
                                , const TMEngMethodInfo&    methiTarget
                                ,       TMEngClassVal&      mecvInstance)
{
    const tCIDLib::TCard4 c4FirstInd = meOwner.c4FirstParmInd(methiTarget);
    const tCIDLib::TCard2 c2MethId   = methiTarget.c2Id();

    //
    //  Get the pointer tot he CML handler object from our value object and cast
    //  it to the right type. Not all of the methods will need it, but enough will
    //  that it's efficient to just do it once.
    //
    TWebsockCMLThread* pthrHandler = static_cast<TWebsockCMLThread*>(mecvInstance.pExtra());

    if (c2MethId == m_c2MethId_DefCtor)
    {
        // Nothing to do
    }
     else if (c2MethId == m_c2MethId_EnableMsgLog)
    {
        // Just pass it through to our parent class
        pthrHandler->EnableMsgLogging(meOwner.bStackValAt(c4FirstInd));
    }
     else if ((c2MethId == m_c2MethId_LogMsg)
          ||  (c2MethId == m_c2MethId_LogMsg1)
          ||  (c2MethId == m_c2MethId_LogMsg2))
    {
        // Get the string and call a helper to format in any formattable tokens
        TString strMsg(meOwner.strStackValAt(c4FirstInd));
        ReplaceTokens
        (
            meOwner, strMsg, (c2MethId - m_c2MethId_LogMsg), c4FirstInd + 1
        );

        // And now create an error object and log it
        TError errToLog
        (
            facCQCWebSrvC().strName()
            , meOwner.strClassPathForId(mecvInstance.c2ClassId())
            , meOwner.c4CurLine()
            , strMsg
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
        TModule::LogEventObj(errToLog);
    }
     else if (c2MethId == m_c2MethId_SetFieldList)
    {
        //
        //  We have to get them from the CML vector to a C++ vector, which we can
        //  then pass to the handler class.
        //
        TMEngVectorVal& mecvList = meOwner.mecvStackAtAs<TMEngVectorVal>(c4FirstInd);

        const tCIDLib::TCard4 c4Count = mecvList.c4ElemCount();
        tCIDLib::TStrList colFlds(c4Count);
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            colFlds.objAdd
            (
                static_cast<const TMEngStringVal&>(mecvList.mecvAt(meOwner, c4Index)).strValue()
            );
        }

        CIDAssert(pthrHandler != 0, L"CML Websocket C++ handler glue not set");
        pthrHandler->SetFieldList(colFlds);
    }
     else if (c2MethId == m_c2MethId_SendMsg)
    {
        CIDAssert(pthrHandler != 0, L"CML Websocket C++ handler glue not set");
        pthrHandler->SendTextMsg(meOwner.strStackValAt(c4FirstInd));
    }
     else if (c2MethId == m_c2MethId_Simulate)
    {
        //
        //  We have to create our own special thread, which will simulate the basic
        //  functionality required to accept a websocket connection. It will listen
        //  for connections and, when one is received, will create a CML handler
        //  thread and point it at our value object. We get a port number to use, since
        //  often 80 will be taken on the machine that the IDE is running on.
        //
        try
        {
            Simulate
            (
                meOwner
                , mecvInstance
                , meOwner.c4StackValAt(c4FirstInd)
                , meOwner.strStackValAt(c4FirstInd + 1)
                , meOwner.strStackValAt(c4FirstInd + 2)
            );
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            ThrowAnErr(meOwner, m_c4ErrId_Simulate, errToCatch);
        }
    }
     else if (c2MethId == m_c2MethId_StartShutdown)
    {
        CIDAssert(pthrHandler != 0, L"CML Websocket C++ handler glue not set");
        pthrHandler->StartShutdown(meOwner.c2StackValAt(c4FirstInd));
    }
     else if (c2MethId == m_c2MethId_ValidateCreds)
    {
        // Validate the passed user credentials
        TCQCSecToken    sectToFill;
        TCQCUserAccount uaccToFill;
        TString         strFailReason;

        tCIDLib::TBoolean bRes = pthrHandler->bValidateLogin
        (
            meOwner.strStackValAt(c4FirstInd)
            , meOwner.strStackValAt(c4FirstInd + 1)
            , sectToFill
            , uaccToFill
            , strFailReason
        );

        if (bRes)
        {
            TMEngStringVal& mecvType = meOwner.mecvStackAtAs<TMEngStringVal>(c4FirstInd + 2);
            mecvType.strValue(tCQCKit::strAltXlatEUserRoles(uaccToFill.eRole()));
        }

        TMEngBooleanVal& mecvRet = meOwner.mecvStackAtAs<TMEngBooleanVal>(c4FirstInd - 1);
        mecvRet.bValue(bRes);
    }
     else if (c2MethId == m_c2MethId_WriteField)
    {
        //
        //  We do this via the poll engine, since it often already has the field info
        //  and do it most efficiently. If not, it'll do it the less efficient way.
        //
        try
        {
            facCQCWebSrvC().WriteField
            (
                meOwner.strStackValAt(c4FirstInd)
                , meOwner.strStackValAt(c4FirstInd + 1)
            );
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            ThrowAnErr(meOwner, m_c4ErrId_FieldWrite, errToCatch);
        }
    }
     else if (c2MethId == m_c2MethId_WriteField2)
    {
        try
        {
            facCQCWebSrvC().WriteField
            (
                meOwner.strStackValAt(c4FirstInd)
                , meOwner.strStackValAt(c4FirstInd + 1)
                , meOwner.strStackValAt(c4FirstInd + 2)
            );
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            ThrowAnErr(meOwner, m_c4ErrId_FieldWrite, errToCatch);
        }
    }
     else
    {
        // We don't know what it is
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


// ---------------------------------------------------------------------------
//  TCMLWSockBaseInfo: Protected, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid
TCMLWSockBaseInfo::ProcessSimReq(       TCIDDataSrcJan&         janSrc
                                ,       TCIDMacroEngine&        meOwner
                                ,       TMEngClassVal&          mecvInstance
                                , const TString&                strURL
                                , const tCIDLib::TKVPList&      colHdrLines
                                , const TString&                strReqType
                                , const TString&                strClassPath)
{
    try
    {
        //
        //  Do basic validation on it. If it fails, just return. An error reply has
        //  already been sent back.
        //
        TString strSecKey;
        const tCIDLib::TBoolean bValid = TWebsockThread::bValidateReqInfo
        (
            janSrc.cdsData(), strURL, colHdrLines, strReqType, strSecKey
        );

        if (!bValid)
            return;

        // Parse the URL, so we can get any query parms
        const TURL urlReq(strURL, tCIDSock::EQualified::DontCare);

        //
        //  Create a handler thread and set it up. We put a janitor on it to make
        //  sure it gets cleaned up. A regular janitor is fine because we aren't
        //  really running it as a separate thread in this case. We are just running
        //  it passively in this thread context.
        //
        TWebsockCMLThread* pthrHandler = new TWebsockCMLThread(kCIDLib::True);
        TJanitor<TWebsockCMLThread> janThread(pthrHandler);

        //
        //  This is the handoff that is normally done by the web server to set up
        //  the websocket handler to take over. In our case, in sim mode, it just
        //  stores the data away and returns.
        //
        TCIDDataSrc* pcdsServer = janSrc.pcdsData();
        pthrHandler->Reset(janSrc, urlReq, strSecKey);

        //
        //  This is a little wierd, but the handler adopts the source normally, so
        //  Reset() orphans it out of the janitor. It then starts the thread which
        //  immeidately puts another one on it in the new thread context. But we are
        //  not starting up a new thread. So we have put another janitor back on it
        //  now to make sure it gets cleaned up.
        //
        TCIDDataSrcJan janDataSrc(pcdsServer, tCIDLib::EAdoptOpts::Adopt, kCIDLib::True);

        //
        //  This tells the CML derivative about the CML stuff that normally he would
        //  have created, but in this case we have to give to him.
        //
        pthrHandler->Simulate(meOwner, mecvInstance, strClassPath);

        //
        //  Set our value object's 'extra' pointer to the thread object. This provides
        //  the back link when the outgoing (as apposed to callback) methods are called
        //  by the derived class.
        //
        mecvInstance.SetExtra(pthrHandler);

        //
        //  And now call the service method on the thread object. It will stay there
        //  until an error or we exit.
        //
        //  Normally this would have been started up by the Reset() call above,
        //  which would have spun off a separate thread to drive the handler. But we
        //  want it to just run in our context.
        //
        pthrHandler->eServiceClient();
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);
    }

    catch(const TDbgExitException&)
    {
        // Pass it on
        throw;
    }

    catch(...)
    {
    }
}


//
//  Handles token replacement with formattable objects at the CML level. We just use
//  a stream that the
//
tCIDLib::TVoid
TCMLWSockBaseInfo::ReplaceTokens(       TCIDMacroEngine&    meOwner
                                ,       TString&            strMsg
                                , const tCIDLib::TCard4     c4Tokens
                                , const tCIDLib::TCard4     c4FirstInd)
{
    TMEngClassInfo& meciStrm = meOwner.meciFind(tCIDMacroEng::EIntrinsics::StringOutStream);
    TMEngTextOutStreamVal* pmecvStrm = static_cast<TMEngTextOutStreamVal*>
    (
        meciStrm.pmecvMakeStorage
        (
            L"TempStream", meOwner, tCIDMacroEng::EConstTypes::NonConst
        )
    );
    TJanitor<TMEngTextOutStreamVal> janStrm(pmecvStrm);

    //
    //  Give it a stream, since we won't be constructing it at the macro
    //  language level, which normally is how it gets one.
    //
    TTextStringOutStream* pstrmActual = new TTextStringOutStream(1024UL);
    pmecvStrm->SetStream(pstrmActual, tCIDLib::EAdoptOpts::Adopt);

    tCIDLib::TCh chToken(kCIDLib::chDigit1);
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Tokens; c4Index++)
    {
        // Reset the output stream for this round
        pstrmActual->Reset();

        //
        //  Get the current formattable object. One gets us down to the actual
        //  top item, then add the current index to get to the current one.
        //
        TMEngFormattableVal& mecvCur = meOwner.mecvStackAtAs<TMEngFormattableVal>
        (
            c4FirstInd + c4Index
        );
        TMEngClassInfo& meciTarget = meOwner.meciFind(mecvCur.c2ClassId());

        //
        //  Generate a macro level call to format this guy to the stream. Tell
        //  the call stack that the stream is a 'repush', though its not, so
        //  that it won't try to delete it on us.
        //
        meOwner.PushPoolValue(tCIDMacroEng::EIntrinsics::Void, tCIDMacroEng::EConstTypes::Const);
        meOwner.PushValue(pmecvStrm, tCIDMacroEng::EStackItems::Parm, kCIDLib::True);
        meOwner.meciPushMethodCall
        (
            meciTarget.c2Id(), TMEngFormattableInfo::c2FormatToId()
        );

        meciTarget.Invoke
        (
            meOwner
            , mecvCur
            , TMEngFormattableInfo::c2FormatToId()
            , tCIDMacroEng::EDispatch::Poly
        );

        //
        //  We cheated this one, so we have to pop the method item as well
        //  the parm and return!
        //
        meOwner.MultiPop(3);

        // And now replace the current token with this result. Flush first!
        pstrmActual->Flush();
        strMsg.eReplaceToken(pstrmActual->strData(), chToken);

        // Move up to the next token digit
        chToken++;
    }
}



//
//  This method is called by the derived CML handler class when it is run within the
//  the IDE (via the Start() method.) We have to listen for socket connections on the
//  indicated port, and basically play like we are a web server to the minimual ammount
//  required to support a single client that can only do web socket connections.
//
//  WE don't create a separate listener thread, and we don't keep listening. We only
//  deal with a single conneciton at a time. When the IDE requests a stop, we break
//  out and return.
//
tCIDLib::TVoid
TCMLWSockBaseInfo::Simulate(        TCIDMacroEngine&    meOwner
                            ,       TMEngClassVal&      mecvInstance
                            , const tCIDLib::TIPPortNum ippnPort
                            , const TString&            strClassPath
                            , const TString&            strCertInfo)
{
    //
    //  First we create a listener. If it throws, let it. It will throw back out of
    //  derived CML class' Simulate method. If it works, put a janitor on it.
    //
    TSocketListener* psocklSim = new TSocketListener
    (
        ippnPort, tCIDSock::ESockProtos::TCP, kCIDLib::True
    );
    TJanitor<TSocketListener> janListen(psocklSim);

    //
    //  And let's enter our loop until asked to stop.
    //
    TThread* pthrCaller = TThread::pthrCaller();
    TIPEndPoint ipepClient;
    while (!pthrCaller->bCheckShutdownRequest())
    {
        // Wait a while for a connect
        TServerStreamSocket* psockSrv = psocklSim->psockListenFor
        (
            kCIDLib::enctOneSecond, ipepClient
        );

        // If we got one, then let's process it
        if (psockSrv)
        {
            //
            //  If we got certificate info, then assume we are accepting secure
            //  connections. Else, non-secure.
            //
            TCIDDataSrc* pcdsDataSrc = 0;
            if (strCertInfo.bIsEmpty())
            {
                pcdsDataSrc = new TCIDSockStreamDataSrc(psockSrv, tCIDLib::EAdoptOpts::Adopt);
            }
             else
            {
                pcdsDataSrc = new TCIDSChanSrvDataSrc
                (
                    L"CQC CML Web"
                    , psockSrv
                    , tCIDLib::EAdoptOpts::Adopt
                    , strCertInfo
                    , tCIDSChan::EConnOpts::None
                );

            }
            TCIDDataSrcJan janSrc(pcdsDataSrc, tCIDLib::EAdoptOpts::Adopt, kCIDLib::True);

            tCIDLib::TCard4     c4ContLen;
            tCIDLib::TCard4     c4ProtoVer;
            TString             strContType;
            TString             strType;
            TString             strURL;
            THTTPClient         httpcSrv;
            THeapBuf            mbufCont(8192, 64 * 1024, 16 * 1024);
            tCIDLib::TKVPList   colInHdrLines;

            // Read a request. This guy doesn't throw
            tCIDNet::ENetPReadRes eRes = httpcSrv.eGetClientMsg
            (
                janSrc.cdsData()
                , TTime::enctNowPlusSecs(5)
                , strType
                , colInHdrLines
                , strContType
                , strURL
                , c4ProtoVer
                , mbufCont
                , c4ContLen
            );

            // If not successful, it already sent an error reply, else process it
            if (eRes == tCIDNet::ENetPReadRes::Success)
            {
                ProcessSimReq
                (
                    janSrc
                    , meOwner
                    , mecvInstance
                    , strURL
                    , colInHdrLines
                    , strType
                    , strClassPath
                );
            }
        }

        // Give the IDE a chance to ask us to stop us
        try
        {
            meOwner.CheckIDEReq();
        }

        catch(const TDbgExitException&)
        {
            // Break out of the loop and return
            break;
        }
    }
}



tCIDLib::TVoid
TCMLWSockBaseInfo::ThrowAnErr(      TCIDMacroEngine&    meOwner
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
TCMLWSockBaseInfo::ThrowAnErr(          TCIDMacroEngine&    meOwner
                                , const tCIDLib::TCard4     c4ToThrow
                                , const TError&             errCaught) const
{
    if (facCQCWebSrvC().bShouldLog(errCaught))
        TModule::LogEventObj(errCaught);

    // Set the exception info on the engine
    meOwner.SetException
    (
        m_c2EnumId_Errors
        , strClassPath()
        , c4ToThrow
        , m_pmeciErrors->strPartialName(c4ToThrow)
        , errCaught.strErrText()
        , meOwner.c4CurLine()
    );

    // And throw the excpetion that represents a macro level exception
    throw TExceptException();
}


