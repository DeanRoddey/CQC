//
// FILE NAME: CQCIGKit_SelDrvDlg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/15/2015
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
//  This file implements a dialog that let's the user choose a driver moniker.
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
#include    "CQCIGKit_SelDrvDlg_.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCSelDrvDlg, TDlgBox)



// ---------------------------------------------------------------------------
//   CLASS: TCQCSelDrvDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCSelDrvDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCSelDrvDlg::TCQCSelDrvDlg() :

    m_pwndCancelButton(nullptr)
    , m_pwndDrivers(nullptr)
    , m_pwndSelectButton(nullptr)
{
}

TCQCSelDrvDlg::~TCQCSelDrvDlg()
{
}


// ---------------------------------------------------------------------------
//  TCQCSelDrvDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  In this one we just take some optional filters and get the list from the name server.
//  This is commonly a reasonable scenario if they want to filter and don't already have
//  a filtered field info cache. And, even so, it might be better, since the info cache
//  will have to download all of the fields for all of the drivers, to filter them and see
//  which ones have any viable fields. IN a large system that's more overhead than this unless
//  the caller has already had to fault in all of the fields in the cache anyway.
//
tCIDLib::TBoolean
TCQCSelDrvDlg::bRun(const   TWindow&            wndOwner
                    ,       TString&            strDriver
                    , const tCQCKit::EDevCats   eDevCat
                    , const TString&            strMake
                    , const TString&            strModel)
{
    // If we have no device category, then set all of the bits since we will take any
    tCQCKit::TDevCatSet btsDevCats;
    if (eDevCat != tCQCKit::EDevCats::Count)
        btsDevCats.bSetBitState(eDevCat, kCIDLib::True);
    else
        btsDevCats.SetAllBits();

    if (!facCQCKit().bFindDrivers(TString::strEmpty(), strMake, strModel, btsDevCats, m_colDrivers))
    {
        TErrBox msgbNone(L"There are no drivers that match the required criteria");
        msgbNone.ShowIt(wndOwner);
        return kCIDLib::False;
    }

    // Sort the list
    m_colDrivers.Sort(TString::eCompI);

    // Store any incoming driver for initial selection
    m_strDriver = strDriver;

    const tCIDLib::TCard4 c4Res = c4RunDlg
    (
        wndOwner, facCQCIGKit(), kCQCIGKit::ridDlg_SelDrv
    );

    if (c4Res == kCQCIGKit::ridDlg_SelDrv_Select)
    {
        strDriver = m_strDriver;
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


tCIDLib::TBoolean
TCQCSelDrvDlg::bRun(const   TWindow&        wndOwner
                    , const TCQCFldCache&   cfcFldInfo
                    ,       TString&        strDriver)
{
    // Get the list of drivers in the field cache
    cfcFldInfo.QueryDevList(m_colDrivers);
    if (m_colDrivers.bIsEmpty())
    {
        TErrBox msgbNone(L"There are no drivers that match the required criteria");
        msgbNone.ShowIt(wndOwner);
        return kCIDLib::False;
    }

    // Sort the list
    m_colDrivers.Sort(&TString::eCompI);

    // Store any incoming driver for initial selection
    m_strDriver = strDriver;
    const tCIDLib::TCard4 c4Res = c4RunDlg
    (
        wndOwner, facCQCIGKit(), kCQCIGKit::ridDlg_SelDrv
    );

    if (c4Res == kCQCIGKit::ridDlg_SelDrv_Select)
    {
        strDriver = m_strDriver;
        return kCIDLib::True;
    }
    return kCIDLib::False;
}



// ---------------------------------------------------------------------------
//  TCQCSelDrvDlg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TCQCSelDrvDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRet = TParent::bCreated();

    // First get typed pointers to the widget we always keep
    CastChildWnd(*this, kCQCIGKit::ridDlg_SelDrv_Cancel, m_pwndCancelButton);
    CastChildWnd(*this, kCQCIGKit::ridDlg_SelDrv_Drivers, m_pwndDrivers);
    CastChildWnd(*this, kCQCIGKit::ridDlg_SelDrv_Select, m_pwndSelectButton);

    // Set the columns on the driver list. We have one column
    tCIDLib::TStrList colCols(1);
    colCols.objAdd(facCQCIGKit().strMsg(kIGKitMsgs::midDlg_SelDrv_Drivers));
    m_pwndDrivers->SetColumns(colCols);

    const tCIDLib::TCard4 c4DrvCount = m_colDrivers.c4ElemCount();
    tCIDLib::TCard4 c4InitSel = kCIDLib::c4MaxCard;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4DrvCount; c4Index++)
    {
        colCols[0] = m_colDrivers[c4Index];
        m_pwndDrivers->c4AddItem(colCols, c4Index);
        if (colCols[0].bCompareI(m_strDriver))
            c4InitSel = c4Index;
    }

    // Select the initial one we found above, if any. else just the 0th one
    if (c4InitSel == kCIDLib::c4MaxCard)
        m_pwndDrivers->SelectByIndex(0);
    else
        m_pwndDrivers->c4SelectById(c4InitSel);

    // Register our handlers
    m_pwndCancelButton->pnothRegisterHandler(this, &TCQCSelDrvDlg::eClickHandler);
    m_pwndSelectButton->pnothRegisterHandler(this, &TCQCSelDrvDlg::eClickHandler);
    m_pwndDrivers->pnothRegisterHandler(this, &TCQCSelDrvDlg::eLBHandler);

    return bRet;
}


// ---------------------------------------------------------------------------
//  TCQCSelDrvDlg: Private, non-virutal methods
// ---------------------------------------------------------------------------
tCIDCtrls::EEvResponses
TCQCSelDrvDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCIGKit::ridDlg_SelDrv_Cancel)
    {
        EndDlg(kCQCIGKit::ridDlg_SelDrv_Cancel);
    }
     else if (wnotEvent.widSource() == kCQCIGKit::ridDlg_SelDrv_Select)
    {
        // If we have something in both lists, we are ok
        const tCIDLib::TCard4 c4DrvIndex = m_pwndDrivers->c4CurItem();

        if (c4DrvIndex != kCIDLib::c4MaxCard)
        {
            m_pwndDrivers->QueryColText(c4DrvIndex, 0, m_strDriver);
            EndDlg(kCQCIGKit::ridDlg_SelDrv_Select);
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}


tCIDCtrls::EEvResponses TCQCSelDrvDlg::eLBHandler(TListChangeInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCIGKit::ridDlg_SelDrv_Drivers)
    {
        if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::Invoked)
        {
            m_pwndDrivers->QueryColText(wnotEvent.c4Index(), 0, m_strDriver);
            EndDlg(kCQCIGKit::ridDlg_SelDrv_Select);
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}
