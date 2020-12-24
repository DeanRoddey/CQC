//
// FILE NAME: ZWaveUSB3Sh_CCImpl.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/31/2018
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
//  This is a simple abstract base class that all of the 'CC implementation' classes
//  are derived from. There's a bit of common functionality we need to have for them all.
//
//  End Point Encapsulation
//
//  One is to set an end point id. Any given impl object can be for a specific end
//  point. If not, the end point is set to 0xFF, which means that no outgoing msgs
//  will be encapsulated.
//
//  Also, our owning unit info object will see if an incoming msg was originally
//  encapsulated. If so, then it will see if our end point matches the end point of
//  the msg. If not, he won't dispatch it to us. If the incoming was not
//  encapsulated, then it doesn't care what our end point is since it can't be for
//  us.
//
//  However the derived class can set a flag on us to allow us to see msgs for all
//  end points. In some cases they may interact in some way, so one impl or all of
//  the interrelated end points may need to see all of their common msgs.
//
//  Fields
//
//  We have a problem that sometimes multiple CCs will drive the value of a field.
//  So it's tricky having CC impl objects create and manage the values of the
//  related fields. So, what we do is to create base classes for each CC that just
//  maintain internal values that reflect the state of the unit (as reported to
//  us at least.) Then we create a derivative of each of those that creates the
//  fields. If the base class reports a change in value or error state, the derived
//  class will update it's field accordingly.
//
//  In those cases where more than one CC would drive a field, the unit handler can
//  either create one that it lets manage the field, and then non-field ones that it
//  just passes on values from to the field one. Or it can just create the field(s)
//  itself and use non-field based CC impls to keep those fields updated.
//
//  So for instance, a lock will have a door lock CC which it uses to control and query
//  the state of the lock with. But the lock may use a notification to report changes.
//  Only one of those can create the field, so typically the field version of the door
//  lock is created and the non-field version of the notification. When changes are
//  seen on the notification, that value is written over to the door lock object to
//  keep it in sync. Generally it also goes the other way as well, with the door lock
//  passing on any queried lock state value to the notification. This way the
//  notification object will be in sync and can tell when it really gets a changed value,
//  thus avoiding bogus value change reports.
//
//  This is a bit more complex but ultimately gives us a lot of flexibility with a
//  minimum of duplicate code.
//
//  Read/Write Access
//
//  We have a number of flags that indicate what access our unit allows for this CC.
//  These are used by us and other things to control access. It can get kind of tricky
//  because there are a number of variations (it's Z-Wave of course.) The flags basically
//  match the values of the EUnitAcc enum since that is what we are setting.
//
//      Read          - Indicates if the unit is readable at all. If not, any other read
//                      flag is ignored and shouldn't be allowed to be set.
//                      This being set, combined with the owning unit's listener flag
//                      indicates that we can read it at any time, which implies it can
//                      be periodically polled as well. If this is set, the derived
//                      class MUST have overridden SendValueQuery() or an error will
//                      occur.
//      ReadAfterWrite- After a successfully acknowledged write, the CC should issue
//                      a read. Only valid if Readable.
//      ReadOnWake    - A read should be issued when a wakeup comes in. Used for
//                      non-listening units.
//      Write         - It can be written to. If this is set, there is no virtual that
//                      must be overridden. Only the derived class knows how to send
//                      values (in response to be told a field of his changed.)
//
//  These can be set in the extra infor for this CC, via the RWAccess value, which is
//  a space separated list of the above values, e.g.:
//
//      RWAccess=Read ReadAfterWrite Write
//
//  In our ctor we try to set reasonable defaults so that many derivatives or owning
//  units don't have to do busy work to set common access scenarios. These are:
//
//      1 - If always on, we set Readable, so polling gets enabled with a default
//          interval.
//      2 - Else, if wakeup is implemented, set readable and read on wakeup
//      3 - Else, no access flags, i.e. assume only notification driven unless we
//          are explicitly set otherwise.
//
//  These could get overridden by RWAccess or by the owning unit creating us and
//  forcing them. Note that, when set via RWAccess, all default flags set by the ctor
//  are cleared and then these are set.
//
//  Event Triggers
//
//  As per Fields above, only one CC impl (or the unit handler itself) will manage a
//  given field. It needs to send event triggers if that is appropriate. The CC impl
//  (or unit handler if it manages the field) has to know whether the field is of a type
//  that is appropriate for triggers.
//
//  In some cases triggers are optional and have to be configured by the user. The
//  unit level handler has to know when this is the case and add a standard unit
//  option (kZWaveUSB3Sh::strUOpt_SendTrigger) that the user can configure. The CC impl
//  or unit that sends the trigger must check for this and only send if it is set and
//  true. If it doesn't exist on the unit, assume we can send the trigger.
//
//  Common Derivatives
//
//  We also create a simple derivative that can be used as a base class for CC impls
//  that have a binary state. It just defines virtuals by which the state can be
//  accessed, which is important to reduce grunt work. Lots of stuff only cares to
//  see if the state changes and, if so, what is the new state or error state. It
//  otherwise does all that is needed. We do similarly for simple level based ones.
//
//  So a representative derivative chain is:
//
//      TZWCCImp - The base CC impl class
//      TZWCCImplBin - Handles grunt work for binary values
//      TZWCCImplBasicSwitch - Non-field handler for the BASIC CC's off/on state, so it
//                              provides storage for the off/on state with get/set methods.
//      TZWCCImplBasicSwitchF - The field handling derivative of the BASIC CC off/on state
//
//  The TZWCCImplBin class' setter reports a change if a new value is stored on him. That
//  is seen by the derivatives which can do whatever they need to do in response, such as
//  send out a BASIC SET in this case. TZWCCImplBasicSwitchF deals with BASIC CC msgs and
//  stores any new incoming off/on value on the TZWCCImplBin class.
//
//  The report of a value change is sent both to the derived classes and to the containing
//  unit (since he may be actually managing the fields or have other reasons to know
//  about this.) The base CC Impl class handles this reporting. All derivatives call
//  ReportValueChange() when their value changes. It in turn calls the virtual
//  ValueChanged() to let derived classes know, and calls ImplValueChanged() on the
//  containing unit object.
//
//  Polling
//
//  We don't poll per se. We assume all units will report their status changes async.
//  But, for any readable CC, we want to at least periodically poll it if we don't
//  get any message for some period of time. And we want to try to get an initial value
//  initially.
//
//  There is a default polling interval that is used when things are going well, so
//  every time we get a value msg we set a next poll time stamp to now plus the default
//  polling interval. That is set to a slightly randomized value based around the
//  actual set value. If the derivatives don't set one explicitly, we use 10 minutes for
//  wall powered and 30 minutes for frequent listeners. Derivatives that can handle a
//  longer poll should do so or they should disable polling if they are readable but
//  for some reason cannot handle a poll.
//
//  When something goes wrong and we don't get a response to a poll (and initially when
//  we first are starting), then we start a graduated timeout where it starts reasonably
//  quick and then gets bumped up more for each successive timeout until it hits a max
//  where we just stay until it responds or we get an async. This way we try to recover
//  fairly quickly if we can, try to get past a short interruption if that's what it is,
//  and then stop using too much battery and Z-Stick time if it's really non-responsive.
//  If we get a response or async, then we go back to happy state and and the default
//  timeout.
//
//  We don't remember the last time we asked for a value. We just watch for 'now' to go
//  beyond the stored 'next poll time' value. If so, then clearly we never got anything
//  (reply or async) within the timeout and do another query.
//
//  The poll interval being non-zero enables this process. It will be left zero for
//  non-readables and if our containing unit is a non-listener. So we only do it for
//  readable, always on units.
//
//  All this is handled in the base CC Impl class. The containing unit calls Process()
//  on us every so so many seconds. We are then free to do things periodically by just
//  keeping the appropriate time stamps. If it's time to do a poll, we call the virtual
//  SendValueQuery() method and update the time stamp to the next time.
//
//  We maintain a timeout count which is bumped ever time we don't get anything before
//  the next time. It gets cleared any time we get a message. The timeout count is used
//  to run up the timeout period more and more.
//
//  This we get three timeouts in a row, we ask the derived class to go into error mode
//  and set our m_bError flag. Field based derivatives will put their field(s) into
//  error state. We keep trying at increasing intervals until we hopefully get a reply.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once

#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: TZWCCImpl
//  PREFIX: zwcci
// ---------------------------------------------------------------------------
class ZWUSB3SHEXPORT TZWCCImpl : public TObject
{
    public :
        // -------------------------------------------------------------------
        //  Public constructors and Destructor
        // -------------------------------------------------------------------
        TZWCCImpl(const TZWCCImpl&) = delete;

        ~TZWCCImpl();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TZWCCImpl& operator=(const TZWCCImpl&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        virtual tCIDLib::TBoolean bBoolFldChanged
        (
            const   TString&                strFldName
            , const tCIDLib::TCard4         c4FldId
            , const tCIDLib::TBoolean       bValue
            ,       tCQCKit::ECommResults&  eRes
        );

        virtual tCIDLib::TBoolean bCardFldChanged
        (
            const   TString&                strFldName
            , const tCIDLib::TCard4         c4FldId
            , const tCIDLib::TCard4         c4Value
            ,       tCQCKit::ECommResults&  eRes
        );

        virtual tCIDLib::TBoolean bFloatFldChanged
        (
            const   TString&                strFldName
            , const tCIDLib::TCard4         c4FldId
            , const tCIDLib::TFloat8        f8Value
            ,       tCQCKit::ECommResults&  eRes
        );

        virtual tCIDLib::TBoolean bIntFldChanged
        (
            const   TString&                strFldName
            , const tCIDLib::TCard4         c4FldId
            , const tCIDLib::TInt4          i4Value
            ,       tCQCKit::ECommResults&  eRes
        );

        virtual tCIDLib::TBoolean bOwnsFld
        (
            const   tCIDLib::TCard4         c4FldId
        )   const;

        virtual tCIDLib::TBoolean bParseExtraInfo();

        virtual tCIDLib::TBoolean bPrepare();

        virtual tCIDLib::TBoolean bStringFldChanged
        (
            const   TString&                strFldName
            , const tCIDLib::TCard4         c4FldId
            , const TString&                strValue
            ,       tCQCKit::ECommResults&  eRes
        );

        virtual tCIDLib::TVoid FormatReport
        (
                    TTextOutStream&         strmTar
        )   const;

        virtual tCIDLib::TVoid Process();

        virtual tCIDLib::TVoid QueryFldDefs
        (
                    tCQCKit::TFldDefList&   colAddTo
            ,       TCQCFldDef&             flddTmp
        )   const;

        virtual tCIDLib::TVoid QueryCCAttrs
        (
                    tCIDMData::TAttrList&   colToFill
            ,       TAttrData&              attrTmp
        )   const;

        virtual tCIDLib::TVoid StoreFldIds();


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bReadAlways() const;

        tCIDLib::TBoolean bIsReadable() const;

        tCIDLib::TBoolean bSeeAllEPs() const
        {
            return m_bSeeAllEPs;
        }

        tCIDLib::TBoolean bSendReadAfterWrite() const;

        tCIDLib::TBoolean bSendReadOnWakeup() const;

        tCIDLib::TBoolean bIsWriteable() const;

        tCIDLib::TCard1 c1EPId() const
        {
            return m_c1EPId;
        }

        tCIDLib::TCard4 c4ImplId() const;

        tCIDLib::TCard4 c4ImplId
        (
            const   tCIDLib::TCard4         c4ToSet
        );

        tZWaveUSB3Sh::ECClasses eClass() const
        {
            return m_eClass;
        }

        tCIDLib::TVoid DisablePoll();

        tCIDLib::TVoid ProcessCCMsg
        (
            const   tCIDLib::TCard1         c1Class
            , const tCIDLib::TCard1         c1Cmd
            , const TZWInMsg&               zwimToHandle
        );

        tCIDLib::TVoid RequestValueQuery();

        const TString& strDesc() const;

        const TString& strDesc
        (
            const   TString&                strToSet
        );

        tCIDLib::TVoid SendInitValueQuery();

        tCIDLib::TVoid SetAccess
        (
            const   tZWaveUSB3Sh::EUnitAcc  eToSet
            , const tZWaveUSB3Sh::EUnitAcc  eMask = tZWaveUSB3Sh::EUnitAcc::AllBits
        );

        tCIDLib::TVoid SetErrState();

        tCIDLib::TVoid SetWriteAccess
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TVoid UnitIsOnline();


    protected :
        // -------------------------------------------------------------------
        //  Hidden constructors
        // -------------------------------------------------------------------
        TZWCCImpl
        (
                    TZWUnitInfo* const      punitiOwner
            , const tZWaveUSB3Sh::ECClasses eClass
            , const tCIDLib::TCard1         c1EPId = 0xFF
        );


        // -------------------------------------------------------------------
        //  Protected, virtual methods
        // -------------------------------------------------------------------
        virtual tCIDLib::TVoid SendValueQuery();


        // -------------------------------------------------------------------
        //  Protected, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bAlwaysOn() const;

        tCIDLib::TBoolean bError() const;

        tCIDLib::TBoolean bFindExtraVal
        (
            const   TString&                strKey
            ,       TString&                strValue
        )   const;

        tCIDLib::TBoolean bFindOptVal
        (
            const   TString&                strKey
            ,       TString&                strToFill
        )   const;

        tCIDLib::TBoolean bQueryFldName
        (
            const   tCIDLib::TCard4         c4FldId
            ,       TString&                strToFill
        )   const;

        tCIDLib::TBoolean bQueryOptVal
        (
            const   TString&                strKey
            , const tCIDLib::TBoolean       bDefault
        )   const;

        tCIDLib::TBoolean bWriteFld
        (
            const   tCIDLib::TCard4         c4FldId
            , const tCIDLib::TBoolean       bToStore
        );

        tCIDLib::TBoolean bWriteFld
        (
            const   tCIDLib::TCard4         c4FldId
            , const tCIDLib::TCard4         c4ToStore
        );

        tCIDLib::TBoolean bWriteFld
        (
            const   tCIDLib::TCard4         c4FldId
            , const tCIDLib::TFloat8        f8ToStore
        );

        tCIDLib::TBoolean bWriteFld
        (
            const   tCIDLib::TCard4         c4FldId
            , const tCIDLib::TInt4          i4ToStore
        );

        tCIDLib::TBoolean bWriteFld
        (
            const   tCIDLib::TCard4         c4FldId
            , const TString&                strToStore
        );

        tCIDLib::TCard4 c4LookupFldId
        (
            const   TString&                strFldName
        )   const;

        tCIDLib::TCard1 c1UnitId() const;

        tCIDLib::TCard4 c4QueryOptVal
        (
            const   TString&                strKey
            , const tCIDLib::TCard4         c4Default
        )   const;

        tCQCKit::EFldAccess eToFldAccess() const;

        tCIDLib::TInt4 i4QueryOptVal
        (
            const   TString&                strKey
            , const tCIDLib::TInt4          i4Default
        )   const;

        tCIDLib::TVoid QueueEventTrig
        (
            const   tCQCKit::EStdDrvEvs     eEvent
            , const TString&                strFld
            , const TString&                strVal1
            , const TString&                strVal2
            , const TString&                strVal3
            , const TString&                strVal4
        );

        const TString& strDrvMoniker() const;

        const TString& strUnitName() const;

        tCIDLib::TVoid SendMsgAsync
        (
                    TZWOutMsg&              zwomToSend
        );

        tCIDLib::TVoid SendWaitAck
        (
                    TZWOutMsg&              zwomToSend
        );

        tCIDLib::TVoid SetFieldErr
        (
            const   tCIDLib::TCard4         c4FldId
        );

        const TZWUnitInfo& unitiOwner() const
        {
            return *m_punitiOwner;
        }


    protected :
        // -------------------------------------------------------------------
        //  Protected, virtual methods
        // -------------------------------------------------------------------
        virtual tZWaveUSB3Sh::ECCMsgRes eHandleCCMsg
        (
            const   tCIDLib::TCard1         c1Class
            , const tCIDLib::TCard1         c1Cmd
            , const TZWInMsg&               zwimToHandle
        ) = 0;

        virtual tCIDLib::TVoid EnteredErrState();

        virtual tCIDLib::TVoid ValueChanged
        (
            const   tZWaveUSB3Sh::EValSrcs  eSource
            , const tCIDLib::TBoolean       bWasInErr
            , const tCIDLib::TInt4          i4ExtraInfo
        );


        // -------------------------------------------------------------------
        //  Protected, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid ReadableChanged();

        tCIDLib::TVoid ReportValChange
        (
            const   tZWaveUSB3Sh::EValSrcs  eSource
            , const tCIDLib::TInt4          i4ExtraInfo
        );

        tCIDLib::TVoid SetDefPollMinutes
        (
            const   tCIDLib::TCard4         c4Minutes
        );


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid CalcDefPollInt();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bError
        //      All CC impls (the non-field base ones) may be in an error state or
        //      they may have (a) good value(s). So we provide this flag for them
        //      to use. It is their responsibility to make sure it is set correctly.
        //      It defaults to true so that we are in error until some valid value
        //      gets stored.
        //
        //  m_bSeeAllEPs
        //      Normally, if an incoming msg was end point enscapsulated, we will
        //      only get called if its end point matches ours. But, in some cases
        //      the derivative may need to see all end points, perhaps because they
        //      interact in some way. So this can be set.
        //
        //  m_c1EPId
        //      If this implementation is for a specific end point, this is from
        //      0 to 127. If so, then CheckEPEncap() will enscapsulate the passed
        //      msg with a target EP of m_c1EPId.
        //
        //      This is is also used to decide whether incoming enscapsulated msgs
        //      are passed to us. See m_bSeeAllEPs above.
        //
        //      We default it to 0xFF. That means no outgoing encapsulation. It assumes
        //      we will not get encapsulated msgs, so the fact that it doesn't match
        //      the zero that will be the default EP value in the incoming msgs
        //      won't matter.
        //
        //  m_c4ImplId
        //      When we are added to the owning unit object, we are assigned an id
        //      (unique within that unit.) The unit is given back that id as well in
        //      the call.
        //
        //  m_c4TOCount
        //      See the Polling section of the header comments above.
        //
        //  m_eClass
        //      The class we represent.
        //
        //  m_enctDefPollInt
        //      See the Polling section of the header comments above. This is the steady
        //      state requested default interval used once we have a responsive unit.
        //      Defaults to 10 minutes for wall powered and 1 hour for frequent listeners,
        //      if the CC acccess is readable. But it will be slightly randomized to avoid
        //      a lot of traffic at the same time.
        //
        //      If the CC is readable and a listener, but a safety poll isn't acceptable
        //      the CC must disable pinging explicitly.
        //
        //  m_enctNextPoll
        //      The time that this CC impl needs to do a poll (if it does them at all.)
        //      This is updated each time we get a value related message or when we fail
        //      to. So we only need to poll if we don't see other traffic for that long.
        //
        //  m_eAccess
        //      Indicates the access our unit allows for this CC. They may be
        //      set via the RWAccess extra info set this CC, or manually set by
        //      a creating unit. We set defaults in our ctor based on some very
        //      rough guesses that are right a lot of the time.
        //
        //      We don't allow any of the optional read bits to be set if readable is
        //      not set.
        //
        //  m_punitiOwner
        //      We get a pointer to our owning unit info object.
        //
        //  m_strDesc
        //      The unit (or derived class) can set a short descriptive name on us
        //      that can be used in trace msgs or log msgs. If a unit has more than
        //      one CC impl (not uncommon), this can help a lot with debugging.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bError;
        tCIDLib::TBoolean       m_bSeeAllEPs;
        tCIDLib::TCard1         m_c1EPId;
        tCIDLib::TCard4         m_c4ImplId;
        tCIDLib::TCard4         m_c4TOCount;
        tZWaveUSB3Sh::ECClasses m_eClass;
        tZWaveUSB3Sh::EUnitAcc  m_eAccess;
        tCIDLib::TEncodedTime   m_enctDefPollInt;
        tCIDLib::TEncodedTime   m_enctNextPoll;
        TZWUnitInfo*            m_punitiOwner;
        TString                 m_strDesc;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWCCImpl, TObject)
};


// ---------------------------------------------------------------------------
//   CLASS: TZWCCImplBin
//  PREFIX: zwcci
// ---------------------------------------------------------------------------
class TZWCCImplBin : public TZWCCImpl
{
    public :
        // -------------------------------------------------------------------
        //  Public constructors and Destructor
        // -------------------------------------------------------------------
        TZWCCImplBin(const TZWCCImplBin&) = delete;

        ~TZWCCImplBin();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TZWCCImplBin& operator=(const TZWCCImplBin&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bState() const;

        tCIDLib::TBoolean bSetBinState
        (
            const   tCIDLib::TBoolean       bNew
            , const tZWaveUSB3Sh::EValSrcs  eSource
            , const tCIDLib::TInt4          i4ExtraInfo = 0
        );


    protected :
        // -------------------------------------------------------------------
        //  Hidden constructors
        // -------------------------------------------------------------------
        TZWCCImplBin
        (
                    TZWUnitInfo* const      punitiOwner
            , const tZWaveUSB3Sh::ECClasses eClass
            , const tCIDLib::TCard1         c1EPId = 0xFF
        );


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bState
        //      The last state set on us. This interacts with the bError value
        //      in the base impl class. If that is true, then this should be
        //      ignored.
        //
        //      When this is changed, we call the virtual BoolImplStateChanged()
        //      method so that derived classes can update appropriately.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bState;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWCCImplBin, TZWCCImpl)
};



// ---------------------------------------------------------------------------
//   CLASS: TZWCCImplLevel
//  PREFIX: zwcci
// ---------------------------------------------------------------------------
class TZWCCImplLevel : public TZWCCImpl
{
    public :
        // -------------------------------------------------------------------
        //  Public constructors and Destructor
        // -------------------------------------------------------------------
        TZWCCImplLevel(const TZWCCImplLevel&) = delete;

        ~TZWCCImplLevel();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TZWCCImplLevel& operator=(const TZWCCImplLevel&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bSetLevel
        (
            const   tCIDLib::TCard4         c4New
            , const tZWaveUSB3Sh::EValSrcs  eSource
            , const tCIDLib::TInt4          i4ExtraInfo = 0
        );

        tCIDLib::TCard4 c4Level() const;


    protected :
        // -------------------------------------------------------------------
        //  Hidden constructors
        // -------------------------------------------------------------------
        TZWCCImplLevel
        (
                    TZWUnitInfo* const      punitiOwner
            , const tZWaveUSB3Sh::ECClasses eClass
            , const tCIDLib::TCard1         c1EPId = 0xFF
        );


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4Level
        //      The last level set on us. This interacts with the bError value
        //      in the base impl class. If that is true, then this should be
        //      ignored.
        //
        //      When this is changed, we call the virtual ImplLevelChanged()
        //      method so that derived classes can update appropriately.
        // -------------------------------------------------------------------
        tCIDLib::TCard4     m_c4Level;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWCCImplLevel, TZWCCImpl)
};

#pragma CIDLIB_POPPACK
