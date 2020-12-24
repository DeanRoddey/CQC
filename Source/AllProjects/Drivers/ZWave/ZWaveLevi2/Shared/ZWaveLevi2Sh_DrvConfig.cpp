//
// FILE NAME: ZWaveLevi2Sh_DrvConfig.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/10/2013
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
#include    "ZWaveLevi2Sh_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TZWaveDrvConfig,TObject)


// ---------------------------------------------------------------------------
//  Local types and constants
// ---------------------------------------------------------------------------
namespace ZWaveLevi2Sh_DrvConfig
{
    // -----------------------------------------------------------------------
    //  Our persistent format version
    // -----------------------------------------------------------------------
    const tCIDLib::TCard2   c2FmtVersion = 1;


    // -----------------------------------------------------------------------
    //  Used to fault in device info files upon access. We key it off of the file
    //  name (the base part of the file name which is what is stored in the unit
    //  objects.) We fault this collection in as well. It's set up as a thread
    //  safe list so we can use its own lock to sync.
    // -----------------------------------------------------------------------
    TKeyedHashSet<TZWDevInfo, TString, TStringKeyOps>*  pcolDevInfoFiles;

}




// ---------------------------------------------------------------------------
//   CLASS: TZWaveDrvConfig
//  PREFIX: dcfg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWaveDrvConfig: Public, static methods
// ---------------------------------------------------------------------------

//
//  Looks up a given device info file, faulting it if not already loaded. If we can't
//  find it or parse it, we return zero. We will only parse it if they provide us with
//  a parser. Otherwise they expect it to be already loaded, and we return it if we
//  already have it.
//
TZWDevInfo*
TZWaveDrvConfig::pzwdiFind(const TString& strFileName, TXMLTreeParser* const pxtprsToUse)
{
    // If we haven't faulted in the list yet, then do that
    if (!TAtomic::pFencedGet(&ZWaveLevi2Sh_DrvConfig::pcolDevInfoFiles))
    {
        TBaseLock lockInit;
        if (!TAtomic::pFencedGet(&ZWaveLevi2Sh_DrvConfig::pcolDevInfoFiles))
        {
            TAtomic::FencedSet
            (
                &ZWaveLevi2Sh_DrvConfig::pcolDevInfoFiles
                , new TKeyedHashSet<TZWDevInfo, TString, TStringKeyOps>
                  (
                    109, TStringKeyOps(), &TZWDevInfo::strKey, tCIDLib::EMTStates::Safe
                  )
            );
        }
    }

    // OK, let's lock the collection and see if we can find what they want
    TZWDevInfo* pzwdiRet = nullptr;
    {
        TLocker lockrSync(ZWaveLevi2Sh_DrvConfig::pcolDevInfoFiles);
        pzwdiRet =  ZWaveLevi2Sh_DrvConfig::pcolDevInfoFiles->pobjFindByKey(strFileName);

        // If not there already, then let's try to load it if we were given a parser
        if (!pzwdiRet && pxtprsToUse)
        {
            // Build up the path to the requested device
            TPathStr    pathFile(facCQCKit().strLocalDataDir());
            pathFile.AddLevel(L"ZWInfo");
            pathFile.AddLevel(strFileName);
            pathFile.AppendExt(L"ZWInfo");

            try
            {
                if (pxtprsToUse->bParseRootEntity(  pathFile
                                                    , tCIDXML::EParseOpts::Validate
                                                    , tCIDXML::EParseFlags::TagsNText))
                {
                    TZWDevInfo zwdiCur;
                    zwdiCur.ParseFromXML(strFileName, pxtprsToUse->xtdocThis().xtnodeRoot());

                    // It worked so add it
                    pzwdiRet = &ZWaveLevi2Sh_DrvConfig::pcolDevInfoFiles->objAdd(zwdiCur);
                }
                 else
                {
                    const TXMLTreeParser::TErrInfo& erriFirst = pxtprsToUse->colErrors()[0];

                    TTextStringOutStream strmOut(512);
                    strmOut << L"[" << erriFirst.strSystemId() << L"/"
                            << erriFirst.c4Line() << L"." << erriFirst.c4Column()
                            << L"] " << erriFirst.strText() << kCIDLib::FlushIt;

                    facZWaveLevi2Sh().ThrowErr
                    (
                        CID_FILE
                        , CID_LINE
                        , kZW2ShErrs::errcDevI_ParseFailed
                        , strmOut.strData()
                        , tCIDLib::ESeverities::Failed
                        , tCIDLib::EErrClasses::Format
                        , strFileName
                    );
                }
            }

            catch(TError& errToCatch)
            {
                if (!errToCatch.bLogged())
                {
                    errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                    TModule::LogEventObj(errToCatch);
                }
            }
        }
    }

    // And return if it we got it, else a zero
    return pzwdiRet;
}



// ---------------------------------------------------------------------------
//  TZWaveDrvConfig: Constructors and Destructor
// ---------------------------------------------------------------------------
TZWaveDrvConfig::TZWaveDrvConfig() :

    m_colGroups(kZWaveLevi2Sh::c4MaxGroupId)
    , m_colUnits(8)
{
}

TZWaveDrvConfig::TZWaveDrvConfig(const TZWaveDrvConfig& dcfgSrc) :

    m_colGroups(dcfgSrc.m_colGroups)
    , m_colUnits(dcfgSrc.m_colUnits)
{
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
        m_colUnits  = dcfgSrc.m_colUnits;
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
        if (m_colUnits[c4Index] != dcfgSrc.m_colUnits[c4Index])
            return kCIDLib::False;
    }

    // Compare the group names. These lists are fixed size so always equal sized
    const tCIDLib::TCard4 c4GrpCount = m_colGroups.tElemCount();
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
tCIDLib::TBoolean
TZWaveDrvConfig::bAddUnit(  const   tCIDLib::TCard4             c4Id
                            , const tZWaveLevi2Sh::EGenTypes    eGenType)
{
    // Make sure it's correctly set up
    if ((c4Id == kCIDLib::c4MaxCard)
    ||  (eGenType == tZWaveLevi2Sh::EGenTypes::None))
    {
        facZWaveLevi2Sh().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kZW2ShErrs::errcCfg_UnitNotCfg
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
        );
    }

    // Make sure that it doesn't alrady exist
    if (punitFindById(c4Id))
        return kCIDLib::False;

    // Make sure the list isn't full
    m_colUnits.CheckIsFull(kZWaveLevi2Sh::c4MaxUnits, L"Z-Wave unit list");

    tCIDLib::TCard4 c4At;
    TZWaveUnit unitNew(c4Id, eGenType);
    m_colUnits.InsertSorted(unitNew, &TZWaveUnit::eCompByName, c4At);
    return kCIDLib::True;
}


//
//  A helper for the client side driver. Unlike punitFindByName(), this does a case
//  insensitive check, so that even if the user enters it with a different cause, it
//  can warn them about it.
//
tCIDLib::TBoolean TZWaveDrvConfig::bCheckDupName(const TString& strToCheck) const
{
    const tCIDLib::TCard4 c4Count = m_colUnits.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TZWaveUnit& unitCur = m_colUnits[c4Index];
        if (unitCur.strName().bCompareI(strToCheck))
            return kCIDLib::True;
    }
    return kCIDLib::False;
}


// Returns true if we have no units and all groups are empty
tCIDLib::TBoolean TZWaveDrvConfig::bIsEmpty() const
{
    for (tCIDLib::TCard4 c4Index = 0; c4Index < kZWaveLevi2Sh::c4MaxGroupId; c4Index++)
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
    for (tCIDLib::TCard4 c4Index = 0; c4Index < kZWaveLevi2Sh::c4MaxGroupId; c4Index++)
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
//  Search for a controller unit that is marked as the VRCOP controller.
//
tCIDLib::TCard4 TZWaveDrvConfig::c4FindVRCOP(const tCIDLib::TBoolean bThrowIfNot) const
{
    const tCIDLib::TCard4 c4UnitCnt = m_colUnits.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4UnitCnt; c4Index++)
    {
        const TZWaveUnit& unitCur = m_colUnits[c4Index];

        if ((unitCur.eGenType() == tZWaveLevi2Sh::EGenTypes::GenCtrl)
        &&  (unitCur.eSpecType() == tZWaveLevi2Sh::ESpecTypes::VRCOP))
        {
            return unitCur.c4Id();
        }
    }

    if (bThrowIfNot)
    {
        facZWaveLevi2Sh().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kZW2ShErrs::errcCfg_NoVRCOP
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
        );
    }

    // Return zero (an invalid id) since we never found it
    return 0;
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
    for (tCIDLib::TCard4 c4Index = 0; c4Index < kZWaveLevi2Sh::c4MaxGroupId; c4Index++)
    {
        if (m_colGroups[c4Index] == strGroupId)
        {
            m_colGroups[c4Index].Clear();
            break;
        }
    }
}


// Find the unit with the indicated id and delete it
tCIDLib::TVoid TZWaveDrvConfig::DeleteUnit(const tCIDLib::TCard4 c4UnitId)
{
    const tCIDLib::TCard4 c4UnitCnt = m_colUnits.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4UnitCnt; c4Index++)
    {
        TZWaveUnit& unitCur = m_colUnits[c4Index];
        if (unitCur.c4Id() == c4UnitId)
        {
            m_colUnits.RemoveAt(c4Index);
            break;
        }
    }
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
        facZWaveLevi2Sh().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kZW2ShErrs::errcCfg_GroupNotFound
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , strName
        );
    }
}


//
//  At this point the driver has asked all of the units to register their fields, and
//  is now giving them a chance to do any init before they go live. They should set
//  all their readable fields to error state, so that they will be that way until good
//  data is stored, and they can do any other (local) setup they need, i.e. nothing
//  that requires talking to the device.
//
//  Those that fail this step will be marked as failed.
//
tCIDLib::TVoid TZWaveDrvConfig::InitVals(MZWLeviSrvFuncs& mzwsfLevi)
{
    const tCIDLib::TCard4 c4UnitCnt = m_colUnits.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4UnitCnt; c4Index++)
    {
        TZWaveUnit& unitCur = m_colUnits[c4Index];
        if (!unitCur.bIsViable())
            continue;

        try
        {
            unitCur.InitVals(mzwsfLevi);

            // Make sure the failed flag is cleared
            unitCur.bFailed(kCIDLib::False);
        }

        catch(TError& errToCatch)
        {
            // Revert it back to failed
            unitCur.bFailed(kCIDLib::True);

            if (mzwsfLevi.eLeviVerboseLevel() >= tCQCKit::EVerboseLvls::Low)
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);

                facZWaveLevi2Sh().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , L"Unit %(1) (%(2)) failed to initialize values"
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::AppStatus
                    , unitCur.strName()
                    , TCardinal(unitCur.c4Id())
                );
            }
        }
    }
}


//
//  This will go through all of the units and let them load their device info. This
//  is necessary to get them ready for use. Only the device info file key is stored,
//  and it must be used to reload the device info, and that is then used to set up the
//  command class handles on the unis and so forth.
//
//  Only those that have had the device info file key set on them are loaded. That is
//  done in the client driver interface by the user, who selects the device type of
//  each unit.
//
//  We have one that does it for a single unit, and another that loads them all.
//
tCIDLib::TVoid
TZWaveDrvConfig::LoadDevInfo(MZWLeviSrvFuncs&   mzwsfLevi
                            , TZWaveUnit&       unitTar
                            , TXMLTreeParser&   xtprsToUse)
{
    if (!unitTar.strDevInfoFile().bIsEmpty())
    {
        if (unitTar.bLoadDevInfo(xtprsToUse))
        {
            // Make sure the failed flag is cleared
            unitTar.bFailed(kCIDLib::False);
        }
         else
        {
            unitTar.bFailed(kCIDLib::True);
            facZWaveLevi2Sh().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kZW2ShErrs::errcDevI_CantSetDevInfo
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::NotFound
                , unitTar.strDevInfoFile()
                , unitTar.strName()
            );
        }
    }
}


tCIDLib::TVoid TZWaveDrvConfig::LoadDevInfo(MZWLeviSrvFuncs& mzwsfLevi)
{
    //
    //  Now let's find all of the device info objects needed and set them on the
    //  units. This needs to be done before the driver starts messing around with
    //  them, since most of the required info from from the loaded info objects.
    //
    const tCIDLib::TCard4 c4UnitCnt = m_colUnits.c4ElemCount();
    if (!c4UnitCnt)
        return;

    // We need an XML parser to parse the files with if we hit any not yet loaded
    TXMLTreeParser  xtprsToUse;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4UnitCnt; c4Index++)
    {
        TZWaveUnit& unitCur = m_colUnits[c4Index];

        //
        //  Call the single unit version to update this one if not already and we have
        //  device info set.
        //
        LoadDevInfo(mzwsfLevi, unitCur, xtprsToUse);
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
        const TZWaveUnit& unitCur = m_colUnits[c4Index];
        if (unitCur.c4Id() == c4ToFind)
            return &unitCur;
    }
    return nullptr;
}

TZWaveUnit* TZWaveDrvConfig::punitFindById(const tCIDLib::TCard4 c4ToFind)
{
    const tCIDLib::TCard4 c4Count = m_colUnits.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TZWaveUnit& unitCur = m_colUnits[c4Index];
        if (unitCur.c4Id() == c4ToFind)
            return &unitCur;
    }
    return nullptr;
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
        const TZWaveUnit& unitCur = m_colUnits[c4Index];
        if (unitCur.strName() == strToFind)
            return &unitCur;
    }
    return nullptr;
}

TZWaveUnit*
TZWaveDrvConfig::punitFindByName(const TString& strToFind)
{
    const tCIDLib::TCard4 c4Count = m_colUnits.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TZWaveUnit& unitCur = m_colUnits[c4Index];
        if (unitCur.strName() == strToFind)
            return &unitCur;
    }
    return nullptr;
}


//
//  This is used to rename a unit, which is special case becasue of duplication
//  concerns and sorting. Use UpdateUnit() to update other values.
//
tCIDLib::TVoid
TZWaveDrvConfig::RenameUnit(const   tCIDLib::TCard4     c4Id
                            , const TString&            strNewName
                            ,       MZWLeviSrvFuncs&    mzwsfLevi
                            ,       tCIDLib::TCard4&    c4NewIndex)
{
    // Find the unit with the passed id
    TZWaveUnit* punitOld = nullptr;
    const tCIDLib::TCard4 c4Count = m_colUnits.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TZWaveUnit& unitCur = m_colUnits[c4Index];
        if (unitCur.c4Id() == c4Id)
        {
            // This is our guy, so save it and break out
            punitOld = &unitCur;
            break;
        }
    }

    //
    //  If we didn't find it, then that's bad since he's only supposed to be
    //  updating existing units here.
    //
    if (!punitOld)
    {
        facZWaveLevi2Sh().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kZW2ShErrs::errcCfg_UnitIdNotFound
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , TCardinal(c4Id)
        );
    }

    // If the name didn't change, then nothing to do
    const tCIDLib::TBoolean bNameChange(strNewName != punitOld->strName());
    if (!bNameChange)
        return;


    // Look up this new name
    const TZWaveUnit* punitDup = punitFindByName(strNewName);

    // If we found it, then it's a dup
    if (punitDup)
    {
        facZWaveLevi2Sh().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kZW2ShErrs::errcCfg_DupUnit
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Already
            , strNewName
        );
    }

    // Looks ok, so update the name and resort
    punitOld->strName(strNewName);
    m_colUnits.Sort(&TZWaveUnit::eCompByName);

    // Find the new index so that we can return it
    c4NewIndex = 0;
    for (; c4NewIndex < c4Count; c4NewIndex++)
    {
        TZWaveUnit& unitCur = m_colUnits[c4NewIndex];
        if (unitCur.c4Id() == c4Id)
            break;
    }

    CIDAssert(c4NewIndex < c4Count, L"Did not find unit again after rename");
}



// Clear our all our data to default values
tCIDLib::TVoid TZWaveDrvConfig::Reset()
{
    for (tCIDLib::TCard4 c4Index = 0; c4Index < kZWaveLevi2Sh::c4MaxGroupId; c4Index++)
        m_colGroups[c4Index].Clear();
    m_colUnits.RemoveAll();
}


//
//  Reset a unit to a new generic type, which puts it back as though it was just
//  created, with the indicated generic type.
//
tCIDLib::TVoid
TZWaveDrvConfig::ResetUnitType( const   tCIDLib::TCard4             c4Id
                                , const tZWaveLevi2Sh::EGenTypes    eType)
{
    unitFindById(c4Id).ResetUnitType(eType);
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
        facZWaveLevi2Sh().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kZW2ShErrs::errcCfg_DupGroup
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
    if (colToSet.c4ElemCount() != kZWaveLevi2Sh::c4MaxGroupId)
    {
        facZWaveLevi2Sh().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kZW2ShErrs::errcCfg_WrongGroupCnt
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Duplicate
            , TCardinal(colToSet.c4ElemCount())
            , TCardinal(kZWaveLevi2Sh::c4MaxGroupId)
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
    return m_colUnits[c4At];
}

const TZWaveUnit& TZWaveDrvConfig::unitAt(const tCIDLib::TCard4 c4At) const
{
    return m_colUnits[c4At];
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
        facZWaveLevi2Sh().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kZW2ShErrs::errcCfg_UnitIdNotFound
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
        facZWaveLevi2Sh().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kZW2ShErrs::errcCfg_UnitIdNotFound
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
        facZWaveLevi2Sh().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kZW2ShErrs::errcCfg_UnitNameNotFound
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
        facZWaveLevi2Sh().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kZW2ShErrs::errcCfg_UnitNameNotFound
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , strToFind
        );
    }
    return *punitCheck;
}


//
//  This is called when the client driver wants us to update a unit's info, but this
//  cannot change the name. It's just to update the other stuff. So the caller knows
//  that the unit won't have change its place in the list.
//
tCIDLib::TVoid
TZWaveDrvConfig::UpdateUnit(const TZWaveUnit& unitNew, MZWLeviSrvFuncs& mzwsfLevi)
{
    // Find the unit with the passed id. The name may have changed
    TZWaveUnit* punitTar = 0;
    const tCIDLib::TCard4 c4Count = m_colUnits.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TZWaveUnit& unitCur = m_colUnits[c4Index];
        if (unitCur.c4Id() == unitNew.c4Id())
        {
            // This is our guy, so save it and break out
            punitTar = &unitCur;
            break;
        }
    }

    //
    //  If we didn't find it, then that's bad since he's only supposed to be
    //  updating existing units here.
    //
    if (!punitTar)
    {
        facZWaveLevi2Sh().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kZW2ShErrs::errcCfg_UnitIdNotFound
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , TCardinal(unitNew.c4Id())
        );
    }

    if (unitNew.strName() != punitTar->strName())
    {
        // Not allowed here
        facZWaveLevi2Sh().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kZW2ShErrs::errcCfg_CantRename
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::CantDo
        );
    }

    // Just assign to it since we didn't update the name
    *punitTar = unitNew;
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
    if (!c2FmtVersion || (c2FmtVersion != ZWaveLevi2Sh_DrvConfig::c2FmtVersion))
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

    tCIDLib::TCard4 c4GroupCnt, c4UnitCnt;
    strmToReadFrom  >> c4GroupCnt
                    >> c4UnitCnt;

    // Stream in the groups and units
    TString strGroup;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4GroupCnt; c4Index++)
    {
        strmToReadFrom >> strGroup;
        m_colGroups[c4Index] = strGroup;
    }

    TZWaveUnit unitCur;
    m_colUnits.RemoveAll();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4UnitCnt; c4Index++)
    {
        strmToReadFrom >> unitCur;
        m_colUnits.objAdd(unitCur);
    }

    // Make sure we get the end marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TZWaveDrvConfig::StreamTo(TBinOutStream& strmToWriteTo) const
{
    const tCIDLib::TCard4 c4GroupCnt  = m_colGroups.tElemCount();
    const tCIDLib::TCard4 c4UnitCnt  = m_colUnits.c4ElemCount();

    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << ZWaveLevi2Sh_DrvConfig::c2FmtVersion
                    << c4GroupCnt
                    << c4UnitCnt;

    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4GroupCnt; c4Index++)
        strmToWriteTo << m_colGroups[c4Index];

    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4UnitCnt; c4Index++)
        strmToWriteTo << m_colUnits[c4Index];

    strmToWriteTo   << tCIDLib::EStreamMarkers::EndObject;
}



