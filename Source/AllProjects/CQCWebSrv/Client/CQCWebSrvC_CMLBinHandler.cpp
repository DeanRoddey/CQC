//
// FILE NAME: CQCWebSrvC_CMLBinHandler.cpp
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
//  This file implements the CMLBin URL handler, which calls out to a CML
//  macro to allow it to generate a page.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCWebSrvC_.hpp"
#include    "CQCWebSrvC_CMLBinHandler_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TWSCMLBinHandler,TWSURLHandler)


// ---------------------------------------------------------------------------
//   CLASS: TWSCMLBinHandler
//  PREFIX: urlh
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// TWSCMLBinHandler: Constructors and Destructor
// ---------------------------------------------------------------------------
TWSCMLBinHandler::TWSCMLBinHandler()
{
}

TWSCMLBinHandler::~TWSCMLBinHandler()
{
}


// ---------------------------------------------------------------------------
// TWSCMLBinHandler: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TCard4
TWSCMLBinHandler::c4ProcessURL( const   TURL&               urlReq
                                , const TString&            strType
                                , const tCIDLib::TKVPList&  colInHdrLines
                                ,       tCIDLib::TKVPList&
                                , const tCIDLib::TKVPList&  colPQVals
                                , const TString&            strEncoding
                                , const TString&            strBoundary
                                ,       THeapBuf&           mbufToFill
                                ,       tCIDLib::TCard4&    c4ContLen
                                ,       TString&            strContType
                                ,       TString&            strRepText)
{
    //
    //  Get the path part of the URL, which we will convert to a macro
    //  repository path. Expand it to get rid of any encoded chars.
    //
    TString strExpPath;
    TURL::ExpandTo
    (
        urlReq.strPath()
        , strExpPath
        , TURL::EExpTypes::Path
        , tCIDLib::EAppendOver::Overwrite
    );

    //
    //  Now we convert it to a CML macro path. The /CMLBin/User/ at the start
    //  of the path, will be flipped and become /User/CMLBin/, since all of the
    //  CMLBin macros will be in the /User space and under the CMLBin subdir.
    //
    //  So put in what we know will be the starting part of the new path, then
    //  copy from the expanded URL path past the /CMLBin/User/ part onto that,
    //  after converting slashes to periods.
    //
    TPathStr pathToUse(L"MEng.User.CMLBin.");
    strExpPath.bReplaceChar(L'/', L'.');
    pathToUse.AppendSubStr(strExpPath, 13);
    if (pathToUse.chLast() == L'.')
        pathToUse.DeleteLast();

    //
    //  They can put an .html extension on it, in order to keep stupid browsers
    //  like IE from thinking it should download a file. So strip that off.
    //
    TString strExt;
    if (pathToUse.bQueryExt(strExt))
    {
        if ((strExt == L"html") || (strExt == L"htm"))
            pathToUse.bRemoveExt();
    }

    //
    //  Set up our macro engine and the handlers we need and get them plugged
    //  into the macro engine.
    //
    TCQCMEngErrHandler          meehLogger;
    TMEngFixedBaseFileResolver  mefrData(facCQCKit().strMacroRootPath());
    TMacroEngParser             meprsToUse;
    TCIDMacroEngine             meTarget;
    meTarget.SetErrHandler(&meehLogger);
    meTarget.SetFileResolver(&mefrData);

    // Try to download and parse the macro. If this fails, we return an error page
    TMEngClassInfo* pmeciTar;
    {
        TCQCMEngClassMgr    mecmCQC(facCQCWebSrvC().sectUser());
        TCQCPrsErrHandler   meehParse;
        if (!meprsToUse.bParse(pathToUse, pmeciTar, &meTarget, &meehParse, &mecmCQC))
        {
            strRepText = L"Macro compile failed";
            c4ContLen = THTTPClient::c4BuildErrReply
            (
                kCIDNet::c4HTTPStatus_SrvError
                , facCQCWebSrvC().strMsg(kCQCWSCErrs::errcMacro_CantLoad, pathToUse)
                , strContType
                , mbufToFill
            );
            return kCIDNet::c4HTTPStatus_SrvError;
        }
    }

    //
    //  Make sure it derives from the correct base class, else we can't
    //  use it. If not, then return an error page.
    //
    TCMLBinBaseInfo* pmeciParent = meTarget.pmeciFindAs<TCMLBinBaseInfo>
    (
        TCMLBinBaseInfo::strPath(), kCIDLib::True
    );

    if (pmeciTar->c2ParentClassId() != pmeciParent->c2Id())
    {
        strRepText = L"Incorrect base class";
        c4ContLen = THTTPClient::c4BuildErrReply
        (
            kCIDNet::c4HTTPStatus_SrvError
            , facCQCWebSrvC().strMsg
              (
                kCQCWSCErrs::errcMacro_CMLBinBaseClass, TCMLBinBaseInfo::strPath()
              )
            , strContType
            , mbufToFill
        );
        return kCIDNet::c4HTTPStatus_SrvError;
    }

    // Looks ok, so create the value object for the main class
    TMEngClassVal* pmecvTarget = pmeciTar->pmecvMakeStorage
    (
        L"$Main$", meTarget, tCIDMacroEng::EConstTypes::NonConst
    );
    TJanitor<TMEngClassVal> janClass(pmecvTarget);

    // Call its default constructor. If it fails, return an error page
    if (!meTarget.bInvokeDefCtor(*pmecvTarget, &facCQCWebSrvC().cuctxToUse()))
    {
        strRepText = L"Macro ctor failed";
        c4ContLen = THTTPClient::c4BuildErrReply
        (
            kCIDNet::c4HTTPStatus_SrvError
            , facCQCWebSrvC().strMsg(kCQCWSCErrs::errcMacro_DefCtor, pathToUse)
            , strContType
            , mbufToFill
        );
        return kCIDNet::c4HTTPStatus_SrvError;
    }

    //
    //  Now we need to load up the vector in the CML level parent class with
    //  the key/value pairs. So we have to find the value list member of the
    //  object. Cast it to its actual type.
    //
    TMEngVectorVal* pmecvValues = static_cast<TMEngVectorVal*>
    (
        pmecvTarget->pmecvFind(TCMLBinBaseInfo::strValListName)
    );

    // If not found, then something is really wrong, so return an error page
    if (!pmecvValues)
    {
        strRepText = L"CMLBin error";
        c4ContLen = THTTPClient::c4BuildErrReply
        (
            kCIDNet::c4HTTPStatus_SrvError
            , L"The m_ReqValues member could not be found in the invoked class"
            , strContType
            , mbufToFill
        );
        return kCIDNet::c4HTTPStatus_SrvError;
    }

    // Ok, let's load it up. Flush it first
    pmecvValues->RemoveAll();
    const TMEngClassInfo* pmeciKVPair = meTarget.pmeciFindAs<TMEngKVPairInfo>
    (
        TMEngKVPairInfo::strPath(), kCIDLib::True
    );
    const tCIDLib::TCard4 c4ValCount = colPQVals.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ValCount; c4Index++)
    {
        const TKeyValuePair& kvalCur = colPQVals[c4Index];
        pmecvValues->AddObject
        (
            new TMEngKVPairVal
            (
                TString::strEmpty()
                , pmeciKVPair->c2Id()
                , tCIDMacroEng::EConstTypes::Const
                , kvalCur.strKey()
                , kvalCur.strValue()
            )
        );
    }


    //
    //  And we need to store the content buffer length and, if there is any content,
    //  we need to store the buffer contents and the encoding.
    //
    TMEngCard4Val* pmecvBufLen = static_cast<TMEngCard4Val*>
    (
        pmecvTarget->pmecvFind(TCMLBinBaseInfo::strContBufLenName)
    );
    TMEngMemBufVal* pmecvContBuf = static_cast<TMEngMemBufVal*>
    (
        pmecvTarget->pmecvFind(TCMLBinBaseInfo::strContBufName)
    );

    TMEngStringVal* pmecvContBoundary = static_cast<TMEngStringVal*>
    (
        pmecvTarget->pmecvFind(TCMLBinBaseInfo::strContBoundaryName)
    );
    TMEngStringVal* pmecvContEncoding = static_cast<TMEngStringVal*>
    (
        pmecvTarget->pmecvFind(TCMLBinBaseInfo::strContEncodingName)
    );
    TMEngStringVal* pmecvContType = static_cast<TMEngStringVal*>
    (
        pmecvTarget->pmecvFind(TCMLBinBaseInfo::strContTypeName)
    );

    if (!pmecvBufLen
    ||  !pmecvContBuf
    ||  !pmecvContBoundary
    ||  !pmecvContEncoding
    ||  !pmecvContType)
    {
        strRepText = L"CMLBin error";
        c4ContLen = THTTPClient::c4BuildErrReply
        (
            kCIDNet::c4HTTPStatus_SrvError
            , L"One or more of the body content members could not be found in the invoked class"
            , strContType
            , mbufToFill
        );
        return kCIDNet::c4HTTPStatus_SrvError;
    }

    pmecvBufLen->c4Value(c4ContLen);
    if (c4ContLen)
    {
        pmecvContBoundary->strValue(strBoundary);
        pmecvContType->strValue(strContType);
        pmecvContEncoding->strValue(strEncoding);
        pmecvContBuf->mbufValue().CopyIn(mbufToFill, c4ContLen);
    }

    //
    //  Set up the call stack. We don't invoke the Start() method because we
    //  want to leave that for debugging in the IDE. We call directly in to
    //  a ProcessReq() method, so we have to set up the stack ourself and do
    //  the call.
    //
    //  The parms for the process request method are:
    //
    //      1.  An output memory buffer to format the return data into.
    //
    //      2.  An output Card4 to indicate the size of the data put into
    //          the buffer.
    //
    //      3.  An output String to put the output content type into
    //
    //      4.  An output String for header response text
    //
    //      5.  An input boolean that indicates if it is a GET or a PUT. If
    //          true it's a GET.
    //
    //  The return value is a result code. If the result code is not
    //  200, i.e. ok, then the output content type string should be
    //  filled with the error code text to be sent back. So push that first.
    //
    //  Put a stack unwind janitor on it to insure the stack gets cleaned up
    //  now matter how we exit.
    //
    //  Note that we make any non-intrinsic values we push to be 'local'
    //  types, which makes the macro engine see them as method locals, so
    //  he'll delete them as they are popped off the stack. For intrinsic
    //  types we can just push values from the macro engine's pool.
    //
    tCIDLib::TCard4 c4Ret = 0;
    try
    {
        TMEngCallStackJan janStack(&meTarget);
        meTarget.PushPoolValue(tCIDMacroEng::EIntrinsics::Card4, tCIDMacroEng::EConstTypes::NonConst);

        //
        //  Push the the memory buffer. The CML memory buffer class has a special
        //  constuctor to allow us to pass in a buffer of ours that he just uses,
        //  so that we don't have to copy in and out. He doesn't adopt it.
        //
        TMEngMemBufInfo* pmeciMBuf = meTarget.pmeciFindAs<TMEngMemBufInfo>
        (
            TMEngMemBufInfo::strPath(), kCIDLib::True
        );
        TMEngMemBufVal* pmecvBuf = new TMEngMemBufVal
        (
            L"ToFill", pmeciMBuf->c2Id(), tCIDMacroEng::EConstTypes::NonConst, &mbufToFill
        );
        meTarget.PushValue(pmecvBuf, tCIDMacroEng::EStackItems::Local);

        // And push the rest, which are easy ones
        meTarget.PushPoolValue(tCIDMacroEng::EIntrinsics::Card4, tCIDMacroEng::EConstTypes::NonConst);
        meTarget.PushPoolValue(tCIDMacroEng::EIntrinsics::String, tCIDMacroEng::EConstTypes::NonConst);
        meTarget.PushPoolValue(tCIDMacroEng::EIntrinsics::String, tCIDMacroEng::EConstTypes::NonConst);
        meTarget.PushBoolean((strType != kCIDNet::pszHTTP_POST), tCIDMacroEng::EConstTypes::Const);

        //
        //  And push the method call and then call it. We looked up the method
        //  id above when we confirmed the target class had provided it.
        //
        const tCIDLib::TCard2 c2MethId = pmeciTar->c2FindMethod
        (
            TCMLBinBaseInfo::strProcessReqMeth
        );
        meTarget.meciPushMethodCall(pmeciTar->c2Id(), c2MethId);
        pmeciTar->Invoke
        (
            meTarget, *pmecvTarget, c2MethId, tCIDMacroEng::EDispatch::Poly
        );

        //
        //  Get the return value and output params out before we clear the stack.
        //  The return value is the number of parms, plus the method call plus
        //  the return value (7) down from the top of the stack.
        //
        c4Ret = meTarget.c4StackValAt(meTarget.c4StackTop() - 7);

        strRepText = meTarget.strStackValAt(meTarget.c4StackTop() - 3);
        strContType = meTarget.strStackValAt(meTarget.c4StackTop() - 4);
        c4ContLen = meTarget.c4StackValAt(meTarget.c4StackTop() - 5);
    }

    // Handle a CML level exception here since the caller won't understand them
    catch(const TExceptException&)
    {
        TString strClass;
        TString strText;
        tCIDLib::TCard4 c4Line;
        meTarget.bGetLastExceptInfo(strClass, strText, c4Line);

        TString strMsg(L"[", 1024);
        strMsg.Append(strClass);
        strMsg.Append(kCIDLib::chComma);
        strMsg.AppendFormatted(c4Line);
        strMsg.Append(L"] - ");
        strMsg.Append(strText);

        // We rethrow it as a regular CIDLib exception
        TLogEvent logevToThrow
        (
            facCQCWebSrvC().strName()
            , strClass
            , c4Line
            , strMsg
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::AppStatus
        );
        logevToThrow.strProcess(L"CQCWebSrv.exe");
        logevToThrow.strHostName(TSysInfo::strIPHostName());
        throw logevToThrow;
    }
    return c4Ret;
}

