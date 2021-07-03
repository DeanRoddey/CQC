//
// FILE NAME: CQCSysCfg_Room.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/03/2014
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
//  This file implements the class that contains the configuration for a room.
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
RTTIDecls(TCQCSysCfgRmInfo,TObject)


// Local types, constants, etc...
namespace
{
    namespace CQCKit_Room
    {
        // -----------------------------------------------------------------------
        //  The persistent format number for the room configuration class. We started
        //  this over at 1 after the conversion from the previous format.
        //
        //  Version 2 -
        //      We added a new room function (HVAC), so we need to expand the function id
        //      object array when we read it in, and we added the new selected HVACs
        //      list.
        //
        //  Version 3 -
        //      Security zones were changed from a string list to a key/value pair list, so
        //      that we can provide for an 'alt-name' for voice control.
        //
        //  Version 4 -
        //      We added a set of 'room mode' global actions.
        //
        //  Version 5 -
        //      Add voice control configuration
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2 c2FmtRoom  = 5;
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCSysCfgRmInfo
//  PREFIX: scri
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCSysCfgRmInfo: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCSysCfgRmInfo::TCQCSysCfgRmInfo() :

    m_bAllowRmSel(kCIDLib::True)
    , m_bEnabled(kCIDLib::False)
    , m_objaFuncIds(tCQCSysCfg::ERmFuncs::Count)
    , m_objaGlobalActs(tCQCSysCfg::EGlobActs::Count)
    , m_objaRoomVars(kCQCSysCfg::c4MaxRoomVars)
{
}

TCQCSysCfgRmInfo::TCQCSysCfgRmInfo(const TString& strName) :

    m_bAllowRmSel(kCIDLib::True)
    , m_bEnabled(kCIDLib::True)
    , m_objaFuncIds(tCQCSysCfg::ERmFuncs::Count)
    , m_objaGlobalActs(tCQCSysCfg::EGlobActs::Count)
    , m_objaRoomVars(kCQCSysCfg::c4MaxRoomVars)
    , m_strName(strName)
    , m_strUniqueId(TUniqueId::strMakeId())
{
}

TCQCSysCfgRmInfo::~TCQCSysCfgRmInfo()
{
}


// ---------------------------------------------------------------------------
//  TCQCSysCfgRmInfo: Public operators
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TCQCSysCfgRmInfo::operator==(const TCQCSysCfgRmInfo& scriSrc) const
{
    return
    (
        (m_bAllowRmSel == scriSrc.m_bAllowRmSel)
        && (m_bEnabled == scriSrc.m_bEnabled)
        && (m_strName == scriSrc.m_strName)
        && (m_strUniqueId == scriSrc.m_strUniqueId)
        && (m_colRoomModes == scriSrc.m_colRoomModes)
        && (m_colSecZones == scriSrc.m_colSecZones)
        && (m_colSelHVACs == scriSrc.m_colSelHVACs)
        && (m_colSelLoads == scriSrc.m_colSelLoads)
        && (m_colXOvers == scriSrc.m_colXOvers)

        && (m_objaFuncIds == scriSrc.m_objaFuncIds)
        && (m_objaRoomVars == scriSrc.m_objaRoomVars)
        && (m_objaGlobalActs == scriSrc.m_objaGlobalActs)

        && (m_scvcVoice == scriSrc.m_scvcVoice)
    );
}


tCIDLib::TBoolean
TCQCSysCfgRmInfo::operator!=(const TCQCSysCfgRmInfo& scriSrc) const
{
    return !operator==(scriSrc);
}



// ---------------------------------------------------------------------------
//  TCQCSysCfgRmInfo: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Helpers to add various configured bits to this room. We get the UID (or the name
//  in some cases which acts as the UID) for that bit to add. They can indicate not
//  to add this one if the current list is empty.
//
//  Security zones are currently set en masse via a separate method below.
//
//  Others just use the room function list, since they just need to store a single
//  UID value.
//
tCIDLib::TVoid
TCQCSysCfgRmInfo::AddHVAC(const TString& strUID, const tCIDLib::TBoolean bNotIfEmpty)
{
    const tCIDLib::TCard4 c4Count = m_colSelHVACs.c4ElemCount();
    if (!c4Count && bNotIfEmpty)
        return;

    // Add it if not already in this list
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (strUID.bCompareI(m_colSelHVACs.objAt(c4Index)))
            return;
    }

    // It's a new one, so add it
    m_colSelHVACs.objAdd(strUID);
}

tCIDLib::TVoid
TCQCSysCfgRmInfo::AddLoad(const TString& strUID, const tCIDLib::TBoolean bNotIfEmpty)
{
    const tCIDLib::TCard4 c4Count = m_colSelLoads.c4ElemCount();
    if (!c4Count && bNotIfEmpty)
        return;

    // Add it if not already in this list
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (strUID.bCompareI(m_colSelLoads.objAt(c4Index)))
            return;
    }

    // It's a new one, so add it
    m_colSelLoads.objAdd(strUID);
}


tCIDLib::TVoid
TCQCSysCfgRmInfo::AddXOver(const TString& strUID, const tCIDLib::TBoolean bNotIfEmpty)
{
    const tCIDLib::TCard4 c4Count = m_colXOvers.c4ElemCount();
    if (!c4Count && bNotIfEmpty)
        return;

    // Add it if not already in this list
    tCIDColAlgo::bAddIfNew(m_colXOvers, strUID, TString::bCompI);
}


//
//  Add a room mode action or update it if one already exists with the same title. We
//  return true if we added a new one.
//
tCIDLib::TBoolean
TCQCSysCfgRmInfo::bAddOrUpdateRoomMode(const TSCGlobActInfo& scliSrc)
{
    // See if we already have this one. If so, update it, else add it
    const tCIDLib::TCard4 c4Count = m_colRoomModes.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TSCGlobActInfo& scliCur = m_colRoomModes[c4Index];
        if (scliCur.m_strTitle.bCompareI(scliSrc.m_strTitle))
        {
            scliCur = scliSrc;
            return kCIDLib::False;
        }
    }

    // Not found, so add it
    m_colRoomModes.objAdd(scliSrc);
    return kCIDLib::True;
}


// Get/set the room selection enable flag
tCIDLib::TBoolean TCQCSysCfgRmInfo::bAllowRmSel() const
{
    return m_bAllowRmSel;
}

tCIDLib::TBoolean TCQCSysCfgRmInfo::bAllowRmSel(const tCIDLib::TBoolean bToSet)
{
    m_bAllowRmSel = bToSet;
    return m_bAllowRmSel;
}


// Get/set the enabled flag for this room
tCIDLib::TBoolean TCQCSysCfgRmInfo::bEnabled() const
{
    return m_bEnabled;
}

tCIDLib::TBoolean TCQCSysCfgRmInfo::bEnabled(const tCIDLib::TBoolean bToSet)
{
    m_bEnabled = bToSet;
    return m_bEnabled;
}


//
//  Get the enable/disable state for the prefined functions. If the function id
//  is not empty, then it's enabled.
//
tCIDLib::TBoolean
TCQCSysCfgRmInfo::bFuncEnabled(const tCQCSysCfg::ERmFuncs eFunc) const
{
    return !m_objaFuncIds[eFunc].bIsEmpty();
}


// Return whether a particular global action is enabled or not
tCIDLib::TBoolean
TCQCSysCfgRmInfo::bGlobalActEnabled(const tCQCSysCfg::EGlobActs eActIndex) const
{
    return m_objaGlobalActs[eActIndex].m_bEnabled;
}


// Find the named room mode action by its unique id and return its index in the list
tCIDLib::TCard4 TCQCSysCfgRmInfo::c4RoomModeById(const TString& strUID) const
{
    const tCIDLib::TCard4 c4Count = m_colRoomModes.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TSCGlobActInfo& scliCur = m_colRoomModes[c4Index];
        if (scliCur.m_strUniqueId.bCompareI(strUID))
            return c4Index;
    }
    return kCIDLib::c4MaxCard;
}


// The current counts of various lists of things we hold
tCIDLib::TCard4 TCQCSysCfgRmInfo::c4HVACCount() const
{
    return m_colSelHVACs.c4ElemCount();
}

tCIDLib::TCard4 TCQCSysCfgRmInfo::c4LoadCount() const
{
    return m_colSelLoads.c4ElemCount();
}

tCIDLib::TCard4 TCQCSysCfgRmInfo::c4RoomModeCount() const
{
    return m_colRoomModes.c4ElemCount();
}

tCIDLib::TCard4 TCQCSysCfgRmInfo::c4SecZoneCount() const
{
    return m_colSecZones.c4ElemCount();
}

tCIDLib::TCard4 TCQCSysCfgRmInfo::c4XOverCount() const
{
    return m_colXOvers.c4ElemCount();
}


//
//  Unlike the copy ctor, this will copy but change the unique id, to create a
//  new room with the same attributes as the original. We'll clear the name as
//  well.
//
tCIDLib::TVoid TCQCSysCfgRmInfo::CopyFrom(const TCQCSysCfgRmInfo& scriSrc)
{
    *this = scriSrc;

    m_strName.Clear();
    m_strUniqueId = TUniqueId::strMakeId();
}


tCIDLib::TVoid
TCQCSysCfgRmInfo::RemoveFromList(const  tCQCSysCfg::ECfgLists   eList
                                , const TString&                strUID)
{
    // Find the appropriate list and/or room function
    tCIDLib::TStrList* pcolList = nullptr;
    tCQCSysCfg::ERmFuncs eRoomFunc1 = tCQCSysCfg::ERmFuncs::Count;
    tCQCSysCfg::ERmFuncs eRoomFunc2 = tCQCSysCfg::ERmFuncs::Count;

    switch(eList)
    {
        case tCQCSysCfg::ECfgLists::Lighting :
            pcolList = &m_colSelLoads;
            eRoomFunc1 = tCQCSysCfg::ERmFuncs::Lighting;
            break;

        case tCQCSysCfg::ECfgLists::HVAC :
            pcolList = &m_colSelHVACs;
            eRoomFunc1 = tCQCSysCfg::ERmFuncs::HVAC;
            break;

        case tCQCSysCfg::ECfgLists::Media :
            eRoomFunc1 = tCQCSysCfg::ERmFuncs::Movies;
            eRoomFunc2 = tCQCSysCfg::ERmFuncs::Music;
            break;

        case tCQCSysCfg::ECfgLists::Security :
            eRoomFunc1 = tCQCSysCfg::ERmFuncs::Security;

            //
            //  This one is different in that the room func isn't a default it's the
            //  security device, and the list is the zones. So we just flush the
            //  zone list here. and don't pass it on to below.
            //
            m_colSecZones.RemoveAll();
            break;

        case tCQCSysCfg::ECfgLists::Weather :
            eRoomFunc1 = tCQCSysCfg::ERmFuncs::Weather;
            break;

        case tCQCSysCfg::ECfgLists::XOvers :
            pcolList = &m_colXOvers;
            break;

        default :
            CIDAssert(pcolList != nullptr, L"RemoveFromList doesn't support this data type");
            break;
    };

    //  If we got a list, remove the item from the list.
    if (pcolList)
        pcolList->bRemoveIfMember(strUID, TString::bCompI);

    // If we got room funcs (defaults) adjust those
    if (eRoomFunc1 != tCQCSysCfg::ERmFuncs::Count)
    {
        if (strFuncId(eRoomFunc1) == strUID)
        {
            if (pcolList && !pcolList->bIsEmpty())
                SetFuncId(eRoomFunc1, pcolList->objAt(0));
            else
                SetFuncId(eRoomFunc1, TString::strEmpty());
        }
    }

    if (eRoomFunc2 != tCQCSysCfg::ERmFuncs::Count)
    {
        if (strFuncId(eRoomFunc2) == strUID)
        {
            if (pcolList && !pcolList->bIsEmpty())
                SetFuncId(eRoomFunc2, pcolList->objAt(0));
            else
                SetFuncId(eRoomFunc2, TString::strEmpty());
        }
    }
}


// Remove a room mode, which isn't something that can be done via the list remove above
tCIDLib::TVoid TCQCSysCfgRmInfo::RemoveRoomModeAt(const tCIDLib::TCard4 c4At)
{
    m_colRoomModes.RemoveAt(c4At);
}



// Reset us back to default values
tCIDLib::TVoid TCQCSysCfgRmInfo::Reset()
{
    m_bAllowRmSel = kCIDLib::True;
    m_bEnabled = kCIDLib::True;
    m_strName.Clear();
    m_scvcVoice.Reset();

    // Clear and/or reset all the various lists
    m_colRoomModes.RemoveAll();
    m_colSecZones.RemoveAll();
    m_colSelHVACs.RemoveAll();
    m_colSelLoads.RemoveAll();
    m_colXOvers.RemoveAll();
    m_objaFuncIds.SetAll(TString::strEmpty());
    m_objaRoomVars.SetAll(TString::strEmpty());

    // And the global actions
    tCQCSysCfg::EGlobActs eAct = tCQCSysCfg::EGlobActs::Min;
    while (eAct <= tCQCSysCfg::EGlobActs::Max)
    {
        m_objaGlobalActs[eAct].Reset();
        eAct++;
    }

    // Assign a new unique id in case it gets reused
    m_strUniqueId = TUniqueId::strMakeId();
}


// Reset one of our global acions
tCIDLib::TVoid
TCQCSysCfgRmInfo::ResetGlobalAct(const tCQCSysCfg::EGlobActs eActIndex)
{
    m_objaGlobalActs[eActIndex].Reset();
}


//
//  Return a ref to the indicated global action or room mode. The non-const ones are just
//  to support the config editing interface. Don't otherwise modify it via those.
//
const TSCGlobActInfo&
TCQCSysCfgRmInfo::scliGlobActAt(const tCQCSysCfg::EGlobActs eActIndex) const
{
    return m_objaGlobalActs[eActIndex];
}

TSCGlobActInfo& TCQCSysCfgRmInfo::scliGlobActAt(const tCQCSysCfg::EGlobActs eActIndex)
{
    return m_objaGlobalActs[eActIndex];
}

const TSCGlobActInfo& TCQCSysCfgRmInfo::scliRoomModeAt(const tCIDLib::TCard4 c4Index) const
{
    return m_colRoomModes[c4Index];
}

TSCGlobActInfo& TCQCSysCfgRmInfo::scliRoomModeAt(const tCIDLib::TCard4 c4Index)
{
    return m_colRoomModes[c4Index];
}


//
//  Returns a ref to our voice configuration. The non-const one is just to support the
//  editor. Don't modify it via this otherwise.
//
const TCQCSysCfgVoice& TCQCSysCfgRmInfo::scvcVoice() const
{
    return m_scvcVoice;
}

TCQCSysCfgVoice& TCQCSysCfgRmInfo::scvcVoice()
{
    return m_scvcVoice;
}

// Return the unique id for the indicated room function
const TString& TCQCSysCfgRmInfo::strFuncId(const tCQCSysCfg::ERmFuncs eFunc) const
{
    return m_objaFuncIds[eFunc];
}


// Return the HVAC UID at the indicated index
const TString& TCQCSysCfgRmInfo::strHVACUIDAt(const tCIDLib::TCard4 c4At) const
{
    return m_colSelHVACs[c4At];
}


// Return the load UID at the indicated index
const TString& TCQCSysCfgRmInfo::strLoadUIDAt(const tCIDLib::TCard4 c4At) const
{
    return m_colSelLoads[c4At];
}


// Return the value of the indicated room variable
const TString& TCQCSysCfgRmInfo::strRoomVarAt(const tCIDLib::TCard4 c4At) const
{
    return m_objaRoomVars[c4At];
}


// Return the name or alt name of the zone at the indicated index
const TString& TCQCSysCfgRmInfo::strZoneAltNameAt(const tCIDLib::TCard4 c4At) const
{
    return m_colSecZones[c4At].strValue();
}

const TString& TCQCSysCfgRmInfo::strZoneNameAt(const tCIDLib::TCard4 c4At) const
{
    return m_colSecZones[c4At].strKey();
}


// Find the alt name for the indicted zone name. If none, return the name itself
const TString& TCQCSysCfgRmInfo::strZoneNameToAltName(const TString& strToFind) const
{
    const tCIDLib::TCard4 c4Count = m_colSecZones.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TKeyValuePair& kvalCur = m_colSecZones[c4Index];
        if (kvalCur.strKey().bCompareI(strToFind))
        {
            if (!kvalCur.strValue().bIsEmpty())
                return kvalCur.strValue();
            return kvalCur.strKey();
        }
    }

    CIDAssert2(TString(L"Could not find security zone ") + strToFind);

    static const TString strRet(L"Unknown zone");
    return strRet;
}


// Return the extra overlay UID at the indicated index
const TString& TCQCSysCfgRmInfo::strXOverUIDAt(const tCIDLib::TCard4 c4At) const
{
    return m_colXOvers[c4At];
}


// Get/set the room unique name
const TString& TCQCSysCfgRmInfo::strName() const
{
    return m_strName;
}

const TString& TCQCSysCfgRmInfo::strName(const TString& strToSet)
{
    m_strName = strToSet;
    return m_strName;
}


// Get the unique id
const TString& TCQCSysCfgRmInfo::strUniqueId() const
{
    return m_strUniqueId;
}


// Enable or disable a prefined funcion
tCIDLib::TVoid
TCQCSysCfgRmInfo::SetFuncId(const   tCQCSysCfg::ERmFuncs    eFunc
                            , const TString&                strToSet)
{
    m_objaFuncIds[eFunc] = strToSet;

    //
    //  Handle some special cases.
    //
    //  If disabling security, clear the zone list.
    //
    if (strToSet.bIsEmpty())
    {
        if (eFunc == tCQCSysCfg::ERmFuncs::Security)
            m_colSecZones.RemoveAll();
    }
}


//
//  Set the lighting loads or HVAC list for this room. It's the UIDs of the ones to
//  set.
//
tCIDLib::TVoid TCQCSysCfgRmInfo::SetHVACs(const tCIDLib::TStrCollect& colToSet)
{
    // Remove any current ones and then iterate the source and add them to our list
    m_colSelHVACs.RemoveAll();

    TColCursor<TString>& cursSrc = *colToSet.pcursNew();
    TJanitor<TColCursor<TString>> janCursor(&cursSrc);
    for (; cursSrc; ++cursSrc)
        m_colSelHVACs.objAdd(*cursSrc);
}

tCIDLib::TVoid TCQCSysCfgRmInfo::SetLoads(const tCIDLib::TStrCollect& colToSet)
{
    // Remove any current ones and then iterate the source and add them to our list
    m_colSelLoads.RemoveAll();

    TColCursor<TString>& cursSrc = *colToSet.pcursNew();
    TJanitor<TColCursor<TString>> janCursor(&cursSrc);
    for (; cursSrc; ++cursSrc)
        m_colSelLoads.objAdd(*cursSrc);
}


// Set the value of a room variable
tCIDLib::TVoid
TCQCSysCfgRmInfo::SetRoomVar(const  tCIDLib::TCard4 c4At
                            , const TString&        strToSet)
{
    m_objaRoomVars[c4At] = strToSet;
}


//
//  Set the security zone list for this room. It's the names/altnames of the zones
//  that are in this room.
//
tCIDLib::TVoid
TCQCSysCfgRmInfo::SetSecZones(const tCIDLib::TKVPCollect& colToSet)
{
    // Remove any current zones first
    m_colSecZones.RemoveAll();

    // And now iterate the source and add them to our list
    TColCursor<TKeyValuePair>& cursSrc = *colToSet.pcursNew();
    TJanitor<TColCursor<TKeyValuePair>> janCursor(&cursSrc);
    for (; cursSrc; ++cursSrc)
        m_colSecZones.objAdd(*cursSrc);
}


// Update the alt name of a zone
tCIDLib::TVoid
TCQCSysCfgRmInfo::SetZoneAltNameAt(const tCIDLib::TCard4 c4At, const TString& strToSet)
{
    m_colSecZones[c4At].strValue(strToSet);
}


// Update our voice configuration
tCIDLib::TVoid TCQCSysCfgRmInfo::SetVoiceCfg(const TCQCSysCfgVoice& scvcVoice)
{
    m_scvcVoice = scvcVoice;
}


// Set the extra overlays list for this room. It's the UIDs of the configured overlays
tCIDLib::TVoid
TCQCSysCfgRmInfo::SetXOvers(const tCIDLib::TStrCollect& colToSet)
{
    // Remove any current zones first
    m_colXOvers.RemoveAll();

    // And now iterate the source and add them to our list
    TColCursor<TString>& cursSrc = *colToSet.pcursNew();
    TJanitor<TColCursor<TString>> janCursor(&cursSrc);
    for (; cursSrc; ++cursSrc)
        m_colXOvers.objAdd(*cursSrc);
}


//
//  Format our content out to JSON or XML. We have to get the overall system config as
//  well, since we need to look up things that our room references.
//
tCIDLib::TVoid
TCQCSysCfgRmInfo::ToJSON(const  TCQCSysCfg&         scfgInfo
                        ,       TTextOutStream&     strmOut
                        , const tCIDLib::TBoolean   bAppend) const
{
    // If not appending, then reset the stream
    if (!bAppend)
        strmOut.Reset();

    strmOut << L"{\n";
    TJSONParser::OutputObjHdr(strName(), strmOut);

    // Output the values for this room
    TJSONParser::OutputBool(L"Music", bFuncEnabled(tCQCSysCfg::ERmFuncs::Music), strmOut);
    TJSONParser::OutputBool(L"Movies", bFuncEnabled(tCQCSysCfg::ERmFuncs::Movies), strmOut);

    //
    //  As a final value, we send our CQC version. Indicate it's the final version so
    //  that no comma is appended.
    //
    TJSONParser::OutputText(L"CQCVer", kCQCKit::pszVersion, strmOut, kCIDLib::True);

    // Close off the room object
    strmOut << L"}\n";
}

tCIDLib::TVoid
TCQCSysCfgRmInfo::ToXML(const   TCQCSysCfg&         scfgInfo
                        ,       TTextOutStream&     strmOut
                        , const tCIDLib::TBoolean   bAppend) const
{
    // If not appending, then reset the stream
    if (!bAppend)
        strmOut.Reset();

    tCIDLib::TCard4 c4SelIndex;

    //
    //  Start the main room element, which includes a summary of what major room
    //  features are enabled.
    //
    strmOut << L"<CQCSysCfg:Room CQCSysCfg:Name=\"";
    facCIDXML().EscapeFor(strName(), strmOut, tCIDXML::EEscTypes::Attribute, TString::strEmpty());

    strmOut << L"\" CQCSysCfg:Music=\""
            << facCQCKit().strBoolYesNo(bFuncEnabled(tCQCSysCfg::ERmFuncs::Music))
            << L"\" CQCSysCfg:Movies=\""
            << facCQCKit().strBoolYesNo(bFuncEnabled(tCQCSysCfg::ERmFuncs::Movies))
            << L"\" CQCSysCfg:Security=\""
            << facCQCKit().strBoolYesNo(bFuncEnabled(tCQCSysCfg::ERmFuncs::Security))
            << L"\" CQCSysCfg:Weather=\""
            << facCQCKit().strBoolYesNo(bFuncEnabled(tCQCSysCfg::ERmFuncs::Weather))
            << L"\">\n";

    // Put out the per-room variables
    strmOut << L"    <CQCSysCfg:RoomVars>\n";
    for (tCIDLib::TCard4 c4Index = 0; c4Index < kCQCSysCfg::c4MaxRoomVars; c4Index++)
    {
        strmOut << L"        <CQCSysCfg:RoomVar>";
        facCIDXML().EscapeFor
        (
            m_objaRoomVars[c4Index], strmOut, tCIDXML::EEscTypes::ElemText, TString::strEmpty()
        );
        strmOut << L"</CQCSysCfg:RoomVar>\n";
    }
    strmOut << L"    </CQCSysCfg:RoomVars>\n";

    // If music is enabled, let's spit that out
    if (bFuncEnabled(tCQCSysCfg::ERmFuncs::Music))
    {
        const TSCMediaInfo& scliMusic = scfgInfo.scliMediaByUID
        (
            m_objaFuncIds[tCQCSysCfg::ERmFuncs::Music], c4SelIndex
        );

        strmOut << L"    <CQCSysCfg:Music CQCSysCfg:Player=\""
                << scliMusic.m_strRendMon
                << L"\" CQCSysCfg:Repo=\""
                << scliMusic.m_strRepoMon
                << L"\" CQCSysCfg:Audio=\""
                << scliMusic.m_strAudioMon
                << L"\" CQCSysCfg:AudioSub=\""
                << scliMusic.m_strAudioSub
                << L"\"/>\n";
    }

    // If movies are enabled, let's spit that out
    if (bFuncEnabled(tCQCSysCfg::ERmFuncs::Movies))
    {
        const TSCMediaInfo& scliMusic = scfgInfo.scliMediaByUID
        (
            m_objaFuncIds[tCQCSysCfg::ERmFuncs::Movies], c4SelIndex
        );

        strmOut << L"    <CQCSysCfg:Movies CQCSysCfg:Player=\""
                << scliMusic.m_strRendMon
                << L"\" CQCSysCfg:Repo=\""
                << scliMusic.m_strRepoMon
                << L"\" CQCSysCfg:Audio=\""
                << scliMusic.m_strAudioMon
                << L"\" CQCSysCfg:AudioSub=\""
                << scliMusic.m_strAudioSub
                << L"\"/>\n";
    }

    // If security is enabled, let's spit that out
    if (bFuncEnabled(tCQCSysCfg::ERmFuncs::Security))
    {
        const TSCSecInfo& scliSec = scfgInfo.scliSecByUID
        (
            m_objaFuncIds[tCQCSysCfg::ERmFuncs::Security], c4SelIndex
        );

        strmOut << L"    <CQCSysCfg:Security CQCSysCfg:Panel=\""
                << scliSec.m_strMoniker
                << L"\" CQCSysCfg:SecArea=\""
                << scliSec.m_strAreaName
                << L"\" CQCSysCfg:ZoneCnt=\""
                << m_colSecZones.c4ElemCount()
                << L"\"/>\n";
    }

    // If weather is enabled, let's spit that out
    if (bFuncEnabled(tCQCSysCfg::ERmFuncs::Weather))
    {
        const TSCWeatherInfo& scliWeather = scfgInfo.scliWeatherByUID
        (
            m_objaFuncIds[tCQCSysCfg::ERmFuncs::Weather], c4SelIndex
        );

        strmOut << L"    <CQCSysCfg:Weather CQCSysCfg:Drv=\""
                << scliWeather.m_strMoniker
                << L"\" CQCSysCfg:Scale=\""
                << (scliWeather.m_bFScale ? L"F" : L"C")
                << L"\"/>\n";
    }

    // If any HVACs are configured for this room, do them
    if (!m_colSelHVACs.bIsEmpty())
    {
        const TSCHVACInfo& scliDef = scfgInfo.scliHVACByUID
        (
            m_objaFuncIds[tCQCSysCfg::ERmFuncs::HVAC], c4SelIndex
        );

        // The default load had to go in the
        strmOut << L"    <CQCSysCfg:HVACs CQCSysCfg:DefHVAC=\""
                << scliDef.m_strTitle
                << L"\">\n";

        const tCIDLib::TCard4 c4Count = m_colSelHVACs.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const TSCHVACInfo& scliCur = scfgInfo.scliHVACByUID
            (
                m_colSelHVACs[c4Index], c4SelIndex
            );

            strmOut << L"    <CQCSysCfg:HVAC CQCSysCfg:Name=\""
                    << scliCur.m_strTitle
                    << L"\" CQCSysCfg:Moniker=\""
                    << scliCur.m_strMoniker
                    << L"\"/>";
        }
    }

    // If any loads are configured for this room, do them
    if (!m_colSelLoads.bIsEmpty())
    {
        const TSCLoadInfo& scliDef = scfgInfo.scliLoadByUID
        (
            m_objaFuncIds[tCQCSysCfg::ERmFuncs::Lighting], c4SelIndex
        );

        // The default load had to go in the
        strmOut << L"    <CQCSysCfg:Loads CQCSysCfg:DefLoad=\""
                << scliDef.m_strTitle
                << L"\">\n";

        const tCIDLib::TCard4 c4Count = m_colSelLoads.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const TSCLoadInfo& scliCur = scfgInfo.scliLoadByUID
            (
                m_colSelLoads[c4Index], c4SelIndex
            );

            strmOut << L"    <CQCSysCfg:Load CQCSysCfg:Name=\""
                    << scliCur.m_strTitle
                    << L"\"";

            if (scliCur.bIsDimmer())
            {
                strmOut << L" CQCSysCfg:Dimmer=\""
                        << scliCur.m_strField
                        << L"\"";

                if (!scliCur.m_strField2.bIsEmpty())
                {
                    strmOut << L" CQCSysCfg:Switch=\""
                            << scliCur.m_strField2
                            << L"\"";
                }
            }
             else
            {
                strmOut << L" CQCSysCfg:Switch=\""
                        << scliCur.m_strField
                        << L"\"";
            }

            if (!scliCur.m_strClrField.bIsEmpty())
            {
                strmOut << L" CQCSysCfg:Color=\""
                        << scliCur.m_strClrField
                        << L"\"";
            }
            strmOut << L"/>\n";
        }

        strmOut << L"    </CQCSysCfg:Loads>\n";
    }

    // If any global actions are enabled, output them
    {
        tCIDLib::TBoolean bDoLoads = kCIDLib::False;
        tCQCSysCfg::EGlobActs eAct = tCQCSysCfg::EGlobActs::Min;
        while (eAct <= tCQCSysCfg::EGlobActs::Max)
        {
            if (m_objaGlobalActs[eAct].m_bEnabled)
            {
                bDoLoads = kCIDLib::True;
                break;
            }
            eAct++;
        }

        if (bDoLoads)
        {
            strmOut << L"    <CQCSysCfg:GlobActs>\n";

            eAct = tCQCSysCfg::EGlobActs::Min;
            while (eAct <= tCQCSysCfg::EGlobActs::Max)
            {
                const TSCGlobActInfo& scliCur = m_objaGlobalActs[eAct];
                if (scliCur.m_bEnabled)
                {
                    strmOut << L"        <CQCSysCfg:GlobAct CQCSysCfg:Name=\""
                            << tCQCSysCfg::strXlatEGlobActs(eAct)
                            << L"\" CQCSysCfg:ActPath=\"";

                    facCIDXML().EscapeFor
                    (
                        scliCur.m_strPath, strmOut, tCIDXML::EEscTypes::Attribute, TString::strEmpty()
                    );
                    strmOut << L"\">\n";


                    // Output the parameters
                    for (tCIDLib::TCard4 c4PInd = 0; c4PInd < kCQCSysCfg::c4MaxGActParms; c4PInd++)
                    {
                        strmOut << L"            <CQCGW:ActParm>";
                        facCIDXML().EscapeFor
                        (
                            scliCur.m_colParms[c4PInd]
                            , strmOut
                            , tCIDXML::EEscTypes::ElemText
                            , TString::strEmpty()
                        );
                        strmOut << L"</CQCGW:ActParm>\n";
                    }
                    strmOut << L"        </CQCSysCfg:GlobAct>\n";
                }

                eAct++;
            }

            strmOut << L"    </CQCSysCfg:GlobActs>\n";
        }
    }

    // And now close off the room element
    strmOut << L"</CQCSysCfg:Room>\n";
}



// ---------------------------------------------------------------------------
//  TCQCSysCfgRmInfo: Protected, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid TCQCSysCfgRmInfo::StreamFrom(TBinInStream& strmToReadFrom)
{
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCKit_Room::c2FmtRoom))
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

    strmToReadFrom  >> m_bAllowRmSel
                    >> m_bEnabled;

    // If V3 or beyond, we read in the security zones, else we have to convert
    if (c2FmtVersion > 2)
    {
        strmToReadFrom >> m_colSecZones;
    }
     else
    {
        tCIDLib::TStrList colTmp;
        strmToReadFrom >> colTmp;

        m_colSecZones.RemoveAll();
        const tCIDLib::TCard4 c4Count = colTmp.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            m_colSecZones.objPlace(colTmp[c4Index], TString::strEmpty());
    }

    strmToReadFrom  >> m_colSelLoads
                    >> m_colXOvers
                    >> m_objaFuncIds
                    >> m_objaGlobalActs
                    >> m_objaRoomVars
                    >> m_strName
                    >> m_strUniqueId;

    //
    //  If V1, we need to expand the function ids object array by one more, and to
    //  default the new select HVACs list, else read it in.
    //
    if (c2FmtVersion == 1)
    {
        // Copy the array, reallocate ours, then copy back over
        TFuncIds objaSave(m_objaFuncIds);
        m_objaFuncIds.Realloc(tCQCSysCfg::ERmFuncs::Count);

        const tCQCSysCfg::ERmFuncs eCount = objaSave.tElemCount();
        tCQCSysCfg::ERmFuncs eIndex = tCQCSysCfg::ERmFuncs::Min;
        while (eIndex <= tCQCSysCfg::ERmFuncs::Max)
        {
            m_objaFuncIds[eIndex] = objaSave[eIndex];
            eIndex++;
        }

        m_colSelHVACs.RemoveAll();
    }
     else
    {
        strmToReadFrom >> m_colSelHVACs;
    }

    // If V4 or beyond, read in the room modes. Else, default it
    if (c2FmtVersion > 3)
        strmToReadFrom >> m_colRoomModes;
    else
        m_colRoomModes.RemoveAll();

    // If V5 or beyond, read in the voice configuration, else default it
    if (c2FmtVersion > 4)
        strmToReadFrom >> m_scvcVoice;
    else
        m_scvcVoice.Reset();

    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TCQCSysCfgRmInfo::StreamTo(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCKit_Room::c2FmtRoom

                    << m_bAllowRmSel
                    << m_bEnabled

                    // In V3 and beyond this is a list of key/value pairs
                    << m_colSecZones

                    << m_colSelLoads
                    << m_colXOvers
                    << m_objaFuncIds
                    << m_objaGlobalActs
                    << m_objaRoomVars
                    << m_strName
                    << m_strUniqueId

                    // V2 stuff
                    << m_colSelHVACs

                    // V4 stuff
                    << m_colRoomModes

                    // V5 stuff
                    << m_scvcVoice

                    << tCIDLib::EStreamMarkers::EndObject;
}


