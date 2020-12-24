//
// FILE NAME: ZWaveLevi2Sh_UnitInfo.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/10/2014
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
#include    "ZWaveLevi2Sh_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TZWaveUnit,TCQCServerBase)


// ---------------------------------------------------------------------------
//  Local types and constants
// ---------------------------------------------------------------------------
namespace ZWaveLevi2Sh_UnitInfo
{
    // -----------------------------------------------------------------------
    //  Base class persistent format version
    //
    //  Version 2  -
    //      We got rid of the m_eStatus member, and replaced it with some separate
    //      flags. These are really runtime only, but we still stream them because
    //      they have to get to the client.
    // -----------------------------------------------------------------------
    const tCIDLib::TCard2       c2FmtVersion    = 2;
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
                        , const TZWaveUnit&     unit2)
{
    if (unit1.m_c4Id < unit2.m_c4Id)
        return tCIDLib::ESortComps::FirstLess;
    else if (unit1.m_c4Id > unit2.m_c4Id)
        return tCIDLib::ESortComps::FirstLess;
    return tCIDLib::ESortComps::Equal;
}

tCIDLib::ESortComps
TZWaveUnit::eCompByName(const   TZWaveUnit&     unit1
                        , const TZWaveUnit&     unit2)
{
    return unit1.m_strName.eCompare(unit2.m_strName);
}


// ---------------------------------------------------------------------------
//  TZWaveUnit: Constructors and Destructor
// ---------------------------------------------------------------------------
TZWaveUnit::TZWaveUnit() :

    m_bDisabled(kCIDLib::False)
    , m_bFailed(kCIDLib::False)
    , m_bMissing(kCIDLib::False)
    , m_c4Id(0)
    , m_colCmdClasses(tCIDLib::EAdoptOpts::Adopt)
    , m_colCmdQ()
    , m_colOptVals(109, TStringKeyOps(), &TZWOptionVal::strKey)
    , m_eGenType(tZWaveLevi2Sh::EGenTypes::None)
    , m_enctLastValue(0)
{
}

TZWaveUnit::TZWaveUnit( const   tCIDLib::TCard4             c4UnitId
                        , const tZWaveLevi2Sh::EGenTypes    eGenType) :

    m_bDisabled(kCIDLib::False)
    , m_bFailed(kCIDLib::False)
    , m_bMissing(kCIDLib::False)
    , m_c4Id(c4UnitId)
    , m_colCmdClasses(tCIDLib::EAdoptOpts::Adopt)
    , m_colCmdQ()
    , m_colOptVals(109, TStringKeyOps(), &TZWOptionVal::strKey)
    , m_eGenType(eGenType)
    , m_enctLastValue(0)
{
    // Initialize the name to something reasonable to start
    SetDefName();
}

//
//  WE ONLY copy persistent stuff required to generate the other information. We
//  don't copy any queued commands, since that's also purely runtime stuff and there
//  would never be any set prior the object being put into place and operating.
//
TZWaveUnit::TZWaveUnit(const TZWaveUnit& unitSrc) :

    m_bDisabled(unitSrc.m_bDisabled)
    , m_bFailed(unitSrc.m_bFailed)
    , m_bMissing(unitSrc.m_bMissing)
    , m_c4Id(unitSrc.m_c4Id)
    , m_colCmdClasses(tCIDLib::EAdoptOpts::Adopt)
    , m_colCmdQ()
    , m_colOptVals(unitSrc.m_colOptVals)
    , m_eGenType(unitSrc.m_eGenType)
    , m_enctLastValue(0)
    , m_strDevInfoFile(unitSrc.m_strDevInfoFile)
    , m_strName(unitSrc.m_strName)
    , m_zwdiThis(unitSrc.m_zwdiThis)
{
    //  If the source's device info was loaded, then create our class objects
    if (unitSrc.m_zwdiThis.m_bLoaded)
        CreateClassObjs();
}

TZWaveUnit::~TZWaveUnit()
{
}


// ---------------------------------------------------------------------------
//  TZWaveUnit: Public operators
// ---------------------------------------------------------------------------

//
//  Handle assignment. We will remove all of our command class handlers. If the
//  source has been set up, then we will recreate ours using the new info.
//
TZWaveUnit& TZWaveUnit::operator=(const TZWaveUnit& unitSrc)
{
    if (&unitSrc != this)
    {
        m_bDisabled     = unitSrc.m_bDisabled;
        m_bFailed       = unitSrc.m_bFailed;
        m_bMissing      = unitSrc.m_bMissing;
        m_c4Id          = unitSrc.m_c4Id;
        m_colOptVals    = unitSrc.m_colOptVals;
        m_eGenType      = unitSrc.m_eGenType;
        m_enctLastValue = 0;
        m_strDevInfoFile= unitSrc.m_strDevInfoFile;
        m_strName       = unitSrc.m_strName;
        m_zwdiThis      = unitSrc.m_zwdiThis;

        // Clean out any existing queued commands and don't copy any over
        m_colCmdQ.RemoveAll();

        //
        //  Clean our command class objects and then, if the source had device info set
        //  then create our new ones.
        //
        m_colCmdClasses.RemoveAll();
        if (!m_strDevInfoFile.bIsEmpty())
        {
            CIDAssert(!m_strDevInfoFile.bIsEmpty(), L"Ready unit hsould have dev info file");
            CreateClassObjs();
        }
    }
    return *this;
}


tCIDLib::TBoolean TZWaveUnit::operator==(const TZWaveUnit& unitSrc) const
{
    //
    //  We only need to compare the persisted stuff. This is only used during
    //  configuration to know if changes have been made. Compare the stuff mostly
    //  likely to be different in different units first.
    //
    //  For all of the stuff that comes from the device info file, just comparing
    //  the device info file names is enough. If those are the same, then all of
    //  the info loaded in that way must be the same. That includes the command
    //  class objects.
    //
    if ((m_c4Id != unitSrc.m_c4Id)
    ||  (m_strName != unitSrc.m_strName)
    ||  (m_eGenType != unitSrc.m_eGenType)
    ||  (m_strDevInfoFile != unitSrc.m_strDevInfoFile)
    ||  (m_bDisabled != unitSrc.m_bDisabled))
    {
        return kCIDLib::False;
    }

    //
    //  Check the command classes, just in case. If we have the same device info
    //  file, we have to have the same device classes and count of options.
    //
    CIDAssert
    (
        m_colCmdClasses.c4ElemCount() == unitSrc.m_colCmdClasses.c4ElemCount()
        && m_colOptVals.c4ElemCount() == unitSrc.m_colOptVals.c4ElemCount()
        , L"Same dev info should create same cmd classes and options"
    );

    //
    //  Compare the selected option values. We know we have the same count, so if we
    //  use our list and look for each one in the target, we know we processed them
    //  all.
    //
    tZWaveLevi2Sh::TOptValList::TCursor cursOpts(&m_colOptVals);
    if (cursOpts.bReset())
    {
        do
        {
            const TZWOptionVal* pzwoptvCur = unitSrc.m_colOptVals.pobjFindByKey
            (
                cursOpts.objRCur().m_strKey
            );

            // If this key isn' in source opts, or the values are different, not equal
            if (!pzwoptvCur || (pzwoptvCur->m_strValue != cursOpts.objRCur().m_strValue))
                return kCIDLib::False;

        }   while (cursOpts.bNext());
    }
    return kCIDLib::True;
}


tCIDLib::TBoolean TZWaveUnit::operator!=(const TZWaveUnit& unitSrc) const
{
    return !operator==(unitSrc);
}


// ---------------------------------------------------------------------------
//  TZWaveUnit: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  We are periodically given a chance by the driver to poll for for data actively
//  where needed. Each command class tracks the last time they got data. So we check
//  how long it's been and if it's been more than a certain time we give them a chance
//  to actively poll.
//
//  We return true if we had to do any polling. This way, the driver can skip over any
//  that didn't actually have to do anything and not waste a poll period for nothing.
//
tCIDLib::TBoolean TZWaveUnit::bActivePoll(MZWLeviSrvFuncs& mzwsfLevi)
{
    tCIDLib::TBoolean bRes = kCIDLib::False;

    const tCIDLib::TCard4 c4Cnt = m_colCmdClasses.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Cnt; c4Index++)
    {
        TZWCmdClass* pzwccCur = m_colCmdClasses[c4Index];

        // Ask this one if he wants to poll. If so, then give him a chance to.
        if (pzwccCur->bCheckPollReady())
        {
            try
            {
                if (pzwccCur->bQueryFldVals(*this, mzwsfLevi))
                    bRes = kCIDLib::True;

            }

            catch(TError& errToCatch)
            {
                if (facZWaveLevi2Sh().bShouldLog(errToCatch))
                {
                    errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                    TModule::LogEventObj(errToCatch);
                }

                // We failed, but obivously did try to poll
                bRes = kCIDLib::True;
            }
        }
    }

    return bRes;
}


tCIDLib::TBoolean
TZWaveUnit::bClassIsSecure(const tZWaveLevi2Sh::ECClasses eToTest) const
{
    tCIDLib::TBoolean bRet = kCIDLib::False;

    const tCIDLib::TCard4 c4Cnt = m_zwdiThis.m_colClasses.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Cnt; c4Index++)
    {
        if ((m_zwdiThis.m_colClasses[c4Index].m_eClass == eToTest)
        &&  m_zwdiThis.m_colClasses[c4Index].m_bSecure)
        {
            bRet = kCIDLib::True;
            break;
        }
    }
    return bRet;
}


tCIDLib::TBoolean TZWaveUnit::bDevInfoLoaded() const
{
    return m_zwdiThis.m_bLoaded;
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


// Get/set the failed flag
tCIDLib::TBoolean TZWaveUnit::bFailed() const
{
    return m_bFailed;
}

tCIDLib::TBoolean TZWaveUnit::bFailed(const tCIDLib::TBoolean bToSet)
{
    m_bFailed = bToSet;
    return m_bFailed;
}


// Get/set the missing flag
tCIDLib::TBoolean TZWaveUnit::bMissing() const
{
    return m_bMissing;
}

tCIDLib::TBoolean TZWaveUnit::bMissing(const tCIDLib::TBoolean bToSet)
{
    m_bMissing = bToSet;
    return m_bMissing;
}


tCIDLib::TBoolean
TZWaveUnit::bImplementsClass(const tZWaveLevi2Sh::ECClasses eToTest) const
{
    //
    //  Look through all of our configured command classes and see if at least one
    //  implements this class.
    //
    const tCIDLib::TCard4 c4Cnt = m_zwdiThis.m_colClasses.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Cnt; c4Index++)
    {
        if (m_zwdiThis.m_colClasses[c4Index].m_eClass == eToTest)
            return kCIDLib::True;
    }
    return kCIDLib::False;
}


//
//  If this unit is diabled, or has failed, or was missing on the last scan, say it
//  is not viable.
//
tCIDLib::TBoolean TZWaveUnit::bIsViable() const
{
    return !m_bDisabled && !m_bFailed && !m_bMissing;
}


//
//  Map the incoming command class to our command class enum. Then find the command
//  class handler for the msg and pass it on to him.
//
tCIDLib::TBoolean
TZWaveUnit::bHandleGenReport(const  tCIDLib::TCard4     c4InstId
                            , const tCIDLib::TCardList& fcolVals
                            ,       MZWLeviSrvFuncs&    mzwsfLevi)
{
    // Can't be less than 2 bytes
    const tCIDLib::TCard4 c4ValCnt = fcolVals.c4ElemCount();
    if (c4ValCnt < 2)
        return kCIDLib::False;

    //
    //  We also handle Wakeup specially and, for any classes currently marked as not
    //  having yet stored their initial info and it's marked readable, then give it
    //  a chance chance to do an active query.
    //
    if ((fcolVals.c4ElemCount() >= 2)
    &&  (fcolVals[0] == COMMAND_CLASS_WAKE_UP)
    &&  (fcolVals[1] == WAKE_UP_NOTIFICATION))
    {
        if (mzwsfLevi.bLeviTraceEnabled())
        {
            TTextOutStream& strmTrace = mzwsfLevi.strmLeviTrace();
            strmTrace   << L"\n\nGot Wakeup for node " << c4Id()
                        << L", InstId=" << c4InstId << kCIDLib::EndLn;
        }

        TString strTmp1, strTmp2;
        const tCIDLib::TCard4 c4Count = m_colCmdClasses.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            TZWCmdClass* pzwccCur = m_colCmdClasses[c4Index];
            if (pzwccCur->bWaitInitVals() && pzwccCur->bCanRead())
            {
                try
                {
                    pzwccCur->bQueryFldVals(*this, mzwsfLevi);

                    // It worked so clear our waiting flag
                    pzwccCur->bWaitInitVals(kCIDLib::False);
                }

                catch(TError& errToCatch)
                {
                    if (mzwsfLevi.bLeviTraceEnabled())
                    {
                        TTextOutStream& strmTrace = mzwsfLevi.strmLeviTrace();
                        strmTrace   << L"    Wakeup processsing failed. Error="
                                    << errToCatch.strErrText()
                                    << kCIDLib::EndLn;
                    }

                    if (mzwsfLevi.eLeviVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
                    {
                        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                        TModule::LogEventObj(errToCatch);

                        facZWaveLevi2Sh().LogMsg
                        (
                            CID_FILE
                            , CID_LINE
                            , L"Failed to send msg (in wakeup) for unit %(1) (%(2)). Class=%(3)"
                            , tCIDLib::ESeverities::Status
                            , tCIDLib::EErrClasses::AppStatus
                            , strName()
                            , TCardinal(c4Id())
                            , tZWaveLevi2Sh::strXlatECClasses(pzwccCur->eCmdClass())
                        );
                    }
                }
            }
        }

        //
        //  If this guy has any queued up commands, send those. In this case, since
        //  we have all of the data sitting here and accessible, we can use multi-
        //  command msgs to send out more than one msg at a time if the target supports
        //  it, but currently we don't include that information in our device info.
        //
        //  We only send outgoing commands here, not queries, so there's no need to
        //  get responses.
        //
        if (!m_colCmdQ.bIsEmpty())
        {
            const tCIDLib::TCard4 c4Count = m_colCmdQ.c4ElemCount();

            if (mzwsfLevi.bLeviTraceEnabled())
            {
                TTextOutStream& strmTrace = mzwsfLevi.strmLeviTrace();
                strmTrace   << L"\nSending " << c4Count << L" queued msgs for unit "
                            << strName() << kCIDLib::EndLn;
            }

            facZWaveLevi2Sh().LogMsg
            (
                CID_FILE
                , CID_LINE
                , L"Sending queued msgs for unit %(1), Id=%(2)"
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
                , strName()
                , TCardinal(c4Id())
            );

            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            {
                const TZWQCmd zwqcCur = m_colCmdQ[c4Index];

                try
                {
                    //
                    //  Find the command class. We might not find it, since some of these
                    //  msgs are for things that we don't need a handler for.
                    //
                    TZWCmdClass* pzwccCur = pzwccFind(zwqcCur.m_eCmdClass, zwqcCur.m_c4InstId);
                    if (pzwccCur)
                        mzwsfLevi.LeviSendUnitMsg(*this, *pzwccCur, zwqcCur.m_fcolBytes);
                    else
                        mzwsfLevi.LeviSendMsg(*this, zwqcCur.m_fcolBytes);
                }

                catch(TError& errToCatch)
                {
                    if (mzwsfLevi.bLeviTraceEnabled())
                    {
                        TTextOutStream& strmTrace = mzwsfLevi.strmLeviTrace();
                        strmTrace   << L"\nFailed to send queued command. Unit="
                                    << strName()
                                    << L"  InstId=" << zwqcCur.m_c4InstId
                                    << L"  Error=" << errToCatch.strErrText()
                                    << kCIDLib::EndLn;
                    }

                    if (mzwsfLevi.eLeviVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
                    {
                        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                        TModule::LogEventObj(errToCatch);

                        facZWaveLevi2Sh().LogMsg
                        (
                            CID_FILE
                            , CID_LINE
                            , L"Failed to send queued msg for unit %(1) (%(2)). Class=%(3)"
                            , tCIDLib::ESeverities::Status
                            , tCIDLib::EErrClasses::AppStatus
                            , strName()
                            , TCardinal(c4Id())
                            , tZWaveLevi2Sh::strXlatECClasses(zwqcCur.m_eCmdClass)
                        );
                    }
                }
            }

            // Flush the list now
            m_colCmdQ.RemoveAll();
        }

        // Send the unit a wakeup complete msg so he can go back to sleep
        try
        {
            if (mzwsfLevi.bLeviTraceEnabled())
            {
                TTextOutStream& strmTrace = mzwsfLevi.strmLeviTrace();
                strmTrace   << L"\nSending Wakeup complete msg" << kCIDLib::NewEndLn;
            }
            tCIDLib::TCardList fcolDone(8);
            fcolDone.RemoveAll();
            fcolDone.c4AddElement(COMMAND_CLASS_WAKE_UP);
            fcolDone.c4AddElement(WAKE_UP_NO_MORE_INFORMATION);
            mzwsfLevi.LeviSendMsg(*this, fcolDone);
        }

        catch(TError& errToCatch)
        {
            if (mzwsfLevi.eLeviVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);

                facZWaveLevi2Sh().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , L"Failed to send wakeup completion msg for unit %(1) (%(2))"
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::AppStatus
                    , strName()
                    , TCardinal(c4Id())
                );
            }
        }

        return kCIDLib::True;
    }

    //
    //  Let the command classes process this guy. Normally we only pass these to
    //  classes of the same instance id, though we let all classes see them of that
    //  same id since sometimes things get reported via multiple classes but the
    //  info is stored by one.
    //
    //  But, if the class is marked as 'all zero instance', we will also pass all zero
    //  instance messages to all of the handles for the incoming class. This gets
    //  around some limitations in some devices. It means that class objects have to
    //  check themselves if they only want to see specific instance ids and ignore
    //  those not for them.
    //
    tCIDLib::TBoolean bHandled = kCIDLib::False;
    const tCIDLib::TCard4 c4Count = m_colCmdClasses.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TZWCmdClass* pzwccCur = m_colCmdClasses[c4Index];

        //
        //  If it's a basic msg, then this class must be the same as the base class
        //  of the unit. Only that class can legally be handled via basic reports.
        //
        if ((fcolVals[0] == COMMAND_CLASS_BASIC)
        &&  (eBaseClass() != pzwccCur->eCmdClass()))
        {
            continue;
        }

        if ((pzwccCur->c4InstId() == c4InstId)
        ||  (pzwccCur->bAllZeroInst() && !c4InstId && (pzwccCur->c4ZWClassId() == fcolVals[0])))
        {
            // If this guy handles this report, update his time stamp
            if (pzwccCur->bHandleGenReport(fcolVals, *this, mzwsfLevi, c4InstId))
            {
                bHandled = kCIDLib::True;
                pzwccCur->ResetTimeStamp();
            }
        }
    }

    return bHandled;
}


//
//  Once the device info file key is set on the unit, this can be called to load
//  up the device info and prep the unit for actual use. On the server side it means
//  it's ready to set up fields and start using. On the client side it means that the
//  option info can be queried and info about command classes implemented can be
//  known and whatnot.
//
tCIDLib::TBoolean TZWaveUnit::bLoadDevInfo(TXMLTreeParser& xtprsToUse)
{
    // If the device info file key hasn't been set, then we can't do it
    if (m_strDevInfoFile.bIsEmpty())
        return kCIDLib::False;

    const TZWDevInfo* pzwdiLoad = TZWaveDrvConfig::pzwdiFind(m_strDevInfoFile, &xtprsToUse);

    // If not loadable, then we can't do this
    if (!pzwdiLoad)
    {
        facZWaveLevi2Sh().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"Failed to load device info for unit %(1) (Key=%(2))"
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
            , strName()
            , m_strDevInfoFile
        );
        return kCIDLib::False;
    }

    // It worked so store the info
    m_zwdiThis = *pzwdiLoad;

    //
    //  Call the private method that creates the command class handlers. If this is
    //  a load of new device info (the dev info file was changed), then this will also
    //  create new initial default option values.
    //
    CreateClassObjs();
    return kCIDLib::True;
}


//
//  Find the indicated option value and return whether it is one whose value was fixed
//  by the device info file or if it can be user modified.
//
tCIDLib::TBoolean TZWaveUnit::bOptValIsFixed(const TString& strKey) const
{
    return m_colOptVals.objFindByKey(strKey).m_bFixed;
}


//
//  We just let all of our class command handlers speak up as the owner of the passed
//  field.
//
tCIDLib::TBoolean
TZWaveUnit::bOwnsFld(const tCIDLib::TCard4 c4FldId, tCIDLib::TCard4& c4CmdClassInd) const
{
    const tCIDLib::TCard4 c4Count = m_colCmdClasses.c4ElemCount();
    c4CmdClassInd = 0;
    for (; c4CmdClassInd < c4Count; c4CmdClassInd++)
    {
        if (m_colCmdClasses[c4CmdClassInd]->bOwnsFld(c4FldId))
            return kCIDLib::True;
    }
    return kCIDLib::False;
}



//
//  Queries the options info for this unit. This is the stuff required to display to
//  the user and let him make selections. The selected values are separate.
//
tCIDLib::TBoolean TZWaveUnit::bQueryOpts(tZWaveLevi2Sh::TOptList& colToFill) const
{
    colToFill.RemoveAll();

    //
    //  Go through all of our command classes and ask them to add their options to
    //  the passed list.
    //
    const tCIDLib::TCard4 c4Count = m_colCmdClasses.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        m_colCmdClasses[c4Index]->LoadOptList(colToFill);

    return !colToFill.bIsEmpty();
}


//
//  Return how many instances of a given command class we have. This controls whether
//  multi-channel/instance commands should be used for that command class or not.
//
tCIDLib::TCard4
TZWaveUnit::c4ClassInstCount(const tZWaveLevi2Sh::ECClasses eToTest) const
{
    tCIDLib::TCard4 c4Ret = 0;

    const tCIDLib::TCard4 c4Cnt = m_zwdiThis.m_colClasses.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Cnt; c4Index++)
    {
        if (m_zwdiThis.m_colClasses[c4Index].m_eClass == eToTest)
            c4Ret++;
    }
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


//
//  Given the key for an option, look it up in our option values list and return
//  a ref to it's allowed values list.
//
const tCIDLib::TStrList& TZWaveUnit::colAllowedOptVals(const TString& strKey) const
{
    return m_colOptVals.objFindByKey(strKey).m_colAllowedVals;
}


//
//  Returns our configured base class (the one that will get basic reports.)
//
tZWaveLevi2Sh::ECClasses TZWaveUnit::eBaseClass() const
{
    return m_zwdiThis.m_eBaseClass;
}


//
//  Provide access to our generic type. This is set during the network scan when the
//  unit is first discovered.
//
tZWaveLevi2Sh::EGenTypes TZWaveUnit::eGenType() const
{
    return m_eGenType;
}


//
//  Provide access to our specific type. This comes from the device info file, so this
//  should only be called after the device info has been loaded.
//
tZWaveLevi2Sh::ESpecTypes TZWaveUnit::eSpecType() const
{
    return m_zwdiThis.m_eSpecType;
}


//
//  The driver should have checked to see if we own the field first. That will get him
//  a command class index, which he will pass to us here so we don't have to do the
//  check again.
//
tCQCKit::ECommResults
TZWaveUnit::eWriteBoolFld(  const   tCIDLib::TCard4     c4FldId
                            , const tCIDLib::TBoolean   bValue
                            ,       MZWLeviSrvFuncs&    mzwsfLevi
                            , const tCIDLib::TCard4     c4CmdIndex)
{
    return m_colCmdClasses[c4CmdIndex]->eWriteBoolFld(c4FldId, bValue, *this, mzwsfLevi);
}

tCQCKit::ECommResults
TZWaveUnit::eWriteCardFld(  const   tCIDLib::TCard4     c4FldId
                            , const tCIDLib::TCard4     c4Value
                            ,       MZWLeviSrvFuncs&    mzwsfLevi
                            , const tCIDLib::TCard4     c4CmdIndex)
{
    return m_colCmdClasses[c4CmdIndex]->eWriteCardFld(c4FldId, c4Value, *this, mzwsfLevi);
}

tCQCKit::ECommResults
TZWaveUnit::eWriteIntFld(const  tCIDLib::TCard4     c4FldId
                        , const tCIDLib::TInt4      i4Value
                        ,       MZWLeviSrvFuncs&    mzwsfLevi
                        , const tCIDLib::TCard4     c4CmdIndex)
{
    return m_colCmdClasses[c4CmdIndex]->eWriteIntFld(c4FldId, i4Value, *this, mzwsfLevi);
}

tCQCKit::ECommResults
TZWaveUnit::eWriteFloatFld( const   tCIDLib::TCard4     c4FldId
                            , const tCIDLib::TFloat8    f8Value
                            ,       MZWLeviSrvFuncs&    mzwsfLevi
                            , const tCIDLib::TCard4     c4CmdIndex)
{
    return m_colCmdClasses[c4CmdIndex]->eWriteFloatFld(c4FldId, f8Value, *this, mzwsfLevi);
}

tCQCKit::ECommResults
TZWaveUnit::eWriteStringFld(const   tCIDLib::TCard4     c4FldId
                            , const TString&            strValue
                            ,       MZWLeviSrvFuncs&    mzwsfLevi
                            , const tCIDLib::TCard4     c4CmdIndex)
{
    return m_colCmdClasses[c4CmdIndex]->eWriteStringFld(c4FldId, strValue, *this, mzwsfLevi);
}



// Get the last time that we got a value for this unit
tCIDLib::TEncodedTime TZWaveUnit::enctLastValue() const
{
    return m_enctLastValue;
}


//
//  Give all of our command class objects a change to initialize their fields to
//  error state, where they will stay until good data is stored to them, and to get
//  get any other data to the correct state to start storing data.
//
tCIDLib::TVoid TZWaveUnit::InitVals(MZWLeviSrvFuncs& mzwsfLevi)
{
    const tCIDLib::TCard4 c4Count = m_colCmdClasses.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        m_colCmdClasses[c4Index]->InitVals(*this, mzwsfLevi);
}


//
//  Look for a command class of this type and instance id.
//
TZWCmdClass*
TZWaveUnit::pzwccFind(  const   tZWaveLevi2Sh::ECClasses    eClass
                        , const tCIDLib::TCard4             c4InstId)
{
    const tCIDLib::TCard4 c4Count = m_colCmdClasses.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TZWCmdClass* pzwccCur = m_colCmdClasses[c4Index];
        if ((pzwccCur->eCmdClass() == eClass)
        &&  (pzwccCur->c4InstId() == c4InstId))
        {
            return pzwccCur;
        }
    }
    return 0;
}


//
//  Pass this on to any of our plugged in command classes, to let them add fields
//  as required.
//
tCIDLib::TVoid
TZWaveUnit::QueryFldDefs(tCQCKit::TFldDefList& colAddTo, TCQCFldDef& flddTmp) const
{
    TString strTmp1, strTmp2;

    const tCIDLib::TCard4 c4Count = m_colCmdClasses.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        m_colCmdClasses[c4Index]->QueryFldDefs(colAddTo, flddTmp, *this, strTmp1, strTmp2);
}


tCIDLib::TBoolean TZWaveUnit::bQueryFldVals(MZWLeviSrvFuncs& mzwsfLevi)
{
    tCIDLib::TCard4 c4Fails = 0;
    const tCIDLib::TCard4 c4Count = m_colCmdClasses.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        try
        {
            TZWCmdClass* pzwccCur = m_colCmdClasses[c4Index];
            if (!pzwccCur->bCanRead())
            {
                //
                //  Just mark it since it's never going to actually do a live query.
                //  It will have to depend on async reports or never get any readable
                //  value at all.
                //
                pzwccCur->bWaitInitVals(kCIDLib::False);
            }
             else
            {
                if (pzwccCur->bQueryFldVals(*this, mzwsfLevi))
                    pzwccCur->bWaitInitVals(kCIDLib::False);
            }
        }

        catch(TError& errToCatch)
        {
            if (mzwsfLevi.eLeviVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);

                facZWaveLevi2Sh().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , L"Unit %(1) failed to get init vals for %(2) [InstId=%(3)]"
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::AppStatus
                    , strName()
                    , tZWaveLevi2Sh::strXlatECClasses(m_colCmdClasses[c4Index]->eCmdClass())
                    , TCardinal(m_colCmdClasses[c4Index]->c4InstId())
                );
            }
            c4Fails++;
        }
    }

    if (c4Fails)
    {
        if (mzwsfLevi.eLeviVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
        {
            facZWaveLevi2Sh().LogMsg
            (
                CID_FILE
                , CID_LINE
                , L"Unit %(1) failed to initialize one or more command classes"
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
                , strName()
            );
        }
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


//
//  If this is a wakeup type device, then the driver will not immediately try to send
//  outgoing commands (driven by incoming client calls) but will instead add them to
//  our queue. Later, when we get a wakeup, we'll send any of them out during the
//  wakeup processing.
//
tCIDLib::TVoid
TZWaveUnit::QueueUpCmd(const TZWQCmd& qcmdToAdd, MZWLeviSrvFuncs& mzwsfLevi)
{
    if (m_colCmdQ.bIsFull(16))
    {
        facZWaveLevi2Sh().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kZW2ShErrs::errcUnit_AutoCfgQFull
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::OutResource
            , strName()
        );
    }

    if (mzwsfLevi.bLeviTraceEnabled())
    {
        TTextOutStream& strmTrace = mzwsfLevi.strmLeviTrace();
        strmTrace   << L"\nQueued up msg for wakeup. Unit=" << strName()
                    << L"  InstId=" << qcmdToAdd.m_c4InstId;

        if (qcmdToAdd.m_fcolBytes.c4ElemCount() > 1)
        {
            strmTrace   << L"  CmdClass=" << qcmdToAdd.m_fcolBytes[0] << L"/0x"
                        << TCardinal(qcmdToAdd.m_fcolBytes[0])
                        << L"  Cmd=" << qcmdToAdd.m_fcolBytes[1] << L"/0x"
                        << TCardinal(qcmdToAdd.m_fcolBytes[1])
                        << kCIDLib::EndLn;
        }
    }

    m_colCmdQ.objAdd(qcmdToAdd);
}


//
//  Set the time we last got a value for this unit to now.
//
tCIDLib::TVoid TZWaveUnit::ResetLastValTime()
{
    m_enctLastValue = TTime::enctNow();
}


//
//  If the driver, during the network scan, sees that the type reported for this
//  unit is different from the previous configuration, it will call this. We will
//  reset this unit as though it were a new one, setting this as the new type.
//
tCIDLib::TVoid TZWaveUnit::ResetUnitType(const tZWaveLevi2Sh::EGenTypes eNew)
{
    m_eGenType = eNew;

    m_bFailed = kCIDLib::False;
    m_colCmdClasses.RemoveAll();
    m_colOptVals.RemoveAll();
    m_enctLastValue = 0;
    m_strDevInfoFile.Clear();
    m_zwdiThis.Reset();
    SetDefName();
}



//
//  Expose the device info file which the config object needs in order to load
//  up our device info.
//
const TString& TZWaveUnit::strDevInfoFile() const
{
    return m_strDevInfoFile;
}


//
//  Convenient access to the user selected values for our command class options, by key.
//  THe 'value' here means the option value key. The actual display value is in the
//  options info.
//
const TString& TZWaveUnit::strFindOptVal(const TString& strKey) const
{
    return m_colOptVals.objFindByKey(strKey).m_strValue;
}


// Expose the make/model info from the device info
const TString& TZWaveUnit::strMake() const
{
    return m_zwdiThis.m_strMake;
}

const TString& TZWaveUnit::strModel() const
{
    return m_zwdiThis.m_strModel;
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


//
//  Set the device info key for this unit. If it's different from the current one,
//  then reset any info that is driven by device info. It'll have to be set up again
//  later by a call to LoadDevInfo(). If it's the same as the current one, then nothing
//  has changed.
//
tCIDLib::TVoid TZWaveUnit::SetDevInfoFile(const TString& strFileName)
{
    if (m_strDevInfoFile != strFileName)
    {
        m_colOptVals.RemoveAll();
        m_colCmdClasses.RemoveAll();
        m_zwdiThis.Reset();

        m_strDevInfoFile = strFileName;
    }
}


//
//  Update the selection value for an option. We get the key for the option itself,
//  which let's us look up the right key/value pair in the value list. And we get the
//  key of the option value (not the display text) which we put into the value of the
//  k/v pair.
//
tCIDLib::TVoid TZWaveUnit::SetOptVal(const TString& strOptKey, const TString& strValKey)
{
    m_colOptVals.objFindByKey(strOptKey).m_strValue = strValKey;
}


//
//  Give all of our plugged in command classes a chance to store away their field
//  ids.
//
tCIDLib::TVoid TZWaveUnit::StoreFldIds(const MZWLeviSrvFuncs& mzwsfLevi)
{
    TString strTmp1;
    TString strTmp2;

    const tCIDLib::TCard4 c4Count = m_colCmdClasses.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        strTmp1.Clear();
        strTmp2.Clear();
        m_colCmdClasses[c4Index]->StoreFldIds(*this, mzwsfLevi, strTmp1, strTmp2);
    }
}


//
//  Provide access to our device info. The caller is responsible for making sure it has
//  been set (by checking our unit status.)
//
const TZWDevInfo& TZWaveUnit::zwdiThis() const
{
    return m_zwdiThis;
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
    if (!c2FmtVersion || (c2FmtVersion > ZWaveLevi2Sh_UnitInfo::c2FmtVersion))
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
                    >> m_eGenType
                    >> m_bDisabled
                    >> m_strName;

    if (c2FmtVersion < 2)
    {
        // Eat the old status enum
        tCIDLib::TInt4 i4Dummy;
        strmToReadFrom >> i4Dummy;
    }
     else
    {
        // Stream in the new separate flags
        strmToReadFrom  >> m_bFailed
                        >> m_bMissing;
    }

    strmToReadFrom  >> m_strDevInfoFile
                    >> m_colOptVals;

    // Make sure we get the end marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);

    // Reset the runtime values.
    m_colCmdClasses.RemoveAll();
    m_enctLastValue = 0;
    m_zwdiThis.Reset();
}


tCIDLib::TVoid TZWaveUnit::StreamTo(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << ZWaveLevi2Sh_UnitInfo::c2FmtVersion
                    << m_c4Id
                    << m_eGenType
                    << m_bDisabled
                    << m_strName
                    << m_bFailed
                    << m_bMissing
                    << m_strDevInfoFile
                    << m_colOptVals
                    << tCIDLib::EStreamMarkers::EndObject;
}


// ---------------------------------------------------------------------------
//  TZWaveUnit: Private, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TZWaveUnit::bCheckFldId(const   tCIDLib::TCard4     c4ToCheck
                        , const tCIDLib::TCh* const pszFldName) const
{
    if (c4ToCheck == kCIDLib::c4MaxCard)
    {
        facZWaveLevi2Sh().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kZW2ShErrs::errcUnit_FldIdNotSet
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotReady
            , strName()
            , TString(pszFldName)
        );
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


tCIDLib::TBoolean
TZWaveUnit::bSetAssoc(          MZWLeviSrvFuncs&    mzwsfLevi
                        , const tCIDLib::TCard4     c4TarId
                        , const tCIDLib::TCard4     c4GroupId)
{
    tCIDLib::TCardList fcolVals(8);

    fcolVals.c4AddElement(COMMAND_CLASS_ASSOCIATION);
    fcolVals.c4AddElement(ASSOCIATION_SET);
    fcolVals.c4AddElement(c4GroupId);
    fcolVals.c4AddElement(c4TarId);

    try
    {
        mzwsfLevi.LeviSendMsg(*this, fcolVals);
    }

    catch(TError& errToCatch)
    {
        if (mzwsfLevi.eLeviVerboseLevel() >= tCQCKit::EVerboseLvls::Low)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);

            facZWaveLevi2Sh().LogMsg
            (
                CID_FILE
                , CID_LINE
                , L"Failed to associate unit %(1) to group %(2) of unit %(3)"
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
                , TCardinal(c4TarId)
                , TCardinal(c4GroupId)
                , m_strName
            );
        }
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


tCIDLib::TBoolean
TZWaveUnit::bSetWakeupInt(          MZWLeviSrvFuncs&    mzwsfLevi
                            , const tCIDLib::TCard4     c4VRCOPId
                            , const tCIDLib::TCard4     c4Seconds)
{
    tCIDLib::TCardList fcolVals(8);

    fcolVals.c4AddElement(COMMAND_CLASS_WAKE_UP);
    fcolVals.c4AddElement(WAKE_UP_INTERVAL_SET);
    fcolVals.c4AddElement(tCIDLib::TCard1(c4Seconds >> 16));
    fcolVals.c4AddElement(tCIDLib::TCard1(c4Seconds >> 8));
    fcolVals.c4AddElement(tCIDLib::TCard1(c4Seconds & 0xFF));
    fcolVals.c4AddElement(c4VRCOPId);

    try
    {
        mzwsfLevi.LeviSendMsg(*this, fcolVals);
    }

    catch(TError& errToCatch)
    {
        if (mzwsfLevi.eLeviVerboseLevel() >= tCQCKit::EVerboseLvls::Low)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);

            facZWaveLevi2Sh().LogMsg
            (
                CID_FILE
                , CID_LINE
                , L"Failed to send wakeup interval set to unit %(1)"
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
                , m_strName
            );
        }
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


//
//  Assuming that device info has been set on us, we will use that info to create
//  our command class handlers. We will delete any existing ones first.
//
tCIDLib::TVoid TZWaveUnit::CreateClassObjs()
{
    TZWOptionVal zwoptvTmp;
    m_colCmdClasses.RemoveAll();
    const tCIDLib::TCard4 c4Cnt = m_zwdiThis.m_colClasses.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Cnt; c4Index++)
    {
        const TZWDevClass& zwdcCur = m_zwdiThis.m_colClasses[c4Index];

        TZWCmdClass* pzwccNew = 0;
        switch(zwdcCur.m_eClass)
        {
            case tZWaveLevi2Sh::ECClasses::Association :
                pzwccNew = new TZWCCAssociation(zwdcCur);
                break;

            case tZWaveLevi2Sh::ECClasses::Basic :
                //
                //  This one is never created. The basic reports are always passed
                //  to the command class for the generic type.
                //
                break;

            case tZWaveLevi2Sh::ECClasses::Battery :
                pzwccNew = new TZWCCBattery(zwdcCur);
                break;

            case tZWaveLevi2Sh::ECClasses::BinSensor :
                pzwccNew = new TZWCCBinSensor(zwdcCur);
                break;

            case tZWaveLevi2Sh::ECClasses::BinSwitch :
                pzwccNew = new TZWCCBinSwitch(zwdcCur);
                break;

            case tZWaveLevi2Sh::ECClasses::ColorSwitch :
                pzwccNew = new TZWCCColorSwitch(zwdcCur);
                break;

            case tZWaveLevi2Sh::ECClasses::Configuration :
                pzwccNew = new TZWCCConfig(zwdcCur);
                break;

            case tZWaveLevi2Sh::ECClasses::DoorLock :
                pzwccNew = new TZWCCEntryCtrl(zwdcCur);
                break;

            case tZWaveLevi2Sh::ECClasses::Hail :
                break;

            case tZWaveLevi2Sh::ECClasses::Meter :
                break;

            case tZWaveLevi2Sh::ECClasses::MLSensor :
                pzwccNew = new TZWCCMLSensor(zwdcCur);
                break;

            case tZWaveLevi2Sh::ECClasses::MLSwitch :
                pzwccNew = new TZWCCMLSwitch(zwdcCur);
                break;

            case tZWaveLevi2Sh::ECClasses::MultiCmd :
                break;

            case tZWaveLevi2Sh::ECClasses::PowerLevel :
                break;

            case tZWaveLevi2Sh::ECClasses::Naming :
                break;

            case tZWaveLevi2Sh::ECClasses::SceneActuatorConf :
                pzwccNew = new TZWCCSceneActConf(zwdcCur);
                break;

            case tZWaveLevi2Sh::ECClasses::SceneControllerConf :
                break;

            case tZWaveLevi2Sh::ECClasses::SceneActivation :
                pzwccNew = new TZWCCSceneActivation(zwdcCur);
                break;

            case tZWaveLevi2Sh::ECClasses::Security :
                break;

            case tZWaveLevi2Sh::ECClasses::SwitchAll :
                break;

            case tZWaveLevi2Sh::ECClasses::ThermoCurTemp :
                pzwccNew = new TZWCCThermoCTemp(zwdcCur);
                break;

            case tZWaveLevi2Sh::ECClasses::ThermoFanMode :
                pzwccNew = new TZWCCThermoFM(zwdcCur);
                break;

            case tZWaveLevi2Sh::ECClasses::ThermoFanOpMode :
                pzwccNew = new TZWCCThermoFOM(zwdcCur);
                break;

            case tZWaveLevi2Sh::ECClasses::ThermoMode :
                pzwccNew = new TZWCCThermoMode(zwdcCur);
                break;

            case tZWaveLevi2Sh::ECClasses::ThermoOpMode :
                pzwccNew = new TZWCCThermoOMode(zwdcCur);
                break;

            case tZWaveLevi2Sh::ECClasses::ThermoSetPnt :
                pzwccNew = new TZWCCThermoSP(zwdcCur);
                break;

            case tZWaveLevi2Sh::ECClasses::UserCode :
                break;

            case tZWaveLevi2Sh::ECClasses::Version :
                break;

            case tZWaveLevi2Sh::ECClasses::WakeUp :
                // This one is never actually created
                break;

            default :
                break;
        };

        if (pzwccNew)
        {
            //
            //  Give this one a chance to update option values. It will check for
            //  all of its option keys and add any that are not present. It might
            //  remove known obsolete ones as well.
            //
            TJanitor<TZWCmdClass> janClass(pzwccNew);
            pzwccNew->LoadOptVals(zwdcCur, m_colOptVals, zwoptvTmp);

            // OK< orphan it out to the command classes list
            m_colCmdClasses.Add(janClass.pobjOrphan());
        }
    }
}


// Creates a default name for newly discovered modules
tCIDLib::TVoid TZWaveUnit::SetDefName()
{
    // Start with the basic type name (the short version which is legal for field names)
    m_strName = L"Unit_";

    // Then a 2 digit hex representation of the unit id
    if (m_c4Id < 0x10)
        m_strName.Append(kCIDLib::chDigit0);
    m_strName.AppendFormatted(m_c4Id, tCIDLib::ERadices::Hex);
}


