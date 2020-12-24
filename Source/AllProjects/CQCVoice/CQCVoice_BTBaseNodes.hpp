//
// FILE NAME: CQCVoice_BTStartNode.hpp
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
//  This is the header for the core nodes of our behavior tree.
//
//  A special one is the root tree. It is our top level node. It has children for all
//  of the possible commands that can be done from a fresh starting point. This guy gets
//  the next available speech event and sees if the Action semantic value matches one of
//  our child node's names. If so, it invokes that child. Else it indicates it doesn't
//  understand that command. This guy doesn't return from its run method until it is
//  asked to stop. So it is always waiting for the next available speech event.
//
//  The other is a base class from which we derive all of our other nodes, so that we
//  can impose common behavior or data on them if needed.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

class TSpeechEvent;


// ---------------------------------------------------------------------------
//   CLASS: TBTCQCVoiceNode
//  PREFIX: btnode
// ---------------------------------------------------------------------------
class TBTCQCVoiceNode : public TAIBTNode
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TBTCQCVoiceNode(const TBTCQCVoiceNode&) = delete;

        ~TBTCQCVoiceNode();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TBTCQCVoiceNode& operator=(const TBTCQCVoiceNode&) = delete;


    protected :
        // -------------------------------------------------------------------
        //  Hidden constructors
        // -------------------------------------------------------------------
        TBTCQCVoiceNode
        (
           const    TString&                strPath
           ,  const TString&                strName
            , const TString&                strNodeType
            , const tCIDLib::TCard4         c4MinChildCnt
            , const tCIDLib::TCard4         c4MaxChildCnt
        );
};



// ---------------------------------------------------------------------------
//   CLASS: TBTStartNode
//  PREFIX: btnode
// ---------------------------------------------------------------------------
class TBTStartNode : public TAIBTNode
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TBTStartNode
        (
            const   TString&                strPath
            , const TString&                strName
        );

        TBTStartNode(const TBTStartNode&) = delete;

        ~TBTStartNode();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TBTStartNode& operator=(const TBTStartNode&) = delete;


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        tCIDAI::ENodeStates eRun
        (
                    TAIBehaviorTree&        btreeOwner
        )   override;

};

#pragma CIDLIB_POPPACK
