//
// FILE NAME: ZWaveUSB3Sh_Unit_Thermostat_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/7/2018
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
//  This is a parameterizeable thermostat unit. Basically we just need to be told some
//  basic info about temp scale and which set points are supported, and we then gen
//  up the correct CC impls for the supported functionality.
//
//  We have two basic setup schemes. One is that the v2 compatibility flag is set:
//
//      V2Compat=Yes
//
//  If so, then we force all the options on and we'll force the correct read/write
//  access on all of the CCs. We require that the parent unit be always on type if this
//  is set or we give an error during bParseExtraInfo(). Given that, we know that
//  the default access for the thermo specific CCs will be Read, so we just need to
//  for on write for those that have to be writeable in V2.
//
//  Otherwise, we look for the Features value, and will use that to decide what
//  optional features we should implement, and we will keep any RWAccess flags set
//  in the individual CCs (or the defaults that their ctors set otherwise.)
//
//      Features=[Fan | LowSP | HighSP]
//
//
//  We have user settable options for temp scale and min/max temps (which we let them
//  set for range purposes.) A huge all purpose range would make things like
//  thermostat interface widgets not as useful for folks who live in an area where
//  the temp range is fairly small.
//
//  Mostly each of the CC impls manages its own affairs so we just set them on our
//  parent and are done. But the current temp is an issue, see Temp Scale below.
//
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once

#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TThermoUnit
//  PREFIX: unit
// ---------------------------------------------------------------------------
class TThermoUnit : public TZWaveUnit
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TThermoUnit() = delete;

        TThermoUnit
        (
                TZWUnitInfo* const          punitiOwner
        );

        TThermoUnit(const TThermoUnit&) = delete;

        ~TThermoUnit();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TThermoUnit& operator=(const TThermoUnit&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bParseExtraInfo() override;

        tCIDLib::TBoolean bPrepare() override;

        tCIDLib::TVoid QueryUnitAttrs
        (
                    tCIDMData::TAttrList&   colToFill
            ,       TAttrData&              attrTmp
        )   const override;



    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bDoXXX
        //      These are the bits that may or may not be implemented on a given
        //      thermostat. We set these in bParseExtraInfo() so that they are
        //      there for QueryUnitAttrs, which needs to only return attributes
        //      for the stuff we actually are doing.
        //
        //  m_bV2Compat
        //      If this is set via the extra info, then everything else is forced
        //      to V2 standards.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bDoFan;
        tCIDLib::TBoolean   m_bDoLowSP;
        tCIDLib::TBoolean   m_bDoHighSP;
        tCIDLib::TBoolean   m_bV2Compat;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TThermoUnit, TZWaveUnit)
};

#pragma CIDLIB_POPPACK
