//
// FILE NAME: CQCInst_MSrvPanel.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/31/2004
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
//  This file implements the info panel that lets the user select whether to
//  install the master server or not and, if not, where it is.
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
RTTIDecls(TInstMSrvPanel,TInstInfoPanel)



// ---------------------------------------------------------------------------
//   CLASS: TInstMSrvPanel
//  PREFIX: pan
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TInstMSrvPanel: Constructors and Destructor
// ---------------------------------------------------------------------------
TInstMSrvPanel::TInstMSrvPanel( TCQCInstallInfo* const  pinfoCur
                                , TMainFrameWnd* const  pwndParent) :

    TInstInfoPanel(L"MSrv Options", pinfoCur, pwndParent)
    , m_pwndDoHere(nullptr)
    , m_pwndDoThere(nullptr)
    , m_pwndMSAddr(nullptr)
{
}

TInstMSrvPanel::~TInstMSrvPanel()
{
}


// ---------------------------------------------------------------------------
//  TInstMSrvPanel: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TInstMSrvPanel::bPanelIsVisible() const
{
    //
    //  We only want to be seen if the master server is not being installed. If not, then
    //  we have to verify that we can contact the Master Server at the location indicated
    //  by the user.
    //
    return !infoCur().viiNewInfo().bMasterServer();
}


tCIDLib::TBoolean
TInstMSrvPanel::bValidate(tCIDCtrls::TWndId& widFailed, TString& strErrText)
{
    //
    //  If not installing the master server here, then the master server
    //  host address must not be empty.
    //
    if (!infoCur().viiNewInfo().bMasterServer()
    &&  infoCur().viiNewInfo().strMSAddr().bIsEmpty())
    {
        widFailed = kCQCInst::ridPan_MSrv_Addr;
        strErrText.LoadFromMsg(kCQCInstErrs::errcVal_EmptyMSAddr, facCQCInst);
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


tCIDLib::TVoid TInstMSrvPanel::Entered()
{
    if (infoCur().viiNewInfo().bMasterServer())
    {
        m_pwndDoHere->SetCheckedState(kCIDLib::True);
        m_pwndDoThere->SetCheckedState(kCIDLib::False);
        m_pwndMSAddr->SetEnable(kCIDLib::False);
    }
     else
    {
        m_pwndDoHere->SetCheckedState(kCIDLib::False);
        m_pwndDoThere->SetCheckedState(kCIDLib::True);
        m_pwndMSAddr->SetEnable(kCIDLib::True);
    }
    m_pwndMSAddr->strWndText(infoCur().viiNewInfo().strMSAddr());

    //
    //  If installing the master server, then disable the 'do there' radio button. Else
    //  disable the 'do here' radio button.
    //
    m_pwndDoHere->SetEnable(infoCur().viiNewInfo().bMasterServer());
    m_pwndDoThere->SetEnable(!infoCur().viiNewInfo().bMasterServer());
}


tCIDLib::TVoid TInstMSrvPanel::SaveContents()
{
    infoCur().viiNewInfo().bMasterServer(m_pwndDoHere->bCheckedState());
    infoCur().viiNewInfo().strMSAddr(m_pwndMSAddr->strWndText());
}


// ---------------------------------------------------------------------------
//  TInstMSrvPanel: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TInstMSrvPanel::bCreated()
{
    TParent::bCreated();

    //
    //  Load up our dialog description and adjust the originals of all the
    //  widgets to be relative to the content area. Our base class provides
    //  a helper to do this.
    //
    LoadDlgItems(kCQCInst::ridPan_MSrv);

    // Get some pointers to the ones we want to interact with
    CastChildWnd(*this, kCQCInst::ridPan_MSrv_DoHere, m_pwndDoHere);
    CastChildWnd(*this, kCQCInst::ridPan_MSrv_DoThere, m_pwndDoThere);
    CastChildWnd(*this, kCQCInst::ridPan_MSrv_Addr, m_pwndMSAddr);

    // Register a click handler for our radio buttons
    m_pwndDoHere->pnothRegisterHandler(this, &TInstMSrvPanel::eClickHandler);
    m_pwndDoThere->pnothRegisterHandler(this, &TInstMSrvPanel::eClickHandler);

    return kCIDLib::True;
}



// ---------------------------------------------------------------------------
//  TInstMSrvPanel: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDCtrls::EEvResponses TInstMSrvPanel::eClickHandler(TButtClickInfo& wnotEvent)
{
    //
    //  If if they are installing here, them force our address into it and
    //  disable it. Else, enable it so they can enter something else.
    //
    if (wnotEvent.widSource() == kCQCInst::ridPan_MSrv_DoHere)
    {
        infoCur().viiNewInfo().strMSAddr(facCIDSock().strIPHostName());
        m_pwndMSAddr->SetEnable(kCIDLib::False);
    }
     else if (wnotEvent.widSource() == kCQCInst::ridPan_MSrv_DoThere)
    {
        m_pwndMSAddr->SetEnable(kCIDLib::True);
    }
    m_pwndMSAddr->strWndText(infoCur().viiNewInfo().strMSAddr());

    return tCIDCtrls::EEvResponses::Handled;
}
