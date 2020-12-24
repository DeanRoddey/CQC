//
// FILE NAME: GE-IRLS_DriverImpl.cpp
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
#include    "GC-IRLS.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TGCIRLSDriver,TBaseIRSrvDriver)


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace GCIRLS_DriverImpl
{
    // How long between active polls if we see no activity
    const tCIDLib::TEncodedTime enctPollInterval = kCIDLib::enctOneMinute;
}



// ---------------------------------------------------------------------------
//   CLASS: THostMonSDriver
//  PREFIX: drv
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  THostMonSDriver: Constructors and Destructor
// ---------------------------------------------------------------------------
TGCIRLSDriver::TGCIRLSDriver(const TCQCDriverObjCfg& cqcdcToLoad) :

    TBaseIRSrvDriver(cqcdcToLoad)
    , m_bTrainingMode(kCIDLib::False)
    , m_c4FldIdFirmwareVer(kCIDLib::c4MaxCard)
    , m_c4FldIdTrainingMode(kCIDLib::c4MaxCard)
    , m_enctLastMsg(0)
    , m_pcommGCIRL(0)
    , m_strPrefixIRData(L"GC-IRL,")
    , m_strPrefixIREnd(L"IRend,")
    , m_strPrefixVer(L"ver,")
    , m_strPort()
{
}


TGCIRLSDriver::~TGCIRLSDriver()
{
    // Clean up the comm port if not already
    if (m_pcommGCIRL)
    {
        try
        {
            if (m_pcommGCIRL->bIsOpen())
                m_pcommGCIRL->Close();

            delete m_pcommGCIRL;
            m_pcommGCIRL= 0;
        }

        catch(...)
        {
        }
    }
}


// ---------------------------------------------------------------------------
//  TGCIRLSDriver: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TGCIRLSDriver::bBlastTrainingMode() const
{
    // No need to lock for this simple check
    return m_bTrainingMode;
}


tCIDLib::TBoolean
TGCIRLSDriver::bCheckBlastTrainingData( tCIDLib::TCard4&    c4DataBytes
                                        , THeapBuf&         mbufToFill)
{
    // If we aren't online, then throw
    CheckOnline(CID_FILE, CID_LINE);

    if (m_bGotTrainingData)
    {
        //
        //  We got something, so copy it over and clear the flags. We store
        //  it as the ASCII text, so we do a cheap transcode to the buffer.
        //
        c4DataBytes = m_strTrainVal.c4Length();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4DataBytes; c4Index++)
            mbufToFill.PutCard1(tCIDLib::TCard1(m_strTrainVal[c4Index]), c4Index);

        m_bTrainingMode = kCIDLib::False;
        m_bGotTrainingData = kCIDLib::False;

        bStoreBoolFld(m_c4FldIdTrainingMode, kCIDLib::False, kCIDLib::True);
        return kCIDLib::True;
    }

    c4DataBytes = 0;
    return kCIDLib::False;
}



tCIDLib::TBoolean
TGCIRLSDriver::bCvtManualBlastData( const   TString&            strText
                                    ,       tCIDLib::TCard4&    c4DataBytes
                                    ,       THeapBuf&           mbufToFill
                                    ,       TString&            strError)
{
    // If it's a UIRT code, convert it. Else it has to be a GC-100 code.
    TString strActual;
    if (facCQCIR().bIsValidUIRTBlastData(strText))
    {
        // Convert to GC-100 format
        TTextStringOutStream strmCvt(&strActual);
        facCQCIR().ProntoToGC100(strText, strmCvt);
    }
     else
    {
        if (!facCQCIR().bIsValidGC100BlastData(strText))
        {
            strError.LoadFromMsg
            (
                kIRErrs::errcFmt_BadIRDataFormat, facCQCIR(), TString(L"GC-100")
            );
            return kCIDLib::False;
        }
        strActual = strText;
    }

    //
    //  The flattened format is just the single byte version of the text,
    //  so we just do a cheap transcode fo the characters to short chars via
    //  truncation.
    //
    const tCIDLib::TCard4 c4Len = strActual.c4Length();
    CIDAssert(c4Len != 0, L"The incoming IR data length was zero");
    for (c4DataBytes = 0; c4DataBytes < c4Len; c4DataBytes++)
        mbufToFill.PutCard1(tCIDLib::TCard1(strActual[c4DataBytes]), c4DataBytes);

    return kCIDLib::True;
}


tCIDLib::TBoolean
TGCIRLSDriver::bGetCommResource(TThread& thrThis)
{
    // Open the port now, and set the configuration
    try
    {
        // Create the port object if needed, else close if open just in case
        if (!m_pcommGCIRL)
            m_pcommGCIRL = facCIDComm().pcommMakeNew(m_strPort);
        else if (m_pcommGCIRL->bIsOpen())
            m_pcommGCIRL->Close();

        m_pcommGCIRL->Open();
        m_pcommGCIRL->SetCfg(m_cpcfgSerial);

        //
        //  We have to pause a bit here, since it takes it a couple
        //  seconds to get happy, then issue a read for a few junk
        //  bytes it seems to spit out. If we get a shutdown request
        //  while waiting, just return immediately.
        //
        if (!thrThis.bSleep(8000))
            return kCIDLib::True;

        tCIDLib::TCard1 ac1Junk[10];
        m_pcommGCIRL->c4ReadRawBufMS(ac1Junk, 10, 50);
    }

    catch(TError& errToCatch)
    {
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::High)
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
tCIDLib::TBoolean TGCIRLSDriver::bResetConnection()
{
    // If we aren't online, then throw, else do it
    CheckOnline(CID_FILE, CID_LINE);

    // Force it out of training mode
    m_bTrainingMode = kCIDLib::False;
    bStoreBoolFld(m_c4FldIdTrainingMode, m_bTrainingMode, kCIDLib::True);
    return (ePing(kCIDLib::False) == tCQCKit::ECommResults::Success);
}



tCIDLib::TVoid TGCIRLSDriver::ClearBlastTrainingData()
{
    // If we aren't online, then throw, else do it
    CheckOnline(CID_FILE, CID_LINE);
    m_strTrainVal.Clear();
    m_bGotTrainingData = kCIDLib::False;
}


tCQCKit::ECommResults
TGCIRLSDriver::eConnectToDevice(TThread& thrThis)
{
    //
    //  First just ping it to see if it's basically alive, and tell the
    //  helper to store any data he gets.
    //
    tCQCKit::ECommResults eRes = ePing(kCIDLib::True);
    if (eRes == tCQCKit::ECommResults::Success)
    {
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::High)
        {
            facGCIRLS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , L"Ping worked, setting IR end mode"
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );
        }

        // Set the IR end mode to the longest one
        static const tCIDLib::TCard1 ac1Send[] = { 0x65, 0x34, 0xD };
        m_pcommGCIRL->c4WriteRawBufMS(ac1Send, 3, 100);

        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::High)
        {
            facGCIRLS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , L"Waiting for set end mode reply"
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );
        }

        // If it's not the expected response, give up
        TString strData;
        if (!bWaitReply(m_strPrefixIREnd, strData, 3000))
        {
            if (eVerboseLevel() >= tCQCKit::EVerboseLvls::High)
            {
                facGCIRLS().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , L"Did not get IR end mode reply"
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::AppStatus
                );
            }
            eRes = tCQCKit::ECommResults::LostConnection;
        }
    }
    return eRes;
}


tCQCKit::EDrvInitRes TGCIRLSDriver::eInitializeImpl()
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
    delete m_pcommGCIRL;
    m_pcommGCIRL = 0;

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
        TFacCQCIR::strFldName_FirmwareVer, kCIDLib::True
    )->c4Id();

    m_c4FldIdTrainingMode = pflddFind
    (
        TFacCQCIR::strFldName_TrainingState, kCIDLib::True
    )->c4Id();

    //
    //  Set the poll time a little faster than normal, since all we are doing
    //  is listening for data coming in. We'll set 100ms for polls, and 5
    //  seconds for reconnection.
    //
    SetPollTimes(100, 5000);

    //
    //  We are not a blaster, but we have to be able to edit models, so
    //  we do have configuration. The user has to load the models up inside
    //  us in order to allow that.
    //
    return tCQCKit::EDrvInitRes::WaitConfig;
}


tCQCKit::ECommResults TGCIRLSDriver::ePollDevice(TThread&)
{
    tCQCKit::ECommResults   eRes = tCQCKit::ECommResults::Success;
    tCIDLib::TEncodedTime   enctCur = TTime::enctNow();
    tCIDLib::TEncodedTime   enctEnd = enctCur + (kCIDLib::enctOneMilliSec * 5);
    if (bGetMsg(m_strPollMsg, enctEnd))
    {
        //
        //  See if we have reached the next event time, and if not, we just
        //  eat it. If we have, reset the next event time for the next round
        //  and process it.
        //
        if (m_strPollMsg.bStartsWith(m_strPrefixIRData))
            ProcessSignal(m_strPollMsg);
    }
     else
    {
        // If we've not had any activity for a while, then ping it
        if (enctCur > m_enctLastMsg + GCIRLS_DriverImpl::enctPollInterval)
            eRes = ePing(kCIDLib::False);
    }
    return eRes;
}


tCIDLib::TVoid TGCIRLSDriver::EnterBlastTrainingMode()
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

    // Ok, go and and set the training mode flag and the field
    m_bTrainingMode = kCIDLib::True;
    bStoreBoolFld(m_c4FldIdTrainingMode, m_bTrainingMode, kCIDLib::True);

    // Clear out our training value fields
    m_bGotTrainingData = kCIDLib::False;
    m_strTrainVal.Clear();
}


tCIDLib::TVoid TGCIRLSDriver::ExitBlastTrainingMode()
{
    // If we aren't online, then throw, else do it
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




// We have to format the data for the indicated command to text
tCIDLib::TVoid
TGCIRLSDriver::FormatBlastData( const   TIRBlasterCmd&  irbcFmt
                                ,       TString&        strToFill)
{
    //
    //  The data is just the text as ASCII, so we can do a cheap transcode
    //  to the string by just expanding each byte.
    //
    const tCIDLib::TCard4 c4Count = irbcFmt.c4DataLen();
    strToFill.Clear();
    const tCIDLib::TCard1* pc1Src = irbcFmt.mbufData().pc1Data();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        strToFill.Append(tCIDLib::TCh(*pc1Src++));
}


tCIDLib::TVoid TGCIRLSDriver::ReleaseCommResource()
{
    try
    {
        if (m_pcommGCIRL->bIsOpen())
            m_pcommGCIRL->Close();
    }

    catch(TError& errToCatch)
    {
        LogError(errToCatch, tCQCKit::EVerboseLvls::Medium, CID_FILE, CID_LINE);
    }
}


tCIDLib::TVoid TGCIRLSDriver::TerminateImpl()
{

    // Call our parent class next
    TParent::TerminateImpl();
}


// ---------------------------------------------------------------------------
//  TGCIRLSDriver: Private, non-virtual methods
// ---------------------------------------------------------------------------


//
//  The signals we get are compressed, so we have to expand them out.
//
tCIDLib::TBoolean
TGCIRLSDriver::bDecodeSignal(const TString& strSig, TString& strToFill)
{
    const tCIDLib::ERadices eRadix(tCIDLib::ERadices::Dec);
    strToFill.Clear();

    // It should start with the prefix
    if (!strSig.bStartsWith(m_strPrefixIRData))
    {
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
        {
            facGCIRLS().LogMsg
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

    //
    //  We don't keep the prefix since it never changes, so start after that
    //  nad parse the rest.
    //
    tCIDLib::TCard4 c4Index = m_strPrefixIRData.c4Length();
    const tCIDLib::TCh* pszSrc = strSig.pszBufferAt(c4Index);
    const tCIDLib::TCh* const pszEnd = pszSrc + (strSig.c4Length() - c4Index);

    // Next we get a carrier frequency
    TString strDecoded;
    while ((pszSrc < pszEnd) && (*pszSrc != kCIDLib::chComma))
        strDecoded.Append(*pszSrc++);

    // Skip the comma and convert the frequency
    *pszSrc++;
    tCIDLib::TCard4 c4Freq;
    if (!strDecoded.bToCard4(c4Freq, eRadix))
    {
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
        {
            facGCIRLS().LogMsg
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
    tCIDLib::TCard4 c4PairsFound = 0;
    TOOPair aoopSig[c4MaxPairs];
    {
        const tCIDLib::TCard4 c4MaxReps = 4;
        tCIDLib::TCard4 c4RepCount = 0;
        TOOPair aRepPairs[c4MaxReps];

        while (pszSrc < pszEnd)
        {
            const tCIDLib::TCh chCur = *pszSrc;

            if (TRawStr::bIsDigit(chCur))
            {
                if (c4PairsFound >= c4MaxPairs)
                {
                    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
                    {
                        facGCIRLS().LogMsg
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

                // Get the next pair to fill in
                TOOPair& oopCur = aoopSig[c4PairsFound++];

                //
                //  Scan forward to the next comma, all of which have to be
                //  digits.
                //
                strDecoded.Clear();
                while ((pszSrc < pszEnd) && TRawStr::bIsDigit(*pszSrc))
                    strDecoded.Append(*pszSrc++);

                // We had to hit a comma
                if (*pszSrc != kCIDLib::chComma)
                {
                    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
                    {
                        facGCIRLS().LogMsg
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
                if (!strDecoded.bToCard4(oopCur.c4On, eRadix))
                {
                    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
                    {
                        facGCIRLS().LogMsg
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
                strDecoded.Clear();
                pszSrc++;
                while ((pszSrc < pszEnd) && TRawStr::bIsDigit(*pszSrc))
                    strDecoded.Append(*pszSrc++);

                if (pszSrc < pszEnd)
                {
                    if ((*pszSrc != kCIDLib::chComma)
                    &&  ((*pszSrc < kCIDLib::chLatin_A)
                    ||   (*pszSrc > kCIDLib::chLatin_D)))
                    {
                        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
                        {
                            facGCIRLS().LogMsg
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
                if (!strDecoded.bToCard4(oopCur.c4Off, eRadix))
                {
                    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
                    {
                        facGCIRLS().LogMsg
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

                // If we've not filled the rep list yet, then at it to the list
                if (c4RepCount < c4MaxReps)
                    aRepPairs[c4RepCount++] = oopCur;
            }
             else
            {
                // It's got to be a rep pair
                const tCIDLib::TCard4 c4RepInd = (chCur - kCIDLib::chLatin_A);
                if (c4RepInd >= c4RepCount)
                {
                    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
                    {
                        facGCIRLS().LogMsg
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
                aoopSig[c4PairsFound++] = aRepPairs[c4RepInd];
                pszSrc++;

                // If we are on a comma, move forward
                if (*pszSrc == kCIDLib::chComma)
                    pszSrc++;
            }
        }
    }

    //
    //  OK, now we can format it out to the final form. Start with the standard
    //  prefix, then the frequency, then another standard bit, then a standard
    //  offset of 1.
    //
    strToFill = L"sendir,%(m):%(c),%(i),";
    strToFill.AppendFormatted(c4Freq);
    strToFill.Append(L",%(C),1");

    // Now format out all of the pairs
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4PairsFound; c4Index++)
    {
        strToFill.Append(kCIDLib::chComma);

        const TOOPair& pairCur = aoopSig[c4Index];
        strToFill.AppendFormatted(pairCur.c4On);
        strToFill.Append(kCIDLib::chComma);
        strToFill.AppendFormatted(pairCur.c4Off);
    }
    return kCIDLib::True;
}


// A helper to get a message from the device
tCIDLib::TBoolean
TGCIRLSDriver::bGetMsg(         TString&                strToFill
                        , const tCIDLib::TEncodedTime   enctEnd)
{
    //
    //  See if we get some data. It returns ASCII text, so we can use a
    //  helper method from the base driver class.
    //
    if (bGetASCIITermMsg2(*m_pcommGCIRL, enctEnd, 0xD, 0, strToFill))
    {
        // Set the last message time stamp
        m_enctLastMsg = TTime::enctNow();
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


//
//  Since this guy sends asyncs, when we wait for a reply, we have to ignore
//  the asycs, passing them off to be processed. The caller provides the
//  prefix of the expected response for us to compare against.
//
tCIDLib::TBoolean
TGCIRLSDriver::bWaitReply(  const   TString&            strExpPref
                            ,       TString&            strToFill
                            , const tCIDLib::TCard4     c4WaitMS)
{
    tCIDLib::TEncodedTime enctCur = TTime::enctNow();
    const tCIDLib::TEncodedTime enctEnd
    (
        enctCur + (kCIDLib::enctOneMilliSec * c4WaitMS)
    );

    while (enctCur < enctEnd)
    {
        // If we don't get anything within the remaining time, then give up
        if (!bGetMsg(strToFill, enctEnd))
            break;

        // If it starts with the expected prefix, it's our guy
        if (strToFill.bStartsWith(strExpPref))
            return kCIDLib::True;

        // See if it's IR data. If so, process it that way
        if (strToFill.bStartsWith(m_strPrefixIRData))
            ProcessSignal(strToFill);

        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::High)
        {
            if (strToFill.c4Length() > 16)
                strToFill.CapAt(16);
            facGCIRLS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , L"Got unknown message, starting with %(1)"
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
                , strToFill
            );
        }

        enctCur = TTime::enctNow();
    }

    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::High)
    {
        facGCIRLS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"Failed to get response. Expected prefix=%(1)"
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
            , strExpPref
        );
    }

    //
    //  If we get here, we never saw the message and they said don't throw
    //  if we don't get one, so we just return false.
    //
    return kCIDLib::False;
}



//
//  Used to periodically ping the device if we don't hear anything for a
//  while. Since it's getting the version, we also use it on connect, in
//  which case we tell it to store the version in the firmware field.
//
tCQCKit::ECommResults TGCIRLSDriver::ePing(const tCIDLib::TBoolean bStore)
{
    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::High)
    {
        facGCIRLS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"Pinging the IRL"
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
    }

    // Ask the device for it's version info
    static const tCIDLib::TCard1 ac1Send[] = { 0x67, 0x76, 0xD };
    m_pcommGCIRL->c4WriteRawBufMS(ac1Send, 3, 250);

    //
    //  Wait for a reply. Tell him not to throw. If none, just return
    //  failure.
    //
    TString strData;
    if (!bWaitReply(m_strPrefixVer, strData, 2500))
        return tCQCKit::ECommResults::LostConnection;

    // We get it, so if they ask to store, do that
    if (bStore)
    {
        strData.Cut(0, 4);
        strData.Prepend(L"Version ");

        bStoreStringFld(m_c4FldIdFirmwareVer, strData, kCIDLib::True);
    }
    return tCQCKit::ECommResults::Success;
}


//
//  When the poll sees a message come in, it calls us here. WE process
//  any messages that make up the incoming signal and queue the generated
//  key, or store it as training data if in trailing mode.
//
tCIDLib::TVoid TGCIRLSDriver::ProcessSignal(const TString& strMsg)
{
    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
    {
        facGCIRLS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"IR event data received in training"
            , strMsg
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
    }

    // If in training mode, then process it
    if (m_bTrainingMode)
    {
        // Encode the signal in our format
        TString strKey;
        if (bDecodeSignal(strMsg, strKey))
        {
            if (eVerboseLevel() >= tCQCKit::EVerboseLvls::High)
            {
                facGCIRLS().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , L"IR signal was expanded successfully"
                    , strKey
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::AppStatus
                );
            }

            // It's a good signal. If training, then store it else queue it
            if (m_bTrainingMode)
            {
                m_bGotTrainingData = kCIDLib::True;
                m_strTrainVal = strKey;
            }
        }
    }
}


