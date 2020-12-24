//
// FILE NAME: CQCAdmin_EvMonTab.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 12/10/2015
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
//  This file implements a tab for editing event monitors
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCAdmin.hpp"
#include    "CQCAdmin_EvMonTab.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TEvMonTab, TContentTab)



// ---------------------------------------------------------------------------
//  CLASS: TEvMonTab
// PREFIX: wnd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TEvMonTab: Constructors and destructor
// ---------------------------------------------------------------------------

//
//  Indicate edit mode in our call to our parent class, since we only support edit
//  mode in this tab type.
//
TEvMonTab::TEvMonTab(const TString& strPath, const TString& strRelPath) :

    TContentTab(strPath, strRelPath, L"Event Monitor", kCIDLib::True)
    , m_pwndClass(nullptr)
    , m_pwndDescr(nullptr)
    , m_pwndParams(nullptr)
    , m_pwndSelButton(nullptr)
{
}

TEvMonTab::~TEvMonTab()
{
}


// ---------------------------------------------------------------------------
//  TEvMonTab: Public, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid
TEvMonTab::CreateTab(       TTabbedWnd&     wndParent
                    , const TString&        strTabText
                    , const TCQCEvMonCfg&   emoncEdit)
{
    // Store away the data, and a second copy for comparisons later
    m_emoncEdit = emoncEdit;
    m_emoncOrg = emoncEdit;

    wndParent.c4CreateTab(this, strTabText, 0, kCIDLib::True);
}


tCIDLib::TVoid TEvMonTab::UpdatePauseState(const tCIDLib::TBoolean bToSet)
{
    m_emoncEdit.bPaused(bToSet);
}




// ---------------------------------------------------------------------------
//  TEvMonTab: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TEvMonTab::AreaChanged( const   TArea&                  areaPrev
                        , const TArea&                  areaNew
                        , const tCIDCtrls::EPosStates   ePosState
                        , const tCIDLib::TBoolean       bOrgChanged
                        , const tCIDLib::TBoolean       bSizeChanged
                        , const tCIDLib::TBoolean       bStateChanged)
{
    // Call our parent first
    TParent::AreaChanged(areaPrev, areaNew, ePosState, bOrgChanged, bSizeChanged, bStateChanged);

    // If the size changed, and we have the client created, then resize it.
    if (bSizeChanged && m_pwndSelButton && (ePosState != tCIDCtrls::EPosStates::Minimized))
        AutoAdjustChildren(areaPrev, areaNew);
}


tCIDLib::TBoolean TEvMonTab::bCreated()
{
    TParent::bCreated();

    // Get the dialog description we'll use to create the controls
    TDlgDesc dlgdChildren;
    facCQCAdmin.bCreateDlgDesc(kCQCAdmin::ridTab_EdEvMon, dlgdChildren);

    // Create the children
    tCIDLib::TCard4 c4InitFocus;
    CreateDlgItems(dlgdChildren, c4InitFocus);

    // Do an initial auto-adjust to get them sized to fit us
    AutoAdjustChildren(dlgdChildren.areaPos(), areaClient());

    //
    //  Set us to use the standard window background. The dialog description we
    //  used to create the content was set to the main window theme, so all of the
    //  widgets should be fine already.
    //
    SetBgnColor(facCIDCtrls().rgbSysClr(tCIDCtrls::ESysColors::Window));

    // Get pointers to our controls we need to interact with
    CastChildWnd(*this, kCQCAdmin::ridTab_EdEvMon_Class, m_pwndClass);
    CastChildWnd(*this, kCQCAdmin::ridTab_EdEvMon_Descr, m_pwndDescr);
    CastChildWnd(*this, kCQCAdmin::ridTab_EdEvMon_Params, m_pwndParams);
    CastChildWnd(*this, kCQCAdmin::ridTab_EdEvMon_SelClass, m_pwndSelButton);

    // Load the incoming info
    m_pwndClass->strWndText(m_emoncEdit.strClassPath());
    m_pwndDescr->strWndText(m_emoncEdit.strDescr());
    m_pwndParams->strWndText(m_emoncEdit.strParams());

    // Set up event handlers
    m_pwndSelButton->pnothRegisterHandler(this, &TEvMonTab::eClickHandler);

    return kCIDLib::True;
}


// Check for changes, saving them if requested
tCIDLib::ESaveRes
TEvMonTab::eDoSave(         TString&                strErrMsg
                    , const tCQCAdmin::ESaveModes   eMode
                    ,       tCIDLib::TBoolean&      bChangesSaved)
{
    // Make sure that we have a description field
    if (m_pwndDescr->bIsEmpty())
    {
        strErrMsg = facCQCAdmin.strMsg(kCQCAErrs::errcEv_NoDescr);
        return tCIDLib::ESaveRes::Errors;
    }

    // And that a class was selected
    if (m_pwndClass->bIsEmpty())
    {
        strErrMsg = facCQCAdmin.strMsg(kCQCAErrs::errcEv_NoClass);
        return tCIDLib::ESaveRes::Errors;
    }

    // Make sure that the class exists
    const TString& strCMLRelPath = m_pwndClass->strWndText();
    try
    {
        TDataSrvClient dsclToUse;
        if (!dsclToUse.bFileExists(strCMLRelPath, tCQCRemBrws::EDTypes::Macro))
        {
            strErrMsg = facCQCAdmin.strMsg(kCQCAErrs::errcEv_ClassNotFnd);
            return tCIDLib::ESaveRes::Errors;
        }
    }

    catch(TError& errToCatch)
    {
        strErrMsg = errToCatch.strErrText();
        return tCIDLib::ESaveRes::Errors;
    }

    // Looks ok, so store the data away in our configuration object
    m_emoncEdit.Set
    (
        strCMLRelPath, m_pwndDescr->strWndText(), m_pwndParams->strWndText()
    );

    if (eMode == tCQCAdmin::ESaveModes::Test)
    {
        if (m_emoncEdit == m_emoncOrg)
            return tCIDLib::ESaveRes::NoChanges;
        return tCIDLib::ESaveRes::OK;
    }

    // There are changes, so write our current edit contents out if allowed
    try
    {
        TWndPtrJanitor janPtr(tCIDCtrls::ESysPtrs::Wait);

        TDataSrvClient dsclToUse;

        tCIDLib::TCard4 c4SerialNum = 0;
        tCIDLib::TEncodedTime enctLastChange;
        tCIDLib::TKVPFList  colXMeta;
        dsclToUse.WriteEventMon
        (
            strRelPath()
            , c4SerialNum
            , enctLastChange
            , m_emoncEdit
            , kCQCRemBrws::c4Flag_OverwriteOK
            , colXMeta
            , facCQCAdmin.sectUser()
        );

        //
        //  Now copy our current edit stuff to the original, since we want to show
        //  no changes again.
        //
        m_emoncOrg = m_emoncEdit;
        bChangesSaved = kCIDLib::True;
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        strErrMsg = errToCatch.strErrText();
        return tCIDLib::ESaveRes::Errors;
    }

    // No changes anymore
    return tCIDLib::ESaveRes::NoChanges;
}


// ---------------------------------------------------------------------------
//  TEvMonTab: Private, non-virtual methods
// ---------------------------------------------------------------------------

tCIDCtrls::EEvResponses TEvMonTab::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCAdmin::ridTab_EdEvMon_SelClass)
    {
        TString strSel;
        const tCIDLib::TBoolean bRes = facCQCTreeBrws().bSelectFile
        (
            *this
            , L"Select Handler Class"
            , tCQCRemBrws::EDTypes::Macro
            , m_emoncEdit.strClassPath()
            , facCQCAdmin.cuctxToUse()
            , tCQCTreeBrws::EFSelFlags::SelectItems
            , strSel
        );

        // If they made a selection, load it to the entry field
        if (bRes)
            m_pwndClass->strWndText(strSel);
    }
    return tCIDCtrls::EEvResponses::Handled;
}




