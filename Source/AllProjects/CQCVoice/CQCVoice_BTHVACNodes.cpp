//
// FILE NAME: CQCVoice_BTHVACNodes.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/20/2017
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
//  This file implements a set of nodes that handle HVAC related commands and queries.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCVoice.hpp"
#include    "CQCVoice_BTHVACNodes.hpp"



// ---------------------------------------------------------------------------
//   CLASS: TBTCmdSetThermoHLSPNode
//  PREFIX: btnode
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TBTCmdSetThermoHLSPNode: Constructors and Destructor
// ---------------------------------------------------------------------------
TBTCmdSetThermoHLSPNode::TBTCmdSetThermoHLSPNode(const TString& strPath, const TString& strName) :

    TBTCQCVoiceNode(strPath, strName, kCQCVoice::strCQCVoiceFact_CmdSetThermoHLSP, 0, 0)
{
}

TBTCmdSetThermoHLSPNode::~TBTCmdSetThermoHLSPNode()
{
}


// ---------------------------------------------------------------------------
//  TBTCmdSetThermoHLSPNode: Public, virtual methods
// ---------------------------------------------------------------------------
tCIDAI::ENodeStates TBTCmdSetThermoHLSPNode::eRun(TAIBehaviorTree& btreeOwner)
{
    TAIBTCtx& ctxTree = btreeOwner.ctxToUse();

    // If no HVAC info, then we can't do anything
    if (!facCQCVoice.bCapFlag(tCQCVoice::ECapFlags::HVACData))
    {
        facCQCVoice.SpeakReply(kCQCVoiceMsgs::midErr_NoHVACData, facCQCVoice.strRoom());
        return tCIDAI::ENodeStates::Failure;
    }

    // If the target (high/low) isn't high confidence, clarify it, else take the original
    TString strSetPoint;
    if (facCQCVoice.bIsHighConfidence(btreeOwner, kCQCVoice::strSemVal_Action))
    {
        strSetPoint = ctxTree.strFindKNumListVal(kCQCVoice::strSemVal_Target);
    }
     else
    {
        facCQCVoice.SpeakReply(kCQCVoiceMsgs::midQ_SPConfirm);

        // We have to get a set point clarification or cancel
        tCIDLib::TStrList colWaitFor(1);
        colWaitFor.objAdd(kCQCVoice::strAction_ClarifyHLSP);

        // Enable this rule temporarily here
        TCQCVoiceRuleJan janRule(kCQCVoice::strAction_ClarifyHLSP, kCIDLib::True);

        tCIDLib::TCard4 c4At;
        TCIDSpeechRecoEv sprecevClarify;
        const tCQCVoice::EWaitRepModes eRep = facCQCVoice.eWaitForAction
        (
            colWaitFor
            , L"set point clarification"
            , sprecevClarify
            , c4At
        );

        // If not successful, a cancel or timeout, we just give up
        if (eRep != tCQCVoice::EWaitRepModes::Success)
            return tCIDAI::ENodeStates::Failure;

        strSetPoint = sprecevClarify.strFindSemVal(kCQCVoice::strSemVal_Target);
    }


    // If the value wasn't high confidence, clarify that
    TString strState = ctxTree.strFindKNumListVal(kCQCVoice::strSemVal_State);
    if (!facCQCVoice.bIsHighConfidence(btreeOwner, kCQCVoice::strSemVal_State))
    {
        facCQCVoice.SpeakReply
        (
            kCQCVoiceMsgs::midQ_ConfirmNumVal, strState, kCQCVoice::strVal_Degrees
        );

        // There's a helper to clearify a signed number value
        const tCQCVoice::EWaitRepModes eRep = facCQCVoice.eClarifySignedNum(strState);
        if (eRep != tCQCVoice::EWaitRepModes::Success)
            return tCIDAI::ENodeStates::Failure;
    }

    // Get the actual set point name the user indicated, to pass to the spoken ack
    TString strSPSpoken = ctxTree.strFindKNumListVal(kCQCVoice::strSemVal_Info);

    // There is a helper to do this
    if (!facCQCVoice.bSetThermoSP(strSetPoint, strState, strSPSpoken))
        return tCIDAI::ENodeStates::Failure;

    return tCIDAI::ENodeStates::Success;
}



// ---------------------------------------------------------------------------
//   CLASS: TBTQuThermoHLSPNode
//  PREFIX: btnode
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TBTQuThermoHLSPNode: Constructors and Destructor
// ---------------------------------------------------------------------------
TBTQuThermoHLSPNode::TBTQuThermoHLSPNode(const TString& strPath, const TString& strName) :

    TBTCQCVoiceNode(strPath, strName, kCQCVoice::strCQCVoiceFact_QueryThermoHLSP, 0, 0)
{
}

TBTQuThermoHLSPNode::~TBTQuThermoHLSPNode()
{
}


// ---------------------------------------------------------------------------
//  TBTQuThermoHLSPNode: Public, virtual methods
// ---------------------------------------------------------------------------
tCIDAI::ENodeStates TBTQuThermoHLSPNode::eRun(TAIBehaviorTree& btreeOwner)
{
    TAIBTCtx& ctxTree = btreeOwner.ctxToUse();

    // If no HVAC info, then we can't do anything
    if (!facCQCVoice.bCapFlag(tCQCVoice::ECapFlags::HVACData))
    {
        facCQCVoice.SpeakReply(kCQCVoiceMsgs::midErr_NoHVACData, facCQCVoice.strRoom());
        return tCIDAI::ENodeStates::Failure;
    }

    // If the high/low rule isn't at least medium confidence, clarify it
    TString strSetPoint;
    if (!facCQCVoice.bIsMedConfidence(btreeOwner, kCQCVoice::strSemVal_Target))
    {
        facCQCVoice.SpeakReply(kCQCVoiceMsgs::midQ_SPConfirm);

        // We have to get a set point clarification or cancel
        tCIDLib::TStrList colWaitFor(1);
        colWaitFor.objAdd(kCQCVoice::strAction_ClarifyHLSP);

        // Enable this rule temporarily here
        TCQCVoiceRuleJan janRule(kCQCVoice::strAction_ClarifyHLSP, kCIDLib::True);

        tCIDLib::TCard4 c4At;
        TCIDSpeechRecoEv sprecevClarify;
        const tCQCVoice::EWaitRepModes eRep = facCQCVoice.eWaitForAction
        (
            colWaitFor, L"set point clarification", sprecevClarify, c4At
        );

        // If not successful a cancel or timeout, we just give up
        if (eRep != tCQCVoice::EWaitRepModes::Success)
            return tCIDAI::ENodeStates::Failure;

        strSetPoint = sprecevClarify.strFindSemVal(kCQCVoice::strSemVal_Target);
    }
     else
    {
        // It's good so get the original one
        strSetPoint = ctxTree.strFindKNumListVal(kCQCVoice::strSemVal_Target);
    }

    // Get our HVAC info
    const TSCHVACInfo& scliOurs = facCQCVoice.scliHVAC();

    // Build up the appropriate field
    TString strSPField(64UL);
    strSPField = L"THERM#";
    if (!scliOurs.m_strThermoSub.bIsEmpty())
    {
        strSPField.Append(scliOurs.m_strThermoSub);
        strSPField.Append(kCIDLib::chTilde);
    }
    strSPField.Append(strSetPoint);
    strSPField.Append(L"SetPnt");

    // Let's get the value and speak it to the user
    try
    {
        const TString& strMoniker = scliOurs.m_strMoniker;
        tCQCKit::TCQCSrvProxy orbcDrv = facCQCKit().orbcCQCSrvAdminProxy(strMoniker);

        tCIDLib::TCard4 c4SerNum = 0;
        tCIDLib::TInt4 i4Temp;
        orbcDrv->bReadIntByName(c4SerNum, strMoniker, strSPField, i4Temp);

        // Get the actual set point name the user spoke to use in the reply
        facCQCVoice.SpeakReply
        (
            kCQCVoiceMsgs::midSpeak_HLSetPoint
            , ctxTree.strFindKNumListVal(kCQCVoice::strSemVal_Info)
            , TInteger(i4Temp)
        );
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        facCQCVoice.SpeakReply(kCQCVoiceMsgs::midErr_QuerySP, strSetPoint);
        return tCIDAI::ENodeStates::Failure;
    }

    // It worked
    return tCIDAI::ENodeStates::Success;
}
