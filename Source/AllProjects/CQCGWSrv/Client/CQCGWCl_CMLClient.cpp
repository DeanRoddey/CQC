//
// FILE NAME: CQCKit_XGWClientClass.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/16/2003
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
//  class which wraps the TXGWClient class.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCGWCl_.hpp"


// ---------------------------------------------------------------------------
//  Magic RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCGWSrvCVal,TMEngClassVal)
RTTIDecls(TCQCGWSrvCInfo,TMEngClassInfo)


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace
{
    namespace CQCMacroEng_GWClientClass
    {
        // -----------------------------------------------------------------------
        //  The names for the types that we support here. Each derivative has to
        //  be able to return strings that contain its name and full name.
        // -----------------------------------------------------------------------
        const TString   strFldDef(L"XGWClient");
        const TString   strGWClientClassPath(L"MEng.System.CQC.Runtime.XGWClient");
        const TString   strFldDefBasePath(L"MEng.System.CQC.Runtime");
    }
}


// ---------------------------------------------------------------------------
//  CLASS: TCQCGWSrvCVal
// PREFIX: mecv
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCGWSrvCVal: Constuctors and Destructor
// ---------------------------------------------------------------------------
TCQCGWSrvCVal::TCQCGWSrvCVal(const  TString&                strName
                            , const tCIDLib::TCard2         c2ClassId
                            , const tCIDMacroEng::EConstTypes  eConst) :

    TMEngClassVal(strName, c2ClassId, eConst)
{
}

TCQCGWSrvCVal::~TCQCGWSrvCVal()
{
}


// ---------------------------------------------------------------------------
//  TCQCGWSrvCVal: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TCQCGWSrvCVal::bDbgFormat(          TTextOutStream&     strmTarget
                            , const TMEngClassInfo&
                            , const tCIDMacroEng::EDbgFmts eFormat
                            , const tCIDLib::ERadices
                            , const TCIDMacroEngine&    ) const
{
    if (eFormat == tCIDMacroEng::EDbgFmts::Long)
    {
        return kCIDLib::True;
    }
    return kCIDLib::False;
}



// ---------------------------------------------------------------------------
//  TCQCGWSrvCVal: Public, non-virtual methods
// ---------------------------------------------------------------------------
const TCQCGWSrvClient& TCQCGWSrvCVal::gwscValue() const
{
    return m_gwscValue;
}

TCQCGWSrvClient& TCQCGWSrvCVal::gwscValue()
{
    return m_gwscValue;
}




// ---------------------------------------------------------------------------
//  CLASS: TCQCGWSrvCInfo
// PREFIX: meci
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCGWSrvCInfo: Public, static methods
// ---------------------------------------------------------------------------
const TString& TCQCGWSrvCInfo::strPath()
{
    return CQCMacroEng_GWClientClass::strGWClientClassPath;
}


// ---------------------------------------------------------------------------
//  TCQCGWSrvCInfo: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCGWSrvCInfo::TCQCGWSrvCInfo(TCIDMacroEngine& meOwner) :

    TMEngClassInfo
    (
        CQCMacroEng_GWClientClass::strFldDef
        , CQCMacroEng_GWClientClass::strFldDefBasePath
        , meOwner
        , kCIDLib::False
        , tCIDMacroEng::EClassExt::Final
        , L"MEng.Object"
    )
    , m_c2EnumId_Errors(kCIDMacroEng::c2BadId)
    , m_c2MethId_DefCtor(kCIDMacroEng::c2BadId)
    , m_c2MethId_Connect(kCIDMacroEng::c2BadId)
    , m_c2MethId_CheckField(kCIDMacroEng::c2BadId)
    , m_c2MethId_Disconnect(kCIDMacroEng::c2BadId)
    , m_c2MethId_DoGlobalAct(kCIDMacroEng::c2BadId)
    , m_c2MethId_GetPollListCount(kCIDMacroEng::c2BadId)
    , m_c2MethId_IsConnected(kCIDMacroEng::c2BadId)
    , m_c2MethId_MWriteField(kCIDMacroEng::c2BadId)
    , m_c2MethId_Ping(kCIDMacroEng::c2BadId)
    , m_c2MethId_Poll(kCIDMacroEng::c2BadId)
    , m_c2MethId_RunMacro(kCIDMacroEng::c2BadId)
    , m_c2MethId_SetPollList(kCIDMacroEng::c2BadId)
    , m_c2MethId_WriteField(kCIDMacroEng::c2BadId)
    , m_c4Err_AlreadyConn(kCIDLib::c4MaxCard)
    , m_c4Err_Connect(kCIDLib::c4MaxCard)
    , m_c4Err_Disconnect(kCIDLib::c4MaxCard)
    , m_c4Err_FldIndex(kCIDLib::c4MaxCard)
    , m_c4Err_GlobalAct(kCIDLib::c4MaxCard)
    , m_c4Err_ListSizes(kCIDLib::c4MaxCard)
    , m_c4Err_Poll(kCIDLib::c4MaxCard)
    , m_c4Err_NotConnected(kCIDLib::c4MaxCard)
    , m_c4Err_RunMacro(kCIDLib::c4MaxCard)
    , m_c4Err_SetPollList(kCIDLib::c4MaxCard)
    , m_c4Err_WriteField(kCIDLib::c4MaxCard)
    , m_c4Err_MWriteField(kCIDLib::c4MaxCard)
    , m_pmeciErrors(nullptr)
    , m_pmeciStrList(nullptr)
{
}

TCQCGWSrvCInfo::~TCQCGWSrvCInfo()
{
}


tCIDLib::TVoid TCQCGWSrvCInfo::Init(TCIDMacroEngine& meOwner)
{
    // Create an enumerated type for our errors
    {
        m_pmeciErrors = new TMEngEnumInfo
        (
            meOwner
            , L"XGWCErrors"
            , strClassPath()
            , L"MEng.Enum"
            , 12
        );
        m_c4Err_AlreadyConn = m_pmeciErrors->c4AddEnumItem(L"AlreadyConn", L"The XML GW client is already connected");
        m_c4Err_Connect = m_pmeciErrors->c4AddEnumItem(L"Connect", TString::strEmpty());
        m_c4Err_Disconnect = m_pmeciErrors->c4AddEnumItem(L"Disconnect", TString::strEmpty());
        m_c4Err_GlobalAct = m_pmeciErrors->c4AddEnumItem(L"GlobalAct", TString::strEmpty());
        m_c4Err_FldIndex = m_pmeciErrors->c4AddEnumItem(L"FldIndex", L"%(1) is not a valid poll list index");
        m_c4Err_ListSizes = m_pmeciErrors->c4AddEnumItem(L"ListSizes", L"All passed lists must be the same size");
        m_c4Err_NotConnected = m_pmeciErrors->c4AddEnumItem(L"NotConnected", L"You must call Connect() before this GW client operation");
        m_c4Err_Ping = m_pmeciErrors->c4AddEnumItem(L"Ping", TString::strEmpty());
        m_c4Err_Poll = m_pmeciErrors->c4AddEnumItem(L"Poll", TString::strEmpty());
        m_c4Err_RunMacro = m_pmeciErrors->c4AddEnumItem(L"RunMacro", TString::strEmpty());
        m_c4Err_SetPollList = m_pmeciErrors->c4AddEnumItem(L"SetPollList", TString::strEmpty());
        m_c4Err_WriteField = m_pmeciErrors->c4AddEnumItem(L"WriteField", TString::strEmpty());
        m_pmeciErrors->BaseClassInit(meOwner);
        m_c2EnumId_Errors = meOwner.c2AddClass(m_pmeciErrors);
        bAddNestedType(m_pmeciErrors->strClassPath());
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


    // Add the default constructor
    {
        TMEngMethodInfo methiNew
        (
            L"ctor1_MEng.System.CQC.Runtime.XGWClient"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.bIsCtor(kCIDLib::True);
        m_c2MethId_DefCtor = c2AddMethodInfo(methiNew);
    }

    // Connect to an XML Gateway and log in
    {
        TMEngMethodInfo methiNew
        (
            L"Connect"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"ServerAddr", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"Port", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddInParm(L"UserName", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"Password", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"Secure", tCIDMacroEng::EIntrinsics::Boolean);
        m_c2MethId_Connect = c2AddMethodInfo(methiNew);
    }

    // Check a field for changes (by index into the poll list)
    {
        TMEngMethodInfo methiNew
        (
            L"CheckField"
            , tCIDMacroEng::EIntrinsics::Boolean
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"Index", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddOutParm(L"State", tCIDMacroEng::EIntrinsics::Boolean);
        methiNew.c2AddOutParm(L"Value", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_CheckField = c2AddMethodInfo(methiNew);
    }

    // Disconnect from the server gracefully
    {
        TMEngMethodInfo methiNew
        (
            L"Disconnect"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        m_c2MethId_Disconnect = c2AddMethodInfo(methiNew);
    }

    // Run a global action on the GW server
    {
        TMEngMethodInfo methiNew
        (
            L"DoGlobalAct"
            , tCIDMacroEng::EIntrinsics::Boolean
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"ActPath", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"Parms", m_pmeciStrList->c2Id());
        methiNew.c2AddOutParm(L"FailReason", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_DoGlobalAct = c2AddMethodInfo(methiNew);
    }

    // Get the number of fields currently in the poll list
    {
        TMEngMethodInfo methiNew
        (
            L"GetPollListCount"
            , tCIDMacroEng::EIntrinsics::Card4
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        m_c2MethId_GetPollListCount = c2AddMethodInfo(methiNew);
    }

    // Check that the connection is still good
    {
        TMEngMethodInfo methiNew
        (
            L"IsConnected"
            , tCIDMacroEng::EIntrinsics::Boolean
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        m_c2MethId_IsConnected = c2AddMethodInfo(methiNew);
    }

    // Write multiple fields
    {
        TMEngMethodInfo methiNew
        (
            L"MWriteField"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"MonikerList", m_pmeciStrList->c2Id());
        methiNew.c2AddInParm(L"FieldList", m_pmeciStrList->c2Id());
        methiNew.c2AddInParm(L"ValueList", m_pmeciStrList->c2Id());
        m_c2MethId_MWriteField = c2AddMethodInfo(methiNew);
    }

    // Do a ping
    {
        TMEngMethodInfo methiNew
        (
            L"Ping"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        m_c2MethId_Ping = c2AddMethodInfo(methiNew);
    }

    // Do a poll
    {
        TMEngMethodInfo methiNew
        (
            L"Poll"
            , tCIDMacroEng::EIntrinsics::Boolean
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        m_c2MethId_Poll = c2AddMethodInfo(methiNew);
    }

    // Run a macro on the GW server
    {
        TMEngMethodInfo methiNew
        (
            L"RunMacro"
            , tCIDMacroEng::EIntrinsics::Boolean
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"ClassPath", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"Parms", m_pmeciStrList->c2Id());
        methiNew.c2AddOutParm(L"Output", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddOutParm(L"ResCode", tCIDMacroEng::EIntrinsics::Int4);
        m_c2MethId_RunMacro = c2AddMethodInfo(methiNew);
    }

    // Set the poll list
    {
        TMEngMethodInfo methiNew
        (
            L"SetPollList"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"Fields", m_pmeciStrList->c2Id());
        m_c2MethId_SetPollList = c2AddMethodInfo(methiNew);
    }

    // Write a field
    {
        TMEngMethodInfo methiNew
        (
            L"WriteField"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"Moniker", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"Field", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"Value", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_WriteField = c2AddMethodInfo(methiNew);
    }
}


TMEngClassVal*
TCQCGWSrvCInfo::pmecvMakeStorage(const  TString&                strName
                                ,       TCIDMacroEngine&
                                , const tCIDMacroEng::EConstTypes  eConst) const
{
    return new TCQCGWSrvCVal(strName, c2Id(), eConst);
}



// ---------------------------------------------------------------------------
//  TCQCGWSrvCInfo: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TCQCGWSrvCInfo::bInvokeMethod(          TCIDMacroEngine&    meOwner
                                , const TMEngMethodInfo&    methiTarget
                                ,       TMEngClassVal&      mecvInstance)
{
    TCQCGWSrvCVal& mecvActual = static_cast<TCQCGWSrvCVal&>(mecvInstance);
    TCQCGWSrvClient& gwscThis = mecvActual.gwscValue();

    const tCIDLib::TCard4 c4FirstInd = meOwner.c4FirstParmInd(methiTarget);
    const tCIDLib::TCard2 c2MethId   = methiTarget.c2Id();

    if (c2MethId == m_c2MethId_DefCtor)
    {
        // Nothing to do
    }
     else if (c2MethId == m_c2MethId_Connect)
    {
        try
        {
            TIPEndPoint ipepSrv
            (
                meOwner.strStackValAt(c4FirstInd)
                , meOwner.c4StackValAt(c4FirstInd + 1)
                , tCIDSock::EAddrTypes::Unspec
            );

            gwscThis.Connect
            (
                ipepSrv
                , meOwner.strStackValAt(c4FirstInd + 2)
                , meOwner.strStackValAt(c4FirstInd + 3)
                , meOwner.bStackValAt(c4FirstInd + 4)
            );
        }

        catch(const TError& errToCatch)
        {
            ThrowAnErr(meOwner, m_c4Err_Connect, errToCatch);
        }
    }
     else if (c2MethId == m_c2MethId_CheckField)
    {
        CheckConnected(meOwner, gwscThis);

        // Get the index and make sure it's ok
        const tCIDLib::TCard4 c4Index = meOwner.c4StackValAt(c4FirstInd);
        if (c4Index >= gwscThis.c4PollListCount())
            ThrowAnErr(meOwner, m_c4Err_FldIndex, TCardinal(c4Index));

        // Looks already to check the field and get the values into locals
        tCIDLib::TBoolean bState;
        TString strValue;
        const tCIDLib::TBoolean bRes = gwscThis.bCheckField
        (
            c4Index, bState, strValue
        );

        // Store the changed flag as the return value
        meOwner.mecvStackAtAs<TMEngBooleanVal>(c4FirstInd - 1).bValue(bRes);
        if (bRes)
        {
            // It changed, so return the state and value
            meOwner.mecvStackAtAs<TMEngBooleanVal>(c4FirstInd + 1).bValue(bState);
            meOwner.mecvStackAtAs<TMEngStringVal>(c4FirstInd + 2).strValue(strValue);
        }
    }
     else if (c2MethId == m_c2MethId_Disconnect)
    {
        try
        {
            gwscThis.Disconnect();
        }

        catch(const TError& errToCatch)
        {
            ThrowAnErr(meOwner, m_c4Err_Disconnect, errToCatch);
        }
    }
     else if (c2MethId == m_c2MethId_DoGlobalAct)
    {
        CheckConnected(meOwner, gwscThis);

        //
        //  Get the CML level vector which holds the parms and get them
        //  into a C++ vector.
        //
        TMEngVectorVal& mecvParms = meOwner.mecvStackAtAs<TMEngVectorVal>(c4FirstInd + 1);
        const tCIDLib::TCard4 c4Count = mecvParms.c4ElemCount();
        TVector<TString> colParms(c4Count);
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            colParms.objAdd
            (
                static_cast<const TMEngStringVal&>
                (
                    mecvParms.mecvAt(meOwner, c4Index)
                ).strValue()
            );
        }

        try
        {
            TString strFailReason;
            const tCIDLib::TBoolean bRes = gwscThis.bDoGlobalAct
            (
                meOwner.strStackValAt(c4FirstInd), colParms, strFailReason
            );

            // Store the result
            meOwner.mecvStackAtAs<TMEngBooleanVal>(c4FirstInd - 1).bValue(bRes);

            // Store the fail reason, which may just be empty
            meOwner.mecvStackAtAs<TMEngStringVal>(c4FirstInd + 2).strValue(strFailReason);
        }

        catch(const TError& errToCatch)
        {
            ThrowAnErr(meOwner, m_c4Err_GlobalAct, errToCatch);
        }
    }
     else if (c2MethId == m_c2MethId_GetPollListCount)
    {
        CheckConnected(meOwner, gwscThis);
        meOwner.mecvStackAtAs<TMEngCard4Val>(c4FirstInd - 1).c4Value
        (
            gwscThis.c4PollListCount()
        );
    }
     else if (c2MethId == m_c2MethId_IsConnected)
    {
        meOwner.mecvStackAtAs<TMEngBooleanVal>(c4FirstInd - 1).bValue
        (
            gwscThis.bIsConnected()
        );
    }
     else if (c2MethId == m_c2MethId_MWriteField)
    {
        CheckConnected(meOwner, gwscThis);

        // Check that all of the incoming lists are the same size
        TMEngVectorVal& mecvMons = meOwner.mecvStackAtAs<TMEngVectorVal>(c4FirstInd);
        TMEngVectorVal& mecvFlds = meOwner.mecvStackAtAs<TMEngVectorVal>(c4FirstInd + 1);
        TMEngVectorVal& mecvVals = meOwner.mecvStackAtAs<TMEngVectorVal>(c4FirstInd + 2);

        const tCIDLib::TCard4 c4Count = mecvMons.c4ElemCount();
        if ((mecvFlds.c4ElemCount() != c4Count)
        ||  (mecvVals.c4ElemCount() != c4Count))
        {
            ThrowAnErr(meOwner, m_c4Err_ListSizes);
        }

        // They seem to be, so build up the C++ versions of these lists
        try
        {
            TVector<TString> colMons(c4Count);
            TVector<TString> colFlds(c4Count);
            TVector<TString> colVals(c4Count);
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            {
                colMons.objAdd
                (
                    static_cast<const TMEngStringVal&>
                    (
                        mecvMons.mecvAt(meOwner, c4Index)
                    ).strValue()
                );

                colFlds.objAdd
                (
                    static_cast<const TMEngStringVal&>
                    (
                        mecvFlds.mecvAt(meOwner, c4Index)
                    ).strValue()
                );

                colVals.objAdd
                (
                    static_cast<const TMEngStringVal&>
                    (
                        mecvVals.mecvAt(meOwner, c4Index)
                    ).strValue()
                );
            }

            // And now pass the call on to the C++ client
            gwscThis.MWriteField(colMons, colFlds, colVals);
        }

        catch(const TError& errToCatch)
        {
            ThrowAnErr(meOwner, m_c4Err_WriteField, errToCatch);
        }
    }
     else if (c2MethId == m_c2MethId_Ping)
    {
        CheckConnected(meOwner, gwscThis);

        try
        {
            gwscThis.Ping();
        }

        catch(const TError& errToCatch)
        {
            ThrowAnErr(meOwner, m_c4Err_Ping, errToCatch);
        }
    }
     else if (c2MethId == m_c2MethId_Poll)
    {
        CheckConnected(meOwner, gwscThis);

        try
        {
            const tCIDLib::TBoolean bRes = gwscThis.bPollFields();
            meOwner.mecvStackAtAs<TMEngBooleanVal>(c4FirstInd - 1).bValue(bRes);
        }

        catch(const TError& errToCatch)
        {
            ThrowAnErr(meOwner, m_c4Err_Poll, errToCatch);
        }
    }
     else if (c2MethId == m_c2MethId_RunMacro)
    {
        CheckConnected(meOwner, gwscThis);

        //
        //  Get the CML level vector which holds the parms and get them
        //  into a C++ vector.
        //
        TMEngVectorVal& mecvParms = meOwner.mecvStackAtAs<TMEngVectorVal>(c4FirstInd + 1);
        const tCIDLib::TCard4 c4Count = mecvParms.c4ElemCount();
        TVector<TString> colParms(c4Count);
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            colParms.objAdd
            (
                static_cast<const TMEngStringVal&>
                (
                    mecvParms.mecvAt(meOwner, c4Index)
                ).strValue()
            );
        }

        try
        {
            tCIDLib::TInt4  i4ResCode;
            TString         strOutput;
            const tCIDLib::TBoolean bRes = gwscThis.bRunMacro
            (
                meOwner.strStackValAt(c4FirstInd), colParms, strOutput, i4ResCode
            );

            // Store the result
            meOwner.mecvStackAtAs<TMEngBooleanVal>(c4FirstInd - 1).bValue(bRes);

            //
            //  Store the output and result code, though they may be
            //  meaningless if the macro never ran.
            //
            meOwner.mecvStackAtAs<TMEngInt4Val>(c4FirstInd + 3).i4Value(i4ResCode);
            meOwner.mecvStackAtAs<TMEngStringVal>(c4FirstInd + 2).strValue(strOutput);
        }

        catch(const TError& errToCatch)
        {
            ThrowAnErr(meOwner, m_c4Err_RunMacro, errToCatch);
        }
    }
     else if (c2MethId == m_c2MethId_SetPollList)
    {
        CheckConnected(meOwner, gwscThis);

        //
        //  Get the CML level vector which holds the field names and
        //  put them into a C++ vector.
        //
        TMEngVectorVal& mecvList = meOwner.mecvStackAtAs<TMEngVectorVal>(c4FirstInd);
        const tCIDLib::TCard4 c4Count = mecvList.c4ElemCount();
        TVector<TString> colFlds(c4Count);
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            colFlds.objAdd
            (
                static_cast<const TMEngStringVal&>
                (
                    mecvList.mecvAt(meOwner, c4Index)
                ).strValue()
            );
        }

        try
        {
            gwscThis.SetPollList(colFlds);

            //
            //  Pause just a bit to give the server time to get ahead
            //  of us and get the initial field values for us. We'll use
            //  the shutdown sensing version so that we'll exit immediately
            //  if our thread is asked to stop.
            //
            TThread::pthrCaller()->bSleep(1500);
        }

        catch(const TError& errToCatch)
        {
            ThrowAnErr(meOwner, m_c4Err_SetPollList, errToCatch);
        }
    }
     else if (c2MethId == m_c2MethId_WriteField)
    {
        CheckConnected(meOwner, gwscThis);

        try
        {
            gwscThis.WriteField
            (
                meOwner.strStackValAt(c4FirstInd)
                , meOwner.strStackValAt(c4FirstInd + 1)
                , meOwner.strStackValAt(c4FirstInd + 2)
            );
        }

        catch(const TError& errToCatch)
        {
            ThrowAnErr(meOwner, m_c4Err_WriteField, errToCatch);
        }
    }
     else
    {
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


// ---------------------------------------------------------------------------
//  TCQCGWSrvCInfo: Protected, inherited methods
// ---------------------------------------------------------------------------

//
//  Called before doing any calls that would have required Connect to be
//  called successfully. The C++ class would throw an error, but this lets
//  us throw a specific CML error.
//
tCIDLib::TVoid
TCQCGWSrvCInfo::CheckConnected(         TCIDMacroEngine& meOwner
                                , const TCQCGWSrvClient& gwscTest) const
{
    if (!gwscTest.bIsConnected())
        ThrowAnErr(meOwner, m_c4Err_NotConnected);
}


// Helpers to handle throwing various configurations of exceptions
tCIDLib::TVoid
TCQCGWSrvCInfo::ThrowAnErr(         TCIDMacroEngine&    meOwner
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

tCIDLib::TVoid
TCQCGWSrvCInfo::ThrowAnErr(         TCIDMacroEngine&    meOwner
                            , const tCIDLib::TCard4     c4ToThrow) const
{
    TString strErrText(m_pmeciErrors->strTextValue(c4ToThrow));

    // Set the exception info on the engine
    meOwner.SetException
    (
        m_c2EnumId_Errors
        , strClassPath()
        , c4ToThrow
        , m_pmeciErrors->strPartialName(c4ToThrow)
        , strErrText
        , meOwner.c4CurLine()
    );

    // And throw the excpetion that represents a macro level exception
    throw TExceptException();
}

tCIDLib::TVoid
TCQCGWSrvCInfo::ThrowAnErr(         TCIDMacroEngine&    meOwner
                            , const tCIDLib::TCard4     c4ToThrow
                            , const MFormattable&       fmtblToken1) const
{
    TString strErrText(m_pmeciErrors->strTextValue(c4ToThrow));
    strErrText.eReplaceToken(fmtblToken1, kCIDLib::chDigit1);

    // Set the exception info on the engine
    meOwner.SetException
    (
        m_c2EnumId_Errors
        , strClassPath()
        , c4ToThrow
        , m_pmeciErrors->strPartialName(c4ToThrow)
        , strErrText
        , meOwner.c4CurLine()
    );

    // And throw the excpetion that represents a macro level exception
    throw TExceptException();
}

