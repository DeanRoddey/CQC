//
// FILE NAME: CQCEvMonEng_MonBaseClass.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/08/2014
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
//  This is the header for the CML base class that all monitors must derive from. We
//  don't need a value class in our case, we just use the standard one.
//
//  We are created by the monitor thread. We store a pointer to the owning thread
//  object in the extra storage of the value objects so that we can link each value
//  object back to the owning thread where necessary.
//
//  When run in the IDE, then the IDE creates the CML class and we are running on
//  the IDE's thread. When run normally in the event server, the monitor thread
//  creates the class and provides the thread to run it.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//

#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//  CLASS: TEvMonBaseInfo
// PREFIX: meci
// ---------------------------------------------------------------------------
class CQCEVMONENGEXP TEvMonBaseInfo : public TMEngClassInfo
{
    public  :
        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static const TString& strPath();


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TEvMonBaseInfo
        (
                    TCIDMacroEngine&        meOwner
        );

        TEvMonBaseInfo(const TEvMonBaseInfo&) = delete;
        TEvMonBaseInfo(TEvMonBaseInfo&&) = delete;

        ~TEvMonBaseInfo();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TEvMonBaseInfo& operator=(const TEvMonBaseInfo&) = delete;
        TEvMonBaseInfo& operator=(TEvMonBaseInfo&&) = delete;


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
        tCIDLib::TVoid ReplaceTokens
        (
                    TCIDMacroEngine&        meOwner
            ,       TString&                strMsg
            , const tCIDLib::TCard4         c4Tokens
            , const tCIDLib::TCard4         c4FirstInd
        );

        tCIDLib::TVoid Simulate
        (
            const   TString&                strClass
            ,       TCIDMacroEngine&        meOwner
            ,       TMEngClassVal&          mecvInst
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
        tCIDLib::TCard2         m_c2MethId_DefCtor;
        tCIDLib::TCard2         m_c2MethId_FieldChanged;
        tCIDLib::TCard2         m_c2MethId_GetName;
        tCIDLib::TCard2         m_c2MethId_LoadConfig;
        tCIDLib::TCard2         m_c2MethId_LogMsg;
        tCIDLib::TCard2         m_c2MethId_LogMsg1;
        tCIDLib::TCard2         m_c2MethId_LogMsg2;
        tCIDLib::TCard2         m_c2MethId_Idle;
        tCIDLib::TCard2         m_c2MethId_Initialize;
        tCIDLib::TCard2         m_c2MethId_ParseFldName;
        tCIDLib::TCard2         m_c2MethId_SetFieldList;
        tCIDLib::TCard2         m_c2MethId_SetIdleSecs;
        tCIDLib::TCard2         m_c2MethId_Simulate;
        tCIDLib::TCard2         m_c2MethId_Terminate;
        tCIDLib::TCard2         m_c2MethId_WriteField;
        tCIDLib::TCard2         m_c2MethId_WriteField2;
        tCIDLib::TCard4         m_c4ErrId_FieldWrite;
        tCIDLib::TCard4         m_c4ErrId_SetFieldList;
        tCIDLib::TCard4         m_c4ErrId_Simulate;
        TMEngEnumInfo*          m_pmeciErrors;
        TMEngVectorInfo*        m_pmeciStrList;


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TEvMonBaseInfo,TMEngClassInfo)
};

#pragma CIDLIB_POPPACK


