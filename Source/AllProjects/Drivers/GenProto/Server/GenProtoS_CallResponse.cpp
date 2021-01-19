//
// FILE NAME: GenProtoS_CallResponse.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/24/2003
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
//  This file implements a simple class that represents a call/response
//  exchange with the target device.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "GenProtoS_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TGenProtoCallRep,TObject)


// ---------------------------------------------------------------------------
//   CLASS: TGenProtoCallRep
//  PREFIX: clrp
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TGenProtoCallRep: Constructors and Destructor
// ---------------------------------------------------------------------------
TGenProtoCallRep::TGenProtoCallRep() :

    m_c4MillisPeriod(0)
    , m_c4MillisToWait(0)
    , m_c4NextPoll(TTime::c4Millis())
    , m_pqryToSend(nullptr)
    , m_prepToExpect(nullptr)
{
}

TGenProtoCallRep::
TGenProtoCallRep(       TGenProtoQuery* const   pqryToSend
                ,       TGenProtoReply* const   prepToExpect
                , const tCIDLib::TCard4         c4MillisToWait) :

    m_c4MillisPeriod(0)
    , m_c4MillisToWait(c4MillisToWait)
    , m_c4NextPoll(TTime::c4Millis())
    , m_pqryToSend(pqryToSend)
    , m_prepToExpect(prepToExpect)
{
}

TGenProtoCallRep::
TGenProtoCallRep(       TGenProtoQuery* const   pqryToSend
                ,       TGenProtoReply* const   prepToExpect
                , const tCIDLib::TCard4         c4MillisToWait
                , const tCIDLib::TCard4         c4MillisPeriod) :

    m_c4MillisPeriod(c4MillisPeriod)
    , m_c4MillisToWait(c4MillisToWait)
    , m_c4NextPoll(TTime::c4Millis())
    , m_pqryToSend(pqryToSend)
    , m_prepToExpect(prepToExpect)
{
}


TGenProtoCallRep::TGenProtoCallRep(const TGenProtoCallRep& clrpToCopy) :

    m_c4MillisPeriod(clrpToCopy.m_c4MillisPeriod)
    , m_c4MillisToWait(clrpToCopy.m_c4MillisToWait)
    , m_c4NextPoll(clrpToCopy.m_c4NextPoll)
    , m_pqryToSend(clrpToCopy.m_pqryToSend)
    , m_prepToExpect(clrpToCopy.m_prepToExpect)
{
}

TGenProtoCallRep::~TGenProtoCallRep()
{
    // We don't own the query/reply pointers, so do nothing
}


// ---------------------------------------------------------------------------
//  TGenProtoCallRep: Public operators
// ---------------------------------------------------------------------------
TGenProtoCallRep&
TGenProtoCallRep::operator=(const TGenProtoCallRep& clrpToAssign)
{
    if (this != &clrpToAssign)
    {
        // We don't own the query and reply pointers, so we can shallow copy
        m_c4MillisPeriod    = clrpToAssign.m_c4MillisPeriod;
        m_c4MillisToWait    = clrpToAssign.m_c4MillisToWait;
        m_c4NextPoll        = clrpToAssign.m_c4NextPoll;
        m_pqryToSend        = clrpToAssign.m_pqryToSend;
        m_prepToExpect      = clrpToAssign.m_prepToExpect;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TGenProtoCallRep: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TGenProtoCallRep::bExpectsReply() const
{
    return (m_prepToExpect != nullptr);
}


tCIDLib::TCard4 TGenProtoCallRep::c4MillisPeriod() const
{
    return m_c4MillisPeriod;
}


tCIDLib::TCard4 TGenProtoCallRep::c4MillisToWait() const
{
    return m_c4MillisToWait;
}


tCIDLib::TCard4 TGenProtoCallRep::c4NextPollTime() const
{
    return m_c4NextPoll;
}


const TGenProtoQuery& TGenProtoCallRep::qryToSend() const
{
    CIDAssert(m_pqryToSend != nullptr, L"m_pqryToSend data is null");
    return *m_pqryToSend;
}

TGenProtoQuery& TGenProtoCallRep::qryToSend()
{
    CIDAssert(m_pqryToSend != nullptr, L"m_pqryToSend data is null");
    return *m_pqryToSend;
}


const TGenProtoReply& TGenProtoCallRep::repToExpect() const
{
    // This is optional, so throw if not set
    if (!m_prepToExpect)
    {
        facGenProtoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kGPDErrs::errcRunT_NoReplyDefined
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotReady
            , m_pqryToSend->strKey()
        );
    }
    return *m_prepToExpect;
}

TGenProtoReply& TGenProtoCallRep::repToExpect()
{
    // This is optional, so throw if not set
    if (!m_prepToExpect)
    {
        facGenProtoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kGPDErrs::errcRunT_NoReplyDefined
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotReady
            , m_pqryToSend->strKey()
        );
    }
    return *m_prepToExpect;
}


tCIDLib::TVoid TGenProtoCallRep::ResetPollTime()
{
    m_c4NextPoll = TTime::c4Millis() + m_c4MillisPeriod;
}


tCIDLib::TVoid
TGenProtoCallRep::Set(          TGenProtoQuery* const   pqryToSend
                        ,       TGenProtoReply* const   prepToExpect
                        , const tCIDLib::TCard4         c4MillisToWait)
{
    // We don't own the pointers, so no cleanup of old code is required
    m_c4MillisToWait = c4MillisToWait;
    m_pqryToSend = pqryToSend;
    m_prepToExpect = prepToExpect;
}


