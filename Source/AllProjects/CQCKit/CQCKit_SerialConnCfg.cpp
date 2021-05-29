//
// FILE NAME: CQCKit_SerialConnCfg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/25/2001
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
#include    "CQCKit_.hpp"


// ---------------------------------------------------------------------------
//  Magic Macros
// ---------------------------------------------------------------------------
AdvRTTIDecls(TCQCSerialConnCfg,TCQCConnCfg)


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace
{
    namespace CQCKit_SerialConnCfg
    {
        // -----------------------------------------------------------------------
        //  Our current persistent format version
        //
        //  Version 2 -
        //      We moved from an enumerated type to represent serial ports, to
        //      a path based system, so that we can support more than just
        //      local port.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2       c2FmtVersion = 2;
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCSerialConnCfg
//  PREFIX: ccfg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCSerialConnCfg: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCSerialConnCfg::TCQCSerialConnCfg() :

    m_ePortType(tCIDComm::EPortTypes::RS232)
    , m_strPortPath()
{
}

TCQCSerialConnCfg::TCQCSerialConnCfg(const  TCommPortCfg&           cpcfgToUse
                                    , const TString&                strPortToUse
                                    , const tCIDComm::EPortTypes    eType) :

    m_cpcfgSerial(cpcfgToUse)
    , m_ePortType(eType)
    , m_strPortPath(strPortToUse)
{
}

TCQCSerialConnCfg::~TCQCSerialConnCfg()
{
}


// ---------------------------------------------------------------------------
//  TCQCSerialConnCfg: Public operators
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TCQCSerialConnCfg::operator==(const TCQCSerialConnCfg& conncfgToCompare) const
{
    if ((m_strPortPath != conncfgToCompare.m_strPortPath)
    ||  (m_cpcfgSerial != conncfgToCompare.m_cpcfgSerial)
    ||  (m_ePortType != conncfgToCompare.m_ePortType))
    {
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


// ---------------------------------------------------------------------------
//  TCQCSerialConnCfg: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TCQCSerialConnCfg::bIsSame(const TCQCConnCfg& conncfgToCompare) const
{
    // See if they are of the same polymorphic type. If not, can't be equal
    if (clsThis() != conncfgToCompare.clsIsA())
        return kCIDLib::False;

    //
    //  They are, so compare them. We cast the other one to our type and
    //  then we can just do an equality test.
    //
    const TCQCSerialConnCfg& conncfgOther = static_cast<const TCQCSerialConnCfg&>
    (
        conncfgToCompare
    );
    return (*this == conncfgOther);
}


tCIDLib::TBoolean TCQCSerialConnCfg::bParseConnInfo(const TString& strInfo)
{
    //
    //  It will be a comm port path at least, possibly with a comma and a baud
    //  rate.
    //
    TStringTokenizer stokPort(&strInfo, L",");

    TString strToken;
    if (!stokPort.bGetNextToken(strToken))
        return kCIDLib::False;

    // Validate the port path
    if (!facCIDComm().bIsValidPortName(strToken))
        return kCIDLib::False;
    m_strPortPath = strToken;

    // Check for a baud rate
    if (stokPort.bGetNextToken(strToken))
    {
        tCIDLib::TCard4 c4Baud;
        if (!strToken.bToCard4(c4Baud, tCIDLib::ERadices::Dec))
            return kCIDLib::False;
        m_cpcfgSerial.c4Baud(c4Baud);
    }

    return kCIDLib::True;
}


tCIDLib::TVoid TCQCSerialConnCfg::LoadFromXML(const TXMLTreeElement& xtnodeSrc)
{
    AssertElemName(xtnodeSrc, L"CQCCfg:SerialConn")

    // Get the port type
    m_ePortType = (xtnodeSrc.xtattrNamed(L"CQCCfg:Type").strValue() == L"RS232")
                  ? tCIDComm::EPortTypes::RS232 : tCIDComm::EPortTypes::RS422;

    //
    //  Get the baud value, and strip off the leading Baud_ prefix. The
    //  remainder will be the actual baud value.
    //
    TString strBaud = xtnodeSrc.xtattrNamed(L"CQCCfg:Baud").strValue();
    strBaud.Cut(0, 5);
    try
    {
        m_cpcfgSerial.c4Baud(strBaud.c4Val(tCIDLib::ERadices::Dec));
    }

    catch(...)
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcCfg_BadBaud
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , xtnodeSrc.xtattrNamed(L"CQCCfg:Baud").strValue()
        );
    }

    const TString& strBits = xtnodeSrc.xtattrNamed(L"CQCCfg:Bits").strValue();
    if (strBits == L"Bits4")
        m_cpcfgSerial.eDataBits(tCIDComm::EDataBits::Four);
    else if (strBits == L"Bits5")
        m_cpcfgSerial.eDataBits(tCIDComm::EDataBits::Five);
    else if (strBits == L"Bits6")
        m_cpcfgSerial.eDataBits(tCIDComm::EDataBits::Six);
    else if (strBits == L"Bits7")
        m_cpcfgSerial.eDataBits(tCIDComm::EDataBits::Seven);
    else if (strBits == L"Bits8")
        m_cpcfgSerial.eDataBits(tCIDComm::EDataBits::Eight);
    else
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcCfg_BadDataBits
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , strBits
        );
    }

    const TString& strParity = xtnodeSrc.xtattrNamed(L"CQCCfg:Parity").strValue();
    if (strParity == L"None")
        m_cpcfgSerial.eParity(tCIDComm::EParities::None);
    else if (strParity == L"Odd")
        m_cpcfgSerial.eParity(tCIDComm::EParities::Odd);
    else if (strParity == L"Even")
        m_cpcfgSerial.eParity(tCIDComm::EParities::Even);
    else if (strParity == L"Space")
        m_cpcfgSerial.eParity(tCIDComm::EParities::Space);
    else if (strParity == L"Mark")
        m_cpcfgSerial.eParity(tCIDComm::EParities::Mark);
    else
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcCfg_BadParity
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , strParity
        );
    }

    const TString& strStop = xtnodeSrc.xtattrNamed(L"CQCCfg:StopBits").strValue();
    if (strStop == L"One")
        m_cpcfgSerial.eStopBits(tCIDComm::EStopBits::One);
    else if (strStop == L"OnePtFive")
        m_cpcfgSerial.eStopBits(tCIDComm::EStopBits::OnePointFive);
    else if (strStop == L"Two")
        m_cpcfgSerial.eStopBits(tCIDComm::EStopBits::Two);
    else
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcCfg_BadStopBits
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , strStop
        );
    }

    // Set up the flags field, which takes multiple attibutes
    tCIDComm::EFlags eFlags = tCIDComm::EFlags::None;

    const TString& strParityCheck = xtnodeSrc.xtattrNamed(L"CQCCfg:ParityCheck").strValue();
    if (strParityCheck == L"Yes")
        eFlags = tCIDLib::eOREnumBits(eFlags, tCIDComm::EFlags::ParityChecking);
    else if (strParityCheck != L"No")
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcCfg_BadParityCheck
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , strParityCheck
        );
    }

    const TString& strCTSOut = xtnodeSrc.xtattrNamed(L"CQCCfg:CTSOut").strValue();
    if (strCTSOut == L"Yes")
        eFlags = tCIDLib::eOREnumBits(eFlags, tCIDComm::EFlags::CTSOut);
    else if (strCTSOut != L"No")
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcCfg_BadCTSOut
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , strCTSOut
        );
    }

    const TString& strDSROut = xtnodeSrc.xtattrNamed(L"CQCCfg:DSROut").strValue();
    if (strDSROut == L"Yes")
        eFlags = tCIDLib::eOREnumBits(eFlags, tCIDComm::EFlags::DSROut);
    else if (strDSROut != L"No")
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcCfg_BadDSROut
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , strDSROut
        );
    }

    const TString& strXIn = xtnodeSrc.xtattrNamed(L"CQCCfg:XIn").strValue();
    if (strXIn == L"Yes")
        eFlags = tCIDLib::eOREnumBits(eFlags, tCIDComm::EFlags::XIn);
    else if (strXIn != L"No")
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcCfg_BadXIn
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , strXIn
        );
    }

    const TString& strXOut = xtnodeSrc.xtattrNamed(L"CQCCfg:XOut").strValue();
    if (strXOut == L"Yes")
        eFlags = tCIDLib::eOREnumBits(eFlags, tCIDComm::EFlags::XOut);
    else if (strXOut != L"No")
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcCfg_BadXOut
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , strXOut
        );
    }

    // We have all the flags now, so set them
    m_cpcfgSerial.eFlags(eFlags);

    // And do the DTR/RTS flags
    const TString& strDTR = xtnodeSrc.xtattrNamed(L"CQCCfg:DTRFlag").strValue();
    if (strDTR == L"Disable")
        m_cpcfgSerial.eDTR(tCIDComm::EPortDTR::Disable);
    else if (strDTR == L"Enable")
        m_cpcfgSerial.eDTR(tCIDComm::EPortDTR::Enable);
    else if (strDTR == L"Handshake")
        m_cpcfgSerial.eDTR(tCIDComm::EPortDTR::Handshake);
    else
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcCfg_BadDTR
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , strDTR
        );
    }

    const TString& strRTS = xtnodeSrc.xtattrNamed(L"CQCCfg:RTSFlag").strValue();
    if (strRTS == L"Disable")
        m_cpcfgSerial.eRTS(tCIDComm::EPortRTS::Disable);
    else if (strRTS == L"Enable")
        m_cpcfgSerial.eRTS(tCIDComm::EPortRTS::Enable);
    else if (strRTS == L"Handshake")
        m_cpcfgSerial.eRTS(tCIDComm::EPortRTS::Handshake);
    else
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcCfg_BadRTS
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , strRTS
        );
    }
}


TString TCQCSerialConnCfg::strConnInfo() const
{
    //
    //  Load up the message and replace the token with the formatted port
    //  info. We have to use a temp stream to format the port type.
    //
    TTextStringOutStream strmTmp(64);
    strmTmp << m_ePortType << kCIDLib::FlushIt;

    TString strRet
    (
        kKitMsgs::midConn_Type_Serial
        , facCQCKit()
        , m_strPortPath
        , strmTmp.strData()
    );
    return strRet;
}


// ---------------------------------------------------------------------------
//  TCQCSerialConnCfg: Public, non-virtual methods
// ---------------------------------------------------------------------------
const TCommPortCfg& TCQCSerialConnCfg::cpcfgSerial() const
{
    return m_cpcfgSerial;
}

const TCommPortCfg&
TCQCSerialConnCfg::cpcfgSerial(const TCommPortCfg& cpcfgToSet)
{
    m_cpcfgSerial = cpcfgToSet;
    return m_cpcfgSerial;
}


const TString& TCQCSerialConnCfg::strPortPath() const
{
    return m_strPortPath;
}

const TString& TCQCSerialConnCfg::strPortPath(const TString& strToSet)
{
    m_strPortPath = strToSet;
    return m_strPortPath;
}


tCIDComm::EPortTypes TCQCSerialConnCfg::ePortType() const
{
    return m_ePortType;
}


// ---------------------------------------------------------------------------
//  TCQCSerialConnCfg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCSerialConnCfg::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Do our parent first
    TParent::StreamFrom(strmToReadFrom);

    // Our stuff should start with a frame marker
    strmToReadFrom.CheckForFrameMarker(CID_FILE, CID_LINE);

    // Check our version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCKit_SerialConnCfg::c2FmtVersion))
    {
        facCIDLib().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCIDErrs::errcGen_UnknownFmtVersion
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(c2FmtVersion)
            , clsThis()
        );
    }

    //
    //  If version 1, then stream in the port enum and convert to the new
    //  path format. We don't have the enum anymore, but it's just the
    //  port number, zero based.
    //
    if (c2FmtVersion == 1)
    {
        tCIDLib::TInt4 i4Port;
        strmToReadFrom  >> i4Port;

        m_strPortPath = L"/Local/COM";
        m_strPortPath.AppendFormatted(tCIDLib::TCard4(i4Port + 1));
    }
     else
    {
        strmToReadFrom >> m_strPortPath;
    }

    strmToReadFrom  >> m_ePortType
                    >> m_cpcfgSerial;

    // Check the end object marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TCQCSerialConnCfg::StreamTo(TBinOutStream& strmToWriteTo) const
{
    TParent::StreamTo(strmToWriteTo);
    strmToWriteTo   << tCIDLib::EStreamMarkers::Frame
                    << CQCKit_SerialConnCfg::c2FmtVersion
                    << m_strPortPath
                    << m_ePortType
                    << m_cpcfgSerial
                    << tCIDLib::EStreamMarkers::EndObject;
}


