//
// FILE NAME: ZWaveLeviS_DriverImpl4.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/03/2014
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
//  This file handles some grunt work overflow from the main driver impl file. This
//  file provides some miscellaneous functionality.
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

//
//  This is a handler for one of the InvokeCmd field commands. This one is used to
//  issue any auto-generation commands form the device info for the passed target. So
//  the target has to be already set up and ready. If it's a wakeup type, the commands
//  will be queued up.
//
tCIDLib::TVoid TZWaveLevi2SDriver::DoAutoCfg(TZWaveUnit& unitTar)
{
    // The unit has to have had it's device info set, and can't be missing
    if (!unitTar.bDevInfoLoaded() || unitTar.bMissing())
    {
        facZWaveLevi2Sh().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kZW2ShErrs::errcCfg_NotReady
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotReady
            , unitTar.strName()
        );
    }

    LogMsg
    (
        L"Starting auto-config for unit %(1) (%(2))"
        , tCQCKit::EVerboseLvls::Off
        , CID_FILE
        , CID_LINE
        , unitTar.strName()
        , TCardinal(unitTar.c4Id())
    );

    // The VRCOP has to be set
    const tCIDLib::TCard4 c4VRCOPId = m_dcfgCurrent.c4FindVRCOP(kCIDLib::True);

    // For each step in the auto-config list, let's process them
    tCIDLib::TCardList fcolVals(16);
    TString strTmp1, strTmp2;
    const tCIDLib::TCard4 c4Count = unitTar.zwdiThis().m_colAutoCfg.c4ElemCount();
    TThread* pthrCaller = TThread::pthrCaller();

    // If it implements wakeup, then assume we need to queue the messages
    const tCIDLib::TBoolean bQMsgs(unitTar.bImplementsClass(tZWaveLevi2Sh::ECClasses::WakeUp));

    tCIDLib::TCard4 c4Index = 0;
    tCIDLib::TCard4 c4RetryCnt = 0;
    while (c4Index < c4Count)
    {
        const TKeyValues& kvalsCur = unitTar.zwdiThis().m_colAutoCfg[c4Index];

        // Be sure to clear out the values list every time
        fcolVals.RemoveAll();

        if (kvalsCur.strKey() == L"Grp")
        {
            // Do a group association to the VRCOP
            fcolVals.c4AddElement(COMMAND_CLASS_ASSOCIATION);
            fcolVals.c4AddElement(ASSOCIATION_SET);
            fcolVals.c4AddElement(kvalsCur.strVal2().c4Val());
            fcolVals.c4AddElement(c4VRCOPId);

            if (unitTar.bImplementsClass(tZWaveLevi2Sh::ECClasses::WakeUp))
            {
                TZWQCmd zwqcNew(0, tZWaveLevi2Sh::ECClasses::None, fcolVals);
                unitTar.QueueUpCmd(zwqcNew, *this);
            }
             else
            {
                bSendWaitAck
                (
                    unitTar.c4Id()
                    , 0
                    , fcolVals
                    , tZWaveLevi2Sh::EMultiTypes::Single
                    , unitTar.bClassIsSecure(tZWaveLevi2Sh::ECClasses::Association)
                    , kCIDLib::True
                );
            }
        }
         else if (kvalsCur.strKey() == L"CfgParm")
        {
            fcolVals.c4AddElement(COMMAND_CLASS_CONFIGURATION);
            fcolVals.c4AddElement(CONFIGURATION_SET);
            fcolVals.c4AddElement(kvalsCur.strVal2().c4Val());

            // Break out the value and size from the third value
            strTmp1 = kvalsCur.strVal3();
            if (!strTmp1.bSplit(strTmp2, L'/'))
            {
                facZWaveLevi2Sh().ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kZW2Errs::errcACfg_BadCfgParmVal
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Format
                    , TCardinal(c4Index + 1)
                    , unitTar.strName()
                );
            }
            const tCIDLib::TInt4 i4Val = strTmp1.i4Val();
            const tCIDLib::TCard4 c4Size = strTmp2.c4Val();

            // Put the value size in
            fcolVals.c4AddElement(c4Size);

            // Put in the bytes in the appropriate size format
            if (c4Size == 1)
            {
                fcolVals.c4AddElement(tCIDLib::TInt1(i4Val));
            }
             else if (c4Size == 2)
            {
                const tCIDLib::TInt2 i2Val = tCIDLib::TInt2(i4Val);
                fcolVals.c4AddElement(tCIDLib::TCard1(i2Val >> 8));
                fcolVals.c4AddElement(tCIDLib::TCard1(i2Val));
            }
             else if (c4Size == 4)
            {
                fcolVals.c4AddElement(tCIDLib::TCard1(i4Val >> 24));
                fcolVals.c4AddElement(tCIDLib::TCard1(i4Val >> 16));
                fcolVals.c4AddElement(tCIDLib::TCard1(i4Val >> 8));
                fcolVals.c4AddElement(tCIDLib::TCard1(i4Val));
            }
             else
            {
                facZWaveLevi2Sh().ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kZW2Errs::errcACfg_BadCfgParmSz
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Format
                    , TCardinal(c4Index + 1)
                    , unitTar.strName()
                );
            }

            if (bQMsgs)
            {
                TZWQCmd zwqcNew(0, tZWaveLevi2Sh::ECClasses::None, fcolVals);
                unitTar.QueueUpCmd(zwqcNew, *this);
            }
             else
            {
                bSendWaitAck
                (
                    unitTar.c4Id()
                    , 0
                    , fcolVals
                    , tZWaveLevi2Sh::EMultiTypes::Single
                    , unitTar.bClassIsSecure(tZWaveLevi2Sh::ECClasses::Configuration)
                    , kCIDLib::True
               );
            }
        }
         else if (kvalsCur.strKey() == L"SceneCfg")
        {
            // We need to send a scene configuration message
            //
            //  Do a group association to the VRCOP. Values 2 and 3 are the group
            //  and scene ids, respectively. We indicate instant duration.
            //
            fcolVals.c4AddElement(COMMAND_CLASS_SCENE_CONTROLLER_CONF);
            fcolVals.c4AddElement(SCENE_CONTROLLER_CONF_SET);
            fcolVals.c4AddElement(kvalsCur.strVal2().c4Val());
            fcolVals.c4AddElement(kvalsCur.strVal3().c4Val());
            fcolVals.c4AddElement(0);

            if (bQMsgs)
            {
                TZWQCmd zwqcNew(0, tZWaveLevi2Sh::ECClasses::None, fcolVals);
                unitTar.QueueUpCmd(zwqcNew, *this);
            }
             else
            {
                bSendWaitAck
                (
                    unitTar.c4Id()
                    , 0
                    , fcolVals
                    , tZWaveLevi2Sh::EMultiTypes::Single
                    , unitTar.bClassIsSecure(tZWaveLevi2Sh::ECClasses::SceneControllerConf)
                    , kCIDLib::True
                );
            }
        }
         else
        {
            LogMsg
            (
                L"Unknown autocfg cmd %(1), for unit %(2)"
                , tCQCKit::EVerboseLvls::Off
                , CID_FILE
                , CID_LINE
                , kvalsCur.strKey()
                , unitTar.strName()
            );
        }

        // If not a queued up for later, wait a bit for a busy signal
        if (!bQMsgs)
        {
            tZWaveLevi2S::ERepTypes eGot;
            const tCIDLib::TBoolean bBusy = bWaitMsg
            (
                tZWaveLevi2S::ERepType_GenReport
                , TTime::enctNowPlusMSs(1500)
                , unitTar.c4Id()
                , 0
                , COMMAND_CLASS_APPLICATION_STATUS
                , APPLICATION_BUSY
                , eGot
                , fcolVals
                , kCIDLib::False
            );

            // IF busy and it's a try again indicator
            if (bBusy && ((fcolVals[2] == 0) || (fcolVals[2] == 1)))
            {
                // Bump the retry count. If it hits three, give up
                c4RetryCnt++;
                if (c4RetryCnt == 3)
                {
                    // Move forward and reset the retry counter
                    c4Index++;
                    c4RetryCnt = 0;

                    facZWaveLevi2Sh().LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , kZW2Errs::errcComm_UnitBusy
                        , tCIDLib::ESeverities::Failed
                        , tCIDLib::EErrClasses::NotReady
                        , unitTar.strName()
                    );
                }
                 else
                {
                    // Sleep a bit. Give up if asked to shut down
                    tCIDLib::TCard4 c4Secs = 2;
                    if (fcolVals[2] == 1)
                    {
                        c4Secs = fcolVals[3];

                        if (c4Secs > 5)
                            c4Secs = 5;
                    }

                    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
                    {
                        facZWaveLevi2Sh().LogMsg
                        (
                            CID_FILE
                            , CID_LINE
                            , L"AutoCfg sleeping for %(1) seconds at unit request"
                            , tCIDLib::ESeverities::Failed
                            , tCIDLib::EErrClasses::NotReady
                            , TCardinal(c4Secs)
                        );
                    }

                    // Sleep the requested amount, break out if asked to shut down
                    if (!pthrCaller->bSleep(c4Secs * 1000))
                        break;
                }
            }
             else
            {
                // Clear the retry count and bump the index
                c4Index++;
                c4RetryCnt = 0;
            }
        }
         else
        {
            c4Index++;
        }
    }

    LogMsg
    (
        bQMsgs ? L"Auto-config msgs queued for later transmission, unit=%(1), Id=%(2)"
               : L"Auto-config done for unit %(1) (%(2))"
        , tCQCKit::EVerboseLvls::Off
        , CID_FILE
        , CID_LINE
        , unitTar.strName()
        , TCardinal(unitTar.c4Id())
    );
}


//
//
//
tCIDLib::TVoid
TZWaveLevi2SDriver::ProbeUnit(const TString& strUnitId, TTextOutStream& strmTar)
{
    LogMsg
    (
        L"Starting probe of unit id %(1)"
        , tCQCKit::EVerboseLvls::Off
        , CID_FILE
        , CID_LINE
        , strUnitId
    );

    // The data name has to be the id of the unit
    const tCIDLib::TCard4 c4UnitId = strUnitId.c4Val();
    if (!c4UnitId || (c4UnitId > 255))
    {
        strmTar << L"The passed unit id was not valid (1 to 255)\n";
        return;
    }

    //
    //  OK, looks reasonable to let's do it. The first thing we'll do, just to
    //  make sure it's there and to get some possibly useful info is to get the
    //  command class version report. This will give us Z-Wave protocol version
    //  info on the device.
    //
    tCIDLib::TCardList fcolVals(128);

#if 0
    fcolVals.RemoveAll();
    fcolVals.c4AddElement(COMMAND_CLASS_SENSOR_MULTILEVEL);
    fcolVals.c4AddElement(SENSOR_MULTILEVEL_GET);

    if (bSendWaitRep(c4UnitId
                    , 2
                    , COMMAND_CLASS_SENSOR_MULTILEVEL
                    , SENSOR_MULTILEVEL_REPORT
                    , fcolVals
                    , tZWaveLevi2Sh::EMultiTypes::Channel
                    , kCIDLib::False
                    , kCIDLib::False))
    {
        strmTar << L"Got report\n";
    }
#endif


    fcolVals.RemoveAll();
    fcolVals.c4AddElement(COMMAND_CLASS_VERSION);
    fcolVals.c4AddElement(VERSION_GET);
    strmTar << L"\nGeneral Info \n" << L"-------------------------\n";
    if (!bSendWaitRep(c4UnitId, 0, COMMAND_CLASS_VERSION, VERSION_REPORT, fcolVals))
    {
        strmTar << L"  [No response to initial version query, giving up]\n";
        return;
    }
    strmTar << L"  Protocol Version = "
            << fcolVals[3] << L"." << fcolVals[4] << kCIDLib::NewLn
            << L"  App Version = "
            << fcolVals[5] << L"." << fcolVals[6]

            << kCIDLib::DNewLn;

    //
    //  Try to determine if single end point, multi-intance, or multi-channel
    //
    tZWaveLevi2Sh::EMultiTypes eMType = tZWaveLevi2Sh::EMultiTypes::Single;
    tCIDLib::TCard4 c4EndPCnt = 1;

    // First try multi-instance
    fcolVals.RemoveAll();
    fcolVals.c4AddElement(COMMAND_CLASS_MULTI_INSTANCE);
    fcolVals.c4AddElement(MULTI_INSTANCE_GET);
    if (bSendWaitRep(c4UnitId
                    , 0
                    , COMMAND_CLASS_MULTI_INSTANCE
                    , MULTI_INSTANCE_REPORT
                    , fcolVals
                    , tZWaveLevi2Sh::EMultiTypes::Single
                    , kCIDLib::False
                    , kCIDLib::False))
    {
        eMType = tZWaveLevi2Sh::EMultiTypes::Instance;
        c4EndPCnt = fcolVals[3];
    }
     else
    {
        // Try multi-channel
        fcolVals.RemoveAll();
        fcolVals.c4AddElement(COMMAND_CLASS_MULTI_INSTANCE);
        fcolVals.c4AddElement(MULTI_CHANNEL_END_POINT_GET_V2);
        if (bSendWaitRep(c4UnitId
                        , 0
                        , COMMAND_CLASS_MULTI_INSTANCE
                        , MULTI_CHANNEL_END_POINT_REPORT_V2
                        , fcolVals
                        , tZWaveLevi2Sh::EMultiTypes::Single
                        , kCIDLib::False
                        , kCIDLib::False))
        {
            eMType = tZWaveLevi2Sh::EMultiTypes::Channel;
            c4EndPCnt = fcolVals[3];
        }
    }

    strmTar << L"  Multi-Type = ";
    switch(eMType)
    {
        case tZWaveLevi2Sh::EMultiTypes::Single :
            strmTar << L"Single-ended";
            break;

        case tZWaveLevi2Sh::EMultiTypes::Instance :
            strmTar << L"Multi-Instance  Count = " << c4EndPCnt;
            break;

        case tZWaveLevi2Sh::EMultiTypes::Channel :
            strmTar << L"Multi-Channel Count = " << c4EndPCnt;
            break;

        default :
            strmTar << L"[Unknown end point type]";
            break;
    };
    strmTar << kCIDLib::NewLn;


    strmTar << kCIDLib::NewLn;


    #if 0
    // When we moved to ZW-Plus interface this disappeared

    //
    //  Now let's do a composite capability query, which if supported will get us
    //  the command classes supported, which is very desireable to know. We just ask
    //  for end point 1.
    //
    fcolVals.RemoveAll();
    fcolVals.c4AddElement(COMMAND_CLASS_COMPOSITE);
    fcolVals.c4AddElement(COMPOSITE_END_POINT_GET);
    strmTar << L"Command Classes\n" << L"-------------------------\n";
    if (bSendWaitRep(c4UnitId
                    , 0
                    , COMMAND_CLASS_COMPOSITE
                    , COMPOSITE_END_POINT_REPORT
                    , fcolVals
                    , tZWaveLevi2Sh::EMultiTypes::Single
                    , kCIDLib::False
                    , kCIDLib::False))
    {
        strmTar << L"  [got comp info]\n";
    }
     else
    {
        // Try the multi-channel version
        fcolVals.RemoveAll();
        fcolVals.c4AddElement(COMMAND_CLASS_COMPOSITE);
        fcolVals.c4AddElement(MULTI_CHANNEL_ENDPNT_GET);
        if (bSendWaitRep(c4UnitId
                        , 0
                        , COMMAND_CLASS_COMPOSITE
                        , MULTI_CHANNEL_ENDPNT_REPORT
                        , fcolVals
                        , tZWaveLevi2Sh::EMultiTypes::Single
                        , kCIDLib::False
                        , kCIDLib::False))
        {
            strmTar << L"  [got comp info]\n";
        }
         else
        {
            strmTar << L"  [No Composite/Multi-Channel Info]\n";
        }
    }
    strmTar << kCIDLib::NewLn;

    #endif


    //
    //  Query association info. First try version two composite assocation. If that
    /// fails, try regular V1 association.
    //
    strmTar << L"Assocation Info\n" << L"-------------------------\n";

    fcolVals.RemoveAll();
    fcolVals.c4AddElement(COMMAND_CLASS_ASSOCIATION);
    fcolVals.c4AddElement(ASSOCIATION_GROUPINGS_GET);
    if (bSendWaitRep(c4UnitId
                    , 0
                    , COMMAND_CLASS_ASSOCIATION
                    , ASSOCIATION_GROUPINGS_REPORT
                    , fcolVals
                    , tZWaveLevi2Sh::EMultiTypes::Single
                    , kCIDLib::False
                    , kCIDLib::False))
    {
        const tCIDLib::TCard4 c4Count = fcolVals[2];

        strmTar << L"  Group Count = " << c4Count << kCIDLib::NewLn;

        //
        //  Query the version info for the association class. If we get no response,
        //  assume 1.
        //
        fcolVals.RemoveAll();
        fcolVals.c4AddElement(COMMAND_CLASS_VERSION);
        fcolVals.c4AddElement(VERSION_COMMAND_CLASS_GET);
        fcolVals.c4AddElement(COMMAND_CLASS_ASSOCIATION);

        tCIDLib::TCard4 c4AssocVer = 1;
        if (bSendWaitRep(c4UnitId
                        , 0
                        , COMMAND_CLASS_VERSION
                        , VERSION_COMMAND_CLASS_REPORT
                        , fcolVals
                        , tZWaveLevi2Sh::EMultiTypes::Single
                        , kCIDLib::False
                        , kCIDLib::False))
        {
            c4AssocVer = fcolVals[3];
        }

        tCIDLib::TCard4 c4CmdClass;
        tCIDLib::TCard4 c4CmdGet;
        tCIDLib::TCard4 c4CmdRep;

//        if (c4AssocVer == 1)
//        {
            c4CmdClass = COMMAND_CLASS_ASSOCIATION;
            c4CmdGet = ASSOCIATION_GET;
            c4CmdRep = ASSOCIATION_REPORT;

        // When we moved to ZW-Plus headers this went away, need to figure this out again
//        }
//         else
//        {
//            c4CmdClass = COMMAND_CLASS_ASSOCIATION;
//            c4CmdGet = ASSOCIATION_COMPOSITE_CMD_GET_V2;
//            c4CmdRep = ASSOCIATION_COMPOSITE_CMD_REPORT_V2;
//        }

        // Get the info on each group
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            fcolVals.RemoveAll();
            fcolVals.c4AddElement(c4CmdClass);
            fcolVals.c4AddElement(c4CmdGet);
            fcolVals.c4AddElement(c4Index + 1);

            strmTar <<L"  Group #" << (c4Index + 1) << L" = ";
            if (bSendWaitRep(c4UnitId
                            , 0
                            , c4CmdClass
                            , c4CmdRep
                            , fcolVals
                            , tZWaveLevi2Sh::EMultiTypes::Single
                            , kCIDLib::False
                            , kCIDLib::False))
            {
                const tCIDLib::TCard4 c4ByteCnt = fcolVals.c4ElemCount();
                if (c4AssocVer == 1)
                {
                    for (tCIDLib::TCard4 c4BInd = 5; c4BInd < c4ByteCnt; c4BInd++)
                    {
                        if (c4BInd > 5)
                            strmTar << L",";
                        strmTar << fcolVals[c4BInd];
                    }
                }
                 else
                {
                    tCIDLib::TCard4 c4BInd = 4;
                    tCIDLib::TCard4 c4EPMask;
                    while (c4BInd < c4ByteCnt)
                    {
                        if (c4BInd > 4)
                            strmTar << L",";

                        c4EPMask = fcolVals[c4BInd + 1];
                        c4EPMask <<= 8;
                        c4EPMask |= fcolVals[c4BInd + 2];
                        strmTar << fcolVals[c4BInd]
                                << L"["
                                << TCardinal(c4EPMask, tCIDLib::ERadices::Hex)
                                << L"]";

                        c4BInd += 3;
                    }
                }
            }
             else
            {
                strmTar << L"[Query Failed]";
            }
            strmTar << kCIDLib::NewLn;
        }
    }
     else
    {
        strmTar << L"  [No Assocation Capabilities Reported]\n";
    }
    strmTar << kCIDLib::NewLn;


    //
    //  Query wakeup info
    //
    fcolVals.RemoveAll();
    fcolVals.c4AddElement(COMMAND_CLASS_WAKE_UP);
    fcolVals.c4AddElement(WAKE_UP_INTERVAL_GET);
    strmTar << L"Wakeup Info\n" << L"-------------------------\n";
    if (bSendWaitRep(c4UnitId
                    , 0
                    , COMMAND_CLASS_WAKE_UP
                    , WAKE_UP_INTERVAL_REPORT
                    , fcolVals
                    , tZWaveLevi2Sh::EMultiTypes::Single
                    , kCIDLib::False
                    , kCIDLib::False))
    {
        tCIDLib::TCard4 c4Secs = fcolVals[2];
        c4Secs <<= 8;
        c4Secs |= fcolVals[3];
        c4Secs <<= 8;
        c4Secs |= fcolVals[4];

        strmTar << L"  Wakeup Seconds = " << c4Secs << kCIDLib::NewLn;
    }
     else
    {
        strmTar << L"  [No Wakeup Info Available]\n";
    }

    strmTar << kCIDLib::NewLn;
}


tCIDLib::TVoid TZWaveLevi2SDriver::StartTrace(const tCIDLib::TBoolean bStart)
{
    //
    //  Either way, first make sure it's closed, so that we will start over
    //  if we are enabled or reenabled.
    //
    m_strmTrace.Close();
    m_bEnableTrace = kCIDLib::False;

    // And now, if starting, try to open it up
    if (bStart)
    {
        // It's in the same path as our config file
        try
        {
            TPathStr pathCfg = m_strCfgPath;
            pathCfg.AddLevel(L"Trace.Txt");
            m_strmTrace.Open
            (
                pathCfg
                , tCIDLib::ECreateActs::CreateAlways
                , tCIDLib::EFilePerms::Default
                , tCIDLib::EFileFlags::SafeStream
                , tCIDLib::EAccessModes::Write
            );

            //
            //  Wait till now to set this, so that we don't start everyone tracing into
            //  a stream we weren't able to open.
            //
            m_bEnableTrace = kCIDLib::True;

            // Put out some initial info
            TTime tmCur(tCIDLib::ESpecialTimes::CurrentTime);
            m_strmTrace << L"Z-Wave V2 Trace\n"
                        << L"  Version: " << m_c4VerMaj << L'.' << m_c4VerMin
                        << L",  Moniker: " << strMoniker()
                        << L",  Time: " << tmCur
                        << L"\n----------------------------------------------------"
                        << kCIDLib::DNewLn;
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);

            LogMsg
            (
                L"Failed to start Z-Wave trace"
                , tCQCKit::EVerboseLvls::Off
                , CID_FILE
                , CID_LINE
            );
        }
    }
}

