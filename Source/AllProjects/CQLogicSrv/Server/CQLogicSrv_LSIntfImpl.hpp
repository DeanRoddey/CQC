//
// FILE NAME: CQLogicSrv_LSIntfImpl.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/21/2009
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
//  This is the implementation of the logic server IDL interface.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TLogicSrvImpl
//  PREFIX: orbs
// ---------------------------------------------------------------------------
class TLogicSrvImpl : public TLogicSrvServerBase
{
    public :
        // --------------------------------------------------------------------
        //  Constructors and Destructor
        // --------------------------------------------------------------------
        TLogicSrvImpl() = default;

        TLogicSrvImpl(const TLogicSrvImpl&) = delete;
        TLogicSrvImpl(TLogicSrvImpl&&) = delete;

        ~TLogicSrvImpl() = default;


        // --------------------------------------------------------------------
        //  Public operators
        // --------------------------------------------------------------------
        TLogicSrvImpl& operator=(const TLogicSrvImpl&) = delete;
        TLogicSrvImpl& operator=(TLogicSrvImpl&&) = delete;


        // --------------------------------------------------------------------
        //  Public, inherited methods
        // --------------------------------------------------------------------
        tCIDLib::TBoolean bQueryFields
        (
            const   tCIDLib::TCard4         c4CfgSerialNum
            ,       tCIDLib::TCard4&        c4BufSz
            ,       THeapBuf&               mbufData
            , const tCIDLib::TBoolean       bReturnAll
        )   final;

        tCIDLib::TBoolean bQueryGraphSamples
        (
            const   TString&                strFldName
            ,       tCIDLib::TCard4&        c4CfgSerialNum
            ,       tCIDLib::TCard4&        c4SerialNum
            ,       tCIDLib::TCard4&        c4NewSamples
            ,       TFundVector<tCIDLib::TFloat4>& fcolNewSamples
        )   final;

        tCIDLib::TCard4 c4QueryGraphFlds
        (
                    TVector<TString>&       colToFill
        )   final;

        tCIDLib::TVoid QueryConfig
        (
                    TCQLogicSrvCfg&         cfgToFill
            ,       tCIDLib::TCard4&        c4SerialNum
        )   final;

        tCIDLib::TVoid ResetElTimeFld
        (
            const   TString&                strFldName
        )   final;

        tCIDLib::TVoid SetConfig
        (
            const   TCQLogicSrvCfg&         cfgToSet
            ,       tCIDLib::TCard4&        c4SerialNum
        )   final;


    protected :
        // --------------------------------------------------------------------
        //  Protected, inherited methods
        // --------------------------------------------------------------------
        tCIDLib::TVoid Initialize() final;

        tCIDLib::TVoid Terminate() final;


    private :
        // --------------------------------------------------------------------
        // Magic macros
        // --------------------------------------------------------------------
        RTTIDefs(TLogicSrvImpl,TLogicSrvServerBase)
};

#pragma CIDLIB_POPPACK



