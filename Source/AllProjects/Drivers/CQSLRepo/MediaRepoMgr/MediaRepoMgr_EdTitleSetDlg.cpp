//
// FILE NAME: MediaRepoMgr_EditTitleSet.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/13/2006
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
//  This is the header for the MediaRepoMgr_EditTitleSetDlg.cpp file, which
//  implements a dialog box for editing title set contents.
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
#include    "MediaRepoMgr_EdTitleSetDlg.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TEditTitleSetDlg,TDlgBox)



// ---------------------------------------------------------------------------
//   CLASS: TEditTitleSetDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TEditTitleSetDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TEditTitleSetDlg::TEditTitleSetDlg() :

    m_bNewImage(kCIDLib::False)
    , m_c2ArtId(0)
    , m_eRes(EEditRes::NoChange)
    , m_eMType(tCQCMedia::EMediaTypes::Count)
    , m_pmdbEdit(nullptr)
    , m_pwndArt(nullptr)
    , m_pwndArtist(nullptr)
    , m_pwndCancel(nullptr)
    , m_pwndColDel(nullptr)
    , m_pwndColDn(nullptr)
    , m_pwndColList(nullptr)
    , m_pwndColUp(nullptr)
    , m_pwndDelArt(nullptr)
    , m_pwndSave(nullptr)
    , m_pwndSetArt(nullptr)
    , m_pwndSortTitle(nullptr)
    , m_pwndTitle(nullptr)
    , m_pwndUserRating(nullptr)
{
    // Set the JPEG image settings for our desired quality
    m_imgiNew.bOptimalEncoding(kCIDLib::True);
    m_imgiNew.c4CompQuality(80);
    m_imgiNew.eOutSample(tCIDJPEG::EOutSamples::F4_2_1);
}

TEditTitleSetDlg::~TEditTitleSetDlg()
{
}


// ---------------------------------------------------------------------------
//  TEditTitleSetDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
TEditTitleSetDlg::EEditRes
TEditTitleSetDlg::eRun( const   TWindow&        wndOwner
                        , const TMediaTitleSet& mtsEdit
                        ,       TMediaDB&       mdbToEdit
                        , const TString&        strRepoMoniker)
{
    // Store incoming stuff for later use
    m_c2ArtId        = mtsEdit.c2ArtId();
    m_eMType         = mtsEdit.eType();
    m_pmdbEdit       = &mdbToEdit;
    m_strRepoMoniker = strRepoMoniker;

    // Copy the incoming for editing
    m_mtsEdit  = mtsEdit;
    c4RunDlg(wndOwner, facMediaRepoMgr, kMediaRepoMgr::ridDlg_EdTitle);

    // Just return whatever result they left
    return m_eRes;
}


// ---------------------------------------------------------------------------
//  TEditTitleSetDlg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TEditTitleSetDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRet = TParent::bCreated();

    // Get typed pointers to some of the widgets
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_EdTitle_Art, m_pwndArt);
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_EdTitle_Artist, m_pwndArtist);
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_EdTitle_Cancel, m_pwndCancel);
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_EdTitle_ColDel, m_pwndColDel);
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_EdTitle_ColDn, m_pwndColDn);
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_EdTitle_ColList, m_pwndColList);
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_EdTitle_ColUp, m_pwndColUp);
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_EdTitle_DelArt, m_pwndDelArt);
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_EdTitle_Save, m_pwndSave);
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_EdTitle_SetArt, m_pwndSetArt);
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_EdTitle_SortTitle, m_pwndSortTitle);
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_EdTitle_Title, m_pwndTitle);
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_EdTitle_UserRating, m_pwndUserRating);

    // Set an AR maintaining placement type on the art
    m_pwndArt->ePlacement(tCIDGraphDev::EPlacement::FitAR);

    // Load up incoming stuff
    m_pwndArtist->strWndText(m_mtsEdit.strArtist());
    m_pwndSortTitle->strWndText(m_mtsEdit.strSortTitle());
    m_pwndTitle->strWndText(m_mtsEdit.strName());

    // If this guy has an art id set, then try to load it
    if (m_c2ArtId)
        LoadArt(m_c2ArtId);

    // Load up the user rating combo box
    m_pwndUserRating->LoadNumRange(1, 10);

    // Select the current value, which is just the rating minus one
    m_pwndUserRating->SelectByIndex(m_mtsEdit.c4UserRating() - 1);

    //
    //  Load the names of the collections. Store the id of the collections in
    //  an extra field of the list window items, so we can be sure we are associating
    //  them with the right collections.
    //
    const tCIDLib::TCard4 c4ColCount = m_mtsEdit.c4ColCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ColCount; c4Index++)
    {
        // Get the id of the collection at this index within the set
        const tCIDLib::TCard2 c2CurId = m_mtsEdit.c2ColIdAt(c4Index);

        // And use that to look up the collection
        const TMediaCollect* pmcolCur = m_pmdbEdit->pmcolById
        (
            m_eMType, c2CurId, kCIDLib::True
        );

        m_pwndColList->c4AddItem(pmcolCur->strName(), pmcolCur->c2Id());
    }

    // Install handlers
    m_pwndCancel->pnothRegisterHandler(this, &TEditTitleSetDlg::eClickHandler);
    m_pwndColDel->pnothRegisterHandler(this, &TEditTitleSetDlg::eClickHandler);
    m_pwndColDn->pnothRegisterHandler(this, &TEditTitleSetDlg::eClickHandler);
    m_pwndColUp->pnothRegisterHandler(this, &TEditTitleSetDlg::eClickHandler);
    m_pwndDelArt->pnothRegisterHandler(this, &TEditTitleSetDlg::eClickHandler);
    m_pwndSave->pnothRegisterHandler(this, &TEditTitleSetDlg::eClickHandler);
    m_pwndSetArt->pnothRegisterHandler(this, &TEditTitleSetDlg::eClickHandler);

    return bRet;
}


// ---------------------------------------------------------------------------
//  TEditTitleSetDlg: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TEditTitleSetDlg::bValidate()
{
    // Scale the image down if we got a new one, and if it's too big
    if (m_bNewImage)
    {
        try
        {
            if ((m_imgiNew.c4Width() > kMediaRepoMgr::c4MaxH)
            ||  (m_imgiNew.c4Height() > kMediaRepoMgr::c4MaxV))
            {
                // We need to scale it
                TSize szNew(m_imgiNew.szImage());

                if ((m_imgiNew.c4Width() > kMediaRepoMgr::c4MaxH)
                &&  (m_imgiNew.c4Height() > kMediaRepoMgr::c4MaxV))
                {
                    // It's bigger in both directions, so take the larger one
                    if (m_imgiNew.c4Width() > m_imgiNew.c4Height())
                        szNew.ScaleToWidthAR(kMediaRepoMgr::c4MaxH);
                    else
                        szNew.ScaleToHeightAR(kMediaRepoMgr::c4MaxV);
                }
                 else if (m_imgiNew.c4Width() > kMediaRepoMgr::c4MaxH)
                {
                    szNew.ScaleToWidthAR(kMediaRepoMgr::c4MaxH);
                }
                 else
                {
                    szNew.ScaleToHeightAR(kMediaRepoMgr::c4MaxV);
                }
                m_imgiNew.Scale(szNew, 4);
            }
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);

            TErrBox msgbErr(facMediaRepoMgr.strMsg(kMRMgrMsgs::midStatus_BadImage));
            msgbErr.ShowIt(*this);
            return kCIDLib::False;
        }
    }

    try
    {
        // The sort title and artist cannot be empty
        TString strSortTitle(m_pwndSortTitle->strWndText());
        strSortTitle.StripWhitespace();
        TString strArtist(m_pwndArtist->strWndText());
        strArtist.StripWhitespace();

        TWindow* pwndErr = 0;
        TString strErr;
        if (strSortTitle.bIsEmpty())
        {
            pwndErr = m_pwndSortTitle;
            strErr.LoadFromMsg(kMRMgrMsgs::midStatus_NoSortTitle, facMediaRepoMgr);
        }
         else if (strArtist.bIsEmpty())
        {
            pwndErr = m_pwndSortTitle;
            strErr.LoadFromMsg(kMRMgrMsgs::midStatus_MustHaveLastName, facMediaRepoMgr);
        }

        // If any errors, show the message, put the focus there, and fail
        if (pwndErr)
        {
            TErrBox msgbErr(strErr);
            msgbErr.ShowIt(*this);
            pwndErr->TakeFocus();
            return kCIDLib::False;
        }

        // Set this basic stuff on our title set
        m_mtsEdit.strArtist(strArtist);
        m_mtsEdit.strSortTitle(strSortTitle);
        m_mtsEdit.c4UserRating(m_pwndUserRating->c4CurItem() + 1);

        //
        //  If they loaded a new image, we need to store it for later upload. What
        //  we do is just 'cache' it in the database on the image object, which is
        //  normally just a server side thing, but it lets us store it such that we
        //  know later it needs to be uploaded, and without having to have some other
        //  data structure to store it in.
        //
        //  If this guy was not already associated with a media image, we create a
        //  new one and it to the database first.
        //
        if (m_bNewImage)
        {
            // Get the image flattened and generate a persistent id for it
            TString strPerId;
            TChunkedBinOutStream strmOut(16 * (1024 * 1024));
            strmOut << m_imgiNew << kCIDLib::FlushIt;

            TChunkedBinInStream strmSrc(strmOut);
            facCQCMedia().CreatePersistentId
            (
                strmSrc, strmOut.c4CurSize(), strPerId
            );

            tCIDLib::TCard2 c2Id = m_mtsEdit.c2ArtId();
            if (!c2Id)
            {
                //
                //  Create a new one and add it to our database. We don't know any of
                //  the paths at this point.
                //
                TMediaImg* pmimgStore = new TMediaImg
                (
                    m_eMType, TString::strEmpty(), TString::strEmpty()
                );
                c2Id = m_pmdbEdit->c2AddImage(pmimgStore);

                // Update the title to reference this art id
                m_mtsEdit.c2ArtId(c2Id);
            }

            //
            //  Store the art on the image object. Reset the source stream before
            //  we do it, so that it's back to the beginning.
            //
            strmSrc.Reset();
            m_pmdbEdit->SetArt
            (
                c2Id
                , m_eMType
                , tCQCMedia::ERArtTypes::LrgCover
                , strmSrc
                , strmOut.c4CurSize()
                , strPerId
            );
        }

        //
        //  Deal with the collections. We first find any that were in the original
        //  but aren't now in our edited set info. For those, we remove them from
        //  the database. So get the original set so we can compare.
        //
        //  If these are not playlists we are removing, then any items they reference
        //  will be removed as well.
        //
        {
            const TMediaTitleSet* pmtsOrg = m_pmdbEdit->pmtsById
            (
                m_eMType, m_mtsEdit.c2Id(), kCIDLib::True
            );

            tCIDLib::TCard4 c4Count = pmtsOrg->c4ColCount();
            tCIDLib::TCard4 c4Index = 0;
            while (c4Index < c4Count)
            {
                const tCIDLib::TCard2 c2CurCol = pmtsOrg->c2ColIdAt(c4Index);

                tCIDLib::TCard2 c2ColInd;
                if(m_mtsEdit.bContainsCol(c2CurCol, c2ColInd))
                {
                    c4Index++;
                }
                 else
                {
                    m_pmdbEdit->PruneHierarchy(m_eMType, kCIDLib::False, c2CurCol);
                    c4Count--;
                }
            }
        }

        //
        //  If the title set has ended up empty, we remove it as well. Else we
        //  update the database with the new title set contents.
        //
        if (!m_mtsEdit.c4ColCount())
        {
            m_pmdbEdit->PruneHierarchy(m_eMType, kCIDLib::True, m_mtsEdit.c2Id());
            m_eRes = EEditRes::Deleted;
        }
         else
        {
            // Assume they changed soemthing so copy over the changes
            m_pmdbEdit->UpdateTitle(m_mtsEdit);
            m_eRes = EEditRes::Changed;
        }
    }

    catch(const TError& errToCatch)
    {
        TExceptDlg dlgErr
        (
            *this
            , strWndText()
            , facMediaRepoMgr.strMsg(kMRMgrMsgs::midStatus_Exception)
            , errToCatch
        );
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


//
//  Delete the selected collection.
//
tCIDLib::TVoid TEditTitleSetDlg::DeleteCollect()
{
    const tCIDLib::TCard4 c4CurInd = m_pwndColList->c4CurItem();
    if (c4CurInd == kCIDLib::c4MaxCard)
        return;

    try
    {
        //
        //  Get this collection id and remove it from our list and from the
        //  title set.
        //
        m_mtsEdit.RemoveColAt(c4CurInd);
        m_pwndColList->RemoveAt(c4CurInd);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        TExceptDlg dlgErr
        (
            *this
            , strWndText()
            , facMediaRepoMgr.strMsg(kMRMgrMsgs::midStatus_Exception)
            , errToCatch
        );
    }
}


// Handle button clicks
tCIDCtrls::EEvResponses TEditTitleSetDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kMediaRepoMgr::ridDlg_EdTitle_Cancel)
    {
        m_eRes = EEditRes::NoChange;
        EndDlg(kMediaRepoMgr::ridDlg_EdTitle_Cancel);
    }
     else if (wnotEvent.widSource() == kMediaRepoMgr::ridDlg_EdTitle_ColDel)
    {
        const tCIDLib::TCard4 c4ColCount = m_pwndColList->c4ItemCount();

        //
        //  If this is the last collection, tell them that the title set is
        //  going to be removed and the dialog closed if they do this. Else,
        //  just confirm the removal.
        //
        TString strQuestion;
        if (c4ColCount == 1)
            strQuestion.LoadFromMsg(kMRMgrMsgs::midQ_DeleteLastCollect, facMediaRepoMgr);
        else
            strQuestion.LoadFromMsg(kMRMgrMsgs::midQ_DeleteCollect, facMediaRepoMgr);

        TYesNoBox msgbQ(strQuestion);
        if (msgbQ.bShowIt(*this))
            DeleteCollect();
    }
     else if (wnotEvent.widSource() == kMediaRepoMgr::ridDlg_EdTitle_ColDn)
    {
        //
        //  If we have more than one collection, and it's not already at the
        //  end, then swap it with the one after it.
        //
        const tCIDLib::TCard4 c4Index = m_pwndColList->c4CurItem();
        const tCIDLib::TCard4 c4Count = m_pwndColList->c4ItemCount();
        if ((c4Count > 1) && (c4Index < (c4Count - 1)))
        {
            m_pwndColList->bMoveDn(kCIDLib::True);
            m_mtsEdit.SwapCollects(c4Index, c4Index + 1);
        }
    }
     else if (wnotEvent.widSource() == kMediaRepoMgr::ridDlg_EdTitle_ColUp)
    {
        //
        //  If we have more than one collection, and it's not already at the
        //  top, then swap it with the one before it.
        //
        const tCIDLib::TCard4 c4Index = m_pwndColList->c4CurItem();
        const tCIDLib::TCard4 c4Count = m_pwndColList->c4ItemCount();
        if ((c4Count > 1) && c4Index)
        {
            m_pwndColList->bMoveUp(kCIDLib::True);
            m_mtsEdit.SwapCollects(c4Index, c4Index - 1);
        }
    }
     else if (wnotEvent.widSource() == kMediaRepoMgr::ridDlg_EdTitle_DelArt)
    {
        // Clear the image window and zero our art id
        m_pwndArt->Reset();
        m_mtsEdit.c2ArtId(0);

        // In case we loaded a new one, forget that now
        m_bNewImage = kCIDLib::False;
    }
     else if (wnotEvent.widSource() == kMediaRepoMgr::ridDlg_EdTitle_Save)
    {
        // Validate the entered stuf and collect it up
        if (bValidate())
            EndDlg(kMediaRepoMgr::ridDlg_EdTitle_Save);
    }
     else if (wnotEvent.widSource() == kMediaRepoMgr::ridDlg_EdTitle_SetArt)
    {
        SelectArt();
    }
    return tCIDCtrls::EEvResponses::Handled;
}


// Called on entry to load the existing art if any
tCIDLib::TVoid TEditTitleSetDlg::LoadArt(const tCIDLib::TCard2 c2ArtId)
{
    // If the id is zero, then there isn't any
    if (!c2ArtId)
        return;

    try
    {
        TWndPtrJanitor janPtr(tCIDCtrls::ESysPtrs::Wait);
        tMediaRepoMgr::TRepoMgrProxyPtr orbcSrv
        (
            facMediaRepoMgr.orbcMakeProxy(m_strRepoMoniker)
        );

        THeapBuf mbufImg(1024 * 64);
        tCIDLib::TCard4 c4Size;
        const tCIDLib::TBoolean bRes = orbcSrv->bQueryArtById
        (
            m_eMType
            , tCQCMedia::ERArtTypes::LrgCover
            , c2ArtId
            , c4Size
            , mbufImg
            , kCIDLib::False
        );

        if (bRes)
        {
            TBinMBufInStream strmSrc(&mbufImg, c4Size);
            TJPEGImage imgArt;
            strmSrc >> imgArt;

            TGraphWndDev gdevCompat(*this);
            m_pwndArt->SetImage(gdevCompat, imgArt);
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
            , facMediaRepoMgr.strMsg
              (
                kMRMgrMsgs::midStatus_CantLoadArt
                , tCQCMedia::strXlatEDataTypes(tCQCMedia::EDataTypes::TitleSet)
              )
            , errToCatch
        );
    }
}


tCIDLib::TVoid TEditTitleSetDlg::SelectArt()
{
    // Let the user select a local image
    TPathStr pathSrcFile;
    {
        tCIDLib::TKVPList colFileTypes(1);
        colFileTypes.objAdd(TKeyValuePair(L"Image Files", L"*.bmp,*.png,*.jpeg,*.jpg"));

        tCIDLib::TStrList colSel;
        const tCIDLib::TBoolean bRes = facCIDCtrls().bOpenFileDlg
        (
            *this
            , facMediaRepoMgr.strMsg(kMRMgrMsgs::midPrompt_SelectImg)
            , TString::strEmpty()
            , colSel
            , colFileTypes
            , tCIDCtrls::EFOpenOpts::FileMustExist
        );

        if (!bRes)
            return;


        pathSrcFile = colSel[0];
    }

    try
    {
        // Use the image factory to laod the file, converting to JPEG if needed
        facCIDImgFact().bLoadToJPEG(pathSrcFile, m_imgiNew, kCIDLib::True);

        // Remember we have a new image to upload
        m_bNewImage = kCIDLib::True;

        // Update our previe window with the new stuff
        TGraphWndDev gdevCompat(*this);
        m_pwndArt->SetImage(gdevCompat, m_imgiNew);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        TExceptDlg dlgErr
        (
            *this
            , strWndText()
            , facMediaRepoMgr.strMsg
              (
                kMRMgrMsgs::midStatus_CantLoadArt
                , tCQCMedia::strXlatEDataTypes(tCQCMedia::EDataTypes::Collect)
              )
            , errToCatch
        );
    }
}

