//
// FILE NAME: CQCVoice_BTHVACNodes.hpp
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
//  This is the header for the HCAC related nodes.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once

#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TBTCmdSetThermoHLSPNode
//  PREFIX: btnode
// ---------------------------------------------------------------------------
class TBTCmdSetThermoHLSPNode : public TBTCQCVoiceNode
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TBTCmdSetThermoHLSPNode
        (
            const   TString&                strPath
            , const TString&                strName
        );

        TBTCmdSetThermoHLSPNode(const TBTCmdSetThermoHLSPNode&) = delete;

        ~TBTCmdSetThermoHLSPNode();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TBTCmdSetThermoHLSPNode& operator=(const TBTCmdSetThermoHLSPNode&) = delete;


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        tCIDAI::ENodeStates eRun
        (
                    TAIBehaviorTree&        btreeOwner
        )   override;
};


// ---------------------------------------------------------------------------
//   CLASS: TBTQuThermoHLSPNode
//  PREFIX: btnode
// ---------------------------------------------------------------------------
class TBTQuThermoHLSPNode : public TBTCQCVoiceNode
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TBTQuThermoHLSPNode
        (
            const   TString&                strPath
            , const TString&                strName
        );

        TBTQuThermoHLSPNode(const TBTQuThermoHLSPNode&) = delete;

        ~TBTQuThermoHLSPNode();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TBTQuThermoHLSPNode& operator=(const TBTQuThermoHLSPNode&) = delete;


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        tCIDAI::ENodeStates eRun
        (
                    TAIBehaviorTree&        btreeOwner
        )   override;
};

#pragma CIDLIB_POPPACK
