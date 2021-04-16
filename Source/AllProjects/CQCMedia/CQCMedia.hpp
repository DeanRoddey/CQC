//
// FILE NAME: CQCMedia.hpp
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
//  This facility defines the media browsing and control interfaces for CQC.
//  Two primary abstract interfaces are defined, for base media browsing, and
//  (for devices that are repositories and players) browsing plus transport
//  and status info.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


// ---------------------------------------------------------------------------
//  Set up our import/export attributes
// ---------------------------------------------------------------------------
#if     defined(PROJ_CQCMEDIA)
#define CQCMEDIAEXPORT  CID_DLLEXPORT
#else
#define CQCMEDIAEXPORT  CID_DLLIMPORT
#endif


// ---------------------------------------------------------------------------
//  Include our underlying headers
// ---------------------------------------------------------------------------
#include    "CIDLib.hpp"
#include    "CIDMath.hpp"

#include    "CIDXML.hpp"
#include    "CQCKit.hpp"
#include    "CQCDriver.hpp"



// ---------------------------------------------------------------------------
//  Forward references
// ---------------------------------------------------------------------------
class TBitmapImage;
class TJPEGImage;


// ---------------------------------------------------------------------------
//  Include our headers, in the needed order.
// ---------------------------------------------------------------------------
#include    "CQCMedia_ErrorIds.hpp"
#include    "CQCMedia_MessageIds.hpp"
#include    "CQCMedia_Shared.hpp"
#include    "CQCMedia_Type.hpp"
#include    "CQCMedia_Constant.hpp"
#include    "CQCMedia_DBInfo.hpp"
#include    "CQCMedia_PLItem.hpp"

//
//  We need some collections of the various types of things we store, and
//  these need to be seen by the headers below.
//
namespace tCQCMedia
{
    // Image lists by id and persistent id, and hashed by unique id
    using TImgIdList   = TRefVector<const TMediaImg>;
    using TNCImgIdList = TRefVector<TMediaImg>;
    using TImgNameList = TRefKeyedHashSet<TMediaImg,TString,TStringKeyOps>;

    // Item lists, by id and by unique id
    using TItemIdList   = TRefVector<const TMediaItem>;
    using TNCItemIdList = TRefVector<TMediaItem>;
    using TItemNameList = TRefKeyedHashSet<TMediaItem,TString,TStringKeyOps>;

    // Collection lists, by id and by unique id or name
    using TColIdList   = TRefVector<const TMediaCollect>;
    using TNCColIdList = TRefVector<TMediaCollect>;
    using TColNameList = TRefKeyedHashSet<TMediaCollect,TString,TStringKeyOps>;


    // Title set lists, by id and by unique id
    using TSetIdList   = TRefVector<const TMediaTitleSet>;
    using TNCSetIdList = TRefVector<TMediaTitleSet>;
    using TSetNameList = TRefKeyedHashSet<TMediaTitleSet,TString,TStringKeyOps>;

    // For just editing lists of items before commiting changes
    using TItemList = TVector<TMediaItem>;

    // A list of card4 values
    using TMediaIdList = TFundVector<tCIDLib::TCard2>;

    // A list of artist map objects
    using TByArtistList = TRefVector<TArtistMap>;
}

#include    "CQCMedia_Database.hpp"
#include    "CQCMedia_CookieFldFilter.hpp"
#include    "CQCMedia_StdRendDrv.hpp"
#include    "CQCMedia_StdRepoDrvEng.hpp"
#include    "CQCMedia_StdRepoDrv.hpp"
#include    "CQCMedia_AudioPlayer.hpp"
#include    "CQCMedia_WMPAudioPlayer.hpp"
#include    "CQCMedia_WMPMetaSrc.hpp"
#include    "CQCMedia_ThisFacility.hpp"


// ---------------------------------------------------------------------------
//  Export the facility object lazy eval method
// ---------------------------------------------------------------------------
extern CQCMEDIAEXPORT TFacCQCMedia& facCQCMedia();

