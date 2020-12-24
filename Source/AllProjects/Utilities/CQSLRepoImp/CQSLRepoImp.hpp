//
// FILE NAME: CQSLRepoImp.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 12/15/2006
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
//  This is the main header for the program. It brings any needed underlying
//  headers and internal headers.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CIDJPEG.hpp"
#include    "CIDXML.hpp"
#include    "CIDOrbUC.hpp"

#include    "CQCMedia.hpp"
#include    "CQSLRepoImp_CQSLRepoClientProxy.hpp"



namespace   tCQSLRepoImp
{
    typedef TCntPtr<TCQSLRepoMgrClientProxy>    TRepoClient;
}

#include    "CQSLRepoImp_Helpers.hpp"
#include    "CQSLRepoImp_DVDPLoader.hpp"
#include    "CQSLRepoImp_JRLoader.hpp"


// Global data
extern tCIDLib::TBoolean    bTestMode;
extern tCIDLib::TCard4      c4ErrCnt;
extern tCIDLib::TCard4      c4MissingArtCnt;
extern tCIDLib::TCard4      c4NoDiscCnt;
extern tCIDLib::TCard4      c4SkippedCnt;
extern TInConsole*          pconIn;
extern TOutConsole*         pconOut;

