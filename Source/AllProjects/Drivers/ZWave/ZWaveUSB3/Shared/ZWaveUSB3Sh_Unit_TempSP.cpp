//
// FILE NAME: ZWaveUSB3Sh_TempSP.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/7/2018
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
//  This is the implementation for a simple temp sensor plus setpoint(s) module.
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
RTTIDecls(TTempSPUnit, TZWaveUnit)



// ---------------------------------------------------------------------------
//   CLASS: TTempSPUnit
//  PREFIX: unit
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TTempSPUnit: Constructors and Destructor
// ---------------------------------------------------------------------------
TTempSPUnit::TTempSPUnit(TZWUnitInfo* const punitiOwner) :

    TZWaveUnit(punitiOwner)
    , m_bDoLowSP(kCIDLib::False)
    , m_bDoHighSP(kCIDLib::False)
    , m_bDoThermoMode(kCIDLib::False)
{
}

TTempSPUnit::~TTempSPUnit()
{
}


// ---------------------------------------------------------------------------
//  TTempSPUnit: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  WE need to store the flags for our optional functionality here instead of waiting
//  for bPrepare, because we may need them in QueryUnitAttrs(), since it should only
//  return attributes on things we actually are doing.
//
tCIDLib::TBoolean TTempSPUnit::bParseExtraInfo()
{
    if (!TParent::bParseExtraInfo())
        return kCIDLib::False;

    // Initialize option flags first
    m_bDoLowSP = kCIDLib::False;
    m_bDoHighSP = kCIDLib::False;

    TString strVal;
    if (bFindExtraVal(L"Features", strVal) && !strVal.bIsEmpty())
    {
        TString strFeature;
        TStringTokenizer stokFeatures(&strVal, L" ");
        while (stokFeatures.bGetNextToken(strFeature))
        {
            if (strFeature.bCompareI(L"LowSP"))
                m_bDoLowSP = kCIDLib::True;
            else if (strFeature.bCompareI(L"HighSP"))
                m_bDoHighSP = kCIDLib::True;
            else
            {
                facZWaveUSB3Sh().LogTraceErr(L"Unknown TempSP feature info");
                return kCIDLib::False;
            }
        }
    }

    if (!m_bDoLowSP && !m_bDoHighSP)
    {
        facZWaveUSB3Sh().LogTraceErr(L"At least one set point must be supported");
        return kCIDLib::False;
    }

    // See if thermo mode is implemented
    m_bDoThermoMode = bSupportsClass(tZWaveUSB3Sh::ECClasses::ThermoMode);

    return kCIDLib::True;
}


//
//  We create and store our CC impl objects. There are quite a few.
//
tCIDLib::TBoolean TTempSPUnit::bPrepare()
{
    if (!TParent::bPrepare())
        return kCIDLib::False;

    TZWUnitInfo& unitiUs = unitiOwner();


    // Get the scale and range options
    const tCIDLib::TBoolean bFScale
    (
        unitiUs.strQueryOptVal(kZWaveUSB3Sh::strUOpt_TempScale, L"F") == L"F"
    );
    const tCIDLib::TInt4 i4Min = unitiUs.i4QueryOptVal
    (
        kZWaveUSB3Sh::strUOpt_CurTempMin, bFScale ? -20 : -30
    );
    const tCIDLib::TInt4 i4Max = unitiUs.i4QueryOptVal
    (
        kZWaveUSB3Sh::strUOpt_CurTempMax, bFScale ? 120 : 55
    );


    // The current temp CC is required. It's a multi-level sensor
    tCIDLib::TCard4 c4ImplId;
    TString strFldName(32UL);
    strFldName = L"Thermo_";
    strFldName.Append(unitiUs.strName());
    strFldName.Append(L"_CurrentTemp");

    TZWCCImplMLSensorF* pzwcciCurTemp = new TZWCCImplMLSensorF
    (
        &unitiUs, strFldName, tCQCKit::EFldSTypes::CurTemp
    );

    //
    //  Force the CC's format since it's not set via extra info in this case.
    //  The default access bits should be fine.
    //
    pzwcciCurTemp->SetFormat
    (
        tCQCKit::EFldTypes::Int
        , i4Min
        , i4Max
        , bFScale ? tZWaveUSB3Sh::ETScales::F : tZWaveUSB3Sh::ETScales::C
    );
    pzwcciCurTemp->strDesc(L"Temperature");
    if (!bAddImplObject(pzwcciCurTemp, c4ImplId))
        return kCIDLib::False;

    // If doing low set point
    if (m_bDoLowSP)
    {
        strFldName = L"Thermo_";
        strFldName.Append(unitiUs.strName());
        strFldName.Append(L"_LowSetPnt");
        TZWCCImplThermoSP* pzwcciSP = new TZWCCImplThermoSP
        (
            &unitiUs, strFldName, kCIDLib::False, bFScale
        );

        if (!bAddImplObject(pzwcciSP, c4ImplId))
            return kCIDLib::False;
    }

    // If doing high set point
    if (m_bDoHighSP)
    {
        strFldName = L"Thermo_";
        strFldName.Append(unitiUs.strName());
        strFldName.Append(L"_HighSetPnt");
        TZWCCImplThermoSP* pzwcciSP = new TZWCCImplThermoSP
        (
            &unitiUs, strFldName, kCIDLib::True, bFScale
        );
        if (!bAddImplObject(pzwcciSP, c4ImplId))
            return kCIDLib::False;
    }

    // If thermo mode is supported, add that
    if (m_bDoThermoMode)
    {
        strFldName = L"Thermo_";
        strFldName.Append(unitiUs.strName());
        strFldName.Append(L"_CurMode");

        TZWCCImplThermoMode* pzwcciMode = new TZWCCImplThermoMode(&unitiUs, strFldName);
        if (!bAddImplObject(pzwcciMode, c4ImplId))
            return kCIDLib::False;
    }

    return kCIDLib::True;
}


//
//  For now we only provide options for current temp. We could maybe later provide some
//  for low/high set point and such, but that gets really messy. If by some chance the
//  temp range gets out of sync with the temp scale, we force them back to reasonable
//  values, which will be seen as a change so that they will get saved back to something
//  reasonable.
//
tCIDLib::TVoid
TTempSPUnit::QueryUnitAttrs(tCIDMData::TAttrList& colToFill, TAttrData& adatTmp) const
{
    TParent::QueryUnitAttrs(colToFill, adatTmp);

    const TZWUnitInfo& unitiUs = unitiOwner();

    adatTmp.Set(L"Thermo Options:", TString::strEmpty(), tCIDMData::EAttrTypes::Separator);
    colToFill.objAdd(adatTmp);

    //
    //  The thermostat CC helper class has a method to do the standard temp scale and
    //  min/max temp range. That'still ok for
    //
    TZWUSB3CCThermo::QueryTempAttrs(unitiUs, colToFill, adatTmp);
}
