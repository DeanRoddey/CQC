//
// FILE NAME: CQCMEng_FieldDefClass.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/01/2014
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
//  class which provides helper functions to V2 classes (and other V2 oriented
//  code potentially.)
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
#include    "CQCMEng_V2HelperClass_.hpp"


// ---------------------------------------------------------------------------
//  Magic RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TV2HelperVal,TMEngClassVal)
RTTIDecls(TV2HelperInfo,TMEngClassInfo)


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace CQCMEng_V2HelperClass
{
    namespace
    {
        // -----------------------------------------------------------------------
        //  The names for the types that we support here. Each derivative has to
        //  be able to return strings that contain its name and full name.
        // -----------------------------------------------------------------------
        const TString   strV2HelperDevClassPath(L"MEng.System.CQC.Runtime.V2Helper.DevClasses");
        const TString   strV2Helper(L"V2Helper");
        const TString   strV2HelperClassPath(L"MEng.System.CQC.Runtime.V2Helper");
        const TString   strV2HelperBasePath(L"MEng.System.CQC.Runtime");
    }
}


// ---------------------------------------------------------------------------
//  CLASS: TV2HelperVal
// PREFIX: mecv
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TV2HelperVal: Constuctors and Destructor
// ---------------------------------------------------------------------------
TV2HelperVal::TV2HelperVal( const   TString&                strName
                            , const tCIDLib::TCard2         c2ClassId
                            , const tCIDMacroEng::EConstTypes  eConst) :

    TMEngClassVal(strName, c2ClassId, eConst)
{
}

TV2HelperVal::~TV2HelperVal()
{
}



// ---------------------------------------------------------------------------
//  CLASS: TV2HelperInfo
// PREFIX: meci
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TV2HelperInfo: Public, static methods
// ---------------------------------------------------------------------------
const TString& TV2HelperInfo::strDevClassPath()
{
    return CQCMEng_V2HelperClass::strV2HelperDevClassPath;
}

const TString& TV2HelperInfo::strClassPath()
{
    return CQCMEng_V2HelperClass::strV2HelperClassPath;
}


// ---------------------------------------------------------------------------
//  TV2HelperInfo: Constructors and Destructor
// ---------------------------------------------------------------------------
TV2HelperInfo::TV2HelperInfo(TCIDMacroEngine& meOwner) :

    TMEngClassInfo
    (
        CQCMEng_V2HelperClass::strV2Helper
        , CQCMEng_V2HelperClass::strV2HelperBasePath
        , meOwner
        , kCIDLib::False
        , tCIDMacroEng::EClassExt::Final
        , L"MEng.Object"
    )
    , m_c2EnumId_DevClasses(kCIDMacroEng::c2BadId)
    , m_c2EnumId_PowerStatus(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetAIOFlds(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetAudioFlds(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetAVProcFlds(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetDevInfoFlds(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetDIOFlds(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetLightFlds(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetMTransFlds(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetNowPlayingFlds(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetPowerFlds(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetProjFlds(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetSecAreaFlds(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetSecZoneFlds(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetSwitcherFlds(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetSwitcherFlds2(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetThermoFlds(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetTunerFlds(kCIDMacroEng::c2BadId)
    , m_c2TypeId_FldDef(kCIDMacroEng::c2BadId)
    , m_c2TypeId_FldList(kCIDMacroEng::c2BadId)
    , m_pmeciDevClasses(nullptr)
    , m_pmeciPowerStatus(nullptr)
{
    bAddClassImport(TCQCFldDefInfo::strClassPath());
}

TV2HelperInfo::~TV2HelperInfo()
{
}


tCIDLib::TVoid TV2HelperInfo::Init(TCIDMacroEngine& meOwner)
{
    //
    //  Create an enum to wrap the C++ tCQCKit::EDevClasses enum.
    //
    {
        m_pmeciDevClasses = new TMEngEnumInfo
        (
            meOwner
            , L"DevClasses"
            , strClassPath()
            , L"MEng.Enum"
            , 25
        );

        m_pmeciDevClasses->c4AddEnumItem(L"None"            , L"None"  , tCQCKit::EDevClasses::None           );
        m_pmeciDevClasses->c4AddEnumItem(L"Audio"           , L"AUD"   , tCQCKit::EDevClasses::Audio          );
        m_pmeciDevClasses->c4AddEnumItem(L"AIO"             , L"AIO"   , tCQCKit::EDevClasses::AIO            );
        m_pmeciDevClasses->c4AddEnumItem(L"AVProcessor"     , L"AVPRC" , tCQCKit::EDevClasses::AVProcessor    );
        m_pmeciDevClasses->c4AddEnumItem(L"ClrLighting"     , L"CLGHT" , tCQCKit::EDevClasses::ClrLighting    );
        m_pmeciDevClasses->c4AddEnumItem(L"ContactClosure"  , L"CTCL"  , tCQCKit::EDevClasses::ContactClosure );
        m_pmeciDevClasses->c4AddEnumItem(L"DeviceInfo"      , L"DEVI"  , tCQCKit::EDevClasses::DeviceInfo     );
        m_pmeciDevClasses->c4AddEnumItem(L"DIO"             , L"DIO"   , tCQCKit::EDevClasses::DIO            );
        m_pmeciDevClasses->c4AddEnumItem(L"Lighting"        , L"LGHT"  , tCQCKit::EDevClasses::Lighting       );
        m_pmeciDevClasses->c4AddEnumItem(L"Lock"            , L"LOCK"  , tCQCKit::EDevClasses::Lock           );
        m_pmeciDevClasses->c4AddEnumItem(L"MediaRenderer"   , L"MREND" , tCQCKit::EDevClasses::MediaRenderer  );
        m_pmeciDevClasses->c4AddEnumItem(L"MediaRepository" , L"MREPO" , tCQCKit::EDevClasses::MediaRepository);
        m_pmeciDevClasses->c4AddEnumItem(L"MediaTransport"  , L"MTRANS", tCQCKit::EDevClasses::MediaTransport );
        m_pmeciDevClasses->c4AddEnumItem(L"MotionSensor"    , L"MOT"   , tCQCKit::EDevClasses::MotionSensor   );
        m_pmeciDevClasses->c4AddEnumItem(L"NowPlaying"      , L"NWPLY" , tCQCKit::EDevClasses::NowPlaying     );
        m_pmeciDevClasses->c4AddEnumItem(L"Power"           , L"PWR"   , tCQCKit::EDevClasses::Power          );
        m_pmeciDevClasses->c4AddEnumItem(L"Projector"       , L"PROJ"  , tCQCKit::EDevClasses::Projector      );
        m_pmeciDevClasses->c4AddEnumItem(L"Relay"           , L"RELY"  , tCQCKit::EDevClasses::Relay          );
        m_pmeciDevClasses->c4AddEnumItem(L"ResMon"          , L"RESMON", tCQCKit::EDevClasses::ResMon         );
        m_pmeciDevClasses->c4AddEnumItem(L"SceneCtrl"       , L"SCNE"  , tCQCKit::EDevClasses::SceneCtrl      );
        m_pmeciDevClasses->c4AddEnumItem(L"Security"        , L"SEC"   , tCQCKit::EDevClasses::Security       );
        m_pmeciDevClasses->c4AddEnumItem(L"Switcher"        , L"SWTCH" , tCQCKit::EDevClasses::Switcher       );
        m_pmeciDevClasses->c4AddEnumItem(L"Thermostat"      , L"THERM" , tCQCKit::EDevClasses::Thermostat     );
        m_pmeciDevClasses->c4AddEnumItem(L"Tuner"           , L"TUNR"  , tCQCKit::EDevClasses::Tuner          );
        m_pmeciDevClasses->c4AddEnumItem(L"TV"              , L"TV"    , tCQCKit::EDevClasses::TV             );
        m_pmeciDevClasses->c4AddEnumItem(L"Weather"         , L"WEATH" , tCQCKit::EDevClasses::Weather        );
        m_pmeciDevClasses->BaseClassInit(meOwner);
        m_c2EnumId_DevClasses = meOwner.c2AddClass(m_pmeciDevClasses);
        bAddNestedType(m_pmeciDevClasses->strClassPath());
    }


    // Create an enum for the standard V2 power status values
    {
        m_pmeciPowerStatus = new TMEngEnumInfo
        (
            meOwner
            , L"PowerStatus"
            , strClassPath()
            , L"MEng.Enum"
            , 5
        );
        m_pmeciPowerStatus->c4AddEnumItem(L"Off", L"Off", tCQCKit::EPowerStatus::Off);
        m_pmeciPowerStatus->c4AddEnumItem(L"Starting", L"Starting", tCQCKit::EPowerStatus::Starting);
        m_pmeciPowerStatus->c4AddEnumItem(L"Ready", L"Ready", tCQCKit::EPowerStatus::Ready);
        m_pmeciPowerStatus->c4AddEnumItem(L"Stopping", L"Stopping", tCQCKit::EPowerStatus::Stopping);
        m_pmeciPowerStatus->c4AddEnumItem(L"Failed", L"Failed", tCQCKit::EPowerStatus::Failed);
        m_pmeciPowerStatus->BaseClassInit(meOwner);
        m_c2EnumId_PowerStatus = meOwner.c2AddClass(m_pmeciPowerStatus);
        bAddNestedType(m_pmeciPowerStatus->strClassPath());
    }

    // Get the field def class now, since it needs to be used below.
    m_c2TypeId_FldDef = meOwner.c2FindClassId(TCQCFldDefInfo::strClassPath());

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
            , L"V2HFldDefList"
            , CQCMEng_V2HelperClass::strV2HelperClassPath
            , TMEngVectorInfo::strPath()
            , m_c2TypeId_FldDef
        );
        TJanitor<TMEngVectorInfo> janNewClass(pmeciNew);
        pmeciNew->BaseClassInit(meOwner);
        bAddNestedType(pmeciNew->strClassPath());
        m_c2TypeId_FldList = meOwner.c2AddClass(janNewClass.pobjOrphan());
    }


    // Add the default constructor
    {
        TMEngMethodInfo methiNew
        (
            L"ctor1_MEng.System.CQC.Runtime.V2Helper"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.bIsCtor(kCIDLib::True);
        m_c2MethId_DefCtor = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"GetAIOFlds"
            , tCIDMacroEng::EIntrinsics::Card4
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInOutParm(L"ToFill", m_c2TypeId_FldList);
        methiNew.c2AddInParm(L"SubIOName", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"IsInput", tCIDMacroEng::EIntrinsics::Boolean);
        methiNew.c2AddInParm(L"IsFloat", tCIDMacroEng::EIntrinsics::Boolean);
        methiNew.c2AddInParm(L"AnaLimit", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_GetAIOFlds = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"GetAudioFlds"
            , tCIDMacroEng::EIntrinsics::Card4
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInOutParm(L"ToFill", m_c2TypeId_FldList);
        methiNew.c2AddInParm(L"SubUnit", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_GetAudioFlds = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"GetAVProcFlds"
            , tCIDMacroEng::EIntrinsics::Card4
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInOutParm(L"ToFill", m_c2TypeId_FldList);
        methiNew.c2AddInParm(L"SubUnit", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"CurLim", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"SetLim", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"SetWriteAlways", tCIDMacroEng::EIntrinsics::Boolean);
        m_c2MethId_GetAVProcFlds = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"GetDevInfoFlds"
            , tCIDMacroEng::EIntrinsics::Card4
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInOutParm(L"ToFill", m_c2TypeId_FldList);
        methiNew.c2AddInParm(L"SubUnit", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_GetDevInfoFlds = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"GetDIOFlds"
            , tCIDMacroEng::EIntrinsics::Card4
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInOutParm(L"ToFill", m_c2TypeId_FldList);
        methiNew.c2AddInParm(L"SubIOName", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"IsInput", tCIDMacroEng::EIntrinsics::Boolean);
        m_c2MethId_GetDIOFlds = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"GetLightFlds"
            , tCIDMacroEng::EIntrinsics::Card4
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInOutParm(L"ToFill", m_c2TypeId_FldList);
        methiNew.c2AddInParm(L"LightName", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"DoDimmer", tCIDMacroEng::EIntrinsics::Boolean);
        methiNew.c2AddInParm(L"DoSwitch", tCIDMacroEng::EIntrinsics::Boolean);
        m_c2MethId_GetLightFlds = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"GetMTransFlds"
            , tCIDMacroEng::EIntrinsics::Card4
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInOutParm(L"ToFill", m_c2TypeId_FldList);
        methiNew.c2AddInParm(L"SubUnit", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_GetMTransFlds = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"GetNowPlayingFlds"
            , tCIDMacroEng::EIntrinsics::Card4
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInOutParm(L"ToFill", m_c2TypeId_FldList);
        methiNew.c2AddInParm(L"SubUnit", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_GetNowPlayingFlds = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"GetPowerFlds"
            , tCIDMacroEng::EIntrinsics::Card4
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInOutParm(L"ToFill", m_c2TypeId_FldList);
        methiNew.c2AddInParm(L"SubUnit", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_GetPowerFlds = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"GetProjFlds"
            , tCIDMacroEng::EIntrinsics::Card4
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInOutParm(L"ToFill", m_c2TypeId_FldList);
        methiNew.c2AddInParm(L"SubUnit", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"ARLimit", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"ARWriteAlways", tCIDMacroEng::EIntrinsics::Boolean);
        m_c2MethId_GetProjFlds = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"GetSecAreaFlds"
            , tCIDMacroEng::EIntrinsics::Card4
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInOutParm(L"ToFill", m_c2TypeId_FldList);
        methiNew.c2AddInParm(L"SubUnit", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"ArmLimit", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_GetSecAreaFlds = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"GetSecZoneFlds"
            , tCIDMacroEng::EIntrinsics::Card4
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInOutParm(L"ToFill", m_c2TypeId_FldList);
        methiNew.c2AddInParm(L"SubUnit", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"IsMotion", tCIDMacroEng::EIntrinsics::Boolean);
        m_c2MethId_GetSecZoneFlds = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"GetSwitcherFlds"
            , tCIDMacroEng::EIntrinsics::Card4
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInOutParm(L"ToFill", m_c2TypeId_FldList);
        methiNew.c2AddInParm(L"SubUnit", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"SrcLim", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"SetWriteAlways", tCIDMacroEng::EIntrinsics::Boolean);
        m_c2MethId_GetSwitcherFlds = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"GetSwitcherFlds2"
            , tCIDMacroEng::EIntrinsics::Card4
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInOutParm(L"ToFill", m_c2TypeId_FldList);
        methiNew.c2AddInParm(L"SubUnit", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"CurSrcLim", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"SetSrcLim", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"SetWriteAlways", tCIDMacroEng::EIntrinsics::Boolean);
        m_c2MethId_GetSwitcherFlds2 = c2AddMethodInfo(methiNew);
    }


    {
        TMEngMethodInfo methiNew
        (
            L"GetThermoFlds"
            , tCIDMacroEng::EIntrinsics::Card4
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInOutParm(L"ToFill", m_c2TypeId_FldList);
        methiNew.c2AddInParm(L"SubUnit", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"TempLim", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"HSPLim", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"LSPLim", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"FanLim", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"FanOpLim", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"ModeLim", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"OpModeLim", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_GetThermoFlds = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"GetTunerFlds"
            , tCIDMacroEng::EIntrinsics::Card4
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInOutParm(L"ToFill", m_c2TypeId_FldList);
        methiNew.c2AddInParm(L"SubUnit", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"PreLim", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"WritePreset", tCIDMacroEng::EIntrinsics::Boolean);
        m_c2MethId_GetTunerFlds = c2AddMethodInfo(methiNew);
    }
}


TMEngClassVal*
TV2HelperInfo::pmecvMakeStorage(const   TString&                strName
                                ,       TCIDMacroEngine&
                                , const tCIDMacroEng::EConstTypes  eConst) const
{
    return new TV2HelperVal(strName, c2Id(), eConst);
}



// ---------------------------------------------------------------------------
//  TV2HelperInfo: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TV2HelperInfo::bInvokeMethod(       TCIDMacroEngine&    meOwner
                            , const TMEngMethodInfo&    methiTarget
                            ,       TMEngClassVal&      mecvInstance)
{
    TV2HelperVal& mecvActual = static_cast<TV2HelperVal&>(mecvInstance);

    const tCIDLib::TCard4 c4FirstInd = meOwner.c4FirstParmInd(methiTarget);
    const tCIDLib::TCard2 c2MethId   = methiTarget.c2Id();

    //
    //  All of the helpers that get the field defs for a specific class just call
    //  the C++ helper, then call CopyFldsBack() to copy those defs over to the
    //  CML output vector, which is always the first parameter.
    //
    if (c2MethId == m_c2MethId_DefCtor)
    {
    }
     else if (c2MethId == m_c2MethId_GetAIOFlds)
    {
        mecvActual.m_colTmpFlds.RemoveAll();
        const tCIDLib::TCard4 c4Ret = TCQCDevClass::c4LoadAIOFlds
        (
            mecvActual.m_colTmpFlds
            , meOwner.strStackValAt(c4FirstInd + 1)
            , meOwner.bStackValAt(c4FirstInd + 2)
            , meOwner.bStackValAt(c4FirstInd + 3)
            , meOwner.strStackValAt(c4FirstInd + 4)
        );
        CopyFldsBack(meOwner, c4FirstInd, mecvActual.m_colTmpFlds, c4Ret);
    }
     else if (c2MethId == m_c2MethId_GetAudioFlds)
    {
        mecvActual.m_colTmpFlds.RemoveAll();
        const tCIDLib::TCard4 c4Ret = TCQCDevClass::c4LoadAudioFlds
        (
            mecvActual.m_colTmpFlds, meOwner.strStackValAt(c4FirstInd + 1)
        );
        CopyFldsBack(meOwner, c4FirstInd, mecvActual.m_colTmpFlds, c4Ret);
    }
     else if (c2MethId == m_c2MethId_GetAVProcFlds)
    {
        mecvActual.m_colTmpFlds.RemoveAll();
        const tCIDLib::TCard4 c4Ret = TCQCDevClass::c4LoadAVProcFlds
        (
            mecvActual.m_colTmpFlds
            , meOwner.strStackValAt(c4FirstInd + 1)
            , meOwner.strStackValAt(c4FirstInd + 2)
            , meOwner.strStackValAt(c4FirstInd + 3)
            , meOwner.bStackValAt(c4FirstInd + 4)
        );
        CopyFldsBack(meOwner, c4FirstInd, mecvActual.m_colTmpFlds, c4Ret);
    }
     else if (c2MethId == m_c2MethId_GetDevInfoFlds)
    {
        mecvActual.m_colTmpFlds.RemoveAll();
        const tCIDLib::TCard4 c4Ret = TCQCDevClass::c4LoadDevInfoFlds
        (
            mecvActual.m_colTmpFlds
            , meOwner.strStackValAt(c4FirstInd + 1)
        );
        CopyFldsBack(meOwner, c4FirstInd, mecvActual.m_colTmpFlds, c4Ret);
    }
     else if (c2MethId == m_c2MethId_GetDIOFlds)
    {
        mecvActual.m_colTmpFlds.RemoveAll();
        const tCIDLib::TCard4 c4Ret = TCQCDevClass::c4LoadDIOFlds
        (
            mecvActual.m_colTmpFlds
            , meOwner.strStackValAt(c4FirstInd + 1)
            , meOwner.bStackValAt(c4FirstInd + 2)
        );
        CopyFldsBack(meOwner, c4FirstInd, mecvActual.m_colTmpFlds, c4Ret);
    }
     else if (c2MethId == m_c2MethId_GetLightFlds)
    {
        mecvActual.m_colTmpFlds.RemoveAll();
        const tCIDLib::TCard4 c4Ret = TCQCDevClass::c4LoadLightFlds
        (
            mecvActual.m_colTmpFlds
            , meOwner.strStackValAt(c4FirstInd + 1)
            , meOwner.bStackValAt(c4FirstInd + 2)
            , meOwner.bStackValAt(c4FirstInd + 3)
        );
        CopyFldsBack(meOwner, c4FirstInd, mecvActual.m_colTmpFlds, c4Ret);
    }
     else if (c2MethId == m_c2MethId_GetMTransFlds)
    {
        mecvActual.m_colTmpFlds.RemoveAll();
        const tCIDLib::TCard4 c4Ret = TCQCDevClass::c4LoadMTransFlds
        (
            mecvActual.m_colTmpFlds, meOwner.strStackValAt(c4FirstInd + 1)
        );
        CopyFldsBack(meOwner, c4FirstInd, mecvActual.m_colTmpFlds, c4Ret);
    }
     else if (c2MethId == m_c2MethId_GetNowPlayingFlds)
    {
        mecvActual.m_colTmpFlds.RemoveAll();
        const tCIDLib::TCard4 c4Ret = TCQCDevClass::c4LoadNowPlayingFlds
        (
            mecvActual.m_colTmpFlds, meOwner.strStackValAt(c4FirstInd + 1)
        );
        CopyFldsBack(meOwner, c4FirstInd, mecvActual.m_colTmpFlds, c4Ret);
    }
     else if (c2MethId == m_c2MethId_GetPowerFlds)
    {
        mecvActual.m_colTmpFlds.RemoveAll();
        const tCIDLib::TCard4 c4Ret = TCQCDevClass::c4LoadPowerFlds
        (
            mecvActual.m_colTmpFlds
            , meOwner.strStackValAt(c4FirstInd + 1)
        );
        CopyFldsBack(meOwner, c4FirstInd, mecvActual.m_colTmpFlds, c4Ret);
    }
     else if (c2MethId == m_c2MethId_GetProjFlds)
    {
        mecvActual.m_colTmpFlds.RemoveAll();
        const tCIDLib::TCard4 c4Ret = TCQCDevClass::c4LoadProjFlds
        (
            mecvActual.m_colTmpFlds
            , meOwner.strStackValAt(c4FirstInd + 1)
            , meOwner.strStackValAt(c4FirstInd + 2)
            , meOwner.bStackValAt(c4FirstInd + 3)
        );
        CopyFldsBack(meOwner, c4FirstInd, mecvActual.m_colTmpFlds, c4Ret);
    }
     else if (c2MethId == m_c2MethId_GetSecAreaFlds)
    {
        mecvActual.m_colTmpFlds.RemoveAll();
        const tCIDLib::TCard4 c4Ret = TCQCDevClass::c4LoadSecAreaFlds
        (
            mecvActual.m_colTmpFlds
            , meOwner.strStackValAt(c4FirstInd + 1)
            , meOwner.strStackValAt(c4FirstInd + 2)
        );
        CopyFldsBack(meOwner, c4FirstInd, mecvActual.m_colTmpFlds, c4Ret);
    }
     else if (c2MethId == m_c2MethId_GetSecZoneFlds)
    {
        mecvActual.m_colTmpFlds.RemoveAll();
        const tCIDLib::TCard4 c4Ret = TCQCDevClass::c4LoadSecZoneFlds
        (
            mecvActual.m_colTmpFlds
            , meOwner.strStackValAt(c4FirstInd + 1)
            , meOwner.bStackValAt(c4FirstInd + 2)
        );
        CopyFldsBack(meOwner, c4FirstInd, mecvActual.m_colTmpFlds, c4Ret);
    }
     else if (c2MethId == m_c2MethId_GetSwitcherFlds)
    {
        mecvActual.m_colTmpFlds.RemoveAll();

        // Pass an empty string for the readable source limit value
        const tCIDLib::TCard4 c4Ret = TCQCDevClass::c4LoadSwitcherFlds
        (
            mecvActual.m_colTmpFlds
            , meOwner.strStackValAt(c4FirstInd + 1)
            , TString::strEmpty()
            , meOwner.strStackValAt(c4FirstInd + 2)
            , meOwner.bStackValAt(c4FirstInd + 3)
        );
        CopyFldsBack(meOwner, c4FirstInd, mecvActual.m_colTmpFlds, c4Ret);
    }
     else if (c2MethId == m_c2MethId_GetSwitcherFlds2)
    {
        mecvActual.m_colTmpFlds.RemoveAll();

        // This one supports different settable vs readable sources
        const tCIDLib::TCard4 c4Ret = TCQCDevClass::c4LoadSwitcherFlds
        (
            mecvActual.m_colTmpFlds
            , meOwner.strStackValAt(c4FirstInd + 1)
            , meOwner.strStackValAt(c4FirstInd + 2)
            , meOwner.strStackValAt(c4FirstInd + 3)
            , meOwner.bStackValAt(c4FirstInd + 4)
        );
        CopyFldsBack(meOwner, c4FirstInd, mecvActual.m_colTmpFlds, c4Ret);
    }
     else if (c2MethId == m_c2MethId_GetThermoFlds)
    {
        mecvActual.m_colTmpFlds.RemoveAll();
        const tCIDLib::TCard4 c4Ret = TCQCDevClass::c4LoadThermoFlds
        (
            mecvActual.m_colTmpFlds
            , meOwner.strStackValAt(c4FirstInd + 1)
            , meOwner.strStackValAt(c4FirstInd + 2)
            , meOwner.strStackValAt(c4FirstInd + 3)
            , meOwner.strStackValAt(c4FirstInd + 4)
            , meOwner.strStackValAt(c4FirstInd + 5)
            , meOwner.strStackValAt(c4FirstInd + 6)
            , meOwner.strStackValAt(c4FirstInd + 7)
            , meOwner.strStackValAt(c4FirstInd + 8)
        );
        CopyFldsBack(meOwner, c4FirstInd, mecvActual.m_colTmpFlds, c4Ret);
    }
     else if (c2MethId == m_c2MethId_GetTunerFlds)
    {
        mecvActual.m_colTmpFlds.RemoveAll();
        const tCIDLib::TCard4 c4Ret = TCQCDevClass::c4LoadTunerFlds
        (
            mecvActual.m_colTmpFlds
            , meOwner.strStackValAt(c4FirstInd + 1)
            , meOwner.strStackValAt(c4FirstInd + 2)
            , meOwner.bStackValAt(c4FirstInd + 3)
        );
        CopyFldsBack(meOwner, c4FirstInd, mecvActual.m_colTmpFlds, c4Ret);
    }
     else
    {
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


// ---------------------------------------------------------------------------
//  TV2HelperInfo: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  All of the methods that query device class field defs ultimately just need to
//  copy back over the fields gotten to the CML vector of field defs and give back
//  the return count. The output list is always the first parm, so we can do that
//  work for all of them here.
//
tCIDLib::TVoid
TV2HelperInfo::CopyFldsBack(        TCIDMacroEngine&        meOwner
                            , const tCIDLib::TCard4         c4FirstInd
                            , const tCQCKit::TFldDefList&   colFlds
                            , const tCIDLib::TCard4         c4RetCnt)
{
    //
    //  Get to the CML vector of field defs, then iterate the passed list and
    //  add them to the CML vector.
    //
    TMEngVectorVal& mecvOut = meOwner.mecvStackAtAs<TMEngVectorVal>(c4FirstInd);
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4RetCnt; c4Index++)
    {
        const TCQCFldDef& flddCur = colFlds[c4Index];

        mecvOut.AddObject
        (
            new TCQCFldDefVal
            (
                TString::strEmpty()
                , m_c2TypeId_FldDef
                , tCIDMacroEng::EConstTypes::NonConst
                , colFlds[c4Index]
            )
        );

    }

    TMEngCard4Val& mecvRet = meOwner.mecvStackAtAs<TMEngCard4Val>(c4FirstInd - 1);
    mecvRet.c4Value(c4RetCnt);
}


