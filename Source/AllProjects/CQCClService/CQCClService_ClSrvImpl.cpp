//
// FILE NAME: CQCClService_ClSrvImpl.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/21/2013
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
//  This file implements our server side IDL interface, to handle the ORB
//  callbacks for the services we provide. Mostly we just pass things on to
//  the facility object.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCClService.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCClSrvImpl,TCQCClServiceServerBase)


// ---------------------------------------------------------------------------
//   CLASS: TCQCClSrvImpl
//  PREFIX: orbs
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// TCQCClSrvImpl: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TCQCClSrvImpl::bQueryRepoDB(const   TString&            strRepoMon
                            , const TString&            strCurSerialNum
                            ,       tCIDLib::TCard4&    c4BufSz
                            ,       THeapBuf&           mbufData)
{
    return facCQCClService().bQueryRepoDB
    (
        strRepoMon, strCurSerialNum, c4BufSz, mbufData
    );
}


tCIDLib::TCard4 TCQCClSrvImpl::c4QueryRepoList(TVector<TString>& colToFill)
{
    return facCQCClService().c4QueryRepoList(colToFill);
}


// ---------------------------------------------------------------------------
//  TCQCClSrvImpl: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCClSrvImpl::Initialize()
{
    // Nothing to do at this time, just pass to our parent
    TParent::Initialize();
}


tCIDLib::TVoid TCQCClSrvImpl::Terminate()
{
    // Call our parent
    TParent::Terminate();
}

