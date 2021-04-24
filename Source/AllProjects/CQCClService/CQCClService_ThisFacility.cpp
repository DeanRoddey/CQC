//
// FILE NAME: CQCClService_ThisFacility.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/20/2013
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
//  This file implements the more generic parts of the facility class for the
//  CQC app shell.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCClService.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TFacCQCClService,TFacility)



// ---------------------------------------------------------------------------
//   CLASS: TCacheThread
//  PREFIX: thr
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCacheThread: Public, static methods
// ---------------------------------------------------------------------------
const TString& TCacheThread::strKey(const TCacheThread& thrSrc)
{
    return thrSrc.m_strRepoMoniker;
}


// ---------------------------------------------------------------------------
//  TCacheThread: Constructors and Destructor
// ---------------------------------------------------------------------------
TCacheThread::TCacheThread(const TString& strRepo) :

    TThread(TString::strConcat(L"RepoCacher_", strRepo))
    , m_c4CurDataSz(0)
    , m_c4FailCnt(0)
    , m_mbufCurData(8, kCIDLib::c4Sz_32M, kCIDLib::c4Sz_64K)
    , m_strRepoMoniker(strRepo)
{
    // Build the path where we'll put our output
    facCQCMedia().QueryRepoCachePath(m_strRepoMoniker, m_strOutPath);
}

TCacheThread::~TCacheThread()
{
}


// ---------------------------------------------------------------------------
//  TCacheThread: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Provide access to our most recent data. This has to be sync'd since this
//  is called by incoming ORB clients to get data, and we need to insure that
//  this thread doesn't modify it while we are copying it.
//
tCIDLib::TCard4 TCacheThread::c4QueryCurData(TMemBuf& mbufToFill) const
{
    tCIDLib::TCard4 c4Ret = 0;
    {
        TLocker lockrSync(&m_mtxSync);
        if (m_c4CurDataSz)
        {
            mbufToFill.CopyIn(m_mbufCurData, m_c4CurDataSz);
            c4Ret = m_c4CurDataSz;
        }
    }
    return c4Ret;
}


const TString& TCacheThread::strCurSerialNum() const
{
    return m_strCurSerialNum;
}


const TString& TCacheThread::strRepoMoniker() const
{
    return m_strRepoMoniker;
}


// ---------------------------------------------------------------------------
//  TCacheThread: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCacheThread::Init()
{
}

tCIDLib::TVoid TCacheThread::Terminate()
{
}


//
//  We watch our repo for new data to be available. When it is, we download it
//  and store the data. We write out the raw metadata DB and any new or changed
//  images. And we store the compressed metadata for later serving up to clients.
//
tCIDLib::EExitCodes TCacheThread::eProcess()
{
    static const tCIDLib::TCard4 c4MaxFails(20);
    static const tCIDLib::TCard4 c4MaxBufSz(kCIDLib::c4Sz_32M);

    // Let the calling thread go now since we initialized ok
    Sync();

    // Try to create our output path. If we can't do that we are doomed
    while (kCIDLib::True)
    {
        try
        {
            TFileSys::MakePath(m_strOutPath);

            // It worked, so break out and move to the next step
            break;
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        if (!bSleep(15000))
            return tCIDLib::EExitCodes::Normal;
    }

    //
    //  This is our data file that we store our metadata DB to. And it's where
    //  we look first for data when we first come up.
    //
    TPathStr pathMeta = m_strOutPath;
    pathMeta.AddLevel(L"MetaData");
    pathMeta.AppendExt(L"CQCRepoDB");

    //
    //  Since we can repeatedly fail to connect to our repo, us a log limited to prevent
    //  endless logging of the same msg overly quick. LImit it to once every 20 minutes.
    //
    TFacCQCClService& facUs = facCQCClService();
    tCIDLib::TBoolean bFirstTime = kCIDLib::True;
    tCIDLib::TBoolean bDone = kCIDLib::False;
    while (!bDone)
    {
        try
        {
            tCIDLib::TBoolean   bNewData = kCIDLib::False;
            tCIDLib::TCard4     c4RawBytes = 0;
            THeapBuf            mbufRaw(8, c4MaxBufSz, kCIDLib::c4Sz_64K);
            TString             strNewSerNum;
            TZLibCompressor     zlibComp;


            // Get a client proxy for the CQCServer that hosts our repo
            tCQCKit::TCQCSrvProxy orbcSrv
            (
                facCQCKit().orbcCQCSrvAdminProxy(m_strRepoMoniker)
            );

            //
            //  A chunked output stream for any streaming out of data that we
            //  don't know how big it needs to be. This is an efficient way since
            //  it allocates nothing until written to and it allocates data in
            //  chunks so we don't have to pre-commit or pay a cost for
            //  incremental expansion.
            //
            TChunkedBinOutStream strmOutTmp(c4MaxBufSz);

            //
            //  If this is the first time through, see if we have old data we can
            //  read in. If we do, let's be optimistic that it's going to be current
            //  and read it. After the first time through we always go back to the
            //  server.
            //
            //  Note that, if it's out of date, then the next time around we will
            //  just overwrite it. The stored data is not compressed, so we read
            //  it in.
            //
            if (bFirstTime)
            {
                // Make sure this gets cleared before anything can go wrong
                bFirstTime = kCIDLib::False;

                if (TFileSys::bExists(pathMeta, tCIDLib::EDirSearchFlags::NormalFiles))
                {
                    try
                    {
                        TBinaryFile flSrc(pathMeta);
                        flSrc.Open
                        (
                            tCIDLib::EAccessModes::Read
                            , tCIDLib::ECreateActs::OpenIfExists
                            , tCIDLib::EFilePerms::Default
                            , tCIDLib::EFileFlags::SequentialScan
                        );

                        c4RawBytes = tCIDLib::TCard4(flSrc.c8CurSize());
                        flSrc.c4ReadBuffer
                        (
                            mbufRaw
                            , c4RawBytes
                            , tCIDLib::EAllData::FailIfNotAll
                        );

                        // Indicate we have new data
                        bNewData = kCIDLib::True;

                        // And clear the fail counter
                        m_c4FailCnt = 0;
                    }

                    catch(TError& errToCatch)
                    {
                        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                        TModule::LogEventObj(errToCatch);

                        facCQCClService().LogMsg
                        (
                            CID_FILE
                            , CID_LINE
                            , kClSrvErrs::errcData_LoadLocal
                            , tCIDLib::ESeverities::Status
                            , tCIDLib::EErrClasses::CantDo
                            , m_strRepoMoniker
                        );

                        // New data never got set, so we'll do the download
                        c4RawBytes = 0;
                    }
                }
            }

            //
            //  If we didn't load it locally, do a download. This will be compressed
            //  format.
            //
            if (!bNewData)
            {
                // Pass in the current serial number
                strNewSerNum = m_strCurSerialNum;
                bNewData = orbcSrv->bQueryData
                (
                    m_strRepoMoniker
                    , kCQCMedia::strQuery_BinMediaDump
                    , strNewSerNum
                    , c4RawBytes
                    , mbufRaw
                );

                // If we got new data, then decompress it
                if (bNewData)
                {
                    strmOutTmp.Reset();
                    const tCIDLib::TCard4 c4DecompSz = TMediaDB::c4DecompBinDump
                    (
                        mbufRaw, c4RawBytes, strmOutTmp
                    );

                    c4RawBytes = strmOutTmp.c4CopyOutTo(mbufRaw, 0);
                    CIDAssert(c4DecompSz == c4RawBytes, L"Raw bytes != decomp size");
                }
            }

            // If we got new data, then let's process it
            if (bNewData)
            {
                facCQCClService().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kClSrvMsgs::midStatus_NewData
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::AppStatus
                    , m_strRepoMoniker
                );

                //
                //  Stream the data out into a temp database. Tell the helper that
                //  it is uncompressed.
                //
                TMediaDB mdbCurrent;
                tCQCMedia::EMTFlags eMTFlags;
                TMediaDB::ParseBinDump
                (
                    mbufRaw
                    , c4RawBytes
                    , eMTFlags
                    , strNewSerNum
                    , mdbCurrent
                    , kCIDLib::False
                );

                //
                //  Let's do an image download scan. Let him use the raw buffer
                //  as a temp.
                //
                DownloadImgs(orbcSrv, mdbCurrent, mbufRaw);

                //
                //  It worked, so write out the data. We don't write the original
                //  content out, because we will have updated the art paths. So we
                //  write new content out. First we write it to a buffer since we
                //  are going to have to compress it later.
                //
                {
                    strmOutTmp.Reset();
                    c4RawBytes = TMediaDB::c4FormatBinDump
                    (
                        strmOutTmp, eMTFlags, strNewSerNum, mdbCurrent
                    );

                    // Copy it out to the raw buffer now
                    strmOutTmp.c4CopyOutTo(mbufRaw, 0);
                }

                // Write the uncompressed stuff to disk for next time
                {
                    TBinaryFile flTar(pathMeta);
                    flTar.Open
                    (
                        tCIDLib::EAccessModes::Write
                        , tCIDLib::ECreateActs::CreateAlways
                        , tCIDLib::EFilePerms::Default
                        , tCIDLib::EFileFlags::SequentialScan
                    );
                    flTar.c4WriteBuffer(mbufRaw, c4RawBytes);
                }

                //
                //  Sync and update our current data stuff. We have to to compress
                //  the data into our buffer since we cache the compressed stuff in
                //  memory.
                //
                {
                    TLocker lockrSync(&m_mtxSync);

                    // Store the new serial number for this data
                    m_strCurSerialNum = strNewSerNum;

                    // Compress to the temp out stream
                    strmOutTmp.Reset();
                    m_c4CurDataSz = TMediaDB::c4CompBinDump(mbufRaw, c4RawBytes, strmOutTmp);

                    // And copy out to our current data buffer
                    const tCIDLib::TCard4 c4OutBytes = strmOutTmp.c4CopyOutTo
                    (
                        m_mbufCurData, 0
                    );

                    CIDAssert
                    (
                        c4OutBytes == m_c4CurDataSz, L"Compressed bytes != streamed bytes"
                    );
                }

                /// It worked so clear the fail counter
                m_c4FailCnt = 0;

                facCQCClService().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kClSrvMsgs::midStatus_NewDataReady
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::AppStatus
                    , m_strRepoMoniker
                );
            }
        }

        catch(TError& errToCatch)
        {
            m_c4FailCnt++;
            if (facCQCClService().bLogWarnings())
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
                facCQCClService().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kClSrvMsgs::midStatus_Except
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::AppError
                    , m_strRepoMoniker
                );
            }
        }

        catch(...)
        {
            m_c4FailCnt++;
            if (facCQCClService().bLogWarnings())
            {
                facCQCClService().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kClSrvMsgs::midStatus_SysExcept
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::AppError
                    , m_strRepoMoniker
                );
            }
        }

        // If we've failed too many times in a row, give up
        if (m_c4FailCnt > c4MaxFails)
        {
            facCQCClService().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kClSrvErrs::errcData_DroppingRepo
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::Init
                , m_strRepoMoniker
            );
            break;
        }

        //
        //  Sleep for a while, break out if asked to stop. If we have not
        //  gotten any data yet, sleep for 30 seconds. Else sleep for 90
        //  seconds since we are just checking for new data once in a while.
        //
        if (!bSleep(m_strCurSerialNum.bIsEmpty() ? 30000 : 90000))
            break;
    }
    return tCIDLib::EExitCodes::Normal;
}


// ---------------------------------------------------------------------------
//  TCacheThread: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  After getting a new database down, before we write it out, we hvae to
//  check for any images that need to be downloaded.
//
//  We have to go through all of the titles, collections, and items and
//  build up a list of unique persistent image ids, i.e. all of the referenced
//  images. We can then use that to see if we find the images and download
//  them by persistent id if now.
//
tCIDLib::TVoid
TCacheThread::DownloadImgs( tCQCKit::TCQCSrvProxy&  orbcSrv
                            , TMediaDB&             mdbTest
                            , THeapBuf&             mbufTmp)
{
    //
    //  Let's iterate the images and see if we have each one. If not, then
    //  download the ones we are missing. We have to do each media type
    //  separately.
    //
    tCIDLib::TCard4         c4RoundInd = 0;
    tCQCMedia::EMediaTypes  eType = tCQCMedia::EMediaTypes::Min;
    THeapBuf                mbufTmp2(100 * 1024, 4 * (1024 * 1024));
    TPathStr                pathThumb;
    TPathStr                pathLrg;
    TString                 strQData;

    while (eType <= tCQCMedia::EMediaTypes::Max)
    {
        const tCIDLib::TCard4 c4Count = mdbTest.c4ImageCnt(eType);
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            //
            //  Watch for shutdown requests in this inner loop, so we
            //  can react quickly.
            //
            if (bCheckShutdownRequest())
                return;

            // Get the current image we want to process
            const TMediaImg& mimgCur = mdbTest.mimgAt(eType, c4Index);

            //
            //  Check the persistent ids. If both are missing, then we can't get any
            //  data, so we skip it. Clear the paths so we won't try to access it.
            //
            const TString& strPerIdLrg = mimgCur.strPersistentId(tCQCMedia::ERArtTypes::LrgCover);
            const TString& strPerIdSml = mimgCur.strPersistentId(tCQCMedia::ERArtTypes::SmlCover);
            if (strPerIdLrg.bIsEmpty() && strPerIdSml.bIsEmpty())
            {
                mdbTest.ClearArtPaths(mimgCur.c2Id(), eType);
                continue;
            }

            //
            //  Build up the paths to the large and thumb images. We force the thumb
            //  to bitmap for maximum load speed. We force the large to jpeg, which
            //  most of them are anyway (i.e. we just write them out as is.)
            //
            pathThumb = m_strOutPath;
            pathThumb.AddLevel(strPerIdSml);
            pathThumb.Append(L"_Thumb");
            pathThumb.AppendExt(L"bmp");

            pathLrg= m_strOutPath;
            pathLrg.AddLevel(strPerIdLrg);
            pathLrg.Append(L"_Lrg");
            pathLrg.AppendExt(L"jpg");

            //
            //  Update the image object to point to our paths, so that clients will
            //  see our local paths.
            //
            mdbTest.SetArtPath(mimgCur.c2Id(), eType, tCQCMedia::ERArtTypes::LrgCover, pathLrg);
            mdbTest.SetArtPath(mimgCur.c2Id(), eType, tCQCMedia::ERArtTypes::SmlCover, pathThumb);

            // If we have the ones we can get, then skip this one
            if ((strPerIdLrg.bIsEmpty() || TFileSys::bExists(pathLrg, tCIDLib::EDirSearchFlags::NormalFiles))
            &&  (strPerIdSml.bIsEmpty() || TFileSys::bExists(pathThumb, tCIDLib::EDirSearchFlags::NormalFiles)))
            {
                continue;
            }

            // we don't have it, so let's try to download it
            if (eType == tCQCMedia::EMediaTypes::Music)
                strQData = L"Music ";
            else
                strQData = L"Movie ";
            strQData.AppendFormatted(mimgCur.c2Id());

            try
            {
                tCIDLib::TCard4 c4DataSz = 0;
                tCIDLib::TBoolean bRes = orbcSrv->bQueryData
                (
                    m_strRepoMoniker
                    , kCQCMedia::strQuery_QueryImgById
                    , strQData
                    , c4DataSz
                    , mbufTmp
                );

                // If we got it, then break out the two parts
                tCIDLib::TCard4 c4ThumbSz;
                tCIDLib::TCard4 c4LargeSz;

                // It should start with an 'L' for large in the first TCard4
                tCIDLib::TCard4 c4BufInd = 0;
                if (tCIDLib::TCh(mbufTmp.c4At(c4BufInd)) != kCIDLib::chLatin_L)
                {
                    facCQCClService().ThrowErr
                    (
                        CID_FILE
                        , CID_LINE
                        , kClSrvErrs::errcData_DataMarker
                        , pathLrg
                        , tCIDLib::ESeverities::Failed
                        , tCIDLib::EErrClasses::Format
                        , TString(L"Large")
                        , m_strRepoMoniker
                    );
                }
                c4BufInd += 4;

                // Next should be the size
                c4LargeSz = mbufTmp.c4At(c4BufInd);
                c4BufInd += 4;

                //
                //  As a sanity check, there should be the same size value
                //  just past the image data.
                //
                if (mbufTmp.c4At(c4BufInd + c4LargeSz) != c4LargeSz)
                {
                    facCQCClService().ThrowErr
                    (
                        CID_FILE
                        , CID_LINE
                        , kClSrvErrs::errcData_SizeInfo
                        , pathLrg
                        , tCIDLib::ESeverities::Failed
                        , tCIDLib::EErrClasses::Format
                        , TString(L"Large")
                        , m_strRepoMoniker
                    );
                }

                //
                //  Copy out the bytes to the other buffer and move forward.
                //  We have to skip the redundant size at the end as well.
                //
                mbufTmp.CopyOut(mbufTmp2, c4LargeSz, c4BufInd);
                c4BufInd += c4LargeSz + 4;

                //
                //  If it's a JPEG, we just write it out as is. Else, we force it
                //  to JPEG and write that out.
                //
                if (c4LargeSz && !strPerIdLrg.bIsEmpty())
                {
                    if (facCIDImgFact().eProbeImg(mbufTmp2, c4LargeSz) == tCIDImage::EImgTypes::JPEG)
                    {
                        TBinaryFile flOut(pathLrg);
                        flOut.Open
                        (
                            tCIDLib::EAccessModes::Write
                            , tCIDLib::ECreateActs::CreateAlways
                            , tCIDLib::EFilePerms::Default
                            , tCIDLib::EFileFlags::SequentialScan
                        );
                        flOut.c4WriteBuffer(mbufTmp2, c4LargeSz);
                    }
                     else
                    {
                        // Not a JPEG, so force it to be
                        TCIDImage* pimgJPEG = 0;
                        facCIDImgFact().bDecodeImgTo
                        (
                            mbufTmp2
                            , c4LargeSz
                            , tCIDImage::EImgTypes::JPEG
                            , pimgJPEG
                            , kCIDLib::True
                        );
                        TJanitor<TCIDImage> janImg(pimgJPEG);

                        // Now let's write it out to the file
                        TBinFileOutStream strmTar
                        (
                            pathLrg
                            , tCIDLib::ECreateActs::CreateAlways
                            , tCIDLib::EFilePerms::Default
                            , tCIDLib::EFileFlags::SequentialScan
                        );
                        strmTar << *pimgJPEG << kCIDLib::FlushIt;
                    }
                }


                // Next should be a T for thumb
                if (tCIDLib::TCh(mbufTmp.c4At(c4BufInd)) != kCIDLib::chLatin_T)
                {
                    facCQCClService().ThrowErr
                    (
                        CID_FILE
                        , CID_LINE
                        , kClSrvErrs::errcData_DataMarker
                        , pathThumb
                        , tCIDLib::ESeverities::Failed
                        , tCIDLib::EErrClasses::Format
                        , TString(L"Small")
                        , m_strRepoMoniker
                    );
                }
                c4BufInd += 4;

                // Next should be the thumb size
                c4ThumbSz = mbufTmp.c4At(c4BufInd);
                c4BufInd += 4;

                //
                //  As a sanity check, there should be the same size value
                //  just past the image data.
                //
                if (mbufTmp.c4At(c4BufInd + c4ThumbSz) != c4ThumbSz)
                {
                    facCQCClService().ThrowErr
                    (
                        CID_FILE
                        , CID_LINE
                        , kClSrvErrs::errcData_SizeInfo
                        , pathThumb
                        , tCIDLib::ESeverities::Failed
                        , tCIDLib::EErrClasses::Format
                        , TString(L"Thumb")
                        , m_strRepoMoniker
                    );
                }

                // Copy out the bytes to the other buffer
                mbufTmp.CopyOut(mbufTmp2, c4ThumbSz, c4BufInd);
                c4BufInd += c4ThumbSz + 4;

                // Load the data as a bitmap
                if (c4ThumbSz && !strPerIdSml.bIsEmpty())
                {
                    TCIDImage* pimgBmp = 0;
                    facCIDImgFact().bDecodeImgTo
                    (
                        mbufTmp2
                        , c4ThumbSz
                        , tCIDImage::EImgTypes::Bitmap
                        , pimgBmp
                        , kCIDLib::True
                    );
                    TJanitor<TCIDImage> janImg(pimgBmp);

                    // Now let's write it out to the file
                    TBinFileOutStream strmTar
                    (
                        pathThumb
                        , tCIDLib::ECreateActs::CreateAlways
                        , tCIDLib::EFilePerms::Default
                        , tCIDLib::EFileFlags::SequentialScan
                    );

                    strmTar << *pimgBmp << kCIDLib::FlushIt;
                }
            }

            catch(TError& errToCatch)
            {
                if (facCQCClService().bShouldLog(errToCatch))
                {
                    errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                    TModule::LogEventObj(errToCatch);

                    facCQCClService().LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , kClSrvErrs::errcData_Except
                        , pathLrg
                        , tCIDLib::ESeverities::Failed
                        , tCIDLib::EErrClasses::Format
                        , m_strRepoMoniker
                    );
                }
            }

            catch(...)
            {
                facCQCClService().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kClSrvErrs::errcData_Except
                    , pathLrg
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Format
                    , m_strRepoMoniker
                );
            }

            //
            //  Every so many downloads, pause a bit so we don't kill the
            //  repo.
            //
            c4RoundInd++;
            if (!(c4RoundInd % 10))
                TThread::Sleep(100);
        }

        eType++;
    }
}




// ---------------------------------------------------------------------------
//   CLASS: TFacCQCClService
//  PREFIX: fac
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TFacCQCClService: Constructors and Destructor
// ---------------------------------------------------------------------------
TFacCQCClService::TFacCQCClService() :

    TFacility
    (
        L"CQCClService"
        , tCIDLib::EModTypes::Exe
        , kCQCKit::c4MajVersion
        , kCQCKit::c4MinVersion
        , kCQCKit::c4Revision
        , tCIDLib::EModFlags::HasMsgFile
    )
    , m_c4MaxClients(0)
    , m_colRepoThreads
      (
        tCIDLib::EAdoptOpts::Adopt
        , 29
        , TStringKeyOps()
        , TCacheThread::strKey
        , tCIDLib::EMTStates::Safe
      )
    , m_evShutdown(tCIDLib::EEventStates::Reset)
    , m_ippnListen(kCQCKit::ippnClSrvDefPort)
    , m_porbsImpl(nullptr)
    , m_thrMaster
      (
          L"ClSrvMasterThread"
          , TMemberFunc<TFacCQCClService>(this, &TFacCQCClService::eMasterThread)
      )
    , m_thrService
      (
          L"ClServiceThread"
          , TMemberFunc<TFacCQCClService>(this, &TFacCQCClService::eServiceThread)
      )
{
}

TFacCQCClService::~TFacCQCClService()
{
}


// ---------------------------------------------------------------------------
//  TFacCQCClService: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  To help with testing of sleep/awake stuff in debug mode, we use one of the
//  signals to allow us to make the same calls that the service handler normally
//  would.
//
tCIDLib::TBoolean TFacCQCClService::bHandleSignal(const tCIDLib::ESignals eSig)
{
    #if CID_DEBUG_ON
    if (eSig == tCIDLib::ESignals::Break)
    {
        try
        {
            if (m_thrMaster.bIsRunning())
                StopProcessing();
            else
                StartProcessing();
        }

        catch(...)
        {
        }
        return kCIDLib::True;
    }
    #endif

    // Trigger our shutdown event to cause the main thread to exit
    m_evShutdown.Trigger();
    return kCIDLib::True;
}



// ---------------------------------------------------------------------------
//  TFacCQCClService: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Our ORB interface calls through to here to check for/get new data if
//  available.
//
tCIDLib::TBoolean
TFacCQCClService::bQueryRepoDB( const   TString&            strRepoMon
                                , const TString&            strSerialNum
                                ,       tCIDLib::TCard4&    c4BufSz
                                ,       THeapBuf&           mbufData)
{
    // Insure we don't return bogus info
    c4BufSz = 0;

    // Lock and see if we have this repo
    TLocker lockrSync(&m_colRepoThreads);

    //
    //  If not found, or the same serial number, then return false and
    //  no data.
    //
    TCacheThread* pthrFind = m_colRepoThreads.pobjFindByKey
    (
        strRepoMon, kCIDLib::False
    );

    if (!pthrFind || (pthrFind->strCurSerialNum() == strSerialNum))
        return kCIDLib::False;

    // We got it, so query the info from it
    c4BufSz = pthrFind->c4QueryCurData(mbufData);
    return kCIDLib::True;
}


//
//  Lets our service handler know if our service thread is running or not.
//  Since he doesn't have direct access to the thread, we have to provide him
//  with a way to check.
//
tCIDLib::TBoolean TFacCQCClService::bSrvRunning() const
{
    return m_thrService.bIsRunning();
}


//
//  This is called by our ORB interface, to let clients get a list of repo
//  monikers that we are currently tracking.
//
tCIDLib::TCard4
TFacCQCClService::c4QueryRepoList(TVector<TString>& colToFill)
{
    // Lock and see if we have this repo
    TLocker lockrSync(&m_colRepoThreads);

    colToFill.RemoveAll();
    TRepoCache::TNCCursor cursInact(&m_colRepoThreads);
    for (; cursInact; ++cursInact)
        colToFill.objAdd(cursInact->strRepoMoniker());
    return colToFill.c4ElemCount();
}


//
//  The main thread of the service. Note that, in debug mode, this is just
//  started directly via the standard main kickoff macro (in the main Cpp file.)
//  But when running a service, then the service handler starts up a thread
//  on us here and manages that thread.
//
//  So, in the real service scenario, we may be stopped and started multiple
//  times if the service paused or unpaused and such.
//
tCIDLib::EExitCodes
TFacCQCClService::eServiceThread(TThread& thrThis, tCIDLib::TVoid*)
{
    // Let our caller go
    thrThis.Sync();

    //
    //  Wait for a bit to give other services time to come up. We are pretty high
    //  on the food chain. If asked to stop while waiting, do nothing.
    //
    if (!thrThis.bSleep(5000))
        return tCIDLib::EExitCodes::Normal;

    //
    //  Ask CQCKit to load core environment/parm stuff. If we can't do this,
    //  then we are doomed and just have to exit.
    //
    TString strFailReason;
    if (!facCQCKit().bLoadEnvInfo(strFailReason, kCIDLib::False))
        return tCIDLib::EExitCodes::BadEnvironment;

    try
    {
        // Parse our command line parms
        ParseParms();

        // If in debug mode, register as a signal handler
        #if CID_DEBUG_ON
        TSignals::c4AddHandler(this);
        #endif

        // Start our processing
        StartProcessing();
    }

    catch(TError& errToCatch)
    {
        if (!errToCatch.bLogged() && bLogWarnings())
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            LogEventObj(errToCatch);
        }
    }

    catch(...)
    {
    }

    //
    //  Wwe just block forever until asked to stop, either via the signal handler if
    //  running in regular process mode, or from the service thread if in service
    //  mode.
    //
    m_evShutdown.WaitFor();

    //
    //  Shut down our update thread if it's running. Typically it'll already
    //  be stopped via a service command.
    //
    StopProcessing();

    return tCIDLib::EExitCodes::Normal;
}


//
//  If our master thread isn't running, start it up. It will start other threads
//  as required.
//
tCIDLib::TVoid TFacCQCClService::StartProcessing()
{
    if (!m_thrMaster.bIsRunning())
        m_thrMaster.Start();
}


//
//  If our master thread is running, then ask it to stop. It will stop any of
//  the processing threads it is currently using.
//
tCIDLib::TVoid TFacCQCClService::StopProcessing()
{
    try
    {
        if (m_thrMaster.bIsRunning())
        {
            m_thrMaster.ReqShutdownSync(10000);
            m_thrMaster.eWaitForDeath(5000);
        }
    }

    catch(TError& errToCatch)
    {
        if (bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            LogEventObj(errToCatch);
        }
    }
}


//
//  Call by our service handler when the service is started. We just start
//  up the service thread.
//
tCIDLib::TVoid TFacCQCClService::StartService()
{
    if (!m_thrService.bIsRunning())
        m_thrService.Start();
}


//
//  When running in the real service mode, this is called by the service handler
//  when a request comes in that requires the service to stop or pause. We
//  trigger the event that the main thread (above) is blocked on and it starts
//  shutting down. The service handler waits for it to finish closing down and
//  reports progress to the service manager.
//
tCIDLib::TVoid TFacCQCClService::StartShutdown()
{
    // Just trigger the event that the main thread is waiting on
    m_evShutdown.Trigger();
}


// Wait for the service thread to completely stop
tCIDLib::TVoid TFacCQCClService::WaitComplete()
{
    try
    {
        m_thrService.eWaitForDeath(5000);
    }

    catch(...)
    {
    }
}


// ---------------------------------------------------------------------------
//  TFacCQCClService: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Handles the stuff we need to do to start our service processing. We have
//  to get the ORB initialized, get our object created, and register it with
//  the auto-rebinder to keep it registered in the name service. We need to
//  install a log server logger as well.
//
//  Note that we have to deal wtih a stupid problem here.
//
tCIDLib::TBoolean TFacCQCClService::bDoInit()
{
    // Make sure the thread list is cleaned out
    m_colRepoThreads.RemoveAll();

    try
    {
        // Initialize the client and server sides of the ORB
        facCIDOrb().InitClient();

        // Install a log server logger
        m_plgrInstalled = new TLogSrvLogger(facCQCKit().strLocalLogDir());
        TModule::InstallLogger(m_plgrInstalled, tCIDLib::EAdoptOpts::Adopt);

        // Log a starting up message
        facCQCClService().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kClSrvMsgs::midStatus_Starting
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::Init
        );

        // Now start up the server side ORB
        facCIDOrb().InitServer(m_ippnListen, m_c4MaxClients);

        // Create an instance of our interface and register it with the ORB
        m_porbsImpl = new TCQCClSrvImpl();
        facCIDOrb().RegisterObject(m_porbsImpl, tCIDLib::EAdoptOpts::Adopt);

        // Register this object with the rebinder
        m_strORBBinding = kCQCKit::pszCQCSlSrvIntf;
        m_strORBBinding.eReplaceToken(TSysInfo::strIPHostName(), kCIDLib::chLatin_h);
        TString strDescr(L"CQC Client Service on host ");
        strDescr.Append(TSysInfo::strIPHostName());
        facCIDOrbUC().RegRebindObj(m_porbsImpl->ooidThis(), m_strORBBinding, strDescr);

        // Start up the rebinder
        facCIDOrbUC().StartRebinder();
    }

    catch(TError& errToCatch)
    {
        if (!errToCatch.bLogged() && bLogWarnings())
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        facCQCClService().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"An error occurred while initializing client service"
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Term
        );
        return kCIDLib::False;
    }

    catch(...)
    {
        return kCIDLib::False;
    }

    // Log a ready up message
    facCQCClService().LogMsg
    (
        CID_FILE
        , CID_LINE
        , kClSrvMsgs::midStatus_Ready
        , tCIDLib::ESeverities::Status
        , tCIDLib::EErrClasses::Init
    );

    return kCIDLib::True;
}


//
//  We wake up periodically and get a list of available repo drivers. If we
//  see that we don't have any of them in our active list, we add new items.
//  We also check to see if any have become inactive and remove them if so.
//
tCIDLib::EExitCodes
TFacCQCClService::eMasterThread(TThread& thrThis, tCIDLib::TVoid* pData)
{
    thrThis.Sync();

    // Do initialization stuff
    if (!bDoInit())
        return tCIDLib::EExitCodes::InitFailed;

    tCIDLib::TStrList colRepos;
    while(kCIDLib::True)
    {
        tCIDLib::TBoolean bRes = kCIDLib::False;
        try
        {
            // See if we need to add any new guys to our list
            bRes = facCQCKit().bFindRepoDrivers(colRepos, TString::strEmpty());

            if (bRes)
            {
                // Lock while we update
                TLocker lockrSync(&m_colRepoThreads);

                const tCIDLib::TCard4 c4Count = colRepos.c4ElemCount();
                for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
                {
                    const TString& strCur = colRepos[c4Index];
                    if (!m_colRepoThreads.bKeyExists(strCur))
                    {
                        TCacheThread* pciNew = new TCacheThread(strCur);
                        m_colRepoThreads.Add(pciNew);

                        // Start it up
                        pciNew->Start();

                        // Log that it's been dropped
                        LogMsg
                        (
                            CID_FILE
                            , CID_LINE
                            , kClSrvMsgs::midStatus_RepoAdded
                            , tCIDLib::ESeverities::Status
                            , tCIDLib::EErrClasses::AppStatus
                            , strCur
                        );
                    }
                }
            }

            //
            //  Check our existing list for any inactive ones. We have to
            //  lock while doign this so we don't interfer with client
            //  threads accessing the list.
            //
            {
                TLocker lockrSync(&m_colRepoThreads);

                TRepoCache::TNCCursor cursInact(&m_colRepoThreads);
                if (cursInact.bReset())
                {
                    do
                    {
                        const TCacheThread& thrCur = cursInact.objRCur();
                        if (!thrCur.bIsRunning())
                        {
                            // Get the repo name out before we remove it
                            TString strName = thrCur.strRepoMoniker();
                            m_colRepoThreads.RemoveAt(cursInact);


                            // Don't access the thrCur value at this point


                            // Log that it's been dropped
                            LogMsg
                            (
                                CID_FILE
                                , CID_LINE
                                , kClSrvMsgs::midStatus_RepoRemoved
                                , tCIDLib::ESeverities::Status
                                , tCIDLib::EErrClasses::AppStatus
                                , strName
                            );
                        }
                    }   while (cursInact.bNext());
                }
            }
        }

        catch(TError& errToCatch)
        {
            if (!errToCatch.bLogged() && bLogWarnings())
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }
        }

        catch(...)
        {
        }

        //
        //  If successful, sleep a bit. If we failed in our query above, then
        //  CQC is probably not running, If we didn'tfail but the list is empty,
        //  then the system may be coming up and want to pick up new ones quickly.
        //  In either case, re-try in 5 seconds.
        //
        //  Else, we are probably just in steady state, so sleep for 25 seconds
        //  waiting for changes.
        //
        //  Break out if asked to stop while sleeping
        //
        tCIDLib::TCard4 c4Sleep;
        if (!bRes || colRepos.bIsEmpty())
            c4Sleep = 5000;
        else
            c4Sleep = 25000;

        if (!thrThis.bSleep(c4Sleep))
            break;
    }

    // Log a stopping message
    facCQCClService().LogMsg
    (
        CID_FILE
        , CID_LINE
        , kClSrvMsgs::midStatus_Stopping
        , tCIDLib::ESeverities::Status
        , tCIDLib::EErrClasses::Init
    );

    // Stop our repo threads if any
    try
    {
        const tCIDLib::TCard4 c4Count = m_colRepoThreads.c4ElemCount();

        TRepoCache::TNCCursor cursStop(&m_colRepoThreads);
        if (cursStop.bReset())
        {
            // First ask them all to start shuttinng down
            do
            {
                cursStop.objWCur().ReqShutdownNoSync();
            }   while (cursStop.bNext());

            // And then wait for them to stop
            tCIDLib::EExitCodes eExit;
            cursStop.bReset();
            do
            {
                cursStop.objWCur().bWaitForDeath(eExit, 2000);
            }   while (cursStop.bNext());
        }
    }

    catch(TError& errToCatch)
    {
        if (facCQCClService().bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            LogEventObj(errToCatch);
        }

        if (facCQCClService().bLogFailures())
        {
            facCQCClService().LogMsg
            (
                CID_FILE
                , CID_LINE
                , L"An error occurred while asking repo threads to stop"
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Term
            );
        }
    }

    catch(...)
    {
        if (facCQCClService().bLogFailures())
        {
            facCQCClService().LogMsg
            (
                CID_FILE
                , CID_LINE
                , L"An unknown error occurred while stopping the repo threads"
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Term
            );
        }
    }


    // Remove our bindings and stop the ORB
    ORBCleanup();

    return tCIDLib::EExitCodes::Normal;
}


//
//  Called at shutdown to remove our name server bindings and stop the
//  ORB.
//
tCIDLib::TVoid TFacCQCClService::ORBCleanup()
{
    //
    //  Shut down the rebinder before we start removing bindings. Otherwise,
    //  he'll just start rebinding them again.
    //
    facCIDOrbUC().StopRebinder();

    try
    {
        //
        //  Get a client name server proxy. Don't wait long. We have to be
        //  able to shut down quickly.
        //
        tCIDOrbUC::TNSrvProxy orbcNS = facCIDOrbUC().orbcNameSrvProxy(1500);
        orbcNS->RemoveBinding(m_strORBBinding, kCIDLib::False);
    }

    catch(...)
    {
        //
        //  Don't bother logging anything. This wouldn't be unusual if
        //  the server isn't running when this client is shut down.
        //
    }


    // If we installed a logger, stop it
    if (m_plgrInstalled)
    {
        try
        {
            //
            //  Tell the module class to stop using our logger. We will clean it up in this
            //  case since we told him not to.
            //
            TModule::OrphanLogger();
            m_plgrInstalled->Cleanup();
            delete m_plgrInstalled;
            m_plgrInstalled = nullptr;
        }

        catch(TError& errToCatch)
        {
            if (bShouldLog(errToCatch))
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                LogEventObj(errToCatch);
            }

            if (bLogFailures())
            {
                LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , L"An error occurred while stopping the logger"
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Term
                );
            }
        }

        // And make sure the pointer gets zero
        m_plgrInstalled = nullptr;
    }

    // And stop the ORB now
    try
    {
        if (m_plgrInstalled)
            m_plgrInstalled->bForceLocal(kCIDLib::True);
        facCIDOrb().Terminate();
    }

    catch(TError& errToCatch)
    {
        if (bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            LogEventObj(errToCatch);
        }

        if (bLogFailures())
        {
            LogMsg
            (
                CID_FILE
                , CID_LINE
                , L"An error occurred while shutting down ORB"
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Term
            );
        }
    }
}



//
//  Called from the main thread on startup to parse command line parms. Mostly
//  it's stuff we need to initialize the ORB.
//
tCIDLib::TVoid TFacCQCClService::ParseParms()
{
    if (bLogInfo())
    {
        LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"Parsing parameters..."
            , tCIDLib::ESeverities::Info
            , tCIDLib::EErrClasses::AppStatus
        );
    }

    TSysInfo::TCmdLineCursor cursParms = TSysInfo::cursCmdLineParms();
    TString strCurParm;
    for (; cursParms; ++cursParms)
    {
        strCurParm = *cursParms;

        // We know what is by what it starts with
        if (strCurParm.bStartsWithI(L"/Port="))
        {
            strCurParm.Cut(0, 6);
            m_ippnListen = strCurParm.c4Val();
        }
         else if (strCurParm.bStartsWithI(L"/Max="))
        {
            strCurParm.Cut(0, 5);
            m_c4MaxClients = strCurParm.c4Val();
        }
         else
        {
            if (bLogWarnings())
            {
                LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , L"'%(1)' is not a known client serivce parameter"
                    , tCIDLib::ESeverities::Warn
                    , tCIDLib::EErrClasses::BadParms
                    , strCurParm
                );
            }
        }
    }

    if (bLogInfo())
    {
        LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"Client serivce parameters parsed ok"
            , tCIDLib::ESeverities::Info
            , tCIDLib::EErrClasses::AppStatus
        );
    }
}


