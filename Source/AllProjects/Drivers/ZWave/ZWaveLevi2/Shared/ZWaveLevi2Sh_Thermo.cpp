//
// FILE NAME: ZWaveLevi2Sh_Thermo.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/31/2014
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
//  This file provides the implementation for the thermostat command class derivative.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "ZWaveLevi2Sh_.hpp"



// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TZWCCThermoCTemp, TZWCmdClass)
RTTIDecls(TZWCCThermoFM, TZWCmdClass)
RTTIDecls(TZWCCThermoFOM, TZWCmdClass)
RTTIDecls(TZWCCThermoSP, TZWCmdClass)
RTTIDecls(TZWCCThermoMode, TZWCmdClass)
RTTIDecls(TZWCCThermoOMode, TZWCmdClass)



// ---------------------------------------------------------------------------
//   CLASS: TZWCCThermoCTemp
//  PREFIX: zwcc
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWCCThermoCTemp: Constructors Destructor
// ---------------------------------------------------------------------------
TZWCCThermoCTemp::TZWCCThermoCTemp(const TZWDevClass& zwdcInst) :

    TZWCmdClass(tZWaveLevi2Sh::ECClasses::ThermoCurTemp, zwdcInst, COMMAND_CLASS_SENSOR_MULTILEVEL)
    , m_c4FldId_CurTemp(kCIDLib::c4MaxCard)
{
    // Pre-build our option keys
    BuildOptKey(L"ThermoTemp_Scale", m_strOptKey_Scale);
}

TZWCCThermoCTemp::~TZWCCThermoCTemp()
{
}


// ---------------------------------------------------------------------------
//  TZWCCThermoCTemp: Public, inherited methods
// ---------------------------------------------------------------------------

//
tCIDLib::TBoolean
TZWCCThermoCTemp::bHandleGenReport( const   tCIDLib::TCardList& fcolVals
                                    , const TZWaveUnit&         unitMe
                                    ,       MZWLeviSrvFuncs&    mzwsfLevi
                                    , const tCIDLib::TCard4     c4FromInst)
{
    // Shouldn't get anything less than 2, but don't tempt fate
    const tCIDLib::TCard4 c4ValCnt = fcolVals.c4ElemCount();
    if (c4ValCnt < 2)
        return kCIDLib::False;

    if ((c4FromInst == c4InstId())
    &&  (fcolVals[0] == COMMAND_CLASS_SENSOR_MULTILEVEL)
    &&  (fcolVals[1] == SENSOR_MULTILEVEL_REPORT))
    {
        // It's the current temp
        StoreTemp(fcolVals, unitMe, mzwsfLevi);
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


tCIDLib::TBoolean TZWCCThermoCTemp::bOwnsFld(const tCIDLib::TCard4 c4FldId) const
{
    return  (m_c4FldId_CurTemp == c4FldId);
}


//
//  This is called when the driver wants us to actively query our field values and
//  store them away.
//
tCIDLib::TBoolean
TZWCCThermoCTemp::bQueryFldVals(TZWaveUnit& unitMe, MZWLeviSrvFuncs& mzwsfLevi)
{
    tCIDLib::TCard4 c4CmdClass;
    tCIDLib::TCard4 c4Get;
    tCIDLib::TCard4 c4Report;

    if (unitMe.eBaseClass() == eCmdClass())
    {
        c4CmdClass = COMMAND_CLASS_BASIC;
        c4Get      = BASIC_GET;
        c4Report   = BASIC_REPORT;
    }
     else
    {
        c4CmdClass = COMMAND_CLASS_SENSOR_MULTILEVEL;
        c4Get      = SENSOR_MULTILEVEL_GET;
        c4Report   = SENSOR_MULTILEVEL_REPORT;
    }

    // Do a multi-level sensor report request, to get the current temp
    mzwsfLevi.LeviSendUnitMsg(unitMe, *this, c4CmdClass, c4Get);

    tCIDLib::TCardList fcolVals(8);
    const tCIDLib::TBoolean bRes = mzwsfLevi.bLeviWaitUnitMsg
    (
        unitMe, *this, c4CmdClass, c4Report, fcolVals
    );

    if (bRes)
        bHandleGenReport(fcolVals, unitMe, mzwsfLevi, c4InstId());
    else
        mzwsfLevi.LeviSetFldErr(m_c4FldId_CurTemp);

    return kCIDLib::True;
}


//
//  Add any of our prompts to the passed list.
//
tCIDLib::TVoid TZWCCThermoCTemp::LoadOptList(tZWaveLevi2Sh::TOptList& colToFill) const
{
    TZWOption& zwoptScale = colToFill.objAdd(TZWOption());
    zwoptScale.m_strKey = m_strOptKey_Scale;
    zwoptScale.m_strPrompt = L"Temperature Scale";
    zwoptScale.m_strInstDescr = strInstDescr();
    zwoptScale.m_colValues.objAdd(TKeyValuePair(L"F", L"Fahrenheit"));
    zwoptScale.m_colValues.objAdd(TKeyValuePair(L"C", L"Celsius"));
}


tCIDLib::TVoid
TZWCCThermoCTemp::LoadOptVals(  const   TZWDevClass&                zwdcInst
                                ,       tZWaveLevi2Sh::TOptValList& colToFill
                                ,       TZWOptionVal&               zwoptvTmp) const
{
    if (!colToFill.bKeyExists(m_strOptKey_Scale))
    {
        zwdcInst.QueryOptDef(zwoptvTmp, m_strOptKey_Scale, L"F");
        colToFill.objAdd(zwoptvTmp);
    }
}

// Make sure our fields are set to error state until good values are stored
tCIDLib::TVoid
TZWCCThermoCTemp::InitVals(const TZWaveUnit& unitMe,  MZWLeviSrvFuncs& mzwsfLevi)
{
    mzwsfLevi.LeviSetFldErr(m_c4FldId_CurTemp);
}


//
//  Add our fields to the list. We either add just a generic multi-level value, or
//  a standard V2 dimmer field, depending on configuration.
//
tCIDLib::TVoid
TZWCCThermoCTemp::QueryFldDefs(         tCQCKit::TFldDefList& colAddTo
                                ,       TCQCFldDef&           flddTmp
                                , const TZWaveUnit&           unitMe
                                ,       TString&              strTmp1
                                ,       TString&              strTmp2) const
{

    const TString& strScale = unitMe.strFindOptVal(m_strOptKey_Scale);

    if (strScale == L"F")
        strTmp1 = L"Range: -20, 120";
    else
        strTmp1 = L"Range: -28, 50";

    // We just support the standard fields so we can use a helper for everything
    strTmp2 = L"THERM#";
    AppendInstName(unitMe, kCIDLib::pszEmptyZStr, strTmp2);
    strTmp2.Append(L"~CurrentTemp");

    flddTmp.Set
    (
        strTmp2
        , tCQCKit::EFldTypes::Int
        , tCQCKit::EFldAccess::Read
        , tCQCKit::EFldSTypes::CurTemp
        , strTmp1
    );
    colAddTo.objAdd(flddTmp);
}


// We look our field and store it away
tCIDLib::TVoid
TZWCCThermoCTemp::StoreFldIds(  const   TZWaveUnit&         unitMe
                                , const MZWLeviSrvFuncs&    mzwsfLevi
                                ,       TString&            strTmp1
                                ,       TString&            strTmp2)
{
    strTmp1 = L"THERM#";
    AppendInstName(unitMe, kCIDLib::pszEmptyZStr, strTmp1);
    strTmp1.Append(L"~CurrentTemp");
    m_c4FldId_CurTemp = mzwsfLevi.c4LeviFldIdFromName(strTmp1);
}


// ---------------------------------------------------------------------------
//  TZWCCThermoCTemp: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  This is called when we get a current temp report, via the multi-level
//  sensor class.
//
tCIDLib::TVoid
TZWCCThermoCTemp::StoreTemp(const   tCIDLib::TCardList&     fcolVals
                            , const TZWaveUnit&             unitMe
                            ,       MZWLeviSrvFuncs&        mzwsfLevi)
{
    if (mzwsfLevi.eLeviVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
    {
        facZWaveLevi2Sh().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"Got temp for thermo %(1)"
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
            , unitMe.strName()
        );
    }

    const tCIDLib::TCard4 c4ValCnt = fcolVals.c4ElemCount();
    if (c4ValCnt < 5)
    {
        facZWaveLevi2Sh().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"Temp report for thermo '%(1)' was too small. Size=%(2)"
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
            , unitMe.strName()
            , TCardinal(c4ValCnt)
        );
        return;
    }

    // If it's not a temperature, then it's bad
    if (fcolVals[2] != 0x1)
    {
        facZWaveLevi2Sh().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"Temp report for thermo '%(1)' was not marked as a temperature."
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
            , unitMe.strName()
        );
        return;
    }

    if (!bCheckFldId(m_c4FldId_CurTemp, L"current temp", unitMe))
        return;

    // Use a helper to get the value. We use the int result
    tCIDLib::TInt4 i4Temp;
    tCIDLib::TFloat8 f8Temp;
    const tCIDLib::TBoolean bRes = bDecodeValue
    (
        fcolVals.ptElements() + 3
        , c4ValCnt - 3
        , kCIDLib::True
        , i4Temp
        , f8Temp
        , L"current temp"
        , mzwsfLevi
    );

    // If not valid, upt it into error state, else store it
    if (bRes)
    {
        mzwsfLevi.bLeviStoreIntFld(m_c4FldId_CurTemp, i4Temp);

        // Make sure we are now marked as having valid info
        bWaitInitVals(kCIDLib::False);
    }
     else
    {
        mzwsfLevi.LeviSetFldErr(m_c4FldId_CurTemp);
    }
}




// ---------------------------------------------------------------------------
//   CLASS: TZWCCThermoFM
//  PREFIX: zwcc
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWCCThermoFM: Constructors Destructor
// ---------------------------------------------------------------------------
TZWCCThermoFM::TZWCCThermoFM(const TZWDevClass& zwdcInst) :

    TZWCmdClass(tZWaveLevi2Sh::ECClasses::ThermoFanMode, zwdcInst, COMMAND_CLASS_THERMOSTAT_FAN_MODE)
    , m_c4FldId_FanMode(kCIDLib::c4MaxCard)
{
}

TZWCCThermoFM::~TZWCCThermoFM()
{
}


// ---------------------------------------------------------------------------
//  TZWCCThermoFM: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TZWCCThermoFM::bHandleGenReport(const   tCIDLib::TCardList& fcolVals
                                , const TZWaveUnit&         unitMe
                                ,       MZWLeviSrvFuncs&    mzwsfLevi
                                , const tCIDLib::TCard4     c4FromInst)
{
    // Shouldn't get anything less than 2, but don't tempt fate
    const tCIDLib::TCard4 c4ValCnt = fcolVals.c4ElemCount();
    if (c4ValCnt < 2)
        return kCIDLib::False;

    if ((c4FromInst == c4InstId())
    &&  (fcolVals[0] == COMMAND_CLASS_THERMOSTAT_FAN_MODE)
    &&  (fcolVals[1] == THERMOSTAT_FAN_MODE_REPORT))
    {
        StoreFanMode(fcolVals[2], unitMe, mzwsfLevi);
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


tCIDLib::TBoolean TZWCCThermoFM::bOwnsFld(const tCIDLib::TCard4 c4FldId) const
{
    return (m_c4FldId_FanMode == c4FldId);
}


//
//  This is called when the driver wants us to actively query our field values and
//  store them away.
//
tCIDLib::TBoolean
TZWCCThermoFM::bQueryFldVals(TZWaveUnit& unitMe, MZWLeviSrvFuncs& mzwsfLevi)
{
    // Query the current mode
    mzwsfLevi.LeviSendUnitMsg
    (
        unitMe, *this, COMMAND_CLASS_THERMOSTAT_FAN_MODE, THERMOSTAT_FAN_MODE_GET
    );

    // And wait for the response.
    tCIDLib::TCardList fcolVals(8);
    const tCIDLib::TBoolean bRes = mzwsfLevi.bLeviWaitUnitMsg
    (
        unitMe, *this, COMMAND_CLASS_THERMOSTAT_FAN_MODE, THERMOSTAT_FAN_MODE_REPORT, fcolVals
    );

    if (bRes)
        bHandleGenReport(fcolVals, unitMe, mzwsfLevi, c4InstId());
    else
        mzwsfLevi.LeviSetFldErr(m_c4FldId_FanMode);

    return kCIDLib::True;
}


tCQCKit::ECommResults
TZWCCThermoFM::eWriteStringFld( const   tCIDLib::TCard4     c4FldId
                                , const TString&            strValue
                                ,       TZWaveUnit&         unitMe
                                ,       MZWLeviSrvFuncs&    mzwsfLevi)
{
    if (c4FldId != m_c4FldId_FanMode)
        return tCQCKit::ECommResults::FieldNotFound;

    // Get the limits object and map the value
    const TCQCFldEnumLimit* pfldlEnum = static_cast<const TCQCFldEnumLimit*>
    (
        mzwsfLevi.pfldlLeviLimsFor(m_c4FldId_FanMode, TCQCFldEnumLimit::clsThis())
    );

    if (!pfldlEnum)
        return tCQCKit::ECommResults::Internal;

    // Get the value. If not found, we fail
    tCIDLib::TCard4 c4Val = pfldlEnum->c4QueryOrdinal(strValue, kCIDLib::False);
    if (c4Val == kCIDLib::c4MaxCard)
        return tCQCKit::ECommResults::BadValue;

    // And we can build up the message
    tCIDLib::TCardList fcolVals(8);
    fcolVals.c4AddElement(COMMAND_CLASS_THERMOSTAT_FAN_MODE);
    fcolVals.c4AddElement(THERMOSTAT_FAN_MODE_SET);
    fcolVals.c4AddElement(c4Val);

    mzwsfLevi.LeviSendUnitMsg(unitMe, *this, fcolVals);

    //
    //  If this device has configured us a known response to wait for, we can
    //  use that as a validation of the operation, and it should give us the
    //  dimmer level as well.
    //
    //  Otherwise, just let the written value be stored, assuming it worked.
    //
    if (c4SetRespCls() && c4SetRespCmd())
    {
        const tCIDLib::TBoolean bRes = mzwsfLevi.bLeviWaitUnitMsg
        (
            unitMe, *this, c4SetRespCls(), c4SetRespCmd(), fcolVals
        );

        if (bRes)
        {
            bHandleGenReport(fcolVals, unitMe, mzwsfLevi, c4InstId());
        }
         else
        {
            facZWaveLevi2Sh().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kZW2ShErrs::errcUnit_NoSetResp
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::NoReply
                , unitMe.strName()
                , tZWaveLevi2Sh::strXlatECClasses(eCmdClass())
            );
        }
    }
    return tCQCKit::ECommResults::Success;
}


// Make sure our fields are set to error state until good values are stored
tCIDLib::TVoid
TZWCCThermoFM::InitVals(const TZWaveUnit& unitMe,  MZWLeviSrvFuncs& mzwsfLevi)
{
    mzwsfLevi.LeviSetFldErr(m_c4FldId_FanMode);
}


//
//  Add our fields to the list. We either add just a generic multi-level value, or
//  a standard V2 dimmer field, depending on configuration.
//
tCIDLib::TVoid
TZWCCThermoFM::QueryFldDefs(        tCQCKit::TFldDefList& colAddTo
                            ,       TCQCFldDef&           flddTmp
                            , const TZWaveUnit&           unitMe
                            ,       TString&              strTmp1
                            ,       TString&              strTmp2) const
{
    strTmp1 = L"THERM#";
    AppendInstName(unitMe, kCIDLib::pszEmptyZStr, strTmp1);
    strTmp1.Append(L"~FanMode");
    flddTmp.Set
    (
        strTmp1
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::ReadWrite
        , L"Enum: AutoLow, OnLow, AutoHigh, OnHigh"
    );
    colAddTo.objAdd(flddTmp);
}


// We look our field and store it away
tCIDLib::TVoid
TZWCCThermoFM::StoreFldIds( const   TZWaveUnit&         unitMe
                            , const MZWLeviSrvFuncs&    mzwsfLevi
                            ,       TString&            strTmp1
                            ,       TString&            strTmp2)
{
    strTmp1 = L"THERM#";
    AppendInstName(unitMe, kCIDLib::pszEmptyZStr, strTmp1);
    strTmp1.Append(L"~FanMode");
    m_c4FldId_FanMode = mzwsfLevi.c4LeviFldIdFromName(strTmp1);
}


// ---------------------------------------------------------------------------
//  TZWCCThermoFM: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  This is called when we get a fan mode report or reponse (as appposed to the fan
//  operating mode, which is done below.)
//
tCIDLib::TVoid
TZWCCThermoFM::StoreFanMode(const   tCIDLib::TCard4     c4Val
                            , const TZWaveUnit&         unitMe
                            ,       MZWLeviSrvFuncs&    mzwsfLevi)
{
    if (mzwsfLevi.eLeviVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
    {
        facZWaveLevi2Sh().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"Got fan mode '%(1)' for thermo %(2)"
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
            , TCardinal(c4Val)
            , unitMe.strName()
        );
    }

    if (!bCheckFldId(m_c4FldId_FanMode, L"fan mode", unitMe))
        return;

    // Get the limits object for the fan mode field
    const TCQCFldEnumLimit* pfldlEnum = static_cast<const TCQCFldEnumLimit*>
    (
        mzwsfLevi.pfldlLeviLimsFor(m_c4FldId_FanMode, TCQCFldEnumLimit::clsThis())
    );

    // Should always be there, but just in case. And check the value range
    if (!pfldlEnum || (c4Val >= pfldlEnum->c4ValCount()))
    {
        // Unknown, put the field into error
        mzwsfLevi.LeviSetFldErr(m_c4FldId_FanMode);
        return;
    }

    // Get our enum value for the Z-Wave ordinal
    const TString& strVal = pfldlEnum->strValueAt(c4Val);

    // Store the value
    mzwsfLevi.bLeviStoreStrFld(m_c4FldId_FanMode, strVal);

    // Make sure we are now marked as having valid info
    bWaitInitVals(kCIDLib::False);

}




// ---------------------------------------------------------------------------
//   CLASS: TZWCCThermoFOM
//  PREFIX: zwcc
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWCCThermoFOM: Constructors Destructor
// ---------------------------------------------------------------------------
TZWCCThermoFOM::TZWCCThermoFOM(const TZWDevClass& zwdcInst) :

    TZWCmdClass(tZWaveLevi2Sh::ECClasses::ThermoFanOpMode, zwdcInst, COMMAND_CLASS_THERMOSTAT_FAN_MODE)
    , m_c4FldId_FanOpMode(kCIDLib::c4MaxCard)
{
}

TZWCCThermoFOM::~TZWCCThermoFOM()
{
}


// ---------------------------------------------------------------------------
//  TZWCCThermoFOM: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  If the device's generic class isn't a binary switch, then we should get reports
//  this way, and may also get them via other command classes anyway. So we watch for
//  msgs that could deliver a level value.
//
//  1.  A binary switch report or set
//  2.  If a device is a scene actuator config, but also supports a local load, it
//      will often report it's current state via a msg with a scene id of 0, which
//      means there's no scene current set, so it's reporting it's current state.
//
tCIDLib::TBoolean
TZWCCThermoFOM::bHandleGenReport(const  tCIDLib::TCardList& fcolVals
                                , const TZWaveUnit&         unitMe
                                ,       MZWLeviSrvFuncs&    mzwsfLevi
                                , const tCIDLib::TCard4     c4FromInst)
{
    // Shouldn't get anything less than 2, but don't tempt fate
    const tCIDLib::TCard4 c4ValCnt = fcolVals.c4ElemCount();
    if (c4ValCnt < 2)
        return kCIDLib::False;

    if ((c4FromInst == c4InstId())
    &&  (fcolVals[0] == COMMAND_CLASS_THERMOSTAT_FAN_STATE)
    &&  (fcolVals[1] == THERMOSTAT_FAN_STATE_REPORT))
    {
        StoreFanOpMode(fcolVals[2], unitMe, mzwsfLevi);
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


tCIDLib::TBoolean TZWCCThermoFOM::bOwnsFld(const tCIDLib::TCard4 c4FldId) const
{
    return (m_c4FldId_FanOpMode == c4FldId);
}


//
//  This is called when the driver wants us to actively query our field values and
//  store them away.
//
tCIDLib::TBoolean
TZWCCThermoFOM::bQueryFldVals(TZWaveUnit& unitMe, MZWLeviSrvFuncs& mzwsfLevi)
{
    // Query the fan state
    mzwsfLevi.LeviSendUnitMsg
    (
        unitMe, *this, COMMAND_CLASS_THERMOSTAT_FAN_STATE, THERMOSTAT_FAN_STATE_GET
    );

    tCIDLib::TCardList fcolVals(8);
    const tCIDLib::TBoolean bRes = mzwsfLevi.bLeviWaitUnitMsg
    (
        unitMe
        , *this
        , COMMAND_CLASS_THERMOSTAT_FAN_STATE
        , THERMOSTAT_FAN_STATE_REPORT
        , fcolVals
    );

    if (bRes)
        bHandleGenReport(fcolVals, unitMe, mzwsfLevi, c4InstId());
    else
        mzwsfLevi.LeviSetFldErr(m_c4FldId_FanOpMode);

    return kCIDLib::True;
}


// Make sure our fields are set to error state until good values are stored
tCIDLib::TVoid
TZWCCThermoFOM::InitVals(const TZWaveUnit& unitMe,  MZWLeviSrvFuncs& mzwsfLevi)
{
    mzwsfLevi.LeviSetFldErr(m_c4FldId_FanOpMode);
}


//
//  Add our fields to the list. We either add just a generic multi-level value, or
//  a standard V2 dimmer field, depending on configuration.
//
tCIDLib::TVoid
TZWCCThermoFOM::QueryFldDefs(       tCQCKit::TFldDefList& colAddTo
                            ,       TCQCFldDef&           flddTmp
                            , const TZWaveUnit&           unitMe
                            ,       TString&              strTmp1
                            ,       TString&              strTmp2) const
{
    strTmp1 = L"THERM#";
    AppendInstName(unitMe, kCIDLib::pszEmptyZStr, strTmp1);
    strTmp1.Append(L"~FanOpMode");
    flddTmp.Set
    (
        strTmp1, tCQCKit::EFldTypes::String, tCQCKit::EFldAccess::Read, L"Enum: Off, Low, High, Unknown"
    );
    colAddTo.objAdd(flddTmp);
}


// We look our field and store it away
tCIDLib::TVoid
TZWCCThermoFOM::StoreFldIds(const  TZWaveUnit&         unitMe
                        , const MZWLeviSrvFuncs&    mzwsfLevi
                        ,       TString&            strTmp1
                        ,       TString&            strTmp2)
{
    strTmp1 = L"THERM#";
    AppendInstName(unitMe, kCIDLib::pszEmptyZStr, strTmp1);
    strTmp1.Append(L"~FanOpMode");
    m_c4FldId_FanOpMode = mzwsfLevi.c4LeviFldIdFromName(strTmp1);
}


// ---------------------------------------------------------------------------
//  TZWCCThermoFOM: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  This is called when we get a fan operating mode report or response. We translate
//  the value to the enumerated value and store the result in the fan op mode field.
//
tCIDLib::TVoid
TZWCCThermoFOM::StoreFanOpMode( const   tCIDLib::TCard4     c4Val
                                , const TZWaveUnit&         unitMe
                                ,       MZWLeviSrvFuncs&    mzwsfLevi)
{
    if (mzwsfLevi.eLeviVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
    {
        facZWaveLevi2Sh().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"Got fan op mode '%(1)' for thermo %(2)"
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
            , TCardinal(c4Val)
            , unitMe.strName()
        );
    }

    if (!bCheckFldId(m_c4FldId_FanOpMode, L"fan op mode", unitMe))
        return;

    // Get the limits object for the fan mode field
    const TCQCFldEnumLimit* pfldlEnum = static_cast<const TCQCFldEnumLimit*>
    (
        mzwsfLevi.pfldlLeviLimsFor(m_c4FldId_FanOpMode, TCQCFldEnumLimit::clsThis())
    );

    // Should always be there, but if not an error
    if (!pfldlEnum)
    {
        mzwsfLevi.LeviSetFldErr(m_c4FldId_FanOpMode);
        return;
    }

    //
    //  If it's > to the count minus one, then take the count minus one. This allows
    //  for future values we don't understand.
    //
    tCIDLib::TCard4 c4Actual = c4Val;
    if (c4Actual >= pfldlEnum->c4ValCount() - 1)
        c4Actual = pfldlEnum->c4ValCount() - 1;

    // Get our enum value for the Z-Wave ordinal
    const TString& strVal = pfldlEnum->strValueAt(c4Actual);

    // Store the value
    mzwsfLevi.bLeviStoreStrFld(m_c4FldId_FanOpMode, strVal);

    // Make sure we are now marked as having valid info
    bWaitInitVals(kCIDLib::False);
}




// ---------------------------------------------------------------------------
//   CLASS: TZWCCThermoSP
//  PREFIX: zwcc
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWCCThermoSP: Constructors Destructor
// ---------------------------------------------------------------------------
TZWCCThermoSP::TZWCCThermoSP(const TZWDevClass& zwdcInst) :

    TZWCmdClass(tZWaveLevi2Sh::ECClasses::ThermoSetPnt, zwdcInst, COMMAND_CLASS_THERMOSTAT_SETPOINT)
    , m_c4FldId_HighSetPnt(kCIDLib::c4MaxCard)
    , m_c4FldId_LowSetPnt(kCIDLib::c4MaxCard)
{
    // Pre-build our option keys
    BuildOptKey(L"ThermoSP_Scale", m_strOptKey_Scale);
}

TZWCCThermoSP::~TZWCCThermoSP()
{
}


// ---------------------------------------------------------------------------
//  TZWCCThermoSP: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  If the device's generic class isn't a binary switch, then we should get reports
//  this way, and may also get them via other command classes anyway. So we watch for
//  msgs that could deliver a level value.
//
//  1.  A binary switch report or set
//  2.  If a device is a scene actuator config, but also supports a local load, it
//      will often report it's current state via a msg with a scene id of 0, which
//      means there's no scene current set, so it's reporting it's current state.
//
tCIDLib::TBoolean
TZWCCThermoSP::bHandleGenReport(const   tCIDLib::TCardList& fcolVals
                                , const TZWaveUnit&         unitMe
                                ,       MZWLeviSrvFuncs&    mzwsfLevi
                                , const tCIDLib::TCard4     c4FromInst)
{
    // Shouldn't get anything less than 2, but don't tempt fate
    const tCIDLib::TCard4 c4ValCnt = fcolVals.c4ElemCount();
    if (c4ValCnt < 2)
        return kCIDLib::False;

    if ((c4FromInst == c4InstId())
    &&  (fcolVals[0] == COMMAND_CLASS_THERMOSTAT_SETPOINT)
    &&  (fcolVals[1] == THERMOSTAT_SETPOINT_REPORT))
    {
        StoreSP(fcolVals, unitMe, mzwsfLevi);
        return kCIDLib::True;
    }

    return kCIDLib::False;
}


tCIDLib::TBoolean TZWCCThermoSP::bOwnsFld(const tCIDLib::TCard4 c4FldId) const
{
    return  (m_c4FldId_HighSetPnt == c4FldId) || (m_c4FldId_LowSetPnt == c4FldId);
}


//
//  This is called when the driver wants us to actively query our field values and
//  store them away.
//
tCIDLib::TBoolean
TZWCCThermoSP::bQueryFldVals(TZWaveUnit& unitMe, MZWLeviSrvFuncs& mzwsfLevi)
{
    tCIDLib::TCardList fcolVals(12);

    // Do high set point (cooling)
    fcolVals.c4AddElement(COMMAND_CLASS_THERMOSTAT_SETPOINT);
    fcolVals.c4AddElement(THERMOSTAT_SETPOINT_GET);
    fcolVals.c4AddElement(2);
    mzwsfLevi.LeviSendUnitMsg(unitMe, *this, fcolVals);
    tCIDLib::TBoolean bRes = mzwsfLevi.bLeviWaitUnitMsg
    (
        unitMe
        , *this
        , COMMAND_CLASS_THERMOSTAT_SETPOINT
        , THERMOSTAT_SETPOINT_REPORT
        , fcolVals
    );
    if (bRes)
        bHandleGenReport(fcolVals, unitMe, mzwsfLevi, c4InstId());
    else
        mzwsfLevi.LeviSetFldErr(m_c4FldId_HighSetPnt);


    // Do low set point (heating)
    fcolVals.RemoveAll();
    fcolVals.c4AddElement(COMMAND_CLASS_THERMOSTAT_SETPOINT);
    fcolVals.c4AddElement(THERMOSTAT_SETPOINT_GET);
    fcolVals.c4AddElement(1);
    mzwsfLevi.LeviSendUnitMsg(unitMe, *this, fcolVals);
    bRes = mzwsfLevi.bLeviWaitUnitMsg
    (
        unitMe
        , *this
        , COMMAND_CLASS_THERMOSTAT_SETPOINT
        , THERMOSTAT_SETPOINT_REPORT
        , fcolVals
    );

    if (bRes)
        bHandleGenReport(fcolVals, unitMe, mzwsfLevi, c4InstId());
    else
        mzwsfLevi.LeviSetFldErr(m_c4FldId_HighSetPnt);

    return kCIDLib::True;
}


//
//  Handle writes to our set point fields.
//
tCQCKit::ECommResults
TZWCCThermoSP::eWriteIntFld(const   tCIDLib::TCard4     c4FldId
                            , const tCIDLib::TInt4      i4Value
                            ,       TZWaveUnit&         unitMe
                            ,       MZWLeviSrvFuncs&    mzwsfLevi)
{
    if ((c4FldId != m_c4FldId_LowSetPnt) && (c4FldId != m_c4FldId_HighSetPnt))
        return tCQCKit::ECommResults::FieldNotFound;

    const TString& strScale = unitMe.strFindOptVal(m_strOptKey_Scale);

    // And we can build up the message
    tCIDLib::TCardList fcolVals(12);
    fcolVals.c4AddElement(COMMAND_CLASS_THERMOSTAT_SETPOINT);
    fcolVals.c4AddElement(THERMOSTAT_SETPOINT_SET);

    // Setpoint type.
    fcolVals.c4AddElement((c4FldId == m_c4FldId_LowSetPnt) ? 1 : 2);

    // Precision 0, temp scale 1/F or 0/C, and size 1 byte (1001 or 0001)
    if (strScale == L"C")
        fcolVals.c4AddElement(8);
    else
        fcolVals.c4AddElement(9);

    // And the value, 1 byte we said it would be
    fcolVals.c4AddElement(i4Value);

    mzwsfLevi.LeviSendUnitMsg(unitMe, *this, fcolVals);

    //
    //  If this device has configured us a known response to wait for, we can
    //  use that as a validation of the operation, and it should give us the
    //  dimmer level as well.
    //
    //  Else, just assume it worked and let the written value be stored to
    //  the field upon return.
    //
    if (c4SetRespCls() && c4SetRespCmd())
    {
        const tCIDLib::TBoolean bRes = mzwsfLevi.bLeviWaitUnitMsg
        (
            unitMe, *this, c4SetRespCls(), c4SetRespCmd(), fcolVals
        );

        if (bRes)
        {
            bHandleGenReport(fcolVals, unitMe, mzwsfLevi, c4InstId());
        }
         else
        {
            facZWaveLevi2Sh().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kZW2ShErrs::errcUnit_NoSetResp
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::NoReply
                , unitMe.strName()
                , tZWaveLevi2Sh::strXlatECClasses(eCmdClass())
            );
        }
    }
    return tCQCKit::ECommResults::Success;
}


//
//  Add any of our prompts to the passed list.
//
tCIDLib::TVoid TZWCCThermoSP::LoadOptList(tZWaveLevi2Sh::TOptList& colToFill) const
{
    TZWOption& zwoptScale = colToFill.objAdd(TZWOption());
    zwoptScale.m_strKey = m_strOptKey_Scale;
    zwoptScale.m_strPrompt = L"Set Point Scale";
    zwoptScale.m_strInstDescr = strInstDescr();
    zwoptScale.m_colValues.objAdd(TKeyValuePair(L"F", L"Fahrenheit"));
    zwoptScale.m_colValues.objAdd(TKeyValuePair(L"C", L"Celsius"));
}


tCIDLib::TVoid
TZWCCThermoSP::LoadOptVals( const   TZWDevClass&                zwdcInst
                            ,       tZWaveLevi2Sh::TOptValList& colToFill
                            ,       TZWOptionVal&               zwoptvTmp) const
{
    if (!colToFill.bKeyExists(m_strOptKey_Scale))
    {
        zwdcInst.QueryOptDef(zwoptvTmp, m_strOptKey_Scale, L"F");
        colToFill.objAdd(zwoptvTmp);
    }
}


// Make sure our fields are set to error state until good values are stored
tCIDLib::TVoid
TZWCCThermoSP::InitVals(const TZWaveUnit& unitMe,  MZWLeviSrvFuncs& mzwsfLevi)
{
    mzwsfLevi.LeviSetFldErr(m_c4FldId_HighSetPnt);
    mzwsfLevi.LeviSetFldErr(m_c4FldId_LowSetPnt);
}


//
//  Add our fields to the list. We either add just a generic multi-level value, or
//  a standard V2 dimmer field, depending on configuration.
//
tCIDLib::TVoid
TZWCCThermoSP::QueryFldDefs(        tCQCKit::TFldDefList& colAddTo
                            ,       TCQCFldDef&           flddTmp
                            , const TZWaveUnit&           unitMe
                            ,       TString&              strTmp1
                            ,       TString&              strTmp2) const
{
    const TString& strScale = unitMe.strFindOptVal(m_strOptKey_Scale);
    const tCIDLib::TCh* pszSPLim = 0;

    strTmp1 = L"THERM#";
    AppendInstName(unitMe, kCIDLib::pszEmptyZStr, strTmp1);
    const tCIDLib::TCard4 c4BaseLen = strTmp1.c4Length();

    strTmp1.CapAt(c4BaseLen);
    strTmp1.Append(L"~HighSetPnt");
    if (strScale == L"C")
        pszSPLim = L"Range: 5, 45";
    else
        pszSPLim = L"Range: 44, 113";
    flddTmp.Set
    (
        strTmp1
        , tCQCKit::EFldTypes::Int
        , tCQCKit::EFldAccess::ReadWrite
        , tCQCKit::EFldSTypes::HighSetPnt
        , pszSPLim
    );
    colAddTo.objAdd(flddTmp);

    strTmp1.CapAt(c4BaseLen);
    strTmp1.Append(L"~LowSetPnt");

    if (strScale == L"C")
        pszSPLim = L"Range: 4, 43";
    else
        pszSPLim = L"Range: 40, 109";
    flddTmp.Set
    (
        strTmp1
        , tCQCKit::EFldTypes::Int
        , tCQCKit::EFldAccess::ReadWrite
        , tCQCKit::EFldSTypes::LowSetPnt
        , pszSPLim
    );
    colAddTo.objAdd(flddTmp);
}


// We look our field and store it away
tCIDLib::TVoid
TZWCCThermoSP::StoreFldIds( const   TZWaveUnit&         unitMe
                            , const MZWLeviSrvFuncs&    mzwsfLevi
                            ,       TString&            strTmp1
                            ,       TString&            strTmp2)
{
    strTmp1 = L"THERM#";
    AppendInstName(unitMe, kCIDLib::pszEmptyZStr, strTmp1);
    const tCIDLib::TCard4 c4BaseLen = strTmp1.c4Length();

    strTmp1.CapAt(c4BaseLen);
    strTmp1.Append(L"~HighSetPnt");
    m_c4FldId_HighSetPnt = mzwsfLevi.c4LeviFldIdFromName(strTmp1);

    strTmp1.CapAt(c4BaseLen);
    strTmp1.Append(L"~LowSetPnt");
    m_c4FldId_LowSetPnt = mzwsfLevi.c4LeviFldIdFromName(strTmp1);
}


// ---------------------------------------------------------------------------
//  TZWCCThermoSP: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid
TZWCCThermoSP::StoreSP( const   tCIDLib::TCardList& fcolVals
                        , const TZWaveUnit&         unitMe
                        ,       MZWLeviSrvFuncs&    mzwsfLevi)
{
    // Make sure it's either low or high set point
    tCIDLib::TCard4 c4FldId;
    const tCIDLib::TCh* pszFld = L"";
    if (fcolVals[2] == 1)
    {
        c4FldId = m_c4FldId_LowSetPnt;
        pszFld = L"low set point";
    }
     else if (fcolVals[2] == 2)
    {
        c4FldId = m_c4FldId_HighSetPnt;
        pszFld = L"high set point";
    }
     else
    {
        return;
    }

    // Log what we got if verbose
    if (mzwsfLevi.eLeviVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
    {
        facZWaveLevi2Sh().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"Got %(1) setpoint for %(2)"
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
            , TString(pszFld)
            , unitMe.strName()
        );
    }

    if (!bCheckFldId(c4FldId, pszFld, unitMe))
        return;

    // Use a helper to get the value. We use the integer result
    tCIDLib::TInt4 i4Temp;
    tCIDLib::TFloat8 f8Temp;
    const tCIDLib::TBoolean bRes = bDecodeValue
    (
        fcolVals.ptElements() + 3
        , fcolVals.c4ElemCount() - 3
        , kCIDLib::True
        , i4Temp
        , f8Temp
        , pszFld
        , mzwsfLevi
    );

    // If not valid, upt it into error state, else store it
    if (bRes)
    {
        mzwsfLevi.bLeviStoreIntFld(c4FldId, i4Temp);

        // Make sure we are now marked as having valid info
        bWaitInitVals(kCIDLib::False);
    }
     else
    {
        mzwsfLevi.LeviSetFldErr(c4FldId);
    }
}





// ---------------------------------------------------------------------------
//   CLASS: TZWCCThermoMode
//  PREFIX: zwcc
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWCCThermoMode: Constructors Destructor
// ---------------------------------------------------------------------------
TZWCCThermoMode::TZWCCThermoMode(const TZWDevClass& zwdcInst) :

    TZWCmdClass(tZWaveLevi2Sh::ECClasses::ThermoMode, zwdcInst, COMMAND_CLASS_THERMOSTAT_MODE)
    , m_c4FldId_Mode(kCIDLib::c4MaxCard)
{
}

TZWCCThermoMode::~TZWCCThermoMode()
{
}


// ---------------------------------------------------------------------------
//  TZWCCThermoMode: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  If the device's generic class isn't a binary switch, then we should get reports
//  this way, and may also get them via other command classes anyway. So we watch for
//  msgs that could deliver a level value.
//
//  1.  A binary switch report or set
//  2.  If a device is a scene actuator config, but also supports a local load, it
//      will often report it's current state via a msg with a scene id of 0, which
//      means there's no scene current set, so it's reporting it's current state.
//
tCIDLib::TBoolean
TZWCCThermoMode::bHandleGenReport(  const   tCIDLib::TCardList& fcolVals
                                    , const TZWaveUnit&         unitMe
                                    ,       MZWLeviSrvFuncs&    mzwsfLevi
                                    , const tCIDLib::TCard4     c4FromInst)
{
    // Shouldn't get anything less than 2, but don't tempt fate
    const tCIDLib::TCard4 c4ValCnt = fcolVals.c4ElemCount();
    if (c4ValCnt < 2)
        return kCIDLib::False;

    if ((c4FromInst == c4InstId())
    &&  (fcolVals[0] == COMMAND_CLASS_THERMOSTAT_MODE)
    &&  (fcolVals[1] == THERMOSTAT_MODE_REPORT))
    {
        StoreMode(fcolVals[2], unitMe, mzwsfLevi);
    }
    return kCIDLib::False;
}


tCIDLib::TBoolean TZWCCThermoMode::bOwnsFld(const tCIDLib::TCard4 c4FldId) const
{
    return  (m_c4FldId_Mode == c4FldId);
}


//
//  This is called when the driver wants us to actively query our field values and
//  store them away.
//
tCIDLib::TBoolean
TZWCCThermoMode::bQueryFldVals(TZWaveUnit& unitMe, MZWLeviSrvFuncs& mzwsfLevi)
{
    tCIDLib::TCardList fcolVals(8);

    // Query the current mode
    mzwsfLevi.LeviSendUnitMsg(unitMe, *this, COMMAND_CLASS_THERMOSTAT_MODE, THERMOSTAT_MODE_GET);

    const tCIDLib::TBoolean bRes = mzwsfLevi.bLeviWaitUnitMsg
    (
        unitMe
        , *this
        , COMMAND_CLASS_THERMOSTAT_MODE
        , THERMOSTAT_MODE_REPORT
        , fcolVals
    );

    if (bRes)
        bHandleGenReport(fcolVals, unitMe, mzwsfLevi, c4InstId());
    else
        mzwsfLevi.LeviSetFldErr(m_c4FldId_Mode);

    return kCIDLib::True;
}


tCQCKit::ECommResults
TZWCCThermoMode::eWriteStringFld(const  tCIDLib::TCard4     c4FldId
                                , const TString&            strValue
                                ,       TZWaveUnit&         unitMe
                                ,       MZWLeviSrvFuncs&    mzwsfLevi)
{
    if (c4FldId != m_c4FldId_Mode)
        return tCQCKit::ECommResults::FieldNotFound;

    // Get the limits object and map the value
    const TCQCFldEnumLimit* pfldlEnum = static_cast<const TCQCFldEnumLimit*>
    (
        mzwsfLevi.pfldlLeviLimsFor(m_c4FldId_Mode, TCQCFldEnumLimit::clsThis())
    );

    if (!pfldlEnum)
        return tCQCKit::ECommResults::Internal;

    // Get the value. If not found, we fail
    tCIDLib::TCard4 c4Val = pfldlEnum->c4QueryOrdinal(strValue, kCIDLib::False);
    if (c4Val == kCIDLib::c4MaxCard)
        return tCQCKit::ECommResults::BadValue;

    // And we can build up the message
    tCIDLib::TCardList fcolVals(8);
    fcolVals.c4AddElement(COMMAND_CLASS_THERMOSTAT_MODE);
    fcolVals.c4AddElement(THERMOSTAT_MODE_SET);
    fcolVals.c4AddElement(c4Val);

    mzwsfLevi.LeviSendUnitMsg(unitMe, *this, fcolVals);

    //
    //  If this device has configured us a known response to wait for, we can
    //  use that as a validation of the operation, and it should give us the
    //  dimmer level as well.
    //
    //  Else, just assume it worked and let the written value be stored to the
    //  field upon return.
    //
    if (c4SetRespCls() && c4SetRespCmd())
    {
        const tCIDLib::TBoolean bRes = mzwsfLevi.bLeviWaitUnitMsg
        (
            unitMe, *this, c4SetRespCls(), c4SetRespCmd(), fcolVals
        );

        if (bRes)
        {
            bHandleGenReport(fcolVals, unitMe, mzwsfLevi, c4InstId());
        }
         else
        {
            facZWaveLevi2Sh().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kZW2ShErrs::errcUnit_NoSetResp
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::NoReply
                , unitMe.strName()
                , tZWaveLevi2Sh::strXlatECClasses(eCmdClass())
            );
        }
    }
    return tCQCKit::ECommResults::Success;
}


// Make sure our fields are set to error state until good values are stored
tCIDLib::TVoid
TZWCCThermoMode::InitVals(const TZWaveUnit& unitMe,  MZWLeviSrvFuncs& mzwsfLevi)
{
    mzwsfLevi.LeviSetFldErr(m_c4FldId_Mode);
}


//
//  Add our fields to the list. We either add just a generic multi-level value, or
//  a standard V2 dimmer field, depending on configuration.
//
tCIDLib::TVoid
TZWCCThermoMode::QueryFldDefs(          tCQCKit::TFldDefList& colAddTo
                                ,       TCQCFldDef&           flddTmp
                                , const TZWaveUnit&           unitMe
                                ,       TString&              strTmp1
                                ,       TString&              strTmp2) const
{
    strTmp1 = L"THERM#";
    AppendInstName(unitMe, kCIDLib::pszEmptyZStr, strTmp1);
    strTmp1.Append(L"~CurMode");

    flddTmp.Set
    (
        strTmp1
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::ReadWrite
        , L"Enum: Off, Heat, Cool, Auto, Aux, Resume, Fan, Furnace, Dry, Moist, Auto"
    );
    colAddTo.objAdd(flddTmp);
}


// We look our field and store it away
tCIDLib::TVoid
TZWCCThermoMode::StoreFldIds(const  TZWaveUnit&         unitMe
                            , const MZWLeviSrvFuncs&    mzwsfLevi
                            ,       TString&            strTmp1
                            ,       TString&            strTmp2)
{
    strTmp1 = L"THERM#";
    AppendInstName(unitMe, kCIDLib::pszEmptyZStr, strTmp1);
    strTmp1.Append(L"~CurMode");
    m_c4FldId_Mode = mzwsfLevi.c4LeviFldIdFromName(strTmp1);
}


// ---------------------------------------------------------------------------
//  TZWCCThermoMode: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  This is called when we get a mode report or response. We translate the value to the
//  enumerated value and store the result in the mode field.
//
tCIDLib::TVoid
TZWCCThermoMode::StoreMode( const   tCIDLib::TCard4     c4Val
                        , const TZWaveUnit&         unitMe
                        ,       MZWLeviSrvFuncs&    mzwsfLevi)
{
    if (mzwsfLevi.eLeviVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
    {
        facZWaveLevi2Sh().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"Got mode '%(1)' for thermo %(2)"
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
            , TCardinal(c4Val)
            , unitMe.strName()
        );
    }

    if (!bCheckFldId(m_c4FldId_Mode, L"thermo mode", unitMe))
        return;

    // Get the limits object for the state field
    const TCQCFldEnumLimit* pfldlEnum = static_cast<const TCQCFldEnumLimit*>
    (
        mzwsfLevi.pfldlLeviLimsFor(m_c4FldId_Mode, TCQCFldEnumLimit::clsThis())
    );

    // Should always be there, but just in case. And check the value range
    if (!pfldlEnum || (c4Val >= pfldlEnum->c4ValCount()))
    {
        facZWaveLevi2Sh().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"%(1) is not a valid mode value. Thermo=%(2)"
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
            , TCardinal(c4Val)
            , unitMe.strName()
        );

        // Unknown, put the field into error
        mzwsfLevi.LeviSetFldErr(m_c4FldId_Mode);
        return;
    }

    // Get our enum value for the Z-Wave ordinal
    const TString& strVal = pfldlEnum->strValueAt(c4Val);

    // Lock and store the value
    mzwsfLevi.bLeviStoreStrFld(m_c4FldId_Mode, strVal);

    // Make sure we are now marked as having valid info
    bWaitInitVals(kCIDLib::False);
}




// ---------------------------------------------------------------------------
//   CLASS: TZWCCThermoOMode
//  PREFIX: zwcc
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWCCThermoOMode: Constructors Destructor
// ---------------------------------------------------------------------------
TZWCCThermoOMode::TZWCCThermoOMode(const TZWDevClass& zwdcInst) :

    TZWCmdClass(tZWaveLevi2Sh::ECClasses::ThermoOpMode, zwdcInst, COMMAND_CLASS_THERMOSTAT_OPERATING_STATE)
    , m_c4FldId_OpMode(kCIDLib::c4MaxCard)
{
}

TZWCCThermoOMode::~TZWCCThermoOMode()
{
}


// ---------------------------------------------------------------------------
//  TZWCCThermoOMode: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  If the device's generic class isn't a binary switch, then we should get reports
//  this way, and may also get them via other command classes anyway. So we watch for
//  msgs that could deliver a level value.
//
//  1.  A binary switch report or set
//  2.  If a device is a scene actuator config, but also supports a local load, it
//      will often report it's current state via a msg with a scene id of 0, which
//      means there's no scene current set, so it's reporting it's current state.
//
tCIDLib::TBoolean
TZWCCThermoOMode::bHandleGenReport( const   tCIDLib::TCardList& fcolVals
                                    , const TZWaveUnit&         unitMe
                                    ,       MZWLeviSrvFuncs&    mzwsfLevi
                                    , const tCIDLib::TCard4     c4FromInst)
{
    // Shouldn't get anything less than 2, but don't tempt fate
    const tCIDLib::TCard4 c4ValCnt = fcolVals.c4ElemCount();
    if (c4ValCnt < 2)
        return kCIDLib::False;

    if ((c4FromInst == c4InstId())
    &&  (fcolVals[0] == COMMAND_CLASS_THERMOSTAT_OPERATING_STATE)
    &&  (fcolVals[1] == THERMOSTAT_OPERATING_STATE_REPORT))
    {
        StoreOpMode(fcolVals[2], unitMe, mzwsfLevi);
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


tCIDLib::TBoolean TZWCCThermoOMode::bOwnsFld(const tCIDLib::TCard4 c4FldId) const
{
    return (m_c4FldId_OpMode == c4FldId);
}


//
//  This is called when the driver wants us to actively query our field values and
//  store them away.
//
tCIDLib::TBoolean
TZWCCThermoOMode::bQueryFldVals(TZWaveUnit& unitMe, MZWLeviSrvFuncs& mzwsfLevi)
{
    // Query the current state
    mzwsfLevi.LeviSendUnitMsg
    (
        unitMe
        , *this
        , COMMAND_CLASS_THERMOSTAT_OPERATING_STATE
        , THERMOSTAT_OPERATING_STATE_GET
    );

    tCIDLib::TCardList fcolVals(8);
    const tCIDLib::TBoolean bRes = mzwsfLevi.bLeviWaitUnitMsg
    (
        unitMe
        , *this
        , COMMAND_CLASS_THERMOSTAT_OPERATING_STATE
        , THERMOSTAT_OPERATING_STATE_REPORT
        , fcolVals
    );

    if (bRes)
        bHandleGenReport(fcolVals, unitMe, mzwsfLevi, c4InstId());
    else
        mzwsfLevi.LeviSetFldErr(m_c4FldId_OpMode);

    return kCIDLib::True;
}


// Make sure our fields are set to error state until good values are stored
tCIDLib::TVoid
TZWCCThermoOMode::InitVals(const TZWaveUnit& unitMe,  MZWLeviSrvFuncs& mzwsfLevi)
{
    mzwsfLevi.LeviSetFldErr(m_c4FldId_OpMode);
}


//
//  Add our fields to the list. We either add just a generic multi-level value, or
//  a standard V2 dimmer field, depending on configuration.
//
tCIDLib::TVoid
TZWCCThermoOMode::QueryFldDefs(         tCQCKit::TFldDefList& colAddTo
                                ,       TCQCFldDef&           flddTmp
                                , const TZWaveUnit&           unitMe
                                ,       TString&              strTmp1
                                ,       TString&              strTmp2) const
{
    strTmp1 = L"THERM#";
    AppendInstName(unitMe, kCIDLib::pszEmptyZStr, strTmp1);
    strTmp1.Append(L"~OpMode");

    flddTmp.Set
    (
        strTmp1
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Read
        , L"Enum: Idle, Heating, Cooling, Fan, PendingHeat, PendingCool, Vent, Unknown"
    );
    colAddTo.objAdd(flddTmp);
}


// We look our field and store it away
tCIDLib::TVoid
TZWCCThermoOMode::StoreFldIds(const  TZWaveUnit&         unitMe
                        , const MZWLeviSrvFuncs&    mzwsfLevi
                        ,       TString&            strTmp1
                        ,       TString&            strTmp2)
{
    strTmp1 = L"THERM#";
    AppendInstName(unitMe, kCIDLib::pszEmptyZStr, strTmp1);
    strTmp1.Append(L"~OpMode");
    m_c4FldId_OpMode = mzwsfLevi.c4LeviFldIdFromName(strTmp1);
}


// ---------------------------------------------------------------------------
//  TZWCCThermoOMode: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  This is called when we get an operating mode report or response, as apposed to
//  the set mode which is handled above.
//
tCIDLib::TVoid
TZWCCThermoOMode::StoreOpMode(  const   tCIDLib::TCard4     c4Val
                                , const TZWaveUnit&         unitMe
                                ,       MZWLeviSrvFuncs&    mzwsfLevi)
{
    if (mzwsfLevi.eLeviVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
    {
        facZWaveLevi2Sh().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"Got operating mode '%(1)' for thermo %(2)"
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
            , TCardinal(c4Val)
            , unitMe.strName()
        );
    }

    if (!bCheckFldId(m_c4FldId_OpMode, L"op mode", unitMe))
        return;

    // Get the limits object for the state field
    const TCQCFldEnumLimit* pfldlEnum = static_cast<const TCQCFldEnumLimit*>
    (
        mzwsfLevi.pfldlLeviLimsFor(m_c4FldId_OpMode, TCQCFldEnumLimit::clsThis())
    );

    // Should always be there, but just in case. And check the value range
    if (!pfldlEnum || (c4Val >= pfldlEnum->c4ValCount()))
    {
        facZWaveLevi2Sh().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"%(1) is not a valid op mode value. Thermo=%(2)"
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
            , TCardinal(c4Val)
            , unitMe.strName()
        );

        // Unknown, put the field into error
        mzwsfLevi.LeviSetFldErr(m_c4FldId_OpMode);
        return;
    }

    // Get our enum value for the Z-Wave ordinal
    const TString& strVal = pfldlEnum->strValueAt(c4Val);

    // Store the value
    mzwsfLevi.bLeviStoreStrFld(m_c4FldId_OpMode, strVal);

    // Make sure we are now marked as having valid info
    bWaitInitVals(kCIDLib::False);
}


