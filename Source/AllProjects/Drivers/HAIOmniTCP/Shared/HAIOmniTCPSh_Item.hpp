//
// FILE NAME: HAIOmniTCPSh_Item.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/09/2013
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
//  This is the header for the HAIOmniTCPSh_Item.cpp file, which implements the
//  classes that represent configured items in the Omni. We have a base type
//  class, plus a derivative for each type of item we support. Some of the
//  data is purely runtime and is used on the server side to track the
//  configured items, and passed to the client side driver when it requests
//  the data. Some is configuration data streamed in and out at this shared
//  facility level.
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
class HAIOMNITCPSHEXPORT THAIOmniItem : public TObject, public MStreamable
{
    public :
        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static tCIDLib::TVoid QueryDefName
        (
            const   tHAIOmniTCPSh::EItemTypes eType
            , const tCIDLib::TCard4         c4ItemNum
            ,       TString&                strToFill
            , const tCIDLib::TBoolean       bBrackets = kCIDLib::False
        );


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        THAIOmniItem() = delete;

        virtual ~THAIOmniItem();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        tCIDLib::TBoolean operator==
        (
            const   THAIOmniItem&           itemSrc
        )   const;

        tCIDLib::TBoolean operator!=
        (
            const   THAIOmniItem&           itemSrc
        )   const;


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        virtual tCIDLib::TBoolean bCanSendTrig() const;

        virtual tCIDLib::TBoolean bCompare
        (
            const   THAIOmniItem&           itemSrc
        )   const;

        virtual tCIDLib::TBoolean bOwnsField
        (
            const   tCIDLib::TCard4         c4FldId
        )   const = 0;

        virtual tCIDLib::TCard4 c4QueryTypes
        (
                    tCIDLib::TStrCollect&   colToFill
            ,       tCIDLib::TCard4&        c4CurIndex
        );

        virtual THAIOmniItem* pitemDup() const = 0;

        virtual tCIDLib::TVoid QueryDisplayText
        (
                    TString&                strToFill
        )   const;

        virtual tCIDLib::TVoid SetType
        (
            const   TString&                strToSet
        );


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bSendTrig() const;

        tCIDLib::TBoolean bSendTrig
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TCard4 c4ItemNum() const;

        tHAIOmniTCPSh::EItemTypes eType() const;

        const TString& strName() const;

        const TString& strName
        (
            const   TString&                strName
        );

        tCIDLib::TVoid SetDefName();


    protected :
        // -------------------------------------------------------------------
        //  Hidden constructors and operators
        // -------------------------------------------------------------------
        THAIOmniItem
        (
            const   tHAIOmniTCPSh::EItemTypes eType
        );

        THAIOmniItem
        (
            const   TString&                strName
            , const tHAIOmniTCPSh::EItemTypes eType
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


        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        );

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const;


        // -------------------------------------------------------------------
        //  Protected, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid SetName
        (
            const   TString&                strToSet
        );


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        //
        //  m_bSendTrig
        //      Controls whether we send a trigger for this item or not,
        //      assuming one would be sent at all for the derived type. This
        //      is persisted.
        //
        //  m_c4ItemNum
        //      The item number for this item. It is a unique id within the
        //      list of items of the same type. This is persisted.
        //
        //  m_eType
        //      The type of item this object represents. This is runtime only
        //      and is set by the derived class ctor.
        //
        //  m_strName
        //      The name given to the item. These are either the ones we generated
        //      initially, or set by the user.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean           m_bSendTrig;
        tCIDLib::TCard4             m_c4ItemNum;
        tHAIOmniTCPSh::EItemTypes   m_eType;
        TString                     m_strName;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(THAIOmniItem,TObject)
};


// ---------------------------------------------------------------------------
//   CLASS: THAIOmniArea
//  PREFIX: item
// ---------------------------------------------------------------------------
class HAIOMNITCPSHEXPORT THAIOmniArea : public THAIOmniItem
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        THAIOmniArea();

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

        tCIDLib::TBoolean operator==
        (
            const   THAIOmniArea&           itemSrc
        )   const;

        tCIDLib::TBoolean operator!=
        (
            const   THAIOmniArea&           itemSrc
        )   const;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCompare
        (
            const   THAIOmniItem&           itemSrc
        )   const;

        tCIDLib::TBoolean bOwnsField
        (
            const   tCIDLib::TCard4         c4FldId
        )   const;

        THAIOmniItem* pitemDup() const;


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


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        );

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4FldId_XXX
        //      The ids of the fields associated with this zone.
        // -------------------------------------------------------------------
        tCIDLib::TCard4 m_c4FldId_Alarmed;
        tCIDLib::TCard4 m_c4FldId_AlarmBits;
        tCIDLib::TCard4 m_c4FldId_AlarmList;
        tCIDLib::TCard4 m_c4FldId_ArmStatus;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(THAIOmniArea,THAIOmniItem)
};



// ---------------------------------------------------------------------------
//   CLASS: THAIOmniThermo
//  PREFIX: item
// ---------------------------------------------------------------------------
class HAIOMNITCPSHEXPORT THAIOmniThermo : public THAIOmniItem
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        THAIOmniThermo();

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

        tCIDLib::TBoolean operator==
        (
            const   THAIOmniThermo&         itemSrc
        )   const;

        tCIDLib::TBoolean operator!=
        (
            const   THAIOmniThermo&         itemSrc
        )   const;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCompare
        (
            const   THAIOmniItem&           itemSrc
        )   const;

        tCIDLib::TBoolean bOwnsField
        (
            const   tCIDLib::TCard4         c4FldId
        )   const;

        THAIOmniItem* pitemDup() const;


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


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        );

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const;


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
class HAIOMNITCPSHEXPORT THAIOmniUnit : public THAIOmniItem
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        THAIOmniUnit();

        THAIOmniUnit
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4ItemNum
            , const tHAIOmniTCPSh::EUnitTypes eType
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

        tCIDLib::TBoolean operator==
        (
            const   THAIOmniUnit&           itemSrc
        )   const;

        tCIDLib::TBoolean operator!=
        (
            const   THAIOmniUnit&           itemSrc
        )   const;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCanSendTrig() const;

        tCIDLib::TBoolean bCompare
        (
            const   THAIOmniItem&           itemSrc
        )   const;

        tCIDLib::TBoolean bOwnsField
        (
            const   tCIDLib::TCard4         c4FldId
        )   const;

        tCIDLib::TCard4 c4QueryTypes
        (
                    tCIDLib::TStrCollect&   colToFill
            ,       tCIDLib::TCard4&        c4CurIndex
        );

        THAIOmniItem* pitemDup() const;

        tCIDLib::TVoid QueryDisplayText
        (
                    TString&                strToFill
        )   const;

        tCIDLib::TVoid SetType
        (
            const   TString&                strToSet
        );


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TCard4 c4FldIdValue() const;

        tCIDLib::TCard4 c4FldIdValue
        (
            const   tCIDLib::TCard4         c4ToSet
        );

        tHAIOmniTCPSh::EUnitTypes eUnitType() const;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        );

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4FldId_XXX
        //      The ids of the fields associated with this zone.
        //
        //  m_eUnitType
        //      The type of unit, i.e. dimmer or binary.
        // -------------------------------------------------------------------
        tCIDLib::TCard4             m_c4FldId_Value;
        tHAIOmniTCPSh::EUnitTypes   m_eUnitType;
        const TString               m_strType_Binary;
        const TString               m_strType_Dimmer;
        const TString               m_strType_Flag;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(THAIOmniUnit,THAIOmniItem)
};



// ---------------------------------------------------------------------------
//   CLASS: THAIOmniZone
//  PREFIX: item
// ---------------------------------------------------------------------------
class HAIOMNITCPSHEXPORT THAIOmniZone : public THAIOmniItem
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        THAIOmniZone();

        THAIOmniZone
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4ItemNum
            , const tHAIOmniTCPSh::EZoneTypes eType
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

        tCIDLib::TBoolean operator==
        (
            const   THAIOmniZone&           itemSrc
        )   const;

        tCIDLib::TBoolean operator!=
        (
            const   THAIOmniZone&           itemSrc
        )   const;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCanSendTrig() const;

        tCIDLib::TBoolean bCompare
        (
            const   THAIOmniItem&           itemSrc
        )   const;

        tCIDLib::TBoolean bOwnsField
        (
            const   tCIDLib::TCard4         c4FldId
        )   const;

        tCIDLib::TCard4 c4QueryTypes
        (
                    tCIDLib::TStrCollect&   colToFill
            ,       tCIDLib::TCard4&        c4CurIndex
        );

        THAIOmniItem* pitemDup() const;

        tCIDLib::TVoid QueryDisplayText
        (
                    TString&                strToFill
        )   const;

        tCIDLib::TVoid SetType
        (
            const   TString&                strToSet
        );


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

        tHAIOmniTCPSh::EZoneTypes eZoneType() const;

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


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        );

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4FldId_XXX
        //      The ids of the fields associated with this zone.
        //
        //  m_eZoneType
        //      The zone type. Which field ids are used depends on this type.
        //      Also, this plus the send trigger flag in the base class
        //      controls whether we send an event trigger when the status
        //      changes, and what type if we do.
        //
        //      The Status field is used for all types. For alarm zones this
        //      is the alarm status, for motion zones it indicates the motion
        //      start/stop status.
        //
        //      Alarm -
        //      The Analog, Arming, Latched are created if it's alarm zone.
        //
        //      Temp/Hum -
        //      If the zone type is temp or humidity, then the CurVal, Low,
        //      and High fields are used, else they are not created or polled.
        //
        //      Temp -
        //      When it's a temp zone, Cur/High/Low are temps in the C/F format
        //      set up for the driver instance. When a humidity zone, they
        //      are percents.
        //
        //      Motion -
        //      The status field is all that's used, and it's only there in
        //      order to have a semantically marked field. So that we can
        //      easily get a list of all fields that are motion sensors, using
        //      standard sementic field type queries. We don't try to poll
        //      the sensors since they are battery powered, so the field will
        //      only have a valid value once we get an async report for it.
        // -------------------------------------------------------------------
        tCIDLib::TCard4             m_c4FldId_Analog;
        tCIDLib::TCard4             m_c4FldId_Arming;
        tCIDLib::TCard4             m_c4FldId_CurVal;
        tCIDLib::TCard4             m_c4FldId_HighSP;
        tCIDLib::TCard4             m_c4FldId_Latched;
        tCIDLib::TCard4             m_c4FldId_LowSP;
        tCIDLib::TCard4             m_c4FldId_Status;
        tHAIOmniTCPSh::EZoneTypes   m_eZoneType;
        const TString               m_strType_Alarm;
        const TString               m_strType_Humidity;
        const TString               m_strType_Motion;
        const TString               m_strType_Temp;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(THAIOmniZone,THAIOmniItem)
};

#pragma CIDLIB_POPPACK

