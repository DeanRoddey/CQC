//
// FILE NAME: CQCHostMonC_Constant.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 05/22/2001
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
//  This is the constants header file for the CQC Host Monitor driver. Note
//  that this is also used by the server side driver, since it shares some
//  constants with us. So we cannot expose anything specific to our side of
//  the road here.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Facility constants
// ---------------------------------------------------------------------------
namespace kHostMonC
{
    // -----------------------------------------------------------------------
    //  General constants
    // -----------------------------------------------------------------------
    const tCIDLib::TCh* const   pszClientFacName    = L"CQCHostMonC";
    const tCIDLib::TCh* const   pszServerFacName    = L"CQCHostMonS";


    // -----------------------------------------------------------------------
    //  The version info for this driver (client and server sides, which we'll
    //  always keep in sync.)
    // -----------------------------------------------------------------------
    const tCIDLib::TCard4       c4MajVersion        = CID_MAJVER;
    const tCIDLib::TCard4       c4MinVersion        = CID_MINVER;
    const tCIDLib::TCard4       c4Revision          = CID_REVISION;


    // -----------------------------------------------------------------------
    //  The names of our fields that have fixed names.
    // -----------------------------------------------------------------------
    const tCIDLib::TCh* const   pszFld_CPUCount     = L"CPUCount";
    const tCIDLib::TCh* const   pszFld_CPUType      = L"CPUType";
    const tCIDLib::TCh* const   pszFld_HostName     = L"HostName";
    const tCIDLib::TCh* const   pszFld_OSVersion    = L"OSVersion";
    const tCIDLib::TCh* const   pszFld_PerMemUsed   = L"PercentMemUsed";
    const tCIDLib::TCh* const   pszFld_PhysicalMemK = L"PhysicalMemK";
}


