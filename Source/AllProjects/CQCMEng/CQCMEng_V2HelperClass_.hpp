//
// FILE NAME: CQCMEng_V2HelperClass_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/01/2014
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
//  This is the header for the CQCMacroEng_V2HelperClass.cpp, which implements
//  the info and value classes required to create a macro level class. In this
//  case the class provides a bunch of helper functionality for V2 drivers. It
//  defines some enums and provides various helper methods.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TV2HelperVal
//  PREFIX: mecv
// ---------------------------------------------------------------------------
class CQCMENGEXPORT TV2HelperVal : public TMEngClassVal
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TV2HelperVal() = delete;

        TV2HelperVal
        (
            const   TString&                strName
            , const tCIDLib::TCard2         c2ClassId
            , const tCIDMacroEng::EConstTypes  eConst
        );

        TV2HelperVal(const TV2HelperVal&) = delete;
        TV2HelperVal(TV2HelperVal&&) = delete;

        ~TV2HelperVal();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TV2HelperVal& operator=(const TV2HelperVal&) = delete;
        TV2HelperVal& operator=(TV2HelperVal&&) = delete;


        // -------------------------------------------------------------------
        //  Public data members
        //
        //  m_colTmpFlds
        //  m_flddTmpDef
        //      Stuff used internally by the info object while processing methods,
        //      to avoid a bunch of overhead.
        // -------------------------------------------------------------------
        tCQCKit::TFldDefList    m_colTmpFlds;
        TCQCFldDef              m_flddTmpDef;


    private :
        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TV2HelperVal,TMEngClassVal)
};



// ---------------------------------------------------------------------------
//  CLASS: TV2HelperInfo
// PREFIX: meci
// ---------------------------------------------------------------------------
class CQCMENGEXPORT TV2HelperInfo : public TMEngClassInfo
{
    public  :
        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static const TString& strDevClassPath();
        static const TString& strClassPath();


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TV2HelperInfo() = delete;

        TV2HelperInfo
        (
                    TCIDMacroEngine&        meOwner
        );

        TV2HelperInfo(const TV2HelperInfo&) = delete;
        TV2HelperInfo(TV2HelperInfo&&) = delete;

        ~TV2HelperInfo();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TV2HelperInfo& operator=(const TV2HelperInfo&) = delete;
        TV2HelperInfo& operator=(TV2HelperInfo&&) = delete;


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
        tCIDLib::TVoid CopyFldsBack
        (
                    TCIDMacroEngine&        meOwner
            , const tCIDLib::TCard4         c4FirstInd
            , const tCQCKit::TFldDefList&   colFlds
            , const tCIDLib::TCard4         c4RetCnt
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c2EnumId_XXX
        //      We store the ids of some nested types that we create, to make
        //      it quicker to deal with them.
        //
        //  m_c2MethId_XXX
        //      To speed up dispatch, we store the ids of the methods that
        //      we set up during init.
        //
        //  m_pmeciXXX
        //      Info objects for some the we create.
        // -------------------------------------------------------------------
        tCIDLib::TCard2     m_c2EnumId_DevClasses;
        tCIDLib::TCard2     m_c2EnumId_PowerStatus;
        tCIDLib::TCard2     m_c2MethId_DefCtor;
        tCIDLib::TCard2     m_c2MethId_GetAIOFlds;
        tCIDLib::TCard2     m_c2MethId_GetAudioFlds;
        tCIDLib::TCard2     m_c2MethId_GetAVProcFlds;
        tCIDLib::TCard2     m_c2MethId_GetDevInfoFlds;
        tCIDLib::TCard2     m_c2MethId_GetDIOFlds;
        tCIDLib::TCard2     m_c2MethId_GetLightFlds;
        tCIDLib::TCard2     m_c2MethId_GetMTransFlds;
        tCIDLib::TCard2     m_c2MethId_GetNowPlayingFlds;
        tCIDLib::TCard2     m_c2MethId_GetPowerFlds;
        tCIDLib::TCard2     m_c2MethId_GetProjFlds;
        tCIDLib::TCard2     m_c2MethId_GetSecAreaFlds;
        tCIDLib::TCard2     m_c2MethId_GetSecZoneFlds;
        tCIDLib::TCard2     m_c2MethId_GetSwitcherFlds;
        tCIDLib::TCard2     m_c2MethId_GetSwitcherFlds2;
        tCIDLib::TCard2     m_c2MethId_GetThermoFlds;
        tCIDLib::TCard2     m_c2MethId_GetTunerFlds;
        tCIDLib::TCard2     m_c2TypeId_FldDef;
        tCIDLib::TCard2     m_c2TypeId_FldList;
        TMEngEnumInfo*      m_pmeciDevClasses;
        TMEngEnumInfo*      m_pmeciPowerStatus;


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TV2HelperInfo,TMEngClassInfo)
};

#pragma CIDLIB_POPPACK



