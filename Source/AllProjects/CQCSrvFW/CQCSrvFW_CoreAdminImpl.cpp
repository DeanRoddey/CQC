//
// FILE NAME: CQCSrvFW_CoreAdminImpl.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/30/2001
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
//  This file provides our implementation of the core admin protocol.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include "CQCSrvFW_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TCIDCoreAdminImpl,TCIDCoreAdminBaseImpl)



// ---------------------------------------------------------------------------
//   CLASS: TCIDCoreAdminImpl
//  PREFIX: orbs
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// Constructors and Destructor
// ---------------------------------------------------------------------------
TCIDCoreAdminImpl::TCIDCoreAdminImpl(TCQCSrvCore* const pfacOwner) :

    TCIDCoreAdminBaseImpl(pfacOwner->strName())
    , m_pfacOwner(pfacOwner)
{
}

TCIDCoreAdminImpl::~TCIDCoreAdminImpl()
{
}


// ---------------------------------------------------------------------------
//  TCIDCoreAdminImpl: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCIDCoreAdminImpl::Initialize()
{
    // Nothing to do
}


tCIDLib::TVoid TCIDCoreAdminImpl::Terminate()
{
    // Nothing to do
}

