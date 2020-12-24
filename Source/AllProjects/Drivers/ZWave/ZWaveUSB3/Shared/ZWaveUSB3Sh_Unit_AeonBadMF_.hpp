//
// FILE NAME: ZWaveUSB3Sh_Unit_AeonBadMF_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/14/2018
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
//
//  THIS HANDLER IS obsolete and the new NMCMSensor handler should be used for
//  such things. At some point we'd like to migrate the few things that use this
//  handler to the new one.
//
//
//
//  This is a specialized handler for the very badly non-compliant family of multi-
//  function sensors from Aeotec. They have multiple ML sensor ends points but don't
//  use the multi-channel system. So the only way to deal with them is that they never
//  have more than one of a given sensor type. We have to look at the data type
//  in incoming ML sensor messages and route only the messages to the CC impl
//  that handles that data type.
//
//  We assume one (optional) motion sensor and some number of multi-level sensors.
//  Anything else would just not be supportable via this handler without changes.
//
//  Since this is all very non-standard, we have to manage all of the impl objects
//  so that we can route msgs and control what is going on.
//
//  We have a vector of CC impls that is indexed by an enum that lists the types of
//  sensor types we support. For each supported type (as reported by the device
//  info file) we create the CC Impl and set it up and put it into its appropriate
//  slot. Then it's a matter of passed on things to the right ones. Unsupported slots
//  are null.
//
//  The device info file extra info tells us what types we have. It is in the
//  form:
//
//      SensTypes=type1[ type2 type3 ...]
//      Motion=[Basic | BinSensor | Notification]
//      Motion2=[Basic | BinSensor | Notification]
//
//  So a list of space separated sensor types to support. The possible values are defined
//  by the tZWaveUSB3Sh::EAeonMSTypes enum.
//
//  There can also be values like this:
//
//      Range_xxx=0 100
//
//  to indicate a specific range to be used for any of the sensors, where xxx is one
//  of the valid sensor types. Temp is not supported for this since it is set by way
//  of use options.
//
//  There cannot be more than one of any given type. We might add more later potentially.
//
//  If motion is supported, then we need to be told which CC to use for motion. They
//  can also tell us to do a second motion handler. This one will be a non-field CC
//  and we'll just monitor it and force any changes into the first motion CC's field.
//  Motion2 is optional. If it present, it cannot be the same type as Motion.
//
//  If either is a notification, we assume that the standard extra info is set in the
//  device info file under the notification class, to tell it what notification id and
//  off/on events to look for. We'd assume the standard motion ones, but it's best to
//  be flexible.
//
//
// CAVEATS/GOTCHAS:
//
//  1)  Since this guy (unusually) has CC impls that it does not set on the base unit
//      class, we have to override callbacks and distribute them to our CC impls, where
//      normally the base class would do that.
//
// LOG:
//
#pragma once

#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TAeonBadMFUnit
//  PREFIX: unit
// ---------------------------------------------------------------------------
class TAeonBadMFUnit : public TZWaveUnit
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TAeonBadMFUnit() = delete;

        TAeonBadMFUnit
        (
                    TZWUnitInfo* const      punitiOwner
        );

        TAeonBadMFUnit(const TAeonBadMFUnit&) = delete;

        ~TAeonBadMFUnit();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TAeonBadMFUnit& operator=(const TAeonBadMFUnit&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bOwnsFld
        (
            const   tCIDLib::TCard4         c4FldId
        )   const final;

        tCIDLib::TBoolean bParseExtraInfo() final;

        tCIDLib::TBoolean bPrepare() final;

        tCIDLib::TVoid FormatCCsInfo
        (
                    TTextOutStream&         strmTar
        )   const final;

        tCIDLib::TVoid HandleCCMsg
        (
            const   tCIDLib::TCard1         c1Class
            , const tCIDLib::TCard1         c1Cmd
            , const TZWInMsg&               zwimToHandle
        )   final;

        tCIDLib::TVoid ImplValueChanged
        (
            const   tCIDLib::TCard4         c4ImplId
            , const tZWaveUSB3Sh::EValSrcs  eSource
            , const tCIDLib::TBoolean       bWasInErr
            , const tCIDLib::TInt4          i4ExtraInfo
        )   final;

        tCIDLib::TVoid QueryFldDefs
        (
                    tCQCKit::TFldDefList&   colAddTo
            ,       TCQCFldDef&             flddTmp
        )   const final;

        tCIDLib::TVoid QueryUnitAttrs
        (
                    tCIDMData::TAttrList&   colToFill
            ,       TAttrData&              attrTmp
        )   const final;

        tCIDLib::TVoid Process() final;

        tCIDLib::TVoid StoreFldIds() final;

        tCIDLib::TVoid UnitIsOnline() final;


    private :
        // -------------------------------------------------------------------
        //  Private class types
        // -------------------------------------------------------------------
        using TFlagList = TEArray
        <
            tCIDLib::TBoolean
            , tZWaveUSB3Sh::EAeonMSTypes
            , tZWaveUSB3Sh::EAeonMSTypes::Count
        >;
        using TCCList = TEArray
        <
            TZWCCImpl*
            , tZWaveUSB3Sh::EAeonMSTypes
            , tZWaveUSB3Sh::EAeonMSTypes::Count
        >;


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCheckRange
        (
            const   tZWaveUSB3Sh::EAeonMSTypes eType
            ,       tCIDLib::TCard4&        c4Min
            ,       tCIDLib::TCard4&        c4Max
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_abSensorFlags
        //      As with m_colImpls this is a set of flags with a slot for each
        //      supported sensor type. We need this to remember which ones are
        //      supported during bParseExtraInfo(), because we need to know during
        //      QueryUnitAttrs() and the impls haven't been created at that point.
        //
        //  m_azwcciSensors
        //      For each sensor type supported, there is a slot in this list. So
        //      a null slot is a non-supported type. It is indexed by the Aeon multi-
        //      sensor enum.
        //
        //  m_eMotionCC
        //      Motion can be done via Binary Sensor, Basic, or Notification. The
        //      device info file tells us which.
        //
        //  m_eMotion2CC
        //      We can have an optional second motion handler, which will not create
        //      a field, we will just monitor it and push any changes into the
        //      main motion handler.
        //
        //      This one is not in the sensors list, since it is a special case,
        //      because it doesn't have a field.
        //
        //  m_pzwcciMotion
        //      This one is in the m_azwcciSensors list, but we also need to talk to
        //      it via its base binary impl class, so we keep a separate pointer. This
        //      doesn't own it. The list above does, well we do via that list.
        //
        //  m_pzwcciMotion2
        //      If there is a motion 2 handler, we have to deal with it specially
        //      so it's not in the list.
        //
        //  m_strMotFldName
        //      We keep our motion field name around (which we set on the main motion
        //      CC impl if we have one) since we need it in event triggers, and we
        //      store the motion guys via the base binary impl class which doesn't
        //      know anything about fields so we can't get it from him.
        // -------------------------------------------------------------------
        TFlagList               m_abSensorFlags;
        TCCList                 m_azwcciSensors;
        tZWaveUSB3Sh::ECClasses m_eMotionCC;
        tZWaveUSB3Sh::ECClasses m_eMotion2CC;
        TZWCCImplBin*           m_pzwcciMotion;
        TZWCCImplBin*           m_pzwcciMotion2;
        TString                 m_strMotFldName;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TAeonBadMFUnit, TZWaveUnit)
};

#pragma CIDLIB_POPPACK
