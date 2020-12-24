//
// FILE NAME: CQCMEng_DevSenseClass.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/08/2014
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
//  This file implements the info and value C++ classes to create a macro level
//  class which wraps the TCQCDevSense class.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCMEng_.hpp"
#include    "CIDMacroEng_CommClasses_.hpp"
#include    "CIDMacroEng_SockClasses_.hpp"
#include    "CQCMEng_DevSenseClass_.hpp"


// ---------------------------------------------------------------------------
//  Magic RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCDevSenseInfo,TMEngClassInfo)


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace CQCMacroEng_DevSenseClass
{
    namespace
    {
        // -----------------------------------------------------------------------
        //  The names for the types that we support here. Each derivative has to
        //  be able to return strings that contain its name and full name.
        // -----------------------------------------------------------------------
        const TString   strClass(L"CQCDevSense");
        const TString   strClassPath(L"MEng.System.CQC.Runtime.CQCDevSense");
        const TString   strBasePath(L"MEng.System.CQC.Runtime");
    }
}



// ---------------------------------------------------------------------------
//  CLASS: TCQCDevSenseInfo
// PREFIX: meci
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCDevSenseInfo: Public, static methods
// ---------------------------------------------------------------------------
const TString& TCQCDevSenseInfo::strClassPath()
{
    return CQCMacroEng_DevSenseClass::strClassPath;
}


// ---------------------------------------------------------------------------
//  TCQCDevSenseInfo: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCDevSenseInfo::TCQCDevSenseInfo(TCIDMacroEngine& meOwner) :

    TMEngClassInfo
    (
        CQCMacroEng_DevSenseClass::strClass
        , CQCMacroEng_DevSenseClass::strBasePath
        , meOwner
        , kCIDLib::True
        , tCIDMacroEng::EClassExt::Final
        , L"MEng.Object"
    )
    , m_c2MethId_DefCtor(kCIDMacroEng::c2BadId)
    , m_c2MethId_Initialize(kCIDMacroEng::c2BadId)
    , m_c2MethId_Terminate(kCIDMacroEng::c2BadId)
    , m_c2MethId_TestOther(kCIDMacroEng::c2BadId)
    , m_c2MethId_TestSerial(kCIDMacroEng::c2BadId)
    , m_c2MethId_TestSocket(kCIDMacroEng::c2BadId)
    , m_c2TypeId_SockProtos(0)
    , m_pmeciCommCfg(nullptr)
    , m_pmeciCommPort(nullptr)
    , m_pmeciEndPoint(nullptr)
    , m_pmeciStreamSocket(nullptr)
{
}

TCQCDevSenseInfo::~TCQCDevSenseInfo()
{
}


tCIDLib::TVoid TCQCDevSenseInfo::Init(TCIDMacroEngine& meOwner)
{
    //
    //  Look up the socket, serial port, port config, socket config, and a few
    //  other things we need to pass in and out.
    //
    m_pmeciCommCfg = static_cast<TMEngCommCfgInfo*>
    (
        meOwner.pmeciFind(TMEngCommCfgInfo::strPath())
    );

    m_pmeciCommPort = static_cast<TMEngCommPortInfo*>
    (
        meOwner.pmeciFind(TMEngCommPortInfo::strPath())
    );

    m_pmeciEndPoint = static_cast<TMEngIPEPInfo*>
    (
        meOwner.pmeciFind(TMEngIPEPInfo::strPath())
    );

    m_pmeciStreamSocket = static_cast<TMEngStreamSocketInfo*>
    (
        meOwner.pmeciFind(TMEngStreamSocketInfo::strPath())
    );

    m_c2TypeId_SockProtos = meOwner.pmeciFind(TMEngSocketInfo::strSockProtosPath())->c2Id();


    // Add the default constructor
    {
        TMEngMethodInfo methiNew
        (
            L"ctor1_MEng.System.CQC.Runtime.CQCDevSense"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.bIsCtor(kCIDLib::True);
        m_c2MethId_DefCtor = c2AddMethodInfo(methiNew);
    }

    // Initialize
    {
        TMEngMethodInfo methiNew
        (
            L"Initialize"
            , tCIDMacroEng::EIntrinsics::Boolean
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        m_c2MethId_Initialize = c2AddMethodInfo(methiNew);
    }

    // Clean up
    {
        TMEngMethodInfo methiNew
        (
            L"Terminate"
            , tCIDMacroEng::EIntrinsics::Void
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        m_c2MethId_Terminate = c2AddMethodInfo(methiNew);
    }

    // Test the passed other config
    {
        TMEngMethodInfo methiNew
        (
            L"TestOther"
            , tCIDMacroEng::EIntrinsics::Boolean
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddOutParm(L"CfgString", tCIDMacroEng::EIntrinsics::String);
        m_c2MethId_TestOther = c2AddMethodInfo(methiNew);
    }

    // Test the passed serial port
    {
        TMEngMethodInfo methiNew
        (
            L"TestSerialPort"
            , tCIDMacroEng::EIntrinsics::Boolean
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"Port", m_pmeciCommPort->c2Id());
        methiNew.c2AddOutParm(L"PortCfg", m_pmeciCommCfg->c2Id());
        m_c2MethId_TestSerial = c2AddMethodInfo(methiNew);
    }

    // Test the passed socket
    {
        TMEngMethodInfo methiNew
        (
            L"TestSocket"
            , tCIDMacroEng::EIntrinsics::Boolean
            , tCIDMacroEng::EVisTypes::Public
            , tCIDMacroEng::EMethExt::Final
        );
        methiNew.c2AddInParm(L"Socket", m_pmeciStreamSocket->c2Id());
        methiNew.c2AddOutParm(L"IPEndPoint", m_pmeciEndPoint->c2Id());
        methiNew.c2AddOutParm(L"Proto", m_c2TypeId_SockProtos);
        m_c2MethId_TestSocket = c2AddMethodInfo(methiNew);
    }
}


TMEngClassVal*
TCQCDevSenseInfo::pmecvMakeStorage( const   TString&                strName
                                    ,       TCIDMacroEngine&
                                    , const tCIDMacroEng::EConstTypes  eConst) const
{
    return new TMEngStdClassVal(strName, c2Id(), eConst);
}



// ---------------------------------------------------------------------------
//  TCQCDevSenseInfo: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TCQCDevSenseInfo::bInvokeMethod(        TCIDMacroEngine&    meOwner
                                , const TMEngMethodInfo&    methiTarget
                                ,       TMEngClassVal&      mecvInstance)
{
    //
    //  All of our methods are called directly by the auto-sensing utility, we
    //  don't have any that CML code calls.
    //
    return kCIDLib::False;
}

