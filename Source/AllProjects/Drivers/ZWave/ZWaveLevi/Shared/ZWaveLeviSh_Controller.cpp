//
// FILE NAME: ZWaveLeviSh_Controller.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/14/2012
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
//  This file provides the implementation for the controller oriented
//  unit types.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "ZWaveLeviSh_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
AdvRTTIDecls(TZWController,TZWaveUnit)


// ---------------------------------------------------------------------------
//  Local types and constants
// ---------------------------------------------------------------------------
namespace ZWaveLeviSh_Controller
{
    // -----------------------------------------------------------------------
    //  Persistent format versions
    // -----------------------------------------------------------------------
    const tCIDLib::TCard2   c2FmtControllerVer  = 1;
}



// ---------------------------------------------------------------------------
//   CLASS: TZWController
//  PREFIX: unit
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWController: Constructors and Destructor
// ---------------------------------------------------------------------------
TZWController::TZWController() :

    TZWaveUnit
    (
        GENERIC_TYPE_GENERIC_CONTROLLER
        , 0
        , L"GenCtrl"
        , L"Z-Wave Controller"
        , tZWaveLeviSh::EUnitCaps
          (
            tZWaveLeviSh::EUnitCap_AsyncNotify
          )
        , tZWaveLeviSh::EUnitCaps
          (
            tZWaveLeviSh::EUnitCap_AsyncNotify
            | tZWaveLeviSh::EUnitCap_Readable
            | tZWaveLeviSh::EUnitCap_Writeable
          )
    )
    , m_c4LastScene(0)
{
    // Override the default opt info to zero
    c4OptInfo(0);
}

TZWController::TZWController(const tCIDLib::TCard4 c4UnitId) :

    TZWaveUnit
    (
        GENERIC_TYPE_GENERIC_CONTROLLER
        , 0
        , L"GenCtrl"
        , L"Z-Wave Controller"
        , tZWaveLeviSh::EUnitCaps
          (
            tZWaveLeviSh::EUnitCap_AsyncNotify
          )
        , tZWaveLeviSh::EUnitCaps
          (
            tZWaveLeviSh::EUnitCap_AsyncNotify
            | tZWaveLeviSh::EUnitCap_Readable
            | tZWaveLeviSh::EUnitCap_Writeable
          )
        , c4UnitId
    )
    , m_c4LastScene(0)
{
    // Override the default opt info to zero
    c4OptInfo(0);
}

TZWController::TZWController(const TZWController& unitSrc) :

    TZWaveUnit(unitSrc)
    , m_c4LastScene(0)
{
}

TZWController::~TZWController()
{
}


// ---------------------------------------------------------------------------
//  TZWController: Public operators
// ---------------------------------------------------------------------------
TZWController& TZWController::operator=(const TZWController& unitSrc)
{
    if (this != &unitSrc)
    {
        TParent::operator=(unitSrc);
    }
    return *this;
}


// We don't have any of our own persisted members, so just call the parent for now
tCIDLib::TBoolean TZWController::operator==(const TZWController& unitSrc) const
{
    return TParent::operator==(unitSrc);
}


tCIDLib::TBoolean TZWController::operator!=(const TZWController& unitSrc) const
{
    return !operator==(unitSrc);
}


// ---------------------------------------------------------------------------
//  TZWController: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TZWController::bOwnsFld(const tCIDLib::TCard4 c4FldId) const
{
    return kCIDLib::False;
}


tCIDLib::TBoolean
TZWController::bStoreFldIds( const  TVector<TCQCFldDef>&    colFlds
                            , const tZWaveLeviSh::TValList& fcolIds)
{
    return kCIDLib::True;
}


//
//  Handles non-basic type reports.
//
tCIDLib::TBoolean
TZWController::bStoreGenReport( const   tCIDLib::TCard4 ac4Values[]
                                , const tCIDLib::TCard4 c4ValCnt
                                ,       TCQCServerBase& sdrvLevi)
{
    // Shouldn't get anything less than 2, but don't tempt fate
    if (c4ValCnt < 2)
        return kCIDLib::False;

    tCIDLib::TBoolean bRes = kCIDLib::True;
    switch (ac4Values[0])
    {
        case COMMAND_CLASS_SCENE_ACTIVATION :
        {
            //
            //  If it's a scene activation, and has the right number of bytes for
            //  such, and it's for a non-zero scene, send out a trigger.
            //
            if ((ac4Values[1] == SCENE_ACTIVATION_SET)
            &&  (c4ValCnt == 4)
            &&  ac4Values[2])
            {
                CheckSendTrigger(ac4Values[2], sdrvLevi);
            }
            break;
        }

        case COMMAND_CLASS_SCENE_ACTUATOR_CONF :
        {
            //
            //  If it's a scene actuator report or get, and has the right number of
            //  bytes, and it's for a non-zero scene number, send out a trigger.
            //
            if ((((ac4Values[1] == SCENE_ACTUATOR_CONF_REPORT) && (c4ValCnt == 5))
            ||  ((ac4Values[1] == SCENE_ACTUATOR_CONF_GET) && (c4ValCnt == 3)))
            &&  ac4Values[2])
            {
                CheckSendTrigger(ac4Values[2], sdrvLevi);
            }
            break;
        }

        default :
            // Nothing we specifically handle, so pass to the parent
            bRes = TParent::bStoreGenReport(ac4Values, c4ValCnt, sdrvLevi);
            break;
    };
    return bRes;
}


// Return our optional info strings
tCIDLib::TBoolean
TZWController::bWantsOptInfo(tCIDLib::TStrCollect& colToFill) const
{
    colToFill.objAdd(facZWaveLeviSh().strMsg(kZWShMsgs::midOpt_Gen_None));
    colToFill.objAdd(facZWaveLeviSh().strMsg(kZWShMsgs::midOpt_Ctrl_IsVRCOP));
    colToFill.objAdd(facZWaveLeviSh().strMsg(kZWShMsgs::midOpt_Ctrl_IsSceneCtrl));

    return kCIDLib::True;
}



tCIDLib::TCard4
TZWController::c4QueryFldDefs(TVector<TCQCFldDef>&   colAddTo
                            , TCQCFldDef&           flddTmp) const
{
    return 0;
}


// Call our parent and zero out our field id
tCIDLib::TVoid TZWController::PrepForUse()
{
    TParent::PrepForUse();

    // Initialize the last scene
    m_c4LastScene = 0;
}


// Set all of our unit status oriented fields to error state
tCIDLib::TVoid TZWController::SetFieldErrs(TCQCServerBase& sdrvLevi)
{
}


// ---------------------------------------------------------------------------
//  TZWController: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Returns true if this guy is marked as the VRCOP controller
tCIDLib::TBoolean TZWController::bIsVRCOP() const
{
    return (c4OptInfo() == 1);
}



// ---------------------------------------------------------------------------
//  TZWController: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  If the controller is a scene controller, then this is called if get a report that
//  a scene was activated. If it's not the same as the previous scene we got, then
//  we store it as the new last scene and send out a user action trigger.
//
tCIDLib::TVoid
TZWController::CheckSendTrigger(const   tCIDLib::TCard4     c4SceneNum
                                ,       TCQCServerBase&     sdrvLevi)
{
    // If not marked as a scene ctrl, nothing to do
    if (c4OptInfo() != 2)
        return;

    // We are a scene ctrl, so send if not alread the active scene
    if (c4SceneNum != m_c4LastScene)
    {
        // Remember the last scene we saw
        m_c4LastScene = c4SceneNum;

        // And send the notification
        TString strSceneId;
        strSceneId.AppendFormatted(m_c4LastScene);
        sdrvLevi.QueueEventTrig
        (
            tCQCKit::EStdDrvEvs::UserAction
            , strName()
            , strSceneId
            , TString::strEmpty()
            , TString::strEmpty()
        );
    }
}

