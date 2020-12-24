//
// FILE NAME: SonosZPPropS_DriverImpl.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 05/14/2012
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
//  This is the header file for the actual driver class. Note that the
//  methods of this class, which is fairly large, are spread out over a few
//  different files.
//
// CAVEATS/GOTCHAS:
//
//  1)  WE MUST wait to turn on eventing on services at the end of the
//      connect method, after we do our initial query, so that we get
//      fully queried values initially. Then, we initialize our serial
//      numbers to the current ones on the services, then we enable
//      eventing. In the poll, for any that we have to poll even if they
//      haven't been marked as changed due to evented data, we will wait
//      until it indicates that it's gotten an initial load of event data.
//
//  2)  There's a bug in the Windows UPNP framework apparently in that
//      it does not seem to report content directory container update id
//      changes. So we can't be informed asynchronously that changes have
//      occurred. So, for now, we have to do a query of one item in order
//      to get the latest ID and see if it's changed.
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: SonosZPPropS
//  PREFIX: drv
// ---------------------------------------------------------------------------
class TSonosZPPropSDriver : public TCQCServerBase
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TSonosZPPropSDriver() = delete;

        TSonosZPPropSDriver
        (
            const   TCQCDriverObjCfg&       cqcdcToLoad
        );

        TSonosZPPropSDriver(const TSonosZPPropSDriver&) = delete;

        ~TSonosZPPropSDriver();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TSonosZPPropSDriver& operator=(const TSonosZPPropSDriver&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bQueryTextVal
        (
            const   TString&                strQueryType
            , const TString&                strDataName
            ,       TString&                strToFill
        );


    protected :
        // -------------------------------------------------------------------
        //  We use this enum to throw an internal 'service died' error, that
        //  is easy to distinguish. When this happens we just recycle our
        //  connection.
        // -------------------------------------------------------------------
        enum ESvcErrs
        {
            ESvcErr_Died
        };


        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bGetCommResource
        (
                    TThread&                thrThis
        );

        tCIDLib::TBoolean bWaitConfig
        (
                    TThread&                thrThis
        );

        tCQCKit::ECommResults eBoolFldValChanged
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4FldId
            , const tCIDLib::TBoolean       bNewValue
        );

        tCQCKit::ECommResults eCardFldValChanged
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4FldId
            , const tCIDLib::TCard4         c4NewValue
        );

        tCQCKit::ECommResults eConnectToDevice
        (
                    TThread&                thrThis
        );

        tCQCKit::ECommResults eFloatFldValChanged
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4FldId
            , const tCIDLib::TFloat8        f8NewValue
        );

        tCQCKit::EDrvInitRes eInitializeImpl();

        tCQCKit::ECommResults ePollDevice
        (
                    TThread&                thrThis
        );

        tCQCKit::ECommResults eStringFldValChanged
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4FldId
            , const TString&                strNewValue
        );

        tCIDLib::TVoid ReleaseCommResource();

        tCIDLib::TVoid TerminateImpl();


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCheckIsLocalGrp();

        tCIDLib::TBoolean bCheckParms
        (
            const tCIDLib::TStrCollect&     colParms
            , const tCIDLib::TCard4         c4ExpCnt
            , const TString&                strCmd
        );

        tCIDLib::TBoolean bFindNamedPL
        (
            const   TString&                strToFind
            ,       tCIDLib::TCard4&        c4PLNum
        );

        tCIDLib::TBoolean bValidatePLID
        (
            const   TString&                strPLID
            ,       tCIDLib::TCard4&        c4PLNum
        )   const;

        tCIDLib::TVoid CheckIsDead
        (
                    TUPnPService&           upnpsToCheck
        );

        tCIDLib::TVoid CheckFavsListReload();

        tCIDLib::TVoid CheckPLListReload();

        tCQCKit::ECommResults eInvokeCmd
        (
            const   TString&                strCmdData
        );

        tCQCKit::ECommResults ePlayFavorite
        (
            const   TString&                strName
        );

        tCIDLib::TVoid LoadFavoritesList();

        tCIDLib::TVoid LoadPlaylistsList();

        tCIDLib::TVoid LogNotCoord();

        tCIDLib::TVoid MonikerToUID
        (
            const   TString&                strMoniker
            ,       TString&                strToFill
        )   const;

        tCIDLib::TVoid RegisterFields();

        tCIDLib::TVoid ReleaseDevice
        (
                    TUPnPDevice&            upnpdToRelease
            , const tCIDLib::TCh* const     pszName
        );

        tCIDLib::TVoid ReleaseService
        (
                    TUPnPService&           upnpsToRelease
            , const tCIDLib::TCh* const     pszName
        );

        tCIDLib::TVoid StoreAVTrans
        (
            const   tCIDLib::TBoolean       bGetCurTime
        );

        tCIDLib::TVoid StoreContDir();

        tCIDLib::TVoid StoreDevProps();

        tCIDLib::TVoid StoreGrpMgmt();

        tCIDLib::TVoid StoreRendCtrl();

        tCIDLib::TVoid UPnPSetup();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4FinderSerialNum
        //      The last serial number we got from the async finder, which we
        //      can use to quickly see if there have been changes and so we
        //      might want to update our device and services.
        //
        //  m_c4FldId_XXX
        //      We look up the ids of our fields after registering them, so
        //      that we can do efficient 'by id' reads/writes.
        //
        //  m_c4SerialNum_XXX
        //      We turn eventing on on some of the services, and they will
        //      then bump a serial number any time a state variable changes,
        //      so we track the last serial number we got from them. This lets
        //      us quick check during polling if there's anything to do.
        //
        //  m_colPLList
        //      The last availble playlists list we downloaded. THis is
        //      set along with m_c4ActivePLLUpdateID when we need to reload
        //      the list. We only reload this if we need to use it and we
        //      see that it's changed.
        //
        //  m_conncfgUPnp
        //      The connection info we get in the ctor and store away for
        //      later use during comm resource acquisition. It will actually be
        //      a UPnP conn config type object.
        //
        //  m_enctNextAVQ
        //      In order to get current media position info, we have to do
        //      a full query of the position info. We only do this every two
        //      seconds. Otherwise, we only store AV transport info if the
        //      serial number has changed.
        //
        //  m_eTransState
        //      The last transport state we saw, so that we can know if it
        //      chnages.
        //
        //  m_i4ActivePLLUpdateID
        //  m_i4LastPLLUpdateID
        //      We maintain a current list of playlists available on the
        //      player. Actually we only update it if required. The active
        //      update id is the one we got with our last update of the
        //      list. The last update id is the last one that was reported
        //      to us. If they don't match, our list is out of date and
        //      need to reload it. We do an initial load on connect, and
        //      get both of them set to the same thing. Then we only
        //      reload it after that if we need to access it and it's
        //      changed. We don't reload just because it changed since it
        //      might not be accessed across numerous changes.
        //
        //  m_i4LastFavUpdateID
        //      The last update id for the favorites list. This is what we got the
        //      last time we loaded the list. Next time the user wants to play from
        //      the favorites or get the list, we see if the id has changed and
        //      reload the list if so.
        //
        //  m_i4LastQUpdateID
        //      The last update id we got for the current queue, which we
        //      track so that we can keep an up to date playlist available
        //      for query by clients.
        //
        //  m_strDevUID
        //      We get the unique id of the UPnP device from a driver prompt.
        //
        //  m_strLastArtURL
        //      The last artwork URL we saw, so that we know when it changes.
        //
        //  m_strTmp1
        //  m_strTmp2
        //      Some temp strings for use in storing fields and such.
        //
        //  m_upnpdZP
        //  m_upnpdMR
        //  m_upnpdMS
        //      The devices we have to keep track of. We have the main zone
        //      player root object, and then the media renderer and mesia
        //      server sub-devices.
        //
        //  m_upnpsXXX
        //      These are the services we set up from the devices above. These
        //      get us the info and control we need.
        //
        //  m_upnptiMeta
        //      A UPnP track item we'll use when storing position info, which
        //      returns us current metadata. This way we don't have to costantly
        //      create and destroy one.
        // -------------------------------------------------------------------
        tCIDLib::TCard4         m_c4FinderSerialNum;
        tCIDLib::TCard4         m_c4FldId_ArtURL;
        tCIDLib::TCard4         m_c4FldId_AVTransURI;
        tCIDLib::TCard4         m_c4FldId_ClearQ;
        tCIDLib::TCard4         m_c4FldId_CrossfadeMode;
        tCIDLib::TCard4         m_c4FldId_CurAlbum;
        tCIDLib::TCard4         m_c4FldId_CurArtist;
        tCIDLib::TCard4         m_c4FldId_CurTrack;
        tCIDLib::TCard4         m_c4FldId_CurTrackDur;
        tCIDLib::TCard4         m_c4FldId_CurTrackNum;
        tCIDLib::TCard4         m_c4FldId_CurTrackTime;
        tCIDLib::TCard4         m_c4FldId_CurTrackURI;
//        tCIDLib::TCard4         m_c4FldId_DevName;
        tCIDLib::TCard4         m_c4FldId_GroupCoord;
//        tCIDLib::TCard4         m_c4FldId_Icon;
        tCIDLib::TCard4         m_c4FldId_InvokeCmd;
//        tCIDLib::TCard4         m_c4FldId_IsBridge;
        tCIDLib::TCard4         m_c4FldId_IsLocalGroup;
        tCIDLib::TCard4         m_c4FldId_Mute;
        tCIDLib::TCard4         m_c4FldId_PBPercent;
        tCIDLib::TCard4         m_c4FldId_PlayMode;
//        tCIDLib::TCard4         m_c4FldId_SetLED;
        tCIDLib::TCard4         m_c4FldId_ShuffleMode;
        tCIDLib::TCard4         m_c4FldId_Transport;
        tCIDLib::TCard4         m_c4FldId_TransState;
        tCIDLib::TCard4         m_c4FldId_Volume;

        tCIDLib::TCard4         m_c4SerialNum_AVTrans;
        tCIDLib::TCard4         m_c4SerialNum_ContDir;
        tCIDLib::TCard4         m_c4SerialNum_DevProps;
        tCIDLib::TCard4         m_c4SerialNum_GrpMgmt;
        tCIDLib::TCard4         m_c4SerialNum_RendCtrl;

        tCIDLib::TKValsList     m_colFavsList;
        tCIDLib::TKVPList       m_colPLList;

        TCQCUPnPConnCfg         m_conncfgUPnP;
        tCIDLib::TEncodedTime   m_enctNextAVQ;
        tCIDLib::TInt4          m_i4ActivePLLUpdateID;
        tCIDLib::TInt4          m_i4LastPLLUpdateID;
        tCIDLib::TInt4          m_i4LastFavsUpdateID;
        tCIDLib::TInt4          m_i4LastQUpdateID;

        TString                 m_strDevUID;
        TString                 m_strLastArtURL;
        TString                 m_strTmp1;
        TString                 m_strTmp2;
        TString                 m_strTmp3;

        TUPnPDevice             m_upnpdZP;
        TUPnPDevice             m_upnpdMedRend;
        TUPnPDevice             m_upnpdMedSrv;

        // TUPnPDevPropsService    m_upnpsDevProps;
        TUPnPGrpMgmtService     m_upnpsGrpMgmt;
        TUPnPAVTransService     m_upnpsAVTrans;
        TUPnPRendCtrlService    m_upnpsRendCtrl;
        TUPnPContDirService     m_upnpsContDir;

        TUPnPTrackItem          m_upnptiMeta;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TSonosZPPropSDriver,TCQCServerBase)
};

#pragma CIDLIB_POPPACK


