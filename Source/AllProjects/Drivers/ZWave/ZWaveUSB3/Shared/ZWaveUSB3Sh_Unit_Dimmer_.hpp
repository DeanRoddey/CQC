//
// FILE NAME: ZWaveUSB3Sh_Unit_Dimmer_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/02/2018
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
//  This is a handler for one or two way dimmers. Extra info in the dev info file
//  tells us if it is read or read/write and what classes to use for dimming and
//  off/on.
//
//      IOType=Write | ReadWrite
//      SwitchCC=[Basic | BinSwitch | MLSwitch]
//      DimCC=[Basic | MLSwitch]
//
//  This is similar to the generic dimmer but driven by device info instead of user
//  configuration.
//
//
//  Since this same stuff is often useful for some other things that have off/on and
//  level functionality, we have an optional value that will affect the field name.
//  Normally, for a non-V2 standard dimmer, the names will be [unit]_Switch and
//  [unit]_Dimmer. We can force that with:
//
//      SwitchName=xxx
//      LevelName=xxxx
//
//  These being defined will force it not to be a V2 dimmer as well of course. This
//  way we avoid doing almost exactly this same stuff for another unit handler types.
//
// CAVEATS/GOTCHAS:
//
//  1)  We assume here that are dealing with wall powered units, since all dimmer/
//      switches are basically.
//
// LOG:
//
#pragma once

#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TDimmerUnit
//  PREFIX: unit
// ---------------------------------------------------------------------------
class TDimmerUnit : public TZWaveUnit
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TDimmerUnit() = delete;

        TDimmerUnit
        (
                    TZWUnitInfo* const      punitiOwner
        );

        TDimmerUnit(const TDimmerUnit&) = delete;

        ~TDimmerUnit();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TDimmerUnit& operator=(const TDimmerUnit&) = delete;


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
        //  m_eDimClass
        //  m_eSwitchClass
        //      The classes we use for switching and dimming. These are driven by
        //      extra info.
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
        RTTIDefs(TDimmerUnit, TZWaveUnit)
};

#pragma CIDLIB_POPPACK
