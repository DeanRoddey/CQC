//
// FILE NAME: ZWaveUSB3S_DriverImpl2.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/24/2017
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
//  This file handles some of the functionality of the main driver. In this case
//  it's the stuff related to joining our controller to the network, i.e. controller
//  replication. This stuff is done on a separate thread since it goes on for a
//  while.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "ZWaveUSB3S_.hpp"


// ---------------------------------------------------------------------------
//  TZWaveUSB3SDriver: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  This method will do a replication process. This can serve one of three purposes:
//
//  1. It an initially add us to the network
//  2. It is done after we are in the network, to pick up changes that the user has made
//      on the master controller.
//  3. It is used to remove us from the network
//
//
//  If replicating that will get us added to an existing Z-Wave network (or removed from
//  it.) The user puts the master controller into include or exclude mode and we invoke
//  learn mode on our side and we get added to the network or removed from it.
//
//  The scan phase will scan the network will find existing units and gather up info
//  about them that we can find.
//
//  In either case, we will only do the scan if were part of the network, or got added
//  to it during the replication.
//
//  We throw on any error. We return false if asked to shut down, else true.
//
//  Note that while the replication loop is running, msgs will still get dispatched to
//  the application command handler. It will ignore them because it sees that a replication
//  pass is occurring, and it doesn't want to modify the existing configuration or nodes
//  which may not match what we are getting now.
//
//  NOTE that this may also remove us from the network if in replication mode. In that
//  case, we don't do anything
//
tCIDLib::TBoolean TZWaveUSB3SDriver::bDoReplication()
{
    //
    //  Remember if we are current in the network. This lets us know before if our
    //  status changes.
    //
    const tCIDLib::TBoolean bWasInNetwork = m_dcfgReplicate.bInNetwork();

    //
    //  Clear this flag before we start the replication loop. WE check it below after
    //  we do our post replication loop setup.
    //
    m_c1GotSecSchemeGet = 0;

    // Do the replication process first
    tCIDLib::TBoolean bRepState = kCIDLib::False;
    TZWInMsg zwimIn;
    TZWOutMsg zwomOut;
    try
    {
        tCIDLib::TEncodedTime enctCur = TTime::enctNow();
        const tCIDLib::TEncodedTime enctEnd(enctCur + (kCIDLib::enctOneSecond * 30));

        SetDrvState(tZWaveUSB3Sh::EDrvStates::Replicating);
        if (facZWaveUSB3Sh().bMediumTrace())
        {
            facZWaveUSB3Sh().LogTraceMsg
            (
                L"Entering receive mode... Config says we are %(1) the network"
                , bWasInNetwork ? TString(L"in") : TString(L"not in")
            );
        }

        //
        //  Put us into receive mode. This is a special case. We have to set a callback
        //  id, since it will call us back in the loop below. But that will make the
        //  I/O thread block on callback, which may not come for some time. So we call
        //  the special non-blocking callback append. That appends a special callback id
        //  that will not trigger the I/O thread to wait for the callback.
        //
        zwomOut.Reset(FUNC_ID_ZW_SET_LEARN_MODE, tZWaveUSB3Sh::EMsgPrios::SpecialCmd);
        zwomOut += 0x1;     // Enable learn mode
        zwomOut.AppendCallback(kCIDLib::True);
        zwomOut.Finalize(kCIDLib::False, kCIDLib::False);
        const tCIDLib::TCard1 c1RepCBId = zwomOut.c1CallbackId();
        USB3Send(zwomOut);

        TThread* pthrUs = TThread::pthrCaller();
        while (enctCur < enctEnd)
        {
            // If asked to shutdown, break out with the status still bad
            if (pthrUs->bCheckShutdownRequest())
            {
                if (facZWaveUSB3Sh().bMediumTrace())
                    facZWaveUSB3Sh().LogTraceMsg(L"Breaking out of replication for shutdown");
                break;
            }

            // Wait for up a while for a message
            const tZWaveUSB3Sh::EMsgTypes eType = eGetReply
            (
                zwimIn, kCIDLib::enctOneSecond, kCIDLib::False
            );

            if (eType != tZWaveUSB3Sh::EMsgTypes::Timeout)
            {
                tCIDLib::TCard1 c1CmdClass, c1CmdId;
                if (zwimIn.bIsAppCmd(c1CmdClass, c1CmdId))
                {
                    if (c1CmdClass == COMMAND_CLASS_CONTROLLER_REPLICATION)
                    {
                        //
                        //  We don't normally get these since these days you go
                        //  back and get the info from the controller's table.
                        //  But, if we did, and didn't acknowledge it, the
                        //  replication would never complete.
                        //
                        zwomOut.Reset
                        (
                            FUNC_ID_ZW_REPLICATION_COMMAND_COMPLETE
                            , tZWaveUSB3Sh::EMsgPrios::SpecialCmd
                        );
                        zwomOut.Finalize(kCIDLib::False, kCIDLib::False);
                        USB3Send(zwomOut);
                    }
                }
                 else if (eType == tZWaveUSB3Sh::EMsgTypes::Request)
                {
                    if (zwimIn.c1CallbackId() == c1RepCBId)
                    {
                        if (zwimIn.m_c1MsgId == FUNC_ID_ZW_SET_LEARN_MODE)
                        {
                            if (zwimIn[1] == LEARN_MODE_STARTED)
                            {
                                if (facZWaveUSB3Sh().bMediumTrace())
                                    facZWaveUSB3Sh().LogTraceMsg(L"Entered controller learn mode");
                            }
                             else if (zwimIn[1] == LEARN_MODE_DONE)
                            {
                                if (facZWaveUSB3Sh().bMediumTrace())
                                    facZWaveUSB3Sh().LogTraceMsg(L"Learn mode completed successfully");
                                bRepState = kCIDLib::True;
                                break;
                            }
                             else if (zwimIn[1] == LEARN_MODE_FAILED)
                            {
                                if (facZWaveUSB3Sh().bMediumTrace())
                                {
                                    facZWaveUSB3Sh().LogTraceMsg
                                    (
                                        L"Learn mode failed by other side. Code=%(1)"
                                        , TCardinal(zwimIn[1])
                                    );
                                }
                                bRepState = kCIDLib::False;
                                break;
                            }
                        }
                    }
                     else if (zwimIn.m_c1MsgId == FUNC_ID_ZW_APPLICATION_CONTROLLER_UPDATE)
                    {
                        if (facZWaveUSB3Sh().bMediumTrace())
                            facZWaveUSB3Sh().LogTraceMsg(L"App controller update");
                    }
                }
            }

            enctCur = TTime::enctNow();
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        if (facZWaveUSB3Sh().bLowTrace())
            facZWaveUSB3Sh().LogTraceMsg(L"Exception in replication loop");
        facZWaveUSB3Sh().FlushMsgTrace();

        facZWaveUSB3S().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kZW3Errs::errcRep_RepLoopFailed
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::CantDo
        );

        // Set the rep state flag and fall through
        bRepState = kCIDLib::False;
    }

    // One way or another we get out of learn mode now
    try
    {
        if (facZWaveUSB3Sh().bLowTrace())
            facZWaveUSB3Sh().LogTraceMsg(L"Exiting learn mode");

        zwomOut.Reset(FUNC_ID_ZW_SET_LEARN_MODE, tZWaveUSB3Sh::EMsgPrios::SpecialCmd);
        zwomOut += 0;  // Disable learn mode
        zwomOut.AppendNullCallback();
        zwomOut.Finalize(kCIDLib::False, kCIDLib::False);
        USB3Send(zwomOut);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        if (facZWaveUSB3Sh().bLowTrace())
            facZWaveUSB3Sh().LogTraceMsg(L"Exception trying to exit learn mode");

        // Bad but fall through
    }

    // If we failed the loop above, we can give up now
    if (!bRepState)
    {
        if (facZWaveUSB3Sh().bLowTrace())
            facZWaveUSB3Sh().LogTraceMsg(L"Abandoning replication operation");
        return kCIDLib::False;
    }


    // Now do the followup steps that we need to do after any replication pass
    try
    {
        if (facZWaveUSB3Sh().bLowTrace())
        {
            facZWaveUSB3Sh().LogTraceMsg
            (
                L"Starting post-replication work... getting controller info"
            );
        }

        //
        //  We need to know if we are in or out of the network so get initial data
        //  about our controller, home id, man ids, etc...
        //
        m_dcfgReplicate.LoadControllerInfo(*this);

        if (facZWaveUSB3Sh().bLowTrace())
        {
            facZWaveUSB3Sh().LogTraceMsg
            (
                L"Controller reports we are %(1) the network. Our id is %(2)"
                , m_dcfgReplicate.bInNetwork() ? TString(L"in") : TString(L"not in")
                , TCardinal(m_dcfgReplicate.c1CtrlId())
            );
        }

        //
        //  Tell the shared facility our new unit id. This is also stored in the
        //  config after this is all done, but he needs it now in case we need to
        //  do any secure comms with the master. It'll get set again redundantly
        //  at the end when all of the new discovery info is stored.
        //
        facZWaveUSB3Sh().SetCtrlId(m_dcfgReplicate.c1CtrlId());

        //
        //  Tell the Z-Stick object about the man ids for our controller, so he can
        //  serve that info up when it is requested. We will broadcast it below,
        //  but just in case the master asks us before we get a chance to. This will
        //  also get redundantly set later when all of the discovry info is stored.
        //
        m_zstCtrl.SetManIds(m_dcfgReplicate.c8ManIds());

        // And now deal with inclusion or exclusion
        if (m_dcfgReplicate.bInNetwork() && !bWasInNetwork)
        {
            /*
             * Not really sure what advantages there are and what our responsibilities
             * are if we enable us as the SUC. We don't seem to get the reports that
             * a SUC should get (which would be useful if we did.) There may be things we
             * have to do to make it all work, but not really sure.
             *

            // If there is no SUC in the network, set us to be it.
            zwomOut.Reset(FUNC_ID_ZW_GET_SUC_NODE_ID, tZWaveUSB3Sh::EMsgPrios::Query);
            zwomOut.Finalize(kCIDLib::False, kCIDLib::False);
            USB3SendCtrlRes(zwomOut, zwimIn, 3000);

            if (zwimIn[0] == 0)
            {
                facZWaveUSB3Sh().LogTraceMsg(L"No SUC found, taking that role");
                zwomOut.Reset(FUNC_ID_ZW_ENABLE_SUC, tZWaveUSB3Sh::EMsgPrios::SpecialCmd);
                zwomOut += 1;
                zwomOut += ZW_SUC_FUNC_BASIC_SUC;
                zwomOut.Finalize(kCIDLib::False, kCIDLib::False);
                USB3Send(zwomOut);

                zwomOut.Reset(FUNC_ID_ZW_SET_SUC_NODE_ID,tZWaveUSB3Sh::EMsgPrios::SpecialCmd);
                zwomOut += m_dcfgReplicate.c1CtrlId();
                zwomOut += 1;
                zwomOut += 0;
                zwomOut += ZW_SUC_FUNC_BASIC_SUC;
                zwomOut.AppendNullCallback();
                zwomOut.Finalize(kCIDLib::False,kCIDLib::False);
                USB3SendCtrlRes(zwomOut, zwimIn, 2000);
            }
             else
            {
                facZWaveUSB3Sh().LogTraceMsg(L"SUC node id is %(1)", TCardinal(zwimIn[0]));
            }
            */


            //
            //  We weren't in the network, but we are now, so we got included. That means
            //  we need to deal with network key exchange stuff.
            //
            SetDrvState(tZWaveUSB3Sh::EDrvStates::SecureJoin);

            // If we have not seen the security scheme get yet, then wait for it
            if (!m_c1GotSecSchemeGet)
            {
                facZWaveUSB3Sh().LogTraceMsg(L"No security scheme get yet, waiting...");
                bUSB3WaitForMsg
                (
                    tZWaveUSB3Sh::EMsgTypes::Request
                    , 0
                    , FUNC_ID_APPLICATION_COMMAND_HANDLER
                    , COMMAND_CLASS_SECURITY
                    , SECURITY_SCHEME_GET
                    , kCIDLib::enctOneSecond * 15
                    , zwimIn
                );
                facZWaveUSB3Sh().LogTraceMsg(L"Got security scheme get");

                m_c1GotSecSchemeGet = zwimIn.c1SrcId();
            }

            //
            //  Remember the master controller id. We can only get that from a msg that
            //  comes from it, as best I can tell. We either got it above, or it
            //  was stored when we
            //
            m_dcfgReplicate.c1MasterId(m_c1GotSecSchemeGet);

            //
            //  Now we can respond to the security scheme get, which must not be secure
            //  yet.
            //
            facZWaveUSB3Sh().SetDefNetworkKey();
            zwomOut.Reset
            (
                m_c1GotSecSchemeGet
                , COMMAND_CLASS_SECURITY
                , SECURITY_SCHEME_REPORT
                , 0
                , 0
                , tZWaveUSB3Sh::EMsgPrios::Security
            );
            zwomOut += 0;
            zwomOut.EndCounter();
            zwomOut.AppendTransOpts();
            zwomOut.AppendCallback();
            zwomOut.Finalize(kCIDLib::False, kCIDLib::False);
            USB3SendCCMsg(zwomOut);


            //
            //  Now wait for the key set command. We have to wait for a while. The
            //  z-stick will store the key on the shared facility as soon as this msg
            //  comes in so that it's set for any other security calls that might
            //  happen before we see it here.
            //
            bUSB3WaitForMsg
            (
                tZWaveUSB3Sh::EMsgTypes::Request
                , 0
                , FUNC_ID_APPLICATION_COMMAND_HANDLER
                , COMMAND_CLASS_SECURITY
                , NETWORK_KEY_SET
                , kCIDLib::enctOneSecond * 30
                , zwimIn
            );
            if (facZWaveUSB3Sh().bLowTrace())
                facZWaveUSB3Sh().LogTraceMsg(L"Network key received");


            //
            //  Store away the key as our driver config data. The Z-stick has already
            //  stored it on the shared facility object as soon as the msg came in, so
            //  that it's set in time for subsequent operations.
            //
            THeapBuf mbufKey(16, 16);
            zwimIn.CopyOutPLBytes(mbufKey, zwimIn.c4FirstCCIndex() + 2, 16);
            if (!bSetCfgData(mbufKey, 32))
            {
                facZWaveUSB3Sh().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kZW3Errs::errcCfg_SaveKey
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Config
                );
            }

            // And finally wait for the security scheme inherit
            bUSB3WaitForMsg
            (
                tZWaveUSB3Sh::EMsgTypes::Request
                , 0
                , FUNC_ID_APPLICATION_COMMAND_HANDLER
                , COMMAND_CLASS_SECURITY
                , SECURITY_SCHEME_INHERIT
                , kCIDLib::enctOneSecond * 20
                , zwimIn
            );
            if (facZWaveUSB3Sh().bLowTrace())
                facZWaveUSB3Sh().LogTraceMsg(L"Got final inclusion msg from master");

            //
            //  Assume we are in the secure network now and update our node info to
            //  reflect that, which changes what classes we report via the NIF and what
            //  we only report via the secure classes supported query.
            //
            SetNodeInfo(kCIDLib::True);

            //
            //  Wait a while for it to send us a naming command. Some masters may not, so
            //  don't wait too long and don't throw if it fails.
            //
            if (facZWaveUSB3Sh().bLowTrace())
                facZWaveUSB3Sh().LogTraceMsg(L"Waiting for naming command from master");

            bUSB3WaitForMsg
            (
                tZWaveUSB3Sh::EMsgTypes::Request
                , m_dcfgReplicate.c1MasterId()
                , FUNC_ID_APPLICATION_COMMAND_HANDLER
                , COMMAND_CLASS_NODE_NAMING
                , NODE_NAMING_NODE_NAME_SET
                , kCIDLib::enctFiveSeconds
                , zwimIn
                , kCIDLib::False
            );

            if (facZWaveUSB3Sh().bLowTrace())
                facZWaveUSB3Sh().LogTraceMsg(L"The driver has been added to the Z-Wave network");
        }
         else if (!m_dcfgReplicate.bInNetwork() && bWasInNetwork)
        {
            if (facZWaveUSB3Sh().bLowTrace())
                facZWaveUSB3Sh().LogTraceMsg(L"The driver has been removed from the Z-Wave network");

            //
            //  We were in the network but we aren't now, so we need to do a little
            //  cleanup.
            //
            //  Make sure the network key gets reverted to default, which will be in the
            //  discovery info by default since it never got set to anything else.
            //
            facZWaveUSB3Sh().SetDefNetworkKey();

            // Set our stored key to the default
            THeapBuf mbufKey(16,16);
            mbufKey.Set(0);
            if (!bSetCfgData(mbufKey))
            {
                facZWaveUSB3Sh().LogMsg
                (
                    CID_FILE
                    ,CID_LINE
                    ,kZW3Errs::errcCfg_SaveKey
                    ,tCIDLib::ESeverities::Failed
                    ,tCIDLib::EErrClasses::Config
                );
            }

            // Set our node info to reflect not being in the network
            SetNodeInfo(kCIDLib::False);
        }
         else if (m_dcfgReplicate.bInNetwork() && bWasInNetwork)
        {
            // If just a refresh, then send our node info just in case
            SetNodeInfo(kCIDLib::True);
        }

        //
        //  If we are in the network now, probe any new units. We can do this here
        //  instead of on the driver thread, since this all has to be synchronously.
        //  And the info gathered will survive the copy ctor back to the real config
        //  once the driver thread sees we are complete.
        //
        if (m_dcfgReplicate.bInNetwork())
            m_dcfgReplicate.ProbeNewUnits(this);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        if (facZWaveUSB3Sh().bLowTrace())
            facZWaveUSB3Sh().LogTraceMsg(L"Exception occurred during post-replication updates");
        facZWaveUSB3Sh().FlushMsgTrace();

        facZWaveUSB3S().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kZW3Errs::errcRep_PostRepFailed
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::CantDo
        );
        return kCIDLib::False;
    }

    facZWaveUSB3Sh().FlushMsgTrace();

    return kCIDLib::True;
}


//
//  We write out the passed config to our configuration file. We don't assume this is
//  the current config. The caller must update the current config if that is needed.
//
tCIDLib::TBoolean TZWaveUSB3SDriver::bSaveConfig(const TZWaveDrvCfg& dcfgNew)
{
    try
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
        strmOut << dcfgNew << kCIDLib::FlushIt;
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        facZWaveUSB3S().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kZW3Errs::errcCfg_SaveFile
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Config
        );
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


//
//  This method is started on the m_thrRepScan thread. It handles the extended processes
//  of background replication and network scanning. While we are doing this the main driver
//  thread will reject any incoming commands and our app cmd handler will ignore any incoming
//  commands.
//
tCIDLib::EExitCodes
TZWaveUSB3SDriver::eRepScanThread(TThread& thrThis, tCIDLib::TVoid*)
{
    SetDrvState(tZWaveUSB3Sh::EDrvStates::Replicating);

    // And now let our caller go
    thrThis.Sync();

    // Get buffers for msg i/o
    TZWInMsg    zwimIn;
    THeapBuf    mbufOut(32, 255);

    if (facZWaveUSB3Sh().bLowTrace())
        facZWaveUSB3Sh().LogTraceMsg(L"Starting bgn replication thread");

    //
    //  Call the replication method to start up the replication thread. Set our
    //  driver state based on the return value.
    //
    try
    {
        if (bDoReplication())
            SetDrvState(tZWaveUSB3Sh::EDrvStates::RepSucceeded);
        else
            SetDrvState(tZWaveUSB3Sh::EDrvStates::RepFailed);
    }

    catch(...)
    {
        // Just to be absolutely safe, since this would shut down CQCServer if it leaked
        if (facZWaveUSB3Sh().bLowTrace())
            facZWaveUSB3Sh().LogTraceMsg(L"System exception during replication");

        SetDrvState(tZWaveUSB3Sh::EDrvStates::RepFailed);
    }

    return tCIDLib::EExitCodes::Normal;
}

