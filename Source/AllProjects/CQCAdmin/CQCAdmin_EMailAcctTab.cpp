//
// FILE NAME: CQCAdmin_EMailAcctTab.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/02/2016
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
//  This file implements a tab for editing E-Mail accounts.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCAdmin.hpp"
#include    "CQCAdmin_EMailAcctTab.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TEMailAcctTab, TContentTab)



// ---------------------------------------------------------------------------
//  CLASS: TEMailAcctTab
// PREFIX: wnd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TEMailAcctTab: Constructors and destructor
// ---------------------------------------------------------------------------

//
//  Indicate edit mode in our call to our parent class, since we only support edit
//  mode in this tab type.
//
TEMailAcctTab::TEMailAcctTab(const TString& strPath, const TString& strRelPath) :

    TContentTab(strPath, strRelPath, L"E-Mail", kCIDLib::True)
    , m_pwndFromAddr(nullptr)
    , m_pwndName(nullptr)
    , m_pwndPassword(nullptr)
    , m_pwndPort(nullptr)
    , m_pwndServer(nullptr)
    , m_pwndType(nullptr)
    , m_pwndUser(nullptr)
{
}

TEMailAcctTab::~TEMailAcctTab()
{
}


// ---------------------------------------------------------------------------
//  TEMailAcctTab: Public, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid
TEMailAcctTab::CreateTab(       TTabbedWnd&         wndParent
                        , const TCQCEMailAccount&   emacctEdit)
{
    //
    //  Though the main facility should not let anyone with less than power user log into
    //  this program, check it just in case.
    //
    CIDAssert
    (
        facCQCAdmin.eUserRole() >= tCQCKit::EUserRoles::PowerUser
        , L"This requires power user or higher privileges"
    )

    // Store away the info, and a second copy for comparisons later
    m_emacctEdit = emacctEdit;
    m_emacctOrg = emacctEdit;

    wndParent.c4CreateTab(this, emacctEdit.strAcctName(), 0, kCIDLib::True);
}


// ---------------------------------------------------------------------------
//  TEMailAcctTab: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TEMailAcctTab::AreaChanged( const   TArea&                  areaPrev
                            , const TArea&                  areaNew
                            , const tCIDCtrls::EPosStates   ePosState
                            , const tCIDLib::TBoolean       bOrgChanged
                            , const tCIDLib::TBoolean       bSizeChanged
                            , const tCIDLib::TBoolean       bStateChanged)
{
    // Call our parent first
    TParent::AreaChanged(areaPrev, areaNew, ePosState, bOrgChanged, bSizeChanged, bStateChanged);

    if (bSizeChanged &&  (ePosState != tCIDCtrls::EPosStates::Minimized) &&  m_pwndUser)
        AutoAdjustChildren(areaPrev, areaNew);
}


tCIDLib::TBoolean TEMailAcctTab::bCreated()
{
    TParent::bCreated();

    // Get the dialog description we'll use to create the controls
    TDlgDesc dlgdChildren;
    facCQCAdmin.bCreateDlgDesc(kCQCAdmin::ridDlg_EdEMAcct, dlgdChildren);

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
    CastChildWnd(*this, kCQCAdmin::ridDlg_EdEMAcct_FromAddr, m_pwndFromAddr);
    CastChildWnd(*this, kCQCAdmin::ridDlg_EdEMAcct_Name, m_pwndName);
    CastChildWnd(*this, kCQCAdmin::ridDlg_EdEMAcct_Password, m_pwndPassword);
    CastChildWnd(*this, kCQCAdmin::ridDlg_EdEMAcct_Port, m_pwndPort);
    CastChildWnd(*this, kCQCAdmin::ridDlg_EdEMAcct_Server, m_pwndServer);
    CastChildWnd(*this, kCQCAdmin::ridDlg_EdEMAcct_Type, m_pwndType);
    CastChildWnd(*this, kCQCAdmin::ridDlg_EdEMAcct_User, m_pwndUser);

    // Load the accounts types combo box
    tCQCKit::EEMailTypes eType = tCQCKit::EEMailTypes::Min;
    for (; eType < tCQCKit::EEMailTypes::Count; eType++)
        m_pwndType->c4AddItem(tCQCKit::strXlatEEMailTypes(eType));

    // Load up the incoming info
    m_pwndFromAddr->strWndText(m_emacctEdit.strFromAddr());
    m_pwndName->strWndText(m_emacctEdit.strAcctName());
    m_pwndPassword->strWndText(m_emacctEdit.strPassword());
    m_pwndServer->strWndText(m_emacctEdit.strServer());
    m_pwndUser->strWndText(m_emacctEdit.strUserName());
    m_pwndType->SelectByIndex(tCIDLib::c4EnumOrd(m_emacctEdit.eType()));

    // The name can't be changed here, so disable it
    m_pwndName->SetEnable(kCIDLib::False);

    // Load up the port, which we have to format out
    TString strPort;
    strPort.AppendFormatted(m_emacctEdit.ippnMail());
    m_pwndPort->strWndText(strPort);

    return kCIDLib::True;
}


tCIDLib::ESaveRes
TEMailAcctTab::eDoSave(         TString&                strErrMsg
                        , const tCQCAdmin::ESaveModes   eMode
                        ,       tCIDLib::TBoolean&      bChangesSaved)
{
    // Get the type
    const tCQCKit::EEMailTypes eType = tCQCKit::EEMailTypes(m_pwndType->c4CurItem());

    //
    //  Make sure that all of the fields are non-empty, except the password.
    //  We don't force them to provide one if their server doesn't require
    //  authentication.
    //
    TString strFromAddr = m_pwndFromAddr->strWndText();
    TString strName = m_pwndName->strWndText();
    TString strPassword = m_pwndPassword->strWndText();
    TString strPort = m_pwndPort->strWndText();
    TString strServer = m_pwndServer->strWndText();
    TString strUser = m_pwndUser->strWndText();

    // Strip any whitespace
    strFromAddr.StripWhitespace();
    strName.StripWhitespace();
    strPassword.StripWhitespace();
    strPort.StripWhitespace();
    strServer.StripWhitespace();
    strUser.StripWhitespace();

    if (strFromAddr.bIsEmpty()
    ||  strName.bIsEmpty()
    ||  strServer.bIsEmpty()
    ||  strUser.bIsEmpty())
    {
        strErrMsg = facCQCAdmin.strMsg(kCQCAErrs::errcEMail_MissingField);
        return tCIDLib::ESaveRes::Errors;
    }

    tCIDLib::TIPPortNum ippnTest;
    if (!strPort.bToCard4(ippnTest, tCIDLib::ERadices::Dec)
    ||  (ippnTest == 0)
    ||  (ippnTest > 0xFFFF))
    {
        strErrMsg = facCQCAdmin.strMsg(kCQCAErrs::errcEMail_BadPort);
        return tCIDLib::ESaveRes::Errors;
    }

    // Looks ok, so store it away
    m_emacctEdit.Set(eType, strName, strFromAddr, strPassword, strServer, strUser, ippnTest);

    // If testing, we just check to see if this is the same as our last stored original values
    if (eMode == tCQCAdmin::ESaveModes::Test)
    {
        if (m_emacctEdit == m_emacctOrg)
            return tCIDLib::ESaveRes::NoChanges;
        return tCIDLib::ESaveRes::OK;
    }

    try
    {
        tCQCKit::TInstSrvProxy orbcIS = facCQCKit().orbcInstSrvProxy();
        tCIDLib::TCard4 c4SerialNum = 0;
        orbcIS->bUpdateEmailAccount(m_emacctEdit, c4SerialNum, facCQCAdmin.sectUser(), kCIDLib::False);

        // It worked so copy the new stuff to the original
        m_emacctOrg = m_emacctEdit;
        bChangesSaved = kCIDLib::True;
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        strErrMsg = errToCatch.strErrText();
        return tCIDLib::ESaveRes::Errors;
    }

    // No more changes now
    return tCIDLib::ESaveRes::NoChanges;
}

