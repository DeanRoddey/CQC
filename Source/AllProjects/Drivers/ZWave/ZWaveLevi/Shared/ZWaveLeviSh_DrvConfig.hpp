//
// FILE NAME: ZWaveLeviSh_DrvConfig.hpp
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
//  This is the header file for the ZWaveLeviSh_DrvConfig.cpp file, which
//  implements the TZWaveDrvConfig class. This is a class that holds all our
//  persistent configuration info, groups, units, etc...
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TZWaveDrvConfig
//  PREFIX: dcfg
// ---------------------------------------------------------------------------
class ZWLEVISHEXPORT TZWaveDrvConfig : public TObject, public MStreamable
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TZWaveDrvConfig();

        TZWaveDrvConfig
        (
            const   TZWaveDrvConfig&        dcfgToCopy
        );

        ~TZWaveDrvConfig();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TZWaveDrvConfig& operator=
        (
            const   TZWaveDrvConfig&        dcfgSrc
        );

        tCIDLib::TBoolean operator==
        (
            const   TZWaveDrvConfig&        dcfgSrc
        )   const;

        tCIDLib::TBoolean operator!=
        (
            const   TZWaveDrvConfig&        dcfgSrc
        )   const;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bAddUnit
        (
                    TZWaveUnit* const       punitToAdd
        );

        tCIDLib::TBoolean bIsEmpty() const;

        tCIDLib::TBoolean bGroupExists
        (
            const   TString&                strToCheck
            ,       tCIDLib::TCard4&        c4GroupNum
        )   const;

        tCIDLib::TBoolean bUnitExists
        (
            const   TString&                strToCheck
        )   const;

        tCIDLib::TBoolean bValidate();

        tCIDLib::TCard4 c4QueryDuePollList
        (
                    tZWaveLeviSh::TUnitList& colToFill
            ,       TCQCServerBase&         sdrvLevi
        );

        tCIDLib::TCard4 c4UnitCnt() const;

        tCIDLib::TVoid DeleteGroup
        (
            const   TString&                strGrpName
        );

        tCIDLib::TVoid DeleteUnit
        (
                    TZWaveUnit* const       punitDel
        );

        tCIDLib::TVoid FindGroupByName
        (
            const   TString&                strName
            ,       tCIDLib::TCard4&        c4GrpId
        );

        const TZWaveUnit* punitFindById
        (
            const   tCIDLib::TCard4         c4ToFind
        )   const;

        TZWaveUnit* punitFindById
        (
            const   tCIDLib::TCard4         c4ToFind
        );

        const TZWaveUnit* punitFindByName
        (
            const   TString&                strToCheck
        )   const;

        TZWaveUnit* punitFindByName
        (
            const   TString&                strToCheck
        );

        tCIDLib::TVoid PrepUnitsForUse();

        tCIDLib::TVoid Reset();

        const TString& strGroupAt
        (
            const   tCIDLib::TCard4         c4At
        )   const;

        tCIDLib::TVoid SetGroupById
        (
            const   tCIDLib::TCard4         c4TarGrpId
            , const TString&                strName
        );

        tCIDLib::TVoid SetGroups
        (
            const   tCIDLib::TStrCollect&   colToSet
        );

        tCIDLib::TVoid SortUnitList();

        TZWaveUnit& unitAt
        (
            const   tCIDLib::TCard4         c4At
        );

        const TZWaveUnit& unitAt
        (
            const   tCIDLib::TCard4         c4At
        )   const;

        const TZWaveUnit& unitFindById
        (
            const   tCIDLib::TCard4         c4ToFind
        )   const;

        TZWaveUnit& unitFindById
        (
            const   tCIDLib::TCard4         c4ToFind
        );

        const TZWaveUnit& unitFindByName
        (
            const   TString&                strToFind
        )   const;

        TZWaveUnit& unitFindByName
        (
            const   TString&                strToFind
        );

        tCIDLib::TVoid UpdateUnit
        (
                    TZWaveUnit* const       punitNew
            ,       TCQCServerBase&         sdrvLevi
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
        //  Private data members
        //
        //  m_colGroups
        //      For each possible group id, we can store a name so that the
        //      user can refer to them by name and we map that to the actual
        //      group id.
        //
        //  m_colUnits
        //      The list of units that we are configured for. We keep it sorted
        //      by unit id.
        // -------------------------------------------------------------------
        TObjArray<TString>      m_colGroups;
        tZWaveLeviSh::TUnitList m_colUnits;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWaveUnit,TObject)
};

#pragma CIDLIB_POPPACK



