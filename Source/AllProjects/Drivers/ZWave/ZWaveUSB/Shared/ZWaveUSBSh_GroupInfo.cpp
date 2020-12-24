//
// FILE NAME: ZWaveUSBSh_GroupInfo.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/20/2005
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
//  This file provides the implementation for the TZWaveGroupInfo class, which
//  represents the info we need to maintain for the groups we are controlling.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "ZWaveUSBSh_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TZWaveGroupInfo,TCQCServerBase)


// ---------------------------------------------------------------------------
//  Local types and constants
// ---------------------------------------------------------------------------
namespace ZWaveUSBSh_GroupInfo
{
    // -----------------------------------------------------------------------
    //  Our persistent format version
    // -----------------------------------------------------------------------
    const tCIDLib::TCard2   c2FmtVersion = 1;
}



// ---------------------------------------------------------------------------
//   CLASS: TZWaveGroupInfo
//  PREFIX: grpi
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWaveGroupInfo: Public, static methods
// ---------------------------------------------------------------------------
tCIDLib::ESortComps
TZWaveGroupInfo::eCompByName(const  TZWaveGroupInfo& grpi1
                            , const TZWaveGroupInfo& grpi2
                            , const tCIDLib::TCard4)
{
    return grpi1.m_strName.eCompare(grpi2.m_strName);
}


// ---------------------------------------------------------------------------
//  TZWaveGroupInfo: Constructors and Destructor
// ---------------------------------------------------------------------------
TZWaveGroupInfo::TZWaveGroupInfo() :

    m_c4FldId(kCIDLib::c4MaxCard)
    , m_fcolUnits(16)
    , m_strName(L"NewGroup")
{
    // Generate a unique id for this guy
    m_strId = ::TUniqueId::strMakeId();
}

TZWaveGroupInfo::TZWaveGroupInfo(const TString& strName) :

    m_c4FldId(kCIDLib::c4MaxCard)
    , m_fcolUnits(16)
    , m_strName(strName)
{
    // Generate a unique id for this guy
    m_strId = ::TUniqueId::strMakeId();
}

TZWaveGroupInfo::TZWaveGroupInfo(const TZWaveGroupInfo& grpiToCopy) :

    m_c4FldId(grpiToCopy.m_c4FldId)
    , m_fcolUnits(grpiToCopy.m_fcolUnits)
    , m_strId(grpiToCopy.m_strId)
    , m_strName(grpiToCopy.m_strName)
{
}

TZWaveGroupInfo::~TZWaveGroupInfo()
{
}


// ---------------------------------------------------------------------------
//  TZWaveGroupInfo: Public operators
// ---------------------------------------------------------------------------
TZWaveGroupInfo&
TZWaveGroupInfo::operator=(const TZWaveGroupInfo& grpiSrc)
{
    if (this != &grpiSrc)
    {
        m_c4FldId   = grpiSrc.m_c4FldId;
        m_fcolUnits = grpiSrc.m_fcolUnits;
        m_strId     = grpiSrc.m_strId;
        m_strName   = grpiSrc.m_strName;
    }
    return *this;
}


tCIDLib::TBoolean TZWaveGroupInfo::operator==(const TZWaveGroupInfo& grpiSrc) const
{
    // Field id is runtime only so don't need to compare that
    return
    (
        (m_strId == grpiSrc.m_strId)
        && (m_strName == grpiSrc.m_strName)
        && (m_fcolUnits == grpiSrc.m_fcolUnits)
    );
}

tCIDLib::TBoolean TZWaveGroupInfo::operator!=(const TZWaveGroupInfo& grpiSrc) const
{
    return !operator==(grpiSrc);
}



// ---------------------------------------------------------------------------
//  TZWaveGroupInfo: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TZWaveGroupInfo::AddUnit(const tCIDLib::TCard1 c1Id)
{
    // Add it if not already in the list
    if (!m_fcolUnits.bElementPresent(c1Id))
        m_fcolUnits.c4AddElement(c1Id);
}


// Returns true if the passed unit id is in this group
tCIDLib::TBoolean
TZWaveGroupInfo::bContainsUnitId(const tCIDLib::TCard1 c1Id) const
{
    return m_fcolUnits.bElementPresent(c1Id);
}


//
//  Get set the id of the driver field that represents the on/off state of this
//  group.
//
tCIDLib::TCard4 TZWaveGroupInfo::c4FldId() const
{
    return m_c4FldId;
}

tCIDLib::TCard4 TZWaveGroupInfo::c4FldId(const tCIDLib::TCard4 c4ToSet)
{
    m_c4FldId = c4ToSet;
    return m_c4FldId;
}


// Get the count of units in this group
tCIDLib::TCard4 TZWaveGroupInfo::c4UnitCount() const
{
    return m_fcolUnits.c4ElemCount();
}


// We just empty the list of unit ids
tCIDLib::TVoid TZWaveGroupInfo::ClearUnitList()
{
    m_fcolUnits.RemoveAll();
}


// Get the list of ids of the units in this group
const tZWaveUSBSh::TIdList TZWaveGroupInfo::fcolUnits() const
{
    return m_fcolUnits;
}


// Get the unique group id
const TString& TZWaveGroupInfo::strId() const
{
    return m_strId;
}


// Get/set the group name
const TString& TZWaveGroupInfo::strName() const
{
    return m_strName;
}

const TString& TZWaveGroupInfo::strName(const TString& strToSet)
{
    m_strName = strToSet;
    return m_strName;
}


// ---------------------------------------------------------------------------
//  TZWaveGroupInfo: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TZWaveGroupInfo::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Make sure we get the start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion != ZWaveUSBSh_GroupInfo::c2FmtVersion))
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

    strmToReadFrom  >> m_strName
                    >> m_strId
                    >> m_fcolUnits;

    // Make sure we get the end marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);

    // Reset any runtime-only fields
    m_c4FldId = kCIDLib::c4MaxCard;
}


tCIDLib::TVoid TZWaveGroupInfo::StreamTo(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << ZWaveUSBSh_GroupInfo::c2FmtVersion
                    << m_strName
                    << m_strId
                    << m_fcolUnits
                    << tCIDLib::EStreamMarkers::EndObject;
}


