//
// FILE NAME: ZWaveLeviSh_Sensor.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/12/2012
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
//  This file provides the implementation for the sensor oriented unit
//  types.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "ZWaveLeviSh_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
AdvRTTIDecls(TZWBinSensor,TZWaveUnit)


// ---------------------------------------------------------------------------
//  Local types and constants
// ---------------------------------------------------------------------------
namespace ZWaveLeviSh_UnitInfo
{
    // -----------------------------------------------------------------------
    //  Persistent format version
    // -----------------------------------------------------------------------
    const tCIDLib::TCard2   c2FmtBinSensorVer   = 1;
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
        , 0
        , L"BinSensor"
        , L"On/Off Sensor"
        , tZWaveLeviSh::EUnitCaps
          (
            tZWaveLeviSh::EUnitCap_AsyncNotify
            | tZWaveLeviSh::EUnitCap_BasicClass
            | tZWaveLeviSh::EUnitCap_Readable
          )
        , tZWaveLeviSh::EUnitCap_None
    )
    , m_c4FldId_State(kCIDLib::c4MaxCard)
{
    // Override the default opt info to zero
    c4OptInfo(0);
}

TZWBinSensor::TZWBinSensor(const tCIDLib::TCard4 c4UnitId) :

    TZWaveUnit
    (
        GENERIC_TYPE_SENSOR_BINARY
        , 0
        , L"BinSensor"
        , L"On/Off Sensor"
        , tZWaveLeviSh::EUnitCaps
          (
            tZWaveLeviSh::EUnitCap_AsyncNotify
            | tZWaveLeviSh::EUnitCap_BasicClass
            | tZWaveLeviSh::EUnitCap_Readable
          )
        , tZWaveLeviSh::EUnitCap_None
        , c4UnitId
    )
    , m_c4FldId_State(kCIDLib::c4MaxCard)
{
    // Override the default opt info to zero
    c4OptInfo(0);
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
TZWBinSensor& TZWBinSensor::operator=(const TZWBinSensor& unitSrc)
{
    if (this != &unitSrc)
    {
        TParent::operator=(unitSrc);
        m_c4FldId_State = unitSrc.m_c4FldId_State;
    }
    return *this;
}


// We don't have any of our own persisted members, so just call the parent for now
tCIDLib::TBoolean TZWBinSensor::operator==(const TZWBinSensor& unitSrc) const
{
    return TParent::operator==(unitSrc);
}


tCIDLib::TBoolean TZWBinSensor::operator!=(const TZWBinSensor& unitSrc) const
{
    return !operator==(unitSrc);
}


// ---------------------------------------------------------------------------
//  TZWBinSensor: Public, inherited methods
// ---------------------------------------------------------------------------

// We aren't settable so we override and always return false
tCIDLib::TBoolean
TZWBinSensor::bBuildSetMsg( const   tCIDLib::TCard4 c4FldVal
                            , const TString&        strValue
                            ,       TString&        strToFill
                            ,       TCQCServerBase& sdrvLevi)
{
    return kCIDLib::False;
}

// Store a received basic report value
tCIDLib::TBoolean
TZWBinSensor::bStoreBasicReport(const   tCIDLib::TCard4 c4Value
                                ,       TCQCServerBase& sdrvLevi)
{
    //
    //  Just write the boolean result to our field. If the field changed,
    //  then send out a motion event if this is a motion sensor.
    //
    StoreValue(c4Value, sdrvLevi);
    return kCIDLib::True;
}

tCIDLib::TBoolean TZWBinSensor::bOwnsFld(const tCIDLib::TCard4 c4FldId) const
{
    return (m_c4FldId_State == c4FldId);
}


tCIDLib::TBoolean
TZWBinSensor::bStoreFldIds( const   TVector<TCQCFldDef>&    colFlds
                            , const tZWaveLeviSh::TValList& fcolIds)
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


// Handles non-basic type reports
tCIDLib::TBoolean
TZWBinSensor::bStoreGenReport(  const   tCIDLib::TCard4 ac4Values[]
                                , const tCIDLib::TCard4 c4ValCnt
                                ,       TCQCServerBase& sdrvLevi)
{
    if (c4ValCnt < 2)
        return kCIDLib::False;

    //
    //  We get general reports as basic status generally, but sometimes
    //  via the sensor class.
    //
    if (((ac4Values[0] == COMMAND_CLASS_BASIC)
    ||   (ac4Values[0] == COMMAND_CLASS_SENSOR_BINARY))
    &&  ((ac4Values[1] == BASIC_SET) || (ac4Values[1] == BASIC_REPORT))
    &&  (c4ValCnt == 3))
    {
        StoreValue(ac4Values[2], sdrvLevi);
        return kCIDLib::True;
    }

    return TParent::bStoreGenReport(ac4Values, c4ValCnt, sdrvLevi);
}


// Return our optional info strings
tCIDLib::TBoolean
TZWBinSensor::bWantsOptInfo(tCIDLib::TStrCollect& colToFill) const
{
    colToFill.objAdd(facZWaveLeviSh().strMsg(kZWShMsgs::midOpt_Gen_None));
    colToFill.objAdd(facZWaveLeviSh().strMsg(kZWShMsgs::midOpt_Sensor_Motion));

    return kCIDLib::True;
}


tCIDLib::TCard4
TZWBinSensor::c4QueryFldDefs(TVector<TCQCFldDef>&   colAddTo
                            , TCQCFldDef&           flddTmp) const
{
    //
    //  We just add our one boolean field, read only. If the 'is a' flag
    //  is set, then the user indicated it's a motion sensor so we set
    //  the semantic field type to indicate that.
    //
    flddTmp.Set
    (
        strName()
        , tCQCKit::EFldTypes::Boolean
        , eDefFldAccess()
        , (c4OptInfo() == 1) ? tCQCKit::EFldSTypes::MotionSensor : tCQCKit::EFldSTypes::Generic
    );
    colAddTo.objAdd(flddTmp);

    return 1;
}


// Call our parent and zero out our field id
tCIDLib::TVoid TZWBinSensor::PrepForUse()
{
    TParent::PrepForUse();
    m_c4FldId_State = kCIDLib::c4MaxCard;
}


// Set all of our unit status oriented fields to error state
tCIDLib::TVoid TZWBinSensor::SetFieldErrs(TCQCServerBase& sdrvLevi)
{
    if (m_c4FldId_State != kCIDLib::c4MaxCard)
        sdrvLevi.SetFieldErr(m_c4FldId_State, kCIDLib::True);
}


// ---------------------------------------------------------------------------
//  TZWBinSensor: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  We can potentially store the value from multiple place,s so we break out
//  the grunt work to here.
//
tCIDLib::TVoid
TZWBinSensor::StoreValue(const tCIDLib::TCard4 c4Val, TCQCServerBase& sdrvLevi)
{
    //
    //  Update our last poll time to now since we got a new value. Tell it to
    //  set our last value time as well.
    //
    ResetPollTime(kCIDLib::True);

    if (!bCheckFldId(m_c4FldId_State, L"state"))
        return;

    tCIDLib::TBoolean bChanged = sdrvLevi.bStoreBoolFld
    (
        m_c4FldId_State, c4Val != 0, kCIDLib::True
    );

    // If changed, ad we are a motion sensor, then send a trigger
    if (bChanged && (c4OptInfo() == 1))
    {
        TString strId;
        TString strName;
        strId.SetFormatted(c4Id());
        if (sdrvLevi.bQueryFldName(m_c4FldId_State, strName))
        {
            sdrvLevi.QueueEventTrig
            (
                tCQCKit::EStdDrvEvs::Motion
                , strName
                , facCQCKit().strBoolStartEnd(c4Val != 0)
                , strId
                , strName
                , TString::strEmpty()
            );
        }
    }
}


