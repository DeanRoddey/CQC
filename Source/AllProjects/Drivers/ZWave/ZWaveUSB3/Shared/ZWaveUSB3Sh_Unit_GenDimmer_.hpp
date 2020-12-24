//
// FILE NAME: ZWaveUSB3Sh_Unit_GenDimmer_.hpp
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
//  This is a generic handler for one or two way dimmers. The user can tell us if it
//  should use basic or multi-level switch to manage the level and basic or bin switch
//  for the off/on control.
//
//  We take an extra info value of:
//
//      IOType=Write | ReadWrite
//
//  And that controls whether we set the fields to read/write or write only. That's
//  the only difference between one and two way generic dimmers for our purposes. This
//  way we can have separate device info files in the list for one and two way dimmers.
//
//  The user will have to insure appropriate associations are set for us to get the
//  notifications if two/way.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once

#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TGenDimmerUnit
//  PREFIX: unit
// ---------------------------------------------------------------------------
class TGenDimmerUnit : public TZWaveUnit
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TGenDimmerUnit() = delete;

        TGenDimmerUnit
        (
                    TZWUnitInfo* const      punitiOwner
        );

        TGenDimmerUnit(const TGenDimmerUnit&) = delete;

        ~TGenDimmerUnit();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TGenDimmerUnit& operator=(const TGenDimmerUnit&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bParseExtraInfo() override;

        tCIDLib::TBoolean bPrepare() override;

        tCIDLib::TVoid ImplInErrState
        (
            const   tCIDLib::TCard4         c4ImplId
        )   override;

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
        //  m_eDimClass
        //  m_eSwitchClass
        //      The user can tell us which classes to use for dimming and off/on.
        //      It may be the same, it may be different. We support basic, binary
        //      switch, and multi-level switch.
        //
        //  m_pzwcciXXX
        //      We keep typed pointers to our switch and dimmer CC impls so that we
        //      can interact with them, but we don't own them. We store them via their
        //      respective bin/level base classes.
        //
        //  m_strDimFldName
        //  m_strSwFldName
        //      We keep our built dimmer and switch field names for use in logging
        //      or event trigers.
        // -------------------------------------------------------------------
        tZWaveUSB3Sh::EUnitAcc  m_eAccess;
        tZWaveUSB3Sh::ECClasses m_eDimClass;
        tZWaveUSB3Sh::ECClasses m_eSwitchClass;
        TZWCCImplBin*           m_pzwcciSwitch;
        TZWCCImplLevel*         m_pzwcciLevel;
        TString                 m_strDimFldName;
        TString                 m_strSwFldName;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TGenDimmerUnit, TZWaveUnit)
};

#pragma CIDLIB_POPPACK
