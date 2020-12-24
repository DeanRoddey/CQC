//
// FILE NAME: DevClsHdr_Security.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/15/2014
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
//  This header contains some constants and types used by the Security device class.
//  Like all of these headers, it is not included anywhere normally, so only those
//  few things that need it will include it.
//
//  See the device class documentaion for what this stuff means. We don't want to
//  try to replicate that here.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//

namespace kDevCls_Security
{
    const tCIDLib::TCh* const   pszSecQN_SecPanelInfo   = L"QSecPanelInfo";
    const tCIDLib::TCh* const   pszSecQT_AreaList       = L"SecAreaList";
    const tCIDLib::TCh* const   pszSecQT_SecArmInfo     = L"SecArmInfo";

    const tCIDLib::TCh* const   pszSecQN_AreaZones      = L"QAreaZones";
    const tCIDLib::TCh* const   pszSecQN_AreaZStats     = L"QAreaZStats";
    const tCIDLib::TCh* const   pszSecQN_AreaAlarms     = L"QAreaAlarms";
}

