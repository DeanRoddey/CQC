//
// FILE NAME: CQCIGKit_Constant.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/19/2004
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
#pragma once


namespace kCQCIGKit
{
    // -----------------------------------------------------------------------
    //  Some keys that we use with the TFacCQCKit::LoadLastPath/StoreLastPath
    //  method, to save/and reload some last used paths since we provide a
    //  few different browsing type interfaces.
    // -----------------------------------------------------------------------
    constexpr const tCIDLib::TCh* const   pszLastPath_GlobAct   = L"LastGlobAct";
    constexpr const tCIDLib::TCh* const   pszLastPath_RepoImage = L"LastRepoImage";
    constexpr const tCIDLib::TCh* const   pszLastPath_Macro     = L"LastMacro";
    constexpr const tCIDLib::TCh* const   pszLastPath_Template  = L"LastTemplate";
    constexpr const tCIDLib::TCh* const   pszLastPath_LastImport= L"LastImportPath";


    // -----------------------------------------------------------------------
    //  Driver related constants
    // -----------------------------------------------------------------------
    constexpr const tCIDLib::TSCh* const  pszClientFunc       = "MakeClientWindow";
}

