//
// FILE NAME: ZWaveUSB3Sh_Unit_GenDimmer.cpp
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
//  This is the implementation for our generic one or two-way dimmer handler.
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
RTTIDecls(TGenDimmerUnit, TZWaveUnit)


// ---------------------------------------------------------------------------
//  Local types and constants
// ---------------------------------------------------------------------------
namespace ZWaveUSB3Sh_Unit_GenDimmer
{
    const TString   strOpt_DimType(L"/UnitOpt/GenDimmer/DimType");
    const TString   strOpt_SwitchType(L"/UnitOpt/GenDimmer/SwitchType");
}


// ---------------------------------------------------------------------------
//   CLASS: TGenDimmerUnit
//  PREFIX: unit
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TGenDimmerUnit: Constructors and Destructor
// ---------------------------------------------------------------------------
TGenDimmerUnit::TGenDimmerUnit(TZWUnitInfo* const punitiOwner) :

    TZWaveUnit(punitiOwner)
    , m_eAccess(tZWaveUSB3Sh::EUnitAcc::Write)
    , m_eDimClass(tZWaveUSB3Sh::ECClasses::Basic)
    , m_eSwitchClass(tZWaveUSB3Sh::ECClasses::Basic)
    , m_pzwcciSwitch(nullptr)
    , m_pzwcciLevel(nullptr)
{
}

TGenDimmerUnit::~TGenDimmerUnit()
{
}


// ---------------------------------------------------------------------------
//  TGenDimmerUnit: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  We parse out our access indicator and save that away for later use when
//  the CCs are created.
//
tCIDLib::TBoolean TGenDimmerUnit::bParseExtraInfo()
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
tCIDLib::TBoolean TGenDimmerUnit::bPrepare()
{
    if (!TParent::bPrepare())
        return kCIDLib::False;

    TZWUnitInfo& unitiUs = unitiOwner();

    // Get the two handler classes
    TString strDimType;
    if (!unitiUs.bFindOptVal(ZWaveUSB3Sh_Unit_GenDimmer::strOpt_DimType, strDimType))
    {
        facZWaveUSB3Sh().LogTraceErr
        (
            L"Unit %(1) has no dimmer command class option", unitiUs.strName()
        );
        return kCIDLib::False;
    }

    TString strSwitchType;
    if (!unitiUs.bFindOptVal(ZWaveUSB3Sh_Unit_GenDimmer::strOpt_SwitchType, strSwitchType))
    {
        facZWaveUSB3Sh().LogTraceErr
        (
            L"Unit %(1) has no switch command class option", unitiUs.strName()
        );
        return kCIDLib::False;
    }

    //
    //  And now create our CC impls based on the classes the user selected. If we are
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

    m_eSwitchClass = tZWaveUSB3Sh::eXlatECClasses(strSwitchType);
    if (m_eSwitchClass == tZWaveUSB3Sh::ECClasses::Basic)
    {
        m_pzwcciSwitch = new TZWCCImplBasicSwitchF
        (
            &unitiUs, m_strSwFldName, tCQCKit::EFldSTypes::LightSwitch
        );
    }
     else if (m_eSwitchClass == tZWaveUSB3Sh::ECClasses::BinSwitch)
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
            L"Unit %(1) unsupported Z-Wave switch command class", unitiUs.strName()
        );
        return kCIDLib::False;
    }

    // Add the switch to our parent class for the usual handling
    m_pzwcciSwitch->SetAccess(m_eAccess, tZWaveUSB3Sh::EUnitAcc::AllBits);
    m_pzwcciSwitch->strDesc(L"Switch");
    if (!bAddImplObject(m_pzwcciSwitch, c4ImplId))
        return kCIDLib::False;


    // Do the same as above but for the dimmer class
    if (tCIDLib::bAllBitsOn(m_eAccess, tZWaveUSB3Sh::EUnitAcc::Read))
    {
        m_strDimFldName = L"LGHT#Dim_";
        m_strDimFldName.Append(unitiUs.strName());
    }
    else
    {
        m_strDimFldName = unitiUs.strName();
        m_strDimFldName.Append(L"_Dimmer");
    }
    if (m_eDimClass == tZWaveUSB3Sh::ECClasses::Basic)
    {
        m_pzwcciLevel = new TZWCCImplBasicLevelF
        (
            &unitiUs, m_strDimFldName, tCQCKit::EFldSTypes::Dimmer
        );
    }
     else if (m_eDimClass == tZWaveUSB3Sh::ECClasses::MLSwitch)
    {
        m_pzwcciLevel = new TZWCCImplMLLevelF
        (
            &unitiUs, m_strDimFldName, tCQCKit::EFldSTypes::Dimmer
        );
    }
     else
    {
        facZWaveUSB3Sh().LogTraceErr
        (
            L"Unit %(1) unsupported Z-Wave level command class", unitiUs.strName()
        );
        return kCIDLib::False;
    }

    //
    //  Optimize here. If both are Basic, then both are going to respond to the
    //  same incoming msgs. So there's really no need to do any polling on both
    //  of them. That's just a redundant operation. In that case we'll disable
    //  polling on the dimmer.
    //
    if ((m_eSwitchClass == tZWaveUSB3Sh::ECClasses::Basic)
    &&  (m_eDimClass == tZWaveUSB3Sh::ECClasses::Basic))
    {
        m_pzwcciLevel->DisablePoll();
    }

    m_pzwcciLevel->SetAccess(m_eAccess, tZWaveUSB3Sh::EUnitAcc::AllBits);
    m_pzwcciLevel->strDesc(L"Dimmer");
    if (!bAddImplObject(m_pzwcciLevel, c4ImplId))
        return kCIDLib::False;

    return kCIDLib::True;
}


//
//  If both dim and switch CCs are the same (basic), then we disable ping on the
//  dimmer. So, to make up for that, we watch for the switch going into error state
//  and force the dimmer into error state as well.
//
tCIDLib::TVoid TGenDimmerUnit::ImplInErrState(const tCIDLib::TCard4 c4ImplId)
{
    if ((c4ImplId == m_pzwcciSwitch->c4ImplId())
    &&  (m_eSwitchClass == tZWaveUSB3Sh::ECClasses::Basic)
    &&  (m_eDimClass == tZWaveUSB3Sh::ECClasses::Basic))
    {
        m_pzwcciLevel->SetErrState();
    }
}


//
//  We have to watch for the switch impl to change and send a trigger if this is a
//  V2 type switch.
//
tCIDLib::TVoid
TGenDimmerUnit::ImplValueChanged(const  tCIDLib::TCard4         c4ImplId
                                , const tZWaveUSB3Sh::EValSrcs  eSource
                                , const tCIDLib::TBoolean       bWasInErr
                                , const tCIDLib::TInt4          )
{
    // If it's a programmatic change, we don't care about it
    if (eSource == tZWaveUSB3Sh::EValSrcs::Program)
        return;

    // If the switch changed and we are V2 type, then send a trigger
    if (c4ImplId == m_pzwcciSwitch->c4ImplId())
    {
        if (m_pzwcciSwitch->bIsReadable())
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
}


//
//  We override this because we need to report some user settable options.
//
tCIDLib::TVoid
TGenDimmerUnit::QueryUnitAttrs(  tCIDMData::TAttrList&   colToFill
                                    , TAttrData&            adatTmp) const
{
    TParent::QueryUnitAttrs(colToFill, adatTmp);

    const TZWUnitInfo& unitiUs = unitiOwner();

    adatTmp.Set(L"CC Options:", TString::strEmpty(), tCIDMData::EAttrTypes::Separator);
    colToFill.objAdd(adatTmp);

    adatTmp.Set
    (
        L"Dimmer Class"
        , ZWaveUSB3Sh_Unit_GenDimmer::strOpt_DimType
        , L"Enum: Basic, MLSwitch"
        , tCIDMData::EAttrTypes::String
    );

    // If no current value, we'll set ML switch if it is supported, else basic.
    adatTmp.SetString
    (
        unitiUs.strQueryOptVal
        (
            ZWaveUSB3Sh_Unit_GenDimmer::strOpt_DimType
            , unitiUs.bSupportsClass(tZWaveUSB3Sh::ECClasses::MLSwitch) ? L"MLSwitch"
                                                                     : L"Basic"
        )
    );
    colToFill.objAdd(adatTmp);


    adatTmp.Set
    (
        L"Switch Class"
        , ZWaveUSB3Sh_Unit_GenDimmer::strOpt_SwitchType
        , L"Enum: Basic, BinSwitch"
        , tCIDMData::EAttrTypes::String
    );

    // If no current value, we'll set bin switch if supported, else basic
    adatTmp.SetString
    (
        unitiUs.strQueryOptVal
        (
            ZWaveUSB3Sh_Unit_GenDimmer::strOpt_SwitchType
            , unitiUs.bSupportsClass(tZWaveUSB3Sh::ECClasses::BinSwitch) ? L"BinSwitch"
                                                                      : L"Basic"
        )
    );
    colToFill.objAdd(adatTmp);
}
