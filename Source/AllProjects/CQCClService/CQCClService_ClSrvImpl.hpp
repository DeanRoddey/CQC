//
// FILE NAME: CQCClService_ClSrvImpl.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/21/2013
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
//  This is the header for the class that implements our remote interface,
//  via which clients access our services. We derive from the IDL generated
//  server stubb and implement the callbacks.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once

#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TCQCClSrvImpl
//  PREFIX: orbs
// ---------------------------------------------------------------------------
class TCQCClSrvImpl : public TCQCClServiceServerBase
{
    public :
        // --------------------------------------------------------------------
        // Constructors and Destructor
        // --------------------------------------------------------------------
        TCQCClSrvImpl() = default;

        TCQCClSrvImpl(const TCQCClSrvImpl&) = delete;
        TCQCClSrvImpl(TCQCClSrvImpl&&) = delete;

        ~TCQCClSrvImpl() = default;


        // --------------------------------------------------------------------
        //  Public operators
        // --------------------------------------------------------------------
        TCQCClSrvImpl& operator=(const TCQCClSrvImpl&) = delete;
        TCQCClSrvImpl& operator=(TCQCClSrvImpl&&) = delete;


        // --------------------------------------------------------------------
        // Public, inherited methods
        // --------------------------------------------------------------------
        tCIDLib::TBoolean bQueryRepoDB
        (
            const   TString&                strRepoMon
            , const TString&                strCurSerialNum
            ,       tCIDLib::TCard4&        c4BufSz_mbufData
            ,       THeapBuf&               mbufData
        )   final;

        tCIDLib::TCard4 c4QueryRepoList
        (
                    tCIDLib::TStrList&      colToFill
        )   final;


    protected :
        // --------------------------------------------------------------------
        //  Protected, inherited methods
        // --------------------------------------------------------------------
        tCIDLib::TVoid Initialize() final;

        tCIDLib::TVoid Terminate() final;


    private :
        // --------------------------------------------------------------------
        //  Magic macros
        // --------------------------------------------------------------------
        RTTIDefs(TCQCClSrvImpl,TCQCClServiceServerBase)
};

#pragma CIDLIB_POPPACK


