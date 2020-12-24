//
// FILE NAME: CQCVoice_BTWeatherQNodes.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/13/2017
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
//  This file implements a set of nodes that handle simple queries.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCVoice.hpp"
#include    "CQCVoice_BTWeatherQNodes.hpp"



// ---------------------------------------------------------------------------
//   CLASS: TBTQuCurrentTempNode
//  PREFIX: btnode
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TBTQuCurrentTempNode: Constructors and Destructor
// ---------------------------------------------------------------------------
TBTQuCurrentTempNode::TBTQuCurrentTempNode(const TString& strPath, const TString& strName) :

    TBTCQCVoiceNode(strPath, strName, kCQCVoice::strCQCVoiceFact_QueryCurrentTemp, 0, 0)
{
}

TBTQuCurrentTempNode::~TBTQuCurrentTempNode()
{
}


// ---------------------------------------------------------------------------
//  TBTQuCurrentTempNode: Public, virtual methods
// ---------------------------------------------------------------------------
tCIDAI::ENodeStates TBTQuCurrentTempNode::eRun(TAIBehaviorTree& btreeOwner)
{
    TAIBTCtx& ctxTree = btreeOwner.ctxToUse();

    //
    //  Make sure that the temp name is med confidence. It may also be empty since we
    //  don't force them to say it. So, if med confidence, get it but still check it.
    //
    TString strTarTemp;
    if (facCQCVoice.bIsMedConfidence(btreeOwner, kCQCVoice::strSemVal_Target))
        strTarTemp = ctxTree.strFindKNumListVal(kCQCVoice::strSemVal_Target);

    if (strTarTemp.bIsEmpty())
    {
        facCQCVoice.SpeakReply(kCQCVoiceMsgs::midQ_ExtOrInt);

        // Enable this rule temporarily here
        TCQCVoiceRuleJan janRule(kCQCVoice::strAction_ClarifyIntExtTemp, kCIDLib::True);

        TCIDSpeechRecoEv sprecevClarify;
        const tCQCVoice::EWaitRepModes eRep = facCQCVoice.eWaitForAction
        (
            kCQCVoice::strAction_ClarifyIntExtTemp
            , L"internal or external temp"
            , sprecevClarify
        );

        // If not successful, then fail
        if (eRep != tCQCVoice::EWaitRepModes::Success)
            return tCIDAI::ENodeStates::Failure;

        // Else we get the state from this guy
        strTarTemp = sprecevClarify.strFindSemVal(kCQCVoice::strSemVal_Target);
    }

    // Try to get the appropriate temp
    tCIDLib::TInt4 i4Temp = 0;
    try
    {
        tCIDLib::TCard4 c4SerNum = 0;
        tCQCKit::TCQCSrvProxy orbcDrv;

        // Make sure we have that information. If not, tell the user and give up
        if (strTarTemp == kCQCVoice::strVal_External)
        {
            // We have to have a weather source for our room
            TString strWeatherMon;
            if (!facCQCVoice.bCheckRoomWeather(strWeatherMon))
                return tCIDAI::ENodeStates::Failure;

            // Let's read the current weather temp
            orbcDrv = facCQCKit().orbcCQCSrvAdminProxy(strWeatherMon);
            orbcDrv->bReadIntByName(c4SerNum, strWeatherMon, L"WEATH#CurTemp", i4Temp);
        }
         else
        {
            // We have to have a thermostat for our room
            TString strThermoMon, strThermoSub;
            if (!facCQCVoice.bCheckRoomThermo(strThermoMon, strThermoSub))
                return tCIDAI::ENodeStates::Failure;

            // Build the thermo current temp field
            TString strFld = L"THERM#";
            if (!strThermoSub.bIsEmpty())
            {
                strFld.Append(strThermoSub);
                strFld.Append(kCIDLib::chTilde);
            }
            strFld.Append(L"CurrentTemp");

            orbcDrv = facCQCKit().orbcCQCSrvAdminProxy(strThermoMon);
            orbcDrv->bReadIntByName(c4SerNum, strThermoMon, strFld, i4Temp);
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        facCQCVoice.SpeakFailed();
        return tCIDAI::ENodeStates::Failure;
    }

    // We got it, so tell the user the temp
    facCQCVoice.SpeakReply
    (
        kCQCVoiceMsgs::midSpeak_CurTemp, strTarTemp, TInteger(i4Temp)
    );

    // It worked
    return tCIDAI::ENodeStates::Success;
}



// ---------------------------------------------------------------------------
//   CLASS: TBTQuWeatherShortFCNode
//  PREFIX: btnode
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TBTQuWeatherShortFCNode: Constructors and Destructor
// ---------------------------------------------------------------------------
TBTQuWeatherShortFCNode::TBTQuWeatherShortFCNode(const TString& strPath, const TString& strName) :

    TBTCQCVoiceNode(strPath, strName, kCQCVoice::strCQCVoiceFact_QueryWeatherShortFC, 0, 0)
{
}

TBTQuWeatherShortFCNode::~TBTQuWeatherShortFCNode()
{
}


// ---------------------------------------------------------------------------
//  TBTQuWeatherShortFCNode: Public, virtual methods
// ---------------------------------------------------------------------------
tCIDAI::ENodeStates TBTQuWeatherShortFCNode::eRun(TAIBehaviorTree& btreeOwner)
{
    TAIBTCtx& ctxTree = btreeOwner.ctxToUse();

    // Get the day semantic value out
    const TString& strDay = ctxTree.strFindKNumListVal(kCQCVoice::strSemVal_Target);
    const tCIDLib::TCard4 c4DayNum
    (
        strDay.bCompareI(kCQCVoice::strVal_Today) ? 1 : 2
    );

    tCIDLib::TCard4 c4Precip;
    tCIDLib::TInt4 i4Low, i4High;
    try
    {
        // We have to have a weather source for our room
        TString strMoniker;
        if (!facCQCVoice.bCheckRoomWeather(strMoniker))
            return tCIDAI::ENodeStates::Failure;

        tCQCKit::TCQCSrvProxy orbcDrv = facCQCKit().orbcCQCSrvAdminProxy(strMoniker);

        //
        //  We need to read a few fields, but it's not enough to worry about the overhead,
        //  so just do them one at a time.
        //
        tCIDLib::TCard4 c4SerNum;
        TString strFld;
        strFld = L"WEATH#Day";
        strFld.AppendFormatted(c4DayNum);
        const tCIDLib::TCard4 c4FldBaseLen = strFld.c4Length();

        // Get the high temp
        strFld.Append(L"High");
        c4SerNum = 0;
        orbcDrv->bReadIntByName(c4SerNum, strMoniker, strFld, i4High);

        // Get the low temp
        strFld.CapAt(c4FldBaseLen);
        strFld.Append(L"Low");
        c4SerNum = 0;
        orbcDrv->bReadIntByName(c4SerNum, strMoniker, strFld, i4Low);

        // Get the precip percentage
        strFld.CapAt(c4FldBaseLen);
        strFld.Append(L"PrecipProb");
        c4SerNum = 0;
        orbcDrv->bReadCardByName(c4SerNum, strMoniker, strFld, c4Precip);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        facCQCVoice.SpeakFailed();
        return tCIDAI::ENodeStates::Failure;
    }

    // We got it, so format out the info and speak it
    TString strText(kCQCVoiceMsgs::midSpeak_ShortWFC, facCQCVoice);
    strText.eReplaceToken(strDay, kCIDLib::chLatin_d);
    strText.eReplaceToken(TCardinal(c4Precip), kCIDLib::chLatin_p);
    strText.eReplaceToken(TInteger(i4Low), kCIDLib::chLatin_l);
    strText.eReplaceToken(TInteger(i4High), kCIDLib::chLatin_h);

    //
    //  Get the current date info into it. We start with the current date. If it's
    //  for tomorrow, we bump it forward by a day.
    //
    TTime tmCur(tCIDLib::ESpecialTimes::CurrentDate);
    if (c4DayNum > 1)
    {
        const tCIDLib::TCard4 c4Julian = tmCur.c4ToJulian();
        tmCur.FromJulian(c4Julian + 1);
    }

    TString strMsg;
    tmCur.FormatToStr(strMsg, strText);

    // And speak it, indicate it has markup
    facCQCVoice.SpeakReply(strMsg, kCIDLib::True);

    return tCIDAI::ENodeStates::Success;
}
