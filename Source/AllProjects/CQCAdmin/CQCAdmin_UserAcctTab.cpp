//
// FILE NAME: CQCAdmin_UserAcctTab.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 12/11/2015
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
//  This file implements a tab for editing user accounts.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCAdmin.hpp"
#include    "CQCAdmin_UserAcctTab.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TUserAcctTab, TContentTab)



// ---------------------------------------------------------------------------
//  CLASS: TUserAcctTab
// PREFIX: wnd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TUserAcctTab: Constructors and destructor
// ---------------------------------------------------------------------------

//
//  Indicate edit mode in our call to our parent class, since we only support edit
//  mode in this tab type.
//
TUserAcctTab::TUserAcctTab(const TString& strPath, const TString& strRelPath) :

    TContentTab(strPath, strRelPath, L"User Account", kCIDLib::True)
    , m_bHaveKMaps(kCIDLib::False)
    , m_pwndDefIntfName(nullptr)
    , m_pwndDescr(nullptr)
    , m_pwndEdKeyMaps(nullptr)
    , m_pwndEnableTms(nullptr)
    , m_pwndEndTm(nullptr)
    , m_pwndFName(nullptr)
    , m_pwndLogin(nullptr)
    , m_pwndLName(nullptr)
    , m_pwndResetPW(nullptr)
    , m_pwndRole(nullptr)
    , m_pwndSelTmpl(nullptr)
    , m_pwndStartTm(nullptr)
    , m_pwndWebPassword(nullptr)
{
}

TUserAcctTab::~TUserAcctTab()
{
}


// ---------------------------------------------------------------------------
//  TUserAcctTab: Public, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid
TUserAcctTab::CreateTab(        TTabbedWnd&         wndParent
                        , const TString&            strTabText
                        , const TCQCUserAccount&    uaccEdit)
{
    //
    //  Though the main facility should let anyone with less than power user log into
    //  this program, check it just in case.
    //
    CIDAssert
    (
        facCQCAdmin.eUserRole() >= tCQCKit::EUserRoles::PowerUser
        , L"This requires power user or higher privileges"
    )

    // Store away the info, and a second copy for comparisons later
    m_uaccEdit = uaccEdit;
    m_uaccOrg = uaccEdit;

    wndParent.c4CreateTab(this, strTabText, 0, kCIDLib::True);
}


// ---------------------------------------------------------------------------
//  TUserAcctTab: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TUserAcctTab::AreaChanged(  const   TArea&                  areaPrev
                            , const TArea&                  areaNew
                            , const tCIDCtrls::EPosStates   ePosState
                            , const tCIDLib::TBoolean       bOrgChanged
                            , const tCIDLib::TBoolean       bSizeChanged
                            , const tCIDLib::TBoolean       bStateChanged)
{
    // Call our parent first
    TParent::AreaChanged(areaPrev, areaNew, ePosState, bOrgChanged, bSizeChanged, bStateChanged);

    if (bSizeChanged && m_pwndWebPassword && (ePosState != tCIDCtrls::EPosStates::Minimized))
        AutoAdjustChildren(areaPrev, areaNew);
}


tCIDLib::TBoolean TUserAcctTab::bCreated()
{
    TParent::bCreated();

    //
    //  Get the dialog description we'll use to create the controls. We need to get it
    //  first so that we can size the client window initially to the size of the dialog.
    //  This will insure that the auto-child sizing gets the right initial relationship
    //  between the controls the client window. We can then size the client up and
    //  they will get correctly positioned.
    //
    TDlgDesc dlgdChildren;
    facCQCAdmin.bCreateDlgDesc(kCQCAdmin::ridTab_EditAcc, dlgdChildren);

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

    // Get pointers to our handful of controls and load those that show data
    CastChildWnd(*this, kCQCAdmin::ridTab_EditAcc_DefIntfName, m_pwndDefIntfName);
    CastChildWnd(*this, kCQCAdmin::ridTab_EditAcc_Descr, m_pwndDescr);
    CastChildWnd(*this, kCQCAdmin::ridTab_EditAcc_EdKeyMaps, m_pwndEdKeyMaps);
    CastChildWnd(*this, kCQCAdmin::ridTab_EditAcc_EnableTms, m_pwndEnableTms);
    CastChildWnd(*this, kCQCAdmin::ridTab_EditAcc_EndTm, m_pwndEndTm);
    CastChildWnd(*this, kCQCAdmin::ridTab_EditAcc_FName, m_pwndFName);
    CastChildWnd(*this, kCQCAdmin::ridTab_EditAcc_LoginName, m_pwndLogin);
    CastChildWnd(*this, kCQCAdmin::ridTab_EditAcc_LName, m_pwndLName);
    CastChildWnd(*this, kCQCAdmin::ridTab_EditAcc_ResetPW, m_pwndResetPW);
    CastChildWnd(*this, kCQCAdmin::ridTab_EditAcc_Role, m_pwndRole);
    CastChildWnd(*this, kCQCAdmin::ridTab_EditAcc_SelTmpl, m_pwndSelTmpl);
    CastChildWnd(*this, kCQCAdmin::ridTab_EditAcc_StartTm, m_pwndStartTm);
    CastChildWnd(*this, kCQCAdmin::ridTab_EditAcc_WPassword, m_pwndWebPassword);

    //
    //  Load up the values into the combo boxes. We just load up the values 1 through
    //  12 am, then pm. They will be in the correct order such that our 0 based 24 hour
    //  values will be the correct indices.
    //
    TString strTmp;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < 12; c4Index++)
    {
        if (c4Index)
            strTmp.SetFormatted(c4Index);
        else
            strTmp = L"12";
        strTmp.Append(L" AM");

        m_pwndStartTm->c4AddItem(strTmp);
        m_pwndEndTm->c4AddItem(strTmp);
    }

    for (tCIDLib::TCard4 c4Index = 0; c4Index < 12; c4Index++)
    {
        if (c4Index)
            strTmp.SetFormatted(c4Index);
        else
            strTmp = L"12";
        strTmp.Append(L" PM");

        m_pwndStartTm->c4AddItem(strTmp);
        m_pwndEndTm->c4AddItem(strTmp);
    }

    //
    //  Load up the roles combo. If the current user is a power user, not an admin,
    //  then don't add Admin as one of the options. They can't upgrade someone beyond
    //  their own level. And they will not get here for any Admin accounts, so there's
    //  no problem with leaving Admin out.
    //
    tCQCKit::EUserRoles eMax  = tCQCKit::EUserRoles::Max;
    if (facCQCAdmin.eUserRole() == tCQCKit::EUserRoles::PowerUser)
        eMax = tCQCKit::EUserRoles::NormalUser;

    for (tCQCKit::EUserRoles eRole = tCQCKit::EUserRoles::Min; eRole <= eMax; eRole++)
        m_pwndRole->c4AddItem(tCQCKit::strLoadEUserRoles(eRole));

    // Select the incoming role
    m_pwndRole->SelectByIndex(tCIDLib::c4EnumOrd(m_uaccEdit.eRole()));

    // Get the incoming content loaded up
    m_pwndDefIntfName->strWndText(m_uaccEdit.strDefInterfaceName());
    m_pwndDescr->strWndText(m_uaccEdit.strDescription());
    m_pwndEnableTms->SetCheckedState(m_uaccEdit.bLimitTime());
    m_pwndFName->strWndText(m_uaccEdit.strFirstName());
    m_pwndLogin->strWndText(m_uaccEdit.strLoginName());
    m_pwndLName->strWndText(m_uaccEdit.strLastName());
    m_pwndWebPassword->strWndText(m_uaccEdit.strWebPassword());

    //
    //  Select the current start/end login hours. If limits are not enabled, then
    //  disable them.
    //
    m_pwndStartTm->SelectByIndex(m_uaccEdit.c4StartHour());
    m_pwndEndTm->SelectByIndex(m_uaccEdit.c4EndHour());
    if (!m_uaccEdit.bLimitTime())
    {
        m_pwndStartTm->SetEnable(kCIDLib::False);
        m_pwndEndTm->SetEnable(kCIDLib::False);
    }

    // And set up any event handlers
    m_pwndEdKeyMaps->pnothRegisterHandler(this, &TUserAcctTab::eClickHandler);
    m_pwndEnableTms->pnothRegisterHandler(this, &TUserAcctTab::eClickHandler);
    m_pwndResetPW->pnothRegisterHandler(this, &TUserAcctTab::eClickHandler);
    m_pwndSelTmpl->pnothRegisterHandler(this, &TUserAcctTab::eClickHandler);

    return kCIDLib::True;
}


tCIDLib::ESaveRes
TUserAcctTab::eDoSave(          TString&                strErrMsg
                        , const tCQCAdmin::ESaveModes   eMode
                        ,       tCIDLib::TBoolean&      bChangesSaved)
{
    // Get the user role
    const tCQCKit::EUserRoles eRole = tCQCKit::EUserRoles(m_pwndRole->c4CurItem());

    const TString& strTmpl = m_pwndDefIntfName->strWndText();
    if (strTmpl.bIsEmpty() && (eRole == tCQCKit::EUserRoles::LimitedUser))
    {
        strErrMsg = facCQCAdmin.strMsg(kCQCAErrs::errcUAcc_NoTmpl);
        return tCIDLib::ESaveRes::Errors;
    }

    // If we do have a template, make sure it's valid
    if (!strTmpl.bIsEmpty())
    {
        try
        {
            TDataSrvClient dsclToUse;
            if (!dsclToUse.bFileExists(strTmpl, tCQCRemBrws::EDTypes::Template))
            {
                strErrMsg = facCQCAdmin.strMsg(kCQCAErrs::errcUAcc_TmplNotFound);
                return tCIDLib::ESaveRes::Errors;
            }
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);

            strErrMsg = facCQCAdmin.strMsg(kCQCAErrs::errcUAcc_TmplNotFound);
            return tCIDLib::ESaveRes::Errors;
        }
    }

    m_uaccEdit.eRole(eRole);
    m_uaccEdit.bLimitTime(m_pwndEnableTms->bCheckedState());
    m_uaccEdit.SetName(m_pwndFName->strWndText(), m_pwndLName->strWndText());
    m_uaccEdit.strDescription(m_pwndDescr->strWndText());
    m_uaccEdit.strDefInterfaceName(strTmpl);
    m_uaccEdit.strWebPassword(m_pwndWebPassword->strWndText());

    if (m_uaccEdit.bLimitTime())
    {
        // Admin accounts cannot be time limited
        if (eRole == tCQCKit::EUserRoles::SystemAdmin)
        {
            strErrMsg = facCQCAdmin.strMsg(kCQCAErrs::errcUAcc_AdminTimeLimit);
            return tCIDLib::ESaveRes::Errors;
        }

        tCIDLib::TCard4 c4End = m_pwndEndTm->c4CurItem();
        tCIDLib::TCard4 c4Start = m_pwndStartTm->c4CurItem();
        if (c4Start >= c4End)
        {
            strErrMsg = facCQCAdmin.strMsg(kCQCAErrs::errcUAcc_TimeOrder);
            return tCIDLib::ESaveRes::Errors;
        }

        m_uaccEdit.c4EndHour(c4End);
        m_uaccEdit.c4StartHour(c4Start);
    }
     else
    {
        m_uaccEdit.c4EndHour(23);
        m_uaccEdit.c4StartHour(0);
    }

    if (eMode == tCQCAdmin::ESaveModes::Test)
    {
        if ((m_uaccEdit == m_uaccOrg) && (m_kmEdit == m_kmOrg))
            return tCIDLib::ESaveRes::NoChanges;
        return tCIDLib::ESaveRes::OK;
    }

    // There are changes, so write our current edit contents out if allowed
    try
    {
        TWndPtrJanitor janPtr(tCIDCtrls::ESysPtrs::Wait);

        // Let's try to write it back to the security server
        tCQCKit::TSecuritySrvProxy orbcSrc = facCQCKit().orbcSecuritySrvProxy();
        orbcSrc->UpdateAccount(m_uaccEdit, facCQCAdmin.sectUser());

        // If we have edited key maps, then store that if it's changed
        if (m_bHaveKMaps)
        {
            if (m_kmEdit != m_kmOrg)
            {
                facCQCIGKit().SetKeyMapData
                (
                    *this, m_kmEdit, facCQCAdmin.cuctxToUse(), m_uaccEdit.strLoginName()
                );

                // Copy the edits to the original
                m_kmOrg = m_kmEdit;
            }
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        strErrMsg = errToCatch.strErrText();
        return tCIDLib::ESaveRes::Errors;
    }

    // Store away the changes as the new originals now
    m_uaccOrg = m_uaccEdit;
    m_kmOrg = m_kmEdit;
    bChangesSaved = kCIDLib::True;

    // No changes anymore
    return tCIDLib::ESaveRes::NoChanges;
}


// ---------------------------------------------------------------------------
//  TUserAcctTab: Private, non-virtaul methods
// ---------------------------------------------------------------------------

tCIDCtrls::EEvResponses TUserAcctTab::eClickHandler(TButtClickInfo& wnotInfo)
{
    if (wnotInfo.widSource() == kCQCAdmin::ridTab_EditAcc_EdKeyMaps)
    {
        // Query the key maps for this user if not already done
        if (!m_bHaveKMaps)
        {
            m_bHaveKMaps = facCQCIGKit().bQueryKeyMapData
            (
                *this, m_kmOrg, facCQCAdmin.cuctxToUse(), m_uaccEdit.strLoginName()
            );

            // Make a copy for editing
            m_kmEdit = m_kmOrg;
        }

        if (m_bHaveKMaps)
        {
            // Invoke the editor
            facCQCIGKit().bEditKeyMap
            (
                *this
                , m_uaccEdit.strLoginName()
                , tCQCIGKit::EKMActModes::Edit
                , facCQCAdmin.cuctxToUse()
                , m_kmEdit
            );
        }
         else
        {
            TErrBox msgbFailed(L"The key mapping data for this user could not be queried");
            msgbFailed.ShowIt(*this);
        }
    }
     else if (wnotInfo.widSource() == kCQCAdmin::ridTab_EditAcc_EnableTms)
    {
        // Enable/disable the start/end times
        m_pwndStartTm->SetEnable(wnotInfo.bState());
        m_pwndEndTm->SetEnable(wnotInfo.bState());
    }
     else if (wnotInfo.widSource() == kCQCAdmin::ridTab_EditAcc_ResetPW)
    {
        // If any changes save them first
        if (m_kmEdit != m_kmOrg)
        {
            TYesNoBox msgbSaveQ
            (
                L"There are oustanding changes that must be saved first. Would you like "
                L"to continue and try to save them?"
            );
            if (msgbSaveQ.bShowIt(*this))
            {
                // We don't really use the changed saved flag here, we it's needed
                tCIDLib::TBoolean bSaved = kCIDLib::False;
                TString strErr;
                if (eSaveChanges(strErr, tCQCAdmin::ESaveModes::Save, bSaved) == tCIDLib::ESaveRes::Errors)
                {
                    TString strFailMsg
                    (
                        L"The account changes could not be saved due to the following error:\n\n"
                    );
                    strFailMsg.Append(strErr);
                    TErrBox msgbSave(strFailMsg);
                    msgbSave.ShowIt(*this);
                    return tCIDCtrls::EEvResponses::Handled;
                }
            }
        }

        // Get the new PW from the user
        TString strNew;
        tCIDLib::TBoolean bRes = facCIDWUtils().bGetText
        (
            *this
            , L"Enter a new password for this user account"
            , TString::strEmpty()
            , strNew
        );

        if (bRes)
        {
            try
            {
                // get a security server proxy and request the change
                tCQCKit::TSecuritySrvProxy orbcSS = facCQCKit().orbcSecuritySrvProxy();

                orbcSS->ResetAccountPassword
                (
                    m_uaccEdit.strLoginName(), strNew, facCQCAdmin.sectUser()
                );
            }

            catch(TError& errToCatch)
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
                TExceptDlg dlgErr
                (
                    *this
                    , strWndText()
                    , L"The password reset attempt failed"
                    , errToCatch
                );
            }
        }
    }
     else if (wnotInfo.widSource() == kCQCAdmin::ridTab_EditAcc_SelTmpl)
    {
        // Let the user select a template
        TString strNew;
        const tCIDLib::TBoolean bRes = facCQCTreeBrws().bSelectFile
        (
            *this
            , L"Select a Template"
            , tCQCRemBrws::EDTypes::Template
            , m_pwndDefIntfName->strWndText()
            , facCQCAdmin.cuctxToUse()
            , tCQCTreeBrws::EFSelFlags::SelectItems
            , strNew
        );

        // If they selected one, update the display
        if (bRes)
            m_pwndDefIntfName->strWndText(strNew);
    }
    return tCIDCtrls::EEvResponses::Handled;
}



