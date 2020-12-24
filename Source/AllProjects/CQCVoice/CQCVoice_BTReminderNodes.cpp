//
// FILE NAME: CQCVoice_BTReminderNodes.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/08/2017
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
//  This file implements a set of nodes that handle reminder oriented stuff.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCVoice.hpp"
#include    "CQCVoice_BTReminderNodes.hpp"




// ---------------------------------------------------------------------------
//   CLASS: TBTCmdCancelAllReminders
//  PREFIX: btnode
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TBTCmdCancelAllReminders: Constructors and Destructor
// ---------------------------------------------------------------------------
TBTCmdCancelAllReminders::
TBTCmdCancelAllReminders(const TString& strPath, const TString& strName) :

    TBTCQCVoiceNode(strPath, strName, kCQCVoice::strCQCVoiceFact_CmdCancelAllReminders, 0, 0)
{
}

TBTCmdCancelAllReminders::~TBTCmdCancelAllReminders()
{
}


// ---------------------------------------------------------------------------
//  TBTCmdCancelAllReminders: Public, virtual methods
// ---------------------------------------------------------------------------
tCIDAI::ENodeStates TBTCmdCancelAllReminders::eRun(TAIBehaviorTree& btreeOwner)
{
    facCQCVoice.CancelAllReminders();
    return tCIDAI::ENodeStates::Success;
}



// ---------------------------------------------------------------------------
//   CLASS: TBTCmdCancelLastReminder
//  PREFIX: btnode
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TBTCmdCancelLastReminder: Constructors and Destructor
// ---------------------------------------------------------------------------
TBTCmdCancelLastReminder::
TBTCmdCancelLastReminder(const TString& strPath, const TString& strName) :

    TBTCQCVoiceNode(strPath, strName, kCQCVoice::strCQCVoiceFact_CmdCancelLastReminder, 0, 0)
{
}

TBTCmdCancelLastReminder::~TBTCmdCancelLastReminder()
{
}


// ---------------------------------------------------------------------------
//  TBTCmdCancelLastReminder: Public, virtual methods
// ---------------------------------------------------------------------------
tCIDAI::ENodeStates TBTCmdCancelLastReminder::eRun(TAIBehaviorTree&)
{
    facCQCVoice.CancelLastReminder();
    return tCIDAI::ENodeStates::Success;
}




// ---------------------------------------------------------------------------
//   CLASS: TBTCmdRemindMe
//  PREFIX: btnode
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TBTCmdRemindMe: Constructors and Destructor
// ---------------------------------------------------------------------------
TBTCmdRemindMe::TBTCmdRemindMe(const TString& strPath, const TString& strName) :

    TBTCQCVoiceNode(strPath, strName, kCQCVoice::strCQCVoiceFact_CmdRemindMe, 0, 0)
{
}

TBTCmdRemindMe::~TBTCmdRemindMe()
{
}


// ---------------------------------------------------------------------------
//  TBTCmdRemindMe: Public, virtual methods
// ---------------------------------------------------------------------------
tCIDAI::ENodeStates TBTCmdRemindMe::eRun(TAIBehaviorTree& btreeOwner)
{
    TAIBTCtx& ctxTree = btreeOwner.ctxToUse();

    // Get the target semantic value, which is where the reminder text is stored
    TString strReminder = ctxTree.strFindKNumListVal(kCQCVoice::strSemVal_Target);

    // Get the minutes out, which is in the state value
    TString strMinutes = ctxTree.strFindKNumListVal(kCQCVoice::strSemVal_State);

    // If the minutes is not at at least medium confidence, then get a clarification
    if (!facCQCVoice.bIsMedConfidence(btreeOwner, kCQCVoice::strSemVal_State))
    {
        // We'll accept a clarify minutes or cancel
        facCQCVoice.SpeakReply(kCQCVoiceMsgs::midQ_MinutesConfirm);

        tCIDLib::TStrList colWaitFor(2);
        colWaitFor.objAdd(kCQCVoice::strAction_CancelOp);
        colWaitFor.objAdd(kCQCVoice::strAction_ClarifyMinutes);

        // Enable the minutes clearify rule temporarily
        TCQCVoiceRuleJan janRule1(kCQCVoice::strAction_ClarifyMinutes, kCIDLib::True);

        tCIDLib::TCard4 c4At;
        TCIDSpeechRecoEv sprecevClarify;
        const tCQCVoice::EWaitRepModes eRep = facCQCVoice.eWaitForAction
        (
            colWaitFor, L"reminder minutes", sprecevClarify, c4At
        );

        // If not successful a cancel or timeout, we just give up
        if (eRep != tCQCVoice::EWaitRepModes::Success)
            return tCIDAI::ENodeStates::Failure;

        strMinutes = sprecevClarify.strFindSemVal(kCQCVoice::strSemVal_State);
    }

    // Make sure we can convert the minutes
    tCIDLib::TCard4 c4Minutes;
    if (!strMinutes.bToCard4(c4Minutes, tCIDLib::ERadices::Dec))
    {
        facCQCVoice.SpeakReply(kCQCVoiceMsgs::midErr_ValidNum);
        return tCIDAI::ENodeStates::Success;
    }

    //
    //  See if there is already a reminder of this type queued up, but only one.
    //  If there are more than one, assume new ones should be added. If only one,
    //  then ask if they want to update it or add a new one.
    //
    tCIDLib::TBoolean bAddNew = kCIDLib::True;
    tCIDLib::TCard4 c4CurCount;
    if (facCQCVoice.bReminderExists(strReminder, c4CurCount) && (c4CurCount == 1))
    {
        facCQCVoice.SpeakReply(kCQCVoiceMsgs::midQ_ReminderAddUpdate);

        tCIDLib::TStrList colWaitFor(2);
        colWaitFor.objAdd(kCQCVoice::strAction_CancelOp);
        colWaitFor.objAdd(kCQCVoice::strAction_ClarifyAddUpdateReminder);

        // Enable the minutes clearify rule temporarily
        TCQCVoiceRuleJan janRule1(kCQCVoice::strAction_ClarifyAddUpdateReminder, kCIDLib::True);

        tCIDLib::TCard4 c4At;
        TCIDSpeechRecoEv sprecevClarify;
        const tCQCVoice::EWaitRepModes eRep = facCQCVoice.eWaitForAction
        (
            colWaitFor, L"reminder add or update", sprecevClarify, c4At
        );

        // If not successful a cancel or timeout, we just give up
        if (eRep != tCQCVoice::EWaitRepModes::Success)
            return tCIDAI::ENodeStates::Failure;

        // If they say to update, do that
        const TString& strAdd = sprecevClarify.strFindSemVal(kCQCVoice::strSemVal_State);
        bAddNew = strAdd == kCQCVoice::strVal_Add;
    }

    //
    //  Let's add a new one or update as indicated. First, replace any instances of
    //  'my' with 'your' so that it sounds more natural.
    //
    tCIDLib::TCard4 c4At = 0;
    strReminder.bReplaceSubStr
    (
        kCQCVoice::strVal_My, kCQCVoice::strVal_Your, c4At, kCIDLib::True, kCIDLib::False
    );
    if (bAddNew)
        facCQCVoice.AddReminder(strReminder, c4Minutes);
    else
        facCQCVoice.UpdateReminderTime(strReminder, c4Minutes);

    return tCIDAI::ENodeStates::Success;
}

