//
// FILE NAME: CQCAct_ThisFacility.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/27/2015
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
//  This file defines facility class for this facility.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TFacCQCAct
//  PREFIX: fac
// ---------------------------------------------------------------------------
class CQCACTEXPORT TFacCQCAct : public TFacility
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TFacCQCAct();

        TFacCQCAct(const TFacCQCAct&) = delete;
        TFacCQCAct(TFacCQCAct&&) = delete;

        ~TFacCQCAct();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TFacCQCAct& operator=(const TFacCQCAct&) = delete;
        TFacCQCAct& operator=(TFacCQCAct&&) = delete;


    private :
        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TFacCQCAct,TFacility)
};

#pragma CIDLIB_POPPACK

