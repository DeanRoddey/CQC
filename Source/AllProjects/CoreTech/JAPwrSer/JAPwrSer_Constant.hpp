//
// FILE NAME: JAPwrSer_Constant.hpp
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
namespace kJAPwrSer
{
    // -----------------------------------------------------------------------
    //  Some JAP related constants
    // -----------------------------------------------------------------------
    const tCIDLib::TIPPortNum   ippnBasePort = 6752;
    const tCIDLib::TCard4       c4MaxPortsPer = 1;


    // -----------------------------------------------------------------------
    //  Our unique port factory id
    // -----------------------------------------------------------------------
    const tCIDLib::TCh* const   pszFactoryId = L"AD345937562ACD3A-D61A0C997B1F01B5";
    const tCIDLib::TCh* const   pszPrefix = L"/JAPwr/";
}

#pragma CIDLIB_POPPACK

