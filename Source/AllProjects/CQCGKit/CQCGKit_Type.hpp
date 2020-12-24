//
// FILE NAME: CQCGKit_Type.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 05/11/2001
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
//  This is the non-class types header for the facility.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)


class TCQCDriverFrameWnd;


// ---------------------------------------------------------------------------
//  Toolkit types namespace
// ---------------------------------------------------------------------------
namespace tCQCGKit
{
    // -----------------------------------------------------------------------
    //  All client drivers basic have three possible connection states. They
    //  are either connected to the CQCServer that is running their server,
    //  or connected to the server but their device is offline, or they are
    //  not connected to their server side object.
    //
    //  Ok, a very few don't talk to a device, such as the CQCHostMon driver,
    //  but they are rare and just won't use the DevOffline state.
    // -----------------------------------------------------------------------
    enum class EConnStates
    {
        Connected
        , DevOffline
        , SrvOffline

        , Count
        , Min           = Connected
        , Max           = SrvOffline
    };


    // -----------------------------------------------------------------------
    //  The logon dialog supports two modes, one for initial logon, and
    //  another for re-logon after the security server goes down and comes
    //  back up.
    // -----------------------------------------------------------------------
    enum class ELogonModes
    {
        Initial
        , Relogon
    };
}

#pragma CIDLIB_POPPACK

