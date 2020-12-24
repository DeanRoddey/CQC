//
// FILE NAME: ZWaveUSB3Sh_Unit_TempSP_.hpp
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
//  This is a simple handler for what is probably a sort of common module type, which
//  is a simple thermostat that is just a temp sensor and one or both set points. So
//  it's not a full thermostat in the V2 sense.
//
//  The temp is assumed to be via MLSensor, so we only need to know which set points
//  to support, so we have an extra info value:
//
//      Features=LowSP,HighSP
//
//  So it's one or both values to indicate which set points to support.
//
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once

#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TTempSPUnit
//  PREFIX: unit
// ---------------------------------------------------------------------------
class TTempSPUnit : public TZWaveUnit
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TTempSPUnit() = delete;

        TTempSPUnit
        (
                TZWUnitInfo* const          punitiOwner
        );

        TTempSPUnit(const TTempSPUnit&) = delete;

        ~TTempSPUnit();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TTempSPUnit& operator=(const TTempSPUnit&) = delete;


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
        //  m_bDoXXXSP
        //      These are the bits that may or may not be implemented on a given
        //      thermostat. We set these in bParseExtraInfo() so that they are
        //      there for QueryUnitAttrs, which needs to only return attributes
        //      for the stuff we actually are doing.
        //
        //  m_bDoThermoMode
        //      The unit may or may not implement thermo mode control. We'll check for
        //      the CC and set this if so as a quick check for later.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bDoLowSP;
        tCIDLib::TBoolean   m_bDoHighSP;
        tCIDLib::TBoolean   m_bDoThermoMode;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TTempSPUnit, TZWaveUnit)
};

#pragma CIDLIB_POPPACK
