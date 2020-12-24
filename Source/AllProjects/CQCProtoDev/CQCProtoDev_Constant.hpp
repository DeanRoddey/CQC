//
// FILE NAME: CQCProtoDev_Constant.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/27/2003
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
//  This is the header for the facility class of this facility.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


namespace kCQCProtoDev
{
    // -----------------------------------------------------------------------
    //  Keys used for local config storage.  The last path ones are not full
    //  paths, just ids that are put into a standard last path scope by a
    //  helper method.
    // -----------------------------------------------------------------------
    const tCIDLib::TCh* const pszStoreKey               = L"ProtoDev";
    const tCIDLib::TCh* const pszCfgKey_Frame           = L"/Wnd/MainFrameState";
    const tCIDLib::TCh* const pszCfgKey_LastManPath     = L"LastManPath";
    const tCIDLib::TCh* const pszCfgKey_LastProtoPath   = L"LastProtoPath";
}

