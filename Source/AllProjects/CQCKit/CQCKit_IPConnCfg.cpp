//
// FILE NAME: CQCKit_IPConnCfg.cpp
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
AdvRTTIDecls(TCQCIPConnCfg,TCQCConnCfg)


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace
{
    namespace CQCKit_IPConnCfg
    {
        // -----------------------------------------------------------------------
        //  Our current persistent format version
        //
        //  Version 2 -
        //      Added the address family type indicator, part of the support for
        //      IPV6.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2       c2FmtVersion = 2;
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCIPConnCfg
//  PREFIX: ccfg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIPConnCfg: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIPConnCfg::TCQCIPConnCfg() :

    m_bResolved(kCIDLib::False)
    , m_eAddrType(tCIDSock::EAddrTypes::Unspec)
    , m_eProto(tCIDSock::ESockProtos::IP)
    , m_ippnTarget(0)
{
}


TCQCIPConnCfg::~TCQCIPConnCfg()
{
}


// ---------------------------------------------------------------------------
//  TCQCIPConnCfg: Public operators
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TCQCIPConnCfg::operator==(const TCQCIPConnCfg& conncfgToCompare) const
{
    // We don't include the runtime only stuff here
    return (m_eProto == conncfgToCompare.m_eProto)
           && (m_strAddr == conncfgToCompare.m_strAddr)
           && (m_ippnTarget == conncfgToCompare.m_ippnTarget);
}


tCIDLib::TBoolean
TCQCIPConnCfg::operator!=(const TCQCIPConnCfg& conncfgToCompare) const
{
    return !operator==(conncfgToCompare);
}


// ---------------------------------------------------------------------------
//  TCQCIPConnCfg: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TCQCIPConnCfg::bIsSame(const TCQCConnCfg& conncfgToCompare) const
{
    // See if they are of the same polymorphic type. If not, can't be equal
    if (clsThis() != conncfgToCompare.clsIsA())
        return kCIDLib::False;

    //
    //  They are, so compare them. We cast the other one to our type and
    //  then we can just do an equality test.
    //
    const TCQCIPConnCfg& conncfgOther = static_cast<const TCQCIPConnCfg&>
    (
        conncfgToCompare
    );
    return (*this == conncfgOther);
}


//
//  This is used to allow the info to be set by way of a formatted string
//  with the address and port in it. Mostly it's for use by the driver
//  IDE.
//
tCIDLib::TBoolean TCQCIPConnCfg::bParseConnInfo(const TString& strInfo)
{
    try
    {
        TIPEndPoint ipepTest(strInfo, m_eAddrType);

        // It didn't throw, so take it
        m_ippnTarget = ipepTest.ippnThis();
        m_strAddr = ipepTest.strAsText();

        m_ipepTarget = ipepTest;
    }

    catch(...)
    {
        // It ain't good
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


//
//  We only get the port and protocol. The end point address is always provided
//  by the user, and the port can be edited if the manifest indicated an
//  editable configuration.
//
tCIDLib::TVoid TCQCIPConnCfg::LoadFromXML(const TXMLTreeElement& xtnodeSrc)
{
    AssertElemName(xtnodeSrc, L"CQCCfg:IPConn");

    // Get the port type
    const TString& strProto = xtnodeSrc.xtattrNamed(L"CQCCfg:SockProto").strValue();

    if (strProto == L"SockProto_IP")
        m_eProto = tCIDSock::ESockProtos::IP;
    else if (strProto == L"SockProto_UDP")
        m_eProto = tCIDSock::ESockProtos::UDP;
    else if (strProto == L"SockProto_TCP")
        m_eProto = tCIDSock::ESockProtos::TCP;
    else
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcCfg_BadSockProto
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , strProto
        );
    }

    const TString& strAddrType = xtnodeSrc.xtattrNamed(L"CQCCfg:AddrType").strValue();
    if (strAddrType == L"IPV4")
        m_eAddrType = tCIDSock::EAddrTypes::IPV4;
    else if (strAddrType == L"IPV6")
        m_eAddrType = tCIDSock::EAddrTypes::IPV6;
    else if (strAddrType == L"Unspecified")
        m_eAddrType = tCIDSock::EAddrTypes::Unspec;
    else
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcCfg_BadAddrType
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , strAddrType
        );
    }

    // Store the port
    m_ippnTarget = xtnodeSrc.xtattrNamed(L"CQCCfg:Port").strValue().c4Val();

    // For a new name resolution next time
    m_bResolved = kCIDLib::False;
}


TString TCQCIPConnCfg::strConnInfo() const
{
    //
    //  Load up the message and replace the token with the connection info.
    //  We don't try to resolve it here if it hasn't been already. If it has
    //  we use that. Else we just use the raw values we were give.
    //
    if (m_bResolved)
        return TString(kKitMsgs::midConn_Type_IP, facCQCKit(), m_ipepTarget);

    return TString
    (
        kKitMsgs::midConn_Type_IP2, facCQCKit(), m_strAddr, TCardinal(m_ippnTarget)
    );
}


// ---------------------------------------------------------------------------
//  TCQCConnCfg: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Get or set the addres family type preference
tCIDSock::EAddrTypes TCQCIPConnCfg::eAddrType() const
{
    return m_eAddrType;
}

tCIDSock::EAddrTypes
TCQCIPConnCfg::eAddrType(const tCIDSock::EAddrTypes eToSet)
{
    // Force it to be resolved again
    m_bResolved = kCIDLib::False;

    m_eAddrType = eToSet;
    return m_eAddrType;
}



// Get the set the socket protocol to use
tCIDSock::ESockProtos TCQCIPConnCfg::eProto() const
{
    return m_eProto;
}

tCIDSock::ESockProtos
TCQCIPConnCfg::eProto(const tCIDSock::ESockProtos eToSet)
{
    // Force it to be resolved again
    m_bResolved = kCIDLib::False;

    m_eProto = eToSet;
    return m_eProto;
}


//
//  Return the end point. If we have not faulted it in yet, then do so,
//  which will force it to resolve.
//
const TIPEndPoint& TCQCIPConnCfg::ipepConn() const
{
    // If not resolved, try to create the end point
    if (!m_bResolved)
    {
        m_ipepTarget = TIPEndPoint(m_strAddr, m_ippnTarget, m_eAddrType);

        //
        //  It worked so mark it now resolved and can just continue to
        //  return what we have until something is changed.
        //
        m_bResolved = kCIDLib::True;
    }
    return m_ipepTarget;
}


tCIDLib::TIPPortNum TCQCIPConnCfg::ippnTarget() const
{
    return m_ippnTarget;
}

tCIDLib::TIPPortNum
TCQCIPConnCfg::ippnTarget(const tCIDLib::TIPPortNum ippnToSet)
{
    // This doesn't affect address resolution, no need to clear resolved flag
    m_ippnTarget = ippnToSet;
    return m_ippnTarget;
}


const TString& TCQCIPConnCfg::strAddr() const
{
    return m_strAddr;
}

const TString& TCQCIPConnCfg::strAddr(const TString& strToSet)
{
    // Force it to be resolved again
    m_bResolved = kCIDLib::False;

    m_strAddr = strToSet;
    return m_strAddr;
}


tCIDLib::TVoid
TCQCIPConnCfg::Set( const   TString&            strAddrToSet
                    , const tCIDLib::TIPPortNum ippnToSet)
{
    // Force it to be resolved again
    m_bResolved = kCIDLib::False;

    m_strAddr = strAddrToSet;
    m_ippnTarget = ippnToSet;
}


// ---------------------------------------------------------------------------
//  TCQCIPConnCfg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCIPConnCfg::StreamFrom(TBinInStream& strmToReadFrom)
{
    TParent::StreamFrom(strmToReadFrom);

    // Our stuff should start with a frame marker
    strmToReadFrom.CheckForFrameMarker(CID_FILE, CID_LINE);

    // Check our version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCKit_IPConnCfg::c2FmtVersion))
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

    strmToReadFrom  >> m_eProto
                    >> m_ippnTarget
                    >> m_strAddr;

    //
    //  If V2 or beyond, read in the address family type, else default it.
    //  We default to IPV4 for backwards compatability.
    //
    if (c2FmtVersion > 1)
        strmToReadFrom >> m_eAddrType;
    else
        m_eAddrType = tCIDSock::EAddrTypes::IPV4;

    // Check the start object marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);

    // For re-resolution
    m_bResolved = kCIDLib::False;
}

tCIDLib::TVoid TCQCIPConnCfg::StreamTo(TBinOutStream& strmToWriteTo) const
{
    TParent::StreamTo(strmToWriteTo);
    strmToWriteTo   << tCIDLib::EStreamMarkers::Frame
                    << CQCKit_IPConnCfg::c2FmtVersion
                    << m_eProto
                    << m_ippnTarget
                    << m_strAddr
                    << m_eAddrType
                    << tCIDLib::EStreamMarkers::EndObject;
}


