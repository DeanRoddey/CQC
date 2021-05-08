// ----------------------------------------------------------------------------
//  FILE: CQCKit_ClSrvClientProxy.cpp
//  DATE: Sat, May 01 11:54:22 2021 -0400
//    ID: 7EE18BDB7E607212-6455454A53F1497B
//
//  This file was generated by the Charmed Quark CIDIDL compiler. Do not make
//  changes by hand, because they will be lost if the file is regenerated.
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Includes
// ----------------------------------------------------------------------------
#include "CQCKit_.hpp"

// ----------------------------------------------------------------------------
// Magic macros
// ----------------------------------------------------------------------------
AdvRTTIDecls(TCQCClServiceClientProxy,TOrbClientBase)


// ----------------------------------------------------------------------------
// TCQCClServiceClientProxy: Public, static data
// ----------------------------------------------------------------------------
const TString TCQCClServiceClientProxy::strInterfaceId(L"7EE18BDB7E607212-6455454A53F1497B");

// ----------------------------------------------------------------------------
// TCQCClServiceClientProxy: Constructors and Destructor
// ----------------------------------------------------------------------------
TCQCClServiceClientProxy::TCQCClServiceClientProxy()
{
}

TCQCClServiceClientProxy::
TCQCClServiceClientProxy(const TOrbObjId& ooidSrc, const TString& strNSBinding)
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

TCQCClServiceClientProxy::~TCQCClServiceClientProxy()
{
}

// ----------------------------------------------------------------------------
// TCQCClServiceClientProxy: Public, non-virtual methods
// ----------------------------------------------------------------------------
tCIDLib::TBoolean TCQCClServiceClientProxy::bQueryRepoDB
(
    const TString& strRepoMon
    , const TString& strCurSerialNum
    , tCIDLib::TCard4& c4BufSz_mbufData
    , COP THeapBuf& mbufData)
{
    #pragma warning(suppress : 26494)
    tCIDLib::TBoolean retVal;
    TCmdQItem* pcqiToUse = pcqiGetCmdItem(ooidThis().oidKey());
    TOrbCmd& ocmdToUse = pcqiToUse->ocmdData();
    try
    {
        ocmdToUse.strmOut() << TString(L"bQueryRepoDB");
        ocmdToUse.strmOut() << strRepoMon;
        ocmdToUse.strmOut() << strCurSerialNum;
        Dispatch(30000, pcqiToUse);
        ocmdToUse.strmIn().Reset();
        ocmdToUse.strmIn() >> retVal;
        ocmdToUse.strmIn() >> c4BufSz_mbufData;
        ocmdToUse.strmIn().c4ReadBuffer(mbufData, c4BufSz_mbufData);
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

tCIDLib::TCard4 TCQCClServiceClientProxy::c4QueryRepoList
(
    COP TVector<TString>& colToFill)
{
    #pragma warning(suppress : 26494)
    tCIDLib::TCard4 retVal;
    TCmdQItem* pcqiToUse = pcqiGetCmdItem(ooidThis().oidKey());
    TOrbCmd& ocmdToUse = pcqiToUse->ocmdData();
    try
    {
        ocmdToUse.strmOut() << TString(L"c4QueryRepoList");
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

