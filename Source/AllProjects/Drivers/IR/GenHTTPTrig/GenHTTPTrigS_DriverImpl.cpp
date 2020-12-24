//
// FILE NAME: GenHTTPTrigS_DriverImpl.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/24/2013
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
#include    "GenHTTPTrigS.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TGenHTTPTrigSDriver,TBaseIRSrvDriver)


// ---------------------------------------------------------------------------
//   CLASS: THostMonSDriver
//  PREFIX: drv
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  THostMonSDriver: Constructors and Destructor
// ---------------------------------------------------------------------------
TGenHTTPTrigSDriver::TGenHTTPTrigSDriver(const TCQCDriverObjCfg& cqcdcToLoad) :

    TBaseIRSrvDriver(cqcdcToLoad)
    , m_bGotRecTrainingData(kCIDLib::False)
    , m_bRecTrainingMode(kCIDLib::False)
    , m_c4FldIdInvoke(kCIDLib::c4MaxCard)
    , m_c4FldIdTrainingMode(kCIDLib::c4MaxCard)
    , m_mbufData(1024, 2048)
    , m_psocklTrig(0)
{
}

TGenHTTPTrigSDriver::~TGenHTTPTrigSDriver()
{
    //
    //  Clean up the socket listener. It should already have been done,
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
}


// ---------------------------------------------------------------------------
//  TGenHTTPTrigSDriver: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TGenHTTPTrigSDriver::bCheckRecTrainingData(TString& strKeyToFill)
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
//  Nothing to do here, since we accept transient connections. So we just
//  return true.
//
tCIDLib::TBoolean TGenHTTPTrigSDriver::bGetCommResource(TThread&)
{
    return kCIDLib::True;
}


tCIDLib::TBoolean TGenHTTPTrigSDriver::bRecTrainingMode()
{
    return m_bRecTrainingMode;
}


//
//  We don't require that we be connected for this one, so that it can be
//  used to reset us in some bad circumstances.
//
tCIDLib::TBoolean TGenHTTPTrigSDriver::bResetConnection()
{
    ExitRecTrainingMode();
    ClearEventQ();

    return kCIDLib::True;
}


tCIDLib::TCard4 TGenHTTPTrigSDriver::c4InvokeFldId() const
{
    return m_c4FldIdInvoke;
}


tCIDLib::TVoid TGenHTTPTrigSDriver::ClearRecTrainingData()
{
    // If we aren't online, then throw, else do it
    CheckOnline(CID_FILE, CID_LINE);
    m_bGotRecTrainingData = kCIDLib::False;
    m_strRecTrainData.Clear();
}


//
//  Our connections are transient, so all we care about here is that
//  we have our listener socket.
//
tCQCKit::ECommResults TGenHTTPTrigSDriver::eConnectToDevice(TThread&)
{
    // If we have an existing socket, clean it up
    if (m_psocklTrig)
    {
        try
        {
            delete m_psocklTrig;
            m_psocklTrig = nullptr;
        }

        catch(TError& errToCatch)
        {
            LogError(errToCatch, tCQCKit::EVerboseLvls::Medium, CID_FILE, CID_LINE);
            return tCQCKit::ECommResults::LostCommRes;
        }
    }

    //
    //  And now let's try to create the new one. We will accept a small
    //  backlog of connections so that we can have time to process current
    //  ones and then get to them.
    //
    try
    {
        m_psocklTrig = new TSocketListener
        (
            m_ippnListen, tCIDSock::ESockProtos::TCP, 8
        );
    }

    catch(TError& errToCatch)
    {
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
        {
            LogError(errToCatch, tCQCKit::EVerboseLvls::Medium, CID_FILE, CID_LINE);
            facGenHTTPTrigS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kHTTPTrErrs::errcInit_CantCreateListener
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );
        }
        return tCQCKit::ECommResults::LostCommRes;
    }

    return tCQCKit::ECommResults::Success;
}


tCQCKit::EDrvInitRes TGenHTTPTrigSDriver::eInitializeImpl()
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

        catch(TError& errToCatch)
        {
            LogError(errToCatch, tCQCKit::EVerboseLvls::Medium, CID_FILE, CID_LINE);
            return tCQCKit::EDrvInitRes::Failed;
        }
        m_psocklTrig = nullptr;
    }

    // Get the port we are to listen on
    const TCQCDriverObjCfg& cqcdcOurs = cqcdcThis();
    TString strPromptVal;
    if (cqcdcOurs.bFindPromptValue(L"PortNum", L"Value", strPromptVal))
        m_ippnListen = strPromptVal.c4Val(tCIDLib::ERadices::Dec);
    else
        m_ippnListen = 45821;

    // We use a fixed seaprator character
    SetKeySepChar(L'#');

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
    //  Look up the ids of our fields, for efficiency. We don't actually get
    //  any firmware info so we don't bother storing the firmware version
    //  field id.
    //
    m_c4FldIdInvoke = pflddFind(TFacCQCIR::strFldName_Invoke, kCIDLib::True)->c4Id();
    m_c4FldIdTrainingMode = pflddFind(TFacCQCIR::strFldName_TrainingState, kCIDLib::True)->c4Id();

    //
    //  Set the poll time as fast as possible so we will react quickly to
    //  incoming trigger strings.
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


tCQCKit::ECommResults TGenHTTPTrigSDriver::ePollDevice(TThread&)
{
    tCQCKit::ECommResults eRetVal = tCQCKit::ECommResults::Success;

    try
    {
        // Wait for an incoming connect
        TServerStreamSocket* psockTrig = m_psocklTrig->psockListenFor
        (
            kCIDLib::enctOneMilliSec * 100, m_ipepClient
        );

        // If we got one, wait for an HTTP request
        if (psockTrig)
        {
            TCIDSockStreamDataSrc* pcdsClient = new TCIDSockStreamDataSrc
            (
                psockTrig, tCIDLib::EAdoptOpts::Adopt
            );
            TCIDDataSrcJan janSrc
            (
                pcdsClient, tCIDLib::EAdoptOpts::Adopt, kCIDLib::True
            );

            //
            //  Get the client request. This guy doesn't throw. It will return an
            //  error to the client for any failure.
            //
            tCIDLib::TCard4         c4ContLen;
            tCIDLib::TCard4         c4ProtoVer;
            tCIDNet::ENetPReadRes   eRes;
            TString                 strContType;
            TString                 strType;
            TString                 strURL;

            eRes = m_httpcSrv.eGetClientMsg
            (
                *pcdsClient
                , TTime::enctNowPlusMSs(8000)
                , strType
                , m_colInHdrLines
                , strContType
                , strURL
                , c4ProtoVer
                , m_mbufData
                , c4ContLen
            );

            //
            //  If we didn't succeed, then the HTTP client object already sent
            //  back a failure error to the client, so we just return.
            //
            if (eRes == tCIDNet::ENetPReadRes::Success)
            {
                const TURL urlReq(strURL, tCIDSock::EQualified::Partial);

                if (m_bRecTrainingMode)
                {
                    m_strRecTrainData = urlReq.strPath();
                    m_bGotRecTrainingData = kCIDLib::True;
                }
                 else
                {
                    //
                    //  We have to rewrite the URL into a form that works for this
                    //  purpose. We put out the resource, then our fixed separator
                    //  (#), then each of the query parms as input parms.
                    //
                    if (bRewriteURL(urlReq, m_strCmdStr))
                        QueueRecEvent(m_strCmdStr);
                }

                // Send it back a trivial response
                TTextMBufOutStream strmTar(4192, 4192, new TUTF8Converter);
                strmTar.eNewLineType(tCIDLib::ENewLineTypes::CRLF);

                strmTar << L"HTTP/1.0 200 OK" << kCIDLib::NewLn
                        << THTTPClient::strHdr_ContType << L": text/html; charset=utf-8"
                        << kCIDLib::NewLn
                        << THTTPClient::strHdr_ContLen << L": 32"
                        << kCIDLib::NewLn
                        << THTTPClient::strHdr_Connection << L": Close"
                        << kCIDLib::DNewLn
                        << L"<html><body>Done</body></html>"
                        << kCIDLib::EndLn;

                // OK, let's send it back
                psockTrig->Send(strmTar.mbufData().pc1Data(), strmTar.c4CurSize());
            }
        }
    }

    catch(TError& errToCatch)
    {
        LogError(errToCatch, tCQCKit::EVerboseLvls::Medium);
        return tCQCKit::ECommResults::LostConnection;
    }

    return tCQCKit::ECommResults::Success;
}


tCIDLib::TVoid TGenHTTPTrigSDriver::EnterRecTrainingMode()
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


tCIDLib::TVoid TGenHTTPTrigSDriver::ExitRecTrainingMode()
{
    // Clear our own flags
    m_bRecTrainingMode = kCIDLib::False;
    m_bGotRecTrainingData = kCIDLib::False;

    // And then update the training mode field
    bStoreBoolFld(m_c4FldIdTrainingMode, m_bRecTrainingMode, kCIDLib::True);
}


//
//  We use transient client connections, so nothing to do.
//
tCIDLib::TVoid TGenHTTPTrigSDriver::ReleaseCommResource()
{
}


tCIDLib::TVoid TGenHTTPTrigSDriver::TerminateImpl()
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


// ---------------------------------------------------------------------------
//  TGenHTTPTrigSDriver: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Rewrites the URL in a form that we can queue up on the base class. We set
//  a fixed # sep character. We rewrite it like this:
//
//  resourcepath#p1 p2
//
//  I.e. the resource is the trainable part. If there are any parms, we add
//  the separator and any query parms in order.
//
tCIDLib::TBoolean
TGenHTTPTrigSDriver::bRewriteURL(const TURL& urlReq, TString& strToFill)
{
    strToFill = urlReq.strPath();

    const tCIDLib::TKVPList& colQPs = urlReq.colQParms();
    if (!colQPs.bIsEmpty())
    {
        strToFill.Append(L'#');

        const tCIDLib::TCard4 c4Count = colQPs.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            if (c4Index)
                strToFill.Append(kCIDLib::chSpace);
            strToFill.Append(colQPs[c4Index].strValue());
        }
    }

    // For debugging purposes, log the message
    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Low)
    {
        facGenHTTPTrigS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , TString(TStrCat(L"Rewritten URL is ", strToFill))
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
    }
    return kCIDLib::True;
}

