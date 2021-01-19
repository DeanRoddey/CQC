// ----------------------------------------------------------------------------
//  FILE: CQCSysCfg_Shared.cpp
//  DATE: Tue, Jan 19 17:39:55 2021 -0500
//
//  This file was generated by the Charmed Quark CIDIDL compiler. Do not make
//  changes by hand, because they will be lost if the file is regenerated.
// ----------------------------------------------------------------------------

#include "CQCSysCfg_.hpp"



static TEnumMap::TEnumValItem aeitemValues_ECfgLists[6] = 
{
    {  tCIDLib::TInt4(tCQCSysCfg::ECfgLists::Lighting), 0, 0,  { L"", L"", L"", L"Lighting", L"ECfgLists::Lighting", L"" } }
  , {  tCIDLib::TInt4(tCQCSysCfg::ECfgLists::Media), 0, 0,  { L"", L"", L"", L"Media", L"ECfgLists::Media", L"" } }
  , {  tCIDLib::TInt4(tCQCSysCfg::ECfgLists::Security), 0, 0,  { L"", L"", L"", L"Security", L"ECfgLists::Security", L"" } }
  , {  tCIDLib::TInt4(tCQCSysCfg::ECfgLists::Weather), 0, 0,  { L"", L"", L"", L"Weather", L"ECfgLists::Weather", L"" } }
  , {  tCIDLib::TInt4(tCQCSysCfg::ECfgLists::XOvers), 0, 0,  { L"", L"", L"", L"XOvers", L"ECfgLists::XOvers", L"" } }
  , {  tCIDLib::TInt4(tCQCSysCfg::ECfgLists::HVAC), 0, 0,  { L"", L"", L"", L"HVAC", L"ECfgLists::HVAC", L"" } }

};

static TEnumMap emapECfgLists
(
     L"ECfgLists"
     , 6
     , kCIDLib::False
     , aeitemValues_ECfgLists
     , nullptr
     , tCIDLib::TCard4(tCQCSysCfg::ECfgLists::Count)
);

const TString& tCQCSysCfg::strXlatECfgLists(const tCQCSysCfg::ECfgLists eVal, const tCIDLib::TBoolean bThrowIfNot)
{
    return emapECfgLists.strMapEnumVal(tCIDLib::TCard4(eVal), TEnumMap::ETextVals::BaseName, bThrowIfNot);
}

tCQCSysCfg::ECfgLists tCQCSysCfg::eXlatECfgLists(const TString& strVal, const tCIDLib::TBoolean bThrowIfNot)
{
    return tCQCSysCfg::ECfgLists(emapECfgLists.i4MapEnumText(strVal, TEnumMap::ETextVals::BaseName, bThrowIfNot));
}

tCIDLib::TBoolean tCQCSysCfg::bIsValidEnum(const tCQCSysCfg::ECfgLists eVal)
{
    return emapECfgLists.bIsValidEnum(tCIDLib::TCard4(eVal));

}



static TEnumMap::TEnumValItem aeitemValues_EGlobActs[8] = 
{
    {  tCIDLib::TInt4(tCQCSysCfg::EGlobActs::OnRoomLoad), 0, 0,  { L"", L"", L"", L"OnRoomLoad", L"EGlobActs::OnRoomLoad", L"Room Loaded" } }
  , {  tCIDLib::TInt4(tCQCSysCfg::EGlobActs::OnIVExit), 0, 0,  { L"", L"", L"", L"OnIVExit", L"EGlobActs::OnIVExit", L"IV Exiting" } }
  , {  tCIDLib::TInt4(tCQCSysCfg::EGlobActs::PreMovie), 0, 0,  { L"", L"", L"", L"PreMovie", L"EGlobActs::PreMovie", L"Movies Selected" } }
  , {  tCIDLib::TInt4(tCQCSysCfg::EGlobActs::PreMusic), 0, 0,  { L"", L"", L"", L"PreMusic", L"EGlobActs::PreMusic", L"Music Selected" } }
  , {  tCIDLib::TInt4(tCQCSysCfg::EGlobActs::MusicPowerOff), 0, 0,  { L"", L"", L"", L"MusicPowerOff", L"EGlobActs::MusicPowerOff", L"Music Power Off" } }
  , {  tCIDLib::TInt4(tCQCSysCfg::EGlobActs::MusicPowerOn), 0, 0,  { L"", L"", L"", L"MusicPowerOn", L"EGlobActs::MusicPowerOn", L"Music Power On" } }
  , {  tCIDLib::TInt4(tCQCSysCfg::EGlobActs::MoviePowerOff), 0, 0,  { L"", L"", L"", L"MoviePowerOff", L"EGlobActs::MoviePowerOff", L"Movie Power Off" } }
  , {  tCIDLib::TInt4(tCQCSysCfg::EGlobActs::MoviePowerOn), 0, 0,  { L"", L"", L"", L"MoviePowerOn", L"EGlobActs::MoviePowerOn", L"Movie Power On" } }

};

static TEnumMap emapEGlobActs
(
     L"EGlobActs"
     , 8
     , kCIDLib::False
     , aeitemValues_EGlobActs
     , nullptr
     , tCIDLib::TCard4(tCQCSysCfg::EGlobActs::Count)
);

const TString& tCQCSysCfg::strXlatEGlobActs(const tCQCSysCfg::EGlobActs eVal, const tCIDLib::TBoolean bThrowIfNot)
{
    return emapEGlobActs.strMapEnumVal(tCIDLib::TCard4(eVal), TEnumMap::ETextVals::Text, bThrowIfNot);
}

tCQCSysCfg::EGlobActs tCQCSysCfg::eXlatEGlobActs(const TString& strVal, const tCIDLib::TBoolean bThrowIfNot)
{
    return tCQCSysCfg::EGlobActs(emapEGlobActs.i4MapEnumText(strVal, TEnumMap::ETextVals::Text, bThrowIfNot));
}

tCIDLib::TVoid TBinInStream_ReadArray(TBinInStream& strmSrc, tCQCSysCfg::EGlobActs* const aeList, const tCIDLib::TCard4 c4Count)
{
    tCIDLib::TCard4 c4Cur;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        strmSrc >> c4Cur;
        aeList[c4Index] = tCQCSysCfg::EGlobActs(c4Cur);
    }
}
tCIDLib::TVoid TBinOutStream_WriteArray(TBinOutStream& strmTar, const tCQCSysCfg::EGlobActs* const aeList, const tCIDLib::TCard4 c4Count)
{
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        strmTar << tCIDLib::TCard4(aeList[c4Index]);
}
tCIDLib::TBoolean tCQCSysCfg::bIsValidEnum(const tCQCSysCfg::EGlobActs eVal)
{
    return emapEGlobActs.bIsValidEnum(tCIDLib::TCard4(eVal));

}



static TEnumMap::TEnumValItem aeitemValues_ERmFuncs[6] = 
{
    {  tCIDLib::TInt4(tCQCSysCfg::ERmFuncs::Lighting), 0, 0,  { L"", L"", L"", L"Lighting", L"ERmFuncs::Lighting", L"" } }
  , {  tCIDLib::TInt4(tCQCSysCfg::ERmFuncs::Movies), 0, 0,  { L"", L"", L"", L"Movies", L"ERmFuncs::Movies", L"" } }
  , {  tCIDLib::TInt4(tCQCSysCfg::ERmFuncs::Music), 0, 0,  { L"", L"", L"", L"Music", L"ERmFuncs::Music", L"" } }
  , {  tCIDLib::TInt4(tCQCSysCfg::ERmFuncs::Security), 0, 0,  { L"", L"", L"", L"Security", L"ERmFuncs::Security", L"" } }
  , {  tCIDLib::TInt4(tCQCSysCfg::ERmFuncs::Weather), 0, 0,  { L"", L"", L"", L"Weather", L"ERmFuncs::Weather", L"" } }
  , {  tCIDLib::TInt4(tCQCSysCfg::ERmFuncs::HVAC), 0, 0,  { L"", L"", L"", L"HVAC", L"ERmFuncs::HVAC", L"" } }

};

static TEnumMap emapERmFuncs
(
     L"ERmFuncs"
     , 6
     , kCIDLib::False
     , aeitemValues_ERmFuncs
     , nullptr
     , tCIDLib::TCard4(tCQCSysCfg::ERmFuncs::Count)
);

const TString& tCQCSysCfg::strXlatERmFuncs(const tCQCSysCfg::ERmFuncs eVal, const tCIDLib::TBoolean bThrowIfNot)
{
    return emapERmFuncs.strMapEnumVal(tCIDLib::TCard4(eVal), TEnumMap::ETextVals::BaseName, bThrowIfNot);
}

tCQCSysCfg::ERmFuncs tCQCSysCfg::eXlatERmFuncs(const TString& strVal, const tCIDLib::TBoolean bThrowIfNot)
{
    return tCQCSysCfg::ERmFuncs(emapERmFuncs.i4MapEnumText(strVal, TEnumMap::ETextVals::BaseName, bThrowIfNot));
}

tCIDLib::TBoolean tCQCSysCfg::bIsValidEnum(const tCQCSysCfg::ERmFuncs eVal)
{
    return emapERmFuncs.bIsValidEnum(tCIDLib::TCard4(eVal));

}

