//
// FILE NAME: CQCTreeBrws_DataSrvBrws.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/26/2015
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
//  This file implements the remote browser derivative that handles browsing the
//  data server.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCTreeBrws_.hpp"
#include    "CQCTreeBrws_DataSrvBrws_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCDataSrvBrws,TCQCTreeBrwsIntf)


// ---------------------------------------------------------------------------
//  Local types and constants
// ---------------------------------------------------------------------------
namespace CQCTreeBrws_DataSrvBrws
{
    namespace
    {
        //
        //  The clipboard format we use, which is just a flattened path string stored
        //  with this format name.
        //
        constexpr const tCIDLib::TCh* const   pszCBFormat = L"FObj:CQCRemBrowsePath";

        //
        //  Some magic values we add to our popup menu if debugging. We need to make sure
        //  they don't conflict with any of the regular menu ids.
        //
        constexpr tCIDLib::TResId   ridMenu_DumpEvents      = 9999;
    }
};




// ---------------------------------------------------------------------------
//   CLASS: TCQCDataSrvBrws
//  PREFIX: rbrws
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCDataSrvBrws: Constructors and Destructor
// ---------------------------------------------------------------------------

// Sets us up for the default scenario of browsing the whole data server
TCQCDataSrvBrws::TCQCDataSrvBrws(const tCQCRemBrws::EDTypes eFilterType) :

    TCQCTreeBrwsIntf
    (
        kCQCRemBrws::strPath_Customize
        , kCQCRemBrws::strItem_Customize
        , facCQCTreeBrws().strMsg(kTBrwsMsgs::midTitle_CustBrower)
    )
    , m_eFilterType(eFilterType)
{
}

TCQCDataSrvBrws::~TCQCDataSrvBrws()
{
}


// ---------------------------------------------------------------------------
//  TCQCDataSrvBrws: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  Files were dropped on us. We need to find the ones that we can accept and
//  upload them to the server. The parent scope is the full hiearchical path,
//  i.e. /Customize/.... The names are just the basic target name, no path or
//  extension. The source paths list is the list of local file paths to upload.
//
tCIDLib::TVoid
TCQCDataSrvBrws::AcceptFiles(const  TString&            strParScope
                            , const tCIDLib::TStrList&  colTarNames
                            , const tCIDLib::TStrList&  colSrcPaths)
{
    // Get the data type of the parent path
    const tCQCRemBrws::EDTypes eType = facCQCRemBrws().eXlatPathType
    (
        strParScope, kCIDLib::True
    );

    const tCIDLib::TCard4 c4Count = colTarNames.c4ElemCount();

    // Shouldn't happen, but just in case
    if (!c4Count)
        return;

    //
    //  Pull out the extensions in the same order as the files, so that we
    //  can more easily below find the types of each file below and can
    //  ignore any not obviously of the correct type.
    //
    TString  strExt;
    TPathStr pathCur;
    tCIDLib::TStrList colExts(c4Count);
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        pathCur = colSrcPaths[c4Index];
        if (!pathCur.bQueryExt(strExt))
            strExt.Clear();
        colExts.objAdd(strExt);
    }

    // Based on that, find the files we can take and process them
    tCIDLib::TBoolean bUploaded = kCIDLib::False;
    TDataSrvClient dsclUpload;

    if ((eType == tCQCRemBrws::EDTypes::Image)
    ||  (eType == tCQCRemBrws::EDTypes::SchEvent)
    ||  (eType == tCQCRemBrws::EDTypes::TrgEvent))
    {
        tCIDLib::TBoolean bDiscardErrs = kCIDLib::False;
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            try
            {
                const TString& strCurExt = colExts[c4Index];
                if (strCurExt.bCompareI(L"BMP")
                ||  strCurExt.bCompareI(L"PNG")
                ||  strCurExt.bCompareI(L"JPG")
                ||  strCurExt.bCompareI(L"JPEG"))
                {
                    UploadImage
                    (
                        colSrcPaths[c4Index]
                        , strParScope
                        , colTarNames[c4Index]
                        , colExts[c4Index]
                        , dsclUpload
                    );
                    bUploaded = kCIDLib::True;
                }
                 else if (strCurExt.bCompareI(L"CQCSEV")
                      ||  strCurExt.bCompareI(L"CQCTEV"))
                {
                    UploadEvent
                    (
                        colSrcPaths[c4Index]
                        , strParScope
                        , colTarNames[c4Index]
                        , strCurExt.bCompareI(L"CQCTEV")
                        , dsclUpload
                    );
                }
            }

            catch(TError& errToCatch)
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);

                //
                //  If they've not told us to discard all errs, see what they want to do.
                //  We run it in discard mode, since we want to present this to the user
                //  as discarding any failed ones.
                //
                if (!bDiscardErrs)
                {
                    const tCIDWUtils::EErrOpts eRes = facCIDWUtils().eGetErrOpt
                    (
                        wndBrowser()
                        , bDiscardErrs
                        , colSrcPaths[c4Index]
                        , kCIDLib::True
                        , kCIDLib::True
                    );

                    // If they asked to cancel, then stop now
                    if (eRes == tCIDWUtils::EErrOpts::Cancel)
                        break;
                }
            }
        }
    }

    // If we managed to upload something
    if (bUploaded)
    {
        try
        {
            // We need to get the updated parent scope info and update the tree
            tCIDLib::TBoolean   bIsScope;
            TDSBrowseItem       dsbiPar;
            tCIDLib::TCardList  fcolPathIds;
            dsclUpload.QueryBrowserItem(strParScope, fcolPathIds, dsbiPar, bIsScope, sectUser());
            wndBrowser().SetUserObj(strParScope, new TDSBrowseItem(dsbiPar));
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);

            facCQCGKit().ShowError
            (
                wndBrowser(), L"Upload Error", errToCatch
            );
        }
    }
}


//
//  Return true if this path is capable of accepting a new child item. This is for supporting
//  drag and drop.
//
tCIDLib::TBoolean
TCQCDataSrvBrws::bAcceptsNew(const  TString&                strPath
                            , const tCIDLib::TStrHashSet&   colExts) const
{
    //
    //  If we are filtered, then we never accept anything since we are being used in the
    //  file selection dialog, not in the main browser tab.
    //
    if (m_eFilterType != tCQCRemBrws::EDTypes::Count)
        return kCIDLib::False;

    // For us, it has to be a user scope
    const TDSBrowseItem& dsbiPath
    (
        *static_cast<const TDSBrowseItem*>(wndBrowser().pobjUser(strPath, kCIDLib::True))
    );
    if (!dsbiPath.bIsUserScope())
        return kCIDLib::False;

    //
    //  It's possibly valid, so let's loo at the data type and then check if
    //  there's any files we can accept.
    //
    tCIDLib::TBoolean bRet = kCIDLib::False;
    switch(dsbiPath.eType())
    {
        case tCQCRemBrws::EDTypes::Image :
            // Just see if are getting any images
            bRet =  colExts.bHasElement(L"PNG")
                    || colExts.bHasElement(L"JPG")
                    || colExts.bHasElement(L"JPEG")
                    || colExts.bHasElement(L"BMP");
            break;

        case tCQCRemBrws::EDTypes::SchEvent :
            bRet = colExts.bHasElement(L"CQCSEV");
            break;

        case tCQCRemBrws::EDTypes::TrgEvent :
            bRet = colExts.bHasElement(L"CQCTEV");
            break;

        default :
            bRet = kCIDLib::False;
            break;
    };
    return bRet;
}


//
//  The browser window calls us here if the user invokes a menu operation on the
//  tree window.
//
tCIDLib::TBoolean
TCQCDataSrvBrws::bDoMenuAction( const   TString&            strPath
                                ,       TTreeBrowseInfo&    wnotToSend)
{
    // If read only, or the user is less than a power user, then don't allow this
    if (tCIDLib::bAllBitsOn(wndBrowser().eTreeStyles(), tCIDCtrls::ETreeStyles::ReadOnly)
    ||  (cuctxUser().eUserRole() < tCQCKit::EUserRoles::PowerUser))
    {
        return kCIDLib::False;
    }

    // Get the area of this item. Tell it to use just the text width
    TArea areaItem;
    wndBrowser().bQueryItemArea(strPath, areaItem, kCIDLib::True, kCIDLib::True);

    // Get the center point of it and convert to screen coordinates
    TPoint pntAt;
    wndBrowser().ToScreenCoordinates(areaItem.pntCenter(), pntAt);

    //
    //  Create the menu and load it up from the resource. Note that we load a simpler
    //  menu if we are in single data type mode.
    //
    TPopupMenu menuAction(L"/Customize Action");
    menuAction.Create
    (
        facCQCTreeBrws()
        , (m_eFilterType == tCQCRemBrws::EDTypes::Count) ? kCQCTreeBrws::ridMenu_DSrvAct
                                                        : kCQCTreeBrws::ridMenu_DSrvAct2
    );

    //
    //  We store a remote browser info on our tree items, so get it out and cast to
    //  the real type.
    //
    const TDSBrowseItem& dsbiPath
    (
        *static_cast<const TDSBrowseItem*>(wndBrowser().pobjUser(strPath, kCIDLib::True))
    );

    //
    //  If debugging and this is an event scope and we are in the full browser, not the
    //  filtered version, then add a magic one to tell the server to dump his event data
    //  for analysis.
    //
    #if CID_DEBUG_ON
    if (((dsbiPath.eType() == tCQCRemBrws::EDTypes::EvMonitor)
    ||   (dsbiPath.eType() == tCQCRemBrws::EDTypes::SchEvent)
    ||   (dsbiPath.eType() == tCQCRemBrws::EDTypes::TrgEvent))
    &&  (m_eFilterType == tCQCRemBrws::EDTypes::Count))
    {
        menuAction.AddActionItem
        (
            kCIDLib::c4MaxCard
            , L"DumpEvents"
            , L"Event Server Dump"
            , CQCTreeBrws_DataSrvBrws::ridMenu_DumpEvents
        );
    }
    #endif

    // Set up the menu enable/disable states based on the browser item info
    SetupMenu(menuAction, strPath, dsbiPath);

    // Get a selection from the user
    const tCIDLib::TCard4 c4Res = menuAction.c4Process
    (
        wndBrowser(), pntAt, tCIDLib::EVJustify::Bottom
    );

    //
    //  If they made a choice, then we have to translate it to the standard action
    //  enum that the browser window will understand.
    //
    if (c4Res)
        return bProcessMenuSelection(c4Res, strPath, wnotToSend);

    // Say we didn't do anything
    return kCIDLib::False;
}


// We don't have a persistent connection so just say yes
tCIDLib::TBoolean TCQCDataSrvBrws::bIsConnected() const
{
    return kCIDLib::True;
}


//
//  In the case of CML macros, we have a special concern when renaming, in that we don't
//  want to allow any spaces, so we override this. We call our parent and, if he fails, we
//  just give up. If he passes, then we see if it's a macro, and check it further.
//
tCIDLib::TBoolean
TCQCDataSrvBrws::bIPEValidate(  const   TString&            strSrc
                                ,       TAttrData&          adatTar
                                , const TString&            strNewVal
                                ,       TString&            strErrMsg
                                ,       tCIDLib::TCard8&    c8UserId) const
{
    if (!TParent::bIPEValidate(strSrc, adatTar, strNewVal, strErrMsg, c8UserId))
        return kCIDLib::False;

    //
    //  If a macro, do more checking. Doesn't matter if it's a scope or item, they all
    //  have to be limited the same way. The parent scope is set in the special type member
    //  so we have to pull the two out and create the whole path to check.
    //
    TString strCheck = adatTar.strSpecType();
    facCQCRemBrws().AddPathComp(strCheck, adatTar.strValue());

    tCQCRemBrws::EDTypes eType = facCQCRemBrws().eXlatPathType(strCheck);
    if (eType == tCQCRemBrws::EDTypes::Macro)
    {
        tCIDLib::TCard4 c4At;
        if (strCheck.bFirstOccurrence(kCIDLib::szWhitespace, c4At, kCIDLib::True))
        {
            strErrMsg = L"CML class paths cannot contain any spaces";
            return kCIDLib::False;
        }
    }
    return kCIDLib::True;
}


//
//  Our own bDoMenuAction calls this if a selection is made. It is also called by the
//  browser window if an accelerator driven command is seen. That's why it's split out
//  so that we can avoid duplicating this code.
//
tCIDLib::TBoolean
TCQCDataSrvBrws::bProcessMenuSelection( const   tCIDLib::TCard4     c4CmdId
                                        , const TString&            strPath
                                        ,       TTreeBrowseInfo&    wnotToSend)
{
    //
    //  If read only, then don't allow this unless it's a non-modifying operation.
    //
    if (tCIDLib::bAllBitsOn(wndBrowser().eTreeStyles(), tCIDCtrls::ETreeStyles::ReadOnly)
    &&  (c4CmdId != kCQCTreeBrws::ridMenu_DSrvAct_Copy)
    &&  (c4CmdId != kCQCTreeBrws::ridMenu_DSrvAct_Export)
    &&  (c4CmdId != kCQCTreeBrws::ridMenu_DSrvAct_View))
    {
        return kCIDLib::False;
    }

    //
    //  Get the browse info for the path. Get a copy since some of the things we do here
    //  could destroy the object and that could lead to accessing it after it's deleted.
    //
    const TDSBrowseItem dsbiPath
    (
        *static_cast<const TDSBrowseItem*>(wndBrowser().pobjUser(strPath, kCIDLib::True))
    );

    tCIDLib::TBoolean bRet = kCIDLib::False;
    switch(c4CmdId)
    {
        case kCQCTreeBrws::ridMenu_DSrvAct_Copy :
        {
            //
            //  This is always possible. What we do is just store the path as a
            //  flattened object with a custom format name.
            //
            TBinMBufOutStream strmCopy(1024UL, 4096UL);
            strmCopy << strPath << kCIDLib::FlushIt;

            TGUIClipboard gclipCopy(wndBrowser());
            gclipCopy.Clear();
            gclipCopy.StoreFlatObject
            (
                CQCTreeBrws_DataSrvBrws::pszCBFormat
                , strmCopy.mbufData()
                , strmCopy.c4CurSize()
            );

            // Also, for convenience store the raw text
            gclipCopy.StoreText(strPath);
            break;
        }

        case kCQCTreeBrws::ridMenu_DSrvAct_Edit :
            // Just send an event so the client code can edit if desired
            wnotToSend = TTreeBrowseInfo
            (
                tCQCTreeBrws::EEvents::Edit
                , strPath
                , dsbiPath.eType()
                , wndBrowser()
            );
            bRet = kCIDLib::True;
            break;

        case kCQCTreeBrws::ridMenu_DSrvAct_Delete :
            //
            //  We can handle this internally. If the user commits and it works,
            //  then set up a deletion notification so that the client code can
            //  do something about it.
            //
            bRet = bDeleteFile(strPath, dsbiPath);
            if (bRet)
            {
                wnotToSend = TTreeBrowseInfo
                (
                    tCQCTreeBrws::EEvents::Deleted
                    , strPath
                    , dsbiPath.eType()
                    , wndBrowser()
                );
            }
            break;

        case kCQCTreeBrws::ridMenu_DSrvAct_Export :
            // Call a helper to do the import
            bRet = bExportFiles(strPath,  dsbiPath.eType());
            break;

        case kCQCTreeBrws::ridMenu_DSrvAct_Import :
            // Call a helper to do the import
            bRet = bImportFiles(strPath, dsbiPath.eType());
            break;

        case kCQCTreeBrws::ridMenu_DSrvAct_NewFile :
        {
            // Call a helper to create a new file
            tCIDLib::TKVPFList colXMeta;
            bRet = bMakeNewFile(strPath, dsbiPath.eType(), colXMeta, wnotToSend);
            break;
        }

        case kCQCTreeBrws::ridMenu_DSrvAct_NewScope :
            // We handle this one internally
            AddScope(strPath, kCIDLib::False);
            break;

        case kCQCTreeBrws::ridMenu_DSrvAct_Paste :
        {
            // Call a helper to do this
            TString strSrcPath;
            bRet = bPastePath(strPath, dsbiPath, strSrcPath);
            if (bRet)
            {
                wnotToSend = TTreeBrowseInfo
                (
                    tCQCTreeBrws::EEvents::Paste
                    , strSrcPath
                    , strPath
                    , dsbiPath.eType()
                    , wndBrowser()
                );
            }
            break;
        }

        case kCQCTreeBrws::ridMenu_DSrvAct_Rename :
            // Call our parent's rename method which does what we want
            bRet = bDoRename(strPath, wnotToSend);
            break;

        case kCQCTreeBrws::ridMenu_DSrvAct_Refresh :
            // We handle this internally
            UpdateScope(strPath);
            break;

        case kCQCTreeBrws::ridMenu_DSrvAct_View :
            // Just send an event so the client code can view if desired
            wnotToSend = TTreeBrowseInfo
            (
                tCQCTreeBrws::EEvents::View, strPath, dsbiPath.eType(), wndBrowser()
            );
            bRet = kCIDLib::True;
            break;


        // These are only valid for events
        case kCQCTreeBrws::ridMenu_DSrvAct_Pause :
        case kCQCTreeBrws::ridMenu_DSrvAct_Resume :
            //
            //  We handle these internally, though we will send a notification in
            //  case the containing appilcation wants to react in some way. We pass
            //  true if we are going to pause it
            //
            bRet = bPauseResumeEvent
            (
                strPath, dsbiPath, c4CmdId == kCQCTreeBrws::ridMenu_DSrvAct_Pause, wnotToSend
            );
            break;

        case kCQCTreeBrws::ridMenu_DSrvAct_CreatePack :
            // Let the contained application do this if he wants
            wnotToSend = TTreeBrowseInfo
            (
                tCQCTreeBrws::EEvents::Package, strPath, dsbiPath.eType(), wndBrowser()
            );
            bRet = kCIDLib::True;
            break;

        case CQCTreeBrws_DataSrvBrws::ridMenu_DumpEvents :
        {
            // One of our magic, debugging only ones
            tCQCEvCl::TEventSrvProxy orbcES = facCQCEvCl().orbcEventSrvProxy();

            TPathStr pathTarFile;
            TProcEnvironment::bFind(L"CQC_RESDIR", pathTarFile);
            pathTarFile.AddLevel(L"EventSrvDump");
            pathTarFile.AppendExt(L"Txt");
            orbcES->DumpEvents(pathTarFile);
            break;
        }

        default :
            bRet = kCIDLib::False;
            break;
    };
    return bRet;
}


//
//  If it's a non-scope path, then it's ok to report it via invocation. If it's a system
//  item, we report it as a view operation, else as an edit.
//
tCIDLib::TBoolean
TCQCDataSrvBrws::bReportInvocation(const TString& strPath, tCIDLib::TBoolean& bAsEdit) const
{
    const TDSBrowseItem& dsbiPath
    (
        *static_cast<const TDSBrowseItem*>(wndBrowser().pobjUser(strPath, kCIDLib::True))
    );

    // If it's a scope, then obviously not
    if (dsbiPath.bIsScope())
        return kCIDLib::False;

    bAsEdit = dsbiPath.bCanInvoke();
    return kCIDLib::True;
}


//
//  We just get our client proxy object set up, and we add our top level scope
//  to the tree browser. After that, it's all faulted in upon access by the user.
//
tCIDLib::TVoid TCQCDataSrvBrws::Initialize(const TCQCUserCtx& cuctxUser)
{
    TParent::Initialize(cuctxUser);

    // Add our top level scope marked as virtual
    wndBrowser().AddScope
    (
        kCQCRemBrws::strPath_Root, kCQCRemBrws::strItem_Customize, kCIDLib::True
    );

    // Put some basic info on it. We won't ever use it
    wndBrowser().SetUserObj
    (
        kCQCRemBrws::strPath_Customize
        , new TDSBrowseItem
          (
            kCQCRemBrws::strPath_Customize
            , tCQCRemBrws::EDTypes::Count
            , 0
            , 0
            , tCQCKit::EUserRoles::LimitedUser
            , tCQCRemBrws::EItemFlags::SpecialScope
          )
    );
}


tCIDLib::TVoid TCQCDataSrvBrws::LoadScope(const TString& strPath)
{
    // Set up the base path for the filter type, if we have one
    const TString& strTypeBase =
    (
        (m_eFilterType == tCQCRemBrws::EDTypes::Count)
        ? TString::strEmpty() : facCQCRemBrws().strFlTypePref(m_eFilterType)
    );
    const tCIDLib::TCard4 c4TypeBaseLen = strTypeBase.c4Length();

    //
    //  If the path to load is shorter than the filter path base, then the filter path
    //  must starts with it. If the path is longer than the filter base, then the path
    //  must start with the filter base.
    //
    if (m_eFilterType != tCQCRemBrws::EDTypes::Count)
    {
        if (((strPath.c4Length() <= c4TypeBaseLen) && !strTypeBase.bStartsWithI(strPath))
        ||  ((strPath.c4Length() > c4TypeBaseLen) && !strPath.bStartsWithI(strTypeBase)))
        {
            wndBrowser().UpdateChildCnt(strPath);
            return;
        }
    }

    // Get a short cut to the browser window
    TCQCTreeBrowser& wndTar = wndBrowser();
    try
    {
        // Ask the data server for the items in this scope
        TVector<TDSBrowseItem>  colItems;
        tCIDLib::TCardList      fcolPathIds;
        TDataSrvClient          dsclToUse;
        dsclToUse.QueryScopeItems(strPath, fcolPathIds, colItems, kCIDLib::False, sectUser());

        // If we got any load them
        TString strChildPath;
        const tCIDLib::TCard4 c4Count = colItems.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const TDSBrowseItem& dsbiCur = colItems[c4Index];

            // Build up the full path to this one
            strChildPath = strPath;
            strChildPath.Append(kCIDLib::chForwardSlash);
            strChildPath.Append(dsbiCur.strName());

            //
            //  As above, if the path is <= the filter base path, then the filter path
            //  must start with the path. If the path is longer than the filter path, then
            //  the path must start with the filter path.
            //
            if (m_eFilterType != tCQCRemBrws::EDTypes::Count)
            {
                if (((strChildPath.c4Length() <= c4TypeBaseLen) && !strTypeBase.bStartsWithI(strChildPath))
                ||  ((strChildPath.c4Length() > c4TypeBaseLen) && !strChildPath.bStartsWithI(strTypeBase)))
                {
                    continue;
                }
            }

            if (dsbiCur.bIsScope())
            {
                wndTar.AddScope(strPath, dsbiCur.strName(), kCIDLib::True);
            }
             else
            {
                wndTar.AddItem(strPath, dsbiCur.strName());

                // If the type flag is set, then mark it paused
                if (dsbiCur.bUserFlag())
                    wndBrowser().SetItemStatus(strChildPath, tCQCTreeBrws::EItemStates::Paused);
            }

            //
            //  Store the browser info object on the tree item so that we can get back
            //  to it later.
            //
            wndTar.SetUserObj(strChildPath, new TDSBrowseItem(dsbiCur));
        }

        //
        //  Probably this happened, if we added any above. But, if the scope was
        //  empty, or they all got filtered, then we need to do this just in case,
        //  to clear set the child count info, which also turns off the virtual
        //  scope flag, so we won't try to fault this one back in.
        //
        wndTar.UpdateChildCnt(strPath);

        //
        //  At the end of an expansion, force the expanded once state on. It
        //  won't get done if this is a lazily faulted in tree and the expanded
        //  scope ended up being empty. That will cause lots of problems later.
        //
        wndTar.ForceExpandedOnce(strPath);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);
        TExceptDlg dlgErr(wndBrowser(), L"Could not expand item", errToCatch);
    }
}


//
//  We generate a default initial name for an item under the indicted scope.
//  We have to insure it's unique as well.
//
tCIDLib::TVoid
TCQCDataSrvBrws::MakeDefName(const  TString&            strParScope
                            ,       TString&            strToFill
                            , const tCIDLib::TBoolean   bIsFile) const
{
    // Figure out the data type
    tCQCRemBrws::EDTypes eType = facCQCRemBrws().eXlatPathType(strParScope);

    //
    //  We will have to repeatedly build up the path to test it for uniqueness.
    //
    TString strDefName;
    if (bIsFile)
    {
        strDefName = tCQCRemBrws::strXlatEDTypes(eType);
        strDefName.StripWhitespace(tCIDLib::EStripModes::Complete);
    }
     else
    {
        strDefName = L"Scope";
    }

    TString strCurPath(strParScope);
    strCurPath.Append(kCIDLib::chForwardSlash);
    strCurPath.Append(strDefName);

    //
    //  Now we add numbers to it until we get a unique name not already in this
    //  scope.
    //
    const tCIDLib::TCard4 c4BaseLen = strCurPath.c4Length();
    tCIDLib::TCard4 c4Num = 1;
    while (kCIDLib::True)
    {
        strCurPath.CapAt(c4BaseLen);
        strCurPath.AppendFormatted(c4Num);

        if (!wndBrowser().bPathExists(strCurPath))
        {
            strToFill = strDefName;
            strToFill.AppendFormatted(c4Num);
            break;
        }

        // Not unique, to try another
        c4Num++;
    }
}


//
//  If the browser window gets an accelerator key translation call, he will
//  call us here to load up an accelerator table for him which he will process.
//  If it causes him to get a menu call, he will pass it on to us.
//
tCIDLib::TVoid
TCQCDataSrvBrws::QueryAccelTable(const  TString&        strPath
                                ,       TAccelTable&    accelToFill) const
{
    //
    //  Just load it up from our menu. So we just create an instance of our
    //  menu but never show it.
    //
    TPopupMenu menuAction(L"/Customize Action");
    menuAction.Create(facCQCTreeBrws(), kCQCTreeBrws::ridMenu_DSrvAct);

    // Get the browser data info for the passed path
    const TDSBrowseItem& dsbiPath
    (
        *static_cast<const TDSBrowseItem*>(wndBrowser().pobjUser(strPath, kCIDLib::True))
    );

    //
    //  Use that to set up the menu. This way, any actions that are not legal
    //  for this item are disabled in the manu, and so won't be added as accel
    //  keys. Otherwise we'd have to check the validity of the commands again
    //  when bProcessMenuSelection is called.
    //
    SetupMenu(menuAction, strPath, dsbiPath);

    // And now set up the accel table from the menu
    accelToFill.Create(menuAction);
}


//
//  The main application has saved something. If it's an event, we need to handle
//  that specially and update the event server.
//
tCIDLib::TVoid TCQCDataSrvBrws::UpdateFile(const TString& strHPath)
{
    // Get the type and relative path
    TString strRelPath;
    const tCQCRemBrws::EDTypes eDType = facCQCRemBrws().eConvertPath
    (
        strHPath, kCIDLib::False, strRelPath, kCIDLib::False
    );

    // We have to convert this into the event server type for below
    tCQCKit::EEvSrvTypes eEvType = facCQCEvCl().eBrwsDTypeToEvType(eDType, kCIDLib::False);
    if (eEvType != tCQCKit::EEvSrvTypes::None)
    {
        try
        {
            tCQCEvCl::TEventSrvProxy orbcES = facCQCEvCl().orbcEventSrvProxy(3000);
            tCIDLib::TCard4 c4SerialNum = 0;
            orbcES->UpdateEvent(eEvType, strRelPath, c4SerialNum, sectUser());
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);

            facCQCGKit().ShowError
            (
                wndBrowser(), L"Could not update the event server", errToCatch
            );
        }
    }
}


// Nothing to do for now
tCIDLib::TVoid TCQCDataSrvBrws::Terminate()
{
}


// ---------------------------------------------------------------------------
//  TCQCDataSrvBrws: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TCQCDataSrvBrws::bCanRename(const TString& strPath) const
{
    // Get the info we stored on the tree item and return its rename flag
    const TDSBrowseItem& dsbiPath
    (
        *static_cast<const TDSBrowseItem*>(wndBrowser().pobjUser(strPath, kCIDLib::True))
    );
    return dsbiPath.bCanRename();
}



tCIDLib::TBoolean
TCQCDataSrvBrws::bRenameItem(const  TString&            strParScope
                            , const TString&            strOldName
                            , const TString&            strNewName
                            , const tCIDLib::TBoolean   bIsScope)
{
    TDSBrowseItem* pdsbiTar = nullptr;
    TString strRelParScope;
    try
    {
        // Find the browser info on this path
        pdsbiTar =
        (
            static_cast<TDSBrowseItem*>(wndBrowser().pobjUser(strParScope, kCIDLib::True))
        );

        // Get the relative parent path
        facCQCRemBrws().CreateRelPath(strParScope, pdsbiTar->eType(), strRelParScope);

        tCIDLib::TCard4 c4ParScopeId;
        tCIDLib::TCard4 c4ItemId;
        TDataSrvClient  dsclToUse;
        tCIDLib::ERenameRes eRes = dsclToUse.eRename
        (
            strRelParScope
            , strOldName
            , strNewName
            , pdsbiTar->eType()
            , bIsScope
            , c4ParScopeId
            , c4ItemId
            , sectUser()
        );

        //
        //  Update the ids on the stuff in the tree we changed. In our case we
        //  know it's a rename within the same scope, since that's all our tree
        //  browser allows. So we don't have to remove any items or move any. We
        //  just update the scope id of the parent scope and the item id of the
        //  target item.
        //
        pdsbiTar->c4IdScope(c4ParScopeId);

        // Set up the old path so we can look it up and update it
        TString strOldPath(strParScope);
        strOldPath.Append(kCIDLib::chForwardSlash);
        strOldPath.Append(strOldName);

        // Get the user data first and update the item id and display name
        pdsbiTar =
        (
            static_cast<TDSBrowseItem*>(wndBrowser().pobjUser(strOldPath, kCIDLib::True))
        );
        pdsbiTar->c4IdItem(c4ItemId);
        pdsbiTar->strName(strNewName);

        // And now tell the tree to update the display text of the item
        wndBrowser().UpdateItem(strParScope, strOldName, strNewName);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        facCQCGKit().ShowError
        (
            wndBrowser(), L"Could not expand item", errToCatch
        );
        return kCIDLib::False;
    }

    //
    //  If this is a an event, then we need to tell the event server about it. We
    //  have to get the relative parent path, and the event type.
    //
    tCQCKit::EEvSrvTypes eEvType = facCQCEvCl().eBrwsDTypeToEvType
    (
        pdsbiTar->eType(), kCIDLib::False
    );
    if (eEvType != tCQCKit::EEvSrvTypes::None)
    {
        try
        {
            tCQCEvCl::TEventSrvProxy orbcES = facCQCEvCl().orbcEventSrvProxy(2000);
            tCIDLib::TCard4 c4SerialNum = 0;
            if (bIsScope)
            {
                orbcES->RenameScope
                (
                    eEvType
                    , strRelParScope
                    , strOldName
                    , strNewName
                    , c4SerialNum
                    , sectUser()
                );
            }
             else
            {
                orbcES->RenameEvent
                (
                    eEvType
                    , strRelParScope
                    , strOldName
                    , strNewName
                    , c4SerialNum
                    , sectUser()
                );
            }
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);

            facCQCGKit().ShowError
            (
                wndBrowser()
                , L"The event server could not be updated, so it may be out of sync "
                  L"with the event configuration."
                , errToCatch
            );
        }
    }
    return kCIDLib::True;
}


//
//  This is called to upload a triggered or scheduled events. The code is almost all the
//  same. This is not to create new ones, so we assume it exists and tell the server
//  that overwrite is OK.
//
tCIDLib::TVoid
TCQCDataSrvBrws::UploadEvent(const  TString&            strSrcPath
                            , const TString&            strParScope
                            , const TString&            strTarName
                            , const tCIDLib::TBoolean   bTrigType
                            ,       TDataSrvClient&     dsclToUse)
{
    TString strHPath(strParScope);
    strHPath.Append(kCIDLib::chForwardSlash);
    strHPath.Append(strTarName);

    //
    //  And we need the file type relative version to use with the data server
    //  client.
    //
    TString strRelPath;
    facCQCRemBrws().CreateRelPath
    (
        strHPath
        , bTrigType ? tCQCRemBrws::EDTypes::TrgEvent : tCQCRemBrws::EDTypes::SchEvent
        , strRelPath
    );

    try
    {
        // Create an input stream over the source path
        TBinFileInStream strmSrc
        (
            strSrcPath
            , tCIDLib::ECreateActs::OpenIfExists
            , tCIDLib::EFilePerms::Default
            , tCIDLib::EFileFlags::SequentialScan
        );

        //
        //  We have to stream in the object to upload it. This has the benefit of proving it
        //  is valid before we foist it on the server.
        //
        tCIDLib::TCard4         c4SerialNum;
        tCIDLib::TKVPFList      colExtraMeta;
        tCIDLib::TEncodedTime   enctLastChange;
        if (bTrigType)
        {
            TCQCTrgEvent csrcNew;
            strmSrc >> csrcNew;
            dsclToUse.WriteTriggeredEvent
            (
                strRelPath
                , c4SerialNum
                , enctLastChange
                , csrcNew
                , kCQCRemBrws::c4Flag_OverwriteOK
                , colExtraMeta
                , sectUser()
            );
        }
         else
        {
            TCQCSchEvent csrcNew;
            strmSrc >> csrcNew;
            dsclToUse.WriteScheduledEvent
            (
                strRelPath
                , c4SerialNum
                , enctLastChange
                , csrcNew
                , kCQCRemBrws::c4Flag_OverwriteOK
                , colExtraMeta
                , sectUser()
            );
        }

        // We need to query back data server browser info to set on the tree item
        tCIDLib::TBoolean   bIsScope;
        TDSBrowseItem       dsbiNew;
        tCIDLib::TCardList  fcolPathIds;
        dsclToUse.QueryBrowserItem(strHPath, fcolPathIds, dsbiNew, bIsScope, sectUser());

        //
        //  It didn't fail, so if it's not a replacement, we need to add it to the
        //  tree control. We need to query the browse info for the path one way or
        //  another, to update an existing one.
        //
        if (!wndBrowser().bPathExists(strHPath))
            wndBrowser().AddItem(strParScope, dsbiNew.strName());
        wndBrowser().SetUserObj(strHPath, new TDSBrowseItem(dsbiNew));
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}



//
//  This is called from our accept files handler, if images where dropped on the
//  browser window. It's called once per file dropped. We let exceptions propagate
//  back up to the caller.
//
tCIDLib::TVoid
TCQCDataSrvBrws::UploadImage(const  TString&            strSrcPath
                            , const TString&            strParScope
                            , const TString&            strTarName
                            , const TString&            strExt
                            ,       TDataSrvClient&     dsclToUse)
{
    //
    //  Load up the data to a PNG image. This will handle any conversion for us,
    //  leaving us with the PNG type we need to upload.
    //
    TPNGImage imgLoad;
    facCIDImgFact().bLoadToPNG(strSrcPath, imgLoad, kCIDLib::True);

    //
    //  We need to create a thumb image and get both images flattend out for sending.
    //  CQCKit provides a helper for this. Use the reported image size as a rough guide
    //  to the size of the output stream needed.
    //
    const tCIDLib::TCard4 c4Sz(imgLoad.c4ImageSz());
    TBinMBufOutStream strmImg(c4Sz + 4096, (c4Sz + 2096) * 2);
    TBinMBufOutStream strmThumb(c4Sz / 2, c4Sz);
    TSize szThumb;
    facCQCKit().PackageImg(imgLoad, strmImg, strmThumb, szThumb);

    // Build up the full heirarchical target path
    TString strHPath(strParScope);
    strHPath.Append(kCIDLib::chForwardSlash);
    strHPath.Append(strTarName);

    //
    //  And we need the file type relative version to use with the data server
    //  client.
    //
    TString strRelPath;
    facCQCRemBrws().CreateRelPath(strHPath, tCQCRemBrws::EDTypes::Image, strRelPath);

    // And let's try to upload it
    try
    {
        tCIDLib::TKVPFList      colExtraMeta;
        tCIDLib::TCard4         c4NewSerialNum = 0;
        tCIDLib::TEncodedTime   enctLastChange;
        dsclToUse.WriteImage
        (
            strRelPath
            , c4NewSerialNum
            , enctLastChange
            , strmImg.mbufData()
            , strmImg.c4CurSize()
            , strmThumb.mbufData()
            , strmThumb.c4CurSize()
            , imgLoad.ePixFmt()
            , imgLoad.eBitDepth()
            , imgLoad.szImage()
            , szThumb
            , kCQCRemBrws::c4Flag_OverwriteOK
            , colExtraMeta
            , sectUser()
        );

        // We need to query back data server browser info to set on the tree item
        tCIDLib::TBoolean   bIsScope;
        TDSBrowseItem       dsbiNew;
        tCIDLib::TCardList  fcolPathIds;
        dsclToUse.QueryBrowserItem(strHPath, fcolPathIds, dsbiNew, bIsScope, sectUser());

        //
        //  It didn't fail, so if it's not a replacement, we need to add it to the
        //  tree control. We need to query the browse info for the path one way or
        //  another, to update an existing one.
        //
        if (!wndBrowser().bPathExists(strHPath))
            wndBrowser().AddItem(strParScope, dsbiNew.strName());
        wndBrowser().SetUserObj(strHPath, new TDSBrowseItem(dsbiNew));
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}


// ---------------------------------------------------------------------------
//  TCQCDataSrvBrws: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  We get the path that the menu was invoked for. We create a default new name that
//  won't clash and add that as a sub-scope. The user can then edit it as desired.
//
tCIDLib::TVoid
TCQCDataSrvBrws::AddScope(  const   TString&            strParHPath
                            , const tCIDLib::TBoolean   bVirtual)
{
    try
    {
        //
        //  Figure out the data type for the parent scope. The user can't create any
        //  paths high enough in the hierarchy that it's above the typed parts, so it
        //  should always map to a type. And we need to get the relative path for use
        //  below, and this does both.
        //
        TString strRelParPath;
        const tCQCRemBrws::EDTypes eDType = facCQCRemBrws().eConvertPath
        (
            strParHPath, kCIDLib::True, strRelParPath, kCIDLib::False
        );

        // We need the browser a number of times so get a convenient ref
        TCQCTreeBrowser& wndTar = wndBrowser();

        //
        //  The first thing we need to do is to get a name from the user. We call
        //  a helper on the browser window do this since it's sort of messy. If they
        //  don't commit, we return and nothing has been done. Indicate it's a scope.
        //
        TString strNewName;
        const tCIDLib::TBoolean bRes = wndTar.bGetNewName
        (
            strParHPath, kCIDLib::True, eDType, strNewName, *this
        );

        if (!bRes)
            return;

        // Build up the new full hierarchical path
        TString strNewHPath(strParHPath);
        facCQCRemBrws().AddPathComp(strNewHPath, strNewName);

        //
        //  At this point the item is in the tree. We need to make sure it gets removed
        //  if we don't explicitly decide to keep it.
        //
        TTreeItemJan janTree(&wndTar, strNewHPath);

        //
        //  Tell the data server to create this scope. This is a typed call so it
        //  takes the relative path.
        //
        TDataSrvClient dsclToUse;
        dsclToUse.MakeNewScope(strRelParPath, strNewName, eDType, sectUser());

        // Now query info info for that item and its parent scope
        tCIDLib::TBoolean   bIsScope;
        TDSBrowseItem       dsbiParScope;
        TDSBrowseItem       dsbiTarget;
        tCIDLib::TCardList  fcolPathIds;
        dsclToUse.QueryBrowserItem2
        (
            strNewHPath, fcolPathIds, dsbiTarget, dsbiParScope, bIsScope, sectUser()
        );

        CIDAssert
        (
            bIsScope, L"AddScope got a file when it queried the path info"
        );

        // Now set the new data server info on both parent and new child
        wndBrowser().SetUserObj(strParHPath, new TDSBrowseItem(dsbiParScope));
        wndBrowser().SetUserObj(strNewHPath, new TDSBrowseItem(dsbiTarget));

        // Looks like it survived, so orphan from the tree janitor
        janTree.Orphan();
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        facCQCGKit().ShowError
        (
            wndBrowser(), L"Add Scope Error", errToCatch
        );
    }
}


//
//  We handle deletes internally and will just send a notification to the client
//  if it goes through, so that they can do anything if needed. We wouldn't get
//  called here if this item wasn't marked as deletable.
//
//  If we return true, the caller will send a notification that this target was
//  deleted.
//
tCIDLib::TBoolean
TCQCDataSrvBrws::bDeleteFile(const TString& strHPath, const TDSBrowseItem&  dsbiPath)
{
    TCQCTreeBrowser& wndTar = wndBrowser();

    // Check with the containing application if it's ok to delete this
    const tCIDLib::TBoolean bIsOpen = bSendQuestionNot
    (
        tCQCTreeBrws::EEvents::IsOpen, strHPath, dsbiPath.eType(), kCIDLib::False
    );

    //
    //  If the passed path is a scope, then let's warn the user about that and
    //  give him a chance to back out. Else, just do a confirmation of the delete
    //  operation.
    //
    if (dsbiPath.bIsScope())
    {
        //
        //  Ask the containing application if there is anything open in this scope or any
        //  nested scope. If so, we tell them it can't be done. Else we ask them if they
        //  really want to do it.
        //
        if (bIsOpen)
        {
            TErrBox msgbBusy(facCQCTreeBrws().strMsg(kTBrwsErrs::errcBrws_ScopeIsBusy));
            msgbBusy.ShowIt(wndTar);
            return kCIDLib::False;
        }
         else
        {
            TString strMsg = facCQCTreeBrws().strMsg(kTBrwsMsgs::midQ_DeleteScope);
            strMsg.Append(L"\n\n");
            strMsg.Append(strHPath);
            TYesNoBox msgbConfirm(strTitle(), strMsg);
            if (!msgbConfirm.bShowIt(wndTar))
                return kCIDLib::False;
        }
    }
     else
    {
        TYesNoBox msgbConfirm
        (
            strTitle()
            , facCQCTreeBrws().strMsg
              (
                bIsOpen ? kTBrwsMsgs::midQ_DeleteOpenItem : kTBrwsMsgs::midQ_DeleteItem
                , strHPath
              )
        );
        if (!msgbConfirm.bShowIt(wndTar))
            return kCIDLib::False;
    }

    //
    //  They agreed, so let's try it. Copy the info object first, because this is going
    //  to destroy it.
    //
    try
    {
        // Get the type relative path
        TString strRelPath;
        facCQCRemBrws().CreateRelPath(strHPath, dsbiPath.eType(), strRelPath);

        TDataSrvClient dsclToUse;
        if (dsbiPath.bIsScope())
            dsclToUse.DeleteScope(strRelPath, dsbiPath.eType(), sectUser());
        else
            dsclToUse.DeleteFile(strRelPath, dsbiPath.eType(), sectUser());

        // It didn't fail, so remove this item from the browser window
        wndTar.RemoveItem(strHPath);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        facCQCGKit().ShowError
        (
            wndBrowser(), L"Could not delete item", errToCatch
        );
        return kCIDLib::False;
    }

    //
    //  That worked. If this is an event, then we need to update the event server as
    //  well.
    //
    const tCQCKit::EEvSrvTypes eEvType = facCQCEvCl().eBrwsDTypeToEvType
    (
        dsbiPath.eType(), kCIDLib::False
    );

    if ((eEvType == tCQCKit::EEvSrvTypes::EvMonitor)
    ||  (eEvType == tCQCKit::EEvSrvTypes::SchEvent)
    ||  (eEvType == tCQCKit::EEvSrvTypes::TrgEvent))
    {
        try
        {
            tCQCEvCl::TEventSrvProxy orbcES = facCQCEvCl().orbcEventSrvProxy(2000);

            //
            //  If a scope, just tell the server to reload his list for this type
            //  of events. Else just tell it to delete the specific event.
            //
            if (dsbiPath.bIsScope())
            {
                orbcES->ReloadList(eEvType);
            }
             else
            {
                // Get the type relative path
                TString strRelPath;
                facCQCRemBrws().CreateRelPath(strHPath, dsbiPath.eType(), strRelPath);

                tCIDLib::TCard4 c4SerialNum = 0;
                orbcES->DeleteEvent(eEvType, strRelPath, c4SerialNum, sectUser());
            }
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);

            facCQCGKit().ShowError
            (
                wndBrowser(), L"Could not update the event server.", errToCatch
            );
        }
    }
    return kCIDLib::True;
}


//
//  This is called when the user askes to export files. Currently this is only supported
//  for triggered/scheduled events. The path can only be a single file currently, not a
//  whole scope or sub-tree. We get the full hierarchical path.
//
tCIDLib::TBoolean
TCQCDataSrvBrws::bExportFiles(  const   TString&                strHPath
                                , const tCQCRemBrws::EDTypes    eDType)
{
    // Not used in this case, but have to pass one
    tCIDLib::TKVPList colTypes(1);

    // Get a target scope to save to, else bail out if no selection
    TPathStr pathOutScope;
    {
        tCIDLib::TStrList colSelList;
        const tCIDLib::TBoolean bGotSel = facCIDCtrls().bOpenFileDlg
        (
            wndBrowser()
            , L"Select Target Folder"
            , TString::strEmpty()
            , colSelList
            , colTypes
            , tCIDLib::eOREnumBits
              (
                tCIDCtrls::EFOpenOpts::SelectFolders
                , tCIDCtrls::EFOpenOpts::PathMustExist
              )
        );

        if (!bGotSel)
            return kCIDLib::False;

        pathOutScope = colSelList[0];
    }


    //
    //  We need the relative path for most of the below, make sure it ends in a slash
    //  if it's a scope. It could be a single incoming file.
    //
    TString strRelPath;
    facCQCRemBrws().CreateRelPath(strHPath, eDType, strRelPath);
    if (wndBrowser().bIsScope(strHPath))
    {
        if (strRelPath.chLast() != kCIDLib::chForwardSlash)
            strRelPath.Append(kCIDLib::chForwardSlash);
    }

    //
    //  Create the list of events to export. If it's just one, then just add that,
    //  else query the list of items under the scope. So below we can just act on a
    //  list no matter what.
    //
    tCIDLib::TStrList colSrcEvs;
    {
        if (wndBrowser().bIsScope(strHPath))
        {
            wndBrowser().QueryChildren(strHPath, colSrcEvs, kCIDLib::True);

            //
            //  This only gets us the base names, so create the full relative paths
            //  paths, to match what we get if it's a single event being passed in.
            //
            const tCIDLib::TCard4 c4Count = colSrcEvs.c4ElemCount();
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
                colSrcEvs[c4Index].Insert(strRelPath, 0);
        }
         else
        {
            colSrcEvs.objAdd(strRelPath);
        }
    }

    tCIDLib::TCard4         c4ErrCnt = 0;
    tCIDLib::TBoolean       bDoAll = kCIDLib::False;
    tCIDWUtils::EErrOpts    eErrOpt = tCIDWUtils::EErrOpts::Cancel;
    TBinMBufOutStream       strmTar(8 * 1024UL);

    tCIDLib::TCard4         c4SerialNum;
    TCQCSchEvent            csrcSched;
    TCQCTrgEvent            csrcTrig;
    TDataSrvClient          dsclExport;
    tCIDLib::TEncodedTime   enctLast;
    TString                 strName;
    TString                 strPathPart;
    TPathStr                pathOut;

    const tCIDLib::TCard4 c4Count = colSrcEvs.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        tCIDLib::TCard4 c4Bytes = 0;
        strmTar.Reset();

        try
        {
            tCIDLib::TKVPFList colMeta;
            if (eDType == tCQCRemBrws::EDTypes::TrgEvent)
            {
                dsclExport.bReadTrigEvent
                (
                    colSrcEvs[c4Index]
                    , c4SerialNum
                    , enctLast
                    , csrcTrig
                    , colMeta
                    , sectUser()
                );
                strmTar << csrcTrig << kCIDLib::FlushIt;
                c4Bytes = strmTar.c4CurSize();
            }
             else if (eDType == tCQCRemBrws::EDTypes::SchEvent)
            {
                dsclExport.bReadSchedEvent
                (
                    colSrcEvs[c4Index]
                    , c4SerialNum
                    , enctLast
                    , csrcSched
                    , colMeta
                    , sectUser()
                );
                strmTar << csrcSched << kCIDLib::FlushIt;
                c4Bytes = strmTar.c4CurSize();
            }
             else
            {
                CIDAssert2(L"Only triggered or scheduled events should be seen here");
            }

            //
            //  We have to add the type relative path of the event to the base output
            //  target scope. We have to flip the separators of the relative path part.
            //  We don't do AddLevel() here because the slashes in the relative part are
            //  the other way so it will add an extra one. So we just append the relative
            //  part then replace the slashes.
            //
            pathOut = pathOutScope;
            pathOut.Append(colSrcEvs[c4Index]);
            pathOut.bReplaceChar(kCIDLib::chForwardSlash, kCIDLib::chPathSep);
            pathOut.AppendExt(facCQCRemBrws().strFlTypeExt(eDType));

            // Now we have to grab just the path part and make sure that path exists
            pathOut.bQueryPath(strPathPart);
            TFileSys::MakePath(strPathPart);

            // Now create an output file stream and stream this guy out
            TBinaryFile bflOut(pathOut);
            bflOut.Open
            (
                tCIDLib::EAccessModes::Write
                , tCIDLib::ECreateActs::CreateAlways
                , tCIDLib::EFilePerms::Default
                , tCIDLib::EFileFlags::SequentialScan
            );

            if (bflOut.c4WriteBuffer(strmTar.mbufData(), c4Bytes) != c4Bytes)
            {
                facCQCTreeBrws().ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kTBrwsErrs::errcExp_NotAllBytes
                    , colSrcEvs[c4Index]
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::CantDo
                );
            }
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);

            // If the do all flag is not set, then get the error option for this error
            c4ErrCnt++;
            if (!bDoAll)
            {
                eErrOpt = facCIDWUtils().eGetErrOpt
                (
                    wndBrowser()
                    , bDoAll
                    , L"Export triggered event"
                    , kCIDLib::True
                    , kCIDLib::False
                );

                // If they asked to cancel, then give up
                if (eErrOpt == tCIDWUtils::EErrOpts::Cancel)
                    break;
            }
        }
    }

    if (c4ErrCnt)
    {
        TString strMsg;
        if (colSrcEvs.c4ElemCount() == 1)
            strMsg = L"The event could not be exported";
        else if (c4ErrCnt == colSrcEvs.c4ElemCount())
            strMsg = L"All export targets failed";
        else
            strMsg = L"Some export targets failed";

        TErrBox msgbFailed(strMsg);
        msgbFailed.ShowIt(wndBrowser());
    }
     else
    {
        TOkBox msgbOK(L"All events were exported successfully");
        msgbOK.ShowIt(wndBrowser());
    }


    // Don't send any notifications since this doesn't change the tree a
    return kCIDLib::True;
}


//
//  This is called when the user asks to import files. Currently this is only supported
//  for images and scheduled/triggered events, but might be for other things later.
//
//  We get the list of files, and then we just call the same method that gets called
//  if files are dropped on us, since it already handles exactly what we want to do.
//
tCIDLib::TBoolean
TCQCDataSrvBrws::bImportFiles(  const   TString&                strParHPath
                                , const tCQCRemBrws::EDTypes    eDType)
{
    // Set up the types we can accept
    tCIDLib::TKVPList colTypes(6);
    colTypes.objPlace(L"All Images", L"*.bmp; *.jpg; *.jpeg; *.png");
    colTypes.objPlace(L"Bitmap Files", L"*.bmp");
    colTypes.objPlace(L"JPEG Files", L"*.jpg; *.jpeg");
    colTypes.objPlace(L"PNG Files", L"*.png");
    colTypes.objPlace(L"Scheduled Events", L"*.CQCSEv");
    colTypes.objPlace(L"Triggered Events", L"*.CQCTEv");

    //
    //  Invoke the file open dialog to let the user select files. We enable it for
    //  multi-selection of course.
    //
    tCIDLib::TStrList colSelFiles;
    const tCIDLib::TBoolean bGotFiles = facCIDCtrls().bOpenFileDlg
    (
        wndBrowser()
        , L"Select Files to Import"
        , TString::strEmpty()
        , colSelFiles
        , colTypes
        , tCIDLib::eOREnumBits
          (
            tCIDCtrls::EFOpenOpts::FileMustExist
            , tCIDCtrls::EFOpenOpts::StrictTypes
            , tCIDCtrls::EFOpenOpts::MultiSelect
          )
    );

    // If any were selected, let's process them
    if (bGotFiles)
    {
        //
        //  We pass two lists, the new names we want them known as when imported and
        //  the original source paths. So the source paths is just what we got back
        //  from the file selection dialog. We have to create the target names list.
        //  The remote browser facility has a helper to create the target name, so this
        //  is rpetty easy.
        //
        tCIDLib::TStrList colTarNames;

        TPathStr pathCur;
        const tCIDLib::TCard4 c4SelCount = colSelFiles.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4SelCount; c4Index++)
        {
            pathCur = colSelFiles[c4Index];
            facCQCRemBrws().AdjustFileName(pathCur);
            colTarNames.objAdd(pathCur);
        }

        AcceptFiles(strParHPath, colTarNames, colSelFiles);
    }

    //
    //  Indicate we don't want to send any notification. We are just adding new files
    //  to the browser, which the containing application doesn't need to react to.
    //
    return kCIDLib::False;
}


//
//  This is called to create a new file. We look at the type and call another helper
//  that handles that particular type. If they create a file, then they return true
//  and we fill in the notification to send. The browser window will send this along
//  to any listeners so that they can react to this.
//
tCIDLib::TBoolean
TCQCDataSrvBrws::bMakeNewFile(  const   TString&                strParHPath
                                , const tCQCRemBrws::EDTypes    eDType
                                , const tCIDLib::TKVPFList&     colExtraMeta
                                ,       TTreeBrowseInfo&        wnotToSend)
{
    // We need the browser a number of times so get a convenient ref
    TCQCTreeBrowser& wndTar = wndBrowser();

    //
    //  The first thing we need to do is to get a name from the user. We call
    //  a helper on the browser window do this since it's sort of messy. IF they
    //  don't commit, we return and nothing has been done. Indicate it's an item,
    //  not a scope.
    //
    TString strNewName;
    const tCIDLib::TBoolean bRes =  wndTar.bGetNewName
    (
        strParHPath, kCIDLib::False, eDType, strNewName, *this
    );

    if (!bRes)
        return kCIDLib::False;

    // Build up the new full hierarchical path
    TString strNewHPath(strParHPath);
    facCQCRemBrws().AddPathComp(strNewHPath, strNewName);

    //
    //  At this point the item is in the tree. We need to make sure it gets removed
    //  if we don't explicitly decide to keep it.
    //
    TTreeItemJan janTree(&wndTar, strNewHPath);

    // Get the type relative versions of both path
    TString strParRelPath;
    facCQCRemBrws().CreateRelPath(strParHPath, eDType, strParRelPath);

    // And the relative version of that
    TString strNewRelPath;
    facCQCRemBrws().CreateRelPath(strNewHPath,eDType,strNewRelPath);

    tCQCKit::EEvSrvTypes eEvType = tCQCKit::EEvSrvTypes::None;
    try
    {
        tCIDLib::TBoolean bRet = kCIDLib::False;
        switch(eDType)
        {
            case tCQCRemBrws::EDTypes::EvMonitor :
            {
                TCQCEvMonCfg emoncNew;
                emoncNew.Set
                (
                    L"MEng.User.EvMons.MyMon"
                    , L"This is a new event monitor"
                    , TString::strEmpty()
                );
                emoncNew.bPaused(kCIDLib::True);
                tCIDLib::TCard4 c4SerialNum = 0;
                tCIDLib::TEncodedTime enctLastChange;
                TDataSrvClient dsclWrite;

                dsclWrite.WriteEventMon
                (
                    strNewRelPath
                    , c4SerialNum
                    , enctLastChange
                    , emoncNew
                    , kCQCRemBrws::c4Flag_None
                    , colExtraMeta
                    , sectUser()
                );
                eEvType = tCQCKit::EEvSrvTypes::EvMonitor;
                break;
            }

            case tCQCRemBrws::EDTypes::GlobalAct :
            {
                TCQCStdCmdSrc csrcNew(L"Default Title");
                csrcNew.AddEvent
                (
                    facCQCKit().strMsg(kKitMsgs::midCmdEv_OnTrigger)
                    , kCQCKit::strEvId_OnTrigger
                    , tCQCKit::EActCmdCtx::Standard
                );
                tCIDLib::TCard4 c4SerialNum = 0;
                tCIDLib::TEncodedTime enctLastChange;
                TDataSrvClient dsclWrite;

                dsclWrite.WriteGlobalAction
                (
                    strNewRelPath
                    , c4SerialNum
                    , enctLastChange
                    , csrcNew
                    , kCQCRemBrws::c4Flag_None
                    , colExtraMeta
                    , sectUser()
                );
                break;
            }

            case tCQCRemBrws::EDTypes::Macro :
            {
                // Take the relative path and make it a dotted style path
                TString strDotted;
                facCQCRemBrws().RelPathToCMLClassPath(strNewRelPath, strDotted);

                //
                //  Now get the default basic macro class template and replace a token
                //  with the dotted class path.
                //
                TString strMacroSrc(facCIDMacroEng().strEmptyClassTemplate());
                strMacroSrc.eReplaceToken(strDotted, L'1');

                tCIDLib::TCard4 c4SerialNum = 0;
                tCIDLib::TEncodedTime enctLastChange;
                TDataSrvClient dsclWrite;
                dsclWrite.WriteMacro
                (
                    strNewRelPath
                    , c4SerialNum
                    , enctLastChange
                    , strMacroSrc
                    , kCQCRemBrws::c4Flag_None
                    , colExtraMeta
                    , sectUser()
                );
                break;
            }

            case tCQCRemBrws::EDTypes::SchEvent :
            {
                TCQCSchEvent csrcNew(L"Default Title");
                csrcNew.bPaused(kCIDLib::True);
                tCIDLib::TCard4 c4SerialNum = 0;
                tCIDLib::TEncodedTime enctLastChange;
                TDataSrvClient dsclWrite;

                dsclWrite.WriteScheduledEvent
                (
                    strNewRelPath
                    , c4SerialNum
                    , enctLastChange
                    , csrcNew
                    , kCQCRemBrws::c4Flag_None
                    , colExtraMeta
                    , sectUser()
                );
                bRet = kCIDLib::True;
                eEvType = tCQCKit::EEvSrvTypes::SchEvent;
                break;
            }

            case tCQCRemBrws::EDTypes::Template :
                // This one is more complex so we use a helper
                bRet = bMakeNewTemplate(strParHPath, strParRelPath, strNewName, colExtraMeta);
                break;

            case tCQCRemBrws::EDTypes::TrgEvent :
            {
                //
                //  Let's create a simple default one. None of the info is really
                //  valid, it's just placeholder stuff. Set it intially paused
                //
                TCQCTrgEvent csrcNew(L"Default Title", tCQCKit::EActCmdCtx::Standard);
                csrcNew.bPaused(kCIDLib::True);
                tCIDLib::TCard4 c4SerialNum = 0;
                tCIDLib::TEncodedTime enctLastChange;
                TDataSrvClient dsclWrite;

                dsclWrite.WriteTriggeredEvent
                (
                    strNewRelPath
                    , c4SerialNum
                    , enctLastChange
                    , csrcNew
                    , kCQCRemBrws::c4Flag_None
                    , colExtraMeta
                    , sectUser()
                );
                bRet = kCIDLib::True;
                eEvType = tCQCKit::EEvSrvTypes::TrgEvent;
                break;
            }

            default :
            {
                // Not something we can edit
                TErrBox msgbView
                (
                    strTitle()
                    , facCQCTreeBrws().strMsg
                    (
                        kTBrwsMsgs::midStatus_CantNew
                        , tCQCRemBrws::strLoadEDTypes(eDType)
                        , strNewHPath
                    )
                );
                msgbView.ShowIt(wndTar);
                return kCIDLib::False;
            }
        }

        // It appears to have worked, so let the item remain
        janTree.Orphan();

        //
        //  Query the browser info for the (newly affected) parent and the new file
        //  item.
        //
        tCIDLib::TBoolean   bIsScope;
        TDSBrowseItem       dsbiFile;
        TDSBrowseItem       dsbiParScope;
        tCIDLib::TCardList  fcolPathIds;
        TDataSrvClient      dsclNew;
        dsclNew.QueryBrowserItem2
        (
            strNewHPath, fcolPathIds, dsbiFile, dsbiParScope, bIsScope, sectUser()
        );
        CIDAssert(!bIsScope, L"Newly created file came back as a scope");
        CIDAssert(dsbiFile.eType() == eDType, L"New file came back as a different type");

        // Set the browse item info on the new guy
        wndTar.SetUserObj(strNewHPath, new TDSBrowseItem(dsbiFile));

        //
        //  For now, the user flag is being used to indicate paused state for events.
        //  It's not currently used for anything else, so this is safe enough for now
        //  without insuring it's an event.
        //
        if (dsbiFile.bUserFlag())
            wndTar.SetItemStatus(strNewHPath, tCQCTreeBrws::EItemStates::Paused);

        // And we need to update the parent scope's browser info
        wndTar.SetUserObj(strParHPath, new TDSBrowseItem(dsbiParScope));
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        TErrBox msgbNew
        (
            strTitle()
            , facCQCRemBrws().strMsg(kTBrwsMsgs::midStatus_CantCreateFile, strNewHPath)
        );
        msgbNew.ShowIt(wndTar);
        return kCIDLib::False;
    }

    // Select this new guy
    wndTar.SelectPath(strNewHPath);

    // If it was an event, then we need to
    if (eEvType != tCQCKit::EEvSrvTypes::None)
    {
        try
        {
            // Get an event server proxy
            tCQCEvCl::TEventSrvProxy orbcES = facCQCEvCl().orbcEventSrvProxy(2000);

            tCIDLib::TCard4 c4SerialNum = 0;
            orbcES->AddEvent(eEvType, strNewRelPath, c4SerialNum, sectUser());
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);

            facCQCGKit().ShowError
            (
                wndTar, L"Could not update the event server", errToCatch
            );
        }
    }

    // And let's fill in the notification to be sent to the containing application
    wnotToSend = TTreeBrowseInfo
    (
        tCQCTreeBrws::EEvents::NewFile, strNewHPath, eDType, wndTar
    );

    return kCIDLib::True;
}



//
//  This is called to create a new template. We generate an empty template, save it
//  which also adds it to the tree view, and then we select it.
//
tCIDLib::TBoolean
TCQCDataSrvBrws::bMakeNewTemplate(  const   TString&            strParHPath
                                    , const TString&            strParRelPath
                                    , const TString&            strNewName
                                    , const tCIDLib::TKVPFList& colExtraMeta)
{
    // Create the relative path
    TString strNewRelPath(strParRelPath);
    strNewRelPath.Append(kCIDLib::chForwardSlash);
    strNewRelPath.Append(strNewName);

    //
    //  Create an empty template. We have to set the template path on it. That's
    //  all the setup we need to create an initial one.
    //
    TCQCIntfTemplate iwdgNew;
    iwdgNew.strTemplateName(strNewRelPath);

    // Flatten it to a buffer
    THeapBuf mbufTmpl(32* 1024UL);
    tCIDLib::TCard4 c4TmplBytes;
    {
        TBinMBufOutStream strmTar(&mbufTmpl);
        strmTar << iwdgNew << kCIDLib::FlushIt;
        c4TmplBytes = strmTar.c4CurSize();
    }

    tCIDLib::TCard4 c4SerialNum = 0;
    tCIDLib::TEncodedTime enctLastChange;
    TDataSrvClient dsclWrite;
    dsclWrite.WriteTemplate
    (
        strNewRelPath
        , c4SerialNum
        , enctLastChange
        , mbufTmpl
        , c4TmplBytes
        , tCQCKit::EUserRoles::LimitedUser
        , TString::strEmpty()
        , kCQCRemBrws::c4Flag_None
        , colExtraMeta
        , sectUser()
    );

    return kCIDLib::True;
}


//
//  If we get a request from the user to pause or resume an event, this is called.
//  We update the configuration
//
tCIDLib::TBoolean
TCQCDataSrvBrws::bPauseResumeEvent( const   TString&            strHPath
                                    , const TDSBrowseItem&      dsbiPath
                                    , const tCIDLib::TBoolean   bPause
                                    ,       TTreeBrowseInfo&    wnotToSend)
{
    //
    //  Let's read in the event, update it and save it back. The event client
    //  facility provides a helper for this, since it involves a fair bit of code.
    //  We lock the bgn thread so that we can update his data as we do this.
    //
    try
    {
        // We need to get the relative path
        TString strRelPath;
        facCQCRemBrws().CreateRelPath(strHPath, dsbiPath.eType(), strRelPath);

        tCIDLib::TCard4 c4SerialNum = 0;
        tCIDLib::TEncodedTime enctLastChange;
        facCQCEvCl().SetEvPauseState
        (
            facCQCEvCl().eBrwsDTypeToEvType(dsbiPath.eType(), kCIDLib::True)
            , strRelPath
            , c4SerialNum
            , enctLastChange
            , bPause
            , sectUser()
        );

        //
        //  We set the tree item's status. Note that, if we resume it, it could still
        //  fail. If that happens, our bgn thread will see that and update it again.
        //  It will see this change as well, but this just makes the update happen
        //  quicker.
        //
        wndBrowser().SetItemStatus
        (
            strHPath
            , bPause ? tCQCTreeBrws::EItemStates::Paused
                     : tCQCTreeBrws::EItemStates::Normal
        );

        //
        //  It worked so let's send a notification. We use the flag field to indicate
        //  paused or resumed (paused is true.)
        //
        wnotToSend = TTreeBrowseInfo
        (
            tCQCTreeBrws::EEvents::PausedResumed
            , strHPath
            , dsbiPath.eType()
            , wndBrowser()
        );
        wnotToSend.bFlag(bPause);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        facCQCGKit().ShowError
        (
            wndBrowser(), L"Could not pause/resume the event server", errToCatch
        );
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


//
//  This is called on a paste operation. Though it shouldn't happen unless there's
//  the right stuff on the clipboard, we still need to check it.
//
tCIDLib::TBoolean
TCQCDataSrvBrws::bPastePath(const   TString&        strTarPath
                            , const TDSBrowseItem&  dsbiPath
                            ,       TString&        strSrcPath)
{
    try
    {
        TGUIClipboard gclipCopy(wndBrowser());
        THeapBuf mbufData;
        tCIDLib::TCard4 c4Bytes;
        if (!gclipCopy.bGetAsFlatObject(CQCTreeBrws_DataSrvBrws::pszCBFormat, mbufData, c4Bytes))
            return kCIDLib::False;

        // The type we expect is there, so stream it out
        TString strSrcPath;
        {
            TBinMBufInStream strmSrc(&mbufData, c4Bytes);
            strmSrc >> strSrcPath;
        }

        // See if the src is a scope or file
        const tCIDLib::TBoolean bSrcScope = wndBrowser().bIsScope(strSrcPath);

        //
        //  Let's find a unique name in the target, if the original name already exists.
        //  We'll ask the server to create it under this name.
        //
        TString strNamePart;
        facCQCRemBrws().QueryNamePart(strSrcPath, strNamePart);

        TString strNewTar(strTarPath);
        facCQCRemBrws().AddPathComp(strNewTar, strNamePart);
        if (wndBrowser().bPathExists(strNewTar))
        {
            const tCIDLib::TCard4 c4BaseLen = strNewTar.c4Length();
            tCIDLib::TCard4 c4Num = 1;
            while (kCIDLib::True)
            {
                strNewTar.CapAt(c4BaseLen);
                strNewTar.AppendFormatted(c4Num);

                if (!wndBrowser().bPathExists(strNewTar))
                    break;

                // Not unique, to try another
                c4Num++;
            }
        }

        //
        //  The server has a command to do all of this atomically, to copy the
        //  source to the new target.
        //
        TDataSrvClient dsclPaste;
        dsclPaste.PastePath(strSrcPath, strNewTar, bSrcScope, sectUser());

        // It apparently worked, so refresh the target path to pick up the changes
        UpdateScope(strTarPath);

        //
        //  We need to update the event server if what we pasted was event stuff. We
        //  tell him to reload his list for the indicated type of event. This is the
        //  simplest, though not most efficient, way of insuring he gets back into
        //  sync, given that we could have pasted a whole tree of events, and insures
        //  it happens atomically.
        //
        const tCQCRemBrws::EDTypes eDType = facCQCRemBrws().eXlatPathType(strSrcPath);
        const tCQCKit::EEvSrvTypes eEvType
        (
            facCQCEvCl().eBrwsDTypeToEvType(eDType, kCIDLib::False)
        );

        if (eEvType != tCQCKit::EEvSrvTypes::None)
        {
            tCQCEvCl::TEventSrvProxy orbcES = facCQCEvCl().orbcEventSrvProxy(2000);
            orbcES->ReloadList(eEvType);
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);
        TExceptDlg dlgErr
        (
            wndBrowser(), L"Could not paste item", errToCatch
        );
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


//
//  Based on the info in the passed data server browser item object, we enable or disable
//  items in the passed menu. We break this out because it's needed both for processing
//  our popup menu itself, but also so that we can provide accel table setup, which we
//  drive from the menu itself as well.
//
//  Note that we have two menus, one is used in the main browser window and the other is
//  used when we are in single resource mode and provides a limited set of functions. So
//  we have to be sure here not to try to modify menu items that don't exist.
//
tCIDLib::TVoid
TCQCDataSrvBrws::SetupMenu(         TMenu&          menuTar
                            , const TString&        strPath
                            , const TDSBrowseItem&  dsbiTar) const
{
    const tCIDLib::TBoolean bEventType
    (
        (dsbiTar.eType() == tCQCRemBrws::EDTypes::EvMonitor)
        || (dsbiTar.eType() == tCQCRemBrws::EDTypes::SchEvent)
        || (dsbiTar.eType() == tCQCRemBrws::EDTypes::TrgEvent)
    );


    //
    //  These are all only available if there's no filter, i.e. in the main browser
    //  window.
    //
    if (m_eFilterType == tCQCRemBrws::EDTypes::Count)
    {
        // Disable items based on the browser item attributes of the selected item
        if (!dsbiTar.bCanEdit())
            menuTar.SetItemEnable(kCQCTreeBrws::ridMenu_DSrvAct_Edit, kCIDLib::False);

        //
        //  View is only valid if it's an item and it's of type image or macro since for
        //  now those are the only types that have a separate view mode. The others are edit
        //  only.
        //
        //  We may add viewing of templates in the admin client, but not so far
        //
        menuTar.SetItemEnable
        (
            kCQCTreeBrws::ridMenu_DSrvAct_View
            , !dsbiTar.bIsScope()
              && ((dsbiTar.eType() == tCQCRemBrws::EDTypes::Image)
              ||  (dsbiTar.eType() == tCQCRemBrws::EDTypes::Macro))
        );

        //
        //  We special case events, which allow pause/resume. None of our other stuff
        //  supports pause/resume. And only events themselves, not the scopes that contain
        //  them.
        //
        if (bEventType && !dsbiTar.bIsScope())
        {
            menuTar.SetItemEnable(kCQCTreeBrws::ridMenu_DSrvAct_Pause, kCIDLib::True);
            menuTar.SetItemEnable(kCQCTreeBrws::ridMenu_DSrvAct_Resume, kCIDLib::True);
        }
         else
        {
            menuTar.SetItemEnable(kCQCTreeBrws::ridMenu_DSrvAct_Pause, kCIDLib::False);
            menuTar.SetItemEnable(kCQCTreeBrws::ridMenu_DSrvAct_Resume, kCIDLib::False);
        }

        //
        //  If it's a not template, image, or macro or not in the user area, we disable the
        //  package item.
        //
        if ((!dsbiTar.bIsUserItem() && !dsbiTar.bIsUserScope())
        ||  ((dsbiTar.eType() != tCQCRemBrws::EDTypes::Image)
        &&   (dsbiTar.eType() != tCQCRemBrws::EDTypes::Macro)
        &&   (dsbiTar.eType() != tCQCRemBrws::EDTypes::Template)))
        {
            menuTar.SetItemEnable(kCQCTreeBrws::ridMenu_DSrvAct_CreatePack, kCIDLib::False);
        }

        //
        //  The import is only available if the selected type is an image or triggered or
        //  scheduled event, and this is somewhere it's legal to create new files.
        //
        if (((dsbiTar.eType() == tCQCRemBrws::EDTypes::Image)
        ||   (dsbiTar.eType() == tCQCRemBrws::EDTypes::SchEvent)
        ||   (dsbiTar.eType() == tCQCRemBrws::EDTypes::TrgEvent))
        &&  dsbiTar.bCanCreateNew())
        {
             menuTar.SetItemEnable(kCQCTreeBrws::ridMenu_DSrvAct_Import, kCIDLib::True);
        }
         else
        {
            menuTar.SetItemEnable(kCQCTreeBrws::ridMenu_DSrvAct_Import, kCIDLib::False);
        }

        //
        //  The export is only available currently for triggered/scheduled events
        //
        if ((dsbiTar.eType() == tCQCRemBrws::EDTypes::SchEvent)
        ||  (dsbiTar.eType() == tCQCRemBrws::EDTypes::TrgEvent))
        {
             menuTar.SetItemEnable(kCQCTreeBrws::ridMenu_DSrvAct_Export, kCIDLib::True);
        }
        else
        {
            menuTar.SetItemEnable(kCQCTreeBrws::ridMenu_DSrvAct_Export, kCIDLib::False);
        }
    }

    //
    // These are available regardless of the menu we are using
    //

    // If it's not legal to copy this guy, then disable copy
    if (!dsbiTar.bCanCopy())
        menuTar.SetItemEnable(kCQCTreeBrws::ridMenu_DSrvAct_Copy, kCIDLib::False);

    // If this is a valid target for a paste, then let's see if we can do that
    if (dsbiTar.bCanPaste())
    {
        TGUIClipboard gclipCopy(wndBrowser());

        // See if there's a flattened object on the clip board
        tCIDLib::TBoolean bEnable = gclipCopy.bIsFormatAvailable
        (
            tCIDCtrls::EClipFormats::FlatObj
        );

        // There is, so see if it's one of our paths
        TString strSrcPath;
        if (bEnable)
        {
            tCIDLib::TCard4 c4Size;
            THeapBuf mbufData(1024UL, 4096UL);
            bEnable = gclipCopy.bGetAsFlatObject
            (
                CQCTreeBrws_DataSrvBrws::pszCBFormat, mbufData, c4Size
            );

            if (bEnable)
            {
                try
                {
                    TBinMBufInStream strmSrc(&mbufData, c4Size);
                    strmSrc >> strSrcPath;
                }

                catch(TError& errToCatch)
                {
                    errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                    TModule::LogEventObj(errToCatch);

                    bEnable = kCIDLib::False;
                }
            }
        }

        //
        //  If there is, and it still exists, make sure the data type of the target is the
        //  same data type as for that of the source.
        //
        if (bEnable)
        {
            const TDSBrowseItem* pdsbiSrc
            (
                static_cast<const TDSBrowseItem*>
                (
                    wndBrowser().pobjUser(strSrcPath, kCIDLib::False)
                )
            );
            bEnable = (pdsbiSrc != nullptr) && (pdsbiSrc->eType() == dsbiTar.eType());
        }

        // And finally set the menu item
        menuTar.SetItemEnable(kCQCTreeBrws::ridMenu_DSrvAct_Paste, bEnable);
    }
     else
    {
        menuTar.SetItemEnable(kCQCTreeBrws::ridMenu_DSrvAct_Paste, kCIDLib::False);
    }

    if (!dsbiTar.bCanDelete())
        menuTar.SetItemEnable(kCQCTreeBrws::ridMenu_DSrvAct_Delete, kCIDLib::False);

    if (!dsbiTar.bCanRename())
        menuTar.SetItemEnable(kCQCTreeBrws::ridMenu_DSrvAct_Rename, kCIDLib::False);

    //
    //  For images, we cannot create new ones. They can drag and drop but can't just
    //  create an image, since there'd be no way to modify it once they did.
    //
    if ((!dsbiTar.bCanCreateNew())
    ||  (dsbiTar.eType() == tCQCRemBrws::EDTypes::Image))
    {
        menuTar.SetItemEnable(kCQCTreeBrws::ridMenu_DSrvAct_NewFile, kCIDLib::False);
    }

    //
    //  If the user is not an admin, we don't allow new, delete, paste or rename of events.
    if (bEventType && (cuctxUser().eUserRole() != tCQCKit::EUserRoles::SystemAdmin))
    {
        menuTar.SetItemEnable(kCQCTreeBrws::ridMenu_DSrvAct_Delete, kCIDLib::False);
        menuTar.SetItemEnable(kCQCTreeBrws::ridMenu_DSrvAct_NewFile, kCIDLib::False);
        menuTar.SetItemEnable(kCQCTreeBrws::ridMenu_DSrvAct_NewScope, kCIDLib::False);
        menuTar.SetItemEnable(kCQCTreeBrws::ridMenu_DSrvAct_Paste, kCIDLib::False);
        menuTar.SetItemEnable(kCQCTreeBrws::ridMenu_DSrvAct_Rename, kCIDLib::False);
    }

    // Refresh only valid on scopes
    menuTar.SetItemEnable(kCQCTreeBrws::ridMenu_DSrvAct_Refresh, dsbiTar.bIsScope());

    if (!dsbiTar.bCanCreateNew())
        menuTar.SetItemEnable(kCQCTreeBrws::ridMenu_DSrvAct_NewScope, kCIDLib::False);
}



//
//  We query the contents of the indicated path, which must be a scope. We don't
//  try to be clever. We temporarily disable drawing, remove the contents of
//  the scope, then call our scope loader above to reload it. This also serves to
//  toss all of the nested scopes, so we go back to virtaul scope mode on all of
//  them, to be subsequently reloaded as well. Not the most efficient, but the
//  safest.
//
tCIDLib::TVoid TCQCDataSrvBrws::UpdateScope(const TString& strPath)
{
    // Get the associated browser info for the target
    const TDSBrowseItem& dsbiPath
    (
        *static_cast<const TDSBrowseItem*>(wndBrowser().pobjUser(strPath, kCIDLib::True))
    );

    // Has to be a scope
    CIDAssert(dsbiPath.bIsScope(), L"Cannot update a non-scope item");

    try
    {
        // First make sure we can get the new data
        TVector<TDSBrowseItem>  colItems;
        tCIDLib::TCardList      fcolPathIds;
        TDataSrvClient          dsclToUse;
        dsclToUse.QueryScopeItems(strPath, fcolPathIds, colItems, kCIDLib::False, sectUser());

        //
        //  It workd, so let's clean out any existing content Disable updates
        //  while doing all fo this.
        //
        TCQCTreeBrowser& wndTar = wndBrowser();
        TWndPaintJanitor janPaint(&wndTar);

        wndBrowser().RemoveChildrenOf(strPath);

        // And let's load up the new stuff
        TString strChildPath;
        const tCIDLib::TCard4 c4Count = colItems.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const TDSBrowseItem& dsbiCur = colItems[c4Index];
            strChildPath = strPath;
            strChildPath.Append(kCIDLib::chForwardSlash);
            strChildPath.Append(dsbiCur.strName());

            if (dsbiCur.bIsScope())
            {
                wndTar.AddScope(strPath, dsbiCur.strName(), kCIDLib::True);
            }
             else
            {
                wndTar.AddItem(strPath, dsbiCur.strName());
                if (dsbiCur.bUserFlag())
                    wndTar.SetItemStatus(strChildPath, tCQCTreeBrws::EItemStates::Paused);
            }

            //
            //  Store the browser info object on the tree item so that we can get
            //  back to it later.
            //
            wndTar.SetUserObj(strChildPath, new TDSBrowseItem(dsbiCur));
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        facCQCGKit().ShowError
        (
            wndBrowser()
            , L"Scope Update Error"
            , errToCatch
        );
    }
}

