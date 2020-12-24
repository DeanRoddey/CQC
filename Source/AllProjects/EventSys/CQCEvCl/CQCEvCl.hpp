//
// FILE NAME: CQCKit.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/24/2015
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
//  This is the main header of the event related client facility.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


// ---------------------------------------------------------------------------
//  Include our underlying headers
// ---------------------------------------------------------------------------
#include    "CQCKit.hpp"
#include    "CQCRemBrws.hpp"


// ---------------------------------------------------------------------------
//  Set up our import/export attributes
// ---------------------------------------------------------------------------
#if     defined(PROJ_CQCEVCL)
#define CQCEVCLEXPORT    CID_DLLEXPORT
#else
#define CQCEVCLEXPORT    CID_DLLIMPORT
#endif


// ---------------------------------------------------------------------------
//  Include our headers, in the needed order.
// ---------------------------------------------------------------------------
#include    "CQCEvCl_ErrorIds.hpp"
#include    "CQCEvCl_MessageIds.hpp"
#include    "CQCEvCl_EventSrvClientProxy.hpp"

namespace tCQCEvCl
{
    using TEvMonList = TVector<TCQCEvMonCfg>;
    using TSchEvList = TVector<TCQCSchEvent>;
    using TTrgEvList = TVector<TCQCTrgEvent>;

    using TKeyedEvMonList = TVector<TKeyedCQCEvMonCfg>;
    using TKeyedSchEvList = TVector<TKeyedCQCSchEvent>;
    using TKeyedTrgEvList = TVector<TKeyedCQCTrgEvent>;


    // A counted pointer over the event server client proxy
    using TEventSrvProxy = TCntPtr<TEventSrvClientProxy>;
}

#include    "CQCEvCl_ThisFacility.hpp"


// ---------------------------------------------------------------------------
//  Export the facility object lazy eval method
// ---------------------------------------------------------------------------
extern CQCEVCLEXPORT TFacCQCEvCl& facCQCEvCl();


// ---------------------------------------------------------------------------
//  For those collection types that we pre-generate in our main cpp file, we want to
//  insure that they don't get generated again in users of the DLL.
// ---------------------------------------------------------------------------
#if     !defined(CQCEVCL_PREINST)
extern template class TVector<TCQCEvMonCfg>;
extern template class TVector<TCQCSchEvent>;
extern template class TVector<TCQCTrgEvent>;

extern template class TVector<TKeyedCQCEvMonCfg>;
extern template class TVector<TKeyedCQCSchEvent>;
extern template class TVector<TKeyedCQCTrgEvent>;
#endif


