//
// FILE NAME: CQCGKit_StdCmdTargets.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/16/2005
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
//  This is the header for the CQCAct_StdCmdTargets.cpp file, which implements
//  some standard command targets that are always available any time actions are
//  invoked. Specific facilities/applications may provide others, but these are
//  the core ones always there.
//
//  Note that the local/global variables command target is actually provided by
//  CQCKit because there are other things down there that use it. We provide targets
//  for macros, driver/field access, and event triggers.
//
//  The System target is a separate file because it's quite alrge.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

class TCIDMacroEngine;
class TCQCMEngErrHandler;
class TMEngFixedBaseFileResolver;


// ---------------------------------------------------------------------------
//  CLASS: TStdFieldCmdTar
// PREFIX: ctar
// ---------------------------------------------------------------------------
class CQCACTEXPORT TStdFieldCmdTar : public TObject, public MCQCCmdTarIntf
{
    public  :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TStdFieldCmdTar();

        TStdFieldCmdTar(const TStdFieldCmdTar&) = delete;
        TStdFieldCmdTar(TStdFieldCmdTar&&) = delete;

        ~TStdFieldCmdTar();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TStdFieldCmdTar& operator=(const TStdFieldCmdTar&) = delete;
        TStdFieldCmdTar& operator=(TStdFieldCmdTar&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bValidateParm
        (
            const   TCQCCmd&                cmdSrc
            , const TCQCCmdCfg&             ccfgTar
            , const tCIDLib::TCard4         c4Index
            , const TString&                strValue
            ,       TString&                strErrText
        )   final;

        tCIDLib::TVoid CmdTarCleanup() final;

        tCIDLib::TVoid CmdTarInitialize
        (
            const   TCQCActEngine&          acteTar
        )   final;

        tCQCKit::ECmdRes eDoCmd
        (
            const   TCQCCmdCfg&             ccfgToDo
            , const tCIDLib::TCard4         c4Index
            , const TString&                strUserId
            , const TString&                strEventId
            ,       TStdVarsTar&            ctarGlobalVars
            ,       tCIDLib::TBoolean&      bResult
            ,       TCQCActEngine&          acteTar
        )   final;

        tCIDLib::TVoid QueryCmds
        (
                    TCollection<TCQCCmd>&   colToFill
            , const tCQCKit::EActCmdCtx     eContext
        )   const final;

        tCIDLib::TVoid SetDefParms
        (
                    TCQCCmdCfg&             ccfgToSet
        )   const final;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bFindFields
        (
            const   TString&                strMon
            ,       TString&                strFldNames
            ,       TString&                strNames
            , const TString&                strRegEx
            , const tCQCKit::EReqAccess     eReqAccess
        );

        tCIDLib::TBoolean bParseTimedFldChInt
        (
            const   TString&                strToParse
            ,       TString&                strErr
            ,       tCIDLib::TCard4&        c4SecsToFill
        )   const;

        tCIDLib::TBoolean bQuerySemFields
        (
            const   TString&                strMon
            , const tCQCKit::EFldSTypes     eSemType
            ,       TString&                strFldNames
            ,       TString&                strNames
            , const tCQCKit::EReqAccess     eReqAccess
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_strTmpX
        //      A couple temp strings to use internally while manipulating
        //      values.
        // -------------------------------------------------------------------
        TString             m_strTmp1;
        TString             m_strTmp2;
        TString             m_strTmp3;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TStdFieldCmdTar,TObject)
};




// ---------------------------------------------------------------------------
//  CLASS: TStdMacroCmdTar
// PREFIX: ctar
// ---------------------------------------------------------------------------
class CQCACTEXPORT TStdMacroCmdTar : public TObject, public MCQCCmdTarIntf
{
    public  :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TStdMacroCmdTar();

        TStdMacroCmdTar(const TStdMacroCmdTar&) = delete;
        TStdMacroCmdTar(TStdMacroCmdTar&&) = delete;

        ~TStdMacroCmdTar();


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        TStdMacroCmdTar& operator=(const TStdMacroCmdTar&) = delete;
        TStdMacroCmdTar& operator=(TStdMacroCmdTar&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bValidateParm
        (
            const   TCQCCmd&                cmdSrc
            , const TCQCCmdCfg&             ccfgTar
            , const tCIDLib::TCard4         c4Index
            , const TString&                strValue
            ,       TString&                strErrText
        )   final;

        tCIDLib::TVoid CmdTarCleanup() override;

        tCIDLib::TVoid CmdTarInitialize
        (
            const   TCQCActEngine&          acteTar
        )   final;

        tCQCKit::ECmdRes eDoCmd
        (
            const   TCQCCmdCfg&             ccfgToDo
            , const tCIDLib::TCard4         c4Index
            , const TString&                strUserId
            , const TString&                strEventId
            ,       TStdVarsTar&            ctarGlobalVars
            ,       tCIDLib::TBoolean&      bResult
            ,       TCQCActEngine&          acteTar
        )   final;

        tCIDLib::TVoid QueryCmds
        (
                    TCollection<TCQCCmd>&   colToFill
            , const tCQCKit::EActCmdCtx     eContext
        )   const final;

        tCIDLib::TVoid SetDefParms
        (
                    TCQCCmdCfg&             ccfgToSet
        )   const final;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        const TCIDMacroEngine* pmeTarget() const;

        TCIDMacroEngine* pmeTarget();

        tCIDLib::TVoid SetEngine
        (
                    TCIDMacroEngine* const  pmeToUse
        );


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid SetupEngine();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_pmeehLogger
        //  m_pmefrData
        //      The standard CQC extensions of the logger handler and file
        //      resolver classes of the macro engine. We'll install these on
        //      the macro engine.
        //
        //      These are pointers so that we don't have to force the macro
        //      engine headers on all our clients (everybody.)
        //
        //  m_pmeToUse
        //      The macro engine we'll use. We make it a member so that if
        //      the action has more than one macro invocation we can just
        //      keep reusing this guy.
        //
        //      This is a pointer so that we don't have to force the macro
        //      engine headers on all our clients (everybody), and so that
        //      we can be given an engine to use for specialized scenarios.
        // -------------------------------------------------------------------
        TCQCMEngErrHandler*         m_pmeehLogger;
        TMEngFixedBaseFileResolver* m_pmefrData;
        TCIDMacroEngine*            m_pmeToUse;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TStdMacroCmdTar,TObject)
};



// ---------------------------------------------------------------------------
//  CLASS: TTrigEvCmdTarget
// PREFIX: ctar
// ---------------------------------------------------------------------------
class CQCACTEXPORT TTrigEvCmdTarget : public TObject, public MCQCCmdTarIntf
{
    public  :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TTrigEvCmdTarget();

        TTrigEvCmdTarget(const TTrigEvCmdTarget&) = delete;
        TTrigEvCmdTarget(TTrigEvCmdTarget&&) = delete;

        ~TTrigEvCmdTarget();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TTrigEvCmdTarget& operator=(const TTrigEvCmdTarget&) = delete;
        TTrigEvCmdTarget& operator=(TTrigEvCmdTarget&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bValidateParm
        (
            const   TCQCCmd&                cmdSrc
            , const TCQCCmdCfg&             ccfgTar
            , const tCIDLib::TCard4         c4Index
            , const TString&                strValue
            ,       TString&                strErrText
        )   final;

        tCIDLib::TVoid CmdTarCleanup() final;

        tCIDLib::TVoid CmdTarInitialize
        (
            const   TCQCActEngine&          acteTar
        )   final;

        tCQCKit::ECmdRes eDoCmd
        (
            const   TCQCCmdCfg&             ccfgToDo
            , const tCIDLib::TCard4         c4Index
            , const TString&                strUserId
            , const TString&                strEventId
            ,       TStdVarsTar&            ctarGlobalVars
            ,       tCIDLib::TBoolean&      bResult
            ,       TCQCActEngine&          acteTar
        )   final;

        tCIDLib::TVoid QueryCmds
        (
                    TCollection<TCQCCmd>&   colToFill
            , const tCQCKit::EActCmdCtx     eContext
        )   const final;

        tCIDLib::TVoid SetDefParms
        (
                    TCQCCmdCfg&             ccfgToSet
        )   const final;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        const TCQCEvent& evData() const;

        tCIDLib::TVoid SetEvent
        (
            const   tCQCKit::TCQCEvPtr&     cptrEvent
        );


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_cptrEvent
        //      This is the received event that we operate on. It's a counted
        //      pointer to the event, which is needed by a number of 'players'
        //      during the processing of events and we don't want to replicate
        //      it over and over.
        // -------------------------------------------------------------------
        tCQCKit::TCQCEvPtr  m_cptrEvent;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TTrigEvCmdTarget,TObject)
};


#pragma CIDLIB_POPPACK


