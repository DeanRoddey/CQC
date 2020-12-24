//
// FILE NAME: CQCMEng_FieldDefClass.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/16/2003
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
//  class which wraps the TCQCFldDef class.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCMEng_.hpp"
#include    "CQCMEng_FieldDefClass_.hpp"


// ---------------------------------------------------------------------------
//  Magic RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCFldDefVal,TMEngClassVal)
RTTIDecls(TCQCFldDefInfo,TMEngClassInfo)


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace CQCMacroEng_FldDefClass
{
    namespace
    {
        // -----------------------------------------------------------------------
        //  The names for the types that we support here. Each derivative has to
        //  be able to return strings that contain its name and full name.
        // -----------------------------------------------------------------------
        const TString   strFldDef(L"CQCFldDef");
        const TString   strFldDefClassPath(L"MEng.System.CQC.Runtime.CQCFldDef");
        const TString   strFldDefBasePath(L"MEng.System.CQC.Runtime");
    }
}


// ---------------------------------------------------------------------------
//  CLASS: TCQCFldDefVal
// PREFIX: mecv
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCFldDefVal: Constuctors and Destructor
// ---------------------------------------------------------------------------
TCQCFldDefVal::TCQCFldDefVal(const  TString&                    strName
                            , const tCIDLib::TCard2             c2ClassId
                            , const tCIDMacroEng::EConstTypes   eConst) :

    TMEngClassVal(strName, c2ClassId, eConst)
{
}

TCQCFldDefVal::TCQCFldDefVal(const  TString&                    strName
                            , const tCIDLib::TCard2             c2ClassId
                            , const tCIDMacroEng::EConstTypes   eConst
                            , const TCQCFldDef&                 flddInit) :

    TMEngClassVal(strName, c2ClassId, eConst)
    , m_flddValue(flddInit)
{
}

TCQCFldDefVal::~TCQCFldDefVal()
{
}


// ---------------------------------------------------------------------------
//  TCQCFldDefVal: Public, inherited methods
// ---------------------------------------------------------------------------a
tCIDLib::TBoolean
TCQCFldDefVal::bDbgFormat(          TTextOutStream&     strmTarget
                            , const TMEngClassInfo&
                            , const tCIDMacroEng::EDbgFmts eFormat
                            , const tCIDLib::ERadices
                            , const TCIDMacroEngine&    ) const
{
    if (eFormat == tCIDMacroEng::EDbgFmts::Long)
    {
        strmTarget  << L"Name: " << m_flddValue.strName() << kCIDLib::NewLn
                    << L"Id: " << m_flddValue.c4Id() << kCIDLib::NewLn
                    << L"Data Type: " << m_flddValue.eType() << kCIDLib::NewLn
                    << L"Sem Type: " << m_flddValue.eSemType() << kCIDLib::NewLn
                    << L"Access: " << m_flddValue.eAccess() << kCIDLib::NewLn
                    << L"Limits: " << m_flddValue.strLimits() << kCIDLib::NewLn
                    << L"WriteAlways: "
                    << facCQCKit().strBoolYesNo(m_flddValue.bAlwaysWrite())
                    << kCIDLib::NewLn;

    }
     else
    {
        strmTarget << m_flddValue.strName();
    }
    return kCIDLib::True;
}


tCIDLib::TVoid
TCQCFldDefVal::CopyFrom(const   TMEngClassVal&      mecvToCopy
                        ,       TCIDMacroEngine&    meOwner)
{
    if (meOwner.bValidation())
        meOwner.CheckSameClasses(*this, mecvToCopy);
    m_flddValue = static_cast<const TCQCFldDefVal&>(mecvToCopy).m_flddValue;
}



// ---------------------------------------------------------------------------
//  TCQCFldDefVal: Public, non-virtual methods
// ---------------------------------------------------------------------------
const TCQCFldDef& TCQCFldDefVal::flddValue() const
{
    return m_flddValue;
}

TCQCFldDef& TCQCFldDefVal::flddValue()
{
    return m_flddValue;
}

TCQCFldDef& TCQCFldDefVal::flddValue(const TCQCFldDef& flddToSet)
{
    m_flddValue = flddToSet;
    return m_flddValue;
}




// ---------------------------------------------------------------------------
//  CLASS: TCQCFldDefInfo
// PREFIX: meci
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCFldDefInfo: Public, static methods
// ---------------------------------------------------------------------------
const TString& TCQCFldDefInfo::strClassPath()
{
    return CQCMacroEng_FldDefClass::strFldDefClassPath;
}


// ---------------------------------------------------------------------------
//  TCQCFldDefInfo: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCFldDefInfo::TCQCFldDefInfo(TCIDMacroEngine& meOwner) :

    TMEngClassInfo
    (
        CQCMacroEng_FldDefClass::strFldDef
        , CQCMacroEng_FldDefClass::strFldDefBasePath
        , meOwner
        , kCIDLib::True
        , tCIDMacroEng::EClassExt::Final
        , L"MEng.Formattable"
    )
    , m_c2EnumId_FldAccess(kCIDMacroEng::c2BadId)
    , m_c2EnumId_FldSTypes(kCIDMacroEng::c2BadId)
    , m_c2EnumId_FldTypes(kCIDMacroEng::c2BadId)
    , m_c2MethId_CheckFldName(kCIDMacroEng::c2BadId)
    , m_c2MethId_DefCtor(kCIDMacroEng::c2BadId)
    , m_c2MethId_Equal(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetAccess(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetId(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetName(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetLimits(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetSemType(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetType(kCIDMacroEng::c2BadId)
    , m_c2MethId_Set1(kCIDMacroEng::c2BadId)
    , m_c2MethId_Set2(kCIDMacroEng::c2BadId)
    , m_c2MethId_SetAlwaysWrite(kCIDMacroEng::c2BadId)
    , m_c2MethId_SetQueuedWrite(kCIDMacroEng::c2BadId)
    , m_c2MethId_SetNoStoreOnWrite(kCIDMacroEng::c2BadId)
    , m_c2MethId_SetWLims(kCIDMacroEng::c2BadId)
    , m_c2MethId_SetWLims2(kCIDMacroEng::c2BadId)
    , m_c2MethId_ValCtor1(kCIDMacroEng::c2BadId)
    , m_c2MethId_ValCtor2(kCIDMacroEng::c2BadId)
    , m_pmeciFldAcc(0)
    , m_pmeciFldSTypes(0)
    , m_pmeciFldTypes(0)
{
}

TCQCFldDefInfo::~TCQCFldDefInfo()
{
}


tCIDLib::TVoid TCQCFldDefInfo::Init(TCIDMacroEngine& meOwner)
{
    // Add nested enumerated types for field access and types.
    {
        m_pmeciFldAcc = new TMEngEnumInfo
        (
            meOwner, L"CQCFldAccess", strClassPath(), L"MEng.Enum", 4
        );
        m_pmeciFldAcc->c4AddEnumItem(L"None", L"None", tCQCKit::EFldAccess::None);
        m_pmeciFldAcc->c4AddEnumItem(L"Read", L"Readable", tCQCKit::EFldAccess::Read);
        m_pmeciFldAcc->c4AddEnumItem(L"Write", L"Writable", tCQCKit::EFldAccess::Write);
        m_pmeciFldAcc->c4AddEnumItem(L"ReadWrite", L"Readable/Writeable", tCQCKit::EFldAccess::ReadWrite);
        m_pmeciFldAcc->BaseClassInit(meOwner);
        m_c2EnumId_FldAccess = meOwner.c2AddClass(m_pmeciFldAcc);
        bAddNestedType(m_pmeciFldAcc->strClassPath());
    }

    {
        m_pmeciFldSTypes = new TMEngEnumInfo
        (
            meOwner, L"CQCFldSTypes", strClassPath(), L"MEng.Enum", 40
        );
        m_pmeciFldSTypes->c4AddEnumItem(L"AnalogIO", L"Analog I/O", tCQCKit::EFldSTypes::AnalogIO);
        m_pmeciFldSTypes->c4AddEnumItem(L"BoolSwitch", L"Boolean Switch", tCQCKit::EFldSTypes::BoolSwitch);
        m_pmeciFldSTypes->c4AddEnumItem(L"CatCookie", L"Media Cat Cookie", tCQCKit::EFldSTypes::CatCookie);
        m_pmeciFldSTypes->c4AddEnumItem(L"ClrLight", L"Color Light", tCQCKit::EFldSTypes::ClrLight);
        m_pmeciFldSTypes->c4AddEnumItem(L"ChannelAdj", L"Channel Adjust", tCQCKit::EFldSTypes::ChannelAdj);
        m_pmeciFldSTypes->c4AddEnumItem(L"ColCookie", L"Media Collection Cookie", tCQCKit::EFldSTypes::ColCookie);
        m_pmeciFldSTypes->c4AddEnumItem(L"ContactClosure", L"Contact Closure", tCQCKit::EFldSTypes::ContactClosure);
        m_pmeciFldSTypes->c4AddEnumItem(L"CurChannel", L"Current Channel", tCQCKit::EFldSTypes::CurChannel);
        m_pmeciFldSTypes->c4AddEnumItem(L"CurExtHumidity", L"Cur Ext Humidity", tCQCKit::EFldSTypes::CurExtHumidity);
        m_pmeciFldSTypes->c4AddEnumItem(L"CurExtTemp", L"Current Ext Temp", tCQCKit::EFldSTypes::CurExtTemp);
        m_pmeciFldSTypes->c4AddEnumItem(L"CurTemp", L"Current Temp", tCQCKit::EFldSTypes::CurTemp);
        m_pmeciFldSTypes->c4AddEnumItem(L"CurWeather", L"Current Weather", tCQCKit::EFldSTypes::CurWeather);
        m_pmeciFldSTypes->c4AddEnumItem(L"DigitalIO", L"Digital I/O", tCQCKit::EFldSTypes::DigitalIO);
        m_pmeciFldSTypes->c4AddEnumItem(L"Dimmer", L"Dimmer", tCQCKit::EFldSTypes::Dimmer);
        m_pmeciFldSTypes->c4AddEnumItem(L"DrvMoniker", L"Driver Moniker", tCQCKit::EFldSTypes::DriverMoniker);
        m_pmeciFldSTypes->c4AddEnumItem(L"Generic", L"Generic", tCQCKit::EFldSTypes::Generic);
        m_pmeciFldSTypes->c4AddEnumItem(L"HighSetPnt", L"High Set Point", tCQCKit::EFldSTypes::HighSetPnt);
        m_pmeciFldSTypes->c4AddEnumItem(L"HumSensor", L"Humidty Sensor", tCQCKit::EFldSTypes::HumSensor);
        m_pmeciFldSTypes->c4AddEnumItem(L"ItemCookie", L"Media Item Cookie", tCQCKit::EFldSTypes::ItemCookie);
        m_pmeciFldSTypes->c4AddEnumItem(L"LEDStatus", L"LED Status", tCQCKit::EFldSTypes::LEDStatus);
        m_pmeciFldSTypes->c4AddEnumItem(L"LightSwitch", L"Light Switch", tCQCKit::EFldSTypes::LightSwitch);
        m_pmeciFldSTypes->c4AddEnumItem(L"LockState", L"Lock State", tCQCKit::EFldSTypes::LockState);
        m_pmeciFldSTypes->c4AddEnumItem(L"LowSetPnt", L"Low Set Point", tCQCKit::EFldSTypes::LowSetPnt);
        m_pmeciFldSTypes->c4AddEnumItem(L"MediaCookie", L"Media Cookie", tCQCKit::EFldSTypes::MediaCookie);
        m_pmeciFldSTypes->c4AddEnumItem(L"MediaRepoDrv", L"Media Repo Driver", tCQCKit::EFldSTypes::MediaRepoDrv);
        m_pmeciFldSTypes->c4AddEnumItem(L"MediaSrc", L"Media Source", tCQCKit::EFldSTypes::MediaSrc);
        m_pmeciFldSTypes->c4AddEnumItem(L"MediaState", L"Media State", tCQCKit::EFldSTypes::MediaState);
        m_pmeciFldSTypes->c4AddEnumItem(L"MediaTransport", L"Media Transport", tCQCKit::EFldSTypes::MediaTransport);
        m_pmeciFldSTypes->c4AddEnumItem(L"Mute", L"Mute", tCQCKit::EFldSTypes::Mute);
        m_pmeciFldSTypes->c4AddEnumItem(L"MotionSensor", L"Motion Sensor", tCQCKit::EFldSTypes::MotionSensor);
        m_pmeciFldSTypes->c4AddEnumItem(L"Power", L"Power", tCQCKit::EFldSTypes::Power);
        m_pmeciFldSTypes->c4AddEnumItem(L"PowerState", L"Power State", tCQCKit::EFldSTypes::PowerState);
        m_pmeciFldSTypes->c4AddEnumItem(L"Relay", L"Relay", tCQCKit::EFldSTypes::Relay);
        m_pmeciFldSTypes->c4AddEnumItem(L"RFIDTag", L"RFID Tag", tCQCKit::EFldSTypes::RFIDTag);
        m_pmeciFldSTypes->c4AddEnumItem(L"SecZoneStat", L"Security Zone Stat", tCQCKit::EFldSTypes::SecZoneStat);
        m_pmeciFldSTypes->c4AddEnumItem(L"SourceInp", L"Source Input", tCQCKit::EFldSTypes::SourceInp);
        m_pmeciFldSTypes->c4AddEnumItem(L"TempSensor", L"Temp Sensor", tCQCKit::EFldSTypes::TempSensor);
        m_pmeciFldSTypes->c4AddEnumItem(L"TitleCookie", L"Media Title Cookie", tCQCKit::EFldSTypes::TitleCookie);
        m_pmeciFldSTypes->c4AddEnumItem(L"TunerFreq", L"Tuner Frequency", tCQCKit::EFldSTypes::TunerFreq);
        m_pmeciFldSTypes->c4AddEnumItem(L"Volume", L"Volume", tCQCKit::EFldSTypes::Volume);
        m_pmeciFldSTypes->c4AddEnumItem(L"VolumeAdj", L"Volume Adjust", tCQCKit::EFldSTypes::VolumeAdj);
        m_pmeciFldSTypes->BaseClassInit(meOwner);
        m_c2EnumId_FldSTypes = meOwner.c2AddClass(m_pmeciFldSTypes);
        bAddNestedType(m_pmeciFldSTypes->strClassPath());
    }

    {
        m_pmeciFldTypes = new TMEngEnumInfo
        (
            meOwner, L"CQCFldTypes", strClassPath(), L"MEng.Enum", 7
        );
        m_pmeciFldTypes->c4AddEnumItem(L"Boolean", L"Boolean", tCQCKit::EFldTypes::Boolean);
        m_pmeciFldTypes->c4AddEnumItem(L"Card", L"Card", tCQCKit::EFldTypes::Card);
        m_pmeciFldTypes->c4AddEnumItem(L"Float", L"Float", tCQCKit::EFldTypes::Float);
        m_pmeciFldTypes->c4AddEnumItem(L"Int", L"Int", tCQCKit::EFldTypes::Int);
        m_pmeciFldTypes->c4AddEnumItem(L"String", L"String", tCQCKit::EFldTypes::String);
        m_pmeciFldTypes->c4AddEnumItem(L"StringList", L"StringList", tCQCKit::EFldTypes::StringList);
        m_pmeciFldTypes->c4AddEnumItem(L"Time", L"Time", tCQCKit::EFldTypes::Time);
        m_pmeciFldTypes->BaseClassInit(meOwner);
        m_c2EnumId_FldTypes = meOwner.c2AddClass(m_pmeciFldTypes);
        bAddNestedType(m_pmeciFldTypes->strClassPath());
    }


    // Add the default constructor
    {
        TMEngMethodInfo methiNew
        (
            L"ctor1_MEng.System.CQC.Runtime.CQCFldDef"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.bIsCtor(kCIDLib::True);
        m_c2MethId_DefCtor = c2AddMethodInfo(methiNew);
    }

    // Check the validity of a field name
    {
        TMEngMethodInfo methiNew
        (
            L"CheckFldName"
            , tCIDMacroEng::EIntrinsics::Boolean
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInParm(L"ToCheck", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_CheckFldName = c2AddMethodInfo(methiNew);
    }

    // Add the equal method
    {
        TMEngMethodInfo methiNew
        (
            L"Equal"
            , tCIDMacroEng::EIntrinsics::Boolean
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInParm(L"Val", c2Id());
        m_c2MethId_Equal = c2AddMethodInfo(methiNew);
    }

    // Add the access
    {
        TMEngMethodInfo methiNew
        (
            L"GetAccess"
            , m_c2EnumId_FldAccess
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        m_c2MethId_GetAccess = c2AddMethodInfo(methiNew);
    }

    // Add the GetId method
    {
        TMEngMethodInfo methiNew
        (
            L"GetId"
            , tCIDMacroEng::EIntrinsics::Card4
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        m_c2MethId_GetId = c2AddMethodInfo(methiNew);
    }

    // Get the limits
    {
        TMEngMethodInfo methiNew
        (
            L"GetLimits"
            , tCIDMacroEng::EIntrinsics::String
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        m_c2MethId_GetLimits = c2AddMethodInfo(methiNew);
    }

    // Get the field name
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

    // Get the 'no store on write' flag
    {
        TMEngMethodInfo methiNew
        (
            L"GetNoStoreOnWrite"
            , tCIDMacroEng::EIntrinsics::Boolean
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        m_c2MethId_GetNoStoreOnWrite = c2AddMethodInfo(methiNew);
    }

    // Get the semantic field type
    {
        TMEngMethodInfo methiNew
        (
            L"GetSemType"
            , m_c2EnumId_FldSTypes
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        m_c2MethId_GetSemType = c2AddMethodInfo(methiNew);
    }

    // Get the field type
    {
        TMEngMethodInfo methiNew
        (
            L"GetType"
            , m_c2EnumId_FldTypes
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        m_c2MethId_GetType = c2AddMethodInfo(methiNew);
    }

    // Setter methods that allow after the fact setup
    {
        TMEngMethodInfo methiNew
        (
            L"Set"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );

        methiNew.c2AddInParm(L"Name", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"Type", m_c2EnumId_FldTypes);
        methiNew.c2AddInParm(L"Access", m_c2EnumId_FldAccess);
        m_c2MethId_Set1 = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"Set2"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );

        methiNew.c2AddInParm(L"Name", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"Type", m_c2EnumId_FldTypes);
        methiNew.c2AddInParm(L"Access", m_c2EnumId_FldAccess);
        methiNew.c2AddInParm(L"SemType", m_c2EnumId_FldSTypes);
        m_c2MethId_Set2 = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"SetAlwaysWrite"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );

        methiNew.c2AddInParm(L"ToSet", tCIDMacroEng::EIntrinsics::Boolean);
        m_c2MethId_SetAlwaysWrite = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"SetQueuedWrite"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );

        methiNew.c2AddInParm(L"ToSet", tCIDMacroEng::EIntrinsics::Boolean);
        m_c2MethId_SetQueuedWrite = c2AddMethodInfo(methiNew);
    }

    // Set the 'no store on write' flag
    {
        TMEngMethodInfo methiNew
        (
            L"SetNoStoreOnWrite"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );

        methiNew.c2AddInParm(L"ToSet", tCIDMacroEng::EIntrinsics::Boolean);
        m_c2MethId_SetNoStoreOnWrite = c2AddMethodInfo(methiNew);
    }

    // Set up this object for a field
    {
        TMEngMethodInfo methiNew
        (
            L"SetWithLimits"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );

        methiNew.c2AddInParm(L"Name", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"Type", m_c2EnumId_FldTypes);
        methiNew.c2AddInParm(L"Access", m_c2EnumId_FldAccess);
        methiNew.c2AddInParm(L"Limits", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_SetWLims = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"SetWithLimits2"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );

        methiNew.c2AddInParm(L"Name", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"Type", m_c2EnumId_FldTypes);
        methiNew.c2AddInParm(L"Access", m_c2EnumId_FldAccess);
        methiNew.c2AddInParm(L"SemType", m_c2EnumId_FldSTypes);
        methiNew.c2AddInParm(L"Limits", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_SetWLims2 = c2AddMethodInfo(methiNew);
    }

    // Some ctors that take initial values
    {
        TMEngMethodInfo methiNew
        (
            L"ctor2_MEng.System.CQC.Runtime.CQCFldDef"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );

        methiNew.c2AddInParm(L"Name", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"Type", m_c2EnumId_FldTypes);
        methiNew.c2AddInParm(L"Access", m_c2EnumId_FldAccess);
        methiNew.bIsCtor(kCIDLib::True);
        m_c2MethId_ValCtor1 = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"ctor3_MEng.System.CQC.Runtime.CQCFldDef"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );

        methiNew.c2AddInParm(L"Name", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"Type", m_c2EnumId_FldTypes);
        methiNew.c2AddInParm(L"Access", m_c2EnumId_FldAccess);
        methiNew.c2AddInParm(L"Limits", tCIDMacroEng::EIntrinsics::String);
        methiNew.bIsCtor(kCIDLib::True);
        m_c2MethId_ValCtor2 = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"ctor4_MEng.System.CQC.Runtime.CQCFldDef"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );

        methiNew.c2AddInParm(L"Name", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"Type", m_c2EnumId_FldTypes);
        methiNew.c2AddInParm(L"Access", m_c2EnumId_FldAccess);
        methiNew.c2AddInParm(L"SemType", m_c2EnumId_FldSTypes);
        methiNew.bIsCtor(kCIDLib::True);
        m_c2MethId_ValCtor3 = c2AddMethodInfo(methiNew);
    }
}


TMEngClassVal*
TCQCFldDefInfo::pmecvMakeStorage(const  TString&                strName
                                ,       TCIDMacroEngine&
                                , const tCIDMacroEng::EConstTypes  eConst) const
{
    return new TCQCFldDefVal(strName, c2Id(), eConst);
}



// ---------------------------------------------------------------------------
//  TCQCFldDefInfo: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TCQCFldDefInfo::bInvokeMethod(          TCIDMacroEngine&    meOwner
                                , const TMEngMethodInfo&    methiTarget
                                ,       TMEngClassVal&      mecvInstance)
{
    TCQCFldDefVal& mecvActual = static_cast<TCQCFldDefVal&>(mecvInstance);

    const tCIDLib::TCard4 c4FirstInd = meOwner.c4FirstParmInd(methiTarget);
    const tCIDLib::TCard2 c2MethId   = methiTarget.c2Id();

    if (c2MethId == m_c2MethId_DefCtor)
    {
        // Reset the value object
        mecvActual.flddValue().Reset();
    }
     else if (c2MethId == m_c2MethId_CheckFldName)
    {
        TMEngBooleanVal& mecvRet
                        = meOwner.mecvStackAtAs<TMEngBooleanVal>(c4FirstInd - 1);
        mecvRet.bValue
        (
            facCQCKit().bIsValidFldName
            (
                meOwner.strStackValAt(c4FirstInd), kCIDLib::False
            )
        );
    }
     else if (c2MethId == TMEngFormattableInfo::c2FormatToId())
    {
        //
        //  The parm is a text stream object. In our case, we can kind of
        //  cheat and just call to it directly.
        //
        TMEngTextOutStreamVal& mecvTarget
                = meOwner.mecvStackAtAs<TMEngTextOutStreamVal>(c4FirstInd);

        mecvTarget.strmTarget(meOwner)
                    << L"Name: " << mecvActual.flddValue().strName() << kCIDLib::NewLn
                    << L"Id: " << mecvActual.flddValue().c4Id() << kCIDLib::NewLn
                    << L"Limits: " << mecvActual.flddValue().strLimits();
    }
     else if (c2MethId == m_c2MethId_Equal)
    {
        TMEngBooleanVal& mecvRet
                        = meOwner.mecvStackAtAs<TMEngBooleanVal>(c4FirstInd - 1);

        TCQCFldDefVal& mecvRHS = meOwner.mecvStackAtAs<TCQCFldDefVal>(c4FirstInd);
        mecvRet.bValue(mecvActual.flddValue() == mecvRHS.flddValue());
    }
     else if (c2MethId == m_c2MethId_GetAccess)
    {
        TMEngEnumVal& mecvRet
                        = meOwner.mecvStackAtAs<TMEngEnumVal>(c4FirstInd - 1);
        mecvRet.c4Ordinal(tCIDLib::c4EnumOrd(mecvActual.flddValue().eAccess()));
    }
     else if (c2MethId == m_c2MethId_GetId)
    {
        TMEngCard4Val& mecvRet
                        = meOwner.mecvStackAtAs<TMEngCard4Val>(c4FirstInd - 1);
        mecvRet.c4Value(mecvActual.flddValue().c4Id());
    }
     else if (c2MethId == m_c2MethId_GetLimits)
    {
        TMEngStringVal& mecvRet
                        = meOwner.mecvStackAtAs<TMEngStringVal>(c4FirstInd - 1);
        mecvRet.strValue(mecvActual.flddValue().strLimits());
    }
     else if (c2MethId == m_c2MethId_GetName)
    {
        TMEngStringVal& mecvRet
                        = meOwner.mecvStackAtAs<TMEngStringVal>(c4FirstInd - 1);
        mecvRet.strValue(mecvActual.flddValue().strName());
    }
     else if (c2MethId == m_c2MethId_GetNoStoreOnWrite)
    {
        TMEngBooleanVal& mecvRet
                        = meOwner.mecvStackAtAs<TMEngBooleanVal>(c4FirstInd - 1);
        mecvRet.bValue(mecvActual.flddValue().bNoStoreOnWrite());
    }
     else if (c2MethId == m_c2MethId_GetType)
    {
        TMEngEnumVal& mecvRet
                        = meOwner.mecvStackAtAs<TMEngEnumVal>(c4FirstInd - 1);
        mecvRet.c4Ordinal(tCIDLib::c4EnumOrd(mecvActual.flddValue().eType()));
    }
     else if (c2MethId == m_c2MethId_GetSemType)
    {
        TMEngEnumVal& mecvRet
                        = meOwner.mecvStackAtAs<TMEngEnumVal>(c4FirstInd - 1);
        mecvRet.c4Ordinal(tCIDLib::c4EnumOrd(mecvActual.flddValue().eSemType()));
    }
     else if ((c2MethId == m_c2MethId_Set1)
          ||  (c2MethId == m_c2MethId_SetWLims)
          ||  (c2MethId == m_c2MethId_Set2)
          ||  (c2MethId == m_c2MethId_SetWLims2))
    {
        // Get the parms that they all have in common
        const TString& strName = meOwner.strStackValAt(c4FirstInd);
        const TMEngEnumVal& mecvType = meOwner.mecvStackAtAs<TMEngEnumVal>(c4FirstInd + 1);
        const TMEngEnumVal& mecvAccess = meOwner.mecvStackAtAs<TMEngEnumVal>(c4FirstInd + 2);

        if (c2MethId == m_c2MethId_Set1)
        {
            mecvActual.flddValue().Set
            (
                strName
                , tCQCKit::EFldTypes(m_pmeciFldTypes->c4MapValue(mecvType))
                , tCQCKit::EFldAccess(m_pmeciFldAcc->c4MapValue(mecvAccess))
            );
        }
         else if (c2MethId == m_c2MethId_Set2)
        {
            const TMEngEnumVal& mecvSType = meOwner.mecvStackAtAs<TMEngEnumVal>(c4FirstInd + 3);
            mecvActual.flddValue().Set
            (
                strName
                , tCQCKit::EFldTypes(m_pmeciFldTypes->c4MapValue(mecvType))
                , tCQCKit::EFldAccess(m_pmeciFldAcc->c4MapValue(mecvAccess))
                , tCQCKit::EFldSTypes(m_pmeciFldSTypes->c4MapValue(mecvSType))
            );
        }
         else if (c2MethId == m_c2MethId_SetWLims)
        {
            mecvActual.flddValue().Set
            (
                strName
                , tCQCKit::EFldTypes(m_pmeciFldTypes->c4MapValue(mecvType))
                , tCQCKit::EFldAccess(m_pmeciFldAcc->c4MapValue(mecvAccess))
                , meOwner.strStackValAt(c4FirstInd + 3)
            );
        }
         else if (c2MethId == m_c2MethId_SetWLims2)
        {
            const TMEngEnumVal& mecvSType = meOwner.mecvStackAtAs<TMEngEnumVal>(c4FirstInd + 3);
            mecvActual.flddValue().Set
            (
                strName
                , tCQCKit::EFldTypes(m_pmeciFldTypes->c4MapValue(mecvType))
                , tCQCKit::EFldAccess(m_pmeciFldAcc->c4MapValue(mecvAccess))
                , tCQCKit::EFldSTypes(m_pmeciFldSTypes->c4MapValue(mecvSType))
                , meOwner.strStackValAt(c4FirstInd + 4)
            );
        }
    }
     else if (c2MethId == m_c2MethId_SetAlwaysWrite)
    {
        mecvActual.flddValue().bAlwaysWrite(meOwner.bStackValAt(c4FirstInd));
    }
     else if (c2MethId == m_c2MethId_SetQueuedWrite)
    {
        mecvActual.flddValue().bQueuedWrite(meOwner.bStackValAt(c4FirstInd));
    }
     else if (c2MethId == m_c2MethId_SetNoStoreOnWrite)
    {
        mecvActual.flddValue().bNoStoreOnWrite(meOwner.bStackValAt(c4FirstInd));
    }
     else if (c2MethId == m_c2MethId_ValCtor1)
    {
        DoValCtor(1, mecvActual, meOwner, c4FirstInd);
    }
     else if (c2MethId == m_c2MethId_ValCtor2)
    {
        DoValCtor(2, mecvActual, meOwner, c4FirstInd);
    }
     else if (c2MethId == m_c2MethId_ValCtor3)
    {
        DoValCtor(3, mecvActual, meOwner, c4FirstInd);
    }
     else
    {
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


// ---------------------------------------------------------------------------
//  TCQCFldDefInfo: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  We have a few value accepting ctors which mostly take the same info,
//  so we have this helper to do the work and it's called from above.
//
tCIDLib::TVoid
TCQCFldDefInfo::DoValCtor(  const   tCIDLib::TCard4     c4Which
                            ,       TCQCFldDefVal&      mecvToSet
                            ,       TCIDMacroEngine&    meOwner
                            , const tCIDLib::TCard4     c4FirstInd)
{
    // Get the common parms together
    const TString& strName= meOwner.strStackValAt(c4FirstInd);
    const tCQCKit::EFldTypes eType = tCQCKit::EFldTypes
    (
        m_pmeciFldTypes->c4MapValue(meOwner.mecvStackAtAs<TMEngEnumVal>(c4FirstInd + 1))
    );

    const tCQCKit::EFldAccess eAcc = tCQCKit::EFldAccess
    (
        m_pmeciFldAcc->c4MapValue(meOwner.mecvStackAtAs<TMEngEnumVal>(c4FirstInd + 2))
    );

    // Call the appropriate one, getting more parms were needed
    if (c4Which == 1)
    {
        mecvToSet.flddValue().Set(strName, eType, eAcc);
    }
     else if (c4Which == 2)
    {
        mecvToSet.flddValue().Set
        (
            strName, eType, eAcc, meOwner.strStackValAt(c4FirstInd + 3)
        );
    }
     else if (c4Which == 3)
    {
        const TMEngEnumVal& mecvSType = meOwner.mecvStackAtAs<TMEngEnumVal>(c4FirstInd + 3);

        mecvToSet.flddValue().Set
        (
            strName
            , eType
            , eAcc
            , tCQCKit::EFldSTypes(m_pmeciFldSTypes->c4MapValue(mecvSType))
        );
    }
}


