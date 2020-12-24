//
// FILE NAME: MediaRepoMgr_EdMItemDlg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/31/2006
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
//  This is the header for the MediaRepoMGr_EdMItemDlg.cpp file, which
//  allows the user to edit media title objects.
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
#include    "MediaRepoMgr_EdMItemDlg.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TEdMItemDlg,TDlgBox)


// ---------------------------------------------------------------------------
//   CLASS: TEdMItemDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TEdMItemDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TEdMItemDlg::TEdMItemDlg() :

    m_pmcolParent(nullptr)
    , m_pwndArtist(nullptr)
    , m_pwndCancel(nullptr)
    , m_pwndDuration(nullptr)
    , m_pwndSave(nullptr)
    , m_pwndTitle(nullptr)
{
}

TEdMItemDlg::~TEdMItemDlg()
{
}


// ---------------------------------------------------------------------------
//  TEdMItemDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------

// This is called to edit an existing item
tCIDLib::TBoolean
TEdMItemDlg::bRun(  const   TWindow&        wndOwner
                    , const TMediaCollect&  mcolParent
                    ,       TMediaItem&     mitemToEdit)
{
    // Store the incoming stuff for use
    m_eMType = mcolParent.eType();
    m_pmcolParent = &mcolParent;

    // Copy the incoming for editing
    m_mitemEdit = mitemToEdit;

    // And start the dialog
    const tCIDLib::TCard4 c4Res = c4RunDlg
    (
        wndOwner, facMediaRepoMgr, kMediaRepoMgr::ridDlg_EdMItem
    );

    if (c4Res == kMediaRepoMgr::ridDlg_EdMItem_Save)
    {
        // Copy the changes back
        mitemToEdit = m_mitemEdit;
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


// ---------------------------------------------------------------------------
//  TEdMItemDlg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TEdMItemDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRet = TParent::bCreated();

    // Get typed pointers to some of the widgets
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_EdMItem_Artist, m_pwndArtist);
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_EdMItem_Cancel, m_pwndCancel);
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_EdMItem_Duration, m_pwndDuration);
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_EdMItem_Save, m_pwndSave);
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_EdMItem_Title, m_pwndTitle);

    //
    //  If it's a new one, set up the parent's artist name as the most likely default
    //  value.
    //
    if (m_mitemEdit.strArtist().bIsEmpty())
        m_pwndArtist->strWndText(m_pmcolParent->strArtist());

    m_pwndArtist->strWndText(m_mitemEdit.strArtist());
    m_pwndTitle->strWndText(m_mitemEdit.strName());

    TString strFmt;
    strFmt.SetFormatted(m_mitemEdit.c4Duration());
    m_pwndDuration->strWndText(strFmt);

    // Register handlers
    m_pwndCancel->pnothRegisterHandler(this, &TEdMItemDlg::eClickHandler);
    m_pwndSave->pnothRegisterHandler(this, &TEdMItemDlg::eClickHandler);

    return bRet;
}


// ---------------------------------------------------------------------------
//  TEdMItemDlg: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  We get the values out of the entry fields, validate them, and store them
//  in our temp item. If they are all valid, we return true, else false.
//
tCIDLib::TBoolean TEdMItemDlg::bValidate()
{
    tCIDLib::TCard4 c4Duration;
    TWindow*        pwndErr = 0;
    TString         strErr;

    // Make sure that the title is non-empty
    if (m_pwndTitle->strWndText().bIsEmpty())
    {
        pwndErr = m_pwndTitle;
        strErr = facMediaRepoMgr.strMsg(kMRMgrMsgs::midStatus_RequiredField);
    }
     else if (m_pwndArtist->strWndText().bIsEmpty())
    {
        pwndErr = m_pwndArtist;
        strErr = facMediaRepoMgr.strMsg(kMRMgrMsgs::midStatus_RequiredField);
    }
     else
    {
        if (!pwndErr)
        {
            try
            {
                c4Duration = m_pwndDuration->strWndText().c4Val(tCIDLib::ERadices::Dec);
            }

            catch(...)
            {
                strErr = facMediaRepoMgr.strMsg(kMRMgrMsgs::midStatus_BadField);
                pwndErr = m_pwndDuration;
            }
        }
    }

    // We have an error, so deal with it
    if (pwndErr)
    {
        // Show the error to the user
        {
            TErrBox msgbErr(strWndText(), strErr);
            msgbErr.ShowIt(*this);
        }

        // And put the focus on the field in error
        pwndErr->TakeFocus();
        return kCIDLib::False;
    }

    // It looks good, so store the data
    m_mitemEdit.strName(m_pwndTitle->strWndText());
    m_mitemEdit.c4Duration(c4Duration);
    m_mitemEdit.strArtist(m_pwndArtist->strWndText());

    return kCIDLib::True;
}


tCIDCtrls::EEvResponses TEdMItemDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kMediaRepoMgr::ridDlg_EdMItem_Cancel)
    {
        EndDlg(kMediaRepoMgr::ridDlg_EdMItem_Cancel);
    }
     else if (wnotEvent.widSource() == kMediaRepoMgr::ridDlg_EdMItem_Save)
    {
        if (bValidate())
            EndDlg(kMediaRepoMgr::ridDlg_EdMItem_Save);
    }
    return tCIDCtrls::EEvResponses::Handled;
}

