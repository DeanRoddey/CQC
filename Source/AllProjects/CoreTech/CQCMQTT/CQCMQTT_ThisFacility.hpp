//
// FILE NAME: CQCMQTT_ThisFacility.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/11/2018
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
//  This is the header for the facility class for this facility. It just provides the
//  unusual 'global to the facility' type functionality.
//

// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TFacCQCMQTT
//  PREFIX: fac
// ---------------------------------------------------------------------------
class CQCMQTTEXPORT TFacCQCMQTT : public TFacility
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TFacCQCMQTT();

        TFacCQCMQTT(const TFacCQCMQTT&) = delete;
        TFacCQCMQTT(TFacCQCMQTT&&) = delete;

        ~TFacCQCMQTT();


        // ---------------------------------------------------------------------------
        //  Public operators
        // ---------------------------------------------------------------------------
        TFacCQCMQTT& operator=(const TFacCQCMQTT&) = delete;
        TFacCQCMQTT& operator=(TFacCQCMQTT&&) = delete;


        // ---------------------------------------------------------------------------
        //  Public, non-virtual methods
        // ---------------------------------------------------------------------------
        tCIDLib::TBoolean bVerbose() const
        {
            return m_bVerbose;
        }


    private :
        // ---------------------------------------------------------------------------
        //  Private data members
        //
        //  m_bVerbose
        //      We can be told to do verbose logging of msgs activity.
        // ---------------------------------------------------------------------------
        tCIDLib::TBoolean   m_bVerbose;
};

#pragma CIDLIB_POPPACK
