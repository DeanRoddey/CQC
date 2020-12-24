//
// FILE NAME: CQCAppShell_ThisFacility.hpp
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
//  This is the header for the facility class for the app shell.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


// ---------------------------------------------------------------------------
//   CLASS: TFacCQCAppShell
//  PREFIX: fac
// ---------------------------------------------------------------------------
class TFacCQCAppShell : public TFacility
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TFacCQCAppShell();

        TFacCQCAppShell(const TFacCQCAppShell&) = delete;
        TFacCQCAppShell(TFacCQCAppShell&&) = delete;

        ~TFacCQCAppShell();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TFacCQCAppShell& operator=(const TFacCQCAppShell&) = delete;
        TFacCQCAppShell& operator=(TFacCQCAppShell&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bQueryNewMsgs
        (
                    tCIDLib::TCard4&        c4MsgIndex
            ,       tCIDLib::TStrList&      colMsgs
            , const tCIDLib::TBoolean       bAddNewLine
        );

        tCIDLib::TCard4 c4QueryApps
        (
                    tCIDLib::TKVPList&      colApps
        );

        tCIDLib::TVoid CycleApps();

        tCIDLib::EExitCodes eMainThread
        (
                    TThread&                thrThis
            ,       tCIDLib::TVoid*         pData
        );

        tCIDLib::TVoid StartAllApps();

        tCIDLib::TVoid StartShutdown();

        tCIDLib::TVoid StopAllApps();


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid ParseParms();

        tCIDLib::TVoid RemoveBindings();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4MaxClients
        //      The max simultaneous clients to support. If its not provided
        //      on the command line, we allow the default Orb max to be used.
        //
        //  m_evShutdown
        //      Because we have to keep sending "we're working on it" messages
        //      to the service handler, we cannot use the usual blocking
        //      shutdown request stuff built into TThread. So the service
        //      thread triggers this event to kick us into shutting down, and
        //      then it just waits for us to die by looping and tell the
        //      service manager to wait.
        //
        //  m_ippnListen
        //      The port to register our ORB listener thread on. It has a
        //      default but can be overridden on the command line.
        //
        //  m_porbsAdmin
        //      Our admin object. We don't own it, the ORB does. We just keep
        //      a typed pointer for ourself, for convenience. It's only used
        //      in this service based version.
        // -------------------------------------------------------------------
        tCIDLib::TCard4     m_c4MaxClients;
        TEvent              m_evShutdown;
        tCIDLib::TIPPortNum m_ippnListen;
        TCQCAppShellImpl*   m_porbsAdmin;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TFacCQCAppShell,TFacility)
};



