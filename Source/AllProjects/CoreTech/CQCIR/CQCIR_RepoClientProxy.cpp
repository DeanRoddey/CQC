// ----------------------------------------------------------------------------
//  FILE: CQCIR_RepoClientProxy.cpp
//  DATE: Fri, Feb 12 21:14:15 2021 -0500
//    ID: E2ABEBB0AA4D1186-3412D00C17FA509A
//
//  This file was generated by the Charmed Quark CIDIDL compiler. Do not make
//  changes by hand, because they will be lost if the file is regenerated.
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Includes
// ----------------------------------------------------------------------------
#include "CQCIR_.hpp"

// ----------------------------------------------------------------------------
// Magic macros
// ----------------------------------------------------------------------------
AdvRTTIDecls(TIRRepoClientProxy,TOrbClientBase)


// ----------------------------------------------------------------------------
// TIRRepoClientProxy: Public, static data
// ----------------------------------------------------------------------------
const TString TIRRepoClientProxy::strInterfaceId(L"E2ABEBB0AA4D1186-3412D00C17FA509A");
const TString TIRRepoClientProxy::strBinding(L"/CQC/CQCDataServer/IRServerObj");



// ----------------------------------------------------------------------------
// TIRRepoClientProxy: Constructors and Destructor
// ----------------------------------------------------------------------------
TIRRepoClientProxy::TIRRepoClientProxy()
{
}

TIRRepoClientProxy::
TIRRepoClientProxy(const TOrbObjId& ooidSrc, const TString& strNSBinding)
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

TIRRepoClientProxy::~TIRRepoClientProxy()
{
}

// ----------------------------------------------------------------------------
// TIRRepoClientProxy: Public, non-virtual methods
// ----------------------------------------------------------------------------
tCIDLib::TBoolean TIRRepoClientProxy::bModelExists
(
    const TString& strDevModel
    , const TString& strBlasterName)
{
    #pragma warning(suppress : 26494)
    tCIDLib::TBoolean retVal;
    TCmdQItem* pcqiToUse = pcqiGetCmdItem(ooidThis().oidKey());
    TOrbCmd& ocmdToUse = pcqiToUse->ocmdData();
    try
    {
        ocmdToUse.strmOut() << TString(L"bModelExists");
        ocmdToUse.strmOut() << strDevModel;
        ocmdToUse.strmOut() << strBlasterName;
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

tCIDLib::TCard4 TIRRepoClientProxy::c4QueryDevCatList
(
    const tCQCKit::EDevCats eCategory
    , const TString& strBlasterName
    , COP TVector<TIRBlasterDevModelInfo>& colToFill)
{
    #pragma warning(suppress : 26494)
    tCIDLib::TCard4 retVal;
    TCmdQItem* pcqiToUse = pcqiGetCmdItem(ooidThis().oidKey());
    TOrbCmd& ocmdToUse = pcqiToUse->ocmdData();
    try
    {
        ocmdToUse.strmOut() << TString(L"c4QueryDevCatList");
        ocmdToUse.strmOut() << eCategory;
        ocmdToUse.strmOut() << strBlasterName;
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

tCIDLib::TCard4 TIRRepoClientProxy::c4QueryDevModelList
(
    const TString& strBlasterName
    , COP TVector<TIRBlasterDevModelInfo>& colToFill)
{
    #pragma warning(suppress : 26494)
    tCIDLib::TCard4 retVal;
    TCmdQItem* pcqiToUse = pcqiGetCmdItem(ooidThis().oidKey());
    TOrbCmd& ocmdToUse = pcqiToUse->ocmdData();
    try
    {
        ocmdToUse.strmOut() << TString(L"c4QueryDevModelList");
        ocmdToUse.strmOut() << strBlasterName;
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

tCIDLib::TVoid TIRRepoClientProxy::DeleteDevModel
(
    const TString& strDevModel
    , const TString& strBlasterName)
{
    TCmdQItem* pcqiToUse = pcqiGetCmdItem(ooidThis().oidKey());
    TOrbCmd& ocmdToUse = pcqiToUse->ocmdData();
    try
    {
        ocmdToUse.strmOut() << TString(L"DeleteDevModel");
        ocmdToUse.strmOut() << strDevModel;
        ocmdToUse.strmOut() << strBlasterName;
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

tCIDLib::TVoid TIRRepoClientProxy::QueryDevModel
(
    const TString& strDevModel
    , const TString& strBlasterName
    , COP TIRBlasterDevModel& irbdmToFill)
{
    TCmdQItem* pcqiToUse = pcqiGetCmdItem(ooidThis().oidKey());
    TOrbCmd& ocmdToUse = pcqiToUse->ocmdData();
    try
    {
        ocmdToUse.strmOut() << TString(L"QueryDevModel");
        ocmdToUse.strmOut() << strDevModel;
        ocmdToUse.strmOut() << strBlasterName;
        Dispatch(30000, pcqiToUse);
        ocmdToUse.strmIn().Reset();
        ocmdToUse.strmIn() >> irbdmToFill;
        GiveBackCmdItem(pcqiToUse);
    }
    catch(TError& errToCatch)
    {
        GiveBackCmdItem(pcqiToUse);
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}

tCIDLib::TVoid TIRRepoClientProxy::WriteDevModel
(
    const TString& strDevModel
    , const TString& strBlasterName
    , const TIRBlasterDevModel& irbdmToWrite)
{
    TCmdQItem* pcqiToUse = pcqiGetCmdItem(ooidThis().oidKey());
    TOrbCmd& ocmdToUse = pcqiToUse->ocmdData();
    try
    {
        ocmdToUse.strmOut() << TString(L"WriteDevModel");
        ocmdToUse.strmOut() << strDevModel;
        ocmdToUse.strmOut() << strBlasterName;
        ocmdToUse.strmOut() << irbdmToWrite;
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

