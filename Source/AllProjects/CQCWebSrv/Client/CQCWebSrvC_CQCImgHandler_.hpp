//
// FILE NAME: CQCWebSrv_CQCImgHandler.hpp
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
//  This is the header for the CQCWebSrv_CQCImgHandler.cpp file, which implements
//  the default URL handlers, i.e. the one that just handles file-based
//  requests. If a URL doesn't get claimed by any of the special handlers,
//  this guy will claim it (it'll get put at the end of the list and just
//  always says it'll take the URL presented.)
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TWSCQCImgHandler
//  PREFIX: urlh
// ---------------------------------------------------------------------------
class TWSCQCImgHandler : public TWSURLHandler
{
    public :
        // --------------------------------------------------------------------
        // Constructors and Destructor
        // --------------------------------------------------------------------
        TWSCQCImgHandler();

        TWSCQCImgHandler(const TWSCQCImgHandler&) = delete;
        TWSCQCImgHandler(TWSCQCImgHandler&&) = delete;

        ~TWSCQCImgHandler();


        // --------------------------------------------------------------------
        //  Public operators
        // --------------------------------------------------------------------
        TWSCQCImgHandler& operator=(const TWSCQCImgHandler&) = delete;
        TWSCQCImgHandler& operator=(TWSCQCImgHandler&&) = delete;


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
        RTTIDefs(TWSCQCImgHandler,TWSURLHandler)
};


#pragma CIDLIB_POPPACK


