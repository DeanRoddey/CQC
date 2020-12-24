//
// FILE NAME: CQCRPortClient_ThisFacility.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/13/2005
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
//  This is the implementation file for the facility class.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCRPortClient_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TFacCQCRPortClient,TFacility)



// ---------------------------------------------------------------------------
//   CLASS: TFacCQCRPortClient
//  PREFIX: fac
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TFacCQCRPortClient: Constructors and Destructor
// ---------------------------------------------------------------------------
TFacCQCRPortClient::TFacCQCRPortClient() :

    TFacility
    (
        L"CQCRPortClient"
        , tCIDLib::EModTypes::SharedLib
        , kCQCKit::c4MajVersion
        , kCQCKit::c4MinVersion
        , kCQCKit::c4Revision
        , tCIDLib::EModFlags::None
    )
{
}

TFacCQCRPortClient::~TFacCQCRPortClient()
{
}


// ---------------------------------------------------------------------------
//  TFacCQCRPortClient: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Validates one of our types of paths and returns the port number from
//  the end of the path.
//
tCIDLib::TBoolean
TFacCQCRPortClient::bParsePath( const   TString&            strPath
                                ,       tCIDLib::TCard4&    c4PortNum) const
{
    if (!strPath.bStartsWithI(TRemSerialSrvClientProxy::strSerialPathPrefix))
        return kCIDLib::False;

    tCIDLib::TBoolean bOk;
    c4PortNum = TRawStr::c4AsBinary
    (
        strPath.pszBufferAt(TRemSerialSrvClientProxy::strSerialPathPrefix.c4Length())
        , bOk
        , tCIDLib::ERadices::Dec
    );
    return bOk;
}


//
//  Gen up one of our chat or serial port proxies.
//
//
//  Since we can't use the name server for this one, it has to have a well known
//  interface id and instance id. So all we need is the end point for the server,
//  and we can manually create an object id for the server side object.
//
tRPortClient::TChatClient
TFacCQCRPortClient::orbcMakeChatProxy(const TIPEndPoint& ipepSrv)
{
    TOrbObjId ooidSrv
    (
        TRemChatClientProxy::strInterfaceId
        , TRemChatClientProxy::strInstanceId
        , ipepSrv.strAsText()
        , ipepSrv.ippnThis()
        , L"TRemChatClientProxy"
    );

    tRPortClient::TChatClient orbcRet;
    orbcRet.SetPointer(new TRemChatClientProxy(ooidSrv, TString::strEmpty()));
    return orbcRet;
}

tRPortClient::TSerialPortClient
TFacCQCRPortClient::orbcMakePortProxy(const TIPEndPoint& ipepSrv)
{
    TOrbObjId ooidSrv
    (
        TRemSerialSrvClientProxy::strInterfaceId
        , TRemSerialSrvClientProxy::strInstanceId
        , ipepSrv.strAsText()
        , ipepSrv.ippnThis()
        , L"TRemSerialSrvClientProxy"
    );

    tRPortClient::TSerialPortClient orbcRet;
    orbcRet.SetPointer(new TRemSerialSrvClientProxy(ooidSrv, TString::strEmpty()));
    return orbcRet;
}


