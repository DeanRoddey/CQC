//
// FILE NAME: ZWaveUSB3Sh_Unit_GenLightSwitch.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/11/2018
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
//  This is the implementation for our generic one or two-way light switch handler.
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
RTTIDecls(TGenLightSwitchUnit, TZWaveUnit)


// ---------------------------------------------------------------------------
//  Local types and constants
// ---------------------------------------------------------------------------
namespace ZWaveUSB3Sh_Unit_GenLightSwitch
{
    const TString   strOpt_SwitchType(L"/UnitOpt/GenLightSwitch/SwitchType");
}


// ---------------------------------------------------------------------------
//   CLASS: TGenLightSwitchUnit
//  PREFIX: unit
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TGenLightSwitchUnit: Constructors and Destructor
// ---------------------------------------------------------------------------
TGenLightSwitchUnit::TGenLightSwitchUnit(TZWUnitInfo* const punitiOwner) :

    TZWaveUnit(punitiOwner)
    , m_eAccess(tZWaveUSB3Sh::EUnitAcc::Write)
    , m_eSwitchClass(tZWaveUSB3Sh::ECClasses::Basic)
    , m_pzwcciSwitch(nullptr)
{
}

TGenLightSwitchUnit::~TGenLightSwitchUnit()
{
}


// ---------------------------------------------------------------------------
//  TGenLightSwitchUnit: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  We parse out our one/two way indicator and save that flag away for later use when
//  the fields are created.
//
tCIDLib::TBoolean TGenLightSwitchUnit::bParseExtraInfo()
{
    if (!TParent::bParseExtraInfo())
        return kCIDLib::False;

    TString strIOType;
    if (!bFindExtraVal(L"IOType", strIOType))
        return kCIDLib::False;

    if (strIOType.bCompareI(L"ReadWrite"))
        m_eAccess = tZWaveUSB3Sh::EUnitAcc::ReadWrite;
    else
        m_eAccess = tZWaveUSB3Sh::EUnitAcc::Write;

    return kCIDLib::True;
}


//  We need to parse our extra data and create our impl objects.
tCIDLib::TBoolean TGenLightSwitchUnit::bPrepare()
{
    if (!TParent::bPrepare())
        return kCIDLib::False;

    TZWUnitInfo& unitiUs = unitiOwner();

    // Get the switch handler class
    TString strSwitchType;
    if (!unitiUs.bFindOptVal(ZWaveUSB3Sh_Unit_GenLightSwitch::strOpt_SwitchType, strSwitchType))
    {
        facZWaveUSB3Sh().LogTraceErr
        (
            L"Unit %(1) has no light switch command class option", unitiUs.strName()
        );
        return kCIDLib::False;
    }

    //
    //  And now create our CC impl based on the class the user selected. If we are
    //  two way, we create a V2 compliant field.
    //
    tCIDLib::TCard4 c4ImplId;
    if (tCIDLib::bAllBitsOn(m_eAccess, tZWaveUSB3Sh::EUnitAcc::Read))
    {
        m_strSwFldName = L"LGHT#Sw_";
        m_strSwFldName.Append(unitiUs.strName());
    }
     else
    {
        m_strSwFldName = unitiUs.strName();
        m_strSwFldName.Append(L"_Switch");
    }

    const tZWaveUSB3Sh::ECClasses eSwitchClass = tZWaveUSB3Sh::eXlatECClasses(strSwitchType);
    if (eSwitchClass == tZWaveUSB3Sh::ECClasses::Basic)
    {
        m_pzwcciSwitch = new TZWCCImplBasicSwitchF
        (
            &unitiUs, m_strSwFldName, tCQCKit::EFldSTypes::LightSwitch
        );
    }
     else if (eSwitchClass == tZWaveUSB3Sh::ECClasses::BinSwitch)
    {
        m_pzwcciSwitch = new TZWCCImplBinSwitchF
        (
            &unitiUs, m_strSwFldName, tCQCKit::EFldSTypes::LightSwitch
        );
    }
     else
    {
        facZWaveUSB3Sh().LogTraceErr
        (
            L"Unit %(1) unsupported light switch command class", unitiUs.strName()
        );
        return kCIDLib::False;
    }

    // Add the impl to our parent class for the usual handling
    m_pzwcciSwitch->strDesc(L"Switch");
    m_pzwcciSwitch->SetAccess(m_eAccess, tZWaveUSB3Sh::EUnitAcc::AllBits);
    if (!bAddImplObject(m_pzwcciSwitch, c4ImplId))
        return kCIDLib::False;

    return kCIDLib::True;
}


//
//  We have to watch for the switch impl to change and send a trigger if this is a
//  V2 type switch.
//
tCIDLib::TVoid
TGenLightSwitchUnit::ImplValueChanged(  const   tCIDLib::TCard4         c4ImplId
                                        , const tZWaveUSB3Sh::EValSrcs  eSource
                                        , const tCIDLib::TBoolean       bWasInErr
                                        , const tCIDLib::TInt4          )
{
    // If it's a programmatic change, we don't care about it
    if (eSource == tZWaveUSB3Sh::EValSrcs::Program)
        return;

    // If it's for our switch CC and that guy is readable
    if ((c4ImplId == m_pzwcciSwitch->c4ImplId()) && m_pzwcciSwitch->bIsReadable())
    {
        TString strId;
        strId.AppendFormatted(c1Id());

        unitiOwner().QueueEventTrig
        (
            tCQCKit::EStdDrvEvs::LoadChange
            , m_strSwFldName
            , facCQCKit().strBoolOffOn(m_pzwcciSwitch->bState())
            , strId
            , strName()
            , TString::strEmpty()
        );
    }
}


//
//  We override this because we need to report some user settable options.
//
tCIDLib::TVoid
TGenLightSwitchUnit::QueryUnitAttrs(tCIDMData::TAttrList&   colToFill
                                    , TAttrData&            adatTmp) const
{
    TParent::QueryUnitAttrs(colToFill, adatTmp);

    const TZWUnitInfo& unitiUs = unitiOwner();

    adatTmp.Set(L"CC Options:", TString::strEmpty(), tCIDMData::EAttrTypes::Separator);
    colToFill.objAdd(adatTmp);

    adatTmp.Set
    (
        L"Switch Class"
        , ZWaveUSB3Sh_Unit_GenLightSwitch::strOpt_SwitchType
        , L"Enum: Basic, BinSwitch"
        , tCIDMData::EAttrTypes::String
    );

    // If no current value, we'll set bin switch if supported, else basic
    adatTmp.SetString
    (
        unitiUs.strQueryOptVal
        (
            ZWaveUSB3Sh_Unit_GenLightSwitch::strOpt_SwitchType
            , unitiUs.bSupportsClass(tZWaveUSB3Sh::ECClasses::BinSwitch) ? L"BinSwitch"
                                                                      : L"Basic"
        )
    );
    colToFill.objAdd(adatTmp);
}
