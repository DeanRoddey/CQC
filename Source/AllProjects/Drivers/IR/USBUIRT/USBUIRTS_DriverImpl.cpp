//
// FILE NAME: USBUIRTS_DriverImpl.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/12/2003
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
//  This file implements the bulk of the driver implementation. Some of it
//  is provided by the common IR server driver base class.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "USBUIRTS.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TUSBUIRTSDriver,TBaseIRSrvDriver)


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace USBUIRTS_DriverImpl
{
    // Our file format version marker
    const tCIDLib::TCard4   c4FmtVersion    = 1;

    //
    //  Most button presses will cause multiple IR events, which is a problem,
    //  because we only can deal with one of them. So we set a minimum
    //  interval and will throw away events until this minimum interval has
    //  passed. So we define here the interval.
    //
    const tCIDLib::TCard4   c4MinEventInterval  = 250;
}



// ---------------------------------------------------------------------------
//  Local callback methods that we tell the UIRT to call
// ---------------------------------------------------------------------------
extern "C" tCIDLib::TVoid __stdcall
USBUIRTRecCallback( const   tCIDLib::TSCh* const    pszIREventStr
                    ,       tCIDLib::TVoid* const   pUserData)
{
    // The user data is a pointer to the driver impl object
    TUSBUIRTSDriver* psdrvTar = static_cast<TUSBUIRTSDriver*>(pUserData);

    //
    //  If the current time is not past the next event time, we just assume
    //  this is a bogus 2nd, 3rd, etc... event, and we ignore it. Else, we
    //  take it and reset the next event time.
    //
    const tCIDLib::TCard4 c4CurTime = TTime::c4Millis();
    if (c4CurTime < psdrvTar->c4NextEventTime())
        return;

    // We are keeping this one, so set the next event time
    psdrvTar->c4NextEventTime(c4CurTime + USBUIRTS_DriverImpl::c4MinEventInterval);

    //
    //  Tell the driver about it. He will either store it as training data
    //  if in training mode, or queue it for processing if not in training
    //  mode.
    //
    psdrvTar->HandleRecEvent(TString(pszIREventStr));
}


extern "C" tCIDLib::TVoid __stdcall
ProgressCallback(const  tCIDLib::TCard4
                , const tCIDLib::TCard4
                , const tCIDLib::TCard4
                ,       tCIDLib::TVoid* const)
{
    // We aren't using the progress callback at this time
}


// ---------------------------------------------------------------------------
//   CLASS: THostMonSDriver
//  PREFIX: drv
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  THostMonSDriver: Constructors and Destructor
// ---------------------------------------------------------------------------
TUSBUIRTSDriver::TUSBUIRTSDriver(const TCQCDriverObjCfg& cqcdcToLoad) :

    TBaseIRSrvDriver(cqcdcToLoad)
    , m_bGotBlastTrainingData(kCIDLib::False)
    , m_bGotRecTrainingData(kCIDLib::False)
    , m_bBlastTrainingMode(kCIDLib::False)
    , m_bRecTrainingMode(kCIDLib::False)
    , m_c4FldIdFirmwareVer(kCIDLib::c4MaxCard)
    , m_c4FldIdInvoke(kCIDLib::c4MaxCard)
    , m_c4FldIdTrainingMode(kCIDLib::c4MaxCard)
    , m_c4NextEventTime(0)
    , m_c4PollCount(0)
    , m_colRecKeyQ(tCIDLib::EMTStates::Unsafe)
    , m_hUIRT(kUSBUIRT::hInvalid)
    , m_pfnGetInfoProc(nullptr)
    , m_pfnGetUIRTInfoProc(nullptr)
    , m_pfnLearnProc(nullptr)
    , m_pfnOpenProc(nullptr)
    , m_pfnSetCfgProc(nullptr)
    , m_pfnSetRecCBProc(nullptr)
    , m_pfnTransmitProc(nullptr)
    , m_pmodSupport(nullptr)
    , m_strErr_NoDevice(kUIRTSErrs::errcDev_NoDevice, facUSBUIRTS())
    , m_strErr_NoResp(kUIRTSErrs::errcDev_NoResp, facUSBUIRTS())
    , m_strErr_NoDLL(kUIRTSErrs::errcDev_NoDLL, facUSBUIRTS())
    , m_strErr_Version(kUIRTSErrs::errcDev_Version, facUSBUIRTS())
    , m_strErr_Unknown(kUIRTSErrs::errcDev_Unknown, facUSBUIRTS())
    , m_thrLearn
      (
        TString(L"USB-UIRTLearnThread_") + cqcdcToLoad.strMoniker()
        , TMemberFunc<TUSBUIRTSDriver>(this, &TUSBUIRTSDriver::eLearnThread)
      )
{
    //
    //  Make sure that we were configured for an 'other' connection. Otherwise,
    //  its a bad configuration.
    //
    if (cqcdcToLoad.conncfgReal().clsIsA() != TCQCOtherConnCfg::clsThis())
    {
        // Note that we are throwing a CQCKit error here!
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcDrv_BadConnCfgType
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Config
            , cqcdcToLoad.conncfgReal().clsIsA()
            , clsIsA()
            , TCQCOtherConnCfg::clsThis()
        );
    }
}

TUSBUIRTSDriver::~TUSBUIRTSDriver()
{
}

// ---------------------------------------------------------------------------
//  TUSBUIRTSDriver: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  If it's one we posted to ourself, store the data. Else just pass it to
//  the base IR driver class.
//
tCIDLib::TCard4
TUSBUIRTSDriver::c4SendCmd(const TString& strCmdId, const TString& strParms)
{
    if (strCmdId == kUSBUIRTS::pszCmd_UIRTData)
    {
        //
        //  If we are in receiver training mode we have to store it as
        //  training data. Else, if we aren't in blaster training mode, we
        //  want to queue it as an event to process.
        //
        if (m_bRecTrainingMode)
        {
            m_bGotRecTrainingData = kCIDLib::True;
            m_strRecTrainData = strParms;
        }
         else
        {
            if (!m_bBlastTrainingMode)
                m_colRecKeyQ.objPut(strParms);
        }

        return 0;
    }

    return TParent::c4SendCmd(strCmdId, strParms);
}


// ---------------------------------------------------------------------------
//  TUSBUIRTSDriver: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TCard4 TUSBUIRTSDriver::c4NextEventTime() const
{
    return m_c4NextEventTime;
}

tCIDLib::TCard4 TUSBUIRTSDriver::c4NextEventTime(const tCIDLib::TCard4 c4ToSet)
{
    m_c4NextEventTime = c4ToSet;
    return m_c4NextEventTime;
}


//
//  NOTE!!!!!
//
//  This is called from the USB UIRT callback. It is NOT a CQC thread, so
//  we have to do the minimum work possible just to queue up the passed
//  event string because, if anything goes wrong, it will inevitably invoke
//  some code that assumes it is running in a CQC thread.
//
//  Since it does come asynchronously from the UIRT calling us back, we
//  just post ourself an async command using the existing async driver command
//  infrastructure.
//
tCIDLib::TVoid TUSBUIRTSDriver::HandleRecEvent(const TString& strEventStr)
{
    pdcmdQSendCmd
    (
        kUSBUIRTS::pszCmd_UIRTData, strEventStr, tCQCKit::EDrvCmdWaits::NoWait
    );
}


// ---------------------------------------------------------------------------
//  TUSBUIRTSDriver: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TUSBUIRTSDriver::bBlastTrainingMode()
{
    // No need to lock for this simple check
    return m_bBlastTrainingMode;
}


tCIDLib::TBoolean
TUSBUIRTSDriver::bCheckBlastTrainingData(tCIDLib::TCard4&   c4DataBytes
                                        , THeapBuf&         mbufToFill)
{
    // If we aren't online, then throw
    CheckOnline(CID_FILE, CID_LINE);

    if (m_bGotBlastTrainingData)
    {
        //
        //  We got something, so copy it over and clear the flags. Be sure
        //  to copy the nul, so that when the data is returned later, we can
        //  just send it as is.
        //
        c4DataBytes = TRawStr::c4StrLen(m_pchBlastTrainData) + 1;
        mbufToFill.CopyIn(m_pchBlastTrainData, c4DataBytes);

        m_bBlastTrainingMode = kCIDLib::False;
        m_bGotBlastTrainingData = kCIDLib::False;

        bStoreBoolFld(m_c4FldIdTrainingMode, kCIDLib::False, kCIDLib::True);
        return kCIDLib::True;
    }

    c4DataBytes = 0;
    return kCIDLib::False;
}


tCIDLib::TBoolean
TUSBUIRTSDriver::bCheckRecTrainingData(TString& strKeyToFill)
{
    // If we aren't online, then throw, else lock and do it
    CheckOnline(CID_FILE, CID_LINE);
    if (m_bGotRecTrainingData)
    {
        // We have some, so copy to output and clear our stuff
        strKeyToFill = m_strRecTrainData;
        m_bGotRecTrainingData = kCIDLib::False;
        m_strRecTrainData.Clear();
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


tCIDLib::TBoolean
TUSBUIRTSDriver::bCvtManualBlastData(const  TString&            strText
                                    ,       tCIDLib::TCard4&    c4DataBytes
                                    ,       THeapBuf&           mbufToFill
                                    ,       TString&            strError)
{
    //
    //  Use a regular expression to validate the data, which must be a
    //  sequence of one or more sets of 4 hex digits, separated by spaces
    //  but the last one doesn't have to have any space after it, and
    //  should not because the client should streaming leading and trailng
    //  space.
    //
    if (!facCQCIR().bIsValidUIRTBlastData(strText))
    {
        strError.LoadFromMsg
        (
            kIRErrs::errcFmt_BadIRDataFormat, facCQCIR(), TString(L"UIRT")
        );
        return kCIDLib::False;
    }

    //
    //  For us, it has to be in Pronto format, since that's our native
    //  format and the only alternative format that this method has to
    //  accept. So we just convert it to the desired ultimate format,
    //  which is single byte ASCII characters with a null terminator. So
    //  we just do a simple transcode by taking the low byte of each char.
    //
    const tCIDLib::TCard4 c4Len = strText.c4Length();
    CIDAssert(c4Len != 0, L"The incoming IR data length was zero");
    for (c4DataBytes = 0; c4DataBytes< c4Len; c4DataBytes++)
        mbufToFill.PutCard1(tCIDLib::TCard1(strText[c4DataBytes]), c4DataBytes);

    // Add the null terminator
    mbufToFill.PutCard1(0, c4DataBytes++);
    return kCIDLib::True;
}


tCIDLib::TBoolean TUSBUIRTSDriver::bGetCommResource(TThread&)
{
    // Try to find the UIRT library if not done so far
    if (m_hUIRT == kUSBUIRT::hInvalid)
        m_hUIRT = m_pfnOpenProc();

    return (m_hUIRT != kUSBUIRT::hInvalid);
}


tCIDLib::TBoolean TUSBUIRTSDriver::bRecTrainingMode()
{
    // No need to lock for this simple check
    return m_bRecTrainingMode;
}


tCIDLib::TBoolean TUSBUIRTSDriver::bResetConnection()
{
    // If we aren't online, then throw, else lock and do it
    CheckOnline(CID_FILE, CID_LINE);

    //
    //  The device doesn't require anything, but we want to get ourselves
    //  out of any training modes and clear the event queue.
    //
    ExitBlastTrainingMode();
    ExitRecTrainingMode();
    ClearEventQ();

    return kCIDLib::True;
}


tCIDLib::TCard4 TUSBUIRTSDriver::c4InvokeFldId() const
{
    return m_c4FldIdInvoke;
}


//
//  The later UIRTs have three zones, with the open air one being
//  zone one, a second one being on the back and third one that you
//  can access as well.
//
tCIDLib::TCard4 TUSBUIRTSDriver::c4ZoneCount() const
{
    return 3;
}


tCIDLib::TVoid TUSBUIRTSDriver::ClearBlastTrainingData()
{
    // If we aren't online, then throw, else lock and do it
    CheckOnline(CID_FILE, CID_LINE);
    m_bGotBlastTrainingData = kCIDLib::False;
}


tCIDLib::TVoid TUSBUIRTSDriver::ClearRecTrainingData()
{
    // If we aren't online, then throw, else lock and do it
    CheckOnline(CID_FILE, CID_LINE);

    m_bGotRecTrainingData = kCIDLib::False;
    m_strRecTrainData.Clear();
}


tCQCKit::ECommResults TUSBUIRTSDriver::eConnectToDevice(TThread&)
{
    //
    //  If we got the comm resource, we are connected, but get the firmware
    //  info and set that field now. If this fails, then something really
    //  bad has happened, since it should be doable even if there's no
    //  hardware attached, so return lostcommres.
    //
    TUIRTInfo Info;
    if (!m_pfnGetUIRTInfoProc(m_hUIRT, &Info))
        return tCQCKit::ECommResults::LostCommRes;

    TString strFW(L"USB-UIRT FWVer=");
    strFW.AppendFormatted(Info.c4FWVersion >> 8);
    strFW.Append(kCIDLib::chPeriod);
    strFW.AppendFormatted(Info.c4FWVersion & 0xFF);

    strFW.Append(L", ProtoVer=");
    strFW.AppendFormatted(Info.c4ProtVersion >> 8);
    strFW.Append(kCIDLib::chPeriod);
    strFW.AppendFormatted(Info.c4ProtVersion & 0xFF);

    strFW.Append(L", Date=");
    strFW.AppendFormatted(tCIDLib::TCard4(Info.c1Year + 2000));
    strFW.Append(kCIDLib::chForwardSlash);
    strFW.AppendFormatted(Info.c1Month);
    strFW.Append(kCIDLib::chForwardSlash);
    strFW.AppendFormatted(Info.c1Day);

    // Lock for field write and set the field
    bStoreStringFld(m_c4FldIdFirmwareVer, strFW, kCIDLib::True);

    // Set the receiver callback
    m_pfnSetRecCBProc(m_hUIRT, USBUIRTRecCallback, this);

    // And set the flags
    m_pfnSetCfgProc(m_hUIRT, kUSBUIRT::c4Cfg_LEDRX | kUSBUIRT::c4Cfg_LEDTX);

    return tCQCKit::ECommResults::Success;
}


tCQCKit::EDrvInitRes TUSBUIRTSDriver::eInitializeImpl()
{
    // Call the base IR driver first
    TParent::eInitializeImpl();

    //
    //  We have to load the USB-UIRT interface DLL dynamically, and look
    //  up the APIs we are going to call. We need to build the path to
    //  the DLL and check that it exists before we try to load it. It
    //  will be in the bin directory.
    //
    //  This is not one of our facilities, so we have to just build up
    //  the raw name ourself, we can't use the CIDlib methods. We just
    //  find it in the system path.
    //
    TPathStr pathLibName;
    TFindBuf fndbUULib;
    if (!TFileSys::bExistsInSysPath(kUSBUIRTS::pszSupportDLL, fndbUULib))
    {
        // Log this so that they'll know why
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
        {
            facUSBUIRTS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kUIRTSErrs::errcComm_DLLNotFound
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::NotFound
            );
        }
        return tCQCKit::EDrvInitRes::Failed;
    }

    //
    //  And try to load this module if we haven't already. It's a third party
    //  module, so we use the ctor that takes the path to load from specifically
    //  and no CIDLib stuff.
    //
    if (!m_pmodSupport)
    {
        try
        {
            m_pmodSupport = new TModule(fndbUULib.pathFileName(), kCIDLib::True);
        }

        catch(const TError& errToCatch)
        {
            if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
            {
                LogError(errToCatch, tCQCKit::EVerboseLvls::Medium);
                facUSBUIRTS().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kUIRTSErrs::errcComm_DLLNotLoaded
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::NotFound
                );
            }
            return tCQCKit::EDrvInitRes::Failed;
        }

        catch(...)
        {
            if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
            {
                facUSBUIRTS().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kUIRTSErrs::errcComm_DLLNotLoaded
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::NotFound
                );
            }
            return tCQCKit::EDrvInitRes::Failed;
        }
    }

    // Load up all of the entry points we need to use
    try
    {
        m_pfnCloseProc = reinterpret_cast<TUUIRTClose>
        (
            m_pmodSupport->pfnGetFuncPtr("UUIRTClose")
        );

        m_pfnGetInfoProc = reinterpret_cast<TUUIRTGetDrvInfo>
        (
            m_pmodSupport->pfnGetFuncPtr("UUIRTGetDrvInfo")
        );

        m_pfnGetUIRTInfoProc = reinterpret_cast<TUUIRTGetUUIRTInfo>
        (
            m_pmodSupport->pfnGetFuncPtr("UUIRTGetUUIRTInfo")
        );

        m_pfnLearnProc = reinterpret_cast<TUUIRTLearnIR>
        (
            m_pmodSupport->pfnGetFuncPtr("UUIRTLearnIR")
        );

        m_pfnOpenProc = reinterpret_cast<TUUIRTOpen>
        (
            m_pmodSupport->pfnGetFuncPtr("UUIRTOpen")
        );

        m_pfnSetCfgProc = reinterpret_cast<TUUIRTSetUUIRTConfig>
        (
            m_pmodSupport->pfnGetFuncPtr("UUIRTSetUUIRTConfig")
        );

        m_pfnSetRecCBProc = reinterpret_cast<TUUIRTSetRecCB>
        (
            m_pmodSupport->pfnGetFuncPtr("UUIRTSetReceiveCallback")
        );

        m_pfnTransmitProc = reinterpret_cast<TUUIRTTransmitIR>
        (
            m_pmodSupport->pfnGetFuncPtr("UUIRTTransmitIR")
        );
    }

    catch(...)
    {
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
        {
            facUSBUIRTS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kUIRTSErrs::errcComm_FuncsNotLoaded
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::NotFound
            );
        }
        return tCQCKit::EDrvInitRes::Failed;
    }

    //
    //  Register our couple of fields. We just provide one that indicates
    //  the training mode state, so that the client can display an
    //  indicator that shows whether the driver is in training mode or not.
    //
    tCQCKit::TFldDefList colFlds(8);
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
    //  interactive device, and it has a good fast speed. Set the recon
    //  time pretty slow since we would only ever fail if the hardware is
    //  not present, so no need to keep banging away fast.
    //
    SetPollTimes(50, 10000);

    //
    //  Crank up the actions processing thread if not already running. It
    //  runs until we are unloaded pulling events out of the queue and
    //  processing them.
    //
    StartActionsThread();

    //
    //  In our case we want to go to 'wait for config' mode, not wait for
    //  comm res, since we need to get configuration before we can go online.
    //
    return tCQCKit::EDrvInitRes::WaitConfig;
}


tCQCKit::ECommResults TUSBUIRTSDriver::ePollDevice(TThread&)
{
    tCQCKit::ECommResults eRetVal = tCQCKit::ECommResults::Success;

    //
    //  If we aren't in blaster or training mode, then let's bump the poll
    //  count, and every so many times through, ping the device to make
    //  sure it's still there.
    //
    if (!m_bBlastTrainingMode && !m_bRecTrainingMode)
    {
        //
        //  Every so many times through, we ping the device to make sure
        //  it's still there. This should be about every 10 seconds at
        //  our poll rate.
        //
        m_c4PollCount++;
        if (m_c4PollCount > 200)
        {
            TUIRTInfo Info;
            if (!m_pfnGetUIRTInfoProc(m_hUIRT, &Info))
                eRetVal = tCQCKit::ECommResults::LostCommRes;
            m_c4PollCount = 0;
        }
    }

    const tCIDLib::TCard4 c4RecKeyCnt = m_colRecKeyQ.c4ElemCount();
    if (eRetVal != tCQCKit::ECommResults::Success)
    {
        // We are losing the connection so flush the queue
        m_colRecKeyQ.RemoveAll();
    }
     else if (c4RecKeyCnt)
    {
        //
        //  If there are any queued receiver keys, queue them up on
        //  our parent class.
        //
        //  BEAR in mind we have a lock above. But all we are doing here
        //  is just passing in soem data to be queued up.
        //
        TString strKey;
        while (m_colRecKeyQ.bGetNextMv(strKey, 0))
            QueueRecEvent(strKey);
    }
    return eRetVal;
}


tCIDLib::TVoid TUSBUIRTSDriver::EnterBlastTrainingMode()
{
    // If we aren't online, then throw, else lock and do it
    CheckOnline(CID_FILE, CID_LINE);

    // If we are already in training mode, then that's an error
    if (m_bBlastTrainingMode)
    {
        facCQCIR().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kIRErrs::errcTrain_AlreadyTraining
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Already
        );
    }

    //
    //  We make them wait until all the queued events are processed before
    //  they can do any training.
    //
    WaitForActions();

    //
    //  Go into training mode. Since it's a blocking call on the USB-UIRT,
    //  we spin off a thread to do the work. When it exits, it'll set
    //  the got data flag appropriately.
    //
    m_bGotBlastTrainingData = kCIDLib::False;
    m_bBlastTrainingMode = kCIDLib::True;
    m_thrLearn.Start();

    // Look for field access and update the training mode field
    bStoreBoolFld
    (
        m_c4FldIdTrainingMode
        , m_bRecTrainingMode || m_bBlastTrainingMode
        , kCIDLib::True
    );
}


tCIDLib::TVoid TUSBUIRTSDriver::EnterRecTrainingMode()
{
    // Can't already be in training mode
    if (m_bRecTrainingMode)
    {
        facCQCIR().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kIRErrs::errcTrain_AlreadyTraining
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Already
            , strMoniker()
        );
    }

    //
    //  We make them wait until all the queued events are processed before
    //  they can do any training.
    //
    WaitForActions();

    // Set the record training mode flag and clear the data flags
    m_bRecTrainingMode = kCIDLib::True;
    m_bGotRecTrainingData = kCIDLib::False;
    m_strRecTrainData.Clear();

    // And then update the training mode field
    bStoreBoolFld
    (
        m_c4FldIdTrainingMode
        , m_bRecTrainingMode || m_bBlastTrainingMode
        , kCIDLib::True
    );
}


tCIDLib::TVoid TUSBUIRTSDriver::ExitBlastTrainingMode()
{
    //
    //  Set the flag that the learn procedure uses as an abort flag. This
    //  will cause the thread to exit.
    //
    m_c4CancelLearn = 1;

    // Wait for the learning thread to exit
    try
    {
        if (m_thrLearn.bIsRunning())
            m_thrLearn.eWaitForDeath(2000);
    }

    catch(const TError& errToCatch)
    {
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Low)
        {
            LogError(errToCatch, tCQCKit::EVerboseLvls::Low);
            facCQCIR().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kIRErrs::errcTrain_StopTrainThread
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::CantDo
                , strMoniker()
            );
        }
    }

    // Clear the flags first, then update the training mode field
    m_bBlastTrainingMode = kCIDLib::False;
    m_bGotBlastTrainingData = kCIDLib::False;

    bStoreBoolFld
    (
        m_c4FldIdTrainingMode
        , m_bRecTrainingMode || m_bBlastTrainingMode
        , kCIDLib::True
    );
}


tCIDLib::TVoid TUSBUIRTSDriver::ExitRecTrainingMode()
{
    // Clear our own flags
    m_bRecTrainingMode = kCIDLib::False;
    m_bGotRecTrainingData = kCIDLib::False;

    // And then update the training mode field
    bStoreBoolFld
    (
        m_c4FldIdTrainingMode
        , m_bRecTrainingMode || m_bBlastTrainingMode
        , kCIDLib::True
    );

    // The UIRT is not in any special mode for this, so nothing more to do
}


// We have to format the data for the indicated command to text
tCIDLib::TVoid
TUSBUIRTSDriver::FormatBlastData(const  TIRBlasterCmd&  irbcFmt
                                ,       TString&        strToFill)
{
    //
    //  The data is just the text as ASCII, so we can do a cheap transcode
    //  to the string by just expanding each byte.
    //
    //  In our case it has a null on it, so don't copy that.
    //
    const tCIDLib::TCard4 c4Count = irbcFmt.c4DataLen();
    strToFill.Clear();
    if (c4Count)
    {
        const tCIDLib::TCard1* pc1Src = irbcFmt.mbufData().pc1Data();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count - 1; c4Index++)
            strToFill.Append(tCIDLib::TCh(*pc1Src++));
    }
}


tCIDLib::TVoid
TUSBUIRTSDriver::InvokeBlastCmd(const   TString&        strDevice
                                , const TString&        strCmd
                                , const tCIDLib::TCard4 c4ZoneNum)
{
    //
    //  We can check the zone number up front without locking. They are
    //  zero based.
    //
    if (c4ZoneNum >= c4ZoneCount())
    {
        facCQCIR().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kIRErrs::errcBlast_BadZone
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::CantDo
            , TCardinal(c4ZoneNum + 1)
            , strMoniker()
        );
    }

    // If we aren't online, then throw
    CheckOnline(CID_FILE, CID_LINE);

    // We can't allow this if either training mode is active
    if (m_bRecTrainingMode || m_bBlastTrainingMode)
    {
        facCQCIR().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kIRErrs::errcTrain_BusyTraining
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::CantDo
            , strMoniker()
        );
    }

    // Look up the command
    tCIDLib::TCard4 c4Repeat;
    const TIRBlasterCmd& irbcInvoke = irbcFromName(strDevice, strCmd, c4Repeat);

    // And send it
    SendIRData(irbcInvoke.mbufData(), irbcInvoke.c4DataLen(), c4Repeat, c4ZoneNum);

    // Reset the poll counter since we ust verified that it's alive
    m_c4PollCount = 0;
}



tCIDLib::TVoid TUSBUIRTSDriver::ReleaseCommResource()
{
    try
    {
        // If we have a handle, then try to close it
        if (m_hUIRT != kUSBUIRT::hInvalid)
        {
            m_pfnCloseProc(m_hUIRT);
            m_hUIRT = kUSBUIRT::hInvalid;
        }
    }

    catch(TError& errToCatch)
    {
        LogError(errToCatch, tCQCKit::EVerboseLvls::Medium, CID_FILE, CID_LINE);
    }
}


tCIDLib::TVoid
TUSBUIRTSDriver::SendBlasterData(const  tCIDLib::TCard4 c4DataBytes
                                , const TMemBuf&        mbufToSend
                                , const tCIDLib::TCard4 c4ZoneNum
                                , const tCIDLib::TCard4 c4RepeatCount)
{
    // We can check the zone number up front without locking
    if (c4ZoneNum >= c4ZoneCount())
    {
        facCQCIR().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kIRErrs::errcBlast_BadZone
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::CantDo
            , TCardinal(c4ZoneNum + 1)
            , strMoniker()
        );
    }

    // If we aren't online, then throw, else lock and do it
    CheckOnline(CID_FILE, CID_LINE);

    // We can't allow this if either training mode is active
    if (m_bRecTrainingMode || m_bBlastTrainingMode)
    {
        facCQCIR().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kIRErrs::errcTrain_BusyTraining
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::CantDo
            , strMoniker()
        );
    }

    // Send the data
    SendIRData(mbufToSend, c4DataBytes, c4RepeatCount, c4ZoneNum);
}


tCIDLib::TVoid TUSBUIRTSDriver::TerminateImpl()
{
    // If we are in blaster training mode, we have to get out
    if (m_bBlastTrainingMode && !m_c4CancelLearn)
    {
        m_c4CancelLearn = 1;

        // Wait for the thread to exit
        try
        {
            if (m_thrLearn.bIsRunning())
                m_thrLearn.eWaitForDeath(2000);
        }

        catch(TError& errToCatch)
        {
            if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Low)
            {
                LogError(errToCatch, tCQCKit::EVerboseLvls::Low, CID_FILE, CID_LINE);
                facCQCIR().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kIRErrs::errcTrain_StopTrainThread
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Term
                    , strMoniker()
                );
            }
        }
    }

    // Unload the module if we loaded it
    if (m_pmodSupport)
    {
        try
        {
            delete m_pmodSupport;
            m_pmodSupport = 0;
        }

        catch(TError& errToCatch)
        {
            LogError(errToCatch, tCQCKit::EVerboseLvls::Low, CID_FILE, CID_LINE);
        }
    }

    // And call the base IR driver class last
    TParent::TerminateImpl();
}


// ---------------------------------------------------------------------------
//  TUSBUIRTSDriver: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::EExitCodes
TUSBUIRTSDriver::eLearnThread(TThread& thrThis, tCIDLib::TVoid*)
{
    // Let the calling thread go
    thrThis.Sync();

    //
    //  We don't have to loop or anything here. We are just going to make
    //  one blocking call.
    //
    try
    {
        m_c4CancelLearn = 0;
        const tCIDLib::TSInt iRes = m_pfnLearnProc
        (
            m_hUIRT
            , kUSBUIRT::c4IRFmt_Pronto
            , m_pchBlastTrainData
            , ProgressCallback
            , 0
            , &m_c4CancelLearn
            , 0
            , 0
            , 0
        );

        if (iRes)
        {
            //
            //  If we didn't get out due to a cancel, then set the flag
            //  indicating we have data, and set the next event time so
            //  that if they don't react quickly, we won't pick up any
            //  other events that come in due to the button they are
            //  pressing.
            //
            if (!m_c4CancelLearn)
                m_bGotBlastTrainingData = kCIDLib::True;
            m_c4NextEventTime = TTime::c4Millis() + 1000;
        }
         else
        {
            // We got an error so log it
            TKrnlError kerrUUIRT = TKrnlError::kerrLast();
            facCQCIR().LogKrnlErr
            (
                CID_FILE
                , CID_LINE
                , kIRErrs::errcTrain_TrainError
                , kerrUUIRT
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Internal
                , strMoniker()
                , strUUIRTErr(kerrUUIRT.errcHostId())
            );
        }
    }

    catch(...)
    {
        facCQCIR().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kIRErrs::errcTrain_TrainUExcept
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Internal
            , strMoniker()
        );
    }
    return tCIDLib::EExitCodes::Normal;
}


const TString&
TUSBUIRTSDriver::strUUIRTErr(const tCIDLib::TOSErrCode errcToLoad)
{
    switch(errcToLoad)
    {
        case kUSBUIRT::c4Err_NoDevice :
            return m_strErr_NoDevice;
            break;

        case kUSBUIRT::c4Err_NoResp :
            return m_strErr_NoResp;
            break;

        case kUSBUIRT::c4Err_NoDLL :
            return m_strErr_NoDLL;
            break;

        case kUSBUIRT::c4Err_Version :
            return m_strErr_Version;
            break;
    };
    return m_strErr_Unknown;
}


tCIDLib::TVoid
TUSBUIRTSDriver::SendIRData(const   TMemBuf&        mbufData
                            , const tCIDLib::TCard4 c4Bytes
                            , const tCIDLib::TCard4 c4RepeatCount
                            , const tCIDLib::TCard4 c4ZoneNum)
{
    //
    //  Map the zone number to the UIRT's zone number. We have to map
    //  our Zone 0 to its Z3 (which is the front open air blaster), so
    //  that we maintain backwards compability with the pre-zone support
    //  version of this driver.
    //
    //  Our mapping is:
    //
    //  Zone 0 : Z3
    //  Zone 1 : Z1
    //  Zone 2 : Z2
    //
    tCIDLib::TSCh chZone;
    switch(c4ZoneNum)
    {
        case 1 :
            chZone = '1';
            break;

        case 2 :
            chZone = '2';
            break;

        default :
            chZone = '3';
            break;
    };

    //
    //  Our data is an ASCII string, but it's stored as binary data and
    //  the null cannot be stored on it, so we have to put on on, which
    //  requires making a local buffer. We have to prepend the zone stuff
    //  before it.
    //
    tCIDLib::TSCh* pszData = new tCIDLib::TSCh[c4Bytes + 3];
    pszData[0] = kCIDLib::chLatin_Z;
    pszData[1] = chZone;
    TRawMem::CopyMemBuf(pszData + 2, mbufData.pc1Data(), c4Bytes);
    pszData[c4Bytes + 2] = 0;
    TArrayJanitor<tCIDLib::TSCh> janBuf(pszData);

    // And send the data
    tCIDLib::TSInt iRet = m_pfnTransmitProc
    (
        m_hUIRT
        , pszData
        , kUSBUIRT::c4IRFmt_Pronto
        , c4RepeatCount
        , 0
        , 0
        , 0
        , 0
    );

    if (!iRet)
    {
        TKrnlError kerrUUIRT = TKrnlError::kerrLast();
        facCQCIR().ThrowKrnlErr
        (
            CID_FILE
            , CID_LINE
            , kIRErrs::errcDev_DevError
            , kerrUUIRT
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Internal
            , strMoniker()
            , strUUIRTErr(kerrUUIRT.errcHostId())
        );
    }
}

