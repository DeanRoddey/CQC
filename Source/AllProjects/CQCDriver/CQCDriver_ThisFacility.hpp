//
// FILE NAME: CQCDriver_ThisFacility.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/06/2000
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
//   CLASS: TFacCQCDriver
//  PREFIX: fac
// ---------------------------------------------------------------------------
class CQCDRIVEREXPORT TFacCQCDriver : public TFacility
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TFacCQCDriver();

        TFacCQCDriver(const TFacCQCDriver&) = delete;
        TFacCQCDriver(TFacCQCDriver&&) = delete;

        ~TFacCQCDriver();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TFacCQCDriver& operator=(const TFacCQCDriver&) = delete;
        TFacCQCDriver& operator=(TFacCQCDriver&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid UpdatePoolItemsStat
        (
            const   tCIDLib::TCard4         c4Used
            , const tCIDLib::TCard4         c4Free
        );


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        //
        //  m_sciCmdItemsFred
        //  m_sciCmdItemsUsed
        //      We maintain stats cache items for the count of pool items that are
        //      currently used and free. This is a very important thing to
        //      track. If we have errors and start eating up these items, we'll be
        //      in a mess of trouble. The driver base class calls us to keep
        //      this udpated.
        // -------------------------------------------------------------------
        TStatsCacheItem         m_sciCmdItemsFree;
        TStatsCacheItem         m_sciCmdItemsUsed;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TFacCQCDriver,TFacility)
};

#pragma CIDLIB_POPPACK


