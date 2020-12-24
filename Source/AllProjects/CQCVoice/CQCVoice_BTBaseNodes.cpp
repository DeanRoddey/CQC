//
// FILE NAME: CQCVoice_BTStartNode.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/06/2017
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
//  This file implements some core nodes that are special cases.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCVoice.hpp"


// ---------------------------------------------------------------------------
//   CLASS: TBTCQCVoiceNode
//  PREFIX: btnode
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TBTCQCVoiceNode: Constructors and sestructor
// ---------------------------------------------------------------------------
TBTCQCVoiceNode::~TBTCQCVoiceNode()
{
}


// ---------------------------------------------------------------------------
//  TBTCQCVoiceNode: Hidden constructors
// ---------------------------------------------------------------------------
TBTCQCVoiceNode::TBTCQCVoiceNode(const  TString&        strPath
                                , const TString&        strName
                                , const TString&        strNodeType
                                , const tCIDLib::TCard4 c4MinChildCnt
                                , const tCIDLib::TCard4 c4MaxChildCnt) :


    TAIBTNode(strPath, strName, strNodeType, c4MinChildCnt, c4MaxChildCnt)
{
}





// ---------------------------------------------------------------------------
//   CLASS: TBTStartNode
//  PREFIX: btnode
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TBTStartNode: Constructors and Destructor
// ---------------------------------------------------------------------------
TBTStartNode::TBTStartNode(const TString& strPath, const TString& strName) :

    TAIBTNode(strPath, strName, kCQCVoice::strCQCVoiceFact_Start, 1, 128)
{
}

TBTStartNode::~TBTStartNode()
{
}


// ---------------------------------------------------------------------------
//  TBTStartNode: Public, virtual methods
// ---------------------------------------------------------------------------
tCIDAI::ENodeStates TBTStartNode::eRun(TAIBehaviorTree& btreeOwner)
{
    // We enter the conversation loop until we time out or are asked to stop.
    tCIDLib::TCard4     c4ChildIndex;
    tCIDLib::TKVPList   colVars;
    tCIDAI::ENodeStates eNodeState;
    TCIDSpeechRecoEv    sprecevCur;
    TString             strAction;
    TString             strSemVal;
    TString             strInfo;
    TString             strTarget;
    TString             strTarType;
    TAIBTCtx&           ctxTree = btreeOwner.ctxToUse();

    //
    //  First time in, get the action from the incoming tree context. After that we'll
    //  get it back from the event getter. It may be empty
    //
    strAction = ctxTree.strFindKNumListVal(kCQCVoice::strSemVal_Action);


    //
    //  If we got here from the wakeup keyword, then we need to ask what they want. Doing
    //  that up front makes the code below simpler. We won't do an initial action in
    //  this case, we'll just fall through and start waiting for a command, because the
    //  action won't match any node.
    //
    //  If they didn't do the wakeup keyword, i.e. they did a command, we'll just do a
    //  one shot and get out.
    //
    const tCIDLib::TBoolean bConvoMode(strAction.bCompareI(kCQCVoice::strAction_WakeupKeyWord));
    if (bConvoMode)
        facCQCVoice.SpeakWhatCanIDo();

    TThread* pthrCaller = TThread::pthrCaller();
    while (!pthrCaller->bCheckShutdownRequest())
    {
        // If there's a child the name of the action, invoke it
        tCIDLib::TBoolean bRanAction = kCIDLib::False;
        if (!strAction.bIsEmpty() && bChildExists(strAction, c4ChildIndex))
        {
            bRanAction = kCIDLib::True;
            try
            {
                eNodeState = eRunChildAt(btreeOwner, c4ChildIndex);
            }

            catch(TError& errToCatch)
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);

                facCQCVoice.LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kCQCVoiceErrs::errcInp_ExceptInBT
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Internal
                    , strAction
                );
                return tCIDAI::ENodeStates::Failure;
            }

            catch(...)
            {
                facCQCVoice.LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kCQCVoiceErrs::errcInp_ExceptInBT
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Internal
                    , strAction
                );
                return tCIDAI::ENodeStates::Failure;
            }
        }

        // If not in conversation mode, then we done if we ran an action
        if (bRanAction && !bConvoMode)
            break;

        //
        //  Set a tree context variable with the target and target type of this round.
        //  If we didn't run an action, set an empty value. If we did, and it set values,
        //  then replace the ones we have.
        //
        if (bRanAction)
        {
            if (ctxTree.bFindKNumListVal(kCQCVoice::strSemVal_Info, strInfo)
            &&  !strInfo.bIsEmpty())
            {
                ctxTree.SetStringVar
                (
                    kCQCVoice::strVar_LastTarget, kCQCVoice::strVarVal_LastInfo, strInfo
                );
            }

            if (ctxTree.bFindKNumListVal(kCQCVoice::strSemVal_Target, strTarget)
            &&  !strTarget.bIsEmpty())
            {
                ctxTree.SetStringVar
                (
                    kCQCVoice::strVar_LastTarget, kCQCVoice::strVarVal_LastTarget, strTarget
                );
            }

            if (ctxTree.bFindKNumListVal(kCQCVoice::strSemVal_TarType, strTarType)
            &&  !strTarType.bIsEmpty())
            {
                ctxTree.SetStringVar
                (
                    kCQCVoice::strVar_LastTarget, kCQCVoice::strVarVal_LastType, strTarType
                );
            }
        }
         else
        {
            strInfo.Clear();
            strTarget.Clear();
            strTarType.Clear();
        }
        ctxTree.SetStringVar(kCQCVoice::strVar_LastTarget, kCQCVoice::strVarVal_LastInfo, strInfo);
        ctxTree.SetStringVar(kCQCVoice::strVar_LastTarget, kCQCVoice::strVarVal_LastTarget, strTarget);
        ctxTree.SetStringVar(kCQCVoice::strVar_LastTarget, kCQCVoice::strVarVal_LastType, strTarType);

        //
        //  Wait for a a reasonably confident event. It will respond to low confidence
        //  events by telling the user to try again.
        //
        const tCQCVoice::EWaitRepModes eRes = facCQCVoice.eWaitForGoodEvent
        (
            sprecevCur, strAction, kCQCVoice::c4ConvoWaitMSs
        );

        // If a no, or cancel, then break out always
        if (strAction.bCompareI(kCQCVoice::strAction_No)
        ||  strAction.bCompareI(kCQCVoice::strAction_CancelOp))
        {
            facCQCVoice.SpeakCallMe();
            break;
        }

        //
        //  If a timeout, then we are done, so tell them to call us if they need anything
        //  and get out.
        //
        if (eRes == tCQCVoice::EWaitRepModes::Timeout)
        {
            facCQCVoice.SpeakCallMe();
            break;
        }

        //
        //  If we got a new event, update the tree with the semantic info. If not,
        //  clear the action, so that we don't trigger again.
        //
        //  We'll now just go back to the top and try again
        //
        if (eRes == tCQCVoice::EWaitRepModes::Success)
            btreeOwner.SetAppInfo(sprecevCur.colSemList());
        else
            strAction.Clear();
    }

    // We always just return success
    return tCIDAI::ENodeStates::Success;
}
