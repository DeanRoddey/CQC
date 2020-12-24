//
// FILE NAME: ZWaveUSB3Sh_GenSceneAct.cpp
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
//  This is the implementation for our generic scene activation handler.
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
//  Magic macros.
// ---------------------------------------------------------------------------
RTTIDecls(TGenSceneActUnit, TZWaveUnit)


// ---------------------------------------------------------------------------
//  Local types and constants
// ---------------------------------------------------------------------------
namespace ZWaveUSB3Sh_Unit_GenSceneAct
{
    const TString   strOpt_OnlyOnChange(L"/UnitOpt/GenSceneAct/OnChange");
}


// ---------------------------------------------------------------------------
//   CLASS: TGenSceneActUnit
//  PREFIX: unit
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TGenSceneActUnit: Constructors and Destructor
// ---------------------------------------------------------------------------
TGenSceneActUnit::TGenSceneActUnit(TZWUnitInfo* const punitiOwner) :

    TZWaveUnit(punitiOwner)
    , m_eActClass(tZWaveUSB3Sh::ECClasses::SceneAct)
{
}

TGenSceneActUnit::~TGenSceneActUnit()
{
}


// ---------------------------------------------------------------------------
//  TGenSceneActUnit: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  We parse out our activation CC to use.
//
tCIDLib::TBoolean TGenSceneActUnit::bParseExtraInfo()
{
    if (!TParent::bParseExtraInfo())
        return kCIDLib::False;

    TString strActClass;
    if (!bFindExtraVal(L"ActClass", strActClass))
        return kCIDLib::False;

    m_eActClass = tZWaveUSB3Sh::eXlatECClasses(strActClass);
    return kCIDLib::True;
}


//  We need to parse our extra data and create our impl objects.
tCIDLib::TBoolean TGenSceneActUnit::bPrepare()
{
    if (!TParent::bPrepare())
        return kCIDLib::False;

    TZWUnitInfo& unitiUs = unitiOwner();

    // Get the 'only on change' option value, taking true if not found
    const tCIDLib::TBoolean bOnlyOnChange =  unitiUs.bQueryOptVal
    (
        ZWaveUSB3Sh_Unit_GenSceneAct::strOpt_OnlyOnChange, kCIDLib::True
    );

    // Now create our CC impl and store it on the parent
    tCIDLib::TCard4 c4ImplId;
    if (m_eActClass == tZWaveUSB3Sh::ECClasses::SceneAct)
    {
        TZWCCImplSceneActState * pzwcciNew = new TZWCCImplSceneActState
        (
            &unitiUs, bOnlyOnChange
        );

        if (!bAddImplObject(pzwcciNew, c4ImplId))
            return kCIDLib::False;
    }
     else if (m_eActClass == tZWaveUSB3Sh::ECClasses::CentralScene)
    {
        TZWCCImplCentralScene* pzwcciNew = new TZWCCImplCentralScene
        (
            &unitiUs, bOnlyOnChange
        );

        if (!bAddImplObject(pzwcciNew, c4ImplId))
            return kCIDLib::False;
    }
     else
    {
        facZWaveUSB3Sh().LogTraceErr
        (
            L"Unit %(1) unsupported Z-Wave scene activation command class", unitiUs.strName()
        );
        return kCIDLib::False;
    }

    return kCIDLib::True;
}


//  We override this because we need to report some user settable options.
tCIDLib::TVoid
TGenSceneActUnit::QueryUnitAttrs(tCIDMData::TAttrList& colToFill, TAttrData& adatTmp) const
{
    TParent::QueryUnitAttrs(colToFill, adatTmp);

    const TZWUnitInfo& unitiUs = unitiOwner();
    adatTmp.Set(L"Scene Options:", TString::strEmpty(), tCIDMData::EAttrTypes::Separator);
    colToFill.objAdd(adatTmp);

    adatTmp.Set
    (
        L"Only On Change"
        , ZWaveUSB3Sh_Unit_GenSceneAct::strOpt_OnlyOnChange
        , tCIDMData::EAttrTypes::Bool
    );

    // If no current value, we'll set true
    adatTmp.SetBool
    (
        unitiUs.bQueryOptVal
        (
            ZWaveUSB3Sh_Unit_GenSceneAct::strOpt_OnlyOnChange, kCIDLib::True
        )
    );
    colToFill.objAdd(adatTmp);
}
