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
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TInstQueryPacket,TObject)


// ---------------------------------------------------------------------------
//   CLASS: TInstQueryPacket
//  PREFIX: spack
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TInstQueryPacket: Constructors and Destructor
// ---------------------------------------------------------------------------
TInstQueryPacket::TInstQueryPacket() :

    m_eMsgBoxRet(tCIDCtrls::EMsgBoxRets::Other)
    , m_eMsgBoxType(tCIDCtrls::EMsgBoxTypes::Count)
    , m_eOp(tCQCInst::EQueryOps::None)
    , m_eResult(tCQCInst::EQueryRes::Success)
{
}

TInstQueryPacket::TInstQueryPacket( const   tCQCInst::EQueryOps     eOp
                                    , const tCIDCtrls::EMsgBoxTypes eMsgBoxType
                                    , const TString&                strMsg) :

    m_eMsgBoxRet(tCIDCtrls::EMsgBoxRets::Other)
    , m_eMsgBoxType(eMsgBoxType)
    , m_eOp(eOp)
    , m_eResult(tCQCInst::EQueryRes::Success)
    , m_strMsg(strMsg)
{
}

TInstQueryPacket::TInstQueryPacket( const   TString&    strMsg
                                    , const TError&     errToDisplay) :

    m_eMsgBoxRet(tCIDCtrls::EMsgBoxRets::Other)
    , m_eMsgBoxType(tCIDCtrls::EMsgBoxTypes::Count)
    , m_errException(errToDisplay)
    , m_eOp(tCQCInst::EQueryOps::ShowFatalErr)
    , m_eResult(tCQCInst::EQueryRes::Success)
{
}

TInstQueryPacket::~TInstQueryPacket()
{
}


// ---------------------------------------------------------------------------
//  TInstQueryPacket: Public, non-virtual methods
// ---------------------------------------------------------------------------
const TError& TInstQueryPacket::errException() const
{
    return m_errException;
}

const TError& TInstQueryPacket::errException(const TError& errToSet)
{
    m_eResult = tCQCInst::EQueryRes::Exception;
    m_errException = errToSet;
    return m_errException;
}


tCIDCtrls::EMsgBoxTypes TInstQueryPacket::eMsgBoxType() const
{
    return m_eMsgBoxType;
}


tCIDCtrls::EMsgBoxRets TInstQueryPacket::eMsgBoxRet() const
{
    return m_eMsgBoxRet;
}

tCIDCtrls::EMsgBoxRets TInstQueryPacket::eMsgBoxRet(const tCIDCtrls::EMsgBoxRets eToSet)
{
    m_eMsgBoxRet = eToSet;
    return m_eMsgBoxRet;
}


tCQCInst::EQueryOps TInstQueryPacket::eOp() const
{
    return m_eOp;
}


tCQCInst::EQueryRes TInstQueryPacket::eResult() const
{
    return m_eResult;
}

tCQCInst::EQueryRes TInstQueryPacket::eResult(const tCQCInst::EQueryRes eToSet)
{
    m_eResult = eToSet;
    return m_eResult;
}


const TString& TInstQueryPacket::strMsg() const
{
    return m_strMsg;
}

