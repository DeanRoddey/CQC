//
// FILE NAME: CQCIntfEd_PasteAttrsDlg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/27/2016
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
//  This file implements a dialog that let's the user select attributes to paste.
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
#include    "CQCIntfEd_.hpp"
#include    "CQCIntfEd_PasteAttrsDlg_.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TPasteAttrsDlg, TDlgBox)



// ---------------------------------------------------------------------------
//   CLASS: TPasteAttrsDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TPasteAttrsDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TPasteAttrsDlg::TPasteAttrsDlg() :

    m_pcolSelected(nullptr)
    , m_piapAvail(nullptr)
    , m_pwndApply(nullptr)
    , m_pwndCancel(nullptr)
    , m_pwndTree(nullptr)
{
}

TPasteAttrsDlg::~TPasteAttrsDlg()
{
}


// ---------------------------------------------------------------------------
//  TPasteAttrsDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TPasteAttrsDlg::bRun(const  TWindow&                wndOwner
                    , const TCQCIntfAttrPack&       iapAvail
                    ,       tCIDMData::TAttrList&   colSelected)
{
    // Store incoming stuff for later use
    m_pcolSelected = &colSelected;
    m_piapAvail = &iapAvail;

    const tCIDLib::TCard4 c4Res = c4RunDlg
    (
        wndOwner, facCQCIntfEd(), kCQCIntfEd::ridDlg_PasteAttrs
    );
    return (c4Res == kCQCIntfEd::ridDlg_PasteAttrs_Apply);
}


// ---------------------------------------------------------------------------
//  TPasteAttrsDlg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TPasteAttrsDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRet = TParent::bCreated();

    // First get typed pointers to the widget we always keep
    CastChildWnd(*this, kCQCIntfEd::ridDlg_PasteAttrs_Apply, m_pwndApply);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_PasteAttrs_Cancel, m_pwndCancel);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_PasteAttrs_Tree, m_pwndTree);

    // Register our handlers
    m_pwndApply->pnothRegisterHandler(this, &TPasteAttrsDlg::eClickHandler);
    m_pwndCancel->pnothRegisterHandler(this, &TPasteAttrsDlg::eClickHandler);
    m_pwndTree->pnothRegisterHandler(this, &TPasteAttrsDlg::eTreeHandler);

    //
    //  Load up any incoming attributes. Separators become scopes in the tree. The list is
    //  only one layer deep, so we don't have to figure out when a scope ends. We'll just
    //  see the next separator.
    //
    TString strCurPar;
    const tCIDLib::TCard4 c4Count = m_piapAvail->c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TAttrData& attrCur = m_piapAvail->attrAt(c4Index);

        if (attrCur.eType() == tCIDMData::EAttrTypes::Separator)
        {
            // Add a scope
            m_pwndTree->AddScope(L"/", attrCur.strName(), kCIDLib::False);

            // Set this as the current parent scope
            strCurPar = L"/";
            strCurPar.Append(attrCur.strName());
        }
         else
        {
            //
            //  Add this one to the current scope. Store the original index as the user
            //  data on this item.
            //
            m_pwndTree->AddItem(strCurPar, attrCur.strName(), c4Index);
        }
    }

    // Expand the root so the user doesn't have to
    m_pwndTree->bExpandPath(L"/", strCurPar);

    return bRet;
}


// ---------------------------------------------------------------------------
//  TPasteAttrsDlg: Private, non-virutal methods
// ---------------------------------------------------------------------------
tCIDCtrls::EEvResponses TPasteAttrsDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCIntfEd::ridDlg_PasteAttrs_Apply)
    {
        //
        //  Get a list of all of the items in the tree that are checked, and their
        //  user flags, in which we stored the user
        //
        tCIDLib::TStrList colSel;
        tCIDLib::TCardList fcolSelIds;
        const tCIDLib::TCard4 c4Count = m_pwndTree->c4QueryAllChecked(colSel, fcolSelIds);
        if (c4Count)
        {
            // Pull out the attributes they kept and put into the caller's list
            m_pcolSelected->RemoveAll();
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            {
                const TAttrData& attrCur = m_piapAvail->attrAt(fcolSelIds[c4Index]);

                // Skip separators, only the actual attributes matter
                if (attrCur.eType() != tCIDMData::EAttrTypes::Separator)
                    m_pcolSelected->objAdd(attrCur);
            }

            EndDlg(kCQCIntfEd::ridDlg_PasteAttrs_Apply);
        }
         else
        {
            TErrBox msgbErr(L"You have not selected any attributes to apply");
            msgbErr.ShowIt(*this);
        }
    }
     else if (wnotEvent.widSource() == kCQCIntfEd::ridDlg_PasteAttrs_Cancel)
    {
        EndDlg(kCQCIntfEd::ridDlg_PasteAttrs_Cancel);
    }

    return tCIDCtrls::EEvResponses::Handled;
}


tCIDCtrls::EEvResponses TPasteAttrsDlg::eTreeHandler(TTreeEventInfo& wnotEvent)
{
    //
    //  If we see a check state change, and it's a scope, then we check/uncheck all of the
    //  direct, non-scope children it has. If it's the root, then we check/uncheck everything.
    //
    if (wnotEvent.eEvent() == tCIDCtrls::ETreeEvents::CheckState)
    {
        if (m_pwndTree->bIsScope(wnotEvent.strPath()))
        {
            if (wnotEvent.strPath() == L"/")
                m_pwndTree->SetAllChecks(wnotEvent.bState());
            else
                m_pwndTree->SetAllChildChecks(wnotEvent.strPath(), wnotEvent.bState());
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}


