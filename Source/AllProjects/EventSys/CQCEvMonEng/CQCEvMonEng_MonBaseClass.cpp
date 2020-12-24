//
// FILE NAME: CQCEvMonEng_MonBaseClass.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/08/2014
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
//  is used as the base class for all monitors.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include "CQCEvMonEng_.hpp"


// ---------------------------------------------------------------------------
//  Magic RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TEvMonBaseInfo,TMEngClassInfo)



// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace CQCEvMonEng_MonBaseClass
{
    namespace
    {
        // -----------------------------------------------------------------------
        //  The names for the types that we support here. Each derivative has to
        //  be able to return strings that contain its name and full name.
        // -----------------------------------------------------------------------
        const TString   strEVMonBase(L"EvMonBase");
        const TString   strEVMonClassPath(L"MEng.System.CQC.Runtime.EvMonBase");
        const TString   strEVMonBasePath(L"MEng.System.CQC.Runtime");
    }
}



// ---------------------------------------------------------------------------
//  CLASS: TCQCEvMonBaseInfo
// PREFIX: meci
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCEvMonBaseInfo: Public, static methods
// ---------------------------------------------------------------------------
const TString& TEvMonBaseInfo::strPath()
{
    return CQCEvMonEng_MonBaseClass::strEVMonClassPath;
}

// ---------------------------------------------------------------------------
//  TEvMonBaseInfo: Constructors and Destructor
// ---------------------------------------------------------------------------
TEvMonBaseInfo::TEvMonBaseInfo(TCIDMacroEngine& meOwner) :

    TMEngClassInfo
    (
        CQCEvMonEng_MonBaseClass::strEVMonBase
        , CQCEvMonEng_MonBaseClass::strEVMonBasePath
        , meOwner
        , kCIDLib::False
        , tCIDMacroEng::EClassExt::Abstract
        , L"MEng.Object"
    )
    , m_c2EnumId_Errors(kCIDMacroEng::c2BadId)
    , m_c2MethId_DefCtor(kCIDMacroEng::c2BadId)
    , m_c2MethId_FieldChanged(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetName(kCIDMacroEng::c2BadId)
    , m_c2MethId_Idle(kCIDMacroEng::c2BadId)
    , m_c2MethId_Initialize(kCIDMacroEng::c2BadId)
    , m_c2MethId_LoadConfig(kCIDMacroEng::c2BadId)
    , m_c2MethId_LogMsg(kCIDMacroEng::c2BadId)
    , m_c2MethId_LogMsg1(kCIDMacroEng::c2BadId)
    , m_c2MethId_LogMsg2(kCIDMacroEng::c2BadId)
    , m_c2MethId_ParseFldName(kCIDMacroEng::c2BadId)
    , m_c2MethId_SetFieldList(kCIDMacroEng::c2BadId)
    , m_c2MethId_SetIdleSecs(kCIDMacroEng::c2BadId)
    , m_c2MethId_Simulate(kCIDMacroEng::c2BadId)
    , m_c2MethId_Terminate(kCIDMacroEng::c2BadId)
    , m_c2MethId_WriteField(kCIDMacroEng::c2BadId)
    , m_c2MethId_WriteField2(kCIDMacroEng::c2BadId)
    , m_c4ErrId_FieldWrite(kCIDLib::c4MaxCard)
    , m_c4ErrId_SetFieldList(kCIDLib::c4MaxCard)
    , m_c4ErrId_Simulate(kCIDLib::c4MaxCard)
    , m_pmeciErrors(0)
    , m_pmeciStrList(0)
{
}

TEvMonBaseInfo::~TEvMonBaseInfo()
{
}


tCIDLib::TVoid TEvMonBaseInfo::Init(TCIDMacroEngine& meOwner)
{
    // We need a vector of strings for our user interface and some members
    {
        m_pmeciStrList = new TMEngVectorInfo
        (
            meOwner
            , L"EvMonStrList"
            , CQCEvMonEng_MonBaseClass::strEVMonClassPath
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
            , L"EVMonErrs"
            , strClassPath()
            , L"MEng.Enum"
            , 3
        );
        m_c4ErrId_FieldWrite = m_pmeciErrors->c4AddEnumItem(L"FieldWrite", TString::strEmpty());
        m_c4ErrId_SetFieldList = m_pmeciErrors->c4AddEnumItem(L"SetFieldList", TString::strEmpty());
        m_c4ErrId_Simulate = m_pmeciErrors->c4AddEnumItem(L"Simulate", TString::strEmpty());
        m_pmeciErrors->BaseClassInit(meOwner);
        m_c2EnumId_Errors = meOwner.c2AddClass(m_pmeciErrors);
        bAddNestedType(m_pmeciErrors->strClassPath());
    }


    // Constructor
    {
        TMEngMethodInfo methiNew
        (
            L"ctor1_MEng.System.CQC.Runtime.EVMonBase"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.bIsCtor(kCIDLib::True);
        m_c2MethId_DefCtor = c2AddMethodInfo(methiNew);
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

    // Get teh configured name of this event monitor
    {
        TMEngMethodInfo methiNew
        (
            L"GetName"
            , tCIDMacroEng::EIntrinsics::String
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        m_c2MethId_GetName = c2AddMethodInfo(methiNew);
    }

    //
    //  Called once every X seconds to allow for idle time processing if there's
    //  nothing else going on.
    //
    {
        TMEngMethodInfo methiNew
        (
            L"Idle"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Required
        );
        m_c2MethId_Idle = c2AddMethodInfo(methiNew);
    }

    //
    //  Called on start, to let the CML handler initialize itself. The pre-parsed
    //  parameters are passed as a vector of strings.
    //
    {
        TMEngMethodInfo methiNew
        (
            L"Initialize"
            , tCIDMacroEng::EIntrinsics::Boolean
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Required
        );
        methiNew.c2AddInParm(L"Params", m_pmeciStrList->c2Id());
        m_c2MethId_Initialize = c2AddMethodInfo(methiNew);
    }

    // Called after init, to let the CML handler load any config
    {
        TMEngMethodInfo methiNew
        (
            L"LoadConfig"
            , tCIDMacroEng::EIntrinsics::Boolean
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Required
        );
        m_c2MethId_LoadConfig = c2AddMethodInfo(methiNew);
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

    // Parse the passed field name. If valid, returns moniker and field name
    {
        TMEngMethodInfo methiNew
        (
            L"ParseFldName"
            , tCIDMacroEng::EIntrinsics::Boolean
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInParm(L"ToParse", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddOutParm(L"Moniker", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddOutParm(L"Field", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_ParseFldName = c2AddMethodInfo(methiNew);
    }

    //
    //  Set a list of fields that the derived class wants the event server to
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

    // Set the number of seconds between idle time callbacks
    {
        TMEngMethodInfo methiNew
        (
            L"SetIdleSecs"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"ToSet", tCIDMacroEng::EIntrinsics::Card4);
        m_c2MethId_SetIdleSecs = c2AddMethodInfo(methiNew);
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
        methiNew.c2AddInParm(L"Name", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"Params", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"ClassPath", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_Simulate = c2AddMethodInfo(methiNew);
    }

    //
    //  Called when it's being stopped (after a successful initialize), to let
    //  the derived class handler clean up.
    //
    {
        TMEngMethodInfo methiNew
        (
            L"Terminate"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Required
        );
        m_c2MethId_Terminate = c2AddMethodInfo(methiNew);
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
TEvMonBaseInfo::pmecvMakeStorage(const  TString&                strName
                                ,       TCIDMacroEngine&        meOwner
                                , const tCIDMacroEng::EConstTypes  eConst) const
{
    // We just create a standard value object. We don't need one of our own
    return new TMEngStdClassVal(strName, c2Id(), eConst);
}


// ---------------------------------------------------------------------------
//  TEvMonBaseInfo: Protected, inherited methods
// ---------------------------------------------------------------------------

//
//  Many of our methods are virtuals that are called by the web server, not called
//  by this class.
//
tCIDLib::TBoolean
TEvMonBaseInfo::bInvokeMethod(          TCIDMacroEngine&    meOwner
                                , const TMEngMethodInfo&    methiTarget
                                ,       TMEngClassVal&      mecvInstance)
{
    const tCIDLib::TCard4 c4FirstInd = meOwner.c4FirstParmInd(methiTarget);
    const tCIDLib::TCard2 c2MethId   = methiTarget.c2Id();

    // Get a reference to the monitor object that owns us
    TCQCEvMonitor* pthrOwner = static_cast<TCQCEvMonitor*>(mecvInstance.pExtra());


    if (c2MethId == m_c2MethId_DefCtor)
    {
        // Nothing to do
    }
     else if (c2MethId == m_c2MethId_GetName)
    {
        TMEngStringVal& mecvRet = meOwner.mecvStackAtAs<TMEngStringVal>(c4FirstInd - 1);
        mecvRet.strValue(pthrOwner->strName());
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
            facCQCEvMonEng().strName()
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
        try
        {
            //
            //  We have to get them from the CML vector to a C++ vector, which we can
            //  then pass to the polling engine.
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
            pthrOwner->SetFieldList(colFlds);
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);

            // Convert to CML exception and rethrow
            ThrowAnErr(meOwner, m_c4ErrId_SetFieldList, errToCatch);
        }
    }
     else if (c2MethId == m_c2MethId_ParseFldName)
    {
        TMEngBooleanVal& mecvRet = meOwner.mecvStackAtAs<TMEngBooleanVal>(c4FirstInd - 1);
        TMEngStringVal& mecvMon = meOwner.mecvStackAtAs<TMEngStringVal>(c4FirstInd + 1);
        TMEngStringVal& mecvFld = meOwner.mecvStackAtAs<TMEngStringVal>(c4FirstInd + 2);

        mecvRet.bValue
        (
            facCQCKit().bParseFldName
            (
                meOwner.strStackValAt(c4FirstInd), mecvMon.strValue(), mecvFld.strValue()
            )
        );
    }
     else if (c2MethId == m_c2MethId_SetIdleSecs)
    {
        // Store away the incoming seconds, clipping to acceptable values
        pthrOwner->SetIdleSecs(meOwner.c4StackValAt(c4FirstInd));
    }
     else if (c2MethId == m_c2MethId_Simulate)
    {
        //
        //  Call our simulator helper. In this case, we have to create our own monitor
        //  object, when normally it's done the other way around.
        //
        try
        {
            //
            //  The name is no longer used, but we don't want to break the existing
            //  CML code by removing it.
            //

            // const TString& strName = meOwner.strStackValAt(c4FirstInd);

            const TString& strParams = meOwner.strStackValAt(c4FirstInd + 1);
            const TString& strClass = meOwner.strStackValAt(c4FirstInd + 2);

            //
            //  We have to create our own event monitor thread object, though we do not
            //  run it as a thread. We also have to fake some configuration as well,
            //  since we don't have any in this case.
            //
            //  We have to provide a path, even though it's not actually loaded from
            //  the data server, so we just set a bogus one.
            //
            TCQCEvMonCfg emoncTest;
            emoncTest.Set(strClass, L"Simulated event monitor", strParams);

            TCQCEvMonitor thrSim(L"/Events/Monitors/User/$BogusTestPath$", emoncTest);
            thrSim.Simulate(meOwner, mecvInstance, strClass);
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);

            // Convert to CML exception and rethrow
            ThrowAnErr(meOwner, m_c4ErrId_Simulate, errToCatch);
        }
    }
     else if (c2MethId == m_c2MethId_WriteField)
    {
        //
        //  We do this via the poll engine, since it often already has the field info
        //  and do it most efficiently. If not, it'll do it the less efficient way.
        //
        try
        {
            facCQCEvMonEng().WriteField
            (
                meOwner.strStackValAt(c4FirstInd)
                , meOwner.strStackValAt(c4FirstInd + 1)
            );
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);

            // Convert to CML exception and rethrow
            ThrowAnErr(meOwner, m_c4ErrId_FieldWrite, errToCatch);
        }
    }
     else if (c2MethId == m_c2MethId_WriteField2)
    {
        try
        {
            facCQCEvMonEng().WriteField
            (
                meOwner.strStackValAt(c4FirstInd)
                , meOwner.strStackValAt(c4FirstInd + 1)
                , meOwner.strStackValAt(c4FirstInd + 2)
            );
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);

            // Convert to CML exception and rethrow
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
//  TEvMonBaseInfo: Protected, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Handles token replacement with formattable objects at the CML level.
//
tCIDLib::TVoid
TEvMonBaseInfo::ReplaceTokens(          TCIDMacroEngine&    meOwner
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


tCIDLib::TVoid
TEvMonBaseInfo::ThrowAnErr(         TCIDMacroEngine&    meOwner
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
TEvMonBaseInfo::ThrowAnErr(         TCIDMacroEngine&    meOwner
                            , const tCIDLib::TCard4     c4ToThrow
                            , const TError&             errCaught) const
{
    if (facCQCEvMonEng().bShouldLog(errCaught))
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


