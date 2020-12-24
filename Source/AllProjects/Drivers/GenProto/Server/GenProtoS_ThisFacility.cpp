//
// FILE NAME: GenProtoS_ThisFacility.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/24/2003
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
//  This is the implementation file for the facility class.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "GenProtoS_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TFacGenProtoS,TFacility)



// ---------------------------------------------------------------------------
//   CLASS: TFacGenProtoS
//  PREFIX: fac
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TFacGenProtoS: Public, static methods
// ---------------------------------------------------------------------------
tGenProtoS::ETypes
TFacGenProtoS::eFldTypeToExprType(const tCQCKit::EFldTypes eFldType)
{
    if (eFldType == tCQCKit::EFldTypes::Boolean)
        return tGenProtoS::ETypes::Boolean;
    else if (eFldType == tCQCKit::EFldTypes::Card)
        return tGenProtoS::ETypes::Card4;
    else if (eFldType == tCQCKit::EFldTypes::Float)
        return tGenProtoS::ETypes::Float8;
    else if (eFldType == tCQCKit::EFldTypes::Int)
        return tGenProtoS::ETypes::Int4;
    else if (eFldType == tCQCKit::EFldTypes::String)
        return tGenProtoS::ETypes::String;

    // Not a known one so return the count value
    return tGenProtoS::ETypes::Count;
}


tGenProtoS::ETypes
TFacGenProtoS::eTokenToType(const tGenProtoS::ETokens eToken)
{
    if (eToken == tGenProtoS::ETokens::ToBool)
        return tGenProtoS::ETypes::Boolean;
    else if (eToken == tGenProtoS::ETokens::ToCard1)
        return tGenProtoS::ETypes::Card1;
    else if (eToken == tGenProtoS::ETokens::ToCard2)
        return tGenProtoS::ETypes::Card2;
    else if (eToken == tGenProtoS::ETokens::ToCard4)
        return tGenProtoS::ETypes::Card4;
    else if (eToken == tGenProtoS::ETokens::ToFloat4)
        return tGenProtoS::ETypes::Float4;
    else if (eToken == tGenProtoS::ETokens::ToFloat8)
        return tGenProtoS::ETypes::Float8;
    else if (eToken == tGenProtoS::ETokens::ToInt1)
        return tGenProtoS::ETypes::Int1;
    else if (eToken == tGenProtoS::ETokens::ToInt2)
        return tGenProtoS::ETypes::Int2;
    else if (eToken == tGenProtoS::ETokens::ToInt4)
        return tGenProtoS::ETypes::Int4;
    else if (eToken == tGenProtoS::ETokens::ToString)
        return tGenProtoS::ETypes::String;

    // Not a known one so return the count value
    return tGenProtoS::ETypes::Count;
}


tGenProtoS::ETokens
TFacGenProtoS::eTypeToToken(const tGenProtoS::ETypes eType)
{
    if (eType == tGenProtoS::ETypes::Boolean)
        return tGenProtoS::ETokens::ToBool;
    else if (eType == tGenProtoS::ETypes::Card1)
        return tGenProtoS::ETokens::ToCard1;
    else if (eType == tGenProtoS::ETypes::Card2)
        return tGenProtoS::ETokens::ToCard2;
    else if (eType == tGenProtoS::ETypes::Card4)
        return tGenProtoS::ETokens::ToCard4;
    else if (eType == tGenProtoS::ETypes::Float4)
        return tGenProtoS::ETokens::ToFloat4;
    else if (eType == tGenProtoS::ETypes::Float8)
        return tGenProtoS::ETokens::ToFloat8;
    else if (eType == tGenProtoS::ETypes::Int1)
        return tGenProtoS::ETokens::ToInt1;
    else if (eType == tGenProtoS::ETypes::Int2)
        return tGenProtoS::ETokens::ToInt2;
    else if (eType == tGenProtoS::ETypes::Int4)
        return tGenProtoS::ETokens::ToInt4;
    else if (eType == tGenProtoS::ETypes::String)
        return tGenProtoS::ETokens::ToString;

    // Not a known one so return the unknown
    return tGenProtoS::ETokens::NoMatch;
}


tGenProtoS::ETypes TFacGenProtoS::eXlatTypeName(const TString& strToXlat)
{
    if (strToXlat == L"Boolean")
        return tGenProtoS::ETypes::Boolean;
    else if (strToXlat == L"Card1")
        return tGenProtoS::ETypes::Card1;
    else if (strToXlat == L"Card2")
        return tGenProtoS::ETypes::Card2;
    else if (strToXlat == L"Card4")
        return tGenProtoS::ETypes::Card4;
    else if (strToXlat == L"Float4")
        return tGenProtoS::ETypes::Float4;
    else if (strToXlat == L"Float8")
        return tGenProtoS::ETypes::Float8;
    else if (strToXlat == L"Int1")
        return tGenProtoS::ETypes::Int1;
    else if (strToXlat == L"Int2")
        return tGenProtoS::ETypes::Int2;
    else if (strToXlat == L"Int4")
        return tGenProtoS::ETypes::Int4;
    else if (strToXlat == L"String")
        return tGenProtoS::ETypes::String;

    // Not a known one so return the count value
    return tGenProtoS::ETypes::Count;
}


const TString& TFacGenProtoS::strXlatType(const tGenProtoS::ETypes eType)
{
    const TString* pstrRet;
    switch(eType)
    {
        case tGenProtoS::ETypes::Boolean :
        {
            static const TString strBool(L"Boolean");
            pstrRet = &strBool;
            break;
        }

        case tGenProtoS::ETypes::Card1 :
        {
            static const TString strCard1(L"Card1");
            pstrRet = &strCard1;
            break;
        }

        case tGenProtoS::ETypes::Card2 :
        {
            static const TString strCard2(L"Card2");
            pstrRet = &strCard2;
            break;
        }

        case tGenProtoS::ETypes::Card4 :
        {
            static const TString strCard4(L"Card4");
            pstrRet = &strCard4;
            break;
        }

        case tGenProtoS::ETypes::Int1 :
        {
            static const TString strInt1(L"Int1");
            pstrRet = &strInt1;
            break;
        }

        case tGenProtoS::ETypes::Int2 :
        {
            static const TString strInt2(L"Int2");
            pstrRet = &strInt2;
            break;
        }

        case tGenProtoS::ETypes::Int4 :
        {
            static const TString strInt4(L"Int4");
            pstrRet = &strInt4;
            break;
        }

        case tGenProtoS::ETypes::Float4 :
        {
            static const TString strFloat4(L"Float4");
            pstrRet = &strFloat4;
            break;
        }

        case tGenProtoS::ETypes::Float8 :
        {
            static const TString strFloat8(L"Float8");
            pstrRet = &strFloat8;
            break;
        }

        case tGenProtoS::ETypes::String :
        {
            static const TString strString(L"String");
            pstrRet = &strString;
            break;
        }

        default :
        {
            static const TString strUnknown(L"???");
            pstrRet = &strUnknown;
            break;
        }
    }
    return *pstrRet;
}


// ---------------------------------------------------------------------------
//  TFacGenProtoS: Constructors and Destructor
// ---------------------------------------------------------------------------
TFacGenProtoS::TFacGenProtoS() :

    TFacility
    (
        L"GenProtoS"
        , tCIDLib::EModTypes::SharedLib
        , kGenProtoS::c4MajVersion
        , kGenProtoS::c4MinVersion
        , kGenProtoS::c4Revision
        , tCIDLib::EModFlags::HasMsgFile
    )
{
}

TFacGenProtoS::~TFacGenProtoS()
{
}


