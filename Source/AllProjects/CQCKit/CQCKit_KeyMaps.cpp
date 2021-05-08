//
// FILE NAME: CQCAct_KeyMaps.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/06/2001
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
//  This file implements the helper classes for maintaining the per user keyboard
//  mappings.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCKit_.hpp"



// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCKeyMapItem,TCQCStdCmdSrc)
RTTIDecls(TCQCKeyMap,TObject)


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace CQCKit_KeyMaps
{
    namespace
    {
        // -----------------------------------------------------------------------
        //  Our current and former persistent format versions
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2       c2ItemFmtVersion    = 2;
        constexpr tCIDLib::TCard2       c2MapFmtVersion     = 3;


        // -----------------------------------------------------------------------
        //  This is the order that we put the keys into the list box.
        // -----------------------------------------------------------------------
        constexpr tCIDCtrls::EExtKeys     aeKeys[] =
        {
            tCIDCtrls::EExtKeys::F2
            , tCIDCtrls::EExtKeys::F3
            , tCIDCtrls::EExtKeys::F4
            , tCIDCtrls::EExtKeys::F5
            , tCIDCtrls::EExtKeys::F6
            , tCIDCtrls::EExtKeys::F7
            , tCIDCtrls::EExtKeys::F8
            , tCIDCtrls::EExtKeys::F9

            , tCIDCtrls::EExtKeys::Volume_Mute
            , tCIDCtrls::EExtKeys::Volume_Down
            , tCIDCtrls::EExtKeys::Volume_Up
            , tCIDCtrls::EExtKeys::Media_PlayPause
            , tCIDCtrls::EExtKeys::Media_Stop
            , tCIDCtrls::EExtKeys::Media_PrevTrack
            , tCIDCtrls::EExtKeys::Media_NextTrack

            , tCIDCtrls::EExtKeys::Browser_Back
            , tCIDCtrls::EExtKeys::Browser_Forward
            , tCIDCtrls::EExtKeys::Browser_Stop
            , tCIDCtrls::EExtKeys::Browser_Refresh
            , tCIDCtrls::EExtKeys::Browser_Search
            , tCIDCtrls::EExtKeys::Browser_Favorites
            , tCIDCtrls::EExtKeys::Browser_Home

            , tCIDCtrls::EExtKeys::Down
            , tCIDCtrls::EExtKeys::Left
            , tCIDCtrls::EExtKeys::Right
            , tCIDCtrls::EExtKeys::Up
        };
        constexpr tCIDLib::TCard4       c4KeyCount = tCIDLib::c4ArrayElems(aeKeys);


        // -----------------------------------------------------------------------
        //  The large previous count of items in the map before this version
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard4       c4OldCount = 22;


        // -----------------------------------------------------------------------
        //  An array of message ids that holds the text that we display for each
        //  name.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TMsgId       amidKeyNames[c4KeyCount] =
        {
            kKitMsgs::midKey_F2
            , kKitMsgs::midKey_F3
            , kKitMsgs::midKey_F4
            , kKitMsgs::midKey_F5
            , kKitMsgs::midKey_F6
            , kKitMsgs::midKey_F7
            , kKitMsgs::midKey_F8
            , kKitMsgs::midKey_F9
            , kKitMsgs::midKey_Volume_Mute
            , kKitMsgs::midKey_Volume_Down
            , kKitMsgs::midKey_Volume_Up
            , kKitMsgs::midKey_Media_PlayPause
            , kKitMsgs::midKey_Media_Stop
            , kKitMsgs::midKey_Media_PrevTrack
            , kKitMsgs::midKey_Media_NextTrack
            , kKitMsgs::midKey_Browser_Back
            , kKitMsgs::midKey_Browser_Forward
            , kKitMsgs::midKey_Browser_Stop
            , kKitMsgs::midKey_Browser_Refresh
            , kKitMsgs::midKey_Browser_Search
            , kKitMsgs::midKey_Browser_Favorites
            , kKitMsgs::midKey_Browser_Home
            , kKitMsgs::midKey_Down
            , kKitMsgs::midKey_Left
            , kKitMsgs::midKey_Right
            , kKitMsgs::midKey_Up
        };


        // -----------------------------------------------------------------------
        //  An array of pointers to strings that we fault in upon first use. We
        //  load up the text for the messages ids in amidKeyNames above.
        // -----------------------------------------------------------------------
        TAtomicFlag         atomInitDone;
        const TString*      apstrNames[c4KeyCount];
    }
}



// ---------------------------------------------------------------------------
//  CLASS: TCQCKeyMapItem
// PREFIX: kmi
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCKeyMapItem: Public, static methods
// ---------------------------------------------------------------------------
tCIDLib::TCard4 TCQCKeyMapItem::c4FindKeyIndex(const tCIDCtrls::EExtKeys eToFind)
{
    tCIDLib::TCard4 c4Index = 0;
    for (; c4Index < CQCKit_KeyMaps::c4KeyCount; c4Index++)
    {
        if (eToFind == CQCKit_KeyMaps::aeKeys[c4Index])
            break;
    }

    // If we didn't find it, then throw
    if (c4Index == CQCKit_KeyMaps::c4KeyCount)
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcKMap_InvalidFKey
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Internal
            , TInteger(tCIDLib::i4EnumOrd(eToFind))
        );
    }
    return c4Index;
}


TCQCKeyMapItem& TCQCKeyMapItem::Nul_TCQCKeyMapItem() noexcept
{
    static TCQCKeyMapItem kmiNull;
    return kmiNull;
}


// ---------------------------------------------------------------------------
//  TCQCKeyMapItem: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCKeyMapItem::TCQCKeyMapItem() :

    m_eKey(tCIDCtrls::EExtKeys::F2)
{
    AddEvent
    (
        facCQCKit().strMsg(kKitMsgs::midCmdEv_OnPress)
        , kCQCKit::strEvId_OnPress
        , tCQCKit::EActCmdCtx::Standard
    );
}

TCQCKeyMapItem::TCQCKeyMapItem(const tCIDCtrls::EExtKeys eKey) :

    m_eKey(eKey)
{
    AddEvent
    (
        facCQCKit().strMsg(kKitMsgs::midCmdEv_OnPress)
        , kCQCKit::strEvId_OnPress
        , tCQCKit::EActCmdCtx::Standard
    );
}

TCQCKeyMapItem::TCQCKeyMapItem( const   tCIDCtrls::EExtKeys eKey
                                , const TString&            strTitle) :

    TCQCStdCmdSrc(strTitle)
    , m_eKey(eKey)
{
    AddEvent
    (
        facCQCKit().strMsg(kKitMsgs::midCmdEv_OnPress)
        , kCQCKit::strEvId_OnPress
        , tCQCKit::EActCmdCtx::Standard
    );
}

TCQCKeyMapItem::~TCQCKeyMapItem()
{
}


// ---------------------------------------------------------------------------
//  TCQCKeyMapItem: Public operators
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TCQCKeyMapItem::operator==(const TCQCKeyMapItem& kmiSrc) const
{
    if (this == &kmiSrc)
        return kCIDLib::True;

    return(TParent::operator==(kmiSrc) && (m_eKey == kmiSrc.m_eKey));
}

tCIDLib::TBoolean TCQCKeyMapItem::operator!=(const TCQCKeyMapItem& kmiSrc) const
{
    return !TCQCKeyMapItem::operator==(kmiSrc);
}


// ---------------------------------------------------------------------------
//  TCQCKeyMapItem: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDCtrls::EExtKeys TCQCKeyMapItem::eKey() const
{
    return m_eKey;
}

tCIDCtrls::EExtKeys TCQCKeyMapItem::eKey(const tCIDCtrls::EExtKeys eNew)
{
    // If in debug mode, then confirm its a valid key
    #if CID_DEBUG_ON
    c4FindKeyIndex(eNew);
    #endif

    m_eKey = eNew;
    return m_eKey;
}


// ---------------------------------------------------------------------------
//  TCQCKeyMapItem: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCKeyMapItem::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Let our parent stream his stuff in first
    TParent::StreamFrom(strmToReadFrom);

    // And next should be the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCKit_KeyMaps::c2ItemFmtVersion))
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

    strmToReadFrom >> m_eKey;
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);

    // Sanity check the key
    c4FindKeyIndex(m_eKey);
}

tCIDLib::TVoid TCQCKeyMapItem::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Let our parent stream his stuff first
    TParent::StreamTo(strmToWriteTo);

    // And then do our stuff
    strmToWriteTo   << CQCKit_KeyMaps::c2ItemFmtVersion
                    << m_eKey
                    << tCIDLib::EStreamMarkers::EndObject;
}




// ---------------------------------------------------------------------------
//  CLASS: TCQCKeyMap
// PREFIX: km
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCKeyMap: Public, static methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TCQCKeyMap::bIsLegalKey(const tCIDCtrls::EExtKeys eKey)
{
    tCIDLib::TCard4 c4Index = 0;
    for (; c4Index < CQCKit_KeyMaps::c4KeyCount; c4Index++)
    {
        if (eKey == CQCKit_KeyMaps::aeKeys[c4Index])
            return kCIDLib::True;
    }
    return kCIDLib::False;
}


tCIDLib::TCard4 TCQCKeyMap::c4KeyCount()
{
    return CQCKit_KeyMaps::c4KeyCount;
}


const TString& TCQCKeyMap::strKeyName(const tCIDCtrls::EExtKeys eKey)
{
    return strKeyName(TCQCKeyMapItem::c4FindKeyIndex(eKey));
}


const TString& TCQCKeyMap::strKeyName(const tCIDLib::TCard4 c4Index)
{
    // Validate the index
    if (c4Index >= CQCKit_KeyMaps::c4KeyCount)
    {
        // Throw a generic CIDLib error, not one of ours!
        facCIDLib().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCIDErrs::errcGen_IndexError
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::BadParms
            , TCardinal(c4Index)
            , TCQCKeyMap::clsThis()
            , TCardinal(CQCKit_KeyMaps::c4KeyCount)
        );
    }

    // Fault in the strings if they haven't already been
    if (!CQCKit_KeyMaps::atomInitDone)
    {
        TBaseLock lockInit;
        if (!CQCKit_KeyMaps::atomInitDone)
        {
            tCIDLib::TCard4 c4Index = 0;
            for (; c4Index < CQCKit_KeyMaps::c4KeyCount; c4Index++)
            {
                CQCKit_KeyMaps::apstrNames[c4Index] = new TString
                (
                    CQCKit_KeyMaps::amidKeyNames[c4Index]
                    , facCQCKit()
                );
            }
            CQCKit_KeyMaps::atomInitDone.Set();
        }
    }
    return *CQCKit_KeyMaps::apstrNames[c4Index];
}


// ---------------------------------------------------------------------------
//  TCQCKeyMap: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCKeyMap::TCQCKeyMap() :

    m_colAllShifted(CQCKit_KeyMaps::c4KeyCount)
    , m_colCtrlShifted(CQCKit_KeyMaps::c4KeyCount)
    , m_colShifted(CQCKit_KeyMaps::c4KeyCount)
    , m_colUnshifted(CQCKit_KeyMaps::c4KeyCount)
{
    // Initialize them all with the correct virtual keys
    tCIDLib::TCard4 c4Index = 0;
    for (; c4Index < CQCKit_KeyMaps::c4KeyCount; c4Index++)
    {
        m_colAllShifted.objAdd(CQCKit_KeyMaps::aeKeys[c4Index]);
        m_colCtrlShifted.objAdd(CQCKit_KeyMaps::aeKeys[c4Index]);
        m_colShifted.objAdd(CQCKit_KeyMaps::aeKeys[c4Index]);
        m_colUnshifted.objAdd(CQCKit_KeyMaps::aeKeys[c4Index]);
    }
}

TCQCKeyMap::~TCQCKeyMap()
{
}


// ---------------------------------------------------------------------------
//  TCQCKeyMap: Public operators
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TCQCKeyMap::operator==(const TCQCKeyMap& kmSrc) const
{
    if (this == &kmSrc)
        return kCIDLib::True;

    return
    (
        (m_colAllShifted == kmSrc.m_colAllShifted)
        && (m_colCtrlShifted == kmSrc.m_colCtrlShifted)
        && (m_colShifted == kmSrc.m_colShifted)
        && (m_colUnshifted == kmSrc.m_colUnshifted)
    );
}

tCIDLib::TBoolean TCQCKeyMap::operator!=(const TCQCKeyMap& kmSrc) const
{
    return !TCQCKeyMap::operator==(kmSrc);
}


// ---------------------------------------------------------------------------
//  TCQCKeyMap: Public, non-virtual methods
// ---------------------------------------------------------------------------
const TVector<TCQCKeyMapItem>&
TCQCKeyMap::colForShifts(const  tCIDLib::TBoolean   bShift
                        , const tCIDLib::TBoolean   bCtrlShift) const
{
    return colFindCol(bShift, bCtrlShift);
}

TVector<TCQCKeyMapItem>&
TCQCKeyMap::colForShifts(const  tCIDLib::TBoolean   bShift
                        , const tCIDLib::TBoolean   bCtrlShift)
{
    return colFindCol(bShift, bCtrlShift);
}


const TCQCKeyMapItem&
TCQCKeyMap::kmiFind(const   tCIDCtrls::EExtKeys eKey
                    , const tCIDLib::TBoolean   bShift
                    , const tCIDLib::TBoolean   bCtrlShift) const
{
    // Validate the key and get its index
    const tCIDLib::TCard4 c4Index = TCQCKeyMapItem::c4FindKeyIndex(eKey);

    // Its ok, so find the col for the shift states, and return that item
    const TVector<TCQCKeyMapItem>& colTarget = colFindCol(bShift, bCtrlShift);
    return colTarget[c4Index];
}

TCQCKeyMapItem&
TCQCKeyMap::kmiFind(const   tCIDCtrls::EExtKeys eKey
                    , const tCIDLib::TBoolean   bShift
                    , const tCIDLib::TBoolean   bCtrlShift)
{
    // Validate the key and get its index
    const tCIDLib::TCard4 c4Index = TCQCKeyMapItem::c4FindKeyIndex(eKey);

    // Its ok, so find the col for the shift states, and return that item
    TVector<TCQCKeyMapItem>& colTarget = colFindCol(bShift, bCtrlShift);
    return colTarget[c4Index];
}


// Mark all of our entries as unused
tCIDLib::TVoid TCQCKeyMap::Reset()
{
    tCIDLib::TCard4 c4Count;

    c4Count = m_colAllShifted.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        m_colAllShifted[c4Index].bUsed(kCIDLib::False);

    c4Count = m_colCtrlShifted.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        m_colCtrlShifted[c4Index].bUsed(kCIDLib::False);

    c4Count = m_colShifted.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        m_colShifted[c4Index].bUsed(kCIDLib::False);

    c4Count = m_colUnshifted.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        m_colUnshifted[c4Index].bUsed(kCIDLib::False);
}


// ---------------------------------------------------------------------------
//  TCQCKeyMap: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCKeyMap::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Stream out a marker to start it off
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // And next should be the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCKit_KeyMaps::c2MapFmtVersion))
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

    // Flush our current collections, to get ready to put in the new ones
    m_colAllShifted.RemoveAll();
    m_colCtrlShifted.RemoveAll();
    m_colShifted.RemoveAll();
    m_colUnshifted.RemoveAll();

    //
    //  If an old version, then we have to use a set of temporary collections.
    //  We can't use the regular ones, because when we read these back in,
    //  they will set the max elements, so we won't be able to re-extend them
    //  back out to hold the new keys.
    //
    if (c2FmtVersion < CQCKit_KeyMaps::c2MapFmtVersion)
    {
        //
        //  Declare the temp ones and read into them. Make them big enough
        //  to hold the previous biggest size so that they don't have to
        //  re-allocate.
        //
        TVector<TCQCKeyMapItem> colAllShifted(CQCKit_KeyMaps::c4OldCount);
        TVector<TCQCKeyMapItem> colCtrlShifted(CQCKit_KeyMaps::c4OldCount);
        TVector<TCQCKeyMapItem> colShifted(CQCKit_KeyMaps::c4OldCount);
        TVector<TCQCKeyMapItem> colUnshifted(CQCKit_KeyMaps::c4OldCount);

        strmToReadFrom  >> colAllShifted
                        >> colCtrlShifted
                        >> colShifted
                        >> colUnshifted;

        // Copy over from the temps into the real collections.
        tCIDLib::TCard4 c4Index = 0;

        const tCIDLib::TCard4 c4Count = colAllShifted.c4ElemCount();
        for (; c4Index < c4Count; c4Index++)
        {
            m_colAllShifted.objAdd(colAllShifted[c4Index]);
            m_colCtrlShifted.objAdd(colCtrlShifted[c4Index]);
            m_colShifted.objAdd(colShifted[c4Index]);
            m_colUnshifted.objAdd(colUnshifted[c4Index]);
        }

        //
        //  And now extend the real collections out with default entries
        //  for the new keys that we support.
        //
        for (; c4Index < CQCKit_KeyMaps::c4KeyCount; c4Index++)
        {
            m_colAllShifted.objAdd(CQCKit_KeyMaps::aeKeys[c4Index]);
            m_colCtrlShifted.objAdd(CQCKit_KeyMaps::aeKeys[c4Index]);
            m_colShifted.objAdd(CQCKit_KeyMaps::aeKeys[c4Index]);
            m_colUnshifted.objAdd(CQCKit_KeyMaps::aeKeys[c4Index]);
        }

        CIDAssert
        (
            m_colUnshifted.c4ElemCount() == CQCKit_KeyMaps::c4KeyCount
            , L"The updating of old key mappings created bad key map length"
        );
    }
     else
    {
        // Its up to date so just stream in directly to our collections
        strmToReadFrom  >> m_colAllShifted
                        >> m_colCtrlShifted
                        >> m_colShifted
                        >> m_colUnshifted;
    }

    // And check for the trailing end object marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}

tCIDLib::TVoid TCQCKeyMap::StreamTo(TBinOutStream& strmToWriteTo) const
{
    //
    //  First we stream out a stream marker and the format version, then
    //  each of the collections of key map items. Finish it off with an
    //  end marker.
    //
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCKit_KeyMaps::c2MapFmtVersion
                    << m_colAllShifted
                    << m_colCtrlShifted
                    << m_colShifted
                    << m_colUnshifted
                    << tCIDLib::EStreamMarkers::EndObject;
}


// ---------------------------------------------------------------------------
//  TCQCKeyMap: Private, non-virtual methods
// ---------------------------------------------------------------------------
TVector<TCQCKeyMapItem>&
TCQCKeyMap::colFindCol( const   tCIDLib::TBoolean   bShift
                        , const tCIDLib::TBoolean   bCtrlShift)
{
    if (bCtrlShift && bShift)
        return m_colAllShifted;
    else if (!bCtrlShift && !bShift)
        return m_colUnshifted;
    else if (bCtrlShift && !bShift)
        return m_colCtrlShifted;

    return m_colShifted;
}


const TVector<TCQCKeyMapItem>&
TCQCKeyMap::colFindCol( const   tCIDLib::TBoolean   bShift
                        , const tCIDLib::TBoolean   bCtrlShift) const
{
    if (bCtrlShift && bShift)
        return m_colAllShifted;
    else if (!bCtrlShift && !bShift)
        return m_colUnshifted;
    else if (bCtrlShift && !bShift)
        return m_colCtrlShifted;

    return m_colShifted;
}


