//
// FILE NAME: CQCDataSrv_ThisFacility.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/23/2002
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
//  This file implements the callbacks from the server core framework we derive
//  front, to handle init and cleanup. It provides a few small helper methods that
//  some of the server side interfaces would otherwise redundantly implement.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCDataSrv.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TFacCQCDataSrv, TCQCSrvCore)



// ---------------------------------------------------------------------------
//   CLASS: TFacDataSrv
//  PREFIX: fac
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TFacDataSrv: Constructors and Destructor
// ---------------------------------------------------------------------------

//
//  We don't ask the server core for a login. We are the thing folks log into,
//  so we can't log in ourselves.
//
TFacCQCDataSrv::TFacCQCDataSrv() :

    TCQCSrvCore
    (
        L"CQCDataSrv"
        , L"CQC Data Server"
        , kCQCKit::ippnDataSrvDefPort
        , kCQCDataSrv::pszEvDataSrv
        , tCIDLib::EModFlags::HasMsgFile
        , tCQCSrvFW::ESrvOpts::None
    )
    , m_porbsAccImpl(nullptr)
    , m_porbsInstImpl(nullptr)
    , m_porbsIRImpl(nullptr)
    , m_porbsSecurityImpl(nullptr)
    , m_porbsSysCfg(nullptr)
{
}


// ---------------------------------------------------------------------------
//  TFacCQCDataSrv: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  These are short cuts for our own code here within the data server to check
//  security tokens without the overhead of going through the ORB. We have a
//  pointer to the security interface, which we created. So we can just call it
//  directly.
//
//  We do have to be careful in that the ORB won't know that we have called into
//  the object. So we have to bump the entered count before we call it. This
//  prevents the ORB from potentially shutting down the object while we are in
//  it. It's pretty unlikely since we are only accessing in-memory data during
//  this, but just in case. There's a janitor class to do this. The object handles
//  it's own internal synchronization.
//
//  For the first one, all the caller cares about is whether it's a valid token.
//  The second takes a minimum user role that the passed token must represent.
//
//  The third one is a special case where it must either be a token for the
//  indicated account, or it must be at least the indicated minimum role. This
//  for things where either the user himself or and admin type can change
//  something related to the user's account.
//
tCIDLib::TBoolean
TFacCQCDataSrv::bCheckSTok(const TCQCSecToken& sectUser)
{
    TOrbSrvEnteredJan janEntered(m_porbsSecurityImpl);
    return m_porbsSecurityImpl->bCheckToken(sectUser);
}

tCIDLib::TBoolean
TFacCQCDataSrv::bCheckMinRole(  const   TCQCSecToken&       sectUser
                                , const tCQCKit::EUserRoles eMinRole)
{
    TOrbSrvEnteredJan janEntered(m_porbsSecurityImpl);

    tCQCKit::EUserRoles     eRole;
    tCQCKit::EUserTypes     eType;
    tCIDLib::TEncodedTime   enctStamp;
    TString                 strDefTmpl;
    TString                 strLoginName;

    m_porbsSecurityImpl->DecodeSecurityToken
    (
        sectUser, strLoginName, eRole, eType, enctStamp, strDefTmpl
    );
    return (eRole >= eMinRole);
}

tCIDLib::TBoolean
TFacCQCDataSrv::bCheckAccess(const  TCQCSecToken&       sectUser
                            , const TString&            strAccount
                            , const tCQCKit::EUserRoles eMinRole)
{
    TOrbSrvEnteredJan janEntered(m_porbsSecurityImpl);

    tCQCKit::EUserRoles     eRole;
    tCQCKit::EUserTypes     eType;
    tCIDLib::TEncodedTime   enctStamp;
    TString                 strDefTmpl;
    TString                 strLoginName;

    m_porbsSecurityImpl->DecodeSecurityToken
    (
        sectUser, strLoginName, eRole, eType, enctStamp, strDefTmpl
    );

    return (strAccount == strLoginName) || (eRole >= eMinRole);
}



// ---------------------------------------------------------------------------
//  TFacCQCDataSrv: Protected, inherited methods
// ---------------------------------------------------------------------------

// The ORB owns these objects and destroys them when we deregister them
tCIDLib::TVoid TFacCQCDataSrv::DeregisterSrvObjs()
{
    if (m_porbsAccImpl)
    {
        facCIDOrb().DeregisterObject(m_porbsAccImpl);
        m_porbsAccImpl = nullptr;
    }

    if (m_porbsInstImpl)
    {
        facCIDOrb().DeregisterObject(m_porbsInstImpl);
        m_porbsInstImpl = nullptr;
    }

    if (m_porbsIRImpl)
    {
        facCIDOrb().DeregisterObject(m_porbsIRImpl);
        m_porbsIRImpl = nullptr;
    }

    if (m_porbsSecurityImpl)
    {
        facCIDOrb().DeregisterObject(m_porbsSecurityImpl);
        m_porbsSecurityImpl = nullptr;
    }

    if (m_porbsSysCfg)
    {
        facCIDOrb().DeregisterObject(m_porbsSysCfg);
        m_porbsSysCfg = nullptr;
    }
}


//
//  In our case, we load our config via a server side object, so we have to create
//  it here, instead of in RegisterSrvObjs() since this would get called before that
//  and the object wouldn't exist yet.
//
tCQCSrvFW::EStateRes
TFacCQCDataSrv::eLoadConfig(tCIDLib::TCard4& c4WaitNext, const tCIDLib::TCard4 c4Count)
{
    try
    {
        // If not created yet, then do so and register it
        if (!m_porbsSecurityImpl)
        {
            m_porbsSecurityImpl = new TCQCSecuritySrvImpl();
            facCIDOrb().RegisterObject(m_porbsSecurityImpl, tCIDLib::EAdoptOpts::Adopt);
        }
    }

    catch(TError& errToCatch)
    {
        // We'll retry it up to 20 times, then throw to give up
		if (c4Count < 20)
		{
			c4WaitNext = 3000;
			return tCQCSrvFW::EStateRes::Retry;
		}

        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }

    // Ask the security object to load his info
    return m_porbsSecurityImpl->eLoadSecurityInfo(c4WaitNext);
}


//
//  Note that some of them will be redirected if we are in debug mode, to point
//  them at the development versions of those directories.
//
tCIDLib::TVoid TFacCQCDataSrv::MakeDirs()
{
    // These are the paths we create under the DataServer directory
    const tCIDLib::TCh* apszDataDirs[] =
    {
        L"Events"
        , L"Events\\Monitors"
        , L"Events\\Monitors\\System"
        , L"Events\\Monitors\\User"
        , L"Events\\Scheduled"
        , L"Events\\Scheduled\\System"
        , L"Events\\Scheduled\\User"
        , L"Events\\Triggered"
        , L"Events\\Triggered\\System"
        , L"Events\\Triggered\\User"
        , L"IRModels"
        , L"Macros"
        , L"Macros\\User\\CQC\\Drivers"
        , L"Macros\\System\\CQC\\Drivers"
        , L"GlobalActs"
        , L"GlobalActs\\System"
        , L"GlobalActs\\User"
        , L"Images"
        , L"Images\\User"
        , L"Images\\System"
        , L"Interfaces"
        , L"Interfaces\\User"
        , L"Interfaces\\System"
        , L"Manifests"
        , L"Manifests\\System"
        , L"Manifests\\User"
        , L"Protocols"
        , L"Protocols\\System"
        , L"Protocols\\User"

        // A terminating null for the loop below
        , nullptr
    };

    // Set up the configuration interface directory
    TPathStr pathDSPath;
    pathDSPath = facCQCKit().strDataDir();
    pathDSPath.AddLevel(L"DataServer");
    if (!TFileSys::bIsDirectory(pathDSPath))
        TFileSys::MakePath(pathDSPath);

    const tCIDLib::TCard4 c4OrgLen = pathDSPath.c4Length();
    tCIDLib::TCard4 c4Index = 0;
    while (apszDataDirs[c4Index])
    {
        pathDSPath.CapAt(c4OrgLen);
        pathDSPath.AddLevel(apszDataDirs[c4Index]);
        if (!TFileSys::bIsDirectory(pathDSPath))
            TFileSys::MakePath(pathDSPath);
        c4Index++;
    }
}


// We don't use any of the extra values
tCIDLib::TVoid
TFacCQCDataSrv::QueryCoreAdminInfo( TString&    strCoreAdminBinding
                                    , TString&  strCoreAdminDesc
                                    , TString&
                                    , TString&
                                    , TString&
                                    , TString&  )
{
    strCoreAdminBinding = kCQCKit::pszCQCDataSrvAdmin;
    strCoreAdminDesc = L"CQC Data Server Core Admin Object";
}


//
//  During startup, the parent class lets us create our server side objects. He
//  handles the core admin interface for us.
//
tCIDLib::TVoid TFacCQCDataSrv::RegisterSrvObjs()
{
    //
    //  The data server handler has a cache size setting. We default it to 2048. But,
    //  just in case of large resources or limited memory, we allow them to set it
    //  from 512 to 4096 via an environment variable, so we check for that.
    //
    tCIDLib::TCard4 c4DSCacheSz = 2048;
    TString strDSCacheSz;
    if (TProcEnvironment::bFind(L"CQC_DSCACHESIZE", strDSCacheSz))
    {
        tCIDLib::TCard4 c4NewSz;
        if (strDSCacheSz.bToCard4(c4NewSz, tCIDLib::ERadices::Dec))
        {
            if ((c4NewSz >= 512) && (c4NewSz <= 4096))
                c4DSCacheSz = c4NewSz;
        }
    }
    m_porbsAccImpl = new TDataSrvAccImpl(c4DSCacheSz);
    facCIDOrb().RegisterObject(m_porbsAccImpl, tCIDLib::EAdoptOpts::Adopt);

    m_porbsIRImpl = new TCQCIRSrvImpl();
    facCIDOrb().RegisterObject(m_porbsIRImpl, tCIDLib::EAdoptOpts::Adopt);

    m_porbsSysCfg = new TCQCSysCfgSrvImpl();
    facCIDOrb().RegisterObject(m_porbsSysCfg, tCIDLib::EAdoptOpts::Adopt);

    m_porbsInstImpl = new TCQCInstSrvImpl();
    facCIDOrb().RegisterObject(m_porbsInstImpl, tCIDLib::EAdoptOpts::Adopt);


    //
    //  Register our objects with the rebinder. He'll bind us as soon as
    //  he's started (back in the main startup code) and then periodically
    //  check and rebind us if the name server has cycled.
    //
    //  Sp they won't be initailly registered until we start the rebinder, so
    //  no clients can get to us until then.
    //
    facCIDOrbUC().RegRebindObj
    (
        m_porbsAccImpl->ooidThis()
        , TDataSrvAccServerBase::strBinding
        , L"CQC File Access Object"
    );
    facCIDOrbUC().RegRebindObj
    (
        m_porbsIRImpl->ooidThis()
        , TIRRepoServerBase::strBinding
        , L"CQC IR Device Model Repository Server Object"
    );
    facCIDOrbUC().RegRebindObj
    (
        m_porbsSysCfg->ooidThis()
        , TCQCSysCfgSrvImpl::strBinding
        , L"CQC System Config Server Object"
    );
    facCIDOrbUC().RegRebindObj
    (
        m_porbsInstImpl->ooidThis()
        , TCQCInstServerBase::strBinding
        , L"CQC Installation Server Object"
    );

    //
    //  The security one we created in the load config callback since we needed
    //  it to load the security config, but we wait until here to register with
    //  the rebinder in case we failed before getting this far.
    //
    CIDAssert
    (
        m_porbsSecurityImpl != nullptr
        , L"The server side security interface did not get created"
    );
    facCIDOrbUC().RegRebindObj
    (
        m_porbsSecurityImpl->ooidThis()
        , TCQCSecureServerBase::strBinding
        , L"CQC Security Server Object"
    );
}


//
//  Called on shutdown to remove all our name server bindings. This must be called
//  after the rebinder is stopped, else he'd just rebind us again!
//
tCIDLib::TVoid TFacCQCDataSrv::UnbindSrvObjs(tCIDOrbUC::TNSrvProxy&  orbcNS)
{
    orbcNS->RemoveBinding(TDataSrvAccServerBase::strBinding, kCIDLib::False);
    orbcNS->RemoveBinding(TCQCInstServerBase::strBinding, kCIDLib::False);
    orbcNS->RemoveBinding(TIRRepoServerBase::strBinding, kCIDLib::False);
    orbcNS->RemoveBinding(TCQCSecureServerBase::strBinding, kCIDLib::False);
    orbcNS->RemoveBinding(TCQCSysCfgSrvImpl::strBinding, kCIDLib::False);
}
