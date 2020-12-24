//
// FILE NAME: CQCMEng_CRCHelpersClass.cpp
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
//  This file implements the info and value classes to create a macro level
//  class that provides helper methods for doing CRC calculations.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCMEng_.hpp"
#include    "CQCMEng_CRCHelpersClass_.hpp"

// ---------------------------------------------------------------------------
//  We have to bring in the otherwise private header for the crypto key
//  CML class from CIDMacroEng.
// ---------------------------------------------------------------------------
#include    "CIDMacroEng_CryptoKey_.hpp"


// ---------------------------------------------------------------------------
//  Magic RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TCRCHelpersValue,TMEngClassVal)
RTTIDecls(TCRCHelpersInfo,TMEngClassInfo)


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace CQCMacroEng_CRCHelpers
{
    namespace
    {
        // -----------------------------------------------------------------------
        //  The names for the types that we support here. Each derivative has to
        //  be able to return strings that contain its name and full name.
        // -----------------------------------------------------------------------
        const TString   strCRCHelpers(L"CRCHelpers");
        const TString   strCRCHelpersClassPath(L"MEng.System.CQC.Runtime.CRCHelpers");
        const TString   strCRCHelpersBasePath(L"MEng.System.CQC.Runtime");
    }
}




// ---------------------------------------------------------------------------
//  CLASS: TCRCHelpersValue
// PREFIX: mecv
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCRCHelpersValue: Constuctors and Destructor
// ---------------------------------------------------------------------------
TCRCHelpersValue::TCRCHelpersValue( const   TString&                strName
                                    , const tCIDLib::TCard2         c2ClassId
                                    , const tCIDMacroEng::EConstTypes  eConst) :

    TMEngClassVal(strName, c2ClassId, eConst)
{
}

TCRCHelpersValue::~TCRCHelpersValue()
{
}




// ---------------------------------------------------------------------------
//  CLASS: TCRCHelpersInfo
// PREFIX: meci
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCRCHelpersInfo: Public, static methods
// ---------------------------------------------------------------------------
const TString& TCRCHelpersInfo::strClassPath()
{
    return CQCMacroEng_CRCHelpers::strCRCHelpersClassPath;
}


// ---------------------------------------------------------------------------
//  TCRCHelpersInfo: Constructors and Destructor
// ---------------------------------------------------------------------------
TCRCHelpersInfo::TCRCHelpersInfo(TCIDMacroEngine& meOwner) :

    TMEngClassInfo
    (
        CQCMacroEng_CRCHelpers::strCRCHelpers
        , CQCMacroEng_CRCHelpers::strCRCHelpersBasePath
        , meOwner
        , kCIDLib::False
        , tCIDMacroEng::EClassExt::Final
        , L"MEng.Object"
    )
    , m_c2MethId_DefCtor(kCIDMacroEng::c2BadId)
    , m_c2MethId_CheckSum1(kCIDMacroEng::c2BadId)
    , m_c2MethId_CheckSum2(kCIDMacroEng::c2BadId)
    , m_c2MethId_CheckSum4(kCIDMacroEng::c2BadId)
    , m_c2MethId_CRC1Full(kCIDMacroEng::c2BadId)
    , m_c2MethId_CRC1InnerLoop(kCIDMacroEng::c2BadId)
    , m_c2MethId_CRCOcelot(kCIDMacroEng::c2BadId)
    , m_c2MethId_CRC1PerByte(kCIDMacroEng::c2BadId)
    , m_pmeciMemBuf(nullptr)
{
    // Import the crypto key class
    bAddClassImport(TMEngCryptoKeyInfo::strPath());
}

TCRCHelpersInfo::~TCRCHelpersInfo()
{
}


tCIDLib::TVoid TCRCHelpersInfo::Init(TCIDMacroEngine& meOwner)
{
    // Look up the memory buffer class info object for later use
    m_pmeciMemBuf = static_cast<TMEngMemBufInfo*>
    (
        meOwner.pmeciFind(TMEngMemBufInfo::strPath())
    );

    // Add the default constructor
    {
        TMEngMethodInfo methiNew
        (
            L"ctor1_MEng.System.CQC.Runtime.CRCHelpers"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.bIsCtor(kCIDLib::True);
        m_c2MethId_DefCtor = c2AddMethodInfo(methiNew);
    }

    // Simple Card1, Card2, Card4 checksums of a range of bytes
    {
        TMEngMethodInfo methiNew
        (
            L"CheckSum1"
            , tCIDMacroEng::EIntrinsics::Card1
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInParm(L"InBuf", tCIDMacroEng::EIntrinsics::MemBuf);
        methiNew.c2AddInParm(L"StartInd", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddInParm(L"EndInd", tCIDMacroEng::EIntrinsics::Card4);
        m_c2MethId_CheckSum1 = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"CheckSum2"
            , tCIDMacroEng::EIntrinsics::Card2
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInParm(L"InBuf", tCIDMacroEng::EIntrinsics::MemBuf);
        methiNew.c2AddInParm(L"StartInd", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddInParm(L"EndInd", tCIDMacroEng::EIntrinsics::Card4);
        m_c2MethId_CheckSum2 = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"CheckSum4"
            , tCIDMacroEng::EIntrinsics::Card4
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInParm(L"InBuf", tCIDMacroEng::EIntrinsics::MemBuf);
        methiNew.c2AddInParm(L"StartInd", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddInParm(L"EndInd", tCIDMacroEng::EIntrinsics::Card4);
        m_c2MethId_CheckSum4 = c2AddMethodInfo(methiNew);
    }

    // The per-byte, inner loop, and full versions of CRC variant 1
    {
        TMEngMethodInfo methiNew
        (
            L"CRC1Full"
            , tCIDMacroEng::EIntrinsics::Card2
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInParm(L"InBuf", tCIDMacroEng::EIntrinsics::MemBuf);
        methiNew.c2AddInParm(L"StartInd", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddInParm(L"EndInd", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddInParm(L"Polynomial", tCIDMacroEng::EIntrinsics::Card2);
        methiNew.c2AddInParm(L"LoopCount", tCIDMacroEng::EIntrinsics::Card4);
        m_c2MethId_CRC1Full = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"CRC1InnerLoop"
            , tCIDMacroEng::EIntrinsics::Card2
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInParm(L"InCRC", tCIDMacroEng::EIntrinsics::Card2);
        methiNew.c2AddInParm(L"Polynomial", tCIDMacroEng::EIntrinsics::Card2);
        methiNew.c2AddInParm(L"Count", tCIDMacroEng::EIntrinsics::Card4);
        m_c2MethId_CRC1InnerLoop = c2AddMethodInfo(methiNew);
    }

    {
        TMEngMethodInfo methiNew
        (
            L"CRC1PerByte"
            , tCIDMacroEng::EIntrinsics::Card2
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInParm(L"InCRC", tCIDMacroEng::EIntrinsics::Card2);
        methiNew.c2AddInParm(L"Polynomial", tCIDMacroEng::EIntrinsics::Card2);
        m_c2MethId_CRC1PerByte = c2AddMethodInfo(methiNew);
    }

    // The CRC used by the Ocelot
    {
        TMEngMethodInfo methiNew
        (
            L"OcelotCRC"
            , tCIDMacroEng::EIntrinsics::Card2
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
            , tCIDMacroEng::EConstTypes::Const
        );
        methiNew.c2AddInParm(L"SrcBuf", tCIDMacroEng::EIntrinsics::MemBuf);
        methiNew.c2AddInParm(L"StartInd", tCIDMacroEng::EIntrinsics::Card4);
        methiNew.c2AddInParm(L"Count", tCIDMacroEng::EIntrinsics::Card4);
        m_c2MethId_CRCOcelot = c2AddMethodInfo(methiNew);
    }
}


TMEngClassVal*
TCRCHelpersInfo::pmecvMakeStorage(  const   TString&                strName
                                    ,       TCIDMacroEngine&
                                    , const tCIDMacroEng::EConstTypes  eConst) const
{
    return new TCRCHelpersValue(strName, c2Id(), eConst);
}


// ---------------------------------------------------------------------------
//  TCRCHelpersInfo: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TCRCHelpersInfo::bInvokeMethod(         TCIDMacroEngine&    meOwner
                                , const TMEngMethodInfo&    methiTarget
                                ,       TMEngClassVal&)
{
    // Don't need this for now
//    TCRCHelpersValue& mecvActual = static_cast<TCRCHelpersValue&>(mecvInstance);

    const tCIDLib::TCard4 c4FirstInd = meOwner.c4FirstParmInd(methiTarget);
    const tCIDLib::TCard2 c2MethId   = methiTarget.c2Id();

    if (c2MethId == m_c2MethId_DefCtor)
    {
        // Nothing to do
    }
     else if (c2MethId == m_c2MethId_CheckSum1)
    {
        TMEngMemBufVal& mecvSrc = meOwner.mecvStackAtAs<TMEngMemBufVal>(c4FirstInd);
        const TMemBuf& mbufSrc = mecvSrc.mbufValue();
        const tCIDLib::TCard4 c4StartInd = meOwner.c4StackValAt(c4FirstInd + 1);
        const tCIDLib::TCard4 c4EndInd = meOwner.c4StackValAt(c4FirstInd + 2);

        // Do an index check
        m_pmeciMemBuf->IndexCheck
        (
            meOwner, mecvSrc, (c4EndInd - c4StartInd) + 1, c4StartInd, kCIDLib::False
        );

        tCIDLib::TCard1 c1Sum = 0;
        for (tCIDLib::TCard4 c4Index = c4StartInd; c4Index <= c4EndInd; c4Index++)
            c1Sum += tCIDLib::TCard1(mbufSrc[c4Index]);

        // And give back the return value
        TMEngCard1Val& mecvRet
                    = meOwner.mecvStackAtAs<TMEngCard1Val>(c4FirstInd - 1);
        mecvRet.c1Value(c1Sum);
    }
     else if (c2MethId == m_c2MethId_CheckSum2)
    {
        TMEngMemBufVal& mecvSrc = meOwner.mecvStackAtAs<TMEngMemBufVal>(c4FirstInd);
        const TMemBuf& mbufSrc = mecvSrc.mbufValue();
        const tCIDLib::TCard4 c4StartInd = meOwner.c4StackValAt(c4FirstInd + 1);
        const tCIDLib::TCard4 c4EndInd = meOwner.c4StackValAt(c4FirstInd + 2);

        // Do an index check
        m_pmeciMemBuf->IndexCheck
        (
            meOwner, mecvSrc, (c4EndInd - c4StartInd) + 1, c4StartInd, kCIDLib::False
        );

        tCIDLib::TCard2 c2Sum = 0;
        for (tCIDLib::TCard4 c4Index = c4StartInd; c4Index <= c4EndInd; c4Index++)
            c2Sum += tCIDLib::TCard1(mbufSrc[c4Index]);

        // And give back the return value
        TMEngCard2Val& mecvRet
                    = meOwner.mecvStackAtAs<TMEngCard2Val>(c4FirstInd - 1);
        mecvRet.c2Value(c2Sum);
    }
     else if (c2MethId == m_c2MethId_CheckSum4)
    {
        TMEngMemBufVal& mecvSrc = meOwner.mecvStackAtAs<TMEngMemBufVal>(c4FirstInd);
        const TMemBuf& mbufSrc = mecvSrc.mbufValue();
        const tCIDLib::TCard4 c4StartInd = meOwner.c4StackValAt(c4FirstInd + 1);
        const tCIDLib::TCard4 c4EndInd = meOwner.c4StackValAt(c4FirstInd + 2);

        // Do an index check
        m_pmeciMemBuf->IndexCheck
        (
            meOwner, mecvSrc, (c4EndInd - c4StartInd) + 1, c4StartInd, kCIDLib::False
        );

        tCIDLib::TCard4 c4Sum = 0;
        for (tCIDLib::TCard4 c4Index = c4StartInd; c4Index <= c4EndInd; c4Index++)
            c4Sum += mbufSrc[c4Index];

        // And give back the return value
        TMEngCard4Val& mecvRet
                    = meOwner.mecvStackAtAs<TMEngCard4Val>(c4FirstInd - 1);
        mecvRet.c4Value(c4Sum);
    }
     else if (c2MethId == m_c2MethId_CRC1Full)
    {
        // Get the buffer, start/end indexes, and polynomial
        TMEngMemBufVal& mecvSrc = meOwner.mecvStackAtAs<TMEngMemBufVal>(c4FirstInd);
        const TMemBuf& mbufSrc = mecvSrc.mbufValue();
        const tCIDLib::TCard4 c4StartInd = meOwner.c4StackValAt(c4FirstInd + 1);
        const tCIDLib::TCard4 c4EndInd = meOwner.c4StackValAt(c4FirstInd + 2);
        const tCIDLib::TCard2 c2Poly = meOwner.c2StackValAt(c4FirstInd + 3);
        const tCIDLib::TCard4 c4LoopCount = meOwner.c4StackValAt(c4FirstInd + 4);

        // Do an index check
        m_pmeciMemBuf->IndexCheck
        (
            meOwner, mecvSrc, (c4EndInd - c4StartInd) + 1, c4StartInd, kCIDLib::False
        );

        tCIDLib::TCard2 c2CRCVal = 0;
        for (tCIDLib::TCard4 c4Index = c4StartInd; c4Index <= c4EndInd; c4Index++)
        {
            c2CRCVal ^= tCIDLib::TCard2(mbufSrc[c4Index]);
            for (tCIDLib::TCard4 c4InInd = 0; c4InInd < c4LoopCount; c4InInd++)
            {
                const tCIDLib::TBoolean bFlag((c2CRCVal & 0x1) != 0);
                c2CRCVal >>= 1;
                if (bFlag)
                    c2CRCVal ^= c2Poly;
            }
        }

        // And give back the return value
        TMEngCard2Val& mecvRet
                    = meOwner.mecvStackAtAs<TMEngCard2Val>(c4FirstInd - 1);
        mecvRet.c2Value(c2CRCVal);
    }
     else if (c2MethId == m_c2MethId_CRC1InnerLoop)
    {
        // Get the incoming value and polynomial
        tCIDLib::TCard2 c2CRCVal = meOwner.c2StackValAt(c4FirstInd);
        const tCIDLib::TCard2 c2Poly = meOwner.c2StackValAt(c4FirstInd + 1);
        const tCIDLib::TCard4 c4LoopCount = meOwner.c2StackValAt(c4FirstInd + 2);

        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4LoopCount; c4Index++)
        {
            const tCIDLib::TBoolean bFlag((c2CRCVal & 0x1) != 0);
            c2CRCVal >>= 1;
            if (bFlag)
                c2CRCVal ^= c2Poly;
        }

        // And give back the return value
        TMEngCard2Val& mecvRet
                    = meOwner.mecvStackAtAs<TMEngCard2Val>(c4FirstInd - 1);
        mecvRet.c2Value(c2CRCVal);
    }
     else if (c2MethId == m_c2MethId_CRC1PerByte)
    {
        // Get the incoming value and polynomial
        tCIDLib::TCard2 c2CRCVal = meOwner.c2StackValAt(c4FirstInd);
        const tCIDLib::TCard2 c2Poly = meOwner.c2StackValAt(c4FirstInd + 1);

        const tCIDLib::TBoolean bFlag((c2CRCVal & 0x1) != 0);
        c2CRCVal >>= 1;
        if (bFlag)
            c2CRCVal ^= c2Poly;

        // And give back the return value
        TMEngCard2Val& mecvRet
                    = meOwner.mecvStackAtAs<TMEngCard2Val>(c4FirstInd - 1);
        mecvRet.c2Value(c2CRCVal);
    }
     else if (c2MethId == m_c2MethId_CRCOcelot)
    {
        TMEngMemBufVal& mecvSrc = meOwner.mecvStackAtAs<TMEngMemBufVal>(c4FirstInd);
        const TMemBuf& mbufSrc = mecvSrc.mbufValue();
        tCIDLib::TCard4 c4Index = meOwner.c4StackValAt(c4FirstInd + 1);
        const tCIDLib::TCard4 c4Count = meOwner.c4StackValAt(c4FirstInd + 2);

        // Check the index and count
        m_pmeciMemBuf->IndexCheck(meOwner, mecvSrc, c4Count, c4Index, kCIDLib::False);

        tCIDLib::TCard2 c2A, c2B;
        tCIDLib::TCard2 c2CRCVal = 0xFFFF;
        tCIDLib::TCard1 c1Local;

        tCIDLib::TCard4 c4CInd = 0;
        while (c4CInd < c4Count)
        {
            c1Local = mbufSrc[c4Index++];

            c2A = c2CRCVal ^ (c1Local << 8);
            c2B = c2A;
            c2A = (((c2A & 0xF000) >> 4) ^ c2B);
            c2B = c2A;
            c2B = ((c2B >> 8) & 0xFF) + ((c2B & 0xFF) << 8);
            c2A = c2A & 0xFF00;
            c2A = (c2A >> 3) + ((c2A & 7) << 13);
            c2B = c2B ^ c2A;
            c2A = c2A >> 1;
            c2A = ((c2A >> 8) & 0xFF) + ((c2A & 0xFF) << 8);
            c2A = (c2A & 0xF000) ^ c2B;
            c2CRCVal = c2A;
            c4CInd++;
        }

        TMEngCard2Val& mecvRet
                    = meOwner.mecvStackAtAs<TMEngCard2Val>(c4FirstInd - 1);
        mecvRet.c2Value(c2CRCVal);
    }
     else
    {
        // We don't know what it is
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


