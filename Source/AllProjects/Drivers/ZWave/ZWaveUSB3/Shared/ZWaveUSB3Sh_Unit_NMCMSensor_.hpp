//
// FILE NAME: ZWaveUSB3Sh_Unit_NMCMSensor.hpp
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
//  NOTE that there is an older handler for this of unit that was originally created
//  for Aeon units (the first we ran across that did this), but it is superseded by
//  this one and any using that one could be updated to use this one but it would be
//  a breaking change that would force uses to rescan those units. So we just left
//  those as is.
//
//
//  This is the implementation for common family of multi-sensor type devices
//  that just randomly include multiple sensors within the unit, without any
//  attempt to use multi-channel features to make them manageable.
//
//  So it's purely based on what CCs and notification ids and sensor types and
//  such that we know what incoming msgs are for what sub-unit
//
//  Since this is all very non-standard, we have to manage all of the impl objects
//  so that we can route msgs and control what is going on.
//
//  The device info file extra info tells us what types we have. This is complex so
//  it is a formatted string in the form:
//
//      Sensor1=xxx
//      Sensor2=xxx
//      Sensor3=xxx
//
//  Where each entry defines one sub-unit within the unit. The numbers must be
//  contiguous, so it can just search for those extra info values until it doesn't
//  find one.
//
//  The xxx bit is a semi-colon separated string in the basic form below, though extra
//  bits are possible based on the type and notification cc.
//
//      type; primarycc; notifycc; access
//
//      Type      = One of the values supported by the tZWaveUSB3Sh::EGenMSTypes enum,
//                  the base part of the enum value names of course.
//
//      PrimaryCC = The CC that is used to read values (if readable) and via which
//                  we will get notifications if there is no notify cc. *
//                      [Basic | BinSensor | MLSensor | Notification]
//
//      NotifyCC  = If the primary CC is not used to report changes, this is a
//                  separate CC that will be used. **
//                      [Basic | BinSensor | MLSensor | Notification]
//
//      ReadAcc   = These are the same values (space separated) that are supported
//                  by the base CC impl class, so see that header.
//
//  The multi-level type sensors won't have a notification CC, they will always use
//  the multi-level sensor type as the primary and they will receive changes via that
//  CC. So the notification CC value will just be blank, something like:
//
//      Humidity; MLSensor; ; Read Write
//
//  If the notification CC is the primary or Notification class, and the type indicates
//  a binary type there will be a set of trailing values to indicate the notification
//  type and ids for off/on state notification, which is basically the same info as the
//  Notification CC takes as his extra info, such as:
//
//      Motion; BinSensor; Notification; Read ReadOnWake; NotType; EventType; 5; 0 7; 8
//
//  So it is notification id type, the event type, the notification id, the off events
//  and the on events.
//
//  This leaves the option open later for possibly an enumerated type that gets
//  notification type msgs as well, but currently it's only binary.
//
//  * It's unlikely but I guess theoretically possible that a binary sensor type won't
//  have any CC other than notification to get the state. We don't support that here. It
//  could be added later by just leave the primary empty, and requiring the notification
//  CC in that case.
//
//  * We assume that multi-level sensor primaries have no notification CC, since it's
//  not really very likely and it complicates things.
//
//  Restrictions
//
//  Clearly, being a non-multi-channel device, no two sensors can use the same binary
//  CC. And no two can use the same multi-level sensor and sensor type combination since
//  it's only the sensor type that lets us distinguish them.
//
//  Multiple binaries can use the Notifcation class for notifications as long as they
//  use different notification ids.
//
//  Unit Options
//
//  The above restrictions mean, for instance, that there can only be one temp sensor, so
//  we can provide unit-wide temp scale/range user settable options. If we have any motion
//  sensor, we'll provide the standard 'send event triggers' option and apply it to all of
//  them.
//
// CAVEATS/GOTCHAS:
//
//  1)  Since this guy has CC impls that it does not set on the base unit class, we
//      have to override callbacks and distribute them to our CC impls, where normally
//      the base class would do that.
//
//      Some common stuff will still be set on the parent, such as battery or other
//      unit-wide stuff, so we still have to call our parent first, then process our
//      own.
//
// LOG:
//
#pragma once

#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TNonMCMultiSensorUnit
//  PREFIX: unit
// ---------------------------------------------------------------------------
class TNonMCMultiSensorUnit : public TZWaveUnit
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TNonMCMultiSensorUnit() = delete;

        TNonMCMultiSensorUnit
        (
                    TZWUnitInfo* const      punitiOwner
        );

        TNonMCMultiSensorUnit(const TNonMCMultiSensorUnit&) = delete;

        ~TNonMCMultiSensorUnit();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TNonMCMultiSensorUnit& operator=(const TNonMCMultiSensorUnit&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bBoolFldChanged
        (
            const   TString&                strFldName
            , const tCIDLib::TCard4         c4FldId
            , const tCIDLib::TBoolean       bValue
            ,       tCQCKit::ECommResults&  eRes
        )   final;

        tCIDLib::TBoolean bCardFldChanged
        (
            const   TString&                strFldName
            , const tCIDLib::TCard4         c4FldId
            , const tCIDLib::TCard4         c4Value
            ,       tCQCKit::ECommResults&  eRes
        )   final;

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

        tCIDLib::TVoid GetInitVals() final;

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
        //  Private data types
        // -------------------------------------------------------------------

        // We need a small class to hold the definintion info for each sensor
        class TSensorInfo
        {
            public :
                TSensorInfo();
                TSensorInfo(const TSensorInfo&);
                ~TSensorInfo();

                tCIDLib::TBoolean bCreateCCs
                (
                            TZWUnitInfo&        unitiUs
                    , const tCIDLib::TCard4     c4Index
                    ,       tCIDLib::TCard4&    c4FauxId
                    ,       TString&            strErr
                );

                tCIDLib::TBoolean bHandleImplChange
                (
                            TZWUnitInfo&        unitiUs
                    , const tCIDLib::TCard4     c4ImplId
                );

                tCIDLib::TBoolean bParseInfo
                (
                    const   TString&            strInfo
                    , const TZWUnitInfo&        unitiSrc
                    ,       TString&            strErr
                );



                // The fixed stuff we get for all sensors from the device info file
                tZWaveUSB3Sh::EUnitAcc      m_eAccess;
                tZWaveUSB3Sh::ECClasses     m_eNotCC;
                tZWaveUSB3Sh::ECClasses     m_ePrimCC;
                tZWaveUSB3Sh::EGenMSTypes   m_eType;

                //
                //  For convenience we remember if this is a binary or multi-level
                //  sensor, during the parsing of the device info. We also remember
                //  the field name for logging and event trigger purposes.
                //
                tCIDLib::TBoolean           m_bIsBinary;
                TString                     m_strFldName;

                //
                //  The actual CCs themselves, which we own. Notify one may be null.
                //  These are created by bCreateCCs().
                //
                TZWCCImpl*                  m_pzwcciNot;
                TZWCCImpl*                  m_pzwcciPrim;

                //
                //  If the Notification CC is used for either primary or notification
                //  (and it can only be one or the other), then these are required.
                //
                tZWaveUSB3Sh::ENotEvTypes   m_eNotEvType;
                tZWaveUSB3Sh::ENotIdTypes   m_eNotIdType;
                tCIDLib::TCard2             m_c2NotId;
                tCIDLib::TByteList          m_fcolOffEvs;
                tCIDLib::TByteList          m_fcolOnEvs;


            private :
                tCIDLib::TBoolean bPushChange
                (
                            TZWCCImpl&      zwcciSrc
                    ,       TZWCCImpl&      zwcciTar
                );
        };


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_colSensors
        //      The list of sensors we parsed from the extra info.
        // -------------------------------------------------------------------
        TVector<TSensorInfo>    m_colSensors;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TNonMCMultiSensorUnit, TZWaveUnit)
};

#pragma CIDLIB_POPPACK
