//
// FILE NAME: GC100Ser_Constant.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/12/2013
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
//  This is the constants header for this facility.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//  Toolkit constants namespace
// ---------------------------------------------------------------------------
namespace kGC100Ser
{
    // -----------------------------------------------------------------------
    //  Some GC related constants
    // -----------------------------------------------------------------------
    const tCIDLib::TIPPortNum   ippnBasePort = 4999;
    const tCIDLib::TCard4       c4MaxPortsPer = 8;


    // -----------------------------------------------------------------------
    //  Our unique port factory id
    // -----------------------------------------------------------------------
    const tCIDLib::TCh* const   pszFactoryId = L"27D3E3594BAA5E16-3477B3C9DFB0C7C2";
    const tCIDLib::TCh* const   pszPrefix = L"/GC100/";
}

#pragma CIDLIB_POPPACK

