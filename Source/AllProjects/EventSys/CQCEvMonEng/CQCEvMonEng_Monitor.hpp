//
// FILE NAME: CQCEvMonEng_Monitor.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/08/2014
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
//  This file defines the actual monitor class that the event server creates (at
//  runtime) for each configured monitor. This class provides the thread that the
//  monitor runs on and the macro engine it runs in.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TCQCEvMonitor
//  PREFIX: thr
// ---------------------------------------------------------------------------
class CQCEVMONENGEXP TCQCEvMonitor : public TThread
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCEvMonitor() = delete;

        TCQCEvMonitor
        (
            const   TString&                strPath
            , const TCQCEvMonCfg&           emoncSrc
        );

        TCQCEvMonitor(const TCQCEvMonitor&) = delete;
        TCQCEvMonitor(TCQCEvMonitor&&) = delete;

        ~TCQCEvMonitor();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        tCIDLib::TVoid operator=(const TCQCEvMonitor&);
        tCIDLib::TVoid operator=(TCQCEvMonitor&&);


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bFailed() const;

        tCIDLib::TBoolean bFailed
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TBoolean bPaused() const;

        tCIDLib::TBoolean bPaused
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TVoid StartMonitor
        (
            const   TCQCUserCtx&            cuctxToUse
        );

        tCIDLib::TCard4 c4SerialNum() const;

        tCIDLib::TCard4 c4SerialNum
        (
            const   tCIDLib::TCard4         c4ToSet
        );

        tCIDLib::TVoid DoMonitorLoop();

        const TCQCEvMonCfg& emoncThis() const;

        TCQCEvMonCfg& emoncThis();

        const TString& strPath() const;

        const TString& strPath
        (
            const   TString&                strToSet
        );

        tCIDLib::TVoid SetConfig
        (
            const   TString&                strPath
            , const TCQCEvMonCfg&           emoncToSet
        );

        tCIDLib::TVoid SetFieldList
        (
            const   tCIDLib::TStrList&      colToSet
        );

        tCIDLib::TVoid SetIdleSecs
        (
            const   tCIDLib::TCard4         c4ToSet
        );

        tCIDLib::TVoid Simulate
        (
                    TCIDMacroEngine&        meOwner
            ,       TMEngClassVal&          mecvInstance
            , const TString&                strClassPath
        );

        tCIDLib::TVoid StopMonitor();


    protected :
        // --------------------------------------------------------------------
        //  Protected, inherited methods
        // --------------------------------------------------------------------
        tCIDLib::EExitCodes eProcess() final;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bDoInit();

        tCIDLib::TBoolean bDoLoadConfig();

        tCIDLib::TBoolean bMonWait
        (
                    TThread&                thrCaller
            , const tCIDLib::TCard4         c4Millis
        );

        tCIDLib::TVoid Cleanup();

        tCIDLib::TVoid DoIdleCallback();

        tCIDLib::TVoid PollFields();



        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bSimMode
        //      To remember if have been invoked in simulator mode.
        //
        //  m_c4IdleSecs
        //      The seconds between idle time callbacks. It defaults to 30 seconds,
        //      but can be set to as low as 5 seconds by the CML handler.
        //
        //  m_c4SerialNum
        //      A serial number for use by client code, usually for a server to make
        //      syncing with the client more efficient.
        //
        //  m_colFieldList
        //      The list of pollinfo objects that we get when we register fields for
        //      monitoring.
        //
        //  m_emoncThis
        //      The configuration info for our monitor. This is where we store the
        //      config in the event server at runtime, so that the thread and config
        //      are stored together.
        //
        //  m_pmecvHandler
        //  m_pmeciHandler
        //      During the init, we keep around pointers to the CML handler's value and
        //      info classes, since we need to use them a lot when making calls to the
        //      derived class. If not in sim mode, we own the value object since we
        //      created it.
        //
        //  m_pmeTarget
        //      Our macro engine. We exist effectively to act as a coordinator between
        //      the client and the CML level handler the client has invoked. When run
        //      normally, we create this. In simulator mode, in the IDE, it is provided
        //      to us by the IDE. That's why it's a pointer.
        //
        //  m_pmefrData
        //      We install a standard CQC CML file resolver so that the macros can
        //      access files in MacroFileRoot for storage and such.
        //
        //  m_strPath
        //      This the data server path our event monitor was loaded from. This is
        //      the type relative path, so /Events/Monitors/User/Bubba and whatnot.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean           m_bSimMode;
        tCIDLib::TCard4             m_c4IdleSecs;
        tCIDLib::TCard4             m_c4SerialNum;
        tCQCEvMonEng::TPollList     m_colFieldList;
        TCQCEvMonCfg                m_emoncThis;
        TMEngClassVal*              m_pmecvHandler;
        const TMEngClassInfo*       m_pmeciHandler;
        TCIDMacroEngine*            m_pmeTarget;
        TMEngFixedBaseFileResolver* m_pmefrData;
        TString                     m_strPath;



        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCEvMonitor,TThread)
};

#pragma CIDLIB_POPPACK


