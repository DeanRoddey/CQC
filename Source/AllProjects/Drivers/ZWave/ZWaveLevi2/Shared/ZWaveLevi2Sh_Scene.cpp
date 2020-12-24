//
// FILE NAME: ZWaveLevi2Sh_Scene.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/01/2014
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
//  This file provides the implementation for the scene related command classes.
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
RTTIDecls(TZWCCSceneActConf, TZWCmdClass)
RTTIDecls(TZWCCSceneActivation, TZWCmdClass)




// ---------------------------------------------------------------------------
//   CLASS: TZWCCSceneActConf
//  PREFIX: zwcc
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWCCSceneActConf: Constructors Destructor
// ---------------------------------------------------------------------------
TZWCCSceneActConf::TZWCCSceneActConf(const TZWDevClass& zwdcInst) :

    TZWCmdClass(tZWaveLevi2Sh::ECClasses::SceneActuatorConf, zwdcInst, COMMAND_CLASS_SCENE_ACTUATOR_CONF)
    , m_c4LastScene(0)
{
    //
    //  We are never readable or writeable, so for efficiency make sure that's marked
    //  as such. The passed device info might have said otherwise, but if so it is
    //  in error. We depend purely on incoming reports.
    //
    SetAccess(kCIDLib::False, kCIDLib::False);
}

TZWCCSceneActConf::~TZWCCSceneActConf()
{
}



// ---------------------------------------------------------------------------
//  TZWCCSceneActConf: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  If the device's generic class isn't a binary sensor, then we should get our value
//  via the generic report.
//
tCIDLib::TBoolean
TZWCCSceneActConf::bHandleGenReport(const   tCIDLib::TCardList& fcolVals
                                    , const TZWaveUnit&         unitMe
                                    ,       MZWLeviSrvFuncs&    mzwsfLevi
                                    , const tCIDLib::TCard4     c4FromInst)
{
    // Shouldn't get anything less than 5, but don't tempt fate
    const tCIDLib::TCard4 c4ValCnt = fcolVals.c4ElemCount();
    if ((c4FromInst != c4InstId()) || (c4ValCnt < 2))
        return kCIDLib::False;

    tCIDLib::TBoolean bRes = kCIDLib::True;
    if (fcolVals[0] == COMMAND_CLASS_SCENE_ACTUATOR_CONF)
    {
        //
        //  If it's a scene actuator report or get, and has the right number of
        //  bytes, and it's for a non-zero scene number, send out a trigger.
        //
        if ((((fcolVals[1] == SCENE_ACTUATOR_CONF_REPORT) && (c4ValCnt == 5))
        ||  ((fcolVals[1] == SCENE_ACTUATOR_CONF_GET) && (c4ValCnt == 3)))
        &&  fcolVals[2])
        {
            if (fcolVals[2] != m_c4LastScene)
            {
                // Remember the last scene we saw
                m_c4LastScene = fcolVals[2];

                // And send the notification
                TString strSceneId;
                strSceneId.AppendFormatted(m_c4LastScene);
                mzwsfLevi.LeviQEventTrig
                (
                    tCQCKit::EStdDrvEvs::UserAction
                    , TString::strEmpty()
                    , unitMe.strName()
                    , strSceneId
                    , TString::strEmpty()
                    , TString::strEmpty()
                );
            }
            bRes = kCIDLib::True;
        }
    }
    return bRes;
}


//
//  We don't have any fields, but make sure our last scene is zeroed, so that the
//  first incoming real scene id will trigger an event.
//
tCIDLib::TVoid
TZWCCSceneActConf::InitVals(const TZWaveUnit& unitMe,  MZWLeviSrvFuncs& mzwsfLevi)
{
    m_c4LastScene = 0;
}



// ---------------------------------------------------------------------------
//   CLASS: TZWCCSceneActivation
//  PREFIX: zwcc
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWCCSceneActivation: Constructors Destructor
// ---------------------------------------------------------------------------
TZWCCSceneActivation::TZWCCSceneActivation(const TZWDevClass& zwdcInst) :

    TZWCmdClass(tZWaveLevi2Sh::ECClasses::SceneActivation, zwdcInst, COMMAND_CLASS_SCENE_ACTIVATION)
    , m_c4LastScene(0)
{
    //
    //  We are never readable or writeable, so for efficiency make sure that's marked
    //  as such. The passed device info might have said otherwise, but if so it is
    //  in error. We depend purely on incoming reports.
    //
    SetAccess(kCIDLib::False, kCIDLib::False);
}

TZWCCSceneActivation::~TZWCCSceneActivation()
{
}



// ---------------------------------------------------------------------------
//  TZWCCSceneActivation: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  If the device's generic class isn't a binary sensor, then we should get our value
//  via the generic report.
//
tCIDLib::TBoolean
TZWCCSceneActivation::bHandleGenReport( const   tCIDLib::TCardList& fcolVals
                                        , const TZWaveUnit&         unitMe
                                        ,       MZWLeviSrvFuncs&    mzwsfLevi
                                        , const tCIDLib::TCard4     c4FromInst)
{
    // Shouldn't get anything less than 5, but don't tempt fate
    const tCIDLib::TCard4 c4ValCnt = fcolVals.c4ElemCount();
    if ((c4FromInst != c4InstId()) || (c4ValCnt < 2))
        return kCIDLib::False;

    tCIDLib::TBoolean bRes = kCIDLib::True;
    if (fcolVals[0] == COMMAND_CLASS_SCENE_ACTIVATION)
    {
        if ((fcolVals[1] == SCENE_ACTIVATION_SET) && (c4ValCnt == 4) && fcolVals[2])
        {
            if (fcolVals[2] != m_c4LastScene)
            {
                // Remember the last scene we saw
                m_c4LastScene = fcolVals[2];

                // And send the notification
                TString strSceneId;
                strSceneId.AppendFormatted(m_c4LastScene);
                mzwsfLevi.LeviQEventTrig
                (
                    tCQCKit::EStdDrvEvs::UserAction
                    , TString::strEmpty()
                    , unitMe.strName()
                    , strSceneId
                    , TString::strEmpty()
                    , TString::strEmpty()
                );
            }
            bRes = kCIDLib::True;
        }
    }

    return bRes;
}


//
//  We don't have any fields, but make sure our last scene is zeroed, so that the
//  first incoming real scene id will trigger an event.
//
tCIDLib::TVoid
TZWCCSceneActivation::InitVals(const TZWaveUnit& unitMe,  MZWLeviSrvFuncs& mzwsfLevi)
{
    m_c4LastScene = 0;
}

