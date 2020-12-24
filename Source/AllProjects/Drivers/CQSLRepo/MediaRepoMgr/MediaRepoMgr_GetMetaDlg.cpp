//
// FILE NAME: MediaRepoMgr_GetMetaDlg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/20/2006
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
//  This is the header for the MediaRepoMGr_GetMetaDlg.cpp file, which implements a
//  small dialog box that is used to allow the user to select a media repository to
//  manage.
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
#include    "MediaRepoMgr_GetMetaDlg.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TGetMetaDataDlg,TDlgBox)



// ---------------------------------------------------------------------------
//   CLASS: TGetMetaDataDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TGetMetaDataDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TGetMetaDataDlg::TGetMetaDataDlg() :

    // Reset the flags that we use to track what we've gotten so far
    m_bGotImage(kCIDLib::False)
    , m_pcolCatNames(nullptr)
    , m_pcolItems(nullptr)
    , m_pimgToFill(nullptr)
    , m_pmcolToFill(nullptr)
    , m_pwndCancel(nullptr)
    , m_thrScrape
      (
        TString(L"TGetMetaDataDlg")
        , TMemberFunc<TGetMetaDataDlg>(this, &TGetMetaDataDlg::eScrapeThread)
      )
    , m_tmidUpdate(kCIDCtrls::tmidInvalid)
{
}

TGetMetaDataDlg::~TGetMetaDataDlg()
{
}


// ---------------------------------------------------------------------------
//  TGetMetaDataDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TGetMetaDataDlg::bRun(  const   TWindow&                    wndOwner
                        , const TString&                    strVolume
                        ,       TMediaCollect&              mcolToFill
                        ,       tCQCMedia::TItemList&       colItems
                        ,       tCIDLib::TStrList&          colCatNames
                        ,       TJPEGImage&                 imgToFill
                        , const TKrnlRemMedia::TCDTOCInfo&  TOCData)
{
    // Store the incoming stuff for use below
    m_pcolItems = &colItems;
    m_pimgToFill = &imgToFill;
    m_pcolCatNames = &colCatNames;
    m_pmcolToFill = &mcolToFill;
    m_strVolume = strVolume;
    m_TOCData = TOCData;

    // Fault in the player instance if not already
    if (!s_pwmpmsData)
        s_pwmpmsData = new TCQCWMPMetaSrc;

    // Make sure the cat name and item lists start clean
    colItems.RemoveAll();
    colCatNames.RemoveAll();

    // Start the load process
    m_bGotImage = kCIDLib::False;
    s_pwmpmsData->StartLoad(strVolume);

    // Set the timeout time
    m_enctEnd = TTime::enctNow() + (30 * kCIDLib::enctOneSecond);

    // And start the dialog
    const tCIDLib::TCard4 c4Res = c4RunDlg
    (
        wndOwner, facMediaRepoMgr, kMediaRepoMgr::ridDlg_MetaWait
    );

    // Make sure the thread is all the way down
    WaitThreadEnd();

    // 1 means success, 0 failure, and 2 cancel
    return (c4Res == 1);
}


// ---------------------------------------------------------------------------
//  TGetMetaDataDlg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TGetMetaDataDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRes = TParent::bCreated();

    // Get typed pointers to some of the widgets
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_MetaWait_Cancel, m_pwndCancel);

    // Setup handlers
    m_pwndCancel->pnothRegisterHandler(this, &TGetMetaDataDlg::eClickHandler);

    // Start up the timer to check the progress every half second
    m_eStatus = EStatus::WaitPlayer;
    m_tmidUpdate = tmidStartTimer(500);

    return bRes;
}


//
//  We watch for changes in the status and end the dialog when we succeed
//  or fail or time out.
//
tCIDLib::TVoid TGetMetaDataDlg::Timer(const tCIDCtrls::TTimerId tmidToDo)
{
    // Shouldn't happen, but just in case
    if (tmidToDo != m_tmidUpdate)
        return;

    if (m_eStatus == EStatus::WaitPlayer)
    {
        try
        {
            // See if we've completed
            tCIDLib::TBoolean bDone = s_pwmpmsData->bCompleteLoad
            (
                m_strVolume
                , *m_pmcolToFill
                , *m_pimgToFill
                , *m_pcolItems
                , *m_pcolCatNames
                , m_strInfoURL
            );

            // If so, do the cleanup processing
            if (bDone)
            {
                if (m_pmcolToFill->eType() == tCQCMedia::EMediaTypes::Music)
                {
                    //
                    //  See if the track count matches. If so, then set all
                    //  the track times and accumlate them all and set that
                    //  on the collection.
                    //
                    const tCIDLib::TCard4 c4TCnt = tCIDLib::MinVal<tCIDLib::TCard4>
                    (
                        (m_TOCData.c1LastTrack - m_TOCData.c1FirstTrack) + 1
                        , m_pcolItems->c4ElemCount()
                    );

                    tCIDLib::TCard4 c4Accum = 0;
                    tCIDLib::TCard4 c4StartBlock;
                    tCIDLib::TCard4 c4BlockCnt;
                    tCIDLib::TCard4 c4Minutes;
                    tCIDLib::TCard4 c4Seconds;
                    for (tCIDLib::TCard4 c4Ind= 0; c4Ind < c4TCnt; c4Ind++)
                    {
                        TMediaItem& mitemCur = m_pcolItems->objAt(c4Ind);
                        TKrnlRemMedia::bExtractCDTrackInfo
                        (
                            m_TOCData
                            , c4Ind
                            , c4StartBlock
                            , c4BlockCnt
                            , c4Minutes
                            , c4Seconds
                        );
                        mitemCur.c4Duration
                        (
                            (c4Minutes * 60) + c4Seconds
                        );
                        c4Accum += mitemCur.c4Duration();
                    }
                    m_pmcolToFill->c4Duration(c4Accum);
                }


                //
                //  Now we need to start the thread and change the status
                //  to waiting for the scrape to end.
                //
                m_thrScrape.Start();
                m_eStatus = EStatus::WaitScrape;
            }
             else
            {
                // Nothing yet, see if we've timed out
                if (TTime::enctNow() >= m_enctEnd)
                {
                    if (m_tmidUpdate != kCIDCtrls::tmidInvalid)
                    {
                        StopTimer(m_tmidUpdate);
                        m_tmidUpdate = kCIDCtrls::tmidInvalid;
                    }
                    TErrBox msgbFailed
                    (
                        L"No metadata was receieved. Have you enabled WMP to "
                        L"retrieve information from the internet?"
                    );
                    msgbFailed.ShowIt(*this);
                    EndDlg(0);
                }
            }
        }

        catch(TError& errToCatch)
        {
            if (m_tmidUpdate != kCIDCtrls::tmidInvalid)
            {
                StopTimer(m_tmidUpdate);
                m_tmidUpdate = kCIDCtrls::tmidInvalid;
            }

            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);

            TErrBox msgbFailed(L"An exception occured while getting metadata");
            msgbFailed.ShowIt(*this);

            EndDlg(0);
        }

        catch(...)
        {
            if (m_tmidUpdate != kCIDCtrls::tmidInvalid)
            {
                StopTimer(m_tmidUpdate);
                m_tmidUpdate = kCIDCtrls::tmidInvalid;
            }

            facMediaRepoMgr.LogMsg
            (
                CID_FILE
                , CID_LINE
                , L"Unknown exception occured while getting metadata"
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );

            TErrBox msgbFailed(L"An unknown exception occured while getting metadata");
            msgbFailed.ShowIt(*this);

            EndDlg(0);
        }
    }
     else if (m_eStatus == EStatus::WaitScrape)
    {
        // Nothing to do, just waiting for the thread to end
    }
     else if (m_eStatus == EStatus::Done)
    {
        // Stop the timer
        if (m_tmidUpdate != kCIDCtrls::tmidInvalid)
        {
            StopTimer(m_tmidUpdate);
            m_tmidUpdate = kCIDCtrls::tmidInvalid;
        }

        // The scrape thread has completed, so end the dialog
        EndDlg(1);
    }
     else if (m_eStatus == EStatus::Cancel)
    {
        // Stop the timer
        if (m_tmidUpdate != kCIDCtrls::tmidInvalid)
        {
            StopTimer(m_tmidUpdate);
            m_tmidUpdate = kCIDCtrls::tmidInvalid;
        }

        // It was cancelled, so end with cancel status
        EndDlg(2);
    }
}


// ---------------------------------------------------------------------------
//  TGetMetaDataDlg: Private, static data
// ---------------------------------------------------------------------------
TCQCWMPMetaSrc* TGetMetaDataDlg::s_pwmpmsData;


// ---------------------------------------------------------------------------
//  TGetMetaDataDlg: Private, non-virtual methods
// ---------------------------------------------------------------------------

tCIDCtrls::EEvResponses TGetMetaDataDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kMediaRepoMgr::ridDlg_MetaWait_Cancel)
    {
        //
        //  Force the status to cancel. This will make the timer end and exit
        //  the dailog.
        //
        m_eStatus = EStatus::Cancel;
    }
    return tCIDCtrls::EEvResponses::Handled;
}


tCIDLib::EExitCodes
TGetMetaDataDlg::eScrapeThread(TThread& thrThis, tCIDLib::TVoid*)
{
    // Let the calling thread go
    thrThis.Sync();

    // Get a convenience flag for the type of media
    const tCIDLib::TBoolean bIsMusic(m_pmcolToFill->eType() == tCQCMedia::EMediaTypes::Music);

    try
    {
        // If we got a more info URL, then do that first
        if (!m_bGotImage && !m_strInfoURL.bIsEmpty())
            m_bGotImage = facMediaRepoMgr.bScrapeMSData(m_strInfoURL, *m_pimgToFill);

        if (!m_bGotImage && !thrThis.bCheckShutdownRequest())
        {
            m_bGotImage = facMediaRepoMgr.bScrapeDCSData
            (
                bIsMusic, m_pmcolToFill->strArtist(), m_pmcolToFill->strName(), *m_pimgToFill
            );
        }

        // If we still haven't gotten the art, then try another source
        if (!m_bGotImage && !thrThis.bCheckShutdownRequest())
        {
            m_bGotImage = facMediaRepoMgr.bScrapeAAOData
            (
                m_pmcolToFill->strArtist(), *m_pimgToFill
            );
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        m_eStatus = EStatus::Done;
        return tCIDLib::EExitCodes::RuntimeError;
    }

    catch(...)
    {
        m_eStatus = EStatus::Done;
        return tCIDLib::EExitCodes::SystemException;
    }

    // If not set to cancel already, set to done
    if (m_eStatus != EStatus::Cancel)
        m_eStatus = EStatus::Done;
    return tCIDLib::EExitCodes::Normal;
}


tCIDLib::TVoid TGetMetaDataDlg::WaitThreadEnd()
{
    //
    //  Let it throw should this somehow fail, since it's better to let
    //  the program go down than have a thread running off in the ozone.
    //
    if (m_thrScrape.bIsRunning())
    {
        m_thrScrape.ReqShutdownSync(12000);
        m_thrScrape.eWaitForDeath(5000);
    }
}

