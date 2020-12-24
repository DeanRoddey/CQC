//
// FILE NAME: CQCSysCfg_Layout.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/03/2013
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
//  This is the overall system configuration layout. The big bit is the list of
//  rooms, but we also have lists of other things, which the rooms reference, via
//  a unique id assigned to each list element. The rooms have unique ids as well,
//  so the names of all these bits and pieces can change without breaking any
//  references.
//
// CAVEATS/GOTCHAS:
//
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

class TCQCSysCfgSrvImpl;

// ---------------------------------------------------------------------------
//   CLASS: TCQCSysCfg
//  PREFIX: scfg
// ---------------------------------------------------------------------------
class CQCSYSCFGEXP TCQCSysCfg : public TObject, public MStreamable
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCSysCfg();

        TCQCSysCfg
        (
            const   TCQCSysCfg&             scfgSrc
        );

        TCQCSysCfg(TCQCSysCfg&&) = delete;

        ~TCQCSysCfg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCSysCfg& operator=
        (
            const   TCQCSysCfg&             scfgSrc
        );

        TCQCSysCfg& operator=(TCQCSysCfg&&) = delete;

        tCIDLib::TBoolean operator==
        (
            const   TCQCSysCfg&             scfgSrc
        )   const;

        tCIDLib::TBoolean operator!=
        (
            const   TCQCSysCfg&             scfgSrc
        )   const;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid AddToRooms
        (
            const   tCQCSysCfg::ECfgLists   eList
            , const TString&                strUID
            , const tCIDLib::TBoolean       bToAll
        );

        tCIDLib::TBoolean bAddOrUpdateHVAC
        (
            const   TSCHVACInfo&            scriToAdd
            ,       tCIDLib::TCard4&        c4Index
        );

        tCIDLib::TBoolean bAddOrUpdateLoad
        (
            const   TSCLoadInfo&            scliToAdd
            ,       tCIDLib::TCard4&        c4Index
        );

        tCIDLib::TBoolean bAddOrUpdateMedia
        (
            const   TSCMediaInfo&           scliToAdd
            ,       tCIDLib::TCard4&        c4Index
        );

        tCIDLib::TBoolean bAddOrUpdateRoom
        (
            const   TCQCSysCfgRmInfo&       scriToAdd
            ,       tCIDLib::TCard4&        c4Index
        );

        tCIDLib::TBoolean bAddOrUpdateSec
        (
            const   TSCSecInfo&             scliToAdd
            ,       tCIDLib::TCard4&        c4Index
        );

        tCIDLib::TBoolean bAddOrUpdateWeather
        (
            const   TSCWeatherInfo&         scliToAdd
            ,       tCIDLib::TCard4&        c4Index
        );

        tCIDLib::TBoolean bAddOrUpdateXOver
        (
            const   TSCXOverInfo&           scliToAdd
            ,       tCIDLib::TCard4&        c4Index
        );

        tCIDLib::TBoolean bAddRes
        (
            const   TSize&                  szToAdd
            ,       tCIDLib::TCard4&        c4Index
        );

        tCIDLib::TBoolean bFindRoomByUID
        (
            const   TString&                strUID
            ,       tCIDLib::TCard4&        c4RoomInd
            , const tCIDLib::TBoolean       bThrowIfNot
        )   const;

        tCIDLib::TBoolean bListAltNameExists
        (
            const   tCQCSysCfg::ECfgLists   eList
            , const TString&                strToCheck
        )   const;

        tCIDLib::TBoolean bListIDExists
        (
            const   tCQCSysCfg::ECfgLists   eList
            , const TString&                strToCheck
        )   const;

        tCIDLib::TBoolean bListTitleExists
        (
            const   tCQCSysCfg::ECfgLists   eList
            , const TString&                strToCheck
        )   const;

        tCIDLib::TBoolean bIsUniqueName
        (
            const   TString&                strToCheck
        )   const;

        tCIDLib::TBoolean bResExists
        (
            const   TSize&                  szToCheck
        )   const;

        tCIDLib::TBoolean bRoomIDExists
        (
            const   tCQCSysCfg::ERmFuncs    eRmFunc
            , const TCQCSysCfgRmInfo&       scriSrc
        )   const;

        tCIDLib::TBoolean bUseCustLights() const;

        tCIDLib::TBoolean bUseCustLights
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TCard4 c4AddRoom
        (
            const   TString&                strName
        );

        tCIDLib::TCard4 c4FindRoomByName
        (
            const   TString&                strName
        )   const;

        tCIDLib::TCard4 c4ListIdCount
        (
            const   tCQCSysCfg::ECfgLists   eList
        )   const;

        tCIDLib::TCard4 c4QueryCfgList
        (
            const   tCQCSysCfg::ECfgLists   eList
            ,       tCIDLib::TKVPCollect&   colToFill
        );

        tCIDLib::TCard4 c4ResCount() const;

        tCIDLib::TCard4 c4RoomCount() const;

        tCIDLib::TVoid DeleteListId
        (
            const   tCQCSysCfg::ECfgLists   eList
            , const TString&                strUID
        );

        tCIDLib::TVoid DeleteRes
        (
            const   TSize&                  szToDel
        );

        tCIDLib::TVoid DeleteRoom
        (
            const   TString&                strUID
        );


        const TSCHVACInfo& scliHVACAt
        (
            const   tCIDLib::TCard4         c4At
        );

        const TSCHVACInfo& scliHVACByUID
        (
            const   TString&                strUID
            ,       tCIDLib::TCard4&        c4Index
        )   const;

        const TSCLoadInfo& scliLoadAt
        (
            const   tCIDLib::TCard4         c4At
        )   const;

        const TSCLoadInfo& scliLoadByUID
        (
            const   TString&                strUID
            ,       tCIDLib::TCard4&        c4Index
        )   const;

        const TSCMediaInfo& scliMediaAt
        (
            const   tCIDLib::TCard4         c4At
        );

        const TSCMediaInfo& scliMediaByUID
        (
            const   TString&                strUID
            ,       tCIDLib::TCard4&        c4Index
        )   const;

        const TSCSecInfo& scliSecAt
        (
            const   tCIDLib::TCard4         c4At
        );

        const TSCSecInfo& scliSecByUID
        (
            const   TString&                strUID
            ,       tCIDLib::TCard4&        c4Index
        )   const;

        const TSCWeatherInfo& scliWeatherAt
        (
            const   tCIDLib::TCard4         c4At
        );

        const TSCWeatherInfo& scliWeatherByUID
        (
            const   TString&                strUID
            ,       tCIDLib::TCard4&        c4Index
        )   const;

        const TSCXOverInfo& scliXOverAt
        (
            const   tCIDLib::TCard4         c4At
        );

        const TSCXOverInfo& scliXOverByUID
        (
            const   TString&                strUID
            ,       tCIDLib::TCard4&        c4Index
        )   const;

        const TCQCSysCfgRmInfo& scriRoomAt
        (
            const   tCIDLib::TCard4         c4At
        )   const;

        TCQCSysCfgRmInfo& scriRoomAt
        (
            const   tCIDLib::TCard4         c4At
        );

        const TCQCSysCfgRmInfo& scriRoomByName
        (
            const   TString&                strName
            ,       tCIDLib::TCard4&        c4Index
        )   const;

        TCQCSysCfgRmInfo& scriRoomByName
        (
            const   TString&                strName
            ,       tCIDLib::TCard4&        c4Index
        );

        const TCQCSysCfgRmInfo& scriRoomByUID
        (
            const   TString&                strUID
            ,       tCIDLib::TCard4&        c4Index
        )   const;

        TCQCSysCfgRmInfo& scriRoomByUID
        (
            const   TString&                strUID
            ,       tCIDLib::TCard4&        c4Index
        );

        const TString& strCustLayout() const;

        const TString& strCustLayout
        (
            const   TString&                strToSet
        );

        const TString& strRoomFuncTitle
        (
            const   tCQCSysCfg::ERmFuncs    eFunc
            , const TCQCSysCfgRmInfo&       scriSrc
        )   const;

        const TString& strTarScope() const;

        const TString& strTarScope
        (
            const   TString&                strToSet
        );

        const TSize& szResAt
        (
            const   tCIDLib::TCard4         c4At
        )   const;

        tCIDLib::TVoid ReplaceRes
        (
            const   TSize&                  szOld
            , const TSize&                  szNew
        );

        tCIDLib::TVoid SortLists();

        tCIDLib::TVoid ToJSON
        (
                    TTextOutStream&         strmTar
            , const tCIDLib::TBoolean       bInclRooms
            , const tCIDLib::TBoolean       bAppend
        );

        tCIDLib::TVoid ToXML
        (
                    TTextOutStream&         strmTar
            , const TString&                strEncoding
            , const tCIDLib::TBoolean       bAppend
        )   const;


    protected :
        // -------------------------------------------------------------------
        //  Let the data server side implementation of our data interface access the
        //  serial number stuff
        // -------------------------------------------------------------------
        friend class TCQCSysCfgSrvImpl;


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


        // -------------------------------------------------------------------
        //  Protected, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TCard4 c4SerialNum() const;

        tCIDLib::TCard4 c4SerialNum
        (
            const   tCIDLib::TCard4         c4ToSet
        );


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TCard4 c4AddOrUpdate
        (
            const   TSysCfgListItem&        scliSrc
            ,       tCQCSysCfg::TItemList&  colTar
        );

        tCIDLib::TCard4 c4ListLookup
        (
            const   TString&                strUID
            , const tCQCSysCfg::TItemList&  colSrc
            , const tCIDLib::TBoolean       bThrowIfNot
            , const tCIDLib::TCh* const     pszType
        )   const;

        tCIDLib::TCard4 c4RoomLookup
        (
            const   TString&                strToFind
            , const tCIDLib::TBoolean       bByUID
            , const tCIDLib::TBoolean       bThrowIfNot
        )   const;

        tCIDLib::TVoid CopyList
        (
            const   tCQCSysCfg::TItemList&  colSrc
            ,       tCQCSysCfg::TItemList&  colTar
        );

        tCIDLib::TVoid DelListItem
        (
            const   TString&                strUID
            ,       tCQCSysCfg::TItemList&  colTar
        );

        tCIDLib::TVoid StreamInList
        (
                    tCQCSysCfg::TItemList&  colTar
            ,       TBinInStream&           strmSRc
        )   const;

        tCIDLib::TVoid StreamOutList
        (
            const   tCQCSysCfg::TItemList&  colSrc
            ,       TBinOutStream&          strmTar
        )   const;

        tCIDLib::TVoid RemoveListRefs
        (
            const   TString&                strUID
            , const tCQCSysCfg::ECfgLists   eList
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bUseCustLights
        //      If there is a custom layout provided, it's always used for security (if
        //      security is enabled) but it can optionally be used instead of the
        //      standard lighting template as well.
        //
        //  m_c4SerialNum
        //      The current serial number, which the server bumps any time the data
        //      is updated. This is not included in object equality, it's just to store
        //      the persistent serial number. And clients shouldn't use it as their
        //      serial number, keep a separate one.
        //
        //  m_colLoadList
        //      Master list of all the loads that the user configures to be available
        //      to the rooms.
        //
        //  m_colMediaList
        //      Master list of all the repo/renderer combos that the user configures
        //      to be available to the rooms.
        //
        //  m_colResList
        //      A list of output resolutions that we generate.
        //
        //  m_colRoomList
        //      The list of defined rooms.
        //
        //  m_colSecList
        //      Master list of all the security devices that the user configures to
        //      be available to the rooms.
        //
        //  m_strCustLayout
        //      They can provide a custom layout that we will invoke from one or more
        //      places.
        //
        //  m_strTarScope
        //      The target scope into which the user indicated he wants the content to be
        //      generated. We insure it doesn't end with a slash.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bUseCustLights;
        tCIDLib::TCard4         m_c4SerialNum;
        tCQCSysCfg::TItemList   m_colHVACList;
        tCQCSysCfg::TItemList   m_colLoadList;
        tCQCSysCfg::TItemList   m_colMediaList;
        tCQCSysCfg::TResList    m_colResList;
        tCQCSysCfg::TRmInfoList m_colRoomList;
        tCQCSysCfg::TItemList   m_colSecList;
        tCQCSysCfg::TItemList   m_colWeatherList;
        tCQCSysCfg::TItemList   m_colXOverList;
        TString                 m_strCustLayout;
        TString                 m_strTarScope;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCSysCfg,TObject)
};

#pragma CIDLIB_POPPACK

