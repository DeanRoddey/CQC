//
// FILE NAME: ZWaveUSB3Sh_Thermostat.cpp
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
//  This is the implementation for a parameterizable thermostat.
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
RTTIDecls(TThermoUnit, TZWaveUnit)



// ---------------------------------------------------------------------------
//   CLASS: TThermoUnit
//  PREFIX: unit
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TThermoUnit: Constructors and Destructor
// ---------------------------------------------------------------------------
TThermoUnit::TThermoUnit(TZWUnitInfo* const punitiOwner) :

    TZWaveUnit(punitiOwner)
    , m_bDoFan(kCIDLib::False)
    , m_bDoLowSP(kCIDLib::False)
    , m_bDoHighSP(kCIDLib::False)
    , m_bV2Compat(kCIDLib::False)
{
}

TThermoUnit::~TThermoUnit()
{
}


// ---------------------------------------------------------------------------
//  TThermoUnit: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  WE need to store the flags for our optional functionality here instead of waiting
//  for bPrepare, because we may need them in QueryUnitAttrs(), since it should only
//  return attributes on things we actually are doing.
//
tCIDLib::TBoolean TThermoUnit::bParseExtraInfo()
{
    if (!TParent::bParseExtraInfo())
        return kCIDLib::False;

    // Initialize option flags first
    m_bDoFan = kCIDLib::False;
    m_bDoLowSP = kCIDLib::False;
    m_bDoHighSP = kCIDLib::False;

    //
    //  First check for the V2 compatibility value. If present, then we don't need
    //  do anything else. We know what that means and will force everything to be
    //  correct.
    //
    //  If not, then we use extra info to set everything up.
    //
    TString strVal;
    if (bFindExtraVal(L"V2Compat", strVal) && strVal.bCompareI(L"yes"))
    {
        // The unit has to be always on
        if (!unitiOwner().bAlwaysOn())
        {
            facZWaveUSB3Sh().LogTraceErr(L"V2 thermos must be always on");
            return kCIDLib::False;
        }

        m_bV2Compat = kCIDLib::True;
        m_bDoFan = kCIDLib::True;
        m_bDoLowSP = kCIDLib::True;
        m_bDoHighSP = kCIDLib::True;
    }
     else
    {
        m_bV2Compat = kCIDLib::False;

        // Get our features extra info
        if (bFindExtraVal(L"Features", strVal) && !strVal.bIsEmpty())
        {
            TString strFeature;
            TStringTokenizer stokFeatures(&strVal, L" ");
            while (stokFeatures.bGetNextToken(strFeature))
            {
                if (strFeature.bCompareI(L"Fan"))
                    m_bDoFan = kCIDLib::True;
                else if (strFeature.bCompareI(L"LowSP"))
                    m_bDoLowSP = kCIDLib::True;
                else if (strFeature.bCompareI(L"HighSP"))
                    m_bDoHighSP = kCIDLib::True;
                else
                {
                    facZWaveUSB3Sh().LogTraceErr(L"Unknown thermostat feature info");
                    return kCIDLib::False;
                }
            }
        }
    }
    return kCIDLib::True;
}


//
//  We create and store our CC impl objects. There are quite a few.
//
tCIDLib::TBoolean TThermoUnit::bPrepare()
{
    if (!TParent::bPrepare())
        return kCIDLib::False;

    //
    //  If all functionality is implemented correctly, assume we will be V2 compatible,
    //  though we have to check the access afterwards to make sure.
    //
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
    {
        if (m_bV2Compat)
        {
            strFldName = L"THERM#";
            strFldName.Append(unitiUs.strName());
            strFldName.Append(L"~CurrentTemp");
        }
         else
        {
            strFldName = L"Thermo_";
            strFldName.Append(unitiUs.strName());
            strFldName.Append(L"_CurrentTemp");
        }

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
    }

    //
    //  The current (settable) mode is required. He will set up his own field limits
    //  based on CC specific extra info.
    //
    {
        if (m_bV2Compat)
        {
            strFldName = L"THERM#";
            strFldName.Append(unitiUs.strName());
            strFldName.Append(L"~CurMode");
        }
         else
        {
            strFldName = L"Thermo_";
            strFldName.Append(unitiUs.strName());
            strFldName.Append(L"_CurMode");
        }

        TZWCCImplThermoMode* pzwcciMode = new TZWCCImplThermoMode(&unitiUs, strFldName);
        if (!bAddImplObject(pzwcciMode, c4ImplId))
            return kCIDLib::False;
    }


    //
    //  The current (readable) operating mode is required. He will set up his own
    //  field limits based on CC specific extra info.
    //
    {
        if (m_bV2Compat)
        {
            strFldName = L"THERM#";
            strFldName.Append(unitiUs.strName());
            strFldName.Append(L"~OpMode");
        }
         else
        {
            strFldName = L"Thermo_";
            strFldName.Append(unitiUs.strName());
            strFldName.Append(L"_OpMode");
        }
        TZWCCImplThermoState* pzwcciState = new TZWCCImplThermoState(&unitiUs, strFldName);
        if (!bAddImplObject(pzwcciState, c4ImplId))
            return kCIDLib::False;
    }


    // If doing fans, then we add the mode and operating mode
    if (m_bDoFan)
    {
        if (m_bV2Compat)
        {
            strFldName = L"THERM#";
            strFldName.Append(unitiUs.strName());
            strFldName.Append(L"~FanMode");
        }
         else
        {
            strFldName = L"Thermo_";
            strFldName.Append(unitiUs.strName());
            strFldName.Append(L"_FanMode");
        }
        TZWCCImplThermoFanMode* pzwcciMode = new TZWCCImplThermoFanMode
        (
            &unitiUs, strFldName
        );
        if (!bAddImplObject(pzwcciMode, c4ImplId))
            return kCIDLib::False;


        if (m_bV2Compat)
        {
            strFldName = L"THERM#";
            strFldName.Append(unitiUs.strName());
            strFldName.Append(L"~FanOpMode");
        }
         else
        {
            strFldName = L"Thermo_";
            strFldName.Append(unitiUs.strName());
            strFldName.Append(L"_FanOpMode");
        }
        TZWCCImplThermoFanState* pzwcciState = new TZWCCImplThermoFanState
        (
            &unitiUs, strFldName
        );
        if (!bAddImplObject(pzwcciState, c4ImplId))
            return kCIDLib::False;
    }

    // If doing low set point
    if (m_bDoLowSP)
    {
        if (m_bV2Compat)
        {
            strFldName = L"THERM#";
            strFldName.Append(unitiUs.strName());
            strFldName.Append(L"~LowSetPnt");
        }
         else
        {
            strFldName = L"Thermo_";
            strFldName.Append(unitiUs.strName());
            strFldName.Append(L"_LowSetPnt");
        }
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
        if (m_bV2Compat)
        {
            strFldName = L"THERM#";
            strFldName.Append(unitiUs.strName());
            strFldName.Append(L"~HighSetPnt");
        }
         else
        {
            strFldName = L"Thermo_";
            strFldName.Append(unitiUs.strName());
            strFldName.Append(L"_HighSetPnt");
        }

        TZWCCImplThermoSP* pzwcciSP = new TZWCCImplThermoSP
        (
            &unitiUs, strFldName, kCIDLib::True, bFScale
        );
        if (!bAddImplObject(pzwcciSP, c4ImplId))
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
TThermoUnit::QueryUnitAttrs(tCIDMData::TAttrList& colToFill, TAttrData& adatTmp) const
{
    TParent::QueryUnitAttrs(colToFill, adatTmp);

    const TZWUnitInfo& unitiUs = unitiOwner();

    adatTmp.Set(L"Thermo Options:", TString::strEmpty(), tCIDMData::EAttrTypes::Separator);
    colToFill.objAdd(adatTmp);

    //
    //  The thermostat CC helper class has a method to do the standard temp scale and
    //  min/max temp range.
    //
    TZWUSB3CCThermo::QueryTempAttrs(unitiUs, colToFill, adatTmp);
}

