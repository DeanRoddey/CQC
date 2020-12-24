//
// FILE NAME: iTunesRendTMS_DriverImpl.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/05/2015
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
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: iTunesRendTMS
//  PREFIX: sdrv
// ---------------------------------------------------------------------------
class TiTunesRendTMSDriver : public TCQCStdMediaRendDrv
{
    public :
        // -------------------------------------------------------------------
        //  Create a type short cut
        // -------------------------------------------------------------------
        typedef TiTPlayerIntfClientProxy    TPlBase;


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TiTunesRendTMSDriver
        (
            const   TCQCDriverObjCfg&       cqcdcToLoad
        );

        ~TiTunesRendTMSDriver();


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bGetCommResource
        (
                    TThread&                thrThis
        );

        tCIDLib::TBoolean bPlayNewItem
        (
            const   TCQCMediaPLItem&        mpliNew
        );

        tCIDLib::TVoid ClearMetaFields();

        tCQCKit::ECommResults eAdjustVolume
        (
            const   tCIDLib::TBoolean       bUp
        );

        tCQCKit::ECommResults eDoTransportCmd
        (
            const   tCQCMedia::ETransCmds   eToSet
        );

        tCQCKit::ECommResults eCheckPlayerStatus
        (
                    tCIDLib::TEncodedTime&  enctCurPos
            ,       tCIDLib::TEncodedTime&  enctTotal
            ,       tCQCMedia::EMediaStates& eState
            ,       tCQCMedia::EEndStates&  eEndState
            ,       tCIDLib::TBoolean&      bMute
            ,       tCIDLib::TCard4&        c4Volume
        );

        tCQCKit::ECommResults eConnectToDevice
        (
                    TThread&                thrThis
        );

        tCQCKit::EDrvInitRes eInitializeImpl();

        tCQCKit::ECommResults eSetMute
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCQCKit::ECommResults eSetVolume
        (
            const   tCIDLib::TCard4         c4ToSet
        );

        tCIDLib::TVoid LoadDrvSpecificFlds
        (
                    TVector<TCQCFldDef>&    colFlds
        );

        tCIDLib::TVoid LookupDrvSpecificFlds
        (
            const   tCIDLib::TCard4         c4ArchVersion
        );

        tCIDLib::TVoid ReleaseCommResource();

        tCIDLib::TVoid ResetPlayer();

        tCIDLib::TVoid TerminateImpl();

        tCIDLib::TVoid UpdateMetaFields
        (
            const   TCQCMediaPLItem&        mpliNew
        );


    private :
        // -------------------------------------------------------------------
        //  Unimplemented constructors and operators
        // -------------------------------------------------------------------
        TiTunesRendTMSDriver();
        TiTunesRendTMSDriver(const TiTunesRendTMSDriver&);
        tCIDLib::TVoid operator=(const TiTunesRendTMSDriver&);


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid ResetState();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bMute
        //      The last mute state we got from the player.
        //
        //  m_c4MuteVolume
        //      The stupid player drops the volume to zero when you mute it
        //      and doesn't restore it when you unmute. So we always keep this
        //      up to date when the last non-zero volume we saw. When we get an
        //      unmute, we set this volume back.
        //
        //  m_c4SerialNum
        //      We pass this in to each player status query, so that it knows
        //      where we have the latest data already or not. it updates it with
        //      the latest value if there are changes.
        //
        //  m_c4Volume
        //      The last volume we got from the player.
        //
        //  m_ePlState
        //      The last player state we saw from the player.
        //
        //  m_eState
        //      The last media state we saw from the player.
        //
        //  m_enctCurPos
        //  m_enctTotal
        //      The last media total and current time we got from the player, if nothing
        //      is playing total should be zero.
        //
        //  m_porbcTray
        //      The client proxy to the tray monitor that we use for our
        //      communications.
        //
        //  m_strPollTmpX
        //      Some strings for the poll method to use to grab data from the
        //      tray monitor.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean           m_bMute;
        tCIDLib::TCard4             m_c4MuteVolume;
        tCIDLib::TCard4             m_c4SerialNum;
        tCIDLib::TCard4             m_c4Volume;
        tCIDLib::TEncodedTime       m_enctCurPos;
        tCIDLib::TEncodedTime       m_enctTotal;
        tCQCMedia::EEndStates       m_eEndState;
        TPlBase::EPlStates          m_ePlState;
        tCQCMedia::EMediaStates     m_eState;
        TiTPlayerIntfClientProxy*   m_porbcTray;
        TString                     m_strBinding;
        TString                     m_strPollTmp1;
        TString                     m_strPollTmp2;
        TString                     m_strPollTmp3;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TiTunesRendTMSDriver,TCQCStdMediaRendDrv)
};

#pragma CIDLIB_POPPACK


