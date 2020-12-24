//
// FILE NAME: CQCGWSrv_WorkerThread.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/27/2002
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
//  This is the header for the class that implements the worker threasd of
//  our thread farm. We use a single thread per client in this gateway, in
//  order to simplify things.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TWorkerThread
//  PREFIX: thr
// ---------------------------------------------------------------------------
class TWorkerThread : public TThread
{
    public :
        // --------------------------------------------------------------------
        // Constructors and Destructors
        // --------------------------------------------------------------------
        TWorkerThread() = delete;

        TWorkerThread
        (
            const   TString&                        strName
        );

        TWorkerThread(const TWorkerThread&) = delete;
        TWorkerThread(TWorkerThread&&) = delete;

        ~TWorkerThread();


        // --------------------------------------------------------------------
        //  Public operators
        // --------------------------------------------------------------------
        TWorkerThread& operator=(const TWorkerThread&) = delete;
        TWorkerThread& operator=(TWorkerThread&&) = delete;


    protected :
        // --------------------------------------------------------------------
        //  Protected, inherited methods
        // --------------------------------------------------------------------
        tCIDLib::EExitCodes eProcess() final;


    private :
        // --------------------------------------------------------------------
        //  Class types
        // --------------------------------------------------------------------
        using TFldList = TVector<TCQCFldPollInfo>;


        // --------------------------------------------------------------------
        //  Private, non-virtual methods
        // --------------------------------------------------------------------
        tCIDLib::TBoolean bGetMsg
        (
            const   tCIDLib::TCard4         c4Timeout
        );

        tCIDLib::TBoolean bLogonSeq();

        tCIDLib::TVoid CheckHeader
        (
            const   tCQCGWSrv::TPacketHdr&  hdrToCheck
        )   const;

        tCIDLib::TVoid DoGlobalAct
        (
            const   TXMLTreeElement&        xtnodeReq
        );

        tCIDLib::TVoid FormatBadValue
        (
                    TTextOutStream&         strmTarget
            , const TCQCFldPollInfo&        cfpiSrc
            , const tCIDLib::TCard4         c4PLIndex
        );

        tCIDLib::TVoid FormatChallengeData
        (
            const   TCQCSecChallenge&       seccToFormat
            ,       TString&                strKey
            ,       TString&                strChallenge
            ,       TString&                strSessKey
        );

        tCIDLib::TVoid FormatGoodValue
        (
                    TTextOutStream&         strmTarget
            , const TCQCFldPollInfo&        cfpiSrc
            , const tCIDLib::TCard4         c4PLIndex
        );

        tCIDLib::TVoid FormatSchEv
        (
            const   TString&                strPath
            , const TCQCSchEvent&           csrcSrc
            ,       TTextOutStream&         strmTar
        );

        tCIDLib::TVoid MWriteField
        (
            const   TXMLTreeElement&        xtnodeReq
        );

        const tCIDLib::TCh* pszMapFldAccess
        (
            const   tCQCKit::EFldAccess     eToMap
        )   const;

        tCIDLib::TVoid PauseSchEv
        (
            const   TXMLTreeElement&        xtnodeReq
        );

        tCIDLib::TVoid PollFields();

        tCIDLib::TVoid QueryDrvList();

        tCIDLib::TVoid QueryDrvInfo
        (
            const   TXMLTreeElement&        xtnodeReq
        );

        tCIDLib::TVoid QueryDrvStatus
        (
            const   TXMLTreeElement&        xtnodeReq
        );

        tCIDLib::TVoid QueryDrvText
        (
            const   TXMLTreeElement&        xtnodeReq
        );

        tCIDLib::TVoid QueryFldInfo
        (
            const   TXMLTreeElement&        xtnodeReq
        );

        tCIDLib::TVoid QueryFldInfoList
        (
            const   TXMLTreeElement&        xtnodeReq
        );

        tCIDLib::TVoid QueryFields();

        tCIDLib::TVoid QueryGlobalActs();

        tCIDLib::TVoid QueryImage
        (
            const   TXMLTreeElement&        xtnodeReq
        );

        tCIDLib::TVoid QueryMacros();

        tCIDLib::TVoid QueryMediaArt
        (
            const   TXMLTreeElement&        xtnodeReq
        );

        tCIDLib::TVoid QueryMediaDB
        (
            const   TXMLTreeElement&        xtnodeReq
        );

        tCIDLib::TVoid QueryMediaRendArt
        (
            const   TXMLTreeElement&        xtnodeReq
        );

        tCIDLib::TVoid QueryMediaRepoDrvs();

        tCIDLib::TVoid QueryPollList();

        tCIDLib::TVoid QueryRendPL
        (
            const   TXMLTreeElement&        xtnodeReq
        );

        tCIDLib::TVoid QueryRoomCfg
        (
            const   TXMLTreeElement&        xtnodeReq
        );

        tCIDLib::TVoid QueryRoomCfgList();

        tCIDLib::TVoid QuerySchEv
        (
            const   TXMLTreeElement&        xtnodeReq
        );

        tCIDLib::TVoid ReadField
        (
            const   TXMLTreeElement&        xtnodeReq
        );

        tCIDLib::TVoid RunMacro
        (
            const   TXMLTreeElement&        xtnodeReq
        );

        tCIDLib::TVoid SendAckReply();

        tCIDLib::TVoid SendAckReply
        (
            const   TString&                strInfoText
        );

        tCIDLib::TVoid SendConnAck
        (
            const   TIPEndPoint&            ipepClient
        );

        tCIDLib::TVoid SendExceptionReply
        (
            const   TError&                 errToSend
        );

        tCIDLib::TVoid SendNakReply
        (
            const   TString&                strReason
        );

        tCIDLib::TVoid SendReply
        (
            const   TMemBuf&                mbufData
            , const tCIDLib::TCard4         c4Bytes
            , const tCIDLib::TBoolean       bCanEncrypt = kCIDLib::True
        );

        tCIDLib::TVoid SendUnknownExceptionReply();

        tCIDLib::TVoid SetOption
        (
            const   TXMLTreeElement&        xtnodeReq
        );

        tCIDLib::TVoid SetPerEvInfo
        (
            const   TXMLTreeElement&        xtnodeReq
        );

        tCIDLib::TVoid SetSchEvInfo
        (
            const   TXMLTreeElement&        xtnodeReq
        );

        tCIDLib::TVoid SetSunEvInfo
        (
            const   TXMLTreeElement&        xtnodeReq
        );

        tCIDLib::TVoid SetPollList
        (
            const   TXMLTreeElement&        xtnodeReq
        );

        tCIDLib::TVoid ServiceClient();

        tCIDLib::TVoid ThrowParseErrReply();

        tCIDLib::TVoid WriteField
        (
            const   TXMLTreeElement&        xtnodeReq
            , const tCIDLib::TBoolean       bSendAck
        );

        const TXMLTreeElement& xtnodeGetMsg
        (
            const   TString&                strExpectedMsg
            , const tCIDLib::TCard4         c4Timeout
        );


        // --------------------------------------------------------------------
        //  Private data members
        //
        //  m_bNewList
        //      After a new poll list is set, we always return all of the field
        //      values on the first poll afterwards. Then we revert to just
        //      sending back fields that have changed. So we need a field to
        //      use as a oneshot.
        //
        //  m_bSecure
        //      Indicates if our current client was via a secure connection or not.
        //      Not currently used but may be useful.
        //
        //  m_c4xxxTO
        //      Some timeout values that we use. We put them here so that we
        //      can adjust them easily and so that we can also adjust them if
        //      we were started in simulator mode.
        //
        //  m_c4SysCfgSerialNum
        //      The latest serial number we got for the system configuration data. We
        //      init to zero to insure we get good data.
        //
        //  m_colFields
        //      The current list of fields that this thread's client is
        //      monitoring. Its a by ref list of our derivative of the
        //      interface engine's base widget.
        //
        //  m_colTmpList
        //      There are some places where we need to deal with strings lists,
        //      so we keep a vector of strings around for that.
        //
        //  m_cuctxClient
        //      We set up a client context after a successful logon, and pass
        //      this around to whoever needs it.
        //
        //  m_eOptFlags
        //      There are a set of options flags that clients can set or clear,
        //      to ask us to do (or not do) certain things.
        //
        //  m_enctLastMsg
        //      We have to watch for connections that just disappear and time
        //      them out. So this is set each time we get a message from a
        //      client. Each time we don't get a message, we check to see
        //      how long its been. If it's been beyond a period of time, we
        //      drop the connection.
        //
        //  m_esrMsgs
        //      The entity source reference that we use to parse messages. We
        //      set it up with a binary memory buffer entity source which we
        //      load new incoming messages into, then set the new data size
        //      and parse from it. This way, we don't have to create a new
        //      entity source for each incoming message.
        //
        //  m_fvXXX
        //      Field storage items for each field type, which are used during
        //      polling.
        //
        //  m_hdrCur
        //      The header we use to read message headers into.
        //
        //  m_ipepClient
        //      We get the client side end point out up front so that, if we
        //      lose them, we can have the end point still available for any
        //      messages logged on the way out.
        //
        //  m_meehLogger
        //      A macro engine error handler that is provided by CQCMacroEng
        //      and just logs everything to the central log server. We
        //      install it on our macro engine instance, who doesn't adopt it,
        //      it just uses it.
        //
        //  m_mefrData
        //      A file resolver for the macro engine. It's the standard
        //      fixed base handler, which we just give a base path to and
        //      it bases all macro file paths relative to that. The base
        //      path is set to the local CQC macro files directory.
        //
        //  m_meTarget
        //      The macro engine that we use to invoke macros.
        //
        //  m_pcdsClient
        //      To avoid having to pass the data source all over the place, we store
        //      a pointer to it here. This is a non-owning pointer. It's actually created
        //      and cleaned up locally within the worker thread.
        //
        //  m_pmbufData
        //      This is the buffer we give to the entity source, but we keep
        //      a separate pointer so that we can read directly into it and then
        //      tell the entity source how much data we put into it.
        //
        //  m_pxesMsgs
        //      The XML entity source that we use for parsing messages. We give
        //      it a pointer to m_pmbufData, and then in turn give ot the
        //      m_esrMsgs entity source reference object. We keep a separate
        //      pointer to this so that we can directly interact with it. The
        //      source ref works via the base entity source class, so we'd
        //      be downcasting it all the time.
        //
        //  m_scfgCur
        //      The latest system configuration data we got.
        //
        //  m_seccLogon
        //      The security challenge object we need for login. Normally, it
        //      would be a transient object, but since we are acting as a proxy
        //      for gateway clients, it has to exist across two calls.
        //
        //  m_strmOutput
        //      We need an output stream in order to gather the output from
        //      the macro, if any, and send it back to the caller. We install
        //      this on the engine as the console output stream.
        //
        //  m_strmReply
        //      The output stream we use to build up outgoing messages.
        //
        //  m_xtprsMsgs
        //      The parser we use to parse messages from the socket.
        //
        //  m_uaccClient
        //      Once we get logged in, our user account info is stored here for
        //      later error reporting mostly.
        // --------------------------------------------------------------------
        tCIDLib::TBoolean           m_bNewFields;
        tCIDLib::TBoolean           m_bSecure;
        tCIDLib::TCard4             m_c4LogOnTO;
        tCIDLib::TCard4             m_c4SysCfgSerNum;
        TFldList                    m_colFields;
        tCIDLib::TStrList           m_colTmpList;
        TCQCUserCtx                 m_cuctxClient;
        tCQCGWSrv::EOptFlags        m_eOptFlags;
        tCIDLib::TEncodedTime       m_enctLastMsg;
        tCIDXML::TEntitySrcRef      m_esrMsgs;
        TCQCBoolFldValue            m_fvBool;
        TCQCCardFldValue            m_fvCard;
        TCQCFloatFldValue           m_fvFloat;
        TCQCIntFldValue             m_fvInt;
        TCQCStringFldValue          m_fvString;
        TCQCStrListFldValue         m_fvStrList;
        TCQCTimeFldValue            m_fvTime;
        tCQCGWSrv::TPacketHdr       m_hdrCur;
        TIPEndPoint                 m_ipepClient;
        TCQCMEngErrHandler          m_meehLogger;
        TMEngFixedBaseFileResolver  m_mefrData;
        TCIDMacroEngine             m_meTarget;
        TCIDDataSrc*                m_pcdsClient;
        THeapBuf*                   m_pmbufData;
        TMemBufEntitySrc*           m_pxesMsgs;
        TCQCSysCfg                  m_scfgCur;
        TCQCSecChallenge            m_seccLogon;
        TTextStringOutStream        m_strmOutput;
        TTextMBufOutStream          m_strmReply;
        TXMLTreeParser              m_xtprsMsgs;
        TCQCUserAccount             m_uaccClient;


        // --------------------------------------------------------------------
        //  Magic macros
        // --------------------------------------------------------------------
        RTTIDefs(TWorkerThread,TThread)
};

#pragma CIDLIB_POPPACK


