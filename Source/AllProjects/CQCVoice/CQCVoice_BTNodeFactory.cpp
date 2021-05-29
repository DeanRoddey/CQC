//
// FILE NAME: CQCVoice_BTNodeFatcory.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/07/2017
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
//  This file implements our behavior tree node factory
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $_CIDLib_Log_$
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCVoice.hpp"

#include    "CQCVoice_BTNodeFactory.hpp"

#include    "CQCVoice_BTSimpleCmdNodes.hpp"
#include    "CQCVoice_BTLightingNodes.hpp"
#include    "CQCVoice_BTHVACNodes.hpp"
#include    "CQCVoice_BTMediaNodes.hpp"
#include    "CQCVoice_BTReminderNodes.hpp"
#include    "CQCVoice_BTSecurityNodes.hpp"
#include    "CQCVoice_BTSimpleQNodes.hpp"
#include    "CQCVoice_BTWeatherQNodes.hpp"



// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCVoiceBTNodeFact, TAIBTNodeFact)



// ---------------------------------------------------------------------------
//  Local types and constants
// ---------------------------------------------------------------------------
namespace
{
    namespace CQCVoice_BTNodeFactory
    {
        // -----------------------------------------------------------------------
        //  The names of the node types we support
        // -----------------------------------------------------------------------
        const TString   strType_CmdArmSecSystem(L"CmdArmSecSystem");
        const TString   strType_CmdBringUpLights(L"CmdBringUpLights");
        const TString   strType_CmdCancelAllReminders(L"CmdCancelAllReminders");
        const TString   strType_CmdCancelLastReminder(L"CmdCancelLastReminder");
        const TString   strType_CmdClearMusicPL(L"CmdClearMusicPL");
        const TString   strType_CmdDisarmSecSystem(L"CmdDisarmSecSystem");
        const TString   strType_CmdKillLights(L"CmdKillLights");
        const TString   strType_CmdMusicTransport(L"CmdMusicTransport");
        const TString   strType_CmdNextMusicItem(L"CmdNextMusicItem");
        const TString   strType_CmdPlayMusicFromCat(L"CmdPlayMusicFromCat");
        const TString   strType_CmdPlayMusicPL(L"CmdPlayMusicPL");
        const TString   strType_CmdReloadCfg(L"CmdReloadCfg");
        const TString   strType_CmdRemindMe(L"CmdRemindMe");
        const TString   strType_CmdSetItToNum(L"CmdSetItToNum");
        const TString   strType_CmdSetLightLevel(L"CmdSetLightLevel");
        const TString   strType_CmdSetLightSwitch(L"CmdSetLightSwitch");
        const TString   strType_CmdSetMusicMute(L"CmdSetMusicMute");
        const TString   strType_CmdSetMusicPLMode(L"CmdSetMusicPLMode");
        const TString   strType_CmdSetMusicVolume(L"CmdSetMusicVolume");
        const TString   strType_CmdSetRoomMode(L"CmdSetRoomMode");
        const TString   strType_CmdSetThermoHLSP(L"CmdSetThermoHLSP");
        const TString   strType_CmdTurnItOffOn(L"CmdTurnItOffOn");

        const TString   strType_QueryCQCVer(L"QueryCQCVer");
        const TString   strType_QueryCurrentTemp(L"QueryCurrentTemp");
        const TString   strType_QueryCurrentTime(L"QueryCurrentTime");
        const TString   strType_QueryLightSwitch(L"QueryLightSwitch");
        const TString   strType_QueryLightSwitchState(L"QueryLightSwitchState");
        const TString   strType_QuerySecZoneState(L"QuerySecZoneState");
        const TString   strType_QueryThermoHLSP(L"QueryThermoHLSP");
        const TString   strType_QueryTodaysDate(L"QueryTodaysDate");
        const TString   strType_QueryWeatherShortFC(L"QueryWeatherShortFC");

        const TString   strType_Start(L"Start");
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCVoiceBTNodeFact
//  PREFIX: nfact
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCVoiceBTNodeFact: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCVoiceBTNodeFact::TCQCVoiceBTNodeFact() :

    TAIBTNodeFact(kCQCVoice::strFact_Name)
{
}

TCQCVoiceBTNodeFact::~TCQCVoiceBTNodeFact()
{
}


// ---------------------------------------------------------------------------
//  TCQCVoiceBTNodeFact: Protected, inherited methods
// ---------------------------------------------------------------------------
TAIBTNode*
TCQCVoiceBTNodeFact::pbtnodeNew(  const   TString&            strPath
                                , const TString&            strName
                                , const TString&            strType)
{
    // Command nodes
    if (strType == CQCVoice_BTNodeFactory::strType_CmdArmSecSystem)
        return new TBTCmdArmSecSystemNode(strPath, strName);

    else if (strType == CQCVoice_BTNodeFactory::strType_CmdCancelAllReminders)
        return new TBTCmdCancelAllReminders(strPath, strName);
    else if (strType == CQCVoice_BTNodeFactory::strType_CmdBringUpLights)
        return new TBTCmdBringUpLightsNode(strPath, strName);
    else if (strType == CQCVoice_BTNodeFactory::strType_CmdCancelLastReminder)
        return new TBTCmdCancelLastReminder(strPath, strName);
    else if (strType == CQCVoice_BTNodeFactory::strType_CmdClearMusicPL)
        return new TBTCmdClearMusicPLNode(strPath, strName);
    else if (strType == CQCVoice_BTNodeFactory::strType_CmdDisarmSecSystem)
        return new TBTCmdDisarmSecSystemNode(strPath, strName);
    else if (strType == CQCVoice_BTNodeFactory::strType_CmdKillLights)
        return new TBTCmdKillLightsNode(strPath, strName);
    else if (strType == CQCVoice_BTNodeFactory::strType_CmdMusicTransport)
        return new TBTCmdMusicTransportNode(strPath, strName);
    else if (strType == CQCVoice_BTNodeFactory::strType_CmdPlayMusicFromCat)
        return new TBTCmdPlayMusicFromCatNode(strPath, strName);
    else if (strType == CQCVoice_BTNodeFactory::strType_CmdNextMusicItem)
        return new TBTCmdNextMusicItemNode(strPath, strName);
    else if (strType == CQCVoice_BTNodeFactory::strType_CmdPlayMusicPL)
        return new TBTCmdPlayMusicPLNode(strPath, strName);
    else if (strType == CQCVoice_BTNodeFactory::strType_CmdReloadCfg)
        return new TBTCmdReloadCfgNode(strPath, strName);
    else if (strType == CQCVoice_BTNodeFactory::strType_CmdRemindMe)
        return new TBTCmdRemindMe(strPath, strName);
    else if (strType == CQCVoice_BTNodeFactory::strType_CmdSetItToNum)
        return new TBTCmdSetItToNumNode(strPath, strName);
    else if (strType == CQCVoice_BTNodeFactory::strType_CmdSetLightLevel)
        return new TBTCmdSetLightLevelNode(strPath, strName);
    else if (strType == CQCVoice_BTNodeFactory::strType_CmdSetLightSwitch)
        return new TBTCmdSetLightSwitchNode(strPath, strName);
    else if (strType == CQCVoice_BTNodeFactory::strType_CmdSetMusicMute)
        return new TBTCmdSetMusicMuteNode(strPath, strName);
    else if (strType == CQCVoice_BTNodeFactory::strType_CmdSetMusicPLMode)
        return new TBTCmdSetMusicPLModeNode(strPath, strName);
    else if (strType == CQCVoice_BTNodeFactory::strType_CmdSetMusicVolume)
        return new TBTCmdSetMusicVolumeNode(strPath, strName);
    else if (strType == CQCVoice_BTNodeFactory::strType_CmdSetRoomMode)
        return new TBTCmdSetRoomModeNode(strPath, strName);
    else if (strType == CQCVoice_BTNodeFactory::strType_CmdSetThermoHLSP)
        return new TBTCmdSetThermoHLSPNode(strPath, strName);
    else if (strType == CQCVoice_BTNodeFactory::strType_CmdTurnItOffOn)
        return new TBTCmdTurnItOffOnNode(strPath, strName);

    // Query nodes
    else if (strType == CQCVoice_BTNodeFactory::strType_QueryCQCVer)
        return new TBTQuCQCVersionNode(strPath, strName);
    else if (strType == CQCVoice_BTNodeFactory::strType_QueryCurrentTemp)
        return new TBTQuCurrentTempNode(strPath, strName);
    else if (strType == CQCVoice_BTNodeFactory::strType_QueryCurrentTime)
        return new TBTQuCurrentTimeNode(strPath, strName);
    else if (strType == CQCVoice_BTNodeFactory::strType_QueryLightSwitch)
        return new TBTQuLightSwitchNode(strPath, strName);
    else if (strType == CQCVoice_BTNodeFactory::strType_QueryLightSwitchState)
        return new TBTQuLightSwitchStateNode(strPath, strName);
    else if (strType == CQCVoice_BTNodeFactory::strType_QuerySecZoneState)
        return new TBTQuSecZoneStateNode(strPath, strName);
    else if (strType == CQCVoice_BTNodeFactory::strType_QueryThermoHLSP)
        return new TBTQuThermoHLSPNode(strPath, strName);
    else if (strType == CQCVoice_BTNodeFactory::strType_QueryTodaysDate)
        return new TBTQuTodaysDateNode(strPath, strName);
    else if (strType == CQCVoice_BTNodeFactory::strType_QueryWeatherShortFC)
        return new TBTQuWeatherShortFCNode(strPath, strName);

    // Special case nodes
    else if (strType == CQCVoice_BTNodeFactory::strType_Start)
        return new TBTStartNode(strPath, strName);

    return nullptr;
}
