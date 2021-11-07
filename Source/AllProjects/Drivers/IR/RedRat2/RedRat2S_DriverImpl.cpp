//
// FILE NAME: RedRat2S_DriverImpl.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/22/2002
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
//  This file implements the bulk of the driver implementation. As usual, the
//  field related stuff is in a separate file.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "RedRat2S.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TRedRat2SDriver,TBaseIRSrvDriver)


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace RedRat2S_DriverImpl
{
    // Our file format version marker
    const   tCIDLib::TCard4 c4FmtVersion    = 1;
}



// ---------------------------------------------------------------------------
//   CLASS: THostMonSDriver
//  PREFIX: drv
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  THostMonSDriver: Constructors and Destructor
// ---------------------------------------------------------------------------
TRedRat2SDriver::TRedRat2SDriver(const TCQCDriverObjCfg& cqcdcToLoad) :

    TBaseIRSrvDriver(cqcdcToLoad)
    , m_bTrainingMode(kCIDLib::False)
    , m_c4FldIdFirmwareVer(kCIDLib::c4MaxCard)
    , m_c4FldIdInvoke(kCIDLib::c4MaxCard)
    , m_c4FldIdTrainingMode(kCIDLib::c4MaxCard)
    , m_c4NextSend(TTime::c4Millis() + 55)
    , m_c4TrainBytes(0)
    , m_expbPoll(2048UL)
    , m_expbWrite(2048UL)
    , m_pcommRedRat(nullptr)
{
}

TRedRat2SDriver::~TRedRat2SDriver()
{
    // Clean up the comm port if not already
    if (m_pcommRedRat)
    {
        try
        {
            if (m_pcommRedRat->bIsOpen())
                m_pcommRedRat->Close();

            delete m_pcommRedRat;
            m_pcommRedRat = nullptr;
        }

        catch(...)
        {
        }
    }
}


// ---------------------------------------------------------------------------
//  TRedRat2SDriver: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TRedRat2SDriver::bBlastTrainingMode() const
{
    return m_bTrainingMode;
}


tCIDLib::TBoolean
TRedRat2SDriver::bCheckBlastTrainingData(tCIDLib::TCard4&   c4DataBytes
                                        , THeapBuf&         mbufToFill)
{
    // If we aren't online, then throw
    CheckOnline(CID_FILE, CID_LINE);

    const tCIDLib::TBoolean bRet = (m_c4TrainBytes != 0);
    if (bRet)
    {
        // We got something, so copy it over and clear the training flag
        mbufToFill  = m_mbufTrainData;
        c4DataBytes = m_c4TrainBytes;

        m_bTrainingMode = kCIDLib::False;

        bStoreBoolFld(m_c4FldIdTrainingMode, kCIDLib::False, kCIDLib::True);
    }
     else
    {
        c4DataBytes = 0;
    }
    return bRet;
}


tCIDLib::TBoolean TRedRat2SDriver::bGetCommResource(TThread& thrThis)
{
    // Open the port now, and set the configuration
    try
    {
        // Create the port object if needed, else close if open just in case
        if (!m_pcommRedRat)
            m_pcommRedRat = facCIDComm().pcommMakeNew(m_strPort);
        else if (m_pcommRedRat->bIsOpen())
            m_pcommRedRat->Close();

        m_pcommRedRat->Open();
        m_pcommRedRat->SetCfg(m_cpcfgSerial);
    }

    catch(const TError&)
    {
        // <TBD> Should we log something here?
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


tCIDLib::TBoolean TRedRat2SDriver::bResetConnection()
{
    // A no-op for us
    return kCIDLib::True;
}


tCIDLib::TCard4 TRedRat2SDriver::c4InvokeFldId() const
{
    return m_c4FldIdInvoke;
}


tCQCKit::ECommResults TRedRat2SDriver::eConnectToDevice(TThread& thrThis)
{
    //
    //  Call bPollDevice() ourself, since we do the same simple exchange
    //  there that we want to do here to check for connection.
    //
    tCQCKit::ECommResults eRes = tCQCKit::ECommResults::Success;
    eRes = ePollDevice(thrThis);
    if (eRes != tCQCKit::ECommResults::Success)
        return eRes;

    // Build and send the firmware query message
    BuildQuery(kRedRat2S::c1Cmd_Firmware, m_expbWrite);
    WriteMsg(m_expbWrite);

    // And get a reply
    tCIDLib::TCard4 c4Error;
    const tRedRat2S::EResults eMsgRes = eGetMsg(thrThis, 250, m_expbPoll, c4Error);

    // If not our expected reply, then return a failure
    if (eMsgRes == tRedRat2S::EResults::Firmware)
    {
        //
        //  Get it into our field storage. Since we know it's ASCII, we'll
        //  do a cheap transcode here ourself.
        //
        TString strTmp;
        for (tCIDLib::TCard4 c4Index = 0; c4Index < m_expbPoll.c4Bytes(); c4Index++)
            strTmp.Append(tCIDLib::TCh(m_expbPoll[c4Index]));

        bStoreStringFld(m_c4FldIdFirmwareVer, strTmp, kCIDLib::True);
    }

    //
    //  Make sure it's not in training mode, which gets the training mode
    //  field up to date also.
    //
    ExitBlastTrainingMode();
    return eRes;
}


tCQCKit::EDrvInitRes TRedRat2SDriver::eInitializeImpl()
{
    // Call our parent IR base drivers
    TParent::eInitializeImpl();

    //
    //  Make sure that we were configured for a serial connection. Otherwise,
    //  its a bad configuration.
    //
    const TCQCDriverObjCfg& cqcdcOurs = cqcdcThis();
    if (cqcdcOurs.conncfgReal().clsIsA() != TCQCSerialConnCfg::clsThis())
    {
        facCQCKit().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcDrv_BadConnCfgType
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Config
            , cqcdcOurs.conncfgReal().clsIsA()
            , clsIsA()
            , TCQCSerialConnCfg::clsThis()
        );
        return tCQCKit::EDrvInitRes::Failed;
    }

    //
    //  Its the right type so do the down cast and get the data out of it
    //  that we need.
    //
    const TCQCSerialConnCfg& conncfgSer
            = static_cast<const TCQCSerialConnCfg&>(cqcdcOurs.conncfgReal());
    m_strPort = conncfgSer.strPortPath();
    m_cpcfgSerial = conncfgSer.cpcfgSerial();

    // Clean up any existing port so it'll get recreated with new stuff
    delete m_pcommRedRat;
    m_pcommRedRat = nullptr;

    //
    //  Register our couple of fields. We just provide one that indicates
    //  the training mode state, so that the client can display an
    //  indicator that shows whether the driver is in training mode or not.
    //
    TVector<TCQCFldDef> colFlds(8);
    TCQCFldDef flddCmd;

    //
    //  Do readable fields for the standard fields that an IR driver
    //  has to provide. The invoke field must be an 'always write' field
    //  since it exists just to invoke IR data sending.
    //
    flddCmd.Set
    (
        TFacCQCIR::strFldName_FirmwareVer
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Read
    );
    colFlds.objAdd(flddCmd);

    flddCmd.Set
    (
        TFacCQCIR::strFldName_Invoke
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Write
    );
    flddCmd.bAlwaysWrite(kCIDLib::True);
    colFlds.objAdd(flddCmd);

    flddCmd.Set
    (
        TFacCQCIR::strFldName_TrainingState
        , tCQCKit::EFldTypes::Boolean
        , tCQCKit::EFldAccess::Read
    );
    colFlds.objAdd(flddCmd);

    // Tell our base class about our fields
    SetFields(colFlds);

    // Look up the ids of our fields, for efficiency
    m_c4FldIdFirmwareVer = pflddFind
    (
        TFacCQCIR::strFldName_FirmwareVer, kCIDLib::True
    )->c4Id();

    m_c4FldIdInvoke = pflddFind
    (
        TFacCQCIR::strFldName_Invoke, kCIDLib::True
    )->c4Id();

    m_c4FldIdTrainingMode = pflddFind
    (
        TFacCQCIR::strFldName_TrainingState, kCIDLib::True
    )->c4Id();

    //
    //  Set the poll time a little faster than normal. This is a very
    //  interactive device, and it has a good fast speed.
    //
    SetPollTimes(250, 5000);

    //
    //  Return wait config in this case since we need to load our blaster
    //  config. Our parent class handles this for us.
    //
    return tCQCKit::EDrvInitRes::WaitConfig;
}


tCQCKit::ECommResults TRedRat2SDriver::ePollDevice(TThread& thrThis)
{
    //
    //  If we are in traing mode, then just watch for signal data to arrive,
    //  and if we get it, store it for the client to come and get. Else,
    //  just send an empty message, which we just have as a static array
    //  here, since it never changes.
    //
    static const tCIDLib::TSCh aschMsg[] = "[#00]";

    tCQCKit::ECommResults eRetVal = tCQCKit::ECommResults::Success;
    if (m_bTrainingMode)
    {
        tCIDLib::TCard4 c4Error;
        const tRedRat2S::EResults eRes = eGetMsg(thrThis, 250, m_expbPoll, c4Error);

        if (eRes == tRedRat2S::EResults::SignalData)
        {
            // Its our guy, so store the results
            m_c4TrainBytes = m_expbPoll.c4Bytes();
            m_mbufTrainData.CopyIn(m_expbPoll.pc1Buffer(), m_c4TrainBytes);
        }
         else if (eRes == tRedRat2S::EResults::ErrorCode)
        {
            // Log a message, and put us back in signal capture mode again
            facRedRat2S().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kRatSErrs::errcProto_SigError
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Protocol
                , TCardinal(c4Error)
            );
            SendCapMsg(thrThis);
        }
    }
     else
    {
        // It should just send back the empty ack message
        WriteMsg(aschMsg);
        if (!bGetAck(thrThis))
            eRetVal = tCQCKit::ECommResults::LostConnection;
    }
    return eRetVal;
}


tCIDLib::TVoid TRedRat2SDriver::EnterBlastTrainingMode()
{
    // If we aren't online, then throw
    CheckOnline(CID_FILE, CID_LINE);

    // If we are already in training mode, then that's an error
    if (m_bTrainingMode)
    {
        facRedRat2S().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kRatSErrs::errcProto_AlreadyTraining
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Already
        );
    }

    SendCapMsg(*TThread::pthrCaller());

    m_c4TrainBytes  = 0;
    m_bTrainingMode = kCIDLib::True;

    bStoreBoolFld(m_c4FldIdTrainingMode, kCIDLib::True, kCIDLib::True);
}


tCIDLib::TVoid TRedRat2SDriver::ExitBlastTrainingMode()
{
    static const tCIDLib::TSCh aschMsg[] = "[T#54]";

    // Clear the flags first, then send our hard coded message
    m_bTrainingMode = kCIDLib::False;

    bStoreBoolFld(m_c4FldIdTrainingMode, kCIDLib::False, kCIDLib::True);
    m_c4TrainBytes  = 0;

    WriteMsg(aschMsg);

    // And it should just send back the empty ack message
    if (!bGetAck(*TThread::pthrCaller()))
    {
        // <TBD> Can we do much about this? Should we go offline?
    }
}


tCIDLib::TVoid
TRedRat2SDriver::InvokeBlastCmd(const   TString&        strDevice
                                , const TString&        strCmd
                                , const tCIDLib::TCard4 c4ZoneNum)
{
    //
    //  We can check the zone number up front without locking. We only
    //  support zone 0, which is what any open air blaster should say it
    //  supports.
    //
    //  Other than it being a valid zone zero, we ignore it.
    //
    if (c4ZoneNum)
    {
        facCQCIR().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kIRErrs::errcBlast_BadZone
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::CantDo
            , TCardinal(c4ZoneNum)
            , strMoniker()
        );
    }

    // If we aren't online, then throw
    CheckOnline(CID_FILE, CID_LINE);

    //
    //  Look up the command. We don't use the repeat count for this
    //  driver.
    //
    tCIDLib::TCard4 c4Repeat;
    const TIRBlasterCmd& irbcInvoke = irbcFromName(strDevice, strCmd, c4Repeat);

    //
    //  Build up the message that we need to send, and send it. Wait for
    //  an ack.
    //
    BuildQuery
    (
        kRedRat2S::c1Cmd_Transmit
        , irbcInvoke.mbufData()
        , irbcInvoke.c4DataLen()
        , m_expbWrite
    );

    // Send it, and wait for an ack
    WriteMsg(m_expbWrite);
    bGetAck(*TThread::pthrCaller(), kCIDLib::True);
}


tCIDLib::TVoid TRedRat2SDriver::ReleaseCommResource()
{
    try
    {
        if (m_pcommRedRat->bIsOpen())
            m_pcommRedRat->Close();
    }

    catch(const TError& errToCatch)
    {
        LogError(errToCatch, tCQCKit::EVerboseLvls::Medium);
    }
}


tCIDLib::TVoid
TRedRat2SDriver::SendBlasterData(const  tCIDLib::TCard4 c4DataBytes
                                , const TMemBuf&        mbufToSend
                                , const tCIDLib::TCard4 c4ZoneNum
                                , const tCIDLib::TCard4 c4RepeatCount)
{
    //
    //  We can check the zone number up front without locking. We only
    //  support zone 0, which is what any open air blaster should say it
    //  supports.
    //
    //  Other than it being a valid zone zero, we ignore it.
    //
    if (c4ZoneNum)
    {
        facCQCIR().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kIRErrs::errcBlast_BadZone
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::CantDo
            , TCardinal(c4ZoneNum)
            , strMoniker()
        );
    }

    // If we aren't online, then throw, else do it
    CheckOnline(CID_FILE, CID_LINE);

    //
    //  Build up the message that we need to send, and send it. Wait for
    //  an ack.
    //
    BuildQuery(kRedRat2S::c1Cmd_Transmit, mbufToSend, c4DataBytes, m_expbWrite);
    WriteMsg(m_expbWrite);
    bGetAck(*TThread::pthrCaller(), kCIDLib::True);
}


tCIDLib::TVoid TRedRat2SDriver::TerminateImpl()
{

    // Pass it on to our parent now
    TParent::TerminateImpl();
}



// ---------------------------------------------------------------------------
//  TRedRat2SDriver: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TRedRat2SDriver::AppendCheckSum(TExpByteBuf& expbTarget)
{
    // Sanity check the format
    if (expbTarget[0] != kRedRat2S::c1Proto_SOM)
    {
        facRedRat2S().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kRatSErrs::errcProto_NoSOM
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Protocol
        );
    }

    if (expbTarget.c1Last() != kRedRat2S::c1Proto_Divider)
    {
        facRedRat2S().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kRatSErrs::errcProto_NoDivider
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Protocol
        );
    }

    //
    //  The bytes after the open bracket and up to but not including the
    //  divider, go into the check sum.
    //
    tCIDLib::TCard1 c1Sum = 0;
    tCIDLib::TCard4 c4Index = 1;
    while (expbTarget[c4Index] != kRedRat2S::c1Proto_Divider)
        c1Sum += expbTarget[c4Index++];

    // Convert this into two two ASCII hex digits
    tCIDLib::TSCh chOne;
    tCIDLib::TSCh chTwo;
    TString::FromHex(c1Sum, chOne, chTwo);
    expbTarget.Append(chOne);
    expbTarget.Append(chTwo);
}


tCIDLib::TBoolean
TRedRat2SDriver::bGetAck(       TThread&            thrThis
                        , const tCIDLib::TBoolean   bThrowIfNot)
{
    tCIDLib::TCard4 c4Error;
    const tRedRat2S::EResults eRes = eGetMsg(thrThis, 500, m_expbPoll, c4Error);
    if ((eRes != tRedRat2S::EResults::Ack))
    {
        if (bThrowIfNot)
        {
            if (eRes == tRedRat2S::EResults::ErrorCode)
            {
                facRedRat2S().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kRatSErrs::errcProto_SigError
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Protocol
                    , TCardinal(c4Error)
                );
            }

            facRedRat2S().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kRatSErrs::errcProto_NoAck
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Protocol
            );
        }
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


tCIDLib::TVoid
TRedRat2SDriver::BuildQuery( const   tCIDLib::TSCh   chCmd
                            ,       TExpByteBuf&    expbTarget)
{
    expbTarget.Reset();
    expbTarget.Append(kRedRat2S::c1Proto_SOM);
    expbTarget.Append(chCmd);
    expbTarget.Append(kRedRat2S::c1Proto_Divider);
    AppendCheckSum(expbTarget);
    expbTarget.Append(kRedRat2S::c1Proto_EOM);
}

tCIDLib::TVoid
TRedRat2SDriver::BuildQuery( const   tCIDLib::TSCh   chCmd
                            , const TMemBuf&        mbufData
                            , const tCIDLib::TCard4 c4Bytes
                            ,       TExpByteBuf&    expbTarget)
{
    expbTarget.Reset();
    expbTarget.Append(kRedRat2S::c1Proto_SOM);
    expbTarget.Append(chCmd);
    expbTarget.Append(mbufData.pc1Data(), c4Bytes);
    expbTarget.Append(kRedRat2S::c1Proto_Divider);
    AppendCheckSum(expbTarget);
    expbTarget.Append(kRedRat2S::c1Proto_EOM);
}


tCIDLib::TCard1
TRedRat2SDriver::c1ConvertHexChars(  const   tCIDLib::TCard1 c1First
                                    , const tCIDLib::TCard1 c1Sec)
{
    tCIDLib::TCard1 c1Ret = TString::c1ToHex(tCIDLib::TSCh(c1First));
    c1Ret <<= 4;
    c1Ret |= TString::c1ToHex(tCIDLib::TSCh(c1Sec));
    return c1Ret;
}


tRedRat2S::EResults
TRedRat2SDriver::eGetMsg(       TThread&            thrThis
                        , const tCIDLib::TCard4     c4WaitFor
                        ,       TExpByteBuf&        expbData
                        ,       tCIDLib::TCard4&    c4Error)
{
    //
    //  We have to character by character, so we do a little state machine
    //  so as to catch bad data as soon as possible and not waste time, and
    //  to know that its validated by the time we hit the end.
    //
    enum class EStates
    {
        Waiting
        , OpenBracket
        , Command
        , Data
        , CheckSum1
        , CheckSum2
        , CloseBracket
        , CR
        , LF
        , Prompt
        , Complete
    };

    // Reset the count on our buffer
    expbData.Reset();

    // And initialize our state machine value
    EStates eCurState = EStates::Waiting;

    // And init the returned result to unknown
    tRedRat2S::EResults eRes = tRedRat2S::EResults::Unknown;

    // Calculate the end time when we'll give up
    tCIDLib::TCard4 c4EndTime = TTime::c4Millis() + c4WaitFor;

    //
    //  These store the two checksum bytes that we get in the message. They
    //  will be used at the end to compare to the one we calculate.
    //
    tCIDLib::TCard1     c1Sum1     = 0;
    tCIDLib::TCard1     c1Sum2     = 0;
    tCIDLib::TCard1     c1OurSum   = 0;

    tCIDLib::TBoolean   bExtended = kCIDLib::False;
    tCIDLib::TCard1     c1Cur;
    while (eCurState < EStates::Complete)
    {
        //
        //  See if we've gone over the time limit. However, since a message
        //  can come in at any time in some cases (such as during training),
        //  if we have moved beyond the Waiting state, add on a little more
        //  time so that we can finish processing this message. Otherwise,
        //  we'll loose it.
        //
        //  We'll only expand it once.
        //
        if (TTime::c4Millis() > c4EndTime)
        {
            if ((eCurState > EStates::Waiting) && !bExtended)
            {
                c4EndTime += 150;
                bExtended = kCIDLib::True;
            }
             else
            {
                break;
            }
        }

        //
        //  Wait for the next char. If we don't get one, the go back to the
        //  top until we run out of time.
        //
        if (!m_pcommRedRat->c4ReadRawBufMS(&c1Cur, 1, 50))
            continue;

        switch(eCurState)
        {
            case EStates::Waiting :
                //
                //  If its not the opening bracket, then we've gotten out
                //  of sync somehow and need to just ignore bytes until we
                //  get the opening byte. If it is our go, move to the next
                //  state.
                //
                if (c1Cur == kRedRat2S::c1Proto_SOM)
                    eCurState = EStates::OpenBracket;
                break;

            case EStates::OpenBracket :
                //
                //  We have two options here. Either its an empty message,
                //  an ack, or its the command. Its at this point that we
                //  can set the return value.
                //
                switch(tCIDLib::TCh(c1Cur))
                {
                    case kRedRat2S::c1Proto_Divider :
                        eRes = tRedRat2S::EResults::Ack;
                        break;

                    case kRedRat2S::c1Cmd_Firmware :
                        eRes = tRedRat2S::EResults::Firmware;
                        break;

                    case kRedRat2S::c1Cmd_SignalCap :
                        eRes = tRedRat2S::EResults::SignalData;
                        break;

                    case kRedRat2S::c1Cmd_ErrorCode :
                        eRes = tRedRat2S::EResults::ErrorCode;
                        break;

                    case kRedRat2S::c1Cmd_EPROM :
                        eRes = tRedRat2S::EResults::EPROMValue;
                        break;

                    default :
                        // Its not something that should be here
                        IncUnknownMsgCounter();
                        facRedRat2S().ThrowErr
                        (
                            CID_FILE
                            , CID_LINE
                            , kRatSErrs::errcProto_UnknownCmd
                            , tCIDLib::ESeverities::Failed
                            , tCIDLib::EErrClasses::Format
                            , TCardinal(c1Cur)
                        );
                        break;
                }

                // Move to the command state now, unless its an ack
                if (eRes == tRedRat2S::EResults::Ack)
                {
                    eCurState = EStates::CheckSum1;
                }
                 else
                {
                    eCurState = EStates::Command;
                    c1OurSum += c1Cur;
                }
                break;

            case EStates::Command :
                //
                //  If its a # sound, then move to the check sum state.
                //  Else, if its its alphanum, its command data, so move
                //  to the data state.
                //
                if (c1Cur == kRedRat2S::c1Proto_Divider)
                {
                    eCurState = EStates::CheckSum1;
                }
                 else if (TRawStr::bIsAlphaNum(tCIDLib::TCh(c1Cur)))
                {
                    eCurState = EStates::Data;
                    expbData.Append(c1Cur);
                }
                 else
                {
                    IncBadMsgCounter();
                    facRedRat2S().ThrowErr
                    (
                        CID_FILE
                        , CID_LINE
                        , kRatSErrs::errcProto_ExpectedCmd
                        , tCIDLib::ESeverities::Failed
                        , tCIDLib::EErrClasses::Format
                        , TCardinal(c1Cur)
                    );
                }
                break;

            case EStates::Data :
                // We store data bytes until we get the # sign
                if (c1Cur == kRedRat2S::c1Proto_Divider)
                {
                    eCurState = EStates::CheckSum1;
                }
                 else if (c1Cur < 128)
                {
                    expbData.Append(c1Cur);
                }
                 else
                {
                    IncBadMsgCounter();
                    facRedRat2S().ThrowErr
                    (
                        CID_FILE
                        , CID_LINE
                        , kRatSErrs::errcProto_BadDataByte
                        , tCIDLib::ESeverities::Failed
                        , tCIDLib::EErrClasses::Format
                        , TCardinal(c1Cur)
                    );
                }
                break;

            case EStates::CheckSum1 :
                c1Sum1 = c1Cur;
                eCurState = EStates::CheckSum2;
                break;

            case EStates::CheckSum2 :
                c1Sum2 = c1Cur;
                eCurState = EStates::CloseBracket;
                break;

            case EStates::CloseBracket :
                // We must see the close bracket now
                if (c1Cur != kRedRat2S::c1Proto_EOM)
                {
                    IncBadMsgCounter();
                    facRedRat2S().ThrowErr
                    (
                        CID_FILE
                        , CID_LINE
                        , kRatSErrs::errcProto_ExpectedClose
                        , tCIDLib::ESeverities::Failed
                        , tCIDLib::EErrClasses::Format
                        , TCardinal(c1Cur)
                    );
                }
                eCurState = EStates::CR;
                break;

            case EStates::CR :
                // We must see the CR now
                if (c1Cur != kRedRat2S::c1Proto_CR)
                {
                    IncBadMsgCounter();
                    facRedRat2S().ThrowErr
                    (
                        CID_FILE
                        , CID_LINE
                        , kRatSErrs::errcProto_ExpectedCR
                        , tCIDLib::ESeverities::Failed
                        , tCIDLib::EErrClasses::Format
                        , TCardinal(c1Cur)
                    );
                }
                eCurState = EStates::LF;
                break;

            case EStates::LF :
                // We must see the LF now
                if (c1Cur != kRedRat2S::c1Proto_LF)
                {
                    IncBadMsgCounter();
                    facRedRat2S().ThrowErr
                    (
                        CID_FILE
                        , CID_LINE
                        , kRatSErrs::errcProto_ExpectedLF
                        , tCIDLib::ESeverities::Failed
                        , tCIDLib::EErrClasses::Format
                        , TCardinal(c1Cur)
                    );
                }
                eCurState = EStates::Prompt;
                break;

            case EStates::Prompt :
                // We must see the > now
                if (c1Cur != kRedRat2S::c1Proto_Prompt)
                {
                    IncBadMsgCounter();
                    facRedRat2S().ThrowErr
                    (
                        CID_FILE
                        , CID_LINE
                        , kRatSErrs::errcProto_ExpectedGT
                        , tCIDLib::ESeverities::Failed
                        , tCIDLib::EErrClasses::Format
                        , TCardinal(c1Cur)
                    );
                }

                // We are done, with at least a basically correct message
                eCurState = EStates::Complete;
                break;

            default :
                facRedRat2S().ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kRatSErrs::errcProto_UnknownState
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Format
                    , TCardinal(tCIDLib::c4EnumOrd(eCurState))
                );
                break;
        }

        //
        //  We haven't gotten it yet, so check for a shutdown request
        //  and break out if so. If not, go back and try again.
        //
        if (thrThis.bCheckShutdownRequest())
            break;
    }

    // If we didn't get a complete message, then we are done
    if (eCurState == EStates::Waiting)
    {
        IncTimeoutCounter();
        return tRedRat2S::EResults::NoReply;
    }
     else if (eCurState != EStates::Complete)
    {
        IncBadMsgCounter();
        return tRedRat2S::EResults::Unknown;
    }

    //
    //  Calculate the checksum on the data bytes we got. We already included
    //  the command byte above.
    //
    for (tCIDLib::TCard1 c4Index = 0; c4Index < expbData.c4Bytes(); c4Index++)
        c1OurSum += expbData[c4Index];

    // Compare it to the one we got in the message
    const tCIDLib::TCard1 c1HisSum = c1ConvertHexChars(c1Sum1, c1Sum2);
    if (c1OurSum != c1HisSum)
    {
        IncBadMsgCounter();
        facRedRat2S().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kRatSErrs::errcProto_BadCheckSum
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(c1HisSum)
            , TCardinal(c1OurSum)
        );
    }

    // If its an error code, then translate that and store it
    if (eRes == tRedRat2S::EResults::ErrorCode)
    {
        // We had to have gotten 2 bytes for an error code
        if (expbData.c4Bytes() != 2)
        {
            facRedRat2S().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kRatSErrs::errcProto_BadErrData
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
            );
        }
        c4Error = c1ConvertHexChars(expbData[0], expbData[1]);
    }
    return eRes;
}


tCIDLib::TVoid TRedRat2SDriver::SendCapMsg(TThread& thrCaller)
{
    static const tCIDLib::TSCh aschMsg[] = "[S#53]";

    //
    //  Send the RedRat our fixed message, which puts it into signal
    //  capture mode.
    //
    WriteMsg(aschMsg);

    // And it should just send back the empty ack message
    if (!bGetAck(thrCaller))
    {
        facRedRat2S().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kRatSErrs::errcProto_CapNotAcked
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotAllWritten
        );
    }
}


tCIDLib::TVoid TRedRat2SDriver::WriteMsg(const TExpByteBuf& expbSrc)
{
    // If the minimum inter-send time hasn't expired, then wait
    const tCIDLib::TCard4 c4CurTime = TTime::c4Millis();
    if (c4CurTime < m_c4NextSend)
        TThread::Sleep(m_c4NextSend - c4CurTime);

    const tCIDLib::TCard4 c4Bytes = expbSrc.c4Bytes();
    const tCIDLib::TCard4 c4Sent = m_pcommRedRat->c4WriteRawBufMS
    (
        expbSrc.pc1Buffer(), c4Bytes, 250
    );

    if (c4Sent != c4Bytes)
    {
        IncFailedWriteCounter();
        facRedRat2S().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kRatSErrs::errcComm_NotAllWritten
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotAllWritten
            , TCardinal(c4Bytes)
            , TCardinal(c4Sent)
        );
    }

    //
    //  The RedRat needs 50ms after any send in order to get back to listening
    //  again. So remember this last send time.
    //
    m_c4NextSend = TTime::c4Millis() + 55;
}

tCIDLib::TVoid TRedRat2SDriver::WriteMsg(const tCIDLib::TSCh* const pchSrc)
{
    // If the minimum inter-send time hasn't expired, then wait
    const tCIDLib::TCard4 c4CurTime = TTime::c4Millis();
    if (c4CurTime < m_c4NextSend)
        TThread::Sleep(m_c4NextSend - c4CurTime);

    const tCIDLib::TCard4 c4Bytes = TRawStr::c4StrLen(pchSrc);
    const tCIDLib::TCard4 c4Sent = m_pcommRedRat->c4WriteRawBufMS(pchSrc, c4Bytes, 500);

    if (c4Sent != c4Bytes)
    {
        IncFailedWriteCounter();
        facRedRat2S().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kRatSErrs::errcComm_NotAllWritten
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotAllWritten
            , TCardinal(c4Bytes)
            , TCardinal(c4Sent)
        );
    }

    //
    //  The RedRat needs 50ms after any send in order to get back to listening
    //  again. So remember the next legal time (with a little slop.)
    //
    m_c4NextSend = TTime::c4Millis() + 55;
}


