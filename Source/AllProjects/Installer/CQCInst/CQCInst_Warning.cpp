//
// FILE NAME: CQCInst_QueryPacket.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/15/2005
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
//  This file implements the query packet class, which the background install
//  thread uses to get info from the user through the foreground thread so
//  as to avoid any synchronization issues or multiple window threads.
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
//   CLASS: TInstWarning
//  PREFIX: spack
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TInstWarning: Constructors and Destructor
// ---------------------------------------------------------------------------
TInstWarning::TInstWarning() :

    m_bHasExcept(kCIDLib::False)
{
}

TInstWarning::TInstWarning(const TString& strMsg) :

    m_bHasExcept(kCIDLib::False)
    , m_strMsg(strMsg)
{
}

TInstWarning::TInstWarning( const   TString&    strMsg
                            , const TString&    strResource) :

    m_bHasExcept(kCIDLib::False)
    , m_strMsg(strMsg)
    , m_strResource(strResource)
{
}

TInstWarning::TInstWarning(const TString& strMsg, const TError& errToDisplay) :

    m_bHasExcept(kCIDLib::True)
    , m_errException(errToDisplay)
    , m_strMsg(strMsg)
{
}

TInstWarning::TInstWarning( const   TString&    strMsg
                            , const TString&    strResource
                            , const TError&     errToDisplay) :

    m_bHasExcept(kCIDLib::True)
    , m_errException(errToDisplay)
    , m_strMsg(strMsg)
    , m_strResource(strResource)
{
}

TInstWarning::TInstWarning(const TInstWarning& warnSrc) :

    m_bHasExcept(warnSrc.m_bHasExcept)
    , m_errException(warnSrc.m_errException)
    , m_strMsg(warnSrc.m_strMsg)
    , m_strResource(warnSrc.m_strResource)
{
}

TInstWarning::~TInstWarning()
{
}


// ---------------------------------------------------------------------------
//  TInstWarning: Public operators
// ---------------------------------------------------------------------------

TInstWarning& TInstWarning::operator=(const TInstWarning& warnSrc)
{
    if (&warnSrc != this)
    {
        m_bHasExcept   = warnSrc.m_bHasExcept;
        m_errException = warnSrc.m_errException;
        m_strMsg       = warnSrc.m_strMsg;
        m_strResource  = warnSrc.m_strResource;
    }
    return *this;
}
