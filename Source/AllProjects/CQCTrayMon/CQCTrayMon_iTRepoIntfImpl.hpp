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
        TiTunesRepoCtrlImpl
        (
                    TCQCTrayiTunesTab* const pwndTab
        );

        ~TiTunesRepoCtrlImpl();


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
        );

        tCIDLib::TBoolean bQueryData2
        (
            const   TString&                strQueryType
            ,       tCIDLib::TCard4&        c4BufSz
            ,       THeapBuf&               mbufIO
        );

        tCIDLib::TBoolean bQueryTextVal
        (
            const   TString&                strQueryType
            , const TString&                strDataName
            ,       TString&                strToFill
        );

        tCIDLib::TBoolean bSendData
        (
            const   TString&                strSendType
            ,       TString&                strDataName
            ,       tCIDLib::TCard4&        c4BufSz
            ,       THeapBuf&               mbufToSend
        );

        tCIDLib::TCard4 c4QueryVal
        (
            const   TString&                strValId
        );

        tCIDLib::TCard4 c4SendCmd
        (
            const   TString&                strCmdId
            , const TString&                strParms
        );

        tCIDLib::TVoid Initialize();

        tCIDLib::TVoid QueryStatus
        (
                    tCQCMedia::ELoadStatus& eCurStatus
            ,       tCQCMedia::ELoadStatus& eLoadStatus
            ,       TString&                strDBSerialNum
            ,       tCIDLib::TCard4&        c4TitleCnt
        );

        tCIDLib::TBoolean bReloadDB();

        tCIDLib::TVoid Terminate();


    private :
        // --------------------------------------------------------------------
        // Unimplemented ctors and operators
        // --------------------------------------------------------------------
        TiTunesRepoCtrlImpl(const TiTunesRepoCtrlImpl&);
        tCIDLib::TVoid operator=(const TiTunesRepoCtrlImpl&);


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


