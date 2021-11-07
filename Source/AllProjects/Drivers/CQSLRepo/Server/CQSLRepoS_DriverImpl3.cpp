//
// FILE NAME: CQSLRepoS_DriverImpl3.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 05/06/2006
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
//  This method handles some grunt work stuff to keep it out of the main impl file.
//  This stuff here is related to handling the upload of new content.
//
//  The way it works is:
//
//  1.  We get an info header which can be used to provide some preview info for the
//      upcoming upload. Currently there's not much, but we do get warned if we are
//      going to get ripped media data and the number of files that it will include.
//      And the number of image files we'll be getting.
//
//      Note that this stuff comesin via ORB interface, to the bStartUpload method.
//      If it's happy with the info, it sends back contact info to the client to
//      connect on another socket for the upload process, and then it kicks off a
//      thread to wait for that upload process to start. The rest of these steps
//      happens on that other socket in the download thread.
//
//      We give the caller back a 'cookie' that he has to provide when he connects
//      on the socket, as a safety precaution.
//
//  2.  The compressed database content is uploaded. We stream it in to make sure it
//      is good. This will have all the new info in it. The only things not set are
//      file paths, which we will figure out and set. Note that, because we do have
//      to update paths, the has we will calculate for this new database at the end
//      is not what the client may have calculated on the new buffer.
//
//  3.  Any new images are uploaded. These are stored in a list. They have the media
//      type an id of the images they represent, so we can go to the new database
//      from #2 to get any other info about them we need.
//
//  4.  If any ripped data is being uploaded, those come next.
//
//
//  Since we know how many to expect for #3 and #4, we know when to stop waiting for
//  more of each of them. When we get the last file, we do the update and then report
//  back to the client success or failure.
//
//  5.  We go through the image files in our list and write them out, creating or
//      overwriting as indicated by the ids/media types of the images. File paths
//      are updated for the large art (all we keep around) in the image objects
//      in the new database.
//
//  6.  We move the ripped media content files to their correct locations and update
//      the paths, in the media items in the database in this case, since we only
//      upload ripped music, which is file per item.
//
//
//  7.  If all that goes well, then we store out the new database buffer to disk,
//      and we update our DB serial number to the new database hash, which of course
//      will be different now from what was uploaded, because we set the file paths
//      for the uploaded files into the database objects that represent them. We set
//      this new hash in our base class and in our database serial number field.
//
//      The repo manager client will then shut down the upload socket, and come back,
//      through the regular ORB interface and download the new database. He will be
//      back in sync with us at that point and all changes will have been committed.
//
//      The client service on all the clients running it will some number of seconds
//      later see a new serial number and download the new database. If new image
//      content was uploaded it will grab those, too.
//
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQSLRepoS_.hpp"


// ---------------------------------------------------------------------------
//  Local types and constants
// ---------------------------------------------------------------------------
namespace CQSLRepoS_DriverImpl
{
    // -----------------------------------------------------------------------
    //  A little structure we use to pass in upload instance data to the upload
    //  thread to pass along the info that bStartUpload got.
    // -----------------------------------------------------------------------
    struct TUploadInfo
    {
        tCIDLib::TCard4         c4Cookie;
        tCIDLib::TCard4         c4DBSize;
        tCIDLib::TCard4         c4ImgCount;
        tCIDLib::TCard4         c4TrackCount;
    };
};



// ---------------------------------------------------------------------------
//  The out of line implementation of our small upload list classes
// ---------------------------------------------------------------------------
TCQSLRepoSDriver::TImgFile::TImgFile(const  tCIDLib::TCard4         c4Size
                                    , const tCIDLib::TCard2         c2Id
                                    , const tCQCMedia::EMediaTypes  eMType
                                    , const TMemBuf&                mbufData) :
    m_c2Id(c2Id)
    , m_c4Size(c4Size)
    , m_eMType(eMType)
    , m_mbufData(c4Size, c4Size)
{
    m_mbufData.CopyIn(mbufData, c4Size);
}


TCQSLRepoSDriver::TRipFile::TRipFile(const  tCIDLib::TCard2         c2Id
                                    , const tCQCMedia::EMediaTypes  eMType
                                    , const TString&                strTmpPath) :
    m_c2Id(c2Id)
    , m_eMType(eMType)
    , m_strTmpPath(strTmpPath)
{
}



// ---------------------------------------------------------------------------
//  TCQSLRepoSDriver: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  This method is called when a new CD has been ripped and they want to upload
//  both the metadata and the new ripped media files. We set up and start a
//  thread that will do the upload and accept the data from the client.
//
tCIDLib::TBoolean TCQSLRepoSDriver::
bStartUpload(const  TString&                strLeaseId
            , const tCIDLib::TCard4         c4DBSize
            , const tCIDLib::TCard4         c4ImgCount
            , const tCIDLib::TCard4         c4TrackCount
            ,       TString&                strHostUpload
            ,       tCIDLib::TIPPortNum&    ippnUpload
            ,       tCIDLib::TCard4&        c4Cookie)
{
    TLocker lockrSync(&m_mtxSync);

    //
    //  Check the lease. We'll check it here, and if valid, we'll assume
    //  that it'll be good through the upload process. So the upload
    //  thread will use the stored id.
    //
    CheckLease(CID_FILE, CID_LINE, strLeaseId);

    //
    //  If there is already an upload going on, then just return false so
    //  that the caller can let the user wait or give up.
    //
    if (m_eUplState != EUplStates::Idle)
        return kCIDLib::False;

    // Fill in the end point info that our uploader thread listens on
    strHostUpload = facCIDSock().strIPHostName();
    ippnUpload = m_ippnUpload;

    //
    //  Make sure there's no data in the temp storage directory. We do some
    //  significant checks here to make sure we aren't going to accidentally
    //  clear out the whole drive. If it doesn't exist, then we create the path.
    //  Any sub-directories will be created as needed.
    //
    if (TFileSys::bExists(m_pathUpload))
    {
        if (m_pathUpload.bIsEmpty()
        ||  m_pathStart.bIsEmpty()
        ||  !TFileSys::bIsFQPath(m_pathStart)
        ||  !m_pathUpload.bIsFullyQualified()
        ||  !m_pathUpload.bStartsWith(m_pathStart))
        {
            facCQSLRepoS().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kCQSLRErrs::errcUpl_BadStorePath
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::CantDo
                , m_pathUpload
            );
        }

        //
        //  Looks ok, so clean it out. Don't remove the directory structure, just
        //  remove any files.
        //
        if (TFileSys::bExists(m_pathUpload))
            TFileSys::CleanPath(m_pathUpload, tCIDLib::ETreeDelModes::Clean);
    }
     else
    {
        TFileSys::MakePath(m_pathUpload);
    }

    //
    //  Create a new cookie for this session, and give back to caller. We
    //  just get the current time stamp and take the low 32 bits.
    //
    c4Cookie = tCIDLib::TCard4(TTime::enctNow());

    // Fill in an upload structure
    CQSLRepoS_DriverImpl::TUploadInfo UpInfo;
    UpInfo.c4DBSize = c4DBSize;
    UpInfo.c4Cookie = c4Cookie;
    UpInfo.c4ImgCount = c4ImgCount;
    UpInfo.c4TrackCount = c4TrackCount;

    // Indicate we ar elaoding
    bStoreStringFld
    (
        m_c4FldId_Status
        , facCQSLRepoS().strMsg(kCQSLRMsgs::midStatus_Loading)
        , kCIDLib::True
    );

    // Start up the thread. He'll copy the data before he lets us go
    m_thrUploader.Start(&UpInfo);

    return kCIDLib::True;
}


// ---------------------------------------------------------------------------
//  TCQSLRepoSDriver: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  The upload thread below calls this each time it is expecting a file to be sent
//  be sent. We do the exchange with the repo manager that uploads a file. In this
//  case it is for DB and image stuff, where we just keep it in memory until the
//  upload is over, so it just returns the raw data to the caller.
//
//  The caller indicates the type of file expected and the expected file sequence
//  number (each thing uploaded gets a sequential number.)
//
//  If it's an image file, then we get an id which we give back. Else it'll just be
//  zero if it's the DB file.
//
tCIDLib::TCard4
TCQSLRepoSDriver::c4AcceptFile(         TServerStreamSocket&    sockSrc
                                ,       THeapBuf&               mbufTmp
                                , const tCIDLib::TCard4         c4ExpFileType
                                , const tCIDLib::TCard4         c4ExpFileNum
                                , const tCIDLib::TCh* const     pszFileDescr
                                ,       tCQCMedia::EMediaTypes& eMediaType
                                ,       TMemBuf&                mbufData
                                ,       tCIDLib::TCard2&        c2Id)
{
    tCQSLRepoUpl::THdr  hdrCur;

    // Log that we are waiting for the file upload to start
    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Low)
    {
        facCQSLRepoS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kCQSLRMsgs::midStatus_UplWaitFile
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
            , TString(pszFileDescr)
        );
    }

    // We should get a file upload packet
    GetUploadPacket(sockSrc, hdrCur, mbufTmp, kCQSLRepoUpl::c4Cmd_StartFile);

    //
    //  We survived so it's the expected start file packet. Make sure it's file seq
    //  number matches our index. If not, we've gotten out of sync somehow.
    //
    if (hdrCur.c4Parm1 != c4ExpFileNum)
    {
        facCQSLRepoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCQSLRErrs::errcUpl_BadFileNum
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Protocol
            , TCardinal(c4ExpFileNum)
            , TCardinal(hdrCur.c4Parm1)
        );
    }

    // Make sure it matches the expected file type
    const tCIDLib::TCard4 c4FileType(hdrCur.c4Parm3 & 0xFF);
    const tCIDLib::TCard4 c4MediaType((hdrCur.c4Parm3 >> 8) & 0xFF);
    c2Id = tCIDLib::TCard2(hdrCur.c4Parm3 >> 16);

    if (c4FileType != c4ExpFileType)
    {
        facCQSLRepoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCQSLRErrs::errcUpl_BadFileTypeInfo
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Protocol
            , TCardinal(c4FileType, tCIDLib::ERadices::Hex)
            , TCardinal(c4ExpFileNum, tCIDLib::ERadices::Hex)
        );
    }

    // If the file type isn't media db, then the id cannot be sero
    if ((c4FileType != kCQSLRepoUpl::c4FileType_DB) && !c2Id)
    {
        facCQSLRepoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCQSLRErrs::errcUpl_NoId
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Protocol
            , TString(L"image")
        );
    }

    //
    //  Get the media type out. Won't really matter for the DB file, so it'll be
    //  none.
    //
    switch(c4MediaType)
    {
        case 0:
            eMediaType = tCQCMedia::EMediaTypes::Count;
            break;

        case 1 :
            eMediaType = tCQCMedia::EMediaTypes::Movie;
            break;

        case 2 :
            eMediaType = tCQCMedia::EMediaTypes::Music;
            break;

        default  :
            CIDAssert2(L"Invalid media type in file upload");
            break;
    };


    //
    //  Ok, we start accepting file content packets now until we get all the data.
    //  These will often be a single packet or less, but could be more.
    //
    tCIDLib::TCard4 c4FileSz = hdrCur.c4Parm2;
    tCIDLib::TCard4 c4SoFar = 0;
    tCIDLib::TCard4 c4SeqNum = 0;
    while (c4SoFar < c4FileSz)
    {
        // We should get a file packet
        GetUploadPacket(sockSrc, hdrCur, mbufTmp, kCQSLRepoUpl::c4Cmd_FileData);
        if (hdrCur.c4Parm1 != c4SeqNum)
        {
            facCQSLRepoS().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kCQSLRErrs::errcUpl_BadSeqNum
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Protocol
                , TCardinal(c4SeqNum)
                , TCardinal(hdrCur.c4Parm1)
            );
        }
        c4SeqNum++;

        mbufData.CopyIn(mbufTmp, hdrCur.c4DataBytes, c4SoFar);
        c4SoFar += hdrCur.c4DataBytes;
    }

    // We should get an end file packet
    GetUploadPacket(sockSrc, hdrCur, mbufTmp, kCQSLRepoUpl::c4Cmd_EndFile);

    return c4FileSz;
}


//
//  This is called for each media file we need to upload. We get
//
tCIDLib::TVoid
TCQSLRepoSDriver::AcceptMediaFile(          TServerStreamSocket&    sockSrc
                                    ,       THeapBuf&               mbufTmp
                                    , const tCIDLib::TCard4         c4ExpFileNum
                                    ,       tCIDLib::TCard2&        c2Id
                                    ,       tCQCMedia::EMediaTypes& eMType
                                    ,       TString&                strTarPath)
{
    tCQSLRepoUpl::THdr  hdrCur;

    // Log that we are waiting for the file upload to start
    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Low)
    {
        facCQSLRepoS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kCQSLRMsgs::midStatus_UplWaitFile
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
            , TString(L"media file")
        );
    }

    // We should get a file upload packet
    GetUploadPacket(sockSrc, hdrCur, mbufTmp, kCQSLRepoUpl::c4Cmd_StartFile);

    //
    //  We survived so it's the expected start file packet. Make sure it's file seq
    //  number matches our index. If not, we've gotten out of sync somehow.
    //
    if (hdrCur.c4Parm1 != c4ExpFileNum)
    {
        facCQSLRepoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCQSLRErrs::errcUpl_BadFileNum
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Protocol
            , TCardinal(c4ExpFileNum)
            , TCardinal(hdrCur.c4Parm1)
        );
    }

    // Make sure it matches the expected file type
    const tCIDLib::TCard4 c4FileType(hdrCur.c4Parm3 & 0xFF);
    const tCIDLib::TCard4 c4MediaType((hdrCur.c4Parm3 >> 8) & 0xFF);
    c2Id = tCIDLib::TCard2(hdrCur.c4Parm3 >> 16);

    if (c4FileType != kCQSLRepoUpl::c4FileType_Track)
    {
        facCQSLRepoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCQSLRErrs::errcUpl_BadFileTypeInfo
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Protocol
            , TCardinal(c4FileType, tCIDLib::ERadices::Hex)
            , TCardinal(c4ExpFileNum, tCIDLib::ERadices::Hex)
        );
    }

    // We always have to get an id
    if (!c2Id)
    {
        facCQSLRepoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCQSLRErrs::errcUpl_NoId
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Protocol
            , TString(L"media file")
        );
    }

    //
    //  Convert the passed up encoded media type to our media type enum. For now
    //  it can only be music.
    //
    switch(c4MediaType)
    {
        case 2 :
            eMType = tCQCMedia::EMediaTypes::Music;
            break;

        default  :
            CIDAssert2(L"Invalid media type in media file upload");
            break;
    };


    //
    //  Create a file path for this guy, in the uploads area, and create this file
    //  We'll write to this guy until we get it all.
    //
    TPathStr strDir;
    TPathStr strMetaFile;
    TPathStr strMediaFile;
    BuildPath
    (
        eMType
        , tCQCMedia::EDataTypes::Item
        , c2Id
        , strDir
        , strMetaFile
        , strMediaFile
        , kCIDLib::True
    );

    // Make sure the path exists
    TFileSys::MakePath(strDir);

    // And now we should be able to create the file
    TBinaryFile flOut(strMediaFile);
    flOut.Open
    (
        tCIDLib::EAccessModes::Excl_Write
        , tCIDLib::ECreateActs::CreateAlways
        , tCIDLib::EFilePerms::Default
        , tCIDLib::EFileFlags::SequentialScan
    );

    //
    //  Ok, we start accepting file content packets now until we get all the data.
    //  These will often be a single packet or less, but could be more.
    //
    tCIDLib::TCard4 c4FileSz = hdrCur.c4Parm2;
    tCIDLib::TCard4 c4SoFar = 0;
    tCIDLib::TCard4 c4SeqNum = 0;
    while (c4SoFar < c4FileSz)
    {
        // We should get a file packet
        GetUploadPacket(sockSrc, hdrCur, mbufTmp, kCQSLRepoUpl::c4Cmd_FileData);
        if (hdrCur.c4Parm1 != c4SeqNum)
        {
            facCQSLRepoS().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kCQSLRErrs::errcUpl_BadSeqNum
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Protocol
                , TCardinal(c4SeqNum)
                , TCardinal(hdrCur.c4Parm1)
            );
        }
        c4SeqNum++;

        flOut.c4WriteBuffer(mbufTmp, hdrCur.c4DataBytes);
        c4SoFar += hdrCur.c4DataBytes;
    }

    // We should get an end file packet
    GetUploadPacket(sockSrc, hdrCur, mbufTmp, kCQSLRepoUpl::c4Cmd_EndFile);

    // Give back the path to the media file
    strTarPath = strMediaFile;
}


//
//  This is called on the way out of the upload thread to clean up any
//  files/dirs that might have been left in the upload directory
//
tCIDLib::TVoid TCQSLRepoSDriver::CleanupUpload()
{
    try
    {
        // <TBD>
    }

    catch(const TError&)
    {
    }

    catch(...)
    {
    }
}


//
//  This is called if the upload fails. It cleans up the socket, sending
//  an error reply first if the socket is available.
//
tCIDLib::TVoid TCQSLRepoSDriver::
ErrorOutUpload(         TServerStreamSocket* const  psockUpload
                , const TError&                     errToSend
                ,       THeapBuf&                   mbufData)
{
    if (psockUpload)
    {
        if (psockUpload->bIsConnected())
        {
            try
            {
                SendError(*psockUpload, errToSend, mbufData);
            }

            catch(...)
            {
            }
        }

        try
        {
            psockUpload->c4Shutdown();
            delete psockUpload;
        }

        catch(...)
        {
        }
    }
}


tCIDLib::EExitCodes
TCQSLRepoSDriver::eUploadThread(TThread& thrThis, tCIDLib::TVoid* pData)
{
    //
    //  Set the uploading thread and get the passed instance data out before
    //  we let the caller go.
    //
    CQSLRepoS_DriverImpl::TUploadInfo* pUpInfo
    (
        static_cast<CQSLRepoS_DriverImpl::TUploadInfo*>(pData)
    );

    //
    //  Get the data out. We have to copy the incoming title, collection,
    //  and image data.
    //
    const tCIDLib::TCard4 c4Cookie = pUpInfo->c4Cookie;
    const tCIDLib::TCard4 c4DBSize = pUpInfo->c4DBSize;
    const tCIDLib::TCard4 c4ImgCount = pUpInfo->c4ImgCount;
    const tCIDLib::TCard4 c4TrackCount = pUpInfo->c4TrackCount;

    // Set the state to indicate we are uploading while we are in here
    TGFJanitor<EUplStates> janState(&m_eUplState, EUplStates::Uploading);

    // Now we can let the calling thread go
    thrThis.Sync();

    tCIDLib::TCard2         c2Id;
    tCIDLib::TCard4         c4ItemSeq = 1;
    tCQCMedia::EMediaTypes  eMediaType;
    tCQSLRepoUpl::THdr      hdrCur;
    THeapBuf                mbufTmp(1024 * 32, 1024 * 32);
    THeapBuf                mbufImg(0x20000, 0x400000, 0x40000);
    TServerStreamSocket*    psockUpload = 0;
    try
    {
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Low)
        {
            facCQSLRepoS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kCQSLRMsgs::midStatus_UplWaitConnect
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
                , TCardinal(c4ImgCount)
                , TCardinal(c4TrackCount)
            );
        }

        //
        //  Create a listener thread and wait for a connection. We will
        //  wait for up to 10 seconds, then give up. Indicate we only
        //  accept one connection.
        //
        {
            TSocketListener sockListen(m_ippnUpload, tCIDSock::ESockProtos::TCP, 1);

            const tCIDLib::TEncodedTime enctEnd(TTime::enctNowPlusSecs(10));
            while (!psockUpload)
            {
                // Watch for a shutdown request
                if (thrThis.bCheckShutdownRequest())
                    break;
                psockUpload = sockListen.psockListenFor(kCIDLib::enctTwoSeconds);

                if (TTime::enctNow() >= enctEnd)
                    break;
            }
        }

        // If we didn't get a socket, then we timed out
        if (!psockUpload)
        {
            facCQSLRepoS().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kCQSLRErrs::errcUpl_NoConnect
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Timeout
            );
        }

        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Low)
        {
            facCQSLRepoS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kCQSLRMsgs::midStatus_UplWaitStart
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );
        }

        // Turn off nagle for this one, since it's counter-productive
        psockUpload->bNagleOn(kCIDLib::False);

        // We should get a start upload packet
        GetUploadPacket(*psockUpload, hdrCur, mbufTmp, kCQSLRepoUpl::c4Cmd_StartUpload);

        // Make sure the cookie is what we expect
        if (c4Cookie != hdrCur.c4Parm1)
        {
            facCQSLRepoS().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kCQSLRErrs::errcUpl_BadCookie
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Protocol
            );
        }

        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Low)
        {
            facCQSLRepoS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kCQSLRMsgs::midStatus_UplWaitFiles
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );
        }

        //
        //  The first thing actually uploaded is always the new database. The id
        //  parameter is not used in this case.
        //
        TMediaDB mdbNew;
        {
            THeapBuf mbufDB(c4DBSize, c4DBSize);
            const tCIDLib::TCard4 c4DBSize = c4AcceptFile
            (
                *psockUpload
                , mbufTmp
                , kCQSLRepoUpl::c4FileType_DB
                , c4ItemSeq++
                , L"Media DB"
                , eMediaType
                , mbufDB
                , c2Id
            );

            if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Low)
            {
                facCQSLRepoS().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kCQSLRMsgs::midStatus_GotDBFile
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::AppStatus
                );
            }

            //
            //  Now we need to stream out the new database. This proves it's basically
            //  ok, and we also need to update it with new info before we store it.
            //
            TBinMBufInStream strmDB(&mbufDB, c4DBSize);
            strmDB >> mdbNew;
        }

        //
        //  Next we should get all of the images. We store these in memory until,
        //  we've completed the upload. Use count plus 1 for the max, in case the
        //  count is zero.
        //
        TImgUpList colImgs(tCIDLib::EAdoptOpts::Adopt, c4ImgCount);
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ImgCount; c4Index++)
        {
            const tCIDLib::TCard4 c4ImgSize = c4AcceptFile
            (
                *psockUpload
                , mbufTmp
                , kCQSLRepoUpl::c4FileType_Image
                , c4ItemSeq++
                , L"Art file"
                , eMediaType
                , mbufImg
                , c2Id
            );

            // Store it away for later
            TImgFile* piflNew = new TImgFile(c4ImgSize, c2Id, eMediaType, mbufImg);
            colImgs.Add(piflNew);

            if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
            {
                facCQSLRepoS().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kCQSLRMsgs::midStatus_GotImgFile
                    , tCQCMedia::strXlatEMediaTypes(eMediaType)
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::AppStatus
                    , TCardinal(c2Id)
                );
            }
        }

        if (c4ImgCount && (eVerboseLevel() >= tCQCKit::EVerboseLvls::Low))
        {
            facCQSLRepoS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kCQSLRMsgs::midStatus_GotImgFiles
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
                , TCardinal(c4ImgCount)
            );
        }

        //
        //  And now accept any ripped media content. These will be stored to the
        //  upload directory and we'll process them at the end. Use track count plus
        //  one for the max, in case the count is zero.
        //
        TString strPath;
        TRipUpList colRips(tCIDLib::EAdoptOpts::Adopt, c4TrackCount);
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4TrackCount; c4Index++)
        {
            AcceptMediaFile
            (
                *psockUpload, mbufTmp, c4ItemSeq++, c2Id, eMediaType, strPath
            );

            // Add this to the list of rip files we'll process at the end
            TRipFile* prflNew = new TRipFile(c2Id, eMediaType, strPath);
            colRips.Add(prflNew);

            if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
            {
                facCQSLRepoS().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kCQSLRMsgs::midStatus_GotTrackFile
                    , tCQCMedia::strXlatEMediaTypes(eMediaType)
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::AppStatus
                    , TCardinal(c2Id)
                );
            }
        }

        if (c4TrackCount && (eVerboseLevel() >= tCQCKit::EVerboseLvls::Low))
        {
            facCQSLRepoS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kCQSLRMsgs::midStatus_GotTrackFiles
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
                , TCardinal(c4TrackCount)
            );
        }

        // We should get an end upload packet
        GetUploadPacket(*psockUpload, hdrCur, mbufTmp, kCQSLRepoUpl::c4Cmd_EndUpload);

        //
        //  Process the files we got. This will store the files, update the database
        //  with any required info related to that storage, and then write out the
        //  new database.
        //
        ProcessUploads(mdbNew, colImgs, colRips);

        // Send a final ack packet
        hdrCur.c4MagicVal = kCQSLRepoUpl::c4MagicVal1;
        hdrCur.c4DataCS = 0;
        hdrCur.c4DataBytes = 0;
        hdrCur.c4Cmd = kCQSLRepoUpl::c4Cmd_FinalAck;
        hdrCur.c4Parm1 = 0;
        hdrCur.c4Parm2 = 0;
        hdrCur.c4Parm3 = 0;
        hdrCur.c4MagicVal2 = kCQSLRepoUpl::c4MagicVal2;
        psockUpload->Send(&hdrCur, sizeof(hdrCur));

        // And close the socket now
        psockUpload->c4Shutdown();
        delete psockUpload;
        psockUpload = 0;

        // OK, log that we finished and we are done
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Low)
        {
            facCQSLRepoS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kCQSLRMsgs::midStatus_UplComplete
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );
        }
    }

    catch(TError& errToCatch)
    {
        LogError(errToCatch, tCQCKit::EVerboseLvls::Low, CID_FILE, CID_LINE);
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Low)
        {
            facCQSLRepoS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kCQSLRMsgs::midStatus_UploadFailed
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );
        }

        // Send an error reply and clean up the socket if we opened it
        ErrorOutUpload(psockUpload, errToCatch, mbufTmp);
    }

    catch(...)
    {
        TLogEvent logevRet
        (
            facCQSLRepoS().strName()
            , CID_FILE
            , CID_LINE
            , kCQSLRMsgs::midStatus_ExceptInUpload
            , facCQSLRepoS().strMsg(kCQSLRMsgs::midStatus_ExceptInUpload)
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Unknown
        );

        ErrorOutUpload(psockUpload, logevRet, mbufTmp);

        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Low)
        {
            facCQSLRepoS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kCQSLRMsgs::midStatus_ExceptInUpload
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Unknown
            );
        }
    }

    //
    //  Clean up the temp directory data in case we failed. If it all worked, then
    //  the temp directory no longer exists and this will do nothing.
    //
    CleanupUpload();

    return tCIDLib::EExitCodes::Normal;
}


//
//  Gets a packet from the client. Each packet is acked, so we we handle
//  that here on behalf of the callers.
//
tCIDLib::TVoid
TCQSLRepoSDriver::GetUploadPacket(          TServerStreamSocket& sockSrc
                                    ,       tCQSLRepoUpl::THdr&  hdrToFill
                                    ,       THeapBuf&            mbufToFill
                                    , const tCIDLib::TCard4      c4ExpectedMsg)
{
    //
    //  Read a packet header first. This will tell us how much more data we
    //  have to read to get the data. The default is to throw if all requested
    //  bytes aren't receieved.
    //
    const tCIDLib::TCard4 c4Bytes = sockSrc.c4ReceiveRawTOMS
    (
        &hdrToFill, 6000, sizeof(hdrToFill)
    );
    if (!c4Bytes || (c4Bytes != sizeof(hdrToFill)))
    {
        facCQSLRepoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCQSLRErrs::errcUpl_Timeout
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Timeout
        );
    }

    // Make sure that the packet header is basically sound
    if ((hdrToFill.c4MagicVal != kCQSLRepoUpl::c4MagicVal1)
    ||  (hdrToFill.c4MagicVal2 != kCQSLRepoUpl::c4MagicVal2))
    {
        facCQSLRepoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCQSLRErrs::errcUpl_BadMagicVal
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
        );
    }

    // If there's an expected command, and this isn't it, then bad
    if (c4ExpectedMsg && (hdrToFill.c4Cmd != c4ExpectedMsg))
    {
        facCQSLRepoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCQSLRErrs::errcUpl_UnexpectedPacket
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Protocol
            , TCardinal(c4ExpectedMsg, tCIDLib::ERadices::Hex)
            , TCardinal(hdrToFill.c4Cmd, tCIDLib::ERadices::Hex)
        );
    }

    // Looks ok, so read in the rest of the data, if any
    if (hdrToFill.c4DataBytes)
    {
        sockSrc.c4ReceiveMBufTOMS(mbufToFill, 5000, hdrToFill.c4DataBytes);
        const tCIDLib::TCard4 c4Sum = mbufToFill.c4CheckSum(0, hdrToFill.c4DataBytes);

        //
        //  If the sum isn't right, then throw. Note that the Orb uses a
        //  hash, but we just use a simple checksum and store it in the
        //  same field.
        //
        if (c4Sum != hdrToFill.c4DataCS)
        {
            facCQSLRepoS().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kCQSLRErrs::errcUpl_BadChecksum
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Protocol
            );
        }
    }

    // Send back an ack
    tCQSLRepoUpl::THdr hdrAck;
    hdrAck.c4MagicVal = kCQSLRepoUpl::c4MagicVal1;
    hdrAck.c4DataCS = 0;
    hdrAck.c4DataBytes = 0;
    hdrAck.c4Cmd = kCQSLRepoUpl::c4Cmd_Ack;
    hdrAck.c4Parm1 = 0;
    hdrAck.c4Parm2 = 0;
    hdrAck.c4Parm3 = 0;
    hdrAck.c4MagicVal2 = kCQSLRepoUpl::c4MagicVal2;
    sockSrc.Send(&hdrAck, sizeof(hdrAck));
}


//
//  This is called at the end of the data upload.
//
tCIDLib::TVoid
TCQSLRepoSDriver::ProcessUploads(       TMediaDB&       mdbNew
                                , const TImgUpList&     colImgs
                                , const TRipUpList&     colRips)
{
    //
    //  We have to lock now because we are going to update the data at this point,
    //  behind the back of the driver thread and incoming client ORB threads.
    //
    TLocker lockrSync(&m_mtxSync);

    // Let's get the image files stored away
    TPathStr strDir;
    TPathStr strMediaFl;
    TPathStr strMetaFl;
    const tCIDLib::TCard4 c4ImgCnt = colImgs.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ImgCnt; c4Index++)
    {
        const TImgFile* pifCur = colImgs[c4Index];

        // Look this one up in the database
        const TMediaImg* pmimgCur = mdbNew.pmimgById
        (
            pifCur->m_eMType, pifCur->m_c2Id, kCIDLib::True
        );

        if (pmimgCur)
        {
            StoreImgFile
            (
                pmimgCur->c2Id()
                , pmimgCur->eType()
                , pmimgCur->strPersistentId(tCQCMedia::ERArtTypes::LrgCover)
                , pifCur->m_c4Size
                , pifCur->m_mbufData
                , strMediaFl
            );

            mdbNew.SetArt
            (
                pmimgCur->c2Id()
                , pmimgCur->eType()
                , tCQCMedia::ERArtTypes::LrgCover
                , pifCur->m_mbufData
                , pifCur->m_c4Size
                , strMediaFl
            );
        }
         else
        {
            // <TBD> Log something
        }
    }

    // Let's get the rip files stored away
    const tCIDLib::TCard4 c4RipCnt = colRips.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4RipCnt; c4Index++)
    {
        const TRipFile* prfCur = colRips[c4Index];

        // Look this one up in the database
        const TMediaItem* pmitemCur = mdbNew.pmitemById
        (
            prfCur->m_eMType, prfCur->m_c2Id, kCIDLib::True
        );

        // Lets build it's real path
        BuildPath
        (
            prfCur->m_eMType
            , tCQCMedia::EDataTypes::Item
            , prfCur->m_c2Id
            , strDir
            , strMetaFl
            , strMediaFl
        );

        //
        //  Let's move it to it's final destination. If something is playing the file
        //  right now, this may fail.
        //
        try
        {
            // Make sure the target path exists
            if (!TFileSys::bIsDirectory(strDir))
                TFileSys::MakePath(strDir);

            TFileSys::ReplaceFile(strMediaFl, prfCur->m_strTmpPath);
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        // Update the database with this new path
        mdbNew.SetLocInfo
        (
            prfCur->m_c2Id, prfCur->m_eMType, tCQCMedia::EDataTypes::Item, strMediaFl
        );
    }

    //
    //  Let's store the new database out, to make sure that works. This will get
    //  us back a new serial number.
    //
    TString strDBSerNum;
    StoreDBFile(mdbNew, strDBSerNum);

    // That worked, so let's make this our new database
    InstallNewDB(mdbNew, strDBSerNum);
}



tCIDLib::TVoid
TCQSLRepoSDriver::SendError(        TServerStreamSocket&    sockUpload
                            , const TLogEvent&              logevRet
                            ,       THeapBuf&               mbufData)
{
    tCQSLRepoUpl::THdr hdrErr;
    hdrErr.c4MagicVal = kCQSLRepoUpl::c4MagicVal1;
    hdrErr.c4Cmd = kCQSLRepoUpl::c4Cmd_SrvError;
    hdrErr.c4Parm1 = 0;
    hdrErr.c4Parm2 = 0;
    hdrErr.c4Parm3 = 0;
    hdrErr.c4MagicVal2 = kCQSLRepoUpl::c4MagicVal2;

    {
        TBinMBufOutStream strmOut(&mbufData);
        strmOut << logevRet << kCIDLib::FlushIt;
        hdrErr.c4DataBytes = strmOut.c4CurSize();
    }
    hdrErr.c4DataCS = mbufData.c4CheckSum(0, hdrErr.c4DataBytes);
    sockUpload.Send(&hdrErr, sizeof(hdrErr));
    sockUpload.Send(mbufData.pc1Data(), hdrErr.c4DataBytes);
}


