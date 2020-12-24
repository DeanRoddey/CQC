//
// FILE NAME: GE-IRES_DriverImpl.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/29/2008
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
//  This file implements the bulk of the driver implementation. Some of the
//  functionality provided here is accessed via server side ORB objects that
//  we register, and which the generic IR client side driver uses to talk
//  to us.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "GC-IRES.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TGCIRESDriver,TBaseIRSrvDriver)


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace GCIRES_DriverImpl
{
    const tCIDLib::TEncodedTime enctMinInterval = kCIDLib::enctOneMilliSec * 250;
    const tCIDLib::TEncodedTime enctPollInterval = kCIDLib::enctOneMinute;
}




// ---------------------------------------------------------------------------
//   CLASS: TGCIRESDriver::TOOPair
//  PREFIX: oop
// ---------------------------------------------------------------------------
TGCIRESDriver::TOOPair::TOOPair() :

    m_c4Off(0)
    , m_c4On(0)
{
}

TGCIRESDriver::TOOPair::TOOPair(const   tCIDLib::TCard4 c4On
                                , const tCIDLib::TCard4 c4Off) :
    m_c4Off(c4Off)
    , m_c4On(c4On)
{
}

TGCIRESDriver::TOOPair::~TOOPair()
{
}

tCIDLib::TVoid
TGCIRESDriver::TOOPair::Set(const   tCIDLib::TCard4 c4On
                            , const tCIDLib::TCard4 c4Off)
{
    m_c4Off = c4Off;
    m_c4On = c4On;
}



// ---------------------------------------------------------------------------
//   CLASS: TGCIRESDriver
//  PREFIX: drv
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TGCIRESDriver: Constructors and Destructor
// ---------------------------------------------------------------------------
TGCIRESDriver::TGCIRESDriver(const TCQCDriverObjCfg& cqcdcToLoad) :

    TBaseIRSrvDriver(cqcdcToLoad)
    , m_bTrainingMode(kCIDLib::False)
    , m_c4FldIdFirmwareVer(kCIDLib::c4MaxCard)
    , m_c4FldIdTrainingMode(kCIDLib::c4MaxCard)
    , m_enctLastMsg(0)
    , m_enctNextEvent(0)
    , m_pcommGCIRE(0)
    , m_strCmdPrefix(L"GC-IRE,")
    , m_strPort()
{
}


TGCIRESDriver::~TGCIRESDriver()
{
    // Clean up the comm port if not already
    if (m_pcommGCIRE)
    {
        try
        {
            if (m_pcommGCIRE->bIsOpen())
                m_pcommGCIRE->Close();

            delete m_pcommGCIRE;
            m_pcommGCIRE= 0;
        }

        catch(...)
        {
        }
    }
}


// ---------------------------------------------------------------------------
//  TGCIRESDriver: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TGCIRESDriver::bCheckRecTrainingData(TString& strKeyToFill)
{
    // If we aren't online, then throw, else do it
    CheckOnline(CID_FILE, CID_LINE);

    //
    //  Store the current value, then clear the member. Return whether it
    //  had a value or not.
    //
    const tCIDLib::TBoolean bRet = !m_strTrainVal.bIsEmpty();

    if (bRet)
    {
        strKeyToFill = m_strTrainVal;
        m_strTrainVal.Clear();
    }
    return bRet;
}


tCIDLib::TBoolean
TGCIRESDriver::bGetCommResource(TThread& thrThis)
{
    // Open the port now, and set the configuration
    try
    {
        // Create the port object if needed, else close if open just in case
        if (!m_pcommGCIRE)
            m_pcommGCIRE = facCIDComm().pcommMakeNew(m_strPort);
        else if (m_pcommGCIRE->bIsOpen())
            m_pcommGCIRE->Close();

        m_pcommGCIRE->Open();
        m_pcommGCIRE->SetCfg(m_cpcfgSerial);

        //
        //  We have to pause a bit here, since it takes it a couple
        //  seconds to get happy, then issue a read for a few junk
        //  bytes it seems to spit out. If we get a shutdown request
        //  while waiting, just return immediately.
        //
        if (!thrThis.bSleep(2000))
            return kCIDLib::True;

        tCIDLib::TCard1 ac1Junk[10];
        m_pcommGCIRE->c4ReadRawBufMS(ac1Junk, 10, 50);
    }

    catch(TError& errToCatch)
    {
        if(eVerboseLevel() > tCQCKit::EVerboseLvls::Medium)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


//
//  We don't check online here, since it's intended to try to get us back
//  online if possible.
//
tCIDLib::TBoolean TGCIRESDriver::bResetConnection()
{
    // If we aren't online, then throw, else do it
    CheckOnline(CID_FILE, CID_LINE);

    // Force it out of training mode
    m_bTrainingMode = kCIDLib::False;
    ClearEventQ();

    bStoreBoolFld(m_c4FldIdTrainingMode, m_bTrainingMode, kCIDLib::True);
    return (ePing(kCIDLib::False) == tCQCKit::ECommResults::Success);
}


tCIDLib::TVoid TGCIRESDriver::ClearRecTrainingData()
{
    // If we aren't online, then throw, else do it
    CheckOnline(CID_FILE, CID_LINE);
    m_strTrainVal.Clear();
}


tCQCKit::ECommResults
TGCIRESDriver::eConnectToDevice(TThread& thrThis)
{
    // Tell it we want CR terminators
    static const tCIDLib::TCard1 ac1Term[] = { 0x74, 0x63, 0xD };
    m_pcommGCIRE->c4WriteRawBufMS(ac1Term, 3, 100);
    if (!bGetMsg(m_strTmp, 2000, kCIDLib::False) || (m_strTmp != L"terminator,CR"))
    {
        return tCQCKit::ECommResults::LostConnection;
    }

    // We just use the ping method for this, and tell it to store the version
    return ePing(kCIDLib::True);
}


tCQCKit::EDrvInitRes TGCIRESDriver::eInitializeImpl()
{
    // Call our parent IR base drivers
    TParent::eInitializeImpl();

    //
    //  Make sure that we were configured for a serial connection. Otherwise,
    //  its a bad configuration.
    //
    const TCQCDriverObjCfg& cqcdcOurs = cqcdcThis();
    if (cqcdcOurs.conncfgReal().clsIsA() != TCQCSerialConnCfg::clsThis())
    {
        facCQCKit().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcDrv_BadConnCfgType
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Config
            , cqcdcOurs.conncfgReal().clsIsA()
            , clsIsA()
            , TCQCSerialConnCfg::clsThis()
        );
        return tCQCKit::EDrvInitRes::Failed;
    }

    //
    //  Its the right type so do the down cast and get the data out of it
    //  that we need.
    //
    const TCQCSerialConnCfg& conncfgSer
            = static_cast<const TCQCSerialConnCfg&>(cqcdcOurs.conncfgReal());
    m_strPort = conncfgSer.strPortPath();
    m_cpcfgSerial = conncfgSer.cpcfgSerial();

    // Clean up any existing port so it'll get recreated with new config
    delete m_pcommGCIRE;
    m_pcommGCIRE = 0;

    //
    //  Register our couple of fields. We just provide one that indicates
    //  the training mode state, so that the client can display an
    //  indicator that shows whether the driver is in training mode or not.
    //
    TVector<TCQCFldDef> colFlds(8);
    TCQCFldDef flddCmd;

    //
    //  Create the fields that we need for a receiver only type of
    //  IR driver.
    //
    flddCmd.Set
    (
        TFacCQCIR::strFldName_FirmwareVer
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Read
    );
    colFlds.objAdd(flddCmd);

    flddCmd.Set
    (
        TFacCQCIR::strFldName_TrainingState
        , tCQCKit::EFldTypes::Boolean
        , tCQCKit::EFldAccess::Read
    );
    colFlds.objAdd(flddCmd);

    // Tell our base class about our fields
    SetFields(colFlds);

    // Look up the ids of our fields, for efficiency
    m_c4FldIdFirmwareVer = pflddFind
    (
        TFacCQCIR::strFldName_FirmwareVer
        , kCIDLib::True
    )->c4Id();

    m_c4FldIdTrainingMode = pflddFind
    (
        TFacCQCIR::strFldName_TrainingState
        , kCIDLib::True
    )->c4Id();

    //
    //  Set the poll time a little faster than normal, since all we are doing
    //  is listening for data coming in. We'll set 100ms for polls, and 5
    //  seconds for reconnection.
    //
    SetPollTimes(100, 5000);

    //
    //  Crank up the actions processing thread if it's not already. It runs
    //  until we are unloaded pulling events out of the queue and processing
    //  them.
    //
    StartActionsThread();

    //
    //  In our case we want to go to 'wait for config' mode, not wait for
    //  comm res, since we need to get configuration before we can go online.
    //
    return tCQCKit::EDrvInitRes::WaitConfig;
}


tCQCKit::ECommResults TGCIRESDriver::ePollDevice(TThread&)
{
    tCQCKit::ECommResults   eRes = tCQCKit::ECommResults::Success;
    tCIDLib::TEncodedTime   enctCur = TTime::enctNow();
    if (bGetMsg(m_strPollMsg, 10, kCIDLib::False))
    {
        //
        //  See if we have reached the next event time, and if not, we just
        //  eat it. If we have, reset the next event time for the next round
        //  and process it.
        //
        if (enctCur >= m_enctNextEvent)
        {
            m_enctNextEvent = enctCur + GCIRES_DriverImpl::enctMinInterval;
            ProcessSignal(m_strPollMsg);
        }

        //
        //  While there are events still waiting, eat them, since they
        //  will be the subsequent events from the same button press.
        //
        while (bGetMsg(m_strPollMsg, 100, kCIDLib::False));
    }
     else
    {
        // If we've not had any activity for a while, then ping it
        if (enctCur > m_enctLastMsg + GCIRES_DriverImpl::enctPollInterval)
            eRes = ePing(kCIDLib::False);
    }
    return eRes;
}


tCIDLib::TVoid TGCIRESDriver::EnterRecTrainingMode()
{
    // If we aren't online, then throw
    CheckOnline(CID_FILE, CID_LINE);

    // We can't already be in training mode
    if (m_bTrainingMode)
    {
        facCQCIR().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kIRErrs::errcTrain_AlreadyTraining
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Already
            , strMoniker()
        );
    }

    //
    //  We make them wait until all the queued events are processed before
    //  they can do any training. It will only wait for a while and then
    //  throw if the queue doesn't empty.
    //
    WaitForActions();

    // Ok, go and and set the training mode flag and the field
    m_bTrainingMode = kCIDLib::True;

    bStoreBoolFld(m_c4FldIdTrainingMode, m_bTrainingMode, kCIDLib::True);

    // Clear out our current and previous training value fields
    m_strTrainVal.Clear();
}


tCIDLib::TVoid TGCIRESDriver::ExitRecTrainingMode()
{
    // If we aren't online, then throw, else lock and do it
    CheckOnline(CID_FILE, CID_LINE);

    if (!m_bTrainingMode)
    {
        facCQCIR().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kIRErrs::errcTrain_NotTraining
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Already
            , strMoniker()
        );
    }
    m_bTrainingMode = kCIDLib::False;

    bStoreBoolFld(m_c4FldIdTrainingMode, m_bTrainingMode, kCIDLib::True);
}


tCIDLib::TVoid TGCIRESDriver::ReleaseCommResource()
{
    try
    {
        if (m_pcommGCIRE->bIsOpen())
            m_pcommGCIRE->Close();
    }

    catch(const TError& errToCatch)
    {
        LogError(errToCatch, tCQCKit::EVerboseLvls::Medium);
    }
}


tCIDLib::TVoid TGCIRESDriver::TerminateImpl()
{

    // Call our parent class next
    TParent::TerminateImpl();
}


// ---------------------------------------------------------------------------
//  TGCIRESDriver: Private, non-virtual methods
// ---------------------------------------------------------------------------


//
//  The signals we get are compressed, so we have to expand them out. We
//  then use that info to create a hash which we format in the output
//  string.
//
tCIDLib::TBoolean
TGCIRESDriver::bEncodeSignal(const TString& strSig, TString& strKeyToFill)
{
    const tCIDLib::ERadices eRadix(tCIDLib::ERadices::Dec);
    strKeyToFill.Clear();

    // It should start with the prefix
    if (!strSig.bStartsWith(m_strCmdPrefix))
    {
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
        {
            facGCIRES().LogMsg
            (
                CID_FILE
                , CID_LINE
                , L"The IR data did not start with the GC-IRE prefix"
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );
        }
        return kCIDLib::False;
    }

    // We don't keep the prefix since it never changes
    tCIDLib::TCard4 c4Index = m_strCmdPrefix.c4Length();

    const tCIDLib::TCh* pszSrc = strSig.pszBufferAt(c4Index);
    const tCIDLib::TCh* const pszEnd = pszSrc + (strSig.c4Length() - c4Index);

    // Next we get a carrier frequency
    m_strTmp.Clear();
    while ((pszSrc < pszEnd) && (*pszSrc != kCIDLib::chComma))
        m_strTmp.Append(*pszSrc++);

    //
    //  Skip the comma and convert the frequency, though we aren't using
    //  it at this point. At least we verify it's a valid frequency number.
    //
    *pszSrc++;
    tCIDLib::TCard4 c4Freq;
    if (!m_strTmp.bToCard4(c4Freq, eRadix))
    {
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
        {
            facGCIRES().LogMsg
            (
                CID_FILE
                , CID_LINE
                , L"The IR frequency value was not valid"
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );
        }
        return kCIDLib::False;
    }

    //
    //  Now we have to start pulling things in one character at a time.
    //  If we see a number, then it's got to be a pair, so we scan in
    //  up to the next comma, and then to the next comma or non-digit.
    //  For each pair we see, until we get four, we store them as the
    //  repeat pairs.
    //
    //  If a non-digit, it's got to be A, B, C or D, and we pull out the
    //  appropriate repeat pair and copy that over.
    //
    const tCIDLib::TCard4 c4MaxPairs = 1024;
    m_colPairs.RemoveAll();
    TOOPair oopCur;
    {
        const tCIDLib::TCard4 c4MaxReps = 4;
        tCIDLib::TCard4 c4RepCount = 0;
        TOOPair aRepPairs[c4MaxReps];

        while (pszSrc < pszEnd)
        {
            const tCIDLib::TCh chCur = *pszSrc;

            if (TRawStr::bIsDigit(chCur))
            {
                if (m_colPairs.c4ElemCount() >= c4MaxPairs)
                {
                    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
                    {
                        facGCIRES().LogMsg
                        (
                            CID_FILE
                            , CID_LINE
                            , L"Maximum pair count was reached"
                            , tCIDLib::ESeverities::Status
                            , tCIDLib::EErrClasses::AppStatus
                        );
                    }
                    return kCIDLib::False;
                }

                //
                //  Scan forward to the next comma, all of which have to be
                //  digits.
                //
                m_strTmp.Clear();
                while ((pszSrc < pszEnd) && TRawStr::bIsDigit(*pszSrc))
                    m_strTmp.Append(*pszSrc++);

                // We had to hit a comma
                if (*pszSrc != kCIDLib::chComma)
                {
                    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
                    {
                        facGCIRES().LogMsg
                        (
                            CID_FILE
                            , CID_LINE
                            , L"Expected a comma next in IR data"
                            , tCIDLib::ESeverities::Status
                            , tCIDLib::EErrClasses::AppStatus
                        );
                    }
                    return kCIDLib::False;
                }

                // Validate and store the on value
                tCIDLib::TCard4 c4OnVal;
                if (!m_strTmp.bToCard4(c4OnVal, eRadix))
                {
                    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
                    {
                        facGCIRES().LogMsg
                        (
                            CID_FILE
                            , CID_LINE
                            , L"On transition value was not a number"
                            , tCIDLib::ESeverities::Status
                            , tCIDLib::EErrClasses::AppStatus
                        );
                    }
                    return kCIDLib::False;
                }

                //
                //  Now move forward and go till we hit a comma or non-digit
                //  or the end.
                //
                m_strTmp.Clear();
                pszSrc++;
                while ((pszSrc < pszEnd) && TRawStr::bIsDigit(*pszSrc))
                    m_strTmp.Append(*pszSrc++);

                if (pszSrc < pszEnd)
                {
                    if ((*pszSrc != kCIDLib::chComma)
                    &&  ((*pszSrc < kCIDLib::chLatin_A)
                    ||   (*pszSrc > kCIDLib::chLatin_D)))
                    {
                        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
                        {
                            facGCIRES().LogMsg
                            (
                                CID_FILE
                                , CID_LINE
                                , L"Expected a comma or repeat character"
                                , tCIDLib::ESeverities::Status
                                , tCIDLib::EErrClasses::AppStatus
                            );
                        }
                        return kCIDLib::False;
                    }

                    // If it was a comma, we can now skip over it
                    if (*pszSrc == kCIDLib::chComma)
                        pszSrc++;
                }

                // Validate and store the off value
                tCIDLib::TCard4 c4OffVal;
                if (!m_strTmp.bToCard4(c4OffVal, eRadix))
                {
                    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
                    {
                        facGCIRES().LogMsg
                        (
                            CID_FILE
                            , CID_LINE
                            , L"Off transition value was not a number"
                            , tCIDLib::ESeverities::Status
                            , tCIDLib::EErrClasses::AppStatus
                        );
                    }
                    return kCIDLib::False;
                }

                // If we've not filled the rep list yet, then do that
                oopCur.Set(c4OnVal, c4OffVal);
                if (c4RepCount < c4MaxReps)
                    aRepPairs[c4RepCount++] = oopCur;

                // And add it to our list
                m_colPairs.objAdd(oopCur);
            }
             else
            {
                // It's got to be a repeat pair, so look it up
                const tCIDLib::TCard4 c4RepInd = (chCur - kCIDLib::chLatin_A);
                if (c4RepInd >= c4RepCount)
                {
                    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
                    {
                        facGCIRES().LogMsg
                        (
                            CID_FILE
                            , CID_LINE
                            , L"Bad repeat character"
                            , tCIDLib::ESeverities::Status
                            , tCIDLib::EErrClasses::AppStatus
                        );
                    }
                    return kCIDLib::False;
                }

                // Copy over this pair and move forward
                m_colPairs.objAdd(aRepPairs[c4RepInd]);
                pszSrc++;

                // If we are on a comma, move forward
                if (*pszSrc == kCIDLib::chComma)
                    pszSrc++;
            }
        }
    }

    //
    //  Now let's generate the hash and format it out. We have to have
    //  at least 4 pairs, or we do nothing since it can't be anything
    //  meaningful.
    //
    const tCIDLib::TCard4 c4Pairs = m_colPairs.c4ElemCount();
    tCIDLib::TCard4 c4Hash = 0;
    if (c4Pairs > 4)
    {
        const tCIDLib::TCard4 c4Prime32 = 16777619;

        // Start the hash with the hash basis value
        c4Hash = 2166136261;
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Pairs - 1; c4Index++)
        {
            // Get the next two successive pairs
            const TOOPair& oopCur = m_colPairs[c4Index];
            const TOOPair& oopNext = m_colPairs[c4Index + 1];

            //
            //  Compare the on and then off values and add their result to the
            //  hash. We break out if one of them is larger than 1000, which
            //  represents a long break.
            //
            tCIDLib::TCard4 c4Value = c4Compare(oopCur.m_c4On, oopNext.m_c4On);
            c4Hash = (c4Hash * c4Prime32) ^ c4Value;
            if (oopNext.m_c4On > 1000)
                break;

            c4Value = c4Compare(oopCur.m_c4Off, oopNext.m_c4Off);
            c4Hash = (c4Hash * c4Prime32) ^ c4Value;
            if (oopNext.m_c4Off > 1000)
                break;
        }
    }

    // Format the hash out to text
    strKeyToFill.SetFormatted(c4Hash, tCIDLib::ERadices::Dec);

    return kCIDLib::True;
}


// A helper to get a message from the device
tCIDLib::TBoolean
TGCIRESDriver::bGetMsg(         TString&            strToFill
                        , const tCIDLib::TCard4     c4WaitFor
                        , const tCIDLib::TBoolean   bThrowIfNot)
{
    //
    //  See if we get some data. It returns ASCII text, so we can use a
    //  helper method from the base driver class.
    //
    if (!bGetASCIITermMsg(*m_pcommGCIRE, c4WaitFor, 0xD, 0, strToFill))
    {
        if (bThrowIfNot)
        {
            facGCIRES().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kGCIREErrs::errcComm_Timeout
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Timeout
            );
        }
        return kCIDLib::False;
    }

    // Set the last message time stamp
    m_enctLastMsg = TTime::enctNow();
    return kCIDLib::True;
}


//
//  Compare the on or off values of two successive pairs and return a
//  relationship value.
//
tCIDLib::TCard4
TGCIRESDriver::c4Compare(const  tCIDLib::TCard4 c4First
                        , const tCIDLib::TCard4 c4Second) const
{
    if (c4Second < ((c4First * 65) / 100))
        return 0;

    if (c4First < ((c4Second * 65) / 100))
        return 2;

    return 1;
}


//
//  Used to periodically ping the device if we don't hear anything for a
//  while. Since it's getting the version, we also use it on connect, in
//  which case we tell it to store the version in the firmware field.
//
tCQCKit::ECommResults TGCIRESDriver::ePing(const tCIDLib::TBoolean bStore)
{
    // Ask the device for it's version info
    static const tCIDLib::TCard1 ac1Send[] = { 0x67, 0x76, 0xD };
    m_pcommGCIRE->c4WriteRawBufMS(ac1Send, 3, 100);

    if (bGetMsg(m_strTmp, 2000, kCIDLib::False))
    {
        if (bStore)
        {
            m_strTmp.Prepend(L"Version ");
            bStoreStringFld(m_c4FldIdFirmwareVer, m_strTmp, kCIDLib::True);
        }
        return tCQCKit::ECommResults::Success;
    }
    return tCQCKit::ECommResults::LostConnection;
}


//
//  When the poll sees a message come in, it calls us here. WE process
//  any messages that make up the incoming signal and queue the generated
//  key, or store it as training data if in trailing mode.
//
tCIDLib::TVoid TGCIRESDriver::ProcessSignal(const TString& strMsg)
{
    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::High)
    {
        facGCIRES().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"Raw IR event data"
            , strMsg
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
    }

    // Encode the signal in our format
    TString strKey;
    if (bEncodeSignal(strMsg, strKey))
    {
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::High)
        {
            facGCIRES().LogMsg
            (
                CID_FILE
                , CID_LINE
                , L"IR signal was expanded successfully"
                , m_strTmp
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );
        }

        // It's a good signal. If training, then store it else queue it
        if (m_bTrainingMode)
            m_strTrainVal = strKey;
         else
            QueueRecEvent(strKey);
    }
}


