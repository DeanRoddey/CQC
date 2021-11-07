//
// FILE NAME: CQCRPortClient_Port_.hpp
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
//  This is the header for the CQCRPortClient_Port.cpp file, which derives a
//  class, TRemCommPort, from the TCommPortBase class. TCommPortBase is an
//  abstract base for comm ports, and this one creates a derivate of that
//  interface that works in terms of our remote serial port server ORB
//  interface, so that we can support remote ports purely through software.
//
//  We don't have to export this class because serial ports are create via
//  the CIDComm facility object, via plugged in factories. Our factory gets
//  plugged in by any containing apps that want to support remote ports.
//
//  In order to be a lot more efficient, we will actually try to read more
//  data than is asked for, and queue it up, and then dispense bytes out
//  of that local cache before asking for more bytes from the other side.
//  A lot of drivers do character by character streaming of data, and that
//  just kills us when going over the internet.
//
//  So what we do is do the original request, and then do a request for
//  more bytes, using a very short timeout so that if nothing is there, we
//  won't make much difference. But usually, by the time the driver goes
//  looking for data, it's already in the input buffer so we usually can
//  make a huge difference with this simple strategy.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//  CLASS: TRemCommPort
// PREFIX: comm
// ---------------------------------------------------------------------------
class TRemCommPort : public TCommPortBase
{
    public  :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TRemCommPort() = delete;

        TRemCommPort
        (
            const   TString&                strPath
            , const TIPEndPoint&            ipepSrv
        );

        TRemCommPort
        (
            const   TString&                strPath
        );

        TRemCommPort(const TRemCommPort&) = delete;
        TRemCommPort(TRemCommPort&&) = delete;

        ~TRemCommPort();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TRemCommPort& operator=(const TRemCommPort&) = delete;
        TRemCommPort& operator=(TRemCommPort&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIsOpen() const final;

        tCIDLib::TBoolean bQueryCTSOn() const final;

        tCIDLib::TBoolean bQueryDSROn() const final;

        tCIDLib::TCard4 c4ReadRawBufMS
        (
                    tCIDLib::TVoid* const   pToFill
            , const tCIDLib::TCard4         c4MaxBytes
            , const tCIDLib::TCard4         c4Wait
        )   final;

        tCIDLib::TCard4 c4ReadRawBuf
        (
                    tCIDLib::TVoid* const   pToFill
            , const tCIDLib::TCard4         c4MaxBytes
            , const tCIDLib::TEncodedTime   enctWait
        )   final;

        tCIDLib::TCard4 c4WriteRawBufMS
        (
            const   tCIDLib::TVoid* const   pToWrite
            , const tCIDLib::TCard4         c4BytesToWrite
            , const tCIDLib::TCard4         c4Wait
        )   final;

        tCIDLib::TVoid Close() final;

        tCIDLib::TVoid Cycle
        (
            const   tCIDComm::EOpenFlags    eFlags = tCIDComm::EOpenFlags::None
            , const tCIDLib::TCard4         c4ReadBufSz = 0
            , const tCIDLib::TCard4         c4WriteBufSz = 0
        )   final;

        tCIDComm::EFlags eFlags() const final;

        tCIDLib::TVoid FlushWriteBuffer() final;

        tCIDLib::TVoid Open
        (
            const   tCIDComm::EOpenFlags    eFlags = tCIDComm::EOpenFlags::None
            , const tCIDLib::TCard4         c4ReadBufSz = 0
            , const tCIDLib::TCard4         c4WriteBufSz = 0
        )   final;

        tCIDLib::TVoid PurgeReadData() final;

        tCIDLib::TVoid PurgeWriteData() final;

        tCIDLib::TVoid QueryCfg
        (
                    TCommPortCfg&           cpcfgToFill
        )   const final;

        tCIDLib::TVoid QueryLineStates
        (
                    tCIDLib::TBoolean&      bCTSOn
            ,       tCIDLib::TBoolean&      bDSROn
            ,       tCIDLib::TBoolean&      bRingOn
            ,       tCIDLib::TBoolean&      bRLSDOn
        )   const final;

        tCIDLib::TVoid QueryXChars
        (
                    tCIDLib::TSCh&          schOn
            ,       tCIDLib::TSCh&          schOff
        )   const final;

        tCIDLib::TVoid Reset() final;

        tCIDLib::TVoid SetBaudRate
        (
            const   tCIDLib::TCard4         c4ToSet
        )   final;

        tCIDLib::TVoid SetBreak
        (
            const   tCIDLib::TBoolean       bToSet
        )   final;

        tCIDLib::TVoid SetDataBits
        (
            const   tCIDComm::EDataBits     eToSet
        )   final;

        tCIDLib::TVoid SetDTR
        (
            const   tCIDComm::EPortDTR      eToSet
        )   final;

        tCIDLib::TVoid SetParity
        (
            const   tCIDComm::EParities     eToSet
        )   final;

        tCIDLib::TVoid SetRTS
        (
            const   tCIDComm::EPortRTS      eToSet
        )   final;

        tCIDLib::TVoid SetStopBits
        (
            const   tCIDComm::EStopBits     eToSet
        )   final;

        tCIDLib::TVoid SetCfg
        (
            const   TCommPortCfg&           cpcfgToSet
        )   final;

        tCIDLib::TVoid SetEOFChar
        (
            const   tCIDLib::TSCh           schToSet
        )   final;

        tCIDLib::TVoid SetFlags
        (
            const   tCIDComm::EFlags        eToSet
            , const tCIDComm::EFlags        eMask
        )   final;



    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4Cookie
        //      When we open a port we get a cookie back that we have to pass
        //      in for any of the other calls. If the port isn't open, this
        //      guy will be c4MaxCard.
        //
        //  m_fcolCache
        //      We use a fundamental deque to provide a simple cache. We read
        //      ahead a bit if we can and cache the bytes here. Then we can
        //      pull them out of here until we empty the cache.
        //
        //  m_ipepServer
        //      The end point of the server that is providing our serial port.
        //      We are given this upon contruction and keep it around so that
        //      we can set up our client proxy.
        //
        //  m_mbufRead
        //  m_mbufWrite
        //      The com port interface supports a raw buffer read/write, but
        //      we can't do that. So we keep some buffers around to use as
        //      temp buffers internally on those calls.
        //
        //  m_orbcPort
        //      Our ORB client proxy for talking to the remote server
        // -------------------------------------------------------------------
        tCIDLib::TCard4             m_c4Cookie;
        TFundDeque<tCIDLib::TCard1> m_fcolCache;
        TIPEndPoint                 m_ipepServer;
        THeapBuf                    m_mbufRead;
        THeapBuf                    m_mbufWrite;
        mutable tRPortClient::TSerialPortClient m_orbcPort;


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TRemCommPort,TCommPortBase)
};

#pragma CIDLIB_POPPACK


