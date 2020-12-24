//
// FILE NAME: CQCVoice_BTSimpleCmdNodes.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/08/2017
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
//  This file implements a set of nodes that handle simple commands.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCVoice.hpp"
#include    "CQCVoice_BTSimpleCmdNodes.hpp"



// ---------------------------------------------------------------------------
//   CLASS: TBTCmdReloadCfgNode
//  PREFIX: btnode
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TBTCmdReloadCfgNode: Constructors and Destructor
// ---------------------------------------------------------------------------
TBTCmdReloadCfgNode::TBTCmdReloadCfgNode(const TString& strPath, const TString& strName) :

    TBTCQCVoiceNode(strPath, strName, kCQCVoice::strCQCVoiceFact_CmdReloadCfg, 0, 0)
{
}

TBTCmdReloadCfgNode::~TBTCmdReloadCfgNode()
{
}


// ---------------------------------------------------------------------------
//  TBTCmdReloadCfgNode: Public, virtual methods
// ---------------------------------------------------------------------------
tCIDAI::ENodeStates TBTCmdReloadCfgNode::eRun(TAIBehaviorTree&)
{
    // Just set the reload request flag
    facCQCVoice.SetReloadReqFlag();
    return tCIDAI::ENodeStates::Success;
}




// ---------------------------------------------------------------------------
//   CLASS: TBTCmdSetItToNumNode
//  PREFIX: btnode
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TBTCmdSetItToNumNode: Constructors and Destructor
// ---------------------------------------------------------------------------
TBTCmdSetItToNumNode::TBTCmdSetItToNumNode(const TString& strPath, const TString& strName) :

    TBTCQCVoiceNode(strPath, strName, kCQCVoice::strCQCVoiceFact_CmdSetItToNum, 0, 0)
{
}

TBTCmdSetItToNumNode::~TBTCmdSetItToNumNode()
{
}


// ---------------------------------------------------------------------------
//  TBTCmdSetItToNumNode: Public, virtual methods
// ---------------------------------------------------------------------------
tCIDAI::ENodeStates TBTCmdSetItToNumNode::eRun(TAIBehaviorTree& btreeOwner)
{
    TAIBTCtx& ctxTree = btreeOwner.ctxToUse();

    //
    //  Look for the standard context variables, and see what the previous target and
    //  target type are. If they are not there, or are empty, then we just tell the
    //  user we don't seem to have been talking about something that could be turned
    //  off or on.
    //
    TString strTarget;
    TString strTarType;
    if (!ctxTree.bQueryValue(kCQCVoice::strVar_LastTarget, kCQCVoice::strVarVal_LastTarget, strTarget)
    ||  !ctxTree.bQueryValue(kCQCVoice::strVar_LastTarget, kCQCVoice::strVarVal_LastType, strTarType)
    ||  strTarget.bIsEmpty()
    ||  strTarType.bIsEmpty())
    {
        facCQCVoice.SpeakReply(kCQCVoiceMsgs::midErr_NoPrevTarget);
        return tCIDAI::ENodeStates::Failure;
    }

    //
    //  We may get an info value, which is used for the spoken name for these types of things.
    //  If not, set it to target.
    //
    TString strInfo;
    if (!ctxTree.bQueryValue(kCQCVoice::strVar_LastTarget, kCQCVoice::strVarVal_LastInfo, strInfo)
    ||  strInfo.bIsEmpty())
    {
        strInfo = strTarget;
    }

    // See if the type is something we could reasonably deal wtih
    if (!strTarType.bCompareI(kCQCVoice::strType_Dimmer)
    &&  !strTarType.bCompareI(kCQCVoice::strType_HLSetPoint))
    {
        facCQCVoice.LogMsg
        (
            CID_FILE
            , CID_LINE
            , kCQCVoiceErrs::errcBT_UknownLastType
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Unknown
            , strTarType
            , strTarget
        );

        facCQCVoice.SpeakReply(kCQCVoiceMsgs::midErr_UnknownLastTarType, strTarType);
        return tCIDAI::ENodeStates::Failure;
    }

    // If the numeric value isn't high confidence, ask them to clarify
    TString strVal = ctxTree.strFindKNumListVal(kCQCVoice::strSemVal_State);
    if (!facCQCVoice.bIsHighConfidence(btreeOwner, kCQCVoice::strSemVal_State))
    {
        facCQCVoice.SpeakReply(kCQCVoiceMsgs::midQ_ConfirmNumVal2);

        // Do the appropriate clarfication type for the target
        tCQCVoice::EWaitRepModes eRep;
        if (strTarType.bCompareI(kCQCVoice::strType_Dimmer))
            eRep = facCQCVoice.eClarifyPercentage(strVal);
        else
            eRep = facCQCVoice.eClarifySignedNum(strVal);

        if (eRep != tCQCVoice::EWaitRepModes::Success)
            return tCIDAI::ENodeStates::Failure;
    }

    // Convert the value to a number
    tCIDLib::TInt4 i4Val;
    if (!strVal.bToInt4(i4Val, tCIDLib::ERadices::Dec))
    {
        facCQCVoice.SpeakReply(kCQCVoiceMsgs::midErr_ValidNum);
        return tCIDAI::ENodeStates::Failure;
    }

    // OK, let's do it, depending on target type
    if (strTarType.bCompareI(kCQCVoice::strType_Dimmer))
    {
        if ((i4Val < 0) || (i4Val > 100))
        {
            facCQCVoice.SpeakReply(kCQCVoiceMsgs::midErr_ValidPercentage);
            return tCIDAI::ENodeStates::Failure;
        }

        if (!facCQCVoice.bSetLightLevel(strTarget, tCIDLib::TCard4(i4Val)))
            return tCIDAI::ENodeStates::Failure;
    }
     else if (strTarType.bCompareI(kCQCVoice::strType_HLSetPoint))
    {
        // There is a helper to set a high/low setpoing
        if (!facCQCVoice.bSetThermoSP(strTarget, strVal, strInfo))
            return tCIDAI::ENodeStates::Failure;
    }

    return tCIDAI::ENodeStates::Success;
}



// ---------------------------------------------------------------------------
//   CLASS: TBTCmdSetRoomModeNode
//  PREFIX: btnode
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TBTCmdSetRoomModeNode: Constructors and Destructor
// ---------------------------------------------------------------------------
TBTCmdSetRoomModeNode::TBTCmdSetRoomModeNode(const TString& strPath, const TString& strName) :

    TBTCQCVoiceNode(strPath, strName, kCQCVoice::strCQCVoiceFact_CmdSetRoomMode, 0, 0)
{
}

TBTCmdSetRoomModeNode::~TBTCmdSetRoomModeNode()
{
}


// ---------------------------------------------------------------------------
//  TBTCmdSetRoomModeNode: Public, virtual methods
// ---------------------------------------------------------------------------
tCIDAI::ENodeStates TBTCmdSetRoomModeNode::eRun(TAIBehaviorTree& btreeOwner)
{
    TAIBTCtx& ctxTree = btreeOwner.ctxToUse();

    // If there are no room modes, then say so and give up
    if (!facCQCVoice.bCapFlag(tCQCVoice::ECapFlags::RoomModes))
    {
        facCQCVoice.SpeakReply(kCQCVoiceMsgs::midErr_NoRoomModes, facCQCVoice.strRoom());
        return tCIDAI::ENodeStates::Failure;
    }

    //
    //  The target is the unique id. The Info value is the name for speaking. For the
    //  id, we get a copy since we may update it if we need to clarify.
    //
    TString strName = ctxTree.strFindKNumListVal(kCQCVoice::strSemVal_Info);
    TString strUID = ctxTree.strFindKNumListVal(kCQCVoice::strSemVal_Target);

    // If the mode is not at at least high confidence, then get a clarification
    if (!facCQCVoice.bIsHighConfidence(btreeOwner, kCQCVoice::strSemVal_Target))
    {
        facCQCVoice.SpeakReply(kCQCVoiceMsgs::midQ_ZoneNameConfirm);

        // We can get a yes or a room mode clarification
        tCIDLib::TStrList colWaitFor(2);
        colWaitFor.objAdd(kCQCVoice::strAction_Yes);
        colWaitFor.objAdd(kCQCVoice::strAction_ClarifyRoomMode);

        // Enable these rules temporarily here
        TCQCVoiceRuleJan janRule1(kCQCVoice::strAction_ClarifyRoomMode, kCIDLib::True);
        TCQCVoiceRuleJan janRule2(kCQCVoice::strAction_Yes, kCIDLib::True);

        tCIDLib::TCard4 c4At;
        TCIDSpeechRecoEv sprecevClarify;
        const tCQCVoice::EWaitRepModes eRep = facCQCVoice.eWaitForAction
        (
            colWaitFor
            , L"room mode clarification"
            , sprecevClarify
            , c4At
        );

        // If not successful a cancel or timeout, we just give up
        if (eRep != tCQCVoice::EWaitRepModes::Success)
            return tCIDAI::ENodeStates::Failure;

        // If a clarification, take the new values
        if (c4At == 1)
        {
            strName = sprecevClarify.strFindSemVal(kCQCVoice::strSemVal_Info);
            strUID = sprecevClarify.strFindSemVal(kCQCVoice::strSemVal_Target);
        }
    }

    // Let's get the info for this one
    const TCQCSysCfgRmInfo& scrmiRm = facCQCVoice.scrmiOurs();
    const tCIDLib::TCard4 c4ModeInd = scrmiRm.c4RoomModeById(strUID);
    if (c4ModeInd == kCIDLib::c4MaxCard)
    {
        facCQCVoice.SpeakReply(kCQCVoiceMsgs::midErr_ModeNotFound, strName);
        return tCIDAI::ENodeStates::Failure;
    }

    // We have it, so get the action info
    const TSCGlobActInfo& scliMode = scrmiRm.scliRoomModeAt(c4ModeInd);

    if (!facCQCVoice.bRunGlobalAct(scliMode))
    {
        facCQCVoice.SpeakReply(kCQCVoiceMsgs::midErr_SetRoomMode, strName);
        return tCIDAI::ENodeStates::Failure;
    }

    facCQCVoice.SpeakReply(kCQCVoiceMsgs::midSpeak_RoomModeSet, strName);
    return tCIDAI::ENodeStates::Success;
}



// ---------------------------------------------------------------------------
//   CLASS: TBTCmdTurnItOffOnNode
//  PREFIX: btnode
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TBTCmdTurnItOffOnNode: Constructors and Destructor
// ---------------------------------------------------------------------------
TBTCmdTurnItOffOnNode::TBTCmdTurnItOffOnNode(const TString& strPath, const TString& strName) :

    TBTCQCVoiceNode(strPath, strName, kCQCVoice::strCQCVoiceFact_CmdReloadCfg, 0, 0)
{
}

TBTCmdTurnItOffOnNode::~TBTCmdTurnItOffOnNode()
{
}


// ---------------------------------------------------------------------------
//  TBTCmdTurnItOffOnNode: Public, virtual methods
// ---------------------------------------------------------------------------
tCIDAI::ENodeStates TBTCmdTurnItOffOnNode::eRun(TAIBehaviorTree& btreeOwner)
{
    TAIBTCtx& ctxTree = btreeOwner.ctxToUse();

    //
    //  Look for the standard context variables, and see what the previous target and
    //  target type are. If they are not there, or are empty, then we just tell the
    //  user we don't seem to have been talking about something that could be turned
    //  off or on.
    //
    TString strTarget;
    TString strTarType;
    if (!ctxTree.bQueryValue(kCQCVoice::strVar_LastTarget, kCQCVoice::strVarVal_LastTarget, strTarget)
    ||  !ctxTree.bQueryValue(kCQCVoice::strVar_LastTarget, kCQCVoice::strVarVal_LastType, strTarType)
    ||  strTarget.bIsEmpty()
    ||  strTarType.bIsEmpty())
    {
        facCQCVoice.SpeakReply(kCQCVoiceMsgs::midErr_NoPrevTarget);
        return tCIDAI::ENodeStates::Failure;
    }

    // See if the type is something we could reasonably turn off or on
    if (!strTarType.bCompareI(kCQCVoice::strType_LightSwitch))
    {
        facCQCVoice.LogMsg
        (
            CID_FILE
            , CID_LINE
            , kCQCVoiceErrs::errcBT_UknownLastType
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Unknown
            , strTarType
            , strTarget
        );

        facCQCVoice.SpeakReply(kCQCVoiceMsgs::midErr_UnknownLastTarType, strTarType);
        return tCIDAI::ENodeStates::Failure;
    }

    //
    //  We may get an info value, which is used for the spoken name for these types of things.
    //  If not, set it to target.
    //
    TString strInfo;
    if (!ctxTree.bQueryValue(kCQCVoice::strVar_LastTarget, kCQCVoice::strVarVal_LastInfo, strInfo)
    ||  strInfo.bIsEmpty())
    {
        strInfo = strTarget;
    }

    // If the state rule isn't high confidence, ask them to clarify
    tCIDLib::TBoolean bState;
    if (facCQCVoice.bIsHighConfidence(btreeOwner, kCQCVoice::strSemVal_State))
    {
        // It's ok, so take it from the original input
        bState =
        (
            btreeOwner.ctxToUse().strFindKNumListVal(kCQCVoice::strSemVal_State) == kCQCVoice::strVal_OnState
        );
    }
     else
    {
        facCQCVoice.SpeakReply(kCQCVoiceMsgs::midQ_OffOrOn);

        // We have a helper for this common clarification
        tCQCVoice::EWaitRepModes eRep = facCQCVoice.eClarifyOffOn(bState);
        if (eRep != tCQCVoice::EWaitRepModes::Success)
            return tCIDAI::ENodeStates::Failure;
    }

    // OK, let's do it, depending on target type
    if (strTarType.bCompareI(kCQCVoice::strType_LightSwitch))
    {
        if (!facCQCVoice.bSetLightState(strTarget, bState))
            return tCIDAI::ENodeStates::Failure;
    }

    return tCIDAI::ENodeStates::Success;
}


