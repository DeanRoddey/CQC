//
// FILE NAME: LutronHWC_Item.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/09/2006
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
//  This file implements a simple class that is used to store per-item config data
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
#include    "LutronHWC_.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(THWCItem,TObject)


// ---------------------------------------------------------------------------
//   CLASS: THWCItem
//  PREFIX: item
// ---------------------------------------------------------------------------

// Returns true if the passed type is one that needs a number (in addition to the address)
tCIDLib::TBoolean THWCItem::bTypeNeedsNumber(const tLutronHWC::EItemTypes eType)
{
    return
    (
        (eType == tLutronHWC::EItemTypes::Button)
        || (eType == tLutronHWC::EItemTypes::LED)
    );
}


// Parses the textual form of a Lutron item address and returns the info
tCIDLib::TBoolean
THWCItem::bXlatAddr(const   TString&                strXlat
                    , const tLutronHWC::EItemTypes  eType
                    ,       tCIDLib::TCard8&        c8Addr
                    ,       tCIDLib::TCard4&        c4AddrBytes
                    ,       TString&                strErrMsg)
{
    TStringTokenizer stokAddr(&strXlat, L":");
    c8Addr = 0;
    c4AddrBytes = 0;

    tCIDLib::TBoolean   bValid = kCIDLib::True;
    tCIDLib::TCard8     c8Cur = 0;
    TString             strVal;
    try
    {
        //
        //  Pull out components and feed each one into the low byte, shifting
        //  the old ones up as we go.
        //
        while (stokAddr.bGetNextToken(strVal))
        {
            strVal.StripWhitespace();
            c8Cur = strVal.c4Val(tCIDLib::ERadices::Dec);

            if (c8Cur > 255)
            {
                // Has to be a single byte value
                strErrMsg = facLutronHWC().strMsg(kLHWCErrs::errcCfg_AddrByteVal);
                bValid = kCIDLib::False;

                // Don't let it mess up the subsequent components
                c8Cur &= 0xFF;
            }
             else if (c8Cur == 0)
            {
                //
                //  If zero, that can only be in the 2nd index slot for a dimmer
                //  only.
                //
                if ((eType != tLutronHWC::EItemTypes::Dimmer) || (c4AddrBytes != 2))
                {
                    strErrMsg = facLutronHWC().strMsg(kLHWCErrs::errcCfg_AddrByteVal);
                    bValid = kCIDLib::False;
                }
            }

            c8Addr <<= 8;
            c8Addr |= c8Cur;
            c4AddrBytes++;
        }

        // We had to get at least 3 and no more than 5 parts
        if ((c4AddrBytes < 3) || (c4AddrBytes > 5))
        {
            strErrMsg = facLutronHWC().strMsg
            (
                kLHWCErrs::errcCfg_AddrBytes, TCardinal64(c8Addr, tCIDLib::ERadices::Hex)
            );
            bValid = kCIDLib::False;
        }
    }

    catch(const TError& errToCatch)
    {
        strErrMsg = errToCatch.strErrText();
        bValid = kCIDLib::False;
    }

    catch(...)
    {
        strErrMsg = L"System exception occurred while parsing the address: ";
        strErrMsg.Append(strXlat);
        return kCIDLib::False;
    }
    return bValid;
}


//
//  A helper to build up the combo address we use uniquely identify items. See the
//  main driver class header comments.
//
tCIDLib::TCard8
THWCItem::c8MakeComboAddr(  const   tLutronHWC::EItemTypes  eType
                            , const tCIDLib::TCard8         c8Addr
                            , const tCIDLib::TCard1         c1Num)
{
    tCIDLib::TCard8 c8Ret = tCIDLib::TCard1(eType);
    c8Ret <<= 8;
    c8Ret |= c1Num;
    c8Ret <<= 48;
    c8Ret |= c8Addr;

    return c8Ret;
}


//
//  We compare types first, and if they are equal, compare names. This is our canonical
//  format that we use to compare previous and new data, and also to format out the
//  data to send to the server.
//
tCIDLib::ESortComps THWCItem::eComp(const THWCItem& item1, const THWCItem& item2)
{
    if (item1.m_eType < item2.m_eType)
        return tCIDLib::ESortComps::FirstLess;
    else if (item1.m_eType > item2.m_eType)
        return tCIDLib::ESortComps::FirstGreater;

    return item1.m_strName.eCompare(item2.m_strName);
}


// Formats the address out in colon separated format
tCIDLib::TVoid
THWCItem::FormatAddr(const  tCIDLib::TCard8     c8Addr
                    , const tCIDLib::TCard4     c4AddrBytes
                    ,       TString&            strToFill)
{
    strToFill.Clear();

    // Can't be more than five address bytes
    TString strCur;
    tCIDLib::TCard4 c4Index = 5;
    do
    {
        c4Index--;

        // If we have something so far, add a colon
        if (!strToFill.bIsEmpty())
            strToFill.Append(kCIDLib::chColon);

        // Get the next byte and format it out if it's not a leading zero
        const tCIDLib::TCard1 c1Cur = tCIDLib::TCard1(c8Addr >> (c4Index * 8));
        if (c1Cur || !strToFill.bIsEmpty())
            strToFill.AppendFormatted(c1Cur);

    }   while (c4Index);
}





// ---------------------------------------------------------------------------
//  THWCItem: Constructors and Destructor
// ---------------------------------------------------------------------------

//
//  For adding a new item, so we only know the name and type at this point. The user
//  has to setup the rest after it's in the list box. We just set some reasonable
//  default values.
//
THWCItem::THWCItem(const TString& strName, const tLutronHWC::EItemTypes eType) :

    m_c1Number(1)
    , m_c4AddrBytes(0)
    , m_c8Addr(0)
    , m_eType(eType)
    , m_strName(strName)
{
}

THWCItem::THWCItem(const THWCItem& itemSrc) :

    m_c1Number(itemSrc.m_c1Number)
    , m_c4AddrBytes(itemSrc.m_c4AddrBytes)
    , m_c8Addr(itemSrc.m_c8Addr)
    , m_eType(itemSrc.m_eType)
    , m_strAddr(itemSrc.m_strAddr)
    , m_strName(itemSrc.m_strName)
{
}

THWCItem::~THWCItem()
{
}


// ---------------------------------------------------------------------------
//  THWCItem: Public operators
// ---------------------------------------------------------------------------
THWCItem& THWCItem::operator=(const THWCItem& itemSrc)
{
    if (this != &itemSrc)
    {
        m_c1Number      = itemSrc.m_c1Number;
        m_c4AddrBytes   = itemSrc.m_c4AddrBytes;
        m_c8Addr        = itemSrc.m_c8Addr;
        m_eType         = itemSrc.m_eType;
        m_strAddr       = itemSrc.m_strAddr;
        m_strName       = itemSrc.m_strName;
    }
    return *this;
}


tCIDLib::TBoolean THWCItem::operator==(const THWCItem& itemSrc) const
{
    if (this == &itemSrc)
        return kCIDLib::True;

    return
    (
        (m_c1Number == itemSrc.m_c1Number)
        && (m_c4AddrBytes == itemSrc.m_c4AddrBytes)
        && (m_c8Addr == itemSrc.m_c8Addr)
        && (m_eType == itemSrc.m_eType)
        && (m_strAddr == itemSrc.m_strAddr)
        && (m_strName == itemSrc.m_strName)
    );
}

tCIDLib::TBoolean THWCItem::operator!=(const THWCItem& itemSrc) const
{
    return !operator==(itemSrc);
}


// ---------------------------------------------------------------------------
//  THWCItem: Public, non-virtual methods
// ---------------------------------------------------------------------------

// We output our info to a row in the passed multi-column list box window
tCIDLib::TVoid
THWCItem::AddToList(TMultiColListBox& wndTar, const tCIDLib::TBoolean bSelect)
{
    tCIDLib::TStrList colCols(tCIDLib::c4EnumOrd(tLutronHWC::EListCols::Count));
    colCols.AddXCopies(TString::strEmpty(), tCIDLib::c4EnumOrd(tLutronHWC::EListCols::Count));

    // Call the other version
    AddToList(wndTar, colCols, bSelect);
}

tCIDLib::TVoid
THWCItem::AddToList(        TMultiColListBox&   wndTar
                    ,       tCIDLib::TStrList&  colTmp
                    , const tCIDLib::TBoolean   bSelect)
{
    colTmp[tCIDLib::c4EnumOrd(tLutronHWC::EListCols::Type)] = tLutronHWC::strXlatEItemTypes(m_eType);
    colTmp[tCIDLib::c4EnumOrd(tLutronHWC::EListCols::Name)] = m_strName;
    colTmp[tCIDLib::c4EnumOrd(tLutronHWC::EListCols::Address)] = m_strAddr;

    // If it needs a number deal with that
    if (bTypeNeedsNumber(m_eType))
        colTmp[tCIDLib::c4EnumOrd(tLutronHWC::EListCols::Number)].SetFormatted(m_c1Number);
    else
        colTmp[tCIDLib::c4EnumOrd(tLutronHWC::EListCols::Number)].Clear();

    const tCIDLib::TCard4 c4Index = wndTar.c4AddItem(colTmp, 0, kCIDLib::c4MaxCard, bSelect);

    //
    //  Create a combination address with our number in the high byte, and store it on this
    //  entry.
    //
    tCIDLib::TCard8 c8ComboAddr = c8MakeComboAddr(m_eType, m_c8Addr, m_c1Number);
    wndTar.SetUserDataAt(c4Index, c8ComboAddr);
}


//
//  The driver gives us a line from the config data, and we parse that to set ourself up.
//  He has pre-tokenized it on commas and stripped leading/trailing whitespace off of
//  the tokens. He checks that are at least two two tokens.
//
//  This is also used to parse our info in from the list box when the user wants to
//  save. The columns are just loaded into tokens and passed here. They are in the same
//  format as what we get from the file.
//
//  They can tell us to fix up bad addresses or report them as errors. When loading the
//  data initially we let it fix up errors so that we can load the data worse case. They
//  set it each time and we set clear it if we didn't need to or leave it set if we
//  did.
//
tCIDLib::TBoolean
THWCItem::bParseFromData(const  tCIDLib::TStrList&  colTokens
                        ,       TString&            strErrMsg
                        ,       tCIDLib::TBoolean&  bFixUpAddrs)
{
    // Check the type
    m_eType = tLutronHWC::eXlatEItemTypes(colTokens[0]);
    if (m_eType == tLutronHWC::EItemTypes::Count)
    {
        strErrMsg = facLutronHWC().strMsg(kLHWCErrs::errcCfg_BadType, colTokens[0]);
        return kCIDLib::False;
    }

    // First we want to check the name that it is at least basically valid for fields
    if (!facCQCKit().bIsValidBaseFldName(colTokens[1]))
    {
        strErrMsg = facLutronHWC().strMsg(kLHWCErrs::errcCfg_BadItemName, m_strName);
        return kCIDLib::False;
    }

    // Looks ok so store it
    m_strName = colTokens[1];

    //
    //  And let's parse the address, which is the next token. If it was never set from
    //  having been just added, then it will be empty, so give a special error for that.
    //
    if (colTokens[2].bIsEmpty())
    {
        strErrMsg = facLutronHWC().strMsg(kLHWCErrs::errcCfg_AddrNotSet, m_strName);
        return kCIDLib::False;
    }

    // Not empty, so has to parse validly
    if (bXlatAddr(colTokens[2], m_eType, m_c8Addr, m_c4AddrBytes, strErrMsg))
    {
        // Indicate we didn't fix the address
        bFixUpAddrs = kCIDLib::False;
    }
     else
    {
        if (!bFixUpAddrs)
            return kCIDLib::False;

        // Fix it and leave the flag set
        m_c8Addr = 0;
    }

    // Looks ok so do the formatted version
    FormatAddr(m_c8Addr, m_c4AddrBytes, m_strAddr);

    // If a type that has number, get that, else zero it
    if (bTypeNeedsNumber(m_eType))
    {
        tCIDLib::TCard4 c4Num;
        if (!colTokens[3].bToCard4(c4Num, tCIDLib::ERadices::Dec) || !c4Num || (c4Num > 255))
        {
            strErrMsg = facLutronHWC().strMsg(kLHWCErrs::errcCfg_BadNumber);
            return kCIDLib::False;
        }
        m_c1Number = tCIDLib::TCard1(c4Num);
    }
     else
    {
        m_c1Number = 0;
    }
    return kCIDLib::True;
}


// Do basic validation on our content and return an error msg if bad
tCIDLib::TBoolean THWCItem::bValidate(TString& strErrMsg) const
{
    if (!facCQCKit().bIsValidBaseFldName(m_strName))
    {
        strErrMsg = facLutronHWC().strMsg(kLHWCErrs::errcCfg_BadItemName, m_strName);
        return kCIDLib::False;
    }

    // They may have never editted the default address
    if (m_c8Addr == 0)
    {
        strErrMsg = facLutronHWC().strMsg(kLHWCErrs::errcCfg_AddrNotSet, m_strName);
        return kCIDLib::False;
    }

    if (m_eType >= tLutronHWC::EItemTypes::Count)
    {
        strErrMsg = facLutronHWC().strMsg
        (
            kLHWCErrs::errcCfg_BadType, TCardinal(tCIDLib::c4EnumOrd(m_eType))
        );
        return kCIDLib::False;
    }

    return kCIDLib::True;
}


tCIDLib::TCard1 THWCItem::c1Number() const
{
    return m_c1Number;
}


tCIDLib::TCard4 THWCItem::c4AddrBytes() const
{
    return m_c4AddrBytes;
}


tCIDLib::TCard8 THWCItem::c8Addr() const
{
    return m_c8Addr;
}


tLutronHWC::EItemTypes THWCItem::eType() const
{
    return m_eType;
}

tLutronHWC::EItemTypes THWCItem::eType(const tLutronHWC::EItemTypes eToSet)
{
    m_eType = eToSet;
    return m_eType;
}


const TString& THWCItem::strAddr() const
{
    return m_strAddr;
}


const TString& THWCItem::strName() const
{
    return m_strName;
}


// ---------------------------------------------------------------------------
//  THWCItem: Protected, inherited methods
// ---------------------------------------------------------------------------

// Format out in the form that works for generating config to send back to the server
tCIDLib::TVoid THWCItem::FormatTo(TTextOutStream& strmDest) const
{
    strmDest << m_eType << kCIDLib::chEquals << m_strName << kCIDLib::chComma << m_strAddr;
    if (m_c1Number)
        strmDest << kCIDLib::chComma << m_c1Number;
}


