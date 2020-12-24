//
// FILE NAME: CQCVoice_BTLightingNodes_.hpp
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
//  This is the header for the BT nodes that are related to lighting operations.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once

#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: TBTCmdBringUpLightsNode
//  PREFIX: btnode
// ---------------------------------------------------------------------------
class TBTCmdBringUpLightsNode : public TBTCQCVoiceNode
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TBTCmdBringUpLightsNode
        (
            const   TString&                strPath
            , const TString&                strName
        );

        TBTCmdBringUpLightsNode(const TBTCmdBringUpLightsNode&) = delete;

        ~TBTCmdBringUpLightsNode();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TBTCmdBringUpLightsNode& operator=(const TBTCmdBringUpLightsNode&) = delete;


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        tCIDAI::ENodeStates eRun
        (
                    TAIBehaviorTree&        btreeOwner
        )   override;
};



// ---------------------------------------------------------------------------
//   CLASS: TBTCmdKillLightsNode
//  PREFIX: btnode
// ---------------------------------------------------------------------------
class TBTCmdKillLightsNode : public TBTCQCVoiceNode
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TBTCmdKillLightsNode
        (
            const   TString&                strPath
            , const TString&                strName
        );

        TBTCmdKillLightsNode(const TBTCmdKillLightsNode&) = delete;

        ~TBTCmdKillLightsNode();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TBTCmdKillLightsNode& operator=(const TBTCmdKillLightsNode&) = delete;


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        tCIDAI::ENodeStates eRun
        (
                    TAIBehaviorTree&        btreeOwner
        )   override;
};


// ---------------------------------------------------------------------------
//   CLASS: TBTCmdSetLightSwitchNode
//  PREFIX: btnode
// ---------------------------------------------------------------------------
class TBTCmdSetLightSwitchNode : public TBTCQCVoiceNode
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TBTCmdSetLightSwitchNode
        (
            const   TString&                strPath
            , const TString&                strName
        );

        TBTCmdSetLightSwitchNode(const TBTCmdSetLightSwitchNode&) = delete;

        ~TBTCmdSetLightSwitchNode();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TBTCmdSetLightSwitchNode& operator=(const TBTCmdSetLightSwitchNode&) = delete;


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        tCIDAI::ENodeStates eRun
        (
                    TAIBehaviorTree&        btreeOwner
        )   override;
};


// ---------------------------------------------------------------------------
//   CLASS: TBTCmdSetLightLevelNode
//  PREFIX: btnode
// ---------------------------------------------------------------------------
class TBTCmdSetLightLevelNode : public TBTCQCVoiceNode
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TBTCmdSetLightLevelNode
        (
            const   TString&                strPath
            , const TString&                strName
        );

        TBTCmdSetLightLevelNode(const TBTCmdSetLightLevelNode&) = delete;

        ~TBTCmdSetLightLevelNode();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TBTCmdSetLightLevelNode& operator=(const TBTCmdSetLightLevelNode&) = delete;


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        tCIDAI::ENodeStates eRun
        (
                    TAIBehaviorTree&        btreeOwner
        )   override;
};


// ---------------------------------------------------------------------------
//   CLASS: TBTQuLightSwitchNode
//  PREFIX: btnode
// ---------------------------------------------------------------------------
class TBTQuLightSwitchNode : public TBTCQCVoiceNode
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TBTQuLightSwitchNode
        (
            const   TString&                strPath
            , const TString&                strName
        );

        TBTQuLightSwitchNode(const TBTQuLightSwitchNode&) = delete;

        ~TBTQuLightSwitchNode();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TBTQuLightSwitchNode& operator=(const TBTQuLightSwitchNode&) = delete;


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        tCIDAI::ENodeStates eRun
        (
                    TAIBehaviorTree&        btreeOwner
        )   override;
};


// ---------------------------------------------------------------------------
//   CLASS: TBTQuLightSwitchStateNode
//  PREFIX: btnode
// ---------------------------------------------------------------------------
class TBTQuLightSwitchStateNode : public TBTCQCVoiceNode
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TBTQuLightSwitchStateNode
        (
            const   TString&                strPath
            , const TString&                strName
        );

        TBTQuLightSwitchStateNode(const TBTQuLightSwitchStateNode&) = delete;

        ~TBTQuLightSwitchStateNode();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TBTQuLightSwitchStateNode& operator=(const TBTQuLightSwitchStateNode&) = delete;


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        tCIDAI::ENodeStates eRun
        (
                    TAIBehaviorTree&        btreeOwner
        )   override;
};

#pragma CIDLIB_POPPACK
