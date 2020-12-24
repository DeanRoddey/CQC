//
// FILE NAME: CQCTreeBrws_SelSrvDrvDlg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/14/2001
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
//  This file implements the server side device driver selection dialog.
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
#include    "CQCTreeBrws_SelSrvDrvDlg_.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TSelSrvDrvDlg,TDlgBox)


// ---------------------------------------------------------------------------
//   CLASS: TSelSrvDrvDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TSelSrvDrvDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TSelSrvDrvDlg::TSelSrvDrvDlg() :

    m_c4SelIndex(kCIDLib::c4MaxCard)
    , m_pwndCancelButton(nullptr)
    , m_pwndCatList(nullptr)
    , m_pwndDevClasses(nullptr)
    , m_pwndDrivers(nullptr)
    , m_pwndMakeList(nullptr)
    , m_pwndSelectButton(nullptr)
{
}

TSelSrvDrvDlg::~TSelSrvDrvDlg()
{
}


// ---------------------------------------------------------------------------
//  TSelSrvDrvDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TSelSrvDrvDlg::bRunDlg( const   TWindow&        wndOwner
                        ,       TCQCDriverCfg&  cqcdcToFill
                        , const TString&        strHost
                        , const TCQCSecToken&   sectUser)
{
    // Store the host name for later to put into the title bar
    m_sectUser = sectUser;
    m_strHost = strHost;

    //
    //  First we need to get a list of all of the possible drivers. So get
    //  an installation server proxy and query them. If we get any, then
    //  sort them. If we can't do this, or there's no point creating the dialog.
    //
    try
    {
        TWndPtrJanitor janPtr(tCIDCtrls::ESysPtrs::Wait);
        tCQCKit::TInstSrvProxy orbcInstSrv = facCQCKit().orbcInstSrvProxy();

        if (!orbcInstSrv->c4QueryDrvManifests(m_colDriverCfg, sectUser))
        {
            TOkBox msgbNone
            (
                strWndText(), L"There were no driver types available to load"
            );
            msgbNone.ShowIt(wndOwner);
            return kCIDLib::False;
        }

        // Sort them before we load them
        m_colDriverCfg.Sort(TCQCDriverCfg::eCompOnDisplayName);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        facCQCGKit().ShowError
        (
            wndOwner
            , L"An error occurred while quering the available drivers"
            , errToCatch
        );
        return kCIDLib::False;
    }

    //
    //  We got it, so try to run the dialog. If successful and we get a
    //  positive response, then fill in the caller's parameter with the
    //  info on the selected device.
    //
    const tCIDLib::TCard4 c4Res = c4RunDlg
    (
        wndOwner, facCQCTreeBrws(), kCQCTreeBrws::ridDlg_SelSDrv
    );

    if (c4Res == kCQCTreeBrws::ridDlg_SelSDrv_Select)
    {
        cqcdcToFill = m_colDriverCfg[m_c4SelIndex];
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


// ---------------------------------------------------------------------------
//  TSelSrvDrvDlg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TSelSrvDrvDlg::bCreated()
{
    const tCIDLib::TBoolean bRes = TParent::bCreated();

    // Store our pre-typed child widget pointers
    CastChildWnd(*this, kCQCTreeBrws::ridDlg_SelSDrv_Cancel, m_pwndCancelButton);
    CastChildWnd(*this, kCQCTreeBrws::ridDlg_SelSDrv_CatList, m_pwndCatList);
    CastChildWnd(*this, kCQCTreeBrws::ridDlg_SelSDrv_DevClasses, m_pwndDevClasses);
    CastChildWnd(*this, kCQCTreeBrws::ridDlg_SelSDrv_List, m_pwndDrivers);
    CastChildWnd(*this, kCQCTreeBrws::ridDlg_SelSDrv_MakeList, m_pwndMakeList);
    CastChildWnd(*this, kCQCTreeBrws::ridDlg_SelSDrv_Select, m_pwndSelectButton);

    // Add the target host to the title bar
    TString strTitle = strWndText();
    strTitle.eReplaceToken(m_strHost, L'1');
    strWndText(strTitle);

    // Set up our list boxes, which are multi-columns ones but only with one column
    tCIDLib::TStrList colCols(1);
    colCols.objAdd(TString::strEmpty());

    colCols[0] = facCQCTreeBrws().strMsg(kTBrwsMsgs::midDlg_SelSDrv_CatPref);
    m_pwndCatList->SetColumns(colCols);

    colCols[0] = facCQCTreeBrws().strMsg(kTBrwsMsgs::midDlg_SelSDrv_MakePref);
    m_pwndMakeList->SetColumns(colCols);

    colCols[0] = facCQCTreeBrws().strMsg(kTBrwsMsgs::midDlg_SelSDrv_MatchPref);
    m_pwndDrivers->SetColumns(colCols);

    // Load the filter list
    LoadFilters();

    // And register our event handlers
    m_pwndCancelButton->pnothRegisterHandler(this, &TSelSrvDrvDlg::eClickHandler);
    m_pwndSelectButton->pnothRegisterHandler(this, &TSelSrvDrvDlg::eClickHandler);
    m_pwndCatList->pnothRegisterHandler(this, &TSelSrvDrvDlg::eLBHandler);
    m_pwndDrivers->pnothRegisterHandler(this, &TSelSrvDrvDlg::eLBHandler);
    m_pwndMakeList->pnothRegisterHandler(this, &TSelSrvDrvDlg::eLBHandler);

    // And do an initial filtered load of the driver list
    FilterDriverList();

    return bRes;
}

// ---------------------------------------------------------------------------
//  TSelSrvDrvDlg: Private, non-virtual methods
// ---------------------------------------------------------------------------

// Handles button clicks
tCIDCtrls::EEvResponses TSelSrvDrvDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    // Just exit with a one or zero, according to the button
    if (wnotEvent.widSource() == kCQCTreeBrws::ridDlg_SelSDrv_Select)
    {
        //
        //  If the driver window isn't empty due to filtering, then one
        //  of them is selected, so we are good.
        //
        m_c4SelIndex = m_pwndDrivers->c4CurItem();
        if (m_c4SelIndex != kCIDLib::c4MaxCard)
        {
            // The actual index is stored in the row id because it's a filtered list
            m_c4SelIndex = m_pwndDrivers->c4IndexToId(m_c4SelIndex);
            EndDlg(kCQCTreeBrws::ridDlg_SelSDrv_Select);
        }
    }
     else if (wnotEvent.widSource() == kCQCTreeBrws::ridDlg_SelSDrv_Cancel)
    {
        EndDlg(kCQCTreeBrws::ridDlg_SelSDrv_Cancel);
    }
    return tCIDCtrls::EEvResponses::Handled;
}


// Handles list box events
tCIDCtrls::EEvResponses TSelSrvDrvDlg::eLBHandler(TListChangeInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCTreeBrws::ridDlg_SelSDrv_List)
    {
        //
        //  If invoked, then act like Select button was chosen. If just
        //  cursored, then update our info on the newly cursored item.
        //
        if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::Invoked)
        {
            m_c4SelIndex = m_pwndDrivers->c4IndexToId(wnotEvent.c4Index());
            EndDlg(kCQCTreeBrws::ridDlg_SelSDrv_Select);
        }
         else if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::SelChanged)
        {
            const TCQCDriverCfg& cqcdcCur = m_colDriverCfg
            [
                m_pwndDrivers->c4IndexToId(wnotEvent.c4Index())
            ];

            // Output the version info
            pwndChildById(kCQCTreeBrws::ridDlg_SelSDrv_Version)->strWndText
            (
                cqcdcCur.strVersion()
            );

            // And the Make/Model info
            pwndChildById(kCQCTreeBrws::ridDlg_SelSDrv_Make)->strWndText(cqcdcCur.strMake());
            pwndChildById(kCQCTreeBrws::ridDlg_SelSDrv_Model)->strWndText(cqcdcCur.strModel());

            // Output the descriptive text
            pwndChildById(kCQCTreeBrws::ridDlg_SelSDrv_Description)->strWndText
            (
                cqcdcCur.strDescription()
            );

            //
            //  If it's V2 compatible, load the device classes list, else
            //  clear it.
            //
            if (cqcdcCur.c4ArchVersion() > 1)
            {
                LoadDevClasses(cqcdcCur);
            }
             else
            {
                m_pwndDevClasses->ClearText();
                m_pwndDevClasses->SetEnable(kCIDLib::False);
            }
        }
         else if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::Cleared)
        {
            pwndChildById(kCQCTreeBrws::ridDlg_SelSDrv_Description)->ClearText();
            pwndChildById(kCQCTreeBrws::ridDlg_SelSDrv_Make)->ClearText();
            pwndChildById(kCQCTreeBrws::ridDlg_SelSDrv_Model)->ClearText();

            m_pwndDevClasses->ClearText();
            m_pwndDevClasses->SetEnable(kCIDLib::False);
        }
    }
     else if ((wnotEvent.widSource() == kCQCTreeBrws::ridDlg_SelSDrv_CatList)
          ||  (wnotEvent.widSource() == kCQCTreeBrws::ridDlg_SelSDrv_MakeList))
    {
        //
        //  Else it's one of the filter lists, so just reload the driver list based
        //  on the filter changes. We will only get one event if multiple items are
        //  modified.
        //
        if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::Marked)
            FilterDriverList();
    }
    return tCIDCtrls::EEvResponses::Handled;
}


//
//  When the user changes the selected filters in the filter list, this
//  is called and we filter the list for only those that match the new
//  filter.
//
tCIDLib::TVoid TSelSrvDrvDlg::FilterDriverList()
{
    tCIDLib::TStrList colCols(1);
    colCols.objAdd(TString::strEmpty());

    //
    //  Get the list of makes and the list of device categories into
    //  unique lists so that we can quickly check against them.
    //
    tCIDLib::TStrHashSet colCats(109, TStringKeyOps());
    tCIDLib::TStrHashSet colMakes(109, TStringKeyOps());

    // Do the cat list first
    TString strVal;
    tCIDLib::TCard4 c4Count = m_pwndCatList->c4ItemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (m_pwndCatList->bIsMarked(c4Index))
        {
            m_pwndCatList->QueryColText(c4Index, 0, strVal);
            colCats.objAdd(strVal);
        }
    }

    // And the make list if needed
    c4Count = m_pwndMakeList->c4ItemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (m_pwndMakeList->bIsMarked(c4Index))
        {
            m_pwndMakeList->QueryColText(c4Index, 0, strVal);
            colMakes.objAdd(strVal);
        }
    }

    // Disable updates while we reload
    TWndPaintJanitor janDrivers(m_pwndDrivers);

    // Remote any existing stuff
    m_pwndDrivers->RemoveAll();

    // OK, now go through and load the ones that meet the filter criteria
    c4Count = m_colDriverCfg.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TCQCDriverCfg& cqcdcCur = m_colDriverCfg[c4Index];
        const TString& strDevCat = tCQCKit::strXlatEDevCats(cqcdcCur.eCategory());

        tCIDLib::TBoolean bCatsOk = colCats.bHasElement(strDevCat);
        tCIDLib::TBoolean bMakesOk = colMakes.bHasElement(cqcdcCur.strMake());

        //
        //  Store the original list index as the per-row id so that we can map
        //  back to the original. The list is sorted and sparse so we need this.
        //
        if (bCatsOk && bMakesOk)
        {
            colCols[0] = cqcdcCur.strDisplayName();
            m_pwndDrivers->c4AddItem(colCols, c4Index);
        }
    }

    //
    //  Select the first driver in the list, if we didn't filter them all out. Force
    //  a notification so that the driver info stuff gets loaded for this initial one.
    //
    if (m_pwndDrivers->c4ItemCount())
        m_pwndDrivers->SelectByIndex(0, kCIDLib::True);
}


//
//  When a new driver is selected in the driver list, this is called and
//  we load up the device classes this driver supports.
//
tCIDLib::TVoid
TSelSrvDrvDlg::LoadDevClasses(const TCQCDriverCfg& cqcdcSrc)
{
    m_pwndDevClasses->SetEnable(kCIDLib::True);

    const tCQCKit::TDevClassList& fcolDevCls = cqcdcSrc.fcolDevClasses();
    const tCIDLib::TCard4 c4Count = fcolDevCls.c4ElemCount();
    if (c4Count)
    {
        // Format them out into a new line separated list
        TTextStringOutStream strmOut(1024UL);

        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            strmOut << tCQCKit::strXlatEDevClasses(fcolDevCls[c4Index])
                    << kCIDLib::NewLn;
        }
        strmOut.Flush();

        m_pwndDevClasses->strWndText(strmOut.strData());
    }
     else
    {
        m_pwndDevClasses->ClearText();
    }
}


//
//  Called on load to get the filter lists loaded up. It's called after
//  LoadDriverList since we need that in order to get the list of device
//  makes to load.
//
tCIDLib::TVoid TSelSrvDrvDlg::LoadFilters()
{
    tCIDLib::TStrList colCols(1);
    colCols.objAdd(TString::strEmpty());

    //
    //  Load up the categories list. It's sorted so store the original cat ordinal
    //  as the row id.
    //
    m_pwndCatList->RemoveAll();
    tCQCKit::EDevCats eCat = tCQCKit::EDevCats::Min;
    while (eCat <= tCQCKit::EDevCats::Max)
    {
        colCols[0] = tCQCKit::strXlatEDevCats(eCat);
        m_pwndCatList->c4AddItem(colCols, tCIDLib::c4EnumOrd(eCat));
        eCat++;
    }

    //
    //  For the make list, this depends on the drivers. So we have to get
    //  a unique list of makes from the list of drivers.
    //
    tCIDLib::TStrHashSet    colUnique(109, TStringKeyOps());
    tCIDLib::TCard4 c4Count = m_colDriverCfg.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TCQCDriverCfg& cqcdcCur = m_colDriverCfg[c4Index];
        tCIDLib::TBoolean bAdded;
        colUnique.objAddIfNew(cqcdcCur.strMake(), bAdded);
    }

    //
    //  Load those up. It's sorted but we just use the text itself so there's no
    //  need to map back to anything.
    //
    tCIDLib::TStrHashSet::TCursor cursUnique(&colUnique);
    cursUnique.bReset();
    do
    {
        colCols[0] = cursUnique.objRCur();
        m_pwndMakeList->c4AddItem(colCols, 0);
    }   while(cursUnique.bNext());

    // Now select all items in both initially, to show all drivers
    m_pwndCatList->MarkAll(kCIDLib::True);
    m_pwndMakeList->MarkAll(kCIDLib::True);
}


