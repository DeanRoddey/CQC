//
// FILE NAME: MediaRepoMgr_UploadDlg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 05/07/2006
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
//  This dialog is used to upload data music/picture data to the server. The
//  data is all in a directory, the path to which is provided to us, and we
//  just upload all the files.
//
//  To avoid overburdening the ORB, which isn't really optimized for bulk
//  data upload, we use a raw socket for this.
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
#include    "MediaRepoMgr.hpp"
#include    "MediaRepoMgr_UploadDlg.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TUploadDataDlg,TDlgBox)


// ---------------------------------------------------------------------------
//  Local types and constants
// ---------------------------------------------------------------------------
namespace MediaRepoMgr_UploadDlg
{
    #if CID_DEBUG_ON
    const tCIDLib::TCard4 c4WaitPacket = 50000;
    #else
    const tCIDLib::TCard4 c4WaitPacket = 8000;
    #endif


    // -----------------------------------------------------------------------
    //  The size of our per-chunk buffer. This is the max we send at a time.
    // -----------------------------------------------------------------------
    const tCIDLib::TCard4   c4BufSz = 32 * 1024;
}



// ---------------------------------------------------------------------------
//   CLASS: TUploadDataDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TUploadDataDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TUploadDataDlg::TUploadDataDlg() :

    m_bCancel(kCIDLib::False)
    , m_c4Cookie(0)
    , m_c4CurFileNum(0)
    , m_c4CurPercent(0)
    , m_c4DBSize(0)
    , m_c4LastPercent(0)
    , m_c4LastFileNum(0)
    , m_colImgs(tCIDLib::EAdoptOpts::NoAdopt)
    , m_eRepStatus(EStatus::Initializing)
    , m_eStatus(EStatus::Initializing)
    , m_mbufChunk(MediaRepoMgr_UploadDlg::c4BufSz, MediaRepoMgr_UploadDlg::c4BufSz)
    , m_pwndCancel(nullptr)
    , m_pwndProgress(nullptr)
    , m_pwndStatus(nullptr)
    , m_thrUpload
      (
        TString(L"MediaRepoMgrUploadThread")
        , TMemberFunc<TUploadDataDlg>(this, &TUploadDataDlg::eUploadThread)
      )
    , m_tmidUpdate(kCIDCtrls::tmidInvalid)
{
}

TUploadDataDlg::~TUploadDataDlg()
{
}


// ---------------------------------------------------------------------------
//  TUploadDataDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TUploadDataDlg::bRun(const  TWindow&    wndOwner
                    ,       TString&    strError
                    , const TMediaDB&   mdbChanges
                    , const TString&    strLeaseId
                    , const TString&    strRepoMoniker)
{
    const tCIDLib::EDirSearchFlags eFiles = tCIDLib::EDirSearchFlags::NormalFiles;

    // Store away the incoming stuff for later use
    m_pstrError = &strError;

    try
    {
        //
        //  Go through the rip directory and load up a list of all of the media
        //  files we are going to need to upload.
        //
        TPathStr pathSearch = facMediaRepoMgr.strRipDir();
        TDirIter diterItems;
        TFindBuf fndbCur;
        if (diterItems.bFindFirst(pathSearch, L"*.wma", fndbCur, eFiles))
        {
            do
            {
                m_colRipFiles.objAdd(fndbCur.pathFileName());
            }   while (diterItems.bFindNext(fndbCur));
        }

        //
        //  Go through the database and find all of the images that we have cached
        //  data on, and set up a list of these. These are the ones we need to
        //  upload.
        //
        tCQCMedia::EMediaTypes eMType = tCQCMedia::EMediaTypes::Min;
        while (eMType <= tCQCMedia::EMediaTypes::Max)
        {
            const tCIDLib::TCard4 c4ImgCnt = mdbChanges.c4ImageCnt(eMType);
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ImgCnt; c4Index++)
            {
                const TMediaImg& mimgCur = mdbChanges.mimgAt(eMType, c4Index);
                if (mimgCur.c4Size(tCQCMedia::ERArtTypes::LrgCover))
                    m_colImgs.Add(&mimgCur);
            }
            eMType++;
        }

        // Flatten the database data out and save the size info
        {
            TBinMBufOutStream strmTar(&m_mbufDBData);
            strmTar << mdbChanges << kCIDLib::FlushIt;
            m_c4DBSize = strmTar.c4CurSize();
        }

        // Set the total file count
        m_c4FileCnt = 1;
        m_c4FileCnt += m_colImgs.c4ElemCount();
        m_c4FileCnt += m_colRipFiles.c4ElemCount();

        //
        //  The rip worked, so lets upload all of the data. We have to upload the
        //  title set, the collection, the image, and the media files and media
        //  item data.
        //
        TWndPtrJanitor janPtr(tCIDCtrls::ESysPtrs::Wait);
        tMediaRepoMgr::TRepoMgrProxyPtr orbcSrv
        (
            facMediaRepoMgr.orbcMakeProxy(strRepoMoniker)
        );

        //
        //  Ok, let's upload the data to the repository driver. We send him
        //  an address type so that he'll create one we can handle. If we
        //  support both, it will be 'unspecified' so he can create which
        //  ever he prefers.
        //
        const tCIDLib::TBoolean bStatus = orbcSrv->bStartUpload
        (
            strLeaseId
            , m_c4DBSize
            , m_colImgs.c4ElemCount()
            , m_colRipFiles.c4ElemCount()
            , m_strHost
            , m_ippnHost
            , m_c4Cookie
        );

        // If he denies us, then tell the user and return
        if (!bStatus)
        {
            TErrBox msgbFailed(facMediaRepoMgr.strMsg(kMRMgrMsgs::midStatus_StartReject));
            msgbFailed.ShowIt(*this);
            return kCIDLib::False;
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        // Use our owner here since we are not created yet!
        TExceptDlg dlgErr
        (
            wndOwner
            , facMediaRepoMgr.strMsg(kMRMgrMsgs::midMain_FrameTitle)
            , facMediaRepoMgr.strMsg(kMRMgrMsgs::midStatus_Exception)
            , errToCatch
        );
        return kCIDLib::False;
    }

    const tCIDLib::TCard4 c4Res = c4RunDlg
    (
        wndOwner, facMediaRepoMgr, kMediaRepoMgr::ridDlg_Upload
    );

    // Make sure the thread is all the way down
    WaitThreadEnd();

    // It automatically ends with a status of 1 if the upload worked
    return (c4Res == 1);
}


// ---------------------------------------------------------------------------
//  TUploadDataDlg: Protected, inherited methods
// ---------------------------------------------------------------------------

// Set the cancel flag and prevent standard cancel
tCIDLib::TBoolean TUploadDataDlg::bCancelRequest()
{
    m_bCancel = kCIDLib::True;
    return kCIDLib::False;
}


tCIDLib::TBoolean TUploadDataDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRet = TParent::bCreated();

    // Get typed pointers to some of the widgets
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_Upload_Cancel, m_pwndCancel);
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_Upload_Progress, m_pwndProgress);
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_Upload_Status, m_pwndStatus);

    // Register handlers
    m_pwndCancel->pnothRegisterHandler(this, &TUploadDataDlg::eClickHandler);

    // Start up the timer
    m_tmidUpdate = tmidStartTimer(100);

    // Start the thread up
    m_bCancel = kCIDLib::False;
    m_thrUpload.Start();

    return bRet;
}


//
//  We watch for changes in the status and either update the status winodw
//  to reflect the status, or when we see a fail/done status, we dismiss
//  ourself.
//
tCIDLib::TVoid TUploadDataDlg::Timer(const tCIDCtrls::TTimerId tmidToDo)
{
    if (tmidToDo != m_tmidUpdate)
        return;

    // Based on the status, do what we need to do
    if (m_eRepStatus != m_eStatus)
    {
        m_eRepStatus = m_eStatus;

        switch(m_eStatus)
        {
            case EStatus::Initializing :
                // Nothing to do here for this one
                break;

            case EStatus::SendingDB :
                // Set the status message to indicate uploading database
                m_pwndStatus->strWndText(facMediaRepoMgr.strMsg(kMRMgrMsgs::midDlg_Upload_DB));
                break;

            case EStatus::SendingImgs :
                // Set the status msgs we'll ues below in this phase
                m_strCurFileMsg.LoadFromMsg(kMRMgrMsgs::midDlg_Upload_CurImage, facMediaRepoMgr);
                break;

            case EStatus::SendingItems :
                // Set the status msgs we'll ues below in this phase
                m_strCurFileMsg.LoadFromMsg(kMRMgrMsgs::midDlg_Upload_CurItem, facMediaRepoMgr);
                break;

            case EStatus::Cancelled :
            case EStatus::Failed :
                //
                //  Error text will have been put into the caller's error parm
                //  if it's a failure.
                //
                if (m_tmidUpdate != kCIDCtrls::tmidInvalid)
                {
                    StopTimer(m_tmidUpdate);
                    m_tmidUpdate = kCIDCtrls::tmidInvalid;
                }
                EndDlg(0);
                break;

            case EStatus::Done :
                // It worked
                if (m_tmidUpdate != kCIDCtrls::tmidInvalid)
                {
                    StopTimer(m_tmidUpdate);
                    m_tmidUpdate = kCIDCtrls::tmidInvalid;
                }
                EndDlg(1);
                break;
        };
    }

    // If sending files/buffers, show percentage and X of Y indicators
    if ((m_eRepStatus == EStatus::SendingDB)
    ||  (m_eRepStatus == EStatus::SendingImgs)
    ||  (m_eRepStatus == EStatus::SendingItems))
    {
        if (m_c4CurPercent != m_c4LastPercent)
        {
            m_pwndProgress->SetValue(m_c4CurPercent);
            m_c4LastPercent = m_c4CurPercent;
        }

        if (m_c4CurFileNum != m_c4LastFileNum)
        {
            m_strTmpFmt = m_strCurFileMsg;
            m_strTmpFmt.eReplaceToken(m_c4CurFileNum, L'1');
            m_strTmpFmt.eReplaceToken(m_c4FileCnt, L'2');
            m_pwndStatus->strWndText(m_strTmpFmt);

            m_c4LastFileNum = m_c4CurFileNum;
        }
    }
}


// ---------------------------------------------------------------------------
//  TUploadDataDlg: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDCtrls::EEvResponses
TUploadDataDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kMediaRepoMgr::ridDlg_Upload_Cancel)
    {
        //
        //  Just set the cancel flag. The timer will cancel us once the
        //  thread has seen this and stopped.
        //
        m_bCancel = kCIDLib::True;
    }
    return tCIDCtrls::EEvResponses::Handled;
}


tCIDLib::EExitCodes
TUploadDataDlg::eUploadThread(TThread& thrThis, tCIDLib::TVoid*)
{
    // Let the calling thread go
    thrThis.Sync();

    try
    {
        // Create the end point he want's us to connect to
        TIPEndPoint ipepSrv(m_strHost, m_ippnHost, tCIDSock::EAddrTypes::Unspec);

        //
        //  Open the upload socket. Note that the server gives us back the
        //  end point that he is going to listen on once he spins up his
        //  listenign thread. So we could get there before he does. So we
        //  retry a few times before giving up.
        //
        TClientStreamSocket sockSrv
        (
            tCIDSock::ESockProtos::TCP, ipepSrv.eAddrType()
        );

        try
        {
            sockSrv.Connect(ipepSrv);
        }

        catch(...)
        {
            TThread::Sleep(2000);
            try
            {
                sockSrv.Connect(ipepSrv);
            }

            catch(...)
            {
                // Let this one propogate, we are giving up
                TThread::Sleep(2000);
                sockSrv.Connect(ipepSrv);
            }
        }

        // Turn off nagle for this one, since it's counter-productive
        sockSrv.bNagleOn(kCIDLib::False);

        // Now we can send the start upload packet, passing in the cookie
        SendPacket
        (
            sockSrv, kCQSLRepoUpl::c4Cmd_StartUpload, m_mbufChunk, 0, m_c4Cookie
        );

        // Move to uploading metadata state, and send the database
        m_eStatus = EStatus::SendingDB;
        UploadBuffer
        (
            sockSrv
            , m_mbufDBData
            , m_c4DBSize
            , kCQSLRepoUpl::c4FileType_DB
            , tCQCMedia::EMediaTypes::Count
            , 0
        );

        //
        //  The DB usually goes by so fast, that we pause a bit afterwards so the
        //  user will not just see a blur go by. And it prevents eating up the network
        //  as well, so we do it below in the other loops also.
        //
        TThread::Sleep(250);

        // If we got canceled, then return now
        if (m_bCancel)
        {
            m_eStatus = EStatus::Cancelled;
            return tCIDLib::EExitCodes::Normal;
        }

        // Do the images
        const tCIDLib::TCard4 c4ImgCnt = m_colImgs.c4ElemCount();
        if (c4ImgCnt)
        {
            m_eStatus = EStatus::SendingImgs;
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ImgCnt; c4Index++)
            {
                const TMediaImg* pmimgCur = m_colImgs[c4Index];
                const tCIDLib::TCard4 c4Size = pmimgCur->c4Size(tCQCMedia::ERArtTypes::LrgCover);

                UploadBuffer
                (
                    sockSrv
                    , pmimgCur->mbufArt(tCQCMedia::ERArtTypes::LrgCover)
                    , c4Size
                    , kCQSLRepoUpl::c4FileType_Image
                    , pmimgCur->eType()
                    , pmimgCur->c2Id()
                );
            }
            TThread::Sleep(25);
        }

        // Do the tracks if any
        const tCIDLib::TCard4 c4RipCnt = m_colRipFiles.c4ElemCount();
        if (c4RipCnt)
        {
            m_eStatus = EStatus::SendingItems;
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4RipCnt; c4Index++)
                UploadMediaFile(sockSrv, m_colRipFiles[c4Index]);
        }

        // Send the end upload packet
        SendPacket(sockSrv, kCQSLRepoUpl::c4Cmd_EndUpload, m_mbufChunk, 0);

        //
        //  And now wait for a final ack, or a final error that might arise
        //  while trying to install the uploaded data.
        //
        tCQSLRepoUpl::THdr hdrAck;
        WaitPacket(sockSrv, hdrAck, m_mbufChunk, kCQSLRepoUpl::c4Cmd_FinalAck);

        // Clean up the rip directory
        try
        {
            TFileSys::CleanPath(facMediaRepoMgr.strRipDir());
        }

        catch(TError& errToCatch)
        {
            facMediaRepoMgr.LogMsg
            (
                CID_FILE
                , CID_LINE
                , kMRMgrErrs::errcFile_CleanRipDir
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );

            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            throw;
        }

        // It all worked
        m_eStatus = EStatus::Done;
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        m_pstrError->LoadFromMsg
        (
            kMRMgrMsgs::midStatus_Exception
            , facMediaRepoMgr
            , errToCatch.strErrText()
        );
        m_eStatus = EStatus::Failed;
        return tCIDLib::EExitCodes::Normal;
    }

    catch(...)
    {
        m_pstrError->LoadFromMsg
        (
            kMRMgrMsgs::midStatus_UnknownException, facMediaRepoMgr
        );
        m_eStatus = EStatus::Failed;
        return tCIDLib::EExitCodes::Normal;
    }

    return tCIDLib::EExitCodes::Normal;
}


//
//  Sends a packet using the passed info, and waits for the ack or error
//  reply that should come back. Note that it can use the passed data buffer
//  to read in an error reply, but it doesn't matter that we overwrite the
//  caller's data since we are doomed at that point anyway.
//
tCIDLib::TVoid
TUploadDataDlg::SendPacket(         TClientStreamSocket&    sockSrv
                            , const tCIDLib::TCard4         c4Cmd
                            , const TMemBuf&                mbufData
                            , const tCIDLib::TCard4         c4DBytes
                            , const tCIDLib::TCard4         c4Parm1
                            , const tCIDLib::TCard4         c4Parm2
                            , const tCIDLib::TCard4         c4Parm3)
{
    static const tCIDLib::TEncodedTime enctWaitSend(kCIDLib::enctOneSecond * 5);

    tCQSLRepoUpl::THdr hdrToUse;
    hdrToUse.c4MagicVal = kCQSLRepoUpl::c4MagicVal1;
    hdrToUse.c4DataBytes = c4DBytes;
    hdrToUse.c4Cmd = c4Cmd;
    hdrToUse.c4Parm1 = c4Parm1;
    hdrToUse.c4Parm2 = c4Parm2;
    hdrToUse.c4Parm3 = c4Parm3;
    hdrToUse.c4MagicVal2 = kCQSLRepoUpl::c4MagicVal2;

    // If any data bytes then calc the checksum
    if (c4DBytes)
        hdrToUse.c4DataCS = mbufData.c4CheckSum(0, c4DBytes);
    else
        hdrToUse.c4DataCS = 0;

    // Send the header
    sockSrv.SendThrottled(&hdrToUse, enctWaitSend, sizeof(hdrToUse));

    //
    //  If there are any data bytes, then send that. We do a throttled send
    //  just in case. We don't want to overrun him.
    //
    if (c4DBytes)
        sockSrv.SendThrottled(mbufData, enctWaitSend, c4DBytes);

    // And we should get an ack packet back
    WaitPacket(sockSrv, hdrToUse, m_mbufChunk, kCQSLRepoUpl::c4Cmd_Ack);
}


//
//  For the stuff that is already in a memory buffer, this is called to upload them
//  to the server.
//
tCIDLib::TVoid
TUploadDataDlg::UploadBuffer(       TClientStreamSocket&    sockTar
                            , const TMemBuf&                mbufData
                            , const tCIDLib::TCard4         c4ToSend
                            , const tCIDLib::TCard4         c4FileType
                            , const tCQCMedia::EMediaTypes  eMType
                            , const tCIDLib::TCard2         c2Id)
{
    // Translate the media type to the upload value
    tCIDLib::TCard4 c4MediaType;
    switch(eMType)
    {
        case tCQCMedia::EMediaTypes::Movie :
            c4MediaType = kCQSLRepoUpl::c4MediaType_Movie;
            break;

        case tCQCMedia::EMediaTypes::Music :
            c4MediaType = kCQSLRepoUpl::c4MediaType_Music;
            break;

        default :
            c4MediaType = kCQSLRepoUpl::c4MediaType_None;
            break;
    };

    //
    //  Send the start file message. We have to send the media type, file type,
    //  and id in the last parm.
    //
    SendPacket
    (
        sockTar
        , kCQSLRepoUpl::c4Cmd_StartFile
        , m_mbufChunk
        , 0
        , ++m_c4CurFileNum
        , c4ToSend
        , (tCIDLib::TCard4(c2Id) << 16) | c4MediaType | c4FileType
    );

    // Calc the percent per packet sent, so we know how much to bump it each time
    tCIDLib::TFloat8 f8PerPacket
    (
        tCIDLib::TFloat8(MediaRepoMgr_UploadDlg::c4BufSz) / tCIDLib::TFloat8(c4ToSend)
    );
    if (f8PerPacket > 1.0)
        f8PerPacket = 1.0;

    tCIDLib::TCard4 c4SoFar = 0;
    tCIDLib::TCard4 c4Seq = 0;
    tCIDLib::TCard4 c4ThisTime = 0;
    tCIDLib::TFloat8 f8CurPer = 0;
    while (c4SoFar < c4ToSend)
    {
        // Watch for a cancel signal
        if (m_bCancel)
            return;

        // Read up to a full buffer's worth of data
        c4ThisTime = tCIDLib::MinVal
        (
            c4ToSend - c4SoFar, MediaRepoMgr_UploadDlg::c4BufSz
        );

        // Copy over the data for this round to the chunk buffer
        mbufData.CopyOut(m_mbufChunk, c4ThisTime, c4SoFar);

        // Send a file buffer packet
        SendPacket
        (
            sockTar
            , kCQSLRepoUpl::c4Cmd_FileData
            , m_mbufChunk
            , c4ThisTime
            , c4Seq
        );

        c4SoFar += c4ThisTime;
        c4Seq++;

        // Bump the current file percent
        f8CurPer += f8PerPacket;
        if (f8CurPer > 1.0)
            f8CurPer = 1.0;
        m_c4CurPercent = tCIDLib::TCard4(f8CurPer * 100.0);
    }

    // Send an end file packet
    SendPacket(sockTar, kCQSLRepoUpl::c4Cmd_EndFile, m_mbufChunk, 0, 0);
}


//
//  This is called for each upload file to upload it to the server. We get the
//  info we need from the file names, which hav that info embedded in them.
//
tCIDLib::TVoid
TUploadDataDlg::UploadMediaFile(        TClientStreamSocket&    sockTar
                                , const TPathStr&               pathFilePath)
{
    // Parse the info out
    TString strMType;
    TString strDType;
    TString strId;

    TString strFileName;
    pathFilePath.bQueryName(strFileName);
    TStringTokenizer stokFile(&strFileName, L"_");

    if (!stokFile.bGetNextToken(strMType)
    ||  !stokFile.bGetNextToken(strDType)
    ||  !stokFile.bGetNextToken(strId))
    {
        return;
    }

    const tCIDLib::TCard4 c4Id = strId.c4Val();
    const tCIDLib::TCard4 c4MType = kCQSLRepoUpl::c4MediaType_Music;

    if (!strMType.bCompareI(L"Music") || !strDType.bCompareI(L"item"))
    {
        // This isn't correct, for now we only support music rip
        return;
    }

    // Open the file
    TBinaryFile flCur(pathFilePath);
    flCur.Open
    (
        tCIDLib::EAccessModes::Excl_Read
        , tCIDLib::ECreateActs::OpenIfExists
        , tCIDLib::EFilePerms::Default
        , tCIDLib::EFileFlags::SequentialScan
    );

    const tCIDLib::TCard4 c4ToSend = tCIDLib::TCard4(flCur.c8CurSize());

    //
    //  Calculate how many packets this will take, and from that,
    //  how many percentage points we need to move forward for each
    //  packet sent.
    //
    tCIDLib::TFloat8 f8PerPacket
    (
        tCIDLib::TFloat8(MediaRepoMgr_UploadDlg::c4BufSz)
        / tCIDLib::TFloat8(c4ToSend)
    );
    if (f8PerPacket > 100.0)
        f8PerPacket = 100.0;

    // Reset our percent indicators
    m_c4CurPercent = 0;
    tCIDLib::TFloat8 f8CurPer = 0;

    //
    //  Send the start file to let him know a new file is coming. We combine
    //  three values into the third parm.
    //
    SendPacket
    (
        sockTar
        , kCQSLRepoUpl::c4Cmd_StartFile
        , m_mbufChunk
        , 0
        , ++m_c4CurFileNum
        , c4ToSend
        , (c4Id << 16) | c4MType | kCQSLRepoUpl::c4FileType_Track
    );

    tCIDLib::TCard4 c4SoFar = 0;
    tCIDLib::TCard4 c4Seq = 0;
    tCIDLib::TCard4 c4ThisTime = 0;
    while (c4SoFar < c4ToSend)
    {
        // Watch for a cancel signal
        if (m_bCancel)
            return;

        // Read up to a full buffer's worth of data
        c4ThisTime = tCIDLib::MinVal
        (
            c4ToSend - c4SoFar, MediaRepoMgr_UploadDlg::c4BufSz
        );
        c4ThisTime = flCur.c4ReadBuffer(m_mbufChunk, c4ThisTime);

        // Send a file buffer packet
        SendPacket
        (
            sockTar
            , kCQSLRepoUpl::c4Cmd_FileData
            , m_mbufChunk
            , c4ThisTime
            , c4Seq
        );

        c4SoFar += c4ThisTime;
        c4Seq++;

        // Bump the current file percent
        f8CurPer += f8PerPacket;
        if (f8CurPer > 1.0)
            f8CurPer = 1.0;
        m_c4CurPercent = tCIDLib::TCard4(f8CurPer * 100.0);

        //
        //  Throttle us back a little so that we don't eat up the
        //  network so bad.
        //
        if (!(c4Seq % 32))
            TThread::Sleep(1);
    }

    // Send an end file packet
    SendPacket(sockTar, kCQSLRepoUpl::c4Cmd_EndFile, m_mbufChunk, 0);
}


//
//  Waits for a packet from the other side and, if there is an expected
//  command, it verifies that it's that command. If it's an error reply,
//  it throws the returned error.
//
tCIDLib::TVoid
TUploadDataDlg::WaitPacket(         TClientStreamSocket&    sockSrv
                            ,       tCQSLRepoUpl::THdr&     hdrToUse
                            ,       TMemBuf&                mbufData
                            , const tCIDLib::TCard4         c4Expected)
{
    const tCIDLib::TCard4 c4Bytes = sockSrv.c4ReceiveRawTOMS
    (
        &hdrToUse, 8000, sizeof(hdrToUse)
    );
    if (!c4Bytes || (c4Bytes != sizeof(hdrToUse)))
    {
        facMediaRepoMgr.ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMRMgrErrs::errcComm_Timeout
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Timeout
        );
    }

    // Make sure that the packet header is basically sound
    if ((hdrToUse.c4MagicVal != kCQSLRepoUpl::c4MagicVal1)
    ||  (hdrToUse.c4MagicVal2 != kCQSLRepoUpl::c4MagicVal2))
    {
        facMediaRepoMgr.ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMRMgrErrs::errcComm_BadPacket
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
        );
    }

    //
    //  If there's an expected command, and this isn't it, then bad. If
    //  it's an error return, then resurrect the error object and throw it
    //
    if (c4Expected && (hdrToUse.c4Cmd != c4Expected))
    {
        if (hdrToUse.c4Cmd == kCQSLRepoUpl::c4Cmd_SrvError)
        {
            // Read the packet data in
            sockSrv.c4ReceiveMBufTOMS(mbufData, 8000, hdrToUse.c4DataBytes);

            TLogEvent logevThrow;
            TBinMBufInStream strmEv(&mbufData, hdrToUse.c4DataBytes);
            strmEv >> logevThrow;
            throw logevThrow;
        }
         else
        {
            facMediaRepoMgr.ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kMRMgrErrs::errcComm_UploadExpected
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Protocol
                , TCardinal(c4Expected, tCIDLib::ERadices::Hex)
                , TCardinal(hdrToUse.c4Cmd, tCIDLib::ERadices::Hex)
            );
        }
    }
}


tCIDLib::TVoid TUploadDataDlg::WaitThreadEnd()
{
    //
    //  Let it throw should this somehow fail, since it's better to let
    //  the program go down than have a thread running off in the ozone.
    //
    m_thrUpload.eWaitForDeath(12000);
}


