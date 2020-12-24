//
// FILE NAME: CQCAdmin_IntfEdTab.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/02/2015
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
//  This file implements a tab for editing interface templates.
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
#include    "CQCAdmin_IntfEdTab.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TIntfEdTab, TContentTab)



// ---------------------------------------------------------------------------
//  Local types and constants
// ---------------------------------------------------------------------------
namespace CQCAdmin_IntfEdTab
{
    namespace
    {
        //
        //  The indices of the status bar slots we use. The path one is just set when the
        //  tab is opened and shows the full path. The others change during editing.
        //
        constexpr tCIDLib::TCard4 c4SlotInd_Id = 0;
        constexpr tCIDLib::TCard4 c4SlotInd_Area = 1;
        constexpr tCIDLib::TCard4 c4SlotInd_Type = 2;
        constexpr tCIDLib::TCard4 c4SlotInd_Path = 3;
    }
};



// ---------------------------------------------------------------------------
//  CLASS: TIntfEdTab
// PREFIX: wnd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TIntfEdTab: Constructors and destructor
// ---------------------------------------------------------------------------

//
//  Indicate edit mode in our call to our parent class, since we only support edit
//  mode in this tab type.
//
TIntfEdTab::TIntfEdTab(const TString& strPath, const TString& strRelPath) :

    TContentTab(strPath, strRelPath, L"Template", kCIDLib::True)
    , m_c8LastMouseOver(0)
    , m_pwndEditor(nullptr)
{
}

TIntfEdTab::~TIntfEdTab()
{
}


// ---------------------------------------------------------------------------
//  TIntfEdTab: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TIntfEdTab::bProcessAccel(const tCIDLib::TVoid* const pMsgData) const
{
    // Pass it on to the editor window
    return m_pwndEditor->bProcessAccel(pMsgData);
}


// ---------------------------------------------------------------------------
//  TIntfEdTab: Public, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid TIntfEdTab::CreateTab(TTabbedWnd& wndParent, const TString& strTabText)
{
    wndParent.c4CreateTab(this, strTabText, 0, kCIDLib::True);

    //
    //  Create the editor window. We give it the relative path since that's all he
    //  needs to know. We let him use our polling engine.
    //
    m_pwndEditor = new TIntfEdMainWnd();
    m_pwndEditor->CreateIEWnd
    (
        *this
        , facCQCAdmin.cuctxToUse()
        , kCIDCtrls::widFirstCtrl
        , strRelPath()
    );

    // Set its initial size
    m_pwndEditor->SetSize(areaClient().szArea(), kCIDLib::True);

    // Set the path slot in the status bar, whic we just set and forget
    SetStatusSlot(CQCAdmin_IntfEdTab::c4SlotInd_Path, strRelPath());

    // Subcribe to the editor window's update topic
    SubscribeToTopic(m_pwndEditor->strEdUpdateTopic(), kCIDLib::False);
}


// ---------------------------------------------------------------------------
//  TIntfEdTab: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TIntfEdTab::AreaChanged(const   TArea&                  areaPrev
                        , const TArea&                  areaNew
                        , const tCIDCtrls::EPosStates   ePosState
                        , const tCIDLib::TBoolean       bOrgChanged
                        , const tCIDLib::TBoolean       bSizeChanged
                        , const tCIDLib::TBoolean       bStateChanged)
{
    // Call our parent first
    TParent::AreaChanged(areaPrev, areaNew, ePosState, bOrgChanged, bSizeChanged, bStateChanged);

    // Keep our editor window to fit
    if ((ePosState != tCIDCtrls::EPosStates::Minimized) && bSizeChanged && m_pwndEditor)
        m_pwndEditor->SetSize(areaClient().szArea(), kCIDLib::True);
}


tCIDLib::TBoolean TIntfEdTab::bCreated()
{
    TParent::bCreated();

    // Set up our status bar slots
    tCIDLib::TCardList fcolSlots(4UL);
    fcolSlots.c4AddElement(256);
    fcolSlots.c4AddElement(92);
    fcolSlots.c4AddElement(256);
    fcolSlots.c4AddElement(0);
    SetStatusAreas(fcolSlots);

    return kCIDLib::True;
}


tCIDLib::TVoid TIntfEdTab::Destroyed()
{
    // Delete the IDE window if we created it.
    if (m_pwndEditor)
    {
        try
        {
            m_pwndEditor->Destroy();
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        delete m_pwndEditor;
        m_pwndEditor = nullptr;
    }

    TParent::Destroyed();
}


tCIDLib::ESaveRes
TIntfEdTab::eDoSave(        TString&                strErrMsg
                    , const tCQCAdmin::ESaveModes   eMode
                    ,       tCIDLib::TBoolean&      bChangesSaved)
{
    const tCIDLib::TBoolean bChanges = m_pwndEditor->bChanges();
    if (eMode == tCQCAdmin::ESaveModes::Test)
    {
        if (bChanges)
            return tCIDLib::ESaveRes::OK;
        return tCIDLib::ESaveRes::NoChanges;
    }

    if (bChanges)
    {
        if (!m_pwndEditor->bSaveChanges(strErrMsg))
            return tCIDLib::ESaveRes::Errors;

        bChangesSaved = kCIDLib::True;
    }

    // No changes anymore since we saved any that existed
    return tCIDLib::ESaveRes::NoChanges;
}



//
//  This method is for synchronous subscriptions, so we get called immediately upon
//  change and shouldn't do anything to change whatever changed or we could get into
//  a circular freakout.
//
tCIDLib::TVoid TIntfEdTab::ProcessPubMsg(const TPubSubMsg& psmsgIn)
{
    try
    {
        // Based on the event, let's update our list
        const TIntfEdUpdateTopic& ieutCur = psmsgIn.objMsgAs<TIntfEdUpdateTopic>();

        // If a widget changed event, and it's the current mouse over widget
        if (ieutCur.m_eType == TIntfEdUpdateTopic::ETypes::Changed)
        {
            if (ieutCur.m_c8DesId == m_c8LastMouseOver)
            {
                SetStatusSlot(CQCAdmin_IntfEdTab::c4SlotInd_Id, ieutCur.m_strWidgetId);
                TString strArea(ieutCur.m_areaPos);
                SetStatusSlot(CQCAdmin_IntfEdTab::c4SlotInd_Area, strArea);
            }
        }
         else if (ieutCur.m_eType == TIntfEdUpdateTopic::ETypes::Enter)
        {
            // Update the status bar slots
            m_c8LastMouseOver = ieutCur.m_c8DesId;
            SetStatusSlot(CQCAdmin_IntfEdTab::c4SlotInd_Id, ieutCur.m_strWidgetId);
            TString strArea(ieutCur.m_areaPos);
            SetStatusSlot(CQCAdmin_IntfEdTab::c4SlotInd_Area, strArea);
            SetStatusSlot(CQCAdmin_IntfEdTab::c4SlotInd_Type, ieutCur.m_strWidgetType);
        }
         else if (ieutCur.m_eType == TIntfEdUpdateTopic::ETypes::Exit)
        {
            // Clear the status bar and the last mouse over
            m_c8LastMouseOver = 0;
            ClearAllStatusSlots();
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);
    }
}

