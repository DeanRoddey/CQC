//
// FILE NAME: CQCMedia_Constant.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/13/2005
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
//  The facility header for constants. Note that many of these are defined
//  in the CQCMedia_Shared.CIDIDL IDL file, so that they can be generated for
//  non-C++ clients.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


namespace kCQCMedia
{
    // -----------------------------------------------------------------------
    //  The version of the browsing protocol we support
    // -----------------------------------------------------------------------
    constexpr tCIDLib::TCard4   c4ProtoVer = 1;


    // -----------------------------------------------------------------------
    //  The maximum number of tokens in a media cookie. This must stay in
    //  sync with the ECookieTypes enum!
    // -----------------------------------------------------------------------
    constexpr tCIDLib::TCard4   c4MaxCookieTokens = 5;


    // -----------------------------------------------------------------------
    //  Our min/max user rating values
    // -----------------------------------------------------------------------
    constexpr tCIDLib::TCard4   c4MinUserRating = 1;
    constexpr tCIDLib::TCard4   c4MaxUserRating = 10;


    // -----------------------------------------------------------------------
    //  Some flags passed into the standard renderer driver's method that
    //  returns a list of field defs that all media renderers should support.
    // -----------------------------------------------------------------------
    constexpr tCIDLib::TCard4   c4RendFlag_None     = 0x0000;
    constexpr tCIDLib::TCard4   c4RendFlag_Seekable = 0x0001;


    // -----------------------------------------------------------------------
    //  The current format for the binary and XML media data base dumps that
    //  we provide to the client service.
    // -----------------------------------------------------------------------
    constexpr tCIDLib::TCard2   c2BinMDBDumpFmtVer  = 1;
    constexpr tCIDLib::TCard2   c2XMLMDBDumpFmtVer  = 1;


    // -----------------------------------------------------------------------
    //  The standard size that we drivers can use to create automatically scaled
    //  cover art thumbs, by scaling down the large art.
    // -----------------------------------------------------------------------
    constexpr tCIDLib::TCard4   c4DefThumbSz = 98;


    // -----------------------------------------------------------------------
    //  The max size of large art that drivers should generally cache. If it's
    //  bigger than this they should scale it. It would be nice if the repo driver
    //  using us would write out this scaled version if that happens, and load
    //  that next time, to avoid the overhead.
    // -----------------------------------------------------------------------
    constexpr tCIDLib::TCard4   c4MaxLrgArtSz = 480;
};

