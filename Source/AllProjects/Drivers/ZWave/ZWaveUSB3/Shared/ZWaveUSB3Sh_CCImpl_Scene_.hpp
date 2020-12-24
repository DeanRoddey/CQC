//
// FILE NAME: ZWaveUSB3Sh_CCImpl_Scene_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/22/2018
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
//  This file implements CC impls for scene activation. In the context of this driver
//  that means using scene activation devices to invoke user action triggers. We
//  support the 'scene activation' CC here.
//
// CAVEATS/GOTCHAS:
//
//  1)  There is no field version of this one, since it just exists to invoke user
//      action triggers.
//
//  2)  Unlike most impls, this guy just directly sends out event triggers, since
//      that's all it exists to do. Most will call to the base impl class to report
//      a value changes, and the unit handler will send triggers if appropriate.
//      But this one has no value really.
//
// LOG:
//
#pragma once

#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TZWCCImplSceneActState
//  PREFIX: zwcci
// ---------------------------------------------------------------------------
class TZWCCImplSceneActState : public TZWCCImpl
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TZWCCImplSceneActState
        (
                    TZWUnitInfo* const      punitiThis
            , const tCIDLib::TBoolean       bOnChangeOnly
            , const tCIDLib::TCard1         c1EPId = 0xFF
        );

        TZWCCImplSceneActState(const TZWCCImplSceneActState&) = delete;

        ~TZWCCImplSceneActState();


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tZWaveUSB3Sh::ECCMsgRes eHandleCCMsg
        (
            const   tCIDLib::TCard1         c1Class
            , const tCIDLib::TCard1         c1Cmd
            , const TZWInMsg&               zwimToHandle
        )   override;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bOnChangeOnly
        //      We can be told to only send another trigger if the scene changes, or
        //      on any incoming scene message. We set this based on that option
        //      during bPrepare() and keep around for later use.
        //
        //  m_c4LastScene
        //      We remember the last scene we saw. See m_bOnChangeOnly above.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bOnChangeOnly;
        tCIDLib::TCard4     m_c4LastScene;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWCCImplSceneActState, TZWCCImpl)
};


#pragma CIDLIB_POPPACK
