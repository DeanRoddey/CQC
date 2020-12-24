//
// FILE NAME: CQCGKit_DrvInfoDlg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/07/2016
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
//  This module implements the simple dialog that queries the driver config for a
//  given driver moniker and displays that info.
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
#include    "CQCGKit_.hpp"
#include    "CQCGKit_DrvInfoDlg_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TDrvInfoDlg,TDlgBox)



// ---------------------------------------------------------------------------
//  Our info tab is a public class defined elsewhere since it is something that
//  might be used by other things. Our stats tab is, so far, for our own use and
//  just defined here.
// ---------------------------------------------------------------------------
class TStatsTab : public TTabWindow
{
    public :
       // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TStatsTab
        (
            const   TString&                strMoniker
        );

        TStatsTab(const TStatsTab&) = delete;

        ~TStatsTab() {}


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TStatsTab& operator=(const TStatsTab&) = delete;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid AreaChanged
        (
            const   TArea&                  areaPrev
            , const TArea&                  areaNew
            , const tCIDCtrls::EPosStates   ePosState
            , const tCIDLib::TBoolean       bOrgChanged
            , const tCIDLib::TBoolean       bSizeChanged
            , const tCIDLib::TBoolean       bStateChanged
        )   override;

        tCIDLib::TBoolean bCreated() override;

        tCIDLib::TVoid Destroyed() override;

        tCIDLib::TVoid TabActivationChange
        (
            const   tCIDLib::TBoolean       bGetting
        )   override;

        tCIDLib::TVoid Timer
        (
            const   tCIDCtrls::TTimerId     tmidSrc
        )   override;



    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::EExitCodes ePollThread
        (
                    TThread&                thrThis
            ,       tCIDLib::TVoid*         pData
        );

        tCIDLib::TVoid SetStatSlot
        (
                    tCIDLib::TCard4&        c4CurVal
            , const tCIDLib::TCard4         c4NewVal
            , const tCIDLib::TCard4         c4SlotInd
        );


        // -------------------------------------------------------------------
        //  Private data members
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bIsActive;
        const tCIDLib::TCard4   m_c4RowCount;
        const tCIDLib::TCard4   m_c4Row_BadMsgs;
        const tCIDLib::TCard4   m_c4Row_FailedWrites;
        const tCIDLib::TCard4   m_c4Row_LoadTime;
        const tCIDLib::TCard4   m_c4Row_LostCommRes;
        const tCIDLib::TCard4   m_c4Row_LostConns;
        const tCIDLib::TCard4   m_c4Row_Naks;
        const tCIDLib::TCard4   m_c4Row_Reconfigures;
        const tCIDLib::TCard4   m_c4Row_Timeouts;
        const tCIDLib::TCard4   m_c4Row_ThreadId;
        const tCIDLib::TCard4   m_c4Row_UnknownMsgs;
        const tCIDLib::TCard4   m_c4Row_UnknownWrites;
        TMutex                  m_mtxSync;
        T2ColSectList*          m_pwndList;
        TString                 m_strMoniker;
        TString                 m_strFmt;
        tCIDCtrls::TTimerId     m_tmidPoll;
        TTime                   m_tmUpSince;
        TThread                 m_thrPoll;

        // Current values and new values left by the bgn thread
        TCQCDrvStats            m_cdstatsCur;
        TCQCDrvStats            m_cdstatsNew;
};

TStatsTab::TStatsTab(const TString& strMoniker) :

    TTabWindow(L"DrvStats", L"Driver Statistics", kCIDLib::False, kCIDLib::True)
    , m_bIsActive(kCIDLib::False)
    , m_c4RowCount(11)
    , m_c4Row_LoadTime(0)
    , m_c4Row_BadMsgs(1)
    , m_c4Row_FailedWrites(2)
    , m_c4Row_LostCommRes(3)
    , m_c4Row_LostConns(4)
    , m_c4Row_Naks(5)
    , m_c4Row_Reconfigures(6)
    , m_c4Row_Timeouts(7)
    , m_c4Row_ThreadId(8)
    , m_c4Row_UnknownMsgs(9)
    , m_c4Row_UnknownWrites(10)
    , m_pwndList(nullptr)
    , m_strMoniker(strMoniker)
    , m_thrPoll
      (
        TString(L"TStatsTabPoller"), TMemberFunc<TStatsTab>(this, &TStatsTab::ePollThread)
      )
    , m_tmidPoll(kCIDCtrls::tmidInvalid)
{
    m_tmUpSince.strDefaultFormat(TTime::strCTime());
}


tCIDLib::TVoid
TStatsTab::AreaChanged( const   TArea&                  areaPrev
                        , const TArea&                  areaNew
                        , const tCIDCtrls::EPosStates   ePosState
                        , const tCIDLib::TBoolean       bOrgChanged
                        , const tCIDLib::TBoolean       bSizeChanged
                        , const tCIDLib::TBoolean       bStateChanged)
{
    TTabWindow::AreaChanged(areaPrev, areaNew, ePosState, bOrgChanged, bSizeChanged, bStateChanged);

    // If size changed and we are not min and our list window exists, resize it
    if (bSizeChanged && (ePosState != tCIDCtrls::EPosStates::Minimized) && m_pwndList)
        m_pwndList->SetSize(areaClient().szArea(), kCIDLib::True);
}

tCIDLib::TBoolean TStatsTab::bCreated()
{
    TTabWindow::bCreated();

    //
    //  Create our list, sized to fit us with no border, and no columns since they are
    //  already set for us with the section list and the defaults are fine.
    //
    tCIDLib::TStrList colCols(2);
    m_pwndList = new T2ColSectList();
    m_pwndList->CreateMCLB
    (
        *this
        , kCIDCtrls::widFirstCtrl
        , areaClient()
        , colCols
        , 0
        , tCIDCtrls::EWndStyles::VisTabChild
        , tCIDLib::eOREnumBits
          (
            tCIDCtrls::EMCLBStyles::AutoLastCol
            , tCIDCtrls::EMCLBStyles::FullRowSel
          )
        , tCIDCtrls::EExWndStyles::None
    );

    // Load initial rows for the stats values, with question marks for the value
    const TString strVal(L"??");
    m_pwndList->AddDataRow(L"Load Time:", strVal, m_c4Row_LoadTime);
    m_pwndList->AddDataRow( L"Bad Msgs:", strVal, m_c4Row_BadMsgs);
    m_pwndList->AddDataRow(L"Failed Writes:", strVal, m_c4Row_FailedWrites);
    m_pwndList->AddDataRow(L"Lost Comm Res:", strVal, m_c4Row_LostCommRes);
    m_pwndList->AddDataRow(L"Lost Conn:", strVal, m_c4Row_LostConns);
    m_pwndList->AddDataRow(L"Neg. Acks:", strVal, m_c4Row_Naks);
    m_pwndList->AddDataRow(L"Reconfigures:", strVal, m_c4Row_Reconfigures);
    m_pwndList->AddDataRow(L"Timeouts:", strVal, m_c4Row_Timeouts);
    m_pwndList->AddDataRow( L"Thread Id:", strVal, m_c4Row_ThreadId);
    m_pwndList->AddDataRow( L"Unknown Msgs:", strVal, m_c4Row_UnknownMsgs);
    m_pwndList->AddDataRow( L"Unknown Writes:", strVal, m_c4Row_UnknownWrites);

    // Size the left column to fit the values we loaded
    m_pwndList->AutoSizeCol(1, kCIDLib::False);

    // Do an initial autosize on the last column to size it up to fit
    m_pwndList->AutoSizeCol(2, kCIDLib::True);

    // Create a timer to update the stats
    m_tmidPoll = tmidStartTimer(1000);

    // And let's start the bgn thread
    m_thrPoll.Start();

    return kCIDLib::True;
}


tCIDLib::TVoid TStatsTab::Destroyed()
{
    // Stop our bgn thread and update timer
    if (m_tmidPoll != kCIDCtrls::tmidInvalid)
    {
        StopTimer(m_tmidPoll);
        m_tmidPoll = kCIDCtrls::tmidInvalid;
    }

    if (m_thrPoll.bIsRunning())
    {
        try
        {
            TWndPtrJanitor janPtr(tCIDCtrls::ESysPtrs::Wait);
            m_thrPoll.ReqShutdownSync(10000);
            m_thrPoll.eWaitForDeath(5000);
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
    }

    TTabWindow::Destroyed();
}


//
//  If getting activated, remember this. The bgn thread will poll quicker when we are
//  active.
//
tCIDLib::TVoid TStatsTab::TabActivationChange(const tCIDLib::TBoolean bGetting)
{
    m_bIsActive = bGetting;
}


tCIDLib::TVoid TStatsTab::Timer(const tCIDCtrls::TTimerId tmidSrc)
{
    if (tmidSrc == m_tmidPoll)
    {
        // Lock and get a copy of the new stats values
        TCQCDrvStats cdstatsTest;
        {
            TLocker lockrSync(&m_mtxSync);
            cdstatsTest = m_cdstatsNew;
        }

        // If any differences, then let's update them
        TWndPaintJanitor janList(m_pwndList);

        if (cdstatsTest.m_c8LoadTime != m_cdstatsCur.m_c8LoadTime)
        {
            m_cdstatsCur.m_c8LoadTime = cdstatsTest.m_c8LoadTime;
            m_tmUpSince.enctTime(cdstatsTest.m_c8LoadTime);
            if (cdstatsTest.m_c8LoadTime == kCIDLib::c8MaxCard)
                m_strFmt = L"??";
            else
                m_tmUpSince.FormatToStr(m_strFmt);
            m_pwndList->SetColText(m_c4Row_LoadTime, 2, m_strFmt);
        }

        SetStatSlot(m_cdstatsCur.m_c4BadMsgs, cdstatsTest.m_c4BadMsgs, m_c4Row_BadMsgs);
        SetStatSlot(m_cdstatsCur.m_c4FailedWrites, cdstatsTest.m_c4FailedWrites, m_c4Row_FailedWrites);
        SetStatSlot(m_cdstatsCur.m_c4LostCommRes, cdstatsTest.m_c4LostCommRes, m_c4Row_LostCommRes);
        SetStatSlot(m_cdstatsCur.m_c4LostConns, cdstatsTest.m_c4LostConns, m_c4Row_LostConns);
        SetStatSlot(m_cdstatsCur.m_c4Naks, cdstatsTest.m_c4Naks, m_c4Row_Naks);
        SetStatSlot(m_cdstatsCur.m_c4Reconfigures, cdstatsTest.m_c4Reconfigures, m_c4Row_Reconfigures);
        SetStatSlot(m_cdstatsCur.m_c4Timeouts, cdstatsTest.m_c4Timeouts, m_c4Row_Timeouts);

        if (cdstatsTest.m_c4ThreadId != m_cdstatsCur.m_c4ThreadId)
        {
            m_cdstatsCur.m_c4ThreadId = cdstatsTest.m_c4ThreadId;

            if (m_cdstatsCur.m_c4ThreadId == kCIDLib::c4MaxCard)
            {
                m_strFmt = L"??";
            }
             else
            {
                m_strFmt.SetFormatted(cdstatsTest.m_c4ThreadId);
                m_strFmt.Append(L" (0x");
                m_strFmt.AppendFormatted(cdstatsTest.m_c4ThreadId, tCIDLib::ERadices::Hex);
                m_strFmt.Append(L")");
            }
            m_pwndList->SetColText(m_c4Row_ThreadId, 2, m_strFmt);
        }

        SetStatSlot(m_cdstatsCur.m_c4UnknownMsgs, cdstatsTest.m_c4UnknownMsgs, m_c4Row_UnknownMsgs);
        SetStatSlot(m_cdstatsCur.m_c4UnknownWrites, cdstatsTest.m_c4UnknownWrites, m_c4Row_UnknownWrites);

        // Save the new stuff as the current stats
        m_cdstatsCur = cdstatsTest;
    }
}


tCIDLib::EExitCodes TStatsTab::ePollThread(TThread& thrThis, tCIDLib::TVoid* pData)
{
    // Let the calling thread go
    thrThis.Sync();

    tCQCKit::TCQCSrvProxy orbcDrv;
    while (kCIDLib::True)
    {
        // See if we've lost (or not gotten) connected
        tCIDLib::TBoolean bIsReady = orbcDrv.pobjData() && !orbcDrv->bCheckForLostConnection();

        // If not, try to reconnect
        if (!bIsReady)
        {
            try
            {
                // Drop any current reference
                orbcDrv.DropRef();

                orbcDrv = facCQCKit().orbcCQCSrvAdminProxy(m_strMoniker, 2000);
                bIsReady = kCIDLib::True;
            }

            catch(TError& errToCatch)
            {
                if (facCQCGKit().bLogWarnings())
                {
                    errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                    TModule::LogEventObj(errToCatch);
                }
            }
        }

        // If we ended up read, then let's do a poll
        if (bIsReady)
        {
            try
            {
                // Read into a temp object since we can't lock across this call
                TCQCDrvStats cdstatsTmp;
                orbcDrv->QueryDriverStats(m_strMoniker, cdstatsTmp);

                // It worked, so lock and copy our temp values to the new values
                TLocker lockrSync(&m_mtxSync);
                m_cdstatsNew = cdstatsTmp;
            }

            catch(TError& errToCatch)
            {
                if (facCQCGKit().bShouldLog(errToCatch))
                {
                    errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                    TModule::LogEventObj(errToCatch);
                }

                // If we lost connection, then drop our current reference
                if (orbcDrv->bCheckForLostConnection())
                    orbcDrv.DropRef();

                //
                //  Reset them all to invalid values to cause the GUI thread to show them all
                //  as unavailable.
                //
                TLocker lockrSync(&m_mtxSync);
                m_cdstatsNew.Reset();
            }
        }

        //
        //  Rest a while and check for a shutdown request at the same time. We slep longer
        //  if not the active tab.
        //
        if (!thrThis.bSleep(m_bIsActive ? 2000 : 5000))
            break;
    }

    return tCIDLib::EExitCodes::Normal;
}


tCIDLib::TVoid
TStatsTab::SetStatSlot(         tCIDLib::TCard4&    c4CurVal
                        , const tCIDLib::TCard4     c4NewVal
                        , const tCIDLib::TCard4     c4SlotInd)
{
    if (c4NewVal != c4CurVal)
    {
        c4CurVal = c4NewVal;
        if (c4NewVal == kCIDLib::c4MaxCard)
            m_strFmt = L"??";
        else
            m_strFmt.SetFormatted(c4CurVal);
        m_pwndList->SetColText(c4SlotInd, 2, m_strFmt);
    }
}




// ---------------------------------------------------------------------------
//  CLASS: TDrvInfoDlg
// PREFIX: dlgb
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TDrvInfoDlg: Constructors and Destructors
// ---------------------------------------------------------------------------
TDrvInfoDlg::TDrvInfoDlg(const TCQCSecToken& sectUser) :

    m_pwndClose(nullptr)
    , m_pwndTabs(nullptr)
    , m_sectUser(sectUser)
{
}

TDrvInfoDlg::~TDrvInfoDlg()
{
}


// ---------------------------------------------------------------------------
//  TDrvInfoDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TDrvInfoDlg::RunDlg(const TWindow& wndOwner, const TString& strMoniker)
{
    // Let's try to get the config info before we bother running the dialog
    try
    {
        TWndPtrJanitor janPtr(tCIDCtrls::ESysPtrs::Wait);
        tCQCKit::TInstSrvProxy orbcIS = facCQCKit().orbcInstSrvProxy();

        tCIDLib::TCard4 c4Ver = 0;
        TString strHost;
        orbcIS->bDrvCfgQueryConfig(c4Ver, strMoniker, strHost, m_cqcdcInfo, m_sectUser);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        TErrBox msgbErr(facCQCGKit().strMsg(kGKitErrs::errcDrvI_GetConfig));
        msgbErr.ShowIt(wndOwner);
        return;
    }

    // Looks like it's going to work, so
    m_strMoniker = strMoniker;
    c4RunDlg(wndOwner, facCQCGKit(), kCQCGKit::ridDlg_DrvInfo);
}

// ---------------------------------------------------------------------------
//  TDrvInfoDlg: Protected, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TDrvInfoDlg::bCreated()
{
    const tCIDLib::TBoolean bRet = TParent::bCreated();

    // Set our title bar text, putting the moniker in it
    strWndText(facCQCGKit().strMsg(kGKitMsgs::midDlg_DrvInfo_Title, m_strMoniker));

    //
    //  Get a typed pointer to our close button. The rest of these are done
    //  below because they are dependent on which flavor of data type we
    //  loaded up for.
    //
    CastChildWnd(*this, kCQCGKit::ridDlg_DrvInfo_Close, m_pwndClose);
    CastChildWnd(*this, kCQCGKit::ridDlg_DrvInfo_Tabs, m_pwndTabs);

    // Register handlers
    m_pwndClose->pnothRegisterHandler(this, &TDrvInfoDlg::eClickHandler);

    //
    //  Create the info tab and add it to the tabbed window, letting it activate and
    //  indicate it cannot be closed.
    //
    TCQCDrvInfoTab* pwndInfo = new TCQCDrvInfoTab
    (
        m_strMoniker, m_cqcdcInfo, kCIDLib::False
    );
    m_pwndTabs->c4CreateTab
    (
        pwndInfo
        , L"Driver Information"
        , kCIDLib::c4MaxCard
        , kCIDLib::True
        , tCIDCtrls::EWndStyles::VisTabChild
        , tCIDCtrls::EExWndStyles::None
    );


    // And the stats tab, non-activated so we don't start the polling unless used
    TStatsTab* pwndStats = new TStatsTab(m_strMoniker);
    m_pwndTabs->c4CreateTab
    (
        pwndStats
        , L"Driver Statistics"
        , kCIDLib::c4MaxCard
        , kCIDLib::False
        , tCIDCtrls::EWndStyles::VisTabChild
        , tCIDCtrls::EExWndStyles::None
    );

    return bRet;
}


// ---------------------------------------------------------------------------
//  TDrvInfoDlg: Private, non-virtual methods
// ---------------------------------------------------------------------------

tCIDCtrls::EEvResponses TDrvInfoDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCIDCtrls::widCancel)
        EndDlg(kCQCGKit::ridDlg_DrvInfo_Close);
    return tCIDCtrls::EEvResponses::Handled;
}

