//
// FILE NAME: HAIOmniS_Item.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/13/2006
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
//  This file implements the classes that model the named items we query
//  from the Omni and make available for control.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "HAIOmniS.hpp"



// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(THAIOmniItem,TObject)
RTTIDecls(THAIOmniArea,THAIOmniItem)
RTTIDecls(THAIOmniThermo,THAIOmniItem)
RTTIDecls(THAIOmniUnit,THAIOmniItem)
RTTIDecls(THAIOmniZone,THAIOmniItem)



// ---------------------------------------------------------------------------
//   CLASS: THAIOmniItem
//  PREFIX: item
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  THAIOmniItem: Destructor
// ---------------------------------------------------------------------------
THAIOmniItem::~THAIOmniItem()
{
}


// ---------------------------------------------------------------------------
//  THAIOmniItem: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TCard4 THAIOmniItem::c4ItemNum() const
{
    return m_c4ItemNum;
}

tHAIOmniSh::EItemTypes THAIOmniItem::eType() const
{
    return m_eType;
}

const TString& THAIOmniItem::strName() const
{
    return m_strName;
}


// ---------------------------------------------------------------------------
//  THAIOmniItem: Hidden constructors and operators
// ---------------------------------------------------------------------------
THAIOmniItem::THAIOmniItem( const   TString&                strName
                            , const tHAIOmniSh::EItemTypes  eType
                            , const tCIDLib::TCard4         c4ItemNum) :
    m_c4ItemNum(c4ItemNum)
    , m_eType(eType)
    , m_strName(strName)
{
}

THAIOmniItem::THAIOmniItem(const THAIOmniItem& itemToCopy) :

    m_c4ItemNum(itemToCopy.m_c4ItemNum)
    , m_eType(itemToCopy.m_eType)
    , m_strName(itemToCopy.m_strName)
{
}

THAIOmniItem& THAIOmniItem::operator=(const THAIOmniItem& itemToAssign)
{
    if (this != &itemToAssign)
    {
        m_c4ItemNum = itemToAssign.m_c4ItemNum;
        m_eType     = itemToAssign.m_eType;
        m_strName   = itemToAssign.m_strName;
    }
    return *this;
}




// ---------------------------------------------------------------------------
//   CLASS: THAIOmniArea
//  PREFIX: item
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  THAIOmniArea: Constructors and Destructor
// ---------------------------------------------------------------------------
THAIOmniArea::THAIOmniArea( const   TString&        strName
                            , const tCIDLib::TCard4 c4ItemNum) :

    THAIOmniItem(strName, tHAIOmniSh::EItem_Area, c4ItemNum)
    , m_c4FldId_Alarmed(kCIDLib::c4MaxCard)
    , m_c4FldId_AlarmBits(kCIDLib::c4MaxCard)
    , m_c4FldId_AlarmList(kCIDLib::c4MaxCard)
    , m_c4FldId_ArmStatus(kCIDLib::c4MaxCard)
    , m_c4FldId_InDelay(kCIDLib::c4MaxCard)
{
}

THAIOmniArea::THAIOmniArea(const THAIOmniArea& itemToCopy) :

    THAIOmniItem(itemToCopy)
    , m_c4FldId_Alarmed(itemToCopy.m_c4FldId_Alarmed)
    , m_c4FldId_AlarmBits(itemToCopy.m_c4FldId_AlarmBits)
    , m_c4FldId_AlarmList(itemToCopy.m_c4FldId_AlarmList)
    , m_c4FldId_ArmStatus(itemToCopy.m_c4FldId_ArmStatus)
    , m_c4FldId_InDelay(itemToCopy.m_c4FldId_InDelay)
{
}

THAIOmniArea::~THAIOmniArea()
{
}


// ---------------------------------------------------------------------------
//  THAIOmniArea: Public operators
// ---------------------------------------------------------------------------
THAIOmniArea& THAIOmniArea::operator=(const THAIOmniArea& itemToAssign)
{
    if (this != &itemToAssign)
    {
        TParent::operator=(itemToAssign);
        m_c4FldId_Alarmed   = itemToAssign.m_c4FldId_Alarmed;
        m_c4FldId_AlarmBits = itemToAssign.m_c4FldId_AlarmBits;
        m_c4FldId_AlarmList = itemToAssign.m_c4FldId_AlarmList;
        m_c4FldId_ArmStatus = itemToAssign.m_c4FldId_ArmStatus;
        m_c4FldId_InDelay   = itemToAssign.m_c4FldId_InDelay;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  THAIOmniTCPShThermo: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean THAIOmniArea::bOwnsField(const tCIDLib::TCard4 c4FldId) const
{
    return  (c4FldId == m_c4FldId_Alarmed)
            || (c4FldId == m_c4FldId_AlarmBits)
            || (c4FldId == m_c4FldId_AlarmList)
            || (c4FldId == m_c4FldId_ArmStatus)
            || (c4FldId == m_c4FldId_InDelay);
}


// ---------------------------------------------------------------------------
//  Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TCard4 THAIOmniArea::c4FldIdAlarmed() const
{
    return m_c4FldId_Alarmed;
}

tCIDLib::TCard4 THAIOmniArea::c4FldIdAlarmed(const tCIDLib::TCard4 c4ToSet)
{
    m_c4FldId_Alarmed = c4ToSet;
    return m_c4FldId_Alarmed;
}


tCIDLib::TCard4 THAIOmniArea::c4FldIdAlarmBits() const
{
    return m_c4FldId_AlarmBits;
}

tCIDLib::TCard4 THAIOmniArea::c4FldIdAlarmBits(const tCIDLib::TCard4 c4ToSet)
{
    m_c4FldId_AlarmBits = c4ToSet;
    return m_c4FldId_AlarmBits;
}


tCIDLib::TCard4 THAIOmniArea::c4FldIdAlarmList() const
{
    return m_c4FldId_AlarmList;
}

tCIDLib::TCard4 THAIOmniArea::c4FldIdAlarmList(const tCIDLib::TCard4 c4ToSet)
{
    m_c4FldId_AlarmList = c4ToSet;
    return m_c4FldId_AlarmList;
}


tCIDLib::TCard4 THAIOmniArea::c4FldIdArmStatus() const
{
    return m_c4FldId_ArmStatus;
}

tCIDLib::TCard4 THAIOmniArea::c4FldIdArmStatus(const tCIDLib::TCard4 c4ToSet)
{
    m_c4FldId_ArmStatus = c4ToSet;
    return m_c4FldId_ArmStatus;
}


tCIDLib::TCard4 THAIOmniArea::c4FldIdInDelay() const
{
    return m_c4FldId_InDelay;
}

tCIDLib::TCard4 THAIOmniArea::c4FldIdInDelay(const tCIDLib::TCard4 c4ToSet)
{
    m_c4FldId_InDelay = c4ToSet;
    return m_c4FldId_InDelay;
}




// ---------------------------------------------------------------------------
//   CLASS: THAIOmniThermo
//  PREFIX: item
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  THAIOmniThermo: Constructors and Destructor
// ---------------------------------------------------------------------------
THAIOmniThermo::THAIOmniThermo( const   TString&        strName
                                , const tCIDLib::TCard4 c4ItemNum) :

    THAIOmniItem(strName, tHAIOmniSh::EItem_Thermo, c4ItemNum)
    , m_c4FldId_CoolSP(kCIDLib::c4MaxCard)
    , m_c4FldId_CurTemp(kCIDLib::c4MaxCard)
    , m_c4FldId_FanMode(kCIDLib::c4MaxCard)
    , m_c4FldId_HeatSP(kCIDLib::c4MaxCard)
    , m_c4FldId_HoldStatus(kCIDLib::c4MaxCard)
    , m_c4FldId_Mode(kCIDLib::c4MaxCard)
{
}

THAIOmniThermo::THAIOmniThermo(const THAIOmniThermo& itemToCopy) :

    THAIOmniItem(itemToCopy)
    , m_c4FldId_CoolSP(itemToCopy.m_c4FldId_CoolSP)
    , m_c4FldId_CurTemp(itemToCopy.m_c4FldId_CurTemp)
    , m_c4FldId_FanMode(itemToCopy.m_c4FldId_FanMode)
    , m_c4FldId_HeatSP(itemToCopy.m_c4FldId_HeatSP)
    , m_c4FldId_HoldStatus(itemToCopy.m_c4FldId_HoldStatus)
    , m_c4FldId_Mode(itemToCopy.m_c4FldId_Mode)
{
}

THAIOmniThermo::~THAIOmniThermo()
{
}


// ---------------------------------------------------------------------------
//  THAIOmniThermo: Public operators
// ---------------------------------------------------------------------------
THAIOmniThermo&
THAIOmniThermo::operator=(const THAIOmniThermo& itemToAssign)
{
    if (this != &itemToAssign)
    {
        TParent::operator=(itemToAssign);
        m_c4FldId_CoolSP     = itemToAssign.m_c4FldId_CoolSP;
        m_c4FldId_CurTemp    = itemToAssign.m_c4FldId_CurTemp;
        m_c4FldId_FanMode    = itemToAssign.m_c4FldId_FanMode;
        m_c4FldId_HeatSP     = itemToAssign.m_c4FldId_HeatSP;
        m_c4FldId_HoldStatus = itemToAssign.m_c4FldId_HoldStatus;
        m_c4FldId_Mode       = itemToAssign.m_c4FldId_Mode;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  THAIOmniThermo: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
THAIOmniThermo::bOwnsField(const tCIDLib::TCard4 c4FldId) const
{
    return ((c4FldId == m_c4FldId_CoolSP)
         || (c4FldId == m_c4FldId_CurTemp)
         || (c4FldId == m_c4FldId_FanMode)
         || (c4FldId == m_c4FldId_HeatSP)
         || (c4FldId == m_c4FldId_HoldStatus)
         || (c4FldId == m_c4FldId_Mode));
}


// ---------------------------------------------------------------------------
//  THAIOmniThermo: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TCard4 THAIOmniThermo::c4FldIdCoolSP() const
{
    return m_c4FldId_CoolSP;
}


tCIDLib::TCard4 THAIOmniThermo::c4FldIdCurTemp() const
{
    return m_c4FldId_CurTemp;
}


tCIDLib::TCard4 THAIOmniThermo::c4FldIdFanMode() const
{
    return m_c4FldId_FanMode;
}


tCIDLib::TCard4 THAIOmniThermo::c4FldIdHeatSP() const
{
    return m_c4FldId_HeatSP;
}


tCIDLib::TCard4 THAIOmniThermo::c4FldIdHoldStatus() const
{
    return m_c4FldId_HoldStatus;
}


tCIDLib::TCard4 THAIOmniThermo::c4FldIdMode() const
{
    return m_c4FldId_Mode;
}


tCIDLib::TVoid
THAIOmniThermo::SetFieldIds(const   tCIDLib::TCard4 c4CoolSP
                            , const tCIDLib::TCard4 c4CurTemp
                            , const tCIDLib::TCard4 c4FanMode
                            , const tCIDLib::TCard4 c4HeatSP
                            , const tCIDLib::TCard4 c4HoldStatus
                            , const tCIDLib::TCard4 c4Mode)
{
    m_c4FldId_CoolSP     = c4CoolSP;
    m_c4FldId_CurTemp    = c4CurTemp;
    m_c4FldId_FanMode    = c4FanMode;
    m_c4FldId_HeatSP     = c4HeatSP;
    m_c4FldId_HoldStatus = c4HoldStatus;
    m_c4FldId_Mode       = c4Mode;
}



// ---------------------------------------------------------------------------
//   CLASS: THAIOmniUnit
//  PREFIX: item
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  THAIOmniUnit: Constructors and Destructor
// ---------------------------------------------------------------------------
THAIOmniUnit::THAIOmniUnit( const   TString&                strName
                            , const tCIDLib::TCard4         c4ItemNum
                            , const tHAIOmniSh::EUnitTypes  eType) :

    THAIOmniItem(strName, tHAIOmniSh::EItem_Unit, c4ItemNum)
    , m_c4FldId_Value(kCIDLib::c4MaxCard)
    , m_eType(eType)
{
}

THAIOmniUnit::THAIOmniUnit(const THAIOmniUnit& itemToCopy) :

    THAIOmniItem(itemToCopy)
    , m_c4FldId_Value(itemToCopy.m_c4FldId_Value)
    , m_eType(itemToCopy.m_eType)
{
}

THAIOmniUnit::~THAIOmniUnit()
{
}


// ---------------------------------------------------------------------------
//  THAIOmniUnit: Public operators
// ---------------------------------------------------------------------------
THAIOmniUnit& THAIOmniUnit::operator=(const THAIOmniUnit& itemToAssign)
{
    if (this != &itemToAssign)
    {
        TParent::operator=(itemToAssign);
        m_c4FldId_Value = itemToAssign.m_c4FldId_Value;
        m_eType         = itemToAssign.m_eType;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  THAIOmniUnit: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
THAIOmniUnit::bOwnsField(const tCIDLib::TCard4 c4FldId) const
{
    return (c4FldId == m_c4FldId_Value);
}


// ---------------------------------------------------------------------------
//  THAIOmniUnit: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TCard4 THAIOmniUnit::c4FldIdValue() const
{
    return m_c4FldId_Value;
}

tCIDLib::TCard4 THAIOmniUnit::c4FldIdValue(const tCIDLib::TCard4 c4ToSet)
{
    m_c4FldId_Value = c4ToSet;
    return m_c4FldId_Value;
}


tHAIOmniSh::EUnitTypes THAIOmniUnit::eType() const
{
    return m_eType;
}




// ---------------------------------------------------------------------------
//   CLASS: THAIOmniZone
//  PREFIX: item
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  THAIOmniZone: Constructors and Destructor
// ---------------------------------------------------------------------------
THAIOmniZone::THAIOmniZone( const   TString&                strName
                            , const tCIDLib::TCard4         c4ItemNum
                            , const tHAIOmniSh::EZoneTypes  eType) :

    THAIOmniItem(strName, tHAIOmniSh::EItem_Zone, c4ItemNum)
    , m_c4FldId_Analog(kCIDLib::c4MaxCard)
    , m_c4FldId_Arming(kCIDLib::c4MaxCard)
    , m_c4FldId_CurVal(kCIDLib::c4MaxCard)
    , m_c4FldId_HighSP(kCIDLib::c4MaxCard)
    , m_c4FldId_Latched(kCIDLib::c4MaxCard)
    , m_c4FldId_LowSP(kCIDLib::c4MaxCard)
    , m_c4FldId_Status(kCIDLib::c4MaxCard)
    , m_eType(eType)
{
}

THAIOmniZone::THAIOmniZone(const THAIOmniZone& itemToCopy) :

    THAIOmniItem(itemToCopy)
    , m_c4FldId_Analog(itemToCopy.m_c4FldId_Analog)
    , m_c4FldId_Arming(itemToCopy.m_c4FldId_Arming)
    , m_c4FldId_CurVal(itemToCopy.m_c4FldId_CurVal)
    , m_c4FldId_HighSP(itemToCopy.m_c4FldId_HighSP)
    , m_c4FldId_Latched(itemToCopy.m_c4FldId_Latched)
    , m_c4FldId_LowSP(itemToCopy.m_c4FldId_LowSP)
    , m_c4FldId_Status(itemToCopy.m_c4FldId_Status)
    , m_eType(itemToCopy.m_eType)
{
}

THAIOmniZone::~THAIOmniZone()
{
}


// ---------------------------------------------------------------------------
//  THAIOmniZone: Public operators
// ---------------------------------------------------------------------------
THAIOmniZone& THAIOmniZone::operator=(const THAIOmniZone& itemToAssign)
{
    if (this != &itemToAssign)
    {
        TParent::operator=(itemToAssign);
        m_c4FldId_Analog    = itemToAssign.m_c4FldId_Analog;
        m_c4FldId_Arming    = itemToAssign.m_c4FldId_Arming;
        m_c4FldId_CurVal    = itemToAssign.m_c4FldId_CurVal;
        m_c4FldId_HighSP    = itemToAssign.m_c4FldId_HighSP;
        m_c4FldId_Latched   = itemToAssign.m_c4FldId_Latched;
        m_c4FldId_LowSP     = itemToAssign.m_c4FldId_LowSP;
        m_c4FldId_Status    = itemToAssign.m_c4FldId_Status;
        m_eType             = itemToAssign.m_eType;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  THAIOmniZone: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
THAIOmniZone::bOwnsField(const tCIDLib::TCard4 c4FldId) const
{
    return ((c4FldId == m_c4FldId_Analog)
         || (c4FldId == m_c4FldId_Arming)
         || (c4FldId == m_c4FldId_CurVal)
         || (c4FldId == m_c4FldId_HighSP)
         || (c4FldId == m_c4FldId_Latched)
         || (c4FldId == m_c4FldId_LowSP)
         || (c4FldId == m_c4FldId_Status));
}


// ---------------------------------------------------------------------------
//  THAIOmniZone: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TCard4 THAIOmniZone::c4FldIdAnalog() const
{
    return m_c4FldId_Analog;
}


tCIDLib::TCard4 THAIOmniZone::c4FldIdArming() const
{
    return m_c4FldId_Arming;
}


tCIDLib::TCard4 THAIOmniZone::c4FldIdCurVal() const
{
    return m_c4FldId_CurVal;
}


tCIDLib::TCard4 THAIOmniZone::c4FldIdHighSP() const
{
    return m_c4FldId_HighSP;
}


tCIDLib::TCard4 THAIOmniZone::c4FldIdLatched() const
{
    return m_c4FldId_Latched;
}


tCIDLib::TCard4 THAIOmniZone::c4FldIdLowSP() const
{
    return m_c4FldId_LowSP;
}


tCIDLib::TCard4 THAIOmniZone::c4FldIdStatus() const
{
    return m_c4FldId_Status;
}


tHAIOmniSh::EZoneTypes THAIOmniZone::eZoneType() const
{
    return m_eType;
}


tCIDLib::TVoid
THAIOmniZone::SetFieldIds(  const   tCIDLib::TCard4 c4Analog
                            , const tCIDLib::TCard4 c4Arming
                            , const tCIDLib::TCard4 c4CurVal
                            , const tCIDLib::TCard4 c4High
                            , const tCIDLib::TCard4 c4Latched
                            , const tCIDLib::TCard4 c4Low
                            , const tCIDLib::TCard4 c4Status)
{
    m_c4FldId_Analog    = c4Analog;
    m_c4FldId_Arming    = c4Arming;
    m_c4FldId_CurVal    = c4CurVal;
    m_c4FldId_HighSP    = c4High;
    m_c4FldId_Latched   = c4Latched;
    m_c4FldId_LowSP     = c4Low;
    m_c4FldId_Status    = c4Status;
}

