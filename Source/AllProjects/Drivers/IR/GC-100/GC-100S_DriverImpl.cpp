//
// FILE NAME: GC100S_DriverImpl.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/04/2002
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
//  This file implements the bulk of the driver implementation.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "GC-100S.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TGC100Driver,TBaseIRSrvDriver)



// ---------------------------------------------------------------------------
//   CLASS: TGC100Driver
//  PREFIX: sdrv
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TGC100Driver: Constructors and Destructor
// ---------------------------------------------------------------------------
TGC100Driver::TGC100Driver(const TCQCDriverObjCfg& cqcdcToLoad) :

    TBaseIRSrvDriver(cqcdcToLoad)
    , m_c2NextId(1)
    , m_c4FailCnt(0)
    , m_c4FldIdFirmwareVer(kCIDLib::c4MaxCard)
    , m_c4FldIdInvoke(kCIDLib::c4MaxCard)
    , m_c4FldIdTrainingMode(kCIDLib::c4MaxCard)
    , m_c4LastPollInd(0)
    , m_c4PollCount(0)
    , m_sockConn()
    , m_strPingQ(L"getversion,1\r")
    , m_strRec(64UL)
    , m_strSend(64UL)
{
}

TGC100Driver::~TGC100Driver()
{
    // Make sure the socket is shutdown
    try
    {
        if (m_sockConn.bIsConnected())
            m_sockConn.c4Shutdown();
    }

    catch(...) {}

    try
    {
        m_sockConn.Close();
    }

    catch(...) {}
}


// ---------------------------------------------------------------------------
//  TGC100Driver: Protected, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TGC100Driver::bCvtManualBlastData(  const   TString&            strText
                                    ,       tCIDLib::TCard4&    c4DataBytes
                                    ,       THeapBuf&           mbufToFill
                                    ,       TString&            strError)
{
    // If it's a UIRT code, convert it. Else it has to be a GC-100 code.
    TString strActual;
    if (facCQCIR().bIsValidUIRTBlastData(strText))
    {
        // Convert to GC-100 format
        TTextStringOutStream strmCvt(&strActual);
        facCQCIR().ProntoToGC100(strText, strmCvt);
    }
     else
    {
        if (!facCQCIR().bIsValidGC100BlastData(strText))
        {
            strError.LoadFromMsg
            (
                kIRErrs::errcFmt_BadIRDataFormat, facCQCIR(), TString(L"GC-100")
            );
            return kCIDLib::False;
        }
        strActual = strText;
    }

    //
    //  The flattened format is just the single byte version of the text,
    //  so we just do a cheap transcode fo the characters to short chars via
    //  truncation.
    //
    const tCIDLib::TCard4 c4Len = strActual.c4Length();
    CIDAssert(c4Len != 0, L"The incoming IR data length was zero");
    for (c4DataBytes = 0; c4DataBytes< c4Len; c4DataBytes++)
        mbufToFill.PutCard1(tCIDLib::TCard1(strActual[c4DataBytes]), c4DataBytes);

    return kCIDLib::True;
}


tCIDLib::TBoolean TGC100Driver::bGetCommResource(TThread& thrThis)
{
    try
    {
        // Just in case, close if already opened, then open
        if (m_sockConn.bIsOpen())
            m_sockConn.Close();
        m_sockConn.Open
        (
            tCIDSock::ESockProtos::TCP, m_conncfgSock.ipepConn().eAddrType()
        );
    }

    catch(TError& errToCatch)
    {
        LogError(errToCatch, tCQCKit::EVerboseLvls::Medium, CID_FILE, CID_LINE);

        // Not much we can do, so eat it, but do a close just in case
        m_sockConn.Close();
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


tCIDLib::TBoolean TGC100Driver::bResetConnection()
{
    // No-op for us
    return kCIDLib::True;
}


tCIDLib::TCard4 TGC100Driver::c4ZoneCount() const
{
    return m_colZoneList.c4ElemCount();
}


tCIDLib::TCard4 TGC100Driver::c4InvokeFldId() const
{
    return m_c4FldIdInvoke;
}


tCQCKit::ECommResults
TGC100Driver::eBoolFldValChanged(const  TString&            strName
                                , const tCIDLib::TCard4     c4FldId
                                , const tCIDLib::TBoolean   bNewValue)
{
    // See if it's one of our relay fields
    const tCIDLib::TCard4 c4Count = m_colFldList.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TGC100Addr& gcaCur = m_colFldList[c4Index];

        if (gcaCur.c4FldId() == c4FldId)
        {
            // It's our guy so set it
            m_strSend = L"setstate,";
            m_strSend.AppendFormatted(gcaCur.c4ModuleNum());
            m_strSend.Append(kCIDLib::chColon);
            m_strSend.AppendFormatted(gcaCur.c4ConnNum());
            m_strSend.Append(kCIDLib::chComma);

            if (bNewValue)
                m_strSend.Append(kCIDLib::chDigit1);
            else
                m_strSend.Append(kCIDLib::chDigit0);
            m_strSend.Append(kCIDLib::chCR);

            //
            //  Send it. Eat the reply since we'll pick up the state in the
            //  polling cycle anyway. But this allows us to catch an error
            //  return if we get one.
            //
            SendMessage(m_strSend);
            GetLineMS(m_strRec, 2000);
            return tCQCKit::ECommResults::Success;
        }
    }

    // Don't know what it is
    IncUnknownWriteCounter();
    return tCQCKit::ECommResults::FieldNotFound;
}


tCQCKit::ECommResults TGC100Driver::eConnectToDevice(TThread& thrThis)
{
    try
    {
        // Try to connect to the remote end point
        m_sockConn.Connect(m_conncfgSock.ipepConn());
    }

    catch(TError& errToCatch)
    {
        LogError(errToCatch, tCQCKit::EVerboseLvls::Medium, CID_FILE, CID_LINE);
        return tCQCKit::ECommResults::LostConnection;
    }

    // We are connected so try to get the configuration
    try
    {
        // It worked so try to load up the configuration from the device
        QueryConfig();

        // Reset any polling stuff now that we've connected
        m_c4PollCount = 0;
        m_c4FailCnt = 0;
    }

    catch(TError& errToCatch)
    {
        if (eVerboseLevel() > tCQCKit::EVerboseLvls::Medium)
        {
            LogError(errToCatch, tCQCKit::EVerboseLvls::Medium, CID_FILE, CID_LINE);
            facGC100().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kGCSErrs::errcComm_GetConfig
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );
        }

        // Assume the worst case scenario and make us cycle the connection
        return tCQCKit::ECommResults::LostCommRes;
    }

    //
    //  We cannot register our fields until we've talked to the device and
    //  figured out what features it has, so we do it here instead of in
    //  the init method.
    //
    RegisterFields();

    // And do an initial poll to get our fields loaded
    if (eVerboseLevel() > tCQCKit::EVerboseLvls::Medium)
    {
        facGC100().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kGCSMsgs::midStatus_GetInitVals
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
    }
    return ePollDevice(thrThis);
}


tCQCKit::EDrvInitRes TGC100Driver::eInitializeImpl()
{
    // Call our parent IR base drivers
    TParent::eInitializeImpl();

    //
    //  Make sure that we were configured for a socket connection. Otherwise,
    //  its a bad configuration.
    //
    const TCQCDriverObjCfg& cqcdcOurs = cqcdcThis();
    if (cqcdcOurs.conncfgReal().clsIsA() != TCQCIPConnCfg::clsThis())
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
            , TCQCIPConnCfg::clsThis()
        );
        return tCQCKit::EDrvInitRes::Failed;
    }

    // It's correct, so store the socket connection for later
    m_conncfgSock = static_cast<const TCQCIPConnCfg&>(cqcdcOurs.conncfgReal());

    //
    //  Set the poll time for 750ms. This is actually kind of slow for an
    //  IP based device, but the GC-100 seems to get confused a bit sometimes,
    //  so we don't want to bang it too fast.
    //
    SetPollTimes(750, 5000);

    //
    //  In our case we want to go to 'wait for config' mode, not wait for
    //  comm res, since we need to get configuration before we can go online.
    //
    return tCQCKit::EDrvInitRes::WaitConfig;
}


tCQCKit::ECommResults TGC100Driver::ePollDevice(TThread& thrThis)
{
    try
    {
        //
        //  Run through the pollable fields and update them. Watch for a
        //  device change command while doing this. If we get one, we will
        //  have to reconfigure.
        //
        //  NOTE:   The -8 version has no contact closures. So if they don't
        //          configure any IR zones as sensors, we have nothing to
        //          poll. In that case, we have to just ping the device
        //          periodically to make sure it's still alive.
        //
        const tCIDLib::TCard4 c4Count = m_colFldList.c4ElemCount();
        if (c4Count)
        {
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            {
                const TGC100Addr& gcaCur = m_colFldList[c4Index];

                // If it's a pollable module, poll it
                if ((gcaCur.eModType() == tGC100::EModTypes::IRSensor)
                ||  (gcaCur.eModType() == tGC100::EModTypes::Relay))
                {
                    const EPollRes eRes = ePollInput
                    (
                        gcaCur.c4ModuleNum()
                        , gcaCur.c4ConnNum()
                        , m_strRec
                        , m_strSend
                    );

                    //
                    //  Either set the state or set the error state for the
                    //  associated field. A new value written from the driver
                    //  will clear any previous error flag.
                    //
                    if ((eRes == EPollRes::Off) || (eRes == EPollRes::On))
                    {
                        bStoreBoolFld
                        (
                            gcaCur.c4FldId()
                            , (eRes == EPollRes::On)
                            , kCIDLib::True
                        );
                    }
                     else
                    {
                        SetFieldErr(gcaCur.c4FldId(), kCIDLib::True);
                    }
                }
            }
        }
         else
        {
            //
            //  Nothing to poll, so every 10 times through, ping the device
            //  so that we'll catch it going offline.
            //
            m_c4PollCount++;
            if (m_c4PollCount >= 10)
            {
                m_c4PollCount = 0;


                //
                //  Even if we get an error message back, that's proof that
                //  it's there. It's conceivable that this ping won't be
                //  a valid one because some variation of the device has
                //  no serial port (and our ping just queries the version
                //  of the first port connector.)
                //
                try
                {
                    SendMessage(m_strPingQ);
                    GetLineMS(m_strRec, 1500);
                }

                catch(TError& errToCatch)
                {
                    //
                    //  If it's from our facility and the error class is
                    //  protocol, then it's a GC error return. Otherwise,
                    //  just rethrow to the handler below.
                    //
                    if ((errToCatch.strFacName() != facGC100().strName())
                    ||  (errToCatch.eClass() != tCIDLib::EErrClasses::Protocol))
                    {
                        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                        throw;
                    }
                }
            }

        }

        // We got a good reply so reset the failure counter
        m_c4FailCnt = 0;
    }

    catch(TError& errToCatch)
    {
        //
        //  Watch for a not understood error. If we get one, then bump the
        //  failed count.
        //
        const tCIDLib::TBoolean bUnknownCmd
        (
            errToCatch.bCheckEvent
            (
                facGC100().strName(), kGCSErrs::errcGC_BadCommand
            )
        );

        if (bUnknownCmd)
            m_c4FailCnt++;

        //
        //  If not the unknown command error, or it was and we've now hit
        //  three of them in a row, then give up. Else, lie for now and
        //  say it worked and we'll try again next time.
        //
        if (!bUnknownCmd || (bUnknownCmd && (m_c4FailCnt > 2)))
        {
            LogError(errToCatch, tCQCKit::EVerboseLvls::Medium, CID_FILE, CID_LINE);
            return tCQCKit::ECommResults::LostCommRes;
        }
    }
    return tCQCKit::ECommResults::Success;
}


// We have to format the data for the indicated command to text
tCIDLib::TVoid
TGC100Driver::FormatBlastData(  const   TIRBlasterCmd&  irbcFmt
                                ,       TString&        strToFill)
{
    //
    //  The data is just the text as ASCII, so we can do a cheap transcode
    //  to the string by just expanding each byte.
    //
    const tCIDLib::TCard4 c4Count = irbcFmt.c4DataLen();
    const tCIDLib::TCard1* pc1Src = irbcFmt.mbufData().pc1Data();
    strToFill.Clear();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        strToFill.Append(tCIDLib::TCh(*pc1Src++));
}


tCIDLib::TVoid
TGC100Driver::InvokeBlastCmd(const  TString&        strDevice
                            , const TString&        strCmd
                            , const tCIDLib::TCard4 c4ZoneNum)
{
    // If we aren't online, then throw
    CheckOnline(CID_FILE, CID_LINE);

    // Make sure the zone number is legal for our zone count
    if (c4ZoneNum >= m_colZoneList.c4ElemCount())
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

    // Look up the command
    tCIDLib::TCard4 c4Repeat;
    const TIRBlasterCmd& irbcInvoke = irbcFromName(strDevice, strCmd, c4Repeat);

    // And call the helper method that does the actual work
    SendIRCmd
    (
        irbcInvoke.mbufData().pc1Data()
        , irbcInvoke.c4DataLen()
        , c4ZoneNum
        , c4Repeat
    );
}


tCIDLib::TVoid TGC100Driver::ReleaseCommResource()
{
    // First do a clean shutdown
    try
    {
        m_sockConn.c4Shutdown();
    }

    catch(TError& errToCatch)
    {
        LogError(errToCatch, tCQCKit::EVerboseLvls::Medium, CID_FILE, CID_LINE);
    }

    // And close it
    try
    {
        m_sockConn.Close();
    }

    catch(TError& errToCatch)
    {
        LogError(errToCatch, tCQCKit::EVerboseLvls::Medium, CID_FILE, CID_LINE);
    }
}


tCIDLib::TVoid
TGC100Driver::SendBlasterData(  const   tCIDLib::TCard4 c4DataBytes
                                , const TMemBuf&        mbufToSend
                                , const tCIDLib::TCard4 c4ZoneNum
                                , const tCIDLib::TCard4 c4RepeatCount)
{
    // If we aren't online, then throw, else lock and do it
    CheckOnline(CID_FILE, CID_LINE);

    // Make sure the zone number is legal for our zone count
    if (c4ZoneNum >= m_colZoneList.c4ElemCount())
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

    //
    //  Call a helper that does the actual send, since we have to also
    //  support sending via command name, and we don't want to replicate
    //  this code.
    //
    SendIRCmd(mbufToSend.pc1Data(), c4DataBytes, c4ZoneNum, c4RepeatCount);
}


tCIDLib::TVoid TGC100Driver::TerminateImpl()
{

    // Call our parent last
    TParent::TerminateImpl();
}


// ---------------------------------------------------------------------------
//  TGC100Driver: Private, non-virtual methods
// ---------------------------------------------------------------------------
TGC100Driver::ESensorCheck
TGC100Driver::eCheckSensor( const   tCIDLib::TCard4 c4ModNum
                            , const tCIDLib::TCard4 c4ConnNum)
{
    //
    //  We just do a state query on the indicated IR connector. If it returns
    //  an error, we assume it's not a sensor, but a blaster.
    //
    try
    {
        TString strMsg(L"getstate,");
        strMsg.AppendFormatted(c4ModNum);
        strMsg.Append(kCIDLib::chColon);
        strMsg.AppendFormatted(c4ConnNum);
        strMsg.Append(L"\r");
        SendMessage(strMsg);

        GetLineMS(strMsg, 1500);
        if (strMsg.bStartsWith(L"state,"))
        {
            if (eVerboseLevel() >= tCQCKit::EVerboseLvls::High)
            {
                if (strMsg.c4Length() > 8)
                    strMsg.CapAt(8);
                facGC100().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kGCSErrs::errcProto_ExpectedReply
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::AppStatus
                    , TString(L"state,")
                    , strMsg
                );
            }
            return ESensorCheck::Sensor;
        }
    }

    catch(TError& errToCatch)
    {
        LogError(errToCatch, tCQCKit::EVerboseLvls::High, CID_FILE, CID_LINE);

        // If we get the 'not an input' then assume a blaster
        if (errToCatch.bCheckEvent( facGC100().strName()
                                    , kGCSErrs::errcGC_NotAnInput))
        {
            return ESensorCheck::Blaster;
        }

        // Something else, just say unsure
        return ESensorCheck::Unknown;
    }
    return ESensorCheck::Blaster;
}


TGC100Driver::EPollRes
TGC100Driver::ePollInput(const  tCIDLib::TCard4 c4ModNum
                        , const tCIDLib::TCard4 c4ConnNum
                        ,       TString&        strSend
                        ,       TString&        strRec)
{
    TGC100Driver::EPollRes eRet(EPollRes::Error);
    try
    {
        strSend = (L"getstate,");
        strSend.AppendFormatted(c4ModNum);
        strSend.Append(kCIDLib::chColon);
        strSend.AppendFormatted(c4ConnNum);
        strSend.Append(L"\r");
        SendMessage(strSend);

        GetLineMS(strRec, 2500);

        //
        //  If we cut th 'get' from the sent message, and replace the
        //  new line with a comma, then the reply we got should start
        //  with that as a prefix.
        //
        strSend.Cut(0, 3);
        strSend.SetLast(kCIDLib::chComma);
        if (!strRec.bStartsWith(strSend))
        {
            if (eVerboseLevel() >= tCQCKit::EVerboseLvls::High)
            {
                if (strRec.c4Length() > 8)
                    strRec.CapAt(8);
                facGC100().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kGCSErrs::errcProto_ExpectedReply
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::AppStatus
                    , strSend
                    , strRec
                );
            }
            return EPollRes::Error;
        }

        //
        //  Cut off the prefix part and remove the CR, which should leave
        //  the state value (a 0 or 1)
        //
        strRec.Cut(0, strSend.c4Length());

        if (strRec == L"0")
            eRet = EPollRes::Off;
        else if (strRec == L"1")
            eRet = EPollRes::On;
        else
            eRet = EPollRes::Error;
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
    return eRet;
}


tCIDLib::TVoid
TGC100Driver::GetLineMS(TString& strToFill, const tCIDLib::TCard4 c4WaitMS)
{
    // Convert to ane end time and call the other version
    const tCIDLib::TEncodedTime enctEnd(TTime::enctNowPlusMSs(c4WaitMS));
    GetLine(strToFill, enctEnd);
}


tCIDLib::TVoid
TGC100Driver::GetLine(TString& strToFill, const tCIDLib::TEncodedTime enctEnd)
{
    try
    {
        //
        //  We check for some async messages, so we set up some local static
        //  strings to use for the comparison.
        //
        static const TString strDevChange(L"devicechange");
        static const TString strStateChange(L"statechange");
        static const TString strSerOverflow(L"serialoverflow");
        static const TString strError(L"unknowncommand ");

        //
        //  It sends us some async notifications, so we have to be prepared
        //  to either ignore or process them.
        //
        tCIDLib::TBoolean bDone = kCIDLib::False;
        while (!bDone)
        {
            // Get a CR terminated line
            strToFill.Clear();
            if (!bGetASCIITermMsg2(m_sockConn, enctEnd, 0xD, 0, strToFill))
            {
                facGC100().ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kGCSErrs::errcComm_Timeout
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Timeout
                );
            }

            //
            //  If it's an error, then translate it to one of our errors and
            //  throw it.
            //
            if (strToFill.bStartsWith(strError))
            {
                // Maps the GC error codes to the error of ours we throw
                static tCIDLib::TCard4 aerrcMap[] =
                {
                    kGCSErrs::errcNoError
                    , kGCSErrs::errcGC_TimeOut
                    , kGCSErrs::errcGC_InvalidModAddr
                    , kGCSErrs::errcGC_InvalidModAddr2
                    , kGCSErrs::errcGC_InvalidConnAddr
                    , kGCSErrs::errcGC_NotABlasterZone1
                    , kGCSErrs::errcGC_NotABlasterZone2
                    , kGCSErrs::errcGC_NotABlasterZone3
                    , kGCSErrs::errcGC_BadIROffset
                    , kGCSErrs::errcGC_TooManyIRTrans
                    , kGCSErrs::errcGC_BadOddTransCount
                    , kGCSErrs::errcGC_NotARelay
                    , kGCSErrs::errcGC_MissingCR
                    , kGCSErrs::errcGC_NotAnInput
                    , kGCSErrs::errcGC_BadCommand
                    , kGCSErrs::errcGC_TooManyIRTrans2
                    , kGCSErrs::errcGC_OddIRTransCnt
                    , 0
                    , 0
                    , 0
                    , 0
                    , kGCSErrs::errcGC_NotIRModule
                    , 0
                    , kGCSErrs::errcGC_NotSupported
                };
                static const tCIDLib::TCard4  c4ErrCount = tCIDLib::c4ArrayElems(aerrcMap);

                //
                //  Get the error code out and convert it. Don't trust that
                //  it's a valid number. If not, throw that error.
                //
                strToFill.Cut(0, strError.c4Length());
                strToFill.StripWhitespace();
                tCIDLib::TCard4 c4ErrCode;
                if (!strToFill.bToCard4(c4ErrCode, tCIDLib::ERadices::Dec))
                {
                    facGC100().ThrowErr
                    (
                        CID_FILE
                        , CID_LINE
                        , kGCSErrs::errcProto_BadErrMsgReply
                        , tCIDLib::ESeverities::Failed
                        , tCIDLib::EErrClasses::Format
                    );
                }

                //
                //  If beyond our known error count or one of the ones with a
                //  zero, then just throw an 'unknown error x' error. Else
                //  throw the actual error.
                //
                //  NOTE that these must use the Protocol error class, since
                //  catches above depend on that to know if it's an error back
                //  from the device as apposed to something else.
                //
                if ((c4ErrCode >= c4ErrCount) || !aerrcMap[c4ErrCode])
                {
                    facGC100().ThrowErr
                    (
                        CID_FILE
                        , CID_LINE
                        , kGCSErrs::errcGC_UnknownErr
                        , tCIDLib::ESeverities::Failed
                        , tCIDLib::EErrClasses::Protocol
                        , TCardinal(c4ErrCode)
                    );

                }

                facGC100().ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , aerrcMap[c4ErrCode]
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Protocol
                );
            }

            //
            //  If it's one of the async notifications, then we need to ignore
            //  it, else we'll assume it's the response. These are all things
            //  that we handle elsewhere or don't expect to see anyway, but
            //  check for just in case.
            //
            if ((strToFill != strDevChange)
            &&  !strToFill.bStartsWith(strStateChange)
            &&  !strToFill.bStartsWith(strSerOverflow))
            {
                bDone = kCIDLib::True;
            }
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}


tCIDLib::TVoid
TGC100Driver::GetReply( const   TString&        strTermStr
                        ,       TString&        strToFill
                        , const tCIDLib::TCard4 c4WaitMS)
{
    try
    {
        // Clear out the member string that we load reples into
        strToFill.Clear();

        //
        //  Get lines until we get the termination string, or we run out of
        //  time.
        //
        TString strLine;
        tCIDLib::TEncodedTime enctCur = TTime::enctNow();
        const tCIDLib::TEncodedTime enctEnd
        (
            enctCur + (c4WaitMS * kCIDLib::enctOneMilliSec)
        );

        while (enctCur < enctEnd)
        {
            GetLine(strLine, enctEnd);

            // Append this one
            strToFill.Append(strLine);
            strToFill.Append(L"\n");

            // Break out if the term string
            if (strLine == strTermStr)
                break;

            enctCur = TTime::enctNow();
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}


tCIDLib::TVoid TGC100Driver::QueryConfig()
{
    // Send a query devices command and get the reply back
    SendMessage(L"getdevices\r");

    TString strConfig(64UL);
    GetReply(TString(L"endlistdevices"), strConfig, 2500);

    // Set up a stream over the reply string
    TTextStringInStream strmIn(&strConfig);

    // Clear out the field and zone lists
    m_colFldList.RemoveAll();
    m_colZoneList.RemoveAll();

    //
    //  Read through the lines, looking for the ones that we are interested
    //  in. For each device line we find, we have to figure out what type
    //  of module it is, and generate the appropriate address map items for
    //  that connector, which later will be used to generate some fields
    //  that we register, and also later to map incoming connector addresses
    //  back to fields.
    //
    tCIDLib::TCard4 c4SensorId = 1;
    tCIDLib::TCard4 c4BlasterId = 1;
    TGC100Addr      gcaCur;
    TString         strLine;
    TString         strMod;
    TString         strCount;
    TString         strType;
    TString         strName;
    TStringTokenizer stokLine(&strLine, L" ,");
    while (!strmIn.bEndOfStream())
    {
        // Get the next line and reset the tokenizer
        strmIn.c4GetLine(strLine);
        stokLine.Reset();

        // Break out on the end line
        if (strLine.bStartsWith(L"endlistdevices"))
            break;

        // Get the first token, which should be device, else ignore this line
        if (!stokLine.bGetNextToken(strType) || !strType.bStartsWith(L"device"))
            continue;

        // Ok, there should be 3 more tokens
        if (!stokLine.bGetNextToken(strMod)
        ||  !stokLine.bGetNextToken(strCount)
        ||  !stokLine.bGetNextToken(strType))
        {
            //
            //  Log a message about this, because it's badly formed, but
            //  try to keep going by moving to the next line.
            //
            facGC100().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kGCSMsgs::midStatus_BadDevLine
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );
            continue;
        }

        // The first two should convert to numbers
        tCIDLib::TCard4 c4Mod;
        tCIDLib::TCard4 c4Count;

        try
        {
            c4Mod = strMod.c4Val(tCIDLib::ERadices::Dec);
            c4Count = strCount.c4Val(tCIDLib::ERadices::Dec);
        }

        catch(...)
        {
            if (eVerboseLevel() > tCQCKit::EVerboseLvls::Medium)
            {
                facGC100().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kGCSMsgs::midStatus_BadDevAddr
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::AppStatus
                );
            }
            continue;
        }

        if (strType == L"SERIAL")
        {
            // Ignore these for now, since we can't really make use of them
        }
         else if (strType == L"RELAY")
        {
            // Add relay map items for the count indicated
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            {
                strName = L"Relay";
                strName.AppendFormatted(c4Index + 1);

                // Set up the temp object and add it
                gcaCur.Set
                (
                    strName, c4Index + 1, c4Mod, tGC100::EModTypes::Relay
                );
                m_colFldList.objAdd(gcaCur);
            }
        }
         else if (strType == L"IR")
        {
            //
            //  IR connectors can be configured either as sensors or as
            //  blasters, and there is no way to figure out which except
            //  to try to query the state, so we do that here.
            //
            //  For sensors, we add to the field list, since these will
            //  be related to polled fields. For blasters, we add to the
            //  zone list, which provides the zone to GC address mapping.
            //
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            {
                const ESensorCheck eCheck = eCheckSensor(c4Mod, c4Index + 1);

                if (eCheck == ESensorCheck::Sensor)
                {
                    strName = L"IRSensor";
                    strName.AppendFormatted(c4SensorId++);

                    gcaCur.Set
                    (
                        strName, c4Index + 1, c4Mod, tGC100::EModTypes::IRSensor
                    );
                    m_colFldList.objAdd(gcaCur);
                }
                 else if (eCheck == ESensorCheck::Blaster)
                {
                    // And load up a zone list item
                    strName = L"IRZone";
                    strName.AppendFormatted(c4BlasterId++);
                    gcaCur.Set
                    (
                        strName, c4Index + 1, c4Mod, tGC100::EModTypes::IRBlaster
                    );
                    m_colZoneList.objAdd(gcaCur);
                }
                 else
                {
                    // We couldn't determine what it is, so leave it out
                    facGC100().LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , kGCSMsgs::midStatus_UnknownIRConnType
                        , tCIDLib::ESeverities::Status
                        , tCIDLib::EErrClasses::AppStatus
                        , TCardinal(c4Mod)
                        , TCardinal(c4Index)
                    );
                }
            }
        }
         else if (strType == L"IR_BLASTER")
        {
            //
            //  It's an iTach FLEX, which only has one zone so they now indicate
            //  in a hard wired way what it is instead of having to query it.
            //
            strName = L"IRZone";
            strName.AppendFormatted(c4BlasterId++);
            gcaCur.Set
            (
                strName, 1, c4Mod, tGC100::EModTypes::IRBlaster
            );
            m_colZoneList.objAdd(gcaCur);
        }
         else
        {
            // Not one that we care about, so skip it
        }
    }
}


tCIDLib::TVoid TGC100Driver::RegisterFields()
{
    const tCIDLib::TCard4 c4Count = m_colFldList.c4ElemCount();
    TVector<TCQCFldDef> colFlds(16 + c4Count);
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
    colFlds.objAdd(flddCmd);

    flddCmd.Set
    (
        TFacCQCIR::strFldName_TrainingState
        , tCQCKit::EFldTypes::Boolean
        , tCQCKit::EFldAccess::Read
    );
    colFlds.objAdd(flddCmd);

    //
    //  Add some GC-100 specific fields, which are based on configuration
    //  info that we got. For each item in the field list, we create a field,
    //  based on the type of GC-100 connector it is for.
    //
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TGC100Addr& gcaCur = m_colFldList[c4Index];

        switch(gcaCur.eModType())
        {
            case tGC100::EModTypes::Relay :
            {
                flddCmd.Set
                (
                    gcaCur.strName()
                    , tCQCKit::EFldTypes::Boolean
                    , tCQCKit::EFldAccess::ReadWrite
                );
                break;
            }

            case tGC100::EModTypes::IRSensor :
            {
                flddCmd.Set
                (
                    gcaCur.strName()
                    , tCQCKit::EFldTypes::Boolean
                    , tCQCKit::EFldAccess::Read
                );
                break;
            }

            default :
            {
                // This is bad, since only the above should be in this list
                facGC100().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , L"Incorrect connector type in field list"
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::AppStatus
                );
                break;
            }
        }

        // Add this one to the list
        colFlds.objAdd(flddCmd);
    }

    // Tell our base class about our fields
    SetFields(colFlds);

    //
    //  Look up the ids of our fields, for efficiency. First we look up
    //  the fixed ones, then we go through the dynanic items and look
    //  up those.
    //
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

    // Initialize thesome that won't get otherwise initial values
    bStoreBoolFld(TFacCQCIR::strFldName_TrainingState, kCIDLib::False, kCIDLib::True);

    // We can't get any firmware info from this device, so set a default value
    bStoreStringFld(m_c4FldIdFirmwareVer, L"GC-100", kCIDLib::True);

    // And look up the ones for the dynamic fields we got from the GC-100
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TGC100Addr& gcaCur = m_colFldList[c4Index];
        gcaCur.c4FldId(pflddFind(gcaCur.strName(), kCIDLib::True)->c4Id());
    }
}


tCIDLib::TVoid
TGC100Driver::SendIRCmd(const   tCIDLib::TCard1* const  pc1Text
                        , const tCIDLib::TCard4         c4Bytes
                        , const tCIDLib::TCard4         c4ZoneNum
                        , const tCIDLib::TCard4         c4RepeatCount)
{
    try
    {
        //
        //  Get the address item from the zone list, and format it. We assume
        //  the caller check it, but if not, the collection will throw.
        //
        const TGC100Addr& gcaZone = m_colZoneList[c4ZoneNum];

        //
        //  The passed data is an ASCII string. But, it has replacement tokens
        //  in it we need to replace, so we need to convert it to a string
        //  object and replace the tokens.
        //
        //  We know the data is ASCII text so we just do a cheap loop and
        //  append transcode.
        //
        TString strTmp(c4Bytes + 32);
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Bytes; c4Index++)
            strTmp.Append(tCIDLib::TCh(pc1Text[c4Index]));

        // Make sure to get the CR on it also
        strTmp.Append(L"\r");

        //
        //  The replacement tokens are the id, which we generate by just
        //  assigning the next available id number, the module address which
        //  we get from the zone list, and the repeat count we are given.
        //
        strTmp.eReplaceToken(gcaZone.c4ModuleNum(), kCIDLib::chLatin_m);
        strTmp.eReplaceToken(gcaZone.c4ConnNum(), kCIDLib::chLatin_c);
        strTmp.eReplaceToken(m_c2NextId++, kCIDLib::chLatin_i);
        strTmp.eReplaceToken(c4RepeatCount, kCIDLib::chLatin_C);

        // And now we can send it
        SendMessage(strTmp);

        //
        //  And wait for the completeir response, or an error possibly. We have
        //  to give a pretty good wait time, because some of them are long and
        //  they get repeated a few times, so it can take a while.
        //
        GetLineMS(strTmp, 2500);
        if (!strTmp.bStartsWith(L"completeir"))
        {
            if (strTmp.c4Length() > 16)
                strTmp.CapAt(16);
            facGC100().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kGCSMsgs::midStatus_UnknownIRReply
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
                , strTmp
            );
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}


tCIDLib::TVoid TGC100Driver::SendMessage(const tCIDLib::TSCh* const pszMsg)
{
    try
    {
        // The message is a nul terminated ASCII string (with \r at the end!)
        const tCIDLib::TCard4 c4Len = TRawStr::c4StrLen(pszMsg);
        m_sockConn.Send(pszMsg, c4Len);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}

tCIDLib::TVoid TGC100Driver::SendMessage(const TString& strMsg)
{
    try
    {
        //
        //  All the messages are short and in ASCII, so we do a cheap transcode
        //  here into local buffer.
        //
        const tCIDLib::TCard4 c4BufSz = 8192;
        tCIDLib::TSCh pszBuf[c4BufSz + 1];

        const tCIDLib::TCard4 c4Len = strMsg.c4Length();
        if (c4Len > c4BufSz)
        {
            facGC100().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kGCSErrs::errcProto_SendTooLong
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::OutResource
            );
        }

        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Len; c4Index++)
            pszBuf[c4Index] = tCIDLib::TSCh(strMsg[c4Index]);

        // Ok send our local buffer
        m_sockConn.Send(pszBuf, c4Len);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}


