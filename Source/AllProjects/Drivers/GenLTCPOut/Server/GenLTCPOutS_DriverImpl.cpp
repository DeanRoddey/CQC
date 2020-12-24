//
// FILE NAME: GenLTCPOutS_DriverImpl.cpp
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
#include    "GenLTCPOutS.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TGenLTCPOutSDriver,TCQCServerBase)


// ---------------------------------------------------------------------------
//   CLASS: TGenLTCPOutSDriver
//  PREFIX: sdrv
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  THostMonSDriver: Constructors and Destructor
// ---------------------------------------------------------------------------
TGenLTCPOutSDriver::TGenLTCPOutSDriver(const TCQCDriverObjCfg& cqcdcToLoad) :

    TCQCServerBase(cqcdcToLoad)
    , m_c4FldId_SendBinMsg(kCIDLib::c4MaxCard)
    , m_c4FldId_SendTextMsg(kCIDLib::c4MaxCard)
    , m_eDecor(tGenLTCPOutS::EDecors::None)
    , m_mbufOut(256, 8192)
    , m_mbufTmp(256, 8192)
    , m_psocklOut(0)
    , m_psockOut(0)
    , m_ptcvtEncode(0)
{
}

TGenLTCPOutSDriver::~TGenLTCPOutSDriver()
{
    //
    //  Clean up the socket and listenter. It should already have been done,
    //  but don't take chances.
    //
    if (m_psocklOut)
    {
        try
        {
            delete m_psocklOut;
            m_psocklOut = 0;
        }

        catch(...) {}
    }

    if (m_psockOut)
    {
        try
        {
            delete m_psockOut;
            m_psockOut = 0;
        }

        catch(...) {}
    }

    // And the text encoder
    if (m_ptcvtEncode)
    {
        try
        {
            delete m_ptcvtEncode;
            m_ptcvtEncode = 0;
        }

        catch(...) {}
    }
}


// ---------------------------------------------------------------------------
//  TGenLTCPOutSDriver: Protected, inherited methods
// ---------------------------------------------------------------------------

//
//  Nothing to do here, since we accept connections. But just in case we
//  haven't been able to create our socket listener yet, try to do that.
//  We won't return success here until we've gotten our listener.
//
tCIDLib::TBoolean TGenLTCPOutSDriver::bGetCommResource(TThread&)
{
    if (!m_psocklOut)
    {
        try
        {
            m_psocklOut = new TSocketListener
            (
                m_ippnListen, tCIDSock::ESockProtos::TCP, 1
            );
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

                facGenLTCPOutS().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kGenLTOErrs::errcInit_CantCreateListener
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::AppStatus
                );
            }
        }
    }
    return (m_psocklOut != 0);
}


//
//  This one isn't relevant to us, so just return true
//
tCIDLib::TBoolean TGenLTCPOutSDriver::bWaitConfig(TThread&)
{
    return kCIDLib::True;
}



//
//  Watch for a connection from a client. If we've gotten one, then let
//  create the socket and indicate we are connected.
//
tCQCKit::ECommResults TGenLTCPOutSDriver::eConnectToDevice(TThread&)
{
    // We should absolutely have the listener here, but make sure
    if (!m_psocklOut)
        return tCQCKit::ECommResults::LostCommRes;

    // Listen for a short while for a connection
    TIPEndPoint ipepClient;
    try
    {
        m_psockOut = m_psocklOut->psockListenFor
        (
            kCIDLib::enctOneMilliSec * 100, ipepClient
        );
    }

    catch(const TError& errToCatch)
    {
        LogError(errToCatch, tCQCKit::EVerboseLvls::Medium);
        return tCQCKit::ECommResults::LostConnection;
    }

    // If we got a socket, then log it and indicate connected
    if (m_psockOut)
    {
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
        {
            facGenLTCPOutS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kGenLTOMsgs::midStatus_GotConn
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
                , ipepClient
            );
        }
        return tCQCKit::ECommResults::Success;
    }
    return tCQCKit::ECommResults::LostConnection;
}


// Handle driver init
tCQCKit::EDrvInitRes TGenLTCPOutSDriver::eInitializeImpl()
{
    //
    //  Just in case the listener didn't get cleaned up, do so since we
    //  might have different config.
    //
    if (m_psocklOut)
    {
        try
        {
            delete m_psocklOut;
        }

        catch(...)
        {
        }
        m_psocklOut = 0;
    }

    // Get the port we are to listen on
    const TCQCDriverObjCfg& cqcdcOurs = cqcdcThis();

    TString strPromptVal;
    if (cqcdcOurs.bFindPromptValue(L"PortNum", L"Value", strPromptVal))
        m_ippnListen = strPromptVal.c4Val(tCIDLib::ERadices::Dec);
    else
        m_ippnListen = 43832;

    //
    //  Get the text encoding and create the encoder for that. If not there
    //  assume ASCII worst case.
    //
    if (!cqcdcOurs.bFindPromptValue(L"TextEncoding", L"Selected", strPromptVal))
        strPromptVal = L"US-ASCII";

    // Clean up any previous one and create the new one
    try
    {
        if (m_ptcvtEncode)
        {
            delete m_ptcvtEncode;
            m_ptcvtEncode = 0;
        }
        m_ptcvtEncode = facCIDEncode().ptcvtMake(strPromptVal);
    }

    catch(TError& errToCatch)
    {
        if (facGenLTCPOutS().bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
    }

    // Get the text decoration prompt and set up our enum
    m_eDecor = tGenLTCPOutS::EDecors::None;
    if (cqcdcOurs.bFindPromptValue(L"TextDecor", L"Selected", strPromptVal))
    {
        if (strPromptVal == L"CR")
            m_eDecor = tGenLTCPOutS::EDecors::CR;
        else if (strPromptVal == L"LF")
            m_eDecor = tGenLTCPOutS::EDecors::LF;
        else if (strPromptVal == L"CRLF")
            m_eDecor = tGenLTCPOutS::EDecors::CRLF;
        else if (strPromptVal == L"STX/ETX")
            m_eDecor = tGenLTCPOutS::EDecors::STXETX;
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
        L"SendBinMsg"
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Write
    );
    flddCmd.bAlwaysWrite(kCIDLib::True);
    colFlds.objAdd(flddCmd);

    flddCmd.Set
    (
        L"SendTextMsg"
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Write
    );
    flddCmd.bAlwaysWrite(kCIDLib::True);
    colFlds.objAdd(flddCmd);

    // Tell our base class about our fields
    SetFields(colFlds);

    //
    //  Look up the ids of our fields, for efficiency. WE don't actually get
    //  any firmware info so we don't bother storing the firmware version
    //  field id.
    //
    m_c4FldId_SendBinMsg = pflddFind(L"SendBinMsg", kCIDLib::True)->c4Id();
    m_c4FldId_SendTextMsg = pflddFind(L"SendTextMsg", kCIDLib::True)->c4Id();

    //
    //  No particular need to poll quickly since we are outgoing. But we do
    //  a quick read on it in each poll to see if the connection is lost. So
    //  we want to check it fairly often.
    //
    SetPollTimes(2000, 5000);

    //
    //  No config in our case, so we return ready to start trying to connect
    //
    return tCQCKit::EDrvInitRes::WaitCommRes;
}


tCQCKit::ECommResults TGenLTCPOutSDriver::ePollDevice(TThread&)
{
    tCQCKit::ECommResults eRetVal = tCQCKit::ECommResults::Success;

    try
    {
        // Just do a simple read with timeout, to watch for loss of connection
        tCIDLib::TCard1 c1Buf;
        m_psockOut->c4ReceiveRawTOMS(&c1Buf, 10, 1, tCIDLib::EAllData::OkIfNotAll);
    }

    catch(TError& errToCatch)
    {
        //
        //  See if the socket is disconnected. If so, then other side has
        //  dropped the connection and we need to go into wait connect
        //  mode.
        //
        if (!m_psockOut->bIsConnected())
        {
            eRetVal = tCQCKit::ECommResults::LostConnection;

            if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Low)
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);

                facGenLTCPOutS().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kGenLTOMsgs::midStatus_LostConn
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::AppStatus
                );
            }
        }
    }

    return eRetVal;
}


// A string field changed
tCQCKit::ECommResults
TGenLTCPOutSDriver::eStringFldValChanged(const  TString&
                                        , const tCIDLib::TCard4 c4FldId
                                        , const TString&        strNewValue)
{
    if (c4FldId == m_c4FldId_SendBinMsg)
    {
        // Call the helper method with the passed value
        if (!bSendBinMsg(strNewValue))
            return tCQCKit::ECommResults::ValueRejected;
    }
     else if (c4FldId == m_c4FldId_SendTextMsg)
    {
        // Call the helper method with the passed value
        if (!bSendTextMsg(strNewValue))
            return tCQCKit::ECommResults::ValueRejected;
    }
     else
    {
        IncUnknownWriteCounter();
        return tCQCKit::ECommResults::FieldNotFound;
    }

    return tCQCKit::ECommResults::Success;
}


//
//  Force the socket down and clean it up. Generally the clients disconnect
//  from us in this driver, but we need to clean up on shutdown or unload.
//
tCIDLib::TVoid TGenLTCPOutSDriver::ReleaseCommResource()
{
    if (m_psockOut)
    {
        try
        {
            if (m_psockOut->bIsConnected())
                m_psockOut->c4Shutdown();
            m_psockOut->Close();
        }

        catch(TError& errToCatch)
        {
            LogMsg(errToCatch, tCQCKit::EVerboseLvls::Medium, CID_FILE, CID_LINE);
        }

        try
        {
            delete m_psockOut;
        }

        catch(TError& errToCatch)
        {
            LogMsg(errToCatch, tCQCKit::EVerboseLvls::Medium, CID_FILE, CID_LINE);
        }

        m_psockOut = 0;
    }
}


tCIDLib::TVoid TGenLTCPOutSDriver::TerminateImpl()
{
    // Clean up the socket listener
    if (m_psocklOut)
    {
        try
        {
            m_psocklOut->Cleanup();
            delete m_psocklOut;
            m_psocklOut = 0;
        }

        catch(TError& errToCatch)
        {
            LogMsg(errToCatch, tCQCKit::EVerboseLvls::Medium, CID_FILE, CID_LINE);
        }
    }
}



// ---------------------------------------------------------------------------
//  TGenLTCPOutSDriver: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Called process a binary message. The value sent is assumed to a space
//  separated list of tokens, each of which is a hex encoded byte. So each one
//  should be something between 00 and FF.
//
//  These values will be pulled out, converted to binary, and put
//  into a memory buffer in the same order, and sent to the port.
//

tCIDLib::TBoolean TGenLTCPOutSDriver::bSendBinMsg(const TString& strToSend)
{
    TStringTokenizer stokMsg(&strToSend, L" \t");

    tCIDLib::TCard4 c4Count = 0;
    try
    {
        while (stokMsg.bGetNextToken(m_strToken))
        {
            tCIDLib::TCard1 c1Cur;

            if (!m_strToken.bToCard1(c1Cur, tCIDLib::ERadices::Hex))
            {
                // It's bad, log and return failure
                if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Low)
                {
                    facGenLTCPOutS().LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , kGenLTOErrs::errcMsg_BadBinToken
                        , tCIDLib::ESeverities::Failed
                        , tCIDLib::EErrClasses::Format
                        , TCardinal(c4Count + 1)
                    );
                }
                return kCIDLib::False;
            }

            // Add it to the buffer
            m_mbufOut.PutCard1(c1Cur, c4Count);
            c4Count++;
        }

        // We have to have gotten at least one 1 byte
        if (!c4Count)
        {
            if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Low)
            {
                facGenLTCPOutS().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kGenLTOErrs::errcMsg_NoMsgData
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Format
                );
            }
            return kCIDLib::False;
        }
    }

    catch(TError& errToCatch)
    {
        LogMsg(errToCatch, tCQCKit::EVerboseLvls::Medium, CID_FILE, CID_LINE);
        return kCIDLib::False;
    }

    // Try to send it
    m_psockOut->Send(m_mbufOut, c4Count);
    return kCIDLib::True;
}


tCIDLib::TBoolean TGenLTCPOutSDriver::bSendTextMsg(const TString& strToSend)
{
    //
    //  We need to transcode the text to a buffer. We have the special case
    //  of an STX/ETX decoration, which means we have to put in a byte at
    //  the start of the buffer. In that case, we have to use a temp buffer
    //  and copy it over.
    //
    tCIDLib::TCard4 c4Count = 0;
    if (m_eDecor == tGenLTCPOutS::EDecors::STXETX)
    {
        // Put in the STX at zero
        m_mbufOut.PutCard1(2, c4Count++);

        // Convert to a temp
        tCIDLib::TCard4 c4Out;
        m_ptcvtEncode->c4ConvertTo(strToSend, m_mbufTmp, c4Out);

        // If no bytes, do nothing
        if (!c4Out)
        {
            if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Low)
            {
                facGenLTCPOutS().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kGenLTOErrs::errcMsg_NoMsgData
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Format
                );
            }
            return kCIDLib::False;
        }

        // Copy the temp in after the STX
        m_mbufOut.CopyIn(m_mbufTmp, c4Out, 1);
        c4Count += c4Out;
    }
     else
    {
        m_ptcvtEncode->c4ConvertTo(strToSend, m_mbufOut, c4Count);

        // If no bytes, do nothing
        if (!c4Count)
        {
            if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Low)
            {
                facGenLTCPOutS().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kGenLTOErrs::errcMsg_NoMsgData
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Format
                );
            }
            return kCIDLib::False;
        }
    }

    // And now put on any terminating decoration
    switch(m_eDecor)
    {
        case tGenLTCPOutS::EDecors::None :
            // Nothing to do
            break;

        case tGenLTCPOutS::EDecors::CR :
            m_mbufOut.PutCard1(0xD, c4Count++);
            break;

        case tGenLTCPOutS::EDecors::LF :
            m_mbufOut.PutCard1(0xA, c4Count++);
            break;

        case tGenLTCPOutS::EDecors::CRLF :
            m_mbufOut.PutCard1(0xD, c4Count++);
            m_mbufOut.PutCard1(0xA, c4Count++);
            break;

        case tGenLTCPOutS::EDecors::STXETX :
            m_mbufOut.PutCard1(3, c4Count++);
            break;

        default :
            // An unknown decoration
            facGenLTCPOutS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kGenLTOErrs::errcMsg_UnknownTextDecor
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Internal
                , TCardinal(tCIDLib::c4EnumOrd(m_eDecor))
            );
            return kCIDLib::False;
    };

    // And now we can send the msg
    m_psockOut->Send(m_mbufOut, c4Count);
    return kCIDLib::True;
}

