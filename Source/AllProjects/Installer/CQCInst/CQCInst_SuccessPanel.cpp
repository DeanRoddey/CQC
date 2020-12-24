//
// FILE NAME: CQCInst_SuccessPanel.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/03/2004
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
//  This file implements the info panel that shows the success/failure
//  status.
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
RTTIDecls(TInstSuccessPanel,TInstInfoPanel)



// ---------------------------------------------------------------------------
//   CLASS: TInstSuccessPanel
//  PREFIX: pan
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TInstSuccessPanel: Constructors and Destructor
// ---------------------------------------------------------------------------
TInstSuccessPanel::TInstSuccessPanel(TCQCInstallInfo* const pinfoCur
                                    , TMainFrameWnd* const  pwndParent) :

    TInstInfoPanel(L"Success", pinfoCur, pwndParent)
    , m_bStatus(kCIDLib::False)
    , m_pwndSaveWarn(nullptr)
    , m_pwndText(nullptr)
    , m_pwndWarnList(nullptr)
    , m_pwndWarnRes(nullptr)
{
}

TInstSuccessPanel::~TInstSuccessPanel()
{
}


// ---------------------------------------------------------------------------
//  TInstSuccessPanel: Public, inherited methods
// ---------------------------------------------------------------------------

tCQCInst::ENavFlags TInstSuccessPanel::eNavFlags() const
{
    // This is the end, so they can just close now
    return tCQCInst::ENavFlags::AllowClose;
}


tCIDLib::TVoid TInstSuccessPanel::Entered()
{
    // Though we currently never re-enter this guy, flush the list first in case
    m_pwndWarnList->RemoveAll();

    // Load up the image/text according to the status
    TString strToLoad;
    if (facCQCInst.colWarnings().bIsEmpty())
    {
        if (m_bStatus)
        {
            m_pwndImg->SetAppImage(L"CQCApp", tCIDCtrls::EAppImgSz::Large);
            strToLoad.LoadFromMsg(kCQCInstMsgs::midPan_Success_Success, facCQCInst);
        }
         else
        {
            // Get the log file path and remove the file name part
            TPathStr pathLogs(facCQCInst.strLogPath());
            pathLogs.bRemoveNameExt();

            m_pwndImg->SetAppImage(L"CQCAppGray", tCIDCtrls::EAppImgSz::Large);
            strToLoad.LoadFromMsg(kCQCInstMsgs::midPan_Success_Failure, facCQCInst, pathLogs);
        }

        // Hide the warnings list and save button
        m_pwndWarnList->SetVisibility(kCIDLib::False);
        m_pwndSaveWarn->SetVisibility(kCIDLib::False);
    }
     else
    {
        m_pwndImg->SetAppImage(L"CQCAppGray", tCIDCtrls::EAppImgSz::Large);
        strToLoad.LoadFromMsg(kCQCInstMsgs::midPan_Success_Warn, facCQCInst);

        // Show the warnings list and save button
        m_pwndWarnList->SetVisibility(kCIDLib::True);
        m_pwndSaveWarn->SetVisibility(kCIDLib::True);

        // Load up the warning list
        const TVector<TInstWarning>& colList = facCQCInst.colWarnings();
        const tCIDLib::TCard4 c4Count = colList.c4ElemCount();
        if (c4Count)
        {
            tCIDLib::TStrList colCols(1);
            colCols.objAdd(TString::strEmpty());
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            {
                const TInstWarning& warnCur = colList[c4Index];
                colCols[0] = warnCur.strMsg();
                m_pwndWarnList->c4AddItem(colCols, c4Index);
            }

            // Select the 0th and force an event so the resource text gets initialized
            m_pwndWarnList->SelectByIndex(0, kCIDLib::True);
        }
    }
    m_pwndText->strWndText(strToLoad);
}


// We watch for the install status and set our m_bStatus flag appropriately
tCIDLib::TVoid TInstSuccessPanel::ReactToChanges()
{
    m_bStatus = (infoCur().eInstStatus() == tCQCInst::EInstStatus::Done);
}


// ---------------------------------------------------------------------------
//  TInstSuccessPanel: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TInstSuccessPanel::bCreated()
{
    TParent::bCreated();

    // Load our controls
    LoadDlgItems(kCQCInst::ridPan_Success);

    // Get some pointers to the ones we want to interact with
    CastChildWnd(*this, kCQCInst::ridPan_Success_Image, m_pwndImg);
    CastChildWnd(*this, kCQCInst::ridPan_Success_SaveWarn, m_pwndSaveWarn);
    CastChildWnd(*this, kCQCInst::ridPan_Success_Text, m_pwndText);
    CastChildWnd(*this, kCQCInst::ridPan_Success_WarnList, m_pwndWarnList);
    CastChildWnd(*this, kCQCInst::ridPan_Success_WarnRes, m_pwndWarnRes);

    // Install event handlers for our warning list and save button
    m_pwndWarnList->pnothRegisterHandler(this, &TInstSuccessPanel::eListHandler);
    m_pwndSaveWarn->pnothRegisterHandler(this, &TInstSuccessPanel::eClickHandler);

    //
    //  We load our content on each entry since we don't know the status till then.
    //  But we do need to so set up the columns on the list.
    //
    tCIDLib::TStrList colCols(1);
    colCols.objAdd(L"");
    m_pwndWarnList->SetColumns(colCols);

    return kCIDLib::True;
}


// ---------------------------------------------------------------------------
//  TInstSuccessPanel: Private, non-virtual methods
// ---------------------------------------------------------------------------

tCIDCtrls::EEvResponses TInstSuccessPanel::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCInst::ridPan_Success_SaveWarn)
    {
        // Invoke the file save dialog. If they commit, format the info out
        tCIDLib::TKVPList colFileTypes(1);
        colFileTypes.objAdd(TKeyValuePair(L"Text Files", L"*.Txt"));

        TString strFileName(L"CQCInstallWarnings.Txt");
        TString strSelected;
        const tCIDLib::TBoolean bRes = facCIDCtrls().bSaveFileDlg
        (
            *this
            , L"Select a Target File"
            , TString::strEmpty()
            , strFileName
            , strSelected
            , colFileTypes
            , tCIDCtrls::EFSaveOpts::FilesOnly
        );

        if (bRes)
        {
            TTextFileOutStream strmTar
            (
                strSelected
                , tCIDLib::ECreateActs::CreateAlways
                , tCIDLib::EFilePerms::Default
                , tCIDLib::EFileFlags::SequentialScan
                , tCIDLib::EAccessModes::Excl_Write
            );

            const TCQCInstallInfo& infoDump = infoCur();
            strmTar << L"CQC Installation Warning Dump\n"
                    << L"    From Version: " << infoDump.viiOldInfo().c4MajVersion()
                    << kCIDLib::chPeriod << infoDump.viiOldInfo().c4MinVersion()
                    << kCIDLib::chPeriod << infoDump.viiOldInfo().c4Revision()
                    << L"\n    From Version: " << infoDump.viiNewInfo().c4MajVersion()
                    << kCIDLib::chPeriod << infoDump.viiNewInfo().c4MinVersion()
                    << kCIDLib::chPeriod << infoDump.viiNewInfo().c4Revision()

                    << kCIDLib::DNewLn
                    << L"The following warnings occurred\n"
                    << L"----------------------------------------\n";

            const TVector<TInstWarning>& colList = facCQCInst.colWarnings();
            const tCIDLib::TCard4 c4Count = colList.c4ElemCount();
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            {
                const TInstWarning& warnCur = colList[c4Index];

                strmTar << warnCur.strMsg() << kCIDLib::NewLn;

                strmTar.c4Indent(4);

                if (!warnCur.strResource().bIsEmpty())
                    strmTar << warnCur.strResource() << kCIDLib::NewLn;
                if (warnCur.bHasExcept())
                    strmTar << warnCur.errException() << kCIDLib::NewLn;

                strmTar.c4Indent(0);
                strmTar << kCIDLib::DNewLn;
            }

            strmTar << kCIDLib::FlushIt;
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}


// When the component combo box selection changes, we display some info about the new one
tCIDCtrls::EEvResponses TInstSuccessPanel::eListHandler(TListChangeInfo& wnotEvent)
{
    if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::SelChanged)
    {
        // Update the warning resource line for the newly selected one
        m_pwndWarnRes->strWndText(facCQCInst.colWarnings()[wnotEvent.c4Index()].strResource());
    }
    return tCIDCtrls::EEvResponses::Handled;
}
