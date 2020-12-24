//
// FILE NAME: CQCKit_CMLBinBaseClass_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/10/2005
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
//  This is the header for the CQCKit_CMLBinBaseClass.cpp file, which implements a
//  CML abstract base class that all CMLBin classes must derive from. We define the
//  abstract interface the derived class must implement and we provide some helper
//  methods at this level to get more information on the HTTP operation that was
//  invoked.
//
//  Since the CMLBin handler class (called by the WS engine when a CMLBin URL is
//  received) needs to get access to some of the data members of the derived class
//  objects, we don't store the instance data in our own value class (at the C++
//  level.) Instead we use the standard CML class value class, and add CML level members
//  to it, so that the derived classes will inherit them at the CML level, and
//  therefore the CMLBin handler can access them via our base CML class defined here.
//  So we just provide the info class, not the value class.
//
//  The issue is that it's the derived class that will get created, not one of ours.
//  So, if we created a C++ class, that's not what we would be creating. We'd be creating
//  the derived, CML level, class. So any C++ members we might define here in a C++
//  value class would not be available.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//  CLASS: TCMLBinBaseInfo
// PREFIX: meci
// ---------------------------------------------------------------------------
class TCMLBinBaseInfo : public TMEngClassInfo
{
    public  :
        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static const TString strContBoundaryName;

        static const TString strContBufName;

        static const TString strContBufLenName;

        static const TString strContEncodingName;

        static const TString strContTypeName;

        static const TString strProcessReqMeth;

        static const TString& strPath();

        static const TString strValListName;


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCMLBinBaseInfo
        (
                    TCIDMacroEngine&        meOwner
        );

        TCMLBinBaseInfo(const TCMLBinBaseInfo&) = delete;
        TCMLBinBaseInfo(TCMLBinBaseInfo&&) = delete;

        ~TCMLBinBaseInfo();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCMLBinBaseInfo& operator=(const TCMLBinBaseInfo&) = delete;
        TCMLBinBaseInfo& operator=(TCMLBinBaseInfo&&) = delete;


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
        tCIDLib::TVoid ThrowAnErr
        (
                    TCIDMacroEngine&        meOwner
            , const tCIDLib::TCard4         c4ToThrow
            , const TString&                strText
        )   const;


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c2MethId_XXX
        //      To speed up dispatch, we store the ids of the methods that
        //      we set up during init.
        //
        //  m_pmeciXXX
        //      These are all types that we create members of or have parameters of.
        //      So we look up the class info objects. We need a vector of KVPairs. And
        //      we need to store any incoming body content and the number of bytes, and
        //      the encoding.
        // -------------------------------------------------------------------
        tCIDLib::TCard2     m_c2EnumId_Errors;
        tCIDLib::TCard2     m_c2MethId_AddReqValue;
        tCIDLib::TCard2     m_c2MethId_BuildErrReply;
        tCIDLib::TCard2     m_c2MethId_DefCtor;
        tCIDLib::TCard2     m_c2MethId_EscapeText;
        tCIDLib::TCard2     m_c2MethId_FindReqValue;
        tCIDLib::TCard2     m_c2MethId_ProcessReq;
        tCIDLib::TCard2     m_c2MethId_QueryBodyContent;
        tCIDLib::TCard2     m_c2MethId_SetInBodyCont;
        tCIDLib::TCard4     m_c4ErrBldErrReply;
        tCIDLib::TCard4     m_c4ErrEscape;
        tCIDLib::TCard4     m_c4ErrReqVal;
        TMEngEnumInfo*      m_pmeciErrors;
        TMEngVectorInfo*    m_pmeciList;
        TMEngKVPairInfo*    m_pmeciKVPair;


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCMLBinBaseInfo,TMEngClassInfo)
};

#pragma CIDLIB_POPPACK

