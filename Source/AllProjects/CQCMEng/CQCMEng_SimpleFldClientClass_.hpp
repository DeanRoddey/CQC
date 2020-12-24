//
// FILE NAME: CQCMEng_SimpleFldClientClass_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/12/2003
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
//  This is the header for the CQCMEng_SimpleFldClientClass.cpp, which
//  implements the info and value classes required to create a macro level
//  class. In this case the class enables macros to do simple CQC field
//  I/O. Many macros have very simple needs for macro I/O, so they can get
//  away with simple 'by name' reads and writes of single fields at a time.
//  This class provides that support.
//
//  With the current implementation, which is completely stateless and
//  creates a new admin proxy for each read/write, the value class is
//  just a placeholder.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)


class TCQCFldDefInfo;


// ---------------------------------------------------------------------------
//   CLASS: TSimpleFldValue
//  PREFIX: mecv
// ---------------------------------------------------------------------------
class CQCMENGEXPORT TSimpleFldValue : public TMEngClassVal
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TSimpleFldValue() = delete;

        TSimpleFldValue
        (
            const   TString&                strName
            , const tCIDLib::TCard2         c2ClassId
            , const tCIDMacroEng::EConstTypes  eConst
        );

        TSimpleFldValue(const TSimpleFldValue&) = delete;
        TSimpleFldValue(TSimpleFldValue&&) = delete;

        ~TSimpleFldValue();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TSimpleFldValue& operator=(const TSimpleFldValue&) = delete;
        TSimpleFldValue& operator=(TSimpleFldValue&&) = delete;


    private :
        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TSimpleFldValue,TMEngClassVal)
};



// ---------------------------------------------------------------------------
//  CLASS: TSimpleFldInfo
// PREFIX: meci
// ---------------------------------------------------------------------------
class CQCMENGEXPORT TSimpleFldInfo : public TMEngClassInfo
{
    public  :
        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static const TString& strClassPath();


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TSimpleFldInfo() = delete;

        TSimpleFldInfo
        (
                    TCIDMacroEngine&        meOwner
            , const TCQCSecToken&           sectUser
        );

        TSimpleFldInfo(const TSimpleFldInfo&) = delete;
        TSimpleFldInfo(TSimpleFldInfo&&) = delete;

        ~TSimpleFldInfo();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TSimpleFldInfo& operator=(const TSimpleFldInfo&) = delete;
        TSimpleFldInfo& operator=(TSimpleFldInfo&&) = delete;


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
        //  m_colSList
        //      A pointer to a vector of strings that we use for string list
        //      read/write.
        //
        //  m_pmeciXXX
        //      Pointers to class info objects for our nested classes. We do
        //      not own them.
        //
        //  m_sectUser
        //      We need a security token in order to do field writes and some other
        //      bits and pieces.
        // -------------------------------------------------------------------
        tCIDLib::TCard2     m_c2EnumId_Errors;
        tCIDLib::TCard2     m_c2EnumId_FldTokRes;
        tCIDLib::TCard2     m_c2MethId_CancelTimedFldChange;
        tCIDLib::TCard2     m_c2MethId_DefCtor;
        tCIDLib::TCard2     m_c2MethId_HasQueuedFldChange;
        tCIDLib::TCard2     m_c2MethId_QueryCQCServers;
        tCIDLib::TCard2     m_c2MethId_QueryData;
        tCIDLib::TCard2     m_c2MethId_QueryDrivers;
        tCIDLib::TCard2     m_c2MethId_QueryFields;
        tCIDLib::TCard2     m_c2MethId_QueryStatusReport;
        tCIDLib::TCard2     m_c2MethId_QueryTextVal;
        tCIDLib::TCard2     m_c2MethId_ReadBoolField;
        tCIDLib::TCard2     m_c2MethId_ReadCardField;
        tCIDLib::TCard2     m_c2MethId_ReadField;
        tCIDLib::TCard2     m_c2MethId_ReadField2;
        tCIDLib::TCard2     m_c2MethId_ReadFloatField;
        tCIDLib::TCard2     m_c2MethId_ReadIntField;
        tCIDLib::TCard2     m_c2MethId_ReadStringField;
        tCIDLib::TCard2     m_c2MethId_ReadStringListField;
        tCIDLib::TCard2     m_c2MethId_ReadTimeField;
        tCIDLib::TCard2     m_c2MethId_ReplaceFldTokens;
        tCIDLib::TCard2     m_c2MethId_SendDrvCmd;
        tCIDLib::TCard2     m_c2MethId_TimedFldChange;
        tCIDLib::TCard2     m_c2MethId_TimedFldChange2;
        tCIDLib::TCard2     m_c2MethId_WaitDriverReady;
        tCIDLib::TCard2     m_c2MethId_WriteBoolField;
        tCIDLib::TCard2     m_c2MethId_WriteCardField;
        tCIDLib::TCard2     m_c2MethId_WriteCharField;
        tCIDLib::TCard2     m_c2MethId_WriteField;
        tCIDLib::TCard2     m_c2MethId_WriteField2;
        tCIDLib::TCard2     m_c2MethId_WriteFloatField;
        tCIDLib::TCard2     m_c2MethId_WriteIntField;
        tCIDLib::TCard2     m_c2MethId_WriteStringField;
        tCIDLib::TCard2     m_c2MethId_WriteStringListField;
        tCIDLib::TCard2     m_c2MethId_WriteTimeField;
        tCIDLib::TCard4     m_c4ErrBadFldName;
        tCIDLib::TCard4     m_c4ErrQueryFailed;
        tCIDLib::TCard4     m_c4ErrReadFailed;
        tCIDLib::TCard4     m_c4ErrSendFailed;
        tCIDLib::TCard4     m_c4ErrTimedChgFailed;
        tCIDLib::TCard4     m_c4ErrWriteFailed;
        tCIDLib::TStrList   m_colSList;
        TMEngEnumInfo*      m_pmeciErrors;
        TCQCFldDefInfo*     m_pmeciFldDef;
        TMEngEnumInfo*      m_pmeciFldTokRes;
        TMEngVectorInfo*    m_pmeciFldDefList;
        TMEngVectorInfo*    m_pmeciStrList;
        TCQCSecToken        m_sectUser;


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TSimpleFldInfo,TMEngClassInfo)
};

#pragma CIDLIB_POPPACK


