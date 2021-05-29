//
// FILE NAME: JAPwrSer_Port.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/12/2013
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
//  This file implements the TJAPwrCommPort class.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


// ---------------------------------------------------------------------------
//  Facility specific includes
// ---------------------------------------------------------------------------
#include    "JAPwrSer_.hpp"


// ---------------------------------------------------------------------------
//  Magic RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TJAPwrCommPort,TCommPortBase)


// ---------------------------------------------------------------------------
//  Local types and constants
// ---------------------------------------------------------------------------
namespace
{
    namespace CQCRPortClient_Port
    {
        constexpr tCIDLib::TCard4   c4CacheSz = 128;
    }
}



// ---------------------------------------------------------------------------
//  CLASS: TJAPwrCommPort
// PREFIX: comm
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TJAPwrCommPort: Public, static method
// ---------------------------------------------------------------------------

// See if the baud rate is one of the ones supported by the device
tCIDLib::TBoolean
TJAPwrCommPort::bCheckBaud(const tCIDLib::TCard4 c4ToCheck)
{
    tCIDLib::TBoolean bRet = kCIDLib::False;
    switch(c4ToCheck)
    {
        case 1200 :
        case 2400 :
        case 4800 :
        case 9600 :
        case 19200 :
        case 38400 :
        case 57600 :
            // These are all ok
            bRet = kCIDLib::True;
            break;

        default :
            break;
    };
    return bRet;
}



// ---------------------------------------------------------------------------
//  TJAPwrCommPort: Constructors and Destructor
// ---------------------------------------------------------------------------
TJAPwrCommPort::TJAPwrCommPort( const   TString&            strPath
                                , const TString&            strName
                                , const TString&            strAddr
                                , const tCIDLib::TCard4     c4PortNum) :

    TCommPortBase(strPath)
    , m_c4PortNum(c4PortNum)
    , m_fcolCache(CQCRPortClient_Port::c4CacheSz)
    , m_ipepTar(strAddr, kJAPwrSer::ippnBasePort, tCIDSock::EAddrTypes::Unspec)
    , m_mbufIO(128, 8192)
    , m_psockPort(0)
    , m_strAddr(strAddr)
    , m_strName(strName)
{
}

TJAPwrCommPort::~TJAPwrCommPort()
{
    try
    {
        Close();
    }

    catch(...)
    {
    }
}


// ---------------------------------------------------------------------------
//  TJAPwrCommPort: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TJAPwrCommPort::bIsOpen() const
{
    // If the socket has been created, say we are open
    return (m_psockPort != 0);
}


// We can't really support this, so just say no
tCIDLib::TBoolean TJAPwrCommPort::bQueryCTSOn() const
{
    CheckOpened();
    return kCIDLib::False;
}


// We can't really support this, so just say no
tCIDLib::TBoolean TJAPwrCommPort::bQueryDSROn() const
{
    CheckOpened();
    return kCIDLib::False;
}


tCIDLib::TCard4
TJAPwrCommPort::c4ReadRawBufMS(         tCIDLib::TVoid* const   pToFill
                                , const tCIDLib::TCard4         c4MaxBytes
                                , const tCIDLib::TCard4         c4Wait)
{
    // Calc the time for the indicted millis and call the other version
    const tCIDLib::TEncodedTime enctWait(kCIDLib::enctOneMilliSec * c4Wait);
    return c4ReadRawBuf(pToFill, c4MaxBytes, enctWait);
}


tCIDLib::TCard4
TJAPwrCommPort::c4ReadRawBuf(       tCIDLib::TVoid* const   pToFill
                            , const tCIDLib::TCard4         c4MaxBytes
                            , const tCIDLib::TEncodedTime   enctWait)
{
    // If it's not been opened yet, then that's an error
    CheckOpened();

    tCIDLib::TCard1* pc1Tar = static_cast<tCIDLib::TCard1*>(pToFill);

    // If we have any bytes in the cache, then take those first
    tCIDLib::TCard4 c4FromCache = 0;
    while (c4FromCache < c4MaxBytes)
    {
        if (m_fcolCache.bIsEmpty())
            break;
        *pc1Tar++ = m_fcolCache.tGetFromBack();
        c4FromCache++;
    }

    // If we got the whole request, then we are done
    if (c4FromCache == c4MaxBytes)
        return c4MaxBytes;

    tCIDLib::TCard4 c4RetVal = 0;
    try
    {
        //
        //  If not connected, try to connect. We might have lost the
        //  connection at some point.
        //
        if (!m_psockPort->bIsConnected())
            Reconnect();

        //
        //  Do another read for the remaining bytes. We reduce the byte count
        //  by the number we got from the cache. We tell it to wait for the
        //  full wait time, so we either get all the bytes requested or it
        //  comes back, since we tell it's ok if we don't read it all.
        //  We read directly into his buffer.
        //
        tCIDLib::TCard4 c4Read = m_psockPort->c4ReceiveRawTO
        (
            pc1Tar
            , enctWait
            , c4MaxBytes - c4FromCache
            , tCIDLib::EAllData::OkIfNotAll
        );

        // The return value is the bytes we read plus those from the cache
        c4RetVal = c4Read + c4FromCache;

        //
        //  If we got any bytes, and the caller is asking for bytes in small
        //  numbers, like 16 or less, then do another read and see if we can
        //  load up the cache again. We do it with a trivial timeout, to just
        //  get bytes that are already present. If he's asking for big chunks,
        //  then we will just degrade performance by trying to cache.
        //
        if (c4Read && (c4MaxBytes <= CQCRPortClient_Port::c4CacheSz))
        {
            c4Read = m_psockPort->c4ReceiveMBufTOMS
            (
                m_mbufIO
                , 5
                , CQCRPortClient_Port::c4CacheSz
                , tCIDLib::EAllData::OkIfNotAll
            );

            // If we got any, load up the cache
            const tCIDLib::TCard1* pc1Src = m_mbufIO.pc1Data();
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Read; c4Index++)
                m_fcolCache.AddAtFront(*pc1Src++);
        }
    }

    catch(const TError& errToCatch)
    {
        // If we are no longer connected, close down
        if (!m_psockPort->bIsConnected())
            DropConn();

        // Convert to a serial port error
        facCIDComm().ThrowKrnlErr
        (
            CID_FILE
            , CID_LINE
            , kCommErrs::errcPort_ReadData
            , TKrnlError::kerrLast()
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::LostConnection
            , strPath()
            , errToCatch.strErrText()
        );
    }
    return c4RetVal;
}


tCIDLib::TCard4
TJAPwrCommPort::c4WriteRawBufMS(const   tCIDLib::TVoid* const   pToWrite
                                , const tCIDLib::TCard4         c4Bytes
                                , const tCIDLib::TCard4         c4Wait)
{
    // Make sure we've been opened
    CheckOpened();

    // If no bytes, don't bother since this is a high overhead operation
    if (!c4Bytes)
        return 0;

    // Just do a throttled send. This will either send all bytes or throw
    try
    {
        //
        //  If not connected, try to connect. We might have lost the
        //  connection at some point.
        //
        if (!m_psockPort->bIsConnected())
            Reconnect();

        //
        //  Set up a wait time based on the incoming millis. We adjust this
        //  slightly if pretty low, since we aren't a real serial port and
        //  might have slightly longer delays.
        //
        tCIDLib::TEncodedTime enctWait;
        if (c4Wait < 500)
            enctWait = kCIDLib::enctOneMilliSec * 500;
        else
            enctWait = kCIDLib::enctOneMilliSec * c4Wait;

        m_psockPort->SendThrottled(pToWrite, enctWait , c4Bytes);
    }

    catch(const TError& errToCatch)
    {
        // If we've lost connection, clean up
        if (!m_psockPort->bIsConnected())
            DropConn();

        // Convert to a serial port error
        facCIDComm().ThrowKrnlErr
        (
            CID_FILE
            , CID_LINE
            , kCommErrs::errcPort_WriteData
            , TKrnlError::kerrLast()
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::LostConnection
            , strPath()
            , errToCatch.strErrText()
        );
    }

    return c4Bytes;
}


//
//  Close us down. We flush the read cache and disconnect and close the
//  socket and delete it and zero it. This means that Open has to be called
//  again.
//
tCIDLib::TVoid TJAPwrCommPort::Close()
{
    m_fcolCache.RemoveAll();
    if (m_psockPort)
    {
        // Drop the socket connection
        DropConn();

        // Clean up the socket
        delete m_psockPort;
        m_psockPort = 0;
    }
}


tCIDLib::TVoid TJAPwrCommPort::Cycle(const  tCIDComm::EOpenFlags eFlags
                                    , const tCIDLib::TCard4      c4ReadBufSz
                                    , const tCIDLib::TCard4      c4WriteBufSz)
{
    // Just flush the cache
    m_fcolCache.RemoveAll();
}


// We can't really do this
tCIDComm::EFlags TJAPwrCommPort::eFlags() const
{
    CheckOpened();
    return tCIDComm::EFlags::None;
}



tCIDLib::TVoid TJAPwrCommPort::FlushWriteBuffer()
{
    // Nothing to do in this case
    CheckOpened();
}


tCIDLib::TVoid TJAPwrCommPort::Open(const   tCIDComm::EOpenFlags eFlags
                                    , const tCIDLib::TCard4      c4ReadBufSz
                                    , const tCIDLib::TCard4      c4WriteBufSz)
{
    // Make sure not already open
    if (m_psockPort)
    {
        facCIDComm().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCommErrs::errcPort_Open
            , TString(L"Port is already open")
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::CantDo
            , strPath()
        );
    }

    //
    //  Try to get our connection to the device. The reconnect call will do
    //  what we need.
    //
    try
    {
        Reconnect();
    }

    catch(const TError& errToCatch)
    {
        // Clean it back up if we got it created
        Close();

        // Turn it into a standard port open error
        facCIDComm().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCommErrs::errcPort_Open
            , errToCatch.strErrText()
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::CantDo
            , strPath()
        );
    }

    // Flush the read cache
    m_fcolCache.RemoveAll();
}


tCIDLib::TVoid TJAPwrCommPort::PurgeReadData()
{
    // Flush the read cache
    m_fcolCache.RemoveAll();
}


tCIDLib::TVoid TJAPwrCommPort::PurgeWriteData()
{
    // Can't really do anything here
}


tCIDLib::TVoid TJAPwrCommPort::QueryCfg(TCommPortCfg&) const
{
    // Nothing we can really do
}


tCIDLib::TVoid
TJAPwrCommPort::QueryLineStates(tCIDLib::TBoolean&      bCTSOn
                                , tCIDLib::TBoolean&    bDSROn
                                , tCIDLib::TBoolean&    bRingOn
                                , tCIDLib::TBoolean&    bRLSDOn) const
{
    CheckOpened();

    // Can't support these, so always just say they are off
    bCTSOn = kCIDLib::False;
    bDSROn = kCIDLib::False;
    bRingOn = kCIDLib::False;
    bRLSDOn = kCIDLib::False;
}


tCIDLib::TVoid
TJAPwrCommPort::QueryXChars(tCIDLib::TSCh& schOn, tCIDLib::TSCh& schOff) const
{
    CheckOpened();

    // Don't support this. Always return nulls
    schOn = tCIDLib::TSCh(0);
    schOff = tCIDLib::TSCh(0);
}


tCIDLib::TVoid TJAPwrCommPort::Reset()
{
    // Flush the read cache
    m_fcolCache.RemoveAll();
}


tCIDLib::TVoid TJAPwrCommPort::SetBaudRate(const tCIDLib::TCard4 c4ToSet)
{
    CheckOpened();

    //
    //  Make sure it's valid for the device. If not, reject it. Not much else
    //  we can do. We have to assume the correct baud is set via the JAP's interface
    //
    if (!bCheckBaud(c4ToSet))
    {
        facCIDComm().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCommErrs::errcPort_SetBaud
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::CantDo
            , TCardinal(c4ToSet)
            , strPath()
        );
    }
}


tCIDLib::TVoid TJAPwrCommPort::SetBreak(const tCIDLib::TBoolean bState)
{
    CheckOpened();
}


tCIDLib::TVoid TJAPwrCommPort::SetCfg(const TCommPortCfg&)
{
    // Nothing we can do. Assume it's been set up correctly via the device itself
}


tCIDLib::TVoid TJAPwrCommPort::SetDataBits(const tCIDComm::EDataBits eToSet)
{
    //
    //  We can only support 8 bit, and it's always set. If that's what we are
    //  told to do, then do nothing. Else reject it.
    //
    if (eToSet != tCIDComm::EDataBits::Eight)
    {
        facCIDComm().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCommErrs::errcPort_SetDataBits
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::CantDo
            , strPath()
        );
    }
}


tCIDLib::TVoid TJAPwrCommPort::SetDTR(const tCIDComm::EPortDTR)
{
    CheckOpened();
}


tCIDLib::TVoid TJAPwrCommPort::SetParity(const tCIDComm::EParities)
{
    CheckOpened();
}


tCIDLib::TVoid TJAPwrCommPort::SetRTS(const tCIDComm::EPortRTS)
{
    CheckOpened();
}


tCIDLib::TVoid TJAPwrCommPort::SetStopBits(const tCIDComm::EStopBits eToSet)
{
    CheckOpened();

    // If not one stop bit, throw since that's the only thing we support
    if (eToSet != tCIDComm::EStopBits::One)
    {
        facCIDComm().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCommErrs::errcPort_SetStopBits
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::CantDo
            , strPath()
        );
    }
}


tCIDLib::TVoid TJAPwrCommPort::SetEOFChar(const tCIDLib::TSCh schToSet)
{
    CheckOpened();
}


tCIDLib::TVoid
TJAPwrCommPort::SetFlags(const tCIDComm::EFlags, const tCIDComm::EFlags)
{
    CheckOpened();
}


// ---------------------------------------------------------------------------
//  TJAPwrCommPort: Private, non-virtual methods
// ---------------------------------------------------------------------------

// Check that Open() has been called yet. If not, throw
tCIDLib::TVoid TJAPwrCommPort::CheckOpened() const
{
    if (!m_psockPort)
    {
        facCIDComm().ThrowKrnlErr
        (
            CID_FILE
            , CID_LINE
            , kCommErrs::errcPort_ReadData
            , TKrnlError::kerrLast()
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotReady
            , strPath()
            , TString(L"Port has not been opened")
        );
    }
}


//
//  Called if we see that we are disconnected. We shutdown and close the
//  socket, but don't delete it.
//
tCIDLib::TVoid TJAPwrCommPort::DropConn()
{
    if (!m_psockPort)
        return;

    try
    {
        if (m_psockPort->bIsConnected())
            m_psockPort->c4Shutdown();
    }

    catch(...)
    {
    }

    try
    {
        m_psockPort->Close();
    }

    catch(...)
    {
    }
}


//
//  Called when we get a read or write request and are not connected. We
//  try to reconnect.
//
tCIDLib::TVoid TJAPwrCommPort::Reconnect()
{
    // Make sure it's closed down
    DropConn();

    //
    //  If we've already created the port, then just do an open on it.
    //  Else we need to do the initial creation, which also does the
    //  open.
    //
    if (m_psockPort)
    {
        m_psockPort->Open(tCIDSock::ESockProtos::TCP, m_ipepTar.eAddrType());
    }
     else
    {
        m_psockPort = new TClientStreamSocket
        (
            tCIDSock::ESockProtos::TCP, m_ipepTar.eAddrType()
        );
    }

    // And try to connect
    m_psockPort->Connect(m_ipepTar);
}

