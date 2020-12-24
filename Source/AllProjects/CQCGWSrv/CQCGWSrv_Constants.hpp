//
// FILE NAME: CQCGWSrv_Constants.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/27/2002
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


#pragma CIDLIB_PACK(CIDLIBPACK)

namespace kCQCGWSrv
{
    // -----------------------------------------------------------------------
    //  If they don't set an explicit max number of simultaneous gateway
    //  clients, we will set it to this.
    // -----------------------------------------------------------------------
    constexpr tCIDLib::TCard4   c4DefMaxGWClients   = 4;


    // -----------------------------------------------------------------------
    //  The initial number of worker threads we'll spin up
    // -----------------------------------------------------------------------
    constexpr tCIDLib::TCard4   c4InitWorkerThreads = 4;


    // -----------------------------------------------------------------------
    //  The version of the protocol that we are currently implementing
    //
    //  Version history:
    //
    //      1.1 - Added support needed for .Net interface viewer, query of
    //            template list, template contents, and images.
    //
    //      1.2 - Added support for media calls.
    // -----------------------------------------------------------------------
    constexpr tCIDLib::TCard4   c4MajProtoVer       = 1;
    constexpr tCIDLib::TCard4   c4MinProtoVer       = 2;
}

#pragma CIDLIB_POPPACK

