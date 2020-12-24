//
// FILE NAME: MediaRepoMgr_SelTitleSet.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/23/2006
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
//  This is the header for the MediaRepoMgr_SelTitleSetDlg.cpp file, which
//  implements a dialog that allows the user to select a title set or create
//  a new one.
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
#include    "MediaRepoMgr_SelTitleSetDlg.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TSelTitleSetDlg,TDlgBox)



// ---------------------------------------------------------------------------
//   CLASS: TSelTitleSetDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TSelTitleSetDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TSelTitleSetDlg::TSelTitleSetDlg() :

    m_bLoadedList(kCIDLib::False)
    , m_pmdbEdit(nullptr)
    , m_pmcolSrc(nullptr)
    , m_pwndCancel(nullptr)
    , m_pwndExisting(nullptr)
    , m_pwndList(nullptr)
    , m_pwndNew(nullptr)
    , m_pwndNewName(nullptr)
    , m_pwndSave(nullptr)
{
}

TSelTitleSetDlg::~TSelTitleSetDlg()
{
}


// ---------------------------------------------------------------------------
//  TSelTitleSetDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TSelTitleSetDlg::bRunDlg(const  TWindow&        wndOwner
                        ,       TMediaTitleSet& mtsEdit
                        , const TMediaCollect&  mcolSrc
                        ,       TMediaDB&       mdbEdit)
{
    // Store incoming stuff for later use
    m_pmdbEdit = &mdbEdit;
    m_pmcolSrc = &mcolSrc;
    m_mtsEdit = mtsEdit;

    const tCIDLib::TCard4 c4Res = c4RunDlg
    (
        wndOwner, facMediaRepoMgr, kMediaRepoMgr::ridDlg_SelTitleSet
    );

    if (c4Res == kMediaRepoMgr::ridDlg_SelTitleSet_Save)
    {
        // Give back the changes
        mtsEdit = m_mtsEdit;
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


// ---------------------------------------------------------------------------
//  TSelTitleSetDlg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TSelTitleSetDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRes = TParent::bCreated();

    // Get typed pointers to some of the widgets
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_SelTitleSet_Cancel, m_pwndCancel);
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_SelTitleSet_Existing, m_pwndExisting);
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_SelTitleSet_New, m_pwndNew);
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_SelTitleSet_List, m_pwndList);
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_SelTitleSet_NewName, m_pwndNewName);
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_SelTitleSet_Save, m_pwndSave);

    //
    //  Select the initial scheme. If we got an incoming title set, then it must
    //  already exist, so let's load the list and select it. Else assume we going
    //  to create a new one.
    //
    if (m_mtsEdit.c2Id())
    {
        m_pwndList->SetEnable(kCIDLib::True);
        m_pwndList->TakeFocus();
        m_pwndNewName->SetEnable(kCIDLib::False);

        m_pwndExisting->SetCheckedState(kCIDLib::True);
        LoadTitleList(m_mtsEdit.c2Id());

        m_pwndExisting->SetCheckedState(kCIDLib::True);
        m_pwndNew->SetCheckedState(kCIDLib::False);
    }
     else
    {
        m_pwndNew->SetCheckedState(kCIDLib::True);
        m_pwndNewName->strWndText(m_pmcolSrc->strName());

        m_pwndExisting->SetCheckedState(kCIDLib::False);
        m_pwndNew->SetCheckedState(kCIDLib::True);
    }

    // Install handlers
    m_pwndCancel->pnothRegisterHandler(this, &TSelTitleSetDlg::eClickHandler);
    m_pwndExisting->pnothRegisterHandler(this, &TSelTitleSetDlg::eClickHandler);
    m_pwndNew->pnothRegisterHandler(this, &TSelTitleSetDlg::eClickHandler);
    m_pwndSave->pnothRegisterHandler(this, &TSelTitleSetDlg::eClickHandler);

    return bRes;
}



// ---------------------------------------------------------------------------
//  TSelTitleSetDlg: Private, non-virtual methods
// ---------------------------------------------------------------------------

tCIDCtrls::EEvResponses TSelTitleSetDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kMediaRepoMgr::ridDlg_SelTitleSet_Cancel)
    {
        EndDlg(kMediaRepoMgr::ridDlg_SelTitleSet_Cancel);
    }
     else if (wnotEvent.widSource() == kMediaRepoMgr::ridDlg_SelTitleSet_Existing)
    {
        // Disable and clear the entry field and enable the list
        m_pwndList->SetEnable(kCIDLib::True);
        m_pwndList->TakeFocus();
        m_pwndNewName->SetEnable(kCIDLib::False);

        if (!m_bLoadedList)
            LoadTitleList(0);

        // Set the state, and uncheck the New one
        m_pwndExisting->SetCheckedState(kCIDLib::True);
        m_pwndNew->SetCheckedState(kCIDLib::False);
    }
     else if (wnotEvent.widSource() == kMediaRepoMgr::ridDlg_SelTitleSet_New)
    {
        // Disable the list box and enable the entry field
        m_pwndNewName->SetEnable(kCIDLib::True);
        m_pwndNewName->TakeFocus();
        m_pwndList->SetEnable(kCIDLib::False);

        // Set the state, and uncheck the New one
        m_pwndExisting->SetCheckedState(kCIDLib::False);
        m_pwndNew->SetCheckedState(kCIDLib::True);
    }
     else if (wnotEvent.widSource() == kMediaRepoMgr::ridDlg_SelTitleSet_Save)
    {
        // Make sure that we have what we need
        tCIDLib::TBoolean bDone = kCIDLib::False;
        if (m_pwndNew->bCheckedState())
        {
            // They are creating a new one, so set it up
            TString strTitle = m_pwndNewName->strWndText();
            if (!strTitle.bIsEmpty())
            {
                // Create a sort title from the regular title
                TString strSortTitle;
                facCQCMedia().bMakeSortTitle(strTitle, strSortTitle);
                strSortTitle.ToUpper();

                m_mtsEdit = TMediaTitleSet
                (
                    strTitle
                    , TUniqueId::strMakeId()
                    , strSortTitle
                    , m_pmcolSrc->eType()
                );

                // Copy over some info from the col that is likely to be correct
                m_mtsEdit.strArtist(m_pmcolSrc->strArtist());
                bDone = kCIDLib::True;
            }
        }
         else
        {
            // They selected an existing one, so use that
            const tCIDLib::TCard4 c4LIndex = m_pwndList->c4CurItem();
            if (c4LIndex != kCIDLib::c4MaxCard)
            {
                m_mtsEdit = *m_pmdbEdit->pmtsById
                (
                    m_pmcolSrc->eType()
                    , tCIDLib::TCard2(m_pwndList->c4IndexToId(c4LIndex))
                    , kCIDLib::True
                );
                bDone = kCIDLib::True;
            }
        }

        if (bDone)
            EndDlg(kMediaRepoMgr::ridDlg_SelTitleSet_Save);
    }
    return tCIDCtrls::EEvResponses::Handled;
}


//
//  Loads up the list with the existing title sets. We store the id of the titles
//  in ane xtra field of the list window item, so we can get back to it.
//
//  If the collection we are getting a set for is a playlist, we only will show
//  playlist title sets.
//
tCIDLib::TVoid TSelTitleSetDlg::LoadTitleList(const tCIDLib::TCard2 c2InitId)
{
    //
    //  Get the count of titles for the collection's media type, and remember if
    //  the collection is a playlist, for later use below.
    //
    const tCIDLib::TBoolean         bIsPL  = m_pmcolSrc->bIsPlayList();
    const tCQCMedia::EMediaTypes    eMType = m_pmcolSrc->eType();

    // Ask for a sorted list of refs to titles for our media type
    TRefVector<const TMediaTitleSet> colSets(tCIDLib::EAdoptOpts::NoAdopt);
    const tCIDLib::TCard4 c4TitleCnt = m_pmdbEdit->c4QuerySetList
    (
        eMType, colSets, TMediaTitleSet::eCompByTitle
    );

    //
    //  And now load up our list window with that sorted list. Look for the initial
    //  selection as we go if we can find it.
    //
    tCIDLib::TCard4 c4InitSel = 0;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4TitleCnt; c4Index++)
    {
        const TMediaTitleSet* pmtsCur = colSets[c4Index];

        // If this colleciton is a playlist, only take playlist titles
        if (bIsPL && !pmtsCur->bIsPlayList())
            continue;

        m_pwndList->c4AddItem(pmtsCur->strName(), pmtsCur->c2Id());

        if (pmtsCur->c2Id() == c2InitId)
            c4InitSel = c4Index;
    }

    if (c4TitleCnt)
        m_pwndList->SelectByIndex(c4InitSel);

    // Remember we've loaded the list
    m_bLoadedList = kCIDLib::True;
}

