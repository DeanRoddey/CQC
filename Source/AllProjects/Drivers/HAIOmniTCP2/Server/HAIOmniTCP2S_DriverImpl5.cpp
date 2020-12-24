//
// FILE NAME: HAIOmniTCP2S_DriverImpl5.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/06/2014
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
//  This file contains some code related to getting configuration information from
//  the Omni. We can gather a fair bit of information to set up our configuration
//  automatically, or get a long way towards that goal. This greatly eases the
//  overhead of setting up the driver.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "HAIOmniTCP2S.hpp"



// ---------------------------------------------------------------------------
//  THAIOmniTCPSDriver: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  We can't assume that they will provide names that are valid CQC field names.
//
tCIDLib::TBoolean THAIOmniTCP2S::bNormalizeName(TString& strName)
{
    tCIDLib::TCard4 c4GoodCnt = 0;
    const tCIDLib::TCard4 c4Count = strName.c4Length();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (TRawStr::bIsAlphaNum(strName[c4Index]))
            c4GoodCnt++;
        else
            strName.PutAt(c4Index, kCIDLib::chUnderscore);
    }
    return (c4GoodCnt != 0);
}


//
//  Used by the import method below, this will enumerate all of the objects of a given
//  type, returning the names and ids.
//
tCIDLib::TCard4
THAIOmniTCP2S::c4EnumObjs(  const   tCIDLib::TCard1                 c1Type
                            ,       tCIDLib::TStrList&              colNames
                            ,       TFundVector<tCIDLib::TCard2>&   fcolIds
                            ,       TMemBuf&                        mbufToUse)
{
    const tCIDLib::TCard1 c1Reserved(1);

    colNames.RemoveAll();
    fcolIds.RemoveAll();

    tCIDLib::TBoolean   bMoreData = kCIDLib::True;
    tCIDLib::TCard2     c2Id = 0;
    TString             strName;
    while (bMoreData)
    {
        SendOmniMsg
        (
            kHAIOmniTCP2S::c1MsgType_ReadName
            , c1Type
            , tCIDLib::TCard1(c2Id >> 8)
            , tCIDLib::TCard1(c2Id & 0xFF)
            , c1Reserved
        );

        bMoreData = bWaitEnd
        (
            0, kHAIOmniTCP2S::c1MsgType_NameData, c1Type, mbufToUse
        );

        // If more data, process this one
        if (bMoreData)
        {
            // Get out the id of this one
            c2Id = mbufToUse[4];
            c2Id <<= 8;
            c2Id |= mbufToUse[5];

            // And get the name out
            strName = TString
            (
                reinterpret_cast<const tCIDLib::TSCh*>(mbufToUse.pc1DataAt(6))
            );
            if (bNormalizeName(strName))
            {
                // If this name isn't a dup, then we can take it
                tCIDLib::TCard4 c4Count = colNames.c4ElemCount();
                tCIDLib::TCard4 c4Index = 0;
                while (c4Index < c4Count)
                {
                    if (colNames[c4Index] == strName)
                        break;
                    c4Index++;
                }

                if (c4Index == c4Count)
                {
                    colNames.objAdd(strName);
                    fcolIds.c4AddElement(c2Id);
                }
            }
        }
    }

    return colNames.c4ElemCount();
}


//
//  This method will fill in a driver configuration object based on information
//  gotten from the Omni. Where it can get actual names of things it does, else
//  it sets them to defaults based on type and id.
//
tCIDLib::TVoid THAIOmniTCP2S::ImportOmniCfg(TOmniTCPDrvCfg& dcfgToFill)
{
    // Get us a buffer to use
    THeapBufPool::TElemJan janPool(&m_splBufs, 128);
    TMemBuf& mbufIn = *janPool;

    // Reset the config so we are starting from scratch
    dcfgToFill.Reset();

    //
    //  For each object type, we do a pass to enumerate them, to get their names
    //  and ids. If needed, we then go back and query more detailed info about
    //  them.
    //
    //  Once we have the info we need or can get, we add these objects to the cfg
    //  object we were given to fill in.
    //
    tCIDLib::TBoolean               bGotIt;
    tCIDLib::TCard4                 c4Count;
    tCIDLib::TStrList               colNames;
    TFundVector<tCIDLib::TCard2>    fcolIds;

    // Set up the trailing data bytes we send to the properties request
    tCIDLib::TCard1 ac1PropData[4] = { 0, 1, 0xFF, 0 };

    //
    //  Do the areas. In this case, if we get none, that may mean that they have only
    //  a single area, which means that they may not be able to name the area. So
    //  we'll just set up a default for area 1. And, even if the Omni gives us a
    //  default name, there may be no configured properties to get, so either way
    //  default in area 1.
    //
    c4Count = c4EnumObjs(kHAIOmniTCP2S::c1ItemType_Area, colNames, fcolIds, mbufIn);
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        SendOmniMsg
        (
            kHAIOmniTCP2S::c1MsgType_ObjPropReq
            , kHAIOmniTCP2S::c1ItemType_Area
            , fcolIds[c4Index]
            , ac1PropData
            , 4
        );

        bGotIt = bWaitEnd
        (
            0
            , kHAIOmniTCP2S::c1MsgType_ObjPropRep
            , kHAIOmniTCP2S::c1ItemType_Area
            , mbufIn
        );

        // If we got data, check that it's enabled, else just take it.
        if (!bGotIt || (bGotIt && mbufIn[10]))
            dcfgToFill.pitemAddArea(THAIOmniArea(colNames[c4Index], fcolIds[c4Index]));
    }

    if (!dcfgToFill.c4AreaCnt())
        dcfgToFill.pitemAddArea(THAIOmniArea(L"A1", 1));

    // Do the thermos
    c4Count = c4EnumObjs(kHAIOmniTCP2S::c1ItemType_Thermostat, colNames, fcolIds, mbufIn);
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        SendOmniMsg
        (
            kHAIOmniTCP2S::c1MsgType_ObjPropReq
            , kHAIOmniTCP2S::c1ItemType_Thermostat
            , fcolIds[c4Index]
            , ac1PropData
            , 4
        );

        bGotIt = bWaitEnd
        (
            0
            , kHAIOmniTCP2S::c1MsgType_ObjPropRep
            , kHAIOmniTCP2S::c1ItemType_Thermostat
            , mbufIn
        );

        // If it's not an unused type
        if (bGotIt && mbufIn[13])
        {
            // We can directly convert the OMni value to our enum
            tHAIOmniTCP2Sh::EThermoTypes eType = tHAIOmniTCP2Sh::EThermoTypes(mbufIn[13]);
            dcfgToFill.pitemAddThermo
            (
                THAIOmniThermo(colNames[c4Index], fcolIds[c4Index], eType)
            );
        }
    }


    // Do the units
    c4Count = c4EnumObjs(kHAIOmniTCP2S::c1ItemType_Unit, colNames, fcolIds, mbufIn);
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        SendOmniMsg
        (
            kHAIOmniTCP2S::c1MsgType_ObjPropReq
            , kHAIOmniTCP2S::c1ItemType_Unit
            , fcolIds[c4Index]
            , ac1PropData
            , 4
        );

        bGotIt = bWaitEnd
        (
            0
            , kHAIOmniTCP2S::c1MsgType_ObjPropRep
            , kHAIOmniTCP2S::c1ItemType_Unit
            , mbufIn
        );

        tHAIOmniTCP2Sh::EUnitTypes eType = tHAIOmniTCP2Sh::EUnitTypes::Unused;
        if (bGotIt)
        {
            switch(mbufIn[9])
            {
                case 1 :
                case 2 :
                case 3 :
                case 4 :
                case 6 :
                case 8 :
                case 9 :
                case 11 :
                    eType = tHAIOmniTCP2Sh::EUnitTypes::Dimmer;
                    break;

                case 12 :
                    eType = tHAIOmniTCP2Sh::EUnitTypes::Flag;
                    break;

                case 13 :
                    eType = tHAIOmniTCP2Sh::EUnitTypes::Switch;
                    break;

                default :
                    break;
            };
        }

        if (eType != tHAIOmniTCP2Sh::EUnitTypes::Unused)
            dcfgToFill.pitemAddUnit(THAIOmniUnit(colNames[c4Index], fcolIds[c4Index], eType));
    }


    // Do the zones
    c4Count = c4EnumObjs(kHAIOmniTCP2S::c1ItemType_Zone, colNames, fcolIds, mbufIn);
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        SendOmniMsg
        (
            kHAIOmniTCP2S::c1MsgType_ObjPropReq
            , kHAIOmniTCP2S::c1ItemType_Zone
            , fcolIds[c4Index]
            , ac1PropData
            , 4
        );

        bGotIt = bWaitEnd
        (
            0
            , kHAIOmniTCP2S::c1MsgType_ObjPropRep
            , kHAIOmniTCP2S::c1ItemType_Zone
            , mbufIn
        );

        tHAIOmniTCP2Sh::EZoneTypes eType = tHAIOmniTCP2Sh::EZoneTypes::Unused;
        if (bGotIt)
        {
            const tCIDLib::TCard1 c1Type = mbufIn[8];
            switch(c1Type)
            {
                case 0 :
                case 1 :
                case 6 :
                case 7 :
                case 8 :
                case 19 :
                case 20 :
                    eType = tHAIOmniTCP2Sh::EZoneTypes::Intrusion;
                    break;

                case 32 :
                case 33 :
                case 56 :
                    eType = tHAIOmniTCP2Sh::EZoneTypes::Fire;
                    break;

                case 54 :
                case 55 :
                    eType = tHAIOmniTCP2Sh::EZoneTypes::Water;
                    break;

                case 2 :
                case 3 :
                case 64 :
                    eType = tHAIOmniTCP2Sh::EZoneTypes::Motion;
                    break;

                case 82 :
                    eType = tHAIOmniTCP2Sh::EZoneTypes::Temp;
                    break;

                case 84 :
                    eType = tHAIOmniTCP2Sh::EZoneTypes::Humidity;
                    break;

                default:
                    // If not one of the above, make some others general alarms
                    if ((c1Type <= 56) || (c1Type == 83) || (c1Type == 87))
                        eType = tHAIOmniTCP2Sh::EZoneTypes::Alarm;
                    break;
            };
        }

        if (eType != tHAIOmniTCP2Sh::EZoneTypes::Unused)
        {
            dcfgToFill.pitemAddZone
            (
                THAIOmniZone(colNames[c4Index], fcolIds[c4Index], eType, mbufIn[9])
            );
        }
    }
}

