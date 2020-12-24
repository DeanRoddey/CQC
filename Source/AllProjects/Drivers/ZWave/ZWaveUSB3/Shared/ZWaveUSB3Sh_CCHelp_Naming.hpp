//
// FILE NAME: ZWaveUSB3Sh_CCNaming.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/21/2017
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
//  This is a CC helper namespace to provide some grunt work helpers for the Node
//  Naming class.
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
//   NAMESPACE: TZWUSB3CCNaming
// ---------------------------------------------------------------------------
namespace TZWUSB3CCNaming
{
    ZWUSB3SHEXPORT tCIDLib::TBoolean bExtractNodeName
    (
        const   TZWInMsg&               zwimSrc
        ,       TString&                strToFill
    );

    //
    //  If not async, and it's not a live listener, and bKnownAwake is false, an
    //  exception will occur.
    //
    ZWUSB3SHEXPORT tCIDLib::TBoolean bQueryUnitName
    (
                TZWUnitInfo&            unitiTar
        , const tCIDLib::TBoolean       bAsync
        , const tCIDLib::TBoolean       bKnownAwake
        ,       TString&                strToFill
        ,       TString&                strErrMsg
    );
}

#pragma CIDLIB_POPPACK
