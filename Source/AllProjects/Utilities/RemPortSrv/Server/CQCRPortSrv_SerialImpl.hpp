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
        );

        tCIDLib::TBoolean bIsOpen
        (
            const   tCIDLib::TCard4         c4Cookie
        );

        tCIDLib::TBoolean bQueryCTSOn
        (
            const   tCIDLib::TCard4         c4Cookie
        );

        tCIDLib::TBoolean bQueryDSROn
        (
            const   tCIDLib::TCard4         c4Cookie
        );

        tCIDLib::TCard4 c4Open
        (
            const   TString&                strPortPath
            , const tCIDComm::EOpenFlags    eFlags
            , const tCIDLib::TCard4         c4ReadBufSz
            , const tCIDLib::TCard4         c4WriteBufSz
        );

        tCIDLib::TCard4 c4WriteBufMS
        (
            const   tCIDLib::TCard4         c4Cookie
            , const tCIDLib::TCard4         c4BufSz_mbufToWrite
            , const THeapBuf&               mbufToWrite
            , const tCIDLib::TCard4         c4Wait
        );

        tCIDLib::TVoid Close
        (
            const   tCIDLib::TCard4         c4Cookie
        );

        tCIDLib::TVoid CloseAllPorts();

        tCIDLib::TVoid Cycle
        (
            const   tCIDLib::TCard4         c4Cookie
            , const tCIDComm::EOpenFlags    eFlags
            , const tCIDLib::TCard4         c4ReadBufSz
            , const tCIDLib::TCard4         c4WriteBufSz
        );

        tCIDLib::TVoid FlushWriteBuffer
        (
            const   tCIDLib::TCard4         c4Cookie
        );

        tCIDLib::TVoid PurgeReadData
        (
            const   tCIDLib::TCard4         c4Cookie
        );

        tCIDLib::TVoid PurgeWriteData
        (
            const   tCIDLib::TCard4         c4Cookie
        );

        tCIDLib::TVoid QueryCfg
        (
            const   tCIDLib::TCard4         c4Cookie
            ,       TCommPortCfg&           cpcfgToFill
        );

        tCIDLib::TVoid QueryFlags
        (
            const   tCIDLib::TCard4         c4Cookie
            ,       tCIDComm::EFlags&       eToFill
        );

        tCIDLib::TVoid QueryLineStates
        (
            const   tCIDLib::TCard4         c4Cookie
            ,       tCIDLib::TBoolean&      bCTSOn
            ,       tCIDLib::TBoolean&      bDSROn
            ,       tCIDLib::TBoolean&      bRingOn
            ,       tCIDLib::TBoolean&      bRLSOn
        );

        tCIDLib::TVoid QueryXChars
        (
            const   tCIDLib::TCard4         c4Cookie
            ,       tCIDLib::TCard4&        c4hOn
            ,       tCIDLib::TCard4&        c4Off
        );

        tCIDLib::TVoid ReadBufMS
        (
            const   tCIDLib::TCard4         c4Cookie
            ,       tCIDLib::TCard4&        c4BufSz_mbufToFill
            ,       THeapBuf&               mbufToFill
            , const tCIDLib::TCard4         c4MaxBytes
            , const tCIDLib::TCard4         c4Wait
        );

        tCIDLib::TVoid ReadBuf
        (
            const   tCIDLib::TCard4         c4Cookie
            ,       tCIDLib::TCard4&        c4BufSz_mbufToFill
            ,       THeapBuf&               mbufToFill
            , const tCIDLib::TCard4         c4MaxBytes
            , const tCIDLib::TEncodedTime   enctEnd
        );

        tCIDLib::TVoid Reset
        (
            const   tCIDLib::TCard4         c4Cookie
        );

        tCIDLib::TVoid SetBaudRate
        (
            const   tCIDLib::TCard4         c4Cookie
            , const tCIDLib::TCard4         c4ToSet
        );

        tCIDLib::TVoid SetBreak
        (
            const   tCIDLib::TCard4         c4Cookie
            , const tCIDLib::TBoolean       bState
        );

        tCIDLib::TVoid SetCfg
        (
            const   tCIDLib::TCard4         c4Cookie
            , const TCommPortCfg&           cpcfgPortCfg
        );

        tCIDLib::TVoid SetDataBits
        (
            const   tCIDLib::TCard4         c4Cookie
            , const tCIDComm::EDataBits     eToSet
        );

        tCIDLib::TVoid SetDTR
        (
            const   tCIDLib::TCard4         c4Cookie
            , const tCIDComm::EPortDTR      eToSet
        );

        tCIDLib::TVoid SetEOFChar
        (
            const   tCIDLib::TCard4         c4Cookie
            , const tCIDLib::TCard4         c4ToSet
        );

        tCIDLib::TVoid SetFlags
        (
            const   tCIDLib::TCard4         c4Cookie
            , const tCIDComm::EFlags        eToSet
            , const tCIDComm::EFlags        eMask
        );

        tCIDLib::TVoid SetParity
        (
            const   tCIDLib::TCard4         c4Cookie
            , const tCIDComm::EParities     eToSet
        );

        tCIDLib::TVoid SetRTS
        (
            const   tCIDLib::TCard4         c4Cookie
            , const tCIDComm::EPortRTS      eToSet
        );

        tCIDLib::TVoid SetStopBits
        (
            const   tCIDLib::TCard4         c4Cookie
            , const tCIDComm::EStopBits     eToSet
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid Initialize();

        tCIDLib::TVoid Terminate();


    private :
        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TRemSerialImpl,TRemSerialSrvServerBase)
};

