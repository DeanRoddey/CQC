//
// FILE NAME: MediaRepoMgr_EdMetaDlg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/23/2006
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
//  This dialog allows the user to edit the metadata for a title.
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
#include    "MediaRepoMgr_EdCastDlg.hpp"
#include    "MediaRepoMgr_EdDescrDlg.hpp"
#include    "MediaRepoMgr_EdMItemDlg.hpp"
#include    "MediaRepoMgr_EdMetaDlg.hpp"
#include    "MediaRepoMgr_SelTitleSetDlg.hpp"



// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TEdMetaDataDlg, TDlgBox)


// ---------------------------------------------------------------------------
//  Local types and constants
// ---------------------------------------------------------------------------
namespace MediaRepoMgr_EdMetaDlg
{
    // -----------------------------------------------------------------------
    //  We keep up with the last few entered values of some of the fields, to
    //  aid in editing. This allows the user to drop down a list and pick from
    //  a list of recent values. These are used to hold those values.
    // -----------------------------------------------------------------------
    const tCIDLib::TCard4   c4MaxPrevItems = 10;
    tCIDLib::TStrList       colPrevArtists;
    tCIDLib::TStrList       colPrevAspects;
    tCIDLib::TStrList       colPrevLabels;
    TString                 strName;
    tCIDLib::TStrList       colPrevMediaFmts;
    tCIDLib::TStrList       colPrevYears;
}



// ---------------------------------------------------------------------------
//   CLASS: TEdMetaDataDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TEdMetaDataDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TEdMetaDataDlg::TEdMetaDataDlg() :

    m_bEditMode(kCIDLib::False)
    , m_bManualMode(kCIDLib::False)
    , m_bNewImage(kCIDLib::False)
    , m_bTitleSelected(kCIDLib::False)
    , m_c2ColId(0)
    , m_c2SetId(0)
    , m_c4KnownItemCnt(0)
    , m_eMType(tCQCMedia::EMediaTypes::Count)
    , m_pwndAction(nullptr)
    , m_pwndAddCategory(nullptr)
    , m_pwndAnamorphic(nullptr)
    , m_pwndArt(nullptr)
    , m_pwndArtist(nullptr)
    , m_pwndAspect(nullptr)
    , m_pwndASIN(nullptr)
    , m_pwndBrowse(nullptr)
    , m_pwndCancel(nullptr)
    , m_pwndCategories(nullptr)
    , m_pwndColName(nullptr)
    , m_pwndContinue(nullptr)
    , m_pwndDelCategory(nullptr)
    , m_pwndDuration(nullptr)
    , m_pwndEditArt(nullptr)
    , m_pwndEditCast(nullptr)
    , m_pwndEditDescr(nullptr)
    , m_pwndItems(nullptr)
    , m_pwndItemDel(nullptr)
    , m_pwndItemDn(nullptr)
    , m_pwndItemIns(nullptr)
    , m_pwndItemUp(nullptr)
    , m_pwndLabel(nullptr)
    , m_pwndLocInfo(nullptr)
    , m_pwndMediaFmt(nullptr)
    , m_pwndMoreArt(nullptr)
    , m_pwndRating(nullptr)
    , m_pwndSelTitle(nullptr)
    , m_pwndSetName(nullptr)
    , m_pwndUPC(nullptr)
    , m_pwndYear(nullptr)
{
}

TEdMetaDataDlg::~TEdMetaDataDlg()
{
}


// ---------------------------------------------------------------------------
//  TEdMetaDataDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  This mode is for editing an existing collection that has already been successfully
//  added, and the user just wants to edit the existing data. We just get the id of
//  the collection. This means it already has to have a containing collection, which
//  the caller provides as well.
//
tCIDLib::TBoolean
TEdMetaDataDlg::bRunDlg(const   TWindow&                wndOwner
                        , const tCIDLib::TCard2         c2TitleId
                        , const tCIDLib::TCard2         c2ColId
                        , const tCQCMedia::EMediaTypes  eMType
                        , const TJPEGImage&             imgiArt
                        ,       TMediaDB&               mdbEdit)
{
    //
    //  Store some incoming info for later use. It's edit mode sine this is an
    //  existing collection. It's not manual mode since it already exists.
    //
    m_bEditMode     = kCIDLib::True;
    m_bTitleSelected= kCIDLib::True;
    m_c2SetId       = c2TitleId;
    m_c2ColId       = c2ColId;
    m_c4KnownItemCnt= 0;
    m_eMType        = eMType;
    m_pmdbEdit      = &mdbEdit;

    // Look up the parent collection to make sure it exists
    const TMediaTitleSet* pmtsOrg = mdbEdit.pmtsById(eMType, c2TitleId, kCIDLib::True);

    // As a sanity check, make sure the title set contains the collection
    tCIDLib::TCard2 c2ColInd;
    if (!pmtsOrg->bContainsCol(c2ColId, c2ColInd))
    {
        TErrBox msgbErr(L"The collection's parent title set could not be found");
        msgbErr.ShowIt(wndOwner);
        return kCIDLib::False;
    }

    // Copy the collection, title set and art info
    m_mcolEdit = *mdbEdit.pmcolById(eMType, c2ColId, kCIDLib::True);
    m_mtsEdit = *pmtsOrg;
    m_imgiEdit = imgiArt;

    //
    //  And make copies of any items it contains, and save the original item
    //  ids for later use to know if any have been removed.
    //
    const tCIDLib::TCard4 c4Count = m_mcolEdit.c4ItemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const tCIDLib::TCard2 c2CurId = m_mcolEdit.c2ItemIdAt(c4Index);
        m_colItems.objAdd(*mdbEdit.pmitemById(eMType, c2CurId, kCIDLib::True));
        m_fcolOrgItemIds.c4AddElement(c2ColId);
    }

    const tCIDLib::TCard4 c4Res = c4RunDlg
    (
        wndOwner, facMediaRepoMgr, kMediaRepoMgr::ridDlg_EdMeta
    );

    return (c4Res == kMediaRepoMgr::ridDlg_EdMeta_Continue);
}


//
//  This mode is used to add a new playlist or a new manual mode collection. All
//  we need is the media type and whehter it's a playlist or not. In this scenario
//  we start with nothing. We just create a new, emtpy collection with a very basic
//  setup, add it to the database and then edit.
//
tCIDLib::TBoolean
TEdMetaDataDlg::bRunDlg(const   TWindow&                wndOwner
                        , const tCQCMedia::EMediaTypes  eMType
                        , const tCIDLib::TBoolean       bIsPlayList
                        ,       TMediaDB&               mdbEdit
                        ,       tCIDLib::TCard2&        c2TitleId
                        ,       tCIDLib::TCard2&        c2ColId)
{
    if (bIsPlayList)
    {
        CIDAssert(eMType == tCQCMedia::EMediaTypes::Music, L"Playlists must be music type");
    }

    //
    //  Store some incoming info for later use. If not a playlist, then it's a
    //  manual mode addition, so just reverse the playlist flag for that. It's
    //  not edit mode sine this is adding a new one.
    //
    m_bManualMode   = !bIsPlayList;
    m_c2ColId       = 0;
    m_c2SetId       = 0;
    m_c4KnownItemCnt= 0;
    m_eMType        = eMType;
    m_pmdbEdit      = &mdbEdit;

    // Start our collection to edit off with reasonable info
    TString strUniqueId;
    if (bIsPlayList)
        strUniqueId = L"PL_";
    strUniqueId.Append(TUniqueId::strMakeId());

    m_mcolEdit = TMediaCollect
    (
        L"New PlayList"
        , strUniqueId
        , 0
        , tCQCMedia::ELocTypes::FileItem
        , eMType
    );
    m_mcolEdit.bIsPlayList(bIsPlayList);

    // Do the all movies/music category depending on type
    if (eMType == tCQCMedia::EMediaTypes::Movie)
        m_mcolEdit.bAddCategory(kCQCMedia::c2CatId_AllMovies);
    else
        m_mcolEdit.bAddCategory(kCQCMedia::c2CatId_AllMusic);

    // if a playlist, set up a couple more things
    if (bIsPlayList)
    {
        m_mcolEdit.strArtist(L"Various Artists");
        m_mcolEdit.bAddCategory(kCQCMedia::c2CatId_PlayLists);
    }

    // And now we can edit the data
    const tCIDLib::TCard4 c4Res = c4RunDlg
    (
        wndOwner, facMediaRepoMgr, kMediaRepoMgr::ridDlg_EdMeta
    );

    if (c4Res == kMediaRepoMgr::ridDlg_EdMeta_Continue)
    {
        // Give back the title and collection ids we created
        c2TitleId = m_c2SetId;
        c2ColId = m_c2ColId;
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


//
//  This mode is for while a CD is being ripped. We get in the collection and
//  item info we got in the metadata query, the number of items, and the image
//  that we gotten from the metadata source. In this case we check to see if the
//  incoming image is set. If so, we set the new image flag, so it'll get cached
//  for later upload.
//
tCIDLib::TBoolean
TEdMetaDataDlg::bRunDlg(const   TWindow&                wndOwner
                        , const TMediaCollect&          mcolNew
                        , const tCQCMedia::TItemList&   colItems
                        , const TJPEGImage&             imgiArt
                        , const tCIDLib::TCard4         c4DiscItemCnt
                        , const tCIDLib::TBoolean       bManual
                        ,       TMediaDB&               mdbEdit
                        ,       tCIDLib::TCard2&        c2TitleId
                        ,       tCIDLib::TCard2&        c2ColId
                        ,       tCIDLib::TCard2&        c2ArtId)
{
    m_bEditMode     = kCIDLib::False;
    m_c2ColId       = 0;
    m_c2SetId       = 0;
    m_c4KnownItemCnt= c4DiscItemCnt;
    m_eMType        = mcolNew.eType();
    m_pmdbEdit      = &mdbEdit;

    // Copy the incoming collection, item, and image data for editing
    m_mcolEdit = mcolNew;
    m_colItems = colItems;
    m_imgiEdit = imgiArt;

    //
    //  If the height is > 1, then it's not just a default constructed image, so
    //  we assume it's incoming image data and set the new image flag to insure it
    //  gets stored.
    //
    if (imgiArt.c4Height() > 1)
        m_bNewImage = kCIDLib::True;

    const tCIDLib::TCard4 c4Res = c4RunDlg
    (
        wndOwner, facMediaRepoMgr, kMediaRepoMgr::ridDlg_EdMeta
    );

    if (c4Res == kMediaRepoMgr::ridDlg_EdMeta_Continue)
    {
        // Give back the ids we created
        c2TitleId = m_c2SetId;
        c2ColId = m_c2ColId;

        return kCIDLib::True;
    }
    return kCIDLib::False;
}



// ---------------------------------------------------------------------------
//  TEdMetaDataDlg: Protected inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TEdMetaDataDlg::bCancelRequest()
{
    TYesNoBox msgbConfirm(L"Discard any changes and exit?");
    return msgbConfirm.bShowIt(*this);
}


tCIDLib::TBoolean TEdMetaDataDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRet = TParent::bCreated();

    // First get typed pointers to some of the widgets
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_EdMeta_Action, m_pwndAction);
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_EdMeta_AddCat, m_pwndAddCategory);
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_EdMeta_Anamorphic, m_pwndAnamorphic);
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_EdMeta_Art, m_pwndArt);
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_EdMeta_Artist, m_pwndArtist);
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_EdMeta_Aspect, m_pwndAspect);
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_EdMeta_ASIN, m_pwndASIN);
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_EdMeta_Browse, m_pwndBrowse);
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_EdMeta_Cancel, m_pwndCancel);
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_EdMeta_CatList, m_pwndCategories);
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_EdMeta_ColName, m_pwndColName);
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_EdMeta_Continue, m_pwndContinue);
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_EdMeta_DelCat, m_pwndDelCategory);
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_EdMeta_Duration, m_pwndDuration);
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_EdMeta_EditArt, m_pwndEditArt);
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_EdMeta_EditCast, m_pwndEditCast);
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_EdMeta_EditDescr, m_pwndEditDescr);
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_EdMeta_ItemList, m_pwndItems);
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_EdMeta_ItemDel, m_pwndItemDel);
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_EdMeta_ItemDn, m_pwndItemDn);
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_EdMeta_ItemIns, m_pwndItemIns);
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_EdMeta_ItemUp, m_pwndItemUp);
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_EdMeta_Label, m_pwndLabel);
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_EdMeta_LocInfo, m_pwndLocInfo);
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_EdMeta_MediaFmt, m_pwndMediaFmt);
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_EdMeta_MoreArt, m_pwndMoreArt);
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_EdMeta_Rating, m_pwndRating);
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_EdMeta_SelTitle, m_pwndSelTitle);
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_EdMeta_SetName, m_pwndSetName);
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_EdMeta_UPC, m_pwndUPC);
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_EdMeta_Year, m_pwndYear);

    // Set an AR maintaining placement type on the art
    m_pwndArt->ePlacement(tCIDGraphDev::EPlacement::FitAR);

    // Change a couple prefixes if movie data
    if (m_eMType == tCQCMedia::EMediaTypes::Movie)
    {
        strChildText
        (
            kMediaRepoMgr::ridDlg_EdMeta_ArtistPref
            , facMediaRepoMgr.strMsg(kMRMgrMsgs::midMain_DirectPref)
        );

        strChildText
        (
            kMediaRepoMgr::ridDlg_EdMeta_LabelPref
            , facMediaRepoMgr.strMsg(kMRMgrMsgs::midMain_StudioPref)
        );
    }

    //
    //  If we are working on a play list, then we disable the action stuff,
    //  and disable the add item, leaving the other item buttons available
    //  for moving or deleting items.
    //
    //  Else, we are working on a real collection. If we are in edit mode,
    //  we just enable the location field if the incoming collection has a
    //  collection level type location type. Else, we disable it and the
    //  action combo. And we disable the item delete button, since we don't
    //  want to get out of sync with the ripped files now.
    //
    //  If not in edit mode, then we are doing a new collection, so we
    //  load the action combo, with the actions that are valid for the
    //  media type. We store the enum value in the loaded list window
    //  item id field, so that we can get back the value (because we
    //  aren't loading them all and cannot therefore just use the index
    //  of the selected item.)
    //
    if (m_mcolEdit.bIsPlayList())
    {
        m_pwndAction->SetEnable(kCIDLib::False);
        m_pwndLocInfo->SetEnable(kCIDLib::False);
        m_pwndItemIns->SetEnable(kCIDLib::False);
        m_pwndBrowse->SetVisibility(kCIDLib::False);
    }
     else
    {
        if (m_bEditMode)
        {
            // We are editing an existing collection
            if (m_eMType == tCQCMedia::EMediaTypes::Music)
            {
                //
                //  If changer based, we just load the changer based option
                //  so so that they can select a new changer slot. They can't
                //  change to ripped. If ripped they can't change at all without
                //  removing the title first, so we disable the location stuff.
                //
                if (m_mcolEdit.eLocType() == tCQCMedia::ELocTypes::Changer)
                {
                    m_pwndAction->c4AddItem
                    (
                        facMediaRepoMgr.strMsg(kMRMgrMsgs::midAct_ChangerBased)
                        , tCIDLib::c4EnumOrd(tCQCMedia::ELocTypes::Changer)
                    );
                    m_pwndAction->SelectByIndex(0);

                    // And show the current location info
                    m_pwndLocInfo->strWndText(m_mcolEdit.strLocInfo());
                }
                 else
                {
                    //
                    //  It doesn't have collection level loc info, so disable
                    //  editing of the location info.
                    //
                    m_pwndAction->SetEnable(kCIDLib::False);
                    m_pwndLocInfo->SetEnable(kCIDLib::False);
                    m_pwndBrowse->SetVisibility(kCIDLib::False);
                }
            }
             else
            {
                //
                //  It's a movie, so allow them to change/edit the location info
                //  stuff
                //
                m_pwndAction->c4AddItem
                (
                    facMediaRepoMgr.strMsg(kMRMgrMsgs::midAct_FileBased)
                    , tCIDLib::c4EnumOrd(tCQCMedia::ELocTypes::FileCol)
                );

                m_pwndAction->c4AddItem
                (
                    facMediaRepoMgr.strMsg(kMRMgrMsgs::midAct_ChangerBased)
                    , tCIDLib::c4EnumOrd(tCQCMedia::ELocTypes::Changer)
                );

                //
                //  Select the existing one as the initial value. It'll get the
                //  file based one by default since it's the first item, so we
                //  only need to change it if changer based.
                //
                //  And display the existing value as the default.
                //
                if (m_mcolEdit.eLocType() == tCQCMedia::ELocTypes::Changer)
                    m_pwndAction->SelectByIndex(1);
                else
                    m_pwndAction->SelectByIndex(0);
                m_pwndLocInfo->strWndText(m_mcolEdit.strLocInfo());
            }

            // Change the default button text
            m_pwndContinue->strWndText
            (
                facMediaRepoMgr.strMsg(kMRMgrMsgs::midDlg_EdMeta_Save)
            );

            //
            //  No add/remove items after it's been ripped, and if it's a movie
            //  there are no items, so either way disable these.
            //
            m_pwndItemDel->SetEnable(kCIDLib::False);
            m_pwndItemIns->SetEnable(kCIDLib::False);
        }
         else
        {
            // We are creating a new collection
            if (m_eMType == tCQCMedia::EMediaTypes::Music)
            {
                //
                //  Can only rip if not doing a manual entry. Can be a changer
                //  based disc either way.
                //
                if (!m_bManualMode)
                {
                    m_pwndAction->c4AddItem
                    (
                        facMediaRepoMgr.strMsg(kMRMgrMsgs::midAct_RipUpload)
                        , tCIDLib::c4EnumOrd(tCQCMedia::ELocTypes::FileItem)
                    );
                }

                m_pwndAction->c4AddItem
                (
                    facMediaRepoMgr.strMsg(kMRMgrMsgs::midAct_ChangerBased)
                    , tCIDLib::c4EnumOrd(tCQCMedia::ELocTypes::Changer)
                );

                //
                //  If not in manual mode disable the loc stuff since there's nothing to
                //  do with those in that case.
                //
                if (!m_bManualMode)
                {
                    m_pwndLocInfo->SetEnable(kCIDLib::False);
                    m_pwndBrowse->SetVisibility(kCIDLib::False);
                }

                m_pwndAction->SelectByIndex(0);
            }
             else
            {
                //
                //  It's a movie so it can be either changer or file based.
                //  Assuming file based is the default so put it in first.
                //
                m_pwndAction->c4AddItem
                (
                    facMediaRepoMgr.strMsg(kMRMgrMsgs::midAct_FileBased)
                    , tCIDLib::c4EnumOrd(tCQCMedia::ELocTypes::FileCol)
                );

                m_pwndAction->c4AddItem
                (
                    facMediaRepoMgr.strMsg(kMRMgrMsgs::midAct_ChangerBased)
                    , tCIDLib::c4EnumOrd(tCQCMedia::ELocTypes::Changer)
                );

                m_pwndAction->SelectByIndex(0);
            }
        }
    }

    // Load the common ratings values
    m_pwndRating->c4AddItem(L"None");
    m_pwndRating->c4AddItem(L"G");
    m_pwndRating->c4AddItem(L"PG");
    m_pwndRating->c4AddItem(L"PG-13");
    m_pwndRating->c4AddItem(L"NC-17");
    m_pwndRating->c4AddItem(L"R");
    m_pwndRating->c4AddItem(L"X");
    m_pwndRating->c4AddItem(L"XXX");

    // If no previous media types, then load a common list
    if (MediaRepoMgr_EdMetaDlg::colPrevMediaFmts.bIsEmpty())
    {
        MediaRepoMgr_EdMetaDlg::colPrevMediaFmts.objAdd(TString::strEmpty());
        MediaRepoMgr_EdMetaDlg::colPrevMediaFmts.objAdd(TString(L"BluRay"));
        MediaRepoMgr_EdMetaDlg::colPrevMediaFmts.objAdd(TString(L"CD"));
        MediaRepoMgr_EdMetaDlg::colPrevMediaFmts.objAdd(TString(L"DVD"));
        MediaRepoMgr_EdMetaDlg::colPrevMediaFmts.objAdd(TString(L"HDDVD"));
    }

    //
    //  If we don't have any previous aspect ratios, then be helful and load up
    //  the common ones so that they can just select them.
    //
    if (MediaRepoMgr_EdMetaDlg::colPrevAspects.bIsEmpty())
    {
        MediaRepoMgr_EdMetaDlg::colPrevAspects.objAdd(TString::strEmpty());
        MediaRepoMgr_EdMetaDlg::colPrevAspects.objAdd(TString(L"1.33"));
        MediaRepoMgr_EdMetaDlg::colPrevAspects.objAdd(TString(L"1.66"));
        MediaRepoMgr_EdMetaDlg::colPrevAspects.objAdd(TString(L"1.78"));
        MediaRepoMgr_EdMetaDlg::colPrevAspects.objAdd(TString(L"1.85"));
        MediaRepoMgr_EdMetaDlg::colPrevAspects.objAdd(TString(L"2.00"));
        MediaRepoMgr_EdMetaDlg::colPrevAspects.objAdd(TString(L"2.35"));
    }

    //
    //  Add empty items for some other ones that have previous value selection
    //  so that any incoming ones that have no value won't end up with one of
    //  the previous values.
    //
    MediaRepoMgr_EdMetaDlg::colPrevArtists.objAdd(TString::strEmpty());
    MediaRepoMgr_EdMetaDlg::colPrevLabels.objAdd(TString::strEmpty());
    MediaRepoMgr_EdMetaDlg::colPrevYears.objAdd(TString::strEmpty());

    //
    //  If this collection has been assigned an image already, then download it
    //  and display it.
    //
    if (m_mcolEdit.c2ArtId())
    {
        // <TBD>
    }

    // Load up the incoming data
    LoadFields();

    // Register handlers
    m_pwndBrowse->pnothRegisterHandler(this, &TEdMetaDataDlg::eClickHandler);
    m_pwndAddCategory->pnothRegisterHandler(this, &TEdMetaDataDlg::eClickHandler);
    m_pwndCancel->pnothRegisterHandler(this, &TEdMetaDataDlg::eClickHandler);
    m_pwndContinue->pnothRegisterHandler(this, &TEdMetaDataDlg::eClickHandler);
    m_pwndDelCategory->pnothRegisterHandler(this, &TEdMetaDataDlg::eClickHandler);
    m_pwndEditArt->pnothRegisterHandler(this, &TEdMetaDataDlg::eClickHandler);
    m_pwndEditCast->pnothRegisterHandler(this, &TEdMetaDataDlg::eClickHandler);
    m_pwndEditDescr->pnothRegisterHandler(this, &TEdMetaDataDlg::eClickHandler);
    m_pwndSelTitle->pnothRegisterHandler(this, &TEdMetaDataDlg::eClickHandler);
    m_pwndItemDel->pnothRegisterHandler(this, &TEdMetaDataDlg::eClickHandler);
    m_pwndItemDn->pnothRegisterHandler(this, &TEdMetaDataDlg::eClickHandler);
    m_pwndItemIns->pnothRegisterHandler(this, &TEdMetaDataDlg::eClickHandler);
    m_pwndItemUp->pnothRegisterHandler(this, &TEdMetaDataDlg::eClickHandler);
    m_pwndItems->pnothRegisterHandler(this, &TEdMetaDataDlg::eLBHandler);
    m_pwndMoreArt->pnothRegisterHandler(this, &TEdMetaDataDlg::eClickHandler);

    // Only if it didn't end up disabled
    if (m_pwndAction->bIsEnabled())
        m_pwndAction->pnothRegisterHandler(this, &TEdMetaDataDlg::eComboHandler);

    return bRet;
}



// ---------------------------------------------------------------------------
//  TEdMetaDataDlg: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  This is called to validate the entered values. Mostly we don't care,
//  but for something like a date, we have to insure it's a valid date,
//  and we don't allow some fields to be empty.
//
tCIDLib::TBoolean TEdMetaDataDlg::bValidate()
{
    //
    //  If not doing a play list, then get the action converted to a location
    //  type. Of if in edit mode, just get the existing type.
    //
    tCQCMedia::ELocTypes eLocType;
    if (m_mcolEdit.bIsPlayList())
    {
        eLocType = tCQCMedia::ELocTypes::FileItem;
    }
     else
    {
        //
        //  If the action control is disabled, then this is one that we cannot
        //  change the location type of, so we go back to the original collection
        //  mode. Else, we take what's selected.
        //
        if (!m_pwndAction->bIsEnabled())
            eLocType = m_mcolEdit.eLocType();
        else
            eLocType = tCQCMedia::ELocTypes(m_pwndAction->c4CurItemId());
    }

    const tCIDLib::TCard4   c4ItemCnt = m_pwndItems->c4ItemCount();
    tCIDLib::TCard4         c4Duration = 0;
    tCIDLib::TCard4         c4Year = 0;
    TWindow*                pwndErr = 0;
    TString                 strAspect;
    TString                 strErrText;
    TString                 strTmp;

    if (!m_bTitleSelected)
    {
        // They've not told us a title set to put this guy in yet
        strErrText = facMediaRepoMgr.strMsg(kMRMgrMsgs::midStatus_NoTitleSet);
        pwndErr = m_pwndSelTitle;
    }
     else if ((m_eMType == tCQCMedia::EMediaTypes::Music)
          &&  m_pwndArtist->strWndText().bIsEmpty())
    {
        strErrText = facMediaRepoMgr.strMsg
        (
            kMRMgrMsgs::midStatus_RequiredField
            , facMediaRepoMgr.strMsg(kMRMgrMsgs::midMetaFld_Artist)
        );
        pwndErr = m_pwndArtist;
    }
     else if (m_pwndColName->strWndText().bIsEmpty())
    {
        strErrText = facMediaRepoMgr.strMsg
        (
            kMRMgrMsgs::midStatus_RequiredField
            , facMediaRepoMgr.strMsg(kMRMgrMsgs::midMetaFld_ColName)
        );
        pwndErr = m_pwndColName;
    }
     else if (!m_pwndCategories->c4ItemCount())
    {
        strErrText = facMediaRepoMgr.strMsg(kMRMgrMsgs::midStatus_NoCats);
        pwndErr = m_pwndCategories;
    }
     else if (m_c4KnownItemCnt && (c4ItemCnt > m_c4KnownItemCnt))
    {
        strErrText = facMediaRepoMgr.strMsg
        (
            kMRMgrMsgs::midStatus_BadItemCnt
            , TCardinal(c4ItemCnt)
            , TCardinal(m_c4KnownItemCnt)
        );
        pwndErr = m_pwndItems;
    }
     else if ((eLocType != tCQCMedia::ELocTypes::FileItem)
          &&  m_pwndLocInfo->bIsEmpty())
    {
        strErrText = facMediaRepoMgr.strMsg(kMRMgrMsgs::midStatus_NeedLocInfo);
        pwndErr = m_pwndLocInfo;
    }
     else
    {
        try
        {
            TString strYear;
            if (m_pwndYear->bQueryCurVal(strYear))
                c4Year = strYear.c4Val(tCIDLib::ERadices::Dec);
            else
                c4Year = 0;
        }

        catch(...)
        {
            strErrText = facMediaRepoMgr.strMsg
            (
                kMRMgrMsgs::midStatus_BadField
                , facMediaRepoMgr.strMsg(kMRMgrMsgs::midMetaFld_Year)
            );
            pwndErr = m_pwndYear;
        }

        if (strErrText.bIsEmpty())
        {
            try
            {
                const TString& strDuration = m_pwndDuration->strWndText();
                if (strDuration.bIsEmpty())
                    c4Duration = 0;
                else
                    c4Duration = strDuration.c4Val(tCIDLib::ERadices::Dec);
            }

            catch(...)
            {
                strErrText = facMediaRepoMgr.strMsg
                (
                    kMRMgrMsgs::midStatus_BadField
                    , facMediaRepoMgr.strMsg(kMRMgrMsgs::midMetaFld_Duration)
                );
                pwndErr = m_pwndDuration;
            }
        }

        if (strErrText.bIsEmpty() && m_pwndAspect->bIsEnabled())
        {
            try
            {
                //
                //  We get the text in this case, since they may have typed
                //  a new value in.
                //
                TString strNewAspect;
                if (m_pwndAspect->bQueryCurVal(strNewAspect))
                {
                    tCIDLib::TFloat8 f8Aspect(strNewAspect.f8Val());
                    strAspect.SetFormatted(TFloat(f8Aspect, 2));
                }
                 else
                {
                    strAspect.Clear();
                }
            }

            catch(...)
            {
                strErrText = facMediaRepoMgr.strMsg
                (
                    kMRMgrMsgs::midStatus_BadField
                    , facMediaRepoMgr.strMsg(kMRMgrMsgs::midMetaFld_Aspect)
                );
                pwndErr = m_pwndAspect;
            }
        }

        // Make sure that all of the items have names
        if (strErrText.bIsEmpty())
        {
            TString strVal;
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ItemCnt; c4Index++)
            {
                m_pwndItems->ValueAt(c4Index, strVal);
                if (strVal.bIsEmpty())
                {
                    strErrText = facMediaRepoMgr.strMsg(kMRMgrMsgs::midStatus_NoItemName);
                    pwndErr = m_pwndItems;
                    break;
                }
            }
        }
    }

    // We have an error, so deal with it
    if (!strErrText.bIsEmpty())
    {
        // Show the error to the user
        {
            TErrBox msgbErr(strWndText(), strErrText);
            msgbErr.ShowIt(*this);
        }

        // And put the focus on the field in error
        pwndErr->TakeFocus();
        return kCIDLib::False;
    }

    //
    //  Do some others that are too complex for the generic error check
    //  scheme above. If we have location info, make sure if it's a changer
    //  based scenario that we have a valid moniker.slot format and that
    //  the moniker refers to a valid driver moniker. If it's FileDisc
    //  then see if we can access the directory and warn if not.
    //
    //  If it's file track, then in our current world this means that we
    //  are going to rip and upload and therefore the location info will
    //  be provided automatically.
    //
    if (eLocType == tCQCMedia::ELocTypes::Changer)
    {
        TString strMoniker;
        tCIDLib::TCard4 c4Slot;
        if (!facCQCMedia().bParseChangerLoc(m_pwndLocInfo->strWndText(), strMoniker, c4Slot))
        {
            TErrBox msgbErr(facMediaRepoMgr.strMsg(kMRMgrMsgs::midStatus_BadChangerLoc));
            msgbErr.ShowIt(*this);
            m_pwndLocInfo->TakeFocus();
            return kCIDLib::False;
        }

        // The slot number cannot be zero
        if (!c4Slot)
        {
            TErrBox msgbErr(facMediaRepoMgr.strMsg(kMRMgrMsgs::midStatus_ZeroSlot));
            msgbErr.ShowIt(*this);
            return kCIDLib::False;
        }

        if (!facCQCKit().bDrvIsLoaded(strMoniker))
        {
            // We coudln't find it, so as if they want to continue or not
            TYesNoBox msgbErr(facMediaRepoMgr.strMsg(kMRMgrMsgs::midStatus_BadChangerMoniker));
            if (!msgbErr.bShowIt(*this))
            {
                m_pwndLocInfo->TakeFocus();
                return kCIDLib::False;
            }
        }
    }
     else if (eLocType == tCQCMedia::ELocTypes::FileCol)
    {
        // See if it's a valid path and that we can access it
        try
        {
            TPathStr pathDisc(m_pwndLocInfo->strWndText());
            pathDisc.bRemoveNameExt();

            if (!TFileSys::bIsDirectory(pathDisc))
            {
                TYesNoBox msgbErr
                (
                    facMediaRepoMgr.strMsg(kMRMgrMsgs::midStatus_FileAccessFailed)
                );

                if (msgbErr.bShowIt(*this))
                {
                    m_pwndLocInfo->TakeFocus();
                    return kCIDLib::False;
                }
            }
        }

        catch(...)
        {
            TErrBox msgbErr(facMediaRepoMgr.strMsg(kMRMgrMsgs::midStatus_FileAccessFailed));
            msgbErr.ShowIt(*this);
            m_pwndLocInfo->TakeFocus();
            return kCIDLib::False;
        }
    }

    //
    //  It's ok, so store the values. The track, categories, and image stuff is
    //  updated as they are edited. So we just have to store the other
    //  stuff here.
    //
    m_mcolEdit.strName(m_pwndColName->strWndText());
    m_mcolEdit.c4Year(c4Year);
    m_mcolEdit.c4Duration(c4Duration);
    SavePrevVals(MediaRepoMgr_EdMetaDlg::colPrevYears, m_pwndYear->strWndText());
    m_mcolEdit.SetLocation(eLocType, m_pwndLocInfo->strWndText());

    // Save the aspect ratio and update the previous value list for it
    m_mcolEdit.strAspectRatio(strAspect);
    SavePrevVals(MediaRepoMgr_EdMetaDlg::colPrevAspects, strAspect);

    // Save the media fmt and update the previous value list for it
    m_mcolEdit.strMediaFormat(m_pwndMediaFmt->strWndText());
    SavePrevVals(MediaRepoMgr_EdMetaDlg::colPrevMediaFmts, m_pwndMediaFmt->strWndText());

    // Set the new artist and update the previous artists list.
    m_mcolEdit.strArtist(m_pwndArtist->strWndText());
    SavePrevVals(MediaRepoMgr_EdMetaDlg::colPrevArtists, m_pwndArtist->strWndText());

    // Store the label and update the previous value list for it
    TString strVal;
    if (m_pwndLabel->bQueryCurVal(strVal))
    {
        m_mcolEdit.strLabel(strVal);
        SavePrevVals(MediaRepoMgr_EdMetaDlg::colPrevLabels, strVal);
    }
     else
    {
        m_mcolEdit.strLabel(TString::strEmpty());
    }

    // Store the anamorphic flag
    m_mcolEdit.bAnamorphic(m_pwndAnamorphic->bCheckedState());

    // Store the rating
    if (m_pwndRating->bQueryCurVal(strVal))
        m_mcolEdit.strRating(strVal);
    else
        m_mcolEdit.strRating(L"None");

    // Store the ASIN/UPC fields. We don't pass judgement on them
    m_mcolEdit.strASIN(m_pwndASIN->strWndText());
    m_mcolEdit.strUPC(m_pwndUPC->strWndText());

    // If no categories are selected, that's bad
    if (!m_pwndCategories->c4ItemCount())
    {
        TErrBox msgbErr(facMediaRepoMgr.strMsg(kMRMgrMsgs::midStatus_NoCats));
        msgbErr.ShowIt(*this);
        m_pwndCategories->TakeFocus();
        return kCIDLib::False;
    }

    //
    //  If a new image was selected, then scale the image if required. We want to
    //  keep the image size pretty small and high quality isn't required.
    //
    if (m_bNewImage)
    {
        try
        {
            if ((m_imgiEdit.c4Width() > kMediaRepoMgr::c4MaxH)
            ||  (m_imgiEdit.c4Height() > kMediaRepoMgr::c4MaxV))
            {
                // We need to scale it
                TSize szNew(m_imgiEdit.szImage());

                if ((m_imgiEdit.c4Width() > kMediaRepoMgr::c4MaxH)
                &&  (m_imgiEdit.c4Height() > kMediaRepoMgr::c4MaxV))
                {
                    // It's bigger in both directions, so take the larger one
                    if (m_imgiEdit.c4Width() > m_imgiEdit.c4Height())
                        szNew.ScaleToWidthAR(kMediaRepoMgr::c4MaxH);
                    else
                        szNew.ScaleToHeightAR(kMediaRepoMgr::c4MaxV);
                }
                 else if (m_imgiEdit.c4Width() > kMediaRepoMgr::c4MaxH)
                {
                    szNew.ScaleToWidthAR(kMediaRepoMgr::c4MaxH);
                }
                 else
                {
                    szNew.ScaleToHeightAR(kMediaRepoMgr::c4MaxV);
                }
                m_imgiEdit.Scale(szNew, 4);
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

    //
    //  If any existing items were removed, and this is not a playlist, then we need
    //  to remove the removed items from the database. We remembered the original
    //  item ids on entry, so we just look for ones in that list that are not in the
    //  collection anymore.
    //
    tCIDLib::TCard4 c4Count;
    if (m_c2ColId && !m_mcolEdit.bIsPlayList())
    {
        c4Count = m_fcolOrgItemIds.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const tCIDLib::TCard2 c2CurId = m_fcolOrgItemIds[c4Index];

            tCIDLib::TCard2 c2ItemInd;
            if (!m_mcolEdit.bContainsItem(c2CurId, c2ItemInd))
                m_pmdbEdit->bRemoveObject(m_eMType, tCQCMedia::EDataTypes::Item, c2CurId);
        }
    }

    //
    //  Now we check for any new ones that have been added (they will have zero ids)
    //  and add those to the database. For any that already existed, just update them
    //  in case we modified anything.
    //
    //  We save up the ids of all of them, whether new or added, in the order they
    //  are in the item list. At the end, we set these as the item ids for our
    //  collection. This should get it up to date now.
    //
    c4Count = m_colItems.c4ElemCount();
    tCQCMedia::TIdList fcolItemIds;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TMediaItem& mitemCur = m_colItems[c4Index];

        tCIDLib::TCard2 c2CurId = mitemCur.c2Id();
        if (c2CurId)
            m_pmdbEdit->UpdateItem(mitemCur);
        else
            c2CurId = m_pmdbEdit->c2AddItem(new TMediaItem(mitemCur));

        // Add to the item id list
        fcolItemIds.c4AddElement(c2CurId);
    }

    // And now reset all of the ids for the collection
    m_mcolEdit.SetItemIds(fcolItemIds);

    //
    //  If they loaded a new image, we need to store it for later upload. We just put
    //  it into the database and add the image data to it, which is normally done for
    //  caching on the server side, but we use it to store the image data until the
    //  user commits changes.
    //
    tCIDLib::TCard2 c2ArtId = m_mcolEdit.c2ArtId();
    if (m_bNewImage)
    {
        // Get the image flattened and generate a persistent id for it
        TString strPerId;
        TChunkedBinOutStream strmTar(16 * (1024 * 1024));
        strmTar << m_imgiEdit << kCIDLib::FlushIt;

        // Create a linked stream and use it to create a persistent id
        TChunkedBinInStream strmSrc(strmTar);
        facCQCMedia().CreatePersistentId(strmSrc, strmTar.c4CurSize(), strPerId);

        //
        //  If we don't already have an image, we have to create one. Else copy
        //  the one we have so that we can update it.
        //
        //
        //  WE HAVE TO set the persistent id on both large and small art, so that the
        //  client service will think small art is available and ask for it, which
        //  will cause the repo driver to gen up the small art from the large.
        //
        TMediaImg mimgUpdate;
        if (c2ArtId)
        {
            //
            //  Update the persistent ids of this image
            //
            //  NOTE: that the copy will naturally drop any previously cached art,
            //  since it's not copied, but that's fine since we are going to set it.
            //
            mimgUpdate = *m_pmdbEdit->pmimgById(m_eMType, c2ArtId, kCIDLib::True);

            mimgUpdate.SetPersistentId(strPerId, tCQCMedia::ERArtTypes::LrgCover);
            mimgUpdate.SetPersistentId(strPerId, tCQCMedia::ERArtTypes::SmlCover);

            m_pmdbEdit->UpdateImage(mimgUpdate);
        }
         else
        {
            //
            //  Create a new one and add it to our database. We don't know any of
            //  the paths at this point, so pass empty strings. The server will
            //  assign them after upload.
            //
            TMediaImg* pmimgNew = new TMediaImg
            (
                m_eMType, TString::strEmpty(), TString::strEmpty()
            );
            pmimgNew->strUniqueId(TUniqueId::strMakeId());
            pmimgNew->SetPersistentId(strPerId, tCQCMedia::ERArtTypes::LrgCover);
            pmimgNew->SetPersistentId(strPerId, tCQCMedia::ERArtTypes::SmlCover);

            // And add it to our database and update the collection with the new id
            c2ArtId = m_pmdbEdit->c2AddImage(pmimgNew);
            m_mcolEdit.c2ArtId(c2ArtId);
        }

        //
        //  Update the database with this new art info, so whether an existing image
        //  or a new one, we are now storing the new image info for later upload.
        //
        //  Reset the source stream now, since it will have been read through above
        //  during creation of the persistent id.
        //
        strmSrc.Reset();
        m_pmdbEdit->SetArt
        (
            c2ArtId
            , m_eMType
            , tCQCMedia::ERArtTypes::LrgCover
            , strmSrc
            , strmTar.c4CurSize()
        );
    }

    //
    //  If we are working on an existing collection (id is not zero), then update
    //  the database with the contents of our working copy. Else, add the new
    //  collection and update our working copy with the id, so that if we use it
    //  further below it'll now have its id set.
    //
    if (m_mcolEdit.c2Id())
    {
        m_pmdbEdit->UpdateCollect(m_mcolEdit);
        m_c2ColId = m_mcolEdit.c2Id();
    }
     else
    {
        m_c2ColId = m_pmdbEdit->c2AddCollect(new TMediaCollect(m_mcolEdit));
        m_mcolEdit.c2Id(m_c2ColId);
    }

    //
    //  If the new title set isn't the same as the original one, we have to remove
    //  the collection from the old one, since it's been moved to a new one now.
    //  Note here we use a method that only removes the reference, not the actual
    //  collection.
    //
    if (m_c2SetId && (m_mtsEdit.c2Id() != m_c2SetId))
    {
        const TMediaTitleSet* pmtsOrg = m_pmdbEdit->pmtsById
        (
            m_eMType, m_c2SetId, kCIDLib::True
        );
        m_pmdbEdit->RemoveFromSet(*pmtsOrg, m_c2ColId);
    }

    //
    //  Make sure the set refs our new collection. If not, then we have to have been
    //  creating new stuff. Set the newly figured out collection id from above on our
    //  temp copy and pass that to add it. It only gets info from the object, it does't
    //  keep any reference so this is safe.
    //
    tCIDLib::TCard2 c2ColInd;
    if (!m_mtsEdit.bContainsCol(m_c2ColId, c2ColInd))
        m_mtsEdit.bAddCollect(m_mcolEdit);

    //
    //  And now either add or update the title set info. If updating, then save
    //  the id, since it will have been something we selected after we got in here,
    //  not something that came in from the caller.
    //
    if (m_mtsEdit.c2Id())
    {
        m_pmdbEdit->UpdateTitle(m_mtsEdit);
        m_c2SetId = m_mtsEdit.c2Id();
    }
     else
    {
        m_c2SetId = m_pmdbEdit->c2AddTitle(new TMediaTitleSet(m_mtsEdit));
    }

    //
    //  Let both the collection and title set refinalize to aggregate contained info.
    //  Do the collection first of course.
    //
    m_pmdbEdit->FinalizeObjects
    (
        m_c2ColId, m_eMType, tCQCMedia::EDataTypes::Collect, kCIDLib::False
    );
    m_pmdbEdit->FinalizeObjects
    (
        m_c2SetId, m_eMType, tCQCMedia::EDataTypes::TitleSet, kCIDLib::False
    );
    return kCIDLib::True;
}


//
//  This is called when the user asks to browse for the location of a piece of
//  media data. This will only be called for collection level media, i.e. movies.
//  Music is located per item, so that will be done in the per-item settings
//  dialog.
//
tCIDLib::TVoid TEdMetaDataDlg::BrowseMedia()
{
    //
    //  We do something different depending on the location type of the current
    //  collection. So look at the currently selection action.
    //
    const tCQCMedia::ELocTypes eLocType = tCQCMedia::ELocTypes(m_pwndAction->c4CurItemId());
    if (eLocType == tCQCMedia::ELocTypes::FileCol)
    {
        //
        //  Just run the local file browser dialog. If there is anything in the
        //  location info window, use that as the initial starting point. Else,
        //  use the location info already in the collection.
        //
        TPathStr pathInit;
        TPathStr pathNew;

        const TString& strCurVal = m_pwndLocInfo->strWndText();
        if (!strCurVal.bIsEmpty() && TFileSys::bExists(strCurVal))
        {
            pathInit = strCurVal;
            pathInit.bExtractNameExt(pathNew);
        }
         else if (!m_mcolEdit.strLocInfo().bIsEmpty()
              &&  TFileSys::bExists(m_mcolEdit.strLocInfo()))
        {
            pathInit = m_mcolEdit.strLocInfo();
            pathInit.bExtractNameExt(pathNew);
        }

        TWndPtrJanitor janPtr(tCIDCtrls::ESysPtrs::Wait);
        tCIDLib::TKVPList colTypes(1);
        colTypes.objAdd(TKeyValuePair(L"Media File", kCIDLib::pszAllFilesSpec));
        tCIDLib::TStrList colSel;
        const tCIDLib::TBoolean bRes = facCIDCtrls().bOpenFileDlg
        (
            *this
            , L"Select a Location"
            , pathInit
            , colSel
            , colTypes
            , tCIDCtrls::EFOpenOpts::FileMustExist
        );

        if (bRes)
        {
            pathNew = colSel[0];
            if (!pathNew.bStartsWith(L"\\\\"))
            {
                TOkBox msgbWarn
                (
                    L"Generally you should set media file locations relative to "
                    L"a shared directory."
                );
                msgbWarn.ShowIt(*this);
            }

            // Update the entry field
            m_pwndLocInfo->strWndText(pathNew);
        }
    }
     else if (eLocType == tCQCMedia::ELocTypes::Changer)
    {
        //
        //  Get the text from the loc info field. It will either be a changer
        //  name, or a changer.slot form, or maybe it won't actually have a
        //  slot, just the period. We want to get the changer driver name.
        //
        tCIDLib::TCard4 c4Ofs;
        TString strChanger = m_pwndLocInfo->strWndText();
        if (strChanger.bIsEmpty())
        {
            TOkBox msgbLoc(facMediaRepoMgr.strMsg(kMRMgrMsgs::midPrompt_EnterChangerName));
            msgbLoc.ShowIt(*this);
            return;
        }

        // If it has a period or period/slot on it, cut that off
        if (strChanger.bFirstOccurrence(kCIDLib::chPeriod, c4Ofs))
            strChanger.CapAt(c4Ofs);

        //
        //  Get a map of slots from the changer. We get a bitset back
        //  with bits on for the taken slots. We also have to pass in the
        //  max slot number, so we ask the changer for that info.
        //
        tCIDLib::TCard4     c4SlotCnt;
        tCIDLib::TCard4     c4TakenCnt;
        TBitset             btsSlots;
        try
        {
            // Get a CQC Server admin proxy for the changer driver
            TWndPtrJanitor janPtr(tCIDCtrls::ESysPtrs::Wait);
            tCQCKit::TCQCSrvProxy orbcChDrv = facCQCKit().orbcCQCSrvAdminProxy(strChanger);

            // And read the slot count field from the driver
            tCIDLib::TCard4 c4SerNum = 0;
            orbcChDrv->bReadCardByName(c4SerNum, strChanger, L"SlotCnt", c4SlotCnt);

            //
            //  And query the available slots from the database for this changer.
            //  Pass zero for the optional collection name list, since we don't
            //  need those.
            //
            c4TakenCnt = m_pmdbEdit->c4QueryAvailSlots(strChanger, btsSlots, c4SlotCnt, 0);
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);

            TExceptDlg dlgErr
            (
                *this
                , strWndText()
                , L"An error occured while querying the slot information"
                , errToCatch
            );
            return;
        }

        //
        //  We have the list, so build up a collection of strings with
        //  the non-taken slot numbers. We'll use the generic list
        //  selection dailog to get the selection.
        //
        tCIDLib::TStrList colSlots(c4SlotCnt - c4TakenCnt);
        TString strTmpFmt(strChanger);
        strTmpFmt.Append(kCIDLib::chPeriod);
        tCIDLib::TCard4 c4Index = 0;
        for (; c4Index < c4SlotCnt; c4Index++)
        {
            if (!btsSlots.bBitState(c4Index))
            {
                strTmpFmt.CapAt(strChanger.c4Length() + 1);
                strTmpFmt.AppendFormatted(c4Index + 1);
                colSlots.objAdd(strTmpFmt);
            }
        }

        const TString strTitle(kMRMgrMsgs::midSelSlot_Title, facMediaRepoMgr);
        TString strSel;
        const tCIDLib::TBoolean bSel = facCIDWUtils().bListSelect
        (
            *this, strTitle, colSlots, strSel
        );
        if (bSel)
            m_pwndLocInfo->strWndText(strSel);
    }
}


// Handles button clicks for this dialog box
tCIDCtrls::EEvResponses TEdMetaDataDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kMediaRepoMgr::ridDlg_EdMeta_AddCat)
    {
        ShowCategoryList();
    }
     else if (wnotEvent.widSource() == kMediaRepoMgr::ridDlg_EdMeta_DelCat)
    {
        //
        //  If we only have one category left, tell them we can't delete it. If
        //  we have none, then do nothing. Else, delete the category.
        //
        if (m_pwndCategories->c4ItemCount() == 1)
        {
            TOkBox msgbLast(facMediaRepoMgr.strMsg(kMRMgrMsgs::midStatus_LastCat));
            msgbLast.ShowIt(*this);
        }
         else if (m_pwndCategories->c4ItemCount() > 1)
        {
            const tCIDLib::TCard2 c2Id = tCIDLib::TCard2
            (
                m_pwndCategories->c4CurItemId()
            );

            // If it's a special category, then don't allow it
            if (facCQCMedia().bIsSpecialCat(c2Id, m_eMType))
            {
                TOkBox msgbSpecial
                (
                    facMediaRepoMgr.strMsg(kMRMgrMsgs::midStatus_SpecialCategory)
                );
                msgbSpecial.ShowIt(*this);
            }
             else
            {
                TString strName;
                m_pwndCategories->ValueAt(m_pwndCategories->c4CurItem(), strName);
                TYesNoBox msgbDelQ(facMediaRepoMgr.strMsg(kMRMgrMsgs::midQ_DelCat, strName));

                if (msgbDelQ.bShowIt(*this))
                {
                    // Remove it from the collection and our list
                    m_mcolEdit.RemoveCategory(c2Id);
                    m_pwndCategories->RemoveCurrent();
                }
            }
        }
    }
     else if (wnotEvent.widSource() == kMediaRepoMgr::ridDlg_EdMeta_Cancel)
    {
        EndDlg(kMediaRepoMgr::ridDlg_EdMeta_Cancel);
    }
     else if (wnotEvent.widSource() == kMediaRepoMgr::ridDlg_EdMeta_Continue)
    {
        // Validate the values. If accepted, then we are done
        if (bValidate())
            EndDlg(kMediaRepoMgr::ridDlg_EdMeta_Continue);
    }
     else if (wnotEvent.widSource() == kMediaRepoMgr::ridDlg_EdMeta_EditArt)
    {
        // Let them browse locally for images
        LoadImage();
    }
     else if (wnotEvent.widSource() == kMediaRepoMgr::ridDlg_EdMeta_EditCast)
    {
        TString     strCast = m_mcolEdit.strCast();
        TEdCastDlg  dlgEdCast;
        if (dlgEdCast.bRun(*this, strCast))
            m_mcolEdit.strCast(strCast);
    }
     else if (wnotEvent.widSource() == kMediaRepoMgr::ridDlg_EdMeta_EditDescr)
    {
        TMediaRepoEdDescrDlg dlgEdit;
        TString strEdit = m_mcolEdit.strDescr();
        if (dlgEdit.bRun(*this, strEdit))
            m_mcolEdit.strDescr(strEdit);
    }
     else if (wnotEvent.widSource() == kMediaRepoMgr::ridDlg_EdMeta_SelTitle)
    {
        //
        //  Update the collection with the name currently in the collection
        //  name field, so that that will be the default.
        //
        m_mcolEdit.strName(m_pwndColName->strWndText());

        //
        //  If there's an existing title set, then make that the ingoing data
        //  so it can be selected initially. Else we start wtih an empty one.
        //
        TSelTitleSetDlg dlgSelTitle;
        TMediaTitleSet mtsNew = m_mtsEdit;
        const tCIDLib::TBoolean bRes = dlgSelTitle.bRunDlg
        (
            *this, mtsNew, m_mcolEdit, *m_pmdbEdit
        );

        if (bRes)
        {
            // Remember we've selected one now, and copy the data back
            m_bTitleSelected = kCIDLib::True;
            m_mtsEdit = mtsNew;
            m_pwndSetName->strWndText(m_mtsEdit.strName());
        }
    }
     else if (wnotEvent.widSource() == kMediaRepoMgr::ridDlg_EdMeta_ItemDel)
    {
        // If there's a selected ited, then ask if they want to delete it
        const tCIDLib::TCard4 c4Pos = m_pwndItems->c4CurItem();
        if (c4Pos != kCIDLib::c4MaxCard)
        {
            TYesNoBox msgbDel(facMediaRepoMgr.strMsg(kMRMgrMsgs::midQ_DeleteItem));

            const tCIDLib::TCard2 c2Id = tCIDLib::TCard2(m_pwndItems->c4IndexToId(c4Pos));
            if (msgbDel.bShowIt(*this))
            {

                //
                //  Tell the collection to remove this item from its list. DO THIS
                //  FIRST else we'll invalidate the stored id if we delete the
                //  window item first.
                //
                m_mcolEdit.RemoveItem(c2Id);

                // And now we can kill the list window item and the list item
                m_colItems.RemoveAt(c4Pos);
                m_pwndItems->RemoveAt(c4Pos);
            }
        }
    }
     else if (wnotEvent.widSource() == kMediaRepoMgr::ridDlg_EdMeta_ItemDn)
    {
        //
        //  If not already at the end, then move it down, both in the list
        //  box and in the collection.
        //
        const tCIDLib::TCard4 c4Pos = m_pwndItems->c4CurItem();
        const tCIDLib::TCard4 c4Count = m_pwndItems->c4ItemCount();
        if ((c4Count > 1) && (c4Pos < c4Count - 2))
        {
            if (m_pwndItems->bMoveDn(kCIDLib::True))
                m_mcolEdit.SwapItems(c4Pos, c4Pos + 1);
        }
    }
     else if (wnotEvent.widSource() == kMediaRepoMgr::ridDlg_EdMeta_ItemIns)
    {
        InsertItem();
    }
     else if (wnotEvent.widSource() == kMediaRepoMgr::ridDlg_EdMeta_ItemUp)
    {
        //
        //  If not already on the first item (or empty), then move the
        //  selected up on, in the list and the collection.
        //
        const tCIDLib::TCard4 c4Pos = m_pwndItems->c4CurItem();
        if (c4Pos && (c4Pos != kCIDLib::c4MaxCard))
        {
            if (m_pwndItems->bMoveUp(kCIDLib::True))
                m_mcolEdit.SwapItems(c4Pos, c4Pos - 1);
        }
    }
     else if (wnotEvent.widSource() == kMediaRepoMgr::ridDlg_EdMeta_MoreArt)
    {
        // We'll go get some more image options for them to pick from
        const TString& strSetName = m_pwndSetName->strWndText();
        const TString& strArtistName = m_pwndArtist->strWndText();
        if (strSetName.bIsEmpty() || strArtistName.bIsEmpty())
        {
            // We have to have a name to use
            TOkBox msgbNoName(facMediaRepoMgr.strMsg(kMRMgrMsgs::midPrompt_MoreArtReq));
            msgbNoName.ShowIt(*this);
        }
         else
        {
            GetMoreImgOpts
            (
                strSetName, m_pwndColName->strWndText(), strArtistName, m_mcolEdit.eType()
            );
        }
    }
     else if (wnotEvent.widSource() == kMediaRepoMgr::ridDlg_EdMeta_Browse)
    {
        // Allow them to select a change/slot or a file location
        BrowseMedia();
    }
    return tCIDCtrls::EEvResponses::Handled;
}


// Handle combo box events for some of the combos
tCIDCtrls::EEvResponses TEdMetaDataDlg::eComboHandler(TListChangeInfo& wnotEvent)
{
    if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::SelChanged)
    {
        // We enable/hide some loc info fields based on the action
        if (wnotEvent.widSource() == kMediaRepoMgr::ridDlg_EdMeta_Action)
        {
            tCQCMedia::ELocTypes eLocType = tCQCMedia::ELocTypes
            (
                m_pwndAction->c4CurItemId()
            );

            if (eLocType == tCQCMedia::ELocTypes::FileItem)
            {
                // Disable the location stuff in this case
                m_pwndLocInfo->ClearText();
                m_pwndLocInfo->SetEnable(kCIDLib::False);
                m_pwndBrowse->SetVisibility(kCIDLib::False);
            }
             else if (eLocType == tCQCMedia::ELocTypes::FileCol)
            {
                m_pwndLocInfo->SetEnable(kCIDLib::True);
                m_pwndBrowse->SetVisibility(kCIDLib::True);

                // If we changed to file type from something else, clear the info
                if (m_mcolEdit.eLocType() != tCQCMedia::ELocTypes::FileCol)
                    m_pwndLocInfo->ClearText();
            }
             else if (eLocType == tCQCMedia::ELocTypes::Changer)
            {
                m_pwndLocInfo->SetEnable(kCIDLib::True);
                m_pwndBrowse->SetVisibility(kCIDLib::True);
            }
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}


// Handle Listbox events
tCIDCtrls::EEvResponses TEdMetaDataDlg::eLBHandler(TListChangeInfo& wnotEvent)
{
    if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::Invoked)
    {
        if (wnotEvent.widSource() == kMediaRepoMgr::ridDlg_EdMeta_ItemList)
        {
            // They double clicked on an item, so let them edit it
            const tCIDLib::TCard4 c4Index = wnotEvent.c4Index();

            // Get a copy of the content to edit
            TMediaItem mitemEdit = m_colItems[c4Index];

            TEdMItemDlg dlgEdit;
            if (dlgEdit.bRun(*this, m_mcolEdit, mitemEdit))
            {
                //
                //  Copy back the data and update the list window item in case the
                //  name was changed.
                //
                m_colItems[c4Index] = mitemEdit;
                m_pwndItems->c4SetText(c4Index, mitemEdit.strName());
            }
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}


//
//  This is used to get more cover art image options for them to pick from.
//  The metadata retrieval dialog just gets one, and it might be the wrong
//  one. This guy will go to Google and get the first few images, and then
//  to a cover art lookup site and get one from there. Then we'll display
//  those choices and let the user pick one.
//
tCIDLib::TVoid
TEdMetaDataDlg::GetMoreImgOpts( const   TString&                strTitleSetName
                                , const TString&                strColName
                                , const TString&                strArtistName
                                , const tCQCMedia::EMediaTypes  eMType)
{
    // We'll fill in a list of image objects
    TRefVector<TCIDImage> colImgs(tCIDLib::EAdoptOpts::Adopt, 5);

    const tCIDLib::TBoolean bIsMusic(eMType == tCQCMedia::EMediaTypes::Music);

    // We will just call the various scraping methods on the facility class
    TJPEGImage imgNew;
    if (facMediaRepoMgr.bScrapeAAOData(strColName, imgNew))
        colImgs.Add(new TJPEGImage(imgNew));

    if (facMediaRepoMgr.bScrapeDCSData(bIsMusic, strArtistName, strColName, imgNew))
        colImgs.Add(new TJPEGImage(imgNew));

    //
    //  If we got no images, then tell the user we failed. Else, let htem
    //  select if they want.
    //
    if (colImgs.bIsEmpty())
    {
        TOkBox msgbNoArt(facMediaRepoMgr.strMsg(kMRMgrMsgs::midStatus_NoArtFound));
        msgbNoArt.ShowIt(*this);
    }
     else
    {
        const TString strTitle(kMRMgrMsgs::midSelArt_Title, facMediaRepoMgr);
        tCIDLib::TCard4 c4Selected;
        if (facCIDWUtils().bSelectImage(*this, colImgs, strTitle, c4Selected))
        {
            m_imgiEdit = *static_cast<TJPEGImage*>(colImgs[c4Selected]);
            TGraphWndDev gdevCompat(*this);
            m_pwndArt->SetImage(gdevCompat, m_imgiEdit);
            m_bNewImage = kCIDLib::True;
        }
    }
}



//
//  Called when the user presses the track insert button.
//
tCIDLib::TVoid TEdMetaDataDlg::InsertItem()
{
    tCIDLib::TCard4 c4At = m_pwndItems->c4CurItem();
    if (c4At == kCIDLib::c4MaxCard)
        c4At = 0;

    try
    {
        TMediaItem mitemNew
        (
            L"New Item"
            , TUniqueId::strMakeId()
            , TString::strEmpty()
            , m_mcolEdit.eType()
        );

        TEdMItemDlg dlgEdit;
        if (dlgEdit.bRun(*this, m_mcolEdit, mitemNew))
        {
            //
            //  Add this guy to our list of items and to our list window, both
            //  at the index where they have selected. Leave the id on this one
            //  at zero since it's a new one.
            //
            m_colItems.InsertAt(mitemNew, c4At);
            m_pwndItems->InsertItem(mitemNew.strName(), 0, c4At);
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
            , L"An error occured while processing the new item"
            , errToCatch
        );
    }
}


//
//  This is called on entry to load up our child controls with the data
//  from the currently selected title set from the match list.
//
tCIDLib::TVoid TEdMetaDataDlg::LoadFields()
{
    TString strFmt;

    // Set the incoming title set name if any
    m_pwndSetName->strWndText(m_mtsEdit.strName());

    // The cast is only used for movies
    if (m_eMType == tCQCMedia::EMediaTypes::Music)
        m_pwndEditCast->SetEnable(kCIDLib::False);
    else
        m_pwndEditCast->SetEnable(kCIDLib::True);

    // Load the art up if any
    TGraphWndDev gdevCompat(*this);
    m_pwndArt->SetImage(gdevCompat, m_imgiEdit);

    //
    //  Load the categories that the collection is part of. If by some chance we
    //  don't find the category, remove it from the collection. We want to load it
    //  sorted, so get a non-adopting list first, sort it, then load the list window.
	//
	//	Add one to the max in case the current count is zero. zero isn't a valid
	//	maximum size.
    //
    tCIDLib::TCard4 c4Count = m_mcolEdit.c4CatCount();
    tCQCMedia::TCatList colCats(tCIDLib::EAdoptOpts::NoAdopt, c4Count);
    tCIDLib::TCard4 c4Index = 0;
    while (c4Index < c4Count)
    {
        const tCIDLib::TCard2 c2CurCatId = m_mcolEdit.c2CatIdAt(c4Index);

        // Else look it up and see if we want to show it or throw it
        const TMediaCat* pmcatCur = m_pmdbEdit->pmcatById(m_eMType, c2CurCatId, kCIDLib::False);
        if (pmcatCur)
        {
            c4Index++;
            colCats.Add(pmcatCur);
        }
         else
        {
            m_mcolEdit.RemoveCategory(c2CurCatId);
            c4Count--;
        }
    }

    c4Count = colCats.c4ElemCount();
    for (c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TMediaCat* pmcatCur = colCats[c4Index];
        m_pwndCategories->c4AddItem(pmcatCur->strName(), pmcatCur->c2Id());
    }
    if (c4Count)
        m_pwndCategories->SelectByIndex(0);

    // The rest is disc specific
    m_pwndColName->strWndText(m_mcolEdit.strName());

    // Load the collection duration
    strFmt.SetFormatted(m_mcolEdit.c4Duration());
    m_pwndDuration->strWndText(strFmt);

    // Load the year via the previous value list loader
    strFmt.SetFormatted(m_mcolEdit.c4Year());
    LoadPrevList(MediaRepoMgr_EdMetaDlg::colPrevYears, *m_pwndYear, strFmt);

    // Load the anamorphic flag
    m_pwndAnamorphic->SetCheckedState(m_mcolEdit.bAnamorphic());

    // Load the ASIN/UPC fields
    m_pwndASIN->strWndText(m_mcolEdit.strASIN());
    m_pwndUPC->strWndText(m_mcolEdit.strUPC());

    // Load the artist via the previous value list loader
    LoadPrevList
    (
        MediaRepoMgr_EdMetaDlg::colPrevArtists, *m_pwndArtist, m_mcolEdit.strArtist()
    );

    // Load the label/studio via the previous value list loader
    LoadPrevList
    (
        MediaRepoMgr_EdMetaDlg::colPrevLabels, *m_pwndLabel, m_mcolEdit.strLabel()
    );

    // Load the media fmt via the previous value list loader
    LoadPrevList
    (
        MediaRepoMgr_EdMetaDlg::colPrevMediaFmts, *m_pwndMediaFmt, m_mcolEdit.strMediaFormat()
    );

    // Some of the fields are data type specific.
    m_pwndItems->RemoveAll();
    if (m_eMType == tCQCMedia::EMediaTypes::Music)
    {
        m_pwndAnamorphic->SetEnable(kCIDLib::False);
        pwndChildById(kMediaRepoMgr::ridDlg_EdMeta_AspectPref)->SetEnable(kCIDLib::False);
        pwndChildById(kMediaRepoMgr::ridDlg_EdMeta_AspectPref2)->SetEnable(kCIDLib::False);
        m_pwndAspect->SetEnable(kCIDLib::False);
        pwndChildById(kMediaRepoMgr::ridDlg_EdMeta_RatingPref)->SetEnable(kCIDLib::False);
        m_pwndRating->SetEnable(kCIDLib::False);
        m_pwndItems->SetEnable(kCIDLib::True);

        // Load up the items we got
        c4Count = m_colItems.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const TMediaItem& mitemCur = m_colItems[c4Index];
            m_pwndItems->c4AddItem(mitemCur.strName(), mitemCur.c2Id());
        }
        if (c4Count)
            m_pwndItems->SelectByIndex(0);
    }
     else
    {
        // We don't allow editing of the item list for movies
        m_pwndItemIns->SetEnable(kCIDLib::False);
        m_pwndItemDel->SetEnable(kCIDLib::False);
        m_pwndItemDn->SetEnable(kCIDLib::False);
        m_pwndItemUp->SetEnable(kCIDLib::False);
        m_pwndItems->SetEnable(kCIDLib::False);

        // Load the aspect ratio and rating
        tCIDLib::TCard4 c4At;
        m_pwndRating->bSelectByText(m_mcolEdit.strRating(), c4At);
        LoadPrevList
        (
            MediaRepoMgr_EdMetaDlg::colPrevAspects
            , *m_pwndAspect
            , m_mcolEdit.strAspectRatio()
        );
    }
}


//
//  Called if the user clicks on the art selection button. We allow the user
//  to browse the local disk for image files, and we load up the contents into
//  caller's image object.
//
tCIDLib::TVoid TEdMetaDataDlg::LoadImage()
{
    // Let the user select a local image
    TPathStr pathSrcFile;
    {
        tCIDLib::TKVPList colFileTypes(1);
        colFileTypes.objAdd(TKeyValuePair(L"Image Files", L"*.bmp;*.png;*.jpeg;*.jpg"));

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
        // Ask the image factory to load the image, converting to JPEG if needed
        facCIDImgFact().bLoadToJPEG(pathSrcFile, m_imgiEdit, kCIDLib::True);
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
        return;
    }

    // Update our previe window with the new stuff
    TGraphWndDev gdevCompat(*this);
    m_pwndArt->SetImage(gdevCompat, m_imgiEdit);

    // Indicate that a new image was selected, and so will have to be uploaded
    m_bNewImage = kCIDLib::True;
}


//
//  This is called for a field that we maintain a list of previously entered
//  values for. If the new value is in the list, then we select it. Else,
//  we force the new value into the entry field of the combo box.
//
tCIDLib::TVoid
TEdMetaDataDlg::LoadPrevList(const  tCIDLib::TStrList&  colToLoad
                            ,       TComboBox&          wndTar
                            , const TString&            strNewVal)
{
    wndTar.RemoveAll();
    const tCIDLib::TCard4 c4Count = colToLoad.c4ElemCount();
    if (c4Count)
    {
        tCIDLib::TCard4 c4SelInd = kCIDLib::c4MaxCard;
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            wndTar.c4AddItem(colToLoad[c4Index]);
            if (colToLoad[c4Index] == strNewVal)
                c4SelInd = c4Index;
        }

        if (c4SelInd == kCIDLib::c4MaxCard)
        {
            // Force the value into the entry field, but not into the list
            wndTar.strWndText(strNewVal);
        }
         else
        {
            wndTar.SelectByIndex(c4SelInd);
        }
    }
}


//
//  Called at the end to store the committed values to the previous value lists.
//
tCIDLib::TVoid
TEdMetaDataDlg::SavePrevVals(       tCIDLib::TStrList&  colTar
                            , const TString&            strNewVal)
{
    // If the new value is empty, do nothing
    if (strNewVal.bIsEmpty())
        return;

    tCIDLib::TCard4 c4Count = colTar.c4ElemCount();
    tCIDLib::TCard4 c4Index = 0;

    for (; c4Index < c4Count; c4Index++)
    {
        if (colTar[c4Index] == strNewVal)
        {
            // Move it to the top if not already
            if (c4Index)
            {
                colTar.RemoveAt(c4Index);
                colTar.InsertAt(strNewVal, 0);
            }
            break;
        }
    }

    // If we didn't find it, then insert it at the start
    if (c4Index == c4Count)
    {
        colTar.InsertAt(strNewVal, 0);
        c4Count++;
    }

    // If that tooks us over the limit, delete the last one
    if (c4Count > MediaRepoMgr_EdMetaDlg::c4MaxPrevItems)
        colTar.RemoveAt(MediaRepoMgr_EdMetaDlg::c4MaxPrevItems);
}


//
//  Loads a popup menu window with the available categories and presents it
//  to the user. If they select one, we add it to the selected title set's list of
//  categories.
//
tCIDLib::TVoid TEdMetaDataDlg::ShowCategoryList()
{
    // Clear the menu window out and load up the new items
    TPopupMenu menuCats(L"Category Menu");
    menuCats.Create();

    try
    {
        tCIDLib::TCard4 c4Count = m_pmdbEdit->c4CatCnt(m_eMType);
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const TMediaCat& mcatCur = m_pmdbEdit->mcatAt(m_eMType, c4Index);
            menuCats.AddActionItem(mcatCur.strName(), mcatCur.strName(), c4Index + 1);
        }

        //
        //  We want to pop it up at the top right of the invoking button. We have
        //  to translate to window coordinates since this is not a child window.
        //
        TPoint pntOrg;
        ToScreenCoordinates(m_pwndAddCategory->areaWnd().pntUR(), pntOrg);

        //
        //  Process the list modally. Tell him he can change the origin of the popup
        //  if it's too big to fit fully at the initial vertical position.
        //
        const tCIDLib::TCard4 c4Sel = menuCats.c4Process(*this, pntOrg);

        //
        //  If they made a selection, then add this category to the list if not
        //  already.
        //
        if (c4Sel)
        {
            const TMediaCat& mcatSel = m_pmdbEdit->mcatAt(m_eMType, c4Sel - 1);

            // If the collection says this is a new category, add it to the list window
            if (m_mcolEdit.bAddCategory(mcatSel.c2Id()))
            {
                m_pwndCategories->c4AddItem(mcatSel.strName(), mcatSel.c2Id());
            }
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
            , L"An error occured while processing the category list"
            , errToCatch
        );
    }
}


