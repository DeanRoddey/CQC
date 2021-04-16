//
// FILE NAME: CQCWebSrv_ThisFacility.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/01/2005
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
//  This file implements the facility class for the CQC web server.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCWebSrv.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TFacCQCWebSrv,TCQCSrvCore)



// ---------------------------------------------------------------------------
//   CLASS: TFacCQCWebSrv
//  PREFIX: fac
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TFacCQCWebSrv: Constructors and Destructor
// ---------------------------------------------------------------------------

//
//  We ask server core for a login and to enable the sending of events. We need
//  the former since user CML based handlers and web socket handlers can do things
//  that require credentials.
//
//  This is separate from web type credentials which is based on basic/digest
//  authentication and controls access to web server based resources. If any of
//  those are CML code, then the CQC level creds are needed.
//
//  Our HTTP/HTTPS ports we init to zero since we only want to use ones that are
//  initialized on the command line.
//
TFacCQCWebSrv::TFacCQCWebSrv() :

    TCQCSrvCore
    (
        L"CQCWebSrv"
        , L"CQC Web Server"
        , kCQCKit::ippnWebSrvDefPort
        , kCQCWebSrv::pszEvWebSrv
        , tCIDLib::EModFlags::HasMsgFile
        , tCQCSrvFW::ESrvOpts::EventsOut | tCQCSrvFW::ESrvOpts::LogIn
    )
    , m_bSecureHelp(kCIDLib::False)
    , m_ippnHTTP(0)
    , m_ippnHTTPS(0)
{
}

TFacCQCWebSrv::~TFacCQCWebSrv()
{
}


// ---------------------------------------------------------------------------
//  TFacCQCWebSrv: Protected, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid TFacCQCWebSrv::PostDeregTerm()
{
    //  Shut down the client facility
    facCQCWebSrvC().Terminate();
}


tCIDLib::TVoid TFacCQCWebSrv::PostUnbindTerm()
{
    // Stop the media cacher engine
    facCQCMedia().StopMDBCacher();
}


// Process our command parameters at this level
tCQCSrvFW::EStateRes TFacCQCWebSrv::eProcessParms(tCIDLib::TKVPList::TCursor& cursParms)
{
    m_bSecureHelp = kCIDLib::False;

    for (; cursParms; ++cursParms)
    {
        const TString& strKey = cursParms->strKey();

        if (strKey.bStartsWithI(L"HTMLDir"))
        {
            m_strHTMLDir = cursParms->strValue();
        }
         else if (strKey.bStartsWithI(L"HTTPPort"))
        {
            m_ippnHTTP = cursParms->strValue().c4Val();
        }
         else if (strKey.bStartsWithI(L"HTTPSPort"))
        {
            m_ippnHTTPS = cursParms->strValue().c4Val();
        }
         else if (strKey.bStartsWithI(L"CertInfo"))
        {
            m_strCertInfo = cursParms->strValue();
        }
         else if (strKey.bCompareI(L"SecureHelp"))
        {
            m_bSecureHelp = kCIDLib::True;
        }
         else
        {
            return tCQCSrvFW::EStateRes::Failed;
        }
    }

    //
    //  If we didn't get certificate info, insure the secure port is ignored, even if set, also
    //  disable secure help in this case as well. This shouldn't happen, but just in case.
    //
    if (m_strCertInfo.bIsEmpty())
    {
        m_bSecureHelp = kCIDLib::False;
        m_ippnHTTPS = 0;
    }

    if ((m_ippnHTTP == 0) && (m_ippnHTTPS == 0))
    {
        facCQCWebSrvC().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kCQCWSCErrs::errcCfg_NoPorts
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::BadParms
        );

        // We can't continue since there's no recovering from this
        tCQCSrvFW::EStateRes::Failed;
    }
    return tCQCSrvFW::EStateRes::Success;
}


// Called by the server framework to let us initialize stuff before ORB interfaces are registered
tCIDLib::TVoid TFacCQCWebSrv::PreRegInit()
{
    //
    //  Tell the interface engine that he is in remote mode, since we need that for our
    //  WebRIVA clients.
    //
    TFacCQCIntfEng::bRemoteMode(kCIDLib::True);

    // Invoke the CML runtime init on some other faclilities we need
    facCQCMEng().InitCMLRuntime(cuctxToUse().sectUser());
    facCQCMedia().InitCMLRuntime();

    // Initalize our client facility, both CML runtime and engine
    facCQCWebSrvC().InitCMLRuntime();
    facCQCWebSrvC().InitWebEngine
    (
        m_ippnHTTP, m_strHTMLDir, m_ippnHTTPS, m_strCertInfo, cuctxToUse(), mhashSrvPW()
    );

    //
    //  We have to ask the media facility to start up support for the local
    //  media cache (maintained by the client service), so that we can get
    //  media info. This is required by the WebRIVA stuff, and possibly other
    //  things.
    //
    facCQCMedia().StartMDBCacher();
}


//
//  Called by the server framework to get server admin interface info from us. This will
//  be used to register a generic admin interface for us.
//
tCIDLib::TVoid
TFacCQCWebSrv::QueryCoreAdminInfo(  TString&    strCoreAdminBinding
                                    , TString&  strCoreAdminDesc
                                    , TString&  strExtra1
                                    , TString&  strExtra2
                                    , TString&  strExtra3
                                    , TString&  strExtra4)
{
    strCoreAdminBinding = kCQCKit::pszCQCWebSrvCoreAdmin;
    strCoreAdminDesc = L"CQC Web Server Core Admin Object";

    // If this is the master server we set some of the extra values
    TString strHost;
    TString strMaster;
    TString strPort;
    if (facCIDOrbUC().strNSHost().bCompareI(TSysInfo::strIPHostName()))
    {
        //
        //  For the benefit of CQCAdmin who uses us to display help and needs to know how
        //  to find the MS web server and and what ports are available to use.
        //
        strExtra1 = kCQCKit::strMSWebSrv;
        strExtra2 = TSysInfo::strIPHostName();

        if (m_bSecureHelp)
        {
            strExtra4 = L"Secure";
            strExtra3.AppendFormatted(m_ippnHTTPS);
        }
         else
        {
            strExtra4 = L"Insecure";
            strExtra3.AppendFormatted(m_ippnHTTP);
        }
    }
}
