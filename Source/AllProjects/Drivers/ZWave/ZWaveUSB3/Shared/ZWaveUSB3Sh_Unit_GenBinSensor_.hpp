//
// FILE NAME: ZWaveUSB3Sh_Unit_GenBinSensor_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/11/2018
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
//  This is a generic binary sensor handler. We can't depend on the device info file
//  for these, because it has to be totally generic. So we let the user select from
//  Basic, Binary Sensor, or Notification, and we create one of that type.
//
//  We also have a user option for them to tell us what type of sensor it is, so that
//  we can create an appropriate field. If they say it is a motion sensor, then we
//  create a V2 compliant motion field.
//
//  NOTE THAT there is also a BinNotSensor handler. That one is semi-generic, and is
//  used for binary sensors that we know about. I.e. it requires a specific device
//  info file for each unit that uses it. It's nice to do that where necessary, but
//  this provides a fallback since there will always be ones we don't know about.
//
//
// CAVEATS/GOTCHAS:
//
//  1)  We only deal with push style notification sensors if it's notification based.
//
//  2)  If notification based, we assume the standard notification V3 notification
//      id and events for the specific type of sensor. So motion would be id 0x7
//      0 for stop, and 0x7/0x8 for motion start, and so forth.
//
//  3)  This guy DOES NOT deal with multiple end points. So it will only work with
//      single point units.
//
// LOG:
//
#pragma once

#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TGenBinSensorUnit
//  PREFIX: unit
// ---------------------------------------------------------------------------
class TGenBinSensorUnit : public TZWaveUnit
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TGenBinSensorUnit() = delete;

        TGenBinSensorUnit
        (
                    TZWUnitInfo* const      punitiOwner
        );

        TGenBinSensorUnit(const TGenBinSensorUnit&) = delete;

        ~TGenBinSensorUnit();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TGenBinSensorUnit& operator=(const TGenBinSensorUnit&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
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
        //  m_eSensorType
        //      This is driven by a user settable option. We set it up in bPrepare
        //      and keep it around for later use in QueryUnitAttrs and decided whether
        //      to send triggers and so forth.
        //
        //  m_pzwcciSensor
        //      We keep a pointer to our sensor impl around (via the binary impl
        //      base class all our pontential types share) so that we can get its
        //      value our for sending triggers and such. we don't own it.
        //
        //  m_strFldName
        //      We keep our field name around for logging, triggers, etc...
        // -------------------------------------------------------------------
        tZWaveUSB3Sh::EBinSensorTypes   m_eSensorType;
        TZWCCImplBin*                   m_pzwcciSensor;
        TString                         m_strFldName;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TGenBinSensorUnit, TZWaveUnit)
};

#pragma CIDLIB_POPPACK
