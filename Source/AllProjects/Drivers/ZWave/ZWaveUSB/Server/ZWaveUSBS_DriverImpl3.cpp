//
// FILE NAME: ZWaveUSBS_DriverImpl3.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/05/2007
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
//  This file contains some overflow grunt work methods from the driver
//  implementation class. This one mostly contains stuff related to gathering
//  information about the controller and units, managing the configuration
//  of groups and units, etc...
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
//
//  We have to bring in Z-Wave specific headers. The project setup for this
//  driver will set up the include path needed.
// ---------------------------------------------------------------------------
#include    "ZWaveUSBS_.hpp"



// ---------------------------------------------------------------------------
//  TZWaveUSBSDriver: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  This method will go out and find all of the units (as defined in the
//  passed configuration info) and ask them for their configuration info.
//
//  If the node map indicates that a unit in the passed config is no longer
//  present, then we'll mark it as offline. If the info we get is out of
//  sync with the configured unit, it will be removed.
//
//  They can tell us to update the node map first. Sometimes we are
//  called after the node map already had to be updated, and sometimes it
//  has not so we need ot do it so that we know what's out there now.
//
//  And they can tell us whether we should re-verify the class types of the
//  currently configured units. Otherwise, we'll not get protocol info on
//  the existing ones.
//
//  They can also supress discovery of new units. This is done when we start
//  up, where we just want to validate exiting configuration and nothing
//  else.
//
tCIDLib::TBoolean
TZWaveUSBSDriver::bValidateNodeCfg(         TZWaveDrvConfig&    dcfgToVal
                                    , const tCIDLib::TBoolean   bUpdateMap
                                    , const tCIDLib::TBoolean   bCheckCurTypes
                                    , const tCIDLib::TBoolean   bDiscover)
{
    // Get a buffer for input
    TSimplePoolJan<THeapBuf> janInBuf(&m_splBufs, 512);
    TMemBuf& mbufIn = janInBuf.objGet();

    // If they want the map updated, then do that
    tCIDLib::TCard4 c4Read;
    if (bUpdateMap)
    {
        c4Read = c4SendReqAndWaitForReply
        (
            FUNC_ID_SERIAL_API_GET_INIT_DATA, 500, 0, mbufIn
        );

        // It has to be at least 35 bytes
        if (c4Read < 35)
        {
            ZWLogDbgMsg1(L"Got invalid initial contoller data. Length=%(1)", TCardinal(c4Read));
            return kCIDLib::False;
        }
        m_bsNodeMap.FromRawArray(mbufIn.pc1DataAt(6), 29);
    }

    tZWaveUSBSh::TUnitList& colCurUnits = dcfgToVal.colUnits();
    tCIDLib::TCard4 c4UnitCount = colCurUnits.c4ElemCount();
    ZWLogDbgMsg1(L"%(1) units were reported available", TCardinal(c4UnitCount));

    //
    //  If we have no units and they didn't ask us to either discover new
    //  units or check current types, then there's nothing to do
    //
    if (!c4UnitCount && !bDiscover && !bCheckCurTypes)
    {
        ZWLogDbgMsg(L"Nothing reported to discover and not asked to verify existing, done...");
        return kCIDLib::True;
    }

    //
    //  Run through the configured units and check each one's id in the
    //  current known node list. If it's not there, mark it missing. If
    //  it is there, then query the unit config. If it's not the same, then
    //  remove the current unit (which will then re-discovered in the next
    //  section below.)
    //
    //  We make a copy of the node map and for each configured unit that we
    //  keep, we turn off that bit in this copy. When we are done, any bits
    //  still on in the discovered map are new ones we need to add to our
    //  config.
    //
    TThread& thrCaller = *TThread::pthrCaller();
    TBitset bsDiscovered(m_bsNodeMap);
    tCIDLib::TCard4 c4Index = 0;

    ZWLogDbgMsg(L"Existing unit verification phase starting...");
    while (c4Index < c4UnitCount)
    {
        TZWaveUnit* punitCur = colCurUnits[c4Index];
        tCIDLib::TCard1 c1CurId = punitCur->c1Id();
        ZWLogDbgMsg1(L"Verifying existing unit with id: %(1)", TCardinal(c1CurId));

        // Remove this one from the discovered list
        bsDiscovered.bSetBitState(c1CurId - 1, kCIDLib::False);

        // If it's in the node map, then update it if asked to
        if (m_bsNodeMap[c1CurId - 1])
        {
            if (bCheckCurTypes)
            {
                try
                {
                    c4SendReqAndWaitForReply
                    (
                        FUNC_ID_ZW_GET_NODE_PROTOCOL_INFO, 5000, 9, c1CurId, mbufIn
                    );

                    //
                    //  Ask the unit if these types are valid for him. If so, we
                    //  are good, else remove this unit. We will accept a generic
                    //  match here.
                    //
                    const tCIDLib::TCard1 c1BaseType = mbufIn[6];
                    const tCIDLib::TCard1 c1GenType = mbufIn[7];
                    const tCIDLib::TCard1 c1SpecType = mbufIn[8];
                    const tZWaveUSBSh::EUnitMatch eMatch = punitCur->eIsMatchingType
                    (
                        c1BaseType, c1GenType, c1SpecType
                    );

                    ZWLogDbgMsg3
                    (
                        L"   BaseType=%(1), GenType=%(2), SpecType=%(3)"
                        , TCardinal(c1BaseType, tCIDLib::ERadices::Hex)
                        , TCardinal(c1GenType, tCIDLib::ERadices::Hex)
                        , TCardinal(c1SpecType, tCIDLib::ERadices::Hex)
                    );

                    if (eMatch < tZWaveUSBSh::EUnitMatch_Generic)
                    {
                        // This one is now out of sync, so remove it
                        colCurUnits.RemoveAt(c4Index);
                        c4UnitCount--;

                        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
                        {
                            ZWLogDbgMsg1
                            (
                                L"Failed to get protocol info on unit %(1)"
                                , TCardinal(c1CurId)
                            );
                        }

                        //
                        //  Put it back on the discovered list so that we will
                        //  add it back to the list as its new self if we are
                        //  in discover mode.
                        //
                        bsDiscovered.bSetBitState(c1CurId - 1, kCIDLib::True);

                        ZWLogDbgMsg1
                        (
                            L"Unit with id %(1) has changed type"
                            , TCardinal(c1CurId)
                        );
                    }
                     else
                    {
                        // It's good so move the index up
                        c4Index++;

                        // Give it the protocol flags
                        punitCur->SetZWFlags(mbufIn[3]);
                    }
                }

                catch(const TError& errToCatch)
                {
                    // Mark this one missing since we can't get the config
                    punitCur->SetMissingState();
                    c4Index++;

                    LogError(errToCatch, tCQCKit::EVerboseLvls::Medium);
                    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
                    {
                        ZWLogDbgMsg1
                        (
                            L"Failed to get protocol info on unit %(1)"
                            , TCardinal(punitCur->c1Id())
                        );
                    }
                }
            }
             else
            {
                c4Index++;
            }
        }
         else
        {
            // It's not there anymore. Mark this one missing
            c4Index++;
            punitCur->SetMissingState();
        }

        // Watch for shutdown requests
        if (thrCaller.bCheckShutdownRequest())
        {
            if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
                ZWLogDbgMsg(L"Breaking out of cfg validation for shutdown");
            return kCIDLib::False;
        }
    }
    ZWLogDbgMsg(L"Existing unit verification phase completed");


    //
    //  If asked to discover new unit, go back and for every bit set in the
    //  discoverted bit set, that's a unit that we got from the primary but
    //  we don't have in our config, so add a default unit object for the
    //  ones we can handle.
    //
    if (bDiscover)
    {
        ZWLogDbgMsg(L"New unit discovery phase starting...");

        for (c4Index = 0; c4Index < kZWaveUSBSh::c1MaxUnits; c4Index++)
        {
            if (!bsDiscovered[c4Index])
                continue;

            ZWLogDbgMsg1
            (
                L"Checking newly discovered unit with id %(1)"
                , TCardinal(c4Index + 1)
            );

            // Get the protocol info for this guy
            try
            {
                c4SendReqAndWaitForReply
                (
                    FUNC_ID_ZW_GET_NODE_PROTOCOL_INFO
                    , 1000
                    , 9
                    , tCIDLib::TCard1(c4Index + 1)
                    , mbufIn
                );

                //
                //  Ask the unit if these types are valid for him. If so, we
                //  are good, else remove this unit. We will accept a generic
                //  match here.
                //
                const tCIDLib::TCard1 c1BaseType = mbufIn[6];
                const tCIDLib::TCard1 c1GenType = mbufIn[7];
                const tCIDLib::TCard1 c1SpecType = mbufIn[8];

                // See if we have a class that can handle this
                TZWaveUnit* punitNew = TZWaveUnit::punitBestMatch
                (
                    c1BaseType, c1GenType, c1SpecType
                );

                //
                //  If we got one, add it to our config. The default status
                //  will be 'ready' until proven otherwise.
                //
                //  Note that assigning an id will set the default unit
                //  name since no name has been assigned yet.
                //
                if (punitNew)
                {
                    punitNew->c1Id(tCIDLib::TCard1(c4Index + 1));
                    dcfgToVal.bAddUnit(punitNew);
                    ZWLogDbgMsg1
                    (
                        L"Discovered new unit with id: %(1)"
                        , TCardinal(c4Index + 1)
                    );

                    // Give it the protocol flags
                    punitNew->SetZWFlags(mbufIn[3]);
                }
                 else
                {
                    ZWLogDbgMsg1
                    (
                        L"Don't support unit with id: %(1)"
                        , TCardinal(c4Index + 1)
                    );
                }

                ZWLogDbgMsg3
                (
                    L"   BaseType=%(1), GenType=%(2), SpecType=%(3)"
                    , TCardinal(c1BaseType, tCIDLib::ERadices::Hex)
                    , TCardinal(c1GenType, tCIDLib::ERadices::Hex)
                    , TCardinal(c1SpecType, tCIDLib::ERadices::Hex)
                );
            }

            catch(const TError& errToCatch)
            {
                // Couldn't get this one for whatever reason
                LogError(errToCatch, tCQCKit::EVerboseLvls::Medium);
                if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
                {
                    ZWLogDbgMsg1
                    (
                        L"Failed to dicover protocol info on unit %(1)"
                        , TCardinal(c4Index + 1)
                    );
                }
            }
        }

        ZWLogDbgMsg(L"New unit discovery phase complete...");
    }
    return kCIDLib::True;
}


//
//  This is called when the client asks us to delete a group from the group
//  list. We get the group id.
//
tCIDLib::TVoid TZWaveUSBSDriver::DeleteGroup(const TString& strGroupId)
{
    // Ask our config guy to delete it. Let him throw if it isn't found
    m_dcfgCurrent.DeleteGroup(strGroupId);

    //
    //  Ok, let's store this data in the config server. If it adds, tell
    //  it to take 2K of extra space for this item, for futher growth
    //  without having to move it.
    //
    {
        TCfgServerClient cfgscLoad(5000);
        tCIDLib::TCard4 c4Ver;
        cfgscLoad.bAddOrUpdate(m_strCfgScope, c4Ver, m_dcfgCurrent, 2048);
    }

    // We removed it, so we have to update our fields
    RegisterFields(m_dcfgCurrent);
}


//
//  This method is started up as the replication thread. It adds our controller
//  to the network (assuming the addition via the primary controller
//  works) and then goes out and does a discovery of all nodes that the primary
//  told our controller about during the replication. We reconcile this info
//  with our existing configuration if any, to attempt to keep as much of it
//  as is still valid, and add default constructed units to our config for any
//  newly found units.
//
//  Note that we support two different types of replication. There is an old
//  style and new style. The new style just adds us to the network and then
//  we actively go out and look up all the units that our controller now
//  sees as part of his network.
//
//  The old style is passive and involves callbacks that provides info on
//  the added units and groups. We ignore this info so that, as much as
//  possible, the old and new work alike. So we let it add us to the network,
//  eat the replication info, and then at that point we do the same stuff
//  in both scenarios.
//
//
tCIDLib::EExitCodes
TZWaveUSBSDriver::eReplicationThread(TThread& thrThis, tCIDLib::TVoid*)
{
    // Set the replication related flags
    m_bRepMode = kCIDLib::True;
    m_eRepRes = tZWaveUSBSh::ERepRes_Replicating;
    m_strRepStatus.Clear();

    // And now let our caller go
    thrThis.Sync();

    //
    //  Flush the poll queue since some of the queued modules could
    //  go away after we replicate and the polling status could change
    //  on them.
    //
    m_colPollQ.RemoveAll();

    // Get buffers for read and write
    TSimplePoolJan<THeapBuf> janInBuf(&m_splBufs, 512);
    TMemBuf& mbufIn = janInBuf.objGet();
    TSimplePoolJan<THeapBuf> janOutBuf(&m_splBufs, 512);
    TMemBuf& mbufOut = janOutBuf.objGet();

    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
        ZWLogDbgMsg(L"Starting to add the controller to the network");

    try
    {
        tCIDLib::TCard4 c4Read;

        //
        //  Based on the ZWave firmware version, we will invoke an old or
        //  new style replication.
        //
        tCIDLib::TEncodedTime enctCur = TTime::enctNow();
        const tCIDLib::TEncodedTime enctEnd(enctCur + (kCIDLib::enctOneSecond * 30));

        // Get the next available callback id
        const tCIDLib::TCard1 c1CallbackId = c1NextCallbackId();

        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
            ZWLogDbgMsg(L"Doing replication. Entering receive mode...");

        // Put us into receive mode
        mbufOut.PutCard1(5, 0);
        mbufOut.PutCard1(REQUEST, 1);
        mbufOut.PutCard1(FUNC_ID_ZW_SET_LEARN_MODE, 2);
        mbufOut.PutCard1(0xFF, 3);
        mbufOut.PutCard1(c1CallbackId, 4);
        SendAndWaitForAck(mbufOut);

        while (enctCur < enctEnd)
        {
            // If asked to shutdown, break out with the status still bad
            if (thrThis.bCheckShutdownRequest())
            {
                if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
                    ZWLogDbgMsg(L"Breaking out of replication for shutdown");
                break;
            }

            tCIDLib::TCard1 c1CmdId;
            tCIDLib::TCard1 c1CBId;
            tZWaveUSBS::EMsgTypes eRes = eGetReply
            (
                c4Read
                , mbufIn
                , kCIDLib::enctOneSecond
                , c1CmdId
                , c1CBId
                , kCIDLib::False
            );

            if (eRes == tZWaveUSBS::EMsgType_CallBack)
            {
                if ((c1CmdId == FUNC_ID_ZW_SET_LEARN_MODE)
                &&  (c1CBId == c1CallbackId))
                {
                    if (mbufIn[4] == 1)
                    {
                        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
                            ZWLogDbgMsg(L"Entered controller learn mode");
                    }
                     else if (mbufIn[4] == 6)
                    {
                        m_eRepRes = tZWaveUSBSh::ERepRes_UpdateCfg;
                        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
                            ZWLogDbgMsg(L"Learn mode completed successfully");
                        break;
                    }
                     else
                    {
                        m_eRepRes = tZWaveUSBSh::ERepRes_Failed;
                        m_strRepStatus = L"Learn mode failed by other side. Code=";
                        m_strRepStatus.AppendFormatted(mbufIn[4]);
                        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
                            ZWLogDbgMsg(m_strRepStatus.pszBuffer());
                        break;
                    }
                }
            }
            enctCur = TTime::enctNow();
        }

        // Take us out of receive mode
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
            ZWLogDbgMsg(L"Exiting controller receive mode");

        mbufOut.PutCard1(5, 0);
        mbufOut.PutCard1(REQUEST, 1);
        mbufOut.PutCard1(FUNC_ID_ZW_SET_LEARN_MODE, 2);
        mbufOut.PutCard1(0, 3);
        mbufOut.PutCard1(c1NextCallbackId(), 4);
        SendAndWaitForAck(mbufOut);

        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
            ZWLogDbgMsg(L"Receive mode exited successfully");

        // If we timed out, then set that status
        if (enctCur >= enctEnd)
        {
            m_eRepRes = tZWaveUSBSh::ERepRes_TimedOut;
            m_strRepStatus = L"Timed out waiting for other side to initiate replication";
            if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
                ZWLogDbgMsg(m_strRepStatus.pszBuffer());
        }

        // If it worked, then we need to do discovery and update ourself
        if (m_eRepRes == tZWaveUSBSh::ERepRes_UpdateCfg)
        {
            //
            //  Get init data again, since it will usually have changed and
            //  we need to get an updated node map as well. Give a little
            //  time for everyone to settle after the replication.
            //
            ZWLogDbgMsg(L"Post-learn updating initial controller data");
            TThread::Sleep(500);
            QueryInitData();
            ZWLogDbgMsg(L"Back from querying initial data, validing node config");


            //
            //  We now want to go out and find all of the existing modules
            //  and reconcile them against our existing config. So make a copy
            //  of our existing stuff and call the validation method. Tell it
            //  that it doesn't have to get a new mode map, since we just did.
            //  Tell it to re-verify any existing nodes against their node
            //  types. Tell it to discover any new nodes and add them to our
            //  config with default settings.
            //
            TZWaveDrvConfig dcfgNew(m_dcfgCurrent);
            if (bValidateNodeCfg(dcfgNew, kCIDLib::False, kCIDLib::True, kCIDLib::True))
            {
                //
                //  Ok, let's store this data in the config server. If it
                //  adds, tell it to take 2K of extra space for this item,
                //  for futher growth without having to move it.
                //
                {
                    TCfgServerClient cfgscLoad(5000);
                    tCIDLib::TCard4 c4Ver;
                    cfgscLoad.bAddOrUpdate(m_strCfgScope, c4Ver, dcfgNew, 2048);
                }
                RegisterFields(dcfgNew);
            }
        }

        // Take us back out of replication mode now with success
        m_eRepRes = tZWaveUSBSh::ERepRes_OK;
        m_bRepMode = kCIDLib::False;
    }

    catch(const TError& errToCatch)
    {
        m_eRepRes = tZWaveUSBSh::ERepRes_Failed;
        m_strRepStatus = errToCatch.strErrText();
        m_bRepMode = kCIDLib::False;

        LogError(errToCatch, tCQCKit::EVerboseLvls::Medium);
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
        {
            ZWLogDbgMsg(L"Replication failed with CIDLib exception");
        }
        return tCIDLib::EExitCodes::RuntimeError;
    }

    catch(...)
    {
        m_eRepRes = tZWaveUSBSh::ERepRes_Failed;
        m_strRepStatus = L"Replication failed with unknown exception";
        m_bRepMode = kCIDLib::False;
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
            ZWLogDbgMsg(m_strRepStatus.pszBuffer());
        return tCIDLib::EExitCodes::RuntimeError;
    }
    return tCIDLib::EExitCodes::Normal;
}


//
//  This is called during connect attempts. It first just tries a simple
//  query to get the version info from the controller, so that we know what
//  we are dealing with. If that works, we know the controller is there and
//  can come online.
//
//  It then get's some overall info on the controller, the unit id, home id,
//  Z-Wave firmware version and so forth.
//
tCIDLib::TVoid TZWaveUSBSDriver::QueryInitData()
{
    // Get a buffer to use for the return info
    TSimplePoolJan<THeapBuf> janPool(&m_splBufs, 512);
    TMemBuf& mbufIn = janPool.objGet();

    // Do a controller version query
    tCIDLib::TCard4 c4Read = 0;
    try
    {
        c4Read = c4SendReqAndWaitForReply
        (
            FUNC_ID_ZW_GET_VERSION, 500, 0, mbufIn
        );
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);

        LogError(errToCatch, tCQCKit::EVerboseLvls::High);
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::High)
        {
            ZWLogDbgMsg(L"Got no controller version reply");
        }
        throw;
    }

    //
    //  We go the expected reply. So pull out the Z-Wave firmware version
    //  that it reported. We store the string itself for reporting, but we
    //  also create a single Card4 value that we can use for comparisons
    //
    //  So first off all, cap the string off at the end of the version
    //  info and copy out the text version.
    //
    const tCIDLib::TCard1* pc1EndPtr = mbufIn.pc1Data() + c4Read;
    tCIDLib::TCard1* pc1Buf = mbufIn.pc1DataAt(3);
    m_strCtrlVersion.FromShortZStr(reinterpret_cast<const tCIDLib::TSCh*>(pc1Buf));

    //
    //  Pull out the version info and set up a version stamp. And
    //  move up to the period.
    //
    pc1Buf = mbufIn.pc1DataAt(10);
    tCIDLib::TCard1* pc1Ptr = pc1Buf;
    while((*pc1Ptr != 0x2E) && (pc1Ptr < pc1EndPtr))
        pc1Ptr++;

    // Convert the part before it to the major version
    tCIDLib::TCard4 c4FWMajVer = 0;
    while (pc1Buf < pc1Ptr)
    {
        c4FWMajVer *= 10;
        c4FWMajVer += (*pc1Buf - 0x30);
        pc1Buf++;
    }

    //
    //  Move up past the period and find the nul terminator at the end of
    //  the version info.
    //
    pc1Buf++;
    while(*pc1Ptr && (pc1Ptr < pc1EndPtr))
        pc1Ptr++;

    // And convert this part to the minor version, skipping leading zeros
    tCIDLib::TCard4 c4FWMinVer = 0;
    tCIDLib::TCard4 c4MulBy = 100000;
    while (pc1Buf < pc1Ptr)
    {
        if (*pc1Buf != 0x30)
        {
            c4FWMinVer *= 10;
            c4FWMinVer += (*pc1Buf - 0x30);
            c4MulBy /= 10;
        }
        pc1Buf++;
    }

    //
    //  We need to create a combined version that we can use for
    //  version comparison. Because of the unfortunate way in which
    //  the minor version is treated, we can't just use it as is.
    //  Version 1.234 is an earlier version than 1.91. So we convert
    //  the value into a ten thousands value, by seeing how
    //  many digits there are and multiplying accordingly. Making
    //  it 12340 vs. 91000, which works out correctly.
    //
    //  We kept up with now many tens we need to multiply by above
    //  during the loop. The major version goes into the top nibble,
    //  leaving plenty of room for the adjusted minor version.
    //
    m_c4FWVersion = (c4FWMajVer << 24);
    m_c4FWVersion |= (c4FWMinVer * c4MulBy);

    //
    //  OK, that worked, so let's try to get the initial data info. This
    //  can be different sizes depending on the firmware version.
    //
    c4Read = c4SendReqAndWaitForReply
    (
        FUNC_ID_SERIAL_API_GET_INIT_DATA, 500, 0, mbufIn
    );

    // It has to be at least 35 bytes
    if (c4Read < 35)
    {
        // <TBD>
    }
    m_c1SerAPIVersion = mbufIn[3];
    m_bIsSUC = (mbufIn[4] & GET_INIT_DATA_FLAG_IS_SUC) != 0;
    m_bInNetwork = (mbufIn[4] & GET_INIT_DATA_FLAG_CONTROLLER_STATUS) != 0;

    //
    //  Set up the node map with all the known nodes. The map is the 29
    //  bytes starting at offset 6. Unit id 1 is at bit index 0 and so
    //  forth.
    //
    m_bsNodeMap.FromRawArray(mbufIn.pc1DataAt(6), 29);

    // And get our id info
    c4SendReqAndWaitForReply(FUNC_ID_MEMORY_GET_ID, 250, 8, mbufIn);
    m_c4HomeId = mbufIn[6];
    m_c4HomeId <<= 8;
    m_c4HomeId |= mbufIn[5];
    m_c4HomeId <<= 8;
    m_c4HomeId |= mbufIn[4];
    m_c4HomeId <<= 8;
    m_c4HomeId |= mbufIn[3];
    m_c1CtrlId = mbufIn[7];

    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::High)
    {
        TString strCtrlInfo(L"Serial Ver=");
        strCtrlInfo.AppendFormatted(m_c1SerAPIVersion);
        strCtrlInfo.Append(L", CtrlId=");
        strCtrlInfo.AppendFormatted(m_c1CtrlId);
        strCtrlInfo.Append(L", HomeId=");
        strCtrlInfo.AppendFormatted(m_c4HomeId, tCIDLib::ERadices::Hex);
        ZWLogDbgMsg(strCtrlInfo.pszBuffer());
    }
}


//
//  Queries a unit for group associations and returns a string that holds
//  the ids of the units it is associated with. The parms string holds the
//  name of the unit to query and the group id to query for, separated by
//  space.
//
tCIDLib::TVoid
TZWaveUSBSDriver::QueryUnitAssoc(const  TString&    strParms
                                ,       TString&    strToFill)
{
    // Parse the three values out
    TStringTokenizer stokVal(&strParms, kCIDLib::szWhitespace);

    tCIDLib::TCard1 c1GrpId;
    TString strGrpId;
    TString strTarUnit;

    if (!stokVal.bGetNextToken(strTarUnit)
    ||  !stokVal.bGetNextToken(strGrpId)
    ||  !strGrpId.bToCard1(c1GrpId)
    ||  stokVal.bMoreTokens())
    {
        // It's not well formed
        facZWaveUSBS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kZWErrs::errcGen_BadParms
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TString(L"query unit association")
        );
    }

    // Look up the target unit
    const TZWaveUnit& unitTar = m_dcfgCurrent.unitFindByName(strTarUnit);

    // Ask it to format us up the appropriate message
    TSimplePoolJan<THeapBuf> janOut(&m_splBufs, 256);
    TMemBuf& mbufOut = janOut.objGet();

    tCIDLib::TCard1 c1RetClass;
    tCIDLib::TCard1 c1RetCmd;
    tCIDLib::TCard1 c1RetLen;

    const tCIDLib::TCard1 c1CBId = c1NextCallbackId();
    BuildUnitGetMsg
    (
        mbufOut
        , unitTar
        , c1RetClass
        , c1RetCmd
        , c1RetLen
        , c1GrpId
        , 0
        , c1CBId
        , tZWaveUSBSh::EUGetCmd_GetGrpAssoc
    );


    // Send and wait for the reply
    TSimplePoolJan<THeapBuf> janIn(&m_splBufs, 512);
    TMemBuf& mbufIn = janIn.objGet();

    SendAndWaitForAck(mbufOut);
    WaitForCmdCB
    (
        unitTar.c1Id()
        , c1RetClass
        , c1CBId
        , c1RetLen
        , kCIDLib::enctFiveSeconds
        , mbufIn
    );
}


//
//  Called when the client driver asks to reset the controller. We put ourself
//  back to ground zero state, though still online since we are still talking
//  to the controller, and reset the controller back to it's default state.
//
//  We let any exceptions propogate back. The operations are done such that,
//  if it fails, it won't leave us in an inconsistent state, though they might
//  have re-replicate to get us back going again.
//
tCIDLib::TVoid TZWaveUSBSDriver::ResetCtrl()
{
    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Low)
    {
        ZWLogDbgMsg1
        (
            L"Resetting the Z-Wave controller for driver=%(1)", strMoniker()
        );
    }

    // Get a buffer from the pool
    TSimplePoolJan<THeapBuf> janPool(&m_splBufs, 64);
    TMemBuf& mbufOut = janPool.objGet();

    // And now reset the controller
    const tCIDLib::TCard1 c1CBId = c1NextCallbackId();
    mbufOut.PutCard1(4, 0);
    mbufOut.PutCard1(REQUEST, 1);
    mbufOut.PutCard1(FUNC_ID_ZW_SET_DEFAULT, 2);
    mbufOut.PutCard1(c1CBId, 3);
    SendAndWaitForAck(mbufOut);

    // It will send a callback, so wait for that
    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
        ZWLogDbgMsg(L"Waiting for controller to reset");
    WaitForCmdCB
    (
        FUNC_ID_ZW_SET_DEFAULT, c1CBId, kCIDLib::enctFiveSeconds, mbufOut
    );

    //
    //  And update our status flags. Worst case, it might die right here, so
    //  set the primary flag just in case, since we know now that we are a
    //  primary controller and no longer in the network. And clear the id for
    //  the same reason.
    //
    m_bInNetwork = kCIDLib::False;
    m_bIsSUC = kCIDLib::False;
    m_bRepMode = kCIDLib::False;
    m_bsNodeMap = kZWaveUSBSh::c1MaxUnits;
    m_c1CtrlId = 0;
    m_c1NextCBId = 1;
    m_c1SerAPIVersion = 0;
    m_c4FWVersion = 0;
    m_c4HomeId = 0;
    m_c4PollableCnt = 0;
    m_c4LastGrpId = 0;
    QueryInitData();

    //
    //  It worked, so let's trash our configuraton and update our field
    //  list and write the new config out to the config server.
    //
    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
        ZWLogDbgMsg(L"Clearing unit/group config and storing it");

    m_dcfgCurrent.Reset();
    RegisterFields(m_dcfgCurrent);
    {
        TCfgServerClient cfgscLoad(1000);
        tCIDLib::TCard4 c4Ver;
        cfgscLoad.bAddOrUpdate(m_strCfgScope, c4Ver, m_dcfgCurrent, 2048);
    }
}


//
//  We allow clients to send a backdoor command to add or delete associations
//  in a unit. The parameter string contains three values, the target unit
//  name, the group number, and the unit id to add/remove. The unit to add
//  or remove is an id because it might be one we don't know about.
//
tCIDLib::TVoid
TZWaveUSBSDriver::SetAssociation(const  tCIDLib::TBoolean   bAdd
                                , const TString&            strParams)
{
    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Low)
    {
        ZWLogDbgMsg2
        (
            L"Got add/delete association for driver %(1), values=%(2)"
            , strMoniker()
            , strParams
        );
    }

    // Parse the three values out
    TStringTokenizer stokVal(&strParams, kCIDLib::szWhitespace);

    tCIDLib::TCard1 c1GrpId;
    tCIDLib::TCard1 c1SrcId;
    TString strGrpId;
    TString strSrcId;
    TString strTarUnit;

    if (!stokVal.bGetNextToken(strTarUnit)
    ||  !stokVal.bGetNextToken(strGrpId)
    ||  !stokVal.bGetNextToken(strSrcId)
    ||  !strGrpId.bToCard1(c1GrpId)
    ||  !strSrcId.bToCard1(c1SrcId)
    ||  stokVal.bMoreTokens())
    {
        // It's not well formed
        facZWaveUSBS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kZWErrs::errcGen_BadParms
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TString(L"set association")
        );
    }

    // Look up the target unit
    const TZWaveUnit& unitTar = m_dcfgCurrent.unitFindByName(strTarUnit);

    // Ask it to format us up the appropriate message
    TSimplePoolJan<THeapBuf> janPool(&m_splBufs, 64);
    TMemBuf& mbufOut = janPool.objGet();

    BuildUnitSetMsg
    (
        mbufOut
        , unitTar
        , c1GrpId
        , c1SrcId
        , 0
        , bAdd ? tZWaveUSBSh::EUSetCmd_AddAssoc
               : tZWaveUSBSh::EUSetCmd_DelAssoc
    );
    SendAndWaitForAck(mbufOut);
}


//
//  We allow clients to send a backdoor command to set Z-Wave unit config
//  parameters. We get the name of the target unit, the parameter number, and
//  the parameter value.
//
tCIDLib::TVoid TZWaveUSBSDriver::SetCfgParam(const TString& strParams)
{
    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Low)
    {
        ZWLogDbgMsg2
        (
            L"Got cfg param set for driver %(1), values=%(2)"
            , strMoniker()
            , strParams
        );
    }

    // Parse the three values out
    TStringTokenizer stokVal(&strParams, kCIDLib::szWhitespace);

    tCIDLib::TCard1 c1PNum;
    tCIDLib::TCard1 c1PVal;
    TString strParm;
    TString strTarUnit;
    TString strVal;

    if (!stokVal.bGetNextToken(strTarUnit)
    ||  !stokVal.bGetNextToken(strParm)
    ||  !stokVal.bGetNextToken(strVal)
    ||  !strParm.bToCard1(c1PNum)
    ||  !strVal.bToCard1(c1PVal)
    ||  stokVal.bMoreTokens())
    {
        // It's not well formed
        facZWaveUSBS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kZWErrs::errcGen_BadParms
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TString(L"set cfg param")
        );
    }

    // Look up the target unit
    const TZWaveUnit& unitTar = m_dcfgCurrent.unitFindByName(strTarUnit);

    // Ask it to format us up the appropriate message
    TSimplePoolJan<THeapBuf> janPool(&m_splBufs, 64);
    TMemBuf& mbufOut = janPool.objGet();

    BuildUnitSetMsg
    (
        mbufOut
        , unitTar
        , c1PNum
        , c1PVal
        , 0
        , tZWaveUSBSh::EUSetCmd_SetCfgParam
    );
    SendAndWaitForAck(mbufOut);
}


//
//  These methods are called when the client driver has gotten changes for
//  the configuration
//
tCIDLib::TVoid
TZWaveUSBSDriver::UpdateConfig(  const   TMemBuf&        mbufData
                                , const tCIDLib::TCard4 c4Bytes)
{
    TZWaveDrvConfig dcfgNew;
    {
        TBinMBufInStream strmSrc(&mbufData, c4Bytes);
        strmSrc >> dcfgNew;
    }

    // Looks good so store it
    m_dcfgCurrent = dcfgNew;

    //
    //  Ok, let's store this data in the config server. If it adds, tell
    //  it to take 2K of extra space for this item, for futher growth
    //  without having to move it.
    //
    {
        TCfgServerClient cfgscLoad(5000);
        tCIDLib::TCard4 c4Ver;
        cfgscLoad.bAddOrUpdate(m_strCfgScope, c4Ver, m_dcfgCurrent, 2048);
    }

    // And finally let's register our fields to reflect the new config
    RegisterFields(m_dcfgCurrent);
}
