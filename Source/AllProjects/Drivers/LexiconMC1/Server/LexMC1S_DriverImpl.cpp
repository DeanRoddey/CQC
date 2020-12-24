//
// FILE NAME: LexMC1S_DriverImpl.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/14/2000
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
//  This is is the implementation for the Lexicon MC-1/DC-2 server side
//  driver class.
//
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "LexMC1S.hpp"



// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TLexMC1SDriver,TCQCServerBase)



// ---------------------------------------------------------------------------
//  Local constants
//
//  c4AckTimeout
//      This is the number of milliseconds that the Lexicon is supposed to
//      respond within, if its there.
// ---------------------------------------------------------------------------
namespace LexMC1S_DriverImpl
{
    const tCIDLib::TCard4   c4AckTimeout  = 250;

    #if CID_DEBUG_ON
    const tCIDLib::TCard4   c4PollWait = 2000;
    #else
    const tCIDLib::TCard4   c4PollWait = 250;
    #endif
}



// ---------------------------------------------------------------------------
//   CLASS: TLexMC1SDriver
//  PREFIX: drv
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TLexMC1SDriver: Constructors and Destructor
// ---------------------------------------------------------------------------
TLexMC1SDriver::TLexMC1SDriver(const TCQCDriverObjCfg& cqcdcToLoad) :

    TCQCServerBase(cqcdcToLoad)
    , m_c4FldAdjustVol(kCIDLib::c4MaxCard)
    , m_c4FldBassLevel(kCIDLib::c4MaxCard)
    , m_c4FldEffect(kCIDLib::c4MaxCard)
    , m_c4FldFPDisplay(kCIDLib::c4MaxCard)
    , m_c4FldInputSrc(kCIDLib::c4MaxCard)
    , m_c4FldMute(kCIDLib::c4MaxCard)
    , m_c4FldNPInput(kCIDLib::c4MaxCard)
    , m_c4FldPower(kCIDLib::c4MaxCard)
    , m_c4FldRate(kCIDLib::c4MaxCard)
    , m_c4FldSetFLLev(kCIDLib::c4MaxCard)
    , m_c4FldSetCLev(kCIDLib::c4MaxCard)
    , m_c4FldSetFRLev(kCIDLib::c4MaxCard)
    , m_c4FldSetRLLev(kCIDLib::c4MaxCard)
    , m_c4FldSetRRLev(kCIDLib::c4MaxCard)
    , m_c4FldSetSLLev(kCIDLib::c4MaxCard)
    , m_c4FldSetSRLev(kCIDLib::c4MaxCard)
    , m_c4FldSetSubLev(kCIDLib::c4MaxCard)
    , m_c4FldStream(kCIDLib::c4MaxCard)
    , m_c4FldTogglePower(kCIDLib::c4MaxCard)
    , m_c4FldVolume(kCIDLib::c4MaxCard)
    , m_c4Timeouts(0)
    , m_pcommLex(0)
{
}

TLexMC1SDriver::~TLexMC1SDriver()
{
    // Clean up the comm port if not already
    if (m_pcommLex)
    {
        try
        {
            if (m_pcommLex->bIsOpen())
                m_pcommLex->Close();

            delete m_pcommLex;
            m_pcommLex = 0;
        }

        catch(...)
        {
        }
    }
}


// ---------------------------------------------------------------------------
//  TLexMC1SDriver: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TLexMC1SDriver::bGetCommResource(TThread& thrThis)
{
    try
    {
        // Create the port object, if not yet, else close it just in case
        if (!m_pcommLex)
            m_pcommLex = facCIDComm().pcommMakeNew(m_strPort);
        else if (m_pcommLex->bIsOpen())
            m_pcommLex->Close();

        // Open the port now, and set the configuration
        m_pcommLex->Open(tCIDComm::EOpenFlags::WriteThrough);
        m_pcommLex->SetCfg(m_cpcfgSerial);

        // Purge any partial gorp we might have waiting
        m_pcommLex->PurgeReadData();
    }

    catch(const TError&)
    {
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


tCIDLib::TBoolean TLexMC1SDriver::bWaitConfig(TThread& thrThis)
{
    return kCIDLib::True;
}


tCQCKit::ECommResults
TLexMC1SDriver::eBoolFldValChanged( const   TString&            strName
                                    , const tCIDLib::TCard4     c4FldId
                                    , const tCIDLib::TBoolean   bNewValue)
{
    if (c4FldId == m_c4FldAdjustVol)
    {
        const tCIDLib::TInt4 i4Val = i4FldValue(m_c4FldVolume);
        if (bNewValue && (i4Val < 12))
            SetVolume(i4Val + 1);
        else if (!bNewValue && (i4Val > -80))
            SetVolume(i4Val - 1);
    }
     else if (c4FldId == m_c4FldPower)
    {
        SendIRCode
        (
            *TThread::pthrCaller()
            , bNewValue ? tLexMC1S::EIRCode_PowerOn : tLexMC1S::EIRCode_PowerOff
        );
    }
     else if (c4FldId == m_c4FldFPDisplay)
    {
        SendIRCode
        (
            *TThread::pthrCaller()
            , bNewValue ? tLexMC1S::EIRCode_FPDOn : tLexMC1S::EIRCode_FPDOff
        );
    }
     else if (c4FldId == m_c4FldNPInput)
    {
        //
        //  There are fx up/down IR commands we could invoke, but they are
        //  kludgey because the first press after a short period doesn't
        //  do anything, it just shows the current f/x and you have to press
        //  it again to make it do anything. So we do it ourself here manually
        //  to insure it works consistently.
        //
        //  So read the effect field to get the current value, and then move
        //  up or down as required, wrapping as required.
        //
        const TString& strCurFX = strFldVal(m_c4FldEffect);

        // Translate it to our enum value
        tLexMC1S::EEffects eFX = eXlatEffectName(strCurFX);
        if (eFX == tLexMC1S::EEffects_Count)
        {
            eFX = tLexMC1S::EEffects_Min;
        }
         else if (bNewValue)
        {
            if (eFX == tLexMC1S::EEffects_Max)
                eFX = tLexMC1S::EEffects_First;
            else
                eFX++;
        }
         else
        {
            if (eFX <= tLexMC1S::EEffects_First)
                eFX = tLexMC1S::EEffects_Max;
            else
                eFX--;
        }

        TLexMsg lmSend(tLexMC1S::ECmd_SetActiveEffect);
        lmSend.AddDataByte(eFX);
        lmSend.SendOn(*m_pcommLex);
        TLexMsg lmReply;
        GetReply(lmSend, lmReply);
    }
     else if (c4FldId == m_c4FldMute)
    {
        SendIRCode(*TThread::pthrCaller(), tLexMC1S::EIRCode_FullMute);
    }
     else if (c4FldId == m_c4FldTogglePower)
    {
        // Send the code for the opposite of the current power state
        tLexMC1S::EIRCodes eCode;
        if (bFldValue(m_c4FldPower))
            eCode = tLexMC1S::EIRCode_PowerOff;
        else
            eCode = tLexMC1S::EIRCode_PowerOn;
        SendIRCode(*TThread::pthrCaller(), eCode);
    }
     else
    {
        IncUnknownWriteCounter();
        return tCQCKit::ECommResults::FieldNotFound;
    }
    return tCQCKit::ECommResults::Success;
}


tCQCKit::ECommResults
TLexMC1SDriver::eConnectToDevice(TThread& thrThis)
{
    try
    {
        TLexMsg lmSend(tLexMC1S::ECmd_HostWakeup);
        lmSend.SendOn(*m_pcommLex);
        TLexMsg lmReply;
        GetReply(lmSend, lmReply, kCIDLib::False, 1000);

        //
        //  For whatever reason, it freaks out if we don't delay after the
        //  host wakeup. We use the special driver aware sleep method in
        //  our base class.
        //
        bSleep(500);

        // Init our communications since he seems to be alive
        InitComm();

        //
        //  Wake up the Lexicon. We do this because it doesn't let us figure
        //  out what its current awake/asleep state is.
        //
        SendIRCode(thrThis, tLexMC1S::EIRCode_PowerOn);

        //
        //  And sleep a bit more to let the power up finish, since it won't
        //  repond for a bit as it starts up. Use the shutdown tolerant
        //  version so we'll break out on a shutdown request.
        //
        //  We use a special sleep function here which is sensitive both to
        //  shutdown requests on the thread and to cleanup requests by the
        //  ORB. If it interrupts us, we just continue with the setup and
        //  return as quickly as possible.
        //
        bSleep(3000);

        // Set the power field to true now
        bStoreBoolFld(m_c4FldPower, kCIDLib::True, kCIDLib::True);

        // And do an initial poll
        PollFromMC1(lmSend, lmReply);
    }

    catch(const TError& errToCatch)
    {
        // Just eat it if its a no-reply error
        if (!errToCatch.bCheckEvent(facLexMC1S().strName()
                                    , kMC1SErrs::errcProto_NoReply))
        {
            LogError(errToCatch, tCQCKit::EVerboseLvls::Medium);
        }
        return tCQCKit::ECommResults::LostConnection;
    }
    return tCQCKit::ECommResults::Success;
}


tCQCKit::EDrvInitRes TLexMC1SDriver::eInitializeImpl()
{
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

    // Clean up any existing port, so that it gets redone with new config
    delete m_pcommLex;
    m_pcommLex = 0;

    //
    //  Load up the list of fields and register them
    //
    TVector<TCQCFldDef> colFlds;
    LoadFieldList(colFlds);
    SetFields(colFlds);

    // Look up our field ids for later use
    m_c4FldAdjustVol = pflddFind(kLexMC1S::pszAdjustVolume, kCIDLib::True)->c4Id();
    m_c4FldBassLevel = pflddFind(kLexMC1S::pszBassLevel, kCIDLib::True)->c4Id();
    m_c4FldEffect = pflddFind(kLexMC1S::pszEffect, kCIDLib::True)->c4Id();
    m_c4FldFPDisplay = pflddFind(kLexMC1S::pszFPDisplay, kCIDLib::True)->c4Id();
    m_c4FldInputSrc = pflddFind(kLexMC1S::pszInputSrc, kCIDLib::True)->c4Id();
    m_c4FldMute = pflddFind(kLexMC1S::pszMute, kCIDLib::True)->c4Id();
    m_c4FldNPInput = pflddFind(kLexMC1S::pszNPInput, kCIDLib::True)->c4Id();
    m_c4FldPower = pflddFind(kLexMC1S::pszPower, kCIDLib::True)->c4Id();
    m_c4FldRate = pflddFind(kLexMC1S::pszRate, kCIDLib::True)->c4Id();
    m_c4FldSetCLev = pflddFind(kLexMC1S::pszSetCLev, kCIDLib::True)->c4Id();
    m_c4FldSetFLLev = pflddFind(kLexMC1S::pszSetFLLev, kCIDLib::True)->c4Id();
    m_c4FldSetFRLev = pflddFind(kLexMC1S::pszSetFRLev, kCIDLib::True)->c4Id();
    m_c4FldSetRLLev = pflddFind(kLexMC1S::pszSetRLLev, kCIDLib::True)->c4Id();
    m_c4FldSetRRLev = pflddFind(kLexMC1S::pszSetRRLev, kCIDLib::True)->c4Id();
    m_c4FldSetSLLev = pflddFind(kLexMC1S::pszSetSLLev, kCIDLib::True)->c4Id();
    m_c4FldSetSRLev = pflddFind(kLexMC1S::pszSetSRLev, kCIDLib::True)->c4Id();
    m_c4FldSetSubLev = pflddFind(kLexMC1S::pszSetSubLev, kCIDLib::True)->c4Id();
    m_c4FldStream = pflddFind(kLexMC1S::pszStream, kCIDLib::True)->c4Id();
    m_c4FldTogglePower = pflddFind(kLexMC1S::pszTogglePwr, kCIDLib::True)->c4Id();
    m_c4FldVolume = pflddFind(kLexMC1S::pszVolume, kCIDLib::True)->c4Id();

    //
    //  We want faster poll times than the default, because of the high
    //  comm speed, interactivity, and amount of data of the device.
    //
    SetPollTimes(200, 2000);

    return tCQCKit::EDrvInitRes::WaitCommRes;
}


tCQCKit::ECommResults
TLexMC1SDriver::eIntFldValChanged(  const   TString&        strName
                                    , const tCIDLib::TCard4 c4FldId
                                    , const tCIDLib::TInt4  i4NewValue)
{
    if (c4FldId == m_c4FldVolume)
    {
        // We have a dedicated helper for this one
        SetVolume(i4NewValue);
    }
     else if ((c4FldId == m_c4FldBassLevel)
          ||  (c4FldId == m_c4FldSetFLLev)
          ||  (c4FldId == m_c4FldSetCLev)
          ||  (c4FldId == m_c4FldSetFRLev)
          ||  (c4FldId == m_c4FldSetSLLev)
          ||  (c4FldId == m_c4FldSetSRLev)
          ||  (c4FldId == m_c4FldSetRLLev)
          ||  (c4FldId == m_c4FldSetRRLev)
          ||  (c4FldId == m_c4FldSetSubLev))
    {
        // Convert the field id to the protocol id for the speaker
        tCIDLib::TCard1 c1Id;
        if (c4FldId == m_c4FldSetFLLev)
            c1Id = 2;
        else if (c4FldId == m_c4FldSetCLev)
            c1Id = 0;
        else  if (c4FldId == m_c4FldSetFRLev)
            c1Id = 3;
        else  if (c4FldId == m_c4FldSetSLLev)
            c1Id = 4;
        else  if (c4FldId == m_c4FldSetSRLev)
            c1Id = 5;
        else  if (c4FldId == m_c4FldSetRLLev)
            c1Id = 6;
        else  if (c4FldId == m_c4FldSetRRLev)
            c1Id = 7;
        else  if ((c4FldId == m_c4FldBassLevel)
              ||  (c4FldId == m_c4FldSetSubLev))
        {
            c1Id = 1;
        }

        //
        //  The value we get is -10 to +10, which is in db. The real value
        //  the Lex wants is in 0.5 DB, so the number of units in that range
        //  is twice as much, and zero based. So, adjust by 10 to get a zero
        //  based value, and then multiply by 2.
        //
        //  The id for the base level is 1, so we pass that as the second
        //  parameter.
        //
        TLexMsg lmSend(tLexMC1S::ECmd_SetOutLevels);
        lmSend.AddDataByte(tCIDLib::TCard1((i4NewValue + 10) * 2));
        lmSend.AddDataByte(c1Id);
        lmSend.SendOn(*m_pcommLex);
        TLexMsg lmReply;
        GetReply(lmSend, lmReply);
    }
     else
    {
        IncUnknownWriteCounter();
        return tCQCKit::ECommResults::FieldNotFound;
    }
    return tCQCKit::ECommResults::Success;
}


tCQCKit::ECommResults TLexMC1SDriver::ePollDevice(TThread& thrThis)
{
    tCQCKit::ECommResults eRes = tCQCKit::ECommResults::Success;
    try
    {
        TLexMsg lmSend;
        TLexMsg lmReply;
        PollFromMC1(lmSend, lmReply);

        // It works, so reset the timeouts counter
        m_c4Timeouts = 0;
    }

    catch(const TError& errToCatch)
    {
        LogError(errToCatch, tCQCKit::EVerboseLvls::High);

        // If we hit 20 timeouts, then assume he go bye bye
        m_c4Timeouts++;
        if (m_c4Timeouts >= 20)
        {
            IncTimeoutCounter();
            m_c4Timeouts = 0;
            eRes = tCQCKit::ECommResults::LostConnection;
        }
    }
    return eRes;
}


tCQCKit::ECommResults
TLexMC1SDriver::eStringFldValChanged(const  TString&        strName
                                    , const tCIDLib::TCard4 c4FldId
                                    , const TString&        strNewValue)
{
    if (c4FldId == m_c4FldEffect)
    {
        const tLexMC1S::EEffects eFX = eXlatEffectName(strNewValue);
        if (eFX == tLexMC1S::EEffects_Count)
            return tCQCKit::ECommResults::BadValue;

        TLexMsg lmSend(tLexMC1S::ECmd_SetActiveEffect);
        lmSend.AddDataByte(eFX);
        lmSend.SendOn(*m_pcommLex);

        TLexMsg lmReply;
        GetReply(lmSend, lmReply);
    }
     else if (c4FldId == m_c4FldInputSrc)
    {
        const tLexMC1S::EInputs eInp = eXlatInputName(strNewValue);
        if (eInp == tLexMC1S::EInputs_Count)
            return tCQCKit::ECommResults::BadValue;

        // Make the input to an IR command to change to that input
        const tLexMC1S::EIRCodes aeMap[tLexMC1S::EInputs_Count] =
        {
            tLexMC1S::EIRCode_Tape
            , tLexMC1S::EIRCode_Tuner
            , tLexMC1S::EIRCode_CD
            , tLexMC1S::EIRCode_Aux
            , tLexMC1S::EIRCode_TV
            , tLexMC1S::EIRCode_VDisc
            , tLexMC1S::EIRCode_DVD
            , tLexMC1S::EIRCode_VCR
        };
        SendIRCode(*TThread::pthrCaller(), aeMap[eInp]);
    }
     else
    {
        IncUnknownWriteCounter();
        return tCQCKit::ECommResults::FieldNotFound;
    }
    return tCQCKit::ECommResults::Success;
}


tCIDLib::TVoid TLexMC1SDriver::ReleaseCommResource()
{
    //
    //  Send a host sleep command, to let him know we are going away. No
    //  reply is expected.
    //
    try
    {
        if (m_pcommLex->bIsOpen())
        {
            TLexMsg lmSend(tLexMC1S::ECmd_Sleep);
            lmSend.SendOn(*m_pcommLex);

            //
            //  Give it a little time to insure that it gets out before we
            //  get destroyed and our comm port gets destroyed.
            //
            TThread::Sleep(100);
        }

        m_pcommLex->Close();
    }

    catch(const TError& errToCatch)
    {
        LogError(errToCatch, tCQCKit::EVerboseLvls::Medium);
    }
}


tCIDLib::TVoid TLexMC1SDriver::TerminateImpl()
{
}


// ---------------------------------------------------------------------------
//  TLexMC1SDriver: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Processes some incoming command we got. The cases we have to deal with
//  are:
//
//  1. It's an ack or response to something we sent it
//  2. It's some async change notification that we need to process and store
//     away the results of.
//  3. It's a sleep/wakeup notification
//
//  We eat #2 and #3 type events internally.
//
//  NOTE: We assume our caller has already locked the sync mutex
//
tCIDLib::TBoolean
TLexMC1SDriver::bProcessMsg(const   TLexMsg&            lmToProc
                            , const tCIDLib::TBoolean   bFromPollThread)
{
    tCIDLib::TBoolean bRet = kCIDLib::True;
    switch(lmToProc.eCmd())
    {
        case tLexMC1S::ECmd_Sleep :
        {
            //
            //  Just mark us as not awake. It will also send this if its
            //  hard powered up into standby mode.
            //
            bStoreBoolFld(m_c4FldPower, kCIDLib::False, kCIDLib::True);
            break;
        }

        case tLexMC1S::ECmd_ParmChange :
        case tLexMC1S::ECmd_FrontPan :
        {
            //
            //  Just eat these. We disable them but one might get in before
            //  we get a chance to do that.
            //
            break;
        }

        case tLexMC1S::ECmd_Wakeup :
        {
            //
            //  Send back an ack for this command, and mark us as awake now.
            //  It will also send this if it was hard powered up into
            //  the awake mode.
            //
            bStoreBoolFld(m_c4FldPower, kCIDLib::True, kCIDLib::True);

            lmToProc.AckOn(*m_pcommLex);
            if (eState() != tCQCKit::EDrvStates::Connected)
            {
                InitComm();
                bSleep(500);
            }
            break;
        }

        case tLexMC1S::ECmd_SysStatus :
        {
            //
            //  If this is not in the context of the polling thread, then
            //  somehow we've gotten out of sync, perhaps he timed out and
            //  this left one of these in the pipeline. So we'll process
            //  it here ourself.
            //
            if (!bFromPollThread)
                StoreSysStatus(lmToProc);
            else
                bRet = kCIDLib::False;
            break;
        }

        default :
            // Its not a special one
            bRet = kCIDLib::False;
            break;
    }
    return bRet;
}


tLexMC1S::EEffects
TLexMC1SDriver::eXlatEffectName(const   TString&            strToXlat
                                , const tCIDLib::TBoolean   bThrowIfNot)
{
    tLexMC1S::EEffects eRet = tLexMC1S::EEffects_Min;
    for (; eRet <= tLexMC1S::EEffects_Max; eRet++)
    {
        if (strToXlat.bCompare(kLexMC1S::apszEffectNames[eRet]))
            return eRet;
    }

    if (bThrowIfNot)
    {
        facLexMC1S().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMC1SErrs::errcVal_Effect
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::BadParms
            , strToXlat
        );
    }

    // Make the compiler happy
    return eRet;
}


tLexMC1S::EInputs
TLexMC1SDriver::eXlatInputName( const   TString&            strToXlat
                                , const tCIDLib::TBoolean   bThrowIfNot)
{
    tLexMC1S::EInputs eRet = tLexMC1S::EInputs_Min;
    for (; eRet <= tLexMC1S::EInputs_Max; eRet++)
    {
        if (strToXlat.bCompare(kLexMC1S::apszInputNames[eRet]))
            return eRet;
    }

    if (bThrowIfNot)
    {
        facLexMC1S().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMC1SErrs::errcVal_Input
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::BadParms
            , strToXlat
        );
    }

    // Make the compiler happy
    return eRet;
}


//
//  We assume anyone calling this has gotten a lock on the sync mutex already,
//  since its usually called by someone who already locked in order to send
//  out whatever we are getting the reply to.
//
tCIDLib::TVoid
TLexMC1SDriver::GetReply(const  TLexMsg&            lmOrg
                        ,       TLexMsg&            lmToFill
                        , const tCIDLib::TBoolean   bFromPollThread
                        , const tCIDLib::TCard4     c4Timeout)
{
    // Calc the end time, so we'll know when we are done
    const tCIDLib::TCard4 c4EndTime = TTime::c4Millis() + c4Timeout;

    //
    //  Loop until we get the reply or no reply. For the rest of the messages
    //  we get, just pass them to bProcessMsg() to make sure we store away
    //  any data we get.
    //
    tLexMC1S::EErrors eErr;
    while (TTime::c4Millis() < c4EndTime)
    {
        //
        //  Get a message from the MC-1. Pass it our end time, so that it
        //  can wait for up to that long.
        //
        const TLexMsg::EMsgResults eRes = lmToFill.eReadFrom
        (
            *m_pcommLex
            , eErr
            , c4EndTime
        );

        if (eRes != TLexMsg::EMsgResult_Ok)
        {
            if (eRes == TLexMsg::EMsgResult_NoReply)
            {
                IncTimeoutCounter();
                facLexMC1S().ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kMC1SErrs::errcProto_NoReply
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Protocol
                    , TCardinal(lmOrg.eCmd(), tCIDLib::ERadices::Hex)
                );
            }
             else if (eRes == TLexMsg::EMsgResult_MalformedReply)
            {
                // Purge input on the comm port to try to get back in sync
                m_pcommLex->PurgeReadData();

                IncBadMsgCounter();
                facLexMC1S().ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kMC1SErrs::errcProto_MalformedReply
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Protocol
                    , TCardinal(lmOrg.eCmd(), tCIDLib::ERadices::Hex)
                );
            }
             else if (eRes == TLexMsg::EMsgResult_Nack)
            {
                IncNakCounter();
                facLexMC1S().ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kMC1SErrs::errcProto_Nack
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Protocol
                    , TCardinal(lmOrg.eCmd(), tCIDLib::ERadices::Hex)
                    , TCardinal(lmToFill[1])
                );
            }
        }
         else
        {
            //
            //  Process it. If its not one of the messages that automatically
            //  get processed, then assume its the reply so check it.
            //
            if (!bProcessMsg(lmToFill, bFromPollThread))
            {
                if (lmToFill.bIsReplyTo(lmOrg))
                    return;

                //
                //  If not an ack, then log it. Acks can legitimately
                //  get left for us to see here, because some commands
                //  take too long for us to wait for them.
                //
                if (lmToFill.eCmd() != tLexMC1S::ECmd_Ack)
                {
                    facLexMC1S().LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , kMC1SErrs::errcProto_WrongReply
                        , tCIDLib::ESeverities::Warn
                        , tCIDLib::EErrClasses::Protocol
                        , TCardinal(lmToFill.eCmd(), tCIDLib::ERadices::Hex)
                        , TCardinal(lmOrg.eCmd(), tCIDLib::ERadices::Hex)
                    );
                }
            }
        }
    }

    // We never got a reply
    facLexMC1S().ThrowErr
    (
        CID_FILE
        , CID_LINE
        , kMC1SErrs::errcProto_NoReply
        , tCIDLib::ESeverities::Failed
        , tCIDLib::EErrClasses::Protocol
        , TCardinal(lmOrg.eCmd(), tCIDLib::ERadices::Hex)
    );
}



//
//  This is just to get some code out of the init method, because of the size.
//  It loads up the pass list with our fields we want to register.
//
tCIDLib::TVoid TLexMC1SDriver::LoadFieldList(TVector<TCQCFldDef>& colLoad)
{
    TCQCFldDef flddCur;

    // Clean it out in case it is being reused.
    colLoad.RemoveAll();

    // To allow us to make the code below a lot less wordy
    const tCQCKit::EFldAccess eRd = tCQCKit::EFldAccess::Read;
    const tCQCKit::EFldAccess eRW = tCQCKit::EFldAccess::ReadWrite;
    const tCQCKit::EFldAccess eWr = tCQCKit::EFldAccess::Write;

    //
    //  Build up some limits strings from their respective name arrays, so
    //  that we will automatically pick up any changes.
    //
    TString strInpLims(L"Enum:");
    tLexMC1S::EInputs eInp = tLexMC1S::EInputs_Min;
    for (; eInp <= tLexMC1S::EInputs_Max; eInp++)
    {
        if (eInp != tLexMC1S::EInputs_Min)
            strInpLims.Append(kCIDLib::chComma);
        strInpLims.Append(kLexMC1S::apszInputNames[eInp]);
    }

    TString strFXLims(L"Enum:");
    tLexMC1S::EEffects eFX = tLexMC1S::EEffects_Min;
    for (; eFX <= tLexMC1S::EEffects_Max; eFX++)
    {
        if (eFX != tLexMC1S::EEffects_Min)
            strFXLims.Append(kCIDLib::chComma);
        strFXLims.Append(kLexMC1S::apszEffectNames[eFX]);
    }

    TString strProdLims(L"Enum:");
    tLexMC1S::EProducts eProd = tLexMC1S::EProducts_Min;
    for (; eProd <= tLexMC1S::EProducts_Max; eProd++)
    {
        if (eProd != tLexMC1S::EProducts_Min)
            strProdLims.Append(kCIDLib::chComma);
        strProdLims.Append(kLexMC1S::apszProductNames[eProd]);
    }

    TString strRateLims(L"Enum:");
    tLexMC1S::ESampleRates eRate = tLexMC1S::ESampleRates_Min;
    for (; eRate <= tLexMC1S::ESampleRates_Max; eRate++)
    {
        if (eRate != tLexMC1S::ESampleRates_Min)
            strRateLims.Append(kCIDLib::chComma);
        strRateLims.Append(kLexMC1S::apszSampleRateNames[eRate]);
    }

    TString strStreamLims(L"Enum:");
    tLexMC1S::EStreamTypes eStream = tLexMC1S::EStreamTypes_Min;
    for (; eStream <= tLexMC1S::EStreamTypes_Max; eStream++)
    {
        if (eStream != tLexMC1S::EEffects_Min)
            strStreamLims.Append(kCIDLib::chComma);
        strStreamLims.Append(kLexMC1S::apszStreamTypeNames[eStream]);
    }

    TString strSWLLims(L"Enum:");
    tLexMC1S::ESWLevels eSWLev = tLexMC1S::ESWLevels_Min;
    for (; eSWLev <= tLexMC1S::ESWLevels_Max; eSWLev++)
    {
        if (eSWLev != tLexMC1S::ESWLevels_Min)
            strSWLLims.Append(kCIDLib::chComma);
        strSWLLims.Append(kLexMC1S::apszSoftwareLevelNames[eSWLev]);
    }

    TString strSWTLims(L"Enum:");
    tLexMC1S::ESWTypes eSWType = tLexMC1S::ESWTypes_Min;
    for (; eSWType <= tLexMC1S::ESWTypes_Max; eSWType++)
    {
        if (eSWType != tLexMC1S::ESWTypes_Min)
            strSWTLims.Append(kCIDLib::chComma);
        strSWTLims.Append(kLexMC1S::apszSoftwareTypeNames[eSWType]);
    }

    // And load up the fields
    flddCur.Set(kLexMC1S::pszEffect, tCQCKit::EFldTypes::String, eRW, strFXLims);
    colLoad.objAdd(flddCur);

    flddCur.Set(kLexMC1S::pszInputSrc, tCQCKit::EFldTypes::String, eRW, strInpLims);
    colLoad.objAdd(flddCur);

    flddCur.Set(kLexMC1S::pszMute, tCQCKit::EFldTypes::Boolean, eRW);
    colLoad.objAdd(flddCur);

    flddCur.Set(kLexMC1S::pszPower, tCQCKit::EFldTypes::Boolean, eRW);
    colLoad.objAdd(flddCur);

    flddCur.Set(kLexMC1S::pszRate, tCQCKit::EFldTypes::String, eRd, strRateLims);
    colLoad.objAdd(flddCur);

    flddCur.Set(kLexMC1S::pszStream, tCQCKit::EFldTypes::String, eRd, strStreamLims);
    colLoad.objAdd(flddCur);

    flddCur.Set(kLexMC1S::pszVolume, tCQCKit::EFldTypes::Int, eRW, L"Range:-80,12");
    colLoad.objAdd(flddCur);


    // Do the write only fields
    flddCur.Set(kLexMC1S::pszAdjustVolume, tCQCKit::EFldTypes::Boolean, eWr, kCIDLib::True);
    colLoad.objAdd(flddCur);

    const TString strLevLims(L"Range: -10, 10");
    flddCur.Set(kLexMC1S::pszBassLevel, tCQCKit::EFldTypes::Int, eWr, strLevLims);
    colLoad.objAdd(flddCur);
    flddCur.Set(kLexMC1S::pszSetCLev, tCQCKit::EFldTypes::Int, eWr, strLevLims);
    colLoad.objAdd(flddCur);
    flddCur.Set(kLexMC1S::pszSetFLLev, tCQCKit::EFldTypes::Int, eWr, strLevLims);
    colLoad.objAdd(flddCur);
    flddCur.Set(kLexMC1S::pszSetFRLev, tCQCKit::EFldTypes::Int, eWr, strLevLims);
    colLoad.objAdd(flddCur);
    flddCur.Set(kLexMC1S::pszSetRLLev, tCQCKit::EFldTypes::Int, eWr, strLevLims);
    colLoad.objAdd(flddCur);
    flddCur.Set(kLexMC1S::pszSetRRLev, tCQCKit::EFldTypes::Int, eWr, strLevLims);
    colLoad.objAdd(flddCur);
    flddCur.Set(kLexMC1S::pszSetSLLev, tCQCKit::EFldTypes::Int, eWr, strLevLims);
    colLoad.objAdd(flddCur);
    flddCur.Set(kLexMC1S::pszSetSRLev, tCQCKit::EFldTypes::Int, eWr, strLevLims);
    colLoad.objAdd(flddCur);
    flddCur.Set(kLexMC1S::pszSetSubLev, tCQCKit::EFldTypes::Int, eWr, strLevLims);
    colLoad.objAdd(flddCur);

    flddCur.Set(kLexMC1S::pszFPDisplay, tCQCKit::EFldTypes::Boolean, eWr, kCIDLib::True);
    colLoad.objAdd(flddCur);

    flddCur.Set(kLexMC1S::pszNPInput, tCQCKit::EFldTypes::Boolean, eWr, kCIDLib::True);
    colLoad.objAdd(flddCur);

    flddCur.Set(kLexMC1S::pszTogglePwr, tCQCKit::EFldTypes::Boolean, eWr, kCIDLib::True);
    colLoad.objAdd(flddCur);

    //
    //  And do the read only fields that are set once upon connect ot the
    //  device and left alone. Clients also generally only read these once
    //  upon connecting to us, or when they run some special display dialog
    //  that displays this static info.
    //
    flddCur.Set(kLexMC1S::pszProduct, tCQCKit::EFldTypes::String, eRd, strProdLims);
    colLoad.objAdd(flddCur);

    flddCur.Set(kLexMC1S::pszSWType, tCQCKit::EFldTypes::String, eRd, strSWTLims);
    colLoad.objAdd(flddCur);

    flddCur.Set(kLexMC1S::pszSWLevel, tCQCKit::EFldTypes::String, eRd, strSWLLims);
    colLoad.objAdd(flddCur);

    flddCur.Set(kLexMC1S::pszSWVersion, tCQCKit::EFldTypes::Card, eRd);
    colLoad.objAdd(flddCur);

    flddCur.Set(kLexMC1S::pszProtoVersion, tCQCKit::EFldTypes::Card, eRd);
    colLoad.objAdd(flddCur);

    flddCur.Set(kLexMC1S::pszTimeStamp, tCQCKit::EFldTypes::String, eRd);
    colLoad.objAdd(flddCur);
}


//
//  This method is called when we first establish contact with the Lexicon,
//  either when we first come up or we see it doing a hard startup. We set
//  it for the options we want and get some static data that never changes.
//
tCIDLib::TVoid TLexMC1SDriver::InitComm()
{
    TLexMsg         lmSend;
    TLexMsg         lmReply;

    // Turn off all front panel notifications
    lmSend.Clear();
    lmSend.eCmd(tLexMC1S::ECmd_SetFPDRegs);
    lmSend.AddDataByte(0);
    lmSend.AddDataByte(0);
    lmSend.AddDataByte(0);
    lmSend.AddDataByte(0);
    lmSend.SendOn(*m_pcommLex);
    GetReply(lmSend, lmReply);

    //
    //  Tell it to send us acks but no async notifications, don't use the
    //  LED.
    //
    lmSend.Clear();
    lmSend.eCmd(tLexMC1S::ECmd_SetComConfig);
    lmSend.AddDataByte(1);
    lmSend.SendOn(*m_pcommLex);
    GetReply(lmSend, lmReply);

    //
    //  Ok, lets get the general system configuration data and store that
    //  stuff away. These are all fields that we only have to store once upon
    //  connect and don't poll.
    //
    lmSend.Clear();
    lmSend.eCmd(tLexMC1S::ECmd_GetSysConfig);
    lmSend.SendOn(*m_pcommLex);
    GetReply(lmSend, lmReply);

    bStoreStringFld
    (
        kLexMC1S::pszProduct
        , kLexMC1S::apszProductNames[lmReply[0]]
        , kCIDLib::True
    );

    bStoreStringFld
    (
        kLexMC1S::pszSWType
        , kLexMC1S::apszSoftwareTypeNames[lmReply[1]]
        , kCIDLib::True
    );

    bStoreStringFld
    (
        kLexMC1S::pszSWLevel
        , kLexMC1S::apszSoftwareLevelNames[lmReply[2]]
        , kCIDLib::True
    );

    bStoreCardFld
    (
        kLexMC1S::pszSWVersion
        , TRawBits::c4From16(lmReply[4], lmReply[3])
        , kCIDLib::True
    );

    bStoreCardFld
    (
        kLexMC1S::pszProtoVersion
        , TRawBits::c4From16(lmReply[6], lmReply[5])
        , kCIDLib::True
    );

    m_c4MaxSysParms = lmReply[7] - 1;
    m_c4MaxEffects  = lmReply[8] - 1;

    TString strTmp;
    m_tcvrtLex.c4ConvertFrom(lmReply.pc1DataAt(9), 14, strTmp);
    bStoreStringFld(kLexMC1S::pszTimeStamp, strTmp, kCIDLib::True);
}


tCIDLib::TVoid TLexMC1SDriver::PollFromMC1(TLexMsg& lmSend, TLexMsg& lmReply)
{
    lmSend.Clear();
    lmSend.eCmd(tLexMC1S::ECmd_GetSysStatus);
    lmSend.SendOn(*m_pcommLex);
    GetReply(lmSend, lmReply, kCIDLib::True, LexMC1S_DriverImpl::c4PollWait);
    StoreSysStatus(lmReply);
}


tCIDLib::TVoid
TLexMC1SDriver::SendIRCode(         TThread&            thrThis
                            , const tLexMC1S::EIRCodes  eIRCode)
{
    // Build an IR code message and send it
    TLexMsg lmSend(tLexMC1S::ECmd_SendIR);
    lmSend.AddDataByte(eIRCode);
    lmSend.SendOn(*m_pcommLex);

    //
    //  If its a power on or off command, then we don't wait for the
    //  ack/nak since it will take too long. It'll just be seen later
    //  as an unneeded async response and ignored.
    //
    if ((eIRCode != tLexMC1S::EIRCode_PowerOn)
    &&  (eIRCode != tLexMC1S::EIRCode_PowerOff))
    {
        TLexMsg lmReply;
        tCIDLib::TCard4 c4Wait = 250;
        GetReply(lmSend, lmReply, kCIDLib::False, c4Wait);

        if (lmReply.eCmd() == tLexMC1S::ECmd_Nack)
        {
            facLexMC1S().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kMC1SErrs::errcProto_Nack
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Internal
                , TCardinal(lmReply.eCmd(), tCIDLib::ERadices::Hex)
                , TCardinal(lmReply[0])
            );
        }
    }

    //
    //  And hold everyone at bay while it completes since the Lex
    //  doesn't respond so well after these ops.
    //
    bSleep(250);
}


tCIDLib::TVoid TLexMC1SDriver::SetVolume(const tCIDLib::TInt4 i4ToSet)
{
    // Make sure its legal
    if ((i4ToSet < -80) || (i4ToSet > 12))
    {
        facLexMC1S().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMC1SErrs::errcVal_Volume
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Range
            , TInteger(i4ToSet)
        );
    }

    // Convert to the MC-1 representation, which is unsigned by biasing it
    const tCIDLib::TCard1 c1New = tCIDLib::TCard1(i4ToSet + 80);

    TLexMsg lmSend(tLexMC1S::ECmd_SetSysVolume);
    lmSend.AddDataByte(c1New);
    lmSend.SendOn(*m_pcommLex);

    TLexMsg lmReply;
    GetReply(lmSend, lmReply);
}


tCIDLib::TVoid TLexMC1SDriver::StoreSysStatus(const TLexMsg& lmStatus)
{
    // Get the results into temps, to test for changes
    const tLexMC1S::EStreamTypes eStream = tLexMC1S::EStreamTypes(lmStatus[4]);

    bStoreStringFld
    (
        m_c4FldEffect
        , kLexMC1S::apszEffectNames[lmStatus[2]]
        , kCIDLib::True
    );
    bStoreStringFld
    (
        m_c4FldInputSrc
        , kLexMC1S::apszInputNames[lmStatus[1]]
        , kCIDLib::True
    );
    bStoreBoolFld(m_c4FldMute, lmStatus[5] != 0, kCIDLib::True);
    bStoreStringFld
    (
        m_c4FldRate
        , kLexMC1S::apszSampleRateNames[lmStatus[3]]
        , kCIDLib::True
    );
    bStoreStringFld
    (
        m_c4FldStream
        , kLexMC1S::apszStreamTypeNames[lmStatus[4]]
        , kCIDLib::True
    );
    bStoreIntFld(m_c4FldVolume, tCIDLib::TInt4(lmStatus[0]) - 80, kCIDLib::True);
}

