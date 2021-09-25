//
// FILE NAME: LutronRA2C_Item.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 12/06/2012
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
//  This file implements a simple class that is used to store config data on the
//  various configured items.
//
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "LutronRA2C_.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TRA2CItem,TObject)


// ---------------------------------------------------------------------------
//   CLASS: TRA2CItem
//  PREFIX: item
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TRA2CItem: Public, static methods
// ---------------------------------------------------------------------------

// Parses the passed string out into our ids
tCIDLib::TBoolean
TRA2CItem::bParseAddr(  const   TString&                strToParse
                        ,       tCIDLib::TCard4&        c4IntID
                        ,       tCIDLib::TCard4&        c4CompID
                        , const tLutronRA2C::EItemTypes eType
                        ,       TString&                strErrMsg)
{
    // If an error, make sure our default is 0:0
    c4CompID = 0;
    c4IntID = 0;

    // Split it out to integration and component ids
    TString strIntID = strToParse;
    TString strCompID;
    if (!strIntID.bSplit(strCompID, kCIDLib::chColon))
    {
        strErrMsg = L"The address must be in the for x:y, where x and y are decimal numbers";
        return kCIDLib::False;
    }

    // Integration id cannot be zero
    if (!strIntID.bToCard4(c4IntID, tCIDLib::ERadices::Dec) || !c4IntID)
    {
        strErrMsg = L"The integration ID could not be converted to anumber";
        return kCIDLib::False;
    }

    if (!strCompID.bToCard4(c4CompID, tCIDLib::ERadices::Dec))
    {
        strErrMsg = L"The component ID could not be converted to anumber";
        return kCIDLib::False;
    }

    // Component id can only be zero for certain types and must be zero for others
    if (c4CompID)
    {
        if (   (eType == tLutronRA2C::EItemTypes::Fan)
            || (eType == tLutronRA2C::EItemTypes::Shade)
            || (eType == tLutronRA2C::EItemTypes::ShadeGroup)
            || (eType == tLutronRA2C::EItemTypes::Thermo))
        {
            strErrMsg = L"This type of item must have a zero component ID";
            return kCIDLib::False;
        }
    }
    else
    {
        if (   (eType == tLutronRA2C::EItemTypes::Button)
            || (eType == tLutronRA2C::EItemTypes::ContactCl)
            || (eType == tLutronRA2C::EItemTypes::Dimmer)
            || (eType == tLutronRA2C::EItemTypes::LED)
            || (eType == tLutronRA2C::EItemTypes::OccSensor))
        {
            strErrMsg = L"This type of item cannot have a zero component ID";
            return kCIDLib::False;
        }
    }
    return kCIDLib::True;
}


// We compare types first, and if they are equal, compare names
tCIDLib::ESortComps TRA2CItem::eComp(const TRA2CItem& item1, const TRA2CItem& item2)
{
    if (item1.m_eType < item2.m_eType)
        return tCIDLib::ESortComps::FirstLess;
    else if (item1.m_eType > item2.m_eType)
        return tCIDLib::ESortComps::FirstGreater;

    return item1.m_strName.eCompare(item2.m_strName);
}


//
//  For sorting purposes we have to do a leading zero version of the addresses, else
//  the text based sorting done by the MC list box won't sort them right.
//
tCIDLib::TVoid
TRA2CItem::FormatAddr(  const   tCIDLib::TCard4 c4IntID
                        , const tCIDLib::TCard4 c4CompID
                        ,       TString&        strToFill)
{
    strToFill.Clear();

    if (c4IntID < 10)
        strToFill.Append(kCIDLib::chDigit0);
    strToFill.AppendFormatted(c4IntID, tCIDLib::ERadices::Dec);

    strToFill.Append(kCIDLib::chColon);

    if (c4CompID < 10)
        strToFill.Append(kCIDLib::chDigit0);
    strToFill.AppendFormatted(c4CompID, tCIDLib::ERadices::Dec);
}



// ---------------------------------------------------------------------------
//  TRA2CItem: Constructors and Destructor
// ---------------------------------------------------------------------------
TRA2CItem::TRA2CItem() :

    m_bSendTrig(kCIDLib::False)
    , m_c4CompID(0)
    , m_c4IntID(0)
    , m_eType(tLutronRA2C::EItemTypes::Count)
    , m_strAddr(L"00:00")
{
}

TRA2CItem::TRA2CItem(const TString& strName, const tLutronRA2C::EItemTypes eType) :

    m_bSendTrig(kCIDLib::False)
    , m_c4CompID(0)
    , m_c4IntID(0)
    , m_eType(eType)
    , m_strName(strName)
{
    FormatAddr(m_c4IntID, m_c4CompID, m_strAddr);

    // If a thermo, default the extra string
    if (eType == tLutronRA2C::EItemTypes::Thermo)
        m_strExtra = L"Fahrenheit";
}


TRA2CItem::TRA2CItem(const TRA2CItem& itemSrc) :

    m_bSendTrig(itemSrc.m_bSendTrig)
    , m_c4CompID(itemSrc.m_c4CompID)
    , m_c4IntID(itemSrc.m_c4IntID)
    , m_eType(itemSrc.m_eType)
    , m_strAddr(itemSrc.m_strAddr)
    , m_strExtra(itemSrc.m_strExtra)
    , m_strName(itemSrc.m_strName)
{
}

TRA2CItem::~TRA2CItem()
{
}


// ---------------------------------------------------------------------------
//  TRA2CItem: Public operators
// ---------------------------------------------------------------------------
TRA2CItem& TRA2CItem::operator=(const TRA2CItem& itemSrc)
{
    if (this != &itemSrc)
    {
        m_bSendTrig = itemSrc.m_bSendTrig;
        m_c4CompID  = itemSrc.m_c4CompID;
        m_c4IntID   = itemSrc.m_c4IntID;
        m_eType     = itemSrc.m_eType;
        m_strAddr   = itemSrc.m_strAddr;
        m_strExtra  = itemSrc.m_strExtra;
        m_strName   = itemSrc.m_strName;
    }
    return *this;
}


tCIDLib::TBoolean TRA2CItem::operator==(const TRA2CItem& itemSrc) const
{
    return
    (
        (m_bSendTrig    == itemSrc.m_bSendTrig)
        && (m_c4CompID  == itemSrc.m_c4CompID)
        && (m_c4IntID   == itemSrc.m_c4IntID)
        && (m_eType     == itemSrc.m_eType)
        && (m_strAddr   == itemSrc.m_strAddr)
        && (m_strExtra  == itemSrc.m_strExtra)
        && (m_strName   == itemSrc.m_strName)
    );
}

tCIDLib::TBoolean TRA2CItem::operator!=(const TRA2CItem& itemSrc) const
{
    return !operator==(itemSrc);
}



// ---------------------------------------------------------------------------
//  TRA2CItem: Public, non-virtual methods
// ---------------------------------------------------------------------------

// We output our info to a row in the passed multi-column list box window
tCIDLib::TVoid
TRA2CItem::AddToList(TMultiColListBox& wndTar, tLutronRA2C::TColStrs& colTmp)
{
    colTmp[tLutronRA2C::EListCols::Type] = tLutronRA2C::strXlatEItemTypes(m_eType);
    colTmp[tLutronRA2C::EListCols::Name] = m_strName;
    colTmp[tLutronRA2C::EListCols::Addr] = m_strAddr;

    if ((m_eType == tLutronRA2C::EItemTypes::Button)
    ||  (m_eType == tLutronRA2C::EItemTypes::OccSensor)
    ||  (m_eType == tLutronRA2C::EItemTypes::Switch))
    {
        if (m_bSendTrig)
            colTmp[tLutronRA2C::EListCols::Trigger] = L"Yes";
        else
            colTmp[tLutronRA2C::EListCols::Trigger] = L"No";
    }
     else
    {
        colTmp[tLutronRA2C::EListCols::Trigger].Clear();
    }

    if (m_eType == tLutronRA2C::EItemTypes::Thermo)
        colTmp[tLutronRA2C::EListCols::Extra] = m_strExtra;
    else
        colTmp[tLutronRA2C::EListCols::Extra].Clear();

    wndTar.c4AddItem(colTmp, 0);
}

tCIDLib::TVoid TRA2CItem::AddToList(TMultiColListBox& wndTar)
{
    tLutronRA2C::EListCols eCol = tLutronRA2C::EListCols::Min;
    tLutronRA2C::TColStrs colCols(tLutronRA2C::EListCols::Count);
    while (eCol < tLutronRA2C::EListCols::Count)
    {
        colCols.objAdd(TString::strEmpty());
        eCol++;
    }

    AddToList(wndTar, colCols);
}


//
//  Checks to see if this item is validly configured. If not it returns False and a msg
//  indicating what is wrong.
//
tCIDLib::TBoolean TRA2CItem::bIsValid(TString& strErrMsg) const
{
    if (!m_c4IntID)
    {
        strErrMsg = L"The integration id cannot be zero";
        return kCIDLib::False;
    }

    if (m_eType >= tLutronRA2C::EItemTypes::Count)
    {
        strErrMsg = L"The item type is not correctly set";
        return kCIDLib::False;
    }

    if (!m_c4CompID)
    {
        if ((m_eType == tLutronRA2C::EItemTypes::Button)
        ||  (m_eType == tLutronRA2C::EItemTypes::LED)
        ||  (m_eType == tLutronRA2C::EItemTypes::OccSensor))
        {
            strErrMsg = tLutronRA2C::strXlatEItemTypes(m_eType);
            strErrMsg.Append(L" items cannot have a zero component ID");
            return kCIDLib::False;
        }
    }

    if (m_strName.bIsEmpty())
    {
        strErrMsg = L"The item name cannot be empty";
        return kCIDLib::False;
    }

    return kCIDLib::True;
}


//
//  Parses our content out from a configuration file line.
//
tCIDLib::TBoolean
TRA2CItem::bParseFromCfgLine(const  tLutronRA2C::EItemTypes eType
                            , const TString&                strSrc
                            ,       TString&                strError)
{
    // We can just store the type, and then we token nice the source line for the values
    m_eType = eType;
    TStringTokenizer stokValues(&strSrc, L",");

    // The first is always the name
    if (!stokValues.bGetNextToken(m_strName))
    {
        strError = L"Expected an item name";
        return kCIDLib::False;
    }

    // Make sure the name is reasonable
    if (!facCQCKit().bIsValidBaseFldName(m_strName))
    {
        strError = L"The name is not valid for a CQC field";
        return kCIDLib::False;
    }

    // Next is always the address
    TString strVal;
    if (!stokValues.bGetNextToken(strVal))
    {
        strError = L"Expected an item address";
        return kCIDLib::False;
    }

    strVal.StripWhitespace();
    if (!bParseAddr(strVal, m_c4IntID, m_c4CompID, m_eType, strError))
        return kCIDLib::False;

    // It's good so format it out into our formated address string
    FormatAddr(m_c4IntID, m_c4CompID, m_strAddr);

    // The rest are only present for particular types of items
    m_bSendTrig = kCIDLib::False;
    m_strExtra.Clear();
    if ((m_eType == tLutronRA2C::EItemTypes::Button)
    ||  (m_eType == tLutronRA2C::EItemTypes::OccSensor)
    ||  (m_eType == tLutronRA2C::EItemTypes::Switch))
    {
        // It has to be Trig or NoTrig, but treat an empty value is NoTrig
        if (stokValues.bGetNextToken(strVal))
        {
            strVal.StripWhitespace();
            if (strVal.bCompareI(L"Trig"))
                m_bSendTrig = kCIDLib::True;
            else if (strVal.bIsEmpty() || strVal.bCompareI(L"NoTrig"))
                m_bSendTrig = kCIDLib::False;
        }
         else
        {
            m_bSendTrig = kCIDLib::False;
        }
    }
     else if (m_eType == tLutronRA2C::EItemTypes::Thermo)
    {
        // It can have an F or C value
        if (stokValues.bGetNextToken(m_strExtra))
            m_strExtra.StripWhitespace();

        if (m_strExtra.bCompareI(L"F"))
        {
            m_strExtra = L"Fahrenheit";
        }
         else if (m_strExtra.bCompareI(L"C"))
        {
            m_strExtra = L"Celcius";
        }
         else
        {
            strError = L"Expected an F or C temperature scale indicator";
            return kCIDLib::False;
        }

    }
     else
    {
        // There should not be any more content
        if (stokValues.bMoreTokens())
        {
            strError = L"Invalid trailing values";
            return kCIDLib::False;
        }
    }
    return kCIDLib::True;
}


tCIDLib::TCard4 TRA2CItem::c4CompID() const
{
    return m_c4CompID;
}

tCIDLib::TCard4 TRA2CItem::c4IntID() const
{
    return m_c4IntID;
}


tLutronRA2C::EItemTypes TRA2CItem::eType() const
{
    return m_eType;
}


//
//  Parses our content out from the indicated line in the list box. We use in-line editing
//  and don't allow any invalid content into the window, so our parsing here can be fairly
//  simple.
//
tCIDLib::TVoid TRA2CItem::ParseFromLB(TMultiColListBox& wndTar, const tCIDLib::TCard4 c4Index)
{
    TString strTmp;
    wndTar.QueryColText(c4Index, tCIDLib::c4EnumOrd(tLutronRA2C::EListCols::Type), strTmp);
    m_eType = tLutronRA2C::eXlatEItemTypes(strTmp);

    wndTar.QueryColText(c4Index, tCIDLib::c4EnumOrd(tLutronRA2C::EListCols::Name), m_strName);

    wndTar.QueryColText(c4Index, tCIDLib::c4EnumOrd(tLutronRA2C::EListCols::Addr), strTmp);

    TString strErrMsg;
    if (!bParseAddr(strTmp, m_c4IntID, m_c4CompID, m_eType, strErrMsg))
    {
    }
    FormatAddr(m_c4IntID, m_c4CompID, m_strAddr);

    wndTar.QueryColText(c4Index, tCIDLib::c4EnumOrd(tLutronRA2C::EListCols::Trigger), strTmp);
    m_bSendTrig = strTmp.bCompareI(L"Yes");

    wndTar.QueryColText(c4Index, tCIDLib::c4EnumOrd(tLutronRA2C::EListCols::Extra), m_strExtra);
}


const TString& TRA2CItem::strExtra() const
{
    return m_strExtra;
}

const TString& TRA2CItem::strExtra(const TString& strToSet)
{
    m_strExtra = strToSet;
    return m_strExtra;
}


const TString& TRA2CItem::strName() const
{
    return m_strName;
}


// ---------------------------------------------------------------------------
//  TRA2CItem: Protected, inherited methods
// ---------------------------------------------------------------------------

//
//  Our support for standard formatting out to a text stream uses the format that the
//  config file needs, which is very convenient.
//
tCIDLib::TVoid TRA2CItem::FormatTo(TTextOutStream& strmDest) const
{
    strmDest    << m_eType
                << kCIDLib::chEquals
                << m_strName
                << kCIDLib::chComma
                << m_c4IntID
                << kCIDLib::chColon
                << m_c4CompID;

    // The rest of the values are type specific as to whether they are present
    switch(m_eType)
    {
        case tLutronRA2C::EItemTypes::Button :
            if (m_bSendTrig)
                strmDest << L", Trig";
            else
                strmDest << L", NoTrig";
            break;

        case tLutronRA2C::EItemTypes::ContactCl :
        case tLutronRA2C::EItemTypes::Dimmer :
        case tLutronRA2C::EItemTypes::LED :
            break;

        case tLutronRA2C::EItemTypes::OccSensor :
            if (m_bSendTrig)
                strmDest << L", Trig";
            else
                strmDest << L", NoTrig";
            break;

        case tLutronRA2C::EItemTypes::Switch :
            if (m_bSendTrig)
                strmDest << L", Trig";
            else
                strmDest << L", NoTrig";
            break;

        case tLutronRA2C::EItemTypes::Thermo :
            strmDest << kCIDLib::chComma << m_strExtra;
            break;

        default :
            CIDAssert2(L"Unknown item type to format");
            break;
    };
}

