//
// FILE NAME: CQCIGKit_AddToPackDlg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/05/2005
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
//  Manages the contents of package and saves it if the user asks to.
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
#include    "CQCIGKit_.hpp"
#include    "CQCIGKit_AddToPackDlg_.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TAddToPackDlg,TDlgBox)



// ---------------------------------------------------------------------------
//   CLASS: TAddToPackDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TAddToPackDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TAddToPackDlg::TAddToPackDlg() :

    m_bEncrypted(kCIDLib::False)
    , m_eDType(tCQCRemBrws::EDTypes::Count)
    , m_eFType(tCQCKit::EPackFlTypes::Count)
    , m_pcuctxToUse(nullptr)
    , m_ppackTar(nullptr)
    , m_pwndAdd(nullptr)
    , m_pwndCancel(nullptr)
    , m_pwndList(nullptr)
    , m_pwndRevert(nullptr)
    , m_pwndRemove(nullptr)
    , m_pwndSave(nullptr)
    , m_pwndTree(nullptr)
{
}

TAddToPackDlg::~TAddToPackDlg()
{
}


// ---------------------------------------------------------------------------
//  TAddToPackDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TAddToPackDlg::bRunDlg( const   TWindow&                wndOwner
                        ,       TCQCPackage&            packTar
                        , const TString&                strInitPath
                        , const tCIDLib::TBoolean       bEncrypted
                        , const tCQCRemBrws::EDTypes    eDType
                        , const TCQCUserCtx&            cuctxToUse)
{
    // Store the incoming values till we can get them displayed
    m_bEncrypted = bEncrypted;
    m_eDType = eDType;
    m_pcuctxToUse = &cuctxToUse;
    m_ppackTar = &packTar;
    m_strInitPath = strInitPath;

    // Translate the incoming data type to a package file type
    switch(m_eDType)
    {
        case tCQCRemBrws::EDTypes::Macro :
            m_eFType = tCQCKit::EPackFlTypes::CMLMacro;
            break;

        case tCQCRemBrws::EDTypes::Image :
            m_eFType = tCQCKit::EPackFlTypes::PNGImage;
            break;

        default :
            break;
    };

    if (m_eFType == tCQCKit::EPackFlTypes::Count)
    {
        TErrBox msgbErr(L"This file type is not understood");
        msgbErr.ShowIt(wndOwner);
        return kCIDLib::False;
    }

    // Remember the base path for our type
    facCQCRemBrws().BasePathForType(eDType, m_strBasePath);

    //
    //  We got it, so try to run the dialog. If successful and we get a
    //  positive response, then fill in the caller's parameter with the
    //  info on the selected device.
    //
    const tCIDLib::TCard4 c4Res = c4RunDlg
    (
        wndOwner, facCQCIGKit(), kCQCIGKit::ridDlg_AddToPack
    );
    return (c4Res == kCQCIGKit::ridDlg_AddToPack_Save);
}


// ---------------------------------------------------------------------------
//  TAddToPackDlg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TAddToPackDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRet = TParent::bCreated();

    // Store our pre-typed child widget pointers
    CastChildWnd(*this, kCQCIGKit::ridDlg_AddToPack_Add, m_pwndAdd);
    CastChildWnd(*this, kCQCIGKit::ridDlg_AddToPack_Cancel, m_pwndCancel);
    CastChildWnd(*this, kCQCIGKit::ridDlg_AddToPack_Revert, m_pwndRevert);
    CastChildWnd(*this, kCQCIGKit::ridDlg_AddToPack_List, m_pwndList);
    CastChildWnd(*this, kCQCIGKit::ridDlg_AddToPack_Save, m_pwndSave);
    CastChildWnd(*this, kCQCIGKit::ridDlg_AddToPack_Remove, m_pwndRemove);

    // Get the place holder window for the tree browser. Get its area and then destroy it.
    TWindow* pwndPH = pwndChildById(kCQCIGKit::ridDlg_AddToPack_Tree);
    TArea areaTree(pwndPH->areaWnd());
    pwndPH->Destroy();
    delete pwndPH;

    // Now let's create the actual browser window
    m_pwndTree = new TCQCTreeTypeBrowser();
    m_pwndTree->CreateTreeTypeBrowser
    (
        *this
        , m_eDType
        , kCQCIGKit::ridDlg_AddToPack_Tree
        , areaTree
        , tCIDLib::eOREnumBits
          (
            tCIDCtrls::EWndStyles::VisTabGroupChild, tCIDCtrls::EWndStyles::Border
          )
        , tCQCTreeBrws::EBrwsStyles::ReadOnly
        , *m_pcuctxToUse
    );
    m_pwndTree->eEdgeAnchor(tCIDCtrls::EEdgeAnchors::SizeBottomRight);

    // If there's an initial path, select it
    if (!m_strInitPath.bIsEmpty())
        m_pwndTree->bSelectBestTry(m_strInitPath);

    // Set the list col titles
    tCIDLib::TStrList colCols(2);
    colCols.objAdd(L"File Type");
    colCols.objAdd(L"File Name");
    m_pwndList->SetColumns(colCols);

    // Display any files currently in the package
    const tCIDLib::TCard4 c4Count = m_ppackTar->c4FileCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TCQCPackageFl& pflCur = m_ppackTar->pflAt(c4Index);

        colCols[0] = tCQCKit::strXlatEPackFlTypes(pflCur.eFileType());
        colCols[1] = pflCur.strName();
        const tCIDLib::TCard4 c4At = m_pwndList->c4AddItem(colCols, 0);

        //
        //  Set the flag on this one as one of the originals. We protect these from
        //  being deleted. Any added after this won't set it.
        //
        m_pwndList->SetFlagAt(c4At, kCIDLib::True);
    }

    // Autosize the 0th column to the content
    m_pwndList->AutoSizeCol(0, kCIDLib::False);

    // And register our button event handlers
    m_pwndAdd->pnothRegisterHandler(this, &TAddToPackDlg::eClickHandler);
    m_pwndCancel->pnothRegisterHandler(this, &TAddToPackDlg::eClickHandler);
    m_pwndList->pnothRegisterHandler(this, &TAddToPackDlg::eLBHandler);
    m_pwndRemove->pnothRegisterHandler(this, &TAddToPackDlg::eClickHandler);
    m_pwndRevert->pnothRegisterHandler(this, &TAddToPackDlg::eClickHandler);
    m_pwndSave->pnothRegisterHandler(this, &TAddToPackDlg::eClickHandler);
    m_pwndTree->pnothRegisterBrowserHandler(this, &TAddToPackDlg::eBrwsHandler);

    // If any incoming files, select the 0th one
    if (m_pwndList->c4ItemCount())
        m_pwndList->SelectByIndex(0);

    return bRet;
}


// ---------------------------------------------------------------------------
//  TAddToPackDlg: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  This is called from either an add click or a double click on an item in the list. We
//  do the work required to add the requested content to the list, or any that were not
//  already there.
//
tCIDLib::TVoid TAddToPackDlg::AddContent()
{
    //
    //  Get the selection. If it's at or below the root path for our data type, then
    //  it's potentially one we want.
    //
    TString strHPath;
    if (!m_pwndTree->bSelectedItem(strHPath))
        return;

    // Get the relative path
    TString strRelPath;
    facCQCRemBrws().CreateRelPath(strHPath, m_eDType, strRelPath);

    // If it's system content, then ignore this unless system edit is enabled
    if (!strRelPath.bStartsWithI(kCQCKit::strRepoScope_User) && !facCQCKit().bSysEdit())
        return;

    // If it's a scope, then we may need to add all of them, else a single item
    tCIDLib::TStrList colClasses;
    if (m_pwndTree->bIsScope(strHPath))
    {
        try
        {
            // Get just item names
            tCIDLib::TCard4 c4ScopeId = 0;
            m_dsclLoad.bQueryScopeNames
            (
                c4ScopeId
                , strRelPath
                , m_eDType
                , colClasses
                , tCQCRemBrws::EQSFlags::Items
                , m_pcuctxToUse->sectUser()
            );
        }

        catch(TError& errToCatch)
        {
            TExceptDlg dlgErr
            (
                *this
                , strWndText()
                , L"An error occurred while querying the macros in the selected scope"
                , errToCatch
            );
            return;
        }
    }
     else
    {
        //
        //  Just add the one path to the list. We have to get just the name part out since
        //  that's what the scope query above does, and leave our parent scope as the
        //  relative path.
        //
        TString strName;
        facCQCRemBrws().SplitPath(strRelPath, strName);
        colClasses.objAdd(strName);
    }

    try
    {
        // Iterate them and call the helper for each one
        const tCIDLib::TCard4 c4Count = colClasses.c4ElemCount();
        TString strCurRelPath;
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            strCurRelPath = strRelPath;
            facCQCRemBrws().AddPathComp(strCurRelPath, colClasses[c4Index]);

            tCIDLib::TCard4 c4At;
            if (!m_ppackTar->bFileIsInPackage(strCurRelPath, m_eFType, c4At))
            {
                facCQCIGKit().AddFileToPack
                (
                    m_dsclLoad
                    , *m_ppackTar
                    , m_eFType
                    , strCurRelPath
                    , m_eDType
                    , m_pcuctxToUse->sectUser()
                );

                // And display it in our list
                tCIDLib::TStrList colCols(2);
                colCols.objAdd(tCQCKit::strXlatEPackFlTypes(m_eFType));
                colCols.objAdd(strCurRelPath);
                m_pwndList->c4AddItem(colCols, 0);
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
            , L"An error occurred while downloading the macro data"
            , errToCatch
        );
    }
}



tCIDCtrls::EEvResponses TAddToPackDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCIGKit::ridDlg_AddToPack_Add)
    {
        // Call a helper since we also need to do this from a double click
        AddContent();
    }
     else if (wnotEvent.widSource() == kCQCIGKit::ridDlg_AddToPack_Cancel)
    {
        EndDlg(kCQCIGKit::ridDlg_AddToPack_Cancel);
    }
     else if (wnotEvent.widSource() == kCQCIGKit::ridDlg_AddToPack_Remove)
    {
        // Get the selected item and then the relative path version of it
        TString strHPath;
        if (m_pwndTree->bSelectedItem(strHPath))
        {
            TString strRelPath;
            facCQCRemBrws().CreateRelPath(strHPath, m_eDType, strRelPath);

            const tCIDLib::TCard4 c4Ind = m_pwndList->c4TextToIndex(strRelPath, 1);
            if (c4Ind != kCIDLib::c4MaxCard)
            {
                m_ppackTar->bRemoveFile(strRelPath);
                m_pwndList->RemoveAt(c4Ind);
            }
        }
    }
     else if (wnotEvent.widSource() == kCQCIGKit::ridDlg_AddToPack_Revert)
    {
        // Go through the list and remove all other than the original ones
        tCIDLib::TCard4 c4Count = m_pwndList->c4ItemCount();
        tCIDLib::TCard4 c4Index = 0;
        while (c4Index < c4Count)
        {
            if (m_pwndList->bFlagAt(c4Index))
            {
                c4Index++;
            }
             else
            {
                TString strPath;
                m_pwndList->QueryColText(c4Index, 1, strPath);
                m_ppackTar->bRemoveFile(strPath);
                m_pwndList->RemoveAt(c4Index);
                c4Count--;
            }
        }
    }
     else if (wnotEvent.widSource() == kCQCIGKit::ridDlg_AddToPack_Save)
    {
        EndDlg(kCQCIGKit::ridDlg_AddToPack_Save);
    }
    return tCIDCtrls::EEvResponses::Handled;
}



tCIDCtrls::EEvResponses TAddToPackDlg::eLBHandler(TListChangeInfo& wnotEvent)
{
    if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::SelChanged)
    {
        // If it's one of the originals, disable the remove button, else enable
        const tCIDLib::TCard4 c4At = m_pwndList->c4CurItem();
        m_pwndRemove->SetEnable(!m_pwndList->bFlagAt(c4At));
    }
    return tCIDCtrls::EEvResponses::Handled;
}


//
//  We get browser events, not the underlying tree events. The browser eats those. The
//  browser will send us edit or view events based on the type of data and whether it's
//  in the user or system area. So we just respond to either as an 'invoked' event.
//
tCIDCtrls::EEvResponses TAddToPackDlg::eBrwsHandler(TTreeBrowseInfo& wnotEvent)
{
    if ((wnotEvent.eEvent() == tCQCTreeBrws::EEvents::Edit)
    ||  (wnotEvent.eEvent() == tCQCTreeBrws::EEvents::View))
    {
        AddContent();
    }
    return tCIDCtrls::EEvResponses::Handled;
}

