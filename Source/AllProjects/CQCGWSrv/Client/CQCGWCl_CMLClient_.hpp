//
// FILE NAME: CQCGWCl_CMLClient_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/18/2010
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
//  This is the header for the CQCMacroEng_GWClientClass.cpp, which implements
//  the info and value classes required to create a macro level class. In this
//  case the class is a wrapper around the XML Gateway client class.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TCQCGWSrvCVal
//  PREFIX: mecv
// ---------------------------------------------------------------------------
class CQCGWCLEXPORT TCQCGWSrvCVal : public TMEngClassVal
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCGWSrvCVal
        (
            const   TString&                strName
            , const tCIDLib::TCard2         c2ClassId
            , const tCIDMacroEng::EConstTypes eConst
        );

        TCQCGWSrvCVal(const TCQCGWSrvCVal&) = delete;
        TCQCGWSrvCVal(TCQCGWSrvCVal&&) = delete;

        ~TCQCGWSrvCVal();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCGWSrvCVal& operator=(const TCQCGWSrvCVal&) = delete;
        TCQCGWSrvCVal& operator=(TCQCGWSrvCVal&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bDbgFormat
        (
                    TTextOutStream&         strmTarget
            , const TMEngClassInfo&         meciThis
            , const tCIDMacroEng::EDbgFmts  eFormat
            , const tCIDLib::ERadices       eRadix
            , const TCIDMacroEngine&        meOwner
        )   const final;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        const TCQCGWSrvClient& gwscValue() const;

        TCQCGWSrvClient& gwscValue();


    private :
        // -------------------------------------------------------------------
        //  Private, data members
        //
        //  m_gwscValue
        //      The C++ gateway client object that is our value.
        // -------------------------------------------------------------------
        TCQCGWSrvClient m_gwscValue;


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCGWSrvCVal,TMEngClassVal)
};



// ---------------------------------------------------------------------------
//  CLASS: TCQCGWSrvCInfo
// PREFIX: meci
// ---------------------------------------------------------------------------
class CQCGWCLEXPORT TCQCGWSrvCInfo : public TMEngClassInfo
{
    public  :
        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static const TString& strPath();


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCGWSrvCInfo
        (
                    TCIDMacroEngine&        meOwner
        );

        TCQCGWSrvCInfo(const TCQCGWSrvCInfo&) = delete;
        TCQCGWSrvCInfo(TCQCGWSrvCInfo&&) = delete;

        ~TCQCGWSrvCInfo();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCGWSrvCInfo& operator=(const TCQCGWSrvCInfo&) = delete;
        TCQCGWSrvCInfo& operator=(TCQCGWSrvCInfo&&) = delete;


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
        tCIDLib::TVoid CheckConnected
        (
                    TCIDMacroEngine&        meOwner
            , const TCQCGWSrvClient&        gwscTest
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
        // -------------------------------------------------------------------
        tCIDLib::TCard2     m_c2EnumId_Errors;
        tCIDLib::TCard2     m_c2MethId_DefCtor;
        tCIDLib::TCard2     m_c2MethId_Connect;
        tCIDLib::TCard2     m_c2MethId_CheckField;
        tCIDLib::TCard2     m_c2MethId_Disconnect;
        tCIDLib::TCard2     m_c2MethId_DoGlobalAct;
        tCIDLib::TCard2     m_c2MethId_GetPollListCount;
        tCIDLib::TCard2     m_c2MethId_IsConnected;
        tCIDLib::TCard2     m_c2MethId_MWriteField;
        tCIDLib::TCard2     m_c2MethId_Ping;
        tCIDLib::TCard2     m_c2MethId_Poll;
        tCIDLib::TCard2     m_c2MethId_RunMacro;
        tCIDLib::TCard2     m_c2MethId_SetPollList;
        tCIDLib::TCard2     m_c2MethId_WriteField;
        tCIDLib::TCard4     m_c4Err_AlreadyConn;
        tCIDLib::TCard4     m_c4Err_Connect;
        tCIDLib::TCard4     m_c4Err_Disconnect;
        tCIDLib::TCard4     m_c4Err_FldIndex;
        tCIDLib::TCard4     m_c4Err_GlobalAct;
        tCIDLib::TCard4     m_c4Err_ListSizes;
        tCIDLib::TCard4     m_c4Err_NotConnected;
        tCIDLib::TCard4     m_c4Err_Ping;
        tCIDLib::TCard4     m_c4Err_Poll;
        tCIDLib::TCard4     m_c4Err_RunMacro;
        tCIDLib::TCard4     m_c4Err_SetPollList;
        tCIDLib::TCard4     m_c4Err_WriteField;
        tCIDLib::TCard4     m_c4Err_MWriteField;
        TMEngEnumInfo*      m_pmeciErrors;
        TMEngVectorInfo*    m_pmeciStrList;


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCGWSrvCInfo,TMEngClassInfo)
};

#pragma CIDLIB_POPPACK


