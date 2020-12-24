//
// FILE NAME: CQCAdmin_GlobalActTab.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 12/06/2015
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
//  This file implements a tab for editing global actions
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCAdmin.hpp"
#include    "CQCAdmin_GlobalActTab.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TGlobalActTab, TContentTab)



// ---------------------------------------------------------------------------
//  CLASS: TGlobalActTab
// PREFIX: wnd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TGlobalActTab: Constructors and destructor
// ---------------------------------------------------------------------------

//
//  Indicate edit mode in our call to our parent class, since we only support edit
//  mode in this tab type.
//
TGlobalActTab::TGlobalActTab(const TString& strPath, const TString& strRelPath) :

    TContentTab(strPath, strRelPath, L"Global Action", kCIDLib::True)
    , m_pwndActText(nullptr)
    , m_pwndEdit(nullptr)
    , m_pwndTest(nullptr)
    , m_pwndTitle(nullptr)
{
}

TGlobalActTab::~TGlobalActTab()
{
}


// ---------------------------------------------------------------------------
//  TGlobalActTab: Public, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid
TGlobalActTab::CreateTab(       TTabbedWnd&     wndParent
                        , const TString&        strTabText
                        , const TCQCStdCmdSrc&  csrcEdit)
{
    // Store away the action, and a second copy for comparisons later
    m_csrcEdit = csrcEdit;
    m_csrcOrg = csrcEdit;

    wndParent.c4CreateTab(this, strTabText, 0, kCIDLib::True);
}


// ---------------------------------------------------------------------------
//  TGlobalActTab: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TGlobalActTab::AreaChanged( const   TArea&                  areaPrev
                            , const TArea&                  areaNew
                            , const tCIDCtrls::EPosStates   ePosState
                            , const tCIDLib::TBoolean       bOrgChanged
                            , const tCIDLib::TBoolean       bSizeChanged
                            , const tCIDLib::TBoolean       bStateChanged)
{
    // Call our parent first
    TParent::AreaChanged(areaPrev, areaNew, ePosState, bOrgChanged, bSizeChanged, bStateChanged);

    //  If the size changed, and we have children and not minimized, then resize it.
    if (bSizeChanged && m_pwndTitle && (ePosState != tCIDCtrls::EPosStates::Minimized))
        AutoAdjustChildren(areaPrev, areaNew);
}


tCIDLib::TBoolean TGlobalActTab::bCreated()
{
    TParent::bCreated();

    // Get the dialog description we'll use to create the controls.
    TDlgDesc dlgdChildren;
    facCQCAdmin.bCreateDlgDesc(kCQCAdmin::ridTab_GlobAct, dlgdChildren);

    // Create the children as children of the client
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

    // Get pointers to our handful of controls and load those that show data
    CastChildWnd(*this, kCQCAdmin::ridTab_GlobAct_ActText, m_pwndActText);
    CastChildWnd(*this, kCQCAdmin::ridTab_GlobAct_Edit, m_pwndEdit);
    CastChildWnd(*this, kCQCAdmin::ridTab_GlobAct_Test, m_pwndTest);
    CastChildWnd(*this, kCQCAdmin::ridTab_GlobAct_Title, m_pwndTitle);

    //
    //  We only have one action event, so format the opcodes for that event out and
    //  display them in the action text window.
    //
    TTextStringOutStream strmFmt(2048UL);
    const TCQCActEvInfo& caeiShow = m_csrcEdit.caeiEventAt(0);
    m_csrcEdit.FormatOpsForEvent(caeiShow.m_strEventId, strmFmt, 1);
    strmFmt.Flush();
    m_pwndActText->SetText(strmFmt.strData());

    // Load the title text
    m_pwndTitle->strWndText(m_csrcEdit.strTitle());

    // And set up our button click handler for the edit button
    m_pwndEdit->pnothRegisterHandler(this, &TGlobalActTab::eClickHandler);
    m_pwndTest->pnothRegisterHandler(this, &TGlobalActTab::eClickHandler);

    return kCIDLib::True;
}


tCIDLib::TVoid TGlobalActTab::Destroyed()
{
    // Clean up the action trace if it exists
    if (m_wndTrace.bIsValid())
        m_wndTrace.Destroy();

    TParent::Destroyed();
}



// ---------------------------------------------------------------------------
//  TGlobalActTab: Private, non-virtual methods
// ---------------------------------------------------------------------------

// There's only one button, so we don't have to check the source id
tCIDCtrls::EEvResponses TGlobalActTab::eClickHandler(TButtClickInfo& wnotClick)
{
    if (wnotClick.widSource() == kCQCAdmin::ridTab_GlobAct_Edit)
    {
        const tCIDLib::TBoolean bRes = facCQCIGKit().bEditAction
        (
            *this, m_csrcEdit, kCIDLib::True, facCQCAdmin.cuctxToUse(), nullptr
        );

        if (bRes)
        {
            // They saved changes, so update the actin preview test
            TTextStringOutStream strmFmt(2048UL);
            const TCQCActEvInfo& caeiShow = m_csrcEdit.caeiEventAt(0);
            m_csrcEdit.FormatOpsForEvent(caeiShow.m_strEventId, strmFmt, 1);
            strmFmt.Flush();
            m_pwndActText->SetText(strmFmt.strData());
        }
    }
     else if (wnotClick.widSource() == kCQCAdmin::ridTab_GlobAct_Test)
    {
        // Invoke the action trace modally
        TWndPtrJanitor janPtr(tCIDCtrls::ESysPtrs::Wait);
        /*
        if (m_wndTrace.bIsValid())
        {
            m_wndTrace.ToTop(kCIDLib::True);
        }
         else
        {
            TString strTitle
            (
                kCQCAMsgs::midTab_GlobAct_TestTitle, facCQCAdmin, m_pwndTitle->strWndText()
            );
            m_wndTrace.CreateWindow(strTitle);
        }
        */

        TString strTitle
        (
            kCQCAMsgs::midTab_GlobAct_TestTitle, facCQCAdmin, m_pwndTitle->strWndText()
        );
        TActTraceWnd wndTrace;
        wndTrace.RunModally
        (
            *this
            , m_csrcEdit
            , facCQCAdmin.cuctxToUse()
            , strTitle
            , kCQCKit::strEvId_OnTrigger
            , kCIDLib::True
            , L"AdminIntf"
        );
    }
   return tCIDCtrls::EEvResponses::Handled;
}


tCIDLib::ESaveRes
TGlobalActTab::eDoSave(         TString&                strErrMsg
                        , const tCQCAdmin::ESaveModes   eMode
                        ,       tCIDLib::TBoolean&      bChangesSaved)
{
    // Make sure it's not empty
    if (!m_csrcEdit.bHasAnyOps())
    {
        strErrMsg = facCQCKit().strMsg(kKitErrs::errcCmd_NoActCommands);
        return tCIDLib::ESaveRes::Errors;
    }

    // Store the title in case it's changed
    m_csrcEdit.strTitle(m_pwndTitle->strWndText());

    if (eMode == tCQCAdmin::ESaveModes::Test)
    {
        if (m_csrcEdit == m_csrcOrg)
            return tCIDLib::ESaveRes::NoChanges;
       return tCIDLib::ESaveRes::OK;
    }

    // There are changes, so write our current edit contents out if allowed
    try
    {
        TDataSrvClient dsclToUse;

        // The id member is used to store the path
        tCIDLib::TCard4 c4SerialNum = 0;
        tCIDLib::TEncodedTime enctLastChange;
        tCIDLib::TKVPFList colXMeta;
        dsclToUse.WriteGlobalAction
        (
            strRelPath()
            , c4SerialNum
            , enctLastChange
            , m_csrcEdit
            , kCQCRemBrws::c4Flag_OverwriteOK
            , colXMeta
            , facCQCAdmin.sectUser()
        );

        //
        //  Now copy our current edit stuff to the original, since we want to show
        //  no changes again.
        //
        m_csrcOrg = m_csrcEdit;
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

