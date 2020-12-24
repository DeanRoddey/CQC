//
// FILE NAME: ZWaveUSB3Sh_CCImpl_CentralScene.cpp
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
RTTIDecls(TZWCCImplCentralScene, TZWCCImpl)



// ---------------------------------------------------------------------------
//   CLASS: TZWCCImplCentralScene
//  PREFIX: zwcci
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWCCImplCentralScene: Constructors and Destructor
// ---------------------------------------------------------------------------
TZWCCImplCentralScene::
TZWCCImplCentralScene(          TZWUnitInfo* const  punitiThis
                        , const tCIDLib::TBoolean   bOnChangeOnly
                        , const tCIDLib::TCard1     c1EPId) :
    TZWCCImpl(punitiThis, tZWaveUSB3Sh::ECClasses::CentralScene, c1EPId)
    , m_bOnChangeOnly(bOnChangeOnly)
    , m_c4LastEvent(0xFF)
    , m_c4LastScene(0)
{
    //
    //  Set a default acccess of write only, whcih is almost always what we would
    //  want. We override any bits set by the base ctor in this case since it might
    //  set some read options on us.
    //
    SetAccess(tZWaveUSB3Sh::EUnitAcc::Write, tZWaveUSB3Sh::EUnitAcc::AllBits);
}

TZWCCImplCentralScene::~TZWCCImplCentralScene()
{
}


// ---------------------------------------------------------------------------
//  TZWCCImplCentralScene: Protected, inherited methods
// ---------------------------------------------------------------------------

tZWaveUSB3Sh::ECCMsgRes
TZWCCImplCentralScene::eHandleCCMsg(const   tCIDLib::TCard1 c1Class
                                    , const tCIDLib::TCard1 c1Cmd
                                    , const TZWInMsg&       zwimIn)
{
    if ((c1Class == COMMAND_CLASS_CENTRAL_SCENE) && (c1Cmd == CENTRAL_SCENE_NOTIFICATION))
    {
        // Get the scene number and event type out
        const tCIDLib::TCard4 c4Scene = zwimIn.c1CCByteAt(4);
        const tCIDLib::TCard4 c4Event = zwimIn.c1CCByteAt(3) & 0x7;

        const tCIDLib::TBoolean bChanged
        (
            (c4Scene != m_c4LastScene) || (c4Event != m_c4LastEvent)
        );

        // Send if needed. We ignore scene zero, which shouldn't be valid
        if ((bChanged || !m_bOnChangeOnly) && c4Scene)
        {
            // REmember these as the new last ones
            m_c4LastEvent = c4Event;
            m_c4LastScene = c4Scene;

            // Format out the scene info
            TString strSceneId;
            strSceneId.AppendFormatted(c4Scene);

            if (c4Event == 0)
                strSceneId.Append(L"_Press");
            else if (c4Event == 1)
                strSceneId.Append(L"_Release");
            else if (c4Event == 2)
                strSceneId.Append(L"_Hold");
            else if (c4Event == 3)
                strSceneId.Append(L"_Press2");
            else if (c4Event == 4)
                strSceneId.Append(L"_Press3");
            else if (c4Event == 5)
                strSceneId.Append(L"_Press4");
            else if (c4Event == 6)
                strSceneId.Append(L"_Press5");

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
