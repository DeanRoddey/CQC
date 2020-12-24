//
// FILE NAME: CQCHostMonS_DriverImpl.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/13/2000
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
//  This is a test driver which implements the host monitor interface.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCHostMonS.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCHostMonSDriver,TCQCServerBase)



// ---------------------------------------------------------------------------
//   CLASS: THostMonSDriver
//  PREFIX: drv
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  THostMonSDriver: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCHostMonSDriver::TCQCHostMonSDriver(const TCQCDriverObjCfg& cqcdcToLoad) :

    TCQCServerBase(cqcdcToLoad)
    , m_c4FldIdPerMemUsed(kCIDLib::c4MaxCard)
    , m_c4VolSerialNum(0)
    , m_f8PhysicalMB(0)
    , m_strFldCPUCount(kHostMonC::pszFld_CPUCount)
    , m_strFldHostName(kHostMonC::pszFld_HostName)
    , m_strFldOSVersion(kHostMonC::pszFld_OSVersion)
    , m_strFldPerMemUsed(kHostMonC::pszFld_PerMemUsed)
    , m_strFldPhysMemK(kHostMonC::pszFld_PhysicalMemK)
{
}

TCQCHostMonSDriver::~TCQCHostMonSDriver()
{
}


// ---------------------------------------------------------------------------
//  TCQCHostMonSDriver: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TCQCHostMonSDriver::bGetCommResource(TThread&)
{
    // We have no comm resource, so just say we are good
    return kCIDLib::True;
}


tCIDLib::TBoolean TCQCHostMonSDriver::bWaitConfig(TThread& thrThis)
{
    // We have no config, so just say we are good
    return kCIDLib::True;
}


tCQCKit::ECommResults TCQCHostMonSDriver::eConnectToDevice(TThread&)
{
    //
    //  We are always 'connected', because we don't have a real device. Since
    //  we set the online state during our init, and never go offline, this
    //  method will never be called, but we have to make the compiler happy,
    //  so just return success.
    //
    return tCQCKit::ECommResults::Success;
}


tCQCKit::EDrvInitRes TCQCHostMonSDriver::eInitializeImpl()
{
    // Set up a collection of field defs to set
    TVector<TCQCFldDef> colFlds(32);
    LoadFields(colFlds);

    // Tell our base class about our fields
    SetFields(colFlds);

    // Call a private helper to set the initial field values
    InitFields();

    //
    //  Indicate that we want our initial state to be waiting for a comm
    //  resource. We don't really have one, and our method will just return
    //  true, but this is far as we can move us forward at this point.
    //
    return tCQCKit::EDrvInitRes::WaitCommRes;
}


tCQCKit::ECommResults TCQCHostMonSDriver::ePollDevice(TThread&)
{
    // Check to see if the volume list has changed
    const tCIDLib::TCard4 c4CurSerNum = TKrnlFileSys::c4VolListSerialNum();
    if (c4CurSerNum != m_c4VolSerialNum)
    {
        m_c4VolSerialNum = c4CurSerNum;

        // Set up a collection of field defs to set
        TVector<TCQCFldDef> colFlds(32);
        LoadFields(colFlds);

        // Tell our base class about our fields
        SetFields(colFlds);

        // Call a private helper to set the initial field values
        InitFields();
    }
     else
    {
        //
        //  No changes, so just do the usual poll. First refresh our
        //  volume list fields, telling it not to force updates, just
        //  update the ones that have changed.
        //
        RefreshVolumes(kCIDLib::False);

        //
        //  Calc the percent memory used. Watch for a possible divide by
        //  zero, which would happen if all memory is used.
        //
        //  NOTE:   The wierd round about math here is because there is no
        //          conversion from either 8 byte types to double without
        //          either a warning or error. So we have to get the
        //          resulting value down to something that can live in a
        //          TCard4 first.
        //
        tCIDLib::TCard8 c8TmpAvail = TSysInfo::c8AvailPhysicalMem();
        tCIDLib::TCard4 c4TmpUsedPercent;
        if (!c8TmpAvail)
        {
            c4TmpUsedPercent = 100;
        }
         else
        {
            // Convert the available to MBs
            const tCIDLib::TFloat8 f8TmpAvail = tCIDLib::TCard4
            (
                c8TmpAvail / 0x100000
            );

            //
            //  Now calc the percent used. We have to subtract it from 1.0 because
            //  what we are really getting is the percent avail, so we have to
            //  flip it the other way.
            //
            c4TmpUsedPercent = tCIDLib::TCard4
            (
                (1.0 - (f8TmpAvail / m_f8PhysicalMB)) * 100.0
            );
        }

        // If the memory value changed, then store it and remember we changed
        bStoreCardFld(m_c4FldIdPerMemUsed, c4TmpUsedPercent, kCIDLib::True);
    }

    //
    //  We will always be successful, since we have no real device that can
    //  go offline.
    //
    return tCQCKit::ECommResults::Success;
}


tCIDLib::TVoid TCQCHostMonSDriver::ReleaseCommResource()
{
    // Nothing to do for this one
}


tCIDLib::TVoid TCQCHostMonSDriver::TerminateImpl()
{
    // Nothing to do for this one
}


// ---------------------------------------------------------------------------
//  TCQCHostMonSDriver: Private, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid TCQCHostMonSDriver::InitFields()
{
    //
    //  Go back and loop up the fields for the volumes and store the field
    //  id in the volume info object so that we can quickly get bakc to the
    //  field at runtime.
    //
    const tCIDLib::TCard4 c4Count = m_colVolInfo.c4ElemCount();
    TString strName;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TVolInfo& voliCur = m_colVolInfo[c4Index];

        strName = L"Volume";
        strName.AppendFormatted(c4Index + 1);
        voliCur.m_c4FldId = pflddFind(strName, kCIDLib::True)->c4Id();
    }

    // Do an initial refresh of the volume fields, forcing a refresh this time
    RefreshVolumes(kCIDLib::True);

    //
    //  And look up any other field ids for fields we have to keep updated.
    //  The others will never change so we'll just set them below and be
    //  done with it.
    //
    m_c4FldIdPerMemUsed = pflddFind(m_strFldPerMemUsed, kCIDLib::True)->c4Id();

    //
    //  Store the total physical memory for later use. we store it as mega
    //  bytes, since that's how it's used in the calculations. We also make
    //  this available in K as a field, but since we need it in this form
    //  for the calculation we do, we just store it separately as a member.
    //
    m_f8PhysicalMB = tCIDLib::TFloat8(TSysInfo::c8TotalPhysicalMem() / 0x100000);

    //
    //  Format out the OS version info. Its kind of complex so we'll use a
    //  temp string stream to do it.
    //
    TTextStringOutStream strmTmp(128);
    tCIDLib::TCard4 c4OSMajVersion;
    tCIDLib::TCard4 c4OSMinVersion;
    tCIDLib::TCard4 c4OSRev;
    tCIDLib::TCard4 c4OSBuildNum;
    TSysInfo::QueryOSInfo(c4OSMajVersion, c4OSMinVersion, c4OSRev, c4OSBuildNum);
    strmTmp << c4OSMajVersion << L"." << c4OSMinVersion << L"."
            << c4OSRev;
    if (c4OSBuildNum)
        strmTmp << L" [Build:" << c4OSBuildNum << L"]" << kCIDLib::FlushIt;

    //
    //  Write to some fields now, since this stuff is never going change while
    //  we are running.
    //
    bStoreStringFld(m_strFldOSVersion, strmTmp.strData(), kCIDLib::True);
    bStoreStringFld(m_strFldHostName, TSysInfo::strIPHostName(), kCIDLib::True);
    bStoreCardFld
    (
        m_strFldPhysMemK
        , tCIDLib::TCard4(TSysInfo::c8TotalPhysicalMem() / 1024)
        , kCIDLib::True
    );
    bStoreCardFld(m_strFldCPUCount, TSysInfo::c4CPUCount(), kCIDLib::True);
}


//
//  Loads up and registers all of the fields that we need, into the passed
//  collection.
//
tCIDLib::TVoid TCQCHostMonSDriver::LoadFields(TVector<TCQCFldDef>& colToFill)
{
    // Do a fixed set of fields first
    colToFill.objAdd
    (
        TCQCFldDef
        (
            m_strFldOSVersion
            , tCQCKit::EFldTypes::String
            , tCQCKit::EFldAccess::Read
        )
    );

    colToFill.objAdd
    (
        TCQCFldDef
        (
            m_strFldHostName
            , tCQCKit::EFldTypes::String
            , tCQCKit::EFldAccess::Read
        )
    );

    colToFill.objAdd
    (
        TCQCFldDef
        (
            m_strFldPhysMemK
            , tCQCKit::EFldTypes::Card
            , tCQCKit::EFldAccess::Read
        )
    );

    colToFill.objAdd
    (
        TCQCFldDef
        (
            m_strFldPerMemUsed
            , tCQCKit::EFldTypes::Card
            , tCQCKit::EFldAccess::Read
        )
    );

    colToFill.objAdd
    (
        TCQCFldDef
        (
            m_strFldCPUCount
            , tCQCKit::EFldTypes::Card
            , tCQCKit::EFldAccess::Read
        )
    );

    // Store the current volume serial number
    m_c4VolSerialNum = TKrnlFileSys::c4VolListSerialNum();

    // Load up a field for each non-removeable volume
    TVector<TVolumeInfo>        colVols;
    TVector<TVolFailureInfo>    colFailures;
    const tCIDLib::TCard4 c4Count = TFileSys::c4QueryAvailableVolumes
    (
        colVols, colFailures, kCIDLib::False, kCIDLib::False
    );

    //
    //  Loop through and find the ones that didn't fail and which are fixed
    //  type volumes (i.e. we don't report on any removeables.)
    //
    m_colVolInfo.RemoveAll();
    TString strName;
    tCIDLib::TCard4 c4VolIndex = 1;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TVolumeInfo& voliCur = colVols[c4Index];

        // Skip if its not a fixed volume
        if (!(voliCur.eHWType() == tCIDLib::EVolHWTypes::Fixed))
            continue;

        // Get the capacity info for this volume
        tCIDLib::TCard8 c8Total;
        tCIDLib::TCard8 c8Free;
        tCIDLib::TCard8 c8User;
        TFileSys::QueryVolumeInfo(voliCur.strVolume(), c8Total, c8Free, c8User);

        //
        //  Looks like a keeper, so put in our list. The field id field will
        //  be set to an invalid value until we can update it after we register
        //  the fields.
        //
        m_colVolInfo.objAdd
        (
            TVolInfo
            (
                voliCur.strVolume()
                , tCIDLib::TCard4(c8Total / 0x100000)
                , tCIDLib::TCard4(c8Free / 0x100000)
            )
        );

        //
        //  Add a field for each volume. We just name them Volume1, Volume2,
        //  and so forth. The actual volume name is formatted into the
        //  string value.
        //
        strName = L"Volume";
        strName.AppendFormatted(c4VolIndex++);
        colToFill.objAdd
        (
            TCQCFldDef
            (
                strName
                , tCQCKit::EFldTypes::String
                , tCQCKit::EFldAccess::Read
            )
        );
    }
}


//
//  This guy runs through the volues and update their formatted values.
//  If we get any errors, we set the field for that volume into error
//  state. In theory, this should never happen, since we check for a
//  volume list change in the poll an update the list when that happens.
//  But, never say never.
//
tCIDLib::TVoid
TCQCHostMonSDriver::RefreshVolumes(const tCIDLib::TBoolean bForce)
{
    tCIDLib::TCard8 c8Total;
    tCIDLib::TCard8 c8Free;
    tCIDLib::TCard8 c8User;

    const tCIDLib::TCard4 c4Count = m_colVolInfo.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TVolInfo& voliCur = m_colVolInfo[c4Index];

        // Query info for this volume into temps
        try
        {
            TFileSys::QueryVolumeInfo(voliCur.m_strVolume, c8Total, c8Free, c8User);

            // If this one has changed, we need to update
            const tCIDLib::TCard4 c4FreeMB = tCIDLib::TCard4(c8Free /  0x100000);
            if ((c4FreeMB != voliCur.m_c4FreeMB) || bForce)
            {
                // It has to store new data in our local data
                voliCur.m_c4FreeMB = c4FreeMB;

                // And format a new field value and store it
                m_strTmpFmt = voliCur.m_strVolume;
                m_strTmpFmt.Append(kCIDLib::chComma);
                m_strTmpFmt.AppendFormatted(voliCur.m_c4TotalMB);
                m_strTmpFmt.Append(kCIDLib::chComma);
                m_strTmpFmt.AppendFormatted(c4FreeMB);

                bStoreStringFld(voliCur.m_c4FldId, m_strTmpFmt, kCIDLib::True);
            }
        }

        catch(const TError& errToCatch)
        {
            LogError(errToCatch, tCQCKit::EVerboseLvls::Medium);

            // Set the field into error state
            SetFieldErr(voliCur.m_c4FldId, kCIDLib::True);
        }
    }
}


