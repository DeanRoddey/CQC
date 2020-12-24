//
// FILE NAME: CQCMEng_ThisFacility.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/24/2015
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
//   CLASS: TFacCQCMEng
//  PREFIX: fac
// ---------------------------------------------------------------------------
class CQCMENGEXPORT TFacCQCMEng : public TFacility
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TFacCQCMEng();

        TFacCQCMEng(const TFacCQCMEng&) = delete;
        TFacCQCMEng(TFacCQCMEng&&) = delete;

        ~TFacCQCMEng();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TFacCQCMEng& operator=(const TFacCQCMEng&) = delete;
        TFacCQCMEng& operator=(TFacCQCMEng&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid InitCMLRuntime
        (
            const   TCQCSecToken&           sectUser
        );


    private :
        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TFacCQCMEng,TFacility)
};

#pragma CIDLIB_POPPACK


