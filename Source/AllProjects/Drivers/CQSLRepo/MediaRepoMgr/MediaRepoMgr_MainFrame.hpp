//
// FILE NAME: MediaRepoMgr_MainFrame.hpp
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
//  This is the header for the MediaRepoMgr_MainFrame.cpp file, which provides the
//  main frame for this program.
//
//  We download the whole database and allow the user to make changes, and then
//  commit them back to the server en masse. So all the editing is done locally
//  on our copy of the database.
//
//  We handle media change callbacks. If we get one, we post a code to ourself and
//  the CodeReceived() handler scans for new media. If new media is found then the
//  dialog is popped up that lets the user choose to add new media if desired. This
//  will only happen if we are the active program, else we ignore it. The user can
//  also force a scan manually.
//
//  We have a details section at the bottom where we can display details on the
//  selected stuff. The user can choose to show details for the title, collection or
//  item level.
//
//  When the uses decides to commit changes, we ask if it is ok to remove any empty
//  title sets and collections. If he says ok, we ask the database to remove any
//  empty containers before we upload the changes.
//
//
// CAVEATS/GOTCHAS:
//
//  1)  For the lists that show sets, collections and items, we use a standard
//      string based list window item and just store the id of the object in one of
//      the list window item's extra values. This lets us map back to the actual
//      object. We are always in music or movie mode, so the ids are never ambiguous.
//      They are always relatve to the current media type.
//
//      We could use the templatized list window items and store the actual object
//      pointers, but this could get quite messy trying to make sure that any object
//      that might have been destroyed isn't any longer being pointed to. Using the
//      ids avoids any direct access. And often we can just work with the ids and
//      anyway, never having to even get the objects.
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//  CLASS: TMainFrameWnd
// PREFIX: wnd
// ---------------------------------------------------------------------------
class TMainFrameWnd : public TFrameWnd
{
    public  :
        // -------------------------------------------------------------------
        // Constructors and Destructor
        // -------------------------------------------------------------------
        TMainFrameWnd();

        ~TMainFrameWnd();


        // -------------------------------------------------------------------
        // Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCreate();

        tCIDLib::TBoolean bEditRepoSettings();

        const TMediaRepoSettings& mrsCurrent() const;

        const TString& strRepoMoniker() const;


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

        tCIDLib::TBoolean bAllowShutdown() override;

        tCIDLib::TVoid CodeReceived
        (
            const   tCIDLib::TInt4          i4Code
            , const tCIDLib::TCard4         c4Data
        )   override;

        tCIDLib::TVoid Destroyed() override;

        tCIDLib::TVoid MenuCommand
        (
            const   tCIDLib::TResId         ridItem
            , const tCIDLib::TBoolean       bChecked
            , const tCIDLib::TBoolean       bEnabled
        )   override;

        tCIDLib::TVoid MediaChange
        (
            const   tCIDCtrls::EMediaChTypes eType
        )   override;


    private :
        // -------------------------------------------------------------------
        //  Used to indicate which list (title, collection, item) in a couple
        //  of places.
        // -------------------------------------------------------------------
        enum class ELists
        {
            Collect
            , Item
            , Title

            , None
        };


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid AddToPlayList
        (
            const   tCIDLib::TBoolean       bCollection
        );

        tCIDLib::TBoolean bFindDiscMetaData
        (
            const   TString&                strVolume
            , const TKrnlRemMedia::EMediaTypes eMType
            ,       TMediaCollect&          mcolToFill
            ,       tCQCMedia::TItemList&   colItems
            ,       tCIDLib::TStrList&      colCatNames
            ,       TJPEGImage&             imgToFill
        );

        tCIDLib::TBoolean bLoadArt
        (
            const   tCIDLib::TCard2         c2Id
            ,       TJPEGImage&             imgArt
        );

        tCIDLib::TBoolean bLoadRepoSettings
        (
            const   TString&                strRepoMoniker
            ,       TMediaRepoSettings&     mrsCurrent
        );

        tCIDLib::TBoolean bSelectRepo
        (
            const   TWindow&                wndOwner
            , const tCIDLib::TBoolean       bReload
        );

        tCIDLib::TBoolean bShowColDetails();

        tCIDLib::TBoolean bShowItemDetails();

        tCIDLib::TBoolean bShowTitleDetails();

        tCIDLib::TCard4 c4FindTitleIndexById
        (
            const   tCIDLib::TCard2         c2TitleId
        )   const;

        tCIDLib::TCard4 c4GetSortedTitleList
        (
                    tCQCMedia::TSetIdList&  colToFill
            , const tCQCMedia::EMediaTypes  eMType
        );

        tCIDLib::TVoid ChangeExtMediaPaths
        (
            const   TString&                strFind
            , const TString&                strRepWith
        );

        tCIDLib::TVoid CheckSaveChanges();

        tCIDLib::TVoid CleanupFiles
        (
            const   tCIDLib::TStrList&      colToDelete
        );

        tCIDLib::TVoid DeleteTitle();

        tCIDCtrls::EEvResponses eLBHandler
        (
                    TListChangeInfo&        wnotEvent
        );

        tCIDLib::TVoid EditCollection();

        tCIDLib::TVoid GetLease();

        tCIDLib::TVoid LoadArt
        (
            const   tCQCMedia::EMediaTypes  eMType
            , const tCIDLib::TCard2         c2Id
        );

        tCIDLib::TVoid LoadItems();

        tCIDLib::TVoid LoadTitles();

        tCIDLib::TVoid LoadTitleCols();

        tCIDLib::TVoid NewPlayList();

        const TMediaCollect* pmcolCurrent();

        const TMediaItem* pmitemCurrent();

        const TMediaTitleSet* pmtsCurrent();

        tCIDLib::TVoid ProcessMedia
        (
            const   TKrnlRemMedia::EMediaTypes eMType
            , const TString&                strPath
            , const tCIDLib::TCard4         c4ItemCnt
            , const tCIDLib::TBoolean       bManual
        );

        tCIDLib::TVoid QueryAvailCats
        (
            const   tCQCMedia::EMediaTypes  eType
            ,       TVector<TMediaCat>&     colToFill
        );

        tCIDLib::TVoid QueryCatNames
        (
            const   TMediaCollect&          mcolEdit
            , const TVector<TMediaCat>&     colAvailCats
            ,       tCIDLib::TStrList&      colToFill
        );

        tCIDLib::TVoid RemoveTitle
        (
            const   tCQCMedia::EMediaTypes  eType
            , const tCIDLib::TCard2         c2Id
        );

        tCIDLib::TVoid SendChanges();

        tCIDLib::TVoid ShowDetails();

        tCIDLib::TVoid ShowNewListTitle
        (
            const   tCQCMedia::EMediaTypes  eType
            , const tCIDLib::TCard2         c2TitleId
        );

        tCIDLib::TVoid StoreRepoSettings();

        tCIDLib::TVoid WriteSlotReport();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_eMTFlags
        //      The media type flags for the repository we are currently editing. It
        //      indicates the types of media supported.
        //
        //  m_eMediaType
        //      The current media type we are browsing, which is based on the media
        //      type menu selection. Be careful because not all opeations are in
        //      terms of the currently selected media type!
        //
        //  m_eSortOrder
        //      The current sort order, which controls what field we are sorting
        //      the available titles on. This is driven by a menu selection. This
        //      indicates how the m_colTitles collection is sorted.
        //
        //  m_hmenuSort
        //  m_hmenuType
        //      We need to interact with these two sub-menus a good bit so we
        //      set them up on start up and save them.
        //
        //  m_mdbEdit
        //      The current media database that the repo selection dialog got for
        //      us, the one we are editing.
        //
        //  m_mrsCurrent
        //      The current media repository settings values, the last ones we got
        //      when a new repo was selected.
        //
        //  m_pwndClient
        //      We create a generic client window to contain all the widgets,
        //      since this is an application window, not a dialog box. We
        //      don't own it, but keep it around since it contains all the
        //      child widgets and we need to get to them sometimes.
        //
        //  m_pwndXXX
        //      Typed pointers to our child widgets we need to interact with
        //      in a type specific way. We don't own them.
        //
        //  m_strDBSerialNum
        //      when we download a new repo, we store the DB serial number so that
        //      we can check to see if it has actually changed since the last time
        //      if the user asks to upload.
        //
        //  m_strLeaseId
        //      We generate a unique id to use for leasing the connection to the repo.
        //      It only allows one manager at a time to be connected, so each client
        //      will generate an id and use this to get and control the lease over
        //      the repo. When a new repo is selected, we get a new lease and drop
        //      any old one.
        //
        //  m_strRepoMoniker
        //      The current repository that we are managing.
        //
        //  m_strRepoPath
        //      The repo path of the currently selected repo.
        //
        //  m_strTitle
        //      Since we put current repo info into the title bar, we can't let the
        //      popups automatically get their title text from our text. So we preload
        //      the basic text we want them to use and pass it in to all of them.
        // -------------------------------------------------------------------
        tCQCMedia::EMediaTypes  m_eMediaType;
        tCQCMedia::EMTFlags     m_eMTFlags;
        tCQCMedia::ESortOrders  m_eSortOrder;
        TSubMenu                m_menuSort;
        TSubMenu                m_menuType;
        TMediaDB                m_mdbEdit;
        TMediaRepoSettings      m_mrsCurrent;
        TStaticImg*             m_pwndArt;
        TStaticText*            m_pwndArtist;
        TStaticText*            m_pwndAspect;
        TMultiEdit*             m_pwndCast;
        TGenericWnd*            m_pwndClient;
        TListBox*               m_pwndColList;
        TStaticText*            m_pwndDateAdded;
        TMultiEdit*             m_pwndDescr;
        TComboBox*              m_pwndDetType;
        TStaticText*            m_pwndDuration;
        TListBox*               m_pwndItemList;
        TStaticText*            m_pwndLabel;
        TStaticText*            m_pwndLocInfo;
        TStaticText*            m_pwndMyRating;
        TStaticText*            m_pwndRating;
        TStaticText*            m_pwndTitle;
        TListBox*               m_pwndTitleList;
        TStaticText*            m_pwndYear;
        TString                 m_strDBSerialNum;
        TString                 m_strLeaseId;
        TString                 m_strRepoMoniker;
        TString                 m_strRepoPath;
        TString                 m_strTitle;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TMainFrameWnd,TFrameWnd)
};

#pragma CIDLIB_POPPACK

