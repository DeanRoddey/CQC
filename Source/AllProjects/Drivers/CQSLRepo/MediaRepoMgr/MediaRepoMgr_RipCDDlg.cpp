//
// FILE NAME: MediaRepoMgr_RipCD.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 05/04/2006
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
//  This is the header for the MediaRepoMgr_RipCDDlg.cpp file, which
//  implements a dialog box that drives the CD ripping process and displays
//  status information.
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
#include    "MediaRepoMgr.hpp"
#include    "MediaRepoMgr_RipCDDlg.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TRipCDDlg,TDlgBox)



// ---------------------------------------------------------------------------
//   CLASS: TRipCDDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TRipCDDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TRipCDDlg::TRipCDDlg() :

    m_bBreak(kCIDLib::False)
    , m_c4CurItem(1)
    , m_c4LastPercent(0)
    , m_c4LastItem(1)
    , m_c4ItemCnt(0)
    , m_eLastStatus(EStatus::WaitStart)
    , m_eStatus(EStatus::Initializing)
    , m_pcolFiles(nullptr)
    , m_pmdbEdit(nullptr)
    , m_pstrError(nullptr)
    , m_pwndCancel(nullptr)
    , m_pwndCurName(nullptr)
    , m_pwndCurItem(nullptr)
    , m_pwndProgress(nullptr)
    , m_strCurItemMsg(kMRMgrMsgs::midDlg_RipCD_CurItem, facMediaRepoMgr)
    , m_thrWorker
      (
        TString(L"RipCDDlgThread")
        , TMemberFunc<TRipCDDlg>(this, &TRipCDDlg::eWorkerThread)
      )
    , m_tmidUpdate(kCIDCtrls::tmidInvalid)
{
}

TRipCDDlg::~TRipCDDlg()
{
}


// ---------------------------------------------------------------------------
//  TRipCDDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TRipCDDlg::bRun(            TMainFrameWnd&          wndOwner
                    , const TString&                strVolume
                    ,       TString&                strError
                    ,       TMediaDB&               mdbEdit
                    , const tCQCMedia::EMediaTypes  eMType
                    , const tCIDLib::TCard2         c2ColId
                    , const TString&                strCatName
                    ,       TVector<TString>&       colFiles)
{
    //
    //  Make sure that the current output codec and format has been
    //  select, or is still valid. If not, tell them that they need to
    //  set it and pop up the settings dialog. WE need to create an
    //  encoder to do the check.
    //
    m_mrsRip = wndOwner.mrsCurrent();
    {
        //
        //  Note that for now we just create a WMA encoder, but later we'll
        //  also have a selection of encoders and we'll have to go look
        //  one up based on configuration settings.
        //
        TCIDDAEWMAEnc* pdaeeInfo = new TCIDDAEWMAEnc;
        TJanitor<TCIDDAEWMAEnc> janEnc(pdaeeInfo);

        if (!pdaeeInfo->bCodecInfoValid(m_mrsRip.strCodec(), m_mrsRip.strCodecFmt()))
        {
            {
                TOkBox msgbSetEnc
                (
                    facMediaRepoMgr.strMsg(kMRMgrMsgs::midMain_FrameTitle)
                    , facMediaRepoMgr.strMsg(kMRMgrMsgs::midPrompt_SetEncodingFormat)
                );
                msgbSetEnc.ShowIt(wndOwner);
            }

            //
            //  Invoke the settings dialog. If they bail out, then we bail out since
            //  we cannot move forward without valid encoding format info.
            //
            if (!wndOwner.bEditRepoSettings())
                return kCIDLib::False;

            // Update our copy
            m_mrsRip = wndOwner.mrsCurrent();
        }
    }

    // Store incoming stuff for later use
    m_pcolFiles     = &colFiles;
    m_pmdbEdit      = &mdbEdit;
    m_pstrError     = &strError;
    m_strCatName    = strCatName;
    m_strVolume     = strVolume;
    m_mcolCD        = *mdbEdit.pmcolById(eMType, c2ColId, kCIDLib::True);

    const tCIDLib::TCard4 c4Res = c4RunDlg
    (
        wndOwner, facMediaRepoMgr, kMediaRepoMgr::ridDlg_RipCD
    );

    // Make sure the worker thread is shut down
    WaitThreadEnd();

    //
    //  The code will be:
    //
    //  0 = It failed
    //  1 = It succeeded
    //  2 = They cancelled
    //
    return (c4Res == 1);
}


// ---------------------------------------------------------------------------
//  TRipCDDlg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TRipCDDlg::bCancelRequest()
{
    // If running, then set the flag and prevent standard cancel
    if (m_eStatus != EStatus::WaitStart)
    {
        m_bBreak = kCIDLib::True;
        return kCIDLib::False;
    }

    // Allow the standard cancel to occur
    return kCIDLib::True;
}


tCIDLib::TBoolean TRipCDDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRet = TParent::bCreated();

    // Get typed pointers to some of the widgets
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_RipCD_Cancel, m_pwndCancel);
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_RipCD_CurName, m_pwndCurName);
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_RipCD_CurItem, m_pwndCurItem);
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_RipCD_Progress, m_pwndProgress);

    // Install handlers
    m_pwndCancel->pnothRegisterHandler(this, &TRipCDDlg::eClickHandler);

    //  Start up the timer and start up the worker thread.
    m_tmidUpdate = tmidStartTimer(250);
    m_thrWorker.Start();

    return bRet;
}


tCIDLib::TVoid TRipCDDlg::Destroyed()
{
    if (m_tmidUpdate != kCIDCtrls::tmidInvalid)
    {
        StopTimer(m_tmidUpdate);
        m_tmidUpdate = kCIDCtrls::tmidInvalid;
    }

    TParent::Destroyed();
}



//
//  We watch for changes in the status and either update the status winodw
//  to reflect the status, or when we see a fail/done status, we dismiss
//  ourself.
//
tCIDLib::TVoid TRipCDDlg::Timer(const tCIDCtrls::TTimerId tmidToDo)
{
    // We should only ever see our timer, but just in case
    if (tmidToDo != m_tmidUpdate)
        return;

    //
    //  If it's not changed since last time, and it's not working status
    //  then do nothing.
    //
    if ((m_eStatus == m_eLastStatus) && (m_eStatus != EStatus::Working))
        return;

    // Store the new status
    m_eLastStatus = m_eStatus;

    // Based on the status, do what we need to do
    switch(m_eStatus)
    {
        case EStatus::Initializing :
            // Show the initializing status string
            m_pwndCurItem->strWndText(facMediaRepoMgr.strMsg(kMRMgrMsgs::midDlg_RipCD_Init));
            break;

        case EStatus::Working :
        {
            //
            //  Update any info that has changed since the last time. Start
            //  with the percentage done on the curren track. NOte that the
            //  ripper class is designed to allow us to access the percent
            //  done value without synchronizing.
            //
            const tCIDLib::TCard4 c4NewPer = m_daerWorker.c4PercentDone();
            if (m_c4LastPercent != c4NewPer)
            {
                m_pwndProgress->SetValue(c4NewPer);
                m_c4LastPercent = c4NewPer;
            }

            if (m_c4LastItem != m_c4CurItem)
            {
                m_strTmpFmt = m_strCurItemMsg;
                m_strTmpFmt.eReplaceToken(m_c4CurItem + 1UL, L'1');
                m_strTmpFmt.eReplaceToken(m_c4ItemCnt, L'2');
                m_pwndCurItem->strWndText(m_strTmpFmt);

                if (m_c4CurItem < m_c4ItemCnt)
                {
                    const tCIDLib::TCard2 c2ItemId = m_mcolCD.c2ItemIdAt(m_c4CurItem);
                    const TMediaItem* pmitemCur = m_pmdbEdit->pmitemById
                    (
                        m_mcolCD.eType(), c2ItemId, kCIDLib::True
                    );
                    m_pwndCurName->strWndText(pmitemCur->strName());
                }
                m_c4LastItem = m_c4CurItem;
            }
            break;
        }

        case EStatus::Failed :
            // Terminate with a code of 0
            EndDlg(0);
            break;

        case EStatus::Cancelled :
            // Terminate with a code of 2
            m_pstrError->LoadFromMsg
            (
                kMRMgrMsgs::midStatus_RipCancelled, facMediaRepoMgr
            );
            EndDlg(2);
            break;

        case EStatus::Done :
            // Terminate with a code of 1
            EndDlg(1);
            break;
    };
}



// ---------------------------------------------------------------------------
//  TRipCDDlg: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDCtrls::EEvResponses TRipCDDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kMediaRepoMgr::ridDlg_RipCD_Cancel)
    {
        //
        //  If we've not started the worker thread, then just do the usual
        //  dialog loop exit. Else, set the break flag. The thread will exit
        //  with a cancel status, and the timer will in turn exit the dialog
        //  with a cancel status.
        //
        if (m_eStatus == EStatus::WaitStart)
            EndDlg(2);
        else
            m_bBreak = kCIDLib::True;
    }
    return tCIDCtrls::EEvResponses::Handled;
}


tCIDLib::EExitCodes
TRipCDDlg::eWorkerThread(TThread& thrThis, tCIDLib::TVoid*)
{
    // Let the calling thread go
    thrThis.Sync();

    try
    {
        // Set the initializing state
        m_eStatus = EStatus::Initializing;

        // Make sure the tmp output directory exists
        if (!TFileSys::bExists(facMediaRepoMgr.strRipDir()))
            TFileSys::MakePath(facMediaRepoMgr.strRipDir());

        // Set up a WMA encoder
        TCIDDAEWMAEnc daeeOut;

        // Open the drive and get the TOC
        TKrnlRemMediaDrv rmmdCur(m_strVolume.pszBuffer());
        if (!rmmdCur.bOpen())
        {
            m_pstrError->LoadFromMsg
            (
                kMRMgrMsgs::midStatus_CantOpenDrv
                , facMediaRepoMgr
                , m_strVolume
            );
            m_eStatus = EStatus::Failed;
            return tCIDLib::EExitCodes::Normal;
        }

        // Try to query CD TOC info
        TKrnlRemMedia::TCDTOCInfo TOCInfo;
        if (!rmmdCur.bQueryCDTOC(TOCInfo))
        {
            m_pstrError->LoadFromMsg
            (
                kMRMgrMsgs::midStatus_CantGetTOC, facMediaRepoMgr
            );
            m_eStatus = EStatus::Failed;
            return tCIDLib::EExitCodes::Normal;
        }

        // Set the jitter correction mode
        m_daerWorker.eJitterOpt
        (
            m_mrsRip.bJitterCorrection() ? tCIDDAE::EJitterOpts::Always
                                         : tCIDDAE::EJitterOpts::None
        );

        //
        //  OK, we can know go into a loop until we've done all of the
        //  tracks. So init all of the current/last state stuff and set
        //  the status to indicate we are working. The timer will now start
        //  showing status.
        //
        //  Set the 'last' ones to an obviously invalid value, so that the
        //  first valid values that get set will be seen as a change and the
        //  displays will show.
        //
        m_c4LastPercent = kCIDLib::c4MaxCard;
        m_c4LastItem = kCIDLib::c4MaxCard;
        TPathStr pathItem;
        TPathStr pathName;
        m_eStatus = EStatus::Working;

        //
        //  Don't trust the TOC, which can be purposefully incorrect. Go
        //  by the metadta item count, unless it's alrger than the TOC
        //  count.
        //
        const tCIDLib::TCard4 c4TOCCnt
        (
            (TOCInfo.c1LastTrack - TOCInfo.c1FirstTrack) + 1
        );
        m_c4CurItem = 0;
        m_c4ItemCnt = m_mcolCD.c4ItemCount();
        if (m_c4ItemCnt > c4TOCCnt)
            m_c4ItemCnt = c4TOCCnt;

        for (; m_c4CurItem < m_c4ItemCnt; m_c4CurItem++)
        {
            const tCIDLib::TCard2 c2ItemId = m_mcolCD.c2ItemIdAt(m_c4CurItem);

            // Get the media item for this track
            const TMediaItem* pmitemCur = m_pmdbEdit->pmitemById
            (
                m_mcolCD.eType(), c2ItemId, kCIDLib::True
            );

            //
            //  Build up the file name for this track. We format info into it that
            //  the server can use to know who it belongs to. We'll upload the
            //  file name with the data for this reason. It also let's us know what
            //  stuff is in our local upload directory.
            //
            facMediaRepoMgr.BuildUploadMediaPath
            (
                tCQCMedia::EMediaTypes::Music
                , tCQCMedia::EDataTypes::Item
                , c2ItemId
                , m_c4CurItem + 1
                , L"wma"
                , pathName
            );

            // Build up the full track path for our local storage
            pathItem = facMediaRepoMgr.strRipDir();
            pathItem.AddLevel(pathName);

            // Add it to the outgoing file list
            m_pcolFiles->objAdd(pathItem);

            // Now do the rip, passing the full path to the temp file
            m_daerWorker.RipTrack
            (
                rmmdCur
                , m_mrsRip.strCodec()
                , m_mrsRip.strCodecFmt()
                , m_c4CurItem
                , daeeOut
                , pathItem
                , m_bBreak
                , m_mcolCD.strName()
                , pmitemCur->strName()
                , pmitemCur->strArtist()
                , m_mcolCD.strLabel()
                , m_strCatName
                , m_mcolCD.c4Year()
            );

            if (m_bBreak)
                break;
        }

        // Set the status to success or cancel
        if (m_bBreak)
            m_eStatus = EStatus::Cancelled;
        else
            m_eStatus = EStatus::Done;
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        m_pstrError->LoadFromMsg
        (
            kMRMgrMsgs::midStatus_Exception
            , facMediaRepoMgr
            , errToCatch.strErrText()
        );
        m_eStatus = EStatus::Failed;
        return tCIDLib::EExitCodes::Normal;
    }

    catch(...)
    {
        m_pstrError->LoadFromMsg
        (
            kMRMgrMsgs::midStatus_UnknownException
            , facMediaRepoMgr
        );
        m_eStatus = EStatus::Failed;
        return tCIDLib::EExitCodes::Normal;
    }
    return tCIDLib::EExitCodes::Normal;
}



tCIDLib::TVoid TRipCDDlg::WaitThreadEnd()
{
    //
    //  Let it throw should this somehow fail, since it's better to let
    //  the program go down than have a thread running off in the ozone.
    //
    if (m_thrWorker.bIsRunning())
        m_thrWorker.eWaitForDeath(10000);
}


