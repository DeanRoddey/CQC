//
// FILE NAME: ZWaveUSB3Sh_Unit_BinNotSensor_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/24/2017
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
//  This is a handler that can deal with a pretty wide range of binary sensors.
//
//  Note that there is a 'generic binary sensor' handler as well. That's one for
//  creating very generic handlers, which will generally require some user config. It
//  uses a single device info file that provides minimial information.
//
//  This one is where we can create a specific device info file for the unit, so we
//  can indicate in the device info file what type of sensor, what CC impl to use,
//  and what types of notification ids are used if it's notification based. So the
//  user doesn't have to provide any configuration. So this one is more semi-generic.
//
//  It would obviously be nice to this one where possible, so that it's as easy
//  as possible on the user. And of course it also allows for auto-config data to
//  be provided as well since there is a specific device info file. That's not possible
//  for a completely generic one, which has just one device info file.
//
//  The 'extra' configuration data has at least the following data.
//
//      Class : Basic | BinSensor | Notification
//       Type : CO | CO2 | Door | Glass | Heat | Intrusion | Motion | Smoke | Tilt| Water
//
//  Class indicates the CC impl class, and Type indicates the type of sensor. NOTE
//  THAT the Type value must match the EBinSensorTypes enum in the IDL file.
//
//
//  If the class indicates this one is notification based, then we need to set up the
//  notification CC impl with the right id info.
//
//  For motions sensors, we create a V2 compliant field and send out motion triggers
//  if configured to do so. For the others we create a field in the form:
//
//      [unitname]_State
//
//  KitchenSmoke_State, if the unit name is KitchenSmoke.
//
// CAVEATS/GOTCHAS:
//
//  1)  We only deal with push style notification sensors
//
// LOG:
//
#pragma once

#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TBinNotSensorUnit
//  PREFIX: unit
// ---------------------------------------------------------------------------
class TBinNotSensorUnit : public TZWaveUnit
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TBinNotSensorUnit() = delete;

        TBinNotSensorUnit
        (
                    TZWUnitInfo* const      punitiOwner
        );

        TBinNotSensorUnit(const TBinNotSensorUnit&) = delete;

        ~TBinNotSensorUnit();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TBinNotSensorUnit& operator=(const TBinNotSensorUnit&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bParseExtraInfo() override;

        tCIDLib::TBoolean bPrepare() override;

        tCIDLib::TVoid ImplValueChanged
        (
            const   tCIDLib::TCard4         c4ImplId
            , const tZWaveUSB3Sh::EValSrcs  eSource
            , const tCIDLib::TBoolean       bWasInErr
            , const tCIDLib::TInt4          i4ExtraInfo
        )   override;

        tCIDLib::TVoid QueryUnitAttrs
        (
                    tCIDMData::TAttrList&   colToFill
            ,       TAttrData&              attrTmp
        )   const override;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_eHandlerClass
        //      Indicates what type of CC impl to create to handle this sensor,
        //      i.e. what types of reports we should expect. This is driven by the
        //      device info.
        //
        //  m_eSensorType
        //      The specific type of binary sensor. This tells us more about the
        //      unit which may be important in setting up the CC impl.
        //
        //  m_pzwcciVal
        //      We don't own it, but we keep a typed pointer to our motion CC impl
        //      so that we can get its value out when we need to send a trigger.
        //      It can be different types but they all derive from the binary
        //      CC impl.
        //
        //  m_strFldName
        //      Since we store the CC impl via a base class that knows nothing of
        //      fields, we save our field name for logging, triggers, etc...
        // -------------------------------------------------------------------
        tZWaveUSB3Sh::ECClasses         m_eHandlerClass;
        tZWaveUSB3Sh::EBinSensorTypes   m_eSensorType;
        TZWCCImplBin*                   m_pzwcciVal;
        TString                         m_strFldName;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TBinNotSensorUnit, TZWaveUnit)
};

#pragma CIDLIB_POPPACK
