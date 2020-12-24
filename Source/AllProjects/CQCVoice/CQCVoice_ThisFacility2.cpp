//
// FILE NAME: CQCVoice_ThisFacility2.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/02/2016
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
//  This file handles some of the facility methods, to keep it from getting too
//  large.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCVoice.hpp"


// ---------------------------------------------------------------------------
//  TFacCQCVoice: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  A helper to run a global action as defined by the room configuration's global actino
//  info class.
//
tCIDLib::TBoolean TFacCQCVoice::bRunGlobalAct(const TSCGlobActInfo& scliToRun)
{
    // Let's try to download the action
    TCQCStdCmdSrc csrcRun;
    try
    {
        tCIDLib::TCard4         c4SerialNum = 0;
        tCIDLib::TEncodedTime   enctLast;
        tCIDLib::TKVPFList      colMeta;
        TDataSrvClient          dsclRead;
        dsclRead.bReadGlobalAction
        (
            scliToRun.m_strPath
            , c4SerialNum
            , enctLast
            , csrcRun
            , colMeta
            , cuctxToUse().sectUser()
        );
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);
        return kCIDLib::False;
    }

    try
    {
        // Build up the parameters. It's a quoted comma list
        TString strParms(128UL);
        for (tCIDLib::TCard4 c4Index = 0; c4Index < kCQCSysCfg::c4MaxRmModeParms; c4Index++)
            TStringTokenizer::BuildQuotedCommaList(scliToRun.m_colParms[c4Index], strParms);

        //
        //  We want to disable speech input while this happens, in case it does any
        //  text to speech.
        //
        SetRecoPause(kCIDLib::True);

        TCQCStdActEngine acteMode(facCQCVoice.cuctxToUse());
        tCQCKit::ECmdRes eRes = acteMode.eInvokeOps
        (
            csrcRun
            , kCQCKit::strEvId_OnTrigger
            , facCQCVoice.ctarGlobalVars()
            , acteMode.colExtra()
            , acteMode.pcmdtDebug()
            , strParms
        );

        SetRecoPause(kCIDLib::False);
    }

    catch(TError& errToCatch)
    {
        // Make sure recog gets turned back on
        SetRecoPause(kCIDLib::False);

        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        return kCIDLib::False;
    }

    catch(...)
    {
        // Make sure recog gets turned back on
        SetRecoPause(kCIDLib::False);
    }

    return kCIDLib::True;
}


//
//  A helper to set a light level. If it has a dimmer field, we use that. If it only has
//  a color field, we'll use the brightness of the color.
//
tCIDLib::TBoolean
TFacCQCVoice::bSetLightLevel(const TString& strName, const tCIDLib::TCard4 c4Percent)
{
    //
    //  Let's try to find the light. If it fails, tell the user
    //
    const TSCLoadInfo* pscliLight = facCQCVoice.pscliLoadByName(strName);
    if (!pscliLight)
    {
        facCQCVoice.SpeakReply(kCQCVoiceMsgs::midErr_LightNotFound);
        return kCIDLib::False;
    }

    try
    {
        const TString& strMoniker = pscliLight->m_strMoniker;
        tCQCKit::TCQCSrvProxy orbcDrv = facCQCKit().orbcCQCSrvAdminProxy(strMoniker);

        TString strField;
        if (pscliLight->m_strField.bStartsWith(kCQCVoice::strVal_DimPref)
        ||  pscliLight->m_strField2.bStartsWith(kCQCVoice::strVal_DimPref))
        {
            if (pscliLight->m_strField.bStartsWith(kCQCVoice::strVal_DimPref))
                strField = pscliLight->m_strField;
            else
                strField = pscliLight->m_strField2;

            orbcDrv->WriteCardByName
            (
                strMoniker, strField, c4Percent, m_cuctxToUse.sectUser(), tCQCKit::EDrvCmdWaits::Wait
            );
        }
         else if (pscliLight->m_strField.bStartsWith(kCQCVoice::strVal_ClrPref)
              ||  pscliLight->m_strField2.bStartsWith(kCQCVoice::strVal_ClrPref))
        {
            //
            //  In this case we have to read the current value, because we only want to
            //  change the brightness part of the color.
            //
            tCIDLib::TCard4 c4SerNum = 0;
            TString strClr;
            orbcDrv->bReadStringByName(c4SerNum, strMoniker, strField, strClr);

            //
            //  Use the HSV class to parse it, since this is the format it uses, and
            //  it saves us a lot of nitpicky work.
            //
            THSVClr hsvClr;
            if (!hsvClr.bParseFromText(strClr, kCIDLib::True, tCIDLib::ERadices::Dec, kCIDLib::chSpace))
            {

                return kCIDLib::False;
            }

            // Set the value level to the passed level, scaled to 0 to 255
            tCIDLib::TFloat4 f4Val = tCIDLib::TFloat4(c4Percent);
            f4Val /= 100.0F;
            f4Val *= 255.0F;
            if (f4Val > 255.0F)
                f4Val = 255.0F;
            hsvClr.f4Value(f4Val);

            //  Format it back out again, has to be hex format!
            TString strValue;
            hsvClr.FormatToText
            (
                strValue, kCIDLib::chSpace, kCIDLib::True, kCIDLib::False, tCIDLib::ERadices::Hex
            );

            // And write it back out
            orbcDrv->WriteStringByName
            (
                strMoniker, strField, strValue, m_cuctxToUse.sectUser(), tCQCKit::EDrvCmdWaits::Wait
            );
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        facCQCVoice.SpeakFailed();
        return kCIDLib::False;
    }

    // It worked to tell the user it did
    tCIDLib::TMsgId midReply;
    midReply = kCQCVoiceMsgs::midStatus_LevelSet;
    facCQCVoice.SpeakReply(midReply, kCQCVoice::strVal_Light, TCardinal(c4Percent));

    return kCIDLib::True;
}


//
//  A helper to set a light state. If the light has a switch field, we use that. If it
//  has a dimmer only, we set it to 100%. If it has a color only, we set the brightness
//  to 100%.
//
tCIDLib::TBoolean
TFacCQCVoice::bSetLightState(const  TString&            strName
                            , const tCIDLib::TBoolean   bState)
{
    //
    //  Let's try to find the light. If it fails, tell the user
    //
    const TSCLoadInfo* pscliLight = facCQCVoice.pscliLoadByName(strName);
    if (!pscliLight)
    {
        facCQCVoice.SpeakReply(kCQCVoiceMsgs::midErr_LightNotFound);
        return kCIDLib::False;
    }

    try
    {
        const TString& strMoniker = pscliLight->m_strMoniker;
        tCQCKit::TCQCSrvProxy orbcDrv = facCQCKit().orbcCQCSrvAdminProxy(strMoniker);

        TString strField;
        if (pscliLight->m_strField.bStartsWith(kCQCVoice::strVal_SwitchPref)
        ||  pscliLight->m_strField2.bStartsWith(kCQCVoice::strVal_SwitchPref))
        {
            if (pscliLight->m_strField.bStartsWith(kCQCVoice::strVal_SwitchPref))
                strField = pscliLight->m_strField;
            else
                strField = pscliLight->m_strField2;

            orbcDrv->WriteBoolByName
            (
                strMoniker, strField, bState, m_cuctxToUse.sectUser(), tCQCKit::EDrvCmdWaits::Wait
            );
        }
         else if (pscliLight->m_strField.bStartsWith(kCQCVoice::strVal_DimPref)
              ||  pscliLight->m_strField2.bStartsWith(kCQCVoice::strVal_DimPref))
        {
            if (pscliLight->m_strField.bStartsWith(kCQCVoice::strVal_DimPref))
                strField = pscliLight->m_strField;
            else
                strField = pscliLight->m_strField2;

            orbcDrv->WriteCardByName
            (
                strMoniker, strField, bState ? 100 : 0, m_cuctxToUse.sectUser(), tCQCKit::EDrvCmdWaits::Wait
            );
        }
         else if (pscliLight->m_strField.bStartsWith(kCQCVoice::strVal_ClrPref)
              ||  pscliLight->m_strField2.bStartsWith(kCQCVoice::strVal_ClrPref))
        {
            //
            //  In this case we have to read the current value, because we only want to
            //  change the brightness part of the color.
            //
            tCIDLib::TCard4 c4SerNum = 0;
            TString strClr;
            orbcDrv->bReadStringByName(c4SerNum, strMoniker, strField, strClr);

            //
            //  Use the HSV class to parse it, since this is the format it uses, and
            //  it saves us a lot of nitpicky work.
            //
            THSVClr hsvClr;
            if (!hsvClr.bParseFromText(strClr, kCIDLib::True, tCIDLib::ERadices::Dec, kCIDLib::chSpace))
            {

                return kCIDLib::False;
            }

            // Set the value level to 255 or 0
            hsvClr.f4Value(bState ? 255.0F : 0.0F);

            //  Format it back out again, has to be hex format!
            TString strValue;
            hsvClr.FormatToText
            (
                strValue, kCIDLib::chSpace, kCIDLib::True, kCIDLib::False, tCIDLib::ERadices::Hex
            );

            // And write it back out
            orbcDrv->WriteStringByName
            (
                strMoniker, strField, strValue, m_cuctxToUse.sectUser(), tCQCKit::EDrvCmdWaits::Wait
            );
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        facCQCVoice.SpeakFailed();
        return kCIDLib::False;
    }

    //
    //  It worked so tell the user it did. We give the spoken name here if we have it,
    //  else the main title.
    //
    const TString& strSpokenName
    (
        pscliLight->m_strAltName.bIsEmpty() ? strName : pscliLight->m_strAltName
    );

    facCQCVoice.SpeakReply
    (
        bState ? kCQCVoiceMsgs::midStatus_TurnedOn : kCQCVoiceMsgs::midStatus_TurnedOff
        , strSpokenName
        , kCQCVoice::strVal_Light
    );

    return kCIDLib::True;
}


//
//  Handles the grunt work of setting the low or high set point for the thermo assigned
//  to our room. The caller should have checked if we have thermo info. If not this is
//  going to throw. We take the value as a string, so that we also handle the conversion
//  check here. strSPName must be Low or High. We also get the set point name the user
//  actually spoke, which we'll use in the spoken acknowledgement.
//
tCIDLib::TBoolean
TFacCQCVoice::bSetThermoSP(const   TString&    strSPName
                            , const TString&    strVal
                            , const TString&    strSpokenSPName)
{
    // Make sure it's a valid signed number
    tCIDLib::TInt4 i4NewSPVal;
    if (!strVal.bToInt4(i4NewSPVal, tCIDLib::ERadices::Dec))
    {
        facCQCVoice.SpeakReply(L"The temperature value was not a valid signed number");
        return kCIDLib::False;
    }

    // Get our HVAC info
    const TSCHVACInfo& scliOurs = facCQCVoice.scliHVAC();

    // Build up the appropriate field
    TString strSPField(64UL);
    strSPField = L"THERM#";
    if (!scliOurs.m_strThermoSub.bIsEmpty())
    {
        strSPField.Append(scliOurs.m_strThermoSub);
        strSPField.Append(kCIDLib::chTilde);
    }
    strSPField.Append(strSPName);
    strSPField.Append(L"SetPnt");

    try
    {
        const TString& strMoniker = scliOurs.m_strMoniker;
        tCQCKit::TCQCSrvProxy orbcDrv = facCQCKit().orbcCQCSrvAdminProxy(strMoniker);

        //
        //  First let's range check the field. If not in the field's range, it will
        //  tell the user and return false, so we just return false as well.
        //
        if (!bRangeCheckIntFld(orbcDrv, strMoniker, strSPField, i4NewSPVal))
            return kCIDLib::False;

        // Looks ok, so let's set it
        orbcDrv->WriteIntByName
        (
            strMoniker, strSPField, i4NewSPVal, m_cuctxToUse.sectUser(), tCQCKit::EDrvCmdWaits::Wait
        );
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        SpeakReply(kCQCVoiceMsgs::midErr_QuerySP, strSPName);
        return kCIDLib::False;
    }

    // It worked
    SpeakReply(kCQCVoiceMsgs::midSpeak_HLSPSet, strSpokenSPName, TInteger(i4NewSPVal));

    return kCIDLib::True;
}



//  A general purpose field write mechanism for the nodes to use. We can either let
//  any exceptions propagate, or return a false on failure. Defaults to not throwing.
//
tCIDLib::TBoolean
TFacCQCVoice::bWriteField( const   TString&            strMoniker
                            , const TString&            strField
                            , const TString&            strValue
                            , const tCIDLib::TBoolean   bThrowIfErr)
{
    try
    {
        tCQCKit::TCQCSrvProxy orbcDrv = facCQCKit().orbcCQCSrvAdminProxy(strMoniker);
        orbcDrv->WriteFieldByName
        (
            strMoniker, strField, strValue, m_cuctxToUse.sectUser(), tCQCKit::EDrvCmdWaits::Wait
        );
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        if (bThrowIfErr)
            throw;

        TModule::LogEventObj(errToCatch);
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


//
//  Looks for a light with the indicated name. Based on the type of light, it figures
//  out if the light is on or off. If a switch we use the switch field. If only a dimmer
//  is available, we use the dime state. If only a color field, we use the brightness of
//  the color.
//
//  We return an enum because we have to be able to indicate:
//
//  1. We couldn't find the light
//  2. We found it but couldn't read the field
//  3. It was on
//  4. It was off
//
tCQCVoice::ELightReads TFacCQCVoice::eQueryLightState(const TString& strName)
{
    const TSCLoadInfo* pscliLight = facCQCVoice.pscliLoadByName(strName);
    if (!pscliLight)
    {
        facCQCVoice.SpeakReply(kCQCVoiceMsgs::midErr_LightNotFound);
        return tCQCVoice::ELightReads::NotFound;
    }

    tCIDLib::TBoolean bState = kCIDLib::False;
    try
    {
        tCIDLib::TCard4 c4SerNum = 0;
        const TString& strMoniker = pscliLight->m_strMoniker;

        tCQCKit::TCQCSrvProxy orbcDrv = facCQCKit().orbcCQCSrvAdminProxy(strMoniker);

        if (pscliLight->m_strField.bStartsWith(kCQCVoice::strVal_SwitchPref)
        ||  pscliLight->m_strField2.bStartsWith(kCQCVoice::strVal_SwitchPref))
        {
            const TString& strField
            (
                pscliLight->m_strField.bStartsWith(kCQCVoice::strVal_SwitchPref)
                ? pscliLight->m_strField : pscliLight->m_strField2
            );
            orbcDrv->bReadBoolByName(c4SerNum, strMoniker, strField, bState);
        }
         else if (pscliLight->m_strField.bStartsWith(kCQCVoice::strVal_DimPref)
              ||  pscliLight->m_strField2.bStartsWith(kCQCVoice::strVal_DimPref))
        {
            const TString& strField
            (
                pscliLight->m_strField.bStartsWith(kCQCVoice::strVal_DimPref)
                ? pscliLight->m_strField : pscliLight->m_strField2
            );
            tCIDLib::TCard4 c4Level;
            orbcDrv->bReadCardByName(c4SerNum, strMoniker, strField, c4Level);
            bState = (c4Level > 0);
        }
         else if (pscliLight->m_strField.bStartsWith(kCQCVoice::strVal_ClrPref)
              ||  pscliLight->m_strField2.bStartsWith(kCQCVoice::strVal_ClrPref))
        {
            const TString& strField
            (
                pscliLight->m_strField.bStartsWith(kCQCVoice::strVal_ClrPref)
                ? pscliLight->m_strField : pscliLight->m_strField2
            );

            TString strClr;
            orbcDrv->bReadStringByName(c4SerNum, strMoniker, strField, strClr);

            //
            //  Use the HSV class to parse it, since this is the format it uses, and
            //  it saves us a lot of nitpicky work.
            //
            THSVClr hsvClr;
            if (!hsvClr.bParseFromText(strClr, kCIDLib::True, tCIDLib::ERadices::Dec, kCIDLib::chSpace))
                return tCQCVoice::ELightReads::Failed;

            bState = hsvClr.f4Value() != 0.0F;
        }
         else
        {
            facCQCVoice.ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kCQCVoiceErrs::errcCfg_UnknownLightType
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Internal
                , strName
            );
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);
        return tCQCVoice::ELightReads::Failed;
    }

    if (bState)
        return tCQCVoice::ELightReads::On;

    return tCQCVoice::ELightReads::Off;
}


//
//  Handles the case where we get some off/on but weren't too confident we heard the
//  state right. We will wait for an off or on clarification and reutrn that.
//
tCQCVoice::EWaitRepModes TFacCQCVoice::eClarifyOffOn(tCIDLib::TBoolean& bState)
{
    tCIDLib::TStrList colWaitFor(2);

    colWaitFor.objAdd(kCQCVoice::strAction_ClarifyOff);
    colWaitFor.objAdd(kCQCVoice::strAction_ClarifyOn);

    // Enable these rules while we are in here
    TCQCVoiceRuleJan janRule1(kCQCVoice::strAction_ClarifyOff, kCIDLib::True);
    TCQCVoiceRuleJan janRule2(kCQCVoice::strAction_ClarifyOn, kCIDLib::True);

    tCIDLib::TCard4 c4At;
    TCIDSpeechRecoEv sprecevClarify;
    const tCQCVoice::EWaitRepModes eRep = eWaitForAction
    (
        colWaitFor
        , L"off or on clarification"
        , sprecevClarify
        , c4At
    );

    // If it worked, reutrn the state they asked for
    if (eRep == tCQCVoice::EWaitRepModes::Success)
        bState = c4At == 1;

    return eRep;
}


//
//  Handles the case where we get some percentage level but weren't too confident we
//  heard the value right. So we get clarification, or cancel or fail.
//
tCQCVoice::EWaitRepModes TFacCQCVoice::eClarifyPercentage(TString& strLevel)
{
    facCQCVoice.SpeakReply(kCQCVoiceMsgs::midQ_PercentLevel);

    // We will accept a cancellation or a percentage clarification
    tCIDLib::TStrList colWaitFor(1);
    colWaitFor.objAdd(kCQCVoice::strAction_ClarifyPercentVal);

    // Enable this rule while in here
    TCQCVoiceRuleJan janRuleJan(kCQCVoice::strAction_ClarifyPercentVal, kCIDLib::True);

    tCIDLib::TCard4 c4At;
    TCIDSpeechRecoEv sprecevClarify;
    const tCQCVoice::EWaitRepModes eRep = eWaitForAction
    (
        colWaitFor
        , L"percentage level clarification"
        , sprecevClarify
        , c4At
    );

    // If successful, get the new value out
    if (eRep == tCQCVoice::EWaitRepModes::Success)
        strLevel = sprecevClarify.strFindSemVal(L"State");

    return eRep;
}


//
//  Handles the case of clarifying some signed number. It may actually be a percentage
//  or something that cannot ultimately be signed. But we have no way of dealing with
//  that cleanly in the grammar, so we have to just let it be signed the caller has
//  to validate the value.
//
//  The value being confirmed must be in the incoming string, since we can get just
//  a Yes, in which case we need to keep that original value.
//
//  The caller has to speak the message to the user to clarify, so that it can give a
//  context specific message.
//
tCQCVoice::EWaitRepModes TFacCQCVoice::eClarifySignedNum(TString& strValue)
{
    // We will accept a cancellation, a yes, or a numeric value clarification
    tCIDLib::TStrList colWaitFor(2);
    colWaitFor.objAdd(kCQCVoice::strAction_ClarifySignedNum);
    colWaitFor.objAdd(kCQCVoice::strAction_Yes);

    // Enable these rules in here
    TCQCVoiceRuleJan janRule1(kCQCVoice::strAction_ClarifySignedNum, kCIDLib::True);
    TCQCVoiceRuleJan janRule2(kCQCVoice::strAction_Yes, kCIDLib::True);

    tCIDLib::TCard4 c4At;
    TCIDSpeechRecoEv sprecevClarify;
    const tCQCVoice::EWaitRepModes eRep = eWaitForAction
    (
        colWaitFor
        , L"numeric value clarification"
        , sprecevClarify
        , c4At
    );

    // If successful, and it was a clarify get the new value out
    if ((eRep == tCQCVoice::EWaitRepModes::Success) && (c4At == 0))
        strValue = sprecevClarify.strFindSemVal(L"State");

    return eRep;
}


//
//  Handles the common scenario of getting an answer from the user to a 'should I
//  turn it off/on?' question. The caller indicates which state is being asked about.
//
//  In either case we will accept yes or no answers. We will also accept an off/on
//  clarifcation response. In the later case we check that it matches the requested
//  new state. If not, we tell the user that the answer was ambiguous.
//
tCQCVoice::EWaitRepModes TFacCQCVoice::eShouldITurnItX(const tCIDLib::TBoolean bTarState)
{
    tCIDLib::TStrList colWaitFor(4);

    colWaitFor.objAdd(kCQCVoice::strAction_No);
    colWaitFor.objAdd(kCQCVoice::strAction_Yes);
    colWaitFor.objAdd(kCQCVoice::strAction_ClarifyOff);
    colWaitFor.objAdd(kCQCVoice::strAction_ClarifyOn);

    // Enable these rules while we are in here. No is not an on demand type
    TCQCVoiceRuleJan janRule1(kCQCVoice::strAction_ClarifyOff, kCIDLib::True);
    TCQCVoiceRuleJan janRule2(kCQCVoice::strAction_ClarifyOn, kCIDLib::True);
    TCQCVoiceRuleJan janRule4(kCQCVoice::strAction_Yes, kCIDLib::True);

    const TString& strWaitDescr = facCQCVoice.strMsg
    (
        bTarState ? kCQCVoiceMsgs::midSpeak_OnConfirm : kCQCVoiceMsgs::midSpeak_OffConfirm
    );

    tCIDLib::TCard4 c4At;
    TCIDSpeechRecoEv sprecevClarify;
    const tCQCVoice::EWaitRepModes eRep = eWaitForAction
    (
        colWaitFor, strWaitDescr, sprecevClarify, c4At
    );

    // If they selected a clarification action, check it against the state
    if (eRep == tCQCVoice::EWaitRepModes::Success)
    {
        if (((c4At == 2) && bTarState)
        ||  ((c4At == 3) && !bTarState))
        {
            SpeakReply(kCQCVoiceMsgs::midSpeak_WaitingForOther, strWaitDescr);
            return tCQCVoice::EWaitRepModes::Failure;
        }
    }

    return eRep;
}


//
//  We support a number of scenarios where we need to wait for specific responses, usually
//  with one main one and some alternate ones, or sometimes just one of a set of
//  alternatives. We have conveneince methods to support those variations, all of which
//  just call this general purpose one. We take them all as just a list of action
//  names to watch for.
//
//  We always will accept a cancel command and return a cancel result. We require all of
//  the semantics to be at high confidence since this is used for clarification and
//  confirmation.
//
//  If we timeout, we'll tell the user we are giving up and what we were waiting for.
//
//  On each round we'll wait for up to a specific time for a reply before we give up.
//
tCQCVoice::EWaitRepModes
TFacCQCVoice::eWaitForAction(  const   tCIDLib::TStrList&      colWaitForActs
                                , const TString&                strWaitForDescr
                                ,       TCIDSpeechRecoEv&       sprecevToFill
                                ,       tCIDLib::TCard4&        c4MatchedAt)
{
    const tCIDLib::TCard4 c4WaitCnt = colWaitForActs.c4ElemCount();
    tCQCVoice::EWaitRepModes eRet = tCQCVoice::EWaitRepModes::Failure;
    TString strAction;
    while (eRet != tCQCVoice::EWaitRepModes::Success)
    {
        // We want to see low confidence events so that we can say we didn't understand.
        if (bGetNextEvent(sprecevToFill, strAction, kCQCVoice::c4WaitReplyMSs, kCIDLib::True))
        {
            // If all values are at least medium confidence, then check it it
            if (bAllAtConfidence(sprecevToFill.colSemList(), tCQCVoice::EConfLevels::Medium))
            {
                // If it's a cancel command, we stop
                if (strAction.bCompareI(kCQCVoice::strAction_CancelOp))
                {
                    eRet = tCQCVoice::EWaitRepModes::Cancel;
                    break;
                }

                // See if it matches any of our guys
                for (c4MatchedAt = 0; c4MatchedAt < c4WaitCnt; c4MatchedAt++)
                {
                    if (strAction.bCompareI(colWaitForActs[c4MatchedAt]))
                    {
                        //
                        //  This only gets us out of this loop. But the main loop will
                        //  exit because we set success.
                        //
                        eRet = tCQCVoice::EWaitRepModes::Success;
                        break;
                    }
                }
            }

            //
            //  If not solid or not one we want, tell them to try again. We won't see
            //  Cancel here since it will break out of the main loop.
            //
            if (eRet != tCQCVoice::EWaitRepModes::Success)
                facCQCVoice.SpeakTryAgain();
        }
         else
        {
            // No response in the time we are willing to wait, so give up
            eRet = tCQCVoice::EWaitRepModes::Timeout;
            break;
        }
    }

    // If we timed out, tell the user what we were waiting for
    if (eRet == tCQCVoice::EWaitRepModes::Timeout)
    {
        // Tell them we were waiting for something else
        SpeakReply(kCQCVoiceMsgs::midSpeak_WaitingForOther, strWaitForDescr);
    }
    return eRet;
}



//
//  One we get into conversation mode, we have to wait for specific responses. If it
//  gets something else, it will tell the user it was waiting for this (using the
//  wait for description, which is plugs into a message and speaks) and returns
//  false. If it times out, it returns false.
//
//  There isn't a single timeout. We see how long it has been since they last responded,
//  so that, as long as they are trying, and don't tell us to stop, we will keep going.
//
//  We also take an alternate wait for action, which can be empty if it has to be just
//  the one thing. Often they will want for either an explicit action or a yes or no,
//  for instance.
//
tCQCVoice::EWaitRepModes
TFacCQCVoice::eWaitForAction(  const   TString&            strWaitFor
                                , const TString&            strWaitForDescr
                                ,       TCIDSpeechRecoEv&   sprecevToFill)
{
    tCIDLib::TCard4 c4At;
    tCIDLib::TStrList colList(2);
    colList.objAdd(strWaitFor);
    return eWaitForAction(colList, strWaitForDescr, sprecevToFill, c4At);
}


// Like above but we'll wait for the main action or a no
tCQCVoice::EWaitRepModes
TFacCQCVoice::eWaitForActionOrNo(  const   TString&            strWaitFor
                                    , const TString&            strWaitForDescr
                                    ,       TCIDSpeechRecoEv&   sprecevToFill)
{
    tCIDLib::TCard4 c4At;
    tCIDLib::TStrList colList(2);
    colList.objAdd(strWaitFor);
    colList.objAdd(kCQCVoice::strAction_No);
    return eWaitForAction(colList, strWaitForDescr, sprecevToFill, c4At);
}

// Like above but we'll wait for the main action or a yes
tCQCVoice::EWaitRepModes
TFacCQCVoice::eWaitForActionOrYes( const   TString&            strWaitFor
                                    , const TString&            strWaitForDescr
                                    ,       TCIDSpeechRecoEv&   sprecevToFill)
{
    tCIDLib::TCard4 c4At;
    tCIDLib::TStrList colList(2);
    colList.objAdd(strWaitFor);
    colList.objAdd(kCQCVoice::strAction_Yes);
    return eWaitForAction(colList, strWaitForDescr, sprecevToFill, c4At);
}


//
//  This helper just waits for any event where the main rule confidence is at least
//  medium or greater. If it gets low confidence ones, it just says it doesn't understand.
//  As long as the user is responding, and we haven't been asked to stop, we'll keep
//  waiting.
//
//  It is assumed this is called within the context of a conversation, so we ask to see
//  low confidence events and indicate we didn't understand, please try again.
//
tCQCVoice::EWaitRepModes
TFacCQCVoice::eWaitForGoodEvent(       TCIDSpeechRecoEv&   sprecevToFill
                                ,       TString&            strAction
                                , const tCIDLib::TCard4     c4WaitMSs)
{
    tCQCVoice::EWaitRepModes eRet = tCQCVoice::EWaitRepModes::Failure;
    TThread* pthrUs = TThread::pthrCaller();
    while (!pthrUs->bCheckShutdownRequest())
    {
        // We want to see low confidence events so that we can say we didn't understand.
        if (bGetNextEvent(sprecevToFill, strAction, c4WaitMSs, kCIDLib::True))
        {
            const TKeyNumPair& knumAction = *sprecevToFill.pknumFindSemVal
            (
                kCQCVoice::strSemVal_Action
            );

            if (bIsMedConfidence(knumAction))
            {
                // We are going to keep it so store the action and return
                strAction = knumAction.strValue();
                eRet = tCQCVoice::EWaitRepModes::Success;
                break;
            }
             else
            {
                // Ask them to try again
                facCQCVoice.SpeakTryAgain();
            }
        }
         else
        {
            // No response in the time we are willing to wait, so give up
            eRet = tCQCVoice::EWaitRepModes::Timeout;
            break;
        }
    }
    return eRet;
}


//
//  If doing visual replies, we have to remove any markup from the text. The only
//  reasonable way to do that is to let it create the markup XML, then parse it, and
//  pull out the actual text. And we need a little recursive helper to get the
//  resulting text out.
//
static tCIDLib::TVoid ProcessMarkupText(const TXMLTreeElement& xtnodePar, TString& strToFill)
{
    const tCIDLib::TCard4 c4ChildCnt = xtnodePar.c4ChildCount();
    for(tCIDLib::TCard4 c4Index = 0; c4Index < c4ChildCnt; c4Index++)
    {
        const TXMLTreeNode& xtnodeCur = xtnodePar.xtnodeChildAt(c4Index);
        if(xtnodeCur.eType() == tCIDXML::ENodeTypes::Text)
        {
            const TXMLTreeText& xtnodeText = xtnodePar.xtnodeChildAtAsText(c4Index);
            strToFill.Append(xtnodeText.strText());
        }
         else if(xtnodeCur.eType() == tCIDXML::ENodeTypes::Element)
        {
            // Recurse
            const TXMLTreeElement& xtnodeElem = xtnodePar.xtnodeChildAtAsElement(c4Index);
            ProcessMarkupText(xtnodeElem, strToFill);
        }
    }
}

tCIDLib::TVoid
TFacCQCVoice::GetMarkedupText(const TString& strXML, TString& strToFill)
{
    // Create a memory buffer entity source over the source string
    tCIDXML::TEntitySrcRef esrText
    (
        new TMemBufEntitySrc(L"VoiceMarkupText", strXML)
    );

    // And let's parse from that
    try
    {
        TXMLTreeParser xtprsText;

        // Indicate we only want the result character text
        const tCIDLib::TBoolean bOk = xtprsText.bParseRootEntity
        (
            esrText
            , tCIDXML::EParseOpts::None
            , tCIDXML::EParseFlags::TagsNText
        );

        if (!bOk)
        {
            const TXMLTreeParser::TErrInfo& erriFirst
                                            = xtprsText.colErrors()[0];

            TTextStringOutStream strmOut(512);
            strmOut << L"[" << erriFirst.c4Line() << L"." << erriFirst.c4Column()
                    << L"] " << erriFirst.strText() << kCIDLib::FlushIt;

            facCQCVoice.ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kCQCVoiceErrs::errcTTS_ParseMarkup
                , strmOut.strData()
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Internal
            );
        }

        //
        //  Get the text out. We should be able to just iterate a flat list of
        //  text nodes and accumulate them.
        //
        strToFill.Clear();
        const TXMLTreeElement& xtnodeRoot = xtprsText.xtdocThis().xtnodeRoot();
        ProcessMarkupText(xtnodeRoot, strToFill);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        // Give back some thing useful
        strToFill = facCQCVoice.strMsg(kCQCVoiceErrs::errcTTS_ParseMarkup);
    }
}


// A randomiszed general "Call me if you need me" helper
tCIDLib::TVoid TFacCQCVoice::SpeakCallMe()
{
    static const tCIDLib::TMsgId amidOptions[] =
    {
        kCQCVoiceMsgs::midSpeak_CallMe1
        , kCQCVoiceMsgs::midSpeak_CallMe2
        , kCQCVoiceMsgs::midSpeak_CallMe3
        , kCQCVoiceMsgs::midSpeak_CallMe4
        , kCQCVoiceMsgs::midSpeak_CallMe5
    };
    static const tCIDLib::TCard4 c4OptCnt = tCIDLib::c4ArrayElems(amidOptions);

    const tCIDLib::TCard4 c4Sel = (m_randSel.c4GetNextNum() >> 4) % c4OptCnt;

    // Some of these have markup so we have to process them all that way
    SpeakReply(amidOptions[c4Sel], kCIDLib::True);
}


//
//  A randomized general 'I couldn't understand x' reply helper
//
tCIDLib::TVoid
TFacCQCVoice::SpeakCantUnderstand( const TString& strWhat)
{
    static const tCIDLib::TMsgId amidOptions[] =
    {
        kCQCVoiceMsgs::midSpeak_CantUnderstand1
        , kCQCVoiceMsgs::midSpeak_CantUnderstand2
        , kCQCVoiceMsgs::midSpeak_CantUnderstand3
        , kCQCVoiceMsgs::midSpeak_CantUnderstand4
        , kCQCVoiceMsgs::midSpeak_CantUnderstand5
    };
    static const tCIDLib::TCard4 c4OptCnt = tCIDLib::c4ArrayElems(amidOptions);

    const tCIDLib::TCard4 c4Sel = (m_randSel.c4GetNextNum() >> 4) % c4OptCnt;
    SpeakReply(amidOptions[c4Sel], strWhat);
}


// A randomized general 'that was done' reply helper
tCIDLib::TVoid TFacCQCVoice::SpeakDone()
{
    static const tCIDLib::TMsgId amidOptions[] =
    {
        kCQCVoiceMsgs::midSpeak_Done1
        , kCQCVoiceMsgs::midSpeak_Done2
        , kCQCVoiceMsgs::midSpeak_Done3
        , kCQCVoiceMsgs::midSpeak_Done4
        , kCQCVoiceMsgs::midSpeak_Done5
    };
    static const tCIDLib::TCard4 c4OptCnt = tCIDLib::c4ArrayElems(amidOptions);

    const tCIDLib::TCard4 c4Sel = (m_randSel.c4GetNextNum() >> 4) % c4OptCnt;
    SpeakReply(amidOptions[c4Sel]);
}


// A randomized general 'that failed' reply helper
tCIDLib::TVoid TFacCQCVoice::SpeakFailed()
{
    static const tCIDLib::TMsgId amidOptions[] =
    {
        kCQCVoiceMsgs::midSpeak_Failed1
        , kCQCVoiceMsgs::midSpeak_Failed2
        , kCQCVoiceMsgs::midSpeak_Failed3
        , kCQCVoiceMsgs::midSpeak_Failed4
        , kCQCVoiceMsgs::midSpeak_Failed5
    };
    static const tCIDLib::TCard4 c4OptCnt = tCIDLib::c4ArrayElems(amidOptions);

    const tCIDLib::TCard4 c4Sel = (m_randSel.c4GetNextNum() >> 4) % c4OptCnt;
    SpeakReply(amidOptions[c4Sel]);
}


// A randomized 'please wait' reply helper
tCIDLib::TVoid TFacCQCVoice::SpeakPleaseWait()
{
    static const tCIDLib::TMsgId amidOptions[] =
    {
        kCQCVoiceMsgs::midSpeak_OneMoment1
        , kCQCVoiceMsgs::midSpeak_OneMoment2
        , kCQCVoiceMsgs::midSpeak_OneMoment3
        , kCQCVoiceMsgs::midSpeak_OneMoment4
        , kCQCVoiceMsgs::midSpeak_OneMoment5
    };
    static const tCIDLib::TCard4 c4OptCnt = tCIDLib::c4ArrayElems(amidOptions);

    const tCIDLib::TCard4 c4Sel = (m_randSel.c4GetNextNum() >> 4) % c4OptCnt;
    SpeakReply(amidOptions[c4Sel]);
}


//
//  Some methods to conveniently speak loaded text, and insert replacement tokens.
//  Just calls teh main version after setting up the text string.
//
tCIDLib::TVoid
TFacCQCVoice::SpeakReply(  const   tCIDLib::TMsgId     midText
                            , const tCIDLib::TBoolean   bMarkup)
{
    TString strMsg(midText, facCQCVoice);
    SpeakReply(strMsg, bMarkup);
}

tCIDLib::TVoid
TFacCQCVoice::SpeakReply(  const   tCIDLib::TMsgId     midText
                            , const MFormattable&       fmtblToken1
                            , const tCIDLib::TBoolean   bMarkup)
{
    TString strMsg(midText, facCQCVoice, fmtblToken1);
    SpeakReply(strMsg, bMarkup);
}

tCIDLib::TVoid
TFacCQCVoice::SpeakReply(  const   tCIDLib::TMsgId     midText
                            , const MFormattable&       fmtblToken1
                            , const MFormattable&       fmtblToken2
                            , const tCIDLib::TBoolean   bMarkup)
{
    TString strMsg(midText, facCQCVoice, fmtblToken1, fmtblToken2);
    SpeakReply(strMsg, bMarkup);
}

tCIDLib::TVoid
TFacCQCVoice::SpeakReply(  const   tCIDLib::TMsgId     midText
                            , const MFormattable&       fmtblToken1
                            , const MFormattable&       fmtblToken2
                            , const MFormattable&       fmtblToken3
                            , const tCIDLib::TBoolean   bMarkup)
{
    TString strMsg(midText, facCQCVoice, fmtblToken1, fmtblToken2, fmtblToken3);
    SpeakReply(strMsg, bMarkup);
}


//
//  This is for the other versions to call, or for internal slash debugging stuff that
//  we don't bother creating loadable text for.
//
//  We always do sync output, meaning we pause recognition while the speaking is going
//  on and we block until it completes. This is being called from the background thread
//  so it's not blocking the GUI.
//
//  We support a markup mode, where they can pass us text that has embedded xml tags
//  in it that control the speech. In those cases, we build an XML document in a string
//  with the passed text as the body, and pass that to be spoken.
//
//  If we are doing visual text, we pass the text to the visual text window, after
//  removing any market if there is markup involved. If we are not doing spoken text,
//  then we just send the text for display (telling the window it doesn't have to
//  wait for a spoken reply, and return.)
//
tCIDLib::TVoid
TFacCQCVoice::SpeakReply(const TString& strText, const tCIDLib::TBoolean bMarkup)
{
    // Pause recognition while in here if doing spoken text
    SetRecoPause(m_bSpeakReplies);
    try
    {
        //
        //  If markup, then build up the actual text to speak and send it. Else just
        //  pass the text as is.
        //
        if (bMarkup)
        {
            TTextStringOutStream strmText(320UL);
            strmText << L"<?xml version=\"1.0\" encoding=\"" << kCIDLib::pszNativeEncoding
                     << L"\"?>\n"

                     << L"<speak version=\"1.0\" xmlns=\"http://www.w3.org/2001/10/synthesis\""
                     << L" xml:lang=\"en-US\">\n"
                     << strText
                     << L"\n</speak>"
                     << kCIDLib::EndLn;

            //
            //  If doing text replies, then we have to parse this XML and get just the
            //  text content and display that, telling it to blink if we are going to
            //  speak it as well.
            //
            if (m_bTextReplies)
            {
                TString strVisText;
                GetMarkedupText(strmText.strData(), strVisText);
                m_pwndVisText->ShowText(strVisText, m_bSpeakReplies);
            }

            // If speaking replies, do that
            if (m_bSpeakReplies)
                m_speechOut.Speak(strmText.strData(), tCIDLib::ESpeechFlags::Markup);

            // If speaking and text, tell the vis text window to stop blinking
            if (m_bTextReplies && m_bSpeakReplies)
                m_pwndVisText->ReadyToListen();
        }
         else
        {
            // If text replies, send it and tell it to blink if speaking
            if (m_bTextReplies)
                m_pwndVisText->ShowText(strText, m_bSpeakReplies);

            // If speaking replies do that
            if (m_bSpeakReplies)
                m_speechOut.Speak(strText, tCIDLib::ESpeechFlags::None);

            // If text replies and speaking, tell the vis text window to stop blinking
            if (m_bTextReplies && m_bSpeakReplies)
                m_pwndVisText->ReadyToListen();
        }
    }

    catch(TError& errToCatch)
    {
        // If we paused recognition, unpause it
        if (m_bSpeakReplies)
            SetRecoPause(kCIDLib::False);

        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }

    catch(...)
    {
        // <TBD> log something here

        // Fall through
    }

    // If we paused recognition, unpause it
    if (m_bSpeakReplies)
        SetRecoPause(kCIDLib::False);
}


// A randomized 'please try again' reply helper
tCIDLib::TVoid TFacCQCVoice::SpeakTryAgain()
{
    static const tCIDLib::TMsgId amidOptions[] =
    {
        kCQCVoiceMsgs::midSpeak_TryAgain1
        , kCQCVoiceMsgs::midSpeak_TryAgain2
        , kCQCVoiceMsgs::midSpeak_TryAgain3
        , kCQCVoiceMsgs::midSpeak_TryAgain4
        , kCQCVoiceMsgs::midSpeak_TryAgain5
    };
    static const tCIDLib::TCard4 c4OptCnt = tCIDLib::c4ArrayElems(amidOptions);
    const tCIDLib::TCard4 c4Sel = (m_randSel.c4GetNextNum() >> 4) % c4OptCnt;
    SpeakReply(amidOptions[c4Sel]);
}


// A randomized 'what can I do for you?' reply helper
tCIDLib::TVoid TFacCQCVoice::SpeakWhatCanIDo()
{
    static const tCIDLib::TMsgId amidOptions[] =
    {
        kCQCVoiceMsgs::midSpeak_WhatCanIDo1
        , kCQCVoiceMsgs::midSpeak_WhatCanIDo2
        , kCQCVoiceMsgs::midSpeak_WhatCanIDo3
        , kCQCVoiceMsgs::midSpeak_WhatCanIDo4
        , kCQCVoiceMsgs::midSpeak_WhatCanIDo5
    };
    static const tCIDLib::TCard4 c4OptCnt = tCIDLib::c4ArrayElems(amidOptions);
    const tCIDLib::TCard4 c4Sel = (m_randSel.c4GetNextNum() >> 4) % c4OptCnt;
    SpeakReply(amidOptions[c4Sel]);
}


//
//  Find a load by name, which the basic room config classes don't support since it's
//  not normally done for auto-generation and such. So we just have to iterate them
//  and find a matching name (or alt-name.)
//
//  If we find it, we return the load info pointer, else null.
//
const TSCLoadInfo* TFacCQCVoice::pscliLoadByName(const TString& strName) const
{
    const tCIDLib::TCard4 c4Count = m_scfgCurrent.c4ListIdCount(tCQCSysCfg::ECfgLists::Lighting);
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TSCLoadInfo& scliCur = m_scfgCurrent.scliLoadAt(c4Index);

        if (scliCur.m_strTitle.bCompareI(strName))
            return &scliCur;
    }
    return nullptr;
}



