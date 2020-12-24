//
// FILE NAME: GenProtoS_ExprValue.hpp
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
//  This is the header file for the GenProtoS_ExprValue.cpp file, which
//  defines the TGenProtoExprVal classes.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)


class TGenProtoCtx;

// ---------------------------------------------------------------------------
//   CLASS: TGenProtoExprVal
//  PREFIX: eval
// ---------------------------------------------------------------------------
class TGenProtoExprVal : public TObject, public MFormattable
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TGenProtoExprVal
        (
            const   tGenProtoS::ETypes      eType
        );

        TGenProtoExprVal
        (
            const   tCIDLib::TBoolean       bInitVal
        );

        TGenProtoExprVal
        (
            const   tCIDLib::TCard1         c1InitVal
        );

        TGenProtoExprVal
        (
            const   tCIDLib::TCard2         c2InitVal
        );

        TGenProtoExprVal
        (
            const   tCIDLib::TCard4         c4InitVal
        );

        TGenProtoExprVal
        (
            const   tCIDLib::TFloat4        f4InitVal
        );

        TGenProtoExprVal
        (
            const   tCIDLib::TFloat8        f8InitVal
        );

        TGenProtoExprVal
        (
            const   tCIDLib::TInt1          i1InitVal
        );

        TGenProtoExprVal
        (
            const   tCIDLib::TInt2          i2InitVal
        );

        TGenProtoExprVal
        (
            const   tCIDLib::TInt4          i4InitVal
        );

        TGenProtoExprVal
        (
            const   TString&                strInitVal
        );

        TGenProtoExprVal
        (
            const   tGenProtoS::ETokens     eToken
        );

        TGenProtoExprVal
        (
            const   TGenProtoExprVal&       evalToCopy
        );

        ~TGenProtoExprVal();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TGenProtoExprVal& operator=
        (
            const   TGenProtoExprVal&       evalToAssign
        );


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCheckType
        (
            const   tGenProtoS::ETypes      eToCheck
        )   const;

        tCIDLib::TBoolean bIsCardType() const;

        tCIDLib::TBoolean bIsCharType() const;

        tCIDLib::TBoolean bIsEqual
        (
            const   TGenProtoExprVal&       evalToCheck
        )   const;

        tCIDLib::TBoolean bIsFloatType() const;

        tCIDLib::TBoolean bIsIntType() const;

        tCIDLib::TBoolean bIsZero() const;

        tCIDLib::TBoolean bValue() const;

        tCIDLib::TBoolean bValue
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TCard1 c1Value() const;

        tCIDLib::TCard1 c1Value
        (
            const   tCIDLib::TCard1         c1ToSet
        );

        tCIDLib::TCard2 c2Value() const;

        tCIDLib::TCard2 c2Value
        (
            const   tCIDLib::TCard2         c2ToSet
        );

        tCIDLib::TCard4 c4Value() const;

        tCIDLib::TCard4 c4Value
        (
            const   tCIDLib::TCard4         c4ToSet
        );

        tGenProtoS::ETypes eType() const;

        tCIDLib::TFloat4 f4Value() const;

        tCIDLib::TFloat4 f4Value
        (
            const   tCIDLib::TFloat4        f4ToSet
        );

        tCIDLib::TFloat8 f8Value() const;

        tCIDLib::TFloat8 f8Value
        (
            const   tCIDLib::TFloat8        f8ToSet
        );

        tCIDLib::TVoid FormatToStr
        (
                    TString&                strToFill
            , const tCIDLib::ERadices       eRadix
            , const tCIDLib::TCard4         c4Width
            , const tCIDLib::EHJustify      eJustify
            , const tCIDLib::TCh            chFill
        );

        tCIDLib::TInt1 i1Value() const;

        tCIDLib::TInt1 i1Value
        (
            const   tCIDLib::TInt1          i1ToSet
        );

        tCIDLib::TInt2 i2Value() const;

        tCIDLib::TInt2 i2Value
        (
            const   tCIDLib::TInt2          i2ToSet
        );

        tCIDLib::TInt4 i4Value() const;

        tCIDLib::TInt4 i4Value
        (
            const   tCIDLib::TInt4          i4ToSet
        );

        tCIDLib::TVoid Reset
        (
            const   tGenProtoS::ETypes      eNewType
            , const TString&                strInit
        );

        tCIDLib::TVoid Reset
        (
            const   tGenProtoS::ETypes      eNewType
        );

        TString& strValue();

        const TString& strValue() const;

        const TString& strValue
        (
            const   TString&                strToSet
        );

        tCIDLib::TVoid SetFrom
        (
            const   TGenProtoExprVal&       evalSrc
        );

        tCIDLib::TVoid ToLower();

        tCIDLib::TVoid ToUpper();

        tCIDLib::TVoid ValueToMemBuf
        (
                    TGenProtoCtx&           ctxTarget
            ,       TMemBuf&                mbufTarget
            ,       tCIDLib::TCard4&        c4AtOfs
        )   const;


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
            tCIDLib::TCard1     c1Value;
            tCIDLib::TCard2     c2Value;
            tCIDLib::TCard4     c4Value;
            tCIDLib::TFloat4    f4Value;
            tCIDLib::TFloat8    f8Value;
            tCIDLib::TInt1      i1Value;
            tCIDLib::TInt2      i2Value;
            tCIDLib::TInt4      i4Value;

            UValStorage();
            UValStorage(const tCIDLib::TBoolean bInit);
            UValStorage(const tCIDLib::TCard1 c1Init);
            UValStorage(const tCIDLib::TCard2 c2Init);
            UValStorage(const tCIDLib::TCard4 c4Init);
            UValStorage(const tCIDLib::TFloat4 f4Init);
            UValStorage(const tCIDLib::TFloat8 f8Init);
            UValStorage(const tCIDLib::TInt1 i1Init);
            UValStorage(const tCIDLib::TInt2 i2Init);
            UValStorage(const tCIDLib::TInt4 i4Init);
        };


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid BadCast
        (
            const   tGenProtoS::ETypes    eSrcType
            , const tGenProtoS::ETypes    eDestType
        )   const;

        tCIDLib::TVoid FaultInStrVal() const;

        tCIDLib::TVoid InitVal();

        tCIDLib::TVoid ValidateType
        (
            const   tGenProtoS::ETypes    eToCheck
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
        tGenProtoS::ETypes  m_eType;
        mutable TString*    m_pstrValue;
        UValStorage         m_uValue;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TGenProtoExprVal,TObject)
};

#pragma CIDLIB_POPPACK
