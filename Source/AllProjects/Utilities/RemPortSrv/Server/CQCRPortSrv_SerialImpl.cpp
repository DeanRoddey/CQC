//
// FILE NAME: CQCRPortSrv_SerialImpl.cpp
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
//  This file implements the server side of the remote serial port ORB
//  interface.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCRPortSrv.hpp"



// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TRemSerialImpl,TRemSerialSrvServerBase)



// ---------------------------------------------------------------------------
//  CLASS: TRemSerialImpl
// PREFIX: orbs
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TRemSerialImpl: Constructors and Destructor
// ---------------------------------------------------------------------------
TRemSerialImpl::TRemSerialImpl(const TOrbObjId& ooidToUse) :

    TRemSerialSrvServerBase(ooidToUse)
{
}

TRemSerialImpl::~TRemSerialImpl()
{
}

// ----------------------------------------------------------------------------
//  TRemSerialImpl: Public, inherited methods
// ----------------------------------------------------------------------------
tCIDLib::TBoolean TRemSerialImpl::bQueryCommPorts(TVector<TString>& colToFill)
{
    // Just delegate to our facility object
    return facCQCRPortSrv().bQueryCommPorts(colToFill);
}


tCIDLib::TBoolean TRemSerialImpl::bIsOpen(const tCIDLib::TCard4 c4Cookie)
{
    TCQCSerPortInfo* pspiCheck = facCQCRPortSrv().pspiFind(c4Cookie);
    return (pspiCheck->pcommPort() != 0);
}


tCIDLib::TBoolean TRemSerialImpl::bQueryCTSOn(const tCIDLib::TCard4 c4Cookie)
{
    TCQCSerPortInfo* pspiQuery = facCQCRPortSrv().pspiFind(c4Cookie);
    return pspiQuery->pcommPort()->bQueryCTSOn();
}


tCIDLib::TBoolean TRemSerialImpl::bQueryDSROn(const tCIDLib::TCard4 c4Cookie)
{
    TCQCSerPortInfo* pspiQuery = facCQCRPortSrv().pspiFind(c4Cookie);
    return pspiQuery->pcommPort()->bQueryDSROn();
}


tCIDLib::TCard4
TRemSerialImpl::c4Open( const   TString&                strPortPath
                        , const tCIDComm::EOpenFlags    eFlags
                        , const tCIDLib::TCard4         c4ReadBufSz
                        , const tCIDLib::TCard4         c4WriteBufSz)
{
    // Find the port info object by remote port path
    TCQCSerPortInfo* pspiOpen = facCQCRPortSrv().pspiFindByPath(strPortPath);

    //
    //  Make sure this is an allowed port, so that they can't somehow manage to slip in
    //  a path we didn't report as available.
    //
    if (!pspiOpen->bAllow())
    {
        facCQCRPortSrv().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kRPortSErrs::errcSPort_NotAllowed
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Authority
            , strPortPath
        );
    }

    // Try to open it
    pspiOpen->pcommPort()->Open(eFlags, c4ReadBufSz, c4WriteBufSz);

    // Return this guy's cookie
    return pspiOpen->c4Cookie();
}


tCIDLib::TCard4
TRemSerialImpl::c4WriteBufMS(const  tCIDLib::TCard4 c4Cookie
                            , const tCIDLib::TCard4 c4BufSz_mbufToWrite
                            , const THeapBuf&       mbufToWrite
                            , const tCIDLib::TCard4 c4Wait)
{
    TCQCSerPortInfo* pspiWrite = facCQCRPortSrv().pspiFind(c4Cookie);
    return pspiWrite->pcommPort()->c4WriteMBufMS
    (
        mbufToWrite, c4BufSz_mbufToWrite, c4Wait
    );
}


tCIDLib::TVoid TRemSerialImpl::CloseAllPorts()
{
    // Just delegate to the facility class
    facCQCRPortSrv().CloseAllPorts();
}


tCIDLib::TVoid TRemSerialImpl::Close(const tCIDLib::TCard4 c4Cookie)
{
    TCQCSerPortInfo* pspiClose = facCQCRPortSrv().pspiFind(c4Cookie);
    pspiClose->pcommPort()->Close();
}


tCIDLib::TVoid
TRemSerialImpl::Cycle(  const   tCIDLib::TCard4         c4Cookie
                        , const tCIDComm::EOpenFlags    eFlags
                        , const tCIDLib::TCard4         c4ReadBufSz
                        , const tCIDLib::TCard4         c4WriteBufSz)
{
    TCQCSerPortInfo* pspiCycle = facCQCRPortSrv().pspiFind(c4Cookie);
    pspiCycle->pcommPort()->Cycle(eFlags, c4ReadBufSz, c4WriteBufSz);
}


tCIDLib::TVoid TRemSerialImpl::FlushWriteBuffer(const tCIDLib::TCard4 c4Cookie)
{
    TCQCSerPortInfo* pspiFlush = facCQCRPortSrv().pspiFind(c4Cookie);
    pspiFlush->pcommPort()->FlushWriteBuffer();
}


tCIDLib::TVoid TRemSerialImpl::PurgeReadData(const tCIDLib::TCard4 c4Cookie)
{
    TCQCSerPortInfo* pspiPurge = facCQCRPortSrv().pspiFind(c4Cookie);
    pspiPurge->pcommPort()->PurgeReadData();
}


tCIDLib::TVoid TRemSerialImpl::PurgeWriteData(const tCIDLib::TCard4 c4Cookie)
{
    TCQCSerPortInfo* pspiPurge = facCQCRPortSrv().pspiFind(c4Cookie);
    pspiPurge->pcommPort()->PurgeWriteData();
}


tCIDLib::TVoid
TRemSerialImpl::QueryCfg(const  tCIDLib::TCard4 c4Cookie
                        ,       TCommPortCfg&   cpcfgToFill)
{
    TCQCSerPortInfo* pspiQuery = facCQCRPortSrv().pspiFind(c4Cookie);
    pspiQuery->pcommPort()->QueryCfg(cpcfgToFill);
}


tCIDLib::TVoid
TRemSerialImpl::QueryFlags( const   tCIDLib::TCard4     c4Cookie
                            ,       tCIDComm::EFlags&   eToFill)
{
    TCQCSerPortInfo* pspiQuery = facCQCRPortSrv().pspiFind(c4Cookie);
    eToFill = pspiQuery->pcommPort()->eFlags();
}


tCIDLib::TVoid
TRemSerialImpl::QueryLineStates(const   tCIDLib::TCard4     c4Cookie
                                ,       tCIDLib::TBoolean&  bCTSOn
                                ,       tCIDLib::TBoolean&  bDSROn
                                ,       tCIDLib::TBoolean&  bRingOn
                                ,       tCIDLib::TBoolean&  bRLSOn)
{
    TCQCSerPortInfo* pspiStates = facCQCRPortSrv().pspiFind(c4Cookie);
    pspiStates->pcommPort()->QueryLineStates(bCTSOn, bDSROn, bRingOn, bRLSOn);
}


tCIDLib::TVoid
TRemSerialImpl::QueryXChars(const   tCIDLib::TCard4     c4Cookie
                            ,       tCIDLib::TCard4&    c4On
                            ,       tCIDLib::TCard4&    c4Off)
{
    TCQCSerPortInfo* pspiQuery = facCQCRPortSrv().pspiFind(c4Cookie);
    tCIDLib::TSCh schOn;
    tCIDLib::TSCh schOff;
    pspiQuery->pcommPort()->QueryXChars(schOn, schOff);
    c4On = schOn;
    c4Off = schOff;
}


tCIDLib::TVoid
TRemSerialImpl::ReadBufMS(  const   tCIDLib::TCard4     c4Cookie
                            ,       tCIDLib::TCard4&    c4BytesRead
                            ,       THeapBuf&           mbufToFill
                            , const tCIDLib::TCard4     c4MaxBytes
                            , const tCIDLib::TCard4     c4Wait)
{
    TCQCSerPortInfo* pspiRead = facCQCRPortSrv().pspiFind(c4Cookie);
    c4BytesRead = pspiRead->pcommPort()->c4ReadMBufMS(mbufToFill, c4MaxBytes, c4Wait);
}


tCIDLib::TVoid
TRemSerialImpl::ReadBuf(const   tCIDLib::TCard4         c4Cookie
                        ,       tCIDLib::TCard4&        c4BytesRead
                        ,       THeapBuf&               mbufToFill
                        , const tCIDLib::TCard4         c4MaxBytes
                        , const tCIDLib::TEncodedTime   enctWait)
{
    TCQCSerPortInfo* pspiRead = facCQCRPortSrv().pspiFind(c4Cookie);
    c4BytesRead = pspiRead->pcommPort()->c4ReadMBuf(mbufToFill, c4MaxBytes, enctWait);
}


tCIDLib::TVoid TRemSerialImpl::Reset(const tCIDLib::TCard4 c4Cookie)
{
    TCQCSerPortInfo* pspiReset = facCQCRPortSrv().pspiFind(c4Cookie);
    pspiReset->pcommPort()->Reset();
}


tCIDLib::TVoid
TRemSerialImpl::SetBaudRate(const   tCIDLib::TCard4 c4Cookie
                            , const tCIDLib::TCard4 c4ToSet)
{
    TCQCSerPortInfo* pspiSet = facCQCRPortSrv().pspiFind(c4Cookie);
    pspiSet->pcommPort()->SetBaudRate(c4ToSet);
}


tCIDLib::TVoid
TRemSerialImpl::SetBreak(const  tCIDLib::TCard4     c4Cookie
                        , const tCIDLib::TBoolean   bState)
{
    TCQCSerPortInfo* pspiSet = facCQCRPortSrv().pspiFind(c4Cookie);
    pspiSet->pcommPort()->SetBreak(bState);
}


tCIDLib::TVoid
TRemSerialImpl::SetCfg( const   tCIDLib::TCard4 c4Cookie
                        , const TCommPortCfg&   cpcfgPortCfg)
{
    TCQCSerPortInfo* pspiSet = facCQCRPortSrv().pspiFind(c4Cookie);
    pspiSet->pcommPort()->SetCfg(cpcfgPortCfg);
}


tCIDLib::TVoid
TRemSerialImpl::SetDataBits(const   tCIDLib::TCard4     c4Cookie
                            , const tCIDComm::EDataBits eToSet)
{
    TCQCSerPortInfo* pspiSet = facCQCRPortSrv().pspiFind(c4Cookie);
    pspiSet->pcommPort()->SetDataBits(eToSet);
}


tCIDLib::TVoid
TRemSerialImpl::SetDTR( const   tCIDLib::TCard4     c4Cookie
                        , const tCIDComm::EPortDTR  eToSet)
{
    TCQCSerPortInfo* pspiSet = facCQCRPortSrv().pspiFind(c4Cookie);
    pspiSet->pcommPort()->SetDTR(eToSet);
}


tCIDLib::TVoid
TRemSerialImpl::SetEOFChar( const   tCIDLib::TCard4 c4Cookie
                            , const tCIDLib::TCard4 c4ToSet)
{
    TCQCSerPortInfo* pspiSet = facCQCRPortSrv().pspiFind(c4Cookie);
    pspiSet->pcommPort()->SetEOFChar(tCIDLib::TSCh(c4ToSet));
}


tCIDLib::TVoid
TRemSerialImpl::SetFlags(const  tCIDLib::TCard4     c4Cookie
                        , const tCIDComm::EFlags    eToSet
                        , const tCIDComm::EFlags    eMask)
{
    TCQCSerPortInfo* pspiSet = facCQCRPortSrv().pspiFind(c4Cookie);
    pspiSet->pcommPort()->SetFlags(eToSet, eMask);
}


tCIDLib::TVoid
TRemSerialImpl::SetParity(  const   tCIDLib::TCard4     c4Cookie
                            , const tCIDComm::EParities eToSet)
{
    TCQCSerPortInfo* pspiSet = facCQCRPortSrv().pspiFind(c4Cookie);
    pspiSet->pcommPort()->SetParity(eToSet);
}


tCIDLib::TVoid
TRemSerialImpl::SetRTS( const   tCIDLib::TCard4     c4Cookie
                        , const tCIDComm::EPortRTS  eToSet)
{
    TCQCSerPortInfo* pspiSet = facCQCRPortSrv().pspiFind(c4Cookie);
    pspiSet->pcommPort()->SetRTS(eToSet);
}


tCIDLib::TVoid
TRemSerialImpl::SetStopBits(const   tCIDLib::TCard4     c4Cookie
                            , const tCIDComm::EStopBits eToSet)
{
    TCQCSerPortInfo* pspiSet = facCQCRPortSrv().pspiFind(c4Cookie);
    pspiSet->pcommPort()->SetStopBits(eToSet);
}


// ---------------------------------------------------------------------------
//  TRemSerialImpl: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TRemSerialImpl::Initialize()
{
}


tCIDLib::TVoid TRemSerialImpl::Terminate()
{
}


