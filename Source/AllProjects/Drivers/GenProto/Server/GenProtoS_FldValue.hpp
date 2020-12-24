//
// FILE NAME: GenProtoS_FldValue.hpp
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
//  This is the header file for the GenProtoS_FldValue.cpp file, which
//  defines the TGenProtoFldVal class. We have many classes which differ only
//  in the the type of the value they store, which will be one of the possible
//  field types supported by CQC drivers.
//
//  In order to avoid the massive overhead of having separate derivatives for
//  each type of these many classes, we just this simple class as a smart
//  union to store the values and insure that the get/set methods are type
//  safe at runtime.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)

class TGenProtoExprVal;

// ---------------------------------------------------------------------------
//   CLASS: TGenProtoFldVal
//  PREFIX: fval
// ---------------------------------------------------------------------------
class GENPROTOSEXPORT TGenProtoFldVal

    : public TObject, public MFormattable
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TGenProtoFldVal
        (
            const   tCQCKit::EFldTypes      eType
        );

        TGenProtoFldVal
        (
            const   TGenProtoFldVal&        fvalSrc
        );

        ~TGenProtoFldVal();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TGenProtoFldVal& operator=
        (
            const   TGenProtoFldVal&        fvalSrc
        );


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCheckFldType
        (
            const   tCQCKit::EFldTypes      eToCheck
        )   const;

        tCIDLib::TBoolean bSetFrom
        (
            const   TGenProtoExprVal&       evalSrc
        );

        tCIDLib::TBoolean bValue() const;

        tCIDLib::TBoolean bValue
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TCard4 c4Value() const;

        tCIDLib::TCard4 c4Value
        (
            const   tCIDLib::TCard4         c4ToSet
        );

        tCIDLib::TFloat8 f8Value() const;

        tCIDLib::TFloat8 f8Value
        (
            const   tCIDLib::TFloat8        f8ToSet
        );

        tCIDLib::TInt4 i4Value() const;

        tCIDLib::TInt4 i4Value
        (
            const   tCIDLib::TInt4          i4ToSet
        );

        const TString& strValue() const;

        const TString& strValue
        (
            const   TString&                strToSet
        );

        tCIDLib::TVoid StreamValue
        (
                    TBinOutStream&          strmTarget
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid FormatTo
        (
                    TTextOutStream&         strmTarget
        )   const final;


    private :
        // -------------------------------------------------------------------
        //  Private class types
        // -------------------------------------------------------------------
        union UValStorage
        {
            tCIDLib::TBoolean   bValue;
            tCIDLib::TCard4     c4Value;
            tCIDLib::TFloat8    f8Value;
            tCIDLib::TInt4      i4Value;
        };


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid BadCast
        (
            const   tCQCKit::EFldTypes      eSrcType
            , const tCQCKit::EFldTypes      eDestType
        );

        tCIDLib::TVoid ValidateType
        (
            const   tCQCKit::EFldTypes      eToCheck
        )   const;


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_eType
        //      The type indicated to us in our constructor. This tells us
        //      what type of value we store.
        //
        //  m_pstrValue
        //      Because we have to be able to give back our value in string
        //      form, no matter what our actual type, and we don't want to
        //      do it by value, we keep the string variant out of the union
        //      and use it even when our type is something else. If our type
        //      isn't actually string, we fault it only when the string form
        //      is asked for. It has to be mutable since we fault it in
        //      sometimes.
        //
        //  m_uValue
        //      The union that we store the value in. The m_eType field tells
        //      us which of them is valid.
        // -------------------------------------------------------------------
        tCQCKit::EFldTypes  m_eType;
        mutable TString*    m_pstrValue;
        UValStorage         m_uValue;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TGenProtoFldVal,TObject)
};

#pragma CIDLIB_POPPACK

