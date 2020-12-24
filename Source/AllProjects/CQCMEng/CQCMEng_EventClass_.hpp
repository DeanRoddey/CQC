//
// FILE NAME: CQCMEng_EventClass_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/14/2006
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
//  This is the header for the CQCMEng_EventClass.cpp, which implements
//  the info and value classes required to create a macro level
//  class. In this case the class is a wrapper around the TCQCEvent C++
//  class, and allows CML programs to manipulate events.
//
//  We also implement a simple CML level class that is used in event handlers.
//  It provides a class that macros invoked from a trigged event handler can
//  derive from in order to get access to the event that triggered the event.
//  So it just lets the derived CML class call in and get an event object
//  filled in.
//
//  Since we need to provide the event as a CML event object to the derived
//  classes, we don't store the instance data in our own value class (at the
//  C++ level.) Instead we use the standard class value class, and add CML
//  level members to it, so that the derived classes will inherit them at the
//  CML level. The event server just has to get access to the member and load
//  it up with the actual event data that came in.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TEventValue
//  PREFIX: mecv
// ---------------------------------------------------------------------------
class CQCMENGEXPORT TEventValue : public TMEngClassVal
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TEventValue
        (
            const   TString&                strName
            , const tCIDLib::TCard2         c2ClassId
            , const tCIDMacroEng::EConstTypes  eConst
        );

        TEventValue(const TEventValue&) = delete;
        TEventValue(TEventValue&&) = delete;

        ~TEventValue();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TEventValue& operator=(const TEventValue&) = delete;
        TEventValue& operator=(TEventValue&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        const TCQCEvent& cevValue() const;

        TCQCEvent& cevValue();

        TCQCEvent& cevValue
        (
            const   TCQCEvent&              evToSet
        );


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_cevValue
        //      This is the event data that our object instance provides access
        //      to.
        // -------------------------------------------------------------------
        TCQCEvent   m_cevValue;


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TEventValue,TMEngClassVal)
};



// ---------------------------------------------------------------------------
//  CLASS: TEventInfo
// PREFIX: meci
// ---------------------------------------------------------------------------
class CQCMENGEXPORT TEventInfo : public TMEngClassInfo
{
    public  :
        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static const TString& strClassPath();


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TEventInfo
        (
                    TCIDMacroEngine&        meOwner
        );

        TEventInfo(const TEventInfo&) = delete;
        TEventInfo(TEventInfo&&) = delete;

        ~TEventInfo();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TEventInfo& operator=(const TEventInfo&) = delete;
        TEventInfo& operator=(TEventInfo&&) = delete;


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
        // -------------------------------------------------------------------
        tCIDLib::TCard2 m_c2EnumId_Errors;
        tCIDLib::TCard2 m_c2EnumId_StdDrvEvs;
        tCIDLib::TCard2 m_c2MethId_AddBlock;
        tCIDLib::TCard2 m_c2MethId_AddValue;
        tCIDLib::TCard2 m_c2MethId_AsBool;
        tCIDLib::TCard2 m_c2MethId_AsCard4;
        tCIDLib::TCard2 m_c2MethId_AsCard8;
        tCIDLib::TCard2 m_c2MethId_AsFloat;
        tCIDLib::TCard2 m_c2MethId_AsInt;
        tCIDLib::TCard2 m_c2MethId_AsString;
        tCIDLib::TCard2 m_c2MethId_DefCtor;
        tCIDLib::TCard2 m_c2MethId_Format;
        tCIDLib::TCard2 m_c2MethId_GetFldChInfo;
        tCIDLib::TCard2 m_c2MethId_GetLoadChInfo;
        tCIDLib::TCard2 m_c2MethId_GetLockStInfo;
        tCIDLib::TCard2 m_c2MethId_GetMotionInfo;
        tCIDLib::TCard2 m_c2MethId_GetPresenceInfo;
        tCIDLib::TCard2 m_c2MethId_GetUserActInfo;
        tCIDLib::TCard2 m_c2MethId_GetZAlarmInfo;
        tCIDLib::TCard2 m_c2MethId_IsFldChangeEv;
        tCIDLib::TCard2 m_c2MethId_IsStdDrvEv;
        tCIDLib::TCard2 m_c2MethId_IsUserActionEv;
        tCIDLib::TCard2 m_c2MethId_Parse;
        tCIDLib::TCard2 m_c2MethId_Reset;
        tCIDLib::TCard2 m_c2MethId_SetValue;
        tCIDLib::TCard2 m_c2MethId_ValCtor;
        tCIDLib::TCard2 m_c2MethId_ValueExists;
        tCIDLib::TCard4 m_c4ErrAccess;
        tCIDLib::TCard4 m_c4ErrCvt;
        tCIDLib::TCard4 m_c4ErrFormat;
        tCIDLib::TCard4 m_c4ErrParse;
        TMEngEnumInfo*  m_pmeciErrors;
        TMEngEnumInfo*  m_pmeciStdDrvEvs;


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TEventInfo,TMEngClassInfo)
};



// ---------------------------------------------------------------------------
//  CLASS: TEventHandlerInfo
// PREFIX: meci
// ---------------------------------------------------------------------------
class CQCMENGEXPORT TEventHandlerInfo : public TMEngClassInfo
{
    public  :
        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static const TString& strHandlerData();
        static const TString& strClassPath();


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TEventHandlerInfo
        (
                    TCIDMacroEngine&        meOwner
        );

        TEventHandlerInfo(const TEventHandlerInfo&) = delete;
        TEventHandlerInfo(TEventHandlerInfo&&) = delete;

        ~TEventHandlerInfo();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TEventHandlerInfo& operator=(const TEventHandlerInfo&) = delete;
        TEventHandlerInfo& operator=(TEventHandlerInfo&&) = delete;


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
        // -------------------------------------------------------------------
        tCIDLib::TCard2 m_c2MethId_DefCtor;
        tCIDLib::TCard2 m_c2MethId_GetEvent;


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TEventHandlerInfo,TMEngClassInfo)
};

#pragma CIDLIB_POPPACK



