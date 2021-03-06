// ----------------------------------------------------------------------------
//  FILE: CQCDataSrv_SysCfgServerBase.hpp
//  DATE: Fri, Feb 12 21:14:16 2021 -0500
//    ID: 6EBA61ABF3342112-8262C7001241EE53
//
//  This file was generated by the Charmed Quark CIDIDL compiler. Do not make
//  changes by hand, because they will be lost if the file is regenerated.
// ----------------------------------------------------------------------------
#pragma once

#pragma CIDLIB_PACK(CIDLIBPACK)

class  TCQCSysCfgServerBase : public TOrbServerBase
{
    public :
        // --------------------------------------------------------------------
        // Public, static data
        // --------------------------------------------------------------------
        static const TString strInterfaceId;
        static const TString strBinding;


        // --------------------------------------------------------------------
        // Constructors and destructor
        // --------------------------------------------------------------------
        TCQCSysCfgServerBase(const TCQCSysCfgServerBase&) = delete;
        TCQCSysCfgServerBase(TCQCSysCfgServerBase&&) = delete;
        ~TCQCSysCfgServerBase();

        // --------------------------------------------------------------------
        // Public operators
        // --------------------------------------------------------------------
        TCQCSysCfgServerBase& operator=(const TCQCSysCfgServerBase&) = delete;
        TCQCSysCfgServerBase& operator=(TCQCSysCfgServerBase&&) = delete;

        // --------------------------------------------------------------------
        // Public, pure virtual methods
        // --------------------------------------------------------------------
        virtual tCIDLib::TBoolean bQueryRoomCfg
        (
            CIOP tCIDLib::TCard4& c4SerialNum
            , TCQCSysCfg& scfgToFill
        ) = 0;

        virtual tCIDLib::TVoid StoreRoomCfg
        (
            const TCQCSysCfg& scfgToStore
            , COP tCIDLib::TCard4& c4NewSerialNum
            , const TCQCSecToken& sectUser
        ) = 0;

    protected :
        // --------------------------------------------------------------------
        // Hidden constructors
        // --------------------------------------------------------------------
        TCQCSysCfgServerBase();
        TCQCSysCfgServerBase(const TOrbObjId& ooidThis);

        // --------------------------------------------------------------------
        // Protected, inherited methods
        // --------------------------------------------------------------------
        tCIDLib::TVoid Dispatch
        (
            const  TString&      strMethodName
            ,      TOrbCmd&      orbcToDispatch
        )   override;

    private :
        // --------------------------------------------------------------------
        // Magic macros
        // --------------------------------------------------------------------
        RTTIDefs(TCQCSysCfgServerBase,TOrbServerBase)
};

#pragma CIDLIB_POPPACK

