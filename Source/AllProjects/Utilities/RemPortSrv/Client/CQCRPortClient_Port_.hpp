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
        TRemCommPort
        (
            const   TString&                strPath
            , const TIPEndPoint&            ipepSrv
        );

        TRemCommPort
        (
            const   TString&                strPath
        );

        ~TRemCommPort();


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIsOpen() const;

        tCIDLib::TBoolean bQueryCTSOn() const;

        tCIDLib::TBoolean bQueryDSROn() const;

        tCIDLib::TCard4 c4ReadRawBufMS
        (
                    tCIDLib::TVoid* const   pToFill
            , const tCIDLib::TCard4         c4MaxBytes
            , const tCIDLib::TCard4         c4Wait
        );

        tCIDLib::TCard4 c4ReadRawBuf
        (
                    tCIDLib::TVoid* const   pToFill
            , const tCIDLib::TCard4         c4MaxBytes
            , const tCIDLib::TEncodedTime   enctWait
        );

        tCIDLib::TCard4 c4WriteRawBufMS
        (
            const   tCIDLib::TVoid* const   pToWrite
            , const tCIDLib::TCard4         c4BytesToWrite
            , const tCIDLib::TCard4         c4Wait
        );

        tCIDLib::TVoid Close();

        tCIDLib::TVoid Cycle
        (
            const   tCIDComm::EOpenFlags    eFlags = tCIDComm::EOpenFlags::None
            , const tCIDLib::TCard4         c4ReadBufSz = 0
            , const tCIDLib::TCard4         c4WriteBufSz = 0
        );

        tCIDComm::EFlags eFlags() const;

        tCIDLib::TVoid FlushWriteBuffer();

        tCIDLib::TVoid Open
        (
            const   tCIDComm::EOpenFlags    eFlags = tCIDComm::EOpenFlags::None
            , const tCIDLib::TCard4         c4ReadBufSz = 0
            , const tCIDLib::TCard4         c4WriteBufSz = 0
        );

        tCIDLib::TVoid PurgeReadData();

        tCIDLib::TVoid PurgeWriteData();

        tCIDLib::TVoid QueryCfg
        (
                    TCommPortCfg&           cpcfgToFill
        )   const;

        tCIDLib::TVoid QueryLineStates
        (
                    tCIDLib::TBoolean&      bCTSOn
            ,       tCIDLib::TBoolean&      bDSROn
            ,       tCIDLib::TBoolean&      bRingOn
            ,       tCIDLib::TBoolean&      bRLSDOn
        )   const;

        tCIDLib::TVoid QueryXChars
        (
                    tCIDLib::TSCh&          schOn
            ,       tCIDLib::TSCh&          schOff
        )   const;

        tCIDLib::TVoid Reset();

        tCIDLib::TVoid SetBaudRate
        (
            const   tCIDLib::TCard4         c4ToSet
        );

        tCIDLib::TVoid SetBreak
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TVoid SetDataBits
        (
            const   tCIDComm::EDataBits     eToSet
        );

        tCIDLib::TVoid SetDTR
        (
            const   tCIDComm::EPortDTR      eToSet
        );

        tCIDLib::TVoid SetParity
        (
            const   tCIDComm::EParities     eToSet
        );

        tCIDLib::TVoid SetRTS
        (
            const   tCIDComm::EPortRTS      eToSet
        );

        tCIDLib::TVoid SetStopBits
        (
            const   tCIDComm::EStopBits     eToSet
        );

        tCIDLib::TVoid SetCfg
        (
            const   TCommPortCfg&           cpcfgToSet
        );

        tCIDLib::TVoid SetEOFChar
        (
            const   tCIDLib::TSCh           schToSet
        );

        tCIDLib::TVoid SetFlags
        (
            const   tCIDComm::EFlags        eToSet
            , const tCIDComm::EFlags        eMask
        );



    private :
        // -------------------------------------------------------------------
        //  Unimplemented constructors and operators
        // -------------------------------------------------------------------
        TRemCommPort(const TRemCommPort&);
        tCIDLib::TVoid operator=(const TRemCommPort&);


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


