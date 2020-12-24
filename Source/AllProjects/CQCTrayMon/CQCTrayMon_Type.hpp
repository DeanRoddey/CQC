//
// FILE NAME: CQCTrayMon_Type.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/14/2004
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
//  This is the non-object types header for this facility.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


namespace tCQCTrayMon
{
    // -----------------------------------------------------------------------
    //  The different types of objects we reference in the iTunes database.
    // -----------------------------------------------------------------------
    enum class EiTunesObjs
    {
        Source
        , PlayList
        , Track
    };


    // -----------------------------------------------------------------------
    //  This used in the app list event class, to indicate what type
    //  of change the object represents. These are used to keep the display
    //  updated.
    // -----------------------------------------------------------------------
    enum class EListEvs
    {
        None
        , Add
        , RecConfig
        , Remove
        , Status
    };


    // -----------------------------------------------------------------------
    //  Used when queuing up commands from the player control driver, to
    //  indicate what type of command it is.
    // -----------------------------------------------------------------------
    enum class EPlCmdTypes
    {
        None
        , PlCmd
        , SelPL
        , SelTrack
        , SetMute
        , SetVolume
    };


    // -----------------------------------------------------------------------
    //  The shell states we track (and report)
    // -----------------------------------------------------------------------
    enum class ESrvStates
    {
        NotFound
        , Stopping
        , Stopped
        , Starting
        , Started

        , Count
    };
}
