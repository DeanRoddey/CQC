//
// FILE NAME: CQCTrayMon_iTunesTab.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/14/2012
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
//  This is the header for the CQCTrayMon_iTunesTab.cpp file, which implements
//  the TCQCTrayiTunesTab class. This class provides support for an iTunes
//  repository driver running on the same machine.
//
//  We basically provide the driver functionality here, the actual iTunes
//  repo driver is just a passthrough. We use a standard repo driver database
//  engine and the driver just passes through commands to us, and we pass
//  them on to the engine. All we have to do is load the database up.
//
//  We spin off a thread which maintains a standard CQC in memory media
//  database just as a repo driver would. We service incoming driver calls
//  from that database just as a real repo driver would.
//
//  Since we can get event notifications from the iTunes database, we can
//  do incremental updates when changes are made via the iTunes interface.
//  We just queue them up and the background thread can apply those changes
//  to the local database. So the driver will see them upon any subsequent
//  accesses.
//
//  Each object in the iTunes database is identified by from two to four
//  ids, representing the iTunes hierarchy (source, playlist, track.) The
//  track has two ids, one that is database wide and one that is relative
//  to the indicated playlist. Playlists only use the first two values.
//
//  We format these ids in order to create our unique ids for title sets,
//  collections, and items. This lets us map from iTunes ids to our objects,
//  and from our objects back to iTunes objects.
//
//  The ids are 4 chunks of 8 hex digits each, separated by dashes, so
//  something like:
//
//      00000001-00000002-00000003-00000004
//
//  There is a special case for tracks (our items.) We don't redundantly
//  store items per-playlist, the playlists just reference them. So we only
//  format out the last value (the database wide id) for the individual
//  tracks (the first three values we set to all zeros.) This way, no
//  matter what iTunes playlist a track gets reported via, we can still
//  find it in our database.
//
//  We do though still also store the full set of ids in the 'alt id' value
//  of the items, so that if we need to map back from our item to a track,
//  we can still do that. On the iTunes side, it doesn't matter which list
//  we get to the track via.
//
//  So the real unique id for tracks is just the last (database wide) id,
//  and that is used for mapping from iTunes events to our items (we can
//  always just grab the database id, no matter what playlist the track
//  is being reported through), but we keep around the full id for if we
//  need to map from our side to the iTunes side (which requires the full
//  four ids in some valid combination that references the desired track.)
//
//  We do distinguish between the 'main library' list which contains the
//  whole list of tracks, and anything else which we treat as a playlist
//  on our side. So we iterate the main list to get all the tracks (and
//  we separate them into collections (via artist/album name.) We store the
//  playlist id of the main list, so that we can quickly check whether
//  some change event is relative to it, or to a playlist. If relative to
//  the main list we affect the actual items, e.g. delete the item if it
//  is deleted from the main list. A delete via a playlist just removes
//  the item from the indicated playlist, but doesn't remove the track.
//
//  There is a lot of nitpicky complexity involved in this. Look at the
//  ProcessEvents() method where change events are handled. We have to
//  very carefully figure out how we are referencing things to decide
//  if we should really affect real stuff, or just playlist references.
//
//  There's no other hierarchy in iTunes, so we just have one collection
//  per set. And we don't look at any 'library' lists, which are the
//  lists that map to directories that tracks might be in. We don't care
//  about the physical distribution, only the logical one driven by the
//  artist/album name driven associations.
//
//  We also support player control and some basic feedback. Incoming
//  commands are queued up to be processed. Feedback data is kept updated
//  by the bgn thread that does the iTunes COM interfacing, so we can
//  just return it immediately from our own members.
//
//
// CAVEATS/GOTCHAS:
//
//  1)  When we register our player and repo objects with the ORB, we use
//      the local host name in the binding, no matter what the user indicated
//      on the Tray Monitor command line. The Tray Monitor and App Control
//      stuff has to be accessed from anywhere, but the drivers that talk
//      to us always assume they are talking to something on the local host.
//
// LOG:
//
//  $Log$
//
#pragma once


struct IiTunes;
struct IITTrack;
struct IITObject;
struct IITPlaylist;
class  TITEventHandler;


// ---------------------------------------------------------------------------
//  CLASS: TDBQChangeInfo
// PREFIX: dbci
//
//  We need this for queuing up change/delete notifications from the change
//  event handler to the tab window.
// ---------------------------------------------------------------------------
class TDBQChangeInfo : public TObject
{
    public  :
        // -------------------------------------------------------------------
        // Constructors and destructor
        // -------------------------------------------------------------------
        TDBQChangeInfo();

        TDBQChangeInfo
        (
            const   tCIDLib::TInt4          i4Src
            , const tCIDLib::TInt4          i4PlayList
            , const tCIDLib::TInt4          i4Track
            , const tCIDLib::TInt4          i4Database
            , const tCIDLib::TBoolean       bDelete
        );

        TDBQChangeInfo
        (
            const   TDBQChangeInfo&         dbciSrc
        );

        ~TDBQChangeInfo();

        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TDBQChangeInfo& operator=
        (
            const   TDBQChangeInfo&         dbciSrc
        );


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCQCTrayMon::EiTunesObjs eObjType() const;


        // -------------------------------------------------------------------
        //  Public class members
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bDelete;
        tCIDLib::TInt4      m_i4Database;
        tCIDLib::TInt4      m_i4PlayList;
        tCIDLib::TInt4      m_i4Src;
        tCIDLib::TInt4      m_i4Track;


    private :
        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TDBQChangeInfo, TObject);
};


// ---------------------------------------------------------------------------
//  CLASS: TPlayerCmd
// PREFIX: plcmd
//
//  We need this for queuing up incoming player commands.
// ---------------------------------------------------------------------------
class TPlayerCmd : public TObject
{
    public  :
        // --------------------------------------------------------------------
        //  Get a short cut type name
        // --------------------------------------------------------------------
        typedef TiTPlayerIntfServerBase     TPlBase;


        // -------------------------------------------------------------------
        // Constructors and destructor
        // -------------------------------------------------------------------
        TPlayerCmd();

        TPlayerCmd
        (
            const   TiTPlayerIntfServerBase::EPlCmds eCmd
        );

        TPlayerCmd
        (
            const   tCQCTrayMon::EPlCmdTypes eType
            , const tCIDLib::TCard4         c4Value
        );

        TPlayerCmd
        (
            const   tCQCTrayMon::EPlCmdTypes eType
            , const tCIDLib::TBoolean       bValue
        );

        TPlayerCmd
        (
            const   tCQCTrayMon::EPlCmdTypes eType
            , const TString&                strValue
        );

        TPlayerCmd
        (
            const   TPlayerCmd&             plcmdSrc
        );

        ~TPlayerCmd();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TPlayerCmd& operator=
        (
            const   TPlayerCmd&             plcmdSrc
        );


        // -------------------------------------------------------------------
        //  Public class members
        // -------------------------------------------------------------------
        tCIDLib::TBoolean           m_bValue;
        tCIDLib::TCard4             m_c4Value;
        TiTPlayerIntfServerBase::EPlCmds m_eCmd;
        tCQCTrayMon::EPlCmdTypes    m_eType;
        TString                     m_strValue;


    private :
        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TPlayerCmd, TObject);
};


// ---------------------------------------------------------------------------
//  CLASS: TiTMediaRepoEng
// PREFIX: srdb
// ---------------------------------------------------------------------------
class TiTMediaRepoEng : public TCQCStdMediaRepoEng
{
    public  :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TiTMediaRepoEng
        (
            const   TString&                strMoniker
        );

        ~TiTMediaRepoEng();


        // -------------------------------------------------------------------
        //  Public, inheritedmethods
        // -------------------------------------------------------------------
        tCIDLib::TCard4 c4LoadRawCoverArt
        (
            const   TMediaImg&              mimgToLoad
            ,       TMemBuf&                mbufToFill
            , const tCQCMedia::ERArtTypes   eType
        );

    private :
        RTTIDefs(TiTMediaRepoEng,TCQCStdMediaRepoEng)
};


// ---------------------------------------------------------------------------
//  CLASS: TCQCTrayiTunesTab
// PREFIX: wnd
// ---------------------------------------------------------------------------
class TCQCTrayiTunesTab : public TCQCTrayMonTab
{
    public  :
        // --------------------------------------------------------------------
        //  Get a short cut type name
        // --------------------------------------------------------------------
        typedef TiTPlayerIntfServerBase     TPlBase;


        // -------------------------------------------------------------------
        // Constructors and destructor
        // -------------------------------------------------------------------
        TCQCTrayiTunesTab();

        ~TCQCTrayiTunesTab();


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid Cleanup() override;

        tCIDLib::TVoid CreateTab
        (
                    TTabbedWnd&             wndParent
        )   override;

        tCIDLib::TVoid RemoveBindings
        (
                    tCIDOrbUC::TNSrvProxy&  orbcNS
        )   override;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------

        // Repo oriented
        tCIDLib::TBoolean bQueryData
        (
            const   TString&                strQueryType
            , const TString&                strDataName
            ,       tCIDLib::TCard4&        c4BufSz
            ,       THeapBuf&               mbufToFill
        );

        tCIDLib::TBoolean bQueryData2
        (
            const   TString&                strQueryType
            ,       tCIDLib::TCard4&        c4BufSz
            ,       THeapBuf&               mbufIO
        );

        tCIDLib::TBoolean bQueryTextVal
        (
            const   TString&                strQueryType
            , const TString&                strDataName
            ,       TString&                strToFill
        );

        tCIDLib::TBoolean bReloadDB();

        tCIDLib::TBoolean bSendData
        (
            const   TString&                strSendType
            ,       TString&                strDataName
            ,       tCIDLib::TCard4&        c4BufSz
            ,       THeapBuf&               mbufToSend
        );

        tCIDLib::TCard4 c4QueryVal
        (
            const   TString&                strValId
        );

        tCIDLib::TCard4 c4SendCmd
        (
            const   TString&                strCmdId
            , const TString&                strParms
        );

        TMutex& mtxDatabase();

        tCIDLib::TVoid QueryStatus
        (
                    tCQCMedia::ELoadStatus& eCurStatus
            ,       tCQCMedia::ELoadStatus& eLoadStatus
            ,       TString&                strDBSerialNum
            ,       tCIDLib::TCard4&        c4TitleCnt
        );



        // Calls from the event handler
        tCIDLib::TVoid OnDBChange
        (
            const   tCIDLib::TInt4          i4Src
            , const tCIDLib::TInt4          i4PL
            , const tCIDLib::TInt4          i4Track
            , const tCIDLib::TInt4          i4DB
            , const tCIDLib::TBoolean       bDelete
        );

        tCIDLib::TVoid OnQuitNotification();

        tCIDLib::TVoid OnStop();

        tCIDLib::TVoid OnTrackChange
        (
            const   TString&                strAlbum
            , const TString&                strArtist
            , const TString&                strTrack
            , const tCIDLib::TCard4         c4Duration
        );

        tCIDLib::TVoid OnVolumeChange
        (
            const   tCIDLib::TCard4         c4Volume
        );


        // Player oriented
        tCIDLib::TBoolean bGetPlayerState
        (
                    tCIDLib::TCard4&        c4SerialNum
            ,       tCIDLib::TBoolean&      bConnected
            ,       tCIDLib::TCard4&        c4Volume
            ,       TiTPlayerIntfServerBase::EPlStates& ePlState
            ,       tCIDLib::TBoolean&      bMute
            ,       tCIDLib::TEncodedTime&  enctTotal
            ,       tCIDLib::TEncodedTime&  enctCurPos
            ,       TString&                strArtist
            ,       TString&                strAlbum
            ,       TString&                strTrack
        );

        tCIDLib::TVoid DoPlayerCmd
        (
            const   TiTPlayerIntfServerBase::EPlCmds eCmd
        );

        tCIDLib::TVoid SelPLByCookie
        (
            const   TString&                strTitleCookie
        );

        tCIDLib::TVoid SelTrackByCookie
        (
            const   TString&                strItemCookie
        );

        tCIDLib::TVoid SetMute
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TVoid SetVolume
        (
            const   tCIDLib::TCard4         c4ToSet
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCreated() override;

        tCIDLib::TVoid Destroyed() override;

        tCIDLib::TVoid Timer
        (
            const   tCIDCtrls::TTimerId     tmidUpdate
        )   override;


    private :
        // -------------------------------------------------------------------
        //  Private class types
        // -------------------------------------------------------------------
        typedef TQueue<TDBQChangeInfo>      TDBEventQ;
        typedef TQueue<TPlayerCmd>          TPlCmdQ;


        // -------------------------------------------------------------------
        //  Unimplemented
        // -------------------------------------------------------------------
        TCQCTrayiTunesTab(const TCQCTrayiTunesTab&);
        tCIDLib::TVoid operator=(const TCQCTrayiTunesTab&);


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCheckEvents
        (
                    TThread&                thrCaller
        );

        tCIDLib::TBoolean bCheckGetObj
        (
            const   tCIDLib::TCard4         c4Res
            , const tCIDLib::TCh* const     pszType
            , const tCIDLib::TCard4         c4Line
        );

        tCIDLib::TBoolean bExtractTrackInfo
        (
                    IITTrack* const         pTrack
            ,       TString&                strAlbumTitle
            ,       TString&                strAlbumArtist
            ,       TString&                strArtist
            ,       TString&                strGenre
            ,       TString&                strLocInfo
            ,       TString&                strSortTitle
            ,       TString&                strTrackName
            ,       TString&                strUID
            ,       TString&                strAltUID
            ,       tCIDLib::TEncodedTime&  enctAdded
            ,       tCIDLib::TCard4&        c4BitRate
            ,       tCIDLib::TCard4&        c4Duration
            ,       tCIDLib::TCard4&        c4Rating
            ,       tCIDLib::TCard4&        c4SampleRate
            ,       tCIDLib::TCard4&        c4TrackNum
            ,       tCIDLib::TCard4&        c4Year
        );

        tCIDLib::TBoolean bGetObjectId
        (
                    IITObject* const        pObject
            ,       TString&                strUID
            ,       TString&                strTrackUID
        );

        tCIDLib::TBoolean bParseObjUID
        (
            const   TString&                strSrc
            ,       tCIDLib::TInt4&         i4Src
            ,       tCIDLib::TInt4&         i4PL
            ,       tCIDLib::TInt4&         i4Track
            ,       tCIDLib::TInt4&         i4DB
        );

        tCIDLib::TBoolean bProcessPlayList
        (
                    TMediaDB&               mdbTar
            ,       IITPlaylist* const      pPList
            ,       tCIDLib::TCard2&        c2TitleId
            ,       tCIDLib::TCard2&        c2ColId
        );

        tCIDLib::TBoolean bProcessTrack
        (
                    TMediaDB&               mdbTar
            ,       IITTrack* const         pTrack
            ,       tCIDLib::TCard2&        c2TitleId
            ,       tCIDLib::TCard2&        c2ColId
            ,       tCIDLib::TCard2&        c2ItemId
            , const tCIDLib::TBoolean       bFromEvent
        );

        tCIDLib::TVoid CalcDBSerialNum
        (
            const   TMediaDB&               mdbSrc
            ,       TString&                strToFill
        );

        tCIDLib::TVoid CheckDBReady
        (
            const   tCIDLib::TCard4         c4Line
        );

        tCIDLib::TVoid CleanupArt
        (
                    TMediaDB&               mdbTar
        );

        tCIDLib::TVoid ConnectToiTunes();

        tCIDLib::TVoid Disconnect();

        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );

        tCIDLib::EExitCodes eiTunesThread
        (
                    TThread&                thrThis
            ,       tCIDLib::TVoid*         pData
        );

        tCIDLib::TVoid FormatObjUID
        (
            const   tCIDLib::TInt4          i4Src
            , const tCIDLib::TInt4          i4PL
            , const tCIDLib::TInt4          i4Track
            , const tCIDLib::TInt4          i4DB
            ,       TString&                strUID
            ,       TString&                strAltUID
        );

        tCIDLib::TVoid FormatListUID
        (
            const   TDBQChangeInfo&         dbciEv
            ,       TString&                strUID
        );

        tCIDLib::TVoid FormatObjUID
        (
            const   TDBQChangeInfo&         dbciEv
            ,       TString&                strUID
            ,       TString&                strAltID
        );

        tCIDLib::TInt4 i4GetPLId
        (
                    IITObject* const        pPList
            ,       tCIDLib::TInt4&         i4SrcIdToFill
        );

        tCIDLib::TVoid LoadDB
        (
                    TThread&                thrLoader
        );

        const TMediaImg* pmimgStoreArt
        (
                    TMediaDB&               mdbTar
            ,       IITTrack* const         pTrack
            , const TString&                strAlbumUID
        );

        const tCIDLib::TCh* pszXlatStatus
        (
            const   tCQCMedia::ELoadStatus eStatus
        );

        tCIDLib::TVoid ProcessEvent
        (
            const   TDBQChangeInfo&         dbciEv
        );

        tCIDLib::TVoid ProcessPlCmd
        (
            const   TPlayerCmd&             plcmdToDo
        );

        tCIDLib::TVoid QLogMsg
        (
            const   TString&                strToQ
        );

        tCIDLib::TVoid QLogMsg
        (
            const   TString&                strToQ
            , const MFormattable&           mfmtblToken1
        );

        tCIDLib::TVoid QLogMsg
        (
            const   TString&                strToQ
            , const MFormattable&           mfmtblToken1
            , const MFormattable&           mfmtblToken2
        );

        tCIDLib::TVoid SetUserRating
        (
            const   TMediaTitleSet&         mtsUpdate
            , const tCIDLib::TCard4         c4Rating
        );

        tCIDLib::TVoid StoreCurPos();

        tCIDLib::TVoid StoreNewStatus
        (
            const   tCQCMedia::ELoadStatus eToSet

        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bCurMute
        //      We track the mute state in order to return to the player
        //      driver.
        //
        //  m_bDBChanges
        //      This is set when we make changes in the event callback. The bgn
        //      thread will check this every so often and generate a new database
        //      serial number.
        //
        //  m_bDelImages
        //      The window sets this to reflect the delete images checkbox.
        //      The bgn thread uses it to clear the images directory before
        //      a reload if this is set.
        //
        //  m_bForceReload
        //      The click handler for the force reload button will set this
        //      flag. The background thread, if not already loading, will
        //      start a new load and clear this flag.
        //
        //  m_bQuitNotification
        //      If the user forces iTunes down, our event handler will set
        //      this to let us know. We'll clean up and go back to trying to
        //      connect.
        //
        //  m_c2NextXXX
        //      During load we have to track the next available id for each
        //      media object type. However, since we have to deal with change
        //      events from iTunes, we have to keep these around so that we
        //      can continue to assign new ids between full loads.
        //
        //  m_c4EvAdviseId
        //      We have to store an event attachement id to later release
        //      that COM connection.
        //
        //  m_enctCurTotal
        //      When a new track starts playing we save the duration, It's zero when no
        //      track is playing, or if there is no duration info for the current track.
        //
        //  m_enctCurPos
        //      The last reported current playback position. This should be relative to
        //      m_enctCurTotal, and should never be larger than it. If so, we just keep the
        //      total value as the current value.
        //
        //  m_c4CurVolume
        //      The last player volume left by the event handler. This is
        //      stored away for later retrieval by the player control driver.
        //
        //  m_c4LastIgnoredCnt
        //  m_c4IgnoredCnt
        //      The loading thread zeros this and then increments it for each
        //      title it ignores due to missing required info, or other errors.
        //      The timer uses the last ignored count value to watch for changes
        //      so it can update the display.
        //
        //  m_c4LastTitleCnt
        //  m_c4TitleCnt
        //      The loading thread zeros and then increments the title count
        //      as it loads. In order to keep the display updated the GUI
        //      timer keeps a last count seen, and updates the display if
        //      they are different.
        //
        //  m_c4PlSerialNum
        //      When we make changes in any of the info that we collect to
        //      return to the player driver as status info, we bump this. This
        //      allows the driver to use a poll method for regular status
        //      polling. So we only return anything if there's new data.
        //
        //  m_colDBChanges
        //      Our handler for database change callbacks queues up change
        //      info here for our bgn thread to process. We use the sync mutex
        //      to sync access to this.
        //
        //  m_colPlCmdQ
        //      Incoming player commands are queued up here to be processed
        //      by the bgn command.
        //
        //  m_dbciTmp
        //      For use by the background thread to read events out of the
        //      database change queue, so he doesn't have to constantly create
        //      and destroy them.
        //
        //  m_eMediaType
        //      We only deal with music, so we just set a media type once
        //      and use it everywhere to avoid verbiage.
        //
        //  m_eLastStatus
        //  m_eStatus
        //      The iTunes thread keeps this up to date with its current
        //      status. The tab uses this to display status info, and the
        //      incoming ORB calls from the driver use it to know if they
        //      can get data from the database.
        //
        //      In order to know when to update the display, the GUI thread
        //      remembers the last status it displayed. If m_eStatus is
        //      different, then it needs to update.
        //
        //  m_eLoadStatus
        //      This is the status we return to our driver. It's not the current
        //      status really, it's whether we have data available. Once we have
        //      loaded once, then we have data available. We only swap in a new
        //      database (on a forced reload) if it loads successfully. So, once
        //      we load once, we always have data, and therefore this status must
        //      be different from the ones above, which indicate the current actual
        //      active status.
        //
        //  m_i4MainListId
        //  m_i4MainSrcId
        //      We remember the source and list id of the main library list.
        //      This way we can tell when any change events are referring
        //      to real tracks, or tracks referenced via a playlist. See
        //      the file level comments above.
        //
        //  m_mbufArtLoad
        //      A buffer to use for loading art images. We have to call the method
        //      repeatedly and we don't want to have to reallocate it every time.
        //      We'll read up to 32K of the image for this, so we preallocate the
        //      buffer to the needed size.
        //
        //  m_mdbLoad
        //      We do loads into this database. Only after it's successfully loaded
        //      do we copy it over into the database engine.
        //
        //  m_mdigMD5
        //  m_mhashUID
        //      These are used to create a unique id from an artist/album
        //      name combo (but much more compact.)
        //
        //  m_mtxSync
        //      We have to allow the driver access to the media database we
        //      maintain, so that incoming ORB based calls from the driver can
        //      get to the data it needs.
        //
        //  m_pathArtDir
        //      We pre-build the path that we store cover art in, since it's
        //      used in a number of places.
        //
        //  m_piTunes
        //      The main iTunes COM object that we create and keep around.
        //      This will cause iTunes to run, unfortunately, but nothing
        //      can be done about that. This is only accessed by the iTunes
        //      thread that we spin off.
        //
        //  m_piTunesEv
        //      An iTunes COM event handler we create. We hand it off to the iTunes
        //      stuff but need to keep a copy around since we still ostensibly own
        //      it. The id for the advise association is in m_c4EvAdviseId.
        //
        //  m_plcmdTmp
        //      A player command object to read in commands from the queue
        //      that incoming driver commands put commands on.
        //
        //  m_porbsIntf
        //      Our server side ORB object. The ORB owns it, but we keep this
        //      one around in case we need to interact with it.
        //
        //  m_pwndXXX
        //      Pointers to our child widgets. We load them from a dialog
        //      resource, and just want to get typed pointers to them to
        //      make it easier to interact with them.
        //
        //  m_srdbEng
        //      We use a standard repo database engine to manage the data and
        //      serve up media info to clients. However we do have to do a
        //      derivative to override the cover art loading callback.
        //
        //  m_strAllMusicCat
        //      We pre-load the all music category name for later use.
        //
        //  m_strCurTrackName
        //      This is kept up to date with the name of the currently
        //      playing track.
        //
        //  m_strDBSerialNum
        //      After a load we generate a DB serial number by flattening the
        //      new database and hashing the result. We have to serve this up to
        //      the driver.
        //
        //  m_strTimerTmp
        //      For use by the timer to use when updating the display
        //
        //  m_strThreadTmp
        //  m_strThreadTmp2
        //      For use by the bgn thread for temp formatting.
        //
        //  m_strmFmt
        //      A temp string output stream for use by the bgn thread for
        //      various purposes.
        //
        //  m_tmidUpdate
        //      Id for the update timer we start.
        //
        //  m_thriTunes
        //      This thread does all of the work of interfacing to iTunes and
        //      maintaining a media database in memory (on behalf of the repo
        //      driver that will be talking to us.)
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bCurMute;
        tCIDLib::TBoolean       m_bDBChanges;
        tCIDLib::TBoolean       m_bDelImages;
        tCIDLib::TBoolean       m_bForceReload;
        tCIDLib::TBoolean       m_bQuitNotification;
        tCIDLib::TCard4         m_c4EvAdviseId;
        tCIDLib::TCard4         m_c4LastFailedCnt;
        tCIDLib::TCard4         m_c4LastIgnoredCnt;
        tCIDLib::TCard4         m_c4IgnoredCnt;
        tCIDLib::TCard4         m_c4LastTitleCnt;
        tCIDLib::TCard4         m_c4CurVolume;
        tCIDLib::TCard4         m_c4PlSerialNum;
        tCIDLib::TCard4         m_c4TracksFailed;
        tCIDLib::TCard4         m_c4TracksIgnored;
        tCIDLib::TCard4         m_c4TitleCnt;
        TDBEventQ               m_colDBChanges;
        tCIDLib::TStringQ       m_colLogQ;
        TPlCmdQ                 m_colPlCmdQ;
        TDBQChangeInfo          m_dbciTmp;
        tCQCMedia::ELoadStatus  m_eLastStatus;
        tCQCMedia::ELoadStatus  m_eLoadStatus;
        tCQCMedia::EMediaTypes  m_eMediaType;
        TiTPlayerIntfServerBase::EPlStates  m_ePlState;
        tCQCMedia::ELoadStatus  m_eStatus;
        tCIDLib::TEncodedTime   m_enctCurPos;
        tCIDLib::TEncodedTime   m_enctCurTotal;
        tCIDLib::TInt4          m_i4MainListId;
        tCIDLib::TInt4          m_i4MainSrcId;
        THeapBuf                m_mbufArtLoad;
        TMediaDB                m_mdbLoad;
        TMessageDigest5         m_mdigMD5;
        TMD5Hash                m_mhashUID;
        TMutex                  m_mtxSync;
        TPathStr                m_pathArtDir;
        IiTunes*                m_piTunes;
        TITEventHandler*        m_piTunesEv;
        TPlayerCmd              m_plcmdTmp;
        TiTunesPlCtrlImpl*      m_porbsPlIntf;
        TiTunesRepoCtrlImpl*    m_porbsRepoIntf;
        TCheckBox*              m_pwndDelImages;
        TStaticText*            m_pwndFailedCnt;
        TPushButton*            m_pwndForceReload;
        TStaticText*            m_pwndIgnoredCnt;
        TTextOutWnd*            m_pwndLog;
        TStaticText*            m_pwndStatus;
        TStaticText*            m_pwndTitleCnt;
        TiTMediaRepoEng         m_srdbEng;
        TString                 m_strAllMusicCat;
        TString                 m_strCurAlbum;
        TString                 m_strCurArtist;
        TString                 m_strCurTrack;
        TString                 m_strDBSerialNum;
        TString                 m_strPlayListCat;
        TString                 m_strTimerTmp;
        TString                 m_strThreadTmp;
        TString                 m_strThreadTmp2;
        TString                 m_strThreadLogTmp;
        TString                 m_strThreadLogTmp2;
        TStreamFmt              m_strmfUID;
        TTextStringOutStream    m_strmFmt;
        TThread                 m_thriTunes;
        tCIDCtrls::TTimerId     m_tmidUpdate;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCTrayiTunesTab, TCQCTrayMonTab)
};


