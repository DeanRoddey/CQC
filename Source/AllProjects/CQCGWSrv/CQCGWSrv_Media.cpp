//
// FILE NAME: CQCGWSrv_Media.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/04/2010
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
//  This file implements private methods of the worker thread that are related
//  to media metadata and such. There are a good number of them, with a lot
//  of formatting involved, so we split them out here.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCGWSrv.hpp"


// ---------------------------------------------------------------------------
//  Local helper macros to handle the optional use of prefixes on XML names.
//  The XML Gateway Server has to provide support for some of the media driver
//  calls, and just uses the same layout, but its names use a prefix, so we
//  want to be able to use the same code for the internal media driver use and
//  the GWS use.
// ---------------------------------------------------------------------------
#define FN(nm) L"CQCGW:" << L#nm

#define FA(nm,vl) L"CQCGW:" << L#nm << L"=\"" << vl << L"\" "

#define FEA(nm,vl) m_strmReply << L"CQCGW:" << L#nm << L"=\""; \
facCIDXML().EscapeFor(vl, m_strmReply, tCIDXML::EEscTypes::Attribute), TString::strEmpty(); \
m_strmReply << L"\" "

#define FSE(nm) m_strmReply << L"<" << L"CQCGW:" << L#nm << L">"

#define FSE2(nm) m_strmReply << L"    <" << L"CQCGW:" << L#nm << L">"

#define FEE(nm) m_strmReply << L"</" << L"CQCGW:" << L#nm << L">"

#define FOE(nm) m_strmReply << L"<" << L"CQCGW:" << L#nm << L" "




// ---------------------------------------------------------------------------
//  TWorkerThread: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  This method will return an XML formatted, ZLib compressed media database from
//  the client service cache. They pass in a serial number, which we check to see
//  if it's up to date or not. If it is, we don't have to return any data. If not,
//  then we return the new serial number and the new data.
//
tCIDLib::TVoid TWorkerThread::QueryMediaDB(const TXMLTreeElement& xtnodeReq)
{
    // Get out the moniker and serial number
    const TString& strMoniker = xtnodeReq.xtattrNamed(L"CQCGW:Moniker").strValue();
    const TString& strSerNum = xtnodeReq.xtattrNamed(L"CQCGW:DBSerialNum").strValue();


    // Let's check it and see what we get
    tCIDLib::TCard4 c4Bytes = 0;
    THeapBuf mbufData;
    TString strErrMsg;
    TString strRes;
    TString strRetSN;
    try
    {
        TFacCQCMedia::TMDBPtr cptrDB;
        tCIDLib::ELoadRes eRes = facCQCMedia().eGetMediaDB(strMoniker, strSerNum, cptrDB);
        if (eRes == tCIDLib::ELoadRes::NotFound)
        {
            strRes = L"Failed";
            strErrMsg = L"No data was found for this moniker";
        }
         else if (eRes == tCIDLib::ELoadRes::NewData)
        {
            //
            //  We have to return new data. Start the stream large enough to
            //  probably hold an average size DB, but it may end up being
            //  expanded. Set it up for UTF-8 for minimum size.
            //
            tCIDLib::TCard4 c4XMLBytes;
            THeapBuf mbufXML(0x200000, 0xA00000);
            {
                TTextMBufOutStream strmXML
                (
                    &mbufXML, tCIDLib::EAdoptOpts::NoAdopt, new TUTF8Converter
                );

                // Get the media types supported
                const TMediaDB& mdbSrc = cptrDB->mdbData();
                const tCQCMedia::EMTFlags eTypes = cptrDB->eMTFlags();

                // Put out the opening XML content
                strmXML << L"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n\n"
                        << L"<MediaDB>\n";

                // Do the formats supported
                if (tCIDLib::bAllBitsOn(eTypes, tCQCMedia::EMTFlags::Music))
                {
                    strmXML << L"<Music>\n";
                    mdbSrc.FormatXML(tCQCMedia::EMediaTypes::Music, strmXML);
                    strmXML << L"</Music>\n";
                }

                if (tCIDLib::bAllBitsOn(eTypes, tCQCMedia::EMTFlags::Movie))
                {
                    strmXML << L"<Movie>\n";
                    mdbSrc.FormatXML(tCQCMedia::EMediaTypes::Movie, strmXML);
                    strmXML << L"</Movie>\n";
                }

                // Close it out now
                strmXML << L"</MediaDB>\n";
                strmXML.Flush();
                c4XMLBytes = strmXML.c4CurSize();
            }

            // Get out the return serial number and set the return type
            strRes = L"NewData";
            strRetSN = cptrDB->strDBSerialNum();

            // Compress the data to a binary buffer
            TZLibCompressor zlibComp;
            TBinMBufInStream strmSrc(&mbufXML, c4XMLBytes);
            TBinMBufOutStream strmTar(&mbufData);
            c4Bytes = zlibComp.c4Compress(strmSrc, strmTar);
        }
         else
        {
            // They are up to date
            strRes = L"NoChanges";
            strRetSN = cptrDB->strDBSerialNum();
        }
    }

    catch(TError& errToCatch)
    {
        strRes = L"Failed";
        strErrMsg = errToCatch.strErrText();
    }


    // OK, we have he info we need
    m_strmReply.Reset();
    m_strmReply << L"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                << kCQCGWSrv::pszDTD
                << L"<CQCGW:Msg><CQCGW:MediaDBInfo CQCGW:DBSerialNum='"
                << strRetSN
                << L"' CQCGW:Result='"
                << strRes
                << L"'>";

    //
    //  If successful, put out the Base64 encoded text. Else, just put out the
    //  error message if failed.
    //
    if (strRes == L"Failed")
    {
        facCIDXML().EscapeFor(strErrMsg, m_strmReply, tCIDXML::EEscTypes::ElemText, TString::strEmpty());
    }
     else if (strRes == L"NewData")
    {
        TBase64 b64Encode;
        b64Encode.c4LineWidth(512);

        TBinMBufInStream strmSrc(&mbufData, c4Bytes);
        b64Encode.Encode(strmSrc, m_strmReply);
    }

    // Close off the two elements and send the reply
    m_strmReply << L"</CQCGW:MediaDBInfo>\n</CQCGW:Msg>\n" << kCIDLib::FlushIt;
    SendReply(m_strmReply.mbufData(), m_strmReply.c4CurSize());
}


//
//  This handles a query of current cover art from a media renderer. The
//  renderer will return whatever cover art it has for the currently playing
//  item, or nothing if there is no current item.
//
tCIDLib::TVoid
TWorkerThread::QueryMediaRendArt(const TXMLTreeElement& xtnodeReq)
{
    // We get the moniker and desired large/small size
    const TString& strRendMon = xtnodeReq.xtattrNamed(L"CQCGW:Moniker").strValue();
    const TString& strSize = xtnodeReq.xtattrNamed(L"CQCGW:Size").strValue();

    //
    //  We have to get a client repository proxy for the CQCServer hosting
    //  this renderer driver.
    //
    tCQCKit::TCQCSrvProxy orbcSrv = facCQCKit().orbcCQCSrvAdminProxy(strRendMon);

    //
    //  And now query the image. The size attribute values are correct for
    //  the renderer current art query size value, so just pass it through.
    //
    TString strRes;
    tCIDLib::TCard4 c4Size;
    THeapBuf mbufData;
    if (!orbcSrv->bQueryData(strRendMon
                            , kCQCMedia::strQuery_QueryArt
                            , strSize
                            , c4Size
                            , mbufData))
    {
        // Couldn't get it for some reason, so return no data
        c4Size = 0;
        strRes = L"Failed";
    }
     else
    {
        if (c4Size)
            strRes = L"NewData";
        else
            strRes = L"Failed";
    }

    //
    //  Format the reply. We embed the image data as element text in Base64
    //  format.
    //
    m_strmReply.Reset();
    m_strmReply << L"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                << kCQCGWSrv::pszDTD
                << L"<CQCGW:Msg><CQCGW:MediaImgData CQCGW:Bytes='"
                << c4Size
                << L"' CQCGW:Result='" << strRes
                << L"' CQCGW:Type='";


    // Now put out the type, and data if we got any
    if (c4Size)
    {
        //
        //  Probe the image data to see what type of image, and format out the
        //  Type attribute, closing off the start element in the process.
        //
        tCIDImage::EImgTypes eType = facCIDImgFact().eProbeImg(mbufData, c4Size);
        switch(eType)
        {
            case tCIDImage::EImgTypes::Bitmap :
            {
                m_strmReply << L"BMP'>";
                break;
            }

            case tCIDImage::EImgTypes::JPEG :
            {
                m_strmReply << L"JPEG'>";
                break;
            }

            case tCIDImage::EImgTypes::PNG :
            {
                m_strmReply << L"PNG'>";
                break;
            }

            default :
            case tCIDImage::EImgTypes::Unknown :
                m_strmReply << L"Unknown'>";
                break;
        };

        //
        //  Put an input stream over the image data and use a base 64
        //  encoder to stream it into the reply stream encoded.
        //
        TBinMBufInStream strmSrc(&mbufData, c4Size);
        TBase64 b64Encode;
        b64Encode.c4LineWidth(512);
        b64Encode.Encode(strmSrc, m_strmReply);

    }
     else
    {
        // No image returned, but we have to provide this attribute
        m_strmReply << L"Unknown'>";
        strRes = L"NoChanges";
    }

    // Close off the two elements and send the reply
    m_strmReply << L"</CQCGW:MediaImgData>\n</CQCGW:Msg>\n" << kCIDLib::FlushIt;
    SendReply(m_strmReply.mbufData(), m_strmReply.c4CurSize());
}


//
//  This handles a query of cover art from the client service cached image data.
//  The caller gives us a repo moniker, a size indicator, the unique id for the art,
//  and the id for the specific size image he wants. The later is so that we can check
//  for changes. If the repo has different images (instead of creating them all from
//  one large image), then the ids will be different.
//
tCIDLib::TVoid
TWorkerThread::QueryMediaArt(const TXMLTreeElement& xtnodeReq)
{
    //
    //  Get the moniker, cookie, and query type out. The cookie can be a
    //  title or collection cookie, depending on the type.
    //
    const TString& strUID = xtnodeReq.xtattrNamed(L"CQCGW:UID").strValue();
    const TString& strSID = xtnodeReq.xtattrNamed(L"CQCGW:SID").strValue();
    const TString& strRepoMon = xtnodeReq.xtattrNamed(L"CQCGW:Moniker").strValue();
    const TString& strSize = xtnodeReq.xtattrNamed(L"CQCGW:Size").strValue();
    const TString& strMType = xtnodeReq.xtattrNamed(L"CQCGW:MType").strValue();

    // Convert the media type into the standard enum
    tCQCMedia::EMediaTypes eMType;
    if (strMType.bCompareI(L"Music"))
        eMType = tCQCMedia::EMediaTypes::Music;
    else if (strMType.bCompareI(L"Movie"))
        eMType = tCQCMedia::EMediaTypes::Movie;
    else
    {
        facCQCMedia().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMedErrs::errcDB_UnknownMediaType
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Unknown
            , strMType
        );
    }

    // And let's ask the CQCMedia facility to load this image for us
    tCIDLib::TCard4 c4Size = 0;
    THeapBuf        mbufData(8);
    TString         strNewPerId;
    const tCIDLib::ELoadRes eRes = facCQCMedia().eQueryLocalCachedArt
    (
        strRepoMon
        , strUID
        , strSID
        , eMType
        , strSize.eCompareI(L"Lrg") == tCIDLib::ESortComps::Equal
        , c4Size
        , mbufData
        , strNewPerId
    );

    //
    //  Format the reply. We embed the image data as element text in Base64
    //  format.
    //
    m_strmReply.Reset();
    m_strmReply << L"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                << kCQCGWSrv::pszDTD
                << L"<CQCGW:Msg><CQCGW:MediaImgData CQCGW:Bytes='"
                << c4Size << L"' CQCGW:Result='";

    if (eRes == tCIDLib::ELoadRes::NewData)
    {
        m_strmReply << L"NewData"
                    << L"' CQCGW:SID='" << strNewPerId;
    }
     else if (eRes == tCIDLib::ELoadRes::NoNewData)
    {
        m_strmReply << L"NoChanges";
    }
     else
    {
        m_strmReply << L"Failed";
    }
    m_strmReply << L"'";

    m_strmReply << L" CQCGW:Type='";
    if (c4Size)
    {
        //
        //  Probe the image data to see what type of image, and format out the
        //  Type attribute, closing off the start element in the process.
        //
        const tCIDImage::EImgTypes eType = facCIDImgFact().eProbeImg(mbufData, c4Size);
        switch(eType)
        {
            case tCIDImage::EImgTypes::Bitmap :
            {
                m_strmReply << L"BMP'>\n";
                break;
            }

            case tCIDImage::EImgTypes::JPEG :
            {
                m_strmReply << L"JPEG'>\n";
                break;
            }

            case tCIDImage::EImgTypes::PNG :
            {
                m_strmReply << L"PNG'>\n";
                break;
            }

            default :
            case tCIDImage::EImgTypes::Unknown :
                m_strmReply << L"Unknown'>\n";
                break;
        };

        //
        //  Put an input stream over the image data and use a base 64
        //  encoder to stream it into the reply stream encoded.
        //
        TBinMBufInStream strmSrc(&mbufData, c4Size);
        TBase64 b64Encode;
        b64Encode.c4LineWidth(512);
        b64Encode.Encode(strmSrc, m_strmReply);
    }
     else
    {
        // No image returned, but we have to provide this attribute
        m_strmReply << L"Unknown'>\n";
    }

    // Close off the two elements and send the reply
    m_strmReply << L"</CQCGW:MediaImgData></CQCGW:Msg>\n" << kCIDLib::FlushIt;
    SendReply(m_strmReply.mbufData(), m_strmReply.c4CurSize());
}



//
//  This method will query the current playlist of a renderer driver and returns
//  key values for each item.
//
tCIDLib::TVoid TWorkerThread::QueryRendPL(const TXMLTreeElement& xtnodeReq)
{
    // Get the moniker of the renderer
    const TString& strMoniker = xtnodeReq.xtattrNamed(L"CQCGW:Moniker").strValue();

    //
    //  We have to get a client repository proxy for the CQCServer hosting
    //  this driver.
    //
    tCQCKit::TCQCSrvProxy orbcSrv = facCQCKit().orbcCQCSrvAdminProxy(strMoniker);

    // Format out the message start
    m_strmReply.Reset();
    m_strmReply << L"<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
                << kCQCGWSrv::pszDTD
                << L"<CQCGW:Msg>\n"
                << FOE(RendPLInfo);

    // Query the category list from this renderer
    TVector<TCQCMediaPLItem> colItems;
    const tCIDLib::TCard4 c4RetItems = facCQCMedia().c4QueryPLItems
    (
        orbcSrv, strMoniker, colItems
    );

    // Add the count attribute, which we don't know until now
    m_strmReply << FA(Count, c4RetItems) << L">\n";

    FSE(Names);
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4RetItems; c4Index++)
    {
        FSE2(Name);
        facCIDXML().EscapeFor
        (
            colItems[c4Index].strItemName(), m_strmReply, tCIDXML::EEscTypes::ElemText, TString::strEmpty()
        );
        FEE(Name);
    }
    FEE(Names);

    FSE(Cookies);
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4RetItems; c4Index++)
    {
        FSE2(Cookie);
        facCIDXML().EscapeFor
        (
            colItems[c4Index].strItemCookie(), m_strmReply, tCIDXML::EEscTypes::ElemText, TString::strEmpty()
        );
        FEE(Cookie);
    }
    FEE(Cookies);

    FSE(Locs);
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4RetItems; c4Index++)
    {
        FSE2(Loc);
        facCIDXML().EscapeFor
        (
            colItems[c4Index].strLocInfo(), m_strmReply, tCIDXML::EEscTypes::ElemText, TString::strEmpty()
        );
        FEE(Loc);
    }
    FEE(Locs);

    FSE(Artists);
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4RetItems; c4Index++)
    {
        FSE2(Artist);
        facCIDXML().EscapeFor
        (
            colItems[c4Index].strItemArtist(), m_strmReply, tCIDXML::EEscTypes::ElemText, TString::strEmpty()
        );
        FEE(Artist);
    }
    FEE(Artists);

    FSE(Durations);
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4RetItems; c4Index++)
    {
        FSE(Duration) << colItems[c4Index].c4ItemSeconds() << FEE(Duration);
    }
    FEE(Durations);

    // In this case we return the list ids
    FSE(Ids);
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4RetItems; c4Index++)
    {
        FSE(Id) << colItems[c4Index].c4ListItemId() << FEE(Id);
    }
    FEE(Ids);

    // Close off the main element
    FEE(RendPLInfo);

    // Now close out the root element and send
    m_strmReply << L"</CQCGW:Msg>" << kCIDLib::EndLn;
    SendReply(m_strmReply.mbufData(), m_strmReply.c4CurSize());
}

