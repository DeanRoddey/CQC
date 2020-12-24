//
// FILE NAME: ZWaveUSBS_DrvConfig.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/02/2005
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
#include    "ZWaveUSBSh_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TZWaveDrvConfig,TObject)


// ---------------------------------------------------------------------------
//  Local types and constants
// ---------------------------------------------------------------------------
namespace ZWaveUSBS_DrvConfig
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

    m_colGroups(8)
    , m_colUnits(tCIDLib::EAdoptOpts::Adopt, 8)
{
    // Set any default content we need to have
    AddDefContent();
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
TZWaveDrvConfig& TZWaveDrvConfig::operator=(const TZWaveDrvConfig& dcfgSrc)
{
    if (this != &dcfgSrc)
    {
        m_colGroups = dcfgSrc.m_colGroups;

        // Polymorphically replicate the units
        m_colUnits.RemoveAll();
        const tCIDLib::TCard4 c4Count = dcfgSrc.m_colUnits.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            m_colUnits.Add(::pDupObject<TZWaveUnit>(dcfgSrc.m_colUnits[c4Index]));
    }
    return *this;
}


// We just have to compare our lists of units and groups
tCIDLib::TBoolean TZWaveDrvConfig::operator==(const TZWaveDrvConfig& dcfgSrc) const
{
    // Check the units
    tCIDLib::TCard4 c4Count = m_colUnits.c4ElemCount();
    if (c4Count != dcfgSrc.m_colUnits.c4ElemCount())
        return kCIDLib::False;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (m_colUnits[c4Index] != dcfgSrc.m_colUnits[c4Index])
            return kCIDLib::False;
    }

    // Check the groups
    c4Count = m_colGroups.c4ElemCount();
    if (c4Count != dcfgSrc.m_colGroups.c4ElemCount())
        return kCIDLib::False;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (m_colGroups[c4Index] != dcfgSrc.m_colGroups[c4Index])
            return kCIDLib::False;
    }
    return kCIDLib::True;
}

tCIDLib::TBoolean TZWaveDrvConfig::operator!=(const TZWaveDrvConfig& dcfgSrc) const
{
    return !operator==(dcfgSrc);
}


// ---------------------------------------------------------------------------
//  TZWaveDrvConfig: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Add the passed group to our list. We insure that neither its name or group
//  id is already in the list. We return true if we added it. Else it was
//  a dup and ignored.
//
tCIDLib::TBoolean TZWaveDrvConfig::bAddGroup(const TZWaveGroupInfo& grpiToAdd)
{
    // Make sure it's set up correct
    if (grpiToAdd.strName().bIsEmpty() || grpiToAdd.strId().bIsEmpty())
    {
        facZWaveUSBSh().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kZWShErrs::errcCfg_GroupNotCfg
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
        );
    }

    // Make sure the name or id don't already exist
    if (pgrpiFindByName(grpiToAdd.strName()) || pgrpiFindById(grpiToAdd.strId()))
        return kCIDLib::False;

    tCIDLib::TCard4 c4At;
    m_colGroups.InsertSorted
    (
        grpiToAdd, &TZWaveGroupInfo::eCompByName, c4At, tCIDLib::ESortDirs::Ascending
    );
    return kCIDLib::True;
}


//
//  Add the passed unit to our list, sorted by id. We insure that neither its
//  name or unit id is already in the list.
//
tCIDLib::TBoolean TZWaveDrvConfig::bAddUnit(TZWaveUnit* const punitToAdd)
{
    // Make sure it's correctly set up
    if ((punitToAdd->c1Id() == kCIDLib::c1MaxCard)
    ||  (punitToAdd->c1ClassGen() == kCIDLib::c1MaxCard))
    {
        // We have to delete it since we would have adopted if it worked
        delete punitToAdd;
        facZWaveUSBSh().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kZWShErrs::errcCfg_UnitNotCfg
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
        );
    }

    // Make sure that it doesn't alrady exist
    if (punitFindById(punitToAdd->c1Id())
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


// Returns true if we have no groups and units
tCIDLib::TBoolean TZWaveDrvConfig::bIsEmpty() const
{
    return m_colGroups.bIsEmpty() && m_colUnits.bIsEmpty();
}


// Returns true if the indicated group or unit name already exists
tCIDLib::TBoolean TZWaveDrvConfig::bGroupExists(const TString& strToCheck) const
{
    return (pgrpiFindByName(strToCheck) != 0);
}


tCIDLib::TBoolean TZWaveDrvConfig::bUnitExists(const TString& strToCheck) const
{
    return (punitFindByName(strToCheck) != 0);
}


//
//  Validate our configuration data. We'll remove any unit ids from groups
//  if those units don't exist anymore, and do any other sanity checking
//  required.
//
tCIDLib::TBoolean TZWaveDrvConfig::bValidate()
{
    // If no groups, we are done
    tCIDLib::TCard4 c4Count = m_colGroups.c4ElemCount();
    if (!c4Count)
        return kCIDLib::True;

    // Check the groups
    tCIDLib::TCard4 c4Index = 0;
    while (c4Index < c4Count)
    {
        TZWaveGroupInfo& grpiCur = m_colGroups[c4Index];

        //
        //  Remove any unit ids that this group references if they are no
        //  longer in our unit list.
        //
        tZWaveUSBSh::TIdList fcolCurU = grpiCur.fcolUnits();
        tCIDLib::TCard4 c4UCount = fcolCurU.c4ElemCount();
        tCIDLib::TCard4 c4UIndex = 0;
        while (c4UIndex < c4UCount)
        {
            if (punitFindById(fcolCurU[c4UIndex]))
            {
                c4UIndex++;
            }
             else
            {
                fcolCurU.Delete(c4UIndex);
                c4UCount--;
            }
        }
    }
    return kCIDLib::True;
}


// Const and non-const access to our group list
const tZWaveUSBSh::TGroupList& TZWaveDrvConfig::colGroups() const
{
    return m_colGroups;
}

tZWaveUSBSh::TGroupList& TZWaveDrvConfig::colGroups()
{
    return m_colGroups;
}


// Const and non-const access to our unit list
const tZWaveUSBSh::TUnitList& TZWaveDrvConfig::colUnits() const
{
    return m_colUnits;
}

tZWaveUSBSh::TUnitList& TZWaveDrvConfig::colUnits()
{
    return m_colUnits;
}


// Delete the group with the indicated id
tCIDLib::TVoid TZWaveDrvConfig::DeleteGroup(const TString& strGroupId)
{
    const tCIDLib::TCard4 c4Count = m_colGroups.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TZWaveGroupInfo& grpiCur = m_colGroups[c4Index];
        if (grpiCur.strId() == strGroupId)
        {
            m_colGroups.RemoveAt(c4Index);
            return;
        }
    }

    // We didn't find it, which is bad
    facZWaveUSBSh().ThrowErr
    (
        CID_FILE
        , CID_LINE
        , kZWShErrs::errcCfg_GrpIdNotFound
        , tCIDLib::ESeverities::Failed
        , tCIDLib::EErrClasses::NotFound
        , strGroupId
    );
}


//
//  Find a group with the passed name. If not found, we throw. We hvae
//  const and non-const versions.
//
const TZWaveGroupInfo&
TZWaveDrvConfig::grpiFindByName(const TString& strToFind) const
{
    const tCIDLib::TCard4 c4Count = m_colGroups.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TZWaveGroupInfo& grpiCur = m_colGroups[c4Index];
        if (grpiCur.strName() == strToFind)
            return grpiCur;
    }

    facZWaveUSBSh().ThrowErr
    (
        CID_FILE
        , CID_LINE
        , kZWShErrs::errcCfg_GroupNotFound
        , tCIDLib::ESeverities::Failed
        , tCIDLib::EErrClasses::NotFound
        , strToFind
    );

    // Make the compiler happy, but we'll never get here
    return m_colGroups[0];
}

TZWaveGroupInfo& TZWaveDrvConfig::grpiFindByName(const TString& strToFind)
{
    const tCIDLib::TCard4 c4Count = m_colGroups.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TZWaveGroupInfo& grpiCur = m_colGroups[c4Index];
        if (grpiCur.strName() == strToFind)
            return grpiCur;
    }

    facZWaveUSBSh().ThrowErr
    (
        CID_FILE
        , CID_LINE
        , kZWShErrs::errcCfg_GroupNotFound
        , tCIDLib::ESeverities::Failed
        , tCIDLib::EErrClasses::NotFound
        , strToFind
    );

    // Make the compiler happy, but we'll never get here
    return m_colGroups[0];
}


//
//  Find any existing group in our list with the passed group id. If not found,
//  we return a null pointer.
//
TZWaveGroupInfo*
TZWaveDrvConfig::pgrpiFindByFldId(const tCIDLib::TCard4 c4ToFind)
{
    const tCIDLib::TCard4 c4Count = m_colGroups.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TZWaveGroupInfo& grpiCur = m_colGroups[c4Index];
        if (grpiCur.c4FldId() == c4ToFind)
            return &grpiCur;
    }
    return 0;
}


//
//  Find any existing group in our list with the passed name. If not found,
//  we return a null pointer.
//
const TZWaveGroupInfo*
TZWaveDrvConfig::pgrpiFindByName(const TString& strToFind) const
{
    const tCIDLib::TCard4 c4Count = m_colGroups.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TZWaveGroupInfo& grpiCur = m_colGroups[c4Index];
        if (grpiCur.strName() == strToFind)
            return &grpiCur;
    }
    return 0;
}

TZWaveGroupInfo*
TZWaveDrvConfig::pgrpiFindByName(const TString& strToFind)
{
    const tCIDLib::TCard4 c4Count = m_colGroups.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TZWaveGroupInfo& grpiCur = m_colGroups[c4Index];
        if (grpiCur.strName() == strToFind)
            return &grpiCur;
    }
    return 0;
}


//
//  Find an existing unit in our list with the passed unit id. If not found,
//  we return a null pointer.
//
const TZWaveUnit*
TZWaveDrvConfig::punitFindById(const tCIDLib::TCard1 c1ToFind) const
{
    const tCIDLib::TCard4 c4Count = m_colUnits.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TZWaveUnit* punitCur = m_colUnits[c4Index];
        if (punitCur->c1Id() == c1ToFind)
            return punitCur;
    }
    return 0;
}

TZWaveUnit* TZWaveDrvConfig::punitFindById(const tCIDLib::TCard1 c1ToFind)
{
    const tCIDLib::TCard4 c4Count = m_colUnits.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TZWaveUnit* punitCur = m_colUnits[c4Index];
        if (punitCur->c1Id() == c1ToFind)
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


// Clear our all our data to default values
tCIDLib::TVoid TZWaveDrvConfig::Reset()
{
    m_colGroups.RemoveAll();
    m_colUnits.RemoveAll();

    // Load any default content
    AddDefContent();
}


// Sort the two lists by their names
tCIDLib::TVoid TZWaveDrvConfig::SortLists()
{
    m_colGroups.Sort(&TZWaveGroupInfo::eCompByName);
    m_colUnits.Sort(&TZWaveUnit::eCompByName);
}


//
//  Find an existing unit in our list with the passed unit id. If not found,
//  we throw.
//
const TZWaveUnit&
TZWaveDrvConfig::unitFindById(const tCIDLib::TCard1 c1ToFind) const
{
    const TZWaveUnit* punitRet = punitFindById(c1ToFind);
    if (!punitRet)
    {
        facZWaveUSBSh().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kZWShErrs::errcCfg_UnitIdNotFound
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , TCardinal(c1ToFind)
        );
    }
    return * punitRet;
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
        facZWaveUSBSh().ThrowErr
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
        facZWaveUSBSh().ThrowErr
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
//  This is called when the client driver wants us to update a group. We get
//  the modified group info (where the id is the consistent bit of info that
//  let's us know which group it is.) We check that the new guy can be taken,
//  and if so we copy in the new info.
//
//  Note that it can be a new group, but he has to tell us explicitly, so that
//  we can know that's what he intends.
//
tCIDLib::TVoid
TZWaveDrvConfig::UpdateGroup(const  TZWaveGroupInfo&        grpiNew
                            ,       TCQCServerBase* const   psdrvSrc
                            , const tCIDLib::TBoolean       bNewGroup)
{
    // Find the group with the passed id. The name may have changed
    const tCIDLib::TCard4 c4Count = m_colGroups.c4ElemCount();
    tCIDLib::TCard4 c4Index = 0;
    const TString& strGrpId = grpiNew.strId();
    for (; c4Index < c4Count; c4Index++)
    {
        TZWaveGroupInfo& grpiCur = m_colGroups[c4Index];
        if (grpiCur.strId() == strGrpId)
            break;
    }

    // Based on whether we found it or not, validate it
    if (c4Index == c4Count)
    {
        // We didn't find it, so it must be a new group
        if (!bNewGroup)
        {
            facZWaveUSBSh().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kZWShErrs::errcCfg_GrpIdNotFound
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::NotFound
                , strGrpId
            );
        }

        //
        //  If we got a server driver pointer, make sure the name isn't going
        //  to be the same as another field name. If not, then we can ony
        //  check against the unit names (we just checked the groups.)
        //
        if ((psdrvSrc && psdrvSrc->bFieldExists(grpiNew.strName()))
        ||  bUnitExists(grpiNew.strName()))
        {
            facZWaveUSBSh().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kZWShErrs::errcCfg_FldNameUsed
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Already
                , grpiNew.strName()
            );
        }

        // Looks reasonable, so let's add it to our list
        m_colGroups.objAdd(grpiNew);
    }
     else
    {
        // We found it, so it cannot be a new one
        if (bNewGroup)
        {
            facZWaveUSBSh().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kZWShErrs::errcCfg_DupGroup
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Already
                , grpiNew.strId()
            );
        }

        //
        //  If the name changed, then see if this field name is taken. We
        //  use the server driver if that was provided, else we can only
        //  check it against the units (we already know it can't match a
        //  group since we are replacing that one.)
        //
        TZWaveGroupInfo& grpiCur = m_colGroups[c4Index];
        const tCIDLib::TBoolean bNameChange
        (
            grpiNew.strName() != grpiCur.strName()
        );

        if (bNameChange
        &&  ((psdrvSrc && psdrvSrc->bFieldExists(grpiNew.strName()))
        ||   bUnitExists(grpiNew.strName())))
        {
            facZWaveUSBSh().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kZWShErrs::errcCfg_FldNameUsed
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Already
                , grpiNew.strName()
            );
        }

        // Ok, looks ok, so copy the new stuff in
        grpiCur = grpiNew;

        // If the name changed, then resort the list
        if (bNameChange)
            m_colGroups.Sort(&TZWaveGroupInfo::eCompByName);
    }
}


//
//  This is called when the client driver wants us to update a unit. We get
//  the modified unit info (where the id is the consistent bit of info that
//  let's us know which unit it is.) We check that the new guy can be taken,
//  and if so we replace the existing object with the new one. The units are
//  polymorphic so that's easier than trying to do a polymorphic assignment
//  of some type to copy the new guy into the old.
//
tCIDLib::TVoid
TZWaveDrvConfig::UpdateUnit(TZWaveUnit* const       punitNew
                            , TCQCServerBase* const psdrvSrc)
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

        if (punitCur->c1Id() == punitNew->c1Id())
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
        facZWaveUSBSh().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kZWShErrs::errcCfg_UnitIdNotFound
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , TCardinal(punitNew->c1Id())
        );
    }


    //
    //  If the name changed, we want to make sure that the new name is
    //  not anything that clashes with any existing field name.
    //
    const tCIDLib::TBoolean bNameChange(punitNew->strName() != punitOld->strName());
    if (bNameChange
    &&  psdrvSrc
    &&  psdrvSrc->bFieldExists(punitNew->strName()))
    {
        facZWaveUSBSh().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kZWShErrs::errcCfg_FldNameUsed
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
    if (!c2FmtVersion || (c2FmtVersion != ZWaveUSBS_DrvConfig::c2FmtVersion))
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
                    << ZWaveUSBS_DrvConfig::c2FmtVersion
                    << m_colGroups
                    << c4UnitCnt;

    // We have to polymorphically stream the units
    TPolyStreamer<TZWaveUnit> pstmrIn(0, &strmToWriteTo);
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4UnitCnt; c4Index++)
        pstmrIn.StreamOut(*m_colUnits[c4Index]);

    strmToWriteTo   << tCIDLib::EStreamMarkers::EndObject;
}


// ---------------------------------------------------------------------------
//  TZWaveDrvConfig: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  This is called after a reset or default ctor, to set up any default
//  units or groups or whatever that are required to be present in a default
//  configuration.
//
tCIDLib::TVoid TZWaveDrvConfig::AddDefContent()
{
    // Add a default 'AllOnOff' group, which they will usually want to have
    TZWaveGroupInfo grpiAll(L"AllOnOff");
    m_colGroups.objAdd(grpiAll);
}


const TZWaveGroupInfo*
TZWaveDrvConfig::pgrpiFindById(const TString& strIdToFind) const
{
    const tCIDLib::TCard4 c4Count = m_colGroups.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TZWaveGroupInfo& grpiCur = m_colGroups[c4Index];
        if (grpiCur.strId() == strIdToFind)
            return &grpiCur;
    }
    return 0;
}

TZWaveGroupInfo*
TZWaveDrvConfig::pgrpiFindById(const TString& strIdToFind)
{
    const tCIDLib::TCard4 c4Count = m_colGroups.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TZWaveGroupInfo& grpiCur = m_colGroups[c4Index];
        if (grpiCur.strId() == strIdToFind)
            return &grpiCur;
    }
    return 0;
}


