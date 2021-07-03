//
// FILE NAME: CQCGWSrv_WorkerQueries.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/16/2005
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
//  This file implements private methods of the work thread that are related
//  to information query. There is a good bit of code, so these actual
//  message handling methods are split out into a few files.
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
//  TWorkerThread: Private, non-virtual methods
// ---------------------------------------------------------------------------
//
//  Allows the client to get the values of the current field list that
//  they have set.
//
tCIDLib::TVoid TWorkerThread::PollFields()
{
    // Build the starting part of the message
    m_strmReply.Reset();
    m_strmReply << L"<?xml version='1.0' encoding='UTF-8'?>\n"
                << kCQCGWSrv::pszDTD
                << L"<CQCGW:Msg><CQCGW:PollReply>";

    // Get a convenience ref to the interface engine
    TCQCPollEngine& polleToUse = facCQCGWSrv.polleThis();

    //
    //  Run through the fields and update any that have changed. For the
    //  most part, this will be a pretty quick run through that does
    //  nothing.
    //
    const tCIDLib::TCard4 c4Count = m_colFields.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TCQCFldPollInfo& cfpiCur = m_colFields[c4Index];

        // Set the current state and then update
        const tCQCPollEng::EFldStates eOldState = cfpiCur.eState();
        const tCIDLib::TBoolean bChanged = cfpiCur.bUpdateValue(polleToUse);

        //
        //  Now let's build up the reply for this one. If we have new
        //  fields, i.e. this is the first pass, we always return the
        //  info for each field. Otherwise return only if someting has
        //  changed.
        //
        if (m_bNewFields || bChanged)
        {
            //
            //  If in ready state, then we have good data to return. Else,
            //  wereturn the error value.
            //
            if (cfpiCur.eState() == tCQCPollEng::EFldStates::Ready)
                FormatGoodValue(m_strmReply, cfpiCur, c4Index);
            else
                FormatBadValue(m_strmReply, cfpiCur, c4Index);
        }
    }

    // Close off the device list tag and finish it off
    m_strmReply << L"</CQCGW:PollReply></CQCGW:Msg>"
                << kCIDLib::FlushIt;

    //
    //  Clear the new fields flag, in case it was set, since we've now done
    //  at least one return of all the field values.
    //
    m_bNewFields = kCIDLib::False;

    // And now send the reply back
    SendReply(m_strmReply.mbufData(), m_strmReply.c4CurSize());
}


//
//  Return a list of the available drivers. It returns the same element type
//  as QueryFields, but without the fields, just the driver elements.
//
tCIDLib::TVoid TWorkerThread::QueryDrvList()
{
    // Look up all of the drivers and basic info about them
    tCIDLib::TStrList       colMonikers;
    tCIDLib::TStrList       colMakes;
    tCIDLib::TStrList       colModels;
    tCQCKit::TDevCatList    fcolCats;
    const tCIDLib::TBoolean bRes = facCQCKit().bFindAllDrivers
    (
        colMonikers, colMakes, colModels, fcolCats
    );

    // Start the message to the open device list tag
    m_strmReply.Reset();
    m_strmReply << L"<?xml version='1.0' encoding='UTF-8'?>\n"
                << kCQCGWSrv::pszDTD
                << L"<CQCGW:Msg><CQCGW:DeviceList>";

    // If we got any, then stream them out
    if (bRes)
    {
        TString strTmp;
        const tCIDLib::TCard4 c4Count = colMonikers.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            strTmp = tCQCKit::strXlatEDevCats(fcolCats[c4Index]);
            strTmp.CutUpTo(kCIDLib::chUnderscore);

            m_strmReply << L"<CQCGW:DeviceDef CQCGW:Moniker='"
                        << colMonikers[c4Index] << L"' CQCGW:DevCat='"
                        << strTmp << L"'/>";
        }
    }

    // Close off the device list tag and finish it off
    m_strmReply << L"</CQCGW:DeviceList></CQCGW:Msg>"
                << kCIDLib::FlushIt;

    // And now send the reply back
    SendReply(m_strmReply.mbufData(), m_strmReply.c4CurSize());
}


//
//  This handles the 'query driver info' request. We get a driver moniker
//  and we get and return some core info.
//
tCIDLib::TVoid TWorkerThread::QueryDrvInfo(const TXMLTreeElement& xtnodeReq)
{
    // Get the moniker attr
    const TString& strMonAttr = xtnodeReq.xtattrNamed(L"CQCGW:Moniker").strValue();

    // Get a client proxy and query the info
    tCQCKit::TCQCSrvProxy orbcSrv(facCQCKit().orbcCQCSrvAdminProxy(strMonAttr));

    tCIDLib::TCard4                     c4ArchVer;
    tCQCKit::EDrvStates                 eState;
    TFundVector<tCQCKit::EDevClasses>   fcolClasses;
    TString                             strMake;
    TString                             strModel;

    orbcSrv->QueryDriverInfo
    (
        strMonAttr
        , eState
        , c4ArchVer
        , fcolClasses
        , strMake
        , strModel
        , m_cuctxClient.sectUser()
    );

    const tCIDLib::TCh* pszState;
    if (eState == tCQCKit::EDrvStates::Connected)
        pszState = L"Online";
    else
        pszState = L"Offline";

    // Send a reply back
    m_strmReply.Reset();
    m_strmReply << L"<?xml version='1.0' encoding='utf-8'?>\n"
                << kCQCGWSrv::pszDTD
                << L"<CQCGW:Msg>"
                << L"<CQCGW:DriverInfo CQCGW:Status='"
                << pszState
                << L"' CQCGW:ArchVer='" << c4ArchVer
                << L"' CQCGW:Make='" << strMake
                << L"' CQCGW:Model='" << strModel
                << L"'/>\n</CQCGW:Msg>"
                << kCIDLib::EndLn;
    SendReply(m_strmReply.mbufData(), m_strmReply.c4CurSize());
}


//
//  This handles the 'query driver status' request. We get a driver moniker
//  and we get and return it's status.
//
tCIDLib::TVoid TWorkerThread::QueryDrvStatus(const TXMLTreeElement& xtnodeReq)
{
    // Get the moniker attrs.
    const TString& strMonAttr = xtnodeReq.xtattrNamed(L"CQCGW:Moniker").strValue();

    //
    //  See if the driver is in the interface engine's list. If so, then we
    //  can do this a lot more efficiently.
    //
    tCQCKit::EDrvStates eState;
    if (!facCQCGWSrv.polleThis().bCheckDrvState(strMonAttr, eState))
    {
        // It wasn't, so let's do it manually
        tCQCKit::TCQCSrvProxy orbcSrv = facCQCKit().orbcCQCSrvAdminProxy(strMonAttr);
        tCIDLib::TCard4         c4ThreadId;
        tCQCKit::EVerboseLvls   eVerbose;
        orbcSrv->QueryDriverState(strMonAttr, eState, eVerbose, c4ThreadId);
    }

    // Send a reply back
    m_strmReply.Reset();
    m_strmReply << L"<?xml version='1.0' encoding='utf-8'?>\n"
                << kCQCGWSrv::pszDTD
                << L"<CQCGW:Msg>"
                << L"<CQCGW:DriverStatus CQCGW:Moniker='"
                << strMonAttr << L"' CQCGW:Status='";

    if (eState == tCQCKit::EDrvStates::Connected)
        m_strmReply << L"Online";
    else
        m_strmReply << L"Offline";

    m_strmReply << L"'/>"
                << L"</CQCGW:Msg>"
                << kCIDLib::EndLn;
    SendReply(m_strmReply.mbufData(), m_strmReply.c4CurSize());
}


//
//  This handles the driver text backdoor query. We get the moniker and
//  the two parms for the driver text query. We give back the status
//  returned by the driver, and the text.
//
tCIDLib::TVoid TWorkerThread::QueryDrvText(const TXMLTreeElement& xtnodeReq)
{
    // Get the attributes out we need
    const TString& strMonAttr = xtnodeReq.xtattrNamed(L"CQCGW:Moniker").strValue();
    const TString& strQType = xtnodeReq.xtattrNamed(L"CQCGW:QType").strValue();
    const TString& strDataName = xtnodeReq.xtattrNamed(L"CQCGW:DataName").strValue();

    tCQCKit::TCQCSrvProxy orbcSrv = facCQCKit().orbcCQCSrvAdminProxy(strMonAttr);
    TString strOutText;
    const tCIDLib::TBoolean bRes = orbcSrv->bQueryTextVal
    (
        strMonAttr, strQType, strDataName, strOutText
    );

    // Send a reply back
    m_strmReply.Reset();
    m_strmReply << L"<?xml version='1.0' encoding='utf-8'?>\n"
                << kCQCGWSrv::pszDTD
                << L"<CQCGW:Msg>"
                << L"<CQCGW:DriverText CQCGW:Status='";

    if (bRes)
        m_strmReply << L"True";
    else
        m_strmReply << L"False";
    m_strmReply << L"'>";

    //
    //  Put the text in, no whitespace between the open/close elements, so
    //  the caller doesn't have to deal with that. We have to encode it.
    //
    facCIDXML().EscapeFor(strOutText, m_strmReply, tCIDXML::EEscTypes::ElemText, TString::strEmpty());

    m_strmReply << L"</CQCGW:DriverText></CQCGW:Msg>" << kCIDLib::EndLn;
    SendReply(m_strmReply.mbufData(), m_strmReply.c4CurSize());
}


//
//  This handles the 'query field info' request, which just wants us to
//  send back some info on the passed field.
//
tCIDLib::TVoid TWorkerThread::QueryFldInfo(const TXMLTreeElement& xtnodeReq)
{
    // Get the moniker/field name attrs. They are in a single attribute
    const TString& strFldAttr = xtnodeReq.xtattrNamed(L"CQCGW:Field").strValue();

    // Break them out into separate strings
    TString strMon;
    TString strFld;
    facCQCKit().ParseFldName(strFldAttr, strMon, strFld);

    //
    //  First see if it's in our polling engine. If so, we can get it from
    //  there cheaply. Else we have to go get it ourself.
    //
    TCQCFldDef flddInfo;
    if (!facCQCGWSrv.polleThis().bQueryFldInfo(strMon, strFld, flddInfo))
    {
        // Not in the engine, so get get it
        tCQCKit::TCQCSrvProxy orbcSrv = facCQCKit().orbcCQCSrvAdminProxy(strMon);
        orbcSrv->QueryFieldDef(strMon, strFld, flddInfo);
    }

    //
    //  Build up the query message and send it. Note that the field access
    //  value we can't just format out, since it uses loadable text and will
    //  get translated, whereas the type stuff is language neutral. So we
    //  have to map it ourself here, to match the values in the DTD.
    //
    m_strmReply.Reset();
    m_strmReply << L"<?xml version='1.0' encoding='utf-8'?>\n"
                << kCQCGWSrv::pszDTD
                << L"<CQCGW:Msg>"
                << L"<CQCGW:FldInfo CQCGW:Type='"
                << tCQCKit::strXlatEFldTypes(flddInfo.eType())
                << L"' CQCGW:Access='"
                << pszMapFldAccess(flddInfo.eAccess())
                << L"'";

    //
    //  If there are limits, send those. Be sure to escape, since it could have
    //  quotes in it potentially.
    //
    if (!flddInfo.strLimits().bIsEmpty())
    {
        m_strmReply << L" CQCGW:Limits='";
        facCIDXML().EscapeFor
        (
            flddInfo.strLimits(), m_strmReply, tCIDXML::EEscTypes::Attribute, TString::strEmpty()
        );
        m_strmReply << L"'";
    }

    // If the semantic type isn't generic, send that
    if (flddInfo.eSemType() != tCQCKit::EFldSTypes::Generic)
    {
        strMon = tCQCKit::strXlatEFldSTypes(flddInfo.eSemType());
        strMon.CutUpTo(L'_');
        m_strmReply << L" CQCGW:SemType='" << strMon << L"'";
    }

    m_strmReply << L"/></CQCGW:Msg>" << kCIDLib::EndLn;
    SendReply(m_strmReply.mbufData(), m_strmReply.c4CurSize());
}


//
//  This handles the 'query field info list' request, which asks us for a
//  list of field info sets for fields in the poll list.
//
tCIDLib::TVoid
TWorkerThread::QueryFldInfoList(const TXMLTreeElement& xtnodeReq)
{
    // Start the message to the open device list tag
    m_strmReply.Reset();
    m_strmReply << L"<?xml version='1.0' encoding='UTF-8'?>\n"
                << kCQCGWSrv::pszDTD
                << L"<CQCGW:Msg><CQCGW:FldInfoList>";

    //
    //  Each child should be a FldName element, which indicates the name of
    //  a field to get the info for. We'll get info from the polling engine
    //  if possible, since it's often going to be there. If not, we'll use
    //  a field cache object, but only initialize it if we have to.
    //
    tCIDLib::TBoolean   bInitCache = kCIDLib::False;
    TCQCFldCache        cfcToUse;
    TString             strMon;
    TString             strFld;
    TCQCFldDef          flddInfo;
    const tCIDLib::TCard4 c4ParmCount = xtnodeReq.c4ChildCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ParmCount; c4Index++)
    {
        const TXMLTreeElement& xtnodeFld = xtnodeReq.xtnodeChildAtAsElement(c4Index);

        // Break out the field name into separate parts
        facCQCKit().ParseFldName
        (
            xtnodeFld.xtattrNamed(L"CQCGW:Name").strValue(), strMon, strFld
        );

        if (!facCQCGWSrv.polleThis().bQueryFldInfo(strMon, strFld, flddInfo))
        {
            // Not in the polling engine, so use the cache. Init if needed
            if (!bInitCache)
                cfcToUse.Initialize(new TCQCFldFilter(tCQCKit::EReqAccess::ReadOrWrite));

            // See if it's in the cache
            if (!cfcToUse.bFldExists(strMon, strFld, flddInfo))
            {
                // Not a field we know
                TString strErr
                (
                    kCQCGWSErrs::errcProto_UnknownFld, facCQCGWSrv, strMon, strFld
                );
                SendNakReply(strErr);
                return;
            }
        }

        // We found it, so format out an info node
        m_strmReply << L"    <CQCGW:FldInfo CQCGW:Type='"
                    << tCQCKit::strXlatEFldTypes(flddInfo.eType())
                    << L"' CQCGW:Access='"
                    << pszMapFldAccess(flddInfo.eAccess())
                    << L"'";

        // If there is a limit value, add it
        if (!flddInfo.strLimits().bIsEmpty())
            m_strmReply << L" CQCGW:Limits='" << flddInfo.strLimits() << L"'";

        // If the semantic type isn't generic, send that
        if (flddInfo.eSemType() != tCQCKit::EFldSTypes::Generic)
        {
            strMon = tCQCKit::strXlatEFldSTypes(flddInfo.eSemType());
            strMon.CutUpTo(L'_');
            m_strmReply << L" CQCGW:SemType='" << strMon << L"'";
        }

        m_strmReply << L"/>";
    }

    // Close off the main and finish it off
    m_strmReply << L"</CQCGW:FldInfoList></CQCGW:Msg>"
                << kCIDLib::FlushIt;

    // And now send the reply back
    SendReply(m_strmReply.mbufData(), m_strmReply.c4CurSize());
}


tCIDLib::TVoid TWorkerThread::QueryFields()
{
    //
    //  We can use a field data cache object to do the grunt work for us. It
    //  already provides the grunt work to find all of the devices and get
    //  their field information.
    //
    TCQCFldCache cfcData;
    try
    {
        // We don't need any filters here
        cfcData.Initialize(0);
    }

    catch(const TError& errToCatch)
    {
        if (facCQCGWSrv.bShouldLog(errToCatch))
            TModule::LogEventObj(errToCatch);

        // Send an exception reply back
        SendExceptionReply(errToCatch);
        return;
    }

    // Start the message to the open device list tag
    m_strmReply.Reset();
    m_strmReply << L"<?xml version='1.0' encoding='UTF-8'?>\n"
                << kCQCGWSrv::pszDTD
                << L"<CQCGW:Msg><CQCGW:DeviceList>";

    // Get the list of drivers which we can then iterate through
    const tCIDLib::TCard4 c4DevCount = cfcData.c4DevCount();
    tCIDLib::TStrList colDrvs(c4DevCount);
    cfcData.QueryDevList(colDrvs);

    TString strTmp;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4DevCount; c4Index++)
    {
        const TString& strMoniker = colDrvs[c4Index];
        const tCQCKit::EDevCats eDevCat = cfcData.eCategory(strMoniker);

        strTmp = tCQCKit::strXlatEDevCats(eDevCat);
        strTmp.CutUpTo(kCIDLib::chUnderscore);

        m_strmReply << L"<CQCGW:DeviceDef CQCGW:Moniker='"
                    << strMoniker << L"' CQCGW:DevCat='"
                    << strTmp << L"'>";

        // Get the field list for this device
        const tCQCKit::TFldDefList& colFldList
        (
            cfcData.colFieldListFor(strMoniker)
        );

        // And now let's iterate the fields for this device
        const tCIDLib::TCard4 c4FldCount = colFldList.c4ElemCount();
        for (tCIDLib::TCard4 c4FldIndex = 0; c4FldIndex < c4FldCount; c4FldIndex++)
        {
            const TCQCFldDef& flddCur = colFldList[c4FldIndex];

            m_strmReply << L"<CQCGW:FieldDef "
                        << L"CQCGW:Name='"
                        << flddCur.strName()
                        << L"' CQCGW:Type='"
                        << tCQCKit::strXlatEFldTypes(flddCur.eType())
                        << L"' CQCGW:AccessType='"
                        << pszMapFldAccess(flddCur.eAccess())
                        << L"'";

            // If there is a limit value, add it
            if (!flddCur.strLimits().bIsEmpty())
                m_strmReply << L" CQCGW:Limits='" << flddCur.strLimits() << L"'";

            // If the semantic type isn't generic, send that
            if (flddCur.eSemType() != tCQCKit::EFldSTypes::Generic)
            {
                strTmp = tCQCKit::strXlatEFldSTypes(flddCur.eSemType());
                strTmp.CutUpTo(L'_');
                m_strmReply << L" CQCGW:SemType='" << strTmp << L"'";
            }

            // Close out the field elemnt
            m_strmReply << L"/>";
        }

        // And close off this device element
        m_strmReply << L"</CQCGW:DeviceDef>";
    }

    // Close off the device list tag and finish it off
    m_strmReply << L"</CQCGW:DeviceList></CQCGW:Msg>" << kCIDLib::FlushIt;

    // And now send the reply back
    SendReply(m_strmReply.mbufData(), m_strmReply.c4CurSize());
}


//
//  This handles the global action query request. It just formats out
//  he list of available global actions, queried from the data server,
//  into the message. The data server provides a method to query a formatted
//  XML fragment that indicates the actions in their hierarchical arrangement.
//
tCIDLib::TVoid TWorkerThread::QueryGlobalActs()
{
    //
    //  Get data server client proxy and ask it to format a hierarchical XML
    //  formatted tree of the available macros. He provides a special API basially
    //  for us, so that we can get this information with minimum hassle. Indicate
    //  that this is a 'sparse' type query, i.e. we may never actually access any
    //  of the actions, we just want to get the heirarchical ifno.
    //
    TDataSrvClient dsclLoad;
    TString strTreeText;

    dsclLoad.QueryTree
    (
        L"/", tCQCRemBrws::EDTypes::GlobalAct, strTreeText, kCIDLib::True, m_cuctxClient.sectUser()
    );

    //
    //  Build up the message, putting all of the macros we got into the body
    //  of the reply element.
    //
    m_strmReply.Reset();
    m_strmReply << L"<?xml version='1.0' encoding='UTF-8'?>\n"
                << kCQCGWSrv::pszDTD
                << L"<CQCGW:Msg>";

    // Output the returned list as the body of the message
    m_strmReply << strTreeText;

    // Cap it off, flush the stream,a nd send the message
    m_strmReply << L"</CQCGW:Msg>" << kCIDLib::FlushIt;

    SendReply(m_strmReply.mbufData(), m_strmReply.c4CurSize());
}



//
//  This handles the image query request. We look up the image that they
//  requested, which gets us a PNG image and some ancillary data. We encode
//  the image using Base64 and make it the element text of an element, and
//  provide the ancillary data via attributes.
//
tCIDLib::TVoid TWorkerThread::QueryImage(const TXMLTreeElement& xtnodeReq)
{
    // Get a data server client
    TDataSrvClient dsclLoad;

    // Get the name of the image and the serial number
    TString strName = xtnodeReq.xtattrNamed(L"CQCGW:ImagePath").strValue();
    const tCIDLib::TCard4 c4InSerial = xtnodeReq.xtattrNamed(L"CQCGW:SerialNum").c4ValueAs();

    // We need to flip the slashes in case they are using old style back slashes
    strName.bReplaceChar(kCIDLib::chBackSlash, kCIDLib::chForwardSlash);

    // See if it exists, if not, return an error
    if (!dsclLoad.bFileExists(strName, tCQCRemBrws::EDTypes::Image))
    {
        TString strErr(kCQCGWSErrs::errcProto_ImgNotFound, facCQCGWSrv, strName);
        SendNakReply(strErr);
        return;
    }

    tCIDLib::TCard4         c4Bytes;
    tCIDLib::TCard4         c4SerialNum = c4InSerial;
    tCIDLib::TEncodedTime   enctLastChange;
    THeapBuf                mbufImg;
    tCIDLib::TKVPFList      colMeta;
    const tCIDLib::TBoolean bNewData = dsclLoad.bReadImage
    (
        strName
        , c4SerialNum
        , enctLastChange
        , mbufImg
        , c4Bytes
        , colMeta
        , m_cuctxClient.sectUser()
    );

    // Stream out the header stuff
    m_strmReply.Reset();
    m_strmReply << L"<?xml version='1.0' encoding='UTF-8'?>\n"
                << kCQCGWSrv::pszDTD
                << L"<CQCGW:Msg>"
                << L"<CQCGW:ImageData CQCGW:SerialNum='";

    // If no new data, then leave blank, else format out the serial number
    if (bNewData)
        m_strmReply << L"0x" << TCardinal(c4SerialNum, tCIDLib::ERadices::Hex);
    m_strmReply << L"'";

    // If we got new data, then base64 encode it into the reply stream
    if (bNewData)
    {
        // Query some image info first
        tCIDLib::TBoolean       bTrans;
        tCIDImage::EPixFmts     eImgFmt;
        tCIDImage::EBitDepths   eBitDepth;
        tCIDPNG::EInterlaces    eInterlace;
        TRGBClr                 rgbTransClr;
        TSize                   szImage;

        {
            // Create an input stream over the buffer and stream in the image info
            TBinMBufInStream strmSrc(&mbufImg, c4Bytes);
            TPNGImage::QueryImgInfo
            (
                strmSrc
                , c4Bytes
                , szImage
                , eImgFmt
                , eBitDepth
                , eInterlace
                , bTrans
                , rgbTransClr
                , kCIDLib::False
            );
        }

        //
        //  If the 'no alpha' flag is set, then we need to see if this
        //  image has alpha channel. If so, then we have to convert it to
        //  a color based tranparency image.
        //
        if (tCIDLib::bAllBitsOn(m_eOptFlags, tCQCGWSrv::EOptFlags::NoAlpha)
        &&  tCIDLib::bAllBitsOn(eImgFmt, tCIDImage::EPixFmts::Alpha))
        {
            //
            //  We'll have to parse the image, so create an input stream and
            //  stream in the whole image.
            //
            TPNGImage imgTmp;
            {
                TBinMBufInStream strmSrc(&mbufImg, c4Bytes);
                strmSrc >> imgTmp;
            }

            //
            //  Create a new temp pixel array and call a method that will
            //  give us back a color based transparency version of the image
            //  along with a transparency color it selected.
            //
            TPixelArray pixaTmp;
            tCIDLib::TCard4 c4TransClr;
            imgTmp.bLoseAlpha(pixaTmp, c4TransClr);

            //
            //  Ok, create a new image object and set our private trans
            //  color value on it.
            //
            TPNGImage imgNew(pixaTmp);
            imgNew.c4TransClr(c4TransClr);
            imgNew.bTransClr(kCIDLib::True);

            //
            //  We need to give back an RGB transparency color, so we need
            //  to convert our format sensitive version to RGB.
            //
            TCIDImage::CvtTransClr
            (
                pixaTmp.ePixFmt()
                , pixaTmp.eBitDepth()
                , c4TransClr
                , imgNew.palClrs()
                , rgbTransClr
            );

            // And indicate there is transparency info now
            bTrans = kCIDLib::True;

            // Update the format and depth flags for the new image
            eImgFmt = imgNew.ePixFmt();
            eBitDepth = imgNew.eBitDepth();

            //
            //  And stream it back into the original buffer. It shouldn't have
            //  to expand, since the new version should be considerably
            //  smaller than the old.
            //
            {
                TBinMBufOutStream strmTar(&mbufImg);
                strmTar << imgNew << kCIDLib::FlushIt;

                // !! Update the byte count since we have new data
                c4Bytes = strmTar.c4CurSize();
            }
        }

        //
        //  If we are to send image info, which is one of the optional flags,
        //  then pull that info out and format it into attributes.
        //
        if (tCIDLib::bAllBitsOn(m_eOptFlags, tCQCGWSrv::EOptFlags::SendImgInfo))
        {
            m_strmReply << L" CQCGW:Flags='"
                        << TCardinal(tCIDLib::c4EnumOrd(eImgFmt), tCIDLib::ERadices::Hex)
                        << L"' CQCGW:Depth='"
                        << TCardinal(tCIDLib::c4EnumOrd(eBitDepth)) << L"'";
            if (bTrans)
                m_strmReply << L" CQCGW:TransClr='" << rgbTransClr << L"'";
        }

        // Ok, we can close out the start element now
        m_strmReply << L">";

        // And now Base64 encode the image data from the image stream
        TBinMBufInStream strmSrc(&mbufImg, c4Bytes);
        TBase64 b64Encode;
        b64Encode.c4LineWidth(512);
        b64Encode.Encode(strmSrc, m_strmReply);

        // And close this element off now
        m_strmReply << L"</CQCGW:ImageData>";
    }
     else
    {
        // Just close out the start element, which is all we need in this case
        m_strmReply << L"/>";
    }

    //
    //  And now close off the elements and send the reply. For this one,
    //  explicitly indicate that we don't want encryption. It's not worth
    //  the overhead for images, and who cares if anyone snooping on the
    //  network can see them.
    //
    m_strmReply   << L"</CQCGW:Msg>" << kCIDLib::FlushIt;
    SendReply(m_strmReply.mbufData(), m_strmReply.c4CurSize(), kCIDLib::False);
}


//
//  This handles the macro query request. It just formats out the list of
//  available macros, queried from the data server, into the message. The
//  data server provides a method to query a formatted XML fragment that
//  indicates the macros in their hierarchical arrangement.
//
tCIDLib::TVoid TWorkerThread::QueryMacros()
{
    //
    //  Get a data server client and ask it to format a hierarchical XML
    //  formatted tree of the available macros. He provides a special API basially
    //  for us, so that we can get this information with minimum hassle.
    //
    TDataSrvClient dsclQuery;
    TString strTreeText;
    dsclQuery.QueryTree
    (
        L"/", tCQCRemBrws::EDTypes::Macro, strTreeText, kCIDLib::True, m_cuctxClient.sectUser()
    );

    //
    //  Build up the message, putting all of the macros we got into the body
    //  of the reply element.
    //
    m_strmReply.Reset();
    m_strmReply << L"<?xml version='1.0' encoding='UTF-8'?>\n"
                << kCQCGWSrv::pszDTD
                << L"<CQCGW:Msg>";

    // Output the returned list as the body of the message
    m_strmReply << strTreeText;

    // Cap it off, flush the stream, and send the message
    m_strmReply << L"</CQCGW:Msg>" << kCIDLib::FlushIt;

    SendReply(m_strmReply.mbufData(), m_strmReply.c4CurSize());
}


//
//  This method returns a list of driver monikers for all of the media repository
//  drivers currently loaded in the system.
//
tCIDLib::TVoid TWorkerThread::QueryMediaRepoDrvs()
{
    // There's a helper method in the CQCKit facility to do this for us
    tCIDLib::TStrList colDrvs;
    facCQCKit().bFindRepoDrivers(colDrvs, TString::strEmpty());

    //
    //  Start the message with the open driver list element. It includes the
    //  count we are going to return.
    //
    m_strmReply.Reset();
    m_strmReply << L"<?xml version='1.0' encoding='UTF-8'?>\n"
                << kCQCGWSrv::pszDTD
                << L"<CQCGW:Msg><CQCGW:DrvList CQCGW:Count='"
                << colDrvs.c4ElemCount() << L"'>";

    // For each one found, put out a DrvMon element
    const tCIDLib::TCard4 c4Count = colDrvs.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        m_strmReply << L"<CQCGW:DrvMon CQCGW:Moniker='" << colDrvs[c4Index] << L"'/>";
    }

    // Cap it off, flush the stream, and send the message
    m_strmReply << L"</CQCGW:DrvList></CQCGW:Msg>" << kCIDLib::FlushIt;
    SendReply(m_strmReply.mbufData(), m_strmReply.c4CurSize());
}



//
//  Returns field info about all of the fields currently in the poll list,
//  in the same format that the 'query field info list' query does.
//
tCIDLib::TVoid TWorkerThread::QueryPollList()
{
    // Start the message to the open device list tag
    m_strmReply.Reset();
    m_strmReply << L"<?xml version='1.0' encoding='UTF-8'?>\n"
                << kCQCGWSrv::pszDTD
                << L"<CQCGW:Msg><CQCGW:FldInfoList>";


    const tCIDLib::TCard4 c4Count = m_colFields.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TCQCFldPollInfo& cfpiCur = m_colFields[c4Index];
        const TCQCFldDef& flddInfo = cfpiCur.flddAssoc();

        m_strmReply << L"<CQCGW:FldInfo CQCGW:Type='"
                    << tCQCKit::strXlatEFldTypes(flddInfo.eType())
                    << L"' CQCGW:Access='"
                    << tCQCKit::strXlatEFldAccess(flddInfo.eAccess())
                    << L"' CQCGW:Limits='"
                    << flddInfo.strLimits()
                    << L"'/>";
    }

    // Close off the main and finish it off
    m_strmReply << L"</CQCGW:FldInfoList></CQCGW:Msg>" << kCIDLib::FlushIt;

    // And now send the reply back
    SendReply(m_strmReply.mbufData(), m_strmReply.c4CurSize());
}


//
//  Return the system configuration info for the room indicated.
//
tCIDLib::TVoid TWorkerThread::QueryRoomCfg(const TXMLTreeElement& xtnodeReq)
{
    // Get the name of the room they want
    const TString& strName = xtnodeReq.xtattrNamed(L"CQCGW:Name").strValue();

    // Query the system configuration info
    TCQCSysCfgClientProxy* porbcProxy = facCIDOrbUC().porbcMakeClient<TCQCSysCfgClientProxy>
    (
        TCQCSysCfgClientProxy::strBinding, 3000
    );
    TCntPtr<TCQCSysCfgClientProxy> orbcProxy(porbcProxy);
    orbcProxy->bQueryRoomCfg(m_c4SysCfgSerNum, m_scfgCur);

    tCIDLib::TCard4 c4RmInd;
    const TCQCSysCfgRmInfo& scriRm = m_scfgCur.scriRoomByName(strName, c4RmInd);

    // Start the message up to the open tag
    m_strmReply.Reset();
    m_strmReply << L"<?xml version='1.0' encoding='UTF-8'?>\n"
                << kCQCGWSrv::pszDTD
                << L"<CQCGW:Msg><CQCGW:RoomCfgRep>\n";

    // Let the room format itself out as the contents, appending to the stream contents
    scriRm.ToXML(m_scfgCur, m_strmReply, kCIDLib::True);

    #if CID_DEBUG_ON
    TTextFileOutStream strmDump
    (
        L".\\Dump.txt"
        , tCIDLib::ECreateActs::CreateAlways
        , tCIDLib::EFilePerms::Default
        , tCIDLib::EFileFlags::SequentialScan
        , tCIDLib::EAccessModes::Excl_Write
    );
    scriRm.ToXML(m_scfgCur, strmDump, kCIDLib::False);
    #endif

    // Close off the main and finish it off
    m_strmReply << L"</CQCGW:RoomCfgRep></CQCGW:Msg>"
                << kCIDLib::FlushIt;

    // And now send the reply back
    SendReply(m_strmReply.mbufData(), m_strmReply.c4CurSize());
}


tCIDLib::TVoid TWorkerThread::QueryRoomCfgList()
{
    // Query the latest system configuration info
    // Query the system configuration info
    TCQCSysCfgClientProxy* porbcProxy = facCIDOrbUC().porbcMakeClient<TCQCSysCfgClientProxy>
    (
        TCQCSysCfgClientProxy::strBinding, 3000
    );
    TCntPtr<TCQCSysCfgClientProxy> orbcProxy(porbcProxy);
    orbcProxy->bQueryRoomCfg(m_c4SysCfgSerNum, m_scfgCur);

    // Start the message up to the open room list tag
    m_strmReply.Reset();
    m_strmReply << L"<?xml version='1.0' encoding='UTF-8'?>\n"
                << kCQCGWSrv::pszDTD
                << L"<CQCGW:Msg><CQCGW:RoomList>";

    // Get the list of rooms and format them all out
    const tCIDLib::TCard4 c4Count = m_scfgCur.c4RoomCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TCQCSysCfgRmInfo& scriCur = m_scfgCur.scriRoomAt(c4Index);

        m_strmReply << L"<CQCGW:RoomInfo CQCGW:Name='"
                    << scriCur.strName() << L"'/>";
    }

    // Close off the main and finish it off
    m_strmReply << L"</CQCGW:RoomList></CQCGW:Msg>"
                << kCIDLib::FlushIt;

    // And now send the reply back
    SendReply(m_strmReply.mbufData(), m_strmReply.c4CurSize());
}



//
//  We have to get a scheduled event client proxy and query the scheduled
//  event with the passed event id.
//
tCIDLib::TVoid TWorkerThread::QuerySchEv(const TXMLTreeElement& xtnodeReq)
{
    TCQCSchEvent csrcRet;
    TString strToGet;
    {
        //
        //  We need to see if this is a path or an old style id. If it's an id, we
        //  create a path for it based on the old id.
        //
        const TString& strId = xtnodeReq.xtattrNamed(L"CQCGW:EvId").strValue();
        if (strId.bStartsWithI(L"/User/"))
        {
            strToGet = strId;
        }
         else
        {
            strToGet = L"/User/";
            strToGet.Append(strId);
        }

        TDataSrvClient          dsclQuery;
        tCIDLib::TCard4         c4SerNum = 0;
        tCIDLib::TEncodedTime   enctLastChange;
        tCIDLib::TKVPFList      colMeta;
        dsclQuery.bReadSchedEvent
        (
            strToGet
            , c4SerNum
            , enctLastChange
            , csrcRet
            , colMeta
            , m_cuctxClient.sectUser()
        );
    }

    //
    //  Build up the reply now, depending on the type of event. We use the
    //  already existing helper that will do the grunt work of formatting
    //  out the right type of reply message for the requested event.
    //
    m_strmReply.Reset();
    m_strmReply << L"<?xml version='1.0' encoding='utf-8'?>\n"
                << kCQCGWSrv::pszDTD
                << L"<CQCGW:Msg>";

    FormatSchEv(strToGet, csrcRet, m_strmReply);

    // Close off the message and send it
    m_strmReply << L"</CQCGW:Msg>" << kCIDLib::FlushIt;
    SendReply(m_strmReply.mbufData(), m_strmReply.c4CurSize());
}


//
//  Lets the client do ad hoc reads of single fields by mon/name. We return
//  the value as a text value.
//
tCIDLib::TVoid TWorkerThread::ReadField(const TXMLTreeElement& xtnodeReq)
{
    // Get the moniker/field name attrs. They are in a single attribute
    const TString& strFldAttr = xtnodeReq.xtattrNamed(L"CQCGW:Field").strValue();

    // Break them out into separate strings
    TString strMon;
    TString strFld;
    facCQCKit().ParseFldName(strFldAttr, strMon, strFld);

    //
    //  If this field is in our polling engine, then we can do it a lot
    //  more efficiently by just getting the value from there. Else, we have
    //  to go get it ourselves.
    //
    tCIDLib::TBoolean bOk;
    TString strVal;
    tCQCKit::EFldTypes eType;
    bOk = facCQCGWSrv.polleThis().bReadValue(strMon, strFld, strVal, eType);
    if (!bOk)
    {
        // It wasn't, so let's do it manually
        tCQCKit::TCQCSrvProxy orbcSrv = facCQCKit().orbcCQCSrvAdminProxy(strMon);
        tCIDLib::TCard4 c4SerialNum = 0;
        try
        {
            bOk = orbcSrv->bReadFieldByName(c4SerialNum, strMon, strFld, strVal, eType);
        }

        catch(...)
        {
            bOk = kCIDLib::False;
        }
    }

    // Build up the reply now
    m_strmReply.Reset();
    m_strmReply << L"<?xml version='1.0' encoding='utf-8'?>\n"
                << kCQCGWSrv::pszDTD
                << L"<CQCGW:Msg>"
                << L"<CQCGW:FldValue CQCGW:Field='" << strMon
                << L"." << strFld << L"' CQCGW:Status='";

    // Put out the status value
    if (bOk)
        m_strmReply << L"CQCGW:Online";
    else
        m_strmReply << L"CQCGW:InError";

    //
    //  And now put otu th evalue and wrap it up. We have to escape the value
    //  for XML attribute restrictions.
    //
    m_strmReply << L"' CQCGW:Value='";
    facCIDXML().EscapeFor(strVal, m_strmReply, tCIDXML::EEscTypes::Attribute, TString::strEmpty());
    m_strmReply << L"'/></CQCGW:Msg>" << kCIDLib::EndLn;
    SendReply(m_strmReply.mbufData(), m_strmReply.c4CurSize());
}


