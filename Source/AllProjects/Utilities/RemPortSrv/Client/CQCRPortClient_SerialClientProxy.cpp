// ----------------------------------------------------------------------------
//  FILE: CQCRPortClient_SerialClientProxy.cpp
//  DATE: Tue, Jan 19 17:39:55 2021 -0500
//    ID: D7E08846B00F19C8-DE9D46C5B2C1AC0C
//
//  This file was generated by the Charmed Quark CIDIDL compiler. Do not make
//  changes by hand, because they will be lost if the file is regenerated.
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Includes
// ----------------------------------------------------------------------------
#include "CQCRPortClient_.hpp"

// ----------------------------------------------------------------------------
// Magic macros
// ----------------------------------------------------------------------------
AdvRTTIDecls(TRemSerialSrvClientProxy,TOrbClientBase)


// ----------------------------------------------------------------------------
// TRemSerialSrvClientProxy: Public, static data
// ----------------------------------------------------------------------------
const TString TRemSerialSrvClientProxy::strInterfaceId(L"D7E08846B00F19C8-DE9D46C5B2C1AC0C");
const TString TRemSerialSrvClientProxy::strInstanceId(L"45FBD494F407FBF2-E88F9E5DE9B35D44");
const TString TRemSerialSrvClientProxy::strSerialPathPrefix(L"/CQCRemSrv/COM");
const TString TRemSerialSrvClientProxy::strSerialFactoryId(L"CQCRemSerialServerFactory");



// ----------------------------------------------------------------------------
// TRemSerialSrvClientProxy: Constructors and Destructor
// ----------------------------------------------------------------------------
TRemSerialSrvClientProxy::TRemSerialSrvClientProxy()
{
}

TRemSerialSrvClientProxy::
TRemSerialSrvClientProxy(const TOrbObjId& ooidSrc, const TString& strNSBinding)
{
    #if CID_DEBUG_ON
    if (!ooidSrc.oidKey().bIsInterfaceId(strInterfaceId))
    {
        facCIDOrb().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kOrbErrs::errcClient_BadIntfId
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::BadParms
            , ooidSrc.oidKey().mhashInterface()
            , strInterfaceId
        );
    }
    #endif
    SetObjId(ooidSrc, strNSBinding, kCIDLib::False);
}

TRemSerialSrvClientProxy::~TRemSerialSrvClientProxy()
{
}

// ----------------------------------------------------------------------------
// TRemSerialSrvClientProxy: Public, non-virtual methods
// ----------------------------------------------------------------------------
tCIDLib::TBoolean TRemSerialSrvClientProxy::bQueryCommPorts
(
    TVector<TString>& colToFill)
{
    #pragma warning(suppress : 26494)
    tCIDLib::TBoolean retVal;
    TCmdQItem* pcqiToUse = pcqiGetCmdItem(ooidThis().oidKey());
    TOrbCmd& ocmdToUse = pcqiToUse->ocmdData();
    try
    {
        ocmdToUse.strmOut() << TString(L"bQueryCommPorts");
        Dispatch(30000, pcqiToUse);
        ocmdToUse.strmIn().Reset();
        ocmdToUse.strmIn() >> retVal;
        ocmdToUse.strmIn() >> colToFill;
        GiveBackCmdItem(pcqiToUse);
    }
    catch(TError& errToCatch)
    {
        GiveBackCmdItem(pcqiToUse);
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
    return retVal;
}

tCIDLib::TVoid TRemSerialSrvClientProxy::CloseAllPorts()
{
    TCmdQItem* pcqiToUse = pcqiGetCmdItem(ooidThis().oidKey());
    TOrbCmd& ocmdToUse = pcqiToUse->ocmdData();
    try
    {
        ocmdToUse.strmOut() << TString(L"CloseAllPorts");
        Dispatch(30000, pcqiToUse);
        ocmdToUse.strmIn().Reset();
        GiveBackCmdItem(pcqiToUse);
    }
    catch(TError& errToCatch)
    {
        GiveBackCmdItem(pcqiToUse);
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}

tCIDLib::TBoolean TRemSerialSrvClientProxy::bIsOpen
(
    const tCIDLib::TCard4 c4Cookie)
{
    #pragma warning(suppress : 26494)
    tCIDLib::TBoolean retVal;
    TCmdQItem* pcqiToUse = pcqiGetCmdItem(ooidThis().oidKey());
    TOrbCmd& ocmdToUse = pcqiToUse->ocmdData();
    try
    {
        ocmdToUse.strmOut() << TString(L"bIsOpen");
        ocmdToUse.strmOut() << c4Cookie;
        Dispatch(30000, pcqiToUse);
        ocmdToUse.strmIn().Reset();
        ocmdToUse.strmIn() >> retVal;
        GiveBackCmdItem(pcqiToUse);
    }
    catch(TError& errToCatch)
    {
        GiveBackCmdItem(pcqiToUse);
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
    return retVal;
}

tCIDLib::TBoolean TRemSerialSrvClientProxy::bQueryCTSOn
(
    const tCIDLib::TCard4 c4Cookie)
{
    #pragma warning(suppress : 26494)
    tCIDLib::TBoolean retVal;
    TCmdQItem* pcqiToUse = pcqiGetCmdItem(ooidThis().oidKey());
    TOrbCmd& ocmdToUse = pcqiToUse->ocmdData();
    try
    {
        ocmdToUse.strmOut() << TString(L"bQueryCTSOn");
        ocmdToUse.strmOut() << c4Cookie;
        Dispatch(30000, pcqiToUse);
        ocmdToUse.strmIn().Reset();
        ocmdToUse.strmIn() >> retVal;
        GiveBackCmdItem(pcqiToUse);
    }
    catch(TError& errToCatch)
    {
        GiveBackCmdItem(pcqiToUse);
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
    return retVal;
}

tCIDLib::TBoolean TRemSerialSrvClientProxy::bQueryDSROn
(
    const tCIDLib::TCard4 c4Cookie)
{
    #pragma warning(suppress : 26494)
    tCIDLib::TBoolean retVal;
    TCmdQItem* pcqiToUse = pcqiGetCmdItem(ooidThis().oidKey());
    TOrbCmd& ocmdToUse = pcqiToUse->ocmdData();
    try
    {
        ocmdToUse.strmOut() << TString(L"bQueryDSROn");
        ocmdToUse.strmOut() << c4Cookie;
        Dispatch(30000, pcqiToUse);
        ocmdToUse.strmIn().Reset();
        ocmdToUse.strmIn() >> retVal;
        GiveBackCmdItem(pcqiToUse);
    }
    catch(TError& errToCatch)
    {
        GiveBackCmdItem(pcqiToUse);
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
    return retVal;
}

tCIDLib::TCard4 TRemSerialSrvClientProxy::c4Open
(
    const TString& strPortPath
    , const tCIDComm::EOpenFlags eFlags
    , const tCIDLib::TCard4 c4ReadBufSz
    , const tCIDLib::TCard4 c4WriteBufSz)
{
    #pragma warning(suppress : 26494)
    tCIDLib::TCard4 retVal;
    TCmdQItem* pcqiToUse = pcqiGetCmdItem(ooidThis().oidKey());
    TOrbCmd& ocmdToUse = pcqiToUse->ocmdData();
    try
    {
        ocmdToUse.strmOut() << TString(L"c4Open");
        ocmdToUse.strmOut() << strPortPath;
        ocmdToUse.strmOut() << eFlags;
        ocmdToUse.strmOut() << c4ReadBufSz;
        ocmdToUse.strmOut() << c4WriteBufSz;
        Dispatch(30000, pcqiToUse);
        ocmdToUse.strmIn().Reset();
        ocmdToUse.strmIn() >> retVal;
        GiveBackCmdItem(pcqiToUse);
    }
    catch(TError& errToCatch)
    {
        GiveBackCmdItem(pcqiToUse);
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
    return retVal;
}

tCIDLib::TCard4 TRemSerialSrvClientProxy::c4WriteBufMS
(
    const tCIDLib::TCard4 c4Cookie
    , const tCIDLib::TCard4 c4BufSz_mbufToWrite
    , const THeapBuf& mbufToWrite
    , const tCIDLib::TCard4 c4Wait)
{
    #pragma warning(suppress : 26494)
    tCIDLib::TCard4 retVal;
    TCmdQItem* pcqiToUse = pcqiGetCmdItem(ooidThis().oidKey());
    TOrbCmd& ocmdToUse = pcqiToUse->ocmdData();
    try
    {
        ocmdToUse.strmOut() << TString(L"c4WriteBufMS");
        ocmdToUse.strmOut() << c4Cookie;
        ocmdToUse.strmOut() << c4BufSz_mbufToWrite;
        ocmdToUse.strmOut().c4WriteBuffer(mbufToWrite, c4BufSz_mbufToWrite);
        ocmdToUse.strmOut() << c4Wait;
        Dispatch(30000, pcqiToUse);
        ocmdToUse.strmIn().Reset();
        ocmdToUse.strmIn() >> retVal;
        GiveBackCmdItem(pcqiToUse);
    }
    catch(TError& errToCatch)
    {
        GiveBackCmdItem(pcqiToUse);
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
    return retVal;
}

tCIDLib::TVoid TRemSerialSrvClientProxy::Close
(
    const tCIDLib::TCard4 c4Cookie)
{
    TCmdQItem* pcqiToUse = pcqiGetCmdItem(ooidThis().oidKey());
    TOrbCmd& ocmdToUse = pcqiToUse->ocmdData();
    try
    {
        ocmdToUse.strmOut() << TString(L"Close");
        ocmdToUse.strmOut() << c4Cookie;
        Dispatch(30000, pcqiToUse);
        ocmdToUse.strmIn().Reset();
        GiveBackCmdItem(pcqiToUse);
    }
    catch(TError& errToCatch)
    {
        GiveBackCmdItem(pcqiToUse);
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}

tCIDLib::TVoid TRemSerialSrvClientProxy::Cycle
(
    const tCIDLib::TCard4 c4Cookie
    , const tCIDComm::EOpenFlags eFlags
    , const tCIDLib::TCard4 c4ReadBufSz
    , const tCIDLib::TCard4 c4WriteBufSz)
{
    TCmdQItem* pcqiToUse = pcqiGetCmdItem(ooidThis().oidKey());
    TOrbCmd& ocmdToUse = pcqiToUse->ocmdData();
    try
    {
        ocmdToUse.strmOut() << TString(L"Cycle");
        ocmdToUse.strmOut() << c4Cookie;
        ocmdToUse.strmOut() << eFlags;
        ocmdToUse.strmOut() << c4ReadBufSz;
        ocmdToUse.strmOut() << c4WriteBufSz;
        Dispatch(30000, pcqiToUse);
        ocmdToUse.strmIn().Reset();
        GiveBackCmdItem(pcqiToUse);
    }
    catch(TError& errToCatch)
    {
        GiveBackCmdItem(pcqiToUse);
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}

tCIDLib::TVoid TRemSerialSrvClientProxy::FlushWriteBuffer
(
    const tCIDLib::TCard4 c4Cookie)
{
    TCmdQItem* pcqiToUse = pcqiGetCmdItem(ooidThis().oidKey());
    TOrbCmd& ocmdToUse = pcqiToUse->ocmdData();
    try
    {
        ocmdToUse.strmOut() << TString(L"FlushWriteBuffer");
        ocmdToUse.strmOut() << c4Cookie;
        Dispatch(30000, pcqiToUse);
        ocmdToUse.strmIn().Reset();
        GiveBackCmdItem(pcqiToUse);
    }
    catch(TError& errToCatch)
    {
        GiveBackCmdItem(pcqiToUse);
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}

tCIDLib::TVoid TRemSerialSrvClientProxy::PurgeReadData
(
    const tCIDLib::TCard4 c4Cookie)
{
    TCmdQItem* pcqiToUse = pcqiGetCmdItem(ooidThis().oidKey());
    TOrbCmd& ocmdToUse = pcqiToUse->ocmdData();
    try
    {
        ocmdToUse.strmOut() << TString(L"PurgeReadData");
        ocmdToUse.strmOut() << c4Cookie;
        Dispatch(30000, pcqiToUse);
        ocmdToUse.strmIn().Reset();
        GiveBackCmdItem(pcqiToUse);
    }
    catch(TError& errToCatch)
    {
        GiveBackCmdItem(pcqiToUse);
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}

tCIDLib::TVoid TRemSerialSrvClientProxy::PurgeWriteData
(
    const tCIDLib::TCard4 c4Cookie)
{
    TCmdQItem* pcqiToUse = pcqiGetCmdItem(ooidThis().oidKey());
    TOrbCmd& ocmdToUse = pcqiToUse->ocmdData();
    try
    {
        ocmdToUse.strmOut() << TString(L"PurgeWriteData");
        ocmdToUse.strmOut() << c4Cookie;
        Dispatch(30000, pcqiToUse);
        ocmdToUse.strmIn().Reset();
        GiveBackCmdItem(pcqiToUse);
    }
    catch(TError& errToCatch)
    {
        GiveBackCmdItem(pcqiToUse);
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}

tCIDLib::TVoid TRemSerialSrvClientProxy::QueryCfg
(
    const tCIDLib::TCard4 c4Cookie
    , TCommPortCfg& cpcfgToFill)
{
    TCmdQItem* pcqiToUse = pcqiGetCmdItem(ooidThis().oidKey());
    TOrbCmd& ocmdToUse = pcqiToUse->ocmdData();
    try
    {
        ocmdToUse.strmOut() << TString(L"QueryCfg");
        ocmdToUse.strmOut() << c4Cookie;
        Dispatch(30000, pcqiToUse);
        ocmdToUse.strmIn().Reset();
        ocmdToUse.strmIn() >> cpcfgToFill;
        GiveBackCmdItem(pcqiToUse);
    }
    catch(TError& errToCatch)
    {
        GiveBackCmdItem(pcqiToUse);
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}

tCIDLib::TVoid TRemSerialSrvClientProxy::QueryFlags
(
    const tCIDLib::TCard4 c4Cookie
    , tCIDComm::EFlags& eToFill)
{
    TCmdQItem* pcqiToUse = pcqiGetCmdItem(ooidThis().oidKey());
    TOrbCmd& ocmdToUse = pcqiToUse->ocmdData();
    try
    {
        ocmdToUse.strmOut() << TString(L"QueryFlags");
        ocmdToUse.strmOut() << c4Cookie;
        Dispatch(30000, pcqiToUse);
        ocmdToUse.strmIn().Reset();
        ocmdToUse.strmIn() >> eToFill;
        GiveBackCmdItem(pcqiToUse);
    }
    catch(TError& errToCatch)
    {
        GiveBackCmdItem(pcqiToUse);
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}

tCIDLib::TVoid TRemSerialSrvClientProxy::QueryLineStates
(
    const tCIDLib::TCard4 c4Cookie
    , tCIDLib::TBoolean& bCTSOn
    , tCIDLib::TBoolean& bDSROn
    , tCIDLib::TBoolean& bRingOn
    , tCIDLib::TBoolean& bRLSOn)
{
    TCmdQItem* pcqiToUse = pcqiGetCmdItem(ooidThis().oidKey());
    TOrbCmd& ocmdToUse = pcqiToUse->ocmdData();
    try
    {
        ocmdToUse.strmOut() << TString(L"QueryLineStates");
        ocmdToUse.strmOut() << c4Cookie;
        Dispatch(30000, pcqiToUse);
        ocmdToUse.strmIn().Reset();
        ocmdToUse.strmIn() >> bCTSOn;
        ocmdToUse.strmIn() >> bDSROn;
        ocmdToUse.strmIn() >> bRingOn;
        ocmdToUse.strmIn() >> bRLSOn;
        GiveBackCmdItem(pcqiToUse);
    }
    catch(TError& errToCatch)
    {
        GiveBackCmdItem(pcqiToUse);
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}

tCIDLib::TVoid TRemSerialSrvClientProxy::QueryXChars
(
    const tCIDLib::TCard4 c4Cookie
    , tCIDLib::TCard4& c4On
    , tCIDLib::TCard4& c4Off)
{
    TCmdQItem* pcqiToUse = pcqiGetCmdItem(ooidThis().oidKey());
    TOrbCmd& ocmdToUse = pcqiToUse->ocmdData();
    try
    {
        ocmdToUse.strmOut() << TString(L"QueryXChars");
        ocmdToUse.strmOut() << c4Cookie;
        Dispatch(30000, pcqiToUse);
        ocmdToUse.strmIn().Reset();
        ocmdToUse.strmIn() >> c4On;
        ocmdToUse.strmIn() >> c4Off;
        GiveBackCmdItem(pcqiToUse);
    }
    catch(TError& errToCatch)
    {
        GiveBackCmdItem(pcqiToUse);
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}

tCIDLib::TVoid TRemSerialSrvClientProxy::ReadBufMS
(
    const tCIDLib::TCard4 c4Cookie
    , tCIDLib::TCard4& c4BufSz_mbufToFill
    , THeapBuf& mbufToFill
    , const tCIDLib::TCard4 c4MaxBytes
    , const tCIDLib::TCard4 c4Wait)
{
    TCmdQItem* pcqiToUse = pcqiGetCmdItem(ooidThis().oidKey());
    TOrbCmd& ocmdToUse = pcqiToUse->ocmdData();
    try
    {
        ocmdToUse.strmOut() << TString(L"ReadBufMS");
        ocmdToUse.strmOut() << c4Cookie;
        ocmdToUse.strmOut() << c4MaxBytes;
        ocmdToUse.strmOut() << c4Wait;
        Dispatch(30000, pcqiToUse);
        ocmdToUse.strmIn().Reset();
        ocmdToUse.strmIn() >> c4BufSz_mbufToFill;
        ocmdToUse.strmIn().c4ReadBuffer(mbufToFill, c4BufSz_mbufToFill);
        GiveBackCmdItem(pcqiToUse);
    }
    catch(TError& errToCatch)
    {
        GiveBackCmdItem(pcqiToUse);
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}

tCIDLib::TVoid TRemSerialSrvClientProxy::ReadBuf
(
    const tCIDLib::TCard4 c4Cookie
    , tCIDLib::TCard4& c4BufSz_mbufToFill
    , THeapBuf& mbufToFill
    , const tCIDLib::TCard4 c4MaxBytes
    , const tCIDLib::TCard8 enctEndTime)
{
    TCmdQItem* pcqiToUse = pcqiGetCmdItem(ooidThis().oidKey());
    TOrbCmd& ocmdToUse = pcqiToUse->ocmdData();
    try
    {
        ocmdToUse.strmOut() << TString(L"ReadBuf");
        ocmdToUse.strmOut() << c4Cookie;
        ocmdToUse.strmOut() << c4MaxBytes;
        ocmdToUse.strmOut() << enctEndTime;
        Dispatch(30000, pcqiToUse);
        ocmdToUse.strmIn().Reset();
        ocmdToUse.strmIn() >> c4BufSz_mbufToFill;
        ocmdToUse.strmIn().c4ReadBuffer(mbufToFill, c4BufSz_mbufToFill);
        GiveBackCmdItem(pcqiToUse);
    }
    catch(TError& errToCatch)
    {
        GiveBackCmdItem(pcqiToUse);
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}

tCIDLib::TVoid TRemSerialSrvClientProxy::Reset
(
    const tCIDLib::TCard4 c4Cookie)
{
    TCmdQItem* pcqiToUse = pcqiGetCmdItem(ooidThis().oidKey());
    TOrbCmd& ocmdToUse = pcqiToUse->ocmdData();
    try
    {
        ocmdToUse.strmOut() << TString(L"Reset");
        ocmdToUse.strmOut() << c4Cookie;
        Dispatch(30000, pcqiToUse);
        ocmdToUse.strmIn().Reset();
        GiveBackCmdItem(pcqiToUse);
    }
    catch(TError& errToCatch)
    {
        GiveBackCmdItem(pcqiToUse);
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}

tCIDLib::TVoid TRemSerialSrvClientProxy::SetBaudRate
(
    const tCIDLib::TCard4 c4Cookie
    , const tCIDLib::TCard4 c4ToSet)
{
    TCmdQItem* pcqiToUse = pcqiGetCmdItem(ooidThis().oidKey());
    TOrbCmd& ocmdToUse = pcqiToUse->ocmdData();
    try
    {
        ocmdToUse.strmOut() << TString(L"SetBaudRate");
        ocmdToUse.strmOut() << c4Cookie;
        ocmdToUse.strmOut() << c4ToSet;
        Dispatch(30000, pcqiToUse);
        ocmdToUse.strmIn().Reset();
        GiveBackCmdItem(pcqiToUse);
    }
    catch(TError& errToCatch)
    {
        GiveBackCmdItem(pcqiToUse);
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}

tCIDLib::TVoid TRemSerialSrvClientProxy::SetBreak
(
    const tCIDLib::TCard4 c4Cookie
    , const tCIDLib::TBoolean bState)
{
    TCmdQItem* pcqiToUse = pcqiGetCmdItem(ooidThis().oidKey());
    TOrbCmd& ocmdToUse = pcqiToUse->ocmdData();
    try
    {
        ocmdToUse.strmOut() << TString(L"SetBreak");
        ocmdToUse.strmOut() << c4Cookie;
        ocmdToUse.strmOut() << bState;
        Dispatch(30000, pcqiToUse);
        ocmdToUse.strmIn().Reset();
        GiveBackCmdItem(pcqiToUse);
    }
    catch(TError& errToCatch)
    {
        GiveBackCmdItem(pcqiToUse);
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}

tCIDLib::TVoid TRemSerialSrvClientProxy::SetCfg
(
    const tCIDLib::TCard4 c4Cookie
    , const TCommPortCfg& cpcfgPortCfg)
{
    TCmdQItem* pcqiToUse = pcqiGetCmdItem(ooidThis().oidKey());
    TOrbCmd& ocmdToUse = pcqiToUse->ocmdData();
    try
    {
        ocmdToUse.strmOut() << TString(L"SetCfg");
        ocmdToUse.strmOut() << c4Cookie;
        ocmdToUse.strmOut() << cpcfgPortCfg;
        Dispatch(30000, pcqiToUse);
        ocmdToUse.strmIn().Reset();
        GiveBackCmdItem(pcqiToUse);
    }
    catch(TError& errToCatch)
    {
        GiveBackCmdItem(pcqiToUse);
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}

tCIDLib::TVoid TRemSerialSrvClientProxy::SetDataBits
(
    const tCIDLib::TCard4 c4Cookie
    , const tCIDComm::EDataBits eToSet)
{
    TCmdQItem* pcqiToUse = pcqiGetCmdItem(ooidThis().oidKey());
    TOrbCmd& ocmdToUse = pcqiToUse->ocmdData();
    try
    {
        ocmdToUse.strmOut() << TString(L"SetDataBits");
        ocmdToUse.strmOut() << c4Cookie;
        ocmdToUse.strmOut() << eToSet;
        Dispatch(30000, pcqiToUse);
        ocmdToUse.strmIn().Reset();
        GiveBackCmdItem(pcqiToUse);
    }
    catch(TError& errToCatch)
    {
        GiveBackCmdItem(pcqiToUse);
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}

tCIDLib::TVoid TRemSerialSrvClientProxy::SetDTR
(
    const tCIDLib::TCard4 c4Cookie
    , const tCIDComm::EPortDTR eToSet)
{
    TCmdQItem* pcqiToUse = pcqiGetCmdItem(ooidThis().oidKey());
    TOrbCmd& ocmdToUse = pcqiToUse->ocmdData();
    try
    {
        ocmdToUse.strmOut() << TString(L"SetDTR");
        ocmdToUse.strmOut() << c4Cookie;
        ocmdToUse.strmOut() << eToSet;
        Dispatch(30000, pcqiToUse);
        ocmdToUse.strmIn().Reset();
        GiveBackCmdItem(pcqiToUse);
    }
    catch(TError& errToCatch)
    {
        GiveBackCmdItem(pcqiToUse);
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}

tCIDLib::TVoid TRemSerialSrvClientProxy::SetEOFChar
(
    const tCIDLib::TCard4 c4Cookie
    , const tCIDLib::TCard4 c4ToSet)
{
    TCmdQItem* pcqiToUse = pcqiGetCmdItem(ooidThis().oidKey());
    TOrbCmd& ocmdToUse = pcqiToUse->ocmdData();
    try
    {
        ocmdToUse.strmOut() << TString(L"SetEOFChar");
        ocmdToUse.strmOut() << c4Cookie;
        ocmdToUse.strmOut() << c4ToSet;
        Dispatch(30000, pcqiToUse);
        ocmdToUse.strmIn().Reset();
        GiveBackCmdItem(pcqiToUse);
    }
    catch(TError& errToCatch)
    {
        GiveBackCmdItem(pcqiToUse);
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}

tCIDLib::TVoid TRemSerialSrvClientProxy::SetFlags
(
    const tCIDLib::TCard4 c4Cookie
    , const tCIDComm::EFlags eToSet
    , const tCIDComm::EFlags eMask)
{
    TCmdQItem* pcqiToUse = pcqiGetCmdItem(ooidThis().oidKey());
    TOrbCmd& ocmdToUse = pcqiToUse->ocmdData();
    try
    {
        ocmdToUse.strmOut() << TString(L"SetFlags");
        ocmdToUse.strmOut() << c4Cookie;
        ocmdToUse.strmOut() << eToSet;
        ocmdToUse.strmOut() << eMask;
        Dispatch(30000, pcqiToUse);
        ocmdToUse.strmIn().Reset();
        GiveBackCmdItem(pcqiToUse);
    }
    catch(TError& errToCatch)
    {
        GiveBackCmdItem(pcqiToUse);
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}

tCIDLib::TVoid TRemSerialSrvClientProxy::SetParity
(
    const tCIDLib::TCard4 c4Cookie
    , const tCIDComm::EParities eToSet)
{
    TCmdQItem* pcqiToUse = pcqiGetCmdItem(ooidThis().oidKey());
    TOrbCmd& ocmdToUse = pcqiToUse->ocmdData();
    try
    {
        ocmdToUse.strmOut() << TString(L"SetParity");
        ocmdToUse.strmOut() << c4Cookie;
        ocmdToUse.strmOut() << eToSet;
        Dispatch(30000, pcqiToUse);
        ocmdToUse.strmIn().Reset();
        GiveBackCmdItem(pcqiToUse);
    }
    catch(TError& errToCatch)
    {
        GiveBackCmdItem(pcqiToUse);
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}

tCIDLib::TVoid TRemSerialSrvClientProxy::SetRTS
(
    const tCIDLib::TCard4 c4Cookie
    , const tCIDComm::EPortRTS eToSet)
{
    TCmdQItem* pcqiToUse = pcqiGetCmdItem(ooidThis().oidKey());
    TOrbCmd& ocmdToUse = pcqiToUse->ocmdData();
    try
    {
        ocmdToUse.strmOut() << TString(L"SetRTS");
        ocmdToUse.strmOut() << c4Cookie;
        ocmdToUse.strmOut() << eToSet;
        Dispatch(30000, pcqiToUse);
        ocmdToUse.strmIn().Reset();
        GiveBackCmdItem(pcqiToUse);
    }
    catch(TError& errToCatch)
    {
        GiveBackCmdItem(pcqiToUse);
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}

tCIDLib::TVoid TRemSerialSrvClientProxy::SetStopBits
(
    const tCIDLib::TCard4 c4Cookie
    , const tCIDComm::EStopBits eToSet)
{
    TCmdQItem* pcqiToUse = pcqiGetCmdItem(ooidThis().oidKey());
    TOrbCmd& ocmdToUse = pcqiToUse->ocmdData();
    try
    {
        ocmdToUse.strmOut() << TString(L"SetStopBits");
        ocmdToUse.strmOut() << c4Cookie;
        ocmdToUse.strmOut() << eToSet;
        Dispatch(30000, pcqiToUse);
        ocmdToUse.strmIn().Reset();
        GiveBackCmdItem(pcqiToUse);
    }
    catch(TError& errToCatch)
    {
        GiveBackCmdItem(pcqiToUse);
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}

