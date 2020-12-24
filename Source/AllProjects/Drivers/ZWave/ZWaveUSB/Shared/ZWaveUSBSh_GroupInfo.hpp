//
// FILE NAME: ZWaveUSBSh_GroupInfo.hpp
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
//  This is the header file for the ZWaveUSBS_GroupInfo.cpp file, which
//  implements the TZWaveGroupInfo class. This class represents the
//  information, only static (persisted) in the case of groups, for a Z-Wave
//  unit that we are controlling.
//
//  A group 'contains' units, i.e. there are units that belong to groups.
//  We keep that association by just storing a list of the ids of the units
//  in each group.
//
//  Note that we ignore the group names from the remote control, and just
//  case about group ids. We let the user create his/her own group names
//  for CQC use (and they cannot have spaces since they are used to generate
//  field names.)
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TZWaveGroupInfo
//  PREFIX: grpi
// ---------------------------------------------------------------------------
class ZWUSBSHEXPORT TZWaveGroupInfo : public TObject, public MStreamable
{
    public :
        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static tCIDLib::ESortComps eCompByName
        (
            const   TZWaveGroupInfo&        grpi1
            , const TZWaveGroupInfo&        grpi2
            , const tCIDLib::TCard4         c4UserData
        );


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TZWaveGroupInfo();

        TZWaveGroupInfo
        (
            const   TString&                strName
        );

        TZWaveGroupInfo
        (
            const   TZWaveGroupInfo&        grpiToCopy
        );

        ~TZWaveGroupInfo();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TZWaveGroupInfo& operator=
        (
            const   TZWaveGroupInfo&        grpiSrc
        );

        tCIDLib::TBoolean operator==
        (
            const   TZWaveGroupInfo&        grpiSrc
        )   const;

        tCIDLib::TBoolean operator!=
        (
            const   TZWaveGroupInfo&        grpiSrc
        )   const;


        // -------------------------------------------------------------------
        //  Public , non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid AddUnit
        (
            const   tCIDLib::TCard1         c1Id
        );

        tCIDLib::TBoolean bContainsUnitId
        (
            const   tCIDLib::TCard1         c1Id
        )   const;

        tCIDLib::TCard4 c4FldId() const;

        tCIDLib::TCard4 c4FldId
        (
            const   tCIDLib::TCard4         c4ToSet
        );

        tCIDLib::TCard4 c4UnitCount() const;

        tCIDLib::TVoid ClearUnitList();

        const tZWaveUSBSh::TIdList fcolUnits() const;

        const TString& strId() const;

        const TString& strName() const;

        const TString& strName
        (
            const   TString&                strToSet
        );


    protected :
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
        //  Private data members
        //
        //  m_c4FldId
        //      The field id of the field that is generated for this group
        //      is stored here, so that we can map back from field id to
        //      group id for outgoing write operations. This is runtime only
        //      and is not persisted.
        //
        //  m_fcolUnits
        //      The list of units that are part of this group. It will never
        //      be empty since we don't bother storing any groups that have
        //      no units in them.
        //
        //  m_strId
        //      The name of the group can change, so we need some way of
        //      uniquely identifying each group. We just generate a unique id
        //      for each one.
        //
        //  m_strName
        //      The user settable name of this group, e.g. "KitchenLights" and
        //      so forth.
        // -------------------------------------------------------------------
        tCIDLib::TCard4         m_c4FldId;
        tZWaveUSBSh::TIdList    m_fcolUnits;
        TString                 m_strId;
        TString                 m_strName;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWaveGroupInfo,TObject)
};

#pragma CIDLIB_POPPACK


