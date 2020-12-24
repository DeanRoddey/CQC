//
// FILE NAME: ZWaveLevih_UnitInfo.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/10/2012
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
//  This is the header file for the ZWaveLevi_UnitInfo.cpp file, which
//  implements the TZWaveUnit class. This abstract base class represents
//  the basic information that we keep up with for all units and the
//  interface that all unit types must implements. There are derivatives of
//  this class that handle dealing with specific types of units.
//
//  We also keep up with some runtime info such as the last time the unit
//  was polled, how many failed polls since the last good one and so forth.
//  This stuff is not persisted, it's just for runtime usage by the driver
//  proper.
//
//  Each derivative will cause the creation of one or more driver
//  fields. The derived classes must tell the driver what fields it needs
//  created. The driver will give him back the ids for those fields, which he
//  must store for later usage.
//
//  These must be streamable, so that they can be stored to the Z-Wave driver
//  configuration and so that they can be streamed to the client for display
//  and configuration purposes.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TZWaveUnit
//  PREFIX: unit
// ---------------------------------------------------------------------------
class ZWLEVISHEXPORT TZWaveUnit

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

        static tCIDLib::ESortComps eCompByNextPoll
        (
            const   TZWaveUnit&             unit1
            , const TZWaveUnit&             unit2
            , const tCIDLib::TCard4         c4UserData
        );

        static TZWaveUnit* punitBestMatch
        (
            const   tCIDLib::TCard4         c4GenType
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
                    TString&                strToFill
            , const tCIDLib::TCard4         c4Index
        );

        virtual tCIDLib::TBoolean bBuildSetMsg
        (
            const   tCIDLib::TCard4         c4FldId
            , const TString&                strValue
            ,       TString&                strToFill
            ,       TCQCServerBase&         sdrvLevi
        );

        virtual tCIDLib::TBoolean bOwnsFld
        (
            const   tCIDLib::TCard4         c4FldId
        )   const = 0;

        virtual tCIDLib::TBoolean bStoreBasicReport
        (
            const   tCIDLib::TCard4         c4Value
            ,       TCQCServerBase&         sdrvLevi
        );

        virtual tCIDLib::TBoolean bStoreFldIds
        (
            const   TVector<TCQCFldDef>&    colFlds
            , const tZWaveLeviSh::TValList& fcolIds
        )   = 0;

        virtual tCIDLib::TBoolean bStoreGenReport
        (
            const   tCIDLib::TCard4         ac4Values[]
            , const tCIDLib::TCard4         c4ValCnt
            ,       TCQCServerBase&         sdrvLevi
        );

        virtual tCIDLib::TBoolean bStoreSceneReport
        (
            const   tCIDLib::TCard4         c4Scene
            , const tCIDLib::TCard4         c4Levle
            , const tCIDLib::TCard4         c4FadeTime
            ,       TCQCServerBase&         sdrvLevi
        );

        virtual tCIDLib::TBoolean bWantsOptInfo
        (
                    tCIDLib::TStrCollect&   colToFill
        )   const;

        virtual tCIDLib::TCard4 c4QueryFldDefs
        (
                    TVector<TCQCFldDef>&    colAddTo
            ,       TCQCFldDef&             flddTmp
        )   const = 0;

        virtual tCIDLib::TVoid PrepForUse();

        virtual tCIDLib::TVoid SetFieldErrs
        (
                    TCQCServerBase&         sdrvLevi
        )   = 0;;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bBasicSupport() const;

        tCIDLib::TBoolean bDecodeValue
        (
            const   tCIDLib::TCard4         ac4Vals[]
            , const tCIDLib::TCard4         c4ValCnt
            ,       tCIDLib::TInt4&         i4Result
            , const tCIDLib::TCh* const     pszFld
            ,       TCQCServerBase&         sdrvLevi
        )   const;

        tCIDLib::TBoolean bDisabled() const;

        tCIDLib::TBoolean bDisabled
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TBoolean bHasCap
        (
            const   tZWaveLeviSh::EUnitCaps eToTest
        )   const;

        tCIDLib::TBoolean bHasFixedCap
        (
            const   tZWaveLeviSh::EUnitCaps eToTest
        )   const;

        tCIDLib::TBoolean bIsBatteryPwr() const;

        tCIDLib::TBoolean bIsPollable() const;

        tCIDLib::TBoolean bIsReadable() const;

        tCIDLib::TBoolean bIsViable() const;

        tCIDLib::TCard1 c1ClassGen() const;

        tCIDLib::TCard1 c1ClassSpec() const;

        tCIDLib::TCard4 c4GetMsgCnt() const;

        tCIDLib::TCard4 c4GetMsgCnt
        (
            const   tCIDLib::TCard4         c4ToSet
        );

        tCIDLib::TCard4 c4IncGetMsgIndex();

        tCIDLib::TCard4 c4Id() const;

        tCIDLib::TCard4 c4Id
        (
            const   tCIDLib::TCard4         c4ToSet
        );

        tCIDLib::TCard4 c4OptInfo() const;

        tCIDLib::TCard4 c4OptInfo
        (
            const   tCIDLib::TCard4         c4ToSet
        );

        tZWaveLeviSh::EUnitCaps eCaps() const;

        tZWaveLeviSh::EUnitCaps eFixedCaps() const;

        tZWaveLeviSh::EUnitStatus eStatus() const;

        tCIDLib::TEncodedTime enctLastPoll() const;

        tCIDLib::TEncodedTime enctLastValue() const;

        tCIDLib::TEncodedTime enctNextPoll() const;

        tCIDLib::TEncodedTime enctPollPeriod() const;

        tCIDLib::TEncodedTime enctPollPeriod
        (
            const   tCIDLib::TEncodedTime   enctToSet
        );

        tCIDLib::TVoid ResetLastValTime();

        tCIDLib::TVoid ResetPollTime
        (
            const   tCIDLib::TBoolean       bSetLastValStamp
        );

        const TString& strName() const;

        const TString& strName
        (
            const   TString&                strToSet
        );

        const TString& strTypeName() const;

        const TString& strTypeInfo() const;

        tCIDLib::TVoid SetCapFlags
        (
            const   tZWaveLeviSh::EUnitCaps eToSet
            , const tCIDLib::TBoolean       bState
        );

        tCIDLib::TVoid SetErrorStatus
        (
                    TCQCServerBase* const   psdrvLevi
        );

        tCIDLib::TVoid SetFailedStatus
        (
                    TCQCServerBase* const   psdrvLevi
        );

        tCIDLib::TVoid SetMissingStatus
        (
                    TCQCServerBase* const   psdrvLevi
        );

        tCIDLib::TVoid SetReadyStatus
        (
                    TCQCServerBase* const   psdrvLevi
        );


    protected :
        // -------------------------------------------------------------------
        //  Hidden Constructors and Destructor
        // -------------------------------------------------------------------
        TZWaveUnit
        (
            const   tCIDLib::TCard1         c1GenType
            , const tCIDLib::TCard1         c1SpecType
            , const TString&                strTypeName
            , const TString&                strTypeInfo
            , const tZWaveLeviSh::EUnitCaps eCaps
            , const tZWaveLeviSh::EUnitCaps eFixedCaps
            , const tCIDLib::TCard4         c4Id = kCIDLib::c4MaxCard
            , const tCIDLib::TCard4         c4GetMsgCnt = 1
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
        tCIDLib::TBoolean bCheckFldId
        (
            const   tCIDLib::TCard4         c4ToCheck
            , const tCIDLib::TCh* const     pszFldName
        )   const;

        tCQCKit::EFldAccess eDefFldAccess() const;

        tCIDLib::TVoid SetDefName();


    private :
        // -------------------------------------------------------------------
        //  Unimplemented
        // -------------------------------------------------------------------
        TZWaveUnit();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bDisabled
        //      The driver doesn't have to interact with all of the units
        //      available on the Z-Wave network. If the user doesn't need a
        //      given unit to be controlled, he can mark it disabled. So we
        //      always have the known units in the configuration, but we can
        //      ignore some if desired. This is persisted of course.
        //
        //  m_c1ClassGen
        //  m_c1ClassSpec
        //      The generic and specific Z-Wave classes of the device. This
        //      is fixed for a given derived class, so they just pass these
        //      in to our ctor. We don't care about the base class since any
        //      modules we will control are slave or slave/routers. The
        //      specific type is generally 0, which means any module of the
        //      generic type. But in some cases the derived class will only
        //      work for a specific type.
        //
        //      These aren't persisted. The derived class sets them on us
        //      during construction.
        //
        //  m_c4GetMsgCnt
        //      Some units have more than one value they need to query (at
        //      least upon initialization, and possibly round robining through
        //      if we have to poll.) They tell us how many, so that the driver
        //      can have access to this info. It will use our m_c4CurGetMsgInd
        //      field to round robin through all the required gets.
        //
        //  m_c4Id
        //      The id of the unit, which is what we need to actually
        //      communicate with the unit at the Z-Wave level.
        //
        //  m_c4OptInfo
        //      Some derivatives need to get some extra info from the user.
        //      They can provide a list of text options that the client driver
        //      will present to the user to select from. This is the index of
        //      the one selected. It can be used for whatever the derived class
        //      wants. If no selection, it's set to zero. This is persistend
        //      since it's a user setting. They must always return their list
        //      of options in the same order, addng new ones at the end, so
        //      that this value remains valid. The virtual method bWantsOptInfo()
        //      indicates whether the derived class wants to get optional info
        //      or not, and provides the list of string values to present.
        //
        //  m_eCaps
        //      The capabilities flags that the derived class sets on us,
        //      based on his knowledge of the unit types he managed. The
        //      derived class will set defaults, and then the user can change
        //      them after that.
        //
        //  m_eFixCaps
        //      Another set of capabilities flags set on us by the derived
        //      class. The ones that are on here are not settable by the
        //      user. They will retain the values provided by the derived
        //      class since the client driver won't allow them to be changed.
        //
        //  m_eStatus
        //      The overall status of the unit. This is a runtime value that
        //      is used to keep track of the health of the unit, so it's not
        //      like disabled which is a user setting.
        //
        //  m_enctLastPoll
        //      The last time this unit was polled. When a poll cycle is
        //      done, it will call our enctNextPoll() method and we'll return
        //      our last poll plus our poll period. It'll take the one with
        //      the oldest last poll that is ready to poll.
        //
        //      This is set even if the unit is not pollable. The units just
        //      call ResetPollTime() when they get a new value, regardless. They
        //      don't know whether it came in because of a poll or an async.
        //      So it's always either the last time we ask for a value, or the
        //      time at which some value came in.
        //
        //  m_enctLastValue
        //      We remember the last time we got a status value for each
        //      unit. If the unit is pollable and this value gets to be
        //      three times the poll period, we put the field into error
        //      state until such time as we get a value for it, since it
        //      seems like it's having issues.
        //
        //      This is usually set by the same ResetPollTime() method that
        //      is used to update m_enctLastPoll, since they are both usually
        //      called because a value has come in.
        //
        //  m_enctPollPeriod
        //      The poll period for this unit. Zero means it's not polled.
        //      it can't be less than two seconds if non-zero.
        //
        //  m_strName
        //      The name that the user gave this unit.
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
        tCIDLib::TBoolean           m_bDisabled;
        tCIDLib::TCard1             m_c1ClassGen;
        tCIDLib::TCard1             m_c1ClassSpec;
        tCIDLib::TCard4             m_c4CurGetMsgIndex;
        tCIDLib::TCard4             m_c4GetMsgCnt;
        tCIDLib::TCard4             m_c4Id;
        tCIDLib::TCard4             m_c4OptInfo;
        tZWaveLeviSh::EUnitCaps     m_eCaps;
        tZWaveLeviSh::EUnitCaps     m_eFixedCaps;
        tZWaveLeviSh::EUnitStatus   m_eStatus;
        tCIDLib::TEncodedTime       m_enctLastPoll;
        tCIDLib::TEncodedTime       m_enctLastValue;
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


#pragma CIDLIB_POPPACK


