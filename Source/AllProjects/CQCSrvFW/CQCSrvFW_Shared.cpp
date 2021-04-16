// ----------------------------------------------------------------------------
//  FILE: CQCSrvFW_Shared.cpp
//  DATE: Fri, Feb 12 21:14:15 2021 -0500
//
//  This file was generated by the Charmed Quark CIDIDL compiler. Do not make
//  changes by hand, because they will be lost if the file is regenerated.
// ----------------------------------------------------------------------------

#include "CQCSrvFW_.hpp"



static TEnumMap::TEnumValItem aeitemValues_ESrvOpts[3] = 
{
    {  tCIDLib::TInt4(tCQCSrvFW::ESrvOpts::EventsIn), 0, 0,  { L"", L"", L"", L"EventsIn", L"ESrvOpts::EventsIn", L"" } }
  , {  tCIDLib::TInt4(tCQCSrvFW::ESrvOpts::EventsOut), 0, 0,  { L"", L"", L"", L"EventsOut", L"ESrvOpts::EventsOut", L"" } }
  , {  tCIDLib::TInt4(tCQCSrvFW::ESrvOpts::LogIn), 0, 0,  { L"", L"", L"", L"LogIn", L"ESrvOpts::LogIn", L"" } }

};

static TEnumMap emapESrvOpts
(
     L"ESrvOpts"
     , 3
     , kCIDLib::True
     , aeitemValues_ESrvOpts
     , nullptr
     , tCIDLib::TCard4(tCQCSrvFW::ESrvOpts::None)
);

const TString& tCQCSrvFW::strXlatESrvOpts(const tCQCSrvFW::ESrvOpts eVal, const tCIDLib::TBoolean bThrowIfNot)
{
    return emapESrvOpts.strMapEnumVal(tCIDLib::TCard4(eVal), TEnumMap::ETextVals::BaseName, bThrowIfNot);
}

tCQCSrvFW::ESrvOpts tCQCSrvFW::eXlatESrvOpts(const TString& strVal, const tCIDLib::TBoolean bThrowIfNot)
{
    return tCQCSrvFW::ESrvOpts(emapESrvOpts.i4MapEnumText(strVal, TEnumMap::ETextVals::BaseName, bThrowIfNot));
}

tCIDLib::TBoolean tCQCSrvFW::bIsValidEnum(const tCQCSrvFW::ESrvOpts eVal)
{
    return emapESrvOpts.bIsValidEnum(tCIDLib::TCard4(eVal));

}



static TEnumMap::TEnumValItem aeitemValues_ESrvStates[19] = 
{
    {  tCIDLib::TInt4(tCQCSrvFW::ESrvStates::Start), 0, 0,  { L"", L"", L"", L"Start", L"ESrvStates::Start", L"Starting" } }
  , {  tCIDLib::TInt4(tCQCSrvFW::ESrvStates::LoadEnv), 0, 0,  { L"", L"", L"", L"LoadEnv", L"ESrvStates::LoadEnv", L"Load environmental info" } }
  , {  tCIDLib::TInt4(tCQCSrvFW::ESrvStates::CreateEvent), 0, 0,  { L"", L"", L"", L"CreateEvent", L"ESrvStates::CreateEvent", L"Create event" } }
  , {  tCIDLib::TInt4(tCQCSrvFW::ESrvStates::SetSigHandler), 0, 0,  { L"", L"", L"", L"SetSigHandler", L"ESrvStates::SetSigHandler", L"Register signal handler" } }
  , {  tCIDLib::TInt4(tCQCSrvFW::ESrvStates::InitClientOrb), 0, 0,  { L"", L"", L"", L"InitClientOrb", L"ESrvStates::InitClientOrb", L"Init client side ORB" } }
  , {  tCIDLib::TInt4(tCQCSrvFW::ESrvStates::InstallLogger), 0, 0,  { L"", L"", L"", L"InstallLogger", L"ESrvStates::InstallLogger", L"Install logger" } }
  , {  tCIDLib::TInt4(tCQCSrvFW::ESrvStates::ParseParms), 0, 0,  { L"", L"", L"", L"ParseParms", L"ESrvStates::ParseParms", L"Parse parameters" } }
  , {  tCIDLib::TInt4(tCQCSrvFW::ESrvStates::MakeDirs), 0, 0,  { L"", L"", L"", L"MakeDirs", L"ESrvStates::MakeDirs", L"Make directories" } }
  , {  tCIDLib::TInt4(tCQCSrvFW::ESrvStates::LogIn), 0, 0,  { L"", L"", L"", L"LogIn", L"ESrvStates::LogIn", L"Log in" } }
  , {  tCIDLib::TInt4(tCQCSrvFW::ESrvStates::InitServerOrb), 0, 0,  { L"", L"", L"", L"InitServerOrb", L"ESrvStates::InitServerOrb", L"Init server side ORB" } }
  , {  tCIDLib::TInt4(tCQCSrvFW::ESrvStates::LoadConfig), 0, 0,  { L"", L"", L"", L"LoadConfig", L"ESrvStates::LoadConfig", L"Load configuration" } }
  , {  tCIDLib::TInt4(tCQCSrvFW::ESrvStates::WaitPrereqs), 0, 0,  { L"", L"", L"", L"WaitPrereqs", L"ESrvStates::WaitPrereqs", L"Wait for prerequisites" } }
  , {  tCIDLib::TInt4(tCQCSrvFW::ESrvStates::PreRegInit), 0, 0,  { L"", L"", L"", L"PreRegInit", L"ESrvStates::PreRegInit", L"Pre-object registration init" } }
  , {  tCIDLib::TInt4(tCQCSrvFW::ESrvStates::RegSrvObjects), 0, 0,  { L"", L"", L"", L"RegSrvObjects", L"ESrvStates::RegSrvObjects", L"Register server objects" } }
  , {  tCIDLib::TInt4(tCQCSrvFW::ESrvStates::StartWorkers), 0, 0,  { L"", L"", L"", L"StartWorkers", L"ESrvStates::StartWorkers", L"Start worker threads" } }
  , {  tCIDLib::TInt4(tCQCSrvFW::ESrvStates::PreBindInit), 0, 0,  { L"", L"", L"", L"PreBindInit", L"ESrvStates::PreBindInit", L"Pre-object binding init" } }
  , {  tCIDLib::TInt4(tCQCSrvFW::ESrvStates::StartRebinder), 0, 0,  { L"", L"", L"", L"StartRebinder", L"ESrvStates::StartRebinder", L"Start ORB rebinder" } }
  , {  tCIDLib::TInt4(tCQCSrvFW::ESrvStates::EnableEvents), 0, 0,  { L"", L"", L"", L"EnableEvents", L"ESrvStates::EnableEvents", L"Enable event processing" } }
  , {  tCIDLib::TInt4(tCQCSrvFW::ESrvStates::Ready), 0, 0,  { L"", L"", L"", L"Ready", L"ESrvStates::Ready", L"Ready" } }

};

static TEnumMap emapESrvStates
(
     L"ESrvStates"
     , 19
     , kCIDLib::False
     , aeitemValues_ESrvStates
     , nullptr
     , tCIDLib::TCard4(tCQCSrvFW::ESrvStates::Count)
);

const TString& tCQCSrvFW::strXlatESrvStates(const tCQCSrvFW::ESrvStates eVal, const tCIDLib::TBoolean bThrowIfNot)
{
    return emapESrvStates.strMapEnumVal(tCIDLib::TCard4(eVal), TEnumMap::ETextVals::BaseName, bThrowIfNot);
}

tCQCSrvFW::ESrvStates tCQCSrvFW::eXlatESrvStates(const TString& strVal, const tCIDLib::TBoolean bThrowIfNot)
{
    return tCQCSrvFW::ESrvStates(emapESrvStates.i4MapEnumText(strVal, TEnumMap::ETextVals::BaseName, bThrowIfNot));
}

tCIDLib::TBoolean tCQCSrvFW::bIsValidEnum(const tCQCSrvFW::ESrvStates eVal)
{
    return emapESrvStates.bIsValidEnum(tCIDLib::TCard4(eVal));

}



static TEnumMap::TEnumValItem aeitemValues_EStateRes[3] = 
{
    {  tCIDLib::TInt4(tCQCSrvFW::EStateRes::Success), 0, 0,  { L"", L"", L"", L"Success", L"EStateRes::Success", L"" } }
  , {  tCIDLib::TInt4(tCQCSrvFW::EStateRes::Retry), 0, 0,  { L"", L"", L"", L"Retry", L"EStateRes::Retry", L"" } }
  , {  tCIDLib::TInt4(tCQCSrvFW::EStateRes::Failed), 0, 0,  { L"", L"", L"", L"Failed", L"EStateRes::Failed", L"" } }

};

static TEnumMap emapEStateRes
(
     L"EStateRes"
     , 3
     , kCIDLib::False
     , aeitemValues_EStateRes
     , nullptr
     , tCIDLib::TCard4(tCQCSrvFW::EStateRes::Count)
);

const TString& tCQCSrvFW::strXlatEStateRes(const tCQCSrvFW::EStateRes eVal, const tCIDLib::TBoolean bThrowIfNot)
{
    return emapEStateRes.strMapEnumVal(tCIDLib::TCard4(eVal), TEnumMap::ETextVals::BaseName, bThrowIfNot);
}

tCQCSrvFW::EStateRes tCQCSrvFW::eXlatEStateRes(const TString& strVal, const tCIDLib::TBoolean bThrowIfNot)
{
    return tCQCSrvFW::EStateRes(emapEStateRes.i4MapEnumText(strVal, TEnumMap::ETextVals::BaseName, bThrowIfNot));
}

tCIDLib::TBoolean tCQCSrvFW::bIsValidEnum(const tCQCSrvFW::EStateRes eVal)
{
    return emapEStateRes.bIsValidEnum(tCIDLib::TCard4(eVal));

}

