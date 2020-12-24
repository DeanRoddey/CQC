//
// FILE NAME: CQCWCHelper_CtrlImpl.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/05/2017
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
//  This is the header for the CQCWCHelper_CtrlImpl.cpp file, which implements
//  our derivative of the server side control interface for this program.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  CLASS: TWCHelperCtrlImpl
// PREFIX: orbs
// ---------------------------------------------------------------------------
class TWCHelperCtrlImpl : public TCQCWCHelperServerBase
{
    public  :
        // -------------------------------------------------------------------
        // Constructors and Destructor
        // -------------------------------------------------------------------
        TWCHelperCtrlImpl
        (
            const   TOrbObjId&              ooidToUse
        );

        ~TWCHelperCtrlImpl();


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid Close() override;

        tCIDLib::TVoid Initialize
        (
            const   TArea&                  areaPar
            , const TArea&                  areaWidget
            , const tCIDLib::TBoolean       bVisibility
        )   override;

        tCIDLib::TVoid Ping() override;

        tCIDLib::TVoid SetArea
        (
            const   TArea&                  areaPar
            , const TArea&                  areaWidget
        )   override;

        tCIDLib::TVoid SetMute
        (
            const   tCIDLib::TBoolean       bNewState
        )   override;

        tCIDLib::TVoid SetVisibility
        (
            const   tCIDLib::TBoolean       bNewState
        )   override;

        tCIDLib::TVoid SetVolume
        (
            const   tCIDLib::TCard4         c4ToSet
        )   override;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid Initialize() override;

        tCIDLib::TVoid Terminate() override;


    private :
        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TWCHelperCtrlImpl,TCQCWCHelperServerBase)
};

