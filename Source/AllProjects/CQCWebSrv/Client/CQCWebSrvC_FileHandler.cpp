//
// FILE NAME: CQCWebSrv_FileHandler.cpp
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
//  This file implements the file-based URL handler.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCWebSrvC_.hpp"
#include    "CQCWebSrvC_FileHandler_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TWSFileHandler,TWSURLHandler)


// ---------------------------------------------------------------------------
//   CLASS: TWSFileHandler
//  PREFIX: urlh
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// TWSFileHandler: Constructors and Destructor
// ---------------------------------------------------------------------------
TWSFileHandler::TWSFileHandler()
{
}

TWSFileHandler::~TWSFileHandler()
{
}


// ---------------------------------------------------------------------------
// TWSFileHandler: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TCard4
TWSFileHandler::c4ProcessURL(const  TURL&               urlReq
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

    // Build up the actual local path to the file
    TPathStr pathFile(facCQCWebSrvC().strRootHTMLPath());
    if (urlReq.strPath() == L"/")
    {
        // Use the default file name since none was given
        pathFile.AddLevel(L"index.html");
    }
     else
    {
        // Expand out the potentially URL encoded path
        TString strURLPath;
        TURL::ExpandTo
        (
            urlReq.strPath()
            , strURLPath
            , TURL::EExpTypes::Path
            , tCIDLib::EAppendOver::Overwrite
        );

        // Convert from URL to file style separators
        strURLPath.bReplaceChar(L'/', L'\\');

        pathFile.AddLevel(strURLPath);
    }

    tCIDLib::TCard4 c4Ret = kCIDNet::c4HTTPStatus_OK;
    if (TFileSys::bExists(pathFile, tCIDLib::EDirSearchFlags::NormalFiles))
    {
        // Read the file into a buffer
        TBinaryFile flSrc(pathFile);
        flSrc.Open
        (
            tCIDLib::EAccessModes::Multi_Read
            , tCIDLib::ECreateActs::OpenIfExists
            , tCIDLib::EFilePerms::Default
            , tCIDLib::EFileFlags::SequentialScan
        );

        mbufToFill.Reallocate(tCIDLib::TCard4(flSrc.c8CurSize()));
        const tCIDLib::TCard4 c4FlSz = flSrc.c4ReadBuffer
        (
            mbufToFill, tCIDLib::TCard4(flSrc.c8CurSize())
        );

        // Look at the extension to figure out the content type
        pathFile.bQueryExt(strContType);
        strContType.ToUpper();

        if ((strContType == L"HTML")
        ||  (strContType == L"HTM")
        ||  (strContType == L"JSON")
        ||  (strContType == L"WML")
        ||  (strContType == L"WMLS")
        ||  (strContType == L"XML"))
        {
            //
            //  These are processed to handle token replacement. Try to figure out
            //  the encoding, else assume Latin1.
            //
            TString strEncoding;
            if (!facCIDEncode().bProbeForEncoding(mbufToFill, c4FlSz, strEncoding))
                strEncoding = L"Latin1";

            TTextMBufInStream strmSrc
            (
                tCIDLib::ForceMove(mbufToFill), c4FlSz, facCIDEncode().ptcvtMake(strEncoding)
            );

            mbufToFill.Reallocate(tCIDLib::TCard4(flSrc.c8CurSize()));
            TTextMBufOutStream strmOut
            (
                &mbufToFill, tCIDLib::EAdoptOpts::NoAdopt, new TUTF8Converter
            );

            //
            //  For each line, we read it in, do the token replacement
            //  pass on it, and write it back out. This is more efficient
            //  than doing the pass on the whole thing at once since there
            //  is much less moving around of text as tokens are removed
            //  and their replacement values stuck in.
            //
            TString strIn;
            TString strOut;
            TCQCCmdRTVSrc crtvToUse(facCQCWebSrvC().cuctxToUse());
            while (!strmSrc.bEndOfStream())
            {
                strmSrc >> strIn;

                // Do token processing without escapement
                const tCQCKit::ECmdPrepRes eRes = facCQCKit().eStdTokenReplace
                (
                    strIn, &crtvToUse, 0, 0, strOut, tCQCKit::ETokRepFlags::NoEscape

                );

                if (eRes == tCQCKit::ECmdPrepRes::Changed)
                    strmOut << strOut << kCIDLib::NewLn;
                else
                    strmOut << strIn << kCIDLib::NewLn;
            }
            strmOut.Flush();
            c4ContLen = strmOut.c4CurSize();
            if (strContType == L"WML")
                strContType = L"text/vnd.wap.wml; charset=utf-8";
            else if (strContType == L"WMLS")
                strContType = L"text/vnd.wap.wmlscript; charset=utf-8";
            else if (strContType == L"XML")
                strContType = L"text/xml; charset=utf-8";
            else if (strContType == "JSON")
                strContType = L"application/json; charset=utf-8";
            else
                strContType = L"text/html; charset=utf-8";
        }
         else if ((strContType == L"CSS")
              ||  (strContType == L"JS")
              ||  (strContType == L"TS")
              ||  (strContType == L"MAP"))
        {
            //
            //  For these no preprocessing and no caching. Caching these seems to
            //  freak Chrome out on refreshes.
            //
            if (strContType == L"CSS")
                strContType = L"text/css; charset=utf-8";
            else if (strContType == L"JS")
                strContType = L"text/javascript; charset=utf-8";
            else if (strContType == L"TS")
                strContType = L"application/x-typescript; charset=utf-8";
            else if (strContType == L"MAP")
                strContType = L"text; charset=utf-8";

            //
            //  Probe it to see if it's something we can figure out. If not, assume
            //  Latin1.
            //
            TString strEncoding;
            if (!facCIDEncode().bProbeForEncoding(mbufToFill, c4FlSz, strEncoding))
                strEncoding = L"Latin1";

            // If already UTF-8, nothing to do, else process it
            if (strEncoding == L"UTF-8")
            {
                c4ContLen = c4FlSz;
            }
             else
            {
                TTextMBufInStream strmSrc
                (
                    tCIDLib::ForceMove(mbufToFill), c4FlSz, facCIDEncode().ptcvtMake(strEncoding)
                );

                mbufToFill.Reallocate(tCIDLib::TCard4(flSrc.c8CurSize()));
                TTextMBufOutStream strmOut
                (
                    &mbufToFill, tCIDLib::EAdoptOpts::NoAdopt, new TUTF8Converter
                );
                strmOut.eNewLineType(tCIDLib::ENewLineTypes::LF);

                // Read in the file and write it back out in the UTF-8 format
                TString strIn;
                TString strOut;
                while (!strmSrc.bEndOfStream())
                {
                    strmSrc >> strIn;
                    strmOut << strIn << kCIDLib::NewLn;
                }
                strmOut.Flush();

                c4ContLen = strmOut.c4CurSize();
            }
        }
         else if ((strContType == L"JPEG") || (strContType == L"JPG")
              ||  (strContType == L"PNG") || (strContType == L"GIF")
              ||  (strContType == L"WBMP") || (strContType == L"BMP")
              ||  (strContType == L"ICO"))
        {
            // Set the content type we will return
            if (strContType == L"GIF")
                strContType = L"image/gif";
            else if (strContType == L"ICO")
                strContType = L"image/x-icon";
            else if (strContType == L"PNG")
                strContType = L"image/png";
            else if (strContType == L"WBMP")
                strContType = L"image/vnd.wap.wbmp";
            else if (strContType == L"BMP")
                strContType = L"image/bmp";
            else
                strContType = L"image/jpeg";

            // Set up the outgoing last modified stamp and add to the out hdr
            TString strVal;
            flSrc.tmLastWrite().FormatToStr(strVal, TTime::strCTime());
            colOutHdrLines.objAdd
            (
                TKeyValuePair(THTTPClient::strHdr_LastModified, strVal)
            );

            //
            //  See if they passed a if modified stamp back to us. If so, then se
            //   if it's changed, and we can send back a not-modified response if so.
            //
            TString strInStamp;
            if (facCQCWebSrvC().bFindHdrLine(THTTPClient::strHdr_IfModifiedSince
                                            , strInStamp
                                            , colInHdrLines)
            &&  (strInStamp == strVal))
            {
                c4Ret = kCIDNet::c4HTTPStatus_NotModified;
                c4ContLen = 0;
                strContType.Clear();
                strRepText = L"Unmodified";
            }
             else
            {
                //
                //  It has changed or they didn't provide a stamp to compare
                //  so return the full contents.
                //
                c4ContLen = c4FlSz;
            }

            //
            //  Give back a cache control to make it good for 60 seconds.
            //
            strVal = THTTPClient::strCC_MaxAge;
            strVal.Append(L"=60");
            colOutHdrLines.objAdd(TKeyValuePair(THTTPClient::strHdr_CacheControl, strVal));
        }
         else
        {
            strRepText = L"File type not supported";
            c4ContLen = THTTPClient::c4BuildErrReply
            (
                kCIDNet::c4HTTPStatus_UnsupMedia
                , facCQCWebSrvC().strMsg(kCQCWSCErrs::errcFile_SupportedType)
                , strContType
                , mbufToFill
            );
            c4Ret = kCIDNet::c4HTTPStatus_UnsupMedia;
        }
    }
     else
    {
        strRepText = L"File not found";
        c4ContLen = THTTPClient::c4BuildErrReply
        (
            kCIDNet::c4HTTPStatus_NotFound
            , facCQCWebSrvC().strMsg(kCQCWSCErrs::errcFile_NotFound)
            , strContType
            , mbufToFill
        );
        c4Ret = kCIDNet::c4HTTPStatus_NotFound;
    }
    return c4Ret;
}


