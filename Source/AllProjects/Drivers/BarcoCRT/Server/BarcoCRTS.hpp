//
// FILE NAME: BarcoCRTS.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/13/2002
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
//  This is the main file for the Barco CRT projector server driver.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//




// ---------------------------------------------------------------------------
//  Include underlying needed headers.
// ---------------------------------------------------------------------------
#include    "CIDComm.hpp"

#include    "CQCKit.hpp"

#include    "CQCDriver.hpp"


// ---------------------------------------------------------------------------
//  Facility constants
// ---------------------------------------------------------------------------
namespace kBarcoCRTS
{
    // -----------------------------------------------------------------------
    //  These are the base name of our facility
    // -----------------------------------------------------------------------
    const tCIDLib::TCh* const   pszServerFacName    = L"BarcoCRTS";


    // -----------------------------------------------------------------------
    //  The version info for this driver. These values come in from the build
    //  environment because all of our drivers are given a version number
    //  equal to the CQC release they are in.
    // -----------------------------------------------------------------------
    const tCIDLib::TCard4       c4MajVersion        = CID_MAJVER;
    const tCIDLib::TCard4       c4MinVersion        = CID_MINVER;
    const tCIDLib::TCard4       c4Revision          = CID_REVISION;


    // -----------------------------------------------------------------------
    //  The names of our fields
    // -----------------------------------------------------------------------
    const tCIDLib::TCh* const   pszFld_HorzPeriod   = L"HorzPeriod";
    const tCIDLib::TCh* const   pszFld_Input        = L"Input";
    const tCIDLib::TCh* const   pszFld_Power        = L"Power";
    const tCIDLib::TCh* const   pszFld_SigType      = L"SigType";
    const tCIDLib::TCh* const   pszFld_VertRate     = L"VertRate";
}


// ---------------------------------------------------------------------------
//  Facility types namespace
// ---------------------------------------------------------------------------
namespace tBarcoCRTS
{
    // -----------------------------------------------------------------------
    //  Used in the map arrays below, to map between the text and numeric
    //  values of some Barco values.
    // -----------------------------------------------------------------------
    struct TMapItem
    {
        tCIDLib::TCard1     c1Value;
        const tCIDLib::TCh* pszName;
        tCIDLib::TCard4     c4OptVal;

    };


    // -----------------------------------------------------------------------
    //  The types of projectors that the protocol can tell us about when we
    //  do the version query. We use the optional value field to store the
    //  base type, since generally that's all we care about internally.
    // -----------------------------------------------------------------------
    const TMapItem aitemPTypes[] =
    {
        {  0, L"800"      , 800     }
      , {  1, L"801"      , 800     }
      , {  2, L"1200"     , 1200    }
      , {  3, L"1100"     , 1100    }
      , {  4, L"1600"     , 1600    }
      , {  5, L"700"      , 700     }
      , {  6, L"1208"     , 1200    }
      , {  7, L"1101"     , 1100    }
      , {  8, L"500"      , 500     }
      , {  9, L"808"      , 800     }
      , { 10, L"1209"     , 1200    }
      , { 11, L"701"      , 700     }
      , { 12, L"RCVDS"    , 0       }
      , { 13, L"801"      , 800     }
      , { 14, L"1208/2"   , 1200    }
      , { 15, L"V701"     , 700     }
      , { 16, L"801"      , 800     }
      , { 17, L"1609"     , 1600    }
      , { 18, L"1109"     , 1100    }
      , { 19, L"V701"     , 700     }
      , { 20, L"1208S"    , 1200    }
      , { 21, L"1209/2"   , 1200    }
      , { 22, L"1109"     , 1100    }
      , { 23, L"808"      , 800     }
      , { 24, L"808s"     , 800     }
      , { 25, L"701"      , 700     }
      , { 26, L"1208s/2"  , 1200    }
      , { 27, L"1209/3"   , 1200    }
      , { 28, L"1109/3"   , 1100    }
      , { 29, L"1609/3"   , 1600    }
      , { 30, L"R808s"    , 800     }
      , { 31, L"V508"     , 500     }
    };
    const tCIDLib::TCard4 c4PTypeCount = tCIDLib::c4ArrayElems(aitemPTypes);


    // -----------------------------------------------------------------------
    //  The signal types that the projector supports. We have different tables
    //  for different projector types.
    //
    //  Table1=800,1100,1200,1600
    //  Table2=700
    // -----------------------------------------------------------------------
    const TMapItem aitemSigTypes1[] =
    {
        {  0 , L"Video"       , 0 }
      , {  1 , L"SVHS"        , 0 }
      , {  2 , L"TTL"         , 0 }
      , {  3 , L"RGsB"        , 0 }
      , {  4 , L"RGBS"        , 0 }
      , {  5 , L"Internal"    , 0 }
      , {  8 , L"ForcedVideo" , 0 }
      , {  9 , L"ForcedSVHS"  , 0 }
      , { 10 , L"ForcedTTL"   , 0 }
      , { 11 , L"ForcedRGsB"  , 0 }
      , { 12 , L"ForcedRGBS"  , 0 }
    };
    const tCIDLib::TCard4 c4SigTypeCount1 = tCIDLib::c4ArrayElems(aitemSigTypes1);

    const TMapItem aitemSigTypes2[] =
    {
        {  0 , L"Video"       , 0 }
      , {  1 , L"S-Video"     , 0 }
      , {  2 , L"RGB"         , 0 }
      , {  3 , L"RGB3S"       , 0 }
      , {  4 , L"Component"   , 0 }
      , {  7 , L"Internal"    , 0 }
    };
    const tCIDLib::TCard4 c4SigTypeCount2 = tCIDLib::c4ArrayElems(aitemSigTypes2);
}


// ---------------------------------------------------------------------------
//  Include our own header files in the needed order
// ---------------------------------------------------------------------------
#include    "BarcoCRTS_ErrorIds.hpp"
#include    "BarcoCRTS_MessageIds.hpp"
#include    "BarcoCRTS_Type.hpp"
#include    "BarcoCRTS_Constant.hpp"
#include    "BarcoCRTS_Msg.hpp"
#include    "BarcoCRTS_DriverImpl.hpp"


// ---------------------------------------------------------------------------
//  Export our facility lazy eval method.
// ---------------------------------------------------------------------------
extern TFacility& facBarcoCRTS();

