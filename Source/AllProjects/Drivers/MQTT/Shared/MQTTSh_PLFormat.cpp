//
// FILE NAME: MQTTSh_PLFormat.cpp
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
RTTIDecls(TMQTTPLFmt, TObject)


// ---------------------------------------------------------------------------
//   CLASS: TMQTTPLFmt
//  PREFIX: mqplf
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TMQTTPLFmt: Constructors and destructor
// ---------------------------------------------------------------------------
TMQTTPLFmt::TMQTTPLFmt() :

    m_bBigEndian(kCIDLib::True)
    , m_c4Bytes(0)
    , m_c4DecDigits(0)
    , m_c4Offset(0)
    , m_c4Precision(0)
    , m_eType(tMQTTSh::EPLTypes::Text)
{
}

TMQTTPLFmt::~TMQTTPLFmt()
{
    try
    {
        m_cptrEncode.DropRef();
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);
    }
}


// ---------------------------------------------------------------------------
//  TMQTTPLFmt: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  A convenience to check if our format is numeric or not. If either deciimal digits
//  or precision is non-zero, that means it's a floating point, though the actual
//  PL format type is text.
//
tCIDLib::TBoolean TMQTTPLFmt::bIsNFNumericPL() const
{
    return
    (
        (m_eType == tMQTTSh::EPLTypes::Card)
        || (m_eType == tMQTTSh::EPLTypes::Int)
    );
}

tCIDLib::TBoolean TMQTTPLFmt::bIsNumericPL() const
{
    return
    (
        (m_eType == tMQTTSh::EPLTypes::Card)
        || (m_eType == tMQTTSh::EPLTypes::Int)
        || (m_c4DecDigits != 0)
        || (m_c4Precision != 0)
    );
}


tCIDLib::TVoid
TMQTTPLFmt::ParseFrom(  const   TString&                strFldName
                        , const tCQCKit::EFldSTypes     eSemType
                        , const tCQCKit::EFldTypes      eFldType
                        , const tCQCKit::EFldAccess     eAccess
                        , const TXMLTreeElement&        xtnodeSrc
                        ,       tCIDLib::TStrCollect&   colErrs)
{
    // Some attribute names used multiple times below
    const TString strBytes(L"Bytes");
    const TString strDecDigits(L"DecDigits");
    const TString strOffset(L"Offset");
    const TString strPrecis(L"Precis");
    const TString strType(L"Type");

    // The payload type should map to our enum
    const TString& strPLType = xtnodeSrc.xtattrNamed(strType).strValue();
    m_eType = tMQTTSh::eXlatEPLTypes(strPLType);

    if (m_eType == tMQTTSh::EPLTypes::BinText)
    {
        // There's an optional text encoding. If not set, we go with US-ASCII
        TString strVal;
        if (!xtnodeSrc.bAttrExists(L"Encoding", strVal))
            strVal = L"US-ASCII";
        m_cptrEncode = facCIDEncode().cptrMake(strVal);
        if (!m_cptrEncode)
            TMQTTCfg::AddBadValErr(colErrs, strFldName, strVal, L"text encoding");

        // We can use offset and bytes if present
        if (xtnodeSrc.bAttrExists(strBytes, strVal))
        {
            if (!strVal.bToCard4(m_c4Bytes, tCIDLib::ERadices::Dec))
                TMQTTCfg::AddBadValErr(colErrs, strFldName, strVal, L"text bytes");
        }

        if (xtnodeSrc.bAttrExists(strOffset, strVal))
        {
            if (!strVal.bToCard4(m_c4Offset, tCIDLib::ERadices::Dec))
                TMQTTCfg::AddBadValErr(colErrs, strFldName, strVal, L"text offset");
        }
    }
     else if ((m_eType == tMQTTSh::EPLTypes::Card)
          ||  (m_eType == tMQTTSh::EPLTypes::Int))
    {
        //
        //  Make sure PLBytes there. The DTD will validate it's a valid value if
        //  present.
        //
        TString strVal;
        if (!xtnodeSrc.bAttrExists(strBytes, strVal))
            TMQTTCfg::AddRequiredValErr(colErrs, strFldName, L"payload bytes");
        else if (!strVal.bToCard4(m_c4Bytes, tCIDLib::ERadices::Dec))
            TMQTTCfg::AddBadValErr(colErrs, strFldName, strVal, L"binary bytes");

        // The offset is optional
        if (xtnodeSrc.bAttrExists(strOffset, strVal)
        &&  !strVal.bToCard4(m_c4Offset, tCIDLib::ERadices::Dec))
        {
            TMQTTCfg::AddBadValErr(colErrs, strFldName, strVal, L"binary offset");
        }

        //
        //  Endianness is defaulted to Big if not explicitly set, so we can
        //  always get it.
        //
        m_bBigEndian = xtnodeSrc.xtattrNamed(L"Endian").strValue() == L"Big";
    }
     else if (m_eType == tMQTTSh::EPLTypes::Text)
    {
        // For now, nothing specific
    }
     else
    {
        TMQTTCfg::AddBadValErr(colErrs, strFldName, strPLType, L"payload type");
    }

    //
    //  Optional precision value. It is only valid for text encoding floating point
    //  values. Same for the optional decimal digits.
    //
    m_c4Precision = 0;
    TString strVal;
    if (xtnodeSrc.bAttrExists(strPrecis, strVal) && !strVal.bToCard4(m_c4Precision))
        TMQTTCfg::AddBadValErr(colErrs, strFldName, strVal, L"precision");

    m_c4DecDigits = 0;
    if (xtnodeSrc.bAttrExists(strDecDigits, strVal) && !strVal.bToCard4(m_c4DecDigits))
        TMQTTCfg::AddBadValErr(colErrs, strFldName, strVal, L"decimal digits");

    // Only one of decimal digits and precision can be used
    if (m_c4Precision && m_c4DecDigits)
        colErrs.objAdd(TString(kMQTTShErrs::errcCfg_FloatConfict, facMQTTSh(), strFldName));

    // Decimal digits is only valid for writeable fields
    if (m_c4DecDigits && !tCIDLib::bAllBitsOn(eAccess, tCQCKit::EFldAccess::Write))
    {
        colErrs.objAdd
        (
            facMQTTSh().strMsg(kMQTTShErrs::errcCfg_FldNotWritable, strDecDigits, strFldName)
        );
    }
}

