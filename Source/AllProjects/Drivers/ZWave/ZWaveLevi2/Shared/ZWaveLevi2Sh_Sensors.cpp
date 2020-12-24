//
// FILE NAME: ZWaveLevi2Sh_SwitchDim.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/28/2014
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
//  This file provides the implementation for the binary/multi-level switch command
//  class derivatives.
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
RTTIDecls(TZWCCBinSensor, TZWCmdClass)
RTTIDecls(TZWCCMLSensor, TZWCmdClass)




// ---------------------------------------------------------------------------
//   CLASS: TZWCCBinSensor
//  PREFIX: zwcc
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWCCBinSensor: Constructors Destructor
// ---------------------------------------------------------------------------
TZWCCBinSensor::TZWCCBinSensor(const TZWDevClass& zwdcInst) :

    TZWCmdClass(tZWaveLevi2Sh::ECClasses::BinSensor, zwdcInst, COMMAND_CLASS_SENSOR_BINARY)
    , m_c4FldId_State(kCIDLib::c4MaxCard)
{
    // Pre-build our option keys
    BuildOptKey(L"BinSens_Trigs", m_strOptKey_Trigs);
    BuildOptKey(L"BinSens_Type", m_strOptKey_Type);

    //
    //  We are never writeable, so for efficiency make sure that's marked as such.
    //  For readable, we take whatever the base class set (from the device info
    //  we passed long to it.)
    //
    SetAccess(bCanRead(), kCIDLib::False);
}

TZWCCBinSensor::~TZWCCBinSensor()
{
}


// ---------------------------------------------------------------------------
//  TZWCCBinSensor: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  If the device's generic class isn't a binary sensor, then we should get our value
//  via the generic report.
//
tCIDLib::TBoolean
TZWCCBinSensor::bHandleGenReport(const  tCIDLib::TCardList& fcolVals
                                , const TZWaveUnit&         unitMe
                                ,       MZWLeviSrvFuncs&    mzwsfLevi
                                , const tCIDLib::TCard4     c4FromInst)
{
    if (fcolVals.c4ElemCount() < 3)
        return kCIDLib::False;

    if ((c4FromInst == c4InstId()))
    {
        if ((fcolVals[0] == COMMAND_CLASS_SENSOR_BINARY)
        &&  (fcolVals[1] == SENSOR_BINARY_REPORT))
        {
            StoreValue(fcolVals[2], unitMe, mzwsfLevi);
        }
         else if ((fcolVals[0] == COMMAND_CLASS_BASIC)
              &&  ((fcolVals[1] == BASIC_REPORT) || (fcolVals[1] == BASIC_SET)))
        {
            StoreValue(fcolVals[2], unitMe, mzwsfLevi);
        }
    }
    return kCIDLib::True;
}


tCIDLib::TBoolean TZWCCBinSensor::bOwnsFld(const tCIDLib::TCard4 c4FldId) const
{
    return (c4FldId == m_c4FldId_State);
}


//
//  This is called when the driver wants us to actively query our field values and
//  store them away.
//
tCIDLib::TBoolean
TZWCCBinSensor::bQueryFldVals(TZWaveUnit& unitMe, MZWLeviSrvFuncs& mzwsfLevi)
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
        c4CmdClass = COMMAND_CLASS_SENSOR_BINARY;
        c4Get      = SENSOR_BINARY_GET;
        c4Report   = SENSOR_BINARY_REPORT;
    }

    tCIDLib::TCardList fcolVals(16);
    mzwsfLevi.LeviSendUnitMsg(unitMe, *this, c4CmdClass, c4Get);
    if (mzwsfLevi.bLeviWaitUnitMsg(unitMe, *this, c4CmdClass, c4Report, fcolVals))
        bHandleGenReport(fcolVals, unitMe, mzwsfLevi, c4InstId());
    else
        mzwsfLevi.LeviSetFldErr(m_c4FldId_State);

    return kCIDLib::True;
}


//
//  Add any of our options to the passed list.
//
tCIDLib::TVoid TZWCCBinSensor::LoadOptList(tZWaveLevi2Sh::TOptList& colToFill) const
{
    TZWOption& zwoptType = colToFill.objAdd(TZWOption());
    zwoptType.m_strKey = m_strOptKey_Type;
    zwoptType.m_strPrompt = L"Binary Sensor Type";
    zwoptType.m_strInstDescr = strInstDescr();
    zwoptType.m_colValues.objAdd(TKeyValuePair(L"Gen", L"Generic Sensor"));
    zwoptType.m_colValues.objAdd(TKeyValuePair(L"Mot", L"Motion Sensor"));

    TZWOption& zwoptTrigs = colToFill.objAdd(TZWOption());
    zwoptTrigs.m_strKey = m_strOptKey_Trigs;
    zwoptTrigs.m_strPrompt = L"Send Triggers";
    zwoptTrigs.m_strInstDescr = strInstDescr();
    zwoptTrigs.m_colValues.objAdd(TKeyValuePair(L"0", L"No"));
    zwoptTrigs.m_colValues.objAdd(TKeyValuePair(L"1", L"Yes"));
}


//
//  Add default option values to the passed list. If we have any defaults in the
//  dev info, we take that, else we provide defaults.
//
tCIDLib::TVoid
TZWCCBinSensor::LoadOptVals(const   TZWDevClass&                zwdcInst
                            ,       tZWaveLevi2Sh::TOptValList& colToFill
                            ,       TZWOptionVal&               zwoptvTmp) const
{
    if (!colToFill.bKeyExists(m_strOptKey_Type))
    {
        zwdcInst.QueryOptDef(zwoptvTmp, m_strOptKey_Type, L"Gen");
        colToFill.objAdd(zwoptvTmp);
    }

    if (!colToFill.bKeyExists(m_strOptKey_Trigs))
    {
        zwdcInst.QueryOptDef(zwoptvTmp, m_strOptKey_Trigs, L"0");
        colToFill.objAdd(zwoptvTmp);
    }
}


// Make sure our fields are set to error state until good values are stored
tCIDLib::TVoid
TZWCCBinSensor::InitVals(const TZWaveUnit& unitMe,  MZWLeviSrvFuncs& mzwsfLevi)
{
    mzwsfLevi.LeviSetFldErr(m_c4FldId_State);
}



//
//  Add our fields to the list. We either add just a generic multi-level value, or
//  a standard V2 dimmer field, depending on configuration.
//
tCIDLib::TVoid
TZWCCBinSensor::QueryFldDefs(       tCQCKit::TFldDefList& colAddTo
                            ,       TCQCFldDef&           flddTmp
                            , const TZWaveUnit&           unitMe
                            ,       TString&              strTmp1
                            ,       TString&              strTmp2) const
{
    const TString& strType = unitMe.strFindOptVal(m_strOptKey_Type);

    if (strType == L"Mot")
    {
        strTmp1 = L"MOT#";
        AppendInstName(unitMe, kCIDLib::pszEmptyZStr, strTmp1);
        flddTmp.Set
        (
            strTmp1
            , tCQCKit::EFldTypes::Boolean
            , tCQCKit::EFldAccess::Read
            , tCQCKit::EFldSTypes::MotionSensor
        );
    }
     else
    {
        // Just create a generic binary sensor field
        BuildInstName(unitMe, kCIDLib::pszEmptyZStr, strTmp1);
        flddTmp.Set(strTmp1, tCQCKit::EFldTypes::Boolean, tCQCKit::EFldAccess::Read);
    }

    colAddTo.objAdd(flddTmp);
}


// We look up our field and store it away
tCIDLib::TVoid
TZWCCBinSensor::StoreFldIds(const   TZWaveUnit&         unitMe
                            , const MZWLeviSrvFuncs&    mzwsfLevi
                            ,       TString&            strTmp1
                            ,       TString&            strTmp2)
{
    const TString& strType = unitMe.strFindOptVal(m_strOptKey_Type);

    if (strType == L"Mot")
        strTmp1 = L"MOT#";
    AppendInstName(unitMe, kCIDLib::pszEmptyZStr, strTmp1);
    m_c4FldId_State = mzwsfLevi.c4LeviFldIdFromName(strTmp1);
}


// ---------------------------------------------------------------------------
//  TZWCCBinSensor: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  We can potentially store the value from multiple place,s so we break out
//  the grunt work to here.
//
tCIDLib::TVoid
TZWCCBinSensor::StoreValue( const   tCIDLib::TCard4     c4Val
                            , const TZWaveUnit&         unitMe
                            ,       MZWLeviSrvFuncs&    mzwsfLevi)
{
    if (!bCheckFldId(m_c4FldId_State, L"state", unitMe))
        return;

    //
    //  Just write the boolean value to our field. If the value changes, and
    //  this guy is a motion sensor, send a motion trigger.
    //
    tCIDLib::TBoolean bChanged = mzwsfLevi.bLeviStoreBoolFld(m_c4FldId_State, c4Val != 0);

    // Make sure we are now marked as having valid info
    bWaitInitVals(kCIDLib::False);

    //
    //  If the value changed, see if we are a motion sensor and configured for triggers.
    //  If so, send a motion trigger.
    //
    if (bChanged)
    {
        const TString& strType = unitMe.strFindOptVal(m_strOptKey_Type);
        const TString& strTrigs = unitMe.strFindOptVal(m_strOptKey_Trigs);
        if ((strType == L"Mot") && (strTrigs == L"1"))
        {
            TString strId;
            strId.SetFormatted(unitMe.c4Id());

            TString strInstName;
            BuildInstName(unitMe, kCIDLib::pszEmptyZStr, strInstName);

            TString strFldName;
            if (mzwsfLevi.bLeviQueryFldName(m_c4FldId_State, strFldName))
            {
                mzwsfLevi.LeviQEventTrig
                (
                    tCQCKit::EStdDrvEvs::Motion
                    , strFldName
                    , facCQCKit().strBoolStartEnd(c4Val != 0)
                    , strId
                    , strInstName
                    , TString::strEmpty()
                );
            }
        }
    }
}





// ---------------------------------------------------------------------------
//   CLASS: TZWCCMLSensor
//  PREFIX: zwcc
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWCCMLSensor: Cosntructors Destructor
// ---------------------------------------------------------------------------
TZWCCMLSensor::TZWCCMLSensor(const TZWDevClass& zwdcInst) :

    TZWCmdClass(tZWaveLevi2Sh::ECClasses::MLSensor, zwdcInst, COMMAND_CLASS_SENSOR_MULTILEVEL)
    , m_c4FldId_Level(kCIDLib::c4MaxCard)
{
    // Pre-build our option keys
    BuildOptKey(L"MLSens_Data", m_strOptKey_Data);
    BuildOptKey(L"MLSens_Scale", m_strOptKey_Scale);
    BuildOptKey(L"MLSens_Type", m_strOptKey_Type);
    BuildOptKey(L"MLSens_Units", m_strOptKey_Units);

    //
    //  We are never writeable, so for efficiency make sure that's marked as such.
    //  For readable, we take whatever the base class set (from the device info
    //  we passed long to it.)
    //
    SetAccess(bCanRead(), kCIDLib::False);
}

TZWCCMLSensor::~TZWCCMLSensor()
{
}


// ---------------------------------------------------------------------------
//  TZWCCMLSensor: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  Basic reports are meaningless for this type, so we should always get general
//  reports.
//
tCIDLib::TBoolean
TZWCCMLSensor::bHandleGenReport(const   tCIDLib::TCardList& fcolVals
                                , const TZWaveUnit&         unitMe
                                ,       MZWLeviSrvFuncs&    mzwsfLevi
                                , const tCIDLib::TCard4     c4FromInst)
{
    //
    //  We have to have at least 5 bytes, maybe more. We also make sure that is is
    //  for the same data type as we are configured for. We don't check the inst
    //  id, since we should only get called if it's appropriate.
    //
    if ((fcolVals.c4ElemCount() >= 5)
    &&  ((fcolVals[0] == COMMAND_CLASS_SENSOR_MULTILEVEL)
    &&   (fcolVals[1] == SENSOR_MULTILEVEL_REPORT)))
    {
        // Only store it if it's our sensor type
        const tCIDLib::TCard4 c4OurType = unitMe.strFindOptVal(m_strOptKey_Type).c4Val();
        if (fcolVals[2] == c4OurType)
        {
            StoreValue(fcolVals, unitMe, mzwsfLevi);
            return kCIDLib::True;
        }
    }
    return kCIDLib::False;
}


tCIDLib::TBoolean TZWCCMLSensor::bOwnsFld(const tCIDLib::TCard4 c4FldId) const
{
    return (c4FldId == m_c4FldId_Level);
}


//
//  This is called when the driver wants us to actively query our field values and
//  store them away.
//
tCIDLib::TBoolean
TZWCCMLSensor::bQueryFldVals(TZWaveUnit& unitMe, MZWLeviSrvFuncs& mzwsfLevi)
{
    const tCIDLib::TCard4 c4CmdClass = COMMAND_CLASS_SENSOR_MULTILEVEL;
    const tCIDLib::TCard4 c4Cmd      = SENSOR_MULTILEVEL_GET;
    const tCIDLib::TCard4 c4Rep      = SENSOR_MULTILEVEL_REPORT;

    // Send a multi-level sensor get
    mzwsfLevi.LeviSendUnitMsg(unitMe, *this, c4CmdClass, c4Cmd);

    // We should always get a general report for this class
    tCIDLib::TCardList fcolVals(8);
    if (mzwsfLevi.bLeviWaitUnitMsg(unitMe, *this, c4CmdClass, c4Rep, fcolVals))
        bHandleGenReport(fcolVals, unitMe, mzwsfLevi, c4InstId());
    else
        mzwsfLevi.LeviSetFldErr(m_c4FldId_Level);

    return kCIDLib::True;
}


//
//  Add any of our prompts to the passed list. We just have a single one, which is
//  whether we are a dimmer or a generic multi-level value.
//
tCIDLib::TVoid
TZWCCMLSensor::LoadOptList(tZWaveLevi2Sh::TOptList& colToFill) const
{
    TZWOption& zwoptType = colToFill.objAdd(TZWOption());
    zwoptType.m_strKey = m_strOptKey_Type;
    zwoptType.m_strPrompt = L"Multi-level Sensor Type";
    zwoptType.m_strInstDescr = strInstDescr();
    zwoptType.m_colValues.objAdd(TKeyValuePair(L"2", L"General"));
    zwoptType.m_colValues.objAdd(TKeyValuePair(L"1", L"Temperature"));
    zwoptType.m_colValues.objAdd(TKeyValuePair(L"3", L"Luminance"));
    zwoptType.m_colValues.objAdd(TKeyValuePair(L"4", L"Power Usage"));
    zwoptType.m_colValues.objAdd(TKeyValuePair(L"5", L"Humidity"));

    TZWOption& zwoptScale = colToFill.objAdd(TZWOption());
    zwoptScale.m_strKey = m_strOptKey_Scale;
    zwoptScale.m_strPrompt = L"Temperature Scale";
    zwoptScale.m_strInstDescr = strInstDescr();
    zwoptScale.m_colValues.objAdd(TKeyValuePair(L"F", L"Fahrenheit"));
    zwoptScale.m_colValues.objAdd(TKeyValuePair(L"C", L"Celsius"));

    TZWOption& zwoptData = colToFill.objAdd(TZWOption());
    zwoptData.m_strKey = m_strOptKey_Data;
    zwoptData.m_strPrompt = L"Sensor Data Type";
    zwoptData.m_strInstDescr = strInstDescr();
    zwoptData.m_colValues.objAdd(TKeyValuePair(L"Int", L"Integer"));
    zwoptData.m_colValues.objAdd(TKeyValuePair(L"Float", L"Float"));

    TZWOption& zwoptUnits = colToFill.objAdd(TZWOption());
    zwoptUnits.m_strKey = m_strOptKey_Units;
    zwoptUnits.m_strPrompt = L"Sensor Units";
    zwoptUnits.m_strInstDescr = strInstDescr();
    zwoptUnits.m_colValues.objAdd(TKeyValuePair(L"0",  L"None"));
    zwoptUnits.m_colValues.objAdd(TKeyValuePair(L"1",  L"dB"));
    zwoptUnits.m_colValues.objAdd(TKeyValuePair(L"2",  L"Degrees"));
    zwoptUnits.m_colValues.objAdd(TKeyValuePair(L"3",  L"KGrams"));
    zwoptUnits.m_colValues.objAdd(TKeyValuePair(L"4",  L"KWatts"));
    zwoptUnits.m_colValues.objAdd(TKeyValuePair(L"5",  L"Lbs"));
    zwoptUnits.m_colValues.objAdd(TKeyValuePair(L"6",  L"Lux"));
    zwoptUnits.m_colValues.objAdd(TKeyValuePair(L"7",  L"M-Bars"));
    zwoptUnits.m_colValues.objAdd(TKeyValuePair(L"8",  L"Ohms"));
    zwoptUnits.m_colValues.objAdd(TKeyValuePair(L"9",  L"Percent"));
    zwoptUnits.m_colValues.objAdd(TKeyValuePair(L"10", L"Volts"));
    zwoptUnits.m_colValues.objAdd(TKeyValuePair(L"11", L"Watts"));
}


//
//  Add any opt values (with an initial default value) to the list for any options we
//  have.
//
tCIDLib::TVoid
TZWCCMLSensor::LoadOptVals( const   TZWDevClass&                zwdcInst
                            ,       tZWaveLevi2Sh::TOptValList& colToFill
                            ,       TZWOptionVal&               zwoptvTmp) const
{
    if (!colToFill.bKeyExists(m_strOptKey_Type))
    {
        zwdcInst.QueryOptDef(zwoptvTmp, m_strOptKey_Type, L"2");
        colToFill.objAdd(zwoptvTmp);
    }

    if (!colToFill.bKeyExists(m_strOptKey_Scale))
    {
        zwdcInst.QueryOptDef(zwoptvTmp, m_strOptKey_Scale, L"F");
        colToFill.objAdd(zwoptvTmp);
    }

    if (!colToFill.bKeyExists(m_strOptKey_Data))
    {
        zwdcInst.QueryOptDef(zwoptvTmp, m_strOptKey_Data, L"Int");
        colToFill.objAdd(zwoptvTmp);
    }

    if (!colToFill.bKeyExists(m_strOptKey_Units))
    {
        zwdcInst.QueryOptDef(zwoptvTmp, m_strOptKey_Units, L"None");
        colToFill.objAdd(zwoptvTmp);
    }
}


// Make sure our fields are set to error state until good values are stored
tCIDLib::TVoid
TZWCCMLSensor::InitVals(const TZWaveUnit& unitMe,  MZWLeviSrvFuncs& mzwsfLevi)
{
    mzwsfLevi.LeviSetFldErr(m_c4FldId_Level);
}



//
//  Add our fields to the list. We either add just a generic multi-level value, or
//  a standard V2 dimmer field, depending on configuration.
//
tCIDLib::TVoid
TZWCCMLSensor::QueryFldDefs(        tCQCKit::TFldDefList& colAddTo
                            ,       TCQCFldDef&           flddTmp
                            , const TZWaveUnit&           unitMe
                            ,       TString&              strTmp1
                            ,       TString&              strTmp2) const
{
    const TString& strScale = unitMe.strFindOptVal(m_strOptKey_Scale);
    const TString& strUnits = unitMe.strFindOptVal(m_strOptKey_Units);

    // Get our sensor type enum from the type option
    const tZWaveLevi2Sh::EMLSensTypes eSensType = tZWaveLevi2Sh::EMLSensTypes
    (
        unitMe.strFindOptVal(m_strOptKey_Type).c4Val()
    );
    const TString strShortName = tZWaveLevi2Sh::strAltXlatEMLSensTypes(eSensType);

    tCQCKit::EFldTypes eDataType;
    if (unitMe.strFindOptVal(m_strOptKey_Data) == L"Int")
        eDataType = tCQCKit::EFldTypes::Int;
    else
        eDataType = tCQCKit::EFldTypes::Float;

    // Based on units, set up any reasonable ranges we an come up with
    strTmp2.Clear();
    if (strUnits == L"9")
    {
        // Percent
        strTmp2 = L"Range: 0, 100";
    }
     else if (strUnits == L"2")
    {
        // Degrees, which w adjust for scale
        if (strScale == L"F")
            strTmp2 = L"Range: -25, 100";
        else
            strTmp2 = L"Range: -28, 50";
    }

    if (eSensType == tZWaveLevi2Sh::EMLSensTypes::Temp)
    {
        BuildInstName(unitMe, strShortName.pszBuffer(), strTmp1);
        flddTmp.Set(strTmp1, eDataType, tCQCKit::EFldAccess::Read, strTmp2);
        colAddTo.objAdd(flddTmp);
    }
     else if ((eSensType == tZWaveLevi2Sh::EMLSensTypes::Gen)
          ||  (eSensType == tZWaveLevi2Sh::EMLSensTypes::Hum))
    {
        BuildInstName(unitMe, strShortName.pszBuffer(), strTmp1);
        flddTmp.Set(strTmp1, eDataType, tCQCKit::EFldAccess::Read, strTmp2);
        colAddTo.objAdd(flddTmp);
    }
     else if (eSensType == tZWaveLevi2Sh::EMLSensTypes::Lum)
    {
        if (strUnits == L"6")
            BuildInstName(unitMe, L"Lux", strTmp1);
        else
            BuildInstName(unitMe, L"Lum", strTmp1);
        flddTmp.Set(strTmp1, eDataType, tCQCKit::EFldAccess::Read, strTmp2);
        colAddTo.objAdd(flddTmp);
    }
     else if (eSensType == tZWaveLevi2Sh::EMLSensTypes::Power)
    {
        strTmp1 = L"RESMON#";
        AppendInstName(unitMe, kCIDLib::pszEmptyZStr, strTmp1);

        flddTmp.Set(strTmp1, eDataType, tCQCKit::EFldAccess::Read);
        colAddTo.objAdd(flddTmp);
    }
}


// We look our field and store it away
tCIDLib::TVoid
TZWCCMLSensor::StoreFldIds( const   TZWaveUnit&         unitMe
                            , const MZWLeviSrvFuncs&    mzwsfLevi
                            ,       TString&            strTmp1
                            ,       TString&            strTmp2)
{
    // Get our sensor type enum from the type option
    const tZWaveLevi2Sh::EMLSensTypes eSensType = tZWaveLevi2Sh::EMLSensTypes
    (
        unitMe.strFindOptVal(m_strOptKey_Type).c4Val()
    );
    const TString strShortName = tZWaveLevi2Sh::strAltXlatEMLSensTypes(eSensType);


    if (eSensType == tZWaveLevi2Sh::EMLSensTypes::Temp)
    {
        BuildInstName(unitMe, strShortName.pszBuffer(), strTmp1);
    }
     else if ((eSensType == tZWaveLevi2Sh::EMLSensTypes::Gen)
          ||  (eSensType == tZWaveLevi2Sh::EMLSensTypes::Hum))
    {
        BuildInstName(unitMe, strShortName.pszBuffer(), strTmp1);
    }
     else if (eSensType == tZWaveLevi2Sh::EMLSensTypes::Lum)
    {
        if (unitMe.strFindOptVal(m_strOptKey_Units) == L"6")
            BuildInstName(unitMe, L"Lux", strTmp1);
        else
            BuildInstName(unitMe, L"Lum", strTmp1);
    }
     else if (eSensType == tZWaveLevi2Sh::EMLSensTypes::Power)
    {
        strTmp1 = L"RESMON#";
        AppendInstName(unitMe, kCIDLib::pszEmptyZStr, strTmp1);
    }

    m_c4FldId_Level = mzwsfLevi.c4LeviFldIdFromName(strTmp1);
}


// ---------------------------------------------------------------------------
//  TZWCCMLSensor: Private, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TZWCCMLSensor::bParseReport(const   tCIDLib::TCardList& fcolVals
                            ,       tCIDLib::TCard1&    c1Type
                            ,       tCIDLib::TCard1&    c1Precision
                            ,       tCIDLib::TCard1&    c1Scale
                            ,       tCIDLib::TCard1&    c1Size)
{
    if (fcolVals.c4ElemCount() < 5)
        return kCIDLib::False;

    // Check the type
    if ((fcolVals[2] < 1) || (fcolVals[2] > 5))
        return kCIDLib::False;

    c1Type = tCIDLib::TCard1(fcolVals[2]);
    c1Precision = tCIDLib::TCard1(fcolVals[3] >> 5) & 0x7;
    c1Scale = tCIDLib::TCard1(fcolVals[3] >> 3) & 0x3;
    c1Size = tCIDLib::TCard1(fcolVals[3]) & 0x7;

    return kCIDLib::True;
}

//
//  We can potentially store the value from multiple place,s so we break out
//  the grunt work to here.
//
tCIDLib::TVoid
TZWCCMLSensor::StoreValue(  const   tCIDLib::TCardList& fcolVals
                            , const TZWaveUnit&         unitMe
                            ,       MZWLeviSrvFuncs&    mzwsfLevi)
{
    // Parse out the format info so we can see what the incoming format is
    tCIDLib::TCard1 c1Type;
    tCIDLib::TCard1 c1Precision;
    tCIDLib::TCard1 c1Scale;
    tCIDLib::TCard1 c1Size;
    tCIDLib::TBoolean bRes = bParseReport(fcolVals, c1Type, c1Precision, c1Scale, c1Size);

    if (!bRes)
    {
        if (mzwsfLevi.bLeviTraceEnabled())
        {
            TTextOutStream& strmTrace = mzwsfLevi.strmLeviTrace();
            strmTrace << L"    Could not parse ML sensor report" << kCIDLib::EndLn;
        }
        mzwsfLevi.LeviSetFldErr(m_c4FldId_Level);
        return;
    }

    if (mzwsfLevi.bLeviTraceEnabled())
    {
        TTextOutStream& strmTrace = mzwsfLevi.strmLeviTrace();
        strmTrace   << L"    Type=" << c1Type
                    << L",    Precision=" << c1Precision
                    << L",    Scale=" << c1Scale
                    << L",    Size=" << c1Size
                    << kCIDLib::EndLn;
    }

    // Get our optoin values and see if the data match that
    const TString& strData = unitMe.strFindOptVal(m_strOptKey_Data);
    const TString& strScale = unitMe.strFindOptVal(m_strOptKey_Scale);
    const TString& strUnits = unitMe.strFindOptVal(m_strOptKey_Units);

    // Make sure we are getting the temp scale we were told to expect
    bRes = kCIDLib::True;
    if ((c1Type == 1) && (strScale == L"C"))
        bRes = (c1Scale == 0);
    else if ((c1Type == 1) && (strScale == L"F"))
        bRes = (c1Scale == 1);

    if (!bRes)
    {
        if (mzwsfLevi.eLeviVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
        {
            facZWaveLevi2Sh().LogMsg
            (
                CID_FILE
                , CID_LINE
                , L"Data format sent for MLSensor %(1) was not as configured"
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
                , unitMe.strName()
            );
        }
        mzwsfLevi.LeviSetFldErr(m_c4FldId_Level);
        return;
    }

    // Looks, ok, so convert and store
    tCIDLib::TInt4 i4Val = 0;
    if (c1Size == 1)
    {
        tCIDLib::TCard1 c1Val = tCIDLib::TCard1(fcolVals[4]);
        i4Val = tCIDLib::TInt1(c1Val);
    }
     else if (c1Size == 2)
    {
        tCIDLib::TCard2 c2Val = tCIDLib::TCard1(fcolVals[4]);
        c2Val <<= 8;
        c2Val |= tCIDLib::TCard1(fcolVals[5]);
        i4Val = tCIDLib::TInt2(c2Val);
    }
     else if (c1Size == 4)
    {
        tCIDLib::TCard4 c4Val = tCIDLib::TCard1(fcolVals[4]);
        c4Val <<= 8;
        c4Val |= tCIDLib::TCard1(fcolVals[5]);
        c4Val <<= 8;
        c4Val |= tCIDLib::TCard1(fcolVals[6]);
        c4Val <<= 8;
        c4Val |= tCIDLib::TCard1(fcolVals[7]);
        i4Val = tCIDLib::TInt4(c4Val);
    }
     else
    {
        if (mzwsfLevi.eLeviVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
        {
            facZWaveLevi2Sh().LogMsg
            (
                CID_FILE
                , CID_LINE
                , L"Got invalid ML sensor data size value. Name=%(1), Size=%(2)"
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
                , unitMe.strName()
                , TCardinal(c1Size)
            );
        }
        mzwsfLevi.LeviSetFldErr(m_c4FldId_Level);
        return;
    }

    // Store the value based on type
    if (c1Precision)
    {
        // Convert to float and store
        tCIDLib::TFloat8 f8Val = tCIDLib::TFloat8(i4Val);
        f8Val /= tCIDLib::TFloat8(10 * c1Precision);
        mzwsfLevi.bLeviStoreFloatFld(m_c4FldId_Level, f8Val);
    }
     else
    {
        // Store directly as an int
        mzwsfLevi.bLeviStoreIntFld(m_c4FldId_Level, i4Val);
    }

    // Make sure we are now marked as having valid info
    bWaitInitVals(kCIDLib::False);
}

