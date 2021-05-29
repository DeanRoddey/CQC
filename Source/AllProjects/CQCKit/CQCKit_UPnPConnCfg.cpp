//
// FILE NAME: CQCKit_UPnPConnCfg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/19/2011
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
//  Implements the simple driver connection configuration derivative for
//  UPnP devices.
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
AdvRTTIDecls(TCQCUPnPConnCfg,TCQCConnCfg)


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace
{
    namespace CQCKit_UPnPConnCfg
    {
        // -----------------------------------------------------------------------
        //  Our current persistent format version
        //
        //  Version 2 -
        //      Added support for an optional service type/variable name that
        //      can be indicated for the driver install wizard to use to get the
        //      displayed names, instead of just using the device's friendly name.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2       c2FmtVersion = 2;
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCUPnPConnCfg
//  PREFIX: ccfg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCUPnPConnCfg: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCUPnPConnCfg::TCQCUPnPConnCfg()
{
}

TCQCUPnPConnCfg::~TCQCUPnPConnCfg()
{
}


// ---------------------------------------------------------------------------
//  TCQCUPnPConnCfg: Public operators
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TCQCUPnPConnCfg::operator==(const TCQCUPnPConnCfg& conncfgToComp) const
{
    return (m_strDevName == conncfgToComp.m_strDevName)
            && (m_strDevType == conncfgToComp.m_strDevType)
            && (m_strDevUID == conncfgToComp.m_strDevUID)
            && (m_strTypeDescr == conncfgToComp.m_strTypeDescr)
            && (m_strDispSvcType == conncfgToComp.m_strDispSvcType)
            && (m_strDispVarName == conncfgToComp.m_strDispVarName);
}



// ---------------------------------------------------------------------------
//  TCQCUPnPConnCfg: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TCQCUPnPConnCfg::bIsSame(const TCQCConnCfg& conncfgToCompare) const
{
    // See if they are of the same polymorphic type. If not, can't be equal
    if (clsThis() != conncfgToCompare.clsIsA())
        return kCIDLib::False;

    //
    //  They are, so compare them. We cast the other one to our type and
    //  then we can just do an equality test.
    //
    const TCQCUPnPConnCfg& conncfgUPnP = static_cast<const TCQCUPnPConnCfg&>
    (
        conncfgToCompare
    );
    return (*this == conncfgUPnP);
}


//
//  This lets us take the config in a formatted string, which is used by
//  some of the test harness programs. For us it's a space separated string
//  with these values:
//
//  devtype devuid typedescr
//
//  When doing it this way we don't bother with the device name. The service
//  type and variable name are optional and only used in the driver install
//  wizard so we can just empty them out. Anything after the device UID is
//  considered the type description.
//
//  The device type URI and device UID can't have spaces in them legally so
//  we don't need anything fancier to reliably parse it.
//
tCIDLib::TBoolean TCQCUPnPConnCfg::bParseConnInfo(const TString& strInfo)
{
    m_strDevType = strInfo;
    if (!m_strDevType.bSplit(m_strDevUID, L' '))
        return kCIDLib::False;
    if (!m_strDevUID.bSplit(m_strTypeDescr, L' '))
        return kCIDLib::False;

    if (m_strDevType.bIsEmpty() || m_strDevUID.bIsEmpty() || m_strTypeDescr.bIsEmpty())
        return kCIDLib::False;

    m_strDevName = L"Unknown";
    m_strDispSvcType.Clear();
    m_strDispVarName.Clear();

    return kCIDLib::True;
}


tCIDLib::TVoid TCQCUPnPConnCfg::LoadFromXML(const TXMLTreeElement& xtnodeSrc)
{
    AssertElemName(xtnodeSrc, L"CQCCfg:UPnPConn")

    m_strDevName = xtnodeSrc.xtattrNamed(L"CQCCfg:DevName").strValue();
    m_strDevType = xtnodeSrc.xtattrNamed(L"CQCCfg:DevType").strValue();
    m_strDevUID = xtnodeSrc.xtattrNamed(L"CQCCfg:DevUID").strValue();
    m_strTypeDescr = xtnodeSrc.xtattrNamed(L"CQCCfg:TypeDescr").strValue();

    // If the display info child element is there, deal with it
    if (xtnodeSrc.c4ChildCount())
    {
        const TXMLTreeElement& xtnodeDisp
                                = xtnodeSrc.xtnodeChildAtAsElement(0);
        AssertElemName(xtnodeDisp, L"CQCCfg:UPnPConnDisp")

        m_strDispSvcType = xtnodeDisp.xtattrNamed(L"CQCCfg:SvcType").strValue();
        m_strDispVarName = xtnodeDisp.xtattrNamed(L"CQCCfg:VarName").strValue();
    }
     else
    {
        m_strDispSvcType.Clear();
        m_strDispVarName.Clear();
    }
}


//
//  We don't format out the optional display info. That's only used by
//  the driver install wizard.
//
TString TCQCUPnPConnCfg::strConnInfo() const
{
    TString strRet
    (
        m_strDevType.c4Length()
        + m_strDevUID.c4Length()
        + m_strTypeDescr.c4Length()
        + 3
    );

    // It's the type and UID separated by a space, then the description
    strRet = (m_strDevType);
    strRet.Append(L" ");
    strRet.Append(m_strDevUID);
    strRet.Append(L" ");
    strRet.Append(m_strTypeDescr);

    return strRet;
}



// ---------------------------------------------------------------------------
//  TCQCUPnPConnCfg: Public, non-virtual methods
// ---------------------------------------------------------------------------
const TString& TCQCUPnPConnCfg::strDevName() const
{
    return m_strDevName;
}

const TString& TCQCUPnPConnCfg::strDevName(const TString& strToSet)
{
    m_strDevName = strToSet;
    return m_strDevName;
}


const TString& TCQCUPnPConnCfg::strDevType() const
{
    return m_strDevType;
}


const TString& TCQCUPnPConnCfg::strDevUID() const
{
    return m_strDevUID;
}

const TString& TCQCUPnPConnCfg::strDevUID(const TString& strToSet)
{
    m_strDevUID = strToSet;
    return m_strDevUID;
}

const TString& TCQCUPnPConnCfg::strDispSvcType() const
{
    return m_strDispSvcType;
}

const TString& TCQCUPnPConnCfg::strDispVarName() const
{
    return m_strDispVarName;
}


const TString& TCQCUPnPConnCfg::strTypeDescr() const
{
    return m_strTypeDescr;
}


// ---------------------------------------------------------------------------
//  TCQCUPnPConnCfg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCUPnPConnCfg::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Do our parent first
    TParent::StreamFrom(strmToReadFrom);

    // Our stuff should start with a frame marker
    strmToReadFrom.CheckForFrameMarker(CID_FILE, CID_LINE);

    // Check our version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCKit_UPnPConnCfg::c2FmtVersion))
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

    strmToReadFrom  >> m_strDevName
                    >> m_strDevType
                    >> m_strDevUID
                    >> m_strTypeDescr;

    // If V2 or beyond, read in the optional display stuff, else clear them
    if (c2FmtVersion > 1)
    {
        strmToReadFrom  >> m_strDispSvcType
                        >> m_strDispVarName;
    }
     else
    {
        m_strDispSvcType.Clear();
        m_strDispVarName.Clear();
    }

    // Check the start object marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}

tCIDLib::TVoid TCQCUPnPConnCfg::StreamTo(TBinOutStream& strmToWriteTo) const
{
    TParent::StreamTo(strmToWriteTo);
    strmToWriteTo   << tCIDLib::EStreamMarkers::Frame
                    << CQCKit_UPnPConnCfg::c2FmtVersion
                    << m_strDevName
                    << m_strDevType
                    << m_strDevUID
                    << m_strTypeDescr
                    << m_strDispSvcType
                    << m_strDispVarName
                    << tCIDLib::EStreamMarkers::EndObject;
}

