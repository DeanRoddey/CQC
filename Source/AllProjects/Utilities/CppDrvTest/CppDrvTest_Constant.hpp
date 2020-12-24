//
// FILE NAME: CppDrvTest_Constant.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/26/2018
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
//  This is the constants header for the CppDrvTest facility.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//

namespace kCppDrvTest
{
    // -----------------------------------------------------------------------
    //  The application specific driver key that we use to open our standard, local
    //  user/application specific object store.
    // -----------------------------------------------------------------------
    const TString strStoreKey          = L"CppDriverDev";

    // -----------------------------------------------------------------------
    //  Keys we store state info under, using the standard helper methods for accessing
    //  the local user/application specific object store.
    // -----------------------------------------------------------------------
    const TString strCfgKey_Frame      = L"/Wnd/MainFrameState";
    const TString strCfgKey_ChatState  = L"/Wnd/ChatState";
    const TString strCfgKey_LastRemEP  = L"/RPort/LastEndPoint";
}
