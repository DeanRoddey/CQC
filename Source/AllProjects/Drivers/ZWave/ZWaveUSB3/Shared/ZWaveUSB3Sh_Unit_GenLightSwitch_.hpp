//
// FILE NAME: ZWaveUSB3Sh_Unit_GenLightSwitch_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/19/2018
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
//  This is a generic handler for one or two way light switches. The user can tell
//  us if it should use basic or binary switch to manage the off/on state.
//
//  We take an extra info value of:
//
//      IOType=Write | ReadWrite
//
//  And that controls whether we set the fields to read/write or write only. That's
//  the only difference between one and two way generic dimmers for our purposes. This
//  way we can have separate device info files in the list for generic one and two way
//  switches.
//
//  The user will have to insure appropriate associations are set for us to get the
//  notifications if two-way.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once

#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TGenLightSwitchUnit
//  PREFIX: unit
// ---------------------------------------------------------------------------
class TGenLightSwitchUnit : public TZWaveUnit
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TGenLightSwitchUnit() = delete;

        TGenLightSwitchUnit
        (
                    TZWUnitInfo* const      punitiOwner
        );

        TGenLightSwitchUnit(const TGenLightSwitchUnit&) = delete;

        ~TGenLightSwitchUnit();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TGenLightSwitchUnit& operator=(const TGenLightSwitchUnit&) = delete;


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

        tCIDLib::TVoid QueryUnitAttrs
        (
                    tCIDMData::TAttrList&   colToFill
            ,       TAttrData&              attrTmp
        )   const override;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_eAccess
        //      Driven by extra info, used in field creation and setting access
        //      flags on the CCs.
        //
        //  m_eSwitchClass
        //      The user can tell us which classes to use for off/on state. We
        //      basic and binary switch.
        //
        //  m_pzwcciXXX
        //      We keep a typed pointer to our switch impl so that we can interact
        //      with it, but we don't own it. We store it via the base binary impl
        //      class, which all our possible impls derive from.
        //
        //  m_strSwFldName
        //      We keep our built switch field name for use in logging or event
        //      triggers.
        // -------------------------------------------------------------------
        tZWaveUSB3Sh::EUnitAcc  m_eAccess;
        tZWaveUSB3Sh::ECClasses m_eSwitchClass;
        TZWCCImplBin*           m_pzwcciSwitch;
        TString                 m_strSwFldName;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TGenLightSwitchUnit, TZWaveUnit)
};

#pragma CIDLIB_POPPACK
