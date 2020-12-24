//
// FILE NAME: CQCAdmin_ContentTab.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/07/2015
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
//  This file implements a base tab window class from which all the viewing/editing
//  tabs derive.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCAdmin.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TContentTab, TTabWindow)



// ---------------------------------------------------------------------------
//  CLASS: TContentTab
// PREFIX: wnd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TContentTab: Constructors and destructor
// ---------------------------------------------------------------------------
TContentTab::TContentTab(const  TString&            strPath
                        , const TString&            strRelPath
                        , const TString&            strType
                        , const tCIDLib::TBoolean   bEditMode) :

    TTabWindow
    (
        (bEditMode ? kCQCAdmin::strTabPref_Edit : kCQCAdmin::strTabPref_View)
        + strPath
        , strType
        , kCIDLib::True
        , kCIDLib::False
    )
    , m_bEditMode(bEditMode)
    , m_fcolStatusSlots(1UL)
    , m_strPath(strPath)
    , m_strRelPath(strRelPath)
{
    // Just add a bogus slot that won't ever get used
    m_fcolStatusSlots.c4AddElement(12);

    // Add empty storage values for each indicated status slot
    const tCIDLib::TCard4 c4Count = m_fcolStatusSlots.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        m_colStatusSlots.objAdd(TString::strEmpty());
}

TContentTab::~TContentTab()
{
}


// ---------------------------------------------------------------------------
//  TContentTab: Public, virtual methods
// ---------------------------------------------------------------------------

// If not overridden, we just return the path
const TString& TContentTab::strContName() const
{
    return m_strPath;
}


// ---------------------------------------------------------------------------
//  TContentTab: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TContentTab::bEditMode() const
{
    return m_bEditMode;
}


//
//  We call a protected virtual and update our tab state based on that and return its
//  result.
//
tCIDLib::ESaveRes
TContentTab::eSaveChanges(          TString&                strErr
                            , const tCQCAdmin::ESaveModes   eMode
                            ,       tCIDLib::TBoolean&      bChangesSaved)
{
    tCIDLib::ESaveRes eRes = eDoSave(strErr, eMode, bChangesSaved);

    if (eRes == tCIDLib::ESaveRes::NoChanges)
        SetState(tCIDCtrls::ETabStates::Normal, kCIDLib::True);
    else if (eRes == tCIDLib::ESaveRes::Errors)
        SetState(tCIDCtrls::ETabStates::Errors, kCIDLib::True);
    else if (eRes == tCIDLib::ESaveRes::OK)
        SetState(tCIDCtrls::ETabStates::Changes, kCIDLib::True);

    return eRes;
}


const TString& TContentTab::strPath() const
{
    return m_strPath;
}


const TString& TContentTab::strRelPath() const
{
    return m_strRelPath;
}


tCIDLib::TVoid
TContentTab::SetStatusAreas(const tCIDLib::TCardList& fcolStatusSlots)
{
    m_fcolStatusSlots = fcolStatusSlots;

    // If zero, then make it one bogus slot that just won't get used
    if (m_fcolStatusSlots.bIsEmpty())
        m_fcolStatusSlots.c4AddElement(128);

    // Add empty storage values for each indicated status slot
    m_colStatusSlots.RemoveAll();
    const tCIDLib::TCard4 c4Count = m_fcolStatusSlots.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        m_colStatusSlots.objAdd(TString::strEmpty());
}


//
//  When we get activated, we get called here to update the status bar for our
//  needs. We assume the slot sizes are pixel sizes.
//
tCIDLib::TVoid TContentTab::UpdateStatusBar(TMainFrameWnd& wndTarget)
{
    wndTarget.SetStatusBarAreas(m_fcolStatusSlots, kCIDLib::False);

    // Now update the slot content
    const tCIDLib::TCard4 c4Count = m_fcolStatusSlots.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        wndTarget.SetStatusBarText(c4Index, m_colStatusSlots[c4Index]);
}


// ---------------------------------------------------------------------------
//  TContentTab: Protected, virtaul methods
// ---------------------------------------------------------------------------

// If not overridden, just say no changes
tCIDLib::ESaveRes
TContentTab::eDoSave(TString&, const tCQCAdmin::ESaveModes, tCIDLib::TBoolean& bChangesSaved)
{
    bChangesSaved = kCIDLib::False;
    return tCIDLib::ESaveRes::NoChanges;
}



// ---------------------------------------------------------------------------
//  TContentTab: Protected, non-virtual methods
// ---------------------------------------------------------------------------

// A convenience to just clear all of our slots
tCIDLib::TVoid TContentTab::ClearAllStatusSlots()
{
    const tCIDLib::TCard4 c4Count = m_colStatusSlots.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        SetStatusSlot(c4Index, TString::strEmpty());
}


// The derived tab code calls this to set status slots
tCIDLib::TVoid
TContentTab::SetStatusSlot( const   tCIDLib::TCard4 c4SlotInd
                            , const TString&        strValue)
{
    // Store the value first
    m_colStatusSlots[c4SlotInd] = strValue;

    // If we are the active tab, then set the real status bar slot
    TTabbedWnd* pwndTabs = static_cast<TTabbedWnd*>(pwndParent());
    if (pwndTabs->bIsActiveTab(*this))
        facCQCAdmin.wndMain().SetStatusBarText(c4SlotInd, strValue);
}

