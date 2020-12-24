//
// FILE NAME: HAIOmniS_DriverImpl2.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/11/2006
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
//  to keep the main logic there and the helper stuff out of the way.
//  Because of the size of this driver, there is yet another overflow file
//  that contains the lowest level stuff.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "HAIOmniS.hpp"



// ---------------------------------------------------------------------------
//  THAIOmniSDriver: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  This is called after registering the fields, to get the initial values
//  into the fields.
//
tCIDLib::TBoolean THAIOmniSDriver::bInitFields()
{
    try
    {

        //
        //  This one gets some simple info, and we check that it's an OmniProII
        //  model, which is the only one we support.
        //
        SendOmniMsg(kHAIOmniS::c1MsgType_SysInfo);
        WaitReply(3000, kHAIOmniS::c1MsgType_SysInfoReply);
        StoreSysInfo();

        // Get the info on all the common stuff that's always there
        SendOmniMsg(kHAIOmniS::c1MsgType_SysStatus);
        WaitReply(3000, kHAIOmniS::c1MsgType_SysStatusReply);
        StoreSysStatus(kCIDLib::True);

        //
        //  If we have any defined items, then do an initial poll on them.
        //  We pass zero as the poll index, which tells them to poll them
        //  all in one pass.
        //
        PollThermos(kCIDLib::True);
        PollUnits(kCIDLib::True);
        PollZones(kCIDLib::True);
    }

    catch(const TError& errToCatch)
    {
        if (errToCatch.bCheckEvent( facHAIOmniS().strName()
                                    , kOmniSErrs::errcDev_UnsupportedModel))
        {
            //
            //  We don't support this model, so change the reconnect
            //  period up high so that we don't just keep doing a useless
            //  reconnect all the time.
            //
            SetPollTimes(150, 30000);
            TModule::LogEventObj(errToCatch);
        }
         else
        {
            if (eVerboseLevel() > tCQCKit::EVerboseLvls::Medium)
                TModule::LogEventObj(errToCatch);
        }
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


//
//  This is called to check that the Omni didn't send us a bad item number in
//  a status message response.
//
tCIDLib::TVoid
THAIOmniSDriver::CheckItemNum(  const   tCIDLib::TCard2     c2Num
                                , const tCIDLib::TCard4     c4MaxNum
                                , const tCIDLib::TCh* const pszTypeName)
{
    if (c2Num > c4MaxNum)
    {
        facHAIOmniS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kOmniSErrs::errcProto_BadItemNum
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , TString(pszTypeName)
            , TCardinal(c2Num)
            , TCardinal(c4MaxNum)
        );
    }
}


//
//  This is called from the poll method. It checks to see if any system events
//  have occured. If so, it sees if any of them are ones that we want to
//  react to.
//
tCIDLib::TVoid THAIOmniSDriver::CheckSysEvents()
{
    // Request any waiting events
    SendOmniMsg(kHAIOmniS::c1MsgType_ReqSysEvents);
    WaitReply(2500, kHAIOmniS::c1MsgType_SysEventsReply);

    // Update the next time to check for events
    m_enctNextEvent = TTime::enctNow() + (250 * kCIDLib::enctOneMilliSec);

    //
    //  There are two bytes per event, so the message type plus the events
    //  data is 2 * events + 1. So we get the message length value out, and
    //  subtract 1 and divide by 2 and that's the number of events we need
    //  to deal with. They are oldest to newest, and that's the order we want
    //  to process them in.
    //
    const tCIDLib::TCard4 c4Count = (m_mbufRead[kHAIOmniS::c4MsgLenOfs] - 1) / 2;
    if (!c4Count)
        return;

    // There can be more than one even, so we'll run up through the buffer
    const tCIDLib::TCard1*  pc1Src = m_mbufRead.pc1DataAt(3);
    TString                 strData;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
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
            facHAIOmniS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , L"System event. Values=%(1), %(2)"
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
                    facHAIOmniS().LogMsg
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
                    facHAIOmniS().LogMsg
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
                CheckItemNum(c1Fourth, m_fcolAreas.c4ElemCount(), L"area");
                THAIOmniArea* pitemCur
                (
                    static_cast<THAIOmniArea*>(m_fcolAreas[c1Fourth - 1])

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
                    facHAIOmniS().LogMsg
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
            // Not supported under this pre-2.16 firmware protocol
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
                facHAIOmniS().LogMsg
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
            CheckItemNum(c2Area, m_fcolAreas.c4ElemCount(), L"area");
            THAIOmniArea* pitemCur
            (
                static_cast<THAIOmniArea*>(m_fcolAreas[c2Area - 1])

            );

            if (pitemCur)
            {
                tCIDLib::TCard4 c4ArmMode = c1First & 0x7;

                //
                //  If the high byte is on, and it's not an Off, then it's the
                //  'in the process of' version of the status, so add 6 to it,
                //  because we are in the arming countdown.
                //
                const tCIDLib::TBoolean bInDelay(c4ArmMode && (c1First & 0x8));
                if (bInDelay)
                    c4ArmMode += 6;

                // If it's not legal, store the unknown value
                if (c4ArmMode >= kHAIOmniS::c4SecModeCnt)
                    c4ArmMode = tCIDLib::TCard1(kHAIOmniS::c4SecModeCnt - 1);

                bStoreBoolFld(pitemCur->c4FldIdInDelay(), bInDelay, kCIDLib::True);
                bStoreStringFld
                (
                    pitemCur->c4FldIdArmStatus()
                    , kHAIOmniS::apszSecModes[c4ArmMode]
                    , kCIDLib::True
                );

                // If the mode is off, then clear the alarm stuff
                if (!c4ArmMode)
                    StoreAlarmFields(*pitemCur, 0);

                //
                //  If changing modes, i.e. the (dis)arm completed, then we
                //  want to push off polling some stuff for a bit, so we
                //  force new times into their next poll times.
                //
                //  We'll keep polling events normally. We'll push back
                //  zone polling a bit, and unit and thermo polling a good
                //  bit. We already do system info slowly but make sure it
                //  doesn't kick off for the next five seconds.
                //
                if (c4ArmMode < 7)
                {
                    const tCIDLib::TEncodedTime enctCur = TTime::enctNow();
                    m_enctNextThermo = enctCur + (3 * kCIDLib::enctOneSecond);
                    m_enctNextUnit = enctCur + (3 * kCIDLib::enctOneSecond);
                    m_enctNextZone = enctCur + (2 * kCIDLib::enctOneSecond);

                    if ((enctCur >= m_enctNextSys)
                    ||  ((m_enctNextSys - enctCur) < (5 * kCIDLib::enctOneSecond)))
                    {
                        m_enctNextSys = enctCur + (5 * kCIDLib::enctOneSecond);
                    }
                }
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

        tCIDLib::TVoid QueueEventTrig
        (
            const   tCQCKit::EStdDrvEvs     eEvent
            , const TString&                strFld
            , const TString&                strVal1
            , const TString&                strVal2
            , const TString&                strVal3
            , const TString&                strVal4
        );


        pc1Src += 2;
    }
}


//
//  These find named items via their names, optionally throwing if the
//  name is not found, or just returning a null.
//
THAIOmniArea*
THAIOmniSDriver::pitemFindArea( const   TString&            strName
                                ,       tCIDLib::TBoolean   bThrowIfNot)
{
    if (m_c4CntAreas)
    {
        THAIOmniArea* pitemCur;
        for (tCIDLib::TCard4 c4Index = 0; c4Index < kHAIOmniSh::c4MaxAreas; c4Index++)
        {
            pitemCur = static_cast<THAIOmniArea*>(m_fcolAreas[c4Index]);
            if (!pitemCur)
                continue;

            if (pitemCur->strName() == strName)
                return pitemCur;
        }
    }

    if (bThrowIfNot)
    {
        facHAIOmniS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kOmniSErrs::errcFld_ItemNotFound
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , TString(L"area")
            , strName
        );
    }
    return 0;
}


THAIOmniUnit*
THAIOmniSDriver::pitemFindUnit( const   TString&            strName
                                ,       tCIDLib::TBoolean   bThrowIfNot)
{
    if (m_c4CntUnits)
    {
        THAIOmniUnit* pitemCur;
        for (tCIDLib::TCard4 c4Index = 0; c4Index < kHAIOmniSh::c4MaxUnits; c4Index++)
        {
            pitemCur = static_cast<THAIOmniUnit*>(m_fcolUnits[c4Index]);
            if (!pitemCur)
                continue;

            if (pitemCur->strName() == strName)
                return pitemCur;
        }
    }

    if (bThrowIfNot)
    {
        facHAIOmniS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kOmniSErrs::errcFld_ItemNotFound
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , TString(L"unit")
            , strName
        );
    }
    return 0;
}


THAIOmniZone*
THAIOmniSDriver::pitemFindZone( const   TString&            strName
                                ,       tCIDLib::TBoolean   bThrowIfNot)
{
    if (m_c4CntZones)
    {
        THAIOmniZone* pitemCur;
        for (tCIDLib::TCard4 c4Index = 0; c4Index < kHAIOmniSh::c4MaxZones; c4Index++)
        {
            pitemCur = static_cast<THAIOmniZone*>(m_fcolZones[c4Index]);
            if (!pitemCur)
                continue;

            if (pitemCur->strName() == strName)
                return pitemCur;
        }
    }

    if (bThrowIfNot)
    {
        facHAIOmniS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kOmniSErrs::errcFld_ItemNotFound
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , TString(L"zone")
            , strName
        );
    }
    return 0;
}



//
//  These find the item (of each supported item type) that owns the passed
//  field id, if any. If no item of that type owns the id, it returns a null
//  pointer.
//
THAIOmniThermo*
THAIOmniSDriver::pitemFindOwningThermo(const tCIDLib::TCard4 c4FldId) const
{
    if (m_c4CntThermos)
    {
        THAIOmniThermo* pitemCur;
        for (tCIDLib::TCard4 c4Index = 0; c4Index < kHAIOmniSh::c4MaxThermos; c4Index++)
        {
            pitemCur = static_cast<THAIOmniThermo*>(m_fcolThermos[c4Index]);
            if (!pitemCur)
                continue;

            if (pitemCur->bOwnsField(c4FldId))
                return pitemCur;
        }
    }
    return 0;
}


THAIOmniUnit*
THAIOmniSDriver::pitemFindOwningUnit(const tCIDLib::TCard4 c4FldId) const
{
    if (m_c4CntUnits)
    {
        THAIOmniUnit* pitemCur;
        for (tCIDLib::TCard4 c4Index = 0; c4Index < kHAIOmniSh::c4MaxUnits; c4Index++)
        {
            pitemCur = static_cast<THAIOmniUnit*>(m_fcolUnits[c4Index]);
            if (!pitemCur)
                continue;

            if (pitemCur->bOwnsField(c4FldId))
                return pitemCur;
        }
    }
    return 0;
}


//
//  These are called to poll named items. We try to reduce the overhead by
//  chunking the requests so that we never ask for more than can be returned
//  in a single message, but also so that we try not to ask for a lot of
//  values for items that aren't defined.
//
tCIDLib::TVoid THAIOmniSDriver::PollThermos(const tCIDLib::TBoolean bDoAll)
{
    static const tCIDLib::TCard4 c4BlockSz = 16;
    static const tCIDLib::TCard4 c4BlockCnt(kHAIOmniSh::c4MaxThermos / c4BlockSz);

    // If no thermos, then nothing to do
    if (m_c4CntThermos)
    {
        const tCIDLib::TCard4 c4EndBlk = bDoAll ? 0 : m_c4NextThermoBlk;
        if (bDoAll)
            m_c4NextThermoBlk = 0;

        tCIDLib::TCard4 c4BlocksDone = 0;
        do
        {
            // See if there are any thermos in this block
            tCIDLib::TCard4 c4SubInd = m_c4NextThermoBlk * c4BlockSz;
            tCIDLib::TCard4 c4EndSubInd = c4SubInd + c4BlockSz;

            for (; c4SubInd < c4EndSubInd; c4SubInd++)
            {
                if (m_fcolThermos[c4SubInd])
                {
                    // There's a thermo in this block so poll it
                    SendOmniMsg
                    (
                        kHAIOmniS::c1MsgType_ReqThermoStatus
                        , tCIDLib::TCard1(c4SubInd + 1)
                        , tCIDLib::TCard1(c4EndSubInd)
                    );
                    WaitReply(2500, kHAIOmniS::c1MsgType_ThermoReply);
                    StoreThermoStatus(c4SubInd + 1, c4EndSubInd);

                    c4BlocksDone++;
                    break;
                }
            }
            m_c4NextThermoBlk++;
            if (m_c4NextThermoBlk == c4BlockCnt)
                m_c4NextThermoBlk = 0;

            // If not doing them all and we've got a block, then we are done
            if (c4BlocksDone && !bDoAll)
                break;
        }   while (m_c4NextThermoBlk != c4EndBlk);
    }

    // Update the next thermo poll time
    m_enctNextThermo = TTime::enctNow() + (500 * kCIDLib::enctOneMilliSec);
}


tCIDLib::TVoid THAIOmniSDriver::PollUnits(const tCIDLib::TBoolean bDoAll)
{
    static const tCIDLib::TCard4 c4BlockSz = 32;
    static const tCIDLib::TCard4 c4BlockCnt((kHAIOmniSh::c4MaxUnits / c4BlockSz) + 1);

    // If no units, then nothing to do
    if (m_c4CntUnits)
    {
        const tCIDLib::TCard4 c4EndBlk = bDoAll ? 0 : m_c4NextUnitBlk;
        if (bDoAll)
            m_c4NextUnitBlk = 0;

        tCIDLib::TCard4 c4BlocksDone = 0;
        do
        {
            // See if there are any Units in this block
            tCIDLib::TCard4 c4SubInd = m_c4NextUnitBlk * c4BlockSz;
            tCIDLib::TCard4 c4EndSubInd = c4SubInd + c4BlockSz;
            if (c4EndSubInd > kHAIOmniSh::c4MaxUnits)
                c4EndSubInd = kHAIOmniSh::c4MaxUnits;

            for (; c4SubInd < c4EndSubInd; c4SubInd++)
            {
                if (m_fcolUnits[c4SubInd])
                {
                    //
                    //  There's a Unit in this block so poll it. There's a
                    //  special method for this, to handle unit numbers greater
                    //  than 255.
                    //
                    SendUnitReq(c4SubInd + 1, c4EndSubInd);
                    WaitReply(2500, kHAIOmniS::c1MsgType_UnitStatusReply);
                    StoreUnitStatus(c4SubInd + 1, c4EndSubInd);

                    c4BlocksDone++;
                    break;
                }
            }
            m_c4NextUnitBlk++;
            if (m_c4NextUnitBlk == c4BlockCnt)
                m_c4NextUnitBlk = 0;

            // If not doing them all and we've got a block, then we are done
            if (c4BlocksDone && !bDoAll)
                break;
        }   while (m_c4NextUnitBlk != c4EndBlk);
    }

    // Update the next unit poll time
    m_enctNextUnit = TTime::enctNow() + (400 * kCIDLib::enctOneMilliSec);
}


tCIDLib::TVoid THAIOmniSDriver::PollZones(const tCIDLib::TBoolean bDoAll)
{
    const tCIDLib::TCard4 c4BlockSz = 44;
    const tCIDLib::TCard4 c4BlockCnt(kHAIOmniSh::c4MaxZones / c4BlockSz);

    // If no Zones, then nothing to do
    if (m_c4CntZones)
    {
        const tCIDLib::TCard4 c4EndBlk = bDoAll ? 0 : m_c4NextZoneBlk;
        if (bDoAll)
            m_c4NextZoneBlk = 0;

        tCIDLib::TCard4 c4BlocksDone = 0;
        do
        {
            //
            //  See if there are any zones in this block. Not that, unfortunately,
            //  the 'secure' indicator is used even in temp/humidity zones, to
            //  indicate if the current value has exceeded the set points. So
            //  we have to poll the main zone info in all cases (though if it's
            //  a hum/temp zone we only store one of the values instead of all
            //  of them.
            //
            tCIDLib::TCard4 c4SubInd = m_c4NextZoneBlk * c4BlockSz;
            tCIDLib::TCard4 c4EndSubInd = c4SubInd + c4BlockSz;
            for (; c4SubInd < c4EndSubInd; c4SubInd++)
            {
                if (m_fcolZones[c4SubInd])
                {
                    SendOmniMsg
                    (
                        kHAIOmniS::c1MsgType_ZoneStatus
                        , tCIDLib::TCard1(c4SubInd + 1)
                        , tCIDLib::TCard1(c4EndSubInd)
                    );
                    WaitReply(2500, kHAIOmniS::c1MsgType_ZoneStatusReply);
                    StoreZoneStatus(c4SubInd + 1, c4EndSubInd);

                    c4BlocksDone++;
                    break;
                }
            }

            //
            //  See if any of them are temp or humidity zones. We always
            //  do the basic zone stuff above, but these are only done
            //  for the non-alarm type zones.
            //
            c4SubInd = m_c4NextZoneBlk * c4BlockSz;
            c4EndSubInd = c4SubInd + c4BlockSz;
            for (; c4SubInd < c4EndSubInd; c4SubInd++)
            {
                if (m_fcolZones[c4SubInd])
                {
                    THAIOmniZone* pitemZone = static_cast<THAIOmniZone*>
                    (
                        m_fcolZones[c4SubInd]
                    );

                    const tHAIOmniSh::EZoneTypes eZType(pitemZone->eZoneType());
                    if ((eZType == tHAIOmniSh::EZone_Humidity)
                    ||  (eZType == tHAIOmniSh::EZone_Temp))
                    {
                        // There is a zone in this block so do it
                        SendOmniMsg
                        (
                            kHAIOmniS::c1MsgType_ReqAuxStatus
                            , tCIDLib::TCard1(c4SubInd + 1)
                            , tCIDLib::TCard1(c4EndSubInd)
                        );
                        WaitReply(2500, kHAIOmniS::c1MsgType_AuxStatusReply);
                        StoreAuxStatus(c4SubInd + 1, c4EndSubInd);
                        break;
                    }
                }
            }

            m_c4NextZoneBlk++;
            if (m_c4NextZoneBlk == c4BlockCnt)
                m_c4NextZoneBlk = 0;

            // If not doing them all and we've got a block, then we are done
            if (c4BlocksDone && !bDoAll)
                break;
        }   while (m_c4NextZoneBlk != c4EndBlk);
    }

    // Set the next zone time
    m_enctNextZone = TTime::enctNow() + (300 * kCIDLib::enctOneMilliSec);
}


//
//  This is called to handle a write to the InvokeCmd field. We parse the
//  passed information and take the requested action.
//
//  AreaAllOff=aaa          - Turn off the units in the area. If aaa=$All$,
//                            then all units in all areas.
//  AreaAllOn=aaa           - Turn on all units in the numbered area. If
//                            aaa=$All$, then all units in all areas.
//  ArmArea=aaa,mmm,ccc     - Arm the named area with the named mode,
//                            using code ccc. If aaa=$All$, then all areas.
//  BypassZone=aaa,zzz,ccc  - Bypass zone named zzz, using code ccc from area
//                            aaa.
//  ClearAllMsgs=aaa        - Clears all messages in the indicated area. If
//                            aaa=$All$, then for all areas.
//  ClearMsg=mmm            - Clears the indicated message number mmm in all areas.
//  ExecButton=bbb          - Invoke the button number bbb
//  PhoneMsg=ddd,mmm        - Phones the indicated message number (the index of the
//                            configured phone number, not the literal one)
//                            and says the indicated message.
//  RestoreZone=aaa,zzz,ccc - Restore zone named zzz, using code ccc from area
//                            aaa.
//  SayMsg=mmm              - Say the message number mmm
//  ShowMsg=mmm             - Show the message number mmm
//  UnitPercent=uuu,ppp     - Sets unit uuu's level to ppp percent (0 to 100.)
//  UnitOff=uuu,ttt
//  UnitOn=uuu,ttt          - Turn off/on the unit named uuu for time ttt.
//                            ttt is interpreted as:
//                              0        : Off or on unconditionally
//                              1..99    : For 1 to 99 seconds
//                              101..199 : For 1 to 99 minutes
//                              201..218 : For 1 to 18 hours
//  UnitDim=uuu,iii,sss     - Increment or decrement the brightness of unit
//  UnitBri=uuu,iii,sss       uuu. The inc/dec value is 1 to 9, and represents
//                            a 10% adjustment up or down. The change occurs
//                            over sss seconds, pass zero for immediate change.
//                            sss is clipped to 99 seconds;
//  UPBLink=lll,sss         - Activates or deactivates UPB link lll. sss is
//                            the state, 1 for activate and 0 for deactivate.
//  UPBLEDOn=uuu,lll        - Sets the state of a UPB LED. LED lll (1 to 8) is
//  UPBLEDOff=uuu,lll         turned on or off for unit uuu.
//
tCIDLib::TVoid THAIOmniSDriver::ProcessUserCmd(const TString& strCmdStr)
{
    tCIDLib::TCard4 c4Count(0);
    TString         strCmd;
    TString         strP1;
    TString         strP2;
    TString         strP3;
    TString         strP4;

    TStringTokenizer stokCmd(&strCmdStr, L",=");

    // Get the command and the up to four parms
    if (stokCmd.bGetNextToken(strCmd))
    {
        c4Count++;
        if (stokCmd.bGetNextToken(strP1))
        {
            c4Count++;
            if (stokCmd.bGetNextToken(strP2))
            {
                c4Count++;
                if (stokCmd.bGetNextToken(strP3))
                {
                    c4Count++;
                    if (stokCmd.bGetNextToken(strP4))
                        c4Count++;
                }
            }
        }
    }

    // We had to at least get the command and one parm
    if (c4Count < 2)
    {
        facHAIOmniS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kOmniSErrs::errcFld_BadCmdFmt
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
        );
    }

    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Low)
    {
        facHAIOmniS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"Got user command"
            , strCmdStr
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
    }

    // Strip the whitespace off the parms
    strCmd.StripWhitespace();
    strP1.StripWhitespace();
    strP2.StripWhitespace();
    strP3.StripWhitespace();
    strP4.StripWhitespace();

    //
    //  Now, based on the command we interpret the paramaters and fill in
    //  the command and p1, p2 values.
    //
    tCIDLib::TCard1 c1Cmd;
    tCIDLib::TCard1 c1P1;
    tCIDLib::TCard2 c2P2;

    if ((strCmd == L"AreaAllOff") || (strCmd == L"AreaAllOn"))
    {
        if (strCmd == L"AreaAllOff")
            c1Cmd = kHAIOmniS::c1Cmd_AllOff;
        else
            c1Cmd = kHAIOmniS::c1Cmd_AllOn;
        c1P1 = 0;

        // Either all areas or a particular named area
        if (strP1 == "$All$")
            c2P2 = 0;
        else
            c2P2 = tCIDLib::TCard2(pitemFindArea(strP1, kCIDLib::True)->c4ItemNum());
    }
     else if ((strCmd == L"ArmArea")
          ||  (strCmd == L"BypassZone")
          ||  (strCmd == L"RestoreZone"))
    {
        // Either all areas or a particular named area
        if (strP1 == "$All$")
            c2P2 = 0;
        else
            c2P2 = tCIDLib::TCard2(pitemFindArea(strP1, kCIDLib::True)->c4ItemNum());

        // Validate the code
        if (!bValidateCode(strP3, c2P2, c1P1))
        {
            facHAIOmniS().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kOmniSErrs::errcFld_BadCode
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Authority
            );
        }

        if (strCmd == L"ArmArea")
        {
            //
            //  Set the security mode enum from the text value, and use the
            //  ordinal to offset the first command of the group of arm areas.
            //
            tCIDLib::TCard1 c1Mode = 0;
            while (c1Mode < kHAIOmniS::c4SecModeCnt - 1)
            {
                if (strP2 == kHAIOmniS::apszSecModes[c1Mode])
                    break;
                c1Mode++;
            }

            if (c1Mode == kHAIOmniS::c4SecModeCnt - 1)
            {
                // It cannot be 'unknown'
                facHAIOmniS().ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kOmniSErrs::errcFld_BadCmdParm
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Format
                    , TCardinal(1)
                );
            }

            // The command is the base disarm code plus the mode index
            c1Cmd = kHAIOmniS::c1Cmd_DisarmArea + c1Mode;
        }
         else
        {
            //
            //  Look up the indicated zone. In this case, we go back and
            //  overwrite P2 with the zone, since it's not needed anymore
            //  now that the code is validated.
            //
            c2P2 = tCIDLib::TCard2(pitemFindZone(strP2, kCIDLib::True)->c4ItemNum());

            if (strCmd == L"BypassZone")
                c1Cmd = kHAIOmniS::c1Cmd_BypassZone;
            else
                c1Cmd = kHAIOmniS::c1Cmd_RestoreZone;
        }
    }
     else if (strCmd == L"ClearAllMsgs")
    {
        c1Cmd = kHAIOmniS::c1Cmd_ClearMessage;
        c1P1 = 0;
        if (strP1 == L"$All$")
            c2P2 = 0;
        else
            c2P2 = tCIDLib::TCard2(pitemFindZone(strP1, kCIDLib::True)->c4ItemNum());
    }
     else if (strCmd == L"ClearMsg")
    {
        c1Cmd = kHAIOmniS::c1Cmd_ClearMessage;
        c1P1 = 0;
        c2P2 = tCIDLib::TCard2(strP1.c4Val(tCIDLib::ERadices::Dec));
    }
     else if (strCmd == L"ExecButton")
    {
        c1Cmd = kHAIOmniS::c1Cmd_ExecButton;
        c1P1 = 0;
        c2P2 = tCIDLib::TCard2(strP1.c4Val(tCIDLib::ERadices::Dec));
    }
     else if (strCmd == L"PhoneMsg")
    {
        c1Cmd = kHAIOmniS::c1Cmd_PhoneMessage;
        c1P1 = tCIDLib::TCard1(strP1.c4Val(tCIDLib::ERadices::Dec));
        c2P2 = tCIDLib::TCard2(strP2.c4Val(tCIDLib::ERadices::Dec));
    }
     else if (strCmd == L"SayMsg")
    {
        c1Cmd = kHAIOmniS::c1Cmd_SayMessage;
        c1P1 = 0;
        c2P2 = tCIDLib::TCard2(strP1.c4Val(tCIDLib::ERadices::Dec));
    }
     else if (strCmd == L"ShowMsg")
    {
        c1Cmd = kHAIOmniS::c1Cmd_ShowMessage;
        c1P1 = 0;
        c2P2 = tCIDLib::TCard2(strP1.c4Val(tCIDLib::ERadices::Dec));
    }
     else if (strCmd == L"UnitPercent")
    {
        c1Cmd = kHAIOmniS::c1Cmd_UnitLightLev;
        c1P1 = tCIDLib::TCard1(strP2.c4Val(tCIDLib::ERadices::Dec));
        if (c1P1 > 100)
            c1P1 = 100;
        c2P2 = tCIDLib::TCard2(pitemFindUnit(strP1, kCIDLib::True)->c4ItemNum());
    }
     else if ((strCmd == L"UnitOff") || (strCmd == L"UnitOn"))
    {
        if (strCmd == L"UnitOff")
            c1Cmd = kHAIOmniS::c1Cmd_UnitOff;
        else
            c1Cmd = kHAIOmniS::c1Cmd_UnitOn;
        c1P1 = tCIDLib::TCard1(strP2.c4Val(tCIDLib::ERadices::Dec));
        c2P2 = tCIDLib::TCard2(pitemFindUnit(strP1, kCIDLib::True)->c4ItemNum());
    }
     else if ((strCmd == L"UnitDim") || (strCmd == L"UnitBri"))
    {
        if (strCmd == L"UnitBri")
            c1Cmd = kHAIOmniS::c1Cmd_UnitBri;
        else
            c1Cmd = kHAIOmniS::c1Cmd_UnitDim;

        //
        //  Get the inc/dec amount and clip back if invalid, then add to the
        //  command.
        //
        c1P1 = tCIDLib::TCard1(strP2.c4Val(tCIDLib::ERadices::Dec));
        if (c1P1 > 9)
            c1P1 = 9;
        c1Cmd += c1P1;

        // Get the seconds and clip if needed
        c1P1 = tCIDLib::TCard1(strP3.c4Val(tCIDLib::ERadices::Dec));
        if (c1P1 > 99)
            c1P1 = 99;

        // And get the unit number
        c2P2 = tCIDLib::TCard2(pitemFindUnit(strP1, kCIDLib::True)->c4ItemNum());
    }
     else if ((strCmd == L"UPBLinkOff") || (strCmd == L"UPBLinkOn"))
    {
        if (strCmd == L"UPBLinkOff")
            c1Cmd = kHAIOmniS::c1Cmd_UPBLinkOff;
        else
            c1Cmd = kHAIOmniS::c1Cmd_UPBLinkOn;

        // Get the link number out. P1 is not used
        c1P1 = 0;
        c2P2 = tCIDLib::TCard2(strP1.c4Val(tCIDLib::ERadices::Dec));
    }
     else if ((strCmd == L"UPBLEDOn") || (strCmd == L"UPBLEDOff"))
    {
        if (strCmd == L"UPBLEDOn")
            c1Cmd = kHAIOmniS::c1Cmd_UPBLEDOn;
        else
            c1Cmd = kHAIOmniS::c1Cmd_UPBLEDOff;

        // Get the LED number out. If not 1 to 8, reject it
        c1P1 = tCIDLib::TCard1(strP2.c4Val(tCIDLib::ERadices::Dec));
        if (c1P1 > 8)
        {
            facHAIOmniS().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kOmniSErrs::errcFld_BadLEDNum
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Unknown
            );
        }

        // And get the unit number out
        c2P2 = tCIDLib::TCard2(pitemFindUnit(strP1, kCIDLib::True)->c4ItemNum());
    }
     else
    {
        facHAIOmniS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kOmniSErrs::errcFld_UnknownCmd
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Unknown
            , strCmd
        );
    }

    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
    {
        facHAIOmniS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"Sending user command"
            , strCmdStr
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
    }

    // Looks ok, so send the info we got
    SendOmniCmd(c1Cmd, c1P1, c2P2);
    bWaitAck(5000, kCIDLib::True);

    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Low)
    {
        facHAIOmniS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"User command acknowledged"
            , strCmdStr
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
    }
}


//
//  This is called on driver termination an each time we try to connect, to
//  release any previously set up named items and reset the associated data.
//
tCIDLib::TVoid THAIOmniSDriver::ReleaseNamedItems()
{
    for (tCIDLib::TCard4 c4Index = 0; c4Index < kHAIOmniSh::c4MaxAreas; c4Index++)
    {
        THAIOmniItem* pitemCur = m_fcolAreas[c4Index];
        if (pitemCur)
            delete pitemCur;
    }
    m_c4CntAreas = 0;
    m_fcolAreas.SetAll(0);

    for (tCIDLib::TCard4 c4Index = 0; c4Index < kHAIOmniSh::c4MaxThermos; c4Index++)
    {
        THAIOmniItem* pitemCur = m_fcolThermos[c4Index];
        if (pitemCur)
            delete pitemCur;
    }
    m_c4CntThermos = 0;
    m_fcolThermos.SetAll(0);
    m_c4ThermoHigh = 0;
    m_c4ThermoLow = kHAIOmniSh::c4MaxThermos;

    for (tCIDLib::TCard4 c4Index = 0; c4Index < kHAIOmniSh::c4MaxUnits; c4Index++)
    {
        THAIOmniItem* pitemCur = m_fcolUnits[c4Index];
        if (pitemCur)
            delete pitemCur;
    }
    m_c4CntUnits = 0;
    m_fcolUnits.SetAll(0);

    for (tCIDLib::TCard4 c4Index = 0; c4Index < kHAIOmniSh::c4MaxZones; c4Index++)
    {
        THAIOmniItem* pitemCur = m_fcolZones[c4Index];
        if (pitemCur)
            delete pitemCur;
    }
    m_c4CntZones = 0;
    m_fcolZones.SetAll(0);
    m_c4ZoneHigh = 0;
    m_c4ZoneLow = kHAIOmniSh::c4MaxZones;
}


//
//  This is called to register our fields. At this point, we've gotten
//  info on all the named items in the Omni and that info is stored away.
//
tCIDLib::TVoid THAIOmniSDriver::RegisterFields()
{
    //
    //  Dump any existing named item objects. We'll rebuild the list from
    //  the driver configuration data.
    //
    ReleaseNamedItems();

    tCIDLib::TCard4     c4CapInd;
    tCIDLib::TCard4     c4Index;
    TVector<TCQCFldDef> colFlds(64, kCQCKit::c4MaxDrvFields);
    TCQCFldDef          flddNew;
    TString             strLim1;
    TString             strLim2;
    TString             strLim3;
    TString             strLim4;
    TString             strLim5;
    TString             strFmt;
    TString             strFmt2;
    TString             strName;

    // Reset the first/last field id values
    m_c4FirstAreaFldId = kCIDLib::c4MaxCard;
    m_c4FirstThermoFldId = kCIDLib::c4MaxCard;
    m_c4FirstUnitFldId = kCIDLib::c4MaxCard;
    m_c4FirstZoneFldId = kCIDLib::c4MaxCard;
    m_c4LastAreaFldId = kCIDLib::c4MaxCard;
    m_c4LastThermoFldId = kCIDLib::c4MaxCard;
    m_c4LastUnitFldId = kCIDLib::c4MaxCard;
    m_c4LastZoneFldId = kCIDLib::c4MaxCard;

    m_c4CntAreas = 0;
    m_c4CntThermos = 0;
    m_c4CntUnits = 0;
    m_c4CntZones = 0;

    //
    //  We need current temp and h/c setpoint limits a number of times below,
    //  and they change depending on the temp scale chosen.
    //
    TString strTempLim;
    TString strSPLim;
    if (m_bDoCelsius)
    {
        strSPLim = L"Range: -18, 50";
        strTempLim = L"Range: -40, 87";
    }
     else
    {
        strSPLim = L"Range: 0, 122";
        strTempLim = L"Range: -40, 189";
    }

    // Set up some other common limits used below
    const TString strPerLim(L"Range:0, 100");
    const TString strFlagLim(L"Range:0, 255");

    //
    //  First do the ones that we know are always going to be there all
    //  the time.
    //
    flddNew.Set
    (
        L"InvokeCmd"
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Write
    );
    flddNew.bAlwaysWrite(kCIDLib::True);
    colFlds.objAdd(flddNew);

    flddNew.Set
    (
        L"MainBattery"
        , tCQCKit::EFldTypes::Card
        , tCQCKit::EFldAccess::Read
    );
    colFlds.objAdd(flddNew);

    flddNew.Set
    (
        L"SoftwareVersion"
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Read
    );
    colFlds.objAdd(flddNew);

    // Do the expansion enclosure fields
    strName = L"Encl";
    for (tCIDLib::TCard4 c4Index = 0; c4Index < kHAIOmniSh::c4MaxAreas; c4Index++)
    {
        strName.CapAt(4);
        strName.AppendFormatted(c4Index + 1);

        strName.Append(L"ACPowerOff");
        flddNew.Set(strName, tCQCKit::EFldTypes::Boolean, tCQCKit::EFldAccess::Read);
        colFlds.objAdd(flddNew);

        strName.CapAt(5);
        strName.Append(L"BatteryLow");
        flddNew.Set(strName, tCQCKit::EFldTypes::Boolean, tCQCKit::EFldAccess::Read);
        colFlds.objAdd(flddNew);

        strName.CapAt(5);
        strName.Append(L"CommFailure");
        flddNew.Set(strName, tCQCKit::EFldTypes::Boolean, tCQCKit::EFldAccess::Read);
        colFlds.objAdd(flddNew);

        strName.CapAt(5);
        strName.Append(L"BatteryLevel");
        flddNew.Set(strName, tCQCKit::EFldTypes::Card, tCQCKit::EFldAccess::Read);
        colFlds.objAdd(flddNew);
    }

    // Do the area status fields, one per each defined area
    {
        strLim1 = L"Enum: ";
        for (c4Index = 0; c4Index < kHAIOmniS::c4SecModeCnt; c4Index++)
        {
            if (c4Index)
                strLim1.Append(L",");
            strLim1.Append(kHAIOmniS::apszSecModes[c4Index]);
        }

        const tHAIOmniSh::TFlagArray& fcolAreas = m_dcfgCurrent.fcolAreas();
        const tCIDLib::TCard4 c4Count = fcolAreas.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 1; c4Index <= c4Count; c4Index++)
        {
            if (!fcolAreas[c4Index - 1])
                continue;

            m_c4CntAreas++;

            // Build the base name for this round and create the object
            c4CapInd = TOmniDrvCfg::c4MakeItemName(tHAIOmniSh::EItem_Area, c4Index, strName);
            m_fcolAreas[c4Index - 1] = new THAIOmniArea(strName, c4Index);

            // And create the fields for this area
            strName.CapAt(c4CapInd);
            strName.Append(L"_Alarmed");
            flddNew.Set(strName, tCQCKit::EFldTypes::Boolean, tCQCKit::EFldAccess::Read);
            colFlds.objAdd(flddNew);

            strName.CapAt(c4CapInd);
            strName.Append(L"_AlarmBits");
            flddNew.Set(strName, tCQCKit::EFldTypes::Card, tCQCKit::EFldAccess::Read);
            colFlds.objAdd(flddNew);

            strName.CapAt(c4CapInd);
            strName.Append(L"_AlarmList");
            flddNew.Set(strName, tCQCKit::EFldTypes::StringList, tCQCKit::EFldAccess::Read);
            colFlds.objAdd(flddNew);

            strName.CapAt(c4CapInd);
            strName.Append(L"_Status");
            flddNew.Set
            (
                strName
                , tCQCKit::EFldTypes::String
                , tCQCKit::EFldAccess::Read
                , strLim1
            );
            colFlds.objAdd(flddNew);

            strName.CapAt(c4CapInd);
            strName.Append(L"_InDelay");
            flddNew.Set(strName, tCQCKit::EFldTypes::Boolean, tCQCKit::EFldAccess::Read);
            colFlds.objAdd(flddNew);
        }
    }

    //
    //  Do the thermostat related fields for the thermostats. By doing all
    //  the items for a given thermo at a time, we insure that all the field
    //  ids for a given thermo are contiguous. THE ORDER IS IMPORTANT since
    //  we store first/last ids for a given thermo below when we look up the
    //  ids.
    //
    {
        strLim1 = L"Enum: ";
        for (c4Index = 0; c4Index < kHAIOmniS::c4ThermoFanModeCnt; c4Index++)
        {
            if (c4Index)
                strLim1.Append(L",");
            strLim1.Append(kHAIOmniS::apszThermoFanModes[c4Index]);
        }

        strLim2 = L"Enum: ";
        for (c4Index = 0; c4Index < kHAIOmniS::c4ThermoModeCnt; c4Index++)
        {
            if (c4Index)
                strLim2.Append(L",");
            strLim2.Append(kHAIOmniS::apszThermoModes[c4Index]);
        }

        const tHAIOmniSh::TFlagArray& fcolThermos = m_dcfgCurrent.fcolThermos();
        const tCIDLib::TCard4 c4Count = fcolThermos.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 1; c4Index <= kHAIOmniSh::c4MaxThermos; c4Index++)
        {
            if (!fcolThermos[c4Index - 1])
                continue;

            m_c4CntThermos++;

            // Build the base name for this round and create the new object
            c4CapInd = TOmniDrvCfg::c4MakeItemName(tHAIOmniSh::EItem_Thermo, c4Index, strName);
            m_fcolThermos[c4Index - 1] = new THAIOmniThermo(strName, c4Index);

            // Do the hold status temp
            strName.CapAt(c4CapInd);
            strName.Append(L"_Hold");
            flddNew.Set(strName, tCQCKit::EFldTypes::Boolean, tCQCKit::EFldAccess::ReadWrite);
            colFlds.objAdd(flddNew);

            // Do the current temp
            strName.CapAt(c4CapInd);
            strName.Append(L"_CurTemp");
            flddNew.Set
            (
                strName
                , tCQCKit::EFldTypes::Int
                , tCQCKit::EFldAccess::Read
                , strTempLim
            );
            colFlds.objAdd(flddNew);

            // Do the cool set point
            strName.CapAt(c4CapInd);
            strName.Append(L"_CoolSP");
            flddNew.Set
            (
                strName
                , tCQCKit::EFldTypes::Int
                , tCQCKit::EFldAccess::ReadWrite
                , strSPLim
            );
            colFlds.objAdd(flddNew);

            // Do the heat set point
            strName.CapAt(c4CapInd);
            strName.Append(L"_HeatSP");
            flddNew.Set
            (
                strName
                , tCQCKit::EFldTypes::Int
                , tCQCKit::EFldAccess::ReadWrite
                , strSPLim
            );
            colFlds.objAdd(flddNew);

            // Do the fan mode
            strName.CapAt(c4CapInd);
            strName.Append(L"_FanMode");
            flddNew.Set
            (
                strName
                , tCQCKit::EFldTypes::String
                , tCQCKit::EFldAccess::ReadWrite
                , strLim1
            );
            colFlds.objAdd(flddNew);

            // Do the thermo mode
            strName.CapAt(c4CapInd);
            strName.Append(L"_Mode");
            flddNew.Set
            (
                strName
                , tCQCKit::EFldTypes::String
                , tCQCKit::EFldAccess::ReadWrite
                , strLim2
            );
            colFlds.objAdd(flddNew);
        }
    }

    // Do the unit fields
    {
        const tHAIOmniSh::TPLCArray& fcolUnits = m_dcfgCurrent.fcolPLCUnits();
        const tCIDLib::TCard4 c4Count = fcolUnits.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 1; c4Index <= c4Count; c4Index++)
        {
            const tHAIOmniSh::EUnitTypes eType = fcolUnits[c4Index - 1];
            if (eType == tHAIOmniSh::EUnit_Unused)
                continue;

            m_c4CntUnits++;

            // Build the base name for this round and create the new object
            c4CapInd = TOmniDrvCfg::c4MakeItemName(tHAIOmniSh::EItem_Unit, c4Index, strName);
            THAIOmniUnit* pitemUnit = new THAIOmniUnit(strName, c4Index, eType);
            m_fcolUnits[c4Index - 1] = pitemUnit;

            // Do the value field
            if (eType == tHAIOmniSh::EUnit_Binary)
            {
                strName.Append(L"_State");
                flddNew.Set
                (
                    strName
                    , tCQCKit::EFldTypes::Boolean
                    , tCQCKit::EFldAccess::ReadWrite
                );
            }
             else
            {
                // It's either a dimer or flag
                const TString* pstrLim;
                if (eType == tHAIOmniSh::EUnit_Dimmer)
                    pstrLim = &strPerLim;
                else
                    pstrLim = &strFlagLim;

                strName.Append(L"_Value");
                flddNew.Set
                (
                    strName
                    , tCQCKit::EFldTypes::Card
                    , tCQCKit::EFldAccess::ReadWrite
                    , *pstrLim
                );
            }
            colFlds.objAdd(flddNew);
        }
    }


    //
    //  Do the zone status fields, for defined zones. By doing all the items
    //  for a given zone at a time, we insure that all the field ids for a
    //  given zone are contiguous. THE ORDER IS IMPORTANT since we store
    //  first/last ids for a given zone below when we look up the ids.
    //
    //  There can be 175 of these, though it's not actually likely to be
    //  anywhere near that. But we put the zone number into a 3 digit field.
    //
    {
        strLim1 = L"Range: 0, 255";
        strLim2 = L"Enum: ";
        for (c4Index = 0; c4Index < kHAIOmniS::c4ZoneArmingCnt; c4Index++)
        {
            if (c4Index)
                strLim2.Append(L",");
            strLim2.Append(kHAIOmniS::apszZoneArming[c4Index]);
        }
        strLim3 = L"Enum: ";
        for (c4Index = 0; c4Index < kHAIOmniS::c4ZoneLatchedCnt; c4Index++)
        {
            if (c4Index)
                strLim3.Append(L",");
            strLim3.Append(kHAIOmniS::apszZoneLatched[c4Index]);
        }

        strLim4 = L"Enum: ";
        for (c4Index = 0; c4Index < kHAIOmniS::c4ZoneStatusCnt; c4Index++)
        {
            if (c4Index)
                strLim4.Append(L",");
            strLim4.Append(kHAIOmniS::apszZoneStatus[c4Index]);
        }

        const tHAIOmniSh::TZoneArray& fcolZones = m_dcfgCurrent.fcolZones();
        const tCIDLib::TCard4 c4Count = fcolZones.c4ElemCount();
        for (c4Index = 1; c4Index <= kHAIOmniSh::c4MaxZones; c4Index++)
        {
            const tHAIOmniSh::EZoneTypes eType = fcolZones[c4Index - 1];
            if (eType == tHAIOmniSh::EZone_Unused)
                continue;

            m_c4CntZones++;

            // Build the base name for this round and create the new object
            c4CapInd = TOmniDrvCfg::c4MakeItemName(tHAIOmniSh::EItem_Zone, c4Index, strName);
            m_fcolZones[c4Index - 1] = new THAIOmniZone
            (
                strName, c4Index, fcolZones[c4Index - 1]
            );

            // The zone status is needed regardless of zone type
            strName.CapAt(c4CapInd);
            strName.Append(L"_Status");
            flddNew.Set
            (
                strName
                , tCQCKit::EFldTypes::String
                , tCQCKit::EFldAccess::Read
                , strLim4
            );
            colFlds.objAdd(flddNew);

            // If an alarm zone, then do the standard zone fields
            if (eType == tHAIOmniSh::EZone_Alarm)
            {
                // Do the analog loop value
                strName.CapAt(c4CapInd);
                strName.Append(L"_Analog");
                flddNew.Set
                (
                    strName
                    , tCQCKit::EFldTypes::Card
                    , tCQCKit::EFldAccess::Read
                    , strLim1
                );
                colFlds.objAdd(flddNew);

                // Do the zone arming status
                strName.CapAt(c4CapInd);
                strName.Append(L"_Arming");
                flddNew.Set
                (
                    strName
                    , tCQCKit::EFldTypes::String
                    , tCQCKit::EFldAccess::Read
                    , strLim2
                );
                colFlds.objAdd(flddNew);

                // Do the latched zone status
                strName.CapAt(c4CapInd);
                strName.Append(L"_Latched");
                flddNew.Set
                (
                    strName
                    , tCQCKit::EFldTypes::String
                    , tCQCKit::EFldAccess::Read
                    , strLim3
                );
                colFlds.objAdd(flddNew);
            }
             else
            {
                // It's a temp/humidity zone
                if (eType == tHAIOmniSh::EZone_Humidity)
                    strLim5 = strPerLim;
                else
                    strLim5 = strTempLim;
                strName.CapAt(c4CapInd);
                strName.Append(L"_CurVal");
                flddNew.Set
                (
                    strName
                    , tCQCKit::EFldTypes::Int
                    , tCQCKit::EFldAccess::Read
                    , strLim5
                );
                colFlds.objAdd(flddNew);

                if (eType == tHAIOmniSh::EZone_Humidity)
                    strLim5 = strPerLim;
                else
                    strLim5 = strSPLim;
                strName.CapAt(c4CapInd);
                strName.Append(L"_LowSP");
                flddNew.Set
                (
                    strName
                    , tCQCKit::EFldTypes::Int
                    , tCQCKit::EFldAccess::ReadWrite
                    , strLim5
                );
                colFlds.objAdd(flddNew);

                strName.CapAt(c4CapInd);
                strName.Append(L"_HighSP");
                flddNew.Set
                (
                    strName
                    , tCQCKit::EFldTypes::Int
                    , tCQCKit::EFldAccess::ReadWrite
                    , strLim5
                );
                colFlds.objAdd(flddNew);
            }
        }
    }

    //
    //  And now register our fields with the base class and go back andlook up
    //  their ids.
    //
    SetFields(colFlds);

    tCIDLib::TCard4 c4FldId1;
    tCIDLib::TCard4 c4FldId2;
    tCIDLib::TCard4 c4FldId3;
    tCIDLib::TCard4 c4FldId4;
    tCIDLib::TCard4 c4FldId5;
    tCIDLib::TCard4 c4FldId6;
    tCIDLib::TCard4 c4FldId7;

    m_c4FldId_InvokeCmd = pflddFind(L"InvokeCmd", kCIDLib::True)->c4Id();
    m_c4FldId_MainBattery = pflddFind(L"MainBattery", kCIDLib::True)->c4Id();


    //
    //  For all these, we'll do a 0 based index in the loops, sine we only
    //  need the 1 based to make the item name.
    //
    for (c4Index = 0; c4Index < kHAIOmniSh::c4MaxAreas; c4Index++)
    {
        THAIOmniArea* pitemCur = static_cast<THAIOmniArea*>(m_fcolAreas[c4Index]);
        if (!pitemCur)
            continue;

        // Build the base name for this round
        c4CapInd = TOmniDrvCfg::c4MakeItemName(tHAIOmniSh::EItem_Area, c4Index + 1, strName);

        strName.CapAt(c4CapInd);
        strName.Append(L"_Alarmed");
        pitemCur->c4FldIdAlarmed(pflddFind(strName, kCIDLib::True)->c4Id());

        strName.CapAt(c4CapInd);
        strName.Append(L"_AlarmBits");
        pitemCur->c4FldIdAlarmBits(pflddFind(strName, kCIDLib::True)->c4Id());

        strName.CapAt(c4CapInd);
        strName.Append(L"_AlarmList");
        pitemCur->c4FldIdAlarmList(pflddFind(strName, kCIDLib::True)->c4Id());

        strName.CapAt(c4CapInd);
        strName.Append(L"_Status");
        pitemCur->c4FldIdArmStatus(pflddFind(strName, kCIDLib::True)->c4Id());

        strName.CapAt(c4CapInd);
        strName.Append(L"_InDelay");
        pitemCur->c4FldIdInDelay(pflddFind(strName, kCIDLib::True)->c4Id());
    }

    strName = L"Encl";
    for (c4Index = 0; c4Index < kHAIOmniSh::c4MaxEnclosures; c4Index++)
    {
        strName.CapAt(4);
        strName.AppendFormatted(c4Index + 1);

        strName.Append(L"ACPowerOff");
        m_ac4FldId_EnclACOff[c4Index] = pflddFind(strName, kCIDLib::True)->c4Id();

        strName.CapAt(5);
        strName.Append(L"BatteryLow");
        m_ac4FldId_EnclBattLow[c4Index] = pflddFind(strName, kCIDLib::True)->c4Id();

        strName.CapAt(5);
        strName.Append(L"CommFailure");
        m_ac4FldId_EnclCommFail[c4Index] = pflddFind(strName, kCIDLib::True)->c4Id();

        strName.CapAt(5);
        strName.Append(L"BatteryLevel");
        m_ac4FldId_EnclBattery[c4Index] = pflddFind(strName, kCIDLib::True)->c4Id();
    }

    for (tCIDLib::TCard4 c4Index = 0; c4Index < kHAIOmniSh::c4MaxThermos; c4Index++)
    {
        THAIOmniThermo* pitemCur
                    = static_cast<THAIOmniThermo*>(m_fcolThermos[c4Index]);
        if (!pitemCur)
            continue;

        // Build the base name for this round
        c4CapInd = TOmniDrvCfg::c4MakeItemName(tHAIOmniSh::EItem_Thermo, c4Index + 1, strName);

        // Do the hold status temp
        strName.CapAt(c4CapInd);
        strName.Append(L"_Hold");
        c4FldId1 = pflddFind(strName, kCIDLib::True)->c4Id();

        // Do the current temp
        strName.CapAt(c4CapInd);
        strName.Append(L"_CurTemp");
        c4FldId2 = pflddFind(strName, kCIDLib::True)->c4Id();

        // Do the cool set point
        strName.CapAt(c4CapInd);
        strName.Append(L"_CoolSP");
        c4FldId3 = pflddFind(strName, kCIDLib::True)->c4Id();

        // Do the heat set point
        strName.CapAt(c4CapInd);
        strName.Append(L"_HeatSP");
        c4FldId4 = pflddFind(strName, kCIDLib::True)->c4Id();

        // Do the fan mode
        strName.CapAt(c4CapInd);
        strName.Append(L"_FanMode");
        c4FldId5 = pflddFind(strName, kCIDLib::True)->c4Id();

        // Do the thermo mode
        strName.CapAt(c4CapInd);
        strName.Append(L"_Mode");
        c4FldId6 = pflddFind(strName, kCIDLib::True)->c4Id();

        if (m_c4FirstThermoFldId == kCIDLib::c4MaxCard)
            m_c4FirstThermoFldId = c4FldId1;
        m_c4LastThermoFldId = c4FldId6;

        pitemCur->SetFieldIds
        (
            c4FldId3, c4FldId2, c4FldId5, c4FldId4, c4FldId1, c4FldId6
        );
    }

    for (tCIDLib::TCard4 c4Index = 0; c4Index < kHAIOmniSh::c4MaxUnits; c4Index++)
    {
        THAIOmniUnit* pitemCur = static_cast<THAIOmniUnit*>(m_fcolUnits[c4Index]);
        if (!pitemCur)
            continue;

        // Build the base name for this round
        c4CapInd = TOmniDrvCfg::c4MakeItemName(tHAIOmniSh::EItem_Unit, c4Index + 1, strName);

        if ((pitemCur->eType() == tHAIOmniSh::EUnit_Dimmer)
        ||  (pitemCur->eType() == tHAIOmniSh::EUnit_Flag))
        {
            strName.Append(L"_Value");
        }
         else
         {
            strName.Append(L"_State");
        }
        pitemCur->c4FldIdValue(pflddFind(strName, kCIDLib::True)->c4Id());

        if (m_c4FirstUnitFldId == kCIDLib::c4MaxCard)
            m_c4FirstUnitFldId = pitemCur->c4FldIdValue();
        m_c4LastUnitFldId = pitemCur->c4FldIdValue();
    }

    for (tCIDLib::TCard4 c4Index = 0; c4Index < kHAIOmniSh::c4MaxZones; c4Index++)
    {
        THAIOmniZone* pitemCur
                    = static_cast<THAIOmniZone*>(m_fcolZones[c4Index]);
        if (!pitemCur)
            continue;

        const tHAIOmniSh::EZoneTypes eZType = pitemCur->eZoneType();

        // Build the base name for this round
        c4CapInd = TOmniDrvCfg::c4MakeItemName(tHAIOmniSh::EItem_Zone, c4Index + 1, strName);

        // The zone status is used in all cases
        strName.CapAt(c4CapInd);
        strName.Append(L"_Status");
        c4FldId1 = pflddFind(strName, kCIDLib::True)->c4Id();

        // If an alarm zone, do the alarm orinete dfileds, else do the others
        if (eZType == tHAIOmniSh::EZone_Alarm)
        {
            c4FldId5 = kCIDLib::c4MaxCard;
            c4FldId6 = kCIDLib::c4MaxCard;
            c4FldId7 = kCIDLib::c4MaxCard;

            strName.CapAt(c4CapInd);
            strName.Append(L"_Analog");
            c4FldId2 = pflddFind(strName, kCIDLib::True)->c4Id();

            strName.CapAt(c4CapInd);
            strName.Append(L"_Arming");
            c4FldId3 = pflddFind(strName, kCIDLib::True)->c4Id();

            strName.CapAt(c4CapInd);
            strName.Append(L"_Latched");
            c4FldId4 = pflddFind(strName, kCIDLib::True)->c4Id();
        }
         else
        {
            c4FldId2 = kCIDLib::c4MaxCard;
            c4FldId3 = kCIDLib::c4MaxCard;
            c4FldId4 = kCIDLib::c4MaxCard;

            strName.CapAt(c4CapInd);
            strName.Append(L"_CurVal");
            c4FldId5 = pflddFind(strName, kCIDLib::True)->c4Id();

            strName.CapAt(c4CapInd);
            strName.Append(L"_LowSP");
            c4FldId6 = pflddFind(strName, kCIDLib::True)->c4Id();

            strName.CapAt(c4CapInd);
            strName.Append(L"_HighSP");
            c4FldId7 = pflddFind(strName, kCIDLib::True)->c4Id();
        }

        // If we've not stored the first zone field id yet, store it
        if (m_c4FirstZoneFldId == kCIDLib::c4MaxCard)
            m_c4FirstZoneFldId = c4FldId1;

        // Store the last zone field id so far, which depends on the type
        if (c4FldId7 != kCIDLib::c4MaxCard)
            m_c4LastZoneFldId = c4FldId7;
        else
            m_c4LastZoneFldId = c4FldId4;

        pitemCur->SetFieldIds
        (
            c4FldId2, c4FldId3, c4FldId5, c4FldId7, c4FldId4, c4FldId6, c4FldId1
        );
    }

    // Reset any polling related stuff now
    m_enctNextEvent = 0;
    m_enctNextSys = 0;
    m_enctNextThermo = 0;
    m_enctNextUnit = 0;
    m_enctNextZone = 0;
    m_c4NextThermoBlk = 0;
    m_c4NextUnitBlk = 0;
    m_c4NextZoneBlk = 0;
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
THAIOmniSDriver::StoreAlarmFields(  const   THAIOmniArea&   itemSrc
                                    , const tCIDLib::TCard4 c4Bits)
{
    // Store the raw alarm bits mask
    bStoreCardFld(itemSrc.c4FldIdAlarmBits(), c4Bits, kCIDLib::True);

    // And set the alarmed field if any alarms
    bStoreBoolFld(itemSrc.c4FldIdAlarmed(), c4Bits != 0, kCIDLib::True);

    // Update the alarm list based on the current bits
    m_colTmpList.RemoveAll();

    for (tCIDLib::TCard4 c4Index = 0; c4Index < kHAIOmniS::c4AlarmTypeCnt; c4Index++)
    {
        if (c4Bits & (0x1UL << c4Index))
            m_colTmpList.objAdd(TString(kHAIOmniS::apszAlarmTypes[c4Index]));
    }
    bStoreSListFld(itemSrc.c4FldIdAlarmList(), m_colTmpList, kCIDLib::True);
}


//
//  This guy is called with a zone aux message, to store the field data.
//  We are told the low and high zone ids that were asked for and therefore
//  which should be in the message.
//
tCIDLib::TVoid
THAIOmniSDriver::StoreAuxStatus(const  tCIDLib::TCard4 c4Low
                                , const tCIDLib::TCard4 c4High)
{
    THAIOmniZone*   pitemCur;
    tCIDLib::TInt4  i4Val;
    for (tCIDLib::TCard4 c4Index = c4Low; c4Index <= c4High; c4Index++)
    {
        pitemCur = static_cast<THAIOmniZone*>(m_fcolZones[c4Index - 1]);
        if (!pitemCur)
            continue;

        // If it's an alarm zone, skip it
        if (pitemCur->eZoneType() == tHAIOmniSh::EZone_Alarm)
            continue;

        const tCIDLib::TCard4 c4Ofs = 3 + ((c4Index - c4Low) * 4);
        const tCIDLib::TCard1 c1CurVal = m_mbufRead[c4Ofs + 1];
        const tCIDLib::TCard1 c1Low = m_mbufRead[c4Ofs + 2];
        const tCIDLib::TCard1 c1High = m_mbufRead[c4Ofs + 3];

        if (pitemCur->eZoneType() == tHAIOmniSh::EZone_Temp)
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
         else
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
    }
}


//
//  This guy is called with a system info message, to store the field data.
//
tCIDLib::TVoid THAIOmniSDriver::StoreSysInfo()
{
    // If it's not an OmniProII, we don't support it

//
//  This check was removed from this (now superceded) driver so that folks
//  with lesser panels, which seems to work ok, can use it.
//
#if 0
    if (m_mbufRead[3] != 16)
    {
        facHAIOmniS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kOmniSErrs::errcDev_UnsupportedModel
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
        );
    }
#endif

    // Get the software version out and store it
    TString strVer;
    strVer.AppendFormatted(m_mbufRead[4]);
    strVer.Append(L'.');
    strVer.AppendFormatted(m_mbufRead[5]);
    tCIDLib::TInt1 i1Rev = tCIDLib::TInt1(m_mbufRead[6]);
    if (i1Rev != 0)
    {
        if (i1Rev < 0)
        {
            strVer.Append(L'X');
            i1Rev *= -1;
        }
        strVer.Append(tCIDLib::TCh(L'A' + (i1Rev - 1)));
    }

    bStoreStringFld(L"SoftwareVersion", strVer, kCIDLib::True);
}


//
//  This guy is called with a system status message, to store the field data.
//  We are told whether this is an initial query to get the fields loaded, or
//  a later poll. We use notifications for some of the arming info instead of
//  polling info.
//
tCIDLib::TVoid
THAIOmniSDriver::StoreSysStatus(const tCIDLib::TBoolean bInitPoll)
{
    bStoreCardFld(m_c4FldId_MainBattery, m_mbufRead[16], kCIDLib::True);

    tCIDLib::TCard1 c1Val;
    tCIDLib::TCard4 c4Ofs;
    THAIOmniArea*   pitemCur;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < kHAIOmniSh::c4MaxAreas; c4Index++)
    {
        // See if this area is defined
        pitemCur = static_cast<THAIOmniArea*>(m_fcolAreas[c4Index]);
        if (pitemCur)
        {
            // It is so store the data
            c1Val = m_mbufRead[17 + c4Index];

            //
            //  If not in the range of known values, use the final 'Unknown'
            //  value. This guy only reports the basic arming mode, not the
            //  in progress modes, so the max value is 6 here, not the number
            //  of security modes we define in the mapping table.
            //
            if (c1Val > 6)
                c1Val = tCIDLib::TCard1(kHAIOmniS::c4SecModeCnt - 1);

            if (bInitPoll)
            {
                //
                //  We'll only get the basic arming status here, i.e. we won't
                //  get any of the 'ing' type in process arming statuses. Those
                //  only come in via events.
                //
                bStoreStringFld
                (
                    pitemCur->c4FldIdArmStatus()
                    , kHAIOmniS::apszSecModes[c1Val]
                    , kCIDLib::True
                );
            }
        }

        c4Ofs = 25 + (c4Index * 2);
        c1Val = m_mbufRead[c4Ofs];

        bStoreBoolFld(m_ac4FldId_EnclACOff[c4Index], (c1Val & 0x1) != 0, kCIDLib::True);
        bStoreBoolFld(m_ac4FldId_EnclBattLow[c4Index], (c1Val & 0x2) != 0, kCIDLib::True);
        bStoreBoolFld(m_ac4FldId_EnclCommFail[c4Index], (c1Val & 0x80) != 0, kCIDLib::True);

        c1Val = m_mbufRead[c4Ofs + 1];
        bStoreCardFld(m_ac4FldId_EnclBattery[c4Index], c1Val, kCIDLib::True);
    }

    // Update the next system status poll time
    m_enctNextSys = TTime::enctNow() + (15 * kCIDLib::enctOneSecond);
}


//
//  This guy is called with a thermo status message, to store the field
//  data. We are told the low and high thermo ids that were asked for and
//  therefore which should be in the message.
//
tCIDLib::TVoid
THAIOmniSDriver::StoreThermoStatus( const   tCIDLib::TCard4 c4Low
                                    , const tCIDLib::TCard4 c4High)
{
    tCIDLib::TCard1 c1Val;
    tCIDLib::TInt4  i4Val;
    THAIOmniThermo* pitemCur;
    for (tCIDLib::TCard4 c4Index = c4Low; c4Index <= c4High; c4Index++)
    {
        pitemCur = static_cast<THAIOmniThermo*>(m_fcolThermos[c4Index - 1]);
        if (!pitemCur)
            continue;

        const tCIDLib::TCard4 c4Ofs = 3 + ((c4Index - c4Low) * 7);

        i4Val = i4FromOmniTemp(m_mbufRead[c4Ofs + 1], m_bDoCelsius, 0, 255);
        if (i4Val == kCIDLib::i4MaxInt)
            SetFieldErr(pitemCur->c4FldIdCurTemp(), kCIDLib::True);
        else
            bStoreIntFld(pitemCur->c4FldIdCurTemp(), i4Val, kCIDLib::True);

        i4Val = i4FromOmniTemp(m_mbufRead[c4Ofs + 2], m_bDoCelsius, 44, 180);
        if (i4Val == kCIDLib::i4MaxInt)
            SetFieldErr(pitemCur->c4FldIdHeatSP(), kCIDLib::True);
        else
            bStoreIntFld(pitemCur->c4FldIdHeatSP(), i4Val, kCIDLib::True);

        i4Val = i4FromOmniTemp(m_mbufRead[c4Ofs + 3], m_bDoCelsius, 44, 180);
        if (i4Val == kCIDLib::i4MaxInt)
            SetFieldErr(pitemCur->c4FldIdCoolSP(), kCIDLib::True);
        else
            bStoreIntFld(pitemCur->c4FldIdCoolSP(), i4Val, kCIDLib::True);

        c1Val = m_mbufRead[c4Ofs + 4];
        if (c1Val >= kHAIOmniS::c4ThermoModeCnt)
            c1Val = kHAIOmniS::c4ThermoModeCnt - 1;
        bStoreStringFld
        (
            pitemCur->c4FldIdMode()
            , kHAIOmniS::apszThermoModes[c1Val]
            , kCIDLib::True
        );

        c1Val = m_mbufRead[c4Ofs + 5];
        if (c1Val >= kHAIOmniS::c4ThermoFanModeCnt)
            c1Val = kHAIOmniS::c4ThermoFanModeCnt - 1;
        bStoreStringFld
        (
            pitemCur->c4FldIdFanMode()
            , kHAIOmniS::apszThermoFanModes[c1Val]
            , kCIDLib::True
        );

        bStoreBoolFld
        (
            pitemCur->c4FldIdHoldStatus()
            , m_mbufRead[c4Ofs + 6] != 0
            , kCIDLib::True
        );
    }
}


//
//  This guy is called with a unit status message, to store the field data.
//  We are told the low and high zone ides that were asked for and therefore
//  which should be in the message.
//
tCIDLib::TVoid
THAIOmniSDriver::StoreUnitStatus(const  tCIDLib::TCard4 c4Low
                                , const tCIDLib::TCard4 c4High)
{
    THAIOmniUnit* pitemCur;
    for (tCIDLib::TCard4 c4Index = c4Low; c4Index <= c4High; c4Index++)
    {
        pitemCur = static_cast<THAIOmniUnit*>(m_fcolUnits[c4Index - 1]);
        if (!pitemCur)
            continue;

        const tCIDLib::TCard4 c4Ofs = 3 + ((c4Index - c4Low) * 3);
        const tCIDLib::TCard1 c1Val = m_mbufRead[c4Ofs];

        if (pitemCur->eType() == tHAIOmniSh::EUnit_Binary)
        {
            if ((c1Val == 0) || (c1Val == 100))
                bStoreBoolFld(pitemCur->c4FldIdValue(), kCIDLib::False, kCIDLib::True);
            else
                bStoreBoolFld(pitemCur->c4FldIdValue(), kCIDLib::True, kCIDLib::True);
        }
         else if (pitemCur->eType() == tHAIOmniSh::EUnit_Flag)
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
    }
}



//
//  This guy is called with a zone status message, to store the field data.
//  We are told the low and high zone ides that were asked for and therefore
//  which should be in the message.
//
tCIDLib::TVoid
THAIOmniSDriver::StoreZoneStatus(const  tCIDLib::TCard4 c4Low
                                , const tCIDLib::TCard4 c4High)
{
    THAIOmniZone* pitemCur;
    for (tCIDLib::TCard4 c4Index = c4Low; c4Index <= c4High; c4Index++)
    {
        pitemCur = static_cast<THAIOmniZone*>(m_fcolZones[c4Index - 1]);
        if (!pitemCur)
            continue;

        const tCIDLib::TCard4 c4Ofs = 3 + ((c4Index - c4Low) * 2);
        const tCIDLib::TCard1 c1Status = m_mbufRead[c4Ofs];
        const tCIDLib::TCard1 c1Analog = m_mbufRead[c4Ofs + 1];

        // We always store status
        bStoreStringFld
        (
            pitemCur->c4FldIdStatus()
            , kHAIOmniS::apszZoneStatus[c1Status & 0x3]
            , kCIDLib::True
        );

        // If an alarm zone, store the regular zone stuff
        if (pitemCur->eZoneType() == tHAIOmniSh::EZone_Alarm)
        {
            bStoreCardFld
            (
                pitemCur->c4FldIdAnalog()
                , m_mbufRead[c4Ofs + 1]
                , kCIDLib::True
            );

            bStoreStringFld
            (
                pitemCur->c4FldIdLatched()
                , kHAIOmniS::apszZoneLatched[(c1Status >> 2) & 0x03]
                , kCIDLib::True
            );

            bStoreStringFld
            (
                pitemCur->c4FldIdArming()
                , kHAIOmniS::apszZoneArming[(c1Status >> 4) & 0x03]
                , kCIDLib::True
            );
        }
    }
}


