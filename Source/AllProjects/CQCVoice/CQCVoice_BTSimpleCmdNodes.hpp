//
// FILE NAME: CQCVoice_BTSimpleCmdNodes_.hpp
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
//  This is the header for some of the simpler behavior tree command nodes, which don't
//  aren't part of any category sufficient to justify a separate file.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TBTCmdReloadCfgNode
//  PREFIX: btnode
// ---------------------------------------------------------------------------
class TBTCmdReloadCfgNode : public TBTCQCVoiceNode
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TBTCmdReloadCfgNode
        (
            const   TString&                strPath
            , const TString&                strName
        );

        TBTCmdReloadCfgNode(const TBTCmdReloadCfgNode&) = delete;

        ~TBTCmdReloadCfgNode();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TBTCmdReloadCfgNode& operator=(const TBTCmdReloadCfgNode&) = delete;


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        tCIDAI::ENodeStates eRun
        (
                    TAIBehaviorTree&        btreeOwner
        )   override;
};



// ---------------------------------------------------------------------------
//   CLASS: TBTCmdSetItToNumNode
//  PREFIX: btnode
// ---------------------------------------------------------------------------
class TBTCmdSetItToNumNode : public TBTCQCVoiceNode
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TBTCmdSetItToNumNode
        (
            const   TString&                strPath
            , const TString&                strName
        );

        TBTCmdSetItToNumNode(const TBTCmdSetItToNumNode&) = delete;

        ~TBTCmdSetItToNumNode();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TBTCmdSetItToNumNode& operator=(const TBTCmdSetItToNumNode&) = delete;


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        tCIDAI::ENodeStates eRun
        (
                    TAIBehaviorTree&        btreeOwner
        )   override;
};


// ---------------------------------------------------------------------------
//   CLASS: TBTCmdSetRoomModeNode
//  PREFIX: btnode
// ---------------------------------------------------------------------------
class TBTCmdSetRoomModeNode : public TBTCQCVoiceNode
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TBTCmdSetRoomModeNode
        (
            const   TString&                strPath
            , const TString&                strName
        );

        TBTCmdSetRoomModeNode(const TBTCmdSetRoomModeNode&) = delete;

        ~TBTCmdSetRoomModeNode();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TBTCmdSetRoomModeNode& operator=(const TBTCmdSetRoomModeNode&) = delete;


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        tCIDAI::ENodeStates eRun
        (
                    TAIBehaviorTree&        btreeOwner
        )   override;
};


// ---------------------------------------------------------------------------
//   CLASS: TBTCmdTurnItOffOnNode
//  PREFIX: btnode
// ---------------------------------------------------------------------------
class TBTCmdTurnItOffOnNode : public TBTCQCVoiceNode
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TBTCmdTurnItOffOnNode
        (
            const   TString&                strPath
            , const TString&                strName
        );

        TBTCmdTurnItOffOnNode(const TBTCmdTurnItOffOnNode&) = delete;

        ~TBTCmdTurnItOffOnNode();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TBTCmdTurnItOffOnNode& operator=(const TBTCmdTurnItOffOnNode&) = delete;


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        tCIDAI::ENodeStates eRun
        (
                    TAIBehaviorTree&        btreeOwner
        )   override;
};


#pragma CIDLIB_POPPACK
