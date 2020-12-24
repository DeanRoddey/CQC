//
// FILE NAME: HAIOmniTCPSh_DrvConfig.cpp
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
//  This file implements the driver configuration class.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "HAIOmniTCPSh_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TOmniTCPDrvCfg,TObject)



// ---------------------------------------------------------------------------
//  Local types and constants
// ---------------------------------------------------------------------------
namespace HAIOmniTCPSh_DrvConfig
{
    // -----------------------------------------------------------------------
    //  Our persistent format
    //
    //  Version 2 -
    //      Changed the zone array from a boolean flag array to an enum based
    //      array so that we can indicate unused or, if used, what type of
    //      zone.
    //
    //  Version 3 -
    //      We completely changed things. We moved the item class hierarchy
    //      that the server uses to store the data live, and now we use that
    //      to store the configuration. So we read in the old stuff and create
    //      the new classes, then we store that thereafter.
    // -----------------------------------------------------------------------
    const tCIDLib::TCard1   c1FmtVersion = 3;
}



// ---------------------------------------------------------------------------
//   CLASS: TOmniTCPDrvCfg
//  PREFIX: dcfg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TOmniTCPDrvCfg: Constructors and Destructor
// ---------------------------------------------------------------------------
TOmniTCPDrvCfg::TOmniTCPDrvCfg()
{
    InitAll();
}

TOmniTCPDrvCfg::TOmniTCPDrvCfg(const TOmniTCPDrvCfg& dcfgSrc)
{
    // We just call a helper to do the work
    DupSrc(dcfgSrc, kCIDLib::True);
}

TOmniTCPDrvCfg::~TOmniTCPDrvCfg()
{
    FreeAll();
}


// ---------------------------------------------------------------------------
//  TOmniTCPDrvCfg: Public oeprators
// ---------------------------------------------------------------------------
TOmniTCPDrvCfg& TOmniTCPDrvCfg::operator=(const TOmniTCPDrvCfg& dcfgSrc)
{
    // We call a helper to do the work
    if (this != &dcfgSrc)
        DupSrc(dcfgSrc, kCIDLib::False);
    return *this;
}


tCIDLib::TBoolean TOmniTCPDrvCfg::operator==(const TOmniTCPDrvCfg& dcfgSrc) const
{
    if (&dcfgSrc == this)
        return kCIDLib::True;

    for (tCIDLib::TCard4 c4Index = 0; c4Index < kHAIOmniTCPSh::c4MaxAreas; c4Index++)
    {
        const THAIOmniArea* pitemSrc = dcfgSrc.m_apitemAreas[c4Index];
        const THAIOmniArea* pitemTar = m_apitemAreas[c4Index];
        if (*pitemSrc != *pitemTar)
            return kCIDLib::False;
    }

    for (tCIDLib::TCard4 c4Index = 0; c4Index < kHAIOmniTCPSh::c4MaxThermos; c4Index++)
    {
        const THAIOmniThermo* pitemSrc = dcfgSrc.m_apitemThermos[c4Index];
        const THAIOmniThermo* pitemTar = m_apitemThermos[c4Index];
        if (*pitemSrc != *pitemTar)
            return kCIDLib::False;
    }

    for (tCIDLib::TCard4 c4Index = 0; c4Index < kHAIOmniTCPSh::c4MaxUnits; c4Index++)
    {
        const THAIOmniUnit* pitemSrc = dcfgSrc.m_apitemUnits[c4Index];
        const THAIOmniUnit* pitemTar = m_apitemUnits[c4Index];
        if (*pitemSrc != *pitemTar)
            return kCIDLib::False;
    }

    for (tCIDLib::TCard4 c4Index = 0; c4Index < kHAIOmniTCPSh::c4MaxZones; c4Index++)
    {
        const THAIOmniZone* pitemSrc = dcfgSrc.m_apitemZones[c4Index];
        const THAIOmniZone* pitemTar = m_apitemZones[c4Index];
        if (*pitemSrc != *pitemTar)
            return kCIDLib::False;
    }

    return kCIDLib::True;
}

tCIDLib::TBoolean TOmniTCPDrvCfg::operator!=(const TOmniTCPDrvCfg& dcfgSrc) const
{
    return !operator==(dcfgSrc);
}


// ---------------------------------------------------------------------------
//  TOmniTCPDrvCfg: Public, non-virtual methods
// ---------------------------------------------------------------------------

// See if the indicated name is used by any other item, of any type
tCIDLib::TBoolean TOmniTCPDrvCfg::bNameUsed(const TString& strToCheck) const
{
    for (tCIDLib::TCard4 c4Index = 0; c4Index < kHAIOmniTCPSh::c4MaxAreas; c4Index++)
    {
        const THAIOmniArea* pitemCur = m_apitemAreas[c4Index];
        if (pitemCur && pitemCur->strName().bCompareI(strToCheck))
            return kCIDLib::True;
    }

    for (tCIDLib::TCard4 c4Index = 0; c4Index < kHAIOmniTCPSh::c4MaxThermos; c4Index++)
    {
        const THAIOmniThermo* pitemCur = m_apitemThermos[c4Index];
        if (pitemCur && pitemCur->strName().bCompareI(strToCheck))
            return kCIDLib::True;
    }

    for (tCIDLib::TCard4 c4Index = 0; c4Index < kHAIOmniTCPSh::c4MaxUnits; c4Index++)
    {
        const THAIOmniUnit* pitemCur = m_apitemUnits[c4Index];
        if (pitemCur && pitemCur->strName().bCompareI(strToCheck))
            return kCIDLib::True;
    }

    for (tCIDLib::TCard4 c4Index = 0; c4Index < kHAIOmniTCPSh::c4MaxZones; c4Index++)
    {
        const THAIOmniZone* pitemCur = m_apitemZones[c4Index];
        if (pitemCur && pitemCur->strName().bCompareI(strToCheck))
            return kCIDLib::True;
    }
    return kCIDLib::False;
}


// Access an area by index. Could be null if that one isn't configured
THAIOmniArea* TOmniTCPDrvCfg::pitemAreaAt(const tCIDLib::TCard4 c4At)
{
    CheckIndex(L"Area", c4At, kHAIOmniTCPSh::c4MaxAreas);
    return m_apitemAreas[c4At];
}


//
//  Look up each of our item types by name. We can either return null or
//  throw if not found.
//
THAIOmniArea*
TOmniTCPDrvCfg::pitemFindArea(  const   TString&            strFind
                                , const tCIDLib::TBoolean   bThrowIfNot)
{
    for (tCIDLib::TCard4 c4Index = 0; c4Index < kHAIOmniTCPSh::c4MaxAreas; c4Index++)
    {
        if (!m_apitemAreas[c4Index])
            continue;

        if (m_apitemAreas[c4Index]->strName().bCompare(strFind))
            return m_apitemAreas[c4Index];
    }

    if (bThrowIfNot)
    {
        facHAIOmniTCPSh().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kOmniTCPShErrs::errcCfg_ItemNotFound
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , TString(L"area")
            , strFind
        );
    }
    return 0;
}

THAIOmniThermo*
TOmniTCPDrvCfg::pitemFindThermo(const   TString&            strFind
                                , const tCIDLib::TBoolean   bThrowIfNot)
{
    for (tCIDLib::TCard4 c4Index = 0; c4Index < kHAIOmniTCPSh::c4MaxThermos; c4Index++)
    {
        if (!m_apitemThermos[c4Index])
            continue;

        if (m_apitemThermos[c4Index]->strName().bCompare(strFind))
            return m_apitemThermos[c4Index];
    }

    if (bThrowIfNot)
    {
        facHAIOmniTCPSh().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kOmniTCPShErrs::errcCfg_ItemNotFound
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , TString(L"thermo")
            , strFind
        );
    }
    return 0;
}

THAIOmniUnit*
TOmniTCPDrvCfg::pitemFindUnit(  const   TString&            strFind
                                , const tCIDLib::TBoolean   bThrowIfNot)
{
    for (tCIDLib::TCard4 c4Index = 0; c4Index < kHAIOmniTCPSh::c4MaxUnits; c4Index++)
    {
        if (!m_apitemUnits[c4Index])
            continue;

        if (m_apitemUnits[c4Index]->strName().bCompareI(strFind))
            return m_apitemUnits[c4Index];
    }

    if (bThrowIfNot)
    {
        facHAIOmniTCPSh().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kOmniTCPShErrs::errcCfg_ItemNotFound
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , TString(L"unit")
            , strFind
        );
    }
    return 0;
}

THAIOmniZone*
TOmniTCPDrvCfg::pitemFindZone(  const   TString&            strFind
                                , const tCIDLib::TBoolean   bThrowIfNot)
{
    for (tCIDLib::TCard4 c4Index = 0; c4Index < kHAIOmniTCPSh::c4MaxZones; c4Index++)
    {
        if (!m_apitemZones[c4Index])
            continue;

        if (m_apitemZones[c4Index]->strName().bCompareI(strFind))
            return m_apitemZones[c4Index];
    }

    if (bThrowIfNot)
    {
        facHAIOmniTCPSh().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kOmniTCPShErrs::errcCfg_ItemNotFound
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , TString(L"zone")
            , strFind
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
TOmniTCPDrvCfg::pitemOwningThermo(const tCIDLib::TCard4 c4FldId) const
{
    THAIOmniThermo* pitemCur;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < kHAIOmniTCPSh::c4MaxThermos; c4Index++)
    {
        pitemCur = m_apitemThermos[c4Index];
        if (!pitemCur)
            continue;

        if (pitemCur->bOwnsField(c4FldId))
            return pitemCur;
    }
    return 0;
}


THAIOmniUnit*
TOmniTCPDrvCfg::pitemOwningUnit(const tCIDLib::TCard4 c4FldId) const
{
    THAIOmniUnit* pitemCur;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < kHAIOmniTCPSh::c4MaxUnits; c4Index++)
    {
        pitemCur = m_apitemUnits[c4Index];
        if (!pitemCur)
            continue;

        if (pitemCur->bOwnsField(c4FldId))
            return pitemCur;
    }
    return 0;
}


// Access a thermo by index. Could be null if that one isn't configured
THAIOmniThermo* TOmniTCPDrvCfg::pitemThermoAt(const tCIDLib::TCard4 c4At)
{
    CheckIndex(L"Thermo", c4At, kHAIOmniTCPSh::c4MaxThermos);
    return m_apitemThermos[c4At];
}

// Access aunit by index. Could be null if that one isn't configured
THAIOmniUnit* TOmniTCPDrvCfg::pitemUnitAt(const tCIDLib::TCard4 c4At)
{
    CheckIndex(L"Unit", c4At, kHAIOmniTCPSh::c4MaxUnits);
    return m_apitemUnits[c4At];
}


// Access a zone by index. Could be null if that one isn't configured
THAIOmniZone* TOmniTCPDrvCfg::pitemZoneAt(const tCIDLib::TCard4 c4At)
{
    CheckIndex(L"Zone", c4At, kHAIOmniTCPSh::c4MaxZones);
    return m_apitemZones[c4At];
}


// Remove the indicated unit type at the indicated index
tCIDLib::TVoid
TOmniTCPDrvCfg::RemoveAt(const  tHAIOmniTCPSh::EItemTypes   eType
                        , const tCIDLib::TCard4             c4At)
{
    switch(eType)
    {
        case tHAIOmniTCPSh::EItem_Area :
        {
            CheckIndex(L"Area", c4At, kHAIOmniTCPSh::c4MaxAreas);
            delete m_apitemAreas[c4At];
            m_apitemAreas[c4At] = 0;
            break;
        }

        case tHAIOmniTCPSh::EItem_Thermo :
        {
            CheckIndex(L"Thermo", c4At, kHAIOmniTCPSh::c4MaxThermos);
            delete m_apitemThermos[c4At];
            m_apitemThermos[c4At] = 0;
            break;
        }

        case tHAIOmniTCPSh::EItem_Unit :
        {
            CheckIndex(L"Unit", c4At, kHAIOmniTCPSh::c4MaxUnits);
            delete m_apitemUnits[c4At];
            m_apitemUnits[c4At] = 0;
            break;
        }

        case tHAIOmniTCPSh::EItem_Zone :
        {
            CheckIndex(L"Zone", c4At, kHAIOmniTCPSh::c4MaxZones);
            delete m_apitemZones[c4At];
            m_apitemZones[c4At] = 0;
            break;
        }

        default :
            // Not one we keep info on
            break;
    };
}


// Remove all configured items and zero all the pointers
tCIDLib::TVoid TOmniTCPDrvCfg::Reset()
{
    FreeAll();
}


//
//  Set the state of the 'send trigger' attribute of all of the items of
//  a given type. We only do this for those types that can send triggers,
//  and within that set only those whose currently settings would allow
//  them to send triggers.
//
tCIDLib::TVoid
TOmniTCPDrvCfg::SetAllTriggers( const   tHAIOmniTCPSh::EItemTypes   eType
                                , const tCIDLib::TBoolean           bNewState)
{
    // Else, get the a pointer to the appropriate list and proces
    tCIDLib::TCard4 c4Count = 0;
    THAIOmniItem** apitemList;
    if (eType == tHAIOmniTCPSh::EItem_Unit)
    {
        apitemList = reinterpret_cast<THAIOmniItem**>(m_apitemUnits);
        c4Count = kHAIOmniTCPSh::c4MaxUnits;
    }
     else if (eType == tHAIOmniTCPSh::EItem_Zone)
    {
        apitemList = reinterpret_cast<THAIOmniItem**>(m_apitemZones);
        c4Count = kHAIOmniTCPSh::c4MaxZones;
    }
     else
    {
        // Not a type that sends triggers at all
        return;
    }

    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        // If defined, and it can send triggers as configured, set it
        THAIOmniItem* pitemCur = apitemList[c4Index];
        if (pitemCur && pitemCur->bCanSendTrig())
            pitemCur->bSendTrig(bNewState);
    }
}


//
//  Set each item at a particular index. If we don't have one there, we
//  create one.
//
tCIDLib::TVoid
TOmniTCPDrvCfg::SetAreaAt(  const   THAIOmniArea&   itemSrc
                            , const tCIDLib::TCard4 c4At)
{
    CheckIndex(L"Area", c4At, kHAIOmniTCPSh::c4MaxAreas);
    if (m_apitemAreas[c4At])
        *m_apitemAreas[c4At] = itemSrc;
    else
        m_apitemAreas[c4At] = new THAIOmniArea(itemSrc);
}

tCIDLib::TVoid
TOmniTCPDrvCfg::SetThermoAt(const   THAIOmniThermo& itemSrc
                            , const tCIDLib::TCard4 c4At)
{
    CheckIndex(L"Thermo", c4At, kHAIOmniTCPSh::c4MaxThermos);
    if (m_apitemThermos[c4At])
        *m_apitemThermos[c4At] = itemSrc;
    else
        m_apitemThermos[c4At] = new THAIOmniThermo(itemSrc);
}

tCIDLib::TVoid
TOmniTCPDrvCfg::SetUnitAt(  const   THAIOmniUnit&   itemSrc
                            , const tCIDLib::TCard4 c4At)
{
    CheckIndex(L"Unit", c4At, kHAIOmniTCPSh::c4MaxUnits);
    if (m_apitemUnits[c4At])
        *m_apitemUnits[c4At] = itemSrc;
    else
        m_apitemUnits[c4At] = new THAIOmniUnit(itemSrc);
}

tCIDLib::TVoid
TOmniTCPDrvCfg::SetZoneAt(  const   THAIOmniZone&   itemSrc
                            , const tCIDLib::TCard4 c4At)
{
    CheckIndex(L"Zone", c4At, kHAIOmniTCPSh::c4MaxZones);
    if (m_apitemZones[c4At])
        *m_apitemZones[c4At] = itemSrc;
    else
        m_apitemZones[c4At] = new THAIOmniZone(itemSrc);
}


// ---------------------------------------------------------------------------
//  TOmniTCPDrvCfg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TOmniTCPDrvCfg::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a start object marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard1 c1FmtVersion;
    strmToReadFrom  >> c1FmtVersion;
    if (!c1FmtVersion || (c1FmtVersion > HAIOmniTCPSh_DrvConfig::c1FmtVersion))
    {
        facCIDLib().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCIDErrs::errcGen_UnknownFmtVersion
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(c1FmtVersion)
            , clsThis()
        );
    }

    // Free all the existing stuff and zero slots
    FreeAll();

    //
    //  If before version 3, we read in the old format, and create the
    //  new format from that. Else, we polymorphically stream in the
    //  new item classes.
    //
    tCIDLib::TCard4 c4Index;
    if (c1FmtVersion < 3)
    {
        // We only need these now in here to read in the old stuff
        typedef TFundArray<tCIDLib::TBoolean>           TFlagArray;
        typedef TFundArray<tHAIOmniTCPSh::EUnitTypes>   TPLCArray;
        typedef TFundArray<tHAIOmniTCPSh::EZoneTypes>   TZoneArray;

        TFlagArray   fcolAreas;
        TPLCArray    fcolPLCUnits;
        TFlagArray   fcolThermos;
        TZoneArray   fcolZones;

        strmToReadFrom >> fcolAreas;

        //
        //  We have to get around an old stupid bug here where there was an incorrect
        //  assumption in the enum array I/O stuff that they were all 4 bytes in memory.
        //  This is one of the few places where such stuff was used for persistent data.
        //
        tCIDLib::TCard4 c4Count;
        {
            strmToReadFrom.CheckForMarker(tCIDLib::EStreamMarkers::StartObject, CID_FILE, CID_LINE);
            strmToReadFrom >> c4Count;
            if (c4Count)
            {
                fcolPLCUnits.Reallocate(c4Count, tHAIOmniTCPSh::EUnit_Unused);
                tCIDLib::TCard4* pc4Buf = new tCIDLib::TCard4[c4Count];
                TArrayJanitor<tCIDLib::TCard4> janBuf(pc4Buf);
                TBinInStream_ReadArray(strmToReadFrom, pc4Buf, c4Count);

                tHAIOmniTCPSh::EUnitTypes* peUnits = (tHAIOmniTCPSh::EUnitTypes*)pc4Buf;
                for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
                    fcolPLCUnits[c4Index] = peUnits[c4Index];
            }
        }

        strmToReadFrom >> fcolThermos;

        //
        //  If pre-V2, then we read in a flag array, and for each one
        //  that is on, we set the zone type to alarm. Else, just read
        //  in the new V2 data.
        //
        if (c1FmtVersion < 2)
        {
            TFlagArray fcolOld;
            strmToReadFrom >> fcolOld;
            const tCIDLib::TCard4 c4Count = fcolOld.c4ElemCount();
            for (c4Index = 0; c4Index < c4Count; c4Index++)
            {
                if (fcolOld[c4Index])
                    fcolZones[c4Index] = tHAIOmniTCPSh::EZone_Alarm;
                else
                    fcolZones[c4Index] = tHAIOmniTCPSh::EZone_Unused;
            }
        }
         else
        {
            // strmToReadFrom >> fcolZones;
        }

        // OK, let's create the new stuff
        const TString& strEmp = TString::strEmpty();
        for (c4Index = 0; c4Index < kHAIOmniTCPSh::c4MaxAreas; c4Index++)
        {
            if (fcolAreas[c4Index])
                m_apitemAreas[c4Index] = new THAIOmniArea(strEmp, c4Index + 1);
        }

        for (c4Index = 0; c4Index < kHAIOmniTCPSh::c4MaxThermos; c4Index++)
        {
            if (fcolThermos[c4Index])
                m_apitemThermos[c4Index] = new THAIOmniThermo(strEmp, c4Index + 1);
        }

        for (c4Index = 0; c4Index < kHAIOmniTCPSh::c4MaxUnits; c4Index++)
        {
            if (fcolPLCUnits[c4Index] != tHAIOmniTCPSh::EUnit_Unused)
            {
                m_apitemUnits[c4Index] = new THAIOmniUnit
                (
                    strEmp, c4Index + 1, fcolPLCUnits[c4Index]
                );
            }
        }

        for (c4Index = 0; c4Index < kHAIOmniTCPSh::c4MaxZones; c4Index++)
        {
            if (fcolZones[c4Index] != tHAIOmniTCPSh::EZone_Unused)
            {
                m_apitemZones[c4Index] = new THAIOmniZone
                (
                    strEmp, c4Index + 1, fcolZones[c4Index]
                );
            }
        }
    }
     else
    {
        //
        //  Read in each block of item types. We store the count before set
        //  one, and a frame marker after each item.
        //
        tCIDLib::TCard4 c4Count;
        {
            strmToReadFrom >> c4Count;
            THAIOmniArea itemArea;
            for (c4Index = 0; c4Index < c4Count; c4Index++)
            {
                strmToReadFrom >> itemArea;
                strmToReadFrom.CheckForFrameMarker(CID_FILE, CID_LINE);

                tCIDLib::TCard4 c4Num = itemArea.c4ItemNum() - 1;
                CIDAssert(c4Num < kHAIOmniTCPSh::c4MaxAreas, L"Invalid area number");
                m_apitemAreas[c4Num] = new THAIOmniArea(itemArea);
            }
        }

        // Thermos
        {
            strmToReadFrom >> c4Count;
            THAIOmniThermo itemThermo;
            for (c4Index = 0; c4Index < c4Count; c4Index++)
            {
                strmToReadFrom >> itemThermo;
                strmToReadFrom.CheckForFrameMarker(CID_FILE, CID_LINE);

                tCIDLib::TCard4 c4Num = itemThermo.c4ItemNum() - 1;
                CIDAssert(c4Num < kHAIOmniTCPSh::c4MaxThermos, L"Invalid thermo number");
                m_apitemThermos[c4Num] = new THAIOmniThermo(itemThermo);
            }
        }

        // Units
        {
            strmToReadFrom >> c4Count;
            THAIOmniUnit itemUnit;
            for (c4Index = 0; c4Index < c4Count; c4Index++)
            {
                strmToReadFrom >> itemUnit;
                strmToReadFrom.CheckForFrameMarker(CID_FILE, CID_LINE);

                // Figure out the item index and store a copy into the list
                tCIDLib::TCard4 c4Num = itemUnit.c4ItemNum() - 1;
                CIDAssert(c4Num < kHAIOmniTCPSh::c4MaxUnits, L"Invalid unit number");
                m_apitemUnits[c4Num] = new THAIOmniUnit(itemUnit);
            }
        }

        // Zones
        {
            strmToReadFrom >> c4Count;
            THAIOmniZone itemZone;
            for (c4Index = 0; c4Index < c4Count; c4Index++)
            {
                strmToReadFrom >> itemZone;
                strmToReadFrom.CheckForFrameMarker(CID_FILE, CID_LINE);

                tCIDLib::TCard4 c4Num = itemZone.c4ItemNum() - 1;
                CIDAssert(c4Num < kHAIOmniTCPSh::c4MaxZones, L"Invalid zone number");
                m_apitemZones[c4Num] = new THAIOmniZone(itemZone);
            }
        }
    }

    // It should end with an end object marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TOmniTCPDrvCfg::StreamTo(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << HAIOmniTCPSh_DrvConfig::c1FmtVersion;


    //
    //  For each type of item, figure out how many are defined and write that
    //  out, then stream out each of them.
    //
    tCIDLib::TCard4 c4Index, c4Count;

    // Areas
    c4Count = 0;
    for (c4Index = 0; c4Index < kHAIOmniTCPSh::c4MaxAreas; c4Index++)
    {
        if (m_apitemAreas[c4Index])
            c4Count++;
    }
    strmToWriteTo << c4Count;
    for (c4Index = 0; c4Index < kHAIOmniTCPSh::c4MaxAreas; c4Index++)
    {
        if (!m_apitemAreas[c4Index])
            continue;
        strmToWriteTo   << *m_apitemAreas[c4Index]
                        << tCIDLib::EStreamMarkers::Frame;
    }

    // Thermos
    c4Count = 0;
    for (c4Index = 0; c4Index < kHAIOmniTCPSh::c4MaxThermos; c4Index++)
    {
        if (m_apitemThermos[c4Index])
            c4Count++;
    }
    strmToWriteTo << c4Count;
    for (c4Index = 0; c4Index < kHAIOmniTCPSh::c4MaxThermos; c4Index++)
    {
        if (!m_apitemThermos[c4Index])
            continue;
        strmToWriteTo << *m_apitemThermos[c4Index]
                      << tCIDLib::EStreamMarkers::Frame;
    }

    // Units
    c4Count = 0;
    for (c4Index = 0; c4Index < kHAIOmniTCPSh::c4MaxUnits; c4Index++)
    {
        if (m_apitemUnits[c4Index])
            c4Count++;
    }
    strmToWriteTo << c4Count;
    for (c4Index = 0; c4Index < kHAIOmniTCPSh::c4MaxUnits; c4Index++)
    {
        if (!m_apitemUnits[c4Index])
            continue;
        strmToWriteTo << *m_apitemUnits[c4Index]
                      << tCIDLib::EStreamMarkers::Frame;
    }

    // Zones
    c4Count = 0;
    for (c4Index = 0; c4Index < kHAIOmniTCPSh::c4MaxZones; c4Index++)
    {
        if (m_apitemZones[c4Index])
            c4Count++;
    }
    strmToWriteTo << c4Count;
    for (c4Index = 0; c4Index < kHAIOmniTCPSh::c4MaxZones; c4Index++)
    {
        if (!m_apitemZones[c4Index])
            continue;
        strmToWriteTo << *m_apitemZones[c4Index]
                      << tCIDLib::EStreamMarkers::Frame;
    }

    // Close it off with an end object
    strmToWriteTo   << tCIDLib::EStreamMarkers::EndObject;
}



// ---------------------------------------------------------------------------
//  TOmniTCPDrvCfg: Private, non-virtual methods
// ---------------------------------------------------------------------------

// A helper to check an incoming index against the max for that type
tCIDLib::TVoid
TOmniTCPDrvCfg::CheckIndex( const   tCIDLib::TCh* const pszType
                            , const tCIDLib::TCard4     c4At
                            , const tCIDLib::TCard4     c4Max)
{
    if (c4At >= c4Max)
    {
        facHAIOmniTCPSh().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kOmniTCPShErrs::errcCfg_BadIndex
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Index
            , TCardinal(c4At)
            , TString(pszType)
        );
    }
}


//
//  There's a lot of code involved in duplicating a source config, and we
//  have to do it both in the copy ctor and assignment, so we moved it out
//  here. We take a flag to indicate whether we should init our slots first,
//  for the ctor.
//
tCIDLib::TVoid
TOmniTCPDrvCfg::DupSrc( const   TOmniTCPDrvCfg&     dcfgSrc
                        , const tCIDLib::TBoolean   bInit)
{
    tCIDLib::TCard4 c4Index;

    // If asked init our slots first
    if (bInit)
        InitAll();

    //
    //  Replicate any items in the source. If we have one in the same slot
    //  we assign. If we don't we dup the source. If we do and he doesn't we
    //  release ours and zero the slot.
    //
    for (c4Index = 0; c4Index < kHAIOmniTCPSh::c4MaxAreas; c4Index++)
    {
        if (m_apitemAreas[c4Index])
        {
            if (dcfgSrc.m_apitemAreas[c4Index])
            {
                // We both have one, so we can just assign
                *m_apitemAreas[c4Index] = *dcfgSrc.m_apitemAreas[c4Index];
            }
             else
            {
                // We need to get rid of ours
                delete m_apitemAreas[c4Index];
                m_apitemAreas[c4Index] = 0;
            }
        }
         else
        {
            // He has one, we don't, so dup his to ours
            if (dcfgSrc.m_apitemAreas[c4Index])
                m_apitemAreas[c4Index] = new THAIOmniArea(*dcfgSrc.m_apitemAreas[c4Index]);
        }
    }

    for (c4Index = 0; c4Index < kHAIOmniTCPSh::c4MaxThermos; c4Index++)
    {
        if (m_apitemThermos[c4Index])
        {
            if (dcfgSrc.m_apitemThermos[c4Index])
            {
                // We both have one, so we can just assign
                *m_apitemThermos[c4Index] = *dcfgSrc.m_apitemThermos[c4Index];
            }
             else
            {
                // We need to get rid of ours
                delete m_apitemThermos[c4Index];
                m_apitemThermos[c4Index] = 0;
            }
        }
         else
        {
            // He has one, we don't, so dup his to ours
            if (dcfgSrc.m_apitemThermos[c4Index])
            {
                m_apitemThermos[c4Index] = new THAIOmniThermo
                (
                    *dcfgSrc.m_apitemThermos[c4Index]
                );
            }
        }
    }

    for (c4Index = 0; c4Index < kHAIOmniTCPSh::c4MaxUnits; c4Index++)
    {
        if (m_apitemUnits[c4Index])
        {
            if (dcfgSrc.m_apitemUnits[c4Index])
            {
                // We both have one, so we can just assign
                *m_apitemUnits[c4Index] = *dcfgSrc.m_apitemUnits[c4Index];
            }
             else
            {
                // We need to get rid of ours
                delete m_apitemUnits[c4Index];
                m_apitemUnits[c4Index] = 0;
            }
        }
         else
        {
            // He has one, we don't, so dup his to ours
            if (dcfgSrc.m_apitemUnits[c4Index])
                m_apitemUnits[c4Index] = new THAIOmniUnit(*dcfgSrc.m_apitemUnits[c4Index]);
        }
    }

    for (c4Index = 0; c4Index < kHAIOmniTCPSh::c4MaxZones; c4Index++)
    {
        if (m_apitemZones[c4Index])
        {
            if (dcfgSrc.m_apitemZones[c4Index])
            {
                // We both have one, so we can just assign
                *m_apitemZones[c4Index] = *dcfgSrc.m_apitemZones[c4Index];
            }
             else
            {
                // We need to get rid of ours
                delete m_apitemZones[c4Index];
                m_apitemZones[c4Index] = 0;
            }
        }
         else
        {
            // He has one, we don't, so dup his to ours
            if (dcfgSrc.m_apitemZones[c4Index])
                m_apitemZones[c4Index] = new THAIOmniZone(*dcfgSrc.m_apitemZones[c4Index]);
        }
    }
}


//
//  Clean up everything and zero out the slots
//
tCIDLib::TVoid TOmniTCPDrvCfg::FreeAll()
{
    tCIDLib::TCard4 c4Index;

    // Clean up all of the slots
    for (c4Index = 0; c4Index < kHAIOmniTCPSh::c4MaxAreas; c4Index++)
    {
        try
        {
            delete m_apitemAreas[c4Index];
        }

        catch(...)
        {
        }
        m_apitemAreas[c4Index] = 0;
    }

    for (c4Index = 0; c4Index < kHAIOmniTCPSh::c4MaxThermos; c4Index++)
    {
        try
        {
            delete m_apitemThermos[c4Index];
        }

        catch(...)
        {
        }
        m_apitemThermos[c4Index] = 0;
    }

    for (c4Index = 0; c4Index < kHAIOmniTCPSh::c4MaxUnits; c4Index++)
    {
        try
        {
            delete m_apitemUnits[c4Index];
        }

        catch(...)
        {
        }
        m_apitemUnits[c4Index] = 0;
    }

    for (c4Index = 0; c4Index < kHAIOmniTCPSh::c4MaxZones; c4Index++)
    {
        try
        {
            delete m_apitemZones[c4Index];
        }

        catch(...)
        {
        }
        m_apitemZones[c4Index] = 0;
    }
}


// Initalize all of our slots
tCIDLib::TVoid TOmniTCPDrvCfg::InitAll()
{
    tCIDLib::TCard4 c4Index;

    for (c4Index = 0; c4Index < kHAIOmniTCPSh::c4MaxAreas; c4Index++)
        m_apitemAreas[c4Index] = 0;

    for (c4Index = 0; c4Index < kHAIOmniTCPSh::c4MaxThermos; c4Index++)
        m_apitemThermos[c4Index] = 0;

    for (c4Index = 0; c4Index < kHAIOmniTCPSh::c4MaxUnits; c4Index++)
        m_apitemUnits[c4Index] = 0;

    for (c4Index = 0; c4Index < kHAIOmniTCPSh::c4MaxZones; c4Index++)
        m_apitemZones[c4Index] = 0;
}

