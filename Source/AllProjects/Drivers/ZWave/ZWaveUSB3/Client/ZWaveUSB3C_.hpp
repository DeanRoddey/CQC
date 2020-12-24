//
// FILE NAME: ZWaveUSB3C_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/05/2018
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
//  Though we don't technically need an internal header for a driver, since
//  it is never imported anywhere directly, we generate ORB based classes and
//  they always follow the letter of the law and use the internal facility
//  header.
//
//  So we just need to include the public header.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once

#include "ZWaveUSB3C.hpp"
#include    "CIDZLib.hpp"
