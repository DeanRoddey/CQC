//
// FILE NAME: CQCTreeBrws_DrvWizDlg_Panels.cpp
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
//  This file implements the 'panels' that are used by the driver installation wizard
//  to get required info from the user to install a driver. The wizard dialog file
//  is the only one that uses these classes.
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
#include    "CQCTreeBrws_DrvWizDlg_.hpp"
#include    "CQCTreeBrws_DrvWizDlg_Panels_.hpp"



// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TDrvWizPanel,TTabWindow)
RTTIDecls(TDWACPromptPanel,TTabWindow)
RTTIDecls(TDWAudDevPanel,TTabWindow)
RTTIDecls(TDWBoolPanel,TTabWindow)
RTTIDecls(TDWFileSelPanel,TTabWindow)
RTTIDecls(TDWListPanel,TTabWindow)
RTTIDecls(TDWMListPanel,TTabWindow)
RTTIDecls(TDWMonikerPanel,TDrvWizPanel)
RTTIDecls(TDWMediaRepoPanel,TDrvWizPanel)
RTTIDecls(TDWNamedListPanel,TTabWindow)
RTTIDecls(TDWODBCPanel,TTabWindow)
RTTIDecls(TDWRangePanel,TDrvWizPanel)
RTTIDecls(TDWSerialPanel,TDrvWizPanel)
RTTIDecls(TDWSockPanel,TDrvWizPanel)
RTTIDecls(TDWSumPanel,TDrvWizPanel)
RTTIDecls(TDWTextPanel,TDrvWizPanel)
RTTIDecls(TDWTTSVoicePanel,TDrvWizPanel)
RTTIDecls(TDWUPnPCommPanel,TDrvWizPanel)



// ---------------------------------------------------------------------------
//   CLASS: TDrvWizPanel
//  PREFIX: pan
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TDrvWizPanel: Public Destructor
// ---------------------------------------------------------------------------
TDrvWizPanel::~TDrvWizPanel()
{
}


// ---------------------------------------------------------------------------
//  TDrvWizPanel: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TDrvWizPanel::CreatePanel(          TCQCDrvWizDlg* const    pwndDlg
                            ,       TTabbedWnd&             wndTabs
                            , const TCQCSecToken&           sectUser)
{
    //
    //  Force an update of the available non-local serial ports, to be sure we get
    //  added after we started.
    //
    if (!facGC100Ser().bUpdateFactory(sectUser))
    {
        TErrBox msgbErr
        (
            L"The list of GC-100 based COM ports could not be refreshed, so recently added "
            L"ones may not be available for selection"
        );
        msgbErr.ShowIt(*pwndDlg);
    }

    if (!facJAPwrSer().bUpdateFactory(sectUser))
    {
        TErrBox msgbErr
        (
            L"The list of Just Add Power based COM ports could not be refreshed, so recently "
            " added ones may not be available for selection"
        );
        msgbErr.ShowIt(*pwndDlg);
    }

    // Load up the dialog description for this type of panel
    TDlgDesc dlgdLoad;
    if (!facCQCTreeBrws().bCreateDlgDesc(m_ridDlgDescr, dlgdLoad))
    {
        facCQCTreeBrws().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kTBrwsErrs::errcDrvI_LoadPanelDlg
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::InitFailed
            , m_strName
        );
    }

    // Save away a pointer to our parent dialog for later use
    m_pwndParent = pwndDlg;

    // Ask the tabbed window to create us and add us as a tab
    wndTabs.c4CreateTab(this, strName());

    // And now create all of the controls as children of us
    CreateDlgItems(dlgdLoad, m_widInitFocus);
}


// Provide access to our panel type
TDrvWizPanel::EPanTypes TDrvWizPanel::eType() const
{
    return m_eType;
}



// ---------------------------------------------------------------------------
//  TDrvWizPanel: Hidden constructors
// ---------------------------------------------------------------------------
TDrvWizPanel::TDrvWizPanel( const   TString&                strName
                            , const TDrvWizPanel::EPanTypes eType
                            , const tCIDLib::TResId         ridDlgDescr
                            ,       TCQCDriverObjCfg* const pcqcdcEdit) :

    TTabWindow(strName, TString::strEmpty(), kCIDLib::False, kCIDLib::True)
    , m_eType(eType)
    , m_pcqcdcEdit(pcqcdcEdit)
    , m_ridDlgDescr(ridDlgDescr)
    , m_strName(strName)
    , m_widInitFocus(kCIDCtrls::widInvalid)
{
}


// ---------------------------------------------------------------------------
//  TDrvWizPanel: Protected, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid
TDrvWizPanel::AreaChanged(  const   TArea&                  areaPrev
                            , const TArea&                  areaNew
                            , const tCIDCtrls::EPosStates   ePosState
                            , const tCIDLib::TBoolean       bOrgChanged
                            , const tCIDLib::TBoolean       bSizeChanged
                            , const tCIDLib::TBoolean       bStateChanged)
{
    // Call our parent first
    TParent::AreaChanged(areaPrev, areaNew, ePosState, bOrgChanged, bSizeChanged, bStateChanged);

    if (bSizeChanged && (ePosState != tCIDCtrls::EPosStates::Minimized))
    {
        // Auto-resize our child widgets
        AutoAdjustChildren(areaPrev, areaNew);
    }
}


// ---------------------------------------------------------------------------
//  TDrvWizPanel: Protected, non-virtual methods
// ---------------------------------------------------------------------------

//
//  There are a few places where file/path wild cards are indicated in a
//  manifest prompt, so we have to parse them out in order to use them in
//  a file open dialog. We get a comma delimited list of wild card specs.
//
tCIDLib::TVoid
TDrvWizPanel::ParseWC(const TString& strWC, tCIDLib::TKVPList& colToFill)
{
    TStringTokenizer stokCrit(&strWC, L";");
    TString strDescr;
    TString strWS;
    while (stokCrit.bGetNextToken(strDescr))
    {
        tCIDLib::TCard4 c4Ofs;
        if (!strDescr.bFirstOccurrence(kCIDLib::chComma, c4Ofs))
            break;

        strDescr.CopyOutSubStr(strWS, c4Ofs + 1);
        strDescr.CapAt(c4Ofs);
        strDescr.StripWhitespace();
        strWS.StripWhitespace();
        colToFill.objPlace(strDescr, strWS);
    }

    // If there weren't any, that's bad, but just put in a default one
    if (colToFill.bIsEmpty())
        colToFill.objPlace(L"All Files", L"*.*");
}


// Let the derived classes get to the driver config we are editing
TCQCDriverObjCfg& TDrvWizPanel::cqcdcEdit()
{
    return *m_pcqcdcEdit;
}

const TCQCDriverObjCfg& TDrvWizPanel::cqcdcEdit() const
{
    return *m_pcqcdcEdit;
}



// ---------------------------------------------------------------------------
//   CLASS: TDWAudDevPanel
//  PREFIX: pan
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TDWAudDevPanel: Constructors and Destructor
// ---------------------------------------------------------------------------
TDWAudDevPanel::TDWAudDevPanel(         TCQCDriverObjCfg* const pcqcdcEdit
                                , const tCIDLib::TCard4         c4PromptInd) :


    TDrvWizPanel
    (
        TString(L"Option ") + TString(TCardinal(c4PromptInd + 1))
        , TDrvWizPanel::EPanTypes::Prompt
        , kCQCTreeBrws::ridDlg_DrvWAudPr
        , pcqcdcEdit
    )
    , m_c4PromptInd(c4PromptInd)
    , m_pwndDescr(nullptr)
    , m_pwndDevList(nullptr)
{
}

TDWAudDevPanel::~TDWAudDevPanel()
{
}

// ---------------------------------------------------------------------------
//  TDWAudDevPanel: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TDWAudDevPanel::bValidate(  TCQCDrvWizDlg&          wndParent
                            , tCIDCtrls::TWndId&    widFailed
                            , TString&              strErrText)
{
    TCQCDriverObjCfg& cqcdcSrc = cqcdcEdit();
    const TCQCDCfgPrompt& cqcdcpCur = cqcdcSrc.cqcdcpAt(m_c4PromptInd);

    // Make sure an audio device is selected
    if (cqcdcSrc.strPromptValue(cqcdcpCur.strName(), L"DevName").bIsEmpty())
    {
        strErrText.LoadFromMsg(kTBrwsErrs::errcDrvI_NoAudioDev, facCQCTreeBrws());
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


tCIDLib::TVoid
TDWAudDevPanel::Initialize(TCQCDrvWizDlg& wndParent, const tCIDLib::TStrList& colPorts)
{
    // Get some pointers to the ones we want to interact with
    CastChildWnd(*this, kCQCTreeBrws::ridDlg_DrvWAudPr_List, m_pwndDevList);
    CastChildWnd(*this, kCQCTreeBrws::ridDlg_DrvWAudPr_Descr, m_pwndDescr);

    TCQCDriverObjCfg& cqcdcSrc = cqcdcEdit();
    const TCQCDCfgPrompt& cqcdcpCur = cqcdcSrc.cqcdcpAt(m_c4PromptInd);

    // Load up the available devices
    tCIDLib::TStrList colDevs;
    try
    {
        TCQCSrvAdminClientProxy* porbcProxy
        (
            facCQCKit().porbcCQCSrvAdminProxy(wndParent.strHost())
        );

        if (porbcProxy)
        {
            TJanitor<TCQCSrvAdminClientProxy> janProxy(porbcProxy);
            porbcProxy->c4QueryAudioDevs(colDevs);
        }
    }

    catch(TError& errToCatch)
    {
        if (facCQCTreeBrws().bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
    }

    const tCIDLib::TCard4 c4Count = colDevs.c4ElemCount();
    if (c4Count)
    {
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            m_pwndDevList->c4AddItem(colDevs[c4Index]);

        tCIDLib::TCard4 c4At;
        TString strCurVal;
        if (!cqcdcSrc.bFindPromptValue(cqcdcpCur.strName(), L"DevName", strCurVal)
        ||  !m_pwndDevList->bSelectByText(strCurVal, c4At))
        {
            m_pwndDevList->SelectByIndex(0);
            cqcdcSrc.SetPromptVal(cqcdcpCur.strName(), L"DevName", colDevs[0]);
        }
    }

    // Register our handlers
    m_pwndDevList->pnothRegisterHandler(this, &TDWAudDevPanel::eLBHandler);
}


// ---------------------------------------------------------------------------
//  TDWAudDevPanel: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDCtrls::EEvResponses TDWAudDevPanel::eLBHandler(TListChangeInfo& wnotInfo)
{
    TCQCDriverObjCfg& cqcdcSrc = cqcdcEdit();
    const TCQCDCfgPrompt& cqcdcpCur = cqcdcSrc.cqcdcpAt(m_c4PromptInd);

    if (wnotInfo.eEvent() == tCIDCtrls::EListEvents::SelChanged)
    {
        TString strDev;
        m_pwndDevList->ValueAt(wnotInfo.c4Index(), strDev);
        cqcdcSrc.SetPromptVal(cqcdcpCur.strName(), L"DevName", strDev);
    }
     else if (wnotInfo.eEvent() == tCIDCtrls::EListEvents::Cleared)
    {
        cqcdcSrc.SetPromptVal(cqcdcpCur.strName(), L"DevName", TString::strEmpty());
    }
    return tCIDCtrls::EEvResponses::Handled;
}



// ---------------------------------------------------------------------------
//   CLASS: TDWACPromptPanel
//  PREFIX: pan
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TDWACPromptPanel: Constructors and Destructor
// ---------------------------------------------------------------------------
TDWACPromptPanel::TDWACPromptPanel(         TCQCDriverObjCfg* const pcqcdcEdit
                                    , const tCIDLib::TCard4         c4PromptInd) :

    TDrvWizPanel
    (
        L"App Control"
        , TDrvWizPanel::EPanTypes::Prompt
        , kCQCTreeBrws::ridDlg_DrvWACPr
        , pcqcdcEdit
    )
    , m_bOptional(kCIDLib::False)
    , m_bPathOnly(kCIDLib::False)
    , m_c4PromptInd(c4PromptInd)
    , m_pwndBinding(nullptr)
    , m_pwndPath(nullptr)
{
    TCQCDriverObjCfg& cqcdcSrc = cqcdcEdit();
    const TCQCDCfgPrompt& cqcdcpOurs = cqcdcSrc.cqcdcpAt(c4PromptInd);

    m_bOptional     = cqcdcpOurs.bOptional();
    m_bPathOnly     = cqcdcpOurs.bFlag();
    m_strFileTypes  = cqcdcpOurs.strLimits();
}

TDWACPromptPanel::~TDWACPromptPanel()
{
}


// ---------------------------------------------------------------------------
//  TDWACPromptPanel: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TDWACPromptPanel::bValidate(TCQCDrvWizDlg&          wndParent
                            , tCIDCtrls::TWndId&    widFailed
                            , TString&              strErrText)
{
    // Store the current info
    tCIDLib::TCard4 c4Index = m_pwndBinding->c4CurItem();
    TString strBind;
    TPathStr pathTar;
    if (c4Index != kCIDLib::c4MaxCard)
    {
        m_pwndBinding->ValueAt(c4Index, strBind);
        pathTar = m_pwndPath->strWndText();
    }

    //
    //  If only the path is wanted, make sure there's no extension. We can't tell
    //  the difference between a file with no extension and a directory.
    //
    if (m_bPathOnly)
    {
        TString strExt;
        if (pathTar.bQueryExt(strExt))
        {
            strErrText.LoadFromMsg(kTBrwsErrs::errcDrvI_PathOnly, facCQCTreeBrws());
            return kCIDLib::False;
        }
    }

    TCQCDriverObjCfg& cqcdcTar = cqcdcEdit();
    const TCQCDCfgPrompt& cqcdcpCur = cqcdcTar.cqcdcpAt(m_c4PromptInd);

    const TString& strKey = cqcdcpCur.strName();
    cqcdcTar.SetPromptVal(strKey, L"Binding", strBind);
    cqcdcTar.SetPromptVal(strKey, L"Path", pathTar);

    // The binding value must always be present. The path can be optional.
    if (cqcdcEdit().strPromptValue(strKey, L"Binding").bIsEmpty())
    {
        strErrText.LoadFromMsg(kTBrwsErrs::errcDrvI_NoTMBinding, facCQCTreeBrws());
        return kCIDLib::False;
    }

    if (!m_bOptional && cqcdcEdit().strPromptValue(strKey, L"Path").bIsEmpty())
    {
        strErrText.LoadFromMsg(kTBrwsErrs::errcDrvI_NoReqACPath, facCQCTreeBrws());
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


tCIDLib::TVoid
TDWACPromptPanel::Initialize(TCQCDrvWizDlg& wndParent, const tCIDLib::TStrList& colPorts)
{
    // Get some pointers to the ones we want to interact with
    CastChildWnd(*this, kCQCTreeBrws::ridDlg_DrvWACPr_Value, m_pwndBinding);
    CastChildWnd(*this, kCQCTreeBrws::ridDlg_DrvWACPr_Path, m_pwndPath);

    // Load up the available bindings
    try
    {
        tCIDOrbUC::TNSrvProxy orbcNS = facCIDOrbUC().orbcNameSrvProxy();
        TVector<TNameServerInfo> colList;
        const tCIDLib::TCard4 c4BindCnt = orbcNS->c4EnumObjects
        (
            TAppCtrlClientProxy::strImplScope, colList, kCIDLib::False
        );

        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4BindCnt; c4Index++)
            m_pwndBinding->c4AddItem(colList[c4Index].strNodeName());

        // Try to select the current values if any
        TCQCDriverObjCfg& cqcdcSrc = cqcdcEdit();
        const TString& strKey = cqcdcSrc.cqcdcpAt(m_c4PromptInd).strName();

        TString strPVal;
        tCIDLib::TCard4 c4At;
        if (cqcdcSrc.bFindPromptValue(strKey, L"Binding", strPVal))
            m_pwndBinding->bSelectByText(strPVal, c4At);

        // Put the path value into the path entry field
        if (cqcdcSrc.bFindPromptValue(strKey, L"Path", strPVal))
            m_pwndPath->strWndText(strPVal);

        // If no bindings, then disable selection combo, else enable
        m_pwndBinding->SetEnable(c4BindCnt != 0);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        TErrBox msgbFail(errToCatch.strErrText());
        msgbFail.ShowIt(wndParent);
    }
}




// ---------------------------------------------------------------------------
//   CLASS: TDWBoolPanel
//  PREFIX: pan
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TDWBoolPanel: Constructors and Destructor
// ---------------------------------------------------------------------------
TDWBoolPanel::TDWBoolPanel(         TCQCDriverObjCfg* const pcqcdcEdit
                            , const tCIDLib::TCard4         c4PromptInd) :

    TDrvWizPanel
    (
        TString(L"Option ") + TString(TCardinal(c4PromptInd + 1))
        , TDrvWizPanel::EPanTypes::Prompt
        , kCQCTreeBrws::ridDlg_DrvWBoolPr
        , pcqcdcEdit
    )
    , m_c4PromptInd(c4PromptInd)
    , m_pwndDescr(nullptr)
    , m_pwndValue(nullptr)
    , m_strSubKey(L"State")
{
}

TDWBoolPanel::~TDWBoolPanel()
{
}

// ---------------------------------------------------------------------------
//  TDWBoolPanel: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TDWBoolPanel::bValidate(TCQCDrvWizDlg&          wndParent
                        , tCIDCtrls::TWndId&    widFailed
                        , TString&              strErrText)
{
    TCQCDriverObjCfg& cqcdcSrc = cqcdcEdit();
    const TCQCDCfgPrompt& cqcdcpCur = cqcdcSrc.cqcdcpAt(m_c4PromptInd);

    // Just store away the value
    cqcdcSrc.SetPromptVal
    (
        cqcdcpCur.strName()
        , m_strSubKey
        , m_pwndValue->bCheckedState() ? kCQCKit::pszFld_True : kCQCKit::pszFld_False
    );
    return kCIDLib::True;
}


tCIDLib::TVoid
TDWBoolPanel::Initialize(TCQCDrvWizDlg& wndParent, const tCIDLib::TStrList& colPorts)
{
    // Get some pointers to the ones we want to interact with
    CastChildWnd(*this, kCQCTreeBrws::ridDlg_DrvWBoolPr_Descr, m_pwndDescr);
    CastChildWnd(*this, kCQCTreeBrws::ridDlg_DrvWBoolPr_Value, m_pwndValue);

    TCQCDriverObjCfg& cqcdcSrc = cqcdcEdit();
    const TCQCDCfgPrompt& cqcdcpCur = cqcdcSrc.cqcdcpAt(m_c4PromptInd);

    // The descriptive text for prompts is dynamic
    m_pwndDescr->strWndText(cqcdcpCur.strDescrText());

    // And the text of the value check box, which is stored in the limit field
    m_pwndValue->strWndText(cqcdcpCur.strLimits());

    // Set the value check box
    TString strPVal;
    if (cqcdcEdit().bFindPromptValue(cqcdcpCur.strName(), m_strSubKey, strPVal))
        m_pwndValue->SetCheckedState(strPVal == kCQCKit::pszFld_True);
}



// ---------------------------------------------------------------------------
//   CLASS: TDWFileSelPanel
//  PREFIX: pan
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TDWFileSelPanel: Constructors and Destructor
// ---------------------------------------------------------------------------
TDWFileSelPanel::TDWFileSelPanel(       TCQCDriverObjCfg* const pcqcdcEdit
                                , const tCIDLib::TCard4         c4PromptInd
                                , const tCIDLib::TBoolean       bFileMode) :

    TDrvWizPanel
    (
        TString(L"Option ") + TString(TCardinal(c4PromptInd + 1))
        , TDrvWizPanel::EPanTypes::Prompt
        , kCQCTreeBrws::ridDlg_DrvWFileSelPr
        , pcqcdcEdit
    )
    , m_bCMLDriver(kCIDLib::False)
    , m_bFileMode(bFileMode)
    , m_bOptional(kCIDLib::False)
    , m_c4PromptInd(c4PromptInd)
    , m_pwndBrowse(nullptr)
    , m_pwndDescr(nullptr)
    , m_pwndFile(nullptr)
    , m_strSubKey(bFileMode ? L"File" : L"Path")
{
    TCQCDriverObjCfg& cqcdcSrc = cqcdcEdit();
    const TCQCDCfgPrompt& cqcdcpCur = cqcdcSrc.cqcdcpAt(m_c4PromptInd);

    m_bCMLDriver = pcqcdcEdit->eType() == tCQCKit::EDrvTypes::Macro;
    m_bOptional = cqcdcpCur.bOptional();
    m_strFileTypes = cqcdcpCur.strLimits();
}

TDWFileSelPanel::~TDWFileSelPanel()
{
}

// ---------------------------------------------------------------------------
//  TDWFileSelPanel: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TDWFileSelPanel::bValidate( TCQCDrvWizDlg&          wndParent
                            , tCIDCtrls::TWndId&    widFailed
                            , TString&              strErrText)
{
    TCQCDriverObjCfg& cqcdcSrc = cqcdcEdit();
    const TCQCDCfgPrompt& cqcdcpCur = cqcdcSrc.cqcdcpAt(m_c4PromptInd);

    // If the file/path isn't optional, make sure one is selected
    const TString& strFile = m_pwndFile->strWndText();
    if (!m_bOptional)
    {
        // const TString& strPath(cqcdcEdit().strPromptValue(cqcdcpCur.strName(), m_strSubKey));
        if (strFile.bIsEmpty())
        {
            widFailed = m_pwndFile->widThis();
            strErrText.LoadFromMsg(kTBrwsErrs::errcDrvI_ReqPrompt, facCQCTreeBrws());
            return kCIDLib::False;
        }
    }

    cqcdcSrc.SetPromptVal(cqcdcpCur.strName(), m_strSubKey, strFile);
    return kCIDLib::True;
}


tCIDLib::TVoid
TDWFileSelPanel::Initialize(TCQCDrvWizDlg& wndParent, const tCIDLib::TStrList& colPorts)
{
    // Get some pointers to the ones we want to interact with
    CastChildWnd(*this, kCQCTreeBrws::ridDlg_DrvWFileSelPr_Browse, m_pwndBrowse);
    CastChildWnd(*this, kCQCTreeBrws::ridDlg_DrvWFileSelPr_Descr, m_pwndDescr);
    CastChildWnd(*this, kCQCTreeBrws::ridDlg_DrvWFileSelPr_Path, m_pwndFile);

    TCQCDriverObjCfg& cqcdcSrc = cqcdcEdit();
    const TCQCDCfgPrompt& cqcdcpCur = cqcdcSrc.cqcdcpAt(m_c4PromptInd);

    // The descriptive text for prompts is dynamic
    m_pwndDescr->strWndText(cqcdcpCur.strDescrText());

    //
    //  Load up the current path if any. If this is a CML driver, then we need to
    //  deal with that.
    //
    TPathStr pathCur;
    if (cqcdcEdit().bFindPromptValue(cqcdcpCur.strName(), m_strSubKey, pathCur))
    {
        if (m_bCMLDriver && !pathCur.bIsEmpty())
        {
            //
            //  If we get one that starts with the macro file root, i.e. they
            //  worked around the bug that existed before we dealt with this and
            //  pointed it at the fully qualified path, then just remove that
            //  part part and keep the rest, which should be right now.
            //
            //  If not empty and it doesn't start with a slash, then clear it.
            //
            if (pathCur.bStartsWithI(facCQCKit().strMacroRootPath()))
                pathCur.Cut(0, facCQCKit().strMacroRootPath().c4Length());

            if (pathCur[0] != L'\\')
                pathCur.Clear();
        }
    }
    m_pwndFile->strWndText(pathCur);

    // Register our browsing button
    m_pwndBrowse->pnothRegisterHandler(this, &TDWFileSelPanel::eClickHandler);
}


// ---------------------------------------------------------------------------
//  TDWFileSelPanel: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDCtrls::EEvResponses TDWFileSelPanel::eClickHandler(TButtClickInfo&)
{
    TCQCDriverObjCfg& cqcdcSrc = cqcdcEdit();

    //
    //  Set up a file type list. This is based on the search criteria from
    //  the manifest file. They are stored in the limits field for file
    //  prompts.
    //
    tCIDLib::TKVPList colFileTypes(4);
    ParseWC(m_strFileTypes, colFileTypes);

    // We have to use the server side macro path if a CML driver
    TPathStr pathSrvMacroPath = facCQCKit().strDataDir();
    pathSrvMacroPath.AddLevel(L"MacroFileRoot");

    //
    //  Get the starting path and file. If a CML driver we have to put the
    //  macro file root before it first.
    //
    TPathStr pathStart;
    TString  strResult;
    if (m_bCMLDriver)
    {
        pathStart = pathSrvMacroPath;
        if (!m_pwndFile->strWndText().bIsEmpty())
            pathStart.AddLevel(m_pwndFile->strWndText());
    }
     else
    {
        pathStart = m_pwndFile->strWndText();
    }
    pathStart.bExtractNameExt(strResult);

    //
    //  If we are in folder mode, we can set folders only and get it to do that
    //  filtering for us. Otherwise, we have to check the return and make sure it's
    //  a file.
    //
    tCIDCtrls::EFOpenOpts eOpts = tCIDCtrls::EFOpenOpts::FileSystemOnly;
    if (!m_bFileMode)
        eOpts = tCIDLib::eOREnumBits(eOpts, tCIDCtrls::EFOpenOpts::SelectFolders);

    tCIDLib::TStrList colSelList;
    const tCIDLib::TBoolean bRes = facCIDCtrls().bOpenFileDlg
    (
        *this
        , m_bFileMode ? L"Select a File" : L"Select a Path"
        , pathStart
        , colSelList
        , colFileTypes
        , eOpts
    );

    if (bRes)
    {
        TString& strSel = colSelList[0];

        // Make sure it has the right slash type
        strSel.bReplaceChar(L'/', L'\\');

        //
        //  If a CML driver, remove the macro file root part. And it has to start
        //  with that path.
        //
        if (m_bCMLDriver)
        {
            if (!strSel.bStartsWithI(pathSrvMacroPath))
            {
                TErrBox msgbErr(facCQCTreeBrws().strMsg(kTBrwsErrs::errcDrvI_NotMacroPath));
                msgbErr.ShowIt(*this);
                return tCIDCtrls::EEvResponses::Handled;
            }
            strSel.Cut(0, pathSrvMacroPath.c4Length());
        }

        // If file mode, make sure it's a file
        if (m_bFileMode && TFileSys::bIsDirectory(strSel))
        {
            TErrBox msgbErr(facCQCTreeBrws().strMsg(kTBrwsErrs::errcDrvI_NotAFile));
            msgbErr.ShowIt(*this);
            return tCIDCtrls::EEvResponses::Handled;
        }

        // Look ok, so update our display and data
        m_pwndFile->strWndText(strSel);
        TCQCDCfgPrompt& cqcdcpCur = cqcdcSrc.cqcdcpAt(m_c4PromptInd);
        cqcdcSrc.SetPromptVal(cqcdcpCur.strName(), m_strSubKey, strSel);
    }

    return tCIDCtrls::EEvResponses::Handled;
}



// ---------------------------------------------------------------------------
//   CLASS: TDWListPanel
//  PREFIX: pan
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TDWListPanel: Constructors and Destructor
// ---------------------------------------------------------------------------
TDWListPanel::TDWListPanel(         TCQCDriverObjCfg* const pcqcdcEdit
                            , const tCIDLib::TCard4         c4PromptInd) :

    TDrvWizPanel
    (
        TString(L"Option ") + TString(TCardinal(c4PromptInd + 1))
        , TDrvWizPanel::EPanTypes::Prompt
        , kCQCTreeBrws::ridDlg_DrvWListPr
        , pcqcdcEdit
    )
    , m_c4PromptInd(c4PromptInd)
    , m_pwndDescr(nullptr)
    , m_pwndList(nullptr)
    , m_strSubKey(L"Selected")
{
    // The formatted list values are in the limits field for this one
    TCQCDriverObjCfg& cqcdcSrc = cqcdcEdit();
    const TCQCDCfgPrompt& cqcdcpCur = cqcdcSrc.cqcdcpAt(m_c4PromptInd);
    m_strListValues = cqcdcpCur.strLimits();
}

TDWListPanel::~TDWListPanel()
{
}

// ---------------------------------------------------------------------------
//  TDWListPanel: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TDWListPanel::bValidate(TCQCDrvWizDlg&          wndParent
                        , tCIDCtrls::TWndId&    widFailed
                        , TString&              strErrText)
{
    TCQCDriverObjCfg& cqcdcSrc = cqcdcEdit();
    const TCQCDCfgPrompt& cqcdcpCur = cqcdcSrc.cqcdcpAt(m_c4PromptInd);

    //
    //  Just store away the value. If nothing in the list, then we store an empty
    //  value.
    //
    TString strVal;
    const tCIDLib::TCard4 c4Sel = m_pwndList->c4CurItem();
    if (c4Sel != kCIDLib::c4MaxCard)
        m_pwndList->ValueAt(c4Sel, strVal);

    cqcdcSrc.SetPromptVal(cqcdcpCur.strName(), m_strSubKey, strVal);
    return kCIDLib::True;
}


tCIDLib::TVoid
TDWListPanel::Initialize(TCQCDrvWizDlg& wndParent, const tCIDLib::TStrList&)
{
    // Get some pointers to the ones we want to interact with
    CastChildWnd(*this, kCQCTreeBrws::ridDlg_DrvWListPr_Descr, m_pwndDescr);
    CastChildWnd(*this, kCQCTreeBrws::ridDlg_DrvWListPr_List, m_pwndList);

    TCQCDriverObjCfg& cqcdcSrc = cqcdcEdit();
    const TCQCDCfgPrompt& cqcdcpCur = cqcdcSrc.cqcdcpAt(m_c4PromptInd);

    // The descriptive text for prompts is dynamic
    m_pwndDescr->strWndText(cqcdcpCur.strDescrText());

    // Load the list with the available values and select any current value
    TString strVal;
    cqcdcEdit().bFindPromptValue(cqcdcpCur.strName(), m_strSubKey, strVal);

    LoadListItems(strVal);
}


// ---------------------------------------------------------------------------
//  TDWListPanel: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TDWListPanel::LoadListItems(const TString& strCurVal)
{
    TStringTokenizer stokValues(&m_strListValues, L",");

    TString strTok;
    m_pwndList->RemoveAll();
    while(stokValues.bGetNextToken(strTok))
    {
        strTok.StripWhitespace();
        m_pwndList->c4AddItem(strTok);
    }

    if (m_pwndList->c4ItemCount())
    {
        tCIDLib::TCard4 c4At;
        if (!m_pwndList->bSelectByText(strCurVal, c4At))
        {
            // Current value isn't available, take the 0th as a default
            TCQCDriverObjCfg& cqcdcSrc = cqcdcEdit();
            const TCQCDCfgPrompt& cqcdcpCur = cqcdcSrc.cqcdcpAt(m_c4PromptInd);

            m_pwndList->SelectByIndex(0);
            m_pwndList->ValueAt(0, strTok);
            cqcdcSrc.SetPromptVal(cqcdcpCur.strName(), m_strSubKey, strTok);
        }
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TDWMListPanel
//  PREFIX: pan
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TDWMListPanel: Constructors and Destructor
// ---------------------------------------------------------------------------
TDWMListPanel::TDWMListPanel(       TCQCDriverObjCfg* const pcqcdcEdit
                            , const tCIDLib::TCard4         c4PromptInd) :

    TDrvWizPanel
    (
        TString(L"Option ") + TString(TCardinal(c4PromptInd + 1))
        , TDrvWizPanel::EPanTypes::Prompt
        , kCQCTreeBrws::ridDlg_DrvWMListPr
        , pcqcdcEdit
    )
    , m_c4PromptInd(c4PromptInd)
    , m_pwndDescr(nullptr)
    , m_pwndList(nullptr)
    , m_strSubKey(L"Selections")
{
}

TDWMListPanel::~TDWMListPanel()
{
}

// ---------------------------------------------------------------------------
//  TDWMListPanel: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TDWMListPanel::bValidate(TCQCDrvWizDlg&         wndParent
                        , tCIDCtrls::TWndId&    widFailed
                        , TString&              strErrText)
{
    TCQCDriverObjCfg& cqcdcSrc = cqcdcEdit();
    const TCQCDCfgPrompt& cqcdcpCur = cqcdcSrc.cqcdcpAt(m_c4PromptInd);

    //
    //  Just store away the value. We just get all of the selected items, query
    //  their text, and format them out.
    //
    TString strVal;
    TString strCur;
    tCIDLib::TCard4 c4Count = m_pwndList->c4ItemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (m_pwndList->bIsCheckedAt(c4Index))
        {
            if (!strVal.bIsEmpty())
                strVal.Append(L",");

            m_pwndList->QueryColText(c4Index, 1, strCur);
            strVal.Append(strCur);
        }
    }

    cqcdcSrc.SetPromptVal(cqcdcpCur.strName(), m_strSubKey, strVal);
    return kCIDLib::True;
}


tCIDLib::TVoid
TDWMListPanel::Initialize(TCQCDrvWizDlg& wndParent, const tCIDLib::TStrList&)
{
    // Get some pointers to the ones we want to interact with
    CastChildWnd(*this, kCQCTreeBrws::ridDlg_DrvWMListPr_Descr, m_pwndDescr);
    CastChildWnd(*this, kCQCTreeBrws::ridDlg_DrvWMListPr_List, m_pwndList);

    //
    //  Set the the column titles. We hide the titles but we have to set the number
    //  of columns. Indicate that we want column 1 to be the search column.
    //
    tCIDLib::TStrList colCols(2);
    colCols.objAdd(TString::strEmpty());
    colCols.objAdd(TString::strEmpty());
    m_pwndList->SetColumns(colCols, 1);

    TCQCDriverObjCfg& cqcdcSrc = cqcdcEdit();
    const TCQCDCfgPrompt& cqcdcpCur = cqcdcSrc.cqcdcpAt(m_c4PromptInd);

    // The descriptive text for prompts is dynamic
    m_pwndDescr->strWndText(cqcdcpCur.strDescrText());

    // Load the list with the available values and select any current value
    TString strVal;
    cqcdcEdit().bFindPromptValue(cqcdcpCur.strName(), m_strSubKey, strVal);
    LoadListItems(strVal, cqcdcpCur.strLimits());
}


// ---------------------------------------------------------------------------
//  TDWMListPanel: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TDWMListPanel::LoadListItems(const TString& strCurVals, const TString& strListVals)
{
    tCIDLib::TStrList colVals(2);
    colVals.objAdd(TString::strEmpty());
    colVals.objAdd(TString::strEmpty());

    //
    //  Load up the available values. Initially not trying to figure out which ones
    //  are selected
    //
    TStringTokenizer stokValues(&strListVals, L",");
    TString strTok;
    m_pwndList->RemoveAll();
    while(stokValues.bGetNextToken(strTok))
    {
        strTok.StripWhitespace();
        colVals[1] = strTok;
        const tCIDLib::TCard4 c4At = m_pwndList->c4AddItem(colVals, 0);
        m_pwndList->SetCheckAt(c4At, kCIDLib::False);
    }

    //
    //  Now go back and parse out the selected values and looking each one up, setting
    //  the check column for the ones in the list.
    //
    stokValues.Reset(&strCurVals, L",");
    while(stokValues.bGetNextToken(strTok))
    {
        strTok.StripWhitespace();
        const tCIDLib::TCard4 c4Index = m_pwndList->c4FindByText(strTok);
        if (c4Index != kCIDLib::c4MaxCard)
            m_pwndList->SetCheckAt(c4Index, kCIDLib::True);
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TDWMonikerPanel
//  PREFIX: pan
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TDWMonikerPanel: Constructors and Destructor
// ---------------------------------------------------------------------------
TDWMonikerPanel::TDWMonikerPanel(       TCQCDriverObjCfg* const pcqcdcEdit
                                , const tCIDLib::TBoolean       bReconfigMode
                                , const tCIDLib::TStrList&      colCurMons) :


    TDrvWizPanel
    (
        L"Moniker"
        , TDrvWizPanel::EPanTypes::Moniker
        , kCQCTreeBrws::ridDlg_DrvWMoniker
        , pcqcdcEdit
    )
    , m_bReconfigMode(bReconfigMode)
    , m_pcolCurMons(&colCurMons)
    , m_pwndMoniker(nullptr)
    , m_strOrgMon(pcqcdcEdit->strMoniker())
{
}

TDWMonikerPanel::~TDWMonikerPanel()
{
}


// ---------------------------------------------------------------------------
//  TDWMonikerPanel: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TDWMonikerPanel::bValidate( TCQCDrvWizDlg&          wndParent
                            , tCIDCtrls::TWndId&    widFailed
                            , TString&              strErrText)
{
    TCQCDriverObjCfg& cqcdcSrc = cqcdcEdit();

    // We have to have a moniker
    widFailed = m_pwndMoniker->widThis();
    const TString& strMon = m_pwndMoniker->strWndText();
    if (strMon.bIsEmpty())
    {
        strErrText.LoadFromMsg(kTBrwsErrs::errcDrvI_NoMoniker, facCQCTreeBrws());
        return kCIDLib::False;
    }

    // Make sure it's not a dup if it's changed
    if (!strMon.bCompareI(m_strOrgMon))
    {
        const tCIDLib::TCard4 c4Count = m_pcolCurMons->c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            if (strMon.bCompareI(m_pcolCurMons->objAt(c4Index)))
            {
                strErrText.LoadFromMsg
                (
                    kTBrwsErrs::errcDrvI_DupMonikerCheck, facCQCTreeBrws()
                );
                return kCIDLib::False;
            }
        }
    }

    // Make sure it is a valid moniker
    if (!facCQCKit().bIsValidMoniker(strMon))
    {
        strErrText.LoadFromMsg
        (
            kTBrwsErrs::errcDrvI_BadMoniker, facCQCTreeBrws()
        );
        return kCIDLib::False;
    }

    // Looks ok so store it
    cqcdcSrc.strMoniker(strMon);
    return kCIDLib::True;
}


tCIDLib::TVoid
TDWMonikerPanel::Initialize(        TCQCDrvWizDlg&      wndParent
                            , const tCIDLib::TStrList&  colPorts)
{
    // Get some pointers to the ones we want to interact with
    CastChildWnd(*this, kCQCTreeBrws::ridDlg_DrvWMon_Moniker, m_pwndMoniker);

    // Load up the incoming moniker
    m_pwndMoniker->strWndText(cqcdcEdit().strMoniker());

    // If in reconfigure mode, make it read only
    m_pwndMoniker->bReadOnly(m_bReconfigMode);
}



// ---------------------------------------------------------------------------
//   CLASS: TDWMediaRepoPanel
//  PREFIX: pan
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TDWMediaRepoPanel: Constructors and Destructor
// ---------------------------------------------------------------------------
TDWMediaRepoPanel::TDWMediaRepoPanel(       TCQCDriverObjCfg* const pcqcdcEdit
                                    , const tCIDLib::TCard4         c4PromptInd) :
    TDrvWizPanel
    (
        TString(L"Option ") + TString(TCardinal(c4PromptInd + 1))
        , TDrvWizPanel::EPanTypes::Prompt
        , kCQCTreeBrws::ridDlg_DrvWMRepoPr
        , pcqcdcEdit
    )
    , m_c4PromptInd(c4PromptInd)
    , m_pwndDescr(nullptr)
    , m_pwndList(nullptr)
    , m_strSubKey(L"Moniker")
{
}

TDWMediaRepoPanel::~TDWMediaRepoPanel()
{
}

// ---------------------------------------------------------------------------
//  TDWMediaRepoPanel: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TDWMediaRepoPanel::bValidate(TCQCDrvWizDlg&         wndParent
                            , tCIDCtrls::TWndId&    widFailed
                            , TString&              strErrText)
{
    TCQCDriverObjCfg& cqcdcSrc = cqcdcEdit();
    const TCQCDCfgPrompt& cqcdcpCur = cqcdcSrc.cqcdcpAt(m_c4PromptInd);

    // There has to be a selection in the combo box
    const tCIDLib::TCard4 c4Sel = m_pwndList->c4CurItem();
    if (c4Sel == kCIDLib::c4MaxCard)
    {
        widFailed = m_pwndList->widThis();
        strErrText.LoadFromMsg(kTBrwsErrs::errcDrvI_NoMediaRepo, facCQCTreeBrws());
        return kCIDLib::False;
    }

    TString strVal;
    m_pwndList->ValueAt(c4Sel, strVal);
    cqcdcSrc.SetPromptVal(cqcdcpCur.strName(), m_strSubKey, strVal);
    return kCIDLib::True;
}


tCIDLib::TVoid
TDWMediaRepoPanel::Initialize(TCQCDrvWizDlg& wndParent, const tCIDLib::TStrList&)
{
    // Get some pointers to the ones we want to interact with
    CastChildWnd(*this, kCQCTreeBrws::ridDlg_DrvWMRepoPr_Descr, m_pwndDescr);
    CastChildWnd(*this, kCQCTreeBrws::ridDlg_DrvWMRepoPr_List, m_pwndList);

    TCQCDriverObjCfg& cqcdcSrc = cqcdcEdit();
    const TCQCDCfgPrompt& cqcdcpCur = cqcdcSrc.cqcdcpAt(m_c4PromptInd);

    // The descriptive text for prompts is dynamic
    m_pwndDescr->strWndText(cqcdcpCur.strDescrText());

    // Load the list with the available media repositories
    tCQCKit::TDevCatSet btsCats;
    btsCats.bSetBitState(tCQCKit::EDevCats::MediaRepository, kCIDLib::True);
    btsCats.bSetBitState(tCQCKit::EDevCats::MediaCombo, kCIDLib::True);
    tCIDLib::TStrList colMakes(1);
    tCIDLib::TStrList colModels(1);
    tCIDLib::TStrList colMonikers(1);

    const tCIDLib::TBoolean bGotAny = facCQCKit().bFindDrivers
    (
        TString::strEmpty()
        , TString::strEmpty()
        , TString::strEmpty()
        , btsCats
        , colMonikers
        , colMakes
        , colModels
    );

    // If we found any, load them
    if (bGotAny)
    {
        const tCIDLib::TCard4 c4DrvCount = colMonikers.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4DrvCount; c4Index++)
            m_pwndList->c4AddItem(colMonikers[c4Index]);

        // And select any incoming one
        TString strVal;
        if (cqcdcEdit().bFindPromptValue(cqcdcpCur.strName(), m_strSubKey, strVal)
        &&  !strVal.bIsEmpty())
        {
            tCIDLib::TCard4 c4At;
            m_pwndList->bSelectByText(strVal, c4At);
        }
    }
}





// ---------------------------------------------------------------------------
//   CLASS: TDWNamedListPanel
//  PREFIX: pan
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TDWNamedListPanel: Constructors and Destructor
// ---------------------------------------------------------------------------
TDWNamedListPanel::TDWNamedListPanel(       TCQCDriverObjCfg* const pcqcdcEdit
                                    , const tCIDLib::TCard4         c4PromptInd) :

    TDrvWizPanel
    (
        TString(L"Option ") + TString(TCardinal(c4PromptInd + 1))
        , TDrvWizPanel::EPanTypes::Prompt
        , kCQCTreeBrws::ridDlg_DrvWNListPr
        , pcqcdcEdit
    )
    , m_bSpacesOK(kCIDLib::False)
    , m_c4Count(0)
    , m_c4PromptInd(c4PromptInd)
    , m_pwndDescr(nullptr)
    , m_pwndList(nullptr)
    , m_strSubKey(L"Selections")
{
    // Store away some info we want conveniently available from the prompt
    TCQCDriverObjCfg& cqcdcSrc = cqcdcEdit();
    const TCQCDCfgPrompt& cqcdcpCur = cqcdcSrc.cqcdcpAt(m_c4PromptInd);

    m_bSpacesOK = cqcdcpCur.bFlag();
    m_c4Count = cqcdcpCur.strLimits().c4Val(tCIDLib::ERadices::Dec);
}

TDWNamedListPanel::~TDWNamedListPanel()
{
}

// ---------------------------------------------------------------------------
//  TDWNamedListPanel: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  We override this and install ourself as the IPE interface. The MC list box will
//  call this to ask if it's ok to edit a column. We allow editing of the value column.
//  Since we are installed as the 'auto-IPE' object, we always deal with raw string
//  values. We set the id to indicate whether spaces are valid or not, which is used
//  by the validation callback below.
//
tCIDLib::TBoolean
TDWNamedListPanel::bIPETestCanEdit( const   tCIDCtrls::TWndId       widSrc
                                    , const tCIDLib::TCard4         c4ColInd
                                    ,       TAttrData&              adatFill
                                    ,       tCIDLib::TBoolean&      bValueSet)
{
    if (c4ColInd == 1)
    {
        adatFill.Set
        (
            L"NamedListValue"
            , m_bSpacesOK ? L"/Value" : L"/ValueNoSpace"
            , tCIDMData::EAttrTypes::String
            , tCIDMData::EAttrEdTypes::Both
        );

        // Indicate we are providing an incoming value (the current if any)
        bValueSet = kCIDLib::True;

        TString strVal;
        m_pwndList->QueryColText(c4IPERow(), c4ColInd, strVal);
        adatFill.SetString(strVal);

        return kCIDLib::True;
    }
    return kCIDLib::False;
}


//
//  The IPE object will call this to validate entered values. In our case, we first
//  call the parent class for basic validation. If that works, we then enforce the
//  'spaces ok' option of this prompt, if needed.
//
tCIDLib::TBoolean
TDWNamedListPanel::bIPEValidate(const   TString&            strSrc
                                ,       TAttrData&          adatVal
                                , const TString&            strNewVal
                                ,       TString&            strErrMsg
                                ,       tCIDLib::TCard8&    c8UserId) const
{
    if (!MIPEIntf::bIPEValidate(strSrc, adatVal, strNewVal, strErrMsg, c8UserId))
        return kCIDLib::False;

    // If it's our guy and it's not empty, validate it
    if ((adatVal.strId() == L"/ValueNoSpace") && !strNewVal.bIsEmpty())
    {
        if (!m_bSpacesOK && strNewVal.bContainsChar(kCIDLib::chSpace))
        {
            strErrMsg = L"This value cannot contain any spaces";
            return kCIDLib::False;
        }
    }
    return kCIDLib::True;
}


//
//  Validate our tab's info and store it away.
//
tCIDLib::TBoolean
TDWNamedListPanel::bValidate(TCQCDrvWizDlg&         wndParent
                            , tCIDCtrls::TWndId&    widFailed
                            , TString&              strErrText)
{
    TString strCur;
    TString strVal;

    for (tCIDLib::TCard4 c4Index = 0; c4Index < m_c4Count; c4Index++)
    {
        // Get the value field and add it
        m_pwndList->QueryColText(c4Index, 1, strCur);

        // Remove leading/trailing whitespace
        strCur.StripWhitespace();
        TStringTokenizer::BuildQuotedCommaList(strCur, strVal);
    }

    // If we got no values, make sure they want to do it
    if (strVal.bIsEmpty())
    {
        TYesNoBox msgbEmpty(facCQCTreeBrws().strMsg(kTBrwsMsgs::midQ_NoSelections));
        if (!msgbEmpty.bShowIt(*this))
        {
            strErrText.Clear();
            return kCIDLib::False;
        }
    }

    TCQCDriverObjCfg& cqcdcSrc = cqcdcEdit();
    const TCQCDCfgPrompt& cqcdcpCur = cqcdcSrc.cqcdcpAt(m_c4PromptInd);
    cqcdcEdit().SetPromptVal(cqcdcpCur.strName(), m_strSubKey, strVal);
    return kCIDLib::True;
}


tCIDLib::TVoid
TDWNamedListPanel::Initialize(TCQCDrvWizDlg& wndParent, const tCIDLib::TStrList&)
{
    // Get some pointers to the ones we want to interact with
    CastChildWnd(*this, kCQCTreeBrws::ridDlg_DrvWNListPr_Descr, m_pwndDescr);
    CastChildWnd(*this, kCQCTreeBrws::ridDlg_DrvWNListPr_List, m_pwndList);

    // Set ourself as the IPE interface on the list window
    m_pwndList->SetAutoIPE(this);

    //
    //  Set the the column titles. We hide the titles but we have to set the
    //  number of columns.
    //
    tCIDLib::TStrList colCols(2);
    colCols.objAdd(TString::strEmpty());
    colCols.objAdd(TString::strEmpty());
    m_pwndList->SetColumns(colCols);

    TCQCDriverObjCfg& cqcdcSrc = cqcdcEdit();
    const TCQCDCfgPrompt& cqcdcpCur = cqcdcSrc.cqcdcpAt(m_c4PromptInd);

    // The descriptive text for prompts is dynamic
    m_pwndDescr->strWndText(cqcdcpCur.strDescrText());

    // Load the list with the available values
    TString strVal;
    cqcdcEdit().bFindPromptValue(cqcdcpCur.strName(), m_strSubKey, strVal);
    LoadListItems(strVal);
}


// ---------------------------------------------------------------------------
//  TDWNamedListPanel: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TDWNamedListPanel::LoadListItems(const TString& strCurVals)
{
    tCIDLib::TStrList colValList;
    if (strCurVals.bIsEmpty())
    {
        // If not, then just start with empty values
        for (tCIDLib::TCard4 c4Index = 0; c4Index < m_c4Count; c4Index++)
            colValList.objAdd(TString::strEmpty());
    }
     else
    {
        // We have a value so parse it. If bad, start with all empty values
        tCIDLib::TCard4 c4ErrInd;
        if (!TStringTokenizer::bParseQuotedCommaList(strCurVals, colValList, c4ErrInd))
        {
            for (tCIDLib::TCard4 c4Index = 0; c4Index < m_c4Count; c4Index++)
                colValList.objAdd(TString::strEmpty());
        }
    }

    // Make sure we got the expected number of values. If not, then reset
    if (colValList.c4ElemCount() != m_c4Count)
    {
        colValList.RemoveAll();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < m_c4Count; c4Index++)
            colValList.objAdd(TString::strEmpty());
    }

    // Load them up
    tCIDLib::TStrList colCols(2);
    colCols.objAdd(TString::strEmpty());
    colCols.objAdd(TString::strEmpty());

    TString strNum;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < m_c4Count; c4Index++)
    {
        strNum.SetFormatted(c4Index + 1);
        strNum.Append(kCIDLib::chPeriod);
        colCols[0] = strNum;
        colCols[1] = colValList[c4Index];
        m_pwndList->c4AddItem(colCols, c4Index);
    }

    // Size the first column to fit the numbers
    m_pwndList->AutoSizeCol(0, kCIDLib::False);
}


// ---------------------------------------------------------------------------
//   CLASS: TDWODBCPanel
//  PREFIX: pan
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TDWODBCPanel: Constructors and Destructor
// ---------------------------------------------------------------------------
TDWODBCPanel::TDWODBCPanel(         TCQCDriverObjCfg* const pcqcdcEdit
                            , const tCIDLib::TCard4         c4PromptInd) :

    TDrvWizPanel
    (
        TString(L"Option ") + TString(TCardinal(c4PromptInd + 1))
        , TDrvWizPanel::EPanTypes::Prompt
        , kCQCTreeBrws::ridDlg_DrvWODBCPr
        , pcqcdcEdit
    )
    , m_c4PromptInd(c4PromptInd)
    , m_pwndDescr(nullptr)
    , m_pwndList(nullptr)
    , m_strSubKey(L"SrcName")
{
}

TDWODBCPanel::~TDWODBCPanel()
{
}

// ---------------------------------------------------------------------------
//  TDWODBCPanel: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TDWODBCPanel::bValidate(TCQCDrvWizDlg&          wndParent
                        , tCIDCtrls::TWndId&    widFailed
                        , TString&              strErrText)
{
    TCQCDriverObjCfg& cqcdcSrc = cqcdcEdit();
    const TCQCDCfgPrompt& cqcdcpCur = cqcdcSrc.cqcdcpAt(m_c4PromptInd);

    // Make sure we have a selection
    const tCIDLib::TCard4 c4Sel = m_pwndList->c4CurItem();
    if (c4Sel == kCIDLib::c4MaxCard)
    {
        widFailed = m_pwndList->widThis();
        strErrText.LoadFromMsg(kTBrwsErrs::errcDrvI_NoODBCSrc, facCQCTreeBrws());
        return kCIDLib::False;
    }

    // The item data holds the original list index
    const TKeyValuePair& kvalSel = m_colSrcs[m_pwndList->c4IndexToId(c4Sel)];
    cqcdcSrc.SetPromptVal(cqcdcpCur.strName(), m_strSubKey, kvalSel.strKey());
    return kCIDLib::True;
}


tCIDLib::TVoid
TDWODBCPanel::Initialize(TCQCDrvWizDlg& wndParent, const tCIDLib::TStrList&)
{
    // Get some pointers to the ones we want to interact with
    CastChildWnd(*this, kCQCTreeBrws::ridDlg_DrvWODBCPr_Descr, m_pwndDescr);
    CastChildWnd(*this, kCQCTreeBrws::ridDlg_DrvWODBCPr_SrcList, m_pwndList);

    TCQCDriverObjCfg& cqcdcSrc = cqcdcEdit();
    const TCQCDCfgPrompt& cqcdcpCur = cqcdcSrc.cqcdcpAt(m_c4PromptInd);

    // The descriptive text for prompts is dynamic
    m_pwndDescr->strWndText(cqcdcpCur.strDescrText());

    // Load the list with the available values and select any current value
    TString strVal;
    if (cqcdcEdit().bFindPromptValue(cqcdcpCur.strName(), m_strSubKey, strVal))
        LoadListItems(strVal);
}


// ---------------------------------------------------------------------------
//  TDWODBCPanel: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TDWODBCPanel::LoadListItems(const TString& strCurVal)
{
    tCIDLib::TStrList colSrcs;
    try
    {
        //
        //  He deals correctly with multiple init calls, so no worries if we
        //  have more than one such prompt in the same driver config.
        //
        facCIDDBase().Initialize();
        facCIDDBase().QuerySources(m_colSrcs, tCIDDBase::ESrcTypes::Both);
    }

    catch(TError& errToCatch)
    {
        if (facCQCTreeBrws().bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
    }

    // Load the list with any sources we got
    const tCIDLib::TCard4 c4Count = m_colSrcs.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TKeyValuePair& kvalCur = m_colSrcs[c4Index];
        m_pwndList->c4AddItem(kvalCur.strKey(), c4Index);
    }

    // Select the incoming value if any, or the zeroth one if no match
    tCIDLib::TCard4 c4SelInd = 0;
    if (!m_pwndList->bSelectByText(strCurVal, c4SelInd))
    {
        if (c4Count)
            m_pwndList->SelectByIndex(0);
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TDWRangePanel
//  PREFIX: pan
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TDWRangePanel: Constructors and Destructor
// ---------------------------------------------------------------------------
TDWRangePanel::TDWRangePanel(       TCQCDriverObjCfg* const pcqcdcEdit
                            , const tCIDLib::TCard4         c4PromptInd) :


    TDrvWizPanel
    (
        TString(L"Option ") + TString(TCardinal(c4PromptInd + 1))
        , TDrvWizPanel::EPanTypes::Prompt
        , kCQCTreeBrws::ridDlg_DrvWRangePr
        , pcqcdcEdit
    )
    , m_c4PromptInd(c4PromptInd)
    , m_i4Max(1)
    , m_i4Min(0)
    , m_pwndDescr(nullptr)
    , m_pwndValue(nullptr)
    , m_strSubKey(L"Value")
{
    // Get out the min/max values
    TCQCDriverObjCfg& cqcdcSrc = cqcdcEdit();
    const TCQCDCfgPrompt& cqcdcpCur = cqcdcSrc.cqcdcpAt(m_c4PromptInd);

    m_i4Max = cqcdcpCur.i4MaxValue();
    m_i4Min = cqcdcpCur.i4MinValue();
}

TDWRangePanel::~TDWRangePanel()
{
}


// ---------------------------------------------------------------------------
//  TDWRangePanel: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TDWRangePanel::bValidate(TCQCDrvWizDlg&         wndParent
                        , tCIDCtrls::TWndId&    widFailed
                        , TString&              strErrText)
{
    // We have to have a moniker
    widFailed = m_pwndValue->widThis();
    TString strVal = m_pwndValue->strWndText();
    tCIDLib::TInt4 i4Val;
    if (!strVal.bToInt4(i4Val, tCIDLib::ERadices::Dec))
    {
        strErrText.LoadFromMsg(kTBrwsErrs::errcDrvI_InvalidNum, facCQCTreeBrws(), strVal);
        return kCIDLib::False;
    }

    if ((i4Val < m_i4Min) || (i4Val > m_i4Max))
    {
        strErrText.LoadFromMsg
        (
            kTBrwsErrs::errcDrvI_Range
            , facCQCTreeBrws()
            , strVal
            , TInteger(m_i4Min)
            , TInteger(m_i4Max)
        );
        return kCIDLib::False;
    }

    // Looks ok so store it. Format it back out for canonical format
    TCQCDriverObjCfg& cqcdcSrc = cqcdcEdit();
    const TCQCDCfgPrompt& cqcdcpCur = cqcdcSrc.cqcdcpAt(m_c4PromptInd);

    strVal.SetFormatted(i4Val);
    cqcdcEdit().SetPromptVal(cqcdcpCur.strName(), m_strSubKey, strVal);

    return kCIDLib::True;
}


tCIDLib::TVoid
TDWRangePanel::Initialize(        TCQCDrvWizDlg&      wndParent
                            , const tCIDLib::TStrList&  colPorts)
{
    // Get some pointers to the ones we want to interact with
    CastChildWnd(*this, kCQCTreeBrws::ridDlg_DrvWRangePr_Descr, m_pwndDescr);
    CastChildWnd(*this, kCQCTreeBrws::ridDlg_DrvWRangePr_Value, m_pwndValue);

    TCQCDriverObjCfg& cqcdcSrc = cqcdcEdit();
    const TCQCDCfgPrompt& cqcdcpCur = cqcdcSrc.cqcdcpAt(m_c4PromptInd);

    // The descriptive text for prompts is dynamic
    m_pwndDescr->strWndText(cqcdcpCur.strDescrText());

    // Load up the incoming value
    TString strVal;
    cqcdcEdit().bFindPromptValue(cqcdcpCur.strName(), m_strSubKey, strVal);
    m_pwndValue->strWndText(strVal);
}




// ---------------------------------------------------------------------------
//   CLASS: TDWSerialPanel
//  PREFIX: pan
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TDWSerialPanel: Constructors and Destructor
// ---------------------------------------------------------------------------
TDWSerialPanel::TDWSerialPanel(TCQCDriverObjCfg* const pcqcdcEdit) :


    TDrvWizPanel
    (
        L"Serial Conn"
        , TDrvWizPanel::EPanTypes::CommCfg
        , kCQCTreeBrws::ridDlg_DrvWSerial
        , pcqcdcEdit
    )
    , m_pwndEdit(nullptr)
    , m_pwndPortList(nullptr)
{
}

TDWSerialPanel::~TDWSerialPanel()
{
}

// ---------------------------------------------------------------------------
//  TDWSerialPanel: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TDWSerialPanel::bValidate(TCQCDrvWizDlg&          wndParent
                        , tCIDCtrls::TWndId&    widFailed
                        , TString&              strErrText)
{
    TCQCDriverObjCfg& cqcdcSrc = cqcdcEdit();

    // Get the config object and update it
    TCQCSerialConnCfg& conncfgSer = static_cast<TCQCSerialConnCfg&>
    (
        cqcdcEdit().conncfgReal()
    );

    // Make sure a port is selected
    if (conncfgSer.strPortPath().bIsEmpty())
    {
        strErrText.LoadFromMsg(kTBrwsErrs::errcDrvI_NoPortSel, facCQCTreeBrws());
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


tCIDLib::TVoid
TDWSerialPanel::Initialize(         TCQCDrvWizDlg&      wndParent
                            , const tCIDLib::TStrList&  colPorts)
{
    // Get some pointers to the ones we want to interact with
    CastChildWnd(*this, kCQCTreeBrws::ridDlg_DrvWSer_Settings, m_pwndEdit);
    CastChildWnd(*this, kCQCTreeBrws::ridDlg_DrvWSer_List, m_pwndPortList);

    // Load up the available ports
    const tCIDLib::TCard4 c4Count = colPorts.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        m_pwndPortList->c4AddItem(colPorts[c4Index]);

    // Register our handlers
    m_pwndEdit->pnothRegisterHandler(this, &TDWSerialPanel::eClickHandler);
    m_pwndPortList->pnothRegisterHandler(this, &TDWSerialPanel::eLBHandler);


    // Load up the incoming info
    TCQCSerialConnCfg& conncfgSer = static_cast<TCQCSerialConnCfg&>(cqcdcEdit().conncfgReal());

    tCIDLib::TCard4 c4At;
    if (!m_pwndPortList->bSelectByText(conncfgSer.strPortPath(), c4At, kCIDLib::True))
    {
        // Force an event if we do this, so that we see this as the initial selection
        if (m_pwndPortList->c4ItemCount())
            m_pwndPortList->SelectByIndex(0, kCIDLib::True);
    }

    // If the conn config is marked as not changeable, then disable the edit button
    if (!cqcdcEdit().bEditableConn())
        m_pwndEdit->SetEnable(kCIDLib::False);
}


// ---------------------------------------------------------------------------
//  TDWSerialPanel: Private, non-virtual methods
// ---------------------------------------------------------------------------

tCIDCtrls::EEvResponses TDWSerialPanel::eClickHandler(TButtClickInfo&)
{
    //
    //  Get a copy of the config settings and pass that to the port config dialog.
    //  If the commit changes, copy them back.
    //
    TCQCDriverObjCfg& cqcdcSrc = cqcdcEdit();
    TCQCSerialConnCfg& conncfgSer = static_cast<TCQCSerialConnCfg&>
    (
        cqcdcSrc.conncfgReal()
    );

    TCommPortCfg cpcfgTmp(conncfgSer.cpcfgSerial());
    if (facCIDWUtils().bCfgSerialPort(*this, cpcfgTmp))
        conncfgSer.cpcfgSerial(cpcfgTmp);

    return tCIDCtrls::EEvResponses::Handled;
}


tCIDCtrls::EEvResponses TDWSerialPanel::eLBHandler(TListChangeInfo& wnotInfo)
{
    if (wnotInfo.eEvent() == tCIDCtrls::EListEvents::SelChanged)
    {
        // Get the serial config and update the port
        TCQCDriverObjCfg& cqcdcSrc = cqcdcEdit();
        TCQCSerialConnCfg& conncfgSer = static_cast<TCQCSerialConnCfg&>
        (
            cqcdcSrc.conncfgReal()
        );

        TString strPort;
        m_pwndPortList->ValueAt(wnotInfo.c4Index(), strPort);
        conncfgSer.strPortPath(strPort);

        //
        //  Update the port info string to tell the user if this port is configurable or
        //  not. We just hide or show it.
        //
        SetVisibility
        (
            kCQCTreeBrws::ridDlg_DrvWSer_Warn, !facCIDComm().bCanConfigure(strPort)
        );
    }

    return tCIDCtrls::EEvResponses::Handled;
}



// ---------------------------------------------------------------------------
//   CLASS: TDWSockPanel
//  PREFIX: pan
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TDWSockPanel: Constructors and Destructor
// ---------------------------------------------------------------------------
TDWSockPanel::TDWSockPanel(TCQCDriverObjCfg* const pcqcdcEdit) :


    TDrvWizPanel
    (
        L"IP Conn"
        , TDrvWizPanel::EPanTypes::CommCfg
        , kCQCTreeBrws::ridDlg_DrvWSock
        , pcqcdcEdit
    )
    , m_pwndAddr(nullptr)
    , m_pwndPort(nullptr)
    , m_pwndProto(nullptr)
{
}

TDWSockPanel::~TDWSockPanel()
{
}


// ---------------------------------------------------------------------------
//  TDWSockPanel: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TDWSockPanel::bValidate(TCQCDrvWizDlg&          wndParent
                        , tCIDCtrls::TWndId&    widFailed
                        , TString&              strErrText)
{
    TCQCDriverObjCfg& cqcdcSrc = cqcdcEdit();

    const TString& strAddr = m_pwndAddr->strWndText();
    if (strAddr.bIsEmpty())
    {
        widFailed = m_pwndAddr->widThis();
        strErrText.LoadFromMsg(kTBrwsErrs::errcDrvI_NoIPAddr, facCQCTreeBrws());
        return kCIDLib::False;
    }

    const TString& strPort = m_pwndPort->strWndText();
    tCIDLib::TIPPortNum ippnVal;
    if (!strPort.bToCard4(ippnVal, tCIDLib::ERadices::Dec))
    {
        widFailed = m_pwndAddr->widThis();
        strErrText.LoadFromMsg(kTBrwsErrs::errcDrvI_BadPort, facCQCTreeBrws());
        return kCIDLib::False;
    }

    try
    {
        TIPEndPoint ipepTest(strAddr, ippnVal, tCIDSock::EAddrTypes::Unspec);
    }

    catch(...)
    {
        widFailed = m_pwndAddr->widThis();
        strErrText.LoadFromMsg(kTBrwsErrs::errcDrvI_BadEndPoint, facCQCTreeBrws());
        return kCIDLib::False;
    }

    // Get the config object and update it
    TCQCIPConnCfg& conncfgEdit = static_cast<TCQCIPConnCfg&>
    (
        cqcdcEdit().conncfgReal()
    );
    conncfgEdit.Set(strAddr, ippnVal);

    return kCIDLib::True;
}


tCIDLib::TVoid
TDWSockPanel::Initialize(       TCQCDrvWizDlg&      wndParent
                        , const tCIDLib::TStrList&  colPorts)
{
    // Get some pointers to the ones we want to interact with
    CastChildWnd(*this, kCQCTreeBrws::ridDlg_DrvWSock_Addr, m_pwndAddr);
    CastChildWnd(*this, kCQCTreeBrws::ridDlg_DrvWSock_Port, m_pwndPort);
    CastChildWnd(*this, kCQCTreeBrws::ridDlg_DrvWSock_Proto, m_pwndProto);

    // Load up the incoming info
    TCQCIPConnCfg& conncfgEdit = static_cast<TCQCIPConnCfg&>
    (
        cqcdcEdit().conncfgReal()
    );

    // Load the address
    m_pwndAddr->strWndText(conncfgEdit.strAddr());

    // Format out the IP port and laod that
    TString strIPPort(TCardinal(conncfgEdit.ippnTarget()));
    m_pwndPort->strWndText(strIPPort);

    // Load up the socket protocols
    tCIDLib::ForEachE<tCIDSock::ESockProtos>
    (
        [this](const tCIDSock::ESockProtos eCur)
        {
            m_pwndProto->c4AddItem(facCIDSock().strXlatProto(eCur));
            return kCIDLib::True;
        }
    );
    m_pwndProto->SelectByIndex(tCIDLib::c4EnumOrd(conncfgEdit.eProto()));

    // We disable the port and protocols if the configuration isn't editable
    m_pwndPort->SetEnable(cqcdcEdit().bEditableConn());
    m_pwndProto->SetEnable(cqcdcEdit().bEditableConn());
}



// ---------------------------------------------------------------------------
//   CLASS: TDWSumPanel
//  PREFIX: pan
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TDWSumPanel: Constructors and Destructor
// ---------------------------------------------------------------------------
TDWSumPanel::TDWSumPanel(TCQCDriverObjCfg* const pcqcdcEdit) :


    TDrvWizPanel
    (
        L"Summary"
        , TDrvWizPanel::EPanTypes::Summary
        , kCQCTreeBrws::ridDlg_DrvWSum
        , pcqcdcEdit
    )
    , m_pwndSummary(nullptr)
{
}

TDWSumPanel::~TDWSumPanel()
{
}


// ---------------------------------------------------------------------------
//  TDWSumPanel: Public, inherited methods
// ---------------------------------------------------------------------------

// Nothing to do in thi scase
tCIDLib::TBoolean TDWSumPanel::bValidate(TCQCDrvWizDlg&, tCIDCtrls::TWndId&, TString&)
{
    return kCIDLib::True;
}


tCIDLib::TVoid
TDWSumPanel::Initialize(TCQCDrvWizDlg& wndParent, const tCIDLib::TStrList&)
{
    // Get a pointer to the summary screen
    CastChildWnd(*this, kCQCTreeBrws::ridDlg_DrvWSum_Info, m_pwndSummary);

    TString strFmt;
    TCQCDriverObjCfg& cqcdcSrc = cqcdcEdit();

    //
    //  We will get called here each time they move to the summary screen, to make
    //  sure we display the right stuff. So we remove all current values.
    //
    m_pwndSummary->RemoveAll();

    //
    //  The general options
    //
    m_pwndSummary->AddSectTitle(L"General Options");
    m_pwndSummary->AddDataRow(L"Moniker", cqcdcSrc.strMoniker());

    //
    // The connection options
    //
    if (cqcdcSrc.conncfgReal().clsIsA() == TCQCSerialConnCfg::clsThis())
    {
        m_pwndSummary->AddSectTitle(L"Serial Options");
        const TCQCSerialConnCfg& conncfgSer
        (
            static_cast<const TCQCSerialConnCfg&>(cqcdcSrc.conncfgReal())
        );
        m_pwndSummary->AddDataRow(L"Port", conncfgSer.strPortPath());

        const TCommPortCfg& cpcfgSer = conncfgSer.cpcfgSerial();

        strFmt.SetFormatted(cpcfgSer.c4Baud());
        m_pwndSummary->AddDataRow(L"Baud", strFmt);
        m_pwndSummary->AddDataRow
        (
            L"Data Bits", tCIDComm::strXlatEDataBits(cpcfgSer.eDataBits())
        );
        m_pwndSummary->AddDataRow
        (
            L"Stop Bits", tCIDComm::strXlatEStopBits(cpcfgSer.eStopBits())
        );
    }
     else if (cqcdcSrc.conncfgReal().clsIsA() == TCQCIPConnCfg::clsThis())
    {
        m_pwndSummary->AddSectTitle(L"IP Options");

        // Don't display the resolved address, show the text info
        const TCQCIPConnCfg& conncfgIP
        (
            static_cast<const TCQCIPConnCfg&>(cqcdcSrc.conncfgReal())
        );
        m_pwndSummary->AddDataRow(L"Address", conncfgIP.strAddr());

        strFmt.SetFormatted(conncfgIP.ippnTarget());
        m_pwndSummary->AddDataRow(L"Port", strFmt);
    }


    // Do all of the prompts
    const tCIDLib::TCard4 c4PromptCnt = cqcdcSrc.c4PromptCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4PromptCnt; c4Index++)
    {
        const TCQCDCfgPrompt& cqcdcpCur = cqcdcSrc.cqcdcpAt(c4Index);
        m_pwndSummary->AddSectTitle(cqcdcpCur.strTitle());

        // Get a cursor for the sub-values
        const TString& strCurKey = cqcdcpCur.strName();

        //
        //  If this key ever got any values set (it may have been added after this driver
        //  instance was installed), then get a cursor for the sub-values and display them.
        //  Else indicate they haven't been set.
        //
        if (cqcdcSrc.nvmPromptVals().bKeyExists(strCurKey))
        {
            TCQCDriverCfg::TValCursor cursVals
            (
                cqcdcSrc.cursPromptSubVals(strCurKey)
            );

            // And display those
            for (; cursVals; ++cursVals)
            {
                const TCQCDriverCfg::TPair& kobjCur = *cursVals;
                m_pwndSummary->AddDataRow(kobjCur.objKey(), kobjCur.objValue());
            }
        }
         else
        {
            // Shouldn't happen, but just in case
            m_pwndSummary->AddDataRow(TString::strEmpty(), L"[NOT AVAILABLE]");
        }
    }

    //
    //  Size the prefix column to fit the prefixes we added. The value column is auto-
    //  sized.
    //
    m_pwndSummary->AutoSizeCol(1, kCIDLib::False);
}



// ---------------------------------------------------------------------------
//   CLASS: TDWTextPanel
//  PREFIX: pan
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TDWTextPanel: Constructors and Destructor
// ---------------------------------------------------------------------------
TDWTextPanel::TDWTextPanel(         TCQCDriverObjCfg* const pcqcdcEdit
                            , const tCIDLib::TCard4         c4PromptInd) :


    TDrvWizPanel
    (
        TString(L"Option ") + TString(TCardinal(c4PromptInd + 1))
        , TDrvWizPanel::EPanTypes::Prompt
        , kCQCTreeBrws::ridDlg_DrvWTextPr
        , pcqcdcEdit
    )
    , m_bGotRegEx(kCIDLib::False)
    , m_c4PromptInd(c4PromptInd)
    , m_pwndDescr(nullptr)
    , m_pwndValue(nullptr)
    , m_strSubKey(L"Text")
{
    TCQCDriverObjCfg& cqcdcSrc = cqcdcEdit();
    const TCQCDCfgPrompt& cqcdcpCur = cqcdcSrc.cqcdcpAt(m_c4PromptInd);

    m_bGotRegEx = !cqcdcpCur.strLimits().bIsEmpty();
    if (m_bGotRegEx)
        m_regxLimit.SetExpression(cqcdcpCur.strLimits());
}

TDWTextPanel::~TDWTextPanel()
{
}

// ---------------------------------------------------------------------------
//  TDWTextPanel: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TDWTextPanel::bValidate(TCQCDrvWizDlg&          wndParent
                            , tCIDCtrls::TWndId&    widFailed
                            , TString&              strErrText)
{
    const TString& strTest = m_pwndValue->strWndText();

    // If we got a regular expression, validate it against that
    tCIDLib::TBoolean bRet = kCIDLib::True;
    if (m_bGotRegEx)
        bRet = m_regxLimit.bFullyMatches(strTest, kCIDLib::True);

    if (!bRet)
        strErrText = facCQCTreeBrws().strMsg(kTBrwsErrs::errcDrvI_BadRegEx);

    // Looks ok so store it
    TCQCDriverObjCfg& cqcdcSrc = cqcdcEdit();
    const TCQCDCfgPrompt& cqcdcpCur = cqcdcSrc.cqcdcpAt(m_c4PromptInd);
    cqcdcSrc.SetPromptVal(cqcdcpCur.strName(), m_strSubKey, strTest);

    return bRet;
}


tCIDLib::TVoid
TDWTextPanel::Initialize(TCQCDrvWizDlg& wndParent, const tCIDLib::TStrList& colPorts)
{
    // Get some pointers to the ones we want to interact with
    CastChildWnd(*this, kCQCTreeBrws::ridDlg_DrvWTextPr_Value, m_pwndValue);
    CastChildWnd(*this, kCQCTreeBrws::ridDlg_DrvWTextPr_Descr, m_pwndDescr);

    TCQCDriverObjCfg& cqcdcSrc = cqcdcEdit();
    const TCQCDCfgPrompt& cqcdcpCur = cqcdcSrc.cqcdcpAt(m_c4PromptInd);

    // The descriptive text for prompts is dynamic
    m_pwndDescr->strWndText(cqcdcpCur.strDescrText());

    // Load up any incoming text
    TString strCurVal;
    cqcdcEdit().bFindPromptValue(cqcdcpCur.strName(), m_strSubKey, strCurVal);
    m_pwndValue->strWndText(strCurVal);
}




// ---------------------------------------------------------------------------
//   CLASS: TDWTTSVoicePanel
//  PREFIX: pan
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TDWTTSVoicePanel: Constructors and Destructor
// ---------------------------------------------------------------------------
TDWTTSVoicePanel::TDWTTSVoicePanel(         TCQCDriverObjCfg* const pcqcdcEdit
                                    , const tCIDLib::TCard4         c4PromptInd) :


    TDrvWizPanel
    (
        TString(L"Option ") + TString(TCardinal(c4PromptInd + 1))
        , TDrvWizPanel::EPanTypes::Prompt
        , kCQCTreeBrws::ridDlg_DrvWTTSPr
        , pcqcdcEdit
    )
    , m_c4PromptInd(c4PromptInd)
    , m_pwndDescr(nullptr)
    , m_pwndList(nullptr)
    , m_strSubKey(L"VoiceName")
{
}

TDWTTSVoicePanel::~TDWTTSVoicePanel()
{
}

// ---------------------------------------------------------------------------
//  TDWTTSVoicePanel: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TDWTTSVoicePanel::bValidate(TCQCDrvWizDlg&          wndParent
                            , tCIDCtrls::TWndId&    widFailed
                            , TString&              strErrText)
{
    TCQCDriverObjCfg& cqcdcSrc = cqcdcEdit();
    const TCQCDCfgPrompt& cqcdcpCur = cqcdcSrc.cqcdcpAt(m_c4PromptInd);

    // Make sure a voice is selected
    if (cqcdcSrc.strPromptValue(cqcdcpCur.strName(), m_strSubKey).bIsEmpty())
    {
        strErrText.LoadFromMsg(kTBrwsErrs::errcDrvI_NoVoice, facCQCTreeBrws());
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


tCIDLib::TVoid
TDWTTSVoicePanel::Initialize(TCQCDrvWizDlg& wndParent, const tCIDLib::TStrList& colPorts)
{
    // Get some pointers to the ones we want to interact with
    CastChildWnd(*this, kCQCTreeBrws::ridDlg_DrvWTTSPr_List, m_pwndList);
    CastChildWnd(*this, kCQCTreeBrws::ridDlg_DrvWTTSPr_Descr, m_pwndDescr);

    TCQCDriverObjCfg& cqcdcSrc = cqcdcEdit();
    const TCQCDCfgPrompt& cqcdcpCur = cqcdcSrc.cqcdcpAt(m_c4PromptInd);

    // The descriptive text for prompts is dynamic
    m_pwndDescr->strWndText(cqcdcpCur.strDescrText());

    // Load up the available voices (on the target machine)
    tCIDLib::TStrList colVoices;
    try
    {
        TCQCSrvAdminClientProxy* porbcProxy
        (
            facCQCKit().porbcCQCSrvAdminProxy(wndParent.strHost())
        );

        if (porbcProxy)
        {
            TJanitor<TCQCSrvAdminClientProxy> janProxy(porbcProxy);
            porbcProxy->c4QueryTTSVoices(colVoices);
        }
    }

    catch(TError& errToCatch)
    {
        if (facCQCTreeBrws().bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
    }

    const tCIDLib::TCard4 c4Count = colVoices.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        m_pwndList->c4AddItem(colVoices[c4Index]);

    // Register our handlers
    m_pwndList->pnothRegisterHandler(this, &TDWTTSVoicePanel::eLBHandler);

    // Select the incoming one, or the 0th one if no match
    TString strCurVal;
    cqcdcEdit().bFindPromptValue(cqcdcpCur.strName(), m_strSubKey, strCurVal);

    tCIDLib::TCard4 c4SelInd = 0;
    if (!m_pwndList->bSelectByText(strCurVal, c4SelInd))
    {
        if (c4Count)
        {
            m_pwndList->SelectByIndex(0);
            cqcdcSrc.SetPromptVal(cqcdcpCur.strName(), m_strSubKey, colVoices[0]);
        }
    }
}


// ---------------------------------------------------------------------------
//  TDWTTSVoicePanel: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDCtrls::EEvResponses TDWTTSVoicePanel::eLBHandler(TListChangeInfo& wnotInfo)
{
    TCQCDriverObjCfg& cqcdcSrc = cqcdcEdit();
    const TCQCDCfgPrompt& cqcdcpCur = cqcdcSrc.cqcdcpAt(m_c4PromptInd);

    if (wnotInfo.eEvent() == tCIDCtrls::EListEvents::SelChanged)
    {
        TString strVoice;
        m_pwndList->ValueAt(wnotInfo.c4Index(), strVoice);
        cqcdcSrc.SetPromptVal(cqcdcpCur.strName(), m_strSubKey, strVoice);
    }
     else if (wnotInfo.eEvent() == tCIDCtrls::EListEvents::Cleared)
    {
        cqcdcSrc.SetPromptVal(cqcdcpCur.strName(), m_strSubKey, TString::strEmpty());
    }
    return tCIDCtrls::EEvResponses::Handled;
}



// ---------------------------------------------------------------------------
//   CLASS: TDWUPnPCommPanel
//  PREFIX: pan
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TDWUPnPCommPanel: Constructors and Destructor
// ---------------------------------------------------------------------------
TDWUPnPCommPanel::TDWUPnPCommPanel(TCQCDriverObjCfg* const pcqcdcEdit) :


    TDrvWizPanel
    (
        L"UPnPComm Conn"
        , TDrvWizPanel::EPanTypes::CommCfg
        , kCQCTreeBrws::ridDlg_DrvWUPnP
        , pcqcdcEdit
    )
    , m_pwndCurDev(nullptr)
    , m_pwndDescr(nullptr)
    , m_pwndSearch(nullptr)
    , m_pwndList(nullptr)
{
}

TDWUPnPCommPanel::~TDWUPnPCommPanel()
{
}

// ---------------------------------------------------------------------------
//  TDWUPnPCommPanel: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TDWUPnPCommPanel::bValidate(TCQCDrvWizDlg&          wndParent
                            , tCIDCtrls::TWndId&    widFailed
                            , TString&              strErrText)
{
    TCQCDriverObjCfg& cqcdcSrc = cqcdcEdit();
    TCQCUPnPConnCfg& conncfgEdit = static_cast<TCQCUPnPConnCfg&>
    (
        cqcdcSrc.conncfgReal()
    );

    if (conncfgEdit.strDevUID().bIsEmpty())
    {
        strErrText = L"You must select a UPnP device";
        widFailed = m_pwndList->widThis();
        return kCIDLib::False;
    }

    // See if it's a valid device
    try
    {
        TUPnPDevice upnpdTest;
        facCIDUPnP().SetDeviceFromUID(conncfgEdit.strDevUID(), upnpdTest);
    }

    catch(...)
    {
        strErrText = L"Could not find such a UPnP device";
        widFailed = m_pwndList->widThis();
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


tCIDLib::TVoid
TDWUPnPCommPanel::Initialize(       TCQCDrvWizDlg&      wndParent
                            , const tCIDLib::TStrList&  colPorts)
{
    // Get some pointers to the ones we want to interact with
    CastChildWnd(*this, kCQCTreeBrws::ridDlg_DrvWUPnP_CurDev, m_pwndCurDev);
    CastChildWnd(*this, kCQCTreeBrws::ridDlg_DrvWUPnP_Descr, m_pwndDescr);
    CastChildWnd(*this, kCQCTreeBrws::ridDlg_DrvWUPnP_List, m_pwndList);
    CastChildWnd(*this, kCQCTreeBrws::ridDlg_DrvWUPnP_Search, m_pwndSearch);

    TCQCUPnPConnCfg& conncfgEdit = static_cast<TCQCUPnPConnCfg&>(cqcdcEdit().conncfgReal());

    //
    //  Set the description text, which includes the UPnP device type from the
    //  manifest.
    //
    TString strDescr = facCQCTreeBrws().strMsg
    (
        kTBrwsMsgs::midDlg_DrvWUPnP_Descr, conncfgEdit.strTypeDescr()
    );
    m_pwndDescr->strWndText(strDescr);

    // Register our handlers
    m_pwndSearch->pnothRegisterHandler(this, &TDWUPnPCommPanel::eClickHandler);
    m_pwndList->pnothRegisterHandler(this, &TDWUPnPCommPanel::eLBHandler);

    // Show the incoming device if we have one
    m_pwndCurDev->strWndText(conncfgEdit.strDevName());
}


// ---------------------------------------------------------------------------
//  TDWUPnPCommPanel: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Do a UPnP search and load up the list (selecting the previously selected
//  device if any and present.)
//
tCIDLib::TVoid TDWUPnPCommPanel::DoSearch()
{
    TCQCUPnPConnCfg& conncfgEdit = static_cast<TCQCUPnPConnCfg&>
    (
        cqcdcEdit().conncfgReal()
    );

    try
    {
        // Change the button text and disable it
        m_pwndSearch->strWndText(L"Searching...");
        m_pwndSearch->SetEnable(kCIDLib::False);

        if (!facCIDUPnP().bFindDevsByType(conncfgEdit.strDevType(), m_colLastSearch))
        {
            m_pwndSearch->strWndText(L"Search...");
            m_pwndSearch->SetEnable(kCIDLib::True);

            TErrBox msgbFail(L"No devices of the desired type were found");
            msgbFail.ShowIt(*this);
            return;
        }

        m_pwndSearch->strWndText(L"Search...");
        m_pwndSearch->SetEnable(kCIDLib::True);
    }

    catch(const TError& errToCatch)
    {
        m_pwndSearch->strWndText(L"Search...");
        m_pwndSearch->SetEnable(kCIDLib::True);

        facCQCGKit().ShowError
        (
            *this
            , L"An error occurred while searching for devices of the desired type"
            , errToCatch
        );
        return;
    }

    tCIDLib::TKVPList   colSvcList;
    TString             strTmp;
    tCIDLib::TCard4     c4InitSel = 0;
    const tCIDLib::TCard4 c4Count = m_colLastSearch.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TKeyValues& kvalsCur = m_colLastSearch[c4Index];

        //
        //  Set up the list window item. If the prompt included display
        //  control info we need to get the serivce indicated and query
        //  the state variable indicated and use that. Else, we just use the
        //  friendly name of the device we got during the find.
        //
        if (conncfgEdit.strDispSvcType().bIsEmpty())
        {
            m_pwndList->c4AddItem(kvalsCur.strVal1(), 0);
        }
         else
        {
            try
            {
                TUPnPDevice upnpdTmp;
                facCIDUPnP().SetDeviceFromUID(kvalsCur.strKey(), upnpdTmp);

                // Query this guy's list of available services
                upnpdTmp.QueryServices(colSvcList);

                // And try to find one of the type we want
                TUPnPService upnpsTmp;
                facCIDUPnP().bSetServiceByType
                (
                    colSvcList
                    , upnpdTmp
                    , upnpsTmp
                    , conncfgEdit.strDispSvcType()
                    , kCIDLib::True
                );

                //
                //  If we don't pump some messages during this process, we
                //  will fail because of stupid COM stuff, despite now much
                //  we try to hide it.
                //
                facCIDCtrls().MsgYield(50);

                // We got the service so try to query the state variable
                upnpsTmp.QueryStateVar(conncfgEdit.strDispVarName(), strTmp, kCIDLib::True);

                // And we got that so we can add the list item
                m_pwndList->c4AddItem(strTmp, 0);
            }

            catch(TError& errToCatch)
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);

                // Fall back to the normal name
                m_pwndList->c4AddItem(kvalsCur.strVal1(), 0);
            }
        }

        if (kvalsCur.strKey() == conncfgEdit.strDevUID())
            c4InitSel = c4Index;
    }

    // Try to select the previous value, if any, else the 0th one
    if (m_pwndList->c4ItemCount())
        m_pwndList->SelectByIndex(c4InitSel, kCIDLib::True);
}


// There's only one button, so just invoke the search method
tCIDCtrls::EEvResponses TDWUPnPCommPanel::eClickHandler(TButtClickInfo&)
{
    DoSearch();
    return tCIDCtrls::EEvResponses::Handled;
}


tCIDCtrls::EEvResponses TDWUPnPCommPanel::eLBHandler(TListChangeInfo& wnotInfo)
{
    if (wnotInfo.eEvent() == tCIDCtrls::EListEvents::SelChanged)
    {
        const TKeyValues& kvalsCur = m_colLastSearch[wnotInfo.c4Index()];
        m_pwndCurDev->strWndText(kvalsCur.strVal1());

        TCQCUPnPConnCfg& conncfgEdit = static_cast<TCQCUPnPConnCfg&>
        (
            cqcdcEdit().conncfgReal()
        );

        TCQCDriverObjCfg& cqcdcSrc = cqcdcEdit();
        conncfgEdit.strDevUID(kvalsCur.strKey());
        conncfgEdit.strDevName(kvalsCur.strVal1());
    }

    return tCIDCtrls::EEvResponses::Handled;
}



