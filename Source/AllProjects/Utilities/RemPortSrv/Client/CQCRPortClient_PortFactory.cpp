//
// FILE NAME: CQCRPortClient_PortFactory.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/14/2005
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
//  This file implements the port factory for our remote ports.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


// ---------------------------------------------------------------------------
//  Facility specific includes
// ---------------------------------------------------------------------------
#include    "CQCRPortClient_.hpp"


// ---------------------------------------------------------------------------
//  Magic RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCRemSrvPortFactory,TComPortFactory)



// ---------------------------------------------------------------------------
//  CLASS: TCQCRemSrvPortFactory
// PREFIX: pfact
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCRemSrvPortFactory: Constructors and Destructor
// ---------------------------------------------------------------------------

// Indicate that these ports can be configured
TCQCRemSrvPortFactory::TCQCRemSrvPortFactory(const TIPEndPoint& ipepSrv) :

    TComPortFactory
    (
        TRemSerialSrvClientProxy::strSerialFactoryId
        , TRemSerialSrvClientProxy::strSerialPathPrefix
        , kCIDLib::True
    )
    , m_ipepServer(ipepSrv)
{
}

TCQCRemSrvPortFactory::~TCQCRemSrvPortFactory()
{
}


// ---------------------------------------------------------------------------
//  TCQCRemSrvPortFactory: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  We put all the ports we support into the passed collection. We don't
//  clear it first since the facility class is building up a list from all
//  the installed factories.
//
tCIDLib::TBoolean
TCQCRemSrvPortFactory::bQueryPorts(         TCollection<TString>& colToFill
                                    , const tCIDLib::TBoolean     bAvailOnly) const
{
    tCIDLib::TBoolean bRet;
    try
    {
        // Try to get a client proxy for our server
        tRPortClient::TSerialPortClient orbcServer
        (
            facCQCRPortClient().orbcMakePortProxy(m_ipepServer)
        );

        //
        //  We got it, so query the ports. We can't do polymorphism via the
        //  ORB interface, so create a temp vector, do the call, then copy
        //  to the caller's collection.
        //
        TVector<TString> colTmp;
        bRet = orbcServer->bQueryCommPorts(colTmp);
        if (bRet)
        {
            const tCIDLib::TCard4 c4Count = colTmp.c4ElemCount();
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
                colToFill.objAdd(colTmp[c4Index]);
        }
    }

    catch(const TError& errToCatch)
    {
        if (facCQCRPortClient().bShouldLog(errToCatch))
            TModule::LogEventObj(errToCatch);
        return kCIDLib::False;
    }
    return bRet;
}


// We will accept any configuration
tCIDLib::TBoolean
TCQCRemSrvPortFactory::bTestCfg(const TCommPortCfg&, TString&) const
{
    return kCIDLib::True;
}


tCIDLib::TBoolean
TCQCRemSrvPortFactory::bValidatePath(const TString& strPath) const
{
    tCIDLib::TCard4 c4Dummy;
    return facCQCRPortClient().bParsePath(strPath, c4Dummy);
}


TCommPortBase* TCQCRemSrvPortFactory::pcommMakeNew(const TString& strPath)
{
    tCIDLib::TCard4 c4PortNum;
    if (!facCQCRPortClient().bParsePath(strPath, c4PortNum))
        return 0;
    return new TRemCommPort(strPath, m_ipepServer);
}


// ---------------------------------------------------------------------------
//  TCQCRemSrvPortFactory: Public, non-virtual methods
// ---------------------------------------------------------------------------
const TIPEndPoint& TCQCRemSrvPortFactory::ipepServer() const
{
    return m_ipepServer;
}


