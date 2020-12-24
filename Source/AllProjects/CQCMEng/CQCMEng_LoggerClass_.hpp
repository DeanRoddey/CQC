//
// FILE NAME: CQCMEng_LoggerClass_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/01/2003
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
//  This is the header for the CQCMEng_LoggerClass.cpp, which implements
//  the info and value classes required to create a macro level class. In this
//  case the class allows CQC macros to log to the centralized log server.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TCQCLoggerVal
//  PREFIX: mecv
// ---------------------------------------------------------------------------
class CQCMENGEXPORT TCQCLoggerVal : public TMEngClassVal
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCLoggerVal() = delete;

        TCQCLoggerVal
        (
            const   TString&                strName
            , const tCIDLib::TCard2         c2ClassId
            , const tCIDMacroEng::EConstTypes  eConst
        );

        TCQCLoggerVal(const TCQCLoggerVal&) = delete;
        TCQCLoggerVal(TCQCLoggerVal&&) = delete;

        ~TCQCLoggerVal();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCLoggerVal& operator=(const TCQCLoggerVal&) = delete;
        TCQCLoggerVal& operator=(TCQCLoggerVal&&) = delete;


    private :
        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCLoggerVal,TMEngClassVal)
};



// ---------------------------------------------------------------------------
//  CLASS: TCQCLoggerInfo
// PREFIX: meci
// ---------------------------------------------------------------------------
class CQCMENGEXPORT TCQCLoggerInfo : public TMEngClassInfo
{
    public  :
        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static const TString& strClassPath();


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCLoggerInfo() = delete;

        TCQCLoggerInfo
        (
                    TCIDMacroEngine&        meOwner
        );

        TCQCLoggerInfo(const TCQCLoggerInfo&) = delete;
        TCQCLoggerInfo(TCQCLoggerInfo&&) = delete;

        ~TCQCLoggerInfo();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCLoggerInfo& operator=(const TCQCLoggerInfo&) = delete;
        TCQCLoggerInfo& operator=(TCQCLoggerInfo&&) = delete;


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
        //  Private, non-virtual mehods
        // -------------------------------------------------------------------
        tCIDLib::TVoid ReplaceTokens
        (
                    TCIDMacroEngine&        meOwner
            ,       TString&                strMsg
            , const tCIDLib::TCard2         c2MethId
            , const tCIDLib::TCard4         c4FirstInd
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c2MethId_XXX
        //      To speed up dispatch, we store the ids of the methods that
        //      we set up during init.
        //
        //  m_pmeciExcept
        //      We reference the exception class in our interface
        //
        //  m_pmecvStrm
        //      A macro level output text stream we can use to format the
        //      replacement tokens.
        //
        //  m_strMsg
        //      A temp string to use for formatting the output message.
        // -------------------------------------------------------------------
        tCIDLib::TCard2         m_c2MethId_DefCtor;
        tCIDLib::TCard2         m_c2MethId_LogExcept;
        tCIDLib::TCard2         m_c2MethId_LogExceptP;
        tCIDLib::TCard2         m_c2MethId_LogMsg;
        tCIDLib::TCard2         m_c2MethId_LogMsgP;
        tCIDLib::TCard2         m_c2MethId_LogMsg1;
        tCIDLib::TCard2         m_c2MethId_LogMsg1P;
        tCIDLib::TCard2         m_c2MethId_LogMsg2;
        tCIDLib::TCard2         m_c2MethId_LogMsg2P;
        tCIDLib::TCard2         m_c2MethId_LogMsg3;
        tCIDLib::TCard2         m_c2MethId_LogMsg3P;
        TMEngExceptInfo*        m_pmeciExcept;
        TMEngTextOutStreamVal*  m_pmecvStrm;
        TString                 m_strMsg;


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCLoggerInfo,TMEngClassInfo)
};

#pragma CIDLIB_POPPACK



