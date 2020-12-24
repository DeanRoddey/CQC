//
// FILE NAME: CQCTrayMon_TabWnd.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/04/2012
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
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


// ---------------------------------------------------------------------------
//  Facility specific includes
// ---------------------------------------------------------------------------
#include    "CQCTrayMon.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCTrayMonTab, TTabWindow)



// ---------------------------------------------------------------------------
//  CLASS: TCQCTrayMonTab
// PREFIX: wnd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCTrayMonTab: Destructor
// ---------------------------------------------------------------------------
TCQCTrayMonTab::~TCQCTrayMonTab()
{
}


// ---------------------------------------------------------------------------
//  TCQCTrayMonTab: Hidden Constructors
// ---------------------------------------------------------------------------
TCQCTrayMonTab::TCQCTrayMonTab(const TString& strName) :

    TTabWindow(strName, TString::strEmpty(), kCIDLib::False, kCIDLib::True)
{
}

