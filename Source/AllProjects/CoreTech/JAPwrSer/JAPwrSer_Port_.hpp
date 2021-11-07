//
// FILE NAME: JAPwrSer_Port_.hpp
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
//  This is the header for the JAPwrSer_Port.cpp file, which derives a
//  class, TJAPwrCommPort, from the TCommPortBase class. TCommPortBase is an
//  abstract base for comm ports, and this one creates a derivate of that
//  interface that works in terms the serial ports of a Just Add Pwoer.
//
//  We don't have to export this class because serial ports are create via
//  the CIDComm facility object, via plugged in factories. Our factory gets
//  plugged in by any containing apps that want to support JAP ports.
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
//  All we need to connect is the end point. The IP address is the JAP
//  unit, and the TCP/IP port is fixed.
//
//  Paths for these ports are in the form:
//
//      /JAPwr/xxx/COMn
//
//  where xxx is a name of a configured JAP (the user has to define which
//  JAPs to make available and give each a name) and n is the serial port number.
//  The application has to load configuration info into the factory, so the
//  factory can look up the config and know the name and IP end point, which
//  it passes to us.
//
// CAVEATS/GOTCHAS:
//
//  1)  We can't set or get the configuration of the port, for various
//      practical reasons. So we just lie about any querying or setting of
//      port config. In some cases, where it might be really important we
//      throw a not supported exception.
//
// LOG:
//
//  $Log$
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

class TClientStreamSocket;


// ---------------------------------------------------------------------------
//  CLASS: TJAPwrCommPort
// PREFIX: comm
// ---------------------------------------------------------------------------
class TJAPwrCommPort : public TCommPortBase
{
    public  :
        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static tCIDLib::TBoolean bCheckBaud
        (
            const   tCIDLib::TCard4         c4ToCheck
        );


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TJAPwrCommPort
        (
            const   TString&                strPath
            , const TString&                strName
            , const TString&                strAddr
            , const tCIDLib::TCard4         c4PortNum
        );

        TJAPwrCommPort(const TJAPwrCommPort&) = delete;
        TJAPwrCommPort(TJAPwrCommPort&&) = delete;

        ~TJAPwrCommPort();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TJAPwrCommPort& operator=(const TJAPwrCommPort&) = delete;
        TJAPwrCommPort& operator=(TJAPwrCommPort&&) = delete;


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
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid CheckOpened() const;

        tCIDLib::TVoid DropConn();

        tCIDLib::TVoid Reconnect();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4PortNum
        //      The port number on the JAP we are connected to, e.g. 1, 2, 3,
        //      and so forth. For now, there is only ever a 1 for JAPs.
        //
        //  m_fcolCache
        //      We use a fundamental deque to provide a simple cache. We read
        //      ahead a bit if we can and cache the bytes here. Then we can
        //      pull them out of here until we empty the cache.
        //
        //  m_ipepTar
        //      The end point we are to connect to. This can't change once
        //      the port is created by the port factory, so we can store it
        //      away for later use.
        //
        //  m_mbufIO
        //      We need a buffer to transcode messages to and from ASCII
        //      that the JAP uses.
        //
        //  m_psockPort
        //      The socket we use to read/write serial data. It's a pointer so
        //      we don't have to force the CIDComm headers on our clients. We
        //      can just bring it in internally.
        //
        //  m_strAddr
        //      The host address of the JAP our port is on.
        //
        //  m_strName
        //      The name of the configured JAP our port is on, which we can
        //      use in logged messages.
        // -------------------------------------------------------------------
        tCIDLib::TCard4             m_c4PortNum;
        TFundDeque<tCIDLib::TCard1> m_fcolCache;
        TIPEndPoint                 m_ipepTar;
        THeapBuf                    m_mbufIO;
        mutable TClientStreamSocket* m_psockPort;
        TString                     m_strAddr;
        TString                     m_strName;


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TJAPwrCommPort,TCommPortBase)
};

#pragma CIDLIB_POPPACK



