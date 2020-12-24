//
// FILE NAME: CQCInst_PrereqPanel.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/02/2017
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
//  This file implements the info panel that lets the user install prerequisite
//  software.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCInst.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TInstPrereqPanel,TInstInfoPanel)


// ---------------------------------------------------------------------------
//   CLASS: TInstPrereqPanel::TPRItem
//  PREFIX: item
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TInstPrereqPanel::TPRItem: Constructors and Destructor
// ---------------------------------------------------------------------------
TInstPrereqPanel::TPRItem::TPRItem() :

    m_eType(EType_URL)
{
}

TInstPrereqPanel::TPRItem::TPRItem( const   ETypes          eType
                                    , const tCIDLib::TMsgId midDescr
                                    , const tCIDLib::TMsgId midTitle
                                    , const tCIDLib::TCh*   pszTarget) :
    m_eType(eType)
    , m_strTarget(pszTarget)
{
    // Load the two displayed strings
    m_strDescr.LoadFromMsg(midDescr, facCQCInst);
    m_strTitle.LoadFromMsg(midTitle, facCQCInst);
}

TInstPrereqPanel::TPRItem::~TPRItem()
{
}




// ---------------------------------------------------------------------------
//   CLASS: TInstPrereqPanel
//  PREFIX: pan
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TInstPrereqPanel: Constructors and Destructor
// ---------------------------------------------------------------------------
TInstPrereqPanel::TInstPrereqPanel( TCQCInstallInfo* const  pinfoCur
                                    , TMainFrameWnd* const  pwndParent) :

    TInstInfoPanel(L"Prereq", pinfoCur, pwndParent)
    , m_pwndInstall(nullptr)
    , m_pwndList(nullptr)
    , m_pwndListText(nullptr)
{
    // Kinect 2.0 runtime
    {
        TPRItem itemKinectRT
        (
            EType_URL
            , kCQCInstMsgs::midPReq_Descr_KinectRT
            , kCQCInstMsgs::midPReq_Title_KinectRT
            , L"https://www.microsoft.com/en-us/download/details.aspx?id=44559"
        );
        m_colItems.objAdd(itemKinectRT);
    }


    // Add the speech platform runtimes
    {
        TPRItem itemSpPlatRT
        (
            EType_URL
            , kCQCInstMsgs::midPReq_Descr_SpeechPlat
            , kCQCInstMsgs::midPReq_Title_SpeechPlat
            , L"https://www.microsoft.com/en-us/download/details.aspx?id=27225"
        );
        m_colItems.objAdd(itemSpPlatRT);
    }

    // Add the speech platform languages
    {
        TPRItem itemSPLangs
        (
            EType_URL
            , kCQCInstMsgs::midPReq_Descr_SPLanguages
            , kCQCInstMsgs::midPReq_Title_SPLanguages
            , L"https://www.microsoft.com/en-us/download/details.aspx?id=27224"
        );
        m_colItems.objAdd(itemSPLangs);
    }
}

TInstPrereqPanel::~TInstPrereqPanel()
{
}


// ---------------------------------------------------------------------------
//  TInstPrereqPanel: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TInstPrereqPanel::bValidate(tCIDCtrls::TWndId& widFailed, TString& strErrText)
{
    // Noting to do
    return kCIDLib::True;
}


tCIDLib::TVoid TInstPrereqPanel::SaveContents()
{
    // Nothing to do
}


// ---------------------------------------------------------------------------
//  TInstPrereqPanel: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TInstPrereqPanel::bCreated()
{
    TParent::bCreated();

    // Set our text for use by any popups
    strWndText(facCQCInst.strMsg(kCQCInstMsgs::midPan_Prereq_Title));

    // Load up our child controls
    LoadDlgItems(kCQCInst::ridPan_Prereq);

    // Get some pointers to the ones we want to interact with
    CastChildWnd(*this, kCQCInst::ridPan_Prereq_Install, m_pwndInstall);
    CastChildWnd(*this, kCQCInst::ridPan_Prereq_List, m_pwndList);
    CastChildWnd(*this, kCQCInst::ridPan_Prereq_ListText, m_pwndListText);

    // Set event handlers
    m_pwndList->pnothRegisterHandler(this,&TInstPrereqPanel::eListHandler);
    m_pwndInstall->pnothRegisterHandler(this,&TInstPrereqPanel::eClickHandler);

    // Set up the columns on the list box
    tCIDLib::TStrList colCols(1);
    colCols.objAdd(facCQCInst.strMsg(kCQCInstMsgs::midPan_Prereq_List));
    m_pwndList->SetColumns(colCols);

    // Load the list of prereqs
    const tCIDLib::TCard4 c4Count = m_colItems.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        colCols[0] = m_colItems[c4Index].m_strTitle;
        m_pwndList->c4AddItem(colCols, c4Index);
    }

    // Select the 0th one
    m_pwndList->SelectByIndex(0);

    return kCIDLib::True;
}


// ---------------------------------------------------------------------------
//  TInstPrereqPanel: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDCtrls::EEvResponses TInstPrereqPanel::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCInst::ridPan_Prereq_Install)
    {
        // Get the selected item and invoke it
        const tCIDLib::TCard4 c4Index = m_pwndList->c4CurItem();
        if (c4Index != kCIDLib::c4MaxCard)
            InvokeItem(m_colItems[c4Index]);
    }

    return tCIDCtrls::EEvResponses::Handled;
}


tCIDCtrls::EEvResponses TInstPrereqPanel::eListHandler(TListChangeInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCInst::ridPan_Prereq_List)
    {
        if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::Invoked)
        {
            InvokeItem(m_colItems[wnotEvent.c4Index()]);
        }
         else if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::SelChanged)
        {
            // Get the text for this one and display it
            m_pwndListText->strWndText(m_colItems[wnotEvent.c4Index()].m_strDescr);
        }
    }

    return tCIDCtrls::EEvResponses::Handled;
}


// Called when the user invokes an item via double click or Install button
tCIDLib::TVoid TInstPrereqPanel::InvokeItem(const TPRItem& itemInvoke)
{
    if (itemInvoke.m_eType == EType_URL)
    {
        TGUIShell::InvokeDefBrowser(*this, itemInvoke.m_strTarget);
    }
     else if (itemInvoke.m_eType == EType_Installer)
    {
        TErrBox msgbErr(L"This type of prequistite is not supported yet");
    }
}
