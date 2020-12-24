//
// FILE NAME: CQCMEng_CRCHelpersClass.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/02/2003
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
//  This is the header for the CQCMEng_CRCHelpersClass.cpp, which implements
//  the info and value classes required to create a macro level
//  class. In this case the class provides helper methods for doing CRC
//  calculations. They are common in control protocols, and often tend to
//  be computer intensive, so by providing helper methods to do some of the
//  more common CRCs and some of the common sub-functions of common CRCs,
//  we can provide a large efficiency boost to device drivers.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TCRCHelpersValue
//  PREFIX: mecv
// ---------------------------------------------------------------------------
class CQCMENGEXPORT TCRCHelpersValue : public TMEngClassVal
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCRCHelpersValue
        (
            const   TString&                strName
            , const tCIDLib::TCard2         c2ClassId
            , const tCIDMacroEng::EConstTypes eConst
        );

        TCRCHelpersValue(const TCRCHelpersValue&) = delete;
        TCRCHelpersValue(TCRCHelpersValue&&) = delete;

        ~TCRCHelpersValue();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCRCHelpersValue& operator=(const TCRCHelpersValue&) = delete;
        TCRCHelpersValue& operator=(TCRCHelpersValue&&) = delete;


    private :
        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCRCHelpersValue,TMEngClassVal)
};



// ---------------------------------------------------------------------------
//  CLASS: TCRCHelpersInfo
// PREFIX: meci
// ---------------------------------------------------------------------------
class CQCMENGEXPORT TCRCHelpersInfo : public TMEngClassInfo
{
    public  :
        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static const TString& strClassPath();


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCRCHelpersInfo
        (
                    TCIDMacroEngine&        meOwner
        );

        TCRCHelpersInfo(const TCRCHelpersInfo&) = delete;
        TCRCHelpersInfo(TCRCHelpersInfo&&) = delete;

        ~TCRCHelpersInfo();


        // -------------------------------------------------------------------
        //  Public oeprators
        // -------------------------------------------------------------------
        TCRCHelpersInfo& operator=(const TCRCHelpersInfo&) = delete;
        TCRCHelpersInfo& operator=(TCRCHelpersInfo&&) = delete;


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
        //  Private data members
        //
        //  m_c2MethId_XXX
        //      To speed up dispatch, we store the ids of the methods that
        //      we set up during init.
        //
        //  m_pmeciMemBuf
        //      We have to make a call into the memory buffer class directly
        //      at the C++ level, so we pre-look it up during init, to avoid
        //      the overhead every time we need it.
        // -------------------------------------------------------------------
        tCIDLib::TCard2     m_c2MethId_DefCtor;
        tCIDLib::TCard2     m_c2MethId_CheckSum1;
        tCIDLib::TCard2     m_c2MethId_CheckSum2;
        tCIDLib::TCard2     m_c2MethId_CheckSum4;
        tCIDLib::TCard2     m_c2MethId_CRC1Full;
        tCIDLib::TCard2     m_c2MethId_CRC1InnerLoop;
        tCIDLib::TCard2     m_c2MethId_CRCOcelot;
        tCIDLib::TCard2     m_c2MethId_CRC1PerByte;
        TMEngMemBufInfo*    m_pmeciMemBuf;


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCRCHelpersInfo,TMEngClassInfo)
};

#pragma CIDLIB_POPPACK


