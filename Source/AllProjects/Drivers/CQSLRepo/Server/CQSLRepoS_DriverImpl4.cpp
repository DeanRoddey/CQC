//
// FILE NAME: CQSLRepoS_DriverImpl4.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 12/11/2013
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
//  This method breaks out some old code that is for loading the data in
//  it's on-disk form that was used prior to 4.4.912. Now all of the metadata
//  is in a single file, in the form of a flattened database object. It used
//  to be all spread out, so if we don't have the main file, we have to assume
//  that we've been upgraded and need to load all the data and put it into
//  the new format and write it out.
//
//  At some point we can dump this code, or possibly just move it out into
//  a separate utility that could still be used for folks upgrading from
//  long ago.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQSLRepoS_.hpp"


//
//  As of version 4.4.912, we stopped storing separate files for all of
//  the stuff except the actual image data and the media data. Instead we
//  just store a flattened media database object.
//
tCIDLib::TVoid TCQSLRepoSDriver::ConvertOldData()
{
    // Set up the top level directories
    TPathStr pathCats(m_pathStart);
    pathCats.AddLevel(L"CtDat");
    TPathStr pathCols(m_pathStart);
    pathCols.AddLevel(L"ClDat");
    TPathStr pathImgs(m_pathStart);
    pathImgs.AddLevel(L"ImDat");
    TPathStr pathItems(m_pathStart);
    pathItems.AddLevel(L"ItDat");
    TPathStr pathTitles(m_pathStart);
    pathTitles.AddLevel(L"TsDat");

    try
    {
        //
        //  Load the categories, items, collections, title sets, and images. We
        //  load them into a temp database. We had an issue with dup cats in the
        //  past, so let's take the opportunity to deal with that. when loading
        //  the cats, it'll give us back a list of dup categories. These will not
        //  have been loaded into the database. We'll use this while loading the
        //  collections to replace the dups with the ones we kept.
        //
        tCQCMedia::TCatList colDupCats(tCIDLib::EAdoptOpts::Adopt);

        TMediaDB mdbTmp;
        LoadCategories(mdbTmp, colDupCats);
        LoadArtImgs(mdbTmp);
        LoadItems(mdbTmp);
        LoadCollections(mdbTmp, colDupCats);
        LoadTitleSets(mdbTmp);

        // Call a special helper on the database to do some other upgrading work
        mdbTmp.CQSLUpdate902();

        //
        //  Tell the database we are done. It'll do a round of finalization
        //  calls on all the collections and title sets to let them do any
        //  required aggregation of info.
        //
        mdbTmp.LoadComplete();

        //
        //  Stream this info out to the new format. We don't do anything with
        //  the DB serial number. The data we've written out will later be loaded
        //  normally as new format stuff.
        //
        TString strDBSerNum;
        StoreDBFile(mdbTmp, strDBSerNum);
    }

    catch(const TError& errToCatch)
    {
        m_srdbEngine.mdbInfo().Reset();
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
        {
            if (facCQSLRepoS().bShouldLog(errToCatch))
                TModule::LogEventObj(errToCatch);

            facCQCMedia().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kMedMsgs::midStatus_ExceptInLoad
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
                , m_pathStart
            );
        }

        return;
    }

    catch(...)
    {
        m_srdbEngine.mdbInfo().Reset();
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
        {
            facCQCMedia().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kMedMsgs::midStatus_ExceptInLoad
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
                , m_pathStart
            );
        }

        return;
    }

    try
    {
        //
        //  Now we want to rename the no longer used directories so that we
        //  won't see them again. Later, the user or the installer can delete
        //  them when it's certain they won't be required.
        //
        TString strNew;
        strNew = pathCats;
        strNew.Append(L"_Save");
        TFileSys::Rename(pathCats, strNew);

        strNew = pathCols;
        strNew.Append(L"_Save");
        TFileSys::Rename(pathCols, strNew);

        strNew = pathTitles;
        strNew.Append(L"_Save");
        TFileSys::Rename(pathTitles, strNew);
    }

    catch(...)
    {
    }
}


//
//  As of version 4.4.902, we converted the .mimg files to a new format
//  which includes a persistent id in the file, so that we don't have to
//  calculate one every time we load.
//
tCIDLib::TVoid
TCQSLRepoSDriver::ConvertOldImgFiles(const  TString&                strPath
                                    , const tCQCMedia::EMediaTypes  eMType)
{
    const tCIDLib::EDirSearchFlags eFlags = tCIDLib::EDirSearchFlags::NormalFiles;

    TDirIter    diterOld;
    TFindBuf    fndbCur;
    TSize       szMaxArt(400);

    if (diterOld.bFindFirst(strPath, L"*.mimg", fndbCur, eFlags))
    {
        THeapBuf                mbufData(0x200000, 0x800000, 0x100000);
        TPathStr                pathTmp;
        TPathStr                pathTmp2;
        TString                 strPerId;
        TString                 strName;

        do
        {
            //
            //  See if there is already an .mimg2 file for this one. If
            //  so, then this guy needs to be renamed since it's already
            //  been done.
            //
            pathTmp = fndbCur.pathFileName();
            pathTmp.Append(L"2");
            if (TFileSys::bExists(pathTmp, eFlags))
            {
                pathTmp = fndbCur.pathFileName();
                pathTmp.Append(L"_Old");

                try
                {
                    TFileSys::Rename(fndbCur.pathFileName(), pathTmp);
                }

                catch(...)
                {
                }

                // And skip this one
                continue;
            }

            try
            {
                // Convert the name to an id
                fndbCur.pathFileName().bQueryName(strName);
                tCIDLib::TCard2 c2Id
                (
                    tCIDLib::TCard2(strName.c4Val(tCIDLib::ERadices::Hex))
                );

                //
                //  Read in the file contents.
                //
                TBinaryFile flSrc(fndbCur.pathFileName());
                flSrc.Open
                (
                    tCIDLib::EAccessModes::Read
                    , tCIDLib::ECreateActs::OpenIfExists
                    , tCIDLib::EFilePerms::Default
                    , tCIDLib::EFileFlags::SequentialScan
                );

                tCIDLib::TCard4 c4DataSz = tCIDLib::TCard4(flSrc.c8CurSize());
                flSrc.c4ReadBuffer(mbufData, c4DataSz, tCIDLib::EAllData::FailIfNotAll);
                flSrc.Close();

                //
                //  We take this opportunity to address the possibility that
                //  overly large images have gotten into the database. If so,
                //  then we try to scale it down. This may or may not actually
                //  scale it. If it's not larger than our max image res,
                //  this won't do anything. But, generally it should make
                //  it smaller if the raw data is this large.
                //
                if (c4DataSz > 32 * 1024)
                {
                    c4DataSz = facCIDImgFact().c4CompactToJPEG
                    (
                        mbufData, c4DataSz, szMaxArt
                    );
                }

                // Generate the persistent id
                facCQCMedia().CreatePersistentId(mbufData, c4DataSz, strPerId);

                //
                //  OK, now let's write out the new 2 version. This method is
                //  used when accepting new uploads as well so it gives back the
                //  path it stored it, which we just ignore here.
                //
                StoreImgFile
                (
                    c2Id, eMType, strPerId, c4DataSz, mbufData, pathTmp
                );
            }

            catch(TError& errToCatch)
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }

        }   while (diterOld.bFindNext(fndbCur));
    }
}


//
//  Load up the art image objects. We don't actually load the image data
//  here, we just set up the image objects, so that we know what's out
//  there. We fault in the image data into the image objects as they are
//  accessed.
//
//  However, we do have to deal with a conversion issue, where we have to
//  convert from the old .mimg file to the .mimg2 file. The old one was
//  just the image data itself, so we can't really use any sort of smart
//  updating. We just see if there are any .mimg files. If so, we read in
//  the data, and write out the new file, creating the peristent id as we
//  go.
//
tCIDLib::TVoid TCQSLRepoSDriver::LoadArtImgs(TMediaDB& mdbTmp)
{
    //
    //  Loop through the media types, since we have to iterate each type
    //  separately.
    //
    const tCIDLib::EDirSearchFlags eDirs = tCIDLib::EDirSearchFlags::NormalDirs;
    const tCIDLib::EDirSearchFlags eFiles = tCIDLib::EDirSearchFlags::NormalFiles;

    TBinaryFile     flTmp;
    TDirIter        diterDirs;
    TDirIter        diterFiles;
    TFindBuf        fndbCur;
    THeapBuf        mbufId(2060UL, 2060UL);
    TPathStr        pathDir;
    TPathStr        pathFile;
    TPathStr        strDummy;
    TString         strName;
    TString         strPerId;

    tCQCMedia::EMediaTypes eType = tCQCMedia::EMediaTypes::Min;
    for (; eType <= tCQCMedia::EMediaTypes::Max; eType++)
    {
        //
        //  Build the root path to categories of this type. It also builds
        //  a file for a particular id, so just pass a dummy id. And we have
        //  to remove the last component of the directory path since it
        //  includes the actual directory name of the dummy image id.
        //
        BuildPath(eType, tCQCMedia::EDataTypes::Image, 0, pathDir, pathFile, strDummy);
        pathDir.bRemoveLevel();

        //
        //  Iterate the directories under here. They will be numbered
        //  directories, each one holding up to a maximum number of
        //  files. If none, then go back and try the next media type.
        //
        if (!diterDirs.bFindFirst(pathDir, L"*", fndbCur, eDirs))
            continue;

        // We found some, so iterate them
        do
        {
            //
            //  First need to handle conversion to the new .mimg2 file
            //  format. This should be a one time thing.
            //
            ConvertOldImgFiles(fndbCur.pathFileName(), eType);

            //
            //  Now let's iterate any files in this directory for our
            //  mimg2 files and load them up.
            //
            pathDir = fndbCur.pathFileName();
            if (diterFiles.bFindFirst(pathDir, L"*.mimg2", fndbCur, eFiles))
            {
                do
                {
                    try
                    {
                        // Get the id from the file name
                        fndbCur.pathFileName().bQueryName(strName);
                        tCIDLib::TCard2 c2Id
                        (
                            tCIDLib::TCard2(strName.c4Val(tCIDLib::ERadices::Hex))
                        );

                        //
                        //  Read in the non-image stuff. What we are mainly
                        //  interested in here is the stored persistent id,
                        //  which we need to get into the image data now.
                        //
                        {
                            TBinFileInStream strmSrc
                            (
                                fndbCur.pathFileName()
                                , tCIDLib::ECreateActs::OpenIfExists
                                , tCIDLib::EFilePerms::Default
                                , tCIDLib::EFileFlags::SequentialScan
                            );

                            tCIDLib::TCard2 c2FmtVer;
                            strmSrc >> c2FmtVer;
                            CIDAssert
                            (
                                c2FmtVer == kCQSLRepoS::c2FmtImages
                                , L"Invalid format version in image file"
                            );
                            strmSrc >> strPerId;
                        }

                        //
                        //  And add it to our list. We have to force an existing id
                        //  and the persistent id in this case. We don't support
                        //  small art, so we set the same persistent id on both large
                        //  and small art. If we don't do this, the client service
                        //  won't think there's any small art.
                        //
                        TMediaImg* pmimgNew = new TMediaImg
                        (
                            eType, fndbCur.pathFileName(), TString::strEmpty()
                        );
                        pmimgNew->c2Id(c2Id);
                        pmimgNew->SetPersistentId(strPerId, tCQCMedia::ERArtTypes::LrgCover);
                        pmimgNew->SetPersistentId(strPerId, tCQCMedia::ERArtTypes::SmlCover);

                        // Generate a unique id for this one
                        pmimgNew->strUniqueId(TUniqueId::strMakeId());
                        mdbTmp.c2AddImage(pmimgNew, kCIDLib::True);
                    }

                    catch(TError& errToCatch)
                    {
                        LogError
                        (
                            errToCatch
                            , tCQCKit::EVerboseLvls::Medium
                            , CID_FILE
                            , CID_LINE
                        );
                        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
                        {
                            facCQSLRepoS().LogMsg
                            (
                                CID_FILE
                                , CID_LINE
                                , kCQSLRErrs::errcDB_BadFile
                                , tCIDLib::ESeverities::Failed
                                , tCIDLib::EErrClasses::Format
                                , fndbCur.pathFileName()
                                , tCQCMedia::strXlatEDataTypes(tCQCMedia::EDataTypes::Cat)
                            );
                        }
                    }
                }   while (diterFiles.bFindNext(fndbCur));
            }
        }   while (diterDirs.bFindNext(fndbCur));
    }
}


//
//  We remove dups and save them for later use to help get rid of the dup refs.
//
tCIDLib::TVoid
TCQSLRepoSDriver::LoadCategories(TMediaDB& mdbTmp, tCQCMedia::TCatList& colDupCats)
{
    //
    //  Loop through the media types, since we have to iterate each type
    //  separately.
    //
    const tCIDLib::EDirSearchFlags eDirs = tCIDLib::EDirSearchFlags::NormalDirs;
    const tCIDLib::EDirSearchFlags eFiles = tCIDLib::EDirSearchFlags::NormalFiles;

    tCIDLib::TCard4 c4At;
    TDirIter        diterDirs;
    TDirIter        diterFiles;
    TFindBuf        fndbCur;
    TPathStr        pathDir;
    TPathStr        pathFile;
    TPathStr        pathDummy;

    tCQCMedia::EMediaTypes eType = tCQCMedia::EMediaTypes::Min;
    for (; eType <= tCQCMedia::EMediaTypes::Max; eType++)
    {
        //
        //  Build the root path to categories of this type. It also builds
        //  a file for a particular id, so just pass a dummy id. And we have
        //  to remove the last component of the directory path since it
        //  includes the actual directory name of the dummy category id.
        //
        BuildPath(eType, tCQCMedia::EDataTypes::Cat, 0, pathDir, pathFile, pathDummy);
        pathDir.bRemoveLevel();

        //
        //  Iterate the directories under here. They will be numbered
        //  directories, each one holding up to a maximum number of
        //  files. If none, then go back and try the next media type.
        //
        if (!diterDirs.bFindFirst(pathDir, L"*", fndbCur, eDirs))
            continue;

        // We found some, so iterate them
        TString strUID;
        do
        {
            // Now let's iterate any files in this directory
            pathDir = fndbCur.pathFileName();
            if (diterFiles.bFindFirst(pathDir, L"*.mcat", fndbCur, eFiles))
            {
                do
                {
                    try
                    {
                        // Try to stream this one in
                        TMediaCat* pmcatCur = new TMediaCat;
                        TJanitor<TMediaCat> janCat(pmcatCur);

                        TBinFileInStream strmTar
                        (
                            fndbCur.pathFileName()
                            , tCIDLib::ECreateActs::OpenIfExists
                            , tCIDLib::EFilePerms::Default
                            , tCIDLib::EFileFlags::SequentialScan
                        );
                        strmTar >> *pmcatCur;

                        //
                        //  See if this is a dup. If so, then don't add it. Put it
                        //  in the dup list for later use.
                        //
                        const TMediaCat* pmcatOrg = mdbTmp.pmcatByUniqueId
                        (
                            eType, pmcatCur->strUniqueId(), kCIDLib::False
                        );

                        //
                        //  If it's a dup put it in the dup list. Put the id of the
                        //  original one into the alt id so we can map back to the
                        //  right one later. Else just add it to the DB.
                        //
                        if (pmcatOrg)
                        {
                            pmcatCur->c4AltId(pmcatOrg->c2Id());
                            colDupCats.InsertSorted
                            (
                                janCat.pobjOrphan(), &TMediaCat::eCompById, c4At
                            );
                        }
                         else
                        {
                            //
                            //  We have to force it to take the existing id, since we
                            //  are loading already set up DB content.
                            //
                            mdbTmp.c2AddCategory(janCat.pobjOrphan(), kCIDLib::True);
                        }
                    }

                    catch(TError& errToCatch)
                    {
                        LogError
                        (
                            errToCatch
                            , tCQCKit::EVerboseLvls::Medium
                            , CID_FILE
                            , CID_LINE
                        );
                        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
                        {
                            facCQSLRepoS().LogMsg
                            (
                                CID_FILE
                                , CID_LINE
                                , kCQSLRErrs::errcDB_BadFile
                                , tCIDLib::ESeverities::Failed
                                , tCIDLib::EErrClasses::Format
                                , fndbCur.pathFileName()
                                , tCQCMedia::strXlatEDataTypes(tCQCMedia::EDataTypes::Cat)
                            );
                        }
                    }
                }   while (diterFiles.bFindNext(fndbCur));
            }
        }   while (diterDirs.bFindNext(fndbCur));
    }
}


tCIDLib::TVoid TCQSLRepoSDriver::LoadItems(TMediaDB& mdbTmp)
{
    const tCIDLib::EDirSearchFlags eDirs = tCIDLib::EDirSearchFlags::NormalDirs;
    const tCIDLib::EDirSearchFlags eFiles = tCIDLib::EDirSearchFlags::NormalFiles;

    //
    //  Loop through the media types, since we have to iterate each type
    //  separately.
    //
    tCIDLib::TBoolean       bRes;
    TDirIter                diterDirs;
    TDirIter                diterFiles;
    TFindBuf                fndbCur;
    TPathStr                pathDir;
    TPathStr                pathFile;
    TPathStr                pathTmp;
    TPathStr                pathTmp2;
    TPathStr                pathDummy;

    tCQCMedia::EMediaTypes  eType = tCQCMedia::EMediaTypes::Min;
    for (; eType <= tCQCMedia::EMediaTypes::Max; eType++)
    {
        //
        //  Build the root path to items of this type. It also builds
        //  a file for a particular id, so just pass a dummy id. And we have
        //  to remove the last component of the directory path since it
        //  includes the actual directory name of the dummy id.
        //
        BuildPath(eType, tCQCMedia::EDataTypes::Item, 0, pathDir, pathFile, pathDummy);
        pathDir.bRemoveLevel();

        //
        //  Iterate the directories under here. They will be numbered
        //  directories, each one holding up to a maximum number of
        //  files. If none, then go back and try the next media type.
        //
        if (!diterDirs.bFindFirst(pathDir, L"*", fndbCur, eDirs))
            continue;

        // We found some, so iterate them
        do
        {
            // Now let's iterate any files in this directory
            pathDir = fndbCur.pathFileName();
            if (diterFiles.bFindFirst(pathDir, L"*.mitem", fndbCur, eFiles))
            {
                do
                {
                    try
                    {
                        // Try to stream this one in
                        TMediaItem* pmitemCur = new TMediaItem;
                        TJanitor<TMediaItem> janItem(pmitemCur);

                        {
                            TBinFileInStream strmSrc
                            (
                                fndbCur.pathFileName()
                                , tCIDLib::ECreateActs::OpenIfExists
                                , tCIDLib::EFilePerms::Default
                                , tCIDLib::EFileFlags::SequentialScan
                            );
                            strmSrc >> *pmitemCur;
                        }

                        //
                        //  If it has a media path, we want to adjust it so
                        //  that it's relative to our current start path.
                        //
                        //  We also used to not set any unique id on items,
                        //  but other repos do now and so the database
                        //  requires it. So, if this guy doesn't have one,
                        //  generate one.
                        //
                        if ((!pmitemCur->strLocInfo().bIsEmpty()
                        &&   !pmitemCur->strLocInfo().bStartsWith(m_pathStart))
                        ||   pmitemCur->strUniqueId().bIsEmpty())
                        {
                            if (!pmitemCur->strLocInfo().bIsEmpty()
                            &&  !pmitemCur->strLocInfo().bStartsWith(m_pathStart))
                            {
                                //
                                //  Find the \ItDat\ part. That's where the
                                //  relative part starts.
                                //
                                pathTmp = pmitemCur->strLocInfo();
                                tCIDLib::TCard4 c4Ofs;
                                bRes = pathTmp.bFirstOccurrence
                                (
                                    L"\\ItDat\\"
                                    , c4Ofs
                                    , kCIDLib::False
                                );

                                if (bRes)
                                {
                                    pathTmp.Cut(0, c4Ofs + 1);
                                    pathTmp.AddToBasePath(m_pathStart);
                                    pmitemCur->strLocInfo(pathTmp);
                                }
                            }

                            if (pmitemCur->strUniqueId().bIsEmpty())
                            {
                                pmitemCur->strUniqueId(TUniqueId::strMakeId());
                                bRes = kCIDLib::True;
                            }

                            if (bRes)
                            {
                                //
                                //  And write it back out. First to a temp,
                                //  and then swap it.
                                //
                                pathTmp2 = fndbCur.pathFileName();
                                pathTmp2.Append(L"_Tmp");
                                {
                                    TBinFileOutStream strmTar
                                    (
                                        pathTmp2
                                        , tCIDLib::ECreateActs::CreateAlways
                                        , tCIDLib::EFilePerms::Default
                                        , tCIDLib::EFileFlags::SequentialScan
                                    );
                                    strmTar << *pmitemCur << kCIDLib::FlushIt;
                                }

                                TFileSys::ReplaceFile
                                (
                                    fndbCur.pathFileName(), pathTmp2
                                );
                            }
                        }

                        //
                        //  Add the item now.
                        //
                        //  We have to force it to take the existing id, since we
                        //  are loading already set up DB content.
                        //
                        mdbTmp.c2AddItem(janItem.pobjOrphan(), kCIDLib::True);
                    }

                    catch(TError& errToCatch)
                    {
                        LogError
                        (
                            errToCatch
                            , tCQCKit::EVerboseLvls::Medium
                            , CID_FILE
                            , CID_LINE
                        );
                        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
                        {
                            facCQSLRepoS().LogMsg
                            (
                                CID_FILE
                                , CID_LINE
                                , kCQSLRErrs::errcDB_BadFile
                                , tCIDLib::ESeverities::Failed
                                , tCIDLib::EErrClasses::Format
                                , fndbCur.pathFileName()
                                , tCQCMedia::strXlatEDataTypes(tCQCMedia::EDataTypes::Item)
                            );
                        }
                    }
                }   while (diterFiles.bFindNext(fndbCur));
            }
        }   while (diterDirs.bFindNext(fndbCur));
    }
}


//
//  Look at all of the categories this collection references. If any aren't in the
//  database, then see if they are in the dup list. If so, replace them with the
//  id of the one we kept, whose id is stored in an alternate id field of the object
//  in the cat list.
//
tCIDLib::TVoid
TCQSLRepoSDriver::CheckDupCats(         TMediaDB&                   mdbSrc
                                ,       TMediaCollect&              mcolCur
                                , const tCQCMedia::TCatList&        colDupCats
                                ,       tCQCMedia::TMediaIdList&    fcolDupReps)
{
    //
    //  Store the ones we are going to add back in. We don't want to do it as we
    //  go since it'll mess with our iteration. Clear the list before we start.
    //
    fcolDupReps.RemoveAll();

    tCIDLib::TCard4 c4At;
    tCIDLib::TCard4 c4Index = 0;
    tCIDLib::TCard4 c4CatCnt = mcolCur.c4CatCount();
    while (c4Index < c4CatCnt)
    {
        const tCIDLib::TCard2 c2Id = mcolCur.c2CatIdAt(c4Index);

        // See if this one is in the database
        const TMediaCat* pmcatFind = mdbSrc.pmcatById(mcolCur.eType(), c2Id, kCIDLib::False);

        if (!pmcatFind)
        {
            // Remove this category, since it's not in the DB
            mcolCur.RemoveCategory(c2Id);
            c4CatCnt--;

            // If it's in the dup list, then remember this one to add back
            const TMediaCat* pmcatDup = colDupCats.pobjKeyedBinarySearch
            (
                c2Id, &TMediaCat::eIdKeyComp, c4At
            );
            if (pmcatDup)
                fcolDupReps.c4AddElement(tCIDLib::TCard2(pmcatDup->c4AltId()));
        }
         else
        {
            c4Index++;
        }
    }

    // Add any we replaced
    mcolCur.AddCatsToList(fcolDupReps);
}


//
//  We check for any dup categories and replace them with the version we kept.
//
tCIDLib::TVoid
TCQSLRepoSDriver::LoadCollections(          TMediaDB&               mdbTmp
                                    , const tCQCMedia::TCatList&    colDupCats)
{
    //
    //  Loop through the media types, since we have to iterate each type
    //  separately.
    //
    const tCIDLib::EDirSearchFlags eDirs = tCIDLib::EDirSearchFlags::NormalDirs;
    const tCIDLib::EDirSearchFlags eFiles = tCIDLib::EDirSearchFlags::NormalFiles;
    TDirIter    diterDirs;
    TDirIter    diterFiles;
    TFindBuf    fndbCur;
    TPathStr    pathDir;
    TPathStr    pathFile;
    TPathStr    pathDummy;

    tCQCMedia::TMediaIdList fcolDupReps;
    tCQCMedia::EMediaTypes eType = tCQCMedia::EMediaTypes::Min;
    for (; eType <= tCQCMedia::EMediaTypes::Max; eType++)
    {
        //
        //  Build the root path to collections of this type. It also builds
        //  a file for a particular id, so just pass a dummy id. And we have
        //  to remove the last component of the directory path since it
        //  includes the actual directory name of the dummy id.
        //
        BuildPath(eType, tCQCMedia::EDataTypes::Collect, 0, pathDir, pathFile, pathDummy);
        pathDir.bRemoveLevel();

        //
        //  Iterate the directories under here. They will be numbered dirs, each one
        //  holding up to a maximum number of files. If none, then go back and try
        //  the next media type.
        //
        if (!diterDirs.bFindFirst(pathDir, L"*", fndbCur, eDirs))
            continue;

        // We found some, so iterate them
        do
        {
            // Now let's iterate any files in this directory
            pathDir = fndbCur.pathFileName();
            if (diterFiles.bFindFirst(pathDir, L"*.mcol", fndbCur, eFiles))
            {
                do
                {
                    try
                    {
                        // Try to stream this one in
                        TMediaCollect* pmcolCur = new TMediaCollect;
                        TJanitor<TMediaCollect> janCol(pmcolCur);
                        TBinFileInStream strmTar
                        (
                            fndbCur.pathFileName()
                            , tCIDLib::ECreateActs::OpenIfExists
                            , tCIDLib::EFilePerms::Default
                            , tCIDLib::EFileFlags::SequentialScan
                        );
                        strmTar >> *pmcolCur;

                        // Take care of any dup categories
                        CheckDupCats(mdbTmp, *pmcolCur, colDupCats, fcolDupReps);

                        //
                        //  We have to force it to take the existing id, since we
                        //  are loading already set up DB content.
                        //
                        mdbTmp.c2AddCollect(janCol.pobjOrphan(), kCIDLib::True);
                    }

                    catch(TError& errToCatch)
                    {
                        LogError
                        (
                            errToCatch
                            , tCQCKit::EVerboseLvls::Medium
                            , CID_FILE
                            , CID_LINE
                        );
                        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
                        {
                            facCQSLRepoS().LogMsg
                            (
                                CID_FILE
                                , CID_LINE
                                , kCQSLRErrs::errcDB_BadFile
                                , tCIDLib::ESeverities::Failed
                                , tCIDLib::EErrClasses::Format
                                , fndbCur.pathFileName()
                                , tCQCMedia::strXlatEDataTypes(tCQCMedia::EDataTypes::Collect)
                            );
                        }
                    }
                }   while (diterFiles.bFindNext(fndbCur));
            }
        }   while (diterDirs.bFindNext(fndbCur));
    }
}

tCIDLib::TVoid TCQSLRepoSDriver::LoadTitleSets(TMediaDB& mdbTmp)
{
    //
    //  Loop through the media types, since we have to iterate each type
    //  separately.
    //
    const tCIDLib::EDirSearchFlags eDirs = tCIDLib::EDirSearchFlags::NormalDirs;
    const tCIDLib::EDirSearchFlags eFiles = tCIDLib::EDirSearchFlags::NormalFiles;
    TDirIter            diterDirs;
    TDirIter            diterFiles;
    TFindBuf            fndbCur;
    TPathStr            pathDir;
    TPathStr            pathFile;
    TPathStr            pathDummy;

    TString             strTmp;
    tCQCMedia::EMediaTypes eType = tCQCMedia::EMediaTypes::Min;
    for (; eType <= tCQCMedia::EMediaTypes::Max; eType++)
    {
        //
        //  Build the root path to title set of this type. It also builds
        //  a file for a particular id, so just pass a dummy id. And we have
        //  to remove the last component of the directory path since it
        //  includes the actual directory name of the dummy id.
        //
        BuildPath(eType, tCQCMedia::EDataTypes::TitleSet, 0, pathDir, pathFile, pathDummy);
        pathDir.bRemoveLevel();

        //
        //  Iterate the directories under here. They will be numbered
        //  directories, each one holding up to a maximum number of
        //  files. If none, then go back and try the next media type.
        //
        if (!diterDirs.bFindFirst(pathDir, L"*", fndbCur, eDirs))
            continue;

        // Assign sequential sequence numbers within each media type
        tCIDLib::TCard4 c4SeqNum = 1;

        // We found some, so iterate them
        do
        {
            // Now let's iterate any files in this directory
            pathDir = fndbCur.pathFileName();
            if (diterFiles.bFindFirst(pathDir, L"*.mts", fndbCur, eFiles))
            {
                do
                {
                    try
                    {
                        // Try to stream this one in
                        TMediaTitleSet* pmtsCur = new TMediaTitleSet;
                        TJanitor<TMediaTitleSet> janSet(pmtsCur);

                        TBinFileInStream strmTar
                        (
                            fndbCur.pathFileName()
                            , tCIDLib::ECreateActs::OpenIfExists
                            , tCIDLib::EFilePerms::Default
                            , tCIDLib::EFileFlags::SequentialScan
                        );
                        strmTar >> *pmtsCur;

                        // Assign it the next sequence number
                        pmtsCur->c4SeqNum(c4SeqNum++);

                        //
                        //  We have to force it to take the existing id, since we
                        //  are loading already set up DB content.
                        //
                        mdbTmp.c2AddTitle(janSet.pobjOrphan(), kCIDLib::True);
                    }

                    catch(TError& errToCatch)
                    {
                        LogError
                        (
                            errToCatch
                            , tCQCKit::EVerboseLvls::Medium
                            , CID_FILE
                            , CID_LINE
                        );
                        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
                        {
                            facCQSLRepoS().LogMsg
                            (
                                CID_FILE
                                , CID_LINE
                                , kCQSLRErrs::errcDB_BadFile
                                , tCIDLib::ESeverities::Failed
                                , tCIDLib::EErrClasses::Format
                                , fndbCur.pathFileName()
                                , tCQCMedia::strXlatEDataTypes(tCQCMedia::EDataTypes::TitleSet)
                            );
                        }
                    }
                }   while (diterFiles.bFindNext(fndbCur));
            }
        }   while (diterDirs.bFindNext(fndbCur));
    }
}


