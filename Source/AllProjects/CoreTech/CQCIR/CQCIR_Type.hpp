//
// FILE NAME: CQCIR_Type.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/29/2003
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
//  This is the non-class types header for the facility.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//  Toolkit types namespace
// ---------------------------------------------------------------------------
namespace tCQCIR
{
    // -----------------------------------------------------------------------
    //  Device capabilities that we can report for each of the devices
    // -----------------------------------------------------------------------
    enum class EIRDevCaps : tCIDLib::TCard2
    {
        None            = 0x0000
        , Blast         = 0x0001
        , BlastTrain    = 0x0002
        , Receive       = 0x0004
    };

    CQCIREXPORT EIRDevCaps eXlatEIRDevCaps
    (
        const   TString&                strToXlat
    );
}

#pragma CIDLIB_POPPACK

EnumBinStreamMacros(tCQCIR::EIRDevCaps)

