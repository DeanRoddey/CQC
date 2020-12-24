//
// FILE NAME: HAIOmniTCPS_DriverImpl2.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 05/16/2008
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
//  This file contains grunt work methods from the main implementation file,
//  because this driver is far too large for one file. This one contains the
//  grunt work logic for getting data from the device and getting it stored
//  into our fields.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "HAIOmniTCPS.hpp"



// ---------------------------------------------------------------------------
//  THAIOmniTCPSDriver: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  This is called after registering the fields, to get the initial values
//  into the fields.
//
tCIDLib::TBoolean THAIOmniTCPSDriver::bInitFields()
{
    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
    {
        facHAIOmniTCPS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"Initializing fields"
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
    }

    try
    {
        //
        //  This one gets some simple info, and we check that it's an OmniProII
        //  model, which is the only one we support.
        //
        THeapBufPool::TElemJan janPool(&m_splBufs, 1024);
        TMemBuf& mbufIn = *janPool;

        SendOmniMsg(kHAIOmniTCPS::c1MsgType_SysInfoReq);
        WaitReply(0, kHAIOmniTCPS::c1MsgType_SysInfoReply, 0, mbufIn);
        StoreSysInfo(mbufIn);

        // Get the info on all the common stuff that's always there
        SendOmniMsg(kHAIOmniTCPS::c1MsgType_SysStatusReq);
        WaitReply(0, kHAIOmniTCPS::c1MsgType_SysStatusReply, 0, mbufIn);
        StoreSysStatus(mbufIn);

        //
        //  Now enable notifications. We have to do this BEFORE we poll all
        //  the info below, otherwise, something could change after we poll
        //  but before we enabled notifications, and we'd never know it.
        //
        SendOmniMsg(kHAIOmniTCPS::c1MsgType_EnableNotify, 1);
        WaitReply(0, kHAIOmniTCPS::c1MsgType_Ack, 0, mbufIn);

        //
        //  Do active polls of these things to get their initial values. We
        //  depend on async notifications afterwards to keep them up to date.
        //
        PollAreas();
// There's a problem with this one, so waiting for some info from HAI
//        PollEnclosures();
        PollThermos();
        PollUnits();
        PollZones();

        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
        {
            facHAIOmniTCPS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , L"Field were initialized successfully"
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );
        }
    }

    catch(const TError& errToCatch)
    {
        if (eVerboseLevel() > tCQCKit::EVerboseLvls::Medium)
            TModule::LogEventObj(errToCatch);
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


//
//  This is called to check that the Omni didn't send us a bad item number in
//  a status message response. We return the adjusted value for the index, so
//  that it can cover both bases.
//
tCIDLib::TCard4
THAIOmniTCPSDriver::c4CheckItemNum( const   tCIDLib::TCard2     c2Num
                                    , const tCIDLib::TCard4     c4MaxNum
                                    , const tCIDLib::TCh* const pszTypeName)
{
    if (!c2Num || (c2Num > c4MaxNum))
    {
        facHAIOmniTCPS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kOmniTCPSErrs::errcProto_BadItemNum
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , TString(pszTypeName)
            , TCardinal(c2Num)
            , TCardinal(c4MaxNum)
        );
    }
    return (c2Num - 1);
}


//
//  A helper to ask the Omni how many of a given type of object he has, since
//  we support more than one model.
//
tCIDLib::TCard4
THAIOmniTCPSDriver::c4QueryObjTypeMax(const tCIDLib::TCard1 c1ObjType)
{
    THeapBufPool::TElemJan janPool(&m_splBufs, 64);
    TMemBuf& mbufIn = *janPool;

    // Send an object capacity request and wait for the expected reply
    SendOmniMsg(kHAIOmniTCPS::c1MsgType_ObjCapReq, c1ObjType);
    WaitReply(0, kHAIOmniTCPS::c1MsgType_ObjCapRep, 0, mbufIn);

    // And pull out the result for return
    tCIDLib::TCard4 c4Ret;
    c4Ret = mbufIn[4];
    c4Ret <<= 8;
    c4Ret |= mbufIn[5];

    return c4Ret;
}


//
//  Translates our zone status number into the correct text value to send
//  out in a standard zone status event trigger.
//
const tCIDLib::TCh*
THAIOmniTCPSDriver::pszXlatZEvStatus(const tCIDLib::TCard4 c4Status) const
{
    const tCIDLib::TCh* pszRet = L"unknown";
    switch(c4Status)
    {
        case 0 :
            pszRet = L"secure";
            break;

        case 1 :
            pszRet = L"notready";
            break;

        case 2 :
            pszRet = L"violated";
            break;

        default :
            break;
    };
    return pszRet;
}


//
//  These are called to poll items. We try to reduce the overhead by chunking the
//  requests so that we never ask for more than can be returned in a single
//  message, but also so that we try not to ask for a lot of values for items
//  that aren't defined. We don't to do single items at a time, but we don't
//  want to poll a lot of data we don't need. So we do it in fairly reasonable
//  chunks.
//
//  This is called on connect to get the initial value of the thermos. After
//  that, we rely on async notifications.
//
tCIDLib::TVoid THAIOmniTCPSDriver::PollAreas()
{
    //
    //  If no areas configured, then nothing to do since we don't have any
    //  fields for them. This shouldn't happen, since any working system would
    //  have 1 area, but we cannot assume anything.
    //
    if (!m_c4CntAreas)
        return;

    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
    {
        facHAIOmniTCPS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"Polling area info"
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
    }

    // Get us a buffer to use
    THeapBufPool::TElemJan  janPool(&m_splBufs, 256);
    TMemBuf& mbufIn = *janPool;

    // Get the info and store it away
    SendObjStatusQuery
    (
        kHAIOmniTCPS::c1ItemType_Area, 1, tCIDLib::TCard2(m_c4MaxAreas)
    );
    WaitReply
    (
        0
        , kHAIOmniTCPS::c1MsgType_ExtObjStatusRep
        , kHAIOmniTCPS::c1ItemType_Area
        , mbufIn
    );
    StoreAreaStatus(mbufIn);

    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
    {
        facHAIOmniTCPS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"Area info poll complete"
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
    }
}


tCIDLib::TVoid THAIOmniTCPSDriver::PollEnclosures()
{
    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
    {
        facHAIOmniTCPS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"Polling enclosure info"
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
    }

    // Get us a buffer to use
    THeapBufPool::TElemJan  janPool(&m_splBufs, 256);
    TMemBuf& mbufIn = *janPool;

    SendObjStatusQuery
    (
        kHAIOmniTCPS::c1ItemType_ExpEnclosure, 1, tCIDLib::TCard2(m_c4MaxEnclosures)
    );
    WaitReply
    (
        0
        , kHAIOmniTCPS::c1MsgType_ExtObjStatusRep
        , kHAIOmniTCPS::c1ItemType_ExpEnclosure
        , mbufIn
    );
    StoreEnclosureStatus(mbufIn);

    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
    {
        facHAIOmniTCPS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"Enclosure poll complete"
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
    }
}


tCIDLib::TVoid THAIOmniTCPSDriver::PollThermos()
{
    // We ask for up to 4 thermos at a time
    static const tCIDLib::TCard4 c4BlockSz = 4;

    //
    //  If no thermos configured, then nothing to do since we don't have any
    //  fields for them.
    //
    if (!m_c4CntThermos)
        return;

    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
    {
        facHAIOmniTCPS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"Polling thermo info"
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
    }

    // Calc the blocks, accounting for a possible partial last one
    tCIDLib::TCard4 c4BlockCnt(m_c4MaxThermos / c4BlockSz);
    if (m_c4MaxThermos % c4BlockSz)
        c4BlockCnt++;

    // Get us a read buffer to use
    THeapBufPool::TElemJan  janPool(&m_splBufs, 512);
    TMemBuf& mbufIn = *janPool;

    //
    //  And now we know how many blocks we need to read, so go through them
    //  and poll any in which there are configured thermos.
    //
    for (tCIDLib::TCard4 c4BlockInd = 0; c4BlockInd < c4BlockCnt; c4BlockInd++)
    {
        // See if there are any thermos in this block
        tCIDLib::TCard4 c4SubInd = c4BlockInd * c4BlockSz;
        tCIDLib::TCard4 c4EndSubInd = c4SubInd + c4BlockSz;
        if (c4EndSubInd > m_c4MaxThermos)
            c4EndSubInd = m_c4MaxThermos;

        for (; c4SubInd < c4EndSubInd; c4SubInd++)
        {
            if (m_dcfgCurrent.pitemThermoAt(c4SubInd))
            {
                // There's a thermo in this block so poll it
                SendObjStatusQuery
                (
                    kHAIOmniTCPS::c1ItemType_Thermostat
                    , tCIDLib::TCard2(c4SubInd + 1)
                    , tCIDLib::TCard2(c4EndSubInd)
                );
                WaitReply
                (
                    0
                    , kHAIOmniTCPS::c1MsgType_ExtObjStatusRep
                    , kHAIOmniTCPS::c1ItemType_Thermostat
                    , mbufIn
                );
                StoreThermoStatus(mbufIn);
                break;
            }
        }
    }

    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
    {
        facHAIOmniTCPS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"Thermo poll complete"
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
    }
}


tCIDLib::TVoid THAIOmniTCPSDriver::PollUnits()
{
    // We can poll up to 8 units a time
    static const tCIDLib::TCard4 c4BlockSz = 8;

    // If no units configured, then nothing to do
    if (!m_c4CntUnits)
        return;

    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
    {
        facHAIOmniTCPS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"Polling unit info"
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
    }

    // Get us a buffer to use
    THeapBufPool::TElemJan  janPool(&m_splBufs, 512);
    TMemBuf& mbufIn = *janPool;

    //
    //  Now we can caculate how many blocks we need to do, accounting for a
    //  partial last one.
    //
    tCIDLib::TCard4 c4BlockCnt(m_c4MaxUnits / c4BlockSz);
    if (m_c4MaxUnits % c4BlockSz)
        c4BlockCnt++;

    for (tCIDLib::TCard4 c4BlockInd = 0; c4BlockInd < c4BlockCnt; c4BlockInd++)
    {
        // See if there are any Units in this block
        tCIDLib::TCard4 c4SubInd = c4BlockInd * c4BlockSz;
        tCIDLib::TCard4 c4EndSubInd = c4SubInd + c4BlockSz;
        if (c4EndSubInd > m_c4MaxUnits)
            c4EndSubInd = m_c4MaxUnits;

        for (; c4SubInd < c4EndSubInd; c4SubInd++)
        {
            if (m_dcfgCurrent.pitemUnitAt(c4SubInd))
            {
                SendObjStatusQuery
                (
                    kHAIOmniTCPS::c1ItemType_Unit
                    , tCIDLib::TCard2(c4SubInd + 1)
                    , tCIDLib::TCard2(c4EndSubInd)
                );
                WaitReply
                (
                    0
                    , kHAIOmniTCPS::c1MsgType_ExtObjStatusRep
                    , kHAIOmniTCPS::c1ItemType_Unit
                    , mbufIn
                );
                StoreUnitStatus(mbufIn);
                break;
            }
        }
    }

    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
    {
        facHAIOmniTCPS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"Unit poll complete"
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
    }
}


tCIDLib::TVoid THAIOmniTCPSDriver::PollZones()
{
    // We poll up to 8 zones at a time.
    const tCIDLib::TCard4 c4BlockSz = 8;

    // If no Zones, then nothing to do
    if (!m_c4CntZones)
        return;

    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
    {
        facHAIOmniTCPS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"Polling zone info"
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
    }

    // Get us buffer to use
    THeapBufPool::TElemJan  janPool(&m_splBufs, 512);
    TMemBuf& mbufIn = *janPool;

    //
    //  Now we can caculate how many blocks we need to do, accounting for a
    //  partial last one.
    //
    tCIDLib::TCard4 c4BlockCnt(m_c4MaxZones / c4BlockSz);
    if (m_c4MaxZones % c4BlockSz)
        c4BlockCnt++;

    for (tCIDLib::TCard4 c4BlockInd = 0; c4BlockInd < c4BlockCnt; c4BlockInd++)
    {
        // See if there are any configured zones in this block.
        tCIDLib::TCard4 c4SubInd = c4BlockInd * c4BlockSz;
        tCIDLib::TCard4 c4EndSubInd = c4SubInd + c4BlockSz;
        if (c4EndSubInd > m_c4MaxZones)
            c4EndSubInd = m_c4MaxZones;
        const tCIDLib::TCard4 c4StartInd = c4SubInd;
        for (; c4SubInd < c4EndSubInd; c4SubInd++)
        {
            if (m_dcfgCurrent.pitemZoneAt(c4SubInd))
            {
                SendObjStatusQuery
                (
                    kHAIOmniTCPS::c1ItemType_Zone
                    , tCIDLib::TCard2(c4SubInd + 1)
                    , tCIDLib::TCard2(c4EndSubInd)
                );
                WaitReply
                (
                    0
                    , kHAIOmniTCPS::c1MsgType_ExtObjStatusRep
                    , kHAIOmniTCPS::c1ItemType_Zone
                    , mbufIn
                );
                StoreZoneStatus(mbufIn);
                break;
            }
        }

        //
        //  See if any of them are temp or humidity zones. We always
        //  do the basic zone stuff above, but these are only done
        //  for the temp/hum type zones.
        //
        c4SubInd = c4StartInd;
        c4EndSubInd = c4SubInd + c4BlockSz;
        if (c4EndSubInd > m_c4MaxZones)
            c4EndSubInd = m_c4MaxZones;
        for (; c4SubInd < c4EndSubInd; c4SubInd++)
        {
            THAIOmniZone* pitemZone = m_dcfgCurrent.pitemZoneAt(c4SubInd);
            if (pitemZone)
            {
                const tHAIOmniTCPSh::EZoneTypes eZType(pitemZone->eZoneType());
                if ((eZType == tHAIOmniTCPSh::EZone_Humidity)
                ||  (eZType == tHAIOmniTCPSh::EZone_Temp))
                {
                    // There is a zone in this block so do it
                    SendObjStatusQuery
                    (
                        kHAIOmniTCPS::c1ItemType_AuxSensor
                        , tCIDLib::TCard2(c4SubInd + 1)
                        , tCIDLib::TCard2(c4EndSubInd)
                    );
                    WaitReply
                    (
                        0
                        , kHAIOmniTCPS::c1MsgType_ExtObjStatusRep
                        , kHAIOmniTCPS::c1ItemType_AuxSensor
                        , mbufIn
                    );
                    StoreAuxStatus(mbufIn);
                    break;
                }
            }
        }
    }

    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
    {
        facHAIOmniTCPS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"Zone poll complete"
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
    }
}


//
//  This is called if we get an async message as we are waiting for an explicit
//  reply.
//
tCIDLib::TVoid
THAIOmniTCPSDriver::ProcessAsync(const  tCIDLib::TCard1 c1Type
                                , const TMemBuf&        mbufMsg)
{
    switch(c1Type)
    {
        case kHAIOmniTCPS::c1MsgType_ExtObjStatusRep :
            //
            //  Based on the data type, we dispatch this one to the correct
            //  handler to extract the data.
            //
            //  NOTE:   He doesn't actually currently send all these, but it
            //          doesn't hurt to go ahead and set up for them. If they
            //          start sending them later, we won't need to update the
            //          driver.
            //
            switch(mbufMsg[3])
            {
                case kHAIOmniTCPS::c1ItemType_Zone :
                    StoreZoneStatus(mbufMsg);
                    break;

                case kHAIOmniTCPS::c1ItemType_Unit :
                    StoreUnitStatus(mbufMsg);
                    break;

                case kHAIOmniTCPS::c1ItemType_Area :
                    StoreAreaStatus(mbufMsg);
                    break;

                case kHAIOmniTCPS::c1ItemType_Thermostat :
                    StoreThermoStatus(mbufMsg);
                    break;

                case kHAIOmniTCPS::c1ItemType_Message :
                    break;

                case kHAIOmniTCPS::c1ItemType_AuxSensor :
                    StoreAuxStatus(mbufMsg);
                    break;

                case kHAIOmniTCPS::c1ItemType_ExpEnclosure :
                    StoreEnclosureStatus(mbufMsg);
                    break;

                default :
                    // Don't care about it
                    break;
            };
            break;


        case kHAIOmniTCPS::c1MsgType_OtherNotifications :
            //
            //  It's one of the 'other notifcations', of which we do deal
            //  with some of them, so call a method to handle it.
            //
            ProcessOtherNotification(mbufMsg);
            break;

        default :
            // Nothing we care about
            break;
    };
}


//
//  This is called from our poling callback, to handle any asyncs that might
//  have arrived.
//
tCIDLib::TVoid THAIOmniTCPSDriver::ProcessAsyncs()
{
    // See if any data is waiting. If not, nothing to do
    if (!m_sockOmni.bWaitForDataReadyMS(1))
        return;

    // Get us buffer to use
    THeapBufPool::TElemJan janPool(&m_splBufs, 1024);
    TMemBuf& mbufIn = *janPool;

    // Now read packets until we get nothing back, using a short timeout
    while (kCIDLib::True)
    {
        //
        //  Not we indicate we expect sequence number 0, which effectively
        //  turns off sequence number checking. We are not expecting anything
        //  other than asyncs here. If we were to see one, it would only be
        //  because we timed out during the periodic ping in the poll callback
        //  so there's a message left around. It'll just end up getting
        //  discarded.
        //
        tCIDLib::TCard1 c1PacketType;
        tCIDLib::TCard4 c4MsgBytes;
        tCIDLib::TBoolean bRes = bGetPacket
        (
            mbufIn
            , c1PacketType
            , c4MsgBytes
            , 5
            , 0
            , kCIDLib::False
            , kCIDLib::True
        );

        // If we got nothing this time, then we are done
        if (!bRes)
            break;

        // Else, we got something, so process it
        ProcessAsync(mbufIn[2], mbufIn);
    }
}


//
//  This gets called when we get an async message that's one of the 'other'
//  notification types, which is the grab bag message for the stuff that does
//  not warrant it's own status type message.
//
tCIDLib::TVoid
THAIOmniTCPSDriver::ProcessOtherNotification(const TMemBuf& mbufSrc)
{
    //
    //  We can have multiple events in the message, so we have to get the
    //  count of them, and loop through them.
    //
    const tCIDLib::TCard4 c4ValCnt = (mbufSrc[1] - 1) / 2;

    // OK, go through the list
    const tCIDLib::TCard1*  pc1Src = mbufSrc.pc1DataAt(3);
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ValCnt; c4Index++)
    {
        //
        //  The events are identified by a way overly convoluted encoding
        //  scheme, based on two bytes. So pull out the bytes, then break
        //  them out into their respective four nibbles, because most of
        //  the encoding is at the nibble or low level.
        //
        const tCIDLib::TCard1 c1High = *pc1Src;
        const tCIDLib::TCard1 c1Low = *(pc1Src + 1);
        const tCIDLib::TCard1 c1First = c1High >> 4;
        const tCIDLib::TCard1 c1Second = c1High & 0xF;
        const tCIDLib::TCard1 c1Third = c1Low >> 4;
        const tCIDLib::TCard1 c1Fourth = c1Low & 0xF;

        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::High)
        {
            facHAIOmniTCPS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , L"Other notification msg. Values=%(1), %(2)"
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
                , TCardinal(c1High, tCIDLib::ERadices::Hex)
                , TCardinal(c1Low, tCIDLib::ERadices::Hex)
            );
        }

        //
        //  If it's something that requires an event be sent, they'll store
        //  the values and set this flag and we'll send it at the end.
        //
        tCIDLib::TBoolean bSend = kCIDLib::False;

        //
        //  Do the top level switch on the first nibble. For each top level
        //  nibble's block, we'll further switch on the second nibble, and
        //  possible bits of the third inside that.
        //
        if (c1First == 0)
        {
            if (c1Second == 0)
            {
                // It's a button event, so send out a user action event
                m_strTmp1 = m_strEvButton;
                m_strTmp2.SetFormatted(c1Low);
                bSend = kCIDLib::True;

                if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
                {
                    facHAIOmniTCPS().LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , L"Processing button notifcation. Id=%(1)"
                        , tCIDLib::ESeverities::Status
                        , tCIDLib::EErrClasses::AppStatus
                        , m_strTmp2
                    );
                }
            }
             else if (c1Second == 2)
            {
                //
                //  It's an Omni alarm message for an area, so we need to
                //  store the new alarm status.
                //
                if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
                {
                    facHAIOmniTCPS().LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , L"Area alarm status notification for area %(1)"
                        , tCIDLib::ESeverities::Status
                        , tCIDLib::EErrClasses::AppStatus
                        , TCardinal(c1Fourth)
                    );
                }

                // If legal and configured, store the new info
                THAIOmniArea* pitemCur = m_dcfgCurrent.pitemAreaAt
                (
                    c4CheckItemNum(c1Fourth, m_c4MaxAreas, L"area")
                );
                if (pitemCur && (c1Third > 0) && (c1Third < 9))
                {
                    //
                    //  We have to get the current alarm bits out and OR this
                    //  one into it and write it back out.
                    //
                    tCIDLib::TCard4 c4Bits;
                    tCIDLib::TCard4 c4SerNum = 0;
                    bReadCardFld(pitemCur->c4FldIdAlarmBits(), c4SerNum, c4Bits);
                    c4Bits |= (0x1UL << (c1Third - 1));

                    StoreAlarmFields(*pitemCur, c4Bits);
                }
            }
             else if (c1Second == 3)
            {
                if ((c1Third & 0xE) == 0xE)
                {
                    // It's an X-10 all on/off command
                    // <TBD>...
                }
                 else if (c1Third == 0)
                {
                    // One of a bunch of events distinguished by the fourth
                }
            }
             else if ((c1Second & 0xC) == 0xC)
            {
                //
                //  It's an X-10 code received, so we send out an event that
                //  indicates the on/off and house/unit codes.
                //
                m_strTmp1 = m_strEvX10Code;
                m_strTmp2 = (c1High & 0x2) ? L"On" : L"Off";
                m_strTmp2.Append(kCIDLib::chSpace);
                m_strTmp2.Append(tCIDLib::TCh(kCIDLib::chLatin_A + c1Third));
                m_strTmp2.Append(kCIDLib::chPeriod);
                m_strTmp2.AppendFormatted(tCIDLib::TCard4(c1Fourth + 1));
                bSend = kCIDLib::True;

                if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
                {
                    facHAIOmniTCPS().LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , L"X-10 notifcation. Data=%(1)"
                        , tCIDLib::ESeverities::Status
                        , tCIDLib::EErrClasses::AppStatus
                        , m_strTmp2
                    );
                }
            }
        }
         else if (c1First == 0x7)
        {
            // We don't currently do anything with this one
        }
         else if (c1First == 0xF)
        {
            if ((c1Second & 0xC) == 0xC)
            {
                //
                //  It's a UPB link command, so we send out the link command
                //  and the link number in an event.
                //
                m_strTmp1 = m_strEvUPBLink;
                switch(tCIDLib::TCard4(c1High & 0x3))
                {
                    case 0 :
                        m_strTmp2 = L"Off";
                        break;

                    case 1 :
                        m_strTmp2 = L"On";
                        break;

                    case 2 :
                        m_strTmp2 = L"Set";
                        break;

                    case 3 :
                        m_strTmp2 = L"FStop";
                        break;

                    default :
                        break;
                };
                m_strTmp2.Append(kCIDLib::chComma);
                m_strTmp2.AppendFormatted(c1Low);
                bSend = kCIDLib::True;

                if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
                {
                    facHAIOmniTCPS().LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , L"UPB Link notification. Data=%(1)"
                        , tCIDLib::ESeverities::Status
                        , tCIDLib::EErrClasses::AppStatus
                        , m_strTmp2
                    );
                }
            }
        }
         else
        {
            //
            //  It has to be a security arming one, since it's the only one
            //  that won't be one of the above first nibbles. Get the area
            //  out and see if it's one we care about.
            //
            const tCIDLib::TCard2 c2Area = c1Second;
            if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
            {
                facHAIOmniTCPS().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , L"Security arming notification. Area=%(1)"
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::AppStatus
                    , TCardinal(c2Area)
                );
            }

            // If legal and configured, let's do it
            THAIOmniArea* pitemCur = m_dcfgCurrent.pitemAreaAt
            (
                c4CheckItemNum(c2Area, m_c4MaxAreas, L"area")
            );
            if (pitemCur)
            {
                tCIDLib::TCard4 c4ArmMode = c1First & 0x7;

                //
                //  If the high byte is on, and it's not an Off, then it's the
                //  'in the process of' version of the status, so add 6 to it.
                //
                if (c4ArmMode && (c1First & 0x8))
                    c4ArmMode += 6;

                // If it's not legal, store the unknown value
                if (c4ArmMode >= kHAIOmniTCPS::c4SecModeCnt)
                    c4ArmMode = tCIDLib::TCard1(kHAIOmniTCPS::c4SecModeCnt - 1);

                bStoreStringFld
                (
                    pitemCur->c4FldIdArmStatus()
                    , kHAIOmniTCPS::apszSecModes[c4ArmMode]
                    , kCIDLib::True
                );

                //
                //  If the mode is off, then clear the alarm stuff. We don't
                //  get any alarm notification above to indicate the alarm has
                //  gone away because of changing the alarm mode.
                //
                if (!c4ArmMode)
                    StoreAlarmFields(*pitemCur, 0);
            }
        }

        // If it was something we need to send a user action for, do that
        if (bSend)
        {
            QueueEventTrig
            (
                tCQCKit::EStdDrvEvs::UserAction
                , TString::strEmpty()
                , m_strTmp1
                , m_strTmp2
                , TString::strEmpty()
                , TString::strEmpty()
            );
        }
        pc1Src += 2;
    }
}


//
//  If we get a change for a zone, we need to send an event trigger, but
//  which one and what info depends on the zone type.
//
tCIDLib::TVoid
THAIOmniTCPSDriver::SendZoneTrig(const  TString&        strFld
                                , const tCIDLib::TCard1 c1Status
                                , const tCIDLib::TCard2 c2Num
                                , const THAIOmniZone&   itemZone)
{
    //
    //  Depending on the type, we send a different type of trigger, or
    //  in some cases none.
    //
    const tHAIOmniTCPSh::EZoneTypes eType = itemZone.eZoneType();

    if (eType == tHAIOmniTCPSh::EZone_Alarm)
    {
        QueueEventTrig
        (
            tCQCKit::EStdDrvEvs::ZoneAlarm
            , strFld
            , TString(pszXlatZEvStatus(c1Status & 0x3))
            , TString(TCardinal(c2Num))
            , itemZone.strName()
            , TString::strEmpty()
        );
    }
     else if (eType == tHAIOmniTCPSh::EZone_Motion)
    {
        QueueEventTrig
        (
            tCQCKit::EStdDrvEvs::Motion
            , strFld
            , facCQCKit().strBoolStartEnd((c1Status & 0x3) != 0)
            , TString(TCardinal(c2Num))
            , itemZone.strName()
            , TString::strEmpty()
        );
    }
}


//
//  For each area, there can one or more of a set of alarm types active. We
//  have a field where we provide the raw bitmap of those alarms. But we also
//  provide a string list field where we indicate the alarms in a list for
//  easy display to a human. This helper will take the raw bits and update
//  the string list field based on that. It needs to be done in too many
//  places so we don't want to replicate it.
//
//  And, since we have it, we also just store the bit mask field here as
//  well and the boolean 'any alarm is active' field, to keep redundancy
//  to a minimum.
//
//  NOTE: We assume the caller has alreayd locked for field access.
//
tCIDLib::TVoid
THAIOmniTCPSDriver::StoreAlarmFields(const  THAIOmniArea&   itemSrc
                                    , const tCIDLib::TCard4 c4Bits)
{
    // Store the raw alarm bits mask
    bStoreCardFld(itemSrc.c4FldIdAlarmBits(), c4Bits, kCIDLib::True);

    // And set the alarmed field if any alarms
    bStoreBoolFld(itemSrc.c4FldIdAlarmed(), c4Bits != 0, kCIDLib::True);

    // Update the alarm list based on the current bits
    m_colTmpList.RemoveAll();

    for (tCIDLib::TCard4 c4Index = 0; c4Index < kHAIOmniTCPS::c4AlarmTypeCnt; c4Index++)
    {
        if (c4Bits & (0x1UL << c4Index))
            m_colTmpList.objAdd(TString(kHAIOmniTCPS::apszAlarmTypes[c4Index]));
    }
    bStoreSListFld(itemSrc.c4FldIdAlarmList(), m_colTmpList, kCIDLib::True);
}


//
//  This guy is called with an area status message, to store the field data.
//
tCIDLib::TVoid THAIOmniTCPSDriver::StoreAreaStatus(const TMemBuf& mbufSrc)
{
    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
    {
        facHAIOmniTCPS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"Storing new area status info"
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
    }

    //
    //  Get the end point of the message out, so we can know when we are done.
    //  And get out the amount to skip for each object.
    //
    const tCIDLib::TCard4 c4MsgLen = mbufSrc[1];
    const tCIDLib::TCard1* pc1End = mbufSrc.pc1DataAt(c4MsgLen);
    const tCIDLib::TCard4 c4ObjLen = mbufSrc[4];

    // Lock and store the data
    {
        tCIDLib::TCard1         c1Val;
        tCIDLib::TCard2         c2Num;
        THAIOmniArea*           pitemCur;
        const tCIDLib::TCard1*  pc1Src = mbufSrc.pc1DataAt(5);

        // Loop until we hit the end of the message
        while (pc1Src < pc1End)
        {
            // Rememeber where the next one will be
            const tCIDLib::TCard1* pc1Next = pc1Src + c4ObjLen;

            // Get the object number out
            c2Num = *pc1Src++;
            c2Num <<= 8;
            c2Num |= *pc1Src++;

            // Validate the number

            pitemCur = m_dcfgCurrent.pitemAreaAt
            (
                c4CheckItemNum(c2Num, m_c4MaxAreas, L"area")
            );

            // If not configured, skip this one and move forward
            if (!pitemCur)
            {
                pc1Src = pc1Next;
                continue;
            }

            // Check the area arming mode
            c1Val = *pc1Src++;
            if (c1Val >= kHAIOmniTCPS::c4SecModeCnt)
                c1Val = tCIDLib::TCard1(kHAIOmniTCPS::c4SecModeCnt - 1);
            bStoreStringFld
            (
                pitemCur->c4FldIdArmStatus()
                , kHAIOmniTCPS::apszSecModes[c1Val]
                , kCIDLib::True
            );

            //
            //  Get the alarm bits out and store them. Save them for the
            //  next field before we move the pointer up.
            //
            const tCIDLib::TCard1 c1AlarmBits = *pc1Src++;
            StoreAlarmFields(*pitemCur, c1AlarmBits);

            // Move to the next one
            pc1Src = pc1Next;
        }
    }

    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
    {
        facHAIOmniTCPS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"Area status info stored"
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
    }
}


//
//  This guy is called with a zone aux message, to store the field data.
//
tCIDLib::TVoid THAIOmniTCPSDriver::StoreAuxStatus(const TMemBuf& mbufSrc)
{
    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
    {
        facHAIOmniTCPS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"Storing aux status info"
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
    }

    //
    //  Get the end point of the message out, so we can know when we are done.
    //  And get out the amount to skip for each object.
    //
    const tCIDLib::TCard4 c4MsgLen = mbufSrc[1];
    const tCIDLib::TCard1* pc1End = mbufSrc.pc1DataAt(c4MsgLen);
    const tCIDLib::TCard4 c4ObjLen = mbufSrc[4];

    // Lock and store the data
    {
        tCIDLib::TCard2         c2Num;
        tCIDLib::TInt4          i4Val;
        THAIOmniZone*           pitemCur;
        const tCIDLib::TCard1*  pc1Src = mbufSrc.pc1DataAt(5);

        // Loop until we hit the end of the message
        while (pc1Src < pc1End)
        {
            // Remember where the next one will start
            const tCIDLib::TCard1* pc1Next = pc1Src + c4ObjLen;

            // Get the object number out
            c2Num = *pc1Src++;
            c2Num <<= 8;
            c2Num |= *pc1Src++;

            // If not configured, or an alarm zone, skip it
            pitemCur = m_dcfgCurrent.pitemZoneAt
            (
                c4CheckItemNum(c2Num, m_c4MaxZones, L"zone aux")
            );

            // If not configured or it's an alarm, skip this one
            if (!pitemCur || (pitemCur->eZoneType() == tHAIOmniTCPSh::EZone_Alarm))
            {
                pc1Src = pc1Next;
                continue;
            }

            // Skip the output status field, then get the other three
            pc1Src++;
            const tCIDLib::TCard1 c1CurVal = *pc1Src++;
            const tCIDLib::TCard1 c1Low = *pc1Src++;
            const tCIDLib::TCard1 c1High = *pc1Src++;

            if (pitemCur->eZoneType() == tHAIOmniTCPSh::EZone_Temp)
            {
                i4Val = i4FromOmniTemp(c1CurVal, m_bDoCelsius, 0, 255);
                if (i4Val == kCIDLib::i4MaxInt)
                    SetFieldErr(pitemCur->c4FldIdCurVal(), kCIDLib::True);
                else
                    bStoreIntFld(pitemCur->c4FldIdCurVal(), i4Val, kCIDLib::True);

                i4Val = i4FromOmniTemp(c1Low, m_bDoCelsius, 44, 180);
                if (i4Val == kCIDLib::i4MaxInt)
                    SetFieldErr(pitemCur->c4FldIdLowSP(), kCIDLib::True);
                else
                    bStoreIntFld(pitemCur->c4FldIdLowSP(), i4Val, kCIDLib::True);

                i4Val = i4FromOmniTemp(c1High, m_bDoCelsius, 44, 180);
                if (i4Val == kCIDLib::i4MaxInt)
                    SetFieldErr(pitemCur->c4FldIdHighSP(), kCIDLib::True);
                else
                    bStoreIntFld(pitemCur->c4FldIdHighSP(), i4Val, kCIDLib::True);
            }
             else if (pitemCur->eZoneType() == tHAIOmniTCPSh::EZone_Humidity)
            {
                if (c1CurVal)
                    i4Val = i4FromOmniTemp(c1CurVal, kCIDLib::False, 44, 156);
                else
                    i4Val = 0;
                if ((i4Val < 0) || (i4Val > 100))
                    SetFieldErr(pitemCur->c4FldIdCurVal(), kCIDLib::True);
                else
                    bStoreIntFld(pitemCur->c4FldIdCurVal(), i4Val, kCIDLib::True);

                if (c1Low)
                    i4Val = i4FromOmniTemp(c1Low, kCIDLib::False, 44, 156);
                else
                    i4Val = 0;
                if ((i4Val < 0) || (i4Val > 100))
                    SetFieldErr(pitemCur->c4FldIdLowSP(), kCIDLib::True);
                else
                    bStoreIntFld(pitemCur->c4FldIdLowSP(), i4Val, kCIDLib::True);

                if (c1High)
                    i4Val = i4FromOmniTemp(c1High, kCIDLib::False, 44, 156);
                else
                    i4Val = 0;
                if ((i4Val < 0) || (i4Val > 100))
                    SetFieldErr(pitemCur->c4FldIdHighSP(), kCIDLib::True);
                else
                    bStoreIntFld(pitemCur->c4FldIdHighSP(), i4Val, kCIDLib::True);
            }

            // Move to the next one
            pc1Src = pc1Next;
        }
    }

    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
    {
        facHAIOmniTCPS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"Aux statis info stored"
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
    }
}


//
//  This guy is called with an expansion enclosure status message, to store the
//  field data.
//
tCIDLib::TVoid THAIOmniTCPSDriver::StoreEnclosureStatus(const TMemBuf& mbufSrc)
{
    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
    {
        facHAIOmniTCPS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"Storing enclosure status"
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
    }

    //
    //  Get the end point of the message out, so we can know when we are done.
    //  And get out the amount to skip for each object.
    //
    const tCIDLib::TCard4 c4MsgLen = mbufSrc[1];
    const tCIDLib::TCard1* pc1End = mbufSrc.pc1DataAt(c4MsgLen);
    const tCIDLib::TCard4 c4ObjLen = mbufSrc[4];

    // Lock and store the data
    {
        tCIDLib::TCard1         c1Val;
        tCIDLib::TCard2         c2Num;
        const tCIDLib::TCard1*  pc1Src = mbufSrc.pc1DataAt(5);

        // Loop until we hit the end of the message
        while (pc1Src < pc1End)
        {
            // REmember where the next one wills tart
            const tCIDLib::TCard1* pc1Next = pc1Src + c4ObjLen;

            // Get the enclosure number out
            c2Num = *pc1Src++;
            c2Num <<= 8;
            c2Num |= *pc1Src++;

            c4CheckItemNum(c2Num, tCIDLib::c4ArrayElems(m_ac4FldId_EnclACOff), L"enclosure");

            c2Num--;
            c1Val = *pc1Src++;
            bStoreBoolFld(m_ac4FldId_EnclACOff[c2Num], (c1Val & 0x1) != 0, kCIDLib::True);
            bStoreBoolFld(m_ac4FldId_EnclBattLow[c2Num], (c1Val & 0x2) != 0, kCIDLib::True);
            bStoreBoolFld(m_ac4FldId_EnclCommFail[c2Num], (c1Val & 0x80) != 0, kCIDLib::True);
            bStoreCardFld(m_ac4FldId_EnclBattery[c2Num], *pc1Src++, kCIDLib::True);

            // Move to the next one
            pc1Src = pc1Next;
        }
    }

    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
    {
        facHAIOmniTCPS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"Enclosure status stored"
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
    }
}


//
//  This guy is called with a system info message, to store the field data. It's
//  only called once on connect, to get some global info that won't change, and
//  to check the Omni model type, so that we can reject any models we don't
//  support.
//
tCIDLib::TVoid THAIOmniTCPSDriver::StoreSysInfo(const TMemBuf& mbufSrc)
{
    // If it's not an OmniPro or ProII, we don't support it
    if ((mbufSrc[3] != 16) && (mbufSrc[3] != 30))
    {
        facHAIOmniTCPS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kOmniTCPSErrs::errcDev_UnsupportedModel
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
        );
    }

    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
    {
        facHAIOmniTCPS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"Storing system info"
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
    }

    // Get the software version out and store it
    TString strVer;
    strVer.AppendFormatted(mbufSrc[4]);
    strVer.Append(L'.');
    strVer.AppendFormatted(mbufSrc[5]);
    tCIDLib::TInt1 i1Rev = tCIDLib::TInt1(mbufSrc[6]);
    if (i1Rev != 0)
    {
        if (i1Rev < 0)
        {
            strVer.Append(L'X');
            i1Rev *= -1;
        }
        strVer.Append(tCIDLib::TCh(L'A' + (i1Rev - 1)));
    }

    // Store the data
    bStoreStringFld(L"SoftwareVersion", strVer, kCIDLib::True);

    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
    {
        facHAIOmniTCPS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"System status stored"
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
    }
}


//
//  This guy is called with a system status message, to store the field data.
//  We don't actually use much out of this query. It has some area stuff in it,
//  but there's a separate area status query that get's us more and we use that
//  to store area info.
//
tCIDLib::TVoid THAIOmniTCPSDriver::StoreSysStatus(const TMemBuf& mbufSrc)
{
    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
    {
        facHAIOmniTCPS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"Storing system status info"
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
    }

    // Store the info
    bStoreCardFld(m_c4FldId_MainBattery, mbufSrc[16], kCIDLib::True);
    bStoreBoolFld(m_c4FldId_TimeValid, mbufSrc[3] != 0, kCIDLib::True);

    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
    {
        facHAIOmniTCPS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"System status info stored"
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
    }
}


//
//  This guy is called with a thermo status message, to store the field
//  data. We are told the low and high thermo ids that were asked for and
//  therefore which should be in the message.
//
tCIDLib::TVoid THAIOmniTCPSDriver::StoreThermoStatus(const TMemBuf& mbufSrc)
{
    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
    {
        facHAIOmniTCPS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"Storing thermo info"
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
    }

    //
    //  Get the end point of the message out, so we can know when we are done.
    //  And get out the amount to skip for each object.
    //
    const tCIDLib::TCard4 c4MsgLen = mbufSrc[1];
    const tCIDLib::TCard1* pc1End = mbufSrc.pc1DataAt(c4MsgLen);
    const tCIDLib::TCard4 c4ObjLen = mbufSrc[4];

    // Store the info
    {
        tCIDLib::TCard1 c1Val;
        tCIDLib::TCard2 c2Num;
        tCIDLib::TInt4  i4Val;
        THAIOmniThermo* pitemCur;
        const tCIDLib::TCard1*  pc1Src = mbufSrc.pc1DataAt(5);

        // Loop until we hit the end of the message
        while (pc1Src < pc1End)
        {
            // Remember where the next one will start
            const tCIDLib::TCard1* pc1Next = pc1Src + c4ObjLen;

            // Get the thermo number out
            c2Num = *pc1Src++;
            c2Num <<= 8;
            c2Num |= *pc1Src++;

            // Validate the number and get the thermo at this slot
            pitemCur = m_dcfgCurrent.pitemThermoAt
            (
                c4CheckItemNum(c2Num, m_c4MaxThermos, L"thermo")
            );

            // If not configured, ignore it
            if (!pitemCur)
            {
                pc1Src = pc1Next;
                continue;
            }

            // Skip the status byte
            pc1Src++;

            // Store the temp
            i4Val = i4FromOmniTemp(*pc1Src++, m_bDoCelsius, 0, 255);
            if (i4Val == kCIDLib::i4MaxInt)
                SetFieldErr(pitemCur->c4FldIdCurTemp(), kCIDLib::True);
            else
                bStoreIntFld(pitemCur->c4FldIdCurTemp(), i4Val, kCIDLib::True);

            // Store the heat SP
            i4Val = i4FromOmniTemp(*pc1Src++, m_bDoCelsius, 44, 180);
            if (i4Val == kCIDLib::i4MaxInt)
                SetFieldErr(pitemCur->c4FldIdHeatSP(), kCIDLib::True);
            else
                bStoreIntFld(pitemCur->c4FldIdHeatSP(), i4Val, kCIDLib::True);

            // Store the cool SP
            i4Val = i4FromOmniTemp(*pc1Src++, m_bDoCelsius, 44, 180);
            if (i4Val == kCIDLib::i4MaxInt)
                SetFieldErr(pitemCur->c4FldIdCoolSP(), kCIDLib::True);
            else
                bStoreIntFld(pitemCur->c4FldIdCoolSP(), i4Val, kCIDLib::True);

            // Store the mode
            c1Val = *pc1Src++;
            if (c1Val >= kHAIOmniTCPS::c4ThermoModeCnt)
                c1Val = kHAIOmniTCPS::c4ThermoModeCnt - 1;
            bStoreStringFld
            (
                pitemCur->c4FldIdMode()
                , kHAIOmniTCPS::apszThermoModes[c1Val]
                , kCIDLib::True
            );

            // Store the fan mode
            c1Val = *pc1Src++;
            if (c1Val >= kHAIOmniTCPS::c4ThermoFanModeCnt)
                c1Val = kHAIOmniTCPS::c4ThermoFanModeCnt - 1;
            bStoreStringFld
            (
                pitemCur->c4FldIdFanMode()
                , kHAIOmniTCPS::apszThermoFanModes[c1Val]
                , kCIDLib::True
            );

            // Store the hold status
            bStoreBoolFld
            (
                pitemCur->c4FldIdHoldStatus()
                , (*pc1Src++) != 0
                , kCIDLib::True
            );

            // Move to the next one
            pc1Src = pc1Next;
        }
    }

    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
    {
        facHAIOmniTCPS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"Thermo info stored"
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
    }
}


//
//  This guy is called with a unit status message, to store the field data.
//  We are told the low and high zone ides that were asked for and therefore
//  which should be in the message.
//
tCIDLib::TVoid THAIOmniTCPSDriver::StoreUnitStatus(const TMemBuf& mbufSrc)
{
    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
    {
        facHAIOmniTCPS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"Storing unit status info"
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
    }

    //
    //  Get the end point of the message out, so we can know when we are done.
    //  And get out the amount to skip for each object.
    //
    const tCIDLib::TCard4 c4MsgLen = mbufSrc[1];
    const tCIDLib::TCard1* pc1End = mbufSrc.pc1DataAt(c4MsgLen);
    const tCIDLib::TCard4 c4ObjLen = mbufSrc[4];

    // Store the data
    {
        tCIDLib::TCard1         c1Val;
        tCIDLib::TCard2         c2Num;
        THAIOmniUnit*           pitemCur;
        const tCIDLib::TCard1*  pc1Src = mbufSrc.pc1DataAt(5);

        // Loop until we hit the end of the message
        while (pc1Src < pc1End)
        {
            // Remember where the next one will start
            const tCIDLib::TCard1* pc1Next = pc1Src + c4ObjLen;

            // Get the unit number out
            c2Num = *pc1Src++;
            c2Num <<= 8;
            c2Num |= *pc1Src++;

            // Validate the number and get the unit at this slot
            pitemCur = m_dcfgCurrent.pitemUnitAt
            (
                c4CheckItemNum(c2Num, m_c4MaxUnits, L"unit")
            );

            // If not configured, ignore it
            if (!pitemCur)
            {
                pc1Src = pc1Next;
                continue;
            }

            c1Val = *pc1Src++;
            if (pitemCur->eUnitType() == tHAIOmniTCPSh::EUnit_Binary)
            {
                const tCIDLib::TBoolean bNewState = !((c1Val == 0) || (c1Val == 100));
                if (bStoreBoolFld(pitemCur->c4FldIdValue(), bNewState, kCIDLib::True)
                &&  pitemCur->bSendTrig())
                {
                    //
                    //  The value changed, so send a load change event trigger.
                    //
                    //  NOTE: This won't happen if it's a report being reflect
                    //  back to us due to us sending out a load change command.
                    //  The field value is already stored upon the Omni acking
                    //  the cmd, so in that case the trigger is sent in the
                    //  field write callback, not here. This only gets changes
                    //  caused by non-field writes (InvokeCmd field) or external
                    //  changes to load states.
                    //
                    TString strFld;
                    if (bQueryFldName(pitemCur->c4FldIdValue(), strFld))
                    {
                        QueueEventTrig
                        (
                            tCQCKit::EStdDrvEvs::LoadChange
                            , strFld
                            , facCQCKit().strBoolOffOn(bNewState)
                            , TString(TCardinal(pitemCur->c4ItemNum()))
                            , pitemCur->strName()
                            , TString::strEmpty()
                        );
                    }
                }
            }
             else if (pitemCur->eUnitType() == tHAIOmniTCPSh::EUnit_Flag)
            {
                bStoreCardFld(pitemCur->c4FldIdValue(), c1Val, kCIDLib::True);
            }
             else
            {
                if (c1Val == 1)
                    bStoreCardFld(pitemCur->c4FldIdValue(), 100, kCIDLib::True);
                else if ((c1Val >= 100) && (c1Val <= 200))
                    bStoreCardFld(pitemCur->c4FldIdValue(), c1Val - 100, kCIDLib::True);
                else if (c1Val == 0)
                    bStoreCardFld(pitemCur->c4FldIdValue(), 0, kCIDLib::True);
            }

            // Move to the enxt one
            pc1Src = pc1Next;
        }
    }

    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
    {
        facHAIOmniTCPS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"Unit status info stored"
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
    }
}



//
//  This guy is called with a zone status message, to store the field data.
//  We are told the low and high zone ides that were asked for and therefore
//  which should be in the message.
//
tCIDLib::TVoid THAIOmniTCPSDriver::StoreZoneStatus(const TMemBuf& mbufSrc)
{
    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
    {
        facHAIOmniTCPS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"Storing zone status info"
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
    }

    //
    //  Get the end point of the message out, so we can know when we are done.
    //  And get out the amount to skip for each object.
    //
    const tCIDLib::TCard4 c4MsgLen = mbufSrc[1];
    const tCIDLib::TCard1* pc1End = mbufSrc.pc1DataAt(c4MsgLen);
    const tCIDLib::TCard4 c4ObjLen = mbufSrc[4];

    // Store the info
    {
        tCIDLib::TCard2         c2Num;
        THAIOmniZone*           pitemCur;
        const tCIDLib::TCard1*  pc1Src = mbufSrc.pc1DataAt(5);

        while (pc1Src < pc1End)
        {
            // Remember where the next one will start
            const tCIDLib::TCard1* pc1Next = pc1Src + c4ObjLen;

            // Get the number out
            c2Num = *pc1Src++;
            c2Num <<= 8;
            c2Num |= *pc1Src++;

            // Validate the number and get the zone at this slow
            pitemCur = m_dcfgCurrent.pitemZoneAt
            (
                c4CheckItemNum(c2Num, m_c4MaxZones, L"zone")
            );

            // If not configured, ignore it
            if (!pitemCur)
            {
                pc1Src = pc1Next;
                continue;
            }

            const tCIDLib::TCard1 c1Status = *pc1Src++;
            const tCIDLib::TCard1 c1Analog = *pc1Src++;

            //
            //  We always store status. If a motion zone, it's a boolean,
            //  else a string.
            //
            tCIDLib::TBoolean bValChanged;
            if (pitemCur->eZoneType() == tHAIOmniTCPSh::EZone_Motion)
            {
                bValChanged = bStoreBoolFld
                (
                    pitemCur->c4FldIdStatus()
                    , (c1Status & 0x3) != 0
                    , kCIDLib::True
                );
            }
             else
            {
                bValChanged = bStoreStringFld
                (
                    pitemCur->c4FldIdStatus()
                    , kHAIOmniTCPS::apszZoneStatus[c1Status & 0x3]
                    , kCIDLib::True
                );
            }

            //
            //  If the value changed and this unit is configured to send
            //  send a trigger, then do so.
            //
            if (bValChanged && pitemCur->bSendTrig())
            {
                //
                //  The value changed, so send a standard event trigger of
                //  the type appropriate for zone type. Look up the field
                //  name first since it has to be part of the info.
                //
                TString strFld;
                if (bQueryFldName(pitemCur->c4FldIdStatus(), strFld))
                    SendZoneTrig(strFld, c1Status, c2Num, *pitemCur);
            }

            // If an alarm zone, store the regular zone stuff
            if (pitemCur->eZoneType() == tHAIOmniTCPSh::EZone_Alarm)
            {
                bStoreCardFld
                (
                    pitemCur->c4FldIdAnalog(), c1Analog, kCIDLib::True
                );

                bStoreStringFld
                (
                    pitemCur->c4FldIdLatched()
                    , kHAIOmniTCPS::apszZoneLatched[(c1Status >> 2) & 0x03]
                    , kCIDLib::True
                );

                bStoreStringFld
                (
                    pitemCur->c4FldIdArming()
                    , kHAIOmniTCPS::apszZoneArming[(c1Status >> 4) & 0x03]
                    , kCIDLib::True
                );
            }

            pc1Src = pc1Next;
        }
    }

    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
    {
        facHAIOmniTCPS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"Zone status info stored"
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
    }
}


