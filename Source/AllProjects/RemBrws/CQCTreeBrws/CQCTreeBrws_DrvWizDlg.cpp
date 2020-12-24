//
// FILE NAME: CQCIGKit_DrvWizDlg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/11/2015
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
//  This file implements the driver configuration wizard.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCTreeBrws_.hpp"
#include    "CQCTreeBrws_DrvWizDlg_Panels_.hpp"
#include    "CQCTreeBrws_DrvWizDlg_.hpp"



// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCDrvWizDlg, TDlgBox)



// ---------------------------------------------------------------------------
//   CLASS: TCQCDrvWizDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCDrvWizDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCDrvWizDlg::TCQCDrvWizDlg(const  TString&            strHost
                            , const tCIDLib::TBoolean   bReconfigMode
                            , const TCQCSecToken&       sectUser) :

    m_bAllowTabMode(kCIDLib::False)
    , m_bReconfigMode(bReconfigMode)
    , m_pcolMonikers(nullptr)
    , m_pwndCancelButton(nullptr)
    , m_pwndNextButton(nullptr)
    , m_pwndPrevButton(nullptr)
    , m_pwndTabs(nullptr)
    , m_sectUser(sectUser)
    , m_strHost(strHost)
{
}

TCQCDrvWizDlg::~TCQCDrvWizDlg()
{
}


// ---------------------------------------------------------------------------
//  TCQCDrvWizDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TCQCDrvWizDlg::bRun(const   TWindow&                    wndOwner
                    ,       TCQCDriverObjCfg&           cqcdcEdit
                    , const tCIDLib::TStrList&          colMonikers
                    , const tCIDLib::TStrList* const    pcolPorts)
{

    // If we got ports, copy them. Else query local ports
    if (pcolPorts)
        m_colPorts = *pcolPorts;
    else
        facCIDComm().bQueryPorts(m_colPorts, kCIDLib::False, kCIDLib::True);

    //
    //  Save a pointer to the current monikers list, and a copy of the driver config
    //  data for us to work on.
    //
    m_cqcdcEdit = cqcdcEdit;
    m_pcolMonikers = &colMonikers;

    const tCIDLib::TCard4 c4Res = c4RunDlg
    (
        wndOwner, facCQCTreeBrws(), kCQCTreeBrws::ridDlg_DrvWiz
    );

    //
    //  When we finally get to the end without errors, the Next button gets changed
    //  to Save and will end the dialog, so we just check for that.
    //
    if (c4Res == kCQCTreeBrws::ridDlg_DrvWiz_Next)
    {
        cqcdcEdit = m_cqcdcEdit;
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


const TString& TCQCDrvWizDlg::strHost() const
{
    return m_strHost;
}


// ---------------------------------------------------------------------------
//  TCQCDrvWizDlg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TCQCDrvWizDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRet = TParent::bCreated();

    // First get typed pointers to the widget we always keep
    CastChildWnd(*this, kCQCTreeBrws::ridDlg_DrvWiz_Cancel, m_pwndCancelButton);
    CastChildWnd(*this, kCQCTreeBrws::ridDlg_DrvWiz_Next, m_pwndNextButton);
    CastChildWnd(*this, kCQCTreeBrws::ridDlg_DrvWiz_Prev, m_pwndPrevButton);
    CastChildWnd(*this, kCQCTreeBrws::ridDlg_DrvWiz_Tabs, m_pwndTabs);

    // Register our handlers
    m_pwndCancelButton->pnothRegisterHandler(this, &TCQCDrvWizDlg::eClickHandler);
    m_pwndNextButton->pnothRegisterHandler(this, &TCQCDrvWizDlg::eClickHandler);
    m_pwndPrevButton->pnothRegisterHandler(this, &TCQCDrvWizDlg::eClickHandler);
    m_pwndTabs->pnothRegisterHandler(this, &TCQCDrvWizDlg::eTabHandler);

    // Set our text so it gets picked up by popups
    strWndText(facCQCTreeBrws().strMsg(kTBrwsMsgs::midDlg_DrvWiz));

    //
    //  Override the tabbed window's background color. Our dialog is of course going
    //  to use the dailog bgn color, but we want the contents of the tabs to use
    //  regular window bgn, so we need the tabbed window to use that as well.
    //
    m_pwndTabs->SetBgnColor
    (
        facCIDCtrls().rgbSysClr(tCIDCtrls::ESysColors::Window), kCIDLib::True
    );

    //
    //  Load the appropriate tabs for the incoming driver config. Some are always
    //  loaded, and some are driven by comm config and prompts.
    //
    //  We always load the moniker panel. We tell it if we are in reconfig mode or
    //  not.
    //
    TRefVector<TDrvWizPanel> colPanels(tCIDLib::EAdoptOpts::NoAdopt);
    colPanels.Add(new TDWMonikerPanel(&m_cqcdcEdit, m_bReconfigMode, *m_pcolMonikers));

    // Depending on the comm configuration type, load up the appropriate tag
    if (m_cqcdcEdit.conncfgReal().bIsA(TCQCIPConnCfg::clsThis()))
        colPanels.Add(new TDWSockPanel(&m_cqcdcEdit));
    else if (m_cqcdcEdit.conncfgReal().bIsA(TCQCSerialConnCfg::clsThis()))
        colPanels.Add(new TDWSerialPanel(&m_cqcdcEdit));
    else if (m_cqcdcEdit.conncfgReal().clsIsA() == TCQCUPnPConnCfg::clsThis())
        colPanels.Add(new TDWUPnPCommPanel(&m_cqcdcEdit));

    //
    //  Next comes any prompts. Remember the index of the first prompt before
    //  we start adding.
    //
    m_c4FirstPrompt = colPanels.c4ElemCount();
    const tCIDLib::TCard4 c4Count = m_cqcdcEdit.c4PromptCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TCQCDCfgPrompt& cqcdcpCur = m_cqcdcEdit.cqcdcpAt(c4Index);

        switch(cqcdcpCur.eType())
        {
            case tCQCKit::EPromptTypes::AudioDev :
                colPanels.Add(new TDWAudDevPanel(&m_cqcdcEdit, c4Index));
                break;

            case tCQCKit::EPromptTypes::Bool :
                colPanels.Add(new TDWBoolPanel(&m_cqcdcEdit, c4Index));
                break;

            case tCQCKit::EPromptTypes::File :
                // Indicate file mode
                colPanels.Add(new TDWFileSelPanel(&m_cqcdcEdit, c4Index, kCIDLib::True));
                break;

            case tCQCKit::EPromptTypes::List :
                colPanels.Add(new TDWListPanel(&m_cqcdcEdit, c4Index));
                break;

            case tCQCKit::EPromptTypes::MultiList :
                colPanels.Add(new TDWMListPanel(&m_cqcdcEdit, c4Index));
                break;

            case tCQCKit::EPromptTypes::NamedMList :
                colPanels.Add(new TDWNamedListPanel(&m_cqcdcEdit, c4Index));
                break;

            case tCQCKit::EPromptTypes::ODBCSrc :
                colPanels.Add(new TDWODBCPanel(&m_cqcdcEdit, c4Index));
                break;

            case tCQCKit::EPromptTypes::Path :
                // Indicate path mode
                colPanels.Add(new TDWFileSelPanel(&m_cqcdcEdit, c4Index, kCIDLib::False));
                break;

            case tCQCKit::EPromptTypes::Range :
                colPanels.Add(new TDWRangePanel(&m_cqcdcEdit, c4Index));
                break;

            case tCQCKit::EPromptTypes::RepoDriver :
                colPanels.Add(new TDWMediaRepoPanel(&m_cqcdcEdit, c4Index));
                break;

            case tCQCKit::EPromptTypes::Text :
                colPanels.Add(new TDWTextPanel(&m_cqcdcEdit, c4Index));
                break;

            case tCQCKit::EPromptTypes::TrayMonBinding :
                colPanels.Add(new TDWACPromptPanel(&m_cqcdcEdit, c4Index));
                break;

            case tCQCKit::EPromptTypes::TTSVoice :
                colPanels.Add(new TDWTTSVoicePanel(&m_cqcdcEdit, c4Index));
                break;

            default :
                break;
        };
    }

    // Remember the index of the last prompt
    m_c4LastPrompt = colPanels.c4ElemCount()  - 1;

    // And we always end with the summary panel
    colPanels.Add(new TDWSumPanel(&m_cqcdcEdit));

    //
    //  Go through all of the tabs and create them now.
    //
    const tCIDLib::TCard4 c4TabCnt = colPanels.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4TabCnt; c4Index++)
        colPanels[c4Index]->CreatePanel(this, *m_pwndTabs, m_sectUser);

    // And now go back and initialize them
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4TabCnt; c4Index++)
        colPanels[c4Index]->Initialize(*this, m_colPorts);

    // Indicate we want to the dialog to handle auto-resizing
    AutoAdjustChildren(areaWndSize(), areaWndSize());

    return bRet;
}


// ---------------------------------------------------------------------------
//  TCQCDrvWizDlg: Private, non-virutal methods
// ---------------------------------------------------------------------------

//
//  Called to move us forward to the next tab, when the user selects the Next
//  button. We get the current tab and validate it. If it fails, we display the
//  message and stay on the current tab. Else, we move forward.
//
tCIDLib::TBoolean TCQCDrvWizDlg::bMoveForward()
{
    tCIDLib::TCard4 c4CurTabInd = m_pwndTabs->c4ActiveTabIndex();
    TDrvWizPanel* pwndCur = static_cast<TDrvWizPanel*>(m_pwndTabs->pwndTabAt(c4CurTabInd));

    // See if we are at the summary screen. If so, we are done
    if (pwndCur->eType() == TDrvWizPanel::EPanTypes::Summary)
        return kCIDLib::True;

    // Else, see if the data is good and store it away if so
    TString strErrMsg;
    tCIDCtrls::TWndId widBad = 0;
    if (!pwndCur->bValidate(*this, widBad, strErrMsg))
    {
        //
        //  They can return an empty string (because they already confirmed with the
        //  user it's OK.)
        //
        if (!strErrMsg.bIsEmpty())
        {
            TErrBox msgErr(strErrMsg);
            msgErr.ShowIt(*this);
        }

        // If we get a window id that needs to be fixed, give it the focus
        if (widBad)
            pwndCur->SetChildFocus(widBad);
        return kCIDLib::False;
    }

    // Make sure we don't deny this attempt to move
    TBoolJanitor janFlag(&m_bAllowTabMode, kCIDLib::True);

    // Move to the next tab
    c4CurTabInd++;
    m_pwndTabs->ActivateAt(c4CurTabInd);

    //
    //  If we are on the summary screen now, change the text of the Next button to
    //  say Install instead. And re-initialize the summary so it displays the latest
    //  data.
    //
    pwndCur = static_cast<TDrvWizPanel*>(m_pwndTabs->pwndTabAt(c4CurTabInd));
    if (pwndCur->eType() == TDrvWizPanel::EPanTypes::Summary)
    {
        m_pwndNextButton->strWndText
        (
            facCQCTreeBrws().strMsg(kTBrwsMsgs::midDlg_DrvWiz_Install)
        );
        pwndCur->Initialize(*this, m_colPorts);
    }

    // Not done yet, so return false
    return kCIDLib::False;
}


// Handle any clicks at the wizard level
tCIDCtrls::EEvResponses TCQCDrvWizDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCTreeBrws::ridDlg_DrvWiz_Cancel)
    {
        TYesNoBox msgbCancel(facCQCTreeBrws().strMsg(kTBrwsMsgs::midQ_CancelDrvWiz));

        if (msgbCancel.bShowIt(*this))
            EndDlg(kCQCTreeBrws::ridDlg_DrvWiz_Cancel);
    }
     else if (wnotEvent.widSource() == kCQCTreeBrws::ridDlg_DrvWiz_Next)
    {
        //
        //  Let's validate the current tab. If it works, then we move forward, else
        //  we show the error and stay here. If it returns True, then they hit next
        //  when on the Summary screen, in which case we are done.
        //
        if (bMoveForward())
            EndDlg(kCQCTreeBrws::ridDlg_DrvWiz_Next);
    }
     else if (wnotEvent.widSource() == kCQCTreeBrws::ridDlg_DrvWiz_Prev)
    {
        // If not on the 0th tab, then move back
        tCIDLib::TCard4 c4CurTabInd = m_pwndTabs->c4ActiveTabIndex();
        if (c4CurTabInd)
        {
            // Make sure we don't deny this attempt to move
            TBoolJanitor janFlag(&m_bAllowTabMode, kCIDLib::True);

            c4CurTabInd--;
            m_pwndTabs->ActivateAt(c4CurTabInd);

            // Make sure the text of the Next button is back to Next
            m_pwndNextButton->strWndText
            (
                facCQCTreeBrws().strMsg(kTBrwsMsgs::midDlg_DrvWiz_Next)
            );
        }

    }
    return tCIDCtrls::EEvResponses::Handled;
}


//
//  We prevent manual selection of tabs. We only allow them to move through the
//  tabs via the Next/Prev buttons.
//
tCIDCtrls::EEvResponses TCQCDrvWizDlg::eTabHandler(TTabEventInfo& wnotEvent)
{
    //
    //  Watch for manual tab changes and prevent them. We set our own flag when we
    //  are forcing a move, so that we know the difference.
    //
    if ((wnotEvent.eEvent() == tCIDCtrls::ETabWEvents::PreChange) && !m_bAllowTabMode)
        wnotEvent.bAllow(kCIDLib::False);

    return tCIDCtrls::EEvResponses::Handled;
}


