//
// FILE NAME: CQCVoice_BTMediaNodes_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/16/2017
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
//  This is the header for media related behavior tree nodes.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once

#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TBTCmdClearMusicPLNode
//  PREFIX: btnode
// ---------------------------------------------------------------------------
class TBTCmdClearMusicPLNode : public TBTCQCVoiceNode
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TBTCmdClearMusicPLNode
        (
            const   TString&                strPath
            , const TString&                strName
        );

        TBTCmdClearMusicPLNode(const TBTCmdClearMusicPLNode&) = delete;

        ~TBTCmdClearMusicPLNode();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TBTCmdClearMusicPLNode& operator=(const TBTCmdClearMusicPLNode&) = delete;


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        tCIDAI::ENodeStates eRun
        (
                    TAIBehaviorTree&        btreeOwner
        )   override;
};


// ---------------------------------------------------------------------------
//   CLASS: TBTCmdNextMusicItemNode
//  PREFIX: btnode
// ---------------------------------------------------------------------------
class TBTCmdNextMusicItemNode : public TBTCQCVoiceNode
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TBTCmdNextMusicItemNode
        (
            const   TString&                strPath
            , const TString&                strName
        );

        TBTCmdNextMusicItemNode(const TBTCmdNextMusicItemNode&) = delete;

        ~TBTCmdNextMusicItemNode();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TBTCmdNextMusicItemNode& operator=(const TBTCmdNextMusicItemNode&) = delete;


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        tCIDAI::ENodeStates eRun
        (
                    TAIBehaviorTree&        btreeOwner
        )   override;
};



// ---------------------------------------------------------------------------
//   CLASS: TBTCmdPlayMusicFromCatNode
//  PREFIX: btnode
// ---------------------------------------------------------------------------
class TBTCmdPlayMusicFromCatNode : public TBTCQCVoiceNode
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TBTCmdPlayMusicFromCatNode
        (
            const   TString&                strPath
            , const TString&                strName
        );

        TBTCmdPlayMusicFromCatNode(const TBTCmdPlayMusicFromCatNode&) = delete;

        ~TBTCmdPlayMusicFromCatNode();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TBTCmdPlayMusicFromCatNode& operator=(const TBTCmdPlayMusicFromCatNode&) = delete;


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        tCIDAI::ENodeStates eRun
        (
                    TAIBehaviorTree&        btreeOwner
        )   override;
};


// ---------------------------------------------------------------------------
//   CLASS: TBTCmdPlayMusicPLNode
//  PREFIX: btnode
// ---------------------------------------------------------------------------
class TBTCmdPlayMusicPLNode : public TBTCQCVoiceNode
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TBTCmdPlayMusicPLNode
        (
            const   TString&                strPath
            , const TString&                strName
        );

        TBTCmdPlayMusicPLNode(const TBTCmdPlayMusicPLNode&) = delete;

        ~TBTCmdPlayMusicPLNode();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TBTCmdPlayMusicPLNode& operator=(const TBTCmdPlayMusicPLNode&) = delete;


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        tCIDAI::ENodeStates eRun
        (
                    TAIBehaviorTree&        btreeOwner
        )   override;
};


// ---------------------------------------------------------------------------
//   CLASS: TBTCmdMusicTransportNode
//  PREFIX: btnode
// ---------------------------------------------------------------------------
class TBTCmdMusicTransportNode : public TBTCQCVoiceNode
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TBTCmdMusicTransportNode
        (
            const   TString&                strPath
            , const TString&                strName
        );

        TBTCmdMusicTransportNode(const TBTCmdMusicTransportNode&) = delete;

        ~TBTCmdMusicTransportNode();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TBTCmdMusicTransportNode& operator=(const TBTCmdMusicTransportNode&) = delete;


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        tCIDAI::ENodeStates eRun
        (
                    TAIBehaviorTree&        btreeOwner
        )   override;
};



// ---------------------------------------------------------------------------
//   CLASS: TBTCmdSetMusicMuteNode
//  PREFIX: btnode
// ---------------------------------------------------------------------------
class TBTCmdSetMusicMuteNode : public TBTCQCVoiceNode
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TBTCmdSetMusicMuteNode
        (
            const   TString&                strPath
            , const TString&                strName
        );

        TBTCmdSetMusicMuteNode(const TBTCmdSetMusicMuteNode&) = delete;

        ~TBTCmdSetMusicMuteNode();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TBTCmdSetMusicMuteNode& operator=(const TBTCmdSetMusicMuteNode&) = delete;


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        tCIDAI::ENodeStates eRun
        (
                    TAIBehaviorTree&        btreeOwner
        )   override;
};


// ---------------------------------------------------------------------------
//   CLASS: TBTCmdSetMusicPLModeNode
//  PREFIX: btnode
// ---------------------------------------------------------------------------
class TBTCmdSetMusicPLModeNode : public TBTCQCVoiceNode
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TBTCmdSetMusicPLModeNode
        (
            const   TString&                strPath
            , const TString&                strName
        );

        TBTCmdSetMusicPLModeNode(const TBTCmdSetMusicPLModeNode&) = delete;

        ~TBTCmdSetMusicPLModeNode();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TBTCmdSetMusicPLModeNode& operator=(const TBTCmdSetMusicPLModeNode&) = delete;


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        tCIDAI::ENodeStates eRun
        (
                    TAIBehaviorTree&        btreeOwner
        )   override;
};



// ---------------------------------------------------------------------------
//   CLASS: TBTCmdSetMusicVolumeNode
//  PREFIX: btnode
// ---------------------------------------------------------------------------
class TBTCmdSetMusicVolumeNode : public TBTCQCVoiceNode
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TBTCmdSetMusicVolumeNode
        (
            const   TString&                strPath
            , const TString&                strName
        );

        TBTCmdSetMusicVolumeNode(const TBTCmdSetMusicVolumeNode&) = delete;

        ~TBTCmdSetMusicVolumeNode();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TBTCmdSetMusicVolumeNode& operator=(const TBTCmdSetMusicVolumeNode&) = delete;


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        tCIDAI::ENodeStates eRun
        (
                    TAIBehaviorTree&        btreeOwner
        )   override;
};

#pragma CIDLIB_POPPACK
