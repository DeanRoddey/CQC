// ----------------------------------------------------------------------------
//  FILE: PlexRepoS_Types.cpp
//  DATE: Tue, Jan 19 17:39:56 2021 -0500
//
//  This file was generated by the Charmed Quark CIDIDL compiler. Do not make
//  changes by hand, because they will be lost if the file is regenerated.
// ----------------------------------------------------------------------------

#include "PlexRepoS_.hpp"



static TEnumMap::TEnumValItem aeitemValues_EElems[8] = 
{
    {  tCIDLib::TInt4(tPlexRepoS::EElems::None), 0, 0,  { L"", L"", L"", L"None", L"EElems::None", L"" } }
  , {  tCIDLib::TInt4(tPlexRepoS::EElems::Director), 0, 0,  { L"", L"", L"", L"Director", L"EElems::Director", L"Director" } }
  , {  tCIDLib::TInt4(tPlexRepoS::EElems::Genre), 0, 0,  { L"", L"", L"", L"Genre", L"EElems::Genre", L"Genre" } }
  , {  tCIDLib::TInt4(tPlexRepoS::EElems::Media), 0, 0,  { L"", L"", L"", L"Media", L"EElems::Media", L"Media" } }
  , {  tCIDLib::TInt4(tPlexRepoS::EElems::Part), 0, 0,  { L"", L"", L"", L"Part", L"EElems::Part", L"Part" } }
  , {  tCIDLib::TInt4(tPlexRepoS::EElems::Role), 0, 0,  { L"", L"", L"", L"Role", L"EElems::Role", L"Role" } }
  , {  tCIDLib::TInt4(tPlexRepoS::EElems::Root), 0, 0,  { L"", L"", L"", L"Root", L"EElems::Root", L"MediaContainer" } }
  , {  tCIDLib::TInt4(tPlexRepoS::EElems::Video), 0, 0,  { L"", L"", L"", L"Video", L"EElems::Video", L"Video" } }

};

static TEnumMap emapEElems
(
     L"EElems"
     , 8
     , kCIDLib::False
     , aeitemValues_EElems
     , nullptr
     , tCIDLib::TCard4(tPlexRepoS::EElems::Count)
);

const TString& tPlexRepoS::strAltXlatEElems(const tPlexRepoS::EElems eVal, const tCIDLib::TBoolean bThrowIfNot)
{
    return emapEElems.strMapEnumVal(tCIDLib::TCard4(eVal), TEnumMap::ETextVals::BaseName, bThrowIfNot);
}

tPlexRepoS::EElems tPlexRepoS::eAltXlatEElems(const TString& strVal, const tCIDLib::TBoolean bThrowIfNot)
{
    return tPlexRepoS::EElems(emapEElems.i4MapEnumText(strVal, TEnumMap::ETextVals::BaseName, bThrowIfNot));
}

const TString& tPlexRepoS::strXlatEElems(const tPlexRepoS::EElems eVal, const tCIDLib::TBoolean bThrowIfNot)
{
    return emapEElems.strMapEnumVal(tCIDLib::TCard4(eVal), TEnumMap::ETextVals::Text, bThrowIfNot);
}

tPlexRepoS::EElems tPlexRepoS::eXlatEElems(const TString& strVal, const tCIDLib::TBoolean bThrowIfNot)
{
    return tPlexRepoS::EElems(emapEElems.i4MapEnumText(strVal, TEnumMap::ETextVals::Text, bThrowIfNot));
}

tCIDLib::TBoolean tPlexRepoS::bIsValidEnum(const tPlexRepoS::EElems eVal)
{
    return emapEElems.bIsValidEnum(tCIDLib::TCard4(eVal));

}



static TEnumMap::TEnumValItem aeitemValues_EAttrs[17] = 
{
    {  tCIDLib::TInt4(tPlexRepoS::EAttrs::Added), 0, 0,  { L"", L"addedAt", L"", L"Added", L"EAttrs::Added", L"" } }
  , {  tCIDLib::TInt4(tPlexRepoS::EAttrs::AspectRatio), 0, 0,  { L"", L"aspectRatio", L"", L"AspectRatio", L"EAttrs::AspectRatio", L"" } }
  , {  tCIDLib::TInt4(tPlexRepoS::EAttrs::Art), 0, 0,  { L"", L"art", L"", L"Art", L"EAttrs::Art", L"" } }
  , {  tCIDLib::TInt4(tPlexRepoS::EAttrs::ArtThumb), 0, 0,  { L"", L"thumb", L"", L"ArtThumb", L"EAttrs::ArtThumb", L"" } }
  , {  tCIDLib::TInt4(tPlexRepoS::EAttrs::Channels), 0, 0,  { L"", L"audioChannels", L"", L"Channels", L"EAttrs::Channels", L"" } }
  , {  tCIDLib::TInt4(tPlexRepoS::EAttrs::Director), 0, 0,  { L"", L"", L"", L"Director", L"EAttrs::Director", L"" } }
  , {  tCIDLib::TInt4(tPlexRepoS::EAttrs::Descr), 0, 0,  { L"", L"summary", L"", L"Descr", L"EAttrs::Descr", L"" } }
  , {  tCIDLib::TInt4(tPlexRepoS::EAttrs::Duration), 0, 0,  { L"", L"duration", L"", L"Duration", L"EAttrs::Duration", L"" } }
  , {  tCIDLib::TInt4(tPlexRepoS::EAttrs::FilePath), 0, 0,  { L"", L"file", L"", L"FilePath", L"EAttrs::FilePath", L"" } }
  , {  tCIDLib::TInt4(tPlexRepoS::EAttrs::Genre), 0, 0,  { L"", L"", L"", L"Genre", L"EAttrs::Genre", L"" } }
  , {  tCIDLib::TInt4(tPlexRepoS::EAttrs::Id), 0, 0,  { L"", L"", L"", L"Id", L"EAttrs::Id", L"" } }
  , {  tCIDLib::TInt4(tPlexRepoS::EAttrs::Key), 0, 0,  { L"", L"key", L"", L"Key", L"EAttrs::Key", L"" } }
  , {  tCIDLib::TInt4(tPlexRepoS::EAttrs::Rating), 0, 0,  { L"", L"contentRating", L"", L"Rating", L"EAttrs::Rating", L"" } }
  , {  tCIDLib::TInt4(tPlexRepoS::EAttrs::Studio), 0, 0,  { L"", L"studio", L"", L"Studio", L"EAttrs::Studio", L"" } }
  , {  tCIDLib::TInt4(tPlexRepoS::EAttrs::Title), 0, 0,  { L"", L"title", L"", L"Title", L"EAttrs::Title", L"" } }
  , {  tCIDLib::TInt4(tPlexRepoS::EAttrs::UserRating), 0, 0,  { L"", L"Rating", L"", L"UserRating", L"EAttrs::UserRating", L"" } }
  , {  tCIDLib::TInt4(tPlexRepoS::EAttrs::Year), 0, 0,  { L"", L"year", L"", L"Year", L"EAttrs::Year", L"" } }

};

static TEnumMap emapEAttrs
(
     L"EAttrs"
     , 17
     , kCIDLib::False
     , aeitemValues_EAttrs
     , nullptr
     , tCIDLib::TCard4(tPlexRepoS::EAttrs::Count)
);

const TString& tPlexRepoS::strAltXlatEAttrs(const tPlexRepoS::EAttrs eVal, const tCIDLib::TBoolean bThrowIfNot)
{
    return emapEAttrs.strMapEnumVal(tCIDLib::TCard4(eVal), TEnumMap::ETextVals::AltText, bThrowIfNot);
}

tPlexRepoS::EAttrs tPlexRepoS::eAltXlatEAttrs(const TString& strVal, const tCIDLib::TBoolean bThrowIfNot)
{
    return tPlexRepoS::EAttrs(emapEAttrs.i4MapEnumText(strVal, TEnumMap::ETextVals::AltText, bThrowIfNot));
}

tCIDLib::TBoolean tPlexRepoS::bIsValidEnum(const tPlexRepoS::EAttrs eVal)
{
    return emapEAttrs.bIsValidEnum(tCIDLib::TCard4(eVal));

}

