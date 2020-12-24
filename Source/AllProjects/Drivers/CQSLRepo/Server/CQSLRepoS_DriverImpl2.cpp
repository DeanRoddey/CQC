//
// FILE NAME: CQSLRepoS_DriverImpl2.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/15/2006
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
//  This method handles some grunt work stuff to keep it out of the main
//  impl file. This stuff here is related to read/writing the repository
//  files.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQSLRepoS_.hpp"
#include    "CQSLRepoS_CatList.hpp"



// ---------------------------------------------------------------------------
//  Local types and constants
// ---------------------------------------------------------------------------
namespace CQSLRepoS_DriverImpl
{
    const tCIDLib::TCard2   c2DBFmtVersion = 1;
}



// ---------------------------------------------------------------------------
//  TCQSLRepoSDriver: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  This is called during driver init, to do any prep work that needs to be done
//  before the database can be loaded or created. We don't actually load the database
//  here, just get to the point where it can be.
//
//  If we got credentials for the remote share, then this includes getting that
//  successfully set up.
//
//  We handle any upgrading of metadata format our layout here so that the main driver
//  code can always expect the data in the current format.
//
tCIDLib::TBoolean TCQSLRepoSDriver::bInitRepo()
{
    try
    {
        if (!m_strShareUserName.bIsEmpty())
        {
            CIDAssert(!m_strSharePassword.bIsEmpty(), L"Have user name, but password is empty")
            const tCIDLib::EConnRes eRes = TFileSys::eAccessRemoteShare
            (
                m_pathStart, m_strShareUserName, m_strSharePassword, kCIDLib::True
            );

            // If an outright failure, then we failed to init
            if (eRes == tCIDLib::EConnRes::Failed)
            {
                if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
                {
                    facCIDLib().LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , kCIDErrs::errcFile_RemPathAcc
                        , m_pathStart
                        , tCIDLib::ESeverities::Failed
                        , tCIDLib::EErrClasses::Authority
                    );
                }
                return kCIDLib::False;
            }

            // If already connected some other way, try to continue but log
            if (eRes == tCIDLib::EConnRes::AlreadyConnected)
            {
                if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Low)
                {
                    facCQSLRepoS().LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , kCQSLRErrs::errcGen_AlreadyConn
                        , m_pathStart
                        , tCIDLib::ESeverities::Failed
                        , tCIDLib::EErrClasses::Already
                    );
                }
            }
        }


        //
        //  Create some paths.
        //
        //  We do the same thing for each path, so just set up a little temp array
        //  of the dirs and we can loop through them. The start path must be first
        //  since the others are subdirs of it.
        //
        m_pathUpload = m_pathStart;
        m_pathUpload.AddLevel(L"Upload");

        TPathStr pathImgs(m_pathStart);
        pathImgs.AddLevel(L"ImDat");

        TPathStr pathImgsMusic(pathImgs);
        pathImgsMusic.AddLevel(tCQCMedia::strXlatEMediaTypes(tCQCMedia::EMediaTypes::Music));

        TPathStr pathImgsMovies(pathImgs);
        pathImgsMovies.AddLevel(tCQCMedia::strXlatEMediaTypes(tCQCMedia::EMediaTypes::Movie));

        TPathStr pathItems(m_pathStart);
        pathItems.AddLevel(L"ItDat");

        TPathStr pathItemsMusic(pathItems);
        pathItemsMusic.AddLevel(tCQCMedia::strXlatEMediaTypes(tCQCMedia::EMediaTypes::Music));

        TString* apstrCreate[] =
        {
            &m_pathStart
            , &pathImgs
            , &pathItems
            , &pathImgsMusic
            , &pathImgsMovies
            , &pathItems
            , &pathItemsMusic
            , &m_pathUpload
        };
        const tCIDLib::TCard4 c4Count = tCIDLib::c4ArrayElems(apstrCreate);
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            //
            //  See if the directory exists. If not, then we need to create it.
            //  If we can't, then obviously we cannot continue.
            //
            if (!TFileSys::bExists(*apstrCreate[c4Index], tCIDLib::EDirSearchFlags::AllDirs))
            {
                if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
                {
                    facCQCMedia().LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , kMedMsgs::midStatus_CreatingRepo
                        , tCIDLib::ESeverities::Status
                        , tCIDLib::EErrClasses::AppStatus
                        , *apstrCreate[c4Index]
                    );
                }
                TFileSys::MakePath(*apstrCreate[c4Index]);
            }
        }

        // Create the path to the DB file
        m_pathDBFile = m_pathStart;
        m_pathDBFile.AddLevel(kCQSLRepoS::pszDBFileName);

        //
        //  If it doesn't exist, but the old data directories do, then we
        //  need to convert from the old to new formats.
        //
        if (!TFileSys::bExists(m_pathDBFile))
        {
            //
            //  We may need to try to convert the old format data over.
            //  If we find ClDat, CtDat, and ImDat directories, then
            //  the old data is still there.
            //
            TPathStr pathCats(m_pathStart);
            pathCats.AddLevel(L"CtDat");
            TPathStr pathCols(m_pathStart);
            pathCols.AddLevel(L"ClDat");
            TPathStr pathSets(m_pathStart);
            pathSets.AddLevel(L"TsDat");

            if (TFileSys::bExists(pathCats)
            &&  TFileSys::bExists(pathCols)
            &&  TFileSys::bExists(pathSets))
            {
                // This code is sequestered over in the #4 source file
                ConvertOldData();
            }
        }
    }

    catch(TError& errToCatch)
    {
        bStoreStringFld(m_c4FldId_LoadStatus, L"Error", kCIDLib::True);
        m_srdbEngine.mdbInfo().Reset();

        LogError(errToCatch, tCQCKit::EVerboseLvls::Medium, CID_FILE, CID_LINE);
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
        {
            facCQCMedia().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kMedErrs::errcDB_CreateFailed
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::AppStatus
                , m_pathStart
            );
        }
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


//
//  This is called to load the database from disk.
//
tCIDLib::TBoolean
TCQSLRepoSDriver::bLoadDB(TMediaDB& mdbToFill, TString& strDBSerNum)
{
    // If it doesn't exist, don't bother trying
    if (!TFileSys::bExists(m_pathDBFile))
        return kCIDLib::False;

    try
    {
        TBinFileInStream strmSrc
        (
            m_pathDBFile
            , tCIDLib::ECreateActs::OpenIfExists
            , tCIDLib::EFilePerms::Default
            , tCIDLib::EFileFlags::SequentialScan
            , tCIDLib::EAccessModes::Read
        );

        //
        //  First write out a stream marker and then a version format so that we can
        //  update it easier later. We also, for efficiency and sanity checking,
        //  store the serial number.
        //
        tCIDLib::TCard2 c2FmtVersion;
        tCIDLib::TCard4 c4DBSize;

        // Should start with a marker
        strmSrc.CheckForStartMarker(CID_FILE, CID_LINE);

        // Make sure the format version is reasonable
        strmSrc >> c2FmtVersion;
        if (!c2FmtVersion || (c2FmtVersion > CQSLRepoS_DriverImpl::c2DBFmtVersion))
        {
            facCIDLib().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kCIDErrs::errcGen_UnknownFmtVersion
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
                , TCardinal(c2FmtVersion)
                , clsThis()
            );
        }

        // Looks reasonable so read in the DB serial number and buffer size
        strmSrc >> c4DBSize
                >> strDBSerNum;

        // Allocate a buffer large enough and read it in
        THeapBuf mbufDB(c4DBSize, c4DBSize);
        strmSrc.c4ReadBuffer(mbufDB, c4DBSize);

        // Stream that into the caller's database
        {
            TBinMBufInStream strmDB(&mbufDB, c4DBSize);
            strmDB >> mdbToFill;
        }
    }

    catch(TError& errToCatch)
    {
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Low)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
        return kCIDLib::False;
    }

    catch(...)
    {
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


//
//  We always update files via a multi-step process, where we create a new
//  temp file, rename any current backup file to a temp backup name, renamee
//  the current file to the backup file, and then rename the new file to the
//  actual file name.
//
//  This insures that we never write a partial file due to some failure halfway
//  through, that we always have a backup file to drop back to, and that (even
//  if we fail after renaming the current file to the backup), the new file
//  is there can be swapped in when we come back.
//
tCIDLib::TVoid
TCQSLRepoSDriver::DoFileSwap(const  TString&    strPath
                            , const TString&    strFileName)
{
    // Build up the file paths
    TPathStr pathOrg(strPath);
    pathOrg.AddLevel(strFileName);
    TPathStr pathNew(pathOrg);
    pathNew.Append(L"_New");
    TPathStr pathBack(pathOrg);
    pathBack.Append(L"_Back");

    // And call a helper file that will do all the work atomically
    TFileSys::ReplaceFile(pathOrg, pathNew, pathBack);
}


//
//  A helper that, given a newly loaded or initialized database, will set it up as
//  our active database and set up the fields as appropriate to reflect that,
//  and store the new passed DB serial number in the appropriate field and on our
//  parent class.
//
tCIDLib::TVoid
TCQSLRepoSDriver::InstallNewDB(TMediaDB& mdbToAccept, const TString& strDBSerNum)
{
    // Tell our live database engine to accept this new DB content
    m_srdbEngine.ResetMediaDB(mdbToAccept);

    // Update the DB serial number field with the new hash
    bStoreStringFld(m_c4FldId_DBSerialNum, strDBSerNum, kCIDLib::True);

    // Set the serial number on our parent class
    SetDBSerialNum(strDBSerNum);

    // Store the total count of titles
    bStoreCardFld
    (
        m_c4FldId_TitleCnt, m_srdbEngine.mdbInfo().c4TotalTitleCnt(), kCIDLib::True
    );

    // Last of all set the status fields
    bStoreStringFld
    (
        m_c4FldId_Status
        , facCQSLRepoS().strMsg(kCQSLRMsgs::midStatus_Loaded)
        , kCIDLib::True
    );
    bStoreStringFld(m_c4FldId_LoadStatus, L"Ready", kCIDLib::True);

    // This is V1 only
    if (c4ArchVersion() == 1)
        bStoreBoolFld(m_c4FldId_Loaded, kCIDLib::True, kCIDLib::True);
}


//
//  This is called if we come up without a database file (or old format content
//  that we can convert forward.) We'll set up some initial data.
//
tCIDLib::TVoid
TCQSLRepoSDriver::SetInitDBContents(TMediaDB& mdbTar)
{
    TMediaCat* pmcatNew;

    //
    //  Create the special ones that have to have specific ids
    pmcatNew = new TMediaCat
    (
        tCQCMedia::EMediaTypes::Movie
        , facCQCMedia().strMsg(kMedMsgs::midCat_AllMovies)
        , TUniqueId::strMakeId()
    );
    pmcatNew->c2Id(kCQCMedia::c2CatId_AllMovies);
    mdbTar.c2AddCategory(pmcatNew, kCIDLib::True);

    pmcatNew = new TMediaCat
    (
        tCQCMedia::EMediaTypes::Music
        , facCQCMedia().strMsg(kMedMsgs::midCat_AllMusic)
        , TUniqueId::strMakeId()
    );
    pmcatNew->c2Id(kCQCMedia::c2CatId_AllMusic);
    mdbTar.c2AddCategory(pmcatNew, kCIDLib::True);

    pmcatNew = new TMediaCat
    (
        tCQCMedia::EMediaTypes::Music
        , facCQCMedia().strMsg(kMedMsgs::midCat_PlayLists)
        , TUniqueId::strMakeId()
    );
    pmcatNew->c2Id(kCQCMedia::c2CatId_PlayLists);
    mdbTar.c2AddCategory(pmcatNew, kCIDLib::True);

    //
    //  And now add the predefined ones. THese don't have special ids, we just
    //  let the database assign them.
    //
    tCIDLib::TCard4 c4CatInd = 0;
    while (c4CatInd < kCQSLRepoS::c4MusicCatsCnt)
    {
        pmcatNew = new TMediaCat
        (
            tCQCMedia::EMediaTypes::Music
            , kCQSLRepoS::apszMusicCats[c4CatInd]
            , TUniqueId::strMakeId()
        );
        mdbTar.c2AddCategory(pmcatNew);

        c4CatInd++;
    }

    c4CatInd = 0;
    while (c4CatInd < kCQSLRepoS::c4MovieCatsCnt)
    {
        pmcatNew = new TMediaCat
        (
            tCQCMedia::EMediaTypes::Movie
            , kCQSLRepoS::apszMovieCats[c4CatInd]
            , TUniqueId::strMakeId()
        );
        mdbTar.c2AddCategory(pmcatNew);

        c4CatInd++;
    }
}


//
//  Though almost all changes to the database are made by the repo manager and
//  then uploaded en masse, we do allow dynamic updates to some info, such as
//  user rating, which can be updated by the clients via a backdoor command. This
//  kind of sucks given our current way of doing things since we have to write out
//  the whole database and this will cause the client service to have see changes
//  and come get the new stuf, etc...
//
//  But it can't be avoided. After such changes, this is called and it just
//  coordinates the other helper methods to do what needs to be done.
//
//  Since the DB we are storing is already the active one, we don't have to do change
//  any loaded, loading, etc... status fields or swap anything in. We just need to
//  store the new data and update some fields.
//
tCIDLib::TVoid TCQSLRepoSDriver::StoreDBChanges()
{
    // Store a new DB file, and update the DB serial number info
    TString strDBSerNum;
    StoreDBFile(m_srdbEngine.mdbInfo(), strDBSerNum);

    // Update the DB serial number field with the new hash
    bStoreStringFld(m_c4FldId_DBSerialNum, strDBSerNum, kCIDLib::True);

    // And set the new serial number on our parent class
    SetDBSerialNum(strDBSerNum);

    //
    //  Though currently this couldn't change, in case it might in the future, go
    //  and and update it as well.
    //
    bStoreCardFld
    (
        m_c4FldId_TitleCnt, m_srdbEngine.mdbInfo().c4TotalTitleCnt(), kCIDLib::True
    );
}



//
//  This is called when we need to actually write out a new DB file. It handles
//  any stuff we want to put into the file, and pre-checking, etc... so that it
//  can be easily changed later. It also generates the database serial number
//  and returns that.
//
tCIDLib::TVoid
TCQSLRepoSDriver::StoreDBFile(const TMediaDB& mdbToStore, TString& strDBSerNum)
{
    // Flatten the database to a buffer
    tCIDLib::TCard4         c4DBSize = 0;
    TChunkedBinOutStream    strmOut(32 * (1024 * 1024));
    strmOut << mdbToStore << kCIDLib::FlushIt;
    c4DBSize = strmOut.c4CurSize();

    // Hash this buffer to create our database serial number
    {
        TChunkedBinInStream strmSrc(strmOut);
        facCQCMedia().CreatePersistentId(strmSrc, c4DBSize, strDBSerNum);
    }

    //
    //  If there is a back2, remove it and rename back to back2. The swapping in of
    //  the new file below will create a new back.
    //
    TPathStr pathTmp1 = m_pathDBFile;
    pathTmp1.Append(L"_Back2");
    if (TFileSys::bExists(pathTmp1))
    {
        try
        {
            TFileSys::DeleteFile(pathTmp1);
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
    }

    TPathStr pathTmp2 = m_pathDBFile;
    pathTmp2.Append(L"_Back");
    if (TFileSys::bExists(pathTmp2))
    {
        try
        {
            TFileSys::Rename(pathTmp2, pathTmp1);
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
    }


    //
    //  Write out the temp file to see if that works. Our file swapper assumes the
    //  new file is the old file with _New added to the extension.
    //
    pathTmp1 = m_pathDBFile;
    pathTmp1.Append(L"_New");
    {
        TBinFileOutStream strmTmp
        (
            pathTmp1
            , tCIDLib::ECreateActs::CreateAlways
            , tCIDLib::EFilePerms::Default
            , tCIDLib::EFileFlags::SequentialScan
            , tCIDLib::EAccessModes::Excl_OutStream
        );

        //
        //  First write out a stream marker and then a version format so that we can
        //  update it easier later. We also, for efficiency and sanity checking,
        //  store the serial number. And we write the buffer size so that, when
        //  reading back, we know how large to make the buffer.
        //
        strmTmp << tCIDLib::EStreamMarkers::StartObject
                << CQSLRepoS_DriverImpl::c2DBFmtVersion
                << c4DBSize
                << strDBSerNum;
        strmOut.c4CopyOutToStream(strmTmp, kCIDLib::c4MaxCard);
        strmTmp.Flush();
    }

    // OK, that worked, so let's swap it in to replace the old file
    DoFileSwap(m_pathStart, kCQSLRepoS::pszDBFileName);
}


//
//  Write out an image to its appropriate file, in the new mimg2 format. It'll
//  give back the path that it stored the file at.
//
tCIDLib::TVoid
TCQSLRepoSDriver::StoreImgFile( const   tCIDLib::TCard2         c2Id
                                , const tCQCMedia::EMediaTypes  eMType
                                , const TString&                strPerId
                                , const tCIDLib::TCard4         c4ImgSz
                                , const THeapBuf&               mbufImg
                                ,       TPathStr&               pathFile)
{
    try
    {
        // Build the path for this image file
        TPathStr pathTarDir;
        TPathStr pathDummy;

        BuildPath
        (
            eMType, tCQCMedia::EDataTypes::Image, c2Id, pathTarDir, pathFile, pathDummy
        );

        // Make sure the path exists
        TFileSys::MakePath(pathTarDir);

        // Stream out the data to a temp file first
        TPathStr pathTmp(pathTarDir);
        pathTmp.AddLevel(L"TmpImg.Tmp");
        {
            TBinFileOutStream strmTar
            (
                pathTmp
                , tCIDLib::ECreateActs::CreateAlways
                , tCIDLib::EFilePerms::Default
                , tCIDLib::EFileFlags::SequentialScan

            );

            strmTar << kCQSLRepoS::c2FmtImages
                    << strPerId
                    << c4ImgSz
                    << tCIDLib::TCard4(c4ImgSz ^ kCIDLib::c4MaxCard);
            strmTar.c4WriteBuffer
            (
                mbufImg, c4ImgSz, tCIDLib::EAllData::FailIfNotAll
            );
            strmTar.Flush();
        }

        // Swap it into place, replacing the old one
        TFileSys::ReplaceFile(pathFile, pathTmp);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}


