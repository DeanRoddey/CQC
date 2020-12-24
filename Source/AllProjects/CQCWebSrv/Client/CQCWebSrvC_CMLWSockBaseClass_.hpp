//
// FILE NAME: CQCKit_CMLWSockBaseClass_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/17/2014
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
//  This is the header for the CQCKit_CMLWSockBaseClass.cpp file, which implements a
//  CML abstract base class that all websocket handlers derive from. The CQC web server
//  uses this abstract interface to interact with websocket handlers on behalf of
//  the websocket clients.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//  CLASS: TCMLWSockBaseInfo
// PREFIX: meci
// ---------------------------------------------------------------------------
class CQCWEBSRVCEXP TCMLWSockBaseInfo : public TMEngClassInfo
{
    public  :
        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static const TString& strPath();


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCMLWSockBaseInfo
        (
                    TCIDMacroEngine&        meOwner
        );

        TCMLWSockBaseInfo(const TCMLWSockBaseInfo&) = delete;
        TCMLWSockBaseInfo(TCMLWSockBaseInfo&&) = delete;

        ~TCMLWSockBaseInfo();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCMLWSockBaseInfo& operator=(const TCMLWSockBaseInfo&) = delete;
        TCMLWSockBaseInfo& operator=(TCMLWSockBaseInfo&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid Init
        (
                    TCIDMacroEngine&        meOwner
        )   final;

        TMEngClassVal* pmecvMakeStorage
        (
            const   TString&                strName
            ,       TCIDMacroEngine&        meOwner
            , const tCIDMacroEng::EConstTypes  eConst
        )   const final;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bInvokeMethod
        (
                    TCIDMacroEngine&        meOwner
            , const TMEngMethodInfo&        methiTarget
            ,       TMEngClassVal&          mecvInstance
        )   final;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid ProcessSimReq
        (
                    TCIDDataSrcJan&         janSrc
            ,       TCIDMacroEngine&        meOwner
            ,       TMEngClassVal&          mecvInstance
            , const TString&                strURL
            , const tCIDLib::TKVPList&      colHdrLines
            , const TString&                strType
            , const TString&                strClassPath
        );

        tCIDLib::TVoid ReplaceTokens
        (
                    TCIDMacroEngine&        meOwner
            ,       TString&                strMsg
            , const tCIDLib::TCard4         c4Tokens
            , const tCIDLib::TCard4         c4FirstInd
        );

        tCIDLib::TVoid Simulate
        (
                    TCIDMacroEngine&        meOwner
            ,       TMEngClassVal&          mecvInstance
            , const tCIDLib::TIPPortNum     ippnPort
            , const TString&                strClassPath
            , const TString&                strCertInfo
        );

        tCIDLib::TVoid ThrowAnErr
        (
                    TCIDMacroEngine&        meOwner
            , const tCIDLib::TCard4         c4ToThrow
            , const TString&                strText
        )   const;

        tCIDLib::TVoid ThrowAnErr
        (
                    TCIDMacroEngine&        meOwner
            , const tCIDLib::TCard4         c4ToThrow
            , const TError&                 errCaught
        )   const;


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c2EnumId_XXX
        //      Class ids for some nested enumerated types
        //
        //  m_c2MethId_XXX
        //      To speed up dispatch, we store the ids of the methods that we set up
        //      during init.
        //
        //  m_pmeciXXX
        //      Info objects for any nested or external classes we want to interact
        //      with directly.
        // -------------------------------------------------------------------
        tCIDLib::TCard2         m_c2EnumId_Errors;
        tCIDLib::TCard2         m_c2MethId_Connected;
        tCIDLib::TCard2         m_c2MethId_DefCtor;
        tCIDLib::TCard2         m_c2MethId_Disconnected;
        tCIDLib::TCard2         m_c2MethId_EnableMsgLog;
        tCIDLib::TCard2         m_c2MethId_FieldChanged;
        tCIDLib::TCard2         m_c2MethId_LogMsg;
        tCIDLib::TCard2         m_c2MethId_LogMsg1;
        tCIDLib::TCard2         m_c2MethId_LogMsg2;
        tCIDLib::TCard2         m_c2MethId_ProcessMsg;
        tCIDLib::TCard2         m_c2MethId_SendMsg;
        tCIDLib::TCard2         m_c2MethId_SetFieldList;
        tCIDLib::TCard2         m_c2MethId_Simulate;
        tCIDLib::TCard2         m_c2MethId_StartShutdown;
        tCIDLib::TCard2         m_c2MethId_ValidateCreds;
        tCIDLib::TCard2         m_c2MethId_WriteField;
        tCIDLib::TCard2         m_c2MethId_WriteField2;
        tCIDLib::TCard4         m_c4ErrId_FieldWrite;
        tCIDLib::TCard4         m_c4ErrId_Simulate;
        TMEngEnumInfo*          m_pmeciErrors;
        TMEngVectorInfo*        m_pmeciStrList;



        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCMLWSockBaseInfo,TMEngClassInfo)
};

#pragma CIDLIB_POPPACK


