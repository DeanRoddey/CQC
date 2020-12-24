//
// FILE NAME: CQCAppShell_ServiceHandler.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/09/2002
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
//  This is the header for the service handler for the app shell. When we
//  run in service mode, this is installed as the service handler. It mainly
//  just delegates to the facility class.
//
//  When we are not in service mode, we are just a standard console app.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: TCQCAppShellService
//  PREFIX: srvh
// ---------------------------------------------------------------------------
class TCQCAppShellService : public TWin32ServiceHandler
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCAppShellService();

        TCQCAppShellService(const TCQCAppShellService&) = delete;
        TCQCAppShellService(TCQCAppShellService&&) = delete;

        ~TCQCAppShellService();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCAppShellService& operator=(const TCQCAppShellService&) = delete;
        TCQCAppShellService& operator=(TCQCAppShellService&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bStartServiceThread() final;

        tCIDLib::TBoolean bStopServiceThread() final;

        tCIDLib::TBoolean bSuspendRequest() final;

        tCIDLib::TVoid ServiceOp
        (
            const   tCIDKernel::EWSrvOps    eOperation
        )   final;

        tCIDLib::TVoid ServiceOp
        (
            const   tCIDKernel::EWSrvPwrOps ePowerOp
        )   final;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_pthrService
        //      The thread that we start to do our work, since we do not
        //      run in the context of the service thread. This MUST be a
        //      pointer, because this object gets created before the service
        //      init is done, it can't cause any objects to get created during
        //      construction that might call any system info calls.
        // -------------------------------------------------------------------
        TThread*    m_pthrService;
};

#pragma CIDLIB_POPPACK


