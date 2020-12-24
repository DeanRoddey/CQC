//
// FILE NAME: GenProtoS_Constant.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/24/2003
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
//  This is the constants header for the generic protocol server driver.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


namespace kGenProtoS
{
    // -----------------------------------------------------------------------
    //  These are the base names of our client and server side driver
    //  facilities.
    // -----------------------------------------------------------------------
    const tCIDLib::TCh* const   pszServerFacName    = L"GenProtoS";


    // -----------------------------------------------------------------------
    //  The version info for this driver, which we as usual set to the release
    //  version.
    // -----------------------------------------------------------------------
    const tCIDLib::TCard4       c4MajVersion        = CID_MAJVER;
    const tCIDLib::TCard4       c4MinVersion        = CID_MINVER;
    const tCIDLib::TCard4       c4Revision          = CID_REVISION;


    // -----------------------------------------------------------------------
    //  We support the ability to look for message start/term delimiters and
    //  this is the max bytes we can accept for a start or term delimiter
    //  byte sequence.
    // -----------------------------------------------------------------------
    const tCIDLib::TCard4       c4MaxSeqBytes       = 4;


    // -----------------------------------------------------------------------
    //  The maximum milliseconds that the minimum send interval can be set
    //  to. This is used to throttle the speed we can send to the device.
    // -----------------------------------------------------------------------
    const tCIDLib::TCard4   c4MaxMinSendInterval    = 2000;


    // -----------------------------------------------------------------------
    //  We let the protocol tell us to be a little lenient of timeouts and
    //  wait until we see a given number of timeouts before we give up. This
    //  is the maximum value we will accept for the maximum timeouts value.
    // -----------------------------------------------------------------------
    const tCIDLib::TCard4   c4MaxMaxTimeouts        = 16;
}


