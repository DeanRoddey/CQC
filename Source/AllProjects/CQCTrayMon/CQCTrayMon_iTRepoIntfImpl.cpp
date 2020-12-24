//
// FILE NAME: CQCTrayMon_iTRepoIntfImpl.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/14/2012
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
//  This implements the IDL generated server stub for the protocol interface
//  that the iTunes repo driver uses to talk to the tray monitor.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCTrayMon.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TiTunesRepoCtrlImpl,TiTRepoIntfServerBase)



// ---------------------------------------------------------------------------
//  CLASS: TiTunesRepoCtrlImpl
// PREFIX: orbs
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TiTunesRepoCtrlImpl: Constructors and Destructor
// ---------------------------------------------------------------------------
TiTunesRepoCtrlImpl::TiTunesRepoCtrlImpl(TCQCTrayiTunesTab* const pwndTab) :

    m_pwndTab(pwndTab)
{
}


TiTunesRepoCtrlImpl::~TiTunesRepoCtrlImpl()
{
}


// ---------------------------------------------------------------------------
//  TiTunesRepoCtrlImpl: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  We just pass the calls on to the iTunes tab which will do any required
//  locking and pass the call on to the repo engine it manages.
//
tCIDLib::TBoolean
TiTunesRepoCtrlImpl::bQueryData(const   TString&            strQType
                                , const TString&            strDataName
                                ,       tCIDLib::TCard4&    c4BufSz
                                ,       THeapBuf&           mbufToFill)
{
    return m_pwndTab->bQueryData(strQType, strDataName, c4BufSz, mbufToFill);
}


tCIDLib::TBoolean
TiTunesRepoCtrlImpl::bQueryData2(const  TString&            strQType
                                ,       tCIDLib::TCard4&    c4BufSz
                                ,       THeapBuf&           mbufIO)
{
    return m_pwndTab->bQueryData2(strQType, c4BufSz, mbufIO);
}


tCIDLib::TBoolean
TiTunesRepoCtrlImpl::bQueryTextVal( const   TString&    strQType
                                    , const TString&    strDataName
                                    ,       TString&    strToFill)
{
    return m_pwndTab->bQueryTextVal(strQType, strDataName, strToFill);
}


tCIDLib::TBoolean
TiTunesRepoCtrlImpl::bSendData( const   TString&            strSType
                                ,       TString&            strDataName
                                ,       tCIDLib::TCard4&    c4BufSz
                                ,       THeapBuf&           mbufToSend)
{
    return m_pwndTab->bSendData(strSType, strDataName, c4BufSz, mbufToSend);
}


tCIDLib::TCard4 TiTunesRepoCtrlImpl::c4QueryVal(const TString& strValId)
{
    return m_pwndTab->c4QueryVal(strValId);
}


tCIDLib::TCard4
TiTunesRepoCtrlImpl::c4SendCmd(const TString& strCmdId, const TString& strParms)
{
    return m_pwndTab->c4SendCmd(strCmdId, strParms);
}


tCIDLib::TVoid TiTunesRepoCtrlImpl::Initialize()
{
}


tCIDLib::TVoid
TiTunesRepoCtrlImpl::QueryStatus(tCQCMedia::ELoadStatus&    eCurStatus
                                , tCQCMedia::ELoadStatus&   eLoadStatus
                                , TString&                  strDBSerialNum
                                , tCIDLib::TCard4&          c4TitleCnt)
{
    m_pwndTab->QueryStatus(eCurStatus, eLoadStatus, strDBSerialNum, c4TitleCnt);
}



tCIDLib::TBoolean TiTunesRepoCtrlImpl::bReloadDB()
{
    return m_pwndTab->bReloadDB();
}


tCIDLib::TVoid TiTunesRepoCtrlImpl::Terminate()
{
}

