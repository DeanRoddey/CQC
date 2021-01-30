//
// FILE NAME: LexMC1S_Msg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/14/2000
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
//  This is is the implementation the simple Lexicon message wrapper.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "LexMC1S.hpp"


// ---------------------------------------------------------------------------
//   CLASS: TLexMsg
//  PREFIX: lm
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TLexMsg: Constructors and Destructor
// ---------------------------------------------------------------------------
TLexMsg::TLexMsg()
{
    TRawMem::SetMemBuf(&m_mpData, kCIDLib::c1MinCard, sizeof(TMsgPacket));
    m_mpData.c1SOP = kLexMC1S::c1SOP;
}

TLexMsg::TLexMsg(const TLexMsg& lmToCopy)
{
    TRawMem::CopyMemBuf(&m_mpData, &lmToCopy.m_mpData, sizeof(TMsgPacket));
}

TLexMsg::TLexMsg(const  tLexMC1S::ECmds         eCmd
                , const tCIDLib::TCard1* const  pc1Data
                , const tCIDLib::TCard1         c1Bytes)
{
    m_mpData.c1SOP = kLexMC1S::c1SOP;
    m_mpData.c1DLLCount = c1Bytes + 3;
    m_mpData.c1Cmd = tCIDLib::TCard1(eCmd);
    m_mpData.c1DataCount = c1Bytes;

    if (c1Bytes)
        TRawMem::CopyMemBuf(m_mpData.ac1Data, pc1Data, c1Bytes);
    m_mpData.ac1Data[c1Bytes] = kLexMC1S::c1EOP;
}

TLexMsg::~TLexMsg()
{
}


// ---------------------------------------------------------------------------
//  TLexMsg: Public operators
// ---------------------------------------------------------------------------
TLexMsg& TLexMsg::operator=(const TLexMsg& lmToAssign)
{
    if (this == &lmToAssign)
        return *this;

    TRawMem::CopyMemBuf(&m_mpData, &lmToAssign.m_mpData, sizeof(TMsgPacket));
    return *this;
}

tCIDLib::TCard1 TLexMsg::operator[](const tCIDLib::TCard4 c4Index) const
{
    if (c4Index >= m_mpData.c1DataCount)
    {
        facLexMC1S().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMC1SErrs::errcMsg_BadDataIndex
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::AppError
            , TCardinal(c4Index)
            , TCardinal(m_mpData.c1DataCount)
        );
    }
    return m_mpData.ac1Data[c4Index];
}


// ---------------------------------------------------------------------------
//  TLexMsg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TLexMsg::AckOn(TCommPortBase& commTarget) const
{
    // Build an ack for this message
    TMsgPacket mpAck;
    mpAck.c1SOP = kLexMC1S::c1SOP;
    mpAck.c1DLLCount = 4;
    mpAck.c1Cmd = tCIDLib::TCard1(tLexMC1S::ECmd_Ack);
    mpAck.c1DataCount = 1;
    mpAck.ac1Data[0] = m_mpData.c1Cmd;
    mpAck.ac1Data[1] = kLexMC1S::c1EOP;

    const tCIDLib::TCard4 c4Sent = commTarget.c4WriteRawBufMS
    (
        &mpAck, mpAck.c1DLLCount + 2, 250
    );

    if (c4Sent != tCIDLib::TCard4(mpAck.c1DLLCount + 2))
    {
        facLexMC1S().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMC1SErrs::errcIO_WriteFailure
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotAllWritten
            , TCardinal(mpAck.c1Cmd)
        );
    }
}



tCIDLib::TVoid TLexMsg::AddDataByte(const tCIDLib::TCard1 c1ToAdd)
{
    if (m_mpData.c1DataCount > 128)
    {
        facLexMC1S().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMC1SErrs::errcMsg_TooManyDataBytes
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::OutResource
        );
    }

    m_mpData.c1DLLCount++;
    m_mpData.ac1Data[m_mpData.c1DataCount++] = c1ToAdd;
    m_mpData.ac1Data[m_mpData.c1DataCount] = kLexMC1S::c1EOP;
}


tCIDLib::TBoolean TLexMsg::bIsReplyTo(const TLexMsg& lmToTest) const
{
    switch(lmToTest.eCmd())
    {
        case tLexMC1S::ECmd_HostWakeup :
        case tLexMC1S::ECmd_SendIR :
        case tLexMC1S::ECmd_SetSysParmValues :
        case tLexMC1S::ECmd_SetFxParmVals :
        case tLexMC1S::ECmd_SetFxNameById :
        case tLexMC1S::ECmd_SetSysBalance :
        case tLexMC1S::ECmd_SetSysVolume :
        case tLexMC1S::ECmd_SetActiveEffect :
        case tLexMC1S::ECmd_SetZ2Input :
        case tLexMC1S::ECmd_ClearZ2Input :
        case tLexMC1S::ECmd_SetZ2Volume :
        case tLexMC1S::ECmd_SetZ2Balance :
        case tLexMC1S::ECmd_SetCustomName :
        case tLexMC1S::ECmd_SetInputNameById :
        case tLexMC1S::ECmd_SetFPDRegs :
        case tLexMC1S::ECmd_SetComConfig :
        case tLexMC1S::ECmd_SetMute :
        case tLexMC1S::ECmd_SetOutLevels :
        case tLexMC1S::ECmd_SetDisplayStr :
            // These all either get an ack or nack
            return (m_mpData.c1Cmd == tLexMC1S::ECmd_Ack)
                || (m_mpData.c1Cmd == tLexMC1S::ECmd_Nack);

        case tLexMC1S::ECmd_HostSleep :
            break;

        case tLexMC1S::ECmd_GetSysConfig :
            return m_mpData.c1Cmd == tLexMC1S::ECmd_SysConfig;

        case tLexMC1S::ECmd_GetSysStatus :
            return m_mpData.c1Cmd == tLexMC1S::ECmd_SysStatus;

        case tLexMC1S::ECmd_GetZ2Status :
            return m_mpData.c1Cmd == tLexMC1S::ECmd_Z2Status;

        case tLexMC1S::ECmd_GetSysParmById :
        case tLexMC1S::ECmd_GetSysParmByName :
            return m_mpData.c1Cmd == tLexMC1S::ECmd_SysParamDef;

        case tLexMC1S::ECmd_GetSysParmValues :
            return m_mpData.c1Cmd == tLexMC1S::ECmd_SysParamVals;

        case tLexMC1S::ECmd_GetEffect :
            return m_mpData.c1Cmd == tLexMC1S::ECmd_EffectDef;

        case tLexMC1S::ECmd_GetFxParmDef :
            return m_mpData.c1Cmd == tLexMC1S::ECmd_EffectParamDef;

        case tLexMC1S::ECmd_GetFxParmVals :
            return m_mpData.c1Cmd == tLexMC1S::ECmd_EffectParamVals;

        case tLexMC1S::ECmd_GetCustomName :
            return m_mpData.c1Cmd == tLexMC1S::ECmd_CustName;

        case tLexMC1S::ECmd_GetInputName :
            return m_mpData.c1Cmd == tLexMC1S::ECmd_InputName;

        case tLexMC1S::ECmd_GetFPDRegs :
            return m_mpData.c1Cmd == tLexMC1S::ECmd_FPDStatus;

        case tLexMC1S::ECmd_GetComConfig :
            return m_mpData.c1Cmd == tLexMC1S::ECmd_ComConfig;

        default :
            break;
    }
    return kCIDLib::False;
}


tCIDLib::TVoid TLexMsg::Clear()
{
    m_mpData.c1SOP = kLexMC1S::c1SOP;
    m_mpData.c1DLLCount = 3;
    m_mpData.c1Cmd = 0;
    m_mpData.c1DataCount = 0;
    m_mpData.ac1Data[0] = kLexMC1S::c1EOP;
}


TLexMsg::EMsgResults
TLexMsg::eReadFrom(         TCommPortBase&      commSrc
                    ,       tLexMC1S::EErrors&  eToFill
                    , const tCIDLib::TCard4     c4ReqEndTime)
{
    // Clear the error return, on the assumption we won't get a nack
    eToFill = tLexMC1S::EErr_NoError;

    //
    //  We use a little state machine here to get in the message. This lets
    //  us catch errors as early as possible and get back to looking for
    //  good data, and to stay in sync better in the face of an intermittent
    //  connection.
    //
    enum EStates
    {
        EState_WaitSOP
        , EState_DLL
        , EState_Cmd
        , EState_DataLen
        , EState_DataBytes
        , EState_EOP
        , EState_Done
    };

    //
    //  Store the requested end time into a local because, if we run out
    //  of time but have a partly valid message, we'll do a one time
    //  extension in order to avoid splitting a message.
    //
    tCIDLib::TCard4 c4EndTime = c4ReqEndTime;

    //
    //  And now enter the state machine and try to get us a valid message.
    //  We start off waiting for a SOP.
    //
    tCIDLib::TBoolean   bExtended = kCIDLib::False;
    tCIDLib::TCard1     c1Cur;
    tCIDLib::TCard4     c4DataCount;
    EStates             eState = EState_WaitSOP;
    do
    {
        //
        //  See if we have run out of time. If so, then see if we have
        //  gotten any bytes so far. If so, and we haven't done so before,
        //  add a little more time.
        //
        if (TTime::c4Millis() > c4EndTime)
        {
            if ((eState != EState_WaitSOP) && !bExtended)
            {
                bExtended = kCIDLib::True;
                c4EndTime += 100;
            }
             else
            {
                break;
            }
        }

        // Wait for a character. If we don't get one, go back to the top
        if (!commSrc.c4ReadRawBufMS(&c1Cur, 1, 10))
            continue;

        // We got something to do the state machine transition
        switch(eState)
        {
            case EState_WaitSOP :
                // If we get a SOP, then start looking for the length
                if (c1Cur == kLexMC1S::c1SOP)
                {
                    m_mpData.c1SOP = c1Cur;
                    eState = EState_DLL;
                }
                break;

            case EState_DLL :
                // Make sure it looks reasonable
                if ((c1Cur < 3) || (c1Cur > 128))
                {
                    // Can't be right, so go back to waiting for a SOP
                    eState = EState_WaitSOP;
                }
                 else
                {
                    // Looks reasonable, so look for the command byte
                    m_mpData.c1DLLCount = c1Cur;
                    eState = EState_Cmd;
                }
                break;

            case EState_Cmd :
                // Store the command and start looking for the data len
                m_mpData.c1Cmd = c1Cur;
                eState = EState_DataLen;
                break;

            case EState_DataLen :
                //
                //  It must be the DLL count minus three. If not, then
                //  go back and start over, else move to get the data bytes.
                //
                if (c1Cur != m_mpData.c1DLLCount - 3)
                {
                    eState = EState_WaitSOP;
                }
                 else
                {
                    m_mpData.c1DataCount = c1Cur;
                    c4DataCount = 0;

                    if (c1Cur)
                        eState = EState_DataBytes;
                    else
                        eState = EState_EOP;
                }
                break;

            case EState_DataBytes :
                // Store this one in the data buffer
                m_mpData.ac1Data[c4DataCount++] = c1Cur;

                //
                //  If we have our data bytes, then move forward. We do one
                //  less than the DLL count, because the last byte should be
                //  the EOP.
                //
                if (c4DataCount == m_mpData.c1DataCount)
                    eState = EState_EOP;
                break;

            case EState_EOP :
                //
                //  If it's an EOP, then we are happy, else bail out and
                //  start over again. Else, we is done.
                //
                if (c1Cur == kLexMC1S::c1EOP)
                {
                    eState = EState_Done;
                    m_mpData.ac1Data[c4DataCount] = kLexMC1S::c1EOP;
                }
                 else
                {
                    eState = EState_WaitSOP;
                }
                break;
        }
    }   while (eState != EState_Done);

    if (eState != EState_Done)
        return EMsgResult_NoReply;

    // Return either ok, or nack result now
    if (m_mpData.c1Cmd == tLexMC1S::ECmd_Nack)
    {
        // Return the error code. Remember to add 1 to convert to our format!
        eToFill = tLexMC1S::EErrors(m_mpData.ac1Data[1] + 1);
        return EMsgResult_Nack;
    }
    return EMsgResult_Ok;
}


const tCIDLib::TCard1* TLexMsg::pc1DataAt(const tCIDLib::TCard4 c4Index) const
{
    if (c4Index >= m_mpData.c1DataCount)
    {
        facLexMC1S().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMC1SErrs::errcMsg_BadDataIndex
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::AppError
            , TCardinal(c4Index)
            , TCardinal(m_mpData.c1DataCount)
        );
    }
    return &m_mpData.ac1Data[c4Index];
}


tCIDLib::TVoid TLexMsg::SendOn(         TCommPortBase&      commTarget
                                , const tCIDLib::TCard4     c4Timeout)
{
    const tCIDLib::TCard4 c4Sent = commTarget.c4WriteRawBufMS
    (
        &m_mpData, m_mpData.c1DLLCount + 2, c4Timeout
    );

    if (c4Sent != tCIDLib::TCard4(m_mpData.c1DLLCount + 2))
    {
        facLexMC1S().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMC1SErrs::errcIO_WriteFailure
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotAllWritten
            , TCardinal(m_mpData.c1Cmd)
        );
    }
}


tCIDLib::TVoid TLexMsg::SetAll( const   tLexMC1S::ECmds         eCmd
                                , const tCIDLib::TCard1* const  pc1Data
                                , const tCIDLib::TCard1         c1Bytes)
{
    m_mpData.c1Cmd = tCIDLib::TCard1(eCmd);
    m_mpData.c1DLLCount = c1Bytes + 3;
    m_mpData.c1DataCount = c1Bytes;

    if (c1Bytes)
        TRawMem::CopyMemBuf(m_mpData.ac1Data, pc1Data, c1Bytes);
    m_mpData.ac1Data[c1Bytes] = kLexMC1S::c1EOP;
}


tCIDLib::TVoid TLexMsg::SetData(const   tCIDLib::TCard1* const  pc1Data
                                , const tCIDLib::TCard1         c1Bytes)
{
    m_mpData.c1DataCount = c1Bytes;
    m_mpData.c1DLLCount = c1Bytes + 3;

    if (c1Bytes)
        TRawMem::CopyMemBuf(m_mpData.ac1Data, pc1Data, c1Bytes);
    m_mpData.ac1Data[c1Bytes] = kLexMC1S::c1EOP;
}

