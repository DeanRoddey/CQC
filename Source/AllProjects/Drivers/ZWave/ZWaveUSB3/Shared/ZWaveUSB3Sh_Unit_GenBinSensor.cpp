//
// FILE NAME: ZWaveUSB3Sh_GenBinSensor.cpp
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
//  This is the implementation for a generic motion sensor unit handler.
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
RTTIDecls(TGenBinSensorUnit, TZWaveUnit)


// ---------------------------------------------------------------------------
//  Local types and constants
// ---------------------------------------------------------------------------
namespace ZWaveUSB3Sh_Unit_GenBinSensorUnit
{
    const TString   strOpt_SensorType(L"/UnitOpt/GenBinSensor/Type");
    const TString   strOpt_CCImpl(L"/UnitOpt/GenBinSensor/CCImpl");
}


// ---------------------------------------------------------------------------
//   CLASS: TGenBinSensorUnit
//  PREFIX: unit
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TGenBinSensorUnit: Constructors and Destructor
// ---------------------------------------------------------------------------
TGenBinSensorUnit::TGenBinSensorUnit(TZWUnitInfo* const punitiOwner) :

    TZWaveUnit(punitiOwner)
    , m_eSensorType(tZWaveUSB3Sh::EBinSensorTypes::Door)
    , m_pzwcciSensor(nullptr)
{
}

TGenBinSensorUnit::~TGenBinSensorUnit()
{
}


// ---------------------------------------------------------------------------
//  TGenBinSensorUnit: Public, inherited methods
// ---------------------------------------------------------------------------

//  We need to parse our extra data and create our impl objects.
tCIDLib::TBoolean TGenBinSensorUnit::bPrepare()
{
    if (!TParent::bPrepare())
        return kCIDLib::False;

    TZWUnitInfo& unitiUs = unitiOwner();

    // Get our sensor type option. If not found, we can't do anything
    TString strType;
    if (!unitiUs.bFindOptVal(ZWaveUSB3Sh_Unit_GenBinSensorUnit::strOpt_SensorType, strType))
    {
        facZWaveUSB3Sh().LogTraceErr
        (
            L"Unit %(1) has no sensor type option value", unitiUs.strName()
        );
        return kCIDLib::False;
    }

    TString strCCImpl;
    if (!unitiUs.bFindOptVal(ZWaveUSB3Sh_Unit_GenBinSensorUnit::strOpt_CCImpl, strCCImpl))
    {
        facZWaveUSB3Sh().LogTraceErr
        (
            L"Unit %(1) has no Z-Wave command class option value", unitiUs.strName()
        );
        return kCIDLib::False;
    }

    //
    //  Make sure the type is legal and store it away for later access. This is just
    //  for server side use.
    //
    m_eSensorType = tZWaveUSB3Sh::eXlatEBinSensorTypes(strType);
    if (m_eSensorType == tZWaveUSB3Sh::EBinSensorTypes::Count)
    {
        facZWaveUSB3Sh().LogTraceErr
        (
            L"Unit %(1), %(2) is not a valid sensor type option value"
            , unitiUs.strName()
            , strType
        );
        return kCIDLib::False;
    }

    // Build our field name and a semantic type
    tCQCKit::EFldSTypes eSemType;
    if (m_eSensorType == tZWaveUSB3Sh::EBinSensorTypes::Motion)
    {
        m_strFldName = L"MOT#";
        m_strFldName.Append(unitiUs.strName());
        eSemType = tCQCKit::EFldSTypes::MotionSensor;
    }
     else
    {
        m_strFldName = unitiUs.strName();
        m_strFldName.Append(L"_State");
        eSemType = tCQCKit::EFldSTypes::Generic;
    }


    // And check the command class
    tCIDLib::TCard4 c4ImplId;
    tZWaveUSB3Sh::ECClasses eClass = tZWaveUSB3Sh::eXlatECClasses(strCCImpl);
    m_pzwcciSensor = nullptr;
    if (eClass == tZWaveUSB3Sh::ECClasses::Basic)
    {
        m_pzwcciSensor = new TZWCCImplBasicSwitchF(&unitiUs, m_strFldName, eSemType);
        if (!bAddImplObject(m_pzwcciSensor, c4ImplId))
            return kCIDLib::False;
    }
     else if (eClass == tZWaveUSB3Sh::ECClasses::BinSensor)
    {
        m_pzwcciSensor = new TZWCCImplBinSensorF(&unitiUs, m_strFldName, eSemType);
        if (!bAddImplObject(m_pzwcciSensor, c4ImplId))
            return kCIDLib::False;
    }
     else if (eClass == tZWaveUSB3Sh::ECClasses::Notification)
    {
        TZWCCImplNotifyState* pzwcciNot = new TZWCCImplNotifyStateF
        (
            &unitiUs, m_strFldName, eSemType
        );

        if (!pzwcciNot->bSetNotIds(m_eSensorType))
        {
            delete pzwcciNot;
            return kCIDLib::False;
        }

        m_pzwcciSensor = pzwcciNot;
        if (!bAddImplObject(m_pzwcciSensor, c4ImplId))
            return kCIDLib::False;
    }
     else
    {
        facZWaveUSB3Sh().LogTraceErr
        (
            L"Unit %(1) unsupported Z-Wave command class", unitiUs.strName()
        );
        return kCIDLib::False;
    }

    return kCIDLib::True;
}


tCIDLib::TVoid
TGenBinSensorUnit::ImplValueChanged(const   tCIDLib::TCard4         c4ImplId
                                    , const tZWaveUSB3Sh::EValSrcs  eSource
                                    , const tCIDLib::TBoolean       bWasInErr
                                    , const tCIDLib::TInt4          )
{
    // If not a programmatic change, and for our value CC
    if ((eSource != tZWaveUSB3Sh::EValSrcs::Program)
    &&  (m_pzwcciSensor->c4ImplId() == c4ImplId))
    {
        TString strId;
        strId.AppendFormatted(c1Id());

        if (m_eSensorType == tZWaveUSB3Sh::EBinSensorTypes::Motion)
        {
            // Motion events are optional
            if (bQueryOptVal(kZWaveUSB3Sh::strUOpt_SendTrigger, kCIDLib::True))
            {
                unitiOwner().QueueEventTrig
                (
                    tCQCKit::EStdDrvEvs::Motion
                    , m_strFldName
                    , facCQCKit().strBoolStartEnd(m_pzwcciSensor->bState())
                    , strId
                    , strName()
                    , TString::strEmpty()
                );
            }
        }
    }
}



//
//  We override this because we need to report some user settable options.
//
//  1.  If this is a motion sensor, then we need an option as to whether it should
//      send triggers.
//  2.  The sensor type to create
//  3.  The Z-Wave CC impl class to use
//
tCIDLib::TVoid
TGenBinSensorUnit::QueryUnitAttrs(  tCIDMData::TAttrList&   colToFill
                                    , TAttrData&            adatTmp) const
{
    TParent::QueryUnitAttrs(colToFill, adatTmp);

    const TZWUnitInfo& unitiUs = unitiOwner();
    if (m_eSensorType == tZWaveUSB3Sh::EBinSensorTypes::Motion)
    {
        adatTmp.Set(L"Motion Options:", TString::strEmpty(), tCIDMData::EAttrTypes::Separator);
        colToFill.objAdd(adatTmp);

        adatTmp.Set
        (
            L"Send Triggers"
            , kZWaveUSB3Sh::strUOpt_SendTrigger
            , tCIDMData::EAttrTypes::Bool
        );

        // If no current value, we'll set true
        adatTmp.SetBool
        (
            unitiUs.bQueryOptVal(kZWaveUSB3Sh::strUOpt_SendTrigger, kCIDLib::True)
        );
        colToFill.objAdd(adatTmp);
    }


    // Let the user select which CC impl to use
    adatTmp.Set(L"Sensor Options:", TString::strEmpty(), tCIDMData::EAttrTypes::Separator);
    colToFill.objAdd(adatTmp);

    adatTmp.Set
    (
        L"Z-Wave Command Class"
        , ZWaveUSB3Sh_Unit_GenBinSensorUnit::strOpt_CCImpl
        , L"Enum: Basic, BinSensor, Notification"
        , tCIDMData::EAttrTypes::String
    );

    // If no current value, we'll set Basic
    adatTmp.SetString
    (
        unitiUs.strQueryOptVal
        (
            ZWaveUSB3Sh_Unit_GenBinSensorUnit::strOpt_CCImpl, L"Basic"
        )
    );
    colToFill.objAdd(adatTmp);

    // Let the user select the sensor type
    TString strLims;
    tZWaveUSB3Sh::FormatEBinSensorTypes(strLims, L"Enum:", kCIDLib::chComma);
    adatTmp.Set
    (
        L"Sensor Type"
        , ZWaveUSB3Sh_Unit_GenBinSensorUnit::strOpt_SensorType
        , strLims
        , tCIDMData::EAttrTypes::String
    );

    // If no current value, we'll set the first one
    adatTmp.SetString
    (
        unitiUs.strQueryOptVal
        (
            ZWaveUSB3Sh_Unit_GenBinSensorUnit::strOpt_SensorType
            , tZWaveUSB3Sh::strXlatEBinSensorTypes(tZWaveUSB3Sh::EBinSensorTypes::Min)
        )
    );
    colToFill.objAdd(adatTmp);
}
