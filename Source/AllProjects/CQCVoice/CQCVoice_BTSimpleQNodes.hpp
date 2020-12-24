//
// FILE NAME: CQCVoice_BTSimpleQNodes_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/07/2017
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
//  This is the header for some of the simpler behavior tree query nodes, which don't
//  do enough to justify their own files.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TBTQuCurrentTimeNode
//  PREFIX: btnode
// ---------------------------------------------------------------------------
class TBTQuCurrentTimeNode : public TBTCQCVoiceNode
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TBTQuCurrentTimeNode
        (
            const   TString&                strPath
            , const TString&                strName
        );

        TBTQuCurrentTimeNode(const TBTQuCurrentTimeNode&) = delete;

        ~TBTQuCurrentTimeNode();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TBTQuCurrentTimeNode& operator=(const TBTQuCurrentTimeNode&) = delete;


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        tCIDAI::ENodeStates eRun
        (
                    TAIBehaviorTree&        btreeOwner
        )   override;
};


// ---------------------------------------------------------------------------
//   CLASS: TBTQuCQCVersionNode
//  PREFIX: btnode
// ---------------------------------------------------------------------------
class TBTQuCQCVersionNode : public TBTCQCVoiceNode
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TBTQuCQCVersionNode
        (
            const   TString&                strPath
            , const TString&                strName
        );

        TBTQuCQCVersionNode(const TBTQuCQCVersionNode&) = delete;

        ~TBTQuCQCVersionNode();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TBTQuCQCVersionNode& operator=(const TBTQuCQCVersionNode&) = delete;


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        tCIDAI::ENodeStates eRun
        (
                    TAIBehaviorTree&        btreeOwner
        )   override;
};


// ---------------------------------------------------------------------------
//   CLASS: TBTQuTodaysDateNode
//  PREFIX: btnode
// ---------------------------------------------------------------------------
class TBTQuTodaysDateNode : public TBTCQCVoiceNode
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TBTQuTodaysDateNode
        (
            const   TString&                strPath
            , const TString&                strName
        );

        TBTQuTodaysDateNode(const TBTQuTodaysDateNode&) = delete;

        ~TBTQuTodaysDateNode();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TBTQuTodaysDateNode& operator=(const TBTQuTodaysDateNode&) = delete;


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        tCIDAI::ENodeStates eRun
        (
                    TAIBehaviorTree&        btreeOwner
        )   override;
};

#pragma CIDLIB_POPPACK
