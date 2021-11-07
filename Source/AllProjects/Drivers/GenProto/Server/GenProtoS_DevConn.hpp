//
// FILE NAME: GenProtoS_DevConn.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/24/2003
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
//  This is the header file for the GenProtoS_DevConn.cpp file, which
//  implements an abstraction for a source connection, so that we don't
//  have to deal with different connection types in the driver itself. We
//  have a base class, and derivatives currently for comm port and stream
//  socket. The driver, at ctor, creates the appropriate type based on
//  the passed connection config, and after that just deals with them
//  abstractly.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TDevConn
//  PREFIX: devc
// ---------------------------------------------------------------------------
class TDevConn : public TObject
{
    public :
        // -------------------------------------------------------------------
        //  Public Destructor
        // -------------------------------------------------------------------
        TDevConn(const TDevConn&) = delete;
        TDevConn(TDevConn&&) = delete;

        ~TDevConn();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TDevConn& operator=(const TDevConn&) = delete;
        TDevConn& operator=(TDevConn&&) = delete;


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        virtual tCIDLib::TBoolean bConnect
        (
                    TThread&                thrThis
            , const tCQCKit::EVerboseLvls   eVerbose
        ) = 0;

        virtual tCIDLib::TBoolean bGetCommResource
        (
                    TThread&                thrThis
            , const tCQCKit::EVerboseLvls   eVerbose
        ) = 0;

        virtual tCIDLib::TBoolean bIsConnected() const = 0;

        virtual tCIDLib::TBoolean bReadByte
        (
            const   tCIDLib::TCard4         c4WaitFor
            ,       tCIDLib::TCard1&        c1ToFill
        ) = 0;

        virtual tCIDLib::TBoolean bWrite
        (
                    TMemBuf&                mbufToFill
            , const tCIDLib::TCard4         c4ToWrite
        ) = 0;

        virtual tCIDLib::TCard4 c4Read
        (
                    TMemBuf&                mbufToFill
            , const tCIDLib::TCard4         c4MaxToRead
            , const tCIDLib::TCard4         c4Wait = kCIDLib::c4MaxWait
        ) = 0;

        virtual tCIDLib::TVoid PurgeReadBuf() = 0;

        virtual tCIDLib::TVoid ReleaseCommResource
        (
            const   tCQCKit::EVerboseLvls   eVerbose
        ) = 0;


    protected :
        // -------------------------------------------------------------------
        //  Hidden constructors
        // -------------------------------------------------------------------
        TDevConn();


    private :
            // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TDevConn,TObject)
};


// ---------------------------------------------------------------------------
//   CLASS: TSerialConn
//  PREFIX: devc
// ---------------------------------------------------------------------------
class TSerialConn : public TDevConn
{
    public :
        // -------------------------------------------------------------------
        //  Public Destructor
        // -------------------------------------------------------------------
        TSerialConn() = delete;

        TSerialConn
        (
            const   TCQCSerialConnCfg&      conncfgToUse
        );

        TSerialConn(const TSerialConn&) = delete;
        TSerialConn(TSerialConn&&) = delete;

        ~TSerialConn();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TSerialConn& operator=(const TSerialConn&) = delete;
        TSerialConn& operator=(TSerialConn&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bConnect
        (
                    TThread&                thrThis
            , const tCQCKit::EVerboseLvls   eVerbose
        )   final;

        tCIDLib::TBoolean bGetCommResource
        (
                    TThread&                thrThis
            , const tCQCKit::EVerboseLvls   eVerbose
        )   final;

        tCIDLib::TBoolean bIsConnected() const final;

        tCIDLib::TBoolean bReadByte
        (
            const   tCIDLib::TCard4         c4WaitFor
            ,       tCIDLib::TCard1&        c1ToFill
        )   final;

        tCIDLib::TBoolean bWrite
        (
                    TMemBuf&                mbufToFill
            , const tCIDLib::TCard4         c4ToWrite
        )   final;

        tCIDLib::TCard4 c4Read
        (
                    TMemBuf&                mbufToFill
            , const tCIDLib::TCard4         c4MaxToRead
            , const tCIDLib::TCard4         c4Wait = kCIDLib::c4MaxWait
        )   final;

        tCIDLib::TVoid PurgeReadBuf() final;

        tCIDLib::TVoid ReleaseCommResource
        (
            const   tCQCKit::EVerboseLvls   eVerbose
        )   final;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_pcommDev
        //      The comm port we use to connect to the device.
        //
        //  m_conncfgToUse
        //      The serial connection configuration we were given in our ctor
        //      to use.
        // -------------------------------------------------------------------
        TCommPortBase*      m_pcommDev;
        TCQCSerialConnCfg   m_conncfgToUse;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TSerialConn,TDevConn)
};


// ---------------------------------------------------------------------------
//   CLASS: TSockConn
//  PREFIX: devc
// ---------------------------------------------------------------------------
class TSockConn : public TDevConn
{
    public :
        // -------------------------------------------------------------------
        //  Public Destructor
        // -------------------------------------------------------------------
        TSockConn() = delete;

        TSockConn
        (
            const   TCQCIPConnCfg&          conncfgToUse
        );

        TSockConn(const TSockConn&) = delete;
        TSockConn(TSockConn&&) = delete;

        ~TSockConn();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TSockConn& operator=(const TSockConn&) = delete;
        TSockConn& operator=(TSockConn&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bConnect
        (
                    TThread&                thrThis
            , const tCQCKit::EVerboseLvls   eVerbose
        )   final;

        tCIDLib::TBoolean bGetCommResource
        (
                    TThread&                thrThis
            , const tCQCKit::EVerboseLvls   eVerbose
        )   final;

        tCIDLib::TBoolean bIsConnected() const final;

        tCIDLib::TBoolean bReadByte
        (
            const   tCIDLib::TCard4         c4WaitFor
            ,       tCIDLib::TCard1&        c1ToFill
        )   final;

        tCIDLib::TBoolean bWrite
        (
                    TMemBuf&                mbufToFill
            , const tCIDLib::TCard4         c4ToWrite
        )   final;

        tCIDLib::TCard4 c4Read
        (
                    TMemBuf&                mbufToFill
            , const tCIDLib::TCard4         c4MaxToRead
            , const tCIDLib::TCard4         c4Wait = kCIDLib::c4MaxWait
        )   final;

        tCIDLib::TVoid PurgeReadBuf() final;

        tCIDLib::TVoid ReleaseCommResource
        (
            const   tCQCKit::EVerboseLvls   eVerbose
        )   final;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_conncfgSock
        //      The connection configuration we were given.
        //
        //  m_sockDev
        //      The socket we use to connect to the target device.
        // -------------------------------------------------------------------
        TCQCIPConnCfg       m_conncfgSock;
        TClientStreamSocket m_sockDev;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TSockConn,TDevConn)
};

#pragma CIDLIB_POPPACK


