//
// FILE NAME: CQCAppShell_AdminImpl.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/09/2002
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
//  This file implements our admin interface which allows CQCClient apps
//  to get inforamtion about the app shell or the apps it controls, and to
//  cycle those apps.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCAppShell.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCAppShellImpl,TCQCAppShellAdminServerBase)



// ---------------------------------------------------------------------------
//   CLASS: TCQCAppShellImpl
//  PREFIX: orbs
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCAppShellImpl: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCAppShellImpl::TCQCAppShellImpl()
{
}


TCQCAppShellImpl::~TCQCAppShellImpl()
{
}


// ---------------------------------------------------------------------------
// TCQCAppShellImpl: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TCQCAppShellImpl::bQueryNewMsgs(        tCIDLib::TCard4&    c4MsgIndex
                                ,       TVector<TString>&   colMsgs
                                , const tCIDLib::TBoolean   bAddNewLine)
{
    // Just delegate to the facility class
    return facCQCAppShell().bQueryNewMsgs(c4MsgIndex, colMsgs, bAddNewLine);
}


tCIDLib::TCard4 TCQCAppShellImpl::c4QueryApps(tCIDLib::TKVPList& colApps)
{
    // Just delegate to the facility class
    return facCQCAppShell().c4QueryApps(colApps);
}


tCIDLib::TVoid TCQCAppShellImpl::CycleApps()
{
    // Just delegate to the facility class
    return facCQCAppShell().CycleApps();
}


// ---------------------------------------------------------------------------
//  TCQCAppShellImpl: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCAppShellImpl::Initialize()
{
    // Nothing to do at this time
}


tCIDLib::TVoid TCQCAppShellImpl::Terminate()
{
    // Nothing to do at this time
}


