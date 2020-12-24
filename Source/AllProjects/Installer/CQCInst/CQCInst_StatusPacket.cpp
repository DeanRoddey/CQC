//
// FILE NAME: CQCInst_StatusPacket.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/29/2002
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
//  This file implements the status packet class, which is how the installer
//  thread queues up events to the display dialog.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCInst.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCStatusPacket,TObject)


// ---------------------------------------------------------------------------
//   CLASS: TCQCStatusPacket
//  PREFIX: spack
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCStatusPacket: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCStatusPacket::TCQCStatusPacket() :

    m_eStep(tCQCInst::EInstSteps::None)
{
}

TCQCStatusPacket::TCQCStatusPacket(const tCQCInst::EInstSteps eStep) :

    m_eStep(eStep)
{
}

TCQCStatusPacket::TCQCStatusPacket(const TCQCStatusPacket& spackToCopy) :

    m_eStep(spackToCopy.m_eStep)
{
}

TCQCStatusPacket::~TCQCStatusPacket()
{
}


// ---------------------------------------------------------------------------
//  TCQCStatusPacket: Public operators
// ---------------------------------------------------------------------------
TCQCStatusPacket&
TCQCStatusPacket::operator=(const TCQCStatusPacket& spackToAssign)
{
    if (this != &spackToAssign)
        m_eStep = spackToAssign.m_eStep;
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCStatusPacket: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCQCInst::EInstSteps TCQCStatusPacket::eStep() const
{
    return m_eStep;
}


