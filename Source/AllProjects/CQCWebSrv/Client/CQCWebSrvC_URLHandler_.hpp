//
// FILE NAME: CQCWebSrv_URLHandler.hpp
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
//  This is the header for the CQCWebSrv_URLHandler.cpp file, which defines
//  an interface for responding to specific URL types with corresponding data
//  output.
//
//  There are various 'magic' areas of the URL namespace, such as /CMLBin/
//  for macro-based pages, /CQCImg/ for retrieving images from the CQC image
//  repository, or the general file-based retrieval if not one of the special
//  ones. And there will likely be others in the future for particular types
//  of 'extensions', such as PHP or whatever.
//
//  To make things flexible, and to keep the main client servicing logic
//  clean, we define an abstract base class for a 'URL handler' that knows
//  how to provide the output for a given type of URL, and then we derive
//  various specific classes for the types of handlers we need. Each worker
//  thread keeps one of each type around, faulting them in upon actual need.
//
//  The return value from c4ProcessURL is the HTTP result code. If it's not
//  200, then the strContType should be filled in with the result text you
//  want sent back on the first line (it must be all simple ASCII.)
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TWSURLHandler
//  PREFIX: thr
// ---------------------------------------------------------------------------
class TWSURLHandler : public TObject
{
    public :
        // --------------------------------------------------------------------
        //  Constructors and destructor
        // --------------------------------------------------------------------
        TWSURLHandler(const TWSURLHandler&) = delete;
        TWSURLHandler(TWSURLHandler&&) = delete;

        ~TWSURLHandler();


        // --------------------------------------------------------------------
        //  Public operators
        // --------------------------------------------------------------------
        TWSURLHandler& operator=(const TWSURLHandler&) = delete;
        TWSURLHandler& operator=(TWSURLHandler&&) = delete;


        // --------------------------------------------------------------------
        //  Public, virtual methods
        // --------------------------------------------------------------------
        virtual tCIDLib::TCard4 c4ProcessURL
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
        ) = 0;


    protected :
        // --------------------------------------------------------------------
        // Hidden Constructors
        // --------------------------------------------------------------------
        TWSURLHandler();


    private :
        // --------------------------------------------------------------------
        //  Magic macros
        // --------------------------------------------------------------------
        RTTIDefs(TWSURLHandler,TObject)
};

#pragma CIDLIB_POPPACK

