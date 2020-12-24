//
// FILE NAME: CQCInst_AgreePanel.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/30/2004
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
//  This file implements the info panel that shows the user the EULA and
//  get's the assent.
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
RTTIDecls(TInstAgreePanel,TInstInfoPanel)



// ---------------------------------------------------------------------------
//   CLASS: TInstAgreePanel
//  PREFIX: pan
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TInstAgreePanel: Constructors and Destructor
// ---------------------------------------------------------------------------
TInstAgreePanel::TInstAgreePanel(TCQCInstallInfo* const pinfoCur
                                , TMainFrameWnd* const  pwndParent) :

    TInstInfoPanel(L"EULA", pinfoCur, pwndParent)
    , m_pwndAgree(nullptr)
    , m_pwndDisagree(nullptr)
    , m_pwndText(nullptr)
{
}

TInstAgreePanel::~TInstAgreePanel()
{
}


// ---------------------------------------------------------------------------
//  TInstAgreePanel: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TInstAgreePanel::bValidate(tCIDCtrls::TWndId& widFailed, TString& strErrText)
{
    // They have to have agreed with the EULA
    if (!infoCur().bLicenseAgree())
    {
        strErrText.LoadFromMsg(kCQCInstErrs::errcVal_NoEULA, facCQCInst);
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


tCIDLib::TVoid TInstAgreePanel::SaveContents()
{
    // Store the status
    infoCur().bLicenseAgree(m_pwndAgree->bCheckedState());
}


// ---------------------------------------------------------------------------
//  TInstAgreePanel: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TInstAgreePanel::bCreated()
{
    TParent::bCreated();

    // Set our text for use by any popups
    strWndText(facCQCInst.strMsg(kCQCInstMsgs::midPan_Agree_Title));

    // Load up our child controls
    LoadDlgItems(kCQCInst::ridPan_Agreement);

    // Get some pointers to the ones we want to interact with
    CastChildWnd(*this, kCQCInst::ridPan_Agree_Text, m_pwndText);
    CastChildWnd(*this, kCQCInst::ridPan_Agree_Agree, m_pwndAgree);
    CastChildWnd(*this, kCQCInst::ridPan_Agree_Disagree, m_pwndDisagree);

    // Load the license text
    m_pwndText->strWndText(facCQCInst.strMsg(kCQCInstMsgs::midLic_Text));

    // Select the radio button for the current agree state
    if (infoCur().bLicenseAgree())
        m_pwndAgree->SetCheckedState(kCIDLib::True);
    else
        m_pwndDisagree->SetCheckedState(kCIDLib::True);

    return kCIDLib::True;
}
