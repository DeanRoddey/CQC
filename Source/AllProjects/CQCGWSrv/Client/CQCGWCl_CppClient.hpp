//
// FILE NAME: CQCGWCl_CppClient.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/02/2013
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
//  This is the header for the CQCGWCl_CppClient.cpp file. This implements a
//  C++ wrapper around the XML Gateway protocol, to hide the details of all
//  the XML formatting and parsing. It just exposes a simple class interface.
//  We don't support the whole API this way yet.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//  CLASS: TCQCGWSrvClient
// PREFIX: gwsc
// ---------------------------------------------------------------------------
class CQCGWCLEXPORT TCQCGWSrvClient : public TObject
{
    public  :
        // -------------------------------------------------------------------
        //  Class types
        // -------------------------------------------------------------------
        class TFieldInfo
        {
            public :
                // -----------------------------------------------------------
                //  Constructors and Destructor
                // -----------------------------------------------------------
                TFieldInfo() = delete;

                TFieldInfo(const TString& strFldName) :

                    m_bChanged(kCIDLib::True)
                    , m_bState(kCIDLib::False)
                    , m_strName(strFldName)
                {
                }

                TFieldInfo(const TFieldInfo&) = default;
                TFieldInfo(TFieldInfo&&) = delete;

                ~TFieldInfo() {}

                TFieldInfo& operator=(const TFieldInfo&) = default;
                TFieldInfo& operator=(TFieldInfo&&) = delete;

                tCIDLib::TBoolean   m_bChanged;
                tCIDLib::TBoolean   m_bState;
                TString             m_strName;
                TString             m_strValue;
        };
        using TNameTreeCol = TBasicTreeCol<TString>;


        // -------------------------------------------------------------------
        // Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCGWSrvClient();

        TCQCGWSrvClient(const TCQCGWSrvClient&) = delete;
        TCQCGWSrvClient(TCQCGWSrvClient&&) = delete;

        ~TCQCGWSrvClient();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCGWSrvClient& operator=(const TCQCGWSrvClient&) = delete;
        TCQCGWSrvClient& operator=(TCQCGWSrvClient&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIsConnected() const;

        tCIDLib::TBoolean bCheckField
        (
            const   tCIDLib::TCard4         c4Index
            ,       tCIDLib::TBoolean&      bState
            ,       TString&                strValue
        );

        tCIDLib::TBoolean bDoGlobalAct
        (
            const   TString&                strClassPath
            , const tCIDLib::TStrList&      colParms
            ,       TString&                strFailReason
        );

        tCIDLib::TBoolean bPollFields();

        tCIDLib::TBoolean bQueryDrvText
        (
            const   TString&                strMoniker
            , const TString&                strQType
            , const TString&                strDataName
            ,       TString&                strOutput
        );

        tCIDLib::TBoolean bQueryMediaRendArt
        (
            const   TString&                strRepoMoniker
            ,       THeapBuf&               mbufData
            ,       tCIDLib::TCard4&        c4DataSz
        );

        tCIDLib::TBoolean bQuerySchEvs
        (
                    tCIDLib::TCard4&        c4SerialNum
            ,       tCIDLib::TStrList&      colNames
            ,       tCIDLib::TStrList&      colIds
            ,       tCIDLib::TStrList&      colTypes
            ,       TFundVector<tCIDLib::TBoolean>& fcolStates
        );

        tCIDLib::TBoolean bReadField
        (
            const   TString&                strMoniker
            , const TString&                strField
            ,       TString&                strToFill
        );

        tCIDLib::TBoolean bRunMacro
        (
            const   TString&                strClassPath
            , const tCIDLib::TStrList&      colParms
            ,       TString&                strOutput
            ,       tCIDLib::TInt4&         i4ResCode
        );

        tCIDLib::TCard4 c4PollListCount() const;

        tCIDLib::TVoid Connect
        (
            const   TIPEndPoint&            ipepSrv
            , const TString&                strUserName
            , const TString&                strPassword
            , const tCIDLib::TBoolean       bSecure
        );

        tCIDLib::TVoid Disconnect();

        tCIDLib::ELoadRes eQueryMediaArt
        (
            const   TString&                strRepoMoniker
            , const TString&                strUID
            , const TString&                strCurPerId
            , const tCQCMedia::EMediaTypes  eMType
            , const tCIDLib::TBoolean       bLarge
            ,       tCIDLib::TCard4&        c4Bytes
            ,       TMemBuf&                mbufToFill
            ,       TString&                strNewPerId
        );

        tCIDLib::ELoadRes eQueryMediaDB
        (
            const   TString&                strRepoMon
            , const TString&                strSerialNum
            ,       TString&                strNewSerialNum
            ,       THeapBuf&               mbufData
            ,       tCIDLib::TCard4&        c4Bytes
            ,       TString&                strErrMsg
        );

        tCQCKit::EUserRoles eUserRole() const;

        const TFieldInfo& fldiAt
        (
            const   tCIDLib::TCard4         c4At
        )   const;

        tCIDLib::TVoid PauseSchEv
        (
            const   TString&                strId
            , const tCIDLib::TBoolean       bNewState
        );

        tCIDLib::TVoid Ping();

        tCIDLib::TVoid QueryDrivers
        (
                    tCIDLib::TStrList&      colMonikers
            ,       TFundVector<tCQCKit::EDevCats>& fcolCats
        );

        tCIDLib::TVoid QueryDrvInfo
        (
            const   TString&                strMoniker
            ,       tCIDLib::TBoolean&      bOnline
            ,       tCIDLib::TCard4&        c4ArchVer
            ,       TString&                strMake
            ,       TString&                strModel
        );

        tCIDLib::TVoid QueryFieldInfo
        (
            const   TString&                strMonFld
            ,       TCQCFldDef&             flddToFill
        );

        tCIDLib::TVoid QueryFields
        (
                    TNamedValMap<TCQCFldDef>& colToFill
        );

        tCIDLib::TVoid QueryGlobActs
        (
                    TNameTreeCol&           colToFill
        );

        tCIDLib::TVoid QueryMacros
        (
                    TNameTreeCol&           colToFill
        );

        tCIDLib::TVoid QueryMediaRepoDrvs
        (
                    tCIDLib::TStrCollect&   colToFill
        );

        tCIDLib::TVoid QueryRendPL
        (
            const   TString&                strMoniker
            ,       tCIDLib::TStrList&      colArtists
            ,       tCIDLib::TStrList&      colNames
            ,       tCIDLib::TStrList&      colCookies
            ,       tCIDLib::TStrList&      colLocs
            ,       tCIDLib::TCardList&     fcolIds
        );

        tCIDLib::TVoid QueryRmCfg
        (
            const   TString&                strRmName
        );

        tCIDLib::TVoid QueryRmCfgList
        (
                    tCIDLib::TStrList&      colRooms
        );

        tCIDLib::TVoid QuerySchEvInfo
        (
            const   TString&                strId
            ,       TString&                strName
            ,       tCIDLib::TBoolean&      bPaused
            ,       TString&                strType
        );

        tCIDLib::TVoid QueryPerEv
        (
            const   TString&                strId
            ,       TString&                strName
            ,       tCIDLib::TBoolean&      bPaused
            ,       TString&                strType
            ,       tCIDLib::TCard4&        c4Period
        );

        tCIDLib::TVoid QuerySchEv
        (
            const   TString&                strId
            ,       TString&                strName
            ,       tCIDLib::TBoolean&      bPaused
            ,       TString&                strType
            ,       TString&                strMask
            ,       tCIDLib::TCard4&        c4Day
            ,       tCIDLib::TCard4&        c4Hour
            ,       tCIDLib::TCard4&        c4Minute
        );

        tCIDLib::TVoid QuerySunEv
        (
            const   TString&                strId
            ,       TString&                strName
            ,       tCIDLib::TBoolean&      bPaused
            ,       TString&                strType
            ,       tCIDLib::TInt4&         i4Offset
        );

        tCIDLib::TVoid SetPerEv
        (
            const   TString&                strId
            , const tCIDLib::TCard4         c4Period
            , const tCIDLib::TEncodedTime   enctStart
        );

        tCIDLib::TVoid SetSchEv
        (
            const   TString&                strId
            , const TString&                strMask
            , const tCIDLib::TCard4         c4Day
            , const tCIDLib::TCard4         c4Hour
            , const tCIDLib::TCard4         c4Minute
        );

        tCIDLib::TVoid SetSunEv
        (
            const   TString&                strId
            , const tCIDLib::TInt4          i4Offset
        );

        tCIDLib::TVoid SetPollList
        (
            const   tCIDLib::TStrCollect&   colList
        );

        tCIDLib::TVoid WriteField
        (
            const   TString&                strMoniker
            , const TString&                strField
            , const TString&                strValue
        );

        tCIDLib::TVoid MWriteField
        (
            const   tCIDLib::TStrList&      colMonikers
            , const tCIDLib::TStrList&      colFields
            , const tCIDLib::TStrList&      colValues
        );


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bGetAckNak
        (
                    TString&                strNakReason
            , const tCIDLib::TCard4         c4WaitFor
        );

        tCIDLib::TBoolean bGetMsg
        (
            const   tCIDLib::TCard4         c4WaitFor
        );

        tCIDLib::TCard4 c4FindFld
        (
            const   TString&                strToFind
            , const tCIDLib::TBoolean       bThrowIfNot
        );

        tCIDLib::TVoid CheckConnected() const;

        tCIDLib::TVoid CheckHeader
        (
            const   tCQCGWSrv::TPacketHdr&  hdrToCheck
        )   const;

        tCQCKit::EFldTypes eXlatFldType
        (
            const   TString&                strToXlat
        )   const;

        tCIDLib::TVoid GetAck
        (
            const   tCIDLib::TCard4         c4WaitFor
        );

        tCIDLib::TVoid GetHierarchy
        (
            const   TXMLTreeElement&        xtnodeList
            ,       TBasicTreeCol<TString>& colToFill
            ,       TString&                strCurPath
            , const TString&                strScopeElem
            , const TString&                strNameElem
        );

        tCIDLib::TVoid HandleLogonChallenge
        (
            const   TXMLTreeElement&        xtnodeChallenge
        );

        tCIDLib::TVoid LogOn();

        tCIDLib::TVoid SendQueryOp
        (
            const   TString&                strOpToSend
        );

        tCIDLib::TVoid SendMsg
        (
            const   tCIDLib::TBoolean       bCanEncrypt = kCIDLib::True
        );

        const TXMLTreeElement& xtnodeGetMsg
        (
            const   tCIDLib::TCh* const     pszExpected
            , const tCIDLib::TCard4         c4WaitFor
            ,       tCIDLib::TBoolean&      bGotExpected
        );

        const TXMLTreeElement& xtnodeGetMsg
        (
            const   tCIDLib::TCh* const     pszExpected
            , const tCIDLib::TCard4         c4WaitFor
        );

        const TXMLTreeElement& xtnodeGetMsg
        (
            const   tCIDLib::TCard4         c4WaitFor
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4SeqNum
        //      The next sequence number need to send. It's stuck into the
        //      header and bumped each time we send a message.
        //
        //  m_colFields
        //      The fields that we are currently polling from the server. It
        //      keeps up with the names of the fields, and holds the field
        //      state and latest value, as well as a change flag that is set
        //      when the state and/or value changed. The client can ask for
        //      this info and we'll clear the change flag until the next time
        //      it changes (during the bDoPoll() method call.)
        //
        //  m_cptrBuf
        //      We pass in a buffer of memory to be parsed each time we
        //      have to process a message. This requires setting up a memory
        //      buffer entity source to pass to the parser. We don't want
        //      any of that stuff here, since it would force us to impose
        //      the XML headers on client code. So we set up the counted
        //      pointer to a buffer, and we can build the entity source
        //      efficiently each time from that, and doing it this way keeps
        //      the buffer alive, even though the parser adopts and destroys
        //      the entity source object.
        //
        //  m_expbSend
        //      A expandable byte buffer used to transcode outgoing messages
        //      into a UTF-8 format before sending them.
        //
        //  m_eUserRole
        //      After a successful login, we will get back the user role of
        //      the account we logged into and will store it here for later
        //      access by clients. This is not meaninful if not currently
        //      connected.
        //
        //  m_hdrCur
        //      The header we use to read message headers into.
        //
        //  m_pmbufData
        //      This is the buffer we give to the entity source, but we keep
        //      a separate pointer so that we can read directly into it and
        //      then tell the entity source how much data we put into it.
        //
        //  m_pcdsClient
        //      A pointer to a data source so that we can support either secure or
        //      non-secure comms, based on which type we allocate.
        //
        //  m_pxtprsMsgs
        //      The parser we use to parse messages from the socket. It's
        //      a pointer so we don't have to force all the XML headers on
        //      downstream code.
        //
        //  m_strCursorId
        //      The id of the last media cursor we got
        //
        //  m_strPassword
        //      The password given on the command line, which we use to log
        //      on to the server.
        //
        //  m_strUserName
        //      The user name given on the command line, which use to log on
        //      to the server.
        //
        //  m_strmMsg
        //      A string based text output stream that we use to build up
        //      messages. So that we can log them to a file, we build to this
        //      text stream and then do a conversion to UTF-8 into a binary
        //      stream for sending.
        //
        //  m_tcvtSend
        //      A text converter used to transcode the outgoing messages to
        //      UTF-8 format before sending them over the socket.
        // -------------------------------------------------------------------
        tCIDLib::TCard4         m_c4SeqNum;
        TVector<TFieldInfo>     m_colFields;
        TCntPtr<const TMemBuf>  m_cptrBuf;
        TBlowfishEncrypter      m_crypSess;
        TExpByteBuf             m_expbData;
        tCQCKit::EUserRoles     m_eUserRole;
        tCQCGWSrv::TPacketHdr   m_hdrCur;
        THeapBuf*               m_pmbufData;
        TCIDDataSrc*            m_pcdsClient;
        TXMLTreeParser*         m_pxtprsMsgs;
        TString                 m_strCursorId;
        TString                 m_strPassword;
        TString                 m_strUserName;
        TTextStringOutStream    m_strmReply;
        TUTF8Converter          m_tcvtSend;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCGWSrvClient,TObject)
};

#pragma CIDLIB_POPPACK

