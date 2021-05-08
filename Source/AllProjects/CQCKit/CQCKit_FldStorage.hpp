//
// FILE NAME: CQCKit_FldStorage.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/01/2003
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
//  Each driver stores a set of values, which must be of a particular type,
//  and for which changes to the value must be tracked, and which have an
//  id that get's assigned by the base server driver, that is used to
//  find fields by client apps.
//
//  So, in order to minimize code in drivers, we provide a set of storage class,
//  based on an abstract base class so that they can be polymorphically stored by
//  the base server driver class. The server side driver class is called by the
//  derived driver class to register the fields that that driver want's to store.
//  It creates the appropriate storage objects and it provides APIs for the derived
//  class to store values of each support field type, and keeps up with whether the
//  value has changed by bumping a serial number on each change.
//
//  Event Triggers
//
//  We also define here the event trigger class. Though there are standard triggers
//  that get sent out, there are only so many of those. So each field can potentially
//  be defined to send a field change event upon change (or a change that meets some
//  criteria.) This allows triggered events to react to any arbitrary field if
//  that is needed, not just predefined standard triggers.
//
//  Instances of the field trigger class are stored in the driver object config in
//  terms of the static configuration of what fields are configured for what triggers.
//  But at runtime, those triggers are stored in the field storage object associated
//  with the field that trigger is for, since this is were all field values are stored,
//  and hence where changes are seen.
//
//  So the base storage class has storage for an event trigger object to be stored on
//  it if configured. The trigger object's default ctor will set it to be unused until
//  some trigger is set on us.
//
//  Error State
//
//  The errors state is in the field value object. See the field value class header
//  comments for details. It's sort of a mess.
//
//
// CAVEATS/GOTCHAS:
//
//  1)  The latching state of field triggers is NOT copied over by the assignment
//      operator. This allows us to maintain the current latching state if the
//      trigger is modified and stored on us again. Otherwise we'd mess up the
//      last state and possibly cause the next trigger not to be sent.
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: TCQCFldEvTrigger
//  PREFIX: fet
// ---------------------------------------------------------------------------
class CQCKITEXPORT TCQCFldEvTrigger : public TObject, public MStreamable
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCFldEvTrigger();

        TCQCFldEvTrigger(const TCQCFldEvTrigger&) = default;
        TCQCFldEvTrigger(TCQCFldEvTrigger&&) = default;

        TCQCFldEvTrigger
        (
            const   TString&                strFldName
        );

        TCQCFldEvTrigger
        (
            const   TString&                strFldName
            , const TCQCExpression&         exprTrigger
        );

        ~TCQCFldEvTrigger();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCFldEvTrigger& operator=(const TCQCFldEvTrigger&) = default;
        TCQCFldEvTrigger& operator=(TCQCFldEvTrigger&&) = default;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bEvaluate
        (
            const   TCQCFldValue&           fvSrc
        );

        tCQCKit::EEvTrLatches eLatching() const;

        tCQCKit::EEvTrLatches eLatching
        (
            const   tCQCKit::EEvTrLatches   eToSet
        );

        tCQCKit::EEvTrTypes eType() const;

        tCQCKit::EEvTrTypes eType
        (
            const   tCQCKit::EEvTrTypes     eToSet
        );

        TCQCExpression& exprTrigger();

        const TCQCExpression& exprTrigger() const;

        const TCQCExpression& exprTrigger
        (
            const   TCQCExpression&         exprToSet
        );

        const TString& strFldName() const;

        const TString& strFldName
        (
            const   TString&                strToSet
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        )   final;

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const final;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_eLastState
        //      The last latching state we were in, so that we can recognize when
        //      a change of state represents a latching state change.
        //
        //  m_eLatching
        //      If it's expression based, it can be latching. If so, then once it
        //      has triggered, it will not re-trigger until the expression eval
        //      result changes. It can be unidirectional, which means it only
        //      triggers with it transitions from false state to true, or it can
        //      be bidirection, which will trigger on a transition either way.
        //
        //      This used to be a boolean and we only supported unidirectional.
        //
        //  m_eType
        //      Indicates whether this trigger is unused, if it should trigger
        //      on any change, or should trigger based on the expressoin in
        //      m_exprTrigger.
        //
        //  m_exprTrigger
        //      The expression that controls the trigger. It is only used if
        //      m_eOnExpr indicates.
        //
        //  m_strFldName
        //      The name of the field that this trigger is for.
        // -------------------------------------------------------------------
        tCQCKit::EEvTrLatchSts  m_eLastState;
        tCQCKit::EEvTrLatches   m_eLatching;
        tCQCKit::EEvTrTypes     m_eType;
        TCQCExpression          m_exprTrigger;
        TString                 m_strFldName;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCFldEvTrigger,TObject)
};




// ---------------------------------------------------------------------------
//   CLASS: TCQCFldStore
//  PREFIX: cfs
// ---------------------------------------------------------------------------
class CQCKITEXPORT TCQCFldStore : public TObject
{
    public :
        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static const TString& strKey(const TCQCFldStore& cfsSrc);


        // -------------------------------------------------------------------
        //  Destructor
        // -------------------------------------------------------------------
        TCQCFldStore() = delete;

        TCQCFldStore(const TCQCFldStore&) = default;
        TCQCFldStore(TCQCFldStore&&) = default;

        ~TCQCFldStore();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCFldStore& operator=(const TCQCFldStore&) = default;
        TCQCFldStore& operator=(TCQCFldStore&&) = default;


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        virtual tCIDLib::TVoid FormatValue
        (
                    TString&                strToFill
            ,       TTextStringOutStream&   strmFmt
        )   const = 0;

        virtual TCQCFldValue& fvThis() = 0;

        virtual const TCQCFldValue& fvThis() const = 0;

        virtual const TCQCFldLimit* pfldlLimits() const = 0;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bAlwaysWrite() const;

        tCIDLib::TBoolean bAlwaysWrite
        (
            const   tCIDLib::TBoolean       bToSet
        );

        //
        //  Inlined for a fast check by derived classes to see if they need
        //  to deal with event triggering on a field write.
        //
        tCIDLib::TBoolean bHasTrigger() const
        {
            return (m_fetTrigger.eType() != tCQCKit::EEvTrTypes::Unused);
        }

        tCIDLib::TBoolean bInError() const;

        tCIDLib::TBoolean bInError
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TBoolean bSetValue
        (
            const   TCQCFldValue&           fvToSet
        );

        tCIDLib::TBoolean bShouldSendCurVal() const;

        tCIDLib::TCard4 c4SerialNum() const;

        TCQCFldEvTrigger& fetTrigger();

        const TCQCFldEvTrigger& fetTrigger() const;

        const TCQCFldEvTrigger& fetTrigger
        (
            const   TCQCFldEvTrigger&       fetToSet
        );

        const TCQCFldDef& flddInfo() const;

        const TString& strMoniker() const;

        tCIDLib::TVoid SendFldChangeTrig
        (
            const   tCIDLib::TBoolean       bWasInError
        );

        tCIDLib::TVoid StreamIn
        (
                    TBinInStream&           strmSrc
        );

        tCIDLib::TVoid StreamOut
        (
                    TBinOutStream&          strmTarget
        )   const;


    protected :
        // -------------------------------------------------------------------
        //  Hidden Constructors
        // -------------------------------------------------------------------
        TCQCFldStore
        (
            const   TString&                strMoniker
            , const TCQCFldDef&             flddInfo
        );


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_fetTrigger
        //      If any field event trigger is set for this field, this guy
        //      holds the info for it. This is loaded from the driver config
        //      info.
        //
        //  m_flddInfo
        //      The field definition for the field that we are storing the
        //      value for.
        //
        //  m_strMoniker
        //      The moniker of the driver this field belows to. We need this
        //      so that we can send field change events ourselves without
        //      our owning driver getting involved.
        // -------------------------------------------------------------------
        TCQCFldEvTrigger    m_fetTrigger;
        TCQCFldDef          m_flddInfo;
        TString             m_strMoniker;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCFldStore,TObject)
};



// ---------------------------------------------------------------------------
//   CLASS: TCQCFldStoreBool
//  PREFIX: cfs
// ---------------------------------------------------------------------------
class CQCKITEXPORT TCQCFldStoreBool : public TCQCFldStore
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCFldStoreBool() = delete;

        TCQCFldStoreBool
        (
            const   TString&                strMoniker
            , const TCQCFldDef&             flddInfo
            ,       TCQCFldBoolLimit* const pfldlToAdopt

        );

        TCQCFldStoreBool(const TCQCFldStoreBool&) = delete;
        TCQCFldStoreBool(TCQCFldStoreBool&&) = delete;

        ~TCQCFldStoreBool();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCFldStoreBool& operator=(const TCQCFldStoreBool&) = delete;
        TCQCFldStoreBool& operator=
        (
                    TCQCFldStoreBool&&      cfsSrc
        );


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid FormatValue
        (
                    TString&                strToFill
            ,       TTextStringOutStream&   strmFmt
        )   const final;

        TCQCFldValue& fvThis() final
        {
            return m_fvCur;
        }

        const TCQCFldValue& fvThis() const final
        {
            return m_fvCur;
        }

        const TCQCFldLimit* pfldlLimits() const final;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bValue() const;

        tCQCKit::EValResults eValidate
        (
            const   tCIDLib::TBoolean       bToVal
        );

        tCQCKit::EValResults eValidate
        (
            const   TString&                strValue
            ,       tCIDLib::TBoolean&      bToFill
        );

        tCIDLib::TVoid SetValue
        (
            const   tCIDLib::TBoolean       bToSet
        );


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_fvCur
        //      The current value. We use a field value object, since there
        //      are some places where we need to pass the value polymorphically
        //      as a field value. We only use the value member of this object,
        //      not the other stuff.
        //
        //  m_pfldlLimits
        //      Our field limits object. We don't really have any limts, but
        //      for consistency, we use one, because limit objects are used
        //      for other things, such as figuring out the optimal type of
        //      widget to use to display a field's value.
        // -------------------------------------------------------------------
        TCQCBoolFldValue    m_fvCur;
        TCQCFldBoolLimit*   m_pfldlLimits;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCFldStoreBool,TCQCFldStore)
};



// ---------------------------------------------------------------------------
//   CLASS: TCQCFldStoreCard
//  PREFIX: cfs
// ---------------------------------------------------------------------------
class CQCKITEXPORT TCQCFldStoreCard : public TCQCFldStore
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCFldStoreCard() = delete;

        TCQCFldStoreCard
        (
            const   TString&                strMoniker
            , const TCQCFldDef&             flddInfo
            ,       TCQCFldCardLimit* const pfldlToAdopt
        );

        TCQCFldStoreCard(const TCQCFldStoreCard&) = delete;
        TCQCFldStoreCard( TCQCFldStoreCard&&) = delete;

        ~TCQCFldStoreCard();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCFldStoreCard& operator=(const TCQCFldStoreCard&) = delete;
        TCQCFldStoreCard& operator=(TCQCFldStoreCard&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid FormatValue
        (
                    TString&                strToFill
            ,       TTextStringOutStream&   strmFmt
        )   const final;

        TCQCFldValue& fvThis() final
        {
            return m_fvCur;
        }

        const TCQCFldValue& fvThis() const final
        {
            return m_fvCur;
        }

        const TCQCFldLimit* pfldlLimits() const final;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TCard4 c4Value() const;

        tCQCKit::EValResults eValidate
        (
            const   tCIDLib::TCard4         c4ToVal
        );

        tCQCKit::EValResults eValidate
        (
            const   TString&                strValue
            ,       tCIDLib::TCard4&        c4ToFill
        );

        tCIDLib::TVoid Inc();

        tCIDLib::TVoid SetValue
        (
            const   tCIDLib::TCard4         c4ToSet
        );


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_fvCur
        //      The current value. We use a field value object, since there
        //      are some places where we need to pass the value polymorphically
        //      as a field value. We only use the value member of this object,
        //      not the other stuff.
        //
        //  m_pfldlLimits
        //      Our field limits object. We always hvae one, even if there
        //      no specific limits set, in which case we just use a range
        //      object with the full legal range for our type. We look at it
        //      via the base cardinal limit type, since we can only take types
        //      derived from that, and it provides us with a single API we can
        //      call to do string to binary conversion and validation.
        // -------------------------------------------------------------------
        TCQCCardFldValue    m_fvCur;
        TCQCFldCardLimit*   m_pfldlLimits;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCFldStoreCard,TCQCFldStore)
};



// ---------------------------------------------------------------------------
//   CLASS: TCQCFldStoreFloat
//  PREFIX: cfs
// ---------------------------------------------------------------------------
class CQCKITEXPORT TCQCFldStoreFloat : public TCQCFldStore
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCFldStoreFloat() = delete;

        TCQCFldStoreFloat(const TCQCFldStoreFloat&) = delete;
        TCQCFldStoreFloat(TCQCFldStoreFloat&&) = delete;

        TCQCFldStoreFloat
        (
            const   TString&                strMoniker
            , const TCQCFldDef&             flddInfo
            ,       TCQCFldFloatLimit* const pfldlToAdopt
        );

        ~TCQCFldStoreFloat();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCFldStoreFloat& operator=(const TCQCFldStoreFloat&) = delete;
        TCQCFldStoreFloat& operator=(TCQCFldStoreFloat&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid FormatValue
        (
                    TString&                strToFill
            ,       TTextStringOutStream&   strmFmt
        )   const final;

        TCQCFldValue& fvThis() final
        {
            return m_fvCur;
        }

        const TCQCFldValue& fvThis() const final
        {
            return m_fvCur;
        }

        const TCQCFldLimit* pfldlLimits() const final;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCQCKit::EValResults eValidate
        (
            const   tCIDLib::TFloat8        f8ToVal
        );

        tCQCKit::EValResults eValidate
        (
            const   TString&                strValue
            ,       tCIDLib::TFloat8&       f8ToFill
        );

        tCIDLib::TFloat8 f8Value() const;

        tCIDLib::TVoid SetValue
        (
            const   tCIDLib::TFloat8        f8ToSet
        );


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_fvCur
        //      The current value. We use a field value object, since there
        //      are some places where we need to pass the value polymorphically
        //      as a field value. We only use the value member of this object,
        //      not the other stuff.
        //
        //  m_pfldlLimits
        //      Our field limits object. We always hvae one, even if there
        //      no specific limits set, in which case we just use a range
        //      object with the full legal range for our type. We look at it
        //      via the base float limit type, since we can only take types
        //      derived from that, and it provides us with a single API we can
        //      call to do string to binary conversion and validation.
        // -------------------------------------------------------------------
        TCQCFloatFldValue   m_fvCur;
        TCQCFldFloatLimit*  m_pfldlLimits;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCFldStoreFloat,TCQCFldStore)
};



// ---------------------------------------------------------------------------
//   CLASS: TCQCFldStoreInt
//  PREFIX: cfs
// ---------------------------------------------------------------------------
class CQCKITEXPORT TCQCFldStoreInt : public TCQCFldStore
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCFldStoreInt() = delete;

        TCQCFldStoreInt
        (
            const   TString&                strMoniker
            , const TCQCFldDef&             flddInfo
            ,       TCQCFldIntLimit* const  pfldlToAdopt
        );

        TCQCFldStoreInt(const TCQCFldStoreInt&) = delete;
        TCQCFldStoreInt(TCQCFldStoreInt&&) = delete;

        ~TCQCFldStoreInt();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCFldStoreInt& operator=(const TCQCFldStoreInt&) = delete;
        TCQCFldStoreInt& operator=(TCQCFldStoreInt&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid FormatValue
        (
                    TString&                strToFill
            ,       TTextStringOutStream&   strmFmt
        )   const final;

        TCQCFldValue& fvThis() final
        {
            return m_fvCur;
        }

        const TCQCFldValue& fvThis() const final
        {
            return m_fvCur;
        }

        const TCQCFldLimit* pfldlLimits() const final;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCQCKit::EValResults eValidate
        (
            const   tCIDLib::TInt4          bToVal
        );

        tCQCKit::EValResults eValidate
        (
            const   TString&                strValue
            ,       tCIDLib::TInt4&         i4ToFill
        );

        tCIDLib::TInt4 i4Value() const;

        tCIDLib::TVoid SetValue
        (
            const   tCIDLib::TInt4          i4ToSet
        );


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_fvCur
        //      The current value. We use a field value object, since there
        //      are some places where we need to pass the value polymorphically
        //      as a field value. We only use the value member of this object,
        //      not the other stuff.
        //
        //  m_pfldlLimits
        //      Our field limits object. We always hvae one, even if there
        //      no specific limits set, in which case we just use a range
        //      object with the full legal range for our type. We look at it
        //      via the base integral limit type, since we can only take types
        //      derived from that, and it provides us with a single API we can
        //      call to do string to binary conversion and validation.
        // -------------------------------------------------------------------
        TCQCIntFldValue     m_fvCur;
        TCQCFldIntLimit*    m_pfldlLimits;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCFldStoreInt,TCQCFldStore)
};



// ---------------------------------------------------------------------------
//   CLASS: TCQCFldStoreString
//  PREFIX: cfs
// ---------------------------------------------------------------------------
class CQCKITEXPORT TCQCFldStoreString : public TCQCFldStore
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCFldStoreString() = delete;

        TCQCFldStoreString
        (
            const   TString&                strMoniker
            , const TCQCFldDef&             flddInfo
            ,       TCQCFldStrLimit* const  pfldlToAdopt

        );

        TCQCFldStoreString(const TCQCFldStoreString&) = delete;
        TCQCFldStoreString(TCQCFldStoreString&&) = delete;

        ~TCQCFldStoreString();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCFldStoreString& operator=(const TCQCFldStoreString&) = delete;
        TCQCFldStoreString& operator=(TCQCFldStoreString&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid FormatValue
        (
                    TString&                strToFill
            ,       TTextStringOutStream&   strmFmt
        )   const final;

        TCQCFldValue& fvThis() final
        {
            return m_fvCur;
        }

        const TCQCFldValue& fvThis() const final
        {
            return m_fvCur;
        }

        const TCQCFldLimit* pfldlLimits() const final;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCQCKit::EValResults eValidate
        (
            const   TString&                strToVal
        );

        const TString& strValue() const;

        tCIDLib::TVoid SetValue
        (
            const   TString&                strToSet
        );


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_pfldlLimits
        //      A pointer to our limits object, if any. This class is ok with
        //      having it be null. We can only accept limit objects derived
        //      from the string field limit class, i.e. ones that deal with
        //      string values.
        //
        //  m_fvCur
        //      The current value. We use a field value object, since there
        //      are some places where we need to pass the value polymorphically
        //      as a field value. We only use the value member of this object,
        //      not the other stuff.
        // -------------------------------------------------------------------
        TCQCStringFldValue  m_fvCur;
        TCQCFldStrLimit*    m_pfldlLimits;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCFldStoreString,TCQCFldStore)
};



// ---------------------------------------------------------------------------
//   CLASS: TCQCFldStoreSList
//  PREFIX: cfs
// ---------------------------------------------------------------------------
class CQCKITEXPORT TCQCFldStoreSList : public TCQCFldStore
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCFldStoreSList() = delete;

        TCQCFldStoreSList
        (
            const   TString&                strMoniker
            , const TCQCFldDef&             flddInfo
            ,       TCQCFldStrListLimit* const pfldlToAdopt
        );

        TCQCFldStoreSList(const TCQCFldStoreSList&) = delete;
        TCQCFldStoreSList(TCQCFldStoreSList&&) = delete;

        ~TCQCFldStoreSList();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCFldStoreSList& operator=(const TCQCFldStoreSList&) = delete;
        TCQCFldStoreSList& operator=(TCQCFldStoreSList&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid FormatValue
        (
                    TString&                strToFill
            ,       TTextStringOutStream&   strmFmt
        )   const final;

        TCQCFldValue& fvThis() final
        {
            return m_fvCur;
        }

        const TCQCFldValue& fvThis() const final
        {
            return m_fvCur;
        }

        const TCQCFldLimit* pfldlLimits() const final;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bClearList();

        const tCIDLib::TStrList& colValue() const;


        tCQCKit::EValResults eValidate
        (
            const   tCIDLib::TStrList&      colToVal
        );

        tCQCKit::EValResults eValidate
        (
            const   TString&                strValue
            ,       tCIDLib::TStrList&      colToFill
        );

        tCIDLib::TVoid SetValue
        (
            const   tCIDLib::TStrList&      colToSet
        );


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCompare
        (
            const   tCIDLib::TStrList&      colToComp
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_fvCur
        //      The current value. We use a field value object, since there
        //      are some places where we need to pass the value polymorphically
        //      as a field value. We only use the value member of this object,
        //      not the other stuff.
        //
        //  m_pfldlLimits
        //      A pointer to our limits object, if any. This class is ok with
        //      having it be null.
        // -------------------------------------------------------------------
        TCQCStrListFldValue     m_fvCur;
        TCQCFldStrListLimit*    m_pfldlLimits;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCFldStoreSList,TCQCFldStore)
};



// ---------------------------------------------------------------------------
//   CLASS: TCQCFldStoreTime
//  PREFIX: cfs
// ---------------------------------------------------------------------------
class CQCKITEXPORT TCQCFldStoreTime : public TCQCFldStore
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCFldStoreTime() = delete;

        TCQCFldStoreTime
        (
            const   TString&                strMoniker
            , const TCQCFldDef&             flddInfo
            ,       TCQCFldTimeLimit* const pfldlToAdopt
        );

        TCQCFldStoreTime(const TCQCFldStoreTime&) = delete;
        TCQCFldStoreTime(TCQCFldStoreTime&&) = delete;

        ~TCQCFldStoreTime();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCFldStoreTime& operator=(const TCQCFldStoreTime&) = delete;
        TCQCFldStoreTime& operator=(TCQCFldStoreTime&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid FormatValue
        (
                    TString&                strToFill
            ,       TTextStringOutStream&   strmFmt
        )   const final;

        TCQCFldValue& fvThis() final
        {
            return m_fvCur;
        }

        const TCQCFldValue& fvThis() const final
        {
            return m_fvCur;
        }

        const TCQCFldLimit* pfldlLimits() const final;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TCard8 c8Value() const;

        tCQCKit::EValResults eValidate
        (
            const   tCIDLib::TCard8&        c8ToVal
        );

        tCQCKit::EValResults eValidate
        (
            const   TString&                strValue
            ,       tCIDLib::TCard8&        c8ToFill
        );

        tCIDLib::TVoid SetToNow();

        tCIDLib::TVoid SetValue
        (
            const   tCIDLib::TCard8&        c8ToSet
        );


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_fvCur
        //      The current value. We use a field value object, since there
        //      are some places where we need to pass the value polymorphically
        //      as a field value. We only use the value member of this object,
        //      not the other stuff.
        //
        //  m_pfldlLimits
        //      A pointer to our limits object, if any. This class is ok with
        //      having it be null, since any possible value is potentially
        //      a legal time stamp.
        // -------------------------------------------------------------------
        TCQCTimeFldValue    m_fvCur;
        TCQCFldTimeLimit*   m_pfldlLimits;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCFldStoreTime,TCQCFldStore)
};

#pragma CIDLIB_POPPACK


