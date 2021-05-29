//
// FILE NAME: CQCMEng_LoggerClass.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/01/2003
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
//  This file implements the info and value classes to create a macro level
//  class which supports logging for CML classes.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCMEng_.hpp"
#include    "CQCMEng_LoggerClass_.hpp"


// ---------------------------------------------------------------------------
//  Magic RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCLoggerVal,TMEngClassVal)
RTTIDecls(TCQCLoggerInfo,TMEngClassInfo)


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace
{
    namespace CQCMacroEng_Logger
    {
        // -----------------------------------------------------------------------
        //  The names for the types that we support here. Each derivative has to
        //  be able to return strings that contain its name and full name.
        // -----------------------------------------------------------------------
        const TString   strLogger(L"CQCLogger");
        const TString   strLoggerClassPath(L"MEng.System.CQC.Runtime.CQCLogger");
        const TString   strLoggerBasePath(L"MEng.System.CQC.Runtime");
    }
}




// ---------------------------------------------------------------------------
//  CLASS: TCQCLoggerVal
// PREFIX: mecv
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCLoggerVal: Constuctors and Destructor
// ---------------------------------------------------------------------------
TCQCLoggerVal::TCQCLoggerVal(const  TString&                strName
                            , const tCIDLib::TCard2         c2ClassId
                            , const tCIDMacroEng::EConstTypes  eConst) :

    TMEngClassVal(strName, c2ClassId, eConst)
{
}

TCQCLoggerVal::~TCQCLoggerVal()
{
}



// ---------------------------------------------------------------------------
//  CLASS: TCQCLoggerInfo
// PREFIX: meci
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCLoggerInfo: Public, static methods
// ---------------------------------------------------------------------------
const TString& TCQCLoggerInfo::strClassPath()
{
    return CQCMacroEng_Logger::strLoggerClassPath;
}


// ---------------------------------------------------------------------------
//  TCQCLoggerInfo: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCLoggerInfo::TCQCLoggerInfo(TCIDMacroEngine& meOwner) :

    TMEngClassInfo
    (
        CQCMacroEng_Logger::strLogger
        , CQCMacroEng_Logger::strLoggerBasePath
        , meOwner
        , kCIDLib::False
        , tCIDMacroEng::EClassExt::Final
        , L"MEng.Object"
    )
    , m_c2MethId_DefCtor(kCIDMacroEng::c2BadId)
    , m_c2MethId_LogExcept(kCIDMacroEng::c2BadId)
    , m_c2MethId_LogExceptP(kCIDMacroEng::c2BadId)
    , m_c2MethId_LogMsg(kCIDMacroEng::c2BadId)
    , m_c2MethId_LogMsgP(kCIDMacroEng::c2BadId)
    , m_c2MethId_LogMsg1(kCIDMacroEng::c2BadId)
    , m_c2MethId_LogMsg1P(kCIDMacroEng::c2BadId)
    , m_c2MethId_LogMsg2(kCIDMacroEng::c2BadId)
    , m_c2MethId_LogMsg2P(kCIDMacroEng::c2BadId)
    , m_c2MethId_LogMsg3(kCIDMacroEng::c2BadId)
    , m_c2MethId_LogMsg3P(kCIDMacroEng::c2BadId)
    , m_pmeciExcept(nullptr)
    , m_pmecvStrm(nullptr)
{
}

TCQCLoggerInfo::~TCQCLoggerInfo()
{
    // Clean up our output stream
    try
    {
        delete m_pmecvStrm;
    }

    catch(const TError& errToCatch)
    {
        if (facCQCKit().bShouldLog(errToCatch))
            TModule::LogEventObj(errToCatch);
    }
}


// ---------------------------------------------------------------------------
//  TCQCLoggerInfo: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCLoggerInfo::Init(TCIDMacroEngine& meOwner)
{
    // Add the default constructor
    {
        TMEngMethodInfo methiNew
        (
            L"ctor1_MEng.System.CQC.Runtime.CQCLogger"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.bIsCtor(kCIDLib::True);
        m_c2MethId_DefCtor = c2AddMethodInfo(methiNew);
    }

    // Look up the exception class which we reference
    m_pmeciExcept = meOwner.pmeciFindAs<TMEngExceptInfo>
    (
        TMEngExceptInfo::strPath(), kCIDLib::True
    );

    //
    //  A convenience to log a cause exception with a msg, and another variation
    //  that takes position info.
    //
    {
        TMEngMethodInfo methiNew
        (
            L"LogExcept"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"MsgToLog", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"Exception", m_pmeciExcept->c2Id());
        m_c2MethId_LogExcept = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"LogExceptP"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"MsgToLog", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"Exception", m_pmeciExcept->c2Id());
        methiNew.c2AddInParm(L"ClassPath", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"LineNum", tCIDMacroEng::EIntrinsics::Card4);
        m_c2MethId_LogExceptP = c2AddMethodInfo(methiNew);
    }

    //
    //  We support a couple variations on logging, for just the message
    //  or the message with 1, 2 or 3 replacement tokens. And we have
    //  another set of those that also take the calling class path and
    //  line number.
    //
    {
        TMEngMethodInfo methiNew
        (
            L"LogMsg"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"MsgToLog", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_LogMsg = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"LogMsgP"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"MsgToLog", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"ClassPath", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"LineNum", tCIDMacroEng::EIntrinsics::Card4);
        m_c2MethId_LogMsgP = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"LogMsg1"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"MsgToLog", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"Token1", tCIDMacroEng::EIntrinsics::Formattable);
        m_c2MethId_LogMsg1 = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"LogMsg1P"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"MsgToLog", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"Token1", tCIDMacroEng::EIntrinsics::Formattable);
        methiNew.c2AddInParm(L"ClassPath", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"LineNum", tCIDMacroEng::EIntrinsics::Card4);
        m_c2MethId_LogMsg1P = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"LogMsg2"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"MsgToLog", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"Token1", tCIDMacroEng::EIntrinsics::Formattable);
        methiNew.c2AddInParm(L"Token2", tCIDMacroEng::EIntrinsics::Formattable);
        m_c2MethId_LogMsg2 = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"LogMsg2P"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"MsgToLog", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"Token1", tCIDMacroEng::EIntrinsics::Formattable);
        methiNew.c2AddInParm(L"Token2", tCIDMacroEng::EIntrinsics::Formattable);
        methiNew.c2AddInParm(L"ClassPath", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"LineNum", tCIDMacroEng::EIntrinsics::Card4);
        m_c2MethId_LogMsg2P = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"LogMsg3"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"MsgToLog", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"Token1", tCIDMacroEng::EIntrinsics::Formattable);
        methiNew.c2AddInParm(L"Token2", tCIDMacroEng::EIntrinsics::Formattable);
        methiNew.c2AddInParm(L"Token3", tCIDMacroEng::EIntrinsics::Formattable);
        m_c2MethId_LogMsg3 = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"LogMsg3P"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"MsgToLog", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"Token1", tCIDMacroEng::EIntrinsics::Formattable);
        methiNew.c2AddInParm(L"Token2", tCIDMacroEng::EIntrinsics::Formattable);
        methiNew.c2AddInParm(L"Token3", tCIDMacroEng::EIntrinsics::Formattable);
        methiNew.c2AddInParm(L"ClassPath", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"LineNum", tCIDMacroEng::EIntrinsics::Card4);
        m_c2MethId_LogMsg3P = c2AddMethodInfo(methiNew);
    }

    //
    //  Create a macro level string out stream, which we will us for the
    //  logging methods and possibly others, which take one or more
    //  formattable objects to replace tokens with. We don't want to
    //  have to create this on every log event.
    //
    TMEngClassInfo& meciStrm = meOwner.meciFind
    (
        tCIDMacroEng::EIntrinsics::StringOutStream
    );
    m_pmecvStrm = static_cast<TMEngTextOutStreamVal*>
    (
        meciStrm.pmecvMakeStorage
        (
            L"TempStream"
            , meOwner
            , tCIDMacroEng::EConstTypes::NonConst
        )
    );

    //
    //  Give it a stream, since we won't be constructing it at the macro
    //  language level, which normally is how it gets one.
    //
    m_pmecvStrm->SetStream
    (
        new TTextStringOutStream(1024UL)
        , tCIDLib::EAdoptOpts::Adopt
    );
}


TMEngClassVal*
TCQCLoggerInfo::pmecvMakeStorage(const  TString&                strName
                                ,       TCIDMacroEngine&
                                , const tCIDMacroEng::EConstTypes  eConst) const
{
    return new TCQCLoggerVal(strName, c2Id(), eConst);
}



// ---------------------------------------------------------------------------
//  TCQCLoggerInfo: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TCQCLoggerInfo::bInvokeMethod(          TCIDMacroEngine&    meOwner
                                , const TMEngMethodInfo&    methiTarget
                                ,       TMEngClassVal&      mecvInstance)
{
    // Don't need this so far
//    TCQCLoggerVal& mecvActual = static_cast<TCQCLoggerVal&>(mecvInstance);

    const tCIDLib::TCard4 c4FirstInd = meOwner.c4FirstParmInd(methiTarget);
    const tCIDLib::TCard2 c2MethId   = methiTarget.c2Id();

    if (c2MethId == m_c2MethId_DefCtor)
    {
        // Nothing to do
    }
     else if ((c2MethId == m_c2MethId_LogExcept)
          ||  (c2MethId == m_c2MethId_LogExceptP))
    {
        TMEngExceptVal& mecvExcept
                    = meOwner.mecvStackAtAs<TMEngExceptVal>(c4FirstInd + 1);

        // Format out the exception info. We pass it as the extra text
        TString strErrText(L"CLASS: ", 256UL);
        strErrText.Append(mecvExcept.strSrcClassPath());
        strErrText.Append(L", LINE: ");
        strErrText.AppendFormatted(mecvExcept.c4LineNum());
        strErrText.Append(L", ERR: ");
        strErrText.Append(mecvExcept.strErrorText());

        //
        //  We either use the current class/line or the one passed if it's a
        //  'P' type call.
        //
        const tCIDLib::TBoolean bPType(c2MethId == m_c2MethId_LogExceptP);
        TError errToLog
        (
            facCIDMacroEng().strName()
            , bPType ? meOwner.strClassPathForId(mecvInstance.c2ClassId())
                     : meOwner.strStackValAt(c4FirstInd + 2)
            , bPType ? mecvExcept.c4LineNum() : meOwner.c4StackValAt(c4FirstInd + 3)
            , mecvExcept.c4ErrorNum()
            , meOwner.strStackValAt(c4FirstInd)
            , strErrText
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
        TModule::LogEventObj(errToLog);
    }
     else if ((c2MethId == m_c2MethId_LogMsg)
          ||  (c2MethId == m_c2MethId_LogMsgP)
          ||  (c2MethId == m_c2MethId_LogMsg1)
          ||  (c2MethId == m_c2MethId_LogMsg1P)
          ||  (c2MethId == m_c2MethId_LogMsg2)
          ||  (c2MethId == m_c2MethId_LogMsg2P)
          ||  (c2MethId == m_c2MethId_LogMsg3)
          ||  (c2MethId == m_c2MethId_LogMsg3P))
    {
        //
        //  Get the message to a temp first. We may have to do token
        //  replacement here. We have a helper to do this and he figures
        //  out if there are any and how many there are.
        //
        m_strMsg = meOwner.strStackValAt(c4FirstInd);
        if ((c2MethId != m_c2MethId_LogMsg)
        &&  (c2MethId != m_c2MethId_LogMsgP))
        {
            ReplaceTokens
            (
                meOwner
                , m_strMsg
                , c2MethId
                , c4FirstInd + 1
            );
        }

        //
        //  For one set we use our own class path and line number. For the
        //  other we take the passed ones.
        //
        if ((c2MethId == m_c2MethId_LogMsg)
        ||  (c2MethId == m_c2MethId_LogMsg1)
        ||  (c2MethId == m_c2MethId_LogMsg2)
        ||  (c2MethId == m_c2MethId_LogMsg3))
        {
            TError errToLog
            (
                facCQCKit().strName()
                , meOwner.strClassPathForId(mecvInstance.c2ClassId())
                , meOwner.c4CurLine()
                , m_strMsg
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );
            TModule::LogEventObj(errToLog);
        }
         else
        {
            //
            //  The class/line number values come after the tokens, so
            //  we have to adjust for that to get to the right params.
            //
            tCIDLib::TCard4 c4ClsOfs;
            if (c2MethId == m_c2MethId_LogMsgP)
                c4ClsOfs = 1;
            else if (c2MethId == m_c2MethId_LogMsg1P)
                c4ClsOfs = 2;
            else if (c2MethId == m_c2MethId_LogMsg2P)
                c4ClsOfs = 3;
            else if (c2MethId == m_c2MethId_LogMsg3P)
                c4ClsOfs = 4;
            else
            {
                CIDAssert2(L"Invalid method id in CML logger class");
            }

            TError errToLog
            (
                facCQCKit().strName()
                , meOwner.strStackValAt(c4FirstInd + c4ClsOfs)
                , meOwner.c4StackValAt(c4FirstInd + c4ClsOfs + 1)
                , m_strMsg
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );
            TModule::LogEventObj(errToLog);
        }
    }
     else
    {
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


// ---------------------------------------------------------------------------
//  TCQCLoggerInfo: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TCQCLoggerInfo::ReplaceTokens(          TCIDMacroEngine&    meOwner
                                ,       TString&            strMsg
                                , const tCIDLib::TCard2     c2MethId
                                , const tCIDLib::TCard4     c4FirstInd)
{
    // Get the actual C++ stream out as it's real type
    TTextStringOutStream& strmActual = static_cast<TTextStringOutStream&>
    (
        m_pmecvStrm->strmTarget(meOwner)
    );

    // Figure out how many tokens
    tCIDLib::TCard4 c4Tokens = 0;
    if ((c2MethId == m_c2MethId_LogMsg1) || (c2MethId == m_c2MethId_LogMsg1P))
        c4Tokens = 1;
    else if ((c2MethId == m_c2MethId_LogMsg2) || (c2MethId == m_c2MethId_LogMsg2P))
        c4Tokens = 2;
    else if ((c2MethId == m_c2MethId_LogMsg3) || (c2MethId == m_c2MethId_LogMsg3P))
        c4Tokens = 3;

    tCIDLib::TCh chToken(kCIDLib::chDigit1);
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Tokens; c4Index++)
    {
        // Reset the output stream for this round
        strmActual.Reset();

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
        meOwner.PushValue(m_pmecvStrm, tCIDMacroEng::EStackItems::Parm, kCIDLib::True);
        meOwner.meciPushMethodCall
        (
            meciTarget.c2Id()
            , TMEngFormattableInfo::c2FormatToId()
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
        strmActual.Flush();
        strMsg.eReplaceToken(strmActual.strData(), chToken);

        // Move up to the next token digit
        chToken++;
    }
}



