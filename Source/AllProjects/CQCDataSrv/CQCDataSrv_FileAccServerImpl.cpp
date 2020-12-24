//
// FILE NAME: CQCDataSrv_FileAccServerImpl.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/28/2015
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
//  This method implements the file access ORB interface, and some helper classes
//  it uses.
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
#include    "CIDZLib.hpp"
#include    "CQCDataSrv.hpp"



// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TDataSrvAccImpl, TDataSrvAccServerBase)


// Enable or disable some verbose logging to help diagnose issues
#if CID_DEBUG_ON
// #define LOG_FILEACC 1
#endif


// ---------------------------------------------------------------------------
//   CLASS: TDSCacheItem
//  PREFIX: dsci
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TDSCacheItem: Constructors and Destructor
// ---------------------------------------------------------------------------
TDSCacheItem::
TDSCacheItem(const  TString&                strScopeName
            , const tCQCRemBrws::EDTypes    eType
            , const tCIDLib::TCard4         c4ScopeId
            , const tCIDLib::TCard4         c4ItemId
            , const TString&                strHPath
            , const tCQCRemBrws::EItemFlags eFlags
            , const tCIDLib::TBoolean       bUserFlag) :

    m_bLoaded(kCIDLib::False)
    , m_dsbiCache
      (
         strScopeName
         , eType
         , c4ScopeId
         , c4ItemId
         , tCQCKit::EUserRoles::LimitedUser
         , eFlags
      )
    , m_enctLastAccess(0)
    , m_pchflData(nullptr)
    , m_strHPath(strHPath)
{
    m_dsbiCache.bUserFlag(bUserFlag);
}

TDSCacheItem::
TDSCacheItem(const  TString&                strFileName
            , const tCQCRemBrws::EDTypes    eType
            , const tCIDLib::TCard4         c4ItemId
            , const TString&                strHPath
            , const tCQCRemBrws::EItemFlags eFlags) :

    m_bLoaded(kCIDLib::False)
    , m_dsbiCache
      (
        strFileName, eType, 0, c4ItemId, tCQCKit::EUserRoles::SystemAdmin, eFlags
      )
    , m_enctLastAccess(0)
    , m_pchflData(nullptr)
    , m_strHPath(strHPath)
{
}

//
//  This will only get called to get the item into the collection, so we don't have to
//  worry about copying data cache info.
//
TDSCacheItem::TDSCacheItem(const TDSCacheItem& dsciSrc) :

    m_bLoaded(dsciSrc.m_bLoaded)
    , m_dsbiCache(dsciSrc.m_dsbiCache)
    , m_enctLastAccess(0)
    , m_pchflData(nullptr)
    , m_strHPath(dsciSrc.m_strHPath)
{
    // As per comments above, there should be no cached data yet
    CIDAssert(dsciSrc.m_pchflData == nullptr, L"Src cache item should not have cached data yet");
}

TDSCacheItem::~TDSCacheItem()
{
    // Clean up the chunked file data if we cached it
    try
    {
        delete m_pchflData;
        m_pchflData = nullptr;
    }

    catch(...)
    {
    }
}


// ---------------------------------------------------------------------------
//  TDSCacheItem: Public operators
// ---------------------------------------------------------------------------

//
//  This shouldn't actually get called though it might if we move an item in the cache.
//  We don't attempt to copy any data cache item. If need be, it will get re-cached
//  again later.
//
TDSCacheItem& TDSCacheItem::operator=(const TDSCacheItem& dsciSrc)
{
    if (this != &dsciSrc)
    {
        m_bLoaded   = dsciSrc.m_bLoaded;
        m_dsbiCache = dsciSrc.m_dsbiCache;
        m_strHPath = dsciSrc.m_strHPath;

        m_enctLastAccess = 0;

        // Release any current cache data
        delete m_pchflData;
        m_pchflData = nullptr;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TDSCacheItem: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TDSCacheItem::bChildItemsLoaded() const
{
    return m_bLoaded;
}

tCIDLib::TBoolean TDSCacheItem::bChildItemsLoaded(const tCIDLib::TBoolean bToSet)
{
    m_bLoaded = bToSet;
    return m_bLoaded;
}


// If we have an extension chunk return the info and true result, else false
tCIDLib::TBoolean
TDSCacheItem::bQueryExtChunk(TString&           strChunkId
                            , TMemBuf&          mbufData
                            , tCIDLib::TCard4&  c4DataSz) const
{
    // Throw if the data isn't cached yet
    CheckDataIsCached();

    // If only two then no extension chunk
    if (m_pchflData->c4ChunkCount() < 3)
    {
        strChunkId.Clear();
        c4DataSz = 0;
        return kCIDLib::False;
    }

    // Else we got one so give it back
    m_pchflData->QueryChunkAt(2, strChunkId, mbufData, c4DataSz);
    return kCIDLib::True;
}


tCIDLib::TBoolean
TDSCacheItem::bSetChunk(const   TString&        strChunkId
                        , const tCIDLib::TCard4 c4Bytes
                        , const TMemBuf&        mbufToSet)
{
    // Throw if the data isn't cached yet
    CheckDataIsCached();

    return m_pchflData->bSetChunkById(strChunkId, c4Bytes, mbufToSet);
}


tCIDLib::TBoolean
TDSCacheItem::bSetMetaVal(  const   TKeyValuePair&      kvalToSet
                            ,       tCIDLib::TBoolean&  bFileChange)
{
    // Throw if the data isn't cached yet
    CheckDataIsCached();

    return m_pchflData->bSetMetaValue
    (
        kvalToSet.strKey(), kvalToSet.strValue(), bFileChange
    );
}


tCIDLib::TBoolean TDSCacheItem::bUnsavedChanges() const
{
    // Throw if the data isn't cached yet
    CheckDataIsCached();

    return m_pchflData->bFileChanged();
}


// Set the last access flag to now
tCIDLib::TVoid TDSCacheItem::BumpLastAccess()
{
    m_enctLastAccess = TTime::enctNow();
}


tCIDLib::TCard4 TDSCacheItem::c4BumpItemId()
{
    return m_dsbiCache.c4BumpItemId();
}


tCIDLib::TCard4 TDSCacheItem::c4BumpScopeId()
{
    return m_dsbiCache.c4BumpScopeId();
}


// Give an estimage of the space required to hold our full file flattened
tCIDLib::TCard4 TDSCacheItem::c4FullSzEstimate() const
{
    // Throw if the data isn't cached yet
    CheckDataIsCached();

    return m_pchflData->c4FullSzEstimate();
}


tCIDLib::TCard4 TDSCacheItem::c4ReadChunk(const TString& strChunkId, TMemBuf& mbufToFill) const
{
    // Throw if the data isn't cached yet
    CheckDataIsCached();

    tCIDLib::TCard4 c4Bytes;
    const TMemBuf& mbufChunk = m_pchflData->mbufChunkById(strChunkId, c4Bytes);

    // If it didn't throw, we have it
    mbufToFill.CopyIn(mbufChunk, 0, c4Bytes);
    return c4Bytes;
}


//
//  Update the name and path of this item bump it's item id , returning the new
//  id.
//
tCIDLib::TCard4 TDSCacheItem::c4Rename(const TString& strName, const TString& strPath)
{
    m_strHPath = strPath;
    m_dsbiCache.strName(strName);
    return m_dsbiCache.c4BumpItemId();
}


tCIDLib::TCard4 TDSCacheItem::c4SerialNum() const
{
    // Throw if the data isn't cached yet
    CheckDataIsCached();

    return m_pchflData->c4SerialNum();
}


const TChunkedFile& TDSCacheItem::chflData() const
{
    // Throw if the data isn't cached yet
    CheckDataIsCached();

    return *m_pchflData;
}


tCIDLib::TEncodedTime TDSCacheItem::enctLastAccess() const
{
    return m_enctLastAccess;
}


tCIDLib::TEncodedTime TDSCacheItem::enctLastFileChange() const
{
    // Throw if the data isn't cached yet
    CheckDataIsCached();

    return m_pchflData->enctLastChange();
}


//
//  If we were created just for hierarhcical purposes initially, there may never be
//  any actual data loaded. But we need to get the extra metadata that we store in the
//  hierarchical info stored. So this is called to do that. We use a special helper on
//  the chunked file class that will read in just the header and meta info with
//  the least overhead possible.
//
tCIDLib::TVoid TDSCacheItem::ForceExtraMeta()
{
    #if defined(LOG_FILEACC)
    facCQCDataSrv.LogMsg
    (
        CID_FILE
        , CID_LINE
        , L"Forcing extra metadata load"
        , m_strHPath
        , tCIDLib::ESeverities::Status
        , tCIDLib::EErrClasses::AppStatus
    );
    #endif

    tCIDLib::TCard4         c4SerialNum;
    tCIDLib::TEncodedTime   enctLastChange;
    tCIDLib::TKVPFList      colMetaVals;
    tCIDLib::TCard4         c4ChunkCnt;

    TString strLocalPath;
    facCQCRemBrws().eConvertPath(m_strHPath, kCIDLib::False, strLocalPath, kCIDLib::True);
    TBinFileInStream strmSrc
    (
        strLocalPath
        , tCIDLib::ECreateActs::OpenIfExists
        , tCIDLib::EFilePerms::Default
        , tCIDLib::EFileFlags::SequentialScan
    );

    TChunkedFile::ExtractMetaInfo(strmSrc, c4SerialNum, enctLastChange, colMetaVals, c4ChunkCnt);
    StoreExtraMeta(colMetaVals);
}


//
//  The data cannot already be loaded. If so we throw. Else we will read in our chunked
//  file data from the file.
//
tCIDLib::TVoid TDSCacheItem::LoadFromFile()
{
    #if defined(LOG_FILEACC)
    facCQCDataSrv.LogMsg
    (
        CID_FILE
        , CID_LINE
        , L"Loading cache item data from chunked file"
        , m_strHPath
        , tCIDLib::ESeverities::Status
        , tCIDLib::EErrClasses::AppStatus
    );
    #endif

    // Make sure it's not already set
    if (m_pchflData != nullptr)
    {
        facCQCDataSrv.ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kDSrvErrs::errcFAcc_AlreadyCached
            , m_strHPath
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Already
        );
    }

    TChunkedFile* pchflNew = new TChunkedFile();
    TJanitor<TChunkedFile> janFile(pchflNew);

    // Get the local file system path for our data server path
    TString strLocalPath;
    facCQCRemBrws().eConvertPath(m_strHPath, kCIDLib::False, strLocalPath, kCIDLib::True);

    // Set up a stream and stream it in
    {
        TBinFileInStream strmSrc
        (
            strLocalPath
            , tCIDLib::ECreateActs::OpenIfExists
            , tCIDLib::EFilePerms::Default
            , tCIDLib::EFileFlags::SequentialScan
        );
        strmSrc >> *pchflNew;
    }

    // It worked so store it
    m_pchflData = janFile.pobjOrphan();

    // Remember the last time the data was accessed
    m_enctLastAccess = TTime::enctNow();

    // Update our separately stored metadata
    StoreExtraMeta(m_pchflData->colMetaValues());
}


// Provide acces sto a specific chunk's data
const TMemBuf&
TDSCacheItem::mbufForChunk(const TString& strChunkId, tCIDLib::TCard4& c4Bytes) const
{
    // Throw if the data isn't cached yet
    CheckDataIsCached();

    return m_pchflData->mbufChunkById(strChunkId, c4Bytes);
}


//
//  In order to support some stuff in clients that deal with the hierarchical info,
//  we need to keep some data ino the hierarchical info object that is actually
//  defined in the chunked file. The stuff we care about is stored as metadata values
//  so we don't have to understand the data format.
//
//  The server access object MUST update the metadata keys appropriately before calling
//  the UpdateXXX() methods, so that we store the right data. We cannot update the
//  chunked file metadata values ourself since we don't understand the file formats.
//
//  We take the metadata as a parameter because this is also called upon initially being
//  loaded for hierarchical purposes but no data yet cached. We still have to get the
//  extra metadata stored in the hierarchy data, so it's loaded directly and passed here.
//
tCIDLib::TVoid TDSCacheItem::StoreExtraMeta(const tCIDLib::TKVPFList& colVals)
{
    #if defined(LOG_FILEACC)
    facCQCDataSrv.LogMsg
    (
        CID_FILE
        , CID_LINE
        , L"Storing extra meta info"
        , m_strHPath
        , tCIDLib::ESeverities::Status
        , tCIDLib::EErrClasses::AppStatus
    );
    #endif

    //
    //  For event types, we keep the paused. FOr templates we keep the minimum role
    //  value.
    //
    switch(m_dsbiCache.eType())
    {
        case tCQCRemBrws::EDTypes::EvMonitor :
        case tCQCRemBrws::EDTypes::SchEvent :
        case tCQCRemBrws::EDTypes::TrgEvent :
        {
            const tCIDLib::TCard4 c4Index = TChunkedFile::c4FindMetaKey
            (
                colVals, kCQCRemBrws::strMetaKey_Paused
            );

            if (c4Index != kCIDLib::c4MaxCard)
            {
                m_dsbiCache.bUserFlag(colVals[c4Index].strValue().bCompareI(kCQCKit::pszFld_True));
            }
             else
            {
                //
                //  Something is bad, we don't want to give up, so we do the safe thing and
                //  set it to admin level. Better they have to come back and fix it than to
                //  make something available that shouldn't be.
                //
                facCQCDataSrv.LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kDSrvErrs::errcFAcc_BadMeta
                    , m_strHPath
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::AppError
                    , TString(L"paused")
                    , TString(L"event")
                );
            }
            break;
        }

        case tCQCRemBrws::EDTypes::Template :
        {
            // Set the minimum role
            const tCIDLib::TCard4 c4Index = TChunkedFile::c4FindMetaKey
            (
                colVals, kCQCRemBrws::strMetaKey_MinRole
            );
            tCQCKit::EUserRoles eMinRole(tCQCKit::EUserRoles::Count);
            if (c4Index != kCIDLib::c4MaxCard)
                eMinRole = tCQCKit::eAltXlatEUserRoles(colVals[c4Index].strValue());

            if (eMinRole == tCQCKit::EUserRoles::Count)
            {
                //
                //  Something is bad, we don't want to give up, so we do the safe thing and
                //  set it to admin level. Better they have to come back and fix it than to
                //  make something available that shouldn't be.
                //
                facCQCDataSrv.LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kDSrvErrs::errcFAcc_BadMeta
                    , m_strHPath
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::AppError
                    , TString(L"min role")
                    , TString(L"template")
                );
                eMinRole = tCQCKit::EUserRoles::SystemAdmin;
            }
            m_dsbiCache.eMinRole(eMinRole);
            break;
        }

        default :
            break;
    };
}



// If we want to drop our cached data this is called
tCIDLib::TVoid TDSCacheItem::UncacheData()
{
    #if defined(LOG_FILEACC)
    facCQCDataSrv.LogMsg
    (
        CID_FILE
        , CID_LINE
        , L"Uncaching cache item's data"
        , m_strHPath
        , tCIDLib::ESeverities::Status
        , tCIDLib::EErrClasses::AppStatus
    );
    #endif

    m_enctLastAccess = 0;
    delete m_pchflData;
    m_pchflData = nullptr;

    m_dsbiCache.bUserFlag(kCIDLib::False);
    m_dsbiCache.eMinRole(tCQCKit::EUserRoles::LimitedUser);
}


//
//  When a client uploads data for a file that already exists and is cached, we need to
//  update it in place. In this case we check that it is already loaded, unlike  the
//  SetData() methods above.
//
//  In this case, we aren't getting data from the file, so we need to write the new
//  data out.
//
//  We also have one to update just one chunk, which is commonly done for the main
//  data chunk.
//
//  The server access object MUST update the metadata keys appropriately before calling
//  these methods, so that we store the right extra metadata to the hierarchical info.
//  We cannot update the chunked file metadata values ourself since we don't understand
//  the file formats.
//
tCIDLib::TVoid
TDSCacheItem::UpdateChunk(  const   TString&            strChunkId
                            , const tCIDLib::TCard4     c4DataBytes
                            , const TMemBuf&            mbufDataChunk)
{
    #if defined(LOG_FILEACC)
    facCQCDataSrv.LogMsg
    (
        CID_FILE
        , CID_LINE
        , TString(L"Updating cache item chunk ", strChunkId)
        , m_strHPath
        , tCIDLib::ESeverities::Status
        , tCIDLib::EErrClasses::AppStatus
    );
    #endif

    // Throw if the data isn't cached yet
    CheckDataIsCached();

    try
    {
        m_pchflData->bSetChunkById(strChunkId, c4DataBytes, mbufDataChunk);
        WriteToFile();
    }

    catch(TError& errToCatch)
    {
        //
        //  We could have gotten half updated data, so let's uncache ourself, which
        //  will force it to be reloaded.
        //
        UncacheData();
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }

    // Remember the last time the data was updated
    m_enctLastAccess = TTime::enctNow();

    // Update our separately stored metadata if the data chunk was updated
    if (strChunkId.bCompareI(kCIDMData::strChunkId_Data))
        StoreExtraMeta(m_pchflData->colMetaValues());
}

tCIDLib::TVoid
TDSCacheItem::UpdateData(const  tCIDLib::TKVPFList& colMeta
                        , const tCIDLib::TCard4     c4DataBytes
                        , const TMemBuf&            mbufDataChunk
                        , const TString&            strExtChunkId
                        , const tCIDLib::TCard4     c4ExtBytes
                        , const TMemBuf&            mbufExtChunk)
{
    #if defined(LOG_FILEACC)
    facCQCDataSrv.LogMsg
    (
        CID_FILE
        , CID_LINE
        , L"Updating cache item full data"
        , m_strHPath
        , tCIDLib::ESeverities::Status
        , tCIDLib::EErrClasses::AppStatus
    );
    #endif

    // Throw if the data isn't cached yet
    CheckDataIsCached();

    try
    {
        // Update our chunked file object and write it out
        tCIDLib::TBoolean bFileChange;
        m_pchflData->bSetMetaValues(colMeta, bFileChange);
        m_pchflData->bSetChunkById(kCIDMData::strChunkId_Data, c4DataBytes, mbufDataChunk);
        if (!strExtChunkId.bIsEmpty())
            m_pchflData->bSetChunkById(strExtChunkId, c4ExtBytes, mbufExtChunk);
        WriteToFile();
    }

    catch(TError& errToCatch)
    {
        //
        //  We could have gotten half updated data, so let's uncache ourself, which
        //  will force it to be reloaded.
        //
        UncacheData();
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }

    // Remember the last time the data was updated
    m_enctLastAccess = TTime::enctNow();

    // Update our separately stored metadata
    StoreExtraMeta(m_pchflData->colMetaValues());
}




// ---------------------------------------------------------------------------
//  TDSCacheItem: Hidden constructors
// ---------------------------------------------------------------------------
TDSCacheItem::TDSCacheItem() :

    m_bLoaded(kCIDLib::False)
    , m_enctLastAccess(0)
    , m_pchflData(nullptr)
{
}


// ---------------------------------------------------------------------------
//  TDSCacheItem: Protected, non-virtual methods
// ---------------------------------------------------------------------------

// A lot of our methods are only valid if the data is already cached
tCIDLib::TVoid TDSCacheItem::CheckDataIsCached() const
{
    if (m_pchflData == nullptr)
    {
        facCQCDataSrv.ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kDSrvErrs::errcFAcc_NotCached
            , m_strHPath
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotReady
       );
    }
}


//
//  This we can do on our own. When we write the chunked file object, it will update
//  itself to indicate it no longer has any unsaved changes. This is called any time
//  our chunked file data is updated (not set, which happens when we are being set from
//  the current file contents.)
//
tCIDLib::TVoid TDSCacheItem::WriteToFile()
{
    #if defined(LOG_FILEACC)
    facCQCDataSrv.LogMsg
    (
        CID_FILE
        , CID_LINE
        , L"Writing cache item to file"
        , m_strHPath
        , tCIDLib::ESeverities::Status
        , tCIDLib::EErrClasses::AppStatus
    );
    #endif

    // Throw if the data isn't cached yet
    CheckDataIsCached();

    // Get the local file system path for our data server path
    TString strLocalPath;
    facCQCRemBrws().eConvertPath(m_strHPath, kCIDLib::False, strLocalPath, kCIDLib::True);

    // Convert that into a temp file path and write it out
    TString strTempPath(strLocalPath);
    strTempPath.Append(L"_New");
    {
        TBinFileOutStream strmOut
        (
            strTempPath
            , tCIDLib::ECreateActs::CreateAlways
            , tCIDLib::EFilePerms::Default
            , tCIDLib::EFileFlags::SequentialScan
        );
        strmOut << *m_pchflData << kCIDLib::FlushIt;
    }

    // The stream is closed now so swap it to the final position
    TFileSys::ReplaceFile(strLocalPath, strTempPath);
}




// ---------------------------------------------------------------------------
//   CLASS: TDataSrvAccImpl::TTransOp
//  PREFIX: top
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TDataSrvAccImpl::TTransOp: Constructors and destructor
// ---------------------------------------------------------------------------

// For upload
TDataSrvAccImpl::TTransOp::
TTransOp(const  tCIDLib::TCard4     c4Cookie
        , const TString&            strHPath
        , const tCIDLib::TCard4     c4DataChunkSz
        , const tCIDLib::TKVPFList& colMetaVals
        , const tCIDLib::TCard4     c4FirstBlockSz
        , const TMemBuf&            mbufFirstBlock
        , const TString&            strExtChunkId
        , const tCIDLib::TCard4     c4ExtSz
        , const TMemBuf&            mbufExt
        , const tCIDLib::TCard4     c4Flags) :

    m_bUpload(kCIDLib::True)
    , m_c4Cookie(c4Cookie)
    , m_c4ExtSz(c4ExtSz)
    , m_c4Flags(c4Flags)
    , m_c4FullSz(c4DataChunkSz)
    , m_c4NextOfs(c4FirstBlockSz)
    , m_colMetaUp(colMetaVals)
    , m_enctEndTime(TTime::enctNowPlusSecs(5))
    , m_mbufData(c4DataChunkSz, c4DataChunkSz)
    , m_mbufExtChunk(c4ExtSz + 1, c4ExtSz + 1)
    , m_strExtChunkId(strExtChunkId)
    , m_strHPath(strHPath)
{
    //  Copy in the initial chunk's worth of data and any extension buffer
    m_mbufData.CopyIn(mbufFirstBlock, c4FirstBlockSz);
    if (c4ExtSz)
        m_mbufExtChunk.CopyIn(mbufExt, c4ExtSz);
}

// For download, no extension chunk here it was returned in the initial reply
TDataSrvAccImpl::TTransOp::
TTransOp(const  tCIDLib::TCard4     c4Cookie
        , const TString&            strHPath
        , const tCIDLib::TCard4     c4DataChunkSz
        , const tCIDLib::TCard4     c4FirstBlockSz
        , const TMemBuf&            mbufDataChunk
        , const tCIDLib::TCard4     c4Flags) :

    m_bUpload(kCIDLib::False)
    , m_c4Cookie(c4Cookie)
    , m_c4ExtSz(0)
    , m_c4Flags(c4Flags)
    , m_c4FullSz(c4DataChunkSz)
    , m_c4NextOfs(c4FirstBlockSz)
    , m_enctEndTime(TTime::enctNowPlusSecs(5))
    , m_mbufData(c4DataChunkSz, c4DataChunkSz)
    , m_strHPath(strHPath)
{
    //
    //  For a download, we need to keep the whole file contents. WE can't go back to
    //  the original, since it might change while we are downloading.
    //
    m_mbufData.CopyIn(mbufDataChunk, c4DataChunkSz);
}

TDataSrvAccImpl::TTransOp::~TTransOp()
{
}




// ---------------------------------------------------------------------------
//   CLASS: TDataSrvAccImpl
//  PREFIX: orbs
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TDataSrvAccImpl: Constructors and Destructor
// ---------------------------------------------------------------------------
TDataSrvAccImpl::TDataSrvAccImpl(const tCIDLib::TCard4 c4DSCacheSz) :

    m_c4NextCookie(1)
    , m_c4NextItemId(1)
    , m_c4NextScopeId(1)
    , m_colTransOps(tCIDLib::EAdoptOpts::Adopt)
    , m_colTreeCache(tCIDLib::EMTStates::Unsafe, kCIDLib::False)
{
}

TDataSrvAccImpl::~TDataSrvAccImpl()
{
}


// ---------------------------------------------------------------------------
//  TDataSrvAccImpl: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  We don't bother with the hierarchical cache here, since all we care about is
//  getting file system info that is not cached anyway. That also means no locking
//  is required here.
//
tCIDLib::TBoolean
TDataSrvAccImpl::bFind( const   TString&            strOrgHPath
                        ,       TFindBuf&           fndbToFill
                        , const tCIDLib::TBoolean   bIsScope)
{
    // Fix up the path if required (slashes, double slashes, etc...)
    TString strPath;
    facCQCKit().PrepRemBrwsPath(strOrgHPath, strPath);

    //
    //  Convert the path to a local path. We don't care what the type is, so the
    //  return doesn't matter. It will throw if it's a malformed path.
    //
    TString strLocalPath;
    facCQCRemBrws().eConvertPath(strPath, bIsScope, strLocalPath, kCIDLib::True);

    return TFileSys::bExists
    (
        strLocalPath
        , fndbToFill
        , bIsScope ? tCIDLib::EDirSearchFlags::NormalDirs : tCIDLib::EDirSearchFlags::NormalFiles
    );
}


//
//  Checks to see if a file exists. We'll check the tree cache first and if it's in
//  there fine. Else we look for the file.
//
tCIDLib::TBoolean
TDataSrvAccImpl::bExists(const  TString&            strOrgHPath
                        , const tCIDLib::TBoolean   bIsScope)
{
    // Fix up the path if required (slashes, double slashes, etc...)
    TString strHPath;
    facCQCKit().PrepRemBrwsPath(strOrgHPath, strHPath);

    TLocker lockrSync(&m_mtxSync);

    tCIDLib::ETreeNodes eNodeType;
    if (m_colTreeCache.bNodeExists(strHPath, eNodeType))
        return kCIDLib::True;

    // Not cached so check the file itself
    TString strLocalPath;
    facCQCRemBrws().eConvertPath(strHPath, bIsScope, strLocalPath, kCIDLib::True);

    return TFileSys::bExists
    (
        strLocalPath
        , bIsScope ? tCIDLib::EDirSearchFlags::NormalDirs : tCIDLib::EDirSearchFlags::NormalFiles
    );
}


//
//  We don't bother with the cache here, since all we care about is getting file
//  system info that is not cached anyway. That also means no locking is required here.
//
tCIDLib::TBoolean TDataSrvAccImpl::bIsScope(const TString& strOrgHToCheck)
{
    // Fix up the path if required (slashes, double slashes, etc...)
    TString strHToCheck;
    facCQCKit().PrepRemBrwsPath(strOrgHToCheck, strHToCheck);

    //
    //  Convert the path to a local path. We don't care what the type is, so the
    //  return doesn't matter. Tell it is a scope so it won't add any extension.
    //  It will throw if it's a malformed path.
    //
    TString strLocalPath;
    facCQCRemBrws().eConvertPath(strHToCheck, kCIDLib::True, strLocalPath, kCIDLib::True);

    // And check the file system with the local path
    return TFileSys::bIsDirectory(strLocalPath);
}


//
//  Let's the client get just the extension chunk by itself, which may be much more
//  efficient in some cases, such as reading a thumbnail without having to read the
//  full image if it is not needed. Note that nocache only means the data, we can
//  always cache the hierarchy info.
//
tCIDLib::TBoolean
TDataSrvAccImpl::bQueryExtChunk(        tCIDLib::TCard4&    c4SerialNum
                                , const TString&            strOrgHPath
                                , const TString&            strExtChunkId
                                ,       tCIDLib::TCard4&    c4BufSz
                                ,       THeapBuf&           mbufExtChunk
                                , const TCQCSecToken&       sectUser
                                , const tCIDLib::TBoolean   bNoCache)
{
    // Do the prep-work and get the path info from the incoming HPath
    TString strHPath;
    TString strLocalPath;
    const tCQCRemBrws::EDTypes eType = eCheckParms
    (
        sectUser, strOrgHPath, kCIDLib::False, strHPath, strLocalPath
    );

    // We need to lock for the rest of the way
    TLocker lockrSync(&m_mtxSync);

    //
    //  Get the hierarchy data cached if not yet. We pass along cache flag so it will
    //  cache if the caller says we can.
    //
    tCIDLib::TBoolean bNewlyCached = !bNoCache;
    TDSCacheItem* pdsciSrc = pdsciUpdateHierCache(strHPath, kCIDLib::False, bNewlyCached);

    // Update the last access stamp
    pdsciSrc->BumpLastAccess();

    //
    //  If the data is not loaded yet, then we are obviously not allowed to, else it
    //  would have happened above, so do it from a temp load of the file and return.
    //
    if (!pdsciSrc->bDataLoaded())
    {
        TChunkedFile* pchflTmp = pchflLoadFile(strLocalPath);
        TJanitor<TChunkedFile> janFile(pchflTmp);

        c4BufSz = pchflTmp->c4QueryChunkById(strExtChunkId, mbufExtChunk);
        c4SerialNum = pchflTmp->c4SerialNum();
        return kCIDLib::True;
    }

    //
    //  We do have data. If they have the correct serial number, and we didn't have to
    //  load it, then tell them they have the right info. If we did have to load it, we
    //  just want to be safe and return data. It's possible we somehow dropped the
    //  entry and are now re-loading it, but more likely it's a bug so let's try to
    //  correct it.
    //
    if ((pdsciSrc->c4SerialNum() == c4SerialNum) && !bNewlyCached)
        return kCIDLib::False;

    // Else give them back the latest data
    pdsciSrc->c4ReadChunk(strExtChunkId, mbufExtChunk);
    c4SerialNum = pdsciSrc->c4SerialNum();
    return kCIDLib::True;
}


//
//  Starts a file transfer operation. If not in the cache we will force it in if
//  not prevented from doing so by the flags.
//
tCIDLib::TBoolean
TDataSrvAccImpl::bQueryFileFirst(       tCIDLib::TCard4&        c4SerialNum
                                , const  TString&               strOrgHPath
                                ,       tCIDLib::TCard4&        c4Cookie
                                ,       tCIDLib::TCard4&        c4DataBytes
                                ,       tCIDLib::TKVPFList&     colMeta
                                ,       tCIDLib::TCard4&        c4FirstBlockSz
                                ,       THeapBuf&               mbufFirstBlock
                                ,       TString&                strExtChunkId
                                ,       tCIDLib::TCard4&        c4ExtChunkSz
                                ,       THeapBuf&               mbufExtChunk
                                ,       tCIDLib::TCard4&        c4Flags
                                ,       tCIDLib::TEncodedTime&  enctLastChange
                                , const TCQCSecToken&           sectUser)
{
    #if defined(LOG_FILEACC)
    {
        TString strMsg
        (
            L"Starting download. Client SN: %(1)", TCardinal(c4SerialNum)
        );
        facCQCDataSrv.LogMsg
        (
            CID_FILE
            , CID_LINE
            , strMsg
            , strOrgHPath
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
    }
    #endif

    // Do the prep-work and get the path info from the incoming HPath
    TString strHPath;
    TString strLocalPath;
    TString strRelPath;
    const tCQCRemBrws::EDTypes eType = eCheckParms
    (
        sectUser, strOrgHPath, kCIDLib::False, strHPath, strRelPath, strLocalPath
    );

    // Make sure we don't stream back bogus data
    c4FirstBlockSz = 0;
    c4ExtChunkSz = 0;
    colMeta.RemoveAll();


    // We need to lock for the rest of the way
    TLocker lockrSync(&m_mtxSync);

    const tCIDLib::TBoolean bCanCache((c4Flags & kCQCRemBrws::c4Flag_NoDataCache) == 0);
    const TChunkedFile* pchflData = nullptr;

    // If already cached or we can cache it, then we will work from that cached one
    {
        //
        //  Get at least the hiearchical info on the file loaded if not already. If the
        //  local file doesn't exist, this will throw. If caching it allowed, tell it to
        //  do so. Otherwise he'll do a partial load of the required meta info and we'll
        //  turn around and load the full thing here.
        //
        tCIDLib::TBoolean bNewlyCached = bCanCache;
        TDSCacheItem* pdsciSrc = pdsciUpdateHierCache(strHPath, kCIDLib::False, bNewlyCached);

        //  If the data is got cached, then get a pointer to the file data
        if (pdsciSrc->bDataLoaded())
            pchflData = &pdsciSrc->chflData();

        // Update the last access stamp
        pdsciSrc->BumpLastAccess();
    }

    // If not set yet, then we can't cache it, so we use a temp
    TJanitor<const TChunkedFile> janFile(nullptr);
    if (!pchflData)
    {
        pchflData = pchflLoadFile(strLocalPath);
        janFile.Set(pchflData);
    }

    //
    //  If the caller's serial number is the same as the current one, then return
    //  false to indicate no new data.
    //
    if (pchflData->c4SerialNum() == c4SerialNum)
    {
        #if defined(LOG_FILEACC)
        {
            facCQCDataSrv.LogMsg
            (
                CID_FILE
                , CID_LINE
                , L"Caller already has most recent data"
                , strHPath
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );
        }
        #endif
        return kCIDLib::False;
    }
     else
    {
        #if defined(LOG_FILEACC)
        {
            TString strMsg
            (
                L"S/N changed (%(1)), returning new data", TCardinal(pchflData->c4SerialNum())
            );
            facCQCDataSrv.LogMsg
            (
                CID_FILE
                , CID_LINE
                , strMsg
                , strHPath
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );
        }
        #endif
    }

    //
    //  If there's an extension chunk for this file, give that back. If not, the id
    //  and size will be zero and nothing will be streamed back for this. The caller
    //  call tell us not to download it. In some cases they may not need it and this
    //  saves transferring useless bytes.
    //
    c4ExtChunkSz = 0;
    strExtChunkId.Clear();
    if (!(c4Flags & kCQCRemBrws::c4Flag_NoExtChunk))
    {
        if (pchflData->c4ChunkCount() > 2)
            pchflData->QueryChunkAt(2, strExtChunkId, mbufExtChunk, c4ExtChunkSz);
    }

    //
    //  Get a convenience ref to the data chunk, and this sets up the caller's data
    //  bytes output parameter.
    //
    const TMemBuf& mbufData = pchflData->mbufChunkById(kCIDMData::strChunkId_Data, c4DataBytes);

    // Let's give him up to a chunk's worth of data
    const tCIDLib::TBoolean bLast = c4DataBytes <= kCQCRemBrws::c4DataBlockSz;
    if (bLast)
        c4FirstBlockSz = c4DataBytes;
    else
        c4FirstBlockSz = kCQCRemBrws::c4DataBlockSz;
    mbufFirstBlock.CopyIn(mbufData, c4FirstBlockSz);

    // If not the last one, then set up a transfer op
    if (!bLast)
    {
        //
        //  We need to put into the transfer list. This will copy the data into
        //  the transfer object for subequent use, and remember that we need to
        //  pick up next time at c4BufSz bytes. No extension or meta, since we
        //  will return those from this call.
        //
        TTransOp* ptopNew = ptopGetFreeDn
        (
            strHPath, c4DataBytes, c4FirstBlockSz, mbufData, c4Flags
        );

        // Give the caller back the cookie
        c4Cookie = ptopNew->m_c4Cookie;

        #if defined(LOG_FILEACC)
        {
            TString strMsg(L"Downloading multiple blocks, cookie=");
            strMsg.AppendFormatted(c4Cookie);
            strMsg.Append(L", Bytes=");
            strMsg.AppendFormatted(c4DataBytes);
            strMsg.Append(L", ExtChunkId=");
            strMsg.Append(strExtChunkId);
            facCQCDataSrv.LogMsg
            (
                CID_FILE
                , CID_LINE
                , strMsg
                , strHPath
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );
        }
        #endif

        // Make sure the done flag is not set
        c4Flags &= ~kCQCRemBrws::c4Flag_Done;
    }
     else
    {
        #if defined(LOG_FILEACC)
        facCQCDataSrv.LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"File fits in one chunk, download complete"
            , strHPath
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
        #endif

        // Make sure the done flag is set
        c4Flags |= kCQCRemBrws::c4Flag_Done;
    }

    // Give back the meta values
    pchflData->chflchMeta().QueryValues(colMeta);

    // Give them back the current serial number and last change
    c4SerialNum = pchflData->c4SerialNum();
    enctLastChange = pchflData->enctLastChange();

    #if defined(LOG_FILEACC)
    // DumpCache();
    #endif

    // We are returning data, though perhaps not completed yet
    return kCIDLib::True;
}


tCIDLib::TBoolean
TDataSrvAccImpl::bQueryFileNext(const   tCIDLib::TCard4     c4Cookie
                                ,       tCIDLib::TCard4&    c4BufSz
                                ,       THeapBuf&           mbufData)
{
    TLocker lockrSync(&m_mtxSync);

    //
    //  Find the transfer op for this cookie. This also validates that it's not
    //  timed out.
    //
    TTransOp* ptopTrans = ptopForCookie(c4Cookie);

    // Give him up to another chunk
    c4BufSz = ptopTrans->m_c4FullSz - ptopTrans->m_c4NextOfs;
    const tCIDLib::TBoolean bLast(c4BufSz <= kCQCRemBrws::c4DataBlockSz);
    if (!bLast)
        c4BufSz = kCQCRemBrws::c4DataBlockSz;

    ptopTrans->m_mbufData.CopyOut(mbufData, c4BufSz, ptopTrans->m_c4NextOfs);
    ptopTrans->m_c4NextOfs += c4BufSz;

    // If the last, we can remove this transfer op
    if (bLast)
    {
        #if defined(LOG_FILEACC)
        {
            TString strMsg(L"Final block transferred, cookie=");
            strMsg.AppendFormatted(ptopTrans->m_c4Cookie);
            strMsg.Append(L", ExtChunkId=");
            strMsg.Append(ptopTrans->m_strExtChunkId);
            facCQCDataSrv.LogMsg
            (
                CID_FILE
                , CID_LINE
                , strMsg
                , ptopTrans->m_strHPath
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );
        }
        #endif
        m_colTransOps.RemoveElem(ptopTrans);
    }
    return bLast;
}


//
//  These are for special scenarios, where the client is wanting info about a file
//  maybe for later download of some of it, or to get information for reports and
//  such.
//
//  If the file is in the hierarchy cache and the data has been cached up, then we
//  use that. Else we just read it from the file and don't cache the data. The chunked
//  file class provides a helper to read this data with minimal overhead.
//
tCIDLib::TBoolean
TDataSrvAccImpl::bQueryItemMeta(const   TString&                strOrgHPath
                                ,       tCIDLib::TKVPFList&     colMetaVals
                                ,       tCIDLib::TStrList&      colChunkIds
                                ,       tCIDLib::TCardList&     fcolChunkSizes
                                ,       tCIDLib::TCard4&        c4SerialNum
                                ,       tCIDLib::TEncodedTime&  enctLastChange
                                , const tCIDLib::TBoolean       bThrowIfNot
                                , const TCQCSecToken&           sectUser)
{
    #if defined(LOG_FILEACC)
    facCQCDataSrv.LogMsg
    (
        CID_FILE
        , CID_LINE
        , L"Returning item meta info"
        , strOrgHPath
        , tCIDLib::ESeverities::Status
        , tCIDLib::EErrClasses::AppStatus
    );
    #endif

    // Make sure we don't stream back bogus data
    colChunkIds.RemoveAll();
    fcolChunkSizes.RemoveAll();

    // Do the prep-work and get the path info from the incoming HPath
    TString strHPath;
    TString strLocalPath;
    TString strRelPath;
    const tCQCRemBrws::EDTypes eType = eCheckParms
    (
        sectUser
        , strOrgHPath
        , kCIDLib::False
        , strHPath
        , strRelPath
        , strLocalPath
        , tCQCKit::EUserRoles::LimitedUser
    );

    // If the file doesn't exist, then either throw or return false
    if (!TFileSys::bExists(strLocalPath, tCIDLib::EDirSearchFlags::NormalFiles))
    {
        if (bThrowIfNot)
        {
            facCQCDataSrv.ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kDSrvErrs::errcFAcc_FileNotFound
                , strOrgHPath
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::NotFound
            );
        }
        return kCIDLib::False;
    }

    // And now we need to lock for the rest
    TLocker lockrSync(&m_mtxSync);

    // See if we have this guy in the cache and make sure it's a file, not a scope
    TTreeCache::TNode* pnodeFl = m_colTreeCache.pnodeAt(strHPath);
    if (pnodeFl)
        CheckTreeNodeType(strHPath, pnodeFl, kCIDLib::False);

    if (pnodeFl && pnodeFl->objData().bDataLoaded())
    {
        TDSCacheItem& dsdciQ = pnodeFl->objData();
        dsdciQ.chflData().QueryChunkSizes(colChunkIds, fcolChunkSizes);
        dsdciQ.chflData().chflchMeta().QueryValues(colMetaVals);
        c4SerialNum = dsdciQ.c4SerialNum();
        enctLastChange = dsdciQ.enctLastFileChange();
    }
     else
    {
        #if defined(LOG_FILEACC)
        facCQCDataSrv.LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"Data was not cached, loading it from the file"
            , strHPath
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
        #endif

        TBinFileInStream strmSrc
        (
            strLocalPath
            , tCIDLib::ECreateActs::OpenIfExists
            , tCIDLib::EFilePerms::Default
            , tCIDLib::EFileFlags::SequentialScan
        );
        TChunkedFile::ExtractInfo
        (
            strmSrc
            , c4SerialNum
            , enctLastChange
            , colMetaVals
            , colChunkIds
            , fcolChunkSizes
        );
    }
    return kCIDLib::True;
}


//
//  This one is for lower overhead access to names of of items and/or scopes within a given
//  scope. So we are just returning names, not all of the browser item info and such,
//  or the paths, just the names of the things relative to the incoming scope.
//
tCIDLib::TBoolean
TDataSrvAccImpl::bQueryScopeNames(          tCIDLib::TCard4&        c4ScopeId
                                    , const TString&                strOrgHPath
                                    ,       tCIDLib::TStrList&      colToFill
                                    , const tCQCRemBrws::EQSFlags   eQSFlags
                                    , const TCQCSecToken&           sectUser)
{
    #if defined(LOG_FILEACC)
    {
        facCQCDataSrv.LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"Client queried scope item names"
            , strOrgHPath
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
    }
    #endif

    // Clear out the output list
    colToFill.RemoveAll();


    // Do the prep-work and get the path info from the incoming HPath
    TString strHPath;
    TString strLocalPath;
    const tCQCRemBrws::EDTypes eType = eCheckParms
    (
        sectUser, strOrgHPath, kCIDLib::True, strHPath, strLocalPath
    );

    // And now we need to lock for the rest
    TLocker lockrSync(&m_mtxSync);

    //
    //  See if it's already in the cache. If not, we need to fault it in. If it is
    //  but its contents have not been faulted in yet, then do that.
    //
    TTreeCache::TNodeNT* pnodePar = nullptr;
    {
        TTreeCache::TNode* pnodeTest = m_colTreeCache.pnodeAt(strHPath);

        // Make sure it's a scope if found and cast it if so
        if (pnodeTest)
        {
            CheckTreeNodeType(strHPath, pnodeTest, kCIDLib::True);
            pnodePar = static_cast<TTreeCache::TNodeNT*>(pnodeTest);
        }
    }

    //
    //  Watch for the special case of the /Customize, which is our top level scope
    //  and is always already in the cache and doesn't map to a specific data type
    //  or to any actual files. So we don't want to try to fault it in.
    //
    if (!strHPath.bCompareI(kCQCRemBrws::strPath_Customize))
    {
        // If not present or it's contents not loaded yet, then fault it in
        if (!pnodePar || !pnodePar->objData().bChildItemsLoaded())
        {
            TFindBuf fndbTest;
            if (!TFileSys::bExists(strLocalPath, fndbTest))
            {
                facCQCDataSrv.ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kDSrvErrs::errcFAcc_ScopeNotFound
                    , strHPath
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::NotFound
                );

                // Make the analyzer happy
                return kCIDLib::False;
            }

            if (!fndbTest.bIsDirectory())
            {
                facCQCDataSrv.ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kDSrvErrs::errcFAcc_WrongType
                    , strHPath
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::TypeMatch
                );

                // Make the analyzer happy
                return kCIDLib::False;
            }

            pnodePar = pnodeFaultInScope(strHPath, kCIDLib::True, eType, strLocalPath);
            CIDAssert(pnodePar != nullptr, L"Failed to fault in scope");
        }
    }

    if (!pnodePar)
    {
        CIDAssert2(L"Parent node never got set");

        // Make the analyzer happy
        return kCIDLib::False;
    }


    //
    //  If the caller alread has the latest version, return false. He already has the
    //  latest stuff.
    //
    const tCIDLib::TCard4 c4CurScopeId = pnodePar->objData().dsbiCache().c4IdScope();
    if (c4CurScopeId == c4ScopeId)
        return kCIDLib::False;

    // Give him back the latest id
    c4ScopeId = c4CurScopeId;

    //
    //  Now let's iterate the children of the scope and load them into the caller's
    //  list.
    //
    TTreeCache::TNode* pnodeCur = pnodePar->pnodeFirstChild();
    while (pnodeCur)
    {
        const TDSBrowseItem& dsbiCur = pnodeCur->objData().dsbiCache();

        if (dsbiCur.bIsScope())
        {
            if (tCIDLib::bAllBitsOn(eQSFlags, tCQCRemBrws::EQSFlags::Scopes))
                colToFill.objAdd(dsbiCur.strName());
        }
         else
        {
            if (tCIDLib::bAllBitsOn(eQSFlags, tCQCRemBrws::EQSFlags::Items))
                colToFill.objAdd(dsbiCur.strName());
        }

        pnodeCur = pnodeCur->pnodeNext();
    }

    // And indicate we are returning new data
    return kCIDLib::True;
}


//
//  Delete a target path. if it's a scope, we remove the scope and all of its
//  contents. If a file, we just remove the file.
//
tCIDLib::TVoid
TDataSrvAccImpl::DeletePath(const   TString&            strOrgHPath
                            , const tCIDLib::TBoolean   bIsScope
                            , const tCIDLib::TBoolean   bMustExist
                            , const TCQCSecToken&       sectUser)
{
    #if defined(LOG_FILEACC)
    {
        facCQCDataSrv.LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"Deleting path"
            , strOrgHPath
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
    }
    // DumpCache();
    #endif

    // Do the prep-work and get the path info from the incoming HPath
    TString strHPath;
    TString strLocalPath;
    const tCQCRemBrws::EDTypes eType = eCheckParms
    (
        sectUser
        , strOrgHPath
        , bIsScope
        , strHPath
        , strLocalPath
        , tCQCKit::EUserRoles::SystemAdmin
    );

    // We need to lock for the rest of the way
    TLocker lockrSync(&m_mtxSync);


    // See if this path is in the hierarchy and valid it's the right type
    tCIDLib::ETreeNodes eNodeType;
    const tCIDLib::TBoolean bInTree = m_colTreeCache.bNodeExists(strHPath, eNodeType);
    if (bInTree)
    {
        CIDAssert
        (
            eNodeType == (bIsScope ? tCIDLib::ETreeNodes::NonTerminal : tCIDLib::ETreeNodes::Terminal)
            , L"The delete path was accessed as a different node type than cached"
        );
    }

    //
    //  Remove it from the hierarchy cache if present. This is the same whether a
    //  scope or file. If a scope, it will automatically remove all of the contained
    //  stuff.
    //
    TTreeCache::TNodeNT* pnodePar = nullptr;
    if (bInTree)
    {
        #if defined(LOG_FILEACC)
        {
            facCQCDataSrv.LogMsg
            (
                CID_FILE
                , CID_LINE
                , L"Removing path from tree cache"
                , strHPath
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );
        }
        #endif
        m_colTreeCache.RemoveNode(strHPath);

        // Get the parent scope and bump its scope id since we changed its contents
        TTreeCache::TNodeNT* pnodePar = pnodeCheckParScope
        (
            strHPath, strLocalPath, eType, EFaultInOpts::Never
        );
        if (pnodePar)
            pnodePar->objData().c4BumpScopeId();
    }


    // Now see if the local file exists
    const tCIDLib::TBoolean bExists = TFileSys::bExists
    (
        strLocalPath
        , bIsScope ? tCIDLib::EDirSearchFlags::NormalDirs : tCIDLib::EDirSearchFlags::NormalFiles
    );

    // If not, then we can either just do nothing or throw
    if (!bExists)
    {
        //
        //  If they don't care if it already exists, then we are done. We already
        //  removed it from the cache, so just return.
        //
        if (!bMustExist)
            return;

        // Else we need to throw
        facCQCDataSrv.ThrowErr
        (
            CID_FILE
            , CID_LINE
            , bIsScope ? kDSrvErrs::errcFAcc_ScopeNotFound : kDSrvErrs::errcFAcc_FileNotFound
            , strLocalPath
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
        );
    }

    //
    //  If it's a file, it's the simpler one. Else we have to do potentially a lot
    //  more work for a scope.
    //
    if (bIsScope)
    {
        //
        //  Remove the tree content from the local file system. As always, be very
        //  careful when doing this kind of thing. Make absolutely sure the path
        //  starts with the local file system prefix for the target data type.
        //
        TString strBasePath;
        facCQCRemBrws().BasePathForType(eType, strBasePath);
        if (strBasePath.bIsEmpty() || !strLocalPath.bStartsWithI(strBasePath))
        {
            facCQCRemBrws().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kDSrvErrs::errcFAcc_NotUnderBasePath
                , strLocalPath
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Internal
            );
        }

        #if defined(LOG_FILEACC)
        {
            facCQCDataSrv.LogMsg
            (
                CID_FILE
                , CID_LINE
                , L"Removing local path"
                , strLocalPath
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );
        }
        #endif

        TFileSys::RemovePath(strLocalPath);
    }
     else
    {
        TFileSys::DeleteFile(strLocalPath);
    }

    #if defined(LOG_FILEACC)
    //DumpCache();
    #endif
}


//
//  This method will rename an item under a given parent scope. We make sure the
//  scope is faulted in, which it must already be.
//
//  We make sure that the source does exist and that the target doesn't exist and
//  throw if not. We update the scope id of the parent scope and the item id of the
//  item.
//
//  We have a special case to deal wtih when they are just trying to change the case
//  of some of the characters but its otherwise the same. We do need to rename the
//  file, and update the names as cached, but other work normally required is not
//  needed or wanted.
//
tCIDLib::ERenameRes
TDataSrvAccImpl::eRenameItem(const  TString&            strOrgParScope
                            , const TString&            strOrgOldName
                            , const TString&            strOrgNewName
                            , const tCIDLib::TBoolean   bIsScope
                            ,       tCIDLib::TCard4&    c4ParScopeId
                            ,       tCIDLib::TCard4&    c4ItemId
                            , const TCQCSecToken&       sectUser)
{
    #if defined(LOG_FILEACC)
    {
        TString strMsg(L"Renaming item. Path=");
        strMsg.Append(strOrgParScope);
        strMsg.Append(L", OrgName=");
        strMsg.Append(strOrgOldName);
        strMsg.Append(L", NewName=");
        strMsg.Append(strOrgNewName);
        facCQCDataSrv.LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"Renaming item"
            , strMsg
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
    }
    #endif

    // Requires system admin user privs
    CheckUserRole(sectUser, tCQCKit::EUserRoles::SystemAdmin);

    // Fix up the path if required (slashes, double slashes, etc...)
    TString strParScope;
    facCQCKit().PrepRemBrwsPath(strOrgParScope, strParScope);
    TString strOldName;
    facCQCKit().PrepRemBrwsPath(strOrgOldName, strOldName);
    TString strNewName;
    facCQCKit().PrepRemBrwsPath(strOrgNewName, strNewName);

    // The parent scope must already be in the hierarchy cache so grab that
    TTreeCache::TNodeNT* pnodePar
    (
        static_cast<TTreeCache::TNodeNT*>(m_colTreeCache.pnodeAt(strParScope))
    );

    // Create full DS paths for the old and new paths
    TString strDSOldPath(strParScope);
    TString strDSNewPath(strParScope);
    facCQCRemBrws().AddPathComp(strDSOldPath, strOldName);
    facCQCRemBrws().AddPathComp(strDSNewPath, strNewName);

    //
    //  See if it's just a case rename. If they are not equal case sensitively but they
    //  are the same insensitively, then it's just a change in case.
    //
    const tCIDLib::TBoolean bCaseOnly
    (
        (strDSOldPath.eCompare(strDSNewPath) != tCIDLib::ESortComps::Equal)
        && (strDSOldPath.eCompareI(strDSNewPath) == tCIDLib::ESortComps::Equal)
    );

    TLocker lockrSync(&m_mtxSync);

    // Create local path versions of them
    TString strLocalOldPath;
    TString strLocalNewPath;
    const tCQCRemBrws::EDTypes eType = eConvertPath
    (
        strDSOldPath, bIsScope, strLocalOldPath, kCIDLib::True
    );
    eConvertPath(strDSNewPath, bIsScope, strLocalNewPath, kCIDLib::True);

    //
    //  If the parent scope is a system scope, then disallow this. The caller should
    //  not do this, but make sure. Though it can be done if the system editing flag is
    //  enabled.
    //
    if (facCQCRemBrws().bIsSystemScope(strParScope, eType) && !facCQCKit().bSysEdit())
    {
        facCQCDataSrv.ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kDSrvErrs::errcFAcc_NoSystemMods
            , strParScope
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Authority
        );
    }

    //
    //  Make sure the source exists. If it's not a case only rename, make sure that the
    //  target does not.
    //
    {
        const tCIDLib::EDirSearchFlags eFlags
        (
            bIsScope ? tCIDLib::EDirSearchFlags::NormalDirs : tCIDLib::EDirSearchFlags::NormalFiles
        );

        if (!TFileSys::bExists(strLocalOldPath, eFlags))
        {
            facCQCDataSrv.ThrowErr
            (
                CID_FILE
                , CID_LINE
                , bIsScope ? kDSrvErrs::errcFAcc_ScopeNotFound
                           : kDSrvErrs::errcFAcc_FileNotFound
                , strLocalOldPath
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::NotFound
            );
        }

        if (!bCaseOnly && TFileSys::bExists(strLocalNewPath, eFlags))
        {
            facCQCDataSrv.ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kDSrvErrs::errcFAcc_TarExists
                , strLocalNewPath
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Already
            );
        }
    }

    // Do the file system level rename
    TFileSys::Rename(strLocalOldPath, strLocalNewPath);

    // Bump the parent scope's scope id and give the new id back
    c4ParScopeId = pnodePar->objData().dsbiCache().c4BumpScopeId();

    //
    //  We need to update the target item under the parent. First update our
    //  user data. Then we need to update the actual node name.
    //
    TTreeCache::TNode* pnodeItem = pnodePar->pnodeFind(strOldName, kCIDLib::False);
    c4ItemId = pnodeItem->objData().c4Rename(strNewName, strDSNewPath);
    pnodeItem->strName(strNewName);

    return tCIDLib::ERenameRes::NotFound;
}


//
//  This method will do a full report on all of the data types in one shot.
//
tCIDLib::TVoid
TDataSrvAccImpl::GenerateReport(const   TString&                    strDescr
                                , const tCQCRemBrws::EDTypeList&    fcolTypes
                                ,       tCIDLib::TCard4&            c4OutBytes
                                ,       THeapBuf&                   mbufOut
                                , const TCQCSecToken&               sectUser)
{
    #if defined(LOG_FILEACC)
    {
        facCQCDataSrv.LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"Starting multi-type report generation"
            , strDescr
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
    }
    #endif

    // Just in case, don't stream back bogus data if we give up
    c4OutBytes = 0;

    // Make sure the security token is valid and they are at least a power user
    CheckUserRole(sectUser, tCQCKit::EUserRoles::PowerUser);

    //
    //  Set up an output stream text stream over a memory buffer, setting a UTF-8
    //  converter on it, and we set the XML decl to indicate that encoding. We don't
    //  use the caller's buffer since we still have to compress it.
    //
    THeapBuf mbufText(0x10000, 0x100000);
    TPathStr pathH;
    TPathStr pathLocal;
    TTextMBufOutStream strmTar(&mbufText, tCIDLib::EAdoptOpts::NoAdopt, new TUTF8Converter);

    // Format out the common header info
    FormatReportHdr(strmTar, strDescr, sectUser);

    // Set up a time object for formatting
    TTime tmFmt;
    tmFmt.strDefaultFormat(TTime::strMMDD_24HHMMSS());

    // Now we have to lock while we do the actual data access
    {
        TLocker lockrSync(&m_mtxSync);

        // Iterate the file types and output each one to our stream
        const tCIDLib::TCard4 c4TypeCnt = fcolTypes.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4TypeCnt; c4Index++)
        {
            const tCQCRemBrws::EDTypes eCurType = fcolTypes[c4Index];
            strmTar << L"<FileType Type=\""
                    << tCQCRemBrws::strAltXlat2EDTypes(eCurType) << L"\">"
                    << L"<Scope Name=\"User\">";

            //
            //  Generate the start (root) paths for this type and call the helper. We
            //  always recurse in this case.
            //
            facCQCRemBrws().CreateTypePaths(eCurType, pathH, pathLocal);
            GenerateReport(strmTar, eCurType, pathH, pathLocal, kCIDLib::True, tmFmt);
            strmTar << L"</Scope></FileType>";
        }
    }

    // Close off the overall report now
    strmTar << L"</Report>\n" << kCIDLib::FlushIt;

    // Create an input stream over the output we just filled in
    TBinMBufInStream strmSrc(&mbufText, strmTar.c4CurSize());

    // And one over the caller's buffer to write out
    TBinMBufOutStream strmComp(&mbufOut);

    // Now let's compress it to the caller's buffer
    TZLibCompressor zlibDecomp;
    c4OutBytes = zlibDecomp.c4Compress(strmSrc, strmComp);

    CIDAssert
    (
        c4OutBytes == strmComp.c4CurSize()
        , L"Compressed bytes != to output stream's reported size"
    );
}


//
//  This will generate a report on files of a specifc type, in a given scope (optionally
//  recursively under it.) The data is XML and is returned in a ZLib compressed buffer.
//
//  We don't force stuff into the hierarchical or data caches.
//
tCIDLib::TVoid
TDataSrvAccImpl::GenerateTypeReport(const   TString&            strDescr
                                    , const   TString&          strOrgHScope
                                    , const tCIDLib::TBoolean   bRecurse
                                    ,       tCIDLib::TCard4&    c4OutBytes
                                    ,       THeapBuf&           mbufOut
                                    , const TCQCSecToken&       sectUser)
{
    #if defined(LOG_FILEACC)
    {
        facCQCDataSrv.LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"Starting single-type report generation"
            , strDescr
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
    }
    #endif


    // Just in case, don't stream back bogus data if we give up
    c4OutBytes = 0;

    // Make sure the security token is valid and they are at least a power user
    CheckUserRole(sectUser, tCQCKit::EUserRoles::PowerUser);

    // Check the hierarchical path and get the data type and local path
    TPathStr pathHPath;
    TString strRelPath;
    TPathStr pathLocalPath;
    const tCQCRemBrws::EDTypes eType = eCheckParms
    (
        sectUser
        , strOrgHScope
        , kCIDLib::True
        , pathHPath
        , strRelPath
        , pathLocalPath
        , tCQCKit::EUserRoles::PowerUser
    );

    // Set up an output buffer stream, which we'll compress into the caller's buffer later
    THeapBuf mbufText(0x10000, 0x100000);
    TTextMBufOutStream strmTar(&mbufText, tCIDLib::EAdoptOpts::NoAdopt, new TUTF8Converter);

    // Format out the common header info
    FormatReportHdr(strmTar, strDescr, sectUser);

    //
    //  And the initial file type and starting scope elements. We just dipslay
    //  the type relative path since we are showing the type explicitly.
    //
    strmTar << L"<FileType Type=\"" << tCQCRemBrws::strAltXlat2EDTypes(eType)
            << L"\"><Scope Name=\"" << strRelPath << L"\">";

    // Set up a time object for formatting
    TTime tmFmt;
    tmFmt.strDefaultFormat(TTime::strMMDD_24HHMMSS());

    // Now we have to lock while we access the data
    {
        TLocker lockrSync(&m_mtxSync);
        GenerateReport(strmTar, eType, pathHPath, pathLocalPath, bRecurse, tmFmt);
    }

    // Close off the elements we started
    strmTar << L"</Scope></FileType></Report>\n" << kCIDLib::FlushIt;

    // Create an input stream over the output we just filled in
    TBinMBufInStream strmSrc(&mbufText, strmTar.c4CurSize());

    // And one over the caller's buffer to write out
    TBinMBufOutStream strmComp(&mbufOut);

    // Now let's compress it to the caller's buffer
    TZLibCompressor zlibDecomp;
    c4OutBytes = zlibDecomp.c4Compress(strmSrc, strmComp);

    CIDAssert
    (
        c4OutBytes == strmComp.c4CurSize()
        , L"Compressed bytes != to output stream's reported size"
    );
}


// Create a new scope under the passed parent scope
tCIDLib::TVoid
TDataSrvAccImpl::MakeNewScope(  const   TString&        strOrgParScope
                                , const TString&        strOrgNewSubScope
                                , const TCQCSecToken&   sectUser)
{
    // Fix up the path if required (slashes, double slashes, etc...)
    TString strParScope;
    facCQCKit().PrepRemBrwsPath(strOrgParScope, strParScope);
    TString strNewSubScope;
    facCQCKit().PrepRemBrwsPath(strOrgNewSubScope, strNewSubScope);

    // Just build up the full path and pass it on to MakePath
    TString strFull(strParScope);
    facCQCRemBrws().AddPathComp(strFull, strNewSubScope);
    MakePath(strFull, sectUser);
}


tCIDLib::TVoid
TDataSrvAccImpl::MakePath(const TString& strOrgHPath, const TCQCSecToken& sectUser)
{
    #if defined(LOG_FILEACC)
    {
        facCQCDataSrv.LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"Making new path"
            , strOrgHPath
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
    }
    #endif

    // Do the prep-work and get the path info from the incoming HPath
    TString strHPath;
    TString strLocalPath;
    const tCQCRemBrws::EDTypes eType = eCheckParms
    (
        sectUser
        , strOrgHPath
        , kCIDLib::True
        , strHPath
        , strLocalPath
        , tCQCKit::EUserRoles::SystemAdmin
    );

    // Lock for the rest
    TLocker lockrSync(&m_mtxSync);

    //
    //  Create the passed path. It must a scope. We work our way down the list of
    //  path components, checking each one, and create each one that isn't present.
    //
    //  For the one where we start actually adding, if that one's parent is currently
    //  in the cache, we need to bump its scope it do indicate its contents has
    //  changed.
    //
    //  So start by seeing if the target is already in the cache. If so, then there
    //  is nothing to do.
    //
    TTreeCache::TNode* pnodeCur = m_colTreeCache.pnodeAt(strHPath);
    if (pnodeCur)
    {
        // It must be a scope path
        if (pnodeCur->eType() != tCIDLib::ETreeNodes::NonTerminal)
        {
            facCQCDataSrv.ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kDSrvErrs::errcFAcc_WrongType
                , strHPath
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::TypeMatch
            );
        }

        // We are good
        return;
    }

    // Create the local path components
    TFileSys::MakePath(strLocalPath);

    //
    //  And now fault in the target DS path into the tree cache, which should now
    //  map correctly to the local path we just created. We know the local path was
    //  created successfully.
    //
    pnodeFaultInScope(strHPath, kCIDLib::False, eType, strLocalPath);
}


//
//  The client is asking us to copy the contents of the source path to the target path.
//  The source may be a single item or a scope. It's far more efficient for us to do this
//  here so that it all becomes just local file system operations.
//
//  It doesn't affect the source and the new copies cannot have been accessed, so there are
//  no data or hierarchy cache concerns at all. We won't fault any of this stuff into the
//  hierarchy cache unless it's later accessed.
//
//  Though the client should not ever send us two paths within different data type scopes,
//  we have to check for that. Other than that, we mostly don't care about data type since
//  we are just moving raw files. But we do need it to call a couple of our own public
//  methods to do some work.
//
//  The client is supposed to set up the target path to be a unique name in the target
//  scope. If the source is a scope it will become the top level scope that we copy to.
//  If the source is an item, the target is the name of the new item.
//
//  So we check to see if the target exists. If so, we throw an error.
//
tCIDLib::TVoid
TDataSrvAccImpl::PastePath( const   TString&            strOrgSrcPath
                            , const TString&            strOrgTarPath
                            , const tCIDLib::TBoolean   bSrcIsScope
                            , const TCQCSecToken&       sectUser)
{
    // Make sure the security token is valid and they are an admin
    CheckUserRole(sectUser, tCQCKit::EUserRoles::SystemAdmin);

    // Fix up the path if required (slashes, double slashes, etc...)
    TString strSrcPath;
    facCQCKit().PrepRemBrwsPath(strOrgSrcPath, strSrcPath);
    TString strTarPath;
    facCQCKit().PrepRemBrwsPath(strOrgTarPath, strTarPath);

    TLocker lockrSync(&m_mtxSync);

    // The caller tells us if the source is a file or scope
    TPathStr pathLSrcPath;
    const tCQCRemBrws::EDTypes eSrcType = facCQCRemBrws().eConvertPath
    (
        strSrcPath, bSrcIsScope, pathLSrcPath, kCIDLib::True
    );

    // We know the target has to be the same
    TPathStr pathLTarPath;
    const tCQCRemBrws::EDTypes eTarType = facCQCRemBrws().eConvertPath
    (
        strTarPath, bSrcIsScope, pathLTarPath, kCIDLib::True
    );

    // If not the same type, then bad
    if (eSrcType != eTarType)
    {
        facCQCDataSrv.ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kDSrvErrs::errcFAcc_SrcTarTypes
            , strSrcPath
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::TypeMatch
        );
    }

    // If the target exists, then bad
    if (TFileSys::bExists(pathLTarPath))
    {
        facCQCDataSrv.ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kDSrvErrs::errcFAcc_TarExists
            , strTarPath
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::TypeMatch
        );
    }

    //
    //  Just to be absolutely safe, see if the target is in the hierarchy cache. If
    //  so and the file doesn't exist, that's bad.
    //
    {
        TDSCacheItem* pdsciTar = m_colTreeCache.pobjAt(strTarPath);
        if (pdsciTar)
        {
            facCQCDataSrv.ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kDSrvErrs::errcFAcc_TarExists2
                , strTarPath
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::TypeMatch
            );
        }
    }

    try
    {
        //
        //  See if the source is at least in the hierarchy cache. If not, load it. If
        //  a file and it has to be loaded, tell it not to cache teh data if not already
        //  cached.
        //
        TDSCacheItem* pdsciSrc = m_colTreeCache.pobjAt(strSrcPath);
        if (!pdsciSrc)
        {
            tCIDLib::TBoolean bCanCache = kCIDLib::False;
            pdsciSrc = pdsciUpdateHierCache(strSrcPath, bSrcIsScope, bCanCache);
        }

        if (bSrcIsScope)
        {
            //
            //  Call a recursive helper to do this. It in turns calls a single
            //  file copying helper, which is what we use for the single file
            //  senario below. But first we have to create the top level target
            //  path which we know doesn't exist.
            //
            TFileSys::MakeDirectory(pathLTarPath);
            CopyDir(pathLSrcPath, pathLTarPath);
        }
         else
        {
            // Just call a file copier helper
            CopyFile(pathLSrcPath, pathLTarPath);
        }

        //
        //  Now, if the target parent is loaded already, we have to get this new one into
        //  his part of the hierarchy, or it'll never try to load it again. Note that, if we
        //  are pasting in a scope, we don't have to worry about the other stuff. This new
        //  child scope we added is not marked as loaded yet, so the other stuff will get
        //  faulted in. We just need to this starting one in place if the parent is already
        //  loaded.
        //
        TString strParScope(strTarPath);
        facCQCRemBrws().bRemoveLastPathItem(strParScope);
        TTreeCache::TNodeNT* pnodeTarPar = static_cast<TTreeCache::TNodeNT*>
        (
            m_colTreeCache.pnodeAt(strParScope)
        );
        if (pnodeTarPar)
        {
            //
            //  Create a new hiearchy cache item for this file. The user/system type
            //  of the parent node indicates what flags we set on the item.
            //
            tCQCRemBrws::EItemFlags eFlags;

            TString strNamePart;
            facCQCRemBrws().QueryNamePart(strTarPath, strNamePart);

            // And add it to the parent
            if (bSrcIsScope)
            {
                // It may have changed from system to user or vice versa
                if (facCQCRemBrws().bIsSystemScope(strTarPath, eTarType))
                    eFlags = tCQCRemBrws::EItemFlags::SystemScope;
                else
                    eFlags = tCQCRemBrws::EItemFlags::UserScope;

                TDSCacheItem dsciNew
                (
                    strNamePart
                    , pdsciSrc->eType()
                    , m_c4NextScopeId++
                    , m_c4NextItemId++
                    , strTarPath
                    , eFlags
                    , kCIDLib::False
                );

                TTreeCache::TNodeNT* pnodeNew = pnodeTarPar->pnodeAddNonTerminal
                (
                    strNamePart, TString::strEmpty(), kCIDLib::False, kCIDLib::True
                );
                pnodeNew->objData(dsciNew);
            }
             else
            {
                // It may have changed from system to user or vice versa
                if (facCQCRemBrws().bIsSystemScope(strTarPath, eTarType))
                    eFlags = tCQCRemBrws::EItemFlags::SystemItem;
                else
                    eFlags = tCQCRemBrws::EItemFlags::UserItem;

                TDSCacheItem dsciNew
                (
                    strNamePart
                    , pdsciSrc->eType()
                    , m_c4NextItemId++
                    , strTarPath
                    , eFlags
                );

                // Copy over the source's min role and user flag stuff
                dsciNew.dsbiCache().eMinRole(pdsciSrc->dsbiCache().eMinRole());
                dsciNew.dsbiCache().bUserFlag(pdsciSrc->dsbiCache().bUserFlag());

                pnodeTarPar->objAddTerminal
                (
                    dsciNew
                    , strNamePart
                    , TString::strEmpty()
                    , kCIDLib::False
                    , kCIDLib::True
                );
            }

            //
            //  We adding something to the parent, so bump his scope id to indicate his
            //  contents have chagned.
            //
            pnodeTarPar->objData().c4BumpScopeId();
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}


// Nothing to do. Just being here is all that counts
tCIDLib::TVoid TDataSrvAccImpl::Ping()
{
}


//
//  Query the item info for the indicated path. It must already have been faulted
//  into the cache. Generally it's used to get the root item info.
//
tCIDLib::TVoid
TDataSrvAccImpl::QueryItemInfo( const   TString&            strOrgHPath
                                ,       tCIDLib::TBoolean&  bIsScope
                                ,       tCIDLib::TCardList& fcolPathIds
                                ,       TDSBrowseItem&      dsbiToFill
                                , const TCQCSecToken&       sectUser)
{
    // Do the prep-work and get the path info from the incoming HPath
    TString strHPath;
    TString strLocalPath;
    CheckParms(sectUser, strOrgHPath, bIsScope, strHPath);

    // And now we need to lock for the rest
    TLocker lockrSync(&m_mtxSync);

    // Make sure it's cached, throw if not
    TTreeCache::TNode* pnodePath = m_colTreeCache.pnodeAt(strHPath);
    if (!pnodePath)
    {
        facCQCDataSrv.ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kDSrvErrs::errcFAcc_NotCached
            , strHPath
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
        );

        // Make the analyzer happy
        return;
    }

    // Tell the caller if it's a scope or not
    bIsScope = (pnodePath->eType() == tCIDLib::ETreeNodes::NonTerminal);

    // Build up the list of ids that represent this scope
    BuildIdPath(pnodePath, fcolPathIds, bIsScope);

    // And give back the browser info
    dsbiToFill = pnodePath->objData().dsbiCache();
}


tCIDLib::TVoid
TDataSrvAccImpl::QueryItemInfo2(const   TString&            strOrgHPath
                                ,       tCIDLib::TBoolean&  bIsScope
                                ,       tCIDLib::TCardList& fcolPathIds
                                ,       TDSBrowseItem&      dsbiTarget
                                ,       TDSBrowseItem&      dsbiParScope
                                , const TCQCSecToken&       sectUser)
{
    // Do the prep-work and get the path info from the incoming HPath
    TString strHPath;
    CheckParms(sectUser, strOrgHPath, bIsScope, strHPath);

    // And now we need to lock for the rest
    TLocker lockrSync(&m_mtxSync);

    // Make sure it's cached, throw if not
    TTreeCache::TNode* pnodePath = m_colTreeCache.pnodeAt(strHPath);
    if (!pnodePath)
    {
        facCQCDataSrv.ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kDSrvErrs::errcFAcc_NotCached
            , strHPath
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
        );

        // Make the analyzer happy
        return;
    }

    // Tell the caller if it's a scope or not
    bIsScope = (pnodePath->eType() == tCIDLib::ETreeNodes::NonTerminal);

    // Build up the list of ids that represent this scope
    BuildIdPath(pnodePath, fcolPathIds, bIsScope);

    // And give back the browser info
    dsbiTarget = pnodePath->objData().dsbiCache();

    // Let's get the parent scope now
    TTreeCache::TNode* pnodePar = pnodePath->pnodeParent();
    dsbiParScope = pnodePar->objData().dsbiCache();
}


//
//  Supports the hiearchy browser, to let a client get the ids associated with
//  a particular path.
//
tCIDLib::TVoid
TDataSrvAccImpl::QueryPathIds(  const   TString&            strOrgHPath
                                ,       tCIDLib::TBoolean&  bIsScope
                                ,       tCIDLib::TCard4&    c4ItemId
                                ,       tCIDLib::TCard4&    c4ScopeId
                                , const TCQCSecToken&       sectUser)
{
    #if defined(LOG_FILEACC)
    {
        facCQCDataSrv.LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"Client queried path ids"
            , strOrgHPath
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
    }
    #endif

    // Do the prep-work and get the path info from the incoming HPath
    TString strHPath;
    CheckParms(sectUser, strOrgHPath, bIsScope, strHPath);

    // And now we need to lock for the rest
    TLocker lockrSync(&m_mtxSync);

    TTreeCache::TNode* pnodePath = m_colTreeCache.pnodeAt(strHPath);
    if (!pnodePath)
    {
        facCQCDataSrv.ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kDSrvErrs::errcFAcc_NotCached
            , strHPath
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
        );

        // Make the analyzer happy
        return;
    }

    const TDSBrowseItem& dsbiQ = pnodePath->objData().dsbiCache();
    bIsScope = dsbiQ.bIsScope();
    c4ItemId = dsbiQ.c4IdItem();

    if (bIsScope)
        c4ScopeId = dsbiQ.c4IdScope();
    else
        c4ScopeId = 0;
}


//
//  Supports the hiearchy browser. Gets the ids fore each path component along a
//  given path.
//
tCIDLib::TVoid
TDataSrvAccImpl::QueryPathIds(  const   TString&            strOrgHPath
                                ,       tCIDLib::TBoolean&  bIsScope
                                ,       tCIDLib::TCardList& fcolPathIds
                                , const TCQCSecToken&       sectUser)
{
    #if defined(LOG_FILEACC)
    {
        facCQCDataSrv.LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"Client queried id path"
            , strOrgHPath
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
    }
    #endif

    // Do the prep-work and get the path info from the incoming HPath
    TString strHPath;
    CheckParms(sectUser, strOrgHPath, bIsScope, strHPath);

    // And now we need to lock for the rest
    TLocker lockrSync(&m_mtxSync);

    TTreeCache::TNode* pnodePath = m_colTreeCache.pnodeAt(strHPath);
    if (!pnodePath)
    {
        facCQCDataSrv.ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kDSrvErrs::errcFAcc_NotCached
            , strHPath
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
        );

        // Make the analyzer happy
        return;
    }

    // Build up the list of ids that represent this scope
    BuildIdPath(pnodePath, fcolPathIds, kCIDLib::True);
}



//
//  This method will return the items in the passed path, which must be a scope.
//  If it isn't in the cache, enough of the tree will be faulted in to get to the
//  target, then its contents will be loaded.
//
//  They tell us if they want us to only return items, or items and sub-scopes
//
tCIDLib::TVoid
TDataSrvAccImpl::QueryScopeItems(const  TString&                strOrgHPath
                                ,       tCIDLib::TCardList&     fcolToFill
                                ,       TVector<TDSBrowseItem>& colToFill
                                , const tCIDLib::TBoolean       bItemsOnly
                                , const TCQCSecToken&           sectUser)
{
    #if defined(LOG_FILEACC)
    {
        facCQCDataSrv.LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"Client queried scope items"
            , strOrgHPath
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
    }
    #endif

    // Clear out the two lists so we can't stream back any bogus data
    colToFill.RemoveAll();
    fcolToFill.RemoveAll();

    // Do the prep-work and get the path info from the incoming HPath
    TString strHPath;
    CheckParms(sectUser, strOrgHPath, kCIDLib::True, strHPath);

    // And now we need to lock for the rest
    TLocker lockrSync(&m_mtxSync);

    //
    //  See if it's already in the cache. If not, we need to fault it in. If it is
    //  but its contents have not been faulted in yet, then do that.
    //
    TTreeCache::TNodeNT* pnodePar = nullptr;
    {
        TTreeCache::TNode* pnodeTest = m_colTreeCache.pnodeAt(strHPath);

        // If it's there, make sure it represents a scoe
        if (pnodeTest && (pnodeTest->eType() != tCIDLib::ETreeNodes::NonTerminal))
        {
            facCQCDataSrv.ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kDSrvErrs::errcFAcc_WrongType
                , strHPath
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::TypeMatch
            );

            // Make the analyzer happy
            return;
        }

        // It's a NT, so cast it to the actual type
        pnodePar = static_cast<TTreeCache::TNodeNT*>(pnodeTest);
    }

    //
    //  Watch for the special case of the /Customize, which is our top level scope
    //  and is always already in the cache and doesn't map to a specific data type
    //  or to any actual files. So we don't want to try to fault it in.
    //
    if (!strHPath.bCompareI(kCQCRemBrws::strPath_Customize))
    {
        // If not present or it's contents not loaded yet, then fault it in
        if (!pnodePar || !pnodePar->objData().bChildItemsLoaded())
        {
            // Convert the path to a local path
            TString strLocalPath;
            const tCQCRemBrws::EDTypes  eType = eConvertPath
            (
                strHPath, kCIDLib::True, strLocalPath, kCIDLib::True
            );

            if (!TFileSys::bIsDirectory(strLocalPath))
            {
                facCQCDataSrv.ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kDSrvErrs::errcFAcc_WrongType
                    , strHPath
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::TypeMatch
                );

                // Make the analyzer happy
                return;
            }

            pnodePar = pnodeFaultInScope(strHPath, kCIDLib::True, eType, strLocalPath);
            CIDAssert(pnodePar != nullptr, L"Failed to fault in scope");
        }
    }

    if (!pnodePar)
    {
        CIDAssert2(L"Parent node never got set");

        // Make the analyzer happy
        return;
    }

    // Build up the list of ids that represent this scope
    BuildIdPath(pnodePar, fcolToFill, kCIDLib::True);

    //
    //  Now let's iterate the children of the scope and load them into the caller's
    //  list.
    //
    TTreeCache::TNode* pnodeCur = pnodePar->pnodeFirstChild();
    while (pnodeCur)
    {
        const TDSBrowseItem& dsbiCur = pnodeCur->objData().dsbiCache();

        // Don't add scopes if they only want items
        if (!bItemsOnly || !dsbiCur.bIsScope())
            colToFill.objAdd(dsbiCur);

        pnodeCur = pnodeCur->pnodeNext();
    }
}


//
//  For some things like the XML Gateway, to let them get a hierarchy of available
//  thises or thats. We return it in a simple XML tree format. We don't cache the
//  info here, since it would fault in a lot of stuff that would otherwise not
//  likely be accessed.
//
//  As with all of the methods here we return relative paths. SO THIS MEANS that this
//  can only be called with a starting scope no higher than the top level scope for
//  a given data type, since we have to return paths relative to that.
//
//  Data type here means the tCQCRemBrws::EDTypes list.
//
tCIDLib::TVoid
TDataSrvAccImpl::QueryTree( const   TString&            strOrgStartScope
                            ,       TString&            strXML
                            , const tCIDLib::TBoolean   bSparse
                            , const TCQCSecToken&       sectUser)
{
    // Make sure the security token is valid
    CheckSecToken(sectUser);

    // Fix up the path if required (slashes, double slashes, etc...)
    TString strStartScope;
    facCQCKit().PrepRemBrwsPath(strOrgStartScope, strStartScope);

    // <TBD>
}


tCIDLib::TVoid
TDataSrvAccImpl::QueryTree2(const   TString&            strOrgStartScope
                            ,       tCIDLib::TStrList&  colToFill
                            , const tCIDLib::TBoolean   bSparse
                            , const TCQCSecToken&       sectUser)
{
    // Do the prep-work and get the path info from the incoming HPath
    TString strStartScope;
    CheckParms(sectUser, strOrgStartScope, kCIDLib::True, strStartScope);

    //
    //  We use a simple stack based scheme to virtual recurse through the hierarchy.
    //  The incoming path may have a trailing slash, so remove it if so.
    //
    TStack<TString> colStack;
    TString& strStart = colStack.objPush(strStartScope);
    if (strStart.chLast() == kCIDLib::chForwardSlash)
        strStart.DeleteLast();

    // Get the data type of the passed path and insure it's a valid one at the same time
    const tCQCRemBrws::EDTypes eDType
    (
        facCQCRemBrws().eXlatPathType(strStart, kCIDLib::True)
    );

    // And now we need to lock for the rest
    TLocker lockrSync(&m_mtxSync);

    //
    //  Now let's check the incoming scope to make sure it's loaded. Third parm only
    //  applies to files so just pass false.
    //
    tCIDLib::TBoolean bCanCache = kCIDLib::False;
    pdsciUpdateHierCache(strStart, kCIDLib::True, bCanCache);

    TString strCurScope;
    TString strChild;
    TString strLocalPath;
    TString strRelPath;
    TDataSrvAccImpl::TTreeCache::TNodeNT* pnodeCurScope;
    while (!colStack.bIsEmpty())
    {
        //
        //  Get the current path out and let's fault it in. We have to get the local
        //  path for this scope.
        //
        strCurScope = colStack.objPop();
        facCQCRemBrws().eConvertPath(strCurScope, kCIDLib::True, strRelPath, strLocalPath);

        //
        //  See if we already have this path, and, if we do has its contents been
        //  faulted in. If not or not, then fault it in.
        //
        pnodeCurScope = static_cast<TDataSrvAccImpl::TTreeCache::TNodeNT*>
        (
            m_colTreeCache.pnodeAt(strCurScope)
        );

        if (!pnodeCurScope || !pnodeCurScope->objData().bChildItemsLoaded())
        {
            pnodeCurScope = static_cast<TDataSrvAccImpl::TTreeCache::TNodeNT*>
            (
                pnodeFaultInScope(strCurScope, kCIDLib::True, eDType, strLocalPath)
            );
        }

        //
        //  Let's run through this guy's children and process them. For items we
        //  add them to the list, for the scopes we push them on the stack for
        //  subsequent processing.
        //
        TTreeCache::TNode* pnodeCur = pnodeCurScope->pnodeFirstChild();
        while (pnodeCur)
        {
            if (pnodeCur->eType() == tCIDLib::ETreeNodes::NonTerminal)
            {
                // Push the full hierarchical path onto the stack for later recursion
                strChild = strCurScope;
                facCQCRemBrws().AddPathComp(strChild, pnodeCur->strName());
                colStack.objPush(strChild);
            }
             else
            {
                // Add the relative path to the return list
                strChild = strRelPath;
                facCQCRemBrws().AddPathComp(strChild, pnodeCur->strName());
                colToFill.objAdd(strChild);
            }

            // Move forward
            pnodeCur = pnodeCur->pnodeNext();
        }
    }
}


tCIDLib::TVoid
TDataSrvAccImpl::SetEventPauseState(const   TString&            strOrgHPath
                                    ,       tCIDLib::TCard4&    c4NewSerialNum
                                    ,       tCIDLib::TCard8&    enctLastChange
                                    , const tCIDLib::TBoolean   bToSet
                                    , const TCQCSecToken&       sectUser)
{
    try
    {
        //
        //  Do the prep-work and get the path info from the incoming HPath. Any valid
        //  login is OK here since this is something we allow from user actions.
        //
        TString strHPath;
        TString strLocalPath;
        const tCQCRemBrws::EDTypes eType = eCheckParms
        (
            sectUser
            , strOrgHPath
            , kCIDLib::False
            , strHPath
            , strLocalPath
            , tCQCKit::EUserRoles::LimitedUser
        );

        //
        //  And let's get this guy, forcing it into the cache if not already. These are
        //  not large, so we always will just cache the data here.
        //
        tCIDLib::TBoolean bCache = kCIDLib::True;
        TDSCacheItem* pdsciFl = pdsciUpdateHierCache(strHPath, kCIDLib::False, bCache);

        // Get an input stream over this guy's data chunk
        tCIDLib::TCard4 c4DataBytes;
        const TMemBuf& mbufData = pdsciFl->mbufForChunk
        (
            kCIDMData::strChunkId_Data, c4DataBytes
        );
        TBinMBufInStream strmSrc(&mbufData, c4DataBytes);

        //
        //  Now based on the specific type, read it in and update it and then write it
        //  back out to a buffer based stream.
        //
        TBinMBufOutStream strmTar(c4DataBytes + 1024);
        if (eType == tCQCRemBrws::EDTypes::EvMonitor)
        {
            TCQCEvMonCfg emoncEdit;
            strmSrc >> emoncEdit;
            emoncEdit.bPaused(bToSet);
            strmTar<< emoncEdit;
        }
         else if (eType == tCQCRemBrws::EDTypes::SchEvent)
        {
            TCQCSchEvent csrcEdit;
            strmSrc >> csrcEdit;
            csrcEdit.bPaused(bToSet);
            strmTar<< csrcEdit;
        }
         else if (eType == tCQCRemBrws::EDTypes::TrgEvent)
        {
            TCQCTrgEvent csrcEdit;
            strmSrc >> csrcEdit;
            csrcEdit.bPaused(bToSet);
            strmTar<< csrcEdit;
        }
        strmTar.Flush();

        //
        //  We have to update the paused metadata key before we update the chunk since it
        //  will also update the extra stored metadata on the hierarchical info so we need
        //  this to be correct.
        //
        TKeyValuePair kvalSet
        (
            kCQCRemBrws::strMetaKey_Paused
            , bToSet ? kCQCKit::pszFld_True : kCQCKit::pszFld_False
        );
        tCIDLib::TBoolean bFileChange;
        pdsciFl->bSetMetaVal(kvalSet, bFileChange);

        // And we can how update the data chunk on this guy which will store to disk as well
        pdsciFl->UpdateChunk(kCIDMData::strChunkId_Data, strmTar.c4CurSize(), strmTar.mbufData());

        // Give the caller back the latest info
        c4NewSerialNum = pdsciFl->c4SerialNum();
        enctLastChange = pdsciFl->enctLastFileChange();
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}


tCIDLib::TVoid
TDataSrvAccImpl::SetPeriodicEvTime( const   TString&            strOrgHPath
                                    ,       tCIDLib::TCard4&    c4NewSerialNum
                                    ,       tCIDLib::TCard8&    enctLastChange
                                    , const tCIDLib::TCard4     c4Period
                                    , const tCIDLib::TCard8     enctStartAt
                                    , const TCQCSecToken&       sectUser)
{
    try
    {
        //
        //  Do the prep-work and get the path info from the incoming HPath. This has to
        //  be at limited user level, since we allow this to be done from user actions.
        //
        TString strHPath;
        TString strLocalPath;
        const tCQCRemBrws::EDTypes eType = eCheckParms
        (
            sectUser
            , strOrgHPath
            , kCIDLib::False
            , strHPath
            , strLocalPath
            , tCQCKit::EUserRoles::LimitedUser
        );

        //
        //  And let's get this guy, forcing it into the cache if not already. These are
        //  not large, so we always will just cache the data here.
        //
        tCIDLib::TBoolean bCache = kCIDLib::True;
        TDSCacheItem* pdsciFl = pdsciUpdateHierCache(strHPath, kCIDLib::False, bCache);

        // Get an input stream over this guy's data chunk
        tCIDLib::TCard4 c4DataBytes;
        const TMemBuf& mbufData = pdsciFl->mbufForChunk
        (
            kCIDMData::strChunkId_Data, c4DataBytes
        );
        TBinMBufInStream strmSrc(&mbufData, c4DataBytes);

        // Now we can stream in the data and update it and write it back out
        TCQCSchEvent csrcEdit;
        strmSrc >> csrcEdit;
        csrcEdit.SetPeriodic(csrcEdit.eType(), c4Period, 0, enctStartAt);
        TBinMBufOutStream strmTar(c4DataBytes + 1024);
        strmTar << csrcEdit << kCIDLib::FlushIt;
        pdsciFl->UpdateChunk
        (
            kCIDMData::strChunkId_Data, strmTar.c4CurSize(), strmTar.mbufData()
        );

        // Give the caller back the latest info
        c4NewSerialNum = pdsciFl->c4SerialNum();
        enctLastChange = pdsciFl->enctLastFileChange();
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}


tCIDLib::TVoid
TDataSrvAccImpl::SetScheduledEvTime(const   TString&            strOrgHPath
                                    ,       tCIDLib::TCard4&    c4NewSerialNum
                                    ,       tCIDLib::TCard8&    enctLastChange
                                    , const tCIDLib::TCard4     c4Day
                                    , const tCIDLib::TCard4     c4Hour
                                    , const tCIDLib::TCard4     c4Minute
                                    , const tCIDLib::TCard4     c4Mask
                                    , const TCQCSecToken&       sectUser)
{
    try
    {
        //
        //  Do the prep-work and get the path info from the incoming HPath. This has
        //  to be done at limited user level since we allow it from user actions.
        //
        TString strHPath;
        TString strLocalPath;
        const tCQCRemBrws::EDTypes eType = eCheckParms
        (
            sectUser
            , strOrgHPath
            , kCIDLib::False
            , strHPath
            , strLocalPath
            , tCQCKit::EUserRoles::LimitedUser
        );

        //
        //  And let's get this guy, forcing it into the cache if not already. These are
        //  not large, so we always will just cache the data here.
        //
        tCIDLib::TBoolean bCache = kCIDLib::True;
        TDSCacheItem* pdsciFl = pdsciUpdateHierCache(strHPath, kCIDLib::False, bCache);

        // Get an input stream over this guy's data chunk
        tCIDLib::TCard4 c4DataBytes;
        const TMemBuf& mbufData = pdsciFl->mbufForChunk
        (
            kCIDMData::strChunkId_Data, c4DataBytes
        );
        TBinMBufInStream strmSrc(&mbufData, c4DataBytes);

        // Now we can stream in the data and update it and write it back out
        TCQCSchEvent csrcEdit;
        strmSrc >> csrcEdit;

        //
        //  We get the current time, break it out into parts, then set the hour/min
        //  to the incoming ones. We want to calculate the next time from that basis.
        //
        TTime tmStart(tCIDLib::ESpecialTimes::CurrentTime);
        tCIDLib::TCard4     c4StYear;
        tCIDLib::EMonths    eStMonth;
        tCIDLib::TCard4     c4StDay;
        tmStart.eAsDateInfo(c4StYear, eStMonth, c4StDay);
        tmStart.FromDetails(c4StYear, eStMonth, c4StDay, c4Hour, c4Minute);

        //
        //  OK, now set the info. If we got an empty mask, then use the already
        //  set one. WE pass the original type back in since we aren't changing that
        //
        csrcEdit.SetScheduled
        (
            csrcEdit.eType()
            , c4Day
            , c4Hour
            , c4Minute
            , c4Mask ? c4Mask : csrcEdit.c4Mask()
            , tmStart.enctTime()
        );

        TBinMBufOutStream strmTar(c4DataBytes + 1024);
        strmTar << csrcEdit << kCIDLib::FlushIt;
        pdsciFl->UpdateChunk(kCIDMData::strChunkId_Data, strmTar.c4CurSize(), strmTar.mbufData());

        // Give the caller back the latest info
        c4NewSerialNum = pdsciFl->c4SerialNum();
        enctLastChange = pdsciFl->enctLastFileChange();
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}


tCIDLib::TVoid
TDataSrvAccImpl::SetSunBasedEvOffset(const  TString&            strOrgHPath
                                    ,       tCIDLib::TCard4&    c4NewSerialNum
                                    ,       tCIDLib::TCard8&    enctLastChange
                                    , const tCIDLib::TInt4      i4Offset
                                    , const TCQCSecToken&       sectUser)
{
    try
    {
        //
        //  Do the prep-work and get the path info from the incoming HPath. This has
        //  to be done at limited user level since we allow it from user actions.
        //
        TString strHPath;
        TString strLocalPath;
        const tCQCRemBrws::EDTypes eType = eCheckParms
        (
            sectUser
            , strOrgHPath
            , kCIDLib::False
            , strHPath
            , strLocalPath
            , tCQCKit::EUserRoles::LimitedUser
        );

        //
        //  And let's get this guy, forcing it into the cache if not already. These are
        //  not large, so we always will just cache the data here.
        //
        tCIDLib::TBoolean bCache = kCIDLib::True;
        TDSCacheItem* pdsciFl = pdsciUpdateHierCache(strHPath, kCIDLib::False, bCache);

        // Get an input stream over this guy's data chunk
        tCIDLib::TCard4 c4DataBytes;
        const TMemBuf& mbufData = pdsciFl->mbufForChunk
        (
            kCIDMData::strChunkId_Data, c4DataBytes
        );
        TBinMBufInStream strmSrc(&mbufData, c4DataBytes);

        // Now we can stream in the data and update it and write it back out
        TCQCSchEvent csrcEdit;
        strmSrc >> csrcEdit;
        csrcEdit.i4Offset(i4Offset);
        TBinMBufOutStream strmTar(c4DataBytes + 1024);
        strmTar << csrcEdit << kCIDLib::FlushIt;
        pdsciFl->UpdateChunk(kCIDMData::strChunkId_Data, strmTar.c4CurSize(), strmTar.mbufData());

        // Give the caller back the latest info
        c4NewSerialNum = pdsciFl->c4SerialNum();
        enctLastChange = pdsciFl->enctLastFileChange();
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}


//
//  This handles file uploads. We have to get the file stored to disk, and we have
//  to get them into the cache if not already, or update the cache if it's already
//  there.
//
tCIDLib::TVoid
TDataSrvAccImpl::UploadFirst(const  TString&                strOrgHPath
                            ,       tCIDLib::TCard4&        c4Cookie
                            ,       tCIDLib::TCard4&        c4SerialNum
                            , const tCIDLib::TCard4         c4DataChunkSz
                            , const tCIDLib::TKVPFList&     colMetaVals
                            , const tCIDLib::TCard4         c4FirstBlockSz
                            ,       THeapBuf&&              mbufFirstBlock
                            , const TString&                strExtChunkId
                            , const tCIDLib::TCard4         c4ExtChunkSz
                            ,       THeapBuf&&              mbufExtChunk
                            , const tCIDLib::TCard4         c4Flags
                            ,       tCIDLib::TEncodedTime&  enctLastChange
                            , const TCQCSecToken&           sectUser)
{
    // Do the prep-work and get the path info from the incoming HPath
    TString strHPath;
    TString strLocalPath;
    TString strRelPath;
    const tCQCRemBrws::EDTypes eType = eCheckParms
    (
        sectUser
        , strOrgHPath
        , kCIDLib::False
        , strHPath
        , strRelPath
        , strLocalPath
        , tCQCKit::EUserRoles::PowerUser
    );

    #if defined(LOG_FILEACC)
    {
        facCQCDataSrv.LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"Starting upload"
            , strOrgHPath
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
    }
    #endif

    // Get the parent local directory
    TPathStr pathParDir(strLocalPath);
    pathParDir.bRemoveNameExt();

    // If it already exists and overwrites are not cool, then reject
    if (TFileSys::bExists(strLocalPath))
    {
        if (!(c4Flags & kCQCRemBrws::c4Flag_OverwriteOK))
        {
            facCQCDataSrv.ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kDSrvErrs::errcFAcc_Overwrite
                , strHPath
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Authority
            );
        }
    }

    // We need to lock for the rest of the way
    TLocker lockrSync(&m_mtxSync);

    //
    //  Check the parent scope and make sure the local path is there and that we
    //  have faulted in the hierarchy down to that point at least.
    //
    if (!pnodeCheckParScope(strHPath, strLocalPath, eType, EFaultInOpts::IfLocalExists))
    {
        facCQCDataSrv.ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kDSrvErrs::errcFAcc_TarScopeNotFnd
            , strHPath
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
        );
    }

    //
    //  If it's going to fit in one block, then we can just call the file writer
    //  method now. Else we need to set up a transfer object.
    //
    const tCIDLib::TBoolean bLast((c4Flags & kCQCRemBrws::c4Flag_Done) != 0);
    if (bLast)
    {
        #if defined(LOG_FILEACC)
        {
            facCQCDataSrv.LogMsg
            (
                CID_FILE
                , CID_LINE
                , L"Single block upload, writing file"
                , strHPath
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );
        }
        #endif

        CIDAssert
        (
            c4DataChunkSz <= kCQCRemBrws::c4DataBlockSz
            , L"bLast was set but full upload size was > block size"
        );

        WriteData
        (
            strLocalPath
            , strHPath
            , strRelPath
            , eType
            , c4SerialNum
            , colMetaVals
            , c4DataChunkSz
            , mbufFirstBlock
            , strExtChunkId
            , c4ExtChunkSz
            , mbufExtChunk
            , c4Flags
            , enctLastChange
        );
    }
     else
    {
        CIDAssert
        (
            c4DataChunkSz > kCQCRemBrws::c4DataBlockSz
            , L"bLast was false but full data chunk size was not > block size"
        );

        // We need to do multiple blocks, so set up a transfer object for upload.
        TTransOp* ptopNew = ptopGetFreeUp
        (
            strHPath
            , c4DataChunkSz
            , c4FirstBlockSz
            , tCIDLib::ForceMove(mbufFirstBlock)
            , strExtChunkId
            , c4ExtChunkSz
            , tCIDLib::ForceMove(mbufExtChunk)
            , colMetaVals
            , c4Flags
        );

        #if defined(LOG_FILEACC)
        {
            TString strMsg(L"Multi-block upload. Cookie=");
            strMsg.AppendFormatted(ptopNew->m_c4Cookie);
            strMsg.Append(L", FullSz=");
            strMsg.AppendFormatted(ptopNew->m_c4FullSz);
            strMsg.Append(L", ExtChunkId=");
            strMsg.Append(strExtChunkId);
            facCQCDataSrv.LogMsg
            (
                CID_FILE
                , CID_LINE
                , strMsg
                , strHPath
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );
        }
        #endif

        // Give the caller the cookie
        c4Cookie = ptopNew->m_c4Cookie;

    }

    #if defined(LOG_FILEACC)
    // DumpCache();
    #endif
}


tCIDLib::TVoid
TDataSrvAccImpl::UploadNext(const   tCIDLib::TCard4         c4BufSz
                            , const THeapBuf&               mbufData
                            , const tCIDLib::TCard4         c4Cookie
                            , const tCIDLib::TBoolean       bLast
                            ,       tCIDLib::TCard4&        c4SerialNum
                            ,       tCIDLib::TEncodedTime&  enctLastChange)
{
    // We need to lock for the rest of the way
    TLocker lockrSync(&m_mtxSync);

    // Find the transfer op. It'll throw if not found
    TTransOp* ptopTrans = ptopForCookie(c4Cookie);

    // Make sure the new data looks reasonable
    CIDAssert
    (
        ptopTrans->m_c4NextOfs + c4BufSz <= ptopTrans->m_c4FullSz
        , L"Upload transfer buffer overflow"
    );

    // Copy in the new data at the next offset and then bump the next offset
    ptopTrans->m_mbufData.CopyIn(mbufData, c4BufSz, ptopTrans->m_c4NextOfs);
    ptopTrans->m_c4NextOfs += c4BufSz;

    // if this is the last, then we are done
    if (bLast)
    {
        #if defined(LOG_FILEACC)
        {
            TString strMsg(L"Multi-block upload complete. Cookie=");
            strMsg.AppendFormatted(ptopTrans->m_c4Cookie);
            facCQCDataSrv.LogMsg
            (
                CID_FILE
                , CID_LINE
                , strMsg
                , ptopTrans->m_strHPath
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );
        }
        #endif

        CIDAssert
        (
            ptopTrans->m_c4NextOfs == ptopTrans->m_c4FullSz
            , L"Last upload buffer not equal to full file size"
        );

        // Get the relative and local paths again
        TString strLocalPath;
        TString strRelPath;
        const tCQCRemBrws::EDTypes  eType = facCQCRemBrws().eConvertPath
        (
            ptopTrans->m_strHPath, kCIDLib::False, strRelPath, strLocalPath
        );

        // Write the data
        WriteData
        (
            strLocalPath
            , ptopTrans->m_strHPath
            , strRelPath
            , eType
            , c4SerialNum
            , ptopTrans->m_colMetaUp
            , ptopTrans->m_c4FullSz
            , ptopTrans->m_mbufData
            , ptopTrans->m_strExtChunkId
            , ptopTrans->m_c4ExtSz
            , ptopTrans->m_mbufExtChunk
            , ptopTrans->m_c4Flags
            , enctLastChange
        );

        // Remove this transfer op since we are done with it now
        m_colTransOps.RemoveElem(ptopTrans);
    }
}


// ---------------------------------------------------------------------------
//  TDataSrvAccImpl: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TDataSrvAccImpl::Initialize()
{
    TParent::Initialize();

    //
    //  Start the hierarchical cache by adding the root /Custoimze item, and put in
    //  the top level scopes for each data type, marked as not having had their
    //  contents loaded yet. To make it easier, we set up a little array of
    //  structures.
    //
    {
        struct THierItem
        {
            const tCIDLib::TCh*     pszParent;
            const tCIDLib::TCh*     pszName;
            const tCIDLib::TCh*     pszDesc;
            tCQCRemBrws::EDTypes    eType;
            tCIDLib::TCard1         bLoaded;
            tCQCRemBrws::EItemFlags eFlags;
        };

        const tCQCRemBrws::EItemFlags eF1 = tCQCRemBrws::EItemFlags::SpecialScope;
        const tCQCRemBrws::EItemFlags eF2 = tCQCRemBrws::EItemFlags::TypeRoot;
        const tCQCRemBrws::EItemFlags eF3 = tCQCRemBrws::EItemFlags::UserRoot;
        const tCQCRemBrws::EItemFlags eF4 = tCQCRemBrws::EItemFlags::SystemRoot;
        const THierItem aItems[] =
        {
            { L"/", L"Customize", L"Customization Root", tCQCRemBrws::EDTypes::Count, 1, eF1 }

          , { L"/Customize", L"Events", L"Events", tCQCRemBrws::EDTypes::Count, 1, eF1 }

          , { L"/Customize/Events", L"Monitors", L"Event Mons", tCQCRemBrws::EDTypes::Count, 1, eF2 }
          , { L"/Customize/Events/Monitors", L"User", L"User Event mons", tCQCRemBrws::EDTypes::EvMonitor, 0, eF3 }
          , { L"/Customize/Events/Monitors", L"System", L"System Event Mons", tCQCRemBrws::EDTypes::EvMonitor, 0, eF4 }

          , { L"/Customize/Events", L"Scheduled", L"Scheduled Events", tCQCRemBrws::EDTypes::Count, 1, eF2 }
          , { L"/Customize/Events/Scheduled", L"User", L"User Scheduled Events", tCQCRemBrws::EDTypes::SchEvent, 0, eF3 }
          , { L"/Customize/Events/Scheduled", L"System", L"System Scheduled Events", tCQCRemBrws::EDTypes::SchEvent, 0, eF4 }

          , { L"/Customize/Events", L"Triggered", L"Triggered Events", tCQCRemBrws::EDTypes::Count, 1, eF2 }
          , { L"/Customize/Events/Triggered", L"User", L"User Triggered Events", tCQCRemBrws::EDTypes::TrgEvent, 0, eF3 }
          , { L"/Customize/Events/Triggered", L"System", L"System Triggered Events", tCQCRemBrws::EDTypes::TrgEvent, 0, eF4 }

          , { L"/Customize", L"GlobalActs", L"Global Actions", tCQCRemBrws::EDTypes::Count, 1, eF2 }
          , { L"/Customize/GlobalActs", L"User", L"User Global Actions", tCQCRemBrws::EDTypes::GlobalAct, 0, eF3 }
          , { L"/Customize/GlobalActs", L"System", L"System Global Actions", tCQCRemBrws::EDTypes::GlobalAct, 0, eF4 }

          , { L"/Customize", L"Images", L"Images", tCQCRemBrws::EDTypes::Count, 1, eF2 }
          , { L"/Customize/Images", L"User", L"User Images", tCQCRemBrws::EDTypes::Image, 0, eF3 }
          , { L"/Customize/Images", L"System", L"System Images", tCQCRemBrws::EDTypes::Image, 0, eF4 }

          , { L"/Customize", L"Interfaces", L"Interfaces", tCQCRemBrws::EDTypes::Count, 1, eF2 }
          , { L"/Customize/Interfaces", L"User", L"User Interfaces", tCQCRemBrws::EDTypes::Template, 0, eF3 }
          , { L"/Customize/Interfaces", L"System", L"System Interfaces", tCQCRemBrws::EDTypes::Template, 0, eF4 }

          , { L"/Customize", L"Macros", L"Macros", tCQCRemBrws::EDTypes::Count, 1, eF2 }
          , { L"/Customize/Macros", L"User", L"User Macros", tCQCRemBrws::EDTypes::Macro, 0, eF3 }
          , { L"/Customize/Macros", L"System", L"System Macros", tCQCRemBrws::EDTypes::Macro, 0, eF4 }
        };


        TString strPath;
        const tCIDLib::TCard4 c4Count = tCIDLib::c4ArrayElems(aItems);
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const THierItem& itemCur = aItems[c4Index];

            // Always allow editing if in the development environment
            strPath = itemCur.pszParent;
            facCQCRemBrws().AddPathComp(strPath, itemCur.pszName);
            TDSCacheItem dsciNew
            (
                itemCur.pszName
                , itemCur.eType
                , m_c4NextScopeId++
                , m_c4NextItemId++
                , strPath
                , itemCur.eFlags
                , kCIDLib::False
            );

            //
            //  We have to be careful to mark those that are already loaded so that
            //  that we don't try to load them again and cause a dup error.
            //
            dsciNew.bChildItemsLoaded(itemCur.bLoaded != 0);

            //
            //  The non-terminal add method doesn't take node data, so we have to
            //  set it after the fact.
            //
            TTreeCache::TNodeNT* pnodeNew = m_colTreeCache.pnodeAddNonTerminal
            (
                itemCur.pszParent, itemCur.pszName, itemCur.pszDesc
            );
            pnodeNew->objData(dsciNew);
        }
    }

    #if defined(LOG_FILEACC)
    // DumpCache();
    #endif
}


tCIDLib::TVoid TDataSrvAccImpl::Terminate()
{
    #if defined(LOG_FILEACC)
    DumpCache(kCIDLib::True);
    #endif

    // Nothing to do at this time, just pass to our parent
    TParent::Terminate();
}



// ---------------------------------------------------------------------------
//  TDataSrvAccImpl: Private, static methods
// ---------------------------------------------------------------------------

//
//  Search the passed metadata list for the passed key and return whether it has the
//  passed value. Throw if not found, or just return false as requested.
//
tCIDLib::TBoolean
TDataSrvAccImpl::bCheckMetaVal( const   tCIDLib::TKVPFList& colMeta
                                , const TString&            strKey
                                , const TString&            strValue
                                , const tCIDLib::TBoolean   bThrowIfNotFnd)
{
    const tCIDLib::TCard4 c4Count = colMeta.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TKeyValuePair& kvalCur = colMeta[c4Index];
        if (kvalCur.strKey().bCompareI(strKey))
            return kvalCur.strValue().bCompareI(strValue);

        c4Index++;
    }

    if (bThrowIfNotFnd)
    {
        facCIDMData().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMDataErrs::errcChFl_MetaKeyNotFound
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , strKey
        );
    }
    return kCIDLib::False;
}


//
//  We work our way up to the root, then start back down, adding the scope id of
//  each scopealong the way and, if it's a file ultimately, we add the item id for
//  the last one.
//
//  WE ASSUME the caller has locked!
//
tCIDLib::TVoid
TDataSrvAccImpl::BuildIdPath(const  TTreeCache::TNode* const    pnodeTar
                            ,       tCIDLib::TCardList&         fcolToFill
                            , const tCIDLib::TBoolean           bIsScope)
{
    // Clean out the list to start
    fcolToFill.RemoveAll();

    TFundStack<tCIDLib::TCard4> fcolStack(1024UL);
    const  TTreeCache::TNode* pnodeCur = pnodeTar;

    // For the first one, we push the scope or item id
    if (bIsScope)
        fcolStack.Push(pnodeCur->objData().dsbiCache().c4IdScope());
    else
        fcolStack.Push(pnodeCur->objData().dsbiCache().c4IdItem());

    pnodeCur = pnodeCur->pnodeParent();
    while (pnodeCur)
    {
        fcolStack.Push(pnodeCur->objData().dsbiCache().c4IdScope());
        pnodeCur = pnodeCur->pnodeParent();
    }

    // Now build up the list by popping them off and adding them to the output
    while (!fcolStack.bIsEmpty())
        fcolToFill.c4AddElement(fcolStack.tPop());
}


//
//  Checks to see if the security token is valid, which means the client has logged
//  in under some valid CQC account.
//
tCIDLib::TVoid TDataSrvAccImpl::CheckSecToken(const TCQCSecToken& sectUser)
{
    if (!facCQCDataSrv.bCheckSTok(sectUser))
    {
        facCQCDataSrv.ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kDSrvErrs::errcFAcc_UserRights
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Authority
        );
    }
}


//
//  Checks that the expected and found type are the same, else it throws. Since
//  we cast the data in the cache based on type, this is particularly important
//  for accessing cached data.
//
tCIDLib::TVoid
TDataSrvAccImpl::CheckType( const   tCQCRemBrws::EDTypes    eExpected
                            , const tCQCRemBrws::EDTypes    eFound
                            , const tCIDLib::TBoolean       bFromCache)
{
    if (eExpected != eFound)
    {
        facCQCDataSrv.ThrowErr
        (
            CID_FILE
            , CID_LINE
            , bFromCache ? kDSrvErrs::errcFAcc_CachedType
                         : kDSrvErrs::errcFAcc_AccessedType
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , tCQCRemBrws::strLoadEDTypes (eExpected)
            , tCQCRemBrws::strLoadEDTypes (eFound)
        );
    }
}


//
//  Check to see if the passed security token is both valid and that it represents
//  a user account of at least the indicated role or greater.
//
tCIDLib::TVoid
TDataSrvAccImpl::CheckUserRole( const   TCQCSecToken&       sectUser
                                , const tCQCKit::EUserRoles eMinRole)
{
    if (!facCQCDataSrv.bCheckMinRole(sectUser, eMinRole))
    {
        facCQCDataSrv.ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kDSrvErrs::errcFAcc_UserRights
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Authority
        );
    }
}


//
//  Helpers for copying files and directories. It's mostly for supporting copy
//  and paste but possibly some other stuff in the future.
//
tCIDLib::TVoid
TDataSrvAccImpl::CopyDir(const TString& strSrcPath, const TString& strTarPath)
{
    //
    //  Iterate all of the files in the source and copy to the target, recursing
    //  into sub-directories.
    //
    TDirIter diterLoad;
    TFindBuf fndbCur;

    // First do sub-scopes
    if (diterLoad.bFindFirst(strSrcPath
                            , kCIDLib::pszAllFilesSpec
                            , fndbCur
                            , tCIDLib::EDirSearchFlags::NormalDirs))
    {
        do
        {
            // Create a new target path
            TString strName;
            fndbCur.pathFileName().bQueryNameExt(strName);

            TPathStr pathTar(strTarPath);
            pathTar.AddLevel(strName);

            // If it doesn't exist, create it
            if (!TFileSys::bIsDirectory(pathTar))
                TFileSys::MakeSubDirectory(strName, strTarPath);

            CopyDir(fndbCur.pathFileName(), pathTar);

        }   while(diterLoad.bFindNext(fndbCur));
    }

    // And copy any files at this level
    if (diterLoad.bFindFirst(strSrcPath
                            , kCIDLib::pszAllFilesSpec
                            , fndbCur
                            , tCIDLib::EDirSearchFlags::NormalFiles))
    {
        do
        {
            // Create a new target path
            TString strName;
            fndbCur.pathFileName().bQueryNameExt(strName);

            TPathStr pathTar(strTarPath);
            pathTar.AddLevel(strName);
            CopyFile(fndbCur.pathFileName(), pathTar);

        }   while(diterLoad.bFindNext(fndbCur));
    }
}

tCIDLib::TVoid
TDataSrvAccImpl::CopyFile(const TString& strSrcFile, const TString& strTarPath)
{
    // For now, it's enough to just copy the file
    TFileSys::CopyFile(strSrcFile, strTarPath);
}


//
//  A wrapper around the method of the same name in the CQCRemBrws facility class,
//  so that we can add a little more checking.
//
tCQCRemBrws::EDTypes
TDataSrvAccImpl::eConvertPath(  const   TString&                strToCvt
                                , const tCIDLib::TBoolean       bIsScope
                                ,       TString&                strToFill
                                , const tCIDLib::TBoolean       bGetFullPath
                                , const tCQCRemBrws::EDTypes    eExpected
                                , const tCIDLib::TBoolean       bFromCache)
{
    const tCQCRemBrws::EDTypes  eRet = facCQCRemBrws().eConvertPath
    (
        strToCvt, bIsScope, strToFill, bGetFullPath
    );

    if (eExpected != tCQCRemBrws::EDTypes::Count)
        CheckType(eExpected, eRet, bFromCache);

    return eRet;
}

tCQCRemBrws::EDTypes
TDataSrvAccImpl::eConvertPath(  const   TString&                strToCvt
                                , const tCIDLib::TBoolean       bIsScope
                                ,       TString&                strRelPath
                                ,       TString&                strLocalPath
                                , const tCQCRemBrws::EDTypes    eExpected
                                , const tCIDLib::TBoolean       bFromCache)
{
    const tCQCRemBrws::EDTypes  eRet = facCQCRemBrws().eConvertPath
    (
        strToCvt, bIsScope, strRelPath, strLocalPath
    );

    if (eExpected != tCQCRemBrws::EDTypes::Count)
        CheckType(eExpected, eRet, bFromCache);

    return eRet;
}


// ---------------------------------------------------------------------------
//  TDataSrvAccImpl: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Most of the methods need to do this. We get an HPath from the client. We need to
//  check it and make sure it's normalized. Then we need to parse it and get the relative
//  path and type, and the local path from that. And we also need to verify their security
//  token.
//
//  If any incoming is bad, we throw.
//
//  This all done before the need for them to lock, since it's just operating on
//  incoming parameters which they pass on to us.
//
//
//  Most of the methods need to do this. We get an HPath from the client. We need to
//  check it and make sure it's normalized. Then we need to parse it and get the relative
//  path and type, and the local path from that. And we also need to verify their security
//  token.
//
//  If any incoming is bad, we throw.
//
//  This all done before the need for them to lock, since it's just operating on
//  incoming parameters which they pass on to us.
//
tCIDLib::TVoid
TDataSrvAccImpl::CheckParms(const   TCQCSecToken&       sectUser
                            , const TString&            strOrgHPath
                            , const tCIDLib::TBoolean   bIsScope
                            ,       TString&            strHPath
                            , const tCQCKit::EUserRoles eMinRole) const
{
    //
    //  Make sure the security token is valid, and for at least the minimum role
    //  if one is provided.
    //
    if (eMinRole == tCQCKit::EUserRoles::Count)
        CheckSecToken(sectUser);
    else
        CheckUserRole(sectUser, eMinRole);

    // Fix up the path if required (slashes, double slashes, etc...)
    facCQCKit().PrepRemBrwsPath(strOrgHPath, strHPath);
}


// Check the passed tree node and make sure it is of the indicated type
tCIDLib::TVoid
TDataSrvAccImpl::CheckTreeNodeType( const   TString&                                    strHPath
                                    , const TDataSrvAccImpl::TTreeCache::TNode* const   pnodeCheck
                                    , const tCIDLib::TBoolean                           bIsScope) const
{
    if ((bIsScope && (pnodeCheck->eType() != tCIDLib::ETreeNodes::NonTerminal))
    ||  (!bIsScope && (pnodeCheck->eType() != tCIDLib::ETreeNodes::Terminal)))
    {
        facCQCDataSrv.ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kDSrvErrs::errcFAcc_WrongType
            , strHPath
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::TypeMatch
        );
    }
}


tCQCRemBrws::EDTypes
TDataSrvAccImpl::eCheckParms(const  TCQCSecToken&       sectUser
                            , const TString&            strOrgHPath
                            , const tCIDLib::TBoolean   bIsScope
                            ,       TString&            strHPath
                            ,       TString&            strLocalPath
                            , const tCQCKit::EUserRoles eMinRole) const
{
    //
    //  Make sure the security token is valid, and for at least the minimum role
    //  if one is provided.
    //
    if (eMinRole == tCQCKit::EUserRoles::Count)
        CheckSecToken(sectUser);
    else
        CheckUserRole(sectUser, eMinRole);

    // Fix up the path if required (slashes, double slashes, etc...)
    facCQCKit().PrepRemBrwsPath(strOrgHPath, strHPath);

    // And get the local path
    return facCQCRemBrws().eConvertPath(strHPath, bIsScope, strLocalPath, kCIDLib::True);
}

tCQCRemBrws::EDTypes
TDataSrvAccImpl::eCheckParms(const  TCQCSecToken&       sectUser
                            , const TString&            strOrgHPath
                            , const tCIDLib::TBoolean   bIsScope
                            ,       TString&            strHPath
                            ,       TString&            strRelPath
                            ,       TString&            strLocalPath
                            , const tCQCKit::EUserRoles eMinRole) const
{
    //
    //  Make sure the security token is valid, and for at least the minimum role
    //  if one is provided.
    //
    if (eMinRole == tCQCKit::EUserRoles::Count)
        CheckSecToken(sectUser);
    else
        CheckUserRole(sectUser, eMinRole);

    // Fix up the path if required (slashes, double slashes, etc...)
    facCQCKit().PrepRemBrwsPath(strOrgHPath, strHPath);

    // Get the relative and local paths
    return facCQCRemBrws().eConvertPath(strHPath, bIsScope, strRelPath, strLocalPath);
}


tCIDLib::TVoid
TDataSrvAccImpl::FormatReportHdr(       TTextOutStream&     strmTar
                                , const TString&            strDescr
                                , const TCQCSecToken&       sectUser)
{
    // And query info on the installed drivers
    tCIDLib::TKValsList colDrvs;
    {
        tCQCKit::TInstSrvProxy orbcIS = facCQCKit().orbcInstSrvProxy();
        orbcIS->DrvCfgQueryReportInfo(colDrvs, sectUser);
    }

    strmTar << L"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n\n"
               L"<!DOCTYPE Report PUBLIC \"" << TFacCQCRemBrws::strReportDTDURN
            << L"\" \"CQCReport.Dtd\">\n<Report GeneratedAt=\"0x"
            << TCardinal64(TTime::enctNow())
            << L"\" Ver=\"" << kCQCKit::pszVersion
            << L"\">";

    // And info on the installed drivers
    strmTar << L"<Drivers>";
    TString strMake;
    TString strModel;
    const tCIDLib::TCard4 c4DrvCnt = colDrvs.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4DrvCnt; c4Index++)
    {
        const TKeyValues& kvalsCur = colDrvs[c4Index];

        // We have to break the make/model values aparent
        strMake = kvalsCur.strVal1();
        strMake.bSplit(strModel, kCIDLib::chForwardSlash);

        strmTar << L"<Driver Moniker=\"" << kvalsCur.strKey()
                << L"\" Make=\"" << strMake
                << L"\" Model=\"" << strModel
                << L"\" Version=\"" << kvalsCur.strVal3()
                << L"\" Host=\"" << kvalsCur.strVal2()
                << L"\"/>";
    }
    strmTar << L"</Drivers>";
}


//
//  An (optionally) recursive helper for the public report generation methods. They
//  call us here with the starting path and we either do that and return, or recurse
//  on ourself and do the whole tree.
//
tCIDLib::TVoid
TDataSrvAccImpl::GenerateReport(        TTextOutStream&         strmTar
                                , const tCQCRemBrws::EDTypes    eType
                                ,       TPathStr&               pathH
                                ,       TPathStr&               pathLocal
                                , const tCIDLib::TBoolean       bRecurse
                                ,       TTime&                  tmFmt)
{
    // Remember the incoming lengths of the paths so we can get back there
    const tCIDLib::TCard4 c4OrgHLen = pathH.c4Length();
    const tCIDLib::TCard4 c4OrgLocalLen = pathLocal.c4Length();

    // We do some special stuff for event types, so figure out if it is one up front
    const tCIDLib::TBoolean bEventType
    (
        (eType == tCQCRemBrws::EDTypes::EvMonitor)
        ||  (eType == tCQCRemBrws::EDTypes::SchEvent)
        ||  (eType == tCQCRemBrws::EDTypes::TrgEvent)
    );

    TString strName;
    TString strTime;
    TDirIter diterSearch;
    TFindBuf fndbCur;
    tCIDLib::TKVPFList colMeta;
    tCIDLib::TCard4 c4SerialNum;
    tCIDLib::TEncodedTime enctLast;
    tCIDLib::TBoolean bPaused;
    tCIDLib::TCard4 c4ChunkCnt;

    // Do any files at this level first
    TPathStr pathSearch(pathLocal);
    pathSearch.AddLevel(L"*");
    pathSearch.AppendExt(facCQCRemBrws().strFlTypeExt(eType));
    if (diterSearch.bFindFirst(pathSearch, fndbCur, tCIDLib::EDirSearchFlags::NormalFiles))
    {
        do
        {
            // Build up the paths for this file
            pathH.CapAt(c4OrgHLen);
            pathLocal.CapAt(c4OrgLocalLen);
            if (!fndbCur.pathFileName().bQueryName(strName))
            {
                CIDAssert2(L"Did not get any name component from path");
            }
            facCQCRemBrws().AddPathComp(pathH, strName);
            pathLocal.AddLevel(strName);
            pathLocal.AppendExt(facCQCRemBrws().strFlTypeExt(eType));

            //
            //  If in the cache, use that. Else we use a special chunked file method that
            //  will just read the info we need with minimal overhead.
            //
            bPaused = kCIDLib::False;
            TDSCacheItem* pdsciFl = pdsciFindHierCache(pathH, kCIDLib::False);
            if (pdsciFl && pdsciFl->bDataLoaded())
            {
                c4SerialNum = pdsciFl->c4SerialNum();
                enctLast = pdsciFl->enctLastFileChange();

                if (bEventType)
                {
                    bPaused = pdsciFl->chflData().bTestMetaValue
                    (
                        kCQCRemBrws::strMetaKey_Paused
                        , facCQCKit().strBoolVal(kCIDLib::True)
                        , kCIDLib::False
                    );
                }
            }
             else
            {
                // Set up a binary input stream over the local path
                TBinFileInStream strmSrc
                (
                    pathLocal
                    , tCIDLib::ECreateActs::OpenIfExists
                    , tCIDLib::EFilePerms::Default
                    , tCIDLib::EFileFlags::SequentialScan
                );

                TChunkedFile::ExtractMetaInfo
                (
                    strmSrc, c4SerialNum, enctLast, colMeta, c4ChunkCnt
                );

                if (bEventType)
                {
                    const tCIDLib::TCard4 c4Count = colMeta.c4ElemCount();
                    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
                    {
                        const TKeyValuePair& kvalCur = colMeta[c4Index];
                        if (kvalCur.strKey().bCompareI(kCQCRemBrws::strMetaKey_Paused)
                        &&  kvalCur.strValue().bCompareI(facCQCKit().strBoolVal(kCIDLib::True)))
                        {
                            bPaused = kCIDLib::True;
                            break;
                        }
                    }
                }
            }

            //
            //  And format this guy out, escaping the name just in case, and we need
            //  to format out the time.
            //
            tmFmt.enctTime(enctLast);
            tmFmt.FormatToStr(strTime);
            strmTar << L"<File Name=\"";
            facCIDXML().EscapeFor(strName, strmTar, tCIDXML::EEscTypes::Attribute);
            strmTar << L"\" SerNum=\"" << c4SerialNum << L"\" LastMod=\""
                    << strTime << L"\"";

            // If it's an event type, let's get whether it's enabled or not
            if (bEventType)
                strmTar << L" Paused=\"" << (bPaused ? L'Y' : L'N') << L"\"";

            strmTar << L"/>";

        }   while (diterSearch.bFindNext(fndbCur));
    }

    // If recursing then let's do any sub-scopes
    if (bRecurse)
    {
        // Cap the paths back to original length
        pathH.CapAt(c4OrgHLen);
        pathLocal.CapAt(c4OrgLocalLen);

        pathSearch = pathLocal;
        pathSearch.AddLevel(L"*");
        if (diterSearch.bFindFirst(pathSearch, fndbCur, tCIDLib::EDirSearchFlags::NormalDirs))
        {
            do
            {
                // Get the name part generate the XML scope element
                if (!fndbCur.pathFileName().bQueryName(strName))
                {
                    CIDAssert2(L"Did not get any name component from path");
                }

                // Escape it just in case
                strmTar << L"<Scope Name=\"";
                facCIDXML().EscapeFor(strName, strmTar, tCIDXML::EEscTypes::Attribute);
                strmTar << L"\">";

                // Build up the paths for this new level
                pathH.CapAt(c4OrgHLen);
                pathLocal.CapAt(c4OrgLocalLen);
                if (!fndbCur.pathFileName().bQueryName(strName))
                {
                    CIDAssert2(L"Did not get any name component from path");
                }
                facCQCRemBrws().AddPathComp(pathH, strName);
                pathLocal.AddLevel(strName);

                // And recurse
                GenerateReport(strmTar, eType, pathH, pathLocal, bRecurse, tmFmt);

                // Close out this scope
                strmTar << L"</Scope>";

            }   while (diterSearch.bFindNext(fndbCur));
        }
    }

    // Clean up the paths back to the original lengths
    pathH.CapAt(c4OrgHLen);
    pathLocal.CapAt(c4OrgLocalLen);
}


//
//  Given a local or DS path, this will return the parent path, and the remaining
//  name part, throwing if it is invalid in some way or would go above the root.
//  Local paths have to be fully qualified, and will not include the file extension
//  just the name part.
//
tCIDLib::TVoid
TDataSrvAccImpl::MakeParentPath(const   TString&                strSrc
                                , const tCQCRemBrws::EDTypes    eType
                                ,       TString&                strParPath
                                ,       TString&                strName
                                , const tCIDLib::TBoolean       bIsHPath)
{
    tCIDLib::TBoolean bBadChars = strSrc.bIsEmpty();
    TPathStr pathTmp(strSrc);
    tCIDLib::TCh chSepChar;
    if (bIsHPath)
    {
        chSepChar = kCIDLib::chForwardSlash;
        bBadChars |= pathTmp.bContainsChar(kCIDLib::chBackSlash)
                     || pathTmp.bContainsChar(kCIDLib::chPeriod)
                     || (pathTmp[0] != kCIDLib::chForwardSlash);
    }
     else
    {
        chSepChar = kCIDLib::chBackSlash;

        bBadChars |= pathTmp.bContainsChar(kCIDLib::chForwardSlash)
                     || !pathTmp.bIsFullyQualified();
        if (!bBadChars)
        {
            // Get it into a path string and normalize it
            pathTmp.Normalize();

            // Make sure it is still within the base path for its type
            TString strBasePath;
            facCQCRemBrws().BasePathForType(eType, strBasePath);
            if (!pathTmp.bStartsWithI(strBasePath))
            {
                facCQCDataSrv.ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kDSrvErrs::errcFAcc_TypePathUnderflow
                    , pathTmp
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Format
                );
            }
        }
    }

    if (bBadChars)
    {
        facCQCDataSrv.ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kDSrvErrs::errcFAcc_BadPathChars
            , pathTmp
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
        );
    }

    // Looks reasonable, so let's find the last slash in it
    tCIDLib::TCard4 c4At;
    if (!pathTmp.bLastOccurrence(chSepChar, c4At))
    {
        CIDAssert2(L"No slash found in path")
    }
    if (pathTmp.chLast() == chSepChar)
    {
        if (!pathTmp.bPrevOccurrence(chSepChar, c4At))
        {
            CIDAssert2(L"No last slash found in path")
        }
    }

    // Copy over the to the caller the stuff before the last slash
    strParPath.CopyInSubStr(pathTmp, 0, c4At);

    // And get the name part out
    pathTmp.bQueryName(strName);
}


//
//  Load a chunked file. We throw if it fails. We return an allocated chunked file
//  that the caller is responsible for. This is typically used when we have to do
//  something and the client asks us not to cache the data and it's not already
//  cached. So we have to temporarily load it.
//
//  No locking required here since we aren't dealing with the cache.
//
TChunkedFile* TDataSrvAccImpl::pchflLoadFile(const TString& strLocalPath)
{
    TChunkedFile* pchflLoad = nullptr;
    try
    {
        TBinFileInStream strmLoad
        (
            strLocalPath
            , tCIDLib::ECreateActs::OpenIfExists
            , tCIDLib::EFilePerms::Default
            , tCIDLib::EFileFlags::SequentialScan
        );

        pchflLoad = new TChunkedFile();
        TJanitor<TChunkedFile> janChunk(pchflLoad);
        strmLoad >> *pchflLoad;

        pchflLoad = janChunk.pobjOrphan();
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
    return pchflLoad;
}


//
//  See if there the passed path is in the cache and that it is of the indicated type.
//  If not there, we return null. If there but not the right type, we throw.
//
//  WE ASSUME THE CALLER has locked before calling us, which should always be
//  the case since he's dealing with files.
//
TDSCacheItem*
TDataSrvAccImpl::pdsciFindHierCache(const   TString&            strHPath
                                    , const tCIDLib::TBoolean   bIsScope)
{
    TTreeCache::TNode* pnodePath = m_colTreeCache.pnodeAt(strHPath);
    if (!pnodePath)
        return nullptr;

    if ((bIsScope && (pnodePath->eType() != tCIDLib::ETreeNodes::NonTerminal))
    ||  (!bIsScope && (pnodePath->eType() != tCIDLib::ETreeNodes::Terminal)))
    {
        facCQCDataSrv.ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kDSrvErrs::errcFAcc_WrongType
            , strHPath
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::TypeMatch
        );
    }
    return &pnodePath->objData();
}


//
//  Given a hierarchical (H) path (i.e. /Customize/...), we check to see if it is in
//  the cache.
//
//  Whether the target is a file or scope, we first have to create the path to
//  it, i.e. the list of scopes (non-terminal nodes) up to that target scope (or the
//  parent scope.) Any scopes between the target (or target's parent) that have not
//  been loaded yet, will be faulted in but their contents not loaded yet. We only
//  care about the final scope in the path.
//
//  If the target is a scope, we don't load its contents, we just make sure it's
//  in the cache. If the target is a file, then we load the contents of the parent
//  scope.
//
//  WE ASSUME THE CALLER has locked before calling us, which should always be
//  the case since he's dealing with files.
//
//  If it's a file, we can optionally go ahead and cache up the data if it is not already
//  cached. Else, if we have to add it, we'll do the minimum required to get the data
//  loaded that we need to put into the hierarchical data.
//
//  We return the cache item associated with the path
//
TDSCacheItem*
TDataSrvAccImpl::pdsciUpdateHierCache(  const   TString&            strHPath
                                        , const tCIDLib::TBoolean   bIsScope
                                        ,       tCIDLib::TBoolean&  bCacheFlag)
{
    #if defined(LOG_FILEACC)
    {
        TString strMsg(L"Updating tree cache. IsScope=");
        strMsg.Append(facCQCKit().strBoolYesNo(bIsScope));
        facCQCDataSrv.LogMsg
        (
            CID_FILE
            , CID_LINE
            , strMsg
            , strHPath
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
    }
    #endif

    // Save the incoming can cache flag and set it to false until we do actually cache
    const tCIDLib::TBoolean bCanCache = bCacheFlag;
    bCacheFlag = kCIDLib::False;

    // If it's already ijn the cache, make sure it's of the right type
    TTreeCache::TNode* pnodeTar = m_colTreeCache.pnodeAt(strHPath);
    if (pnodeTar)
    {
        CheckTreeNodeType(strHPath, pnodeTar, bIsScope);
        #if defined(LOG_FILEACC)
        {
            facCQCDataSrv.LogMsg
            (
                CID_FILE
                , CID_LINE
                , L"Path was already in the tree cache"
                , strHPath
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );
        }
        #endif

        // If if a file and we can cache and it's not already, then do so
        TDSCacheItem& dsciTar = pnodeTar->objData();
        if (!bIsScope && bCanCache && !dsciTar.bDataLoaded())
        {
            dsciTar.LoadFromFile();
            bCacheFlag = kCIDLib::True;
        }

        // And we can return the data now
        return &pnodeTar->objData();
    }

    //
    //  Convert it to a local path and see if it really exists. We will throw
    //  if it does not map to a valid file or directory.
    //
    TString strLocalPath;
    const tCQCRemBrws::EDTypes  eType = eConvertPath
    (
        strHPath, bIsScope, strLocalPath, kCIDLib::True
    );

    // See if the file exists locally. If not, then no need to go further
    if ((bIsScope && !TFileSys::bIsDirectory(strLocalPath))
    ||  (!bIsScope && !TFileSys::bExists(strLocalPath)))
    {
        facCQCDataSrv.ThrowErr
        (
            CID_FILE
            , CID_LINE
            , bIsScope ? kDSrvErrs::errcFAcc_ScopeNotFound
                       : kDSrvErrs::errcFAcc_FileNotFound
            , strLocalPath
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
        );
    }

    // Not found and it is a good path, so let's update the hierarchy
    if (bIsScope)
    {
        //
        //  It's a scope. So all we have to do is fault in the non-terminals up to
        //  this target. We can't just let the tree collection fault them in, which
        //  it can do, since we also need to set up the remote browser info that
        //  goes into the nodes.
        //
        pnodeTar = pnodeFaultInScope(strHPath, kCIDLib::False, eType, strLocalPath);
    }
     else
    {
        //
        //  It's a file, so we need to get it into the cache hierarchy at least, and
        //  load the data if we are told we can.
        //
        //  So start by checking the parent node and getting the node pointer. We should
        //  not need to fault it in since it should already be.
        //
        TTreeCache::TNodeNT* pnodePar = pnodeCheckParScope
        (
            strHPath, strLocalPath, eType, EFaultInOpts::IfLocalExists
        );

        // If the parent path doesn't exist, then we have to fail
        if (!pnodePar)
        {
            facCQCDataSrv.ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kDSrvErrs::errcFAcc_TarScopeNotFnd
                , strLocalPath
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::NotFound
            );
        }

        // And now see if the file is in the cache. If not, we need to add it
        pnodeTar = m_colTreeCache.pnodeAt(strHPath);
        if (!pnodeTar)
        {
            // Get out just the name part of the new file
            TPathStr pathTmp(strLocalPath);
            TString strName;
            pathTmp.bQueryName(strName);

            //
            //  Create a new cache item for this file. The user/system type
            //  of the parent node indicates what flags we set on the item.
            //
            tCQCRemBrws::EItemFlags eFlags;
            if (facCQCRemBrws().bIsSystemScope(strHPath, eType))
                eFlags = tCQCRemBrws::EItemFlags::SystemItem;
            else
                eFlags = tCQCRemBrws::EItemFlags::UserItem;

            pnodePar->objAddTerminal
            (
                TDSCacheItem(strName, eType, m_c4NextItemId++, strHPath, eFlags)
                , strName
                , TString::strEmpty()
                , kCIDLib::False
                , kCIDLib::True
            );

            pnodeTar = m_colTreeCache.pnodeAt(strHPath);
        }

        //
        //  If there data isn't yet loaded and we can cache then do that, else at
        //  least load the info we have to have
        //
        if (!pnodeTar->objData().bDataLoaded())
        {
            if (bCanCache)
            {
                pnodeTar->objData().LoadFromFile();
                bCacheFlag = kCIDLib::True;
            }
             else
            {
                pnodeTar->objData().ForceExtraMeta();
            }
        }
    }

    #if defined(LOG_FILEACC)
    // DumpCache();
    #endif

    // Return the cache item object
    return &pnodeTar->objData();
}


//
//  This method will take the passed DS path, remove the last component, and see if
//  that path is in the hierarchy cache. If not, it will optionally fault it in, also
//  optionally only if the local equivalent path also exists.
//
//  It will make sure that the target scope is not above the base path for the file
//  type. It's assumed the caller already checked the path for validaty and passes
//  us the equivalent local path.
//
//  WE ASSUME the caller has locked!
//
TDataSrvAccImpl::TTreeCache::TNodeNT*
TDataSrvAccImpl::pnodeCheckParScope(const   TString&                strHPath
                                    , const TString&                strLocalPath
                                    , const tCQCRemBrws::EDTypes    eType
                                    , const EFaultInOpts            eFaultInOpt)
{
    // Get the parent part of the data server path
    TString strParHPath, strDSName;
    MakeParentPath(strHPath, eType, strParHPath, strDSName, kCIDLib::True);

    // Make sure that we didn't end up above the base path for this file type
    if (!strParHPath.bStartsWithI(facCQCRemBrws().strFlTypePref(eType)))
    {
        facCQCDataSrv.ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kDSrvErrs::errcFAcc_TypePathUnderflow
            , strHPath
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
        );
    }

    // See if this guy is in the cache
    TTreeCache::TNodeNT* pnodeRet = nullptr;
    {
        TTreeCache::TNode* pnodeTest = m_colTreeCache.pnodeAt(strParHPath);
        if (pnodeTest)
        {
            CIDAssert
            (
                pnodeTest->eType() == tCIDLib::ETreeNodes::NonTerminal
                , L"The parent scope is not a non-terminal node"
            )
            pnodeRet = static_cast<TTreeCache::TNodeNT*>(pnodeTest);
        }
    }

    // Convert the local path to get the parent as well
    TString strParLocPath, strLocName;
    MakeParentPath(strLocalPath, eType, strParLocPath, strLocName, kCIDLib::False);

    // If not, see if we can fault it in
    if (!pnodeRet && (eFaultInOpt != EFaultInOpts::Never))
    {
        //
        //  If we always fault it in, or if the local directory for this scope
        //  exists, then fault it in.
        //
        if ((eFaultInOpt == EFaultInOpts::Always) || TFileSys::bIsDirectory(strParLocPath))
            pnodeRet = pnodeFaultInScope(strParHPath, kCIDLib::True, eType, strParLocPath);
    }

    // If the scope is there, but its contents hasn't been faulted in yet, do that
    if (pnodeRet && !pnodeRet->objData().bChildItemsLoaded())
        pnodeRet = pnodeFaultInScope(strParHPath, kCIDLib::True, eType, strParLocPath);

    return pnodeRet;
}


//
//  This is called to fault in parts of the cache hierarchy. We get a scope path. Do
//  two things, one optional:
//
//  1.  We fault in all of the non-terminal nodes that represent the scopes down
//      to the target, setting up their info and marking them as not-loaded.
//  2.  If asked, we will load up the contents of the final, target scope, and mark
//      it loaded. It's not recursive, we only load up the actual contents.
//
//  WE ASSUME THE CALLER has proven that this data server path actually maps to a
//  real local scope.
//
//  WE ASSUME THE CALLER has locked before calling us, which he should have if he
//  is doing anything that requires this be called.
//
//  WE have to deal with the fact that, if we are loading the contents of the target
//  scope, that some of its children may have already been loaded previously as
//  we filled in parts of the hiearchy required to get down to something below it.
//  So we check whether they exist before trying to adding them to the tree.
//
TDataSrvAccImpl::TTreeCache::TNodeNT*
TDataSrvAccImpl::pnodeFaultInScope( const   TString&                strHScope
                                    , const tCIDLib::TBoolean       bLoadContents
                                    , const tCQCRemBrws::EDTypes    eType
                                    , const TString&                strLocalPath)
{
    #if defined(LOG_FILEACC)
    {
        TString strMsg(L"Faulting in scope. LoadCont=");
        strMsg.Append(facCQCKit().strBoolYesNo(bLoadContents));
        strMsg.Append(L", Type=");
        strMsg.Append(tCQCRemBrws::strLoadEDTypes (eType));
        facCQCDataSrv.LogMsg
        (
            CID_FILE
            , CID_LINE
            , strMsg
            , strHScope
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
    }
    #endif

    // Get the root node, to start our work
    TTreeCache::TNodeNT* pnodeCurPar = m_colTreeCache.pnodeRoot();

    //
    //  Break out the passed scope into separate components, so that we trace
    //  our way down through the hierarchy and create the non-terminals required.
    //
    TString strScope;
    TStringTokenizer stokPath(&strHScope, L"/");
    TTreeCache::TNode* pnodeChild = nullptr;
    TString strCurPath;
    while (stokPath.bGetNextToken(strScope))
    {
        // Get the named child of the current node
        pnodeChild = pnodeCurPar->pnodeFind(strScope, kCIDLib::False);
        strCurPath.Append(L"/");
        strCurPath.Append(strScope);

        //
        //  If found, insure it's a scope, else this is a bad path. Shouldn't happen,
        //  see the method comments, but if debugging do so.
        //
        #if CID_DEBUG_ON
        if (pnodeChild && (pnodeChild->eType() == tCIDLib::ETreeNodes::Terminal))
        {
            facCQCDataSrv.ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kDSrvErrs::errcFAcc_TermNodeInPath
                , strHScope
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::AppError
            );
        }
        #endif

        // If not found, then we need to add it
        if (!pnodeChild)
        {
            TTreeCache::TNodeNT* pnodeNewScope = pnodeCurPar->pnodeAddNonTerminal
            (
                strScope, TString::strEmpty(), kCIDLib::False, kCIDLib::True
            );

            //
            //  Set up the remote browser info. Figure out the flags we want to
            //  set on this one. At this point, any special scopes have been
            //  added to the cache, since they are forced in up front. So the only
            //  is is whether this one is in the System or User parts of the tree.
            //  If system, then we mark it as a system scope, else as a user scope.
            //
            tCQCRemBrws::EItemFlags eFlags;
            if (facCQCRemBrws().bIsSystemScope(strCurPath, eType))
                eFlags = tCQCRemBrws::EItemFlags::SystemScope;
            else
                eFlags = tCQCRemBrws::EItemFlags::UserScope;

            pnodeNewScope->objData().dsbiCache().Set
            (
                strScope
                , eType
                , m_c4NextScopeId++
                , m_c4NextItemId++
                , tCQCKit::EUserRoles::LimitedUser
                , eFlags
            );

            // Make this guy the new current parent
            pnodeCurPar = pnodeNewScope;
        }
         else
        {
            //
            //  Cast this one to the new current parent. We checked above it really
            //  is a non-terminal node.
            //
            pnodeCurPar = static_cast<TTreeCache::TNodeNT*>(pnodeChild);
        }
    }

    //
    //  If asked, let's load up the contents of this scope. The caller passes us
    //  the local path that he already mapped it to, in order to verify it's a
    //  valid path.
    //
    if (bLoadContents)
    {
        // Remember now if the target is a system scope
        const tCIDLib::TBoolean bSysScope
        (
            facCQCRemBrws().bIsSystemScope(strHScope, eType)
        );

        // Append a trailing slash if not already there
        if (strCurPath.chLast() != kCIDLib::chForwardSlash)
            strCurPath.Append(kCIDLib::chForwardSlash);

        const tCIDLib::TCard4 c4ParLen = strCurPath.c4Length();
        TDirIter diterLoad;
        TFindBuf fndbCur;

        // First do sub-scopes
        if (diterLoad.bFindFirst(strLocalPath
                                , kCIDLib::pszAllDirsSpec
                                , fndbCur
                                , tCIDLib::EDirSearchFlags::NormalDirs))
        {
            TString strName;
            do
            {
                fndbCur.pathFileName().bQueryName(strName);

                // If it isn't already in the cache, add it
                if (!pnodeCurPar->pnodeFind(strName, kCIDLib::False))
                {
                    // It's a directory, so add a non-terminal
                    TTreeCache::TNodeNT* pnodeNew = pnodeCurPar->pnodeAddNonTerminal
                    (
                        strName, TString::strEmpty(), kCIDLib::False, kCIDLib::True
                    );

                    // Set up the browser info
                    strCurPath.CapAt(c4ParLen);
                    strCurPath.Append(strName);
                    pnodeNew->objData().dsbiCache().Set
                    (
                        strName
                        , eType
                        , m_c4NextScopeId++
                        , m_c4NextItemId++
                        , tCQCKit::EUserRoles::LimitedUser
                        , bSysScope ? tCQCRemBrws::EItemFlags::SystemScope
                                    : tCQCRemBrws::EItemFlags::UserScope
                    );
                }
            }   while(diterLoad.bFindNext(fndbCur));
        }

        // And now do the files
        TPathStr pathFileSpec(L"*");
        pathFileSpec.AppendExt(facCQCRemBrws().strFlTypeExt(eType));
        if (diterLoad.bFindFirst(strLocalPath
                                , pathFileSpec
                                , fndbCur
                                , tCIDLib::EDirSearchFlags::NormalFiles))
        {
            TString strName;
            do
            {
                fndbCur.pathFileName().bQueryName(strName);

                // If it isn't already in the cache, add it
                if (!pnodeCurPar->pnodeFind(strName, kCIDLib::False))
                {
                    // It's a file, so a terminal node
                    strCurPath.CapAt(c4ParLen);
                    strCurPath.Append(strName);
                    TDSCacheItem dsciAdd
                    (
                        strName
                        , eType
                        , m_c4NextItemId++
                        , strCurPath
                        , bSysScope ? tCQCRemBrws::EItemFlags::SystemItem
                                    : tCQCRemBrws::EItemFlags::UserItem
                    );

                    TDSCacheItem& dsciNew = pnodeCurPar->objAddTerminal
                    (
                        dsciAdd, strName, TString::strEmpty(), kCIDLib::False, kCIDLib::True
                    );

                    //
                    //  In this case, since we are not caching the data yet, we need to
                    //  ask him to load up the extra metadata at least, since that has to
                    //  be there for hierarchical purposes.
                    //
                    dsciNew.ForceExtraMeta();
                }
            }   while(diterLoad.bFindNext(fndbCur));
        }

        // Mark it loaded now
        pnodeCurPar->objData().bChildItemsLoaded(kCIDLib::True);
    }
    return pnodeCurPar;
}


//
//  Finds the transaction operation for the passed cookie. This lets us find the
//  previous transaction in a multi-transaction transfer of an image.
//
//  WE ASSUME the caller locked.
//
TDataSrvAccImpl::TTransOp*
TDataSrvAccImpl::ptopForCookie(const tCIDLib::TCard4 c4Cookie)
{
    TRefBag<TTransOp>::TNCCursor cursFind(&m_colTransOps);
    if (cursFind.bIsValid())
    {
        const tCIDLib::TEncodedTime enctCur = TTime::enctNow();
        tCIDLib::TCard4 c4Index = 0;
        for (; cursFind; ++cursFind)
        {
            TTransOp& topCur = *cursFind;

            // If it's our guy, then check it
            if (topCur.m_c4Cookie == c4Cookie)
            {
                // See if he's timed out
                if (topCur.m_enctEndTime < enctCur)
                {
                    m_colTransOps.RemoveAt(cursFind);
                    return nullptr;
                }
                return &topCur;
            }
        }
    }

    facCQCDataSrv.ThrowErr
    (
        CID_FILE
        , CID_LINE
        , kDSrvErrs::errcFAcc_NoTransOp
        , tCIDLib::ESeverities::Status
        , tCIDLib::EErrClasses::AppStatus
        , TCardinal(c4Cookie)
    );

    // Make the compiler happy
    return nullptr;
}


//
//  Finds a free transaction operation object in the cache, or creates a new one if
//  no free ones are already there. We assume the caller has locked.
//
//  WE ASSUME the caller locked.
//
TDataSrvAccImpl::TTransOp*
TDataSrvAccImpl::ptopGetFreeDn( const   TString&            strHPath
                                , const tCIDLib::TCard4     c4DataChunkSz
                                , const tCIDLib::TCard4     c4FirstBlockSz
                                , const TMemBuf&            mbufDataChunk
                                , const tCIDLib::TCard4     c4Flags)
{
    //
    //  Search the collection for an item that's there, but has timed out. If
    //  we find one, steal it, put a new cookie in it, reset the start time,
    //  and replace it's storage item with the new one.
    //
    TRefBag<TTransOp>::TNCCursor cursFind(&m_colTransOps);
    const tCIDLib::TEncodedTime enctCurTime = TTime::enctNow();
    for (; cursFind; ++cursFind)
    {
        TTransOp& topCur = *cursFind;

        //
        //  If this one is out of time, let's steal it. Even if the client is
        //  still doing a transfer, it would have timed out next time anyway.
        //
        if (topCur.m_enctEndTime < enctCurTime)
        {
            #if defined(LOG_FILEACC)
            {
                facCQCDataSrv.LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , L"Taking existing transfer op item"
                    , strHPath
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::AppStatus
                );
            }
            #endif

            topCur.m_bUpload = kCIDLib::False;
            topCur.m_c4Flags = c4Flags;
            topCur.m_c4Cookie = ++m_c4NextCookie;
            topCur.m_enctEndTime = enctCurTime + kCIDLib::enctFiveSeconds;
            topCur.m_c4FullSz = c4DataChunkSz;
            topCur.m_c4NextOfs = c4FirstBlockSz;
            topCur.m_strHPath = strHPath;

            // No extension chunk for downloads it was returned in the first call
            topCur.m_c4ExtSz = 0;
            topCur.m_strExtChunkId.Clear();

            // No meta values for downloads, it was returned in the first call
            topCur.m_colMetaUp.RemoveAll();

            // For a download, store away all of the data chunk for downloading
            topCur.m_mbufData.Reallocate(c4DataChunkSz, kCIDLib::False);
            topCur.m_mbufData.CopyIn(mbufDataChunk, c4DataChunkSz);

            return &topCur;
        }
    }

    // We didn't find one, so allocate one
    TTransOp* ptopRet = new TTransOp
    (
        ++m_c4NextCookie
        , strHPath
        , c4DataChunkSz
        , c4FirstBlockSz
        , mbufDataChunk
        , c4Flags
    );
    m_colTransOps.Add(ptopRet);

    return ptopRet;
}


TDataSrvAccImpl::TTransOp*
TDataSrvAccImpl::ptopGetFreeUp( const   TString&            strHPath
                                , const tCIDLib::TCard4     c4DataChunkSz
                                , const tCIDLib::TCard4     c4FirstBlockSz
                                ,       THeapBuf&&          mbufFirstBlock
                                , const TString&            strExtChunkId
                                , const tCIDLib::TCard4     c4ExtChunkSz
                                ,       THeapBuf&&          mbufExtChunk
                                , const tCIDLib::TKVPFList& colMetaVals
                                , const tCIDLib::TCard4     c4Flags)
{
    //
    //  Search the collection for an item that's there, but has timed out. If
    //  we find one, steal it, put a new cookie in it, reset the start time,
    //  and replace it's storage item with the new one.
    //
    TRefBag<TTransOp>::TNCCursor cursFind(&m_colTransOps);
    const tCIDLib::TEncodedTime enctCurTime = TTime::enctNow();
    for (; cursFind; ++cursFind)
    {
        TTransOp& topCur = *cursFind;

        //
        //  If this one is out of time, let's steal it. Even if the client is
        //  still doing a transfer, it would have timed out next time anyway.
        //
        if (topCur.m_enctEndTime < enctCurTime)
        {
            #if defined(LOG_FILEACC)
            {
                facCQCDataSrv.LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , L"Taking existing transfer op item"
                    , strHPath
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::AppStatus
                );
            }
            #endif

            topCur.m_bUpload = kCIDLib::True;
            topCur.m_c4Flags = c4Flags;
            topCur.m_c4Cookie = ++m_c4NextCookie;
            topCur.m_enctEndTime = enctCurTime + kCIDLib::enctFiveSeconds;
            topCur.m_c4FullSz = c4DataChunkSz;
            topCur.m_c4NextOfs = c4FirstBlockSz;
            topCur.m_strExtChunkId = strExtChunkId;
            topCur.m_c4ExtSz = c4ExtChunkSz;
            topCur.m_strHPath = strHPath;
            topCur.m_colMetaUp = colMetaVals;

            // For an upload copy in the initial block's worth
            topCur.m_mbufData = tCIDLib::ForceMove(mbufFirstBlock);

            // And the extension block if any
            if (c4ExtChunkSz)
                topCur.m_mbufExtChunk = tCIDLib::ForceMove(mbufExtChunk);

            return &topCur;
        }
    }

    // We didn't find one, so allocate one
    TTransOp* ptopRet = new TTransOp
    (
        ++m_c4NextCookie
        , strHPath
        , c4DataChunkSz
        , colMetaVals
        , c4FirstBlockSz
        , mbufFirstBlock
        , strExtChunkId
        , c4ExtChunkSz
        , mbufExtChunk
        , c4Flags
    );
    m_colTransOps.Add(ptopRet);

    return ptopRet;
}


//
//  This is called to store incoming file data.
//
//  We assume that the caller has locked before calling this, since we have to deal
//  with the cache. It's possible we won't cache it (because the flags indicate the
//  caller doesn't want that and we haven't already loaded the data previously.) But
//  the caller can't know that so he has to lock.
//
//  When we first got the upload we faulted in the parent scope and its contents, so
//  we know that is there. If this is not a new file, it should already be in the
//  hierarchy cache at least because of that.
//
tCIDLib::TVoid
TDataSrvAccImpl::WriteData( const   TString&                strLocalFile
                            , const TString&                strHPath
                            , const TString&                strRelPath
                            , const tCQCRemBrws::EDTypes    eType
                            ,       tCIDLib::TCard4&        c4SerialNum
                            , const tCIDLib::TKVPFList&     colMetaVals
                            , const tCIDLib::TCard4         c4DataChunkSz
                            , const TMemBuf&                mbufDataChunk
                            , const TString&                strExtChunkId
                            , const tCIDLib::TCard4         c4ExtChunkSz
                            , const TMemBuf&                mbufExtChunk
                            , const tCIDLib::TCard4         c4Flags
                            ,       tCIDLib::TEncodedTime&  enctLastChange)
{
    try
    {
        #if defined(LOG_FILEACC)
        {
            TString strMsg(L"Writing data to local file. Path=", strHPath.c4Length() + 8);
            strMsg.Append(strHPath);
            facCQCDataSrv.LogMsg
            (
                CID_FILE
                , CID_LINE
                , strMsg
                , strLocalFile
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );
        }
        #endif


        //
        //  The local parent directory has to exist. This was checked when we started
        //  the file upload process, so it should be there. Any directories must be
        //  created explicitly by the clients. If it's not there someone removed it
        //  from behind our back so throw.
        //
        {
            TPathStr pathTmp(strLocalFile);
            pathTmp.bRemoveNameExt();
            if (!TFileSys::bIsDirectory(pathTmp))
            {
                facCQCDataSrv.ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kDSrvErrs::errcFAcc_TarScopeVanished
                    , pathTmp
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::NotFound
                );
            }
        }

        // A convenience for whether the caller wants caching or not
        const tCIDLib::TBoolean bCanCache((c4Flags & kCQCRemBrws::c4Flag_NoDataCache) == 0);

        // See if the local file exists
        const tCIDLib::TBoolean bNewFile = !TFileSys::bExists(strLocalFile);

        // If debugging, make sure that it's not in the hiearchy cache if new
        CIDAssert
        (
            !bNewFile || !m_colTreeCache.bNodeExists(strHPath)
            , L"A newly uploaded file was already in the cache"
        );

        //
        //  Write it out successfully before we do anything else. If this
        //  fails we don't want to have bogus info in the cache.
        //
        {
            TChunkedFile chflTmp
            (
                colMetaVals
                , c4DataChunkSz
                , mbufDataChunk
                , strExtChunkId
                , c4ExtChunkSz
                , mbufExtChunk
                , kCIDLib::False
            );

            TString strTempPath(strLocalFile);
            strTempPath.Append(L"_New");
            {
                TBinFileOutStream strmOut
                (
                    strTempPath
                    , tCIDLib::ECreateActs::CreateAlways
                    , tCIDLib::EFilePerms::Default
                    , tCIDLib::EFileFlags::SequentialScan
                );
                strmOut << chflTmp << kCIDLib::FlushIt;
            }
            TFileSys::ReplaceFile(strLocalFile, strTempPath);
        }

        //
        //  Make sure it is cached from a hierarchy standpoint one way or another. We
        //  wrote it out above if new, so this guy won't complain. If caching is allowed
        //  tell it to cache the data. If it had to add it, but can't cache, it'll do
        //  at least the minimum load of meta info needed by the hierarchy stuff.
        //
        tCIDLib::TBoolean bNewlyCached = bCanCache;
        TDSCacheItem* pdsciTar = pdsciUpdateHierCache(strHPath, kCIDLib::False, bNewlyCached);

        if (pdsciTar->bDataLoaded())
        {
            //
            //  The data is cached. If not just done above, then we need to update it
            //  since it's previously loaded data. This will write out the new stuff.
            //
            if (!bNewlyCached)
            {
                pdsciTar->UpdateData
                (
                    colMetaVals
                    , c4DataChunkSz
                    , mbufDataChunk
                    , strExtChunkId
                    , c4ExtChunkSz
                    , mbufExtChunk
                );
            }

            // And give back the new serial number and last change time
            c4SerialNum = pdsciTar->c4SerialNum();
            enctLastChange = pdsciTar->enctLastFileChange();
        }
         else
        {
            //
            //  We don't have cached info to work with, so read in the serial number
            //  and last time stamp info from the file. The chunked file class has a
            //  helper to do this pretty efficiently.
            //
            TBinFileInStream strmIn
            (
                strLocalFile
                , tCIDLib::ECreateActs::OpenIfExists
                , tCIDLib::EFilePerms::Default
                , tCIDLib::EFileFlags::SequentialScan
            );
            tCIDLib::TCard4 c4ChunkCnt;
            TChunkedFile::ExtractHdrInfo(strmIn, c4SerialNum, enctLastChange, c4ChunkCnt);
        }

        #if defined(LOG_FILEACC)
        {
            TString strMsg(bNewFile ? L"New file written. " : L"File updated. ", 32UL);
            strMsg.Append(L"New serial number=");
            strMsg.AppendFormatted(c4SerialNum);
            facCQCDataSrv.LogMsg
            (
                CID_FILE
                , CID_LINE
                , strMsg
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );
        }
        #endif

        //  If a new file, the parent scope contents id must be bumped
        if (bNewFile)
            pdsciTar->c4BumpScopeId();
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}




// If debugging, we have a helper to dump the cache info out to the console
#if defined(LOG_FILEACC)
tCIDLib::TVoid
TDataSrvAccImpl::DumpLevel(         TTextOutStream&             strmTar
                            , const TTreeCache::TNodeNT* const  pnodePar
                            , const tCIDLib::TCard4             c4Level
                            ,       TTime&                      tmAccess
                            ,       TTime&                      tmChange)
{
    // First, do all of the terminal nodes
    TTreeCache::TNode* pnodeCur = pnodePar->pnodeFirstChild();

    // Now do all of the non-terminals
    pnodeCur = pnodePar->pnodeFirstChild();
    while (pnodeCur)
    {
        // If it's a non-terminal, then recurse
        if (pnodeCur->eType() == tCIDLib::ETreeNodes::NonTerminal)
        {
            const TDSCacheItem& dsciCur = pnodeCur->objData();

            // Indent over for this level
            strmTar << TTextOutStream::Spaces(c4Level * 3);

            strmTar << pnodeCur->strName()
                    << L" (IID=" << dsciCur.dsbiCache().c4IdItem()
                    << L", SID=" << dsciCur.dsbiCache().c4IdScope()
                    << L")";

            if (!dsciCur.bChildItemsLoaded())
                strmTar << L" [sparse]";
            strmTar << kCIDLib::NewLn;

            // Recurse on this level
            DumpLevel
            (
                strmTar
                , static_cast<const TTreeCache::TNodeNT*>(pnodeCur)
                , c4Level + 1
                , tmAccess
                , tmChange
            );
        }

        // Move forward
        pnodeCur = pnodeCur->pnodeNext();
    }

    // And do the terminals
    pnodeCur = pnodePar->pnodeFirstChild();
    while (pnodeCur)
    {
        if (pnodeCur->eType() == tCIDLib::ETreeNodes::Terminal)
        {
            const TDSCacheItem& dsciCur = pnodeCur->objData();

            // Indent over for this level
            strmTar << TTextOutStream::Spaces(c4Level * 3);

            strmTar << pnodeCur->strName()
                    << L" (IID=" << pnodeCur->objData().dsbiCache().c4IdItem()
                    << L", Data=";


            if (dsciCur.bDataLoaded())
            {
                tmAccess = dsciCur.enctLastAccess();
                tmChange = dsciCur.enctLastFileChange();
                strmTar << L"Ser#" << dsciCur.c4SerialNum()
                        << L", LastAcc: " << tmAccess
                        << L", LastChange: " << tmChange;
            }
             else
            {
                strmTar << L"[NotCached]";
            }

            strmTar << L")\n";
        }

        // Move forward
        pnodeCur = pnodeCur->pnodeNext();
    }
}

tCIDLib::TVoid
TDataSrvAccImpl::DumpCache(const tCIDLib::TBoolean bToFile)
{
    TTextOutStream* pstrmTar = nullptr;
    if (bToFile)
    {
        try
        {
            TPathStr pathOut;
            TProcEnvironment::bFind(L"CQC_RESDIR", pathOut);
            pathOut.AddLevel(L"DSCacheDump.Txt");

            pstrmTar = new TTextFileOutStream
            (
                pathOut
                , tCIDLib::ECreateActs::CreateAlways
                , tCIDLib::EFilePerms::Default
                , tCIDLib::EFileFlags::SequentialScan
                , tCIDLib::EAccessModes::Write
                , new TUSASCIIConverter()
            );
        }

        catch(...)
        {
            pstrmTar = new TOutConsole;
        }
    }
     else
    {
        pstrmTar = new TOutConsole;
    }
    TJanitor<TTextOutStream> janStrm(pstrmTar);

    // Provide two time objects for the recursive helper to use, with the right formatting
    TString strFmt(L"%(D,2,0)/%(M,2,0)/%(y,2,0)-%(H,2,0):%(u,2,0):%(s,2,0)");
    TTime   tmAccess;
    TTime   tmChange;
    tmAccess.strDefaultFormat(strFmt);
    tmChange.strDefaultFormat(strFmt);

    TTreeCache::TNodeNT* pnodeCurPar = m_colTreeCache.pnodeRoot();
    *pstrmTar <<  L"-------------------------------------\n"
                  L"Data Server Cache\n"
                  L"-------------------------------------\n"
                  L"/\n";
    DumpLevel(*pstrmTar, pnodeCurPar, 1, tmAccess, tmChange);
    *pstrmTar << kCIDLib::DNewLn;

    *pstrmTar << kCIDLib::NewEndLn;
}
#endif
