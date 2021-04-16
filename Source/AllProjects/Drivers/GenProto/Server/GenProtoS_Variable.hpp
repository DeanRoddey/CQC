//
// FILE NAME: GenProtoS_Variable.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/24/2003
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
//  This is the header file for the GenProtoS_Variable.cpp file, which
//  defines the TGenProtoVarInfo class. This class represents a single
//  'variable' declared in the Variables= block of the protocol file. Each
//  one has a name, a type, and a value. The value is represented by the
//  standard expression value class.
//
//  This same class is used for constants. The only difference is the state
//  of a boolean flag. Otherwise they are the same, and have a name, type,
//  and value.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: TGenProtoVarInfo
//  PREFIX: vari
// ---------------------------------------------------------------------------
class GENPROTOSEXPORT TGenProtoVarInfo : public TObject
{
    public :
        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static const TString& strKey
        (
            const   TGenProtoVarInfo&       variSrc
        );


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TGenProtoVarInfo() = delete;

        TGenProtoVarInfo
        (
            const   TString&                strName
            , const tGenProtoS::ETypes      eType
            , const tCIDLib::TBoolean       bConst
        );

        TGenProtoVarInfo
        (
            const   TString&                strName
            , const tGenProtoS::ETypes      eType
            , const tCIDLib::TBoolean       bConst
            , const TGenProtoExprVal&         evalInit
        );

        TGenProtoVarInfo
        (
            const   TGenProtoVarInfo&       variSrc
        );

        TGenProtoVarInfo(TGenProtoVarInfo&&) = delete;

        ~TGenProtoVarInfo();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TGenProtoVarInfo& operator=
        (
            const   TGenProtoVarInfo&       variSrc
        );

        TGenProtoVarInfo& operator=(TGenProtoVarInfo&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIsConst() const;

        tGenProtoS::ETypes eType() const;

        const TGenProtoExprVal& evalThis() const;

        TGenProtoExprVal& evalThis();

        const TString& strName() const;

        tCIDLib::TVoid Set
        (
            const   TGenProtoExprVal&       evalToSet
        );


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bConst
        //      Indicates whether this is a const or variable. They are
        //      indentical except that const's cannot be set.
        //
        //  m_evalCurrent
        //      This is the field value object that we keep the current value
        //      in at run time. It holds all the possible types, and handles
        //      conversions as necessary.
        //
        //  m_strName
        //      The name of the variable.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bConst;
        TGenProtoExprVal    m_evalCurrent;
        TString             m_strName;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TGenProtoVarInfo,TObject)
};

#pragma CIDLIB_POPPACK


