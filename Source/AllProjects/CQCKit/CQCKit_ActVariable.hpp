//
// FILE NAME: CQCKit_ActVariable.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/10/2008
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
//  This file defines an action variable. Each variable used by the action
//  system is similar to a field, in that it has a name, a data type, and
//  potentially limits set on it's value within that data type.
//
//  It also has a runtime value as well. We reuse some of the field oriented
//  functionality to implement variables since they are so similar and there
//  would be a lot of grunt work otherwise.
//
//  The main functionality we provide is support for doing math and boolean
//  logic ops, each of which must deal with type conversion of the source
//  to the target type. That takes a huge amount of grunt work off or the
//  users of this class (mainly the variable action target.)
//
//  By making them so similar to fields, we simplify some things a lot, such
//  as supporting variable based widgets in the interface system.
//
//  These don't have to be streamable. Variables are runtime things only.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

class TStdVarsTar;


// ---------------------------------------------------------------------------
//   CLASS: TCQCActVar
//  PREFIX: var
// ---------------------------------------------------------------------------
class CQCKITEXPORT TCQCActVar : public TObject
{
    public :
        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static tCIDLib::ESortComps eComp
        (
            const   TCQCActVar&             var1
            , const TCQCActVar&             Var2
        );

        static tCIDLib::ESortComps eIComp
        (
            const   TCQCActVar&             var1
            , const TCQCActVar&             var2
        );

        static const TString& strKey(const TCQCActVar& varSrc);


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCActVar() = delete;

        TCQCActVar
        (
            const   TString&                strName
            , const tCQCKit::EFldTypes      eType
            , const TString&                strLimits
            , const TString&                strInitValue
            , const tCIDLib::TCard4         c4VarId
        );

        TCQCActVar
        (
            const   TString&                strName
            , const tCQCKit::EFldTypes      eType
            , const tCIDLib::TCh* const     pszLimits
            , const TString&                strInitValue
            , const tCIDLib::TCard4         c4VarId
        );

        TCQCActVar
        (
            const   TString&                strName
            , const tCQCKit::EFldTypes      eType
            , const TString&                strInitValue
            , const tCIDLib::TCard4         c4VarId
        );

        TCQCActVar(const TCQCActVar&) = delete;
        TCQCActVar(TCQCActVar&&) = delete;

        ~TCQCActVar();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        tCIDLib::TBoolean operator==
        (
            const   TCQCActVar&             varToComp
        )   const;

        tCIDLib::TBoolean operator!=
        (
            const   TCQCActVar&             varToComp
        )   const;

        TCQCActVar& operator=(const TCQCActVar&) = delete;
        TCQCActVar& operator=(TCQCActVar&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bAdd
        (
            const   TString&                strToAdd
        );

        tCIDLib::TBoolean bAppendValue
        (
            const   TString&                strToAppend
        );

        tCIDLib::TBoolean bBooleanOp
        (
            const   TString&                strToAnd
            , const tCQCKit::ELogOps        eLogOp
        );

        tCIDLib::TBoolean bCapAt
        (
            const   tCIDLib::TCard4         c4Index
        );

        tCIDLib::TBoolean bDelSubStr
        (
            const   tCIDLib::TCard4         c4Start
            , const tCIDLib::TCard4         c4Count
        );

        tCIDLib::TBoolean bDivide
        (
            const   TString&                strDivisor
        );

        tCIDLib::TBoolean bInsertValue
        (
            const   TString&                strToInsert
            , const tCIDLib::TCard4         c4At
        );

        tCIDLib::TBoolean bIsNumericType() const;

        tCIDLib::TBoolean bMultiply
        (
            const   TString&                strMulBy
        );

        tCIDLib::TBoolean bNegate();

        tCIDLib::TBoolean bPermanent() const;

        tCIDLib::TBoolean bPermanent
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TBoolean bQueryEnumVals
        (
                    tCIDLib::TStrList&      colToFill
        )   const;

        tCIDLib::TBoolean bReadOnly() const;

        tCIDLib::TBoolean bReadOnly
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TBoolean bReplaceChars
        (
            const   tCIDLib::TCh            chReplace
            , const tCIDLib::TCh            chReplaceWith
        );

        tCIDLib::TBoolean bReplaceSubStr
        (
            const   TString&                strReplace
            , const TString&                strReplaceWith
            , const tCIDLib::TBoolean       bCaseSensitive
        );

        tCIDLib::TBoolean bReplaceToken
        (
            const   tCIDLib::TCh            chToken
            , const TString&                strReplaceWith
        );

        tCIDLib::TBoolean bSetCharAt
        (
            const   tCIDLib::TCh            chToSet
            , const tCIDLib::TCard4         c4At
        );

        tCIDLib::TBoolean bSetTime
        (
            const   tCIDLib::TCard8         c8ToSet
        );

        tCIDLib::TBoolean bSetValue
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TBoolean bSetValue
        (
            const   tCIDLib::TCard4         c4ToSet
        );

        tCIDLib::TBoolean bSetValue
        (
            const   tCIDLib::TCard8         c8ToSet
        );

        tCIDLib::TBoolean bSetValue
        (
            const   tCIDLib::TFloat8        f8ToSet
        );

        tCIDLib::TBoolean bSetValue
        (
            const   tCIDLib::TInt4          i4ToSet
        );

        tCIDLib::TBoolean bSetValue
        (
            const   tCIDLib::TInt8          i8ToSet
        );

        tCIDLib::TBoolean bSetValue
        (
            const   tCIDLib::TCh* const     pszToSet
        );

        tCIDLib::TBoolean bSetValue
        (
            const   TString&                strToSet
        );

        tCIDLib::TBoolean bSetValueFrom
        (
            const   TCQCActVar&             varSrc
        );

        tCIDLib::TBoolean bStrip
        (
            const   TString&                strStripChars
            , const tCIDLib::EStripModes    eMode
        );

        tCIDLib::TBoolean bSubtract
        (
            const   TString&                strValue
        );

        tCIDLib::TBoolean bUpLow
        (
            const   tCIDLib::TBoolean       bUpper
        );

        tCIDLib::TCard4 c4SerialNum() const;

        tCIDLib::TCard4 c4VarId() const;

        tCQCKit::EFldTypes eType() const;

        const TCQCFldLimit& fldlVar() const;

        const TCQCFldValue& fvCurrent() const;

        tCIDLib::TVoid QueryNthEnumVal
        (
            const   tCIDLib::TCard4         c4Index
            ,       TString&                strToFill
        )   const;

        tCIDLib::TVoid QueryNumericLimits
        (
                    TString&                strMinVal
            ,       TString&                strMaxVal
        )   const;

        const TString& strLimits() const;

        const TString& strName() const;

        const TString& strValue() const;

        tCIDLib::TVoid SetFormat
        (
            const   tCIDLib::ERadices       eRadix
            , const tCIDLib::TCard4         c4Digits
        );


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCvtNumType
        (
            const   tCIDLib::TCh* const     pszOp
            , const tCIDLib::TCard4         c4Line
            , const TString&                strValue
            , const tCIDLib::TBoolean       bStrUnsigned = kCIDLib::False
        );

        tCIDLib::TBoolean bTestLimits
        (
            const   tCQCKit::EValResults    eRes
            , const tCIDLib::TCh* const     pszOpName
        );

        tCIDLib::TVoid ThrowBadOp
        (
            const   tCIDLib::TCh* const     pszOpName
            , const tCIDLib::TCard4         c4Line
        );

        tCIDLib::TVoid ThrowBadParm
        (
            const   tCIDLib::TCh* const     pszOpName
            , const tCIDLib::TCard4         c4Line
        );

        tCIDLib::TVoid ThrowBadTarVal
        (
            const   tCIDLib::TCh* const     pszOpName
            , const tCIDLib::TCard4         c4Line
        );

        tCIDLib::TVoid ThrowLimits
        (
            const   tCIDLib::TCh* const     pszOpName
            , const tCIDLib::TCard4         c4Line
        );

        tCIDLib::TVoid ThrowNotNum
        (
            const   tCIDLib::TCh* const     pszOpName
            , const tCIDLib::TCard4         c4Line
        );

        tCIDLib::TVoid ThrowReadOnly
        (
            const   tCIDLib::TCh* const     pszOpName
            , const tCIDLib::TCard4         c4Line
        );

        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bPermanent
        //      Variables can be marked as permanent, so that the client
        //      code can decline to allow them to be removed if that is
        //      desirable.
        //
        //  m_bReadOnly
        //      Variables can be marked as read only, which will cause an
        //      exception to be thrown if any attempt is made to modify them.
        //
        //  m_c4Digits
        //      Used for formatting the value to text. It is only used for
        //      floating point types.
        //
        //  m_c4FmtSerialNum
        //  m_c4SerialNum
        //      We keep a serial number and bump it up every time we change
        //      the value. We also store the serial number last time we dia
        //      a format to the m_strFmt member. This lets us know if we need
        //      format it again when asked for the formatted value.
        //
        //      We use the format string for temp purposes often, because
        //      the thing being formatted is about to be used to chagne the
        //      variable anyway. So we just zero ut the format serial number
        //      and use the format string as a temp. If the change succeeds
        //      we'd have to reformat anyway.
        //
        //  m_c4VarId
        //      Variables can be deleted and re-created. To make it possible
        //      to catch that this has happened (which might change the data
        //      type or limits that some client code has previously seen and
        //      adapted to), each variable is assigned the next number from
        //      a running counter. Client code can cache the id and check
        //      it later to watch for this kind of thing.
        //
        //  m_c4SrcVal
        //  m_c8SrcVal
        //      The eFindNumType() method will fill one of these in with
        //      the incoming value that has been evaluated and converted
        //      to our type, if our type is Card or Time.
        //
        //  m_eRadix
        //      Used for formatting the value to text. Used only for int and
        //      card values.
        //
        //  m_eType
        //      The data type for this variable.
        //
        //  m_f8SrcVal
        //      The eFindNumType() method will fill one of these in with
        //      the incoming value that has been evaluated and converted
        //      to our type, if our type is Float.
        //
        //  m_i4SrcVal
        //  m_i8SrcVal
        //      The eFindNumType() method will fill one of these in with
        //      the incoming value that has been evaluated and converted
        //      to our type, if our type is Int. i8 currently only ever
        //      happens when a string is converted to signed, since we don't
        //      have a 64 bit int value normally.
        //
        //  m_pfldlTest
        //      The formatted limits are parsed into this actual limit object
        //      which is used for the legal value testing (and conversion from
        //      text format to the actual binary format of the variable.)
        //
        //  m_pfvCurrent
        //      The current value of the variable, which is allocated as
        //      based on the data type.
        //
        //  m_strFmt
        //      We have to provide the formatted value of our variable for
        //      various display and logging reasons. So we just format it
        //      into here when asked for, and return a ref to that. Variales
        //      aren't used in high performance situations, so this isn't
        //      to bad a deal. We know if we need to reformat because we
        //      store the serial number each time we format it out.
        //
        //  m_strLimits
        //      The formatted version of the limits set on this variable.
        //
        //  m_strName
        //      The name of the varaible.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bPermanent;
        tCIDLib::TBoolean       m_bReadOnly;
        tCIDLib::TCard4         m_c4Digits;
        mutable tCIDLib::TCard4 m_c4FmtSerialNum;
        tCIDLib::TCard4         m_c4SerialNum;
        tCIDLib::TCard4         m_c4SrcVal;
        tCIDLib::TCard4         m_c4VarId;
        tCIDLib::TCard8         m_c8SrcVal;
        tCIDLib::ERadices       m_eRadix;
        tCQCKit::EFldTypes      m_eType;
        tCIDLib::TFloat8        m_f8SrcVal;
        tCIDLib::TInt4          m_i4SrcVal;
        tCIDLib::TInt8          m_i8SrcVal;
        TCQCFldLimit*           m_pfldlTest;
        TCQCFldValue*           m_pfvCurrent;
        TString                 m_strLimits;
        mutable TString         m_strFmt;
        TString                 m_strName;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCActVar,TObject)
};

#pragma CIDLIB_POPPACK



