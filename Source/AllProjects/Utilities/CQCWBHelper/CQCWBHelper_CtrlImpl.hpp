//
// FILE NAME: CQCWBHelper_CtrlImpl.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/16/2016
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
//  This is the header for the CQCWBHelper_CtrlImpl.cpp file, which implements
//  our derivative of the server side control interface for this program.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  CLASS: TWBHelperCtrlImpl
// PREFIX: orbs
// ---------------------------------------------------------------------------
class TWBHelperCtrlImpl : public TCQCWBHelperServerBase
{
    public  :
        // -------------------------------------------------------------------
        // Constructors and Destructor
        // -------------------------------------------------------------------
        TWBHelperCtrlImpl
        (
            const   TOrbObjId&              ooidToUse
        );

        ~TWBHelperCtrlImpl();


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid Close() override;

        tCIDLib::TVoid DoOp
        (
            const   tWebBrowser::ENavOps    eToDo
        )   override;

        tCIDLib::TVoid Initialize
        (
            const   TArea&                  areaPar
            , const TArea&                  areaWidget
            , const tCIDLib::TBoolean       bVisibility
            , const TString&                strInitURL
        )   override;

        tCIDLib::TVoid Ping() override;

        tCIDLib::TVoid SetArea
        (
            const   TArea&                  areaPar
            , const TArea&                  areaWidget
        )   override;

        tCIDLib::TVoid SetURL
        (
            const   TString&                strToSet
        )   override;

        tCIDLib::TVoid SetVisibility
        (
            const   tCIDLib::TBoolean       bNewState
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
        RTTIDefs(TWBHelperCtrlImpl,TCQCWBHelperServerBase)
};

