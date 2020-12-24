//
// FILE NAME: ZWaveUSBSh_UnitInfo.cpp
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
//  This file provides the implementation for the TZWaveUnit class, which
//  represents the info we need to maintain for the units we are controlling.
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
RTTIDecls(TZWaveUnit,TCQCServerBase)
AdvRTTIDecls(TZWBinSensor,TZWaveUnit)
AdvRTTIDecls(TZWBinSwitch,TZWaveUnit)
AdvRTTIDecls(TZWEntryCtrl,TZWaveUnit)
AdvRTTIDecls(TZWMLSwitch,TZWaveUnit)


// ---------------------------------------------------------------------------
//  Local types and constants
// ---------------------------------------------------------------------------
namespace ZWaveUSBSh_unitnfo
{
    // -----------------------------------------------------------------------
    //  Our persistent format versions
    // -----------------------------------------------------------------------
    const tCIDLib::TCard2   c2FmtVersion        = 1;
    const tCIDLib::TCard2   c2FmtBinSensorVer   = 1;
    const tCIDLib::TCard2   c2FmtBinSwVer       = 1;
    const tCIDLib::TCard2   c2FmtEntryCtrlVer   = 1;
    const tCIDLib::TCard2   c2FmtMultiSwVer     = 1;
}



// ---------------------------------------------------------------------------
//   CLASS: TZWaveUnit
//  PREFIX: unit
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWaveUnit: Public, static methods
// ---------------------------------------------------------------------------
tCIDLib::ESortComps
TZWaveUnit::eCompById(const TZWaveUnit& unit1, const TZWaveUnit& unit2, const tCIDLib::TCard4)
{
    if (unit1.m_c1Id < unit2.m_c1Id)
        return tCIDLib::ESortComps::FirstLess;
    else if (unit1.m_c1Id > unit2.m_c1Id)
        return tCIDLib::ESortComps::FirstLess;
    return tCIDLib::ESortComps::Equal;
}

tCIDLib::ESortComps
TZWaveUnit::eCompByName(const TZWaveUnit& unit1, const TZWaveUnit& unit2, const tCIDLib::TCard4)
{
    return unit1.m_strName.eCompare(unit2.m_strName);
}


//
//  This method will check all of the available unit derivatives for one
//  that hopefully is a specific match, but at least a generic match. Else
//  it returns zero to indicate we cannot handle that type.
//
TZWaveUnit*
TZWaveUnit::punitBestMatch( const   tCIDLib::TCard1 c1BaseType
                            , const tCIDLib::TCard1 c1GenType
                            , const tCIDLib::TCard1 c1SpecType)
{
    // We create a list of class names and the types they represent
    struct TZWClassList
    {
        const tCIDLib::TCh* pszClass;
        tCIDLib::TCard1     c1GenType;
        tCIDLib::TCard1     c1SpecType;
    };
    static TZWClassList aList[] =
    {
        { L"TZWBinSensor", GENERIC_TYPE_SENSOR_BINARY, SPECIFIC_TYPE_NOT_USED }
      , { L"TZWBinSwitch", GENERIC_TYPE_SWITCH_BINARY, SPECIFIC_TYPE_NOT_USED }
      , { L"TZWEntryCtrl", GENERIC_TYPE_ENTRY_CONTROL,  SPECIFIC_TYPE_NOT_USED }
      , { L"TZWMLSwitch", GENERIC_TYPE_SWITCH_MULTILEVEL, SPECIFIC_TYPE_NOT_USED }
    };
    const tCIDLib::TCard4 c4Count = tCIDLib::c4ArrayElems(aList);

    // If the base type isn't slave or routing slave, return zero now
    if ((c1BaseType != BASIC_TYPE_SLAVE)
    &&  (c1BaseType != BASIC_TYPE_ROUTING_SLAVE))
    {
        return 0;
    }

    // Loop through our class types and find the best match
    tCIDLib::TCard4 c4Match = kCIDLib::c4MaxCard;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TZWClassList& CurItem = aList[c4Index];

        if (CurItem.c1GenType == c1GenType)
        {
            c4Match = c4Index;

            //
            //  If we match the specific type, then we are good and can exit.
            //  If the passed module has no specific type, and this base type
            //  matches one of our entries above that is purely generic and
            //  also has no base type, then we will break out here.
            //
            if (CurItem.c1SpecType == c1SpecType)
                break;

            //
            //  This is a generic match at least. If the passed module has
            //  no specific type, then we are done. Else, we'll keep going
            //  to see if we can find a better match.
            //
            if (c1SpecType == SPECIFIC_TYPE_NOT_USED)
                break;
        }
    }

    // If none found, return zero, else gen up an object and return it
    if (c4Match == kCIDLib::c4MaxCard)
        return 0;
    return ::pobjMakeNewOfClass<TZWaveUnit>(aList[c4Match].pszClass);
}


// ---------------------------------------------------------------------------
//  TZWaveUnit: Destructor
// ---------------------------------------------------------------------------
TZWaveUnit::~TZWaveUnit()
{
}


// ---------------------------------------------------------------------------
//  TZWaveUnit: Public operators
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TZWaveUnit::operator==(const TZWaveUnit& unitSrc) const
{
    // Only need to compare the persistent values
    return
    (
        (m_c1Id == unitSrc.m_c1Id)
        && (m_strName == unitSrc.m_strName)
        && (m_eStatus == unitSrc.m_eStatus)
        && (m_c1ZWFlags == unitSrc.m_c1ZWFlags)
        && (m_enctPollPeriod == unitSrc.m_enctPollPeriod)
    );
}

tCIDLib::TBoolean TZWaveUnit::operator!=(const TZWaveUnit& unitSrc) const
{
    return !operator==(unitSrc);
}



// ---------------------------------------------------------------------------
//  TZWaveUnit: Public, virtual methods
// ---------------------------------------------------------------------------

//
//  Our derived classes will call us here to see if it's a generic message
//  that we can handle for all unit types.
//
tCIDLib::TBoolean
TZWaveUnit::bBuildGetMsg(       TMemBuf&                mbufTar
                        ,       tCIDLib::TCard1&        c1RetClass
                        ,       tCIDLib::TCard1&        c1RetCmd
                        ,       tCIDLib::TCard1&        c1RetLen
                        , const tCIDLib::TCard1         c1Val1
                        , const tCIDLib::TCard1         c1Val2
                        , const tCIDLib::TCard1         c1CBId
                        , const tZWaveUSBSh::EUGetCmds  eCmd) const
{
    mbufTar.PutCard1(REQUEST, 1);
    mbufTar.PutCard1(FUNC_ID_ZW_SEND_DATA, 2);
    mbufTar.PutCard1(c1Id(), 3);

    tCIDLib::TBoolean   bRet = kCIDLib::True;
    tCIDLib::TCard1     c1Cnt = 4;
    if (eCmd == tZWaveUSBSh::EUGetCmd_GetReport)
    {
        //
        //  We can handle basic report gets which many derivatives will
        //  use.
        //
        mbufTar.PutCard1(sizeof(ZW_BASIC_GET_FRAME), c1Cnt++);
        mbufTar.PutCard1(COMMAND_CLASS_BASIC, c1Cnt++);
        mbufTar.PutCard1(BASIC_GET, c1Cnt++);

        // We have to give him back info on what we expect back
        c1RetClass = COMMAND_CLASS_BASIC;
        c1RetCmd = BASIC_REPORT;
        c1RetLen = sizeof(ZW_BASIC_REPORT_FRAME);
    }
     else if (eCmd == tZWaveUSBSh::EUGetCmd_GetGrpAssoc)
    {
        mbufTar.PutCard1(sizeof(ZW_ASSOCIATION_GET_FRAME), c1Cnt++);
        mbufTar.PutCard1(COMMAND_CLASS_ASSOCIATION, c1Cnt++);
        mbufTar.PutCard1(ASSOCIATION_GET, c1Cnt++);
        mbufTar.PutCard1(c1Val1, c1Cnt++);

        //
        //  We have to give him back info on what we expect back. THis one
        //  ones has a variable length, so we have to give back zero.
        //
        c1RetClass = COMMAND_CLASS_ASSOCIATION;
        c1RetCmd = ASSOCIATION_REPORT;
        c1RetLen = sizeof(ZW_ASSOCIATION_REPORT_1BYTE_FRAME);
    }
     else
    {
        bRet = kCIDLib::False;
    }

    mbufTar.PutCard1(TRANSMIT_OPTION_ACK | TRANSMIT_OPTION_AUTO_ROUTE, c1Cnt++);
    mbufTar.PutCard1(c1CBId, c1Cnt++);

    // Go back now and put in the count
    mbufTar.PutCard1(c1Cnt, 0);

    return bRet;
}


//
//  Our derived classes will call us here to see if it's a generic message
//  that we can handle for all unit types.
//
tCIDLib::TBoolean
TZWaveUnit::bBuildSetMsg(       TMemBuf&                mbufTar
                        , const tCIDLib::TCard1         c1Val1
                        , const tCIDLib::TCard1         c1Val2
                        , const tCIDLib::TCard1         c1CBId
                        , const tZWaveUSBSh::EUSetCmds  eCmd) const
{
    mbufTar.PutCard1(REQUEST, 1);
    mbufTar.PutCard1(FUNC_ID_ZW_SEND_DATA, 2);
    mbufTar.PutCard1(c1Id(), 3);

    tCIDLib::TBoolean   bRet = kCIDLib::True;
    tCIDLib::TCard1     c1Cnt = 4;

    if (eCmd == tZWaveUSBSh::EUSetCmd_AddAssoc)
    {
        // Value 1 is the parm number, and value 2 is the parm value
        mbufTar.PutCard1(sizeof(ZW_ASSOCIATION_SET_1BYTE_FRAME), c1Cnt++);
        mbufTar.PutCard1(COMMAND_CLASS_ASSOCIATION, c1Cnt++);
        mbufTar.PutCard1(ASSOCIATION_SET, c1Cnt++);
        mbufTar.PutCard1(c1Val1, c1Cnt++);
        mbufTar.PutCard1(c1Val2, c1Cnt++);
    }
     else if (eCmd == tZWaveUSBSh::EUSetCmd_DelAssoc)
    {
    }
     else if (eCmd == tZWaveUSBSh::EUSetCmd_OffOn)
    {
        // For any derivatives that use the basic set, we can do it for them
        mbufTar.PutCard1(sizeof(ZW_BASIC_SET_FRAME), c1Cnt++);
        mbufTar.PutCard1(COMMAND_CLASS_BASIC, c1Cnt++);
        mbufTar.PutCard1(BASIC_SET, c1Cnt++);
        mbufTar.PutCard1((c1Val1 == 0) ? 0 : 0xFF, c1Cnt++);
    }
     else if (eCmd == tZWaveUSBSh::EUSetCmd_SetCfgParam)
    {
        mbufTar.PutCar1(sizeof(ZW_CONFIGURATION_SET_1BYTE_FRAME), c1Cnt++);
        mbufTar.PutCar1(COMMAND_CLASS_CONFIGURATION, c1Cnt++);
        mbufTar.PutCar1(CONFIGURATION_SET, c1Cnt++);
        mbufTar.PutCar1(c1Val1, c1Cnt++);
        mbufTar.PutCar1(1, c1Cnt++);
        mbufTar.PutCar1(c1Val2, c1Cnt++);
    }
     else
    {
        bRet = kCIDLib::False;
    }

    mbufTar.PutCard1(TRANSMIT_OPTION_ACK | TRANSMIT_OPTION_AUTO_ROUTE, c1Cnt++);
    mbufTar.PutCard1(c1CBId, c1Cnt++);

    // Go back now and put in the count
    mbufTar.PutCard1(c1Cnt, 0);

    return bRet;
}



// ---------------------------------------------------------------------------
//  TZWaveUnit: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Returns true if the unit has all the indicated caps
tCIDLib::TBoolean
TZWaveUnit::bHasCap(const tZWaveUSBSh::EUnitCaps eToTest) const
{
    return (tZWaveUSBSh::EUnitCaps(m_eCaps & eToTest) == eToTest);
}


//
//  Get the values of unit capabilities flags bits. We don't want the client
//  driver to have to deal with the Z-Wave includes, so we just provide some
//  boolean getters.
//
tCIDLib::TBoolean TZWaveUnit::bIsListening() const
{
    return (m_c1ZWFlags & NODEINFO_LISTENING_SUPPORT) != 0;
}


//
//  Based on the type info that the derived class provided for us, we can
//  tell the caller if we can handle the indicated module type, and if so
//  how good a match we are.
//
tZWaveUSBSh::EUnitMatch
TZWaveUnit::eIsMatchingType(const   tCIDLib::TCard1 c1BaseType
                            , const tCIDLib::TCard1 c1GenType
                            , const tCIDLib::TCard1 c1SpecType)
{
    // The base always currently has to be slave or routing slave
    if ((c1BaseType != BASIC_TYPE_SLAVE)
    &&  (c1BaseType != BASIC_TYPE_ROUTING_SLAVE))
    {
        return tZWaveUSBSh::EUnitMatch_None;
    }

    // If the generic type isn't the same, then obviously not
    if (c1GenType != m_c1ClassGen)
        return tZWaveUSBSh::EUnitMatch_None;

    // If the specific class doesn't match, see if it matches generically
    if (m_c1ClassSpec != c1SpecType)
    {
        if (m_c1ClassSpec == SPECIFIC_TYPE_NOT_USED)
            return tZWaveUSBSh::EUnitMatch_Generic;
        return tZWaveUSBSh::EUnitMatch_None;
    }

    // We got a specific match
    return tZWaveUSBSh::EUnitMatch_Specific;
}


// Get the generic and specific classes set by the derived class
tCIDLib::TCard1 TZWaveUnit::c1ClassGen() const
{
    return m_c1ClassGen;
}

tCIDLib::TCard1 TZWaveUnit::c1ClassSpec() const
{
    return m_c1ClassSpec;
}


// Get/set the unit id
tCIDLib::TCard1 TZWaveUnit::c1Id() const
{
    return m_c1Id;
}

tCIDLib::TCard1 TZWaveUnit::c1Id(const tCIDLib::TCard1 c1ToSet)
{
    m_c1Id = c1ToSet;

    //
    //  If no name has been set yet, then we can now assign the default
    //  name based on the id.
    //
    if (m_strName.bIsEmpty())
        SetDefName();
    return m_c1Id;
}


// Increment the number of polling retries on this unit. Return new value
tCIDLib::TCard4 TZWaveUnit::c4IncRetries()
{
    return ++m_c4Retries;
}


// Get/set the poll retries for this unit
tCIDLib::TCard4 TZWaveUnit::c4Retries() const
{
    return m_c4Retries;
}

tCIDLib::TCard4 TZWaveUnit::c4Retries(const tCIDLib::TCard4 c4ToSet)
{
    m_c4Retries = c4ToSet;
    return m_c4Retries;
}


// Get the unit capabilities flags
tZWaveUSBSh::EUnitCaps TZWaveUnit::eCaps() const
{
    return m_eCaps;
}


//
//  Get the overall unit status. We have state specific setters, since each
//  change of the status may require updating some other members.
//
tZWaveUSBSh::EUnitStatus TZWaveUnit::eStatus() const
{
    return m_eStatus;
}


// Get the last stored lag time
tCIDLib::TEncodedTime TZWaveUnit::enctLagTime() const
{
    return m_enctLagTime;
}


// Get the last time that this unit was polled. Zero if not polled yet
tCIDLib::TEncodedTime TZWaveUnit::enctLastPoll() const
{
    return m_enctLastPoll;
}


// Get the next poll time. Which is our last poll time plus our poll period
tCIDLib::TEncodedTime TZWaveUnit::enctNextPoll() const
{
    return m_enctLastPoll + m_enctPollPeriod;
}


// Get/set the poll period
tCIDLib::TEncodedTime TZWaveUnit::enctPollPeriod() const
{
    return m_enctPollPeriod;
}

tCIDLib::TEncodedTime
TZWaveUnit::enctPollPeriod(const tCIDLib::TEncodedTime enctToSet)
{
    m_enctPollPeriod = enctToSet;
    return m_enctPollPeriod;
}


// We reset our last poll time to now
tCIDLib::TVoid TZWaveUnit::ResetPollTime()
{
    m_enctLastPoll = TTime::enctNow();
}


// Get/set the unit name
const TString& TZWaveUnit::strName() const
{
    return m_strName;
}

const TString& TZWaveUnit::strName(const TString& strToSet)
{
    m_strName = strToSet;
    return m_strName;
}


// Get the type name
const TString& TZWaveUnit::strTypeName() const
{
    return m_strTypeName;
}


// Get the type info
const TString& TZWaveUnit::strTypeInfo() const
{
    return m_strTypeInfo;
}


// Puts this unit into error state
tCIDLib::TVoid TZWaveUnit::SetErrorState(TCQCServerBase& sdrvTarget)
{
    m_eStatus = tZWaveUSBSh::EUnitStatus_Error;
    m_c4Retries = 0;
    SetFieldErrs(sdrvTarget);
}


// Puts this unit into failed state
tCIDLib::TVoid TZWaveUnit::SetFailedState()
{
    m_eStatus = tZWaveUSBSh::EUnitStatus_Failed;
    m_c4Retries = 0;
}


// Puts this unit into missing state
tCIDLib::TVoid TZWaveUnit::SetMissingState()
{
    m_eStatus = tZWaveUSBSh::EUnitStatus_Missing;
    m_c4Retries = 0;
}


// Puts this unit into ready state
tCIDLib::TVoid TZWaveUnit::SetReadyState()
{
    m_eStatus = tZWaveUSBSh::EUnitStatus_Ready;
    m_c4Retries = 0;

    // Set up our next poll time if polling is enabled
    if (m_enctPollPeriod)
        ResetPollTime();
}


//
//  The Z-Wave level unit capabilities flag is set on us once it is seen
//  during discovery.
//
tCIDLib::TVoid
TZWaveUnit::SetZWFlags(const tCIDLib::TCard1 c1ToSet)
{
    m_c1ZWFlags = c1ToSet;

    // If not a listening node, then turn off polling
    if (!(c1ToSet & NODEINFO_LISTENING_SUPPORT))
        m_eCaps = tCIDLib::eClearEnumBits(m_eCaps, tZWaveUSBSh::EUnitCap_Pollable);
}


// Store the time between now and when we were scheduled to poll
tCIDLib::TVoid TZWaveUnit::StoreLagTime()
{
    const tCIDLib::TEncodedTime enctSched(m_enctLastPoll + m_enctPollPeriod);
    const tCIDLib::TEncodedTime enctCur = TTime::enctNow();

    if (enctSched >= enctCur)
        m_enctLagTime = 0;
    else
        m_enctLagTime = enctCur - enctSched;
}



// ---------------------------------------------------------------------------
//  TZWaveUnit: Hidden Constructors and Operators
// ---------------------------------------------------------------------------
TZWaveUnit::TZWaveUnit( const   tCIDLib::TCard1         c1GenType
                        , const tCIDLib::TCard1         c1SpecType
                        , const TString&                strTypeName
                        , const TString&                strTypeInfo
                        , const tZWaveUSBSh::EUnitCaps  eCaps
                        , const tCIDLib::TCard1         c1UnitId) :

    m_c1ClassGen(c1GenType)
    , m_c1ClassSpec(c1SpecType)
    , m_c1Id(c1UnitId)
    , m_c1ZWFlags(0)
    , m_c4Retries(0)
    , m_eCaps(eCaps)
    , m_eStatus(tZWaveUSBSh::EUnitStatus_Ready)
    , m_enctLagTime(0)
    , m_enctLastPoll(0)
    , m_enctPollPeriod(kCIDLib::enctTwoSeconds)
    , m_strTypeInfo(strTypeInfo)
    , m_strTypeName(strTypeName)
{
}

TZWaveUnit::TZWaveUnit(const TZWaveUnit& unitToCopy) :

    m_c1ClassGen(unitToCopy.m_c1ClassGen)
    , m_c1ClassSpec(unitToCopy.m_c1ClassSpec)
    , m_c1Id(unitToCopy.m_c1Id)
    , m_c1ZWFlags(unitToCopy.m_c1ZWFlags)
    , m_c4Retries(unitToCopy.m_c4Retries)
    , m_eCaps(unitToCopy.m_eCaps)
    , m_eStatus(unitToCopy.m_eStatus)
    , m_enctLagTime(unitToCopy.m_enctLagTime)
    , m_enctLastPoll(unitToCopy.m_enctLastPoll)
    , m_enctPollPeriod(unitToCopy.m_enctPollPeriod)
    , m_strName(unitToCopy.m_strName)
    , m_strTypeInfo(unitToCopy.m_strTypeInfo)
    , m_strTypeName(unitToCopy.m_strTypeName)
{
}

TZWaveUnit& TZWaveUnit::operator=(const TZWaveUnit& unitToAssign)
{
    if (this != &unitToAssign)
    {
        m_c1ClassGen    = unitToAssign.m_c1ClassGen;
        m_c1ClassSpec   = unitToAssign.m_c1ClassSpec;
        m_c1Id          = unitToAssign.m_c1Id;
        m_c1ZWFlags     = unitToAssign.m_c1ZWFlags;
        m_c4Retries     = unitToAssign.m_c4Retries;
        m_eCaps         = unitToAssign.m_eCaps;
        m_eStatus       = unitToAssign.m_eStatus;
        m_enctLagTime   = unitToAssign.m_enctLagTime;
        m_enctLastPoll  = unitToAssign.m_enctLastPoll;
        m_enctPollPeriod= unitToAssign.m_enctPollPeriod;
        m_strName       = unitToAssign.m_strName;
        m_strTypeInfo   = unitToAssign.m_strTypeInfo;
        m_strTypeName   = unitToAssign.m_strTypeName;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TZWaveUnit: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TZWaveUnit::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Make sure we get the start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion != ZWaveUSBSh_unitnfo::c2FmtVersion))
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

    strmToReadFrom  >> m_c1Id
                    >> m_strName
                    >> m_eStatus
                    >> m_c1ZWFlags
                    >> m_enctPollPeriod;

    // Make sure we get the end marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);

    // Reset the runtime fields
    m_c4Retries = 0;
}


tCIDLib::TVoid TZWaveUnit::StreamTo(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << ZWaveUSBSh_unitnfo::c2FmtVersion
                    << m_c1Id
                    << m_strName
                    << m_eStatus
                    << m_c1ZWFlags
                    << m_enctPollPeriod
                    << tCIDLib::EStreamMarkers::EndObject;
}


// ---------------------------------------------------------------------------
//  TZWaveUnit: Protected, non-virtual methods
// ---------------------------------------------------------------------------

// Creates a default name for newly discovered modules
tCIDLib::TVoid TZWaveUnit::SetDefName()
{
    //
    //  Start with the type name provided by the derived class, which will be
    //  something like "BinSwitch" or "MultiSwitch" and so forth.
    //
    m_strName = m_strTypeName;

    // Then an underscore and a 2 digit hex representation of the unit id
    m_strName.Append(kCIDLib::chUnderscore);
    if (m_c1Id < 0x10)
        m_strName.Append(kCIDLib::chDigit0);
    m_strName.AppendFormatted(m_c1Id, tCIDLib::ERadices::Hex);
}


// ---------------------------------------------------------------------------
//   CLASS: TZWBinSensor
//  PREFIX: unit
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWBinSensor: Constructors and Destructor
// ---------------------------------------------------------------------------
TZWBinSensor::TZWBinSensor() :

    TZWaveUnit
    (
        GENERIC_TYPE_SENSOR_BINARY
        , SPECIFIC_TYPE_NOT_USED
        , L"BinSensor"
        , L"On/Off Sensor"
        , tZWaveUSBSh::EUnitCaps_BinPoll
        , kCIDLib::c1MaxCard
    )
    , m_c4FldId_State(kCIDLib::c4MaxCard)
{
}

TZWBinSensor::TZWBinSensor(const tCIDLib::TCard1 c1UnitId) :

    TZWaveUnit
    (
        GENERIC_TYPE_SENSOR_BINARY
        , SPECIFIC_TYPE_NOT_USED
        , L"BinSensor"
        , L"On/Off Sensor"
        , tZWaveUSBSh::EUnitCaps_BinPoll
        , c1UnitId
    )
    , m_c4FldId_State(kCIDLib::c4MaxCard)
{
}

TZWBinSensor::TZWBinSensor(const TZWBinSensor& unitToCopy) :

    TZWaveUnit(unitToCopy)
    , m_c4FldId_State(unitToCopy.m_c4FldId_State)
{
}

TZWBinSensor::~TZWBinSensor()
{
}


// ---------------------------------------------------------------------------
//  TZWBinSensor: Public operators
// ---------------------------------------------------------------------------
TZWBinSensor& TZWBinSensor::operator=(const TZWBinSensor& unitToAssign)
{
    if (this != &unitToAssign)
    {
        TParent::operator=(unitToAssign);
        m_c4FldId_State = unitToAssign.m_c4FldId_State;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TZWBinSensor: Public, inherited methods
// ---------------------------------------------------------------------------

// Build up a basic report query message
tCIDLib::TBoolean
TZWBinSensor::bBuildGetMsg(         TMemBuf&                mbufTar
                            ,       tCIDLib::TCard1&        c1Class
                            ,       tCIDLib::TCard1&        c1Cmd
                            ,       tCIDLib::TCard1&        c1Len
                            , const tCIDLib::TCard1         c1Val1
                            , const tCIDLib::TCard1         c1Val2
                            , const tCIDLib::TCard1         c1CBId
                            , const tZWaveUSBSh::EUGetCmds  eCmd) const
{
    // The parent class can handle any stuff for us for now
    if (TParent::bBuildGetMsg(mbufTar, c1Class, c1Cmd, c1Len, c1Val1, c1Val2, c1CBId, eCmd))
        return kCIDLib::True;

    return kCIDLib::False;
}


tCIDLib::TBoolean
TZWBinSensor::bBuildSetMsg(         TMemBuf&                mbufTar
                            , const tCIDLib::TCard1         c1Val1
                            , const tCIDLib::TCard1         c1Val2
                            , const tCIDLib::TCard1         c1CBId
                            , const tZWaveUSBSh::EUSetCmds  eCmd) const
{
    if (TParent::bBuildSetMsg(mbufTar, c1Val1, c1Val2, c1CBId, eCmd))
        return kCIDLib::True;

    return kCIDLib::False;
}


tCIDLib::TBoolean TZWBinSensor::bOwnsFld(const tCIDLib::TCard4 c4FldId) const
{
    return (m_c4FldId_State == c4FldId);
}


tCIDLib::TBoolean
TZWBinSensor::bStoreFldIds( const   TVector<TCQCFldDef>&        colFlds
                            , const tZWaveUSBSh::TFldIdList&    fcolIds)
{
    // Find our field and store the id
    const TString& strOurFld = strName();
    const tCIDLib::TCard4 c4Count = colFlds.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TCQCFldDef& flddCur = colFlds[c4Index];
        if (flddCur.strName() == strName())
        {
            m_c4FldId_State = fcolIds[c4Index];
            return kCIDLib::True;
        }
    }
    return kCIDLib::False;
}


tCIDLib::TCard4
TZWBinSensor::c4QueryFldDefs(TVector<TCQCFldDef>&   colAddTo
                            , TCQCFldDef&           flddTmp) const
{
    // We just add our one boolean field, read only
    flddTmp.Set
    (
        strName()
        , tCQCKit::EFldTypes::Boolean
        , tCQCKit::EFldAccess::Read
        , tCQCKit::EFldSTypes::MotionSensor
    );

    // If not pollable, we have to be always write
    if (!bHasCap(tZWaveUSBSh::EUnitCap_Pollable) || !enctPollPeriod())
        flddTmp.bAlwaysWrite(kCIDLib::True);
    colAddTo.objAdd(flddTmp);

    return 1;
}


tCIDLib::TVoid
TZWBinSensor::HandleAppCmd( const   tCIDLib::TCard1* const  pc1Data
                            , const tCIDLib::TCard4         c4Bytes
                            ,       TCQCServerBase&         sdrvTarget)
{
    if ((pc1Data[0] == COMMAND_CLASS_BASIC)
    &&  ((pc1Data[1] == BASIC_SET) || (pc1Data[1] == BASIC_REPORT)))
    {
        // We just need to pull out the on/off value and store it
        const ZW_BASIC_SET_FRAME* pRepData
        (
            reinterpret_cast<const ZW_BASIC_SET_FRAME*>(pc1Data)
        );

        const tCIDLib::TBoolean bNewValue(pRepData->value != 0);
        tCIDLib::TBoolean bChanged;
        {
            bChanged = sdrvTarget.bStoreBoolFld
            (
                m_c4FldId_State, bNewValue, kCIDLib::True
            );
        }

        // If it changed, then send out a standard motion event
        if (bChanged)
        {
            mzwsfLevi.LeviQEventTrig
            (
                tCQCKit::EStdDrvEvs::Motion
                , strName()
                , facCQCKit().strBoolStartEnd(bNewValue)
                , TCardinal(c1Id())
                , strName()
                , TString::strEmpty()
            );
        }
    }
}


tCIDLib::TVoid TZWBinSensor::SetFieldErrs(TCQCServerBase& sdrvTarget)
{
    sdrvTarget.SetFieldErr(m_c4FldId_State, kCIDLib::True);
}




// ---------------------------------------------------------------------------
//   CLASS: TZWBinSwitch
//  PREFIX: unit
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWBinSwitch: Constructors and Destructor
// ---------------------------------------------------------------------------
TZWBinSwitch::TZWBinSwitch() :

    TZWaveUnit
    (
        GENERIC_TYPE_SWITCH_BINARY
        , SPECIFIC_TYPE_NOT_USED
        , L"BinSwitch"
        , L"On/Off Switch"
        , tZWaveUSBSh::EUnitCaps_BinPoll
        , kCIDLib::c1MaxCard
    )
    , m_c4FldId_Switch(kCIDLib::c4MaxCard)
{
}

TZWBinSwitch::TZWBinSwitch(const tCIDLib::TCard1 c1UnitId) :

    TZWaveUnit
    (
        GENERIC_TYPE_SWITCH_BINARY
        , SPECIFIC_TYPE_NOT_USED
        , L"BinSwitch"
        , L"On/Off Switch"
        , tZWaveUSBSh::EUnitCaps_BinPoll
        , c1UnitId
    )
    , m_c4FldId_Switch(kCIDLib::c4MaxCard)
{
}

TZWBinSwitch::TZWBinSwitch(const TZWBinSwitch& unitToCopy) :

    TZWaveUnit(unitToCopy)
    , m_c4FldId_Switch(unitToCopy.m_c4FldId_Switch)
{
}

TZWBinSwitch::~TZWBinSwitch()
{
}


// ---------------------------------------------------------------------------
//  TZWBinSwitch: Public operators
// ---------------------------------------------------------------------------
TZWBinSwitch& TZWBinSwitch::operator=(const TZWBinSwitch& unitToAssign)
{
    if (this != &unitToAssign)
    {
        TParent::operator=(unitToAssign);
        m_c4FldId_Switch = unitToAssign.m_c4FldId_Switch;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TZWBinSwitch: Public, inherited methods
// ---------------------------------------------------------------------------

// Build up a basic report query message
tCIDLib::TBoolean
TZWBinSwitch::bBuildGetMsg(         TMemBuf&                mbufTar
                            ,       tCIDLib::TCard1&        c1Class
                            ,       tCIDLib::TCard1&        c1Cmd
                            ,       tCIDLib::TCard1&        c1Len
                            , const tCIDLib::TCard1         c1Val1
                            , const tCIDLib::TCard1         c1Val2
                            , const tCIDLib::TCard1         c1CBId
                            , const tZWaveUSBSh::EUGetCmds  eCmd) const
{
    // The parent class can handle any stuff for us for now
    if (TParent::bBuildGetMsg(mbufTar, c1Class, c1Cmd, c1Len, c1Val1, c1Val2, c1CBId, eCmd))
        return kCIDLib::True;

    return kCIDLib::False;
}


// Build up a generic set message
tCIDLib::TBoolean
TZWBinSwitch::bBuildSetMsg(         TMemBuf&                mbufTar
                            , const tCIDLib::TCard1         c1Val1
                            , const tCIDLib::TCard1         c1Val2
                            , const tCIDLib::TCard1         c1CBId
                            , const tZWaveUSBSh::EUSetCmds  eCmd) const
{
    // Let the parent handle anything it can
    if (TParent::bBuildSetMsg(mbufTar, c1Val1, c1Val2, c1CBId, eCmd))
        return kCIDLib::True;

    return kCIDLib::True;
}


tCIDLib::TBoolean TZWBinSwitch::bOwnsFld(const tCIDLib::TCard4 c4FldId) const
{
    return (m_c4FldId_Switch == c4FldId);
}


tCIDLib::TBoolean
TZWBinSwitch::bStoreFldIds( const   TVector<TCQCFldDef>&        colFlds
                            , const tZWaveUSBSh::TFldIdList&    fcolIds)
{
    // Find our field and store the id
    const TString& strOurFld = strName();
    const tCIDLib::TCard4 c4Count = colFlds.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TCQCFldDef& flddCur = colFlds[c4Index];
        if (flddCur.strName() == strName())
        {
            m_c4FldId_Switch = fcolIds[c4Index];
            return kCIDLib::True;
        }
    }
    return kCIDLib::False;
}


tCIDLib::TCard4
TZWBinSwitch::c4QueryFldDefs(TVector<TCQCFldDef>&   colAddTo
                            , TCQCFldDef&           flddTmp) const
{
    // We just add our one boolean field
    flddTmp.Set
    (
        strName()
        , tCQCKit::EFldTypes::Boolean
        , tCQCKit::EFldAccess::ReadWrite
        , tCQCKit::EFldSTypes::LightSwitch
    );

    // If not pollable, we have to be always write
    if (!bHasCap(tZWaveUSBSh::EUnitCap_Pollable) || !enctPollPeriod())
        flddTmp.bAlwaysWrite(kCIDLib::True);
    colAddTo.objAdd(flddTmp);

    return 1;
}


tCIDLib::TVoid
TZWBinSwitch::HandleAppCmd( const   tCIDLib::TCard1* const  pc1Data
                            , const tCIDLib::TCard4         c4Bytes
                            ,       TCQCServerBase&         sdrvTarget)
{
    if ((pc1Data[0] == COMMAND_CLASS_BASIC)
    &&  ((pc1Data[1] == BASIC_SET) || (pc1Data[1] == BASIC_REPORT)))
    {
        // We just need to pull out the on/off value and store it
        const ZW_BASIC_SET_FRAME* pRepData
        (
            reinterpret_cast<const ZW_BASIC_SET_FRAME*>(pc1Data)
        );

        const tCIDLib::TBoolean bNewValue(pRepData->value != 0);
        tCIDLib::TBoolean bChanged;
        {
            bChanged = sdrvTarget.bStoreBoolFld
            (
                m_c4FldId_Switch, bNewValue, kCIDLib::True
            );
        }

        // If it changed, then send out a standard load change event
        if (bChanged)
        {
            sdrvTarget.QueueEventTrig
            (
                tCQCKit::EStdDrvEvs::LoadChange
                , strName()
                , facCQCKit().strBoolOffOn(bNewValue)
                , TCardinal(c1Id())
                , strName()
                , TString::strEmpty()
            );
        }
    }
}


tCIDLib::TVoid TZWBinSwitch::SetFieldErrs(TCQCServerBase& sdrvTarget)
{
    sdrvTarget.SetFieldErr(m_c4FldId_Switch, kCIDLib::True);
}



// ---------------------------------------------------------------------------
//   CLASS: TZWEntryCtrl
//  PREFIX: unit
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWEntryCtrl: Constructors and Destructor
// ---------------------------------------------------------------------------
TZWEntryCtrl::TZWEntryCtrl() :

    TZWaveUnit
    (
        GENERIC_TYPE_ENTRY_CONTROL
        , SPECIFIC_TYPE_NOT_USED
        , L"EntryCtrl"
        , L"Entry Control"
        , tZWaveUSBSh::EUnitCaps_BinPoll
        , kCIDLib::c1MaxCard
    )
    , m_c4FldId_State(kCIDLib::c4MaxCard)
{
}

TZWEntryCtrl::TZWEntryCtrl(const tCIDLib::TCard1 c1UnitId) :

    TZWaveUnit
    (
        GENERIC_TYPE_ENTRY_CONTROL
        , SPECIFIC_TYPE_NOT_USED
        , L"EntryCtrl"
        , L"Entry Control"
        , tZWaveUSBSh::EUnitCaps_BinPoll
        , c1UnitId
    )
    , m_c4FldId_State(kCIDLib::c4MaxCard)
{
}

TZWEntryCtrl::TZWEntryCtrl(const TZWEntryCtrl& unitToCopy) :

    TZWaveUnit(unitToCopy)
    , m_c4FldId_State(unitToCopy.m_c4FldId_State)
{
}

TZWEntryCtrl::~TZWEntryCtrl()
{
}


// ---------------------------------------------------------------------------
//  TZWEntryCtrl: Public operators
// ---------------------------------------------------------------------------
TZWEntryCtrl& TZWEntryCtrl::operator=(const TZWEntryCtrl& unitToAssign)
{
    if (this != &unitToAssign)
    {
        TParent::operator=(unitToAssign);
        m_c4FldId_State = unitToAssign.m_c4FldId_State;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TZWEntryCtrl: Public, inherited methods
// ---------------------------------------------------------------------------

// Build up a lock report query message
tCIDLib::TBoolean
TZWEntryCtrl::bBuildGetMsg(         TMemBuf&                mbufTar
                            ,       tCIDLib::TCard1&        c1Class
                            ,       tCIDLib::TCard1&        c1Cmd
                            ,       tCIDLib::TCard1&        c1Len
                            , const tCIDLib::TCard1         c1Val1
                            , const tCIDLib::TCard1         c1Val2
                            , const tCIDLib::TCard1         c1CBId
                            , const tZWaveUSBSh::EUGetCmds  eCmd) const
{
    //
    //  The parent class can handle any stuff for us for now since the
    //  entry control device just maps the basic get to the lock get
    //  internally.
    //
    if (TParent::bBuildGetMsg(mbufTar, c1Class, c1Cmd, c1Len, c1Val1, c1Val2, c1CBId, eCmd))
        return kCIDLib::True;

    return kCIDLib::False;
}


// Build up a lock set message
tCIDLib::TBoolean
TZWEntryCtrl::bBuildSetMsg(         TMemBuf&                mbufTar
                            , const tCIDLib::TCard1         c1Val1
                            , const tCIDLib::TCard1         c1Val2
                            , const tCIDLib::TCard1         c1CBId
                            , const tZWaveUSBSh::EUSetCmds  eCmd) const
{
    //
    //  We have to override the off on and use a lock specific command
    //  since locks don't respond to the basic set.
    //
    if (eCmd == tZWaveUSBSh::EUSetCmd_OffOn)
    {
        mbufTar.PutCard1(REQUEST, 1);
        mbufTar.PutCard1(FUNC_ID_ZW_SEND_DATA, 2);
        mbufTar.PutCard1(c1Id(), 3);

        //
        //  This guy only handles off and on commands, though they are mapped
        //  to the lock command here, not the basic set command, since entry
        //  control devices use a specific command for safety.
        //
        tCIDLib::TCard1 c1Cnt = 4;
        {
            mbufTar.PutCard1(sizeof(ZW_LOCK_SET_FRAME), c1Cnt++);
            mbufTar.PutCard1(COMMAND_CLASS_LOCK, c1Cnt++);
            mbufTar.PutCard1(LOCK_SET, c1Cnt++);
            mbufTar.PutCard1((c1Val1 == 0) ? 0 : 1, c1Cnt++);
        }

        mbufTar.PutCard1(TRANSMIT_OPTION_ACK | TRANSMIT_OPTION_AUTO_ROUTE, c1Cnt++);
        mbufTar.PutCard1(c1CBId, c1Cnt++);

        // Go back now and put in the count
        mbufTar.PutCard1(c1Cnt, 0);

        return kCIDLib::True;
    }

    // Let the parent have a crack at it
    return TParent::bBuildSetMsg(mbufTar, c1Val1, c1Val2, c1CBId, eCmd);
}


tCIDLib::TBoolean TZWEntryCtrl::bOwnsFld(const tCIDLib::TCard4 c4FldId) const
{
    return (m_c4FldId_State == c4FldId);
}


tCIDLib::TBoolean
TZWEntryCtrl::bStoreFldIds( const   TVector<TCQCFldDef>&        colFlds
                            , const tZWaveUSBSh::TFldIdList&    fcolIds)
{
    // Find our field and store the id
    const TString& strOurFld = strName();
    const tCIDLib::TCard4 c4Count = colFlds.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TCQCFldDef& flddCur = colFlds[c4Index];
        if (flddCur.strName() == strName())
        {
            m_c4FldId_State = fcolIds[c4Index];
            return kCIDLib::True;
        }
    }
    return kCIDLib::False;
}


tCIDLib::TCard4
TZWEntryCtrl::c4QueryFldDefs(TVector<TCQCFldDef>&   colAddTo
                            , TCQCFldDef&           flddTmp) const
{
    // We just add our one boolean field
    flddTmp.Set
    (
        strName()
        , tCQCKit::EFldTypes::Boolean
        , tCQCKit::EFldAccess::ReadWrite
    );

    // If not pollable, we have to be always write
    if (!bHasCap(tZWaveUSBSh::EUnitCap_Pollable) || !enctPollPeriod())
        flddTmp.bAlwaysWrite(kCIDLib::True);
    colAddTo.objAdd(flddTmp);

    return 1;
}


tCIDLib::TVoid
TZWEntryCtrl::HandleAppCmd( const   tCIDLib::TCard1* const  pc1Data
                            , const tCIDLib::TCard4         c4Bytes
                            ,       TCQCServerBase&         sdrvTarget)
{
    if ((pc1Data[0] == COMMAND_CLASS_BASIC)
    &&  ((pc1Data[1] == BASIC_SET) || (pc1Data[1] == BASIC_REPORT)))
    {
        // We just need to pull out the on/off value and store it
        const ZW_BASIC_SET_FRAME* pRepData
        (
            reinterpret_cast<const ZW_BASIC_SET_FRAME*>(pc1Data)
        );

        sdrvTarget.bStoreBoolFld
        (
            m_c4FldId_State, pRepData->value != 0, kCIDLib::True
        );
    }
     else if ((pc1Data[0] == COMMAND_CLASS_LOCK)
          &&  ((pc1Data[0] == LOCK_SET) || (pc1Data[0] == LOCK_REPORT)))
    {
        // We just need to pull out the on/off value and store it
        const ZW_LOCK_SET_FRAME* pRepData
        (
            reinterpret_cast<const ZW_LOCK_SET_FRAME*>(pc1Data)
        );

        sdrvTarget.bStoreBoolFld
        (
            m_c4FldId_State, pRepData->lockState != 0, kCIDLib::True
        );
    }
     else if (pc1Data[0] == COMMAND_CLASS_WAKE_UP)
    {
        // Do we need to deal with this?
    }
}


tCIDLib::TVoid TZWEntryCtrl::SetFieldErrs(TCQCServerBase& sdrvTarget)
{
    sdrvTarget.SetFieldErr(m_c4FldId_State, kCIDLib::True);
}




// ---------------------------------------------------------------------------
//   CLASS: TZWMLSwitch
//  PREFIX: unit
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWMLSwitch: Constructors and Destructor
// ---------------------------------------------------------------------------
TZWMLSwitch::TZWMLSwitch() :

    TZWaveUnit
    (
        GENERIC_TYPE_SWITCH_MULTILEVEL
        , SPECIFIC_TYPE_NOT_USED
        , L"MLSwitch"
        , L"Multi-level Switch"
        , tZWaveUSBSh::EUnitCaps_MLPoll
        , kCIDLib::c1MaxCard
    )
    , m_c4FldId_Switch(kCIDLib::c4MaxCard)
{
}

TZWMLSwitch::TZWMLSwitch(const tCIDLib::TCard1 c1UnitId) :

    TZWaveUnit
    (
        GENERIC_TYPE_SWITCH_MULTILEVEL
        , SPECIFIC_TYPE_NOT_USED
        , L"MLSwitch"
        , L"Multi-level Switch"
        , tZWaveUSBSh::EUnitCaps_MLPoll
        , c1UnitId
    )
    , m_c4FldId_Switch(kCIDLib::c4MaxCard)
{
}

TZWMLSwitch::TZWMLSwitch(const TZWMLSwitch& unitToCopy) :

    TZWaveUnit(unitToCopy)
    , m_c4FldId_Switch(unitToCopy.m_c4FldId_Switch)
{
}

TZWMLSwitch::~TZWMLSwitch()
{
}


// ---------------------------------------------------------------------------
//  TZWMLSwitch: Public operators
// ---------------------------------------------------------------------------
TZWMLSwitch& TZWMLSwitch::operator=(const TZWMLSwitch& unitToAssign)
{
    if (this != &unitToAssign)
    {
        TParent::operator=(unitToAssign);
        m_c4FldId_Switch = unitToAssign.m_c4FldId_Switch;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TZWMLSwitch: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TZWMLSwitch::bOwnsFld(const tCIDLib::TCard4 c4FldId) const
{
    return (m_c4FldId_Switch == c4FldId);
}


tCIDLib::TBoolean
TZWMLSwitch::bStoreFldIds(  const   TVector<TCQCFldDef>&        colFlds
                            , const tZWaveUSBSh::TFldIdList&    fcolIds)
{
    // Find our field and store the id
    const TString& strOurFld = strName();
    const tCIDLib::TCard4 c4Count = colFlds.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TCQCFldDef& flddCur = colFlds[c4Index];
        if (flddCur.strName() == strName())
        {
            m_c4FldId_Switch = fcolIds[c4Index];
            return kCIDLib::True;
        }
    }
    return kCIDLib::False;
}


// Build up a basic report query message
tCIDLib::TBoolean
TZWMLSwitch::bBuildGetMsg(          TMemBuf&                mbufTar
                            ,       tCIDLib::TCard1&        c1Class
                            ,       tCIDLib::TCard1&        c1Cmd
                            ,       tCIDLib::TCard1&        c1Len
                            , const tCIDLib::TCard1         c1Val1
                            , const tCIDLib::TCard1         c1Val2
                            , const tCIDLib::TCard1         c1CBId
                            , const tZWaveUSBSh::EUGetCmds  eCmd) const
{
    // The parent class can handle any stuff for us for now
    if (TParent::bBuildGetMsg(mbufTar, c1Class, c1Cmd, c1Len, c1Val1, c1Val2, c1CBId, eCmd))
        return kCIDLib::True;

    return kCIDLib::False;
}


tCIDLib::TBoolean
TZWMLSwitch::bBuildSetMsg(          TMemBuf&                mbufTar
                            , const tCIDLib::TCard1         c1Val1
                            , const tCIDLib::TCard1         c1Val2
                            , const tCIDLib::TCard1         c1CBId
                            , const tZWaveUSBSh::EUSetCmds  eCmd) const
{
    if (TParent::bBuildSetMsg(mbufTar, c1Val1, c1Val2, c1CBId, eCmd))
        return kCIDLib::True;

    mbufTar.PutCard1(REQUEST, 1);
    mbufTar.PutCard1(FUNC_ID_ZW_SEND_DATA, 2);
    mbufTar.PutCard1(c1Id(), 3);

    tCIDLib::TBoolean   bRet = kCIDLib::True;
    tCIDLib::TCard1     c1Cnt = 4;
    switch(eCmd)
    {
        case tZWaveUSBSh::EUSetCmd_RampStart :
        {
            // The value indicates up or down here
            const tCIDLib::TBoolean bUp(c1Val1 != 0);

            // No wrap, set 0x40 if down, don't if up
            mbufTar.PutCard1(sizeof(ZW_SWITCH_MULTILEVEL_START_LEVEL_CHANGE_FRAME), c1Cnt++);
            mbufTar.PutCard1(COMMAND_CLASS_SWITCH_MULTILEVEL, c1Cnt++);
            mbufTar.PutCard1(SWITCH_MULTILEVEL_START_LEVEL_CHANGE, c1Cnt++);
            mbufTar.PutCard1(bUp ? 0 : 0x40, c1Cnt++);
            mbufTar.PutCard1(bUp ? 0 : 99, c1Cnt++);
            break;
        }

        case tZWaveUSBSh::EUSetCmd_RampEnd :
        {
            // The value is not used here
            mbufTar.PutCard1(sizeof(ZW_SWITCH_MULTILEVEL_STOP_LEVEL_CHANGE_FRAME), c1Cnt++);
            mbufTar.PutCard1(COMMAND_CLASS_SWITCH_MULTILEVEL, c1Cnt++);
            mbufTar.PutCard1(SWITCH_MULTILEVEL_STOP_LEVEL_CHANGE, c1Cnt++);
            break;
        }

        case tZWaveUSBSh::EUSetCmd_SetLevel :
        {
            // Set it to a specific level indicated in the value
            mbufTar.PutCard1(sizeof(ZW_SWITCH_MULTILEVEL_SET_FRAME), c1Cnt++);
            mbufTar.PutCard1(COMMAND_CLASS_SWITCH_MULTILEVEL, c1Cnt++);
            mbufTar.PutCard1(SWITCH_MULTILEVEL_SET, c1Cnt++);
            mbufTar.PutCard1(c1Val1, c1Cnt++);
            break;
        }

        default :
            bRet = kCIDLib::False;
            break;
    };

    mbufTar.PutCard1(TRANSMIT_OPTION_ACK | TRANSMIT_OPTION_AUTO_ROUTE, c1Cnt++);
    mbufTar.PutCard1(c1CBId, c1Cnt++);

    // Go back now and put in the count
    mbufTar.PutCard1(c1Cnt, 0);

    return bRet;
}


tCIDLib::TCard4
TZWMLSwitch::c4QueryFldDefs(TVector<TCQCFldDef>&    colAddTo
                            , TCQCFldDef&           flddTmp) const
{
    // Add our one card field with the right range for a multi-level switch
    flddTmp.Set
    (
        strName()
        , tCQCKit::EFldTypes::Card
        , tCQCKit::EFldAccess::ReadWrite
        , tCQCKit::EFldSTypes::Dimmer
        , L"Range:0,99"
    );

    // If not pollable, we have to be always write
    if (!bHasCap(tZWaveUSBSh::EUnitCap_Pollable))
        flddTmp.bAlwaysWrite(kCIDLib::True);
    colAddTo.objAdd(flddTmp);

    return 1;
}


tCIDLib::TVoid
TZWMLSwitch::HandleAppCmd(  const   tCIDLib::TCard1* const  pc1Data
                            , const tCIDLib::TCard4         c4Bytes
                            ,       TCQCServerBase&         sdrvTarget)
{
    if ((pc1Data[0] == COMMAND_CLASS_BASIC)
    &&  ((pc1Data[1] == BASIC_SET) || (pc1Data[1] == BASIC_REPORT)))
    {
        // We just need to pull out the on/off value and store it
        const ZW_BASIC_SET_FRAME* pRepData
        (
            reinterpret_cast<const ZW_BASIC_SET_FRAME*>(pc1Data)
        );

        sdrvTarget.bStoreCardFld(m_c4FldId_Switch, pRepData->value, kCIDLib::True);
    }
}


tCIDLib::TVoid TZWMLSwitch::SetFieldErrs(TCQCServerBase& sdrvTarget)
{
    sdrvTarget.SetFieldErr(m_c4FldId_Switch, kCIDLib::True);
}

