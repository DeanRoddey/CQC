//
// FILE NAME: CQCMEng_CQCUtilsClass_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/17/2007
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
//  This is the header for the CQCMEng_CQCUtilsClass.cpp, which implements
//  the info class required to create a macro level class. In this case
//  the class is kind of a grab bag class that provides some utility
//  functions that don't fit elsewhere and don't really warrant their own
//  classes. It has no data associated with it, so we don't create a
//  value class. We just use the standard one.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//  CLASS: TCQCUtilsInfo
// PREFIX: meci
// ---------------------------------------------------------------------------
class CQCMENGEXPORT TCQCUtilsInfo : public TMEngClassInfo
{
    public  :
        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static const TString& strClassPath();


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCUtilsInfo() = delete;

        TCQCUtilsInfo
        (
                    TCIDMacroEngine&        meOwner
            , const TCQCSecToken&           sectUser
        );

        TCQCUtilsInfo(const TCQCUtilsInfo&) = delete;
        TCQCUtilsInfo(TCQCUtilsInfo&&) = delete;

        ~TCQCUtilsInfo();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCUtilsInfo& operator=(const TCQCUtilsInfo&) = delete;
        TCQCUtilsInfo& operator=(TCQCUtilsInfo&&) = delete;


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
            , const TError&                 errCaught
        )   const;

        tCIDLib::TVoid ThrowAnErr
        (
                    TCIDMacroEngine&        meOwner
            , const tCIDLib::TCard4         c4ToThrow
        )   const;

        tCIDLib::TVoid ThrowAnErr
        (
                    TCIDMacroEngine&        meOwner
            , const tCIDLib::TCard4         c4ToThrow
            , const MFormattable&           fmtblToken1
        )   const;


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
        //      Pointers to class info objects for our nested classes. We do
        //      not own them.
        //
        //  m_sectUser
        //      The creating code has to give us a security token, since we need it
        //      to do various operations.
        // -------------------------------------------------------------------
        tCIDLib::TCard2 m_c2EnumId_Errors;
        tCIDLib::TCard2 m_c2MethId_CalcGeoDistance;
        tCIDLib::TCard2 m_c2MethId_DefCtor;
        tCIDLib::TCard2 m_c2MethId_GetLocInfo;
        tCIDLib::TCard2 m_c2MethId_SendEMail;
        tCIDLib::TCard2 m_c2MethId_SendHTMLEMail;
        tCIDLib::TCard2 m_c2MethId_SendUserAct;
        tCIDLib::TCard2 m_c2MethId_StartRemApp;
        tCIDLib::TCard4 m_c4ErrACSNotFound;
        tCIDLib::TCard4 m_c4ErrNoLocInfo;
        tCIDLib::TCard4 m_c4ErrOpFailed;
        TMEngEnumInfo*  m_pmeciErrors;
        TCQCSecToken    m_sectUser;


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCUtilsInfo,TMEngClassInfo)
};

#pragma CIDLIB_POPPACK




