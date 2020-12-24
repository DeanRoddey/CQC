//
// FILE NAME: ZWaveUSB3Sh_CCImpl.cpp
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
//  This is the base class for all of the CC implementation derivatives.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "ZWaveUSB3Sh_.hpp"


// ---------------------------------------------------------------------------
//  Magic maros
// ---------------------------------------------------------------------------
RTTIDecls(TZWCCImpl, TObject)
RTTIDecls(TZWCCImplBin, TZWCCImpl)
RTTIDecls(TZWCCImplLevel, TZWCCImpl)



// ---------------------------------------------------------------------------
//   CLASS: TZWCCImpl
//  PREFIX: zwcci
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWCCImpl: Constructors and Destructor
// ---------------------------------------------------------------------------
TZWCCImpl::~TZWCCImpl()
{
}


// ---------------------------------------------------------------------------
//  TZWCCImpl: Public, virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TZWCCImpl::bBoolFldChanged( const   TString&
                            , const tCIDLib::TCard4
                            , const tCIDLib::TBoolean
                            ,       tCQCKit::ECommResults&  eRes)
{
    eRes = tCQCKit::ECommResults::Unhandled;
    return kCIDLib::False;
}

tCIDLib::TBoolean
TZWCCImpl::bCardFldChanged( const  TString&
                            , const tCIDLib::TCard4
                            , const tCIDLib::TCard4
                            ,       tCQCKit::ECommResults&  eRes)
{
    eRes = tCQCKit::ECommResults::Unhandled;
    return kCIDLib::False;
}

tCIDLib::TBoolean
TZWCCImpl::bFloatFldChanged(const   TString&
                            , const tCIDLib::TCard4
                            , const tCIDLib::TFloat8
                            ,       tCQCKit::ECommResults&  eRes)
{
    eRes = tCQCKit::ECommResults::Unhandled;
    return kCIDLib::False;
}

tCIDLib::TBoolean
TZWCCImpl::bIntFldChanged(  const  TString&
                            , const tCIDLib::TCard4
                            , const tCIDLib::TInt4
                            ,       tCQCKit::ECommResults&  eRes)
{
    eRes = tCQCKit::ECommResults::Unhandled;
    return kCIDLib::False;
}


tCIDLib::TBoolean TZWCCImpl::bOwnsFld(const tCIDLib::TCard4 c4FldId) const
{
    return kCIDLib::False;
}


//
//  The derived class must call us first, and return false if we return false. We check
//  for some common extra info flags.
//
tCIDLib::TBoolean TZWCCImpl::bParseExtraInfo()
{
    //
    //  The unit info class has a method to get the access flags, since they are needed
    //  in other places before the CC impl is created sometimes. So we use that same
    //  method in order to avoid redundancy.
    //
    //  NOTE that we will override any default flags set in the ctor, not just set new
    //  ones indicated in the RWAccess value.
    //
    const tZWaveUSB3Sh::EUnitAcc eOrgFlags = m_eAccess;
    tZWaveUSB3Sh::EUnitAcc eNewAcc;
    const tCIDLib::EFindRes2 eRes = m_punitiOwner->eProbeCCAccess
    (
        eClass(), eNewAcc, m_c1EPId
    );

    // If the probe failed, return failure on our part
    if (eRes == tCIDLib::EFindRes2::Error)
        return kCIDLib::False;

    if (eRes == tCIDLib::EFindRes2::Found)
    {
        // Take the new flags
        m_eAccess = eNewAcc;

        // If the readable state changed, then update some other things
        if (tCIDLib::bBitsChanged(m_eAccess, eOrgFlags, tZWaveUSB3Sh::EUnitAcc::Read))
            ReadableChanged();
    }
    return kCIDLib::True;
}


// The derived class must call us first, and return false if we return false
tCIDLib::TBoolean TZWCCImpl::bPrepare()
{
    return kCIDLib::True;
}


tCIDLib::TBoolean
TZWCCImpl::bStringFldChanged(const  TString&
                            , const tCIDLib::TCard4
                            , const TString&
                            ,       tCQCKit::ECommResults&  eRes)
{
    eRes = tCQCKit::ECommResults::Unhandled;
    return kCIDLib::False;
}


//
//  The owning unit calls us here to ask us to format out our info. If the derived
//  class anything that needs to be reported, it can override, call us first, then
//  output his stuff. For most, we probably do what is needed.
//
tCIDLib::TVoid
TZWCCImpl::FormatReport(TTextOutStream& strmTar) const
{
    strmTar << L"Descr: " << m_strDesc
            << L", Class: " << m_eClass;

    // Do the access bits in a readable way
    strmTar << L", Access: ";
    if (tCIDLib::bAllBitsOn(m_eAccess, tZWaveUSB3Sh::EUnitAcc::Read))
        strmTar << L"Read ";
    if (tCIDLib::bAllBitsOn(m_eAccess, tZWaveUSB3Sh::EUnitAcc::ReadOnWake))
        strmTar << L"ReadOnWake ";
    if (tCIDLib::bAllBitsOn(m_eAccess, tZWaveUSB3Sh::EUnitAcc::ReadAfterWrite))
        strmTar << L"ReadAfterWrite ";
    if (tCIDLib::bAllBitsOn(m_eAccess, tZWaveUSB3Sh::EUnitAcc::Write))
        strmTar << L"Write";

    // If polling is enabled, then format the period
    if (m_enctDefPollInt)
    {
        strmTar << L", Poll Secs: "
                << tCIDLib::TCard4(m_enctDefPollInt / kCIDLib::enctOneSecond);
    }

    // If we are not just default end point, then display end point info
    if (m_c1EPId != 0xFF)
    {
        strmTar << L", EP Id: " << m_c1EPId
                << L", See AllEPs :"
                << facCQCKit().strBoolYesNo(m_bSeeAllEPs);
    }

    strmTar << kCIDLib::NewLn;
}


//
//  The derived class must call us if it overrides. At this level we provide some
//  standard support, such as regular polls.
//
tCIDLib::TVoid TZWCCImpl::Process()
{
    //
    //  If no default poll interval or not readable then nothing to do. We shouldn't
    //  get called here if not readable, but be safe.
    //
    if (!m_enctDefPollInt)
        return;

    #if CID_DEBUG_ON
    if (!bIsReadable())
    {
        facZWaveUSB3Sh().LogTraceErr
        (
            L"Process() called on %(1)/%(2) but it is not readable"
            , strUnitName()
            , strDesc()
        );
        return;
    }
    #endif

    //
    //  If we have gone past the last calculated poll time, then try one. If the next
    //  poll is zero, this is the first one, so we don't treat that like a failure.
    //
    const tCIDLib::TEncodedTime enctNow = TTime::enctNow();
    if (enctNow > m_enctNextPoll)
    {
        //
        //  Bump the timeout count. It gets reset when we get a valid value msg that
        //  takes us out of error state.
        //
        m_c4TOCount++;

        // If not the first one, and trace is enabled, then log something
        if (m_enctNextPoll)
        {
            // First time just a msg if high tracing. Else an error if low
            if (m_c4TOCount == 1)
            {
                if (facZWaveUSB3Sh().bHighTrace())
                {
                    facZWaveUSB3Sh().LogTraceMsg
                    (
                        L"%(1)/%(2)- Poll attempt #%(3)"
                        , strUnitName()
                        , m_strDesc
                        , TCardinal(m_c4TOCount)
                    );
                }
            }
             else
            {
                if (facZWaveUSB3Sh().bLowTrace())
                {
                    facZWaveUSB3Sh().LogTraceErr
                    (
                        L"%(1)/%(2)- Poll attempt #%(3)"
                        , strUnitName()
                        , m_strDesc
                        , TCardinal(m_c4TOCount)
                    );
                }
            }
        }

        // If we've hit three, then go to error state
        if (m_c4TOCount > 3)
        {
            if (!m_bError)
            {
                SetErrState();
                facZWaveUSB3Sh().LogTraceErr
                (
                    L"%(1)/%(2)- Entered error state due to three failed polls"
                    , strUnitName()
                    , strDesc()
                );
            }
        }

        //
        //  Based on the timeout count, set a next poll. The first few we want to
        //  be fairly quick so we can try initially to get a value. After that we
        //  go to a long interval. For wall powered we stay at 5 minutes, and for
        //  frequent listeners we go to 20. Once we ever get a response we will go
        //  the default interval.
        //
        if (m_c4TOCount < 3 )
            m_enctNextPoll = enctNow + (kCIDLib::enctOneSecond * 30);
        else if (m_c4TOCount == 3)
            m_enctNextPoll = enctNow + kCIDLib::enctOneMinute;
        else if (m_c4TOCount == 4)
            m_enctNextPoll = enctNow + (kCIDLib::enctOneMinute * 2);
        else if (m_c4TOCount == 5)
            m_enctNextPoll = enctNow + (kCIDLib::enctOneMinute * 4);
        else
        {
            if (m_punitiOwner->punitThis()->bFreqListener())
                m_enctNextPoll = enctNow + (kCIDLib::enctOneMinute * 20);
            else
                m_enctNextPoll = enctNow + (kCIDLib::enctOneMinute * 5);
        }

        if (facZWaveUSB3Sh().bHighTrace())
        {
            facZWaveUSB3Sh().LogTraceMsg
            (
                L"%(1)/%(2)- New poll time is #%(3) seconds"
                , strUnitName()
                , m_strDesc
                , TCardinal(tCIDLib::TCard4((m_enctNextPoll - enctNow) / kCIDLib::enctOneSecond))
            );
        }

        // Send a value query
        SendValueQuery();
    }
}


// A do nothing impl for those that don't create any fields
tCIDLib::TVoid TZWCCImpl::QueryFldDefs(tCQCKit::TFldDefList&, TCQCFldDef&) const
{
}


//
//  A do nothing impl. Most of these will not have any user settable options. They
//  mostly will be driven by the unit handler classes that create the impls. But if
//  some impl has a non-negotiable option it has to have, it can override this. It
//  should call us first just in case.
//
tCIDLib::TVoid
TZWCCImpl::QueryCCAttrs(tCIDMData::TAttrList&, TAttrData&) const
{
}


// A do nothing impl for those that don't create any fields
tCIDLib::TVoid TZWCCImpl::StoreFldIds()
{
}


// ---------------------------------------------------------------------------
//  TZWCCImpl: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Returns whether the read access flag is set
tCIDLib::TBoolean TZWCCImpl::bIsReadable() const
{
    return tCIDLib::bAllBitsOn(m_eAccess, tZWaveUSB3Sh::EUnitAcc::Read);
}


// Returns whether the write access flag is set
tCIDLib::TBoolean TZWCCImpl::bIsWriteable() const
{
    return tCIDLib::bAllBitsOn(m_eAccess, tZWaveUSB3Sh::EUnitAcc::Write);
}



//
//  If the readable flag is on, and our parent unit is an always on unit, then we can
//  read the value any time, which implies we can poll it regularly as well.
//
tCIDLib::TBoolean TZWCCImpl::bReadAlways() const
{
    return  tCIDLib::bAllBitsOn(m_eAccess, tZWaveUSB3Sh::EUnitAcc::Read)
            && m_punitiOwner->bAlwaysOn();
}


// Returns whether the read after write flag is set
tCIDLib::TBoolean TZWCCImpl::bSendReadAfterWrite() const
{
    return tCIDLib::bAllBitsOn(m_eAccess, tZWaveUSB3Sh::EUnitAcc::ReadAfterWrite);
}


// Returns whether the read on wakeup flag is set
tCIDLib::TBoolean TZWCCImpl::bSendReadOnWakeup() const
{
    return tCIDLib::bAllBitsOn(m_eAccess, tZWaveUSB3Sh::EUnitAcc::ReadOnWake);
}



//
//  Get or set our impl id. Typically set by the base unit class when impls are added
//  to it. But some units manage their own. If they have more than one they may need
//  to set some type of unique id on each one so they can distinguish which one is
//  sending them some callback.
//
tCIDLib::TCard4 TZWCCImpl::c4ImplId() const
{
    return m_c4ImplId;
}

tCIDLib::TCard4 TZWCCImpl::c4ImplId(const tCIDLib::TCard4 c4ToSet)
{
    m_c4ImplId = c4ToSet;
    return m_c4ImplId;
}


//
//  Polling is enabled (very slow) for readable CCs on listening units. But sometimes
//  there's no need, so we allow it to be disabled explicitly.
//
tCIDLib::TVoid TZWCCImpl::DisablePoll()
{
    m_enctDefPollInt = 0;
}


//
//  This is called by the unit handler for any incoming msgs, to give us a change
//  to handle it. We call a virtual method that derived classes can override to
//  handle msgs. The return tells us if they ignore it, if they handled ir, or if
//  they handled it and it was a msg that provided them with a value (even if it
//  isn't different from what is there.)
//
//  For now we watch for unhandled msgs, but which would indicate that the device is
//  communicating. If we are in error state and this is a polled unit, we will reset
//  the timeout sequence to try to get going again as quickly as possible.
//
//  Or we look for msgs that represent a valid value msg for that class (even if not
//  a different value from the current one.) In this case, if that got us out of error
//  state (if we were in it) we assume we can set the next default poll interval.
//
tCIDLib::TVoid
TZWCCImpl::ProcessCCMsg(const   tCIDLib::TCard1 c1Class
                        , const tCIDLib::TCard1 c1Cmd
                        , const TZWInMsg&       zwimToHandle)
{
    tZWaveUSB3Sh::ECCMsgRes eRes = eHandleCCMsg(c1Class, c1Cmd, zwimToHandle);

    //
    //  Regardless of result, if it's a wakeup notification and this guy is marked as
    //  read on wakeup, we call SendValueQuery(). It will queue up a value query if
    //  that is possible.
    //
    if ((c1Class == COMMAND_CLASS_WAKE_UP)
    &&  (c1Cmd == WAKE_UP_NOTIFICATION)
    &&  bSendReadOnWakeup()
    &&  bIsReadable())
    {

        SendValueQuery();
    }

    if ((eRes == tZWaveUSB3Sh::ECCMsgRes_Unhandled) && m_bError && m_enctDefPollInt)
    {
        //
        //  Notification might be a valid value msg for this guy, but if so we should
        //  not be in error state after having processed it above since he would have
        //  called ReportValueChange() and that would have cleared the flag. It could
        //  be some other notification that the device sends such as the case being
        //  opened and closed, which would be typical for getting a unit going again
        //  after something like replacing the batteries.
        //
        //  NOTE that polled units would never actually send a wakeup, but we translate
        //  node info msgs into a faux wakeup as well, so if they put the batteries back
        //  in we might get one of those. This scheme avoids having to have another, non-
        //  CC msgs related callback.
        //
        //  If this CC is marked as read on wakeup, and its a wakeup notification, we
        //  queue up a read if that's valid.
        //
        if (((c1Class == COMMAND_CLASS_WAKE_UP) && (c1Cmd == WAKE_UP_NOTIFICATION))
        ||  ((c1Class == COMMAND_CLASS_NOTIFICATION_V3) && (c1Cmd == NOTIFICATION_REPORT_V3)))
        {
            if (facZWaveUSB3Sh().bMediumTrace())
            {
                facZWaveUSB3Sh().LogTraceMsg
                (
                    L"%(1)/%(2)- Got msg when in error state, restarting timeout sequence"
                    , strUnitName()
                    , m_strDesc
                );
            }
            m_enctNextPoll = 0;
            m_c4TOCount = 0;
        }
    }
     else if (eRes == tZWaveUSB3Sh::ECCMsgRes_Value)
    {
        // If this guy is set up for polling, reset for the next poll
        if (m_enctDefPollInt && !m_bError)
        {
            if (facZWaveUSB3Sh().bHighTrace())
            {
                facZWaveUSB3Sh().LogTraceMsg
                (
                    L"%(1)/%(2)- Valid value received, setting next default poll interval"
                    , strUnitName()
                    , m_strDesc
                );
            }
            m_enctNextPoll = TTime::enctNow() + m_enctDefPollInt;
            m_c4TOCount = 0;
        }
    }
}


//
//  In some cases the unit handlers may need to force one of its CC impls to do a value
//  query when it sees a change in another. But we don't expose the SendValueQuery()
//  method for good reason. It doesn't know our condition. So we provide this one. If

//  If this CC impl is non-polling, then we just call SendValueQuery() since there can't
//  be any conflict.
//
//  If this guy is polling, and it isn't currently in a timeout condition, then we'll
//  reset the next poll interval to now and call Process to get it to do a query now.
//
//  If it's in a timeout condition, we don't want to try to make it poll now since it
//  could be waiting for one already. So we'll just reset the timeout counter so that
//  it starts the progressive timeout sequenece again, and we'll force the next poll
//  time to the initial poll timeout poll time, so that it'll do it soon but not too
//  soon.
//
tCIDLib::TVoid TZWCCImpl::RequestValueQuery()
{
    // Obviously should only be called for readables
    CIDAssert(bIsReadable(), L"RequestValueQuery() called for a non-readable CC impl");

    if (facZWaveUSB3Sh().bHighTrace())
    {
        facZWaveUSB3Sh().LogTraceErr
        (
            L"%(1)/%(2)- An external value query request was made"
            , strUnitName()
            , m_strDesc
        );
    }

    if (m_enctDefPollInt)
    {
        if (m_c4TOCount)
        {
            m_c4TOCount = 0;
            m_enctNextPoll = TTime::enctNowPlusSecs(30);
        }
         else
        {
            m_enctNextPoll = 0;
            Process();
        }
    }
     else
    {
        SendValueQuery();
    }
}


//
//  Get/set a short descriptive name on this CC, to indicate what purpose it serves.
//  Sometimes the derived class is very specific and can do this himself, others
//  (most) are more general and the unit class that creates them will set something
//  appropriate.
//
const TString& TZWCCImpl::strDesc() const
{
    return m_strDesc;
}

const TString&  TZWCCImpl::strDesc(const TString& strToSet)
{
    m_strDesc = strToSet;
    return m_strDesc;
}


//
//  The driver calls this on all CC impls that are viable when it hits GetInitVals()
//  to get us to issue a first query. Instead of just calling SendValueQuery() he
//  calls this so that we have more control, which we need.
//
tCIDLib::TVoid TZWCCImpl::SendInitValueQuery()
{
    //
    //  If pollable, we need to treat this like a first poll attempt, to start trying
    //  to get an initial value. Else, we just send out a first query.
    //
    if (m_enctDefPollInt)
    {
        // Set up an initial next poll time in case this initial one fails
        m_c4TOCount = 0;
        m_enctNextPoll = TTime::enctNowPlusSecs(30);
    }

    // And now ask the derived class to send an initial value query
    SendValueQuery();
}


//
//  Set up the type of access we have to our module. In the second version, we jsut
//  set basic read/write. This will not override the optional read flags unless read
//  is disabled. This lets ctors of derived classes just update their read/write
//  access without undoing those other flags. If they want to do that, they have to
//  call the first version.
//
tCIDLib::TVoid
TZWCCImpl::SetAccess(const  tZWaveUSB3Sh::EUnitAcc  eToSet
                    , const tZWaveUSB3Sh::EUnitAcc  eMask)
{
    // Preview what the result is going to be
    tZWaveUSB3Sh::EUnitAcc eNewBits = tCIDLib::eMaskEnumBits(m_eAccess, eToSet, eMask);

    // If any read flags are on, the readable must also be set
    if (tCIDLib::bAnyBitsOn(eNewBits, tZWaveUSB3Sh::EUnitAcc::OptReadFlags)
    &&  !tCIDLib::bAllBitsOn(eNewBits, tZWaveUSB3Sh::EUnitAcc::Read))
    {
        facZWaveUSB3Sh().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kZW3ShErrs::errcCC_OptReadBits
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Config
            , strUnitName()
            , m_strDesc
        );
    }

    // Looks ok, so we can store them. Remember if readable changed first though
    const tCIDLib::TBoolean bReadChanged
    (
        tCIDLib::bBitsChanged(eNewBits, m_eAccess, tZWaveUSB3Sh::EUnitAcc::Read)
    );
    m_eAccess = eNewBits;

    // If readable state changed, update some other bits
    if (bReadChanged)
        ReadableChanged();
}



//
//  Derived classes or the containing unit call this if they know that this CC has
//  entered error state. If that is a change, it is acted on.
//
tCIDLib::TVoid TZWCCImpl::SetErrState()
{
    // If not already in error state, we enter it
    if (!m_bError)
    {
        m_bError = kCIDLib::True;
        if (facZWaveUSB3Sh().bLowTrace())
        {
            facZWaveUSB3Sh().LogTraceMsg
            (
                L"%(1)/%(2)- Entered error state", strUnitName(), m_strDesc
            );
        }

        // Let other stuff in the hierarchy know about this, and then the unit
        EnteredErrState();
        m_punitiOwner->punitThis()->ImplInErrState(m_c4ImplId);
    }
}


//
//  This can be done via the generic SetAccess(), but this is a convenience to handle
//  the fairly common scenario where the defaults set by our ctor are find for read,
//  but the derived class needs to add writeability.
//
tCIDLib::TVoid TZWCCImpl::SetWriteAccess(const tCIDLib::TBoolean bToSet)
{
    if (bToSet)
        m_eAccess = tCIDLib::eOREnumBits(m_eAccess, tZWaveUSB3Sh::EUnitAcc::Write);
    else
        m_eAccess = tCIDLib::eClearEnumBits(m_eAccess, tZWaveUSB3Sh::EUnitAcc::Write);
}


//
//  We got a node info frame when the unit is in Ready state. So if we are in error
//  or timeout state, we will reset the timeout sequence since probably it's now back
//  alive.
//
tCIDLib::TVoid TZWCCImpl::UnitIsOnline()
{
    if (m_bError || m_c4TOCount)
    {
        if (m_enctDefPollInt)
        {
            if (facZWaveUSB3Sh().bHighTrace())
            {
                facZWaveUSB3Sh().LogTraceErr
                (
                    L"%(1)/%(2)- Node info received, resetting poll timeout sequence"
                    , strUnitName()
                    , m_strDesc
                );
            }
            m_enctNextPoll = 0;
            m_c4TOCount = 0;
        }
    }
}


// ---------------------------------------------------------------------------
//  TZWCCImpl: Hidden constructors
// ---------------------------------------------------------------------------

//
//  We assume that the access info is going to be set after creation or be gotten
//  via extra info on our CC. But we try to set reasonable defaults so that we
//  reduce the busy work required by the outside world, so they can just do nothing
//  if the defaults are ok.
//
TZWCCImpl::TZWCCImpl(       TZWUnitInfo* const      punitiOwner
                    , const tZWaveUSB3Sh::ECClasses eClass
                    , const tCIDLib::TCard1         c1EPId) :

    m_bError(kCIDLib::True)
    , m_bSeeAllEPs(kCIDLib::False)
    , m_c1EPId(c1EPId)
    , m_c4ImplId(0)
    , m_c4TOCount(0)
    , m_eClass(eClass)
    , m_eAccess(tZWaveUSB3Sh::EUnitAcc::None)
    , m_enctDefPollInt(0)
    , m_enctNextPoll(0)
    , m_punitiOwner(punitiOwner)
    , m_strDesc(L"??")
{
    //
    //  If our owner is and always on type, then turn on readable as the default.
    //  Else if we support wakeup, then set readable and read on wakeup. This may
    //  not be right, but it is often enough. The derived class and/or unit that
    //  creates the derived class can override as required.
    //
    if (punitiOwner->bAlwaysOn())
    {
        m_eAccess = tZWaveUSB3Sh::EUnitAcc::Read;
    }
    else if (punitiOwner->bSupportsClass(tZWaveUSB3Sh::ECClasses::Wakeup))
    {
        m_eAccess = tZWaveUSB3Sh::EUnitAcc::BatteryRead;
    }

    if (bIsReadable())
        ReadableChanged();
}


// ---------------------------------------------------------------------------
//  TZWCCImpl: Protected, virtual methods
// ---------------------------------------------------------------------------

// A do nothing impl for write only or non-queryable derivatives
tCIDLib::TVoid TZWCCImpl::SendValueQuery()
{
    facZWaveUSB3Sh().LogTraceErr
    (
        L"Unhandled SendValueQuery() called on %(1)/%(2)"
        , strUnitName()
        , m_strDesc
    );
}


// ---------------------------------------------------------------------------
//  TZWCCImpl: Protected, non-virtual methods
// ---------------------------------------------------------------------------

// A passthrough to get the always on info from our unit info
tCIDLib::TBoolean TZWCCImpl::bAlwaysOn() const
{
    return m_punitiOwner->bAlwaysOn();
}


//
//  Get our error state. If we go into error state, we call a virtual to let derived
//  classes know and they call down to us and we let the owning unit know.
//
tCIDLib::TBoolean TZWCCImpl::bError() const
{
    return m_bError;
}


//
//  A helper to find extra data for our class and end point
//
tCIDLib::TBoolean
TZWCCImpl::bFindExtraVal(const TString& strKey, TString& strValue) const
{
    return m_punitiOwner->deviUnit().bFindExtraVal(m_eClass, m_c1EPId, strKey, strValue);
}


// A pasthrough to get the generic text format value of an option
tCIDLib::TBoolean
TZWCCImpl::bFindOptVal(const TString& strKey, TString& strToFill) const
{
    return m_punitiOwner->bFindOptVal(strKey, strToFill);
}


// Get the name of a field given its id
tCIDLib::TBoolean
TZWCCImpl::bQueryFldName(const tCIDLib::TCard4 c4FldId, TString& strToFill) const
{
    return m_punitiOwner->bQueryFldName(c4FldId, strToFill);
}


// A passthrough to access boolean options
tCIDLib::TBoolean
TZWCCImpl::bQueryOptVal(const TString& strKey, const tCIDLib::TBoolean bDefault) const
{
    return m_punitiOwner->bQueryOptVal(strKey, bDefault);
}


// Helpers for our derivatives to store their field values
tCIDLib::TBoolean
TZWCCImpl::bWriteFld(const tCIDLib::TCard4 c4FldId, const tCIDLib::TBoolean bToStore)
{
    return m_punitiOwner->bWriteFld(c4FldId, bToStore);
}

tCIDLib::TBoolean
TZWCCImpl::bWriteFld(const tCIDLib::TCard4 c4FldId, const tCIDLib::TCard4 c4ToStore)
{
    return m_punitiOwner->bWriteFld(c4FldId, c4ToStore);
}

tCIDLib::TBoolean
TZWCCImpl::bWriteFld(const tCIDLib::TCard4 c4FldId, const tCIDLib::TFloat8 f8ToStore)
{
    return m_punitiOwner->bWriteFld(c4FldId, f8ToStore);
}

tCIDLib::TBoolean
TZWCCImpl::bWriteFld(const tCIDLib::TCard4 c4FldId, const tCIDLib::TInt4 i4ToStore)
{
    return m_punitiOwner->bWriteFld(c4FldId, i4ToStore);
}

tCIDLib::TBoolean
TZWCCImpl::bWriteFld(const tCIDLib::TCard4 c4FldId, const TString& strToStore)
{
    return m_punitiOwner->bWriteFld(c4FldId, strToStore);
}


tCIDLib::TCard1 TZWCCImpl::c1UnitId() const
{
    return m_punitiOwner->c1Id();
}


tCIDLib::TCard4 TZWCCImpl::c4LookupFldId(const TString& strFldName) const
{
    return m_punitiOwner->c4FindFldId(strFldName);
}


// A passthrough to access Card4 options
tCIDLib::TCard4
TZWCCImpl::c4QueryOptVal(const TString& strKey, const tCIDLib::TCard4 c4Default) const
{
    return m_punitiOwner->c4QueryOptVal(strKey, c4Default);
}


//
//  Given the access set on us, create the appropriate field access. It's not necessarly
//  the same. The big one is that the CC may be neither readable nor writeable, it's
//  just async notification driven, but clearly the field has to be readable. Otherwise,
//  we return what we have.
//
tCQCKit::EFldAccess TZWCCImpl::eToFldAccess() const
{
    tCQCKit::EFldAccess eRet = tCQCKit::EFldAccess::None;

    if (tCIDLib::bAllBitsOn(m_eAccess, tZWaveUSB3Sh::EUnitAcc::Read))
        eRet = tCIDLib::eOREnumBits(eRet, tCQCKit::EFldAccess::Read);

    if (tCIDLib::bAllBitsOn(m_eAccess, tZWaveUSB3Sh::EUnitAcc::Write))
        eRet = tCIDLib::eOREnumBits(eRet, tCQCKit::EFldAccess::Write);

    //
    //  If neither of the above, then it is a purely notification driven CC, but
    //  the field itself still needs to be readable.
    //
    if (eRet == tCQCKit::EFldAccess::None)
        eRet = tCQCKit::EFldAccess::Read;

    return eRet;
}


// A passthrough to access Card4 options
tCIDLib::TInt4
TZWCCImpl::i4QueryOptVal(const TString& strKey, const tCIDLib::TInt4 i4Default) const
{
    return m_punitiOwner->i4QueryOptVal(strKey, i4Default);
}


// A passthrough to let CC impls send triggres
tCIDLib::TVoid
TZWCCImpl::QueueEventTrig(  const   tCQCKit::EStdDrvEvs     eEvent
                            , const TString&                strFld
                            , const TString&                strVal1
                            , const TString&                strVal2
                            , const TString&                strVal3
                            , const TString&                strVal4)
{
    m_punitiOwner->QueueEventTrig(eEvent, strFld, strVal1, strVal2, strVal3, strVal4);
}


const TString& TZWCCImpl::strDrvMoniker() const
{
    return m_punitiOwner->strDrvMoniker();
}


const TString& TZWCCImpl::strUnitName() const
{
    return m_punitiOwner->strName();
}


//
//  Helpers our derived classes use to send msgs. We end point encap if needed,
//  then pass it on to our owning unit info object.
//
tCIDLib::TVoid TZWCCImpl::SendMsgAsync(TZWOutMsg& zwomToSend)
{
    // End point encapsulate the msg if needed
    if (m_c1EPId != 0xFF)
        zwomToSend.EndPointEncap(0, m_c1EPId);

    m_punitiOwner->SendMsgAsync(zwomToSend);
}


tCIDLib::TVoid TZWCCImpl::SendWaitAck(TZWOutMsg& zwomToSend)
{
    // End point encapsulate the msg if needed
    if (m_c1EPId != 0xFF)
        zwomToSend.EndPointEncap(0, m_c1EPId);

    m_punitiOwner->SendWaitAck(zwomToSend);
}


tCIDLib::TVoid TZWCCImpl::SetFieldErr(const tCIDLib::TCard4 c4FldId)
{
    m_punitiOwner->SetFldError(c4FldId);
}



// ---------------------------------------------------------------------------
//  TZWCCImpl: Protected, virtual methods
// ---------------------------------------------------------------------------

// Many don't need to see this, since they were the ones that set the state
tCIDLib::TVoid TZWCCImpl::EnteredErrState()
{
}


// Many don't need to see this, since they were the ones that set the value
tCIDLib::TVoid
TZWCCImpl::ValueChanged(const   tZWaveUSB3Sh::EValSrcs
                        , const tCIDLib::TBoolean
                        , const tCIDLib::TInt4)
{
}


// ---------------------------------------------------------------------------
//  TZWCCImpl: Protected, non-virtual methods
// ---------------------------------------------------------------------------

//
//  If no poll interval set, see if we could have one. If so, set a default
//  that will be used if not overridden later.
//
//  If we have a poll interface see if we need to display polling.
//
tCIDLib::TVoid TZWCCImpl::ReadableChanged()
{
    const tCIDLib::TBoolean bReadable
    (
        tCIDLib::bAllBitsOn(m_eAccess, tZWaveUSB3Sh::EUnitAcc::Read)
    );

    if (!m_enctDefPollInt)
    {
        if (bReadable && m_punitiOwner->bAlwaysOn())
        {
            CalcDefPollInt();
            m_enctNextPoll = TTime::enctNow();
        }
    }
     else if (!bReadable)
    {
        m_enctDefPollInt = 0;
    }
}


tCIDLib::TVoid
TZWCCImpl::ReportValChange( const   tZWaveUSB3Sh::EValSrcs  eSource
                            , const tCIDLib::TInt4          i4ExtraInfo)
{
    //  Make sure we are not in error state now, but remember if we were
    tCIDLib::TBoolean bWasInErr = m_bError;
    m_bError = kCIDLib::False;

    if (bWasInErr)
    {
        facZWaveUSB3Sh().LogTraceMsg
        (
            L"%(1)/%(2)- Transitioning out of error state"
            , strUnitName()
            , m_strDesc
        );
    }

    // Let derived classes and the unit know about this
    ValueChanged(eSource, bWasInErr, i4ExtraInfo);
    m_punitiOwner->punitThis()->ImplValueChanged
    (
        m_c4ImplId, eSource, bWasInErr, i4ExtraInfo
    );
}


//
//  Let derived classes change the default poll interval. Zero will disable it, though
//  there is a specialized method for that that would be more readable. We don't allow
//  anything less than 2 minutes if it's non-zero.
//
tCIDLib::TVoid TZWCCImpl::SetDefPollMinutes(const tCIDLib::TCard4 c4Minutes)
{
    if (!c4Minutes)
        m_enctDefPollInt = 0;
    else if (c4Minutes < 2)
        m_enctDefPollInt = kCIDLib::enctOneMinute * 2;
    else
        m_enctDefPollInt = kCIDLib::enctOneMinute * c4Minutes;
}


// ---------------------------------------------------------------------------
//  TZWCCImpl: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  This is used to set our default initial poll interval, which we want to randomize
//  a bit. If we get poll failures we crank it up to higher levels to stop wasting time
//  talking to an unresponsive unit, and to limit battery suckage.
//
tCIDLib::TVoid TZWCCImpl::CalcDefPollInt()
{
    // Should be called for non-listener, but be careful
    if (!bAlwaysOn())
    {
        m_enctDefPollInt = 0;
        return;
    }

    // Start with a default minutes
    if (m_punitiOwner->punitThis()->bFreqListener())
        m_enctDefPollInt = 60;
    else
        m_enctDefPollInt = 10;

    // Now multiply the minutes to get the poll interval
    m_enctDefPollInt *= kCIDLib::enctOneMinute;

    // And randomlize by 1/8 of that
    const tCIDLib::TCard8 c8RandRange = m_enctDefPollInt / 8;
    const tCIDLib::TCard8 c8HalfRange = c8RandRange / 2;

    const tCIDLib::TCard8 c8RandOfs = facCIDCrypto().c8GetRandom() % c8RandRange;
    const tCIDLib::TCard8 c8HalfOfs = c8RandOfs / 2;

    // If the offset is larger than half the range, add half of it, else sub half of it
    if (c8RandOfs >= c8HalfRange)
        m_enctDefPollInt += c8HalfOfs;
    else
        m_enctDefPollInt -= c8HalfOfs;
}



// ---------------------------------------------------------------------------
//   CLASS: TZWCCImplBin
//  PREFIX: zwcci
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWCCImplBin: Public constructors and Destructor
// ---------------------------------------------------------------------------
TZWCCImplBin::~TZWCCImplBin()
{
}


// ---------------------------------------------------------------------------
//  TZWCCImplBin: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Provide access to our current state
tCIDLib::TBoolean TZWCCImplBin::bState() const
{
    return m_bState;
}


//
//  Sets a new state on us. We return true if the new value is different from the old
//  value, or if we were in error state before, i.e. if a reportable change in state
//  occurred. They tell us if it is from the unit itself or an outgoing change from a
//  field write.
//
tCIDLib::TBoolean
TZWCCImplBin::bSetBinState( const   tCIDLib::TBoolean       bNewValue
                            , const tZWaveUSB3Sh::EValSrcs  eSrc
                            , const tCIDLib::TInt4          i4ExtraInfo)
{
    if (bError() || (bNewValue != m_bState))
    {
        m_bState = bNewValue;

        // Let everyone in the hierarchy know about the change
        ReportValChange(eSrc, i4ExtraInfo);
        return kCIDLib::True;
    }

    // No change to report
    return kCIDLib::False;
}


// ---------------------------------------------------------------------------
//  TZWCCImplBin: Hidden constructors
// ---------------------------------------------------------------------------

TZWCCImplBin::TZWCCImplBin(         TZWUnitInfo* const      punitiOwner
                            , const tZWaveUSB3Sh::ECClasses eClass
                            , const tCIDLib::TCard1         c1EPId) :

    TZWCCImpl(punitiOwner, eClass, c1EPId)
    , m_bState(kCIDLib::False)
{
}



// ---------------------------------------------------------------------------
//   CLASS: TZWCCImplLevel
//  PREFIX: zwcci
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWCCImplLevel: Public constructors and Destructor
// ---------------------------------------------------------------------------
TZWCCImplLevel::~TZWCCImplLevel()
{
}

// ---------------------------------------------------------------------------
//  TZWCCImplLevel: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Provide access to our current level
tCIDLib::TCard4 TZWCCImplLevel::c4Level() const
{
    return m_c4Level;
}


//
//  Sets a new level on us. If the new level is different from the old level, or we
//  were in error state before, we consider this a change and return true. Else we
//  return false. If there is a change we call our level changed virtual as well.
//
//  They tell us if it is from the unit or outgoing from the driver via field write
//  and we pass that on.
//
tCIDLib::TBoolean
TZWCCImplLevel::bSetLevel(  const   tCIDLib::TCard4         c4New
                            , const tZWaveUSB3Sh::EValSrcs  eSrc
                            , const tCIDLib::TInt4          i4ExtraInfo)
{
    if (bError() || (c4New != m_c4Level))
    {
        m_c4Level = c4New;

        // Report this change to everyone
        ReportValChange(eSrc, i4ExtraInfo);
        return kCIDLib::True;
    }

    // No change to report
    return kCIDLib::False;
}


// ---------------------------------------------------------------------------
//  TZWCCImplLevel: Hidden constructors
// ---------------------------------------------------------------------------
TZWCCImplLevel::TZWCCImplLevel(         TZWUnitInfo* const      punitiOwner
                                , const tZWaveUSB3Sh::ECClasses eClass
                                , const tCIDLib::TCard1         c1EPId) :

    TZWCCImpl(punitiOwner, eClass, c1EPId)
    , m_c4Level(0)
{
}

