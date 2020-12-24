//
// FILE NAME: MediaRepoMgr_EdMetaDlg.hpp
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
//  This is the header for MediaRepoMgr_EdMetaDlg.cpp, which implements a dialog
//  that allows the user to edit the metadata for a collection.
//
//  This is ultimately far and away the most complex bit in the repo manager since
//  most editing is done here. And because it's used in three different modes,
//  each of which has a different bRun() entry point.
//
//  1.  To edit an existing collection, which has to have an existing title set since
//      it is in the database, and any items it references must exist of course. So
//      in that case get the ids of the set/col, get copies of the set, collection,
//      and item and edit that stuff.
//
//  2.  To manually create a new playlist or collection. In this case, we don't get
//      any new info except whether it's a playlist or collection and the media type.
//      We just start with empty set, collection, and item list data and let the user
//      edit that.
//
//  3.  When a CD or DVD has been added with metadata retrieval. In this case we get
//      a collection and items already filled out (to the extent that the data was
//      available) but not in the database yet. So we start with that and edit it.
//      There will be no title set assigned generally here because the metadata
//      doesn't allow us to determine that.
//
//  We have an issue with newly chosen art. The art must later be uploaded as well
//  as the metadata changes, but we don't want to upload it until we know the
//  metadata is going to be saved. So we just store it in the database using the
//  same storage that the server side driver does to cache image data. When the
//  user commits (back in the main frame), any image objects that have image data
//  on them need to be uploaded. They will already be in our local DB and have the
//  correct ids and all that.
//
//  The user selects a title set for the collection here as well. This can be either
//  a new one (typical when they rip a new CD add a new movie), or they can select
//  from among existing ones. So we have to be able to deal with storing a newly
//  created one until it's time to commit and then add it to the database and get
//  the collection added to it.
//
//
//  We never make any changes to the database until the user commits. We only edit
//  our copies of the data. This does make for a bit of extra complexity but it's
//  a reuirement anyway in scenario #3 where we get pre-filled info that is not in
//  the database yet and may never be committed. And this lets us easily back out
//  of any changes.
//
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TEdMetaDataDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------
class TEdMetaDataDlg : public TDlgBox
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TEdMetaDataDlg();

        TEdMetaDataDlg(const TEdMetaDataDlg&) = delete;

        ~TEdMetaDataDlg();


        // -------------------------------------------------------------------
        //  Unimplemented constructors and operators
        // -------------------------------------------------------------------
        TEdMetaDataDlg& operator=(const TEdMetaDataDlg&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bRunDlg
        (
            const   TWindow&                wndOwner
            , const tCIDLib::TCard2         c2TitleId
            , const tCIDLib::TCard2         c2ColId
            , const tCQCMedia::EMediaTypes  eMType
            , const TJPEGImage&             imgiArt
            ,       TMediaDB&               mdbEdit
        );

        tCIDLib::TBoolean bRunDlg
        (
            const   TWindow&                wndOwner
            , const tCQCMedia::EMediaTypes  eMType
            , const tCIDLib::TBoolean       bIsPlayList
            ,       TMediaDB&               mdbEdit
            ,       tCIDLib::TCard2&        c2ColId
            ,       tCIDLib::TCard2&        c2SetId
        );

        tCIDLib::TBoolean bRunDlg
        (
            const   TWindow&                wndOwner
            , const TMediaCollect&          mcolNew
            , const tCQCMedia::TItemList&   colItems
            , const TJPEGImage&             imgiArt
            , const tCIDLib::TCard4         c4DiscItemCnt
            , const tCIDLib::TBoolean       bManual
            ,       TMediaDB&               mdbEdit
            ,       tCIDLib::TCard2&        c2ParId
            ,       tCIDLib::TCard2&        c2ColId
            ,       tCIDLib::TCard2&        c2ArtId
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCancelRequest() override;

        tCIDLib::TBoolean bCreated() override;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bValidate();

        tCIDLib::TVoid BrowseMedia();

        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );

        tCIDCtrls::EEvResponses eComboHandler
        (
                    TListChangeInfo&        wnotEvent
        );

        tCIDCtrls::EEvResponses eLBHandler
        (
                    TListChangeInfo&        wnotEvent
        );

        tCIDLib::TVoid GetMoreImgOpts
        (
            const   TString&                strTitleName
            , const TString&                strColName
            , const TString&                strArtistName
            , const tCQCMedia::EMediaTypes  eMType
        );

        tCIDLib::TVoid InsertItem();

        tCIDLib::TVoid LoadImage();

        tCIDLib::TVoid LoadPrevList
        (
            const   tCIDLib::TStrList&      colToLoad
            ,       TComboBox&              wndTar
            , const TString&                strCurVal
        );

        tCIDLib::TVoid LoadFields();

        tCIDLib::TVoid SavePrevVals
        (
                    tCIDLib::TStrList&      colTar
            , const TString&                strNewVal
        );

        tCIDLib::TVoid ShowCategoryList();

        tCIDLib::TVoid UpdateCategoryList();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bEditMode
        //      We can run in edit mode or rip mode. In edit mode the process
        //      button says Save, while in rip mode it says Save if a movie is
        //      being processed, else RipCD if a CD is being processed.
        //
        //  m_bManualMode
        //      We let them manually enter collections, and this flag is set if
        //      it is a manual one. For now, we need to restrict the location
        //      types for a manual one.
        //
        //  m_bNewImage
        //      Remember if we get a new image that will need to be uploaded
        //      if they commit.
        //
        //  m_bTitleSelected
        //      We have to have a separate flag to make sure we have selected a title,
        //      since they could have selected a new one that isn't in the database
        //      until we commit.
        //
        //  m_c2ColId
        //  m_c2SetId
        //      On entry we store any incoming collection and set id. If none, they
        //      will be zero, indicating we are creating new stuff. If the collection
        //      id is non-zero the title set has to be, sine it couldn have gotten
        //      into the database without having been assigned to one. If we create
        //      new stuff, these are updated with the new ids for return to the
        //      caller.
        //
        //  m_c4KnownItemCnt
        //      If the incoming collection has a fixed number of items, which is when
        //      a CD is being ripped. This is used to insure that we get all of the
        //      item metadata we need.
        //
        //  m_colItems
        //      A copy of the items to edit. We can modify these all we want and
        //      commit them at the end.
        //
        //  m_eMType
        //      We have to access this a lot, so we pull it out from the collection
        //      on entry and keep it around.
        //
        //  m_fcolOrgItemIds
        //      On entry, if we have a pre-existing collection that we are editing,
        //      then remember the original item ids, so we can check at the end
        //      whether any have been removed. On return, if we are creating a new
        //      collection, we give back the ids.
        //
        //  m_mcolEdit
        //  m_mtsEdit
        //      We save the original collection and title set on entry so that we can
        //      edit it until we are ready to commit. If this is new stuff, then the
        //      title set will be empty (and have a zero id) which tells use we have
        //      to select one before the user can commit.
        //
        //  m_pmdbEdit
        //      A pointer to the database we are editing.
        //
        //  m_c4KnownItemCnt
        //      If the caller is letting us edit metadata for a CD or DVD that it
        //      just examined, then it knows the number of tracks that there must
        //      be, and we can enforce that. If not, when creating a play list,
        //      then this will be zero.
        //
        //  m_imgiEdit
        //      The cover art image. If one is selected while in this dialog, then
        //      m_bNewImage flag is set.
        //
        //  m_pwndXXX
        //      Typed pointers to some of our widgets
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bEditMode;
        tCIDLib::TBoolean       m_bManualMode;
        tCIDLib::TBoolean       m_bNewImage;
        tCIDLib::TBoolean       m_bTitleSelected;
        tCIDLib::TCard2         m_c2ColId;
        tCIDLib::TCard2         m_c2SetId;
        tCIDLib::TCard4         m_c4KnownItemCnt;
        tCQCMedia::TItemList    m_colItems;
        tCQCMedia::EMediaTypes  m_eMType;
        tCQCMedia::TMediaIdList m_fcolOrgItemIds;
        TJPEGImage              m_imgiEdit;
        TMediaCollect           m_mcolEdit;
        TMediaTitleSet          m_mtsEdit;
        TMediaDB*               m_pmdbEdit;
        TComboBox*              m_pwndAction;
        TPushButton*            m_pwndAddCategory;
        TCheckBox*              m_pwndAnamorphic;
        TStaticImg*             m_pwndArt;
        TComboBox*              m_pwndArtist;
        TComboBox*              m_pwndAspect;
        TEntryField*            m_pwndASIN;
        TPushButton*            m_pwndBrowse;
        TPushButton*            m_pwndCancel;
        TEntryField*            m_pwndColName;
        TPushButton*            m_pwndContinue;
        TPushButton*            m_pwndDelCategory;
        TEntryField*            m_pwndDuration;
        TPushButton*            m_pwndEditArt;
        TPushButton*            m_pwndEditCast;
        TPushButton*            m_pwndEditDescr;
        TListBox*               m_pwndCategories;
        TListBox*               m_pwndItems;
        TPushButton*            m_pwndItemDel;
        TPushButton*            m_pwndItemDn;
        TPushButton*            m_pwndItemIns;
        TPushButton*            m_pwndItemUp;
        TComboBox*              m_pwndLabel;
        TEntryField*            m_pwndLocInfo;
        TComboBox*              m_pwndMediaFmt;
        TPushButton*            m_pwndMoreArt;
        TComboBox*              m_pwndRating;
        TPushButton*            m_pwndSelTitle;
        TStaticText*            m_pwndSetName;
        TEntryField*            m_pwndUPC;
        TComboBox*              m_pwndYear;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TEdMetaDataDlg,TDlgBox)
};

#pragma CIDLIB_POPPACK


