//
// FILE NAME: CQCIGKit_AboutDlg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/26/2016
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
//  This file implements the about dialog.
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
#include    "CQCIGKit_AboutDlg_.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TAboutDlg,TDlgBox)



// ---------------------------------------------------------------------------
//   CLASS: TAboutDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TAboutDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TAboutDlg::TAboutDlg()
{
}

TAboutDlg::~TAboutDlg()
{
}


// ---------------------------------------------------------------------------
//  TAboutDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TAboutDlg::Run(const TWindow& wndOwner)
{
    c4RunDlg(wndOwner, facCQCIGKit(), kCQCIGKit::ridDlg_About);
}


// ---------------------------------------------------------------------------
//  TAboutDlg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TAboutDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRet = TParent::bCreated();

    // Load up our logo image
    TBitmap bmpLogo = facCQCGKit().bmpLoadLogo(L"CQS_Small", kCIDLib::False);
    pwndChildAs<TStaticImg>(kCQCIGKit::ridDlg_About_Logo)->SetBitmap(bmpLogo);

    // Set the version
    TStaticText* pwndVer = pwndChildAs<TStaticText>(kCQCIGKit::ridDlg_About_Version);
    TString strVer = pwndVer->strWndText();
    strVer.eReplaceToken(kCQCKit::pszVersion, L'1');
    pwndVer->strWndText(strVer);

    //
    //  Change the color of the copyright one. And we need to get the text out and
    //  update it with the current year for the copyright.
    //
    TStaticMultiText* pwndTitle2 = nullptr;
    CastChildWnd(*this, kCQCIGKit::ridDlg_About_Title2, pwndTitle2);
    pwndTitle2->SetTextColor(TRGBClr(0x1F, 0x21, 0x50));
    TString strCopyright(pwndTitle2->strWndText(), 8UL);

    TTime tmNow(tCIDLib::ESpecialTimes::CurrentDate);
    tCIDLib::TCard4     c4Year;
    tCIDLib::EMonths    eMonth;
    tCIDLib::TCard4     c4Day;
    tmNow.eAsDateInfo(c4Year, eMonth, c4Day);
    strCopyright.eReplaceToken(TCardinal(c4Year), L'1');
    pwndTitle2->strWndText(strCopyright);

    TPushButton* pwndOK = pwndChildAs<TPushButton>(kCQCIGKit::ridDlg_About_OK);
    pwndOK->pnothRegisterHandler(this, &TAboutDlg::eClickHandler);

    return bRet;
}


// ---------------------------------------------------------------------------
//  TAboutDlg: Private, non-virtual methods
// ---------------------------------------------------------------------------

tCIDCtrls::EEvResponses TAboutDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    EndDlg(kCQCIGKit::ridDlg_About_OK);
    return tCIDCtrls::EEvResponses::Handled;
}
