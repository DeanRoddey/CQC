//
// FILE NAME: CQCGKit_SystemTarget.hpp
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
//  This is the header for the CQCAct_SystemTarget.cpp file, which implements
//  the System action target.
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
//  CLASS: TStdSystemCmdTar
// PREFIX: ctar
// ---------------------------------------------------------------------------
class CQCACTEXPORT TStdSystemCmdTar : public TObject, public MCQCCmdTarIntf
{
    public  :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TStdSystemCmdTar();

        TStdSystemCmdTar(const TStdSystemCmdTar&) = delete;
        TStdSystemCmdTar(TStdSystemCmdTar&&) = delete;

        ~TStdSystemCmdTar();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TStdSystemCmdTar& operator=(const TStdSystemCmdTar&) = delete;
        TStdSystemCmdTar& operator=(TStdSystemCmdTar&&) = delete;


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
        tCIDLib::TBoolean bIsInList
        (
            const  TCQCCmdCfg&              ccfgSrc
        );

        tCIDLib::TCard4 c4GetImage
        (
            const   TURL&                   urlImg
            ,       TMemBuf&                mbufImg
            ,       TString&                strImgType
        );

        tCIDLib::TVoid FaultInTTS();

        tCIDLib::TVoid FmtNumForSpeech
        (
            const   tCIDLib::TCard4         c4Val
            ,       TString&                strToFill
        );

        tCIDLib::TVoid GetLatLongInfo
        (
            const   tCIDLib::TBoolean       bServer
            ,       tCIDLib::TFloat8&       f8Lat
            ,       tCIDLib::TFloat8&       f8Long
            , const TCQCSecToken&           sectUser
        );

        tCIDLib::TVoid InitTimers();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4ReqNum
        //      If we have to do any digest authentication in URL queries,
        //      we use this as the request number, do we can bump it each time.
        //
        //  m_enctNextLLCheck
        //      We will reaquire the lat/long info every thirty minutes, just
        //      in case it should change. So we set this to thirty minutes
        //      form the time we last checked. If now is beyond this, it's
        //      time to get it again.
        //
        //  m_f8Lat
        //  m_f8Long
        //      The lat/long info we last got from the MS. If these are zero
        //      them we've not gotten it yet.
        //
        //  m_pspchTTS
        //      A speech object to use for any TTS work. We have to create it
        //      for each new action invocation, because of threading sillinness
        //      wrt to the underlying COM stuff. So we create in Initialize()
        //      and delete it (if faulted in) in the CmdTarCleanup() method.
        //
        //  m_strTmp
        //      A temp string for formatting.
        // -------------------------------------------------------------------
        tCIDLib::TCard4         m_c4ReqNum;
        tCIDLib::TEncodedTime   m_enctNextLLCheck;
        tCIDLib::TFloat8        m_f8Lat;
        tCIDLib::TFloat8        m_f8Long;
        tCIDLib::TEncodedTime   m_aenctTimers[kCQCAct::c4MaxSysTarTimers];
        TSpeech*                m_pspchTTS;
        TString                 m_strTmp;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TStdSystemCmdTar,TObject)
};

#pragma CIDLIB_POPPACK

