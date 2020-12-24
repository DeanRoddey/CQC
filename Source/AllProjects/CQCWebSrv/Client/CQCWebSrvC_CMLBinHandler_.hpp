//
// FILE NAME: CQCWebSrvC_CMLBinHandler.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/09/2005
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
//  This is the header for the CQCWebSrv_CMLBinHandler.cpp file, which implements
//  the 'CML-Bin' handler. Any URLs that start with /CMLBin/User/ will be passed
//  to us. We will convert that URL to a CML class path. That path must derive from
//  the CML-Bin base class. If so, we'll dispatch the query info to it and it will
//  give back the data to return (or error info.)
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TWSCMLBinHandler
//  PREFIX: urlh
// ---------------------------------------------------------------------------
class TWSCMLBinHandler : public TWSURLHandler
{
    public :
        // --------------------------------------------------------------------
        // Constructors and Destructor
        // --------------------------------------------------------------------
        TWSCMLBinHandler();

        TWSCMLBinHandler(const TWSCMLBinHandler&) = delete;
        TWSCMLBinHandler(TWSCMLBinHandler&&) = delete;

        ~TWSCMLBinHandler();


        // --------------------------------------------------------------------
        //  Public operators
        // --------------------------------------------------------------------
        TWSCMLBinHandler& operator=(const TWSCMLBinHandler&) = delete;
        TWSCMLBinHandler& operator=(TWSCMLBinHandler&&) = delete;


        // --------------------------------------------------------------------
        //  Public, inherited methods
        // --------------------------------------------------------------------
        tCIDLib::TCard4 c4ProcessURL
        (
            const   TURL&                   urlReq
            , const TString&                strType
            , const tCIDLib::TKVPList&      colInHdrLines
            ,       tCIDLib::TKVPList&      colOutHdrLines
            , const tCIDLib::TKVPList&      colPQVals
            , const TString&                strEncoding
            , const TString&                strBoundary
            ,       THeapBuf&               mbufToFill
            ,       tCIDLib::TCard4&        c4ContLen
            ,       TString&                strContType
            ,       TString&                strRepText
        )   final;


    private :
        // --------------------------------------------------------------------
        //  Magic macros
        // --------------------------------------------------------------------
        RTTIDefs(TWSCMLBinHandler,TWSURLHandler)
};


#pragma CIDLIB_POPPACK


