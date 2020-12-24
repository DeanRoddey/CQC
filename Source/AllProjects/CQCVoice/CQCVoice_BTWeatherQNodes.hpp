//
// FILE NAME: CQCVoice_BTWeatherQNodes_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/14/2017
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
//  This is the header for the weather related query nodes
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TBTQuCurrentTempNode
//  PREFIX: btnode
// ---------------------------------------------------------------------------
class TBTQuCurrentTempNode : public TBTCQCVoiceNode
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TBTQuCurrentTempNode
        (
            const   TString&                strPath
            , const TString&                strName
        );

        TBTQuCurrentTempNode(const TBTQuCurrentTempNode&) = delete;

        ~TBTQuCurrentTempNode();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TBTQuCurrentTempNode& operator=(const TBTQuCurrentTempNode&) = delete;


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        tCIDAI::ENodeStates eRun
        (
                    TAIBehaviorTree&        btreeOwner
        )   override;
};


// ---------------------------------------------------------------------------
//   CLASS: TBTQuWeatherShortFCNode
//  PREFIX: btnode
// ---------------------------------------------------------------------------
class TBTQuWeatherShortFCNode : public TBTCQCVoiceNode
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TBTQuWeatherShortFCNode
        (
            const   TString&                strPath
            , const TString&                strName
        );

        TBTQuWeatherShortFCNode(const TBTQuWeatherShortFCNode&) = delete;

        ~TBTQuWeatherShortFCNode();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TBTQuWeatherShortFCNode& operator=(const TBTQuWeatherShortFCNode&) = delete;


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        tCIDAI::ENodeStates eRun
        (
                    TAIBehaviorTree&        btreeOwner
        )   override;
};

#pragma CIDLIB_POPPACK
