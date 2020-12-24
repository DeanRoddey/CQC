//
// FILE NAME: ZWaveUSBC_EdUnitDlg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/11/2007
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
//  This file implements the unit editing dialog
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
#include    "ZWaveUSBC_.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TZWaveEdUnitDlg,TDlgBox)



// ---------------------------------------------------------------------------
//   CLASS: TZWaveEdUnitDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWaveEdUnitDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TZWaveEdUnitDlg::TZWaveEdUnitDlg() :

    m_punitToEdit(nullptr)
    , m_pwndCancel(nullptr)
    , m_pwndName(nullptr)
    , m_pwndPolled(nullptr)
    , m_pwndPollInt(nullptr)
    , m_pwndSave(nullptr)
{
}

TZWaveEdUnitDlg::~TZWaveEdUnitDlg()
{
}


// ---------------------------------------------------------------------------
//  TZWaveEdUnitDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TZWaveEdUnitDlg::bRunDlg(const  TWindow&    wndOwner
                        , const TString&    strMoniker
                        ,       TZWaveUnit& unitToEdit)
{
    // Save the moniker and a pointer to the incoming unit
    m_strMoniker = strMoniker;
    m_punitToEdit = &unitToEdit;

    //
    //  We got it, so try to run the dialog. If successful and we get a
    //  positive response, then fill in the caller's parameter with the
    //  stored IR event key.
    //
    const tCIDLib::TCard4 c4Res = c4RunDlg
    (
        wndOwner, facZWaveUSBC(), kZWaveUSBC::ridDlg_EdUnit
    );

    return (c4Res == kZWaveUSBC::ridDlg_EdUnit_Save);
}


// ---------------------------------------------------------------------------
//  TZWaveEdUnitDlg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TZWaveEdUnitDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRet = TParent::bCreated();

    // Store our pre-typed child widget pointers
    CastChildWnd(*this, kZWaveUSBC::ridDlg_EdUnit_Cancel, m_pwndCancel);
    CastChildWnd(*this, kZWaveUSBC::ridDlg_EdUnit_Name, m_pwndName);
    CastChildWnd(*this, kZWaveUSBC::ridDlg_EdUnit_Polled, m_pwndPolled);
    CastChildWnd(*this, kZWaveUSBC::ridDlg_EdUnit_PollInt, m_pwndPollInt);
    CastChildWnd(*this, kZWaveUSBC::ridDlg_EdUnit_Save, m_pwndSave);

    // And register our button event handlers
    m_pwndCancel->pnothRegisterHandler(this, &TZWaveEdUnitDlg::eClickHandler);
    m_pwndSave->pnothRegisterHandler(this, &TZWaveEdUnitDlg::eClickHandler);

    // Set up some info in the title bar
    TString strTitleInfo(strWndText());
    strTitleInfo.eReplaceToken(m_punitToEdit->c1Id(), kCIDLib::chDigit1);
    strTitleInfo.eReplaceToken(m_punitToEdit->strTypeName(), kCIDLib::chDigit2);
    strWndText(strTitleInfo);

    // Load up the name
    m_pwndName->strWndText(m_punitToEdit->strName());

    //
    //  If this unit is pollable, then set up the polling stuff, else
    //  we disable them.
    //
    if (m_punitToEdit->bHasCap(tZWaveUSBSh::EUnitCap_Pollable))
    {
        // Load up the poll interval values
        m_pwndPollInt->SetNumRange(2, 120, 2);

        //
        //  Get the current poll interval. It's stored as a time stamp in
        //  the unit, and the minimum should be 2 seconds (when it is is
        //  non-zero.) Since we start at 2 seconds when we load the spin box
        //  then zero represents 2 seconds and so forth. So we substract
        //  2 below when we select the item.
        //
        tCIDLib::TInt4 i4PollInt = tCIDLib::TInt4
        (
            m_punitToEdit->enctPollPeriod() / kCIDLib::enctOneSecond
        );

        if (i4PollInt < 2)
            i4PollInt = 2;
        else if (i4PollInt > 120)
            i4PollInt = 120;

        // Load the current polling info
        if (m_punitToEdit->enctPollPeriod())
        {
            m_pwndPolled->SetCheckedState(kCIDLib::True);
            m_pwndPollInt->SetValue(i4PollInt);
        }
         else
        {
            m_pwndPolled->SetCheckedState(kCIDLib::False);
        }
    }
     else
    {
        m_pwndPolled->SetEnable(kCIDLib::False);
        m_pwndPollInt->SetEnable(kCIDLib::False);
    }

    return bRet;
}


// ---------------------------------------------------------------------------
//  TZWaveEdUnitDlg: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDCtrls::EEvResponses TZWaveEdUnitDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kZWaveUSBC::ridDlg_EdUnit_Cancel)
    {
        EndDlg(kZWaveUSBC::ridDlg_EdUnit_Cancel);
    }
     else if (wnotEvent.widSource() == kZWaveUSBC::ridDlg_EdUnit_Save)
    {
        //
        //  Sanity check the info, which mainly just means making sure that
        //  the name isn't less than 3 character long, and it's just characters that
        //  are valid for field names.
        //
        TString strName = m_pwndName->strWndText();
        if (strName.c4Length() < 3)
        {
            TOkBox msgbWarn(facZWaveUSBSh().strMsg(kZWShErrs::errcCfg_NameLength));
            msgbWarn.ShowIt(*this);
            return tCIDCtrls::EEvResponses::Handled;
        }

        if (!facCQCKit().bIsValidBaseFldName(strName))
        {
            TOkBox msgbWarn(facZWaveUSBSh().strMsg(kZWShErrs::errcCfg_UnitNameChars));
            msgbWarn.ShowIt(*this);
            return tCIDCtrls::EEvResponses::Handled;
        }

        // Name looks ok so we can store the info
        m_punitToEdit->strName(strName);
        if (m_pwndPolled->bCheckedState())
            m_punitToEdit->enctPollPeriod(m_pwndPollInt->i4CurValue() * kCIDLib::enctOneSecond);
        else
            m_punitToEdit->enctPollPeriod(0);

        // And end with a save status
        EndDlg(kZWaveUSBC::ridDlg_EdUnit_Save);
    }
    return tCIDCtrls::EEvResponses::Handled;
}


