//
// FILE NAME: GenTrigS_DriverImpl.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/16/2007
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
#include    "GenTrigS.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TGenTrigSDriver,TBaseIRSrvDriver)


// ---------------------------------------------------------------------------
//   CLASS: THostMonSDriver
//  PREFIX: drv
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  THostMonSDriver: Constructors and Destructor
// ---------------------------------------------------------------------------
TGenTrigSDriver::TGenTrigSDriver(const TCQCDriverObjCfg& cqcdcToLoad) :

    TBaseIRSrvDriver(cqcdcToLoad)
    , m_bGotRecTrainingData(kCIDLib::False)
    , m_bRecTrainingMode(kCIDLib::False)
    , m_bSocket(kCIDLib::False)
    , m_c4FldIdInvoke(kCIDLib::c4MaxCard)
    , m_c4FldIdTrainingMode(kCIDLib::c4MaxCard)
    , m_eTermType(tGenTrigS::ETermTypes::CRLF)
    , m_pcommTrig(nullptr)
    , m_psockTrig(nullptr)
{
}

TGenTrigSDriver::~TGenTrigSDriver()
{
    // Clean up the socket or port
    if (m_pcommTrig)
    {
        try
        {
            delete m_pcommTrig;
            m_pcommTrig = nullptr;
        }

        catch(...) {}
    }

    if (m_psockTrig)
    {
        try
        {
            delete m_psockTrig;
            m_psockTrig = nullptr;
        }

        catch(...) {}
    }
}


// ---------------------------------------------------------------------------
//  TGenTrigSDriver: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TGenTrigSDriver::bCheckRecTrainingData(TString& strKeyToFill)
{
    // If we aren't online, then throw
    CheckOnline(CID_FILE, CID_LINE);

    // If we have some receiver training data, then give it back
    if (m_bGotRecTrainingData)
    {
        // We have some, so copy to output and clear ours
        strKeyToFill = m_strRecTrainData;
        m_bGotRecTrainingData = kCIDLib::False;
        m_strRecTrainData.Clear();
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


tCIDLib::TBoolean TGenTrigSDriver::bGetCommResource(TThread&)
{
    // Open the port/socket now, and set the configuration
    try
    {
        if (m_bSocket)
        {
            // Just open, we don't connect yet. Close if already just in case
            if (m_psockTrig->bIsOpen())
                m_psockTrig->Close();

            m_psockTrig->Open
            (
                m_conncfgSock.eProto(), m_conncfgSock.ipepConn().eAddrType()
            );
        }
         else
        {
            // Close it if already open just in case
            if (m_pcommTrig->bIsOpen())
                m_pcommTrig->Close();

            m_pcommTrig->Open();
            m_pcommTrig->SetCfg(m_cpcfgSerial);
        }
    }

    catch(TError& errToCatch)
    {
        LogError(errToCatch, tCQCKit::EVerboseLvls::Medium, CID_FILE, CID_LINE);
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


tCIDLib::TBoolean TGenTrigSDriver::bRecTrainingMode()
{
    return m_bRecTrainingMode;
}


//
//  We don't require that we be connected for this one, so that it can be
//  used to reset us in some bad circumstances.
//
tCIDLib::TBoolean TGenTrigSDriver::bResetConnection()
{
    //
    //  The device doesn't require anything, but we want to get ourselves
    //  out of any training modes and clear the event queue.
    //
    if (m_bSocket)
    {
        // <TBD>
    }
     else
    {
        m_pcommTrig->PurgeReadData();
        m_pcommTrig->PurgeWriteData();
    }
    ExitRecTrainingMode();
    ClearEventQ();

    return kCIDLib::True;
}


tCIDLib::TCard4 TGenTrigSDriver::c4InvokeFldId() const
{
    return m_c4FldIdInvoke;
}


tCIDLib::TVoid TGenTrigSDriver::ClearRecTrainingData()
{
    // If we aren't online, then throw, else do it
    CheckOnline(CID_FILE, CID_LINE);
    m_bGotRecTrainingData = kCIDLib::False;
    m_strRecTrainData.Clear();
}


tCQCKit::ECommResults TGenTrigSDriver::eConnectToDevice(TThread&)
{
    // If socket based, then try to connect
    if (m_bSocket)
    {
        // If not connected, then try to connect
        if (!m_psockTrig->bIsConnected())
        {
            try
            {
                // Try to connect to the remote end point
                m_psockTrig->Connect(m_conncfgSock.ipepConn());
            }

            catch(TError& errToCatch)
            {
                LogError(errToCatch, tCQCKit::EVerboseLvls::Medium, CID_FILE, CID_LINE);
                return tCQCKit::ECommResults::LostConnection;
            }
        }
    }
    return tCQCKit::ECommResults::Success;
}


tCQCKit::EDrvInitRes TGenTrigSDriver::eInitializeImpl()
{
    // Call our parent IR base drivers
    TParent::eInitializeImpl();

    //
    //  See which kind of configuration we got. It needs to be serial
    //  or socket. We store away the info and the type of connection we are
    //  to use. Clean up any previous ones
    //
    delete m_pcommTrig;
    m_pcommTrig = nullptr;
    delete m_psockTrig;
    m_psockTrig = nullptr;

    const TCQCDriverObjCfg& cqcdcOurs = cqcdcThis();
    if (cqcdcOurs.conncfgReal().clsIsA() == TCQCSerialConnCfg::clsThis())
    {
        const TCQCSerialConnCfg& conncfgSer
                = static_cast<const TCQCSerialConnCfg&>(cqcdcOurs.conncfgReal());
        m_strPort = conncfgSer.strPortPath();
        m_cpcfgSerial = conncfgSer.cpcfgSerial();
        m_pcommTrig = facCIDComm().pcommMakeNew(m_strPort);
        m_bSocket = kCIDLib::False;
    }
     else if (cqcdcOurs.conncfgReal().clsIsA() == TCQCIPConnCfg::clsThis())
    {
        m_conncfgSock = static_cast<const TCQCIPConnCfg&>(cqcdcOurs.conncfgReal());
        m_psockTrig = new TClientStreamSocket
        (
            m_conncfgSock.eProto(), tCIDSock::EAddrTypes::Unspec
        );
        m_bSocket = kCIDLib::True;
    }
     else
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
    //  We may get a termination type prompt. If it's not there, then
    //  it's probably an RP6 driver. We switched them over to this driver
    //  which can handle that functionality plus it's more general. But
    //  that driver didn't provide the prompt because it knew what type
    //  of termination the RP6 uses.
    //
    TString strPromptVal;
    m_eTermType = tGenTrigS::ETermTypes::CRLF;
    if (cqcdcOurs.bFindPromptValue(L"TermType", L"Selected", strPromptVal))
    {
        if (strPromptVal == L"CR")
            m_eTermType = tGenTrigS::ETermTypes::CR;
        else if (strPromptVal == L"LF")
            m_eTermType = tGenTrigS::ETermTypes::LF;
        else if (strPromptVal == L"CR/LF")
            m_eTermType = tGenTrigS::ETermTypes::CRLF;
        else
        {
            if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
            {
                facGenTrigS().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kGenTrigSErrs::errInit_BadTermPrompt
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::AppStatus
                    , strPromptVal
                );
                return tCQCKit::EDrvInitRes::Failed;
            }
        }
    }

    //
    //  We can have a driver prompt that lets the user enter a separator
    //  character for us to use. If not provided (for older installs), then
    //  we set it to a null character, so that there is no separator.
    //
    if (cqcdcOurs.bFindPromptValue(L"SepChar", L"Text", strPromptVal))
    {
        tCIDLib::TCh chSepChar = 0;
        if (!strPromptVal.bIsEmpty())
            chSepChar = strPromptVal[0];

        // Set it on our parent class
        SetKeySepChar(chSepChar);
    }

    //
    //  Register our couple of fields. We just provide one that indicates
    //  the training mode state, so that the client can display an
    //  indicator that shows whether the driver is in training mode or not.
    //
    TVector<TCQCFldDef> colFlds(8);
    TCQCFldDef flddCmd;

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

    // Look up the ids of the fields we need to update regularly, for efficiency.
    m_c4FldIdInvoke = pflddFind(TFacCQCIR::strFldName_Invoke, kCIDLib::True)->c4Id();
    m_c4FldIdTrainingMode = pflddFind(TFacCQCIR::strFldName_TrainingState, kCIDLib::True)->c4Id();

    // These we need to just store something or get an initial value stored
    bStoreStringFld(TFacCQCIR::strFldName_FirmwareVer, cqcdcThis().strVersion(), kCIDLib::True);
    UseDefFldValue(m_c4FldIdTrainingMode);

    //
    //  Set the poll time faster than normal. This is a very interactive
    //  device, and it has a good fast speed.
    //
    SetPollTimes(50, 5000);

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


tCQCKit::ECommResults TGenTrigSDriver::ePollDevice(TThread&)
{
    tCQCKit::ECommResults eRetVal = tCQCKit::ECommResults::Success;

    //
    //  Look for messages with a short timeout, i.e. just get anything that
    //  is waiting. We have to pass in etiher the socket or port, basd on
    //  connection type.
    //
    TObject* pobjComm;
    if (m_bSocket)
        pobjComm = m_psockTrig;
    else
        pobjComm = m_pcommTrig;

    // And we have to set the line term chars based on what we were told
    tCIDLib::TCard1 c1Term1;
    tCIDLib::TCard1 c1Term2;
    switch(m_eTermType)
    {
        case tGenTrigS::ETermTypes::CR :
            c1Term1 = 13;
            c1Term2 = 0;
            break;

        case tGenTrigS::ETermTypes::LF :
            c1Term1 = 10;
            c1Term2 = 0;
            break;

        case tGenTrigS::ETermTypes::CRLF :
            c1Term1 = 13;
            c1Term2 = 10;
            break;

        default :
            if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
            {
                facGenTrigS().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kGenTrigSErrs::errInit_BadTermType
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::AppStatus
                    , TCardinal(tCIDLib::c4EnumOrd(m_eTermType))
                );
            }
            c1Term1 = 13;
            c1Term2 = 10;
            break;
    };

    while (bGetASCIITermMsg(*pobjComm, 10, c1Term1, c1Term2, m_strPollStr))
    {
        //
        //  Ok, we have a key. If we are in training mode, then just store
        //  it in the m_strTrainVal member. Else, we need to queue it on
        //  the action queue.
        //
        if (m_bRecTrainingMode)
        {
            m_strRecTrainData = m_strPollStr;

            // If we have a sep char, store just the key part
            if (chSepChar())
                m_strRecTrainData.bCapAtChar(chSepChar());

            m_bGotRecTrainingData = kCIDLib::True;
        }
         else
        {
            QueueRecEvent(m_strPollStr);
        }
    }
    return eRetVal;
}


tCIDLib::TVoid TGenTrigSDriver::EnterRecTrainingMode()
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
    bStoreBoolFld(m_c4FldIdTrainingMode, m_bRecTrainingMode, kCIDLib::True);
}


tCIDLib::TVoid TGenTrigSDriver::ExitRecTrainingMode()
{
    // Clear our own flags
    m_bRecTrainingMode = kCIDLib::False;
    m_bGotRecTrainingData = kCIDLib::False;

    // And then update the training mode field
    bStoreBoolFld(m_c4FldIdTrainingMode, m_bRecTrainingMode, kCIDLib::True);
}


tCIDLib::TVoid TGenTrigSDriver::ReleaseCommResource()
{
    try
    {
        if (m_bSocket)
        {
            if (m_psockTrig->bIsConnected())
                m_psockTrig->c4Shutdown();
            m_psockTrig->Close();
        }
         else
        {
            if (m_pcommTrig->bIsOpen())
                m_pcommTrig->Close();
        }
    }

    catch(TError& errToCatch)
    {
        LogError(errToCatch, tCQCKit::EVerboseLvls::Medium, CID_FILE, CID_LINE);
    }
}


tCIDLib::TVoid TGenTrigSDriver::TerminateImpl()
{
    // Nothing to do for now

    // Just call the base IR driver class last
    TParent::TerminateImpl();
}


