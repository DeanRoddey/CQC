//
// FILE NAME: CQCMacroEngS_DriverClass.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/12/2003
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
//  We have to bring in some otherwise private CML class headers from the
//  CIDMacroEng and CQCKit facilities, so that we can directly create CML objects
//  of these types in C++.
// ---------------------------------------------------------------------------
#include    "CIDMacroEng_CommClasses_.hpp"
#include    "CIDMacroEng_SockClasses_.hpp"
#include    "CQCMEng_FieldDefClass_.hpp"
#include    "CQCMEng_V2HelperClass_.hpp"


// ---------------------------------------------------------------------------
//  Magic RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCDriverGlueVal, TMEngClassVal)
RTTIDecls(TCQCDriverGlueInfo, TMEngClassInfo)
RTTIDecls(TCQCDriverInfo, TMEngClassInfo)


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace
{
    namespace CQCGenDrv_DriverClass
    {
        // -----------------------------------------------------------------------
        //  The names for the types that we support here. Each derivative has to
        //  be able to return strings that contain its name and full name.
        // -----------------------------------------------------------------------
        const TString   strBasePath(L"MEng.System.CQC.Runtime");

        const TString   strGlueName(L"CQCDriverGlue");
        const TString   strGlueClassPath(L"MEng.System.CQC.Runtime.CQCDriverGlue");

        const TString   strDriverName(L"CQCDriverBase");
        const TString   strDriverClassPath(L"MEng.System.CQC.Runtime.CQCDriverBase");
        const TString   strCommResClassPath(L"MEng.System.CQC.Runtime.CQCDriverBase.CommResults");
        const TString   strInitResClassPath(L"MEng.System.CQC.Runtime.CQCDriverBase.DrvInitRes");
        const TString   strVerbLevelClassPath(L"MEng.System.CQC.Runtime.CQCDriverBase.VerboseLvls");
    }
}




// ---------------------------------------------------------------------------
//  CLASS: TCQCDriverGlueVal
// PREFIX: mecv
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCDriverGlueVal: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCDriverGlueVal::TCQCDriverGlueVal(const  TString&                strName
                                    , const tCIDLib::TCard2         c2ClassId
                                    , const tCIDMacroEng::EConstTypes  eConst) :

    TMEngClassVal(strName, c2ClassId, eConst)
    , m_psdrvImpl(0)
{
}

TCQCDriverGlueVal::~TCQCDriverGlueVal()
{
    // We don't own the impl object!
}


// ---------------------------------------------------------------------------
//  TCQCDriverGlueVal: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TCQCDriverGlueVal::bDbgFormat(          TTextOutStream&
                                , const TMEngClassInfo&
                                , const tCIDMacroEng::EDbgFmts
                                , const tCIDLib::ERadices
                                , const TCIDMacroEngine&) const
{
    return kCIDLib::False;
}


// ---------------------------------------------------------------------------
//  TCQCDriverGlueVal: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCDriverGlueVal::Initialize(TCQCGenDrvGlue* const psdrvImpl)
{
    m_psdrvImpl = psdrvImpl;
}


TCQCGenDrvGlue& TCQCDriverGlueVal::sdrvImpl()
{
    return *m_psdrvImpl;
}

const TCQCGenDrvGlue& TCQCDriverGlueVal::sdrvImpl() const
{
    return *m_psdrvImpl;
}



// ---------------------------------------------------------------------------
//  CLASS: TCQCDriverGlueInfo
// PREFIX: meci
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCDriverGlueInfo: Public, static methods
// ---------------------------------------------------------------------------
const TString& TCQCDriverGlueInfo::strPath()
{
    return CQCGenDrv_DriverClass::strGlueClassPath;
}


// ---------------------------------------------------------------------------
//  TCQCDriverGlueInfo: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCDriverGlueInfo::TCQCDriverGlueInfo(TCIDMacroEngine& meOwner) :

    TMEngClassInfo
    (
        CQCGenDrv_DriverClass::strGlueName
        , CQCGenDrv_DriverClass::strBasePath
        , meOwner
        , kCIDLib::False
        , tCIDMacroEng::EClassExt::Final
        , L"MEng.Object"
    )
    , m_c2MethId_DefCtor(kCIDMacroEng::c2BadId)
{
}

TCQCDriverGlueInfo::~TCQCDriverGlueInfo()
{
}


// ---------------------------------------------------------------------------
//  TCQCDriverGlueInfo: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCDriverGlueInfo::Init(TCIDMacroEngine&)
{
    // Add the default constructor
    {
        TMEngMethodInfo methiNew
        (
            L"ctor1_MEng.System.CQC.Runtime.CQCDriverGlue"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.bIsCtor(kCIDLib::True);
        m_c2MethId_DefCtor = c2AddMethodInfo(methiNew);
    }
}


TMEngClassVal*
TCQCDriverGlueInfo::pmecvMakeStorage(const  TString&                strName
                                    ,       TCIDMacroEngine&
                                    , const tCIDMacroEng::EConstTypes  eConst) const
{
    return new TCQCDriverGlueVal(strName, c2Id(), eConst);
}


// ---------------------------------------------------------------------------
//  TCQCDriverGlueInfo: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TCQCDriverGlueInfo::bInvokeMethod(          TCIDMacroEngine&
                                    , const TMEngMethodInfo&    methiTarget
                                    ,       TMEngClassVal&      )
{
//    TCQCDriverGlueVal& mecvActual  = static_cast<TCQCDriverGlueVal&>(mecvInstance);
    const tCIDLib::TCard2 c2MethId = methiTarget.c2Id();

    if (c2MethId == m_c2MethId_DefCtor)
    {
        // Nothing to do. We are initialized at the C++ level
    }
     else
    {
        return kCIDLib::False;
    }
    return kCIDLib::True;
}




// ---------------------------------------------------------------------------
//  CLASS: TCQCDriverInfo
// PREFIX: meci
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCDriverInfo: Public, static methods
// ---------------------------------------------------------------------------
const TString& TCQCDriverInfo::strPath()
{
    return CQCGenDrv_DriverClass::strDriverClassPath;
}

const TString& TCQCDriverInfo::strCommResPath()
{
    return CQCGenDrv_DriverClass::strCommResClassPath;
}

const TString& TCQCDriverInfo::strInitResPath()
{
    return CQCGenDrv_DriverClass::strInitResClassPath;
}

const TString& TCQCDriverInfo::strVerbLevelPath()
{
    return CQCGenDrv_DriverClass::strVerbLevelClassPath;
}


// ---------------------------------------------------------------------------
//  TCQCDriverInfo: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCDriverInfo::TCQCDriverInfo(TCIDMacroEngine& meOwner) :

    TMEngClassInfo
    (
        CQCGenDrv_DriverClass::strDriverName
        , CQCGenDrv_DriverClass::strBasePath
        , meOwner
        , kCIDLib::False
        , tCIDMacroEng::EClassExt::NonFinal
        , L"MEng.Object"
    )
    , m_c2EnumId_CommResults(kCIDMacroEng::c2BadId)
    , m_c2EnumId_Errors(kCIDMacroEng::c2BadId)
    , m_c2EnumId_InitRes(kCIDMacroEng::c2BadId)
    , m_c2EnumId_VerboseLevel(kCIDMacroEng::c2BadId)
    , m_c2EnumId_ZoneStates(kCIDMacroEng::c2BadId)
    , m_c2MemId_Glue(kCIDMacroEng::c2BadId)
    , m_c2MethId_ClientCmd(kCIDMacroEng::c2BadId)
    , m_c2MethId_Connect(kCIDMacroEng::c2BadId)
    , m_c2MethId_DefCtor(kCIDMacroEng::c2BadId)
    , m_c2MethId_DelConfigStr(kCIDMacroEng::c2BadId)
    , m_c2MethId_FldChBool(kCIDMacroEng::c2BadId)
    , m_c2MethId_FldChCard(kCIDMacroEng::c2BadId)
    , m_c2MethId_FldChFloat(kCIDMacroEng::c2BadId)
    , m_c2MethId_FldChInt(kCIDMacroEng::c2BadId)
    , m_c2MethId_FldChString(kCIDMacroEng::c2BadId)
    , m_c2MethId_FldChStrList(kCIDMacroEng::c2BadId)
    , m_c2MethId_FldChTime(kCIDMacroEng::c2BadId)
    , m_c2MethId_FldIdFromName(kCIDMacroEng::c2BadId)
    , m_c2MethId_FindFldIdCl(kCIDMacroEng::c2BadId)
    , m_c2MethId_FindFldIdSub(kCIDMacroEng::c2BadId)
    , m_c2MethId_FindFldIdPNS(kCIDMacroEng::c2BadId)
    , m_c2MethId_FreeCommRes(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetArchVer(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetASCIITermMsg(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetASCIITermMsg2(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetASCIIStartStopMsg(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetASCIIStartStopMsg2(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetCommRes(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetConfigStr(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetDriverInfo(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetFldErrState(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetFldInfoById(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetFldInfoByName(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetFldName(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetIsConnected(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetMoniker(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetStartLenMsg(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetStartLenMsg2(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetStartStopMsg(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetStartStopMsg2(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetTermedMsg(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetTermedMsg2(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetVerboseLevel(kCIDMacroEng::c2BadId)
    , m_c2MethId_FormatStrList(kCIDMacroEng::c2BadId)
    , m_c2MethId_IncBadMsgs(kCIDMacroEng::c2BadId)
    , m_c2MethId_IncFailedWrite(kCIDMacroEng::c2BadId)
    , m_c2MethId_IncNaks(kCIDMacroEng::c2BadId)
    , m_c2MethId_IncReconfigured(kCIDMacroEng::c2BadId)
    , m_c2MethId_IncTimeouts(kCIDMacroEng::c2BadId)
    , m_c2MethId_IncUnknownMsgs(kCIDMacroEng::c2BadId)
    , m_c2MethId_IncUnknownWrite(kCIDMacroEng::c2BadId)
    , m_c2MethId_InitOther(kCIDMacroEng::c2BadId)
    , m_c2MethId_InitSerial(kCIDMacroEng::c2BadId)
    , m_c2MethId_InitSocket(kCIDMacroEng::c2BadId)
    , m_c2MethId_LogMsg(kCIDMacroEng::c2BadId)
    , m_c2MethId_LogMsg1(kCIDMacroEng::c2BadId)
    , m_c2MethId_LogMsg2(kCIDMacroEng::c2BadId)
    , m_c2MethId_LogMsg3(kCIDMacroEng::c2BadId)
    , m_c2MethId_Poll(kCIDMacroEng::c2BadId)
    , m_c2MethId_QueryBoolVal(kCIDMacroEng::c2BadId)
    , m_c2MethId_QueryBufVal(kCIDMacroEng::c2BadId)
    , m_c2MethId_QueryCardVal(kCIDMacroEng::c2BadId)
    , m_c2MethId_QueryIntVal(kCIDMacroEng::c2BadId)
    , m_c2MethId_QueryTextVal(kCIDMacroEng::c2BadId)
    , m_c2MethId_ReadBoolFld(kCIDMacroEng::c2BadId)
    , m_c2MethId_ReadCardFld(kCIDMacroEng::c2BadId)
    , m_c2MethId_ReadFloatFld(kCIDMacroEng::c2BadId)
    , m_c2MethId_ReadIntFld(kCIDMacroEng::c2BadId)
    , m_c2MethId_ReadStringFld(kCIDMacroEng::c2BadId)
    , m_c2MethId_ReadStrListFld(kCIDMacroEng::c2BadId)
    , m_c2MethId_ReadTimeFld(kCIDMacroEng::c2BadId)
    , m_c2MethId_SendLoadEv(kCIDMacroEng::c2BadId)
    , m_c2MethId_SendLoadEv2(kCIDMacroEng::c2BadId)
    , m_c2MethId_SendLockEv(kCIDMacroEng::c2BadId)
    , m_c2MethId_SendMotionEv(kCIDMacroEng::c2BadId)
    , m_c2MethId_SendMotionEv2(kCIDMacroEng::c2BadId)
    , m_c2MethId_SendPresenceEv(kCIDMacroEng::c2BadId)
    , m_c2MethId_SendUserActEv(kCIDMacroEng::c2BadId)
    , m_c2MethId_SendZoneEv(kCIDMacroEng::c2BadId)
    , m_c2MethId_SendZoneEv2(kCIDMacroEng::c2BadId)
    , m_c2MethId_SetAllErrStates(kCIDMacroEng::c2BadId)
    , m_c2MethId_SetConfigStr(kCIDMacroEng::c2BadId)
    , m_c2MethId_SetFields(kCIDMacroEng::c2BadId)
    , m_c2MethId_SetFldErrState(kCIDMacroEng::c2BadId)
    , m_c2MethId_SetPollTimes(kCIDMacroEng::c2BadId)
    , m_c2MethId_SetTOExtension(kCIDMacroEng::c2BadId)
    , m_c2MethId_SetVerboseLevel(kCIDMacroEng::c2BadId)
    , m_c2MethId_Terminate(kCIDMacroEng::c2BadId)
    , m_c2MethId_Simulate(kCIDMacroEng::c2BadId)
    , m_c2MethId_UseDefFldValue(kCIDMacroEng::c2BadId)
    , m_c2MethId_VerbosityChanged(kCIDMacroEng::c2BadId)
    , m_c2MethId_WaitConfig(kCIDMacroEng::c2BadId)
    , m_c2MethId_WriteBool(kCIDMacroEng::c2BadId)
    , m_c2MethId_WriteBoolByName(kCIDMacroEng::c2BadId)
    , m_c2MethId_WriteCard(kCIDMacroEng::c2BadId)
    , m_c2MethId_WriteCardByName(kCIDMacroEng::c2BadId)
    , m_c2MethId_WriteFloat(kCIDMacroEng::c2BadId)
    , m_c2MethId_WriteFloatByName(kCIDMacroEng::c2BadId)
    , m_c2MethId_WriteInt(kCIDMacroEng::c2BadId)
    , m_c2MethId_WriteIntByName(kCIDMacroEng::c2BadId)
    , m_c2MethId_WriteString(kCIDMacroEng::c2BadId)
    , m_c2MethId_WriteStringByName(kCIDMacroEng::c2BadId)
    , m_c2MethId_WriteStrList(kCIDMacroEng::c2BadId)
    , m_c2MethId_WriteStrListByName(kCIDMacroEng::c2BadId)
    , m_c2MethId_WriteTime(kCIDMacroEng::c2BadId)
    , m_c2MethId_WriteTimeByName(kCIDMacroEng::c2BadId)
    , m_c2TypeId_CommPort(kCIDMacroEng::c2BadId)
    , m_c2TypeId_FldList(kCIDMacroEng::c2BadId)
    , m_c2TypeId_StreamSocket(kCIDMacroEng::c2BadId)
    , m_c4ErrAccessFailed(kCIDLib::c4MaxCard)
    , m_c4ErrAlreadyConfiged(kCIDLib::c4MaxCard)
    , m_c4ErrAlreadyConnected(kCIDLib::c4MaxCard)
    , m_c4ErrBadFldType(kCIDLib::c4MaxCard)
    , m_c4ErrConvertErr(kCIDLib::c4MaxCard)
    , m_c4ErrCppExcept(kCIDLib::c4MaxCard)
    , m_c4ErrDelCfgStr(kCIDLib::c4MaxCard)
    , m_c4ErrFldNotFound(kCIDLib::c4MaxCard)
    , m_c4ErrFldWriteErr(kCIDLib::c4MaxCard)
    , m_c4ErrGetCfgStr(kCIDLib::c4MaxCard)
    , m_c4ErrGetMsg(kCIDLib::c4MaxCard)
    , m_c4ErrLimitViolation(kCIDLib::c4MaxCard)
    , m_c4ErrNotConnected(kCIDLib::c4MaxCard)
    , m_c4ErrNoOverride(kCIDLib::c4MaxCard)
    , m_c4ErrSetCfgStr(kCIDLib::c4MaxCard)
    , m_c4ErrUnknownConnType(kCIDLib::c4MaxCard)
    , m_c4ErrUnknownExcept(kCIDLib::c4MaxCard)
    , m_c4ErrFldRegFailed(kCIDLib::c4MaxCard)
    , m_c4ErrFldIdNotFound(kCIDLib::c4MaxCard)
    , m_c4ErrSetVerbErr(kCIDLib::c4MaxCard)
    , m_pmeciCommResults(nullptr)
    , m_pmeciErrors(nullptr)
    , m_pmeciInitRes(nullptr)
    , m_pmeciVerboseLevel(nullptr)
    , m_pmeciZoneStates(nullptr)
    , m_pmecvStrm(nullptr)
{
    //
    //  Add our info class to our list of imports so that it will be loaded
    //  by the engine before we get initialized and need it. Also, import
    //  the CQCFldDef class, which we reference in our methods below. And
    //  import the comm resource class as well. And we need the comm port
    //  config class and the socket end point classes, which are used in
    //  parms of the abstract init methods.
    //
    bAddClassImport(TMEngCommCfgInfo::strPath());
    bAddClassImport(TMEngCommPortInfo::strPath());
    bAddClassImport(TMEngIPEPInfo::strPath());
    bAddClassImport(TCQCFldDefInfo::strClassPath());
    bAddClassImport(TMEngCommPortInfo::strPath());
    bAddClassImport(TMEngStreamSocketInfo::strPath());
    bAddClassImport(TMEngNamedValMapInfo::strPath());
    bAddClassImport(CQCGenDrv_DriverClass::strGlueClassPath);
    bAddClassImport(TV2HelperInfo::strClassPath());
}

TCQCDriverInfo::~TCQCDriverInfo()
{
    // Clean up our output stream
    try
    {
        delete m_pmecvStrm;
    }

    catch(const TError& errToCatch)
    {
        if (facCQCGenDrvS().bShouldLog(errToCatch))
            TModule::LogEventObj(errToCatch);
    }
}


// ---------------------------------------------------------------------------
//  TCQCDriverInfo: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCDriverInfo::Init(TCIDMacroEngine& meOwner)
{
    //
    //  Add our data members. We support derivation by macro level classes,
    //  so we must use macro level data storage. We add a member of our
    //  info object so that it will be available in subsequent calls.
    //
    {
        const TMEngClassInfo& meciInfo = meOwner.meciFind
        (
            CQCGenDrv_DriverClass::strGlueClassPath
        );

        m_c2MemId_Glue = c2AddMember
        (
            TMEngMemberInfo
            (
                L"m_GlueInfo"
                , meciInfo.c2Id()
                , tCIDMacroEng::EConstTypes::NonConst
            )
            , meciInfo
        );
    }

    // Get some types that we need to use in our interface and in our methods
    const tCIDLib::TCard2 c2FldDefId
    (
        meOwner.c2FindClassId(TCQCFldDefInfo::strClassPath())
    );

    m_pmeciDevClasses = static_cast<TMEngEnumInfo*>
    (
        meOwner.pmeciFind(TV2HelperInfo::strDevClassPath())
    );

    //
    //  Create a nested class which is a vector of field info objects. We
    //  need this as a parameter to the field setting method that this
    //  macro class provides to it's derived driver classes. It's base
    //  class is our classpath. We also have to add it as a nested class
    //  name in our class, so that the loader will load it when someone
    //  imports us. And we have to add an import to our class for it.
    //
    {
        TMEngVectorInfo* pmeciNew = new TMEngVectorInfo
        (
            meOwner
            , L"CQCFieldList"
            , CQCGenDrv_DriverClass::strDriverClassPath
            , TMEngVectorInfo::strPath()
            , c2FldDefId
        );
        TJanitor<TMEngVectorInfo> janNewClass(pmeciNew);
        pmeciNew->BaseClassInit(meOwner);
        bAddNestedType(pmeciNew->strClassPath());
        m_c2TypeId_FldList = meOwner.c2AddClass(janNewClass.pobjOrphan());
    }

    // Create an enum for our errors that we throw
    {
        m_pmeciErrors = new TMEngEnumInfo
        (
            meOwner, L"CQCDrvErrors", strClassPath(), L"MEng.Enum", 24
        );
        m_c4ErrAlreadyConfiged = m_pmeciErrors->c4AddEnumItem(L"AlreadyConfigured", L"It is too late to change the driver configuration");
        m_c4ErrAlreadyConnected = m_pmeciErrors->c4AddEnumItem(L"AlreadyConnected", L"You cannot %(1) after the driver is connected");
        m_c4ErrBadFldType = m_pmeciErrors->c4AddEnumItem(L"BadFldType", L"Field %(1) was accessed as a type different from its defined type");
        m_c4ErrConvertErr = m_pmeciErrors->c4AddEnumItem(L"ConvertErr", L"The value written to field %(1) could not be converted to the field's type");
        m_c4ErrCppExcept = m_pmeciErrors->c4AddEnumItem(L"CppExcept", L"An unhandled CIDLib C++ exception occured");
        m_c4ErrDelCfgStr = m_pmeciErrors->c4AddEnumItem(L"DelCfgString", TString::strEmpty());
        m_c4ErrFldNotFound = m_pmeciErrors->c4AddEnumItem(L"FldNotFound", L"Field '%(1)' was not found");
        m_c4ErrFldWriteErr = m_pmeciErrors->c4AddEnumItem(L"FldWriteErr", TString::strEmpty());
        m_c4ErrGetCfgStr = m_pmeciErrors->c4AddEnumItem(L"GetCfgString", TString::strEmpty());
        m_c4ErrGetMsg = m_pmeciErrors->c4AddEnumItem(L"GetMsgFailed", TString::strEmpty());
        m_c4ErrLimitViolation = m_pmeciErrors->c4AddEnumItem(L"LimitViolation", L"Value '%(1)' exceeds the limits for field %(2)");
        m_c4ErrNotConnected = m_pmeciErrors->c4AddEnumItem(L"NotConnected", L"You cannot get bytes until in connected state");
        m_c4ErrNoOverride = m_pmeciErrors->c4AddEnumItem(L"NoOverride", L"%(1) method was not implemented by the driver");
        m_c4ErrUnknownConnType = m_pmeciErrors->c4AddEnumItem(L"UnknownConnType", L"%(1) is not a valid source connection type");
        m_c4ErrUnknownExcept = m_pmeciErrors->c4AddEnumItem(L"UnknownExcept", L"An unhandled unknown C++ exception occurred");
        m_c4ErrFldRegFailed = m_pmeciErrors->c4AddEnumItem(L"FldRegFailed", TString::strEmpty());
        m_c4ErrFldIdNotFound = m_pmeciErrors->c4AddEnumItem(L"FldIdNotFound", L"Field id %(1) was not found");
        m_c4ErrAccessFailed = m_pmeciErrors->c4AddEnumItem(L"AccessFailed", TString::strEmpty());
        m_c4ErrSetCfgStr = m_pmeciErrors->c4AddEnumItem(L"SetCfgString", TString::strEmpty());
        m_c4ErrSetVerbErr = m_pmeciErrors->c4AddEnumItem(L"SetVerbosityErr", TString::strEmpty());
        m_pmeciErrors->BaseClassInit(meOwner);
        m_c2EnumId_Errors = meOwner.c2AddClass(m_pmeciErrors);
        bAddNestedType(m_pmeciErrors->strClassPath());
    }

    //
    //  Create an enum to wrap the tCQCKit::ECommResults enum, which we
    //  have to return from our connect and poll methods.
    //
    {
        m_pmeciCommResults = new TMEngEnumInfo
        (
            meOwner, L"CommResults", strClassPath(), L"MEng.Enum", 24
        );
        m_pmeciCommResults->c4AddEnumItem(L"Success", L"Success", tCQCKit::ECommResults::Success);
        m_pmeciCommResults->c4AddEnumItem(L"LostConnection", L"Lost connection", tCQCKit::ECommResults::LostConnection);
        m_pmeciCommResults->c4AddEnumItem(L"LostCommRes", L"Lost communications resource", tCQCKit::ECommResults::LostCommRes);
        m_pmeciCommResults->c4AddEnumItem(L"ValueRejected", L"Value Rejected", tCQCKit::ECommResults::ValueRejected);

        m_pmeciCommResults->c4AddEnumItem(L"Access", L"Access Denied", tCQCKit::ECommResults::Access);
        m_pmeciCommResults->c4AddEnumItem(L"BadValue", L"Bad Value", tCQCKit::ECommResults::BadValue);
        m_pmeciCommResults->c4AddEnumItem(L"Busy", L"Device Busy", tCQCKit::ECommResults::Busy);
        m_pmeciCommResults->c4AddEnumItem(L"CmdParms", L"Invalid Command Parameters", tCQCKit::ECommResults::CmdParms);
        m_pmeciCommResults->c4AddEnumItem(L"CommError", L"Comm Error", tCQCKit::ECommResults::CommError);
        m_pmeciCommResults->c4AddEnumItem(L"DeviceResponse", L"Device Response", tCQCKit::ECommResults::DeviceResponse);
        m_pmeciCommResults->c4AddEnumItem(L"Exception", L"Exception Occurred", tCQCKit::ECommResults::Exception);
        m_pmeciCommResults->c4AddEnumItem(L"FieldNotFound", L"Field Not Found", tCQCKit::ECommResults::FieldNotFound);
        m_pmeciCommResults->c4AddEnumItem(L"Full", L"No Space Available", tCQCKit::ECommResults::Full);
        m_pmeciCommResults->c4AddEnumItem(L"Internal", L"Internal Error", tCQCKit::ECommResults::Internal);
        m_pmeciCommResults->c4AddEnumItem(L"Limits", L"Out of Range Limits", tCQCKit::ECommResults::Limits);
        m_pmeciCommResults->c4AddEnumItem(L"MissingInfo", L"Missing Info", tCQCKit::ECommResults::MissingInfo);
        m_pmeciCommResults->c4AddEnumItem(L"ResNotFound", L"Resource Not Found", tCQCKit::ECommResults::ResNotFound);
        m_pmeciCommResults->c4AddEnumItem(L"NotReady", L"Not Ready", tCQCKit::ECommResults::NotReady);
        m_pmeciCommResults->c4AddEnumItem(L"NotSupported", L"Not Supported", tCQCKit::ECommResults::NotSupported);
        m_pmeciCommResults->c4AddEnumItem(L"PowerState", L"Power State", tCQCKit::ECommResults::PowerState);
        m_pmeciCommResults->c4AddEnumItem(L"ResNotAvail", L"Resource Not Avail", tCQCKit::ECommResults::ResNotAvail);
        m_pmeciCommResults->c4AddEnumItem(L"Unhandled", L"Unhandled", tCQCKit::ECommResults::Unhandled);
        m_pmeciCommResults->c4AddEnumItem(L"UnknownCmd", L"Unknown Command", tCQCKit::ECommResults::UnknownCmd);

        m_pmeciCommResults->BaseClassInit(meOwner);
        m_c2EnumId_CommResults = meOwner.c2AddClass(m_pmeciCommResults);
        bAddNestedType(m_pmeciCommResults->strClassPath());
    }


    //
    //  Create an enum to wrap the tCQCKit::EDrvInitRes enum, which we
    //  have to return from our init method.
    //
    {
        m_pmeciInitRes = new TMEngEnumInfo
        (
            meOwner, L"DrvInitRes", strClassPath(), L"MEng.Enum", 3
        );
        m_pmeciInitRes->c4AddEnumItem(L"Failed", L"The driver initialization failed");
        m_pmeciInitRes->c4AddEnumItem(L"WaitConfig", L"The driver wishes to wait for configuration next");
        m_pmeciInitRes->c4AddEnumItem(L"WaitCommRes", L"The driver wishes to wait for comm resources next");
        m_pmeciInitRes->BaseClassInit(meOwner);
        m_c2EnumId_InitRes = meOwner.c2AddClass(m_pmeciInitRes);
        bAddNestedType(m_pmeciInitRes->strClassPath());
    }


    // Create an enum for the verbosity level enum
    {
        m_pmeciVerboseLevel = new TMEngEnumInfo
        (
            meOwner, L"VerboseLvls", strClassPath(), L"MEng.Enum", 4
        );
        m_pmeciVerboseLevel->c4AddEnumItem(L"Off", L"Off", tCQCKit::EVerboseLvls::Off);
        m_pmeciVerboseLevel->c4AddEnumItem(L"Low", L"Low", tCQCKit::EVerboseLvls::Low);
        m_pmeciVerboseLevel->c4AddEnumItem(L"Medium", L"Medium", tCQCKit::EVerboseLvls::Medium);
        m_pmeciVerboseLevel->c4AddEnumItem(L"High", L"High", tCQCKit::EVerboseLvls::High);
        m_pmeciVerboseLevel->BaseClassInit(meOwner);
        m_c2EnumId_VerboseLevel = meOwner.c2AddClass(m_pmeciVerboseLevel);
        bAddNestedType(m_pmeciVerboseLevel->strClassPath());
    }

    //
    //  Create an enum to represent the possible zone states in a zone
    //  event. It wraps the tCQCKit::EZoneStates enum.
    //
    {
        m_pmeciZoneStates = new TMEngEnumInfo
        (
            meOwner, L"ZoneStates", strClassPath(), L"MEng.Enum", 5
        );

        m_pmeciZoneStates->c4AddEnumItem(L"Secure", L"Secure", tCQCKit::EZoneStates::Secure);
        m_pmeciZoneStates->c4AddEnumItem(L"NotReady", L"Not Ready", tCQCKit::EZoneStates::NotReady);
        m_pmeciZoneStates->c4AddEnumItem(L"Violated", L"Violated", tCQCKit::EZoneStates::Violated);
        m_pmeciZoneStates->c4AddEnumItem(L"Bypassed", L"ByPassed", tCQCKit::EZoneStates::Bypassed);
        m_pmeciZoneStates->c4AddEnumItem(L"Unknown", L"Unknown", tCQCKit::EZoneStates::Unknown);
        m_pmeciZoneStates->BaseClassInit(meOwner);
        m_c2EnumId_ZoneStates = meOwner.c2AddClass(m_pmeciZoneStates);
        bAddNestedType(m_pmeciZoneStates->strClassPath());
    }

    // Look up any oher types of classes we need to use below or to save for later
    const tCIDLib::TCard2 c2CommCfgId = meOwner.c2FindClassId(TMEngCommCfgInfo::strPath());
    const tCIDLib::TCard2 c2EndPointId = meOwner.c2FindClassId(TMEngIPEPInfo::strPath());
    const tCIDLib::TCard2 c2NameValMapId = meOwner.c2FindClassId(TMEngNamedValMapInfo::strPath());

    m_c2TypeId_CommPort = meOwner.c2FindClassId(TMEngCommPortInfo::strPath());
    m_c2TypeId_StreamSocket = meOwner.c2FindClassId(TMEngStreamSocketInfo::strPath());




    // Add the default constructor
    {
        TMEngMethodInfo methiNew
        (
            L"ctor1_MEng.System.CQC.Runtime.CQCDriverBase"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.bIsCtor(kCIDLib::True);
        m_c2MethId_DefCtor = c2AddMethodInfo(methiNew);
    }

    // The device connect callback method (pure method)
    {
        TMEngMethodInfo methiNew
        (
            L"Connect"
            , m_c2EnumId_CommResults
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Required
        );
        m_c2MethId_Connect = c2AddMethodInfo(methiNew);
    }


    //
    //  Create a macro level string out stream, which we will use for the
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

    //
    //  We have a field changed callback for each possible type. It's not
    //  required, and we provide a default that will return an error if
    //  we get called.
    //
    {
        TMEngMethodInfo methiNew
        (
            L"BoolFldChanged"
            , m_c2EnumId_CommResults
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::NonFinal
        );
        methiNew.c2AddInParm(L"FldId", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddInParm(L"NewValue", tCIDMacroEng::EIntrinsics::Boolean);
        m_c2MethId_FldChBool = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"CardFldChanged"
            , m_c2EnumId_CommResults
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::NonFinal
        );
        methiNew.c2AddInParm(L"FldId", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddInParm(L"NewValue", tCIDMacroEng::EIntrinsics::Card4);
        m_c2MethId_FldChCard = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"FloatFldChanged"
            , m_c2EnumId_CommResults
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::NonFinal
        );
        methiNew.c2AddInParm(L"FldId", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddInParm(L"NewValue", tCIDMacroEng::EIntrinsics::Float8);
        m_c2MethId_FldChFloat = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"IntFldChanged"
            , m_c2EnumId_CommResults
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::NonFinal
        );
        methiNew.c2AddInParm(L"FldId", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddInParm(L"NewValue", tCIDMacroEng::EIntrinsics::Int4);
        m_c2MethId_FldChInt = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"StringFldChanged"
            , m_c2EnumId_CommResults
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::NonFinal
        );
        methiNew.c2AddInParm(L"FldId", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddInParm(L"NewValue", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_FldChString = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"StrListFldChanged"
            , m_c2EnumId_CommResults
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::NonFinal
        );
        methiNew.c2AddInParm(L"FldId", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddInParm(L"NewValue", tCIDMacroEng::EIntrinsics::StringList);
        m_c2MethId_FldChStrList = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"TimeFldChanged"
            , m_c2EnumId_CommResults
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::NonFinal
        );
        methiNew.c2AddInParm(L"FldId", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddInParm(L"NewVlaue", tCIDMacroEng::EIntrinsics::Card8);
        m_c2MethId_FldChTime = c2AddMethodInfo(methiNew);
    }

    //
    //  A backdoor method to send a command to the driver. You send a
    //  string of text, and an identifier to say what the string contains.
    //  It's non-final and must be provided by the driver if they want to
    //  support this.
    //
    {
        TMEngMethodInfo methiNew
        (
            L"ClientCmd"
            , tCIDMacroEng::EIntrinsics::Card4
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::NonFinal
        );
        methiNew.c2AddInParm(L"CmdId", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"Data", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_ClientCmd = c2AddMethodInfo(methiNew);
    }


    // Delete the config string for this drive
    {
        TMEngMethodInfo methiNew
        (
            L"DelConfigStr"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        m_c2MethId_DelConfigStr = c2AddMethodInfo(methiNew);
    }

    // Look up a field and get it's id and another one for V2 drivers
    {
        TMEngMethodInfo methiNew
        (
            L"FldIdFromName"
            , tCIDMacroEng::EIntrinsics::Card4
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInParm(L"NameToFind", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_FldIdFromName = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"FindFldIdCl"
            , tCIDMacroEng::EIntrinsics::Card4
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );

        methiNew.c2AddInParm(L"DevClass", m_pmeciDevClasses->c2Id());
        methiNew.c2AddInParm(L"Name", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_FindFldIdCl = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"FindFldIdSub"
            , tCIDMacroEng::EIntrinsics::Card4
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );

        methiNew.c2AddInParm(L"DevClass", m_pmeciDevClasses->c2Id());
        methiNew.c2AddInParm(L"SubUnit", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"NameToFind", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_FindFldIdSub = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"FindFldIdPNS"
            , tCIDMacroEng::EIntrinsics::Card4
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );

        methiNew.c2AddInParm(L"DevClass", m_pmeciDevClasses->c2Id());
        methiNew.c2AddInParm(L"Prefix", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"NameToFind", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"Suffix", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_FindFldIdPNS = c2AddMethodInfo(methiNew);
    }

    // Free the driver comm resource
    {
        TMEngMethodInfo methiNew
        (
            L"FreeCommResource"
            , tCIDMacroEng::EIntrinsics::Boolean
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Required
        );
        m_c2MethId_FreeCommRes = c2AddMethodInfo(methiNew);
    }

    // A convenience to format out a string list
    {
        TMEngMethodInfo methiNew
        (
            L"FormatStrList"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInParm(L"FldId", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddOutParm(L"ToFill", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_FormatStrList = c2AddMethodInfo(methiNew);
    }

    // Get the driver architecture version
    {
        TMEngMethodInfo methiNew
        (
            L"GetArchVersion"
            , tCIDMacroEng::EIntrinsics::Card4
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        m_c2MethId_GetArchVer = c2AddMethodInfo(methiNew);
    }

    // Get a terminated ASCII message
    {
        TMEngMethodInfo methiNew
        (
            L"GetASCIITermedMsg"
            , tCIDMacroEng::EIntrinsics::Boolean
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInOutParm(L"SrcObj", tCIDMacroEng::EIntrinsics::Object);
        methiNew.c2AddInParm(L"WaitFor", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddInParm(L"TermCh1", tCIDMacroEng::EIntrinsics::Card1);
        methiNew.c2AddInParm(L"TermCh2", tCIDMacroEng::EIntrinsics::Card1);
        methiNew.c2AddOutParm(L"OutStr", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_GetASCIITermMsg = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"GetASCIITermedMsg2"
            , tCIDMacroEng::EIntrinsics::Boolean
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInOutParm(L"SrcObj", tCIDMacroEng::EIntrinsics::Object);
        methiNew.c2AddInParm(L"EndTime", tCIDMacroEng::EIntrinsics::Card8);
        methiNew.c2AddInParm(L"TermCh1", tCIDMacroEng::EIntrinsics::Card1);
        methiNew.c2AddInParm(L"TermCh2", tCIDMacroEng::EIntrinsics::Card1);
        methiNew.c2AddOutParm(L"OutStr", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_GetASCIITermMsg2 = c2AddMethodInfo(methiNew);
    }

    // Get a start/end delimited ASCII message
    {
        TMEngMethodInfo methiNew
        (
            L"GetASCIIStartStopMsg"
            , tCIDMacroEng::EIntrinsics::Boolean
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInOutParm(L"SrcObj", tCIDMacroEng::EIntrinsics::Object);
        methiNew.c2AddInParm(L"WaitFor", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddInParm(L"StartByte", tCIDMacroEng::EIntrinsics::Card1);
        methiNew.c2AddInParm(L"EndByte", tCIDMacroEng::EIntrinsics::Card1);
        methiNew.c2AddOutParm(L"ToFill", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_GetASCIIStartStopMsg = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"GetASCIIStartStopMsg2"
            , tCIDMacroEng::EIntrinsics::Boolean
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInOutParm(L"SrcObj", tCIDMacroEng::EIntrinsics::Object);
        methiNew.c2AddInParm(L"EndTime", tCIDMacroEng::EIntrinsics::Card8);
        methiNew.c2AddInParm(L"StartByte", tCIDMacroEng::EIntrinsics::Card1);
        methiNew.c2AddInParm(L"EndByte", tCIDMacroEng::EIntrinsics::Card1);
        methiNew.c2AddOutParm(L"ToFill", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_GetASCIIStartStopMsg2 = c2AddMethodInfo(methiNew);
    }

    // Obtain the driver comm resource
    {
        TMEngMethodInfo methiNew
        (
            L"GetCommResource"
            , tCIDMacroEng::EIntrinsics::Boolean
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Required
        );
        m_c2MethId_GetCommRes = c2AddMethodInfo(methiNew);
    }

    //
    //  Get the driver's config string. Return indicates whether it was
    //  available or not. If not, the output string will be empty.
    //
    {
        TMEngMethodInfo methiNew
        (
            L"GetConfigStr"
            , tCIDMacroEng::EIntrinsics::Boolean
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddOutParm(L"ToFill", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_GetConfigStr = c2AddMethodInfo(methiNew);
    }

    // Return the make, model, and version info from the driver manfest
    {
        TMEngMethodInfo methiNew
        (
            L"GetDriverInfo"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddOutParm(L"Make", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddOutParm(L"Model", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddOutParm(L"MajVersion", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddOutParm(L"MinVersion", tCIDMacroEng::EIntrinsics::Card4);
        m_c2MethId_GetDriverInfo = c2AddMethodInfo(methiNew);
    }

    // Get a particular field's error state
    {
        TMEngMethodInfo methiNew
        (
            L"GetFldErrState"
            , tCIDMacroEng::EIntrinsics::Boolean
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInParm(L"FldId", tCIDMacroEng::EIntrinsics::Card4);
        m_c2MethId_GetFldErrState = c2AddMethodInfo(methiNew);
    }

    // Given a field id , get it's field definition
    {
        TMEngMethodInfo methiNew
        (
            L"GetFldInfoById"
            , tCIDMacroEng::EIntrinsics::Card4
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInParm(L"FldId", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddOutParm(L"DefToFill", c2FldDefId);
        m_c2MethId_GetFldInfoById = c2AddMethodInfo(methiNew);
    }

    //
    //  Given a field name, get it's field definition, and return its field
    //  id.
    //
    {
        TMEngMethodInfo methiNew
        (
            L"GetFldInfoByName"
            , tCIDMacroEng::EIntrinsics::Card4
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInParm(L"Name", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddOutParm(L"DefToFill", c2FldDefId);
        m_c2MethId_GetFldInfoByName = c2AddMethodInfo(methiNew);
    }


    // Get a particular field's name via it's id
    {
        TMEngMethodInfo methiNew
        (
            L"GetFldName"
            , tCIDMacroEng::EIntrinsics::String
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInParm(L"FldId", tCIDMacroEng::EIntrinsics::Card4);
        m_c2MethId_GetFldName = c2AddMethodInfo(methiNew);
    }

    // Get whether the driver is connected or not
    {
        TMEngMethodInfo methiNew
        (
            L"GetIsConnected"
            , tCIDMacroEng::EIntrinsics::Boolean
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        m_c2MethId_GetIsConnected = c2AddMethodInfo(methiNew);
    }

    // Get the driver moniker
    {
        TMEngMethodInfo methiNew
        (
            L"GetMoniker"
            , tCIDMacroEng::EIntrinsics::String
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        m_c2MethId_GetMoniker = c2AddMethodInfo(methiNew);
    }

    // Get a start + length delimited binary message
    {
        TMEngMethodInfo methiNew
        (
            L"GetStartLenMsg"
            , tCIDMacroEng::EIntrinsics::Card4
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInOutParm(L"SrcObj", tCIDMacroEng::EIntrinsics::Object);
        methiNew.c2AddInParm(L"WaitFor", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddInParm(L"StartByte", tCIDMacroEng::EIntrinsics::Card1);
        methiNew.c2AddInParm(L"LenOfs", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddInParm(L"LenSub", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddInParm(L"TrailBytes", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddInParm(L"MaxBytes", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddOutParm(L"OutBuf", tCIDMacroEng::EIntrinsics::MemBuf);
        m_c2MethId_GetStartLenMsg = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"GetStartLenMsg2"
            , tCIDMacroEng::EIntrinsics::Card4
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInOutParm(L"SrcObj", tCIDMacroEng::EIntrinsics::Object);
        methiNew.c2AddInParm(L"EndTime", tCIDMacroEng::EIntrinsics::Card8);
        methiNew.c2AddInParm(L"StartByte", tCIDMacroEng::EIntrinsics::Card1);
        methiNew.c2AddInParm(L"LenOfs", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddInParm(L"LenSub", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddInParm(L"TrailBytes", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddInParm(L"MaxBytes", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddOutParm(L"OutBuf", tCIDMacroEng::EIntrinsics::MemBuf);
        m_c2MethId_GetStartLenMsg2 = c2AddMethodInfo(methiNew);
    }

    // Get a start/end delimited binary message
    {
        TMEngMethodInfo methiNew
        (
            L"GetStartStopMsg"
            , tCIDMacroEng::EIntrinsics::Card4
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInOutParm(L"SrcObj", tCIDMacroEng::EIntrinsics::Object);
        methiNew.c2AddInParm(L"WaitFor", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddInParm(L"StartByte", tCIDMacroEng::EIntrinsics::Card1);
        methiNew.c2AddInParm(L"EndByte", tCIDMacroEng::EIntrinsics::Card1);
        methiNew.c2AddInParm(L"MaxBytes", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddOutParm(L"OutBuf", tCIDMacroEng::EIntrinsics::MemBuf);
        m_c2MethId_GetStartStopMsg = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"GetStartStopMsg2"
            , tCIDMacroEng::EIntrinsics::Card4
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInOutParm(L"SrcObj", tCIDMacroEng::EIntrinsics::Object);
        methiNew.c2AddInParm(L"EndTime", tCIDMacroEng::EIntrinsics::Card8);
        methiNew.c2AddInParm(L"StartByte", tCIDMacroEng::EIntrinsics::Card1);
        methiNew.c2AddInParm(L"EndByte", tCIDMacroEng::EIntrinsics::Card1);
        methiNew.c2AddInParm(L"MaxBytes", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddOutParm(L"OutBuf", tCIDMacroEng::EIntrinsics::MemBuf);
        m_c2MethId_GetStartStopMsg2 = c2AddMethodInfo(methiNew);
    }

    // Get a terminated binary message
    {
        TMEngMethodInfo methiNew
        (
            L"GetTermedMsg"
            , tCIDMacroEng::EIntrinsics::Card4
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInOutParm(L"SrcObj", tCIDMacroEng::EIntrinsics::Object);
        methiNew.c2AddInParm(L"WaitFor", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddInParm(L"Term1", tCIDMacroEng::EIntrinsics::Card1);
        methiNew.c2AddInParm(L"Term2", tCIDMacroEng::EIntrinsics::Card1);
        methiNew.c2AddInParm(L"TwoTerms", tCIDMacroEng::EIntrinsics::Boolean);
        methiNew.c2AddOutParm(L"OutBuf", tCIDMacroEng::EIntrinsics::MemBuf);
        m_c2MethId_GetTermedMsg = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"GetTermedMsg2"
            , tCIDMacroEng::EIntrinsics::Card4
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInOutParm(L"SrcObj", tCIDMacroEng::EIntrinsics::Object);
        methiNew.c2AddInParm(L"EndTime", tCIDMacroEng::EIntrinsics::Card8);
        methiNew.c2AddInParm(L"Term1", tCIDMacroEng::EIntrinsics::Card1);
        methiNew.c2AddInParm(L"Term2", tCIDMacroEng::EIntrinsics::Card1);
        methiNew.c2AddInParm(L"TwoTerms", tCIDMacroEng::EIntrinsics::Boolean);
        methiNew.c2AddOutParm(L"OutBuf", tCIDMacroEng::EIntrinsics::MemBuf);
        m_c2MethId_GetTermedMsg2 = c2AddMethodInfo(methiNew);
    }

    // Get the verbose mode
    {
        TMEngMethodInfo methiNew
        (
            L"GetVerboseLevel"
            , m_c2EnumId_VerboseLevel
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        m_c2MethId_GetVerboseLevel = c2AddMethodInfo(methiNew);
    }

    // The initialization callback methods
    {
        TMEngMethodInfo methiNew
        (
            L"InitializeOther"
            , m_c2EnumId_InitRes
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::NonFinal
        );
        methiNew.c2AddInParm(L"CfgString", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"PromptVals", c2NameValMapId);
        m_c2MethId_InitOther = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"InitializeSerial"
            , m_c2EnumId_InitRes
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::NonFinal
        );
        methiNew.c2AddInParm(L"TargetPort", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"CommCfg", c2CommCfgId);
        methiNew.c2AddInParm(L"PromptVals", c2NameValMapId);
        m_c2MethId_InitSerial = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"InitializeSocket"
            , m_c2EnumId_InitRes
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::NonFinal
        );
        methiNew.c2AddInParm(L"EndPt", c2EndPointId);
        methiNew.c2AddInParm(L"PromptVals", c2NameValMapId);
        m_c2MethId_InitSocket = c2AddMethodInfo(methiNew);
    }

    // Increment some instrumentation fields that all drivers have
    {
        TMEngMethodInfo methiNew
        (
            L"IncBadMsgs"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        m_c2MethId_IncBadMsgs = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"IncFailedWrite"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        m_c2MethId_IncFailedWrite = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"IncNaks"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        m_c2MethId_IncNaks = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"IncReconfigured"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        m_c2MethId_IncReconfigured = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"IncTimeouts"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        m_c2MethId_IncTimeouts = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"IncUnknownMsgs"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        m_c2MethId_IncUnknownMsgs = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"IncUnknownWrite"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        m_c2MethId_IncUnknownWrite = c2AddMethodInfo(methiNew);
    }

    // Let them log a message to the log server, with token replacement
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

    // The device polling callback method (pure method)
    {
        TMEngMethodInfo methiNew
        (
            L"Poll"
            , m_c2EnumId_CommResults
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Required
        );
        m_c2MethId_Poll = c2AddMethodInfo(methiNew);
    }

    //
    //  Various optional methods that allow clients to talk to the driver
    //  without going through the field mechanism.
    //
    {
        TMEngMethodInfo methiNew
        (
            L"QueryBoolVal"
            , tCIDMacroEng::EIntrinsics::Boolean
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::NonFinal
        );
        methiNew.c2AddInParm(L"ValId", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_QueryBoolVal = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"QueryBufVal"
            , tCIDMacroEng::EIntrinsics::Boolean
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::NonFinal
        );
        methiNew.c2AddInParm(L"ValId", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"DataName", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddOutParm(L"OutBytes", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddOutParm(L"ToFill", tCIDMacroEng::EIntrinsics::MemBuf);
        m_c2MethId_QueryBufVal = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"QueryCardVal"
            , tCIDMacroEng::EIntrinsics::Card4
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::NonFinal
        );
        methiNew.c2AddInParm(L"ValId", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_QueryCardVal = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"QueryIntVal"
            , tCIDMacroEng::EIntrinsics::Int4
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::NonFinal
        );
        methiNew.c2AddInParm(L"ValId", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_QueryIntVal = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"QueryTextVal"
            , tCIDMacroEng::EIntrinsics::Boolean
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::NonFinal
        );
        methiNew.c2AddInParm(L"ValId", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"DataName", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddOutParm(L"ToFill", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_QueryTextVal = c2AddMethodInfo(methiNew);
    }

    //
    //  These allow the driver to read back the values of it's fields,
    //  which it sometimes needs to do if a field write requires building
    //  up a message that sets more than one value in the device.
    //
    {
        TMEngMethodInfo methiNew
        (
            L"ReadBoolFld"
            , tCIDMacroEng::EIntrinsics::Boolean
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInParm(L"FldId", tCIDMacroEng::EIntrinsics::Card4);
        m_c2MethId_ReadBoolFld = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"ReadBoolFldByName"
            , tCIDMacroEng::EIntrinsics::Boolean
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInParm(L"FldName", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_ReadBoolFldByName = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"ReadCardFld"
            , tCIDMacroEng::EIntrinsics::Card4
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInParm(L"FldId", tCIDMacroEng::EIntrinsics::Card4);
        m_c2MethId_ReadCardFld = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"ReadCardFldByName"
            , tCIDMacroEng::EIntrinsics::Card4
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInParm(L"FldName", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_ReadCardFldByName = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"ReadFloatFld"
            , tCIDMacroEng::EIntrinsics::Float8
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInParm(L"FldId", tCIDMacroEng::EIntrinsics::Card4);
        m_c2MethId_ReadFloatFld = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"ReadFloatFldByName"
            , tCIDMacroEng::EIntrinsics::Float8
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInParm(L"FldName", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_ReadFloatFldByName = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"ReadIntFld"
            , tCIDMacroEng::EIntrinsics::Int4
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInParm(L"FldId", tCIDMacroEng::EIntrinsics::Card4);
        m_c2MethId_ReadIntFld = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"ReadIntFldByName"
            , tCIDMacroEng::EIntrinsics::Int4
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInParm(L"FldName", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_ReadIntFldByName = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"ReadStringFld"
            , tCIDMacroEng::EIntrinsics::String
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInParm(L"FldId", tCIDMacroEng::EIntrinsics::Card4);
        m_c2MethId_ReadStringFld = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"ReadStringFldByName"
            , tCIDMacroEng::EIntrinsics::String
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInParm(L"FldName", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_ReadStringFldByName = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"ReadStrListFld"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInParm(L"FldId", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddOutParm(L"ToFill", tCIDMacroEng::EIntrinsics::StringList);
        m_c2MethId_ReadStrListFld = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"ReadStrListFldByName"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInParm(L"FldName", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddOutParm(L"ToFill", tCIDMacroEng::EIntrinsics::StringList);
        m_c2MethId_ReadStrListFldByName = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"ReadTimeFld"
            , tCIDMacroEng::EIntrinsics::Card8
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInParm(L"FldId", tCIDMacroEng::EIntrinsics::Card4);
        m_c2MethId_ReadTimeFld = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"ReadTimeFldByName"
            , tCIDMacroEng::EIntrinsics::Card8
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInParm(L"FldName", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_ReadTimeFldByName = c2AddMethodInfo(methiNew);
    }

    // Send a load change event
    {
        TMEngMethodInfo methiNew
        (
            L"SendLoadEvent"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"State", tCIDMacroEng::EIntrinsics::Boolean);
        methiNew.c2AddInParm(L"LoadNum", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddInParm(L"FldName", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_SendLoadEv = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"SendLoadEvent2"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"State", tCIDMacroEng::EIntrinsics::Boolean);
        methiNew.c2AddInParm(L"LoadNum", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddInParm(L"FldName", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"LoadName", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_SendLoadEv2 = c2AddMethodInfo(methiNew);
    }

    // Send a loock change event
    {
        TMEngMethodInfo methiNew
        (
            L"SendLockEvent"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"State", tCIDMacroEng::EIntrinsics::Boolean);
        methiNew.c2AddInParm(L"LockId", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"Code", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"Type", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_SendLockEv = c2AddMethodInfo(methiNew);
    }

    // Send a motion event
    {
        TMEngMethodInfo methiNew
        (
            L"SendMotionEvent"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"State", tCIDMacroEng::EIntrinsics::Boolean);
        methiNew.c2AddInParm(L"SensorNum", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddInParm(L"FldName", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_SendMotionEv = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"SendMotionEvent2"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"State", tCIDMacroEng::EIntrinsics::Boolean);
        methiNew.c2AddInParm(L"SensorNum", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddInParm(L"FldName", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"SensorName", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_SendMotionEv2 = c2AddMethodInfo(methiNew);
    }

    // Send a presence event
    {
        TMEngMethodInfo methiNew
        (
            L"SendPresenceEvent"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"EnterExit", tCIDMacroEng::EIntrinsics::Boolean);
        methiNew.c2AddInParm(L"IDInfo", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"Area", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_SendPresenceEv = c2AddMethodInfo(methiNew);
    }

    // Send a user action event
    {
        TMEngMethodInfo methiNew
        (
            L"SendUserActEvent"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"EvType", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"EvData", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_SendUserActEv = c2AddMethodInfo(methiNew);
    }

    // Send a zone alarm event
    {
        TMEngMethodInfo methiNew
        (
            L"SendZoneEvent"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"State", m_c2EnumId_ZoneStates);
        methiNew.c2AddInParm(L"ZoneNum", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddInParm(L"FldName", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_SendZoneEv = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"SendZoneEvent2"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"State", m_c2EnumId_ZoneStates);
        methiNew.c2AddInParm(L"ZoneNum", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddInParm(L"FldName", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"Name", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_SendZoneEv2 = c2AddMethodInfo(methiNew);
    }


    // Set all fields to initial error state (only legal before connected)
    {
        TMEngMethodInfo methiNew
        (
            L"SetAllErrStates"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        m_c2MethId_SetAllErrStates = c2AddMethodInfo(methiNew);
    }

    // Set the driver's config string
    {
        TMEngMethodInfo methiNew
        (
            L"SetConfigStr"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"ToSet", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_SetConfigStr = c2AddMethodInfo(methiNew);
    }

    //
    //  The 'set fields' method that the derived class calls to set his
    //  list of fields that he wants to export.
    //
    {
        TMEngMethodInfo methiNew
        (
            L"SetFields"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"ToSet", m_c2TypeId_FldList);
        m_c2MethId_SetFields = c2AddMethodInfo(methiNew);
    }

    // Set a particular field's error state
    {
        TMEngMethodInfo methiNew
        (
            L"SetFldErrState"
            , tCIDMacroEng::EIntrinsics::Boolean
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"FldId", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddInParm(L"NewValue", tCIDMacroEng::EIntrinsics::Boolean);
        m_c2MethId_SetFldErrState = c2AddMethodInfo(methiNew);
    }

    // Set the poll and reconnect times
    {
        TMEngMethodInfo methiNew
        (
            L"SetPollTimes"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"PollMillis", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddInParm(L"ReconnMillis", tCIDMacroEng::EIntrinsics::Card4);
        m_c2MethId_SetPollTimes = c2AddMethodInfo(methiNew);
    }

    // Set the message reading timeout extension
    {
        TMEngMethodInfo methiNew
        (
            L"SetTOExtension"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"ToSet", tCIDMacroEng::EIntrinsics::Card8);
        m_c2MethId_SetTOExtension = c2AddMethodInfo(methiNew);
    }

    // Set the verbose mode
    {
        TMEngMethodInfo methiNew
        (
            L"SetVerboseLevel"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"ToSet", m_c2EnumId_VerboseLevel);
        m_c2MethId_SetVerboseLevel = c2AddMethodInfo(methiNew);
    }

    // The simulator method, only used in debugging mode
    {
        TMEngMethodInfo methiNew
        (
            L"Simulate"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        m_c2MethId_Simulate = c2AddMethodInfo(methiNew);
    }

    // Terminate the driver
    {
        TMEngMethodInfo methiNew
        (
            L"Terminate"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::NonFinal
        );
        m_c2MethId_Terminate = c2AddMethodInfo(methiNew);
    }

    //
    //  Tell the driver that the default value for the indicated field should
    //  be taken as the valid initial value.
    //
    {
        TMEngMethodInfo methiNew
        (
            L"UseDefFldValue"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::NonFinal
        );
        methiNew.c2AddInParm(L"FldId", tCIDMacroEng::EIntrinsics::Card4);
        m_c2MethId_UseDefFldValue = c2AddMethodInfo(methiNew);
    }


    // The option method to wait for configuration
    {
        TMEngMethodInfo methiNew
        (
            L"VerbosityChanged"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::NonFinal
        );
        methiNew.c2AddInParm(L"NewLevel", m_c2EnumId_VerboseLevel);
        m_c2MethId_VerbosityChanged = c2AddMethodInfo(methiNew);
    }

    // The optional method to wait for configuration
    {
        TMEngMethodInfo methiNew
        (
            L"WaitConfig"
            , tCIDMacroEng::EIntrinsics::Boolean
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::NonFinal
        );
        m_c2MethId_WaitConfig = c2AddMethodInfo(methiNew);
    }

    //
    //  We provide methods to the derived classes that will allow them
    //  to update their field values based on data they get from the
    //  device. We provide one per data types.
    //
    {
        TMEngMethodInfo methiNew
        (
            L"WriteBoolFld"
            , tCIDMacroEng::EIntrinsics::Boolean
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"FldId", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddInParm(L"NewValue", tCIDMacroEng::EIntrinsics::Boolean);
        m_c2MethId_WriteBool = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"WriteBoolFldByName"
            , tCIDMacroEng::EIntrinsics::Boolean
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"FldName", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"NewValue", tCIDMacroEng::EIntrinsics::Boolean);
        m_c2MethId_WriteBoolByName = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"WriteCardFld"
            , tCIDMacroEng::EIntrinsics::Boolean
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"FldId", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddInParm(L"NewValue", tCIDMacroEng::EIntrinsics::Card4);
        m_c2MethId_WriteCard = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"WriteCardFldByName"
            , tCIDMacroEng::EIntrinsics::Boolean
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"FldName", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"NewValue", tCIDMacroEng::EIntrinsics::Card4);
        m_c2MethId_WriteCardByName = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"WriteFloatFld"
            , tCIDMacroEng::EIntrinsics::Boolean
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"FldId", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddInParm(L"NewValue", tCIDMacroEng::EIntrinsics::Float8);
        m_c2MethId_WriteFloat = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"WriteFloatFldByName"
            , tCIDMacroEng::EIntrinsics::Boolean
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"FldName", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"NewValue", tCIDMacroEng::EIntrinsics::Float8);
        m_c2MethId_WriteFloatByName = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"WriteIntFld"
            , tCIDMacroEng::EIntrinsics::Boolean
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"FldId", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddInParm(L"NewValue", tCIDMacroEng::EIntrinsics::Int4);
        m_c2MethId_WriteInt = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"WriteIntFldByName"
            , tCIDMacroEng::EIntrinsics::Boolean
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"FldName", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"NewValue", tCIDMacroEng::EIntrinsics::Int4);
        m_c2MethId_WriteIntByName = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"WriteStringFld"
            , tCIDMacroEng::EIntrinsics::Boolean
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"FldId", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddInParm(L"NewValue", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_WriteString = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"WriteStringFldByName"
            , tCIDMacroEng::EIntrinsics::Boolean
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"FldName", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"NewValue", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_WriteStringByName = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"WriteStrListFld"
            , tCIDMacroEng::EIntrinsics::Boolean
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"FldId", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddInParm(L"NewValue", tCIDMacroEng::EIntrinsics::StringList);
        m_c2MethId_WriteStrList = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"WriteStrListFldByName"
            , tCIDMacroEng::EIntrinsics::Boolean
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"FldName", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"NewValue", tCIDMacroEng::EIntrinsics::StringList);
        m_c2MethId_WriteStrListByName = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"WriteTimeFld"
            , tCIDMacroEng::EIntrinsics::Boolean
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"FldId", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddInParm(L"NewValue", tCIDMacroEng::EIntrinsics::Card8);
        m_c2MethId_WriteTime = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"WriteTimeFldByName"
            , tCIDMacroEng::EIntrinsics::Boolean
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"FldName", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"NewValue", tCIDMacroEng::EIntrinsics::Card8);
        m_c2MethId_WriteTimeByName = c2AddMethodInfo(methiNew);
    }
}


TMEngClassVal*
TCQCDriverInfo::pmecvMakeStorage(const  TString&                strName
                                ,       TCIDMacroEngine&
                                , const tCIDMacroEng::EConstTypes  eConst) const
{
    return new TMEngStdClassVal(strName, c2Id(), eConst);
}


// ---------------------------------------------------------------------------
//  TCQCDriverInfo: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TCQCDriverInfo::bInvokeMethod(          TCIDMacroEngine&    meOwner
                                , const TMEngMethodInfo&    methiTarget
                                ,       TMEngClassVal&      mecvInstance)
{
    TMEngStdClassVal& mecvActual = static_cast<TMEngStdClassVal&>(mecvInstance);

    const tCIDLib::TCard4 c4FirstInd = meOwner.c4FirstParmInd(methiTarget);
    const tCIDLib::TCard2 c2MethId   = methiTarget.c2Id();

    //
    //  Get some members out, on the assumption that they will be used most
    //  of the time.
    //
    TCQCDriverGlueVal& mecvGlue = static_cast<TCQCDriverGlueVal&>
    (
        mecvActual.mecvFind(m_c2MemId_Glue, meOwner)
    );
    TCQCGenDrvGlue& sdrvGlue = mecvGlue.sdrvImpl();


    tCIDLib::TCard4 c4FldId = 0;
    tCIDLib::TCard4 c4SerialNum = 0;

    if (c2MethId == m_c2MethId_DefCtor)
    {
        //
        //  We don't do anything. In this case, the object will have been
        //  created directly by the C++ generic driver class, and it will
        //  have set up the object. We just have to have this in order to
        //  create the object.
        //
    }
     else if (c2MethId == m_c2MethId_ClientCmd)
    {
        // Throw an exception, since they didn't override it
        ThrowAnErr(meOwner, m_c4ErrNoOverride, TString(L"ClientCmd"));
    }
     else if (c2MethId == m_c2MethId_DelConfigStr)
    {
        //
        //  Build the path for this driver. They have a fixed format with
        //  just the moniker being used as the differentiator.
        //
        TString strKey(kCQCKit::pszOSKey_CQCSrvDriverCfgs);
        strKey.Append(sdrvGlue.strMoniker());

        try
        {
            // Get a local config server proxy
            TCfgServerClient cfgsGet(4000);
            cfgsGet.bDeleteObjectIfExists(strKey);
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            ThrowAnErr(meOwner, m_c4ErrDelCfgStr, errToCatch);
        }
    }
     else if ((c2MethId == m_c2MethId_FldChBool)
          ||  (c2MethId == m_c2MethId_FldChCard)
          ||  (c2MethId == m_c2MethId_FldChFloat)
          ||  (c2MethId == m_c2MethId_FldChInt)
          ||  (c2MethId == m_c2MethId_FldChString)
          ||  (c2MethId == m_c2MethId_FldChStrList)
          ||  (c2MethId == m_c2MethId_FldChTime))
    {
        // Increment the 'unknown write' instrumentation field
        sdrvGlue.IncUnknownWriteCounter();

        // And set the return value to field not found
        TMEngEnumVal& mecvRet = meOwner.mecvStackAtAs<TMEngEnumVal>(c4FirstInd - 1);
        mecvRet.c4Ordinal(m_pmeciCommResults->c4FromMapValue(tCQCKit::ECommResults::Unhandled));
    }
     else if (c2MethId == m_c2MethId_FldIdFromName)
    {
        try
        {
            c4FldId = c4CheckFldName(meOwner, sdrvGlue, c4FirstInd);
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            ThrowAnErr(meOwner, m_c4ErrAccessFailed, errToCatch);
        }

        TMEngCard4Val& mecvRet = meOwner.mecvStackAtAs<TMEngCard4Val>(c4FirstInd - 1);
        mecvRet.c4Value(c4FldId);
    }
     else if (c2MethId == m_c2MethId_FindFldIdCl)
    {
        try
        {
            TMEngEnumVal& mecvClass = meOwner.mecvStackAtAs<TMEngEnumVal>(c4FirstInd);
            c4FldId = sdrvGlue.flddFind
            (
                tCQCKit::EDevClasses(m_pmeciDevClasses->c4MapValue(mecvClass))
                , meOwner.strStackValAt(c4FirstInd + 1)
            ).c4Id();
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            ThrowAnErr(meOwner, m_c4ErrAccessFailed, errToCatch);
        }

        TMEngCard4Val& mecvRet = meOwner.mecvStackAtAs<TMEngCard4Val>(c4FirstInd - 1);
        mecvRet.c4Value(c4FldId);
    }
     else if (c2MethId == m_c2MethId_FindFldIdSub)
    {
        try
        {
            TMEngEnumVal& mecvClass = meOwner.mecvStackAtAs<TMEngEnumVal>(c4FirstInd);
            c4FldId = sdrvGlue.flddFind
            (
                tCQCKit::EDevClasses(m_pmeciDevClasses->c4MapValue(mecvClass))
                , meOwner.strStackValAt(c4FirstInd + 1)
                , meOwner.strStackValAt(c4FirstInd + 2)
            ).c4Id();
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            ThrowAnErr(meOwner, m_c4ErrAccessFailed, errToCatch);
        }

        TMEngCard4Val& mecvRet = meOwner.mecvStackAtAs<TMEngCard4Val>(c4FirstInd - 1);
        mecvRet.c4Value(c4FldId);
    }
     else if (c2MethId == m_c2MethId_FindFldIdPNS)
    {
        try
        {
            TMEngEnumVal& mecvClass = meOwner.mecvStackAtAs<TMEngEnumVal>(c4FirstInd);
            c4FldId = sdrvGlue.flddFind
            (
                tCQCKit::EDevClasses(m_pmeciDevClasses->c4MapValue(mecvClass))
                , meOwner.strStackValAt(c4FirstInd + 1)
                , meOwner.strStackValAt(c4FirstInd + 2)
                , meOwner.strStackValAt(c4FirstInd + 3)
            ).c4Id();
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            ThrowAnErr(meOwner, m_c4ErrAccessFailed, errToCatch);
        }

        TMEngCard4Val& mecvRet = meOwner.mecvStackAtAs<TMEngCard4Val>(c4FirstInd - 1);
        mecvRet.c4Value(c4FldId);
    }
     else if (c2MethId == m_c2MethId_FormatStrList)
    {
        try
        {
            TMEngStringVal& mecvToFill = meOwner.mecvStackAtAs<TMEngStringVal>(c4FirstInd + 1);

            // This guy is thread safe
            sdrvGlue.FormatStrListFld
            (
                meOwner.c4StackValAt(c4FirstInd), mecvToFill.strValue()
            );
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            ThrowAnErr(meOwner, m_c4ErrAccessFailed, errToCatch);
        }
    }
     else if (c2MethId == m_c2MethId_GetArchVer)
    {
        // Return the architecture version
        TMEngCard4Val& mecvRet = meOwner.mecvStackAtAs<TMEngCard4Val>(c4FirstInd - 1);
        mecvRet.c4Value(mecvGlue.sdrvImpl().c4ArchVersion());
    }
     else if ((c2MethId == m_c2MethId_GetASCIITermMsg)
          ||  (c2MethId == m_c2MethId_GetASCIITermMsg2)
          ||  (c2MethId == m_c2MethId_GetTermedMsg)
          ||  (c2MethId == m_c2MethId_GetTermedMsg2))
    {
        //
        //  Get the common parameters out. These are the same in both of
        //  method variants.
        //
        //  We have to get the source object out and look at the actual type
        //  of it, and use that info to get a pointer to the actual serial port,
        //  socket, etc... that is providing the actual source.
        //
        TObject* pobjSrc = 0;
        TMEngClassVal& mecvSrc = meOwner.mecvStackAtAs<TMEngClassVal>(c4FirstInd);
        if (mecvSrc.c2ClassId() == m_c2TypeId_CommPort)
        {
            TMEngCommPortVal& mecvComm = static_cast<TMEngCommPortVal&>(mecvSrc);
            pobjSrc = &mecvComm.commValue();
        }
         else if (mecvSrc.c2ClassId() == m_c2TypeId_StreamSocket)
        {
            TMEngStreamSocketVal& mecvSock = static_cast<TMEngStreamSocketVal&>(mecvSrc);
            pobjSrc = &mecvSock.sockStreamValue(meOwner);
        }
         else
        {
            // Dunno what this is
            ThrowAnErr
            (
                meOwner
                , m_c4ErrUnknownConnType
                , meOwner.meciFind(mecvSrc.c2ClassId()).strClassPath()
            );
        }
        const tCIDLib::TCard1 c1Term1 = meOwner.c1StackValAt(c4FirstInd + 2);
        const tCIDLib::TCard1 c1Term2 = meOwner.c1StackValAt(c4FirstInd + 3);


        // And now get the variant specific stuff and make the call
        try
        {
            if ((c2MethId == m_c2MethId_GetASCIITermMsg)
            ||  (c2MethId == m_c2MethId_GetASCIITermMsg2))
            {
                TMEngBooleanVal& mecvRet = meOwner.mecvStackAtAs<TMEngBooleanVal>(c4FirstInd - 1);
                TMEngStringVal& mecvStr = meOwner.mecvStackAtAs<TMEngStringVal>(c4FirstInd + 4);

                if (c2MethId == m_c2MethId_GetASCIITermMsg)
                {
                    mecvRet.bValue
                    (
                        sdrvGlue.bGetASCIITermMsg
                        (
                            *pobjSrc
                            , meOwner.c4StackValAt(c4FirstInd + 1)
                            , c1Term1
                            , c1Term2
                            , mecvStr.strValue()
                        )
                    );
                }
                 else
                {
                    mecvRet.bValue
                    (
                        sdrvGlue.bGetASCIITermMsg2
                        (
                            *pobjSrc
                            , meOwner.c8StackValAt(c4FirstInd + 1)
                            , c1Term1
                            , c1Term2
                            , mecvStr.strValue()
                        )
                    );
                }
            }
             else if ((c2MethId == m_c2MethId_GetTermedMsg)
                  ||  (c2MethId == m_c2MethId_GetTermedMsg2))
            {
                TMEngCard4Val& mecvRet = meOwner.mecvStackAtAs<TMEngCard4Val>(c4FirstInd - 1);
                TMEngMemBufVal& mecvBuf = meOwner.mecvStackAtAs<TMEngMemBufVal>(c4FirstInd + 5);

                if (c2MethId == m_c2MethId_GetTermedMsg)
                {
                    mecvRet.c4Value
                    (
                        sdrvGlue.c4GetTermedMsg
                        (
                            *pobjSrc
                            , meOwner.c4StackValAt(c4FirstInd + 1)
                            , c1Term1
                            , c1Term2
                            , meOwner.bStackValAt(c4FirstInd + 4)
                            , mecvBuf.mbufValue()
                        )
                    );
                }
                 else
                {
                    mecvRet.c4Value
                    (
                        sdrvGlue.c4GetTermedMsg2
                        (
                            *pobjSrc
                            , meOwner.c8StackValAt(c4FirstInd + 1)
                            , c1Term1
                            , c1Term2
                            , meOwner.bStackValAt(c4FirstInd + 4)
                            , mecvBuf.mbufValue()
                        )
                    );
                }
            }
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            ThrowAnErr(meOwner, m_c4ErrGetMsg, errToCatch);
        }
    }
     else if ((c2MethId == m_c2MethId_GetASCIIStartStopMsg)
          ||  (c2MethId == m_c2MethId_GetASCIIStartStopMsg2)
          ||  (c2MethId == m_c2MethId_GetStartStopMsg)
          ||  (c2MethId == m_c2MethId_GetStartStopMsg2))
    {
        TObject* pobjSrc = 0;
        TMEngClassVal& mecvSrc = meOwner.mecvStackAtAs<TMEngClassVal>(c4FirstInd);
        if (mecvSrc.c2ClassId() == m_c2TypeId_CommPort)
        {
            TMEngCommPortVal& mecvComm = static_cast<TMEngCommPortVal&>(mecvSrc);
            pobjSrc = &mecvComm.commValue();
        }
         else if (mecvSrc.c2ClassId() == m_c2TypeId_StreamSocket)
        {
            TMEngStreamSocketVal& mecvSock = static_cast<TMEngStreamSocketVal&>(mecvSrc);
            pobjSrc = &mecvSock.sockStreamValue(meOwner);
        }
         else
        {
            // Dunno what this is
            ThrowAnErr
            (
                meOwner
                , m_c4ErrUnknownConnType
                , meOwner.meciFind(mecvSrc.c2ClassId()).strClassPath()
            );
        }
        const tCIDLib::TCard1 c1Start = meOwner.c1StackValAt(c4FirstInd + 2);
        const tCIDLib::TCard1 c1End = meOwner.c1StackValAt(c4FirstInd + 3);

        try
        {
            if ((c2MethId == m_c2MethId_GetASCIIStartStopMsg)
            ||  (c2MethId == m_c2MethId_GetASCIIStartStopMsg2))
            {
                TMEngBooleanVal& mecvRet = meOwner.mecvStackAtAs<TMEngBooleanVal>(c4FirstInd - 1);
                TMEngStringVal& mecvStr = meOwner.mecvStackAtAs<TMEngStringVal>(c4FirstInd + 4);

                if (c2MethId == m_c2MethId_GetASCIIStartStopMsg)
                {
                    mecvRet.bValue
                    (
                        sdrvGlue.bGetASCIIStartStopMsg
                        (
                            *pobjSrc
                            , meOwner.c4StackValAt(c4FirstInd + 1)
                            , c1Start
                            , c1End
                            , mecvStr.strValue()
                        )
                    );
                }
                 else
                {
                    mecvRet.bValue
                    (
                        sdrvGlue.bGetASCIIStartStopMsg2
                        (
                            *pobjSrc
                            , meOwner.c8StackValAt(c4FirstInd + 1)
                            , c1Start
                            , c1End
                            , mecvStr.strValue()
                        )
                    );
                }
            }
             else if ((c2MethId == m_c2MethId_GetStartStopMsg)
                  ||  (c2MethId == m_c2MethId_GetStartStopMsg2))
            {
                const tCIDLib::TCard4 c4MaxBytes = meOwner.c4StackValAt(c4FirstInd + 4);
                TMEngCard4Val& mecvRet = meOwner.mecvStackAtAs<TMEngCard4Val>(c4FirstInd - 1);
                TMEngMemBufVal& mecvBuf = meOwner.mecvStackAtAs<TMEngMemBufVal>(c4FirstInd + 5);

                if (c2MethId == m_c2MethId_GetStartStopMsg)
                {
                    mecvRet.c4Value
                    (
                        sdrvGlue.c4GetStartStopMsg
                        (
                            *pobjSrc
                            , meOwner.c4StackValAt(c4FirstInd + 1)
                            , c1Start
                            , c1End
                            , c4MaxBytes
                            , mecvBuf.mbufValue()
                        )
                    );
                }
                 else
                {
                    mecvRet.c4Value
                    (
                        sdrvGlue.c4GetStartStopMsg2
                        (
                            *pobjSrc
                            , meOwner.c8StackValAt(c4FirstInd + 1)
                            , c1Start
                            , c1End
                            , c4MaxBytes
                            , mecvBuf.mbufValue()
                        )
                    );
                }
            }
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            ThrowAnErr(meOwner, m_c4ErrGetMsg, errToCatch);
        }
    }
     else if (c2MethId == m_c2MethId_GetConfigStr)
    {
        //
        //  Build the path for this driver. They have a fixed format with
        //  just the moniker being used as the differentiator.
        //
        TString strKey(kCQCKit::pszOSKey_CQCSrvDriverCfgs);
        strKey.Append(sdrvGlue.strMoniker());
        TMEngStringVal& mecvOut = meOwner.mecvStackAtAs<TMEngStringVal>(c4FirstInd);

        try
        {
            // Get a local config server proxy
            TCfgServerClient cfgsGet(4000);

            tCIDLib::TCard4 c4Ver = 0;
            tCIDLib::ELoadRes eRes = cfgsGet.eReadObject
            (
                strKey
                , mecvOut.strValue()
                , c4Ver
            );
            TMEngBooleanVal& mecvRet = meOwner.mecvStackAtAs<TMEngBooleanVal>(c4FirstInd - 1);
            mecvRet.bValue(eRes != tCIDLib::ELoadRes::NotFound);
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            ThrowAnErr(meOwner, m_c4ErrGetCfgStr, errToCatch);
        }
    }
     else if (c2MethId == m_c2MethId_GetDriverInfo)
    {
        TMEngStringVal& mecvMake = meOwner.mecvStackAtAs<TMEngStringVal>(c4FirstInd);
        TMEngStringVal& mecvModel = meOwner.mecvStackAtAs<TMEngStringVal>(c4FirstInd + 1);
        TMEngCard4Val& mecvMajVer = meOwner.mecvStackAtAs<TMEngCard4Val>(c4FirstInd + 2);
        TMEngCard4Val& mecvMinVer = meOwner.mecvStackAtAs<TMEngCard4Val>(c4FirstInd + 3);

        // We eat the driver state and device category
        tCQCKit::EDevCats eCat;
        TString strMoniker;

        tCIDLib::TCard4 c4MajVer, c4MinVer;
        sdrvGlue.QueryDrvInfo
        (
            strMoniker
            , mecvMake.strValue()
            , mecvModel.strValue()
            , c4MajVer
            , c4MinVer
            , eCat
        );

        mecvMajVer.c4Value(c4MajVer);
        mecvMinVer.c4Value(c4MinVer);
    }
     else if (c2MethId == m_c2MethId_GetFldErrState)
    {
        try
        {
            TMEngBooleanVal& mecvRet = meOwner.mecvStackAtAs<TMEngBooleanVal>(c4FirstInd - 1);
            mecvRet.bValue(sdrvGlue.bFieldInErr(meOwner.c4StackValAt(c4FirstInd)));
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            ThrowAnErr(meOwner, m_c4ErrFldNotFound, errToCatch);
        }
    }
     else if (c2MethId == m_c2MethId_GetFldInfoById)
    {
        try
        {
            c4FldId = meOwner.c4StackValAt(c4FirstInd);
            TCQCFldDef flddRet = sdrvGlue.flddFromId(c4FldId);

            // Copy this info to the caller's output parm
            TCQCFldDefVal& mecvDef = meOwner.mecvStackAtAs<TCQCFldDefVal>(c4FirstInd + 1);
            mecvDef.flddValue(flddRet);
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            if (errToCatch.bCheckEvent( facCQCKit().strName()
                                        , kKitErrs::errcFld_UnknownFldId))
            {
                // It's not a known fld id
                ThrowAnErr(meOwner, m_c4ErrFldIdNotFound, TCardinal(c4FldId));
            }
             else
            {
                // Throw a generic error
                ThrowAnErr(meOwner, m_c4ErrCppExcept, errToCatch);
            }
        }

        // Give back the field id is as the return
        TMEngCard4Val& mecvRet = meOwner.mecvStackAtAs<TMEngCard4Val>(c4FirstInd - 1);
        mecvRet.c4Value(c4FldId);
    }
     else if (c2MethId == m_c2MethId_GetFldInfoByName)
    {
        try
        {
            c4FldId = c4CheckFldName(meOwner, sdrvGlue, c4FirstInd);
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            ThrowAnErr(meOwner, m_c4ErrAccessFailed, errToCatch);
        }

        try
        {
            // Get the field def for this field
            TCQCFldDef flddRet = sdrvGlue.flddFromId(c4FldId);

            // Copy this info to the caller's output parm
            TCQCFldDefVal& mecvDef = meOwner.mecvStackAtAs<TCQCFldDefVal>(c4FirstInd + 1);
            mecvDef.flddValue(flddRet);
        }

        catch(TError& errToCatch)
        {
            if (errToCatch.bCheckEvent( facCQCKit().strName()
                                        , kKitErrs::errcFld_UnknownFldId))
            {
                // It's not a known fld id
                ThrowAnErr(meOwner, m_c4ErrFldIdNotFound, TCardinal(c4FldId));
            }
             else
            {
                // Throw a generic error
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                ThrowAnErr(meOwner, m_c4ErrCppExcept, errToCatch);
            }
        }

        // Give back the field id is as the return
        TMEngCard4Val& mecvRet = meOwner.mecvStackAtAs<TMEngCard4Val>(c4FirstInd - 1);
        mecvRet.c4Value(c4FldId);
    }
     else if (c2MethId == m_c2MethId_GetFldName)
    {
        try
        {
            c4FldId = meOwner.c4StackValAt(c4FirstInd);
            TMEngStringVal& mecvRet = meOwner.mecvStackAtAs<TMEngStringVal>(c4FirstInd - 1);
            tCQCKit::EFldTypes eType;
            sdrvGlue.QueryFldName(c4FldId, eType, mecvRet.strValue());
        }

        catch(TError& errToCatch)
        {
            if (errToCatch.bCheckEvent( facCQCKit().strName()
                                        , kKitErrs::errcFld_UnknownFldId))
            {
                // It's not a known fld id
                ThrowAnErr(meOwner, m_c4ErrFldIdNotFound, TCardinal(c4FldId));
            }
             else
            {
                // Throw a generic error
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                ThrowAnErr(meOwner, m_c4ErrCppExcept, errToCatch);
            }
        }
    }
     else if (c2MethId == m_c2MethId_GetIsConnected)
    {
        TMEngBooleanVal& mecvRet = meOwner.mecvStackAtAs<TMEngBooleanVal>(c4FirstInd - 1);
        mecvRet.bValue(sdrvGlue.eState() == tCQCKit::EDrvStates::Connected);
    }
     else if (c2MethId == m_c2MethId_GetMoniker)
    {
        // Just return the moniker of the driver object
        TMEngStringVal& mecvRet = meOwner.mecvStackAtAs<TMEngStringVal>(c4FirstInd - 1);
        mecvRet.strValue(sdrvGlue.strMoniker());
    }
     else if ((c2MethId == m_c2MethId_GetStartLenMsg)
          ||  (c2MethId == m_c2MethId_GetStartLenMsg2))
    {
        TObject* pobjSrc = 0;
        TMEngClassVal& mecvSrc = meOwner.mecvStackAtAs<TMEngClassVal>(c4FirstInd);
        if (mecvSrc.c2ClassId() == m_c2TypeId_CommPort)
        {
            TMEngCommPortVal& mecvComm = static_cast<TMEngCommPortVal&>(mecvSrc);
            pobjSrc = &mecvComm.commValue();
        }
         else if (mecvSrc.c2ClassId() == m_c2TypeId_StreamSocket)
        {
            TMEngStreamSocketVal& mecvSock = static_cast<TMEngStreamSocketVal&>(mecvSrc);
            pobjSrc = &mecvSock.sockStreamValue(meOwner);
        }
         else
        {
            // Dunno what this is
            ThrowAnErr
            (
                meOwner
                , m_c4ErrUnknownConnType
                , meOwner.meciFind(mecvSrc.c2ClassId()).strClassPath()
            );
        }
        const tCIDLib::TCard1 c1Start = meOwner.c1StackValAt(c4FirstInd + 2);
        const tCIDLib::TCard4 c4LenOfs = meOwner.c4StackValAt(c4FirstInd + 3);
        const tCIDLib::TCard4 c4LenSub = meOwner.c4StackValAt(c4FirstInd + 4);
        const tCIDLib::TCard4 c4TrailBytes = meOwner.c4StackValAt(c4FirstInd + 5);
        const tCIDLib::TCard4 c4MaxBytes = meOwner.c4StackValAt(c4FirstInd + 6);
        TMEngMemBufVal& mecvOut = meOwner.mecvStackAtAs<TMEngMemBufVal>(c4FirstInd + 7);
        TMEngCard4Val& mecvRet = meOwner.mecvStackAtAs<TMEngCard4Val>(c4FirstInd - 1);

        try
        {
            if (c2MethId == m_c2MethId_GetStartLenMsg)
            {
                mecvRet.c4Value
                (
                    sdrvGlue.c4GetStartLenMsg
                    (
                        *pobjSrc
                        , meOwner.c4StackValAt(c4FirstInd + 1)
                        , c1Start
                        , c4LenOfs
                        , c4LenSub
                        , c4TrailBytes
                        , c4MaxBytes
                        , mecvOut.mbufValue()
                    )
                );
            }
             else
            {
                mecvRet.c4Value
                (
                    sdrvGlue.c4GetStartLenMsg2
                    (
                        *pobjSrc
                        , meOwner.c8StackValAt(c4FirstInd + 1)
                        , c1Start
                        , c4LenOfs
                        , c4LenSub
                        , c4TrailBytes
                        , c4MaxBytes
                        , mecvOut.mbufValue()
                    )
                );
            }
        }

        catch(const TError& errToCatch)
        {
            ThrowAnErr(meOwner, m_c4ErrGetMsg, errToCatch);
        }
    }
     else if (c2MethId == m_c2MethId_GetVerboseLevel)
    {
        // Get the enumerated return value, which will be of the VerboseLvl type
        TMEngEnumVal& mecvRet = meOwner.mecvStackAtAs<TMEngEnumVal>(c4FirstInd - 1);
        mecvRet.c4Ordinal(tCIDLib::c4EnumOrd(sdrvGlue.eVerboseLevel()));
    }
     else if (c2MethId == m_c2MethId_IncBadMsgs)
    {
        // Driver field locks for us on this
        sdrvGlue.IncBadMsgCounter();
    }
     else if (c2MethId == m_c2MethId_IncFailedWrite)
    {
        // Driver field locks for us on this
        sdrvGlue.IncFailedWriteCounter();
    }
     else if (c2MethId == m_c2MethId_IncNaks)
    {
        // Driver field locks for us on this
        sdrvGlue.IncNakCounter();
    }
     else if (c2MethId == m_c2MethId_IncReconfigured)
    {
        // Driver field locks for us on this
        sdrvGlue.IncReconfigCounter();
    }
     else if (c2MethId == m_c2MethId_IncTimeouts)
    {
        // Driver field locks for us on this
        sdrvGlue.IncTimeoutCounter();
    }
     else if (c2MethId == m_c2MethId_IncUnknownMsgs)
    {
        // Driver field locks for us on this
        sdrvGlue.IncUnknownMsgCounter();
    }
     else if (c2MethId == m_c2MethId_IncUnknownWrite)
    {
        // Driver field locks for us on this
        sdrvGlue.IncUnknownWriteCounter();
    }
     else if ((c2MethId == m_c2MethId_InitOther)
          ||  (c2MethId == m_c2MethId_InitSerial)
          ||  (c2MethId == m_c2MethId_InitSocket))
    {
        // Throw an exception, since they didn't override the init
        ThrowAnErr(meOwner, m_c4ErrNoOverride, TString(L"Driver init"));
    }
     else if ((c2MethId == m_c2MethId_LogMsg)
          ||  (c2MethId == m_c2MethId_LogMsg1)
          ||  (c2MethId == m_c2MethId_LogMsg2)
          ||  (c2MethId == m_c2MethId_LogMsg3))
    {
        //
        //  Get the message to a temp first. We may have to do token
        //  replacement here.
        //
        TString strMsg(meOwner.strStackValAt(c4FirstInd));
        ReplaceTokens
        (
            meOwner
            , strMsg
            , (c2MethId - m_c2MethId_LogMsg)
            , c4FirstInd + 1
        );

        TError errToLog
        (
            facCQCGenDrvS().strName()
            , meOwner.strClassPathForId(mecvInstance.c2ClassId())
            , meOwner.c4CurLine()
            , strMsg
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
        TModule::LogEventObj(errToLog);
    }
     else if ((c2MethId == m_c2MethId_QueryBoolVal)
          ||  (c2MethId == m_c2MethId_QueryCardVal)
          ||  (c2MethId == m_c2MethId_QueryIntVal)
          ||  (c2MethId == m_c2MethId_QueryTextVal))
    {
        // They were called but the driver didn't provide them
        TMEngMethodInfo& methiBad = methiFind(c2MethId);
        ThrowAnErr(meOwner, m_c4ErrNoOverride, methiBad.strName());
    }
     else if (c2MethId == m_c2MethId_ReadBoolFld)
    {
        c4FldId = meOwner.c4StackValAt(c4FirstInd);
        TMEngBooleanVal& mecvRet = meOwner.mecvStackAtAs<TMEngBooleanVal>(c4FirstInd - 1);
        tCIDLib::TBoolean bValue;

        sdrvGlue.bReadBoolFld(c4FldId, c4SerialNum, bValue, kCIDLib::False);
        mecvRet.bValue(bValue);
    }
     else if (c2MethId == m_c2MethId_ReadBoolFldByName)
    {
        TMEngBooleanVal& mecvRet = meOwner.mecvStackAtAs<TMEngBooleanVal>(c4FirstInd - 1);
        c4FldId = c4CheckFldName(meOwner, sdrvGlue, c4FirstInd);
        tCIDLib::TBoolean bValue;
        sdrvGlue.bReadBoolFld(c4FldId, c4SerialNum, bValue, kCIDLib::False);
        mecvRet.bValue(bValue);
    }
     else if (c2MethId == m_c2MethId_ReadCardFld)
    {
        c4FldId = meOwner.c4StackValAt(c4FirstInd);
        TMEngCard4Val& mecvRet = meOwner.mecvStackAtAs<TMEngCard4Val>(c4FirstInd - 1);
        tCIDLib::TCard4 c4Value;
        sdrvGlue.bReadCardFld(c4FldId, c4SerialNum, c4Value, kCIDLib::False);
        mecvRet.c4Value(c4Value);
    }
     else if (c2MethId == m_c2MethId_ReadCardFldByName)
    {
        TMEngCard4Val& mecvRet = meOwner.mecvStackAtAs<TMEngCard4Val>(c4FirstInd - 1);
        c4FldId = c4CheckFldName(meOwner, sdrvGlue, c4FirstInd);
        tCIDLib::TCard4 c4Value;
        sdrvGlue.bReadCardFld(c4FldId, c4SerialNum, c4Value, kCIDLib::False);
        mecvRet.c4Value(c4Value);
    }
     else if (c2MethId == m_c2MethId_ReadFloatFld)
    {
        c4FldId = meOwner.c4StackValAt(c4FirstInd);
        TMEngFloat8Val& mecvRet = meOwner.mecvStackAtAs<TMEngFloat8Val>(c4FirstInd - 1);
        tCIDLib::TFloat8 f8Value;
        sdrvGlue.bReadFloatFld(c4FldId, c4SerialNum, f8Value, kCIDLib::False);
        mecvRet.f8Value(f8Value);
    }
     else if (c2MethId == m_c2MethId_ReadFloatFldByName)
    {
        TMEngFloat8Val& mecvRet = meOwner.mecvStackAtAs<TMEngFloat8Val>(c4FirstInd - 1);
        c4FldId = c4CheckFldName(meOwner, sdrvGlue, c4FirstInd);
        tCIDLib::TFloat8 f8Value;
        sdrvGlue.bReadFloatFld(c4FldId, c4SerialNum, f8Value, kCIDLib::False);
        mecvRet.f8Value(f8Value);
    }
     else if (c2MethId == m_c2MethId_ReadIntFld)
    {
        c4FldId = meOwner.c4StackValAt(c4FirstInd);
        TMEngInt4Val& mecvRet = meOwner.mecvStackAtAs<TMEngInt4Val>(c4FirstInd - 1);
        tCIDLib::TInt4 i4Value;
        sdrvGlue.bReadIntFld(c4FldId, c4SerialNum, i4Value, kCIDLib::False);
        mecvRet.i4Value(i4Value);
    }
     else if (c2MethId == m_c2MethId_ReadIntFldByName)
    {
        TMEngInt4Val& mecvRet = meOwner.mecvStackAtAs<TMEngInt4Val>(c4FirstInd - 1);
        c4FldId = c4CheckFldName(meOwner, sdrvGlue, c4FirstInd);
        tCIDLib::TInt4      i4Value;
        tCIDLib::TCard4     c4SerialNum = 0;
        sdrvGlue.bReadIntFld(c4FldId, c4SerialNum, i4Value, kCIDLib::False);
        mecvRet.i4Value(i4Value);
    }
     else if (c2MethId == m_c2MethId_ReadStringFld)
    {
        c4FldId = meOwner.c4StackValAt(c4FirstInd);
        TMEngStringVal& mecvRet = meOwner.mecvStackAtAs<TMEngStringVal>(c4FirstInd - 1);
        sdrvGlue.bReadStringFld(c4FldId, c4SerialNum, mecvRet.strValue(), kCIDLib::False);
    }
     else if (c2MethId == m_c2MethId_ReadStringFldByName)
    {
        TMEngStringVal& mecvRet = meOwner.mecvStackAtAs<TMEngStringVal>(c4FirstInd - 1);
        c4FldId = c4CheckFldName(meOwner, sdrvGlue, c4FirstInd);
        sdrvGlue.bReadStringFld(c4FldId, c4SerialNum, mecvRet.strValue(), kCIDLib::False);
    }
     else if (c2MethId == m_c2MethId_ReadStrListFld)
    {
        c4FldId = meOwner.c4StackValAt(c4FirstInd);
        TMEngStrListVal& mecvToFill = meOwner.mecvStackAtAs<TMEngStrListVal>(c4FirstInd + 1);
        sdrvGlue.bReadSListFld(c4FldId, c4SerialNum, mecvToFill.colValue(), kCIDLib::False);
    }
     else if (c2MethId == m_c2MethId_ReadStrListFldByName)
    {
        c4FldId = c4CheckFldName(meOwner, sdrvGlue, c4FirstInd);
        TMEngStrListVal& mecvToFill = meOwner.mecvStackAtAs<TMEngStrListVal>(c4FirstInd + 1);
        sdrvGlue.bReadSListFld(c4FldId, c4SerialNum, mecvToFill.colValue(), kCIDLib::False);
    }
     else if (c2MethId == m_c2MethId_ReadTimeFld)
    {
        c4FldId = meOwner.c4StackValAt(c4FirstInd);
        TMEngCard8Val& mecvRet = meOwner.mecvStackAtAs<TMEngCard8Val>(c4FirstInd - 1);
        tCIDLib::TCard8 c8Val;
        sdrvGlue.bReadTimeFld(c4FldId, c4SerialNum, c8Val, kCIDLib::False);
        mecvRet.c8Value(c8Val);
    }
     else if (c2MethId == m_c2MethId_ReadTimeFldByName)
    {
        TMEngCard8Val& mecvRet = meOwner.mecvStackAtAs<TMEngCard8Val>(c4FirstInd - 1);
        c4FldId = c4CheckFldName(meOwner, sdrvGlue, c4FirstInd);
        tCIDLib::TCard8 c8Val;
        sdrvGlue.bReadTimeFld(c4FldId, c4SerialNum, c8Val, kCIDLib::False);
        mecvRet.c8Value(c8Val);
    }
     else if ((c2MethId == m_c2MethId_SendLoadEv)
          ||  (c2MethId == m_c2MethId_SendLoadEv2))
    {
        TString strName;
        if (c2MethId == m_c2MethId_SendLoadEv2)
            strName = meOwner.strStackValAt(c4FirstInd + 3);

        m_strTmp.SetFormatted(meOwner.c4StackValAt(c4FirstInd + 1));
        sdrvGlue.QueueEventTrig
        (
            tCQCKit::EStdDrvEvs::LoadChange
            , meOwner.strStackValAt(c4FirstInd + 2)
            , facCQCKit().strBoolOffOn(meOwner.bStackValAt(c4FirstInd))
            , m_strTmp
            , strName
            , TString::strEmpty()
        );
    }
     else if (c2MethId == m_c2MethId_SendLockEv)
    {
        sdrvGlue.QueueEventTrig
        (
            tCQCKit::EStdDrvEvs::LockStatus
            , TString::strEmpty()
            , meOwner.bStackValAt(c4FirstInd) ? L"locked" : L"unlocked"
            , meOwner.strStackValAt(c4FirstInd + 1)
            , meOwner.strStackValAt(c4FirstInd + 2)
            , meOwner.strStackValAt(c4FirstInd + 3)
        );
    }
     else if ((c2MethId == m_c2MethId_SendMotionEv)
          ||  (c2MethId == m_c2MethId_SendMotionEv2))
    {
        TString strName;
        if (c2MethId == m_c2MethId_SendMotionEv2)
            strName = meOwner.strStackValAt(c4FirstInd + 3);

        m_strTmp.SetFormatted(meOwner.c4StackValAt(c4FirstInd + 1));
        sdrvGlue.QueueEventTrig
        (
            tCQCKit::EStdDrvEvs::Motion
            , meOwner.strStackValAt(c4FirstInd + 2)
            , facCQCKit().strBoolStartEnd(meOwner.bStackValAt(c4FirstInd))
            , m_strTmp
            , strName
            , TString::strEmpty()
        );
    }
     else if (c2MethId == m_c2MethId_SendPresenceEv)
    {
        sdrvGlue.QueueEventTrig
        (
            tCQCKit::EStdDrvEvs::Presence
            , TString::strEmpty()
            , meOwner.bStackValAt(c4FirstInd) ? L"enter" : L"exit"
            , meOwner.strStackValAt(c4FirstInd + 1)
            , meOwner.strStackValAt(c4FirstInd + 2)
            , TString::strEmpty()
        );
    }
     else if (c2MethId == m_c2MethId_SendUserActEv)
    {
        // Send a user action event with the two string parms
        sdrvGlue.QueueEventTrig
        (
            tCQCKit::EStdDrvEvs::UserAction
            , TString::strEmpty()
            , meOwner.strStackValAt(c4FirstInd)
            , meOwner.strStackValAt(c4FirstInd + 1)
            , TString::strEmpty()
            , TString::strEmpty()
        );
    }
     else if ((c2MethId == m_c2MethId_SendZoneEv)
          ||  (c2MethId == m_c2MethId_SendZoneEv2))
    {
        // Check the state
        const TMEngEnumVal& mecvState = meOwner.mecvStackAtAs<TMEngEnumVal>(c4FirstInd);
        const tCQCKit::EZoneStates eState = (tCQCKit::EZoneStates)mecvState.c4Ordinal();

        // If the v2 version get the zone name, else leave blank
        TString strName;
        if (c2MethId == m_c2MethId_SendZoneEv2)
            strName = meOwner.strStackValAt(c4FirstInd + 3);

        // Format out the zone number
        m_strTmp.SetFormatted(meOwner.c4StackValAt(c4FirstInd + 1));

        sdrvGlue.QueueEventTrig
        (
            tCQCKit::EStdDrvEvs::ZoneAlarm
            , meOwner.strStackValAt(c4FirstInd + 2)
            , tCQCKit::strAltXlatEZoneStates(eState)
            , m_strTmp
            , strName
            , TString::strEmpty()
        );
    }
     else if (c2MethId == m_c2MethId_SetFldErrState)
    {
        try
        {
            sdrvGlue.SetFieldErr
            (
                meOwner.c4StackValAt(c4FirstInd)
                , meOwner.bStackValAt(c4FirstInd + 1)
            );
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            ThrowAnErr(meOwner, m_c4ErrFldNotFound, errToCatch);
        }
    }
     else if (c2MethId == m_c2MethId_SetAllErrStates)
    {
        // Not legal if already connected
        if (sdrvGlue.eState() >= tCQCKit::EDrvStates::Connected)
            ThrowAnErr(meOwner, m_c4ErrAlreadyConnected, TString(L" set all error states"));

        // Just in case... still catch errors
        try
        {
            sdrvGlue.SetAllErrStates();
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            ThrowAnErr(meOwner, m_c4ErrCppExcept, errToCatch);
        }
    }
     else if (c2MethId == m_c2MethId_SetConfigStr)
    {
        //
        //  Build the path for this driver. They have a fixed format with
        //  just the moniker being used as the differentiator.
        //
        TString strKey(kCQCKit::pszOSKey_CQCSrvDriverCfgs);
        strKey.Append(sdrvGlue.strMoniker());

        try
        {
            // Get a local config server proxy
            TCfgServerClient cfgsGet(4000);

            // And store the passed string
            tCIDLib::TCard4 c4Ver = 0;
            cfgsGet.bAddOrUpdate
            (
                strKey, c4Ver, meOwner.strStackValAt(c4FirstInd), 256
            );
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            ThrowAnErr(meOwner, m_c4ErrSetCfgStr, errToCatch);
        }
    }
     else if (c2MethId == m_c2MethId_SetFields)
    {
        // Get the field list parameter, which is a vector of field infos
        TMEngVectorVal& mecvList = meOwner.mecvStackAtAs<TMEngVectorVal>(c4FirstInd);

        //
        //  We cannot pass the data directly in the form we are getting it,
        //  so we have to make copies into our own collection. But this is
        //  rarely done and there are generally never more than few tens of
        //  objects, and they are not complex.
        //
        const tCIDLib::TCard4 c4Count = mecvList.c4ElemCount();
        TVector<TCQCFldDef> colFlds(c4Count);
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            colFlds.objAdd
            (
                static_cast<const TCQCFldDefVal&>(mecvList.mecvAt(meOwner, c4Index)).flddValue()
            );
        }

        try
        {
            // And now set our field
            sdrvGlue.SetDrvFields(colFlds);
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            ThrowAnErr(meOwner, m_c4ErrFldRegFailed, errToCatch);
        }
    }
     else if (c2MethId == m_c2MethId_SetPollTimes)
    {
        // If we are already past the connected stage, this isn't legal
        if (sdrvGlue.eState() > tCQCKit::EDrvStates::Connected)
            ThrowAnErr(meOwner, m_c4ErrAlreadyConfiged);

        sdrvGlue.SetPollTimes
        (
            meOwner.c4StackValAt(c4FirstInd)
            , meOwner.c4StackValAt(c4FirstInd + 1)
        );
    }
     else if (c2MethId == m_c2MethId_SetTOExtension)
    {
        sdrvGlue.enctExtend(meOwner.c8StackValAt(c4FirstInd));
    }
     else if (c2MethId == m_c2MethId_SetVerboseLevel)
    {
        const TMEngEnumVal& mecvSet = meOwner.mecvStackAtAs<TMEngEnumVal>(c4FirstInd);

        // Set a new verbosity level
        sdrvGlue.SetNewVerbosity(tCQCKit::EVerboseLvls(mecvSet.c4Ordinal()));
    }
     else if (c2MethId == m_c2MethId_Simulate)
    {
        //
        //  Now let's do the simulation loop. We won't come back from here
        //  until the user breaks out via the IDE.
        //
        //  Note that we clean up the impl object because he doesn't adopt,
        //  since in the normal scenario it's the impl object that creates
        //  us.
        //
        TCQCGenDrvGlue* psdrvTarget = new TCQCGenDrvGlue;
        TJanitor<TCQCGenDrvGlue> janGlue(psdrvTarget);

        try
        {
            mecvGlue.Initialize(psdrvTarget);
            psdrvTarget->Simulate(meOwner, mecvActual, *this);
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            ThrowAnErr(meOwner, m_c4ErrCppExcept, errToCatch);
        }

        catch(const TDbgExitException&)
        {
            throw;
        }

        catch(const TExceptException&)
        {
            throw;
        }

        catch(...)
        {
            ThrowAnErr(meOwner, m_c4ErrUnknownExcept);
        }
    }
     else if (c2MethId == m_c2MethId_Terminate)
    {
        //
        //  The derived class doesn't have to implement this one so we
        //  just do nothing if they don't implement it.
        //
    }
     else if (c2MethId == m_c2MethId_UseDefFldValue)
    {
        c4FldId = meOwner.c4StackValAt(c4FirstInd);
        sdrvGlue.UseDefFldValue(c4FldId);
    }
     else if (c2MethId == m_c2MethId_VerbosityChanged)
    {
        // Empty default implementation if not overridden
    }
     else
    {
        //
        //  NOTE:   We only have to get a field lock here. Since this is
        //          the driver itself storing the value, there will be no
        //          callback to the driver if a changed value is stored. So
        //          there's no need for a call lock.
        //
        tCIDLib::TBoolean bByName = kCIDLib::False;
        try
        {
            tCIDLib::TBoolean bRes;
            if (c2MethId == m_c2MethId_WriteBool)
            {
                c4FldId = meOwner.c4StackValAt(c4FirstInd);
                CheckFldType(meOwner, sdrvGlue, c4FldId, tCQCKit::EFldTypes::Boolean);
                const tCIDLib::TBoolean bValue = meOwner.bStackValAt(c4FirstInd + 1);
                bRes = sdrvGlue.bStoreBoolFld(c4FldId, bValue, kCIDLib::True);
            }
             else if (c2MethId == m_c2MethId_WriteBoolByName)
            {
                bByName = kCIDLib::True;
                const TString& strName = meOwner.strStackValAt(c4FirstInd);
                c4FldId = c4CheckFldType(meOwner, sdrvGlue, strName, tCQCKit::EFldTypes::Boolean);
                const tCIDLib::TBoolean bValue = meOwner.bStackValAt(c4FirstInd + 1);
                bRes = sdrvGlue.bStoreBoolFld(c4FldId, bValue, kCIDLib::True);
            }
             else if (c2MethId == m_c2MethId_WriteCard)
            {
                c4FldId = meOwner.c4StackValAt(c4FirstInd);
                CheckFldType(meOwner, sdrvGlue, c4FldId, tCQCKit::EFldTypes::Card);
                const tCIDLib::TCard4 c4Value = meOwner.c4StackValAt(c4FirstInd + 1);
                bRes = sdrvGlue.bStoreCardFld(c4FldId, c4Value, kCIDLib::True);
            }
             else if (c2MethId == m_c2MethId_WriteCardByName)
            {
                bByName = kCIDLib::True;
                const TString& strName = meOwner.strStackValAt(c4FirstInd);
                c4FldId = c4CheckFldType(meOwner, sdrvGlue, strName, tCQCKit::EFldTypes::Card);
                const tCIDLib::TCard4 c4Value = meOwner.c4StackValAt(c4FirstInd + 1);
                bRes = sdrvGlue.bStoreCardFld(c4FldId, c4Value, kCIDLib::True);
            }
             else if (c2MethId == m_c2MethId_WriteFloat)
            {
                c4FldId = meOwner.c4StackValAt(c4FirstInd);
                CheckFldType(meOwner, sdrvGlue, c4FldId, tCQCKit::EFldTypes::Float);
                const tCIDLib::TFloat8 f8Value = meOwner.f8StackValAt(c4FirstInd + 1);
                bRes = sdrvGlue.bStoreFloatFld(c4FldId, f8Value, kCIDLib::True);
            }
             else if (c2MethId == m_c2MethId_WriteFloatByName)
            {
                bByName = kCIDLib::True;
                const TString& strName = meOwner.strStackValAt(c4FirstInd);
                c4FldId = c4CheckFldType(meOwner, sdrvGlue, strName, tCQCKit::EFldTypes::Float);
                const tCIDLib::TFloat8 f8Value = meOwner.f8StackValAt(c4FirstInd + 1);
                bRes = sdrvGlue.bStoreFloatFld(c4FldId, f8Value, kCIDLib::True);
            }
             else if (c2MethId == m_c2MethId_WriteInt)
            {
                c4FldId = meOwner.c4StackValAt(c4FirstInd);
                CheckFldType(meOwner, sdrvGlue, c4FldId, tCQCKit::EFldTypes::Int);
                const tCIDLib::TInt4 i4Value = meOwner.i4StackValAt(c4FirstInd + 1);
                bRes = sdrvGlue.bStoreIntFld(c4FldId, i4Value, kCIDLib::True);
            }
             else if (c2MethId == m_c2MethId_WriteIntByName)
            {
                bByName = kCIDLib::True;
                const TString& strName = meOwner.strStackValAt(c4FirstInd);
                c4FldId = c4CheckFldType(meOwner, sdrvGlue, strName, tCQCKit::EFldTypes::Int);
                const tCIDLib::TInt4 i4Value = meOwner.i4StackValAt(c4FirstInd + 1);
                bRes = sdrvGlue.bStoreIntFld(c4FldId, i4Value, kCIDLib::True);
            }
             else if (c2MethId == m_c2MethId_WriteString)
            {
                c4FldId = meOwner.c4StackValAt(c4FirstInd);
                CheckFldType(meOwner, sdrvGlue, c4FldId, tCQCKit::EFldTypes::String);
                const TString& strValue = meOwner.strStackValAt(c4FirstInd + 1);
                bRes = sdrvGlue.bStoreStringFld(c4FldId, strValue, kCIDLib::True);
            }
             else if (c2MethId == m_c2MethId_WriteStringByName)
            {
                bByName = kCIDLib::True;
                const TString& strName = meOwner.strStackValAt(c4FirstInd);
                c4FldId = c4CheckFldType(meOwner, sdrvGlue, strName, tCQCKit::EFldTypes::String);
                const TString& strValue = meOwner.strStackValAt(c4FirstInd + 1);
                bRes = sdrvGlue.bStoreStringFld(c4FldId, strValue, kCIDLib::True);
            }
             else if (c2MethId == m_c2MethId_WriteStrList)
            {
                c4FldId = meOwner.c4StackValAt(c4FirstInd);
                CheckFldType(meOwner, sdrvGlue, c4FldId, tCQCKit::EFldTypes::StringList);
                TMEngStrListVal& mecvList = meOwner.mecvStackAtAs<TMEngStrListVal>(c4FirstInd + 1);
                bRes = sdrvGlue.bStoreSListFld(c4FldId, mecvList.colValue(), kCIDLib::True);
            }
             else if (c2MethId == m_c2MethId_WriteStrListByName)
            {
                bByName = kCIDLib::True;
                const TString& strName = meOwner.strStackValAt(c4FirstInd);
                c4FldId = c4CheckFldType(meOwner, sdrvGlue, strName, tCQCKit::EFldTypes::StringList);
                TMEngStrListVal& mecvList = meOwner.mecvStackAtAs<TMEngStrListVal>(c4FirstInd + 1);
                bRes = sdrvGlue.bStoreSListFld(c4FldId, mecvList.colValue(), kCIDLib::True);
            }
             else if (c2MethId == m_c2MethId_WriteTime)
            {
                c4FldId = meOwner.c4StackValAt(c4FirstInd);
                CheckFldType(meOwner, sdrvGlue, c4FldId, tCQCKit::EFldTypes::Time);
                bRes = sdrvGlue.bStoreTimeFld
                (
                    c4FldId, meOwner.c8StackValAt(c4FirstInd + 1), kCIDLib::True
                );
            }
             else if (c2MethId == m_c2MethId_WriteTimeByName)
            {
                bByName = kCIDLib::True;
                const TString& strName = meOwner.strStackValAt(c4FirstInd);
                c4FldId = c4CheckFldType(meOwner, sdrvGlue, strName, tCQCKit::EFldTypes::Time);
                bRes = sdrvGlue.bStoreTimeFld
                (
                    c4FldId, meOwner.c8StackValAt(c4FirstInd + 1), kCIDLib::True
                );
            }
             else
            {
                return kCIDLib::False;
            }

            // Give back the result
            TMEngBooleanVal& mecvRet = meOwner.mecvStackAtAs<TMEngBooleanVal>(c4FirstInd - 1);
            mecvRet.bValue(bRes);
        }

        catch(TError& errToCatch)
        {
            //
            //  Get the field name. If it's a by name write, it's easy, else
            //  we have to look it up by id.
            //
            TString strName;
            if (bByName)
            {
                strName = meOwner.strStackValAt(c4FirstInd);
            }
             else
            {
                //
                //  It's possible that we got here because we got an invalid
                //  field id, so this could fail.
                //
                try
                {
                    tCQCKit::EFldTypes eType;
                    sdrvGlue.QueryFldName(c4FldId, eType, strName);
                }

                catch(...)
                {
                    strName = L"Field (Id=";
                    strName.AppendFormatted(c4FldId);
                    strName.Append(L')');
                }
            }

            //
            //  Watch for some specific errors and translate them to macro
            //  exceptions, and anything else into a generic field write
            //  macro exception.
            //
            if (errToCatch.bCheckEvent( facCQCKit().strName()
                                        , kKitErrs::errcFld_LimitViolation))
            {
                ThrowAnErr
                (
                    meOwner
                    , m_c4ErrLimitViolation
                    , meOwner.mecvStackAt(c4FirstInd + 1)
                    , strName
                );
            }
             else if (errToCatch.bCheckEvent(facCQCKit().strName()
                                            , kKitErrs::errcFld_CannotConvertVal))
            {
                ThrowAnErr(meOwner, m_c4ErrConvertErr, strName);
            }
             else
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                ThrowAnErr(meOwner, m_c4ErrFldWriteErr, errToCatch);
            }
        }
    }
    return kCIDLib::True;
}


// ---------------------------------------------------------------------------
//  TCQCDriverInfo: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Given a field name on the staci, a method above can call us here to get
//  that field name out and look up the field id of the field. This both
//  confirms it exists and gets the caller the field id it needs.
//
tCIDLib::TCard4
TCQCDriverInfo::c4CheckFldName(         TCIDMacroEngine&    meOwner
                                , const TCQCGenDrvGlue&     sdrvGlue
                                , const tCIDLib::TCard4     c4Index) const
{
    const TString& strToFind = meOwner.strStackValAt(c4Index);

    //
    //  Do a simple field id lookup to test if it exists, which also gets
    //  us our return value.
    //
    tCIDLib::TCard4 c4Id;
    if (!sdrvGlue.bQueryFldId(strToFind, c4Id, kCIDLib::False))
    {
        TString strTmp(m_pmeciErrors->strTextValue(m_c4ErrFldNotFound));
        strTmp.eReplaceToken(strToFind, L'1');

        // Set the exception info on the engine
        meOwner.SetException
        (
            m_c2EnumId_Errors
            , strClassPath()
            , m_c4ErrFldNotFound
            , m_pmeciErrors->strPartialName(m_c4ErrFldNotFound)
            , strTmp
            , meOwner.c4CurLine()
        );

        // And throw the excpetion that represents a macro level exception
        throw TExceptException();
    }
    return c4Id;
}


//
//  Looks up a field's type and id. It verifies that the type matches the
//  one passed, and returns the id if so, else it throws.
//
tCIDLib::TCard4
TCQCDriverInfo::c4CheckFldType(         TCIDMacroEngine&    meOwner
                                , const TCQCGenDrvGlue&     sdrvGlue
                                , const TString&            strToFind
                                , const tCQCKit::EFldTypes  eToCheck) const
{
    tCIDLib::TCard4 c4FldId;

    const tCQCKit::EFldTypes eType = sdrvGlue.eFldTypeByName
    (
        strToFind, c4FldId, kCIDLib::False
    );

    // If not found, throw
    if (eToCheck == tCQCKit::EFldTypes::Count)
    {
        TString strTmp(m_pmeciErrors->strTextValue(m_c4ErrFldNotFound));
        strTmp.eReplaceToken(strToFind, L'1');
        ThrowAnErr(meOwner, m_c4ErrFldNotFound, strTmp);
    }

    if (eType != eToCheck)
        ThrowAnErr(meOwner, m_c4ErrBadFldType, strToFind);

    return c4FldId;
}


//
//  Checks the field type of the indicated field, by id, to make sure it
//  matches the type passed.
//
tCIDLib::TVoid
TCQCDriverInfo::CheckFldType(       TCIDMacroEngine&    meOwner
                            , const TCQCGenDrvGlue&     sdrvGlue
                            , const tCIDLib::TCard4     c4FldId
                            , const tCQCKit::EFldTypes  eToCheck) const
{
    TString strName;
    const tCQCKit::EFldTypes eType = sdrvGlue.eFldTypeById
    (
        c4FldId, strName, kCIDLib::False
    );

    if (eToCheck == tCQCKit::EFldTypes::Count)
    {
        TString strTmp(m_pmeciErrors->strTextValue(m_c4ErrFldIdNotFound));
        strTmp.eReplaceToken(TCardinal(c4FldId), L'1');
        ThrowAnErr(meOwner, m_c4ErrFldIdNotFound, strTmp);
    }

    if (eType != eToCheck)
        ThrowAnErr(meOwner, m_c4ErrBadFldType, strName);
}


//
//  Handles token replacement with formattable objects at the CML level,
//  which we have to massage out to get into our own C++ formattable
//  type.
//
tCIDLib::TVoid
TCQCDriverInfo::ReplaceTokens(          TCIDMacroEngine&    meOwner
                                ,       TString&            strMsg
                                , const tCIDLib::TCard4     c4Tokens
                                , const tCIDLib::TCard4     c4FirstInd)
{
    // Get the actual C++ stream out as it's real type
    TTextStringOutStream& strmActual = static_cast<TTextStringOutStream&>
    (
        m_pmecvStrm->strmTarget(meOwner)
    );

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


tCIDLib::TVoid
TCQCDriverInfo::ThrowAnErr(         TCIDMacroEngine&    meOwner
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

tCIDLib::TVoid
TCQCDriverInfo::ThrowAnErr(         TCIDMacroEngine&    meOwner
                            , const tCIDLib::TCard4     c4ToThrow
                            , const TError&             errCaught) const
{
    if (facCQCGenDrvS().bShouldLog(errCaught))
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
TCQCDriverInfo::ThrowAnErr(         TCIDMacroEngine&    meOwner
                            , const tCIDLib::TCard4     c4ToThrow
                            , const MFormattable&       fmtblToken) const
{
    TString strText(m_pmeciErrors->strTextValue(c4ToThrow));
    strText.eReplaceToken(fmtblToken, kCIDLib::chDigit1);

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
TCQCDriverInfo::ThrowAnErr(         TCIDMacroEngine&    meOwner
                            , const tCIDLib::TCard4     c4ToThrow
                            , const TMEngClassVal&      mecvToken1
                            , const MFormattable&       fmtblToken2) const
{
    TTextStringOutStream strmTmp(1024UL);
    mecvToken1.bDbgFormat
    (
        strmTmp
        , meOwner.meciFind(mecvToken1.c2ClassId())
        , tCIDMacroEng::EDbgFmts::Short
        , tCIDLib::ERadices::Dec
        , meOwner
    );
    strmTmp.Flush();

    TString strText(m_pmeciErrors->strTextValue(c4ToThrow));
    strText.eReplaceToken(strmTmp.strData(), kCIDLib::chDigit1);
    strText.eReplaceToken(fmtblToken2, kCIDLib::chDigit2);

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


