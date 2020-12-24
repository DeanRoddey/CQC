//
// FILE NAME: HAIOmniSh_DrvConfig.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/13/2006
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
//  This is the header for the HAIOmniSh_DrvConfig.cpp file, which implements
//  the class that is used to store and pass around the Omni driver
//  configuration data. The user has to tell us what zones, areas, thermos,
//  etc... they want us to handle. This has to be stored and reloaded and
//  passed between the server and client sides of the driver.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: TOmniDrvCfg
//  PREFIX: dcfg
// ---------------------------------------------------------------------------
class HAIOMNISHEXPORT TOmniDrvCfg : public TObject, public MStreamable
{
    public :
        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static tCIDLib::TCard4 c4MakeItemName
        (
            const   tHAIOmniSh::EItemTypes  eType
            , const tCIDLib::TCard4         c4ItemNum
            ,       TString&                strToFill
        );


        // -------------------------------------------------------------------
        //  Destructor
        // -------------------------------------------------------------------
        TOmniDrvCfg();

        TOmniDrvCfg
        (
            const   tHAIOmniSh::TFlagArray  fcolAreas
            , const tHAIOmniSh::TPLCArray   fcolPLCUnits
            , const tHAIOmniSh::TFlagArray  fcolThermos
            , const tHAIOmniSh::TZoneArray  fcolZones
        );

        TOmniDrvCfg
        (
            const   TOmniDrvCfg&            dcfgToCopy
        );

        ~TOmniDrvCfg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TOmniDrvCfg& operator=
        (
            const   TOmniDrvCfg&            dcfgToAssign
        );


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bParseFrom
        (
                    TTextInStream&          strmSrc
        );

        const tHAIOmniSh::TFlagArray& fcolAreas() const;

        tHAIOmniSh::TFlagArray& fcolAreas();

        const tHAIOmniSh::TPLCArray& fcolPLCUnits() const;

        tHAIOmniSh::TPLCArray& fcolPLCUnits();

        const tHAIOmniSh::TFlagArray& fcolThermos() const;

        tHAIOmniSh::TFlagArray& fcolThermos();

        const tHAIOmniSh::TZoneArray& fcolZones() const;

        tHAIOmniSh::TZoneArray& fcolZones();

        tCIDLib::TVoid FormatTo
        (
                TTextOutStream&             strTar
        );

        tCIDLib::TVoid Reset();


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        );

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        //
        //  m_fcolAreas
        //  m_fcolZones
        //      The flags that indicate which of the areas, zones, thermos,
        //      etc... the driver is to interface to. Zome are just booleans
        //      lists that indicate whether the item is configured or not.
        //      Some are lists of enums that indicate unused or the type of
        //      the item.
        // -------------------------------------------------------------------
        tHAIOmniSh::TFlagArray  m_fcolAreas;
        tHAIOmniSh::TPLCArray   m_fcolPLCUnits;
        tHAIOmniSh::TFlagArray  m_fcolThermos;
        tHAIOmniSh::TZoneArray  m_fcolZones;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TOmniDrvCfg,TObject)
};


#pragma CIDLIB_POPPACK

