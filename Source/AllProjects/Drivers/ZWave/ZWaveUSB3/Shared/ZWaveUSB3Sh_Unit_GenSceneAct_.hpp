//
// FILE NAME: ZWaveUSB3Sh_Unit_GenSceneAct_.hpp
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
//  This is a generic handler for scene activation devices. In this case it's for
//  things that just do scene activation, like button fobs and such. Other devices
//  make also include this functionality.
//
//  We take an extra info value of:
//
//      ActClass=[SceneAct | CentralScene]
//
//  That tells us which CC to use to receive scene activation events. These must
//  handle our 'only on change' option.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once

#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TGenSceneActUnit
//  PREFIX: unit
// ---------------------------------------------------------------------------
class TGenSceneActUnit : public TZWaveUnit
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TGenSceneActUnit() = delete;

        TGenSceneActUnit
        (
                    TZWUnitInfo* const      punitiOwner
        );

        TGenSceneActUnit(const TGenSceneActUnit&) = delete;

        ~TGenSceneActUnit();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TGenSceneActUnit& operator=(const TGenSceneActUnit&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bParseExtraInfo() override;

        tCIDLib::TBoolean bPrepare() override;

        tCIDLib::TVoid QueryUnitAttrs
        (
                    tCIDMData::TAttrList&   colToFill
            ,       TAttrData&              attrTmp
        )   const override;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_eActClass
        //      We get told via an extra info value which CC to use to watch for
        //      activation msgs.
        // -------------------------------------------------------------------
        tZWaveUSB3Sh::ECClasses m_eActClass;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TGenSceneActUnit, TZWaveUnit)
};

#pragma CIDLIB_POPPACK
