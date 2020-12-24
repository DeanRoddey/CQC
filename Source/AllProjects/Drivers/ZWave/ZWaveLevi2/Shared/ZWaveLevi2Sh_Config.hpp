//
// FILE NAME: ZWaveLevi2Sh_Config.hpp
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
//  This is the header file for the ZWaveLevi2Sh_Config.cpp file, which implements
//  the TZWCmdClass derivatives for the Configuration command class. This guy has no
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
//   CLASS: TZWCCConfig
//  PREFIX: zwcc
// ---------------------------------------------------------------------------
class ZWLEVI2SHEXPORT TZWCCConfig : public TZWCmdClass
{
    public :
        // -------------------------------------------------------------------
        //  Consructors and Destructor
        // -------------------------------------------------------------------
        TZWCCConfig
        (
            const   TZWDevClass&            zwdcInst
        );

        ~TZWCCConfig();


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
        tCIDLib::TBoolean bQueryConfigParm
        (
                    TZWaveUnit&             unitSrc
            , const tCIDLib::TCard4         c4ParmId
            ,       tCIDLib::TCard4&        c4ToFill
            ,       MZWLeviSrvFuncs&        mzwsfLevi
        );

        tCIDLib::TBoolean bSetConfigParm
        (
                    TZWaveUnit&             unitTar
            , const tCIDLib::TCard4         c4ParmId
            , const tCIDLib::TCard4         c4ByteCnt
            , const tCIDLib::TCard4         c4Value
            ,       MZWLeviSrvFuncs&        mzwsfLevi
        );


    private :
        // -------------------------------------------------------------------
        //  Unimplemented
        // -------------------------------------------------------------------
        TZWCCConfig();
        TZWCCConfig(const TZWCCConfig&);
        tCIDLib::TVoid operator=(const TZWCCConfig&);


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWCCConfig, TZWCmdClass)
};

#pragma CIDLIB_POPPACK


