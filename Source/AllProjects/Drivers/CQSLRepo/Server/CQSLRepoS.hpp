//
// FILE NAME: CQSLRepoS.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/02/2006
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
//  This is the main header file for the CQSL media repository driver. Like
//  most standard repos it is is based on the standard media DB which provides
//  all of the core data storage and query capabilities.
//
//  But, in our case, instead of just loading up pre-existing data, we have
//  to support a repository manager client that allows our data to be updated
//  in place. So we provide a separate ORB interface, in addition to the
//  standard driver backdoor command interface, that the repo manager can use
//  to talk to us in a much more structured way.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Include underlying needed headers.
// ---------------------------------------------------------------------------
#include    "CIDOrb.hpp"
#include    "CIDOrbUC.hpp"
#include    "CIDCrypto.hpp"
#include    "CIDXML.hpp"
#include    "CIDJPEG.hpp"
#include    "CIDImgFact.hpp"

#include    "CQCKit.hpp"
#include    "CQCDriver.hpp"
#include    "CQCMedia.hpp"
#include    "CQCMedia_RepoUpload.hpp"


// ---------------------------------------------------------------------------
//  Include our own header files in the needed order
// ---------------------------------------------------------------------------
#include    "CQSLRepoS_ErrorIds.hpp"
#include    "CQSLRepoS_MessageIds.hpp"


// ---------------------------------------------------------------------------
//  Facility types
// ---------------------------------------------------------------------------
namespace tCQSLRepoS
{
}


// ---------------------------------------------------------------------------
//  Some more of our headers that need to see the types and constants
// ---------------------------------------------------------------------------
#include    "CQSLRepoS_CQSLRepoServerBase.hpp"
#include    "CQSLRepoS_DriverImpl.hpp"
#include    "CQSLRepoS_MgrImpl.hpp"



// ---------------------------------------------------------------------------
//  Facility constants
// ---------------------------------------------------------------------------
namespace kCQSLRepoS
{
    // -----------------------------------------------------------------------
    //  General constants
    // -----------------------------------------------------------------------
    const tCIDLib::TCh* const   pszServerFacName= L"CQSLRepoS";


    // -----------------------------------------------------------------------
    //  The version info for this driver, which we just set to the CQC
    //  version.
    // -----------------------------------------------------------------------
    const tCIDLib::TCard4       c4MajVersion    = CID_MAJVER;
    const tCIDLib::TCard4       c4MinVersion    = CID_MINVER;
    const tCIDLib::TCard4       c4Revision      = CID_REVISION;


    // -----------------------------------------------------------------------
    //  We break the per-media type directories up into sub-dirs so that no
    //  one directory get's too big. This is the max number of titles that
    //  we'll put in one of these sub-directories.
    // -----------------------------------------------------------------------
    const tCIDLib::TCard4       c4MaxObjsPerDir = 500;


    // -----------------------------------------------------------------------
    //  The current file format version number for our .mimg2 files.
    // -----------------------------------------------------------------------
    const tCIDLib::TCard2       c2FmtImages = 1;


    // -----------------------------------------------------------------------
    //  The name of our main metadata DB file, which is stored in the root of
    //  the repo.
    // -----------------------------------------------------------------------
    const tCIDLib::TCh* const   pszDBFileName = L"CQSLRepo.MetaDB";
}



// ---------------------------------------------------------------------------
//  Global functions. These aren't exported. They are just for internal use.
// ---------------------------------------------------------------------------
extern TFacility& facCQSLRepoS();

