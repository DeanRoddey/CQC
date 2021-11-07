//
// FILE NAME: CQCRPortSrv_SerialImpl.hpp
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
//  This is the header for the CQCRPortSrv_SerialImpl.cpp file, which
//  implements our derivative of the server side serial port server stub.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  CLASS: TRemSerialImpl
// PREFIX: orbs
// ---------------------------------------------------------------------------
class TRemSerialImpl : public TRemSerialSrvServerBase
{
    public  :
        // -------------------------------------------------------------------
        // Constructors and Destructor
        // -------------------------------------------------------------------
        TRemSerialImpl
        (
            const   TOrbObjId&              ooidToUse
        );

        ~TRemSerialImpl();


        // --------------------------------------------------------------------
        // Public, inherited methods
        // --------------------------------------------------------------------
        tCIDLib::TBoolean bQueryCommPorts
        (
                    TVector<TString>&       colToFill
        )   final;

        tCIDLib::TBoolean bIsOpen
        (
            const   tCIDLib::TCard4         c4Cookie
        )   final;

        tCIDLib::TBoolean bQueryCTSOn
        (
            const   tCIDLib::TCard4         c4Cookie
        )   final;

        tCIDLib::TBoolean bQueryDSROn
        (
            const   tCIDLib::TCard4         c4Cookie
        )   final;

        tCIDLib::TCard4 c4Open
        (
            const   TString&                strPortPath
            , const tCIDComm::EOpenFlags    eFlags
            , const tCIDLib::TCard4         c4ReadBufSz
            , const tCIDLib::TCard4         c4WriteBufSz
        )   final;

        tCIDLib::TCard4 c4WriteBufMS
        (
            const   tCIDLib::TCard4         c4Cookie
            , const tCIDLib::TCard4         c4BufSz_mbufToWrite
            , const THeapBuf&               mbufToWrite
            , const tCIDLib::TCard4         c4Wait
        )   final;

        tCIDLib::TVoid Close
        (
            const   tCIDLib::TCard4         c4Cookie
        )   final;

        tCIDLib::TVoid CloseAllPorts() final;

        tCIDLib::TVoid Cycle
        (
            const   tCIDLib::TCard4         c4Cookie
            , const tCIDComm::EOpenFlags    eFlags
            , const tCIDLib::TCard4         c4ReadBufSz
            , const tCIDLib::TCard4         c4WriteBufSz
        )   final;

        tCIDLib::TVoid FlushWriteBuffer
        (
            const   tCIDLib::TCard4         c4Cookie
        )   final;

        tCIDLib::TVoid PurgeReadData
        (
            const   tCIDLib::TCard4         c4Cookie
        )   final;

        tCIDLib::TVoid PurgeWriteData
        (
            const   tCIDLib::TCard4         c4Cookie
        )   final;

        tCIDLib::TVoid QueryCfg
        (
            const   tCIDLib::TCard4         c4Cookie
            ,       TCommPortCfg&           cpcfgToFill
        )   final;

        tCIDLib::TVoid QueryFlags
        (
            const   tCIDLib::TCard4         c4Cookie
            ,       tCIDComm::EFlags&       eToFill
        )   final;

        tCIDLib::TVoid QueryLineStates
        (
            const   tCIDLib::TCard4         c4Cookie
            ,       tCIDLib::TBoolean&      bCTSOn
            ,       tCIDLib::TBoolean&      bDSROn
            ,       tCIDLib::TBoolean&      bRingOn
            ,       tCIDLib::TBoolean&      bRLSOn
        )   final;

        tCIDLib::TVoid QueryXChars
        (
            const   tCIDLib::TCard4         c4Cookie
            ,       tCIDLib::TCard4&        c4hOn
            ,       tCIDLib::TCard4&        c4Off
        )   final;

        tCIDLib::TVoid ReadBufMS
        (
            const   tCIDLib::TCard4         c4Cookie
            ,       tCIDLib::TCard4&        c4BufSz_mbufToFill
            ,       THeapBuf&               mbufToFill
            , const tCIDLib::TCard4         c4MaxBytes
            , const tCIDLib::TCard4         c4Wait
        )   final;

        tCIDLib::TVoid ReadBuf
        (
            const   tCIDLib::TCard4         c4Cookie
            ,       tCIDLib::TCard4&        c4BufSz_mbufToFill
            ,       THeapBuf&               mbufToFill
            , const tCIDLib::TCard4         c4MaxBytes
            , const tCIDLib::TEncodedTime   enctEnd
        )   final;

        tCIDLib::TVoid Reset
        (
            const   tCIDLib::TCard4         c4Cookie
        )   final;

        tCIDLib::TVoid SetBaudRate
        (
            const   tCIDLib::TCard4         c4Cookie
            , const tCIDLib::TCard4         c4ToSet
        )   final;

        tCIDLib::TVoid SetBreak
        (
            const   tCIDLib::TCard4         c4Cookie
            , const tCIDLib::TBoolean       bState
        )   final;

        tCIDLib::TVoid SetCfg
        (
            const   tCIDLib::TCard4         c4Cookie
            , const TCommPortCfg&           cpcfgPortCfg
        )   final;

        tCIDLib::TVoid SetDataBits
        (
            const   tCIDLib::TCard4         c4Cookie
            , const tCIDComm::EDataBits     eToSet
        )   final;

        tCIDLib::TVoid SetDTR
        (
            const   tCIDLib::TCard4         c4Cookie
            , const tCIDComm::EPortDTR      eToSet
        )   final;

        tCIDLib::TVoid SetEOFChar
        (
            const   tCIDLib::TCard4         c4Cookie
            , const tCIDLib::TCard4         c4ToSet
        )   final;

        tCIDLib::TVoid SetFlags
        (
            const   tCIDLib::TCard4         c4Cookie
            , const tCIDComm::EFlags        eToSet
            , const tCIDComm::EFlags        eMask
        )   final;

        tCIDLib::TVoid SetParity
        (
            const   tCIDLib::TCard4         c4Cookie
            , const tCIDComm::EParities     eToSet
        )   final;

        tCIDLib::TVoid SetRTS
        (
            const   tCIDLib::TCard4         c4Cookie
            , const tCIDComm::EPortRTS      eToSet
        )   final;

        tCIDLib::TVoid SetStopBits
        (
            const   tCIDLib::TCard4         c4Cookie
            , const tCIDComm::EStopBits     eToSet
        )   final;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid Initialize() final;

        tCIDLib::TVoid Terminate() final;


    private :
        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TRemSerialImpl,TRemSerialSrvServerBase)
};

