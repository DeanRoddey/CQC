//
// FILE NAME: ZWaveUSB3Sh_UnitInfo.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/24/2017
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
//  This is the implementation for the Z-Wave unit info class, which contains all
//  the information (automatically gathered or user provided) to set up a unit
//  class at runtime.
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
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TZWUnitInfo, TObject)


// ---------------------------------------------------------------------------
//  Local types and constants
// ---------------------------------------------------------------------------
namespace ZWaveUSB3Sh_UnitInfo
{
    // Our persistent format version
    const tCIDLib::TCard2   c2FmtVersion    = 1;
}


// ---------------------------------------------------------------------------
//   CLASS: TZWUnitInfo
//  PREFIX: unit
// ---------------------------------------------------------------------------
tCIDLib::TCard1 TZWUnitInfo::c1Key(const TZWUnitInfo& unitiSrc)
{
    return unitiSrc.c1Id();
}

tCIDLib::ESortComps
TZWUnitInfo::eCompById( const   TZWUnitInfo& uniti1
                        , const TZWUnitInfo& uniti2)
{
    if (uniti1.c1Id() < uniti2.c1Id())
        return tCIDLib::ESortComps::FirstLess;
    else if (uniti1.c1Id() > uniti2.c1Id())
        return tCIDLib::ESortComps::FirstLess;
    return tCIDLib::ESortComps::Equal;
}

tCIDLib::ESortComps
TZWUnitInfo::eCompByManIds( const   TZWUnitInfo& uniti1
                            , const TZWUnitInfo& uniti2)
{
    if (uniti1.c1Id() < uniti2.c1Id())
        return tCIDLib::ESortComps::FirstLess;
    else if (uniti1.c1Id() > uniti2.c1Id())
        return tCIDLib::ESortComps::FirstLess;
    return tCIDLib::ESortComps::Equal;
}

tCIDLib::ESortComps
TZWUnitInfo::eCompByName(const  TZWUnitInfo& uniti1
                        , const TZWUnitInfo& uniti2)
{
    return uniti1.strName().eCompare(uniti2.strName());
}

const TString& TZWUnitInfo::strUIKey(const TZWUnitInfo& unitiSrc)
{
    return unitiSrc.strName();
}



// ---------------------------------------------------------------------------
//  TZWUnitInfo: Destructor
// ---------------------------------------------------------------------------

//
//  Setting m_eState here is IMPORTANT, even though we call Reset(), because it will
//  call eState() to set it, and that will look at the old state, which wouldn't be
//  initialized.
//
TZWUnitInfo::TZWUnitInfo() :

    m_colOptVals(37, TStringKeyOps(kCIDLib::False), &TKeyValuePair::strExtractKey)
    , m_colWakeupQ()
    , m_eState(tZWaveUSB3Sh::EUnitStates::InitUnit)
    , m_punitThis(nullptr)
    , m_pmzwciOwner(nullptr)
{
    Reset(0);
}

//
//  Setting m_eState here is IMPORTANT, even though we call Reset(), because it will
//  call eState() to set it, and that will look at the old state, which wouldn't be
//  initialized. ANd the same for the unit object which would try to delete an
//  uninitialized pointer.
//
TZWUnitInfo::TZWUnitInfo(const  tCIDLib::TCard1 c1UnitId) :

    m_colOptVals(37, TStringKeyOps(kCIDLib::False), &TKeyValuePair::strExtractKey)
    , m_colWakeupQ()
    , m_eState(tZWaveUSB3Sh::EUnitStates::InitUnit)
    , m_punitThis(nullptr)
    , m_pmzwciOwner(nullptr)
{
    // Reset us with the passed unit id (STATE MUST be initialized first!)
    Reset(c1UnitId);
}


//
//  The runtime only stuff is not copied, just reset. BE SURE to initialize the
//  unit object pointer since the reset will try to delete it.
//
TZWUnitInfo::TZWUnitInfo(const TZWUnitInfo& unitiSrc) :

    m_bIsCtrl(unitiSrc.m_bIsCtrl)
    , m_bListener(unitiSrc.m_bListener)
    , m_bFreqListener(unitiSrc.m_bFreqListener)
    , m_bHasManSpec(unitiSrc.m_bHasManSpec)
    , m_bIsSecure(unitiSrc.m_bIsSecure)
    , m_bManSpecSecure(unitiSrc.m_bManSpecSecure)
    , m_c1Id(unitiSrc.m_c1Id)
    , m_c4SerialNum(unitiSrc.m_c4SerialNum)
    , m_c4TryCount(0)
    , m_colOptVals(unitiSrc.m_colOptVals)
    , m_deviUnit(unitiSrc.m_deviUnit)
    , m_eState(unitiSrc.m_eState)
    , m_eNameSrc(unitiSrc.m_eNameSrc)
    , m_pmzwciOwner(unitiSrc.m_pmzwciOwner)
    , m_strName(unitiSrc.m_strName)

    // Runtime only (INITIALIZE unit pointer though!)
    , m_colWakeupQ()
    , m_punitThis(nullptr)
{
    ResetRuntime();
}

TZWUnitInfo::~TZWUnitInfo()
{
    // Clean up our unit object if we got one
    try
    {
        delete m_punitThis;
    }

    catch(...)
    {
    }
    m_punitThis  = nullptr;
}


// ---------------------------------------------------------------------------
//  TZWUnitInfo: Public operators
// ---------------------------------------------------------------------------

//
//  The unit object is not copied, and any existing one is destroyed. State is taken
//  back to initial state and some other runtime only things are defaulted.
//
TZWUnitInfo& TZWUnitInfo::operator=(const TZWUnitInfo& unitiSrc)
{
    if (&unitiSrc != this)
    {
        m_bIsCtrl           = unitiSrc.m_bIsCtrl;
        m_bListener         = unitiSrc.m_bListener;
        m_bFreqListener     = unitiSrc.m_bFreqListener;
        m_bHasManSpec       = unitiSrc.m_bHasManSpec;
        m_bIsSecure         = unitiSrc.m_bIsSecure;
        m_bManSpecSecure    = unitiSrc.m_bManSpecSecure;
        m_c1Id              = unitiSrc.m_c1Id;
        m_c4SerialNum       = unitiSrc.m_c4SerialNum;
        m_colOptVals        = unitiSrc.m_colOptVals;
        m_deviUnit          = unitiSrc.m_deviUnit;
        m_eNameSrc          = unitiSrc.m_eNameSrc;
        m_eState            = unitiSrc.m_eState;
        m_pmzwciOwner       = unitiSrc.m_pmzwciOwner;
        m_strName           = unitiSrc.m_strName;

        // Reset runtime only stuff
        ResetRuntime();
    }
    return *this;
}


//
//  The runtime only stuff is not included here (next proc time, unit object,
//  driver pointer, wakeup queue, etc...) The state is included
//
tCIDLib::TBoolean TZWUnitInfo::operator==(const TZWUnitInfo& unitiSrc) const
{
    if (this == &unitiSrc)
        return kCIDLib::True;

    //
    //  Put stuff first that is most likely not be the same, which would be unit
    //  specific stuff.
    //
    return
    (
        (m_c1Id == unitiSrc.m_c1Id)
        && (m_c4SerialNum == unitiSrc.m_c4SerialNum)
        && (m_eState == unitiSrc.m_eState)
        && (m_deviUnit == unitiSrc.m_deviUnit)
        && (m_colOptVals == unitiSrc.m_colOptVals)
        && (m_bIsCtrl == unitiSrc.m_bIsCtrl)
        && (m_bListener == unitiSrc.m_bListener)
        && (m_bFreqListener == unitiSrc.m_bFreqListener)
        && (m_bHasManSpec == unitiSrc.m_bHasManSpec)
        && (m_bIsSecure == unitiSrc.m_bIsSecure)
        && (m_bManSpecSecure == unitiSrc.m_bManSpecSecure)
        && (m_eNameSrc == unitiSrc.m_eNameSrc)
        && (m_strName == unitiSrc.m_strName)
    );
}

tCIDLib::TBoolean TZWUnitInfo::operator!=(const TZWUnitInfo& unitiSrc) const
{
    return !operator==(unitiSrc);
}


// ---------------------------------------------------------------------------
//  TZWUnitInfo: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  The client interface will call this (indirectly via a backdoor command) when
//  the user asks to approve a unit that has been identified.
//
tCIDLib::TVoid TZWUnitInfo::ApproveForUse()
{
    // Make sure we are in wait approve state
    if (m_eState != tZWaveUSB3Sh::EUnitStates::WaitApprove)
    {
        facZWaveUSB3Sh().LogTraceErr
        (
            L"Got approve call from client for unit %(1) but it is not waiting for approval"
            , strName()
        );
        return;
    }

    //
    //  Set it to get init values state, and reset the next processing time so it
    //  will do it quickly.
    //
    eState(tZWaveUSB3Sh::EUnitStates::GetInitVals);
    m_enctNextProc = 0;
    m_c4TryCount = 0;
}


//
//  This will send out msgs to set up any auto-configuration that our device info
//  file contained. The caller can tell us if he knows the unit is awake. If so,
//  then we force them to be queued up to send now, even if not always on. Else
//  if not always on, they are just queued up and wait for a wakeup.
//
tCIDLib::TBoolean
TZWUnitInfo::bDoAutoCfg(const tCIDLib::TBoolean bIsAwake, TString& strStatus)
{
    tCIDLib::TBoolean bErrs = kCIDLib::False;
    tCIDLib::TCard4 c4Count;
    TString strErr;

    const tZWaveUSB3Sh::TAssocList& colAssoc = m_deviUnit.colAssociations();
    c4Count = colAssoc.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TZWAssocInfo& associCur = colAssoc[c4Index];
        const tCIDLib::TBoolean bRes = TZWUSB3CCAssoc::bSetAssoc
        (
            *this
            , associCur.m_c1Group
            , facZWaveUSB3Sh().c1CtrlId()
            , kCIDLib::True
            , bIsAwake
            , strErr
            , associCur.m_c1EndPoint
        );

        if (!bRes)
        {
            bErrs = kCIDLib::True;
            facZWaveUSB3Sh().LogTraceErr
            (
                L"Auto-cfg - Could not associate unit %(1) with driver. Error=%(2)"
                , m_strName
                , strErr
            );
        }
    }

    const tZWaveUSB3Sh::TCfgParamList& colParams = m_deviUnit.colCfgParams();
    c4Count = colParams.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TZWCfgParam& cfgpCur = colParams[c4Index];
        const tCIDLib::TBoolean bRes = TZWUSB3CCConfig::bSetParam
        (
            *this
            , cfgpCur.m_c1ParamNum
            , cfgpCur.m_c4ByteCnt
            , cfgpCur.m_i4Value
            , kCIDLib::True
            , bIsAwake
            , strErr
        );

        if (!bRes)
        {
            bErrs = kCIDLib::True;
            facZWaveUSB3Sh().LogTraceErr
            (
                L"Auto-cfg - Could not set cfg param %(1) on unit %(2). Error=%(3)"
                , TCardinal(cfgpCur.m_c1ParamNum)
                , m_strName
                , strErr
            );
        }
    }

    // If there's a wakeup interval set, let's send that out
    if (m_deviUnit.c4WakeupInt() != 0)
    {
        TString strErr;
        const tCIDLib::TBoolean bRes = TZWUSB3CCWakeup::bSetWakeup
        (
            *this
            , facZWaveUSB3Sh().c1CtrlId()
            , m_deviUnit.c4WakeupInt()
            , kCIDLib::True
            , bIsAwake
            , strErr
        );

        if (!bRes)
        {
            bErrs = kCIDLib::True;
            facZWaveUSB3Sh().LogTraceErr
            (
                L"Auto-cfg - Could not set wakeup interval on unit %(1). Error=%(2)"
                , m_strName
                , strErr
            );
        }
    }

    const tCIDLib::TBoolean bMsgsSent(bAlwaysOn() || bIsAwake);
    if (bMsgsSent)
        strStatus = L"Auto-config messages were transmitted.";
    else
        strStatus = L"Auto-config messages were queued pending unit wakeup.";
    if (bErrs)
        strStatus.Append(L" Some errors occurred. Use the trace log to get details.");

    // If tracing, log some info
    if (facZWaveUSB3Sh().bLowTrace())
    {
        if (bMsgsSent)
            facZWaveUSB3Sh().LogTraceMsg(L"Auto-cfg msgs sent for unit %(1)", m_strName);
        else
            facZWaveUSB3Sh().LogTraceMsg(L"Auto-cfg msgs wait-queued for unit %(1)", m_strName);
    }
    return !bErrs;
}



//
//  This is called by the client if the user tells us to use a specific device info
//  file. This is only for those units that we cannot auto-id. This will prevent the
//  man ids from being overridden during unit scans, so that we keep this setup.
//
//  The only want to get rid of it is for the user to ask for a unit rescan.
//
tCIDLib::TBoolean
TZWUnitInfo::bForceManIds(const tCIDLib::TCard8 c8ToSet, TString& strErr)
{
    facZWaveUSB3Sh().LogTraceMsg(L"Unit %(1) type is being manually set", strName());

    // Try to load up the device info into a temp device info object
    TZWDevInfo deviTmp;
    if (facZWaveUSB3Sh().bQueryDevInfo(c8ToSet, deviTmp, strErr))
    {
        // Indicate we don't know for sure that it's awake if a non-listener
        StoreDevInfo(deviTmp, kCIDLib::False);

        // If it worked, go to WaitApprove
        if (m_punitThis)
            eState(tZWaveUSB3Sh::EUnitStates::WaitApprove);

        return kCIDLib::True;
    }

    facZWaveUSB3Sh().LogTraceErr
    (
        L"No device info matched new man ids for unit %(1)", strName()
    );
    return kCIDLib::False;
}


//
//  This is called if we get asked by the user to force a rescan of a unit.
//
//  We return true if we were at or beyond GetInitVals state, which means the caller
//  should re-register fields since we had them but are now in a state where we should
//  not have them.
//
//  This is tricky. If we ever loaded device info, we want to clear anything that
//  came from the device info file. But we can't just reset it because that would
//  clear the basic node info. We assume it couldn't have gotten that out of sync since
//  that would require a replication to change the actual type.
//
tCIDLib::TBoolean TZWUnitInfo::bForceRescan(const tCIDLib::TBoolean bKnownAwake)
{
    // We will be resetting it, so remember if we need to re-register fields
    facZWaveUSB3Sh().LogTraceMsg(L"Unit %(1) is being forced to rescan", strName());
    const tCIDLib::TBoolean bReReg(m_eState >= tZWaveUSB3Sh::EUnitStates::GetInitVals);
    if (bReReg)
        facZWaveUSB3Sh().LogTraceMsg(L"A field re-registration is required");

    //
    //  Reset any info that we got before. We don't just directly call the device
    //  info object, since there is also the unit handler object to clean up and
    //  possibly other things moving forward. Indicate this is a rescan type
    //  reset, which will keep the command classes.
    //
    ResetDevInfo(kCIDLib::True);

    //
    //  And push us back to get unit info state. We assume the basic node protocol
    //  info stuff that we don upon first seeing a new node cannot change without
    //  a replication occurring.
    //
    eState(tZWaveUSB3Sh::EUnitStates::GetUnitInfo);

    //
    //  Various things above will have incremented the serial number, probably a number
    //  of times already.
    //
    return bReReg;
}


//
//  Check to see if we have an option value, and return if it so. This always
//  just returns it as text.
//
tCIDLib::TBoolean
TZWUnitInfo::bFindOptVal(const TString& strKey, TString& strToFill) const
{
    const TKeyValuePair* pkvalOpt = m_colOptVals.pobjFindByKey(strKey);
    if (!pkvalOpt)
        return kCIDLib::False;

    strToFill = pkvalOpt->strValue();
    return kCIDLib::True;
}


//
//  Look for an option that should have a boolean value. If we can't find it or
//  convert it, we return the indicated default instead.
//
tCIDLib::TBoolean
TZWUnitInfo::bQueryOptVal(  const   TString&            strKey
                            , const tCIDLib::TBoolean   bDefault) const
{
    const TKeyValuePair* pkvalOpt = m_colOptVals.pobjFindByKey(strKey);
    if (!pkvalOpt)
        return bDefault;

    tCIDLib::TBoolean bRes;
    if (!facCQCKit().bCheckBoolVal(pkvalOpt->strValue(), bRes))
        bRes = bDefault;

    return bRes;
}


// Return return if the indicated option key exists
tCIDLib::TBoolean TZWUnitInfo::bOptExists(const TString& strKey) const
{
    return (m_colOptVals.pobjFindByKey(strKey) != nullptr);
}


//
//  Since these are often created generically via the manuafacturer id based lookup,
//  which doesn't have unit id info, it has to be set after it's created by the
//  caller who knows the id.
//
tCIDLib::TCard1 TZWUnitInfo::c1Id(const tCIDLib::TCard1 c1ToSet)
{
    IncSerialNum();
    m_c1Id = c1ToSet;
    return m_c1Id;
}


//
//  Look for an option that should have a card4 value. If we can't find it or
//  convert it, we return the default.
//
tCIDLib::TCard4
TZWUnitInfo::c4QueryOptVal( const   TString&            strKey
                            , const tCIDLib::TCard4     c4Default) const
{
    const TKeyValuePair* pkvalOpt = m_colOptVals.pobjFindByKey(strKey);
    if (!pkvalOpt)
        return c4Default;

    tCIDLib::TCard4 c4Ret;
    if (!pkvalOpt->strValue().bToCard4(c4Ret, tCIDLib::ERadices::Auto))
        c4Ret = c4Default;

    return c4Ret;
}


//
//  These can pre-test whether a given CC/Endpoint combo has been configured for
//  particular access via the extra info. This is sometimes needed so that we can know
//  up front whether we are V2 compatible or meet some other access criteria before we
//  create the CCs.
//
tCIDLib::EFindRes2
TZWUnitInfo::eCheckCCAccess(const   tZWaveUSB3Sh::ECClasses eClass
                            , const tZWaveUSB3Sh::EUnitAcc  eCanHave
                            , const tZWaveUSB3Sh::EUnitAcc  eMustHave
                            , const tCIDLib::TCard1         c1EndPoint) const
{
    // Call the probe method to get the flags
    tZWaveUSB3Sh::EUnitAcc eFound = tZWaveUSB3Sh::EUnitAcc::None;
    tCIDLib::EFindRes2 eRet = eProbeCCAccess(eClass, eFound, c1EndPoint);

    // If not found, then return that status now
    if (eRet != tCIDLib::EFindRes2::Found)
        return eRet;

    // The OR of can bits and found bits has to equal can bits
    if ((tCIDLib::TInt4(eCanHave) | tCIDLib::TInt4(eFound)) != tCIDLib::TInt4(eCanHave))
        return tCIDLib::EFindRes2::NoMatch;

    // The AND of must and found bits has to equal must bits
    if ((tCIDLib::TInt4(eMustHave) | tCIDLib::TInt4(eFound)) != tCIDLib::TInt4(eMustHave))
        return tCIDLib::EFindRes2::NoMatch;

    return tCIDLib::EFindRes2::Found;
}



//
//  Sometimes, in order to know before we create a CC if we have the stuff we need to
//  be V2 compliant, we need to know the access flags set on the CC, if any. So this
//  method is provided. That info comes from the device info, which we own.
//
//  The base CC impl class uses this as well, to keep it in one place.
//
//  It doesn't return all the possible values:
//
//      NotFound        - Obviously just no RWAccess is set. Incoming flag values were
//                        not changed.
//      Error           - There were bad values in the extra info
//      Found           - We got it and set any flags we found, clearing the others
//
//  For backwards compatibility we look for a couple variations of some of the access
//  flag values. We simplfied and consolidated these values in 5.3.915 but there are
//  existing driver configurations with the old values out there potentially.
//
tCIDLib::EFindRes2
TZWUnitInfo::eProbeCCAccess(const   tZWaveUSB3Sh::ECClasses eClass
                            ,       tZWaveUSB3Sh::EUnitAcc& eToFill
                            , const tCIDLib::TCard1         c1EndPoint) const
{
    TString strVal;
    if (!m_deviUnit.bFindExtraVal(eClass, c1EndPoint, L"RWAccess", strVal))
        return tCIDLib::EFindRes2::NotFound;

    eToFill = tZWaveUSB3Sh::EUnitAcc::None;

    // It can just be None by itself
    strVal.StripWhitespace();
    if (strVal.bCompareI(L"None"))
    {
        // It's legal, but nothing else to do
    }
     else
    {
        // This is a set of space separated values
        TStringTokenizer stokAccess(&strVal, L" ");
        TString strToken;
        while (stokAccess.bGetNextToken(strToken))
        {
            if (strToken.bCompareI(L"Readable") || strToken.bCompareI(L"Read"))
                eToFill = tCIDLib::eOREnumBits(eToFill, tZWaveUSB3Sh::EUnitAcc::Read);
            else if (strToken.bCompareI(L"Writeable") || strToken.bCompareI(L"Write"))
                eToFill = tCIDLib::eOREnumBits(eToFill, tZWaveUSB3Sh::EUnitAcc::Write);
            else if (strToken.bCompareI(L"ReadAfterWrite"))
                eToFill = tCIDLib::eOREnumBits(eToFill, tZWaveUSB3Sh::EUnitAcc::ReadAfterWrite);
            else if (strToken.bCompareI(L"ReadOnWake") || strToken.bCompareI(L"ReadOnWakeUp"))
                eToFill = tCIDLib::eOREnumBits(eToFill, tZWaveUSB3Sh::EUnitAcc::ReadOnWake);
            else
            {
                facZWaveUSB3Sh().LogTraceErr(L"Invalid RWAccess value in extra info");
                return tCIDLib::EFindRes2::Error;
            }
        }
    }

    // If any optional read flags set but readable is not, then an error
    if (tCIDLib::bAnyBitsOn(eToFill, tZWaveUSB3Sh::EUnitAcc::OptReadFlags)
    &&  !tCIDLib::bAllBitsOn(eToFill, tZWaveUSB3Sh::EUnitAcc::Read))
    {
        facZWaveUSB3Sh().LogTraceErr(L"Optional read flags set but read flag not set");
        return tCIDLib::EFindRes2::Error;
    }

    return tCIDLib::EFindRes2::Found;
}


//
//  Get or set the unit state. This is runtime only, see the header comments.
//
tZWaveUSB3Sh::EUnitStates TZWUnitInfo::eState() const
{
    return m_eState;
}

tZWaveUSB3Sh::EUnitStates
TZWUnitInfo::eState(const tZWaveUSB3Sh::EUnitStates eToSet)
{
    // If the state changed, then let's note that
    if (eToSet != m_eState)
    {
        const tZWaveUSB3Sh::EUnitStates eOldState = m_eState;
        m_eState = eToSet;

        IncSerialNum();

        // If tracing, then log a state change
        if (facZWaveUSB3Sh().bMediumTrace())
        {
            facZWaveUSB3Sh().LogTraceMsg
            (
                L"Unit '%(1)' changed state from %(2) to %(3)"
                , strName()
                , tZWaveUSB3Sh::strAltXlatEUnitStates(eOldState)
                , tZWaveUSB3Sh::strAltXlatEUnitStates(m_eState)
            );
        }
    }
    return m_eState;
}


// Formats out our auto-config steps
tCIDLib::TVoid TZWUnitInfo::FormatAutoCfg(TTextOutStream& strmTar) const
{
    m_deviUnit.FormatAutoCfg(strmTar);
}


//
//  Generates a report on the important aspects of this unit. This is for runtime
//  when we want to see not what is configured (device info stuff) but what actually
//  has gotten set and created in terms of units and CC impls.
//
//  Unfortunately this means that various unit infos need to override this because
//  they create CC impls that they don't set on the base unit class. So it's always
//  possible we'll miss something and have to add later when we realize this.
//
tCIDLib::TVoid TZWUnitInfo::FormatReport(TTextOutStream& strmTar) const
{
    strmTar << L"\nUnit: " << m_strName
            << L"\n-------------------------------\n";

    strmTar << L"Unit Id: " << m_c1Id;
    if (m_bFreqListener)
        strmTar << L", FreqListener";
    else if (m_bListener)
        strmTar << L", Listener";
    else
        strmTar << L", Non-Listener";

    // The rest requires we be far enough along to have a handler
    if (m_punitThis)
    {
        strmTar << L", Handler: ";
        if (m_punitThis)
            strmTar << m_punitThis->clsIsA().strClassName();
        else
            strmTar << L"[None]";
        strmTar << kCIDLib::NewLn;

        // Ask the unit handler to report his info
        m_punitThis->FormatReport(strmTar);
    }
     else
    {
        if (m_deviUnit.c8ManIds())
        {
            strmTar << L"\n[Could not match id "
                    << TCardinal64(m_deviUnit.c8ManIds(), tCIDLib::ERadices::Hex)
                    << L" to a supported unit type]\n";
        }
         else
        {
            strmTar << L"\n[No unit type information is available yet]\n";
        }
    }

    strmTar << kCIDLib::EndLn;
}


//
//  If we are in a state to deal with such things, the driver will pass along to us any
//  app update msgs that are for our unit.
//
//  The driver will handle any exceptions that propagate out of here.
//
//  We will get these for a couple reasons:
//
//  1.  We are in GetUnitInfo state, which means that we are issuing requests for
//      the node info frame. This means we got one, and can move forward.
//  2.  We are in WaitWakeup state. If so, we treat this as though a wakeup occurred
//      and we can move forward.
//  3.  We are in Ready state, which probably means that they changed the batteries or
//      or unplugged it and plugged it back in. We will use this to try to get restarted
//      as quickly as possible. We tell the unit about it.
//
tCIDLib::TVoid TZWUnitInfo::HandleAppUpdate(const TZWInMsg& zwimIn)
{
    // If this guy is disabled, ignore this
    if (m_eState == tZWaveUSB3Sh::EUnitStates::Disabled)
        return;

    if ((m_eState == tZWaveUSB3Sh::EUnitStates::GetUnitInfo)
    ||  (m_eState == tZWaveUSB3Sh::EUnitStates::WaitWakeup))
    {
        // Search the classes to see if security and/or man spec is there
        const tCIDLib::TCard4 c4Len = zwimIn.m_c1PLLength;
        for (tCIDLib::TCard4 c4Index = 6; c4Index < c4Len; c4Index++)
        {
            const tCIDLib::TCard1 c1Cur = zwimIn[c4Index];
            if (c1Cur == COMMAND_CLASS_SECURITY)
                m_bIsSecure = kCIDLib::True;
            else if (c1Cur == COMMAND_CLASS_MANUFACTURER_SPECIFIC)
                m_bHasManSpec = kCIDLib::True;
        }

        //
        //  If it's a controller set it to disabled. We don't care about controllers
        //  but we want to keep it in the list so we don't keep rediscovering it
        //  on every replication.
        //
        //  If we have manspec, then it's not secure. So we can move to GetManSpec
        //  now. Else, if secure, it could be a secure class so we have to check that.
        //  Else, we know we cannot auto-id it, so go to NoAutoMatch.
        //
        //  Updating the state will bump the serial number.
        //
        if (m_bIsCtrl)
            eState(tZWaveUSB3Sh::EUnitStates::Disabled);
        else if (m_bHasManSpec)
            eState(tZWaveUSB3Sh::EUnitStates::GetManSpec);
        else if (m_bIsSecure)
            eState(tZWaveUSB3Sh::EUnitStates::GetSecureCls);
        else
            eState(tZWaveUSB3Sh::EUnitStates::NoAutoMatch);

        m_enctNextProc = 0;
        m_c4TryCount = 0;
    }
     else if (m_eState == tZWaveUSB3Sh::EUnitStates::Ready)
    {
        if (m_punitThis)
            m_punitThis->UnitIsOnline();

        // If any queued up msgs, then send them
        SendWakeupQ();

        m_enctNextProc = 0;
        m_c4TryCount = 0;
    }
}


//
//  If we are in a state to deal with such things, the driver will pass along to us any
//  command class msgs that are for our unit. We will handle some messages ourself, some
//  others we will will pass on to the unit object if we have one.
//
//  The driver will handle any exceptions that propagate out of here and log something.
//
//  NOTE THAT we must only respond to stuff that changes our config info if we are in
//  a state that would allow for that sort of change. Else, some random or delayed msg
//  from the other unit we were to receive could whack our unit state and create a mess.
//
//  If we are in the right state, we can then move us forward to the next available
//  state. If we change the state, we reset the next process time to zero so that
//  the next call to Process() will take the next step. We also reset the try counter
//  for him since he doesn't have any point at which he knows the state changed.
//
//  ALSO we are not allowed to make config changes after it hits GetInitVals or beyond
//  so this insures that we don't accidentally change something.
//
tCIDLib::TVoid
TZWUnitInfo::HandleCCMsg(const  tCIDLib::TCard1 c1Class
                        , const tCIDLib::TCard1 c1Cmd
                        , const TZWInMsg&       zwimIn)
{
    // If this guy is disabled, ignore this
    if (m_eState == tZWaveUSB3Sh::EUnitStates::Disabled)
        return;

    //
    //  It's a command class msg. We handle some ourself, pass others to our unit
    //  object if we have created it yet.
    //
    tCIDLib::TBoolean bPassToUnit = kCIDLib::False;
    if (c1Class == COMMAND_CLASS_NODE_NAMING)
    {
        if (c1Cmd == NODE_NAMING_NODE_NAME_REPORT)
        {
            //  Only respond to this if we are in GetName state
            if (m_eState == tZWaveUSB3Sh::EUnitStates::GetName)
            {
                // Only actually take the name if the name hasn't been set otherwise
                if (m_eNameSrc < tZWaveUSB3Sh::EUnitNameSrcs::Unit)
                {
                    //
                    //  There's a bit of work to this one, so the naming class provides a
                    //  helper. It will also normalize it to valid CQC field name
                    //  requirements.
                    //
                    TString strNewName;
                    if (TZWUSB3CCNaming::bExtractNodeName(zwimIn, strNewName))
                    {
                        // If the driver say it's unique, then let's store it.
                        if (m_pmzwciOwner->bUSB3IsUniqueUnitName(strNewName))
                        {
                            if (facZWaveUSB3Sh().bMediumTrace())
                            {
                                facZWaveUSB3Sh().LogTraceMsg
                                (
                                    L"Taking name from unit 0x%(1). Name=%(2)"
                                    , TCardinal(m_c1Id, tCIDLib::ERadices::Hex)
                                    , strNewName
                                );
                            }

                            // Store it and remember it's a unit provided name
                            m_strName = strNewName;
                            m_eNameSrc = tZWaveUSB3Sh::EUnitNameSrcs::Unit;
                            IncSerialNum();
                        }
                         else
                        {
                            if (facZWaveUSB3Sh().bMediumTrace())
                            {
                                facZWaveUSB3Sh().LogTraceErr
                                (
                                    L"Got non-unique name (%(1)) unit 0x%(2), ignored it"
                                    , strNewName
                                    , TCardinal(m_c1Id, tCIDLib::ERadices::Hex)
                                );
                            }
                        }
                    }
                }

                // We can move to WaitApprove now since we have the info we need
                eState(tZWaveUSB3Sh::EUnitStates::WaitApprove);
                m_c4TryCount = 0;
                m_enctNextProc = 0;
            }
        }
    }
     else if (c1Class == COMMAND_CLASS_MANUFACTURER_SPECIFIC)
    {
        if (c1Cmd == MANUFACTURER_SPECIFIC_REPORT)
        {
            //  Only respond to this if we are in GetManSpec state
            if (m_eState == tZWaveUSB3Sh::EUnitStates::GetManSpec)
            {
                tCIDLib::TCard8 c8NewId = zwimIn.c1CCByteAt(2);
                c8NewId <<= 8;
                c8NewId |= zwimIn.c1CCByteAt(3);
                c8NewId <<= 8;
                c8NewId |= zwimIn.c1CCByteAt(4);
                c8NewId <<= 8;
                c8NewId |= zwimIn.c1CCByteAt(5);
                c8NewId <<= 8;
                c8NewId |= zwimIn.c1CCByteAt(6);
                c8NewId <<= 8;
                c8NewId |= zwimIn.c1CCByteAt(7);

                // If we have info on it. If so, then let's store it away
                TString strErr;
                TZWDevInfo deviTmp;
                if (facZWaveUSB3Sh().bQueryDevInfo(c8NewId, deviTmp, strErr))
                {
                    StoreDevInfo(deviTmp, kCIDLib::True);

                    //
                    //  If we have naming support, move to naming state. Else
                    //  move to wait approve state.
                    //
                    if (m_deviUnit.bSupportsClass(tZWaveUSB3Sh::ECClasses::Naming))
                        eState(tZWaveUSB3Sh::EUnitStates::GetName);
                    else
                        eState(tZWaveUSB3Sh::EUnitStates::WaitApprove);

                    m_c4TryCount = 0;
                    m_enctNextProc = 0;
                }
                 else
                {
                    // Move to NoAutoMatch since it's not going to happen
                    eState(tZWaveUSB3Sh::EUnitStates::NoAutoMatch);
                    m_c4TryCount = 0;
                    m_enctNextProc = 0;
                }
            }
        }
    }
     else if (c1Class == COMMAND_CLASS_SECURITY)
    {
        if (c1Cmd == SECURITY_COMMANDS_SUPPORTED_REPORT)
        {
            // Only respond to this if we are in GetSecureCls state
            if (m_eState == tZWaveUSB3Sh::EUnitStates::GetSecureCls)
            {
                //
                //  This one is somewhat unusual in that we can get more than one. Each
                //  one indicates how many more are left.
                //
                const tCIDLib::TCard4 c4ReportsLeft = zwimIn.c1CCByteAt(2);
                if (facZWaveUSB3Sh().bMediumTrace())
                {
                    facZWaveUSB3Sh().LogTraceMsg
                    (
                        L"Got secure CCs report for unit %(1), name=%(2). %(3) more left"
                        , TCardinal(m_c1Id, tCIDLib::ERadices::Hex)
                        , m_strName
                        , TCardinal(c4ReportsLeft)
                    );
                }

                // Get the ones out we got this time
                tCIDLib::TCard4 c4ByteInd = 3;
                const tCIDLib::TCard4 c4ClassBytes = zwimIn.c4CCBytes();
                while (c4ByteInd < c4ClassBytes)
                {
                    const tCIDLib::TCard1 c1Cur = zwimIn.c1CCByteAt(c4ByteInd++);

                    if (c1Cur == COMMAND_CLASS_MARK)
                    {
                        //
                        //  Don't care about controlled classes so break out on the
                        //  divider mark.
                        //
                        break;
                    }
                     else if (c1Cur >= 0xF1)
                    {
                        // It's an extended class tht we don't understand, so skip it
                        c4ByteInd++;
                    }
                     else
                    {
                        //
                        //  If we have man spec, then that's all we need to know. We
                        //  set our man spec secure class. Later when we get all of
                        //  the reports, we'll use this to move forward.
                        //
                        if (c1Cur == COMMAND_CLASS_MANUFACTURER_SPECIFIC)
                        {
                            m_bHasManSpec = kCIDLib::True;
                            m_bManSpecSecure = kCIDLib::True;
                        }
                    }
                }

                // If we have gotten them all, then move forward
                if (!c4ReportsLeft)
                {
                    if (m_bHasManSpec)
                        eState(tZWaveUSB3Sh::EUnitStates::GetManSpec);
                    else
                        eState(tZWaveUSB3Sh::EUnitStates::NoAutoMatch);

                    m_c4TryCount = 0;
                    m_enctNextProc = 0;
                }
            }
        }
    }
     else if (c1Class == COMMAND_CLASS_WAKE_UP)
    {
        // We will process this post-unit, but we want them to see it
        if (c1Cmd == WAKE_UP_NOTIFICATION)
            bPassToUnit = kCIDLib::True;
    }
     else if (c1Class == COMMAND_CLASS_NO_OPERATION)
    {
        //
        //  For now, not using these, but we could later. Note that this is a fake
        //  message generated by the Z-Stick in response to no-op msgs that we send
        //  out ourself (i.e. not ones he does internally.)
        //
    }

     else
    {
        // Nothing specially handled by us, so let the unit see it
        bPassToUnit = kCIDLib::True;
    }

    //
    //  It's nothing special, if we have our unit object, pass it on to the unit
    //  handler for evaluation. Only do this if we are in a state that can make use
    //  of it. That limits a lot of redundant state checks in the units.
    //
    if (bPassToUnit
    &&  m_punitThis
    && (m_punitThis->eState() >= tZWaveUSB3Sh::EUnitStates::FirstViableState))
    {
        try
        {
            m_punitThis->HandleCCMsg(c1Class, c1Cmd, zwimIn);
        }

        catch(TError& errToCatch)
        {
            if (facZWaveUSB3Sh().eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }
        }

        catch(...)
        {
        }
    }

    //
    //  Do this after the unit has seen it. Some units queue up stuff that they can
    //  only send on wakeup. If we did this before the units saw it, we wouldn't send
    //  these messages until the next wakeup.
    //
    if (c1Class == COMMAND_CLASS_WAKE_UP)
    {
        if (c1Cmd == WAKE_UP_NOTIFICATION)
        {
            // Send any queued up msgs
            SendWakeupQ();

            if (m_eState == tZWaveUSB3Sh::EUnitStates::WaitWakeup)
            {
                if (facZWaveUSB3Sh().bMediumTrace())
                {
                    facZWaveUSB3Sh().LogTraceMsg
                    (
                        L"Got wakeup for unit %(1), trying to get info", m_strName
                    );
                }

                //
                //  We treat this as a response to our ping requests. They aren't
                //  working because this guy is (we know now) a non-listener. So
                //  we move up to GetUnitInfo while he's still awake.
                //
                eState(tZWaveUSB3Sh::EUnitStates::GetUnitInfo);

                m_c4TryCount = 0;
                m_enctNextProc = 0;
            }
             else if ((m_eState == tZWaveUSB3Sh::EUnitStates::GetSecureCls)
                  ||  (m_eState == tZWaveUSB3Sh::EUnitStates::GetManSpec)
                  ||  (m_eState == tZWaveUSB3Sh::EUnitStates::GetName))
            {
                //
                //  For these, just zero the next processing time, so that we can
                //  start trying to move forward again while it's awake.
                //
                m_c4TryCount = 0;
                m_enctNextProc = 0;
            }
        }

        // The unit typically needs to see this one
        bPassToUnit = kCIDLib::True;
    }
}


//
//  Look for an option that should have a Int4 value. If we can't find it or
//  convert it, we return the default.
//
tCIDLib::TInt4
TZWUnitInfo::i4QueryOptVal( const   TString&        strKey
                            , const tCIDLib::TInt4  i4Default) const
{
    const TKeyValuePair* pkvalOpt = m_colOptVals.pobjFindByKey(strKey);
    if (!pkvalOpt)
        return i4Default;

    tCIDLib::TInt4 i4Ret;
    if (!pkvalOpt->strValue().bToInt4(i4Ret, tCIDLib::ERadices::Auto))
        i4Ret = i4Default;

    return i4Ret;
}


//
//  Once we figure out the type of the unit (via man spec ids or user selection), this
//  is called to create the appropriate unit class handlers.
//
//  This also has to be called on the client side because we need the unit objects in
//  order to know what the available unit options are. So we get a flag that lets us
//  do anything special (or not do things we might normally do.) That's also why it
//  is public.
//
tCIDLib::TVoid TZWUnitInfo::MakeUnitObject(const tCIDLib::TBoolean bClientSide)
{
    // If debugging, make sure device info has been loaded.
    CIDAssert(m_deviUnit.bIsValid(), L"Can't make unit object without device info");

    // Normally wouldn't happen but often does from the client side
    if (m_punitThis)
    {
        try
        {
            delete m_punitThis;
            m_punitThis = nullptr;
        }

        catch(...)
        {
        }
    }

    // Map the class name in the info to our enum
    const tZWaveUSB3Sh::EUnitClasses eClass = tZWaveUSB3Sh::eXlatEUnitClasses
    (
        m_deviUnit.strUnitClass()
    );

    // If it wasn't mapped, we do nothing
    if (eClass == tZWaveUSB3Sh::EUnitClasses::Count)
    {
        facZWaveUSB3Sh().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kZW3ShErrs::errcDbg_UnitClassNotFound
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , m_deviUnit.strUnitClass()
            , TCardinal(m_c1Id)
        );
    }

    // Else let's gen up the proper one
    switch(eClass)
    {
        case tZWaveUSB3Sh::EUnitClasses::TAeonBadMFUnit :
            m_punitThis = new TAeonBadMFUnit(this);
            break;

        case tZWaveUSB3Sh::EUnitClasses::TBinNotSensorUnit :
            m_punitThis = new TBinNotSensorUnit(this);
            break;

        case tZWaveUSB3Sh::EUnitClasses::TDimmerUnit :
            m_punitThis = new TDimmerUnit(this);
            break;

        case tZWaveUSB3Sh::EUnitClasses::TDualBinSensorUnit :
            m_punitThis = new TDualBinSensorUnit(this);
            break;

        case tZWaveUSB3Sh::EUnitClasses::TGenRGBWUnit :
            m_punitThis = new TGenRGBWUnit(this);
            break;

        case tZWaveUSB3Sh::EUnitClasses::TGenBinSensorUnit :
            m_punitThis = new TGenBinSensorUnit(this);
            break;

        case tZWaveUSB3Sh::EUnitClasses::TGenDimmerUnit :
            m_punitThis = new TGenDimmerUnit(this);
            break;

        case tZWaveUSB3Sh::EUnitClasses::TGenSceneActUnit :
            m_punitThis = new TGenSceneActUnit(this);
            break;

        case tZWaveUSB3Sh::EUnitClasses::TLightSwitchUnit :
            m_punitThis = new TLightSwitchUnit(this);
            break;

        case tZWaveUSB3Sh::EUnitClasses::TLockUnit :
            m_punitThis = new TLockUnit(this);
            break;

        case tZWaveUSB3Sh::EUnitClasses::TLockNotUnit :
            m_punitThis = new TLockNotUnit(this);
            break;

        case tZWaveUSB3Sh::EUnitClasses::TMultiChComboUnit :
            m_punitThis = new TMultiChComboUnit(this);
            break;

        case tZWaveUSB3Sh::EUnitClasses::TNChOutletUnit :
            m_punitThis = new TNChOutletUnit(this);
            break;

        case tZWaveUSB3Sh::EUnitClasses::TNonMCMultiSensorUnit :
            m_punitThis = new TNonMCMultiSensorUnit(this);
            break;

        case tZWaveUSB3Sh::EUnitClasses::TOutletUnit :
            m_punitThis = new TOutletUnit(this);
            break;

        case tZWaveUSB3Sh::EUnitClasses::TTempSPUnit :
            m_punitThis = new TTempSPUnit(this);
            break;

        case tZWaveUSB3Sh::EUnitClasses::TThermoUnit :
            m_punitThis = new TThermoUnit(this);
            break;

        case tZWaveUSB3Sh::EUnitClasses::TZWController :
            m_punitThis = new TZWController(this);
            break;

        default :
            // Not something we understand
            break;
    };

    //
    //  If we created it, update us with any unit options. Only the specific class
    //  understands the options it supports. If we didn't create a unit object, we
    //  go into failed mode.
    //
    if (m_punitThis)
    {
        //
        //  Now tell the unit to do any work it needs to get prepared for use. Mostly
        //  this involves parsing extra info from the device info file and creating
        //  CC impl objects. If this fails, we mark ourself failed. It can't send any
        //  msgs here since we don't provide it with a driver interface.
        //
        try
        {
            //
            //  Parse any extra info and store it away if needed. Sometimes this is
            //  needed by QueryUnitOptions(), so we give them a chance to do it up
            //  front if needed. They can store the info away. Or they can just wait
            //  and parse that data in bPrepare() below if that only need it there.
            //
            m_punitThis->bParseExtraInfo();

            //
            //  Do a dummy query of the unit's options. This will cause default values
            //  to be stored on us for any options we don't have a value for yet. If
            //  this is a brand new unit, it will default them all. Else it will pick
            //  up any new ones.
            //
            tCIDMData::TAttrList colAttrs;
            QueryUnitAttrs(colAttrs);

            // Now let it prepare, which will create any CC impls. If it fails, we fail
            if (!m_punitThis->bPrepare())
                eState(tZWaveUSB3Sh::EUnitStates::Failed);
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            eState(tZWaveUSB3Sh::EUnitStates::Failed);
        }

        catch(...)
        {
            eState(tZWaveUSB3Sh::EUnitStates::Failed);
        }
    }
     else
    {
        eState(tZWaveUSB3Sh::EUnitStates::Failed);
    }
}


//
//  This is called when this unit's containing configuration object is about to be
//  put into use. This lets us do stuff that we only actually need/want to do when
//  we are becoming the active configuration for the driver. There's not a good
//  place to do these things otherwise.
//
//  We can't do any unit I/O, this is just for initialization of runtime members.
//
//  We also, if we have valid device info, create our unit object, since this is
//  called upon load of old config and upon replication config being assigned to
//  us. It also may be set, since it gets set when device info is first assigned
//  during discovery, but that's fine. The call will delete any old one first. That
//  may be redundant, but it's not worth worrying about. This insures we get one
//  relevant for the device info we have now.
//
tCIDLib::TVoid
TZWUnitInfo::PrepForUse(const   tCIDLib::TEncodedTime   enctNow
                        ,       MZWaveCtrlIntf* const   pmzwciOwner)
{
    //
    //  Set up a random time of the day to re-do our auto-config. Make sure it's
    //  after now.
    //
    tCIDLib::TCard4 c4Hour = facCIDCrypto().c4GetRandom() % 12;
    tCIDLib::TCard4 c4Min = facCIDCrypto().c4GetRandom() % 60;
    TTime tmAutoCfg(tCIDLib::ESpecialTimes::CurrentDate);
    tmAutoCfg += (kCIDLib::enctOneDay
                 + (kCIDLib::enctOneHour * c4Hour)
                 + (kCIDLib::enctOneMinute * c4Min));
    m_enctNextAutoCfg = tmAutoCfg.enctTime();

    // Run it up by days until it's after now
    while (m_enctNextAutoCfg < enctNow)
        m_enctNextAutoCfg += kCIDLib::enctOneDay;

    // And we need a pointer to our owning driver via the controller interface
    m_pmzwciOwner = pmzwciOwner;

    // If we can, make our unit object. Say this is not client side, it's server side
    if (m_deviUnit.bIsValid())
        MakeUnitObject(kCIDLib::False);

    //
    //  If our state is Ready, set it back to GetInitVals since that is needed to
    //  force us to create fields and get initial values.
    //
    if (m_eState == tZWaveUSB3Sh::EUnitStates::Ready)
        m_eState = tZWaveUSB3Sh::EUnitStates::GetInitVals;
}


// Just a passthrough to allow our CC imples to get to our driver
tCIDLib::TVoid
TZWUnitInfo::QueueEventTrig(const   tCQCKit::EStdDrvEvs eEvent
                            , const TString&            strFld
                            , const TString&            strVal1
                            , const TString&            strVal2
                            , const TString&            strVal3
                            , const TString&            strVal4)
{
    m_pmzwciOwner->USB3QEventTrig(eEvent, strFld, strVal1, strVal2, strVal3, strVal4);
}


//
//  The driver will call this on each object periodically. It gives an opportunity to do
//  any work required to move our unit towards ready state. If ready, then it lets us do
//  any periodic processing required.
//
//  Once we get to the point where we've created our unit object, we will start asking
//  it to do things as well.
//
//  All this is async, we just send and wait for the result to come back. Our
//  HandleCCMsg() method watches for them and, if in the appropriate state for that
//  sort of msg, moves us to the next state that is valid for us.
//
//  The m_enctNextProc is used to either remember the next minimum time we want. When
//  HandleCCMsg() moves us forward, it zeros this so that we get back here and do the
//  next step as soon as possible. If we don't get the response, then we get back in
//  here on the same state and try again.
//
//  HandleCCMsg() zeros m_c4TryCount when it moves us forward, and we bump it for each
//  time we send here. That way we can time out on some states if that is appropriate.
//  Scenarios where we time out are:
//
//      - If we get a couple failures on GetUnitInfo, we fall back to ping state again
//        since, it could be a frequent listener and won't wake up without the ping.
//        We can't know if it is yet, but if it is then without the ping we'll never
//        get anywhere.
//      - We aren't going to hold things up over the node name. We'll try it once if
//        it's supported, but then just move on if it fails.
//
tCIDLib::TVoid TZWUnitInfo::Process()
{
    // If this one is marked as disabled, then ignore it
    if (m_eState == tZWaveUSB3Sh::EUnitStates::Disabled)
        return;

    // If not time yet, do nothing
    const tCIDLib::TEncodedTime enctNow = TTime::enctNow();
    if (enctNow < m_enctNextProc)
        return;

    try
    {
        // Based on our current state, let's decide what to do
        switch(m_eState)
        {
            case tZWaveUSB3Sh::EUnitStates::InitUnit :
            {
                // Shouldn't see this her, but just in case we don't want to go wild
                m_enctNextProc = TTime::enctNowPlusSecs(30);
                break;
            }

            case tZWaveUSB3Sh::EUnitStates::GetUnitInfo :
            {
                //
                //  If we have already done this, then we move to wait wakeup
                //  if it's a non-listener. Else we keep trying. First time we'll
                //  do the usual progressively longer timeout.
                //
                if (m_c4TryCount && !bAlwaysOn())
                {
                    facZWaveUSB3Sh().LogTraceErr
                    (
                        L"Unit '%(1)' failed unit info query twice, waiting for wakeup"
                        , strName()
                    );
                    eState(tZWaveUSB3Sh::EUnitStates::WaitWakeup);
                    m_enctNextProc = TTime::enctNowPlusSecs(30);
                    m_c4TryCount = 0;
                }
                 else
                {
                    TZWOutMsg zwomOut;
                    zwomOut.Reset
                    (
                        c1Id(), FUNC_ID_ZW_REQUEST_NODE_INFO, tZWaveUSB3Sh::EMsgPrios::Async
                    );
                    zwomOut.Finalize(kCIDLib::False, kCIDLib::False);
                    m_pmzwciOwner->USB3Send(zwomOut);

                    m_enctNextProc = enctCalcNextProc(15);
                }
                break;
            }

            case tZWaveUSB3Sh::EUnitStates::WaitWakeup :
            case tZWaveUSB3Sh::EUnitStates::Disabled :
            case tZWaveUSB3Sh::EUnitStates::Failed :
                // Nothing to do for now, just set another long next time
                m_enctNextProc = TTime::enctNowPlusMins(30);
                break;

            case tZWaveUSB3Sh::EUnitStates::WaitDevInfo :
                //
                //  No man id query support, so nothing to do until the user tells
                //  us what we are.
                //
                m_enctNextProc = TTime::enctNowPlusSecs(30);
                break;

            case tZWaveUSB3Sh::EUnitStates::WaitApprove :
                //
                //  Our type is known now, so the user needs to approve us.
                //
                m_enctNextProc = TTime::enctNowPlusSecs(30);
                break;

            case tZWaveUSB3Sh::EUnitStates::GetSecureCls :
            {
                //
                //  Queue up a request for this guy's secure classes. This may cause
                //  multiple reports. We assume we won't get here unless our unit is
                //  awake, so we send directly.
                //
                TZWOutMsg zwomOut;
                zwomOut.Reset
                (
                    c1Id()
                    , COMMAND_CLASS_SECURITY
                    , SECURITY_COMMANDS_SUPPORTED_GET
                    , SECURITY_COMMANDS_SUPPORTED_REPORT
                    , 2
                    , tZWaveUSB3Sh::EMsgPrios::Async
                );
                zwomOut.AppendTransOpts();
                zwomOut.AppendCallback();

                // Send this one securely always
                zwomOut.Finalize(m_bFreqListener, kCIDLib::True);
                m_pmzwciOwner->USB3Send(zwomOut);

                m_enctNextProc = enctCalcNextProc(15);
                break;
            }

            case tZWaveUSB3Sh::EUnitStates::GetManSpec :
            {
                //
                //  We assume we won't get here unless our unit is awake, so we send
                //  directly.
                //
                //  We don't have device info at this point!! So we use our temp
                //  frequent listener/secure flags for this.
                //
                TZWOutMsg zwomOut;
                zwomOut.Reset
                (
                    c1Id()
                    , COMMAND_CLASS_MANUFACTURER_SPECIFIC
                    , MANUFACTURER_SPECIFIC_GET
                    , MANUFACTURER_SPECIFIC_REPORT
                    , 2
                    , tZWaveUSB3Sh::EMsgPrios::Async
                );
                zwomOut.AppendTransOpts();
                zwomOut.AppendCallback();
                zwomOut.Finalize
                (
                    m_bFreqListener, m_bManSpecSecure
                );
                m_pmzwciOwner->USB3Send(zwomOut);

                m_enctNextProc = enctCalcNextProc(15);
                break;
            }

            case tZWaveUSB3Sh::EUnitStates::GetName :
            {
                if (m_c4TryCount > 1)
                {
                    facZWaveUSB3Sh().LogTraceErr
                    (
                        L"Unit '%(1)' failed name query twice, keeping default name"
                        , strName()
                    );
                    eState(tZWaveUSB3Sh::EUnitStates::WaitApprove);
                    m_enctNextProc = 0;
                    m_c4TryCount = 0;
                }
                 else
                {
                    //
                    //  We assume we won't get here unless our unit is awake, so we send
                    //  directly. We have device info by now.
                    //
                    TZWOutMsg zwomOut;
                    zwomOut.Reset
                    (
                        c1Id()
                        , COMMAND_CLASS_NODE_NAMING
                        , NODE_NAMING_NODE_NAME_GET
                        , NODE_NAMING_NODE_NAME_REPORT
                        , 2
                        , tZWaveUSB3Sh::EMsgPrios::Async
                    );
                    zwomOut.AppendTransOpts();
                    zwomOut.AppendCallback();
                    zwomOut.Finalize
                    (
                        bFreqListener()
                        , m_deviUnit.bSecureClass(tZWaveUSB3Sh::ECClasses::Naming)
                    );
                    m_pmzwciOwner->USB3Send(zwomOut);
                    m_enctNextProc = enctCalcNextProc(15);
                }
                break;
            }

            case tZWaveUSB3Sh::EUnitStates::NoAutoMatch :
            {
                // Just wait good while
                m_enctNextProc = TTime::enctNowPlusSecs(30);
                break;
            }

            //
            //  These states are for the actual unit objects to deal with. For getting
            //  initial values, we make a one time call to a a method on the unit object
            //  to let it queue up queries to get its needed values, then we move to
            //  ready state.
            //
            case tZWaveUSB3Sh::EUnitStates::GetInitVals :
            {
                try
                {
                    m_punitThis->GetInitVals();
                }

                catch(TError& errToCatch)
                {
                    if (facZWaveUSB3Sh().eVerboseLevel() >= tCQCKit::EVerboseLvls::High)
                    {
                        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                        TModule::LogEventObj(errToCatch);
                    }

                    // Fall through
                }

                catch(...)
                {
                    // Fall through
                }

                eState(tZWaveUSB3Sh::EUnitStates::Ready);
                break;
            }

            case tZWaveUSB3Sh::EUnitStates::Ready :
            {
                //
                //  If it is past time for our next auto-configuration, then let's do
                //  that. We send it out once a day to just make sure things stay
                //  correctly configured.
                //
                if (enctNow > m_enctNextAutoCfg)
                {
                    //
                    //  Move the time forward first until we get to the same time
                    //  a day from now.
                    //
                    const tCIDLib::TEncodedTime enctNow = TTime::enctNow();
                    while (m_enctNextAutoCfg < enctNow)
                        m_enctNextAutoCfg += kCIDLib::enctOneDay;

                    if (facZWaveUSB3Sh().bLowTrace())
                    {
                        TTime tmFmt(m_enctNextAutoCfg);
                        tmFmt.strDefaultFormat(TTime::strMMDD_24HHMM());
                        facZWaveUSB3Sh().LogTraceMsg
                        (
                            L"Unit %(1), next auto-config scheduled for: %(2)"
                            , strName()
                            , tmFmt
                        );
                    }

                    try
                    {
                        TString strErr;
                        if (!bDoAutoCfg(kCIDLib::False, strErr))
                        {
                            facZWaveUSB3Sh().LogMsg
                            (
                                CID_FILE
                                , CID_LINE
                                , kZW3ShErrs::errcUnit_AutoCfgFailed
                                , strErr
                                , tCIDLib::ESeverities::Failed
                                , tCIDLib::EErrClasses::AppStatus
                                , strName()
                            );
                        }
                    }

                    catch(TError& errToCatch)
                    {
                        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                        TModule::LogEventObj(errToCatch);

                        facZWaveUSB3Sh().LogTraceErr
                        (
                            L"Unit '%(1)' failed scheduled auto-config", strName()
                        );

                        // Fall through
                    }
                }

                //
                //  Do a fairly slow call to the unit's Process method, so that
                //  it can do various things as required.
                //
                try
                {
                    m_punitThis->Process();

                    // Reset the try count since this worked
                    m_c4TryCount = 0;

                    //
                    //  We do this on a 15 second cycle, but the individual CCs in the
                    //  unit will have their own stamps. We just want to call often
                    //  enough that they can have reasonable time accuracy. We can't
                    //  call too quickly since we want there to be time for any previously
                    //  kicked off operation to have finished.
                    //
                    //  We don't call enctCalcNextProc() here.
                    //
                    m_enctNextProc = TTime::enctNowPlusSecs(15);
                }

                catch(TError& errToCatch)
                {
                    if (facZWaveUSB3Sh().eVerboseLevel() >= tCQCKit::EVerboseLvls::High)
                    {
                        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                        TModule::LogEventObj(errToCatch);
                    }

                    m_enctNextProc = enctCalcNextProc(30);
                }

                catch(...)
                {
                    m_enctNextProc = enctCalcNextProc(30);
                }
                break;
            }

            default :
            {
                // Shouldn't happen, but just in case, make sure we don't go wild
                m_enctNextProc = TTime::enctNowPlusSecs(10);

                #if CID_DEBUG_ON
                facZWaveUSB3Sh().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , L"Process() got an unknown state"
                    , tCIDLib::ESeverities::Status
                );
                #endif
                break;
            }
        };
    }

    catch(TError& errToCatch)
    {
        if (facZWaveUSB3Sh().eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
            facZWaveUSB3Sh().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kZW3ShErrs::errcUnit_ExceptInProc
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
                , strName()
            );
        }
    }

    catch(...)
    {
        if (facZWaveUSB3Sh().eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
        {
            facZWaveUSB3Sh().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kZW3ShErrs::errcUnit_SysExceptInProc
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
                , strName()
            );
        }
    }
}


//
//  This unit fills in a list of attribute data objects with info on all of the editable
//  attributes he has. Some of these are 'unit options' which vary by unit type. And
//  some are ones we add here, to edit some of our own members directly. The unit option
//  values are stored in m_colOptVals.
//
tCIDLib::TVoid TZWUnitInfo::QueryUnitAttrs(tCIDMData::TAttrList& colToFill)
{
    // We have to have a unit object for this to be valid
    if (!m_punitThis)
        return;

    // Let the unit add any of his options, but first to a temp list
    TAttrData adatTmp;
    tCIDMData::TAttrList colUnitOpts;
    m_punitThis->QueryUnitAttrs(colUnitOpts, adatTmp);

    // Go through them and for any we don't have stored values for, store one
    TString strVal;
    const tCIDLib::TCard4 c4Count = colUnitOpts.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        // If the current one isn't a separator, then check it
        TAttrData& adatCur = colUnitOpts[c4Index];
        if (adatCur.eType() != tCIDMData::EAttrTypes::Separator)
        {
            TKeyValuePair* pkvalTar = m_colOptVals.pobjFindByKey(adatCur.strId());
            if (!pkvalTar)
            {
                adatCur.FormatToText(strVal);
                m_colOptVals.objAdd(TKeyValuePair(adatCur.strId(), strVal));
            }
        }
    }

    //
    //  Now we need to add some standard ones to the caller's list. We want these
    //  to be first. They apply to all units and they are not options.
    //
    adatTmp.Set(L"Unit Options:", TString::strEmpty(), tCIDMData::EAttrTypes::Separator);
    colToFill.objAdd(adatTmp);

    //
    //  Indicate it must be single line and it is required. Also limit to in-place only
    //  since it will never be long or multi-line, and that way we don't have to do any
    //  extra validation in the visual editing scenario.
    //
    adatTmp.Set
    (
        L"Name"
        , kZWaveUSB3Sh::strUAttr_Name
        , kCIDMData::strAttrLim_SingleReq
        , tCIDMData::EAttrTypes::String
        , tCIDMData::EAttrEdTypes::InPlace
    );
    adatTmp.SetString(m_strName);
    colToFill.objAdd(adatTmp);

    // Now append the temp list above to this one
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        colToFill.objAdd(colUnitOpts[c4Index]);
}


//
//  Resets just the stuff that comes from discovery and/or the device info file. We
//  need to do this separately if we want to force a unit to go back through the
//  discovery phase. The main Reset() above calls this to reset these bits.
//
//  We can do either a full reset of a rescan type reset. THe latter doesn't remove
//  all of the info.
//
tCIDLib::TVoid TZWUnitInfo::ResetDevInfo(const tCIDLib::TBoolean bRescan)
{
    m_deviUnit.Reset(bRescan);

    // If we have a unit object, delete it
    try
    {
        if (m_punitThis)
            delete m_punitThis;
    }

    catch(...)
    {
    }

    m_punitThis = nullptr;
    IncSerialNum();
}



//
//  The name is something that can be edited by the client side interface. If this is
//  called assume it is a user provided name.
//
const TString& TZWUnitInfo::strName(const TString& strToSet)
{
    m_strName = strToSet;
    m_eNameSrc = tZWaveUSB3Sh::EUnitNameSrcs::User;

    IncSerialNum();
    return m_strName;
}


//
//  Look for an option that should have a string value. If we can't find it, we return
//  the indicated default instead.
//
const TString&
TZWUnitInfo::strQueryOptVal(const   TString&    strKey
                            , const TString&    strDefault) const
{
    const TKeyValuePair* pkvalOpt = m_colOptVals.pobjFindByKey(strKey);
    if (!pkvalOpt)
        return strDefault;

    return pkvalOpt->strValue();
}


//
//  These are helpers that our unit and his CC impl objects use to send msgs.
//  We want to constrict their outgoing msgs so that we can make modifications
//  to them, or queue them up if our unit is not a live listener. This way we
//  can avoid a lot of redundant work.
//
//  We will end point encapsulate if needed. And, if we have to queue it for later
//  sending during wakeup, we change its priority to wakeup priority.
//
//  This may also be used by some other code that needs to send msgs to our unit,
//  so that we get appropriate queuing for non-listeners.
//
//  They can tell us they know for sure that the module is awake, in which case we
//  will treat it like a listening unit.
//
tCIDLib::TVoid
TZWUnitInfo::SendMsgAsync(  const   TZWOutMsg&          zwomToSend
                            , const tCIDLib::TBoolean   bKnownAwake)
{
    if (bAlwaysOn() || bKnownAwake)
    {
        m_pmzwciOwner->USB3Send(zwomToSend);
    }
     else
    {
        if (m_colWakeupQ.bIsFull(64))
        {
            if (facZWaveUSB3Sh().bMediumTrace())
            {
                facZWaveUSB3Sh().LogTraceErr
                (
                    L"The wakeup queue for unit '%(1)' is full, cannot queue up msg"
                    , m_strName
                );
            }
        }
         else
        {
            m_colWakeupQ.objAdd(zwomToSend);
        }
    }
}

tCIDLib::TVoid
TZWUnitInfo::SendWaitAck(const TZWOutMsg& zwomToSend, const tCIDLib::TBoolean bKnownAwake)
{
    if (bAlwaysOn() || bKnownAwake)
    {
        m_pmzwciOwner->USB3SendCCMsg(zwomToSend);
    }
     else
    {
        if (m_colWakeupQ.bIsFull(64))
        {
            if (facZWaveUSB3Sh().bMediumTrace())
            {
                facZWaveUSB3Sh().LogTraceErr
                (
                    L"The wakeup queue for unit '%(1)' is full, cannot queue up msg"
                    , m_strName
                );
            }
        }
         else
        {
            m_colWakeupQ.objAdd(zwomToSend);
        }
    }
}


//
//  This one can't be sent to a non-listener unless you know it's awake since we
//  wait for a reply.
//
tCIDLib::TVoid
TZWUnitInfo::SendWaitReply( const   TZWOutMsg&          zwomToSend
                            ,       TZWInMsg&           zwimReply
                            , const tCIDLib::TBoolean   bKnownAwake)
{
    if (!bAlwaysOn() && !bKnownAwake)
    {
        facZWaveUSB3Sh().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kZW3ShErrs::errcMsg_BlockOnNonListener
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotReady
            , m_strName
        );
    }

    m_pmzwciOwner->USB3SendCCMsg(zwomToSend, zwimReply);
}


//
//  Set the value for one of this unit's options. Normally this is not called. They
//  are set via attribute objects edited on the client side via a standard attribute
//  editor, see QueryUnitAttrs() and StoreUnitAttr(). StoreUnitAttr() will call us for
//  any editable values that are driven by unit options.
//
//  But we want to be able to directly set options as well, particuarly at the start
//  when we want to store initial default values for them.
//
tCIDLib::TVoid TZWUnitInfo::SetOption(const TString& strKey, const TString& strValue)
{
    TKeyValuePair* pkvalSet = m_colOptVals.pobjFindByKey(strKey);

    // If not found we need to add it, else update it
    if (!pkvalSet)
        m_colOptVals.objAdd(TKeyValuePair(strKey, strValue));
    else
        pkvalSet->strValue(strValue);

    IncSerialNum();
}


//
//  This is called by the client driver when the user edits a value in the attribute
//  editor. We need to store that new value.
//
//  There are a set that created by us that are directly editing our members. The rest
//  we just store the value by calling SetOption() above. The unit handler (and his
//  CC Impls) are not informed of this change. They will only react to this once the
//  changes are saved and the server driver redones all of the fields and such.
//
//  At this point the values have been validated, so we accept them as is.
//
tCIDLib::TVoid TZWUnitInfo::StoreUnitAttr(const TAttrData& adatToStore)
{
    const TString& strAttrId = adatToStore.strId();
    if (strAttrId.bStartsWithI(kZWaveUSB3Sh::strUAttr_Prefix))
    {
        //
        //  Its a unit level attribute (not an option), so we check the full id and
        //  update our appropriate member.
        //
        if (strAttrId.bCompareI(kZWaveUSB3Sh::strUAttr_Name))
        {
            m_strName = adatToStore.strValue();
        }
    }
     else
    {
        // These are key/value string pairs so format the value to text
        TString strVal;
        adatToStore.FormatToText(strVal);
        SetOption(strAttrId, strVal);
    }
}


//
//  Get the unit object for this info object. This is when you know it exists or you
//  want to get an exception if it doesn't.
//
const TZWaveUnit& TZWUnitInfo::unitThis() const
{
    if (!m_punitThis)
    {
        facZWaveUSB3Sh().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kZW3ShErrs::errcCfg_NoUnit
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotReady
            , m_strName
            , TCardinal(m_c1Id)
        );
    }
    return *m_punitThis;
}

TZWaveUnit& TZWUnitInfo::unitThis()
{
    if (!m_punitThis)
    {
        facZWaveUSB3Sh().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kZW3ShErrs::errcCfg_NoUnit
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotReady
            , m_strName
            , TCardinal(m_c1Id)
        );
    }
    return *m_punitThis;
}


// ---------------------------------------------------------------------------
//  TZWUnitInfo: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TZWUnitInfo::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Make sure we get the start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion != ZWaveUSB3Sh_UnitInfo::c2FmtVersion))
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

    strmToReadFrom  >> m_c1Id
                    >> m_eNameSrc
                    >> m_c4SerialNum
                    >> m_strName
                    >> m_eState
                    >> m_colOptVals

                    >> m_deviUnit

                    >> m_bIsCtrl
                    >> m_bListener
                    >> m_bFreqListener
                    >> m_bHasManSpec
                    >> m_bIsSecure
                    >> m_bManSpecSecure                    ;

    // Make sure we get the end marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);

    // Reset any stuff that is runtime only
    ResetRuntime();
}


tCIDLib::TVoid TZWUnitInfo::StreamTo(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << ZWaveUSB3Sh_UnitInfo::c2FmtVersion

                    << m_c1Id
                    << m_eNameSrc
                    << m_c4SerialNum
                    << m_strName
                    << m_eState
                    << m_colOptVals

                    << m_deviUnit

                    //
                    //  This is really temporary stuff. But, if we don't store it,
                    //  then any units that had gotten woken up, but not all the
                    //  way to some stopping point, would have to be woken up again
                    //  and that's a big PITA for the user. So we remember this stuff
                    //  and if the state indicates it, we start using it again when
                    //  streamed back in.
                    //
                    << m_bIsCtrl
                    << m_bListener
                    << m_bFreqListener
                    << m_bHasManSpec
                    << m_bIsSecure
                    << m_bManSpecSecure

                    << tCIDLib::EStreamMarkers::EndObject;
}


// ---------------------------------------------------------------------------
//  TZWUnitInfo: Protected, non-virtual methods
// ---------------------------------------------------------------------------

// A helper for
tCIDLib::TBoolean
TZWUnitInfo::bQueryFldName(const tCIDLib::TCard4 c4FldId, TString& strToFill) const
{
    return m_pmzwciOwner->bUSB3QueryFldName(c4FldId, strToFill);
}


// Helpers for our derivatives to store their field values
tCIDLib::TBoolean
TZWUnitInfo::bWriteFld(const tCIDLib::TCard4 c4FldId, const tCIDLib::TBoolean bToStore)
{
    return m_pmzwciOwner->bUSB3WriteFld(c4FldId, bToStore);
}

tCIDLib::TBoolean
TZWUnitInfo::bWriteFld(const tCIDLib::TCard4 c4FldId, const tCIDLib::TCard4 c4ToStore)
{
    return m_pmzwciOwner->bUSB3WriteFld(c4FldId, c4ToStore);
}

tCIDLib::TBoolean
TZWUnitInfo::bWriteFld(const tCIDLib::TCard4 c4FldId, const tCIDLib::TFloat8 f8ToStore)
{
    return m_pmzwciOwner->bUSB3WriteFld(c4FldId, f8ToStore);
}

tCIDLib::TBoolean
TZWUnitInfo::bWriteFld(const tCIDLib::TCard4 c4FldId, const tCIDLib::TInt4 i4ToStore)
{
    return m_pmzwciOwner->bUSB3WriteFld(c4FldId, i4ToStore);
}

tCIDLib::TBoolean
TZWUnitInfo::bWriteFld(const tCIDLib::TCard4 c4FldId, const TString& strToStore)
{
    return m_pmzwciOwner->bUSB3WriteFld(c4FldId, strToStore);
}


// A helper to look up a field name, since we hide access to the driver
tCIDLib::TCard4 TZWUnitInfo::c4FindFldId(const TString& strFldName) const
{
    return m_pmzwciOwner->c4USB3LookupFldId(strFldName);
}


// A helper to provide access to our driver's moniker
const TString& TZWUnitInfo::strDrvMoniker() const
{
    return m_pmzwciOwner->strUSB3Moniker();
}


tCIDLib::TVoid TZWUnitInfo::SetFldError(const tCIDLib::TCard4 c4FldId)
{
    m_pmzwciOwner->USB3SetFieldErr(c4FldId);
}



// ---------------------------------------------------------------------------
///  TZWUnitInfo: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  THIS IS ONLY for the driver to call, from the replication thread after a
//  replication is complete and there are new units that need initialization.
//
//  Query basic info about our unit from our local controller. This is done
//  synchronously because there's no way to indentify which unit an async reply would
//  be for. It's just coming from our local controller, so it should be quick. We
//  directly update this object with just the basic flags about listening, frequent
//  listening and is it a controller.
//
//  Because it is unreliable, and another one being in the input queue somehow could
//  cause us to get bad info, we first do a wait for such a message, to eat any
//  that might be there already. And use a special msg priority.
//
//  We assume that this info we store here cannot change without a replication because
//  it is stored during replication.
//
tCIDLib::TBoolean TZWUnitInfo::bProbeUnit(MZWaveCtrlIntf* const pmzwciOwner)
{
    //
    //  If this is a new unit, this is called before PrepForUse, so we need to get
    //  the owning driver pointer set here before we start doing msg I/O below.
    //
    m_pmzwciOwner = pmzwciOwner;

    // Make sure no lingering msg of this type is in the queue
    tCIDLib::TBoolean bRes;
    TZWInMsg zwimIn;
    tCIDLib::TCard4 c4BogusInfoCnt = 0;
    do
    {
        bRes = m_pmzwciOwner->bUSB3WaitForMsg
        (
            tZWaveUSB3Sh::EMsgTypes::Response
            , 0
            , FUNC_ID_ZW_GET_NODE_PROTOCOL_INFO
            , 0
            , 0
            , kCIDLib::enctOneMilliSec * 10
            , zwimIn
            , kCIDLib::False
        );

        if (bRes)
            c4BogusInfoCnt++;

    }   while (bRes);

    if (c4BogusInfoCnt && facZWaveUSB3Sh().bTraceEnabled())
    {
        facZWaveUSB3Sh().LogTraceErr
        (
            L"Got %(1) bogus protocol info msgs while probing unit %(2)"
            , TCardinal(c4BogusInfoCnt)
            , TCardinal(m_c1Id)
        );
    }

    //
    //  And now issue our own query. WE USE the special Local priority, which is the
    //  maximum priority to minimize the possibility of us timing out here because of
    //  other msgs taking a long time.
    //
    TZWOutMsg zwomOut;
    zwomOut.Reset(FUNC_ID_ZW_GET_NODE_PROTOCOL_INFO, tZWaveUSB3Sh::EMsgPrios::Local);
    zwomOut += c1Id();
    zwomOut.Finalize(kCIDLib::False, kCIDLib::False);
    m_pmzwciOwner->USB3Send(zwomOut);

    // And wait again for such a message
    bRes = m_pmzwciOwner->bUSB3WaitForMsg
    (
        tZWaveUSB3Sh::EMsgTypes::Response
        , 0
        , FUNC_ID_ZW_GET_NODE_PROTOCOL_INFO
        , 0
        , 0
        , kCIDLib::enctFiveSeconds
        , zwimIn
        , kCIDLib::False
    );

    // If no info, or the type is zero, then doesn't exist as far as our ctrl knows
    if (!bRes || !zwimIn[3])
    {
        if (facZWaveUSB3Sh().bLowTrace())
            facZWaveUSB3Sh().LogTraceErr(L"No node info available for id: %(1)", TCardinal(c1Id()));
        return kCIDLib::False;
    }

    // Looks reasonable to store the basic flags we need
    m_bListener = (zwimIn[0] & NODEINFO_LISTENING_SUPPORT) != 0;
    m_bFreqListener = (zwimIn[1] & 0x60) != 0;

    //
    //  See if it's a controller, of the master/secondary type, or installer tool, basically
    //  things we don't need to treat as a user interactable unit.
    //
    m_bIsCtrl = kCIDLib::False;
    const tCIDLib::TCard1 c1SpecType = zwimIn[5];
    switch(zwimIn[4])
    {
        case GENERIC_TYPE_STATIC_CONTROLLER :
            if ((c1SpecType == SPECIFIC_TYPE_PC_CONTROLLER)
            ||  (c1SpecType == SPECIFIC_TYPE_GATEWAY)
            ||  (c1SpecType == SPECIFIC_TYPE_STATIC_INSTALLER_TOOL))
            {
                m_bIsCtrl = kCIDLib::True;
            }
            break;

        case GENERIC_TYPE_GENERIC_CONTROLLER :
            if ((c1SpecType == SPECIFIC_TYPE_PORTABLE_REMOTE_CONTROLLER)
            ||  (c1SpecType == SPECIFIC_TYPE_PORTABLE_INSTALLER_TOOL))
            {
                m_bIsCtrl = kCIDLib::True;
            }
            break;

        default:
            break;
    };

    // If a controller, disable it
    if (m_bIsCtrl)
        eState(tZWaveUSB3Sh::EUnitStates::Disabled);

    if (facZWaveUSB3Sh().bTraceEnabled())
    {
        if (m_bIsCtrl)
        {
            facZWaveUSB3Sh().LogTraceMsg
            (
                L"Got info for unit '%(1)'. It is a controller and will be ignored"
                , strName()
            );
        }
         else
        {
            tZWaveUSB3Sh::EGenTypes eGenType = tZWaveUSB3Sh::eAltNumEGenTypes(zwimIn[4]);
            if (eGenType == tZWaveUSB3Sh::EGenTypes::Count)
                eGenType = tZWaveUSB3Sh::EGenTypes::None;

            facZWaveUSB3Sh().LogTraceMsg
            (
                L"Got info for unit '%(1)'. Listener=%(2), "
                L"FreqListener=%(3), GenType=%(4)"
                , strName()
                , facCQCKit().strBoolYesNo(m_bListener)
                , facCQCKit().strBoolYesNo(m_bFreqListener)
                , tZWaveUSB3Sh::strXlatEGenTypes(eGenType)
            );
        }
    }
    return kCIDLib::True;
}


//
//  When our Process() method is called, if the current step didn't work, we have to set
//  a next time to try it. This needs to be increased more as successive failures occur,
//  so as not to waste time trying to talk often to a currently unresponsive unit. And it
//  needs to be further adjusted for frequent listeners to avoid battery suckage.
//
//  We get a base seconds that would be used for the first round no matter what, so that
//  we can try it a couple times before we start extending the time. We adjust that as
//  needed and return the adjusted time time.
//
tCIDLib::TEncodedTime TZWUnitInfo::enctCalcNextProc(const tCIDLib::TCard4 c4BaseSeconds)
{
    // Thity minutes maximum adjustment
    const tCIDLib::TCard4 c4MaxTOSecs = 30 * 60;

    //
    //  Bump the count before we calculate. Upon first retry it will be zero since the
    //  previous success will have zeroed it.
    //
    m_c4TryCount++;

    // First assume a doubling of base seconds for each retry
    tCIDLib::TCard4 c4AdjSeconds = m_c4TryCount * c4BaseSeconds;

    // If it's a mains powered device, cut that time in half
    if (bAlwaysOn() && !bFreqListener())
        c4AdjSeconds /= 2;

    // If over the max, then clip to the max
    if (c4AdjSeconds > c4MaxTOSecs)
        c4AdjSeconds = c4MaxTOSecs;

    return TTime::enctNowPlusSecs(c4AdjSeconds);
}


// Increment our serial number, skipping 0 if we wrap
tCIDLib::TVoid TZWUnitInfo::IncSerialNum()
{
    m_c4SerialNum++;
    if (!m_c4SerialNum)
        m_c4SerialNum++;
}


//
//  This is just to avoid duplicating code in the constructors. Units really aren't
//  reused normally.
//
tCIDLib::TVoid TZWUnitInfo::Reset(const tCIDLib::TCard1 c1UnitId)
{
    // Remember if the id changed
    const tCIDLib::TBoolean bNewId(c1UnitId != m_c1Id);

    m_c1Id = c1UnitId;
    m_pmzwciOwner = nullptr;

    m_bIsCtrl           = kCIDLib::False;
    m_bListener         = kCIDLib::False;
    m_bFreqListener     = kCIDLib::False;
    m_bHasManSpec       = kCIDLib::False;
    m_bIsSecure         = kCIDLib::False;
    m_bManSpecSecure    = kCIDLib::False;
    m_c4TryCount         = 0;

    // Set a new random, non-zero serial number
    m_c4SerialNum = facCIDCrypto().c4GetRandom(kCIDLib::True) & 0x7FFFFFF;

    // Stuff the user provides
    m_colOptVals.RemoveAll();

    // Fully reset the device info stuff if we got it
    m_deviUnit.Reset(kCIDLib::False);

    // Set a default name if we got an id, else clear it
    if (m_c1Id)
    {
        m_strName = L"Unit_";
        m_strName.AppendFormatted(m_c1Id, tCIDLib::ERadices::Hex);
    }
     else
    {
        m_strName.Clear();
    }
    m_eNameSrc = tZWaveUSB3Sh::EUnitNameSrcs::Default;

    // Do any runtime stuff
    ResetRuntime();

    //
    //  And we have to be on the initial state, which is what is set until we do the
    //  prep of the newly found unit.
    //
    m_eState = tZWaveUSB3Sh::EUnitStates::InitUnit;
}


// This is called to reset some runtime only values to their correct initial states
tCIDLib::TVoid TZWUnitInfo::ResetRuntime()
{
    m_c4TryCount = 0;
    m_enctNextAutoCfg = TTime::enctNow();
    m_enctNextProc = 0;

    if (m_punitThis)
    {
        try
        {
            delete m_punitThis;
        }

        catch(...)
        {
        }
        m_punitThis  = nullptr;
    }
    m_colWakeupQ.RemoveAll();
}


//
//  This is called if we get a wakeup or NIF and are in ready state and are not an
//  always on type, which means that we could have msgs queued up waiting to be sent.
//
//  THey will be bumped up to special 'wakeup' state, so that they will be sent
//  quickly before the device goes back to sleep. We just queue them up to be sent
//  we don't wait around.
//
//  Also, just in case they have been there a long time, we update their callback
//  ids, since they could have wrapped around by now, many times actually in some
//  cases.
//
tCIDLib::TVoid TZWUnitInfo::SendWakeupQ()
{
    tCIDLib::TCard4 c4Count = m_colWakeupQ.c4ElemCount();
    if (c4Count)
    {
        if (facZWaveUSB3Sh().bMediumTrace())
        {
            facZWaveUSB3Sh().LogTraceMsg
            (
                L"Sending %(1) queued up msgs for %(2)", TCardinal(c4Count), m_strName);
        }

        TZWOutMsg zwomOut;
        while (c4Count)
        {
            zwomOut = m_colWakeupQ[--c4Count];
            m_colWakeupQ.RemoveAt(c4Count);
            zwomOut.ChangePriority(tZWaveUSB3Sh::EMsgPrios::Wakeup);
            zwomOut.UpdateCallback();

            try
            {
                m_pmzwciOwner->USB3Send(zwomOut);
            }

            catch(TError& errToCatch)
            {
                if (facZWaveUSB3Sh().eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
                {
                    errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                    TModule::LogEventObj(errToCatch);
                }
            }

            catch(...)
            {
            }
        }
    }
}


//
//  When we load our device info from file, this is called to store away the info
//  and do some busy work we want to insure gets done
//
tCIDLib::TVoid
TZWUnitInfo::StoreDevInfo(  const   TZWDevInfo&         deviOther
                            , const tCIDLib::TBoolean   bKnownAwake)
{
    // Store the incoming device info
    m_deviUnit = deviOther;

    if (facZWaveUSB3Sh().bMediumTrace())
    {
        facZWaveUSB3Sh().LogTraceMsg
        (
            L"Id'd unit %(1), make/model=%(2)/%(3). Waiting for use approval"
            , m_strName
            , m_deviUnit.strMake()
            , m_deviUnit.strModel()
        );
    }

    // Make our unit object now, indicating server side mode
    MakeUnitObject(kCIDLib::False);

    // We've made changes so bump our serial number
    IncSerialNum();

    //
    //  If this is happening on the server side, send auto config now. This way, if
    //  we are doing this because we got a node info frame from a non-listener, we
    //  can get these messages out now. It can also get called on the client side.
    //
    if (m_pmzwciOwner)
    {
        TString strErr;
        if (!bDoAutoCfg(bKnownAwake, strErr))
        {
            if (facZWaveUSB3Sh().bMediumTrace())
            {
                facZWaveUSB3Sh().LogTraceMsg
                (
                    L"Unit %(1) pre-queued auto-config had errors"
                    , m_strName
                );
            }
        }
    }
}
