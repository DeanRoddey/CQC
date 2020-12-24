//
// FILE NAME: CQCWebRIVA_ThisFacility.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 05/29/2017
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
//  This is the header for the facility class for the Web based RIVA client.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TFacCQCWebRIVA
//  PREFIX: fac
// ---------------------------------------------------------------------------
class CQCWEBRIVAEXP TFacCQCWebRIVA : public TFacility
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TFacCQCWebRIVA();

        TFacCQCWebRIVA(const TFacCQCWebRIVA&) = delete;
        TFacCQCWebRIVA(TFacCQCWebRIVA&&) = delete;

        ~TFacCQCWebRIVA();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TFacCQCWebRIVA& operator=(const TFacCQCWebRIVA&) = delete;
        TFacCQCWebRIVA& operator=(TFacCQCWebRIVA&&) = delete;


    private :
        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TFacCQCWebRIVA, TFacility)
};

#pragma CIDLIB_POPPACK

