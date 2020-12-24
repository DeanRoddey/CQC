//
// FILE NAME: CQCGKit_VarsTarget.hpp
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
//  This is the header for the CQCKit_VarsTarget.cpp file, which implements a
//  standard CQC action command target. Most of them are in the CQCAct facility,
//  but this one is needed down here, because there are some other things that
//  can manipulate action variables down here.
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
//  CLASS: TStdVarsTar
// PREFIX: ctar
// ---------------------------------------------------------------------------
class CQCKITEXPORT TStdVarsTar : public TObject, public MCQCCmdTarIntf
{
    public  :
        // -------------------------------------------------------------------
        //  Class types
        // -------------------------------------------------------------------
        typedef tCQCKit::TVarList::TCursor  TCursor;


        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static tCIDLib::TBoolean bValidVarName
        (
            const   TString&                strToCheck
            ,       TString&                strVarName
            , const TString&                strPrefix
            ,       TString&                strReason
            ,       tCIDLib::TBoolean&      bByValue
            , const tCIDLib::TBoolean       bDerefOK = kCIDLib::True
        );

        static tCIDLib::TBoolean bValidVarName
        (
            const   TString&                strToCheck
            ,       TString&                strVarName
            ,       TString&                strReason
            ,       tCIDLib::TBoolean&      bByValue
            , const tCIDLib::TBoolean       bDerefOK = kCIDLib::True
        );

        static tCIDLib::TBoolean bValidVarName
        (
            const   TString&                strToCheck
            , const TString&                strPrefix
            ,       TString&                strReason
        );

        static tCIDLib::TBoolean bVarExists
        (
            const   TString&                strToFind
            ,       TStdVarsTar&            ctarLocals
            ,       TStdVarsTar&            ctarGlobals
        );

        static TCQCActVar& varFind
        (
            const   TString&                strToFind
            ,       TStdVarsTar&            ctarLocals
            ,       TStdVarsTar&            ctarGlobals
            , const tCIDLib::TCh* const     pszFile
            , const tCIDLib::TCard4         c4Line
            , const tCIDLib::TBoolean       bCreateIfNot
        );

        static const TCQCActVar& varFind
        (
            const   TString&                strToFind
            , const TStdVarsTar&            ctarLocals
            , const TStdVarsTar&            ctarGlobals
            , const tCIDLib::TCh* const     pszFile
            , const tCIDLib::TCard4         c4Line
        );

        TCQCActVar& varTestAndSet
        (
            const   TString&                strToFind
            ,       TStdVarsTar&            ctarGlobals
            , const tCIDLib::TCh* const     pszFile
            , const tCIDLib::TCard4         c4Line
            ,       tCIDLib::TBoolean&      bResult
        );


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TStdVarsTar() = delete;

        TStdVarsTar
        (
            const   tCIDLib::EMTStates      eMTSafe
            , const tCIDLib::TBoolean       bLocal
        );

        TStdVarsTar(const TStdVarsTar&) = delete;

        ~TStdVarsTar();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TStdVarsTar& operator=(const TStdVarsTar&) = delete;


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
        );

        tCIDLib::TVoid CmdTarCleanup();

        tCIDLib::TVoid CmdTarInitialize
        (
            const   TCQCActEngine&          acteToUse
        );

        tCQCKit::ECmdRes eDoCmd
        (
            const   TCQCCmdCfg&             ccfgToDo
            , const tCIDLib::TCard4         c4Index
            , const TString&                strUserId
            , const TString&                strEventId
            ,       TStdVarsTar&            ctarGlobalVars
            ,       tCIDLib::TBoolean&      bResult
            ,       TCQCActEngine&          acteTar
        );

        tCIDLib::TVoid QueryCmds
        (
                    TCollection<TCQCCmd>&   colToFill
            , const tCQCKit::EActCmdCtx     eContext
        )   const;

        tCIDLib::TVoid SetDefParms
        (
                    TCQCCmdCfg&             ccfgToSet
        )   const;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid AddActParmVar
        (
            const   TString&                strName
            , const TString&                strValue
        );

        tCIDLib::TBoolean bAddOrUpdateVar
        (
            const   TString&                strName
            , const TString&                strValue
            , const tCQCKit::EFldTypes      eType
            , const TString&                strLimits
        );

        tCIDLib::TBoolean bVarValue
        (
            const   TString&                strName
            ,       TString&                strToFill
        )   const;

        TCursor cursVars() const;

        tCQCKit::EValQRes eVarValue
        (
            const   TString&                strKey
            ,       TString&                strToFill
            ,       tCIDLib::TCard4&        c4SerialNum
            ,       tCIDLib::TCard4&        c4VarId
        )   const;

        tCQCKit::EValQRes eVarValue
        (
            const   TString&                strKey
            ,       TString&                strToFill
            ,       tCIDLib::TCard4&        c4SerialNum
        )   const;

        tCIDLib::TVoid DeleteAllVars();

        const TString& strValue
        (
            const   TString&                strKey
        )   const;

        tCIDLib::TVoid UpdateVar
        (
            const   TString&                strKey
            , const TString&                strToSet
        );

        TCQCActVar& varFindByName
        (
            const   TString&                strName
        );

        const TCQCActVar& varFindByName
        (
            const   TString&                strName
        )   const;


    private :
        // -------------------------------------------------------------------
        //  Declare our friends
        // -------------------------------------------------------------------
        friend class TStdVarTarLockJan;


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid CheckVarType
        (
            const   TCQCActVar&             varTar
            , const tCQCKit::EFldTypes      eExtType
        );

        tCIDLib::TVoid SetVar
        (
            const   TString&                strKey
            , const TString&                strValue
            ,       MCQCCmdTracer* const    pcmdtDebug
            ,       tCIDLib::TBoolean&      bAdded
        );

        TCQCActVar& varFindIt
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4Line
        );


        // -------------------------------------------------------------------
        //  Private, static methods
        // -------------------------------------------------------------------
        static tCIDLib::TVoid ThrowNotFound
        (
            const   TString&                strName
            , const tCIDLib::TCh* const     pszFile
            , const tCIDLib::TCard4         c4Line
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bIsLocal
        //      We store the incoming ctor parameter that tells us if we are
        //      local vars or global vars, so that we can quickly do one thing
        //      or the other where necessary.
        //
        //  m_c4NextId
        //      We assign each new variable an id from this running sequential
        //      counter.
        //
        //  m_colVars
        //      Storage for the variable objects that define our vars. If this
        //      object instance needs to be thread safe, then we make this
        //      collection threadsafe, and use it to lock during command processing.
        //
        //  m_strTmp
        //      We do a lot of string swizzling, so to avoid creating tmp
        //      strings, we keep one around for temp use.
        //
        //  m_strPrefix
        //      The variable name prefix for our variables. We store it during
        //      ctor to avoid having to do a conditional every time we need
        //      to check for our prefix.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bIsLocal;
        tCIDLib::TCard4         m_c4NextId;
        tCQCKit::TVarList       m_colVars;
        TString                 m_strTmp;
        TString                 m_strTmp2;
        TString                 m_strPrefix;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TStdVarsTar,TObject)
};


#pragma CIDLIB_POPPACK


