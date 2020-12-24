//
// FILE NAME: ZWaveUSBS_DrvConfig.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/20/2005
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
//  This is the header file for the ZWaveUSBS_DrvCOnfig.cpp file, which
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
class ZWUSBSHEXPORT TZWaveDrvConfig : public TObject, public MStreamable
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
        tCIDLib::TBoolean bAddGroup
        (
            const   TZWaveGroupInfo&        grpiToAdd
        );

        tCIDLib::TBoolean bAddUnit
        (
                    TZWaveUnit* const       punitToAdd
        );

        tCIDLib::TBoolean bIsEmpty() const;

        tCIDLib::TBoolean bGroupExists
        (
            const   TString&                strToCheck
        )   const;

        tCIDLib::TBoolean bUnitExists
        (
            const   TString&                strToCheck
        )   const;

        tCIDLib::TBoolean bValidate();

        const tZWaveUSBSh::TGroupList& colGroups() const;

        tZWaveUSBSh::TGroupList& colGroups();

        const tZWaveUSBSh::TUnitList& colUnits() const;

        tZWaveUSBSh::TUnitList& colUnits();

        tCIDLib::TVoid DeleteGroup
        (
            const   TString&                strGroupId
        );

        const TZWaveGroupInfo& grpiFindByName
        (
            const   TString&                strToFind
        )   const;

        TZWaveGroupInfo& grpiFindByName
        (
            const   TString&                strToFind
        );

        TZWaveGroupInfo* pgrpiFindByFldId
        (
            const   tCIDLib::TCard4         c4ToFind
        );

        TZWaveGroupInfo* pgrpiFindByName
        (
            const   TString&                strToFind
        );

        const TZWaveUnit* punitFindById
        (
            const   tCIDLib::TCard1         c1ToFind
        )   const;

        TZWaveUnit* punitFindById
        (
            const   tCIDLib::TCard1         c1ToFind
        );

        const TZWaveUnit* punitFindByName
        (
            const   TString&                strToCheck
        )   const;

        TZWaveUnit* punitFindByName
        (
            const   TString&                strToCheck
        );

        tCIDLib::TVoid Reset();

        tCIDLib::TVoid SortLists();

        const TZWaveUnit& unitFindById
        (
            const   tCIDLib::TCard1         c1ToFind
        )   const;

        const TZWaveUnit& unitFindByName
        (
            const   TString&                strToFind
        )   const;

        TZWaveUnit& unitFindByName
        (
            const   TString&                strToFind
        );

        tCIDLib::TVoid UpdateGroup
        (
            const   TZWaveGroupInfo&        grpiNew
            ,       TCQCServerBase* const   psdrvSrc
            , const tCIDLib::TBoolean       bNewGroup
        );

        tCIDLib::TVoid UpdateUnit
        (
                    TZWaveUnit* const       punitNew
            ,       TCQCServerBase* const   psdrvSrc
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
        tCIDLib::TVoid AddDefContent();

        const TZWaveGroupInfo* pgrpiFindByName
        (
            const   TString&                strToCheck
        )   const;

        const TZWaveGroupInfo* pgrpiFindById
        (
            const   TString&                strId
        )   const;

        TZWaveGroupInfo* pgrpiFindById
        (
            const   TString&                strId
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_colGroups
        //      The list of groups that we are configured for. We keep it
        //      sorted by name.
        //
        //  m_colUnits
        //      The list of units that we are configured for. We keep it sorted
        //      by unit id.
        // -------------------------------------------------------------------
        tZWaveUSBSh::TGroupList m_colGroups;
        tZWaveUSBSh::TUnitList  m_colUnits;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWaveUnit,TObject)
};

#pragma CIDLIB_POPPACK



