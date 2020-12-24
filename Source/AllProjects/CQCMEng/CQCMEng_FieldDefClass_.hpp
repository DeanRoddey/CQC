//
// FILE NAME: CQCMEng_FieldDefClass_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/16/2003
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
//  This is the header for the CQCMacroEng_FieldDefClass.cpp, which implements
//  the info and value classes required to create a macro level class. In this
//  case the class is a wrapper around the TCQCFldDef class. This class defines
//  a field and is used by drivers to register their list of fields with the
//  base driver class. So we have to allow the macro based driver to create
//  them so that they can register their fields.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TCQCFldDefVal
//  PREFIX: mecv
// ---------------------------------------------------------------------------
class CQCMENGEXPORT TCQCFldDefVal : public TMEngClassVal
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCFldDefVal
        (
            const   TString&                strName
            , const tCIDLib::TCard2         c2ClassId
            , const tCIDMacroEng::EConstTypes  eConst
        );

        TCQCFldDefVal
        (
            const   TString&                strName
            , const tCIDLib::TCard2         c2ClassId
            , const tCIDMacroEng::EConstTypes  eConst
            , const TCQCFldDef&             flddInit
        );

        TCQCFldDefVal(const TCQCFldDefVal&) = delete;
        TCQCFldDefVal(TCQCFldDefVal&&) = delete;

        ~TCQCFldDefVal();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCFldDefVal& operator=(const TCQCFldDefVal&) = delete;
        TCQCFldDefVal& operator=(TCQCFldDefVal&&) = delete;


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

        tCIDLib::TVoid CopyFrom
        (
            const   TMEngClassVal&          mecvToCopy
            ,       TCIDMacroEngine&        meOwner
        )   final;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        const TCQCFldDef& flddValue() const;

        TCQCFldDef& flddValue();

        TCQCFldDef& flddValue
        (
            const   TCQCFldDef&             flddValue
        );


    private :
        // -------------------------------------------------------------------
        //  Private, data members
        //
        //  m_flddValue
        //      The C++ field def object that is our value.
        // -------------------------------------------------------------------
        TCQCFldDef  m_flddValue;


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCFldDefVal,TMEngClassVal)
};



// ---------------------------------------------------------------------------
//  CLASS: TCQCFldDefInfo
// PREFIX: meci
// ---------------------------------------------------------------------------
class CQCMENGEXPORT TCQCFldDefInfo : public TMEngClassInfo
{
    public  :
        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static const TString& strClassPath();


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCFldDefInfo
        (
                    TCIDMacroEngine&        meOwner
        );

        TCQCFldDefInfo(const TCQCFldDefInfo&) = delete;
        TCQCFldDefInfo(TCQCFldDefInfo&&) = delete;

        ~TCQCFldDefInfo();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCFldDefInfo& operator=(const TCQCFldDefInfo&) = delete;
        TCQCFldDefInfo& operator=(TCQCFldDefInfo&&) = delete;


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
        tCIDLib::TVoid DoValCtor
        (
            const   tCIDLib::TCard4         c4Which
            ,       TCQCFldDefVal&          mecvToSet
            ,       TCIDMacroEngine&        meOwner
            , const tCIDLib::TCard4         c4FirstInd
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
        // -------------------------------------------------------------------
        tCIDLib::TCard2 m_c2EnumId_FldAccess;
        tCIDLib::TCard2 m_c2EnumId_FldSTypes;
        tCIDLib::TCard2 m_c2EnumId_FldTypes;
        tCIDLib::TCard2 m_c2MethId_CheckFldName;
        tCIDLib::TCard2 m_c2MethId_DefCtor;
        tCIDLib::TCard2 m_c2MethId_Equal;
        tCIDLib::TCard2 m_c2MethId_GetAccess;
        tCIDLib::TCard2 m_c2MethId_GetId;
        tCIDLib::TCard2 m_c2MethId_GetLimits;
        tCIDLib::TCard2 m_c2MethId_GetName;
        tCIDLib::TCard2 m_c2MethId_GetNoStoreOnWrite;
        tCIDLib::TCard2 m_c2MethId_GetSemType;
        tCIDLib::TCard2 m_c2MethId_GetType;
        tCIDLib::TCard2 m_c2MethId_Set1;
        tCIDLib::TCard2 m_c2MethId_Set2;
        tCIDLib::TCard2 m_c2MethId_SetWLims;
        tCIDLib::TCard2 m_c2MethId_SetWLims2;
        tCIDLib::TCard2 m_c2MethId_SetAlwaysWrite;
        tCIDLib::TCard2 m_c2MethId_SetQueuedWrite;
        tCIDLib::TCard2 m_c2MethId_SetNoStoreOnWrite;
        tCIDLib::TCard2 m_c2MethId_ValCtor1;
        tCIDLib::TCard2 m_c2MethId_ValCtor2;
        tCIDLib::TCard2 m_c2MethId_ValCtor3;
        TMEngEnumInfo*  m_pmeciFldAcc;
        TMEngEnumInfo*  m_pmeciFldSTypes;
        TMEngEnumInfo*  m_pmeciFldTypes;


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCFldDefInfo,TMEngClassInfo)
};

#pragma CIDLIB_POPPACK


