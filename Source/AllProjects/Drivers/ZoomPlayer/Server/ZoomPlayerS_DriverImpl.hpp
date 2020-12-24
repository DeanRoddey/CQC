//
// FILE NAME: ZoomPlayerS_DriverImpl.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/16/2005
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
//   CLASS: ZoomPlayerS
//  PREFIX: drv
// ---------------------------------------------------------------------------
class TZoomPlayerSDriver : public TCQCStdMediaRendDrv
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TZoomPlayerSDriver
        (
            const   TCQCDriverObjCfg&       cqcdcToLoad
        );

        ~TZoomPlayerSDriver();


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

        tCQCKit::ECommResults eFloatFldValChanged
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4FldId
            , const tCIDLib::TFloat8        f8NewValue
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

        tCIDLib::TVoid UpdateMetaFields
        (
            const   TCQCMediaPLItem&        mpliNew
        );


    private :
        // -------------------------------------------------------------------
        //  Unimplemented constructors and operators
        // -------------------------------------------------------------------
        TZoomPlayerSDriver();
        TZoomPlayerSDriver(const TZoomPlayerSDriver&);
        tCIDLib::TVoid operator=(const TZoomPlayerSDriver&);


        // -------------------------------------------------------------------
        //  Private class types
        // -------------------------------------------------------------------
        enum ETimes
        {
            ETime_Total
            , ETime_Current
            , ETime_Both
        };


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bGetMsg
        (
                    tCIDLib::TCard4&        c4CmdId
            ,       TString&                strParms
            , const tCIDLib::TCard4         c4WaitFor
        );

        tCIDLib::TCard4 c4ExtractTimeFld
        (
            const   TString&                strTime
            ,       tCIDLib::TCard4&        c4Ofs
        )   const;

        tCIDLib::TEncodedTime enctConvertTime
        (
            const   TString&                strTime
        )   const;

        tCIDLib::TVoid DoPlayURL
        (
            const   TString&                strURL
        );

        tCIDLib::TVoid ProcessMsg
        (
            const   tCIDLib::TCard4         c4CmdId
            , const TString&                strParms
        );

        tCIDLib::TVoid ProcessMsgs();

        tCIDLib::TVoid SendCmd
        (
            const   tCIDLib::TCard4         c4ToSend
        );

        tCIDLib::TVoid SendCmd
        (
            const   tCIDLib::TCard4         c4ToSend
            , const TString&                strParms
        );

        tCIDLib::TVoid SendCmd
        (
            const   tCIDLib::TCard4         c4ToSend
            , const tCIDLib::TCh* const     pszParms
        );

        tCIDLib::TVoid SendExFunc
        (
            const   TString&                strFunc
            , const tCIDLib::TCard4         c4Value
        );

        tCIDLib::TVoid SendFunc
        (
            const   TString&                strFunc
        );

        tCIDLib::TVoid StoreTimeline
        (
            const   TString&                strValue
            , const ETimes                  eToStore
        );

        tCIDLib::TVoid WaitForFuncAck
        (
            const   tCIDLib::TCard4         c4CmdId
            , const TString&                strFunc
            , const tCIDLib::TCard4         c4WaitFor
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bMute
        //      The last mute state we got from the device.
        //
        //  m_c4FldId_XXX
        //      We look up the ids of our fields after registering them, so
        //      that we can do efficient 'by id' reads/writes.
        //
        //  m_c4Volume
        //      The last volume we got from the device.
        //
        //  m_conncfgSock
        //      The connection info we get in the ctor and store away for
        //      later use during comm resource acquisition.
        //
        //  m_enctCurPos
        //  m_enctCurTotal
        //      The last media position and media length we stored, for
        //      giving back to the parent class when he asks.
        //
        //  m_eEndState
        //      The last end of media state we stored.
        //
        //  m_eMediaType
        //      The media type of the currently playing item.
        //
        //  m_eState
        //      The last media state we stored.
        //
        //  m_ePlayMode
        //      The current play mode, which we get from the player. We keep
        //      up with this because some things have different meanings
        //      according to the mode.
        //
        //  m_mbufIn
        //      A memory buffer to get incoming message into, from which
        //      we then transcode to text for parsing.
        //
        //  m_sockConn
        //      The socket we use for our communications with the player.
        //
        //  m_strLocInfoPattern
        //      Clients can send us a pattern string, via the SetLocInfoPattern
        //      field which we store here. It has to have a %(L) token in
        //      it. Any time we send a location info to the player, we see
        //      if this pattern is non-empty. If so, we take this and then
        //      replace the token with the actual location info.
        //
        //  m_strIn
        //  m_strOut
        //  m_strPoll
        //  m_strTmp
        //  m_strTmp2
        //      Used in reading/write messages, for efficiency, so we don't
        //      have to construct one every time. The poll string is used
        //      in the ePoll method. Tmps are for immediate use for whatever
        //      purpose, so don't expect it to remain valid across any
        //      call. The others are used the core message I/O methods in
        //      the DriverImpl2 file.
        //
        //  m_tcvtIO
        //      The protocol uses UTF-8 so we need a transcoder to transcode
        //      between our Unicode format.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean           m_bMute;
        tCIDLib::TCard4             m_c4FldId_AudioTrack;
        tCIDLib::TCard4             m_c4FldId_Navigation;
        tCIDLib::TCard4             m_c4FldId_PlayMode;
        tCIDLib::TCard4             m_c4FldId_PlayURL;
        tCIDLib::TCard4             m_c4FldId_PlayerVer;
        tCIDLib::TCard4             m_c4FldId_ScreenMode;
        tCIDLib::TCard4             m_c4FldId_SetAR;
        tCIDLib::TCard4             m_c4FldId_SetLocInfoPat;
        tCIDLib::TCard4             m_c4FldId_ToggleNext;
        tCIDLib::TCard4             m_c4Volume;
        TCQCIPConnCfg               m_conncfgSock;
        tCIDLib::TEncodedTime       m_enctCurPos;
        tCIDLib::TEncodedTime       m_enctCurTotal;
        tCQCMedia::EEndStates       m_eEndState;
        tCQCMedia::EMediaTypes      m_eMediaType;
        tCQCMedia::EMediaStates     m_eState;
        tZoomPlayerS::EPlayModes    m_ePlayMode;
        THeapBuf                    m_mbufIn;
        TClientStreamSocket         m_sockConn;
        TString                     m_strLocInfoPattern;
        TString                     m_strIn;
        TString                     m_strOut;
        TString                     m_strPoll;
        TString                     m_strTmp;
        TString                     m_strTmp2;
        TString                     m_strTmpOut;
        TUTF8Converter              m_tcvtIO;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TZoomPlayerSDriver,TCQCStdMediaRendDrv)
};

#pragma CIDLIB_POPPACK


