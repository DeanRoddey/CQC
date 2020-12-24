//
// FILE NAME: ZWaveLeviS_DriverImpl2.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/10/2012
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
//  of interacting with the controller, such as enumerating available devices
//  and so forth.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "ZWaveLeviS_.hpp"




// ---------------------------------------------------------------------------
//  TZWaveLeviSDriver: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Search the passed configuration for a controller unit that is marked as
//  the VRCOP controller.
//
tCIDLib::TCard4
TZWaveLeviSDriver::c4FindVRCOP( const   TZWaveDrvConfig&    dcfgSrc
                                , const tCIDLib::TBoolean   bThrowIfNot)
{
    const tCIDLib::TCard4 c4UnitCnt = dcfgSrc.c4UnitCnt();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4UnitCnt; c4Index++)
    {
        const TZWaveUnit& unitCur = dcfgSrc.unitAt(c4Index);

        if (unitCur.bIsA(TZWController::clsThis()))
        {
            const TZWController& unitCtrl
            (
                static_cast<const TZWController&>(unitCur)
            );

            if (unitCtrl.bIsVRCOP())
                return unitCtrl.c4Id();
        }
    }

    if (bThrowIfNot)
    {
        facZWaveLeviSh().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kZWErrs::errcCfg_NoVRCOP
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
        );
    }

    // Return zero (an invalid id) since we never found it
    return 0;
}



//
//  This method is called upon connect. We ask the controller for all of the
//  available devices. We reconcile this with our stored configuration.
//
//  The iteration mechanism is really stupid. We have to go through every
//  unit id and ask if there's a unit of every possible type there that we
//  want to support. So we have to do the loop again and again.
//
//  Note that the calling connect method disables processing unexpected msgs
//  during this process, because we aren't ready to deal with unit status
//  info yet.
//
//  Because there would otherwise be a bodacious amount of replicated code,
//  due to the stupid iteration scheme, we have a helper that we pass the
//  Z-Wave class to and it does the work for a given type. We just call it
//  for each type we want to support.
//
tCIDLib::TVoid
TZWaveLeviSDriver::EnumeratedDevType(const  tCIDLib::TCh* const         pszType
                                    , const tCIDLib::TCard4             c4GenId
                                    ,       tZWaveLeviSh::TValArray&    fcolTypes)
{
    // Pre-build the unchanging part of the query msg
    TString strQMsg= L"?FI0,";
    strQMsg.AppendFormatted(c4GenId);
    strQMsg.Append(L",0,");
    const tCIDLib::TCard4 c4FixedLen = strQMsg.c4Length();

    tCIDLib::TCard4 c4GotID;
    tCIDLib::TCard4 c4ResCode;
    TString strRep;
    TThread* pthrCaller = TThread::pthrCaller();
    for (tCIDLib::TCard4 c4ID = 1; c4ID <= kZWaveLeviSh::c4MaxUnitId; c4ID++)
    {
        // Watch for a shutdown request
        if (pthrCaller->bCheckShutdownRequest())
        {
            facZWaveLeviS().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kZWErrs::errcCfg_Interrupted
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Shutdown
            );
        }

        // Clear the current group
        bSendAndWaitAck(L"N", 5000, c4ResCode, kCIDLib::True);
        if (!pthrCaller->bSleep(500))
            break;

        // Add the per-round value to the fixed part and send it
        strQMsg.CapAt(c4FixedLen);
        strQMsg.AppendFormatted(c4ID);

        // Send and wait for an ack, throw if not forthcoming
        bSendAndWaitAck(strQMsg, 8000, c4ResCode, kCIDLib::True);

        //
        //  Wait for a node id reply, or error. We tell the message getter
        //  to throw errors, so we only come back if we got the response.
        //
        if (!bWaitMsg(tZWaveLeviS::ERepType_UnitID, 8000, strRep, c4ResCode, kCIDLib::False))
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

        // The value returns is a unit id or zero if not found
        if (!strRep.bToCard4(c4GotID, tCIDLib::ERadices::Dec))
        {
            LogMsg
            (
                L"Invalid node id enumerating type=%(1). Got=%(2)"
                , tCQCKit::EVerboseLvls::Medium
                , CID_FILE
                , CID_LINE
                , TString(pszType)
                , strRep
            );
            break;
        }

        if (c4GotID)
        {
            // Watch for a reuse of an id, or an ID that's invalid
            if (m_bsNodeMap.bBitState(c4GotID - 1))
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
             else if (c4GotID > kZWaveLeviSh::c4MaxUnitId)
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
                m_bsNodeMap.bSetBitState(c4GotID - 1, kCIDLib::True);
                fcolTypes[c4GotID - 1] = c4GenId;
            }
        }
         else
        {
            // We are done. No more of this type
            break;
        }
    }
}

tCIDLib::TVoid
TZWaveLeviSDriver::EnumeratedDevices(TZWaveDrvConfig& dcfgUpdate)
{
    // Clear the node map out to start
    m_bsNodeMap.Clear();

    // We will build up a list of unit type for each unit id found
    tZWaveLeviSh::TValArray fcolTypes(kZWaveLeviSh::c4MaxUnitId);
    fcolTypes.SetAll(0);

    // Enumerate the various types we support
    EnumeratedDevType
    (
        L"Controllers"
        , GENERIC_TYPE_GENERIC_CONTROLLER
        , fcolTypes
    );
    EnumeratedDevType(L"Binary Sensor", GENERIC_TYPE_SENSOR_BINARY, fcolTypes);
    EnumeratedDevType(L"Binary Switch", GENERIC_TYPE_SWITCH_BINARY, fcolTypes);
    EnumeratedDevType(L"Entry Control", GENERIC_TYPE_ENTRY_CONTROL, fcolTypes);
    EnumeratedDevType(L"Multi-Level Switch", GENERIC_TYPE_SWITCH_MULTILEVEL, fcolTypes);
    EnumeratedDevType(L"Thermostats", GENERIC_TYPE_THERMOSTAT, fcolTypes);

    //
    //  Now we need to check against our existing configuration. So tell the
    //  new configuration to prep all his units for us. This will put them
    //  into missing state and zero out all field ids and otherwise get them
    //  into a 'not set up' state. If we don't find them below, they will remain
    //  in this missing state.
    //
    dcfgUpdate.PrepUnitsForUse();

    tCIDLib::TCard4 c4NewCnt = 0;
    tCIDLib::TCard4 c4RemovedCnt = 0;
    for (tCIDLib::TCard4 c4ID = 1; c4ID <= kZWaveLeviSh::c4MaxUnitId; c4ID++)
    {
        //
        //  See if we found something at this id in the scan. If not, then
        //  move on to the next one.
        //
        if (!m_bsNodeMap.bBitState(c4ID - 1))
            continue;

        // See if we have a currently configured unit at this id
        TZWaveUnit* punitCur = dcfgUpdate.punitFindById(c4ID);

        //
        //  If found, check it against the type info we found. If it's the
        //  same, then just put it into error state until we get a value,
        //  else remove it.
        //
        if (punitCur)
        {
            if (punitCur->c1ClassGen() == fcolTypes[c4ID - 1])
            {
                //
                //  Mark it as in error state since we found it, but we don't
                //  have info for it yet. Don't pass it driver ponter since
                //  this config isn't live yet.
                //
                punitCur->SetErrorStatus(0);
            }
             else
            {
                //
                //  The type has changed. So we remove this one. We'll log
                //  a message if in verbose mode.
                //
                LogMsg
                (
                    L"Unit '%(1)' no longer matches it's discovered unit type. "
                    L"It is being replaced."
                    , tCQCKit::EVerboseLvls::Low
                    , CID_FILE
                    , CID_LINE
                    , punitCur->strName()
                );
                c4RemovedCnt++;
                dcfgUpdate.DeleteUnit(punitCur);

                // Zero the pointer so we'll add a new one below
                punitCur = 0;
            }
        }

        //
        //  If not found, or removed, add it with default settings. If found,
        //  just log name, id and type info.
        //
        if (punitCur)
        {
            LogMsg
            (
                L"Unit %(1), Type=%(2), ID=%(3), unchanged from previous scan"
                , tCQCKit::EVerboseLvls::Medium
                , CID_FILE
                , CID_LINE
                , punitCur->strName()
                , punitCur->strTypeName()
                , TCardinal(punitCur->c4Id())
            );
        }
         else
        {
            TZWaveUnit* punitNew = TZWaveUnit::punitBestMatch(fcolTypes[c4ID - 1]);
            if (punitNew)
            {
                punitNew->c4Id(c4ID);
                dcfgUpdate.bAddUnit(punitNew);
                c4NewCnt++;

                //
                //  Set this one to error status since we found it but no data
                //  yet. No driver pointer since we don't want him trying to
                //  do field stuff. We'lve not stored this config yet.
                //
                punitNew->SetErrorStatus(0);

                LogMsg
                (
                    L"Unit %(1), Type=%(2), ID=%(3) not previously seen, added it."
                    , tCQCKit::EVerboseLvls::Medium
                    , CID_FILE
                    , CID_LINE
                    , punitNew->strName()
                    , punitNew->strTypeName()
                    , TCardinal(punitNew->c4Id())
                );
            }
        }
    }

    //
    //  Make one more pass now and look for any that are marked as missnig.
    //  These are ones that we have in our config, but it wasn't reported as
    //  available when we enumerated. They are already marked as missing but
    //  we want to count them for the message we log at the end.
    //
    tCIDLib::TCard4 c4MissingCnt = 0;
    const tCIDLib::TCard4 c4UnitCnt = dcfgUpdate.c4UnitCnt();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4UnitCnt; c4Index++)
    {
        TZWaveUnit& unitCur = dcfgUpdate.unitAt(c4Index);
        if (unitCur.eStatus() == tZWaveLeviSh::EUnitStatus_Missing)
            c4MissingCnt++;
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
//  This is called at the end of the connect phase. We try to go through and
//  get all of the values of the defined units.
//
//  At this point, response to unsolicited messages is enabled, so while we
//  are waiting for acks from other queries, we'll be processing any incoming
//  reports.
//
tCIDLib::TVoid TZWaveLeviSDriver::GetInitVals()
{
    const tCIDLib::TCard4 c4RoundMax = 3;

    //
    //  Let's try to be as efficient as possible and send out small groups
    //  of value queries, a few at a time as long as we can, then handle any
    //  trailing ones.
    //
    tCIDLib::TCard4     c4Index = 0;
    tCIDLib::TCard4     c4ResCode;
    TString             strQMsg;

    const tCIDLib::TCard4 c4UnitCnt = m_dcfgCurrent.c4UnitCnt();
    TThread* pthrThis = TThread::pthrCaller();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4UnitCnt; c4Index++)
    {
        TZWaveUnit& unitCur = m_dcfgCurrent.unitAt(c4Index);

        //
        //  We skip it if is non-viable, or non-readable, or it is battery
        //  powered.
        //
        if (!unitCur.bIsViable()
        ||  !unitCur.bIsReadable()
        ||  unitCur.bIsBatteryPwr())
        {
            continue;
        }

        // Do an initial poll time reset on this one
        unitCur.ResetPollTime(kCIDLib::True);

        //
        //  We have to go through all of the get message that the unit
        //  needs to get all the info.
        //
        const tCIDLib::TCard4 c4GetCnt = unitCur.c4GetMsgCnt();
        for (tCIDLib::TCard4 c4GetInd = 0; c4GetInd < c4GetCnt; c4GetInd++)
        {
            if (unitCur.bBuildGetMsg(strQMsg, c4GetInd))
            {
                if (!bSendAndWaitAck(strQMsg, 8000, c4ResCode, kCIDLib::False)
                ||  !bWaitTransAck(8000, c4ResCode, kCIDLib::False))
                {
                    LogMsg
                    (
                        L"Initial value query failed. ResCode=%(1), Unit=%(2)"
                        , tCQCKit::EVerboseLvls::High
                        , CID_FILE
                        , CID_LINE
                        , TCardinal(c4ResCode)
                        , unitCur.strName()
                    );
                    unitCur.SetErrorStatus(this);

                    // Don't bother doing any more of this guy's msgs if more than one
                    break;
                }

                // Process async messages a bit
                while (bWaitMsg(tZWaveLeviS::ERepTypes_Count, 100, c4ResCode, kCIDLib::False))
                {}
            }
        }
    }

    //
    //  Just process messages for a bit now, to pick up more remaining incoming
    //  reports before we return. We pass in a valid that it won't ever
    //  actually see, so it'll just wait for messages for the indicated
    //  timeout. Again, tell it not to throw errors, since we know we will
    //  timeout.
    //
    bWaitMsg(tZWaveLeviS::ERepTypes_Count, 2000, c4ResCode, kCIDLib::False);
}


//
//  This method is called with a new driver configuration. We use this info
//  to register our fields. There are a set of them that we always do. But
//  the bulk of them are driven by the unit and group configuration info in
//  the passed object.
//
tCIDLib::TVoid TZWaveLeviSDriver::RegisterFields(TZWaveDrvConfig& dcfgNew)
{
    // Build up a field list based on the new config
    TVector<TCQCFldDef> colFlds(64, kCQCKit::c4MaxDrvFields);
    TCQCFldDef flddNew;

    //
    //  Add some fixed fields. We need a command field to take free form
    //  commands, and some that provide read-only access to some device
    //  or network info.
    //
    flddNew.Set
    (
        kZWaveLeviSh::strFld_Command
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Write
        , kCIDLib::True
    );
    colFlds.objAdd(flddNew);

    flddNew.Set
    (
        kZWaveLeviSh::strFld_PollsDue
        , tCQCKit::EFldTypes::Card
        , tCQCKit::EFldAccess::Read
    );
    colFlds.objAdd(flddNew);


    //
    //  Create the fields for all of the units that we find in the passed
    //  configuration.
    //
    //  We update the pollable unit count while doing this.
    //
    m_c4PollableCnt = 0;
    const tCIDLib::TCard4 c4UnitCnt = dcfgNew.c4UnitCnt();
    for (tCIDLib::TCard4 c4UnitInd = 0; c4UnitInd < c4UnitCnt; c4UnitInd++)
    {
        const TZWaveUnit& unitCur = dcfgNew.unitAt(c4UnitInd);

        // Skip non-viable ones, since we don't do fields for those
        if (!unitCur.bIsViable())
            continue;

        // Allow the unit object to add its fields to the list
        unitCur.c4QueryFldDefs(colFlds, flddNew);

        // If pollable, bump the counter
        if (unitCur.bIsPollable())
            m_c4PollableCnt++;
    }

    // Ok, let's set our fields and look them up
    {
        //
        //  We may not be getting called from Connect if this is an uploaded
        //  config we are processing here.
        //
        SetFields(colFlds);

        // Bump the reconfigured field
        IncReconfigCounter();

        //
        //  Go back and look up all the unit field ids and store them in their
        //  respective group/unit objects. In order to allow the unit objects
        //  to find their fields, we have to look up all of the ids and put
        //  them into a vector. They can then go through the field list and
        //  find their fields, and the id for that field will be in the same
        //  index in the id list.
        //
        const tCIDLib::TCard4 c4FldCnt = colFlds.c4ElemCount();
        tZWaveLeviSh::TValList fcolIds(c4FldCnt);
        for (tCIDLib::TCard4 c4FInd = 0; c4FInd < c4FldCnt; c4FInd++)
        {
            const tCIDLib::TCard4 c4CurId
            (
                pflddFind(colFlds[c4FInd].strName(), kCIDLib::True)->c4Id()
            );
            fcolIds.c4AddElement(c4CurId);
        }

        for (tCIDLib::TCard4 c4UnitInd = 0; c4UnitInd < c4UnitCnt; c4UnitInd++)
        {
            TZWaveUnit& unitCur = dcfgNew.unitAt(c4UnitInd);

            // Skip non-viable ones
            if (!unitCur.bIsViable())
                continue;

            if (unitCur.bStoreFldIds(colFlds, fcolIds))
            {
                //
                //  If this is not a battery powered unit and it's readable,
                //  then mark it in error status until we get a valid value
                //  stored. If battery powered or not readable, then no value
                //  will ever be stored, so we don't want to do this.
                //
                //  This was done during the device enum phase, but we couldn't
                //  set fields then. The fields are stored now, so we can get
                //  them to error state, where they will stay if we don't get
                //  some valid data.
                //
                if (!unitCur.bIsBatteryPwr() && unitCur.bIsReadable())
                    unitCur.SetErrorStatus(this);
            }
             else
            {
                // Log this and mark it as failed
                LogMsg
                (
                    L"Unit '%(1)' did not find its field ids. Marking it failed"
                    , tCQCKit::EVerboseLvls::Medium
                    , CID_FILE
                    , CID_LINE
                    , unitCur.strName()
                );

                //
                //  Mark it failed since something has gone badly wrong. We
                //  pass in ourself, so that if any fields did get stored, they
                //  can be set to error state.
                //
                unitCur.SetFailedStatus(this);
            }
        }

        // Look up any others that we do by id
        m_c4FldId_Command = pflddFind(kZWaveLeviSh::strFld_Command, kCIDLib::True)->c4Id();
        m_c4FldId_PollsDue = pflddFind(kZWaveLeviSh::strFld_PollsDue, kCIDLib::True)->c4Id();
    }
}

