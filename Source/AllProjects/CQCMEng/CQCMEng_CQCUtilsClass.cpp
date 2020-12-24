//
// FILE NAME: CQCMEng_CQCUtilsClass.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/17/2007
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
//  This file implements the info class to create a macro level class which
//  supports a grab bag of utility methods. It doesn't have any data, so we
//  just use the standard value class.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCMEng_.hpp"
#include    "CQCMEng_CQCUtilsClass_.hpp"


// ---------------------------------------------------------------------------
//  Magic RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCUtilsInfo,TMEngClassInfo)


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace CQCMacroEng_CQCUtils
{
    namespace
    {
        // -----------------------------------------------------------------------
        //  The names for the types that we support here. Each derivative has to
        //  be able to return strings that contain its name and full name.
        // -----------------------------------------------------------------------
        const TString   strCQCUtils(L"CQCUtils");
        const TString   strCQCUtilsClassPath(L"MEng.System.CQC.Runtime.CQCUtils");
        const TString   strCQCUtilsBasePath(L"MEng.System.CQC.Runtime");
    }
}



// ---------------------------------------------------------------------------
//  CLASS: TCQCUtilsInfo
// PREFIX: meci
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCUtilsInfo: Public, static methods
// ---------------------------------------------------------------------------
const TString& TCQCUtilsInfo::strClassPath()
{
    return CQCMacroEng_CQCUtils::strCQCUtilsClassPath;
}


// ---------------------------------------------------------------------------
//  TCQCUtilsInfo: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCUtilsInfo::TCQCUtilsInfo(TCIDMacroEngine& meOwner, const TCQCSecToken& sectUser) :

    TMEngClassInfo
    (
        CQCMacroEng_CQCUtils::strCQCUtils
        , CQCMacroEng_CQCUtils::strCQCUtilsBasePath
        , meOwner
        , kCIDLib::False
        , tCIDMacroEng::EClassExt::Final
        , L"MEng.Object"
    )
    , m_c2EnumId_Errors(kCIDMacroEng::c2BadId)
    , m_c2MethId_CalcGeoDistance(kCIDMacroEng::c2BadId)
    , m_c2MethId_DefCtor(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetLocInfo(kCIDMacroEng::c2BadId)
    , m_c2MethId_SendEMail(kCIDMacroEng::c2BadId)
    , m_c2MethId_SendHTMLEMail(kCIDMacroEng::c2BadId)
    , m_c2MethId_SendUserAct(kCIDMacroEng::c2BadId)
    , m_c2MethId_StartRemApp(kCIDMacroEng::c2BadId)
    , m_c4ErrOpFailed(kCIDLib::c4MaxCard)
    , m_pmeciErrors(nullptr)
    , m_sectUser(sectUser)
{
}

TCQCUtilsInfo::~TCQCUtilsInfo()
{
}


// ---------------------------------------------------------------------------
//  TCQCUtilsInfo: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCUtilsInfo::Init(TCIDMacroEngine& meOwner)
{
    // Create an enumerated type for our errors
    {
        m_pmeciErrors = new TMEngEnumInfo
        (
            meOwner
            , L"CQCUtilsErrs"
            , strClassPath()
            , L"MEng.Enum"
            , 3
        );
        m_c4ErrACSNotFound = m_pmeciErrors->c4AddEnumItem(L"ACSNotFound", L"App Ctrl Server '%(1)' was not found");
        m_c4ErrNoLocInfo = m_pmeciErrors->c4AddEnumItem(L"NoLocInfo", L"The location information could not be accessed");
        m_c4ErrOpFailed = m_pmeciErrors->c4AddEnumItem(L"OpFailed", TString::strEmpty());
        m_pmeciErrors->BaseClassInit(meOwner);
        m_c2EnumId_Errors = meOwner.c2AddClass(m_pmeciErrors);
        bAddNestedType(m_pmeciErrors->strClassPath());
    }

    // Add the default constructor
    {
        TMEngMethodInfo methiNew
        (
            L"ctor1_MEng.System.CQC.Runtime.CQCUtils"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.bIsCtor(kCIDLib::True);
        m_c2MethId_DefCtor = c2AddMethodInfo(methiNew);
    }

    // Calc the distance between two lat/long values
    {
        TMEngMethodInfo methiNew
        (
            L"CalcGeoDistance"
            , tCIDMacroEng::EIntrinsics::Card4
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInParm(L"Lat1", tCIDMacroEng::EIntrinsics::Float8);
        methiNew.c2AddInParm(L"Long1", tCIDMacroEng::EIntrinsics::Float8);
        methiNew.c2AddInParm(L"Lat2", tCIDMacroEng::EIntrinsics::Float8);
        methiNew.c2AddInParm(L"Long2", tCIDMacroEng::EIntrinsics::Float8);
        methiNew.c2AddInParm(L"InMiles", tCIDMacroEng::EIntrinsics::Boolean);
        m_c2MethId_CalcGeoDistance = c2AddMethodInfo(methiNew);
    }

    // Get the system lat/long info from the config server on the MS
    {
        TMEngMethodInfo methiNew
        (
            L"GetLatLong"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddOutParm(L"Lat", tCIDMacroEng::EIntrinsics::Float8);
        methiNew.c2AddOutParm(L"Long", tCIDMacroEng::EIntrinsics::Float8);
        m_c2MethId_GetLocInfo = c2AddMethodInfo(methiNew);
    }

    // Send an e-mail via the standard CQC e-mail accounts
    {
        TMEngMethodInfo methiNew
        (
            L"SendEMail"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInParm(L"AcctName", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"ToAddr", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"Subject", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"Message", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_SendEMail = c2AddMethodInfo(methiNew);
    }

    // Same as above but with HTML content
    {
        TMEngMethodInfo methiNew
        (
            L"SendHTMLEMail"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInParm(L"AcctName", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"ToAddr", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"Subject", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"Message", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_SendHTMLEMail = c2AddMethodInfo(methiNew);
    }

    // Fake a user action event trigger
    {
        TMEngMethodInfo methiNew
        (
            L"SendUserAction"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInParm(L"Moniker", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"Type", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"Data", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_SendUserAct = c2AddMethodInfo(methiNew);
    }

    // Start an app remotely via the App Control Server
    {
        TMEngMethodInfo methiNew
        (
            L"StartRemApp"
            , tCIDMacroEng::EIntrinsics::Boolean
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInParm(L"Binding", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"RemPath", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"Parms", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"InitPath", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_StartRemApp = c2AddMethodInfo(methiNew);
    }
}


TMEngClassVal*
TCQCUtilsInfo::pmecvMakeStorage(const   TString&               strName
                                ,       TCIDMacroEngine&
                                , const tCIDMacroEng::EConstTypes eConst) const
{
    return new TMEngStdClassVal(strName, c2Id(), eConst);
}



// ---------------------------------------------------------------------------
//  TCQCUtilsInfo: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TCQCUtilsInfo::bInvokeMethod(       TCIDMacroEngine&    meOwner
                            , const TMEngMethodInfo&    methiTarget
                            ,       TMEngClassVal&      mecvInstance)
{
    // Don't need this so far
//    TCQCUtilsVal& mecvActual = static_cast<TCQCUtilsVal&>(mecvInstance);

    const tCIDLib::TCard4 c4FirstInd = meOwner.c4FirstParmInd(methiTarget);
    const tCIDLib::TCard2 c2MethId   = methiTarget.c2Id();

    if (c2MethId == m_c2MethId_DefCtor)
    {
        // Nothing to do
    }
     else if (c2MethId == m_c2MethId_CalcGeoDistance)
    {
        try
        {
            const tCIDLib::TCard4 c4Dist = TMathLib::c4GeoDistance
            (
                meOwner.f8StackValAt(c4FirstInd)
                , meOwner.f8StackValAt(c4FirstInd + 1)
                , meOwner.f8StackValAt(c4FirstInd + 2)
                , meOwner.f8StackValAt(c4FirstInd + 3)
                , meOwner.bStackValAt(c4FirstInd + 4)
            );

            // Put the result in the return value
            TMEngCard4Val& mecvRet = meOwner.mecvStackAtAs<TMEngCard4Val>(c4FirstInd - 1);
            mecvRet.c4Value(c4Dist);
        }

        catch(const TError& errToCatch)
        {
            ThrowAnErr(meOwner, m_c4ErrOpFailed, errToCatch);
        }
    }
     else if (c2MethId == m_c2MethId_GetLocInfo)
    {
        try
        {
            tCIDLib::TFloat8 f8Lat = 0;
            tCIDLib::TFloat8 f8Long = 0;
            if (!facCQCKit().bGetLocationInfo(tCIDLib::ECSSides::Server, f8Lat, f8Long, kCIDLib::False, m_sectUser))
                ThrowAnErr(meOwner, m_c4ErrNoLocInfo);

            // We got it, so get the output parms and fill them in
            meOwner.mecvStackAtAs<TMEngFloat8Val>(c4FirstInd).f8Value(f8Lat);
            meOwner.mecvStackAtAs<TMEngFloat8Val>(c4FirstInd + 1).f8Value(f8Long);
        }

        catch(const TError& errToCatch)
        {
            ThrowAnErr(meOwner, m_c4ErrNoLocInfo, errToCatch);
        }
    }
     else if ((c2MethId == m_c2MethId_SendEMail)
          ||  (c2MethId == m_c2MethId_SendHTMLEMail))
    {
        try
        {
            facCQCKit().SendEMail
            (
                meOwner.strStackValAt(c4FirstInd)
                , meOwner.strStackValAt(c4FirstInd + 1)
                , meOwner.strStackValAt(c4FirstInd + 2)
                , meOwner.strStackValAt(c4FirstInd + 3)
                , (c2MethId == m_c2MethId_SendHTMLEMail)
                , m_sectUser
            );
        }

        catch(const TError& errToCatch)
        {
            ThrowAnErr(meOwner, m_c4ErrOpFailed, errToCatch);
        }
    }
     else if (c2MethId == m_c2MethId_StartRemApp)
    {
        try
        {
            // Build up the full binding path, based on the name we get
            TString strBinding(TAppCtrlClientProxy::strImplBinding);
            strBinding.eReplaceToken(meOwner.strStackValAt(c4FirstInd), kCIDLib::chLatin_b);

            //
            //  Look up the provided binding in the name server. If not found,
            //  then throw.
            //
            TAppCtrlClientProxy* porbcProxy
            (
                facCIDOrbUC().porbcMakeClient<TAppCtrlClientProxy>(strBinding)
            );

            if (!porbcProxy)
                ThrowAnErr(meOwner, m_c4ErrACSNotFound, meOwner.strStackValAt(c4FirstInd));

            // That worked, so invoke it and store the result
            TJanitor<TAppCtrlClientProxy> janProxy(porbcProxy);

            const TString& strAppPath = meOwner.strStackValAt(c4FirstInd + 1);
            const TString& strParms = meOwner.strStackValAt(c4FirstInd + 2);
            const TString& strInitPath = meOwner.strStackValAt(c4FirstInd + 3);
            TMEngBooleanVal& mecvRet = meOwner.mecvStackAtAs<TMEngBooleanVal>(c4FirstInd - 1);
            mecvRet.bValue(porbcProxy->bStartApp(strAppPath, strParms, strInitPath));
        }

        catch(const TError& errToCatch)
        {
            ThrowAnErr(meOwner, m_c4ErrOpFailed, errToCatch);
        }
    }
     else if (c2MethId == m_c2MethId_SendUserAct)
    {
        try
        {
            facCQCKit().QueueStdEventTrig
            (
                tCQCKit::EStdDrvEvs::UserAction
                , meOwner.strStackValAt(c4FirstInd)
                , TString::strEmpty()
                , meOwner.strStackValAt(c4FirstInd + 1)
                , meOwner.strStackValAt(c4FirstInd + 2)
                , TString::strEmpty()
                , TString::strEmpty()
            );
        }

        catch(const TError& errToCatch)
        {
            ThrowAnErr(meOwner, m_c4ErrOpFailed, errToCatch);
        }
    }
     else
    {
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


// ---------------------------------------------------------------------------
//  TCQCUtilsInfo: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TCQCUtilsInfo::ThrowAnErr(          TCIDMacroEngine&    meOwner
                            , const tCIDLib::TCard4     c4ToThrow
                            , const TError&             errCaught) const
{
    if (facCQCKit().bShouldLog(errCaught))
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

tCIDLib::TVoid
TCQCUtilsInfo::ThrowAnErr(          TCIDMacroEngine&    meOwner
                            , const tCIDLib::TCard4     c4ToThrow) const
{
    TString strErrText(m_pmeciErrors->strTextValue(c4ToThrow));

    // Set the exception info on the engine
    meOwner.SetException
    (
        m_c2EnumId_Errors
        , strClassPath()
        , c4ToThrow
        , m_pmeciErrors->strPartialName(c4ToThrow)
        , strErrText
        , meOwner.c4CurLine()
    );

    // And throw the excpetion that represents a macro level exception
    throw TExceptException();
}


tCIDLib::TVoid
TCQCUtilsInfo::ThrowAnErr(          TCIDMacroEngine&    meOwner
                            , const tCIDLib::TCard4     c4ToThrow
                            , const MFormattable&       fmtblToken1) const
{
    TString strErrText(m_pmeciErrors->strTextValue(c4ToThrow));
    strErrText.eReplaceToken(fmtblToken1, kCIDLib::chDigit1);

    // Set the exception info on the engine
    meOwner.SetException
    (
        m_c2EnumId_Errors
        , strClassPath()
        , c4ToThrow
        , m_pmeciErrors->strPartialName(c4ToThrow)
        , strErrText
        , meOwner.c4CurLine()
    );

    // And throw the excpetion that represents a macro level exception
    throw TExceptException();
}


