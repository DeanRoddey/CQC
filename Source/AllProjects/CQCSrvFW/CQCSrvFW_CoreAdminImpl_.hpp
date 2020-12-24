//
// FILE NAME: CQCSrvFW_CoreAdminImpl_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/30/2001
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
//  This is the header for the CQCSrvFW_CoreAdminImpl.cpp file, which
//  implements the TCIDCoreAdminImpl class. This is our implementation of the
//  CIDOrbUC defined core admin remote protocol. It allows us to be controlled
//  by any management client that understands this protocol.
//
//  We provide this functionality on behalf of our derived classes.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


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
        TCIDCoreAdminImpl() = delete;

        TCIDCoreAdminImpl
        (
                    TCQCSrvCore* const      pfacOwner
        );

        TCIDCoreAdminImpl(const TCIDCoreAdminImpl&) = delete;

        ~TCIDCoreAdminImpl();


        // --------------------------------------------------------------------
        //  Public operators
        // --------------------------------------------------------------------
        TCIDCoreAdminImpl& operator=(const TCIDCoreAdminImpl&) = delete;


        // --------------------------------------------------------------------
        // Public, inherited methods
        // --------------------------------------------------------------------
        tCIDLib::TVoid AdminStop() override
        {
            m_pfacOwner->Shutdown(tCIDLib::EExitCodes::AdminStop);
        }


    protected :
        // --------------------------------------------------------------------
        //  Protected, inherited methods
        // --------------------------------------------------------------------
        tCIDLib::TVoid Initialize() override;

        tCIDLib::TVoid Terminate() override;


    private :
        // --------------------------------------------------------------------
        //  Private data members
        //
        //  m_pfacOwner
        //      The server class gives us a pointer to himself. This lets us get
        //      the facility name and to call some protected methods on him (we
        //      are a friend.)
        // --------------------------------------------------------------------
        TCQCSrvCore*    m_pfacOwner;


        // --------------------------------------------------------------------
        //  Magic macros
        // --------------------------------------------------------------------
        RTTIDefs(TCIDCoreAdminImpl,TCIDCoreAdminServerBase)
};

#pragma CIDLIB_POPPACK

