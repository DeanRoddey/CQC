//
// FILE NAME: CQCMedia_Type.hpp
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
//  This is the non-class types header for the facility. Some of them are in
//  the IDL generated shared header.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//  Facility types namespace
// ---------------------------------------------------------------------------
namespace tCQCMedia
{
    // -----------------------------------------------------------------------
    //  We can store art at 3 levels, and this is used to target a particular
    //  one in some places.
    // -----------------------------------------------------------------------
    enum class EArtLevels
    {
        Item
        , Collect
        , TitleSet
    };


    // -----------------------------------------------------------------------
    //  This is passed to a method of the facility class that returns one
    //  of a set of special images from the image repo
    // -----------------------------------------------------------------------
    enum class ESpcImages
    {
        PlayList
        , Missing

        , Count
    };



    // -----------------------------------------------------------------------
    //  Seme lists of things that we keep up with and/or take or return in
    //  interfaces.
    // -----------------------------------------------------------------------
    using TCharList     = TFundVector<tCIDLib::TCh>;
    using TCookieList   = tCIDLib::TStrList;
    using TCountList    = TFundVector<tCIDLib::TCard4>;
    using TIdList       = TFundVector<tCIDLib::TCard2>;
    using TNameList     = tCIDLib::TStrList;
    using TYearList     = TFundVector<tCIDLib::TCard4>;


    // -----------------------------------------------------------------------
    //  A structure shared between the Quick Time player class and the external
    //  app it has to spawn off to manager the actual player.
    // -----------------------------------------------------------------------
    struct TQTSharedInfo
    {
        tCIDLib::TBoolean           bMute;
        tCIDLib::TBoolean           bQTAvail;
        tCIDLib::TEncodedTime       enctDeadMan;
        tCIDLib::TCard4             c4InitVolume;
        tCIDLib::TCard4             c4Volume;
        tCQCMedia::EEndStates       eEndState;
        tCQCMedia::EMediaStates     eState;
        tCIDLib::TEncodedTime       enctCurPos;
        tCIDLib::TInt4              i4QTVersion;
        tCIDLib::TIPPortNum         ippnServer;

        tCIDLib::TCard4             c4OIDBytes;
        tCIDLib::TCard1             ac1OID[1];
    };
    using TQTSharedBuf = TTypedMBuf<TQTSharedInfo>;
}

#pragma CIDLIB_POPPACK


