//
// FILE NAME: CQCKit_TrgEvent.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/06/2005
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
//  This is the header for the CQCKit_TrgEvent.cpp file, which implements
//  a simple derivative of the standard command source class, i.e. it's an
//  'action'. The base class provides all the core action stuff. We just add
//  to it a set of event filter expressions that will be used to decide at
//  runtime whether this event should be triggered.
//
//  The filter is up to 4 configured items that mostly refer to values in the
//  incoming event, plus some more generic stuff like is night time. We provide
//  a fixed set of these types of filter criteria that the user can configure.
//  They won't always provide complete filtering, and sometimes the action will
//  have to do more examination and might decide to do nothing with it. But
//  the optimum scenario is to have as many false triggers as possible, for
//  efficiency.
//
//  So we also define a triggered event filter class that is used to hold those
//  configured criteria. The triggered event just has an array of them, and
//  marks those that are configured as in use so that the event server knows to
//  check them.
//
//  We define a runtime value class that provides access to some standard stuff from
//  the incoming event. And we provide a wrapper class that the incoming event can be
//  put into, along with the actual defined triggered event and passed in as the cmd
//  source, so that this correct runtime value will be gen'd up when the action
//  engine asks for it.
//
//  We define a simple derivative that includes the path it was loaded from, which is
//  often useful, such as by the event server, when a list of events keyed by the path
//  is needed.
//
//  As of 5.3.928 we support a 'serialization' option. You can mark a triggered event
//  such that only one instance of it will be run at a time. Others are grabbed by
//  event server threads as soon as they are available and run, in parallel if two
//  of the same time come in at the same time. In some cases this is not desirable and
//  you want some events to serialize instances, deferring them until the previous one
//  is done.
//
// CAVEATS/GOTCHAS:
//
//  1)  We do not have to override pcrtsMakeNew() in the triggered event class, in
//      in order to gen up one of our triggered even oriented RTV classes specific
//      to the triggered event source. We have a simple Ex derivative of the triggered
//      event which can be used by folks who need that, such as the event server or
//      the IV. That gives them the event info plus the incoming trigger info in one
//      object that they can queue up for processing, and that also provides the info
//      required to create the runtime values object when they process it.
//
//  2)  As of 5.0, we moved them to the standard hierarchical storage. They used to have
//      a unique id that was the actual id, plus a human readable name, and actions
//      referred to them via the id. Now the path is the unique id. During the upgrade
//      process the old id will be read in and the events will be moved to their new
//      location under the /User section of the triggered events scope, with the id
//      as the name.
//
//      Actions that reference the ids will be updated so put that path prefix before
//      the id, making them now correctly refer to them in the new scheme. The user
//      can rename them after the ugprade if they want to give them more meaningful
//      names.
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//  CLASS: TCQCTEvFilter
// PREFIX: cevf
// ---------------------------------------------------------------------------
class CQCKITEXPORT TCQCTEvFilter : public TObject, public MStreamable
{
    public  :
        // -------------------------------------------------------------------
        //  Public, static types
        // -------------------------------------------------------------------
        enum class EFldTypes
        {
            Alt
            , Device
            , Field
            , Unused
        };


        // -------------------------------------------------------------------
        //  Public static methods
        // -------------------------------------------------------------------
        static tCIDLib::TVoid QueryFilterTypeInfo
        (
            const   tCQCKit::ETEvFilters    eEvType
            ,       tCIDLib::TBoolean&      bFldUsed
            ,       tCIDLib::TBoolean&      bCompUsed
            ,       tCIDLib::TBoolean&      bFldReq
            ,       tCIDLib::TBoolean&      bCompReq
            ,       tCIDLib::TBoolean&      bFldRegEx
            ,       tCIDLib::TBoolean&      bCompRegEx
        );

        static tCIDLib::TVoid QueryFilterValPrefs
        (
            const   tCQCKit::ETEvFilters    eEvType
            ,       TString&                strFldPref
            ,       EFldTypes&              eFldType
            ,       TString&                strCompPref
        );


        // -------------------------------------------------------------------
        // Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCTEvFilter();

        TCQCTEvFilter
        (
            const   TCQCTEvFilter&          cevfSrc
        );

        ~TCQCTEvFilter();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCTEvFilter& operator=
        (
            const   TCQCTEvFilter&          cevfSrc
        );

        tCIDLib::TBoolean operator==
        (
            const   TCQCTEvFilter&          cevfSrc
        )   const;

        tCIDLib::TBoolean operator!=
        (
            const   TCQCTEvFilter&          cevfSrc
        )   const;


        // -------------------------------------------------------------------
        //  Public non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCheckValidRegX
        (
            const   tCQCKit::ETEvFilters    eType
            , const tCIDLib::TBoolean       bCompValRegEx
            , const tCIDLib::TBoolean       bFldRegEx
        )   const;

        tCIDLib::TBoolean bCompValRegEx() const;

        tCIDLib::TBoolean bEvaluate
        (
            const   TCQCEvent&              cevToTest
            , const tCIDLib::TBoolean       bIsNight
            , const tCIDLib::TEncodedTime   enctNow
            , const tCIDLib::TCard4         c4Hour
            , const tCIDLib::TCard4         c4Minute
        )   const;

        tCIDLib::TBoolean bFldRegEx() const;

        tCIDLib::TBoolean bNegate() const;

        tCQCKit::ETEvFilters eType() const;

        const TString& strCompVal() const;

        const TString& strEvFld() const;

        tCIDLib::TVoid Reset();

        tCIDLib::TVoid SearchNReplace
        (
            const   TString&                strFind
            , const TString&                strReplaceWith
            , const tCIDLib::TBoolean       bRegEx
            , const tCIDLib::TBoolean       bFullMatch
            ,       TRegEx&                 regxFind
        );

        tCIDLib::TVoid Set
        (
            const   tCIDLib::TBoolean       bNegate
            , const tCQCKit::ETEvFilters    eType
            , const TString&                strEvFld
            , const TString&                strCompVal
            , const tCIDLib::TBoolean       bCompValRegEx
            , const tCIDLib::TBoolean       bFldRegEx
        );

        tCIDLib::TVoid ToXML
        (
                    TTextOutStream&         strmTarget
        )   const;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        )   override;

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const override;

    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bDoComp
        (
            const   tCIDLib::TBoolean       bCompVal
            , const TString&                strTestVal
            , const tCIDLib::TBoolean       bEmptyMatch = kCIDLib::False
        )   const;

        tCIDLib::TBoolean bDoMonikerComp
        (
            const   tCIDLib::TBoolean       bCompVal
            , const TString&                strTestVal
        )   const;

        tCIDLib::TVoid DoSetup();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bCompValRegEx
        //  m_bFldRegEx
        //      Indicates whether or not the value in the comp value or field
        //      name is a regular expression or just a literal value.
        //
        //  m_bNegate
        //      The user can negate the result, so it's Not whatever.
        //
        //  m_eType
        //      The filter type, which tells us how to evaluate this filter.
        //
        //  m_pregxComp
        //  m_pregxFld
        //      If either of the comp value or field names is a regular
        //      expression, these are created and set up with the expression.
        //
        //  m_strEvFld
        //  m_strCompVal
        //      Some filter types need no more info than the fields above,
        //      but some need one or two values entered by the user.
        //
        //  m_strEvSrc
        //      For covenience we pull out the event source if there is one,
        //      and their should be generally, and store it here during the
        //      processing of the filter. This prevents a lot of redundancy.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bCompValRegEx;
        tCIDLib::TBoolean       m_bFldRegEx;
        tCIDLib::TBoolean       m_bNegate;
        tCQCKit::ETEvFilters    m_eType;
        TRegEx*                 m_pregxComp;
        TRegEx*                 m_pregxFld;
        TString                 m_strCompVal;
        TString                 m_strEvFld;
        mutable TString         m_strEvSrc;
        mutable TString         m_strTmp;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCTEvFilter,TObject)
};



// ---------------------------------------------------------------------------
//   CLASS: TEventRTVs
//  PREFIX: crts
// ---------------------------------------------------------------------------
class CQCKITEXPORT TEventRTVs : public TCQCCmdRTVSrc
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TEventRTVs
        (
            const   TCQCUserCtx&            cuctxToUse
        );

        TEventRTVs
        (
            const   TCQCUserCtx&            cuctxToUse
            , const tCQCKit::TCQCEvPtr&     cptrData
        );

        TEventRTVs(const TEventRTVs&) = delete;

        ~TEventRTVs();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TEventRTVs& operator=(const TEventRTVs&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bRTValue
        (
            const   TString&                strId
            ,       TString&                strToFill
        )   const final;


    private :
        // -------------------------------------------------------------------
        //  Private, data members
        //
        //  m_cptrData
        //      We are passed a counted pointer to the incoming event. This
        //      way we can share it along with any other users. This is a
        //      counted pointer to const, and the other users of it get the
        //      same so no one is changing it.
        //
        //  m_strTmpX
        //      Some temp strings used to maniuplate the event info.
        // -------------------------------------------------------------------
        tCQCKit::TCQCEvPtr  m_cptrData;
        mutable TString     m_strTmp1;
        mutable TString     m_strTmp2;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TEventRTVs,TCQCCmdRTVSrc)
};



// ---------------------------------------------------------------------------
//  CLASS: TCQCTrgEvent
// PREFIX: csrc
// ---------------------------------------------------------------------------
class CQCKITEXPORT TCQCTrgEvent : public TCQCStdCmdSrc
{
    public  :
        // -------------------------------------------------------------------
        //  Public class types
        // -------------------------------------------------------------------
        using TFilterList = TObjArray<TCQCTEvFilter>;


        // -------------------------------------------------------------------
        // Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCTrgEvent();

        TCQCTrgEvent
        (
            const   tCQCKit::EActCmdCtx     eContext
        );

        TCQCTrgEvent
        (
            const   TString&                strTitle
            , const tCQCKit::EActCmdCtx     eContext
        );

        TCQCTrgEvent(const TCQCTrgEvent&) = default;
        TCQCTrgEvent(TCQCTrgEvent&&) = default;

        ~TCQCTrgEvent();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCTrgEvent& operator=(const TCQCTrgEvent&) = default;
        TCQCTrgEvent& operator=(TCQCTrgEvent&&) = default;

        tCIDLib::TBoolean operator==
        (
            const   TCQCTrgEvent&           csrcSrc
        )   const;

        tCIDLib::TBoolean operator!=
        (
            const   TCQCTrgEvent&           csrcSrc
        )   const;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bEvaluate
        (
            const   TCQCEvent&              cevToTest
            , const tCIDLib::TBoolean       bIsNight
            , const tCIDLib::TEncodedTime   enctNow
            , const tCIDLib::TCard4         c4Hour
            , const tCIDLib::TCard4         c4Minute
        )   const;

        tCIDLib::TBoolean bLoggable() const;

        tCIDLib::TBoolean bLoggable
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TBoolean bPaused() const;

        tCIDLib::TBoolean bPaused
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TBoolean bSerialized() const;

        tCIDLib::TBoolean bSerialized
        (
            const   tCIDLib::TBoolean       bToSet
        );

        const TFilterList& colFilters() const;

        TFilterList& colFilters();

        tCQCKit::ETEvFiltLOps eFilterLogOp() const;

        tCQCKit::ETEvFiltLOps eFilterLogOp
        (
            const   tCQCKit::ETEvFiltLOps   eLogOp
        );

        tCIDLib::TVoid FilterSearchNReplace
        (
            const   TString&                strFind
            , const TString&                strReplaceWith
            , const tCIDLib::TBoolean       bRegEx
            , const tCIDLib::TBoolean       bFullMatch
            ,       TRegEx&                 regxFind
        );

        tCIDLib::TVoid SetContext
        (
            const   tCQCKit::EActCmdCtx     eContext
        );

        tCIDLib::TVoid SetAt
        (
            const   tCIDLib::TCard4         c4At
            , const tCIDLib::TBoolean       bNegate
            , const tCQCKit::ETEvFilters    eType
            , const TString&                strEvFld
            , const TString&                strCompVal
            , const tCIDLib::TBoolean       bCompValRegEx
            , const tCIDLib::TBoolean       bFldRegEx
        );


        // -------------------------------------------------------------------
        //  Just needed for the 5.0 upgrade process
        // -------------------------------------------------------------------
        const TString& strId() const;

        const TString& strId
        (
            const   TString&                strToSEt
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
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid CommInit();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bLoggable
        //      Each event can be marked as to whether it should be logged
        //      when invoked or not. Most of the time you don't care and
        //      don't want the extra traffic. But for debugging purposes,
        //      sometimes it's a good thing to be able to do this.
        //
        //  m_bPaused
        //      Events can be paused so that they don't run when their
        //      scheduled times arrive.
        //
        //  m_bSerialized
        //      See the header comments, serializes running of events of this type.
        //
        //  m_colFilters
        //      The filters used to evalute whether this event is to be
        //      invoked for a given incoming event broadcast.
        //
        //  m_eContext
        //      The action context is different in some places where triggered
        //      events are invoked. Client code must tell us which context
        //      to report for our trigger event.
        //
        //  m_eLogOp
        //      The logical operation to use to combine the results of the
        //      configured filters.
        //
        //  m_strId
        //      Pre-5.0 a unique id was used to represent each event. This is no
        //      longer needed since we store them hierarchically now. But we need
        //      to still read it in for the 5.0 upgrade process. Later we can get
        //      rid of it.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bLoggable;
        tCIDLib::TBoolean       m_bPaused;
        tCIDLib::TBoolean       m_bSerialized;
        TFilterList             m_colFilters;
        tCQCKit::EActCmdCtx     m_eContext;
        tCQCKit::ETEvFiltLOps   m_eLogOp;
        TString                 m_strId;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCTrgEvent,TCQCStdCmdSrc)
};



// ---------------------------------------------------------------------------
//  CLASS: TCQCTrgEventEx
// PREFIX: csrc
//
//  We need a trivial derivative of the triggered event class, so that we
//  can set a triggre data on it (that came in and caused this triggered
//  event to be run), and override the method that gens up a new RTV source
//  object. That RTV source object needs access to the event.
// ---------------------------------------------------------------------------
class CQCKITEXPORT TCQCTrgEventEx : public TCQCTrgEvent
{
    public  :
        // -------------------------------------------------------------------
        // Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCTrgEventEx() = delete;

        TCQCTrgEventEx
        (
            const   TCQCTrgEvent&           cevParent
            , const tCQCKit::TCQCEvPtr&     cptrTrigger
        );

        TCQCTrgEventEx
        (
            const   TString&                strTitle
            , const tCQCKit::EActCmdCtx     eContext
        );

        TCQCTrgEventEx(const TCQCTrgEventEx&) = default;
        TCQCTrgEventEx(TCQCTrgEventEx&&) = default;

        ~TCQCTrgEventEx();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCTrgEventEx& operator=(const TCQCTrgEventEx&) = default;
        TCQCTrgEventEx& operator=(TCQCTrgEventEx&&) = default;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        [[nodiscard]] TCQCCmdRTVSrc* pcrtsMakeNew
        (
            const   TCQCUserCtx&            cuctxToUse
        )   const override;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCQCKit::TCQCEvPtr cptrTrigger() const
        {
            return m_cptrTrigger;
        }


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_cptrTrigger
        //      The trigger info that that we we need to use when genning up the
        //      RTV source object.
        // -------------------------------------------------------------------
        tCQCKit::TCQCEvPtr m_cptrTrigger;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCTrgEventEx,TCQCTrgEvent)
};


// ---------------------------------------------------------------------------
//  CLASS: TKeyedCQCTrgEvent
// PREFIX: csrc
//
//  A convenience for folks who need to keep a list of these events, keyed by
//  the path.
// ---------------------------------------------------------------------------
class CQCKITEXPORT TKeyedCQCTrgEvent : public TCQCTrgEvent
{
    public  :
        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TKeyedCQCTrgEvent();

        TKeyedCQCTrgEvent
        (
            const   TString&                strPath
            , const TCQCTrgEvent&           csrcBase
        );

        TKeyedCQCTrgEvent(const TKeyedCQCTrgEvent&) = default;
        TKeyedCQCTrgEvent(TKeyedCQCTrgEvent&&) = default;

        ~TKeyedCQCTrgEvent();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TKeyedCQCTrgEvent& operator=(const TKeyedCQCTrgEvent&) = default;
        TKeyedCQCTrgEvent& operator=(TKeyedCQCTrgEvent&&) = default;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TCard4 c4SerialNum() const;

        tCIDLib::TCard4 c4SerialNum
        (
            const   tCIDLib::TCard4         c4ToSet
        );

        const TString& strPath() const;

        const TString& strPath
        (
            const   TString&                strToSet
        );

        tCIDLib::TVoid SetConfig
        (
            const   TCQCTrgEvent&           csrcBase
        );


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4SerialNum
        //      A serial number for the client code's use, typically used to make
        //      syncing with clients more efficient.
        //
        //  m_strPath
        //      The path from which this event was loaded. It could change if the
        //      user does a rename.
        // -------------------------------------------------------------------
        tCIDLib::TCard4 m_c4SerialNum;
        TString         m_strPath;
};

#pragma CIDLIB_POPPACK

