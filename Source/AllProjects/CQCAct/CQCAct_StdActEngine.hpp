//
// FILE NAME: CQCAct_StdActEngine.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/01/2008
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
//  This class just provides a derivative of the standard action engine virtual
//  interface (from CQCKit.) This derivative is one that is used in most places
//  to run actions. It either runs the action in the context of the calling thread
//  or it can run it from a separate background thread. The latter scenario is
//  used when running actions in GUI applications. This allows the foreground
//  to continue to pump msgs (generally in a modal loop to lock things up until
//  the action completes.)
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: TCQCStdActEngine
//  PREFIX: acte
// ---------------------------------------------------------------------------
class CQCACTEXPORT TCQCStdActEngine : public TCQCActEngine
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCStdActEngine() = delete;

        TCQCStdActEngine
        (
            const   TCQCUserCtx&            cuctxToUse
        );

        TCQCStdActEngine(const TCQCStdActEngine&) = delete;
        TCQCStdActEngine(TCQCStdActEngine&&) = delete;

        ~TCQCStdActEngine();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCStdActEngine& operator=(const TCQCStdActEngine&) = delete;
        TCQCStdActEngine& operator=(TCQCStdActEngine&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCQCKit::TCmdTarList& colExtra() final;

        const TStdVarsTar& ctarLocals() const final;

        TStdVarsTar& ctarLocals() final;

        MCQCCmdTracer* const pcmdtDebug() final;

        tCIDLib::TVoid Reset() final;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCQCKit::ECmdRes eInvokeOps
        (
            const   MCQCCmdSrcIntf&         mcsrcSend
            , const TString&                strEventId
            ,       TStdVarsTar&            ctarGlobals
            ,       tCQCKit::TCmdTarList&   colExtraTars
            ,       MCQCCmdTracer* const    pcmdtDebug
            , const TString&                strParms
        );

        tCQCKit::ECmdRes eInvokeOpsInBgn
        (
            const   MCQCCmdSrcIntf&         mcsrcSend
            , const TString&                strEventId
            ,       TStdVarsTar&            ctarGlobals
            ,       tCQCKit::TCmdTarList&   colExtraTars
            ,       MCQCCmdTracer* const    pcmdtDebug
            , const TString&                strParms
        );

        const TEvSrvCmdTar& ctarEventSrv() const;

        TEvSrvCmdTar& ctarEventSrv();

        const TStdFieldCmdTar& ctarFlds() const;

        TStdFieldCmdTar& ctarFlds();

        const TStdMacroCmdTar& ctarMacro() const;

        TStdMacroCmdTar& ctarMacro();

        const TStdSystemCmdTar& ctarSystem() const;

        TStdSystemCmdTar& ctarSystem();


    protected :
        // -------------------------------------------------------------------
        //  Protected, virtual methods
        // -------------------------------------------------------------------
        virtual tCIDLib::TBoolean bDoCmdPrep
        (
                    TCQCCmdCfg&             ccfgSrc
            , const TCQCCmdRTVSrc&          crtsVals
            , const TStdVarsTar&            ctarGVars
            , const TStdVarsTar&            ctarLVars
            ,       TString&                strTmp
        );

        virtual tCIDLib::TVoid CmdInvokeErr
        (
            const   TError&                 errToCatch
            , const tCIDLib::TCard4         c4Index
        );

        virtual tCIDLib::TVoid WaitEnd
        (
                    tCIDLib::TBoolean&      bDone
        );

        virtual tCIDLib::TVoid PreLoop
        (
            const   MCQCCmdSrcIntf&         mcsrcSend
            ,       tCQCKit::TCmdTarList&   colTargets
            , const TString&                strEventId
        );


    private :
        // -------------------------------------------------------------------
        //  When invoked for background operations, this is used to get the
        //  incoming data to the processing thread.
        // -------------------------------------------------------------------
        struct TActInfo
        {
            const MCQCCmdSrcIntf*   pmcsrcSender;
            const TString*          pstrEventId;
            TStdVarsTar*            pctarGlobals;
            tCQCKit::TCmdTarList*   pcolExtraTars;
            MCQCCmdTracer*          pcmdtDebug;
            tCQCKit::ECmdRes        eRes;
            TError                  errCaught;
            tCIDLib::TCard4         c4ErrIndex;
            tCIDLib::TBoolean       bDoneFlag;
            const TString*          pstrParms;
        };


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::EExitCodes eBgnProc
        (
                    TThread&                thrThis
            ,       tCIDLib::TVoid*         pData
        );

        tCQCKit::ECmdRes eInvoke
        (
            const   MCQCCmdSrcIntf&         mcsrcSend
            , const TString&                strEventId
            ,       TStdVarsTar&            ctarGlobals
            ,       tCQCKit::TCmdTarList&   colExtraTars
            ,       MCQCCmdTracer* const    pcmdtDebug
            ,       TError&                 errCaught
            ,       tCIDLib::TCard4&        c4ErrIndex
            , const TString&                strParms
        );

        MCQCCmdTarIntf* pctarFind
        (
            const   tCIDLib::TCard4         c4TarId
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_colExtra
        //      For technical reasons we keep a list of the extra targets.
        //      If anyone wants to invoke another engine in a nested way
        //      then we need to be able to pass on the extra targets. They
        //      are not adopted, so that's not a problem.
        //
        //  m_colOps
        //      We have to steal a copy of the caller's opcodes for the
        //      event we are going to invoke, so that we can modify them
        //      as we process them.
        //
        //  m_colTargets
        //      This is the full list of targets that we set up for quick
        //      searching by id when we need to find a target to dispatch
        //      to. We don't adopt them.
        //
        //  m_ctarXXX
        //      The command targets that we always provide. The caller can
        //      pass others for more specialized needs.  Note that the locals
        //      doesn't have to be thread safe since we are only accessed by
        //      by one thread at a time, but if the caller provides a globals
        //      target, it should be.
        //
        //  m_pcmdtDebug
        //      They can pass us a tracer object for debugging purposes, and
        //      we'll call back to them with tracing info.
        // -------------------------------------------------------------------
        tCQCKit::TCmdTarList            m_colExtra;
        MCQCCmdSrcIntf::TOpcodeBlock    m_colOps;
        tCQCKit::TCmdTarList            m_colTargets;
        TEvSrvCmdTar                    m_ctarEventSrv;
        TStdFieldCmdTar                 m_ctarFlds;
        TStdVarsTar                     m_ctarLocals;
        TStdMacroCmdTar                 m_ctarMacro;
        TStdSystemCmdTar                m_ctarSystem;
        MCQCCmdTracer*                  m_pcmdtDebug;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCStdActEngine,TCQCActEngine)
};

#pragma CIDLIB_POPPACK


