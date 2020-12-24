//
// FILE NAME: MediaRepoMgr_Stats.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/23/2006
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
//  This is the header for the MediaRepoMgr_StatsDlg.cpp file, which
//  implements a dialog for showing some repository stats.
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
#include    "MediaRepoMgr_StatsDlg.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TStatsDlg,TDlgBox)



// ---------------------------------------------------------------------------
//   CLASS: TStatsDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TStatsDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TStatsDlg::TStatsDlg() :

    m_pmdbSrc(nullptr)
    , m_pwndClose(nullptr)
{
}

TStatsDlg::~TStatsDlg()
{
}


// ---------------------------------------------------------------------------
//  TStatsDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TStatsDlg::RunDlg(  const   TWindow&    wndOwner
                    , const TMediaDB&   mdbSrc
                    , const TString&    strRepoPath)
{
    // Store the incoming info
    m_pmdbSrc = &mdbSrc;
    m_strRepoPath = strRepoPath;

    // And run the dialog
    c4RunDlg(wndOwner, facMediaRepoMgr, kMediaRepoMgr::ridDlg_Stats);
}


// ---------------------------------------------------------------------------
//  TStatsDlg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TStatsDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRet = TParent::bCreated();

    // Get typed pointers to some of the widgets
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_Stats_Close, m_pwndClose);

    // Get the stats from teh passed source database
    tCIDLib::TCard4 c4MovieCollects;
    tCIDLib::TCard4 c4MovieItems;
    tCIDLib::TCard4 c4MovieImages;
    tCIDLib::TCard4 c4MovieTitles;
    tCIDLib::TCard4 c4MusicCollects;
    tCIDLib::TCard4 c4MusicItems;
    tCIDLib::TCard4 c4MusicImages;
    tCIDLib::TCard4 c4MusicTitles;

    m_pmdbSrc->QueryStats
    (
        c4MovieTitles
        , c4MovieCollects
        , c4MovieItems
        , c4MovieImages
        , c4MusicTitles
        , c4MusicCollects
        , c4MusicItems
        , c4MusicImages
    );

    // Load up the stats
    const TString strMovie(L", Movie=");
    const TString strMusic(L"Music=");
    TString strFmt;

    strFmt = strMusic;
    strFmt.AppendFormatted(c4MusicTitles);
    strFmt.Append(strMovie);
    strFmt.AppendFormatted(c4MovieTitles);
    strChildText(kMediaRepoMgr::ridDlg_Stats_Titles, strFmt);

    strFmt = strMusic;
    strFmt.AppendFormatted(c4MusicCollects);
    strFmt.Append(strMovie);
    strFmt.AppendFormatted(c4MovieCollects);
    strChildText(kMediaRepoMgr::ridDlg_Stats_Collections, strFmt);

    strFmt = strMusic;
    strFmt.AppendFormatted(c4MusicItems);
    strFmt.Append(strMovie);
    strFmt.AppendFormatted(c4MovieItems);
    strChildText(kMediaRepoMgr::ridDlg_Stats_Items, strFmt);

    strFmt = strMusic;
    strFmt.AppendFormatted(c4MusicImages);
    strFmt.Append(strMovie);
    strFmt.AppendFormatted(c4MovieImages);
    strChildText(kMediaRepoMgr::ridDlg_Stats_Images, strFmt);

    strChildText(kMediaRepoMgr::ridDlg_Stats_Path, m_strRepoPath);

    // Install handlers
    m_pwndClose->pnothRegisterHandler(this, &TStatsDlg::eClickHandler);

    return bRet;
}


// ---------------------------------------------------------------------------
//  TStatsDlg: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDCtrls::EEvResponses TStatsDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kMediaRepoMgr::ridDlg_Stats_Close)
        EndDlg(kMediaRepoMgr::ridDlg_Stats_Close);
    return tCIDCtrls::EEvResponses::Handled;
}
