//
// FILE NAME: CQCMEng_EventClass.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/14/2006
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
//  class that wraps a C++ level event object.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCMEng_.hpp"
#include    "CQCMEng_EventClass_.hpp"


// ---------------------------------------------------------------------------
//  Magic RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TEventValue,TMEngClassVal)
RTTIDecls(TEventInfo,TMEngClassInfo)
RTTIDecls(TEventHandlerInfo,TMEngClassInfo)


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace
{
    namespace CQCMEng_Event
    {
        // -----------------------------------------------------------------------
        //  The names for the types that we support here. Each derivative has to
        //  be able to return strings that contain its name and full name.
        // -----------------------------------------------------------------------
        const TString   strBasePath(L"MEng.System.CQC.Runtime");
        const TString   strEvent(L"Event");
        const TString   strEventClassPath(L"MEng.System.CQC.Runtime.Event");
        const TString   strEventHandler(L"EventHandler");
        const TString   strEventHandlerClassPath(L"MEng.System.CQC.Runtime.EventHandler");
        const TString   strHandlerData(L"m_EventData");
    }
}



// ---------------------------------------------------------------------------
//  CLASS: TEventValue
// PREFIX: mecv
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TEventValue: Constuctors and Destructor
// ---------------------------------------------------------------------------
TEventValue::TEventValue(const  TString&                strName
                        , const tCIDLib::TCard2         c2ClassId
                        , const tCIDMacroEng::EConstTypes  eConst) :

    TMEngClassVal(strName, c2ClassId, eConst)
{
}

TEventValue::~TEventValue()
{
}


// ---------------------------------------------------------------------------
//  TCQCEvent: Public, non-virtual methods
// ---------------------------------------------------------------------------
const TCQCEvent& TEventValue::cevValue() const
{
    return m_cevValue;
}

TCQCEvent& TEventValue::cevValue()
{
    return m_cevValue;
}

TCQCEvent& TEventValue::cevValue(const TCQCEvent& cevToSet)
{
    m_cevValue = cevToSet;
    return m_cevValue;
}




// ---------------------------------------------------------------------------
//  CLASS: TEventInfo
// PREFIX: meci
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TEventInfo: Public, static methods
// ---------------------------------------------------------------------------
const TString& TEventInfo::strClassPath()
{
    return CQCMEng_Event::strEventClassPath;
}


// ---------------------------------------------------------------------------
//  TEventInfo: Constructors and Destructor
// ---------------------------------------------------------------------------
TEventInfo::TEventInfo(TCIDMacroEngine& meOwner) :

    TMEngClassInfo
    (
        CQCMEng_Event::strEvent
        , CQCMEng_Event::strBasePath
        , meOwner
        , kCIDLib::True
        , tCIDMacroEng::EClassExt::NonFinal
        , L"MEng.Object"
    )
    , m_c2EnumId_Errors(kCIDMacroEng::c2BadId)
    , m_c2EnumId_StdDrvEvs(kCIDMacroEng::c2BadId)
    , m_c2MethId_AddBlock(kCIDMacroEng::c2BadId)
    , m_c2MethId_AddValue(kCIDMacroEng::c2BadId)
    , m_c2MethId_AsBool(kCIDMacroEng::c2BadId)
    , m_c2MethId_AsCard4(kCIDMacroEng::c2BadId)
    , m_c2MethId_AsCard8(kCIDMacroEng::c2BadId)
    , m_c2MethId_AsFloat(kCIDMacroEng::c2BadId)
    , m_c2MethId_AsInt(kCIDMacroEng::c2BadId)
    , m_c2MethId_AsString(kCIDMacroEng::c2BadId)
    , m_c2MethId_DefCtor(kCIDMacroEng::c2BadId)
    , m_c2MethId_Format(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetFldChInfo(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetLoadChInfo(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetLockStInfo(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetMotionInfo(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetPresenceInfo(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetUserActInfo(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetZAlarmInfo(kCIDMacroEng::c2BadId)
    , m_c2MethId_IsFldChangeEv(kCIDMacroEng::c2BadId)
    , m_c2MethId_IsStdDrvEv(kCIDMacroEng::c2BadId)
    , m_c2MethId_IsUserActionEv(kCIDMacroEng::c2BadId)
    , m_c2MethId_Parse(kCIDMacroEng::c2BadId)
    , m_c2MethId_Reset(kCIDMacroEng::c2BadId)
    , m_c2MethId_SetValue(kCIDMacroEng::c2BadId)
    , m_c2MethId_ValCtor(kCIDMacroEng::c2BadId)
    , m_c2MethId_ValueExists(kCIDMacroEng::c2BadId)
    , m_c4ErrAccess(0)
    , m_c4ErrCvt(0)
    , m_c4ErrFormat(0)
    , m_c4ErrParse(0)
    , m_pmeciErrors(nullptr)
    , m_pmeciStdDrvEvs(nullptr)
{
}

TEventInfo::~TEventInfo()
{
}


tCIDLib::TVoid TEventInfo::Init(TCIDMacroEngine& meOwner)
{
    // Create an enumerated type for our errors
    {
        m_pmeciErrors = new TMEngEnumInfo
        (
            meOwner
            , L"EvObjErrors"
            , strClassPath()
            , L"MEng.Enum"
            , 3
        );
        m_c4ErrAccess = m_pmeciErrors->c4AddEnumItem(L"AccessErr", TString::strEmpty());
        m_c4ErrCvt = m_pmeciErrors->c4AddEnumItem(L"ConvertErr", TString::strEmpty());
        m_c4ErrFormat = m_pmeciErrors->c4AddEnumItem(L"FormattErr", TString::strEmpty());
        m_c4ErrParse = m_pmeciErrors->c4AddEnumItem(L"ParseErr", TString::strEmpty());
        m_pmeciErrors->BaseClassInit(meOwner);
        m_c2EnumId_Errors = meOwner.c2AddClass(m_pmeciErrors);
        bAddNestedType(m_pmeciErrors->strClassPath());
    }

    // Create an enum for the standard driver events
    {
        m_pmeciStdDrvEvs = new TMEngEnumInfo
        (
            meOwner
            , L"StdDrvEvents"
            , strClassPath()
            , L"MEng.Enum"
            , 7
        );
        m_pmeciStdDrvEvs->c4AddEnumItem(L"FldChange", L"Field Change", tCQCKit::EStdDrvEvs::FldChange);
        m_pmeciStdDrvEvs->c4AddEnumItem(L"LoadChange", L"Load Change", tCQCKit::EStdDrvEvs::LoadChange);
        m_pmeciStdDrvEvs->c4AddEnumItem(L"LockStatus", L"Load Status", tCQCKit::EStdDrvEvs::LockStatus);
        m_pmeciStdDrvEvs->c4AddEnumItem(L"Motion", L"Motion Event", tCQCKit::EStdDrvEvs::Motion);
        m_pmeciStdDrvEvs->c4AddEnumItem(L"Presence", L"Presence Event", tCQCKit::EStdDrvEvs::Presence);
        m_pmeciStdDrvEvs->c4AddEnumItem(L"UserAction", L"User Action", tCQCKit::EStdDrvEvs::UserAction);
        m_pmeciStdDrvEvs->c4AddEnumItem(L"ZoneAlarm", L"Zone Alarm", tCQCKit::EStdDrvEvs::ZoneAlarm);
        m_pmeciStdDrvEvs->BaseClassInit(meOwner);
        m_c2EnumId_StdDrvEvs = meOwner.c2AddClass(m_pmeciStdDrvEvs);
        bAddNestedType(m_pmeciStdDrvEvs->strClassPath());
    }


    // Add a new block to an event structure
    {
        TMEngMethodInfo methiNew
        (
            L"AddBlock"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"Parent", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"Name", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_AddBlock = c2AddMethodInfo(methiNew);
    }

    // Add a new value to an existing block
    {
        TMEngMethodInfo methiNew
        (
            L"AddValue"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"Parent", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"Name", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"Value", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_AddValue = c2AddMethodInfo(methiNew);
    }

    // Get a value as various data types
    {
        TMEngMethodInfo methiNew
        (
            L"AsBool"
            , tCIDMacroEng::EIntrinsics::Boolean
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInParm(L"Path", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_AsBool = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"AsCard4"
            , tCIDMacroEng::EIntrinsics::Card4
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInParm(L"Path", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_AsCard4 = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"AsCard8"
            , tCIDMacroEng::EIntrinsics::Card8
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInParm(L"Path", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_AsCard8 = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"AsFloat"
            , tCIDMacroEng::EIntrinsics::Float8
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInParm(L"Path", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_AsFloat = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"AsInt"
            , tCIDMacroEng::EIntrinsics::Int4
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInParm(L"Path", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_AsInt = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"AsString"
            , tCIDMacroEng::EIntrinsics::String
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInParm(L"Path", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_AsString = c2AddMethodInfo(methiNew);
    }

    // Add the default constructor
    {
        TMEngMethodInfo methiNew
        (
            L"ctor1_MEng.System.CQC.Runtime.Event"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.bIsCtor(kCIDLib::True);
        m_c2MethId_DefCtor = c2AddMethodInfo(methiNew);
    }

    // Format our contents back out to a string
    {
        TMEngMethodInfo methiNew
        (
            L"Format"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddOutParm(L"ToFill", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_Format = c2AddMethodInfo(methiNew);
    }

    // If it's a field change, return the type specific info
    {
        TMEngMethodInfo methiNew
        (
            L"GetFldChangeInfo"
            , tCIDMacroEng::EIntrinsics::Boolean
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddOutParm(L"FldName", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddOutParm(L"FldVal", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_GetFldChInfo = c2AddMethodInfo(methiNew);
    }

    // If it's a load change, return the type specific info
    {
        TMEngMethodInfo methiNew
        (
            L"GetLoadChangeInfo"
            , tCIDMacroEng::EIntrinsics::Boolean
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddOutParm(L"State", tCIDMacroEng::EIntrinsics::Boolean);
        methiNew.c2AddOutParm(L"LoadNum", tCIDMacroEng::EIntrinsics::Card4);
        m_c2MethId_GetLoadChInfo = c2AddMethodInfo(methiNew);
    }

    // If it's a lock state change, return the type specific info
    {
        TMEngMethodInfo methiNew
        (
            L"GetLockStateInfo"
            , tCIDMacroEng::EIntrinsics::Boolean
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddOutParm(L"State", tCIDMacroEng::EIntrinsics::Boolean);
        methiNew.c2AddOutParm(L"LockId", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddOutParm(L"Code", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddOutParm(L"Type", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_GetLockStInfo = c2AddMethodInfo(methiNew);
    }

    // If it's a motion change, return the type specific info
    {
        TMEngMethodInfo methiNew
        (
            L"GetMotionInfo"
            , tCIDMacroEng::EIntrinsics::Boolean
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddOutParm(L"Type", tCIDMacroEng::EIntrinsics::Boolean);
        methiNew.c2AddOutParm(L"SensorNum", tCIDMacroEng::EIntrinsics::Card4);
        m_c2MethId_GetMotionInfo = c2AddMethodInfo(methiNew);
    }

    // If it's a presence change, return the type specific info
    {
        TMEngMethodInfo methiNew
        (
            L"GetPresenceInfo"
            , tCIDMacroEng::EIntrinsics::Boolean
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddOutParm(L"Type", tCIDMacroEng::EIntrinsics::Boolean);
        methiNew.c2AddOutParm(L"Id", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddOutParm(L"AreaId", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_GetPresenceInfo = c2AddMethodInfo(methiNew);
    }

    // If it's a motion change, return the type specific info
    {
        TMEngMethodInfo methiNew
        (
            L"GetUserActInfo"
            , tCIDMacroEng::EIntrinsics::Boolean
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddOutParm(L"Type", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddOutParm(L"Data", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_GetUserActInfo = c2AddMethodInfo(methiNew);
    }

    // If it's a zone alarm, return the type specific info
    {
        TMEngMethodInfo methiNew
        (
            L"GetZoneAlarmInfo"
            , tCIDMacroEng::EIntrinsics::Boolean
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddOutParm(L"State", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddOutParm(L"ZoneNum", tCIDMacroEng::EIntrinsics::Card4);
        m_c2MethId_GetZAlarmInfo = c2AddMethodInfo(methiNew);
    }


    // Check to see if it's one of the standard driver events
    {
        TMEngMethodInfo methiNew
        (
            L"IsStdDrvEvent"
            , tCIDMacroEng::EIntrinsics::Boolean
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInParm(L"ToCheck", m_c2EnumId_StdDrvEvs);
        m_c2MethId_IsStdDrvEv = c2AddMethodInfo(methiNew);
    }

    // Parse our contents from a formatted event string
    {
        TMEngMethodInfo methiNew
        (
            L"Parse"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"Text", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_Parse = c2AddMethodInfo(methiNew);
    }

    // Reset us back to empty
    {
        TMEngMethodInfo methiNew
        (
            L"Reset"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        m_c2MethId_Reset = c2AddMethodInfo(methiNew);
    }

    // The initial value ctor that takes a formatted event and parses it
    {
        TMEngMethodInfo methiNew
        (
            L"ctor2_MEng.System.CQC.Runtime.Event"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.bIsCtor(kCIDLib::True);
        methiNew.c2AddInParm(L"Text", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_ValCtor = c2AddMethodInfo(methiNew);
    }

    // See if a particular value exists
    {
        TMEngMethodInfo methiNew
        (
            L"ValueExists"
            , tCIDMacroEng::EIntrinsics::Boolean
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInParm(L"Path", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_ValueExists = c2AddMethodInfo(methiNew);
    }




    //
    //  These test for common types of events. These are now deprecated
    //  and there is a new, more generic, method that can test for all
    //  of the standard driver event types.
    //
    {
        TMEngMethodInfo methiNew
        (
            L"IsFldChangeEv"
            , tCIDMacroEng::EIntrinsics::Boolean
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        m_c2MethId_IsFldChangeEv = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"IsUserActionEv"
            , tCIDMacroEng::EIntrinsics::Boolean
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        m_c2MethId_IsUserActionEv = c2AddMethodInfo(methiNew);
    }
}


TMEngClassVal*
TEventInfo::pmecvMakeStorage(const  TString&                strName
                            ,       TCIDMacroEngine&
                            , const tCIDMacroEng::EConstTypes  eConst) const
{
    return new TEventValue(strName, c2Id(), eConst);
}


// ---------------------------------------------------------------------------
//  TEventInfo: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TEventInfo::bInvokeMethod(          TCIDMacroEngine&    meOwner
                            , const TMEngMethodInfo&    methiTarget
                            ,       TMEngClassVal&      mecvInstance)
{
    TEventValue& mecvActual = static_cast<TEventValue&>(mecvInstance);
    TCQCEvent& cevThis = mecvActual.cevValue();

    const tCIDLib::TCard4 c4FirstInd = meOwner.c4FirstParmInd(methiTarget);
    const tCIDLib::TCard2 c2MethId   = methiTarget.c2Id();

    if (c2MethId == m_c2MethId_AddBlock)
    {
        try
        {
            cevThis.AddBlock
            (
                meOwner.strStackValAt(c4FirstInd)
                , meOwner.strStackValAt(c4FirstInd + 1)
            );
        }

        catch(const TError& errToCatch)
        {
            ThrowAnErr(meOwner, m_c4ErrAccess, errToCatch.strErrText());
        }
    }
     else if (c2MethId == m_c2MethId_AddValue)
    {
        try
        {
            cevThis.AddValue
            (
                meOwner.strStackValAt(c4FirstInd)
                , meOwner.strStackValAt(c4FirstInd + 1)
                , meOwner.strStackValAt(c4FirstInd + 2)
            );
        }

        catch(const TError& errToCatch)
        {
            ThrowAnErr(meOwner, m_c4ErrAccess, errToCatch.strErrText());
        }
    }
     else if (c2MethId == m_c2MethId_AsBool)
    {
        TMEngBooleanVal& mecvRet = meOwner.mecvStackAtAs<TMEngBooleanVal>(c4FirstInd - 1);
        try
        {
            mecvRet.bValue(cevThis.bValueAs(meOwner.strStackValAt(c4FirstInd)));
        }

        catch(const TError& errToCatch)
        {
            ThrowAnErr(meOwner, m_c4ErrCvt, errToCatch.strErrText());
        }
    }
     else if (c2MethId == m_c2MethId_AsCard4)
    {
        TMEngCard4Val& mecvRet = meOwner.mecvStackAtAs<TMEngCard4Val>(c4FirstInd - 1);
        try
        {
            mecvRet.c4Value(cevThis.c4ValueAs(meOwner.strStackValAt(c4FirstInd)));
        }

        catch(const TError& errToCatch)
        {
            ThrowAnErr(meOwner, m_c4ErrCvt, errToCatch.strErrText());
        }
    }
     else if (c2MethId == m_c2MethId_AsCard8)
    {
        TMEngCard8Val& mecvRet = meOwner.mecvStackAtAs<TMEngCard8Val>(c4FirstInd - 1);
        try
        {
            mecvRet.c8Value(cevThis.c8ValueAs(meOwner.strStackValAt(c4FirstInd)));
        }

        catch(const TError& errToCatch)
        {
            ThrowAnErr(meOwner, m_c4ErrCvt, errToCatch.strErrText());
        }
    }
     else if (c2MethId == m_c2MethId_AsFloat)
    {
        TMEngFloat8Val& mecvRet = meOwner.mecvStackAtAs<TMEngFloat8Val>(c4FirstInd - 1);
        try
        {
            mecvRet.f8Value(cevThis.f8ValueAs(meOwner.strStackValAt(c4FirstInd)));
        }

        catch(const TError& errToCatch)
        {
            ThrowAnErr(meOwner, m_c4ErrCvt, errToCatch.strErrText());
        }
    }
     else if (c2MethId == m_c2MethId_AsInt)
    {
        TMEngInt4Val& mecvRet = meOwner.mecvStackAtAs<TMEngInt4Val>(c4FirstInd - 1);
        try
        {
            mecvRet.i4Value(cevThis.i4ValueAs(meOwner.strStackValAt(c4FirstInd)));
        }

        catch(const TError& errToCatch)
        {
            ThrowAnErr(meOwner, m_c4ErrCvt, errToCatch.strErrText());
        }
    }
     else if (c2MethId == m_c2MethId_AsString)
    {
        TMEngStringVal& mecvRet = meOwner.mecvStackAtAs<TMEngStringVal>(c4FirstInd - 1);
        try
        {
            mecvRet.strValue(cevThis.strValue(meOwner.strStackValAt(c4FirstInd)));
        }

        catch(const TError& errToCatch)
        {
            ThrowAnErr(meOwner, m_c4ErrCvt, errToCatch.strErrText());
        }
    }
     else if (c2MethId == m_c2MethId_DefCtor)
    {
        // Just reset the object
        cevThis.Reset();
    }
     else if (c2MethId == m_c2MethId_Format)
    {
        TMEngStringVal& mecvFmt = meOwner.mecvStackAtAs<TMEngStringVal>(c4FirstInd);
        try
        {
            TTextStringOutStream strmOut(&mecvFmt.strValue());
            strmOut << cevThis << kCIDLib::FlushIt;
        }

        catch(const TError& errToCatch)
        {
            ThrowAnErr(meOwner, m_c4ErrFormat, errToCatch.strErrText());
        }
    }
     else if (c2MethId == m_c2MethId_GetFldChInfo)
    {
        TMEngBooleanVal& mecvRet = meOwner.mecvStackAtAs<TMEngBooleanVal>(c4FirstInd - 1);
        if (cevThis.bIsDrvEv(tCQCKit::EStdDrvEvs::FldChange))
        {
            mecvRet.bValue(kCIDLib::True);

            TString strFldName;
            cevThis.bQuerySrc(strFldName);
            meOwner.mecvStackAtAs<TMEngStringVal>(c4FirstInd).strValue(strFldName);
            meOwner.mecvStackAtAs<TMEngStringVal>(c4FirstInd + 1).strValue
            (
                cevThis.strValue(TCQCEvent::strPath_FldValue)
            );
        }
         else
        {
            mecvRet.bValue(kCIDLib::False);
        }
    }
     else if (c2MethId == m_c2MethId_GetLoadChInfo)
    {
        try
        {
            TMEngBooleanVal& mecvRet = meOwner.mecvStackAtAs<TMEngBooleanVal>(c4FirstInd - 1);
            if (cevThis.bIsDrvEv(tCQCKit::EStdDrvEvs::LoadChange))
            {
                mecvRet.bValue(kCIDLib::True);

                meOwner.mecvStackAtAs<TMEngBooleanVal>(c4FirstInd).bValue
                (
                    cevThis.bValueEquals
                    (
                        TCQCEvent::strPath_LoadState
                        , facCQCKit().strBoolOffOn(kCIDLib::True)
                    )
                );
                meOwner.mecvStackAtAs<TMEngCard4Val>(c4FirstInd + 1).c4Value
                (
                    cevThis.c4ValueAs(TCQCEvent::strPath_LoadNum)
                );
            }
             else
            {
                mecvRet.bValue(kCIDLib::False);
            }
        }

        catch(const TError& errToCatch)
        {
            ThrowAnErr(meOwner, m_c4ErrFormat, errToCatch.strErrText());
        }
    }
     else if (c2MethId == m_c2MethId_GetLockStInfo)
    {
        try
        {
            TMEngBooleanVal& mecvRet = meOwner.mecvStackAtAs<TMEngBooleanVal>(c4FirstInd - 1);
            if (cevThis.bIsDrvEv(tCQCKit::EStdDrvEvs::LockStatus))
            {
                mecvRet.bValue(kCIDLib::True);

                meOwner.mecvStackAtAs<TMEngBooleanVal>(c4FirstInd).bValue
                (
                    cevThis.bValueEquals
                    (
                        TCQCEvent::strPath_LockState
                        , facCQCKit().strBoolLockedUnlocked(kCIDLib::True)
                    )
                );
                meOwner.mecvStackAtAs<TMEngStringVal>(c4FirstInd + 1).strValue
                (
                    cevThis.strValue(TCQCEvent::strPath_LockId)
                );
                meOwner.mecvStackAtAs<TMEngStringVal>(c4FirstInd + 2).strValue
                (
                    cevThis.strValue(TCQCEvent::strPath_LockCode)
                );
                meOwner.mecvStackAtAs<TMEngStringVal>(c4FirstInd + 3).strValue
                (
                    cevThis.strValue(TCQCEvent::strPath_LockType)
                );
            }
             else
            {
                mecvRet.bValue(kCIDLib::False);
            }
        }

        catch(const TError& errToCatch)
        {
            ThrowAnErr(meOwner, m_c4ErrFormat, errToCatch.strErrText());
        }
    }
     else if (c2MethId == m_c2MethId_GetMotionInfo)
    {
        try
        {
            TMEngBooleanVal& mecvRet = meOwner.mecvStackAtAs<TMEngBooleanVal>(c4FirstInd - 1);
            if (cevThis.bIsDrvEv(tCQCKit::EStdDrvEvs::Motion))
            {
                mecvRet.bValue(kCIDLib::True);

                meOwner.mecvStackAtAs<TMEngBooleanVal>(c4FirstInd).bValue
                (
                    cevThis.bValueEquals
                    (
                        TCQCEvent::strPath_MotionType
                        , facCQCKit().strBoolStartEnd(kCIDLib::True)
                    )
                );
                meOwner.mecvStackAtAs<TMEngCard4Val>(c4FirstInd + 1).c4Value
                (
                    cevThis.c4ValueAs(TCQCEvent::strPath_MotionNum)
                );
            }
             else
            {
                mecvRet.bValue(kCIDLib::False);
            }
        }

        catch(const TError& errToCatch)
        {
            ThrowAnErr(meOwner, m_c4ErrFormat, errToCatch.strErrText());
        }
    }
     else if (c2MethId == m_c2MethId_GetPresenceInfo)
    {
        try
        {
            TMEngBooleanVal& mecvRet = meOwner.mecvStackAtAs<TMEngBooleanVal>(c4FirstInd - 1);
            if (cevThis.bIsDrvEv(tCQCKit::EStdDrvEvs::Presence))
            {
                mecvRet.bValue(kCIDLib::True);

                meOwner.mecvStackAtAs<TMEngBooleanVal>(c4FirstInd).bValue
                (
                    cevThis.bValueEquals
                    (
                        TCQCEvent::strPath_PresType
                        , facCQCKit().strBoolEnterExit(kCIDLib::True)
                    )
                );
                meOwner.mecvStackAtAs<TMEngStringVal>(c4FirstInd + 1).strValue
                (
                    cevThis.strValue(TCQCEvent::strPath_PresIdInfo)
                );
                meOwner.mecvStackAtAs<TMEngStringVal>(c4FirstInd + 2).strValue
                (
                    cevThis.strValue(TCQCEvent::strPath_PresArea)
                );
            }
             else
            {
                mecvRet.bValue(kCIDLib::False);
            }
        }

        catch(const TError& errToCatch)
        {
            ThrowAnErr(meOwner, m_c4ErrFormat, errToCatch.strErrText());
        }
    }
     else if (c2MethId == m_c2MethId_GetUserActInfo)
    {
        try
        {
            TMEngBooleanVal& mecvRet = meOwner.mecvStackAtAs<TMEngBooleanVal>(c4FirstInd - 1);
            if (cevThis.bIsDrvEv(tCQCKit::EStdDrvEvs::UserAction))
            {
                mecvRet.bValue(kCIDLib::True);

                meOwner.mecvStackAtAs<TMEngStringVal>(c4FirstInd).strValue
                (
                    cevThis.strValue(TCQCEvent::strPath_ActType)
                );
                meOwner.mecvStackAtAs<TMEngStringVal>(c4FirstInd + 1).strValue
                (
                    cevThis.strValue(TCQCEvent::strPath_ActData)
                );
            }
             else
            {
                mecvRet.bValue(kCIDLib::False);
            }
        }

        catch(const TError& errToCatch)
        {
            ThrowAnErr(meOwner, m_c4ErrFormat, errToCatch.strErrText());
        }
    }
     else if (c2MethId == m_c2MethId_GetZAlarmInfo)
    {
        try
        {
            TMEngBooleanVal& mecvRet = meOwner.mecvStackAtAs<TMEngBooleanVal>(c4FirstInd - 1);
            if (cevThis.bIsDrvEv(tCQCKit::EStdDrvEvs::ZoneAlarm))
            {
                mecvRet.bValue(kCIDLib::True);

                meOwner.mecvStackAtAs<TMEngStringVal>(c4FirstInd).strValue
                (
                    cevThis.strValue(TCQCEvent::strPath_ZoneStatus)
                );

                meOwner.mecvStackAtAs<TMEngCard4Val>(c4FirstInd + 1).c4Value
                (
                    cevThis.c4ValueAs(TCQCEvent::strPath_ZoneNum)
                );
            }
             else
            {
                mecvRet.bValue(kCIDLib::False);
            }
        }

        catch(const TError& errToCatch)
        {
            ThrowAnErr(meOwner, m_c4ErrFormat, errToCatch.strErrText());
        }
    }
     else if (c2MethId == m_c2MethId_IsFldChangeEv)
    {
        // !!! Deprecated, use IsStdDrvEvent
        TMEngBooleanVal& mecvRet = meOwner.mecvStackAtAs<TMEngBooleanVal>(c4FirstInd - 1);
        try
        {
            mecvRet.bValue(cevThis.bIsDrvEv(tCQCKit::EStdDrvEvs::FldChange));
        }

        catch(const TError& errToCatch)
        {
            ThrowAnErr(meOwner, m_c4ErrAccess, errToCatch.strErrText());
        }
    }
     else if (c2MethId == m_c2MethId_IsUserActionEv)
    {
        // !!! Deprecated, use IsStdDrvEvent
        TMEngBooleanVal& mecvRet = meOwner.mecvStackAtAs<TMEngBooleanVal>(c4FirstInd - 1);
        try
        {
            mecvRet.bValue(cevThis.bIsDrvEv(tCQCKit::EStdDrvEvs::UserAction));
        }

        catch(const TError& errToCatch)
        {
            ThrowAnErr(meOwner, m_c4ErrAccess, errToCatch.strErrText());
        }
    }
     else if (c2MethId == m_c2MethId_IsStdDrvEv)
    {
        TMEngBooleanVal& mecvRet = meOwner.mecvStackAtAs<TMEngBooleanVal>(c4FirstInd - 1);
        TMEngEnumVal& mecvType = meOwner.mecvStackAtAs<TMEngEnumVal>(c4FirstInd);
        try
        {
            // Get the C++ enum out of the CML enum
            const tCQCKit::EStdDrvEvs eCheck = tCQCKit::EStdDrvEvs
            (
                m_pmeciStdDrvEvs->c4MapValue(mecvType)
            );
            mecvRet.bValue(cevThis.bIsDrvEv(eCheck));
        }

        catch(const TError& errToCatch)
        {
            ThrowAnErr(meOwner, m_c4ErrAccess, errToCatch.strErrText());
        }
    }
     else if ((c2MethId == m_c2MethId_Parse) || (c2MethId == m_c2MethId_ValCtor))
    {
        try
        {
            TTextStringInStream strmSrc(&meOwner.strStackValAt(c4FirstInd));
            cevThis.ParseFrom(strmSrc);
        }

        catch(const TError& errToCatch)
        {
            ThrowAnErr(meOwner, m_c4ErrParse, errToCatch.strErrText());
        }
    }
     else if (c2MethId == m_c2MethId_Reset)
    {
        cevThis.Reset();
    }
     else if (c2MethId == m_c2MethId_SetValue)
    {
        try
        {
            cevThis.SetValue
            (
                meOwner.strStackValAt(c4FirstInd)
                , meOwner.strStackValAt(c4FirstInd + 1)
            );
        }

        catch(const TError& errToCatch)
        {
            ThrowAnErr(meOwner, m_c4ErrAccess, errToCatch.strErrText());
        }
    }
     else if (c2MethId == m_c2MethId_ValueExists)
    {
        TMEngBooleanVal& mecvRet = meOwner.mecvStackAtAs<TMEngBooleanVal>(c4FirstInd - 1);
        try
        {
            mecvRet.bValue(cevThis.bValueExists(meOwner.strStackValAt(c4FirstInd)));
        }

        catch(const TError& errToCatch)
        {
            ThrowAnErr(meOwner, m_c4ErrAccess, errToCatch.strErrText());
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
//  TEventInfo: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TEventInfo::ThrowAnErr(         TCIDMacroEngine&    meOwner
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





// ---------------------------------------------------------------------------
//  CLASS: TEventHandlerInfo
// PREFIX: meci
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TEventHandlerInfo: Public, static methods
// ---------------------------------------------------------------------------
const TString& TEventHandlerInfo::strHandlerData()
{
    return CQCMEng_Event::strHandlerData;
}

const TString& TEventHandlerInfo::strClassPath()
{
    return CQCMEng_Event::strEventHandlerClassPath;
}


// ---------------------------------------------------------------------------
//  TEventHandlerInfo: Constructors and Destructor
// ---------------------------------------------------------------------------
TEventHandlerInfo::TEventHandlerInfo(TCIDMacroEngine& meOwner) :

    TMEngClassInfo
    (
        CQCMEng_Event::strEventHandler
        , CQCMEng_Event::strBasePath
        , meOwner
        , kCIDLib::False
        , tCIDMacroEng::EClassExt::Abstract
        , L"MEng.Object"
    )
    , m_c2MethId_DefCtor(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetEvent(kCIDMacroEng::c2BadId)
{
    // Import the event class as one of our imported classes
    bAddClassImport(CQCMEng_Event::strEventClassPath);
}

TEventHandlerInfo::~TEventHandlerInfo()
{
}


tCIDLib::TVoid TEventHandlerInfo::Init(TCIDMacroEngine& meOwner)
{
    // Look up the event class type which we need to use
    TEventInfo* pmeciEvent = meOwner.pmeciFindAs<TEventInfo>
    (
        CQCMEng_Event::strEventClassPath
        , kCIDLib::True
    );

    //
    //  Register the CML level data members we want our instances to have,
    //  so that we can use a helper method in our base class to have them
    //  loaded into the value objects when we make storage below.
    //
    {
        TMEngMemberInfo memiValues
        (
            CQCMEng_Event::strHandlerData
            , pmeciEvent->c2Id()
            , tCIDMacroEng::EConstTypes::Const
        );
        c2AddMember(memiValues, *pmeciEvent);
    }

    // Add the default constructor
    {
        TMEngMethodInfo methiNew
        (
            L"ctor1_MEng.System.CQC.Runtime.EventHandler"
            , pmeciEvent->c2Id()
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.bIsCtor(kCIDLib::True);
        m_c2MethId_DefCtor = c2AddMethodInfo(methiNew);
    }

    // Give back the event info
    {
        TMEngMethodInfo methiNew
        (
            L"GetEvent"
            , tCIDMacroEng::EIntrinsics::Card4
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInParm(L"ToFill", pmeciEvent->c2Id());
        m_c2MethId_GetEvent = c2AddMethodInfo(methiNew);
    }
}


TMEngClassVal*
TEventHandlerInfo::pmecvMakeStorage(const   TString&                strName
                                    ,       TCIDMacroEngine&        meOwner
                                    , const tCIDMacroEng::EConstTypes  eConst) const
{
    //
    //  Create a new value object (just a standard class value), then load it
    //  with any CML level members we defined for our class.
    //
    TMEngStdClassVal* pmecvNew = new TMEngStdClassVal(strName, c2Id(), eConst);
    TJanitor<TMEngStdClassVal> janNew(pmecvNew);
    LoadMemberVals(*pmecvNew, meOwner);
    return janNew.pobjOrphan();
}


// ---------------------------------------------------------------------------
//  TEventHandlerInfo: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TEventHandlerInfo::bInvokeMethod(       TCIDMacroEngine&    meOwner
                                , const TMEngMethodInfo&    methiTarget
                                ,       TMEngClassVal&      mecvInstance)
{
    TMEngStdClassVal& mecvActual = static_cast<TMEngStdClassVal&>(mecvInstance);

    const tCIDLib::TCard4 c4FirstInd = meOwner.c4FirstParmInd(methiTarget);
    const tCIDLib::TCard2 c2MethId   = methiTarget.c2Id();

    if (c2MethId == m_c2MethId_DefCtor)
    {
        // Nothing to do
    }
     else if (c2MethId == m_c2MethId_GetEvent)
    {
        // Get the CML level event member
        TMEngClassVal* pmecvEvMem = mecvActual.pmecvFind(CQCMEng_Event::strHandlerData);
        #if CID_DEBUG_ON
        if (!pmecvEvMem)
        {
            facCIDMacroEng().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kMEngErrs::errcClass_MemberNotFound
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::NotFound
                , CQCMEng_Event::strHandlerData
                , CQCMEng_Event::strEventHandlerClassPath
            );
        }
        #endif
        TEventValue* pmecvEvent = static_cast<TEventValue*>(pmecvEvMem);

        // And now we can copy it to the caller's output parameter
        TEventValue& mecvOut = meOwner.mecvStackAtAs<TEventValue>(c4FirstInd);
        mecvOut.cevValue(pmecvEvent->cevValue());
    }
     else
    {
        // We don't know what it is
        return kCIDLib::False;
    }
    return kCIDLib::True;
}

