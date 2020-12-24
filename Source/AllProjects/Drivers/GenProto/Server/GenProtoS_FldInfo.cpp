//
// FILE NAME: GenProtoS_FldInfo.cpp
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
//  This file implements the class that tracks the information we need to
//  keep up with for each field that the protocol file defines.
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
RTTIDecls(TGenProtoFldInfo,TObject)


// ---------------------------------------------------------------------------
//   CLASS: TGenProtoFldInfo
//  PREFIX: ddat
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TGenProtoFldInfo: Constructors and Destructor
// ---------------------------------------------------------------------------
TGenProtoFldInfo::TGenProtoFldInfo( const   TString&            strName
                                    , const tCQCKit::EFldTypes  eType
                                    , const tCIDLib::TCard4     c4Id
                                    , const TString&            strLimits
                                    , const tCQCKit::EFldAccess eAccess
                                    , const tCQCKit::EFldSTypes eSemType) :

    m_bIsReviveField(kCIDLib::False)
    , m_c4LastSendTime(0)
    , m_c4MinSendInterval(0)
    , m_c4WaitMillis(0)
    , m_colWriteCmd(tCIDLib::EAdoptOpts::Adopt)
    , m_flddInfo(strName, eType, eAccess, eSemType, strLimits)
    , m_prepAck(0)
    , m_prepNak(0)
{
    // Store the field id, which isn't set in the ctor
    m_flddInfo.c4Id(c4Id);
}

TGenProtoFldInfo::TGenProtoFldInfo(const TGenProtoFldInfo& fldiToCopy) :

    m_bIsReviveField(fldiToCopy.m_bIsReviveField)
    , m_c4LastSendTime(fldiToCopy.m_c4LastSendTime)
    , m_c4MinSendInterval(fldiToCopy.m_c4MinSendInterval)
    , m_c4WaitMillis(fldiToCopy.m_c4WaitMillis)
    , m_colWriteCmd(tCIDLib::EAdoptOpts::Adopt, fldiToCopy.m_colWriteCmd.c4ElemCount())
    , m_flddInfo(fldiToCopy.m_flddInfo)
    , m_prepAck(fldiToCopy.m_prepAck)
    , m_prepNak(fldiToCopy.m_prepNak)
{
    // And dup the expressions that build the write command
    const tCIDLib::TCard4 c4Count = fldiToCopy.m_colWriteCmd.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        m_colWriteCmd.Add
        (
            ::pDupObject<TGenProtoExprNode>(fldiToCopy.m_colWriteCmd[c4Index])
        );
    }
}

TGenProtoFldInfo::~TGenProtoFldInfo()
{
    //
    //  Flush the write command collection. Its not required, since the
    //  collection will clean up anyway, but just to make it obvious to
    //  readers of the code that we are destroying the expression list.
    //
    m_colWriteCmd.RemoveAll();
}


// ---------------------------------------------------------------------------
//  TGenProtoFldInfo: Public operators
// ---------------------------------------------------------------------------
TGenProtoFldInfo& TGenProtoFldInfo::operator=(const TGenProtoFldInfo& fldiToAssign)
{
    if (this != &fldiToAssign)
    {
        // Copy over the field info member
        m_bIsReviveField    = fldiToAssign.m_bIsReviveField;
        m_c4LastSendTime    = fldiToAssign.m_c4LastSendTime;
        m_c4MinSendInterval = fldiToAssign.m_c4MinSendInterval;
        m_c4WaitMillis      = fldiToAssign.m_c4WaitMillis;
        m_flddInfo          = fldiToAssign.m_flddInfo;
        m_prepAck           = fldiToAssign.m_prepAck;
        m_prepNak           = fldiToAssign.m_prepNak;

        // Clear our list of write command expressions and dup the source's
        m_colWriteCmd.RemoveAll();
        const tCIDLib::TCard4 c4Count = fldiToAssign.m_colWriteCmd.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            m_colWriteCmd.Add
            (
                ::pDupObject<TGenProtoExprNode>(fldiToAssign.m_colWriteCmd[c4Index])
            );
        }
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TGenProtoFldInfo: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TGenProtoFldInfo::AddAckInfo(       TGenProtoReply* const   prepAck
                            ,       TGenProtoReply* const   prepNak
                            , const tCIDLib::TCard4         c4Wait)
{
    m_c4WaitMillis  = c4Wait;
    m_prepAck       = prepAck;
    m_prepNak       = prepNak;
}


tCIDLib::TVoid
TGenProtoFldInfo::AddWriteCmdItem(TGenProtoExprNode* const pnodeToAdopt)
{
    m_colWriteCmd.Add(pnodeToAdopt);
}


tCIDLib::TBoolean TGenProtoFldInfo::bHasWriteCmds() const
{
    return !m_colWriteCmd.bIsEmpty();
}


tCIDLib::TBoolean TGenProtoFldInfo::bIsReviveField() const
{
    return m_bIsReviveField;
}

tCIDLib::TBoolean
TGenProtoFldInfo::bIsReviveField(const tCIDLib::TBoolean bToSet)
{
    m_bIsReviveField = bToSet;
    return m_bIsReviveField;
}


// Just a convenience that checks the access fields of the field def
tCIDLib::TBoolean TGenProtoFldInfo::bIsWriteable() const
{
    return ((m_flddInfo.eAccess() == tCQCKit::EFldAccess::ReadWrite)
           || (m_flddInfo.eAccess() == tCQCKit::EFldAccess::Write));
}


tCIDLib::TVoid TGenProtoFldInfo::BuildWriteCmd(TGenProtoCtx& ctxToUse)
{
    const tCIDLib::TCard4 c4Count = m_colWriteCmd.c4ElemCount();

    // If no write command items are defined, then throw
    if (!c4Count)
    {
        facGenProtoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kGPDErrs::errcRunT_NoWriteCmd
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::CantDo
            , m_flddInfo.strName()
        );
    }

    //
    //  Allrighty. Lets loop through the expressions that make up the write
    //  command for this field. For each one, ask the resulting value object
    //  to store itself on the target memory buffer, keeping up with the
    //  bytes written.
    //
    tCIDLib::TCard4 c4Bytes = 0;

    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        //
        //  Update the context with the bytes in the send buffer. We have
        //  to do this at each step because any expression could refer to
        //  the current length.
        //
        ctxToUse.c4SendBytes(c4Bytes);

        // Get the node and evaluate it with the current context
        TGenProtoExprNode* pnodeCur = m_colWriteCmd[c4Index];
        pnodeCur->Evaluate(ctxToUse);

        //
        //  And ask it to write its value to our buffer. It will update
        //  the c4Bytes parameter by the bytes written.
        //
        pnodeCur->evalCur().ValueToMemBuf
        (
            ctxToUse
            , ctxToUse.mbufSend()
            , c4Bytes
        );
    }

    // And do the final update of the length
    ctxToUse.c4SendBytes(c4Bytes);
}


tCIDLib::TCard4 TGenProtoFldInfo::c4MillisToWait() const
{
    return m_c4WaitMillis;
}


tCIDLib::TCard4 TGenProtoFldInfo::c4MinSendInterval() const
{
    return m_c4MinSendInterval;
}

tCIDLib::TCard4
TGenProtoFldInfo::c4MinSendInterval(const tCIDLib::TCard4 c4ToSet)
{
    m_c4MinSendInterval = c4ToSet;

    //
    //  If clearing the interval, also clear the last send time, so that our
    //  next send time will be zero always.
    //
    if (!c4ToSet)
        m_c4LastSendTime = 0;

    return m_c4MinSendInterval;
}


// Calc the next time that it's legal to send this message
tCIDLib::TCard4 TGenProtoFldInfo::c4NextSendTime() const
{
    return m_c4LastSendTime + m_c4MinSendInterval;
}


const TCQCFldDef& TGenProtoFldInfo::flddInfo() const
{
    return m_flddInfo;
}

TCQCFldDef& TGenProtoFldInfo::flddInfo()
{
    return m_flddInfo;
}


TGenProtoReply* TGenProtoFldInfo::prepAck()
{
    return m_prepAck;
}


TGenProtoReply* TGenProtoFldInfo::prepNak()
{
    return m_prepNak;
}


tCIDLib::TVoid
TGenProtoFldInfo::SetLastSendTime(const tCIDLib::TCard4 c4ToSet)
{
    //
    //  Only store it if we have a minimum interval, so that if we don't
    //  have one set, our next send time will always be zero.
    //
    if (m_c4MinSendInterval)
        m_c4LastSendTime = c4ToSet;
}


