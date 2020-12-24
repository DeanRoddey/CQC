//
// FILE NAME: HAIOmniTCP2Sh_DrvConfig.cpp
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
//  This file implements the driver configuration class.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "HAIOmniTCP2Sh_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TOmniTCPDrvCfg,TObject)



// ---------------------------------------------------------------------------
//  Local types and constants
// ---------------------------------------------------------------------------
namespace HAIOmniTCP2Sh_DrvConfig
{
    // -----------------------------------------------------------------------
    //  Our persistent format
    // -----------------------------------------------------------------------
    const tCIDLib::TCard1   c1FmtVersion = 1;
}



// ---------------------------------------------------------------------------
//  A comparator to look up items by their numeric id as the key. This is used for
//  binary searches of the lists, which are in id order.
//
//  Normally we'd just make this a static member of the item class, but for some
//  reason the compiler just won't be happy and there wasn't time to fight about
//  it.
// ---------------------------------------------------------------------------
static tCIDLib::ESortComps
eCompItemById(const tCIDLib::TCard2& c2Id, const THAIOmniItem& item2)
{
    if (c2Id < item2.c2ItemNum())
        return tCIDLib::ESortComps::FirstLess;
    else if (c2Id > item2.c2ItemNum())
        return tCIDLib::ESortComps::FirstGreater;
    return tCIDLib::ESortComps::Equal;
}



// ---------------------------------------------------------------------------
//   CLASS: TOmniTCPDrvCfg
//  PREFIX: dcfg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TOmniTCPDrvCfg: Public, static methods
// ---------------------------------------------------------------------------

tCIDLib::ESortComps
TOmniTCPDrvCfg::eCompById(  const   THAIOmniItem&   item1
                            , const THAIOmniItem&   item2)
{
    if (item1.c2ItemNum() < item2.c2ItemNum())
        return tCIDLib::ESortComps::FirstLess;
    else if (item1.c2ItemNum() > item2.c2ItemNum())
        return tCIDLib::ESortComps::FirstGreater;
    return tCIDLib::ESortComps::Equal;
}


// ---------------------------------------------------------------------------
//  TOmniTCPDrvCfg: Constructors and Destructor
// ---------------------------------------------------------------------------
TOmniTCPDrvCfg::TOmniTCPDrvCfg() :

    m_colAreas(tCIDLib::EAdoptOpts::Adopt, kHAIOmniTCP2Sh::c4MaxAreas)
    , m_colThermos(tCIDLib::EAdoptOpts::Adopt, kHAIOmniTCP2Sh::c4MaxThermos)
    , m_colUnits(tCIDLib::EAdoptOpts::Adopt, kHAIOmniTCP2Sh::c4MaxUnits)
    , m_colZones(tCIDLib::EAdoptOpts::Adopt, kHAIOmniTCP2Sh::c4MaxZones)
{
}

TOmniTCPDrvCfg::TOmniTCPDrvCfg(const TOmniTCPDrvCfg& dcfgSrc) :

    m_colAreas(tCIDLib::EAdoptOpts::Adopt, kHAIOmniTCP2Sh::c4MaxAreas)
    , m_colThermos(tCIDLib::EAdoptOpts::Adopt, kHAIOmniTCP2Sh::c4MaxThermos)
    , m_colUnits(tCIDLib::EAdoptOpts::Adopt, kHAIOmniTCP2Sh::c4MaxUnits)
    , m_colZones(tCIDLib::EAdoptOpts::Adopt, kHAIOmniTCP2Sh::c4MaxZones)
{
    // Replicate the source elements
    RepList(dcfgSrc.m_colAreas, m_colAreas);
    RepList(dcfgSrc.m_colThermos, m_colThermos);
    RepList(dcfgSrc.m_colUnits, m_colUnits);
    RepList(dcfgSrc.m_colZones, m_colZones);
}

TOmniTCPDrvCfg::~TOmniTCPDrvCfg()
{
    Reset();
}


// ---------------------------------------------------------------------------
//  TOmniTCPDrvCfg: Public oeprators
// ---------------------------------------------------------------------------
TOmniTCPDrvCfg& TOmniTCPDrvCfg::operator=(const TOmniTCPDrvCfg& dcfgSrc)
{
    // We call a helper to do the work
    if (this != &dcfgSrc)
    {
        RepList(dcfgSrc.m_colAreas, m_colAreas);
        RepList(dcfgSrc.m_colThermos, m_colThermos);
        RepList(dcfgSrc.m_colUnits, m_colUnits);
        RepList(dcfgSrc.m_colZones, m_colZones);
    }
    return *this;
}


tCIDLib::TBoolean TOmniTCPDrvCfg::operator==(const TOmniTCPDrvCfg& dcfgSrc) const
{
    // First just check the list counts. If not the same, then no need to go further
    if ((m_colAreas.c4ElemCount() != dcfgSrc.m_colAreas.c4ElemCount())
    ||  (m_colThermos.c4ElemCount() != dcfgSrc.m_colThermos.c4ElemCount())
    ||  (m_colUnits.c4ElemCount() != dcfgSrc.m_colUnits.c4ElemCount())
    ||  (m_colZones.c4ElemCount() != dcfgSrc.m_colZones.c4ElemCount()))
    {
        return kCIDLib::False;
    }

    tCIDLib::TCard4 c4Count = m_colAreas.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (!m_colAreas[c4Index]->bCompare(*dcfgSrc.m_colAreas[c4Index]))
            return kCIDLib::False;
    }

    c4Count = m_colThermos.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (!m_colThermos[c4Index]->bCompare(*dcfgSrc.m_colThermos[c4Index]))
            return kCIDLib::False;
    }

    c4Count = m_colUnits.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (!m_colUnits[c4Index]->bCompare(*dcfgSrc.m_colUnits[c4Index]))
            return kCIDLib::False;
    }

    c4Count = m_colZones.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (!m_colZones[c4Index]->bCompare(*dcfgSrc.m_colZones[c4Index]))
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

// Return how many of each configured type we have
tCIDLib::TCard4 TOmniTCPDrvCfg::c4AreaCnt() const
{
    return m_colAreas.c4ElemCount();
}

tCIDLib::TCard4 TOmniTCPDrvCfg::c4ThermoCnt() const
{
    return m_colThermos.c4ElemCount();
}

tCIDLib::TCard4 TOmniTCPDrvCfg::c4UnitCnt() const
{
    return m_colUnits.c4ElemCount();
}

tCIDLib::TCard4 TOmniTCPDrvCfg::c4ZoneCnt() const
{
    return m_colZones.c4ElemCount();
}


//
//  Provide access by position in the list to each configured type. This is used
//  for iteration.
//
const THAIOmniArea& TOmniTCPDrvCfg::itemAreaAt(const tCIDLib::TCard4 c4At) const
{
    return *static_cast<const THAIOmniArea*>(m_colAreas[c4At]);
}

THAIOmniArea& TOmniTCPDrvCfg::itemAreaAt(const tCIDLib::TCard4 c4At)
{
    return *static_cast<THAIOmniArea*>(m_colAreas[c4At]);
}

const THAIOmniThermo& TOmniTCPDrvCfg::itemThermoAt(const tCIDLib::TCard4 c4At) const
{
    return *static_cast<const THAIOmniThermo*>(m_colThermos[c4At]);
}

THAIOmniThermo& TOmniTCPDrvCfg::itemThermoAt(const tCIDLib::TCard4 c4At)
{
    return *static_cast<THAIOmniThermo*>(m_colThermos[c4At]);
}

const THAIOmniUnit& TOmniTCPDrvCfg::itemUnitAt(const tCIDLib::TCard4 c4At) const
{
    return *static_cast<const THAIOmniUnit*>(m_colUnits[c4At]);
}

THAIOmniUnit& TOmniTCPDrvCfg::itemUnitAt(const tCIDLib::TCard4 c4At)
{
    return *static_cast<THAIOmniUnit*>(m_colUnits[c4At]);
}

const THAIOmniZone& TOmniTCPDrvCfg::itemZoneAt(const tCIDLib::TCard4 c4At) const
{
    return *static_cast<const THAIOmniZone*>(m_colZones[c4At]);
}

THAIOmniZone& TOmniTCPDrvCfg::itemZoneAt(const tCIDLib::TCard4 c4At)
{
    return *static_cast<THAIOmniZone*>(m_colZones[c4At]);
}


// Add items to the various lists
THAIOmniArea* TOmniTCPDrvCfg::pitemAddArea(const THAIOmniArea& itemToAdd)
{
    CheckDup(itemToAdd, m_colAreas, L"areas");
    THAIOmniArea* pitemRet = new THAIOmniArea(itemToAdd);

    tCIDLib::TCard4 c4At;
    m_colAreas.InsertSorted(pitemRet, eCompById, c4At);
    return pitemRet;
}

THAIOmniThermo* TOmniTCPDrvCfg::pitemAddThermo(const THAIOmniThermo& itemToAdd)
{
    CheckDup(itemToAdd, m_colThermos, L"thermos");
    THAIOmniThermo* pitemRet = new THAIOmniThermo(itemToAdd);

    tCIDLib::TCard4 c4At;
    m_colThermos.InsertSorted(pitemRet, eCompById, c4At);
    return pitemRet;
}

THAIOmniUnit* TOmniTCPDrvCfg::pitemAddUnit(const THAIOmniUnit& itemToAdd)
{
    CheckDup(itemToAdd, m_colUnits, L"units");
    THAIOmniUnit* pitemRet = new THAIOmniUnit(itemToAdd);

    tCIDLib::TCard4 c4At;
    m_colUnits.InsertSorted(pitemRet, &eCompById, c4At);
    return pitemRet;
}


THAIOmniZone* TOmniTCPDrvCfg::pitemAddZone(const THAIOmniZone& itemToAdd)
{
    CheckDup(itemToAdd, m_colZones, L"zones");
    THAIOmniZone* pitemRet = new THAIOmniZone(itemToAdd);

    tCIDLib::TCard4 c4At;
    m_colZones.InsertSorted(pitemRet, eCompById, c4At);
    return pitemRet;
}


// Returns true if we are empty.
tCIDLib::TBoolean TOmniTCPDrvCfg::bIsEmpty() const
{
    return m_colAreas.bIsEmpty()
           && m_colThermos.bIsEmpty()
           && m_colUnits.bIsEmpty()
           && m_colZones.bIsEmpty();
}


//
//  See if the indicated name is used by any other item of the same type.
//
tCIDLib::TBoolean
TOmniTCPDrvCfg::bNameUsed(  const   TString&                    strToCheck
                            , const tHAIOmniTCP2Sh::EItemTypes  eType) const
{
    const TItemList* pcolList = pcolListForType(eType);
    if (pcolList)
    {
        const tCIDLib::TCard4 c4Count = pcolList->c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            if (pcolList->pobjAt(c4Index)->strName().bCompareI(strToCheck))
                return kCIDLib::True;
        }
    }
    return kCIDLib::False;
}


// Access an area by id. Could be null if that one isn't configured
THAIOmniArea*
TOmniTCPDrvCfg::pitemAreaById(  const   tCIDLib::TCard2     c2Id
                                ,       tCIDLib::TCard4&    c4At)
{
    THAIOmniArea* pitemRet = static_cast<THAIOmniArea*>
    (
        m_colAreas.pobjKeyedBinarySearch(c2Id, eCompItemById, c4At)
    );

    if (!pitemRet)
        c4At = kCIDLib::c4MaxCard;
    return pitemRet;
}


//
//  Look up each of our item types by name. We can either return null or
//  throw if not found.
//
THAIOmniArea*
TOmniTCPDrvCfg::pitemFindArea(  const   TString&            strFind
                                , const tCIDLib::TBoolean   bThrowIfNot)
{
    return static_cast<THAIOmniArea*>
    (
        pitemFindInList(strFind, m_colAreas, L"area", bThrowIfNot)
    );
}

const THAIOmniArea*
TOmniTCPDrvCfg::pitemFindArea(  const   TString&            strFind
                                , const tCIDLib::TBoolean   bThrowIfNot) const
{
    return static_cast<const THAIOmniArea*>
    (
        pitemFindInList(strFind, m_colAreas, L"area", bThrowIfNot)
    );
}

THAIOmniThermo*
TOmniTCPDrvCfg::pitemFindThermo(const   TString&            strFind
                                , const tCIDLib::TBoolean   bThrowIfNot)
{
    return static_cast<THAIOmniThermo*>
    (
        pitemFindInList(strFind, m_colThermos, L"thermo", bThrowIfNot)
    );
}

THAIOmniUnit*
TOmniTCPDrvCfg::pitemFindUnit(  const   TString&            strFind
                                , const tCIDLib::TBoolean   bThrowIfNot)
{
    return static_cast<THAIOmniUnit*>
    (
        pitemFindInList(strFind, m_colUnits, L"unit", bThrowIfNot)
    );
}

THAIOmniZone*
TOmniTCPDrvCfg::pitemFindZone(  const   TString&            strFind
                                , const tCIDLib::TBoolean   bThrowIfNot)
{
    return static_cast<THAIOmniZone*>
    (
        pitemFindInList(strFind, m_colZones, L"zone", bThrowIfNot)
    );
}


// Generically get an item by the base class
const THAIOmniItem*
TOmniTCPDrvCfg::pitemById(  const   tHAIOmniTCP2Sh::EItemTypes  eType
                            , const tCIDLib::TCard2             c2Id
                            ,       tCIDLib::TCard4&            c4At) const
{
    const TItemList* pcolList = pcolListForType(eType);
    const THAIOmniItem* pitemRet = pcolList->pobjKeyedBinarySearch(c2Id, eCompItemById, c4At);

    if (!pitemRet)
        c4At = kCIDLib::c4MaxCard;
    return pitemRet;
}

THAIOmniItem*
TOmniTCPDrvCfg::pitemById(  const   tHAIOmniTCP2Sh::EItemTypes  eType
                            , const tCIDLib::TCard2             c2Id
                            ,       tCIDLib::TCard4&            c4At)
{
    TItemList* pcolList = pcolListForType(eType);
    THAIOmniItem* pitemRet = pcolList->pobjKeyedBinarySearch(c2Id, eCompItemById, c4At);

    if (!pitemRet)
        c4At = kCIDLib::c4MaxCard;
    return pitemRet;
}



//
//  These find the item (of each supported item type) that owns the passed
//  field id, if any. If no item of that type owns the id, it returns a null
//  pointer.
//
THAIOmniThermo*
TOmniTCPDrvCfg::pitemOwningThermo(const tCIDLib::TCard4 c4FldId)
{
    return static_cast<THAIOmniThermo*>(pitemFldOwner(c4FldId, m_colThermos));
}


THAIOmniUnit*
TOmniTCPDrvCfg::pitemOwningUnit(const tCIDLib::TCard4 c4FldId)
{
    return static_cast<THAIOmniUnit*>(pitemFldOwner(c4FldId, m_colUnits));
}


// Access a thermo by id. Could be null if that one isn't configured
THAIOmniThermo*
TOmniTCPDrvCfg::pitemThermoById(const   tCIDLib::TCard2     c2Id
                                ,       tCIDLib::TCard4&    c4At)
{
    THAIOmniThermo* pitemRet = static_cast<THAIOmniThermo*>
    (
        m_colThermos.pobjKeyedBinarySearch(c2Id, eCompItemById, c4At)
    );

    if (!pitemRet)
        c4At = kCIDLib::c4MaxCard;
    return pitemRet;
}


// Access aunit by id. Could be null if that one isn't configured
THAIOmniUnit*
TOmniTCPDrvCfg::pitemUnitById(  const   tCIDLib::TCard2     c2Id
                                ,       tCIDLib::TCard4&    c4At)
{
    THAIOmniUnit* pitemRet = static_cast<THAIOmniUnit*>
    (
        m_colUnits.pobjKeyedBinarySearch(c2Id, eCompItemById, c4At)
    );

    if (!pitemRet)
        c4At = kCIDLib::c4MaxCard;
    return pitemRet;
}


// Access a zone by index. Could be null if that one isn't configured
THAIOmniZone*
TOmniTCPDrvCfg::pitemZoneById(  const   tCIDLib::TCard2     c2Id
                                ,       tCIDLib::TCard4&    c4At)
{
    THAIOmniZone* pitemRet = static_cast<THAIOmniZone*>
    (
        m_colZones.pobjKeyedBinarySearch(c2Id, eCompItemById, c4At)
    );

    if (!pitemRet)
        c4At = kCIDLib::c4MaxCard;
    return pitemRet;
}


//
//  Remove the item of the indicated type and with the indicated id.
//
tCIDLib::TVoid
TOmniTCPDrvCfg::RemoveItem( const   tHAIOmniTCP2Sh::EItemTypes  eType
                            , const tCIDLib::TCard2             c2Id)
{
    TItemList* pcolList = pcolListForType(eType);
    if (pcolList)
    {
        tCIDLib::TCard4 c4At;
        if (pcolList->pobjKeyedBinarySearch(c2Id, eCompItemById, c4At))
            pcolList->RemoveAt(c4At);
    }
}


// Remove all configured items
tCIDLib::TVoid TOmniTCPDrvCfg::Reset()
{
    m_colAreas.RemoveAll();
    m_colThermos.RemoveAll();
    m_colUnits.RemoveAll();
    m_colZones.RemoveAll();
}


//
//  Set the state of the 'send trigger' attribute of all of the items of
//  a given type. We only do this for those types that can send triggers,
//  and within that set only those whose currently settings would allow
//  them to send triggers.
//
tCIDLib::TVoid
TOmniTCPDrvCfg::SetAllTriggers( const   tHAIOmniTCP2Sh::EItemTypes  eType
                                , const tCIDLib::TBoolean           bNewState)
{
    TItemList* pcolList = pcolListForType(eType);
    if (pcolList)
    {
        const tCIDLib::TCard4 c4Count = pcolList->c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            THAIOmniItem* pitemCur = pcolList->pobjAt(c4Index);
            if (pitemCur->bCanSendTrig())
                pitemCur->bSendTrig(bNewState);
        }
    }
}


// Udpate each of the item types by their index into their respective lists
tCIDLib::TVoid
TOmniTCPDrvCfg::SetAreaAt(  const   THAIOmniArea&   itemToSet
                            , const tCIDLib::TCard4 c4At)
{
    *static_cast<THAIOmniArea*>(m_colAreas[c4At]) = itemToSet;
}

tCIDLib::TVoid
TOmniTCPDrvCfg::SetThermoAt(const   THAIOmniThermo& itemToSet
                            , const tCIDLib::TCard4 c4At)
{
    *static_cast<THAIOmniThermo*>(m_colThermos[c4At]) = itemToSet;
}

tCIDLib::TVoid
TOmniTCPDrvCfg::SetUnitAt(  const   THAIOmniUnit&   itemToSet
                            , const tCIDLib::TCard4 c4At)
{
    *static_cast<THAIOmniUnit*>(m_colUnits[c4At]) = itemToSet;
}

tCIDLib::TVoid
TOmniTCPDrvCfg::SetZoneAt(  const   THAIOmniZone&   itemToSet
                            , const tCIDLib::TCard4 c4At)
{
    *static_cast<THAIOmniZone*>(m_colZones[c4At]) = itemToSet;
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
    if (!c1FmtVersion || (c1FmtVersion > HAIOmniTCP2Sh_DrvConfig::c1FmtVersion))
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

    // Clean out our lists
    Reset();

    //
    //  Read in each block of item types. We store the count before set
    //  one, and a frame marker after each item.
    //
    tCIDLib::TCard4 c4Index;
    tCIDLib::TCard4 c4Count;
    {
        strmToReadFrom >> c4Count;
        THAIOmniArea itemArea;
        for (c4Index = 0; c4Index < c4Count; c4Index++)
        {
            strmToReadFrom >> itemArea;
            strmToReadFrom.CheckForFrameMarker(CID_FILE, CID_LINE);

            tCIDLib::TCard2 c2Num = itemArea.c2ItemNum() - 1;
            CIDAssert(c2Num < kHAIOmniTCP2Sh::c4MaxAreas, L"Invalid area number");
            m_colAreas.Add(new THAIOmniArea(itemArea));
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

            tCIDLib::TCard2 c2Num = itemThermo.c2ItemNum() - 1;
            CIDAssert(c2Num < kHAIOmniTCP2Sh::c4MaxThermos, L"Invalid thermo number");
            m_colThermos.Add(new THAIOmniThermo(itemThermo));
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
            tCIDLib::TCard2 c2Num = itemUnit.c2ItemNum() - 1;
            CIDAssert(c2Num < kHAIOmniTCP2Sh::c4MaxUnits, L"Invalid unit number");
            m_colUnits.Add(new THAIOmniUnit(itemUnit));

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

            tCIDLib::TCard2 c2Num = itemZone.c2ItemNum() - 1;
            CIDAssert(c2Num < kHAIOmniTCP2Sh::c4MaxZones, L"Invalid zone number");
            m_colZones.Add(new THAIOmniZone(itemZone));
        }
    }

    // It should end with an end object marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TOmniTCPDrvCfg::StreamTo(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << HAIOmniTCP2Sh_DrvConfig::c1FmtVersion;


    //
    //  For each type of item, figure out how many are defined and write that
    //  out, then stream out each of them.
    //
    tCIDLib::TCard4 c4Index, c4Count;

    // Areas
    c4Count = m_colAreas.c4ElemCount();
    strmToWriteTo << c4Count;
    for (c4Index = 0; c4Index < c4Count; c4Index++)
    {
        strmToWriteTo   << *m_colAreas[c4Index]
                        << tCIDLib::EStreamMarkers::Frame;
    }

    // Thermos
    c4Count = m_colThermos.c4ElemCount();
    strmToWriteTo << c4Count;
    for (c4Index = 0; c4Index < c4Count; c4Index++)
    {
        strmToWriteTo << *m_colThermos[c4Index]
                      << tCIDLib::EStreamMarkers::Frame;
    }

    // Units
    c4Count = m_colUnits.c4ElemCount();
    strmToWriteTo << c4Count;
    for (c4Index = 0; c4Index < c4Count; c4Index++)
    {
        strmToWriteTo << *m_colUnits[c4Index]
                      << tCIDLib::EStreamMarkers::Frame;
    }

    // Zones
    c4Count = m_colZones.c4ElemCount();
    strmToWriteTo << c4Count;
    for (c4Index = 0; c4Index < c4Count; c4Index++)
    {
        strmToWriteTo << *m_colZones[c4Index]
                      << tCIDLib::EStreamMarkers::Frame;
    }

    // Close it off with an end object
    strmToWriteTo   << tCIDLib::EStreamMarkers::EndObject;
}



// ---------------------------------------------------------------------------
//  TOmniTCPDrvCfg: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Check that the passed item is not a dup either in name or id. If so, then throw
//  an exception.
//
tCIDLib::TVoid
TOmniTCPDrvCfg::CheckDup(const  THAIOmniItem&       itemCheck
                        , const TItemList&          colList
                        , const tCIDLib::TCh* const pszType) const
{
    const tCIDLib::TCard2 c2Id = itemCheck.c2ItemNum();
    const TString& strName = itemCheck.strName();

    const tCIDLib::TCard4 c4Count = colList.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const THAIOmniItem* pitemCur = colList[c4Index];

        if (pitemCur->strName().bCompareI(strName))
        {
            facHAIOmniTCP2Sh().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kOmniTCP2ShErrs::errcCfg_DupName
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Already
                , strName
                , TString(pszType)
            );
        }

        if (pitemCur->c2ItemNum() == c2Id)
        {
            facHAIOmniTCP2Sh().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kOmniTCP2ShErrs::errcCfg_DupId
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Already
                , TCardinal(c2Id)
                , TString(pszType)
            );
        }
    }
}


// Return the list for the indicated type
TOmniTCPDrvCfg::TItemList*
TOmniTCPDrvCfg::pcolListForType(const tHAIOmniTCP2Sh::EItemTypes eType)
{
    TItemList* pcolList = 0;

    if (eType == tHAIOmniTCP2Sh::EItemTypes::Area)
        pcolList = &m_colAreas;
    else if (eType == tHAIOmniTCP2Sh::EItemTypes::Thermo)
        pcolList = &m_colThermos;
    else if (eType == tHAIOmniTCP2Sh::EItemTypes::Unit)
        pcolList = &m_colUnits;
    else if (eType == tHAIOmniTCP2Sh::EItemTypes::Zone)
        pcolList = &m_colZones;

    return pcolList;
}

const TOmniTCPDrvCfg::TItemList*
TOmniTCPDrvCfg::pcolListForType(const tHAIOmniTCP2Sh::EItemTypes eType) const
{
    const TItemList* pcolList = 0;

    if (eType == tHAIOmniTCP2Sh::EItemTypes::Area)
        pcolList = &m_colAreas;
    else if (eType == tHAIOmniTCP2Sh::EItemTypes::Thermo)
        pcolList = &m_colThermos;
    else if (eType == tHAIOmniTCP2Sh::EItemTypes::Unit)
        pcolList = &m_colUnits;
    else if (eType == tHAIOmniTCP2Sh::EItemTypes::Zone)
        pcolList = &m_colZones;

    return pcolList;
}



//
//  Look up the indicate name in the list. We have ot just do a brute force search
//  since we don't have a sorted key for the name.
//
const THAIOmniItem*
TOmniTCPDrvCfg::pitemFindInList(const   TString&            strName
                                , const TItemList&          colSrc
                                , const tCIDLib::TCh* const pszType
                                , const tCIDLib::TBoolean   bThrowIfNot) const
{
    const tCIDLib::TCard4 c4Count = colSrc.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (colSrc[c4Index]->strName() == strName)
            return colSrc[c4Index];
    }

    if (bThrowIfNot)
    {
        facHAIOmniTCP2Sh().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kOmniTCP2ShErrs::errcCfg_ItemNotFound
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , TString(pszType)
            , strName
        );
    }
    return 0;
}

THAIOmniItem*
TOmniTCPDrvCfg::pitemFindInList(const   TString&            strName
                                ,       TItemList&          colSrc
                                , const tCIDLib::TCh* const pszType
                                , const tCIDLib::TBoolean   bThrowIfNot)
{
    return const_cast<THAIOmniItem*>
    (
        const_cast<const TOmniTCPDrvCfg*>(this)->pitemFindInList
        (
            strName, colSrc, pszType, bThrowIfNot
        )
    );
}


// See if there's an item in the passed list that owns the indicated field id
THAIOmniItem*
TOmniTCPDrvCfg::pitemFldOwner(  const   tCIDLib::TCard4 c4FldId
                                ,       TItemList&      colSrc)
{
    const tCIDLib::TCard4 c4Count = colSrc.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (colSrc[c4Index]->bOwnsField(c4FldId))
            return colSrc[c4Index];
    }
    return 0;
}


// Replicate the items of the source list to the target
tCIDLib::TVoid
TOmniTCPDrvCfg::RepList(const TItemList& colSrc, TItemList& colTar)
{
    colTar.RemoveAll();
    const tCIDLib::TCard4 c4Count = colSrc.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        colTar.Add(::pDupObject<THAIOmniItem>(colSrc[c4Index]));
}


