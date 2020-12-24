//
// FILE NAME: ZWaveLevi2Sh_DrvConfig.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/10/2014
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
//  This is the header file for the ZWaveLevi2Sh_DrvConfig.cpp file, which
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
class ZWLEVI2SHEXPORT TZWaveDrvConfig : public TObject, public MStreamable
{
    public :
        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static TZWDevInfo* pzwdiFind
        (
            const   TString&                strFileName
            ,       TXMLTreeParser* const   pxtprsToUse
        );


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
            const   tCIDLib::TCard4         c4Id
            , const tZWaveLevi2Sh::EGenTypes eGenType
        );

        tCIDLib::TBoolean bCheckDupName
        (
            const   TString&                strToCheck
        )   const;

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

        tCIDLib::TCard4 c4FindVRCOP
        (
            const   tCIDLib::TBoolean       bThrowIfNot
        )   const;

        tCIDLib::TCard4 c4UnitCnt() const;

        tCIDLib::TVoid DeleteGroup
        (
            const   TString&                strGrpName
        );

        tCIDLib::TVoid DeleteUnit
        (
            const   tCIDLib::TCard4         c4UnitId
        );

        tCIDLib::TVoid FindGroupByName
        (
            const   TString&                strName
            ,       tCIDLib::TCard4&        c4GrpId
        );

        tCIDLib::TVoid InitVals
        (
                    MZWLeviSrvFuncs&        mzwsfLevi
        );

        tCIDLib::TVoid LoadDevInfo
        (
                    MZWLeviSrvFuncs&        mzwsfLevi
            ,       TZWaveUnit&             unitTar
            ,       TXMLTreeParser&         xtprsToUse
        );

        tCIDLib::TVoid LoadDevInfo
        (
                    MZWLeviSrvFuncs&        mzwsfLevi
        );

        const TZWaveUnit* punitFindById
        (
            const   tCIDLib::TCard4         c4Id
        )   const;

        TZWaveUnit* punitFindById
        (
            const   tCIDLib::TCard4         c4Id
        );

        const TZWaveUnit* punitFindByName
        (
            const   TString&                strToCheck
        )   const;

        TZWaveUnit* punitFindByName
        (
            const   TString&                strToCheck
        );

        tCIDLib::TVoid RenameUnit
        (
            const   tCIDLib::TCard4         c4Id
            , const TString&                strNewName
            ,       MZWLeviSrvFuncs&        mzwsfLevi
            ,       tCIDLib::TCard4&        c4NewIndex
        );

        tCIDLib::TVoid Reset();

        tCIDLib::TVoid ResetUnitType
        (
            const   tCIDLib::TCard4         c4Id
            , const tZWaveLevi2Sh::EGenTypes eType
        );

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
            const   TZWaveUnit&             unitNew
            ,       MZWLeviSrvFuncs&        mzwsfLevi
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
        TObjArray<TString>          m_colGroups;
        tZWaveLevi2Sh::TUnitList    m_colUnits;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWaveUnit,TObject)
};

#pragma CIDLIB_POPPACK



