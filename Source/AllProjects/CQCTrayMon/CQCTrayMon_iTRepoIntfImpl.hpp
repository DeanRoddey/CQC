//
// FILE NAME: CQCTrayMon_iTRepoIntfImpl.hpp
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
//  This is the header for the CQCTrayMon_iTunesIntfImpl.cpp file, which
//  provides the implementation of the IDL generated server stub for the
//  iTunes portion of the tray monitor app.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//
#pragma once


class TCQCTrayiTunesTab;

// ---------------------------------------------------------------------------
//  CLASS: TiTunesRepoCtrlImpl
// PREFIX: orbs
// ---------------------------------------------------------------------------
class TiTunesRepoCtrlImpl : public TiTRepoIntfServerBase
{
    public :
        // --------------------------------------------------------------------
        // Constructors and Destructor
        // --------------------------------------------------------------------
        TiTunesRepoCtrlImpl() = delete;

        TiTunesRepoCtrlImpl
        (
                    TCQCTrayiTunesTab* const pwndTab
        );

        TiTunesRepoCtrlImpl(const TiTunesRepoCtrlImpl&) = delete;
        TiTunesRepoCtrlImpl(TiTunesRepoCtrlImpl&&) = delete;

        ~TiTunesRepoCtrlImpl();


        // --------------------------------------------------------------------
        // Unimplemented ctors and operators
        // --------------------------------------------------------------------
        TiTunesRepoCtrlImpl& operator=(const TiTunesRepoCtrlImpl&) = delete;
        TiTunesRepoCtrlImpl& operator=(TiTunesRepoCtrlImpl&&) = delete;


        // --------------------------------------------------------------------
        // Public, inherited methods
        // --------------------------------------------------------------------

        // Repo oriented
        tCIDLib::TBoolean bQueryData
        (
            const   TString&                strQueryType
            , const TString&                strDataName
            ,       tCIDLib::TCard4&        c4BufSz
            ,       THeapBuf&               mbufToFill
        )   final;

        tCIDLib::TBoolean bQueryData2
        (
            const   TString&                strQueryType
            ,       tCIDLib::TCard4&        c4BufSz
            ,       THeapBuf&               mbufIO
        )   final;

        tCIDLib::TBoolean bQueryTextVal
        (
            const   TString&                strQueryType
            , const TString&                strDataName
            ,       TString&                strToFill
        )   final;

        tCIDLib::TBoolean bSendData
        (
            const   TString&                strSendType
            ,       TString&                strDataName
            ,       tCIDLib::TCard4&        c4BufSz
            ,       THeapBuf&               mbufToSend
        )   final;

        tCIDLib::TCard4 c4QueryVal
        (
            const   TString&                strValId
        )   final;

        tCIDLib::TCard4 c4SendCmd
        (
            const   TString&                strCmdId
            , const TString&                strParms
        )   final;

        tCIDLib::TVoid Initialize() final;

        tCIDLib::TVoid QueryStatus
        (
                    tCQCMedia::ELoadStatus& eCurStatus
            ,       tCQCMedia::ELoadStatus& eLoadStatus
            ,       TString&                strDBSerialNum
            ,       tCIDLib::TCard4&        c4TitleCnt
        )   final;

        tCIDLib::TBoolean bReloadDB() final;

        tCIDLib::TVoid Terminate() final;


    private :
        // --------------------------------------------------------------------
        //  Private data members
        //
        //  m_pwndTab
        //      The tab that owns us gives us a pointer to himself since we
        //      have to pass on the incoming calls to him.
        // --------------------------------------------------------------------
        TCQCTrayiTunesTab*  m_pwndTab;


        // --------------------------------------------------------------------
        // Magic macros
        // --------------------------------------------------------------------
        RTTIDefs(TiTunesRepoCtrlImpl, TiTRepoIntfServerBase)
};


