//
// FILE NAME: GenProtoS_DriverImpl.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/22/2003
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
//  This is a driver which implements the main driver logic for the generic
//  protocol driver.
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
//  Local data
// ---------------------------------------------------------------------------
namespace GenProtoS_DriverImpl
{
}


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TGenProtoSDriver,TCQCServerBase)


// ---------------------------------------------------------------------------
//   CLASS: TGenProtoSDriver
//  PREFIX: drv
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  THostMonSDriver: Constructors and Destructor
// ---------------------------------------------------------------------------
TGenProtoSDriver::TGenProtoSDriver(const TCQCDriverObjCfg& cqcdcToLoad) :

    TCQCServerBase(cqcdcToLoad)
    , m_bDeadMode(kCIDLib::False)
    , m_bTestMode(kCIDLib::False)
    , m_c4AcceptStateInd(0)
    , m_c4LastSendTime(0)
    , m_c4MinSendInterval(0)
    , m_c4PostConnWait(0)
    , m_c4Timeouts(0)
    , m_c4TimeoutMax(2)
    , m_colFields(64)
    , m_colMaps(kGenProtoS_::c4Modulus, TStringKeyOps(), &TGenProtoMap::strKey)
    , m_colMatches(tCIDLib::EAdoptOpts::Adopt)
    , m_colQueries(kGenProtoS_::c4Modulus, TStringKeyOps(), &TGenProtoQuery::strKey)
    , m_colReplies(kGenProtoS_::c4Modulus, TStringKeyOps(), &TGenProtoReply::strKey)
    , m_colStateMachine(tCIDLib::EAdoptOpts::Adopt)
    , m_colVariables(kGenProtoS_::c4Modulus, TStringKeyOps(), &TGenProtoVarInfo::strKey)
    , m_ctxQuery(tGenProtoS::ESpecNodes::Query, L"query")
    , m_ctxReply(tGenProtoS::ESpecNodes::StateMachine, L"reply")
    , m_ctxWriteCmd(tGenProtoS::ESpecNodes::WriteCmd, L"write command")
    , m_eMsgToShow(tGenProtoS::EDbgMsgs::Default)
    , m_eProtoType(tGenProtoS::EProtoTypes::TwoWay)
    , m_pdevcTar(nullptr)
    , m_pmdbgCallback(nullptr)
{
    // Tell the context object about us
    m_ctxQuery.SetDriver(this);
    m_ctxReply.SetDriver(this);
    m_ctxWriteCmd.SetDriver(this);
}

TGenProtoSDriver::~TGenProtoSDriver()
{
    // Clean up the device connection if not already
    try
    {
        delete m_pdevcTar;
    }

    catch(...)
    {
        facGenProtoS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kGPDMsgs::midStatus_DevConnCleanup
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::InitFailed
            , strMoniker()
        );
    }
}


// ---------------------------------------------------------------------------
//  TGenProtoSDriver: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TGenProtoSDriver::bTestMode() const
{
    return m_bTestMode;
}

tCIDLib::TBoolean TGenProtoSDriver::bTestMode(const tCIDLib::TBoolean bToSet)
{
    m_bTestMode = bToSet;
    return m_bTestMode;
}


tGenProtoS::EDbgMsgs TGenProtoSDriver::eMsgToShow() const
{
    return m_eMsgToShow;
}

tGenProtoS::EDbgMsgs
TGenProtoSDriver::eMsgToShow(const tGenProtoS::EDbgMsgs eToSet)
{
    m_eMsgToShow = eToSet;
    return m_eMsgToShow;
}


tCIDLib::TVoid TGenProtoSDriver::HideMsgs(const tGenProtoS::EDbgMsgs eToHide)
{
    m_eMsgToShow = tCIDLib::eClearEnumBits(m_eMsgToShow, eToHide);
}


tCIDLib::TVoid
TGenProtoSDriver::SetDebugCallback(MGenProtoDebug* const pmdbgToUse)
{
    m_pmdbgCallback = pmdbgToUse;
}


tCIDLib::TVoid TGenProtoSDriver::ShowMsgs(const tGenProtoS::EDbgMsgs eToShow)
{
    m_eMsgToShow = tCIDLib::eOREnumBits(m_eMsgToShow, eToShow);
}


// ---------------------------------------------------------------------------
//  TGenProtoSDriver: Protected, inherited methods
// ---------------------------------------------------------------------------

// Called so that we can obtain our comm resource
tCIDLib::TBoolean TGenProtoSDriver::bGetCommResource(TThread& thrThis)
{
    if (m_pdevcTar->bGetCommResource(thrThis, eVerboseLevel()))
    {
        if ((m_pmdbgCallback != nullptr)
        &&  tCIDLib::bAllBitsOn(m_eMsgToShow, tGenProtoS::EDbgMsgs::CommRes))
        {
            m_pmdbgCallback->Msg(L"Got comm res");
        }
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


//
//  We override this one because we also want to check for our having a
//  callback installed. If we do, then we are in the test harness and want
//  to log it even if it's not over the minimum level because they are
//  going to the test harness in that case.
//
//  We don't bother to call the parent class in this case.
//
tCIDLib::TBoolean
TGenProtoSDriver::bShouldLog(const  TError&                 errToCatch
                            , const tCQCKit::EVerboseLvls   eMinLevel) const
{
    // If already logged, then no need to do it
    if (errToCatch.bLogged())
        return kCIDLib::False;
    return (eVerboseLevel() >= eMinLevel) || (m_pmdbgCallback != 0);
}


// Called back to let us get any config, which we don't have any of
tCIDLib::TBoolean TGenProtoSDriver::bWaitConfig(TThread& thrThis)
{
    // Just say we are happy
    return kCIDLib::True;
}


tCQCKit::ECommResults
TGenProtoSDriver::eBoolFldValChanged(const  TString&            strName
                                    , const tCIDLib::TCard4     c4FldId
                                    , const tCIDLib::TBoolean   bNewValue)
{
    tCQCKit::ECommResults eRes = tCQCKit::ECommResults::ValueRejected;
    try
    {
        if ((m_pmdbgCallback != nullptr)
        &&  tCIDLib::bAllBitsOn(m_eMsgToShow, tGenProtoS::EDbgMsgs::FldWrite))
        {
            m_pmdbgCallback->Msg(L"Got write for boolean field ", strName);
        }

        //
        //  Find this field and check that the name matches the name that we have
        //  for this field id.
        //
        TGenProtoFldInfo* pfldiTar = pfldiValidateId(strName, c4FldId);
        if (pfldiTar)
        {
            //
            //  Set up the value object in the write command context. This
            //  will make the value available to expressions. We have to set
            //  the type as well since it floats with the value written.
            //
            m_ctxWriteCmd.evalWrite().Reset(tGenProtoS::ETypes::Boolean);
            m_ctxWriteCmd.evalWrite().bValue(bNewValue);

            // And try to do the write
            eRes = eWriteField(*pfldiTar);
        }
         else
        {
            IncUnknownWriteCounter();
            eRes = tCQCKit::ECommResults::FieldNotFound;
        }
    }

    catch(TError& errToCatch)
    {
        if (bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        if (m_pmdbgCallback != nullptr)
            m_pmdbgCallback->Msg(L"Exception occured during field write. See logs");

        eRes = tCQCKit::ECommResults::Exception;
    }
    return eRes;
}


tCQCKit::ECommResults
TGenProtoSDriver::eCardFldValChanged(const  TString&        strName
                                    , const tCIDLib::TCard4 c4FldId
                                    , const tCIDLib::TCard4 c4NewValue)

{
    tCQCKit::ECommResults eRes = tCQCKit::ECommResults::ValueRejected;
    try
    {
        if ((m_pmdbgCallback != nullptr)
        &&  tCIDLib::bAllBitsOn(m_eMsgToShow, tGenProtoS::EDbgMsgs::FldWrite))
        {
            m_pmdbgCallback->Msg(L"Got write for card field ", strName);
        }

        //
        //  Find this field and check that the name matches the name that we have
        //  for this field id.
        //
        TGenProtoFldInfo* pfldiTar = pfldiValidateId(strName, c4FldId);
        if (pfldiTar)
        {
            //
            //  Set up the value object in the write command context. This
            //  will make the value available to expressions. We have to set
            //  the type as well since it floats with the value written.
            //
            m_ctxWriteCmd.evalWrite().Reset(tGenProtoS::ETypes::Card4);
            m_ctxWriteCmd.evalWrite().c4Value(c4NewValue);

            // Now call the common field writing method
            eRes = eWriteField(*pfldiTar);
        }
         else
        {
            IncUnknownWriteCounter();
            eRes = tCQCKit::ECommResults::FieldNotFound;
        }
    }

    catch(TError& errToCatch)
    {
        if (bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        if (m_pmdbgCallback != nullptr)
            m_pmdbgCallback->Msg(L"Exception occured during field write. See logs");

        eRes = tCQCKit::ECommResults::Exception;
    }
    return eRes;
}


//
//
//  If we are a 'dead if off' device, and any exchanges fail, then we don't
//  stay offline, we just put all the fields but one into error state and
//  return success. This leaves the one field available to force the device
//  on. This is 'dead mode', and we have the m_bDeadMode flag. If we actually
//  do get connected, we'll write values to all the fields, which will take
//  them out of error mode, and we'll clear the flag.
//
tCQCKit::ECommResults TGenProtoSDriver::eConnectToDevice(TThread& thrThis)
{
    tCQCKit::ECommResults eRes = tCQCKit::ECommResults::LostCommRes;
    try
    {
        // And do the try connect and reconnect cycle
        eRes = eDoConnect(thrThis);

        //
        //  Make sure we are out of dead mode if successful. Else, if we failed,
        //  we still return success, but enter dead mode. In this case, we want
        //  to force SetDeadMode to do something, so we set the flag to the
        //  opposite of what we pass in.
        //
        if (m_eProtoType == tGenProtoS::EProtoTypes::DeadIfOff)
        {
            if (eRes == tCQCKit::ECommResults::Success)
            {
                m_bDeadMode = kCIDLib::True;
                SetDeadMode(kCIDLib::False, thrThis);
            }
             else
            {
                m_bDeadMode = kCIDLib::False;
                SetDeadMode(kCIDLib::True, thrThis);
                eRes = tCQCKit::ECommResults::Success;
            }
        }
    }

    catch(TError& errToCatch)
    {
        if (bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        if (m_pmdbgCallback != nullptr)
            m_pmdbgCallback->Msg(L"Exception occured during connect. See logs");
        eRes = tCQCKit::ECommResults::LostCommRes;
    }
    return eRes;
}


tCQCKit::EDrvInitRes TGenProtoSDriver::eInitializeImpl()
{
    //
    //  The 'server library' name is the name of the protocol file
    //  to load.
    //
    const TString& strProto = cqcdcThis().strServerLibName();

    //
    //  If in test mode, then we've already been asked to parse a protocol
    //  file contents given to us by the test harness, else we need to
    //  load it up ourself.
    //
    if (!m_bTestMode)
    {
        //
        //  Get an installation server client proxy and ask for the file
        //  contents for this protocol.
        //
        tCIDLib::TCard4 c4SrcBytes;
        THeapBuf        mbufSrc;
        try
        {
            tCQCKit::TInstSrvProxy orbcIS = facCQCKit().orbcInstSrvProxy(5000);
            if (!orbcIS->bQueryProtoFile(strProto, c4SrcBytes, mbufSrc))
            {
                if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
                {
                    facGenProtoS().LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , kGPDMsgs::midStatus_ProtoNotFound
                        , tCIDLib::ESeverities::Failed
                        , tCIDLib::EErrClasses::NotFound
                        , strProto
                        , strMoniker()
                    );
                }
                return tCQCKit::EDrvInitRes::Failed;
            }
        }

        catch(TError& errToCatch)
        {
            if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
            {
                if (!errToCatch.bLogged())
                {
                    errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                    TModule::LogEventObj(errToCatch);
                }

                facGenProtoS().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kGPDMsgs::midStatus_DownloadPDL
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::NotFound
                    , strProto
                    , strMoniker()
                );
            }
            return tCQCKit::EDrvInitRes::Failed;
        }

        //
        //  Try to parse the protocol data. We have to contact the data server
        //  and get the protocol contents.
        //
        try
        {
            // Clean out all of the lists
            m_colFields.RemoveAll();
            m_colMaps.RemoveAll();
            m_colMatches.RemoveAll();
            m_colPollEvents.RemoveAll();
            m_colQueries.RemoveAll();
            m_colReconnect.RemoveAll();
            m_colReplies.RemoveAll();
            m_colStateMachine.RemoveAll();
            m_colVariables.RemoveAll();

            ParseProtocol(mbufSrc, c4SrcBytes);
        }

        catch(TError& errToCatch)
        {
            if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
            {
                if (!errToCatch.bLogged())
                {
                    errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                    TModule::LogEventObj(errToCatch);
                }

                facGenProtoS().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kGPDMsgs::midStatus_ProtoParseFailed
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::NotFound
                    , strProto
                    , strMoniker()
                );
            }
            return tCQCKit::EDrvInitRes::Failed;
        }
    }

    // If a dead if off type protocol, then start off in dead mode
    if (m_eProtoType == tGenProtoS::EProtoTypes::DeadIfOff)
        m_bDeadMode = kCIDLib::True;

    //
    //  And now based on connection config, create the correct device
    //  connection object and store it. If we have an existing one,
    //  kill it and create a new one since the config might have changed.
    //
    try
    {
        if (m_pdevcTar)
        {
            m_pdevcTar->ReleaseCommResource(eVerboseLevel());
            delete m_pdevcTar;
            m_pdevcTar = 0;
        }

        if (cqcdcThis().conncfgReal().clsIsA() == TCQCIPConnCfg::clsThis())
        {
            m_pdevcTar = new TSockConn
            (
                static_cast<const TCQCIPConnCfg&>(cqcdcThis().conncfgReal())
            );
        }
         else if (cqcdcThis().conncfgReal().clsIsA() == TCQCSerialConnCfg::clsThis())
        {
            m_pdevcTar = new TSerialConn
            (
                static_cast<const TCQCSerialConnCfg&>(cqcdcThis().conncfgReal())
            );
        }
         else
        {
            // It has to be serial or socket connection
            if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
            {
                facGenProtoS().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kGPDMsgs::midStatus_BadConnType
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::InitFailed
                    , strMoniker()
                    , strProto
                );
            }
            return tCQCKit::EDrvInitRes::Failed;
        }
    }

    catch(TError& errToCatch)
    {
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
        {
            if (!errToCatch.bLogged())
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }

            facGenProtoS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kGPDMsgs::midStatus_DevConnInitFailed
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::InitFailed
                , strMoniker()
                , strProto
            );
        }
        return tCQCKit::EDrvInitRes::Failed;
    }

    //
    //  Register the fields that were defined in the protocol. We interate
    //  through the list of our own field def objects and create a vector of
    //  CQC field def objects, which we then register.
    //
    TCQCServerBase::TSetFieldList   colRegList;
    const tCIDLib::TCard4 c4Count = m_colFields.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        colRegList.objAdd(m_colFields[c4Index].flddInfo());
    SetFields(colRegList);

    //
    //  Go back now and look up the ids of all of the fields and update the
    //  field info list with the ids.
    //
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TCQCFldDef& flddCur = m_colFields[c4Index].flddInfo();
        flddCur.c4Id(pflddFind(flddCur.strName(), kCIDLib::True)->c4Id());
    }

    //
    //  Crank the poll time down low, because we want to handle async msgs
    //  quickly, if the device sends them. Otherwise, the poll callback will
    //  just check for poll exchanges that have come due, so there's no real
    //  overhead in involved in having a fast poll time.
    //
    SetPollTimes(50, 5000);

    if ((m_pmdbgCallback != nullptr)
    &&  tCIDLib::bAllBitsOn(m_eMsgToShow, tGenProtoS::EDbgMsgs::CommRes))
    {
        m_pmdbgCallback->Msg(L"Initialized successfully, waiting for comm res");
    }

    return tCQCKit::EDrvInitRes::WaitCommRes;
}



tCQCKit::ECommResults
TGenProtoSDriver::eFloatFldValChanged(  const   TString&            strName
                                        , const tCIDLib::TCard4     c4FldId
                                        , const tCIDLib::TFloat8    f8NewValue)
{
    tCQCKit::ECommResults eRes = tCQCKit::ECommResults::ValueRejected;
    try
    {
        if ((m_pmdbgCallback != nullptr)
        &&  tCIDLib::bAllBitsOn(m_eMsgToShow, tGenProtoS::EDbgMsgs::FldWrite))
        {
            m_pmdbgCallback->Msg(L"Got write for float field ", strName);
        }

        //
        //  Find this field and check that the name matches the name that we have
        //  for this field id.
        //
        TGenProtoFldInfo* pfldiTar = pfldiValidateId(strName, c4FldId);
        if (pfldiTar)
        {
            //
            //  Set up the value object in the write command context. This
            //  will make the value available to expressions. We have to set
            //  the type as well since it floats with the value written.
            //
            m_ctxWriteCmd.evalWrite().Reset(tGenProtoS::ETypes::Float8);
            m_ctxWriteCmd.evalWrite().f8Value(f8NewValue);

            // Now call the common field writing method
            eRes = eWriteField(*pfldiTar);
        }
         else
        {
            IncUnknownWriteCounter();
            eRes = tCQCKit::ECommResults::FieldNotFound;
        }
    }

    catch(TError& errToCatch)
    {
        if (bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        if (m_pmdbgCallback != nullptr)
            m_pmdbgCallback->Msg(L"Exception occured during field write. See logs");

        eRes = tCQCKit::ECommResults::Exception;
    }
    return eRes;
}



tCQCKit::ECommResults
TGenProtoSDriver::eIntFldValChanged(const   TString&        strName
                                    , const tCIDLib::TCard4 c4FldId
                                    , const tCIDLib::TInt4  i4NewValue)
{
    tCQCKit::ECommResults eRes = tCQCKit::ECommResults::ValueRejected;
    try
    {
        if ((m_pmdbgCallback != nullptr)
        &&  tCIDLib::bAllBitsOn(m_eMsgToShow, tGenProtoS::EDbgMsgs::FldWrite))
        {
            m_pmdbgCallback->Msg(L"Got write for int field ", strName);
        }

        //
        //  Find this field and check that the name matches the name that we have
        //  for this field id.
        //
        TGenProtoFldInfo* pfldiTar = pfldiValidateId(strName, c4FldId);
        if (pfldiTar)
        {
            //
            //  Set up the value object in the write command context. This
            //  will make the value available to expressions. We have to set
            //  the type as well since it floats with the value written.
            //
            m_ctxWriteCmd.evalWrite().Reset(tGenProtoS::ETypes::Int4);
            m_ctxWriteCmd.evalWrite().i4Value(i4NewValue);

            // Now call the common field writing method
            eRes = eWriteField(*pfldiTar);
        }
         else
        {
            IncUnknownWriteCounter();
            eRes = tCQCKit::ECommResults::FieldNotFound;
        }
    }

    catch(TError& errToCatch)
    {
        if (bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        if (m_pmdbgCallback != nullptr)
            m_pmdbgCallback->Msg(L"Exception occured during field write. See logs");

        eRes = tCQCKit::ECommResults::Exception;
    }
    return eRes;
}


tCQCKit::ECommResults TGenProtoSDriver::ePollDevice(TThread& thrThis)
{
    //
    //  If this is a one way only protocol, then just say we succeeded. But,
    //  let's at least do a faux read, so that we can sense loss of socket
    //  connection, which we won't if we only send.
    //
    if (m_eProtoType == tGenProtoS::EProtoTypes::OneWay)
    {
        try
        {
            tCIDLib::TCard1 c1Dummy;
            m_pdevcTar->bReadByte(1, c1Dummy);
        }

        catch(TError& errToCatch)
        {
            if (bShouldLog(errToCatch))
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }

            if (m_pdevcTar->bIsConnected())
                return tCQCKit::ECommResults::LostConnection;
            return tCQCKit::ECommResults::LostCommRes;
        }
        return tCQCKit::ECommResults::Success;
    }

    try
    {
        //
        //  First of all, check for any async events that might have come in. By
        //  processing them first, we avoid confusion later.
        //
        CheckAsync(thrThis);

        //
        //  Loop through our list of poll events. For any whose next poll time
        //  has been reached or passed, do them and reset their poll time for
        //  the next time that they should be polled.
        //
        const tCIDLib::TCard4 c4Count   = m_colPollEvents.c4ElemCount();
        const tCIDLib::TCard4 c4Now     = TTime::c4Millis();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            //
            //  Get the current call/rep and check its poll time. If not time yet,
            //  then go back and try the next one.
            //
            TGenProtoCallRep& clrpCur = m_colPollEvents[c4Index];
            if (clrpCur.c4NextPollTime() > c4Now)
                continue;

            if ((m_pmdbgCallback != nullptr)
            &&  tCIDLib::bAllBitsOn(m_eMsgToShow, tGenProtoS::EDbgMsgs::PollEvents))
            {
                m_pmdbgCallback->Msg
                (
                    L"Poll event '"
                    , clrpCur.qryToSend().strKey()
                    , L"' is ready to poll"
                );
            }

            //
            //  Do a full exchange on this one. If it fails, return a failure
            //  and we will go offline and try to start reconnecting.
            //
            if (bDoFullExchange(thrThis, clrpCur, kCIDLib::False))
            {
                // We got a good one, so reset the timeout counter
                m_c4Timeouts = 0;

                if (m_eProtoType == tGenProtoS::EProtoTypes::DeadIfOff)
                {
                    //
                    //  We got something, so if we are in dead mode, try to get
                    //  back live again.
                    //
                    SetDeadMode(kCIDLib::False, thrThis);
                }
            }
             else
            {
                //
                //  Bump the timeout counter. If it hits the max timeouts, then
                //  we want to say we lost the connection.
                //
                m_c4Timeouts++;
                if (m_c4Timeouts >= m_c4TimeoutMax)
                {
                    m_c4Timeouts = 0;

                    if (m_eProtoType == tGenProtoS::EProtoTypes::DeadIfOff)
                    {
                        SetDeadMode(kCIDLib::True, thrThis);
                        break;
                    }
                     else
                    {
                        if (m_pmdbgCallback != nullptr)
                            m_pmdbgCallback->Msg(L"Max timeouts reached, connection lost");

                        IncTimeoutCounter();
                        return tCQCKit::ECommResults::LostConnection;
                    }
                }
            }
            clrpCur.ResetPollTime();
        }
    }

    catch(TError& errToCatch)
    {
        if (bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        if (m_pmdbgCallback != nullptr)
            m_pmdbgCallback->Msg(L"Exception occured during poll. See logs");

        //
        //  If we are no longer connected, say we lost comm resource, else we
        //  just lost connection.
        //
        if (m_pdevcTar->bIsConnected())
            return tCQCKit::ECommResults::LostConnection;
        return tCQCKit::ECommResults::LostCommRes;
    }
    return tCQCKit::ECommResults::Success;
}


tCQCKit::ECommResults
TGenProtoSDriver::eStringFldValChanged( const   TString&        strName
                                        , const tCIDLib::TCard4 c4FldId
                                        , const TString&        strNewValue)
{
    tCQCKit::ECommResults eRes = tCQCKit::ECommResults::ValueRejected;
    try
    {
        if ((m_pmdbgCallback != nullptr)
        &&  tCIDLib::bAllBitsOn(m_eMsgToShow, tGenProtoS::EDbgMsgs::FldWrite))
        {
            m_pmdbgCallback->Msg(L"Got write for string field ", strName);
        }

        //
        //  Find this field and check that the name matches the name that we have
        //  for this field id.
        //
        TGenProtoFldInfo* pfldiTar = pfldiValidateId(strName, c4FldId);
        if (pfldiTar)
        {
            //
            //  Set up the value object in the write command context. This
            //  will make the value available to expressions.
            //
            m_ctxWriteCmd.evalWrite().Reset
            (
                tGenProtoS::ETypes::String
                , strNewValue
            );

            // Now call the common field writing method
            eRes = eWriteField(*pfldiTar);
        }
         else
        {
            IncUnknownWriteCounter();
            eRes = tCQCKit::ECommResults::FieldNotFound;
        }
    }

    catch(TError& errToCatch)
    {
        if (bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        if (m_pmdbgCallback != nullptr)
            m_pmdbgCallback->Msg(L"Exception occured during field write. See logs");

        eRes = tCQCKit::ECommResults::Exception;
    }
    return eRes;
}


tCIDLib::TVoid TGenProtoSDriver::ReleaseCommResource()
{
    if ((m_pmdbgCallback != nullptr)
    &&  tCIDLib::bAllBitsOn(m_eMsgToShow, tGenProtoS::EDbgMsgs::CommRes))
    {
        m_pmdbgCallback->Msg(L"Releasing comm res");
    }

    if (m_pdevcTar != nullptr)
        m_pdevcTar->ReleaseCommResource(eVerboseLevel());
}


tCIDLib::TVoid TGenProtoSDriver::TerminateImpl()
{
    // Clean up the device connection object
    try
    {
        delete m_pdevcTar;
        m_pdevcTar = nullptr;
    }

    catch(...)
    {
        facGenProtoS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kGPDMsgs::midStatus_DevConnCleanup
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::InitFailed
            , strMoniker()
        );
    }
}


// ---------------------------------------------------------------------------
//  TGenProtoSDriver: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TGenProtoSDriver::bDoFullExchange(          TThread&            thrCalling
                                    ,       TGenProtoCallRep&   clrpToDo
                                    , const tCIDLib::TBoolean   bConnecting)
{
    // Build the query command send send it
    TGenProtoQuery& qryConn = clrpToDo.qryToSend();
    qryConn.BuildQueryCmd(m_ctxQuery);

    // If we have a debug callback, then call it
    if ((m_pmdbgCallback != nullptr)
    &&  tCIDLib::bAllBitsOn(m_eMsgToShow, tGenProtoS::EDbgMsgs::SendBytes))
    {
        m_pmdbgCallback->Event
        (
            L"Sending="
            , m_ctxQuery.mbufSend()
            , m_ctxQuery.c4SendBytes()
        );
    }

    //
    //  If throttling, make sure we don't exceed the minimum send interval.
    //  For queries, we don't have a per-msg minimum interval, so we just have
    //  to check the overall min interval.
    //
    if (m_c4MinSendInterval)
        WaitForMinSendInterval(0);

    // And lets send this command to the device
    if (!m_pdevcTar->bWrite(m_ctxQuery.mbufSend(), m_ctxQuery.c4SendBytes()))
    {
        facGenProtoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kGPDErrs::errcRunT_QueryMsgError
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotAllWritten
            , qryConn.strKey()
        );
    }

    // Remember our new last send time if we have a minimum interval
    if (m_c4MinSendInterval)
        m_c4LastSendTime = TTime::c4Millis();;

    // If this call/reply has no expected reply, we are done
    if (!clrpToDo.bExpectsReply())
        return kCIDLib::True;

    //
    //  We may get a reply that's not the one we expect, due to the user
    //  directly modifying the device or whatever. So we calculate the max
    //  time we are to wait and continue to loop until we get what we are
    //  looking for, or time out. Any other replies we see we just process
    //  and try again.
    //
    tCIDLib::TBoolean bRet = kCIDLib::False;
    tCIDLib::TCard4 c4EndTime = TTime::c4Millis() + clrpToDo.c4MillisToWait();
    while (kCIDLib::True)
    {
        // Check the time again and give up if its expired
        if (TTime::c4Millis() > c4EndTime)
        {
            //
            //  If debugging and they want poll events, then report this.
            //  But only do it if not connecting, since we expect failures
            //  in that case.
            //
            if (m_pmdbgCallback && !bConnecting)
            {
                m_pmdbgCallback->Msg
                (
                    L"Timed out waiting for the reply to query "
                    , qryConn.strKey()
                );
             }
            break;
        }

        //
        //  Else, wait for up to the remaining time for a new reply. If we
        //  don't get one, then try again until we run out of time
        //
        if (!bGetReply(thrCalling, c4EndTime, kCIDLib::False))
            continue;

        //
        //  Match this reply up to one of our defined reply objects. We call a
        //  helper method to do this. The reply data will be in the reply
        //  context object. Send the expected reply as the prefered reply, so
        //  that it will be checked first. Since it almost always will be
        //  the reply we get, this is most efficient.
        //
        TGenProtoReply* prepFound = prepMatchReply
        (
            m_ctxReply
            , &clrpToDo.repToExpect()
        );

        //
        //  If we didn't find a match, then bump an error counter and continue
        //  if we still have time left.
        //
        if (!prepFound)
        {
            if (facGenProtoS().bLogFailures())
            {
                facGenProtoS().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kGPDErrs::errcRunT_NoMatchForReply
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::NotFound
                );
            }

            if ((m_pmdbgCallback != nullptr)
            &&  tCIDLib::bAllBitsOn(m_eMsgToShow, tGenProtoS::EDbgMsgs::MsgMatch))
            {
                m_pmdbgCallback->Msg
                (
                    L"Msg did not match expected reply for query "
                    , qryConn.strKey()
                );
            }
            continue;
        }

        //
        //  We found a matching reply. So do the validation step on it. This will
        //  confirm that the reply is of a valid form. There may not be any
        //  validation steps, in which case nothing will happen here.
        //
        if (!prepFound->bValidateReply(m_ctxReply))
        {
            if (m_pmdbgCallback != nullptr)
            {
                m_pmdbgCallback->Msg
                (
                    L"Failed to validate reply "
                    , prepFound->strKey()
                );
            }
            return kCIDLib::False;
        }

        //
        //  It validated ok, so lets invoke the store expressions. These will
        //  pull out the values from the message and store them in our fields.
        //  Pass in our changes counter, which it will bump if it modifies the
        //  value of any fields.
        //
        prepFound->StoreValues(m_ctxReply, m_colFields);

        // If it was the reply we were looking for, then return true.
        if (prepFound->strKey() == clrpToDo.repToExpect().strKey())
        {
            bRet = kCIDLib::True;
            break;
        }

        //
        //  Else, slip a little more time onto the end time to make up for
        //  the time we spent processing this one, and go again.
        //
        c4EndTime += 100;
    }
    return bRet;
}


tCIDLib::TBoolean
TGenProtoSDriver::bGetReply(        TThread&            thrThis
                            , const tCIDLib::TCard4     c4EndTime
                            , const tCIDLib::TBoolean   bFastCheck)
{
    // Use a local byte counter and only set the reply context at the end
    tCIDLib::TCard4 c4Bytes = 0;

    // Get a quick ref to the target buffer we will be filling in
    TMemBuf& mbufTarget = m_ctxReply.mbufReply();

    // Init the current bytes recieved to zero
    m_ctxReply.c4ReplyBytes(0);

    //
    //  We use the state machine to get legal messages off the com port. So
    //  we have to run the state machine through its states. This index is
    //  used to track the current state. We always start at zero and we go
    //  back to zero if any state fails to match a defined transition.
    //
    tCIDLib::TCard1     c1Cur;
    tCIDLib::TCard4     c4StateInd = 0;
    tCIDLib::TCard4     c4ActualEnd = c4EndTime;
    try
    {
        while (kCIDLib::True)
        {
            // Watch for shutdown requests
            if (thrThis.bCheckShutdownRequest())
                break;

            //
            //  If we are out of time, then we need to break out. However, if
            //  we have at least a legal byte, and we've not done so already,
            //  extend the end time a bit so that we can handle getting a msg
            //  that arrives just as we are running out of time.
            //
            //  If doing a fast check, then add more since we don't have much
            //  to start with.
            //
            if (TTime::c4Millis() > c4ActualEnd)
            {
                if (c4Bytes && (c4ActualEnd == c4EndTime))
                {
                    if (bFastCheck)
                        c4ActualEnd += 1000;
                    else
                        c4ActualEnd += 500;
                }
                 else
                {
                    // If we had some bytes, show that in the test harness
                    if ((m_pmdbgCallback != nullptr)
                    &&  c4Bytes
                    &&  tCIDLib::bAllBitsOn(m_eMsgToShow, tGenProtoS::EDbgMsgs::ReceiveBytes))
                    {
                        TString strCount;
                        strCount.AppendFormatted(c4Bytes);
                        m_pmdbgCallback->Msg
                        (
                            TString(L"Timed out with partial msg. Bytes=")
                            , strCount
                        );
                    }
                    break;
                }
            }

            if (m_pdevcTar->bReadByte(25, c1Cur))
            {
                //
                //  We have a byte, so let's let the state handler for the
                //  current state pass judgement on it. We have to store it
                //  in the state machine expression context which we pass
                //  to the evaluation.
                //
                m_ctxReply.c1InputByte(c1Cur);
                TGenProtoStateInfo* pstatiCur = m_colStateMachine[c4StateInd];
                c4StateInd = pstatiCur->c4NextState(m_ctxReply);

                // If no transition, then reset and start over
                if (c4StateInd == kCIDLib::c4MaxCard)
                {
                    // If we have a debug callback, then call it
                    if ((m_pmdbgCallback != nullptr)
                    &&  tCIDLib::bAllBitsOn(m_eMsgToShow, tGenProtoS::EDbgMsgs::StateMachine))
                    {
                        m_pmdbgCallback->StateTransition
                        (
                            c1Cur
                            , L"Failed to match, returning to state"
                            , m_colStateMachine[0]->strName()
                        );

                        // And indicate the bytes we discarded
                        mbufTarget.PutCard1(c1Cur, c4Bytes++);
                        m_pmdbgCallback->Event
                        (
                            L"Discarded Bytes="
                            , mbufTarget
                            , c4Bytes
                        );
                    }

                    c4Bytes = 0;
                    c4StateInd = 0;
                    m_ctxReply.c4ReplyBytes(0);
                }
                 else
                {
                    // If we have a debug callback, then call it
                    if ((m_pmdbgCallback != nullptr)
                    &&  tCIDLib::bAllBitsOn(m_eMsgToShow, tGenProtoS::EDbgMsgs::StateMachine))
                    {
                        m_pmdbgCallback->StateTransition
                        (
                            c1Cur
                            , L"caused transition to state"
                            , m_colStateMachine[c4StateInd]->strName()
                        );
                    }

                    //
                    //  It's a good byte, so add to the buffer, reallocate if
                    //  required.
                    //
                    if (c4Bytes == mbufTarget.c4Size())
                        mbufTarget.Reallocate(c4Bytes * 2);
                    mbufTarget.PutCard1(c1Cur, c4Bytes++);

                    //
                    //  Keep the context reply bytes value updated, because any
                    //  of the state transitions we do could reference this
                    //  value.
                    //
                    m_ctxReply.c4ReplyBytes(c4Bytes);

                    // If this state has any variable stores, then run them
                    m_colStateMachine[c4StateInd]->StoreValues(m_ctxReply);

                    // If we got to the accept state, then we've got a message
                    if (c4StateInd == m_c4AcceptStateInd)
                        break;
                }
            }
             else
            {
                //
                //  If they wanted a fast check, and we are on the 0th byte,
                //  then give up now.
                //
                if (bFastCheck && !c4Bytes)
                    break;
            }
        }
    }

    catch(TError& errToCatch)
    {
        if (bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        //
        //  Watch for port errors. If we get any, then flush the receive data
        //  from the port, and return a failure to go off line.
        //
        if (errToCatch.bCheckEvent( facCIDComm().strName()
                                    , kCommErrs::errcPort_ReadData))
        {
            m_pdevcTar->PurgeReadBuf();
            return kCIDLib::False;
        }
         else
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            throw;
        }
    }

    // Do a final update of the reply bytes
    m_ctxReply.c4ReplyBytes(c4Bytes);

    // Return true if we made it to the accept state
    return (c4StateInd == m_c4AcceptStateInd);
}


// Check for any incoming async replies and process them
tCIDLib::TVoid TGenProtoSDriver::CheckAsync(TThread& thrCalling)
{
    //
    //  Loop, checking for messages with a very short timeout, so that
    //  basically it will only catch stuff that's already here. As soon as
    //  we don't have anything available, we'll just fall out. Indicate that
    //  we want bGetReply() to do a fast check and fall out immediately if
    //  no data is present.
    //
    //  Just in case of a crazy fast continual send, only do up to 8 at
    //  a round.
    //
    tCIDLib::TCard4 c4Count = 0;
    while (bGetReply(thrCalling, TTime::c4Millis() + 50, kCIDLib::True)
    &&     (c4Count < 16))
    {
        // Bump our count of received messages
        c4Count++;

        if ((m_pmdbgCallback != nullptr)
        &&  tCIDLib::bAllBitsOn(m_eMsgToShow, tGenProtoS::EDbgMsgs::MsgMatch))
        {
            m_pmdbgCallback->Msg(L"Got an async message");
        }

        //
        //  Match this reply up to one of our defined reply objects. We call
        //  a helper method to do this. Send in a zero for the expected reply
        //  since we don't have one here.
        //
        TGenProtoReply* prepFound = prepMatchReply(m_ctxReply, 0);

        //
        //  If we didn't find a match, then bump an error counter and we
        //  we done. Else process it.
        //
        if (prepFound)
        {
            //
            //  We found a matching reply. So do the validation step on it.
            //  This will confirm that the reply is of a valid form. There
            //  may not be any validation steps, in which case nothing will
            //  happen here.
            //
            //  Even if this one fails, go back and see if more are available.
            //
            if (!prepFound->bValidateReply(m_ctxReply))
            {
                if ((m_pmdbgCallback != nullptr)
                &&  tCIDLib::bAllBitsOn(m_eMsgToShow, tGenProtoS::EDbgMsgs::MsgMatch))
                {
                    m_pmdbgCallback->Msg(L"Async msg failed to validate");
                }
                continue;
            }

            //
            //  It validated ok, so lets invoke the store expressions. These
            //  will pull out the values from the message and store them in
            //  our fields.
            //
            prepFound->StoreValues(m_ctxReply, m_colFields);
        }
         else
        {
            if ((m_pmdbgCallback != nullptr)
            &&  tCIDLib::bAllBitsOn(m_eMsgToShow, tGenProtoS::EDbgMsgs::MsgMatch))
            {
                m_pmdbgCallback->Msg(L"No match was found for an async msg");
            }

            // <TBD> Bump an error counter here
        }

        // Check for a shutdown request
        if (thrCalling.bCheckShutdownRequest())
            break;
    }
}


tCQCKit::ECommResults TGenProtoSDriver::eDoConnect(TThread& thrThis)
{
    // Flush any current input that might have been left from a previous attempt
    m_pdevcTar->PurgeReadBuf();

    //
    //  Try to get the comm resource connected. If not, then recycle the connection
    //  just to be safe.
    //
    if (!m_pdevcTar->bConnect(thrThis, eVerboseLevel()))
        return tCQCKit::ECommResults::LostCommRes;

    //
    //  We need to do the call/response defined by our protocol for the connect
    //  attempt. If it doesn't work, then return failure status, else fall through
    //  and do the poll exchanges to get the fields up to date.
    //
    //  If this is a one way connection, it won't have any of this stuff, so
    //  we skip it.
    //
    if (m_eProtoType != tGenProtoS::EProtoTypes::OneWay)
    {
        if (!bDoFullExchange(thrThis, m_clrpTryConnect, kCIDLib::True))
            return tCQCKit::ECommResults::LostConnection;

        // If they defined a post-connect pause, then do that
        if (m_c4PostConnWait)
            TThread::Sleep(m_c4PostConnWait);

        const tCIDLib::TCard4 c4Count = m_colReconnect.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            if (!bDoFullExchange(thrThis, m_colReconnect[c4Index], kCIDLib::False))
                return tCQCKit::ECommResults::LostConnection;
        }
    }

    if ((m_pmdbgCallback != nullptr)
    &&  tCIDLib::bAllBitsOn(m_eMsgToShow, tGenProtoS::EDbgMsgs::Connect))
    {
        m_pmdbgCallback->Msg(L"Connected to device");
    }
    return tCQCKit::ECommResults::Success;
}


tCQCKit::ECommResults
TGenProtoSDriver::eWriteField(TGenProtoFldInfo& fldiTar)
{
    //
    //  Ask the field object to build up the write command into the context
    //  object's send buffer.
    //
    try
    {
        fldiTar.BuildWriteCmd(m_ctxWriteCmd);
    }

    catch(TError& errToCatch)
    {
        if (m_pmdbgCallback != nullptr)
        {
            m_pmdbgCallback->Msg
            (
                TString(L"Error building write for field :")
                +  fldiTar.flddInfo().strName()
                , L"\n   Error: "
                , errToCatch.strErrText()
            );
        }
         else
        {
            if (bShouldLog(errToCatch))
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }
        }

        // Don't take any chances, say we lost comm res
        return tCQCKit::ECommResults::LostCommRes;
    }

    //
    //  If we have a debug stream, then output the data we are going to
    //  write.
    //
    if ((m_pmdbgCallback != nullptr)
    &&  tCIDLib::bAllBitsOn(m_eMsgToShow, tGenProtoS::EDbgMsgs::SendBytes))
    {
        m_pmdbgCallback->Event
        (
            L"Sending="
            , m_ctxWriteCmd.mbufSend()
            , m_ctxWriteCmd.c4SendBytes()
        );
    }

    //
    //  Make sure we've not exceeded the mininum inter-message interval
    //  for this device.
    //
    WaitForMinSendInterval(fldiTar.c4NextSendTime());

    // And lets send it to the device
    const tCIDLib::TCard4 c4ToSend = m_ctxWriteCmd.c4SendBytes();
    if (!m_pdevcTar->bWrite(m_ctxWriteCmd.mbufSend(), c4ToSend))
    {
        IncFailedWriteCounter();
        return tCQCKit::ECommResults::LostCommRes;
    }

    // Remember our new last send times
    if (m_c4MinSendInterval || fldiTar.c4MinSendInterval())
    {
        const tCIDLib::TCard4 c4New = TTime::c4Millis();
        if (m_c4MinSendInterval)
            m_c4LastSendTime = c4New;
        fldiTar.SetLastSendTime(c4New);
    }

    // WE need the thread in a couple places below
    TThread& thrMe = *TThread::pthrCaller();

    //
    //  If this field has an ack/nak reply set, then we need to wait for
    //  one of them to come back.
    //
    if (fldiTar.prepAck())
    {
        // Calculate the end time so we can break out when time is up
        tCIDLib::TCard4 c4EndTime = TTime::c4Millis()
                                    + fldiTar.c4MillisToWait();

        while (kCIDLib::True)
        {
            // If time is up, then break out
            if (TTime::c4Millis() > c4EndTime)
            {
                if (m_pmdbgCallback != nullptr)
                {
                    m_pmdbgCallback->Msg
                    (
                        L"Timed out waiting for the reply to field write "
                        , fldiTar.flddInfo().strName()
                    );
                }

                IncFailedWriteCounter();
                return tCQCKit::ECommResults::LostConnection;
            }

            // Try to get another reply. If nothing, then try again
            if (!bGetReply(thrMe, c4EndTime, kCIDLib::False))
                continue;

            //
            //  Match this reply up to one of our defined reply objects. We
            //  call a helper method to do this. The reply data will be in
            //  the reply context object. We send the ack as the expected
            //  reply, since its the most likely response. If that's not it,
            //  the reply list will be searched.
            //
            TGenProtoReply* prepFound = prepMatchReply
            (
                m_ctxReply
                , fldiTar.prepAck()
            );

            //
            //  If we didn't find a match, then bump an error counter and
            //  continue if we still have time left.
            //
            if (!prepFound)
            {
                IncUnknownMsgCounter();
                if ((m_pmdbgCallback != nullptr)
                &&  tCIDLib::bAllBitsOn(m_eMsgToShow, tGenProtoS::EDbgMsgs::MsgMatch))
                {
                    m_pmdbgCallback->Msg(L"No match was found for the reply");
                }
                continue;
            }

            //
            //  We found a matching reply. So do the validation step on it.
            //  This will confirm that the reply is of a valid form. There
            //  may not be any validation steps, in which case nothing will
            //  happen here.
            //
            //  <TBD> Eventually, we should bump an instrumentation value
            //  here.
            //
            if (!prepFound->bValidateReply(m_ctxReply))
            {
                IncBadMsgCounter();
                if ((m_pmdbgCallback != nullptr)
                &&  tCIDLib::bAllBitsOn(m_eMsgToShow, tGenProtoS::EDbgMsgs::MsgMatch))
                {
                    m_pmdbgCallback->Msg(L"The reply did not validate");
                }
                break;
            }

            //
            //  It validated ok, so lets invoke the store expressions. These
            //  will pull out the values from the message and store them in
            //  our fields. Normally, these ack/nak replies don't return data
            //  but they can.
            //
            prepFound->StoreValues(m_ctxReply, m_colFields);

            //
            //  If it was the ack or nack we were waiting for, then break out.
            //  If a nack, then bump the nack counter and get out with a value
            //  rejected. Note that the nak is optional, so only check it if
            //  it's defined.
            //
            if (fldiTar.prepNak()
            &&  (prepFound->strKey() == fldiTar.prepNak()->strKey()))
            {
                if (m_pmdbgCallback != nullptr)
                {
                    m_pmdbgCallback->Msg
                    (
                        L"Got a nak reply writing to field "
                        , fldiTar.flddInfo().strName()
                    );
                }
                IncNakCounter();
                return tCQCKit::ECommResults::DeviceResponse;
            }

            if (prepFound->strKey() == fldiTar.prepAck()->strKey())
                break;

            //
            //  Not one of the ones we were expecting so, slip a little more
            //  time onto the end time to make up for the time we spent
            //  processing this one, and go again.
            //
            c4EndTime += 100;
        }
    }

    // It seems to have worked
    return tCQCKit::ECommResults::Success;
}


TGenProtoFldInfo*
TGenProtoSDriver::pfldiValidateId(  const   TString&        strFldName
                                    , const tCIDLib::TCard4 c4FldId)
{
    //
    //  Our list only has the fields that we define, not the magic
    //  instrumentation fields, so we have to do a search, we cannot just
    //  use the id as an index.
    //
    TGenProtoFldInfo* pfldiRet = 0;
    const tCIDLib::TCard4 c4Count = m_colFields.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        pfldiRet = &m_colFields[c4Index];
        if (pfldiRet->flddInfo().c4Id() == c4FldId)
            break;
    }

    //
    //  If debugging, then make sure our name is the same as what the caller
    //  thinks this field should be named.
    //
    #if CID_DEBUG_ON
    if (pfldiRet)
    {
        if (pfldiRet->flddInfo().strName() != strFldName)
        {
            facGenProtoS().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kGPDErrs::errcRunT_FldIdNameMatch
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::NotAllRead
                , TCardinal(c4FldId)
                , pfldiRet->flddInfo().strName()
                , strFldName
            );
        }
    }
    #endif
    return pfldiRet;
}


TGenProtoFldInfo* TGenProtoSDriver::pfldiFindByName(const TString& strToUse)
{
    // Search the field vector
    const tCIDLib::TCard4 c4Count = m_colFields.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TGenProtoFldInfo* pfldiCur = &m_colFields[c4Index];
        if (pfldiCur->flddInfo().strName() == strToUse)
            return pfldiCur;
    }
    return 0;
}

const TGenProtoFldInfo*
TGenProtoSDriver::pfldiFindByName(const TString& strToUse) const
{
    // Search the field vector
    const tCIDLib::TCard4 c4Count = m_colFields.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TGenProtoFldInfo* pfldiCur = &m_colFields[c4Index];
        if (pfldiCur->flddInfo().strName() == strToUse)
            return pfldiCur;
    }
    return 0;
}


TGenProtoMap* TGenProtoSDriver::pmapFindByName(const TString& strNameToFind)
{
    return m_colMaps.pobjFindByKey(strNameToFind);
}


TGenProtoExprNode*
TGenProtoSDriver::pnodeOptimizeNode(TGenProtoExprNode* const pnodeToOpt)
{
    // Assume no optimization, in which case we'll return the input node
    TGenProtoExprNode* pnodeRet = pnodeToOpt;

    //
    //  If the node is const, and is not already a const expression node,
    //  lets evaluate it and convert to a const expression node, deleting
    //  the original tree.
    //
    //  This is a recursive op, but we have to start it here because we
    //  have to act on the top level node of the expression tree. If this
    //  node is not const, then we just invoke a recursive optimize call
    //  on it and the operation will go through the rest of the tree, working
    //  downward looking for const expressions to collapse.
    //
    //  Note that we have to use a context in order to evaluate the value
    //  of the node, so that we can create a const node with that value.
    //  However, we don't know the real context of the call, and it doesn't
    //  matter anyway since this operation is done before the runtime
    //  environment is really working.
    //
    //  However, no node that requires any runtime (context) info should
    //  report itself as const, so we should never end up trying to evaluate
    //  any node that would fail due to the context info not being valid.
    //  So we just arbitrarily use the write command context.
    //
    if (pnodeToOpt->bIsConst())
    {
        if (pnodeToOpt->clsIsA() != TGenProtoConstNode::clsThis())
        {
            //
            //  Evaluate the node value and create a new node to replace it.
            //  Then delete the original.
            //
            pnodeToOpt->Evaluate(m_ctxWriteCmd);
            pnodeRet = new TGenProtoConstNode(pnodeToOpt->evalCur());
            delete pnodeToOpt;
        }
    }
     else
    {
        //
        //  We can't do anything at this level, and will just return this
        //  node again. However, something underneath it might be optimizable,
        //  so recurse on it.
        //
        pnodeToOpt->Optimize(m_ctxWriteCmd);
    }
    return pnodeRet;
}


TGenProtoReply* TGenProtoSDriver::prepFindByName(const TString& strNameToFind)
{
    return m_colReplies.pobjFindByKey(strNameToFind);
}


TGenProtoReply*
TGenProtoSDriver::prepMatchReply(TGenProtoCtx&          ctxToUse
                                , TGenProtoReply* const prepExpected)
{
    // If we have a debug callback, then call it
    if ((m_pmdbgCallback != nullptr)
    &&  tCIDLib::bAllBitsOn(m_eMsgToShow, tGenProtoS::EDbgMsgs::ReceiveBytes))
    {
        m_pmdbgCallback->Event
        (
            L"Received="
            , ctxToUse.mbufReply()
            , ctxToUse.c4ReplyBytes()
        );
    }

    //
    //  If there is an expected reply, then lets check it first. If it matches,
    //  we take it, and are done.
    //
    const tCIDLib::TCard4 c4Count = m_colMatches.c4ElemCount();
    if (prepExpected)
    {
        //
        //  Find any msg match blocks that target the indicated reply, and
        //  see if they match.
        //
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            TGenProtoMsgMatch* pmsgmCur = m_colMatches[c4Index];

            try
            {
                // If it targets this reply, see if it matches the message
                if (pmsgmCur->prepMatch() == prepExpected)
                {
                    if (pmsgmCur->bMatches(ctxToUse))
                        return prepExpected;
                }
            }

            catch(TError& errToCatch)
            {
                if (m_pmdbgCallback != nullptr)
                {
                    m_pmdbgCallback->Msg
                    (
                        TString(L"Error in msg match:")
                        +  pmsgmCur->prepMatch()->strKey()
                        , L"\n   Error: "
                        , errToCatch.strErrText()
                    );
                }
                 else
                {
                    if (bShouldLog(errToCatch))
                    {
                        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                        TModule::LogEventObj(errToCatch);
                    }
                }
            }
        }
    }
     else
    {
        if ((m_pmdbgCallback != nullptr)
        &&  tCIDLib::bAllBitsOn(m_eMsgToShow, tGenProtoS::EDbgMsgs::MsgMatch))
        {
            m_pmdbgCallback->Msg(L"There is no expected reply, matching other replies");
        }
    }

    TGenProtoReply* prepRet = 0;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TGenProtoMsgMatch* pmsgmCur = m_colMatches[c4Index];

        // If its the expected, then skip it, since we checked it above
        if (pmsgmCur->prepMatch() == prepExpected)
            continue;

        // Else check it
        try
        {
            if (pmsgmCur->bMatches(ctxToUse))
            {
                prepRet = pmsgmCur->prepMatch();
                break;
            }
        }

        catch(TError& errToCatch)
        {
            if (m_pmdbgCallback != nullptr)
            {
                m_pmdbgCallback->Msg
                (
                    TString(L"Error in msg match:")
                    +  pmsgmCur->prepMatch()->strKey()
                    , L"\n   Error: "
                    , errToCatch.strErrText()
                );
            }
             else
            {
                if (bShouldLog(errToCatch))
                {
                    errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                    TModule::LogEventObj(errToCatch);
                }
            }
        }
    }

    if ((m_pmdbgCallback != nullptr)
    &&  tCIDLib::bAllBitsOn(m_eMsgToShow, tGenProtoS::EDbgMsgs::MsgMatch))
    {
        if (prepRet != nullptr)
            m_pmdbgCallback->Msg(L"Matched: ", prepRet->strKey());
        else
            m_pmdbgCallback->Msg(L"No reply matched the message received");
    }
    return prepRet;
}


TGenProtoVarInfo*
TGenProtoSDriver::pvariFindByName(const TString& strNameToFind)
{
    return m_colVariables.pobjFindByKey(strNameToFind);
}


//
//  If this is a 'dead if off' type device, if any exchange fails, this is
//  called. If we aren't already in dead mode, we set it. This involved
//  putting all the fields into error, except one that is marked as the
//  'revive' field. That field will be a boolean write-only field and is
//  used to force the device back on.
//
//  If we are coming out of dead mode, then we go back through the try
//  connect and reconnect cycles, in order to get the fields back up to
//  date.
//
tCIDLib::TVoid
TGenProtoSDriver::SetDeadMode(const tCIDLib::TBoolean bToSet, TThread& thrThis)
{
    // If setting a node other than what we currently are
    if (bToSet != m_bDeadMode)
    {
        const tCIDLib::TCard4 c4Count = m_colFields.c4ElemCount();

        // Find the revive thread for later use
        TGenProtoFldInfo* pfldiRevive = 0;
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            TGenProtoFldInfo* pfldiCur = &m_colFields[c4Index];
            if (pfldiCur->bIsReviveField())
            {
                pfldiRevive = pfldiCur;
                break;
            }
        }

        // This is really bad. It should have been enforced at compile time
        #if CID_DEBUG_ON
        if (!pfldiRevive)
        {
            return;
        }
        #endif

        const TCQCFldDef& flddRevive = pfldiRevive->flddInfo();

        // If exiting dead mode, we have to do the connection cycle
        tCIDLib::TBoolean bActual = bToSet;
        if (eDoConnect(thrThis) != tCQCKit::ECommResults::Success)
        {
            //
            //  It failed, so force the flag back to false so that
            //  we get all the fields back in error again, since
            //  some might have gotten set.
            //
            bActual = kCIDLib::True;
        }

        // If entering dead mode, then set the revive field to false
        if (bActual)
            bStoreBoolFld(flddRevive.c4Id(), kCIDLib::False, kCIDLib::True);

        // If in the test harness, say what we are doing
        if (m_pmdbgCallback != nullptr)
        {
            if (bActual)
                m_pmdbgCallback->Msg(L"Entering Dead Mode");
            else
                m_pmdbgCallback->Msg(L"Exiting Dead Mode");
        }

        //
        //  If entering dead mode, then we have to put all of the fields
        //  (except the revive field into error.) If exiting, then we
        //  got the readables out of error above when we stored new values,
        //  so we just do the write-only ones.
        //
        TGenProtoFldInfo* pfldiCur = 0;
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            pfldiCur = &m_colFields[c4Index];
            const TCQCFldDef& flddCur = pfldiCur->flddInfo();

            if (pfldiCur->bIsReviveField())
            {
                // Just in case, make sure it hasn't gotten into error state
                SetFieldErr(flddCur.c4Id(), kCIDLib::False);
            }
             else
            {
                if (bActual)
                    SetFieldErr(flddCur.c4Id(), kCIDLib::True);
                else if (flddCur.eAccess() == tCQCKit::EFldAccess::Write)
                    SetFieldErr(flddCur.c4Id(), kCIDLib::False);
            }
        }

        // If exiting dead mode, then set the revive field
        if (!bActual)
            bStoreBoolFld(flddRevive.c4Id(), kCIDLib::True, kCIDLib::True);

        // Set the new state
        m_bDeadMode = bToSet;
    }
}


tCIDLib::TVoid
TGenProtoSDriver::WaitForMinSendInterval(const tCIDLib::TCard4 c4PerMsg)
{
    // If neither the overall or per-msg minimum interval is set, do nothing
    if (!c4PerMsg && !m_c4MinSendInterval)
        return;

    //
    //  Decide whether the per message next target time or the overall next
    //  target time is larger and use that one.
    //
    const tCIDLib::TCard4 c4OverallTarget = m_c4LastSendTime
                                            + m_c4MinSendInterval;

    const tCIDLib::TCard4 c4Target = (c4PerMsg > c4OverallTarget)
                                     ? c4PerMsg : c4OverallTarget;

    const tCIDLib::TCard4 c4Now = TTime::c4Millis();

    // If now is before the target, then wait until the target
    if (c4Now < c4Target)
        TThread::Sleep(c4Target - c4Now);
}


