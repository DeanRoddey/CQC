//
// FILE NAME: CQCInst_CQCVoicePanel.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/30/2017
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
//  This file implements the info panel that lets the user set the CQCVoice tray
//  app program options.
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
RTTIDecls(TInstCQCVoicePanel,TInstInfoPanel)



// ---------------------------------------------------------------------------
//   CLASS: TInstCQCVoicePanel
//  PREFIX: pan
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TInstCQCVoicePanel: Constructors and Destructor
// ---------------------------------------------------------------------------
TInstCQCVoicePanel::TInstCQCVoicePanel( TCQCInstallInfo* const  pinfoCur
                                        , TMainFrameWnd* const  pwndParent) :

    TInstInfoPanel(L"CQC Voice Options", pinfoCur, pwndParent)
    , m_bAutoStartAvail(kCIDLib::False)
    , m_pwndAutoStart(nullptr)
    , m_pwndInstall(nullptr)
    , m_pwndPort(nullptr)
    , m_pwndPassword(nullptr)
    , m_pwndReplyStyle(nullptr)
    , m_pwndUserName(nullptr)
{
}

TInstCQCVoicePanel::~TInstCQCVoicePanel()
{
}


// ---------------------------------------------------------------------------
//  TInstCQCVoicePanel: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TInstCQCVoicePanel::bPanelIsVisible() const
{
    // If CQCVoice is being installed and not using previous opts
    return infoCur().viiNewInfo().bCQCVoice() && !infoCur().bUsePrevOpts();
}


tCIDLib::TBoolean
TInstCQCVoicePanel::bValidate(tCIDCtrls::TWndId& widFailed, TString& strErrText)
{
   const TCQCVerInstallInfo& viiNew = infoCur().viiNewInfo();

    // If not installing this component, then just say we are fine
    if (!viiNew.bCQCVoice())
        return kCIDLib::True;

    // Make sure the port is valid
    if (!wndParent().bValidatePort(*m_pwndPort, strErrText))
    {
        widFailed = m_pwndPort->widThis();
        return kCIDLib::False;
    }

    // Make sure that the user name and password are non-empty
    if (viiNew.strCQCVoiceUserName().bIsEmpty()
    ||  viiNew.strCQCVoicePassword().bIsEmpty())
    {
        strErrText.LoadFromMsg(kCQCInstErrs::errcVal_CQCVoiceUser, facCQCInst);
        widFailed = m_pwndUserName->widThis();
        return kCIDLib::False;
    }

    if (m_pwndReplyStyle->c4CurItem() == kCIDLib::c4MaxCard)
    {
        strErrText.LoadFromMsg(kCQCInstErrs::errcVal_CQCVoiceRepStyle, facCQCInst);
        widFailed = m_pwndReplyStyle->widThis();
        return kCIDLib::False;
    }

    return kCIDLib::True;
}


tCIDLib::TVoid TInstCQCVoicePanel::Entered()
{
    const TCQCVerInstallInfo& viiNew = infoCur().viiNewInfo();

    m_pwndInstall->SetCheckedState(viiNew.bCQCVoice());
    wndParent().LoadPortOpts(*this, *m_pwndPort, viiNew.ippnCQCVoice());

    m_pwndUserName->strWndText(viiNew.strCQCVoiceUserName());
    m_pwndPassword->strWndText(viiNew.strCQCVoicePassword());

    // Remember if task scheduling is available
    m_bAutoStartAvail = TKrnlTaskSched::bSupportAvail();

    // Select the reply style
    const tCIDLib::TCard4 c4Index = tCIDLib::TCard4(viiNew.eCQCVoiceRepStyle());
    if (m_pwndReplyStyle->c4SelectById(c4Index) == kCIDLib::c4MaxCard)
        m_pwndReplyStyle->SelectByIndex(0);

    // Auto-start may not be available
    if (m_bAutoStartAvail)
    {
        m_pwndAutoStart->SetEnable(kCIDLib::True);
        m_pwndAutoStart->SetCheckedState(viiNew.bCQCVoiceAutoStart());
    }
     else
    {
        m_pwndAutoStart->SetEnable(kCIDLib::False);
        m_pwndAutoStart->SetCheckedState(kCIDLib::False);
    }
}


tCIDLib::TVoid
TInstCQCVoicePanel::ReportPortUsage(tCQCInst::TPortCntList& fcolPorts)
{
    if (infoCur().viiNewInfo().bCQCVoice())
        fcolPorts[infoCur().viiNewInfo().ippnCQCVoice()]++;
}


tCIDLib::TVoid TInstCQCVoicePanel::SaveContents()
{
    TCQCVerInstallInfo& viiNew = infoCur().viiNewInfo();

    // Store the current settings away
    viiNew.bCQCVoice(m_pwndInstall->bCheckedState());
    viiNew.ippnCQCVoice(wndParent().ippnExtractPort(*m_pwndPort));

    viiNew.strCQCVoicePassword(m_pwndPassword->strWndText());
    viiNew.strCQCVoiceUserName(m_pwndUserName->strWndText());

    if (m_bAutoStartAvail)
        viiNew.bCQCVoiceAutoStart(m_pwndAutoStart->bCheckedState());
    else
        viiNew.bCQCVoiceAutoStart(kCIDLib::False);

    // Store the reply style
    const tCIDLib::TCard4 c4RepStyleInd = m_pwndReplyStyle->c4CurItem();

    // Shouldn't happen, but just in case
    if (c4RepStyleInd >= tCIDLib::c4EnumOrd(tCQCKit::EReplyStyles::Count))
    {
        viiNew.eCQCVoiceRepStyle(tCQCKit::EReplyStyles::Spoken);
    }
     else
    {
        viiNew.eCQCVoiceRepStyle
        (
            tCQCKit::EReplyStyles(m_pwndReplyStyle->c4IndexToId(c4RepStyleInd))
        );
    }
}


// ---------------------------------------------------------------------------
//  TInstCQCVoicePanel: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TInstCQCVoicePanel::bCreated()
{
    TParent::bCreated();

    // Load our controls
    LoadDlgItems(kCQCInst::ridPan_CQCVoice);

    // Get some pointers to the ones we want to interact with
    CastChildWnd(*this, kCQCInst::ridPan_CQCVoice_AutoStart, m_pwndAutoStart);
    CastChildWnd(*this, kCQCInst::ridPan_CQCVoice_DoInstall, m_pwndInstall);
    CastChildWnd(*this, kCQCInst::ridPan_CQCVoice_Password, m_pwndPassword);
    CastChildWnd(*this, kCQCInst::ridPan_CQCVoice_Port, m_pwndPort);
    CastChildWnd(*this, kCQCInst::ridPan_CQCVoice_ReplyStyle, m_pwndReplyStyle);
    CastChildWnd(*this, kCQCInst::ridPan_CQCVoice_UserName, m_pwndUserName);

    // Register a click handler for our enable/disable button
    m_pwndInstall->pnothRegisterHandler(this, &TInstCQCVoicePanel::eClickHandler);

    //  Load up the reply style possibilities, skipping the none entry.
    tCQCKit::EReplyStyles eRepStyle = tCQCKit::EReplyStyles::None;
    eRepStyle++;

    TString strFmt;
    while (eRepStyle <= tCQCKit::EReplyStyles::Max)
    {
        m_pwndReplyStyle->c4AddItem
        (
            tCQCKit::strXlatEReplyStyles(eRepStyle), tCIDLib::c4EnumOrd(eRepStyle)
        );
        eRepStyle++;
    }

    return kCIDLib::True;
}


// ---------------------------------------------------------------------------
//  TInstCQCVoicePanel: Private, non-virtual
// ---------------------------------------------------------------------------
tCIDCtrls::EEvResponses TInstCQCVoicePanel::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCInst::ridPan_CQCVoice_DoInstall)
    {
        // Enable/disable the other stuff based on the new checked state
        m_pwndAutoStart->SetEnable(wnotEvent.bState());
        m_pwndPassword->SetEnable(wnotEvent.bState());
        m_pwndPort->SetEnable(wnotEvent.bState());
        m_pwndUserName->SetEnable(wnotEvent.bState());
    }
    return tCIDCtrls::EEvResponses::Handled;
}

