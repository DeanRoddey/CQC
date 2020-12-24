//
// FILE NAME: BarcoCRTS_DriverImpl2.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/13/2002
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
//  This file contains some of the overflow from the main driver implementation
//  file.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "BarcoCRTS.hpp"



// ---------------------------------------------------------------------------
//  TBarcoCRTS: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  This method provide the grunt work for sending a status query to the
//  projector and, if it replies, parsing the results and storing it.
//
tBarcoCRTS::EMsgResults TBarcoCRTSDriver::eDoStatusPoll(TThread& thrThis)
{
    tBarcoCRTS::EMsgResults eRes;

    // Send the pre-fab message to the projector
    if (!m_msgQueryStatus.bSendOn(m_pcommBarco))
    {
        IncFailedWriteCounter();
        return tBarcoCRTS::EMsgResults::SendFailed;
    }

    // And try to get a response
    eRes = m_msgPollReply.eReadShortFrom(m_pcommBarco, 4000);

    //
    //  If we got it, then lets parse out the information we care about. Else,
    //  lets update failure stats.
    //
    if (eRes == tBarcoCRTS::EMsgResults::Ok)
    {
        const tCIDLib::TBoolean bPowerState = (m_msgPollReply[2] & 0x1) == 0;

        // BE SURE we get unlocked before the sleep below!!
        tCIDLib::TBoolean bChanged = bStoreBoolFld
        (
            m_c4FldIdPower, bPowerState, kCIDLib::True
        );

        if (bChanged)
        {
            //
            //  Sleep for a while because the projector goes off into the
            //  ozone for a few seconds. We use a special sleep function here
            //  which is sensitive both to shutdown requests on the thread
            //  and to cleanup requests by the ORB. If it returns false, then
            //  we were interrupted, so we return false to get out as soon as
            //  possible.
            //
            if (!bSleep(3000))
                return tBarcoCRTS::EMsgResults::NoReply;
        }
    }

    if (eRes == tBarcoCRTS::EMsgResults::Ok)
    {
        //
        //  We just simulate the bits required, which are:
        //
        //      Dat1    - The horizontal period
        //      Dat2    - The source number
        //      Dat3    - The bit for the signal type
        //      Dat4    - The vertical frequency
        //
        tCIDLib::TCard4 c4Source = c4FldValue(m_c4FldIdInput);

        m_msgPollReply.Set
        (
            m_c1ProjAddr
            , tBarcoCRTS::ECmds::QueryCurSource
            , 10
            , tCIDLib::TCard1(c4Source)
            , 0x40
            , 72
        );
        eRes = tBarcoCRTS::EMsgResults::Ok;

        // If it worked, get the data out
        if (eRes == tBarcoCRTS::EMsgResults::Ok)
        {
            bStoreCardFld(m_c4FldIdHorzPeriod, m_msgPollReply[0], kCIDLib::True);
            if (m_msgPollReply[1] == 255)
                bStoreCardFld(m_c4FldIdInput, 0, kCIDLib::True);
            else
                bStoreCardFld(m_c4FldIdInput, m_msgPollReply[1], kCIDLib::True);
            bStoreCardFld(m_c4FldIdVertRate, m_msgPollReply[3], kCIDLib::True);

            // Translate the signal type to text format to store
            bStoreStringFld
            (
                m_c4FldIdSigType
                , pszXlatSigType(m_msgPollReply[2] >> 4)
                , kCIDLib::True
            );
        }
    }
    return eRes;
}


const tCIDLib::TCh*
TBarcoCRTSDriver::pszXlatSigType(const tCIDLib::TCard1 c1RawValue)
{
    //
    //  According to the projector type, use different lookup tables. We
    //  just go by the base projector type that we were configured for by
    //  the user.
    //
    tCIDLib::TCard4 c4Index;
    if (m_c4ProjBaseType == 700)
    {
        for (c4Index = 0; c4Index < tBarcoCRTS::c4SigTypeCount2; c4Index++)
        {
            if (tBarcoCRTS::aitemSigTypes2[c4Index].c1Value == c1RawValue)
                return tBarcoCRTS::aitemSigTypes2[c4Index].pszName;
        }
    }
     else
    {
        for (c4Index = 0; c4Index < tBarcoCRTS::c4SigTypeCount1; c4Index++)
        {
            if (tBarcoCRTS::aitemSigTypes1[c4Index].c1Value == c1RawValue)
                return tBarcoCRTS::aitemSigTypes1[c4Index].pszName;
        }
    }
    return L"Unknown";
}


