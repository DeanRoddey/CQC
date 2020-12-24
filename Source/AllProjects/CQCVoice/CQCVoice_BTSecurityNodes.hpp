//
// FILE NAME: CQCVoice_BTSecurityNodes.hpp
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
//  This is the header for the security related nodes.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TBTCmdArmSecSystemNode
//  PREFIX: btnode
// ---------------------------------------------------------------------------
class TBTCmdArmSecSystemNode : public TBTCQCVoiceNode
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TBTCmdArmSecSystemNode
        (
            const   TString&                strPath
            , const TString&                strName
        );

        TBTCmdArmSecSystemNode(const TBTCmdArmSecSystemNode&) = delete;

        ~TBTCmdArmSecSystemNode();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TBTCmdArmSecSystemNode& operator=(const TBTCmdArmSecSystemNode&) = delete;


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        tCIDAI::ENodeStates eRun
        (
                    TAIBehaviorTree&        btreeOwner
        )   override;
};


// ---------------------------------------------------------------------------
//   CLASS: TBTCmdDisarmSecSystemNode
//  PREFIX: btnode
// ---------------------------------------------------------------------------
class TBTCmdDisarmSecSystemNode : public TBTCQCVoiceNode
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TBTCmdDisarmSecSystemNode
        (
            const   TString&                strPath
            , const TString&                strName
        );

        TBTCmdDisarmSecSystemNode(const TBTCmdDisarmSecSystemNode&) = delete;

        ~TBTCmdDisarmSecSystemNode();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TBTCmdDisarmSecSystemNode& operator=(const TBTCmdDisarmSecSystemNode&) = delete;


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        tCIDAI::ENodeStates eRun
        (
                    TAIBehaviorTree&        btreeOwner
        )   override;
};



// ---------------------------------------------------------------------------
//   CLASS: TBTQuSecZoneStateNode
//  PREFIX: btnode
// ---------------------------------------------------------------------------
class TBTQuSecZoneStateNode : public TBTCQCVoiceNode
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TBTQuSecZoneStateNode
        (
            const   TString&                strPath
            , const TString&                strName
        );

        TBTQuSecZoneStateNode(const TBTQuSecZoneStateNode&) = delete;

        ~TBTQuSecZoneStateNode();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TBTQuSecZoneStateNode& operator=(const TBTQuSecZoneStateNode&) = delete;


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        tCIDAI::ENodeStates eRun
        (
                    TAIBehaviorTree&        btreeOwner
        )   override;
};

#pragma CIDLIB_POPPACK

