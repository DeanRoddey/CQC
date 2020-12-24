//
// FILE NAME: MediaRepoMgr_MainFrame.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/01/2006
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
//  This file implements the main frame window.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "MediaRepoMgr.hpp"
#include    "MediaRepoMgr_EdMCatsDlg.hpp"
#include    "MediaRepoMgr_EdMetaDlg.hpp"
#include    "MediaRepoMgr_GetMetaDlg.hpp"
#include    "MediaRepoMgr_RipCDDlg.hpp"
#include    "MediaRepoMgr_ScanDrvsDlg.hpp"
#include    "MediaRepoMgr_SelMetaMatchDlg.hpp"
#include    "MediaRepoMgr_SelRepoDlg.hpp"
#include    "MediaRepoMgr_StatsDlg.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TMainFrameWnd,TFrameWnd)


// ---------------------------------------------------------------------------
//  Local types and constants
// ---------------------------------------------------------------------------
namespace MediaRepoMgr_MainFrame
{
    // -----------------------------------------------------------------------
    //  Some codes we post to ourself
    //
    //  i4ScanDrvs -    We post this to ourself when we get a media arrival
    //                  notification.
    // -----------------------------------------------------------------------
    const tCIDLib::TInt4    i4ScanDrvs  = 1001;
}


// ---------------------------------------------------------------------------
//  CLASS: TMainFrameWnd
// PREFIX: wnd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TMainFrameWnd: Constructors and destructor
// ---------------------------------------------------------------------------
TMainFrameWnd::TMainFrameWnd() :

    m_eMediaType(tCQCMedia::EMediaTypes::Count)
    , m_eMTFlags(tCQCMedia::EMTFlags::None)
    , m_eSortOrder(tCQCMedia::ESortOrders::Title)
    , m_menuSort()
    , m_menuType()
    , m_pwndArt(nullptr)
    , m_pwndArtist(nullptr)
    , m_pwndAspect(nullptr)
    , m_pwndCast(nullptr)
    , m_pwndClient(nullptr)
    , m_pwndColList(nullptr)
    , m_pwndDateAdded(nullptr)
    , m_pwndDescr(nullptr)
    , m_pwndDetType(nullptr)
    , m_pwndDuration(nullptr)
    , m_pwndItemList(nullptr)
    , m_pwndLabel(nullptr)
    , m_pwndLocInfo(nullptr)
    , m_pwndMyRating(nullptr)
    , m_pwndRating(nullptr)
    , m_pwndTitle(nullptr)
    , m_pwndTitleList(nullptr)
    , m_pwndYear(nullptr)
    , m_strTitle(facMediaRepoMgr.strMsg(kMRMgrMsgs::midMain_FrameTitle))
{
    // Generate a lease id to use in leasing the repository
    m_strLeaseId = TUniqueId::strMakeId();
}

TMainFrameWnd::~TMainFrameWnd()
{
}


// ---------------------------------------------------------------------------
//  TMainFrameWnd: Public, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid
TMainFrameWnd::AreaChanged( const   TArea&                  areaPrev
                            , const TArea&                  areaNew
                            , const tCIDCtrls::EPosStates   ePosState
                            , const tCIDLib::TBoolean       bOrgChanged
                            , const tCIDLib::TBoolean       bSizeChanged
                            , const tCIDLib::TBoolean       bStateChanged)
{
    TParent::AreaChanged(areaPrev, areaNew, ePosState, bOrgChanged, bSizeChanged, bStateChanged);

    if ((ePosState != tCIDCtrls::EPosStates::Minimized)
    &&  (m_pwndClient != nullptr)
    &&  bSizeChanged)
    {
        m_pwndClient->SetSize(areaClient().szArea(), kCIDLib::True);
    }
}


tCIDLib::TBoolean TMainFrameWnd::bCreate()
{
    //
    //  Get the initial repository to manage. If we successfully connect, it will do
    //  an initial lease. If they bail, we have to give up. If they do it, then the
    //  data will be stored away and ready for us to use. Tell it not to try to
    //  display the data, since we aren't created yet.
    //
    if (!bSelectRepo(TWindow::wndDesktop(), kCIDLib::False))
        return kCIDLib::False;

    TString strTitle(kMRMgrMsgs::midMain_FrameTitle, facMediaRepoMgr);
    strTitle.Append(L" [");
    strTitle.Append(m_strRepoMoniker);
    strTitle.Append(L"]");

    // Get the dialog description that we use to create our contents
    TDlgDesc dlgdChildren;
    facMediaRepoMgr.bCreateDlgDesc(kMediaRepoMgr::ridMain_Main, dlgdChildren);

    // Figure out the size required to fit that contents
    TArea areaInit;
    AreaForClient
    (
        dlgdChildren.areaPos()
        , areaInit
        , tCIDCtrls::EWndStyles::StdFrame
        , tCIDCtrls::EExWndStyles::None
        , kCIDLib::True
    );

    //
    //  Read in any previous size/pos info, or default it if not found, though we
    //  will override that default. Pass the calculated required size above as the
    //  minimum.
    //
    TCQCFrameState fstInit;
    facCQCGKit().bReadFrameState
    (
        kMediaRepoMgr::pszCfgKey_MainFrame, fstInit, areaInit.szArea()
    );

    //
    //  If there was no previously stored area, then we'll just use the natural
    //  size of the dialog, instead of letting the system select it. We'll still let
    //  it choose the position.
    //
    areaInit = fstInit.areaFrame();
    if (fstInit.areaFrame() == facCIDCtrls().areaDefWnd())
        areaInit.SetSize(dlgdChildren.areaPos().szArea());

    CreateFrame
    (
        nullptr
        , areaInit
        , strTitle
        , tCIDCtrls::EWndStyles::StdFrame
        , tCIDCtrls::EExWndStyles::None
        , tCIDCtrls::EFrameStyles::StdFrame
        , kCIDLib::False
    );
    return kCIDLib::True;
}


//
//  Allow some of our dialogs to force us to get repo settings if we have not already
//  gotten whatever they need.
//
tCIDLib::TBoolean TMainFrameWnd::bEditRepoSettings()
{
    TRepoSettingsDlg dlgSettings;
    if (dlgSettings.bRunDlg(*this, m_mrsCurrent))
    {
        StoreRepoSettings();
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


// Provide access to the repo settings we have
const TMediaRepoSettings& TMainFrameWnd::mrsCurrent() const
{
    return m_mrsCurrent;
}


const TString& TMainFrameWnd::strRepoMoniker() const
{
    return m_strRepoMoniker;
}


// ---------------------------------------------------------------------------
//  TMainFrameWnd: Protected, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TMainFrameWnd::bAllowShutdown()
{
    // Make sure that they really want to exit
    {
        TYesNoBox msgbAsk
        (
            facMediaRepoMgr.strMsg(kMRMgrMsgs::midMain_FrameTitle)
            , facMediaRepoMgr.strMsg(kMRMgrMsgs::midQ_Shutdown)
        );
        if (!msgbAsk.bShowIt(*this))
            return kCIDLib::False;
    }

    //
    //  If we have the lease on a repository, drop it, but make sure we give
    //  them a chance to save changes before we do.
    //
    if (!m_strRepoMoniker.bIsEmpty())
    {
        CheckSaveChanges();
        try
        {
            tMediaRepoMgr::TRepoMgrProxyPtr orbcSrv
            (
                facMediaRepoMgr.orbcMakeProxy(m_strRepoMoniker)
            );
            orbcSrv->DropLease(m_strLeaseId);
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);

            TExceptDlg dlgErr
            (
                *this
                , m_strTitle
                , facMediaRepoMgr.strMsg
                  (
                    kMRMgrMsgs::midStatus_LeaseDropFailed, m_strRepoMoniker
                  )
                , errToCatch
            );
        }
    }
    return kCIDLib::True;
}


tCIDLib::TBoolean TMainFrameWnd::bCreated()
{
    // Load up our menu bar
    SetMenuBar(facMediaRepoMgr, kMediaRepoMgr::ridMenu_Main);

    // And now call our parent class
    TParent::bCreated();

    // Get some sub-menus out that we deal with a lot and store them
    m_menuSort.Set(menuCur(),  kMediaRepoMgr::ridMenu_Tools_Sort);
    m_menuType.Set(menuCur(),  kMediaRepoMgr::ridMenu_MType);

    // Load our frame icon
    facMediaRepoMgr.SetFrameIcon(*this, L"RepoMgr");

    // Create a generic window and set it as our client window
    m_pwndClient = new TGenericWnd;
    m_pwndClient->CreateGenWnd
    (
        *this
        , areaClient()
        , tCIDCtrls::EWndStyles::ClippingVisChild
        , tCIDCtrls::EExWndStyles::ControlParent
        , kCIDCtrls::widFirstCtrl
    );

    //
    //  Tell it to populate itself from our main window dialog description, and to
    //  do an initial auto-fit to get the children fit to the size we are starting
    //  at.
    //
    TDlgDesc dlgdChildren;
    facMediaRepoMgr.bCreateDlgDesc(kMediaRepoMgr::ridMain_Main, dlgdChildren);
    tCIDCtrls::TWndId widInitFocus;
    m_pwndClient->PopulateFromDlg(dlgdChildren, widInitFocus, kCIDLib::True);


    // Set our dialog content size as the minimum
    SetMinMaxSize(dlgdChildren.areaPos().szArea(), TSize());

    // Get typed pointers to our child widgets
    CastChildWnd(*m_pwndClient, kMediaRepoMgr::ridMain_Artist, m_pwndArtist);
    CastChildWnd(*m_pwndClient, kMediaRepoMgr::ridMain_Art, m_pwndArt);
    CastChildWnd(*m_pwndClient, kMediaRepoMgr::ridMain_Aspect, m_pwndAspect);
    CastChildWnd(*m_pwndClient, kMediaRepoMgr::ridMain_Cast, m_pwndCast);
    CastChildWnd(*m_pwndClient, kMediaRepoMgr::ridMain_ColList, m_pwndColList);
    CastChildWnd(*m_pwndClient, kMediaRepoMgr::ridMain_DateAdded, m_pwndDateAdded);
    CastChildWnd(*m_pwndClient, kMediaRepoMgr::ridMain_Descr, m_pwndDescr);
    CastChildWnd(*m_pwndClient, kMediaRepoMgr::ridMain_DetType, m_pwndDetType);
    CastChildWnd(*m_pwndClient, kMediaRepoMgr::ridMain_Duration, m_pwndDuration);
    CastChildWnd(*m_pwndClient, kMediaRepoMgr::ridMain_ItemList, m_pwndItemList);
    CastChildWnd(*m_pwndClient, kMediaRepoMgr::ridMain_Label, m_pwndLabel);
    CastChildWnd(*m_pwndClient, kMediaRepoMgr::ridMain_LocInfo, m_pwndLocInfo);
    CastChildWnd(*m_pwndClient, kMediaRepoMgr::ridMain_MyRating, m_pwndMyRating);
    CastChildWnd(*m_pwndClient, kMediaRepoMgr::ridMain_Rating, m_pwndRating);
    CastChildWnd(*m_pwndClient, kMediaRepoMgr::ridMain_Title, m_pwndTitle);
    CastChildWnd(*m_pwndClient, kMediaRepoMgr::ridMain_TitleList, m_pwndTitleList);
    CastChildWnd(*m_pwndClient, kMediaRepoMgr::ridMain_Year, m_pwndYear);

    // Set an AR maintaining placement type on the art
    m_pwndArt->ePlacement(tCIDGraphDev::EPlacement::FitAR);

    // We have an initial media type, so get the menu in synce with that
    m_menuType.SetItemCheck
    (
        (m_eMediaType == tCQCMedia::EMediaTypes::Music)
            ? kMediaRepoMgr::ridMenu_MType_Music
            : kMediaRepoMgr::ridMenu_MType_Movies
        , kCIDLib::True
    );

    // And  do the same for the sort order
    menuCur().SetItemCheck(kMediaRepoMgr::ridMenu_Tools_Sort_Title, kCIDLib::True);

    // Load up the detail types combo
    m_pwndDetType->c4AddItem
    (
        facMediaRepoMgr.strMsg(kMRMgrMsgs::midMain_DetType_Collect)
    );
    m_pwndDetType->c4AddItem
    (
        facMediaRepoMgr.strMsg(kMRMgrMsgs::midMain_DetType_Item)
    );
    m_pwndDetType->c4AddItem
    (
        facMediaRepoMgr.strMsg(kMRMgrMsgs::midMain_DetType_Title)
    );

    // Select item details initially
    m_pwndDetType->SelectByIndex(0);

    // Register event handlres
    m_pwndColList->pnothRegisterHandler(this, &TMainFrameWnd::eLBHandler);
    m_pwndDetType->pnothRegisterHandler(this, &TMainFrameWnd::eLBHandler);
    m_pwndItemList->pnothRegisterHandler(this, &TMainFrameWnd::eLBHandler);
    m_pwndTitleList->pnothRegisterHandler(this, &TMainFrameWnd::eLBHandler);

    // Do an initial load of the titles, which will  fault in the other lists
    LoadTitles();

    // Activate ourself
    Activate();

    return kCIDLib::True;
}


// We post some messages to ourself for some things
tCIDLib::TVoid
TMainFrameWnd::CodeReceived(const tCIDLib::TInt4 i4Code, const tCIDLib::TCard4)
{
    if (i4Code == MediaRepoMgr_MainFrame::i4ScanDrvs)
    {
        //
        //  Check some things that would indicate that the user is doing
        //  things that we don't want to interrupt.
        //
        if (facCIDCtrls().bPopupOK()
        &&  !facCIDCtrls().bOwnsActivePopups(hwndThis(), kCIDLib::True))
        {
            //
            //  Use the scan drives dialog to get a drive to process. If they
            //  select a drive, then process it.
            //
            tCIDLib::TCard4 c4ItemCnt;
            TKrnlRemMedia::EMediaTypes eMType;
            TString strVol;
            TScanDrvsDlg dlgScan;
            if (dlgScan.bRunDlg(*this, strVol, eMType, c4ItemCnt))
                ProcessMedia(eMType, strVol, c4ItemCnt, kCIDLib::False);
        }
    }
}



tCIDLib::TVoid TMainFrameWnd::Destroyed()
{
    // Do a last save of any state changes
    facCQCGKit().StoreFrameState(*this, kMediaRepoMgr::pszCfgKey_MainFrame);

    return TParent::Destroyed();
}


tCIDLib::TVoid
TMainFrameWnd::MenuCommand( const   tCIDLib::TResId     ridItem
                            , const tCIDLib::TBoolean   bChecked
                            , const tCIDLib::TBoolean   bEnabled)
{
    if (ridItem == kMediaRepoMgr::ridMenu_Mng_EditCats)
    {
        // If the user commits changes, then the passed database will be updated
        TEdMCatsDlg dlgEdCats;
        dlgEdCats.RunDlg(*this, m_mdbEdit);
    }
     else if (ridItem == kMediaRepoMgr::ridMenu_Mng_DelTitle)
    {
        DeleteTitle();
    }
     else if ((ridItem == kMediaRepoMgr::ridMenu_Mng_AddColToPlayList)
          ||  (ridItem == kMediaRepoMgr::ridMenu_Mng_AddItemToPlayList))
    {
        AddToPlayList((ridItem == kMediaRepoMgr::ridMenu_Mng_AddColToPlayList));
    }
     else if (ridItem == kMediaRepoMgr::ridMenu_Mng_NewPlayList)
    {
        NewPlayList();
    }
     else if (ridItem == kMediaRepoMgr::ridMenu_Mng_AddManualCollect)
    {
        // Add a collection of the current type, manually
        ProcessMedia
        (
            (m_eMediaType == tCQCMedia::EMediaTypes::Music)
                ? TKrnlRemMedia::EMediaTypes::CDR
                : TKrnlRemMedia::EMediaTypes::DVDR
            , TString::strEmpty()
            , 0
            , kCIDLib::True
        );
    }
     else if ((ridItem == kMediaRepoMgr::ridMenu_MType_Movies)
          ||  (ridItem == kMediaRepoMgr::ridMenu_MType_Music))
    {
        // They want to select a new media type
        if (ridItem == kMediaRepoMgr::ridMenu_MType_Movies)
            m_eMediaType = tCQCMedia::EMediaTypes::Movie;
        else if (ridItem == kMediaRepoMgr::ridMenu_MType_Music)
            m_eMediaType = tCQCMedia::EMediaTypes::Music;

        // Check the new item
        m_menuType.SetItemCheck(ridItem, kCIDLib::True, kCIDLib::True);

        // And now reload the lists
        LoadTitles();
    }
     else if (ridItem == kMediaRepoMgr::ridMenu_Repo_Select)
    {
        // Just pass on to a private helper to do the work
        bSelectRepo(*this, kCIDLib::True);
    }
     else if (ridItem == kMediaRepoMgr::ridMenu_Repo_Stats)
    {
        TStatsDlg dlgStats;
        dlgStats.RunDlg(*this, m_mdbEdit, m_strRepoPath);
    }
     else if (ridItem == kMediaRepoMgr::ridMenu_Repo_Exit)
    {
        if (bAllowShutdown())
            facCIDCtrls().ExitLoop(tCIDLib::EExitCodes::Normal);
    }
     else if (ridItem == kMediaRepoMgr::ridMenu_Repo_Upload)
    {
        // Call a helper to do this
        SendChanges();
    }
     else if (ridItem == kMediaRepoMgr::ridMenu_Tools_ScanDrive)
    {
        //
        //  Use the scan drives dialog to get a drive to process. If they
        //  select a drive, then process it.
        //
        tCIDLib::TCard4 c4ItemCnt;
        TKrnlRemMedia::EMediaTypes eMType;
        TString strVol;
        TScanDrvsDlg dlgScan;
        if (dlgScan.bRunDlg(*this, strVol, eMType, c4ItemCnt))
            ProcessMedia(eMType, strVol, c4ItemCnt, kCIDLib::False);
    }
     else if (ridItem == kMediaRepoMgr::ridMenu_Tools_Settings)
    {
        TRepoSettingsDlg dlgSettings;
        if (dlgSettings.bRunDlg(*this, m_mrsCurrent))
            StoreRepoSettings();
    }
     else if (ridItem == kMediaRepoMgr::ridMenu_Tools_WriteSlotReport)
    {
        WriteSlotReport();
    }
     else if ((ridItem == kMediaRepoMgr::ridMenu_Tools_Sort_Artist)
          ||  (ridItem == kMediaRepoMgr::ridMenu_Tools_Sort_Title)
          ||  (ridItem == kMediaRepoMgr::ridMenu_Tools_Sort_Year))
    {
        // They want to select a new sort order
        if (ridItem == kMediaRepoMgr::ridMenu_Tools_Sort_Artist)
            m_eSortOrder = tCQCMedia::ESortOrders::Artist;
        else if (ridItem == kMediaRepoMgr::ridMenu_Tools_Sort_Title)
            m_eSortOrder = tCQCMedia::ESortOrders::Title;
        else if (ridItem == kMediaRepoMgr::ridMenu_Tools_Sort_Year)
            m_eSortOrder = tCQCMedia::ESortOrders::Year;

        // Check the new item
        m_menuSort.SetItemCheck(ridItem, kCIDLib::True, kCIDLib::True);

        // Just reload the title list
        LoadTitles();
    }
     else if (ridItem == kMediaRepoMgr::ridMenu_Tools_UpdateExtLocs)
    {
        //
        //  Get the find and replace with values from the user. We use the generic
        //  replace dialog. We just pass in empty recall lists since this isn't
        //  one that's going to be used often, so no use keeping recall lists.
        //
        //  We don't allow any of the optional bits.
        //
        tCIDLib::TBoolean   bFullMatchOnly;
        tCIDLib::TBoolean   bSelectionOnly;
        tCIDLib::TStrList   colFindRecall;
        tCIDLib::TStrList   colRepRecall;
        TString             strFind;
        TString             strRepWith;
        const tCIDLib::TBoolean bRes = facCIDWUtils().bGetReplaceOpts
        (
            *this
            , strFind
            , strRepWith
            , colFindRecall
            , colRepRecall
            , bFullMatchOnly
            , bSelectionOnly
            , tCIDWUtils::EFindFlags::None
        );

        // Call a helper to do this work if they committed
        if (bRes)
            ChangeExtMediaPaths(strFind, strRepWith);
     }
}



//
//  We get this call when a removable media driver has media inserted or
//  removed. If inserted, then we check the configured drive and find out
//  what's in it. If it's a DVD or CD, then we pop up a dialog asking if
//  they'd like to process it.
//
tCIDLib::TVoid TMainFrameWnd::MediaChange(const tCIDCtrls::EMediaChTypes eType)
{
    //
    //  If we aren't the active window (a dialog is up), then we ignore
    //  this. Else, we watch for an arrival.
    //
    if (!bIsActive() || (eType != tCIDCtrls::EMediaChTypes::Arrived))
        return;

    // Post ourself a msg to handle this
    SendCode(MediaRepoMgr_MainFrame::i4ScanDrvs, 0);
}


// ---------------------------------------------------------------------------
//  TMainFrameWnd: Private, non-virtual methods
//
//  Others are in the 2 file to keep this file from getting too big
// ---------------------------------------------------------------------------

//
//  If the currently selected item or collection is of music type, and is in
//  our repository (i.e. file based item location type), then we present the
//  user with a list of playlists to add it to. We can add a whole collection
//  or a single item.
//
tCIDLib::TVoid TMainFrameWnd::AddToPlayList(const tCIDLib::TBoolean bCollection)
{
    // If we aren't in music mode right now, then this is meaningless
    if (m_eMediaType != tCQCMedia::EMediaTypes::Music)
        return;

    try
    {
        //
        //  Get pointers to the current title set, collection, and item. If
        //  any are not available, we do nothing.
        //
        const TMediaTitleSet* pmtsCur = pmtsCurrent();
        const TMediaCollect* pmcolCur = pmcolCurrent();
        const TMediaItem* pmitemCur = pmitemCurrent();
        if (!pmtsCur || !pmcolCur || !pmitemCur)
            return;

        // If the current collection is itself a playlist, then do nothing
        if (pmcolCur->bIsPlayList())
            return;

        //
        //  If this is not a item based file location type, then we cannot
        //  use it.
        //
        if (pmcolCur->eLocType() != tCQCMedia::ELocTypes::FileItem)
            return;

        // We can potentially add it, so get a list of defined playlists
        TVector<TString>    colNames;
        tCQCMedia::TIdList  fcolIds;
        const tCIDLib::TCard4 c4Count = m_mdbEdit.c4QueryPlayLists(colNames, fcolIds);

        // If we didn't find any, tell them so and give up
        if (!c4Count)
        {
            TOkBox msgbNoPL
            (
                m_strTitle, facMediaRepoMgr.strMsg(kMRMgrMsgs::midStatus_NoPlayLists)
            );
            msgbNoPL.ShowIt(*this);
            return;
        }

        // We can use the generic list selection dialog for this
        tCIDLib::TBoolean   bRes = kCIDLib::False;
        TString             strSelected;
        {
            bRes = facCIDWUtils().bListSelect
            (
                *this
                , facMediaRepoMgr.strMsg(kMRMgrMsgs::midSelPL_Title)
                , colNames
                , strSelected
            );
        }

        // If nothing selected, then we don't do anything
        if (!bRes)
            return;

        // Find this one in the list
        tCIDLib::TCard4 c4SelInd = 0;
        while (c4SelInd < c4Count)
        {
            if (colNames[c4SelInd] == strSelected)
                break;
            c4SelInd++;
        }
        CIDAssert(c4SelInd < c4Count, L"Didn't find the selected value in the list");

        //
        //  They selected one, so let's update the target playlist. We will either
        //  be adding a single item or all of the items in a collection. Look up
        //  the collection for the target playlist first.
        //
        if (bCollection)
        {
            m_mdbEdit.bAddColToPL
            (
                fcolIds[c4SelInd], m_eMediaType, pmcolCur->c2Id()
            );
        }
         else
        {
            // Add the single currently selected item
            m_mdbEdit.bAddItemToPL
            (
                fcolIds[c4SelInd], m_eMediaType, pmitemCur->c2Id()
            );
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        TExceptDlg dlgErr
        (
            *this
            , m_strTitle
            , facMediaRepoMgr.strMsg(kMRMgrMsgs::midStatus_Exception)
            , errToCatch
        );
    }
}


//
//  Called when we start up and when the user asks to select a new repo. We get an
//  owner window passed, and don't assume us, because on startup we haven't been
//  created yet. The bReload flag allows the caller to keep us from trying to
//  load content before we've been created. In that case, this info will be used
//  later upon creation of the window.
//
tCIDLib::TBoolean
TMainFrameWnd::bSelectRepo( const   TWindow&            wndOwner
                            , const tCIDLib::TBoolean   bReload)
{
    tCQCMedia::EMediaTypes  eInitType;
    TMediaDB                mdbNew;
    TString                 strNewRepo;

    // If any changes, give them a chance to save them first
    CheckSaveChanges();

    //
    //  Pass in the current repo, if any, so he can drop it. On output, if a new
    //  selection was made, we get the new one back.
    //
    strNewRepo = m_strRepoMoniker;
    TMediaRepoSelRepoDlg dlgSelRepo;
    const tCIDLib::TBoolean bRes = dlgSelRepo.bRunDlg
    (
        wndOwner
        , m_strLeaseId
        , eInitType
        , mdbNew
        , strNewRepo
        , m_strRepoPath
        , m_strDBSerialNum
    );
    if (!bRes)
        return kCIDLib::False;

    //
    //  Looks ok. Let's download the settings for this repository from the master
    //  service. We want to do this before we commit.
    //
    TMediaRepoSettings mrsNew;
    if (!bLoadRepoSettings(strNewRepo, mrsNew))
        return kCIDLib::False;

    // OK, looks good, so store the info
    m_eMediaType = eInitType;
    m_mdbEdit = mdbNew;
    m_mrsCurrent = mrsNew;
    m_strRepoMoniker = strNewRepo;

    // If asked to reload, then do that
    if (bReload)
    {
        //
        //  Now we need to reload. Note that the selection dialog above dropped
        //  any previous repository lease for us and leased the new one.
        //
        LoadTitles();

        // Update the media type menu to reflect the new active media type
        tCIDLib::TCard4 c4CmdId = 0;
        if (m_eMediaType == tCQCMedia::EMediaTypes::Movie)
            c4CmdId = kMediaRepoMgr::ridMenu_MType_Movies;
        else if (m_eMediaType == tCQCMedia::EMediaTypes::Music)
            c4CmdId = kMediaRepoMgr::ridMenu_MType_Music;
        if (c4CmdId)
            m_menuType.SetItemCheck(c4CmdId, kCIDLib::True);

        // Update the title bar
        TString strTitle(kMRMgrMsgs::midMain_FrameTitle, facMediaRepoMgr);
        strTitle.Append(L" [");
        strTitle.Append(m_strRepoMoniker);
        strTitle.Append(L"]");
        strWndText(strTitle);
    }
    return kCIDLib::True;
}


//
//  This will look through the title set list box and see if one has the indicated title
//  set. The basic list box doesn't have it's own id to mapping scheme, so we do it our
//  self. The title set id is set as the per-row data.
//
//  Returns max card if not found.
//
tCIDLib::TCard4 TMainFrameWnd::c4FindTitleIndexById(const tCIDLib::TCard2 c2TitleId) const
{
    const tCIDLib::TCard4 c4Count = m_pwndTitleList->c4ItemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (m_pwndTitleList->c4IndexToId(c4Index) == c2TitleId)
            return c4Index;
    }
    return kCIDLib::c4MaxCard;
}



//
//  This is called before selecting a new repo or exiting, to see if there are any
//  changes and giving the user to save them.
//
tCIDLib::TVoid TMainFrameWnd::CheckSaveChanges()
{
    //
    //  If the current serial number is empty, we've not selected a repo yet, so
    //  nothing to do.
    //
    if (m_strDBSerialNum.bIsEmpty())
        return;

    TString strTestSerNum;
    {
        tCIDLib::TCard4 c4DBSize;
        TChunkedBinOutStream strmTar(32 * (1024 * 1024));
        strmTar << m_mdbEdit << kCIDLib::FlushIt;
        c4DBSize = strmTar.c4CurSize();

        // Hash this data to create our database serial number
        TChunkedBinInStream strmSrc(strmTar);
        facCQCMedia().CreatePersistentId(strmSrc, c4DBSize, strTestSerNum);
    }

    // If the serial number is different, then ask if we should change them
    if (strTestSerNum != m_strDBSerialNum)
    {
        TYesNoBox msgbSave
        (
            facMediaRepoMgr.strMsg(kMRMgrMsgs::midMain_FrameTitle)
            , facMediaRepoMgr.strMsg(kMRMgrMsgs::midQ_SaveChanges)
        );

        if (msgbSave.bShowIt(*this))
            SendChanges();
    }
}


//
//  Loads the indicated art id and sets it on our static image widget. We have
//  some special concerns here. We cache image data in the database until the user
//  commits changes. So we first check for the cached data. If none, we try to
//  download it.
//
tCIDLib::TVoid
TMainFrameWnd::LoadArt( const   tCQCMedia::EMediaTypes  eMType
                        , const tCIDLib::TCard2         c2Id)
{
    // If it's zero, there isn't any so just clear the image widget
    if (!c2Id)
    {
        m_pwndArt->Reset();
        return;
    }

    try
    {
        // Look up the image
        const TMediaImg* pmimgLoad = m_mdbEdit.pmimgById(eMType, c2Id, kCIDLib::False);
        if (!pmimgLoad)
        {
            // Not much we can do
            m_pwndArt->Reset();
            return;
        }

        tCIDLib::TCard4 c4ArtSz = 0;
        THeapBuf        mbufArt;

        // See if there is cached art not yet uploaded
        c4ArtSz = pmimgLoad->c4QueryArt(mbufArt, tCQCMedia::ERArtTypes::LrgCover, 0);

        if (!c4ArtSz)
        {
            TWndPtrJanitor janPtr(tCIDCtrls::ESysPtrs::Wait);
            tMediaRepoMgr::TRepoMgrProxyPtr orbcSrv
            (
                facMediaRepoMgr.orbcMakeProxy(m_strRepoMoniker)
            );

            //
            //  Note that large art is what is cached for later upload. But here we
            //  just get the thumbnail since we just need it for the small display.
            //
            const tCIDLib::TBoolean bRes = orbcSrv->bQueryArtById
            (
                eMType
                , tCQCMedia::ERArtTypes::SmlCover
                , c2Id
                , c4ArtSz
                , mbufArt
                , kCIDLib::False
            );

            if (!bRes)
                c4ArtSz = 0;
        }

        //
        //  If we got something, try to display it. Else just clear the image
        //  display.
        //
        if (c4ArtSz)
        {
            TJPEGImage imgLoad;
            TBinMBufInStream strmSrc(&mbufArt, c4ArtSz);
            strmSrc >> imgLoad;

            TGraphWndDev gdevCompat(*this);
            m_pwndArt->SetImage(gdevCompat, imgLoad);
        }
         else
        {
            m_pwndArt->Reset();
        }
    }

    catch(TError& errToCatch)
    {
        m_pwndArt->Reset();

        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        TExceptDlg dlgErr
        (
            *this
            , m_strTitle
            , facMediaRepoMgr.strMsg(kMRMgrMsgs::midStatus_Exception)
            , errToCatch
        );
    }
}


//
//  Creates a new play list. We just set up the basic info in a collection
//  object, mark it as a play list, and then from there forward we just
//  treat it like any other collection and invoke the collection metadata
//  editing dialog, and then upload the edited data if they commit.
//
tCIDLib::TVoid TMainFrameWnd::NewPlayList()
{
    try
    {
        //
        //  This version of bRunDlg() is for adding a new playlist. In this mode
        //  it doesn't need to know anything but the media type and that is is a
        //  playlist. It'll give back the id of the new collection, which is all
        //  we need to look up the info and update ourself appropriately.
        //
        tCIDLib::TCard2 c2ColId;
        tCIDLib::TCard2 c2SetId;

        TEdMetaDataDlg dlgEdit;
        const tCIDLib::TBoolean bRes = dlgEdit.bRunDlg
        (
            *this
            , tCQCMedia::EMediaTypes::Music
            , kCIDLib::True
            , m_mdbEdit
            , c2SetId
            , c2ColId
        );

        // If we created one, add this new guy to our list and select it
        if (bRes)
            ShowNewListTitle(tCQCMedia::EMediaTypes::Music, c2SetId);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        TExceptDlg dlgErr
        (
            *this
            , m_strTitle
            , facMediaRepoMgr.strMsg(kMRMgrMsgs::midStatus_Exception)
            , errToCatch
        );
    }
}


//
//  Helpers to get the current list window item from the three lists that display the
//  hiearchical data, in a typed way that's immediately useful to the caller. They
//  will of course return null if the list is empty. The list windows store the ids
//  of the things so we just get the id out and look up the thing.
//
const TMediaCollect* TMainFrameWnd::pmcolCurrent()
{
    const tCIDLib::TCard4 c4Index = m_pwndColList->c4CurItem();
    if (c4Index == kCIDLib::c4MaxCard)
        return nullptr;

    return m_mdbEdit.pmcolById
    (
        m_eMediaType
        , tCIDLib::TCard2(m_pwndColList->c4IndexToId(c4Index))
        , kCIDLib::True
    );
}

const TMediaItem* TMainFrameWnd::pmitemCurrent()
{
    const tCIDLib::TCard4 c4Index = m_pwndItemList->c4CurItem();
    if (c4Index == kCIDLib::c4MaxCard)
        return nullptr;

    return m_mdbEdit.pmitemById
    (
        m_eMediaType
        , tCIDLib::TCard2(m_pwndItemList->c4IndexToId(c4Index))
        , kCIDLib::True
    );
}

const TMediaTitleSet* TMainFrameWnd::pmtsCurrent()
{
    const tCIDLib::TCard4 c4Index = m_pwndTitleList->c4CurItem();
    if (c4Index == kCIDLib::c4MaxCard)
        return nullptr;

    return m_mdbEdit.pmtsById
    (
        m_eMediaType
        , tCIDLib::TCard2(m_pwndTitleList->c4IndexToId(c4Index))
        , kCIDLib::True
    );
}


//
//  This is called from a couple of places where the user decides to process
//  a CD, DVD, or BD in a drive. We go through the steps of:
//
//  1. Downloading the metadata
//  2. Checking if this colleciton is already in the database
//  3. Letting the user edit the metadata
//  4. Scaling down the image we got during metadta retrieval/editing, if neded
//  5. Ripping of the CD if it's a CD and they choose to rip and upload
//  6. Creating any new categories that were in the metadata and kept by the
//     user during editing.
//  7. If we ripped a CD, then we have to upload the data, and commit any metadata
//     at this point as well, since the new files we are uploading will otherwise
//     not be referenced.
//
//  NOTE:   BE CAREFUL about calling any helper methods here. We are not
//          necessarily adding a new collection of the same type as what
//          we are currently displaying. Most of the helpers deal with the
//          the currently displayed media type.
//
//          We check at the end to see if the new one is of the same type
//          as displayed and do any required visual updates there.
//
tCIDLib::TVoid
TMainFrameWnd::ProcessMedia(const   TKrnlRemMedia::EMediaTypes  eMType
                            , const TString&                    strPath
                            , const tCIDLib::TCard4             c4DiscItemCnt
                            , const tCIDLib::TBoolean           bDoManual)
{
    try
    {
        TString strErrText;

        // Figure out the media type and create the new title set
        const tCQCMedia::EMediaTypes eType
        (
            TKrnlRemMedia::bIsCDType(eMType)
            ? tCQCMedia::EMediaTypes::Music
            : tCQCMedia::EMediaTypes::Movie
        );

        //
        //  If not a manual entry, then get the unique id for this collection,
        //  and see if it is already in the repository. Else, generate
        //  a UID.
        //
        tCIDLib::TBoolean bManualEntry(bDoManual);
        TString strUID;
        TKrnlRemMedia::TCDTOCInfo TOCData;

        //
        //  If not manual mode, try to make a unique id based on the content in
        //  the volume.
        //
        if (!bManualEntry)
        {
            // If we can't get one, then tell the user and switch to manual mode
            if (!facMediaRepoMgr.bMakeUniqueId(strPath, eMType, TOCData, strUID))
            {
                TErrBox msgbOver
                (
                    m_strTitle
                    , facMediaRepoMgr.strMsg(kMRMgrMsgs::midStatus_ColIdNotGenerated, strPath)
                );
                msgbOver.ShowIt(*this);
                bManualEntry = kCIDLib::True;
            }
        }

        //
        //  If in manual mode, create a manual mode type of id. We prefix them with
        //  ME_ so that they won't clash with any real ids.
        //
        if (bManualEntry)
        {
            strUID = L"ME_";
            strUID.Append(TUniqueId::strMakeId());
        }

        //
        //  See if it is in the driver's database. If so, then tell them
        //  that it already exists. It shouldn't even happen on a manual
        //  entry, but do the check anyway. Doesn't hurt and will catch
        //  the infinitesimal possibility of a dup unique id being generated.
        //
        tCIDLib::TBoolean bDupCol;
        TString strName;
        {
            tCIDLib::TCard2 c2Id;
            bDupCol = m_mdbEdit.bCheckUIDExists
            (
                eType, tCQCMedia::EDataTypes::Collect, strUID, c2Id, strName
            );
        }

        if (bDupCol)
        {
            TOkBox msgbOver
            (
                m_strTitle
                , facMediaRepoMgr.strMsg(kMRMgrMsgs::midStatus_ColExists, strName)
            );
            msgbOver.ShowIt(*this);
            return;
        }

        //
        //  OK, now we need to try to get meta data one way or another.
        //
        tCIDLib::TBoolean       bStatus = kCIDLib::False;
        tCQCMedia::TItemList    colItems;
        TMediaCollect           mcolNew(eType);
        TJPEGImage              imgNew;
        tCIDLib::TStrList       colCatNames;

        // Set the generated id on the new collection
        mcolNew.strUniqueId(strUID);

        //
        //  First thing we want to try is to see if we have My Movies support
        //  and if this is a movie we are doing. If so, we can try to look up
        //  the info there. If we have a disk, it'll load any matches from
        //  that first, but they can still do a manual search.
        //
        if (m_mrsCurrent.bUseMyMovies() && (eType == tCQCMedia::EMediaTypes::Movie))
        {
            bStatus = bFindDiscMetaData
            (
                strPath, eMType, mcolNew, colItems, colCatNames, imgNew
            );
        }

        //
        //  OK. Now either we got the data automatically above, or we didn't or
        //  didn't even try to. So let's try other options.
        //
        if (!bStatus)
        {
            if (bManualEntry)
            {
                //
                //  It's going to be a fully manual entry, so set the little bit
                //  of stuff we can set.
                //
                mcolNew.strArtist(L"Various Artists");
                mcolNew.strName(L"Unknown");
            }
             else
            {
                //
                //  If we get here, then it's either music or they don't have
                //  My Movies support, so we can try to do the WMP or scraping
                //  methods to get the data.
                //
                TGetMetaDataDlg dlgMetaWait;
                bStatus = dlgMetaWait.bRun
                (
                    *this
                    , strPath
                    , mcolNew
                    , colItems
                    , colCatNames
                    , imgNew
                    , TOCData
                );
            }
        }

        //
        //  Look up any category names we got and update the collection to ref
        //  those.
        //
        const tCIDLib::TCard4 c4CatNames = colCatNames.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4CatNames; c4Index++)
        {
            const TMediaCat* pmcatCur = m_mdbEdit.pmcatByName
            (
                eType, colCatNames[c4Index], kCIDLib::False
            );
            if (pmcatCur)
                mcolNew.bAddCategory(pmcatCur->c2Id());
        }

        //
        //  It's possible one of the metadata operations above cleared the unique
        //  id we set, since it may have been preparing to set one and failed. So
        //  if the id is empty, put it back.
        //
        if (mcolNew.strUniqueId().bIsEmpty())
            mcolNew.strUniqueId(strUID);

        //
        //  At this point we have some atomicity issues. We have to edit the metadata
        //  and get the data into the DB so that ids are set, so that we can use those
        //  ids to create the upload files. But, if the user cancels later during the
        //  rip, we don't want to save the data.
        //
        //  So at this point we save the database, so that we can restore it later
        //  if they cancel.
        //
        TMediaDB mdbSave(m_mdbEdit);

        //
        //  Ok, so let them edit the metadata. A lot goes on here. We pass in:
        //
        //  1. The collection info we got
        //  2. The image we got
        //  3. The items we got
        //
        //  The dialog does the rest of the work. It will copy any changes to our
        //  local database if the user commits.
        //
        //  We'll get back the id of the parent title set selected or created, and of
        //  the newly added collection. Any added items will be gettable through the
        //  collection if needed.
        //
        tCIDLib::TCard2 c2ArtId;
        tCIDLib::TCard2 c2ColId;
        tCIDLib::TCard2 c2ParId;
        {
            TEdMetaDataDlg dlgEdit;
            bStatus = dlgEdit.bRunDlg
            (
                *this
                , mcolNew
                , colItems
                , imgNew
                , c4DiscItemCnt
                , bManualEntry
                , m_mdbEdit
                , c2ParId
                , c2ColId
                , c2ArtId
            );
        }

        //
        //  If they cancelled here. then nothing has changed and we can just return
        //  since they didn't commit above.
        //
        if (!bStatus)
            return;

        //
        //  If the location type indicates track based files and it's music, then
        //  in our current scheme of things, that implies we are going to rip and
        //  upload.
        //
        //  It gives us back the list of files that it wrote out locally, but we
        //  don't really need them. They are there in the upload directory awaiting
        //  an upload of changes by the user. However, if they cancel, we do delete
        //  any files that were ripped, so we use it for that.
        //
        //  We do this first before we edit the metadata, so that we can be sure this
        //  is going to work before we let the user commit the changes.
        //
        tCIDLib::TStrList colFiles;
        if ((eType == tCQCMedia::EMediaTypes::Music)
        &&  (mcolNew.eLocType() == tCQCMedia::ELocTypes::FileItem))
        {
            strErrText.Clear();
            if (eType == tCQCMedia::EMediaTypes::Music)
            {
                TRipCDDlg dlgRip;
                bStatus = dlgRip.bRun
                (
                    *this
                    , strPath
                    , strErrText
                    , m_mdbEdit
                    , eType
                    , c2ColId
                    , colCatNames.bIsEmpty() ? TString::strEmpty() : colCatNames[0]
                    , colFiles
                );
            }

            // If the dialog was cancelled or it failed, then we are done
            if (!bStatus)
            {
                // Delete any files that did get ripped
                CleanupFiles(colFiles);

                // Put back the original database to undo the metadata changes
                m_mdbEdit = mdbSave;

                // If an error, show the user the error
                if (!strErrText.bIsEmpty())
                {
                    TErrBox msgbFailed(m_strTitle, strErrText);
                    msgbFailed.ShowIt(*this);
                }
                return;
            }
        }

        // If the eject after rip flag is set, then eject
        if (m_mrsCurrent.bRipEject())
        {
            TKrnlRemMediaDrv rmmdDrv(strPath.pszBuffer());
            if (rmmdDrv.bOpen())
            {
                TWndPtrJanitor janPtr(tCIDCtrls::ESysPtrs::Wait);
                rmmdDrv.bOpenDoor(kCIDLib::True);
            }
        }

        //
        //  If the changes were committed, and the new stuff is of the same media
        //  type as what we are displaying, then we need to insert the new title and
        //  select it.
        //
        if (bStatus)
            ShowNewListTitle(eType, c2ParId);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        TExceptDlg dlgErr
        (
            *this
            , m_strTitle
            , facMediaRepoMgr.strMsg(kMRMgrMsgs::midStatus_Exception)
            , errToCatch
        );
    }
}


// Removes the passed title from our database and list window
tCIDLib::TVoid
TMainFrameWnd::RemoveTitle( const   tCQCMedia::EMediaTypes  eType
                            , const tCIDLib::TCard2         c2TitleId)
{

    // Remove from the database
    m_mdbEdit.PruneHierarchy(eType, kCIDLib::True, c2TitleId);

    // Find it in our list window and remove it
    tCIDLib::TCard4 c4Count = m_pwndTitleList->c4ItemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (tCIDLib::TCard2(m_pwndTitleList->c4IndexToId(c4Index)) == c2TitleId)
        {
            m_pwndTitleList->RemoveAt(c4Index);
            break;
        }
    }
}


//
//  When a new title set is created, we need to get it into the list in the correct
//  sorted order, and select it.
//
//  The only way we can figure out where it goes is to query the sorted list of titles.
//  That's a bit piggy, but there's no more efficient way to do it. Our list can be
//  sorted by multiple criteria, and we don't want to dup the sorting code here.
//
//  Note, we only need to do anything if the new title is of the type we are currently
//  displaying, which isn't always the case.
//
tCIDLib::TVoid
TMainFrameWnd::ShowNewListTitle(const   tCQCMedia::EMediaTypes  eType
                                , const tCIDLib::TCard2         c2TitleId)
{
    if (eType != m_eMediaType)
        return;

    // Get the sorted list of titles
    tCQCMedia::TSetIdList colSets(tCIDLib::EAdoptOpts::NoAdopt);
    const tCIDLib::TCard4 c4TitleCnt = c4GetSortedTitleList(colSets, eType);

    //
    //  Find our guy in the list. We can't do a binary search unless we
    //  search on the current sort criteria, which is never on id. So we
    //  have to just do a bulk search. It's not like adding a new title
    //  happens often.
    //
    tCIDLib::TCard4 c4At = 0;
    while (c4At < c4TitleCnt)
    {
        if (colSets[c4At]->c2Id() == c2TitleId)
            break;
        c4At++;
    }

    //
    //  See if this guy is already at that location. If so, not a new one, but we
    //  still want to reselect it to make sure any newly added collections will be
    //  shown. If not, insert it at the correct place and select it.
    //
    if ((c4At < m_pwndTitleList->c4ItemCount())
    &&  (tCIDLib::TCard2(m_pwndTitleList->c4IndexToId(c4At)) == c2TitleId))
    {
        m_pwndTitleList->SelectByIndex(c4At, kCIDLib::True);
    }
     else
    {
        // Look up the set and insert it at the found index, selecting it
        const TMediaTitleSet* pmtsNew = m_mdbEdit.pmtsById
        (
            eType, c2TitleId, kCIDLib::True
        );

        m_pwndTitleList->InsertItem(pmtsNew->strName(), c4At, c2TitleId);

        // Select it and force an event so the data updates
        m_pwndTitleList->SelectByIndex(c4At, kCIDLib::True);
    }
}

