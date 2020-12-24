//
// FILE NAME: CQSLRepoS_DriverImpl.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/02/2006
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
//  This is the header file for the actual driver class. Note that the
//  methods of this class, which is fairly large, are spread out over a few
//  different files.
//
//  For the standard media repo data management functionality and serving up
//  data to clients in the standard media repo way, we use a media repo engine
//  provided by CQCMedia, but we do our own derivative of it so that we can
//  override the loading of image files. We don't have just raw images stored,
//  there's other info, so we have to deal with that ourself.
//
//  This same driver works in both V1 and V2 compliant modes. The V1 stuff is
//  just for backwards compatibility. Mostly it's a difference in the fields
//  and field names we expose, plus a bit of extra functionality in the back
//  door calls interface. We just look at the driver architecture version set
//  by the manifest file.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)

class TCQSLRepoMgrImpl;


// ---------------------------------------------------------------------------
//  CLASS: TCQSLMediaRepoEng
// PREFIX: srdb
// ---------------------------------------------------------------------------
class TCQSLMediaRepoEng : public TCQCStdMediaRepoEng
{
    public  :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQSLMediaRepoEng
        (
            const   TString&                strMoniker
        );

        ~TCQSLMediaRepoEng();


        // -------------------------------------------------------------------
        //  Public, inheritedmethods
        // -------------------------------------------------------------------
        tCIDLib::TCard4 c4LoadRawCoverArt
        (
            const   TMediaImg&              mimgToLoad
            ,       TMemBuf&                mbufToFill
            , const tCQCMedia::ERArtTypes   eType
        );

    private :
        RTTIDefs(TCQSLMediaRepoEng,TCQCStdMediaRepoEng)
};



// ---------------------------------------------------------------------------
//   CLASS: TCQSLRepoSDriver
//  PREFIX: drv
// ---------------------------------------------------------------------------
class TCQSLRepoSDriver : public TCQCStdMediaRepoDrv
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQSLRepoSDriver() = delete;

        TCQSLRepoSDriver
        (
            const   TCQCDriverObjCfg&       cqcdcToLoad
        );

        TCQSLRepoSDriver(const TCQSLRepoSDriver&) = delete;

        ~TCQSLRepoSDriver();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQSLRepoSDriver& operator=(const TCQSLRepoSDriver&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bQueryData
        (
            const   TString&                strQueryType
            , const TString&                strDataName
            ,       tCIDLib::TCard4&        c4OutBytes
            ,       THeapBuf&               mbufToFill
        );

        tCIDLib::TBoolean bQueryData2
        (
            const   TString&                strQueryType
            ,       tCIDLib::TCard4&        c4IOBytes
            ,       THeapBuf&               mbufIO
        );

        tCIDLib::TBoolean bQueryTextVal
        (
            const   TString&                strQueryType
            , const TString&                strDataName
            ,       TString&                strToFill
        );

        tCIDLib::TCard4 c4QueryVal
        (
            const   TString&                strValId
        );

        tCIDLib::TBoolean bSendData
        (
            const   TString&                strSendType
            ,       TString&                strDataName
            ,       tCIDLib::TCard4&        c4Bytes
            ,       THeapBuf&               mbufToSend
        );

        tCIDLib::TCard4 c4SendCmd
        (
            const   TString&                strCmd
            , const TString&                strParms
        );


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bGetLease
        (
            const   TString&                strLeaseId
            , const tCIDLib::TBoolean       bOverride
        );

        tCIDLib::TBoolean bQueryArtById
        (
            const   tCQCMedia::EMediaTypes  eMediaType
            , const tCQCMedia::ERArtTypes   eArtType
            , const tCIDLib::TCard2         c2Id
            ,       tCIDLib::TCard4&        c4ArtSz
            ,       THeapBuf&               mbufArt
        );

        tCIDLib::TBoolean bQueryDB
        (
                    tCIDLib::TCard4&        c4CompSz
            ,       THeapBuf&               mbufComp
            , const TString&                strDBSerNum
            ,       TString&                strRepoPath
        );

        tCIDLib::TBoolean bStartUpload
        (
            const   TString&                strLeaseId
            , const tCIDLib::TCard4         c4DBSize
            , const tCIDLib::TCard4         c4ImgCount
            , const tCIDLib::TCard4         c4TrackCount
            ,       TString&                strHostUpload
            ,       tCIDLib::TIPPortNum&    ippnUpload
            ,       tCIDLib::TCard4&        c4Cookie
        );

        tCIDLib::TVoid DropLease
        (
            const   TString&                strLeaseId
        );

        tCIDLib::TVoid QueryChangers
        (
                    tCIDLib::TStrList&      colMonikersToFill
        );


    protected :
        // -------------------------------------------------------------------
        //  Some small classes we use to hold lists of images and ripped files
        //  until the upload is done and we are ready to process them.
        // -------------------------------------------------------------------
        class TImgFile
        {
            public :
                TImgFile
                (
                    const   tCIDLib::TCard4         c4Size
                    , const tCIDLib::TCard2         c2Id
                    , const tCQCMedia::EMediaTypes  eMType
                    , const TMemBuf&                mbufData
                );

                ~TImgFile();

                tCIDLib::TCard2         m_c2Id;
                tCIDLib::TCard4         m_c4Size;
                tCQCMedia::EMediaTypes  m_eMType;
                THeapBuf                m_mbufData;
        };
        typedef TRefVector<TImgFile>    TImgUpList;

        class TRipFile
        {
            public :
                TRipFile
                (
                    const   tCIDLib::TCard2         c2Id
                    , const tCQCMedia::EMediaTypes  eMType
                    , const TString&                strTmpPath
                );

                ~TRipFile();

                tCIDLib::TCard2         m_c2Id;
                tCQCMedia::EMediaTypes  m_eMType;
                TString                 m_strTmpPath;
        };
        typedef TRefVector<TRipFile>    TRipUpList;


        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bGetCommResource
        (
                    TThread&                thrThis
        );

        tCIDLib::TBoolean bWaitConfig
        (
                    TThread&                thrThis
        );

        tCQCKit::ECommResults eBoolFldValChanged
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4FldId
            , const tCIDLib::TBoolean       bNewValue
        );

        tCQCKit::ECommResults eConnectToDevice
        (
                    TThread&                thrThis
        );

        tCQCKit::EDrvInitRes eInitializeImpl();

        tCQCKit::ECommResults ePollDevice
        (
                    TThread&                thrThis
        );

        tCIDLib::TVoid GetUploadPacket
        (
                    TServerStreamSocket&    sockSrc
            ,       tCQSLRepoUpl::THdr&     hdrToFill
            ,       THeapBuf&               mbufToFill
            , const tCIDLib::TCard4         c4ExpectedMsg
        );

        tCIDLib::TVoid ReleaseCommResource();

        tCIDLib::TVoid TerminateImpl();


    private :
        // -------------------------------------------------------------------
        //  Class types
        //
        //  Uploads are handled by a separate thread. When no uploads are
        //  active, we are in idle state. When the thread is uploading, then
        //  we are in Uploading state and we deny any other uploads.
        //
        //  During the commit phase of the upload, we keep up with what we've
        //  done so far using the ESteps enum, so that we can undo those
        //  steps.
        // -------------------------------------------------------------------
        enum class EUplStates
        {
            Idle
            , Uploading
        };


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TCard4 c4AcceptFile
        (
                    TServerStreamSocket&    sockSrc
            ,       THeapBuf&               mbufTmp
            , const tCIDLib::TCard4         c4ExpFileType
            , const tCIDLib::TCard4         c4ExpFileNum
            , const tCIDLib::TCh* const     pszFileDescr
            ,       tCQCMedia::EMediaTypes& eMediaType
            ,       TMemBuf&                mbufData
            ,       tCIDLib::TCard2&        c2Id
        );

        tCIDLib::TVoid AcceptMediaFile
        (
                    TServerStreamSocket&    sockSrc
            ,       THeapBuf&               mbufTmp
            , const tCIDLib::TCard4         c4ExpFileNum
            ,       tCIDLib::TCard2&        c2Id
            ,       tCQCMedia::EMediaTypes& eMType
            ,       TString&                strPath
        );

        tCIDLib::TBoolean bInitRepo();

        tCIDLib::TBoolean bLoadDB
        (
                    TMediaDB&               mdbToFill
            ,       TString&                strDBSerNum
        );

        tCIDLib::TVoid BuildPath
        (
            const   tCQCMedia::EMediaTypes  eType
            , const tCQCMedia::EDataTypes   eDType
            , const tCIDLib::TCard2         c2Id
            ,       TPathStr&               strTarget
            ,       TPathStr&               strMetaFl
            ,       TPathStr&               strMediaFl
            , const tCIDLib::TBoolean       bUpload = kCIDLib::False
        );

        tCIDLib::TVoid CheckLease
        (
            const   tCIDLib::TCh* const     pszFile
            , const tCIDLib::TCard4         c4Line
            , const TString&                strLease
        );

        tCIDLib::TVoid CheckMagicVal
        (
            const   tCIDLib::TCard4         c4Got
            , const tCIDLib::TCard4         c4Expected
            , const tCIDLib::TCh* const     pszFileType
            , const TString&                strFilePath
        );

        tCIDLib::TVoid CleanupUpload();

        tCIDLib::TVoid ErrorOutUpload
        (
                    TServerStreamSocket* const psockSrv
            , const TError&                 errToSend
            ,       THeapBuf&               mbufData
        );

        tCIDLib::TVoid DoFileSwap
        (
            const   TString&                strPath
            , const TString&                strFileName
        );

        tCIDLib::EExitCodes eUploadThread
        (
                    TThread&                thrThis
            ,       tCIDLib::TVoid*         pData
        );

        tCIDLib::TVoid InstallNewDB
        (
                    TMediaDB&               mdbToAccept
            , const TString&                strSerNum
        );

        tCIDLib::TVoid ProcessUploads
        (
                    TMediaDB&               mdbNew
            , const TImgUpList&             colImgs
            , const TRipUpList&             colRips
        );

        tCIDLib::TVoid RenewNSLease();

        tCIDLib::TVoid SendError
        (
                    TServerStreamSocket&    sockUpload
            , const TLogEvent&              logevRet
            ,       THeapBuf&               mbufData
        );

        tCIDLib::TVoid SetInitDBContents
        (
                    TMediaDB&               mdbTar
        );

        tCIDLib::TVoid StoreArt
        (
                    tCIDLib::TCard2&        c2Id
            , const tCQCMedia::EMediaTypes  eType
            , const tCIDLib::TCard4         c4BufSz
            , const THeapBuf&               mbufArt
        );

        tCIDLib::TVoid StoreDBChanges();

        tCIDLib::TVoid StoreDBFile
        (
            const   TMediaDB&               mdbToStore
            ,       TString&                strDBSerNum
        );

        tCIDLib::TVoid StoreImgFile
        (
            const   tCIDLib::TCard2         c2Id
            , const tCQCMedia::EMediaTypes  eMType
            , const TString&                strPerId
            , const tCIDLib::TCard4         c4ImgSz
            , const THeapBuf&               mbufImg
            ,       TPathStr&               pathTar
        );


        //
        //  These are only for pre-4.4.912 compatibilty, to load the data
        //  up from its old format.
        //
        tCIDLib::TVoid ConvertOldData();
        tCIDLib::TVoid ConvertOldImgFiles(const TString&, const tCQCMedia::EMediaTypes);
        tCIDLib::TVoid LoadArtImgs(TMediaDB&);
        tCIDLib::TVoid LoadCategories(TMediaDB&, tCQCMedia::TCatList&);
        tCIDLib::TVoid LoadCollections(TMediaDB&, const tCQCMedia::TCatList&);
        tCIDLib::TVoid LoadItems(TMediaDB&);
        tCIDLib::TVoid LoadTitleSets(TMediaDB&);
        tCIDLib::TVoid CheckDupCats
        (
            TMediaDB&
            , TMediaCollect&
            , const tCQCMedia::TCatList&
            , tCQCMedia::TMediaIdList&
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4FldId_XXX
        //      We look up the ids of our fields after registering them, so that we
        //      can do efficient 'by id' reads/writes.
        //
        //  m_c4UploadCookie
        //      When we get a request to start an upload, we give them back a cookie.
        //      They have to pass this back in on the other socket that is used for
        //      the upload, to prove they are the one who started the upload. If this
        //      is non-zero, then an upload is in progress so we deny another upload
        //      until this one is done. The upload thread will zero it out when it
        //      exits.
        //
        //  m_enctLastClientLease
        //      The last time we got a client lease. After DropLease it is zeroed out.
        //
        //  m_eUplState
        //      The upload thread sets this while it is handling an upload, and we deny
        //      any other upload or reload requests if it indicates we are not in idle
        //      state.
        //
        //  m_ippnUpload
        //      The port that we were told, during install, to use for file upload.
        //
        //  m_pathDBFile
        //      We pre-build the pat to the DB file, since we have to write it back
        //      out any time the database changes, i.e. when the repo manager client
        //      uploads new data or if individual DB changes are sent (such as a
        //      dynamic update of title rating.)
        //
        //  m_pathStartPath
        //      We get the top of the path to search as one of the driver prompts when
        //      we are loaded. This is the root path of the repo directory.
        //
        //  m_pobjMgrIntf
        //      The management ORB interface we install. We keep the name server lease
        //      up to date ourself by periodically renewing it.
        //
        //  m_srdbEngine
        //      We use a standard repo database engine to provide the database and
        //      serving up data to media data clients. We have to create our own small
        //      derivative to handle image loading.
        //
        //  m_strClientLeaseId
        //      We only allow one client manager to connect at once. Each one generates
        //      a unique id when it starts, and calls our bRenewLease() method to try
        //      to get the lease. If no one else has it currently, then we store his
        //      id and set the last lease time stamp.
        //
        //      In case of horrible death, we allow an override as long as the last
        //      lease time was more than a couple minutes ago.
        //
        //  m_strDTXXX
        //      Since we have to refer pretty often to the data types we support, in
        //      error messages and such, we just go ahead and load up strings with
        //      those names in them.
        //
        //  m_strLoadMsg
        //      We set a field to a status message during the loading process and it must
        //      be translatable, so we pre-load it to avoid having to reload it all the
        //      time.
        //
        //  m_strMgrBinding
        //  m_strMgrBindingScope
        //      The name server binding and binding scope for the management ORB interface
        //      we register.
        //
        //  m_strUploadPath
        //      The directory that we do temp uploads to until we can get them stored
        //      away. We prebuild this path on startup so that it's always ready for
        //      use.
        //
        //  m_strSharePassword
        //  m_strShareUserName
        //      The user can provide login credentials for the remote share. Else they will
        //      be empty and the current account's credentials will be used.
        //
        //  m_thrUpload
        //      We start this thread up when we get an upload request. The upload happens
        //      asynchronously on a separate thread and using a raw socket interfaces, not
        //      the ORB, so that we don't eat up all the ORB socket bandwidth uploading
        //      ripped files and such.
        // -------------------------------------------------------------------
        tCIDLib::TCard4         m_c4FldId_DBSerialNum;
        tCIDLib::TCard4         m_c4FldId_Loaded;
        tCIDLib::TCard4         m_c4FldId_LoadStatus;
        tCIDLib::TCard4         m_c4FldId_Status;
        tCIDLib::TCard4         m_c4FldId_ReloadDB;
        tCIDLib::TCard4         m_c4FldId_TitleCnt;
        tCIDLib::TEncodedTime   m_enctLastClientLease;
        EUplStates              m_eUplState;
        tCIDLib::TIPPortNum     m_ippnUpload;
        TMutex                  m_mtxSync;
        TCQSLMediaRepoEng       m_srdbEngine;
        TPathStr                m_pathDBFile;
        TPathStr                m_pathStart;
        TPathStr                m_pathUpload;
        TCQSLRepoMgrImpl*       m_porbsMgrIntf;
        TString                 m_strClientLeaseId;
        TString                 m_strDTCategory;
        TString                 m_strDTCollect;
        TString                 m_strDTImage;
        TString                 m_strDTItem;
        TString                 m_strDTTitleSet;
        TString                 m_strLoadMsg;
        TString                 m_strMgrBinding;
        TString                 m_strMgrBindingScope;
        TString                 m_strSharePassword;
        TString                 m_strShareUserName;
        TThread                 m_thrUploader;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQSLRepoSDriver,TCQCStdMediaRepoDrv)
};

#pragma CIDLIB_POPPACK


