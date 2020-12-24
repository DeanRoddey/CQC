//
// FILE NAME: ZWaveUSB3Sh_CCHelp_Wakeup.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/20/2018
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
//  Wakeup class.
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
//   NAMESPACE: TZWUSB3CCWakeup
// ---------------------------------------------------------------------------
namespace TZWUSB3CCWakeup
{
    // Query the wakeup interval and target node, if any
    ZWUSB3SHEXPORT tCIDLib::TBoolean bQueryWakeup
    (
                TZWUnitInfo&            unitiSrc
        ,       tCIDLib::TCard1&        c1ReportTo
        ,       tCIDLib::TCard4&        c4Interval
        , const tCIDLib::TBoolean       bAsync
        , const tCIDLib::TBoolean       bKnownAwake
        ,       TString&                strErrMsg
    );


    // Set the wakeup interval and node to report to
    ZWUSB3SHEXPORT tCIDLib::TBoolean bSetWakeup
    (
                TZWUnitInfo&            unitiSrc
        , const tCIDLib::TCard1         c1ReportTo
        , const tCIDLib::TCard4         c4Interval
        , const tCIDLib::TBoolean       bAsync
        , const tCIDLib::TBoolean       bKnownAwake
        ,       TString&                strErrMsg
    );
}

#pragma CIDLIB_POPPACK

