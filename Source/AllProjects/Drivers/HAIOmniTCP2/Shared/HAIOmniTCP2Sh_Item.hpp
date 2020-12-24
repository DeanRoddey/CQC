//
// FILE NAME: HAIOmniTCP2Sh_Item.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/06/2014
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
//  This is the header for the HAIOmniTCP2Sh_Item.cpp file, which implements the
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
class HAIOMNITCP2SHEXP THAIOmniItem

    : public TObject, public MStreamable, public MDuplicable
{
    public :
        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static tCIDLib::TVoid QueryDefName
        (
            const   tHAIOmniTCP2Sh::EItemTypes eType
            , const tCIDLib::TCard2         c2ItemNum
            ,       TString&                strName
            , const tCIDLib::TBoolean       bBrackets = kCIDLib::False
        );


        // -------------------------------------------------------------------
        //  Destructor
        // -------------------------------------------------------------------
        virtual ~THAIOmniItem();


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        virtual tCIDLib::TBoolean bCanSendTrig() const;

        virtual tCIDLib::TBoolean bCompare
        (
            const   THAIOmniItem&           itemSrc
        )   const;

        virtual tCIDLib::TBoolean bHasSubType
        (
                    TString&                strToFill
        )   const;

        virtual tCIDLib::TBoolean bOwnsField
        (
            const   tCIDLib::TCard4         c4FldId
        )   const = 0;

        virtual tCIDLib::TCard4 c4QueryFldDefs
        (
                    tCQCKit::TFldDefList&   colAddTo
            ,       TCQCFldDef&             flddTmp
            , const tCIDLib::TBoolean       bCelsius
        )   const = 0;

        virtual tCIDLib::TCard4 c4QueryTypes
        (
                    tCIDLib::TStrCollect&   colToFill
            ,       tCIDLib::TCard4&        c4CurIndex
        );

        virtual tCIDLib::TVoid Finalize
        (
                    TCQCServerBase&         sdrvOwn
        );

        virtual THAIOmniItem* pitemDup() const = 0;

        virtual tCIDLib::TVoid QueryFirstLastId
        (
                    tCIDLib::TCard4&        c4First
            ,       tCIDLib::TCard4&        c4Last
        )   const = 0;

        virtual tCIDLib::TVoid SetSubType
        (
            const   TString&                strToSet
        );

        virtual tCIDLib::TVoid StoreFldIds
        (
            const   TCQCServerBase&         sdrvOwner
        ) = 0;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bSendTrig() const;

        tCIDLib::TBoolean bSendTrig
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TCard2 c2ItemNum() const;

        tHAIOmniTCP2Sh::EItemTypes eType() const;

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
            const   tHAIOmniTCP2Sh::EItemTypes eType
        );

        THAIOmniItem
        (
            const   TString&                strName
            , const tHAIOmniTCP2Sh::EItemTypes eType
            , const tCIDLib::TCard2         c2ItemNum
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
        )   override;

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const override;


        // -------------------------------------------------------------------
        //  Protected, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid SetName
        (
            const   TString&                strToSet
        );


    private :
        // -------------------------------------------------------------------
        //  Unimplemented
        // -------------------------------------------------------------------
        THAIOmniItem();


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        //
        //  m_bSendTrig
        //      Controls whether we send a trigger for this item or not,
        //      assuming one would be sent at all for the derived type. This
        //      is persisted.
        //
        //  m_c2ItemNum
        //      The item number for this item. It is a unique id within the
        //      list of items of the same type. This is persisted.
        //
        //  m_eType
        //      The type of item this object represents. This is runtime only
        //      and is set by the derived class ctor.
        //
        //  m_strName
        //      The name given to the item. Initially it will be either something
        //      read from the Omni or that we default.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean           m_bSendTrig;
        tCIDLib::TCard2             m_c2ItemNum;
        tHAIOmniTCP2Sh::EItemTypes  m_eType;
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
class HAIOMNITCP2SHEXP THAIOmniArea : public THAIOmniItem
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        THAIOmniArea();

        THAIOmniArea
        (
            const   TString&                strName
            , const tCIDLib::TCard2         c2ItemNum
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
        tCIDLib::TBoolean bCompare
        (
            const   THAIOmniItem&           itemSrc
        )   const override;

        tCIDLib::TBoolean bOwnsField
        (
            const   tCIDLib::TCard4         c4FldId
        )   const override;

        tCIDLib::TCard4 c4QueryFldDefs
        (
                    tCQCKit::TFldDefList&   colAddTo
            ,       TCQCFldDef&             flddTmp
            , const tCIDLib::TBoolean       bCelsius
        )   const override;

        THAIOmniItem* pitemDup() const override;

        tCIDLib::TVoid QueryFirstLastId
        (
                    tCIDLib::TCard4&        c4First
            ,       tCIDLib::TCard4&        c4Last
        )   const override;

        tCIDLib::TVoid StoreFldIds
        (
            const   TCQCServerBase&         sdrvOwner
        )   override;

        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TCard4 c4FldIdInAlarm() const;

        tCIDLib::TCard4 c4FldIdAlarms() const;

        tCIDLib::TCard4 c4FldIdArming() const;

        tCIDLib::TCard4 c4FldIdArmMode() const;



    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        )   override;

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const override;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4FldId_XXX
        //      The ids of the fields associated with this zone.
        // -------------------------------------------------------------------
        tCIDLib::TCard4 m_c4FldId_Alarms;
        tCIDLib::TCard4 m_c4FldId_Arming;
        tCIDLib::TCard4 m_c4FldId_ArmMode;
        tCIDLib::TCard4 m_c4FldId_InAlarm;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(THAIOmniArea,THAIOmniItem)
        DefPolyDup(THAIOmniArea)
};



// ---------------------------------------------------------------------------
//   CLASS: THAIOmniThermo
//  PREFIX: item
// ---------------------------------------------------------------------------
class HAIOMNITCP2SHEXP THAIOmniThermo : public THAIOmniItem
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        THAIOmniThermo();

        THAIOmniThermo
        (
            const   TString&                strName
            , const tCIDLib::TCard2         c2ItemNum
            , const tHAIOmniTCP2Sh::EThermoTypes eType
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
        tCIDLib::TBoolean bCompare
        (
            const   THAIOmniItem&           itemSrc
        )   const override;

        tCIDLib::TBoolean bOwnsField
        (
            const   tCIDLib::TCard4         c4FldId
        )   const override;

        tCIDLib::TCard4 c4QueryFldDefs
        (
                    tCQCKit::TFldDefList&   colAddTo
            ,       TCQCFldDef&             flddTmp
            , const tCIDLib::TBoolean       bCelsius
        )   const override;

        tCIDLib::TVoid Finalize
        (
                    TCQCServerBase&         sdrvOwn
        )   override;

        THAIOmniItem* pitemDup() const override;

        tCIDLib::TVoid QueryFirstLastId
        (
                    tCIDLib::TCard4&        c4First
            ,       tCIDLib::TCard4&        c4Last
        )   const override;

        tCIDLib::TVoid StoreFldIds
        (
            const   TCQCServerBase&         sdrvOwner
        )   override;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TCard4 c4FldIdCurTemp() const;

        tCIDLib::TCard4 c4FldIdFanMode() const;

        tCIDLib::TCard4 c4FldIdFanOpMode() const;

        tCIDLib::TCard4 c4FldIdHighSP() const;

        tCIDLib::TCard4 c4FldIdHoldStatus() const;

        tCIDLib::TCard4 c4FldIdMode() const;

        tCIDLib::TCard4 c4FldIdOpMode() const;

        tCIDLib::TCard4 c4FldIdLowSP() const;

        tHAIOmniTCP2Sh::EThermoTypes eThermoType() const;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        )   override;

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const override;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bCelcius
        //      The driver tells us if we are C or F range. It's used during
        //      field def setup.
        //
        //  m_c4FldId_XXX
        //      The ids of the fields associated with this thermo.
        // -------------------------------------------------------------------
        tCIDLib::TCard4             m_c4FldId_CurTemp;
        tCIDLib::TCard4             m_c4FldId_FanMode;
        tCIDLib::TCard4             m_c4FldId_FanOpMode;
        tCIDLib::TCard4             m_c4FldId_HighSP;
        tCIDLib::TCard4             m_c4FldId_HoldStatus;
        tCIDLib::TCard4             m_c4FldId_LowSP;
        tCIDLib::TCard4             m_c4FldId_Mode;
        tCIDLib::TCard4             m_c4FldId_OpMode;
        tHAIOmniTCP2Sh::EThermoTypes m_eThermoType;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(THAIOmniThermo,THAIOmniItem)
        DefPolyDup(THAIOmniThermo)
};



// ---------------------------------------------------------------------------
//   CLASS: THAIOmniUnit
//  PREFIX: item
// ---------------------------------------------------------------------------
class HAIOMNITCP2SHEXP THAIOmniUnit : public THAIOmniItem
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        THAIOmniUnit();

        THAIOmniUnit
        (
            const   TString&                strName
            , const tCIDLib::TCard2         c2ItemNum
            , const tHAIOmniTCP2Sh::EUnitTypes eType
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
        tCIDLib::TBoolean bCanSendTrig() const override;

        tCIDLib::TBoolean bCompare
        (
            const   THAIOmniItem&           itemSrc
        )   const override;

        tCIDLib::TBoolean bHasSubType
        (
                    TString&                strToFill
        )   const override;

        tCIDLib::TBoolean bOwnsField
        (
            const   tCIDLib::TCard4         c4FldId
        )   const override;

        tCIDLib::TCard4 c4QueryFldDefs
        (
                    tCQCKit::TFldDefList&   colAddTo
            ,       TCQCFldDef&             flddTmp
            , const tCIDLib::TBoolean       bCelsius
        )   const override;

        tCIDLib::TCard4 c4QueryTypes
        (
                    tCIDLib::TStrCollect&   colToFill
            ,       tCIDLib::TCard4&        c4CurIndex
        )   override;

        THAIOmniItem* pitemDup() const override;

        tCIDLib::TVoid QueryFirstLastId
        (
                    tCIDLib::TCard4&        c4First
            ,       tCIDLib::TCard4&        c4Last
        )   const override;

        tCIDLib::TVoid SetSubType
        (
            const   TString&                strToSet
        )   override;

        tCIDLib::TVoid StoreFldIds
        (
            const   TCQCServerBase&         sdrvOwner
        )   override;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TCard4 c4FldIdValue() const;

        tHAIOmniTCP2Sh::EUnitTypes eUnitType() const;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        )   override;

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const override;


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
        tHAIOmniTCP2Sh::EUnitTypes  m_eUnitType;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(THAIOmniUnit,THAIOmniItem)
        DefPolyDup(THAIOmniUnit)
};



// ---------------------------------------------------------------------------
//   CLASS: THAIOmniZone
//  PREFIX: item
// ---------------------------------------------------------------------------
class HAIOMNITCP2SHEXP THAIOmniZone : public THAIOmniItem
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        THAIOmniZone();

        THAIOmniZone
        (
            const   TString&                strName
            , const tCIDLib::TCard2         c2ItemNum
            , const tHAIOmniTCP2Sh::EZoneTypes eType
            , const tCIDLib::TCard4         c4AreaNum
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
        tCIDLib::TBoolean bCanSendTrig() const override;

        tCIDLib::TBoolean bCompare
        (
            const   THAIOmniItem&           itemSrc
        )   const override;

        tCIDLib::TBoolean bHasSubType
        (
                    TString&                strToFill
        )   const override;

        tCIDLib::TBoolean bOwnsField
        (
            const   tCIDLib::TCard4         c4FldId
        )   const override;

        tCIDLib::TCard4 c4QueryFldDefs
        (
                    tCQCKit::TFldDefList&   colAddTo
            ,       TCQCFldDef&             flddTmp
            , const tCIDLib::TBoolean       bCelsius
        )   const override;

        tCIDLib::TCard4 c4QueryTypes
        (
                    tCIDLib::TStrCollect&   colToFill
            ,       tCIDLib::TCard4&        c4CurIndex
        )   override;

        THAIOmniItem* pitemDup() const override;

        tCIDLib::TVoid QueryFirstLastId
        (
                    tCIDLib::TCard4&        c4First
            ,       tCIDLib::TCard4&        c4Last
        )   const override;

        tCIDLib::TVoid SetSubType
        (
            const   TString&                strToSet
        )   override;

        tCIDLib::TVoid StoreFldIds
        (
            const   TCQCServerBase&         sdrvOwner
        )   override;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bAlarmType() const;

        tCIDLib::TCard4 c4AreaNum() const;

        tCIDLib::TCard4 c4AreaNum
        (
            const   tCIDLib::TCard4         c4ToSet
        );

        tCIDLib::TCard4 c4FldIdAnalog() const;

        tCIDLib::TCard4 c4FldIdArming() const;

        tCIDLib::TCard4 c4FldIdCurVal() const;

        tCIDLib::TCard4 c4FldIdHighSP() const;

        tCIDLib::TCard4 c4FldIdLatched() const;

        tCIDLib::TCard4 c4FldIdLowSP() const;

        tCIDLib::TCard4 c4FldIdStatus() const;

        tHAIOmniTCP2Sh::EZoneTypes eZoneType() const;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        )   override;

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const override;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4AreaNum
        //      The area that this zone belongs to.
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
        tCIDLib::TCard4             m_c4AreaNum;
        tCIDLib::TCard4             m_c4FldId_Analog;
        tCIDLib::TCard4             m_c4FldId_Arming;
        tCIDLib::TCard4             m_c4FldId_CurVal;
        tCIDLib::TCard4             m_c4FldId_HighSP;
        tCIDLib::TCard4             m_c4FldId_Latched;
        tCIDLib::TCard4             m_c4FldId_LowSP;
        tCIDLib::TCard4             m_c4FldId_Status;
        tHAIOmniTCP2Sh::EZoneTypes  m_eZoneType;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(THAIOmniZone,THAIOmniItem)
        DefPolyDup(THAIOmniZone)
};

#pragma CIDLIB_POPPACK

