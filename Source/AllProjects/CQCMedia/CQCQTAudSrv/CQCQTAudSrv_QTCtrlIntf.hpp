//
// FILE NAME: CQCQTAudSrv_SrvAdminImpl.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/11/2007
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
//  This is the header for the TQTIntfServerImpl class, which implements the
//  IDL interface provided by CQCKit for CQCQTAudSrv remote admin.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TQTIntfServerImpl
//  PREFIX: orbs
// ---------------------------------------------------------------------------
class TQTIntfServerImpl : public TQTIntfServerBase
{
    public :
        // --------------------------------------------------------------------
        // Constructors and Destructor
        // --------------------------------------------------------------------
        TQTIntfServerImpl();

        ~TQTIntfServerImpl();


        // --------------------------------------------------------------------
        // Public, inherited methods
        // --------------------------------------------------------------------
        tCIDLib::TBoolean bMute
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TCard4 c4Volume
        (
            const   tCIDLib::TCard4         c4ToSet
        );

        tCIDLib::TVoid LoadFile
        (
            const   TString&                strPath
            , const TString&                strAudioDevice
        );

        tCIDLib::TVoid Pause();

        tCIDLib::TVoid Play();

        tCIDLib::TVoid Reset();

        tCIDLib::TVoid Stop();


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid Initialize();

        tCIDLib::TVoid Terminate();


    private :
        // --------------------------------------------------------------------
        //  Unimplemented ctors and operators
        // --------------------------------------------------------------------
        TQTIntfServerImpl(const TQTIntfServerImpl&);
        tCIDLib::TVoid operator=(const TQTIntfServerImpl&);


        // --------------------------------------------------------------------
        //  Private data members
        // --------------------------------------------------------------------


        // --------------------------------------------------------------------
        //  Magic macros
        // --------------------------------------------------------------------
        RTTIDefs(TQTIntfServerImpl,TQTIntfServerBase)
};

#pragma CIDLIB_POPPACK


