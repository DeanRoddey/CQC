//
// FILE NAME: CQCRPortClient_Port.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/14/2005
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
//  This file implements the TRemCommPort class.
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
#include    "CQCRPortClient_.hpp"


// ---------------------------------------------------------------------------
//  Magic RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TRemCommPort,TCommPortBase)


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
//  CLASS: TRemCommPort
// PREFIX: comm
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TRemCommPort: Constructors and Destructor
// ---------------------------------------------------------------------------
TRemCommPort::TRemCommPort(const TString& strPath, const TIPEndPoint& ipepSrv) :

    TCommPortBase(strPath)
    , m_c4Cookie(kCIDLib::c4MaxCard)
    , m_fcolCache(CQCRPortClient_Port::c4CacheSz)
    , m_ipepServer(ipepSrv)
    , m_mbufRead(8192UL, 0x100000)
    , m_mbufWrite(8192UL, 0x100000)
{
}

TRemCommPort::TRemCommPort(const TString& strPath) :

    TCommPortBase(strPath)
    , m_c4Cookie(kCIDLib::c4MaxCard)
    , m_fcolCache(CQCRPortClient_Port::c4CacheSz)
    , m_mbufRead(8192UL, 0x100000)
    , m_mbufWrite(8192UL, 0x100000)
{
}

TRemCommPort::~TRemCommPort()
{
    try
    {
        if (m_orbcPort.pobjData())
        {
            m_orbcPort->ResetProxy();
            m_orbcPort.DropRef();
        }
    }

    catch(...)
    {
    }
}


// ---------------------------------------------------------------------------
//  TRemCommPort: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TRemCommPort::bIsOpen() const
{
    return (m_c4Cookie != kCIDLib::c4MaxCard);
}


tCIDLib::TBoolean TRemCommPort::bQueryCTSOn() const
{
    tCIDLib::TBoolean   bCTSOn;
    tCIDLib::TBoolean   bDSROn;
    tCIDLib::TBoolean   bRingOn;
    tCIDLib::TBoolean   bRLSDOn;
    m_orbcPort->QueryLineStates(m_c4Cookie, bCTSOn, bDSROn, bRingOn, bRLSDOn);
    return bCTSOn;
}


tCIDLib::TBoolean TRemCommPort::bQueryDSROn() const
{
    tCIDLib::TBoolean   bCTSOn;
    tCIDLib::TBoolean   bDSROn;
    tCIDLib::TBoolean   bRingOn;
    tCIDLib::TBoolean   bRLSDOn;
    m_orbcPort->QueryLineStates(m_c4Cookie, bCTSOn, bDSROn, bRingOn, bRLSDOn);
    return bDSROn;
}


tCIDLib::TCard4
TRemCommPort::c4ReadRawBufMS(       tCIDLib::TVoid* const   pToFill
                            , const tCIDLib::TCard4         c4MaxBytes
                            , const tCIDLib::TCard4         c4Wait)
{
    // Set up the wait time in ticks and call the other version
    const tCIDLib::TEncodedTime enctWait(kCIDLib::enctOneMilliSec * c4Wait);
    return c4ReadRawBuf(pToFill, c4MaxBytes, enctWait);
}


tCIDLib::TCard4
TRemCommPort::c4ReadRawBuf(         tCIDLib::TVoid* const   pToFill
                            , const tCIDLib::TCard4         c4MaxBytes
                            , const tCIDLib::TEncodedTime   enctWait)
{
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
        //  We can't really do raw buffers over the ORB interface so we have
        //  a heap buffer for the read, the copy to the caller's buffer.
        //
        //  We reduce the byte count by the number we got from the cache.
        //
        tCIDLib::TCard4 c4Read = 0;
        m_orbcPort->ReadBuf
        (
            m_c4Cookie, c4Read, m_mbufRead, c4MaxBytes - c4FromCache, enctWait
        );

        //
        //  Copy the data over to the caller's buffer. We copy in at the place
        //  we left off by cache bytes. And the return bytes is the bytes read
        //  plus the bytes we got from the cache.
        //
        m_mbufRead.CopyOut(pc1Tar, c4Read);
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
            m_orbcPort->ReadBufMS
            (
                m_c4Cookie, c4Read, m_mbufRead, CQCRPortClient_Port::c4CacheSz, 5
            );

            // If we got any, load up the cache
            const tCIDLib::TCard1* pc1Src = m_mbufRead.pc1Data();
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Read; c4Index++)
                m_fcolCache.AddAtFront(*pc1Src++);
        }
    }

    catch(const TError& errToCatch)
    {
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
TRemCommPort::c4WriteRawBufMS(  const   tCIDLib::TVoid* const   pToWrite
                                , const tCIDLib::TCard4         c4BytesToWrite
                                , const tCIDLib::TCard4         c4Wait)
{
    // If no bytes, don't bother since this is a high overhead operation
    if (!c4BytesToWrite)
        return 0;

    //
    //  We can't really do raw buffers over the ORB interface so we make
    //  a heap buffer for the write, and copy the data into it before
    //  sending.
    //
    m_mbufWrite.CopyIn(pToWrite, c4BytesToWrite);
    const tCIDLib::TCard4 c4Written = m_orbcPort->c4WriteBufMS
    (
        m_c4Cookie, c4BytesToWrite, m_mbufWrite, c4Wait
    );
    return c4Written;
}


tCIDLib::TVoid TRemCommPort::Close()
{
    try
    {
        m_fcolCache.RemoveAll();
        const tCIDLib::TCard4 c4Cookie = m_c4Cookie;
        m_c4Cookie = kCIDLib::c4MaxCard;
        if (m_orbcPort.pobjData())
        {
            m_orbcPort->Close(c4Cookie);
            m_orbcPort->ResetProxy();
        }
    }

    catch(...)
    {
        m_c4Cookie = kCIDLib::c4MaxCard;
        if (m_orbcPort.pobjData())
            m_orbcPort->ResetProxy();
    }
}


tCIDLib::TVoid TRemCommPort::Cycle( const   tCIDComm::EOpenFlags eFlags
                                    , const tCIDLib::TCard4      c4ReadBufSz
                                    , const tCIDLib::TCard4      c4WriteBufSz)
{
    m_fcolCache.RemoveAll();
    m_orbcPort->Cycle(m_c4Cookie, eFlags, c4ReadBufSz, c4WriteBufSz);
}


tCIDComm::EFlags TRemCommPort::eFlags() const
{
    tCIDComm::EFlags eRet;
    m_orbcPort->QueryFlags(m_c4Cookie, eRet);
    return eRet;
}


tCIDLib::TVoid TRemCommPort::FlushWriteBuffer()
{
    m_orbcPort->FlushWriteBuffer(m_c4Cookie);
}


tCIDLib::TVoid TRemCommPort::Open(  const   tCIDComm::EOpenFlags eFlags
                                    , const tCIDLib::TCard4      c4ReadBufSz
                                    , const tCIDLib::TCard4      c4WriteBufSz)
{
    // Try to get our connection to the server set up
    try
    {
        m_orbcPort = facCQCRPortClient().orbcMakePortProxy(m_ipepServer);
    }

    catch(const TError& errToCatch)
    {
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
    m_c4Cookie = m_orbcPort->c4Open(strPath(), eFlags, c4ReadBufSz, c4WriteBufSz);
    m_fcolCache.RemoveAll();
}


tCIDLib::TVoid TRemCommPort::PurgeReadData()
{
    m_fcolCache.RemoveAll();
    m_orbcPort->PurgeReadData(m_c4Cookie);
}


tCIDLib::TVoid TRemCommPort::PurgeWriteData()
{
    m_orbcPort->PurgeWriteData(m_c4Cookie);
}


tCIDLib::TVoid TRemCommPort::QueryCfg(TCommPortCfg& cpcfgToFill) const
{
    m_orbcPort->QueryCfg(m_c4Cookie, cpcfgToFill);
}


tCIDLib::TVoid
TRemCommPort::QueryLineStates(  tCIDLib::TBoolean&      bCTSOn
                                , tCIDLib::TBoolean&    bDSROn
                                , tCIDLib::TBoolean&    bRingOn
                                , tCIDLib::TBoolean&    bRLSDOn) const
{
    m_orbcPort->QueryLineStates(m_c4Cookie, bCTSOn, bDSROn, bRingOn, bRLSDOn);
}


tCIDLib::TVoid
TRemCommPort::QueryXChars(tCIDLib::TSCh& schOn, tCIDLib::TSCh& schOff) const
{
    tCIDLib::TCard4 c4Off;
    tCIDLib::TCard4 c4On;
    m_orbcPort->QueryXChars(m_c4Cookie, c4On, c4Off);
    schOn = tCIDLib::TSCh(c4On);
    schOff = tCIDLib::TSCh(c4Off);
}


tCIDLib::TVoid TRemCommPort::Reset()
{
    m_fcolCache.RemoveAll();
    m_orbcPort->Reset(m_c4Cookie);
}


tCIDLib::TVoid TRemCommPort::SetBaudRate(const tCIDLib::TCard4 c4ToSet)
{
    m_orbcPort->SetBaudRate(m_c4Cookie, c4ToSet);
}


tCIDLib::TVoid TRemCommPort::SetBreak(const tCIDLib::TBoolean bState)
{
    m_orbcPort->SetBaudRate(m_c4Cookie, bState);
}


tCIDLib::TVoid TRemCommPort::SetCfg(const TCommPortCfg& cpcfgToSet)
{
    m_orbcPort->SetCfg(m_c4Cookie, cpcfgToSet);
}


tCIDLib::TVoid TRemCommPort::SetDataBits(const tCIDComm::EDataBits eToSet)
{
    m_orbcPort->SetDataBits(m_c4Cookie, eToSet);
}


tCIDLib::TVoid TRemCommPort::SetDTR(const tCIDComm::EPortDTR eToSet)
{
    m_orbcPort->SetDTR(m_c4Cookie, eToSet);
}


tCIDLib::TVoid TRemCommPort::SetParity(const tCIDComm::EParities eToSet)
{
    m_orbcPort->SetParity(m_c4Cookie, eToSet);
}


tCIDLib::TVoid TRemCommPort::SetRTS(const tCIDComm::EPortRTS eToSet)
{
    m_orbcPort->SetRTS(m_c4Cookie, eToSet);
}


tCIDLib::TVoid TRemCommPort::SetStopBits(const tCIDComm::EStopBits eToSet)
{
    m_orbcPort->SetStopBits(m_c4Cookie, eToSet);
}


tCIDLib::TVoid TRemCommPort::SetEOFChar(const tCIDLib::TSCh schToSet)
{
    m_orbcPort->SetEOFChar(m_c4Cookie, schToSet);
}


tCIDLib::TVoid TRemCommPort::SetFlags(  const   tCIDComm::EFlags    eToSet
                                        , const tCIDComm::EFlags    eMask)
{
    m_orbcPort->SetFlags(m_c4Cookie, eToSet, eMask);
}

