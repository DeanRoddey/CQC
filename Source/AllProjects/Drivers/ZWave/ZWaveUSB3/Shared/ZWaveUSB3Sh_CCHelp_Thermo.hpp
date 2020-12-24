//
// FILE NAME: ZWaveUSB3Sh_CCHelp_Thermo.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/26/2018
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
//  This is a CC helper namespace to provide some grunt work helpers for the
//  thermostat related command classes.
//
//  These helper namespaces are for utility type CCs, not for those that implement
//  actual automation functionality, or to implement the utility aspects of CCs that
//  may also be implemented as a CCImpl type class.
//
// CAVEATS/GOTCHAS:
//
//
// LOG:
//
#pragma once

#pragma CIDLIB_PACK(CIDLIBPACK)



// ---------------------------------------------------------------------------
//   NAMESPACE: TZWUSB3CCThermo
// ---------------------------------------------------------------------------
namespace TZWUSB3CCThermo
{
    // Get the supported modes and fan modes
    ZWUSB3SHEXPORT tCIDLib::TBoolean bQueryModes
    (
                TZWUnitInfo&                unitiTar
        ,       tCIDLib::TCard4&            c4FanModes
        ,       tCIDLib::TCard4&            c4Modes
        , const tCIDLib::TBoolean           bAsync
        , const tCIDLib::TBoolean           bKnownAwake
        ,       TString&                    strErrMsg
    );

    // Get the list of set points supported
    ZWUSB3SHEXPORT tCIDLib::TBoolean bQuerySPs
    (
                TZWUnitInfo&                unitiTar
        ,       tCIDLib::TCard4&            c4SPMask
        , const tCIDLib::TBoolean           bAsync
        , const tCIDLib::TBoolean           bKnownAwake
        ,       TString&                    strErrMsg
    );

    //
    //  Sets up the standard unit option attributes for the current temp of a
    //  thermostat and also for most other temp sensors via MLSensor. It assumes the
    //  standard temp scale and min/max limits unit options are set for the unit.
    //
    //  We assume the caller already added any section separator
    //
    ZWUSB3SHEXPORT tCIDLib::TVoid QueryTempAttrs
    (
        const   TZWUnitInfo&                unitiSrc
        ,       tCIDMData::TAttrList&       colToFill
        ,       TAttrData&                  adatTmp
    );
}

#pragma CIDLIB_POPPACK

