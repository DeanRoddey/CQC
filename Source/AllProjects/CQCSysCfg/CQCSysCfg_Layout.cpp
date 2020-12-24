//
// FILE NAME: CQCSysCfg_Layout.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/13/2011
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
//  This file implements the main system configuration classes.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCSysCfg_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCSysCfg,TObject)



namespace CQCKit_Layout
{
    namespace
    {
        // -----------------------------------------------------------------------
        //  The persistent format number for the overall system configuration class.
        //
        //  Version 1 -
        //      We changed over to a new format for version 4.5, with a different file
        //      name on the server side, so we reverted back to V1.
        //
        //  Version 2 -
        //      Added support for thermos
        //
        //  Version 3 -
        //      Added support for a custom floor layout template.
        //
        //  Version 4 -
        //      And an option to use he above instead of the standard lighting screen as
        //      well. It's always used for security if that is enabled.
        //
        //  Version 5 -
        //      Any conversions required for 5.x, mostly path formats
        //
        //  Version 6 -
        //      Added a serial number that we can bump each time it is modified.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2 c2FmtSysCfg   = 6;
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCSysCfg
//  PREFIX: scfg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCSysCfg: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCSysCfg::TCQCSysCfg() :

    m_bUseCustLights(kCIDLib::False)
    , m_c4SerialNum(1)
    , m_colHVACList(tCIDLib::EAdoptOpts::Adopt)
    , m_colLoadList(tCIDLib::EAdoptOpts::Adopt)
    , m_colMediaList(tCIDLib::EAdoptOpts::Adopt)
    , m_colRoomList(4UL)
    , m_colResList(2UL)
    , m_colSecList(tCIDLib::EAdoptOpts::Adopt)
    , m_colWeatherList(tCIDLib::EAdoptOpts::Adopt)
    , m_colXOverList(tCIDLib::EAdoptOpts::Adopt)
{
}

TCQCSysCfg::TCQCSysCfg(const TCQCSysCfg& scfgSrc) :

    m_bUseCustLights(scfgSrc.m_bUseCustLights)
    , m_c4SerialNum(scfgSrc.m_c4SerialNum)
    , m_colHVACList
      (
        tCIDLib::EAdoptOpts::Adopt, scfgSrc.m_colHVACList.c4ElemCount()
      )
    , m_colLoadList
      (
        tCIDLib::EAdoptOpts::Adopt, scfgSrc.m_colLoadList.c4ElemCount()
      )
    , m_colMediaList
      (
        tCIDLib::EAdoptOpts::Adopt, scfgSrc.m_colMediaList.c4ElemCount()
      )
    , m_colResList(scfgSrc.m_colResList)
    , m_colRoomList(scfgSrc.m_colRoomList)
    , m_colSecList
      (
        tCIDLib::EAdoptOpts::Adopt, scfgSrc.m_colSecList.c4ElemCount()
      )
    , m_colWeatherList
      (
        tCIDLib::EAdoptOpts::Adopt, scfgSrc.m_colWeatherList.c4ElemCount()
      )
    , m_colXOverList
      (
        tCIDLib::EAdoptOpts::Adopt, scfgSrc.m_colXOverList.c4ElemCount()
      )
    , m_strCustLayout(scfgSrc.m_strCustLayout)
    , m_strTarScope(scfgSrc.m_strTarScope)
{
    CopyList(scfgSrc.m_colHVACList, m_colHVACList);
    CopyList(scfgSrc.m_colLoadList, m_colLoadList);
    CopyList(scfgSrc.m_colMediaList, m_colMediaList);
    CopyList(scfgSrc.m_colSecList, m_colSecList);
    CopyList(scfgSrc.m_colWeatherList, m_colWeatherList);
    CopyList(scfgSrc.m_colXOverList, m_colXOverList);
}

TCQCSysCfg::~TCQCSysCfg()
{
}


// ---------------------------------------------------------------------------
//  TCQCSysCfg: Public operators
// ---------------------------------------------------------------------------
TCQCSysCfg& TCQCSysCfg::operator=(const TCQCSysCfg& scfgSrc)
{
    if (&scfgSrc != this)
    {
        m_bUseCustLights= scfgSrc.m_bUseCustLights;
        m_c4SerialNum   = scfgSrc.m_c4SerialNum;
        m_colResList    = scfgSrc.m_colResList;
        m_colRoomList   = scfgSrc.m_colRoomList;
        m_strCustLayout = scfgSrc.m_strCustLayout;
        m_strTarScope   = scfgSrc.m_strTarScope;

        CopyList(scfgSrc.m_colHVACList, m_colHVACList);
        CopyList(scfgSrc.m_colLoadList, m_colLoadList);
        CopyList(scfgSrc.m_colMediaList, m_colMediaList);
        CopyList(scfgSrc.m_colSecList, m_colSecList);
        CopyList(scfgSrc.m_colWeatherList, m_colWeatherList);
        CopyList(scfgSrc.m_colXOverList, m_colXOverList);
    }
    return *this;
}


//
//  The serial number is not included in the comparison. It's just for persistence
//  over time.
//
tCIDLib::TBoolean TCQCSysCfg::operator==(const TCQCSysCfg& scfgSrc) const
{
    // Check the non-list stuff
    if ((m_strTarScope != scfgSrc.m_strTarScope)
    ||  (m_strCustLayout != scfgSrc.m_strCustLayout)
    ||  (m_bUseCustLights != scfgSrc.m_bUseCustLights))
    {
        return kCIDLib::False;
    }

    //
    //  Make sure we have the same number of rooms, loads, etc... If any of these
    //  are different, no need going through the overhead of doing all the
    //  comparisons.
    //
    if ((scfgSrc.m_colHVACList.c4ElemCount() != m_colHVACList.c4ElemCount())
    ||  (scfgSrc.m_colLoadList.c4ElemCount() != m_colLoadList.c4ElemCount())
    ||  (scfgSrc.m_colMediaList.c4ElemCount() != m_colMediaList.c4ElemCount())
    ||  (scfgSrc.m_colResList.c4ElemCount() != m_colResList.c4ElemCount())
    ||  (scfgSrc.m_colRoomList.c4ElemCount() != m_colRoomList.c4ElemCount())
    ||  (scfgSrc.m_colSecList.c4ElemCount() != m_colSecList.c4ElemCount())
    ||  (scfgSrc.m_colWeatherList.c4ElemCount() != m_colWeatherList.c4ElemCount())
    ||  (scfgSrc.m_colXOverList.c4ElemCount() != m_colXOverList.c4ElemCount()))
    {
        return kCIDLib::False;
    }

    //
    //  And now do the actual comparisons
    //
    //  Do the rooms last. Since they reference all of the above lists, we now
    //  know that the same names mean the same things.
    //
    if ((scfgSrc.m_colHVACList != m_colHVACList)
    ||  (scfgSrc.m_colLoadList != m_colLoadList)
    ||  (scfgSrc.m_colMediaList != m_colMediaList)
    ||  (scfgSrc.m_colResList != m_colResList)
    ||  (scfgSrc.m_colRoomList != m_colRoomList)
    ||  (scfgSrc.m_colSecList != m_colSecList)
    ||  (scfgSrc.m_colWeatherList != m_colWeatherList)
    ||  (scfgSrc.m_colXOverList != m_colXOverList)
    ||  (scfgSrc.m_colRoomList != m_colRoomList))
    {
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


tCIDLib::TBoolean TCQCSysCfg::operator!=(const TCQCSysCfg& scfgSrc) const
{
    return !operator==(scfgSrc);
}


// ---------------------------------------------------------------------------
//  TCQCSysCfg: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Add the passed thing to either all rooms or any rooms that already have some,
//  if not already in their list. They shouldn't be, but check.
//
//  Only HVAC, loads, and extra overlays are supported for this. These are the ones
//  that just need a UID of the configured thing (from the Global tab.)
//
tCIDLib::TVoid
TCQCSysCfg::AddToRooms( const   tCQCSysCfg::ECfgLists   eList
                        , const TString&                strUID
                        , const tCIDLib::TBoolean       bToAll)
{
    const tCIDLib::TCard4 c4Count = m_colRoomList.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TCQCSysCfgRmInfo& scriCur = m_colRoomList[c4Index];

        switch(eList)
        {
            case tCQCSysCfg::ECfgLists::Lighting :
            scriCur.AddLoad(strUID, !bToAll);
            break;

        case tCQCSysCfg::ECfgLists::HVAC :
            scriCur.AddHVAC(strUID, !bToAll);
            break;

        case tCQCSysCfg::ECfgLists::XOvers :
            scriCur.AddXOver(strUID, !bToAll);
            break;

            default :
            {
                CIDAssert2(L"AddToList doesn't support this data type");
                break;
            }
        };
    }
}


//
//  Add the indicated HVAC and return its index. If we find it already in the list,
//  then we update it. We return true if we added a new one.
//
tCIDLib::TBoolean
TCQCSysCfg::bAddOrUpdateHVAC(const TSCHVACInfo& scliAdd, tCIDLib::TCard4& c4Index)
{
    c4Index = c4AddOrUpdate(scliAdd, m_colHVACList);
    return (c4Index != kCIDLib::c4MaxCard);
}


//
//  Add the indicated load and return its index. If we find it already in the list,
//  then we update it. We return true if we added a new one.
//
tCIDLib::TBoolean
TCQCSysCfg::bAddOrUpdateLoad(const TSCLoadInfo& scliAdd, tCIDLib::TCard4& c4Index)
{
    c4Index = c4AddOrUpdate(scliAdd, m_colLoadList);
    return (c4Index != kCIDLib::c4MaxCard);
}


//
//  Add the indicated media info and return its index. If we find it already in the
//  list, then we update it. We return true if we added a new one.
//
tCIDLib::TBoolean
TCQCSysCfg::bAddOrUpdateMedia(const TSCMediaInfo& scliToAdd, tCIDLib::TCard4& c4Index)
{
    c4Index = c4AddOrUpdate(scliToAdd, m_colMediaList);
    return (c4Index != kCIDLib::c4MaxCard);
}


//
//  Add the indicated room object and return its index. If we find it already in the
//  list, then we just update it. We return true if we added a new one.
//
tCIDLib::TBoolean
TCQCSysCfg::bAddOrUpdateRoom(const TCQCSysCfgRmInfo& scriNew, tCIDLib::TCard4& c4Index)
{
    // Look it up to see if we have it already
    c4Index = c4RoomLookup(scriNew.strUniqueId(), kCIDLib::True, kCIDLib::False);

    tCIDLib::TBoolean bRet(c4Index == kCIDLib::c4MaxCard);
    if (bRet)
    {
        // It's a new one
        c4Index = m_colRoomList.c4ElemCount();
        m_colRoomList.objAdd(scriNew);
    }
     else
    {
        // We already have it, so update it
        m_colRoomList[c4Index] = scriNew;
    }
    return bRet;
}


//
//  Add the indicated security info object and return its index. If we find it already
//  in the list, then we just update it. We return true if we added a new one.
//
tCIDLib::TBoolean
TCQCSysCfg::bAddOrUpdateSec(const TSCSecInfo& scliToAdd, tCIDLib::TCard4& c4Index)
{
    c4Index = c4AddOrUpdate(scliToAdd, m_colSecList);
    return (c4Index != kCIDLib::c4MaxCard);
}


//
//  Add the indicated weather info object and return its index. If we find it already
//  in the list, then we just update it. We return true if we added a new one.
//
tCIDLib::TBoolean
TCQCSysCfg::bAddOrUpdateWeather(const TSCWeatherInfo& scliToAdd, tCIDLib::TCard4& c4Index)
{
    c4Index = c4AddOrUpdate(scliToAdd, m_colWeatherList);
    return (c4Index != kCIDLib::c4MaxCard);
}


//
//  Add the indicated exra overlay object and return its index. If we find it already
//  in the list, then we just update it. We return true if we added a new one.
//
tCIDLib::TBoolean
TCQCSysCfg::bAddOrUpdateXOver(const TSCXOverInfo& scliToAdd, tCIDLib::TCard4& c4Index)
{
    c4Index = c4AddOrUpdate(scliToAdd, m_colXOverList);
    return (c4Index != kCIDLib::c4MaxCard);
}


//
//  If the resolution isn't in our list already, add it. If already there we don't
//  need to do anything. We return the new or existing index of the resolution.
//
tCIDLib::TBoolean TCQCSysCfg::bAddRes(const TSize& szToAdd, tCIDLib::TCard4& c4Index)
{
    const tCIDLib::TCard4 c4Count = m_colResList.c4ElemCount();
    for (c4Index = 0; c4Index < c4Count; c4Index++)
    {
        // If already there, so return false
        if (m_colResList[c4Index] == szToAdd)
            return kCIDLib::False;

    }

    //
    //  Not already there, so add it and return true. The index will already be at
    //  the end of the list.
    //
    m_colResList.objAdd(szToAdd);
    return kCIDLib::True;
}


// Look up a room by name and return the index if found, else max card.
tCIDLib::TCard4 TCQCSysCfg::c4FindRoomByName(const TString& strName) const
{
    return c4RoomLookup(strName, kCIDLib::False, kCIDLib::True);
}


//
//  Look up a room by UID and return the index. We can either throw if not
//  found or return a boolean find result.
//
tCIDLib::TBoolean
TCQCSysCfg::bFindRoomByUID( const   TString&            strUID
                            ,       tCIDLib::TCard4&    c4RoomInd
                            , const tCIDLib::TBoolean   bThrowIfNot) const
{
    c4RoomInd = c4RoomLookup(strUID, kCIDLib::True, bThrowIfNot);
    return (c4RoomInd != kCIDLib::c4MaxCard);
}


//
//  See if the passed name is a dup of any existing room name. The UIDs we know
//  are unique, but we also want to warn if they use a display name twice, to keep
//  things from getting confused.
//
tCIDLib::TBoolean TCQCSysCfg::bIsUniqueName(const TString& strToCheck) const
{
    tCIDLib::TCard4 c4Count = m_colRoomList.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (m_colRoomList[c4Index].strName().bCompareI(strToCheck))
            return kCIDLib::False;
    }

    // It's unique
    return kCIDLib::True;
}


//
//  Returns true if the passed alt name already is in use in the indicated list.
//
tCIDLib::TBoolean
TCQCSysCfg::bListAltNameExists( const   tCQCSysCfg::ECfgLists   eList
                                , const TString&                strName) const
{
    const tCQCSysCfg::TItemList* pcolSrc = nullptr;
    switch(eList)
    {
        case tCQCSysCfg::ECfgLists::HVAC :
            pcolSrc = &m_colHVACList;
            break;

        case tCQCSysCfg::ECfgLists::Lighting :
            pcolSrc = &m_colLoadList;
            break;

        case tCQCSysCfg::ECfgLists::Media :
            pcolSrc = &m_colMediaList;
            break;

        case tCQCSysCfg::ECfgLists::Security :
            pcolSrc = &m_colSecList;
            break;

        case tCQCSysCfg::ECfgLists::Weather :
            pcolSrc = &m_colWeatherList;
            break;

        case tCQCSysCfg::ECfgLists::XOvers :
            pcolSrc = &m_colXOverList;
            break;

        default :
            break;
    };

    if (pcolSrc)
    {
        const tCIDLib::TCard4 c4Count = pcolSrc->c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const TSysCfgListItem& scliCur = *pcolSrc->pobjAt(c4Index);
            if (scliCur.m_strAltName.bCompareI(strName))
                return kCIDLib::True;
        }
    }

    // Never found it
    return kCIDLib::False;
}


//
//  Returns true if the passed id exists within the indicated list.
//
tCIDLib::TBoolean
TCQCSysCfg::bListIDExists(  const   tCQCSysCfg::ECfgLists   eList
                            , const TString&                strUID) const
{
    const tCQCSysCfg::TItemList* pcolSrc = nullptr;
    switch(eList)
    {
        case tCQCSysCfg::ECfgLists::HVAC :
            pcolSrc = &m_colHVACList;
            break;

        case tCQCSysCfg::ECfgLists::Lighting :
            pcolSrc = &m_colLoadList;
            break;

        case tCQCSysCfg::ECfgLists::Media :
            pcolSrc = &m_colMediaList;
            break;

        case tCQCSysCfg::ECfgLists::Security :
            pcolSrc = &m_colSecList;
            break;

        case tCQCSysCfg::ECfgLists::Weather :
            pcolSrc = &m_colWeatherList;
            break;

        case tCQCSysCfg::ECfgLists::XOvers :
            pcolSrc = &m_colXOverList;
            break;

        default :
            break;
    };

    if (pcolSrc)
    {
        const tCIDLib::TCard4 c4Count = pcolSrc->c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const TSysCfgListItem& scliCur = *pcolSrc->pobjAt(c4Index);
            if (scliCur.m_strUniqueId.bCompareI(strUID))
                return kCIDLib::True;
        }
    }

    // Never found it
    return kCIDLib::False;
}


//
//  Returns true if the passed title exists within the indicated list.
//
tCIDLib::TBoolean
TCQCSysCfg::bListTitleExists(const  tCQCSysCfg::ECfgLists   eList
                            , const TString&                strTitle) const
{
    const tCQCSysCfg::TItemList* pcolSrc = nullptr;
    switch(eList)
    {
        case tCQCSysCfg::ECfgLists::HVAC :
            pcolSrc = &m_colHVACList;
            break;

        case tCQCSysCfg::ECfgLists::Lighting :
            pcolSrc = &m_colLoadList;
            break;

        case tCQCSysCfg::ECfgLists::Media :
            pcolSrc = &m_colMediaList;
            break;

        case tCQCSysCfg::ECfgLists::Security :
            pcolSrc = &m_colSecList;
            break;

        case tCQCSysCfg::ECfgLists::Weather :
            pcolSrc = &m_colWeatherList;
            break;

        case tCQCSysCfg::ECfgLists::XOvers :
            pcolSrc = &m_colXOverList;
            break;

        default :
            break;
    };

    if (pcolSrc)
    {
        const tCIDLib::TCard4 c4Count = pcolSrc->c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const TSysCfgListItem& scliCur = *pcolSrc->pobjAt(c4Index);
            if (scliCur.m_strTitle.bCompareI(strTitle))
                return kCIDLib::True;
        }
    }

    // Never found it
    return kCIDLib::False;
}


//
//  A convenience to check whether the id for a given room function exists.
//
tCIDLib::TBoolean
TCQCSysCfg::bRoomIDExists(  const   tCQCSysCfg::ERmFuncs    eRmFunc
                            , const TCQCSysCfgRmInfo&       scriSrc) const
{
    // Get the UID for this room function. If empty, return false
    const TString& strUID = scriSrc.strFuncId(eRmFunc);
    if (strUID.bIsEmpty())
        return kCIDLib::False;

    // Figure out the list that corresponds to the passed fun function
    const tCQCSysCfg::TItemList* pcolSrc = nullptr;
    switch(eRmFunc)
    {
        case tCQCSysCfg::ERmFuncs::HVAC :
            pcolSrc = &m_colHVACList;
            break;

        case tCQCSysCfg::ERmFuncs::Lighting :
            pcolSrc = &m_colLoadList;
            break;

        case tCQCSysCfg::ERmFuncs::Movies :
        case tCQCSysCfg::ERmFuncs::Music :
            pcolSrc = &m_colMediaList;
            break;

        case tCQCSysCfg::ERmFuncs::Security :
            pcolSrc = &m_colSecList;
            break;

        case tCQCSysCfg::ERmFuncs::Weather :
            pcolSrc = &m_colWeatherList;
            break;

        default :
            break;
    };

    if (pcolSrc)
    {
        const tCIDLib::TCard4 c4Count = pcolSrc->c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const TSysCfgListItem& scliCur = *pcolSrc->pobjAt(c4Index);
            if (scliCur.m_strUniqueId.bCompareI(strUID))
                return kCIDLib::True;
        }
    }

    // Never found it
    return kCIDLib::False;
}



//
//  See if the passed resolution is already in the list or not.
//
tCIDLib::TBoolean TCQCSysCfg::bResExists(const TSize& szToCheck) const
{
    const tCIDLib::TCard4 c4Count = m_colResList.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        // If already there, then return true
        if (m_colResList[c4Index] == szToCheck)
            return kCIDLib::True;
    }
    return kCIDLib::False;
}



//
//  Get or set the use custom layout for lights option. This will also be forced off
//  if the custom layout template option is emptied out.
//
tCIDLib::TBoolean TCQCSysCfg::bUseCustLights() const
{
    return m_bUseCustLights;
}

tCIDLib::TBoolean TCQCSysCfg::bUseCustLights(const tCIDLib::TBoolean bToSet)
{
    m_bUseCustLights = bToSet;
    return m_bUseCustLights;
}



//
//  Add a new room, in this case just one with default values, with just the name
//  set. So we just make sure the name is unique. If so, we create a new room object
//  and add it.
//
tCIDLib::TCard4 TCQCSysCfg::c4AddRoom(const TString& strName)
{
    // Make sure it's not a dup
    if (!bIsUniqueName(strName))
    {
        facCQCSysCfg().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCQCSCfgErrs::errcSysCfg_UniqueName
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Duplicate
            , strName
        );
    }

    //
    //  Looks ok so add it, The return index is the current count before we
    //  add the new one.
    //
    const tCIDLib::TCard4 c4Ret = m_colRoomList.c4ElemCount();
    m_colRoomList.objPlace(strName);
    return c4Ret;
}



// Return the size of various lists we keep
tCIDLib::TCard4 TCQCSysCfg::c4ListIdCount(const tCQCSysCfg::ECfgLists eList) const
{
    const tCQCSysCfg::TItemList* pcolSrc = nullptr;
    switch(eList)
    {
        case tCQCSysCfg::ECfgLists::HVAC :
            pcolSrc = &m_colHVACList;
            break;

        case tCQCSysCfg::ECfgLists::Lighting :
            pcolSrc = &m_colLoadList;
            break;

        case tCQCSysCfg::ECfgLists::Media :
            pcolSrc = &m_colMediaList;
            break;

        case tCQCSysCfg::ECfgLists::Security :
            pcolSrc = &m_colSecList;
            break;

        case tCQCSysCfg::ECfgLists::Weather :
            pcolSrc = &m_colWeatherList;
            break;

        case tCQCSysCfg::ECfgLists::XOvers :
            pcolSrc = &m_colXOverList;
            break;

        default :
            break;
    };


    if (!pcolSrc)
        return 0;
    return pcolSrc->c4ElemCount();
}


tCIDLib::TCard4 TCQCSysCfg::c4ResCount() const
{
    return m_colResList.c4ElemCount();
}

tCIDLib::TCard4 TCQCSysCfg::c4RoomCount() const
{
    return m_colRoomList.c4ElemCount();
}


//
//  Returns a key/value pair list for all of the configured objects for the
//  indicated list. The key is the display name and the value is the unique
//  id.
//
tCIDLib::TCard4
TCQCSysCfg::c4QueryCfgList( const   tCQCSysCfg::ECfgLists   eList
                            ,       tCIDLib::TKVPCollect&   colToFill)
{
    colToFill.RemoveAll();

    tCQCSysCfg::TItemList* pcolSrc = nullptr;
    switch(eList)
    {
        case tCQCSysCfg::ECfgLists::HVAC :
            pcolSrc = &m_colHVACList;
            break;

        case tCQCSysCfg::ECfgLists::Lighting :
            pcolSrc = &m_colLoadList;
            break;

        case tCQCSysCfg::ECfgLists::Media :
            pcolSrc = &m_colMediaList;
            break;

        case tCQCSysCfg::ECfgLists::Security :
            pcolSrc = &m_colSecList;
            break;

        case tCQCSysCfg::ECfgLists::Weather :
            pcolSrc = &m_colWeatherList;
            break;

        case tCQCSysCfg::ECfgLists::XOvers :
            pcolSrc = &m_colXOverList;
            break;

        default :
            break;
    };


    if (pcolSrc)
    {
        const tCIDLib::TCard4 c4Count = pcolSrc->c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const TSysCfgListItem& scliCur = *pcolSrc->pobjAt(c4Index);
            colToFill.objAdd
            (
                TKeyValuePair(scliCur.m_strTitle, scliCur.m_strUniqueId)
            );
        }
    }
    return colToFill.c4ElemCount();
}


//
//  Delete items from our various lists a by UID. We also remove any references to
//  them from the configured rooms.
//
tCIDLib::TVoid
TCQCSysCfg::DeleteListId(const  tCQCSysCfg::ECfgLists   eList
                        , const TString&                strUID)
{
    tCQCSysCfg::TItemList* pcolSrc = nullptr;
    switch(eList)
    {
        case tCQCSysCfg::ECfgLists::HVAC :
            pcolSrc = &m_colHVACList;
            break;

        case tCQCSysCfg::ECfgLists::Lighting :
            pcolSrc = &m_colLoadList;
            break;

        case tCQCSysCfg::ECfgLists::Media :
            pcolSrc = &m_colMediaList;
            break;

        case tCQCSysCfg::ECfgLists::Security :
            pcolSrc = &m_colSecList;
            break;

        case tCQCSysCfg::ECfgLists::Weather :
            pcolSrc = &m_colWeatherList;
            break;

        case tCQCSysCfg::ECfgLists::XOvers :
            pcolSrc = &m_colXOverList;
            break;

        default :
            break;
    };

    CIDAssert(pcolSrc != nullptr, L"There is no list for this syscfg list enum");

    // Delete the item from our list
    DelListItem(strUID, *pcolSrc);

    // Remove all references from configured rooms
    RemoveListRefs(strUID, eList);
}


tCIDLib::TVoid TCQCSysCfg::DeleteRes(const TSize& szToDel)
{
    const tCIDLib::TCard4 c4Count = m_colResList.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (m_colResList[c4Index] == szToDel)
        {
            // This is our guy, remove and break out
            m_colResList.RemoveAt(c4Index);
            break;
        }
    }
}


tCIDLib::TVoid TCQCSysCfg::DeleteRoom(const TString& strUID)
{
    // Call the other lookup version to find it. If found, delete it
    tCIDLib::TCard4 c4Index;
    if (bFindRoomByUID(strUID, c4Index, kCIDLib::False))
        m_colRoomList.RemoveAt(c4Index);
}


//
// Proivde access to our various lists by index or UID
//
const TSCHVACInfo& TCQCSysCfg::scliHVACAt(const tCIDLib::TCard4 c4At)
{
    return static_cast<const TSCHVACInfo&>(*m_colHVACList[c4At]);
}

const TSCHVACInfo&
TCQCSysCfg::scliHVACByUID(  const   TString&            strUID
                            ,       tCIDLib::TCard4&    c4Index) const
{
    c4Index = c4ListLookup(strUID, m_colHVACList, kCIDLib::True, L"HVAC");
    return static_cast<const TSCHVACInfo&>(*m_colHVACList[c4Index]);
}


const TSCLoadInfo& TCQCSysCfg::scliLoadAt(const tCIDLib::TCard4 c4At) const
{
    return static_cast<const TSCLoadInfo&>(*m_colLoadList[c4At]);
}

const TSCLoadInfo&
TCQCSysCfg::scliLoadByUID(  const   TString&            strUID
                            ,       tCIDLib::TCard4&    c4Index) const
{
    c4Index = c4ListLookup(strUID, m_colLoadList, kCIDLib::True, L"load");
    return static_cast<const TSCLoadInfo&>(*m_colLoadList[c4Index]);
}


const TSCMediaInfo& TCQCSysCfg::scliMediaAt(const tCIDLib::TCard4 c4At)
{
    return static_cast<const TSCMediaInfo&>(*m_colMediaList[c4At]);
}

const TSCMediaInfo&
TCQCSysCfg::scliMediaByUID( const   TString&            strUID
                            ,       tCIDLib::TCard4&    c4Index) const
{
    c4Index = c4ListLookup(strUID, m_colMediaList, kCIDLib::True, L"media");
    return static_cast<const TSCMediaInfo&>(*m_colMediaList[c4Index]);
}


const TCQCSysCfgRmInfo& TCQCSysCfg::scriRoomAt(const tCIDLib::TCard4 c4Index) const
{
    return m_colRoomList[c4Index];
}

TCQCSysCfgRmInfo& TCQCSysCfg::scriRoomAt(const tCIDLib::TCard4 c4Index)
{
    return m_colRoomList[c4Index];
}


const TCQCSysCfgRmInfo&
TCQCSysCfg::scriRoomByName( const   TString&            strName
                            ,       tCIDLib::TCard4&    c4Index) const
{
    // Call the lookup helper by name, tell it to throw if not found
    c4Index = c4RoomLookup(strName, kCIDLib::False, kCIDLib::True);
    return m_colRoomList[c4Index];
}

TCQCSysCfgRmInfo&
TCQCSysCfg::scriRoomByName( const   TString&            strName
                            ,       tCIDLib::TCard4&    c4Index)
{
    // Call the lookup helper, tell it to throw if not found
    c4Index = c4RoomLookup(strName, kCIDLib::False, kCIDLib::True);
    return m_colRoomList[c4Index];
}


const TCQCSysCfgRmInfo&
TCQCSysCfg::scriRoomByUID(  const   TString&            strUID
                            ,       tCIDLib::TCard4&    c4Index) const
{
    // Call the lookup helper by UID, tell it to throw if not found
    c4Index = c4RoomLookup(strUID, kCIDLib::True, kCIDLib::True);
    return m_colRoomList[c4Index];
}

TCQCSysCfgRmInfo&
TCQCSysCfg::scriRoomByUID(  const   TString&            strUID
                            ,       tCIDLib::TCard4&    c4Index)
{
    // Call the lookup helper, tell it to throw if not found
    c4Index = c4RoomLookup(strUID, kCIDLib::True, kCIDLib::True);
    return m_colRoomList[c4Index];
}


const TSCSecInfo& TCQCSysCfg::scliSecAt(const tCIDLib::TCard4 c4At)
{
    return static_cast<const TSCSecInfo&>(*m_colSecList[c4At]);
}

const TSCSecInfo&
TCQCSysCfg::scliSecByUID(const  TString&            strUID
                        ,       tCIDLib::TCard4&    c4Index) const
{
    c4Index = c4ListLookup(strUID, m_colSecList, kCIDLib::True, L"security");
    return static_cast<const TSCSecInfo&>(*m_colSecList[c4Index]);
}


const TSCWeatherInfo& TCQCSysCfg::scliWeatherAt(const tCIDLib::TCard4 c4At)
{
    return static_cast<const TSCWeatherInfo&>(*m_colWeatherList[c4At]);
}

const TSCWeatherInfo&
TCQCSysCfg::scliWeatherByUID(const  TString&            strUID
                            ,       tCIDLib::TCard4&    c4Index) const
{
    c4Index = c4ListLookup(strUID, m_colWeatherList, kCIDLib::True, L"weather");
    return static_cast<const TSCWeatherInfo&>(*m_colWeatherList[c4Index]);
}


const TSCXOverInfo& TCQCSysCfg::scliXOverAt(const tCIDLib::TCard4 c4At)
{
    return static_cast<const TSCXOverInfo&>(*m_colXOverList[c4At]);
}

const TSCXOverInfo&
TCQCSysCfg::scliXOverByUID( const   TString&            strUID
                            ,       tCIDLib::TCard4&    c4Index) const
{
    c4Index = c4ListLookup(strUID, m_colXOverList, kCIDLib::True, L"extra overs");
    return static_cast<const TSCXOverInfo&>(*m_colXOverList[c4Index]);
}


// Get/set the custom layout template
const TString& TCQCSysCfg::strCustLayout() const
{
    return m_strCustLayout;
}

const TString& TCQCSysCfg::strCustLayout(const TString& strToSet)
{
    m_strCustLayout = strToSet;

    // If it was emptyed, then force off the use cust lights option
    if (m_strCustLayout.bIsEmpty())
        m_bUseCustLights = kCIDLib::False;

    return m_strCustLayout;
}


//
//  Return the title for a particular function of the passed room. We just get the
//  unique id of the referenced function (from the room), then look it up in the
//  respective list, and return the configured display title.
//
const TString&
TCQCSysCfg::strRoomFuncTitle(const  tCQCSysCfg::ERmFuncs    eFunc
                            , const TCQCSysCfgRmInfo&       scriSrc) const
{
    const TString& strUID = scriSrc.strFuncId(eFunc);

    // If not configured for this room, then return an empty string
    if (strUID.bIsEmpty())
        return TString::strEmpty();

    // Map to the correct list for the room function
    const tCQCSysCfg::TItemList* pcolSrc = nullptr;
    switch(eFunc)
    {
        case tCQCSysCfg::ERmFuncs::HVAC :
            pcolSrc = &m_colHVACList;
            break;

        case tCQCSysCfg::ERmFuncs::Lighting :
            pcolSrc = &m_colLoadList;
            break;

        case tCQCSysCfg::ERmFuncs::Movies :
        case tCQCSysCfg::ERmFuncs::Music :
            pcolSrc = &m_colMediaList;
            break;

        case tCQCSysCfg::ERmFuncs::Security :
            pcolSrc = &m_colSecList;
            break;

        case tCQCSysCfg::ERmFuncs::Weather :
            pcolSrc = &m_colWeatherList;
            break;

        default :
            break;
    };

    if (pcolSrc)
    {
        const tCIDLib::TCard4 c4Count = pcolSrc->c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const TSysCfgListItem& scliCur = *pcolSrc->pobjAt(c4Index);
            if (scliCur.m_strUniqueId.bCompareI(strUID))
                return scliCur.m_strTitle;
        }
    }

    // Never found it
    return TString::strEmpty();
}


// Get/set the target scope name
const TString& TCQCSysCfg::strTarScope() const
{
    return m_strTarScope;
}

const TString& TCQCSysCfg::strTarScope(const TString& strToSet)
{
    m_strTarScope = strToSet;

    // Make sure it doesn't end with a slash
    if (!m_strTarScope.bIsEmpty() && (m_strTarScope.chLast() == kCIDLib::chForwardSlash))
        m_strTarScope.DeleteLast();

    return m_strTarScope;
}


// Return the output resolution at the indicated index
const TSize& TCQCSysCfg::szResAt(const tCIDLib::TCard4 c4At) const
{
    return m_colResList[c4At];
}


// Replace a resolution in the list with a new one
tCIDLib::TVoid
TCQCSysCfg::ReplaceRes(const TSize& szOld, const TSize& szNew)
{
    const tCIDLib::TCard4 c4Count = m_colResList.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (m_colResList[c4Index] == szOld)
        {
            m_colResList[c4Index] = szNew;
            break;
        }
    }
}


//
//  We just sort each of the lists based on the main title.
//
tCIDLib::TVoid TCQCSysCfg::SortLists()
{
    m_colHVACList.Sort(&TSysCfgListItem::eCompByTitle);
    m_colLoadList.Sort(&TSysCfgListItem::eCompByTitle);
    m_colMediaList.Sort(&TSysCfgListItem::eCompByTitle);
    m_colSecList.Sort(&TSysCfgListItem::eCompByTitle);
    m_colWeatherList.Sort(&TSysCfgListItem::eCompByTitle);
}


//
//  Format our data out to JSON. The caller tells us if we wants us to include the
//  room data or just the global data. Often he just wants the global data and will
//  be interested later in just a specific room at a time.
//
tCIDLib::TVoid
TCQCSysCfg::ToJSON(         TTextOutStream&     strmTar
                    , const tCIDLib::TBoolean   bInclRooms
                    , const tCIDLib::TBoolean   bAppend)
{
    const tCIDLib::TCard4 c4RmCnt = m_colRoomList.c4ElemCount();

    // If not appending, then clear the stream first
    if (!bAppend)
        strmTar.Reset();

    // Put out the main global data object header
    TJSONParser::OutputObjHdr(L"Global", strmTar);
    tCIDLib::TCard4 c4Count;

    // Do the various global mapping lists
    TJSONParser::OutputObjHdr(L"HVAC", strmTar);
    strmTar << L"},\n";

    TJSONParser::OutputObjHdr(L"Loads", strmTar);
    strmTar << L"},\n";

    TJSONParser::OutputObjHdr(L"Media", strmTar);
    strmTar << L"},\n";

    TJSONParser::OutputObjHdr(L"Sec", strmTar);
    strmTar << L"},\n";

    TJSONParser::OutputObjHdr(L"Weather", strmTar);
    strmTar << L"},\n";

    // At the end output the room count. Indicate it's the last so no comma is appended
    TJSONParser::OutputCard(L"RmCnt", c4RmCnt, strmTar, kCIDLib::True);
    c4Count = m_colWeatherList.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TSysCfgListItem& scliCur = *m_colWeatherList[c4Index];
        if (c4Index)
            strmTar << L", ";

        strmTar << L"\"" << scliCur.m_strUniqueId << L"\" : \""
                << scliCur.m_strTitle << "\"";
    }
    strmTar << L"}";

    // If they want rooms, do that, putting out a comma to end the global stuff above
    if (bInclRooms)
    {
        strmTar << kCIDLib::chComma << kCIDLib::NewLn
                << L"\"Rooms\" : [\n";

        strmTar << L"]\n";
    }

    // Put out a final new line/flush
    strmTar << kCIDLib::EndLn;
}



//
//  Format our data out to XML. The caller tells us what encoding he is ultimately going
//  to encode it to for transmission, so we can put that into the XML decl.
//
tCIDLib::TVoid
TCQCSysCfg::ToXML(          TTextOutStream&     strmTar
                    , const TString&            strEncoding
                    , const tCIDLib::TBoolean   bAppend) const
{
    // If not appending, then reset the stream first
    if (!bAppend)
        strmTar.Reset();

    // Output up to the end of the root element
    strmTar << L"<?xml version=\"1.0\" encoding=\""
            << strEncoding << L"\"?>\n"
            << L"<CQCSysCfg:Root CQCSysCfg:RmCnt=\"" << m_colRoomList.c4ElemCount()
            << L"\">\n";

    // Output the global mapping lists

    // Do all of the rooms
    const tCIDLib::TCard4 c4Count = m_colRoomList.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        // Get the current room. If disable, skip it
        const TCQCSysCfgRmInfo& scriCur = m_colRoomList[c4Index];
        if (!scriCur.bEnabled())
            continue;

        if (c4Index)
            strmTar << L"\n";

        // Format this one out, passing ourself along to him for reference lookups
        scriCur.ToXML(*this, strmTar, kCIDLib::True);
    }

    // Now close off the root
    strmTar << L"</CQCSysCfg:Root>\n\n";
}



// ---------------------------------------------------------------------------
//  TCQCSysCfg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCSysCfg::StreamFrom(TBinInStream& strmToReadFrom)
{
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCKit_Layout::c2FmtSysCfg))
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

    // Do the simple stuff first
    strmToReadFrom  >> m_strTarScope
                    >> m_colResList
                    >> m_colRoomList;

    //
    //  Do the polymorhic lists. Sort them first so that they are sorted for
    //  display.
    //
    StreamInList(m_colLoadList, strmToReadFrom);
    StreamInList(m_colMediaList, strmToReadFrom);
    StreamInList(m_colSecList, strmToReadFrom);
    StreamInList(m_colWeatherList, strmToReadFrom);
    StreamInList(m_colXOverList, strmToReadFrom);
    SortLists();

    // If V2 or beyond read in the HVAC list, else default it
    if (c2FmtVersion > 1)
        StreamInList(m_colHVACList, strmToReadFrom);
    else
        m_colHVACList.RemoveAll();

    // If V3 or beyond read in the custom layout template, else default
    if (c2FmtVersion > 2)
        strmToReadFrom  >> m_strCustLayout;
    else
        m_strCustLayout.Clear();

    // If V4 or beyond read in the new custom layout related flags, esle default
    if (c2FmtVersion > 3)
        strmToReadFrom >> m_bUseCustLights;
    else
        m_bUseCustLights = kCIDLib::False;

    // If less than V4, then update any paths for 5.x
    if (c2FmtVersion < 4)
        facCQCKit().Make50Path(m_strTarScope);

    // If less than 6, default the serial number, else read it in
    if (c2FmtVersion < 6)
        m_c4SerialNum = 1;
    else
        strmToReadFrom >> m_c4SerialNum;

    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TCQCSysCfg::StreamTo(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCKit_Layout::c2FmtSysCfg

                    << m_strTarScope
                    << m_colResList
                    << m_colRoomList;

    // Do the polymorhic lists.
    StreamOutList(m_colLoadList, strmToWriteTo);
    StreamOutList(m_colMediaList, strmToWriteTo);
    StreamOutList(m_colSecList, strmToWriteTo);
    StreamOutList(m_colWeatherList, strmToWriteTo);
    StreamOutList(m_colXOverList, strmToWriteTo);

    // V2 stuff
    StreamOutList(m_colHVACList, strmToWriteTo);

                    // V3 stuff
    strmToWriteTo   << m_strCustLayout

                    // V4 stuff
                    << m_bUseCustLights

                    // V6 stuff
                    << m_c4SerialNum

                    << tCIDLib::EStreamMarkers::EndObject;
}


// ---------------------------------------------------------------------------
//  TCQCSysCfg: Protected, non-virtual methods
// ---------------------------------------------------------------------------

// Get or set the serial number
tCIDLib::TCard4 TCQCSysCfg::c4SerialNum() const
{
    return m_c4SerialNum;
}

tCIDLib::TCard4 TCQCSysCfg::c4SerialNum(const tCIDLib::TCard4 c4ToSet)
{
    m_c4SerialNum = c4ToSet;
    return m_c4SerialNum;
}


// ---------------------------------------------------------------------------
//  TCQCSysCfg: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Generically update or add an item to one of our polymorphic lists.
//
tCIDLib::TCard4
TCQCSysCfg::c4AddOrUpdate(  const   TSysCfgListItem&        scliSrc
                            ,       tCQCSysCfg::TItemList&  colTar)
{
    // See if it already exists or not
    tCIDLib::TCard4 c4Index = c4ListLookup
    (
        scliSrc.m_strUniqueId, colTar, kCIDLib::False, 0
    );

    tCIDLib::TBoolean bNew(c4Index == kCIDLib::c4MaxCard);
    if (bNew)
    {
        // It doesn't so add it, return the current count as the new index
        c4Index = colTar.c4ElemCount();
        colTar.Add(::pDupObject<TSysCfgListItem>(scliSrc));
    }
     else
    {
        //
        //  We already have it so just update it.
        //
        //  NOTE we use a polymophic copy method here, not assign, else we would
        //  slice since we are dealing with them via the base class here.
        //
        colTar[c4Index]->CopyFrom(scliSrc);
    }
    return c4Index;
}


// Look up a unique id in one of the lists and return its index
tCIDLib::TCard4
TCQCSysCfg::c4ListLookup(const  TString&                strUID
                        , const tCQCSysCfg::TItemList&  colSrc
                        , const tCIDLib::TBoolean       bThrowIfNot
                        , const tCIDLib::TCh* const     pszType) const
{
    const tCIDLib::TCard4 c4Count = colSrc.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (colSrc[c4Index]->m_strUniqueId == strUID)
            return c4Index;
    }

    if (bThrowIfNot)
    {
        facCQCSysCfg().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCQCSCfgErrs::errcSysCfg_NotFnd
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , TString(pszType)
            , TString(L"UID")
            , strUID
        );
    }
    return kCIDLib::c4MaxCard;
}


//
//  A generic room lookup. It looks up by either UID or name.
//
tCIDLib::TCard4
TCQCSysCfg::c4RoomLookup(const  TString&            strToFind
                        , const tCIDLib::TBoolean   bByUID
                        , const tCIDLib::TBoolean   bThrowIfNot) const
{
    const tCIDLib::TCard4 c4Count = m_colRoomList.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TCQCSysCfgRmInfo& scriCur = m_colRoomList[c4Index];
        if (bByUID)
        {
            if (scriCur.strUniqueId() == strToFind)
                return c4Index;
        }
         else
        {
            if (scriCur.strName().bCompareI(strToFind))
                return c4Index;
        }
    }

    // Never found it
    if (bThrowIfNot)
    {
        facCQCSysCfg().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCQCSCfgErrs::errcSysCfg_NotFnd
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , TString(L"room")
            , bByUID ? TString(L"UID") : TString(L"name")
            , strToFind
        );
    }
    return kCIDLib::c4MaxCard;
}


//
//  Since our lists are mostly polymorphic, we have to copy them one item at atime,
//  but we can use this single helper, because all the list classes support
//  polymorphic copying.
//
tCIDLib::TVoid
TCQCSysCfg::CopyList(const  tCQCSysCfg::TItemList&  colSrc
                    ,       tCQCSysCfg::TItemList&  colTar)
{
    colTar.RemoveAll();

    const tCIDLib::TCard4 c4Count = colSrc.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        colTar.Add(::pDupObject<TSysCfgListItem>(colSrc[c4Index]));
}


tCIDLib::TVoid
TCQCSysCfg::DelListItem(const TString& strUID, tCQCSysCfg::TItemList& colSrc)
{
    const tCIDLib::TCard4 c4Count = colSrc.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (colSrc[c4Index]->m_strUniqueId == strUID)
        {
            colSrc.RemoveAt(c4Index);
            break;
        }
    }
}


// Stream in our out one of our polymorphic lists
tCIDLib::TVoid
TCQCSysCfg::StreamInList(tCQCSysCfg::TItemList& colTar, TBinInStream& strmSrc) const
{
    colTar.RemoveAll();

    // The whole list is bracketed with start/end markers
    strmSrc.CheckForStartMarker(CID_FILE, CID_LINE);

    // Stream in the count and validate it
    tCIDLib::TCard4 c4Cnt, c4XORCnt;
    strmSrc >> c4Cnt >> c4XORCnt;
    if ((c4Cnt ^ kCIDLib::c4MaxCard) != c4XORCnt)
    {
    }

    TPolyStreamer<TSysCfgListItem> pstrmRead(&strmSrc, 0);
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Cnt; c4Index++)
    {
        // Each should start with a frame marker
        strmSrc.CheckForFrameMarker(CID_FILE, CID_LINE);

        TSysCfgListItem* pscliCur = pstrmRead.pobjStreamIn();
        colTar.Add(pscliCur);
    }

    strmSrc.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid
TCQCSysCfg::StreamOutList(const tCQCSysCfg::TItemList& colSrc, TBinOutStream& strmTar) const
{
    // Bracket the whole list with start/end markers
    strmTar << tCIDLib::EStreamMarkers::StartObject;

    // Output the count and XOR'd count
    const tCIDLib::TCard4 c4Count = colSrc.c4ElemCount();
    strmTar << c4Count
            << tCIDLib::TCard4(c4Count ^ kCIDLib::c4MaxCard);

    // And polymorphically stream them out
    TPolyStreamer<TSysCfgListItem> pstrmWrite(0, &strmTar);
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        strmTar << tCIDLib::EStreamMarkers::Frame;
        pstrmWrite.StreamOut(*colSrc[c4Index]);
    }

    strmTar << tCIDLib::EStreamMarkers::EndObject;
}


//
//  When an entry is removed from one of the configured function lists, we have to
//  remove any references to them in rooms and as defaults in rooms.
//
tCIDLib::TVoid
TCQCSysCfg::RemoveListRefs(const TString& strUID, const tCQCSysCfg::ECfgLists eList)
{
    const tCIDLib::TCard4 c4Count = m_colRoomList.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TCQCSysCfgRmInfo& scriCur = m_colRoomList[c4Index];
        scriCur.RemoveFromList(eList, strUID);
    }
}

