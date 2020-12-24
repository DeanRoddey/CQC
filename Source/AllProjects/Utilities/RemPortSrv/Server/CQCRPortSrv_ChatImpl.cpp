//
// FILE NAME: CQCRPortSrv_ChatImpl.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/22/2014
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
//  This file implements the server side of the remote chat ORB interface.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCRPortSrv.hpp"



// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TRemChatImpl,TRemChatServerBase)



// ---------------------------------------------------------------------------
//  CLASS: TRemChatImpl
// PREFIX: orbs
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TRemChatImpl: Constructors and Destructor
// ---------------------------------------------------------------------------
TRemChatImpl::TRemChatImpl(const TOrbObjId& ooidToUse) :

    TRemChatServerBase(ooidToUse)
    , m_colInMsgs(tCIDLib::EMTStates::Safe)
    , m_colOutMsgs(tCIDLib::EMTStates::Safe)
    , m_enctLastMsg(0)
    , m_enctTimeout((TRemChatServerBase::c4PollSeconds * kCIDLib::enctOneSecond) * 2)
{
}

TRemChatImpl::~TRemChatImpl()
{
}


// ---------------------------------------------------------------------------
//  TRemChatImpl: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TRemChatImpl::bExchange(const TString& strSendMsg, TString& strRecMsg)
{
    // Update our last msg stamp
    m_enctLastMsg = TTime::enctNow();

    // If there's an incoming msg, queue it up
    if (!strSendMsg.bIsEmpty())
        m_colInMsgs.objPushBottom(strSendMsg);

    // Return true if we have any outgoing available
    return m_colOutMsgs.bPopTop(strRecMsg);
}


// ---------------------------------------------------------------------------
//  TRemChatImpl: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  If it has been more than twice the polling period since we got a msg from a
//  client, then say we don't have an active one, else we do.
//
tCIDLib::TBoolean TRemChatImpl::bActiveClient() const
{
    return (TTime::enctNow() - m_enctLastMsg) < m_enctTimeout;
}


// Just queue it up if we aren't full
tCIDLib::TBoolean TRemChatImpl::bQueueMsg(const TString& strMsg)
{
    if (m_colOutMsgs.bIsFull(256))
        return kCIDLib::False;

    // Push it onto the bottom
    m_colOutMsgs.objPushBottom(strMsg);
    return kCIDLib::True;
}


// If there's a msg available in the input queue, return it
tCIDLib::TBoolean TRemChatImpl::bGetMsg(TString& strMsg)
{
    return m_colInMsgs.bPopTop(strMsg);
}


// ---------------------------------------------------------------------------
//  TRemChatImpl: Protected, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid TRemChatImpl::Initialize()
{
}


tCIDLib::TVoid TRemChatImpl::Terminate()
{
}

