//
// FILE NAME: CQCIGKit_CmdReplaceDlg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/07/2010
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
//  This file implements the search/replace options dialog for the action editor.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $_CIDLib_Log_$
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCIGKit_.hpp"
#include    "CQCIGKit_CmdReplaceDlg_.hpp"



// ----------------------------------------------------------------------------
//  Do our RTTI macros
// ----------------------------------------------------------------------------
RTTIDecls(TActCmdReplaceDlg,TCQCDlg)



// ---------------------------------------------------------------------------
//  CLASS: TActCmdReplaceDlg
// PREFIX: dlgb
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TActCmdReplaceDlg: Constructors and Destructors
// ---------------------------------------------------------------------------
TActCmdReplaceDlg::TActCmdReplaceDlg() :

    m_bRegEx(kCIDLib::False)
    , m_bFullMatch(kCIDLib::True)
    , m_bThisEvent(kCIDLib::True)
{
}

TActCmdReplaceDlg::~TActCmdReplaceDlg()
{
}


// ---------------------------------------------------------------------------
//  TActCmdReplaceDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TActCmdReplaceDlg::bRunDlg( TWindow&                wndOwner
                            , tCIDLib::TBoolean&    bFullMatch
                            , tCIDLib::TBoolean&    bRegEx
                            , tCIDLib::TBoolean&    bThisEvent
                            , TString&              strFindStr
                            , TString&              strRepStr)
{
    // Store any incoming values we need
    m_bFullMatch = bFullMatch;
    m_bRegEx = bRegEx;
    m_bThisEvent = bThisEvent;
    m_strFindStr = strFindStr;
    m_strRepStr = strRepStr;

    const tCIDLib::TCard4 c4Res = c4RunDlg
    (
        wndOwner, facCQCIGKit(), kCQCIGKit::ridDlg_CmdRep
    );

    if (c4Res == kCQCIGKit::ridDlg_CmdRep_Replace)
    {
        // Copy new values back to the caller's parms, since we are committing
        bFullMatch  = m_bFullMatch;
        bRegEx      = m_bRegEx;
        bThisEvent  = m_bThisEvent;
        strFindStr  = m_strFindStr;
        strRepStr   = m_strRepStr;

        return kCIDLib::True;
    }
    return kCIDLib::False;
}


// ---------------------------------------------------------------------------
//  TActCmdReplaceDlg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TActCmdReplaceDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRet = TParent::bCreated();

    // Get typed convenience pointers to some of our widgets
    CastChildWnd(*this, kCQCIGKit::ridDlg_CmdRep_AllEvents, m_pwndAllEvents);
    CastChildWnd(*this, kCQCIGKit::ridDlg_CmdRep_Cancel, m_pwndCancel);
    CastChildWnd(*this, kCQCIGKit::ridDlg_CmdRep_FullMatch, m_pwndFullMatch);
    CastChildWnd(*this, kCQCIGKit::ridDlg_CmdRep_RegEx, m_pwndRegEx);
    CastChildWnd(*this, kCQCIGKit::ridDlg_CmdRep_ThisEvent, m_pwndThisEvent);
    CastChildWnd(*this, kCQCIGKit::ridDlg_CmdRep_FindStr, m_pwndFindStr);
    CastChildWnd(*this, kCQCIGKit::ridDlg_CmdRep_RepStr, m_pwndRepStr);
    CastChildWnd(*this, kCQCIGKit::ridDlg_CmdRep_Replace, m_pwndReplace);

    // Set any incoming info
    m_pwndFindStr->strWndText(m_strFindStr);
    m_pwndRepStr->strWndText(m_strRepStr);

    // Initially set the just this event radio button
    if (m_bThisEvent)
        m_pwndThisEvent->SetCheckedState(kCIDLib::True);
    else
        m_pwndAllEvents->SetCheckedState(kCIDLib::True);

    m_pwndFullMatch->SetCheckedState(m_bFullMatch);
    m_pwndRegEx->SetCheckedState(m_bRegEx);

    //
    //  Install handlers for the push buttons. The rest just hold what they
    //  hold and we'll check them at the end if necessary.
    //
    m_pwndCancel->pnothRegisterHandler(this, &TActCmdReplaceDlg::eClickHandler);
    m_pwndReplace->pnothRegisterHandler(this, &TActCmdReplaceDlg::eClickHandler);

    return bRet;
}


// ---------------------------------------------------------------------------
//  TActCmdReplaceDlg: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDCtrls::EEvResponses
TActCmdReplaceDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCIGKit::ridDlg_CmdRep_Replace)
    {
        // Get the options together and check them for validity
        const tCIDLib::TBoolean bDoFullMatch = m_pwndFullMatch->bCheckedState();
        const tCIDLib::TBoolean bDoRegEx     = m_pwndRegEx->bCheckedState();
        const TString&          strFindStr   = m_pwndFindStr->strWndText();
        const TString&          strRepStr    = m_pwndRepStr->strWndText();

        // The find string cannot be empty if reg ex or not full match
        if (strFindStr.bIsEmpty() && (bDoRegEx || !bDoFullMatch))
        {
            // Make sure that they really want to remove it
            TErrBox msgError
            (
                strWndText(), facCQCIGKit().strMsg(kIGKitMsgs::midStatus_SearchEmptyStr)
            );
            msgError.ShowIt(*this);
            return tCIDCtrls::EEvResponses::Handled;
        }

        // If a reg ex, make sure it's valid
        if (bDoRegEx)
        {
            try
            {
                TRegEx regxTest;
                regxTest.SetExpression(strFindStr);
            }

            catch(...)
            {
                TErrBox msgError
                (
                    strWndText()
                    , facCQCIGKit().strMsg(kIGKitMsgs::midStatus_BadRegEx)
                );
                msgError.ShowIt(*this);
                return tCIDCtrls::EEvResponses::Handled;
            }
        }

        // Get the values back out till we can get them back to the caller
        m_bFullMatch  = m_pwndFullMatch->bCheckedState();
        m_bRegEx      = m_pwndRegEx->bCheckedState();
        m_bThisEvent  = m_pwndThisEvent->bCheckedState();
        m_strFindStr  = m_pwndFindStr->strWndText();
        m_strRepStr   = m_pwndRepStr->strWndText();

        EndDlg(kCQCIGKit::ridDlg_CmdRep_Replace);
    }
     else if (wnotEvent.widSource() == kCQCIGKit::ridDlg_CmdRep_Cancel)
    {
        EndDlg(kCQCIGKit::ridDlg_CmdRep_Cancel);
    }
    return tCIDCtrls::EEvResponses::Handled;
}



