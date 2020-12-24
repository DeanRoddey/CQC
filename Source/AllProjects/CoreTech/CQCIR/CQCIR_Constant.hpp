//
// FILE NAME: CQCIR_Constant.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/29/2003
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
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//  Toolkit constants namespace
// ---------------------------------------------------------------------------
namespace kCQCIR
{
    // -----------------------------------------------------------------------
    //  The IR server side drivers (and the generic client side) have a need
    //  to talk outside of the field interface. So they use the 'backdoor'
    //  interfaces also provided by the CQCServer admin interface. These are
    //  ids and data names used in those calls to do the various things that
    //  these drivers need to do.
    // -----------------------------------------------------------------------


    // -----------------------------------------------------------------------
    //  These are used with the bQueryData() method to get binary data. The
    //  string could be used to indicate some specialization of the data to
    //  get but so far we don't need it.
    // -----------------------------------------------------------------------
    const TString strQueryBTrainingData(L"QBTrainingData");
    const TString strQueryMappedEvents(L"QMappedEvs");
    const TString strQueryBlasterCfg(L"QBlasterCfg");
    const TString strCvtManualBlastData(L"CvtManualBData");


    // -----------------------------------------------------------------------
    //  These are used with bQueryData2
    // -----------------------------------------------------------------------
    const TString strFmtBlastData(L"FormatBlastData");


    // -----------------------------------------------------------------------
    //  These are used with bQueryTextVal
    // -----------------------------------------------------------------------
    const TString strCheckKeyUsed(L"CheckKeyUsed");
    const TString strQueryRTrainingData(L"QueryRTData");
    const TString strQueryDevCmdList(L"QDevCmdList");
    const TString strQueryFmtBlastData(L"QFmtBData");


    // -----------------------------------------------------------------------
    //  These are used with bQueryVal.
    // -----------------------------------------------------------------------
    const TString strQueryBTrainMode(L"QueryBTMode");
    const TString strQueryRTrainMode(L"QueryRTMode");


    // -----------------------------------------------------------------------
    //  These are used with the bSendData() method to send IR data. We just
    //  format any parameter data into the buffer.
    // -----------------------------------------------------------------------
    const TString strInvokeBlastCmd(L"InvBCmd");
    const TString strSendCmd(L"SendCmd");
    const TString strSendData(L"SendData");
    const TString strStoreBlasterCfg(L"StoreBCfg");
    const TString strStoreRecCfg(L"StoreRCfg");
    const TString strStoreTrainData(L"StrTrainData");


    // -----------------------------------------------------------------------
    //  These are used with SendCmd to send simple commands. The command
    //  string, if needed, provides any extra info.
    // -----------------------------------------------------------------------
    const TString strClearBTraining(L"ClearBTFlag");
    const TString strClearRTraining(L"ClearRTFlag");
    const TString strDelAllEvents(L"DelAllEvents");
    const TString strDeleteEvent(L"DelEvent");
    const TString strEnterBTraining(L"EnterBTMode");
    const TString strEnterRTraining(L"EnterRTMode");
    const TString strExitBTraining(L"ExitBTMode");
    const TString strExitRTraining(L"ExitRTMode");
    const TString strLoadModel(L"LoadModel");
    const TString strResetConn(L"ResetConn");
    const TString strUnloadModel(L"UnloadModel");
}

#pragma CIDLIB_POPPACK


