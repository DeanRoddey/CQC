//
// FILE NAME: CQCKit_GetVersionDlg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 12/21/2003
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
//  This small dialog allows the user to select a target version from a list of versions.
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
#include "CQCGKit_.hpp"
#include "CQCGKit_GetVersionDlg_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TGetVersionDlg,TDlgBox)


// ---------------------------------------------------------------------------
//   CLASS: TGetVersionDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TGetVersionDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TGetVersionDlg::TGetVersionDlg() :

    m_c8RetVal(0)
    , m_pwndAccept(nullptr)
    , m_pwndCancel(nullptr)
    , m_pwndVersion(nullptr)
{
}

TGetVersionDlg::~TGetVersionDlg()
{
}


// ---------------------------------------------------------------------------
//  TGetVersionDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TGetVersionDlg::bRun(const  TWindow&            wndOwner
                    , const TString&            strTitle
                    ,       tCIDLib::TCard8&    c8ToFill)
{
    // Save the title and incoming value for later use
    m_c8RetVal = c8ToFill;
    m_strTitle = strTitle;

    const tCIDLib::TCard4 c4Res = c4RunDlg
    (
        wndOwner, facCQCGKit(), kCQCGKit::ridDlg_GetVersion
    );

    if (c4Res == kCQCGKit::ridDlg_GetVer_Accept)
    {
        c8ToFill = m_c8RetVal;
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


// ---------------------------------------------------------------------------
//  TGetVersionDlg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TGetVersionDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRet = TParent::bCreated();

    // Set the title to what we were previously given
    strWndText(m_strTitle);

    // First get typed pointers to some of the widgets
    CastChildWnd(*this, kCQCGKit::ridDlg_GetVer_Accept, m_pwndAccept);
    CastChildWnd(*this, kCQCGKit::ridDlg_GetVer_Cancel, m_pwndCancel);
    CastChildWnd(*this, kCQCGKit::ridDlg_GetVer_Versions, m_pwndVersion);

    // If a non-zero incoming value, then display it
    if (m_c8RetVal)
    {
        TString strFmt;
        facCQCKit().FormatVersionStr(strFmt, m_c8RetVal);
        m_pwndVersion->strWndText(strFmt);
    }

    // Register handlers for our buttons
    m_pwndAccept->pnothRegisterHandler(this, &TGetVersionDlg::eClickHandler);
    m_pwndCancel->pnothRegisterHandler(this, &TGetVersionDlg::eClickHandler);

    return bRet;
}


// ---------------------------------------------------------------------------
//  TGetVersionDlg: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDCtrls::EEvResponses TGetVersionDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCGKit::ridDlg_GetVer_Accept)
    {
        //
        //  Get the text from the entry field and validate it as a standard
        //  version number.
        //
        try
        {
            tCIDLib::TCard4 c4MajVer;
            tCIDLib::TCard4 c4MinVer;
            tCIDLib::TCard4 c4Revision;
            TModule::ParseVersionStr
            (
                m_pwndVersion->strWndText(), c4MajVer, c4MinVer, c4Revision
            );

            // It worked, so store the result and exit the dialog
            m_c8RetVal = (tCIDLib::TCard8(c4MajVer) << 48)
                         | (tCIDLib::TCard8(c4MinVer) << 32)
                         | tCIDLib::TCard8(c4Revision);
            EndDlg(kCQCGKit::ridDlg_GetVer_Accept);
        }

        catch(const TError&)
        {
            // Tell them it failed, and don't dismiss
            TErrBox msgbError(facCQCGKit().strMsg(kGKitMsgs::midStatus_BadVersionStr));
            msgbError.ShowIt(*this);
        }
    }
     else if (wnotEvent.widSource() == kCQCGKit::ridDlg_GetVer_Cancel)
    {
        EndDlg(kCQCGKit::ridDlg_GetVer_Cancel);
    }
    return tCIDCtrls::EEvResponses::Handled;
}

