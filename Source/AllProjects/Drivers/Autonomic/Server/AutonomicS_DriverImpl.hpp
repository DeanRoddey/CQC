//
// FILE NAME: AutonomicS_DriverImpl.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/07/2007
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
//  This is the header file for the actual driver class. We are really just
//  a passthrough for media data so we have no media database like most
//  repository drivers would.
//
//  We get our art via web server, so we have to provide a small derivative
//  of the standard media repo engine and override the art loading method.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//  CLASS: TAutonomicMediaRepoEng
// PREFIX: srdb
// ---------------------------------------------------------------------------
class TAutonomicMediaRepoEng : public TCQCStdMediaRepoEng
{
    public  :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TAutonomicMediaRepoEng
        (
            const   TString&                strMoniker
            , const tCQCMedia::EMTFlags     eSupportedTypes
            , const TIPEndPoint&            ipepSrv
        );

        ~TAutonomicMediaRepoEng();


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TCard4 c4LoadCoverArt
        (
            const   TMediaImg&              mimgToLoad
            ,       TMemBuf&                mbufToFill
            , const tCIDLib::TBoolean       bLarge
        );


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_ipepSrv
        //      The driver gives us the end point of the web server, which we
        //      need for art downloading.
        // -------------------------------------------------------------------
        TIPEndPoint m_ipepSrv;
};



// ---------------------------------------------------------------------------
//   CLASS: TAutonomicSDriver
//  PREFIX: drv
// ---------------------------------------------------------------------------
class TAutonomicSDriver : public TCQCStdMediaRepoDrv
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TAutonomicSDriver
        (
            const   TCQCDriverObjCfg&       cqcdcToLoad
        );

        ~TAutonomicSDriver();



    protected :
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

        tCIDLib::TCard4 c4LoadCoverArt
        (
            const   TString&                strArtPath
            ,       TMemBuf&                mbufToFill
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
        //  Unimplemented constructors and operators
        // -------------------------------------------------------------------
        TAutonomicSDriver();
        TAutonomicSDriver(const TAutonomicSDriver&);
        tCIDLib::TVoid operator=(const TAutonomicSDriver&);


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bDoPlayMedia
        (
            const   TString&                strToPlay
            , const tCIDLib::TBoolean       bEnqueue
        );

        tCIDLib::TBoolean bGetMsg
        (
                    TString&                strToFill
            , const tCIDLib::TEncodedTime   enctEnd
        );

        tCIDLib::TBoolean bLoadGenres();

        tCIDLib::TBoolean bParseAlbumText
        (
            const   TString&                strToParse
            ,       TString&                strGUID
            ,       TString&                strTitle
            ,       TString&                strSortTitle
            ,       TString&                strArtistFN
            ,       TString&                strArtistLN
        );

        tCIDLib::TBoolean bLoadTitles();

        tCIDLib::TBoolean bMonitorLoad();

        tCIDLib::TBoolean bProcessMsg
        (
            const   TString&                strToHandle
        );

        tCIDLib::TBoolean bSendAndWaitForRep
        (
            const   TString&                strToSend
            , const TString&                strRepPrefix
            , const tCIDLib::TCard4         c4WaitFor
            ,       TString&                strToFill
            , const tCIDLib::TBoolean       bThrowIfNot = kCIDLib::False
        );

        tCIDLib::TBoolean bWaitForReply
        (
            const   TString&                strReplyPref
            , const tCIDLib::TCard4         c4WaitFor
            ,       TString&                strToFill
            , const tCIDLib::TBoolean       bThrowIfNot = kCIDLib::False
        );

        tCQCKit::ECommResults eInitFields();

        tCIDLib::EExitCodes eLoaderThread
        (
                    TThread&                thrThis
            ,       tCIDLib::TVoid*         pData
        );

        tCIDLib::TVoid SendMsg
        (
            const   TString&                strToSend
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4FldId_XXX
        //      We look up the ids of our fields after registering them, so
        //      that we can do efficient 'by id' reads/writes.
        //
        //  m_c4TitlesDone
        //      During the loading process the loading thread keeps this up
        //      to date to indicate how many titles have been loaded.
        //
        //  m_conncfgSock
        //      Our connection configuration info that we store away in the
        //      ctor for later use.
        //
        //  m_eConnStatus
        //      We have to go throug a few steps to get connected, and we
        //      do it asynchronously via a loader thread. It updates this
        //      field to let the Connect() callback know when it is done and
        //      has all the data ready.
        //
        //  m_enctLastMsg
        //      We set this every time our low level message reader gets
        //      a new message. The poll method will do a proactive ping
        //      every so often if no messages have arrived.
        //
        //  m_enctNextLoad
        //      If the load is failing, we cannot just keep banging away
        //      relentlessly. So we use a timer to remember the next available
        //      time to do a load, which will be the time after the good or
        //      bad load plus some number of seconds.
        //
        //  m_mbufSend
        //      A temp buffer that is used by SendMsg(), to flatten messages
        //      into before sending.
        //
        //  m_mdbLoad
        //      We load into this offline database and then store the results
        //      on our parent class, so that we don't keep the driver locked
        //      during the load.
        //
        //  m_strMCEInstance
        //      We get the name of a particular MCE instance to connect to
        //      from our driver configuration.
        //
        //  m_sockConn
        //      Our socket that we use to talk to the Autonomic server. We
        //      also have to talk to a web server for cover art, but we do
        //      that via an HTTP client object and don't keep it around.
        //
        //  m_srdbEngine
        //      We use a standard repo database engine to provide the database
        //      and serving up data to media data clients.
        //
        //  m_strLoadMsg
        //      We set a field to a status message during the loading process
        //      and it must be translatable, so we pre-load it to avoid having
        //      to reload it all the time.
        //
        //  m_strMsg_XXX
        //      We set up some message prefix strings up front, to be more
        //      efficient. These include any leading spaces so they reflect
        //      the nesting of the messages. This makes it easier for us to
        //      tell nested list oriented msgs from top level async msgs that
        //      might show up while the list is being processed. We use them
        //      for sending and for checking incoming. The ones we send out
        //      are never nested ones, so there's no conflict.
        //
        //  m_strPollTmp
        //      A string for the poll callback to use, so that he doesn't
        //      have to constantly create and destroy them.
        //
        //  m_strVal_XXX
        //      Some pre-setup strings for values we commonly look for, to
        //      make it more efficient.
        //
        //  m_tcvtIO
        //      We need to transcode outgoing strings to ASCII for sending.
        //
        //  m_thrLoader
        //      We start this guy up on the eLoaderThread() method to do async
        //      loading of the media metadata from MCE into our database.
        // -------------------------------------------------------------------
        tCIDLib::TCard2             m_c2NextId_Cat;
        tCIDLib::TCard2             m_c2NextId_Title;
        tCIDLib::TCard4             m_c4FldId_AdjVolume;
        tCIDLib::TCard4             m_c4FldId_AdjChannel;
        tCIDLib::TCard4             m_c4FldId_CIDName;
        tCIDLib::TCard4             m_c4FldId_CIDNumber;
        tCIDLib::TCard4             m_c4FldId_CurColArtist;
        tCIDLib::TCard4             m_c4FldId_CurColName;
        tCIDLib::TCard4             m_c4FldId_CurItemTotal;
        tCIDLib::TCard4             m_c4FldId_CurItemTime;
        tCIDLib::TCard4             m_c4FldId_EnqueueMedia;
        tCIDLib::TCard4             m_c4FldId_Loaded;
        tCIDLib::TCard4             m_c4FldId_MediaState;
        tCIDLib::TCard4             m_c4FldId_Mute;
        tCIDLib::TCard4             m_c4FldId_Navigation;
        tCIDLib::TCard4             m_c4FldId_PlayMedia;
        tCIDLib::TCard4             m_c4FldId_ReloadDB;
        tCIDLib::TCard4             m_c4FldId_Running;
        tCIDLib::TCard4             m_c4FldId_ScreenNavigation;
        tCIDLib::TCard4             m_c4FldId_Status;
        tCIDLib::TCard4             m_c4FldId_TitleCnt;
        tCIDLib::TCard4             m_c4FldId_Transport;
        tCIDLib::TCard4             m_c4FldId_Volume;
        tCIDLib::TCard4             m_c4TitlesDone;
        TCQCIPConnCfg               m_conncfgSock;
        tAutonomicS::EConnStatus    m_eConnStatus;
        tCIDLib::TEncodedTime       m_enctLastMsg;
        tCIDLib::TEncodedTime       m_enctNextLoad;
        THeapBuf                    m_mbufSend;
        TAutonomicMediaRepoEng      m_srdbEngine;
        TString                     m_strMCEInstance;
        TString                     m_strMsg_Album;
        TString                     m_strMsg_BannerTerm;
        TString                     m_strMsg_BeginAlbums;
        TString                     m_strMsg_BeginGenres;
        TString                     m_strMsg_BrowseAlbums;
        TString                     m_strMsg_BrowseGenres;
        TString                     m_strMsg_ClearMusicFilter;
        TString                     m_strMsg_EndAlbums;
        TString                     m_strMsg_EndGenres;
        TString                     m_strMsg_Events;
        TString                     m_strMsg_Genre;
        TString                     m_strMsg_GetMCEStatus;
        TString                     m_strMsg_MediaFilter;
        TString                     m_strMsg_MediaName;
        TString                     m_strMsg_MediaState;
        TString                     m_strMsg_More;
        TString                     m_strMsg_MusicFilter;
        TString                     m_strMsg_Mute;
        TString                     m_strMsg_Navigate;
        TString                     m_strMsg_NoMore;
        TString                     m_strMsg_Ping;
        TString                     m_strMsg_PlayAlbum;
        TString                     m_strMsg_Pong;
        TString                     m_strMsg_ReportState;
        TString                     m_strMsg_SendKeys;
        TString                     m_strMsg_SendRemote;
        TString                     m_strMsg_SetMediaFilter;
        TString                     m_strMsg_StartMCE;
        TString                     m_strMsg_StateChanged;
        TString                     m_strMsg_SubscribeOn;
        TString                     m_strMsg_Trans_Next;
        TString                     m_strMsg_Trans_Play;
        TString                     m_strMsg_Trans_Prev;
        TString                     m_strMsg_Trans_Random;
        TString                     m_strMsg_Trans_Repeat;
        TString                     m_strMsg_Trans_Stop;
        TString                     m_strMsg_VolumeDown;
        TString                     m_strMsg_VolumeUp;
        TString                     m_strVal_False;
        TString                     m_strVal_True;
        TString                     m_strPollTmp;
        TClientStreamSocket         m_sockConn;
        TString                     m_strLoadMsg;
        TUTF8Converter              m_tcvtIO;
        TThread                     m_thrLoader;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TAutonomicSDriver,TCQCStdMediaRepoDrv)
};

#pragma CIDLIB_POPPACK


