//
// FILE NAME: MQTTS_IOEvent.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/04/2019
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
//  This file implements the simple I/O event class used to report info from the
//  I/O thread to the main thread.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//

// ---------------------------------------------------------------------------
//  Bring in our own public header
// ---------------------------------------------------------------------------
#include    "MQTTS_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TMQTTIOEvent, TObject)



// ---------------------------------------------------------------------------
//   CLASS: TMQTTIOEvent
//  PREFIX: ioev
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TMQTTIOEvent: Constructor and Destructor
// ---------------------------------------------------------------------------
TMQTTIOEvent::TMQTTIOEvent()
{
    Reset();
}

TMQTTIOEvent::~TMQTTIOEvent()
{
}


// Called by the pool to reset us
tCIDLib::TVoid TMQTTIOEvent::Reset()
{
    m_bV2Compat = kCIDLib::False;
    m_eCommRes  = tCQCKit::ECommResults::Success;
    m_eEvent    = tMQTTS::EIOEvents::None;
    m_eFldType  = tCQCKit::EFldTypes::Count;
    m_eIOState  = tMQTTSh::EClStates::Count;
    m_eSemType  = tCQCKit::EFldSTypes::Count;
    m_strValue.Clear();
    m_strFldName.Clear();
}


tCIDLib::TVoid TMQTTIOEvent::SetBadValEvent(const TMQTTFldCfg& mqfldcSrc)
{
    Reset();
    m_bV2Compat = mqfldcSrc.bV2Compat();
    m_eFldType = mqfldcSrc.eFldType();
    m_eEvent = tMQTTS::EIOEvents::FldError;
    m_eIOState = tMQTTSh::EClStates::Count;
    m_eSemType = mqfldcSrc.eSemType();
    m_strFldName = mqfldcSrc.strFldName();
}


tCIDLib::TVoid
TMQTTIOEvent::SetFldWriteRes(const  TMQTTFldCfg&            mqfldcSrc
                            , const tCQCKit::ECommResults   eResult)
{
    Reset();
    m_bV2Compat = mqfldcSrc.bV2Compat();
    m_eEvent = tMQTTS::EIOEvents::FieldWrite;
    m_eFldType = mqfldcSrc.eFldType();
    m_eIOState = tMQTTSh::EClStates::Count;
    m_eSemType = mqfldcSrc.eSemType();
    m_strFldName = mqfldcSrc.strFldName();
    m_eCommRes  = eResult;
}

tCIDLib::TVoid
TMQTTIOEvent::SetNewValEvent(const TMQTTFldCfg& mqfldcSrc, const TString& strNewFldVal)
{
    Reset();
    m_bV2Compat = mqfldcSrc.bV2Compat();
    m_eFldType = mqfldcSrc.eFldType();
    m_eEvent = tMQTTS::EIOEvents::FldValue;
    m_eIOState = tMQTTSh::EClStates::Count;
    m_eSemType = mqfldcSrc.eSemType();
    m_strBaseName = mqfldcSrc.strBaseName();
    m_strFldName = mqfldcSrc.strFldName();
    m_strValue = strNewFldVal;
}
