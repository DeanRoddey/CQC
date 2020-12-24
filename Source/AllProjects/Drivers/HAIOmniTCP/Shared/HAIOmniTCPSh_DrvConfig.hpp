//
// FILE NAME: HAIOmniTCPSh_DrvConfig.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 05/16/2008
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
//  This is the header for the HAIOmniTCPSh_DrvConfig.cpp file, which
//  implements the class that is used to store and pass around the Omni TCP
//  driver configuration data. The user has to tell us what zones, areas,
//  thermos, etc... they want us to handle. This has to be stored and
//  reloaded and passed between the server and client sides of the driver.
//
//  Note that we had an older configuration that was very simple and just
//  stored some basic flags for each thing. As of CQC 4.2.917 this was
//  replaced. We moved the item classes over from the server and now we
//  use that to store the configuration. This makes it possible to allow for
//  much more configuration data per item easily. In the process we
//  significantly extended the configuration options.
//
//  All the old config did was to read in some arrays, so we just use the
//  config format version to know which we are reading in.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: TOmniTCPDrvCfg
//  PREFIX: dcfg
// ---------------------------------------------------------------------------
class HAIOMNITCPSHEXPORT TOmniTCPDrvCfg : public TObject, public MStreamable
{
    public :
        // -------------------------------------------------------------------
        //  Destructor
        // -------------------------------------------------------------------
        TOmniTCPDrvCfg();

        TOmniTCPDrvCfg
        (
            const   TOmniTCPDrvCfg&         dcfgToCopy
        );

        ~TOmniTCPDrvCfg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TOmniTCPDrvCfg& operator=
        (
            const   TOmniTCPDrvCfg&         dcfgSrc
        );

        tCIDLib::TBoolean operator==
        (
            const   TOmniTCPDrvCfg&         dcfgSrc
        )   const;

        tCIDLib::TBoolean operator!=
        (
            const   TOmniTCPDrvCfg&         dcfgSrc
        )   const;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bNameUsed
        (
            const   TString&                strToCheck
        )   const;

        THAIOmniArea* pitemAreaAt
        (
            const   tCIDLib::TCard4         c4At
        );

        THAIOmniArea* pitemFindArea
        (
            const   TString&                strFind
            , const tCIDLib::TBoolean       bThrowIfNot = kCIDLib::True
        );

        THAIOmniThermo* pitemFindThermo
        (
            const   TString&                strFind
            , const tCIDLib::TBoolean       bThrowIfNot = kCIDLib::True
        );

        THAIOmniUnit* pitemFindUnit
        (
            const   TString&                strFind
            , const tCIDLib::TBoolean       bThrowIfNot = kCIDLib::True
        );

        THAIOmniZone* pitemFindZone
        (
            const   TString&                strFind
            , const tCIDLib::TBoolean       bThrowIfNot = kCIDLib::True
        );

        THAIOmniThermo* pitemOwningThermo
        (
            const   tCIDLib::TCard4         c4FldId
        )   const;

        THAIOmniUnit* pitemOwningUnit
        (
            const   tCIDLib::TCard4         c4FldId
        )   const;

        THAIOmniThermo* pitemThermoAt
        (
            const   tCIDLib::TCard4         c4At
        );

        THAIOmniUnit* pitemUnitAt
        (
            const   tCIDLib::TCard4         c4At
        );

        THAIOmniZone* pitemZoneAt
        (
            const   tCIDLib::TCard4         c4At
        );

        tCIDLib::TVoid RemoveAt
        (
            const   tHAIOmniTCPSh::EItemTypes eType
            , const tCIDLib::TCard4         c4At
        );

        tCIDLib::TVoid Reset();

        tCIDLib::TVoid SetAllTriggers
        (
            const   tHAIOmniTCPSh::EItemTypes eType
            , const tCIDLib::TBoolean       bNewState
        );

        tCIDLib::TVoid SetAreaAt
        (
            const   THAIOmniArea&           itemSrc
            , const tCIDLib::TCard4         c4At
        );

        tCIDLib::TVoid SetThermoAt
        (
            const   THAIOmniThermo&         itemSrc
            , const tCIDLib::TCard4         c4At
        );

        tCIDLib::TVoid SetUnitAt
        (
            const   THAIOmniUnit&           itemSrc
            , const tCIDLib::TCard4         c4At
        );

        tCIDLib::TVoid SetZoneAt
        (
            const   THAIOmniZone&           itemSrc
            , const tCIDLib::TCard4         c4At
        );


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
        // -------------------------------------------------------------------
        tCIDLib::TVoid CheckIndex
        (
            const   tCIDLib::TCh* const     pszType
            , const tCIDLib::TCard4         c4At
            , const tCIDLib::TCard4         c4Max
        );

        tCIDLib::TVoid DupSrc
        (
            const   TOmniTCPDrvCfg&         dcfgSrc
            , const tCIDLib::TBoolean       bInit
        );

        tCIDLib::TVoid FreeAll();

        tCIDLib::TVoid InitAll();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_apAreas
        //  m_apThermos
        //  m_apUnits
        //  m_apZones
        //      We keep simple arrays of each type. Since there's a well
        //      defined max for each and we want to allow for quick access by
        //      item number (minus one for an idex), this works out easily
        //      enough.
        // -------------------------------------------------------------------
        THAIOmniArea*   m_apitemAreas[kHAIOmniTCPSh::c4MaxAreas];
        THAIOmniThermo* m_apitemThermos[kHAIOmniTCPSh::c4MaxThermos];
        THAIOmniUnit*   m_apitemUnits[kHAIOmniTCPSh::c4MaxUnits];
        THAIOmniZone*   m_apitemZones[kHAIOmniTCPSh::c4MaxZones];


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TOmniTCPDrvCfg,TObject)
};


#pragma CIDLIB_POPPACK

