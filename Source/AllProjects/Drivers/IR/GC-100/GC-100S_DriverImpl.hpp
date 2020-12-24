//
// FILE NAME: GC100S_DriverImpl.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/04/2002
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
//  This is the header file for the actual driver class. We inherit from the
//  IDL generated base server class, and provide implementations for the
//  stubbed out virtual methods it created for us.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)



// ---------------------------------------------------------------------------
//   CLASS: TGC100Driver
//  PREFIX: orbs
// ---------------------------------------------------------------------------
class TGC100Driver : public TBaseIRSrvDriver
{
    public :
        // -------------------------------------------------------------------
        //  Class types
        // -------------------------------------------------------------------
        using TAddrList = TVector<TGC100Addr>;

        enum class EPollRes
        {
            Off
            , On
            , Error
        };

        enum class ESensorCheck
        {
            Unknown
            , Sensor
            , Blaster
        };


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TGC100Driver
        (
            const   TCQCDriverObjCfg&       cqcdcToLoad
        );

        TGC100Driver(const TGC100Driver&) = delete;

        ~TGC100Driver();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TGC100Driver& operator=(const TGC100Driver&) = delete;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCvtManualBlastData
        (
            const   TString&                strText
            ,       tCIDLib::TCard4&        c4DataBytes
            ,       THeapBuf&               mbufToFill
            ,       TString&                strError
        );

        tCIDLib::TBoolean bGetCommResource
        (
                    TThread&                thrThis
        );

        tCIDLib::TBoolean bResetConnection();

        tCIDLib::TCard4 c4ZoneCount() const;

        tCIDLib::TCard4 c4InvokeFldId() const;

        tCQCKit::ECommResults eBoolFldValChanged
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4FldId
            , const tCIDLib::TBoolean       bNewValue
        );

        tCQCKit::ECommResults eConnectToDevice
        (
                    TThread&                thrThis
        );

        tCQCKit::EDrvInitRes eInitializeImpl();

        tCQCKit::ECommResults ePollDevice
        (
                    TThread&                thrThis
        );

        tCIDLib::TVoid FormatBlastData
        (
            const   TIRBlasterCmd&          irbcFmt
            ,       TString&                strToFill
        );

        tCIDLib::TVoid InvokeBlastCmd
        (
            const   TString&                strDevice
            , const TString&                strCmd
            , const tCIDLib::TCard4         c4ZoneNum
        );

        tCIDLib::TVoid ReleaseCommResource();

        tCIDLib::TVoid SendBlasterData
        (
            const   tCIDLib::TCard4         c4DataBytes
            , const TMemBuf&                mbufToSend
            , const tCIDLib::TCard4         c4ZoneNum
            , const tCIDLib::TCard4         c4RepeatCount
        );

        tCIDLib::TVoid TerminateImpl();


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        ESensorCheck eCheckSensor
        (
            const   tCIDLib::TCard4         c4ModNum
            , const tCIDLib::TCard4         c4ConnNum
        );

        EPollRes ePollInput
        (
            const   tCIDLib::TCard4         c4ModNum
            , const tCIDLib::TCard4         c4ConnNum
            ,       TString&                strSend
            ,       TString&                strRec
        );

        tCIDLib::TVoid GetLine
        (
                    TString&                strToFill
            , const tCIDLib::TEncodedTime   enctEnd
        );

        tCIDLib::TVoid GetLineMS
        (
                    TString&                strToFill
            , const tCIDLib::TCard4         c4WaitMS
        );

        tCIDLib::TVoid GetReply
        (
            const   TString&                strTermStr
            ,       TString&                strToFill
            , const tCIDLib::TCard4         c4WaitMS
        );

        tCIDLib::TVoid QueryConfig();

        tCIDLib::TVoid RegisterFields();

        tCIDLib::TVoid SendIRCmd
        (
            const   tCIDLib::TCard1* const  pc1Text
            , const tCIDLib::TCard4         c4Bytes
            , const tCIDLib::TCard4         c4ZoneNum
            , const tCIDLib::TCard4         c4RepeatCount
        );

        tCIDLib::TVoid SendMessage
        (
            const   tCIDLib::TSCh* const    pszMsg
        );

        tCIDLib::TVoid SendMessage
        (
            const   TString&                strMsg
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c2NextId
        //      We have to put an id number into each IR command we send,
        //      which it sends back to us on the completion. So we just
        //      use a counter that we bump up for each one.
        //
        //  m_c4FailCnt
        //      The GC-100 sometimes just gets confused and will return a
        //      message not understood error. We try to be forgiving of this
        //      and our poll loop will watch for them and bump this counter.
        //      If we get three in row, we'll disconnect and try to reset
        //      the connection. If we get a good poll, we zero it back out.
        //
        //  m_c4FldIdXXX
        //      The field ids of our fields, which we look up after we
        //      register them, so that we can do 'by id' field access.
        //
        //  m_c4LastPollInd
        //      We round robin through the m_colFldList vector during each
        //      poll cycle. We'll do up to a certain number and then stop,
        //      and then pick up there again next time. So this is used to
        //      keep up with where we left off last time.
        //
        //  m_c4PollCount
        //      If there's nothing to poll, we just need to do a ping to
        //      the device every so many times through the poll callback.
        //
        //  m_colFldList
        //  m_colZoneList
        //      Each connector on the GC-100 has an address composed of
        //      a module and connector number. We need to be able to map
        //      between them and our fields and zone numbers. So for each
        //      connector that corresponds to a field, we add an address
        //      item to the field list, and for each one that corresponds
        //      to a blaster zone, we add to the zone list. Each item
        //      contains field id/name, connector type, and connector id
        //      info.
        //
        //  m_cpcfgSock
        //      The socket configuration that we were given upon creation.
        //      We use this to configure our socket connection.
        //
        //  m_sockConn
        //      The stream socket we use to talk to the device.
        //
        //  m_strPingQ
        //      If we don't have anything to poll actively, we'll periodically
        //      send a simple ping in the form of a query.
        //
        //  m_strRec
        //  m_strSend
        //      Some temp strings used by the poll method, to avoid it
        //      constantly creating new strings and destroying them, which
        //      just contributes to heap churn.
        // -------------------------------------------------------------------
        tCIDLib::TCard2     m_c2NextId;
        tCIDLib::TCard4     m_c4FailCnt;
        tCIDLib::TCard4     m_c4FldIdFirmwareVer;
        tCIDLib::TCard4     m_c4FldIdInvoke;
        tCIDLib::TCard4     m_c4FldIdTrainingMode;
        tCIDLib::TCard4     m_c4LastPollInd;
        tCIDLib::TCard4     m_c4PollCount;
        TAddrList           m_colFldList;
        TAddrList           m_colZoneList;
        TCQCIPConnCfg       m_conncfgSock;
        TClientStreamSocket m_sockConn;
        const TString       m_strPingQ;
        TString             m_strRec;
        TString             m_strSend;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TGC100Driver,TBaseIRSrvDriver)
};

#pragma CIDLIB_POPPACK


