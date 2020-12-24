//
// FILE NAME: VarDriverC_Constant.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 12/05/2003
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
//  This is the constants header file for the CQC variable driver.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Facility constants
// ---------------------------------------------------------------------------
namespace kVarDriverC
{
    // -----------------------------------------------------------------------
    //  General constants
    // -----------------------------------------------------------------------
    const tCIDLib::TCh* const   pszClientFacName    = L"VarDriverC";
    const tCIDLib::TCh* const   pszServerFacName    = L"VarDriverS";


    // -----------------------------------------------------------------------
    //  Some command and query strings we send to the server side driver in
    //  backdoor commands. These are used when setting or querying field
    //  config, setting default field values, and querying info on the
    //  currently selected field.
    // -----------------------------------------------------------------------
    const tCIDLib::TCh* const   pszCfg_Full         = L"FullCfg";
    const tCIDLib::TCh* const   pszCfg_SetDefault   = L"SetFldDefault";
    const tCIDLib::TCh* const   pszCfg_CurFldVals   = L"CurFldVals";


    // -----------------------------------------------------------------------
    //  The version info for this driver which we just set to the CQC release
    //  version.
    // -----------------------------------------------------------------------
    const tCIDLib::TCard4       c4MajVersion        = CID_MAJVER;
    const tCIDLib::TCard4       c4MinVersion        = CID_MINVER;
    const tCIDLib::TCard4       c4Revision          = CID_REVISION;
}


