//
// FILE NAME: CQCSysCfg_ThisFacility.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/06/2011
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
//   CLASS: TFacCQCSysCfg
//  PREFIX: fac
// ---------------------------------------------------------------------------
class CQCSYSCFGEXP TFacCQCSysCfg : public TFacility
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TFacCQCSysCfg();

        TFacCQCSysCfg(const TFacCQCSysCfg&) = delete;
        TFacCQCSysCfg(TFacCQCSysCfg&&) = delete;

        ~TFacCQCSysCfg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TFacCQCSysCfg& operator=(const TFacCQCSysCfg&) = delete;
        TFacCQCSysCfg& operator=(TFacCQCSysCfg&&) = delete;


    private :
        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TFacCQCSysCfg,TFacility)

};

#pragma CIDLIB_POPPACK


