//
// FILE NAME: CQCPollEng_ThisFacility.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/25/2008
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
//   CLASS: TFacCQCPollEng
//  PREFIX: fac
// ---------------------------------------------------------------------------
class CQCPOLLENGEXPORT TFacCQCPollEng : public TFacility
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TFacCQCPollEng();

        TFacCQCPollEng(const TFacCQCPollEng&) = delete;
        TFacCQCPollEng(TFacCQCPollEng&&) = delete;

        ~TFacCQCPollEng();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TFacCQCPollEng& operator=(const TFacCQCPollEng&) = delete;
        TFacCQCPollEng& operator=(TFacCQCPollEng&&) = delete;


    private :
        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TFacCQCPollEng,TFacility)
};

#pragma CIDLIB_POPPACK
