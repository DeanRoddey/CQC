//
// FILE NAME: MediaRepoMgr_EdMCatsDlg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/04/2006
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
//  This is the header for the MediaRepoMGr_EdMCatsDlg.cpp file, which
//  allows the user to edit the media categories.
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
#include    "MediaRepoMgr_EdMCatsDlg.hpp"



// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TEdMCatsDlg,TDlgBox)



// ---------------------------------------------------------------------------
//   CLASS: TEdMCatsDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TEdMCatsDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TEdMCatsDlg::TEdMCatsDlg() :

    m_eType(tCQCMedia::EMediaTypes::Music)
    , m_objaDel(tCQCMedia::EMediaTypes::Count)
    , m_objaNames(tCQCMedia::EMediaTypes::Count)
    , m_pmdbEdit(nullptr)
    , m_pwndAdd(nullptr)
    , m_pwndCancel(nullptr)
    , m_pwndDelete(nullptr)
    , m_pwndList(nullptr)
    , m_pwndMType(nullptr)
    , m_pwndSave(nullptr)
{
}

TEdMCatsDlg::~TEdMCatsDlg()
{
}


// ---------------------------------------------------------------------------
//  TEdMCatsDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TEdMCatsDlg::RunDlg(const TWindow& wndOwner, TMediaDB& mdbEdit)
{
    // Store the incoming database for later use
    m_pmdbEdit = &mdbEdit;

    c4RunDlg(wndOwner, facMediaRepoMgr, kMediaRepoMgr::ridDlg_EdMCats);
}


// ---------------------------------------------------------------------------
//  TEdMCatsDlg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TEdMCatsDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRet = TParent::bCreated();

    // Get typed pointers to some of the widgets
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_EdMCats_Add, m_pwndAdd);
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_EdMCats_Cancel, m_pwndCancel);
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_EdMCats_Delete, m_pwndDelete);
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_EdMCats_List, m_pwndList);
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_EdMCats_MType, m_pwndMType);
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_EdMCats_Save, m_pwndSave);

    // Register handlers
    m_pwndAdd->pnothRegisterHandler(this, &TEdMCatsDlg::eClickHandler);
    m_pwndCancel->pnothRegisterHandler(this, &TEdMCatsDlg::eClickHandler);
    m_pwndDelete->pnothRegisterHandler(this, &TEdMCatsDlg::eClickHandler);
    m_pwndList->pnothRegisterHandler(this, &TEdMCatsDlg::eLBHandler);
    m_pwndMType->pnothRegisterHandler(this, &TEdMCatsDlg::eComboHandler);
    m_pwndSave->pnothRegisterHandler(this, &TEdMCatsDlg::eClickHandler);

    // We don't use the column titles but have to set them
    tCIDLib::TStrList colCols(1);
    colCols.objAdd(L"Category Name");
    m_pwndList->SetColumns(colCols);

    // Load up the names list from the database
    tCQCMedia::EMediaTypes eMType = tCQCMedia::EMediaTypes::Min;
    while (eMType <= tCQCMedia::EMediaTypes::Max)
    {
        tCIDLib::TKVPList& colCur = m_objaNames[eMType];

        const tCIDLib::TCard4 c4CatCnt = m_pmdbEdit->c4CatCnt(eMType);
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4CatCnt; c4Index++)
        {
            const TMediaCat& mcatCur = m_pmdbEdit->mcatAt(eMType, c4Index);
            colCur.objAdd
            (
                TKeyValuePair(mcatCur.strName(), mcatCur.strUniqueId())
            );
        }

        // Sort it by the names
        colCur.Sort(TKeyValuePair::eCompKey);

        eMType++;
    }

    //
    //  Load up the supported media types to select from. This will force an initial
    //  load since we've already installed the event handlers.
    //
    m_pwndMType->c4AddItem
    (
        tCQCMedia::strXlatEMediaTypes(tCQCMedia::EMediaTypes::Movie)
        , tCIDLib::c4EnumOrd(tCQCMedia::EMediaTypes::Movie)
    );

    m_pwndMType->c4AddItem
    (
        tCQCMedia::strXlatEMediaTypes(tCQCMedia::EMediaTypes::Music)
        , tCIDLib::c4EnumOrd(tCQCMedia::EMediaTypes::Music)
    );

    // Select the music type initially, force an event to get the list loaded
    m_pwndMType->SelectByIndex(0, kCIDLib::True);

    return bRet;
}


// ---------------------------------------------------------------------------
//  TEdMCatsDlg: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Called when the user pressed the Add button.
//
tCIDLib::TVoid TEdMCatsDlg::AddCategory()
{
    // Get the new name from them
    const TString strPrompt(kMRMgrMsgs::midPrompt_EnterCatName, facMediaRepoMgr);
    TString strName;

    const tCIDLib::TBoolean bRes = facCIDWUtils().bGetText2
    (
        *this
        , L"Add New Category"
        , strPrompt
        , TString::strEmpty()
        , strName
        , L"[a-zA-Z0-9 _\\-]+"
    );

    if (bRes)
    {
        // See if it mathes an existing category. If so no can do
        if (!bCheckDup(strName, m_eType))
        {
            //
            //  Looks ok, so let's add it. We do a sorted insert on the collection
            //  which will tell us where also to insert it into the list window.
            //
            tCIDLib::TKVPList& colList = m_objaNames[m_eType];
            tCIDLib::TCard4 c4InsertPos;

            TKeyValuePair kvalNew(strName, TUniqueId::strMakeId());
            colList.InsertSorted(kvalNew, &TKeyValuePair::eCompKey, c4InsertPos);

            //
            //  Remove list at from its previous place and insert it in the new
            //  place. Tell it to select it at the new position.
            //
            tCIDLib::TStrList colCols(1);
            colCols.objAdd(strName);
            m_pwndList->c4AddItem(colCols, 0, c4InsertPos, kCIDLib::True);
            m_pwndList->SetUserDataAt(c4InsertPos, kvalNew.strValue());

            //
            //  Just to be sure, check the deleted category list. They may have
            //  deleted it and are now putting it back. We don't want it to them
            //  be removed again when we commit.
            //
            tCIDLib::TKVPList& colDelList = m_objaDel[m_eType];
            const tCIDLib::TCard4 c4Count = colDelList.c4ElemCount();
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            {
                // Compare case insensitive, in case they changed the case
                if (colDelList[c4Index].strKey().bCompareI(strName))
                {
                    colDelList.RemoveAt(c4Index);
                    break;
                }
            }
        }
    }
}


//
//  Check the passed name to see if it's a dup for the indicated media type, meaning
//  a duplicate name, case insensitive.
//
tCIDLib::TBoolean
TEdMCatsDlg::bCheckDup( const   TString&                strToCheck
                        , const tCQCMedia::EMediaTypes  eType) const
{
    const tCIDLib::TKVPList& colList = m_objaNames[eType];
    const tCIDLib::TCard4 c4CatCnt = colList.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4CatCnt; c4Index++)
    {
        const TKeyValuePair& kvalCur = colList[c4Index];
        if (strToCheck.bCompareI(kvalCur.strKey()))
        {
            // Show the original name, not the new one we are checking
            TOkBox msgbDup
            (
                facMediaRepoMgr.strMsg(kMRMgrMsgs::midStatus_DupName, kvalCur.strKey())
            );
            msgbDup.ShowIt(*this);
            return kCIDLib::True;
        }
    }
    return kCIDLib::False;
}


//
//  Checks to see if the passed name is for one of the special categories for
//  the indicated media type, one that cannot be edited or deleted.
//
tCIDLib::TBoolean
TEdMCatsDlg::bCheckIsSpecial(const  TString&                strName
                            , const TString&                strUniqueId
                            , const tCQCMedia::EMediaTypes  eType) const
{
    // Note that this may be one they've added here, so it won't be in the DB
    const TMediaCat* pmcatCheck = m_pmdbEdit->pmcatByUniqueId
    (
        eType, strUniqueId, kCIDLib::False
    );

    // If in the DB and it's special, can't do it
    tCQCMedia::EMediaTypes eDummy;
    if (pmcatCheck && facCQCMedia().bIsSpecialCat(pmcatCheck->c2Id(), eDummy))
    {
        TOkBox msgbSpecial
        (
            facMediaRepoMgr.strMsg(kMRMgrMsgs::midStatus_SpecialCategory)
        );
        msgbSpecial.ShowIt(*this);
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


//
//  This is called to commit the changes we have made.
//
tCIDLib::TBoolean TEdMCatsDlg::bCommitChanges()
{
    //
    //  OK, go through the deleted lists and delete any categories that were
    //  removed.
    //
    tCQCMedia::EMediaTypes eMType = tCQCMedia::EMediaTypes::Min;
    while (eMType <= tCQCMedia::EMediaTypes::Max)
    {
        tCIDLib::TKVPList& colCur = m_objaDel[eMType];

        const tCIDLib::TCard4 c4Count = colCur.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const TKeyValuePair& kvalCur = colCur[c4Index];
            const TMediaCat* pmcatDel = m_pmdbEdit->pmcatByUniqueId
            (
                eMType, kvalCur.strValue(), kCIDLib::False
            );

            //
            //  If found, we have to remove all references to this category,
            //  in addition to removing the category itself.
            //
            if (pmcatDel)
            {
                const tCIDLib::TCard2 c2Id = pmcatDel->c2Id();
                m_pmdbEdit->bRemoveObject(eMType, tCQCMedia::EDataTypes::Cat, c2Id);
                m_pmdbEdit->RemoveAllCatRefs(eMType, c2Id);
            }
        }
        eMType++;
    }

    //
    //  Now go through and add any that we don't have already, and update any that
    //  we do, where the name has changed.
    //
    eMType = tCQCMedia::EMediaTypes::Min;
    while (eMType <= tCQCMedia::EMediaTypes::Max)
    {
        tCIDLib::TKVPList& colCur = m_objaNames[eMType];

        const tCIDLib::TCard4 c4Count = colCur.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const TKeyValuePair& kvalCur = colCur[c4Index];

            // See if we can find this guy by unique id
            const TMediaCat* pmcatCur = m_pmdbEdit->pmcatByUniqueId
            (
                eMType, kvalCur.strValue(), kCIDLib::False
            );

            if (pmcatCur)
            {
                // See if the name changed. If so, we need to refresh it
                if (pmcatCur->strName() != kvalCur.strKey())
                {
                    TMediaCat mcatUpdate = *pmcatCur;
                    mcatUpdate.strName(kvalCur.strKey());
                    m_pmdbEdit->UpdateCategory(mcatUpdate);
                }
            }
             else
            {
                // It's got to be new
                TMediaCat* pmcatNew = new TMediaCat
                (
                    eMType, kvalCur.strKey(), kvalCur.strValue()
                );
                m_pmdbEdit->c2AddCategory(pmcatNew);
            }
        }
        eMType++;
    }
    return kCIDLib::True;
}


// Deletes the currently selected category
tCIDLib::TVoid TEdMCatsDlg::DeleteCurrent()
{
    // If no selection, then do nothing
    const tCIDLib::TCard4 c4At = m_pwndList->c4CurItem();
    if (c4At == kCIDLib::c4MaxCard)
        return;

    // Get the one we will be deleting. Get copies so the delete doesn't kill them
    tCIDLib::TKVPList& colList = m_objaNames[m_eType];
    TString strName = colList[c4At].strKey();
    TString strUniqueId = colList[c4At].strValue();

    //
    //  See if this is a special category. If so, they can't delete it. We look it
    //  up by the unique id.
    //
    if (bCheckIsSpecial(strName, strUniqueId, m_eType))
        return;

    // Nothing special, so let then decide whether to delete it
    TYesNoBox msgbDelCat
    (
        facMediaRepoMgr.strMsg(kMRMgrMsgs::midQ_DeleteCat, strName)
    );

    if (msgbDelCat.bShowIt(*this))
    {
        try
        {
            // And remove the current item from our list and the window
            colList.RemoveAt(c4At);
            m_pwndList->RemoveCurrent();

            // Add this one to the list of categories we deleted
            m_objaDel[m_eType].objAdd(TKeyValuePair(strName, strUniqueId));
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
        }
    }
}


// Handle button events
tCIDCtrls ::EEvResponses TEdMCatsDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kMediaRepoMgr::ridDlg_EdMCats_Add)
    {
        AddCategory();
    }
     else if (wnotEvent.widSource() == kMediaRepoMgr::ridDlg_EdMCats_Cancel)
    {
        // Just exit without complaint or comment
        EndDlg(kMediaRepoMgr::ridDlg_EdMCats_Cancel);
    }
     else if (wnotEvent.widSource() == kMediaRepoMgr::ridDlg_EdMCats_Delete)
    {
        DeleteCurrent();
    }
     else if (wnotEvent.widSource() == kMediaRepoMgr::ridDlg_EdMCats_Save)
    {
        // If we can commit the changes, then exit with non-zero
        if (bCommitChanges())
            EndDlg(kMediaRepoMgr::ridDlg_EdMCats_Save);
    }
    return tCIDCtrls::EEvResponses::Handled;
}


// Handle combo box events
tCIDCtrls::EEvResponses TEdMCatsDlg::eComboHandler(TListChangeInfo& wnotEvent)
{
    if ((wnotEvent.eEvent() == tCIDCtrls::EListEvents::SelChanged)
    &&  (wnotEvent.widSource() == kMediaRepoMgr::ridDlg_EdMCats_MType))
    {
        // Store the new media type and reload
        m_eType = tCQCMedia::EMediaTypes(m_pwndMType->c4CurItemId());
        LoadCats();
    }
    return tCIDCtrls::EEvResponses::Handled;
}


// Handle list box events
tCIDCtrls::EEvResponses TEdMCatsDlg::eLBHandler(TListChangeInfo& wnotEvent)
{
    if ((wnotEvent.eEvent() == tCIDCtrls::EListEvents::Invoked)
    &&  (wnotEvent.widSource() == kMediaRepoMgr::ridDlg_EdMCats_List))
    {
        // If they double click on one, we let them edit
        EditCategory(wnotEvent.c4Index());
    }
    return tCIDCtrls::EEvResponses::Handled;
}


//
//  If the user double clicks on a category in the list, this is called to
//  allow the user to edit the name.
//
tCIDLib::TVoid TEdMCatsDlg::EditCategory(const tCIDLib::TCard4 c4At)
{
    //
    //  Get the current list and the selected cat info. Get copies so that a remove
    //  below won't invalidate them.
    //
    tCIDLib::TKVPList& colList = m_objaNames[m_eType];
    TString strName = colList[c4At].strKey();
    const TString strUniqueId = colList[c4At].strValue();

    // If it's a special category, don't allow it
    if (bCheckIsSpecial(strName, strUniqueId, m_eType))
        return;

    // Let the user provide a new name. If they commit, see if we can do it
    const TString strPrompt(kMRMgrMsgs::midPrompt_NewCatName, facMediaRepoMgr);
    const tCIDLib::TBoolean bRes = facCIDWUtils().bGetText2
    (
        *this
        , L"Edit Category"
        , strPrompt
        , TString::strEmpty()
        , strName
        , L"[a-zA-Z0-9 _\\-]+"
    );

    if (bRes)
    {
        //
        //  Just in case, get rid of any white space (including reducing any internal
        //  white space to a single space.
        //
        strName.Strip(kCIDLib::szWhitespace, tCIDLib::EStripModes::Full);

        // Can't be a duplicate
        if (bCheckDup(strName, m_eType))
            return;

        //
        //  We need to get things re-sorted. So remove this one from the list where
        //  it is. Then create a new one with the new name.
        //
        colList.RemoveAt(c4At);
        TKeyValuePair kvalNew(strName, strUniqueId);

        // Insert it again sorted, which gets us the insert index for the next step
        tCIDLib::TCard4 c4InsertPos;
        colList.InsertSorted(kvalNew, &TKeyValuePair::eCompKey, c4InsertPos);

        //
        //  Remove it from the list at from its previous place and insert it in the
        //  new place. Tell it to select it at the new position.
        //
        m_pwndList->RemoveAt(c4At);

        tCIDLib::TStrList colCols(1);
        colCols.objAdd(strName);
        m_pwndList->c4AddItem(colCols, 0, c4InsertPos, kCIDLib::True);
        m_pwndList->SetUserDataAt(c4InsertPos, strUniqueId);
    }
}


//
//  Load the categories for the currently selected type and load them to our
//  category list window. The lists are already sorted.
//
tCIDLib::TVoid TEdMCatsDlg::LoadCats()
{
    try
    {
        m_pwndList->RemoveAll();
        tCIDLib::TKVPList& colList = m_objaNames[m_eType];

        const tCIDLib::TCard4 c4CatCnt = colList.c4ElemCount();
        tCIDLib::TStrList colCols(1);
        colCols.objAdd(TString::strEmpty());
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4CatCnt; c4Index++)
        {
            const TKeyValuePair& kvalCur = colList[c4Index];

            colCols[0] = kvalCur.strKey();
            const tCIDLib::TCard4 c4At = m_pwndList->c4AddItem(colCols, 0);
            m_pwndList->SetUserDataAt(c4At, kvalCur.strValue());
        }

        if (c4CatCnt)
            m_pwndList->SelectByIndex(0);
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
    }
}

