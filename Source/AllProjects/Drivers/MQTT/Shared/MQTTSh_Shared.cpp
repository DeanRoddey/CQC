// ----------------------------------------------------------------------------
//  FILE: MQTTSh_Shared.cpp
//  DATE: Tue, Jan 19 17:39:56 2021 -0500
//
//  This file was generated by the Charmed Quark CIDIDL compiler. Do not make
//  changes by hand, because they will be lost if the file is regenerated.
// ----------------------------------------------------------------------------

#include "MQTTSh_.hpp"



static TEnumMap::TEnumValItem aeitemValues_EClStates[8] = 
{
    {  tCIDLib::TInt4(tMQTTSh::EClStates::LoadConfig), 0, 0,  { L"", L"", L"", L"LoadConfig", L"EClStates::LoadConfig", L"" } }
  , {  tCIDLib::TInt4(tMQTTSh::EClStates::Initialize), 0, 0,  { L"", L"", L"", L"Initialize", L"EClStates::Initialize", L"" } }
  , {  tCIDLib::TInt4(tMQTTSh::EClStates::WaitServer), 0, 0,  { L"", L"", L"", L"WaitServer", L"EClStates::WaitServer", L"" } }
  , {  tCIDLib::TInt4(tMQTTSh::EClStates::Connecting), 0, 0,  { L"", L"", L"", L"Connecting", L"EClStates::Connecting", L"" } }
  , {  tCIDLib::TInt4(tMQTTSh::EClStates::Subscribing), 0, 0,  { L"", L"", L"", L"Subscribing", L"EClStates::Subscribing", L"" } }
  , {  tCIDLib::TInt4(tMQTTSh::EClStates::Ready), 0, 0,  { L"", L"", L"", L"Ready", L"EClStates::Ready", L"" } }
  , {  tCIDLib::TInt4(tMQTTSh::EClStates::Disconnecting), 0, 0,  { L"", L"", L"", L"Disconnecting", L"EClStates::Disconnecting", L"" } }
  , {  tCIDLib::TInt4(tMQTTSh::EClStates::Disconnected), 0, 0,  { L"", L"", L"", L"Disconnected", L"EClStates::Disconnected", L"" } }

};

static TEnumMap emapEClStates
(
     L"EClStates"
     , 8
     , kCIDLib::False
     , aeitemValues_EClStates
     , nullptr
     , tCIDLib::TCard4(tMQTTSh::EClStates::Count)
);

const TString& tMQTTSh::strLoadEClStates(const tMQTTSh::EClStates eVal, const tCIDLib::TBoolean bThrowIfNot)
{
    return emapEClStates.strMapEnumVal(tCIDLib::TCard4(eVal), TEnumMap::ETextVals::BaseName, bThrowIfNot);
}
TTextOutStream& tMQTTSh::operator<<(TTextOutStream& strmTar, const tMQTTSh::EClStates eVal)
{
    strmTar << emapEClStates.strMapEnumVal(tCIDLib::TCard4(eVal), TEnumMap::ETextVals::BaseName, kCIDLib::False);
    return strmTar;
}
tCIDLib::TBoolean tMQTTSh::bIsValidEnum(const tMQTTSh::EClStates eVal)
{
    return emapEClStates.bIsValidEnum(tCIDLib::TCard4(eVal));

}



static TEnumMap::TEnumValItem aeitemValues_EMapDirs[3] = 
{
    {  tCIDLib::TInt4(tMQTTSh::EMapDirs::In), 0, 0,  { L"", L"", L"", L"In", L"EMapDirs::In", L"" } }
  , {  tCIDLib::TInt4(tMQTTSh::EMapDirs::Out), 0, 0,  { L"", L"", L"", L"Out", L"EMapDirs::Out", L"" } }
  , {  tCIDLib::TInt4(tMQTTSh::EMapDirs::InOut), 0, 0,  { L"", L"", L"", L"InOut", L"EMapDirs::InOut", L"" } }

};

static TEnumMap emapEMapDirs
(
     L"EMapDirs"
     , 3
     , kCIDLib::False
     , aeitemValues_EMapDirs
     , nullptr
     , tCIDLib::TCard4(tMQTTSh::EMapDirs::Count)
);

const TString& tMQTTSh::strXlatEMapDirs(const tMQTTSh::EMapDirs eVal, const tCIDLib::TBoolean bThrowIfNot)
{
    return emapEMapDirs.strMapEnumVal(tCIDLib::TCard4(eVal), TEnumMap::ETextVals::BaseName, bThrowIfNot);
}

tMQTTSh::EMapDirs tMQTTSh::eXlatEMapDirs(const TString& strVal, const tCIDLib::TBoolean bThrowIfNot)
{
    return tMQTTSh::EMapDirs(emapEMapDirs.i4MapEnumText(strVal, TEnumMap::ETextVals::BaseName, bThrowIfNot));
}

tCIDLib::TBoolean tMQTTSh::bIsValidEnum(const tMQTTSh::EMapDirs eVal)
{
    return emapEMapDirs.bIsValidEnum(tCIDLib::TCard4(eVal));

}



static TEnumMap::TEnumValItem aeitemValues_EInMapRes[4] = 
{
    {  tCIDLib::TInt4(tMQTTSh::EInMapRes::BadVal), 0, 0,  { L"", L"", L"", L"BadVal", L"EInMapRes::BadVal", L"" } }
  , {  tCIDLib::TInt4(tMQTTSh::EInMapRes::NotFound), 0, 0,  { L"", L"", L"", L"NotFound", L"EInMapRes::NotFound", L"" } }
  , {  tCIDLib::TInt4(tMQTTSh::EInMapRes::Ignore), 0, 0,  { L"", L"", L"", L"Ignore", L"EInMapRes::Ignore", L"" } }
  , {  tCIDLib::TInt4(tMQTTSh::EInMapRes::GoodVal), 0, 0,  { L"", L"", L"", L"GoodVal", L"EInMapRes::GoodVal", L"" } }

};

static TEnumMap emapEInMapRes
(
     L"EInMapRes"
     , 4
     , kCIDLib::False
     , aeitemValues_EInMapRes
     , nullptr
     , tCIDLib::TCard4(tMQTTSh::EInMapRes::Count)
);

const TString& tMQTTSh::strXlatEInMapRes(const tMQTTSh::EInMapRes eVal, const tCIDLib::TBoolean bThrowIfNot)
{
    return emapEInMapRes.strMapEnumVal(tCIDLib::TCard4(eVal), TEnumMap::ETextVals::BaseName, bThrowIfNot);
}

tMQTTSh::EInMapRes tMQTTSh::eXlatEInMapRes(const TString& strVal, const tCIDLib::TBoolean bThrowIfNot)
{
    return tMQTTSh::EInMapRes(emapEInMapRes.i4MapEnumText(strVal, TEnumMap::ETextVals::BaseName, bThrowIfNot));
}

tCIDLib::TBoolean tMQTTSh::bIsValidEnum(const tMQTTSh::EInMapRes eVal)
{
    return emapEInMapRes.bIsValidEnum(tCIDLib::TCard4(eVal));

}



static TEnumMap::TEnumValItem aeitemValues_EMsgSrcs[3] = 
{
    {  tCIDLib::TInt4(tMQTTSh::EMsgSrcs::None), 0, 0,  { L"", L"", L"", L"None", L"EMsgSrcs::None", L"" } }
  , {  tCIDLib::TInt4(tMQTTSh::EMsgSrcs::Driver), 0, 0,  { L"", L"DRV", L"", L"Driver", L"EMsgSrcs::Driver", L"" } }
  , {  tCIDLib::TInt4(tMQTTSh::EMsgSrcs::IOThread), 0, 0,  { L"", L"IOT", L"", L"IOThread", L"EMsgSrcs::IOThread", L"" } }

};

static TEnumMap emapEMsgSrcs
(
     L"EMsgSrcs"
     , 3
     , kCIDLib::False
     , aeitemValues_EMsgSrcs
     , nullptr
     , tCIDLib::TCard4(tMQTTSh::EMsgSrcs::Count)
);

TTextOutStream& tMQTTSh::operator<<(TTextOutStream& strmTar, const tMQTTSh::EMsgSrcs eVal)
{
    strmTar << emapEMsgSrcs.strMapEnumVal(tCIDLib::TCard4(eVal), TEnumMap::ETextVals::AltText, kCIDLib::False);
    return strmTar;
}
tCIDLib::TBoolean tMQTTSh::bIsValidEnum(const tMQTTSh::EMsgSrcs eVal)
{
    return emapEMsgSrcs.bIsValidEnum(tCIDLib::TCard4(eVal));

}



static TEnumMap::TEnumValItem aeitemValues_EPLTypes[4] = 
{
    {  tCIDLib::TInt4(tMQTTSh::EPLTypes::BinText), 0, 0,  { L"", L"", L"", L"BinText", L"EPLTypes::BinText", L"" } }
  , {  tCIDLib::TInt4(tMQTTSh::EPLTypes::Card), 0, 0,  { L"", L"", L"", L"Card", L"EPLTypes::Card", L"" } }
  , {  tCIDLib::TInt4(tMQTTSh::EPLTypes::Int), 0, 0,  { L"", L"", L"", L"Int", L"EPLTypes::Int", L"" } }
  , {  tCIDLib::TInt4(tMQTTSh::EPLTypes::Text), 0, 0,  { L"", L"", L"", L"Text", L"EPLTypes::Text", L"" } }

};

static TEnumMap emapEPLTypes
(
     L"EPLTypes"
     , 4
     , kCIDLib::False
     , aeitemValues_EPLTypes
     , nullptr
     , tCIDLib::TCard4(tMQTTSh::EPLTypes::Count)
);

const TString& tMQTTSh::strXlatEPLTypes(const tMQTTSh::EPLTypes eVal, const tCIDLib::TBoolean bThrowIfNot)
{
    return emapEPLTypes.strMapEnumVal(tCIDLib::TCard4(eVal), TEnumMap::ETextVals::BaseName, bThrowIfNot);
}

tMQTTSh::EPLTypes tMQTTSh::eXlatEPLTypes(const TString& strVal, const tCIDLib::TBoolean bThrowIfNot)
{
    return tMQTTSh::EPLTypes(emapEPLTypes.i4MapEnumText(strVal, TEnumMap::ETextVals::BaseName, bThrowIfNot));
}

TTextOutStream& tMQTTSh::operator<<(TTextOutStream& strmTar, const tMQTTSh::EPLTypes eVal)
{
    strmTar << emapEPLTypes.strMapEnumVal(tCIDLib::TCard4(eVal), TEnumMap::ETextVals::BaseName, kCIDLib::False);
    return strmTar;
}
tCIDLib::TBoolean tMQTTSh::bIsValidEnum(const tMQTTSh::EPLTypes eVal)
{
    return emapEPLTypes.bIsValidEnum(tCIDLib::TCard4(eVal));

}

