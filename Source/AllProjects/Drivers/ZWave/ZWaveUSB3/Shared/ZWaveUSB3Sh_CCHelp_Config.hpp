//
// FILE NAME: ZWaveUSB3Sh_CCHelp_Config.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/18/2018
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
//  Configuration class.
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
//   NAMESPACE: TZWUSB3CCConfig
// ---------------------------------------------------------------------------
namespace TZWUSB3CCConfig
{
    // Query the value of a config parameter
    ZWUSB3SHEXPORT tCIDLib::TBoolean bQueryParam
    (
                TZWUnitInfo&            unitiSrc
        , const tCIDLib::TCard1         c1ParamNum
        ,       tCIDLib::TInt4&         i4Value
        , const tCIDLib::TBoolean       bAsync
        , const tCIDLib::TBoolean       bKnownAwake
        ,       TString&                strErrMsg
    );

    // Set a particular parameter to a particular value
    ZWUSB3SHEXPORT tCIDLib::TBoolean bSetParam
    (
                TZWUnitInfo&            unitiTar
        , const tCIDLib::TCard1         c1ParamNum
        , const tCIDLib::TCard4         c4ByteCnt
        , const tCIDLib::TInt4          i4Value
        , const tCIDLib::TBoolean       bAsync
        , const tCIDLib::TBoolean       bKnownAwake
        ,       TString&                strErrMsg
    );
}

#pragma CIDLIB_POPPACK

