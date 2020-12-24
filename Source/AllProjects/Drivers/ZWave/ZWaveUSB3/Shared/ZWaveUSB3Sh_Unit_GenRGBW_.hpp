//
// FILE NAME: ZWaveUSB3Sh_Unit_GenRGBW_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/16/2017
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
//  In theory this class should handle most fairly standard RGB(W) controllers.
//  It uses the color light CC for HSV color control and to support setting
//  individual color components. It can support other CCs for off/on control and
//  dimming if the unit allows that.
//
//  Unit level extra info is used to parameterize this one. The possible values
//  are:
//
//      WhiteComp=[compid]
//      OffOn=[Basic | BinSwitch | MLSwitch]
//      Dim=[Basic | MLSwitch]
//
//  If it supports white in addition to RGB, the WhiteComp value tells it what color
//  component to use for white. If not present, it only does RGB.
//
//  If it supports Off/On it can be done via Basic, BinSwitch or MLSwitch. If it
//  supports dimming separate from the colors, it can support that via Basic or
//  MLSwitch.
//
// CAVEATS/GOTCHAS:
//
//
// LOG:
//
#pragma once

#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TGenRGBWUnit
//  PREFIX: unit
// ---------------------------------------------------------------------------
class TGenRGBWUnit : public TZWaveUnit
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TGenRGBWUnit() = delete;

        TGenRGBWUnit
        (
                TZWUnitInfo* const      punitiOwner
        );

        TGenRGBWUnit(const TGenRGBWUnit&) = delete;

        ~TGenRGBWUnit();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TGenRGBWUnit& operator=(const TGenRGBWUnit&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
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
        //  m_pzwcciColor
        //  m_pzwcciLevel
        //  m_pzwcciSwitch
        //      Our CCs that we create, some stored as their respective bin/level
        //      base classes. We don't own them, we just keep typed pointers so that
        //      we can interact with them.
        //
        //  m_strSwFldName
        //      Our light switch field name which we keep around for use in logging
        //      or event triggers.
        // -------------------------------------------------------------------
        TZWCCImplCLghtRGBF* m_pzwcciColor;
        TZWCCImplLevel*     m_pzwcciLevel;
        TZWCCImplBin*       m_pzwcciSwitch;
        TString             m_strSwFldName;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TGenRGBWUnit, TZWaveUnit)
};

#pragma CIDLIB_POPPACK

