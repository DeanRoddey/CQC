//
// FILE NAME: CQCMedia_PLItem.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 12/20/2006
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
//  This is the header file for the CQCMedia_PLItem.cpp file. This file
//  implements a class used by various media renderer drivers. These guys
//  need to keep a current playlist of things that have been queued up on
//  them. They then watch for what teh player device is playing an set up
//  their metadata fields by looking up that media in a list of these
//  object.
//
//  So we just provide storage for all the standard info that a renderer
//  driver will need for each media item. This includes often rendundant
//  collection level info since many items in the list will be from the
//  same collection. But we have to do it this way so that the renderers
//  can be purely item based while still being able to show collection level
//  info for whatever item is being played.
//
//  We also provide a playlist manager class. All the renderer drivers that
//  don't just pass the playlist info to some external player need to
//  manage a playlist and the logic is always the same. So this prevents
//  a lot of redundancy.
//
//  The following rules apply with the play list manager:
//
//  1.  The playlist manager keeps up with the current play list item index.
//      When the list is empty or new items have just been added but nothing
//      has been selected yet to start playing, the index will be
//      kCIDLib::c4MaxCard, so be careful of using the index.
//
//      Also, if you remove an item, and it was the current item, then index
//      is set back to max card, and the action return indicates you must
//      select a new item.
//
//  2.  When a new item is selected by the manager, it will mark the item
//      played on the assumption that the caller is about to start playing
//      it. This allows the random shulffle selection stuff to work correctly.
//
//  3.  It handles adding and removing items and returns a status to tell
//      the calling app what to do after these operations, i.e. if it needs
//      to make a selection and start playing something because new items
//      were added to a previously empty playlist.
//
//  4.  If the calling app asks to remove an item, the remove method will
//      return a status to tell it if any action is required, e.g. stop
//      if that was the list item, or select a new item if that was the
//      current item. In that latter case, the current index is set to
//      max card so that the selection of a new item won't delete an another
//      item if you are in jukebox mode.
//
//  5.  Each new item added is given a unique id. This lets clients get back
//      into sync if there are additions or removals, i.e. they can know
//      which one they already have and which are new or missing.
//
//
// CAVEATS/GOTCHAS:
//
//  1)  These are streamable, but not for reasons of persistence, just for
//      being able to send them over the ORB, so order of enum values and
//      such aren't an issue.
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: TCQCMediaPLItem
//  PREFIX: mpli
// ---------------------------------------------------------------------------
class CQCMEDIAEXPORT TCQCMediaPLItem : public TObject, public MStreamable
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCMediaPLItem();

        TCQCMediaPLItem(const TCQCMediaPLItem&) = default;
        TCQCMediaPLItem(TCQCMediaPLItem&&) = default;

        ~TCQCMediaPLItem();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCMediaPLItem& operator=(const TCQCMediaPLItem&) = default;
        TCQCMediaPLItem& operator=(TCQCMediaPLItem&&) = default;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bPlayed() const;

        tCIDLib::TBoolean bPlayed
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TCard4 c4BitDepth() const;

        tCIDLib::TCard4 c4BitDepth
        (
            const   tCIDLib::TCard4         c4ToSet
        );

        tCIDLib::TCard4 c4BitRate() const;

        tCIDLib::TCard4 c4BitRate
        (
            const   tCIDLib::TCard4         c4ToSet
        );

        tCIDLib::TCard4 c4Channels() const;

        tCIDLib::TCard4 c4Channels
        (
            const   tCIDLib::TCard4         c4ToSet
        );

        tCIDLib::TCard4 c4ColYear() const;

        tCIDLib::TCard4 c4ColYear
        (
            const   tCIDLib::TCard4         c4ToSet
        );

        tCIDLib::TCard4 c4ListItemId() const;

        tCIDLib::TCard4 c4ItemSeconds() const;

        tCIDLib::TCard4 c4ItemSeconds
        (
            const   tCIDLib::TCard4         c4ToSet
        );

        tCQCMedia::ELocTypes eLocType() const;

        tCQCMedia::ELocTypes eLocType
        (
            const   tCQCMedia::ELocTypes    eToSet
        );

        tCQCMedia::EMediaTypes eMediaType() const;

        tCQCMedia::EMediaTypes eMediaType
        (
            const   tCQCMedia::EMediaTypes  eToSet
        );

        tCIDLib::TVoid Reset();

        const TString& strColArtist() const;

        const TString& strColArtist
        (
            const   TString&                strToSet
        );

        const TString& strColAspectRatio() const;

        const TString& strColAspectRatio
        (
            const   TString&                strToSet
        );

        const TString& strColCast() const;

        const TString& strColCast
        (
            const   TString&                strToSet
        );

        const TString& strColCookie() const;

        const TString& strColCookie
        (
            const   TString&                strToSet
        );

        const TString& strColDescription() const;

        const TString& strColDescription
        (
            const   TString&                strToSet
        );

        const TString& strColLabel() const;

        const TString& strColLabel
        (
            const   TString&                strToSet
        );

        const TString& strColLeadActor() const;

        const TString& strColLeadActor
        (
            const   TString&                strToSet
        );

        const TString& strColName() const;

        const TString& strColName
        (
            const   TString&                strToSet
        );

        const TString& strColRating() const;

        const TString& strColRating
        (
            const   TString&                strToSet
        );

        const TString& strFmtTime() const;

        const TString& strItemArtist() const;

        const TString& strItemArtist
        (
            const   TString&                strToSet
        );

        const TString& strItemCookie() const;

        const TString& strItemCookie
        (
            const   TString&                strToSet
        );

        const TString& strItemName() const;

        const TString& strItemName
        (
            const   TString&                strToSet
        );

        const TString& strLocInfo() const;

        const TString& strLocInfo
        (
            const   TString&                strToSet
        );

        const TString& strMediaFormat() const;

        const TString& strMediaFormat
        (
            const   TString&                strToSet
        );

        const TString& strRepoMoniker() const;

        const TString& strRepoMoniker
        (
            const   TString&                strToSet
        );

        const TString& strTitleCookie() const;

        const TString& strTitleCookie
        (
            const   TString&                strToSet
        );

        const TString& strTitleName() const;

        const TString& strTitleName
        (
            const   TString&                strToSet
        );

        const TString& strUserData() const;

        const TString& strUserData
        (
            const   TString&                strToSet
        );

        tCIDLib::TVoid Set
        (
            const   tCIDLib::TCard4         c4BitDepth
            , const tCIDLib::TCard4         c4BitRate
            , const tCIDLib::TCard4         c4Channels
            , const tCIDLib::TCard4         c4Duration
            , const tCIDLib::TCard4         c4Year
            , const tCQCMedia::ELocTypes    eLocType
            , const tCQCMedia::EMediaTypes  eMediaType
            , const TString&                strArtist
            , const TString&                strAspectRatio
            , const TString&                strCast
            , const TString&                strColCookie
            , const TString&                strColName
            , const TString&                strColRating
            , const TString&                strDescription
            , const TString&                strLabel
            , const TString&                strLeadActor
            , const TString&                strLocInfo
            , const TString&                strMediaFmt
            , const TString&                strRepoMoniker
            , const TString&                strTitleCookie
            , const TString&                strTitleName
            , const tCIDLib::TCard4         c4ListItemId
        );

        tCIDLib::TVoid Set
        (
            const   tCIDLib::TCard4         c4BitDepth
            , const tCIDLib::TCard4         c4BitRate
            , const tCIDLib::TCard4         c4Channels
            , const tCIDLib::TCard4         c4Duration
            , const tCIDLib::TCard4         c4Year
            , const tCQCMedia::ELocTypes    eLocType
            , const tCQCMedia::EMediaTypes  eMediaType
            , const TString&                strArtist
            , const TString&                strAspectRatio
            , const TString&                strCast
            , const TString&                strColCookie
            , const TString&                strColName
            , const TString&                strColRating
            , const TString&                strDescription
            , const TString&                strLabel
            , const TString&                strLeadActor
            , const TString&                strLocInfo
            , const TString&                strItemArtist
            , const TString&                strItemCookie
            , const TString&                strItemName
            , const TString&                strMediaFmt
            , const TString&                strRepoMoniker
            , const TString&                strTitleCookie
            , const TString&                strTitleName
            , const tCIDLib::TCard4         c4ListItemId
        );

        tCIDLib::TVoid SetListItemId
        (
            const   tCIDLib::TCard4         c4ToSet
        );


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
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid FormatTotalTime();


        // -------------------------------------------------------------------
        //  Public data members
        //
        //  m_bPlayed
        //      This is used by renderer drivers when they are doing shuffle
        //      style playback, so that they can not repeat any song until
        //      all of the current playlist items have been played.
        //
        //  m_c4ListItemId
        //      A unique id assigned by the playlist manager for each new
        //      item.
        //
        //  m_eMediaType
        //      The type of media represented by this item object.
        //
        //  m_xxxColXXX
        //      The collection level metadata fields
        //
        //  m_strFmtTime
        //      Any time that the seconds are set, we update this to hold
        //      the time in the standard media time format "Xm, Ys" since
        //      that's commonly wanted.
        //
        //  m_xxxItemXXX
        //      The item level metadata fields
        //
        //  m_strLocInfo
        //      The location information, which will either be collection
        //      or item level info, according to the m_eLocType value.
        //
        //  m_xxxTitleXXX
        //      The title level metadata fields
        //
        //  m_strLocInfo
        //      The location info might come from the collection or the
        //      item, based on the location type indicated in the collection.
        //
        //  m_strUserData
        //      For use by client code, to provide some sort of association
        //      for a media item, generally by renderer drivers.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bPlayed;
        tCIDLib::TCard4         m_c4BitDepth;
        tCIDLib::TCard4         m_c4BitRate;
        tCIDLib::TCard4         m_c4Channels;
        tCIDLib::TCard4         m_c4ItemSeconds;
        tCIDLib::TCard4         m_c4ListItemId;
        tCIDLib::TCard4         m_c4ColYear;
        tCQCMedia::ELocTypes    m_eLocType;
        tCQCMedia::EMediaTypes  m_eMediaType;
        TString                 m_strColArtist;
        TString                 m_strColAspectRatio;
        TString                 m_strColCast;
        TString                 m_strColCookie;
        TString                 m_strColDescription;
        TString                 m_strColLabel;
        TString                 m_strColLeadActor;
        TString                 m_strColName;
        TString                 m_strColRating;
        TString                 m_strFmtTime;
        TString                 m_strItemArtist;
        TString                 m_strItemCookie;
        TString                 m_strItemName;
        TString                 m_strLocInfo;
        TString                 m_strMediaFmt;
        TString                 m_strRepoMoniker;
        TString                 m_strTitleCookie;
        TString                 m_strTitleName;
        TString                 m_strUserData;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCMediaPLItem,TObject)
};




// ---------------------------------------------------------------------------
//   CLASS: TCQCMediaPLMgr
//  PREFIX: mplm
// ---------------------------------------------------------------------------
class CQCMEDIAEXPORT TCQCMediaPLMgr : public TObject, public MStreamable
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCMediaPLMgr();

        TCQCMediaPLMgr
        (
            const   TString&                strMoniker
        );

        TCQCMediaPLMgr(const TCQCMediaPLMgr&) = delete;
        TCQCMediaPLMgr(TCQCMediaPLMgr&&) = delete;

        ~TCQCMediaPLMgr();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCMediaPLMgr& operator=(const TCQCMediaPLMgr&) = delete;
        TCQCMediaPLMgr& operator=(TCQCMediaPLMgr&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bHasItemPlayed
        (
            const   tCIDLib::TCard4         c4At
        )   const;

        tCIDLib::TBoolean bIsEmpty() const
        {
            return m_colPlayList.bIsEmpty();
        }

        tCIDLib::TBoolean bIsFull() const;

        tCIDLib::TBoolean bQueryCurCookie
        (
                    TString&                strRepo
            ,       TString&                strCookie
            ,       tCIDLib::TBoolean&      bItem
        )   const;

        tCIDLib::TCard4 c4CurIndex() const
        {
            return m_c4CurIndex;
        }

        tCIDLib::TBoolean bQueryCurItemId
        (
                    tCIDLib::TCard4&        c4ToFill
        )   const;

        tCIDLib::TCard4 c4FindByLocInfo
        (
            const   TString&                strToFind
        );

        tCIDLib::TCard4 c4ItemCount() const
        {
            return m_colPlayList.c4ElemCount();
        }

        tCIDLib::TCard4 c4SerialNum() const
        {
            return m_c4SerialNum;
        }

        tCQCMedia::EPLMgrActs eAddMedia
        (
            const   TString&                strDefRepo
            , const TString&                strToPlay
            , const tCIDLib::TBoolean       bEnqueue
            , const tCQCMedia::EMTFlags     eMTFlags
            , const TString&                strUserData
        );

        tCQCMedia::EPLModes ePlayListMode() const;

        tCQCMedia::EPLModes ePlayListMode
        (
            const   tCQCMedia::EPLModes     eToSet
        );

        tCQCMedia::EPLMgrActs eRandomCatUpdate
        (
            const   TString&                strRepoMoniker
            , const TString&                strCatCookie
        );

        tCQCMedia::EPLMgrActs eRemoveItemAt
        (
            const   tCIDLib::TCard4         c4At
        );

        tCQCMedia::EPLMgrActs eRemoveItemById
        (
            const   tCIDLib::TCard4         c4Id
        );

        tCQCMedia::EPLMgrSelRes eSelectNewItem
        (
            const   tCIDLib::TBoolean       bNext
            ,       tCIDLib::TCard4&        c4NewIndex
        );

        tCQCMedia::EPLMgrActs eSelectItemById
        (
            const   tCIDLib::TCard4         c4Id
        );

        tCQCKit::EVerboseLvls eVerboseLevel() const;

        tCQCKit::EVerboseLvls eVerboseLevel
        (
            const   tCQCKit::EVerboseLvls   eToSet
        );

        const TCQCMediaPLItem& mpliAt
        (
            const   tCIDLib::TCard4         c4At
        )   const;

        const TCQCMediaPLItem& mpliCurrent() const;

        tCIDLib::TVoid Reset();

        const TString& strItemCookieAt
        (
            const   tCIDLib::TCard4         c4At
        )   const;

        const TString& strMoniker() const;

        const TString& strMoniker
        (
            const   TString&                strToSet
        );

        tCIDLib::TVoid SetItemList
        (
            const   TVector<TCQCMediaPLItem>& colToSet
            , const tCIDLib::TBoolean       bSetIds
        );


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
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TCard4 c4ShuffleSelect();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4CurIndex
        //      This is the index of the currently selected item in the play
        //      list. If the list is empty, this will be set to c4MaxCard.
        //
        //  m_c4NextItemId
        //      This is used to assign unique ids to the items in our list.
        //      It starts at one, and though it will never roll over in any
        //      realistic scenario, we will not set it to 0 if that happens,
        //      so zero is never used.
        //
        //  m_c4SerialNum
        //      This is bumped any time that the play list is changed.
        //
        //  m_colPlayList
        //      This is our list of standard playlist items that represents
        //      the playlist.
        //
        //  m_ePlayListMode
        //      Client code tells us what play list mode we are in, which
        //      controls how we choose the next item to return.
        //
        //  m_eVerboseLvl
        //      Client code can tell us to be verbose about logging.
        //
        //  m_randShuffle
        //      We need a random number generator to do random shuffle type
        //      selections.
        //
        //  m_strRepoMoniker
        //      This can be set for use in any errors logged and such. It is
        //      assumed to be the moniker of the renderer driver managing this
        //      playlist.
        // -------------------------------------------------------------------
        tCIDLib::TCard4             m_c4CurIndex;
        tCIDLib::TCard4             m_c4NextItemId;
        tCIDLib::TCard4             m_c4SerialNum;
        TVector<TCQCMediaPLItem>    m_colPlayList;
        tCQCMedia::EPLModes         m_ePlayListMode;
        tCQCKit::EVerboseLvls       m_eVerboseLvl;
        TRandomNum                  m_randShuffle;
        TString                     m_strMoniker;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCMediaPLMgr,TObject)
};

#pragma CIDLIB_POPPACK

