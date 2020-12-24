//
// FILE NAME: CQCMQTT_ThisFacility.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/21/2018
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
//  This is the implementation for the facility class.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCMQTT_.hpp"



// ---------------------------------------------------------------------------
//   CLASS: TFacCQCMQTT
//  PREFIX: fac
// ---------------------------------------------------------------------------

// -------------------------------------------------------------------
//  Constructors and Destructor
// -------------------------------------------------------------------
TFacCQCMQTT::TFacCQCMQTT() :

    TFacility
    (
        L"CQCMQTT"
        , tCIDLib::EModTypes::SharedLib
        , kCQCKit::c4MajVersion
        , kCQCKit::c4MinVersion
        , kCQCKit::c4Revision
        , tCIDLib::EModFlags::HasMsgFile
    )
    , m_bVerbose(kCIDLib::False)
{
}

TFacCQCMQTT::~TFacCQCMQTT()
{
}
