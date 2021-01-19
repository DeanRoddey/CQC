// ----------------------------------------------------------------------------
//  FILE: CQCVoice_SysCfgIntfClientProxy.hpp
//  DATE: Tue, Jan 19 17:39:55 2021 -0500
//    ID: 6EBA61ABF3342112-8262C7001241EE53
//
//  This file was generated by the Charmed Quark CIDIDL compiler. Do not make
//  changes by hand, because they will be lost if the file is regenerated.
// ----------------------------------------------------------------------------
#pragma once

#pragma CIDLIB_PACK(CIDLIBPACK)

class  TCQCSysCfgClientProxy : public TOrbClientBase
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
        TCQCSysCfgClientProxy();
        TCQCSysCfgClientProxy(const TOrbObjId& ooidSrc, const TString& strNSBinding);
        TCQCSysCfgClientProxy(const TCQCSysCfgClientProxy&) = delete;
        TCQCSysCfgClientProxy(TCQCSysCfgClientProxy&&) = delete;
        ~TCQCSysCfgClientProxy();

        // --------------------------------------------------------------------
        // Public operators
        // --------------------------------------------------------------------
        TCQCSysCfgClientProxy& operator=(const TCQCSysCfgClientProxy&) = delete;
        TCQCSysCfgClientProxy& operator=(TCQCSysCfgClientProxy&&) = delete;

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
        tCIDLib::TBoolean bQueryRoomCfg
        (
            tCIDLib::TCard4& c4SerialNum
            , TCQCSysCfg& scfgToFill
        );

        tCIDLib::TVoid StoreRoomCfg
        (
            const TCQCSysCfg& scfgToStore
            , tCIDLib::TCard4& c4NewSerialNum
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
        RTTIDefs(TCQCSysCfgClientProxy,TOrbClientBase)
};

#pragma CIDLIB_POPPACK

inline tCIDLib::TVoid
TCQCSysCfgClientProxy::SetObjId(const  TOrbObjId& ooidToSet
              , const TString& strNSBinding
              , const tCIDLib::TBoolean bCheck)
{
    TParent::SetObjId(ooidToSet, strNSBinding, bCheck);
}

