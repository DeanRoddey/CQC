//
// FILE NAME: CQCDataSrv_FileAccServerImpl.hpp
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
//  This file implements the server side of the ORB interface that is used by clients
//  to access the customization data that the data server stores. It is fairly
//  simple in terms of the public interface, but pretty complex internally.
//
//  We maintain a cache that serves two purposes. One is provide information about
//  the hierarchical information, which is mostly used by the tree browser window that
//  is used in the AI's main interface and some other resource browsing dialogs.
//
//  It also serves as a data cache, to hold information in memory for quick access.
//  The data cache works in terms of TChunkedFile objects. We don't care what is in
//  them. The clients that manipulate the files handle that. It is a flexible format
//  that they can use in various ways to store what they want.
//
//  However, there is currently an assumption that any files we deal with will have at
//  most the main data chunk and one extension chunk. Most will only have the main data
//  chunk (and of course the metadata chunk which all have.) We can serve up these
//  three bits of info from the data cache.
//
//  Files are often loaded up for hierarchy purposes without the data being cached.
//  The data will be faulted in if someone actually asks for it (and indicates it's
//  ok to cache it, sometimes they tell us not to.)
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TDSCacheItem
//  PREFIX: dsci
// ---------------------------------------------------------------------------
class TDSCacheItem
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TDSCacheItem
        (
            const   TString&                strScopeName
            , const tCQCRemBrws::EDTypes    eType
            , const tCIDLib::TCard4         c4ScopeId
            , const tCIDLib::TCard4         c4ItemId
            , const TString&                strHPath
            , const tCQCRemBrws::EItemFlags eFlags
            , const tCIDLib::TBoolean       bUserFlag
        );

        TDSCacheItem
        (
            const   TString&                strFileName
            , const tCQCRemBrws::EDTypes    eType
            , const tCIDLib::TCard4         c4ItemId
            , const TString&                strHPath
            , const tCQCRemBrws::EItemFlags eFlags
        );

        TDSCacheItem
        (
            const   TDSCacheItem&           dsciSrc
        );

        TDSCacheItem(TDSCacheItem&&) = delete;

        ~TDSCacheItem();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TDSCacheItem& operator=
        (
            const   TDSCacheItem&           dsciSrc
        );


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bDataLoaded() const
        {
            return (m_pchflData != nullptr);
        }

        tCIDLib::TBoolean bChildItemsLoaded() const;

        tCIDLib::TBoolean bChildItemsLoaded
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TBoolean bQueryExtChunk
        (
                    TString&                strChunkId
            ,       TMemBuf&                mbufExtData
            ,       tCIDLib::TCard4&        c4DataSz
        )   const;

        tCIDLib::TBoolean bSetChunk
        (
            const   TString&                strChunkId
            , const tCIDLib::TCard4         c4Bytes
            , const TMemBuf&                mbufToSet
        );

        tCIDLib::TBoolean bSetMetaVal
        (
            const   TKeyValuePair&          kvalToSet
            ,       tCIDLib::TBoolean&      bFileChange
        );

        tCIDLib::TBoolean bUnsavedChanges() const;

        tCIDLib::TVoid BumpLastAccess();

        tCIDLib::TCard4 c4BumpItemId();

        tCIDLib::TCard4 c4BumpScopeId();

        tCIDLib::TCard4 c4FullSzEstimate() const;

        tCIDLib::TCard4 c4ReadChunk
        (
            const   TString&                strChunkId
            ,       TMemBuf&                mbufToFill
        )   const;

        tCIDLib::TCard4 c4Rename
        (
            const   TString&                strName
            , const TString&                strPath
        );

        tCIDLib::TCard4 c4SerialNum() const;

        const TChunkedFile& chflData() const;

        tCIDLib::TEncodedTime enctLastAccess() const;

        tCIDLib::TEncodedTime enctLastFileChange() const;

        tCQCRemBrws::EDTypes eType() const
        {
            return m_dsbiCache.eType();
        }

        const TDSBrowseItem& dsbiCache() const
        {
            return m_dsbiCache;
        }

        TDSBrowseItem& dsbiCache()
        {
            return m_dsbiCache;
        }

        tCIDLib::TVoid ForceExtraMeta();

        tCIDLib::TVoid LoadFromFile();

        const TMemBuf& mbufForChunk
        (
            const   TString&                strChunkId
            ,       tCIDLib::TCard4&        c4Bytes
        )   const;

        const TString& strHPath() const
        {
            return m_strHPath;
        }

        tCIDLib::TVoid StoreExtraMeta
        (
            const   tCIDLib::TKVPFList&     colVals
        );

        tCIDLib::TVoid UncacheData();

        tCIDLib::TVoid UpdateChunk
        (
            const   TString&                strChunkId
            , const tCIDLib::TCard4         c4DataBytes
            , const TMemBuf&                mbufDataChunk
        );

        tCIDLib::TVoid UpdateData
        (
            const   tCIDLib::TKVPFList&     colMeta
            , const tCIDLib::TCard4         c4DataBytes
            , const TMemBuf&                mbufDataChunk
            , const TString&                strExtChunkId
            , const tCIDLib::TCard4         c4ExtBytes
            , const TMemBuf&                mbufExtChunk
        );


    protected :
        // -------------------------------------------------------------------
        //  Declare our friends and hidden constructors
        // -------------------------------------------------------------------
        TDSCacheItem();
        friend class TBasicTreeNode<TDSCacheItem>;


        // -------------------------------------------------------------------
        //  Protected, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid CheckDataIsCached() const;

        tCIDLib::TVoid UpdateMeta();

        tCIDLib::TVoid WriteToFile();


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bLoaded
        //      If this is a scope (non-terminal node), then we may have only loaded
        //      enough of it to create the tree structure down to the actual scopes
        //      that are being accessed. This flag tells us if we have actually loaded
        //      the structure underneath this scope or not, so that we only have to
        //      fault it in if its contents are actually directly accessed.
        //
        //  m_dsbiCache
        //      This is the remote browser item that we have to serve up for any
        //      hierarchical browsing requests.
        //
        //  m_enctLastAccess
        //      The last time this data was read or written. For writes, we update it
        //      ourself. For accesses, the server access object has to call our
        //      BumpLastAccess() method to let us know.
        //
        //  m_pchflData
        //      The chunked file data object that holds the actual data. We fault it
        //      in as required. If null then the data is not yet cached for this item.
        //
        //  m_strHPath
        //      The full data server (virtual) path to this entry.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bLoaded;
        TDSBrowseItem           m_dsbiCache;
        tCIDLib::TEncodedTime   m_enctLastAccess;
        TChunkedFile*           m_pchflData;
        TString                 m_strHPath;
};



// ---------------------------------------------------------------------------
//   CLASS: TDataSrvAccImpl
//  PREFIX: orbs
// ---------------------------------------------------------------------------
class TDataSrvAccImpl : public TDataSrvAccServerBase
{
    public :
        // -------------------------------------------------------------------
        //  We need a small class to hold file data during the transfer of files
        //  larger than our transfer chunk size. For uploads we also have to keep
        //  any extension chunk that was sent in the initial call, since we can't
        //  put it to use until we get the main data chunk completed.
        // -------------------------------------------------------------------
        class TTransOp
        {
            public :
                TTransOp() = delete;

                // For downloads
                TTransOp
                (
                    const   tCIDLib::TCard4         c4Cookie
                    , const TString&                strHPath
                    , const tCIDLib::TCard4         c4DataChunkSz
                    , const tCIDLib::TCard4         c4InitSz
                    , const TMemBuf&                mbufDataChunk
                    , const tCIDLib::TCard4         c4Flags
                );

                // For uploads
                TTransOp
                (
                    const   tCIDLib::TCard4         c4Cookie
                    , const TString&                strHPath
                    , const tCIDLib::TCard4         c4DataChunkSz
                    , const tCIDLib::TKVPFList&     colMetaVals
                    , const tCIDLib::TCard4         c4FirstBlockSz
                    , const TMemBuf&                mbufFirstBlock
                    , const TString&                strExtChunkId
                    , const tCIDLib::TCard4         c4ExtSz
                    , const TMemBuf&                mbufExt
                    , const tCIDLib::TCard4         c4Flags
                );

                TTransOp(const TTransOp&) = delete;
                TTransOp(TTransOp&&) = delete;

                ~TTransOp();

                TTransOp& operator=(const TTransOp&) = delete;
                TTransOp& operator=(TTransOp&&) = delete;

                tCIDLib::TCard4         m_c4Flags;
                tCIDLib::TBoolean       m_bUpload;
                tCIDLib::TCard4         m_c4Cookie;
                tCIDLib::TCard4         m_c4ExtSz;
                tCIDLib::TCard4         m_c4FullSz;
                tCIDLib::TCard4         m_c4NextOfs;
                tCIDLib::TKVPFList      m_colMetaUp;
                tCIDLib::TEncodedTime   m_enctEndTime;
                THeapBuf                m_mbufData;
                THeapBuf                m_mbufExtChunk;
                TString                 m_strExtChunkId;
                TString                 m_strHPath;
        };


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TDataSrvAccImpl() = delete;

        TDataSrvAccImpl
        (
            const   tCIDLib::TCard4         c4DSCacheSz
        );

        TDataSrvAccImpl(const TDataSrvAccImpl&) = delete;
        TDataSrvAccImpl(TDataSrvAccImpl&&) = delete;

        ~TDataSrvAccImpl();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TDataSrvAccImpl& operator=(const TDataSrvAccImpl&) = delete;
        TDataSrvAccImpl& operator=(TDataSrvAccImpl&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bFind
        (
            const   TString&                strHPath
            ,       TFindBuf&               fndbToFill
            , const tCIDLib::TBoolean       bIsScope
        )   final;

        tCIDLib::TBoolean bExists
        (
            const   TString&                strHPath
            , const tCIDLib::TBoolean       bIsScope
        )   final;

        tCIDLib::TBoolean bIsScope
        (
            const   TString&                strHToCheck
        )   final;

        tCIDLib::TBoolean bQueryExtChunk
        (
                    tCIDLib::TCard4&        c4SerialNum
            , const TString&                strHPath
            , const TString&                strExtChunkId
            ,       tCIDLib::TCard4&        c4BufSz
            ,       THeapBuf&               mbufExtChunk
            , const TCQCSecToken&           sectUser
            , const tCIDLib::TBoolean       bNoCache
        )   final;

        tCIDLib::TBoolean bQueryFileFirst
        (
                    tCIDLib::TCard4&        c4SerialNum
            , const TString&                strHPath
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
            , const TCQCSecToken&           sectUser
        )   final;

        tCIDLib::TBoolean bQueryFileNext
        (
            const   tCIDLib::TCard4         c4Cookie
            ,       tCIDLib::TCard4&        c4BufSz
            ,       THeapBuf&               mbufData
        )   final;

        tCIDLib::TBoolean bQueryItemMeta
        (
            const   TString&                strHPath
            ,       tCIDLib::TKVPFList&     colMetaVals
            ,       tCIDLib::TStrList&      colChunkIds
            ,       tCIDLib::TCardList&     fcolChunkSizes
            ,       tCIDLib::TCard4&        c4SerialNum
            ,       tCIDLib::TEncodedTime&  enctLastChange
            , const tCIDLib::TBoolean       bThrowIfNot
            , const TCQCSecToken&           sectUser
        )   final;

        tCIDLib::TBoolean bQueryScopeNames
        (
                    tCIDLib::TCard4&        c4ScopeId
            , const TString&                strHPath
            ,       tCIDLib::TStrList&      colItems
            , const tCQCRemBrws::EQSFlags   eQSFlags
            , const TCQCSecToken&           sectUser
        );

        tCIDLib::TVoid DeletePath
        (
            const   TString&                strHPath
            , const tCIDLib::TBoolean       bIsScope
            , const tCIDLib::TBoolean       bMustExist
            , const TCQCSecToken&           sectUser
        )   final;

        tCIDLib::ERenameRes eRenameItem
        (
            const   TString&                strHParPath
            , const TString&                strOldName
            , const TString&                strNewName
            , const tCIDLib::TBoolean       bIsScope
            ,       tCIDLib::TCard4&        c4ParScopeId
            ,       tCIDLib::TCard4&        c4ItemId
            , const TCQCSecToken&           sectUser
        )   final;

        tCIDLib::TVoid GenerateReport
        (
            const   TString&                strDescr
            , const tCQCRemBrws::EDTypeList& fcolTypes
            ,       tCIDLib::TCard4&        c4OutBytes
            ,       THeapBuf&               mbufOut
            , const TCQCSecToken&           sectUser
        )   final;

        tCIDLib::TVoid GenerateTypeReport
        (
            const   TString&                strDescr
            , const TString&                strHpathScope
            , const tCIDLib::TBoolean       bRecurse
            ,       tCIDLib::TCard4&        c4OutBytes
            ,       THeapBuf&               mbufOut
            , const TCQCSecToken&           sectUser
        )   final;

        tCIDLib::TVoid MakeNewScope
        (
            const   TString&                strHParScope
            , const TString&                strNewSubScope
            , const TCQCSecToken&           sectUser
        )   final;

        tCIDLib::TVoid MakePath
        (
            const   TString&                strHPath
            , const TCQCSecToken&           sectUser
        )   final;

        tCIDLib::TVoid PastePath
        (
            const   TString&                strHSrcPath
            , const TString&                strHTarScope
            , const tCIDLib::TBoolean       bSrcIsScope
            , const TCQCSecToken&           sectUser
        )   final;

        tCIDLib::TVoid Ping() final;

        tCIDLib::TVoid QueryItemInfo
        (
            const   TString&                strHPath
            ,       tCIDLib::TBoolean&      bIsScope
            ,       tCIDLib::TCardList&     fcolPathIds
            ,       TDSBrowseItem&          dsbiToFill
            , const TCQCSecToken&           sectUser
        )   final;

        tCIDLib::TVoid QueryItemInfo2
        (
            const   TString&                strHPath
            ,       tCIDLib::TBoolean&      bIsScope
            ,       tCIDLib::TCardList&     fcolPathIds
            ,       TDSBrowseItem&          dsbiTarget
            ,       TDSBrowseItem&          dsbiParScope
            , const TCQCSecToken&           sectUser
        )   final;

        tCIDLib::TVoid QueryPathIds
        (
            const   TString&                strHPath
            ,       tCIDLib::TBoolean&      bIsScope
            ,       tCIDLib::TCard4&        c4ItemId
            ,       tCIDLib::TCard4&        c4ScopeId
            , const TCQCSecToken&           sectUser
        )   final;

        tCIDLib::TVoid QueryPathIds
        (
            const   TString&                strHPath
            ,       tCIDLib::TBoolean&      bIsScope
            ,       tCIDLib::TCardList&     fcolPathIds
            , const TCQCSecToken&           sectUser
        )   final;

        tCIDLib::TVoid QueryScopeItems
        (
            const   TString&                strHPath
            ,       tCIDLib::TCardList&     fcolPathIds
            ,       TVector<TDSBrowseItem>& colItems
            , const tCIDLib::TBoolean       bItemsOnly
            , const TCQCSecToken&           sectUser
        )   final;

        tCIDLib::TVoid QueryTree
        (
            const   TString&                strHStartScope
            ,       TString&                strXML
            , const tCIDLib::TBoolean       bSparse
            , const TCQCSecToken&           sectUser
        )   final;

        tCIDLib::TVoid QueryTree2
        (
            const   TString&                strHStartScope
            ,       tCIDLib::TStrList&      colToFill
            , const tCIDLib::TBoolean       bSparse
            , const TCQCSecToken&           sectUser
        )   final;

        tCIDLib::TVoid SetEventPauseState
        (
            const   TString&                strHPath
            ,       tCIDLib::TCard4&        c4NewSerialNum
            ,       tCIDLib::TCard8&        enctLastChange
            , const tCIDLib::TBoolean       bToSet
            , const TCQCSecToken&           sectUser
        )   final;

        tCIDLib::TVoid SetPeriodicEvTime
        (
            const   TString&                strHPath
            ,       tCIDLib::TCard4&        c4NewSerialNum
            ,       tCIDLib::TCard8&        enctLastChange
            , const tCIDLib::TCard4         c4Period
            , const tCIDLib::TCard8         enctStartAt
            , const TCQCSecToken&           sectUser
        )   final;

        tCIDLib::TVoid SetScheduledEvTime
        (
            const   TString&                strHPath
            ,       tCIDLib::TCard4&        c4NewSerialNum
            ,       tCIDLib::TCard8&        enctLastChange
            , const tCIDLib::TCard4         c4Day
            , const tCIDLib::TCard4         c4Hour
            , const tCIDLib::TCard4         c4Minute
            , const tCIDLib::TCard4         c4Mask
            , const TCQCSecToken&           sectUser
        )   final;

        tCIDLib::TVoid SetSunBasedEvOffset
        (
            const   TString&                strHPath
            ,       tCIDLib::TCard4&        c4NewSerialNum
            ,       tCIDLib::TCard8&        enctLastChange
            , const tCIDLib::TInt4          i4Offset
            , const TCQCSecToken&           sectUser
        )   final;

        tCIDLib::TVoid UploadFirst
        (
            const   TString&                strHPath
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
            , const TCQCSecToken&           sectUser
        )   final;

        tCIDLib::TVoid UploadNext
        (
            const   tCIDLib::TCard4         c4BufSz
            , const THeapBuf&               mbufData
            , const tCIDLib::TCard4         c4Cookie
            , const tCIDLib::TBoolean       bLast
            ,       tCIDLib::TCard4&        c4SerialNum
            ,       tCIDLib::TEncodedTime&  enctLastChange
        )   final;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid Initialize() final;

        tCIDLib::TVoid Terminate() final;


    private :
        // -------------------------------------------------------------------
        //  Private data types
        // -------------------------------------------------------------------
        using TTreeCache = TBasicTreeCol<TDSCacheItem>;
        enum class EFaultInOpts
        {
            Always
            , Never
            , IfLocalExists
        };


        // -------------------------------------------------------------------
        //  Private, static methods
        // -------------------------------------------------------------------
        static tCIDLib::TBoolean bCheckMetaVal
        (
            const   tCIDLib::TKVPFList&     colMeta
            , const TString&                strKey
            , const TString&                strValue
            , const tCIDLib::TBoolean       bThrowIfNotFnd = kCIDLib::True
        );

        static tCIDLib::TVoid BuildIdPath
        (
            const   TTreeCache::TNode* const pnodeTar
            ,       tCIDLib::TCardList&     fcolToFill
            , const tCIDLib::TBoolean       bIsScope
        );

        static tCIDLib::TVoid CheckSecToken
        (
            const   TCQCSecToken&           sectUser
        );

        static tCIDLib::TVoid CheckType
        (
            const   tCQCRemBrws::EDTypes    eExpected
            , const tCQCRemBrws::EDTypes    eFound
            , const tCIDLib::TBoolean       bFromCache
        );

        static tCIDLib::TVoid CheckUserRole
        (
            const   TCQCSecToken&           sectUser
            , const tCQCKit::EUserRoles     eMinRole
        );

        static tCIDLib::TVoid CopyDir
        (
            const   TString&                strSrcPath
            , const TString&                strTarPath
        );

        static tCIDLib::TVoid CopyFile
        (
            const   TString&                strSrcFile
            , const TString&                strTarPath
        );

        static tCQCRemBrws::EDTypes eConvertPath
        (
            const   TString&                strToCvt
            , const tCIDLib::TBoolean       bIsScope
            ,       TString&                strToFill
            , const tCIDLib::TBoolean       bGetFullPath
            , const tCQCRemBrws::EDTypes    eExpected = tCQCRemBrws::EDTypes::Count
            , const tCIDLib::TBoolean       bFromCache = kCIDLib::False
        );

        static tCQCRemBrws::EDTypes eConvertPath
        (
            const   TString&                strToCvt
            , const tCIDLib::TBoolean       bIsScope
            ,       TString&                strRelPath
            ,       TString&                strFullPath
            , const tCQCRemBrws::EDTypes    eExpected = tCQCRemBrws::EDTypes::Count
            , const tCIDLib::TBoolean       bFromCache = kCIDLib::False
        );


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid CheckParms
        (
            const   TCQCSecToken&           sectUser
            , const TString&                strOrgHPath
            , const tCIDLib::TBoolean       bIsScope
            ,       TString&                strHPath
            , const tCQCKit::EUserRoles     eMinRole = tCQCKit::EUserRoles::Count
        )   const;

        tCIDLib::TVoid CheckTreeNodeType
        (
            const   TString&                strHPath
            , const TTreeCache::TNode* const pnodeCheck
            , const tCIDLib::TBoolean       bIsScope
        )   const;

        tCQCRemBrws::EDTypes eCheckParms
        (
            const   TCQCSecToken&           sectUser
            , const TString&                strOrgHPath
            , const tCIDLib::TBoolean       bIsScope
            ,       TString&                strHPath
            ,       TString&                strLocalPath
            , const tCQCKit::EUserRoles     eMinRole = tCQCKit::EUserRoles::Count
        )   const;

        tCQCRemBrws::EDTypes eCheckParms
        (
            const   TCQCSecToken&           sectUser
            , const TString&                strOrgHPath
            , const tCIDLib::TBoolean       bIsScope
            ,       TString&                strHPath
            ,       TString&                strRelPath
            ,       TString&                strLocalPath
            , const tCQCKit::EUserRoles     eMinRole = tCQCKit::EUserRoles::Count
        )   const;

        tCIDLib::TVoid FaultInExtraMeta();

        tCIDLib::TVoid FormatReportHdr
        (
                    TTextOutStream&         strmTar
            , const TString&                strDescr
            , const TCQCSecToken&           sectUser
        );

        tCIDLib::TVoid GenerateReport
        (
                    TTextOutStream&         strmTar
            , const tCQCRemBrws::EDTypes    eType
            ,       TPathStr&               pathHScope
            ,       TPathStr&               pathLocalScope
            , const tCIDLib::TBoolean       bRecurse
            ,       TTime&                  tmFmt
        );

        tCIDLib::TVoid MakeParentPath
        (
            const   TString&                strSrc
            , const tCQCRemBrws::EDTypes    eType
            ,       TString&                strParPart
            ,       TString&                strNamePart
            , const tCIDLib::TBoolean       bIsHPath
        );

        TChunkedFile* pchflLoadFile
        (
            const   TString&                strLocalPath
        );

        TDSCacheItem* pdsciFindHierCache
        (
            const   TString&                strHPath
            , const tCIDLib::TBoolean       bIsScope
        );

        TDSCacheItem* pdsciUpdateHierCache
        (
            const   TString&                strHPath
            , const tCIDLib::TBoolean       bIsScope
            ,       tCIDLib::TBoolean&      bCanCache
        );

        TTreeCache::TNodeNT* pnodeCheckParScope
        (
            const   TString&                strHPath
            , const TString&                strLocalPath
            , const tCQCRemBrws::EDTypes    eType
            , const EFaultInOpts            eFaultInOpt
        );

        TTreeCache::TNodeNT* pnodeFaultInScope
        (
            const   TString&                strHScope
            , const tCIDLib::TBoolean       bLoadContents
            , const tCQCRemBrws::EDTypes    eType
            , const TString&                strLocalPath
        );

        TTransOp* ptopForCookie
        (
            const   tCIDLib::TCard4         c4Cookie
        );

        TTransOp* ptopGetFreeDn
        (
            const   TString&                strHPath
            , const tCIDLib::TCard4         c4DataChunkSz
            , const tCIDLib::TCard4         c4FirstBlockSz
            , const TMemBuf&                mbufDataChunk
            , const tCIDLib::TCard4         c4Flags
        );

        TTransOp* ptopGetFreeUp
        (
            const   TString&                strHPath
            , const tCIDLib::TCard4         c4DataChunkSz
            , const tCIDLib::TCard4         c4FirstBlockSz
            ,       THeapBuf&&              mbufDataChunk
            , const TString&                strExtChunkId
            , const tCIDLib::TCard4         c4ExtSz
            ,       THeapBuf&&              mbufExt
            , const tCIDLib::TKVPFList&     colMeta
            , const tCIDLib::TCard4         c4Flags
        );


        #if CID_DEBUG_ON
        tCIDLib::TVoid DumpCache
        (
            const   tCIDLib::TBoolean       bToFile
        );
        tCIDLib::TVoid DumpLevel
        (
                    TTextOutStream&         strmTar
            , const TTreeCache::TNodeNT* const pnodePar
            , const tCIDLib::TCard4         c4Level
            ,       TTime&                  tmAccess
            ,       TTime&                  tmChange
        );
        #endif

        tCIDLib::TVoid WriteData
        (
            const   TString&                strLocalFile
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
            ,       tCIDLib::TEncodedTime&  enctLastChange
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4NextCookie
        //      This is a running counter that we use to generate new file transfer
        //      cookies.
        //
        //  m_c4NextItemId
        //  m_c4NextScopeId
        //      We have to assign some ids to entries in the hierarchical cache,
        //      and these are used to do that. They are just run up over time, skipping
        //      zero in the unlikely event that they ever wrapped.
        //
        //  m_colDataCache
        //      A keyed hash set for caching data. The key is the full data server
        //      path, upper cased.
        //
        //  m_colTransOps
        //      A list of currently outstanding file transfers. They are only put
        //      into here if they are going to need more than one chunk, which usually
        //      they don't. So these are only the ones that we need to do more
        //      chunks on.
        //
        //  m_colTreeCache
        //      The cache of the hierarchical data. We set it up to do case in-
        //      sensitive comparisons when looking up paths through the tree
        //      structure.
        //
        //  m_mtxSync
        //      To synchronize access to the resources we manage.
        // -------------------------------------------------------------------
        tCIDLib::TCard4     m_c4NextCookie;
        tCIDLib::TCard4     m_c4NextItemId;
        tCIDLib::TCard4     m_c4NextScopeId;
        TRefBag<TTransOp>   m_colTransOps;
        TTreeCache          m_colTreeCache;
        TMutex              m_mtxSync;


        // -------------------------------------------------------------------
        //  Magic Intfs
        // -------------------------------------------------------------------
        RTTIDefs(TDataSrvAccImpl, TDataSrvAccServerBase)
};

#pragma CIDLIB_POPPACK


