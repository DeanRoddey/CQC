//
// FILE NAME: UPnPMediaRendS_DriverImpl.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/18/2011
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
//   CLASS: UPnPMediaRendS
//  PREFIX: drv
// ---------------------------------------------------------------------------
class TUPnPMediaRendSDriver : public TCQCStdMediaRendDrv
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TUPnPMediaRendSDriver
        (
            const   TCQCDriverObjCfg&       cqcdcToLoad
        );

        ~TUPnPMediaRendSDriver();


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

        tCIDLib::TBoolean bWaitConfig
        (
                    TThread&                thrThis
        );

        tCQCKit::ECommResults eAdjustVolume
        (
            const   tCIDLib::TBoolean       bUp
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

        tCQCKit::ECommResults eDoTransportCmd
        (
            const   tCQCMedia::ETransCmds   eToSet
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

        tCQCKit::ECommResults eStringFldValChanged
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4FldId
            , const TString&                strNewValue
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


    private :
        // -------------------------------------------------------------------
        //  Unimplemented constructors and operators
        // -------------------------------------------------------------------
        TUPnPMediaRendSDriver();
        TUPnPMediaRendSDriver(const TUPnPMediaRendSDriver&);
        tCIDLib::TVoid operator=(const TUPnPMediaRendSDriver&);



        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid QueryPosInfo();

        tCIDLib::TVoid QueryRendCtrlInfo();

        tCIDLib::TVoid QueryTransInfo();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bDoDBVolume
        //      A fixed driver prompt tells us whether to use db or vendor
        //      specific volume. If the former, we query the range and store
        //      those values in m_i4MaxVol/m_i4MinVol. If the latter, we get
        //      the range values as part of the prompt and set those range
        //      members during init.
        //
        //  m_bFlipSeps
        //      A fixed driver prompt can tell us whether we should flip back
        //      slashes to forward slashes when we send location info from the
        //      media repo to the player.
        //
        //  m_bMute
        //      The last mute state we got from the device
        //
        //  m_bRemoveUNCSlashes
        //      A fixed driver prompt can tell us whether we should remove the \\
        //      leading UNC name slashes from the location info before formatting
        //      it into the manifest provided pattern and sendng it.
        //
        //  m_c4FldId_XXX
        //      We look up the ids of our fields after registering them, so
        //      that we can do efficient 'by id' reads/writes. Most of them are
        //      handled by our standard renderer base class.
        //
        //  m_c4Volume
        //      The last volume we got from the device.
        //
        //  m_conncfgUPnp
        //      The connection info we get in the ctor and store away for
        //      later use during comm resource acquisition. It will actually be
        //      a UPnP conn config type object.
        //
        //  m_enctCurPos
        //  m_enctCurTotal
        //      The last media position time and total current media time
        //      we got during polling.
        //
        //  m_enctLastPosQ
        //  m_enctLastRCQ
        //  m_enctLastTransQ
        //      The last time we polled for various bits of info that we
        //      poll for.
        //
        //  m_enctWaitStart
        //      When we are asked to start a new piece of media playing, we have
        //      to wait first for either timeout because it never started playing,
        //      or for it to go into something besides stopped. Otherwise, we'll
        //      immediately report stopped (because we'll probably poll it again
        //      before it gets a chance to start playing.) So we set this and our
        //      player status callback will return loading status until we hit this
        //      time our we get a non-stopped status.
        //
        //  m_eEndState
        //  m_eState
        //      The last media state and end state that we set during polling
        //      of status. This is info we have to return to the parent
        //      class.
        //
        //  m_i4MaxVol
        //  m_i4MinVol
        //      UPnP devices have their own volume settings, which we have
        //      to query and store, so that we can convert between that and
        //      our percentage view of the world.
        //
        //  m_strDevUID
        //      We get the unique id of the UPnP device from a driver
        //      prompt.
        //
        //  m_strLocInfoPattern
        //      We get a pattern value (with an %(L) token in it) from a
        //      driver install prompt. This let's the location value we
        //      send out be formatted in various ways to suit the needs
        //      of the target device.
        //
        //  m_upnpdUPnPMediaRend
        //      Our UPnP device object that is set up to talk to the
        //      UPnPMediaRend driver. We get this first, and use it to get
        //      the services below.
        //
        //  m_upnpsAVTrans
        //  m_upnpsRendCtrl
        //      The two UPnP service objects that provide the control interface
        //      to the player.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bDoDBVolume;
        tCIDLib::TBoolean       m_bFlipSeps;
        tCIDLib::TBoolean       m_bMute;
        tCIDLib::TBoolean       m_bRemoveUNCSlashes;
        tCIDLib::TCard4         m_c4FldId_SetLocInfoPat;
        tCIDLib::TCard4         m_c4PollRound;
        tCIDLib::TCard4         m_c4Volume;
        TCQCUPnPConnCfg         m_conncfgUPnP;
        tCIDLib::TEncodedTime   m_enctCurPos;
        tCIDLib::TEncodedTime   m_enctCurTotal;
        tCIDLib::TEncodedTime   m_enctLastPosQ;
        tCIDLib::TEncodedTime   m_enctLastRCQ;
        tCIDLib::TEncodedTime   m_enctLastTransQ;
        tCIDLib::TEncodedTime   m_enctWaitStart;
        tCQCMedia::EEndStates   m_eEndState;
        tCQCMedia::EMediaStates m_eState;
        tCIDLib::TInt4          m_i4MaxVol;
        tCIDLib::TInt4          m_i4MinVol;
        TString                 m_strDevUID;
        TString                 m_strLocInfoPattern;
        TString                 m_strTmpPoll;
        TUPnPDevice             m_upnpdUPnPMediaRend;
        TUPnPAVTransService     m_upnpsAVTrans;
        TUPnPRendCtrlService    m_upnpsRendCtrl;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TUPnPMediaRendSDriver,TCQCStdMediaRendDrv)
};

#pragma CIDLIB_POPPACK


