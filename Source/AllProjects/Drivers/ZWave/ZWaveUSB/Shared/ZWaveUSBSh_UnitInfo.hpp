//
// FILE NAME: ZWaveUSBSh_UnitInfo.hpp
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
//  This is the header file for the ZWaveUSBS_UnitInfo.cpp file, which
//  implements the TZWaveUnit class. This abstract base class represents
//  the basic information that we keep up with for all units. There are
//  derivatives of this class that handle dealing with specific types of
//  units.
//
//  Each of the drived classes will set a generic and specific type id on
//  the base unit class. These are the class types that the Z-Wave network
//  uses. The base class also stores the unit id and the capabilities flags
//  for the unit. We don't expose the caps flags but just provide a set of
//  boolean getter methods that check the specific bits.
//
//  We also keep up with some runtime info such as the last time the unit
//  was polled, how many failed polls since the last good one, and how many
//  seconds it should be polled (so that more important ones can be polled
//  more often.)
//
//  Each derivative will cause the creation of one or more driver
//  fields. The derived classes must tell the driver what fields it needs
//  created. The driver will give him back the ids for those fields, which he
//  must store and later when he is asked to poll his device (or an async
//  report comes in for him), he must update those fields.
//
//  These must be streamable, so that they can be stored to the Z-Wave driver
//  configuration and so that they can be streamed to the client for display
//  and configuration purposes.
//
// CAVEATS/GOTCHAS:
//
//  1)  We can only update fields during the HandleAppCmd() method, because
//      that is the only one where the driver does a field lock before he
//      calls us. Though, we can store an initial value during the bStoreFldIds
//      call.
//
//  2)  Unless we know a type of unit cannot be polled, we'll initially set
//      the pollable flag. Later, during the network scan, if we get node
//      capabilities flags that indicate it's non-listening, we'll turn the
//      polling flag off, since such nodes will need to use async notifications
//      to work.
//
//  3)  We only need equality testing at the base unit level since the derived
//      classes only define field id members, which are runtime only.
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TZWaveUnit
//  PREFIX: unit
// ---------------------------------------------------------------------------
class ZWUSBSHEXPORT TZWaveUnit

    : public TObject, public MStreamable, public MDuplicable
{
    public :
        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static tCIDLib::ESortComps eCompById
        (
            const   TZWaveUnit&             unit1
            , const TZWaveUnit&             unit2
            , const tCIDLib::TCard4         c4UserData
        );

        static tCIDLib::ESortComps eCompByName
        (
            const   TZWaveUnit&             unit1
            , const TZWaveUnit&             unit2
            , const tCIDLib::TCard4         c4UserData
        );

        static TZWaveUnit* punitBestMatch
        (
            const   tCIDLib::TCard1         c1BaseType
            , const tCIDLib::TCard1         c1GenType
            , const tCIDLib::TCard1         c1SpecType
        );


        // -------------------------------------------------------------------
        //  Destructor
        // -------------------------------------------------------------------
        ~TZWaveUnit();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        tCIDLib::TBoolean operator==
        (
            const   TZWaveUnit&             unitSrc
        )   const;

        tCIDLib::TBoolean operator!=
        (
            const   TZWaveUnit&             unitSrc
        )   const;



        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        virtual tCIDLib::TBoolean bBuildGetMsg
        (
                    TMemBuf&                mbufTar
            ,       tCIDLib::TCard1&        c1RetClass
            ,       tCIDLib::TCard1&        c1RetCmd
            ,       tCIDLib::TCard1&        c1RetLen
            , const tCIDLib::TCard1         c1Val1
            , const tCIDLib::TCard1         c1Val2
            , const tCIDLib::TCard1         c1CBId
            , const tZWaveUSBSh::EUGetCmds  eCmd
        )   const;

        virtual tCIDLib::TBoolean bBuildSetMsg
        (
                    TMemBuf&                mbufTar
            , const tCIDLib::TCard1         c1Val1
            , const tCIDLib::TCard1         c1Val2
            , const tCIDLib::TCard1         c1CBId
            , const tZWaveUSBSh::EUSetCmds  eCmd
        )   const;

        virtual tCIDLib::TBoolean bStoreFldIds
        (
            const   TVector<TCQCFldDef>&    colFlds
            , const tZWaveUSBSh::TFldIdList& fcolIds
        )   = 0;

        virtual tCIDLib::TBoolean bOwnsFld
        (
            const   tCIDLib::TCard4         c4FldId
        )   const = 0;

        virtual tCIDLib::TCard4 c4QueryFldDefs
        (
                    TVector<TCQCFldDef>&    colAddTo
            ,       TCQCFldDef&             flddTmp
        )   const = 0;

        virtual tCIDLib::TVoid HandleAppCmd
        (
            const   tCIDLib::TCard1* const  pc1Data
            , const tCIDLib::TCard4         c4Bytes
            ,       TCQCServerBase&         sdrvTarget
        )   = 0;

        virtual tCIDLib::TVoid SetFieldErrs
        (
                    TCQCServerBase&         sdrvTarget
        )   = 0;;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bHasCap
        (
            const   tZWaveUSBSh::EUnitCaps  eToTest
        )   const;

        tCIDLib::TBoolean bIsListening() const;

        tCIDLib::TBoolean bIsListening
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TCard1 c1ClassGen() const;

        tCIDLib::TCard1 c1ClassSpec() const;

        tCIDLib::TCard1 c1Id() const;

        tCIDLib::TCard1 c1Id
        (
            const   tCIDLib::TCard1         c1ToSet
        );

        tCIDLib::TCard4 c4IncRetries();

        tCIDLib::TCard4 c4Retries() const;

        tCIDLib::TCard4 c4Retries
        (
            const   tCIDLib::TCard4         c4ToSet
        );

        tZWaveUSBSh::EUnitCaps eCaps() const;

        tZWaveUSBSh::EUnitMatch eIsMatchingType
        (
            const   tCIDLib::TCard1         c1BaseType
            , const tCIDLib::TCard1         c1GenType
            , const tCIDLib::TCard1         c1SpecType
        );

        tZWaveUSBSh::EUnitStatus eStatus() const;

        tCIDLib::TEncodedTime enctLagTime() const;

        tCIDLib::TEncodedTime enctLastPoll() const;

        tCIDLib::TEncodedTime enctNextPoll() const;

        tCIDLib::TEncodedTime enctPollPeriod() const;

        tCIDLib::TEncodedTime enctPollPeriod
        (
            const   tCIDLib::TEncodedTime   enctToSet
        );

        tCIDLib::TVoid ResetPollTime();

        const TString& strName() const;

        const TString& strName
        (
            const   TString&                strToSet
        );

        const TString& strTypeName() const;

        const TString& strTypeInfo() const;

        tCIDLib::TVoid SetErrorState
        (
                    TCQCServerBase&         sdrvTarget
        );

        tCIDLib::TVoid SetFailedState();

        tCIDLib::TVoid SetMissingState();

        tCIDLib::TVoid SetReadyState();

        tCIDLib::TVoid SetZWFlags
        (
            const   tCIDLib::TCard1         c1ToSet
        );

        tCIDLib::TVoid StoreLagTime();


    protected :
        // -------------------------------------------------------------------
        //  Hidden Constructors and Destructor
        // -------------------------------------------------------------------
        TZWaveUnit
        (
            const   tCIDLib::TCard1         c1GenType
            , const tCIDLib::TCard1         c1SpecType
            , const TString&                strTypeName
            , const TString&                strTypelInfo
            , const tZWaveUSBSh::EUnitCaps  eCaps
            , const tCIDLib::TCard1         c1Id = kCIDLib::c1MaxCard
        );

        TZWaveUnit
        (
            const   TZWaveUnit&             unitToCopy
        );

        TZWaveUnit& operator=
        (
            const   TZWaveUnit&             unitToAssign
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
        //  Protected, non-virtual method
        // -------------------------------------------------------------------
        tCIDLib::TVoid SetDefName();


    private :
        // -------------------------------------------------------------------
        //  Unimplemented
        // -------------------------------------------------------------------
        TZWaveUnit();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c1ClassGen
        //  m_c1ClassSpec
        //      The generic and specifc Z-Wave classes of the device. This is
        //      fixed for a given derived class, so they just pass these in to
        //      our constructor. We don't care about the base class since any
        //      modules we will control are slave or slave/routers.
        //
        //      Mostly they will just set the generic class and leave the
        //      specific class zero, meaning that they just support the generic
        //      Z-Wave defined functionality for that device class. Only if
        //      it supports a specific OEM device version will it set the
        //      specific version.
        //
        //      During replication, these are used to find the best available
        //      unit derivative to handle each unit.
        //
        //      These aren't persisted. The derived class sets them on us
        //      during construction.
        //
        //  m_c1Id
        //      The id of the unit, which is what we need to actually
        //      communicate with the unit at the Z-Wave level.
        //
        //  m_c1ZWFlags
        //      We get a byte of flags from the unit info we get about each
        //      known unit. It's stored here and we provide boolean getter
        //      methods to access any of the bits we want to expose.
        //
        //  m_c4Retries
        //      We'll allow a unit a few failures before we declare its
        //      fields in error. This is used to track how many have
        //      happened so far. This is runtime only.
        //
        //  m_eCaps
        //      The capabilities flags that the derived class sets on us,
        //      based on his knowledge of the unit types he managed. It is
        //      assumed that they are static once set.
        //
        //  m_eStatus
        //      The overall status of the unit. This is a runtime value that
        //      is used to keep track of the health of the unit.
        //
        //  m_enctLagTime
        //      The StoreLagTime() method is called just before we are
        //      polled. We store the difference between the current time
        //      and our scheduled poll time. This is a runtime thing only,
        //      and only has meaning if the unit is polled.
        //
        //  m_enctLastPoll
        //      The last time this unit was polled. When a poll cycle is
        //      done, it will call our enctNextPoll() method and we'll return
        //      our last poll plus our poll period. It'll take the one with
        //      the oldest last poll that is ready to poll.
        //
        //  m_enctPollPeriod
        //      The poll period for this unit. Zero means it's not polled.
        //      it can't be less than two seconds if non-zero.
        //
        //  m_strName
        //      The name that the user gave this unit. When we first see a
        //      particular unit during replication, we'll give a name like
        //      "BinSwitch_XX", where XX is the hex id of the unit. The user
        //      can then rename it as desired.
        //
        //  m_strTypeInfo
        //      The derived type gives us a short model info description. It
        //      would be something like 'motion detector' or if it is an OEM
        //      specific derivative perhaps something like 'ACME thermo' and
        //      so forth. It's used in logged messages and for display. This
        //      isn't persisted, it's just set on us by the derived type any
        //      time one is created.
        //
        //  m_strTypeName
        //      The derived type gives us a type name string, which is a short
        //      string used in generating names and error messages. It would
        //      be something like BinSwitch, Thermo, MDetect, and so forth.
        //      This isn't persisted, it's just set on us by the derived type
        //      any time one is created.
        // -------------------------------------------------------------------
        tCIDLib::TCard1             m_c1ClassGen;
        tCIDLib::TCard1             m_c1ClassSpec;
        tCIDLib::TCard1             m_c1Id;
        tCIDLib::TCard1             m_c1ZWFlags;
        tCIDLib::TCard4             m_c4Retries;
        tZWaveUSBSh::EUnitCaps      m_eCaps;
        tZWaveUSBSh::EUnitStatus    m_eStatus;
        tCIDLib::TEncodedTime       m_enctLagTime;
        tCIDLib::TEncodedTime       m_enctLastPoll;
        tCIDLib::TEncodedTime       m_enctPollPeriod;
        TString                     m_strDescription;
        TString                     m_strName;
        TString                     m_strTypeInfo;
        TString                     m_strTypeName;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWaveUnit,TObject)
};



// ---------------------------------------------------------------------------
//   CLASS: TZWBinSensor
//  PREFIX: unit
//
//  This class represents an entry control unit, basically locks. We just need
//  to have storage for our one field id, which is the boolean field that we
//  use to control the lock/unlock state.
// ---------------------------------------------------------------------------
class ZWUSBSHEXPORT TZWBinSensor : public TZWaveUnit
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TZWBinSensor();

        TZWBinSensor
        (
            const   tCIDLib::TCard1         c1Id
        );

        TZWBinSensor
        (
            const   TZWBinSensor&           unitToCopy
        );

        ~TZWBinSensor();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TZWBinSensor& operator=
        (
            const   TZWBinSensor&           unitToAssign
        );


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bBuildGetMsg
        (
                    TMemBuf&                mbufTar
            ,       tCIDLib::TCard1&        c1RetClass
            ,       tCIDLib::TCard1&        c1RetCmd
            ,       tCIDLib::TCard1&        c1RetLen
            , const tCIDLib::TCard1         c1Val1
            , const tCIDLib::TCard1         c1Val2
            , const tCIDLib::TCard1         c1CBId
            , const tZWaveUSBSh::EUGetCmds  eCmd
        )   const;

        tCIDLib::TBoolean bBuildSetMsg
        (
                    TMemBuf&                mbufTar
            , const tCIDLib::TCard1         c1Val1
            , const tCIDLib::TCard1         c1Val2
            , const tCIDLib::TCard1         c1CBId
            , const tZWaveUSBSh::EUSetCmds  eCmd
        )   const;

        tCIDLib::TBoolean bStoreFldIds
        (
            const   TVector<TCQCFldDef>&    colFlds
            , const tZWaveUSBSh::TFldIdList& fcolIds
        );

        tCIDLib::TBoolean bOwnsFld
        (
            const   tCIDLib::TCard4         c4FldId
        )   const;

        tCIDLib::TCard4 c4QueryFldDefs
        (
                    TVector<TCQCFldDef>&    colAddTo
            ,       TCQCFldDef&             flddTmp
        )   const;

        tCIDLib::TVoid HandleAppCmd
        (
            const   tCIDLib::TCard1* const  pc1Data
            , const tCIDLib::TCard4         c4Bytes
            ,       TCQCServerBase&         sdrvTarget
        );

        tCIDLib::TVoid SetFieldErrs
        (
                    TCQCServerBase&         sdrvTarget
        );


    private :
        // -------------------------------------------------------------------
        //  Private data members.
        //
        //  m_c4FldId_State
        //      We just need one boolean field to represent the state of the
        //      sensor.
        // -------------------------------------------------------------------
        tCIDLib::TCard4 m_c4FldId_State;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWBinSensor,TZWaveUnit)
        DefPolyDup(TZWBinSensor)
};



// ---------------------------------------------------------------------------
//   CLASS: TZWBinSwitch
//  PREFIX: unit
//
//  This class represents a generic binary switch unit. We just need to have
//  storage for our one field id, which is the boolean field that we use to
//  control the switch.
// ---------------------------------------------------------------------------
class ZWUSBSHEXPORT TZWBinSwitch : public TZWaveUnit
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TZWBinSwitch();

        TZWBinSwitch
        (
            const   tCIDLib::TCard1         c1Id
        );

        TZWBinSwitch
        (
            const   TZWBinSwitch&           unitToCopy
        );

        ~TZWBinSwitch();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TZWBinSwitch& operator=
        (
            const   TZWBinSwitch&           unitToAssign
        );


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bBuildGetMsg
        (
                    TMemBuf&                mbufTar
            ,       tCIDLib::TCard1&        c1RetClass
            ,       tCIDLib::TCard1&        c1RetCmd
            ,       tCIDLib::TCard1&        c1RetLen
            , const tCIDLib::TCard1         c1Val1
            , const tCIDLib::TCard1         c1Val2
            , const tCIDLib::TCard1         c1CBId
            , const tZWaveUSBSh::EUGetCmds  eCmd
        )   const;

        tCIDLib::TBoolean bBuildSetMsg
        (
                    TMemBuf&                mbufTar
            , const tCIDLib::TCard1         c1Val1
            , const tCIDLib::TCard1         c1Val2
            , const tCIDLib::TCard1         c1CBId
            , const tZWaveUSBSh::EUSetCmds  eCmd
        )   const;

        tCIDLib::TBoolean bStoreFldIds
        (
            const   TVector<TCQCFldDef>&    colFlds
            , const tZWaveUSBSh::TFldIdList& fcolIds
        );

        tCIDLib::TBoolean bOwnsFld
        (
            const   tCIDLib::TCard4         c4FldId
        )   const;

        tCIDLib::TCard4 c4QueryFldDefs
        (
                    TVector<TCQCFldDef>&    colAddTo
            ,       TCQCFldDef&             flddTmp
        )   const;

        tCIDLib::TVoid HandleAppCmd
        (
            const   tCIDLib::TCard1* const  pc1Data
            , const tCIDLib::TCard4         c4Bytes
            ,       TCQCServerBase&         sdrvTarget
        );

        tCIDLib::TVoid SetFieldErrs
        (
                    TCQCServerBase&         sdrvTarget
        );


    private :
        // -------------------------------------------------------------------
        //  Private data members.
        //
        //  m_c4FldId_Switch
        //      We just need one boolean field to represent the switch.
        // -------------------------------------------------------------------
        tCIDLib::TCard4 m_c4FldId_Switch;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWBinSwitch,TZWaveUnit)
        DefPolyDup(TZWBinSwitch)
};



// ---------------------------------------------------------------------------
//   CLASS: TZWEntryCtrl
//  PREFIX: unit
//
//  This class represents an entry control unit, basically locks. We just need
//  to have storage for our one field id, which is the boolean field that we
//  use to control the lock/unlock state.
// ---------------------------------------------------------------------------
class ZWUSBSHEXPORT TZWEntryCtrl : public TZWaveUnit
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TZWEntryCtrl();

        TZWEntryCtrl
        (
            const   tCIDLib::TCard1         c1Id
        );

        TZWEntryCtrl
        (
            const   TZWEntryCtrl&           unitToCopy
        );

        ~TZWEntryCtrl();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TZWEntryCtrl& operator=
        (
            const   TZWEntryCtrl&           unitToAssign
        );


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bBuildGetMsg
        (
                    TMemBuf&                mbufTar
            ,       tCIDLib::TCard1&        c1RetClass
            ,       tCIDLib::TCard1&        c1RetCmd
            ,       tCIDLib::TCard1&        c1RetLen
            , const tCIDLib::TCard1         c1Val1
            , const tCIDLib::TCard1         c1Val2
            , const tCIDLib::TCard1         c1CBId
            , const tZWaveUSBSh::EUGetCmds  eCmd
        )   const;

        tCIDLib::TBoolean bBuildSetMsg
        (
                    TMemBuf&                mbufTar
            , const tCIDLib::TCard1         c1Val1
            , const tCIDLib::TCard1         c1Val2
            , const tCIDLib::TCard1         c1CBId
            , const tZWaveUSBSh::EUSetCmds  eCmd
        )   const;

        tCIDLib::TBoolean bStoreFldIds
        (
            const   TVector<TCQCFldDef>&    colFlds
            , const tZWaveUSBSh::TFldIdList& fcolIds
        );

        tCIDLib::TBoolean bOwnsFld
        (
            const   tCIDLib::TCard4         c4FldId
        )   const;

        tCIDLib::TCard4 c4QueryFldDefs
        (
                    TVector<TCQCFldDef>&    colAddTo
            ,       TCQCFldDef&             flddTmp
        )   const;

        tCIDLib::TVoid HandleAppCmd
        (
            const   tCIDLib::TCard1* const  pc1Data
            , const tCIDLib::TCard4         c4Bytes
            ,       TCQCServerBase&         sdrvTarget
        );

        tCIDLib::TVoid SetFieldErrs
        (
                    TCQCServerBase&         sdrvTarget
        );


    private :
        // -------------------------------------------------------------------
        //  Private data members.
        //
        //  m_c4FldId_State
        //      We just need one boolean field to represent the state of the
        //      lock
        // -------------------------------------------------------------------
        tCIDLib::TCard4 m_c4FldId_State;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWEntryCtrl,TZWaveUnit)
        DefPolyDup(TZWEntryCtrl)
};



// ---------------------------------------------------------------------------
//   CLASS: TZWMLSwitch
//  PREFIX: unit
//
//  This class represents a generic multi-level switch unit. We just need to
//  have storage for our one field id, which is the card field that we use
//  to control the switch.
// ---------------------------------------------------------------------------
class ZWUSBSHEXPORT TZWMLSwitch : public TZWaveUnit
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TZWMLSwitch();

        TZWMLSwitch
        (
            const   tCIDLib::TCard1         c1Id
        );

        TZWMLSwitch
        (
            const   TZWMLSwitch&            unitToCopy
        );

        ~TZWMLSwitch();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TZWMLSwitch& operator=
        (
            const   TZWMLSwitch&            unitToAssign
        );


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bBuildGetMsg
        (
                    TMemBuf&                mbufTar
            ,       tCIDLib::TCard1&        c1RetClass
            ,       tCIDLib::TCard1&        c1RetCmd
            ,       tCIDLib::TCard1&        c1RetLen
            , const tCIDLib::TCard1         c1Val1
            , const tCIDLib::TCard1         c1Val2
            , const tCIDLib::TCard1         c1CBId
            , const tZWaveUSBSh::EUGetCmds  eCmd
        )   const;

        tCIDLib::TBoolean bBuildSetMsg
        (
                    TMemBuf&                mbufTar
            , const tCIDLib::TCard1         c1Val1
            , const tCIDLib::TCard1         c1Val2
            , const tCIDLib::TCard1         c1CBId
            , const tZWaveUSBSh::EUSetCmds  eCmd
        )   const;

        tCIDLib::TBoolean bStoreFldIds
        (
            const   TVector<TCQCFldDef>&    colFlds
            , const tZWaveUSBSh::TFldIdList& fcolIds
        );

        tCIDLib::TBoolean bOwnsFld
        (
            const   tCIDLib::TCard4         c4FldId
        )   const;

        tCIDLib::TCard4 c4QueryFldDefs
        (
                    TVector<TCQCFldDef>&    colAddTo
            ,       TCQCFldDef&             flddTmp
        )   const;

        tCIDLib::TVoid HandleAppCmd
        (
            const   tCIDLib::TCard1* const  pc1Data
            , const tCIDLib::TCard4         c4Bytes
            ,       TCQCServerBase&         sdrvTarget
        );

        tCIDLib::TVoid SetFieldErrs
        (
                    TCQCServerBase&         sdrvTarget
        );


    private :
        // -------------------------------------------------------------------
        //  Private data members.
        //
        //  m_c4FldId_Switch
        //      We just need one card field to represent the switch.
        // -------------------------------------------------------------------
        tCIDLib::TCard4 m_c4FldId_Switch;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWMLSwitch,TZWaveUnit)
        DefPolyDup(TZWMLSwitch)
};


#pragma CIDLIB_POPPACK


