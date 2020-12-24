//
// FILE NAME: ZWaveUSB3Sh_Unit_Controller_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/04/2017
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
//  We see controllers in the unit list, and to avoid lots of special casing we want
//  to treat them like regular units. But we don't really need any info fro them and
//  they will create no fields. Any that we have a device info file for will point them
//  at this class. Any that we don't, we just force them to some default setup with this
//  as their header. This is just a do-nothing handler.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once

#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: TZWController
//  PREFIX: unit
// ---------------------------------------------------------------------------
class TZWController : public TZWaveUnit
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TZWController() = delete;

        TZWController
        (
                TZWUnitInfo* const      punitiOwner
        );

        TZWController(const TZWController&) = delete;

        ~TZWController();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TZWController& operator=(const TZWController&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bPrepare() override;


    private :
        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWController, TZWaveUnit)
};

#pragma CIDLIB_POPPACK
