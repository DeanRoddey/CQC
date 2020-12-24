//
// FILE NAME: ZWaveLeviSh_DrvConfig.cpp
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
//  This file implements the TZWaveDrvConfig class, which is just a container
//  class to hold all our configuration and handle streaming it in and out.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "ZWaveLeviSh_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TZWaveDrvConfig,TObject)


// ---------------------------------------------------------------------------
//  Local types and constants
// ---------------------------------------------------------------------------
namespace ZWaveLeviSh_DrvConfig
{
    // -----------------------------------------------------------------------
    //  Our persistent format version
    // -----------------------------------------------------------------------
    const tCIDLib::TCard2   c2FmtVersion = 1;
}




// ---------------------------------------------------------------------------
//   CLASS: TZWaveDrvConfig
//  PREFIX: dcfg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWaveDrvConfig: Constructors and Destructor
// ---------------------------------------------------------------------------
TZWaveDrvConfig::TZWaveDrvConfig() :

    m_colGroups(kZWaveLeviSh::c4MaxGroupId)
    , m_colUnits(tCIDLib::EAdoptOpts::Adopt, 8)
{
}

TZWaveDrvConfig::TZWaveDrvConfig(const TZWaveDrvConfig& dcfgToCopy) :

    m_colGroups(dcfgToCopy.m_colGroups)
    , m_colUnits(tCIDLib::EAdoptOpts::Adopt, dcfgToCopy.m_colUnits.c4ElemCount())
{
    // Polymorphically replicate the units
    const tCIDLib::TCard4 c4Count = dcfgToCopy.m_colUnits.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        m_colUnits.Add(::pDupObject<TZWaveUnit>(dcfgToCopy.m_colUnits[c4Index]));
}

TZWaveDrvConfig::~TZWaveDrvConfig()
{
}


// ---------------------------------------------------------------------------
//  TZWaveDrvConfig: Public operators
// ---------------------------------------------------------------------------
TZWaveDrvConfig& TZWaveDrvConfig::operator=(const TZWaveDrvConfig& dcfgToAssign)
{
    if (this != &dcfgToAssign)
    {
        m_colGroups = dcfgToAssign.m_colGroups;

        // Polymorphically replicate the units
        m_colUnits.RemoveAll();
        const tCIDLib::TCard4 c4Count = dcfgToAssign.m_colUnits.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            m_colUnits.Add(::pDupObject<TZWaveUnit>(dcfgToAssign.m_colUnits[c4Index]));
    }
    return *this;
}


tCIDLib::TBoolean
TZWaveDrvConfig::operator==(const TZWaveDrvConfig& dcfgSrc) const
{
    // If they have different unit counts, can't be equal
    const tCIDLib::TCard4 c4UnitCount = m_colUnits.c4ElemCount();
    if (dcfgSrc.m_colUnits.c4ElemCount() != c4UnitCount)
        return kCIDLib::False;

    // The same number, so compare them, If any aren't equal, then we are done
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4UnitCount; c4Index++)
    {
        if (*m_colUnits[c4Index] != *dcfgSrc.m_colUnits[c4Index])
            return kCIDLib::False;
    }

    // Compare the group names. These lists are fixed size so always equal sized
    const tCIDLib::TCard4 c4GrpCount = m_colGroups.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4GrpCount; c4Index++)
    {
        if (m_colGroups[c4Index] != dcfgSrc.m_colGroups[c4Index])
            return kCIDLib::False;
    }
    return kCIDLib::True;
}

tCIDLib::TBoolean
TZWaveDrvConfig::operator!=(const TZWaveDrvConfig& dcfgSrc) const
{
    return !operator==(dcfgSrc);
}


// ---------------------------------------------------------------------------
//  TZWaveDrvConfig: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Add the passed unit to our list, sorted by id. We insure that neither its
//  name or unit id is already in the list.
//
tCIDLib::TBoolean TZWaveDrvConfig::bAddUnit(TZWaveUnit* const punitToAdd)
{
    // Make sure it's correctly set up
    if ((punitToAdd->c4Id() == kCIDLib::c4MaxCard)
    ||  (punitToAdd->c1ClassGen() == kCIDLib::c1MaxCard))
    {
        // We have to delete it since we would have adopted if it worked
        delete punitToAdd;
        facZWaveLeviSh().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kZWShErrs::errcCfg_UnitNotCfg
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
        );
    }

    // Make sure that it doesn't alrady exist
    if (punitFindById(punitToAdd->c4Id())
    ||  punitFindByName(punitToAdd->strName()))
    {
        // We have to delete it since we would have adopted if it worked
        delete punitToAdd;
        return kCIDLib::False;
    }

    tCIDLib::TCard4 c4At;
    m_colUnits.InsertSorted
    (
        punitToAdd, &TZWaveUnit::eCompByName, c4At, tCIDLib::ESortDirs::Ascending
    );
    return kCIDLib::True;
}


// Returns true if we have no units and all groups are empty
tCIDLib::TBoolean TZWaveDrvConfig::bIsEmpty() const
{
    for (tCIDLib::TCard4 c4Index = 0; c4Index < kZWaveLeviSh::c4MaxGroupId; c4Index++)
    {
        if (!m_colGroups[c4Index].bIsEmpty())
            return kCIDLib::False;
    }
    return m_colUnits.bIsEmpty();
}


//
//  Returns true if the indicated group or unit name exists in our list,
//  and returns the group number if so (the 1 based number used by the
//  outside world.
//
tCIDLib::TBoolean
TZWaveDrvConfig::bGroupExists(const TString& strToCheck, tCIDLib::TCard4& c4GrpNum) const
{
    for (tCIDLib::TCard4 c4Index = 0; c4Index < kZWaveLeviSh::c4MaxGroupId; c4Index++)
    {
        if (m_colGroups[c4Index] == strToCheck)
        {
            c4GrpNum = c4Index + 1;
            return kCIDLib::True;
        }
    }
    return kCIDLib::False;
}


tCIDLib::TBoolean TZWaveDrvConfig::bUnitExists(const TString& strToCheck) const
{
    return (punitFindByName(strToCheck) != 0);
}


//
//  Validate our configuration data. Currently nothing required, so just
//  return true.
//
tCIDLib::TBoolean TZWaveDrvConfig::bValidate()
{
    return kCIDLib::True;
}


//
//  Return a list of all of the units that are due for polling, sorted by
//  their next poll times (earliest times first, which means the ones most
//  out of date first.)
//
//  If any are more than 4x it's poll period overdue, then put them into error
//  state until such time that we get some values for them.
//
tCIDLib::TCard4
TZWaveDrvConfig::c4QueryDuePollList(tZWaveLeviSh::TUnitList&    colToFill
                                    , TCQCServerBase&           sdrvLevi)
{
    const tCIDLib::TEncodedTime enctNow = TTime::enctNow();

    colToFill.RemoveAll();
    const tCIDLib::TCard4 c4UnitCount = m_colUnits.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4UnitCount; c4Index++)
    {
        TZWaveUnit* punitCur = m_colUnits[c4Index];

        //
        //  If not pollable, not readable, or battery powered, or not viable,
        //  then skip this one.
        //
        if (!punitCur->bIsPollable()
        ||  !punitCur->bIsViable()
        ||  !punitCur->bIsReadable()
        ||  punitCur->bIsBatteryPwr())
        {
            continue;
        }

        const tCIDLib::TEncodedTime enctNP = punitCur->enctNextPoll();
        if (enctNP <= enctNow)
        {
            tCIDLib::TCard4 c4At;
            colToFill.InsertSorted
            (
                punitCur
                , &TZWaveUnit::eCompByNextPoll
                , c4At
                , tCIDLib::ESortDirs::Ascending
            );

            //
            //  If we haven't gotten a message in four  times its poll period,
            //  then put it into error state.
            //
            const tCIDLib::TEncodedTime enctLV = punitCur->enctLastValue();
            if ((enctLV > enctNow)
            ||  (enctNow - enctLV) > (punitCur->enctPollPeriod() * 4))
            {
                punitCur->SetErrorStatus(&sdrvLevi);
            }
        }
    }
    return colToFill.c4ElemCount();
}


// Return the count of units
tCIDLib::TCard4 TZWaveDrvConfig::c4UnitCnt() const
{
    return m_colUnits.c4ElemCount();
}


//
//  Delete the group with the indicated id. It just amounts to clearing the
//  group name out if we find it.
//
tCIDLib::TVoid TZWaveDrvConfig::DeleteGroup(const TString& strGroupId)
{
    for (tCIDLib::TCard4 c4Index = 0; c4Index < kZWaveLeviSh::c4MaxGroupId; c4Index++)
    {
        if (m_colGroups[c4Index] == strGroupId)
        {
            m_colGroups[c4Index].Clear();
            break;
        }
    }
}


//
//  Delete the indicated unit. We just remove it from the unit list.
//
tCIDLib::TVoid TZWaveDrvConfig::DeleteUnit(TZWaveUnit* const punitDel)
{
    m_colUnits.RemoveElem(punitDel);
}


//
//  Find the id of a group by name and throw if not found.
//
tCIDLib::TVoid
TZWaveDrvConfig::FindGroupByName(const  TString&            strName
                                ,       tCIDLib::TCard4&    c4GrpId)
{
    if (!bGroupExists(strName, c4GrpId))
    {
        facZWaveLeviSh().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kZWShErrs::errcCfg_GroupNotFound
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , strName
        );
    }
}


//
//  Find an existing unit in our list with the passed unit id. If not found,
//  we return a null pointer.
//
const TZWaveUnit*
TZWaveDrvConfig::punitFindById(const tCIDLib::TCard4 c4ToFind) const
{
    const tCIDLib::TCard4 c4Count = m_colUnits.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TZWaveUnit* punitCur = m_colUnits[c4Index];
        if (punitCur->c4Id() == c4ToFind)
            return punitCur;
    }
    return 0;
}

TZWaveUnit* TZWaveDrvConfig::punitFindById(const tCIDLib::TCard4 c4ToFind)
{
    const tCIDLib::TCard4 c4Count = m_colUnits.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TZWaveUnit* punitCur = m_colUnits[c4Index];
        if (punitCur->c4Id() == c4ToFind)
            return punitCur;
    }
    return 0;
}


//
//  Find the unit with the passed name. If found, we return a pointer, else
//  we return null.
//
const TZWaveUnit*
TZWaveDrvConfig::punitFindByName(const TString& strToFind) const
{
    const tCIDLib::TCard4 c4Count = m_colUnits.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TZWaveUnit* punitCur = m_colUnits[c4Index];
        if (punitCur->strName() == strToFind)
            return punitCur;
    }
    return 0;
}

TZWaveUnit*
TZWaveDrvConfig::punitFindByName(const TString& strToFind)
{
    const tCIDLib::TCard4 c4Count = m_colUnits.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TZWaveUnit* punitCur = m_colUnits[c4Index];
        if (punitCur->strName() == strToFind)
            return punitCur;
    }
    return 0;
}


//
//  We tell each unit to prepare itself for use. It will set itself to missing
//  state. Any that aren't found as the configuration is validated against
//  actually found units will be left in missing state. We also reset some
//  time stamps and such.
//
//  WE CANNOT set fields here since this configuration hasn't been actually
//  loaded yet. So we don't get a driver to use.
//
tCIDLib::TVoid TZWaveDrvConfig::PrepUnitsForUse()
{
    const tCIDLib::TCard4 c4Count = m_colUnits.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        m_colUnits[c4Index]->PrepForUse();
}


// Clear our all our data to default values
tCIDLib::TVoid TZWaveDrvConfig::Reset()
{
    for (tCIDLib::TCard4 c4Index = 0; c4Index < kZWaveLeviSh::c4MaxGroupId; c4Index++)
        m_colGroups[c4Index].Clear();
    m_colUnits.RemoveAll();
}


// Get the group name at a particular index (not group id, so zero based)
const TString& TZWaveDrvConfig::strGroupAt(const tCIDLib::TCard4 c4At) const
{
    return m_colGroups[c4At];
}


//
//  Set the name of a group by its group ID (one based.) THe name cannot
//  already exist except at the indicated id.
//
tCIDLib::TVoid
TZWaveDrvConfig::SetGroupById(  const   tCIDLib::TCard4 c4TarGrpId
                                , const TString&        strName)
{
    CIDAssert(c4TarGrpId > 0, L"Group ids cannot be zero");

    //
    //  See if the group name already exists. If so, it must be at the indicated
    //  index, in which case there's nothing to do since they are doing a
    //  redundant set. If it's at another index, it's a dup and we have to
    //  reject it.
    //
    tCIDLib::TCard4 c4GrpNum;
    if (bGroupExists(strName, c4GrpNum))
    {
        if (c4TarGrpId == c4GrpNum)
            return;

        // It's a dump have to reject
        facZWaveLeviSh().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kZWShErrs::errcCfg_DupGroup
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Duplicate
            , strName
        );
    }

    // Looks OK so store the name
    m_colGroups[c4TarGrpId - 1] = strName;
}


//
//  Set all the groups at once. This is for use by the group editor dialog,
//  so he doesn't have to set them back one at at time.
//
tCIDLib::TVoid TZWaveDrvConfig::SetGroups(const tCIDLib::TStrCollect& colToSet)
{
    if (colToSet.c4ElemCount() != kZWaveLeviSh::c4MaxGroupId)
    {
        facZWaveLeviSh().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kZWShErrs::errcCfg_WrongGroupCnt
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Duplicate
            , TCardinal(colToSet.c4ElemCount())
            , TCardinal(kZWaveLeviSh::c4MaxGroupId)
        );
    }

    TColCursor<TString>* pcursSet = colToSet.pcursNew();
    TJanitor<TColCursor<TString> > janCursor(pcursSet);
    pcursSet->bReset();
    tCIDLib::TCard4 c4Index = 0;
    do
    {
        m_colGroups[c4Index++] = pcursSet->objRCur();
    }   while(pcursSet->bNext());
}


// Sort the unit list
tCIDLib::TVoid TZWaveDrvConfig::SortUnitList()
{
    m_colUnits.Sort(&TZWaveUnit::eCompByName);
}


// Provide index based access to our units
TZWaveUnit& TZWaveDrvConfig::unitAt(const tCIDLib::TCard4 c4At)
{
    return *m_colUnits[c4At];
}

const TZWaveUnit& TZWaveDrvConfig::unitAt(const tCIDLib::TCard4 c4At) const
{
    return *m_colUnits[c4At];
}


//
//  Find an existing unit in our list with the passed unit id. If not found,
//  we throw.
//
const TZWaveUnit&
TZWaveDrvConfig::unitFindById(const tCIDLib::TCard4 c4ToFind) const
{
    const TZWaveUnit* punitRet = punitFindById(c4ToFind);
    if (!punitRet)
    {
        facZWaveLeviSh().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kZWShErrs::errcCfg_UnitIdNotFound
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , TCardinal(c4ToFind)
        );
    }
    return *punitRet;
}

TZWaveUnit& TZWaveDrvConfig::unitFindById(const tCIDLib::TCard4 c4ToFind)
{
    TZWaveUnit* punitRet = punitFindById(c4ToFind);
    if (!punitRet)
    {
        facZWaveLeviSh().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kZWShErrs::errcCfg_UnitIdNotFound
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , TCardinal(c4ToFind)
        );
    }
    return *punitRet;
}


//
//  Find a unit with the passed name. If not found, we throw. We hvae
//  const and non-const versions.
//
const TZWaveUnit&
TZWaveDrvConfig::unitFindByName(const TString& strToFind) const
{
    const TZWaveUnit* punitCheck = punitFindByName(strToFind);

    if (!punitCheck)
    {
        facZWaveLeviSh().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kZWShErrs::errcCfg_UnitNameNotFound
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , strToFind
        );
    }
    return *punitCheck;
}

TZWaveUnit& TZWaveDrvConfig::unitFindByName(const TString& strToFind)
{
    TZWaveUnit* punitCheck = punitFindByName(strToFind);
    if (!punitCheck)
    {
        facZWaveLeviSh().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kZWShErrs::errcCfg_UnitNameNotFound
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , strToFind
        );
    }
    return *punitCheck;
}



//
//  This is called when the client driver wants us to update a unit. We get
//  the modified unit info (where the id is the consistent bit of info that
//  lets us know which unit it is even if the name changed.) We check that the
//  new guy can be taken, and if so we replace the existing object with the
//  new one. The units are polymorphic so that's easier than trying to do a
//  polymorphic assignment of some type to copy the new guy into the old.
//
tCIDLib::TVoid
TZWaveDrvConfig::UpdateUnit(TZWaveUnit* const punitNew, TCQCServerBase& sdrvLevi)
{
    // Put a janitor on this guy unit's safely stored away
    TJanitor<TZWaveUnit> janNew(punitNew);

    // Find the unit with the passed id. The name may have changed
    TZWaveUnit* punitOld = 0;
    tCIDLib::TCard4 c4Index = 0;
    const tCIDLib::TCard4 c4Count = m_colUnits.c4ElemCount();
    for (; c4Index < c4Count; c4Index++)
    {
        TZWaveUnit* punitCur = m_colUnits[c4Index];
        if (punitCur->c4Id() == punitNew->c4Id())
        {
            // This is our guy, so save it and break out
            punitOld = punitCur;
            break;
        }
    }

    //
    //  If we didn't find it, then that's bad since he's only supposed to be
    //  updating existing units here.
    //
    if (!punitOld)
    {
        facZWaveLeviSh().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kZWShErrs::errcCfg_UnitIdNotFound
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , TCardinal(punitNew->c4Id())
        );
    }


    //
    //  If the name changed, we want to make sure that the new name is
    //  not anything that clashes with any existing field name.
    //
    const tCIDLib::TBoolean bNameChange(punitNew->strName() != punitOld->strName());
    if (bNameChange && sdrvLevi.bFieldExists(punitNew->strName()))
    {
        facZWaveLeviSh().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kZWShErrs::errcCfg_DupUnit
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Already
            , punitNew->strName()
        );
    }

    //
    //  Looks ok, so we now want to remove the old unit object from the list,
    //  and insert the new one in its place. The unit list is adopting so
    //  removing the old one will delete it, then orphan the new one out of
    //  the janitor into the list, sorted.
    //
    m_colUnits.RemoveAt(c4Index);
    tCIDLib::TCard4 c4At;
    m_colUnits.InsertSorted
    (
        janNew.pobjOrphan()
        , &TZWaveUnit::eCompByName
        , c4At
        , tCIDLib::ESortDirs::Ascending
    );
}


// ---------------------------------------------------------------------------
//  TZWaveDrvConfig: Protected, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TZWaveDrvConfig::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Make sure we get the start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion != ZWaveLeviSh_DrvConfig::c2FmtVersion))
    {
        facCIDLib().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCIDErrs::errcGen_UnknownFmtVersion
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(c2FmtVersion)
            , clsThis()
        );
    }

    tCIDLib::TCard4 c4UnitCnt;
    strmToReadFrom  >> m_colGroups
                    >> c4UnitCnt;

    // We have to polymorphically stream the units
    m_colUnits.RemoveAll();
    TPolyStreamer<TZWaveUnit> pstmrIn(&strmToReadFrom, 0);
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4UnitCnt; c4Index++)
    {
        TZWaveUnit* punitCur;
        punitCur = pstmrIn.pobjStreamIn();
        m_colUnits.Add(punitCur);
    }

    // Make sure we get the end marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TZWaveDrvConfig::StreamTo(TBinOutStream& strmToWriteTo) const
{
    const tCIDLib::TCard4 c4UnitCnt  = m_colUnits.c4ElemCount();

    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << ZWaveLeviSh_DrvConfig::c2FmtVersion
                    << m_colGroups
                    << c4UnitCnt;

    // We have to polymorphically stream the units
    TPolyStreamer<TZWaveUnit> pstmrIn(0, &strmToWriteTo);
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4UnitCnt; c4Index++)
        pstmrIn.StreamOut(*m_colUnits[c4Index]);

    strmToWriteTo   << tCIDLib::EStreamMarkers::EndObject;
}



