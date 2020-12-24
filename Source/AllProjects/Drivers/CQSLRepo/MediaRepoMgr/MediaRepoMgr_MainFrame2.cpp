//
// FILE NAME: MediaRepoMgr_MainFrame2.cpp
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
//  This file handles some overflow from the main .cpp file for the main
//  frame window, to keep it from getting too large.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "MediaRepoMgr.hpp"
#include    "MediaRepoMgr_EdMetaDlg.hpp"
#include    "MediaRepoMgr_EdTitleSetDlg.hpp"
#include    "MediaRepoMgr_EdMItemDlg.hpp"
#include    "MediaRepoMgr_SelMetaMatchDlg.hpp"
#include    "MediaRepoMgr_UploadDlg.hpp"



// ---------------------------------------------------------------------------
//  TMainFrameWnd: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  This is called if we have MM support and there's a readable disc in a drive
//  for us to attempt an automatic lookup of. So we'll generate the id and do
//  a lookup and return whether we did or not. If we did, then the collection,
//  list of category names, and the cover art image are filled in.
//
tCIDLib::TBoolean
TMainFrameWnd::bFindDiscMetaData(const  TString&                    strVolume
                                , const TKrnlRemMedia::EMediaTypes  eMType
                                ,       TMediaCollect&              mcolToFill
                                ,       tCQCMedia::TItemList&       colItems
                                ,       tCIDLib::TStrList&          colCatNames
                                ,       TJPEGImage&                 imgToFill)
{
    tCIDLib::TBoolean bRes = kCIDLib::False;
    try
    {
        TMyMoviesMetaSrc rmsQ(m_mrsCurrent);

        //
        //  Invoke the selection dialog with the metadata source and the initial
        //  disc id based search to do. If they bail out with no selection, we are
        //  done. Else we try to load their selected match.
        //
        TRepoMetaMatch   rmmSel;
        TSelMetaMatchDlg dlgSel;
        if (!dlgSel.bRunDlg(*this, rmsQ, strVolume, eMType, rmmSel))
            return kCIDLib::False;

        // OK, they selected one, so let's try to load it
        TWndPtrJanitor janPtr(tCIDCtrls::ESysPtrs::Wait);
        TString strFailReason;
        bRes = rmsQ.bLoadTitleById
        (
            rmmSel.strLoadID(), mcolToFill, imgToFill, colCatNames, strFailReason
        );

        if (!bRes)
        {
            TErrBox msgbFailed(L"Query Failed", strFailReason);
            msgbFailed.ShowIt(*this);
            return bRes;
        }

        // For a faux item into the item list, to represent the movie
        colItems.objAdd
        (
            TMediaItem
            (
                L"Movie"
                , TUniqueId::strMakeId()
                , TString::strEmpty()
                , tCQCMedia::EMediaTypes::Movie
            )
        );
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        TExceptDlg dlgErr
        (
            *this
            , facMediaRepoMgr.strMsg(kMRMgrMsgs::midMain_FrameTitle)
            , L"An error occurred while retreiving the metadata"
            , errToCatch
        );
        bRes = kCIDLib::False;
    }
    return bRes;
}


//
//  Tries to download the cover art for the indicated id and the current media type.
//  There might be none, in which case it returns zero and a default empty image.
//
//  Note that here we get the thumbnail since we just need it for the small display.
//
tCIDLib::TBoolean
TMainFrameWnd::bLoadArt(const tCIDLib::TCard2 c2Id, TJPEGImage& imgArt)
{
    tCIDLib::TBoolean bRet = kCIDLib::True;
    try
    {
        tMediaRepoMgr::TRepoMgrProxyPtr orbcSrv
        (
            facMediaRepoMgr.orbcMakeProxy(m_strRepoMoniker)
        );

        tCIDLib::TCard4 c4ArtSz;
        THeapBuf        mbufArt;

        bRet = orbcSrv->bQueryArtById
        (
            m_eMediaType
            , tCQCMedia::ERArtTypes::SmlCover
            , c2Id
            , c4ArtSz
            , mbufArt
            , kCIDLib::False
        );
        if (bRet)
        {
            TBinMBufInStream strmSrc(&mbufArt, c4ArtSz);
            strmSrc >> imgArt;
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
        bRet = kCIDLib::False;
    }

    if (!bRet)
        imgArt.szResolution(TSize(1, 1));
    return bRet;
}


//
//  This is called after a new repo selection. We query the repo settings for
//  the new repository and store them away. Note that we take repo moniker, we don't
//  use the currently set one, because we want to make sure this will work before
//  commit to a newly selected repo.
//
tCIDLib::TBoolean
TMainFrameWnd::bLoadRepoSettings(const  TString&            strRepoMoniker
                                ,       TMediaRepoSettings& mrsToFill)
{
    try
    {
        // Build up the binding name
        TString strBinding(kMediaRepoMgr::pszOSKey_Settings);
        strBinding.eReplaceToken(strRepoMoniker, L'1');

        //
        //  Try to read in the value. If it's not found, then it's
        //  not been set so we keep the defaults from the def ctor
        //  above.
        //
        tCIDLib::TCard4 c4SerialNum = 0;
        const tCIDLib::ELoadRes eRes = facCQCGKit().eReadStoreObj
        (
            strBinding, c4SerialNum, mrsToFill, kCIDLib::False
        );
    }

    catch(TError& errToCatch)
    {
        if (facMediaRepoMgr.bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        TErrBox msgbFail
        (
            m_strTitle
            , facMediaRepoMgr.strMsg(kMRMgrMsgs::midStatus_CouldNotGetSet)
        );
        msgbFail.ShowIt(*this);

        return kCIDLib::False;
    }
    return kCIDLib::True;
}


//
//  These check the details type combo box and return true to indicate if
//  it's set to the indicated type. The order is collections, items, then
//  details, so we just go by the index.
//
tCIDLib::TBoolean TMainFrameWnd::bShowColDetails()
{
    return (m_pwndDetType->c4CurItem() == 0);
}

tCIDLib::TBoolean TMainFrameWnd::bShowItemDetails()
{
    return (m_pwndDetType->c4CurItem() == 1);
}

tCIDLib::TBoolean TMainFrameWnd::bShowTitleDetails()
{
    return (m_pwndDetType->c4CurItem() == 2);
}


//
//  Gets the list of title sets for the indicated media type, sorted by our current
//  sort order.
//
tCIDLib::TCard4
TMainFrameWnd::c4GetSortedTitleList(        tCQCMedia::TSetIdList&  colToFill
                                    , const tCQCMedia::EMediaTypes  eMType)
{
    // Select a sort function and then call the query
    tCIDLib::ESortComps (*pfnSort)(const TMediaTitleSet&, const TMediaTitleSet&);
    switch(m_eSortOrder)
    {
        case tCQCMedia::ESortOrders::Artist :
            pfnSort = &TMediaTitleSet::eCompByArtist;
            break;

        case tCQCMedia::ESortOrders::Title :
            pfnSort = &TMediaTitleSet::eCompByTitle;
            break;

        case tCQCMedia::ESortOrders::Year :
            pfnSort = &TMediaTitleSet::eCompByYear;
            break;

        default :
            CIDAssert2(L"Unknown title list sort order");
            break;
    };
    return m_mdbEdit.c4QuerySetList(eMType, colToFill, pfnSort);
}


//
//  This is called when the user acks to do a search and replace on external media
//  file paths. For now that's just movies since the audio is ripped by us and in
//  the same directory as the repository.
//
tCIDLib::TVoid
TMainFrameWnd::ChangeExtMediaPaths(const TString& strFind, const TString& strRepWith)
{
    try
    {
        tCIDLib::TCard4 c4ModCnt = 0;
        if (m_mdbEdit.bChangeExtMediaPaths(strFind, strRepWith, c4ModCnt))
        {
            //
            //  If any changes reported, force a refresh of the current item just in
            //  case it was affected.
            //
            if (c4ModCnt)
            {
                m_pwndTitleList->SelectByIndex
                (
                    m_pwndTitleList->c4CurItem(), kCIDLib::True
                );
            }
        }

        TOkBox msgbDone
        (
            m_strTitle
            , facMediaRepoMgr.strMsg
              (
                kMRMgrMsgs::midStatus_SearchRepDone, TCardinal(c4ModCnt)
              )
        );
        msgbDone.ShowIt(*this);
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
//  If the user bails out of something, we need to clean up any files that were
//  written to the upload directory, but only the ones associated with that particular
//  operation, since others could be there from previous operations still awaiting
//  upload. We get a list of the files to delete.
//
tCIDLib::TVoid TMainFrameWnd::CleanupFiles(const tCIDLib::TStrList& colFiles)
{
    const tCIDLib::TCard4 c4Count = colFiles.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        try
        {
            TFileSys::DeleteFile(colFiles[c4Index]);
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
    }
}


//
//  We provide a menu option to delete the currently selected title, and all of its
//  collections and all of their items.
//
tCIDLib::TVoid TMainFrameWnd::DeleteTitle()
{
    // Get the seleced title. If none, then just do nothing
    tCIDLib::TCard4 c4TitleInd = m_pwndTitleList->c4CurItem();
    if (c4TitleInd == kCIDLib::c4MaxCard)
        return;

    // Get the selected title set
    const TMediaTitleSet* pmtsCur = pmtsCurrent();

    // Make sure that they really want to do it
    {
        TYesNoBox msgbQ
        (
            facMediaRepoMgr.strMsg(kMRMgrMsgs::midMain_FrameTitle)
            , facMediaRepoMgr.strMsg
              (
                kMRMgrMsgs::midQ_DeleteTitle, pmtsCur->strName()
              )
        );
        if (!msgbQ.bShowIt(*this))
            return;
    }

    // Remove this item from the database and then from our list
    try
    {
        m_mdbEdit.PruneHierarchy(m_eMediaType, kCIDLib::True, pmtsCur->c2Id());

        // Remove it from the window list
        m_pwndTitleList->RemoveCurrent();
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
//  We need to react to changes in any of the list boxes or combo boxes and
//  update data accordingly.
//
tCIDCtrls::EEvResponses TMainFrameWnd::eLBHandler(TListChangeInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kMediaRepoMgr::ridMain_ColList)
    {
        if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::SelChanged)
        {
            // Show the items for the newly selected collection
            LoadItems();

            // If showing collection details, then update the details
            if (bShowColDetails())
                ShowDetails();
        }
         else if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::Invoked)
        {
            EditCollection();
        }
         else if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::Cleared)
        {
            // Clear out the item list window
            m_pwndItemList->RemoveAll();

            // If we are showing details for collections, then update them
            if (bShowColDetails())
                ShowDetails();
        }
    }
     else if (wnotEvent.widSource() == kMediaRepoMgr::ridMain_DetType)
    {
        if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::SelChanged)
        {
            //
            //  They changed the type of details they want to show, so just
            //  call the show details method again.
            //
            ShowDetails();
        }
    }
     else if (wnotEvent.widSource() == kMediaRepoMgr::ridMain_ItemList)
    {
        if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::SelChanged)
        {
            // If we are showing details for items, then update the details
            if (bShowItemDetails())
                ShowDetails();
        }
         else if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::Invoked)
        {
            const TMediaItem* pmitemCur = pmitemCurrent();
            if (pmitemCur)
            {
                const TMediaCollect* pmcolCur = pmcolCurrent();

                // Copy the item for editing
                TMediaItem mitemEdit = *pmitemCur;

                TEdMItemDlg dlgEdit;
                if (dlgEdit.bRun(*this, *pmcolCur, mitemEdit))
                {
                    // Update the database first
                    m_mdbEdit.UpdateItem(mitemEdit);

                    // Update the window item in case the name was changed
                    m_pwndItemList->c4SetText
                    (
                        m_pwndItemList->c4CurItem(), mitemEdit.strName()
                    );
                }
            }
        }
    }
     else if (wnotEvent.widSource() == kMediaRepoMgr::ridMain_TitleList)
    {
        if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::SelChanged)
        {
            // Show the collections for the newly selected title
            LoadTitleCols();

            // If showing title details, then update the details
            if (bShowTitleDetails())
                ShowDetails();
        }
         else if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::Invoked)
        {
            const TMediaTitleSet* pmtsCur = pmtsCurrent();
            if (pmtsCur)
            {
                //
                //  This guy only deals with existing items from the DB, so we
                //  just give it a ref to our item object. It'll copy that internally
                //  and edit that copy. If the user commits, it'll put it in the DB
                //  and so the object pointed to by our pointer is updated.
                //
                TEditTitleSetDlg dlgEdit;
                TEditTitleSetDlg::EEditRes eRes = dlgEdit.eRun
                (
                    *this, *pmtsCur, m_mdbEdit, m_strRepoMoniker
                );

                if (eRes == TEditTitleSetDlg::EEditRes::Changed)
                {
                    // Update the current one in case the name changed
                    const tCIDLib::TCard4 c4TitleInd = m_pwndTitleList->c4CurItem();
                    m_pwndTitleList->c4SetText(c4TitleInd, pmtsCur->strName());

                    //
                    //  Reselect the title so that the collection list will update, in
                    //  case we modified currently displayed info.
                    //
                    m_pwndTitleList->SelectByIndex(c4TitleInd, kCIDLib::True);
                }
                 else if (eRes == TEditTitleSetDlg::EEditRes::Deleted)
                {
                    //
                    //  They removed all the collections and so the title set was
                    //  deleted. So we need to remove it from our list and the DB.
                    //  The collections will already have been removed.
                    //
                    m_pwndTitleList->RemoveCurrent();
                }
            }
        }
         else if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::Cleared)
        {
            // Clear out the collection list window
            m_pwndColList->RemoveAll();

            // If we are showing details for titles, then update them
            if (bShowItemDetails())
                ShowDetails();
        }
    }
     else
    {
        return tCIDCtrls::EEvResponses::Handled;
    }
    return tCIDCtrls::EEvResponses::Handled;
}



//
//  This is called if they double click on a collection. We let them edit the
//  collection info (and the items it contains and the art image.)
//
tCIDLib::TVoid TMainFrameWnd::EditCollection()
{
    //
    //  Get pointers to the current title set and collection. If either is not
    //  available, we do nothing.
    //
    const TMediaTitleSet* pmtsPar = pmtsCurrent();
    const TMediaCollect*  pmcolCur = pmcolCurrent();
    if (!pmtsPar || !pmcolCur)
        return;

    TJPEGImage imgArt;
    bLoadArt(pmcolCur->c2ArtId(), imgArt);

    TEdMetaDataDlg dlgEdit;
    tCIDLib::TBoolean bRet = dlgEdit.bRunDlg
    (
        *this, pmtsPar->c2Id(), pmcolCur->c2Id(), m_eMediaType, imgArt, m_mdbEdit
    );

    if (bRet)
    {
        //
        //  First of all, before we look up the (potentially new) title set, see if
        //  one we started off with is empty. If so, ask if they want to delete it.
        //
        if (!pmtsPar->c4ColCount())
        {
            TYesNoBox msgbDel
            (
                L"The previous title set is now empty. Do you want to delete it?"
            );
            if (msgbDel.bShowIt(*this))
            {
                try
                {
                    m_mdbEdit.PruneHierarchy(m_eMediaType, kCIDLib::True, pmtsPar->c2Id());

                    // Remove it from the window list
                    m_pwndTitleList->RemoveCurrent();
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
        }

        //
        //  They may have assigned this collection to a new title set that had to be
        //  created, or they may have assigned it to a new one. So we need to look up
        //  the parent title set and either add it, select it, or re-select the existing
        //  one (to insure we update any title set data we show for the selected one.)
        //
        tCIDLib::TCard2 c2ColInd;
        if (!m_mdbEdit.bFindColContainer(m_eMediaType, pmcolCur->c2Id(), c2ColInd, pmtsPar))
        {
            // This is bad, it's not in any title set
            TErrBox msgbErr(L"Could not find the parent title set of the collection");
            msgbErr.ShowIt(*this);
            return;
        }

        ShowNewListTitle(tCQCMedia::EMediaTypes::Music, pmtsPar->c2Id());
    }
}


//
//  When a collection is selected, we need to load its items. So we get called here
//  when it's time to load a new collection's items. We get the current selection in
//  the collection list and  we load up the item list with the items for the col.
//
//  If there is no selection in the collection list, we just empty the item
//  list.
//
tCIDLib::TVoid TMainFrameWnd::LoadItems()
{
    m_pwndItemList->RemoveAll();

    // Get this collection from the list. If none, then return with the list empty
    const TMediaCollect* pmcolCur = pmcolCurrent();
    if (!pmcolCur)
        return;

    const tCIDLib::TCard4 c4ItemCount = pmcolCur->c4ItemCount();
    for (tCIDLib::TCard4 c4ItemInd = 0; c4ItemInd < c4ItemCount; c4ItemInd++)
    {
        // Look up the item at this index
        tCIDLib::TCard2 c2ItemId = pmcolCur->c2ItemIdAt(c4ItemInd);
        const TMediaItem* pmitemCur = m_mdbEdit.pmitemById
        (
            m_eMediaType, c2ItemId, kCIDLib::False
        );

        // Add an item to our load list
        m_pwndItemList->c4AddItem(pmitemCur->strName(), c2ItemId);
    }

    if (c4ItemCount)
        m_pwndItemList->SelectByIndex(0, kCIDLib::True);
}


//
//  Called to load up the browser on initial load, or after a change in the media
//  type. We load up the titles into the title list. The selection of an initial
//  item causes the faulting in of its collections, and that faults in the detail
//  display on the selected collection, and the loading of the item list.
//
tCIDLib::TVoid TMainFrameWnd::LoadTitles()
{
    m_pwndTitleList->RemoveAll();

    // If there are no titles for the current media type, then just return empty
    const tCIDLib::TCard4 c4TitleCnt = m_mdbEdit.c4TitleSetCnt(m_eMediaType);
    if (!c4TitleCnt)
        return;

    // Change a few prefixes to better reflect the media type
    if (m_eMediaType == tCQCMedia::EMediaTypes::Movie)
    {
        m_pwndClient->strChildText
        (
            kMediaRepoMgr::ridMain_ArtistPref
            , facMediaRepoMgr.strMsg(kMRMgrMsgs::midMain_DirectPref)
        );

        m_pwndClient->strChildText
        (
            kMediaRepoMgr::ridMain_LabelPref
            , facMediaRepoMgr.strMsg(kMRMgrMsgs::midMain_StudioPref)
        );
    }
     else
    {
        m_pwndClient->strChildText
        (
            kMediaRepoMgr::ridMain_ArtistPref
            , facMediaRepoMgr.strMsg(kMRMgrMsgs::midMain_ArtistPref)
        );

        m_pwndClient->strChildText
        (
            kMediaRepoMgr::ridMain_LabelPref
            , facMediaRepoMgr.strMsg(kMRMgrMsgs::midMain_LabelPref)
        );
    }

    // Set the title list prefix to reflect the media type
    TWindow* pwndTitle = m_pwndClient->pwndChildById(kMediaRepoMgr::ridMain_TitleListPref);
    pwndTitle->strWndText
    (
        facMediaRepoMgr.strMsg
        (
            kMRMgrMsgs::midMain_TitleListPref
            , tCQCMedia::strXlatEMediaTypes(m_eMediaType)
        )
    );

    // Get the sorted list of titles
    tCQCMedia::TSetIdList colSets(tCIDLib::EAdoptOpts::NoAdopt);
    const tCIDLib::TCard4 c4Count = c4GetSortedTitleList(colSets, m_eMediaType);

    // And load the list window from that, already sorted
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4TitleCnt; c4Index++)
    {
        const TMediaTitleSet* pmtsCur = colSets[c4Index];
        m_pwndTitleList->c4AddItem(pmtsCur->strName(), pmtsCur->c2Id());
    }

    if (c4TitleCnt)
        m_pwndTitleList->SelectByIndex(0, kCIDLib::True);

    // Update the title text with the current repo moniker, just in case
    TString strTitle(kMRMgrMsgs::midMain_FrameTitle, facMediaRepoMgr);
    strTitle.Append(L" [");
    strTitle.Append(m_strRepoMoniker);
    strTitle.Append(L"]");
    strWndText(strTitle);
}


//
//  When a title is selected, we need to load it's collections. So we get called
//  here when that happens.
//
//  If there is no selection in the title list, we just empty the collection
//  list.
//
tCIDLib::TVoid TMainFrameWnd::LoadTitleCols()
{
    m_pwndColList->RemoveAll();

    // Get the currently selected title set. If empty, then just return empty
    const TMediaTitleSet* pmtsCur = pmtsCurrent();
    if (!pmtsCur)
        return;

    //
    //  For each of its collections, get the id and see if it's in our list.
    //  If we hit one that's not, then break out, create the client proxy,
    //  and download any that we are missing.
    //
    const tCIDLib::TCard4 c4ColCount = pmtsCur->c4ColCount();
    for (tCIDLib::TCard4 c4ColIndex = 0; c4ColIndex < c4ColCount; c4ColIndex++)
    {
        tCIDLib::TCard2 c2ColId = pmtsCur->c2ColIdAt(c4ColIndex);
        const TMediaCollect* pmcolCur = m_mdbEdit.pmcolById
        (
            m_eMediaType, c2ColId, kCIDLib::False
        );

        if (pmcolCur)
            m_pwndColList->c4AddItem(pmcolCur->strName(), c2ColId);
    }

    if (c4ColCount)
        m_pwndColList->SelectByIndex(0, kCIDLib::True);
}



//
//  When the user commits changes, this is called. We do some checking to see if
//  there are changes to be sent. If so, we invoke the upload dialog, which will
//  do the upload. If it succeeds, we query back the database to pick up any changes
//  the server side made in the process of storing away the data.
//
tCIDLib::TVoid TMainFrameWnd::SendChanges()
{
    // Invoke the upload dialog
    TString strErrText;
    TUploadDataDlg dlgUpload;

    const tCIDLib::TBoolean bRes = dlgUpload.bRun
    (
        *this, strErrText, m_mdbEdit, m_strLeaseId, m_strRepoMoniker
    );

    if (!bRes)
    {
        TErrBox msgbFailed(m_strTitle, strErrText);
        msgbFailed.ShowIt(*this);
        return;
    }

    //
    //  Just to be sure, we now download the database again. Load it into a temp DB
    //  first, and copy it to ours after success. This also naturally removes any
    //  cached image data we had, so that we don't try to upload them again.
    //
    try
    {
        tCQCMedia::EMTFlags eMTFlags;
        TMediaDB            mdbNew;
        TString             strDBSerNum;
        TString             strRepoPath;

        tMediaRepoMgr::TRepoMgrProxyPtr orbcRepo
        (
            facMediaRepoMgr.orbcMakeProxy(m_strRepoMoniker)
        );

        const tCIDLib::TBoolean bNewData = facMediaRepoMgr.bDownloadDB
        (
            orbcRepo, mdbNew, eMTFlags, strDBSerNum, strRepoPath
        );

        // We should always get new data, but be sure
        if (bNewData)
        {
            m_mdbEdit.TakeFrom(mdbNew);
            m_eMTFlags = eMTFlags;
            m_strRepoPath = strRepoPath;
            m_strDBSerialNum = strDBSerNum;
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
            , facMediaRepoMgr.strMsg(kMRMgrErrs::errcComm_RefreshDB)
            , errToCatch
        );
    }
}


//
//  When the selection in one of the lists changes, this is called to
//  update the detail display. We show details for different things basd
//  on the details type combo box, so we have to get info from a different
//  place for each scenario.
//
tCIDLib::TVoid TMainFrameWnd::ShowDetails()
{
    // Get the list window item for the detail level, if there is one
    tCIDLib::TCard4 c4ListInd = kCIDLib::c4MaxCard;
    if (bShowColDetails())
        c4ListInd = m_pwndColList->c4CurItem();
    else if (bShowItemDetails())
        c4ListInd = m_pwndItemList->c4CurItem();
    else if (bShowTitleDetails())
        c4ListInd = m_pwndTitleList->c4CurItem();

    // If the list has been emptied, then clear them and return
    if (c4ListInd == kCIDLib::c4MaxCard)
    {
        m_pwndArt->Reset();
        m_pwndAspect->ClearText();
        m_pwndArtist->ClearText();
        m_pwndCast->ClearText();
        m_pwndDateAdded->ClearText();
        m_pwndDescr->ClearText();
        m_pwndDuration->ClearText();
        m_pwndLabel->ClearText();
        m_pwndLocInfo->ClearText();
        m_pwndMyRating->ClearText();
        m_pwndRating->ClearText();
        m_pwndTitle->ClearText();
        m_pwndYear->ClearText();
        return;
    }

    // There is one, so find it and output the details
    TString strArtist;
    TString strAspect;
    TString strCast;
    TString strDateAdded;
    TString strDescr;
    TString strDuration;
    TString strLabel;
    TString strLocInfo;
    TString strMyRating;
    TString strRating;
    TString strTitle;
    TString strYear;

    if (bShowColDetails())
    {
        const TMediaCollect* pmcolCur = pmcolCurrent();
        if (pmcolCur)
        {
            strArtist = pmcolCur->strArtist();
            strAspect = pmcolCur->strAspectRatio();
            strCast = pmcolCur->strCast();
            strDescr = pmcolCur->strDescr();
            strDuration.SetFormatted(pmcolCur->c4Duration());
            strLabel = pmcolCur->strLabel();
            strLocInfo = pmcolCur->strLocInfo();
            strRating = pmcolCur->strRating();
            strTitle = pmcolCur->strName();
            strYear.SetFormatted(pmcolCur->c4Year());

            // Download the collection level cover art
            LoadArt(pmcolCur->eType(), pmcolCur->c2ArtId());
        }
         else
        {
            // Clear the cover art
            m_pwndArt->Reset();
        }
    }
     else if (bShowItemDetails())
    {
        const TMediaItem* pmitemCur = pmitemCurrent();
        if (pmitemCur)
        {
            strArtist = pmitemCur->strArtist();
            strDuration.SetFormatted(pmitemCur->c4Duration());
            strLocInfo = pmitemCur->strLocInfo();
            strTitle = pmitemCur->strName();
        }

        // Clear the cover art
        m_pwndArt->Reset();
    }
     else if (bShowTitleDetails())
    {
        const TMediaTitleSet* pmtsCur = pmtsCurrent();
        if (pmtsCur)
        {
            TTime tmAdded(pmtsCur->enctAdded());
            tmAdded.FormatToStr(strDateAdded, TTime::strYYYYMMDD());

            strArtist = pmtsCur->strArtist();
            strMyRating.SetFormatted(pmtsCur->c4UserRating());
            strTitle = pmtsCur->strName();

            // Download the title level cover art
            LoadArt(pmtsCur->eType(), pmtsCur->c2ArtId());
        }
         else
        {
            // Clear the cover art
            m_pwndArt->Reset();
        }
    }

    if (!strAspect.bIsEmpty())
        strAspect.Append(L":1");

    m_pwndArtist->strWndText(strArtist);
    m_pwndAspect->strWndText(strAspect);
    m_pwndCast->strWndText(strCast);
    m_pwndDateAdded->strWndText(strDateAdded);
    m_pwndDescr->strWndText(strDescr);
    m_pwndDuration->strWndText(strDuration);
    m_pwndLabel->strWndText(strLabel);
    m_pwndLocInfo->strWndText(strLocInfo);
    m_pwndMyRating->strWndText(strMyRating);
    m_pwndRating->strWndText(strRating);
    m_pwndTitle->strWndText(strTitle);
    m_pwndYear->strWndText(strYear);
}



//
//  Stores our current repo settings to the master server config server.
//
tCIDLib::TVoid TMainFrameWnd::StoreRepoSettings()
{
    try
    {
        // Build up the binding name
        TString strBinding(kMediaRepoMgr::pszOSKey_Settings);
        strBinding.eReplaceToken(m_strRepoMoniker, L'1');

        tCIDLib::TCard4 c4SerialNum = 0;
        facCQCGKit().bAddUpdateStoreObj
        (
            strBinding, c4SerialNum, m_mrsCurrent, 1024
        );
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        TErrBox msgbFail
        (
            m_strTitle, facMediaRepoMgr.strMsg(kMRMgrMsgs::midStatus_CouldNotStoreSet)
        );
        msgbFail.ShowIt(*this);
    }
}


//
//  We let them pick an output directory and we write out a report with
//  the changer slots and what titles are in them.
//
tCIDLib::TVoid TMainFrameWnd::WriteSlotReport()
{
    // Let them select from the list of available changer drivers
    TVector<TString> colNames;
    try
    {
        tMediaRepoMgr::TRepoMgrProxyPtr orbcSrv
        (
            facMediaRepoMgr.orbcMakeProxy(m_strRepoMoniker)
        );
        orbcSrv->QueryChangers(colNames);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        TExceptDlg dlgErr
        (
            *this
            , m_strTitle
            , L"An error occured while querying the repository changer list"
            , errToCatch
        );
        return;
    }

    // If none, then tell them we can't do it
    if (colNames.bIsEmpty())
    {

        return;
    }

    // Use the generic list selection dialog to let them select one
    TString strChangerMoniker;
    tCIDLib::TBoolean bRes = facCIDWUtils().bListSelect
    (
        *this
        , L"Select a changer to report on"
        , colNames
        , strChangerMoniker
    );

    if (!bRes)
        return;

    //
    //  Get a map of slots from the changer. We get a bitset back
    //  with bits on for the taken slots. We also have to pass in the
    //  max slot number, so we ask the changer for that info.
    //
    tCIDLib::TCard4 c4SlotCnt;
    tCIDLib::TCard4 c4TakenCnt;
    TBitset         btsSlots;
    try
    {
        // Get a CQC Server admin proxy for the changer driver
        TWndPtrJanitor janPtr(tCIDCtrls::ESysPtrs::Wait);
        tCQCKit::TCQCSrvProxy orbcChDrv = facCQCKit().orbcCQCSrvAdminProxy
        (
            strChangerMoniker
        );

        // And read type slot count field from the driver
        tCIDLib::TCard4 c4SerNum = 0;
        orbcChDrv->bReadCardByName(c4SerNum, strChangerMoniker, L"SlotCnt", c4SlotCnt);

        // And now get a slot list
        c4TakenCnt = m_mdbEdit.c4QueryAvailSlots
        (
            strChangerMoniker, btsSlots, c4SlotCnt, &colNames
        );
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        TExceptDlg dlgErr
        (
            *this
            , m_strTitle
            , L"An error occured while querying the slot information"
            , errToCatch
        );
        return;
    }

    // If no slots are taken, then tell them nothing to do
    if (!c4TakenCnt)
    {


        return;
    }

    // Let them pick a target directory
    tCIDLib::TKVPList colTypes(1);
    colTypes.objAdd(TKeyValuePair(L"All", L"*.*"));
    TPathStr pathTar;
    tCIDLib::TStrList colSels;
    bRes = facCIDCtrls().bOpenFileDlg
    (
        *this
        , L"Select a target directory for the file"
        , TString::strEmpty()
        , colSels
        , colTypes
        , tCIDLib::eOREnumBits
          (
            tCIDCtrls::EFOpenOpts::SelectFolders
            , tCIDCtrls::EFOpenOpts::PathMustExist
          )
    );

    if (!bRes)
        return;

    // Ok, let's try to write out the report
    try
    {
        pathTar.AddLevel(L"CQSLRepo_SlotReport.Txt");
        TTextFileOutStream strmTar
        (
            pathTar
            , tCIDLib::ECreateActs::CreateAlways
            , tCIDLib::EFilePerms::Default
            , tCIDLib::EFileFlags::SequentialScan
            , tCIDLib::EAccessModes::Excl_Write
            , new TUTF8Converter
        );

        strmTar << L"CQSLRepo Slot Report for Changer: "
                << strChangerMoniker
                << L"\n-----------------------------------------\n\n";

        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4SlotCnt; c4Index++)
        {
            // Put out the number first
            if (c4Index < 9)
                strmTar << L"  ";
            else if (c4Index < 99)
                strmTar << L" ";

            strmTar << (c4Index + 1UL) << L". ";

            if (btsSlots.bBitState(c4Index))
                strmTar << colNames[c4Index];
            strmTar << kCIDLib::NewLn;
        }
        strmTar.Flush();
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        TExceptDlg dlgErr
        (
            *this
            , m_strTitle
            , L"An error occured while generating the report"
            , errToCatch
        );
        return;
    }
}

