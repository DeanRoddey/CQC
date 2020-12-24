//
// FILE NAME: CQCVoice_CoreAdminImpl.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/02/2016
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
//  This is the header for the CQCVoice_CoreAdminImpl.cpp file, which implements the
//  standard backend server admin interface. This allows us to be controlled from the
//  CQC app shell and other things.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once

#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TCIDCoreAdminImpl
//  PREFIX: orbs
// ---------------------------------------------------------------------------
class TCIDCoreAdminImpl : public TCIDCoreAdminBaseImpl
{
    public :
        // --------------------------------------------------------------------
        // Constructors and Destructor
        // --------------------------------------------------------------------
        TCIDCoreAdminImpl();

        TCIDCoreAdminImpl(const TCIDCoreAdminImpl&) = delete;

        ~TCIDCoreAdminImpl();


        // --------------------------------------------------------------------
        //  Public operators
        // --------------------------------------------------------------------
        TCIDCoreAdminImpl& operator=(const TCIDCoreAdminImpl&) = delete;


        // --------------------------------------------------------------------
        // Public, inherited methods
        // --------------------------------------------------------------------
        tCIDLib::TVoid AdminStop() override;


    protected :
        // --------------------------------------------------------------------
        //  Protected, inherited methods
        // --------------------------------------------------------------------
        tCIDLib::TVoid Initialize() override;

        tCIDLib::TVoid Terminate() override;


    private :
        // --------------------------------------------------------------------
        //  Magic macros
        // --------------------------------------------------------------------
        RTTIDefs(TCIDCoreAdminImpl,TCIDCoreAdminServerBase)
};

#pragma CIDLIB_POPPACK

