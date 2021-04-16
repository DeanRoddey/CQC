// ----------------------------------------------------------------------------
//  FILE: CQCKit_InstClientProxy.cpp
//  DATE: Fri, Feb 12 21:14:14 2021 -0500
//    ID: 825C0E370D8C5D6C-0B3B6D941AB56445
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
AdvRTTIDecls(TCQCInstClientProxy,TOrbClientBase)


// ----------------------------------------------------------------------------
// TCQCInstClientProxy: Public, static data
// ----------------------------------------------------------------------------
const TString TCQCInstClientProxy::strInterfaceId(L"825C0E370D8C5D6C-0B3B6D941AB56445");
const TString TCQCInstClientProxy::strBinding(L"/CQC/CQCDataServer/InstServerObj");



// ----------------------------------------------------------------------------
// TCQCInstClientProxy: Constructors and Destructor
// ----------------------------------------------------------------------------
TCQCInstClientProxy::TCQCInstClientProxy()
{
}

TCQCInstClientProxy::
TCQCInstClientProxy(const TOrbObjId& ooidSrc, const TString& strNSBinding)
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

TCQCInstClientProxy::~TCQCInstClientProxy()
{
}

// ----------------------------------------------------------------------------
// TCQCInstClientProxy: Public, non-virtual methods
// ----------------------------------------------------------------------------
tCIDLib::TBoolean TCQCInstClientProxy::bAddOrUpdateCheatSheet
(
    const TString& strUserName
    , CIOP tCIDLib::TCard4& c4SerialNum
    , const TCQCActVarList& cavlCheatSheet
    , const TCQCSecToken& sectUser)
{
    #pragma warning(suppress : 26494)
    tCIDLib::TBoolean retVal;
    TCmdQItem* pcqiToUse = pcqiGetCmdItem(ooidThis().oidKey());
    TOrbCmd& ocmdToUse = pcqiToUse->ocmdData();
    try
    {
        ocmdToUse.strmOut() << TString(L"bAddOrUpdateCheatSheet");
        ocmdToUse.strmOut() << strUserName;
        ocmdToUse.strmOut() << c4SerialNum;
        ocmdToUse.strmOut() << cavlCheatSheet;
        ocmdToUse.strmOut() << sectUser;
        Dispatch(30000, pcqiToUse);
        ocmdToUse.strmIn().Reset();
        ocmdToUse.strmIn() >> retVal;
        ocmdToUse.strmIn() >> c4SerialNum;
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

tCIDLib::TBoolean TCQCInstClientProxy::bUpdateEmailAccount
(
    const TCQCEMailAccount& emacctToAdd
    , CIOP tCIDLib::TCard4& c4SerialNum
    , const TCQCSecToken& sectUser
    , const tCIDLib::TBoolean bMustBeNew)
{
    #pragma warning(suppress : 26494)
    tCIDLib::TBoolean retVal;
    TCmdQItem* pcqiToUse = pcqiGetCmdItem(ooidThis().oidKey());
    TOrbCmd& ocmdToUse = pcqiToUse->ocmdData();
    try
    {
        ocmdToUse.strmOut() << TString(L"bUpdateEmailAccount");
        ocmdToUse.strmOut() << emacctToAdd;
        ocmdToUse.strmOut() << c4SerialNum;
        ocmdToUse.strmOut() << sectUser;
        ocmdToUse.strmOut() << bMustBeNew;
        Dispatch(30000, pcqiToUse);
        ocmdToUse.strmIn().Reset();
        ocmdToUse.strmIn() >> retVal;
        ocmdToUse.strmIn() >> c4SerialNum;
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

tCIDLib::TBoolean TCQCInstClientProxy::bAddOrUpdateGC100Cfg
(
    const tCIDLib::TCard4 c4BufSz_mbufToSet
    , const THeapBuf& mbufToSet
    , CIOP tCIDLib::TCard4& c4SerialNum
    , const TCQCSecToken& sectUser)
{
    #pragma warning(suppress : 26494)
    tCIDLib::TBoolean retVal;
    TCmdQItem* pcqiToUse = pcqiGetCmdItem(ooidThis().oidKey());
    TOrbCmd& ocmdToUse = pcqiToUse->ocmdData();
    try
    {
        ocmdToUse.strmOut() << TString(L"bAddOrUpdateGC100Cfg");
        ocmdToUse.strmOut() << c4BufSz_mbufToSet;
        ocmdToUse.strmOut().c4WriteBuffer(mbufToSet, c4BufSz_mbufToSet);
        ocmdToUse.strmOut() << c4SerialNum;
        ocmdToUse.strmOut() << sectUser;
        Dispatch(30000, pcqiToUse);
        ocmdToUse.strmIn().Reset();
        ocmdToUse.strmIn() >> retVal;
        ocmdToUse.strmIn() >> c4SerialNum;
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

tCIDLib::TBoolean TCQCInstClientProxy::bAddOrUpdateJAPCfg
(
    const tCIDLib::TCard4 c4BufSz_mbufToFill
    , const THeapBuf& mbufToFill
    , CIOP tCIDLib::TCard4& c4SerialNum
    , const TCQCSecToken& sectUser)
{
    #pragma warning(suppress : 26494)
    tCIDLib::TBoolean retVal;
    TCmdQItem* pcqiToUse = pcqiGetCmdItem(ooidThis().oidKey());
    TOrbCmd& ocmdToUse = pcqiToUse->ocmdData();
    try
    {
        ocmdToUse.strmOut() << TString(L"bAddOrUpdateJAPCfg");
        ocmdToUse.strmOut() << c4BufSz_mbufToFill;
        ocmdToUse.strmOut().c4WriteBuffer(mbufToFill, c4BufSz_mbufToFill);
        ocmdToUse.strmOut() << c4SerialNum;
        ocmdToUse.strmOut() << sectUser;
        Dispatch(30000, pcqiToUse);
        ocmdToUse.strmIn().Reset();
        ocmdToUse.strmIn() >> retVal;
        ocmdToUse.strmIn() >> c4SerialNum;
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

tCIDLib::TBoolean TCQCInstClientProxy::bDeleteEMailAccount
(
    const TString& strAccount
    , const TCQCSecToken& sectUser)
{
    #pragma warning(suppress : 26494)
    tCIDLib::TBoolean retVal;
    TCmdQItem* pcqiToUse = pcqiGetCmdItem(ooidThis().oidKey());
    TOrbCmd& ocmdToUse = pcqiToUse->ocmdData();
    try
    {
        ocmdToUse.strmOut() << TString(L"bDeleteEMailAccount");
        ocmdToUse.strmOut() << strAccount;
        ocmdToUse.strmOut() << sectUser;
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

tCIDLib::TBoolean TCQCInstClientProxy::bDrvCfgDriverPresent
(
    const TString& strMoniker
    , COP TString& strHost
    , const TCQCSecToken& sectUser)
{
    #pragma warning(suppress : 26494)
    tCIDLib::TBoolean retVal;
    TCmdQItem* pcqiToUse = pcqiGetCmdItem(ooidThis().oidKey());
    TOrbCmd& ocmdToUse = pcqiToUse->ocmdData();
    try
    {
        ocmdToUse.strmOut() << TString(L"bDrvCfgDriverPresent");
        ocmdToUse.strmOut() << strMoniker;
        ocmdToUse.strmOut() << sectUser;
        Dispatch(30000, pcqiToUse);
        ocmdToUse.strmIn().Reset();
        ocmdToUse.strmIn() >> retVal;
        ocmdToUse.strmIn() >> strHost;
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

tCIDLib::TBoolean TCQCInstClientProxy::bDrvCfgMoveDrivers
(
    const TString& strOldHost
    , const TString& strNewHost
    , COP TVector<TString>& colMsgs
    , const TCQCSecToken& sectUser)
{
    #pragma warning(suppress : 26494)
    tCIDLib::TBoolean retVal;
    TCmdQItem* pcqiToUse = pcqiGetCmdItem(ooidThis().oidKey());
    TOrbCmd& ocmdToUse = pcqiToUse->ocmdData();
    try
    {
        ocmdToUse.strmOut() << TString(L"bDrvCfgMoveDrivers");
        ocmdToUse.strmOut() << strOldHost;
        ocmdToUse.strmOut() << strNewHost;
        ocmdToUse.strmOut() << sectUser;
        Dispatch(30000, pcqiToUse);
        ocmdToUse.strmIn().Reset();
        ocmdToUse.strmIn() >> retVal;
        ocmdToUse.strmIn() >> colMsgs;
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

tCIDLib::TBoolean TCQCInstClientProxy::bDrvCfgQueryConfig
(
    CIOP tCIDLib::TCard4& c4Version
    , const TString& strMoniker
    , CIOP TString& strHost
    , TCQCDriverObjCfg& cqcdcToFill
    , const TCQCSecToken& sectUser)
{
    #pragma warning(suppress : 26494)
    tCIDLib::TBoolean retVal;
    TCmdQItem* pcqiToUse = pcqiGetCmdItem(ooidThis().oidKey());
    TOrbCmd& ocmdToUse = pcqiToUse->ocmdData();
    try
    {
        ocmdToUse.strmOut() << TString(L"bDrvCfgQueryConfig");
        ocmdToUse.strmOut() << c4Version;
        ocmdToUse.strmOut() << strMoniker;
        ocmdToUse.strmOut() << strHost;
        ocmdToUse.strmOut() << sectUser;
        Dispatch(10000, pcqiToUse);
        ocmdToUse.strmIn().Reset();
        ocmdToUse.strmIn() >> retVal;
        if (retVal)
        {
            ocmdToUse.strmIn() >> c4Version;
            ocmdToUse.strmIn() >> strHost;
            ocmdToUse.strmIn() >> cqcdcToFill;
        }
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

tCIDLib::TBoolean TCQCInstClientProxy::bDrvCfgQueryHostConfigs
(
    CIOP tCIDLib::TCard4& c4Version
    , const TString& strHost
    , TVector<TCQCDriverObjCfg>& colCfgs
    , TFundVector<tCIDLib::TCard4>& fcolCfgVers
    , const TCQCSecToken& sectUser)
{
    #pragma warning(suppress : 26494)
    tCIDLib::TBoolean retVal;
    TCmdQItem* pcqiToUse = pcqiGetCmdItem(ooidThis().oidKey());
    TOrbCmd& ocmdToUse = pcqiToUse->ocmdData();
    try
    {
        ocmdToUse.strmOut() << TString(L"bDrvCfgQueryHostConfigs");
        ocmdToUse.strmOut() << c4Version;
        ocmdToUse.strmOut() << strHost;
        ocmdToUse.strmOut() << sectUser;
        Dispatch(10000, pcqiToUse);
        ocmdToUse.strmIn().Reset();
        ocmdToUse.strmIn() >> retVal;
        if (retVal)
        {
            ocmdToUse.strmIn() >> c4Version;
            ocmdToUse.strmIn() >> colCfgs;
            ocmdToUse.strmIn() >> fcolCfgVers;
        }
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

tCIDLib::TBoolean TCQCInstClientProxy::bDrvCfgQueryHosts
(
    COP TVector<TString>& colHosts
    , const TCQCSecToken& sectUser)
{
    #pragma warning(suppress : 26494)
    tCIDLib::TBoolean retVal;
    TCmdQItem* pcqiToUse = pcqiGetCmdItem(ooidThis().oidKey());
    TOrbCmd& ocmdToUse = pcqiToUse->ocmdData();
    try
    {
        ocmdToUse.strmOut() << TString(L"bDrvCfgQueryHosts");
        ocmdToUse.strmOut() << sectUser;
        Dispatch(30000, pcqiToUse);
        ocmdToUse.strmIn().Reset();
        ocmdToUse.strmIn() >> retVal;
        ocmdToUse.strmIn() >> colHosts;
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

tCIDLib::TBoolean TCQCInstClientProxy::bDrvCfgQueryMonList
(
    CIOP tCIDLib::TCard4& c4Version
    , const TString& strHost
    , TVector<TString>& colToFill
    , TFundVector<tCIDLib::TCard4>& fcolVers
    , TFundVector<tCIDLib::TBoolean>& fcolClDrvs
    , const TCQCSecToken& sectUser)
{
    #pragma warning(suppress : 26494)
    tCIDLib::TBoolean retVal;
    TCmdQItem* pcqiToUse = pcqiGetCmdItem(ooidThis().oidKey());
    TOrbCmd& ocmdToUse = pcqiToUse->ocmdData();
    try
    {
        ocmdToUse.strmOut() << TString(L"bDrvCfgQueryMonList");
        ocmdToUse.strmOut() << c4Version;
        ocmdToUse.strmOut() << strHost;
        ocmdToUse.strmOut() << sectUser;
        Dispatch(10000, pcqiToUse);
        ocmdToUse.strmIn().Reset();
        ocmdToUse.strmIn() >> retVal;
        if (retVal)
        {
            ocmdToUse.strmIn() >> c4Version;
            ocmdToUse.strmIn() >> colToFill;
            ocmdToUse.strmIn() >> fcolVers;
            ocmdToUse.strmIn() >> fcolClDrvs;
        }
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

tCIDLib::TBoolean TCQCInstClientProxy::bDrvCfgQueryMonList
(
    COP TVector<TKeyValuePair>& colToFill
    , const TCQCSecToken& sectUser)
{
    #pragma warning(suppress : 26494)
    tCIDLib::TBoolean retVal;
    TCmdQItem* pcqiToUse = pcqiGetCmdItem(ooidThis().oidKey());
    TOrbCmd& ocmdToUse = pcqiToUse->ocmdData();
    try
    {
        ocmdToUse.strmOut() << TString(L"bDrvCfgQueryMonList1");
        ocmdToUse.strmOut() << sectUser;
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

tCIDLib::TBoolean TCQCInstClientProxy::bDrvCfgSetPause
(
    const TString& strMoniker
    , const tCIDLib::TBoolean bNewState
    , COP tCIDLib::TCard4& c4NewVersion
    , const TCQCSecToken& sectUser)
{
    #pragma warning(suppress : 26494)
    tCIDLib::TBoolean retVal;
    TCmdQItem* pcqiToUse = pcqiGetCmdItem(ooidThis().oidKey());
    TOrbCmd& ocmdToUse = pcqiToUse->ocmdData();
    try
    {
        ocmdToUse.strmOut() << TString(L"bDrvCfgSetPause");
        ocmdToUse.strmOut() << strMoniker;
        ocmdToUse.strmOut() << bNewState;
        ocmdToUse.strmOut() << sectUser;
        Dispatch(30000, pcqiToUse);
        ocmdToUse.strmIn().Reset();
        ocmdToUse.strmIn() >> retVal;
        ocmdToUse.strmIn() >> c4NewVersion;
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

tCIDLib::TBoolean TCQCInstClientProxy::bQueryCheatSheet
(
    CIOP tCIDLib::TCard4& c4SerialNum
    , tCIDLib::ELoadRes& eLoadRes
    , const TString& strUserName
    , TCQCActVarList& cavlCheatSheet
    , const TCQCSecToken& sectUser)
{
    #pragma warning(suppress : 26494)
    tCIDLib::TBoolean retVal;
    TCmdQItem* pcqiToUse = pcqiGetCmdItem(ooidThis().oidKey());
    TOrbCmd& ocmdToUse = pcqiToUse->ocmdData();
    try
    {
        ocmdToUse.strmOut() << TString(L"bQueryCheatSheet");
        ocmdToUse.strmOut() << c4SerialNum;
        ocmdToUse.strmOut() << strUserName;
        ocmdToUse.strmOut() << sectUser;
        Dispatch(10000, pcqiToUse);
        ocmdToUse.strmIn().Reset();
        ocmdToUse.strmIn() >> retVal;
        if (retVal)
        {
            ocmdToUse.strmIn() >> c4SerialNum;
            ocmdToUse.strmIn() >> eLoadRes;
            ocmdToUse.strmIn() >> cavlCheatSheet;
        }
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

tCIDLib::TBoolean TCQCInstClientProxy::bQueryDrvManifest
(
    const TString& strMake
    , const TString& strModel
    , COP TCQCDriverCfg& cqcdcToFill)
{
    #pragma warning(suppress : 26494)
    tCIDLib::TBoolean retVal;
    TCmdQItem* pcqiToUse = pcqiGetCmdItem(ooidThis().oidKey());
    TOrbCmd& ocmdToUse = pcqiToUse->ocmdData();
    try
    {
        ocmdToUse.strmOut() << TString(L"bQueryDrvManifest");
        ocmdToUse.strmOut() << strMake;
        ocmdToUse.strmOut() << strModel;
        Dispatch(30000, pcqiToUse);
        ocmdToUse.strmIn().Reset();
        ocmdToUse.strmIn() >> retVal;
        ocmdToUse.strmIn() >> cqcdcToFill;
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

tCIDLib::TBoolean TCQCInstClientProxy::bQueryEmailAccount
(
    const TString& strAccount
    , COP TCQCEMailAccount& emacctToFill
    , const tCIDLib::TBoolean bThrowIfNot
    , const TCQCSecToken& sectUser)
{
    #pragma warning(suppress : 26494)
    tCIDLib::TBoolean retVal;
    TCmdQItem* pcqiToUse = pcqiGetCmdItem(ooidThis().oidKey());
    TOrbCmd& ocmdToUse = pcqiToUse->ocmdData();
    try
    {
        ocmdToUse.strmOut() << TString(L"bQueryEmailAccount");
        ocmdToUse.strmOut() << strAccount;
        ocmdToUse.strmOut() << bThrowIfNot;
        ocmdToUse.strmOut() << sectUser;
        Dispatch(30000, pcqiToUse);
        ocmdToUse.strmIn().Reset();
        ocmdToUse.strmIn() >> retVal;
        ocmdToUse.strmIn() >> emacctToFill;
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

tCIDLib::TBoolean TCQCInstClientProxy::bQueryEMailAccountNames
(
    COP TVector<TString>& colToFill
    , const TCQCSecToken& sectUser)
{
    #pragma warning(suppress : 26494)
    tCIDLib::TBoolean retVal;
    TCmdQItem* pcqiToUse = pcqiGetCmdItem(ooidThis().oidKey());
    TOrbCmd& ocmdToUse = pcqiToUse->ocmdData();
    try
    {
        ocmdToUse.strmOut() << TString(L"bQueryEMailAccountNames");
        ocmdToUse.strmOut() << sectUser;
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

tCIDLib::TBoolean TCQCInstClientProxy::bQueryKeyMap
(
    CIOP tCIDLib::TCard4& c4SerialNum
    , tCIDLib::ELoadRes& eLoadRes
    , const TString& strUserName
    , TCQCKeyMap& kmToFill
    , const TCQCSecToken& sectUser)
{
    #pragma warning(suppress : 26494)
    tCIDLib::TBoolean retVal;
    TCmdQItem* pcqiToUse = pcqiGetCmdItem(ooidThis().oidKey());
    TOrbCmd& ocmdToUse = pcqiToUse->ocmdData();
    try
    {
        ocmdToUse.strmOut() << TString(L"bQueryKeyMap");
        ocmdToUse.strmOut() << c4SerialNum;
        ocmdToUse.strmOut() << strUserName;
        ocmdToUse.strmOut() << sectUser;
        Dispatch(10000, pcqiToUse);
        ocmdToUse.strmIn().Reset();
        ocmdToUse.strmIn() >> retVal;
        if (retVal)
        {
            ocmdToUse.strmIn() >> c4SerialNum;
            ocmdToUse.strmIn() >> eLoadRes;
            ocmdToUse.strmIn() >> kmToFill;
        }
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

tCIDLib::TBoolean TCQCInstClientProxy::bQueryLocationInfo
(
    COP tCIDLib::TFloat8& f8Lat
    , COP tCIDLib::TFloat8& f8Long
    , const TCQCSecToken& sectUser)
{
    #pragma warning(suppress : 26494)
    tCIDLib::TBoolean retVal;
    TCmdQItem* pcqiToUse = pcqiGetCmdItem(ooidThis().oidKey());
    TOrbCmd& ocmdToUse = pcqiToUse->ocmdData();
    try
    {
        ocmdToUse.strmOut() << TString(L"bQueryLocationInfo");
        ocmdToUse.strmOut() << sectUser;
        Dispatch(30000, pcqiToUse);
        ocmdToUse.strmIn().Reset();
        ocmdToUse.strmIn() >> retVal;
        ocmdToUse.strmIn() >> f8Lat;
        ocmdToUse.strmIn() >> f8Long;
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

tCIDLib::TBoolean TCQCInstClientProxy::bQueryProtoFile
(
    const TString& strFileName
    , tCIDLib::TCard4& c4BufSz_mbufToFill
    , COP THeapBuf& mbufToFill)
{
    #pragma warning(suppress : 26494)
    tCIDLib::TBoolean retVal;
    TCmdQItem* pcqiToUse = pcqiGetCmdItem(ooidThis().oidKey());
    TOrbCmd& ocmdToUse = pcqiToUse->ocmdData();
    try
    {
        ocmdToUse.strmOut() << TString(L"bQueryProtoFile");
        ocmdToUse.strmOut() << strFileName;
        Dispatch(30000, pcqiToUse);
        ocmdToUse.strmIn().Reset();
        ocmdToUse.strmIn() >> retVal;
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
    return retVal;
}

tCIDLib::TBoolean TCQCInstClientProxy::bQueryZWDIIndex
(
    tCIDLib::TCard4& c4BufSz_mbufIndex
    , COP THeapBuf& mbufIndex
    , tCIDLib::TCard4& c4BufSz_mbufDTD
    , COP THeapBuf& mbufDTD)
{
    #pragma warning(suppress : 26494)
    tCIDLib::TBoolean retVal;
    TCmdQItem* pcqiToUse = pcqiGetCmdItem(ooidThis().oidKey());
    TOrbCmd& ocmdToUse = pcqiToUse->ocmdData();
    try
    {
        ocmdToUse.strmOut() << TString(L"bQueryZWDIIndex");
        Dispatch(30000, pcqiToUse);
        ocmdToUse.strmIn().Reset();
        ocmdToUse.strmIn() >> retVal;
        ocmdToUse.strmIn() >> c4BufSz_mbufIndex;
        ocmdToUse.strmIn().c4ReadBuffer(mbufIndex, c4BufSz_mbufIndex);
        ocmdToUse.strmIn() >> c4BufSz_mbufDTD;
        ocmdToUse.strmIn().c4ReadBuffer(mbufDTD, c4BufSz_mbufDTD);
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

tCIDLib::TBoolean TCQCInstClientProxy::bQueryZWDIFile
(
    const TString& strFileName
    , tCIDLib::TCard4& c4BufSz_mbufData
    , COP THeapBuf& mbufData)
{
    #pragma warning(suppress : 26494)
    tCIDLib::TBoolean retVal;
    TCmdQItem* pcqiToUse = pcqiGetCmdItem(ooidThis().oidKey());
    TOrbCmd& ocmdToUse = pcqiToUse->ocmdData();
    try
    {
        ocmdToUse.strmOut() << TString(L"bQueryZWDIFile");
        ocmdToUse.strmOut() << strFileName;
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

tCIDLib::TCard4 TCQCInstClientProxy::c4QueryDrvManifests
(
    COP TVector<TCQCDriverCfg>& colToFill
    , const TCQCSecToken& sectUser)
{
    #pragma warning(suppress : 26494)
    tCIDLib::TCard4 retVal;
    TCmdQItem* pcqiToUse = pcqiGetCmdItem(ooidThis().oidKey());
    TOrbCmd& ocmdToUse = pcqiToUse->ocmdData();
    try
    {
        ocmdToUse.strmOut() << TString(L"c4QueryDrvManifests");
        ocmdToUse.strmOut() << sectUser;
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

tCIDLib::TVoid TCQCInstClientProxy::DrvCfgQueryDrvsInfo
(
    COP TVector<TString>& colMonikers
    , COP TVector<TString>& colMakes
    , COP TVector<TString>& colModels
    , COP TFundVector<tCQCKit::EDevCats>& fcolCats
    , const TCQCSecToken& sectUser)
{
    TCmdQItem* pcqiToUse = pcqiGetCmdItem(ooidThis().oidKey());
    TOrbCmd& ocmdToUse = pcqiToUse->ocmdData();
    try
    {
        ocmdToUse.strmOut() << TString(L"DrvCfgQueryDrvsInfo");
        ocmdToUse.strmOut() << sectUser;
        Dispatch(30000, pcqiToUse);
        ocmdToUse.strmIn().Reset();
        ocmdToUse.strmIn() >> colMonikers;
        ocmdToUse.strmIn() >> colMakes;
        ocmdToUse.strmIn() >> colModels;
        ocmdToUse.strmIn() >> fcolCats;
        GiveBackCmdItem(pcqiToUse);
    }
    catch(TError& errToCatch)
    {
        GiveBackCmdItem(pcqiToUse);
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}

tCIDLib::TVoid TCQCInstClientProxy::DrvCfgRemoveDrv
(
    const TString& strMoniker
    , const TString& strHost
    , COP tCIDLib::TCard4& c4NewVersion
    , const TCQCSecToken& sectUser)
{
    TCmdQItem* pcqiToUse = pcqiGetCmdItem(ooidThis().oidKey());
    TOrbCmd& ocmdToUse = pcqiToUse->ocmdData();
    try
    {
        ocmdToUse.strmOut() << TString(L"DrvCfgRemoveDrv");
        ocmdToUse.strmOut() << strMoniker;
        ocmdToUse.strmOut() << strHost;
        ocmdToUse.strmOut() << sectUser;
        Dispatch(30000, pcqiToUse);
        ocmdToUse.strmIn().Reset();
        ocmdToUse.strmIn() >> c4NewVersion;
        GiveBackCmdItem(pcqiToUse);
    }
    catch(TError& errToCatch)
    {
        GiveBackCmdItem(pcqiToUse);
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}

tCIDLib::TVoid TCQCInstClientProxy::DrvCfgQueryReportInfo
(
    COP TVector<TKeyValues>& colToFill
    , const TCQCSecToken& sectUser)
{
    TCmdQItem* pcqiToUse = pcqiGetCmdItem(ooidThis().oidKey());
    TOrbCmd& ocmdToUse = pcqiToUse->ocmdData();
    try
    {
        ocmdToUse.strmOut() << TString(L"DrvCfgQueryReportInfo");
        ocmdToUse.strmOut() << sectUser;
        Dispatch(30000, pcqiToUse);
        ocmdToUse.strmIn().Reset();
        ocmdToUse.strmIn() >> colToFill;
        GiveBackCmdItem(pcqiToUse);
    }
    catch(TError& errToCatch)
    {
        GiveBackCmdItem(pcqiToUse);
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}

tCIDLib::TVoid TCQCInstClientProxy::DrvCfgSetConfig
(
    const TString& strMoniker
    , const TString& strHost
    , const TCQCDriverObjCfg& cqcdcToSet
    , const tCIDLib::TBoolean bNewDriver
    , COP tCIDLib::TCard4& c4NewVersion
    , const TCQCSecToken& sectUser)
{
    TCmdQItem* pcqiToUse = pcqiGetCmdItem(ooidThis().oidKey());
    TOrbCmd& ocmdToUse = pcqiToUse->ocmdData();
    try
    {
        ocmdToUse.strmOut() << TString(L"DrvCfgSetConfig");
        ocmdToUse.strmOut() << strMoniker;
        ocmdToUse.strmOut() << strHost;
        ocmdToUse.strmOut() << cqcdcToSet;
        ocmdToUse.strmOut() << bNewDriver;
        ocmdToUse.strmOut() << sectUser;
        Dispatch(30000, pcqiToUse);
        ocmdToUse.strmIn().Reset();
        ocmdToUse.strmIn() >> c4NewVersion;
        GiveBackCmdItem(pcqiToUse);
    }
    catch(TError& errToCatch)
    {
        GiveBackCmdItem(pcqiToUse);
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}

tCIDLib::ELoadRes TCQCInstClientProxy::eQueryGC100Cfg
(
    CIOP tCIDLib::TCard4& c4SerialNum
    , tCIDLib::TCard4& c4BufSz_mbufToFill
    , COP THeapBuf& mbufToFill
    , const TCQCSecToken& sectUser)
{
    #pragma warning(suppress : 26494)
    tCIDLib::ELoadRes retVal;
    TCmdQItem* pcqiToUse = pcqiGetCmdItem(ooidThis().oidKey());
    TOrbCmd& ocmdToUse = pcqiToUse->ocmdData();
    try
    {
        ocmdToUse.strmOut() << TString(L"eQueryGC100Cfg");
        ocmdToUse.strmOut() << c4SerialNum;
        ocmdToUse.strmOut() << sectUser;
        Dispatch(30000, pcqiToUse);
        ocmdToUse.strmIn().Reset();
        ocmdToUse.strmIn() >> retVal;
        ocmdToUse.strmIn() >> c4SerialNum;
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
    return retVal;
}

tCIDLib::ELoadRes TCQCInstClientProxy::eQueryJAPCfg
(
    CIOP tCIDLib::TCard4& c4SerialNum
    , tCIDLib::TCard4& c4BufSz_mbufToFill
    , COP THeapBuf& mbufToFill
    , const TCQCSecToken& sectUser)
{
    #pragma warning(suppress : 26494)
    tCIDLib::ELoadRes retVal;
    TCmdQItem* pcqiToUse = pcqiGetCmdItem(ooidThis().oidKey());
    TOrbCmd& ocmdToUse = pcqiToUse->ocmdData();
    try
    {
        ocmdToUse.strmOut() << TString(L"eQueryJAPCfg");
        ocmdToUse.strmOut() << c4SerialNum;
        ocmdToUse.strmOut() << sectUser;
        Dispatch(30000, pcqiToUse);
        ocmdToUse.strmIn().Reset();
        ocmdToUse.strmIn() >> retVal;
        ocmdToUse.strmIn() >> c4SerialNum;
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
    return retVal;
}

tCIDLib::TVoid TCQCInstClientProxy::Ping()
{
    TCmdQItem* pcqiToUse = pcqiGetCmdItem(ooidThis().oidKey());
    TOrbCmd& ocmdToUse = pcqiToUse->ocmdData();
    try
    {
        ocmdToUse.strmOut() << TString(L"Ping");
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

tCIDLib::TVoid TCQCInstClientProxy::QueryVoiceFiles
(
    tCIDLib::TCard4& c4BufSz_mbufTree
    , COP THeapBuf& mbufTree
    , tCIDLib::TCard4& c4BufSz_mbufGrammar
    , COP THeapBuf& mbufGrammar)
{
    TCmdQItem* pcqiToUse = pcqiGetCmdItem(ooidThis().oidKey());
    TOrbCmd& ocmdToUse = pcqiToUse->ocmdData();
    try
    {
        ocmdToUse.strmOut() << TString(L"QueryVoiceFiles");
        Dispatch(30000, pcqiToUse);
        ocmdToUse.strmIn().Reset();
        ocmdToUse.strmIn() >> c4BufSz_mbufTree;
        ocmdToUse.strmIn().c4ReadBuffer(mbufTree, c4BufSz_mbufTree);
        ocmdToUse.strmIn() >> c4BufSz_mbufGrammar;
        ocmdToUse.strmIn().c4ReadBuffer(mbufGrammar, c4BufSz_mbufGrammar);
        GiveBackCmdItem(pcqiToUse);
    }
    catch(TError& errToCatch)
    {
        GiveBackCmdItem(pcqiToUse);
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}

tCIDLib::TVoid TCQCInstClientProxy::RenameEMailAccount
(
    const TString& strOldName
    , const TString& strNewName
    , const TCQCSecToken& sectUser)
{
    TCmdQItem* pcqiToUse = pcqiGetCmdItem(ooidThis().oidKey());
    TOrbCmd& ocmdToUse = pcqiToUse->ocmdData();
    try
    {
        ocmdToUse.strmOut() << TString(L"RenameEMailAccount");
        ocmdToUse.strmOut() << strOldName;
        ocmdToUse.strmOut() << strNewName;
        ocmdToUse.strmOut() << sectUser;
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

tCIDLib::TVoid TCQCInstClientProxy::SetKeyMap
(
    const TString& strUserName
    , const TCQCKeyMap& kmToSEt
    , const TCQCSecToken& sectUser)
{
    TCmdQItem* pcqiToUse = pcqiGetCmdItem(ooidThis().oidKey());
    TOrbCmd& ocmdToUse = pcqiToUse->ocmdData();
    try
    {
        ocmdToUse.strmOut() << TString(L"SetKeyMap");
        ocmdToUse.strmOut() << strUserName;
        ocmdToUse.strmOut() << kmToSEt;
        ocmdToUse.strmOut() << sectUser;
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

tCIDLib::TBoolean TCQCInstClientProxy::SetLocationInfo
(
    const tCIDLib::TFloat8 f8Lat
    , const tCIDLib::TFloat8 f8Long
    , const TCQCSecToken& sectUser)
{
    #pragma warning(suppress : 26494)
    tCIDLib::TBoolean retVal;
    TCmdQItem* pcqiToUse = pcqiGetCmdItem(ooidThis().oidKey());
    TOrbCmd& ocmdToUse = pcqiToUse->ocmdData();
    try
    {
        ocmdToUse.strmOut() << TString(L"SetLocationInfo");
        ocmdToUse.strmOut() << f8Lat;
        ocmdToUse.strmOut() << f8Long;
        ocmdToUse.strmOut() << sectUser;
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

tCIDLib::TVoid TCQCInstClientProxy::UploadProtocol
(
    const TString& strFileName
    , const tCIDLib::TCard4 c4BufSz_mbufText
    , const THeapBuf& mbufText
    , const tCIDLib::TBoolean bSystem
    , const TCQCSecToken& sectUser)
{
    TCmdQItem* pcqiToUse = pcqiGetCmdItem(ooidThis().oidKey());
    TOrbCmd& ocmdToUse = pcqiToUse->ocmdData();
    try
    {
        ocmdToUse.strmOut() << TString(L"UploadProtocol");
        ocmdToUse.strmOut() << strFileName;
        ocmdToUse.strmOut() << c4BufSz_mbufText;
        ocmdToUse.strmOut().c4WriteBuffer(mbufText, c4BufSz_mbufText);
        ocmdToUse.strmOut() << bSystem;
        ocmdToUse.strmOut() << sectUser;
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

tCIDLib::TVoid TCQCInstClientProxy::UploadManifest
(
    const TString& strFileName
    , const tCIDLib::TCard4 c4BufSz_mbufText
    , const THeapBuf& mbufText
    , const tCIDLib::TBoolean bSystem
    , const TCQCSecToken& sectUser)
{
    TCmdQItem* pcqiToUse = pcqiGetCmdItem(ooidThis().oidKey());
    TOrbCmd& ocmdToUse = pcqiToUse->ocmdData();
    try
    {
        ocmdToUse.strmOut() << TString(L"UploadManifest");
        ocmdToUse.strmOut() << strFileName;
        ocmdToUse.strmOut() << c4BufSz_mbufText;
        ocmdToUse.strmOut().c4WriteBuffer(mbufText, c4BufSz_mbufText);
        ocmdToUse.strmOut() << bSystem;
        ocmdToUse.strmOut() << sectUser;
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

