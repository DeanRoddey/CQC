//
// FILE NAME: MQTTSh_Config.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/31/2019
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
//  This is the header for the configuration info related classes. This is driven
//  by the configuration file, which is an XML file that we parse in.
//
//  * This configuration info will grow over time. Currently it only allows you to
//    define single topic to field relationships. Later, we'll probably allow for
//    more 'device oriented' configuration as well, based on the standard device
//    classes, so you can create things like thermos or media players in one shot.
//
//  Single Fields
//
//  For fields though each particular field type in the config (which are driven
//  by the semantic field types) has a particular set of configuable values, there's
//  only a fairly small set overall. Each one defines the info required to set up
//  the CQC field, the associated MQTT topic, information about the MQTT payload
//  for that topic, and usually a value map of some sort ot map values back and forth
//  between the field and the MQTT topic.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//

#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: TMQTTFldCfg
//  PREFIX: mqfldc
// ---------------------------------------------------------------------------
class MQTTSHEXPORT TMQTTFldCfg : public TObject
{
    public :
        // -------------------------------------------------------------------
        //  Public types
        // -------------------------------------------------------------------
        using TMapPtr   = TCntPtr<TMQTTValMap>;
        using TRXPtr    = TCntPtr<TRegEx>;


        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TMQTTFldCfg();

        TMQTTFldCfg(const TMQTTFldCfg&) = default;

        ~TMQTTFldCfg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TMQTTFldCfg& operator=(const TMQTTFldCfg&) = default;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bReadable() const
        {
            return tCIDLib::bAnyBitsOn(m_eAccess, tCQCKit::EFldAccess::Read);
        }

        tCIDLib::TBoolean bAlwaysWrite() const
        {
            return m_bAlwaysWrite;
        }

        tCIDLib::TBoolean bMapOutVal
        (
            const   TString&                strFldVal
            ,       TMemBuf&                mbufMQTTPL
            ,       tCIDLib::TCard4&        c4PLBytes
        )   const;

        tCIDLib::TBoolean bOnConnect() const
        {
            return m_bOnConnect;
        }

        tCIDLib::TBoolean bMatchesInTopic
        (
            const   TString&                strToCheck
        )   const;

        tCIDLib::TBoolean bPackageOutVal
        (
            const   TString&                strMQTTVal
            ,       TMemBuf&                mbufTar
            ,       tCIDLib::TCard4&        c4Bytes
        )   const;

        tCIDLib::TBoolean bRejected() const
        {
            return m_bRejected;
        }

        tCIDLib::TBoolean bRejected(const tCIDLib::TBoolean bToSet)
        {
            m_bRejected = bToSet;
            return m_bRejected;
        }

        tCIDLib::TBoolean bRetainWrite() const
        {
            return m_bRetainWrite;
        }

        tCIDLib::TBoolean bV2Compat() const
        {
            return m_bV2Compat;
        }

        const tCQCMQTT::EQOSLevs eActualQOS() const
        {
            return m_eActualQOS;
        }

        tCQCMQTT::EQOSLevs eActualQOS(const tCQCMQTT::EQOSLevs eToSet)
        {
            m_eActualQOS = eToSet;
            return m_eActualQOS;
        }

        tCQCKit::EFldTypes eFldType() const
        {
            return m_eType;
        }

        tMQTTSh::EInMapRes eMapInVal
        (
            const   TMemBuf&                mbufVal
            , const tCIDLib::TCard4         c4Bytes
            ,       TString&                strFldVal
        )   const;

        tMQTTSh::EPLTypes ePLType() const
        {
            return m_mqplfData.m_eType;
        }

        tCQCKit::EFldSTypes eSemType() const
        {
            return m_eSemType;
        }

        tCIDLib::TVoid InitializeFld
        (
            const   tCIDLib::TCard1         c1SubAckCode
        );

        tCIDLib::TVoid ParseFrom
        (
            const   TXMLTreeElement&        xtnodeSrc
            ,       tCIDLib::TStrCollect&   colErrs
        );

        tCIDLib::TVoid QueryFldDef
        (
                    TCQCFldDef&             flddToFill
        )   const;

        const TString& strBaseName() const
        {
            return m_strBaseName;
        }

        const TString& strFldName() const
        {
            return m_strFldName;
        }

        const TString& strOnConnect() const
        {
            return m_strOnConnect;
        }

        const TString& strTopicPath() const
        {
            return m_strTopicPath;
        }

        tCIDLib::TVoid Set
        (
            const   tCQCKit::EFldAccess     eAccess
            , const tCQCKit::EFldSTypes     eSemType
            , const tCQCKit::EFldTypes      eType
            , const TString&                strBaseName
            , const TString&                strFldName
            , const TString&                strLimits
            , const TString&                strTopicPath
            , const TMapPtr&                cptrMap
            , const TMQTTPLFmt&             mqplfData
            , const tCIDLib::TBoolean       bV2Compat
        );


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bExtractCard
        (
            const   TMemBuf&                mbufPL
            , const tCIDLib::TCard4         c4PLBytes
            ,       tCIDLib::TCard4&        c4NewVal
        )   const;

        tCIDLib::TBoolean bExtractInt
        (
            const  TMemBuf&                 mbufPL
            , const tCIDLib::TCard4         c4PLBytes
            ,       tCIDLib::TInt4&         i4NewVal
        )   const;


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bAlwaysWrite
        //      Only used by generic type fields and only if read/write, to let the user
        //      indicate the field should be marked as always write.
        //
        //  m_bOnConnect
        //      m_strOnConnect may be present but empty because they want us to just send
        //      an empty payload. So we need to remember separately if it was present or
        //      not.
        //
        //  m_bRejected
        //      If the subscription for this topic is rejected, it can be marked as such
        //      here. It defaults to true so that any we somehow didn't get an appropriate
        //      response for will be marked rejected.
        //
        //  m_bRetainWrite
        //      They can tell us whether to ask the server to retain the value for this
        //      field if send it out in response to a field write.
        //
        //  m_bV2Compat
        //      When setting up the field info, we remember if it is V2 compatible,
        //      since that will be needed later to control whether we send out standard
        //      event triggers.
        //
        //  m_cptrMap
        //      A mapping object to map incoming and outgoing values. We look at them via
        //      the base class. The one to allocate is driven by the config. In some cases
        //      we will create a default one if one is not indicated for common scenarios.
        //
        //  m_cptrAltInTopic
        //      If we get an AltInTopic attribute, we set up this regular expression from
        //      it and will compare any incoming topics to see if they fully match. If so
        //      we will process that incoming msg. This is in addition to our main topic
        //      in m_strTopicPath. We set m_bAltInTopic if this needs to be processed.
        //      It's in a counted pointer because the regular expression engine isn't
        //      copyable and this let's us still have default generated copy/assign.
        //
        //  m_eAccess
        //      The access for the field. Sometimes from the file, sometimes forced
        //      based on type.
        //
        //  m_eActualQOS
        //      Runtime only, for use by the application. He can use this to store the
        //      actual QOS allowed when this topic is subscribed.
        //
        //  m_eSemType
        //      The semantic field type, which drives the other members that are valid.
        //
        //  m_eType
        //      The field data type.
        //
        //  m_f8MaxValue
        //  m_f8MinValue
        //      The range if there is one for this type. It could be int or float but we
        //      just store them as float either way.
        //
        //  m_mbufFmt
        //      We need a temp buffer for some formats, and this lets us avoid creating
        //      and destroying them all the time.
        //
        //  m_mqplfData
        //      The data representation info for the payload. The other stuff here is
        //      about the field that the payload data goes to. This is about how to
        //      interpret the payload itself, and it also provides the in/out value
        //      mapper to translate values.
        //
        //  m_strBaseName
        //      The base part of the field name. We create the full field name based on
        //      on whether it meets potential V2 field requirements or not.
        //
        //  m_strFldName
        //      The actual field name we build based on the base name plus the other
        //      info (which may or may not allow it to be V2 compatible.)
        //
        //  m_strLimits
        //      If the field type needs a field limit, this is it formatted out.
        //
        //  m_strOnConnect
        //      An initial value to send out for this field. Even if the field is not
        //      writable an initial value can be sent, sometimes required to get
        //      the device configured.
        //
        //  m_strTopicPath
        //      The topic for this field (though we may have an alt in topic as well,
        //      see m_regxAltInTopic.)
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bAltInTopic;
        tCIDLib::TBoolean       m_bAlwaysWrite;
        tCIDLib::TBoolean       m_bOnConnect;
        tCIDLib::TBoolean       m_bRejected;
        tCIDLib::TBoolean       m_bRetainWrite;
        tCIDLib::TBoolean       m_bV2Compat;
        TMapPtr                 m_cptrMap;
        TRXPtr                  m_cptrAltInTopic;
        tCQCKit::EFldAccess     m_eAccess;
        tCQCMQTT::EQOSLevs      m_eActualQOS;
        tCQCKit::EFldSTypes     m_eSemType;
        tCQCKit::EFldTypes      m_eType;
        tCIDLib::TFloat8        m_f8MaxValue;
        tCIDLib::TFloat8        m_f8MinValue;
        mutable THeapBuf        m_mbufFmt;
        TMQTTPLFmt              m_mqplfData;
        TString                 m_strBaseName;
        TString                 m_strFldName;
        TString                 m_strLimits;
        TString                 m_strOnConnect;
        TString                 m_strTopicPath;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TMQTTFldCfg, TObject)
};



// ---------------------------------------------------------------------------
//   CLASS: TMQTTCfg
//  PREFIX: mqcfg
// ---------------------------------------------------------------------------
class MQTTSHEXPORT TMQTTCfg : public TObject
{
    public :
        // -------------------------------------------------------------------
        //  Class types
        // -------------------------------------------------------------------
        using TFldList      = TKeyedHashSet<TMQTTFldCfg, TString, TStringKeyOps>;
        using TFldCursor    = TFldList::TCursor;


        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static tCIDLib::TVoid AddBadValErr
        (
                    tCIDLib::TStrCollect&   colErrs
            , const TString&                strFldName
            , const TString&                strValue
            , const TString&                strValDescr
        );

        static tCIDLib::TVoid AddError
        (
                    tCIDLib::TStrCollect&   colErrs
            , const TString&                strFldName
            , const TString&                strErrText
        );

        static tCIDLib::TVoid AddRequiredValErr
        (
                    tCIDLib::TStrCollect&   colErrs
            , const TString&                strFldName
            , const TString&                strValDescr
        );

        static tCIDLib::TVoid AddNotUsedValErr
        (
                    tCIDLib::TStrCollect&   colErrs
            , const TString&                strFldName
            , const TString&                strValDescr
        );


        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TMQTTCfg();

        TMQTTCfg(const TMQTTCfg&) = default;

        ~TMQTTCfg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TMQTTCfg& operator=(const TMQTTCfg&) = default;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bMsgTrace() const
        {
            return m_bMsgTrace;
        }

        tCIDLib::TBoolean bParseFrom
        (
            const   TString&                strPath
            , const TString&                strMoniker
            ,       tCIDLib::TStrCollect&   colErrs
        );

        tCIDLib::TBoolean bParseFrom
        (
                    tCIDXML::TEntitySrcRef& esrSrc
            , const TString&                strMoniker
            ,       tCIDLib::TStrCollect&   colErrs
        );

        tCIDLib::TBoolean bSecureConn() const
        {
            return m_bSecure;
        }

        tCIDLib::TBoolean bVerboseMode() const
        {
            return m_bVerboseMode;
        }

        tCIDLib::TCard4 c4FldCount() const
        {
            return m_colFldList.c4ElemCount();
        }

        tCIDLib::TCard4 c4Version() const
        {
            return m_c4Version;
        }

        TFldCursor cursFldList() const;

        tCIDLib::TIPPortNum ippnMQTT() const
        {
            return m_ippnMQTT;
        }

        tCIDLib::TVoid InitFldsForTopic
        (
            const   TString&                strTopic
            , const tCIDLib::TCard1         c1Code
        );

        const TMQTTFldCfg* pmqfldcFindFld
        (
            const   TString&                strToFind
        )   const;

        const TString& strCertInfo() const
        {
            return m_strCertInfo;
        }

        const TString& strPrefProtocol() const
        {
            return m_strPrefProtocol;
        }

        const TString& strMQTTAddr() const
        {
            return m_strMQTTAddr;
        }

        const TString& strPassword() const
        {
            return m_strPassword;
        }

        const TString& strSecPrincipal() const
        {
            return m_strSecPrincipal;
        }

        const TString& strUserName() const
        {
            return m_strUserName;
        }


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid ParseFrom
        (
            const   TXMLTreeElement&        xtnodeSrc
            , const TString&                strMoniker
            ,       tCIDLib::TStrCollect&   colErrs
        );

        tCIDLib::TVoid ParseDevClsPower
        (
            const   TString&                strMoniker
            , const TXMLTreeElement&        xtnodeCls
            ,       TMQTTFldCfg&            mqfldcTmp
            ,       tCIDLib::TStrCollect&   colErrs
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bMsgTrace
        //      Normally this is turned on by the user via a field. But in some cases
        //      we want it on from the very beginning to catch the startup process.
        //      So it can be set in the config to force the initial value.
        //
        //  m_bSecure
        //      Indicates we need to do a secure socket connection.
        //
        //  m_bVerboseMode
        //      Same as m_bMsgTrace above.
        //
        //  m_c4Version
        //      The format version of the config file.
        //
        //  m_colFldList
        //      The list of single fields we are configured for. Each of these maps a
        //      single topic to a single CQC field. They are keyed on field name which
        //      must be unique.
        //
        //  m_ippnMQTT
        //      The port we should connect to the MQTT server on.
        //
        //  m_strCertInfo
        //      If they want to use a client side certificate, this will be set to the
        //      cert store info.
        //
        //  m_strPrefProtocol
        //      Some servers require an ALPN type preferred protocol negotiation on
        //      secure connections, so we let them set one. If not set it's just zero
        //      and ALPN won't be done.
        //
        //  m_strMQTTAddr
        //      The address of the MQTT server to connect to
        //
        //  m_strPassword
        //      If set in the config file, these will be sent in the connect message.
        //
        //  m_strSecPrincipal
        //      if m_bSecure is set, this has to be the security principal, i.e. the
        //       correct security certificate name for the server we are connecting to.
        //      Generally it's the same as m_strMQTTAddr, but may not be. If not set,
        //      we use the address.
        //
        //  m_strUserName
        //      If set in the config file, these will be sent in the connect   message.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bMsgTrace;
        tCIDLib::TBoolean   m_bSecure;
        tCIDLib::TBoolean   m_bVerboseMode;
        tCIDLib::TCard4     m_c4Version;
        TFldList            m_colFldList;
        tCIDLib::TIPPortNum m_ippnMQTT;
        TString             m_strCertInfo;
        TString             m_strPrefProtocol;
        TString             m_strMQTTAddr;
        TString             m_strPassword;
        TString             m_strSecPrincipal;
        TString             m_strUserName;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TMQTTCfg, TObject)
};

#pragma CIDLIB_POPPACK
