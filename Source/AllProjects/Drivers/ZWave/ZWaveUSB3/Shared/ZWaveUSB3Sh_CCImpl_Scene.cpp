//
// FILE NAME: ZWaveUSB3Sh_CCImpl_Notification.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/13/2018
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
//  This is the implementation classes for the Z-Wave notification command class.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "ZWaveUSB3Sh_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TZWCCImplSceneActState, TZWCCImpl)



// ---------------------------------------------------------------------------
//   CLASS: TZWCCImplSceneActState
//  PREFIX: zwcci
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWCCImplSceneActState: Constructors and Destructor
// ---------------------------------------------------------------------------
TZWCCImplSceneActState::
TZWCCImplSceneActState(         TZWUnitInfo* const  punitiThis
                        , const tCIDLib::TBoolean   bOnChangeOnly
                        , const tCIDLib::TCard1     c1EPId) :

    TZWCCImpl(punitiThis, tZWaveUSB3Sh::ECClasses::SceneAct, c1EPId)
    , m_bOnChangeOnly(bOnChangeOnly)
    , m_c4LastScene(0)
{
    //
    //  Make sure our access is always none, overriding the default set by the base,
    //  which may have set some read options.
    //
    SetAccess(tZWaveUSB3Sh::EUnitAcc::None, tZWaveUSB3Sh::EUnitAcc::AllBits);
}

TZWCCImplSceneActState::~TZWCCImplSceneActState()
{
}


// ---------------------------------------------------------------------------
//  TZWCCImplSceneActState: Protected, inherited methods
// ---------------------------------------------------------------------------

tZWaveUSB3Sh::ECCMsgRes
TZWCCImplSceneActState::eHandleCCMsg(const  tCIDLib::TCard1 c1Class
                                    , const tCIDLib::TCard1 c1Cmd
                                    , const TZWInMsg&       zwimIn)
{
    if ((c1Class == COMMAND_CLASS_SCENE_ACTIVATION)
    &&  (c1Cmd == SCENE_ACTIVATION_SET))
    {
        // Get the scene number out
        const tCIDLib::TCard4 c4SceneNum = zwimIn.c1CCByteAt(2);

        // Remember if it is different from the previous then store it
        const tCIDLib::TBoolean bChanged(c4SceneNum != m_c4LastScene);
        m_c4LastScene = c4SceneNum;

        //
        //  If it's different from the last scene, or we aren't doing on change only
        //  then let's send a trigger. We ignore scene zero which is the local load.
        //  But we do store, so any non-zero scene will subsequently trigger.
        //
        if ((bChanged || !m_bOnChangeOnly) && c4SceneNum)
        {
            TString strSceneId;
            strSceneId.AppendFormatted(m_c4LastScene);

            QueueEventTrig
            (
                tCQCKit::EStdDrvEvs::UserAction
                , TString::strEmpty()
                , strUnitName()
                , strSceneId
                , TString::strEmpty()
                , TString::strEmpty()
            );
        }
        return tZWaveUSB3Sh::ECCMsgRes_Value;
    }
    return tZWaveUSB3Sh::ECCMsgRes_Unhandled;
}
