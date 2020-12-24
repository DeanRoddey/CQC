//
// FILE NAME: HAIOmniTCP2Sh_DrvConfig.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/06/2014
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
//  This is the header for the HAIOmniTCP2Sh_DrvConfig.cpp file, which
//  implements the class that is used to store and pass around the Omni TCP
//  driver configuration data. The user has to tell us what zones, areas,
//  thermos, etc... they want us to handle. This has to be stored and
//  reloaded and passed between the server and client sides of the driver.
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
class HAIOMNITCP2SHEXP TOmniTCPDrvCfg : public TObject, public MStreamable
{
    public :
        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static tCIDLib::ESortComps eCompById
        (
            const   THAIOmniItem&           item1
            , const THAIOmniItem&           item2
        );


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TOmniTCPDrvCfg();

        TOmniTCPDrvCfg
        (
            const   TOmniTCPDrvCfg&         dcfgSrc
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
        tCIDLib::TCard4 c4AreaCnt() const;

        tCIDLib::TCard4 c4ThermoCnt() const;

        tCIDLib::TCard4 c4UnitCnt() const;

        tCIDLib::TCard4 c4ZoneCnt() const;

        tCIDLib::TBoolean bIsEmpty() const;

        tCIDLib::TBoolean bNameUsed
        (
            const   TString&                strToCheck
            , const tHAIOmniTCP2Sh::EItemTypes eType
        )   const;

        const THAIOmniArea& itemAreaAt
        (
            const   tCIDLib::TCard4         c4At
        )   const;

        THAIOmniArea& itemAreaAt
        (
            const   tCIDLib::TCard4         c4At
        );

        const THAIOmniThermo& itemThermoAt
        (
            const   tCIDLib::TCard4         c4At
        )   const;

        THAIOmniThermo& itemThermoAt
        (
            const   tCIDLib::TCard4         c4At
        );

        const THAIOmniUnit& itemUnitAt
        (
            const   tCIDLib::TCard4         c4At
        )   const;

        THAIOmniUnit& itemUnitAt
        (
            const   tCIDLib::TCard4         c4At
        );

        const THAIOmniZone& itemZoneAt
        (
            const   tCIDLib::TCard4         c4At
        )   const;

        THAIOmniZone& itemZoneAt
        (
            const   tCIDLib::TCard4         c4At
        );

        THAIOmniArea* pitemAddArea
        (
            const   THAIOmniArea&           itemToAdd
        );

        THAIOmniThermo* pitemAddThermo
        (
            const   THAIOmniThermo&         itemToAdd
        );

        THAIOmniUnit* pitemAddUnit
        (
            const   THAIOmniUnit&           itemToAdd
        );

        THAIOmniZone* pitemAddZone
        (
            const   THAIOmniZone&           itemToAdd
        );

        THAIOmniArea* pitemAreaById
        (
            const   tCIDLib::TCard2         c2Id
            ,       tCIDLib::TCard4&        c4At
        );

        THAIOmniArea* pitemFindArea
        (
            const   TString&                strFind
            , const tCIDLib::TBoolean       bThrowIfNot = kCIDLib::True
        );

        const THAIOmniArea* pitemFindArea
        (
            const   TString&                strFind
            , const tCIDLib::TBoolean       bThrowIfNot = kCIDLib::True
        )   const;

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

        THAIOmniItem* pitemById
        (
            const   tHAIOmniTCP2Sh::EItemTypes eType
            , const tCIDLib::TCard2         c2Id
            ,       tCIDLib::TCard4&        c4At
        );

        const THAIOmniItem* pitemById
        (
            const   tHAIOmniTCP2Sh::EItemTypes eType
            , const tCIDLib::TCard2         c2Id
            ,       tCIDLib::TCard4&        c4At
        )   const;

        THAIOmniThermo* pitemOwningThermo
        (
            const   tCIDLib::TCard4         c4FldId
        );

        THAIOmniUnit* pitemOwningUnit
        (
            const   tCIDLib::TCard4         c4FldId
        );

        THAIOmniThermo* pitemThermoById
        (
            const   tCIDLib::TCard2         c2Id
            ,       tCIDLib::TCard4&        c4At
        );

        THAIOmniUnit* pitemUnitById
        (
            const   tCIDLib::TCard2         c2Id
            ,       tCIDLib::TCard4&        c4At
        );

        THAIOmniZone* pitemZoneById
        (
            const   tCIDLib::TCard2         c2Id
            ,       tCIDLib::TCard4&        c4At
        );

        tCIDLib::TVoid RemoveItem
        (
            const   tHAIOmniTCP2Sh::EItemTypes eType
            , const tCIDLib::TCard2         c2Id
        );

        tCIDLib::TVoid Reset();

        tCIDLib::TVoid SetAllTriggers
        (
            const   tHAIOmniTCP2Sh::EItemTypes eType
            , const tCIDLib::TBoolean       bNewState
        );

        tCIDLib::TVoid SetAreaAt
        (
            const   THAIOmniArea&           itemToSet
            , const tCIDLib::TCard4         c4At
        );

        tCIDLib::TVoid SetThermoAt
        (
            const   THAIOmniThermo&         itemToSet
            , const tCIDLib::TCard4         c4At
        );

        tCIDLib::TVoid SetUnitAt
        (
            const   THAIOmniUnit&           itemToSet
            , const tCIDLib::TCard4         c4At
        );

        tCIDLib::TVoid SetZoneAt
        (
            const   THAIOmniZone&           itemToSet
            , const tCIDLib::TCard4         c4At
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected class types
        // -------------------------------------------------------------------
        typedef TRefVector<THAIOmniItem>    TItemList;


        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        )   override;

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const override;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid CheckDup
        (
            const   THAIOmniItem&           itemCheck
            , const TItemList&              colList
            , const tCIDLib::TCh* const     pszType
        )   const;

        const TItemList* pcolListForType
        (
            const   tHAIOmniTCP2Sh::EItemTypes eType
        )   const;

        TItemList* pcolListForType
        (
            const   tHAIOmniTCP2Sh::EItemTypes eType
        );

        const THAIOmniItem* pitemFindInList
        (
            const   TString&                strName
            , const TItemList&              colSrc
            , const tCIDLib::TCh* const     pszType
            , const tCIDLib::TBoolean       bThrowIfNot
        )   const;

        THAIOmniItem* pitemFindInList
        (
            const   TString&                strName
            ,       TItemList&              colSrc
            , const tCIDLib::TCh* const     pszType
            , const tCIDLib::TBoolean       bThrowIfNot
        );

        THAIOmniItem* pitemFldOwner
        (
            const   tCIDLib::TCard4         c4FldId
            ,       TItemList&              colSrc
        );

        tCIDLib::TVoid RepList
        (
            const   TItemList&              colSrc
            ,       TItemList&              colTar
        );



        // -------------------------------------------------------------------
        //  Private data members
        // -------------------------------------------------------------------
        TItemList   m_colAreas;
        TItemList   m_colThermos;
        TItemList   m_colUnits;
        TItemList   m_colZones;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TOmniTCPDrvCfg,TObject)
};

#pragma CIDLIB_POPPACK

