//
// FILE NAME: CQCGWCl_ThisFacility.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/02/2013
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


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: TFacCQCGWCl
//  PREFIX: fac
// ---------------------------------------------------------------------------
class CQCGWCLEXPORT TFacCQCGWCl : public TFacility
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TFacCQCGWCl();

        TFacCQCGWCl(const TFacCQCGWCl&) = delete;
        TFacCQCGWCl(TFacCQCGWCl&&) = delete;

        ~TFacCQCGWCl();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TFacCQCGWCl& operator=(const TFacCQCGWCl&) = delete;
        TFacCQCGWCl& operator=(TFacCQCGWCl&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid InitCMLRuntime();


    private :
        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TFacCQCGWCl,TFacility)
};

#pragma CIDLIB_POPPACK


