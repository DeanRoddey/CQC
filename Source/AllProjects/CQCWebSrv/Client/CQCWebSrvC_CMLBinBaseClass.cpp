//
// FILE NAME: CQCKit_CMLBinBaseClass.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/02/2003
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
//  This file implements the info and value classes to create a CML level
//  class that is used as the base class for all 'CMLBin' classes (which
//  are used in the web server for CML-based web page generation.)
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCWebSrvC_.hpp"


// ---------------------------------------------------------------------------
//  Magic RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TCMLBinBaseInfo,TMEngClassInfo)


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace CQCWebSrvC_CMLBinBase
{
    namespace
    {
        // -----------------------------------------------------------------------
        //  The names for the types that we support here. Each derivative has to
        //  be able to return strings that contain its name and full name.
        // -----------------------------------------------------------------------
        const TString   strCMLBinBase(L"CMLBinBase");
        const TString   strCMLBinBaseClassPath(L"MEng.System.CQC.Runtime.CMLBinBase");
        const TString   strCMLBinBaseBasePath(L"MEng.System.CQC.Runtime");
    }
}


// ---------------------------------------------------------------------------
//  CLASS: TCMLBinBaseInfo
// PREFIX: meci
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCMLBinBaseInfo: Public, static methods
// ---------------------------------------------------------------------------
const TString TCMLBinBaseInfo::strContBoundaryName(L"m_ContBoundary");
const TString TCMLBinBaseInfo::strContBufLenName(L"m_ContBufLen");
const TString TCMLBinBaseInfo::strContBufName(L"m_ContBuf");
const TString TCMLBinBaseInfo::strContEncodingName(L"m_ContEncoding");
const TString TCMLBinBaseInfo::strContTypeName(L"m_ContType");
const TString TCMLBinBaseInfo::strProcessReqMeth(L"ProcessReq");
const TString TCMLBinBaseInfo::strValListName(L"m_ReqValues");

const TString& TCMLBinBaseInfo::strPath()
{
    return CQCWebSrvC_CMLBinBase::strCMLBinBaseClassPath;
}


// ---------------------------------------------------------------------------
//  TCMLBinBaseInfo: Constructors and Destructor
// ---------------------------------------------------------------------------
TCMLBinBaseInfo::TCMLBinBaseInfo(TCIDMacroEngine& meOwner) :

    TMEngClassInfo
    (
        CQCWebSrvC_CMLBinBase::strCMLBinBase
        , CQCWebSrvC_CMLBinBase::strCMLBinBaseBasePath
        , meOwner
        , kCIDLib::False
        , tCIDMacroEng::EClassExt::Abstract
        , L"MEng.Object"
    )
    , m_c2EnumId_Errors(kCIDMacroEng::c2BadId)
    , m_c2MethId_AddReqValue(kCIDMacroEng::c2BadId)
    , m_c2MethId_BuildErrReply(kCIDMacroEng::c2BadId)
    , m_c2MethId_DefCtor(kCIDMacroEng::c2BadId)
    , m_c2MethId_EscapeText(kCIDMacroEng::c2BadId)
    , m_c2MethId_FindReqValue(kCIDMacroEng::c2BadId)
    , m_c2MethId_ProcessReq(kCIDMacroEng::c2BadId)
    , m_c2MethId_QueryBodyContent(kCIDMacroEng::c2BadId)
    , m_c2MethId_SetInBodyCont(kCIDMacroEng::c2BadId)
    , m_c4ErrBldErrReply(kCIDLib::c4MaxCard)
    , m_c4ErrEscape(kCIDLib::c4MaxCard)
    , m_c4ErrReqVal(kCIDLib::c4MaxCard)
    , m_pmeciErrors(nullptr)
    , m_pmeciKVPair(nullptr)
    , m_pmeciList(nullptr)
{
    //
    //  For imports of any non-intrinsic classes we reference. The instantiated
    //  vector (of KVPairs) has to be done dynamically below.
    //
    bAddClassImport(TMEngKVPairInfo::strPath());
}

TCMLBinBaseInfo::~TCMLBinBaseInfo()
{
}


tCIDLib::TVoid TCMLBinBaseInfo::Init(TCIDMacroEngine& meOwner)
{
    //
    //  We need to create a vector of KVPair objects as a nested type, so look
    //  up the KVPair type since it's non-intrinsic.
    //
    m_pmeciKVPair = meOwner.pmeciFindAs<TMEngKVPairInfo>
    (
        TMEngKVPairInfo::strPath(), kCIDLib::True
    );

    {
        m_pmeciList = new TMEngVectorInfo
        (
            meOwner
            , L"CMLBinValues"
            , CQCWebSrvC_CMLBinBase::strCMLBinBaseClassPath
            , TMEngVectorInfo::strPath()
            , m_pmeciKVPair->c2Id()
        );
        m_pmeciList->BaseClassInit(meOwner);
        meOwner.c2AddClass(m_pmeciList);
        bAddNestedType(m_pmeciList->strClassPath());
    }

    // Create an enumerated type for our errors
    {
        m_pmeciErrors = new TMEngEnumInfo
        (
            meOwner
            , L"CMLBinErrors"
            , strClassPath()
            , L"MEng.Enum"
            , 3
        );
        m_c4ErrBldErrReply = m_pmeciErrors->c4AddEnumItem(L"ErrReplyErr", TString::strEmpty());
        m_c4ErrEscape = m_pmeciErrors->c4AddEnumItem(L"EscapeErr", TString::strEmpty());
        m_c4ErrReqVal = m_pmeciErrors->c4AddEnumItem(L"ReqValErr", TString::strEmpty());
        m_pmeciErrors->BaseClassInit(meOwner);
        m_c2EnumId_Errors = meOwner.c2AddClass(m_pmeciErrors);
        bAddNestedType(m_pmeciErrors->strClassPath());
    }

    //
    //  Register the CML level data members we want our instances to have, so that we
    //  can use a helper method in our base class to have them loaded into the value
    //  objects when we make per-instance storage below.
    //
    {
        TMEngMemberInfo memiValues
        (
            strValListName, m_pmeciList->c2Id(), tCIDMacroEng::EConstTypes::Const
        );
        c2AddMember(memiValues, *m_pmeciList);
    }

    {
        TMEngMemberInfo memiContBuf
        (
            strContBufName
            , tCIDLib::TCard2(tCIDMacroEng::EIntrinsics::MemBuf)
            , tCIDMacroEng::EConstTypes::Const
        );
        c2AddMember(memiContBuf, meOwner.meciFind(tCIDMacroEng::EIntrinsics::MemBuf));
    }

    {
        TMEngMemberInfo memiContBufLen
        (
            strContBufLenName
            , tCIDLib::TCard2(tCIDMacroEng::EIntrinsics::Card4)
            , tCIDMacroEng::EConstTypes::Const
        );
        c2AddMember(memiContBufLen, meOwner.meciFind(tCIDMacroEng::EIntrinsics::Card4));
    }

    {
        TMEngMemberInfo memiContType
        (
            strContTypeName
            , tCIDLib::TCard2(tCIDMacroEng::EIntrinsics::String)
            , tCIDMacroEng::EConstTypes::Const
        );
        c2AddMember(memiContType, meOwner.meciFind(tCIDMacroEng::EIntrinsics::String));
    }

    {
        TMEngMemberInfo memiContEncoding
        (
            strContEncodingName
            , tCIDLib::TCard2(tCIDMacroEng::EIntrinsics::String)
            , tCIDMacroEng::EConstTypes::Const
        );
        c2AddMember(memiContEncoding, meOwner.meciFind(tCIDMacroEng::EIntrinsics::String));
    }

    {
        TMEngMemberInfo memiContBoundary
        (
            strContBoundaryName
            , tCIDLib::TCard2(tCIDMacroEng::EIntrinsics::String)
            , tCIDMacroEng::EConstTypes::Const
        );
        c2AddMember(memiContBoundary, meOwner.meciFind(tCIDMacroEng::EIntrinsics::String));
    }

    // And our CML level methods

    //
    //  Add a value to the list of query/post values, mostly for testing purposes, so
    //  that the derived class can load up values that it will then see when it calls
    //  its own processing method.
    //
    {
        TMEngMethodInfo methiNew
        (
            L"AddReqValue"
            , tCIDMacroEng::EIntrinsics::Boolean
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"ToAdd", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"Value", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_AddReqValue = c2AddMethodInfo(methiNew);
    }

    // Build a default error reply
    {
        TMEngMethodInfo methiNew
        (
            L"BuildErrReply"
            , tCIDMacroEng::EIntrinsics::Card4
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInParm(L"ErrCode", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddInParm(L"ErrText", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddOutParm(L"ContType", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddOutParm(L"ToFill", tCIDMacroEng::EIntrinsics::MemBuf);
        m_c2MethId_BuildErrReply = c2AddMethodInfo(methiNew);
    }

    // Add the default constructor
    {
        TMEngMethodInfo methiNew
        (
            L"ctor1_MEng.System.CQC.Runtime.CMLBinBase"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.bIsCtor(kCIDLib::True);
        m_c2MethId_DefCtor = c2AddMethodInfo(methiNew);
    }

    // Escape the input string to the output stream, for HTML body text
    {
        TMEngMethodInfo methiNew
        (
            L"EscapeText"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInParm(L"SrcText", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddOutParm(L"TarStrm", tCIDMacroEng::EIntrinsics::TextOutStream);
        m_c2MethId_EscapeText = c2AddMethodInfo(methiNew);
    }

    // Find the indicated query value in our value list
    {
        TMEngMethodInfo methiNew
        (
            L"FindReqValue"
            , tCIDMacroEng::EIntrinsics::Boolean
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInParm(L"ToFind", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddOutParm(L"ToFill", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_FindReqValue = c2AddMethodInfo(methiNew);
    }

    // The virtual request processing method
    {
        TMEngMethodInfo methiNew
        (
            strProcessReqMeth
            , tCIDMacroEng::EIntrinsics::Card4
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Required
        );
        methiNew.c2AddOutParm(L"ToFill", tCIDMacroEng::EIntrinsics::MemBuf);
        methiNew.c2AddOutParm(L"ContLen", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddOutParm(L"ContType", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddOutParm(L"RepText", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"IsAGet", tCIDMacroEng::EIntrinsics::Boolean);
        m_c2MethId_ProcessReq = c2AddMethodInfo(methiNew);
    }


    //
    //  Let the derived class get any body content we got. If it returns zero
    //  then there was none.
    //
    {
        TMEngMethodInfo methiNew
        (
            L"QueryBodyContent"
            , tCIDMacroEng::EIntrinsics::Card4
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddOutParm(L"ToFill", tCIDMacroEng::EIntrinsics::MemBuf);
        methiNew.c2AddOutParm(L"ContType", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddOutParm(L"Encoding", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddOutParm(L"Boundary", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_QueryBodyContent = c2AddMethodInfo(methiNew);
    }


    //
    //  For testing purposes, allows the macro to force content into the incoming
    //  body content and encoding members.
    //
    {
        TMEngMethodInfo methiNew
        (
            L"SetInBodyCont"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::NonConst
        );
        methiNew.c2AddInParm(L"ToSet", tCIDMacroEng::EIntrinsics::MemBuf);
        methiNew.c2AddInParm(L"ContBytes", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddInParm(L"ContType", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"Encoding", tCIDMacroEng::EIntrinsics::String);
        methiNew.c2AddInParm(L"Boundary", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_SetInBodyCont = c2AddMethodInfo(methiNew);
    }
}


TMEngClassVal*
TCMLBinBaseInfo::pmecvMakeStorage(  const   TString&                strName
                                    ,       TCIDMacroEngine&        meOwner
                                    , const tCIDMacroEng::EConstTypes  eConst) const
{
    //
    //  Create a new value object (just a standard class value), then load it
    //  with any CML level members we defined for our class.
    //
    TMEngStdClassVal* pmecvNew = new TMEngStdClassVal(strName, c2Id(), eConst);
    TJanitor<TMEngStdClassVal> janNew(pmecvNew);
    LoadMemberVals(*pmecvNew, meOwner);
    return janNew.pobjOrphan();
}


// ---------------------------------------------------------------------------
//  TCMLBinBaseInfo: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TCMLBinBaseInfo::bInvokeMethod(         TCIDMacroEngine&    meOwner
                                , const TMEngMethodInfo&    methiTarget
                                ,       TMEngClassVal&      mecvInstance)
{
    const tCIDLib::TCard4 c4FirstInd = meOwner.c4FirstParmInd(methiTarget);
    const tCIDLib::TCard2 c2MethId   = methiTarget.c2Id();

    if (c2MethId == m_c2MethId_DefCtor)
    {
        // Nothing to do
    }
     else if (c2MethId == m_c2MethId_BuildErrReply)
    {
        try
        {
            //
            //  We use a standard one from THTTPClient. It wants an output stream,
            //  so put a text output stream over the caller's buffer.
            //
            TTextMBufOutStream strmTar
            (
                &meOwner.mecvStackAtAs<TMEngMemBufVal>(c4FirstInd + 3).mbufValue()
                , tCIDLib::EAdoptOpts::NoAdopt
                , new TUTF8Converter
            );

            // Set the cont type to UTF-8 since that's what we set on the converter
            meOwner.mecvStackAtAs<TMEngStringVal>(c4FirstInd + 2).strValue(L"utf-8");
            THTTPClient::BuildErrReply
            (
                meOwner.c4StackValAt(c4FirstInd)
                , meOwner.strStackValAt(c4FirstInd + 1)
                , strmTar
            );

            //
            //  Flush the stream to make sure all the data is in the buffer, and
            //  then give back the size of data we formatted out.
            //
            strmTar.Flush();
            meOwner.mecvStackAtAs<TMEngCard4Val>(c4FirstInd - 1).c4Value(strmTar.c4CurSize());
        }

        catch(const TError& errToCatch)
        {
            ThrowAnErr(meOwner, m_c4ErrBldErrReply, errToCatch.strErrText());
        }
    }
     else if (c2MethId == m_c2MethId_EscapeText)
    {
        try
        {
            TMEngTextOutStreamVal& mecvStrm
            (
                meOwner.mecvStackAtAs<TMEngTextOutStreamVal>(c4FirstInd + 1)
            );

            THTTPClient::EscapeBodyText
            (
                meOwner.strStackValAt(c4FirstInd)
                , mecvStrm.strmTarget(meOwner)
            );
        }

        catch(const TError& errToCatch)
        {
            ThrowAnErr(meOwner, m_c4ErrEscape, errToCatch.strErrText());
        }
    }
     else if ((c2MethId == m_c2MethId_AddReqValue)
          ||  (c2MethId == m_c2MethId_FindReqValue))
    {
        try
        {
            //
            //  These both have a first parm that's a value to find or to add,
            //  so in either case we need to find the target value.
            //
            const TString& strToFind = meOwner.strStackValAt(c4FirstInd);

            // We have to extract the CML level collection. Cast to it's actual type
            TMEngVectorVal* pmecvVals = static_cast<TMEngVectorVal*>
            (
                mecvInstance.pmecvFind(strValListName)
            );

            const tCIDLib::TCard4 c4Count = pmecvVals->c4ElemCount();
            tCIDLib::TCard4 c4Index = 0;
            for (; c4Index < c4Count; c4Index++)
            {
                // Get the CML level KVPair pair object at this index
                const TMEngKVPairVal& mecvCur = static_cast<const TMEngKVPairVal&>
                (
                    pmecvVals->mecvAt(meOwner, c4Index)
                );

                if (mecvCur.strKey() == strToFind)
                    break;
            }

            //
            //  Set the return value, which will will indicate that the key
            //  was found if finding, or that it was added if adding (i.e.
            //  not just updated.
            //
            TMEngBooleanVal& mecvRet(meOwner.mecvStackAtAs<TMEngBooleanVal>(c4FirstInd - 1));
            mecvRet.bValue(c4Index < c4Count);
            if (mecvRet.bValue())
            {
                TMEngKVPairVal& mecvFound = static_cast<TMEngKVPairVal&>
                (
                    pmecvVals->mecvAt(meOwner, c4Index)
                );

                // If adding, update this guy, else get its value for return
                if (c2MethId == m_c2MethId_AddReqValue)
                {
                    mecvFound.strValue(meOwner.strStackValAt(c4FirstInd + 1));
                }
                 else
                {
                    TMEngStringVal& mecvToFill
                    (
                        meOwner.mecvStackAtAs<TMEngStringVal>(c4FirstInd + 1)
                    );
                    mecvToFill.strValue(mecvFound.strValue());
                }
            }
             else
            {
                // Not found, so if adding, just add a new one
                pmecvVals->AddObject
                (
                    new TMEngKVPairVal
                    (
                        TString::strEmpty()
                        , m_pmeciKVPair->c2Id()
                        , tCIDMacroEng::EConstTypes::NonConst
                        , strToFind
                        , meOwner.strStackValAt(c4FirstInd + 1)
                    )
                );
            }

            // Flip the return value if adding, since the meaning is the opposite
            if (c2MethId == m_c2MethId_AddReqValue)
                mecvRet.Negate();
        }

        catch(const TError& errToCatch)
        {
            ThrowAnErr(meOwner, m_c4ErrReqVal, errToCatch.strErrText());
        }
    }
     else if (c2MethId == m_c2MethId_QueryBodyContent)
    {
        // We return the bytes available, so get that class member
        TMEngCard4Val* pmecvBufLen = static_cast<TMEngCard4Val*>
        (
            mecvInstance.pmecvFind(TCMLBinBaseInfo::strContBufLenName)
        );

        // And get a convenience copy out
        const tCIDLib::TCard4 c4BufLen = pmecvBufLen->c4Value();

        // Set up the return value
        TMEngCard4Val& mecvRet(meOwner.mecvStackAtAs<TMEngCard4Val>(c4FirstInd - 1));
        mecvRet.c4Value(c4BufLen);

        // If there's any content, copy over the other info
        if (c4BufLen)
        {
            TMEngMemBufVal* pmecvContBuf = static_cast<TMEngMemBufVal*>
            (
                mecvInstance.pmecvFind(TCMLBinBaseInfo::strContBufName)
            );

            TMEngStringVal* pmecvContType = static_cast<TMEngStringVal*>
            (
                mecvInstance.pmecvFind(TCMLBinBaseInfo::strContTypeName)
            );

            TMEngStringVal* pmecvContEncoding = static_cast<TMEngStringVal*>
            (
                mecvInstance.pmecvFind(TCMLBinBaseInfo::strContEncodingName)
            );

            TMEngStringVal* pmecvContBoundary = static_cast<TMEngStringVal*>
            (
                mecvInstance.pmecvFind(TCMLBinBaseInfo::strContBoundaryName)
            );

            // Copy over the content type
            TMEngStringVal& mecvType(meOwner.mecvStackAtAs<TMEngStringVal>(c4FirstInd + 1));
            mecvType.strValue(pmecvContType->strValue());

            // Copy over the encoding
            TMEngStringVal& mecvEncoding(meOwner.mecvStackAtAs<TMEngStringVal>(c4FirstInd + 2));
            mecvEncoding.strValue(pmecvContEncoding->strValue());

            // Copy over the boundary
            TMEngStringVal& mecvBoundary(meOwner.mecvStackAtAs<TMEngStringVal>(c4FirstInd + 3));
            mecvBoundary.strValue(pmecvContBoundary->strValue());

            // And the buffer contents
            TMEngMemBufVal& mecvCont(meOwner.mecvStackAtAs<TMEngMemBufVal>(c4FirstInd));
            mecvCont.mbufValue().CopyIn(pmecvContBuf->mbufValue(), c4BufLen);
        }
    }
     else if (c2MethId == m_c2MethId_SetInBodyCont)
    {
        // Look up our members we need to set
        TMEngCard4Val* pmecvBufLen = static_cast<TMEngCard4Val*>
        (
            mecvInstance.pmecvFind(TCMLBinBaseInfo::strContBufLenName)
        );
        TMEngMemBufVal* pmecvContBuf = static_cast<TMEngMemBufVal*>
        (
            mecvInstance.pmecvFind(TCMLBinBaseInfo::strContBufName)
        );
        TMEngStringVal* pmecvContType = static_cast<TMEngStringVal*>
        (
            mecvInstance.pmecvFind(TCMLBinBaseInfo::strContTypeName)
        );
        TMEngStringVal* pmecvContEncoding = static_cast<TMEngStringVal*>
        (
            mecvInstance.pmecvFind(TCMLBinBaseInfo::strContEncodingName)
        );
        TMEngStringVal* pmecvContBoundary = static_cast<TMEngStringVal*>
        (
            mecvInstance.pmecvFind(TCMLBinBaseInfo::strContBoundaryName)
        );

        // Set the length from the incoming param
        const tCIDLib::TCard4 c4Len = meOwner.c4StackValAt(c4FirstInd + 1);
        pmecvBufLen->c4Value(c4Len);

        // If they passed content set the other stuff, else clear them
        if (pmecvBufLen->c4Value())
        {
            const TMEngMemBufVal& mecvCont(meOwner.mecvStackAtAs<TMEngMemBufVal>(c4FirstInd));
            pmecvContBuf->mbufValue().CopyIn(mecvCont.mbufValue(), c4Len);

            pmecvContType->strValue(meOwner.strStackValAt(c4FirstInd + 2));
            pmecvContEncoding->strValue(meOwner.strStackValAt(c4FirstInd + 3));
            pmecvContBoundary->strValue(meOwner.strStackValAt(c4FirstInd + 4));
        }
         else
        {
            // The buffer we just ignore, but make sure the encoding is cleared
            pmecvContType->strValue(TString::strEmpty());
            pmecvContEncoding->strValue(TString::strEmpty());
            pmecvContBoundary->strValue(TString::strEmpty());
        }
    }
     else
    {
        // We don't know what it is
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


// ---------------------------------------------------------------------------
//  TCMLBinBaseInfo: Protected, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TCMLBinBaseInfo::ThrowAnErr(        TCIDMacroEngine&    meOwner
                            , const tCIDLib::TCard4     c4ToThrow
                            , const TString&            strText) const
{
    // Set the exception info on the engine
    meOwner.SetException
    (
        m_c2EnumId_Errors
        , strClassPath()
        , c4ToThrow
        , m_pmeciErrors->strPartialName(c4ToThrow)
        , strText
        , meOwner.c4CurLine()
    );

    // And throw the excpetion that represents a macro level exception
    throw TExceptException();
}


