//
// FILE NAME: CQCVoice_BTSecurityNodes.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/21/2017
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
//  This file implements a set of nodes that handle security related commands and queries.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCVoice.hpp"
#include    "CQCVoice_BTSecurityNodes.hpp"



// ---------------------------------------------------------------------------
//   CLASS: TBTCmdArmSecSystemNode
//  PREFIX: btnode
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TBTCmdArmSecSystemNode: Constructors and Destructor
// ---------------------------------------------------------------------------
TBTCmdArmSecSystemNode::TBTCmdArmSecSystemNode(const TString& strPath, const TString& strName) :

    TBTCQCVoiceNode(strPath, strName, kCQCVoice::strCQCVoiceFact_CmdArmSecSystem, 0, 0)
{
}

TBTCmdArmSecSystemNode::~TBTCmdArmSecSystemNode()
{
}


// ---------------------------------------------------------------------------
//  TBTCmdArmSecSystemNode: Public, virtual methods
// ---------------------------------------------------------------------------
tCIDAI::ENodeStates TBTCmdArmSecSystemNode::eRun(TAIBehaviorTree& btreeOwner)
{
    TAIBTCtx& ctxTree = btreeOwner.ctxToUse();

    // If no arming modes or arming code info, then we can't do anything
    if (!facCQCVoice.bCapFlag(tCQCVoice::ECapFlags::SecArmModes))
    {
        facCQCVoice.SpeakReply(kCQCVoiceMsgs::midErr_NoSecArmModes, facCQCVoice.strRoom());
        return tCIDAI::ENodeStates::Failure;
    }

    if (!facCQCVoice.bCapFlag(tCQCVoice::ECapFlags::SecArmingCode))
    {
        facCQCVoice.SpeakReply(kCQCVoiceMsgs::midErr_NoSecArmCode, facCQCVoice.strRoom());
        return tCIDAI::ENodeStates::Failure;
    }

    // Get the mode, which is the state semantic value
    TString strMode = ctxTree.strFindKNumListVal(kCQCVoice::strSemVal_State);

    // <TBD> if not high confidence, verify it
    if (!facCQCVoice.bIsHighConfidence(btreeOwner, kCQCVoice::strSemVal_State))
    {
        facCQCVoice.SpeakReply(kCQCVoiceMsgs::midQ_ArmModeConfirm);

        // We have to get an arming mode clarification or cancel
        tCIDLib::TStrList colWaitFor(1);
        colWaitFor.objAdd(kCQCVoice::strAction_ClarifyArmMode);

        // Enable this rule temporarily here
        TCQCVoiceRuleJan janRule(kCQCVoice::strAction_ClarifyArmMode, kCIDLib::True);

        tCIDLib::TCard4 c4At;
        TCIDSpeechRecoEv sprecevClarify;
        const tCQCVoice::EWaitRepModes eRep = facCQCVoice.eWaitForAction
        (
            colWaitFor
            , L"arming mode clarification"
            , sprecevClarify
            , c4At
        );

        // If not successful, a cancel or timeout, we just give up
        if (eRep != tCQCVoice::EWaitRepModes::Success)
            return tCIDAI::ENodeStates::Failure;

        strMode = sprecevClarify.strFindSemVal(kCQCVoice::strSemVal_State);
    }

    try
    {
        // Get our security info, since we need the area name and code
        const TSCSecInfo& scliOurs = facCQCVoice.scliSecurity();

        // Build up the command and write it
        TString strCmd(64UL);
        strCmd = L"ArmArea : ";
        strCmd.Append(scliOurs.m_strAreaName);
        strCmd.Append(kCIDLib::chComma);
        strCmd.Append(strMode);
        strCmd.Append(kCIDLib::chComma);
        strCmd.Append(scliOurs.m_strArmingCode);

        const TString& strMoniker = scliOurs.m_strMoniker;
        tCQCKit::TCQCSrvProxy orbcDrv = facCQCKit().orbcCQCSrvAdminProxy(strMoniker);

        // Tell it to throw if an error
        facCQCVoice.bWriteField(strMoniker, L"InvokeCmd", strCmd, kCIDLib::True);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        facCQCVoice.SpeakReply(kCQCVoiceMsgs::midErr_ArmSecSystem);
        return tCIDAI::ENodeStates::Failure;
    }

    // It worked
    facCQCVoice.SpeakReply(kCQCVoiceMsgs::midSpeak_SecSystemArmed, strMode);

    return tCIDAI::ENodeStates::Success;
}




// ---------------------------------------------------------------------------
//   CLASS: TBTCmdDisarmSecSystemNode
//  PREFIX: btnode
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TBTCmdDisarmSecSystemNode: Constructors and Destructor
// ---------------------------------------------------------------------------
TBTCmdDisarmSecSystemNode::TBTCmdDisarmSecSystemNode(const TString& strPath, const TString& strName) :

    TBTCQCVoiceNode(strPath, strName, kCQCVoice::strCQCVoiceFact_CmdDisarmSecSystem, 0, 0)
{
}

TBTCmdDisarmSecSystemNode::~TBTCmdDisarmSecSystemNode()
{
}


// ---------------------------------------------------------------------------
//  TBTCmdDisarmSecSystemNode: Public, virtual methods
// ---------------------------------------------------------------------------
tCIDAI::ENodeStates TBTCmdDisarmSecSystemNode::eRun(TAIBehaviorTree& btreeOwner)
{
    // Just say that we are not allowed to do this
    facCQCVoice.SpeakReply(kCQCVoiceMsgs::midSpeak_CantDisarmSec);

    return tCIDAI::ENodeStates::Success;
}




// ---------------------------------------------------------------------------
//   CLASS: TBTQuSecZoneStateNode
//  PREFIX: btnode
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TBTQuSecZoneStateNode: Constructors and Destructor
// ---------------------------------------------------------------------------
TBTQuSecZoneStateNode::TBTQuSecZoneStateNode(const  TString& strPath
                                            , const TString& strName) :

    TBTCQCVoiceNode(strPath, strName, kCQCVoice::strCQCVoiceFact_QuerySecZoneState, 0, 0)
{
}

TBTQuSecZoneStateNode::~TBTQuSecZoneStateNode()
{
}


// ---------------------------------------------------------------------------
//  TBTQuSecZoneStateNode: Public, virtual methods
// ---------------------------------------------------------------------------
tCIDAI::ENodeStates TBTQuSecZoneStateNode::eRun(TAIBehaviorTree& btreeOwner)
{
    TAIBTCtx& ctxTree = btreeOwner.ctxToUse();

    // Make sure zones are available
    if (!facCQCVoice.bCapFlag(tCQCVoice::ECapFlags::SecZones))
    {
        facCQCVoice.SpeakReply(kCQCVoiceMsgs::midErr_NoSecZones, facCQCVoice.strRoom());
        return tCIDAI::ENodeStates::Failure;
    }

    // Get the zone to check, clarify if not at least med confidence
    TString strZoneName;
    if (facCQCVoice.bIsMedConfidence(btreeOwner, kCQCVoice::strSemVal_Target))
    {
        strZoneName = ctxTree.strFindKNumListVal(kCQCVoice::strSemVal_Target);
    }
     else
    {
        facCQCVoice.SpeakReply(kCQCVoiceMsgs::midQ_ZoneNameConfirm);

        // We have to get a set point clarification
        tCIDLib::TStrList colWaitFor(1);
        colWaitFor.objAdd(kCQCVoice::strAction_ClarifyZoneName);

        // Enable this rule temporarily here
        TCQCVoiceRuleJan janRule(kCQCVoice::strAction_ClarifyZoneName, kCIDLib::True);

        tCIDLib::TCard4 c4At;
        TCIDSpeechRecoEv sprecevClarify;
        const tCQCVoice::EWaitRepModes eRep = facCQCVoice.eWaitForAction
        (
            colWaitFor, L"Zone name clarification", sprecevClarify, c4At
        );

        // If not successful a cancel or timeout, we just give up
        if (eRep != tCQCVoice::EWaitRepModes::Success)
            return tCIDAI::ENodeStates::Failure;

        strZoneName = sprecevClarify.strFindSemVal(kCQCVoice::strSemVal_Target);
    }

    // Get the state to check, at least med conf
    TString strToCheck;
    if (facCQCVoice.bIsMedConfidence(btreeOwner, kCQCVoice::strSemVal_State))
    {
        strToCheck = ctxTree.strFindKNumListVal(kCQCVoice::strSemVal_State);
    }
     else
    {
        // Get the descriptive value to say to the user
        facCQCVoice.SpeakReply
        (
            kCQCVoiceMsgs::midQ_ZoneStateConfirm
            , ctxTree.strFindKNumListVal(kCQCVoice::strSemVal_Info)
        );

        // We have to get a set point clarification
        tCIDLib::TStrList colWaitFor(1);
        colWaitFor.objAdd(kCQCVoice::strAction_ClarifyZoneState);

        // Enable this rule temporarily here
        TCQCVoiceRuleJan janRule(kCQCVoice::strAction_ClarifyZoneState, kCIDLib::True);

        tCIDLib::TCard4 c4At;
        TCIDSpeechRecoEv sprecevClarify;
        const tCQCVoice::EWaitRepModes eRep = facCQCVoice.eWaitForAction
        (
            colWaitFor
            , L"Zone state clarification"
            , sprecevClarify
            , c4At
        );

        // If not successful a cancel or timeout, we just give up
        if (eRep != tCQCVoice::EWaitRepModes::Success)
            return tCIDAI::ENodeStates::Failure;

        strToCheck = sprecevClarify.strFindSemVal(kCQCVoice::strSemVal_State);
    }

    // Get our room's security info
    const TSCSecInfo& scliOurs = facCQCVoice.scliSecurity();

    // Get the current state of the zone
    TString strCurZoneState;
    try
    {
        TString strFld = L"SEC#Zone_";
        strFld.Append(strZoneName);
        strFld.Append(L"_Status");

        const TString& strMoniker = scliOurs.m_strMoniker;
        tCQCKit::TCQCSrvProxy orbcDrv = facCQCKit().orbcCQCSrvAdminProxy(strMoniker);

        tCIDLib::TCard4 c4SerNum = 0;
        orbcDrv->bReadStringByName(c4SerNum, strMoniker, strFld, strCurZoneState);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        facCQCVoice.SpeakReply(kCQCVoiceMsgs::midErr_ArmSecSystem);
        return tCIDAI::ENodeStates::Failure;
    }

    //
    //  It worked, so tell the user the status. Use the alt name of it is available,
    //  else the raw zone name.
    //
    facCQCVoice.SpeakReply
    (
        (strCurZoneState == strToCheck) ? kCQCVoiceMsgs::midSpeak_ZoneStateIs
                                        : kCQCVoiceMsgs::midSpeak_ZoneStateIsNot
        , facCQCVoice.scrmiOurs().strZoneNameToAltName(strZoneName)
        , ctxTree.strFindKNumListVal(kCQCVoice::strSemVal_Info)
    );

    return tCIDAI::ENodeStates::Success;
}
