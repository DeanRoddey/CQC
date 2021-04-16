// ----------------------------------------------------------------------------
//  FILE: CQLogicSrv_LSIntfServerBase.hpp
//  DATE: Fri, Feb 12 21:14:16 2021 -0500
//    ID: 2B54D6A15C078D54-8E985C2CDE56A303
//
//  This file was generated by the Charmed Quark CIDIDL compiler. Do not make
//  changes by hand, because they will be lost if the file is regenerated.
// ----------------------------------------------------------------------------
#pragma once

#pragma CIDLIB_PACK(CIDLIBPACK)

class  TLogicSrvServerBase : public TOrbServerBase
{
    public :
        // --------------------------------------------------------------------
        // Public, static data
        // --------------------------------------------------------------------
        static const TString strInterfaceId;
        static const TString strBinding;
        static const TString strCoreAdminBinding;


        // --------------------------------------------------------------------
        // Constructors and destructor
        // --------------------------------------------------------------------
        TLogicSrvServerBase(const TLogicSrvServerBase&) = delete;
        TLogicSrvServerBase(TLogicSrvServerBase&&) = delete;
        ~TLogicSrvServerBase();

        // --------------------------------------------------------------------
        // Public operators
        // --------------------------------------------------------------------
        TLogicSrvServerBase& operator=(const TLogicSrvServerBase&) = delete;
        TLogicSrvServerBase& operator=(TLogicSrvServerBase&&) = delete;

        // --------------------------------------------------------------------
        // Public, pure virtual methods
        // --------------------------------------------------------------------
        virtual tCIDLib::TBoolean bQueryFields
        (
            const tCIDLib::TCard4 c4CfgSerialNum
            , tCIDLib::TCard4& c4BufSz_mbufData
            , COP THeapBuf& mbufData
            , const tCIDLib::TBoolean bReturnAll
        ) = 0;

        virtual tCIDLib::TBoolean bQueryGraphSamples
        (
            const TString& strFldName
            , CIOP tCIDLib::TCard4& c4CfgSerialNum
            , CIOP tCIDLib::TCard4& c4SerialNum
            , COP tCIDLib::TCard4& c4NewSamples
            , COP TFundVector<tCIDLib::TFloat4>& fcolNewSamples
        ) = 0;

        virtual tCIDLib::TCard4 c4QueryGraphFlds
        (
            COP TVector<TString>& colToFill
        ) = 0;

        virtual tCIDLib::TVoid QueryConfig
        (
            COP TCQLogicSrvCfg& lscfgToFill
            , COP tCIDLib::TCard4& c4CfgSerialNum
        ) = 0;

        virtual tCIDLib::TVoid ResetElTimeFld
        (
            const TString& strField
        ) = 0;

        virtual tCIDLib::TVoid SetConfig
        (
            const TCQLogicSrvCfg& lscfgNew
            , COP tCIDLib::TCard4& c4CfgSerialNum
        ) = 0;

    protected :
        // --------------------------------------------------------------------
        // Hidden constructors
        // --------------------------------------------------------------------
        TLogicSrvServerBase();
        TLogicSrvServerBase(const TOrbObjId& ooidThis);

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
        RTTIDefs(TLogicSrvServerBase,TOrbServerBase)
};

#pragma CIDLIB_POPPACK

