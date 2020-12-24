//
// FILE NAME: ZWaveUSB3Sh_DevInfoInfo.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 05/04/2018
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
//  The implementations of our classes that track the info we get from device info
//  files.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "ZWaveUSB3Sh_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TZWAssocInfo, TObject)
RTTIDecls(TZWCfgParam, TObject)
RTTIDecls(TZWClassInfo, TObject)
RTTIDecls(TZWXInfoItem, TObject)
RTTIDecls(TZWDevInfo, TObject)


// ---------------------------------------------------------------------------
//  Local types and constants
// ---------------------------------------------------------------------------
namespace ZWaveUSB3Sh_DevInfo
{
    // Persistent format versions for our classes
    const tCIDLib::TCard2   c2AssocFmtVersion   = 1;
    const tCIDLib::TCard2   c2CfgPFmtVersion    = 1;
    const tCIDLib::TCard2   c2ClassFmtVersion   = 1;
    const tCIDLib::TCard2   c2XInfoFmtVersion   = 1;
    const tCIDLib::TCard2   c2DevInfoFmtVersion = 1;
}



// ---------------------------------------------------------------------------
//   CLASS: TZWAssocInfo
//  PREFIX: associ
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWAssocInfo: Constructors and Destructor
// ---------------------------------------------------------------------------

// Set purposefully invalid values in case they get used by accident
TZWAssocInfo::TZWAssocInfo() :

    m_c1Group(0xFF)
    , m_c1EndPoint(0xFF)
{
}

TZWAssocInfo::TZWAssocInfo( const   TString&        strDesc
                            , const tCIDLib::TCard1 c1Group
                            , const tCIDLib::TCard1 c1EndPoint) :

    m_c1Group(c1Group)
    , m_c1EndPoint(c1EndPoint)
    , m_strDesc(strDesc)
{
    if ((m_c1EndPoint > 127) && (m_c1EndPoint != kCIDLib::c1MaxCard))
    {
        facZWaveUSB3Sh().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kZW3ShErrs::errcCfg_EPIdRange
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Range
            , TCardinal(m_c1EndPoint)
        );
    }
}

TZWAssocInfo::~TZWAssocInfo()
{
}


// ---------------------------------------------------------------------------
//  TZWAssocInfo: Public operators
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TZWAssocInfo::operator==(const TZWAssocInfo& associSrc) const
{
    if (&associSrc == this)
        return kCIDLib::True;

    return
    (
        (m_c1Group == associSrc.m_c1Group)
        && (m_c1EndPoint == associSrc.m_c1EndPoint)
        && (m_strDesc == associSrc.m_strDesc)
    );
}

tCIDLib::TBoolean TZWAssocInfo::operator!=(const TZWAssocInfo& associSrc) const
{
    return !operator==(associSrc);
}


// ---------------------------------------------------------------------------
//  TZWAssocInfo: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TZWAssocInfo::StreamFrom(TBinInStream& strmToReadFrom)
{
     // Make sure we get the start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion != ZWaveUSB3Sh_DevInfo::c2AssocFmtVersion))
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

    strmToReadFrom  >> m_c1Group
                    >> m_strDesc
                    >> m_c1EndPoint;;

    // Make sure we get the end marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}

tCIDLib::TVoid TZWAssocInfo::StreamTo(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << ZWaveUSB3Sh_DevInfo::c2AssocFmtVersion

                    << m_c1Group
                    << m_strDesc
                    << m_c1EndPoint

                    << tCIDLib::EStreamMarkers::EndObject;
}


// ---------------------------------------------------------------------------
//   CLASS: TZWCfgParam
//  PREFIX: associ
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWCfgParam: Constructors and Destructor
// ---------------------------------------------------------------------------

// Set invalid values in case they get used accidentally
TZWCfgParam::TZWCfgParam() :

    m_c1ParamNum(kCIDLib::c1MaxCard)
    , m_c4ByteCnt(64)
    , m_i4Value(0)
{
}

TZWCfgParam::TZWCfgParam(const  TString&        strDesc
                        , const tCIDLib::TCard1 c1ParamNum
                        , const tCIDLib::TInt4  i4Value
                        , const tCIDLib::TCard4 c4ByteCnt) :

    m_c1ParamNum(c1ParamNum)
    , m_c4ByteCnt(c4ByteCnt)
    , m_i4Value(i4Value)
    , m_strDesc(strDesc)
{
}

TZWCfgParam::~TZWCfgParam()
{
}


// ---------------------------------------------------------------------------
//  TZWCfgParam: Public operators
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TZWCfgParam::operator==(const TZWCfgParam& cfgpSrc) const
{
    if (&cfgpSrc == this)
        return kCIDLib::True;

    return
    (
        (m_c1ParamNum == cfgpSrc.m_c1ParamNum)
        && (m_c4ByteCnt == cfgpSrc.m_c4ByteCnt)
        && (m_i4Value == cfgpSrc.m_i4Value)
        && (m_strDesc == cfgpSrc.m_strDesc)
    );
}

tCIDLib::TBoolean TZWCfgParam::operator!=(const TZWCfgParam& cfgpSrc) const
{
    return !operator==(cfgpSrc);
}


// ---------------------------------------------------------------------------
//  TZWCfgParam: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TZWCfgParam::StreamFrom(TBinInStream& strmToReadFrom)
{
     // Make sure we get the start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion != ZWaveUSB3Sh_DevInfo::c2CfgPFmtVersion))
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

    strmToReadFrom  >> m_c1ParamNum
                    >> m_strDesc
                    >> m_i4Value
                    >> m_c4ByteCnt;

    // Make sure we get the end marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}

tCIDLib::TVoid TZWCfgParam::StreamTo(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << ZWaveUSB3Sh_DevInfo::c2CfgPFmtVersion

                    << m_c1ParamNum
                    << m_strDesc
                    << m_i4Value
                    << m_c4ByteCnt

                    << tCIDLib::EStreamMarkers::EndObject;
}



// ---------------------------------------------------------------------------
//   CLASS: TZWClassInfo
//  PREFIX: zwci
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWClassInfo: Constructors and Destructor
// ---------------------------------------------------------------------------
TZWClassInfo::TZWClassInfo() :

    m_bSecure(kCIDLib::False)
    , m_c4Version(1)
    , m_eClass(tZWaveUSB3Sh::ECClasses::Count)
{
}

//
//  When we first find out about a class being supported via discovery, this is
//  all we know.
//
TZWClassInfo::TZWClassInfo(const tZWaveUSB3Sh::ECClasses eClass) :

    m_bSecure(kCIDLib::False)
    , m_c4Version(1)
    , m_eClass(eClass)
{
}

TZWClassInfo::TZWClassInfo( const   tZWaveUSB3Sh::ECClasses     eClass
                            , const tCIDLib::TCard4             c4Ver
                            , const tCIDLib::TBoolean           bSecure
                            , const tZWaveUSB3Sh::TXInfoList&   colExtraInfo) :
    m_bSecure(bSecure)
    , m_c4Version(c4Ver)
    , m_colExtraInfo(colExtraInfo)
    , m_eClass(eClass)
{
}

TZWClassInfo::~TZWClassInfo()
{
}


// ---------------------------------------------------------------------------
//  TZWClassInfo: Public operators
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TZWClassInfo::operator==(const TZWClassInfo& zwciSrc) const
{
    return
    (
        (m_bSecure == zwciSrc.m_bSecure)
        && (m_c4Version == zwciSrc.m_c4Version)
        && (m_colExtraInfo == zwciSrc.m_colExtraInfo)
        && (m_eClass == zwciSrc.m_eClass)
    );
}

tCIDLib::TBoolean TZWClassInfo::operator!=(const TZWClassInfo& zwciSrc) const
{
    return !operator==(zwciSrc);
}



// ---------------------------------------------------------------------------
//  TZWClassInfo: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TZWClassInfo::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Make sure we get the start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion != ZWaveUSB3Sh_DevInfo::c2ClassFmtVersion))
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

    strmToReadFrom  >> m_eClass
                    >> m_c4Version
                    >> m_colExtraInfo
                    >> m_bSecure;

    // Make sure we get the end marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TZWClassInfo::StreamTo(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << ZWaveUSB3Sh_DevInfo::c2ClassFmtVersion;

    strmToWriteTo   << m_eClass
                    << m_c4Version
                    << m_colExtraInfo
                    << m_bSecure
                    << tCIDLib::EStreamMarkers::EndObject;
}



// ---------------------------------------------------------------------------
//   CLASS: TZWXInfoItem
//  PREFIX: xii
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWXInfoItem: Constructors and Destructor
// ---------------------------------------------------------------------------
TZWXInfoItem::TZWXInfoItem() :

    m_c1EPId(0xFF)
{
}

TZWXInfoItem::TZWXInfoItem( const   TString&        strKey
                            , const TString&        strValue
                            , const tCIDLib::TCard1 c1EPId) :

    m_c1EPId(c1EPId)
    , m_strKey(strKey)
    , m_strValue(strValue)
{
}

TZWXInfoItem::~TZWXInfoItem()
{
}


// ---------------------------------------------------------------------------
//  TZWXInfoItem: Public operators
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TZWXInfoItem::operator==(const TZWXInfoItem& xiiSrc) const
{
    if (this == &xiiSrc)
        return kCIDLib::True;

    return
    (
        (m_strValue == xiiSrc.m_strValue)
        && (m_strKey == xiiSrc.m_strKey)
        && (m_c1EPId == xiiSrc.m_c1EPId)
    );
}

tCIDLib::TBoolean TZWXInfoItem::operator!=(const TZWXInfoItem& xiiSrc) const
{
    return !operator==(xiiSrc);
}



// ---------------------------------------------------------------------------
//  TZWXInfoItem: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TZWXInfoItem::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Make sure we get the start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion != ZWaveUSB3Sh_DevInfo::c2XInfoFmtVersion))
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

    strmToReadFrom  >> m_strKey
                    >> m_c1EPId
                    >> m_strValue;

    // Make sure we get the end marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);

}

tCIDLib::TVoid TZWXInfoItem::StreamTo(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << ZWaveUSB3Sh_DevInfo::c2XInfoFmtVersion

                    << m_strKey
                    << m_c1EPId
                    << m_strValue

                    << tCIDLib::EStreamMarkers::EndObject;
}



// ---------------------------------------------------------------------------
//   CLASS: TZWDevInfo
//  PREFIX: unit
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWDevInfo: Public, static methods
// ---------------------------------------------------------------------------

tCIDLib::ESortComps
TZWDevInfo::eCompByManIds(  const   TZWDevInfo& devi1
                            , const TZWDevInfo& devi2)
{
    if (devi1.m_c8ManIds < devi2.m_c8ManIds)
        return tCIDLib::ESortComps::FirstLess;
    else if (devi1.m_c8ManIds > devi2.m_c8ManIds)
        return tCIDLib::ESortComps::FirstGreater;
    return tCIDLib::ESortComps::Equal;
}


// ---------------------------------------------------------------------------
//  TZWDevInfo: Constructors and destructor
// ---------------------------------------------------------------------------

TZWDevInfo::TZWDevInfo() :

    m_c4WakeupInt(0)
{
    Reset(kCIDLib::False);
}

TZWDevInfo::~TZWDevInfo()
{
}


// ---------------------------------------------------------------------------
//  TZWDevInfo: Public operators
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TZWDevInfo::operator==(const TZWDevInfo& deviSrc) const
{
    if (this == &deviSrc)
        return kCIDLib::True;

    //
    //  Put stuff first that is most likely not be the same, which would be unit
    //  specific stuff.
    //
    return
    (
        (m_c8ManIds == deviSrc.m_c8ManIds)
        && (m_colExtraInfo == deviSrc.m_colExtraInfo)
        && (m_c4WakeupInt == deviSrc.m_c4WakeupInt)
        && (m_colAssociations == deviSrc.m_colAssociations)
        && (m_colCfgParams == deviSrc.m_colCfgParams)
        && (m_colCCs == deviSrc.m_colCCs)
        && (m_strMake == deviSrc.m_strMake)
        && (m_strModel == deviSrc.m_strModel)
        && (m_strUnitClass == deviSrc.m_strUnitClass)
    );
}

tCIDLib::TBoolean TZWDevInfo::operator!=(const TZWDevInfo& deviSrc) const
{
    return !operator==(deviSrc);
}


// ---------------------------------------------------------------------------
//  TZWDevInfo: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  A convenience to look through our list of extra key/values for a particular one.
//  This is for unit level info. There is a separate one for per-class stuff.
//
tCIDLib::TBoolean
TZWDevInfo::bFindExtraVal(const TString& strKey, TString& strValue) const
{
    const tCIDLib::TCard4 c4Count = m_colExtraInfo.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TZWXInfoItem& xiiCur = m_colExtraInfo[c4Index];
        if (xiiCur.m_strKey.bCompareI(strKey))
        {
            strValue = xiiCur.m_strValue;
            return kCIDLib::True;
        }
    }
    return kCIDLib::False;
}


// Same as above but we check the list of extra values for a specific class
tCIDLib::TBoolean
TZWDevInfo::bFindExtraVal( const   tZWaveUSB3Sh::ECClasses  eClass
                            , const tCIDLib::TCard1         c1EPId
                            , const TString&                strKey
                            ,       TString&                strValue) const
{
    const tCIDLib::TCard4 c4Count = m_colCCs.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TZWClassInfo& zwciCur = m_colCCs[c4Index];
        if (zwciCur.m_eClass == eClass)
        {
            // Search this guy's values
            const tCIDLib::TCard4 c4CCount = zwciCur.m_colExtraInfo.c4ElemCount();
            for (tCIDLib::TCard4 c4CIndex = 0; c4CIndex < c4CCount; c4CIndex++)
            {
                const TZWXInfoItem& xiiCur = zwciCur.m_colExtraInfo[c4CIndex];
                if ((xiiCur.m_strKey.bCompareI(strKey))
                &&  (xiiCur.m_c1EPId == c1EPId))
                {
                    strValue = xiiCur.m_strValue;
                    return kCIDLib::True;
                }
            }

            // Never found the key
            return kCIDLib::False;
        }
    }

    // Never found the class
    return kCIDLib::False;
}


// See if we have been loaded from an device info file yet
tCIDLib::TBoolean TZWDevInfo::bIsValid() const
{
    return
    (
        (m_c8ManIds != 0)
        && !m_strMake.bIsEmpty()
        && !m_strModel.bIsEmpty()
        && !m_strUnitClass.bIsEmpty()
    );
}



// Returns true if we support the passed class and it is marked as secure
tCIDLib::TBoolean TZWDevInfo::bSecureClass(const tZWaveUSB3Sh::ECClasses eClass) const
{
    tCIDLib::TCard4 c4At = c4FindCCIndex(eClass);
    if (c4At == kCIDLib::c4MaxCard)
        return kCIDLib::False;

    return m_colCCs[c4At].m_bSecure;
}


//
//  Returns true if the passed class is in our list. And we have another version
//  that returns true if we have the class and our version number for it is equal
//  to or greater than the passed version.
//
tCIDLib::TBoolean TZWDevInfo::bSupportsClass(const tZWaveUSB3Sh::ECClasses eClass) const
{
    return (c4FindCCIndex(eClass) != kCIDLib::c4MaxCard);
}

tCIDLib::TBoolean
TZWDevInfo::bSupportsClass(const   tZWaveUSB3Sh::ECClasses  eClass
                            , const tCIDLib::TCard4         c4Version) const
{
    tCIDLib::TCard4 c4At = c4FindCCIndex(eClass);
    if (c4At == kCIDLib::c4MaxCard)
        return kCIDLib::False;

    return (m_colCCs[c4At].m_c4Version >= c4Version);
}


//
//  Return the version of a class that we support. If we don't support this class we
//  either return 1 or throw.
//
tCIDLib::TCard4
TZWDevInfo::c4ClassVersion(const    tZWaveUSB3Sh::ECClasses eClass
                            , const tCIDLib::TBoolean       bThrowIfNot
                            , const TString&                strName) const
{
    tCIDLib::TCard4 c4At = c4FindCCIndex(eClass);
    if (c4At == kCIDLib::c4MaxCard)
    {
        if (bThrowIfNot)
        {
            facZWaveUSB3Sh().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kZW3ShErrs::errcUnit_ClassNotSup
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::NotSupported
                , strName
                , tZWaveUSB3Sh::strXlatECClasses(eClass)
            );
        }
        return 1;
    }

    return m_colCCs[c4At].m_c4Version;
}


tCIDLib::TCard4 TZWDevInfo::c4FindCCIndex(const tZWaveUSB3Sh::ECClasses eFind) const
{
    const tCIDLib::TCard4 c4Count = m_colCCs.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (m_colCCs[c4Index].m_eClass == eFind)
            return c4Index;
    }
    return kCIDLib::c4MaxCard;
}


// Return our man ids, which could be zero if we have not bene loaded yet
tCIDLib::TCard8 TZWDevInfo::c8ManIds() const
{
    return m_c8ManIds;
}


// Formats out our auto-config steps
tCIDLib::TVoid TZWDevInfo::FormatAutoCfg(TTextOutStream& strmTar) const
{
    tCIDLib::TCard4 c4Count;

    strmTar << L"Add Driver to Association Groups:\n";
    c4Count = m_colAssociations.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TZWAssocInfo& associCur = m_colAssociations[c4Index];
        strmTar << L"    " << associCur.m_strDesc
                << L" (# " << associCur.m_c1Group;
        if (associCur.m_c1EndPoint != 0xFF)
            strmTar << L", EndPnt# " << associCur.m_c1EndPoint;
        strmTar << L")\n";
    }

    strmTar << L"\n\nSet Configuration Parameters:\n";
    c4Count = m_colCfgParams.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TZWCfgParam& cfgpCur = m_colCfgParams[c4Index];

        strmTar << L"    " << cfgpCur.m_strDesc
                << L" (# " << cfgpCur.m_c1ParamNum
                << L", Bytes: " << cfgpCur.m_c4ByteCnt
                << L", Value: " << cfgpCur.m_i4Value
                << L")\n";
    }

    strmTar.Flush();
}


//
//  This is called to let us parse our content out of the XML based device info file
//  content.
//
tCIDLib::TVoid TZWDevInfo::ParseFrom(const TXMLTreeElement& xtnodeDev)
{
    tCIDLib::TCard4 c4At;

    // Clear the lists that we load here, so that they will not accumulate
    m_colAssociations.RemoveAll();
    m_colCfgParams.RemoveAll();
    m_colCCs.RemoveAll();
    m_colExtraInfo.RemoveAll();

    try
    {
        m_strMake = xtnodeDev.xtattrNamed(L"Make").strValue();
        m_strModel = xtnodeDev.xtattrNamed(L"Model").strValue();
        m_strUnitClass = xtnodeDev.xtattrNamed(L"UnitClass").strValue();
        m_c8ManIds = xtnodeDev.xtattrNamed(L"ManIds").c8ValueAs(tCIDLib::ERadices::Hex);

        // Do the classes
        tCIDLib::TCard4 c4Count;
        {
            const TXMLTreeElement& xtnodeCCs = xtnodeDev.xtnodeFindElement
            (
                L"CmdClasses", 0, c4At
            );
            c4Count = xtnodeCCs.c4ChildCount();
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            {
                // Skip non-elements
                if (xtnodeCCs.eChildTypeAt(c4Index) != tCIDXML::ENodeTypes::Element)
                    continue;

                const TXMLTreeElement& xtnodeCC = xtnodeCCs.xtnodeChildAtAsElement(c4Index);
                const TXMLTreeAttr& xtattrName = xtnodeCC.xtattrNamed(L"Name");
                const TXMLTreeAttr& xtattrVer = xtnodeCC.xtattrNamed(L"Version");
                const TXMLTreeAttr& xtattrSec = xtnodeCC.xtattrNamed(L"Secure");

                TZWClassInfo zwciCur;
                zwciCur.m_bSecure = xtattrSec.bValueAs();
                zwciCur.m_c4Version = xtattrVer.c4ValueAs();
                zwciCur.m_eClass = tZWaveUSB3Sh::eXlatECClasses(xtattrName.strValue());

                // See if there's any extra info and parse that out if so
                const TXMLTreeElement* pxtnodeExtra = xtnodeCC.pxtnodeFindElement
                (
                    L"ExtraInfo", 0, c4At
                );
                if (pxtnodeExtra)
                {
                    const tCIDLib::TCard4 c4XCount = pxtnodeExtra->c4ChildCount();
                    for (tCIDLib::TCard4 c4XIndex = 0; c4XIndex < c4XCount; c4XIndex++)
                    {
                        // Skip non-elements
                        if (pxtnodeExtra->eChildTypeAt(c4XIndex) != tCIDXML::ENodeTypes::Element)
                            continue;

                        const TXMLTreeElement& xtnodeCur = pxtnodeExtra->xtnodeChildAtAsElement(c4XIndex);

                        const TXMLTreeAttr& xtattrKey = xtnodeCur.xtattrNamed(L"Name");
                        const TXMLTreeAttr& xtattrValue = xtnodeCur.xtattrNamed(L"Value");
                        const TXMLTreeAttr& xtattrEndPnt = xtnodeCur.xtattrNamed(L"EndPnt");
                        zwciCur.m_colExtraInfo.objAdd
                        (
                            TZWXInfoItem
                            (
                                xtattrKey.strValue()
                                , xtattrValue.strValue()
                                , xtattrEndPnt.c1ValueAs()
                            )
                        );
                    }
                }

                m_colCCs.objAdd(zwciCur);
            }
        }

        // Parse out the auto-config steps and store those
        {
            const TXMLTreeElement* pxtnodeACs = xtnodeDev.pxtnodeFindElement
            (
                L"AutoCfg", 0, c4At
            );
            if (pxtnodeACs)
            {
                // Get the wakeup interval value
                const TXMLTreeAttr& xtattrWI = pxtnodeACs->xtattrNamed(L"WI");
                if (!xtattrWI.strValue().bToCard4(m_c4WakeupInt, tCIDLib::ERadices::Auto))
                    m_c4WakeupInt = 0;

                c4Count = pxtnodeACs->c4ChildCount();
                for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
                {
                    // Skip non-elements
                    if (pxtnodeACs->eChildTypeAt(c4Index) != tCIDXML::ENodeTypes::Element)
                        continue;

                    // It's a mixture of types, so check the Q name
                    const TXMLTreeElement& xtnodeCur
                    (
                        pxtnodeACs->xtnodeChildAtAsElement(c4Index)
                    );

                    if (xtnodeCur.strQName() == L"Grp")
                    {
                        const TXMLTreeAttr& xtattrNum = xtnodeCur.xtattrNamed(L"N");
                        const TXMLTreeAttr& xtattrDescr = xtnodeCur.xtattrNamed(L"D");
                        const TXMLTreeAttr& xtattrEP = xtnodeCur.xtattrNamed(L"EP");
                        m_colAssociations.objAdd
                        (
                            TZWAssocInfo
                            (
                                xtattrDescr.strValue()
                                , xtattrNum.c1ValueAs(tCIDLib::ERadices::Auto)
                                , xtattrEP.c1ValueAs(tCIDLib::ERadices::Auto)
                            )
                        );
                    }
                     else if (xtnodeCur.strQName() == L"CfgParm")
                    {
                        const TXMLTreeAttr& xtattrNum = xtnodeCur.xtattrNamed(L"N");
                        const TXMLTreeAttr& xtattrVal = xtnodeCur.xtattrNamed(L"V");
                        const TXMLTreeAttr& xtattrCnt = xtnodeCur.xtattrNamed(L"C");
                        const TXMLTreeAttr& xtattrDescr = xtnodeCur.xtattrNamed(L"D");
                        m_colCfgParams.objAdd
                        (
                            TZWCfgParam
                            (
                                xtattrDescr.strValue()
                                , xtattrNum.c1ValueAs(tCIDLib::ERadices::Auto)
                                , xtattrVal.i4ValueAs()
                                , xtattrCnt.c4ValueAs(tCIDLib::ERadices::Auto)
                            )
                        );
                    }
                     else
                    {
                        // Don't know what this is
                    }
                }
            }
        }

        //
        //  Parse out the extra info. We store them as key/value pairs, where the key is
        //  based on the attribute name, and the value is the attribute value.
        //
        {
            const TXMLTreeElement* pxtnodeExtra = xtnodeDev.pxtnodeFindElement
            (
                L"ExtraInfo", 0, c4At
            );
            if (pxtnodeExtra)
            {
                c4Count = pxtnodeExtra->c4ChildCount();
                for (tCIDLib::TCard4 c4XIndex = 0; c4XIndex < c4Count; c4XIndex++)
                {
                    // Skip non-elements
                    if (pxtnodeExtra->eChildTypeAt(c4XIndex) != tCIDXML::ENodeTypes::Element)
                        continue;

                    const TXMLTreeElement& xtnodeCur = pxtnodeExtra->xtnodeChildAtAsElement(c4XIndex);

                    // No end point on these since they are unit level
                    const TXMLTreeAttr& xtattrKey = xtnodeCur.xtattrNamed(L"Name");
                    const TXMLTreeAttr& xtattrValue = xtnodeCur.xtattrNamed(L"Value");
                    m_colExtraInfo.objAdd
                    (
                        TZWXInfoItem(xtattrKey.strValue(), xtattrValue.strValue(), 0xFF)
                    );
                }
            }
        }
    }

    catch(TError& errToCatch)
    {
        Reset(kCIDLib::False);
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}


// Reset us back to defaults
tCIDLib::TVoid TZWDevInfo::Reset(const tCIDLib::TBoolean bRescan)
{
    m_c4WakeupInt = 0;
    m_c8ManIds = 0;
    m_colAssociations.RemoveAll();
    m_colCfgParams.RemoveAll();
    m_colExtraInfo.RemoveAll();
    m_strMake.Clear();
    m_strModel.Clear();
    m_strUnitClass.Clear();

    // If not a rescan type, remove the CCs
    if (!bRescan)
        m_colCCs.RemoveAll();
}


//
//  We use this class to stream back gathered device info to the client when he
//  requests info on a unit. This isn't all the device info, since that can only be
//  known by a device info file. This is just the stuff we can discover from the
//  unit itself. This is a convenient class to return the info in and there may
//  be other discovered info to return in the future.
//
tCIDLib::TVoid
TZWDevInfo::Set(const   tCIDLib::TCard8             c8ManIds
                , const tZWaveUSB3Sh::TCInfoList&   colClasses)
{
    Reset(kCIDLib::False);

    m_c8ManIds = c8ManIds;
    m_colCCs = colClasses;
}


// ---------------------------------------------------------------------------
//  TZWDevInfo: Public operators
// ---------------------------------------------------------------------------
tCIDLib::TVoid TZWDevInfo::FormatTo(TTextOutStream& strmDest) const
{
    TStreamFmt strmfName(14, 0, tCIDLib::EHJustify::Right);
    TStreamFmt strmfValue(0, 0, tCIDLib::EHJustify::Left);

    strmDest    << strmfName << L"Make: " << strmfValue << m_strMake << kCIDLib::NewLn
                << strmfName << L"Model: " << strmfValue << m_strModel << kCIDLib::NewLn;

    strmDest    << strmfName << L"ManIds: " << strmfValue;
    strmDest    << TCardinal(tCIDLib::TCard2(m_c8ManIds >> 32), tCIDLib::ERadices::Hex)
                << kCIDLib::chSpace
                << TCardinal(tCIDLib::TCard2(m_c8ManIds >> 16), tCIDLib::ERadices::Hex)
                << kCIDLib::chSpace
                << TCardinal(tCIDLib::TCard2(m_c8ManIds), tCIDLib::ERadices::Hex)
                << kCIDLib::NewLn;

    if (!m_strUnitClass.bIsEmpty())
        strmDest << strmfName << L"CppClass: " << strmfValue << m_strUnitClass  << kCIDLib::NewLn;

    strmDest << strmfName << L"Classes: \n";
    TString strVal;
    tCIDLib::TCard4 c4Count = m_colCCs.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        strmDest    << strmfName << L" "
                    << strmfValue << m_colCCs[c4Index].m_eClass
                    << L" (" << m_colCCs[c4Index].m_c4Version;
        if (m_colCCs[c4Index].m_bSecure)
            strmDest << L", Secure";
        strmDest    << L")\n";
    }

    strmDest << strmfName << L"\nExtra Info: \n";
    c4Count = m_colExtraInfo.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TZWXInfoItem& xiiCur = m_colExtraInfo[c4Index];
        strmDest    << strmfName << xiiCur.m_strKey << kCIDLib::chEquals
                    << strmfValue << xiiCur.m_strValue
                    << kCIDLib::NewLn;
    }

    strmDest.Flush();
}


tCIDLib::TVoid TZWDevInfo::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Make sure we get the start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion != ZWaveUSB3Sh_DevInfo::c2DevInfoFmtVersion))
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

    strmToReadFrom  >> m_colAssociations
                    >> m_colCCs
                    >> m_colExtraInfo
                    >> m_c4WakeupInt
                    >> m_colCfgParams
                    >> m_strMake
                    >> m_c8ManIds
                    >> m_strModel
                    >> m_strUnitClass;

    // Make sure we get the end marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TZWDevInfo::StreamTo(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << ZWaveUSB3Sh_DevInfo::c2DevInfoFmtVersion

                    << m_colAssociations
                    << m_colCCs
                    << m_colExtraInfo
                    << m_c4WakeupInt
                    << m_colCfgParams
                    << m_strMake
                    << m_c8ManIds
                    << m_strModel
                    << m_strUnitClass

                    << tCIDLib::EStreamMarkers::EndObject;
}

