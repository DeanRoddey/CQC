//
// FILE NAME: ZWaveLevi2S_DriverImpl2.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/10/2014
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
//  This file handles some grunt work overflow from the main driver impl
//  file. This file handles stuff related to the higher level activities
//  of interacting with the controller, read/write config, enumerating available
//  devices and so forth.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "ZWaveLevi2S_.hpp"




// ---------------------------------------------------------------------------
//  TZWaveLevi2SDriver: Private, non-virtual methods
// ---------------------------------------------------------------------------


// Reads the config in from the config file
tCIDLib::TBoolean TZWaveLevi2SDriver::bReadCfgFile()
{
    // If no file, then just return true now, to start with defaults
    if (!TFileSys::bExists(m_strCfgFile, tCIDLib::EDirSearchFlags::NormalFiles))
    {
        m_dcfgCurrent.Reset();
        return kCIDLib::True;
    }

    try
    {
        TBinFileInStream strmSrc
        (
            m_strCfgFile
            , tCIDLib::ECreateActs::OpenIfExists
            , tCIDLib::EFilePerms::Default
            , tCIDLib::EFileFlags::SequentialScan
        );
        strmSrc >> m_dcfgCurrent;
    }

    catch(TError& errToCatch)
    {
        if (!errToCatch.bLogged()
        &&  (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        // It was there but something is awry, return false
        return kCIDLib::False;
    }
    return kCIDLib::True;
}



//
//  This method is called upon connect. We ask the controller for all of the
//  available devices. We reconcile this with our stored configuration.
//
//  The iteration mechanism is really stupid. We have to go through every unit it
//  and ask if there's a unit of every possible type there that we want to support.
//  So we have to do the loop again and again.
//
//  Note that the calling connect method disables processing unexpected msgs during
//  this process, because we aren't ready to deal with unit status info yet and the
//  config info we are working on is usually a temp one, not the real thing.
//
//  Because there would otherwise be a bodacious amount of replicated code, due to
//  the stupid iteration scheme, we have a helper that we pass the Z-Wave class to
//  and it does the work for a given type. We just call it for each type we want to
//  support.
//
//  At the end of this, the units are in one of these states:
//
//      Missing - It existed before but wasn't found in this network scan.
//      WaitCfg - It's waiting to have the device info set
//
//  We've not tried to load the device info on them yet. That will be done back
//  in the caller which has to copy the config data back to the final target
//  config object before doing that. He typically will pass us a temp object to
//  make sure it all works before overwriting the real config.
//
tCIDLib::TVoid
TZWaveLevi2SDriver::EnumerateDevType(const  tCIDLib::TCh* const         pszType
                                    , const tZWaveLevi2Sh::EGenTypes    eGType
                                    ,       tZWaveLevi2Sh::TGTypeArray& fcolGTypes)
{
    // Pre-build the unchanging part of the query msg
    TString strQMsg= L">?FI0,";
    strQMsg.AppendFormatted(m_fcolGTypeMap[eGType]);
    strQMsg.Append(L",0,");

    const tCIDLib::TCard4   c4FixedLen = strQMsg.c4Length();
    TThread*                pthrCaller = TThread::pthrCaller();
    tCIDLib::TCardList      fcolVals(64);

    //
    //  None of these messages are node specific, we are talking to the VRCOP, so don't
    //  pass in the id we are indexing through as the node to get a response from when
    //  waiting on responses below.
    //
    for (tCIDLib::TCard4 c4ID = 1; c4ID <= kZWaveLevi2Sh::c4MaxUnitId; c4ID++)
    {
        // Watch for a shutdown request
        if (pthrCaller->bCheckShutdownRequest())
        {
            facZWaveLevi2S().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kZW2Errs::errcCfg_Interrupted
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Shutdown
            );
        }

        // Clear the current group and wait for the ack, but not trans ack
        SendVRCOPMsg(L">N", kCIDLib::False);
        if (!pthrCaller->bSleep(150))
            break;

        // Add the per-round value to the fixed part and send it
        strQMsg.CapAt(c4FixedLen);
        strQMsg.AppendFormatted(c4ID);

        // Send and again wait for just the ack, not the trans ack
        SendVRCOPMsg(strQMsg, kCIDLib::False);

        // Wait for a node id reply, or error
        if (!bWaitMsg(tZWaveLevi2S::ERepType_UnitID, 0, 0, fcolVals, kCIDLib::False))
        {
            LogMsg
            (
                L"No response enumerating type=%(2)"
                , tCQCKit::EVerboseLvls::Medium
                , CID_FILE
                , CID_LINE
                , TCardinal(c4ID)
                , TString(pszType)
            );
            break;
        }

        if (fcolVals.c4ElemCount() != 1)
        {
            LogMsg
            (
                L"Unit id reply had %(1) values, instead of 1"
                , tCQCKit::EVerboseLvls::Medium
                , CID_FILE
                , CID_LINE
                , TCardinal(fcolVals.c4ElemCount())
            );
            continue;
        }

        if (fcolVals[0])
        {
            // Watch for a reuse of an id, or an ID that's invalid
            if (m_bsNodeMap.bBitState(fcolVals[0] - 1))
            {
                LogMsg
                (
                    L"More than one unit reported for ID %(1). Dup type=%(2)"
                    , tCQCKit::EVerboseLvls::Medium
                    , CID_FILE
                    , CID_LINE
                    , TCardinal(c4ID)
                    , TString(pszType)
                );
            }
             else if (fcolVals[0] > kZWaveLevi2Sh::c4MaxUnitId)
            {
                LogMsg
                (
                    L"Invalid unit ID received (%(1)). Type=%(2) "
                    , tCQCKit::EVerboseLvls::Medium
                    , CID_FILE
                    , CID_LINE
                    , TCardinal(c4ID)
                    , TString(pszType)
                );
            }
             else
            {
                //
                //  Mark this one gotten, and store it's type in the types
                //  list.
                //
                m_bsNodeMap.bSetBitState(fcolVals[0] - 1, kCIDLib::True);
                fcolGTypes[fcolVals[0] - 1] = eGType;
            }
        }
         else
        {
            // We are done. No more of this type
            break;
        }
    }
}

tCIDLib::TVoid TZWaveLevi2SDriver::EnumerateDevices(TZWaveDrvConfig& dcfgUpdate)
{
    if (m_bEnableTrace)
    {
        m_strmTrace << L"------------------------\n"
                    << L"Starting Network Scan\n"
                    << L"------------------------" << kCIDLib::EndLn;
    }

    // Clear the node map out to start
    m_bsNodeMap.Clear();

    // We will build up a list of generic type for each unit id found
    tZWaveLevi2Sh::TGTypeArray fcolTypes(kZWaveLevi2Sh::c4MaxUnitId);
    fcolTypes.SetAll(tZWaveLevi2Sh::EGenTypes::None);

    // Enumerate the various types we support
    EnumerateDevType(L"Generic Ctrls", tZWaveLevi2Sh::EGenTypes::GenCtrl, fcolTypes);
    EnumerateDevType(L"Multi-Level Switch", tZWaveLevi2Sh::EGenTypes::MLSwitch, fcolTypes);
    EnumerateDevType(L"Binary Switch", tZWaveLevi2Sh::EGenTypes::BinSwitch, fcolTypes);
    EnumerateDevType(L"Binary Sensor", tZWaveLevi2Sh::EGenTypes::BinSensor, fcolTypes);
    EnumerateDevType(L"Entry Control", tZWaveLevi2Sh::EGenTypes::EntryCtrl, fcolTypes);
    EnumerateDevType(L"Multi-Level Sensor", tZWaveLevi2Sh::EGenTypes::MLSensor, fcolTypes);
    EnumerateDevType(L"Thermostats", tZWaveLevi2Sh::EGenTypes::Thermo, fcolTypes);
    EnumerateDevType(L"Static Ctrls", tZWaveLevi2Sh::EGenTypes::StaticCtrl, fcolTypes);
    EnumerateDevType(L"Not Sensors", tZWaveLevi2Sh::EGenTypes::NotSensor, fcolTypes);

    if (m_bEnableTrace)
    {
        m_strmTrace << L"\n------------------------\n"
                    << L"Network Scan Complete\n"
                    << L"------------------------" << kCIDLib::NewEndLn;
    }

    //
    //  And now loop through the ones we got and see if they are new ones (not previously
    //  seen and in our config) or if they are existing ones. If existing, make sure
    //  that they are reporting the same generic type, else reset the unit to the new
    //  type.
    //
    tCIDLib::TCard4 c4NewCnt = 0;
    tCIDLib::TCard4 c4RemovedCnt = 0;
    for (tCIDLib::TCard4 c4ID = 1; c4ID <= kZWaveLevi2Sh::c4MaxUnitId; c4ID++)
    {
        //
        //  See if we found something at this id in the scan. If not, then
        //  move on to the next one.
        //
        if (!m_bsNodeMap.bBitState(c4ID - 1))
            continue;

        // See if we have a currently configured unit at this id
        TZWaveUnit* punitCur = dcfgUpdate.punitFindById(c4ID);

        if (punitCur)
        {
            if (punitCur->eGenType() != fcolTypes[c4ID - 1])
            {
                if (m_bEnableTrace)
                {
                    m_strmTrace << L"Unit '" << punitCur->strName()
                                << L"/" << punitCur->c4Id()
                                << L"' no longer matches it's actual unit type ("
                                << tZWaveLevi2Sh::strXlatEGenTypes(punitCur->eGenType())
                                << L"). It is being replaced" << kCIDLib::EndLn;
                }

                c4RemovedCnt++;
                dcfgUpdate.ResetUnitType(c4ID, fcolTypes[c4ID - 1]);
            }
             else
            {
                if (m_bEnableTrace)
                {
                    m_strmTrace << L"Unit '" << punitCur->strName()
                                << L"/" << punitCur->c4Id()
                                << L"' is unchanged. Type="
                                << tZWaveLevi2Sh::strXlatEGenTypes(punitCur->eGenType())
                                << kCIDLib::EndLn;
                }
            }
        }
         else
        {
            if (dcfgUpdate.bAddUnit(c4ID, fcolTypes[c4ID - 1]))
            {
                c4NewCnt++;

                if (m_bEnableTrace)
                {
                    m_strmTrace << L"New unit discovered. Id=" << c4ID
                                << L", Type="
                                << tZWaveLevi2Sh::strXlatEGenTypes(fcolTypes[c4ID - 1])
                                << kCIDLib::EndLn;
                }
            }
        }
    }

    if (m_bEnableTrace)
        m_strmTrace << kCIDLib::NewEndLn;

    //
    //  Let's see if any are missing, meaning that they were in the original config
    //  but not found during the scan. This time we go the opposite way and iterate
    //  the configured units.
    //
    const tCIDLib::TCard4 c4UnitCnt = dcfgUpdate.c4UnitCnt();
    tCIDLib::TCard4 c4MissingCnt = 0;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4UnitCnt; c4Index++)
    {
        TZWaveUnit& unitCur = dcfgUpdate.unitAt(c4Index);
        if (!m_bsNodeMap.bBitState(unitCur.c4Id() - 1))
        {
            unitCur.bMissing(kCIDLib::True);
            c4MissingCnt++;
        }
         else
        {
            unitCur.bMissing(kCIDLib::False);
        }
    }

    //
    //  OK, we are done with the enumeration. Log a message about how many
    //  added, removed or are missing.
    //
    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
    {
        LogMsg
        (
            L"Unit scan complete. New=%(1), Removed=%(2), Missing=%(3)"
            , tCQCKit::EVerboseLvls::Medium
            , CID_FILE
            , CID_LINE
            , TCardinal(c4NewCnt)
            , TCardinal(c4RemovedCnt)
            , TCardinal(c4MissingCnt)
        );
    }
}


//
//  This is called at the end of the connect phase. We try to go through and get all
//  of the field data of the defined units. We only do this for those that are in or
//  beyond Init state and that don't implement Wakeup. If < Init, then it's failed for
//  some reason or is missing, so no point. If a Wakeup type, it's not going to be
//  listening, so we'll just process the data when we get a wakeup (If it's not happened
//  already via async incoming data, which may come before the wakeup if it is sent.)
//
//  At this point, response to unsolicited messages is enabled, so while we are waiting
//  for acks from other queries, we'll be processing any incoming reports.
//
tCIDLib::TVoid TZWaveLevi2SDriver::GetInitVals()
{
    const tCIDLib::TCard4 c4UnitCnt = m_dcfgCurrent.c4UnitCnt();
    TThread* pthrThis = TThread::pthrCaller();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4UnitCnt; c4Index++)
    {
        TZWaveUnit& unitCur = m_dcfgCurrent.unitAt(c4Index);

        // We skip it if is non-viable. Or, if it implements wakeup.
        if (!unitCur.bIsViable()
        ||  unitCur.bImplementsClass(tZWaveLevi2Sh::ECClasses::WakeUp))
        {
            continue;
        }

        if (!unitCur.bQueryFldVals(*this))
        {
            LogMsg
            (
                L"Unit %(1) (%(2)) failed to get some or all field data"
                , tCQCKit::EVerboseLvls::Low
                , CID_FILE
                , CID_LINE
                , unitCur.strName()
                , TCardinal(unitCur.c4Id())
            );
        }

        // Process async messages a bit
        EatMsgs(100, kCIDLib::False);
    }

    //
    //  Just process messages for a bit now, to pick up more remaining incoming
    //  reports before we return. We pass in a rep type that it won't ever actually see,
    //  so it'll just wait for messages for the indicated timeout. Again, tell it not
    //  to throw errors, since we know we will timeout.
    //
    EatMsgs(1500, kCIDLib::False);
}


//
//  This method is called with a new driver configuration. We use this info
//  to register our fields. There are a set of them that we always do. But
//  the bulk of them are driven by the unit and group configuration info in
//  the passed object.
//
tCIDLib::TVoid TZWaveLevi2SDriver::RegisterFields(TZWaveDrvConfig& dcfgNew)
{
    // Build up a field list based on the new config
    TVector<TCQCFldDef> colFlds(64);
    TCQCFldDef flddNew;

    //
    //  Add some fixed fields. We need a command field to take free form
    //  commands, and some that provide read-only access to some device
    //  or network info.
    //
    flddNew.Set
    (
        kZWaveLevi2Sh::strFld_InvokeCmd
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Write
        , kCIDLib::True
    );
    colFlds.objAdd(flddNew);

    //
    //  Create the fields for all of the units that we find in the passed config,
    //  unless they are disabled or have no device info set on them. Even if they
    //  were not found during the scan, we still create the fields. They will just
    //  remain in error.
    //
    const tCIDLib::TCard4 c4UnitCnt = dcfgNew.c4UnitCnt();
    for (tCIDLib::TCard4 c4UnitInd = 0; c4UnitInd < c4UnitCnt; c4UnitInd++)
    {
        const TZWaveUnit& unitCur = dcfgNew.unitAt(c4UnitInd);

        // If it has it's make/model info and not disabled, then do it
        if (unitCur.bDevInfoLoaded() && !unitCur.bDisabled())
            unitCur.QueryFldDefs(colFlds, flddNew);
    }

    // Ok, let's set our fields and look them up
    {
        SetFields(colFlds);

        // Bump the reconfigured field
        IncReconfigCounter();

        //
        //  Go back and look up all the unit field ids and store them in their
        //  respective group/unit objects.
        //
        for (tCIDLib::TCard4 c4UnitInd = 0; c4UnitInd < c4UnitCnt; c4UnitInd++)
        {
            TZWaveUnit& unitCur = dcfgNew.unitAt(c4UnitInd);
            if (unitCur.bDevInfoLoaded() && !unitCur.bDisabled())
                unitCur.StoreFldIds(*this);
        }

        // Look up any others that we do by id
        m_c4FldId_InvokeCmd = pflddFind(kZWaveLevi2Sh::strFld_InvokeCmd, kCIDLib::True)->c4Id();
    }
}



// Writes out config out to the config file
tCIDLib::TVoid TZWaveLevi2SDriver::WriteCfgFile()
{
    // Make sure the path part exists
    TFileSys::MakePath(m_strCfgPath);

    TBinFileOutStream strmOut
    (
        m_strCfgFile
        , tCIDLib::ECreateActs::CreateAlways
        , tCIDLib::EFilePerms::Default
        , tCIDLib::EFileFlags::SequentialScan
    );
    strmOut << m_dcfgCurrent << kCIDLib::FlushIt;
}


