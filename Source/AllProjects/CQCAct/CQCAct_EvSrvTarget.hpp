//
// FILE NAME: CQCAct_EvSrvTarget.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/30/2008
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
//  This is the header for the CQCAct_SrvTarget.cpp file, which implements
//  a standard CQC action target that exposes some event server functionality
//  via the action system.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//  CLASS: TEvSrvCmdTar
// PREFIX: ctar
// ---------------------------------------------------------------------------
class CQCACTEXPORT TEvSrvCmdTar : public TObject, public MCQCCmdTarIntf
{
    public  :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TEvSrvCmdTar();

        TEvSrvCmdTar(const TEvSrvCmdTar&) = delete;
        TEvSrvCmdTar(TEvSrvCmdTar&&) = delete;

        ~TEvSrvCmdTar();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TEvSrvCmdTar& operator=(const TEvSrvCmdTar&) = delete;
        TEvSrvCmdTar& operator=(TEvSrvCmdTar&&) = delete;


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
        tCIDLib::TBoolean bValMask
        (
            const   TString&                strValue
            ,       tCIDLib::TCard4&        c4Mask
            ,       TString&                strErrText
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_strTmpX
        //      A couple temp strings to use internally while manipulating
        //      values.
        // -------------------------------------------------------------------
        TString m_strTmp1;
        TString m_strTmp2;
        TString m_strTmp3;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TEvSrvCmdTar,TObject)
};

#pragma CIDLIB_POPPACK

