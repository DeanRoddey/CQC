//
// FILE NAME: ZWaveUSB3Sh_Unit_LightSwitch_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/24/2018
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
//  This is a handler for simple light switches. We have a write only or a read/write
//  field that reflects (or controls) the state of the switch. The extra info in
//  the device info file tells us whether its write only or read/write, and it indicates
//  the CC to use.
//
//      IOType=[Write | ReadWrite]
//      SwitchCC=[Basic | BinSwitch]
//
//  We can optionally accept async reports of status changes via another CC, currently
//  either notification or the switch CC not used above for control and query.
//
//      AsyncNot=[Notification | Basic | BinSwitch]
//
//  If notify, the CC's extra info sets the notification ids.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once

#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TLightSwitchUnit
//  PREFIX: unit
// ---------------------------------------------------------------------------
class TLightSwitchUnit : public TZWaveUnit
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TLightSwitchUnit() = delete;

        TLightSwitchUnit
        (
                    TZWUnitInfo* const      punitiOwner
        );

        TLightSwitchUnit(const TLightSwitchUnit&) = delete;

        ~TLightSwitchUnit();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TLightSwitchUnit& operator=(const TLightSwitchUnit&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bParseExtraInfo() override;

        tCIDLib::TBoolean bPrepare() override;

        tCIDLib::TVoid ImplValueChanged
        (
            const   tCIDLib::TCard4         c4ImplId
            , const tZWaveUSB3Sh::EValSrcs  eSource
            , const tCIDLib::TBoolean       bWasInErr
            , const tCIDLib::TInt4          i4ExtraInfo
        )   override;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_eAccess
        //      Driven by extra info, used in field creation and setting access
        //      flags on the CCs.
        //
        //  m_eNotifyClass
        //      Can be _Count if not used, else the CC to get notifications on. We will
        //      push changes from it to the switch CC.
        //
        //  m_eSwitchClass
        //      The user can tell us which class to to interact with the switch by.
        //
        //  m_pzwcciNotify
        //      Optional async notification CC if we don't get them via the switch CC.
        //      We don't own it, it's set on the base class, we just keep a typed pointer.
        //
        //  m_pzwcciSwitch
        //      We keep a typed pointer to our switch CC impl so that we can interact
        //      with it, but we don't own it. We store it via the bin impl base class.
        //
        //  m_strSwFldName
        //      We keep our built switch field name for use in logging or event trigers.
        // -------------------------------------------------------------------
        tZWaveUSB3Sh::EUnitAcc  m_eAccess;
        tZWaveUSB3Sh::ECClasses m_eNotifyClass;
        tZWaveUSB3Sh::ECClasses m_eSwitchClass;
        TZWCCImplBin*           m_pzwcciNotify;
        TZWCCImplBin*           m_pzwcciSwitch;
        TString                 m_strSwFldName;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TLightSwitchUnit, TZWaveUnit)
};

#pragma CIDLIB_POPPACK
