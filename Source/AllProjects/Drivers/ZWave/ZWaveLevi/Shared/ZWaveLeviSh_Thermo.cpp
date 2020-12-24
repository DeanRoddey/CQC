//
// FILE NAME: ZWaveLeviSh_Thermo.cpp
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
//  This file provides the implementation for the thermostat type units.
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
AdvRTTIDecls(TZWThermo,TZWaveUnit)


// ---------------------------------------------------------------------------
//  Local types and constants
// ---------------------------------------------------------------------------
namespace ZWaveLeviSh_Thermo
{
    // -----------------------------------------------------------------------
    //  Persistent format versions
    //
    //  Version 2 -
    //      Our format didn't actually change, but we added options info
    //      values for C/F temp scale, so we need to force it to initially
    //      F scale, since that's what it was by default before.
    // -----------------------------------------------------------------------
    const tCIDLib::TCard2   c2FmtThermoVer   = 2;
}



// ---------------------------------------------------------------------------
//   CLASS: TZWThermo
//  PREFIX: unit
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWThermo: Constructors and Destructor
// ---------------------------------------------------------------------------

//
//  Note we start off assuming async notifications, and so we set the initial
//  get msg count to 6, since we include the set points if async is enabled,
//  not due to wanting to poll them but to insure that we get their initial
//  values. If later they change the async status, we need to change it to
//  3, which will happen once we read it back in. Actually it may get set back
//  upon reading this one in, but we have to set it initially to match the
//  default async enabled caps for newly created thermo objects.
//
//  We also initially force our opts info value to 1, which indicate the F
//  scale, so that it'll be defaulted to that for new objects.
//
TZWThermo::TZWThermo() :

    TZWaveUnit
    (
        GENERIC_TYPE_THERMOSTAT
        , SPECIFIC_TYPE_THERMOSTAT_GENERAL
        , L"Thermo"
        , L"Thermostat"
        , tZWaveLeviSh::EUnitCaps
          (
            tZWaveLeviSh::EUnitCap_AsyncNotify
            | tZWaveLeviSh::EUnitCap_Readable
            | tZWaveLeviSh::EUnitCap_Writeable
          )
        , tZWaveLeviSh::EUnitCap_Readable
        , kCIDLib::c4MaxCard
        , 6
    )
    , m_c4FldId_CurTemp(kCIDLib::c4MaxCard)
    , m_c4FldId_FanMode(kCIDLib::c4MaxCard)
    , m_c4FldId_FanState(kCIDLib::c4MaxCard)
    , m_c4FldId_Mode(kCIDLib::c4MaxCard)
    , m_c4FldId_State(kCIDLib::c4MaxCard)

    , m_c4FldId_HighSetPnt(kCIDLib::c4MaxCard)
    , m_c4FldId_LowSetPnt(kCIDLib::c4MaxCard)
{
    // Override the default opt info to 1 (F scale)
    c4OptInfo(1);
}

TZWThermo::TZWThermo(const tCIDLib::TCard4 c4UnitId) :

    TZWaveUnit
    (
        GENERIC_TYPE_THERMOSTAT
        , SPECIFIC_TYPE_THERMOSTAT_GENERAL
        , L"Thermo"
        , L"Thermostat"
        , tZWaveLeviSh::EUnitCaps
          (
            tZWaveLeviSh::EUnitCap_AsyncNotify
            | tZWaveLeviSh::EUnitCap_Readable
            | tZWaveLeviSh::EUnitCap_Writeable
          )
        , tZWaveLeviSh::EUnitCap_Readable
        , c4UnitId
        , 6
    )
    , m_c4FldId_CurTemp(kCIDLib::c4MaxCard)
    , m_c4FldId_FanMode(kCIDLib::c4MaxCard)
    , m_c4FldId_FanState(kCIDLib::c4MaxCard)
    , m_c4FldId_Mode(kCIDLib::c4MaxCard)
    , m_c4FldId_State(kCIDLib::c4MaxCard)

    , m_c4FldId_HighSetPnt(kCIDLib::c4MaxCard)
    , m_c4FldId_LowSetPnt(kCIDLib::c4MaxCard)
{
    // Override the default opt info to 1 (F scale)
    c4OptInfo(1);
}

TZWThermo::TZWThermo(const TZWThermo& unitSrc) :

    TZWaveUnit(unitSrc)
    , m_c4FldId_CurTemp(unitSrc.m_c4FldId_CurTemp)
    , m_c4FldId_FanMode(unitSrc.m_c4FldId_FanMode)
    , m_c4FldId_FanState(unitSrc.m_c4FldId_FanState)
    , m_c4FldId_Mode(unitSrc.m_c4FldId_Mode)
    , m_c4FldId_State(unitSrc.m_c4FldId_State)

    , m_c4FldId_HighSetPnt(unitSrc.m_c4FldId_HighSetPnt)
    , m_c4FldId_LowSetPnt(unitSrc.m_c4FldId_LowSetPnt)
{
}

TZWThermo::~TZWThermo()
{
}


// ---------------------------------------------------------------------------
//  TZWThermo: Public operators
// ---------------------------------------------------------------------------
TZWThermo& TZWThermo::operator=(const TZWThermo& unitSrc)
{
    if (this != &unitSrc)
    {
        TParent::operator=(unitSrc);

        m_c4FldId_CurTemp = unitSrc.m_c4FldId_CurTemp;
        m_c4FldId_FanMode = unitSrc.m_c4FldId_FanMode;
        m_c4FldId_FanState = unitSrc.m_c4FldId_FanState;
        m_c4FldId_Mode = unitSrc.m_c4FldId_Mode;
        m_c4FldId_State = unitSrc.m_c4FldId_State;

        m_c4FldId_HighSetPnt = unitSrc.m_c4FldId_HighSetPnt;
        m_c4FldId_LowSetPnt = unitSrc.m_c4FldId_LowSetPnt;
    }
    return *this;
}


// We don't have any of our own persisted members, so just call the parent for now
tCIDLib::TBoolean TZWThermo::operator==(const TZWThermo& unitSrc) const
{
    return TParent::operator==(unitSrc);
}


tCIDLib::TBoolean TZWThermo::operator!=(const TZWThermo& unitSrc) const
{
    return !operator==(unitSrc);
}


// ---------------------------------------------------------------------------
//  TZWThermo: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  We will round robin through our possible values, first asking for the
//  temp, then the mode, then the state. These devices will almost certainly
//  do async reporting, so this isn't too much of a problem.
//
tCIDLib::TBoolean
TZWThermo::bBuildGetMsg(TString& strToFill, const tCIDLib::TCard4 c4Index)
{
    if (c4Index == 0)
    {
        // Do a multi-level sensor report request, to get the current temp
        strToFill = L"N";
        strToFill.AppendFormatted(c4Id());
        strToFill.Append(kCIDLib::chComma);
        strToFill.Append(L"SE,");
        strToFill.AppendFormatted(tCIDLib::TCard4(COMMAND_CLASS_SENSOR_MULTILEVEL));
        strToFill.Append(L",");
        strToFill.AppendFormatted(tCIDLib::TCard4(SENSOR_MULTILEVEL_GET));
    }
     else if (c4Index == 1)
    {
        // Query the fan state
        strToFill = L"N";
        strToFill.AppendFormatted(c4Id());
        strToFill.Append(kCIDLib::chComma);
        strToFill.Append(L"SE,");
        strToFill.AppendFormatted(tCIDLib::TCard4(COMMAND_CLASS_THERMOSTAT_FAN_STATE));
        strToFill.Append(L",");
        strToFill.AppendFormatted(tCIDLib::TCard4(THERMOSTAT_FAN_STATE_GET));
    }
     else if (c4Index == 2)
    {
        // Query the current state
        strToFill = L"N";
        strToFill.AppendFormatted(c4Id());
        strToFill.Append(kCIDLib::chComma);
        strToFill.Append(L"SE,");
        strToFill.AppendFormatted(tCIDLib::TCard4(COMMAND_CLASS_THERMOSTAT_OPERATING_STATE));
        strToFill.Append(L",");
        strToFill.AppendFormatted(tCIDLib::TCard4(THERMOSTAT_OPERATING_STATE_GET));
    }

     else if ((c4Index == 3) || (c4Index == 4))
    {
        // Only done if we are async, else we only report 3
        strToFill = L"N";
        strToFill.AppendFormatted(c4Id());
        strToFill.Append(kCIDLib::chComma);
        strToFill.Append(L"SE,");
        strToFill.AppendFormatted(tCIDLib::TCard4(COMMAND_CLASS_THERMOSTAT_SETPOINT));
        strToFill.Append(L",");
        strToFill.AppendFormatted(tCIDLib::TCard4(THERMOSTAT_SETPOINT_GET));

        // Either cooling or heating
        if (c4Index == 3)
            strToFill.Append(L",2");
        else
            strToFill.Append(L",1");
    }
     else if (c4Index == 5)
    {
        // Query the current mode, only ever invoked if async mode
        strToFill = L"N";
        strToFill.AppendFormatted(c4Id());
        strToFill.Append(kCIDLib::chComma);
        strToFill.Append(L"SE,");
        strToFill.AppendFormatted(tCIDLib::TCard4(COMMAND_CLASS_THERMOSTAT_MODE));
        strToFill.Append(L",");
        strToFill.AppendFormatted(tCIDLib::TCard4(THERMOSTAT_MODE_GET));
    }

    return kCIDLib::True;
}

//
tCIDLib::TBoolean
TZWThermo::bBuildSetMsg(const   tCIDLib::TCard4 c4FldId
                        , const TString&        strValue
                        ,       TString&        strToFill
                        ,       TCQCServerBase& sdrvLevi)
{
    if (c4FldId == m_c4FldId_FanMode)
    {
        // Get the limits object and map the value
        const TCQCFldEnumLimit* pfldlEnum = static_cast<const TCQCFldEnumLimit*>
        (
            sdrvLevi.pfldlLimsFor(m_c4FldId_FanMode, TCQCFldEnumLimit::clsThis())
        );

        if (!pfldlEnum)
            return kCIDLib::False;

        // Get the value. If not found, we fail
        tCIDLib::TCard4 c4Val = pfldlEnum->c4QueryOrdinal(strValue, kCIDLib::False);
        if (c4Val == kCIDLib::c4MaxCard)
            return kCIDLib::False;

        // And we can build up the message
        strToFill = L"N";
        strToFill.AppendFormatted(c4Id());
        strToFill.Append(kCIDLib::chComma);
        strToFill.Append(L"SE,");
        strToFill.AppendFormatted(tCIDLib::TCard4(COMMAND_CLASS_THERMOSTAT_FAN_MODE));
        strToFill.Append(L",");
        strToFill.AppendFormatted(tCIDLib::TCard4(THERMOSTAT_FAN_MODE_SET));
        strToFill.Append(L",");
        strToFill.AppendFormatted(c4Val);
    }
     else if (c4FldId == m_c4FldId_Mode)
    {
        // Get the limits object and map the value
        const TCQCFldEnumLimit* pfldlEnum = static_cast<const TCQCFldEnumLimit*>
        (
            sdrvLevi.pfldlLimsFor(m_c4FldId_Mode, TCQCFldEnumLimit::clsThis())
        );

        if (!pfldlEnum)
            return kCIDLib::False;

        // Get the value. If not found, we fail
        tCIDLib::TCard4 c4Val = pfldlEnum->c4QueryOrdinal(strValue, kCIDLib::False);
        if (c4Val == kCIDLib::c4MaxCard)
            return kCIDLib::False;

        // And we can build up the message
        strToFill = L"N";
        strToFill.AppendFormatted(c4Id());
        strToFill.Append(kCIDLib::chComma);
        strToFill.Append(L"SE,");
        strToFill.AppendFormatted(tCIDLib::TCard4(COMMAND_CLASS_THERMOSTAT_MODE));
        strToFill.Append(L",");
        strToFill.AppendFormatted(tCIDLib::TCard4(THERMOSTAT_MODE_SET));
        strToFill.Append(L",");
        strToFill.AppendFormatted(c4Val);
    }
     else if ((c4FldId == m_c4FldId_LowSetPnt) || (c4FldId == m_c4FldId_HighSetPnt))
    {
        BuildSPSetMsg
        (
            c4FldId == m_c4FldId_LowSetPnt ? 1 : 2
            , c4OptInfo() == 0
            , strValue
            , strToFill
        );
    }
     else
    {
        // Not something we understand
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


tCIDLib::TBoolean TZWThermo::bOwnsFld(const tCIDLib::TCard4 c4FldId) const
{
    return  (m_c4FldId_CurTemp == c4FldId)
            || (m_c4FldId_FanMode == c4FldId)
            || (m_c4FldId_FanState == c4FldId)
            || (m_c4FldId_Mode == c4FldId)
            || (m_c4FldId_State == c4FldId)

            // If we never set these, they will be max card, so they won't match
            || (m_c4FldId_LowSetPnt == c4FldId)
            || (m_c4FldId_HighSetPnt == c4FldId);
}


// We don't get anything via the basic report that we need to store
tCIDLib::TBoolean
TZWThermo::bStoreBasicReport(const  tCIDLib::TCard4 c4Value
                            ,       TCQCServerBase& sdrvLevi)
{
    //
    //  Update our last poll time to now since we got a new value. Tell it
    //  to update the last value stamp as well.
    //
    ResetPollTime(kCIDLib::True);

    return kCIDLib::True;
}


tCIDLib::TBoolean
TZWThermo::bStoreFldIds(const   TVector<TCQCFldDef>&    colFlds
                        , const tZWaveLeviSh::TValList& fcolIds)
{
    const TString strCurTemp(strName(), L"_CurTemp");
    const TString strFanMode(strName(), L"_FanMode");
    const TString strFanState(strName(), L"_FanState");
    const TString strMode(strName(), L"_Mode");
    const TString strState(strName(), L"_State");

    const TString strLowSP(strName(), L"_LowSetPnt");
    const TString strHighSP(strName(), L"_HighSetPnt");

    m_c4FldId_CurTemp = kCIDLib::c4MaxCard;
    m_c4FldId_FanMode = kCIDLib::c4MaxCard;
    m_c4FldId_FanState = kCIDLib::c4MaxCard;
    m_c4FldId_Mode = kCIDLib::c4MaxCard;
    m_c4FldId_State = kCIDLib::c4MaxCard;

    m_c4FldId_LowSetPnt = kCIDLib::c4MaxCard;
    m_c4FldId_HighSetPnt = kCIDLib::c4MaxCard;

    // Find our fields and store the ids
    const tCIDLib::TCard4 c4Count = colFlds.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TCQCFldDef& flddCur = colFlds[c4Index];

        if (flddCur.strName() == strCurTemp)
            m_c4FldId_CurTemp = fcolIds[c4Index];

        else if (flddCur.strName() == strFanMode)
            m_c4FldId_FanMode = fcolIds[c4Index];

        else if (flddCur.strName() == strFanState)
            m_c4FldId_FanState = fcolIds[c4Index];

        else if (flddCur.strName() == strMode)
            m_c4FldId_Mode = fcolIds[c4Index];

        else if (flddCur.strName() == strState)
            m_c4FldId_State = fcolIds[c4Index];

        else if (flddCur.strName() == strLowSP)
            m_c4FldId_LowSetPnt = fcolIds[c4Index];
        else if (flddCur.strName() == strHighSP)
            m_c4FldId_HighSetPnt = fcolIds[c4Index];
    }

    // Return true if we got all our fields
    tCIDLib::TBoolean bRes
    (
        (m_c4FldId_CurTemp != kCIDLib::c4MaxCard)
        && (m_c4FldId_FanMode != kCIDLib::c4MaxCard)
        && (m_c4FldId_FanState != kCIDLib::c4MaxCard)
        && (m_c4FldId_Mode != kCIDLib::c4MaxCard)
        && (m_c4FldId_State != kCIDLib::c4MaxCard)
    );

    // If not already failed, and we are async, test the set points
    if (bRes && bHasCap(tZWaveLeviSh::EUnitCap_AsyncNotify))
    {
        if ((m_c4FldId_LowSetPnt == kCIDLib::c4MaxCard)
        ||  (m_c4FldId_HighSetPnt == kCIDLib::c4MaxCard))
        {
            bRes = kCIDLib::False;
        }
    }
    return bRes;
}


//
//  Handles non-basic type reports, which we'll get for
//
tCIDLib::TBoolean
TZWThermo::bStoreGenReport( const   tCIDLib::TCard4 ac4Values[]
                            , const tCIDLib::TCard4 c4ValCnt
                            ,       TCQCServerBase& sdrvLevi)
{
    // Shouldn't get anything less than 2, but don't tempt fate
    if (c4ValCnt < 2)
        return kCIDLib::False;

    if ((ac4Values[0] == COMMAND_CLASS_THERMOSTAT_MODE)
    &&  ((ac4Values[1] == THERMOSTAT_MODE_GET)
    ||   (ac4Values[1] == THERMOSTAT_MODE_REPORT)))
    {
        // Ignore unless we are marked async, else it's write only
        if (bHasCap(tZWaveLeviSh::EUnitCap_AsyncNotify))
            StoreMode(ac4Values[2], sdrvLevi);
    }
     else if ((ac4Values[0] == COMMAND_CLASS_SENSOR_MULTILEVEL)
          &&  ((ac4Values[1] == SENSOR_MULTILEVEL_GET)
          ||   (ac4Values[1] == SENSOR_MULTILEVEL_REPORT)))
    {
        // It's the current temp
        StoreTemp(ac4Values, c4ValCnt, sdrvLevi);
    }
     else if ((ac4Values[0] == COMMAND_CLASS_THERMOSTAT_FAN_MODE)
          &&  ((ac4Values[1] == THERMOSTAT_FAN_MODE_GET)
          ||   (ac4Values[1] == THERMOSTAT_FAN_MODE_REPORT)))
    {
        // Write only, we don't store it
    }
     else if ((ac4Values[0] == COMMAND_CLASS_THERMOSTAT_FAN_STATE)
          &&  ((ac4Values[1] == THERMOSTAT_FAN_STATE_GET)
          ||   (ac4Values[1] == THERMOSTAT_FAN_STATE_REPORT)))
    {
        // It's the fan mode
        StoreFanState(ac4Values[2], sdrvLevi);
    }
     else if ((ac4Values[0] == COMMAND_CLASS_THERMOSTAT_OPERATING_STATE)
          &&  ((ac4Values[1] == THERMOSTAT_OPERATING_STATE_GET)
          ||   (ac4Values[1] == THERMOSTAT_OPERATING_STATE_REPORT)))
    {
        StoreState(ac4Values[2], sdrvLevi);
    }
     else if ((ac4Values[0] == COMMAND_CLASS_THERMOSTAT_SETPOINT)
          &&  ((ac4Values[1] == THERMOSTAT_SETPOINT_GET)
          ||   (ac4Values[1] == THERMOSTAT_SETPOINT_REPORT)))
    {
        // Ignore unless we are marked async, else these fields don't exist
        if (bHasCap(tZWaveLeviSh::EUnitCap_AsyncNotify))
            StoreSP(ac4Values, c4ValCnt, sdrvLevi);
    }
     else
    {
        // Nothing of ours specifically so pass to the base class
        return TParent::bStoreGenReport(ac4Values, c4ValCnt, sdrvLevi);
    }
    return kCIDLib::True;
}


//
//  Return our optional info strings. Zero is Celsius and 1 is Fahrenheit.
//
tCIDLib::TBoolean
TZWThermo::bWantsOptInfo(tCIDLib::TStrCollect& colToFill) const
{
    colToFill.objAdd(facZWaveLeviSh().strMsg(kZWShMsgs::midOpt_Thermo_Celsius));
    colToFill.objAdd(facZWaveLeviSh().strMsg(kZWShMsgs::midOpt_Thermo_Fahren));

    return kCIDLib::True;
}


tCIDLib::TCard4
TZWThermo::c4QueryFldDefs(  TVector<TCQCFldDef>&    colAddTo
                            , TCQCFldDef&           flddTmp) const
{
    const tCIDLib::TBoolean bAsync(bHasCap(tZWaveLeviSh::EUnitCap_AsyncNotify));
    TString strTmp;

    // Do the current temp field
    strTmp = strName();
    strTmp.Append(L"_CurTemp");
    flddTmp.Set
    (
        strTmp
        , tCQCKit::EFldTypes::Int
        , tCQCKit::EFldAccess::Read
        , tCQCKit::EFldSTypes::CurTemp
    );
    colAddTo.objAdd(flddTmp);

    // Do the fan mode setting field
    strTmp = strName();
    strTmp.Append(L"_FanMode");
    flddTmp.Set
    (
        strTmp
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Write
        , L"Enum: AutoLow, OnLow, AutoHigh, OnHigh"
    );
    colAddTo.objAdd(flddTmp);

    // Do the fan state field
    strTmp = strName();
    strTmp.Append(L"_FanState");
    flddTmp.Set
    (
        strTmp
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Read
        , L"Enum: Idle, Low, High, Unknown"
    );
    colAddTo.objAdd(flddTmp);

    // Do the operating mode field (readable if async)
    strTmp = strName();
    strTmp.Append(L"_Mode");
    flddTmp.Set
    (
        strTmp
        , tCQCKit::EFldTypes::String
        , bAsync ? tCQCKit::EFldAccess::ReadWrite : tCQCKit::EFldAccess::Write
        , L"Enum: Off, Heat, Cool, Auto, Aux, Resume, Fan, Furnace, Dry, Moist, Auto"
    );
    colAddTo.objAdd(flddTmp);

    // Do the current state
    strTmp = strName();
    strTmp.Append(L"_State");
    flddTmp.Set
    (
        strTmp
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Read
        , L"Enum: Idle, Heating, Cooling, Fan, PendingHeat, PendingCool, Vent, Unknown"
    );
    colAddTo.objAdd(flddTmp);

    // We have at least this many
    tCIDLib::TCard4 c4Ret = 5;

    // If async add read/write set points
    if (bAsync)
    {
        strTmp = strName();
        strTmp.Append(L"_LowSetPnt");
        flddTmp.Set
        (
            strTmp
            , tCQCKit::EFldTypes::Int
            , tCQCKit::EFldAccess::ReadWrite
            , tCQCKit::EFldSTypes::LowSetPnt
            , (c4OptInfo() == 0) ? L"Range: 4, 43" : L"Range: 40, 109"
        );
        colAddTo.objAdd(flddTmp);

        strTmp = strName();
        strTmp.Append(L"_HighSetPnt");
        flddTmp.Set
        (
            strTmp
            , tCQCKit::EFldTypes::Int
            , tCQCKit::EFldAccess::ReadWrite
            , tCQCKit::EFldSTypes::HighSetPnt
            , (c4OptInfo() == 0) ? L"Range: 5, 45" : L"Range: 44, 113"
        );
        colAddTo.objAdd(flddTmp);

        // Add these to the count
        c4Ret += 2;
    }
    return c4Ret;
}


// Call our parent first, reset our runtime stuff
tCIDLib::TVoid TZWThermo::PrepForUse()
{
    TParent::PrepForUse();

    // Mark all the fields not in use
    m_c4FldId_CurTemp = kCIDLib::c4MaxCard;
    m_c4FldId_FanMode = kCIDLib::c4MaxCard;
    m_c4FldId_FanState = kCIDLib::c4MaxCard;
    m_c4FldId_Mode = kCIDLib::c4MaxCard;
    m_c4FldId_State = kCIDLib::c4MaxCard;

    m_c4FldId_HighSetPnt = kCIDLib::c4MaxCard;
    m_c4FldId_LowSetPnt = kCIDLib::c4MaxCard;
}


// Set all of our readable device based fields to error state
tCIDLib::TVoid TZWThermo::SetFieldErrs(TCQCServerBase& sdrvLevi)
{
    if (m_c4FldId_CurTemp != kCIDLib::c4MaxCard)
        sdrvLevi.SetFieldErr(m_c4FldId_CurTemp, kCIDLib::True);

    if (bHasCap(tZWaveLeviSh::EUnitCap_AsyncNotify)
    &&  (m_c4FldId_Mode != kCIDLib::c4MaxCard))
    {
        sdrvLevi.SetFieldErr(m_c4FldId_Mode, kCIDLib::True);
    }

    if (m_c4FldId_FanState != kCIDLib::c4MaxCard)
        sdrvLevi.SetFieldErr(m_c4FldId_FanState, kCIDLib::True);

    if (m_c4FldId_State != kCIDLib::c4MaxCard)
        sdrvLevi.SetFieldErr(m_c4FldId_State, kCIDLib::True);

    if (m_c4FldId_HighSetPnt != kCIDLib::c4MaxCard)
        sdrvLevi.SetFieldErr(m_c4FldId_HighSetPnt, kCIDLib::True);

    if (m_c4FldId_LowSetPnt != kCIDLib::c4MaxCard)
        sdrvLevi.SetFieldErr(m_c4FldId_LowSetPnt, kCIDLib::True);
}


// ---------------------------------------------------------------------------
//  TZWThermo: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  A helper to set a set point. It's used both internally if we are in async
//  mode (and thus support set points directly as fields) and get a field
//  write, or if the driver calls us for a write to the Command field to set
//  a set point.
//
tCIDLib::TVoid
TZWThermo::BuildSPSetMsg(const  tCIDLib::TCard4     c4SPId
                        , const tCIDLib::TBoolean   bCelsius
                        , const TString&            strValue
                        ,       TString&            strToFill) const
{
    // And we can build up the message
    strToFill = L"N";
    strToFill.AppendFormatted(c4Id());
    strToFill.Append(kCIDLib::chComma);
    strToFill.Append(L"SE,");
    strToFill.AppendFormatted(tCIDLib::TCard4(COMMAND_CLASS_THERMOSTAT_SETPOINT));
    strToFill.Append(L",");
    strToFill.AppendFormatted(tCIDLib::TCard4(THERMOSTAT_SETPOINT_SET));
    strToFill.Append(L',');

    // Setpoint type. High byte is reserved zeor which it will be here
    strToFill.AppendFormatted(c4SPId);

    // Precision 0, temp scale 1/F or 0/C, and size 1 byte (1001 or 0001)
    if (bCelsius)
        strToFill.Append(L",8,");
    else
        strToFill.Append(L",9,");

    //
    //  Convert to a 1 byte int. In reality they will never be negative but
    //  we can't assume that.
    //
    tCIDLib::TInt1 i1Val = tCIDLib::TInt1(strValue.i4Val(tCIDLib::ERadices::Dec));
    strToFill.AppendFormatted(i1Val);
}


// ---------------------------------------------------------------------------
//  TZWThermo: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TZWThermo::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Make sure we get the start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > ZWaveLeviSh_Thermo::c2FmtThermoVer))
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

    // Call our parent class first
    TParent::StreamFrom(strmToReadFrom);

    // We don't have anything of our own yet

    // Make sure we get the end marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);

    //
    //  If we were V1, then force the opt info to 1, so that we default to
    //  F scale with the new options info we are using to allow that
    //  selection.
    //
    if (c2FmtVersion == 1)
        c4OptInfo(1);

    // Our number of queries is not fixed, so update based on config
    if (bHasCap(tZWaveLeviSh::EUnitCap_AsyncNotify))
        c4GetMsgCnt(6);
    else
        c4GetMsgCnt(3);
}


tCIDLib::TVoid TZWThermo::StreamTo(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << ZWaveLeviSh_Thermo::c2FmtThermoVer;

    // Do our parent's stuff now
    TParent::StreamTo(strmToWriteTo);

    // Nothing of our own yet, so just put out the end marker

    strmToWriteTo   << tCIDLib::EStreamMarkers::EndObject;
}


// ---------------------------------------------------------------------------
//  TZWThermo: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  This is called when we get a mode report or response. We translate the
//  value to the enumerated value and store the result in the mode field.
//
tCIDLib::TVoid
TZWThermo::StoreFanState(const tCIDLib::TCard4 c4Val, TCQCServerBase& sdrvLevi)
{
    if (sdrvLevi.eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
    {
        facZWaveLeviSh().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"Got fan state '%(1)' for thermo %(2)"
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
            , TCardinal(c4Val)
            , strName()
        );
    }

    if (!bCheckFldId(m_c4FldId_FanState, L"fan state"))
        return;

    // Get the limits object for the fan mode field
    const TCQCFldEnumLimit* pfldlEnum = static_cast<const TCQCFldEnumLimit*>
    (
        sdrvLevi.pfldlLimsFor(m_c4FldId_FanState, TCQCFldEnumLimit::clsThis())
    );

    // Should always be there, but just in case. And check the value range
    if (!pfldlEnum || (c4Val >= pfldlEnum->c4ValCount()))
    {
        // Unknown, put the field into error
        sdrvLevi.SetFieldErr(m_c4FldId_FanState, kCIDLib::True);
        return;
    }

    //
    //  Update our last poll time to now since we got a new value. Tell it to
    //  reset the last value time stamp as well.
    //
    ResetPollTime(kCIDLib::True);

    // Get our enum value for the Z-Wave ordinal
    const TString& strVal = pfldlEnum->strValueAt(c4Val);

    // Store the value
    sdrvLevi.bStoreStringFld(m_c4FldId_FanState, strVal, kCIDLib::True);
}


//
//  This is called when we get a mode report or response. We translate the
//  value to the enumerated value and store the result in the mode field.
//
tCIDLib::TVoid
TZWThermo::StoreMode(const tCIDLib::TCard4 c4Val, TCQCServerBase& sdrvLevi)
{
    if (sdrvLevi.eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
    {
        facZWaveLeviSh().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"Got mode '%(1)' for thermo %(2)"
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
            , TCardinal(c4Val)
            , strName()
        );
    }

    if (!bCheckFldId(m_c4FldId_Mode, L"mode"))
        return;

    // Get the limits object for the state field
    const TCQCFldEnumLimit* pfldlEnum = static_cast<const TCQCFldEnumLimit*>
    (
        sdrvLevi.pfldlLimsFor(m_c4FldId_Mode, TCQCFldEnumLimit::clsThis())
    );

    // Should always be there, but just in case. And check the value range
    if (!pfldlEnum || (c4Val >= pfldlEnum->c4ValCount()))
    {
        facZWaveLeviSh().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"%(1) is not a valid mode value. Thermo=%(2)"
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
            , TCardinal(c4Val)
            , strName()
        );

        // Unknown, put the field into error
        sdrvLevi.SetFieldErr(m_c4FldId_State, kCIDLib::True);
        return;
    }

    //
    //  Update our last poll time to now since we got a new value. Tell it to
    //  reset the last value time stamp as well.
    //
    ResetPollTime(kCIDLib::True);

    // Get our enum value for the Z-Wave ordinal
    const TString& strVal = pfldlEnum->strValueAt(c4Val);

    // Lock and store the value
    sdrvLevi.bStoreStringFld(m_c4FldId_Mode, strVal, kCIDLib::True);
}


//
//  This is called when we get a state report or response. We translate the
//  value to the enumerated value and store the result in the operating
//  state field.
//
tCIDLib::TVoid
TZWThermo::StoreState(const tCIDLib::TCard4 c4Val, TCQCServerBase& sdrvLevi)
{
    if (sdrvLevi.eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
    {
        facZWaveLeviSh().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"Got operating state '%(1)' for thermo %(2)"
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
            , TCardinal(c4Val)
            , strName()
        );
    }

    if (!bCheckFldId(m_c4FldId_State, L"state"))
        return;

    // Get the limits object for the state field
    const TCQCFldEnumLimit* pfldlEnum = static_cast<const TCQCFldEnumLimit*>
    (
        sdrvLevi.pfldlLimsFor(m_c4FldId_State, TCQCFldEnumLimit::clsThis())
    );

    // Should always be there, but just in case. And check the value range
    if (!pfldlEnum || (c4Val >= pfldlEnum->c4ValCount()))
    {
        facZWaveLeviSh().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"%(1) is not a valid state value. Thermo=%(2)"
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
            , TCardinal(c4Val)
            , strName()
        );

        // Unknown, put the field into error
        sdrvLevi.SetFieldErr(m_c4FldId_State, kCIDLib::True);
        return;
    }

    //
    //  Update our last poll time to now since we got a new value. Tell it to
    //  reset the last value time stamp as well.
    //
    ResetPollTime(kCIDLib::True);

    // Get our enum value for the Z-Wave ordinal
    const TString& strVal = pfldlEnum->strValueAt(c4Val);

    // Store the value
    sdrvLevi.bStoreStringFld(m_c4FldId_State, strVal, kCIDLib::True);
}


tCIDLib::TVoid
TZWThermo::StoreSP( const   tCIDLib::TCard4     ac4Values[]
                    , const tCIDLib::TCard4     c4ValCnt
                    ,       TCQCServerBase&     sdrvLevi)
{
    // Make sure it's either low or high set point
    tCIDLib::TCard4 c4FldId;
    const tCIDLib::TCh* pszFld = L"";
    if (ac4Values[2] == 1)
    {
        c4FldId = m_c4FldId_LowSetPnt;
        pszFld = L"low set point";
    }
     else if (ac4Values[2] == 2)
    {
        c4FldId = m_c4FldId_HighSetPnt;
        pszFld = L"high set point";
    }
     else
    {
        // Ignore it, not one we care about
        return;
    }

    // Log what we got if verbose
    if (sdrvLevi.eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
    {
        facZWaveLeviSh().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"Got %(1) setpoint for %(2)"
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
            , TString(pszFld)
            , strName()
        );
    }

    if (!bCheckFldId(c4FldId, pszFld))
        return;

    // Use a helper to get the value
    tCIDLib::TInt4 i4Temp;
    const tCIDLib::TBoolean bRes = bDecodeValue
    (
        &ac4Values[3], c4ValCnt - 3, i4Temp, pszFld, sdrvLevi
    );

    // If not valid, upt it into error state, else store it
    if (bRes)
    {
        //
        //  Update our last poll time to now since we got a new value. Tell
        //  it to reset the last value time stamp as well.
        //
        ResetPollTime(kCIDLib::True);
        sdrvLevi.bStoreIntFld(c4FldId, i4Temp, kCIDLib::True);
    }
     else
    {
        sdrvLevi.SetFieldErr(c4FldId, kCIDLib::True);
    }
}


//
//  This is called when we get a current temp report, via the multi-level
//  sensor class.
//
tCIDLib::TVoid
TZWThermo::StoreTemp(const  tCIDLib::TCard4 ac4Values[]
                    , const tCIDLib::TCard4 c4ValCnt
                    ,       TCQCServerBase& sdrvLevi)
{
    if (sdrvLevi.eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
    {
        facZWaveLeviSh().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"Got temp for thermo %(1)"
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
            , strName()
        );
    }

    // If it's not a temperature, then it's bad
    if (ac4Values[2] != 0x1)
    {
        facZWaveLeviSh().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"Temp report for thermo '%(1)' was not marked as a temperature."
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
            , strName()
        );
        return;
    }

    if (!bCheckFldId(m_c4FldId_CurTemp, L"current temp"))
        return;

    // Use a helper to get the value
    tCIDLib::TInt4 i4Temp;
    const tCIDLib::TBoolean bRes = bDecodeValue
    (
        &ac4Values[3]
        , c4ValCnt - 3
        , i4Temp
        , L"current temp"
        , sdrvLevi
    );

    // If not valid, upt it into error state, else store it
    if (bRes)
    {
        //
        //  Update our last poll time to now since we got a new value. Tell
        //  it to reset the last value time stamp as well.
        //
        ResetPollTime(kCIDLib::True);
        sdrvLevi.bStoreIntFld(m_c4FldId_CurTemp, i4Temp, kCIDLib::True);
    }
     else
    {
        sdrvLevi.SetFieldErr(m_c4FldId_CurTemp, kCIDLib::True);
    }
}

