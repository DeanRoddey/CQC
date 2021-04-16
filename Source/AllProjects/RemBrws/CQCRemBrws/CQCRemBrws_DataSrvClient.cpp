//
// FILE NAME: CQCRemBrws_DataSrvClient.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/24/2015
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
//  This is the implementation file for the data server client class.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCRemBrws_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TDataSrvClient,TObject)


// Enable or disable some verbose logging to help diagnose issues
#if CID_DEBUG_ON
//#define LOG_FILEACC 1
#endif


// ---------------------------------------------------------------------------
//   CLASS: TDataSrvClient
//  PREFIX: dscl
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TDataSrvClient: Public, static methods
// ---------------------------------------------------------------------------

//
//  Convenience methods to find a specific value in a list of meta values returned
//  by the data server. The source is for the thrown exception if the key isn't found
//  in the one that just returns the string. We also have one that just returns a
//  boolean to indicate if it was found or not.
//
tCIDLib::TBoolean
TDataSrvClient::bFindMetaVal(const  TString&            strKeyToFind
                            , const tCIDLib::TKVPFList& colList
                            ,       TString&            strToFill)
{
    const tCIDLib::TCard4 c4Count = colList.c4ElemCount();
    tCIDLib::TCard4 c4Index = 0;
    while (c4Index < c4Count)
    {
        if (strKeyToFind.bCompareI(colList[c4Index].strKey()))
            break;
        c4Index++;
    }

    if (c4Index == c4Count)
        return kCIDLib::False;

    strToFill = colList[c4Index].strValue();
    return kCIDLib::True;;
}


const TString&
TDataSrvClient::strFindMetaVal( const   TString&            strSrc
                                , const TString&            strKeyToFind
                                , const tCIDLib::TKVPFList& colList)
{
    const tCIDLib::TCard4 c4Count = colList.c4ElemCount();
    tCIDLib::TCard4 c4Index = 0;
    while (c4Index < c4Count)
    {
        if (strKeyToFind.bCompareI(colList[c4Index].strKey()))
            break;
        c4Index++;
    }

    if (c4Index == c4Count)
    {
        facCQCRemBrws().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kRBrwsErrs::errcRem_MetaKeyNotFound
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , strKeyToFind
            , strSrc
        );
    }
    return colList[c4Index].strValue();
}


// ---------------------------------------------------------------------------
//  TDataSrvClient: Constructors and Destructor
// ---------------------------------------------------------------------------
TDataSrvClient::TDataSrvClient(const tCIDLib::TBoolean bQuickTest) :

    m_porbcDS(nullptr)
{
    // Try to create the proxy object
    CheckReady(bQuickTest);
}

TDataSrvClient::~TDataSrvClient()
{
    // Clean up the client proxy if we created it
    try
    {
        delete m_porbcDS;
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        facCQCRemBrws().LogEventObj(errToCatch);
    }
}


// ---------------------------------------------------------------------------
//  TDataSrvClient: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Check to see if a specific file exists
tCIDLib::TBoolean
TDataSrvClient::bFileExists(const   TString&                strToFind
                            , const tCQCRemBrws::EDTypes    eType)
{
    CheckReady();

    // Convert the path to a full hierachical path
    TString strHPath;
    facCQCRemBrws().ToHPath(strToFind, eType, strHPath);

    CheckIsHPath(strHPath);
    return m_porbcDS->bExists(strHPath, kCIDLib::False);
}

tCIDLib::TBoolean
TDataSrvClient::bFileExists(const   TString&                strRelScope
                            , const TString&                strToFind
                            , const tCQCRemBrws::EDTypes    eType)
{
    CheckReady();

    // Convert the path to a full hierachical path
    TString strHPath;
    facCQCRemBrws().ToHPath(strRelScope, strToFind, eType, strHPath);

    CheckIsHPath(strHPath);
    return m_porbcDS->bExists(strHPath, kCIDLib::False);
}



// Some methods for getting file system info for files or scopes
tCIDLib::TBoolean
TDataSrvClient::bFindFile(  const   TString&                strRelToFind
                            , const tCQCRemBrws::EDTypes    eType
                            ,       TFindBuf&               fndbToFill)
{
    CheckReady();

    // Convert the path to a full hierachical path
    TString strHPath;
    facCQCRemBrws().ToHPath(strRelToFind, eType, strHPath);

    CheckIsHPath(strHPath);
    return m_porbcDS->bFind(strHPath, fndbToFill, kCIDLib::False);
}

tCIDLib::TBoolean
TDataSrvClient::bFindFile(  const   TString&                strRelScope
                            , const TString&                strToFind
                            , const tCQCRemBrws::EDTypes    eType
                            ,       TFindBuf&               fndbToFill)
{
    CheckReady();

    // Convert the path to a full hierachical path
    TString strHPath;
    facCQCRemBrws().ToHPath(strRelScope, strToFind, eType, strHPath);

    CheckIsHPath(strHPath);
    return m_porbcDS->bFind(strHPath, fndbToFill, kCIDLib::False);
}


// Some as above, but for scopes in this case
tCIDLib::TBoolean
TDataSrvClient::bFindScope( const   TString&                strRelToFind
                            , const tCQCRemBrws::EDTypes    eType
                            ,       TFindBuf&               fndbToFill)
{
    CheckReady();

    // Convert the path to a full hierachical path
    TString strHPath;
    facCQCRemBrws().ToHPath(strRelToFind, eType, strHPath);

    CheckIsHPath(strHPath);
    return m_porbcDS->bFind(strHPath, fndbToFill, kCIDLib::True);
}

tCIDLib::TBoolean
TDataSrvClient::bFindScope( const   TString&                strRelScope
                            , const TString&                strToFind
                            , const tCQCRemBrws::EDTypes    eType
                            ,       TFindBuf&               fndbToFill)
{
    CheckReady();

    // Convert the path to a full hierachical path
    TString strHPath;
    facCQCRemBrws().ToHPath(strRelScope, strToFind, eType, strHPath);

    CheckIsHPath(strHPath);
    return m_porbcDS->bFind(strHPath, fndbToFill, kCIDLib::True);
}


// Return whether we are currently connected
tCIDLib::TBoolean TDataSrvClient::bIsConnected() const
{
    if (!m_porbcDS)
        return kCIDLib::False;

    return !m_porbcDS->bCheckForLostConnection();
}


// Return whether the indicated path represents a file or scope
tCIDLib::TBoolean
TDataSrvClient::bIsScope(const TString& strRelToCheck, const tCQCRemBrws::EDTypes eType)
{
    CheckReady();

    // Convert the path to a full hierachical path
    TString strHPath;
    facCQCRemBrws().ToHPath(strRelToCheck, eType, strHPath);

    CheckIsHPath(strHPath);
    return m_porbcDS->bIsScope(strHPath);
}


//
//  A type specific helper method, to get some needed info about an image without
//  having to download the whole thing.
//
tCIDLib::TBoolean
TDataSrvClient::bQueryImageInfo(const   TString&                strRelPath
                                ,       tCIDLib::TCard4&        c4ImageSz
                                ,       tCIDLib::TCard4&        c4ThumbSz
                                ,       tCIDImage::EPixFmts&    eImgFmt
                                ,       tCIDImage::EBitDepths&  eImgDepth
                                ,       TSize&                  szImg
                                ,       TSize&                  szThumb
                                ,       tCIDLib::TCard4&        c4SerialNum
                                ,       tCIDLib::TEncodedTime&  enctLastChange
                                , const tCIDLib::TBoolean       bThrowIfNot
                                , const TCQCSecToken&           sectUser)
{
    c4ImageSz = 0;
    c4ThumbSz = 0;

    // Make sure we have been correctly initialized
    CheckReady();

    // Convert the path to a full hierachical path and validate it
    TString strHPath;
    facCQCRemBrws().ToHPath(strRelPath, tCQCRemBrws::EDTypes::Image, strHPath);
    CheckIsHPath(strHPath);

    // Query the chunk sizes and meta info for this item
    tCIDLib::TStrList colIds;
    tCIDLib::TCardList fcolSizes;
    tCIDLib::TKVPFList colMeta;
    tCIDLib::TBoolean bRes = m_porbcDS->bQueryItemMeta
    (
        strHPath
        , colMeta
        , colIds
        , fcolSizes
        , c4SerialNum
        , enctLastChange
        , bThrowIfNot
        , sectUser
    );

    if (!bRes)
        return kCIDLib::False;

    //
    //  We should have three chunks (meta, data, thumb extension) and four meta values.
    //
    CIDAssert(colIds.c4ElemCount() == 3, L"Meta query for image did not return 3 sizes");
    CIDAssert(colMeta.c4ElemCount() == 4, L"Meta query for image did not return 4 meta values");
    CIDAssert
    (
        colIds[2] == kCQCRemBrws::strChunkId_Thumbnail
        ,L"Image chunks query did not have correct thumbnail chunk id"
    );
    c4ImageSz = fcolSizes[1];
    c4ThumbSz = fcolSizes[2];

    //
    //  And we can give back other info that are stored in metavalues for all repo
    //  images.
    //
    try
    {
        const TString& strDepth = strFindMetaVal(strRelPath, kCQCRemBrws::strMetaKey_ImgDepth, colMeta);
        const TString& strFmt = strFindMetaVal(strRelPath, kCQCRemBrws::strMetaKey_ImgFormat, colMeta);
        const TString& strSzImg = strFindMetaVal(strRelPath, kCQCRemBrws::strMetaKey_ImgSize, colMeta);
        const TString& strSzThumb = strFindMetaVal(strRelPath, kCQCRemBrws::strMetaKey_ThumbSize, colMeta);

        // These two are just the actual enum values formatted as numbers
        eImgDepth = tCIDImage::EBitDepths(strDepth.c4Val());
        eImgFmt = tCIDImage::EPixFmts(strFmt.c4Val());

        szImg.ParseFromText(strSzImg);
        szThumb.ParseFromText(strSzThumb);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
    return kCIDLib::True;
}


// Get general info on any file
tCIDLib::TBoolean
TDataSrvClient::bQueryItemMetaData( const   TString&                strRelPath
                                    , const tCQCRemBrws::EDTypes    eType
                                    ,       tCIDLib::TKVPFList&     colMetaVals
                                    ,       tCIDLib::TStrList&      colChunkIds
                                    ,       tCIDLib::TCardList&     fcolChunkSizes
                                    ,       tCIDLib::TCard4&        c4SerialNum
                                    ,       tCIDLib::TEncodedTime&  enctLastChange
                                    , const tCIDLib::TBoolean       bThrowIfNot
                                    , const TCQCSecToken&           sectUser)
{
    // Conver the type relative path to a hierarchical path
    TString strHPath;
    facCQCRemBrws().ToHPath(strRelPath, eType, strHPath);
    CheckIsHPath(strHPath);

    return m_porbcDS->bQueryItemMeta
    (
        strHPath
        , colMetaVals
        , colChunkIds
        , fcolChunkSizes
        , c4SerialNum
        , enctLastChange
        , bThrowIfNot
        , sectUser
    );
}


//
//  Query the names of items (not scopes) under the indicate scope path
//
tCIDLib::TBoolean
TDataSrvClient::bQueryScopeNames(       tCIDLib::TCard4&        c4ScopeId
                                , const TString&                strRelPath
                                , const tCQCRemBrws::EDTypes    eType
                                ,       tCIDLib::TStrList&      colToFill
                                , const tCQCRemBrws::EQSFlags   eQSFlags
                                , const TCQCSecToken&           sectUser)
{
    CheckReady();

    // Convert the path to a full hierachical path
    TString strHPath;
    facCQCRemBrws().ToHPath(strRelPath, eType, strHPath);
    return m_porbcDS->bQueryScopeNames
    (
        c4ScopeId, strHPath, colToFill, eQSFlags, sectUser
    );
}


//
//  A type specific method that lets the client get some basic info about a template
//  without having to download the whole thing.
//
tCIDLib::TBoolean
TDataSrvClient::bQueryTemplateInfo( const   TString&                strRelPath
                                    ,       tCIDLib::TCard4&        c4TmplSz
                                    ,       tCQCKit::EUserRoles&    eMinRole
                                    ,       TString&                strNotes
                                    ,       tCIDLib::TCard4&        c4SerialNum
                                    ,       tCIDLib::TEncodedTime&  enctLastChange
                                    , const tCIDLib::TBoolean       bThrowIfNot
                                    , const TCQCSecToken&           sectUser)
{
    CheckReady();

    // Convert the path to a full hierachical path
    TString strHPath;
    facCQCRemBrws().ToHPath(strRelPath, tCQCRemBrws::EDTypes::Template, strHPath);

    CheckIsHPath(strHPath);

    //
    //  This call will return the metadata values plus the size of the data chunks.
    //  This enough to do what we need.
    //
    tCIDLib::TKVPFList colMeta;
    tCIDLib::TStrList colChunkIds;
    tCIDLib::TCardList fcolSizes;
    const tCIDLib::TBoolean bRes = m_porbcDS->bQueryItemMeta
    (
        strHPath
        , colMeta
        , colChunkIds
        , fcolSizes
        , c4SerialNum
        , enctLastChange
        , bThrowIfNot
        , sectUser
    );

    if (!bRes)
    {
        c4TmplSz = 0;
        strNotes.Clear();
        return kCIDLib::False;
    }

    //
    //  Check the meta values, which should have the notes and min role values
    //  redundantly stored for this purpose.
    //
    tCIDLib::TCard4 c4GotCnt = 0;
    tCIDLib::TCard4 c4Count = colMeta.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TKeyValuePair& kvalCur = colMeta[c4Index];
        if (kvalCur.strKey() == kCIDMData::strChFlMetaKey_Notes)
        {
            c4GotCnt++;
            strNotes = kvalCur.strValue();
        }
         else if (kvalCur.strKey() == kCQCRemBrws::strMetaKey_MinRole)
        {
            c4GotCnt++;
            eMinRole = tCQCKit::eAltXlatEUserRoles(kvalCur.strValue());
            CIDAssert
            (
                eMinRole != tCQCKit::EUserRoles::Count
                , TString(L"Invalid user role for template ") + strRelPath
            );
        }
    }

    // Go through the chunks and find the main data chunk size
    c4Count = colMeta.c4ElemCount();
    tCIDLib::TCard4 c4Index = 0;
    while (c4Index < c4Count)
    {
        if (colChunkIds[c4Index].bCompareI(kCIDMData::strChunkId_Data))
        {
            c4TmplSz = fcolSizes[c4Index];
            break;
        }
        c4Index++;
    }

    // If we didn't get both meta values or the data size, then an error
    CIDAssert
    (
        c4GotCnt == 2
        , TString(L"Did not get both notes and user role for template ") + strRelPath
    );
    CIDAssert
    (
        c4Index < c4Count
        , TString(L"Did not get data chunk size for template ") + strRelPath
    );

    return kCIDLib::True;
}


// A type specific helper to read the configuration of an event monitor
tCIDLib::TBoolean
TDataSrvClient::bReadEventMon(  const   TString&                strRelPath
                                ,       tCIDLib::TCard4&        c4SerialNum
                                ,       tCIDLib::TEncodedTime&  enctLastChange
                                ,       TCQCEvMonCfg&           emoncToFill
                                ,       tCIDLib::TKVPFList&     colMeta
                                , const TCQCSecToken&           sectUser
                                , const tCIDLib::TBoolean       bNoCache)
{
    THeapBuf mbufData(8);
    THeapBuf mbufExt(8);

    tCIDLib::TCard4 c4Flags = kCQCRemBrws::c4Flag_None;
    if (bNoCache)
        c4Flags |= kCQCRemBrws::c4Flag_NoDataCache;

    TString strExtChunkId;
    colMeta.RemoveAll();
    tCIDLib::TBoolean bRes = kCIDLib::False;
    try
    {
        tCIDLib::TCard4 c4DataBytes, c4ExtBytes;
        bRes = bReadRawData
        (
            strRelPath
            , tCQCRemBrws::EDTypes::EvMonitor
            , c4SerialNum
            , colMeta
            , mbufData
            , c4DataBytes
            , strExtChunkId
            , mbufExt
            , c4ExtBytes
            , c4Flags
            , enctLastChange
            , sectUser
        );

        // If we got new data, stream in the event monitor config it holds
        if (bRes)
        {
            TBinMBufInStream strmSrc(&mbufData, c4DataBytes);
            strmSrc >> emoncToFill;
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
    return bRes;
}


//
//  A generic method to read a whole chunked file.
//
tCIDLib::TBoolean
TDataSrvClient::bReadFile(  const   TString&                strRelPath
                            , const tCQCRemBrws::EDTypes    eDType
                            ,       tCIDLib::TCard4&        c4SerialNum
                            ,       tCIDLib::TEncodedTime&  enctLastChange
                            ,       TChunkedFile&           chflToFill
                            , const TCQCSecToken&           sectUser
                            , const tCIDLib::TBoolean       bNoCache)
{
    THeapBuf mbufData(8);
    THeapBuf mbufExt(8);

    tCIDLib::TCard4 c4Flags = kCQCRemBrws::c4Flag_None;
    if (bNoCache)
        c4Flags |= kCQCRemBrws::c4Flag_NoDataCache;

    tCIDLib::TKVPFList  colMeta;
    TString             strExtChunkId;
    tCIDLib::TBoolean bRes = kCIDLib::False;
    try
    {
        tCIDLib::TCard4 c4DataBytes, c4ExtBytes;
        bRes = bReadRawData
        (
            strRelPath
            , eDType
            , c4SerialNum
            , colMeta
            , mbufData
            , c4DataBytes
            , strExtChunkId
            , mbufExt
            , c4ExtBytes
            , c4Flags
            , enctLastChange
            , sectUser
        );

        // If new data, then load it into the caller's file object
        if (bRes)
        {
            chflToFill.Reset(c4DataBytes, mbufData, colMeta);
            if (!strExtChunkId.bIsEmpty())
                chflToFill.AddExtChunk(strExtChunkId, c4ExtBytes, mbufExt, kCIDLib::False);
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
    return bRes;
}


// A type specific helper to read a global action
tCIDLib::TBoolean
TDataSrvClient::bReadGlobalAction(  const   TString&                strRelPath
                                    ,       tCIDLib::TCard4&        c4SerialNum
                                    ,       tCIDLib::TEncodedTime&  enctLastChange
                                    ,       TCQCStdCmdSrc&          csrcToFill
                                    ,       tCIDLib::TKVPFList&     colMeta
                                    , const TCQCSecToken&           sectUser
                                    , const tCIDLib::TBoolean       bNoCache)
{
    THeapBuf mbufData(8);
    THeapBuf mbufExt(8);

    tCIDLib::TCard4 c4Flags = kCQCRemBrws::c4Flag_None;
    if (bNoCache)
        c4Flags |= kCQCRemBrws::c4Flag_NoDataCache;

    TString strExtChunkId;

    colMeta.RemoveAll();
    tCIDLib::TBoolean bRes = kCIDLib::False;
    try
    {
        tCIDLib::TCard4 c4DataBytes, c4ExtBytes;
        bRes = bReadRawData
        (
            strRelPath
            , tCQCRemBrws::EDTypes::GlobalAct
            , c4SerialNum
            , colMeta
            , mbufData
            , c4DataBytes
            , strExtChunkId
            , mbufExt
            , c4ExtBytes
            , c4Flags
            , enctLastChange
            , sectUser
        );

        // If we got new data, stream it in
        if (bRes)
        {
            TBinMBufInStream strmSrc(&mbufData, c4DataBytes);
            strmSrc >> csrcToFill;
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
    return bRes;
}


//
//  Type specific helpers to read an image, an image plus thumb, or just a thumb. For
//  the image and thumb versions, we can return a buffer or a an already streamed in
//  image object.
//
tCIDLib::TBoolean
TDataSrvClient::bReadImage( const   TString&                strRelPath
                            ,       tCIDLib::TCard4&        c4SerNum
                            ,       tCIDLib::TEncodedTime&  enctLast
                            ,       TPNGImage&              imgToFill
                            ,       tCIDLib::TKVPFList&     colMeta
                            , const TCQCSecToken&           sectUser
                            , const tCIDLib::TBoolean       bNoCache)
{
    // Call the other version to get the raw bytes, then stream it in
    THeapBuf mbufData(8);
    tCIDLib::TBoolean bRes = kCIDLib::False;
    try
    {
        tCIDLib::TCard4 c4Bytes;
        bRes = bReadImage
        (
            strRelPath, c4SerNum, enctLast, mbufData, c4Bytes, colMeta, sectUser, bNoCache
        );

        if (bRes)
        {
            TBinMBufInStream strmSrc(&mbufData, c4Bytes);
            strmSrc >> imgToFill;
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
    return bRes;
}

tCIDLib::TBoolean
TDataSrvClient::bReadImage( const   TString&                strRelPath
                            ,       tCIDLib::TCard4&        c4SerialNum
                            ,       tCIDLib::TEncodedTime&  enctLastChange
                            ,       TMemBuf&                mbufToFill
                            , COP   tCIDLib::TCard4&        c4Bytes
                            ,       tCIDLib::TKVPFList&     colMeta
                            , const TCQCSecToken&           sectUser
                            , const tCIDLib::TBoolean       bNoCache)
{
    THeapBuf mbufExt(8);

    tCIDLib::TCard4 c4Flags = kCQCRemBrws::c4Flag_None;
    if (bNoCache)
        c4Flags |= kCQCRemBrws::c4Flag_NoDataCache;

    // Tell it not to return the extension chunk since we don't need the thumb here
    c4Flags |= kCQCRemBrws::c4Flag_NoExtChunk;

    TString strExtChunkId;

    colMeta.RemoveAll();
    tCIDLib::TBoolean bRes = kCIDLib::False;
    try
    {
        tCIDLib::TCard4 c4ExtBytes;
        bRes = bReadRawData
        (
            strRelPath
            , tCQCRemBrws::EDTypes::Image
            , c4SerialNum
            , colMeta
            , mbufToFill
            , c4Bytes
            , strExtChunkId
            , mbufExt
            , c4ExtBytes
            , c4Flags
            , enctLastChange
            , sectUser
        );
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
    return bRes;
}


tCIDLib::TBoolean
TDataSrvClient::bReadImage( const   TString&                strRelPath
                            ,       tCIDLib::TCard4&        c4SerialNum
                            ,       tCIDLib::TEncodedTime&  enctLastChange
                            ,       TMemBuf&                mbufFull
                            , COP   tCIDLib::TCard4&        c4FullBytes
                            ,       TMemBuf&                mbufThumb
                            , COP   tCIDLib::TCard4&        c4ThumbBytes
                            ,       tCIDLib::TKVPFList&     colMeta
                            , const TCQCSecToken&           sectUser
                            , const tCIDLib::TBoolean       bNoCache)
{
    tCIDLib::TCard4 c4Flags = kCQCRemBrws::c4Flag_None;
    if (bNoCache)
        c4Flags |= kCQCRemBrws::c4Flag_NoDataCache;

    TString strExtChunkId;

    colMeta.RemoveAll();
    tCIDLib::TBoolean bRes = kCIDLib::False;
    try
    {
        bRes = bReadRawData
        (
            strRelPath
            , tCQCRemBrws::EDTypes::Image
            , c4SerialNum
            , colMeta
            , mbufFull
            , c4FullBytes
            , strExtChunkId
            , mbufThumb
            , c4ThumbBytes
            , c4Flags
            , enctLastChange
            , sectUser
        );
    }
    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
    return bRes;
}


tCIDLib::TBoolean
TDataSrvClient::bReadThumb( const   TString&                strRelPath
                            ,       tCIDLib::TCard4&        c4SerialNum
                            ,       tCIDLib::TEncodedTime&  enctLastChange
                            ,       THeapBuf&               mbufData
                            , COP   tCIDLib::TCard4&        c4Bytes
                            , const TCQCSecToken&           sectUser
                            , const tCIDLib::TBoolean       bNoCache)
{
    TString strHPath;
    facCQCRemBrws().ToHPath(strRelPath, tCQCRemBrws::EDTypes::Image, strHPath);
    CheckIsHPath(strHPath);

    tCIDLib::TBoolean bRes = kCIDLib::False;
    try
    {
        bRes = m_porbcDS->bQueryExtChunk
        (
            c4SerialNum
            , strHPath
            , kCQCRemBrws::strChunkId_Thumbnail
            , c4Bytes
            , mbufData
            , sectUser
            , bNoCache
        );
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
    return bRes;
}


tCIDLib::TBoolean
TDataSrvClient::bReadThumb( const   TString&                strRelPath
                            ,       tCIDLib::TCard4&        c4SerNum
                            ,       tCIDLib::TEncodedTime&  enctLast
                            ,       TPNGImage&              imgToFill
                            , const TCQCSecToken&           sectUser
                            , const tCIDLib::TBoolean       bNoCache)
{
    THeapBuf mbufExt;
    tCIDLib::TBoolean bRes = kCIDLib::False;
    try
    {
        tCIDLib::TCard4 c4Bytes;
        if (bReadThumb(strRelPath, c4SerNum, enctLast, mbufExt, c4Bytes, sectUser, bNoCache))
        {
            TBinMBufInStream strmSrc(&mbufExt, c4Bytes);
            strmSrc >> imgToFill;
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
    return bRes;
}


//
//  A type specific helper to read a macro.
//
tCIDLib::TBoolean
TDataSrvClient::bReadMacro( const   TString&                strRelPath
                            ,       tCIDLib::TCard4&        c4SerialNum
                            ,       tCIDLib::TEncodedTime&  enctLastChange
                            ,       TString&                strToFill
                            , const tCIDLib::TBoolean       bDoEncrypted
                            ,       tCIDLib::TKVPFList&     colMeta
                            , const TCQCSecToken&           sectUser
                            , const tCIDLib::TBoolean       bNoCache)
{
    tCIDLib::TBoolean bRes = kCIDLib::False;
    try
    {
        THeapBuf mbufData(8);
        THeapBuf mbufExt(8);

        tCIDLib::TCard4 c4Flags = kCQCRemBrws::c4Flag_None;
        if (bNoCache)
            c4Flags |= kCQCRemBrws::c4Flag_NoDataCache;

        tCIDLib::TCard4 c4DataBytes, c4ExtBytes;
        TString strExtChunkId;

        colMeta.RemoveAll();
        bRes = bReadRawData
        (
            strRelPath
            , tCQCRemBrws::EDTypes::Macro
            , c4SerialNum
            , colMeta
            , mbufData
            , c4DataBytes
            , strExtChunkId
            , mbufExt
            , c4ExtBytes
            , c4Flags
            , enctLastChange
            , sectUser
        );

        if (bRes)
        {
            TBinMBufInStream strmSrc(&mbufData, c4DataBytes);
            strmSrc >> strToFill;
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }

    return bRes;
}


//
//  A type specific helper to read scheduled event. The caller indicates if the
//  event needs to have it's next time updated and written back, so that it can
//  all be done on the server.
//
tCIDLib::TBoolean
TDataSrvClient::bReadSchedEvent(const   TString&                strRelPath
                                ,       tCIDLib::TCard4&        c4SerialNum
                                ,       tCIDLib::TEncodedTime&  enctLastChange
                                ,       TCQCSchEvent&           csrcToFill
                                ,       tCIDLib::TKVPFList&     colMeta
                                , const TCQCSecToken&           sectUser
                                , const tCIDLib::TBoolean       bNoCache)
{
    THeapBuf mbufData(8);
    THeapBuf mbufExt(8);

    tCIDLib::TCard4 c4Flags = kCQCRemBrws::c4Flag_None;
    if (bNoCache)
        c4Flags |= kCQCRemBrws::c4Flag_NoDataCache;

    TString strExtChunkId;

    colMeta.RemoveAll();
    tCIDLib::TBoolean bRes = kCIDLib::False;
    try
    {
        tCIDLib::TCard4 c4DataBytes, c4ExtBytes;
        bRes = bReadRawData
        (
            strRelPath
            , tCQCRemBrws::EDTypes::SchEvent
            , c4SerialNum
            , colMeta
            , mbufData
            , c4DataBytes
            , strExtChunkId
            , mbufExt
            , c4ExtBytes
            , c4Flags
            , enctLastChange
            , sectUser
        );

        // If we got new data, stream it in
        if (bRes)
        {
            TBinMBufInStream strmSrc(&mbufData, c4DataBytes);
            strmSrc >> csrcToFill;
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
    return bRes;
}


//
//  A type specific helper to stream down a template. In this case, we just return
//  it as a memory buffer. Templates are too high level for us to be dependent on
//  here. It would require that everything that used this facility also link in the
//  IV engine. So the IV engine will probably it's own wrapper around this to get
//  the actual template object.
//
tCIDLib::TBoolean
TDataSrvClient::bReadTemplate(  const   TString&                strRelPath
                                ,       tCIDLib::TCard4&        c4SerialNum
                                ,       tCIDLib::TEncodedTime&  enctLastChange
                                ,       TMemBuf&                mbufToFill
                                ,       tCIDLib::TCard4&        c4Bytes
                                ,       tCIDLib::TKVPFList&     colMeta
                                , const TCQCSecToken&           sectUser
                                , const tCIDLib::TBoolean       bNoCache)
{
    THeapBuf mbufExt(8);

    tCIDLib::TCard4 c4Flags = kCQCRemBrws::c4Flag_None;
    if (bNoCache)
        c4Flags |= kCQCRemBrws::c4Flag_NoDataCache;

    TString strExtChunkId;
    colMeta.RemoveAll();
    tCIDLib::TBoolean bRes = kCIDLib::False;
    try
    {
        tCIDLib::TCard4 c4ExtBytes;
        bRes = bReadRawData
        (
            strRelPath
            , tCQCRemBrws::EDTypes::Template
            , c4SerialNum
            , colMeta
            , mbufToFill
            , c4Bytes
            , strExtChunkId
            , mbufExt
            , c4ExtBytes
            , c4Flags
            , enctLastChange
            , sectUser
        );
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }

    return bRes;
}


// A type specific helper to read triggered event
tCIDLib::TBoolean
TDataSrvClient::bReadTrigEvent( const   TString&                strRelPath
                                ,       tCIDLib::TCard4&        c4SerialNum
                                ,       tCIDLib::TEncodedTime&  enctLastChange
                                ,       TCQCTrgEvent&           csrcToFill
                                ,       tCIDLib::TKVPFList&     colMeta
                                , const TCQCSecToken&           sectUser
                                , const tCIDLib::TBoolean       bNoCache)
{
    THeapBuf mbufData(8);
    THeapBuf mbufExt(8);

    tCIDLib::TCard4 c4Flags = kCQCRemBrws::c4Flag_None;
    if (bNoCache)
        c4Flags |= kCQCRemBrws::c4Flag_NoDataCache;

    TString strExtChunkId;
    colMeta.RemoveAll();

    tCIDLib::TBoolean bRes = kCIDLib::False;
    try
    {
        tCIDLib::TCard4 c4DataBytes, c4ExtBytes;
        bRes = bReadRawData
        (
            strRelPath
            , tCQCRemBrws::EDTypes::TrgEvent
            , c4SerialNum
            , colMeta
            , mbufData
            , c4DataBytes
            , strExtChunkId
            , mbufExt
            , c4ExtBytes
            , c4Flags
            , enctLastChange
            , sectUser
        );

        // If we got new data, stream it in
        if (bRes)
        {
            TBinMBufInStream strmSrc(&mbufData, c4DataBytes);
            strmSrc >> csrcToFill;
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
    return bRes;
}


// Check to see if a specific scope exists
tCIDLib::TBoolean
TDataSrvClient::bScopeExists(const  TString&                strRelToFind
                            , const tCQCRemBrws::EDTypes    eType)
{
    CheckReady();

    // Convert the path to a full hierachical path
    TString strHPath;
    facCQCRemBrws().ToHPath(strRelToFind, eType, strHPath);

    CheckIsHPath(strHPath);
    return m_porbcDS->bExists(strHPath, kCIDLib::True);
}

tCIDLib::TBoolean
TDataSrvClient::bScopeExists(const  TString&                strRelScope
                            , const TString&                strToFind
                            , const tCQCRemBrws::EDTypes    eType)
{
    CheckReady();

    // Convert the path to a full hierachical path
    TString strHPath;
    facCQCRemBrws().ToHPath(strRelScope, strToFind, eType, strHPath);

    CheckIsHPath(strHPath);
    return m_porbcDS->bExists(strHPath, kCIDLib::True);
}



// Delete either a file or a scope (and all of it's contents.)
tCIDLib::TVoid
TDataSrvClient::DeleteFile( const   TString&                strRelPath
                            , const tCQCRemBrws::EDTypes    eType
                            , const TCQCSecToken&           sectUser
                            , const tCIDLib::TBoolean       bMustExist)
{
    CheckReady();

    // Convert the path to a full hierachical path
    TString strHPath;
    facCQCRemBrws().ToHPath(strRelPath, eType, strHPath);

    CheckIsHPath(strHPath);
    m_porbcDS->DeletePath(strHPath, kCIDLib::False, bMustExist, sectUser);
}


tCIDLib::TVoid
TDataSrvClient::DeleteScope(const   TString&                strRelPath
                            , const tCQCRemBrws::EDTypes    eType
                            , const TCQCSecToken&           sectUser
                            , const tCIDLib::TBoolean       bMustExist)
{
    CheckReady();

    // Convert the path to a full hierachical path
    TString strHPath;
    facCQCRemBrws().ToHPath(strRelPath, eType, strHPath);

    CheckIsHPath(strHPath);
    m_porbcDS->DeletePath(strHPath, kCIDLib::True, bMustExist, sectUser);
}


//
//  Rename a scope or file. The caller has to say which he expects it to be, just
//  as a safety measure, and the server will throw if it's not.
//
tCIDLib::ERenameRes
TDataSrvClient::eRename(const   TString&                strRelParScope
                        , const TString&                strOldName
                        , const TString&                strNewName
                        , const tCQCRemBrws::EDTypes    eType
                        , const tCIDLib::TBoolean       bIsScope
                        ,       tCIDLib::TCard4&        c4ParScopeId
                        ,       tCIDLib::TCard4&        c4TarItemId
                        , const TCQCSecToken&           sectUser)
{
    CheckReady();

    // Convert the parent path to a full hierachical path
    TString strDSParPath;
    facCQCRemBrws().ToHPath(strRelParScope, eType, strDSParPath);

    CheckIsHPath(strDSParPath);
    return m_porbcDS->eRenameItem
    (
        strDSParPath
        , strOldName
        , strNewName
        , bIsScope
        , c4ParScopeId
        , c4TarItemId
        , sectUser
    );
}


//
//  Generates reports by asking the data server for report info. This is returned as a
//  compressed buffer of XML. We have one version that does a specific scope (and
//  optionally contained content) and one that does a full report on all data types.
//
tCIDLib::TVoid
TDataSrvClient::GenerateFullReport(         TTextOutStream& strmTar
                                    , const TString&        strDescr
                                    , const TCQCSecToken&   sectUser)
{
    CheckReady();

    // Just set up a vector with all of the types supported by the report mechanism
    tCQCRemBrws::EDTypeList fcolTypes;
    fcolTypes.c4AddElement(tCQCRemBrws::EDTypes::EvMonitor);
    fcolTypes.c4AddElement(tCQCRemBrws::EDTypes::GlobalAct);
    fcolTypes.c4AddElement(tCQCRemBrws::EDTypes::Image);
    fcolTypes.c4AddElement(tCQCRemBrws::EDTypes::Macro);
    fcolTypes.c4AddElement(tCQCRemBrws::EDTypes::SchEvent);
    fcolTypes.c4AddElement(tCQCRemBrws::EDTypes::Template);
    fcolTypes.c4AddElement(tCQCRemBrws::EDTypes::TrgEvent);

    // Ask the server for a report for these types
    tCIDLib::TCard4 c4Bytes;
    THeapBuf mbufData;
    m_porbcDS->GenerateReport(strDescr, fcolTypes, c4Bytes, mbufData, sectUser);

    // Decompress the data
    THeapBuf mbufDecomp;
    {
        TZLibCompressor zlibDecomp;
        TBinMBufInStream strmSrc(&mbufData, c4Bytes);
        TBinMBufOutStream strmTmp(&mbufDecomp);
        c4Bytes = zlibDecomp.c4Decompress(strmSrc, strmTmp);
    }

    //
    //  We want to 'pretty print' the text on this side, so we need to parse the XML
    //  so that we can print from the tree representation. So create a memory buffer
    //  entity source over the buffer. Give away the buffer since we don't need it
    //  anymore.
    //
    tCIDXML::TEntitySrcRef esrReport
    (
        new TMemBufEntitySrc
        (
            L"CQCReport", TString::strEmpty(), tCIDLib::ForceMove(mbufDecomp), c4Bytes
        )
    );

    // Set up an XML tree parser and force our in-memory DTD onto it
    TXMLTreeParser xtprsReport;
    xtprsReport.AddMapping
    (
        new TMemBufEntitySrc
        (
            L"CQCReport.DTD", TFacCQCRemBrws::strReportDTDURN, TFacCQCRemBrws::strReportDTD
        )
    );

    const tCIDLib::TBoolean bOk = xtprsReport.bParseRootEntity
    (
        esrReport
        , tCIDXML::EParseOpts::Validate
        , tCIDXML::EParseFlags::TagsNText
    );

    if (!bOk)
    {
        //
        //  Format the first error in the error list, into a string that
        //  we can pass as a replacement parameter to the real error we
        //  log.
        //
        const TXMLTreeParser::TErrInfo& erriFirst
                                        = xtprsReport.colErrors()[0];

        TTextStringOutStream strmOut(512);
        strmOut << L"[" << erriFirst.strSystemId() << L"/"
                << erriFirst.c4Line() << L"." << erriFirst.c4Column()
                << L"] " << erriFirst.strText() << kCIDLib::FlushIt;

        facCQCRemBrws().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kRBrwsErrs::errcRep_ParseFailed
            , strmOut.strData()
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
        );
    }


    // Dump the tree to the caller's stream
    xtprsReport.xtdocThis().xtnodeRoot().PrintTo(strmTar, 0);

    // Flush it since the above call doesn't know it's a top level calls
    strmTar.Flush();
}


tCIDLib::TVoid
TDataSrvClient::GenerateReport(         TTextOutStream&         strmTar
                                , const TString&                strDescr
                                , const TString&                strRelScope
                                , const tCQCRemBrws::EDTypes    eType
                                , const tCIDLib::TBoolean       bRecurse
                                , const TCQCSecToken&           sectUser)
{
    CheckReady();

    // Convert the path to a full hierachical path
    TString strHPath;
    facCQCRemBrws().ToHPath(strRelScope, eType, strHPath);
    CheckIsHPath(strHPath);

    tCIDLib::TCard4 c4Bytes;
    THeapBuf mbufData;
    m_porbcDS->GenerateTypeReport(strDescr, strHPath, bRecurse, c4Bytes, mbufData, sectUser);

    // Decompress the data
    THeapBuf mbufDecomp;
    {
        TZLibCompressor zlibDecomp;
        TBinMBufInStream strmSrc(&mbufData, c4Bytes);
        TBinMBufOutStream strmTmp(&mbufDecomp);
        c4Bytes = zlibDecomp.c4Decompress(strmSrc, strmTmp);
    }


    //
    //  We want to 'pretty print' the text on this side, so we need to parse the XML
    //  so that we can print from the tree representation. So create a memory buffer
    //  entity source over the buffer. Give away the buffer since we don't need it
    //  anymore.
    //
    tCIDXML::TEntitySrcRef esrReport
    (
        new TMemBufEntitySrc
        (
            L"CQCReport", TString::strEmpty(), tCIDLib::ForceMove(mbufDecomp), c4Bytes
        )
    );

    // Set up an XML tree parser and force our in-memory DTD onto it
    TXMLTreeParser xtprsReport;
    xtprsReport.AddMapping
    (
        new TMemBufEntitySrc
        (
            L"CQCReport.DTD", TFacCQCRemBrws::strReportDTDURN, TFacCQCRemBrws::strReportDTD
        )
    );

    const tCIDLib::TBoolean bOk = xtprsReport.bParseRootEntity
    (
        esrReport
        , tCIDXML::EParseOpts::Validate
        , tCIDXML::EParseFlags::TagsNText
    );

    if (!bOk)
    {
        //
        //  Format the first error in the error list, into a string that
        //  we can pass as a replacement parameter to the real error we
        //  log.
        //
        const TXMLTreeParser::TErrInfo& erriFirst
                                        = xtprsReport.colErrors()[0];

        TTextStringOutStream strmOut(512);
        strmOut << L"[" << erriFirst.strSystemId() << L"/"
                << erriFirst.c4Line() << L"." << erriFirst.c4Column()
                << L"] " << erriFirst.strText() << kCIDLib::FlushIt;

        facCQCRemBrws().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kRBrwsErrs::errcRep_ParseFailed
            , strmOut.strData()
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
        );
    }


    // Dump the tree to the caller's stream
    xtprsReport.xtdocThis().xtnodeRoot().PrintTo(strmTar, 0);

    // Flush it since the above call doesn't know it's a top level calls
    strmTar.Flush();
}


//
//  Create a new scope under a parent path. It is assumed that the scope has no
//  path component, so we don't have to do any conversion on it.
//
tCIDLib::TVoid
TDataSrvClient::MakeNewScope(const  TString&                strRelParScope
                            , const TString&                strNewSubScope
                            , const tCQCRemBrws::EDTypes    eType
                            , const TCQCSecToken&           sectUser)
{
    CheckReady();

    // Convert the path to a full hierachical path
    TString strHPath;
    facCQCRemBrws().ToHPath(strRelParScope, eType, strHPath);

    CheckIsHPath(strHPath);
    m_porbcDS->MakeNewScope(strHPath, strNewSubScope, sectUser);
}


// Create all of the missing components of the passed path
tCIDLib::TVoid
TDataSrvClient::MakePath(const  TString&                strRelPath
                        , const tCQCRemBrws::EDTypes    eType
                        , const TCQCSecToken&           sectUser)
{
    CheckReady();

    // Convert the path to a full hierachical path
    TString strHPath;
    facCQCRemBrws().ToHPath(strRelPath, eType, strHPath);

    CheckIsHPath(strHPath);
    m_porbcDS->MakePath(strHPath, sectUser);
}


//
//  The data server will replicate the contents of the from path to the to scope. Both must
//  exist already. Any conflicts will result in overwrites. The source can be a single item,
//  or a scope. If a scope, and it has sub-scopes, the process proceeds recursively.
//
tCIDLib::TVoid
TDataSrvClient::PastePath(  const   TString&            strSrcPath
                            , const TString&            strTarScope
                            , const tCIDLib::TBoolean   bSrcIsScope
                            , const TCQCSecToken&       sectUser)
{
    CheckReady();

    CheckIsHPath(strSrcPath);
    CheckIsHPath(strTarScope);
    m_porbcDS->PastePath(strSrcPath, strTarScope, bSrcIsScope, sectUser);
}


// Ping the server to make sure it's still there. If not, an error will occur
tCIDLib::TVoid TDataSrvClient::Ping()
{
    CheckReady();
    m_porbcDS->Ping();
}


tCIDLib::TVoid
TDataSrvClient::QueryBrowserItem(const  TString&                strHPath
                                ,       tCIDLib::TCardList&     fcolPathIds
                                ,       TDSBrowseItem&          dsbiToFill
                                ,       tCIDLib::TBoolean&      bIsScope
                                , const TCQCSecToken&           sectUser)
{
    CheckReady();
    CheckIsHPath(strHPath);
    m_porbcDS->QueryItemInfo(strHPath, bIsScope, fcolPathIds, dsbiToFill, sectUser);
}


tCIDLib::TVoid
TDataSrvClient::QueryBrowserItem2(  const   TString&                strHPath
                                    ,       tCIDLib::TCardList&     fcolPathIds
                                    ,       TDSBrowseItem&          dsbiTarget
                                    ,       TDSBrowseItem&          dsbiParScope
                                    ,       tCIDLib::TBoolean&      bIsScope
                                    , const TCQCSecToken&           sectUser)
{
    CheckReady();
    CheckIsHPath(strHPath);
    m_porbcDS->QueryItemInfo2
    (
        strHPath, bIsScope, fcolPathIds, dsbiTarget, dsbiParScope, sectUser
    );
}


//
//  Query the browser items and path ids of all of the items (or only the non-scope
//  items) under the passed parent path. We get a full hierarchical path here
//
tCIDLib::TVoid
TDataSrvClient::QueryScopeItems(const   TString&                strHPath
                                ,       tCIDLib::TCardList&     fcolPathIds
                                ,       TVector<TDSBrowseItem>& colItems
                                , const tCIDLib::TBoolean       bItemsOnly
                                , const TCQCSecToken&           sectUser)
{
    CheckReady();
    CheckIsHPath(strHPath);
    m_porbcDS->QueryScopeItems(strHPath, fcolPathIds, colItems, bItemsOnly, sectUser);
}


//
//  Query the hierarchy underneath the indicated start path, in XML format, where
//  each node/element just has the name of that file or scope, or just a flat list
//  of paths. We get a relative path and type here
//
tCIDLib::TVoid
TDataSrvClient::QueryTree(  const   TString&                strRelStartPath
                            , const tCQCRemBrws::EDTypes    eType
                            ,       TString&                strXML
                            , const tCIDLib::TBoolean       bSparse
                            , const TCQCSecToken&           sectUser)
{
    CheckReady();

    // Convert the path to a full hierachical path
    TString strHPath;
    facCQCRemBrws().ToHPath(strRelStartPath, eType, strHPath);

    CheckIsHPath(strHPath);
    m_porbcDS->QueryTree(strHPath, strXML, bSparse, sectUser);
}

tCIDLib::TVoid
TDataSrvClient::QueryTree2( const   TString&                strStartPath
                            , const tCQCRemBrws::EDTypes    eType
                            ,       tCIDLib::TStrList&      colToFill
                            , const tCIDLib::TBoolean       bSparse
                            , const TCQCSecToken&           sectUser)
{
    CheckReady();

    // Convert the path to a full hierachical path
    TString strHPath;
    facCQCRemBrws().ToHPath(strStartPath, eType, strHPath);

    CheckIsHPath(strHPath);
    m_porbcDS->QueryTree2(strHPath, colToFill, bSparse, sectUser);
}



//
//  In this case, the server knows the file format and can deal with this for us
//  which makes things a lot simpler.
//
tCIDLib::TVoid
TDataSrvClient::SetEventPauseState( const   tCQCKit::EEvSrvTypes    eEvType
                                    , const TString&                strRelPath
                                    ,       tCIDLib::TCard4&        c4NewSerialNum
                                    ,       tCIDLib::TEncodedTime&  enctLastChange
                                    , const tCIDLib::TBoolean       bPaused
                                    , const TCQCSecToken&           sectUser)
{
    // Convert the event type to a data type
    tCQCRemBrws::EDTypes eDType = tCQCRemBrws::EDTypes::Count;
    if (eEvType == tCQCKit::EEvSrvTypes::EvMonitor)
        eDType = tCQCRemBrws::EDTypes::EvMonitor;
    else if (eEvType == tCQCKit::EEvSrvTypes::SchEvent)
        eDType = tCQCRemBrws::EDTypes::SchEvent;
    else if (eEvType == tCQCKit::EEvSrvTypes::TrgEvent)
        eDType = tCQCRemBrws::EDTypes::TrgEvent;
    else
    {
        facCQCRemBrws().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kRBrwsErrs::errcRem_EventType
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Unknown
            , TInteger(tCIDLib::i4EnumOrd(eEvType))
        );
    }

    // Make sure we are ready to operate
    CheckReady();

    // Convert the path to a full hierachical path
    TString strHPath;
    facCQCRemBrws().ToHPath(strRelPath, eDType, strHPath);
    CheckIsHPath(strHPath);

    m_porbcDS->SetEventPauseState
    (
        strHPath, c4NewSerialNum, enctLastChange, bPaused, sectUser
    );
}

tCIDLib::TVoid
TDataSrvClient::SetPeriodicEvTime(  const   TString&                strRelPath
                                    ,       tCIDLib::TCard4&        c4NewSerialNum
                                    ,       tCIDLib::TEncodedTime&  enctLastChange
                                    , const tCIDLib::TEncodedTime   enctStartTime
                                    , const tCIDLib::TCard4         c4Period
                                    , const TCQCSecToken&           sectUser)
{
    // Make sure we are ready to operate
    CheckReady();

    // Convert the path to a full hierachical path
    TString strHPath;
    facCQCRemBrws().ToHPath(strRelPath, tCQCRemBrws::EDTypes::SchEvent, strHPath);
    CheckIsHPath(strHPath);

    m_porbcDS->SetPeriodicEvTime
    (
        strHPath, c4NewSerialNum, enctLastChange, c4Period, enctStartTime, sectUser
    );
}

tCIDLib::TVoid
TDataSrvClient::SetScheduledEvTime( const   TString&                strRelPath
                                    ,       tCIDLib::TCard4&        c4NewSerialNum
                                    ,       tCIDLib::TEncodedTime&  enctLastChange
                                    , const tCIDLib::TCard4         c4Day
                                    , const tCIDLib::TCard4         c4Hour
                                    , const tCIDLib::TCard4         c4Min
                                    , const tCIDLib::TCard4         c4Mask
                                    , const TCQCSecToken&           sectUser)
{
    // Make sure we are ready to operate
    CheckReady();

    // Convert the path to a full hierachical path
    TString strHPath;
    facCQCRemBrws().ToHPath(strRelPath, tCQCRemBrws::EDTypes::SchEvent, strHPath);
    CheckIsHPath(strHPath);

    m_porbcDS->SetScheduledEvTime
    (
        strHPath, c4NewSerialNum, enctLastChange, c4Day, c4Hour, c4Min, c4Mask, sectUser
    );
}

tCIDLib::TVoid
TDataSrvClient::SetSunBasedEvOffset(const   TString&                strRelPath
                                    ,       tCIDLib::TCard4&        c4NewSerialNum
                                    ,       tCIDLib::TEncodedTime&  enctLastChange
                                    , const tCIDLib::TCard4         i4Offset
                                    , const TCQCSecToken&           sectUser)
{
    // Make sure we are ready to operate
    CheckReady();

    // Convert the path to a full hierachical path
    TString strHPath;
    facCQCRemBrws().ToHPath(strRelPath, tCQCRemBrws::EDTypes::SchEvent, strHPath);
    CheckIsHPath(strHPath);

    m_porbcDS->SetSunBasedEvOffset
    (
        strHPath, c4NewSerialNum, enctLastChange, i4Offset, sectUser
    );
}


//
//  A generic file uploader, for when the caller already has the chunked file. This is
//  mostly for package import. It's basically just a pass through to the write data
//  method.
//
tCIDLib::TVoid
TDataSrvClient::UploadFile( const   TString&                strRelPath
                            , const tCQCRemBrws::EDTypes    eDType
                            ,       tCIDLib::TCard4&        c4NewSerialNum
                            ,       tCIDLib::TEncodedTime&  enctLastChange
                            , const TChunkedFile&           chflToSend
                            ,       tCIDLib::TCard4&        c4Flags
                            , const TCQCSecToken&           sectUser)
{
    tCIDLib::TCard4 c4DataChunkSz;
    const TMemBuf& mbufDataChunk = chflToSend.mbufChunkById
    (
        kCIDMData::strChunkId_Data, c4DataChunkSz
    );

    // See if there's any extension chunk. If more than two there is
    tCIDLib::TCard4 c4ExtSz = 0;
    const TMemBuf* pmbufExt = nullptr;
    TString strExtId;
    THeapBuf mbufEmpty(8);
    if (chflToSend.c4ChunkCount() > 2)
        pmbufExt = &chflToSend.mbufChunkAt(2, strExtId, c4ExtSz);
    else
        pmbufExt = &mbufEmpty;

    WriteRawData
    (
        strRelPath
        , eDType
        , c4NewSerialNum
        , chflToSend.colMetaValues()
        , mbufDataChunk
        , c4DataChunkSz
        , strExtId
        , *pmbufExt
        , c4ExtSz
        , c4Flags
        , enctLastChange
        , sectUser
    );
}


// A type specific helper to send up an event monitor configuration
tCIDLib::TVoid
TDataSrvClient::WriteEventMon(  const   TString&                strRelPath
                                ,       tCIDLib::TCard4&        c4NewSerialNum
                                ,       tCIDLib::TEncodedTime&  enctLastChange
                                , const TCQCEvMonCfg&           emoncToSend
                                , const tCIDLib::TCard4         c4Flags
                                , const tCIDLib::TKVPFList&     colExtraMeta
                                , const TCQCSecToken&           sectUser)
{
    // Make sure we are ready to operate
    CheckReady();

    // Flatten the incomning config object
    TBinMBufOutStream strmTar(32 * 1024);
    strmTar << emoncToSend << kCIDLib::FlushIt;

    //
    //  Set up the paused metadata key that has to match the paused state, and we
    //  store the description in a title meta value.
    //
    const tCIDLib::TCard4 c4XMeta = colExtraMeta.c4ElemCount();
    tCIDLib::TKVPFList colMeta(2 + c4XMeta);
    colMeta.objPlace(kCIDMData::strChFlMetaKey_Title, emoncToSend.strDescr());
    colMeta.objPlace
    (
        kCQCRemBrws::strMetaKey_Paused, facCQCKit().strBoolVal(emoncToSend.bPaused())
    );

    // If there are any extra meta values to add, add those
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4XMeta; c4Index++)
        colMeta.objAdd(colExtraMeta[c4Index]);

    THeapBuf mbufExt(8);
    WriteRawData
    (
        strRelPath
        , tCQCRemBrws::EDTypes::EvMonitor
        , c4NewSerialNum
        , colMeta
        , strmTar.mbufData()
        , strmTar.c4CurSize()
        , TString::strEmpty()
        , mbufExt
        , 0
        , c4Flags
        , enctLastChange
        , sectUser
    );
}


// A type specific helper to send up a global action
tCIDLib::TVoid
TDataSrvClient::WriteGlobalAction(  const   TString&                strRelPath
                                    ,       tCIDLib::TCard4&        c4NewSerialNum
                                    ,       tCIDLib::TEncodedTime&  enctLastChange
                                    , const TCQCStdCmdSrc&          csrcToSend
                                    , const tCIDLib::TCard4         c4Flags
                                    , const tCIDLib::TKVPFList&     colExtraMeta
                                    , const TCQCSecToken&           sectUser)
{
    CheckReady();

    // Stream out the data to a stream
    TBinMBufOutStream strmTar(32 * 1024);
    strmTar << csrcToSend << kCIDLib::FlushIt;

    // We store the internal readable title as a meta value
    const tCIDLib::TCard4 c4XMeta = colExtraMeta.c4ElemCount();
    tCIDLib::TKVPFList colMeta(1 + c4XMeta);
    colMeta.objPlace(kCIDMData::strChFlMetaKey_Title, csrcToSend.strTitle());

    // If there are any extra meta values to add, add those
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4XMeta; c4Index++)
        colMeta.objAdd(colExtraMeta[c4Index]);

    THeapBuf mbufExt(8);
    WriteRawData
    (
        strRelPath
        , tCQCRemBrws::EDTypes::GlobalAct
        , c4NewSerialNum
        , colMeta
        , strmTar.mbufData()
        , strmTar.c4CurSize()
        , TString::strEmpty()
        , mbufExt
        , 0
        , c4Flags
        , enctLastChange
        , sectUser
    );
}


//
//  A type specific helper to send up an image, both full and thumb. We also pass
//  pass a little extra data with it, to give is some info about the image format
//  that it would otherwise have to re-stream the image back in as a PNG to get.
//  These are set as meta values for quick access.
//
//  This one is tricky since we have to set two different chunks and we need to set
//  metadata values. So we have to do the 'multi-chunk' format here.
//
tCIDLib::TVoid
TDataSrvClient::WriteImage( const   TString&                strRelPath
                            ,       tCIDLib::TCard4&        c4NewSerialNum
                            ,       tCIDLib::TEncodedTime&  enctLastChange
                            , const TMemBuf&                mbufImg
                            , const tCIDLib::TCard4         c4ImgBytes
                            , const TMemBuf&                mbufThumb
                            , const tCIDLib::TCard4         c4ThumbBytes
                            , const tCIDImage::EPixFmts     eImgFmt
                            , const tCIDImage::EBitDepths   eImgDepth
                            , const TSize&                  szImg
                            , const TSize&                  szThumb
                            , const tCIDLib::TCard4         c4Flags
                            , const tCIDLib::TKVPFList&     colExtraMeta
                            , const TCQCSecToken&           sectUser)
{
    // Set up the metadata flags
    const tCIDLib::TCard4 c4XMeta = colExtraMeta.c4ElemCount();
    tCIDLib::TKVPFList colMeta(4 + c4XMeta);
    colMeta.objPlace
    (
        kCQCRemBrws::strMetaKey_ImgDepth
        , TString(TCardinal(tCIDLib::c4EnumOrd(eImgDepth)))
    );
    colMeta.objPlace
    (
        kCQCRemBrws::strMetaKey_ImgFormat
        , TString(TCardinal(tCIDLib::c4EnumOrd(eImgFmt)))
    );
    colMeta.objPlace(kCQCRemBrws::strMetaKey_ImgSize, TString(szImg));
    colMeta.objPlace(kCQCRemBrws::strMetaKey_ThumbSize, TString(szThumb));

    // If there are any extra meta values to add, add those
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4XMeta; c4Index++)
        colMeta.objAdd(colExtraMeta[c4Index]);

    WriteRawData
    (
        strRelPath
        , tCQCRemBrws::EDTypes::Image
        , c4NewSerialNum
        , colMeta
        , mbufImg
        , c4ImgBytes
        , kCQCRemBrws::strChunkId_Thumbnail
        , mbufThumb
        , c4ThumbBytes
        , c4Flags
        , enctLastChange
        , sectUser
    );
}

tCIDLib::TVoid
TDataSrvClient::WriteImage( const   TString&                strRelPath
                            ,       tCIDLib::TCard4&        c4NewSerialNum
                            ,       tCIDLib::TEncodedTime&  enctLastChange
                            , const TPNGImage&              imgFull
                            , const TPNGImage&              imgThumb
                            , const tCIDImage::EPixFmts     eImgFmt
                            , const tCIDImage::EBitDepths   eImgDepth
                            , const tCIDLib::TCard4         c4Flags
                            , const tCIDLib::TKVPFList&     colExtraMeta
                            , const TCQCSecToken&           sectUser)
{
    CheckReady();

    // Stream the two images out
    TBinMBufOutStream strmImg
    (
        imgFull.c4ImageSz() + 4096, imgFull.c4ImageSz() + (imgFull.c4ImageSz() / 2)
    );
    strmImg << imgFull << kCIDLib::FlushIt;

    TBinMBufOutStream strmThumb
    (
        imgThumb.c4ImageSz() + 4096, imgThumb.c4ImageSz() + (imgThumb.c4ImageSz() / 2)
    );
    strmImg << imgThumb << kCIDLib::FlushIt;

    // Then call the other version
    WriteImage
    (
        strRelPath
        , c4NewSerialNum
        , enctLastChange
        , strmImg.mbufData()
        , strmImg.c4CurSize()
        , strmThumb.mbufData()
        , strmThumb.c4CurSize()
        , eImgFmt
        , eImgDepth
        , imgFull.szImage()
        , imgThumb.szImage()
        , c4Flags
        , colExtraMeta
        , sectUser
    );
}


//
//  A type specific helper to send up a macro. The server will check to see if it was
//  originally encrypted and encrypt it if needed. He has access to the key from the
//  original file.
//
tCIDLib::TVoid
TDataSrvClient::WriteMacro( const   TString&                strRelPath
                            ,       tCIDLib::TCard4&        c4NewSerialNum
                            ,       tCIDLib::TEncodedTime&  enctLastChange
                            , const TString&                strToSend
                            , const tCIDLib::TCard4         c4Flags
                            , const tCIDLib::TKVPFList&     colExtraMeta
                            , const TCQCSecToken&           sectUser)
{
    try
    {
        TBinMBufOutStream strmText(0x200000UL);
        strmText << strToSend << kCIDLib::FlushIt;

        const tCIDLib::TCard4 c4XMeta = colExtraMeta.c4ElemCount();
        tCIDLib::TKVPFList colMeta(1 + c4XMeta);

        //
        //  Currently no standard meta values for macros.
        //
        //  If there are any extra meta values to add, add those
        //
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4XMeta; c4Index++)
            colMeta.objAdd(colExtraMeta[c4Index]);

        THeapBuf mbufExt(8);
        WriteRawData
        (
            strRelPath
            , tCQCRemBrws::EDTypes::Macro
            , c4NewSerialNum
            , colMeta
            , strmText.mbufData()
            , strmText.c4CurSize()
            , TString::strEmpty()
            , mbufExt
            , 0
            , c4Flags
            , enctLastChange
            , sectUser
        );
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}


// A type specific helper to send up a scheduled
tCIDLib::TVoid
TDataSrvClient::WriteScheduledEvent(const   TString&                strRelPath
                                    ,       tCIDLib::TCard4&        c4NewSerialNum
                                    ,       tCIDLib::TEncodedTime&  enctLastChange
                                    , const TCQCSchEvent&           csrcToSend
                                    , const tCIDLib::TCard4         c4Flags
                                    , const tCIDLib::TKVPFList&     colExtraMeta
                                    , const TCQCSecToken&           sectUser)
{
    CheckReady();

    // Stream out the data to a stream
    TBinMBufOutStream strmTar(8 * 1024);
    strmTar << csrcToSend << kCIDLib::FlushIt;

    // Set up the paused metadata value
    const tCIDLib::TCard4 c4XMeta = colExtraMeta.c4ElemCount();
    tCIDLib::TKVPFList colMeta(1 + c4XMeta);
    colMeta.objPlace
    (
        kCQCRemBrws::strMetaKey_Paused, facCQCKit().strBoolVal(csrcToSend.bPaused())
    );

    // If there are any extra meta values to add, add those
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4XMeta; c4Index++)
        colMeta.objAdd(colExtraMeta[c4Index]);

    THeapBuf mbufExt(8);
    WriteRawData
    (
        strRelPath
        , tCQCRemBrws::EDTypes::SchEvent
        , c4NewSerialNum
        , colMeta
        , strmTar.mbufData()
        , strmTar.c4CurSize()
        , TString::strEmpty()
        , mbufExt
        , 0
        , c4Flags
        , enctLastChange
        , sectUser
    );
}


//
//  A type specific helper to send up a template. We can't be dependent on the IV
//  engine here, so we have to accept the data already flattened.
//
tCIDLib::TVoid
TDataSrvClient::WriteTemplate(  const   TString&                strRelPath
                                ,       tCIDLib::TCard4&        c4NewSerialNum
                                ,       tCIDLib::TEncodedTime&  enctLastChange
                                , const TMemBuf&                mbufTmpl
                                , const tCIDLib::TCard4         c4TmplBytes
                                , const tCQCKit::EUserRoles     eMinRole
                                , const TString&                strNotes
                                , const tCIDLib::TCard4         c4Flags
                                , const tCIDLib::TKVPFList&     colExtraMeta
                                , const TCQCSecToken&           sectUser)
{
    // Set up the metadata values
    const tCIDLib::TCard4 c4XMeta = colExtraMeta.c4ElemCount();
    tCIDLib::TKVPFList colMeta(2 + c4XMeta);
    colMeta.objPlace
    (
        kCQCRemBrws::strMetaKey_MinRole, tCQCKit::strAltXlatEUserRoles(eMinRole)
    );
    colMeta.objPlace(kCIDMData::strChFlMetaKey_Notes, strNotes);

    // If there are any extra meta values to add, add those
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4XMeta; c4Index++)
        colMeta.objAdd(colExtraMeta[c4Index]);

    THeapBuf mbufExt(8);
    WriteRawData
    (
        strRelPath
        , tCQCRemBrws::EDTypes::Template
        , c4NewSerialNum
        , colMeta
        , mbufTmpl
        , c4TmplBytes
        , TString::strEmpty()
        , mbufExt
        , 0
        , c4Flags
        , enctLastChange
        , sectUser
    );
}


// A type specific helper to send up a triggered event action
tCIDLib::TVoid
TDataSrvClient::WriteTriggeredEvent(const   TString&                strRelPath
                                    ,       tCIDLib::TCard4&        c4NewSerialNum
                                    ,       tCIDLib::TEncodedTime&  enctLastChange
                                    , const TCQCTrgEvent&           csrcToSend
                                    , const tCIDLib::TCard4         c4Flags
                                    , const tCIDLib::TKVPFList&     colExtraMeta
                                   , const TCQCSecToken&           sectUser)
{
    CheckReady();

    // Stream out the data to a stream
    TBinMBufOutStream strmTar(8 * 1024);
    strmTar << csrcToSend << kCIDLib::FlushIt;

    // Set up the paused metadata value
    const tCIDLib::TCard4 c4XMeta = colExtraMeta.c4ElemCount();
    tCIDLib::TKVPFList colMeta(1 + c4XMeta);
    colMeta.objPlace
    (
        kCQCRemBrws::strMetaKey_Paused, facCQCKit().strBoolVal(csrcToSend.bPaused())
    );

    // If there are any extra meta values to add, add those
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4XMeta; c4Index++)
        colMeta.objAdd(colExtraMeta[c4Index]);

    THeapBuf mbufExt(8);
    WriteRawData
    (
        strRelPath
        , tCQCRemBrws::EDTypes::TrgEvent
        , c4NewSerialNum
        , colMeta
        , strmTar.mbufData()
        , strmTar.c4CurSize()
        , TString::strEmpty()
        , mbufExt
        , 0
        , c4Flags
        , enctLastChange
        , sectUser
    );
}


// ---------------------------------------------------------------------------
//  TDataSrvClient: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  A generic file/chunk downloader, which uses the first/next scheme supported by the
//  ORB interface. The public methods above can call this to get an output stream filed
//  with the data. They can then create an input stream over that and stream in what
//  they know to be in it.
//
//  The return indicates whether new data was available or not. If the caller's serial
//  number is up to date, then the server doesn't return anything. If we return data,
//  we also update the caller's serial number.
//
//  The flags are in and out, though only the encrypted flag is currently the only
//  one that comes back. Others may later. On input, encrypted tells the server if
//  the caller is willing to accept an encrypted file or not.
//
//  To avoid being abusive we download larger files in successive blocks and add them
//  to the output stream. If it's small enough, we get it in one.
//
//  If there is an extension chunk, it is returned.
//
tCIDLib::TBoolean
TDataSrvClient::bReadRawData(const  TString&                strRelPath
                            , const tCQCRemBrws::EDTypes    eType
                            , CIOP  tCIDLib::TCard4&        c4SerialNum
                            ,       tCIDLib::TKVPFList&     colMetaVals
                            ,       TMemBuf&                mbufDataChunk
                            , COP   tCIDLib::TCard4&        c4DataBytes
                            ,       TString&                strExtChunkId
                            ,       TMemBuf&                mbufExtChunk
                            , COP   tCIDLib::TCard4&        c4ExtBytes
                            ,       tCIDLib::TCard4&        c4Flags
                            ,       tCIDLib::TEncodedTime&  enctLastChange
                            , const TCQCSecToken&           sectUser)
{
    #if defined(LOG_FILEACC)
    const tCIDLib::TCard4 c4OrgSerialNum = c4SerialNum;
    #endif

    // Make sure it doesn't have double slashes
    {
        tCIDLib::TCard4 c4At = 0;
        CIDAssert(!strRelPath.bFirstOccurrence(L"//", c4At), L"Read path has double slashes!");
    }

    // Convert the path to a full hierachical path
    TString strHPath;
    facCQCRemBrws().ToHPath(strRelPath, eType, strHPath);
    CheckIsHPath(strHPath);

    // We need a max block sized buffer for each subsequent round
    THeapBuf mbufBlock(kCQCRemBrws::c4DataBlockSz, kCQCRemBrws::c4DataBlockSz);

    //
    //  And another for any extension. We can't pass the caller's directly since it
    //  has to be a heap buffer specifically.
    //
    THeapBuf mbufExt(8, kCQCRemBrws::c4DataBlockSz);

    // Now loop until we get all of the file
    tCIDLib::TBoolean   bNewData = kCIDLib::False;
    tCIDLib::TCard4     c4Cookie;
    tCIDLib::TCard4     c4GotBytes = 0;

    //
    //  Ask for the first block, putting the bytes returned into the caller's param
    //  so that if it fits in one block we don't need to transfer.
    //
    bNewData = m_porbcDS->bQueryFileFirst
    (
        c4SerialNum
        , strHPath
        , c4Cookie
        , c4DataBytes
        , colMetaVals
        , c4GotBytes
        , mbufBlock
        , strExtChunkId
        , c4ExtBytes
        , mbufExt
        , c4Flags
        , enctLastChange
        , sectUser
    );

    #if defined(LOG_FILEACC)
    {
        TString strMsg(L"Read file with SN: ", 64UL);
        strMsg.AppendFormatted(c4OrgSerialNum);
        if (bNewData)
            strMsg.Append(L", got new data");
        else
            strMsg.Append(L", had latest data");
        strMsg.Append(L". New SN:");
        strMsg.AppendFormatted(c4SerialNum);
        strMsg.Append(L". Type=");
        strMsg.Append(tCQCRemBrws::strLoadEDTypes(eType));
        strMsg.Append(L", Encrypted=");
        strMsg.Append(facCQCKit().strBoolYesNo(c4Flags & kCQCRemBrws::c4Flag_Encrypted) != 0);
        facCQCRemBrws().LogMsg
        (
            CID_FILE
            , CID_LINE
            , strMsg
            , strRelPath
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
    }
    #endif

    // If we got new data, let's deal with that
    if (bNewData)
    {
        // Copy over any extension chunk if we got one
        if (!strExtChunkId.bIsEmpty())
            mbufExtChunk.CopyIn(mbufExt, c4ExtBytes);

        // Make sure the data chunk buffer is large enough
        if (mbufDataChunk.c4Size() < c4DataBytes)
            mbufDataChunk.Reallocate(c4DataBytes, kCIDLib::False);

        //
        //  Set up a stream over the data chunk buffer so we can easily add to it
        //  and write out the initial block we got.
        //
        TBinMBufOutStream strmTar(&mbufDataChunk);
        strmTar.c4WriteBuffer(mbufBlock, c4GotBytes);

        // If not just a single block, then we are done
        if (!(c4Flags & kCQCRemBrws::c4Flag_Done))
        {
            //
            //  It's too big for one call, so complete it a block at a time until
            //  we get it all.
            //
            tCIDLib::TBoolean bDone = kCIDLib::False;
            while (!bDone)
            {
                tCIDLib::TCard4 c4ThisTime = 0;
                bDone = m_porbcDS->bQueryFileNext(c4Cookie, c4ThisTime, mbufBlock);
                strmTar.c4WriteBuffer(mbufBlock, c4ThisTime);
                c4GotBytes += c4ThisTime;
            }
        }

        // Make sure the stream is flushed
        strmTar.Flush();
    }
    else
    {
        c4DataBytes = 0;
        c4ExtBytes = 0;
    }
    return bNewData;
}


//
//  Make sure the path, usually a type relative path configred to a full hierarchical
//  path, is one of our data server paths. If not, we throw.
//
tCIDLib::TVoid TDataSrvClient::CheckIsHPath(const TString& strToCheck) const
{
    if (!strToCheck.bStartsWith(L"/Customize"))
    {
        facCQCRemBrws().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kRBrwsErrs::errcDS_NotHPath
            , strToCheck
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::AppError
        );
    }
}


//
//  See if we have our client proxy. If not, try to create it. If we can't, then
//  throw. We get a quick test flag, which defaults to false, but some folks want to
//  give up immediately if the name server reports the data server not being there.
//  That's not likely but it helps prevent psycho scenarios from making things hang
//  up threads that don't want to pay that cost.
//
tCIDLib::TVoid TDataSrvClient::CheckReady(const tCIDLib::TBoolean bQuickTest)
{
    if (!m_porbcDS)
    {
        try
        {
            m_porbcDS = facCIDOrbUC().porbcMakeClient<TDataSrvAccClientProxy>
            (
                TDataSrvAccClientProxy::strBinding, 4000, bQuickTest
            );
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            throw;
        }
    }
}


//
//  A generic file uploader. The caller flattens out its type specific data and
//  calls this to upload it. We take an input stream that the caller creates over
//  the output stream he flattened the data to. The caller has to tell us how
//  many bytes since we can't get that from the base input stream class.
//
//  We could be sending a whole chunked file or a single chunk of the file (optionally
//  with metadata values.)
//
//  To avoid being abusive, we use a reasonable sized block for uploads. Anything
//  larger than that gets sent as multiple sequential blocks.
//
tCIDLib::TVoid
TDataSrvClient::WriteRawData(const  TString&                strRelPath
                            , const tCQCRemBrws::EDTypes    eType
                            ,       tCIDLib::TCard4&        c4NewSerialNum
                            , const tCIDLib::TKVPFList&     colMetaVals
                            , const TMemBuf&                mbufDataChunk
                            , const tCIDLib::TCard4         c4DataChunkSz
                            , const TString&                strExtChunkId
                            , const TMemBuf&                mbufExtChunk
                            , const tCIDLib::TCard4         c4ExtChunkSz
                            , const tCIDLib::TCard4         c4Flags
                            ,       tCIDLib::TEncodedTime&  enctLastChange
                            , const TCQCSecToken&           sectUser)
{
    // Make sure it doesn't have double slashes
    {
        tCIDLib::TCard4 c4At = 0;
        CIDAssert(!strRelPath.bFirstOccurrence(L"//", c4At), L"Write path has double slashes!");
    }

    //
    //  Convert the path to a full hierachical path.
    //
    //  NOTE that this will also compensate for any chance that a double slash got into
    //  a path. If that happens it will actually target the same path as without the
    //  slashes (since Windows ignores doubles) but it will get the path into the data
    //  cache under a different name, which is very bad.
    //
    TString strHPath;
    facCQCRemBrws().ToHPath(strRelPath, eType, strHPath);
    CheckIsHPath(strHPath);

    // We need a max block sized buffer for each round
    THeapBuf mbufBlock(kCQCRemBrws::c4DataBlockSz, kCQCRemBrws::c4DataBlockSz);

    // Set up an input stream over the memory buffer
    TBinMBufInStream strmData(&mbufDataChunk, c4DataChunkSz);

    // Read up to a block out of the stream
    tCIDLib::TCard4 c4ThisTime = strmData.c4ReadBuffer
    (
        mbufBlock
        , tCIDLib::MinVal(kCQCRemBrws::c4DataBlockSz, c4DataChunkSz)
        , tCIDLib::EAllData::OkIfNotAll
    );

    // If we can send it all in one, set the done flag
    tCIDLib::TBoolean bLast(c4DataChunkSz <= kCQCRemBrws::c4DataBlockSz);

    // Send the first block. Force the done flag if this is the last one
    tCIDLib::TCard4 c4Cookie;
    m_porbcDS->UploadFirst
    (
        strHPath
        , c4Cookie
        , c4NewSerialNum
        , c4DataChunkSz
        , colMetaVals
        , c4ThisTime
        , mbufBlock
        , strExtChunkId
        , c4ExtChunkSz
        , mbufExtChunk
        , c4Flags | (bLast ? kCQCRemBrws::c4Flag_Done : 0)
        , enctLastChange
        , sectUser
    );

    // It didn't fail. If not the last block continue until we write it all
    if (!bLast)
    {
        tCIDLib::TCard4 c4SoFar = c4ThisTime;
        while (c4SoFar < c4DataChunkSz)
        {
            // Read up to another chunk's worth
            c4ThisTime = c4DataChunkSz - c4SoFar;
            if (c4ThisTime > kCQCRemBrws::c4DataBlockSz)
                c4ThisTime = kCQCRemBrws::c4DataBlockSz;

            c4ThisTime = strmData.c4ReadBuffer
            (
                mbufBlock, c4ThisTime, tCIDLib::EAllData::OkIfNotAll
            );

            // And send it, telling him if this is the last one
            CIDAssert
            (
                (c4SoFar + c4ThisTime) <= c4DataChunkSz
                , L"Multi-block file upload exceeded original byte size"
            );

            bLast = (c4SoFar + c4ThisTime) == c4DataChunkSz;
            m_porbcDS->UploadNext
            (
                c4ThisTime
                , mbufBlock
                , c4Cookie
                , bLast
                , c4NewSerialNum
                , enctLastChange
            );

            c4SoFar += c4ThisTime;
        }
    }
}
