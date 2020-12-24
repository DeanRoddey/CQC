//
// FILE NAME: CQCVoice_BTSimpleQNodes.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/07/2017
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
//  This file implements a set of nodes that handle simple queries.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCVoice.hpp"
#include    "CQCVoice_BTSimpleQNodes.hpp"



// ---------------------------------------------------------------------------
//   CLASS: TBTQuCurrentTimeNode
//  PREFIX: btnode
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TBTQuCurrentTimeNode: Constructors and Destructor
// ---------------------------------------------------------------------------
TBTQuCurrentTimeNode::TBTQuCurrentTimeNode(const TString& strPath, const TString& strName) :

    TBTCQCVoiceNode(strPath, strName, kCQCVoice::strCQCVoiceFact_QueryCurrentTime, 0, 0)
{
}

TBTQuCurrentTimeNode::~TBTQuCurrentTimeNode()
{
}


// ---------------------------------------------------------------------------
//  TBTQuCurrentTimeNode: Public, virtual methods
// ---------------------------------------------------------------------------
tCIDAI::ENodeStates TBTQuCurrentTimeNode::eRun(TAIBehaviorTree&)
{
    //
    //  Get the current time and set up the hour and minute values. The hours differ
    //  depending on local time format (24 or 12.)
    //
    TTime tmCur(tCIDLib::ESpecialTimes::CurrentTime);

    // Format it out using a token string
    TString strFmt;
    tmCur.FormatToStr(strFmt, TLocale::b24Hour() ? TTime::str24HHMM() : TTime::strHHMMap());

    // Indicate this has markup
    facCQCVoice.SpeakReply(kCQCVoiceMsgs::midSpeak_CurrentTime, strFmt);

    // We always just return success
    return tCIDAI::ENodeStates::Success;
}




// ---------------------------------------------------------------------------
//   CLASS: TBTQuCQCVersionNode
//  PREFIX: btnode
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TBTQuCQCVersionNode: Constructors and Destructor
// ---------------------------------------------------------------------------
TBTQuCQCVersionNode::TBTQuCQCVersionNode(const TString& strPath, const TString& strName) :

    TBTCQCVoiceNode(strPath, strName, kCQCVoice::strCQCVoiceFact_QueryCQCVer, 0, 0)
{
}

TBTQuCQCVersionNode::~TBTQuCQCVersionNode()
{
}


// ---------------------------------------------------------------------------
//  TBTQuCQCVersionNode: Public, virtual methods
// ---------------------------------------------------------------------------
tCIDAI::ENodeStates TBTQuCQCVersionNode::eRun(TAIBehaviorTree&)
{
    facCQCVoice.SpeakReply
    (
        kCQCVoiceMsgs::midSpeak_CQCVersion
        , TCardinal(kCQCKit::c4MajVersion)
        , TCardinal(kCQCKit::c4MinVersion)
        , TCardinal(kCQCKit::c4Revision)
    );

    // We always just return success
    return tCIDAI::ENodeStates::Success;
}



// ---------------------------------------------------------------------------
//   CLASS: TBTQuTodaysDateNode
//  PREFIX: btnode
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TBTQuTodaysDateNode: Constructors and Destructor
// ---------------------------------------------------------------------------
TBTQuTodaysDateNode::TBTQuTodaysDateNode(const TString& strPath, const TString& strName) :

    TBTCQCVoiceNode(strPath, strName, kCQCVoice::strCQCVoiceFact_QueryTodaysDate, 0, 0)
{
}

TBTQuTodaysDateNode::~TBTQuTodaysDateNode()
{
}


// ---------------------------------------------------------------------------
//  TBTQuTodaysDateNode: Public, virtual methods
// ---------------------------------------------------------------------------
tCIDAI::ENodeStates TBTQuTodaysDateNode::eRun(TAIBehaviorTree&)
{
    // Get the current time and a loadable string that holds the replacement tokens
    TTime tmCur(tCIDLib::ESpecialTimes::CurrentTime);
    TString strPattern(kCQCVoiceMsgs::midSpeakP_TodaysDate, facCQCVoice);

    // And now format this current time out using the pattern
    TString strMsg;
    tmCur.FormatToStr(strMsg, strPattern);

    // Indicate this has markup
    facCQCVoice.SpeakReply(strMsg, kCIDLib::True);

    // We always just return success
    return tCIDAI::ENodeStates::Success;
}

