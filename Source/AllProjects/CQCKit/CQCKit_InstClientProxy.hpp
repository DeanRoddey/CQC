// ----------------------------------------------------------------------------
//  FILE: CQCKit_InstClientProxy.hpp
//  DATE: Fri, Feb 12 21:14:14 2021 -0500
//    ID: 825C0E370D8C5D6C-0B3B6D941AB56445
//
//  This file was generated by the Charmed Quark CIDIDL compiler. Do not make
//  changes by hand, because they will be lost if the file is regenerated.
// ----------------------------------------------------------------------------
#pragma once

#pragma CIDLIB_PACK(CIDLIBPACK)

class CQCKITEXPORT TCQCInstClientProxy : public TOrbClientBase
{
    public :
        // --------------------------------------------------------------------
        // Public, static data
        // --------------------------------------------------------------------
        static const TString strInterfaceId;
        static const TString strBinding;



        // --------------------------------------------------------------------
        // Public Constructors and Destructor
        // --------------------------------------------------------------------
        TCQCInstClientProxy();
        TCQCInstClientProxy(const TOrbObjId& ooidSrc, const TString& strNSBinding);
        TCQCInstClientProxy(const TCQCInstClientProxy&) = delete;
        TCQCInstClientProxy(TCQCInstClientProxy&&) = delete;
        ~TCQCInstClientProxy();

        // --------------------------------------------------------------------
        // Public operators
        // --------------------------------------------------------------------
        TCQCInstClientProxy& operator=(const TCQCInstClientProxy&) = delete;
        TCQCInstClientProxy& operator=(TCQCInstClientProxy&&) = delete;

        // --------------------------------------------------------------------
        // Public, inherited methods
        // --------------------------------------------------------------------
        tCIDLib::TVoid SetObjId
        (
            const   TOrbObjId& ooidToSet
            , const TString& strNSBinding
            , const tCIDLib::TBoolean bCheck = kCIDLib::True
        )   override;

        // --------------------------------------------------------------------
        // Public, non-virtual methods
        // --------------------------------------------------------------------
        tCIDLib::TBoolean bAddOrUpdateCheatSheet
        (
            const TString& strUserName
            , CIOP tCIDLib::TCard4& c4SerialNum
            , const TCQCActVarList& cavlCheatSheet
            , const TCQCSecToken& sectUser
        );

        tCIDLib::TBoolean bUpdateEmailAccount
        (
            const TCQCEMailAccount& emacctToAdd
            , CIOP tCIDLib::TCard4& c4SerialNum
            , const TCQCSecToken& sectUser
            , const tCIDLib::TBoolean bMustBeNew
        );

        tCIDLib::TBoolean bAddOrUpdateGC100Cfg
        (
            const tCIDLib::TCard4 c4BufSz_mbufToSet
            , const THeapBuf& mbufToSet
            , CIOP tCIDLib::TCard4& c4SerialNum
            , const TCQCSecToken& sectUser
        );

        tCIDLib::TBoolean bAddOrUpdateJAPCfg
        (
            const tCIDLib::TCard4 c4BufSz_mbufToFill
            , const THeapBuf& mbufToFill
            , CIOP tCIDLib::TCard4& c4SerialNum
            , const TCQCSecToken& sectUser
        );

        tCIDLib::TBoolean bDeleteEMailAccount
        (
            const TString& strAccount
            , const TCQCSecToken& sectUser
        );

        tCIDLib::TBoolean bDrvCfgDriverPresent
        (
            const TString& strMoniker
            , COP TString& strHost
            , const TCQCSecToken& sectUser
        );

        tCIDLib::TBoolean bDrvCfgMoveDrivers
        (
            const TString& strOldHost
            , const TString& strNewHost
            , COP TVector<TString>& colMsgs
            , const TCQCSecToken& sectUser
        );

        tCIDLib::TBoolean bDrvCfgQueryConfig
        (
            CIOP tCIDLib::TCard4& c4Version
            , const TString& strMoniker
            , CIOP TString& strHost
            , TCQCDriverObjCfg& cqcdcToFill
            , const TCQCSecToken& sectUser
        );

        tCIDLib::TBoolean bDrvCfgQueryHostConfigs
        (
            CIOP tCIDLib::TCard4& c4Version
            , const TString& strHost
            , TVector<TCQCDriverObjCfg>& colCfgs
            , TFundVector<tCIDLib::TCard4>& fcolCfgVers
            , const TCQCSecToken& sectUser
        );

        tCIDLib::TBoolean bDrvCfgQueryHosts
        (
            COP TVector<TString>& colHosts
            , const TCQCSecToken& sectUser
        );

        tCIDLib::TBoolean bDrvCfgQueryMonList
        (
            CIOP tCIDLib::TCard4& c4Version
            , const TString& strHost
            , TVector<TString>& colToFill
            , TFundVector<tCIDLib::TCard4>& fcolVers
            , TFundVector<tCIDLib::TBoolean>& fcolClDrvs
            , const TCQCSecToken& sectUser
        );

        tCIDLib::TBoolean bDrvCfgQueryMonList
        (
            COP TVector<TKeyValuePair>& colToFill
            , const TCQCSecToken& sectUser
        );

        tCIDLib::TBoolean bDrvCfgSetPause
        (
            const TString& strMoniker
            , const tCIDLib::TBoolean bNewState
            , COP tCIDLib::TCard4& c4NewVersion
            , const TCQCSecToken& sectUser
        );

        tCIDLib::TBoolean bQueryCheatSheet
        (
            CIOP tCIDLib::TCard4& c4SerialNum
            , tCIDLib::ELoadRes& eLoadRes
            , const TString& strUserName
            , TCQCActVarList& cavlCheatSheet
            , const TCQCSecToken& sectUser
        );

        tCIDLib::TBoolean bQueryDrvManifest
        (
            const TString& strMake
            , const TString& strModel
            , COP TCQCDriverCfg& cqcdcToFill
        );

        tCIDLib::TBoolean bQueryEmailAccount
        (
            const TString& strAccount
            , COP TCQCEMailAccount& emacctToFill
            , const tCIDLib::TBoolean bThrowIfNot
            , const TCQCSecToken& sectUser
        );

        tCIDLib::TBoolean bQueryEMailAccountNames
        (
            COP TVector<TString>& colToFill
            , const TCQCSecToken& sectUser
        );

        tCIDLib::TBoolean bQueryKeyMap
        (
            CIOP tCIDLib::TCard4& c4SerialNum
            , tCIDLib::ELoadRes& eLoadRes
            , const TString& strUserName
            , TCQCKeyMap& kmToFill
            , const TCQCSecToken& sectUser
        );

        tCIDLib::TBoolean bQueryLocationInfo
        (
            COP tCIDLib::TFloat8& f8Lat
            , COP tCIDLib::TFloat8& f8Long
            , const TCQCSecToken& sectUser
        );

        tCIDLib::TBoolean bQueryProtoFile
        (
            const TString& strFileName
            , tCIDLib::TCard4& c4BufSz_mbufToFill
            , COP THeapBuf& mbufToFill
        );

        tCIDLib::TBoolean bQueryZWDIIndex
        (
            tCIDLib::TCard4& c4BufSz_mbufIndex
            , COP THeapBuf& mbufIndex
            , tCIDLib::TCard4& c4BufSz_mbufDTD
            , COP THeapBuf& mbufDTD
        );

        tCIDLib::TBoolean bQueryZWDIFile
        (
            const TString& strFileName
            , tCIDLib::TCard4& c4BufSz_mbufData
            , COP THeapBuf& mbufData
        );

        tCIDLib::TCard4 c4QueryDrvManifests
        (
            COP TVector<TCQCDriverCfg>& colToFill
            , const TCQCSecToken& sectUser
        );

        tCIDLib::TVoid DrvCfgQueryDrvsInfo
        (
            COP TVector<TString>& colMonikers
            , COP TVector<TString>& colMakes
            , COP TVector<TString>& colModels
            , COP TFundVector<tCQCKit::EDevCats>& fcolCats
            , const TCQCSecToken& sectUser
        );

        tCIDLib::TVoid DrvCfgRemoveDrv
        (
            const TString& strMoniker
            , const TString& strHost
            , COP tCIDLib::TCard4& c4NewVersion
            , const TCQCSecToken& sectUser
        );

        tCIDLib::TVoid DrvCfgQueryReportInfo
        (
            COP TVector<TKeyValues>& colToFill
            , const TCQCSecToken& sectUser
        );

        tCIDLib::TVoid DrvCfgSetConfig
        (
            const TString& strMoniker
            , const TString& strHost
            , const TCQCDriverObjCfg& cqcdcToSet
            , const tCIDLib::TBoolean bNewDriver
            , COP tCIDLib::TCard4& c4NewVersion
            , const TCQCSecToken& sectUser
        );

        tCIDLib::ELoadRes eQueryGC100Cfg
        (
            CIOP tCIDLib::TCard4& c4SerialNum
            , tCIDLib::TCard4& c4BufSz_mbufToFill
            , COP THeapBuf& mbufToFill
            , const TCQCSecToken& sectUser
        );

        tCIDLib::ELoadRes eQueryJAPCfg
        (
            CIOP tCIDLib::TCard4& c4SerialNum
            , tCIDLib::TCard4& c4BufSz_mbufToFill
            , COP THeapBuf& mbufToFill
            , const TCQCSecToken& sectUser
        );

        tCIDLib::TVoid Ping();

        tCIDLib::TVoid QueryVoiceFiles
        (
            tCIDLib::TCard4& c4BufSz_mbufTree
            , COP THeapBuf& mbufTree
            , tCIDLib::TCard4& c4BufSz_mbufGrammar
            , COP THeapBuf& mbufGrammar
        );

        tCIDLib::TVoid RenameEMailAccount
        (
            const TString& strOldName
            , const TString& strNewName
            , const TCQCSecToken& sectUser
        );

        tCIDLib::TVoid SetKeyMap
        (
            const TString& strUserName
            , const TCQCKeyMap& kmToSEt
            , const TCQCSecToken& sectUser
        );

        tCIDLib::TBoolean SetLocationInfo
        (
            const tCIDLib::TFloat8 f8Lat
            , const tCIDLib::TFloat8 f8Long
            , const TCQCSecToken& sectUser
        );

        tCIDLib::TVoid UploadProtocol
        (
            const TString& strFileName
            , const tCIDLib::TCard4 c4BufSz_mbufText
            , const THeapBuf& mbufText
            , const tCIDLib::TBoolean bSystem
            , const TCQCSecToken& sectUser
        );

        tCIDLib::TVoid UploadManifest
        (
            const TString& strFileName
            , const tCIDLib::TCard4 c4BufSz_mbufText
            , const THeapBuf& mbufText
            , const tCIDLib::TBoolean bSystem
            , const TCQCSecToken& sectUser
        );

    protected :
        // --------------------------------------------------------------------
        // Declare our friends
        // --------------------------------------------------------------------
        friend class TFacCIDOrb;

    private :
        // --------------------------------------------------------------------
        // Magic macros
        // --------------------------------------------------------------------
        RTTIDefs(TCQCInstClientProxy,TOrbClientBase)
};

#pragma CIDLIB_POPPACK

inline tCIDLib::TVoid
TCQCInstClientProxy::SetObjId(const  TOrbObjId& ooidToSet
              , const TString& strNSBinding
              , const tCIDLib::TBoolean bCheck)
{
    TParent::SetObjId(ooidToSet, strNSBinding, bCheck);
}

