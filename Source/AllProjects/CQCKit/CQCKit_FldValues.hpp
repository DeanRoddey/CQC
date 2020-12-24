//
// FILE NAME: CQCKit_FldValues.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 05/09/2003
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
//  There are a number of places, outside of the driver itself which has it's
//  own set of classes for this, where a polymorphic collection of field
//  values are maintained, based on driver and field id. So we provide this
//  basic set of classes to use for those types of situations. The base
//  class stores the ids and field type and the derived types just provide
//  the extra storage for the field value.
//
//  ERROR STATE
//
//  We have an error state. For some horribly stupid reason, it ended up defaulted
//  to false. This means that any field that doesn't get any value written to it
//  will end up with a default value that appears valid. Changing that right now
//  would be a horrible thing to deal with, because it would probably break lots
//  of drivers. Sigh...
//
//  So we have a 'first write' flag that can be used to know when the first actual
//  setting of a value happens. The base driver class uses this to know when we are
//  storing the first actual value, which to him is the same has having been in
//  error state, so he'll check both flags. If either is set, then the value being
//  stored is treated as a change.
//
//  Set the error state also clears this first write flag. That lets a driver which
//  has to come on line without all fields ready (like the Z-Wave driver) to just
//  set them all to error state up front, and then any that don't get a value will
//  just nturally be in error state.
//
// CAVEATS/GOTCHAS:
//
//  1)  In our bStoreValue, methods, we let the field limit object directly
//      write to our value. It is not supposed to modify the output parm
//      unless the value is legally storable, so this should be safe, since
//      we won't end up with a non-legal, half-done value or anything and
//      can avoid the copy of the value.
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: TCQCFldValue
//  PREFIX: fv
// ---------------------------------------------------------------------------
class CQCKITEXPORT TCQCFldValue : public TObject
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCFldValue(TCQCFldValue&&) = delete;

        ~TCQCFldValue();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCFldValue& operator=(TCQCFldValue&&) = delete;


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        virtual tCIDLib::TBoolean bPolyValueCopy
        (
            const   TCQCFldValue&           fvSrc
        ) = 0;

        virtual tCQCKit::EValResults eStoreValue
        (
            const   TString&                strValue
            , const TCQCFldLimit&           fldlTest
        ) = 0;

        virtual tCQCKit::EStmtRes eTestStatement
        (
            const   tCQCKit::EStatements    eToTest
            , const TString&                strTestValue
        )   const = 0;

        virtual tCIDLib::TVoid Format
        (
                    TString&                strToFill
        )   const = 0;

        virtual tCIDLib::TVoid StreamInValue
        (
                    TBinInStream&           strmSrc
        ) = 0;

        virtual tCIDLib::TVoid StreamOutValue
        (
                    TBinOutStream&          strmTar
        )   const = 0;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bGotFirstVal() const
        {
            return m_bGotFirstVal;
        }

        tCIDLib::TBoolean bInError() const;

        tCIDLib::TBoolean bInError
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TCard4 c4DriverId() const;

        tCIDLib::TCard4 c4FieldId() const;

        tCIDLib::TCard4 c4IncSerialNum();

        tCIDLib::TCard4 c4SerialNum() const;

        tCIDLib::TCard4 c4SerialNum
        (
            const   tCIDLib::TCard4         c4ToSet
        );

        tCQCKit::EFldTypes eFldType() const;

        tCIDLib::TVoid SetIds
        (
            const   tCIDLib::TCard4         c4NewDriverId
            , const tCIDLib::TCard4         c4NewFldId
        );

        tCIDLib::TVoid UseDefValue();


    protected :
        // -------------------------------------------------------------------
        //  Hidden constructors and operators
        // -------------------------------------------------------------------
        TCQCFldValue
        (
            const   tCQCKit::EFldTypes      eFldType
        );

        TCQCFldValue
        (
            const   tCIDLib::TCard4         c4DriverId
            , const tCIDLib::TCard4         c4FieldId
            , const tCQCKit::EFldTypes      eFldType
        );

        TCQCFldValue
        (
            const   TCQCFldValue&           fvSrc
        );

        tCIDLib::TVoid operator=
        (
            const   TCQCFldValue&           fvSrc
        );


        // -------------------------------------------------------------------
        //  Protected, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid ClearError();


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  bGotFirstVal
        //      This is always initialized to false and is not copied or assigned.
        //      It will be set to true upon the first setting of a value or upon
        //      an explicit error state being set. Once set it stays set unless
        //      the SetIds() method is called to re-use this object for another/
        //      field.
        //
        //      See the header comments above.
        //
        //  m_bInError
        //      Used to store any per-field error indication. Most drivers
        //      don't do any per-field error states, since the driver is either
        //      online or offline. But some devices can have some fields in
        //      error without all of them being. When used inside a field storage
        //      class, it uses this to track its error state.
        //
        //      We will automatically clear it when a legal value is stored. So
        //      the derived classes have to clear it any time they get a good
        //      value.
        //
        //      See the header comments for some messiness, which is why we have
        //      bGotFirstVal to work around this stupidity.
        //
        //  m_c4DriverId
        //  m_c4FieldId
        //      The driver and field ids for this field.
        //
        //  m_c4SerialNum
        //      This is bumped any time the error state is changed or a new value
        //      is stored.
        //
        //  m_eFldType
        //      The field type of the field represented by this class. This
        //      allows for certain checks via the base class without having
        //      to do RTTI. The derived classes set it on behalf of the
        //      outside world which creates them.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bGotFirstVal;
        tCIDLib::TBoolean   m_bInError;
        tCIDLib::TCard4     m_c4DriverId;
        tCIDLib::TCard4     m_c4FieldId;
        tCIDLib::TCard4     m_c4SerialNum;
        tCQCKit::EFldTypes  m_eFldType;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCFldValue, TObject)
};



// ---------------------------------------------------------------------------
//   CLASS: TCQCBoolFldValue
//  PREFIX: fv
// ---------------------------------------------------------------------------
class CQCKITEXPORT TCQCBoolFldValue : public TCQCFldValue
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCBoolFldValue();

        TCQCBoolFldValue
        (
            const   tCIDLib::TCard4         c4DriverId
            , const tCIDLib::TCard4         c4FieldId
        );

        TCQCBoolFldValue
        (
            const   TCQCBoolFldValue&       fvSrc
        );

        TCQCBoolFldValue(TCQCBoolFldValue&&) = delete;

        ~TCQCBoolFldValue();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCBoolFldValue& operator=
        (
            const   TCQCBoolFldValue&       fvSrc
        );

        TCQCBoolFldValue& operator=(TCQCBoolFldValue&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bPolyValueCopy
        (
            const   TCQCFldValue&           fvSrc
        )   final;

        tCQCKit::EValResults eStoreValue
        (
            const   TString&                strValue
            , const TCQCFldLimit&           fldlTest
        )   final;

        tCQCKit::EStmtRes eTestStatement
        (
            const   tCQCKit::EStatements    eToTest
            , const TString&                strTestValue
        )   const final;

        tCIDLib::TVoid Format
        (
                    TString&                strToFill
        )   const final;

        tCIDLib::TVoid StreamInValue
        (
                    TBinInStream&           strmSrc
        )   final;

        tCIDLib::TVoid StreamOutValue
        (
                    TBinOutStream&          strmTar
        )   const final;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bSetValue
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TBoolean bValue() const;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bValue
        //      This is the value storage for this type of field.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean m_bValue;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCBoolFldValue, TCQCFldValue)
};


// ---------------------------------------------------------------------------
//   CLASS: TCQCCardFldValue
//  PREFIX: fv
// ---------------------------------------------------------------------------
class CQCKITEXPORT TCQCCardFldValue : public TCQCFldValue
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCCardFldValue();

        TCQCCardFldValue
        (
            const   tCIDLib::TCard4         c4DriverId
            , const tCIDLib::TCard4         c4FieldId
        );

        TCQCCardFldValue
        (
            const   TCQCCardFldValue&       fvSrc
        );

        TCQCCardFldValue(TCQCCardFldValue&&) = delete;

        ~TCQCCardFldValue();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCCardFldValue& operator=
        (
            const   TCQCCardFldValue&       fvSrc
        );

        TCQCCardFldValue& operator=(TCQCCardFldValue&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bPolyValueCopy
        (
            const   TCQCFldValue&           fvSrc
        )   final;

        tCQCKit::EValResults eStoreValue
        (
            const   TString&                strValue
            , const TCQCFldLimit&           fldlTest
        )   final;

        tCQCKit::EStmtRes eTestStatement
        (
            const   tCQCKit::EStatements    eToTest
            , const TString&                strTestValue
        )   const final;

        tCIDLib::TVoid Format
        (
                    TString&                strToFill
        )   const final;

        tCIDLib::TVoid StreamInValue
        (
                    TBinInStream&           strmSrc
        )   final;

        tCIDLib::TVoid StreamOutValue
        (
                    TBinOutStream&          strmTar
        )   const final;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bSetValue
        (
            const   tCIDLib::TCard4         c4ToSet
        );

        tCIDLib::TCard4 c4Value() const;

        tCIDLib::TVoid Inc();


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4Value
        //      This is the value storage for this type of field.
        // -------------------------------------------------------------------
        tCIDLib::TCard4 m_c4Value;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCCardFldValue, TCQCFldValue)
};



// ---------------------------------------------------------------------------
//   CLASS: TCQCFloatFldValue
//  PREFIX: fv
// ---------------------------------------------------------------------------
class CQCKITEXPORT TCQCFloatFldValue : public TCQCFldValue
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCFloatFldValue();

        TCQCFloatFldValue
        (
            const   tCIDLib::TCard4         c4DriverId
            , const tCIDLib::TCard4         c4FieldId
        );

        TCQCFloatFldValue
        (
            const   TCQCFloatFldValue&      fvSrc
        );

        TCQCFloatFldValue(TCQCFloatFldValue&&) = delete;

        ~TCQCFloatFldValue();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCFloatFldValue& operator=
        (
            const   TCQCFloatFldValue&      fvSrc
        );

        TCQCFloatFldValue& operator=(TCQCFloatFldValue&&) = delete;

        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bPolyValueCopy
        (
            const   TCQCFldValue&           fvSrc
        )   final;

        tCQCKit::EValResults eStoreValue
        (
            const   TString&                strValue
            , const TCQCFldLimit&           fldlTest
        )   final;

        tCQCKit::EStmtRes eTestStatement
        (
            const   tCQCKit::EStatements    eToTest
            , const TString&                strTestValue
        )   const final;

        tCIDLib::TVoid Format
        (
                    TString&                strToFill
        )   const final;

        tCIDLib::TVoid StreamInValue
        (
                    TBinInStream&           strmSrc
        )   final;

        tCIDLib::TVoid StreamOutValue
        (
                    TBinOutStream&          strmTar
        )   const final;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bSetValue
        (
            const   tCIDLib::TFloat8        f8ToSet
        );

        tCIDLib::TFloat8 f8Value() const;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_f8Value
        //      This is the value storage for this type of field.
        // -------------------------------------------------------------------
        tCIDLib::TFloat8 m_f8Value;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCFloatFldValue, TCQCFldValue)
};



// ---------------------------------------------------------------------------
//   CLASS: TCQCIntFldValue
//  PREFIX: fv
// ---------------------------------------------------------------------------
class CQCKITEXPORT TCQCIntFldValue : public TCQCFldValue
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCIntFldValue();

        TCQCIntFldValue
        (
            const   tCIDLib::TCard4         c4DriverId
            , const tCIDLib::TCard4         c4FieldId
        );

        TCQCIntFldValue
        (
            const   TCQCIntFldValue&        fvSrc
        );

        TCQCIntFldValue(TCQCIntFldValue&&) = delete;

        ~TCQCIntFldValue();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCIntFldValue& operator=
        (
            const   TCQCIntFldValue&        fvSrc
        );

        TCQCIntFldValue& operator=(TCQCIntFldValue&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bPolyValueCopy
        (
            const   TCQCFldValue&           fvSrc
        )   final;

        tCQCKit::EValResults eStoreValue
        (
            const   TString&                strValue
            , const TCQCFldLimit&           fldlTest
        )   final;

        tCQCKit::EStmtRes eTestStatement
        (
            const   tCQCKit::EStatements    eToTest
            , const TString&                strTestValue
        )   const final;

        tCIDLib::TVoid Format
        (
                    TString&                strToFill
        )   const final;

        tCIDLib::TVoid StreamInValue
        (
                    TBinInStream&           strmSrc
        )   final;

        tCIDLib::TVoid StreamOutValue
        (
                    TBinOutStream&          strmTar
        )   const final;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bSetValue
        (
            const   tCIDLib::TInt4          i4ToSet
        );

        tCIDLib::TInt4 i4Value() const;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_i4Value
        //      This is the value storage for this type of field.
        // -------------------------------------------------------------------
        tCIDLib::TInt4  m_i4Value;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCIntFldValue, TCQCFldValue)
};



// ---------------------------------------------------------------------------
//   CLASS: TCQCStringFldValue
//  PREFIX: fv
// ---------------------------------------------------------------------------
class CQCKITEXPORT TCQCStringFldValue : public TCQCFldValue
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCStringFldValue();

        TCQCStringFldValue
        (
            const  tCIDLib::TCard4          c4DriverId
            , const tCIDLib::TCard4         c4FieldId
        );

        TCQCStringFldValue
        (
            const   TCQCStringFldValue&     fvSrc
        );

        TCQCStringFldValue(TCQCStringFldValue&&) = delete;

        ~TCQCStringFldValue();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCStringFldValue& operator=
        (
            const   TCQCStringFldValue&     fvSrc
        );

        TCQCStringFldValue& operator=(TCQCStringFldValue&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bPolyValueCopy
        (
            const   TCQCFldValue&           fvSrc
        )   final;

        tCQCKit::EValResults eStoreValue
        (
            const   TString&                strValue
            , const TCQCFldLimit&           fldlTest
        )   final;

        tCQCKit::EStmtRes eTestStatement
        (
            const   tCQCKit::EStatements    eToTest
            , const TString&                strTestValue
        )   const final;

        tCIDLib::TVoid Format
        (
                    TString&                strToFill
        )   const final;

        tCIDLib::TVoid StreamInValue
        (
                    TBinInStream&           strmSrc
        )   final;

        tCIDLib::TVoid StreamOutValue
        (
                    TBinOutStream&          strmTar
        )   const final;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bSetValue
        (
            const   TString&                strToSet
        );

        const TString& strValue() const;

        TString& strValue();


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_strValue
        //      This is the value storage for this type of field.
        // -------------------------------------------------------------------
        TString m_strValue;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCStringFldValue, TCQCFldValue)
};



// ---------------------------------------------------------------------------
//   CLASS: TCQCStrListFldValue
//  PREFIX: fv
// ---------------------------------------------------------------------------
class CQCKITEXPORT TCQCStrListFldValue : public TCQCFldValue
{
    public :
        // -------------------------------------------------------------------
        //  Public class types
        // -------------------------------------------------------------------
        using TValList = tCIDLib::TStrList;


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCStrListFldValue();

        TCQCStrListFldValue
        (
            const tCIDLib::TCard4           c4DriverId
            , const tCIDLib::TCard4         c4FieldId
        );

        TCQCStrListFldValue
        (
            const   TCQCStrListFldValue&    fvSrc
        );

        TCQCStrListFldValue(TCQCStrListFldValue&&) = delete;

        ~TCQCStrListFldValue();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCStrListFldValue& operator=
        (
            const   TCQCStrListFldValue&    fvSrc
        );

        TCQCStrListFldValue& operator=(TCQCStrListFldValue&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bPolyValueCopy
        (
            const   TCQCFldValue&           fvSrc
        )   final;

        tCQCKit::EValResults eStoreValue
        (
            const   TString&                strValue
            , const TCQCFldLimit&           fldlTest
        )   final;

        tCQCKit::EStmtRes eTestStatement
        (
            const   tCQCKit::EStatements    eToTest
            , const TString&                strTestValue
        )   const final;

        tCIDLib::TVoid Format
        (
                    TString&                strToFill
        )   const final;

        tCIDLib::TVoid StreamInValue
        (
                    TBinInStream&           strmSrc
        )   final;

        tCIDLib::TVoid StreamOutValue
        (
                    TBinOutStream&          strmTar
        )   const final;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bClearList();

        tCIDLib::TBoolean bSetValue
        (
            const   TValList&               colToSet
        );

        tCIDLib::TBoolean bSetValue
        (
            const   TString&                strText
            ,       tCIDLib::TCard4&        c4ErrIndex
        );

        const TValList& colValue() const;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_colComp
        //  m_colValue
        //      This is the value storage for this type of field. We also keep
        //      a second one that we use for parsing into when they set our
        //      value via text format, so we have to test parse it first and
        //      compare the new value to the old and so forth. This way we
        //      don't have to create a list every time this is done.
        // -------------------------------------------------------------------
        mutable TValList    m_colComp;
        TValList            m_colValue;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCStrListFldValue, TCQCFldValue)
};


// ---------------------------------------------------------------------------
//   CLASS: TCQCTimeFldValue
//  PREFIX: fv
// ---------------------------------------------------------------------------
class CQCKITEXPORT TCQCTimeFldValue : public TCQCFldValue
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCTimeFldValue();

        TCQCTimeFldValue
        (
            const  tCIDLib::TCard4          c4DriverId
            , const tCIDLib::TCard4         c4FieldId
        );

        TCQCTimeFldValue
        (
            const   TCQCTimeFldValue&       fvSrc
        );

        TCQCTimeFldValue(TCQCTimeFldValue&&) = delete;

        ~TCQCTimeFldValue();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCTimeFldValue& operator=
        (
            const   TCQCTimeFldValue&       fvSrc
        );

        TCQCTimeFldValue& operator=(TCQCTimeFldValue&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bPolyValueCopy
        (
            const   TCQCFldValue&           fvSrc
        )   final;

        tCQCKit::EValResults eStoreValue
        (
            const   TString&                strValue
            , const TCQCFldLimit&           fldlTest
        )   final;

        tCQCKit::EStmtRes eTestStatement
        (
            const   tCQCKit::EStatements    eToTest
            , const TString&                strTestValue
        )   const final;

        tCIDLib::TVoid Format
        (
                    TString&                strToFill
        )   const final;

        tCIDLib::TVoid StreamInValue
        (
                    TBinInStream&           strmSrc
        )   final;

        tCIDLib::TVoid StreamOutValue
        (
                    TBinOutStream&          strmTar
        )   const final;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bSetValue
        (
            const   tCIDLib::TCard8&        c8ToSet
        );

        tCIDLib::TCard8 c8Value() const;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c8Value
        //      This is the value storage for this type of field. We just
        //      store the raw time stamp value, instead of a TTime object,
        //      since we have to expose it as a time stamp anyway, since that
        //      is something that non-CIDLib based code can understand.
        // -------------------------------------------------------------------
        tCIDLib::TCard8 m_c8Value;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCTimeFldValue, TCQCFldValue)
};


#pragma CIDLIB_POPPACK


