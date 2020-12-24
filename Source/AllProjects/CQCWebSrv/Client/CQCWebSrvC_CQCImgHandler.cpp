//
// FILE NAME: CQCWebSrv_CQCImgHandler.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/09/2005
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
//  This file implements the CQC image repository URL handler.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCWebSrvC_.hpp"
#include    "CQCWebSrvC_CQCImgHandler_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TWSCQCImgHandler,TWSURLHandler)


// ---------------------------------------------------------------------------
//   CLASS: TWSCQCImgHandler
//  PREFIX: urlh
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// TWSCQCImgHandler: Constructors and Destructor
// ---------------------------------------------------------------------------
TWSCQCImgHandler::TWSCQCImgHandler()
{
}

TWSCQCImgHandler::~TWSCQCImgHandler()
{
}


// ---------------------------------------------------------------------------
// TWSCQCImgHandler: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TCard4
TWSCQCImgHandler::c4ProcessURL( const   TURL&               urlReq
                                , const TString&            strType
                                , const tCIDLib::TKVPList&  colInHdrLines
                                ,       tCIDLib::TKVPList&  colOutHdrLines
                                , const tCIDLib::TKVPList&
                                , const TString&            strEncoding
                                , const TString&
                                ,       THeapBuf&           mbufToFill
                                ,       tCIDLib::TCard4&    c4ContLen
                                ,       TString&            strContType
                                ,       TString&            strRepText)
{
    static const TString strLastModPref(L"CQCRepoSerNum:");

    // We can only handle GET operations
    if (strType != kCIDNet::pszHTTP_GET)
    {
        strRepText = L"File type not supported";
        c4ContLen = THTTPClient::c4BuildErrReply
        (
            kCIDNet::c4HTTPStatus_BadRequest
            , facCQCWebSrvC().strMsg(kCQCWSCErrs::errcGen_CantPUT)
            , strContType
            , mbufToFill
        );
        return kCIDNet::c4HTTPStatus_BadRequest;
    }

    //
    //  Get the path part of the URL, which we will convert to an image
    //  repository path. Expand it first to get rid of any encoded chars.
    //
    TPathStr pathImg;
    TURL::ExpandTo
    (
        urlReq.strPath()
        , pathImg
        , TURL::EExpTypes::Path
        , tCIDLib::EAppendOver::Overwrite
    );

    //
    //  Now cut off the /CQCImg prefix leaving us with the / starting char. THen
    //  remove the extension (they are all PNGs in the repository and the extension
    //  is not used in queries.) The remainder should be a valid image repository
    //  path.
    //
    pathImg.Cut(0, 7);
    pathImg.bRemoveExt();

    // Flip to the file style path separators from the URL style
    pathImg.bReplaceChar(L'/', L'\\');

    // Get data server client
    TDataSrvClient dsclLoad;

    //
    //  See if they passed in a if modified stamp, which would be one that
    //  we gave them. We just format out the serial number with a prefix. So
    //  we can pull that back out.
    //
    tCIDLib::TCard4 c4InSerialNum = 0;
    TString strInStamp;
    if (facCQCWebSrvC().bFindHdrLine(THTTPClient::strHdr_IfModifiedSince
                                    , strInStamp
                                    , colInHdrLines))
    {
        if (strInStamp.bStartsWith(strLastModPref))
        {
            strInStamp.Cut(0, strLastModPref.c4Length());
            if (!strInStamp.bToCard4(c4InSerialNum))
                c4InSerialNum = 0;
        }
    }

    // See if the image exists. If not, return a not found error status now
    if (!dsclLoad.bFileExists(pathImg, tCQCRemBrws::EDTypes::Image))
    {
        strRepText = L"File type not supported";
        c4ContLen = THTTPClient::c4BuildErrReply
        (
            kCIDNet::c4HTTPStatus_NotFound
            , facCQCWebSrvC().strMsg(kCQCWSCErrs::errcImg_CantLoad, pathImg)
            , strContType
            , mbufToFill
        );
        return kCIDNet::c4HTTPStatus_NotFound;
    }

    //
    //  Check to see if new data is available, passing in the incoming serial
    //  numberif any (else it's zero.)
    //
    tCIDLib::TCard4 c4NewSerNum = c4InSerialNum;
    tCIDLib::TEncodedTime enctLastChange;
    tCIDLib::TKVPFList colMeta;
    const tCIDLib::TBoolean bRes = dsclLoad.bReadImage
    (
        pathImg
        , c4NewSerNum
        , enctLastChange
        , mbufToFill
        , c4ContLen
        , colMeta
        , facCQCWebSrvC().cuctxToUse().sectUser()
    );

    // Create an outgoing stamp and add to the out header lines
    TString strOutStamp(strLastModPref);
    strOutStamp.AppendFormatted(c4NewSerNum);
    colOutHdrLines.objAdd
    (
        TKeyValuePair(THTTPClient::strHdr_LastModified, strOutStamp)
    );

    // If not changed, just return not modified
    if (!bRes)
    {
        c4ContLen = 0;
        strContType.Clear();
        return kCIDNet::c4HTTPStatus_NotModified;
    }

    strContType = L"image/png";
    return kCIDNet::c4HTTPStatus_OK;
}

