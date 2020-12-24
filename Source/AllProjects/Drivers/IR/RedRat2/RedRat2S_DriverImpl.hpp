//
// FILE NAME: RedRat2S_DriverImpl.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/28/2002
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
//   CLASS: TRedRat2SDriver
//  PREFIX: orbs
// ---------------------------------------------------------------------------
class TRedRat2SDriver : public TBaseIRSrvDriver
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TRedRat2SDriver
        (
            const   TCQCDriverObjCfg&       cqcdcToLoad
        );

        ~TRedRat2SDriver();


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bBlastTrainingMode();

        tCIDLib::TBoolean bCheckBlastTrainingData
        (
                    tCIDLib::TCard4&        c4DataBytes
            ,       THeapBuf&               mbufToFill
        );

        tCIDLib::TBoolean bGetCommResource
        (
                    TThread&                thrThis
        );

        tCIDLib::TBoolean bResetConnection();

        tCIDLib::TCard4 c4InvokeFldId() const;

        tCQCKit::ECommResults eConnectToDevice
        (
                    TThread&                thrThis
        );

        tCQCKit::EDrvInitRes eInitializeImpl();

        tCQCKit::ECommResults ePollDevice
        (
                    TThread&                thrThis
        );

        tCIDLib::TVoid EnterBlastTrainingMode();

        tCIDLib::TVoid ExitBlastTrainingMode();

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
        //  Unimplemented constructors and operators
        // -------------------------------------------------------------------
        TRedRat2SDriver(const TRedRat2SDriver&);
        tCIDLib::TVoid operator=(const TRedRat2SDriver&);


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid AppendCheckSum
        (
                    TExpByteBuf&            expbData
        );

        tCIDLib::TBoolean bGetAck
        (
                    TThread&                thrThis
            , const tCIDLib::TBoolean       bThrowIfNot = kCIDLib::False
        );

        tCIDLib::TVoid BuildQuery
        (
            const   tCIDLib::TSCh           chCmd
            ,       TExpByteBuf&            expbTarget
        );

        tCIDLib::TVoid BuildQuery
        (
            const   tCIDLib::TSCh           chCmd
            , const TMemBuf&                mbufData
            , const tCIDLib::TCard4         c4Bytes
            ,       TExpByteBuf&            expbTarget
        );

        tCIDLib::TCard1 c1ConvertHexChars
        (
            const   tCIDLib::TCard1         c1First
            , const tCIDLib::TCard1         c1Secd
        );

        tRedRat2S::EResults eGetMsg
        (
                    TThread&                thrThis
            , const tCIDLib::TCard4         c4WaitFor
            ,       TExpByteBuf&            expbData
            ,       tCIDLib::TCard4&        c4Error
        );

        tCIDLib::TVoid SendCapMsg
        (
                    TThread&                thrCaller
        );

        tCIDLib::TVoid WriteMsg
        (
            const   TExpByteBuf&            expbSrc
        );

        tCIDLib::TVoid WriteMsg
        (
            const   tCIDLib::TSCh* const    pchSrc
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bTrainingMode
        //      Indicates whether we are currently in traing mode or not. If
        //      so, then the poll thread will watch for signal data instead
        //      of just pinging the device. If it sees the signal data arrive,
        //      it updates m_c4TrainBytes and m_mbufTrainData.
        //
        //  m_c4FldIdXXX
        //      The field ids of our fields, which we look up after we
        //      register them, so that we can do 'by id' field access.
        //
        //  m_c4NextSend
        //      The RedRat cannot take two messages faster than 50ms apart.
        //      So every time we write, we store the minimum next time we
        //      can send, so the WriteMsg() method will wait if needed.
        //
        //  m_c4TrainBytes
        //      Used during training, it holds the bytes we recieved. It is
        //      set to zero when we enter traing mode, and as long as its
        //      zero, we've recieved no data.
        //
        //  m_cpcfgSerial
        //      The comm port configuration that we were given upon creation.
        //      We use this to configure our serial connection.
        //
        //  m_expbPoll
        //  m_expbWrite
        //      Some buffers for message handling. The first is used by the
        //      poll thread to read messages into, and the second by incoming
        //      write requests, so that they won't have to constantly create
        //      and destroy buffers.
        //
        //  m_mbufTrainData
        //      Used during training, this is where we store the data that we
        //      receive. m_c4TrainBytes is set to indicate how many bytes were
        //      stored.
        //
        //  m_pcommRedRat
        //      The comm port via which we talk to the RedRat.
        //
        //  m_strPort
        //      The serial port we were told to configuration ourself on.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bTrainingMode;
        tCIDLib::TCard4     m_c4FldIdFirmwareVer;
        tCIDLib::TCard4     m_c4FldIdInvoke;
        tCIDLib::TCard4     m_c4FldIdTrainingMode;
        tCIDLib::TCard4     m_c4NextSend;
        tCIDLib::TCard4     m_c4TrainBytes;
        TCommPortCfg        m_cpcfgSerial;
        TExpByteBuf         m_expbPoll;
        TExpByteBuf         m_expbWrite;
        THeapBuf            m_mbufTrainData;
        TCommPortBase*      m_pcommRedRat;
        TString             m_strPort;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TRedRat2SDriver,TBaseIRSrvDriver)
};

#pragma CIDLIB_POPPACK


