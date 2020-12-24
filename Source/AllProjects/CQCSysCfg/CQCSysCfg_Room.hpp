//
// FILE NAME: CQCSysCfg_Room.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/03/2014
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
//  This file defines the configuration for a room. Each room is used to drive the
//  generation of user logic and templates for that room.
//
//  Mostly it's
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TCQCSysCfgRmInfo
//  PREFIX: scri
// ---------------------------------------------------------------------------
class CQCSYSCFGEXP TCQCSysCfgRmInfo : public TObject, public MStreamable
{
    public :
        TCQCSysCfgRmInfo();

        TCQCSysCfgRmInfo
        (
            const   TString&                strName
        );

        TCQCSysCfgRmInfo(const TCQCSysCfgRmInfo&) = default;
        TCQCSysCfgRmInfo(TCQCSysCfgRmInfo&&) = default;

        ~TCQCSysCfgRmInfo();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCSysCfgRmInfo& operator=(const TCQCSysCfgRmInfo&) = default;
        TCQCSysCfgRmInfo& operator=(TCQCSysCfgRmInfo&&) = default;

        tCIDLib::TBoolean operator==
        (
            const   TCQCSysCfgRmInfo&       scriSrc
        )   const;

        tCIDLib::TBoolean operator!=
        (
            const   TCQCSysCfgRmInfo&       scriSrc
        )   const;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid AddHVAC
        (
            const   TString&                strUID
            , const tCIDLib::TBoolean       bNotIfEmpty = kCIDLib::False
        );

        tCIDLib::TVoid AddLoad
        (
            const   TString&                strUID
            , const tCIDLib::TBoolean       bNotIfEmpty = kCIDLib::False
        );

        tCIDLib::TVoid AddXOver
        (
            const   TString&                strUID
            , const tCIDLib::TBoolean       bNotIfEmpty = kCIDLib::False
        );

        tCIDLib::TBoolean bAddOrUpdateRoomMode
        (
            const   TSCGlobActInfo&         scliSrc
        );

        [[nodiscard]] tCIDLib::TBoolean bAllowRmSel() const;

        tCIDLib::TBoolean bAllowRmSel
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TBoolean bEnabled() const;

        tCIDLib::TBoolean bEnabled
        (
            const   tCIDLib::TBoolean       bToSet
        );

        [[nodiscard]] tCIDLib::TBoolean bFuncEnabled
        (
            const   tCQCSysCfg::ERmFuncs    eFunc
        )   const;

        [[nodiscard]] tCIDLib::TBoolean bGlobalActEnabled
        (
            const   tCQCSysCfg::EGlobActs   eActIndex
        )   const;

        [[nodiscard]] tCIDLib::TCard4 c4HVACCount() const;

        [[nodiscard]] tCIDLib::TCard4 c4LoadCount() const;

        [[nodiscard]] tCIDLib::TCard4 c4RoomModeCount() const;

        tCIDLib::TCard4 c4RoomModeById
        (
            const   TString&                strUID
        )   const;

        [[nodiscard]] tCIDLib::TCard4 c4SecZoneCount() const;

        [[nodiscard]] tCIDLib::TCard4 c4XOverCount() const;

        tCIDLib::TVoid CopyFrom
        (
            const   TCQCSysCfgRmInfo&       scriSrc
        );

        tCIDLib::TVoid RemoveFromList
        (
            const   tCQCSysCfg::ECfgLists   eList
            , const TString&                strUID
        );

        tCIDLib::TVoid RemoveRoomModeAt
        (
            const   tCIDLib::TCard4         c4At
        );

        tCIDLib::TVoid Reset();

        tCIDLib::TVoid ResetGlobalAct
        (
            const   tCQCSysCfg::EGlobActs   eActIndex
        );

        [[nodiscard]] const TSCGlobActInfo& scliGlobActAt
        (
            const   tCQCSysCfg::EGlobActs   eActIndex
        )   const;

        [[nodiscard]] TSCGlobActInfo& scliGlobActAt
        (
            const   tCQCSysCfg::EGlobActs   eActIndex
        );

        [[nodiscard]] const TSCGlobActInfo& scliRoomModeAt
        (
            const   tCIDLib::TCard4         c4Index
        )   const;

        [[nodiscard]] TSCGlobActInfo& scliRoomModeAt
        (
            const   tCIDLib::TCard4         c4Index
        );

        [[nodiscard]] const TCQCSysCfgVoice& scvcVoice() const;

        [[nodiscard]] TCQCSysCfgVoice& scvcVoice();

        [[nodiscard]] const TString& strFuncId
        (
            const   tCQCSysCfg::ERmFuncs    eFunc
        )   const;

        [[nodiscard]] const TString& strHVACUIDAt
        (
            const   tCIDLib::TCard4         c4At
        )   const;

        [[nodiscard]] const TString& strLoadUIDAt
        (
            const   tCIDLib::TCard4         c4At
        )   const;

        [[nodiscard]] const TString& strName() const;

        const TString& strName
        (
            const   TString&                strToSet
        );

        [[nodiscard]] const TString& strRoomVarAt
        (
            const   tCIDLib::TCard4         c4At
        )   const;

        [[nodiscard]] const TString& strZoneAltNameAt
        (
            const   tCIDLib::TCard4         c4At
        )   const;

        [[nodiscard]] const TString& strZoneNameAt
        (
            const   tCIDLib::TCard4         c4At
        )   const;

        [[nodiscard]] const TString& strZoneNameToAltName
        (
            const   TString&                strToFind
        )   const;

        [[nodiscard]] const TString& strUniqueId() const;

        [[nodiscard]] const TString& strXOverUIDAt
        (
            const   tCIDLib::TCard4         c4At
        )   const;

        tCIDLib::TVoid SetFuncId
        (
            const   tCQCSysCfg::ERmFuncs    eFunc
            , const TString&                strToSet
        );

        tCIDLib::TVoid SetHVACs
        (
            const   tCIDLib::TStrCollect&   colToSet
        );

        tCIDLib::TVoid SetLoads
        (
            const   tCIDLib::TStrCollect&   colToSet
        );

        tCIDLib::TVoid SetRoomVar
        (
            const   tCIDLib::TCard4         c4Index
            , const TString&                strToSet
        );

        tCIDLib::TVoid SetSecZones
        (
            const   tCIDLib::TKVPCollect&   colToSet
        );

        tCIDLib::TVoid SetZoneAltNameAt
        (
            const   tCIDLib::TCard4         c4At
            , const TString&                strToSet
        );

        tCIDLib::TVoid SetVoiceCfg
        (
            const   TCQCSysCfgVoice&        scvcVoice
        );

        tCIDLib::TVoid SetXOvers
        (
            const   tCIDLib::TStrCollect&   colToSet
        );

        tCIDLib::TVoid ToJSON
        (
            const   TCQCSysCfg&             scfgInfo
            ,       TTextOutStream&         strmOut
            , const tCIDLib::TBoolean       bAppend
        )   const;

        tCIDLib::TVoid ToXML
        (
            const   TCQCSysCfg&             scfgInfo
            ,       TTextOutStream&         strmOut
            , const tCIDLib::TBoolean       bAppend
        )   const;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        )   final;

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const final;


    private :
        // -------------------------------------------------------------------
        //  Private data types
        // -------------------------------------------------------------------
        using TFuncIds = TObjArray<TString, tCQCSysCfg::ERmFuncs>;


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bAllowRmSel
        //      Controls whether this room allows the user to select other rooms
        //      via a button on the main screen or not. If false, then it only
        //      allows access to that room itself.
        //
        //  m_bEnabled
        //      Rooms can be temporarily disabled so that they will not be
        //      used in solution generation. It can still be edited while
        //      disabled.
        //
        //  m_colRoomVars
        //      We set up four standard global variables, and each room can set them to
        //      different values as desired, to drive functionality in their custom stuff.
        //
        //  m_colSecZones
        //      The security zones names for this room. Each one represents some sensor,
        //      door, window, etc... They are all from the security driver with which this
        //      room is associated (in m_objaFuncIds.)
        //
        //      We store the name from the driver, plus an 'alt name' that is used by
        //      voice control.
        //
        //  m_colSelHVACs
        //  m_colSelLoads
        //      These are the list of selected loads and HVACs this room can control.
        //
        //  m_colXOvers
        //      A list of UIDs of configured extra overlays that this room should provide
        //      access to.
        //
        //  m_objaFuncIds
        //      We can store a value for each functional area. They have the following
        //      meanings:
        //
        //          Lights, HVAC -  The id of the default one for the room. In these
        //                          cases they can select multiples that can be controlled
        //                          from this room, and this is the default one that is
        //                          actually in this room.
        //
        //          Music, Movies,- The UID of the configured entry (from the global list)
        //          Security,       that this room uses. If empty, that functionality is
        //          Weather         not enabled for this room.
        //
        //      It is indexed by the room functions enum.
        //
        //  m_objaGlobActs
        //      The list of potential global actions we can support. It is indexed by
        //      the global actions enum.
        //
        //  m_colRoomModes
        //      A list of global actions (same class as the global actions above) that
        //      the user can add to. Each one represents a 'room mode' that the user can
        //      set, which just makes us run the action as configured.
        //
        //  m_scvcVoice
        //      Voice related configuration for this room.
        //
        //  m_strName
        //      The unique name of this room. This has to be something that can act as
        //      a prefix for template names, or other resources we might create. so we
        //      limit it to alpha-numeric and space, no punctuation.
        //
        //  m_strUniqueId
        //      A generated unique id for this room, so that the name can be changed
        //      without breaking any references, mostly during configuration in the
        //      dialogs.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean           m_bAllowRmSel;
        tCIDLib::TBoolean           m_bEnabled;
        tCQCSysCfg::TRoomModeList   m_colRoomModes;
        tCIDLib::TKVPList           m_colSecZones;
        tCIDLib::TStrList           m_colSelHVACs;
        tCIDLib::TStrList           m_colSelLoads;
        tCIDLib::TStrList           m_colXOvers;
        TFuncIds                    m_objaFuncIds;
        tCQCSysCfg::TGlobActList    m_objaGlobalActs;
        tCIDLib::TStrArray          m_objaRoomVars;
        TCQCSysCfgVoice             m_scvcVoice;
        TString                     m_strName;
        TString                     m_strUniqueId;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCSysCfgRmInfo,TObject)
};

#pragma CIDLIB_POPPACK

