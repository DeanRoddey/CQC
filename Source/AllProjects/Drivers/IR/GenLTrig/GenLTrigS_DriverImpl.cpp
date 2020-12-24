//
// FILE NAME: GenLTrigS_DriverImpl.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/15/2013
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
//  This file implements our connection type specific driver stuff, the rest
//  being in the base clas.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "GenLTrigS.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TGenLTrigSDriver,TBaseIRSrvDriver)


// ---------------------------------------------------------------------------
//   CLASS: THostMonSDriver
//  PREFIX: drv
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  THostMonSDriver: Constructors and Destructor
// ---------------------------------------------------------------------------
TGenLTrigSDriver::TGenLTrigSDriver(const TCQCDriverObjCfg& cqcdcToLoad) :

    TBaseIRSrvDriver(cqcdcToLoad)
    , m_bGotRecTrainingData(kCIDLib::False)
    , m_bRecTrainingMode(kCIDLib::False)
    , m_c4FldIdInvoke(kCIDLib::c4MaxCard)
    , m_c4FldIdTrainingMode(kCIDLib::c4MaxCard)
    , m_eTermType(tGenLTrigS::ETermTypes::CRLF)
    , m_psocklTrig(nullptr)
    , m_psockTrig(nullptr)
{
}

TGenLTrigSDriver::~TGenLTrigSDriver()
{
    //
    //  Clean up the socket and listenter. It should already have been done,
    //  but don't take chances.
    //
    if (m_psocklTrig)
    {
        try
        {
            delete m_psocklTrig;
            m_psocklTrig = nullptr;
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
//  TGenLTrigSDriver: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TGenLTrigSDriver::bCheckRecTrainingData(TString& strKeyToFill)
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


//
//  Nothing to do here, since we accept connections. But just in case we
//  haven't been able to create our socket listener yet, try to do that.
//  We won't return success here until we've gotten our listener.
//
tCIDLib::TBoolean TGenLTrigSDriver::bGetCommResource(TThread&)
{
    if (!m_psocklTrig)
    {
        try
        {
            m_psocklTrig = new TSocketListener
            (
                m_ippnListen, tCIDSock::ESockProtos::TCP, 1
            );
        }

        catch(TError& errToCatch)
        {
            if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
            {
                LogMsg(errToCatch, tCQCKit::EVerboseLvls::Medium, CID_FILE, CID_LINE);
                facGenLTrigS().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kGenLTErrs::errcInit_CantCreateListener
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::AppStatus
                );
            }
        }
    }
    return (m_psocklTrig != nullptr);
}


tCIDLib::TBoolean TGenLTrigSDriver::bRecTrainingMode()
{
    return m_bRecTrainingMode;
}


//
//  We don't require that we be connected for this one, so that it can be
//  used to reset us in some bad circumstances.
//
tCIDLib::TBoolean TGenLTrigSDriver::bResetConnection()
{
    ExitRecTrainingMode();
    ClearEventQ();

    return kCIDLib::True;
}


tCIDLib::TCard4 TGenLTrigSDriver::c4InvokeFldId() const
{
    return m_c4FldIdInvoke;
}


tCIDLib::TVoid TGenLTrigSDriver::ClearRecTrainingData()
{
    // If we aren't online, then throw, else do it
    CheckOnline(CID_FILE, CID_LINE);
    m_bGotRecTrainingData = kCIDLib::False;
    m_strRecTrainData.Clear();
}


//
//  Watch for a connection from a client. If we've gotten one, then let
//  create the socket and indicate we are connected.
//
tCQCKit::ECommResults TGenLTrigSDriver::eConnectToDevice(TThread&)
{
    // We should absolutely have the listener here, but make sure
    if (!m_psocklTrig)
        return tCQCKit::ECommResults::LostCommRes;

    //
    //  Make sure we get out of training mode if we got stuck there when the
    //  connection was dropped.
    //
    m_bRecTrainingMode = kCIDLib::False;

    // Listen for a short while for a connection
    TIPEndPoint ipepClient;
    try
    {
        m_psockTrig = m_psocklTrig->psockListenFor
        (
            kCIDLib::enctOneMilliSec * 100, ipepClient
        );
    }

    catch(TError& errToCatch)
    {
        LogError(errToCatch, tCQCKit::EVerboseLvls::Medium, CID_FILE, CID_LINE);
        return tCQCKit::ECommResults::LostConnection;
    }

    // If we got a socket, then log it and indicate connected
    if (m_psockTrig)
    {
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
        {
            facGenLTrigS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kGenLTMsgs::midStatus_GotConn
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
                , ipepClient
            );
        }
        return tCQCKit::ECommResults::Success;
    }
    return tCQCKit::ECommResults::LostConnection;
}


tCQCKit::EDrvInitRes TGenLTrigSDriver::eInitializeImpl()
{
    // Call our parent IR base drivers
    TParent::eInitializeImpl();

    //
    //  Just in case the listener didn't get cleaned up, do so since we
    //  might have different config.
    //
    if (m_psocklTrig)
    {
        try
        {
            delete m_psocklTrig;
        }

        catch(...)
        {
        }
        m_psocklTrig = nullptr;
    }

    // Get the port we are to listen on
    const TCQCDriverObjCfg& cqcdcOurs = cqcdcThis();
    TString strPromptVal;
    if (cqcdcOurs.bFindPromptValue(L"PortNum", L"Value", strPromptVal))
        m_ippnListen = strPromptVal.c4Val(tCIDLib::ERadices::Dec);
    else
        m_ippnListen = 43831;

    // Get the termination type. If not set, we go with CR/LF
    m_eTermType = tGenLTrigS::ETermTypes::CRLF;
    if (cqcdcOurs.bFindPromptValue(L"TermType", L"Selected", strPromptVal))
    {
        if (strPromptVal == L"CR")
            m_eTermType = tGenLTrigS::ETermTypes::CR;
        else if (strPromptVal == L"LF")
            m_eTermType = tGenLTrigS::ETermTypes::LF;
        else if (strPromptVal == L"CR/LF")
            m_eTermType = tGenLTrigS::ETermTypes::CRLF;
        else
        {
            if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
            {
                facGenLTrigS().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kGenLTErrs::errcInit_BadTermPrompt
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::AppStatus
                    , strPromptVal
                );
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

    //
    //  Look up the ids of our fields, for efficiency. WE don't actually get
    //  any firmware info so we don't bother storing the firmware version
    //  field id.
    //
    m_c4FldIdInvoke = pflddFind(TFacCQCIR::strFldName_Invoke, kCIDLib::True)->c4Id();
    m_c4FldIdTrainingMode = pflddFind(TFacCQCIR::strFldName_TrainingState, kCIDLib::True)->c4Id();

    //
    //  Set the poll time as fast as possible so we will react quickly to
    //  incoming trigger strings. Set the reconnect pretty fast as well since
    //  we have to listen for connections.
    //
    SetPollTimes(50, 1000);

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


tCQCKit::ECommResults TGenLTrigSDriver::ePollDevice(TThread&)
{
    tCQCKit::ECommResults eRetVal = tCQCKit::ECommResults::Success;

    // We have to set the line term chars based on what we were told
    tCIDLib::TCard1 c1Term1;
    tCIDLib::TCard1 c1Term2;
    switch(m_eTermType)
    {
        case tGenLTrigS::ETermTypes::CR :
            c1Term1 = 13;
            c1Term2 = 0;
            break;

        case tGenLTrigS::ETermTypes::LF :
            c1Term1 = 10;
            c1Term2 = 0;
            break;

        case tGenLTrigS::ETermTypes::CRLF :
            c1Term1 = 13;
            c1Term2 = 10;
            break;

        default :
            if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
            {
                facGenLTrigS().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kGenLTErrs::errcInit_BadTermType
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::AppStatus
                    , TCardinal(tCIDLib::c4EnumOrd(m_eTermType))
                );
            }
            c1Term1 = 13;
            c1Term2 = 10;
            break;
    };

    try
    {
        while (bGetASCIITermMsg(*m_psockTrig, 10, c1Term1, c1Term2, m_strPollStr))
        {
            //
            //  Ok, we have a key. If we are in training mode, then just store
            //  it in the m_strTrainVal member. Else, we need to queue it on
            //  the action queue.
            //
            if (m_bRecTrainingMode)
            {
                //
                //  If we have a seperator character set, split it on that, so
                //  that we only store the key part as the training data.
                //
                m_strRecTrainData = m_strPollStr;
                if (chSepChar())
                    m_strRecTrainData.bCapAtChar(chSepChar());
                m_bGotRecTrainingData = kCIDLib::True;
            }
             else
            {
                QueueRecEvent(m_strPollStr);
            }
        }
    }

    catch(TError& errToCatch)
    {
        //
        //  See if the socket is disconnected. If so, then other side has
        //  dropped the connection and we need to go into wait connect
        //  mode.
        //
        if (!m_psockTrig->bIsConnected())
        {
            eRetVal = tCQCKit::ECommResults::LostConnection;

            if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Low)
            {
                LogMsg(errToCatch, tCQCKit::EVerboseLvls::Low, CID_FILE, CID_LINE);
                facGenLTrigS().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kGenLTMsgs::midStatus_LostConn
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::AppStatus
                );
            }
        }
    }

    return eRetVal;
}


tCIDLib::TVoid TGenLTrigSDriver::EnterRecTrainingMode()
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


tCIDLib::TVoid TGenLTrigSDriver::ExitRecTrainingMode()
{
    // Clear our own flags
    m_bRecTrainingMode = kCIDLib::False;
    m_bGotRecTrainingData = kCIDLib::False;

    // And then update the training mode field
    bStoreBoolFld(m_c4FldIdTrainingMode, m_bRecTrainingMode, kCIDLib::True);
}


//
//  Force the socket down and clean it up. Generally the clients disconnect
//  from us in this driver, but we need to clean up on shutdown or unload.
//
tCIDLib::TVoid TGenLTrigSDriver::ReleaseCommResource()
{
    if (m_psockTrig)
    {
        try
        {
            if (m_psockTrig->bIsConnected())
                m_psockTrig->c4Shutdown();
            m_psockTrig->Close();
        }

        catch(TError& errToCatch)
        {
            LogError(errToCatch, tCQCKit::EVerboseLvls::Medium, CID_FILE, CID_LINE);
        }

        try
        {
            delete m_psockTrig;
        }

        catch(TError& errToCatch)
        {
            LogError(errToCatch, tCQCKit::EVerboseLvls::Medium, CID_FILE, CID_LINE);
        }

        m_psockTrig = nullptr;
    }
}


tCIDLib::TVoid TGenLTrigSDriver::TerminateImpl()
{
    // Clean up the socket listener
    if (m_psocklTrig)
    {
        try
        {
            m_psocklTrig->Cleanup();
            delete m_psocklTrig;
            m_psocklTrig = nullptr;
        }

        catch(TError& errToCatch)
        {
            LogError(errToCatch, tCQCKit::EVerboseLvls::Medium, CID_FILE, CID_LINE);
        }
    }

    // Just call the base IR driver class last
    TParent::TerminateImpl();
}


