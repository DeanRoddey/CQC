//
// FILE NAME: CQCServer_SrvAdminImpl.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/29/2001
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
//  This file implements the CQCServer remote admin interface. We just
//  delegate almost everything to the facility object.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCQTAudSrv.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TQTIntfServerImpl,TQTIntfServerBase)



// ---------------------------------------------------------------------------
//   CLASS: TQTIntfServerImpl
//  PREFIX: orbs
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TQTIntfServerImpl: Constructors and Destructor
// ---------------------------------------------------------------------------
TQTIntfServerImpl::TQTIntfServerImpl()
{
}

TQTIntfServerImpl::~TQTIntfServerImpl()
{
}


// ---------------------------------------------------------------------------
//  TQTIntfServerImpl: Public, inherited methods
// ---------------------------------------------------------------------------

// These all just call through to the facility class who does the work
tCIDLib::TBoolean
TQTIntfServerImpl::bMute(const tCIDLib::TBoolean bToSet)
{
    return facCQCQTAudSrv.bMute(bToSet);
}


tCIDLib::TCard4
TQTIntfServerImpl::c4Volume(const tCIDLib::TCard4 c4ToSet)
{
    return facCQCQTAudSrv.c4Volume(c4ToSet);
}


tCIDLib::TVoid
TQTIntfServerImpl::LoadFile(const   TString& strPath
                            , const TString& strAudioDevice)
{
    facCQCQTAudSrv.LoadFile(strPath, strAudioDevice);
}

tCIDLib::TVoid TQTIntfServerImpl::Pause()
{
    facCQCQTAudSrv.Pause();
}

tCIDLib::TVoid TQTIntfServerImpl::Play()
{
    facCQCQTAudSrv.Play();
}

tCIDLib::TVoid TQTIntfServerImpl::Reset()
{
    facCQCQTAudSrv.Reset();
}

tCIDLib::TVoid TQTIntfServerImpl::Stop()
{
    facCQCQTAudSrv.Stop();
}


// ---------------------------------------------------------------------------
//  TQTIntfServerImpl: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TQTIntfServerImpl::Initialize()
{
}


tCIDLib::TVoid TQTIntfServerImpl::Terminate()
{
}


