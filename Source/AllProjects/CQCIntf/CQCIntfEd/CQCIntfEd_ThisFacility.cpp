//
// FILE NAME: CQCIntfEd_ThisFacility.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/01/2015
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
#include "CQCIntfEd_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TFacCQCIntfEd,TGUIFacility)



// ---------------------------------------------------------------------------
//   CLASS: TFacCQCIntfEd
//  PREFIX: fac
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TFacCQCIntfEd: Constructors and Destructor
// ---------------------------------------------------------------------------
TFacCQCIntfEd::TFacCQCIntfEd() :

    TGUIFacility
    (
        L"CQCIntfEd"
        , tCIDLib::EModTypes::SharedLib
        , kCIDLib::c4MajVersion
        , kCIDLib::c4MinVersion
        , kCIDLib::c4Revision
        , tCIDLib::EModFlags::HasMsgsAndRes
    )
{
    //
    //  We have to put the interface engine to designer mode before we load any
    //  templates. This prevents it from doing some things that should only be
    //  done at viewing time.
    //
    facCQCIntfEng().bDesMode(kCIDLib::True);
}

TFacCQCIntfEd::~TFacCQCIntfEd()
{
}


// ---------------------------------------------------------------------------
//  TFacCQCIntfEd: Public, non-virtual methods
// ---------------------------------------------------------------------------



// ---------------------------------------------------------------------------
//  TFacCQCIntfEd: Private, non-virtual methods
// ---------------------------------------------------------------------------

