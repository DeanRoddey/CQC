//
// FILE NAME: ZWaveLeviSh_Ctrl.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/10/2012
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
//  This is the header file for the ZWaveLevi_Ctrl.cpp file, which
//  implements the TZWaveCtrl class. Because we have abstracted out the
//  unit classes, so that we can plug in various types of units over time
//  without changing the main driver, we have to have some way for them to
//  access the driver itself to do certain things. Most stuff they need to
//  do is doable via the base driver class. But, for anything that is in
//  thea ctual Leviton driver class, we have to expose it via this guy.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TZWaveCtrl
//  PREFIX: zwctrl
// ---------------------------------------------------------------------------
class ZWLEVISHEXPORT MZWaveCtrl
{
    public :
        // -------------------------------------------------------------------
        //  Destructor
        // -------------------------------------------------------------------
        ~MZWaveCtrl();


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------


    protected :
        // -------------------------------------------------------------------
        //  Hidden constructors
        // -------------------------------------------------------------------
        MZWaveCtrl();


    private :
        // -------------------------------------------------------------------
        //  Unimplemented
        // -------------------------------------------------------------------
        MZWaveCtrl(const MZWaveCtrl&);
        tCIDLib::TVoid operator=(const MZWaveCtrl&);
};

#pragma CIDLIB_POPPACK

