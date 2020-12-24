//
// FILE NAME: CQCWebSrv_FileHandler.hpp
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
//  This is the header for the CQCWebSrv_FileHandler.cpp file, which implements
//  the default URL handlers, i.e. the one that just handles file-based
//  requests. If a URL doesn't get claimed by any of the special handlers,
//  this guy will claim it (it'll get put at the end of the list and just
//  always says it'll take the URL presented.) It'll try to process it as a file
//  and fail if that doesn't work.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TWSFileHandler
//  PREFIX: urlh
// ---------------------------------------------------------------------------
class TWSFileHandler : public TWSURLHandler
{
    public :
        // --------------------------------------------------------------------
        // Constructors and Destructor
        // --------------------------------------------------------------------
        TWSFileHandler();

        TWSFileHandler(const TWSFileHandler&) = delete;
        TWSFileHandler(TWSFileHandler&&) = delete;

        ~TWSFileHandler();


        // --------------------------------------------------------------------
        //  Public operators
        // --------------------------------------------------------------------
        TWSFileHandler& operator=(const TWSFileHandler&) = delete;
        TWSFileHandler& operator=(TWSFileHandler&&) = delete;


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
            , const TString&
            ,       THeapBuf&               mbufToFill
            ,       tCIDLib::TCard4&        c4ContLen
            ,       TString&                strContType
            ,       TString&                strRepText
        )   final;


    private :
        // --------------------------------------------------------------------
        //  Magic macros
        // --------------------------------------------------------------------
        RTTIDefs(TWSFileHandler,TWSURLHandler)
};


#pragma CIDLIB_POPPACK


