//
// FILE NAME: CQCMEng_ThisFacility.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/24/2015
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
//  This is the implementation file for the facility class.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCMEng_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TFacCQCMEng,TFacility)



// ---------------------------------------------------------------------------
//   CLASS: TFacCQCMEng
//  PREFIX: fac
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TFacCQCMEng: Constructors and Destructor
// ---------------------------------------------------------------------------
TFacCQCMEng::TFacCQCMEng() :

    TFacility
    (
        L"CQCMEng"
        , tCIDLib::EModTypes::SharedLib
        , kCQCKit::c4MajVersion
        , kCQCKit::c4MinVersion
        , kCQCKit::c4Revision
        , tCIDLib::EModFlags::HasMsgFile
    )
{
}

TFacCQCMEng::~TFacCQCMEng()
{
}


// ---------------------------------------------------------------------------
//  TFacCQCMEng: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Client apps have to call this to get our CML runtime classes registered
//  with the macro engine facility, so that these classes are available to
//  any macros they might load. We just allocate one of our standard runtime
//  class loader objects and install it on the macro engine.
//
//  Unfortunately there is no way to have this fault in, so containing apps
//  must call this during init, before they start up other threads.
//
tCIDLib::TVoid TFacCQCMEng::InitCMLRuntime(const TCQCSecToken& sectUser)
{
    facCIDMacroEng().AddClassLoader(new TCQCRTClassLoader(sectUser));
}


