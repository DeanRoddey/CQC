//
// FILE NAME: ZWaveUSB3Sh_Unit_GenDimmer.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/02/2018
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
//  This is the implementation for our parameterizable dimmer handler.
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
RTTIDecls(TDimmerUnit, TZWaveUnit)


// ---------------------------------------------------------------------------
//   CLASS: TDimmerUnit
//  PREFIX: unit
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TDimmerUnit: Constructors and Destructor
// ---------------------------------------------------------------------------
TDimmerUnit::TDimmerUnit(TZWUnitInfo* const punitiOwner) :

    TZWaveUnit(punitiOwner)
    , m_eAccess(tZWaveUSB3Sh::EUnitAcc::Write)
    , m_eDimClass(tZWaveUSB3Sh::ECClasses::Basic)
    , m_eSwitchClass(tZWaveUSB3Sh::ECClasses::Basic)
    , m_pzwcciSwitch(nullptr)
    , m_pzwcciLevel(nullptr)
{
}

TDimmerUnit::~TDimmerUnit()
{
}


// ---------------------------------------------------------------------------
//  TDimmerUnit: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  We parse out our one/two way indicator and save that away for later use when
//  the CCs are created.
//
tCIDLib::TBoolean TDimmerUnit::bParseExtraInfo()
{
    if (!TParent::bParseExtraInfo())
        return kCIDLib::False;

    TZWUnitInfo& unitiUs = unitiOwner();
    TString strIOType;
    if (!bFindExtraVal(L"IOType", strIOType))
        return kCIDLib::False;

    if (strIOType.bCompareI(L"ReadWrite"))
        m_eAccess = tZWaveUSB3Sh::EUnitAcc::ReadWrite;
    else
        m_eAccess = tZWaveUSB3Sh::EUnitAcc::Write;

    // Look up or switch and dimmer CCs
    TString strDimCC;
    TString strSwitchCC;
    if (!bFindExtraVal(L"DimCC", strDimCC)
    ||  !bFindExtraVal(L"SwitchCC", strSwitchCC))
    {
        return kCIDLib::False;
    }

    // Translate them. If either is bad we fail
    m_eDimClass = tZWaveUSB3Sh::eXlatECClasses(strDimCC);
    m_eSwitchClass = tZWaveUSB3Sh::eXlatECClasses(strSwitchCC);

    if ((m_eDimClass == tZWaveUSB3Sh::ECClasses::Count)
    ||  (m_eSwitchClass == tZWaveUSB3Sh::ECClasses::Count))
    {
        facZWaveUSB3Sh().LogTraceErr
        (
            L"Unit %(1), the switch or dimmer CC extra info is invalid (%(2), %(3)"
            , unitiUs.strName()
            , strDimCC
            , strSwitchCC
        );
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


//  We need to parse our extra data and create our impl objects.
tCIDLib::TBoolean TDimmerUnit::bPrepare()
{
    if (!TParent::bPrepare())
        return kCIDLib::False;

    TZWUnitInfo& unitiUs = unitiOwner();

    //
    //  See if they want to force the names. These are optional so we can just wait
    //  till now and get them.
    //
    tCIDLib::TBoolean bForceName = kCIDLib::False;
    TString strSwitchName = L"Switch";
    TString strLevelName = L"Dimmer";
    if (bFindExtraVal(L"SwitchName", strSwitchName)
    &&  bFindExtraVal(L"LevelName", strLevelName))
    {
        bForceName = kCIDLib::True;
    }

    // If possible, be V2 compliant
    tCIDLib::TCard4 c4ImplId;
    if (tCIDLib::bAllBitsOn(m_eAccess, tZWaveUSB3Sh::EUnitAcc::Read) && !bForceName)
    {
        m_strSwFldName = L"LGHT#Sw_";
        m_strSwFldName.Append(unitiUs.strName());
    }
     else
    {
        m_strSwFldName = unitiUs.strName();
        m_strSwFldName.Append(kCIDLib::chUnderscore);
        m_strSwFldName.Append(strSwitchName);
    }

    // And now create our CC impls based on the classes the user selected.
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
     else if (m_eSwitchClass == tZWaveUSB3Sh::ECClasses::MLSwitch)
    {
        m_pzwcciSwitch = new TZWCCImplMLSwitchF
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
    m_pzwcciSwitch->strDesc(strSwitchName);
    if (!bAddImplObject(m_pzwcciSwitch, c4ImplId))
        return kCIDLib::False;


    // Do the same as above but for the dimmer class
    if (tCIDLib::bAllBitsOn(m_eAccess, tZWaveUSB3Sh::EUnitAcc::Read) && !bForceName)
    {
        m_strDimFldName = L"LGHT#Dim_";
        m_strDimFldName.Append(unitiUs.strName());
    }
    else
    {
        m_strDimFldName = unitiUs.strName();
        m_strDimFldName.Append(kCIDLib::chUnderscore);
        m_strDimFldName.Append(strLevelName);
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
    m_pzwcciLevel->SetAccess(m_eAccess, tZWaveUSB3Sh::EUnitAcc::AllBits);
    m_pzwcciLevel->strDesc(strLevelName);
    if (!bAddImplObject(m_pzwcciLevel, c4ImplId))
        return kCIDLib::False;

    return kCIDLib::True;
}


//
//  We have to watch for the switch impl to change and send a trigger if this is a
//  V2 type switch.
//
tCIDLib::TVoid
TDimmerUnit::ImplValueChanged(  const   tCIDLib::TCard4         c4ImplId
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


