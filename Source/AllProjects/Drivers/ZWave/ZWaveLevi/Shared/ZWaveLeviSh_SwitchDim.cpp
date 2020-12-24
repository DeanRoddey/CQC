//
// FILE NAME: ZWaveLeviSh_SwitchDim.cpp
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
//  This file provides the implementation for the Z-Wave unit class
//  derivatives related to switches and dimmers.
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
AdvRTTIDecls(TZWBinSwitch,TZWaveUnit)
AdvRTTIDecls(TZWMLSwitch,TZWaveUnit)


// ---------------------------------------------------------------------------
//  Local types and constants
// ---------------------------------------------------------------------------
namespace ZWaveLeviSh_UnitInfo
{
    // -----------------------------------------------------------------------
    //  Base class persistent format versions
    // -----------------------------------------------------------------------
    const tCIDLib::TCard2   c2FmtBinSwVer       = 1;
    const tCIDLib::TCard2   c2FmtMultiSwVer     = 1;
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
        , 0
        , L"BinSwitch"
        , L"On/Off Switch"
        , tZWaveLeviSh::EUnitCaps
          (
            tZWaveLeviSh::EUnitCap_BasicClass
            | tZWaveLeviSh::EUnitCap_Readable
            | tZWaveLeviSh::EUnitCap_Writeable
          )
        , tZWaveLeviSh::EUnitCap_None
    )
    , m_c4FldId_Switch(kCIDLib::c4MaxCard)
{
    // Override the default opt info to zero
    c4OptInfo(0);
}

TZWBinSwitch::TZWBinSwitch(const tCIDLib::TCard4 c4UnitId) :

    TZWaveUnit
    (
        GENERIC_TYPE_SWITCH_BINARY
        , 0
        , L"BinSwitch"
        , L"On/Off Switch"
        , tZWaveLeviSh::EUnitCaps
          (
            tZWaveLeviSh::EUnitCap_BasicClass
            | tZWaveLeviSh::EUnitCap_Readable
            | tZWaveLeviSh::EUnitCap_Writeable
          )
        , tZWaveLeviSh::EUnitCap_None
        , c4UnitId
    )
    , m_c4FldId_Switch(kCIDLib::c4MaxCard)
{
    // Override the default opt info to zero
    c4OptInfo(0);
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
TZWBinSwitch& TZWBinSwitch::operator=(const TZWBinSwitch& unitSrc)
{
    if (this != &unitSrc)
    {
        TParent::operator=(unitSrc);
        m_c4FldId_Switch = unitSrc.m_c4FldId_Switch;
    }
    return *this;
}

// We don't have any of our own persisted members, so just call the parent for now
tCIDLib::TBoolean TZWBinSwitch::operator==(const TZWBinSwitch& unitSrc) const
{
    return TParent::operator==(unitSrc);
}


tCIDLib::TBoolean TZWBinSwitch::operator!=(const TZWBinSwitch& unitSrc) const
{
    return !operator==(unitSrc);
}


// ---------------------------------------------------------------------------
//  TZWBinSwitch: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TZWBinSwitch::bBuildSetMsg( const   tCIDLib::TCard4 c4FldId
                            , const TString&        strValue
                            ,       TString&        strToFill
                            ,       TCQCServerBase& sdrvLevi)
{
    if (c4FldId == m_c4FldId_Switch)
    {
        // We respond to on/off commands
        if (strValue.bCompareI(kZWaveLeviSh::strGetMsg_Off)
        ||  strValue.bCompareI(kZWaveLeviSh::strGetMsg_On))
        {
            // It's a simple on or off command
            strToFill = L"N";
            strToFill.AppendFormatted(c4Id());
            if (strValue.bCompareI(kZWaveLeviSh::strGetMsg_On))
                strToFill.Append(L"ON");
            else
                strToFill.Append(L"OF");
        }
    }
     else
    {
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


// Store a received basic report value
tCIDLib::TBoolean
TZWBinSwitch::bStoreBasicReport(const   tCIDLib::TCard4 c4Value
                                ,       TCQCServerBase& sdrvLevi)
{
    // Call a helper to store it
    StoreValue(c4Value, sdrvLevi);
    return kCIDLib::True;
}


tCIDLib::TBoolean TZWBinSwitch::bOwnsFld(const tCIDLib::TCard4 c4FldId) const
{
    return (m_c4FldId_Switch == c4FldId);
}


tCIDLib::TBoolean
TZWBinSwitch::bStoreFldIds( const   TVector<TCQCFldDef>&    colFlds
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
            m_c4FldId_Switch = fcolIds[c4Index];
            return kCIDLib::True;
        }
    }
    return kCIDLib::False;
}


// Handles non-basic type reports
tCIDLib::TBoolean
TZWBinSwitch::bStoreGenReport(  const   tCIDLib::TCard4 ac4Values[]
                                , const tCIDLib::TCard4 c4ValCnt
                                ,       TCQCServerBase& sdrvLevi)
{
    // We get general reports as basic status
    if ((ac4Values[0] == COMMAND_CLASS_BASIC)
    &&  ((ac4Values[1] == BASIC_SET) || (ac4Values[1] == BASIC_REPORT))
    &&  (c4ValCnt == 3))
    {
        StoreValue(ac4Values[2], sdrvLevi);
        return kCIDLib::True;
    }
     else if ((c4ValCnt == 5)
          &&  (ac4Values[0] == COMMAND_CLASS_SCENE_ACTUATOR_CONF)
          &&  (ac4Values[1] == SCENE_ACTUATOR_CONF_REPORT)
          &&  (ac4Values[2] == 0))
    {
        //
        //  It's a scene actuator config report for scene 0, so we treat that as
        //  the current value.
        //
        StoreValue(ac4Values[3], sdrvLevi);
        return kCIDLib::True;
    }
    return TParent::bStoreGenReport(ac4Values, c4ValCnt, sdrvLevi);
}


// We treat these as current value reports
tCIDLib::TBoolean
TZWBinSwitch::bStoreSceneReport(const   tCIDLib::TCard4
                                , const tCIDLib::TCard4 c4Level
                                , const tCIDLib::TCard4
                                ,       TCQCServerBase& sdrvLevi)
{
    // Call a helper to store the value
    StoreValue(c4Level, sdrvLevi);
    return kCIDLib::True;
}


// Return our optional info strings
tCIDLib::TBoolean
TZWBinSwitch::bWantsOptInfo(tCIDLib::TStrCollect& colToFill) const
{
    colToFill.objAdd(facZWaveLeviSh().strMsg(kZWShMsgs::midOpt_Gen_None));
    colToFill.objAdd(facZWaveLeviSh().strMsg(kZWShMsgs::midOpt_Switch_Light));

    return kCIDLib::True;
}


tCIDLib::TCard4
TZWBinSwitch::c4QueryFldDefs(TVector<TCQCFldDef>&   colAddTo
                            , TCQCFldDef&           flddTmp) const
{
    //
    //  We just add our one boolean field. If the 'is a' flag is set, then
    //  the user indicated it is a light switch, so we set that semantic
    //  type.
    //
    flddTmp.Set
    (
        strName()
        , tCQCKit::EFldTypes::Boolean
        , eDefFldAccess()
        , (c4OptInfo() == 1) ? tCQCKit::EFldSTypes::LightSwitch : tCQCKit::EFldSTypes::Generic
    );
    colAddTo.objAdd(flddTmp);

    return 1;
}


// Call our parent and zero out our field id
tCIDLib::TVoid TZWBinSwitch::PrepForUse()
{
    TParent::PrepForUse();
    m_c4FldId_Switch = kCIDLib::c4MaxCard;
}


// Set all of our readable unit status oriented fields to error state
tCIDLib::TVoid TZWBinSwitch::SetFieldErrs(TCQCServerBase& sdrvLevi)
{
    if (m_c4FldId_Switch != kCIDLib::c4MaxCard)
    {
        // If it's readable, mark it
        if (sdrvLevi.flddFromId(m_c4FldId_Switch).bIsReadable())
            sdrvLevi.SetFieldErr(m_c4FldId_Switch, kCIDLib::True);
    }
}


// ---------------------------------------------------------------------------
//  TZWBinSwitch: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  We can potentially store the value from multiple place,s so we break out
//  the grunt work to here.
//
tCIDLib::TVoid
TZWBinSwitch::StoreValue(const tCIDLib::TCard4 c4Val, TCQCServerBase& sdrvLevi)
{
    //
    //  Update our last poll time to now since we got a new value. Tell it to
    //  reset the last value time stamp as well.
    //
    ResetPollTime(kCIDLib::True);

    if (!bCheckFldId(m_c4FldId_Switch, L"switch"))
        return;

    //
    //  Just write the boolean value to our field. If the value changes, and
    //  this guy is a light switch, send a load change trigger.
    //
    tCIDLib::TBoolean bChanged = sdrvLevi.bStoreBoolFld
    (
        m_c4FldId_Switch, c4Val != 0, kCIDLib::True
    );

    // If changed and we are a light switch, send a trigger
    if (bChanged && (c4OptInfo() == 1))
    {
        TString strId;
        TString strName;
        strId.SetFormatted(c4Id());
        if (sdrvLevi.bQueryFldName(m_c4FldId_Switch, strName))
        {
            sdrvLevi.QueueEventTrig
            (
                tCQCKit::EStdDrvEvs::LoadChange
                , strName
                , facCQCKit().strBoolOffOn(c4Val != 0)
                , strId
                , strName
                , TString::strEmpty()
            );
        }
    }
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
        , 0
        , L"MLSwitch"
        , L"Multi-level Switch"
        , tZWaveLeviSh::EUnitCaps
          (
            tZWaveLeviSh::EUnitCap_BasicClass
            | tZWaveLeviSh::EUnitCap_Readable
            | tZWaveLeviSh::EUnitCap_Writeable
          )
        , tZWaveLeviSh::EUnitCap_None
    )
    , m_c4FldId_Level(kCIDLib::c4MaxCard)
    , m_c4FldId_State(kCIDLib::c4MaxCard)
{
    // Override the default opt info to zero
    c4OptInfo(0);
}

TZWMLSwitch::TZWMLSwitch(const tCIDLib::TCard4 c4UnitId) :

    TZWaveUnit
    (
        GENERIC_TYPE_SWITCH_MULTILEVEL
        , 0
        , L"MLSwitch"
        , L"Multi-level Switch"
        , tZWaveLeviSh::EUnitCaps
          (
            tZWaveLeviSh::EUnitCap_BasicClass
            | tZWaveLeviSh::EUnitCap_Readable
            | tZWaveLeviSh::EUnitCap_Writeable
          )
        , tZWaveLeviSh::EUnitCap_None
        , c4UnitId
    )
    , m_c4FldId_Level(kCIDLib::c4MaxCard)
    , m_c4FldId_State(kCIDLib::c4MaxCard)
{
    // Override the default opt info to zero
    c4OptInfo(0);
}

TZWMLSwitch::TZWMLSwitch(const TZWMLSwitch& unitToCopy) :

    TZWaveUnit(unitToCopy)
    , m_c4FldId_Level(unitToCopy.m_c4FldId_Level)
    , m_c4FldId_State(unitToCopy.m_c4FldId_State)
{
}

TZWMLSwitch::~TZWMLSwitch()
{
}


// ---------------------------------------------------------------------------
//  TZWMLSwitch: Public operators
// ---------------------------------------------------------------------------
TZWMLSwitch& TZWMLSwitch::operator=(const TZWMLSwitch& unitSrc)
{
    if (this != &unitSrc)
    {
        TParent::operator=(unitSrc);
        m_c4FldId_Level = unitSrc.m_c4FldId_Level;
        m_c4FldId_State = unitSrc.m_c4FldId_State;
    }
    return *this;
}


// We don't have any of our own persisted members, so just call the parent for now
tCIDLib::TBoolean TZWMLSwitch::operator==(const TZWMLSwitch& unitSrc) const
{
    return TParent::operator==(unitSrc);
}


tCIDLib::TBoolean TZWMLSwitch::operator!=(const TZWMLSwitch& unitSrc) const
{
    return !operator==(unitSrc);
}


// ---------------------------------------------------------------------------
//  TZWMLSwitch: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  Handle a value write
//
tCIDLib::TBoolean
TZWMLSwitch::bBuildSetMsg(  const   tCIDLib::TCard4 c4FldId
                            , const TString&        strValue
                            ,       TString&        strToFill
                            ,       TCQCServerBase& sdrvLevi)

{
    if (c4FldId == m_c4FldId_Level)
    {
        // We can do either on/off or level
        if (strValue.bCompareI(kZWaveLeviSh::strGetMsg_Off)
        ||  strValue.bCompareI(kZWaveLeviSh::strGetMsg_On))
        {
            // It's a simple on or off command
            strToFill = L"N";
            strToFill.AppendFormatted(c4Id());
            if (strValue.bCompareI(kZWaveLeviSh::strGetMsg_On))
                strToFill.Append(L"ON");
            else
                strToFill.Append(L"OF");
        }
         else
        {
            // Has to be a level command, so convert to a number
            tCIDLib::TCard4 c4Val;
            if (!strValue.bToCard4(c4Val, tCIDLib::ERadices::Dec)
            ||  (c4Val > 99))
            {
                return kCIDLib::False;
            }

            strToFill = L"N";
            strToFill.AppendFormatted(c4Id());
            strToFill.Append(L'L');
            strToFill.AppendFormatted(c4Val);
        }
    }
     else if (c4FldId == m_c4FldId_State)
    {
        // Has to be a simple on/off
        if (strValue.bCompareI(kZWaveLeviSh::strGetMsg_Off)
        ||  strValue.bCompareI(kZWaveLeviSh::strGetMsg_On))
        {
            // It's a simple on or off command
            strToFill = L"N";
            strToFill.AppendFormatted(c4Id());
            if (strValue.bCompareI(kZWaveLeviSh::strGetMsg_On))
                strToFill.Append(L"ON");
            else
                strToFill.Append(L"OF");
        }
         else
        {
            // Not legal
            return kCIDLib::False;
        }
    }
     else
    {
        // Nothing we understand
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


// Store away an incoming basic report
tCIDLib::TBoolean
TZWMLSwitch::bStoreBasicReport( const   tCIDLib::TCard4 c4Value
                                ,       TCQCServerBase& sdrvLevi)
{
    // Call a helper to store the value
    StoreValue(c4Value, sdrvLevi);
    return kCIDLib::True;
}



tCIDLib::TBoolean TZWMLSwitch::bOwnsFld(const tCIDLib::TCard4 c4FldId) const
{
    return  (m_c4FldId_Level == c4FldId)
            || (m_c4FldId_State == c4FldId);
}


tCIDLib::TBoolean
TZWMLSwitch::bStoreFldIds(  const   TVector<TCQCFldDef>&    colFlds
                            , const tZWaveLeviSh::TValList& fcolIds)
{
    const TString& strLevel = strName();
    const TString strState(strLevel, L"_State");

    // Init our fields to an invalid value so we know if we got them all
    m_c4FldId_Level = kCIDLib::c4MaxCard;
    m_c4FldId_State = kCIDLib::c4MaxCard;

    // Find our field and store the id
    const tCIDLib::TCard4 c4Count = colFlds.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TCQCFldDef& flddCur = colFlds[c4Index];
        if (flddCur.strName() == strLevel)
            m_c4FldId_Level = fcolIds[c4Index];
        else if (flddCur.strName() == strState)
            m_c4FldId_State = fcolIds[c4Index];
    }

    // If they all got set, then
    return (m_c4FldId_Level != kCIDLib::c4MaxCard)
           && (m_c4FldId_State != kCIDLib::c4MaxCard);
}


// Handles non-basic type reports
tCIDLib::TBoolean
TZWMLSwitch::bStoreGenReport(const  tCIDLib::TCard4 ac4Values[]
                            , const tCIDLib::TCard4 c4ValCnt
                            ,       TCQCServerBase& sdrvLevi)
{
    if ((c4ValCnt == 3)
    &&  (ac4Values[0] == COMMAND_CLASS_BASIC)
    &&  ((ac4Values[1] == BASIC_SET) || (ac4Values[1] == BASIC_REPORT)))
    {
        StoreValue(ac4Values[2], sdrvLevi);
        return kCIDLib::True;
    }
     else if ((c4ValCnt == 5)
          &&  (ac4Values[0] == COMMAND_CLASS_SCENE_ACTUATOR_CONF)
          &&  (ac4Values[1] == SCENE_ACTUATOR_CONF_REPORT)
          &&  (ac4Values[2] == 0))
    {
        //
        //  It's a scene actuator config report for scene 0, so we treat that as
        //  the current value.
        //
        StoreValue(ac4Values[3], sdrvLevi);
        return kCIDLib::True;
    }
    return TParent::bStoreGenReport(ac4Values, c4ValCnt, sdrvLevi);
}


// We treat these as current value reports
tCIDLib::TBoolean
TZWMLSwitch::bStoreSceneReport( const   tCIDLib::TCard4
                                , const tCIDLib::TCard4 c4Level
                                , const tCIDLib::TCard4
                                ,       TCQCServerBase& sdrvLevi)
{
    // Call a helper to store the value
    StoreValue(c4Level, sdrvLevi);
    return kCIDLib::True;
}


// Return our optional info strings
tCIDLib::TBoolean
TZWMLSwitch::bWantsOptInfo(tCIDLib::TStrCollect& colToFill) const
{
    colToFill.objAdd(facZWaveLeviSh().strMsg(kZWShMsgs::midOpt_Gen_None));
    colToFill.objAdd(facZWaveLeviSh().strMsg(kZWShMsgs::midOpt_Switch_Dimmer));

    return kCIDLib::True;
}


tCIDLib::TCard4
TZWMLSwitch::c4QueryFldDefs(TVector<TCQCFldDef>&    colAddTo
                            , TCQCFldDef&           flddTmp) const
{
    const TString& strLevel = strName();
    const TString strState(strLevel, L"_State");

    //
    //  If a dimmer, then we set semantic field type info on our
    //  fields. Else they are just generic.
    //
    if (c4OptInfo() == 1)
    {
        flddTmp.Set
        (
            strLevel
            , tCQCKit::EFldTypes::Card
            , eDefFldAccess()
            , tCQCKit::EFldSTypes::Dimmer
            , L"Range:0,99"
        );
        colAddTo.objAdd(flddTmp);

        flddTmp.Set
        (
            strState
            , tCQCKit::EFldTypes::Boolean
            , eDefFldAccess()
            , tCQCKit::EFldSTypes::LightSwitch
        );
        colAddTo.objAdd(flddTmp);
    }
     else
    {
        flddTmp.Set
        (
            strName()
            , tCQCKit::EFldTypes::Card
            , eDefFldAccess()
        );
        colAddTo.objAdd(flddTmp);

        flddTmp.Set
        (
            strState
            , tCQCKit::EFldTypes::Boolean
            , eDefFldAccess()
        );
        colAddTo.objAdd(flddTmp);
    }

    // And indicate we added 2 fields
    return 2;
}


// Call our parent and zero out our field id
tCIDLib::TVoid TZWMLSwitch::PrepForUse()
{
    TParent::PrepForUse();
    m_c4FldId_Level = kCIDLib::c4MaxCard;
    m_c4FldId_State = kCIDLib::c4MaxCard;
}


// Set all of our readable unit status oriented fields to error state
tCIDLib::TVoid TZWMLSwitch::SetFieldErrs(TCQCServerBase& sdrvLevi)
{
    if (m_c4FldId_Level != kCIDLib::c4MaxCard)
    {
        // If it's readable, mark it
        if (sdrvLevi.flddFromId(m_c4FldId_Level).bIsReadable())
            sdrvLevi.SetFieldErr(m_c4FldId_Level, kCIDLib::True);
    }

    if (m_c4FldId_State != kCIDLib::c4MaxCard)
    {
        // If it's readable, mark it
        if (sdrvLevi.flddFromId(m_c4FldId_State).bIsReadable())
            sdrvLevi.SetFieldErr(m_c4FldId_State, kCIDLib::True);
    }
}


// ---------------------------------------------------------------------------
//  TZWMLSwitch: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  We can potentially store the value from multiple place,s so we break out
//  the grunt work to here.
//
tCIDLib::TVoid
TZWMLSwitch::StoreValue(const tCIDLib::TCard4 c4Val, TCQCServerBase& sdrvLevi)
{
    // Watch for the special On value which we only use for the state
    if (c4Val == 0xFF)
        return;

    // If not the magic ON value, but greater than the max dimmer value
    if ((c4Val > 99) && (c4Val != 0xFF))
    {
        // Invalid value. Put the field into error state
        if (m_c4FldId_Level != kCIDLib::c4MaxCard)
            sdrvLevi.SetFieldErr(m_c4FldId_Level, kCIDLib::True);

        if (sdrvLevi.eVerboseLevel() >= tCQCKit::EVerboseLvls::High)
        {
            facZWaveLeviSh().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kZWShErrs::errcUnit_BadValue
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Range
                , strName()
            );
        }
        return;
    }

    //
    //  Update our last poll time to now since we got a new value. Tell him to
    //  update the last value time as well.
    //
    ResetPollTime(kCIDLib::True);

    // Set the state based on non-zero value
    if (bCheckFldId(m_c4FldId_State, L"state"))
        sdrvLevi.bStoreBoolFld(m_c4FldId_State, c4Val != 0, kCIDLib::True);

    // If it's a valid dimmer value, store that
    if (c4Val < 100)
    {
        if (bCheckFldId(m_c4FldId_Level, L"dimmer"))
            sdrvLevi.bStoreCardFld(m_c4FldId_Level, c4Val, kCIDLib::True);
    }
}


