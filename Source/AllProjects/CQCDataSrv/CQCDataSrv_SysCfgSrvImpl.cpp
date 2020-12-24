//
// FILE NAME: CQCDataSrv_SysCfgSrvImpl.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/29/2002
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
RTTIDecls(TCQCSysCfgSrvImpl, TCQCSysCfgServerBase)


// ---------------------------------------------------------------------------
//   CLASS: TCQCSysCfgSrvImpl
//  PREFIX: orbs
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCSysCfgSrvImpl: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCSysCfgSrvImpl::TCQCSysCfgSrvImpl()
{
}

TCQCSysCfgSrvImpl::~TCQCSysCfgSrvImpl()
{
}


// ---------------------------------------------------------------------------
//  TCQCSysCfgSrvImpl: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  If their serial number is not the same as ours, we return the data and return true
//  and the new serial number. Else we just return false and no data is streamed back
//  since this is a poll type method.
//
tCIDLib::TBoolean
TCQCSysCfgSrvImpl::bQueryRoomCfg(tCIDLib::TCard4& c4SerialNum, TCQCSysCfg& scfgToFill)
{
    // We have to lock while doing this
    TLocker lockrSync(&m_mtxSync);

    if (m_scfgData.c4SerialNum() == c4SerialNum)
        return kCIDLib::False;

    c4SerialNum = m_scfgData.c4SerialNum();
    scfgToFill = m_scfgData;

    return kCIDLib::True;
}


//
//  We create the directory if it doesn't exist, and write the data out. We update the
//  serial number to be one more than the current one.
//
tCIDLib::TVoid
TCQCSysCfgSrvImpl::StoreRoomCfg(const   TCQCSysCfg&         scfgToStore
                                ,       tCIDLib::TCard4&    c4NewSerialNum
                                , const TCQCSecToken&       sectUser)
{
    // We have to lock while doing this
    TLocker lockrSync(&m_mtxSync);

    //
    //  If the data didn't actually change do nothing. The serial number is not included
    //  in the comparison.
    //
    if (m_scfgData == scfgToStore)
    {
        // Just give back the current serial number
        c4NewSerialNum = m_scfgData.c4SerialNum();
    }
     else
    {
        // We give back the new serial number, which is the current plus 1
        c4NewSerialNum = m_scfgData.c4SerialNum() + 1;

        // Now copy the new data and put our new serial number back in
        m_scfgData = scfgToStore;
        m_scfgData.c4SerialNum(c4NewSerialNum);

        // Let's create the directory if not there
        TPathStr pathCfg = facCQCKit().strDataDir();
        if (!TFileSys::bExists(pathCfg, L"SysCfg", tCIDLib::EDirSearchFlags::NormalDirs))
            TFileSys::MakeSubDirectory(L"SysCfg", pathCfg);

        // Build up the full paths to the temp and real files
        pathCfg.AddLevel(L"SysCfg");
        TPathStr pathTmp = pathCfg;
        pathTmp.AddLevel(L"RoomCfg.TmpFile");
        pathCfg.AddLevel(L"RoomCfg.CQCRmCfg2");

        //
        //  Write the data to the temp file. Use a scope to make sure the stream
        //  is closed when we are done so that we can rename it below.
        //
        {
            TBinFileOutStream strmTmp
            (
                pathTmp
                , tCIDLib::ECreateActs::CreateAlways
                , tCIDLib::EFilePerms::Default
                , tCIDLib::EFileFlags::SequentialScan
                , tCIDLib::EAccessModes::Excl_Write
            );

            strmTmp << m_scfgData;
            strmTmp.Flush();
        }

        //
        //  Ok, the temp file is out there. If there is a current file with
        //  the target name, we need to rename it to a backup name.
        //
        if (TFileSys::bExists(pathCfg))
        {
            // Build up the back up file name
            TPathStr pathBak(pathCfg);
            pathBak.Append(L"_Bak");

            // If that exists, then delete it
            if (TFileSys::bExists(pathBak))
                TFileSys::DeleteFile(pathBak);

            // Now rename the original to the backup
            TFileSys::Rename(pathCfg, pathBak);
        }

        // Now we can rename the temp file to the target file
        TFileSys::Rename(pathTmp, pathCfg);

        // Log the fact that we stored the new file
        if (facCQCDataSrv.bLogInfo())
        {
            facCQCDataSrv.LogMsg
            (
                CID_FILE
                , CID_LINE
                , kDSrvMsgs::midStatus_RmCfgStored
                , tCIDLib::ESeverities::Info
                , tCIDLib::EErrClasses::AppStatus
            );
        }
    }
}


// ---------------------------------------------------------------------------
//  TCQCSysCfgSrvImpl: Protected, inherited methods
// ---------------------------------------------------------------------------

//
//  Let's try to do an initial load of the data. Don't need to lock here since we are
//  not exposed yet.
//
tCIDLib::TVoid TCQCSysCfgSrvImpl::Initialize()
{
    // Build up the path to the system configuration data
    TPathStr pathCfg = facCQCKit().strDataDir();
    pathCfg.AddLevel(L"SysCfg");
    pathCfg.AddLevel(L"RoomCfg.CQCRmCfg2");

    // If it exists, try to load it
    if (TFileSys::bExists(pathCfg, tCIDLib::EDirSearchFlags::NormalFiles))
    {
        try
        {
            TBinFileInStream strmSrc
            (
                pathCfg
                , tCIDLib::ECreateActs::OpenIfExists
                , tCIDLib::EFilePerms::Default
                , tCIDLib::EFileFlags::SequentialScan
                , tCIDLib::EAccessModes::Multi_Read
            );
            strmSrc >> m_scfgData;
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);

            facCQCDataSrv.LogMsg
            (
                CID_FILE
                , CID_LINE
                , kDSrvErrs::errcSCfg_CantLoad
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Init
            );
        }
    }
}


tCIDLib::TVoid TCQCSysCfgSrvImpl::Terminate()
{
}
