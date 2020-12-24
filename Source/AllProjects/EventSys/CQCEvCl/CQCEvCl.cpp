//
// FILE NAME: CQCKit.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/05/2000
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
//  This is the main cpp file of the facility. It provides the lazy eval
//  method for our facility object, and some other odds and ends.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  For this file we want to pre instantiate some collections, but not for any
//  other files. Only this file defines this toke, which prevents the use of the
//  extern declaration.
// ---------------------------------------------------------------------------
#define CQCEVCL_PREINST  1


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCEvCl_.hpp"


// ---------------------------------------------------------------------------
//  For the templates we pre-instantiate, for them to be fully instantiated here
// ---------------------------------------------------------------------------
template class CQCEVCLEXPORT TVector<TCQCEvMonCfg>;
template class CQCEVCLEXPORT TVector<TCQCSchEvent>;
template class CQCEVCLEXPORT TVector<TCQCTrgEvent>;

template class CQCEVCLEXPORT TVector<TKeyedCQCEvMonCfg>;
template class CQCEVCLEXPORT TVector<TKeyedCQCSchEvent>;
template class CQCEVCLEXPORT TVector<TKeyedCQCTrgEvent>;


// ---------------------------------------------------------------------------
//  Glocal functions
// ---------------------------------------------------------------------------
TFacCQCEvCl& facCQCEvCl()
{
    static TFacCQCEvCl* pfacThis = new TFacCQCEvCl();
    return *pfacThis;
}
