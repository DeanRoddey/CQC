//
// FILE NAME: CQCAdmin_UserLimitsDlg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 05/20/2016
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
//  This file implements a dialog that edits per-user limits used by things like the
//  slider and volume knob.
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
#include    "CQCIntfEd_UserLimitsDlg_.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TUserLimitsDlg, TDlgBox)



// ---------------------------------------------------------------------------
//   CLASS: TUserLimitsDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TUserLimitsDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TUserLimitsDlg::TUserLimitsDlg() :

    m_pwndCancelButton(nullptr)
    , m_pwndRoles(nullptr)
    , m_pwndDispMax(nullptr)
    , m_pwndDispMin(nullptr)
    , m_pwndResetButton(nullptr)
    , m_pwndSetMax(nullptr)
    , m_pwndSetMin(nullptr)
    , m_pwndSaveButton(nullptr)
{
}

TUserLimitsDlg::~TUserLimitsDlg()
{
}


// ---------------------------------------------------------------------------
//  TUserLimitsDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TUserLimitsDlg::bRun(const  TWindow&                wndOwner
                    ,       tCQCIntfEng::TUserLims& fcolVals
                    , const tCIDLib::TInt4          i4MinLimit
                    , const tCIDLib::TInt4          i4MaxLimit)
{
    m_fcolOrg = fcolVals;
    m_fcolVals = fcolVals;
    m_i4MinLimit = i4MinLimit;
    m_i4MaxLimit = i4MaxLimit;

    const tCIDLib::TCard4 c4Res = c4RunDlg
    (
        wndOwner, facCQCIntfEd(), kCQCIntfEd::ridDlg_Limits
    );

    if (c4Res == kCQCIntfEd::ridDlg_Limits_Save)
    {
        // Copy back the edited stuff
        fcolVals = m_fcolVals;
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


// ---------------------------------------------------------------------------
//  TUserLimitsDlg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TUserLimitsDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRet = TParent::bCreated();

    // First get typed pointers to the widget we always keep
    CastChildWnd(*this, kCQCIntfEd::ridDlg_Limits_Cancel, m_pwndCancelButton);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_Limits_MaxDisp, m_pwndDispMax);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_Limits_MinDisp, m_pwndDispMin);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_Limits_ResetAll, m_pwndResetButton);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_Limits_Roles, m_pwndRoles);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_Limits_SetMax, m_pwndSetMax);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_Limits_SetMin, m_pwndSetMin);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_Limits_Save, m_pwndSaveButton);

    // Load the combo box with the user types
    tCQCKit::EUserRoles eRole = tCQCKit::EUserRoles::Min;
    for (; eRole <= tCQCKit::EUserRoles::Max; eRole++)
        m_pwndRoles->c4AddItem(tCQCKit::strLoadEUserRoles(eRole));

    // Register our handlers
    m_pwndCancelButton->pnothRegisterHandler(this, &TUserLimitsDlg::eClickHandler);
    m_pwndResetButton->pnothRegisterHandler(this, &TUserLimitsDlg::eClickHandler);
    m_pwndSaveButton->pnothRegisterHandler(this, &TUserLimitsDlg::eClickHandler);

    m_pwndRoles->pnothRegisterHandler(this, &TUserLimitsDlg::eListHandler);
    m_pwndSetMax->pnothRegisterHandler(this, &TUserLimitsDlg::eSliderHandler);
    m_pwndSetMin->pnothRegisterHandler(this, &TUserLimitsDlg::eSliderHandler);


    // Select the 0th item and force an event to get the per-level stuff set up
    m_pwndRoles->SelectByIndex(0, kCIDLib::True);

    return bRet;
}


// ---------------------------------------------------------------------------
//  TUserLimitsDlg: Private, non-virutal methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TUserLimitsDlg::bValidate() const
{
    // Make sure that none of the values are equal or inverted
    tCQCKit::EUserRoles eRole = tCQCKit::EUserRoles::Min;
    for (; eRole <= tCQCKit::EUserRoles::Max; eRole++)
    {
        const tCIDLib::TCard4 c4Lims = m_fcolVals[eRole];
        const tCIDLib::TInt4 i4Min = TRawBits::i2Low16From32(c4Lims);
        const tCIDLib::TInt4 i4Max = TRawBits::i2High16From32(c4Lims);

        if (i4Max <= i4Min)
        {
            m_pwndRoles->SelectByIndex(tCIDLib::c4EnumOrd(eRole), kCIDLib::True);

            TErrBox msgbErr(L"The maximum cannot be <= than the minimum level");
            msgbErr.ShowIt(*this);
            return kCIDLib::False;
        }
    }
    return kCIDLib::True;
}


tCIDCtrls::EEvResponses TUserLimitsDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCIntfEd::ridDlg_Limits_Cancel)
    {
        if (m_fcolVals != m_fcolOrg)
        {
            TYesNoBox msgbDiscard(strWndText(), facCQCIntfEd().strMsg(kIEdMsgs::midQ_Discard));
            if (msgbDiscard.bShowIt(*this))
                EndDlg(kCQCIntfEd::ridDlg_Limits_Cancel);
        }
         else
        {
            EndDlg(kCQCIntfEd::ridDlg_Limits_Cancel);
        }
    }
     else if (wnotEvent.widSource() == kCQCIntfEd::ridDlg_Limits_Save)
    {
        if (bValidate())
            EndDlg(kCQCIntfEd::ridDlg_Limits_Save);
    }
     else if (wnotEvent.widSource() == kCQCIntfEd::ridDlg_Limits_ResetAll)
    {
        const tCIDLib::TCard4 c4DefLim = TRawBits::c4From16
        (
            tCIDLib::TCard2(m_i4MinLimit), tCIDLib::TCard2(m_i4MaxLimit)
        );
        tCQCKit::EUserRoles eRole = tCQCKit::EUserRoles::Min;
        for (; eRole <= tCQCKit::EUserRoles::Max; eRole++)
            m_fcolVals[eRole] = c4DefLim;

        // Reselect the current user role and force it to update
        m_pwndRoles->SelectByIndex(m_pwndRoles->c4CurItem(), kCIDLib::True);
    }
    return tCIDCtrls::EEvResponses::Handled;
}


tCIDCtrls::EEvResponses TUserLimitsDlg::eListHandler(TListChangeInfo& wnotEvent)
{
    if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::SelChanged)
    {
        if (wnotEvent.widSource() == kCQCIntfEd::ridDlg_Limits_Roles)
            SetLimitSliders();
    }
    return tCIDCtrls::EEvResponses::Handled;
}


tCIDCtrls::EEvResponses TUserLimitsDlg::eSliderHandler(TSliderChangeInfo& wnotEvent)
{
    if ((wnotEvent.eEvent() == tCIDCtrls::ESldrEvents::Change)
    ||  (wnotEvent.eEvent() == tCIDCtrls::ESldrEvents::EndTrack)
    ||  (wnotEvent.eEvent() == tCIDCtrls::ESldrEvents::Track))
    {
        // Update the working data
        const tCQCKit::EUserRoles eRole = tCQCKit::EUserRoles(m_pwndRoles->c4CurItem());

        if (wnotEvent.widSource() == kCQCIntfEd::ridDlg_Limits_SetMax)
        {
            // Replace the high word
            m_fcolVals[eRole] &= 0x0000FFFF;
            m_fcolVals[eRole] |= wnotEvent.i4Value() << 16;

            // Update the display value
            TString strVal;
            strVal.SetFormatted(wnotEvent.i4Value());
            m_pwndDispMax->strWndText(strVal);
        }
         else
        {
            // Replace the low word
            m_fcolVals[eRole] &= 0xFFFF0000;
            m_fcolVals[eRole] |= wnotEvent.i4Value() & 0xFFFF;

            // Update the display value
            TString strVal;
            strVal.SetFormatted(wnotEvent.i4Value());
            m_pwndDispMin->strWndText(strVal);
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}


tCIDLib::TVoid TUserLimitsDlg::SetLimitSliders()
{
    // Query the current limits for the selected user type
    const tCQCKit::EUserRoles eRole = tCQCKit::EUserRoles(m_pwndRoles->c4CurItem());

    const tCIDLib::TCard4 c4Lims = m_fcolVals[eRole];
    tCIDLib::TInt4 i4Min = TRawBits::i2Low16From32(c4Lims);
    tCIDLib::TInt4 i4Max = TRawBits::i2High16From32(c4Lims);

    // Update the numeric displays
    TString strVal;
    strVal.SetFormatted(i4Max);
    m_pwndDispMax->strWndText(strVal);
    strVal.SetFormatted(i4Min);
    m_pwndDispMin->strWndText(strVal);

    // And set up the sliders
    m_pwndSetMin->SetRange(m_i4MinLimit, m_i4MaxLimit);
    m_pwndSetMax->SetRange(m_i4MinLimit, m_i4MaxLimit);
    m_pwndSetMin->SetValue(i4Min);
    m_pwndSetMax->SetValue(i4Max);
}

