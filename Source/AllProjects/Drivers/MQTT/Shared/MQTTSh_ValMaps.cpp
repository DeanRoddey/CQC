//
// FILE NAME: MQTTSh_SubInfo.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/31/2018
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
//  This is the implementation for the config related classes
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "MQTTSh_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TMQTTValMap, TObject)
RTTIDecls(TMQTTComplexMap, TMQTTValMap)
RTTIDecls(TMQTTBoolGenMap, TMQTTValMap)
RTTIDecls(TMQTTBoolNumMap, TMQTTValMap)
RTTIDecls(TMQTTBoolTextMap, TMQTTValMap)
RTTIDecls(TMQTTEnumMap, TMQTTValMap)
RTTIDecls(TMQTTJSONMap, TMQTTComplexMap)
RTTIDecls(TMQTTMLineMap, TMQTTComplexMap)
RTTIDecls(TMQTTPassthroughMap, TMQTTValMap)
RTTIDecls(TMQTTScaleRangeMap, TMQTTValMap)
RTTIDecls(TMQTTTextMap, TMQTTValMap)



// ---------------------------------------------------------------------------
//   CLASS: TMQTTValMap
//  PREFIX: mqvmp
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TMQTTValMap :  Public, virtual methods
// ---------------------------------------------------------------------------

//
//  For most maps these are just pass-through methods, since values in the MQTT msgs
//  are not embedded in a large piece of text.
//
tMQTTSh::EInMapRes
TMQTTValMap::ePreMapInVal(const TString& strInVal, TString& strMQTTVal) const
{
    strMQTTVal = strInVal;
    return tMQTTSh::EInMapRes::GoodVal;
}

tCIDLib::TBoolean
TMQTTValMap::bPostMapOutVal(const TString& strMQTTVal, TString& strOutVal) const
{
    strOutVal = strMQTTVal;
    return kCIDLib::True;
}


// ---------------------------------------------------------------------------
//  TMQTTValMap :  Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TMQTTValMap::AddErr(        tCIDLib::TStrCollect&   colErrs
                    , const TString&                strFldName
                    , const TString&                strErr
                    , const MFormattable&           mfmtblToken1
                    , const MFormattable&           mfmtblToken2)
{
    TString strText(L"Value map error for field ", 512UL);
    strText += strFldName;
    strText += L" - ";
    strText += strErr;

    if (!MFormattable::bIsNullObject(mfmtblToken1))
        strText.eReplaceToken(mfmtblToken1, kCIDLib::chDigit1);

    if (!MFormattable::bIsNullObject(mfmtblToken2))
        strText.eReplaceToken(mfmtblToken2, kCIDLib::chDigit2);

    colErrs.objAdd(strText);
}



// ---------------------------------------------------------------------------
//   CLASS: TMQTTComplexMap
//  PREFIX: mqvmp
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TMQTTComplexMap : Public destructor
// ---------------------------------------------------------------------------
TMQTTComplexMap::~TMQTTComplexMap()
{
}


// ---------------------------------------------------------------------------
//  TMQTTComplexMap : Public, inherited methods
// ---------------------------------------------------------------------------

//
//  The derived classes will all us first and then parse its own stuff if we are
//  successful.
//
tCIDLib::TBoolean
TMQTTComplexMap::bParseFrom(const   TString&                strFldName
                            , const tCQCKit::EFldAccess     eAccess
                            , const TXMLTreeElement&        xtnodeSrc
                            ,       tCIDLib::TStrCollect&   colErrs)
{
    const tCIDLib::TCard4 c4ErrCnt = colErrs.c4ElemCount();
    m_bStripWS = xtnodeSrc.xtattrNamed(L"StripWS").bValueAs();

    //
    //  Get the from/to extraction values and parse them if they were provided. If
    //  not we set them to start/end to get the whole value.
    //
    TString strFrom;
    m_eFrom = EExFroms::Start;
    if (xtnodeSrc.bAttrExists(L"ExtractFrom", strFrom))
    {
        if (strFrom.bCompareI(L"Start"))
        {
            m_eFrom = EExFroms::Start;
        }
        else if (strFrom.bStartsWithI(L"Offset="))
        {
            m_eFrom = EExFroms::Offset;
            strFrom.Cut(0, 7);
            if (!strFrom.bToCard4(m_c4FromOfs, tCIDLib::ERadices::Auto))
                AddErr(colErrs, strFldName, L"ExtractFrom offset is not an unsigned number");
        }
        else if (strFrom.bStartsWithI(L"Char="))
        {
            m_eFrom = EExFroms::Char;
            strFrom.Cut(0, 5);
            if (strFrom.c4Length() == 1)
                m_chFrom = strFrom[0];
            else
                AddErr(colErrs, strFldName, L"ExtractFrom must be a single character");
        }
        else
        {
            AddErr(colErrs, strFldName, L"Unknown ExtractFrom value");
        }
    }

    TString strTo;
    m_eTo = EExTos::End;
    if (xtnodeSrc.bAttrExists(L"ExtractTo", strTo))
    {
        if (strTo.bCompareI(L"End"))
        {
            m_eTo = EExTos::End;
        }
        else if (strTo.bStartsWithI(L"Count="))
        {
            m_eTo = EExTos::Count;
            strTo.Cut(0, 6);
            if (!strTo.bToCard4(m_c4ToCount, tCIDLib::ERadices::Auto))
                AddErr(colErrs, strFldName, L"ExtractTo count is not an unsigned number");
        }
        else if (strTo.bStartsWithI(L"Offset="))
        {
            m_eTo = EExTos::Offset;
            strTo.Cut(0, 7);
            if (!strTo.bToCard4(m_c4ToOfs, tCIDLib::ERadices::Auto))
                AddErr(colErrs, strFldName, L"ExtractTo offset is not an unsigned number");
        }
        else if (strTo.bStartsWithI(L"Char="))
        {
            m_eTo = EExTos::Char;
            strTo.Cut(0, 5);
            if (strTo.c4Length() == 1)
                m_chTo = strTo[0];
            else
                AddErr(colErrs, strFldName, L"ExtractTo must be a single character");
        }
        else
        {
            AddErr(colErrs, strFldName, L"Unknown ExtractTo value");
        }
    }

    // If no new errors then we passed
    return (colErrs.c4ElemCount() == c4ErrCnt);
}



// ---------------------------------------------------------------------------
//  TMQTTComplexMap : Hidden constructors and operators
// ---------------------------------------------------------------------------
TMQTTComplexMap::TMQTTComplexMap() :

    m_bStripWS(kCIDLib::True)
    , m_c4FromOfs(0)
    , m_chFrom(kCIDLib::chSpace)
    , m_chTo(kCIDLib::chSpace)
    , m_c4ToCount(kCIDLib::c4MaxCard)
    , m_c4ToOfs(kCIDLib::c4MaxCard)
    , m_eFrom(EExFroms::Start)
    , m_eTo(EExTos::End)
{
}


// ---------------------------------------------------------------------------
//  TMQTTComplexMap : Protected, non-virtual methods
// ---------------------------------------------------------------------------
tMQTTSh::EInMapRes TMQTTComplexMap::eProcessValue(TString& strToProcess) const
{
    // Get the leading part of the value trimmed if needed
    switch(m_eFrom)
    {
        case EExFroms::Start :
            // Do nothing
            break;

        case EExFroms::Char :
            // But up to the first matching char
            strToProcess.CutUpTo(m_chFrom);
            break;

        case EExFroms::Offset :
            // Cut up to the offset. If it's beyond the end, we failed
            if (m_c4FromOfs >= strToProcess.c4Length())
                return tMQTTSh::EInMapRes::NotFound;

            // This guy takes a length, not an offset
            strToProcess.Cut(0, m_c4FromOfs + 1);
            break;

        default :
            CIDAssert2(L"Unknown multi-line map start extraction type");
            break;
    };

    // And trim the trailing bit if needed
    const tCIDLib::TCard4 c4EndLen = strToProcess.c4Length();
    switch(m_eTo)
    {
        case EExTos::End :
            // Do nothing
            break;

        case EExTos::Char :
        {
            // Move to the next instance of this char
            strToProcess.bCapAtChar(m_chTo);
            break;
        }

        case EExTos::Count :
            // Cap it at this count, unless it's beyond the end
            if ((m_c4ToCount >= c4EndLen) &&  (m_c4ToCount != kCIDLib::c4MaxCard))
                return tMQTTSh::EInMapRes::NotFound;
            strToProcess.CapAt(m_c4ToCount);
            break;

        case EExTos::Offset :
            //
            //  Cap it this many characters from the end, unless the string is
            //  shorter than this.
            //
            if (strToProcess.c4Length() < m_c4ToOfs)
                return tMQTTSh::EInMapRes::NotFound;

            strToProcess.CapAt(c4EndLen - m_c4ToOfs);
            break;

        default :
            CIDAssert2(L"Unknown multi-line map end extraction type");
            return tMQTTSh::EInMapRes::NotFound;
    };

    // If asked to, strip leading/trailing whitespace
    if (m_bStripWS)
        strToProcess.StripWhitespace();

    return tMQTTSh::EInMapRes::GoodVal;
}



// ---------------------------------------------------------------------------
//   CLASS: TMQTTBoolGenMap
//  PREFIX: mqvmp
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TMQTTBoolGenMap: Constructors and destructor
// ---------------------------------------------------------------------------
TMQTTBoolGenMap::TMQTTBoolGenMap() :

    m_strOutFalse(L"false")
    , m_strOutTrue(L"true")
{
}

TMQTTBoolGenMap::~TMQTTBoolGenMap()
{
}


// ---------------------------------------------------------------------------
// TMQTTBoolGenMap:  Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TMQTTBoolGenMap::bIsValidFor(const  tCQCKit::EFldTypes      eType
                            , const tCQCKit::EFldSTypes     eSemType
                            , const TMQTTPLFmt&             mqplfVals) const
{
    // This one is just for boolean fields
    return (eType == tCQCKit::EFldTypes::Boolean);
}


tCIDLib::TBoolean
TMQTTBoolGenMap::bMapOutVal(const   tCQCKit::EFldTypes  eType
                            , const tCQCKit::EFldSTypes eSemType
                            , const TString&            strFldVal
                            ,        TString&           strMQTTVal
                            , const TMQTTPLFmt&         mqplfVals) const
{
    tCIDLib::TBoolean bVal;
    if (!facCQCKit().bCheckBoolVal(strFldVal, bVal))
        return kCIDLib::False;

    if (bVal)
        strMQTTVal = m_strOutTrue;
    else
        strMQTTVal = m_strOutFalse;

    return kCIDLib::True;
}


    // We just need to get the true/false outgoing values
tCIDLib::TBoolean
TMQTTBoolGenMap::bParseFrom(const   TString&                strFldName
                            , const tCQCKit::EFldAccess     eAccess
                            , const TXMLTreeElement&        xtnodeSrc
                            ,       tCIDLib::TStrCollect&   colErrs)
{
    const tCIDLib::TCard4 c4ErrCnt = colErrs.c4ElemCount();

    const tCIDLib::TBoolean bGotFOut = xtnodeSrc.bAttrExists(L"FalseOut", m_strOutFalse);
    if (!bGotFOut)
        m_strOutFalse.Clear();

    const tCIDLib::TBoolean bGotTOut = xtnodeSrc.bAttrExists(L"TrueOut", m_strOutTrue);
    if (!bGotTOut)
        m_strOutTrue.Clear();

    // If writeable we should have defined output values
    if ((!bGotFOut || !bGotTOut)
    &&  tCIDLib::bAllBitsOn(eAccess, tCQCKit::EFldAccess::Write))
    {
        AddErr
        (
            colErrs
            , strFldName
            , L"The field is writeable but all output values were not set"
        );
    }

    // If no new errors, we didn't fail
    return (colErrs.c4ElemCount() == c4ErrCnt);
}

tMQTTSh::EInMapRes
TMQTTBoolGenMap::eMapInVal(const    tCQCKit::EFldTypes  eType
                            , const tCQCKit::EFldSTypes eSemType
                            , const TString&            strMQTTVal
                            ,       TString&            strFldVal
                            , const TMQTTPLFmt&         mqplfVals) const
{
    //
    //  If the payload is binary, convert to a number, else compare as text.
    //
    if ((mqplfVals.m_eType == tMQTTSh::EPLTypes::BinText)
    ||  (mqplfVals.m_eType == tMQTTSh::EPLTypes::Text))
    {
        if ((strMQTTVal == L"0")
        ||  strMQTTVal.bCompareI(facCQCKit().strBoolVal(kCIDLib::False))
        ||  strMQTTVal.bCompareI(facCQCKit().strBoolOffOn(kCIDLib::False))
        ||  strMQTTVal.bCompareI(facCQCKit().strBoolYesNo(kCIDLib::False)))
        {
            strFldVal = facCQCKit().strBoolVal(kCIDLib::False);
        }
         else if ((strMQTTVal == L"1")
              ||  strMQTTVal.bCompareI(facCQCKit().strBoolVal(kCIDLib::True))
              ||  strMQTTVal.bCompareI(facCQCKit().strBoolOffOn(kCIDLib::True))
              ||  strMQTTVal.bCompareI(facCQCKit().strBoolYesNo(kCIDLib::True)))
        {
            strFldVal = facCQCKit().strBoolVal(kCIDLib::True);
        }
        else
        {
            return tMQTTSh::EInMapRes::BadVal;
        }
    }
     else
    {
        tCIDLib::TInt8 i8Val;
        if (!strMQTTVal.bToInt8(i8Val, tCIDLib::ERadices::Auto))
            return tMQTTSh::EInMapRes::BadVal;
        strFldVal = facCQCKit().strBoolVal(i8Val != 0);
    }
    return tMQTTSh::EInMapRes::GoodVal;
}



// ---------------------------------------------------------------------------
//   CLASS: TMQTTBoolNumMap
//  PREFIX: mqvmp
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TMQTTBoolNumMap: Constructors and destructor
// ---------------------------------------------------------------------------
TMQTTBoolNumMap::TMQTTBoolNumMap() :

    m_colMapIn(4)
    , m_i8FalseVal(0)
    , m_i8TrueVal(1)
{
}

TMQTTBoolNumMap::~TMQTTBoolNumMap()
{
}


// ---------------------------------------------------------------------------
// TMQTTBoolNumMap:  Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TMQTTBoolNumMap::bIsValidFor(const  tCQCKit::EFldTypes      eType
                            , const tCQCKit::EFldSTypes     eSemType
                            , const TMQTTPLFmt&             mqplfVals) const
{
    // This one is just for boolean fields and non-floating point numeric MQTT payloads
    return
    (
        (eType == tCQCKit::EFldTypes::Boolean) && mqplfVals.bIsNFNumericPL()
    );
}


tCIDLib::TBoolean
TMQTTBoolNumMap::bMapOutVal(const   tCQCKit::EFldTypes  eType
                            , const tCQCKit::EFldSTypes eSemType
                            , const TString&            strFldVal
                            ,        TString&           strMQTTVal
                            , const TMQTTPLFmt&         mqplfVals) const
{
    // The value has to be true or false
    tCIDLib::TBoolean bSrcVal;
    if (!facCQCKit().bCheckBoolVal(strFldVal, bSrcVal))
        return kCIDLib::False;

    tCIDLib::TInt8 i8OutVal = bSrcVal ? m_i8TrueVal : m_i8FalseVal;
    if (mqplfVals.m_eType == tMQTTSh::EPLTypes::Card)
    {
        strMQTTVal.SetFormatted(tCIDLib::TCard4(i8OutVal));
    }
     else if (mqplfVals.m_eType == tMQTTSh::EPLTypes::Int)
    {
        strMQTTVal.SetFormatted(tCIDLib::TInt4(i8OutVal));
    }
     else
    {
        CIDAssert2(L"Unknown MQTT payload data type");
    }

    return kCIDLib::True;
}


// We look through all of the children, and put each into both direction maps
tCIDLib::TBoolean
TMQTTBoolNumMap::bParseFrom(const   TString&                strFldName
                            , const tCQCKit::EFldAccess     eAccess
                            , const TXMLTreeElement&        xtnodeSrc
                            ,       tCIDLib::TStrCollect&   colErrs)
{
    const tCIDLib::TCard4 c4ErrCnt = colErrs.c4ElemCount();

    //  Get the main output values. If not set, set them to 0 and 1.
    TString strVal;
    m_i8FalseVal = 0;
    m_i8TrueVal = 1;

    const tCIDLib::TBoolean bGotFOut = xtnodeSrc.bAttrExists(L"FalseOut", strVal);
    if (bGotFOut)
    {
        if (!strVal.bToInt8(m_i8FalseVal, tCIDLib::ERadices::Auto))
            AddErr(colErrs, strFldName, L"Invalid out False value");
    }

    const tCIDLib::TBoolean bGotTOut = xtnodeSrc.bAttrExists(L"TrueOut", strVal);
    if (bGotTOut)
    {
        if (!strVal.bToInt8(m_i8TrueVal, tCIDLib::ERadices::Auto))
            AddErr(colErrs, strFldName, L"Invalid out True value");
    }

    // If we got no out values but it's writeable, that's bad
    if ((!bGotFOut || !bGotTOut)
    &&  tCIDLib::bAllBitsOn(eAccess, tCQCKit::EFldAccess::Write))
    {
        AddErr
        (
            colErrs
            , strFldName
            , L"The field is writeable but all output values were not set"
        );
    }

    const tCIDLib::TCard4 c4Count = xtnodeSrc.c4ChildCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TXMLTreeElement& xtnodeCur = xtnodeSrc.xtnodeChildAtAsElement(c4Index);
        const tCIDLib::TBoolean bTarVal = xtnodeCur.xtattrNamed(L"TarVal").bValueAs();

        // Get the min/max values for this slot
        tCIDLib::TInt8 i8Min, i8Max;
        const TString& strMin = xtnodeCur.xtattrNamed(L"MinSrc").strValue();
        const TString& strMax = xtnodeCur.xtattrNamed(L"MaxSrc").strValue();

        if (!strMin.bToInt8(i8Min, tCIDLib::ERadices::Auto)
        ||  !strMax.bToInt8(i8Max, tCIDLib::ERadices::Auto))
        {
            AddErr(colErrs, strFldName, L"Invalid min or max range value");
        }
         else
        {
            m_colMapIn.objAdd(TARange(bTarVal, i8Min, i8Max));
        }
    }

    // If no new errors then we passed
    return (colErrs.c4ElemCount() == c4ErrCnt);
}


tMQTTSh::EInMapRes
TMQTTBoolNumMap::eMapInVal( const   tCQCKit::EFldTypes  eType
                            , const tCQCKit::EFldSTypes eSemType
                            , const TString&            strMQTTVal
                            ,       TString&            strFldVal
                            , const TMQTTPLFmt&         mqplfVals) const
{
    // The incoming value number be a number
    tCIDLib::TInt8 i8Val;
    if (!strMQTTVal.bToInt8(i8Val))
        return tMQTTSh::EInMapRes::BadVal;

    // Look through the list of ranges and find the first one that this is within
    const tCIDLib::TCard4 c4Count = m_colMapIn.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TARange& arCur = m_colMapIn[c4Index];
        if ((i8Val >= arCur.m_i8MinVal) && (i8Val <= arCur.m_i8MaxVal))
        {
            strFldVal = facCQCKit().strBoolVal(arCur.m_bTarVal);
            return tMQTTSh::EInMapRes::GoodVal;
        }
    }
    return tMQTTSh::EInMapRes::BadVal;
}


tCIDLib::TVoid
TMQTTBoolNumMap::Reset( const   tCIDLib::TInt8  i8FalseVal
                        , const tCIDLib::TInt8  i8TrueVal)
{
    m_colMapIn.RemoveAll();
    m_i8FalseVal = i8FalseVal;
    m_i8TrueVal = i8TrueVal;
}



// ---------------------------------------------------------------------------
//   CLASS: TMQTTBoolTextMap
//  PREFIX: mqvmp
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TMQTTBoolTextMap: Constructors and destructor
// ---------------------------------------------------------------------------
TMQTTBoolTextMap::TMQTTBoolTextMap() :

    m_colInFalseMap(23, TStringKeyOps(kCIDLib::False))
    , m_colInTrueMap(23, TStringKeyOps(kCIDLib::False))
    , m_strOutFalse(L"false")
    , m_strOutTrue(L"true")
{
}

TMQTTBoolTextMap::~TMQTTBoolTextMap()
{
}


// ---------------------------------------------------------------------------
// TMQTTBoolTextMap:  Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TMQTTBoolTextMap::bIsValidFor(const tCQCKit::EFldTypes      eType
                            , const tCQCKit::EFldSTypes     eSemType
                            , const TMQTTPLFmt&             mqplfVals) const
{
    // This one is just for boolean fields and text payloads
    return
    (
        (eType == tCQCKit::EFldTypes::Boolean)
        && ((mqplfVals.m_eType == tMQTTSh::EPLTypes::BinText)
        ||  (mqplfVals.m_eType == tMQTTSh::EPLTypes::Text))
    );
}


tCIDLib::TBoolean
TMQTTBoolTextMap::bMapOutVal(const   tCQCKit::EFldTypes  eType
                            , const tCQCKit::EFldSTypes eSemType
                            , const TString&            strFldVal
                            ,        TString&           strMQTTVal
                            , const TMQTTPLFmt&         mqplfVals) const
{
    // Make sure it's a valid boolean text value
    tCIDLib::TBoolean bVal;
    if (!facCQCKit().bCheckBoolVal(strFldVal, bVal))
        return kCIDLib::False;

    // It is, so just return the configured output value for that state
    if (bVal)
        strMQTTVal = m_strOutTrue;
    else
        strMQTTVal = m_strOutFalse;

    return kCIDLib::True;
}



tCIDLib::TBoolean
TMQTTBoolTextMap::bParseFrom(const  TString&                strFldName
                            , const tCQCKit::EFldAccess     eAccess
                            , const TXMLTreeElement&        xtnodeSrc
                            ,       tCIDLib::TStrCollect&   colErrs)
{
    const tCIDLib::TCard4 c4ErrCnt = colErrs.c4ElemCount();

    const tCIDLib::TBoolean bGotFOut = xtnodeSrc.bAttrExists(L"FalseOut", m_strOutFalse);
    if (!bGotFOut)
        m_strOutFalse.Clear();

    const tCIDLib::TBoolean bGotTOut = xtnodeSrc.bAttrExists(L"TrueOut", m_strOutTrue);
    if (!bGotTOut)
        m_strOutTrue.Clear();

    if ((!bGotFOut || !bGotTOut)
    &&  tCIDLib::bAllBitsOn(eAccess, tCQCKit::EFldAccess::Write))
    {
        AddErr
        (
            colErrs
            , strFldName
            , L"The field is writeable but all output values were not set"
        );
    }

    // Load up the incoming map values, each to their respective collections
    const tCIDLib::TCard4 c4Count = xtnodeSrc.c4ChildCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TXMLTreeElement& xtnodeCur = xtnodeSrc.xtnodeChildAtAsElement(c4Index);
        const TString& strFldVal = xtnodeCur.xtattrNamed(L"TarVal").strValue();
        const TString& strMQTTVal = xtnodeCur.xtattrNamed(L"MQTTVal").strValue();
        if (strFldVal == L"True")
            m_colInTrueMap.objAdd(strMQTTVal);
        else if (strFldVal == L"False")
            m_colInFalseMap.objAdd(strMQTTVal);
    }

    // If no new errors, we didnt' fail
    return (colErrs.c4ElemCount() == c4ErrCnt);
}


tMQTTSh::EInMapRes
TMQTTBoolTextMap::eMapInVal(const   tCQCKit::EFldTypes  eType
                            , const tCQCKit::EFldSTypes eSemType
                            , const TString&            strMQTTVal
                            ,       TString&            strFldVal
                            , const TMQTTPLFmt&         mqplfVals) const
{
    if (m_colInFalseMap.bHasElement(strMQTTVal))
        strFldVal = facCQCKit().strBoolVal(kCIDLib::False);
    else if (m_colInTrueMap.bHasElement(strMQTTVal))
        strFldVal = facCQCKit().strBoolVal(kCIDLib::True);
    else
        return tMQTTSh::EInMapRes::BadVal;

    return tMQTTSh::EInMapRes::GoodVal;
}


tCIDLib::TVoid
TMQTTBoolTextMap::Reset(const TString& strFalseOut, const TString& strTrueOut)
{
    m_colInFalseMap.RemoveAll();
    m_colInTrueMap.RemoveAll();
    m_strOutFalse = strFalseOut;
    m_strOutTrue = strTrueOut;
}




// ---------------------------------------------------------------------------
//   CLASS: TMQTTEnumMap
//  PREFIX: mqvmp
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TMQTTEnumMap: Constructors and destructor
// ---------------------------------------------------------------------------
TMQTTEnumMap::TMQTTEnumMap() :

    m_colMapIn(23, TStringKeyOps(kCIDLib::False), TKeyValuePair::strExtractKey)
    , m_colMapOut(23, TStringKeyOps(kCIDLib::False), TKeyValuePair::strExtractKey)
{
}

TMQTTEnumMap::~TMQTTEnumMap()
{
}


// ---------------------------------------------------------------------------
// TMQTTEnumMap:  Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TMQTTEnumMap::bIsValidFor(  const   tCQCKit::EFldTypes      eType
                            , const tCQCKit::EFldSTypes     eSemType
                            , const TMQTTPLFmt&             mqplfVals) const
{
    //
    //  This one is really just for string fields. The payload can be numbers but
    //  we just look at the incoming values as text.
    //
    return (eType == tCQCKit::EFldTypes::String);
}


tCIDLib::TBoolean
TMQTTEnumMap::bMapOutVal(const  tCQCKit::EFldTypes  eType
                        , const tCQCKit::EFldSTypes eSemType
                        , const TString&            strFldVal
                        ,        TString&           strMQTTVal
                        , const TMQTTPLFmt&         mqplfVals) const
{
    const TKeyValuePair* pkvalMap = m_colMapOut.pobjFindByKey(strFldVal);
    if (!pkvalMap)
        return kCIDLib::False;
    strMQTTVal = pkvalMap->strValue();
    return kCIDLib::True;
}


// We look through all of the children, and put each into both direction maps
tCIDLib::TBoolean
TMQTTEnumMap::bParseFrom(const  TString&                strFldName
                        , const tCQCKit::EFldAccess
                        , const TXMLTreeElement&        xtnodeSrc
                        ,       tCIDLib::TStrCollect&   colErrs)
{
    const tCIDLib::TCard4 c4ErrCnt = colErrs.c4ElemCount();

    const tCIDLib::TCard4 c4Count = xtnodeSrc.c4ChildCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TXMLTreeElement& xtnodeCur = xtnodeSrc.xtnodeChildAtAsElement(c4Index);
        const TString& strFldVal = xtnodeCur.xtattrNamed(L"FldVal").strValue();
        const TString& strMQTTVal = xtnodeCur.xtattrNamed(L"MQTTVal").strValue();

        // Make sure no dups on the field side
        if (m_colMapOut.bKeyExists(strFldVal))
        {
            AddErr(colErrs, strFldName, L"No field value duplicates are allowed here");
        }
        else
        {
            // Add it to both maps
            m_colMapIn.objAdd(TKeyValuePair(strMQTTVal, strFldVal));
            m_colMapOut.objAdd(TKeyValuePair(strFldVal, strMQTTVal));
        }
    }

    // If no new errors then we passed
    return (colErrs.c4ElemCount() == c4ErrCnt);
}

tMQTTSh::EInMapRes
TMQTTEnumMap::eMapInVal(const   tCQCKit::EFldTypes  eType
                        , const tCQCKit::EFldSTypes eSemType
                        , const TString&            strMQTTVal
                        ,       TString&            strFldVal
                        , const TMQTTPLFmt&         mqplfVals) const
{
    const TKeyValuePair* pkvalMap = m_colMapIn.pobjFindByKey(strMQTTVal);
    if (!pkvalMap)
        return tMQTTSh::EInMapRes::BadVal;
    strFldVal = pkvalMap->strValue();
    return tMQTTSh::EInMapRes::GoodVal;
}



// ---------------------------------------------------------------------------
//   CLASS: TMQTTJSONMap
//  PREFIX: mqvmp
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TMQTTJSONMap: Constructors and destructor
// ---------------------------------------------------------------------------
TMQTTJSONMap::TMQTTJSONMap() :

    m_bIgnoreNotFnd(kCIDLib::False)
    , m_pmqvmpValue(nullptr)
{
}

TMQTTJSONMap::~TMQTTJSONMap()
{
    try
    {
        delete m_pmqvmpValue;
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);
    }
}


// ---------------------------------------------------------------------------
// TMQTTJSONMap:  Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TMQTTJSONMap::bIsValidFor(  const   tCQCKit::EFldTypes      eType
                            , const tCQCKit::EFldSTypes     eSemType
                            , const TMQTTPLFmt&             mqplfVals) const
{
    // This one is ok for a text payload and whatever the nested map will accept
    return
    (
        m_pmqvmpValue->bIsValidFor(eType, eSemType, mqplfVals)
        && ((mqplfVals.m_eType == tMQTTSh::EPLTypes::BinText)
        ||  (mqplfVals.m_eType == tMQTTSh::EPLTypes::Text))
    );
}


// Map the field value to an MQTT value using our nested map
tCIDLib::TBoolean
TMQTTJSONMap::bMapOutVal(const  tCQCKit::EFldTypes  eType
                        , const tCQCKit::EFldSTypes eSemType
                        , const TString&            strFldVal
                        ,        TString&           strMQTTVal
                        , const TMQTTPLFmt&         mqplfVals) const
{
    return m_pmqvmpValue->bMapOutVal(eType, eSemType, strFldVal, strMQTTVal, mqplfVals);
}



// Embed the MQTT value into the larger output value
tCIDLib::TBoolean
TMQTTJSONMap::bPostMapOutVal(const TString& strMQTTVal, TString& strOutVal) const
{
    strOutVal = m_strOutJSON;
    strOutVal.eReplaceToken(strMQTTVal, kCIDLib::chLatin_v);
    return kCIDLib::True;
}



tCIDLib::TBoolean
TMQTTJSONMap::bParseFrom(const  TString&                strFldName
                        , const tCQCKit::EFldAccess     eAccess
                        , const TXMLTreeElement&        xtnodeSrc
                        ,       tCIDLib::TStrCollect&   colErrs)
{
    // Call our parent first and fail if he fails
    if (!TParent::bParseFrom(strFldName, eAccess, xtnodeSrc, colErrs))
        return kCIDLib::False;

    const tCIDLib::TCard4 c4ErrCnt = colErrs.c4ElemCount();

    // Get the incoming path
    if (!xtnodeSrc.bAttrExists(L"InPath", m_strInPath))
        AddErr(colErrs, strFldName, L"JSONMap requires an InPath attribute");

    // This one has to be Yes or No as per the DTD and is defaulted
    m_bIgnoreNotFnd = xtnodeSrc.xtattrNamed(L"IgnoreNotFnd").bValueAs();

    //
    //  Just iterate the nodes and process what we find. The DTD insured any present are
    //  in the right order, so we don't have to enforce that.
    //
    m_strOutJSON.Clear();
    delete m_pmqvmpValue;
    m_pmqvmpValue = nullptr;
    const tCIDLib::TCard4 c4ChildCnt = xtnodeSrc.c4ChildCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ChildCnt; c4Index++)
    {
        const TXMLTreeElement& xtnodeCur = xtnodeSrc.xtnodeChildAtAsElement(c4Index);
        if (xtnodeCur.strQName() == L"JSONOut")
        {
            const TXMLTreeText& xtnodeText = xtnodeCur.xtnodeChildAtAsText(0);
            m_strOutJSON = xtnodeText.strText();
            m_strOutJSON.StripWhitespace();

            // Make sure there is a token in it
            if (m_strOutJSON.eTokenExists(kCIDLib::chLatin_v) == tCIDLib::EFindRes::NotFound)
                AddErr(colErrs, strFldName, L"The output JSON does not have a %(v) token");
        }
         else
        {
            // Has to be a map
            m_pmqvmpValue = facMQTTSh().pmqvmpMakeNew(xtnodeCur.strQName());
            if (m_pmqvmpValue)
                m_pmqvmpValue->bParseFrom(strFldName, eAccess, xtnodeCur, colErrs);
            else
                AddErr(colErrs, strFldName, L"Unknown JSON nested value map type");
        }
    }

    if (tCIDLib::bAllBitsOn(eAccess, tCQCKit::EFldAccess::Write) && m_strOutJSON.bIsEmpty())
    {
        AddErr(colErrs, strFldName, L"JSONMap needs a JSONOut value if the field is writable");
        return kCIDLib::False;
    }

    // If no map, make it a default passthrough
    if (!m_pmqvmpValue)
        m_pmqvmpValue = new TMQTTPassthroughMap();

    // If no new errors then we passed
    return (colErrs.c4ElemCount() == c4ErrCnt);
}


// Map the MQTT value to a field value, using our nested map
tMQTTSh::EInMapRes
TMQTTJSONMap::eMapInVal(const   tCQCKit::EFldTypes  eType
                        , const tCQCKit::EFldSTypes eSemType
                        , const TString&            strMQTTVal
                        ,       TString&            strFldVal
                        , const TMQTTPLFmt&         mqplfVals) const
{
    return m_pmqvmpValue->eMapInVal(eType, eSemType, strMQTTVal, strFldVal, mqplfVals);
}

// WE parse the JSON and extract the configured named JSON value to get the MQTT value
tMQTTSh::EInMapRes
TMQTTJSONMap::ePreMapInVal(const TString& strInVal, TString& strMQTTVal) const
{
    // Parse the incoming JSON. If that fails, obviously bad
    tCIDLib::TBoolean bFound = kCIDLib::False;
    {
        // Parse it and see if we find the passed path
        TTextStringInStream strmSrc(&strInVal);
        try
        {
            m_jprsIn.bParse
            (
                strmSrc
                , [&strMQTTVal, &bFound, this]( const   TString&                strPath
                                                , const TString&                strVal
                                                , const tCIDNet::EJSONVTypes    eType)
                  {
                    // If not our path, then obviously not
                    if (!strPath.bCompareI(m_strInPath))
                        return kCIDLib::False;

                    if ((eType == tCIDNet::EJSONVTypes::String)
                    ||  (eType == tCIDNet::EJSONVTypes::Number))
                    {
                        bFound = kCIDLib::True;
                        strMQTTVal = strVal;
                    }
                    else if (eType == tCIDNet::EJSONVTypes::True)
                    {
                        bFound = kCIDLib::True;
                        strMQTTVal = facCQCKit().strBoolVal(kCIDLib::True);
                    }
                    else if (eType == tCIDNet::EJSONVTypes::False)
                    {
                        bFound = kCIDLib::True;
                        strMQTTVal = facCQCKit().strBoolVal(kCIDLib::False);
                    }

                    // Keep going if not found
                    return !bFound;
                  }
            );
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
            return tMQTTSh::EInMapRes::NotFound;
        }
    }

    if (!bFound)
    {
        // Ignore if told to, else a not found error
        if (m_bIgnoreNotFnd)
            return tMQTTSh::EInMapRes::Ignore;
        return tMQTTSh::EInMapRes::NotFound;
    }

    // Call our parent to process this value and return his result
    return eProcessValue(strMQTTVal);
}



// ---------------------------------------------------------------------------
//   CLASS: TMQTTMLineMap
//  PREFIX: mqvmp
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TMQTTMLineMap: Constructors and destructor
// ---------------------------------------------------------------------------
TMQTTMLineMap::TMQTTMLineMap() :

    m_c4LineNum(1)
    , m_pmqvmpValue(nullptr)
{
}

TMQTTMLineMap::~TMQTTMLineMap()
{
    try
    {
        delete m_pmqvmpValue;
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);
    }
}


// ---------------------------------------------------------------------------
// TMQTTMLineMap:  Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TMQTTMLineMap::bIsValidFor( const   tCQCKit::EFldTypes      eType
                            , const tCQCKit::EFldSTypes     eSemType
                            , const TMQTTPLFmt&             mqplfVals) const
{
    // This one is ok for a text payload and whatever the nested map will accept
    return
    (
        m_pmqvmpValue->bIsValidFor(eType, eSemType, mqplfVals)
        && ((mqplfVals.m_eType == tMQTTSh::EPLTypes::BinText)
        ||  (mqplfVals.m_eType == tMQTTSh::EPLTypes::Text))
    );
}


// Use the nested map to map the fld value to an MQTT value
tCIDLib::TBoolean
TMQTTMLineMap::bMapOutVal(const tCQCKit::EFldTypes  eType
                        , const tCQCKit::EFldSTypes eSemType
                        , const TString&            strFldVal
                        ,        TString&           strMQTTVal
                        , const TMQTTPLFmt&         mqplfVals) const
{
    return m_pmqvmpValue->bMapOutVal(eType, eSemType, strFldVal, strMQTTVal, mqplfVals);
}


// Embed the MQTT value into the larger output value
tCIDLib::TBoolean
TMQTTMLineMap::bPostMapOutVal(const TString& strMQTTVal, TString& strOutVal)   const
{
    // Format it into the outgoing text
    strOutVal = m_strOutText;
    strOutVal.eReplaceToken(strMQTTVal, kCIDLib::chLatin_v);
    return kCIDLib::True;
}


tCIDLib::TBoolean
TMQTTMLineMap::bParseFrom(const TString&                strFldName
                        , const tCQCKit::EFldAccess     eAccess
                        , const TXMLTreeElement&        xtnodeSrc
                        ,       tCIDLib::TStrCollect&   colErrs)
{
    // Call our parent first and fail if he fails
    if (!TParent::bParseFrom(strFldName, eAccess, xtnodeSrc, colErrs))
        return kCIDLib::False;

    const tCIDLib::TCard4 c4ErrCnt = colErrs.c4ElemCount();

    if (!xtnodeSrc.xtattrNamed(L"LineNum").strValue().bToCard4(m_c4LineNum, tCIDLib::ERadices::Auto))
        AddErr(colErrs, strFldName, L"MLineMap LineNum is not an unsigned number");

    // Iterate the children and process the ones we find, both are optional
    m_strOutText.Clear();
    delete m_pmqvmpValue;
    m_pmqvmpValue = nullptr;
    const tCIDLib::TCard4 c4ChildCnt = xtnodeSrc.c4ChildCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ChildCnt; c4Index++)
    {
        const TXMLTreeElement& xtnodeCur = xtnodeSrc.xtnodeChildAtAsElement(c4Index);
        if (xtnodeCur.strQName() == L"MLineOut")
        {
            const TXMLTreeText& xtnodeText = xtnodeCur.xtnodeChildAtAsText(0);
            m_strOutText = xtnodeText.strText();
            m_strOutText.StripWhitespace();

            // Make sure there is a token in it
            if (m_strOutText.eTokenExists(kCIDLib::chLatin_v) == tCIDLib::EFindRes::NotFound)
                AddErr(colErrs, strFldName, L"The multi-line output does not have a %(v) token");
        }
         else
        {
            // Has to be a map
            m_pmqvmpValue = facMQTTSh().pmqvmpMakeNew(xtnodeCur.strQName());
            if (m_pmqvmpValue)
                m_pmqvmpValue->bParseFrom(strFldName, eAccess, xtnodeCur, colErrs);
            else
                AddErr(colErrs, strFldName, L"Unknown MLine nested value map type");
        }
    }

    // If no map, make it a default passthrough
    if (!m_pmqvmpValue)
        m_pmqvmpValue = new TMQTTPassthroughMap();

    if (tCIDLib::bAllBitsOn(eAccess, tCQCKit::EFldAccess::Write) && m_strOutText.bIsEmpty())
    {
        AddErr(colErrs, strFldName, L"MLineMap needs an MLineOut value if the field is writable");
        return kCIDLib::False;
    }

    // If no new errors then we passed
    return (colErrs.c4ElemCount() == c4ErrCnt);
}


// Use our nested map to map the extracted MQTT value to a field value
tMQTTSh::EInMapRes
TMQTTMLineMap::eMapInVal(const   tCQCKit::EFldTypes  eType
                        , const tCQCKit::EFldSTypes eSemType
                        , const TString&            strMQTTVal
                        ,       TString&            strFldVal
                        , const TMQTTPLFmt&         mqplfVals) const
{

    return m_pmqvmpValue->eMapInVal(eType, eSemType, strMQTTVal, strFldVal, mqplfVals);
}


// Extract the MQTT value from the raw input value
tMQTTSh::EInMapRes
TMQTTMLineMap::ePreMapInVal(const TString& strInVal, TString& strMQTTVal) const
{
    // Put a stream over it and read down to the configured line
    TTextStringInStream strmSrc(&strInVal);
    tCIDLib::TCard4 c4Line = 0;
    while (c4Line < m_c4LineNum)
    {
        strmSrc >> strMQTTVal;
        c4Line++;
    }

    // If we never got there, that's bad
    if (c4Line < m_c4LineNum)
        return tMQTTSh::EInMapRes::NotFound;

    // Call our parent to process this value and return his result
    return eProcessValue(strMQTTVal);
}




// ---------------------------------------------------------------------------
//   CLASS: TMQTTPassthroughMap
//  PREFIX: mqvmp
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TMQTTPassthroughMap: Constructors and destructor
// ---------------------------------------------------------------------------
TMQTTPassthroughMap::TMQTTPassthroughMap()
{
}

TMQTTPassthroughMap::~TMQTTPassthroughMap()
{
}


// ---------------------------------------------------------------------------
// TMQTTPassthroughMap:  Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TMQTTPassthroughMap::bIsValidFor(const  tCQCKit::EFldTypes      eType
                                , const tCQCKit::EFldSTypes     eSemType
                                , const TMQTTPLFmt&             mqplfVals) const
{
    // We can theoretically be used for any type
    return kCIDLib::True;
}


tCIDLib::TBoolean
TMQTTPassthroughMap::bMapOutVal(const   tCQCKit::EFldTypes  eType
                                , const tCQCKit::EFldSTypes eSemType
                                , const TString&            strFldVal
                                ,        TString&           strMQTTVal
                                , const TMQTTPLFmt&         mqplfVals) const
{
    strMQTTVal = strFldVal;
    return kCIDLib::True;
}


// Nothing to do. We just pass through values in and out
tCIDLib::TBoolean
TMQTTPassthroughMap::bParseFrom(const   TString&
                                , const tCQCKit::EFldAccess
                                , const TXMLTreeElement&
                                ,       tCIDLib::TStrCollect& )
{
    return kCIDLib::True;
}


tMQTTSh::EInMapRes
TMQTTPassthroughMap::eMapInVal( const   tCQCKit::EFldTypes  eType
                                , const tCQCKit::EFldSTypes eSemType
                                , const TString&            strMQTTVal
                                ,       TString&            strFldVal
                                , const TMQTTPLFmt&         mqplfVals) const
{
    strFldVal = strMQTTVal;
    return tMQTTSh::EInMapRes::GoodVal;
}




// ---------------------------------------------------------------------------
//   CLASS: TMQTTScaleRangeMap
//  PREFIX: mqvmp
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TMQTTScaleRangeMap: Constructors and destructor
// ---------------------------------------------------------------------------
TMQTTScaleRangeMap::TMQTTScaleRangeMap() :

    m_f8FldMax(1)
    , m_f8FldMin(0)
    , m_f8MQTTMax(1)
    , m_f8MQTTMin(0)
{
}

TMQTTScaleRangeMap::~TMQTTScaleRangeMap()
{
}


// ---------------------------------------------------------------------------
// TMQTTScaleRangeMap:  Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TMQTTScaleRangeMap::bIsValidFor(const   tCQCKit::EFldTypes      eType
                                , const tCQCKit::EFldSTypes     eSemType
                                , const TMQTTPLFmt&             mqplfVals) const
{
    // It needs to be some numeric field type
    return
    (
        (eType == tCQCKit::EFldTypes::Card)
        || (eType == tCQCKit::EFldTypes::Int)
        || (eType == tCQCKit::EFldTypes::Float)
    );
}


tCIDLib::TBoolean
TMQTTScaleRangeMap::bMapOutVal( const   tCQCKit::EFldTypes  eType
                                , const tCQCKit::EFldSTypes eSemType
                                , const TString&            strFldVal
                                ,        TString&           strMQTTVal
                                , const TMQTTPLFmt&         mqplfVals) const
{
    // Get the same MQTT percent of the range as the field is of his
    tCIDLib::TFloat8 f8Tmp;
    if (!strFldVal.bToFloat8(f8Tmp))
        return kCIDLib::False;
    f8Tmp -= m_f8FldMin;
    tCIDLib::TFloat8 f8MQTTVal = m_f8MQTTRange * (f8Tmp / m_f8FldRange);

    // Check it against the set range
    if ((f8MQTTVal < m_f8MQTTMin) || (f8MQTTVal > m_f8MQTTMax))
        return kCIDLib::False;

    // And we can format it back out as the appropriate type
    if (eType == tCQCKit::EFldTypes::Card)
        strMQTTVal.SetFormatted(tCIDLib::TCard4(f8MQTTVal));
    else if (eType == tCQCKit::EFldTypes::Int)
        strMQTTVal.SetFormatted(tCIDLib::TCard4(f8MQTTVal));
    else if (eType == tCQCKit::EFldTypes::Float)
        strMQTTVal.SetFormatted(f8MQTTVal, mqplfVals.m_c4DecDigits);
    else
        return kCIDLib::False;

    return kCIDLib::True;
}


//
//  We just have the min/max range value attributes to get out
//
tCIDLib::TBoolean
TMQTTScaleRangeMap::bParseFrom( const   TString&                strFldName
                                , const tCQCKit::EFldAccess     eAccess
                                , const TXMLTreeElement&        xtnodeSrc
                                ,       tCIDLib::TStrCollect&   colErrs)
{
    const tCIDLib::TCard4 c4ErrCnt = colErrs.c4ElemCount();

    if (!xtnodeSrc.xtattrNamed(L"MQMin").strValue().bToFloat8(m_f8MQTTMin)
    ||  !xtnodeSrc.xtattrNamed(L"MQMax").strValue().bToFloat8(m_f8MQTTMax)
    ||  !xtnodeSrc.xtattrNamed(L"FldMin").strValue().bToFloat8(m_f8FldMin)
    ||  !xtnodeSrc.xtattrNamed(L"FldMax").strValue().bToFloat8(m_f8FldMax))
    {
        AddErr(colErrs, strFldName, L"One or more min/max range values is invalid");
    }

    if ((m_f8MQTTMax <= m_f8MQTTMin)
    ||  (m_f8FldMax <= m_f8FldMin))
    {
        AddErr(colErrs, strFldName, L"One or more max range values is <= the min value");
    }

    // Pre-calc the actual ranges
    m_f8FldRange = m_f8FldMax - m_f8FldMin;
    m_f8MQTTRange = m_f8MQTTMax - m_f8MQTTMin;

    // If no new errors then we passed
    return (colErrs.c4ElemCount() == c4ErrCnt);
}


tMQTTSh::EInMapRes
TMQTTScaleRangeMap::eMapInVal(  const   tCQCKit::EFldTypes  eType
                                , const tCQCKit::EFldSTypes eSemType
                                , const TString&            strMQTTVal
                                ,       TString&            strFldVal
                                , const TMQTTPLFmt&         mqplfVals) const
{
    // Calc the percentage of incoming value in the MQTT range and transfer to the fld range
    tCIDLib::TFloat8 f8Tmp;
    if (!strMQTTVal.bToFloat8(f8Tmp))
        return tMQTTSh::EInMapRes::BadVal;
    f8Tmp -= m_f8MQTTMin;
    tCIDLib::TFloat8 f8FldVal = m_f8FldRange * (f8Tmp / m_f8MQTTRange);

    // If an non-float, clip down the integral value it falls into
    if ((eType == tCQCKit::EFldTypes::Card) || (eType == tCQCKit::EFldTypes::Int))
        TMathLib::Round(f8FldVal, tCIDLib::ERoundTypes::Down);

    // If this is outside of the min/max, then bad
    if ((f8FldVal < m_f8FldMin) || (f8FldVal > m_f8FldMax))
        return tMQTTSh::EInMapRes::BadVal;

	//
    //	Else format out as the actual field type. For floats just format out
	//	a reasonable number of decimal digits, more than anyone is likely to
	//	need.
	//
    if (eType == tCQCKit::EFldTypes::Card)
        strFldVal.SetFormatted(tCIDLib::TCard4(f8FldVal));
    else if (eType == tCQCKit::EFldTypes::Int)
        strFldVal.SetFormatted(tCIDLib::TCard4(f8FldVal));
    else if (eType == tCQCKit::EFldTypes::Float)
        strFldVal.SetFormatted(f8FldVal, 5);
    else
        return tMQTTSh::EInMapRes::BadVal;

    return tMQTTSh::EInMapRes::GoodVal;
}



// ---------------------------------------------------------------------------
//   CLASS: TMQTTTextMap
//  PREFIX: mqvmp
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TMQTTTextMap: Constructors and destructor
// ---------------------------------------------------------------------------
TMQTTTextMap::TMQTTTextMap() :

    m_colMapIn(23, TStringKeyOps(kCIDLib::False))
    , m_colMapOut(23, TStringKeyOps(kCIDLib::False))
{
}

TMQTTTextMap::~TMQTTTextMap()
{
}


// ---------------------------------------------------------------------------
// TMQTTTextMap:  Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TMQTTTextMap::bIsValidFor(  const   tCQCKit::EFldTypes      eType
                            , const tCQCKit::EFldSTypes     eSemType
                            , const TMQTTPLFmt&             mqplfVals) const
{
    // We can theoretically be used for any type
    return kCIDLib::True;
}


tCIDLib::TBoolean
TMQTTTextMap::bMapOutVal(const  tCQCKit::EFldTypes  eType
                        , const tCQCKit::EFldSTypes eSemType
                        , const TString&            strFldVal
                        ,        TString&           strMQTTVal
                        , const TMQTTPLFmt&         mqplfVals) const
{
    //
    //  We have to iterate all the map entries and see if if the field vlaue is in one
    //  of their source value lists.
    //
    TValMap::TCursor cursMap = TValMap::TCursor(&m_colMapOut);
    for (; cursMap; ++cursMap)
    {
        if (cursMap->objValue().bHasElement(strFldVal))
        {
            strMQTTVal = cursMap->objKey();
            return kCIDLib::True;
        }
    }
    return kCIDLib::False;
}


//
//  We look through all of the children, each of which and put them into the
//  appropriate map.
//
tCIDLib::TBoolean
TMQTTTextMap::bParseFrom(const  TString&                strFldName
                        , const tCQCKit::EFldAccess     eAccess
                        , const TXMLTreeElement&        xtnodeSrc
                        ,       tCIDLib::TStrCollect&   colErrs)
{
    const tCIDLib::TCard4 c4ErrCnt = colErrs.c4ElemCount();

    tCIDLib::TCard4 c4ErrInd;
    TStringTokenizer stokSrc;
    TValMap::TPair kobjCur(TString::strEmpty(), tCIDLib::TStrHashSet(23, TStringKeyOps(kCIDLib::False)));
    const tCIDLib::TCard4 c4Count = xtnodeSrc.c4ChildCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TXMLTreeElement& xtnodeCur = xtnodeSrc.xtnodeChildAtAsElement(c4Index);
        const TString& strTarVal = xtnodeCur.xtattrNamed(L"TarVal").strValue();

        const tMQTTSh::EMapDirs eDir = tMQTTSh::eXlatEMapDirs(xtnodeCur.xtattrNamed(L"Dir").strValue());
        if (eDir == tMQTTSh::EMapDirs::Count)
        {
            AddErr
            (
                colErrs
                , strFldName
                , L"Text map entry '%(1)' has bad map direction value: %(2)"
                , strTarVal
                , xtnodeCur.xtattrNamed(L"Dir").strValue()
            );
            return kCIDLib::False;
        }

        // Get a ref to the target collection
        TValMap& colTarMap
        (
            ((eDir == tMQTTSh::EMapDirs::In) || (eDir == tMQTTSh::EMapDirs::InOut))
            ? m_colMapIn : m_colMapOut
        );

        // Get the target value and make sure it's not a dup
        if (colTarMap.bKeyExists(strTarVal))
        {
            AddErr
            (
                colErrs
                , strFldName
                , L"Text map entry '%(1)' is a duplicate target value"
                , strTarVal
            );
        }

        // Reset the current pair and load up the info to it
        kobjCur.objValue().RemoveAll();
        kobjCur.objKey(strTarVal);

        // Get the text of the element which is a quoted comma list
        const TXMLTreeText& xtnodeSrcVals = xtnodeCur.xtnodeChildAtAsText(0);
        if (!stokSrc.bParseQuotedCommaList(xtnodeSrcVals.strText(), kobjCur.objValue(), c4ErrInd))
        {
            AddErr
            (
                colErrs
                , strFldName
                , L"Text map entry '%(1)' has badly formatted source values"
                , strTarVal
            );
        }

        // Looks reasonable to add it to the target map
        colTarMap.objAdd(kobjCur);
    }

    // If no new errors then we passed
    return (colErrs.c4ElemCount() == c4ErrCnt);
}


tMQTTSh::EInMapRes
TMQTTTextMap::eMapInVal(const   tCQCKit::EFldTypes  eType
                        , const tCQCKit::EFldSTypes eSemType
                        , const TString&            strMQTTVal
                        ,       TString&            strFldVal
                        , const TMQTTPLFmt&         mqplfVals) const
{
    //
    //  We have to iterate all the map entries and see if if the field vlaue is in one
    //  of their source value lists.
    //
    TValMap::TCursor cursMap = TValMap::TCursor(&m_colMapIn);
    for (; cursMap; ++cursMap)
    {
        if (cursMap->objValue().bHasElement(strMQTTVal))
        {
            strFldVal = cursMap->objKey();
            return tMQTTSh::EInMapRes::GoodVal;
        }
    }
    return tMQTTSh::EInMapRes::BadVal;
}

