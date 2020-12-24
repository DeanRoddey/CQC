//
// FILE NAME: CQSLRepoImp_DVDPLoader.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 12/25/2006
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
//  This is the header for the DVD Profiler loader class. This is a slightly
//  modified ripoff of the loader from the DVD Profiler driver. As there,
//  it just loads up an in-memory database. So by the time we start doing
//  the import, we know that it's going to parse successfully and is legal
//  enough to be accepted by the database.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)

namespace TRepoImpHelpers
{
    extern tCIDLib::TBoolean bGetFile
    (
        const   TString&                    strPrompt
        ,       TString&                    strFile
        , const tCIDLib::TBoolean           bMustExist
    );

    extern tCIDLib::TBoolean bGetPath
    (
        const   TString&                    strPrompt
        ,       TString&                    strPath
        , const tCIDLib::TBoolean           bMustExist
    );

    extern tCIDLib::TBoolean bGetRepo
    (
        const   TString&                    strPrompt
        ,       TString&                    strMoniker
        ,       tCQSLRepoImp::TRepoClient&  orbcRepoMgr
    );

    extern tCIDLib::TBoolean bGetYNAnswer
    (
        const   TString&                    strQ
    );

    extern tCIDLib::TCard4 c4GetIndex
    (
        const   TString&                    strPrompt
        , const tCIDLib::TCard4             c4Min
        , const tCIDLib::TCard4             c4Max
    );
}

#pragma CIDLIB_POPPACK

