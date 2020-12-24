//
// FILE NAME: CQCTreeBrws_BrowserWnd.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/19/2015
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
//  This is a key client side bit of functionality. It provides a tree based browser
//  for various available services that a client might want to expose for viewing or
//  editing.
//
//  We create our own derivative of the tree view control, so that we can deal with
//  drag and drop operations, and can do custom drawing.
//
//  We use the 'user flag' of the tree control as an indicator for custom drawing.
//  If the flag is true, it indicates that that item is in some way in a 'non-normal'
//  state and it will be displayed in red. The browsers queue up change indicators
//  on the browser window to inform of good/bad state changes and the window will
//  update the flag and redraw the affected item(s). We define a small class for
//  such change notifications, which can also include other types of changes.
//
//  We also have a simple derivative that is designed for browsing a single type of
//  data from the data server. So it installs a data server browser and sets itself up
//  for a specific data type. It also has a 'select only' flag. If select only, the user
//  can't create a new file. Else they can create new scopes and files.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TTreeBrowseChange
//  PREFIX: tbc
// ---------------------------------------------------------------------------
class CQCTREEBRWSEXP TTreeBrowseChange
{
    public :
        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static const TString& strKey
        (
            const   TTreeBrowseChange&      tbcSrc
        );


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TTreeBrowseChange();

        TTreeBrowseChange
        (
            const   TString&                strPath
            , const TString&                strOpName
            , const TString&                strData
        );

        TTreeBrowseChange(const TTreeBrowseChange&) = default;
        TTreeBrowseChange(TTreeBrowseChange&&) = default;

        ~TTreeBrowseChange();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TTreeBrowseChange& operator=(const TTreeBrowseChange&) = default;
        TTreeBrowseChange& operator=(TTreeBrowseChange&&) = default;


        TString m_strData;
        TString m_strKey;
        TString m_strOpName;
        TString m_strPath;
};



// ---------------------------------------------------------------------------
//   CLASS: TTreeBrowseInfo
//  PREFIX: wnot
// ---------------------------------------------------------------------------
class CQCTREEBRWSEXP TTreeBrowseInfo : public TCtrlNotify
{
    public  :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TTreeBrowseInfo();

        TTreeBrowseInfo
        (
            const   tCQCTreeBrws::EEvents   eEvent
            , const TString&                strPath
            , const tCQCRemBrws::EDTypes    eType
            , const TWindow&                wndSrc
        );

        TTreeBrowseInfo
        (
            const   tCQCTreeBrws::EEvents   eEvent
            , const TString&                strOldPath
            , const TString&                strPath
            , const tCQCRemBrws::EDTypes    eType
            , const TWindow&                wndSrc
        );

        TTreeBrowseInfo(const TTreeBrowseInfo&) = default;
        TTreeBrowseInfo(TTreeBrowseInfo&&) = default;

        ~TTreeBrowseInfo();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TTreeBrowseInfo& operator=(const TTreeBrowseInfo&) = default;
        TTreeBrowseInfo& operator=(TTreeBrowseInfo&&) = default;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bFlag() const;

        tCIDLib::TBoolean bFlag
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCQCTreeBrws::EEvents eEvent() const;

        tCQCRemBrws::EDTypes eType() const;

        const TString& strOldPath() const;

        const TString& strPath() const;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bFlag
        //      Used in sync notifications where they the handler can give back a yes
        //      or no response.
        //
        //  m_eEvent
        //      The event being reported. For those things other than edit/view type stuff,
        //      which the client program handles, we are reporting things already done, so
        //      he needs to react to them.
        //
        //  m_eType
        //      The type of the data being reported on. It's sort of implied by the path,
        //      but the sender already knows it so it's just easier to send it along and save
        //      the receiver work.
        //
        //  m_strOldPath
        //      If the event is a rename, this the old path, and m_strPath is the new path.
        //      It's the full hierarchical path
        //
        //  m_strPath
        //      The selected path that the event is for. It's the full hierarchical path.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bFlag;
        tCQCTreeBrws::EEvents   m_eEvent;
        tCQCRemBrws::EDTypes    m_eType;
        TString                 m_strOldPath;
        TString                 m_strPath;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TTreeBrowseInfo,TCtrlNotify)
        DefPolyDup(TTreeBrowseInfo)
};


// ---------------------------------------------------------------------------
//   CLASS: MRemBrwsBrwsCB
//  PREFIX: rbbc
// ---------------------------------------------------------------------------
class CQCTREEBRWSEXP MRemBrwsBrwsCB
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        MRemBrwsBrwsCB() = default;
        MRemBrwsBrwsCB(const MRemBrwsBrwsCB&) = default;
        MRemBrwsBrwsCB(MRemBrwsBrwsCB&&) = default;
        virtual ~MRemBrwsBrwsCB() = default;


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        MRemBrwsBrwsCB& operator=(const MRemBrwsBrwsCB&) = default;
        MRemBrwsBrwsCB& operator=(MRemBrwsBrwsCB&&) = default;


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        virtual tCIDLib::TVoid TreeBrowserEvent
        (
                    TTreeBrowseInfo&        wnotEvent
        ) = 0;
};



// ---------------------------------------------------------------------------
//   CLASS: TCQCTreeBrowser
//  PREFIX: wnd
// ---------------------------------------------------------------------------
class CQCTREEBRWSEXP TCQCTreeBrowser : public TTreeView, public MDragAndDrop
{
    public :
        // -------------------------------------------------------------------
        //  Public, static data
        // -------------------------------------------------------------------
        static const TNotificationId    nidBrowseEvent;


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCTreeBrowser();

        TCQCTreeBrowser(const TCQCTreeBrowser&) = delete;
        TCQCTreeBrowser(TCQCTreeBrowser&&) = delete;

        ~TCQCTreeBrowser();


        // -------------------------------------------------------------------
        //  Pubic operators
        // -------------------------------------------------------------------
        TCQCTreeBrowser& operator=(const TCQCTreeBrowser&) = delete;
        TCQCTreeBrowser& operator=(TCQCTreeBrowser&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bProcessAccel
        (
            const   tCIDLib::TVoid* const   pMsgData
        )   const final;

        tCIDLib::TVoid DragExit() final;

        tCIDLib::TBoolean bCanAcceptFiles
        (
            const   tCIDLib::TStrList&      colFiles
            , const tCIDLib::TStrHashSet&   colExts
        )   const final;

        tCIDLib::TBoolean bDragDropped
        (
            const   TPoint&                 pntAt
            , const tCIDLib::TStrList&      colFiles
            , const tCIDLib::TStrHashSet&   colExts
        )   final;

        tCIDLib::TBoolean bDragEnter
        (
            const   TPoint&                 pntAt
            , const tCIDLib::TStrList&      colFiles
            , const tCIDLib::TStrHashSet&   colExts
        )   final;

        tCIDLib::TBoolean bDragOver
        (
            const   TPoint&                 pntAt
            , const tCIDLib::TStrList&      colFiles
            , const tCIDLib::TStrHashSet&   colExts
        )   final;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid AddBrowser
        (
                    TCQCTreeBrwsIntf* const pbrwsToAdopt
        );

        tCIDLib::TBoolean bGetNewName
        (
            const   TString&                strParHPath
            , const tCIDLib::TBoolean       bIsScope
            , const tCQCRemBrws::EDTypes    eDType
            ,       TString&                strToFill
            ,       TCQCTreeBrwsIntf&       rbrwsValidate
        );

        tCIDLib::TVoid CreateBrowser
        (
            const   TWindow&                wndParent
            , const tCIDCtrls::TWndId       widId
            , const TArea&                  areaInit
            , const tCIDCtrls::EWndStyles   eStyles
            , const tCQCTreeBrws::EBrwsStyles eBrwsStyles
            , const TCQCUserCtx&            cuctxToUse
        );

        tCIDLib::TVoid FileUpdated
        (
            const   TString&                strPath
        );

        tCIDLib::TVoid ProcessChanges();

        tCIDLib::TVoid QueueChange
        (
            const   TString&                strPath
            , const TString&                strOpName
            , const TString&                strData
        );

        tCIDLib::TVoid SetItemStatus
        (
            const   TString&                strPath
            , const tCQCTreeBrws::EItemStates eToSet
        );


        // -------------------------------------------------------------------
        //  We provide a templatized method to allow for typesafe registration of
        //  event handlers. In our case we have to give it a type specific name
        //  becase we are derived from the tree view, which also provides his own
        //  notificaton registration method with the usual name.
        // -------------------------------------------------------------------
        template <typename T> TNotHandlerBase*
        pnothRegisterBrowserHandler(        T* const        pTarget
                                    ,       tCIDCtrls::EEvResponses (T::*pfnTarget)(TTreeBrowseInfo&)
                                    , const tCIDLib::EEnds  eEnd = tCIDLib::EEnds::Last)
        {
            CheckHandlerRegParms(this, pTarget);

            // Add a request object to us (the source button)
            AddNotifyRequest
            (
                new TNotifyRequestFor<T, TTreeBrowseInfo>(pTarget, TCQCTreeBrowser::nidBrowseEvent)
            );

            // Add a handler object to the target
            TNotHandlerBase* pnothNew = new TNotifyHandlerFor<T, TTreeBrowseInfo>
            (
                pTarget, TCQCTreeBrowser::nidBrowseEvent, pfnTarget
            );
            pTarget->AddNotifyHandler(pnothNew, eEnd);
            return pnothNew;
        }


    protected :
        // -------------------------------------------------------------------
        //  Protected inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCreated() override;

        tCIDLib::TVoid CodeReceived
        (
            const   tCIDLib::TInt4          i4Code
            , const tCIDLib::TCard4         c4Data
        )   override;

        tCIDLib::TVoid Destroyed() override;

        tCIDCtrls::ETreeCustRets eCustomDraw
        (
            const   TString&                strPath
            ,       TObject* const          pobjUser
            , const tCIDLib::TCard4         c4UserFlag
            , const tCIDLib::TBoolean       bPost
            , const tCIDLib::TBoolean       bSelected
            , const tCIDLib::TCard4         c4Level
            ,       TRGBClr&                rgbBgn
            ,       TRGBClr&                rgbText
        )   override;

        tCIDLib::TVoid MenuCommand
        (
            const   tCIDLib::TResId         ridItem
            , const tCIDLib::TBoolean       bChecked
            , const tCIDLib::TBoolean       bEnabled
        )   override;


    private :
        // -------------------------------------------------------------------
        //  Private class types
        // -------------------------------------------------------------------
        using TBrowserList = TRefVector<TCQCTreeBrwsIntf> ;
        using TChangeQ = TQueue<TTreeBrowseChange>;


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bTestCanDrop
        (
            const   TPoint&                 pntAt
            , const tCIDLib::TStrHashSet&   colExts
            ,       TString&                strPath
        );

        tCIDLib::TVoid CheckChangeQ();

        tCIDLib::TVoid CleanupBrowsers();

        tCIDCtrls::EEvResponses eTreeHandler
        (
                    TTreeEventInfo&         wnotEvent
        );

        tCIDLib::TVoid MakeDefName
        (
            const   TString&                strParScope
            ,       TString&                strToFill
            , const tCIDLib::TBoolean       bIsFile
        );

        TCQCTreeBrwsIntf* pbrwsFindOwner
        (
            const   TString&                strPath
            , const tCIDLib::TBoolean       bThrowIfNot
        );

        const TCQCTreeBrwsIntf* pbrwsFindOwner
        (
            const   TString&                strPath
            , const tCIDLib::TBoolean       bThrowIfNot
        )   const;


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bInChangeQ
        //      It's possible that we could get re-entered on the CheckChangeQ()
        //      method if we do a callback to one on of the browser objects and it
        //      does something that allows processing of the message queue, and
        //      there has been another code posted to process the queue. So this is
        //      used to avoid that. We set it upon entry to CheckChangeQ() and clear
        //      it when we leave. Any newly queue stuff will still get processed
        //      since it's already in the queue by the time the request to process
        //      is posted.
        //
        //  m_colBrowsers
        //      An adopting list of TCQCRemBrws objects. All browsers must be added
        //      before we are created. The list is dynamic at viewing time.
        //
        //  m_colDropFiles
        //      If we get a successful file drop, then we store the files here and
        //      post ourself an async code so that we can let the shell continue
        //      on and we'll process it in our own thread.
        //
        //  m_colChangeQ
        //      We allow the browsers to queue up change notifications on us. We
        //      set it to be thread safe, but ultimately we do some non-atomic ops
        //      on it so we have to lock its mutex in those cases, else we can just
        //      just do single ops on it. It's not really a queue, it's a keyed hash
        //      set so that we can efficiently just update existing items if the
        //      status changes again before the window processes it.
        //
        //      It's not really a queue, it's a keyed hash set so that we can see if
        //      we already have an item for that path and just updated it if so, to
        //      avoid the list getting really large if the GUI thread gets hung up
        //      for a bit. This way, it shouldn't ever get bigger than the number of
        //      drivers.
        //
        //  m_cuctxUser
        //      We need a security token to access the info, and we want to know the
        //      user's role and such, so the containing app has to provide us with the
        //      logged in user's user context.
        //
        //  m_strDropPath
        //      If we get a successful drop we store the target path here. See
        //      the m_colDropFiles member.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bInChangeQ;
        TBrowserList        m_colBrowsers;
        tCIDLib::TStrList   m_colDropFiles;
        TChangeQ            m_colChangeQ;
        TCQCUserCtx         m_cuctxToUse;
        TString             m_strDropPath;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCTreeBrowser,TTreeView)
};


// ---------------------------------------------------------------------------
//   CLASS: TCQCTreeTypeBrowser
//  PREFIX: wnd
// ---------------------------------------------------------------------------
class CQCTREEBRWSEXP TCQCTreeTypeBrowser : public TCQCTreeBrowser
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCTreeTypeBrowser();

        TCQCTreeTypeBrowser(const TCQCTreeTypeBrowser&) = delete;

        ~TCQCTreeTypeBrowser();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCTreeTypeBrowser& operator=(const TCQCTreeTypeBrowser&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid CreateTreeTypeBrowser
        (
            const   TWindow&                wndOwner
            , const tCQCRemBrws::EDTypes    eDType
            , const tCIDCtrls::TWndId       widId
            , const TArea&                  areaInit
            , const tCIDCtrls::EWndStyles   eStyles
            , const tCQCTreeBrws::EBrwsStyles eBrwsStyles
            , const TCQCUserCtx&            cuctxToUse
        );


    private :
        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCTreeTypeBrowser, TCQCTreeBrowser)
};

#pragma CIDLIB_POPPACK

