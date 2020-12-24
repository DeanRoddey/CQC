//
// FILE NAME: CQCVoice_BTLightingNodes.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/13/2017
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
//  This file implements a set of nodes that handle lighting related commands.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCVoice.hpp"
#include    "CQCVoice_BTLightingNodes.hpp"



// ---------------------------------------------------------------------------
//   CLASS: TBTCmdBringUpLightsNode
//  PREFIX: btnode
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TBTCmdBringUpLightsNode: Constructors and Destructor
// ---------------------------------------------------------------------------
TBTCmdBringUpLightsNode::TBTCmdBringUpLightsNode(const TString& strPath, const TString& strName) :

    TBTCQCVoiceNode(strPath, strName, kCQCVoice::strCQCVoiceFact_CmdBringUpLights, 0, 0)
{
}

TBTCmdBringUpLightsNode::~TBTCmdBringUpLightsNode()
{
}


// ---------------------------------------------------------------------------
//  TBTCmdBringUpLightsNode: Public, virtual methods
// ---------------------------------------------------------------------------
tCIDAI::ENodeStates TBTCmdBringUpLightsNode::eRun(TAIBehaviorTree& btreeOwner)
{
    TAIBTCtx& ctxTree = btreeOwner.ctxToUse();

    //
    //  Make sure that the target (light name) is at least med conf. If not, then we
    //  need to tell the user we couldn't understand it.
    //
    if (!facCQCVoice.bIsMedConfidence(btreeOwner, kCQCVoice::strSemVal_Target))
    {
        facCQCVoice.SpeakCantUnderstand(kCQCVoice::strVal_Light);
        return tCIDAI::ENodeStates::Failure;
    }

    // OK, get it out
    const TString& strLightName = ctxTree.strFindKNumListVal(kCQCVoice::strSemVal_Target);

    // The facility class provides a helper for this
    if (!facCQCVoice.bSetLightState(strLightName, kCIDLib::True))
        return tCIDAI::ENodeStates::Failure;

    return tCIDAI::ENodeStates::Success;
}


// ---------------------------------------------------------------------------
//   CLASS: TBTCmdKillLightsNode
//  PREFIX: btnode
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TBTCmdKillLightsNode: Constructors and Destructor
// ---------------------------------------------------------------------------
TBTCmdKillLightsNode::TBTCmdKillLightsNode(const TString& strPath, const TString& strName) :

    TBTCQCVoiceNode(strPath, strName, kCQCVoice::strCQCVoiceFact_CmdKillLights, 0, 0)
{
}

TBTCmdKillLightsNode::~TBTCmdKillLightsNode()
{
}


// ---------------------------------------------------------------------------
//  TBTCmdKillLightsNode: Public, virtual methods
// ---------------------------------------------------------------------------
tCIDAI::ENodeStates TBTCmdKillLightsNode::eRun(TAIBehaviorTree& btreeOwner)
{
    TAIBTCtx& ctxTree = btreeOwner.ctxToUse();

    //
    //  Make sure that the target (light name) is at least med conf. If not, then we
    //  need to tell the user we couldn't understand it.
    //
    if (!facCQCVoice.bIsMedConfidence(btreeOwner, kCQCVoice::strSemVal_Target))
    {
        facCQCVoice.SpeakCantUnderstand(kCQCVoice::strVal_Light);
        return tCIDAI::ENodeStates::Failure;
    }

    // OK, get it out
    const TString& strLightName = ctxTree.strFindKNumListVal(kCQCVoice::strSemVal_Target);

    // The facility class provides a helper for this
    if (!facCQCVoice.bSetLightState(strLightName, kCIDLib::False))
        return tCIDAI::ENodeStates::Failure;

    return tCIDAI::ENodeStates::Success;
}




// ---------------------------------------------------------------------------
//   CLASS: TBTCmdSetLightSwitchNode
//  PREFIX: btnode
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TBTCmdSetLightSwitchNode: Constructors and Destructor
// ---------------------------------------------------------------------------
TBTCmdSetLightSwitchNode::TBTCmdSetLightSwitchNode(const TString& strPath, const TString& strName) :

    TBTCQCVoiceNode(strPath, strName, kCQCVoice::strCQCVoiceFact_CmdSetLightSwitch, 0, 0)
{
}

TBTCmdSetLightSwitchNode::~TBTCmdSetLightSwitchNode()
{
}


// ---------------------------------------------------------------------------
//  TBTCmdSetLightSwitchNode: Public, virtual methods
// ---------------------------------------------------------------------------
tCIDAI::ENodeStates TBTCmdSetLightSwitchNode::eRun(TAIBehaviorTree& btreeOwner)
{
    TAIBTCtx& ctxTree = btreeOwner.ctxToUse();

    //
    //  Make sure that the target (light name) is at least med conf. If not, then we
    //  need to tell the user we couldn't understand it.
    //
    if (!facCQCVoice.bIsMedConfidence(btreeOwner, kCQCVoice::strSemVal_Target))
    {
        facCQCVoice.SpeakCantUnderstand(kCQCVoice::strVal_Light);
        return tCIDAI::ENodeStates::Failure;
    }

    // OK, get it out
    const TString& strLightName = ctxTree.strFindKNumListVal(kCQCVoice::strSemVal_Target);

    // If the state (off/on) isn't at least med conf, ask them to clarify
    tCIDLib::TBoolean bState;
    if (facCQCVoice.bIsMedConfidence(btreeOwner, kCQCVoice::strSemVal_State))
    {
        // It's ok, so take it from the original input
        const TString& strState = ctxTree.strFindKNumListVal(kCQCVoice::strSemVal_State);
        bState = strState.bCompareI(kCQCVoice::strVal_OnState);
    }
     else
    {
        facCQCVoice.SpeakReply(kCQCVoiceMsgs::midQ_OffOrOn);

        // We have a helper for this common clarification
        tCQCVoice::EWaitRepModes eRep = facCQCVoice.eClarifyOffOn(bState);
        if (eRep != tCQCVoice::EWaitRepModes::Success)
            return tCIDAI::ENodeStates::Failure;
    }

    // The facility class provides a helper for this
    if (!facCQCVoice.bSetLightState(strLightName, bState))
        return tCIDAI::ENodeStates::Failure;

    return tCIDAI::ENodeStates::Success;
}




// ---------------------------------------------------------------------------
//   CLASS: TBTCmdSetLightLevelNode
//  PREFIX: btnode
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TBTCmdSetLightLevelNode: Constructors and Destructor
// ---------------------------------------------------------------------------
TBTCmdSetLightLevelNode::TBTCmdSetLightLevelNode(const TString& strPath, const TString& strName) :

    TBTCQCVoiceNode(strPath, strName, kCQCVoice::strCQCVoiceFact_CmdSetLightLevel, 0, 0)
{
}

TBTCmdSetLightLevelNode::~TBTCmdSetLightLevelNode()
{
}


// ---------------------------------------------------------------------------
//  TBTCmdSetLightLevelNode: Public, virtual methods
// ---------------------------------------------------------------------------
tCIDAI::ENodeStates TBTCmdSetLightLevelNode::eRun(TAIBehaviorTree& btreeOwner)
{
    //
    //  Make sure that the light name isn't at least med conf. If not, then we need to
    //  tell the user we couldn't understand it.
    //
    if (!facCQCVoice.bIsMedConfidence(btreeOwner, kCQCVoice::strSemVal_Target))
    {
        facCQCVoice.SpeakCantUnderstand(kCQCVoice::strVal_Light);
        return tCIDAI::ENodeStates::Failure;
    }

    // OK, get it out
    const TString& strLightName = btreeOwner.ctxToUse().strFindKNumListVal
    (
        kCQCVoice::strSemVal_Target
    );

    // If the percent rule isn't at least medium conf, ask them to clarify
    tCIDLib::TCard4 c4Level = 0;
    TString strLevel = btreeOwner.ctxToUse().strFindKNumListVal(kCQCVoice::strSemVal_State);
    if (!facCQCVoice.bIsMedConfidence(btreeOwner, kCQCVoice::strSemVal_State))
    {
        const tCQCVoice::EWaitRepModes eRep = facCQCVoice.eClarifyPercentage(strLevel);
        if (eRep != tCQCVoice::EWaitRepModes::Success)
            return tCIDAI::ENodeStates::Failure;
    }

    // Make sure it's a percentage
    if (!strLevel.bToCard4(c4Level, tCIDLib::ERadices::Dec) || (c4Level > 100))
    {
        facCQCVoice.SpeakReply
        (
            L"The received light level could not be converted to a percent."
        );
        return tCIDAI::ENodeStates::Failure;
    }


    // The facility class provides a helper for this
    if (!facCQCVoice.bSetLightLevel(strLightName, c4Level))
        return tCIDAI::ENodeStates::Failure;

    return tCIDAI::ENodeStates::Success;
}



// ---------------------------------------------------------------------------
//   CLASS: TBTQuLightSwitchNode
//  PREFIX: btnode
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TBTQuLightSwitchNode: Constructors and Destructor
// ---------------------------------------------------------------------------
TBTQuLightSwitchNode::TBTQuLightSwitchNode(const TString& strPath, const TString& strName) :

    TBTCQCVoiceNode(strPath, strName, kCQCVoice::strCQCVoiceFact_QueryLightSwitch, 0, 0)
{
}

TBTQuLightSwitchNode::~TBTQuLightSwitchNode()
{
}


// ---------------------------------------------------------------------------
//  TBTQuLightSwitchNode: Public, virtual methods
// ---------------------------------------------------------------------------
tCIDAI::ENodeStates TBTQuLightSwitchNode::eRun(TAIBehaviorTree& btreeOwner)
{
    TAIBTCtx& ctxTree = btreeOwner.ctxToUse();

    //
    //  Make sure that the light name is at least med conf. If not, then we need to
    //  tell the user we couldn't understand it.
    //
    if (!facCQCVoice.bIsMedConfidence(btreeOwner, kCQCVoice::strSemVal_Target))
    {
        facCQCVoice.SpeakCantUnderstand(kCQCVoice::strVal_Light);
        return tCIDAI::ENodeStates::Failure;
    }

    // OK, get it out
    const TString& strLightName = ctxTree.strFindKNumListVal(kCQCVoice::strSemVal_Target);

    // There's a helper to get the light state
    const tCQCVoice::ELightReads eRead = facCQCVoice.eQueryLightState(strLightName);

    // If the light wasn't found just tell the user and return failure
    if (eRead == tCQCVoice::ELightReads::NotFound)
    {
        facCQCVoice.SpeakReply(kCQCVoiceMsgs::midErr_LightNotFound);
        return tCIDAI::ENodeStates::Failure;
    }

    // If the read failed, then tell the user and return failure
    if (eRead == tCQCVoice::ELightReads::Failed)
    {
        facCQCVoice.SpeakReply(kCQCVoiceMsgs::midErr_FieldReadErr, kCQCVoice::strVal_Light);
        return tCIDAI::ENodeStates::Failure;
    }

    //
    //  We got it, so tell the user the state, and ask if they'd like to change it to
    //  the other state.
    //
    const tCIDLib::TBoolean bState(eRead == tCQCVoice::ELightReads::On);
    facCQCVoice.SpeakReply
    (
        kCQCVoiceMsgs::midSpeak_DevState
        , kCQCVoice::strVal_Light
        , facCQCKit().strBoolOffOn(bState)
    );

    // It worked
    return tCIDAI::ENodeStates::Success;
}



// ---------------------------------------------------------------------------
//   CLASS: TBTQuLightSwitchStateNode
//  PREFIX: btnode
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TBTQuLightSwitchStateNode: Constructors and Destructor
// ---------------------------------------------------------------------------
TBTQuLightSwitchStateNode::TBTQuLightSwitchStateNode(const TString& strPath, const TString& strName) :

    TBTCQCVoiceNode(strPath, strName, kCQCVoice::strCQCVoiceFact_QueryLightSwitchState, 0, 0)
{
}

TBTQuLightSwitchStateNode::~TBTQuLightSwitchStateNode()
{
}


// ---------------------------------------------------------------------------
//  TBTQuLightSwitchStateNode: Public, virtual methods
// ---------------------------------------------------------------------------
tCIDAI::ENodeStates TBTQuLightSwitchStateNode::eRun(TAIBehaviorTree& btreeOwner)
{
    TAIBTCtx& ctxTree = btreeOwner.ctxToUse();

    //
    //  Make sure that the light name is at least med conf. If not, then we need to
    //  tell the user we couldn't understand it.
    //
    if (!facCQCVoice.bIsMedConfidence(btreeOwner, kCQCVoice::strSemVal_Target))
    {
        facCQCVoice.SpeakCantUnderstand(kCQCVoice::strVal_Light);
        return tCIDAI::ENodeStates::Failure;
    }

    // OK, get it out
    const TString& strLightName = ctxTree.strFindKNumListVal(kCQCVoice::strSemVal_Target);

    // Get the state they want to check. If not confident, then clarify
    tCIDLib::TBoolean bState;
    if (facCQCVoice.bIsMedConfidence(btreeOwner, kCQCVoice::strSemVal_State))
    {
        // It's ok, so take it from the original input
        const TString& strState = ctxTree.strFindKNumListVal(kCQCVoice::strSemVal_State);
        bState = strState.bCompareI(kCQCVoice::strVal_OnState);
    }
     else
    {
        facCQCVoice.SpeakReply(kCQCVoiceMsgs::midQ_OffOrOn);

        // We have a helper for this common clarification
        tCQCVoice::EWaitRepModes eRep = facCQCVoice.eClarifyOffOn(bState);
        if (eRep != tCQCVoice::EWaitRepModes::Success)
            return tCIDAI::ENodeStates::Failure;
    }

    // There's a helper to get the light state
    const tCQCVoice::ELightReads eRead = facCQCVoice.eQueryLightState(strLightName);

    // If the light wasn't found just tell the user and return failure
    if (eRead == tCQCVoice::ELightReads::NotFound)
    {
        facCQCVoice.SpeakReply(kCQCVoiceMsgs::midErr_LightNotFound);
        return tCIDAI::ENodeStates::Failure;
    }

    // If the read failed, then tell the user and return failure
    if (eRead == tCQCVoice::ELightReads::Failed)
    {
        facCQCVoice.SpeakReply(kCQCVoiceMsgs::midErr_FieldReadErr, kCQCVoice::strVal_Light);
        return tCIDAI::ENodeStates::Failure;
    }

    // Indicate if it was or was not
    facCQCVoice.SpeakReply
    (
        (eRead == tCQCVoice::ELightReads::On) ? kCQCVoiceMsgs::midSpeak_YesItIs
                                             : kCQCVoiceMsgs::midSpeak_NotItIsNot
    );

    // It worked
    return tCIDAI::ENodeStates::Success;
}

