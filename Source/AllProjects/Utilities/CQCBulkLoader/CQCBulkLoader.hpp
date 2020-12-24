//
// FILE NAME: CQCBulkLoader.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/15/2010
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
//  This is the main header for the the CQC bulk loader program. This program
//  will read a text file which describes drivers, images, and possibly other
//  things over time, to load into a CQC system.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//

// ----------------------------------------------------------------------------
//  Bring in required underlying headers. We bring in some from CQCKit that
//  it doesn't publically export since not many folks use them and we need
//  to limit the impact of that fundamental facility.
// ----------------------------------------------------------------------------
#include    "CIDXML.hpp"

#include    "CIDPNG.hpp"

#include    "CIDJPEG.hpp"

#include    "CIDOrbUC.hpp"

#include    "CQCKit.hpp"

#include    "CQCDriver.hpp"

#include    "CQCRemBrws.hpp"

#include    "CQCEvCl.hpp"


// ----------------------------------------------------------------------------
//  Facility types and constants
// ----------------------------------------------------------------------------
namespace tCQCBulkLoader
{
    // -------------------------------------------------------------------
    //  Each image can either have no transparency info (or it's alpha
    //  based and in the image itself), or it can be color based with an
    //  explicit color indicated, or color based and a point within the
    //  image indicated that we need to pull out and use as the
    //  color transparecy based color.
    // -------------------------------------------------------------------
    enum EImgTrans
    {
        EImgTrans_None
        , EImgTrans_Color
        , EImgTrans_SrcClr
    };
}


// ----------------------------------------------------------------------------
//  Bring in our own headers
// ----------------------------------------------------------------------------
#include    "CQCBulkLoader_CfgInfo.hpp"
#include    "CQCBulkLoader_ThisFacility.hpp"


// ----------------------------------------------------------------------------
//  Our one global object, the facility object
// ----------------------------------------------------------------------------
extern TFacCQCBulkLoader  facCQCBulkLoader;

