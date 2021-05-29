//
// FILE NAME: CQCMEng_SimpleFldClientClass.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/12/2003
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
//  This file implements the info and value classes to create a macro level
//  class that provides simple CQC field I/O.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCMEng_.hpp"
#include    "CQCMEng_SimpleFldClientClass_.hpp"
#include    "CQCMEng_FieldDefClass_.hpp"




// ---------------------------------------------------------------------------
//  Magic RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TSimpleFldValue,TMEngClassVal)
RTTIDecls(TSimpleFldInfo,TMEngClassInfo)


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace
{
    namespace CQCMEng_SimpleFldClient
    {
        // -----------------------------------------------------------------------
        //  The names for the types that we support here. Each derivative has to
        //  be able to return strings that contain its name and full name.
        // -----------------------------------------------------------------------
        const TString   strBasePath(L"MEng.System.CQC.Runtime");
        const TString   strStrVector(L"MEng.System.CQC.Runtime.SimpleFldClient.");

        const TString   strFldIOName(L"SimpleFldClient");
        const TString   strFldIOClassPath(L"MEng.System.CQC.Runtime.SimpleFldClient");
    }
}




// ---------------------------------------------------------------------------
//  CLASS: TSimpleFldValue
// PREFIX: mecv
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TSimpleFldValue: Constuctors and Destructor
// ---------------------------------------------------------------------------
TSimpleFldValue::TSimpleFldValue(const  TString&                strName
                                , const tCIDLib::TCard2         c2ClassId
                                , const tCIDMacroEng::EConstTypes  eConst) :

    TMEngClassVal(strName, c2ClassId, eConst)
{
}

TSimpleFldValue::~TSimpleFldValue()
{
}




// ---------------------------------------------------------------------------
//  CLASS: TSimpleFldInfo
// PREFIX: meci
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TSimpleFldInfo: Public, static methods
// ---------------------------------------------------------------------------
const TString& TSimpleFldInfo::strClassPath()
{
    return CQCMEng_SimpleFldClient::strFldIOClassPath;
}


// ---------------------------------------------------------------------------
//  TSimpleFldInfo: Constructors and Destructor
// ---------------------------------------------------------------------------
TSimpleFldInfo::TSimpleFldInfo(TCIDMacroEngine& meOwner, const TCQCSecToken& sectUser) :

    TMEngClassInfo
    (
        CQCMEng_SimpleFldClient::strFldIOName
        , CQCMEng_SimpleFldClient::strBasePath
        , meOwner
        , kCIDLib::False
        , tCIDMacroEng::EClassExt::Final
        , L"MEng.Object"
    )
    , m_c2EnumId_Errors(kCIDMacroEng::c2BadId)
    , m_c2EnumId_FldTokRes(kCIDMacroEng::c2BadId)
    , m_c2MethId_CancelTimedFldChange(kCIDMacroEng::c2BadId)
    , m_c2MethId_DefCtor(kCIDMacroEng::c2BadId)
    , m_c2MethId_HasQueuedFldChange(kCIDMacroEng::c2BadId)
    , m_c2MethId_QueryCQCServers(kCIDMacroEng::c2BadId)
    , m_c2MethId_QueryData(kCIDMacroEng::c2BadId)
    , m_c2MethId_QueryDrivers(kCIDMacroEng::c2BadId)
    , m_c2MethId_QueryFields(kCIDMacroEng::c2BadId)
    , m_c2MethId_QueryStatusReport(kCIDMacroEng::c2BadId)
    , m_c2MethId_QueryTextVal(kCIDMacroEng::c2BadId)
    , m_c2MethId_ReadBoolField(kCIDMacroEng::c2BadId)
    , m_c2MethId_ReadCardField(kCIDMacroEng::c2BadId)
    , m_c2MethId_ReadField(kCIDMacroEng::c2BadId)
    , m_c2MethId_ReadFloatField(kCIDMacroEng::c2BadId)
    , m_c2MethId_ReadIntField(kCIDMacroEng::c2BadId)
    , m_c2MethId_ReadStringField(kCIDMacroEng::c2BadId)
    , m_c2MethId_ReadStringListField(kCIDMacroEng::c2BadId)
    , m_c2MethId_ReadTimeField(kCIDMacroEng::c2BadId)
    , m_c2MethId_SendDrvCmd(kCIDMacroEng::c2BadId)
    , m_c2MethId_TimedFldChange(kCIDMacroEng::c2BadId)
    , m_c2MethId_TimedFldChange2(kCIDMacroEng::c2BadId)
    , m_c2MethId_ReplaceFldTokens(kCIDMacroEng::c2BadId)
    , m_c2MethId_WaitDriverReady(kCIDMacroEng::c2BadId)
    , m_c2MethId_WriteBoolField(kCIDMacroEng::c2BadId)
    , m_c2MethId_WriteCardField(kCIDMacroEng::c2BadId)
    , m_c2MethId_WriteCharField(kCIDMacroEng::c2BadId)
    , m_c2MethId_WriteField(kCIDMacroEng::c2BadId)
    , m_c2MethId_WriteField2(kCIDMacroEng::c2BadId)
    , m_c2MethId_WriteFloatField(kCIDMacroEng::c2BadId)
    , m_c2MethId_WriteIntField(kCIDMacroEng::c2BadId)
    , m_c2MethId_WriteStringField(kCIDMacroEng::c2BadId)
    , m_c2MethId_WriteStringListField(kCIDMacroEng::c2BadId)
    , m_c2MethId_WriteTimeField(kCIDMacroEng::c2BadId)
    , m_c4ErrBadFldName(kCIDMacroEng::c2BadId)
    , m_c4ErrQueryFailed(0)
    , m_c4ErrReadFailed(0)
    , m_c4ErrSendFailed(0)
    , m_c4ErrWriteFailed(0)
    , m_pmeciErrors(nullptr)
    , m_pmeciFldDef(nullptr)
    , m_pmeciFldTokRes(nullptr)
    , m_pmeciFldDefList(nullptr)
    , m_pmeciStrList(nullptr)
    , m_sectUser(sectUser)
{
    // Import the field definition class
    bAddClassImport(TCQCFldDefInfo::strClassPath());
}

TSimpleFldInfo::~TSimpleFldInfo()
{
}


tCIDLib::TVoid TSimpleFldInfo::Init(TCIDMacroEngine& meOwner)
{
    // Create an enumerated type for our errors
    {
        m_pmeciErrors = new TMEngEnumInfo
        (
            meOwner
            , L"SimpleFldErrors"
            , strClassPath()
            , L"MEng.Enum"
            , 7
        );
        m_c4ErrBadFldName = m_pmeciErrors->c4AddEnumItem(L"BadFldName", L"The name must be in moniker.field format");
        m_c4ErrQueryFailed = m_pmeciErrors->c4AddEnumItem(L"QueryFailed", TString::strEmpty());
        m_c4ErrReadFailed = m_pmeciErrors->c4AddEnumItem(L"ReadFailed", TString::strEmpty());
        m_c4ErrSendFailed = m_pmeciErrors->c4AddEnumItem(L"SendFailed", TString::strEmpty());
        m_c4ErrTimedChgFailed = m_pmeciErrors->c4AddEnumItem(L"TimedChgFailed", TString::strEmpty());
        m_c4ErrWriteFailed = m_pmeciErrors->c4AddEnumItem(L"WriteFailed", TString::strEmpty());
        m_pmeciErrors->BaseClassInit(meOwner);
        m_c2EnumId_Errors = meOwner.c2AddClass(m_pmeciErrors);
        bAddNestedType(m_pmeciErrors->strClassPath());
    }

    // Create an enum for the return value of ReplaceFldTokens
    {
        m_pmeciFldTokRes = new TMEngEnumInfo
        (
            meOwner
            , L"FldTokenResults"
            , strClassPath()
            , L"MEng.Enum"
            , 2
        );
        m_pmeciFldTokRes->c4AddEnumItem(L"Unchanged", L"Unchanged", tCQCKit::ECmdPrepRes::Unchanged);
        m_pmeciFldTokRes->c4AddEnumItem(L"Failed", L"Failed", tCQCKit::ECmdPrepRes::Failed);
        m_pmeciFldTokRes->c4AddEnumItem(L"Changed", L"Changed", tCQCKit::ECmdPrepRes::Changed);
        m_pmeciFldTokRes->BaseClassInit(meOwner);
        m_c2EnumId_FldTokRes = meOwner.c2AddClass(m_pmeciFldTokRes);
        bAddNestedType(m_pmeciFldTokRes->strClassPath());
    }

    // Create a nested vector of strings type
    {
        TMEngVectorInfo* pmeciNew = new TMEngVectorInfo
        (
            meOwner
            , L"StrList"
            , strClassPath()
            , TMEngVectorInfo::strPath()
            , tCIDLib::TCard2(tCIDMacroEng::EIntrinsics::String)
        );
        TJanitor<TMEngVectorInfo> janNewClass(pmeciNew);
        pmeciNew->BaseClassInit(meOwner);
        bAddNestedType(pmeciNew->strClassPath());
        meOwner.c2AddClass(janNewClass.pobjOrphan());
        m_pmeciStrList = pmeciNew;
    }

    // Look up the field def class
    m_pmeciFldDef = meOwner.pmeciFindAs<TCQCFldDefInfo>
    (
        TCQCFldDefInfo::strClassPath(), kCIDLib::True
    );


    // Create a vector of field def classes
    {
        TMEngVectorInfo* pmeciNew = new TMEngVectorInfo
        (
            meOwner
            , L"FldDefList"
            , strClassPath()
            , TMEngVectorInfo::strPath()
            , m_pmeciFldDef->c2Id()
        );
        TJanitor<TMEngVectorInfo> janNewClass(pmeciNew);
        pmeciNew->BaseClassInit(meOwner);
        bAddNestedType(pmeciNew->strClassPath());
        meOwner.c2AddClass(janNewClass.pobjOrphan());
        m_pmeciFldDefList = pmeciNew;
    }


    // Add the default constructor
    {
        TMEngMethodInfo methiNew
        (
            L"ctor1_MEng.System.CQC.Runtime.SimpleFldClient"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.bIsCtor(kCIDLib::True);
        m_c2MethId_DefCtor = c2AddMethodInfo(methiNew);
    }

    // Cancel any pending timed field change for the indicated field
    {
        TMEngMethodInfo methiNew
        (
            L"CancelTimedFldChange"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInParm(L"Moniker", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"FieldName", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_CancelTimedFldChange = c2AddMethodInfo(methiNew);
    }

    // Check to see if a field has a queued up timed field change
    {
        TMEngMethodInfo methiNew
        (
            L"HasQueuedFldChange"
            , tCIDMacroEng::EIntrinsics::Boolean
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInParm(L"Moniker", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"FieldName", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddOutParm(L"EndTime", tCIDMacroEng::EIntrinsics::Card8);
        m_c2MethId_HasQueuedFldChange = c2AddMethodInfo(methiNew);
    }

    // Query a list of CQCServers in the system
    {
        TMEngMethodInfo methiNew
        (
            L"QueryCQCServers"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddOutParm(L"ListToFill", m_pmeciStrList->c2Id());
        m_c2MethId_QueryCQCServers = c2AddMethodInfo(methiNew);
    }

    // Query a data buffer via the backdoor method
    {
        TMEngMethodInfo methiNew
        (
            L"QueryData"
            , tCIDMacroEng::EIntrinsics::Boolean
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInParm(L"Moniker", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"QueryId", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"DataName", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddOutParm(L"ByteCnt", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddOutParm(L"ToFill", tCIDMacroEng::EIntrinsics::MemBuf);
        m_c2MethId_QueryData = c2AddMethodInfo(methiNew);
    }

    // Query the list of available drivers
    {
        TMEngMethodInfo methiNew
        (
            L"QueryDrivers"
            , tCIDMacroEng::EIntrinsics::Card4
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddOutParm(L"Drivers", m_pmeciStrList->c2Id());
        m_c2MethId_QueryDrivers = c2AddMethodInfo(methiNew);
    }

    // Query the list of fields for a given driver
    {
        TMEngMethodInfo methiNew
        (
            L"QueryFields"
            , tCIDMacroEng::EIntrinsics::Card4
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInParm(L"Moniker", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddOutParm(L"FldDefs", m_pmeciFldDefList->c2Id());
        methiNew.c2AddOutParm(L"FldListId", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddOutParm(L"DriverId", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddOutParm(L"DriverListId", tCIDMacroEng::EIntrinsics::Card4);
        m_c2MethId_QueryFields = c2AddMethodInfo(methiNew);
    }

    // Query a status report from a given CQCServer
    {
        TMEngMethodInfo methiNew
        (
            L"QueryStatusReport"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddOutParm(L"Report", m_pmeciStrList->c2Id());
        m_c2MethId_QueryStatusReport = c2AddMethodInfo(methiNew);
    }

    // Query a text buffer via the backdoor method
    {
        TMEngMethodInfo methiNew
        (
            L"QueryTextVal"
            , tCIDMacroEng::EIntrinsics::Boolean
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInParm(L"Moniker", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"QueryId", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"DataName", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddOutParm(L"ToFill", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_QueryTextVal = c2AddMethodInfo(methiNew);
    }

    // Do standard field token replacement
    {
        TMEngMethodInfo methiNew
        (
            L"ReplaceFldTokens"
            , m_c2EnumId_FldTokRes
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInParm(L"InSrc", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddOutParm(L"ToFill", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_ReplaceFldTokens = c2AddMethodInfo(methiNew);
    }

    // Send a driver command
    {
        TMEngMethodInfo methiNew
        (
            L"SendDrvCmd"
            , tCIDMacroEng::EIntrinsics::Card4
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInParm(L"Moniker", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"CmdId", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"ValToSet", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_SendDrvCmd = c2AddMethodInfo(methiNew);
    }

    // Do a timed field change
    {
        TMEngMethodInfo methiNew
        (
            L"TimedFldChange"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInParm(L"Moniker", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"FieldName", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"NewValue", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"FinalValue", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"Minutes", tCIDMacroEng::EIntrinsics::Card4);
        m_c2MethId_TimedFldChange = c2AddMethodInfo(methiNew);
    }

    // Same as above but supports seconds
    {
        TMEngMethodInfo methiNew
        (
            L"TimedFldChange2"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInParm(L"Moniker", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"FieldName", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"NewValue", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"FinalValue", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"Minutes", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddInParm(L"Seconds", tCIDMacroEng::EIntrinsics::Card4);
        m_c2MethId_TimedFldChange2 = c2AddMethodInfo(methiNew);
    }

    //
    //  !!!!! All of the field read/write methdos must be contiguous so that
    //  we can check for them via a >= <= method id check below!
    //

    // Read field methods
    {
        TMEngMethodInfo methiNew
        (
            L"ReadBoolField"
            , tCIDMacroEng::EIntrinsics::Boolean
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"Moniker", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"FieldName", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_ReadBoolField = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"ReadCardField"
            , tCIDMacroEng::EIntrinsics::Card4
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"Moniker", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"FieldName", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_ReadCardField = c2AddMethodInfo(methiNew);
    }

    // Read a field generically as a string
    {
        TMEngMethodInfo methiNew
        (
            L"ReadField"
            , tCIDMacroEng::EIntrinsics::String
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"Moniker", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"FieldName", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_ReadField = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"ReadFloatField"
            , tCIDMacroEng::EIntrinsics::Float8
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"Moniker", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"FieldName", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_ReadFloatField = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"ReadIntField"
            , tCIDMacroEng::EIntrinsics::Int4
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"Moniker", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"FieldName", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_ReadIntField = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"ReadStringField"
            , tCIDMacroEng::EIntrinsics::String
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"Moniker", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"FieldName", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_ReadStringField = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"ReadStringListField"
            , tCIDMacroEng::EIntrinsics::Card4
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"Moniker", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"FieldName", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddOutParm(L"ToFill", m_pmeciStrList->c2Id());
        m_c2MethId_ReadStringListField = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"ReadTimeField"
            , tCIDMacroEng::EIntrinsics::Card8
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"Moniker", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"FieldName", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_ReadTimeField = c2AddMethodInfo(methiNew);
    }

    // Write field methods
    {
        TMEngMethodInfo methiNew
        (
            L"WriteBoolField"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"Moniker", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"FieldName", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"ToWrite", tCIDMacroEng::EIntrinsics::Boolean);
        m_c2MethId_WriteBoolField = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"WriteCardField"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"Moniker", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"FieldName", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"ToWrite", tCIDMacroEng::EIntrinsics::Card4);
        m_c2MethId_WriteCardField = c2AddMethodInfo(methiNew);
    }

    // Write a field generically via a string value
    {
        TMEngMethodInfo methiNew
        (
            L"WriteField"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"Moniker", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"FieldName", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"ToWrite", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_WriteField = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"WriteFloatField"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"Moniker", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"FieldName", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"ToWrite", tCIDMacroEng::EIntrinsics::Float8);
        m_c2MethId_WriteFloatField = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"WriteIntField"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"Moniker", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"FieldName", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"ToWrite", tCIDMacroEng::EIntrinsics::Int4);
        m_c2MethId_WriteIntField = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"WriteStringField"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"Moniker", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"FieldName", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"ToWrite", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_WriteStringField = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"WriteStringListField"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"Moniker", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"FieldName", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"ToWrite", m_pmeciStrList->c2Id());
        m_c2MethId_WriteStringListField = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"WriteTimeField"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"Moniker", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"FieldName", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"ToWrite", tCIDMacroEng::EIntrinsics::Card8);
        m_c2MethId_WriteTimeField = c2AddMethodInfo(methiNew);
    }

    //
    //  !!!!! All of the field read/write methdos must be contiguous so that
    //  we can check for them via a >= <= method id check below! So put any
    //  other methods below this point.
    //

    // Wait for up to a given time for a device driver to come online
    {
        TMEngMethodInfo methiNew
        (
            L"WaitDriverReady"
            , tCIDMacroEng::EIntrinsics::Boolean
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInParm(L"Moniker", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"Millis", tCIDMacroEng::EIntrinsics::Card4);
        m_c2MethId_WaitDriverReady = c2AddMethodInfo(methiNew);
    }

    // Like ReadField above but takes a single string with moniker.field
    {
        TMEngMethodInfo methiNew
        (
            L"ReadField2"
            , tCIDMacroEng::EIntrinsics::String
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"Field", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_ReadField2 = c2AddMethodInfo(methiNew);
    }

    // Like WriteField above but takes a single string with moniker.field
    {
        TMEngMethodInfo methiNew
        (
            L"WriteField2"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"Field", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"ToWrite", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_WriteField2 = c2AddMethodInfo(methiNew);
    }
}


TMEngClassVal*
TSimpleFldInfo::pmecvMakeStorage(const  TString&                strName
                                ,       TCIDMacroEngine&
                                , const tCIDMacroEng::EConstTypes  eConst) const
{
    return new TSimpleFldValue(strName, c2Id(), eConst);
}


// ---------------------------------------------------------------------------
//  TSimpleFldInfo: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TSimpleFldInfo::bInvokeMethod(          TCIDMacroEngine&    meOwner
                                , const TMEngMethodInfo&    methiTarget
                                ,       TMEngClassVal&      )
{
    // Don't need this so far
//    TSimpleFldValue& mecvActual = static_cast<TSimpleFldValue&>(mecvInstance);

    const tCIDLib::TCard4 c4FirstInd = meOwner.c4FirstParmInd(methiTarget);
    const tCIDLib::TCard2 c2MethId   = methiTarget.c2Id();

    if (c2MethId == m_c2MethId_CancelTimedFldChange)
    {
        try
        {
            // Get an admin proxy for the CQCServer that holds this driver
            const TString& strMoniker = meOwner.strStackValAt(c4FirstInd);
            tCQCKit::TCQCSrvProxy orbcAdmin = facCQCKit().orbcCQCSrvAdminProxy
            (
                strMoniker
            );

            // And make the call
            orbcAdmin->CancelTimedWrite
            (
                strMoniker, meOwner.strStackValAt(c4FirstInd + 1), m_sectUser
            );
        }

        catch(const TError errToCatch)
        {
            // Throw a generic read or write failed error
            ThrowAnErr(meOwner, m_c4ErrTimedChgFailed, errToCatch);
        }
    }
     else if (c2MethId == m_c2MethId_HasQueuedFldChange)
    {
        try
        {
            // Get an admin proxy for the CQCServer that holds this driver
            const TString& strMoniker = meOwner.strStackValAt(c4FirstInd);
            tCQCKit::TCQCSrvProxy orbcAdmin = facCQCKit().orbcCQCSrvAdminProxy
            (
                strMoniker
            );

            // And make the call
            tCIDLib::TCard8 c8EndTime;
            const tCIDLib::TBoolean bRet = orbcAdmin->bHasQueuedTimedWrite
            (
                strMoniker
                , meOwner.strStackValAt(c4FirstInd + 1)
                , c8EndTime
            );

            //
            //  Give back the output value and result. No need to update the out
            //  parm if the return is false.
            //
            if (bRet)
                meOwner.mecvStackAtAs<TMEngCard8Val>(c4FirstInd + 2).c8Value(c8EndTime);
            meOwner.mecvStackAtAs<TMEngBooleanVal>(c4FirstInd - 1).bValue(bRet);
        }

        catch(const TError errToCatch)
        {
            // Throw a generic read or write failed error
            ThrowAnErr(meOwner, m_c4ErrTimedChgFailed, errToCatch);
        }
    }
     else if (c2MethId == m_c2MethId_DefCtor)
    {
        // Nothing to do
    }
     else if (c2MethId == m_c2MethId_QueryCQCServers)
    {
        // Clean out the output vector
        TMEngVectorVal& mecvOut = meOwner.mecvStackAtAs<TMEngVectorVal>(c4FirstInd);
        mecvOut.RemoveAll();

        // We give back a list of the hosts where CQCServers are running
        try
        {
            // Get a name server client proxy so we can get the moniker list
            tCIDOrbUC::TNSrvProxy orbcNS = facCIDOrbUC().orbcNameSrvProxy();

            //
            //  Enumerate the bindings in the driver scope. The host names
            //  are in the description field of the name server info objects.
            //
            TVector<TNameServerInfo> colNSItems;
            if (orbcNS->c4EnumObjects(  TCQCSrvAdminClientProxy::strDrvScope
                                        , colNSItems
                                        , kCIDLib::False))
            {
                const tCIDLib::TCard4 c4Count = colNSItems.c4ElemCount();

                // Find the unique host names
                tCIDLib::TStrHashSet colUnique(109, TStringKeyOps());

                for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
                {
                    tCIDLib::TBoolean bAdded;
                    const TString& strCur = colNSItems[c4Index].strDescription();
                    colUnique.objAddIfNew(strCur, bAdded);
                }

                // Now iterate the unique list and add those to the output list
                tCIDLib::TStrHashSet::TCursor cursUnique(&colUnique);
                if (cursUnique.bReset())
                {
                    do
                    {
                        mecvOut.AddObject
                        (
                            new TMEngStringVal
                            (
                                TString::strEmpty()
                                , tCIDMacroEng::EConstTypes::NonConst
                                , cursUnique.objRCur()
                            )
                        );
                    }   while (cursUnique.bNext());
                }
            }
        }

        catch(const TError& errToCatch)
        {
            // Throw a generic read or write failed error
            ThrowAnErr(meOwner, m_c4ErrQueryFailed, errToCatch);
        }
    }
     else if (c2MethId == m_c2MethId_QueryData)
    {
        try
        {
            // Get an admin proxy for the CQCServer that holds this driver
            const TString& strMoniker = meOwner.strStackValAt(c4FirstInd);
            tCQCKit::TCQCSrvProxy orbcAdmin = facCQCKit().orbcCQCSrvAdminProxy
            (
                strMoniker
            );

            TMEngMemBufVal& mecvToFill = meOwner.mecvStackAtAs<TMEngMemBufVal>
            (
                c4FirstInd + 4
            );

            tCIDLib::TCard4 c4Bytes;
            const tCIDLib::TBoolean bRet = orbcAdmin->bQueryData
            (
                strMoniker
                , meOwner.strStackValAt(c4FirstInd + 1)
                , meOwner.strStackValAt(c4FirstInd + 2)
                , c4Bytes
                , mecvToFill.mbufValue()
            );

            if (bRet)
                meOwner.mecvStackAtAs<TMEngCard4Val>(c4FirstInd + 3).c4Value(c4Bytes);
            meOwner.mecvStackAtAs<TMEngBooleanVal>(c4FirstInd - 1).bValue(bRet);
        }

        catch(const TError& errToCatch)
        {
            // Throw a generic read or write failed error
            ThrowAnErr(meOwner, m_c4ErrQueryFailed, errToCatch);
        }
    }
     else if (c2MethId == m_c2MethId_QueryDrivers)
    {
        try
        {
            // Clean out the output vector
            TMEngVectorVal& mecvOut = meOwner.mecvStackAtAs<TMEngVectorVal>(c4FirstInd);
            mecvOut.RemoveAll();

            // Look up a list of all the available driver monikers
            tCIDLib::TCard4 c4Count = 0;
            if (facCQCKit().bFindAllDrivers(m_colSList))
            {
                // We found some, so copy them over to the CML level vector
                c4Count= m_colSList.c4ElemCount();
                for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
                {
                    mecvOut.AddObject
                    (
                        new TMEngStringVal
                        (
                            TString::strEmpty()
                            , tCIDMacroEng::EConstTypes::NonConst
                            , m_colSList[c4Index]
                        )
                    );
                }
            }

            // And return the count of monikers found
            meOwner.mecvStackAtAs<TMEngCard4Val>(c4FirstInd - 1).c4Value(c4Count);
        }

        catch(const TError& errToCatch)
        {
            // Throw a generic read or write failed error
            ThrowAnErr(meOwner, m_c4ErrQueryFailed, errToCatch);
        }
    }
     else if (c2MethId == m_c2MethId_QueryFields)
    {
        try
        {
            const TString& strMoniker = meOwner.strStackValAt(c4FirstInd);

            // Get an admin proxy for the CQCServer that holds this driver
            tCQCKit::TCQCSrvProxy orbcAdmin = facCQCKit().orbcCQCSrvAdminProxy
            (
                strMoniker
            );

            // Ask for a list of fields for the provided moniker
            tCIDLib::TCard4 c4DriverId;
            tCIDLib::TCard4 c4DriverListId;
            tCIDLib::TCard4 c4FldListId;
            tCQCKit::TFldDefList colFldDefs;
            tCQCKit::EDrvStates eState;
            const tCIDLib::TCard4 c4Count = orbcAdmin->c4QueryFields
            (
                strMoniker, eState, colFldDefs, c4FldListId, c4DriverId, c4DriverListId
            );

            // Copy over the field defs to the CML level vector
            TMEngVectorVal& mecvOut = meOwner.mecvStackAtAs<TMEngVectorVal>(c4FirstInd + 1);
            mecvOut.RemoveAll();
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            {
                mecvOut.AddObject
                (
                    new TCQCFldDefVal
                    (
                        TString::strEmpty()
                        , m_pmeciFldDef->c2Id()
                        , tCIDMacroEng::EConstTypes::NonConst
                        , colFldDefs[c4Index]
                    )
                );
            }

            // Give back the various ids
            meOwner.mecvStackAtAs<TMEngCard4Val>(c4FirstInd + 2).c4Value(c4FldListId);
            meOwner.mecvStackAtAs<TMEngCard4Val>(c4FirstInd + 3).c4Value(c4DriverId);
            meOwner.mecvStackAtAs<TMEngCard4Val>(c4FirstInd + 4).c4Value(c4DriverListId);

            // And return the count of drivers found
            meOwner.mecvStackAtAs<TMEngCard4Val>(c4FirstInd - 1).c4Value(c4Count);
        }

        catch(const TError& errToCatch)
        {
            // Throw a generic read or write failed error
            ThrowAnErr(meOwner, m_c4ErrQueryFailed, errToCatch);
        }
    }
     else if (c2MethId == m_c2MethId_QueryStatusReport)
    {
    }
     else if (c2MethId == m_c2MethId_QueryTextVal)
    {
        try
        {
            // Get an admin proxy for the CQCServer that holds this driver
            const TString& strMoniker = meOwner.strStackValAt(c4FirstInd);
            tCQCKit::TCQCSrvProxy orbcAdmin = facCQCKit().orbcCQCSrvAdminProxy
            (
                strMoniker
            );

            TMEngStringVal& mecvToFill = meOwner.mecvStackAtAs<TMEngStringVal>
            (
                c4FirstInd + 3
            );

            const tCIDLib::TBoolean bRet = orbcAdmin->bQueryTextVal
            (
                strMoniker
                , meOwner.strStackValAt(c4FirstInd + 1)
                , meOwner.strStackValAt(c4FirstInd + 2)
                , mecvToFill.strValue()
            );

            meOwner.mecvStackAtAs<TMEngBooleanVal>(c4FirstInd - 1).bValue(bRet);
        }

        catch(const TError& errToCatch)
        {
            // Throw a generic read or write failed error
            ThrowAnErr(meOwner, m_c4ErrQueryFailed, errToCatch);
        }
    }
     else if (c2MethId == m_c2MethId_ReplaceFldTokens)
    {
        // Get the output string value and return value
        TMEngStringVal& mecvOut = meOwner.mecvStackAtAs<TMEngStringVal>(c4FirstInd + 1);
        TMEngEnumVal& mecvRet = meOwner.mecvStackAtAs<TMEngEnumVal>(c4FirstInd - 1);

        tCQCKit::ECmdPrepRes eRes = facCQCKit().eStdTokenReplace
        (
            meOwner.strStackValAt(c4FirstInd)
            , 0
            , 0
            , 0
            , mecvOut.strValue()
            , tCQCKit::ETokRepFlags::None
        );
        mecvRet.c4Ordinal(tCIDLib::c4EnumOrd(eRes));
    }
     else if (c2MethId == m_c2MethId_SendDrvCmd)
    {
        try
        {
            // Get an admin proxy for the CQCServer that holds this driver
            const TString& strMoniker = meOwner.strStackValAt(c4FirstInd);
            tCQCKit::TCQCSrvProxy orbcAdmin = facCQCKit().orbcCQCSrvAdminProxy(strMoniker);
            const tCIDLib::TCard4 c4Ret = orbcAdmin->c4SendCmd
            (
                strMoniker
                , meOwner.strStackValAt(c4FirstInd + 1)
                , meOwner.strStackValAt(c4FirstInd + 2)
                , m_sectUser
            );

            meOwner.mecvStackAtAs<TMEngCard4Val>(c4FirstInd - 1).c4Value(c4Ret);
        }

        catch(const TError& errToCatch)
        {
            ThrowAnErr(meOwner, m_c4ErrSendFailed, errToCatch);
        }
    }
     else if (c2MethId == m_c2MethId_TimedFldChange)
    {
        try
        {
            // Get an admin proxy for the CQCServer that holds this driver
            const TString& strMoniker = meOwner.strStackValAt(c4FirstInd);
            tCQCKit::TCQCSrvProxy orbcAdmin = facCQCKit().orbcCQCSrvAdminProxy
            (
                strMoniker
            );

            // And make the call * 60 to convert incoming minutes to seconds
            orbcAdmin->DoTimedWrite
            (
                strMoniker
                , meOwner.strStackValAt(c4FirstInd + 1)
                , meOwner.strStackValAt(c4FirstInd + 2)
                , meOwner.strStackValAt(c4FirstInd + 3)
                , meOwner.c4StackValAt(c4FirstInd + 4) * 60
                , m_sectUser
                , tCQCKit::EDrvCmdWaits::DontCare
            );
        }

        catch(const TError errToCatch)
        {
            // Throw a generic read or write failed error
            ThrowAnErr(meOwner, m_c4ErrTimedChgFailed, errToCatch);
        }
    }
     else if (c2MethId == m_c2MethId_TimedFldChange2)
    {
        // Same as above but allows for minutes and seconds (and thus sub-minute times)
        try
        {
            // Get an admin proxy for the CQCServer that holds this driver
            const TString& strMoniker = meOwner.strStackValAt(c4FirstInd);
            tCQCKit::TCQCSrvProxy orbcAdmin = facCQCKit().orbcCQCSrvAdminProxy
            (
                strMoniker
            );

            // Add up the seconds
            const tCIDLib::TCard4 c4Secs
            (
                (meOwner.c4StackValAt(c4FirstInd + 4) * 60)
                + meOwner.c4StackValAt(c4FirstInd + 5)
            );

            // And make the call
            orbcAdmin->DoTimedWrite
            (
                strMoniker
                , meOwner.strStackValAt(c4FirstInd + 1)
                , meOwner.strStackValAt(c4FirstInd + 2)
                , meOwner.strStackValAt(c4FirstInd + 3)
                , c4Secs
                , m_sectUser
                , tCQCKit::EDrvCmdWaits::DontCare
            );
        }

        catch(const TError errToCatch)
        {
            // Throw a generic read or write failed error
            ThrowAnErr(meOwner, m_c4ErrTimedChgFailed, errToCatch);
        }
    }
     else if ((c2MethId >= m_c2MethId_ReadBoolField)
          &&  (c2MethId <= m_c2MethId_WriteTimeField))
    {
        const tCIDLib::TBoolean bRead = (c2MethId <= m_c2MethId_ReadStringField);

        //
        //  It's one of the read/write methods. We want to group these all
        //  together because they all require getting a CQCServer admin
        //  proxy. So start by getting the two string parms and trying
        //  to get the proxy.
        //
        const TString& strMoniker = meOwner.strStackValAt(c4FirstInd);
        const TString& strField = meOwner.strStackValAt(c4FirstInd + 1);

        try
        {
            // Get a client proxy for the CQCServer hosting this driver
            tCQCKit::TCQCSrvProxy orbcProxy = facCQCKit().orbcCQCSrvAdminProxy
            (
                strMoniker
            );

            // And do the correct read or write operation
            tCIDLib::TCard4 c4SerNum = 0;
            if (c2MethId == m_c2MethId_ReadBoolField)
            {
                tCIDLib::TBoolean bVal;
                orbcProxy->bReadBoolByName(c4SerNum, strMoniker, strField, bVal);
                TMEngBooleanVal& mecvRet
                        = meOwner.mecvStackAtAs<TMEngBooleanVal>(c4FirstInd - 1);
                mecvRet.bValue(bVal);
            }
             else if (c2MethId == m_c2MethId_ReadCardField)
            {
                tCIDLib::TCard4 c4Val;
                orbcProxy->bReadCardByName(c4SerNum, strMoniker, strField, c4Val);
                TMEngCard4Val& mecvRet
                        = meOwner.mecvStackAtAs<TMEngCard4Val>(c4FirstInd - 1);
                mecvRet.c4Value(c4Val);
            }
             else if (c2MethId == m_c2MethId_ReadFloatField)
            {
                tCIDLib::TFloat8 f8Val;
                orbcProxy->bReadFloatByName(c4SerNum, strMoniker, strField, f8Val);
                TMEngFloat8Val& mecvRet
                        = meOwner.mecvStackAtAs<TMEngFloat8Val>(c4FirstInd - 1);
                mecvRet.f8Value(f8Val);
            }
             else if (c2MethId == m_c2MethId_ReadIntField)
            {
                tCIDLib::TInt4 i4Val;
                orbcProxy->bReadIntByName(c4SerNum, strMoniker, strField, i4Val);
                TMEngInt4Val& mecvRet
                        = meOwner.mecvStackAtAs<TMEngInt4Val>(c4FirstInd - 1);
                mecvRet.i4Value(i4Val);
            }
             else if ((c2MethId == m_c2MethId_ReadStringField)
                  ||  (c2MethId == m_c2MethId_ReadField))
            {
                // These both have the same parameters
                TMEngStringVal& mecvRet
                        = meOwner.mecvStackAtAs<TMEngStringVal>(c4FirstInd - 1);
                if (c2MethId == m_c2MethId_ReadStringField)
                {
                    orbcProxy->bReadStringByName
                    (
                        c4SerNum, strMoniker, strField, mecvRet.strValue()
                    );
                }
                 else
                {
                    tCQCKit::EFldTypes eType;
                    orbcProxy->bReadFieldByName
                    (
                        c4SerNum, strMoniker, strField, mecvRet.strValue(), eType
                    );
                }
            }
             else if (c2MethId == m_c2MethId_ReadStringListField)
            {
                //
                //  We have to get the value into a vector of strings at the
                //  C++ level and then load them up into the caller's CML
                //  level vector of strings. Not very efficient but they
                //  usually don't have many items.
                //
                orbcProxy->bReadSListByName
                (
                    c4SerNum, strMoniker, strField, m_colSList
                );

                // Get the caller's CML vector and load it up
                TMEngVectorVal& mecvList
                        = meOwner.mecvStackAtAs<TMEngVectorVal>(c4FirstInd + 2);
                mecvList.RemoveAll();
                const tCIDLib::TCard4 c4ValCount = m_colSList.c4ElemCount();
                for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ValCount; c4Index++)
                {
                    mecvList.AddObject
                    (
                        new TMEngStringVal
                        (
                            TString::strEmpty()
                            , tCIDMacroEng::EConstTypes::NonConst
                            , m_colSList[c4Index]
                         )
                    );
                }

                // Return the count
                TMEngCard4Val& mecvRet
                        = meOwner.mecvStackAtAs<TMEngCard4Val>(c4FirstInd - 1);
                mecvRet.c4Value(c4ValCount);
            }
             else if (c2MethId == m_c2MethId_ReadTimeField)
            {
                tCIDLib::TCard8 c8Val;
                orbcProxy->bReadTimeByName(c4SerNum, strMoniker, strField, c8Val);
                TMEngCard8Val& mecvRet
                        = meOwner.mecvStackAtAs<TMEngCard8Val>(c4FirstInd - 1);
                mecvRet.c8Value(c8Val);
            }
             else if (c2MethId == m_c2MethId_WriteBoolField)
            {
                orbcProxy->WriteBoolByName
                (
                    strMoniker
                    , strField
                    , meOwner.bStackValAt(c4FirstInd + 2)
                    , m_sectUser
                    , tCQCKit::EDrvCmdWaits::DontCare
                );
            }
             else if (c2MethId == m_c2MethId_WriteCardField)
            {
                orbcProxy->WriteCardByName
                (
                    strMoniker
                    , strField
                    , meOwner.c4StackValAt(c4FirstInd + 2)
                    , m_sectUser
                    , tCQCKit::EDrvCmdWaits::DontCare
                );
            }
             else if (c2MethId == m_c2MethId_WriteFloatField)
            {
                orbcProxy->WriteFloatByName
                (
                    strMoniker
                    , strField
                    , meOwner.f8StackValAt(c4FirstInd + 2)
                    , m_sectUser
                    , tCQCKit::EDrvCmdWaits::DontCare
                );
            }
             else if (c2MethId == m_c2MethId_WriteIntField)
            {
                orbcProxy->WriteIntByName
                (
                    strMoniker
                    , strField
                    , meOwner.i4StackValAt(c4FirstInd + 2)
                    , m_sectUser
                    , tCQCKit::EDrvCmdWaits::DontCare
                );
            }
             else if ((c2MethId == m_c2MethId_WriteStringField)
                  ||  (c2MethId == m_c2MethId_WriteField))
            {
                // Both these have the same parms
                if (c2MethId == m_c2MethId_WriteStringField)
                {
                    orbcProxy->WriteStringByName
                    (
                        strMoniker
                        , strField
                        , meOwner.strStackValAt(c4FirstInd + 2)
                        , m_sectUser
                        , tCQCKit::EDrvCmdWaits::DontCare
                    );
                }
                 else
                {
                    orbcProxy->WriteFieldByName
                    (
                        strMoniker
                        , strField
                        , meOwner.strStackValAt(c4FirstInd + 2)
                        , m_sectUser
                        , tCQCKit::EDrvCmdWaits::DontCare
                    );
                }
            }
             else if (c2MethId == m_c2MethId_WriteStringListField)
            {
                //
                //  We have to get the caller's value into a vector of
                //  strings at the C++ level to write them out.
                //
                const TMEngVectorVal& mecvList
                    = meOwner.mecvStackAtAs<TMEngVectorVal>(c4FirstInd + 2);
                const tCIDLib::TCard4 c4ValCount = mecvList.c4ElemCount();

                // Load it up with the caller's strings
                m_colSList.RemoveAll();
                for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ValCount; c4Index++)
                {
                    const TMEngStringVal* pmecvCur = static_cast<const TMEngStringVal*>
                    (
                        &mecvList.mecvAt(meOwner, c4Index)
                    );
                    m_colSList.objAdd(pmecvCur->strValue());
                }

                // And do the write
                orbcProxy->WriteSListByName
                (
                    strMoniker
                    , strField
                    , m_colSList
                    , m_sectUser
                    , tCQCKit::EDrvCmdWaits::DontCare
                );
            }
             else if (c2MethId == m_c2MethId_WriteTimeField)
            {
                orbcProxy->WriteTimeByName
                (
                    strMoniker
                    , strField
                    , meOwner.c8StackValAt(c4FirstInd + 2)
                    , m_sectUser
                    , tCQCKit::EDrvCmdWaits::DontCare
                );
            }
             else
            {
                // Something is very wrong, since we checked above
                facCQCKit().ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kKitErrs::errcDbg_BadFldIOMethod
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Unknown
                );
            }
        }

        catch(const TError& errToCatch)
        {
            // Throw a generic read or write failed error
            ThrowAnErr
            (
                meOwner
                , bRead ? m_c4ErrReadFailed : m_c4ErrWriteFailed
                , errToCatch
            );
        }
    }
     else if ((c2MethId == m_c2MethId_ReadField2)
          ||  (c2MethId == m_c2MethId_WriteField2))
    {
        // Get the moniker.field and break it out
        const TString& strFld = meOwner.strStackValAt(c4FirstInd);
        TString strMoniker;
        TString strField;
        if (!facCQCKit().bParseFldName(strFld, strMoniker, strField))
            ThrowAnErr(meOwner, m_c4ErrBadFldName);

        try
        {
            // Get a client proxy for the CQCServer hosting this driver
            tCQCKit::TCQCSrvProxy orbcProxy = facCQCKit().orbcCQCSrvAdminProxy(strMoniker);
            if (c2MethId == m_c2MethId_ReadField2)
            {
                TMEngStringVal& mecvRet
                            = meOwner.mecvStackAtAs<TMEngStringVal>(c4FirstInd - 1);
                tCIDLib::TCard4 c4SerNum = 0;
                tCQCKit::EFldTypes eType;
                orbcProxy->bReadFieldByName
                (
                    c4SerNum, strMoniker, strField, mecvRet.strValue(), eType
                );
            }
             else
            {
                orbcProxy->WriteFieldByName
                (
                    strMoniker
                    , strField
                    , meOwner.strStackValAt(c4FirstInd + 1)
                    , m_sectUser
                    , tCQCKit::EDrvCmdWaits::DontCare
                );
            }
        }

        catch(const TError& errToCatch)
        {
            ThrowAnErr(meOwner, m_c4ErrReadFailed, errToCatch);
        }
    }
     else if (c2MethId == m_c2MethId_WaitDriverReady)
    {
        // And calculate the end time
        const tCIDLib::TEncodedTime enctEnd
        (
            TTime::enctNow()
            + (meOwner.c4StackValAt(c4FirstInd + 1) * kCIDLib::enctOneMilliSec)
        );

        // And the moniker we'll need to reference below
        const TString& strMoniker = meOwner.strStackValAt(c4FirstInd);

        // We need to do the shutdown checking type of sleep via the thread
        TThread* pthrMe = TThread::pthrCaller();

        // Get an admin proxy for the CQCServer that hosts the target device
        tCQCKit::TCQCSrvProxy orbcAdmin;
        tCIDLib::TBoolean bRetVal = kCIDLib::True;
        while (bRetVal)
        {
            try
            {
                orbcAdmin = facCQCKit().orbcCQCSrvAdminProxy(strMoniker);
                break;
            }

            catch(...)
            {
                // Just eat it and keep waiting
            }

            // If we've run out of time, then give up
            if (TTime::enctNow() >= enctEnd)
            {
                bRetVal = kCIDLib::False;
                break;
            }

            // Not yet, so sleep a bit. If we get a shutdown request, break out
            if (!pthrMe->bSleep(500))
            {
                bRetVal = kCIDLib::False;
                break;
            }
        }

        // Until we run out of time, keep waiting
        while (bRetVal)
        {
            // Get the driver status
            tCIDLib::TCard4     c4ThreadId;
            tCQCKit::EDrvStates eState;
            tCQCKit::EVerboseLvls eVerbose;
            try
            {
                orbcAdmin->QueryDriverState(strMoniker, eState, eVerbose, c4ThreadId);

                // If it's online, then we are done
                if (eState == tCQCKit::EDrvStates::Connected)
                    break;
            }

            catch(...)
            {
                // Just eat it and keep waiting
            }

            // If we've run out of time, then give up
            if (TTime::enctNow() >= enctEnd)
            {
                bRetVal = kCIDLib::False;
                break;
            }

            // Not yet, so sleep a bit. If we get a shutdown request, break out
            if (!pthrMe->bSleep(500))
            {
                bRetVal = kCIDLib::False;
                break;
            }
        }
        meOwner.mecvStackAtAs<TMEngBooleanVal>(c4FirstInd - 1).bValue(bRetVal);
    }
     else
    {
        // We don't know what it is
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


// ---------------------------------------------------------------------------
//  TSimpleFldInfo: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TSimpleFldInfo::ThrowAnErr(         TCIDMacroEngine&    meOwner
                            , const tCIDLib::TCard4     c4ToThrow) const
{
    // Set the exception info on the engine
    meOwner.SetException
    (
        m_c2EnumId_Errors
        , strClassPath()
        , c4ToThrow
        , m_pmeciErrors->strPartialName(c4ToThrow)
        , m_pmeciErrors->strTextValue(c4ToThrow)
        , meOwner.c4CurLine()
    );

    // And throw the excpetion that represents a macro level exception
    throw TExceptException();
}

tCIDLib::TVoid
TSimpleFldInfo::ThrowAnErr(         TCIDMacroEngine&    meOwner
                            , const tCIDLib::TCard4     c4ToThrow
                            , const TError&             errCaught) const
{
    if (facCQCKit().bShouldLog(errCaught))
        TModule::LogEventObj(errCaught);

    // Set the exception info on the engine
    meOwner.SetException
    (
        m_c2EnumId_Errors
        , strClassPath()
        , c4ToThrow
        , m_pmeciErrors->strPartialName(c4ToThrow)
        , errCaught.strErrText()
        , meOwner.c4CurLine()
    );

    // And throw the excpetion that represents a macro level exception
    throw TExceptException();
}



