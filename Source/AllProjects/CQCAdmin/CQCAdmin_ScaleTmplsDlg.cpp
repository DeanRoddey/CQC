//
// FILE NAME: CQCAdmin_ScaleTmplsDlg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/28/2014
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
//  This file implements the template scaling dialog.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


// ---------------------------------------------------------------------------
//  Includes
//
//  We only need the generation facility within here
// ---------------------------------------------------------------------------
#include    "CQCAdmin.hpp"
#include    "CQCAdmin_ScaleTmplsDlg.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TScaleTmplsCfg,TObject)
RTTIDecls(TCQCScaleTmplsDlg,TDlgBox)


// ---------------------------------------------------------------------------
//  Local types and constants
// ---------------------------------------------------------------------------
namespace CQCAdmin_ScaleTmplsDlg
{
    namespace
    {
        // -----------------------------------------------------------------------
        //  Our configuration class' persistent format version
        //
        //  Version 2 -
        //      Adjust for 5.x path styles
        // -----------------------------------------------------------------------
        constexpr   tCIDLib::TCard2     c2CfgFmtVersion = 2;
    }
};




// ---------------------------------------------------------------------------
//   CLASS: TScaleTmplsCfg
//  PREFIX: cfg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TScaleTmplsCfg: Constructors and Destructor
// ---------------------------------------------------------------------------
TScaleTmplsCfg::TScaleTmplsCfg() :

    m_bDoSubDirs(kCIDLib::False)
{
}

TScaleTmplsCfg::~TScaleTmplsCfg()
{
}


// ---------------------------------------------------------------------------
//  TScaleTmplsCfg: Public, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid TScaleTmplsCfg::Reset()
{
    m_bDoSubDirs = kCIDLib::False;
    m_colTarResList.RemoveAll();
    m_strSrcScope.Clear();
    m_strTarScope.Clear();
}


// ---------------------------------------------------------------------------
//  TScaleTmplsCfg: Protected, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid TScaleTmplsCfg::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a frame marker
    strmToReadFrom.CheckForFrameMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCAdmin_ScaleTmplsDlg::c2CfgFmtVersion))
    {
        facCIDLib().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCIDErrs::errcGen_UnknownFmtVersion
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(c2FmtVersion)
            , clsThis()
        );
    }

    // Stream our stuff out
    strmToReadFrom  >> m_bDoSubDirs
                    >> m_colTarResList
                    >> m_szSrcRes
                    >> m_strSrcScope
                    >> m_strTarScope;

    // If version 1, we have to adjust the paths
    if (c2FmtVersion < 2)
    {
        facCQCKit().Make50Path(m_strSrcScope);
        facCQCKit().Make50Path(m_strTarScope);
    }

    // Make sure we get the end marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TScaleTmplsCfg::StreamTo(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo   << tCIDLib::EStreamMarkers::Frame
                    << CQCAdmin_ScaleTmplsDlg::c2CfgFmtVersion

                    << m_bDoSubDirs
                    << m_colTarResList
                    << m_szSrcRes
                    << m_strSrcScope
                    << m_strTarScope

                    << tCIDLib::EStreamMarkers::EndObject;
}






// ---------------------------------------------------------------------------
//   CLASS: TCQCScaleTmplsDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCScaleTmplsDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCScaleTmplsDlg::TCQCScaleTmplsDlg() :

    m_bCleanTar(kCIDLib::False)
    , m_colImgs(109, TStringKeyOps(kCIDLib::False), &TKeyValuePair::strExtractKey)
    , m_colMsgQ(tCIDLib::EMTStates::Safe)
    , m_colScopes(109, TStringKeyOps(kCIDLib::False))
    , m_eStatus(EStatus::Success)
    , m_pwndAddTarRes(nullptr)
    , m_pwndCleanTar(nullptr)
    , m_pwndCloseButton(nullptr)
    , m_pwndDelTarRes(nullptr)
    , m_pwndDoSubs(nullptr)
    , m_pwndScaleButton(nullptr)
    , m_pwndSelSrcScope(nullptr)
    , m_pwndSelTarScope(nullptr)
    , m_pwndSrcResX(nullptr)
    , m_pwndSrcResY(nullptr)
    , m_pwndSrcScope(nullptr)
    , m_pwndStatus(nullptr)
    , m_pwndTarResList(nullptr)
    , m_pwndTarScope(nullptr)
    , m_strPrevVerMeta(L"/AdminScaleDlg/PrevVersion")
    , m_strmImg(kCIDLib::c4Sz_1M, kCIDLib::c4Sz_32M)
    , m_strmThumb(kCIDLib::c4Sz_256K)
    , m_szMinImg(1, 1)
    , m_thrWorker
      (
        TString(L"ScaleTemplatesDlgThread")
        , TMemberFunc<TCQCScaleTmplsDlg>(this, &TCQCScaleTmplsDlg::eScalingThread)
      )
    , m_tmidUpdate(kCIDCtrls::tmidInvalid)
{
}

TCQCScaleTmplsDlg::~TCQCScaleTmplsDlg()
{
}


// ---------------------------------------------------------------------------
//  TCQCScaleTmplsDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCScaleTmplsDlg::Run(const TWindow& wndOwner)
{
    c4RunDlg(wndOwner, facCQCAdmin, kCQCAdmin::ridDlg_ScaleTmpls);
}


// ---------------------------------------------------------------------------
//  TCQCScaleTmplsDlg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TCQCScaleTmplsDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRet = TParent::bCreated();

    // Store our pre-typed child widget pointers
    CastChildWnd(*this, kCQCAdmin::ridDlg_ScaleTmpls_AddTarRes, m_pwndAddTarRes);
    CastChildWnd(*this, kCQCAdmin::ridDlg_ScaleTmpls_CleanTar, m_pwndCleanTar);
    CastChildWnd(*this, kCQCAdmin::ridDlg_ScaleTmpls_Close, m_pwndCloseButton);
    CastChildWnd(*this, kCQCAdmin::ridDlg_ScaleTmpls_DelTarRes, m_pwndDelTarRes);
    CastChildWnd(*this, kCQCAdmin::ridDlg_ScaleTmpls_DoSubs, m_pwndDoSubs);
    CastChildWnd(*this, kCQCAdmin::ridDlg_ScaleTmpls_SelSrcScope, m_pwndSelSrcScope);
    CastChildWnd(*this, kCQCAdmin::ridDlg_ScaleTmpls_SelTarScope, m_pwndSelTarScope);
    CastChildWnd(*this, kCQCAdmin::ridDlg_ScaleTmpls_SrcScope, m_pwndSrcScope);
    CastChildWnd(*this, kCQCAdmin::ridDlg_ScaleTmpls_SrcResX, m_pwndSrcResX);
    CastChildWnd(*this, kCQCAdmin::ridDlg_ScaleTmpls_SrcResY, m_pwndSrcResY);
    CastChildWnd(*this, kCQCAdmin::ridDlg_ScaleTmpls_Status, m_pwndStatus);
    CastChildWnd(*this, kCQCAdmin::ridDlg_ScaleTmpls_TarScope, m_pwndTarScope);
    CastChildWnd(*this, kCQCAdmin::ridDlg_ScaleTmpls_TarResList, m_pwndTarResList);
    CastChildWnd(*this, kCQCAdmin::ridDlg_ScaleTmpls_Scale, m_pwndScaleButton);

    // Register our close button with our click handler
    m_pwndAddTarRes->pnothRegisterHandler(this, &TCQCScaleTmplsDlg::eClickHandler);
    m_pwndCleanTar->pnothRegisterHandler(this, &TCQCScaleTmplsDlg::eClickHandler);
    m_pwndCloseButton->pnothRegisterHandler(this, &TCQCScaleTmplsDlg::eClickHandler);
    m_pwndDelTarRes->pnothRegisterHandler(this, &TCQCScaleTmplsDlg::eClickHandler);
    m_pwndSelSrcScope->pnothRegisterHandler(this, &TCQCScaleTmplsDlg::eClickHandler);
    m_pwndSelTarScope->pnothRegisterHandler(this, &TCQCScaleTmplsDlg::eClickHandler);
    m_pwndScaleButton->pnothRegisterHandler(this, &TCQCScaleTmplsDlg::eClickHandler);

    // Load up any saved previous values
    LoadLastSettings();

    return bRet;
}


//
//  We have to watch for status messages from the background thread and completion
//  or failure status.
//
tCIDLib::TVoid TCQCScaleTmplsDlg::Timer(const tCIDCtrls::TTimerId tmidToDo)
{
    if (tmidToDo != m_tmidUpdate)
        return;

    while (m_colMsgQ.bPopBottom(m_strMsg))
        m_pwndStatus->strWndText(m_strMsg);

    // Watch for completion or failure and re-enable the scale button when done
    if (m_eStatus != EStatus::Processing)
    {
        // Kill the timer until the next time
        StopTimer(m_tmidUpdate);
        m_tmidUpdate = kCIDCtrls::tmidInvalid;

        // Make sure the thread is stopped. Indicate just wait for termination
        bStopWorker(kCIDLib::True);

        // Re-enable stuff we disabled before
        m_pwndAddTarRes->SetEnable(kCIDLib::True);
        m_pwndCleanTar->SetEnable(kCIDLib::True);
        m_pwndDelTarRes->SetEnable(kCIDLib::True);
        m_pwndDoSubs->SetEnable(kCIDLib::True);
        m_pwndSrcScope->SetEnable(kCIDLib::True);
        m_pwndSrcResX->SetEnable(kCIDLib::True);
        m_pwndSrcResY->SetEnable(kCIDLib::True);
        m_pwndScaleButton->SetEnable(kCIDLib::True);
        m_pwndSelSrcScope->SetEnable(kCIDLib::True);
        m_pwndSelTarScope->SetEnable(kCIDLib::True);
        m_pwndTarScope->SetEnable(kCIDLib::True);
        m_pwndTarResList->SetEnable(kCIDLib::True);
    }
}


// ---------------------------------------------------------------------------
//  TCQCScaleTmplsDlg: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Runs through the current list of target resolutions and makes sure the passed
//  one is not already in the list.
//
tCIDLib::TBoolean TCQCScaleTmplsDlg::bIsUniqueTarRes(const TSize& szCheck)
{
    TSize szCur;
    const tCIDLib::TCard4 c4TarResCnt = m_pwndTarResList->c4ItemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4TarResCnt; c4Index++)
    {
        szCur.SetFromPacked(m_pwndTarResList->c4IndexToId(c4Index));
        if (szCur == szCheck)
            return kCIDLib::False;
    }
    return kCIDLib::True;
}



//
//  Shutdown or wait for shutdown of the worker thread. If this is called after the
//  thread itself indicates it's done, then we just wait. We don't ask it to shut
//  down since it already on the way. Else we try to make it stop, which is usually
//  done if they hit Close while processing is going on.
//
tCIDLib::TBoolean TCQCScaleTmplsDlg::bStopWorker(const tCIDLib::TBoolean bWaitOnly)
{
    // If not running, then nothing to do
    if (m_thrWorker.bIsRunning())
    {
        try
        {
            if (bWaitOnly)
            {
                m_pwndStatus->strWndText(L"Waiting for processing to complete...");
            }
             else
            {
                m_pwndStatus->strWndText(L"Stopping background processing...");
                m_thrWorker.ReqShutdownSync(10000);
            }

            m_thrWorker.eWaitForDeath(4000);
        }

        catch(TError& errToCatch)
        {
            // Tell them we couldn't do it
            if (facCQCAdmin.bShouldLog(errToCatch))
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }

            TExceptDlg dlgErr
            (
                *this
                , strWndText()
                , L"Failed to stop background processing thread"
                , errToCatch
            );
            return kCIDLib::False;
        }
    }

    //
    //  If just waiting, then we aren't going to close, so display the final
    //  status.
    //
    if (m_eStatus == EStatus::Failure)
        m_pwndStatus->strWndText(L"The scaling operation failed");
    else
        m_pwndStatus->strWndText(L"The scaling operation succeeded");

    return kCIDLib::True;
}


//
//  When the user asks to do the scaling, we validate the entered data and return
//  whether it is acceptable or not.
//
tCIDLib::TBoolean TCQCScaleTmplsDlg::bValidate()
{
    // Get the source and target scopes
    TString strSrc = m_pwndSrcScope->strWndText();
    TString strTar = m_pwndTarScope->strWndText();

    //
    //  For now, during the transition from 4.x to 5.x, just automatically convert to
    //  5.x type paths in case they user the wrong type of slashes, and put the values
    //  back.
    //
    facCQCKit().Make50Path(strSrc);
    facCQCKit().Make50Path(strTar);
    m_pwndSrcScope->strWndText(strSrc);
    m_pwndTarScope->strWndText(strTar);

    // Make sure the target under the /User section, but isn't /User itself.
    if (!strTar.bStartsWithI(L"/User")
    ||  strTar.bCompareI(L"/User")
    ||  strTar.bCompareI(L"/User/"))
    {
        TErrBox msgbErr(facCQCAdmin.strMsg(kCQCAErrs::errcScale_BadTarScope));
        msgbErr.ShowIt(*this);

        m_pwndTarScope->TakeFocus();
        return kCIDLib::False;
    }

    //
    //  Make sure the source scope is not empty and is in either the user or
    //  system areas.
    //
    if (strTar.bIsEmpty()
    ||  (!strTar.bStartsWithI(L"/User")
    &&   !strTar.bStartsWithI(L"/System")))
    {
        TErrBox msgbErr(facCQCAdmin.strMsg(kCQCAErrs::errcScale_BadSrc));
        msgbErr.ShowIt(*this);

        m_pwndTarScope->TakeFocus();
        return kCIDLib::False;
    }

    // Make sure it exists
    try
    {
        TDataSrvClient dsclTest;
        if (!dsclTest.bIsScope(strSrc, tCQCRemBrws::EDTypes::Template))
        {
            TErrBox msgbErr(facCQCAdmin.strMsg(kCQCAErrs::errcScale_SrcNotFound));
            msgbErr.ShowIt(*this);

            m_pwndSrcScope->TakeFocus();
            return kCIDLib::False;
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        TExceptDlg dlgErr
        (
            *this
            , strWndText()
            , L"Couldn't confirm the existence of the source scope"
            , errToCatch
        );
        return kCIDLib::False;
    }

    //
    //  Make sure that the target is NOT within the source directory. This can
    //  cause all kinds of problems. To avoid a possible partial match, we have to
    //  create a version of the source with an ending slash.
    //
    {
        TPathStr pathSrcTest(strSrc);
        pathSrcTest.AddTrailingSeparator();
        if (strTar.bStartsWithI(pathSrcTest))
        {
            TErrBox msgbErr(facCQCAdmin.strMsg(kCQCAErrs::errcScale_TarInSrc));
            msgbErr.ShowIt(*this);
            m_pwndTarScope->TakeFocus();
            return kCIDLib::False;
        }
    }

    // Check the src res values
    tCIDLib::TCard4 c4SrcWidth = 0;
    tCIDLib::TCard4 c4SrcHeight = 0;
    if (!m_pwndSrcResX->strWndText().bToCard4(c4SrcWidth, tCIDLib::ERadices::Dec)
    ||  !m_pwndSrcResY->strWndText().bToCard4(c4SrcHeight, tCIDLib::ERadices::Dec)
    ||  (c4SrcWidth < 64)
    ||  (c4SrcHeight < 64)
    ||  (c4SrcWidth > 8192)
    ||  (c4SrcHeight > 8192))
    {
        TErrBox msgbErr(facCQCAdmin.strMsg(kCQCAErrs::errcScale_BadRes, TString(L"source")));
        msgbErr.ShowIt(*this);
        m_pwndSrcResX->TakeFocus();
        return kCIDLib::False;
    }

    // If there are no target resolutions, then we can't do anything
    const tCIDLib::TCard4 c4TarResCnt = m_pwndTarResList->c4ItemCount();
    if (!c4TarResCnt)
    {
        TErrBox msgbErr(facCQCAdmin.strMsg(kCQCAErrs::errcScale_NoTarRes));
        msgbErr.ShowIt(*this);
        m_pwndTarResList->TakeFocus();
        return kCIDLib::False;
    }

    m_cfgInfo.m_bDoSubDirs = m_pwndDoSubs->bCheckedState();
    m_cfgInfo.m_strSrcScope = strSrc;
    m_cfgInfo.m_strTarScope = strTar;
    m_cfgInfo.m_szSrcRes.Set(c4SrcWidth, c4SrcHeight);

    m_cfgInfo.m_colTarResList.RemoveAll();
    TSize szCur;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4TarResCnt; c4Index++)
    {
        szCur.SetFromPacked(m_pwndTarResList->c4IndexToId(c4Index));
        m_cfgInfo.m_colTarResList.objAdd(szCur);
    }

    // Store this info as the last settings
    try
    {
        tCIDLib::TCard4 c4Ver = 0;
        facCQCGKit().bAddUpdateStoreObj
        (
            kCQCAdmin::pszCfgKey_AutogenLast, c4Ver, m_cfgInfo, 2048
        );
    }

    catch(TError& errToCatch)
    {
        if (!errToCatch.bLogged())
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);

            TErrBox msgbErr(facCQCAdmin.strMsg(kCQCAErrs::errcScale_CantStore));
            msgbErr.ShowIt(*this);
        }
    }

    return kCIDLib::True;
}


tCIDLib::TVoid
TCQCScaleTmplsDlg::CreatePath(          TDataSrvClient&         dsclToUse
                                , const TString&                strPath
                                , const tCIDLib::TBoolean       bImage)
{
    CIDAssert(strPath.bStartsWithI(L"/User/"), L"Path is not in User scope");

    //
    //  We just break out each / separated chunk and see if it exists. If not
    //  we create it. It has to start with /User
    //
    TStringTokenizer stokPath(&strPath, L"/");
    TString strCur;

    //
    //  The new path always stays one ahead of the base. So we use new to check for
    //  existenced, and base plus current component to create the new base.
    //
    TString strBase(L"/");
    TString strNew(strBase);

    const tCQCRemBrws::EDTypes eType = bImage ? tCQCRemBrws::EDTypes::Image
                                              : tCQCRemBrws::EDTypes::Template;
    while (stokPath.bGetNextToken(strCur))
    {
        facCQCRemBrws().AddPathComp(strNew, strCur);
        if (!dsclToUse.bScopeExists(strNew, eType))
            dsclToUse.MakeNewScope(strBase, strCur, eType, facCQCAdmin.sectUser());

        // Add this one to the base and move forward
        facCQCRemBrws().AddPathComp(strBase, strCur);
    }
}


tCIDCtrls::EEvResponses TCQCScaleTmplsDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCAdmin::ridDlg_ScaleTmpls_AddTarRes)
    {
        // Get the currently entered source resolution
        tCIDLib::TCard4 c4SrcWidth = 0;
        tCIDLib::TCard4 c4SrcHeight = 0;
        if (!m_pwndSrcResX->strWndText().bToCard4(c4SrcWidth, tCIDLib::ERadices::Dec)
        ||  !m_pwndSrcResY->strWndText().bToCard4(c4SrcHeight, tCIDLib::ERadices::Dec))
        {
            TErrBox msgbNoSrcRes(L"You must enter a valid source resolution first");
            msgbNoSrcRes.ShowIt(*this);
            return tCIDCtrls::EEvResponses::Handled;
        }

        // Store it and pass it in as the base size for the AR
        m_cfgInfo.m_szSrcRes.Set(c4SrcWidth, c4SrcHeight);
        TSize szNew;
        const tCIDLib::TBoolean bRes = facCIDWUtils().bGetSize
        (
            *this
            , L"Enter a new target resolution"
            , szNew
            , m_cfgInfo.m_szSrcRes
            , TSize(64, 64)
            , TSize(8192, 8192)
            , kCIDLib::False
        );

        if (bRes)
        {
            // Make sure it's a unique resolution
            if (bIsUniqueTarRes(szNew))
            {
                TString strRes;
                strRes.AppendFormatted(szNew.c4Width());
                strRes.Append(L'x');
                strRes.AppendFormatted(szNew.c4Height());

                m_pwndTarResList->c4AddItem(strRes, szNew.c4Packed());
            }
             else
            {
                TErrBox msgbErr(facCQCAdmin.strMsg(kCQCAErrs::errcScale_NotUniqueRes));
                msgbErr.ShowIt(*this);
            }
        }
    }
     else if (wnotEvent.widSource() == kCQCAdmin::ridDlg_ScaleTmpls_CleanTar)
    {
        // Just store the new checked state for the thread to see when it starts
        m_bCleanTar = m_pwndCleanTar->bCheckedState();
    }
     else if (wnotEvent.widSource() == kCQCAdmin::ridDlg_ScaleTmpls_Close)
    {
        // If processing, then force it to stop
        bStopWorker(kCIDLib::False);

        // And we can exit now
        EndDlg(kCQCAdmin::ridDlg_ScaleTmpls_Close);
    }
     else if (wnotEvent.widSource() == kCQCAdmin::ridDlg_ScaleTmpls_DelTarRes)
    {
        if (m_pwndTarResList->c4ItemCount())
            m_pwndTarResList->RemoveCurrent();
    }
     else if ((wnotEvent.widSource() == kCQCAdmin::ridDlg_ScaleTmpls_SelSrcScope)
          ||  (wnotEvent.widSource() == kCQCAdmin::ridDlg_ScaleTmpls_SelTarScope))
    {
        Select(wnotEvent.widSource() == kCQCAdmin::ridDlg_ScaleTmpls_SelSrcScope);
    }
     else if (wnotEvent.widSource() == kCQCAdmin::ridDlg_ScaleTmpls_Scale)
    {
        try
        {
            if (bValidate())
            {
                // Initialize the status enum and flush the status msg queue
                m_eStatus = EStatus::Processing;
                m_colMsgQ.RemoveAll();

                m_pwndAddTarRes->SetEnable(kCIDLib::False);
                m_pwndCleanTar->SetEnable(kCIDLib::False);
                m_pwndDelTarRes->SetEnable(kCIDLib::False);
                m_pwndDoSubs->SetEnable(kCIDLib::False);
                m_pwndSrcScope->SetEnable(kCIDLib::False);
                m_pwndSrcResX->SetEnable(kCIDLib::False);
                m_pwndSrcResY->SetEnable(kCIDLib::False);
                m_pwndScaleButton->SetEnable(kCIDLib::False);
                m_pwndSelSrcScope->SetEnable(kCIDLib::False);
                m_pwndSelTarScope->SetEnable(kCIDLib::False);
                m_pwndTarScope->SetEnable(kCIDLib::False);
                m_pwndTarResList->SetEnable(kCIDLib::False);

                // Kick off the background scaling operation
                m_thrWorker.Start();

                // Enable the timer that pulls status info from the bgn thread
                m_tmidUpdate = tmidStartTimer(250);
            }
        }

        catch(TError& errToCatch)
        {
            // Tell them we couldn't do it
            if (facCQCAdmin.bShouldLog(errToCatch))
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }

            TExceptDlg dlgErr
            (
                *this
                , strWndText()
                , L"The template scaling operation failed"
                , errToCatch
            );
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}


//
//  If the data validates, then this is started on a thread to do the actual scaling
//  of the templates and images.
//
tCIDLib::EExitCodes TCQCScaleTmplsDlg::eScalingThread(TThread& thrThis, tCIDLib::TVoid*)
{
    // Let the calling thread go
    thrThis.Sync();

    //
    //  We have to make sure the interface engine is in designer mode, but get back to the
    //  original mode. So save the current mode before we change it.
    //
    const tCIDLib::TBoolean bOrgDesMode = TFacCQCIntfEng::bDesMode();
    try
    {
        // Now set the designer mode flag
        TFacCQCIntfEng::bDesMode(kCIDLib::True);

        m_colMsgQ.objPushTop(L"Connecting to server");
        TDataSrvClient dsclToUse;

        // If the users says so, clean up the target dirs first
        if (m_bCleanTar)
        {
            m_colMsgQ.objPushTop(L"Cleaning up target paths");
            if (dsclToUse.bScopeExists(m_cfgInfo.m_strTarScope, tCQCRemBrws::EDTypes::Image))
            {
                dsclToUse.DeleteScope
                (
                    m_cfgInfo.m_strTarScope
                    , tCQCRemBrws::EDTypes::Image
                    , facCQCAdmin.sectUser()
                );
            }

            if (dsclToUse.bScopeExists(m_cfgInfo.m_strTarScope, tCQCRemBrws::EDTypes::Template))
            {
                dsclToUse.DeleteScope
                (
                    m_cfgInfo.m_strTarScope
                    , tCQCRemBrws::EDTypes::Template
                    , facCQCAdmin.cuctxToUse().sectUser()
                );
            }
        }

        // Loop through the defined target resolutions
        const tCIDLib::TCard4 c4TarResCnt = m_cfgInfo.m_colTarResList.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4TarResCnt; c4Index++)
        {
            const TSize& szCur = m_cfgInfo.m_colTarResList[c4Index];

            // Calculate the horizontal and vertical scaling factors
            m_f8XScale = tCIDLib::TFloat8(szCur.c4Width())
                         / tCIDLib::TFloat8(m_cfgInfo.m_szSrcRes.c4Width());

            m_f8YScale = tCIDLib::TFloat8(szCur.c4Height())
                         / tCIDLib::TFloat8(m_cfgInfo.m_szSrcRes.c4Height());

            // Add the resolution to the target scope
            TString strResPart;
            strResPart.SetFormatted(szCur.c4Width());
            strResPart.Append(L'x');
            strResPart.AppendFormatted(szCur.c4Height());

            // Set up the msg queue prefix for this round, which includes the res
            m_strMsgQPref = L"[";
            m_strMsgQPref.Append(strResPart);
            m_strMsgQPref.Append(L"] ");

            TString strTar = m_cfgInfo.m_strTarScope;
            facCQCRemBrws().AddPathComp(strTar, strResPart);

            // Make sure the image and scope lists are flushed
            m_colImgs.RemoveAll();
            m_colScopes.RemoveAll();

            // Create the target scopes up to the starting point
            CreatePath(dsclToUse, strTar, kCIDLib::False);

            m_strImgDir = strTar;
            CreatePath(dsclToUse, m_strImgDir, kCIDLib::True);

            // And start processing at this level
            TGraphWndDev gdevTmp(*this);
            ProcessScope(gdevTmp, thrThis, dsclToUse, m_cfgInfo.m_strSrcScope, strTar);
        }

        // Make sure we get back to the original designer mode
        TFacCQCIntfEng::bDesMode(bOrgDesMode);

        // Indicate we were successful
        m_eStatus = EStatus::Success;
    }

    catch(TError& errToCatch)
    {
        // Make sure we get back to the original designer mode
        TFacCQCIntfEng::bDesMode(bOrgDesMode);

        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        // Indicate we failed
        m_eStatus = EStatus::Failure;
    }

    return tCIDLib::EExitCodes::Normal;
}



//
//  See if there are any previous values stored in our private object store. If so,
//  then load them up.
//
tCIDLib::TVoid TCQCScaleTmplsDlg::LoadLastSettings()
{
    // If there's no previously stored info, then do nothing
    try
    {
        tCIDLib::TCard4 c4Ver = 0;
        tCIDLib::ELoadRes eRes = facCQCGKit().eReadStoreObj
        (
            kCQCAdmin::pszCfgKey_AutogenLast, c4Ver, m_cfgInfo, kCIDLib::False
        );

        if (eRes != tCIDLib::ELoadRes::NewData)
            return;
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        m_cfgInfo.Reset();
        TExceptDlg dlgErr
        (
            *this
            , strWndText()
            , L"Could not load the previous scaling settings, they have been reset. Sorry"
            , errToCatch
        );
    }

    // We got something, or we got an error and set back to defaults, so load it up
    TString strRes;
    m_pwndSrcScope->strWndText(m_cfgInfo.m_strSrcScope);

    if (m_cfgInfo.m_szSrcRes.c4Width())
    {
        strRes.SetFormatted(m_cfgInfo.m_szSrcRes.c4Width());
        m_pwndSrcResX->strWndText(strRes);
    }

    if (m_cfgInfo.m_szSrcRes.c4Height())
    {
        strRes.SetFormatted(m_cfgInfo.m_szSrcRes.c4Height());
        m_pwndSrcResY->strWndText(strRes);
    }

    m_pwndTarScope->strWndText(m_cfgInfo.m_strTarScope);


    // Load up the target resolution list
    const tCIDLib::TCard4 c4ResCnt = m_cfgInfo.m_colTarResList.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ResCnt; c4Index++)
    {
        const TSize& szCur = m_cfgInfo.m_colTarResList[c4Index];

        strRes.SetFormatted(szCur.c4Width());
        strRes.Append(L'x');
        strRes.AppendFormatted(szCur.c4Height());

        m_pwndTarResList->c4AddItem(strRes, szCur.c4Packed());
    }


    // Set the do sub-dirs check box
    m_pwndDoSubs->SetCheckedState(m_cfgInfo.m_bDoSubDirs);
}


//
//  Download the indicated template and stream it into the passed template object.
//  We give back the current serial number.
//
tCIDLib::TVoid
TCQCScaleTmplsDlg::LoadTemplate(        TDataSrvClient&     dsclToUse
                                , const TString&            strPath
                                ,       tCIDLib::TCard4&    c4CurSerialNum
                                ,       tCIDLib::TKVPFList& colMeta
                                ,       TCQCIntfTemplate&   iwdgToFill)
{
    tCIDLib::TEncodedTime   enctLast;
    tCIDLib::TCard4         c4Bytes;

    c4CurSerialNum = 0;
    const tCIDLib::TBoolean bNewData = dsclToUse.bReadTemplate
    (
        strPath
        , c4CurSerialNum
        , enctLast
        , m_mbufData
        , c4Bytes
        , colMeta
        , facCQCAdmin.sectUser()
    );

    // Stream it in from the buffer we got
    TBinMBufInStream strmIn(&m_mbufData, c4Bytes);
    strmIn >> iwdgToFill;
}


//
//  A recursive method that will process our way down through all of the templates
//  under the original source scope.
//
tCIDLib::TVoid
TCQCScaleTmplsDlg::ProcessScope(        TGraphDrawDev&  gdevTmp
                                ,       TThread&        thrThis
                                ,       TDataSrvClient& dsclToUse
                                , const TString&        strSrc
                                , const TString&        strTar)
{
    // Query the templates and sub-dirs in the source at this level.
    tCIDLib::TStrList colFiles;
    tCIDLib::TStrList colSubDirs;

    tCIDLib::TCard4 c4Id = 0;
    dsclToUse.bQueryScopeNames
    (
        c4Id
        , strSrc
        , tCQCRemBrws::EDTypes::Template
        , colFiles
        , tCQCRemBrws::EQSFlags::Items
        , facCQCAdmin.sectUser()
    );

    c4Id = 0;
    dsclToUse.bQueryScopeNames
    (
        c4Id
        , strSrc
        , tCQCRemBrws::EDTypes::Template
        , colSubDirs
        , tCQCRemBrws::EQSFlags::Scopes
        , facCQCAdmin.sectUser()
    );


    TString                 strSrcPath;
    TString                 strTarPath;
    tCIDLib::TKVPFList      colMeta;
    tCIDLib::TStrList       colChunkIds;
    tCIDLib::TCardList      fcolChunkSizes;
    tCIDLib::TCard4         c4PrevSerialNum = 0;
    tCIDLib::TCard4         c4SerialNum = 0;
    tCIDLib::TEncodedTime   enctLastChange;
    TString                 strMetaVal;

    const tCIDLib::TCard4   c4FileCnt = colFiles.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4FileCnt; c4Index++)
    {
        const TString& strCurName = colFiles[c4Index];

        strSrcPath = strSrc;
        facCQCRemBrws().AddPathComp(strSrcPath, strCurName);

        strTarPath = strTar;
        facCQCRemBrws().AddPathComp(strTarPath, strCurName);

        //
        //  See if the target already exists. If so, let's query the metadata on it.
        //  We store the version of the original when we scale them, so we can see
        //  if we might be able to skip this one.
        //
        c4PrevSerialNum = 0;
        tCIDLib::TBoolean bTarExists = dsclToUse.bQueryItemMetaData
        (
            strTarPath
            , tCQCRemBrws::EDTypes::Template
            , colMeta
            , colChunkIds
            , fcolChunkSizes
            , c4SerialNum
            , enctLastChange
            , kCIDLib::False
            , facCQCAdmin.sectUser()
        );

        // If we got data, then it exists, so let's see if it has version info
        if (bTarExists)
        {
            if (TDataSrvClient::bFindMetaVal(m_strPrevVerMeta, colMeta, strMetaVal))
            {
                if (!strMetaVal.bToCard4(c4PrevSerialNum, tCIDLib::ERadices::Dec))
                    c4PrevSerialNum = 0;
            }
        }


        //
        //  Query the source template. THis will get us back the current serial number
        //  so that we can compare to the previous one above if we got it.
        //
        m_strMsg = m_strMsgQPref;
        m_strMsg.Append(L"Processing ");
        m_strMsg.Append(strCurName);
        m_colMsgQ.objAdd(m_strMsg);
        LoadTemplate(dsclToUse, strSrcPath, c4SerialNum, colMeta, m_iwdgCur);

        //
        //  Query all of the images referenced by this template, including system,
        //  and process any we haven't already done. Tell him if the template needs
        //  to be updated, since if not he doesn't need to do any search and replace.
        //
        ScaleImages(thrThis, dsclToUse, m_iwdgCur, (c4SerialNum != c4PrevSerialNum));

        // Scale the template now if it changed since we last scaled it
        if (c4SerialNum != c4PrevSerialNum)
        {
            facCQCIntfEng().ScaleTemplate(gdevTmp, m_iwdgCur, m_f8XScale, m_f8YScale);

            // Dump it back to a memory buffer
            tCIDLib::TCard4 c4Size;
            {
                TBinMBufOutStream strmTar(&m_mbufData);
                strmTar << m_iwdgCur << kCIDLib::FlushIt;
                c4Size = strmTar.c4CurSize();
            }

            //
            //  Reuse the meta list to store the original serial number for
            //  next time.
            //
            colMeta.RemoveAll();
            strMetaVal.SetFormatted(c4SerialNum);
            colMeta.objPlace(m_strPrevVerMeta, strMetaVal, kCIDLib::False);

            // Tell the data server not to memory cache if it's not already
            c4SerialNum = 0;
            dsclToUse.WriteTemplate
            (
                strTarPath
                , c4SerialNum
                , enctLastChange
                , m_mbufData
                , c4Size
                , m_iwdgCur.eMinRole()
                , m_iwdgCur.strNotes()
                , kCQCRemBrws::c4Flag_OverwriteOK | kCQCRemBrws::c4Flag_NoDataCache
                , colMeta
                , facCQCAdmin.sectUser()
            );

            // Don't beat the server too bad
            if (!thrThis.bSleep(500))
            {
                m_colMsgQ.objAdd(L"Process was cancelled");
                return;
            }
        }
        else
        {
            // Throttle a bit still in this case and watch for shutdown request
            if (!thrThis.bSleep(50))
            {
                m_colMsgQ.objAdd(L"Process was cancelled");
                return;
            }
        }
    }

    //
    //  If we are doing a recursiive operation, then let's now process the sub-
    //  dirs as well.
    //
    if (m_cfgInfo.m_bDoSubDirs)
    {
        const tCIDLib::TCard4 c4SubCnt = colSubDirs.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4SubCnt; c4Index++)
        {
            const TString& strCurName = colSubDirs[c4Index];

            strSrcPath = strSrc;
            facCQCRemBrws().AddPathComp(strSrcPath, strCurName);

            strTarPath = strTar;
            facCQCRemBrws().AddPathComp(strTarPath, strCurName);

            // Create the new scope if it doesn't exist
            if (!dsclToUse.bScopeExists(strTarPath, tCQCRemBrws::EDTypes::Template))
            {
                dsclToUse.MakeNewScope
                (
                    strTar
                    , strCurName
                    , tCQCRemBrws::EDTypes::Template
                    , facCQCAdmin.sectUser()
                );
            }

            ProcessScope(gdevTmp, thrThis, dsclToUse, strSrcPath, strTarPath);
        }
    }
}


//
//  Check the passed source image. If it's not already be processed then process it
//  and write to the target image scope. If the source has changed since we last
//  processed the template, then we need to do search and replace to update any
//  image path references.
//
tCIDLib::TVoid
TCQCScaleTmplsDlg::ScaleImage(  TDataSrvClient&             dsclToUse
                                , TString&                  strSrc
                                , TCQCIntfTemplate&         iwdgTar
                                , const tCIDLib::TBoolean   bNeedRep)
{
    static const tCQCIntfEng::ERepFlags eFlags = tCIDLib::eOREnumBits
    (
        tCQCIntfEng::ERepFlags::Image
        , tCQCIntfEng::ERepFlags::ActionParms
        , tCQCIntfEng::ERepFlags::Children
    );

    TSize szNew;
    TString strTar;
    TString strTmp;
    TString strRegEx;
    TString strTok;

    // See if it's in the list
    const TKeyValuePair* pkvalCur = m_colImgs.pobjFindByKey(strSrc);

    // If not, download it, process it, and load it back up in the target
    if (!pkvalCur)
    {
        tCIDLib::TKVPFList      colMeta;
        tCIDLib::TCard4         c4SerNum = 0;
        tCIDLib::TCard4         c4PrevSerNum = 0;
        tCIDLib::TEncodedTime   enctLast;
        tCIDLib::TStrList       colChunkIds;
        tCIDLib::TCardList      fcolChunkSizes;
        try
        {
            //
            //  Create the target path. We recreate the source dir structure under
            //  the target, changing /System to /S and /User to /U. So take the srce
            //  and replace the first part first.
            //
            strTmp = strSrc;
            if (strTmp.bStartsWithI(L"/User"))
                strTmp.Cut(2, 3);
            else if (strTmp.bStartsWithI(L"/System"))
                strTmp.Cut(2, 5);

            // Add this modified path to the target image path
            strTar = m_strImgDir;
            facCQCRemBrws().AddPathComp(strTar, strTmp);

            //
            //  See if the target already exists. If so, let's query the metadata on it.
            //  We store the version of the original when we scale them, so we can see
            //  if we might be able to skip this one.
            //
            c4SerNum = 0;
            tCIDLib::TBoolean bTarExists = dsclToUse.bQueryItemMetaData
            (
                strTar
                , tCQCRemBrws::EDTypes::Image
                , colMeta
                , colChunkIds
                , fcolChunkSizes
                , c4SerNum
                , enctLast
                , kCIDLib::False
                , facCQCAdmin.sectUser()
            );

            // If we got data, then it exists, so let's see if it has version info
            c4PrevSerNum = 0;
            if (bTarExists)
            {
                if (TDataSrvClient::bFindMetaVal(m_strPrevVerMeta, colMeta, strTmp))
                {
                    if (!strTmp.bToCard4(c4PrevSerNum, tCIDLib::ERadices::Dec))
                        c4PrevSerNum = 0;
                }
            }

            // Read the original content
            c4SerNum = 0;
            const tCIDLib::TBoolean bGotImg = dsclToUse.bReadImage
            (
                strSrc, c4SerNum, enctLast, m_imgSrc, colMeta, facCQCAdmin.sectUser()
            );

            // If we got the image and the version chaned, try to process it
            if (bGotImg && (c4SerNum != c4PrevSerNum))
            {
                // Get the size and scale it
                szNew = m_imgSrc.szImage();
                szNew.Scale(m_f8XScale, m_f8YScale);

                // Don't allow anything less than our min allowed
                szNew.TakeLarger(m_szMinImg);

                // Scale the image to this new size
                m_imgSrc.Scale(szNew, 3);

                //
                //  Make sure this path exists. So get a copy of this back and cut off
                //  the name.ext part. See if this scope exists, and create it if so.
                //
                strTmp = strTar;
                facCQCRemBrws().bRemoveLastPathItem(strTmp);
                if (!dsclToUse.bScopeExists(strTmp, tCQCRemBrws::EDTypes::Image))
                    CreatePath(dsclToUse, strTmp, kCIDLib::True);

                //
                //  Reuse the meta list to store the original serial number for
                //  next time.
                //
                colMeta.RemoveAll();
                strTmp.SetFormatted(c4SerNum);
                colMeta.objPlace(m_strPrevVerMeta, strTmp, kCIDLib::False);

                // Now upload the image, tell data server not to cache if not already.
                facCQCIGKit().UploadImage
                (
                    dsclToUse
                    , m_imgSrc
                    , strTar
                    , m_strmImg
                    , m_strmThumb
                    , kCIDLib::False
                    , colMeta
                    , facCQCAdmin.cuctxToUse()
                );
            }

            //
            //  Add this one to the image list, so we won't try keep trying to
            //  do it. Even if we didn't do it above, that's ok because we don't
            //  need to now.
            //
            pkvalCur = &m_colImgs.objPlace(strSrc, strTar);
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
    }

    //
    //  Replace all references to this image if we found it and the template is going
    //  to be updated.
    //
    if (pkvalCur && bNeedRep)
    {
        iwdgTar.Replace
        (
            eFlags
            , pkvalCur->strKey()
            , pkvalCur->strValue()
            , kCIDLib::False
            , kCIDLib::True
            , m_regxImgs
        );
    }
}


//
//  Ask the template (and all contained widgets) for all referenced images. Then
//  we loop through them and call the helper above to scale each one.
//
tCIDLib::TVoid
TCQCScaleTmplsDlg::ScaleImages(         TThread&            thrThis
                                ,       TDataSrvClient&     dsclToUse
                                ,       TCQCIntfTemplate&   iwdgTar
                                , const tCIDLib::TBoolean   bNeedRep)
{
    //
    //  Query all referenced images in this template. We include system and scope
    //  type references. Set up the key ops objects for non-case sensitive hashing,
    //  so that any case variation doesn't cause us to process the same one twice.
    //
    tCIDLib::TStrHashSet    colImages(109, TStringKeyOps(kCIDLib::False));
    tCIDLib::TStrHashSet    colScopes(109, TStringKeyOps(kCIDLib::False));

    m_iwdgCur.QueryReferencedImgs
    (
        colImages, colScopes, kCIDLib::True, kCIDLib::True
    );

    // Go through the individual image refs
    TString strCur;
    {
        tCIDLib::TStrHashSet::TCursor cursImgs(&colImages);
        if (cursImgs.bReset())
        {
            do
            {
                //
                //  Get a copy of the current image path and scale it. This guy will
                //  modify the path so don't use it after we get back.
                //
                strCur = cursImgs.objRCur();
                ScaleImage(dsclToUse, strCur, iwdgTar, bNeedRep);
            }   while(cursImgs.bNext());
        }
    }

    // Do any of the scope based images
    {
        tCIDLib::TStrHashSet::TCursor cursScopes(&colScopes);
        if (cursScopes.bReset())
        {
            static const tCQCIntfEng::ERepFlags eFlags = tCIDLib::eOREnumBits
            (
                tCQCIntfEng::ERepFlags::Image
                , tCQCIntfEng::ERepFlags::Children
            );

            tCIDLib::TStrList   colImgPaths;
            tCIDLib::TStrList   colDirs;
            TString             strTar;
            TString             strNewRelPart;
            TString             strCurImg;
            do
            {
                tCIDLib::TCard4 c4At = 0;
                TString strCurScope = cursScopes.objRCur();

                // If we already have this scope, skip it
                if (m_colScopes.bHasElement(strCurScope))
                    continue;

                //
                //  Get a list of images in this scope and process them all. This will
                //  add them to the image list as well, but that's ok. Some of these may
                //  have been referenced as individual images as well, so this will insure
                //  we don't do them redundantly at that level either.
                //
                tCIDLib::TCard4 c4Id = 0;
                dsclToUse.bQueryScopeNames
                (
                    c4Id
                    , strCurScope
                    , tCQCRemBrws::EDTypes::Image
                    , colImgPaths
                    , tCQCRemBrws::EQSFlags::Items
                    , facCQCAdmin.sectUser()
                );

                // Iterate the images list and process each one
                const tCIDLib::TCard4 c4ImgCnt = colImgPaths.c4ElemCount();
                for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ImgCnt; c4Index++)
                {
                    strCurImg = strCurScope;
                    facCQCRemBrws().AddPathComp(strCurImg, colImgPaths[c4Index]);
                    ScaleImage(dsclToUse, strCurImg, iwdgTar, bNeedRep);
                }

                //
                //  Update any widgets that reference this scope. We require a full
                //  match so we won't affect any image paths we have already updated.
                //  Build up the part of the new path relative to the base image output
                //  directory, ending with a slash, since we want the resulting path
                //  to have one.
                //
                strNewRelPart = strCurScope;
                if (strNewRelPart.bStartsWithI(L"/User"))
                    strNewRelPart.Cut(2, 3);
                else if (strNewRelPart.bStartsWithI(L"/System"))
                    strNewRelPart.Cut(2, 5);

                if (strNewRelPart.chLast() != kCIDLib::chForwardSlash)
                    strNewRelPart.Append(kCIDLib::chForwardSlash);

                //
                //  Graft this new scope onto the overall image output directory and that
                //  is what we want the new one to be. Replace the current scope with this.
                //
                strTar = m_strImgDir;
                facCQCRemBrws().AddPathComp(strTar, strNewRelPart);

                if (bNeedRep)
                {
                    iwdgTar.Replace
                    (
                        eFlags
                        , strCurScope
                        , strTar
                        , kCIDLib::False
                        , kCIDLib::True
                        , m_regxImgs
                    );

                    //
                    //  Do it again, this time either adding a trailing slash if one wasn't
                    //  there, or removing it if it was, so that we are sure to get both
                    //  versions. We require a full match, so we would otherwise miss one
                    //  or the other.
                    //
                    if (strCurScope.chLast() == kCIDLib::chForwardSlash)
                        strCurScope.DeleteLast();
                    else
                        strCurScope.Append(kCIDLib::chForwardSlash);
                    iwdgTar.Replace
                    (
                        eFlags
                        , strCurScope
                        , strTar
                        , kCIDLib::False
                        , kCIDLib::True
                        , m_regxImgs
                    );
                }

            }  while(cursScopes.bNext());
        }
    }
}


//
//  Select the source or target scope. They actually just pick some template within
//  the desired scope.
//
tCIDLib::TVoid TCQCScaleTmplsDlg::Select(const tCIDLib::TBoolean bSrc)
{
    //
    //  Use the flags to limit them differently depending on whether it's the source
    //  or target. Source doesn't have to be in the user area. Target has to be.
    //
    tCQCTreeBrws::EFSelFlags eFlags = tCQCTreeBrws::EFSelFlags::SelectScopes;
    if (bSrc)
        eFlags = tCIDLib::eOREnumBits<tCQCTreeBrws::EFSelFlags>(eFlags, tCQCTreeBrws::EFSelFlags::Edit);

    TString strSelected;
    const tCIDLib::TBoolean bRes = facCQCTreeBrws().bSelectFile
    (
        *this
        , bSrc ? L"Select Source Scope" : L"Select Target Scope"
        , tCQCRemBrws::EDTypes::Template
        , bSrc ? L"/" : L"/User"
        , facCQCAdmin.cuctxToUse()
        , eFlags
        , strSelected
    );

    if (bRes)
    {
        // If the target, it has to be not in the /User area, but under it
        if (!bSrc && !strSelected.bStartsWithI(L"/User/"))
        {
            TErrBox msgbErr(facCQCAdmin.strMsg(kCQCAErrs::errcScale_BadTarScope));
            msgbErr.ShowIt(*this);
            return;
        }

        if (bSrc)
            m_pwndSrcScope->strWndText(strSelected);
        else
            m_pwndTarScope->strWndText(strSelected);
    }
}

