//
// FILE NAME: CQCKit_FldStorage.cpp
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
//  This is the implementation file for our family of field storage classes.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCKit_.hpp"


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace CQCKit_DriverObjCfg
{
    namespace
    {
        // -----------------------------------------------------------------------
        //  For version for the event trigger class
        //
        //  Version 2 -
        //      Added latching capability.
        //
        //  Version 3 -
        //      Expanded latching to support bi-directional latching.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2EvTrFmtVersion = 3;
    }
}


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCFldEvTrigger,TObject)
RTTIDecls(TCQCFldStore,TObject)
RTTIDecls(TCQCFldStoreBool,TCQCFldStore)
RTTIDecls(TCQCFldStoreCard,TCQCFldStore)
RTTIDecls(TCQCFldStoreFloat,TCQCFldStore)
RTTIDecls(TCQCFldStoreInt,TCQCFldStore)
RTTIDecls(TCQCFldStoreString,TCQCFldStore)
RTTIDecls(TCQCFldStoreSList,TCQCFldStore)
RTTIDecls(TCQCFldStoreTime,TCQCFldStore)



// ---------------------------------------------------------------------------
//   CLASS: TCQCFldEvTrigger
//  PREFIX: fet
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCFldEvTrigger: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCFldEvTrigger::TCQCFldEvTrigger() :

    m_eLastState(tCQCKit::EEvTrLatchSts::First)
    , m_eLatching(tCQCKit::EEvTrLatches::Unlatched)
    , m_eType(tCQCKit::EEvTrTypes::Unused)
    , m_exprTrigger()
{
}

TCQCFldEvTrigger::TCQCFldEvTrigger(const TString& strFldName) :

    m_eLastState(tCQCKit::EEvTrLatchSts::First)
    , m_eLatching(tCQCKit::EEvTrLatches::Unlatched)
    , m_eType(tCQCKit::EEvTrTypes::Unused)
    , m_strFldName(strFldName)
{
}

TCQCFldEvTrigger::TCQCFldEvTrigger( const   TString&            strFldName
                                    , const TCQCExpression&     exprTrigger) :
    m_eLastState(tCQCKit::EEvTrLatchSts::First)
    , m_eLatching(tCQCKit::EEvTrLatches::Unlatched)
    , m_eType(tCQCKit::EEvTrTypes::OnExpression)
    , m_exprTrigger(exprTrigger)
    , m_strFldName(strFldName)
{
}

TCQCFldEvTrigger::~TCQCFldEvTrigger()
{
}


// ---------------------------------------------------------------------------
//  TCQCFldEvTrigger: Public operators
// ---------------------------------------------------------------------------

// WE DO NOT update the last latching state!
TCQCFldEvTrigger& TCQCFldEvTrigger::operator=(const TCQCFldEvTrigger& fetSrc)
{
    if (this != &fetSrc)
    {
        m_eLatching     = fetSrc.m_eLatching;
        m_eType         = fetSrc.m_eType;
        m_exprTrigger   = fetSrc.m_exprTrigger;
        m_strFldName    = fetSrc.m_strFldName;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCFldEvTrigger: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Evaluate our expression based on the passed value and return the result.
//  Note that we don't necessarily return the real result if we are a latching
//  trigger. We only return true if the latching state changes.
//
//  NOTE that we also keep the latching state up to date, even if we aren't
//  configured for latching. This way, if a trigger is added to the field, we will
//  be in the right state and the next transition will work correctly.
//
tCIDLib::TBoolean TCQCFldEvTrigger::bEvaluate(const TCQCFldValue& fvSrc)
{
    CIDAssert
    (
        m_eType == tCQCKit::EEvTrTypes::OnExpression
        , L"bEvalute can't be called on non-expression field trigger"
    );

    //
    //  Evaluate and get a pass/fail result. We don't support dynamic comparison
    //  values via replacement tokens here, so we pass in a null pointer for the
    //  global variables target.
    //
    tCIDLib::TBoolean bRes = m_exprTrigger.bEvaluate(fvSrc, nullptr);

    if (bRes)
    {
        // The expression passed. Deal with latching if enabled.
        if (m_eLatching != tCQCKit::EEvTrLatches::Unlatched)
        {
            if (m_eLastState == tCQCKit::EEvTrLatchSts::First)
            {
                // We always eat the first one, to get a first state
                bRes = kCIDLib::False;
            }
             else if ((m_eLatching == tCQCKit::EEvTrLatches::Unidirectional)
                  ||  (m_eLatching == tCQCKit::EEvTrLatches::Bidirectional))
            {
                //
                //  For either of these, if the previous state was not false, then
                //  there's no transition and we do nothing.
                //
                if (m_eLastState != tCQCKit::EEvTrLatchSts::False)
                    bRes = kCIDLib::False;
            }
             else if (m_eLatching == tCQCKit::EEvTrLatches::Bidirectional)
            {
                // If it's the same as last time we pretend nothing happened
                if (m_eLastState == tCQCKit::EEvTrLatchSts::True)
                    bRes = kCIDLib::False;
            }
        }

        // And remember we were last in true state
        m_eLastState = tCQCKit::EEvTrLatchSts::True;
    }
     else
    {
        if ((m_eLastState == tCQCKit::EEvTrLatchSts::First)
        ||  (m_eLatching == tCQCKit::EEvTrLatches::Unidirectional))
        {
            //
            //  We just eat this one. If unidirectional, we don't report transitions
            //  to false, only to true, so doesn't matter if it changed or not. If the
            //  first one, we always eat it.
            //
            bRes = kCIDLib::False;
        }
         else if (m_eLatching == tCQCKit::EEvTrLatches::Bidirectional)
        {
            //
            //  If the previous state was true, then we force an event, else there
            //  was no transition.
            //
            if (m_eLastState == tCQCKit::EEvTrLatchSts::True)
                bRes = kCIDLib::True;
        }

        // And remember we were last in false state
        m_eLastState = tCQCKit::EEvTrLatchSts::False;
    }
    return bRes;
}


// Get or set the latching type, for the configuration dailog
tCQCKit::EEvTrLatches TCQCFldEvTrigger::eLatching() const
{
    return m_eLatching;
}

tCQCKit::EEvTrLatches TCQCFldEvTrigger::eLatching(const tCQCKit::EEvTrLatches eToSet)
{
    m_eLatching = eToSet;
    return m_eLatching;
}


//
//  Get or set the trigger type
//
//  If setting a new type that is not an expression type, then make sure that latching
//  is disabled. NOTE THAT we do not reset the latching state here. It's been kept
//  correct even if latching is disabled.
//
tCQCKit::EEvTrTypes TCQCFldEvTrigger::eType() const
{
    return m_eType;
}

tCQCKit::EEvTrTypes TCQCFldEvTrigger::eType(const tCQCKit::EEvTrTypes eToSet)
{
    m_eType = eToSet;

    if (m_eType != tCQCKit::EEvTrTypes::OnExpression)
        m_eLatching = tCQCKit::EEvTrLatches::Unlatched;

    return m_eType;
}


// Provide access to the expressions, for the configuration dialog mainly
TCQCExpression& TCQCFldEvTrigger::exprTrigger()
{
    return m_exprTrigger;
}

const TCQCExpression& TCQCFldEvTrigger::exprTrigger() const
{
    return m_exprTrigger;
}

const TCQCExpression&
TCQCFldEvTrigger::exprTrigger(const TCQCExpression& exprToSet)
{
    m_exprTrigger = exprToSet;
    return m_exprTrigger;
}


//
//  Get or set the name of the field this trigger is for. We get the field
//  value to evaluate directly from the caller, so we don't have to look up
//  the field or anything. But we have to include the field name in the
//  trigger that goes out.
//
const TString& TCQCFldEvTrigger::strFldName() const
{
    return m_strFldName;
}

const TString& TCQCFldEvTrigger::strFldName(const TString& strToSet)
{
    m_strFldName = strToSet;
    return m_strFldName;
}


// ---------------------------------------------------------------------------
//  TCQCFldEvTrigger: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCFldEvTrigger::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a start object marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check our version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCKit_DriverObjCfg::c2EvTrFmtVersion))
    {
        facCIDLib().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCIDErrs::errcGen_UnknownFmtVersion
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(c2FmtVersion)
            , clsThis()
        );
    }

    // And now do our stuff
    strmToReadFrom  >> m_strFldName
                    >> m_eType
                    >> m_exprTrigger;

    //
    //  If V2 or beyond, read in the latching stuff, else default it. If V3 or beyond
    //  we converted from a boolean latch to an enum so we need to convert.
    //
    if (c2FmtVersion == 1)
    {
        m_eLatching = tCQCKit::EEvTrLatches::Unlatched;
    }
     else if (c2FmtVersion == 2)
    {
        tCIDLib::TBoolean bLatch;
        strmToReadFrom >> bLatch;
        if (bLatch)
            m_eLatching = tCQCKit::EEvTrLatches::Unidirectional;
        else
            m_eLatching = tCQCKit::EEvTrLatches::Unlatched;
    }
     else
    {
        strmToReadFrom >> m_eLatching;
    }

    // And should end with an end object marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);

    // Initially any runtime stuff
    m_eLastState = tCQCKit::EEvTrLatchSts::First;
}


tCIDLib::TVoid TCQCFldEvTrigger::StreamTo(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCKit_DriverObjCfg::c2EvTrFmtVersion
                    << m_strFldName
                    << m_eType
                    << m_exprTrigger

                       // V2 stuff
                    << m_eLatching

                    << tCIDLib::EStreamMarkers::EndObject;
}




// ---------------------------------------------------------------------------
//   CLASS: TCQCFldStore
//  PREFIX: cfs
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  Public, static methods
// ---------------------------------------------------------------------------
const TString& TCQCFldStore::strKey(const TCQCFldStore& cfsSrc)
{
    return cfsSrc.flddInfo().strName();
}


// ---------------------------------------------------------------------------
//  TCQCFldStore: Destructor
// ---------------------------------------------------------------------------
TCQCFldStore::~TCQCFldStore()
{
}


// ---------------------------------------------------------------------------
//  TCQCFldStore: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TCQCFldStore::bAlwaysWrite() const
{
    return m_flddInfo.bAlwaysWrite();
}

tCIDLib::TBoolean TCQCFldStore::bAlwaysWrite(const tCIDLib::TBoolean bToSet)
{
    return m_flddInfo.bAlwaysWrite(bToSet);
}


tCIDLib::TBoolean TCQCFldStore::bInError() const
{
    return m_fvThis.bInError();
}

tCIDLib::TBoolean TCQCFldStore::bInError(const tCIDLib::TBoolean bToSet)
{
    m_fvThis.bInError(bToSet);
    return bToSet;
}


//
//  A generic value setter, for when the caller isn't aware of the actual
//  type necessarily.
//
tCIDLib::TBoolean TCQCFldStore::bSetValue(const TCQCFldValue& fvToSet)
{
    return m_fvThis.bPolyValueCopy(fvToSet);
}


//
//  A convenience used by the device driver base class. Normally we won't
//  call the driver for a field write if the value being written is the
//  same as the currnet field value. But, if the field is always write, or
//  it's write only, then we always write the value. So the driver needs
//  to make this check all the time.
//
tCIDLib::TBoolean TCQCFldStore::bShouldSendCurVal() const
{
    return  m_flddInfo.bAlwaysWrite()
            || (m_flddInfo.eAccess() == tCQCKit::EFldAccess::Write);
}


// Give access to the serial number
tCIDLib::TCard4 TCQCFldStore::c4SerialNum() const
{
    // Just pass it on to the field value class holds the serial number
    return m_fvThis.c4SerialNum();
}


//
//  Note that the assignment of the trigger won't update the latching state! This
//  helps insure that updating the trigger won't mess up the currently stored latching
//  state and stop the next transition possibly from causing a trigger. The latching
//  state is purely run time stuff so it doesn't need to be assigned from the source
//  anyway, so this isn't an issue.
//
TCQCFldEvTrigger& TCQCFldStore::fetTrigger()
{
    return m_fetTrigger;
}

const TCQCFldEvTrigger& TCQCFldStore::fetTrigger() const
{
    return m_fetTrigger;
}

const TCQCFldEvTrigger& TCQCFldStore::fetTrigger(const TCQCFldEvTrigger& fetToSet)
{
    m_fetTrigger = fetToSet;
    return m_fetTrigger;
}


const TCQCFldDef& TCQCFldStore::flddInfo() const
{
    return m_flddInfo;
}


//
//  Provide access to the field value ref we have. We can only provide
//  the base class view of it here.
//
const TCQCFldValue& TCQCFldStore::fvThis() const
{
    return m_fvThis;
}

TCQCFldValue& TCQCFldStore::fvThis()
{
    return m_fvThis;
}


const TString& TCQCFldStore::strMoniker() const
{
    return m_strMoniker;
}


//
//  The driver will call this if we report to him our value changed and he thinks
//  we might need to send a field change trigger.
//
//  He tells us if the field was in error and has gotten his first valid value.
//  If so, we won't send a trigger, but we'll do the evaluation of the value so
//  that we are ready for the next one to send a trigger if appropriate. Mostly this
//  is for latching triggers.
//
tCIDLib::TVoid TCQCFldStore::SendFldChangeTrig(const tCIDLib::TBoolean  bWasInError)
{
    //
    //  If we send on any change, then just send. If we send on expression,
    //  then do the expression evaluation.
    //
    //  Note that, if an expression, we may not get back the real result
    //  if the trigger is a latching type, since it will suppress multiple
    //  sequential true results. See the trigger class for details.
    //
    tCIDLib::TBoolean bSend = kCIDLib::False;
    if (m_fetTrigger.eType() == tCQCKit::EEvTrTypes::OnChange)
        bSend = kCIDLib::True;
    else if (m_fetTrigger.eType() == tCQCKit::EEvTrTypes::OnExpression)
        bSend = m_fetTrigger.bEvaluate(m_fvThis);

    //
    //  Even if we could send, don't if we are getting our first valid value after
    //  startup or having been in error state.
    //
    if (bSend && !bWasInError)
    {
        //
        //  Ok, we gotta send one. Ask the field value object to format itself, since
        //  we have to pass the new value in the event.
        //
        TString strVal;
        m_fvThis.Format(strVal);

        // If the value is very long, we cap it and add a truncation indicator
        TString strVal2;
        if (strVal.c4Length() > 64)
        {
            strVal.CapAt(64);
            strVal2 = L"yes";
        }

        facCQCKit().QueueStdEventTrig
        (
            tCQCKit::EStdDrvEvs::FldChange
            , m_strMoniker
            , m_flddInfo.strName()
            , strVal
            , strVal2
            , TString::strEmpty()
            , TString::strEmpty()
        );
    }
}


//
//  We can do these on behalf of all our derivatives because we can just
//  pass it on to the value object, to which we have a ref. This is not
//  the standard streaming scheme, but just to stream the actual value
//  in/out.
//
tCIDLib::TVoid TCQCFldStore::StreamIn(TBinInStream& strmSrc)
{
    m_fvThis.StreamInValue(strmSrc);
}

tCIDLib::TVoid TCQCFldStore::StreamOut(TBinOutStream& strmTarget) const
{
    m_fvThis.StreamOutValue(strmTarget);
}


// ---------------------------------------------------------------------------
//  TCQCFldStore: Hidden Constructors
// ---------------------------------------------------------------------------
TCQCFldStore::TCQCFldStore( const   TString&        strMoniker
                            , const TCQCFldDef&     flddInfo
                            ,       TCQCFldValue&   fvThis) :

    m_flddInfo(flddInfo)
    , m_fvThis(fvThis)
    , m_strMoniker(strMoniker)
{
}


// ---------------------------------------------------------------------------
//   CLASS: TCQCFldStoreBool
//  PREFIX: fldd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCFldStoreBool: Public Destructor
// ---------------------------------------------------------------------------
TCQCFldStoreBool::TCQCFldStoreBool( const   TString&                strMoniker
                                    , const TCQCFldDef&             flddInfo
                                    ,       TCQCFldBoolLimit* const pfldlToAdopt) :

    TCQCFldStore(strMoniker, flddInfo, m_fvCur)
    , m_fvCur(0, 0)
    , m_pfldlLimits(pfldlToAdopt)
{
}

TCQCFldStoreBool::~TCQCFldStoreBool()
{
    delete m_pfldlLimits;
}


// ---------------------------------------------------------------------------
//  TCQCFldStoreBool: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TCQCFldStoreBool::FormatValue(TString& strToFill, TTextStringOutStream&) const
{
    m_fvCur.Format(strToFill);
}

const TCQCFldLimit* TCQCFldStoreBool::pfldlLimits() const
{
    return m_pfldlLimits;
}


// ---------------------------------------------------------------------------
//  TCQCFldStoreBool: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TCQCFldStoreBool::bValue() const
{
    return m_fvCur.bValue();
}


tCQCKit::EValResults
TCQCFldStoreBool::eValidate(const tCIDLib::TBoolean bToTest)
{
    if (m_fvCur.bValue() == bToTest)
        return tCQCKit::EValResults::Unchanged;
    return m_pfldlLimits->eValidate(bToTest);
}

tCQCKit::EValResults
TCQCFldStoreBool::eValidate(const   TString&            strValue
                            ,       tCIDLib::TBoolean&  bToFill)
{
    tCQCKit::EValResults eRes = m_pfldlLimits->eValidate(strValue, bToFill);
    if ((eRes == tCQCKit::EValResults::OK) && (bToFill == m_fvCur.bValue()))
        eRes = tCQCKit::EValResults::Unchanged;
    return eRes;
}


tCIDLib::TVoid TCQCFldStoreBool::SetValue(const tCIDLib::TBoolean bToSet)
{
    m_fvCur.bSetValue(bToSet);
}




// ---------------------------------------------------------------------------
//   CLASS: TCQCFldStoreCard
//  PREFIX: fldd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCFldStoreCard: Public Destructor
// ---------------------------------------------------------------------------
TCQCFldStoreCard::TCQCFldStoreCard( const   TString&                strMoniker
                                    , const TCQCFldDef&             flddInfo
                                    ,       TCQCFldCardLimit* const pfldlToAdopt) :

    TCQCFldStore(strMoniker, flddInfo, m_fvCur)
    , m_fvCur(0, 0)
    , m_pfldlLimits(pfldlToAdopt)
{
}

TCQCFldStoreCard::~TCQCFldStoreCard()
{
    delete m_pfldlLimits;
}


// ---------------------------------------------------------------------------
//  TCQCFldStoreCard: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TCQCFldStoreCard::FormatValue(TString& strToFill, TTextStringOutStream&) const
{
    m_fvCur.Format(strToFill);
}

const TCQCFldLimit* TCQCFldStoreCard::pfldlLimits() const
{
    return m_pfldlLimits;
}


// ---------------------------------------------------------------------------
//  TCQCFldStoreCard: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TCard4 TCQCFldStoreCard::c4Value() const
{
    return m_fvCur.c4Value();
}


tCQCKit::EValResults TCQCFldStoreCard::eValidate(const tCIDLib::TCard4 c4ToTest)
{
    if (m_fvCur.c4Value() == c4ToTest)
        return tCQCKit::EValResults::Unchanged;
    return m_pfldlLimits->eValidate(c4ToTest);
}

tCQCKit::EValResults
TCQCFldStoreCard::eValidate(const   TString&            strValue
                            ,       tCIDLib::TCard4&    c4ToFill)
{
    tCQCKit::EValResults eRes = m_pfldlLimits->eValidate(strValue, c4ToFill);
    if ((eRes == tCQCKit::EValResults::OK) && (c4ToFill == m_fvCur.c4Value()))
        eRes = tCQCKit::EValResults::Unchanged;
    return eRes;
}


tCIDLib::TVoid TCQCFldStoreCard::Inc()
{
    m_fvCur.Inc();
}


tCIDLib::TVoid TCQCFldStoreCard::SetValue(const tCIDLib::TCard4 c4ToSet)
{
    m_fvCur.bSetValue(c4ToSet);
}




// ---------------------------------------------------------------------------
//   CLASS: TCQCFldStoreFloat
//  PREFIX: fldd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCFldStoreFloat: Public Destructor
// ---------------------------------------------------------------------------
TCQCFldStoreFloat::TCQCFldStoreFloat(const  TString&                 strMoniker
                                    , const TCQCFldDef&              flddInfo
                                    ,       TCQCFldFloatLimit* const pfldlToAdopt) :
    TCQCFldStore(strMoniker, flddInfo, m_fvCur)
    , m_fvCur(0, 0)
    , m_pfldlLimits(pfldlToAdopt)
{
}

TCQCFldStoreFloat::~TCQCFldStoreFloat()
{
    delete m_pfldlLimits;
}


// ---------------------------------------------------------------------------
//  TCQCFldStoreFloat: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TCQCFldStoreFloat::FormatValue(TString& strToFill, TTextStringOutStream&) const
{
    m_fvCur.Format(strToFill);
}

const TCQCFldLimit* TCQCFldStoreFloat::pfldlLimits() const
{
    return m_pfldlLimits;
}


// ---------------------------------------------------------------------------
//  TCQCFldStoreFloat: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCQCKit::EValResults
TCQCFldStoreFloat::eValidate(const tCIDLib::TFloat8 f8ToTest)
{
    if (m_fvCur.f8Value() == f8ToTest)
        return tCQCKit::EValResults::Unchanged;
    return m_pfldlLimits->eValidate(f8ToTest);
}

tCQCKit::EValResults
TCQCFldStoreFloat::eValidate(const  TString&            strValue
                            ,       tCIDLib::TFloat8&   f8ToFill)
{
    tCQCKit::EValResults eRes = m_pfldlLimits->eValidate(strValue, f8ToFill);
    if ((eRes == tCQCKit::EValResults::OK) && (f8ToFill == m_fvCur.f8Value()))
        eRes = tCQCKit::EValResults::Unchanged;
    return eRes;
}


tCIDLib::TFloat8 TCQCFldStoreFloat::f8Value() const
{
    return m_fvCur.f8Value();
}


tCIDLib::TVoid TCQCFldStoreFloat::SetValue(const tCIDLib::TFloat8 f8ToSet)
{
    m_fvCur.bSetValue(f8ToSet);
}




// ---------------------------------------------------------------------------
//   CLASS: TCQCFldStoreInt
//  PREFIX: fldd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCFldStoreInt: Public Destructor
// ---------------------------------------------------------------------------
TCQCFldStoreInt::TCQCFldStoreInt(const  TString&                strMoniker
                                , const TCQCFldDef&             flddInfo
                                ,       TCQCFldIntLimit* const  pfldlToAdopt) :
    TCQCFldStore(strMoniker, flddInfo, m_fvCur)
    , m_fvCur(0, 0)
    , m_pfldlLimits(pfldlToAdopt)
{
}

TCQCFldStoreInt::~TCQCFldStoreInt()
{
    delete m_pfldlLimits;
}


// ---------------------------------------------------------------------------
//  TCQCFldStoreInt: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TCQCFldStoreInt::FormatValue(TString& strToFill, TTextStringOutStream&) const
{
    m_fvCur.Format(strToFill);
}

const TCQCFldLimit* TCQCFldStoreInt::pfldlLimits() const
{
    return m_pfldlLimits;
}


// ---------------------------------------------------------------------------
//  TCQCFldStoreInt: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCQCKit::EValResults TCQCFldStoreInt::eValidate(const tCIDLib::TInt4 i4ToTest)
{
    if (m_fvCur.i4Value() == i4ToTest)
        return tCQCKit::EValResults::Unchanged;
    return m_pfldlLimits->eValidate(i4ToTest);
}

tCQCKit::EValResults
TCQCFldStoreInt::eValidate( const   TString&        strValue
                            ,       tCIDLib::TInt4& i4ToFill)
{
    tCQCKit::EValResults eRes = m_pfldlLimits->eValidate(strValue, i4ToFill);
    if ((eRes == tCQCKit::EValResults::OK) && (i4ToFill == m_fvCur.i4Value()))
        eRes = tCQCKit::EValResults::Unchanged;
    return eRes;
}


tCIDLib::TInt4 TCQCFldStoreInt::i4Value() const
{
    return m_fvCur.i4Value();
}


tCIDLib::TVoid TCQCFldStoreInt::SetValue(const tCIDLib::TInt4 i4ToSet)
{
    m_fvCur.bSetValue(i4ToSet);
}




// ---------------------------------------------------------------------------
//   CLASS: TCQCFldStoreString
//  PREFIX: fldd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCFldStoreString: Public Destructor
// ---------------------------------------------------------------------------
TCQCFldStoreString::TCQCFldStoreString( const   TString&               strMoniker
                                        , const TCQCFldDef&            flddInfo
                                        ,       TCQCFldStrLimit* const pfldlToAdopt) :

    TCQCFldStore(strMoniker, flddInfo, m_fvCur)
    , m_fvCur(0, 0)
    , m_pfldlLimits(pfldlToAdopt)
{
}

TCQCFldStoreString::~TCQCFldStoreString()
{
    delete m_pfldlLimits;
}


// ---------------------------------------------------------------------------
//  TCQCFldStoreString: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TCQCFldStoreString::FormatValue(TString& strToFill, TTextStringOutStream&) const
{
    strToFill = m_fvCur.strValue();
}

const TCQCFldLimit* TCQCFldStoreString::pfldlLimits() const
{
    return m_pfldlLimits;
}


// ---------------------------------------------------------------------------
//  TCQCFldStoreString: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCQCKit::EValResults TCQCFldStoreString::eValidate(const TString& strToTest)
{
    if (m_fvCur.strValue() == strToTest)
        return tCQCKit::EValResults::Unchanged;
    return m_pfldlLimits->eValidate(strToTest);
}


const TString& TCQCFldStoreString::strValue() const
{
    return m_fvCur.strValue();
}


tCIDLib::TVoid TCQCFldStoreString::SetValue(const TString& strToSet)
{
    m_fvCur.bSetValue(strToSet);
}




// ---------------------------------------------------------------------------
//   CLASS: TCQCFldStoreSList
//  PREFIX: fldd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCFldStoreSList: Public Destructor
// ---------------------------------------------------------------------------
TCQCFldStoreSList::TCQCFldStoreSList(const  TString&                   strMoniker
                                    , const TCQCFldDef&                flddInfo
                                    ,       TCQCFldStrListLimit* const pfldlToAdopt) :

    TCQCFldStore(strMoniker, flddInfo, m_fvCur)
    , m_fvCur(0, 0)
    , m_pfldlLimits(pfldlToAdopt)
{
}

TCQCFldStoreSList::~TCQCFldStoreSList()
{
    delete m_pfldlLimits;
}


// ---------------------------------------------------------------------------
//  TCQCFldStoreSList: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TCQCFldStoreSList::FormatValue(TString& strToFill, TTextStringOutStream&) const
{
    //
    //  Format our strings into the format that we expect to see, which is a
    //  comma separated list of quoted values.
    //
    strToFill.Clear();
    const TVector<TString>& colList = m_fvCur.colValue();
    const tCIDLib::TCard4 c4Count = colList.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (c4Index)
            strToFill.Append(L",");
        strToFill.Append(kCIDLib::chQuotation);
        strToFill.Append(colList[c4Index]);
        strToFill.Append(kCIDLib::chQuotation);
    }
}

const TCQCFldLimit* TCQCFldStoreSList::pfldlLimits() const
{
    return m_pfldlLimits;
}


// ---------------------------------------------------------------------------
//  TCQCFldStoreSList: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TCQCFldStoreSList::bClearList()
{
    return m_fvCur.bClearList();
}


const TVector<TString>& TCQCFldStoreSList::colValue() const
{
    return m_fvCur.colValue();
}


tCQCKit::EValResults
TCQCFldStoreSList::eValidate(const TVector<TString>& colToTest)
{
    if (bCompare(colToTest))
        return tCQCKit::EValResults::Unchanged;
    return m_pfldlLimits->eValidate(colToTest);
}

tCQCKit::EValResults
TCQCFldStoreSList::eValidate(const  TString&            strValue
                            ,       TVector<TString>&   colToFill)
{
    tCQCKit::EValResults eRes = m_pfldlLimits->eValidate(strValue, colToFill);
    if ((eRes == tCQCKit::EValResults::OK) && bCompare(colToFill))
        eRes = tCQCKit::EValResults::Unchanged;
    return eRes;
}



tCIDLib::TVoid TCQCFldStoreSList::SetValue(const TVector<TString>& colToSet)
{
    m_fvCur.bSetValue(colToSet);
}


// ---------------------------------------------------------------------------
//  TCQCFldStoreSList: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TCQCFldStoreSList::bCompare(const TVector<TString>& colToComp)
{
    const TVector<TString>& colList = m_fvCur.colValue();
    const tCIDLib::TCard4 c4Count = colList.c4ElemCount();
    if (c4Count != colToComp.c4ElemCount())
        return kCIDLib::False;

    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (colList[c4Index] != colToComp[c4Index])
            return kCIDLib::False;
    }
    return kCIDLib::True;
}




// ---------------------------------------------------------------------------
//   CLASS: TCQCFldStoreTime
//  PREFIX: fldd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCFldStoreTime: Public Destructor
// ---------------------------------------------------------------------------
TCQCFldStoreTime::TCQCFldStoreTime( const   TString&                strMoniker
                                    , const TCQCFldDef&             flddInfo
                                    ,       TCQCFldTimeLimit* const pfldlToAdopt) :

    TCQCFldStore(strMoniker, flddInfo, m_fvCur)
    , m_fvCur(0, 0)
    , m_pfldlLimits(pfldlToAdopt)
{
}

TCQCFldStoreTime::~TCQCFldStoreTime()
{
    delete m_pfldlLimits;
}


// ---------------------------------------------------------------------------
//  TCQCFldStoreTime: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid
TCQCFldStoreTime::FormatValue(TString& strToFill, TTextStringOutStream&) const
{
    m_fvCur.Format(strToFill);
}


const TCQCFldLimit* TCQCFldStoreTime::pfldlLimits() const
{
    return m_pfldlLimits;
}



// ---------------------------------------------------------------------------
//  TCQCFldStoreTime: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TCard8 TCQCFldStoreTime::c8Value() const
{
    return m_fvCur.c8Value();
}


tCQCKit::EValResults TCQCFldStoreTime::eValidate(const tCIDLib::TCard8& c8ToTest)
{
    if (m_fvCur.c8Value() == c8ToTest)
        return tCQCKit::EValResults::Unchanged;
    return m_pfldlLimits->eValidate(c8ToTest);
}

tCQCKit::EValResults
TCQCFldStoreTime::eValidate(const   TString&            strValue
                            ,       tCIDLib::TCard8&    c8ToFill)
{
    tCQCKit::EValResults eRes = m_pfldlLimits->eValidate(strValue, c8ToFill);
    if ((eRes == tCQCKit::EValResults::OK) && (c8ToFill == m_fvCur.c8Value()))
        eRes = tCQCKit::EValResults::Unchanged;
    return eRes;
}


tCIDLib::TVoid TCQCFldStoreTime::SetToNow()
{
    m_fvCur.bSetValue(TTime::enctNow());
}


//
//  We assume that CQCServer will not call us here unless the value of the
//  field has changed, however the driver may fool it by setting the value
//  after the base driver class has checked. So, we still check to see if
//  the value has changed.
//
tCIDLib::TVoid
TCQCFldStoreTime::SetValue(const tCIDLib::TCard8& c8ToSet)
{
    m_fvCur.bSetValue(c8ToSet);
}

