//
// FILE NAME: CQCMacroEngS_DriverClass.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/12/2003
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
//  This file implements a couple of macro level classes needed to provide
//  the core functionality of the generic driver. There are some complications
//  here because of the fact that we have an external class (one implementd
//  as in C++) which is non-final, i.e. from which real macro classes will
//  derive. This means that we have to use macro level members, not C++
//  members, to store any info, since there is no inheritance relationship,
//  at the C++ level, between the end user's macro class and the base driver
//  class we provide here. Only by using macro level data members do we
//  insure that the value class we get called on will have that needed data.
//
//  So, to allow us to store some data we need, we create separate macro
//  level classes that hold that information. We'll add these as members of
//  our base driver class, so that derived classes will inherit it, which will
//  mean that it's there for us to use at our level. The extra class
//  MEng.System.CQC.Runtime.DriverGlue stores the data that glues the C++
//  generic driver and the macro level base driver class together
//
//  In the standard scenario, CQCServer creates the C++ generic driver instance
//  and he creates a macro engine and loads the derived macro class and
//  sets a pointer to himself in our extra info member. In the simulator mode
//  used by the driver development test harness, it is backwards, i.e. the
//  derived macro driver class is created and run, and calls a Simulate()
//  method on our base class which creates a C++ generic driver object and
//  stores it in the extra info member.
//
//  Since the base driver class's value class doesn't need any C++ level
//  code or data, we just use the standard one provided by the macro engine,
//  which it uses internally for macro level value objects.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//  CLASS: TCQCDriverGlueVal
// PREFIX: mecv
// ---------------------------------------------------------------------------
class TCQCDriverGlueVal : public TMEngClassVal
{
    public  :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCDriverGlueVal
        (
            const   TString&                strName
            , const tCIDLib::TCard2         c2ClassId
            , const tCIDMacroEng::EConstTypes  eConst
        );

        TCQCDriverGlueVal(const TCQCDriverGlueVal&) = delete;
        TCQCDriverGlueVal(TCQCDriverGlueVal&&) = delete;

        ~TCQCDriverGlueVal();


        // -------------------------------------------------------------------
        //  Public methods
        // -------------------------------------------------------------------
        TCQCDriverGlueVal& operator=(const TCQCDriverGlueVal&) = delete;
        TCQCDriverGlueVal& operator=(TCQCDriverGlueVal&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bDbgFormat
        (
                    TTextOutStream&         strmTarget
            , const TMEngClassInfo&         meciThis
            , const tCIDMacroEng::EDbgFmts     eFormat
            , const tCIDLib::ERadices       eRadix
            , const TCIDMacroEngine&        meOwner
        )   const final;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid Initialize
        (
                    TCQCGenDrvGlue* const   psdrvImpl
        );

        TCQCGenDrvGlue& sdrvImpl();

        const TCQCGenDrvGlue& sdrvImpl() const;


    private :
        // -------------------------------------------------------------------
        //  Private, data members
        //
        //  m_psdrvImpl
        //      The C++ generic driver implementation object that we do our
        //      work through. We don't own it.
        // -------------------------------------------------------------------
        TCQCGenDrvGlue*     m_psdrvImpl;


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCDriverGlueVal,TMEngClassVal)
};


// ---------------------------------------------------------------------------
//  CLASS: TCQCDriverGlueInfo
// PREFIX: meci
// ---------------------------------------------------------------------------
class TCQCDriverGlueInfo : public TMEngClassInfo
{
    public  :
        // -------------------------------------------------------------------
        //  Public, static data
        // -------------------------------------------------------------------
        static const TString& strPath();


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCDriverGlueInfo
        (
                    TCIDMacroEngine&        meOwner
        );

        TCQCDriverGlueInfo(const TCQCDriverGlueInfo&) = delete;
        TCQCDriverGlueInfo(TCQCDriverGlueInfo&&) = delete;

        ~TCQCDriverGlueInfo();


        // -------------------------------------------------------------------
        //  Public methods
        // -------------------------------------------------------------------
        TCQCDriverGlueInfo& operator=(const TCQCDriverGlueInfo&) = delete;
        TCQCDriverGlueInfo& operator=(TCQCDriverGlueInfo&&) = delete;


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
        // -------------------------------------------------------------------
        tCIDLib::TCard2 m_c2MethId_DefCtor;


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCDriverGlueInfo,TMEngClassInfo)
};



// ---------------------------------------------------------------------------
//  CLASS: TCQCDriverInfo
// PREFIX: meci
// ---------------------------------------------------------------------------
class TCQCDriverInfo : public TMEngClassInfo
{
    public  :
        // -------------------------------------------------------------------
        //  Public, static data
        // -------------------------------------------------------------------
        static const TString& strCommResPath();

        static const TString& strInitResPath();

        static const TString& strVerbLevelPath();

        static const TString& strPath();


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCDriverInfo
        (
                    TCIDMacroEngine&        meOwner
        );

        TCQCDriverInfo(const TCQCDriverInfo&) = delete;
        TCQCDriverInfo(TCQCDriverInfo&&) = delete;

        ~TCQCDriverInfo();


        // -------------------------------------------------------------------
        //  Public methods
        // -------------------------------------------------------------------
        TCQCDriverInfo& operator=(const TCQCDriverInfo&) = delete;
        TCQCDriverInfo& operator=(TCQCDriverInfo&&) = delete;


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
        tCIDLib::TCard4 c4CheckFldName
        (
                    TCIDMacroEngine&        meOwner
            , const TCQCGenDrvGlue&         sdrvGlue
            , const tCIDLib::TCard4         c4StackInd
        )   const;

        tCIDLib::TCard4 c4CheckFldType
        (
                    TCIDMacroEngine&        meOwner
            , const TCQCGenDrvGlue&         sdrvGlue
            , const TString&                strNameToFind
            , const tCQCKit::EFldTypes      eToCheck
        )   const;

        tCIDLib::TVoid CheckFldType
        (
                    TCIDMacroEngine&        meOwner
            , const TCQCGenDrvGlue&         sdrvGlue
            , const tCIDLib::TCard4         c4FldId
            , const tCQCKit::EFldTypes      eToCheck
        )   const;

        tCIDLib::TVoid ReplaceTokens
        (
                    TCIDMacroEngine&        meOwner
            ,       TString&                strMsg
            , const tCIDLib::TCard4         c4Tokens
            , const tCIDLib::TCard4         c4FirstInd
        );

        tCIDLib::TVoid ThrowAnErr
        (
                    TCIDMacroEngine&        meOwner
            , const tCIDLib::TCard4         c4ToThrow
        )   const;

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
            , const MFormattable&           fmtblToken
        )   const;

        tCIDLib::TVoid ThrowAnErr
        (
                    TCIDMacroEngine&        meOwner
            , const tCIDLib::TCard4         c4ToThrow
            , const TMEngClassVal&          mecvToken1
            , const MFormattable&           fmtblToken2
        )   const;


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c2EnumId_XXX
        //      We have to create some nested enum types, so we cache the
        //      type id here for fast access later.
        //
        //  m_c2MemId_XXX
        //      To speed up finding our members, we have to use macro level
        //      data members here since this is a non-final class, we store
        //      the ids as we add them.
        //
        //  m_c2MethId_XXX
        //      To speed up dispatch, we store the ids of the methods that
        //      we set up during init.
        //
        //  m_c4ErrXXX
        //      We store the ordinals of our error enum values as we set them,
        //      so that we can use these later to throw them.
        //
        //  m_pmeciErrors
        //      We need to keep a pointer to our enum class info, to aid in
        //      exception throwing. We don't own it.
        // -------------------------------------------------------------------
        tCIDLib::TCard2         m_c2EnumId_CommResults;
        tCIDLib::TCard2         m_c2EnumId_Errors;
        tCIDLib::TCard2         m_c2EnumId_InitRes;
        tCIDLib::TCard2         m_c2EnumId_VerboseLevel;
        tCIDLib::TCard2         m_c2EnumId_ZoneStates;
        tCIDLib::TCard2         m_c2MemId_Glue;
        tCIDLib::TCard2         m_c2MethId_ClientCmd;
        tCIDLib::TCard2         m_c2MethId_Connect;
        tCIDLib::TCard2         m_c2MethId_DefCtor;
        tCIDLib::TCard2         m_c2MethId_DelConfigStr;
        tCIDLib::TCard2         m_c2MethId_FldChBool;
        tCIDLib::TCard2         m_c2MethId_FldChCard;
        tCIDLib::TCard2         m_c2MethId_FldChFloat;
        tCIDLib::TCard2         m_c2MethId_FldChInt;
        tCIDLib::TCard2         m_c2MethId_FldChString;
        tCIDLib::TCard2         m_c2MethId_FldChStrList;
        tCIDLib::TCard2         m_c2MethId_FldChTime;
        tCIDLib::TCard2         m_c2MethId_FldIdFromName;
        tCIDLib::TCard2         m_c2MethId_FindFldIdCl;
        tCIDLib::TCard2         m_c2MethId_FindFldIdSub;
        tCIDLib::TCard2         m_c2MethId_FindFldIdPNS;
        tCIDLib::TCard2         m_c2MethId_FreeCommRes;
        tCIDLib::TCard2         m_c2MethId_GetArchVer;
        tCIDLib::TCard2         m_c2MethId_GetASCIITermMsg;
        tCIDLib::TCard2         m_c2MethId_GetASCIITermMsg2;
        tCIDLib::TCard2         m_c2MethId_GetASCIIStartStopMsg;
        tCIDLib::TCard2         m_c2MethId_GetASCIIStartStopMsg2;
        tCIDLib::TCard2         m_c2MethId_GetConfigStr;
        tCIDLib::TCard2         m_c2MethId_GetCommRes;
        tCIDLib::TCard2         m_c2MethId_GetDriverInfo;
        tCIDLib::TCard2         m_c2MethId_GetFldErrState;
        tCIDLib::TCard2         m_c2MethId_GetFldInfoById;
        tCIDLib::TCard2         m_c2MethId_GetFldInfoByName;
        tCIDLib::TCard2         m_c2MethId_GetFldName;
        tCIDLib::TCard2         m_c2MethId_GetIsConnected;
        tCIDLib::TCard2         m_c2MethId_GetMoniker;
        tCIDLib::TCard2         m_c2MethId_GetStartLenMsg;
        tCIDLib::TCard2         m_c2MethId_GetStartLenMsg2;
        tCIDLib::TCard2         m_c2MethId_GetStartStopMsg;
        tCIDLib::TCard2         m_c2MethId_GetStartStopMsg2;
        tCIDLib::TCard2         m_c2MethId_GetTermedMsg;
        tCIDLib::TCard2         m_c2MethId_GetTermedMsg2;
        tCIDLib::TCard2         m_c2MethId_GetVerboseLevel;
        tCIDLib::TCard2         m_c2MethId_FormatStrList;
        tCIDLib::TCard2         m_c2MethId_IncBadMsgs;
        tCIDLib::TCard2         m_c2MethId_IncFailedWrite;
        tCIDLib::TCard2         m_c2MethId_IncNaks;
        tCIDLib::TCard2         m_c2MethId_IncReconfigured;
        tCIDLib::TCard2         m_c2MethId_IncTimeouts;
        tCIDLib::TCard2         m_c2MethId_IncUnknownMsgs;
        tCIDLib::TCard2         m_c2MethId_IncUnknownWrite;
        tCIDLib::TCard2         m_c2MethId_InitOther;
        tCIDLib::TCard2         m_c2MethId_InitSerial;
        tCIDLib::TCard2         m_c2MethId_InitSocket;
        tCIDLib::TCard2         m_c2MethId_LogMsg;
        tCIDLib::TCard2         m_c2MethId_LogMsg1;
        tCIDLib::TCard2         m_c2MethId_LogMsg2;
        tCIDLib::TCard2         m_c2MethId_LogMsg3;
        tCIDLib::TCard2         m_c2MethId_Poll;
        tCIDLib::TCard2         m_c2MethId_QueryBoolVal;
        tCIDLib::TCard2         m_c2MethId_QueryBufVal;
        tCIDLib::TCard2         m_c2MethId_QueryCardVal;
        tCIDLib::TCard2         m_c2MethId_QueryIntVal;
        tCIDLib::TCard2         m_c2MethId_QueryTextVal;
        tCIDLib::TCard2         m_c2MethId_ReadBoolFld;
        tCIDLib::TCard2         m_c2MethId_ReadBoolFldByName;
        tCIDLib::TCard2         m_c2MethId_ReadCardFld;
        tCIDLib::TCard2         m_c2MethId_ReadCardFldByName;
        tCIDLib::TCard2         m_c2MethId_ReadFloatFld;
        tCIDLib::TCard2         m_c2MethId_ReadFloatFldByName;
        tCIDLib::TCard2         m_c2MethId_ReadIntFld;
        tCIDLib::TCard2         m_c2MethId_ReadIntFldByName;
        tCIDLib::TCard2         m_c2MethId_ReadStringFld;
        tCIDLib::TCard2         m_c2MethId_ReadStringFldByName;
        tCIDLib::TCard2         m_c2MethId_ReadStrListFld;
        tCIDLib::TCard2         m_c2MethId_ReadStrListFldByName;
        tCIDLib::TCard2         m_c2MethId_ReadTimeFld;
        tCIDLib::TCard2         m_c2MethId_ReadTimeFldByName;
        tCIDLib::TCard2         m_c2MethId_SendLoadEv;
        tCIDLib::TCard2         m_c2MethId_SendLoadEv2;
        tCIDLib::TCard2         m_c2MethId_SendLockEv;
        tCIDLib::TCard2         m_c2MethId_SendMotionEv;
        tCIDLib::TCard2         m_c2MethId_SendMotionEv2;
        tCIDLib::TCard2         m_c2MethId_SendPresenceEv;
        tCIDLib::TCard2         m_c2MethId_SendUserActEv;
        tCIDLib::TCard2         m_c2MethId_SendZoneEv;
        tCIDLib::TCard2         m_c2MethId_SendZoneEv2;
        tCIDLib::TCard2         m_c2MethId_SetAllErrStates;
        tCIDLib::TCard2         m_c2MethId_SetConfigStr;
        tCIDLib::TCard2         m_c2MethId_SetFields;
        tCIDLib::TCard2         m_c2MethId_SetFldErrState;
        tCIDLib::TCard2         m_c2MethId_SetPollTimes;
        tCIDLib::TCard2         m_c2MethId_SetTOExtension;
        tCIDLib::TCard2         m_c2MethId_SetVerboseLevel;
        tCIDLib::TCard2         m_c2MethId_Simulate;
        tCIDLib::TCard2         m_c2MethId_Terminate;
        tCIDLib::TCard2         m_c2MethId_UseDefFldValue;
        tCIDLib::TCard2         m_c2MethId_VerbosityChanged;
        tCIDLib::TCard2         m_c2MethId_WaitConfig;
        tCIDLib::TCard2         m_c2MethId_WriteBool;
        tCIDLib::TCard2         m_c2MethId_WriteBoolByName;
        tCIDLib::TCard2         m_c2MethId_WriteCard;
        tCIDLib::TCard2         m_c2MethId_WriteCardByName;
        tCIDLib::TCard2         m_c2MethId_WriteFloat;
        tCIDLib::TCard2         m_c2MethId_WriteFloatByName;
        tCIDLib::TCard2         m_c2MethId_WriteInt;
        tCIDLib::TCard2         m_c2MethId_WriteIntByName;
        tCIDLib::TCard2         m_c2MethId_WriteString;
        tCIDLib::TCard2         m_c2MethId_WriteStringByName;
        tCIDLib::TCard2         m_c2MethId_WriteStrList;
        tCIDLib::TCard2         m_c2MethId_WriteStrListByName;
        tCIDLib::TCard2         m_c2MethId_WriteTime;
        tCIDLib::TCard2         m_c2MethId_WriteTimeByName;
        tCIDLib::TCard2         m_c2TypeId_CommPort;
        tCIDLib::TCard2         m_c2TypeId_FldList;
        tCIDLib::TCard2         m_c2TypeId_StreamSocket;
        tCIDLib::TCard4         m_c4ErrAccessFailed;
        tCIDLib::TCard4         m_c4ErrAlreadyConfiged;
        tCIDLib::TCard4         m_c4ErrAlreadyConnected;
        tCIDLib::TCard4         m_c4ErrBadFldType;
        tCIDLib::TCard4         m_c4ErrConvertErr;
        tCIDLib::TCard4         m_c4ErrCppExcept;
        tCIDLib::TCard4         m_c4ErrDelCfgStr;
        tCIDLib::TCard4         m_c4ErrFldNotFound;
        tCIDLib::TCard4         m_c4ErrFldWriteErr;
        tCIDLib::TCard4         m_c4ErrGetCfgStr;
        tCIDLib::TCard4         m_c4ErrGetMsg;
        tCIDLib::TCard4         m_c4ErrLimitViolation;
        tCIDLib::TCard4         m_c4ErrNotConnected;
        tCIDLib::TCard4         m_c4ErrNoOverride;
        tCIDLib::TCard4         m_c4ErrSetCfgStr;
        tCIDLib::TCard4         m_c4ErrUnknownConnType;
        tCIDLib::TCard4         m_c4ErrUnknownExcept;
        tCIDLib::TCard4         m_c4ErrFldRegFailed;
        tCIDLib::TCard4         m_c4ErrFldIdNotFound;
        tCIDLib::TCard4         m_c4ErrSetVerbErr;
        TMEngEnumInfo*          m_pmeciCommResults;
        TMEngEnumInfo*          m_pmeciDevClasses;
        TMEngEnumInfo*          m_pmeciErrors;
        TMEngEnumInfo*          m_pmeciInitRes;
        TMEngEnumInfo*          m_pmeciVerboseLevel;
        TMEngEnumInfo*          m_pmeciZoneStates;
        TMEngTextOutStreamVal*  m_pmecvStrm;
        TString                 m_strTmp;


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCDriverInfo,TMEngClassInfo)
};



#pragma CIDLIB_POPPACK


