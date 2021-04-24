//
// FILE NAME: CQCGKit_TreeBrowser.cpp
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
//  This file implements the standard CQC tree browser window.
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
#include    "CQCTreeBrws_.hpp"
#include    "CQCTreeBrws_CfgSrvBrws_.hpp"
#include    "CQCTreeBrws_DataSrvBrws_.hpp"
#include    "CQCTreeBrws_DevicesBrws_.hpp"
#include    "CQCTreeBrws_HelpBrws_.hpp"
#include    "CQCTreeBrws_SystemBrws_.hpp"



// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TTreeBrowseInfo, TCtrlNotify)
AdvRTTIDecls(TCQCTreeBrowser, TTreeView)
AdvRTTIDecls(TCQCTreeTypeBrowser, TCQCTreeBrowser)


// ---------------------------------------------------------------------------
//   CLASS: TTreeBrowseChange
//  PREFIX: tbc
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TTreeBrowseChange: Public, static methods
// ---------------------------------------------------------------------------
const TString& TTreeBrowseChange::strKey(const TTreeBrowseChange& tbcSrc)
{
    return tbcSrc.m_strPath;
}


// ---------------------------------------------------------------------------
//  TTreeBrowseChange: Constructors and Destructor
// ---------------------------------------------------------------------------
TTreeBrowseChange::TTreeBrowseChange()
{
}

TTreeBrowseChange::TTreeBrowseChange(const  TString&    strPath
                                    , const TString&    strOpName
                                    , const TString&    strData) :
    m_strData(strData)
    , m_strPath(strPath)
    , m_strOpName(strOpName)
{
    m_strKey = strPath;
    m_strKey.Append(kCIDLib::chForwardSlash);
    m_strKey.Append(strOpName);
}

TTreeBrowseChange::~TTreeBrowseChange()
{
}




// ---------------------------------------------------------------------------
//   CLASS: TTreeBrowseInfo
//  PREFIX: wnot
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TTreeBrowseInfo: Constructors and Destructor
// ---------------------------------------------------------------------------
TTreeBrowseInfo::TTreeBrowseInfo() :

    m_bFlag(kCIDLib::False)
    , m_eEvent(tCQCTreeBrws::EEvents::Count)
    , m_eType(tCQCRemBrws::EDTypes::Count)
{
}

TTreeBrowseInfo::TTreeBrowseInfo(const  tCQCTreeBrws::EEvents   eEvent
                                , const TString&                strPath
                                , const tCQCRemBrws::EDTypes    eType
                                , const TWindow&                wndSrc) :
    TCtrlNotify(wndSrc)
    , m_bFlag(kCIDLib::False)
    , m_eEvent(eEvent)
    , m_eType(eType)
    , m_strPath(strPath)
{
}

TTreeBrowseInfo::TTreeBrowseInfo(const  tCQCTreeBrws::EEvents   eEvent
                                , const TString&                strOldPath
                                , const TString&                strPath
                                , const tCQCRemBrws::EDTypes    eType
                                , const TWindow&                wndSrc) :
    TCtrlNotify(wndSrc)
    , m_bFlag(kCIDLib::False)
    , m_eEvent(eEvent)
    , m_eType(eType)
    , m_strOldPath(strOldPath)
    , m_strPath(strPath)
{
}

TTreeBrowseInfo::~TTreeBrowseInfo()
{
}


// ---------------------------------------------------------------------------
//  TTreeBrowseInfo: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TTreeBrowseInfo::bFlag() const
{
    return m_bFlag;
}

tCIDLib::TBoolean TTreeBrowseInfo::bFlag(const tCIDLib::TBoolean bToSet)
{
    m_bFlag = bToSet;
    return m_bFlag;
}

tCQCTreeBrws::EEvents TTreeBrowseInfo::eEvent() const
{
    return m_eEvent;
}


tCQCRemBrws::EDTypes TTreeBrowseInfo::eType() const
{
    return m_eType;
}


const TString& TTreeBrowseInfo::strOldPath() const
{
    return m_strOldPath;
}


const TString& TTreeBrowseInfo::strPath() const
{
    return m_strPath;
}




// ---------------------------------------------------------------------------
//   CLASS: TCQCTreeBrowser
//  PREFIX: wnd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// TCQCTreeBrowser: Public, static method
// ---------------------------------------------------------------------------
const TNotificationId    TCQCTreeBrowser::nidBrowseEvent(L"TCQCTreeBrowserEvent");


// ---------------------------------------------------------------------------
//  TCQCTreeBrowser: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCTreeBrowser::TCQCTreeBrowser() :

    TTreeView()
    , m_bInChangeQ(kCIDLib::False)
    , m_colBrowsers(tCIDLib::EAdoptOpts::Adopt)
    , m_colChangeQ(tCIDLib::EMTStates::Safe)
{
}

TCQCTreeBrowser::~TCQCTreeBrowser()
{
    // Clean up any brower objects we got
    CleanupBrowsers();
}



// ---------------------------------------------------------------------------
//  TCQCTreeBrowser: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  We will get called here if the main frame gets an accelerator command and
//  we have the focus. He will pass it to our tabbed window parent, who will pass
//  it to us if we are the active tab.
//
tCIDLib::TBoolean
TCQCTreeBrowser::bProcessAccel(const tCIDLib::TVoid* const pMsgData) const
{
    // If we don't have the focus, do nothing
    if (!bHasFocus())
        return kCIDLib::False;

    // Get the currently selected item. If none, do nothing
    TString strCurPath;
    if (!bSelectedItem(strCurPath))
        return kCIDLib::False;

    //
    //  See which browser object owns this path. If none, we are done, which means
    //  it's one of the top level paths basically.
    //
    const TCQCTreeBrwsIntf* pbrwsCur = pbrwsFindOwner(strCurPath, kCIDLib::False);
    if (!pbrwsCur)
        return kCIDLib::False;

    //
    //  Ask the browser object to fill in an accellerator table for us. This is
    //  sort of wierd but we don't want to do it every time the selection changes
    //  since that would be serious overkill, and he can't process the table
    //  himself, not being a window. So this is the lowest overhead way of doing
    //  it. We just let him provide us a table and we process it right here. We
    //  get the menu command and pass it on to him.
    //
    TAccelTable accelBrws;
    pbrwsCur->QueryAccelTable(strCurPath, accelBrws);

    return accelBrws.bProcessAccel(*this, pMsgData);
}


tCIDLib::TVoid TCQCTreeBrowser::DragExit()
{
    // Nothing really to do in this case
}


//
//  If the extensions indicate a file type that we can accept (assuming it gets
//  dropped on the right area of the tree), then say we'll allow this operation to
//  provisionally continue.
//
tCIDLib::TBoolean
TCQCTreeBrowser::bCanAcceptFiles(const  tCIDLib::TStrList&      colFiles
                                , const tCIDLib::TStrHashSet&   colExts) const
{
    return  colExts.bHasElement(L"JPG")
            || colExts.bHasElement(L"JPEG")
            || colExts.bHasElement(L"PNG")
            || colExts.bHasElement(L"BMP")
            || colExts.bHasElement(L"CQCTEV")
            || colExts.bHasElement(L"CQCSEV");
}


//
//  They dropped. Check the to see if we are over something that will accept at
//  least file we got. If so, we store the file list and post ourself an async
//  command. We don't want to lock up the shell while we do this.
//
tCIDLib::TBoolean
TCQCTreeBrowser::bDragDropped(  const   TPoint&                 pntAt
                                , const tCIDLib::TStrList&      colFiles
                                , const tCIDLib::TStrHashSet&   colExts)
{
    if (!bTestCanDrop(pntAt, colExts, m_strDropPath))
        return kCIDLib::False;

    m_colDropFiles = colFiles;
    SendCode(kCQCTreeBrws_::i4FileDrop, 0);
    return kCIDLib::True;
}


//
//  We don't need to do anything special on entry, so we just call a helper that
//  does location sensitive feedback.
//
tCIDLib::TBoolean
TCQCTreeBrowser::bDragEnter(const   TPoint&                 pntAt
                            , const tCIDLib::TStrList&      colFiles
                            , const tCIDLib::TStrHashSet&   colExts)
{
    TString strPath;
    return bTestCanDrop(pntAt, colExts, strPath);
}


//
//  Just call a helper to see if we are over an item taht can accept at least one
//  of the files we have.
//
tCIDLib::TBoolean
TCQCTreeBrowser::bDragOver( const   TPoint&                 pntAt
                            , const tCIDLib::TStrList&      colFiles
                            , const tCIDLib::TStrHashSet&   colExts)
{
    TString strPath;
    return bTestCanDrop(pntAt, colExts, strPath);
}



// ---------------------------------------------------------------------------
//  TCQCTreeBrowser: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCTreeBrowser::AddBrowser(TCQCTreeBrwsIntf* const pbrwsToAdopt)
{
    if (bIsValid())
    {
        facCQCTreeBrws().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kTBrwsErrs::errcBrws_AlreadyCreated
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Already
        );
    }

    // Add this guy to the list
    m_colBrowsers.Add(pbrwsToAdopt);
}


//
//  This is a helper to get a new name. It's a bit messy so we provide this for all
//  of the browsers to use. We want to get a name before the file is created. So we
//  have to put a dummy item into the tree, suppressing any subsequent notifications
//  from being reacted to, then push it into in place edit mode to get a new name.
//  If the user doesn't commit, we remove that dummy item again. If they commit, we
//  make sure it's unique within its parent scope. The in place editor is given a
//  pointer to the calling browser (via the IPE mixin) so that it can pass any other
//  required judgement on the name.
//
//  It's always a file here. Scopes are handled differently.
//
tCIDLib::TBoolean
TCQCTreeBrowser::bGetNewName(const  TString&                strParHPath
                            , const tCIDLib::TBoolean       bIsScope
                            , const tCQCRemBrws::EDTypes    eDType
                            ,       TString&                strToFill
                            ,       TCQCTreeBrwsIntf&       rbrwsValidate)
{
    // Create a new dummy item with a default name
    TString strDefName(L" ");

    // Create the new hierarchical path
    TString strNewHPath(strParHPath);
    facCQCRemBrws().AddPathComp(strNewHPath, strDefName);

    //
    //  Add it to the browser. If a scope, it's always non-virtual since we just created
    //  it and therefore it obviously doesn't have any content to fault in.
    //
    if (bIsScope)
        AddScope(strParHPath, strDefName, kCIDLib::False);
    else
        AddItem(strParHPath, strDefName);

    // Make sure the target is visible
    MakeItemVisible(strNewHPath);

    // Get the area of the target item, tell it to use the whole window width
    TArea areaItem;
    if (!bQueryItemArea(strNewHPath, areaItem, kCIDLib::False, kCIDLib::False))
    {
        RemoveItem(strNewHPath);
        return kCIDLib::False;
    }

    //
    //  We need to fit a border and some margin into this guy, so inflate it
    //  out by 1 pixel beyond what the actual tree cell is.
    //
    areaItem.Inflate(1, 1);

    //
    //  It's only the size of the existing column, so enforce a minimum width so that
    //  they can type something reasonable.
    //
    if (areaItem.c4Width() < 128)
        areaItem.c4Width(128);

    TAttrData adatEdit;
    adatEdit.Set
    (
        bIsScope ? kCQCTreeBrws::strBrwsAttr_ScopeName : kCQCTreeBrws::strBrwsAttr_ItemName
        , bIsScope ? kCQCTreeBrws::strBrwsAttr_ScopeName : kCQCTreeBrws::strBrwsAttr_ItemName
        , kCIDMData::strAttrLim_Required
        , tCIDMData::EAttrTypes::String
    );

    //
    //  Store the parent path as the special type value so that it'savailable in the
    //  validation callback.
    //
    adatEdit.SetSpecType(strParHPath);

    // Invoke the in place editor now at this area
    TInPlaceEdit ipeVal;
    if (ipeVal.bProcess(*this, areaItem, adatEdit, rbrwsValidate, 0))
    {
        //
        //  Give them back the actual name that was used. If they later decide
        //  they can't create the file, they can remove it since they know the path.
        //
        UpdateItem(strParHPath, strDefName, adatEdit.strValue());
        strToFill = adatEdit.strValue();
        return kCIDLib::True;
    }

    // Remove the new guy since they bailed
    RemoveItem(strNewHPath);
    return kCIDLib::False;
}


//
//  Create ourself, which will kick off the creation of any contained windows
//
tCIDLib::TVoid
TCQCTreeBrowser::CreateBrowser( const   TWindow&                    wndParent
                                , const tCIDCtrls::TWndId           widId
                                , const TArea&                      areaInit
                                , const tCIDCtrls::EWndStyles       eStyles
                                , const tCQCTreeBrws::EBrwsStyles   eBrwsStyles
                                , const TCQCUserCtx&                cuctxToUse)
{
    // Store away the user context
    m_cuctxToUse = cuctxToUse;

    // If asked to, we add all of the browers
    if (tCIDLib::bAllBitsOn(eBrwsStyles, tCQCTreeBrws::EBrwsStyles::StdBrowsers))
    {
        AddBrowser(new TCQCDataSrvBrws());
        AddBrowser(new TCQCCfgSrvBrws());
        AddBrowser(new TCQCDevicesBrws());
        AddBrowser(new TCQCSystemBrws());
        AddBrowser(new TCQCHelpBrws());
    }

    // Set up the underlying browser styles to use below
    tCIDCtrls::ETreeStyles eTreeStyles = tCIDCtrls::ETreeStyles::HasLines;
    if (tCIDLib::bAllBitsOn(eBrwsStyles, tCQCTreeBrws::EBrwsStyles::InvokeOnEnter))
        eTreeStyles = tCIDLib::eOREnumBits(eTreeStyles, tCIDCtrls::ETreeStyles::InvokeOnEnter);
    if (tCIDLib::bAllBitsOn(eBrwsStyles, tCQCTreeBrws::EBrwsStyles::ReadOnly))
        eTreeStyles = tCIDLib::eOREnumBits(eTreeStyles, tCIDCtrls::ETreeStyles::ReadOnly);

    // Create ourself, calling the tree view parent class' create method.
    Create(wndParent, widId, areaInit, eStyles, tCIDCtrls::EExWndStyles::None, eTreeStyles);
}


//
//  When a file is opened for editing and the user does a save, Updated is called
//  so that we can do any adjusting required.
//
tCIDLib::TVoid TCQCTreeBrowser::FileUpdated(const TString& strPath)
{
    TCQCTreeBrwsIntf* pbrwsCur = pbrwsFindOwner(strPath, kCIDLib::True);
    pbrwsCur->UpdateFile(strPath);
}


//
//  We find the browser object that owns the passed parent scope and ask him
//  to provide us with a default iniital name for a newly created item in that
//  scope. It can be a child file or a child scope.
//
tCIDLib::TVoid
TCQCTreeBrowser::MakeDefName(const  TString&            strParScope
                            ,       TString&            strToFill
                            , const tCIDLib::TBoolean   bIsFile)
{
    // Find the handler for this path
    const TCQCTreeBrwsIntf* pbrwsCur = pbrwsFindOwner(strParScope, kCIDLib::True);
    pbrwsCur->MakeDefName(strParScope, strToFill, bIsFile);
}


tCIDLib::TVoid TCQCTreeBrowser::ProcessChanges()
{
    // Default is async, which is what we want
    SendCode(kCQCTreeBrws_::i4ChangeQ, 0);
}


//
//  The browser objects can call this to tell us asynchronously about changes, which
//  they need if they do any background processing. We'll handle these and turn around
//  and call them back on the GUI thread.
//
tCIDLib::TVoid
TCQCTreeBrowser::QueueChange(const  TString&    strPath
                            , const TString&    strOpName
                            , const TString&    strData)
{
    m_colChangeQ.CheckIsFull(8192, L"browser tree change queue");
    m_colChangeQ.objPut(TTreeBrowseChange(strPath, strOpName, strData));
}


//
//  A convenience to set the 'item status' of an item in the tree. We use the user
//  flag to indicate a normal, paused, or failed status, which is used by our custom
//  drawing stuff below to color code the items.
//
tCIDLib::TVoid
TCQCTreeBrowser::SetItemStatus( const   TString&                    strPath
                                , const tCQCTreeBrws::EItemStates   eToSet)
{
    // Set the user flag for this guy and tell it to redraw the item
    SetUserFlag(strPath, tCIDLib::c4EnumOrd(eToSet), kCIDLib::True);
}


// ---------------------------------------------------------------------------
//  TCQCTreeBrowser: Protected, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Once created we need to load up the top level items for all of the browsers
//  we got. Note that the data server will already know about these.
//
//  The default initialization is to add a non-virtual root, which is what we want, so
//  we don't need to reset the tree.
//
tCIDLib::TBoolean TCQCTreeBrowser::bCreated()
{
    TParent::bCreated();

    //
    //  Set window text on us. It won't display but it'll get picked up when we do
    //  popups owned by this window.
    //
    strWndText(facCQCTreeBrws().strMsg(kTBrwsMsgs::midTitle_BrowserWnd));

    // Now load the stuff under the root
    const tCIDLib::TCard4 c4Count = m_colBrowsers.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        // Add this one to the list and store the returned id on it
        TCQCTreeBrwsIntf* pbrwsCur = m_colBrowsers[c4Index];

        // Give him a pointer to us
        pbrwsCur->SetWindows(this);

        // Initialize it now, giving it a copy of our security token to use
        try
        {
            pbrwsCur->Initialize(m_cuctxToUse);
        }

        catch(TError& errToCatch)
        {
            if (facCQCTreeBrws().bShouldLog(errToCatch))
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }

            // Skip it for now
            continue;
        }
    }

    //
    //  Pre-expand the root item, so that we don't get an expanding event for it
    //  later and try to load it twice. As long as we do it before we install the
    //  event handler below.
    //
    SetExpandState(kCQCRemBrws::strPath_Root, kCIDLib::True);

    //
    //  This is kind of wierd but we have to see notifications from our own base
    //  class and react to them internally, sending out our own notifications to
    //  listeners where that is relevant.
    //
    pnothRegisterHandler(this, &TCQCTreeBrowser::eTreeHandler);

    // Initialize drag and drop support on this window
    InitDragAndDrop(*this);

    // Enable custom draw for pre-per-item drawing
    SetCustomDrawOpts(tCIDCtrls::ETreeCustOpts::PreItem);

    return kCIDLib::True;
}


//
//  We post an async code to ourself to handle file drops so that we don't lock
//  up the shell thread. Also, the browser objects can post to us to let us know
//  they have queued up change notifications in our change queue.
//
tCIDLib::TVoid
TCQCTreeBrowser::CodeReceived(  const   tCIDLib::TInt4  i4Code
                                , const tCIDLib::TCard4 c4Data)
{
    if (i4Code == kCQCTreeBrws_::i4ChangeQ)
    {
        //
        //  A browser object is telling us he has added items to the queue. Our
        //  QueueChange() method does this when they call it. We need to process
        //  any items in the queue and flush it.
        //
        CheckChangeQ();
    }
     else if (i4Code == kCQCTreeBrws_::i4FileDrop)
    {
        //
        //  Process the names and create new ones if needed that are acceptable for
        //  our needs, and we get rid of any path component and extension. So, for
        //  each entry in m_colDropFiles we have the corresponding basic name in
        //  colRealNames. We remove any that couldn't possibly be correct, so we
        //  don't have to bother with them.
        //
        tCIDLib::TCard4 c4Count = m_colDropFiles.c4ElemCount();
        tCIDLib::TStrList colRealNames(c4Count);

        tCIDLib::TCard4 c4Index = 0;
        while (c4Index < c4Count)
        {
            TPathStr pathCur = m_colDropFiles[c4Index];

            // It can't be valid if it ends in a slash
            if ((pathCur.chLast() == kCIDLib::chForwardSlash)
            ||  (pathCur.chLast() == kCIDLib::chBackSlash))
            {
                c4Count--;
                continue;
            }

            // Strip this guy down to just the base name and replace unwanted chars
            facCQCRemBrws().AdjustFileName(pathCur);
            colRealNames.objAdd(pathCur);
            c4Index++;
        }

        // If any dups, get permission
        tCIDLib::TBoolean bDoAll = kCIDLib::False;
        tCIDWUtils::EOverOpts eAllOpt = tCIDWUtils::EOverOpts::Overwrite;
        TString strCurPath;

        c4Index = 0;
        while (c4Index < c4Count)
        {
            const TString& strCur = colRealNames[c4Index];

            // Create the full path and store it
            strCurPath = m_strDropPath;
            strCurPath.Append(kCIDLib::chForwardSlash);
            strCurPath.Append(strCur);

            //
            //  If this one exists and they've not already told us what to do, then
            //  ask. Else we keep the option they told us to use for all items.
            //
            tCIDWUtils::EOverOpts eCurOpt = eAllOpt;
            if (!bDoAll && bPathExists(strCurPath))
                eCurOpt = facCIDWUtils().eGetOverOpt(*this, bDoAll, strCurPath, kCIDLib::True);

            // If they asked to cancel, then remove all files and break out
            if (eCurOpt == tCIDWUtils::EOverOpts::Cancel)
            {
                m_colDropFiles.RemoveAt(c4Index);
                colRealNames.RemoveAll();
                c4Count = 0;
                break;
            }

            // If we can overwrite, keep this one, else remove it
            if (eCurOpt == tCIDWUtils::EOverOpts::Overwrite)
            {
                c4Index++;
            }
             else
            {
                // Remove this one from both lists so they stay in sync
                m_colDropFiles.RemoveAt(c4Index);
                colRealNames.RemoveAt(c4Index);
                c4Count--;
            }
        }

        // If we ended up with any files, then process them
        if (c4Count)
        {
            //
            //  Find the browser object that owns the drop path and let him handle the
            //  drop.
            //
            TCQCTreeBrwsIntf* pbrwsCur = pbrwsFindOwner(m_strDropPath, kCIDLib::False);
            if (pbrwsCur)
            {
                try
                {
                    pbrwsCur->AcceptFiles(m_strDropPath, colRealNames, m_colDropFiles);
                }

                catch(TError& errToCatch)
                {
                    errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                    TModule::LogEventObj(errToCatch);

                    TErrBox msgbFailed(L"One or more of the files could not be uploaded to CQC");
                    msgbFailed.ShowIt(*this);
                }
            }
             else
            {
                TString strMsg = TString::strConcat(L"There was no browser handler for path: ", m_strDropPath);
                TErrBox msgbFailed(strMsg);
                msgbFailed.ShowIt(*this);
            }
        }
    }
}


tCIDLib::TVoid TCQCTreeBrowser::Destroyed()
{
    // Terminate drag and drop support
    TermDragAndDrop(*this);

    //
    //  Clean up the browser objects now before we delete the tree window, since they
    //  reference it. They would get cleaned up anyway at the end, but just being
    //  safe.
    //
    CleanupBrowsers();

    TParent::Destroyed();
}


//
//  We give the browser handler for the item a chance to provide colors for the
//  items, in order to reflect status that they are keeping up with.
//
//  WE get the user object associated with the path, to hopefully vastly speed up
//  the process. Most likely the browser object won't have to do any looking, he'll
//  look at some flag in the object and know what to do.
//
//  For now, all we allow is to let them indicate whether the item is in a 'non-normal'
//  state, which we'll use to set the text color to red.
//
tCIDCtrls::ETreeCustRets
TCQCTreeBrowser::eCustomDraw(const  TString&            strPath
                            ,       TObject* const      pobjUser
                            , const tCIDLib::TCard4     c4UserFlag
                            , const tCIDLib::TBoolean   bPost
                            , const tCIDLib::TBoolean   bSelected
                            , const tCIDLib::TCard4     c4Level
                            ,       TRGBClr&            rgbBgn
                            ,       TRGBClr&            rgbText)
{
    TCQCTreeBrwsIntf* pbrwsCur = pbrwsFindOwner(strPath, kCIDLib::False);

    // Shouldn't happen, but just in case
    if (!pbrwsCur)
        return tCIDCtrls::ETreeCustRets::None;

    // If the user flag is set, then draw it in a non-normal state
    if (c4UserFlag)
    {
        const tCQCTreeBrws::EItemStates eNewState = tCQCTreeBrws::EItemStates(c4UserFlag);

        if (eNewState == tCQCTreeBrws::EItemStates::Bad)
            rgbText.Set(0xFF, 0, 0);
        else if (eNewState == tCQCTreeBrws::EItemStates::Paused)
            rgbText.Set(0xAA, 0xAA, 0xAA);
        else
            rgbText.Set(0xCC, 0xCC, 0xCC);

        return tCIDCtrls::ETreeCustRets::NewTextClr;
    }
    return tCIDCtrls::ETreeCustRets::None;
}


//
//  We get called here when we get a key stroke that matches the accel table that
//  the browser for the selected item provided to us. So we just turn around and
//  pass it to the browser. It would be nice if we could it directly, but it's just
//  easier to do by allowing the accelerator table to handle the translation and
//  works by calling the target window's menu handler.
//
tCIDLib::TVoid
TCQCTreeBrowser::MenuCommand(const  tCIDLib::TResId     ridItem
                            , const tCIDLib::TBoolean   bChecked
                            , const tCIDLib::TBoolean   bEnabled)
{
    // Get the currently selected item. If none, do nothing
    TString strCurPath;
    if (!bSelectedItem(strCurPath))
        return;

    // See which browser object owns this path. If none, do nothing.
    TCQCTreeBrwsIntf* pbrwsCur = pbrwsFindOwner(strCurPath, kCIDLib::False);
    if (!pbrwsCur)
        return;

    // Pass it on to the browser
    TTreeBrowseInfo wnotToSend;
    if (pbrwsCur->bProcessMenuSelection(ridItem, strCurPath, wnotToSend))
        SendAsyncNotify(new TTreeBrowseInfo(wnotToSend), nidBrowseEvent);
}


// ---------------------------------------------------------------------------
//  TCQCTreeBrowser: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  This is called from some of the drag and drop callbacks. We use the point to get
//  the tree item that the mouse is over. We see what data type it the item
//  represents and whether it's a system or user scope. Then we decide if there's
//  any files in the provided list that could go there.
//
//  For anything in the the system area, we always say no. Otherwise, it's based on
//  having the right type of file for the target and the target being a scope.
//
tCIDLib::TBoolean
TCQCTreeBrowser::bTestCanDrop(  const   TPoint&                 pntScrAt
                                , const tCIDLib::TStrHashSet&   colExts
                                ,       TString&                strPath)
{
    // Convert the point to our coordinates
    TPoint pntAt;
    FromScreenCoordinates(pntScrAt, pntAt);

    // See if we hit anything
    if (!bItemAt(pntAt, strPath))
        return kCIDLib::False;

    // It has to be a scope
    if (!bIsScope(strPath))
        return kCIDLib::False;

    // Find the handler for this path
    const TCQCTreeBrwsIntf* pbrwsCur = pbrwsFindOwner(strPath, kCIDLib::False);
    if (!pbrwsCur)
        return kCIDLib::False;

    // And give it a change to pass judgement
    if (!pbrwsCur->bAcceptsNew(strPath, colExts))
        return kCIDLib::False;

    return kCIDLib::True;
}


//
//  This method checks for queued up changes. These are just passed back to the
//  browser objects that posted them. This lets them handle things in the GUI thread
//  context based on something has happened in a bgn thread they are using to
//  monitor the system in some way. The contents of the change objects are completely
//  up to the browsers since they post them and process them.
//
//  We don't do any sync here, so browsers' bgn threads can continue to add items
//  while we process. The queue is thread safe, and all we are doing here is an
//  atomic check for another item (and the only other things that happens ia an atomic
//  add of items in QueueChange()).
//
tCIDLib::TVoid TCQCTreeBrowser::CheckChangeQ()
{
    // Prevent any re-entry here which can happen
    if (m_bInChangeQ)
        return;
    TBoolJanitor janQ(&m_bInChangeQ, kCIDLib::True);

    // Supress redrawing until we are done
    TWndPaintJanitor janDraw(this);

    //
    //  While there are items in the queue, process them. We do a zero time wait, so
    //  we only get what's there.
    //
    TTreeBrowseChange tbcCur;
    while (m_colChangeQ.bGetNext(tbcCur, 0, kCIDLib::False))
    {
        // Find the browser object that handles the path of this change
        TCQCTreeBrwsIntf* pbrwsCur = pbrwsFindOwner(tbcCur.m_strPath, kCIDLib::False);
        if (pbrwsCur)
        {
            try
            {
                // And pass this change on to him to process
                pbrwsCur->ProcessChange(tbcCur.m_strPath, tbcCur.m_strOpName, tbcCur.m_strData);
            }

            catch(TError& errToCatch)
            {
                if (facCQCTreeBrws().bLogWarnings())
                {
                    errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                    TModule::LogEventObj(errToCatch);
                }
            }
        }
         else
        {
            // Shouldn't happen

        }
    }
}


tCIDLib::TVoid TCQCTreeBrowser::CleanupBrowsers()
{
    tCIDLib::TCard4 c4Count = m_colBrowsers.c4ElemCount();
    while (c4Count)
    {
        c4Count--;
        try
        {
            delete m_colBrowsers.pobjOrphanAt(c4Count);
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        catch(...)
        {
        }
    }
}


//
//  We handle any events sent by our own base class, so that we can process them
//  internally and only expose browser specific notifications to our client code.
//
tCIDCtrls::EEvResponses TCQCTreeBrowser::eTreeHandler(TTreeEventInfo& wnotEvent)
{
    // Find the browser object that handles the indicated path
    const TString& strPath = wnotEvent.strPath();
    TCQCTreeBrwsIntf* pbrwsCur = pbrwsFindOwner(strPath, kCIDLib::False);
    if (!pbrwsCur)
    {
        // If it's not the root path, then something is wrong
        if (strPath != kCQCRemBrws::strPath_Root)
        {
            TErrBox msgbUnhandled
            (
                L"CQC Tree Browser"
                , L"No browser was available to handle the path: " + strPath
            );
            msgbUnhandled.ShowIt(*this);
        }
        return tCIDCtrls::EEvResponses::Handled;
    }

    //
    //  Get the data type of this path. Tell it not to throw if no match, since this
    //  could be the root or a high level scope that is above the typed sections of
    //  the hierarchy.
    //
    const tCQCRemBrws::EDTypes eType = facCQCRemBrws().eXlatPathType(strPath, kCIDLib::False);
    if (wnotEvent.eEvent() == tCIDCtrls::ETreeEvents::Expanding)
    {
        //
        //  They are expanding an item. See if it's been expanded previously. If not,
        //  then we need to fault in the contents and load them up, so pass it on
        //  to the browser handler.
        //
        if (!bHasBeenExpanded(strPath))
            pbrwsCur->LoadScope(strPath);
    }
     else if (wnotEvent.eEvent() == tCIDCtrls::ETreeEvents::Invoked)
    {
        //
        //  If its for a valid data type and the owning browser says it's ok to report it,
        //  we send a notification. He tells us whether to send it as a view or an edit.
        //
        tCIDLib::TBoolean bAsEdit = kCIDLib::False;
        if ((eType != tCQCRemBrws::EDTypes::Count)
        &&  pbrwsCur->bReportInvocation(strPath, bAsEdit))
        {
            tCQCTreeBrws::EEvents eEvType;
            if (bAsEdit)
                eEvType = tCQCTreeBrws::EEvents::Edit;
            else
                eEvType = tCQCTreeBrws::EEvents::View;
            SendAsyncNotify
            (
                new TTreeBrowseInfo(eEvType, strPath, eType, *this), nidBrowseEvent
            );
        }
    }
     else if (wnotEvent.eEvent() == tCIDCtrls::ETreeEvents::Menu)
    {
        //
        //  Pass it to the browser, who knows what the legal options are. If it says
        //  to, we send the returned notification out.
        //
        TTreeBrowseInfo wnotToSend;
        if (pbrwsCur->bDoMenuAction(strPath, wnotToSend))
            SendAsyncNotify(new TTreeBrowseInfo(wnotToSend), nidBrowseEvent);
    }
     else if (wnotEvent.eEvent() == tCIDCtrls::ETreeEvents::Rename)
    {
        //
        //  The user clicked on the current tree item, which we get as a rename
        //  request. First send a sync notification to ask if the containing program
        //  will allow it.
        //
        TTreeBrowseInfo wnotToSend
        (
            tCQCTreeBrws::EEvents::CanRename
            , strPath
            , facCQCRemBrws().eXlatPathType(strPath, kCIDLib::True)
            , *this
        );
        SendSyncNotify(wnotToSend, nidBrowseEvent);

        if (wnotToSend.bFlag())
        {
            // The user said ok. He may not actually do it but we don't care
            pbrwsCur->bDoRename(strPath, wnotToSend);
        }
    }
     else if (wnotEvent.eEvent() == tCIDCtrls::ETreeEvents::SelChanged)
    {
        // We just pass this one on
        TTreeBrowseInfo wnotToSend
        (
            tCQCTreeBrws::EEvents::SelChanged
            , wnotEvent.strOldPath()
            , wnotEvent.strPath()
            , eType
            , *this
        );
        SendAsyncNotify(new TTreeBrowseInfo(wnotToSend), nidBrowseEvent);
    }
    return tCIDCtrls::EEvResponses::Handled;
}


// Loop through the browser objects and find the one that owns this path
TCQCTreeBrwsIntf*
TCQCTreeBrowser::pbrwsFindOwner(const   TString&            strPath
                                , const tCIDLib::TBoolean   bThrowIfNot)
{
    return
    (
        const_cast<TCQCTreeBrwsIntf*>
        (
            const_cast<const TCQCTreeBrowser*>(this)->pbrwsFindOwner
            (
                strPath, bThrowIfNot
            )
        )
    );
}

const TCQCTreeBrwsIntf*
TCQCTreeBrowser::pbrwsFindOwner(const   TString&            strPath
                                , const tCIDLib::TBoolean   bThrowIfNot) const
{
    // If it's the root, then there is no handler
    if (strPath == kCQCRemBrws::strPath_Root)
        return nullptr;

    const TCQCTreeBrwsIntf* pbrwsRet = nullptr;
    const tCIDLib::TCard4 c4Count = m_colBrowsers.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (strPath.bStartsWithI(m_colBrowsers[c4Index]->strRootPath()))
        {
            pbrwsRet = m_colBrowsers[c4Index];
            break;
        }
    }

    if (!pbrwsRet)
    {
        // If asked to throw if not found, then do that
        if (bThrowIfNot)
        {
            facCQCTreeBrws().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kTBrwsErrs::errcBrws_NoHandler
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::NotFound
                , strPath
            );
        }

        TErrBox msgbNotFnd
        (
            L"Could not find the browser object that owned this part of the hierarchy"
        );
        msgbNotFnd.ShowIt(*this);
    }

    return pbrwsRet;
}




// ---------------------------------------------------------------------------
//   CLASS: TCQCTreeTypeBrowser
//  PREFIX: wnd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCTreeTypeBrowser: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCTreeTypeBrowser::TCQCTreeTypeBrowser()
{
}

TCQCTreeTypeBrowser::~TCQCTreeTypeBrowser()
{
}


// ---------------------------------------------------------------------------
//  TCQCTreeTypeBrowser: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TCQCTreeTypeBrowser::CreateTreeTypeBrowser( const   TWindow&                    wndOwner
                                            , const tCQCRemBrws::EDTypes        eDType
                                            , const tCIDCtrls::TWndId           widId
                                            , const TArea&                      areaInit
                                            , const tCIDCtrls::EWndStyles       eStyles
                                            , const tCQCTreeBrws::EBrwsStyles   eBrwsStyles
                                            , const TCQCUserCtx&                cuctxToUse)
{
    // Create a data server browser for the requested data type
    AddBrowser(new TCQCDataSrvBrws(eDType));

    // And now create ourself
    CreateBrowser(wndOwner, widId, areaInit, eStyles, eBrwsStyles, cuctxToUse);

    //
    //  !!! This should IN NO WAY be required, but unless we do this, the /Customize scope
    //  will end up marked as already expanded, despite the fact that we set it to be
    //  virtual and have not in any way changed that. But, if we don't do it, then
    //  bExpandPath not expand it, and will subsequently fail on the next level down.
    //
    SetAutoChild(kCQCRemBrws::strPath_Customize);
}

