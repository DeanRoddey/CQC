//
// FILE NAME: CQCDataSrv_IRServerImpl.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/18/2002
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
//  This file implements the IR repository server interface.
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
#include    "CQCDataSrv.hpp"


// ---------------------------------------------------------------------------
//  Magic Securitys
// ---------------------------------------------------------------------------
RTTIDecls(TCQCIRSrvImpl,TIRRepoServerBase)


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace
{
    namespace CQCDataSrv_IRServerImpl
    {
        // -----------------------------------------------------------------------
        //  Prefix and extension for our files
        // -----------------------------------------------------------------------
        constexpr const tCIDLib::TCh* const   pszFilePref = L"IRData_";
        constexpr const tCIDLib::TCh* const   pszFileExt  = L"CQCIR";
    }
}


// ---------------------------------------------------------------------------
//   CLASS: TCQCIRSrvImpl
//  PREFIX: orbs
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIRSrvImpl: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIRSrvImpl::TCQCIRSrvImpl() :

    m_colKey(109, TStringKeyOps())
{
    //
    //  Set up our IR repo path. In dev environ mode we point at the source files.
    //  Else we point it at the real delivered system path.
    //
    if (facCQCKit().bDevMode())
    {
        if (!TProcEnvironment::bFind(L"CQC_SRCTREE", m_strRepoDir))
        {
            facCQCDataSrv.ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kDSrvErrs::errcInst_DevEnvNotFnd
                , tCIDLib::ESeverities::ProcFatal
                , tCIDLib::EErrClasses::NotFound
            );
        }
        m_strRepoDir.Append(L"\\Miscellaneous\\IRLibraries");
    }
     else
    {
        // It's in the standard place
        m_strRepoDir = facCQCKit().strDataDir();
        m_strRepoDir.Append(L"\\DataServer\\IRModels");
    }
}

TCQCIRSrvImpl::~TCQCIRSrvImpl()
{
    m_colKey.RemoveAll();
}


// ---------------------------------------------------------------------------
//  TCQCIRSrvImpl: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TCQCIRSrvImpl::bModelExists(const   TString&    strDevModel
                            , const TString&    strBlasterName)
{
    // Lock and look up the key for the indicated blaster
    TLocker lockrSync(&m_mtxSync);
    TBlasterDevList* pcolKey = pcolFindKey(strBlasterName);

    // If it exists in the key, it exists
    return pcolKey->bKeyExists(strDevModel);
}


tCIDLib::TCard4
TCQCIRSrvImpl::c4QueryDevCatList(const  tCQCKit::EDevCats eCategory
                                , const TString&          strBlasterName
                                ,       TModInfoList&     colToFill)
{
    // Lock and look up the key for the indicated blaster
    TLocker lockrSync(&m_mtxSync);
    TBlasterDevList* pcolKey = pcolFindKey(strBlasterName);

    // Search the key and copy over any that are in the indicated category
    TBlasterDevList::TCursor cursKey(pcolKey);
    for (; cursKey; ++cursKey)
    {
        if (cursKey->eCategory() == eCategory)
            colToFill.objAdd(*cursKey);
    }
    return colToFill.c4ElemCount();
}


tCIDLib::TCard4
TCQCIRSrvImpl::c4QueryDevModelList( const  TString&         strBlasterName
                                    ,      TModInfoList&    colToFill)
{
    // Lock and look up the key for the indicated blaster
    TLocker lockrSync(&m_mtxSync);
    TBlasterDevList* pcolKey = pcolFindKey(strBlasterName);

    // Search the list and copy over all of the items to the caller's list
    TBlasterDevList::TCursor cursKey(pcolKey);
    for (; cursKey; ++cursKey)
        colToFill.objAdd(*cursKey);

    return colToFill.c4ElemCount();
}


tCIDLib::TVoid
TCQCIRSrvImpl::DeleteDevModel(  const   TString&    strDevModel
                                , const TString&    strBlasterName)
{
    // Lock and look up the key for the indicated blaster
    TLocker lockrSync(&m_mtxSync);
    TBlasterDevList* pcolKey = pcolFindKey(strBlasterName);

    // Look up this model in the key list
    TIRBlasterDevModelInfo* pirbdmiRead = pcolKey->pobjFindByKey(strDevModel);

    // If we didn't find it, that's an error
    if (!pirbdmiRead)
    {
        facCQCDataSrv.ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kDSrvErrs::errcIR_UnknownModel
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , strDevModel
            , strBlasterName
        );
    }

    // Build up the path to the file in question
    TPathStr pathToLoad(m_strRepoDir);
    pathToLoad.AddLevel(strBlasterName);
    pathToLoad.AddLevel(pirbdmiRead->strDataFile());
    pathToLoad.AppendExt(CQCDataSrv_IRServerImpl::pszFileExt);

    // Remove the key from the key list
    pcolKey->bRemoveKey(strDevModel);

    // Delete the file
    TFileSys::DeleteFile(pathToLoad);
}


tCIDLib::TVoid
TCQCIRSrvImpl::QueryDevModel(const  TString&            strDevModel
                            , const TString&            strBlasterName
                            ,       TIRBlasterDevModel& irbdmToFill)
{
    // Lock and look up the key for the indicated blaster
    TLocker lockrSync(&m_mtxSync);
    TBlasterDevList* pcolKey = pcolFindKey(strBlasterName);

    // Look up this model in the key list
    TIRBlasterDevModelInfo* pirbdmiRead = pcolKey->pobjFindByKey(strDevModel);

    // If we didn't find it, that's an error
    if (!pirbdmiRead)
    {
        facCQCDataSrv.ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kDSrvErrs::errcIR_UnknownModel
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , strDevModel
            , strBlasterName
        );
    }

    // Build up the path to the file in question
    TPathStr pathToLoad(m_strRepoDir);
    pathToLoad.AddLevel(strBlasterName);
    pathToLoad.AddLevel(pirbdmiRead->strDataFile());
    pathToLoad.AppendExt(CQCDataSrv_IRServerImpl::pszFileExt);

    tCIDLib::TCard4 c4ToRead;
    tCIDLib::TCard4 c4Read;
    THeapBuf        mbufToFill;
    {
        // Open it up as a raw binary file
        TBinaryFile flSrc(pathToLoad);
        flSrc.Open
        (
            tCIDLib::EAccessModes::Multi_Read
            , tCIDLib::ECreateActs::OpenIfExists
            , tCIDLib::EFilePerms::Default
            , tCIDLib::EFileFlags::SequentialScan
        );

        // And read it into the caller's buffer, returning the size
        c4ToRead = tCIDLib::TCard4(flSrc.c8CurSize());
        c4Read = flSrc.c4ReadBuffer
        (
            mbufToFill, c4ToRead, tCIDLib::EAllData::FailIfNotAll
        );
    }

    // Make sure we read it all successfully
    if (c4Read != c4ToRead)
    {
        facCIDLib().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCIDErrs::errcFile_NotAllData
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotAllRead
            , facCIDLib().strMsg(kCIDMsgs::midGen_read)
            , TCardinal(c4ToRead)
            , TCardinal(c4Read)
        );
    }

    //
    //  The last TCard4 value should be a checksum. So run through the
    //  buffer and checksum it, and see if it matches.
    //
    const tCIDLib::TCard4 c4Sum = mbufToFill.c4CheckSum(0, c4Read - 4);
    if (c4Sum != mbufToFill.c4At(c4Read - 4))
    {
        facCQCDataSrv.ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kDSrvErrs::errcIR_BadCheckSum
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , strDevModel
        );
    }

    //
    //  Open it up as a binary input stream and ask the caller's device
    //  object stream in from it. Set the current end of stream so that
    //  it doesn't include the trailing checksum.
    //
    TBinMBufInStream strmDev(&mbufToFill, c4Read - 4);
    strmDev >> irbdmToFill;
}


tCIDLib::TVoid
TCQCIRSrvImpl::WriteDevModel(const  TString&            strDevModel
                            , const TString&            strBlasterName
                            , const TIRBlasterDevModel& irbdmToWrite)
{
    //
    //  A little state machine for the steps we go through, so that we can
    //  undo it if something goes awry.
    //
    enum EStates
    {
        EState_WriteDev
        , EState_CreateTmp
        , EState_RemoveBak
        , EState_RenameCur
        , EState_RenameTmp
    };

    // Look and look up this device
    TLocker lockrSync(&m_mtxSync);
    TBlasterDevList* pcolKey = pcolFindKey(strBlasterName);

    // Look up this device in the key
    TIRBlasterDevModelInfo* pirbdmiData = pcolKey->pobjFindByKey(strDevModel);

    //
    //  If we found it, then us the file name from the object, else we have
    //  to gen up a new file name.
    //
    TPathStr pathNewFile;
    if (pirbdmiData)
    {
        pathNewFile = pirbdmiData->strDataFile();
    }
     else
    {
        //
        //  Create a new device info object and add it to the key. We have
        //  to generate a unique id name for it.
        //
        pathNewFile = CQCDataSrv_IRServerImpl::pszFilePref;
        pathNewFile.Append(TUniqueId::strMakeId());

        //
        //  There is some slight chance that we might get a dup, so loop
        //  until we get a unique file.
        //
        TPathStr pathCheck;
        while (kCIDLib::True)
        {
            pathCheck = m_strRepoDir;
            pathCheck.AddLevel(strBlasterName);
            pathCheck.AddLevel(pathNewFile);
            pathCheck.AppendExt(CQCDataSrv_IRServerImpl::pszFileExt);

            if (!TFileSys::bExists(pathCheck))
                break;

            pathNewFile = CQCDataSrv_IRServerImpl::pszFilePref;
            pathNewFile.Append(TUniqueId::strMakeId());
        }
    }

    // Make sure that the path exists, else create it
    TPathStr pathBackup(m_strRepoDir);
    pathBackup.AddLevel(strBlasterName);
    if (!TFileSys::bIsDirectory(pathBackup))
        TFileSys::MakePath(pathBackup);

    // Build up the path to the backup file for this device
    pathBackup.AddLevel(pathNewFile);
    pathBackup.AppendExt(L"Backup");

    // And the path for a temp file we'll write to first
    TPathStr pathTmp(m_strRepoDir);
    pathTmp.AddLevel(strBlasterName);
    pathTmp.AddLevel(L"TempWrite.Data");

    // And the actual file we want to create
    TPathStr pathToWrite(m_strRepoDir);
    pathToWrite.AddLevel(strBlasterName);
    pathToWrite.AddLevel(pathNewFile);
    pathToWrite.AppendExt(CQCDataSrv_IRServerImpl::pszFileExt);

    EStates eState = EState_WriteDev;
    try
    {
        // Make a memory buffer output stream and stream the caller's object
        TBinMBufOutStream strmOut(1024 * 16);
        strmOut << irbdmToWrite << kCIDLib::FlushIt;

        // Checksum the buffer and write the checksum
        const tCIDLib::TCard4 c4Sum = strmOut.mbufData().c4CheckSum
        (
            0
            , strmOut.c4CurPos()
        );
        strmOut << c4Sum << kCIDLib::FlushIt;

        //
        //  Use the temp write file to write it out to. Once we are sure its
        //  written, we'll rename it to the real file name, backing up the
        //  current file, if any.
        //
        eState = EState_CreateTmp;
        tCIDLib::TCard4 c4Written;
        {
            TBinaryFile flTarget(pathTmp);
            flTarget.Open
            (
                tCIDLib::EAccessModes::Excl_Write
                , tCIDLib::ECreateActs::CreateAlways
                , tCIDLib::EFilePerms::Default
                , tCIDLib::EFileFlags::SequentialScan
            );

            c4Written = flTarget.c4WriteBuffer
            (
                strmOut.mbufData()
                , strmOut.c4CurPos()
            );
        }

        if (c4Written != strmOut.c4CurPos())
        {
            facCIDLib().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kCIDErrs::errcFile_NotAllData
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::NotAllRead
                , facCIDLib().strMsg(kCIDMsgs::midGen_write)
                , TCardinal(strmOut.c4CurPos())
                , TCardinal(c4Written)
            );
        }


        // If it exists, then back it up, getting rid of any existing backup
        if (TFileSys::bExists(pathToWrite))
        {
            eState = EState_RemoveBak;
            if (TFileSys::bExists(pathBackup))
                TFileSys::DeleteFile(pathBackup);

            eState = EState_RenameCur;
            TFileSys::Rename(pathToWrite, pathBackup);
        }

        // And finally move the temp file to the new file
        eState = EState_RenameTmp;
        TFileSys::Rename(pathTmp, pathToWrite);

        //
        //  Now either update the old index object, or create a new one and
        //  add it to the index. If updating, it cannot change the model
        //  name since we looked it up by model name to begin with.
        //
        if (pirbdmiData)
            pirbdmiData->UpdateFrom(irbdmToWrite);
        else
            pcolKey->objPlace(pathNewFile, irbdmToWrite);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);

        //
        //  If it died while trying to rename the temp file, then see if
        //  we created a backup and if so try to restore it.
        //
        if (eState == EState_RenameTmp)
        {
            if (TFileSys::bExists(pathBackup))
                TFileSys::Rename(pathBackup, pathToWrite);
        }
        throw;
    }

    if (facCQCDataSrv.bLogInfo())
    {
        facCQCDataSrv.LogMsg
        (
            CID_FILE
            , CID_LINE
            , kDSrvMsgs::midStatus_Updated
            , tCIDLib::ESeverities::Info
            , tCIDLib::EErrClasses::AppStatus
            , TString(L"IR model")
            , strDevModel
        );
    }
}


// ---------------------------------------------------------------------------
//  TCQCIRSrvImpl: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCIRSrvImpl::Initialize()
{
    // Nothing to do at this time, just pass to our parent
    TParent::Initialize();
}


tCIDLib::TVoid TCQCIRSrvImpl::Terminate()
{
    // Nothing to do at this time, just pass to our parent
    TParent::Terminate();
}


// ---------------------------------------------------------------------------
//  TCQCIRSrvImpl: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TCQCIRSrvImpl::LoadKey(         TBlasterDevList&    colToFill
                        , const TString&            strBlasterName)
{
    // If in debug mode, make sure its empty
    CIDAssert
    (
        colToFill.bIsEmpty()
        , L"The key collection should be empty when LoadKey() is called"
    )

    // Build the path to the files for the particular blaster
    TPathStr pathDir(m_strRepoDir);
    pathDir.AddLevel(strBlasterName);
    pathDir.AddLevel(L"*");
    pathDir.AppendExt(CQCDataSrv_IRServerImpl::pszFileExt);

    // Create a directory iterator and run through the files
    TDirIter    diterList;
    TFindBuf    fndbInfo;
    if (diterList.bFindFirst(pathDir, fndbInfo))
    {
        tCIDLib::TCard4         c4RepeatCount;
        tCQCKit::EDevCats       eCategory;
        TIRBlasterDevModel      irbdmTmp;
        TPathStr                pathBase;
        TString                 strDescr;
        TString                 strDevModel;
        TString                 strDevMake;

        TPathStr pathTmp;
        do
        {
            //
            //  Create an input stream for this one. And ask the temp IR
            //  device object to stream in the header info.
            //
            try
            {
                TBinFileInStream strmTmp
                (
                    fndbInfo.pathFileName()
                    , tCIDLib::ECreateActs::OpenIfExists
                    , tCIDLib::EFilePerms::Default
                    , tCIDLib::EFileFlags::SequentialScan
                );

                TIRBlasterDevModel::StreamHeader
                (
                    strmTmp
                    , c4RepeatCount
                    , strDevModel
                    , strDevMake
                    , strDescr
                    , eCategory
                );

                //
                //  Pull the base part of the name out, since that's all that
                //  we store the in the key objects.
                //
                pathBase = fndbInfo.pathFileName();
                pathBase.bRemovePath();
                pathBase.bRemoveExt();

                //
                //  Create a dev info object for this one and add it to the
                //  key.
                //
                colToFill.objPlace
                (
                    strDevModel, strDevMake, strDescr, pathBase, eCategory
                );

                // And close this stream in preperation for the next round
                strmTmp.Close();
            }

            catch(const TError& errToCatch)
            {
                if (facCQCDataSrv.bShouldLog(errToCatch))
                    TModule::LogEventObj(errToCatch);

                if (facCQCDataSrv.bLogFailures())
                {
                    facCQCDataSrv.LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , kDSrvErrs::errcIR_KeyLoadFailed
                        , tCIDLib::ESeverities::Failed
                        , tCIDLib::EErrClasses::CantDo
                        , fndbInfo.pathFileName()
                    );
                }
            }

            catch(...)
            {
                if (facCQCDataSrv.bLogFailures())
                {
                    facCQCDataSrv.LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , kDSrvErrs::errcIR_KeyLoadFailed
                        , tCIDLib::ESeverities::Failed
                        , tCIDLib::EErrClasses::CantDo
                        , fndbInfo.pathFileName()
                    );
                }
            }
        }   while (diterList.bFindNext(fndbInfo));
    }
}


//
//  The public methods above call this method to find the right key list for
//  a particular blaster type name.
//
TCQCIRSrvImpl::TBlasterDevList*
TCQCIRSrvImpl::pcolFindKey(const TString& strDevName)
{
    TCQCIRSrvImpl::TBlasterDevList* pcolRet = nullptr;
    TKeyList::TPair* pkvalFind = m_colKey.pkobjFindByKey(strDevName);
    if (pkvalFind)
    {
        pcolRet = &pkvalFind->objValue();
    }
     else
    {
        TKeyList::TPair& kvalNew = m_colKey.kobjAdd
        (
            strDevName
            , TBlasterDevList(29, TStringKeyOps(), &TIRBlasterDevModelInfo::strKey)
        );
        LoadKey(kvalNew.objValue(), strDevName);

        pcolRet = &kvalNew.objValue();
    }
    return pcolRet;
}


