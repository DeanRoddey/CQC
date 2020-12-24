//
// FILE NAME: HAIOmniS_Item.hpp
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
//  This is the header for the HAIOmniS_Item.cpp file, which implements the
//  classes that represent configured items in the Omni. For each one we need
//  to keep up with the name of the item, it's item number (unique within the
//  type of item), the type of the item, the field ids for the fields that
//  represent that item and any item type specific info that might be needed.
//
//  The last two are unique to each type of item, so we create a base class
//  and then derive a class from it for each type of item. The item type is
//  implicit in the type, but we also store a type enum in the base class, to
//  allow for quick deterimation of item type via the base class.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: THAIOmniItem
//  PREFIX: item
// ---------------------------------------------------------------------------
class THAIOmniItem : public TObject
{
    public :
        // -------------------------------------------------------------------
        //  Destructor
        // -------------------------------------------------------------------
        virtual ~THAIOmniItem();


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        virtual tCIDLib::TBoolean bOwnsField
        (
            const   tCIDLib::TCard4         c4FldId
        )   const = 0;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TCard4 c4ItemNum() const;

        tHAIOmniSh::EItemTypes eType() const;

        const TString& strName() const;


    protected :
        // -------------------------------------------------------------------
        //  Hidden constructors and operators
        // -------------------------------------------------------------------
        THAIOmniItem
        (
            const   TString&                strName
            , const tHAIOmniSh::EItemTypes  eType
            , const tCIDLib::TCard4         c4ItemNum
        );

        THAIOmniItem
        (
            const   THAIOmniItem&           itemToCopy
        );

        THAIOmniItem& operator=
        (
            const   THAIOmniItem&           itemToAssign
        );


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        //
        //  m_c4ItemNum
        //      The item number for this item. It is a unique id within the
        //      list of items of the same type.
        //
        //  m_eType
        //      The type of item this object represents.
        //
        //  m_strName
        //      The name given to the item. These are our own generated names,
        //      are in the form Area1, Thermo02, etc...
        // -------------------------------------------------------------------
        tCIDLib::TCard4         m_c4ItemNum;
        tHAIOmniSh::EItemTypes  m_eType;
        TString                 m_strName;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(THAIOmniItem,TObject)
};


// ---------------------------------------------------------------------------
//   CLASS: THAIOmniArea
//  PREFIX: item
// ---------------------------------------------------------------------------
class THAIOmniArea : public THAIOmniItem
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        THAIOmniArea
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4ItemNum
        );

        THAIOmniArea
        (
            const   THAIOmniArea&           itemToCopy
        );

        ~THAIOmniArea();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        THAIOmniArea& operator=
        (
            const   THAIOmniArea&           itemToAssign
        );


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bOwnsField
        (
            const   tCIDLib::TCard4         c4FldId
        )   const;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TCard4 c4FldIdAlarmed() const;

        tCIDLib::TCard4 c4FldIdAlarmed
        (
            const   tCIDLib::TCard4         c4ToSet
        );

        tCIDLib::TCard4 c4FldIdAlarmBits() const;

        tCIDLib::TCard4 c4FldIdAlarmBits
        (
            const   tCIDLib::TCard4         c4ToSet
        );

        tCIDLib::TCard4 c4FldIdAlarmList() const;

        tCIDLib::TCard4 c4FldIdAlarmList
        (
            const   tCIDLib::TCard4         c4ToSet
        );

        tCIDLib::TCard4 c4FldIdArmStatus() const;

        tCIDLib::TCard4 c4FldIdArmStatus
        (
            const   tCIDLib::TCard4         c4ToSet
        );

        tCIDLib::TCard4 c4FldIdInDelay() const;

        tCIDLib::TCard4 c4FldIdInDelay
        (
            const   tCIDLib::TCard4         c4ToSet
        );


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4FldId_XXX
        //      The ids of the fields associated with this zone. These fields
        //      represent:
        //
        //      Alarmed   - Whether we are currently alarmed or not in this.
        //      AlarmBits - The raw alarm status bits if they want to use them.
        //      AlarmList - A human readable list of alarm values that the bits
        //                  represent.
        //      ArmStatus - The current armed/arming status
        //      InDelay   - True if we are currently in an arming countdown
        //                  delay.
        // -------------------------------------------------------------------
        tCIDLib::TCard4 m_c4FldId_Alarmed;
        tCIDLib::TCard4 m_c4FldId_AlarmBits;
        tCIDLib::TCard4 m_c4FldId_AlarmList;
        tCIDLib::TCard4 m_c4FldId_ArmStatus;
        tCIDLib::TCard4 m_c4FldId_InDelay;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(THAIOmniArea,THAIOmniItem)
};



// ---------------------------------------------------------------------------
//   CLASS: THAIOmniThermo
//  PREFIX: item
// ---------------------------------------------------------------------------
class THAIOmniThermo : public THAIOmniItem
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        THAIOmniThermo
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4ItemNum
        );

        THAIOmniThermo
        (
            const   THAIOmniThermo&         itemToCopy
        );

        ~THAIOmniThermo();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        THAIOmniThermo& operator=
        (
            const   THAIOmniThermo&         itemToAssign
        );


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bOwnsField
        (
            const   tCIDLib::TCard4         c4FldId
        )   const;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TCard4 c4FldIdCoolSP() const;

        tCIDLib::TCard4 c4FldIdCurTemp() const;

        tCIDLib::TCard4 c4FldIdFanMode() const;

        tCIDLib::TCard4 c4FldIdHeatSP() const;

        tCIDLib::TCard4 c4FldIdHoldStatus() const;

        tCIDLib::TCard4 c4FldIdMode() const;

        tCIDLib::TVoid SetFieldIds
        (
            const   tCIDLib::TCard4         c4CoolSP
            , const tCIDLib::TCard4         c4CurTemp
            , const tCIDLib::TCard4         c4FanMode
            , const tCIDLib::TCard4         c4HeatSP
            , const tCIDLib::TCard4         c4HoldStatus
            , const tCIDLib::TCard4         c4Mode
        );


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4FldId_XXX
        //      The ids of the fields associated with this thermo.
        // -------------------------------------------------------------------
        tCIDLib::TCard4     m_c4FldId_CoolSP;
        tCIDLib::TCard4     m_c4FldId_CurTemp;
        tCIDLib::TCard4     m_c4FldId_FanMode;
        tCIDLib::TCard4     m_c4FldId_HeatSP;
        tCIDLib::TCard4     m_c4FldId_HoldStatus;
        tCIDLib::TCard4     m_c4FldId_Mode;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(THAIOmniThermo,THAIOmniItem)
};



// ---------------------------------------------------------------------------
//   CLASS: THAIOmniUnit
//  PREFIX: item
// ---------------------------------------------------------------------------
class THAIOmniUnit : public THAIOmniItem
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        THAIOmniUnit
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4ItemNum
            , const tHAIOmniSh::EUnitTypes  eType
        );

        THAIOmniUnit
        (
            const   THAIOmniUnit&           itemToCopy
        );

        ~THAIOmniUnit();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        THAIOmniUnit& operator=
        (
            const   THAIOmniUnit&           itemToAssign
        );


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bOwnsField
        (
            const   tCIDLib::TCard4         c4FldId
        )   const;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TCard4 c4FldIdValue() const;

        tCIDLib::TCard4 c4FldIdValue
        (
            const   tCIDLib::TCard4         c4ToSet
        );

        tHAIOmniSh::EUnitTypes eType() const;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4FldId_XXX
        //      The ids of the fields associated with this zone.
        //
        //  m_eType
        //      The type of unit, i.e. dimmer or binary.
        // -------------------------------------------------------------------
        tCIDLib::TCard4         m_c4FldId_Value;
        tHAIOmniSh::EUnitTypes  m_eType;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(THAIOmniUnit,THAIOmniItem)
};



// ---------------------------------------------------------------------------
//   CLASS: THAIOmniZone
//  PREFIX: item
// ---------------------------------------------------------------------------
class THAIOmniZone : public THAIOmniItem
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        THAIOmniZone
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4ItemNum
            , const tHAIOmniSh::EZoneTypes  eType
        );

        THAIOmniZone
        (
            const   THAIOmniZone&           itemToCopy
        );

        ~THAIOmniZone();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        THAIOmniZone& operator=
        (
            const   THAIOmniZone&           itemToAssign
        );


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bOwnsField
        (
            const   tCIDLib::TCard4         c4FldId
        )   const;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TCard4 c4FldIdAnalog() const;

        tCIDLib::TCard4 c4FldIdArming() const;

        tCIDLib::TCard4 c4FldIdCurVal() const;

        tCIDLib::TCard4 c4FldIdHighSP() const;

        tCIDLib::TCard4 c4FldIdLatched() const;

        tCIDLib::TCard4 c4FldIdLowSP() const;

        tCIDLib::TCard4 c4FldIdStatus() const;

        tHAIOmniSh::EZoneTypes eZoneType() const;

        tCIDLib::TVoid SetFieldIds
        (
            const   tCIDLib::TCard4         c4Analog
            , const tCIDLib::TCard4         c4Arming
            , const tCIDLib::TCard4         c4CurVal
            , const tCIDLib::TCard4         c4HighSP
            , const tCIDLib::TCard4         c4Latched
            , const tCIDLib::TCard4         c4LowSp
            , const tCIDLib::TCard4         c4Status
        );


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4FldId_XXX
        //      The ids of the fields associated with this zone.
        //
        //  m_eType
        //      The zone type. Which field ids are used depends on this type.
        //      The Analog, Arming, Latched, and Status fields are always
        //      used, even though they may just indicate default info when
        //      the zone is not an alarm zone. If the zone type is temp or
        //      humidity, then the CurVal, Low, and High fields are used,
        //      else they are not created or polled.
        //
        //      When it's a temp zone, Cur/High/Low are temps in the C/F format
        //      set up for the driver instance. When a humidity zone, they
        //      are percents.
        // -------------------------------------------------------------------
        tCIDLib::TCard4         m_c4FldId_Analog;
        tCIDLib::TCard4         m_c4FldId_Arming;
        tCIDLib::TCard4         m_c4FldId_CurVal;
        tCIDLib::TCard4         m_c4FldId_HighSP;
        tCIDLib::TCard4         m_c4FldId_Latched;
        tCIDLib::TCard4         m_c4FldId_LowSP;
        tCIDLib::TCard4         m_c4FldId_Status;
        tHAIOmniSh::EZoneTypes  m_eType;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(THAIOmniZone,THAIOmniItem)
};

#pragma CIDLIB_POPPACK

