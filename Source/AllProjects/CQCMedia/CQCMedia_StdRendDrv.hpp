//
// FILE NAME: CQCMedia_StdRendDrv.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/31/2013
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
//  This is the header file for the standard media renderer driver base class.
//  Renderer drivers will share the bulk of functionality and really only need
//  to deal with the specifics of the player they control. So most of them
//  will derive from this class to vastly simplify things.
//
//  This class can implement a V1 or V2 driver architecture compliant mode,
//  depending on the architecture set on the driver config (driven by the
//  manifest file.)
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: TCQCStdMediaRendDrv
//  PREFIX: drv
// ---------------------------------------------------------------------------
class CQCMEDIAEXPORT TCQCStdMediaRendDrv : public TCQCServerBase
{
    public :
        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static tCIDLib::TVoid QueryStdRendFields
        (
                    TVector<TCQCFldDef>&    colToFill
            , const tCQCMedia::EMTFlags     eTypes
            , const tCIDLib::TCard4         c4ArchVer
            , const tCIDLib::TCard4         c4Flags
        );

        static const TString& strRendFld
        (
            const   tCQCMedia::ERendFlds    eFld
            , const tCIDLib::TBoolean       bV2
        );

        static const TString& strRendFld1
        (
            const   tCQCMedia::ERendFlds    eFld
        );

        static const TString& strRendFld2
        (
            const   tCQCMedia::ERendFlds    eFld
        );


        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TCQCStdMediaRendDrv() = delete;

        TCQCStdMediaRendDrv(const TCQCStdMediaRendDrv&) = delete;
        TCQCStdMediaRendDrv(TCQCStdMediaRendDrv&&) = delete;

        ~TCQCStdMediaRendDrv();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCStdMediaRendDrv& operator=(const TCQCStdMediaRendDrv&) = delete;
        TCQCStdMediaRendDrv& operator=(TCQCStdMediaRendDrv&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bQueryData
        (
            const   TString&                strQType
            , const TString&                strDataName
            ,       tCIDLib::TCard4&        c4OutBytes
            ,       THeapBuf&               mbufToFill
        )   override;

        tCIDLib::TCard4 c4QueryVal
        (
            const   TString&                strValId
        )   override;


    protected :
        // -------------------------------------------------------------------
        //  Hidden constructors
        // -------------------------------------------------------------------
        TCQCStdMediaRendDrv
        (
            const   TCQCDriverObjCfg&       cqcdcToLoad
            , const tCQCMedia::EPLModes     eDefPLMode
            , const tCIDLib::TCard4         c4FldFlags
            , const tCQCMedia::EMTFlags     eMTFlags
        );


        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bWaitConfig
        (
                    TThread&                thrThis
        )   override;

        tCQCKit::ECommResults eBoolFldValChanged
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4FldId
            , const tCIDLib::TBoolean       bNewValue
        )   override;

        tCQCKit::ECommResults eCardFldValChanged
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4FldId
            , const tCIDLib::TCard4         c4NewValue
        )   override;

        tCQCKit::ECommResults eConnectToDevice
        (
                    TThread&                thrThis
        )   override;

        tCQCKit::ECommResults eFloatFldValChanged
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4FldId
            , const tCIDLib::TFloat8        f8NewValue
        )   override;

        tCQCKit::EDrvInitRes eInitializeImpl() override;

        tCQCKit::ECommResults ePollDevice
        (
                    TThread&                thrThis
        )   override;

        tCQCKit::ECommResults eStringFldValChanged
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4FldId
            , const TString&                strNewValue
        )   override;


        // -------------------------------------------------------------------
        //  Protected, virtual methods
        // -------------------------------------------------------------------
        virtual tCIDLib::TBoolean bPlayNewItem
        (
            const   TCQCMediaPLItem&        mpliNew
        ) = 0;

        virtual tCIDLib::TVoid ClearMetaFields();

        virtual tCQCKit::ECommResults eAdjustVolume
        (
            const   tCIDLib::TBoolean       bUp
        ) = 0;

        virtual tCQCKit::ECommResults eCheckPlayerStatus
        (
                    tCIDLib::TEncodedTime&  enctCurPos
            ,       tCIDLib::TEncodedTime&  enctTotal
            ,       tCQCMedia::EMediaStates& eState
            ,       tCQCMedia::EEndStates&  eEndState
            ,       tCIDLib::TBoolean&      bMute
            ,       tCIDLib::TCard4&        c4Volume
        ) = 0;

        virtual tCQCKit::ECommResults eDoTransportCmd
        (
            const   tCQCMedia::ETransCmds   eToSet
        ) = 0;

        virtual tCQCKit::ECommResults eSetMute
        (
            const   tCIDLib::TBoolean       bToSet
        ) = 0;

        virtual tCQCKit::ECommResults eStartPowerTrans
        (
            const   tCIDLib::TBoolean       bNewState
        );

        virtual tCQCKit::ECommResults eSetVolume
        (
            const   tCIDLib::TCard4         c4ToSet
        ) = 0;

        virtual tCIDLib::TVoid LoadDrvSpecificFlds
        (
                    TVector<TCQCFldDef>&    colFlds
        );

        virtual tCIDLib::TVoid LookupDrvSpecificFlds
        (
            const   tCIDLib::TCard4         c4ArchVersion
        );

        virtual tCIDLib::TVoid ResetPlayer() = 0;

        virtual tCIDLib::TVoid UpdateMetaFields
        (
            const   TCQCMediaPLItem&        mpliCur
        );


        // -------------------------------------------------------------------
        //  Protected, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TCard4 c4DefVolume() const;

        tCIDLib::TVoid SelectNextPrev
        (
            const   tCIDLib::TBoolean       bNext
        );

        tCIDLib::TVoid SetMTFlags
        (
            const   tCQCMedia::EMTFlags     eToSet
        );

        tCIDLib::TVoid StorePowerStatus
        (
            const   tCQCKit::EPowerStatus   eStatus
        );


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCheckArt
        (
            const   tCIDLib::TBoolean       bLarge
            ,       tCIDLib::TCard4&        c4CacheBytes
            ,       THeapBuf&               mbufCache
            ,       tCIDLib::TCard4&        c4RetBytes
            ,       THeapBuf&               mbufRet
        );

        tCIDLib::TBoolean bDelPLItem
        (
            const   tCIDLib::TCard4         c4ItemId
        );

        tCIDLib::TBoolean bDoPlayMedia
        (
            const   TString&                strToPlay
            , const tCIDLib::TBoolean       bEnqueue
        );

        tCIDLib::TBoolean bRandomCatUpdate();

        tCIDLib::TBoolean bSelPLItem
        (
            const   tCIDLib::TCard4         c4ItemId
        );

        tCIDLib::TVoid LoadNewItem();

        tCIDLib::TVoid StorePLFields();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4CurLArtBytes
        //  m_c4CurSArtBytes
        //      In order to avoid doing redundant queries to the repository,
        //      we keep around the last art we queried, until the current item
        //      changes. Then these are zeroed, to indicate we don't have any
        //      cached art until somoene asks and we fault it again. We have
        //      to fault in both small and large art as they are requested,
        //      since we have to serve up either.
        //
        //  m_c4DefVolume
        //      A default volume gotten from a driver prompt. We store it for
        //      the derived class to use if he wants.
        //
        //  m_c4FldFlags
        //      The derived class provides the flags we should pass to the
        //      standard field query.
        //
        //  m_c4FldId_XXX
        //      We look up the ids of our fields after registering them, so
        //      that we can do efficient 'by id' reads/writes.
        //
        //  m_eLastEndState
        //  m_eLastState
        //      The last states we got from the derived class, so that
        //      we can know when it changes during polling.
        //
        //  m_eMTFlags
        //      The derived class tells us what media types it supports.
        //      We need this to make sure that we only get acceptable media
        //      queued up on us.
        //
        //  m_enctLastRandCat
        //      If in random category mode, we try to get another batch of
        //      values when the list gets below a certain amount. But, if
        //      they give us a category with fewer items than the size we
        //      try to maintain, we'd do the query after the end of every
        //      song. It's a fairly heavy operation on the media driver, so
        //      we limit how often we'll do it to no more than one once
        //      every 10 minutes worst case. We reset this any time the
        //      playlist mode is set to random cat or any time that the
        //      random category is set and we are in random cat mode, to
        //      allow a new query immediately.
        //
        //  m_mplmList
        //      We use a standard play list manager to handle our playlist.
        //      It provides all the grunt work required to keep up with the
        //      list and do new song selection and so forth.
        //
        //  m_mbufCurLArt
        //  m_mbufCurSArt
        //      This is where we cache current cover art. If m_c4CurLArtBytes
        //      or m_c4CurSArtBytes is non-zero, then this holds the most
        //      recent cover art of that size.
        //
        //  m_strDefRepoMoniker
        //      The default repository moniker that the user set during the
        //      driver install. This will be the default unless it is changed
        //      by writing to the repository moniker field.
        //
        //  m_strRandomCat
        //      This is the category that we use if we are put into
        //      random category mode. By default we'll se it to all music.
        //      But it can be set via the SetShuffleCat field.
        //
        //  m_strTmp
        //  m_strTmp2
        //      Some temp strings. Don't assume that they will be unaffected
        //      across any calls.
        // -------------------------------------------------------------------
        tCIDLib::TCard4         m_c4CurLArtBytes;
        tCIDLib::TCard4         m_c4CurSArtBytes;
        tCIDLib::TCard4         m_c4DefVolume;
        tCIDLib::TCard4         m_c4FldFlags;
        tCIDLib::TCard4         m_c4FldId_ActiveRepository;
        tCIDLib::TCard4         m_c4FldId_AdjVolume;
        tCIDLib::TCard4         m_c4FldId_ClearPL;
        tCIDLib::TCard4         m_c4FldId_CurAspectRatio;
        tCIDLib::TCard4         m_c4FldId_CurCast;
        tCIDLib::TCard4         m_c4FldId_CurColArtist;
        tCIDLib::TCard4         m_c4FldId_CurColCookie;
        tCIDLib::TCard4         m_c4FldId_CurColName;
        tCIDLib::TCard4         m_c4FldId_CurDescr;
        tCIDLib::TCard4         m_c4FldId_CurItemName;
        tCIDLib::TCard4         m_c4FldId_CurItemArtist;
        tCIDLib::TCard4         m_c4FldId_CurItemCookie;
        tCIDLib::TCard4         m_c4FldId_CurItemTime;
        tCIDLib::TCard4         m_c4FldId_CurItemTotal;
        tCIDLib::TCard4         m_c4FldId_CurLabel;
        tCIDLib::TCard4         m_c4FldId_CurLeadActor;
        tCIDLib::TCard4         m_c4FldId_CurRating;
        tCIDLib::TCard4         m_c4FldId_CurTitleName;
        tCIDLib::TCard4         m_c4FldId_CurTitleCookie;
        tCIDLib::TCard4         m_c4FldId_CurYear;
        tCIDLib::TCard4         m_c4FldId_DelPLItem;
        tCIDLib::TCard4         m_c4FldId_EnqueueMedia;
        tCIDLib::TCard4         m_c4FldId_MediaState;
        tCIDLib::TCard4         m_c4FldId_Mute;
        tCIDLib::TCard4         m_c4FldId_PBPercent;
        tCIDLib::TCard4         m_c4FldId_PlayerStatus;
        tCIDLib::TCard4         m_c4FldId_PlayMedia;
        tCIDLib::TCard4         m_c4FldId_PLIndex;
        tCIDLib::TCard4         m_c4FldId_PLItemCnt;
        tCIDLib::TCard4         m_c4FldId_PLItemKey;
        tCIDLib::TCard4         m_c4FldId_PLMode;
        tCIDLib::TCard4         m_c4FldId_PLSerialNum;
        tCIDLib::TCard4         m_c4FldId_Power;
        tCIDLib::TCard4         m_c4FldId_PowerStatus;
        tCIDLib::TCard4         m_c4FldId_SelPLItem;
        tCIDLib::TCard4         m_c4FldId_SetRandomCat;
        tCIDLib::TCard4         m_c4FldId_SetRepository;
        tCIDLib::TCard4         m_c4FldId_Transport;
        tCIDLib::TCard4         m_c4FldId_Volume;
        tCIDLib::TCard4         m_c4FldId_ZoneNum;
        tCQCMedia::EEndStates   m_eLastEndState;
        tCQCMedia::EMediaStates m_eLastState;
        tCQCMedia::EMTFlags     m_eMTFlags;
        tCIDLib::TEncodedTime   m_enctLastRandCat;
        THeapBuf                m_mbufCurLArt;
        THeapBuf                m_mbufCurSArt;
        TCQCMediaPLMgr          m_mplmList;
        TString                 m_strDefRepoMoniker;
        TString                 m_strRandomCat;
        TString                 m_strTmp;
        TString                 m_strTmp2;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCStdMediaRendDrv,TCQCServerBase)
};

#pragma CIDLIB_POPPACK


