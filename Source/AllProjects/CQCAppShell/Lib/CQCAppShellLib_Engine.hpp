//
// FILE NAME: CQCAppShellLib_Engine.hpp
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
//  This is the header for the CQCAppShell_Engine.cpp file, which implements
//  a simple derivative of the core CIDLib app shell engine. We have to
//  override some virtuals in order to provide some of the application
//  specific functionality.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TCQCAppShEngine
//  PREFIX: aseng
// ---------------------------------------------------------------------------
class TCQCAppShEngine : public TAppShEngine
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCAppShEngine();

        TCQCAppShEngine(const TCQCAppShEngine&) = delete;
        TCQCAppShEngine(TCQCAppShEngine&&) = delete;

        ~TCQCAppShEngine();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCAppShEngine& operator=(const TCQCAppShEngine&) = delete;
        TCQCAppShEngine& operator=(TCQCAppShEngine&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bDoWaitCalls() const;

        tCIDLib::TBoolean bDoWaitCalls
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TCard4 c4ErrCount() const;

        tCIDLib::TVoid ResetErrCount();


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCheckShutdownComplete
        (
                    TAppShRecord&           asrTarget
            ,       tCIDLib::EExitCodes&    eToFill
        )   final;

        tCIDLib::TVoid Exited
        (
                    TAppShRecord&           asrExited
            , const tCIDLib::EExitCodes     eCode
        )   final;

        tCIDLib::TVoid IdleCallback() final;

        tCIDLib::TVoid Restarted
        (
                    TAppShRecord&           asrRestarted
        )   final;

        tCIDLib::TVoid ShellException
        (
                    TError&                 errThatOccured
            ,       TAppShRecord&           asrInvolved
        )   final;

        tCIDLib::TVoid ShellMessage
        (
            const   TString&                strMsg
        )   final;

        tCIDLib::TVoid ShellUnknownException
        (
                    TAppShRecord&           asrInvolved
        )   final;

        tCIDLib::TVoid Started
        (
                    TAppShRecord&           asrRestarted
        )   final;

        tCIDLib::TVoid StartShutdown
        (
                    TAppShRecord&           asrTarget
        )   final;


        // -------------------------------------------------------------------
        //  Protected, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid ShutdownProcess
        (
                    TAppShRecord&           asrTarget
        );


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bDoWaitCalls
        //      If this is set, our IdleCallback() method will make calls
        //      to the service manager, doing wait calls to keep it from
        //      timing us out.
        //
        //  m_c4ErrCount
        //      This is bumped when we get an exception. The app shell can
        //      reset it and get it, and uses it to track errors during some
        //      operations.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bDoWaitCalls;
        tCIDLib::TCard4     m_c4ErrCount;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCAppShEngine,TAppShEngine)
};

#pragma CIDLIB_POPPACK

