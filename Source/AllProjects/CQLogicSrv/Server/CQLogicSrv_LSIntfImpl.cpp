//
// FILE NAME: CQLogicSrv_LSIntfImpl.cpp
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
//  This file implements our implementation of this server's IDL based
//  remote interface. We just delegate to the facility class.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQLogicSrv.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TLogicSrvImpl,TLogicSrvServerBase)



// ---------------------------------------------------------------------------
//   CLASS: TLogicSrvImpl
//  PREFIX: orbs
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TLogicSrvImpl: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  All of these just forward the calls to the facility class
//
tCIDLib::TBoolean
TLogicSrvImpl::bQueryFields(const   tCIDLib::TCard4     c4CfgSerialNum
                            ,       tCIDLib::TCard4&    c4BufSz
                            ,       THeapBuf&           mbufData
                            , const tCIDLib::TBoolean   bReturnAll)
{
    return facCQLogicSrv.bQueryFields(c4CfgSerialNum, c4BufSz, mbufData, bReturnAll);
}


tCIDLib::TBoolean TLogicSrvImpl::
bQueryGraphSamples( const   TString&                        strFldName
                    ,       tCIDLib::TCard4&                c4CfgSerialNum
                    ,       tCIDLib::TCard4&                c4SerialNum
                    ,       tCIDLib::TCard4&                c4NewSamples
                    ,       TFundVector<tCIDLib::TFloat4>&  fcolNewSamples)
{
    return facCQLogicSrv.bQueryGraphSamples
    (
        strFldName, c4CfgSerialNum, c4SerialNum, c4NewSamples, fcolNewSamples
    );
}


tCIDLib::TCard4 TLogicSrvImpl::c4QueryGraphFlds(TVector<TString>& colToFill)
{
    return facCQLogicSrv.c4QueryGraphFlds(colToFill);
}


tCIDLib::TVoid
TLogicSrvImpl::QueryConfig( TCQLogicSrvCfg&     cfgToFill
                            , tCIDLib::TCard4&  c4SerialNum)
{
    facCQLogicSrv.QueryConfig(cfgToFill, c4SerialNum);
}



tCIDLib::TVoid TLogicSrvImpl::ResetElTimeFld(const TString& strFldName)
{
    facCQLogicSrv.ResetElTimeFld(strFldName);
}



tCIDLib::TVoid
TLogicSrvImpl::SetConfig(const  TCQLogicSrvCfg&     cfgToSet
                        ,       tCIDLib::TCard4&    c4SerialNum)
{
    facCQLogicSrv.SetConfig(cfgToSet, c4SerialNum);
}


// ---------------------------------------------------------------------------
//  TLogicSrvImpl: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TLogicSrvImpl::Initialize()
{
}


tCIDLib::TVoid TLogicSrvImpl::Terminate()
{
}


