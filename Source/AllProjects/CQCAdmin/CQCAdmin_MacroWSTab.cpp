//
// FILE NAME: CQCAdmin_MacroWSTab.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/15/2015
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
//  This file implements a tab for read only viewing of text based content.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
//
//
// ---------------------------------------------------------------------------
#include    "CQCAdmin.hpp"
#include    "CIDMacroDbg.hpp"
#include    "CQCAdmin_MacroWSTab.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TMacroWSTab, TContentTab)


// ---------------------------------------------------------------------------
//  Local types and constants
// ---------------------------------------------------------------------------
namespace CQCAdmin_MacroWSTab
{
    namespace
    {
        // The indices of the status bar slots we use
        constexpr tCIDLib::TCard4 c4SlotInd_CurLine     = 0;
        constexpr tCIDLib::TCard4 c4SlotInd_State       = 1;
        constexpr tCIDLib::TCard4 c4SlotInd_RO_RW       = 2;
        constexpr tCIDLib::TCard4 c4SlotInd_Path        = 3;
    }
};



// ---------------------------------------------------------------------------
//  CLASS: TMacroWSTab
// PREFIX: wnd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TMacroWSTab: Constructors and destructor
// ---------------------------------------------------------------------------

//
//  Indicate edit mode in our call to our parent class, since we only support edit
//  mode in this tab type.
//
//  Set up the class manager for read only mode. We don't need the popup menu because
//  we have the main browser in the program to manage files. It's only for selection
//  for us here. That means we don't need to pass a callback handler either.
//
TMacroWSTab::TMacroWSTab(const TString& strPath, const TString& strRelPath) :

    TContentTab(strPath, strRelPath, L"CML Macro", kCIDLib::True)
    , m_mecmToUse
      (
        this, L"Select a Macro", facCQCAdmin.cuctxToUse(), kCIDLib::True, nullptr
      )
    , m_pwndIDE(nullptr)
{
}

TMacroWSTab::~TMacroWSTab()
{
}


// ---------------------------------------------------------------------------
//  TMacroWSTab: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TMacroWSTab::bProcessAccel(const tCIDLib::TVoid* const pMsgData) const
{
    // Pass it on to the IDE window
    return m_pwndIDE->bProcessAccel(pMsgData);
}


// We override this and display the path in the macro path format
const TString& TMacroWSTab::strContName() const
{
    return m_pwndIDE->strCurClassPath();
}


// ---------------------------------------------------------------------------
//  TMacroWSTab: Public, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid
TMacroWSTab::CreateTab(TTabbedWnd& wndParent, const TString& strTabText)
{
    wndParent.c4CreateTab(this, strTabText, 0, kCIDLib::True);

    //
    //  Set the macro on the IDE window now. We have to do this after we are fully
    //  created since it will cause a notification and our parent will not have
    //  seen that we are the active tab and set up the status bar for our stuff.
    //
    TString strClassPath;
    facCQCRemBrws().RelPathToCMLClassPath(strRelPath(),strClassPath);

    m_pwndIDE->bOpenSession(strClassPath, kCIDLib::False);
    m_pwndIDE->SetBgnColor(facCIDCtrls().rgbSysClr(tCIDCtrls::ESysColors::Window),kCIDLib::True);

    // Now we can show the window
    m_pwndIDE->SetVisibility(kCIDLib::True);
}


// ---------------------------------------------------------------------------
//  TMacroWSTab: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TMacroWSTab::AreaChanged(const  TArea&                  areaPrev
                        , const TArea&                  areaNew
                        , const tCIDCtrls::EPosStates   ePosState
                        , const tCIDLib::TBoolean       bOrgChanged
                        , const tCIDLib::TBoolean       bSizeChanged
                        , const tCIDLib::TBoolean       bStateChanged)
{
    // Call our parent first
    TParent::AreaChanged(areaPrev, areaNew, ePosState, bOrgChanged, bSizeChanged, bStateChanged);

    // Keep our IDE window to fit
    if ((ePosState != tCIDCtrls::EPosStates::Minimized) && bSizeChanged && m_pwndIDE)
    {
        TArea areaIDE(areaClient());
        m_pwndIDE->SetSize(areaIDE.szArea(), kCIDLib::True);
    }
}


tCIDLib::TBoolean TMacroWSTab::bCreated()
{
    TParent::bCreated();

    // Initialize our class manager and file resolver
    m_mefrToUse.strBasePath(facCQCKit().strMacroRootPath());

    //
    //  Set up our status bar slots. Set the last one to zero to give it the remaining
    //  space.
    //
    tCIDLib::TCardList fcolSlots(3UL);
    fcolSlots.c4AddElement(38);
    fcolSlots.c4AddElement(128);
    fcolSlots.c4AddElement(38);
    fcolSlots.c4AddElement(0);
    SetStatusAreas(fcolSlots);

    //
    //  Create the IDE window. It's just a standard pane window container that
    //  is overridden to provide some dbg stuff.
    //
    m_pwndIDE = new TMacroDbgMainWnd(&m_mecmToUse, &m_mefrToUse);

    TArea areaIDE(areaClient());
    m_pwndIDE->CreatePaneCont(*this, areaIDE, kCIDCtrls::widFirstCtrl);

    // Register for events from the IDE
    m_pwndIDE->pnothRegisterHandler(this, &TMacroWSTab::eDbgHandler);

    return kCIDLib::True;
}


tCIDLib::TVoid TMacroWSTab::Destroyed()
{
    // Delete the IDE window if we created it.
    if (m_pwndIDE)
    {
        try
        {
            m_pwndIDE->Destroy();
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        delete m_pwndIDE;
        m_pwndIDE = nullptr;
    }

    TParent::Destroyed();
}


tCIDLib::ESaveRes
TMacroWSTab::eDoSave(       TString&                strErrMsg
                    , const tCQCAdmin::ESaveModes   eMode
                    ,       tCIDLib::TBoolean&      bChangesSaved)
{
    if (eMode == tCQCAdmin::ESaveModes::Test)
    {
        if (!m_pwndIDE->bAnyChanges())
            return tCIDLib::ESaveRes::NoChanges;
        return tCIDLib::ESaveRes::OK;
    }

    if (!m_pwndIDE->bSaveAll(strErrMsg))
        return tCIDLib::ESaveRes::Errors;

    // No changes anymore
    bChangesSaved = kCIDLib::True;
    return tCIDLib::ESaveRes::NoChanges;
}


// ---------------------------------------------------------------------------
//  TMacroWSTab: Private, non-virtual methods
// ---------------------------------------------------------------------------

// We use this to keep the status bar updated
tCIDCtrls::EEvResponses TMacroWSTab::eDbgHandler(TCMLDbgChangeInfo& wnotEvent)
{
    if (wnotEvent.eEvent() == tCIDMacroDbg::EEvents::IDEState)
    {
        // The state is just passed in the current file member
        SetStatusSlot(CQCAdmin_MacroWSTab::c4SlotInd_State, wnotEvent.strCurFile());
    }
     else if (wnotEvent.eEvent() == tCIDMacroDbg::EEvents::LineChanged)
    {
        TString strLine;
        strLine.AppendFormatted(wnotEvent.c4LineNum());
        SetStatusSlot(CQCAdmin_MacroWSTab::c4SlotInd_CurLine, strLine);
    }
     else if (wnotEvent.eEvent() == tCIDMacroDbg::EEvents::NewFile)
    {
        SetStatusSlot(CQCAdmin_MacroWSTab::c4SlotInd_Path, wnotEvent.strCurFile());

        TString strText;
        if (wnotEvent.strCurFile().bIsEmpty())
        {
            SetStatusSlot(CQCAdmin_MacroWSTab::c4SlotInd_CurLine, strText);
            SetStatusSlot(CQCAdmin_MacroWSTab::c4SlotInd_RO_RW, strText);
        }
         else
        {
            strText.AppendFormatted(wnotEvent.c4LineNum());
            SetStatusSlot(CQCAdmin_MacroWSTab::c4SlotInd_CurLine, strText);

            if (m_pwndIDE->eCurSrcMode() == tCIDMacroEng::EResModes::ReadOnly)
                strText = L"RO";
            else
                strText = L"RW";
            SetStatusSlot(CQCAdmin_MacroWSTab::c4SlotInd_RO_RW, strText);
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}

