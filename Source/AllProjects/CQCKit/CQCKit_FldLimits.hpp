//
// FILE NAME: CQCKit_FldLimits.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/05/2003
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
//  Each CQC field has an optional 'limits' description, which is a string.
//  Limits come in a number of variants, each of which uses a particular
//  convention, or set of conventions, to encode it's limit info into the
//  string that represents the limits. The string is stored in the TCQCFldDef
//  class, which describes a field.

//  The string starts with a name and colon, where the name indicates the
//  type of limit. That is used to create a particular type of limit object
//  that will parse the rest of the content and provide the limit checking.
//
//  The available limit schemes for each field type are:
//
//  EFldType_Card
//  EFldType_Int
//  EFldType_Float
//      Range:
//          The limits are a min/max range (inclusive), comma delimited. So,
//          if field can be from -1 to 10, it would be "Range:-1,10".
//      GtThan:
//          The value must be greater than the provided value. So if it must
//          be greater than -80, it would be "GtThan:-80".
//      LsThan:
//          The value must be less than the provided value. So if it must
//          be less than 16, it would be "LsThan:16".
//
//  EFldType_String
//      Regex:
//          A CIDLib regular expression. So if it must be two lower case words
//          separated by a slash, it would be "Regex:[a-z]*/[a-z]*". A partial
//          match is not allowed, the whole value must match the pattern.
//      Enum:
//          A list of comma separated values. Leading/trailing spaces are
//          stripped if present! So the value has to be "This", "That", or
//          "The Other", it would be "Enum:This,That,The Other". When the
//          value is compared, it is case sensitive.
//
//  EFldType_Boolean
//  EFldType_Time
//  EFldType_StringList
//      These have no defined limits.
//
//
//  We define a base class which provides the polymorphic base interface for all
//  limit objects, then derive a class for each available limit scheme.
//
//  Note that GtThan/LsThan are implemented via the range limit classes, since they
//  are just a range with one end pegged at the min or max. This cuts down a lot on
//  the amount of code required. For consistency, for the numeric types, we will
//  install a limits object, even if no limits are set, and just set the limits to
//  the type's natural range. This prevents having to have reundanant string to
//  binary conversion code.
//
//  For sanity's sake, we have a base class for each of the fundamental numeric types,
//  and each type of limit that is legal for the numerics have a separate derivative
//  of those classes, one each for each numeric type. That way, a storage class
//  (which is type specific) can just call a single method, typed for it's field type,
//  and have the conversion binary and the validation done. This avoids a lot of ugly
//  code and/or a double conversion, which  would be required if we passed the text
//  value into the limit checker and then turned around and had to convert it again
//  to store the value.
//
//  As a convenience to client side apps, we provide a suggestion for the optimal
//  visual representation of the kind of data type and limits that we represent.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TCQCFldLimit
//  PREFIX: fldl
// ---------------------------------------------------------------------------
class CQCKITEXPORT TCQCFldLimit : public TObject
{
    public :
        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        [[nodiscard]] static TCQCFldLimit* pfldlMakeNew
        (
            const   TCQCFldDef&             flddSrcFld
        );

        [[nodiscard]] static TCQCFldLimit* pfldlMakeNew
        (
            const   TString&                strName
            , const TString&                strLimStr
            , const tCQCKit::EFldTypes      eType
        );


        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TCQCFldLimit(const TCQCFldLimit&) = default;
        TCQCFldLimit(TCQCFldLimit&&) = default;

        ~TCQCFldLimit();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCFldLimit& operator=(const TCQCFldLimit&) = default;
        TCQCFldLimit& operator=(TCQCFldLimit&&) = default;


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        virtual tCIDLib::TBoolean bNextPrevVal
        (
            const   TString&                strCurVal
            ,       TString&                strNewVal
            , const tCIDLib::TBoolean       bNext
            , const tCIDLib::TBoolean       bWrapOK
        )   const;

        virtual [[nodiscard]] tCIDLib::TBoolean bSameLimits
        (
            const   TCQCFldLimit&           fldlSrc
        )   const;

        virtual [[nodiscard]] tCIDLib::TBoolean bParseLimits
        (
            const   TString&                strLimits
        ) = 0;

        virtual [[nodiscard]] tCIDLib::TBoolean bValidate
        (
            const   TString&                strToValidate
        );

        virtual [[nodiscard]] tCQCKit::EOptFldReps eOptimalRep() const = 0;

        virtual tCIDLib::TVoid QueryDefVal
        (
                    TString&                strToFill
        ) = 0;

        virtual tCIDLib::TCard4 c4QueryValues
        (
                    tCIDLib::TStrList&      colToFill
        )   const;

        virtual tCIDLib::TVoid FormatToText
        (
                    TString&                strToFill
        );


    protected :
        // -------------------------------------------------------------------
        //  Hidden constructors
        // -------------------------------------------------------------------
        TCQCFldLimit();


        // -------------------------------------------------------------------
        //  Protected, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bBreakOutLimits
        (
            const   TString&                strLimits
            , const TString&                strExpectedPref
        );

        tCIDLib::TBoolean bBreakOutLimits
        (
            const   TString&                strLimits
            , const TString&                strExpectedPref
            ,       TString&                strParm1
        );

        tCIDLib::TBoolean bBreakOutLimits
        (
            const   TString&                strLimits
            , const TString&                strExpectedPref
            ,       TString&                strParm1
            ,       TString&                strParm2
        );

        tCIDLib::TCard2 c2CheckFmtVersion
        (
            const   tCIDLib::TCard2         c2Expected
            ,       TBinInStream&           strmSrc
        );

        tCIDLib::TVoid ValidateTypes
        (
            const   TClass&                 clsSrc
        )   const;


    private :
        // -------------------------------------------------------------------
        //  Private, static methods
        // -------------------------------------------------------------------
        static tCIDLib::TVoid CantTakeLimitType
        (
            const   TString&                strName
            , const TString&                strLimits
        );


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCFldLimit,TObject)
};



// ---------------------------------------------------------------------------
//   CLASS: TCQCFldBoolLimit
//  PREFIX: fldl
// ---------------------------------------------------------------------------
class CQCKITEXPORT TCQCFldBoolLimit : public TCQCFldLimit
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TCQCFldBoolLimit();

        TCQCFldBoolLimit(const TCQCFldBoolLimit&) = default;
        TCQCFldBoolLimit(TCQCFldBoolLimit&&) = default;

        ~TCQCFldBoolLimit();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCFldBoolLimit& operator=(const TCQCFldBoolLimit&) = default;
        TCQCFldBoolLimit& operator=(TCQCFldBoolLimit&&) = default;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        [[nodiscard]] tCIDLib::TBoolean bParseLimits
        (
            const   TString&                strLimits
        )   final;

        [[nodiscard]] tCIDLib::TBoolean bValidate
        (
            const   TString&                strToValidate
        )   final;

        tCIDLib::TCard4 c4QueryValues
        (
                    tCIDLib::TStrList&      colToFill
        )   const final;

        [[nodiscard]] tCQCKit::EOptFldReps eOptimalRep() const final;

        tCIDLib::TVoid QueryDefVal
        (
                    TString&                strToFill
        )   final;


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        virtual tCQCKit::EValResults eValidate
        (
            const   TString&                strToValidate
            ,       tCIDLib::TBoolean&      bToFill
        )   const;

        virtual tCQCKit::EValResults eValidate
        (
            const   tCIDLib::TBoolean       bToVal
        )   const;


    private :
        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCFldBoolLimit,TCQCFldLimit)
};



// ---------------------------------------------------------------------------
//   CLASS: TCQCFldCardLimit
//  PREFIX: fldl
// ---------------------------------------------------------------------------
class CQCKITEXPORT TCQCFldCardLimit : public TCQCFldLimit
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCFldCardLimit();

        TCQCFldCardLimit(const TCQCFldCardLimit&) = default;
        TCQCFldCardLimit(TCQCFldCardLimit&&) = default;

        ~TCQCFldCardLimit();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCFldCardLimit& operator=(const TCQCFldCardLimit&) = default;
        TCQCFldCardLimit& operator=(TCQCFldCardLimit&&) = default;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bNextPrevVal
        (
            const   TString&                strCurVal
            ,       TString&                strNewVal
            , const tCIDLib::TBoolean       bNext
            , const tCIDLib::TBoolean       bWrapOK
        )   const final;

        [[nodiscard]] tCIDLib::TBoolean bParseLimits
        (
            const   TString&                strLimits
        )   override;

        [[nodiscard]] tCIDLib::TBoolean bValidate
        (
            const   TString&                strToValidate
        )   final;

        [[nodiscard]] tCQCKit::EOptFldReps eOptimalRep() const final;

        tCIDLib::TVoid QueryDefVal
        (
                    TString&                strToFill
        )   final;

        tCIDLib::TCard4 c4QueryValues
        (
                    tCIDLib::TStrList&      colToFill
        )   const final;


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        virtual tCQCKit::EValResults eValidate
        (
            const   TString&                strToValidate
            , COP   tCIDLib::TCard4&        c4ToFill
        )   const;

        virtual tCQCKit::EValResults eValidate
        (
            const   tCIDLib::TCard4         c4ToVal
        )   const;


        // -------------------------------------------------------------------
        //  Public, non-virtual mmethods
        // -------------------------------------------------------------------
        [[nodiscard]] tCIDLib::TCard4 c4LimitRange() const
        {
            // This will return zero if the range is at the maximum!
            return (m_c4Max - m_c4Min) + 1;
        }

        [[nodiscard]] tCIDLib::TCard4 c4Max() const
        {
            return m_c4Max;
        }

        [[nodiscard]] tCIDLib::TCard4 c4Min() const
        {
            return m_c4Min;
        }


    protected :
        // -------------------------------------------------------------------
        //  Hidden Constructors
        // -------------------------------------------------------------------
        TCQCFldCardLimit
        (
            const   tCIDLib::TCard4         c4Min
            , const tCIDLib::TCard4         c4Max
        );


        // -------------------------------------------------------------------
        //  Protected, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid SetRange
        (
            const   tCIDLib::TCard4         c4Min
            , const tCIDLib::TCard4         c4Max
        );


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4Max
        //  m_c4Min
        //      The values, inclusive, of the range. They will not be allowed
        //      to get inverted, so max is always greater than min.
        // -------------------------------------------------------------------
        tCIDLib::TCard4 m_c4Max;
        tCIDLib::TCard4 m_c4Min;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCFldCardLimit,TCQCFldLimit)
};



// ---------------------------------------------------------------------------
//   CLASS: TCQCFldFloatLimit
//  PREFIX: fldl
// ---------------------------------------------------------------------------
class CQCKITEXPORT TCQCFldFloatLimit : public TCQCFldLimit
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCFldFloatLimit();

        TCQCFldFloatLimit(const TCQCFldFloatLimit&) = default;
        TCQCFldFloatLimit(TCQCFldFloatLimit&&) = default;

        ~TCQCFldFloatLimit();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCFldFloatLimit& operator=(const TCQCFldFloatLimit&) = default;
        TCQCFldFloatLimit& operator=(TCQCFldFloatLimit&&) = default;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        [[nodiscard]] tCIDLib::TBoolean bParseLimits
        (
            const   TString&                strLimits
        )   override;

        [[nodiscard]] tCIDLib::TBoolean bValidate
        (
            const   TString&                strToValidate
        )   final;

        [[nodiscard]] tCQCKit::EOptFldReps eOptimalRep() const final;

        tCIDLib::TVoid QueryDefVal
        (
                    TString&                strToFill
        )   final;


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        virtual tCQCKit::EValResults eValidate
        (
            const   TString&                strToValidate
            ,       tCIDLib::TFloat8&       f8ToFill
        )   const;

        virtual tCQCKit::EValResults eValidate
        (
            const   tCIDLib::TFloat8&       f8ToVal
        )   const;


        // -------------------------------------------------------------------
        //  Public, non-virtual mmethods
        // -------------------------------------------------------------------
        [[nodiscard]] tCIDLib::TFloat8 f8LimitRange() const
        {
            // Note that this will return zero if the range is at the max!
            return (m_f8Max - m_f8Min) + kCIDLib::f8Epsilon;
        }

        [[nodiscard]] tCIDLib::TFloat8 f8Max() const
        {
            return m_f8Max;
        }

        [[nodiscard]] tCIDLib::TFloat8 f8Min() const
        {
            return m_f8Min;
        }


    protected :
        // -------------------------------------------------------------------
        //  Hidden Constructors
        // -------------------------------------------------------------------
        TCQCFldFloatLimit
        (
            const   tCIDLib::TFloat8        f8Min
            , const tCIDLib::TFloat8        f8Max
        );


        // -------------------------------------------------------------------
        //  Protected, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid SetRange
        (
            const   tCIDLib::TFloat8        f8Min
            , const tCIDLib::TFloat8        f8Max
        );


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_f8Max
        //  m_f8Min
        //      The values, inclusive, of the range. They will not be allowed
        //      to get inverted, so max is always greater than min.
        // -------------------------------------------------------------------
        tCIDLib::TFloat8    m_f8Max;
        tCIDLib::TFloat8    m_f8Min;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCFldFloatLimit,TCQCFldLimit)
};


// ---------------------------------------------------------------------------
//   CLASS: TCQCFldIntLimit
//  PREFIX: fldl
// ---------------------------------------------------------------------------
class CQCKITEXPORT TCQCFldIntLimit : public TCQCFldLimit
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCFldIntLimit();

        TCQCFldIntLimit(const TCQCFldIntLimit&) = default;
        TCQCFldIntLimit(TCQCFldIntLimit&&) = default;

        ~TCQCFldIntLimit();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCFldIntLimit& operator=(const TCQCFldIntLimit&) = default;
        TCQCFldIntLimit& operator=(TCQCFldIntLimit&&) = default;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bNextPrevVal
        (
            const   TString&                strCurVal
            ,       TString&                strNewVal
            , const tCIDLib::TBoolean       bNext
            , const tCIDLib::TBoolean       bWrapOK
        )   const final;

        [[nodiscard]] tCIDLib::TBoolean bParseLimits
        (
            const   TString&                strLimits
        )   override;

        [[nodiscard]] tCIDLib::TBoolean bValidate
        (
            const   TString&                strToValidate
        )   final;

        [[nodiscard]] tCQCKit::EOptFldReps eOptimalRep() const final;

        tCIDLib::TVoid QueryDefVal
        (
                    TString&                strToFill
        )   final;

        tCIDLib::TCard4 c4QueryValues
        (
                    tCIDLib::TStrList&      colToFill
        )   const final;


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        virtual tCQCKit::EValResults eValidate
        (
            const   TString&                strToValidate
            ,       tCIDLib::TInt4&         i4ToFill
        )   const;

        virtual tCQCKit::EValResults eValidate
        (
            const   tCIDLib::TInt4          i4ToVal
        )   const;


        // -------------------------------------------------------------------
        //  Public, non-virtual mmethods
        // -------------------------------------------------------------------
        [[nodiscard]] tCIDLib::TInt4 i4LimitRange() const
        {
            // Note that this will return 0 if we are at the max range!
            return (m_i4Max - m_i4Min) + 1;
        }

        [[nodiscard]] tCIDLib::TInt4 i4Max() const
        {
            return m_i4Max;
        }

        [[nodiscard]] tCIDLib::TInt4 i4Min() const
        {
            return m_i4Min;
        }


    protected :
        // -------------------------------------------------------------------
        //  Hidden Constructors
        // -------------------------------------------------------------------
        TCQCFldIntLimit
        (
            const   tCIDLib::TInt4          i4Min
            , const tCIDLib::TInt4          i4Max
        );


        // -------------------------------------------------------------------
        //  Protected, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid SetRange
        (
            const   tCIDLib::TInt4          i4Min
            , const tCIDLib::TInt4          i4Max
        );


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_i4Max
        //  m_i4Min
        //      The values, inclusive, of the range. They will not be allowed
        //      to get inverted, so max is always greater than min.
        // -------------------------------------------------------------------
        tCIDLib::TInt4 m_i4Max;
        tCIDLib::TInt4 m_i4Min;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCFldIntLimit,TCQCFldLimit)
};



// ---------------------------------------------------------------------------
//   CLASS: TCQCFldStrLimit
//  PREFIX: fldl
// ---------------------------------------------------------------------------
class CQCKITEXPORT TCQCFldStrLimit : public TCQCFldLimit
{
    public :
        // -------------------------------------------------------------------
        //  Destructor
        // -------------------------------------------------------------------
        TCQCFldStrLimit();

        TCQCFldStrLimit(const TCQCFldStrLimit&) = default;
        TCQCFldStrLimit(TCQCFldStrLimit&&) = default;

        ~TCQCFldStrLimit();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCFldStrLimit& operator=(const TCQCFldStrLimit&) = default;
        TCQCFldStrLimit& operator=(TCQCFldStrLimit&&) = default;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        [[nodiscard]] tCIDLib::TBoolean bParseLimits
        (
            const   TString&                strLimits
        )   override;

        [[nodiscard]] tCQCKit::EOptFldReps eOptimalRep() const override;

        tCIDLib::TVoid QueryDefVal
        (
                    TString&                strToFill
        )   override;


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        virtual tCQCKit::EValResults eValidate
        (
            const   TString&                strToVal
        )   const;


    private :
        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCFldStrLimit,TCQCFldLimit)
};



// ---------------------------------------------------------------------------
//   CLASS: TCQCFldStrListLimit
//  PREFIX: fldl
// ---------------------------------------------------------------------------
class CQCKITEXPORT TCQCFldStrListLimit : public TCQCFldLimit
{
    public :
        // -------------------------------------------------------------------
        //  Destructor
        // -------------------------------------------------------------------
        TCQCFldStrListLimit();

        TCQCFldStrListLimit(const TCQCFldStrListLimit&) = default;
        TCQCFldStrListLimit(TCQCFldStrListLimit&&) = default;

        ~TCQCFldStrListLimit();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCFldStrListLimit& operator=(const TCQCFldStrListLimit&) = default;
        TCQCFldStrListLimit& operator=(TCQCFldStrListLimit&&) = default;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        [[nodiscard]] tCIDLib::TBoolean bParseLimits
        (
            const   TString&                strLimits
        )   final;

        [[nodiscard]] tCQCKit::EOptFldReps eOptimalRep() const final;

        tCIDLib::TVoid QueryDefVal
        (
                    TString&                strToFill
        )   final;


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        virtual tCQCKit::EValResults eValidate
        (
            const   TString&                strToValidate
            ,       tCIDLib::TStrList&      colToFill
        )   const;

        virtual tCQCKit::EValResults eValidate
        (
            const   tCIDLib::TStrList&      colToVal
        )   const;


    private :
        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCFldStrListLimit,TCQCFldLimit)
};





// ---------------------------------------------------------------------------
//   CLASS: TCQCFldTimeLimit
//  PREFIX: fldl
// ---------------------------------------------------------------------------
class CQCKITEXPORT TCQCFldTimeLimit : public TCQCFldLimit
{
    public :
        // -------------------------------------------------------------------
        //  Constructor and Destructor
        // -------------------------------------------------------------------
        TCQCFldTimeLimit();

        TCQCFldTimeLimit(const TCQCFldTimeLimit&) = default;
        TCQCFldTimeLimit(TCQCFldTimeLimit&&) = default;

        ~TCQCFldTimeLimit();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCFldTimeLimit& operator=(const TCQCFldTimeLimit&) = default;
        TCQCFldTimeLimit& operator=(TCQCFldTimeLimit&&) = default;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        [[nodiscard]] tCIDLib::TBoolean bParseLimits
        (
            const   TString&                strLimits
        )   final;

        [[nodiscard]] tCQCKit::EOptFldReps eOptimalRep() const final;

        tCIDLib::TVoid QueryDefVal
        (
                    TString&                strToFill
        )   final;


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        virtual tCQCKit::EValResults eValidate
        (
            const   TString&                strToValidate
            , COP   tCIDLib::TCard8&        c8ToFill
        )   const;

        virtual tCQCKit::EValResults eValidate
        (
            const   tCIDLib::TCard8&        c8ToVal
        )   const;


    private :
        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCFldTimeLimit,TCQCFldLimit)
};




// ---------------------------------------------------------------------------
//   CLASS: TCQCFldCRangeLimit
//  PREFIX: fldl
// ---------------------------------------------------------------------------
class CQCKITEXPORT TCQCFldCRangeLimit : public TCQCFldCardLimit
{
    public :
        // -------------------------------------------------------------------
        //  Constructor and Destructor
        // -------------------------------------------------------------------
        TCQCFldCRangeLimit();

        TCQCFldCRangeLimit
        (
            const   tCIDLib::TCard4         c4Min
            , const tCIDLib::TCard4         c4Max
        );

        TCQCFldCRangeLimit(const TCQCFldCRangeLimit&) = default;
        TCQCFldCRangeLimit(TCQCFldCRangeLimit&&) = default;

        ~TCQCFldCRangeLimit();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCFldCRangeLimit& operator=(const TCQCFldCRangeLimit&) = default;
        TCQCFldCRangeLimit& operator=(TCQCFldCRangeLimit&&) = default;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        [[nodiscard]] tCIDLib::TBoolean bSameLimits
        (
            const   TCQCFldLimit&           fldlSrc
        )   const   final;

        [[nodiscard]] tCIDLib::TBoolean bParseLimits
        (
            const   TString&                strLimits
        )   final;

        tCIDLib::TVoid FormatToText
        (
                    TString&                strToFill
        )   final;


    private :
        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCFldCRangeLimit,TCQCFldCardLimit)
};



// ---------------------------------------------------------------------------
//   CLASS: TCQCFldFRangeLimit
//  PREFIX: fldl
// ---------------------------------------------------------------------------
class CQCKITEXPORT TCQCFldFRangeLimit : public TCQCFldFloatLimit
{
    public :
        // -------------------------------------------------------------------
        //  Constructor and Destructor
        // -------------------------------------------------------------------
        TCQCFldFRangeLimit();

        TCQCFldFRangeLimit
        (
            const   tCIDLib::TFloat8        f8Min
            , const tCIDLib::TFloat8        f8Max
        );

        TCQCFldFRangeLimit(const TCQCFldFRangeLimit&) = default;
        TCQCFldFRangeLimit(TCQCFldFRangeLimit&&) = default;

        ~TCQCFldFRangeLimit();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCFldFRangeLimit& operator=(const TCQCFldFRangeLimit&) = default;
        TCQCFldFRangeLimit& operator=(TCQCFldFRangeLimit&&) = default;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        [[nodiscard]] tCIDLib::TBoolean bSameLimits
        (
            const   TCQCFldLimit&           fldlSrc
        )   const final;

        [[nodiscard]] tCIDLib::TBoolean bParseLimits
        (
            const   TString&                strLimits
        )   final;

        tCIDLib::TVoid FormatToText
        (
                    TString&                strToFill
        )   final;


    private :
        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCFldFRangeLimit,TCQCFldFloatLimit)
};



// ---------------------------------------------------------------------------
//   CLASS: TCQCFldIRangeLimit
//  PREFIX: fldl
// ---------------------------------------------------------------------------
class CQCKITEXPORT TCQCFldIRangeLimit : public TCQCFldIntLimit
{
    public :
        // -------------------------------------------------------------------
        //  Constructor and Destructor
        // -------------------------------------------------------------------
        TCQCFldIRangeLimit();

        TCQCFldIRangeLimit
        (
            const   tCIDLib::TInt4          i4Min
            , const tCIDLib::TInt4          i4Max
        );

        TCQCFldIRangeLimit(const TCQCFldIRangeLimit&) = delete;
        TCQCFldIRangeLimit(TCQCFldIRangeLimit&&) = delete;

        ~TCQCFldIRangeLimit();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCFldIRangeLimit& operator=(const TCQCFldIRangeLimit&) = default;
        TCQCFldIRangeLimit& operator=(TCQCFldIRangeLimit&&) = default;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        [[nodiscard]] tCIDLib::TBoolean bSameLimits
        (
            const   TCQCFldLimit&           fldlSrc
        )   const final;

        [[nodiscard]] tCIDLib::TBoolean bParseLimits
        (
            const   TString&                strLimits
        )   final;

        tCIDLib::TVoid FormatToText
        (
                    TString&                strToFill
        )   final;


    private :
        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCFldIRangeLimit,TCQCFldIntLimit)
};



// ---------------------------------------------------------------------------
//   CLASS: TCQCFldEnumLimit
//  PREFIX: fldl
// ---------------------------------------------------------------------------
class CQCKITEXPORT TCQCFldEnumLimit : public TCQCFldStrLimit
{
    public :
        // -------------------------------------------------------------------
        //  Class types
        // -------------------------------------------------------------------
        using TEnumList = tCIDLib::TStrList;
        using TEnumCursor = TEnumList::TCursor;


        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static tCIDLib::TBoolean bParseEnumLim
        (
            const   TString&                strLimits
            ,       TCollection<TString>&   colToFill
        );


        // -------------------------------------------------------------------
        //  Constructor and Destructor
        // -------------------------------------------------------------------
        TCQCFldEnumLimit();

        TCQCFldEnumLimit(const TCQCFldEnumLimit&) = default;
        TCQCFldEnumLimit(TCQCFldEnumLimit&&) = default;

        ~TCQCFldEnumLimit();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCFldEnumLimit& operator=(const TCQCFldEnumLimit&) = default;
        TCQCFldEnumLimit& operator=(TCQCFldEnumLimit&&) = default;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bNextPrevVal
        (
            const   TString&                strCurVal
            ,       TString&                strNewVal
            , const tCIDLib::TBoolean       bNext
            , const tCIDLib::TBoolean       bWrapOK
        )   const final;

        [[nodiscard]] tCIDLib::TBoolean bSameLimits
        (
            const   TCQCFldLimit&           fldlSrc
        )   const final;

        [[nodiscard]] tCIDLib::TBoolean bParseLimits
        (
            const   TString&                strLimits
        )   final;

        [[nodiscard]] tCIDLib::TBoolean bValidate
        (
            const   TString&                strToValidate
        )   final;

        [[nodiscard]] tCQCKit::EOptFldReps eOptimalRep() const final;

        tCQCKit::EValResults eValidate
        (
            const   TString&                strToValidate
        )   const final;

        tCIDLib::TVoid QueryDefVal
        (
                    TString&                strToFill
        )   final;

        tCIDLib::TVoid FormatToText
        (
                    TString&                strToFill
        )   final;


        // -------------------------------------------------------------------
        //  Public, non-virtual mmethods
        // -------------------------------------------------------------------
        [[nodiscard]] tCIDLib::TCard4 c4ValCount() const;

        [[nodiscard]] TEnumCursor cursEnum() const;

        tCIDLib::TCard4 c4QueryOrdinal
        (
            const   TString&                strForVal
            , const tCIDLib::TBoolean       bThrowIfNot = kCIDLib::True
        )   const;

        tCIDLib::TCard4 c4QueryValues
        (
                    tCIDLib::TStrList&      colToFill
        )   const   final;

        const TString& strValueAt
        (
            const   tCIDLib::TCard4         c4At
        )   const;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_colList
        //      The allowed values of this enumeration. It's a collection of
        //      string values. Because we need to be able to return them in
        //      the original order and know what the first value is (which
        //      is the default), we just use a vector. The values are always
        //      small and fairly limited in number.
        // -------------------------------------------------------------------
        TEnumList   m_colList;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCFldEnumLimit,TCQCFldStrLimit)
};



// ---------------------------------------------------------------------------
//   CLASS: TCQCFldRegExLimit
//  PREFIX: fldl
// ---------------------------------------------------------------------------
class CQCKITEXPORT TCQCFldRegExLimit : public TCQCFldStrLimit
{
    public :
        // -------------------------------------------------------------------
        //  Constructor and Destructor
        // -------------------------------------------------------------------
        TCQCFldRegExLimit();

        TCQCFldRegExLimit(const TCQCFldRegExLimit&) = default;
        TCQCFldRegExLimit(TCQCFldRegExLimit&&) = default;

        ~TCQCFldRegExLimit();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCFldRegExLimit& operator=(const TCQCFldRegExLimit&) = default;
        TCQCFldRegExLimit& operator=(TCQCFldRegExLimit&&) = default;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        [[nodiscard]] tCIDLib::TBoolean bSameLimits
        (
            const   TCQCFldLimit&           fldlSrc
        )   const final;

        [[nodiscard]] tCIDLib::TBoolean bParseLimits
        (
            const   TString&                strLimits
        )   final;

        [[nodiscard]] tCIDLib::TBoolean bValidate
        (
            const   TString&                strToValidate
        )   final;

        [[nodiscard]] tCQCKit::EOptFldReps eOptimalRep() const final;

        tCQCKit::EValResults eValidate
        (
            const   TString&                strToValidate
        )   const final;

        tCIDLib::TVoid QueryDefVal
        (
                    TString&                strToFill
        )   final;

        tCIDLib::TVoid FormatToText
        (
                    TString&                strToFill
        )   final;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_pregxLimit
        //      The regular expression engine that has been compiled with the
        //      limit expression. We make this a pointer so that we don't
        //      for the reg-ex header on the users of this facility. We just
        //      forward declare it at the top of this header.
        // -------------------------------------------------------------------
        TRegEx* m_pregxLimit;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCFldRegExLimit,TCQCFldStrLimit)
};


// ---------------------------------------------------------------------------
//   CLASS: TCQCFldMediaImgLimit
//  PREFIX: fldl
// ---------------------------------------------------------------------------
class CQCKITEXPORT TCQCFldMediaImgLimit : public TCQCFldStrLimit
{
    public :
        // -------------------------------------------------------------------
        //  Class types
        // -------------------------------------------------------------------
        typedef TVector<TString>            TTypeList;


        // -------------------------------------------------------------------
        //  Constructor and Destructor
        // -------------------------------------------------------------------
        TCQCFldMediaImgLimit();

        TCQCFldMediaImgLimit(const TCQCFldMediaImgLimit&) = default;
        TCQCFldMediaImgLimit(TCQCFldMediaImgLimit&&) = default;

        ~TCQCFldMediaImgLimit();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCFldMediaImgLimit& operator=(const TCQCFldMediaImgLimit&) = default;
        TCQCFldMediaImgLimit& operator=(TCQCFldMediaImgLimit&&) = default;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        [[nodiscard]] tCIDLib::TBoolean bSameLimits
        (
            const   TCQCFldLimit&           fldlSrc
        )   const final;

        [[nodiscard]] tCIDLib::TBoolean bParseLimits
        (
            const   TString&                strLimits
        )   final;

        [[nodiscard]] tCIDLib::TBoolean bValidate
        (
            const   TString&                strToValidate
        )   final;

        tCIDLib::TCard4 c4QueryValues
        (
                    tCIDLib::TStrList&      colToFill
        )   const final;

        [[nodiscard]] tCQCKit::EOptFldReps eOptimalRep() const final;

        tCQCKit::EValResults eValidate
        (
            const   TString&                strToValidate
        )   const final;

        tCIDLib::TVoid QueryDefVal
        (
                    TString&                strToFill
        )   final;

        tCIDLib::TVoid FormatToText
        (
                    TString&                strToFill
        )   final;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        const TTypeList& colTypes() const;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_colTypes
        //      The allowed image types for this field. It will be values
        //      like "JPEG", "PNG, "BMP", to tell the client the possible image
        //      types the field will refer to. The field will be a string
        //      field, and the string will be some reference path to an image
        //      in a media repository.
        // -------------------------------------------------------------------
        TTypeList   m_colTypes;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCFldMediaImgLimit,TCQCFldStrLimit)
};


#pragma CIDLIB_POPPACK


