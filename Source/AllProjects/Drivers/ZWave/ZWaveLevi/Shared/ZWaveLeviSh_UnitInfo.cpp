//
// FILE NAME: ZWaveLeviSh_UnitInfo.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/10/2012
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
//  This file provides the implementation for the TZWaveUnit class, which
//  represents the info we need to maintain for the units we are controlling.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "ZWaveLeviSh_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TZWaveUnit,TCQCServerBase)


// ---------------------------------------------------------------------------
//  Local types and constants
// ---------------------------------------------------------------------------
namespace ZWaveLeviSh_UnitInfo
{
    // -----------------------------------------------------------------------
    //  Base class persistent format version
    //
    //  Version 2 -
    //      Forgot to stream the caps flag, when it was changed to have non-
    //      fixed characteristics in it.
    //
    //  Version 3 -
    //      Updated the old 'has a' boolean field to a more generic selection
    //      from a list of values the derived class provides. So we read in
    //      the old boolean and convert to 0 or 1 index into the new list. The
    //      classes that used the old scheme will provide new appropriate
    //      strings for the true/false values.
    // -----------------------------------------------------------------------
    const tCIDLib::TCard2   c2FmtVersion        = 3;
}



// ---------------------------------------------------------------------------
//   CLASS: TZWaveUnit
//  PREFIX: unit
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWaveUnit: Public, static methods
// ---------------------------------------------------------------------------
tCIDLib::ESortComps
TZWaveUnit::eCompById(  const   TZWaveUnit&     unit1
                        , const TZWaveUnit&     unit2
                        , const tCIDLib::TCard4 c4UserData)
{
    if (unit1.m_c4Id < unit2.m_c4Id)
        return tCIDLib::ESortComps::FirstLess;
    else if (unit1.m_c4Id > unit2.m_c4Id)
        return tCIDLib::ESortComps::FirstLess;
    return tCIDLib::ESortComps::Equal;
}

tCIDLib::ESortComps
TZWaveUnit::eCompByName(const   TZWaveUnit&     unit1
                        , const TZWaveUnit&     unit2
                        , const tCIDLib::TCard4 c4UserData)
{
    return unit1.m_strName.eCompare(unit2.m_strName);
}


//
//  Note that we are sorting really by 'most out of date', so we make the
//  earlier next poll times be 'greater', so that they will be first in the
//  list, since they are the ones that are the most out of date.
//
tCIDLib::ESortComps
TZWaveUnit::eCompByNextPoll(const   TZWaveUnit&     unit1
                            , const TZWaveUnit&     unit2
                            , const tCIDLib::TCard4 c4UserData)
{
    const tCIDLib::TEncodedTime enct1 = unit1.enctNextPoll();
    const tCIDLib::TEncodedTime enct2 = unit2.enctNextPoll();

    if (enct1 < enct2)
        return tCIDLib::ESortComps::FirstGreater;
    else if (enct1 > enct2)
        return tCIDLib::ESortComps::FirstLess;
    return tCIDLib::ESortComps::Equal;
}


//
//  This method will check all of the available unit derivatives for one
//  that matches the passed ZWave type info. With this controller we really
//  only can practically get the generic type. We may look for specific
//  types but that means we only get those specific types, so still the
//  generic type is all that is required here.
//
TZWaveUnit* TZWaveUnit::punitBestMatch(const tCIDLib::TCard4 c4GenType)
{
    // We create a list of class names and the types they represent
    struct TZWClassList
    {
        const tCIDLib::TCh* pszClass;
        tCIDLib::TCard4     c4GenType;
    };
    static TZWClassList aList[] =
    {
        { L"TZWController", GENERIC_TYPE_GENERIC_CONTROLLER }
      , { L"TZWBinSensor", GENERIC_TYPE_SENSOR_BINARY }
      , { L"TZWBinSwitch", GENERIC_TYPE_SWITCH_BINARY }
      , { L"TZWEntryCtrl", GENERIC_TYPE_ENTRY_CONTROL }
      , { L"TZWMLSwitch", GENERIC_TYPE_SWITCH_MULTILEVEL }
      , { L"TZWThermo", GENERIC_TYPE_THERMOSTAT }
    };
    const tCIDLib::TCard4 c4Count = tCIDLib::c4ArrayElems(aList);

    // Loop through our class types and find the match
    tCIDLib::TCard4 c4Match = kCIDLib::c4MaxCard;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TZWClassList& CurItem = aList[c4Index];
        if (CurItem.c4GenType == c4GenType)
        {
            c4Match = c4Index;
            break;
        }
    }

    // If none found, return zero, else gen up an object and return it
    if (c4Match == kCIDLib::c4MaxCard)
        return 0;
    return ::pobjMakeNewOfClass<TZWaveUnit>(aList[c4Match].pszClass);
}


// ---------------------------------------------------------------------------
//  TZWaveUnit: Destructor
// ---------------------------------------------------------------------------
TZWaveUnit::~TZWaveUnit()
{
}


// ---------------------------------------------------------------------------
//  TZWaveUnit: Public operators
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TZWaveUnit::operator==(const TZWaveUnit& unitSrc) const
{
    //
    //  We only need to compare the persisted stuff. This is only used during
    //  configuration to know if changes have been made. Compare the stuff
    //  mostly likely to be different in different units first.
    //
    return
    (
        (m_c4Id == unitSrc.m_c4Id)
        && (m_strName == unitSrc.m_strName)
        && (m_strTypeName == unitSrc.m_strTypeName)
        && (m_strTypeInfo == unitSrc.m_strTypeInfo)
        && (m_c4OptInfo == unitSrc.m_c4OptInfo)
        && (m_bDisabled == unitSrc.m_bDisabled)
        && (m_c1ClassGen == unitSrc.m_c1ClassGen)
        && (m_c1ClassSpec == unitSrc.m_c1ClassSpec)
        && (m_eCaps == unitSrc.m_eCaps)
        && (m_eFixedCaps == unitSrc.m_eFixedCaps)
        && (m_enctPollPeriod == unitSrc.m_enctPollPeriod)
    );
}


tCIDLib::TBoolean TZWaveUnit::operator!=(const TZWaveUnit& unitSrc) const
{
    return !operator==(unitSrc);
}


// ---------------------------------------------------------------------------
//  TZWaveUnit: Public, virtual methods
// ---------------------------------------------------------------------------

//
//  If they don't override this, we just build a basic get message. We ignore
//  the index in this case since we assume that it's just a basic unit with
//  a single value.
//
tCIDLib::TBoolean
TZWaveUnit::bBuildGetMsg(TString& strToFill, const tCIDLib::TCard4)
{
    strToFill = L"?N";
    strToFill.AppendFormatted(c4Id());

    return kCIDLib::True;
}


// If not overridden, we return false to indicate not handled
tCIDLib::TBoolean
TZWaveUnit::bBuildSetMsg(const  tCIDLib::TCard4 c4FldId
                        , const TString&        strValue
                        ,       TString&        strToFill
                        ,       TCQCServerBase& sdrvLevi)
{
    return kCIDLib::False;
}


//
//  If not overridden, we just say we don't need to get any optional user
//  configuration info.
//
tCIDLib::TBoolean
TZWaveUnit::bWantsOptInfo(tCIDLib::TStrCollect& colToFill) const
{
    colToFill.RemoveAll();
    return kCIDLib::False;
}



//
//  If the derived class doesn't overide, we just return false to say it was
//  not processed.
//
tCIDLib::TBoolean
TZWaveUnit::bStoreBasicReport(const tCIDLib::TCard4, TCQCServerBase& sdrvLevi)
{
    return kCIDLib::False;
}


//
//  If the derived class doesn't override, or passes on to us any general
//  reports it doesn't specifically handle, we try to handle some for them
//  and will log any unhandled ones if in verbose mode.
//
tCIDLib::TBoolean
TZWaveUnit::bStoreGenReport(const   tCIDLib::TCard4 ac4Values[]
                            , const tCIDLib::TCard4 c4ValCnt
                            ,       TCQCServerBase& sdrvLevi)
{
    if (c4ValCnt < 2)
        return kCIDLib::False;

    tCIDLib::TBoolean bRet = kCIDLib::True;
    switch(ac4Values[0])
    {
        case COMMAND_CLASS_APPLICATION_STATUS :
        {
            //
            //  The unit can tell us its busy and to try again later, and
            //  provide a time to wait before trying again. We don't use
            //  this yet, but handle it so it doesn't show up as an un
            //  handled report.
            //
            if (sdrvLevi.eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
            {
                const tCIDLib::TCh* pszMsg = 0;
                if (ac4Values[1] == APPLICATION_BUSY)
                    pszMsg = L"Unit %(1) (%(2)) reported that it is busy";
                else if (ac4Values[1] == APPLICATION_REJECTED_REQUEST)
                    pszMsg = L"Unit %(1) (%(2)) rejected a requested";
                else

                if (pszMsg)
                {
                    facZWaveLeviSh().LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , pszMsg
                        , tCIDLib::ESeverities::Status
                        , tCIDLib::EErrClasses::AppStatus
                        , strName()
                        , TCardinal(c4Id())
                    );
                }
            }

            break;
        }

        case COMMAND_CLASS_BATTERY :
        {
            if (ac4Values[1] == BATTERY_REPORT)
            {
                //
                //  If the battery level is low and we have not reported
                //  it in over an hour, then we can send out a battery level
                //  event trigger.
                //

            }
            break;
        }

        case COMMAND_CLASS_MANUFACTURER_PROPRIETARY :
        {
            // If not handled, don't report as a unhandled
            break;
        }

        case COMMAND_CLASS_WAKE_UP :
        {
            if (ac4Values[1] == WAKE_UP_NOTIFICATION)
            {
                //
                //  It's a battery powered unit that is awake for a bit. So
                //  let's force a poll of it immediately if pollable and not
                //  polled in the last minute.
                //
                const tCIDLib::TEncodedTime enctNow = TTime::enctNow();
                if (bIsPollable()
                &&  (enctNow > m_enctLastPoll)
                &&  ((enctNow - m_enctLastPoll) > kCIDLib::enctOneMinute))
                {
                    m_enctLastPoll = 0;
                }
            }
            break;
        }

        case COMMAND_CLASS_SECURITY :
        case COMMAND_CLASS_ALARM :
            // If the derived class doesn't handle these, just eat them
            break;

        default :
        {
            bRet = kCIDLib::False;
            break;
        }
    };
    return bRet;
}


//
//  By default, if they don't handle these, we just ignore them and return
//  false to say they weren't processed.
//
tCIDLib::TBoolean
TZWaveUnit::bStoreSceneReport(  const   tCIDLib::TCard4 c4Scene
                                , const tCIDLib::TCard4 c4Levle
                                , const tCIDLib::TCard4 c4FadeTime
                                ,       TCQCServerBase& sdrvLevi)
{
    return kCIDLib::False;
}


//
//  This is called before the configuration is actually put into use. It
//  is done before the driver goes through and finds the available units
//  and validates them and the configuration against each other.
//
//  We set our state to missing, until proven otherwise, and default some
//  stamps and such. No field stuff here since the fields driven by this
//  config have not been created yet.
//
//  Derived classes should zero out field ids and otherwise get into a
//  not set up state, and call us as well.
//
tCIDLib::TVoid TZWaveUnit::PrepForUse()
{
    m_eStatus = tZWaveLeviSh::EUnitStatus_Missing;

    //
    //  Set these one poll period back. which is enough to insure that they
    //  will be immediately ready to poll, but not so much that they look
    //  like they have failed.
    //
    m_enctLastPoll = TTime::enctNow() - m_enctPollPeriod;
    m_enctLastValue = m_enctLastPoll;

    // Zero out the get message index
    m_c4CurGetMsgIndex = 0;
}




// ---------------------------------------------------------------------------
//  TZWaveUnit: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  A convenience for info that could be gotten via bHasCap() below, but we
//  use this often enough that it's worth having a separate method for it.
//
tCIDLib::TBoolean TZWaveUnit::bBasicSupport() const
{
    const tZWaveLeviSh::EUnitCaps eToTest(tZWaveLeviSh::EUnitCap_BasicClass);
    return (tZWaveLeviSh::EUnitCaps(m_eCaps & eToTest) == eToTest);
}


//
//  A general helper for extracting the variable size numbers that many
//  device classes return. This one is for integral values, and will return
//  it pre-divided by any precision.
//
tCIDLib::TBoolean
TZWaveUnit::bDecodeValue(const  tCIDLib::TCard4     ac4Vals[]
                        , const tCIDLib::TCard4     c4ValCnt
                        ,       tCIDLib::TInt4&     i4Result
                        , const tCIDLib::TCh* const pszFld
                        ,       TCQCServerBase&     sdrvLevi) const
{
    // The first value is bitmapped so pull the values out
    const tCIDLib::TCard4 c4Precision = (ac4Vals[0] & 0xE0) >> 5;
    const tCIDLib::TCard4 c4Scale = (ac4Vals[0] & 0x18) >> 3;
    const tCIDLib::TCard4 c4Size = (ac4Vals[0] & 7);

    //
    //  Use the size to pull the bytes out and build up the value. It is
    //  two's complement, so we have to be careful about how we treat it,
    //  to insure we get the appropriate signed value.
    //
    if (c4Size == 1)
    {
        tCIDLib::TCard1 c1Val = tCIDLib::TCard1(ac4Vals[1]);
        i4Result = tCIDLib::TInt1(c1Val);
    }
     else if (c4Size == 2)
    {
        tCIDLib::TCard2 c2Val = tCIDLib::TCard1(ac4Vals[1]);
        c2Val <<= 8;
        c2Val |= tCIDLib::TCard1(ac4Vals[2]);
        i4Result = tCIDLib::TInt2(c2Val);
    }
     else if (c4Size == 4)
    {
        tCIDLib::TCard4 c4Val = tCIDLib::TCard1(ac4Vals[1]);
        c4Val <<= 8;
        c4Val |= tCIDLib::TCard1(ac4Vals[2]);
        c4Val <<= 8;
        c4Val |= tCIDLib::TCard1(ac4Vals[3]);
        c4Val <<= 8;
        c4Val |= tCIDLib::TCard1(ac4Vals[4]);
        i4Result = tCIDLib::TInt4(c4Val);
    }
     else
    {
        if (sdrvLevi.eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
        {
            facZWaveLeviSh().LogMsg
            (
                CID_FILE
                , CID_LINE
                , L"%(1) is not a valid value size. Target=%(2)/%(3)"
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
                , TCardinal(c4Size)
                , strName()
                , TString(pszFld)
            );
        }
        return kCIDLib::False;
    }

    //
    //  If there's any precision, pre-divide it so that we only store the
    //  integral part.
    //
    if (c4Precision)
        i4Result /= (10 * c4Precision);

    // It worked
    return kCIDLib::True;
}



// Get/set the disabled flag
tCIDLib::TBoolean TZWaveUnit::bDisabled() const
{
    return m_bDisabled;
}

tCIDLib::TBoolean TZWaveUnit::bDisabled(const tCIDLib::TBoolean bToSet)
{
    m_bDisabled = bToSet;
    return m_bDisabled;
}


// Returns true if the unit has all the indicated caps
tCIDLib::TBoolean
TZWaveUnit::bHasCap(const tZWaveLeviSh::EUnitCaps eToTest) const
{
    return (tZWaveLeviSh::EUnitCaps(m_eCaps & eToTest) == eToTest);
}


// Returns true if the unit has all the indicated fixed caps
tCIDLib::TBoolean
TZWaveUnit::bHasFixedCap(const tZWaveLeviSh::EUnitCaps eToTest) const
{
    return (tZWaveLeviSh::EUnitCaps(m_eFixedCaps & eToTest) == eToTest);
}


// A convenience for checking the battery powered caps flag
tCIDLib::TBoolean TZWaveUnit::bIsBatteryPwr() const
{
    const tZWaveLeviSh::EUnitCaps eToTest(tZWaveLeviSh::EUnitCap_Battery);
    return (tZWaveLeviSh::EUnitCaps(m_eCaps & eToTest) == eToTest);
}


//
//  If this unit is readable, the poll interval is non-zero, then it is
//  a potentially pollable unit, though it still might be failed or missing.
//
tCIDLib::TBoolean TZWaveUnit::bIsPollable() const
{
    return bHasCap(tZWaveLeviSh::EUnitCap_Readable)
           && (m_enctPollPeriod != 0);
}


// A convenience for checking the readable caps flag
tCIDLib::TBoolean TZWaveUnit::bIsReadable() const
{
    const tZWaveLeviSh::EUnitCaps eToTest(tZWaveLeviSh::EUnitCap_Readable);
    return (tZWaveLeviSh::EUnitCaps(m_eCaps & eToTest) == eToTest);
}


//
//  If this unit is diabled, or marked missing or failed, it shouldn't
//  be used, so we return false.
//
tCIDLib::TBoolean TZWaveUnit::bIsViable() const
{
    return !m_bDisabled
           && (m_eStatus != tZWaveLeviSh::EUnitStatus_Failed)
           && (m_eStatus != tZWaveLeviSh::EUnitStatus_Missing);
}


// Get the generic classes set by the derived class
tCIDLib::TCard1 TZWaveUnit::c1ClassGen() const
{
    return m_c1ClassGen;
}


// Get the specific classes set by the derived class
tCIDLib::TCard1 TZWaveUnit::c1ClassSpec() const
{
    return m_c1ClassSpec;
}


//
//  Get or set the count of get messages this unit requires. Most just set
//  in the call to our ctor, but some need to dynamically figure this out
//  and set it up after the fact.
//
tCIDLib::TCard4 TZWaveUnit::c4GetMsgCnt() const
{
    return m_c4GetMsgCnt;
}

tCIDLib::TCard4 TZWaveUnit::c4GetMsgCnt(const tCIDLib::TCard4 c4ToSet)
{
    m_c4GetMsgCnt = c4ToSet;
    return m_c4GetMsgCnt;
}


// Return the current get message index, bumping it for the next round
tCIDLib::TCard4 TZWaveUnit::c4IncGetMsgIndex()
{
    // Get the value we'll return
    const tCIDLib::TCard4 c4Ret = m_c4CurGetMsgIndex;

    // And now bump it for the next time
    m_c4CurGetMsgIndex++;
    if (m_c4CurGetMsgIndex >= m_c4GetMsgCnt)
        m_c4CurGetMsgIndex = 0;

    return c4Ret;
}


// Get/set the unit id
tCIDLib::TCard4 TZWaveUnit::c4Id() const
{
    return m_c4Id;
}

tCIDLib::TCard4 TZWaveUnit::c4Id(const tCIDLib::TCard4 c4ToSet)
{
    m_c4Id = c4ToSet;

    //
    //  If no name has been set yet, then we can now assign the default
    //  name based on the id.
    //
    if (m_strName.bIsEmpty())
        SetDefName();
    return m_c4Id;
}



// Get or set the option info value. The meaning depends on the unit type
tCIDLib::TCard4 TZWaveUnit::c4OptInfo() const
{
    return m_c4OptInfo;
}

tCIDLib::TCard4 TZWaveUnit::c4OptInfo(const tCIDLib::TCard4 c4ToSet)
{
    m_c4OptInfo = c4ToSet;
    return m_c4OptInfo;
}


// Get the unit capabilities flags
tZWaveLeviSh::EUnitCaps TZWaveUnit::eCaps() const
{
    return m_eCaps;
}


//
//  Get the overall unit status. We have specific setter methods for each
//  status, to make it more convenient for the driver, and they also have
//  to potentially set other things if the status changes.
//
tZWaveLeviSh::EUnitStatus TZWaveUnit::eStatus() const
{
    return m_eStatus;
}


// Get the last time that this unit was polled. Zero if not polled yet
tCIDLib::TEncodedTime TZWaveUnit::enctLastPoll() const
{
    return m_enctLastPoll;
}


// Get the last time that we got a value for this unit
tCIDLib::TEncodedTime TZWaveUnit::enctLastValue() const
{
    return m_enctLastValue;
}


// Get the next poll time. Which is our last poll time plus our poll period
tCIDLib::TEncodedTime TZWaveUnit::enctNextPoll() const
{
    return m_enctLastPoll + m_enctPollPeriod;
}


// Get/set the poll period
tCIDLib::TEncodedTime TZWaveUnit::enctPollPeriod() const
{
    return m_enctPollPeriod;
}

tCIDLib::TEncodedTime
TZWaveUnit::enctPollPeriod(const tCIDLib::TEncodedTime enctToSet)
{
    m_enctPollPeriod = enctToSet;
    return m_enctPollPeriod;
}


//
//  Set the time we last got a value for this unit to now. Normally this
//  is done via ResetPollTime() below, but in case someone needs to change
//  this by itself...
//
tCIDLib::TVoid TZWaveUnit::ResetLastValTime()
{
    m_enctLastValue = TTime::enctNow();
}


//
//  We reset our last poll time to now. We can optionally also update the
//  last value received stamp, mostly for dealing with async reports. In
//  that case we want to both indicate it doesn't need to be polled again
//  until the poll interval from now and to indicate we set a new value.
//
//  It's called whether the unit is set for polling or not. Updating the last
//  poll value doesn't hurt anything if not, and it makes things simpler.
//
tCIDLib::TVoid
TZWaveUnit::ResetPollTime(const tCIDLib::TBoolean bSetLastValStamp)
{
    m_enctLastPoll = TTime::enctNow();
    if (bSetLastValStamp)
        m_enctLastValue = m_enctLastPoll;
}


// Get/set the unit name
const TString& TZWaveUnit::strName() const
{
    return m_strName;
}

const TString& TZWaveUnit::strName(const TString& strToSet)
{
    m_strName = strToSet;
    return m_strName;
}


// Get the type name
const TString& TZWaveUnit::strTypeName() const
{
    return m_strTypeName;
}


// Get the type info
const TString& TZWaveUnit::strTypeInfo() const
{
    return m_strTypeInfo;
}


// Set a capabilities flag
tCIDLib::TVoid
TZWaveUnit::SetCapFlags( const  tZWaveLeviSh::EUnitCaps eToSet
                        , const tCIDLib::TBoolean       bState)
{
    if (bState)
        m_eCaps = tCIDLib::eOREnumBits(m_eCaps, eToSet);
    else
        m_eCaps = tCIDLib::eClearEnumBits(m_eCaps, eToSet);
}


//
//  Setters to put this unit into the various statuses that it can be
//  in. Some of them need to do other things as the status changes. If
//  we get a driver pointer, we can also update fields if that is appropriate
//  for the new status. Sometimes we don't get one because these are being
//  called for new unit config that's not installed yet.
//
tCIDLib::TVoid TZWaveUnit::SetErrorStatus(TCQCServerBase* const psdrvLevi)
{
    m_eStatus = tZWaveLeviSh::EUnitStatus_Error;

    // Put all our fields into error state if asked to
    if (psdrvLevi)
        SetFieldErrs(*psdrvLevi);
}

tCIDLib::TVoid TZWaveUnit::SetFailedStatus(TCQCServerBase* const psdrvLevi)
{
    m_eStatus = tZWaveLeviSh::EUnitStatus_Failed;

    //
    //  Most likely no fields ever got set, but in case they did, set them
    //  to error state, where they will stay.
    //
    if (psdrvLevi)
        SetFieldErrs(*psdrvLevi);
}

tCIDLib::TVoid TZWaveUnit::SetMissingStatus(TCQCServerBase* const psdrvLevi)
{
    m_eStatus = tZWaveLeviSh::EUnitStatus_Missing;
}

tCIDLib::TVoid TZWaveUnit::SetReadyStatus(TCQCServerBase* const psdrvLevi)
{
    m_eStatus = tZWaveLeviSh::EUnitStatus_Ready;

    //
    //  Reset some stuff that is related to polling and such. We don't
    //  set these to zero, since that would make us look way out of
    //  date and the field would get marked as in error. Just set it
    //  one poll period back, which is enough to insure we poll as soon
    //  as possible.
    //
    m_enctLastPoll = TTime::enctNow() - m_enctPollPeriod;
    m_enctLastValue = m_enctLastPoll;

    //
    //  If we got a driver pointer, put all our fields into error state in
    //  preparation for getting new info.
    //
    if (psdrvLevi)
        SetFieldErrs(*psdrvLevi);
}


// ---------------------------------------------------------------------------
//  TZWaveUnit: Hidden Constructors and Operators
// ---------------------------------------------------------------------------

TZWaveUnit::TZWaveUnit( const   tCIDLib::TCard1         c1GenType
                        , const tCIDLib::TCard1         c1SpecType
                        , const TString&                strTypeName
                        , const TString&                strTypeInfo
                        , const tZWaveLeviSh::EUnitCaps eCaps
                        , const tZWaveLeviSh::EUnitCaps eFixedCaps
                        , const tCIDLib::TCard4         c4UnitId
                        , const tCIDLib::TCard4         c4GetMsgCnt) :

    m_bDisabled(kCIDLib::False)
    , m_c1ClassGen(c1GenType)
    , m_c1ClassSpec(c1SpecType)
    , m_c4CurGetMsgIndex(0)
    , m_c4GetMsgCnt(c4GetMsgCnt)
    , m_c4OptInfo(kCIDLib::c4MaxCard)
    , m_c4Id(c4UnitId)
    , m_eCaps(eCaps)
    , m_eFixedCaps(eFixedCaps)
    , m_eStatus(tZWaveLeviSh::EUnitStatus_Missing)
    , m_enctLastPoll(0)
    , m_enctLastValue(0)
    , m_enctPollPeriod(0)
    , m_strTypeInfo(strTypeInfo)
    , m_strTypeName(strTypeName)
{
    // If async is on, force the poll interval to max
    if (m_eCaps & tZWaveLeviSh::EUnitCap_AsyncNotify)
        m_enctPollPeriod = kZWaveLeviSh::c4MaxPollSecs * kCIDLib::enctOneSecond;
}

TZWaveUnit::TZWaveUnit(const TZWaveUnit& unitSrc) :

    m_bDisabled(unitSrc.m_bDisabled)
    , m_c1ClassGen(unitSrc.m_c1ClassGen)
    , m_c1ClassSpec(unitSrc.m_c1ClassSpec)
    , m_c4CurGetMsgIndex(unitSrc.m_c4CurGetMsgIndex)
    , m_c4GetMsgCnt(unitSrc.m_c4GetMsgCnt)
    , m_c4Id(unitSrc.m_c4Id)
    , m_c4OptInfo(unitSrc.m_c4OptInfo)
    , m_eCaps(unitSrc.m_eCaps)
    , m_eFixedCaps(unitSrc.m_eFixedCaps)
    , m_eStatus(unitSrc.m_eStatus)
    , m_enctLastPoll(unitSrc.m_enctLastPoll)
    , m_enctLastValue(unitSrc.m_enctLastValue)
    , m_enctPollPeriod(unitSrc.m_enctPollPeriod)
    , m_strName(unitSrc.m_strName)
    , m_strTypeInfo(unitSrc.m_strTypeInfo)
    , m_strTypeName(unitSrc.m_strTypeName)
{
}

TZWaveUnit& TZWaveUnit::operator=(const TZWaveUnit& unitSrc)
{
    if (this != &unitSrc)
    {
        m_bDisabled         = unitSrc.m_bDisabled;
        m_c1ClassGen        = unitSrc.m_c1ClassGen;
        m_c1ClassSpec       = unitSrc.m_c1ClassSpec;
        m_c4CurGetMsgIndex  = unitSrc.m_c4CurGetMsgIndex;
        m_c4GetMsgCnt       = unitSrc.m_c4GetMsgCnt;
        m_c4Id              = unitSrc.m_c4Id;
        m_c4OptInfo         = unitSrc.m_c4OptInfo;
        m_eCaps             = unitSrc.m_eCaps;
        m_eFixedCaps        = unitSrc.m_eFixedCaps;
        m_eStatus           = unitSrc.m_eStatus;
        m_enctLastPoll      = unitSrc.m_enctLastPoll;
        m_enctLastValue     = unitSrc.m_enctLastValue;
        m_enctPollPeriod    = unitSrc.m_enctPollPeriod;
        m_strName           = unitSrc.m_strName;
        m_strTypeInfo       = unitSrc.m_strTypeInfo;
        m_strTypeName       = unitSrc.m_strTypeName;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TZWaveUnit: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TZWaveUnit::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Make sure we get the start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > ZWaveLeviSh_UnitInfo::c2FmtVersion))
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

    strmToReadFrom  >> m_c4Id
                    >> m_bDisabled
                    >> m_strName
                    >> m_eStatus
                    >> m_enctPollPeriod;

    //
    //  If version 3 or beyond, read in the option index. Else, read in the
    //  old boolean and convert to a 0 or one index.
    //
    //  Note that the derived class perhaps didn't support any optional info,
    //  and in the upgrade here we really want to not set 0 or 1, but to
    //  set it to max card. In theory it wouldn't matter since the derived
    //  class will report it doesn't have any optional info. But, to be safe,
    //  we'll ask the derived class and set max card if not supported, else
    //  we do 0 or 1 based on the old boolean.
    //
    if (c2FmtVersion > 2)
    {
        strmToReadFrom >> m_c4OptInfo;

        //
        //  Correct an error here. If they have opt info and it's max card,
        //  set it to zero.
        //
        if (m_c4OptInfo == kCIDLib::c4MaxCard)
        {
            tCIDLib::TStrList colTmp;
            if (bWantsOptInfo(colTmp))
                m_c4OptInfo = 0;
        }
    }
     else
    {
        tCIDLib::TBoolean bIsAState;
        strmToReadFrom >> bIsAState;

        tCIDLib::TStrList colTmp;
        if (bWantsOptInfo(colTmp))
            m_c4OptInfo = bIsAState ? 1 : 0;
        else
            m_c4OptInfo = kCIDLib::c4MaxCard;
    }

    //
    //  If V2 or beyond, read in the caps flag. Otherwise, leave what was put
    //  there in the ctor by the derived class.
    //
    if (c2FmtVersion > 1)
        strmToReadFrom >> m_eCaps;

    // Make sure we get the end marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);

    //
    //  Reset the runtime fields.
    //
    //  DO NOT reset the 'is a check' message id. Those are set on us by the
    //  drived class upon ctor and stay valid.
    //
    m_c4CurGetMsgIndex = 0;
    m_enctLastValue = 0;
    m_enctLastPoll = 0;
}


tCIDLib::TVoid TZWaveUnit::StreamTo(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << ZWaveLeviSh_UnitInfo::c2FmtVersion
                    << m_c4Id
                    << m_bDisabled
                    << m_strName
                    << m_eStatus
                    << m_enctPollPeriod
                    << m_c4OptInfo

                    // V2 stuff
                    << m_eCaps

                    << tCIDLib::EStreamMarkers::EndObject;
}


// ---------------------------------------------------------------------------
//  TZWaveUnit: Protected, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TZWaveUnit::bCheckFldId(const   tCIDLib::TCard4     c4ToCheck
                        , const tCIDLib::TCh* const pszFldName) const
{
    if (c4ToCheck == kCIDLib::c4MaxCard)
    {
        facZWaveLeviSh().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kZWShErrs::errcUnit_FldIdNotSet
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotReady
            , strName()
            , TString(pszFldName)
        );
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


// Based on the caps flags, we return a likely default field access type
tCQCKit::EFldAccess TZWaveUnit::eDefFldAccess() const
{
    const tCIDLib::TBoolean bReadable = bHasCap(tZWaveLeviSh::EUnitCap_Readable);
    const tCIDLib::TBoolean bWriteable = bHasCap(tZWaveLeviSh::EUnitCap_Writeable);

    tCQCKit::EFldAccess eRet;
    if (bReadable && bWriteable)
        eRet = tCQCKit::EFldAccess::ReadWrite;
    else if (bReadable && !bWriteable)
        eRet = tCQCKit::EFldAccess::Read;
    else if  (!bReadable && bWriteable)
        eRet = tCQCKit::EFldAccess::Write;
    else
    {
        // This isn't legal
        CIDAssert2(L"Unit caps provide no field access");
        eRet = tCQCKit::EFldAccess::Read;
    }
    return eRet;
}


// Creates a default name for newly discovered modules
tCIDLib::TVoid TZWaveUnit::SetDefName()
{
    //
    //  Start with the type name provided by the derived class, which will be
    //  something like "BinSwitch" or "MultiSwitch" and so forth.
    //
    m_strName = m_strTypeName;

    // Then an underscore and a 2 digit hex representation of the unit id
    m_strName.Append(kCIDLib::chUnderscore);
    if (m_c4Id < 0x10)
        m_strName.Append(kCIDLib::chDigit0);
    m_strName.AppendFormatted(m_c4Id, tCIDLib::ERadices::Hex);
}

