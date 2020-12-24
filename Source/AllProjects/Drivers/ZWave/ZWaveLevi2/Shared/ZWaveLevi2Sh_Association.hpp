//
// FILE NAME: ZWaveLevi2Sh_Association.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/05/2014
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
//  This is the header file for the ZWaveLevi2Sh_Association.cpp file, which implements
//  the TZWCmdClass derivatives for the association command class. This guy has no
//  fields, but we need it to store secure and multi-instance info on this class,
//  which is needed any time a message is sent for any class.
//
//  Since it has no fields, we don't do override any of the virtuals. We just
//  provide some of our own methods for doing association related operations, to
//  keep that detail stuff out of the driver itself.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: TZWCCAssociation
//  PREFIX: zwcc
// ---------------------------------------------------------------------------
class ZWLEVI2SHEXPORT TZWCCAssociation : public TZWCmdClass
{
    public :
        // -------------------------------------------------------------------
        //  Consructors and Destructor
        // -------------------------------------------------------------------
        TZWCCAssociation
        (
            const   TZWDevClass&            zwdcInst
        );

        ~TZWCCAssociation();


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid InitVals
        (
            const   TZWaveUnit&             unitMe
            ,       MZWLeviSrvFuncs&        mzwsfLevi
        );

        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bDelAllAssociation
        (
                    TZWaveUnit&             unitSrc
            , const tCIDLib::TCard4         c4InstId
            , const tCIDLib::TCard4         c4GroupId
            ,       MZWLeviSrvFuncs&        mzwsfLevi
        );

        tCIDLib::TBoolean bDelAssociation
        (
                    TZWaveUnit&             unitSrc
            , const tCIDLib::TCard4         c4InstId
            , const tCIDLib::TCard4         c4GroupId
            , const TZWaveUnit&             unitRemove
            ,       MZWLeviSrvFuncs&        mzwsfLevi
        );

        tCIDLib::TBoolean bDoAssociation
        (
                    TZWaveUnit&             unitSrc
            , const tCIDLib::TCard4         c4InstId
            , const TZWaveUnit&             unitTar
            , const tCIDLib::TCard4         c4GroupId
            ,       MZWLeviSrvFuncs&        mzwsfLevi
        );

        tCIDLib::TBoolean bQueryAssociation
        (
                    TZWaveUnit&             unitSrc
            , const tCIDLib::TCard4         c4GroupId
            ,       tCIDLib::TCardList&     fcolVals
            ,       MZWLeviSrvFuncs&        mzwsfLevi
        );


    private :
        // -------------------------------------------------------------------
        //  Unimplemented
        // -------------------------------------------------------------------
        TZWCCAssociation();
        TZWCCAssociation(const TZWCCAssociation&);
        tCIDLib::TVoid operator=(const TZWCCAssociation&);


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWCCAssociation, TZWCmdClass)
};

#pragma CIDLIB_POPPACK



