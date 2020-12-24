//
// FILE NAME: CQCSrvDrvTI_ThisFacility.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/02/2018
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
//   CLASS: TFacCQCSrvDrvTI
//  PREFIX: fac
// ---------------------------------------------------------------------------
class CQCSRVDRVTIEXPORT TFacCQCSrvDrvTI : public TFacility
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TFacCQCSrvDrvTI();

        TFacCQCSrvDrvTI(const TFacCQCSrvDrvTI&) = delete;

        ~TFacCQCSrvDrvTI();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TFacCQCSrvDrvTI& operator=(const TFacCQCSrvDrvTI&) = delete;


    private :
        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TFacCQCSrvDrvTI,TFacility)
};

#pragma CIDLIB_POPPACK

