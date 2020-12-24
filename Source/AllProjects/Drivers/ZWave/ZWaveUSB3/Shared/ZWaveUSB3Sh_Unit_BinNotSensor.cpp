//
// FILE NAME: ZWaveUSB3Sh_NotBinSensor.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/02/2017
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
//  This is the implementation for a semi-generic binary sensor, see the header
//  class comments.
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
RTTIDecls(TBinNotSensorUnit, TZWaveUnit)




// ---------------------------------------------------------------------------
//   CLASS: TBinNotSensorUnit
//  PREFIX: unit
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TBinNotSensorUnit: Constructors and Destructor
// ---------------------------------------------------------------------------
TBinNotSensorUnit::TBinNotSensorUnit(TZWUnitInfo* const punitiOwner) :

    TZWaveUnit(punitiOwner)
    , m_eHandlerClass(tZWaveUSB3Sh::ECClasses::None)
    , m_eSensorType(tZWaveUSB3Sh::EBinSensorTypes::Count)
    , m_pzwcciVal(nullptr)
{
}

TBinNotSensorUnit::~TBinNotSensorUnit()
{
}


// ---------------------------------------------------------------------------
//  TBinNotSensorUnit: Public, inherited methods
// ---------------------------------------------------------------------------

// We parse our extra info and store the data away for later use
tCIDLib::TBoolean TBinNotSensorUnit::bParseExtraInfo()
{
    if (!TParent::bParseExtraInfo())
        return kCIDLib::False;

    TZWUnitInfo& unitiUs = unitiOwner();

    // Default our members related to the type of sensor
    m_eHandlerClass = tZWaveUSB3Sh::ECClasses::None;
    m_eSensorType = tZWaveUSB3Sh::EBinSensorTypes::Count;

    // Figure out what CC to use
    TString strHandlerClass;
    if (!bFindExtraVal(L"Class", strHandlerClass))
    {
        if (facZWaveUSB3Sh().bLowTrace())
        {
            facZWaveUSB3Sh().LogTraceErr
            (
                L"No handler class info for unit %(1)", unitiUs.strName()
            );
        }
        return kCIDLib::False;
    }

    m_eHandlerClass = tZWaveUSB3Sh::eXlatECClasses(strHandlerClass);

    //
    //  Get the type of the sensor, which isn't a CQC driver semantic type but types
    //  of sensors we can understand.
    //
    TString strType;
    if (!bFindExtraVal(L"Type", strType))
    {
        if (facZWaveUSB3Sh().bLowTrace())
            facZWaveUSB3Sh().LogTraceErr(L"No sensor type for unit %(1)", unitiUs.strName());
        return kCIDLib::False;
    }

    m_eSensorType = tZWaveUSB3Sh::eXlatEBinSensorTypes(strType);
    if (m_eSensorType == tZWaveUSB3Sh::EBinSensorTypes::Count)
    {
        if (facZWaveUSB3Sh().bLowTrace())
        {
            facZWaveUSB3Sh().LogTraceErr
            (
                L"Bad sensor type for unit %(1), value=", unitiUs.strName(), strType
            );
        }
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


//  We need to create our impl objects.
tCIDLib::TBoolean TBinNotSensorUnit::bPrepare()
{
    if (!TParent::bPrepare())
        return kCIDLib::False;

    TZWUnitInfo& unitiUs = unitiOwner();

    // Create the right field name and semantic type
    tCQCKit::EFldSTypes eSemType;
    if (m_eSensorType == tZWaveUSB3Sh::EBinSensorTypes::Motion)
    {
        m_strFldName = L"MOT#";
        m_strFldName.Append(unitiUs.strName());
        eSemType = tCQCKit::EFldSTypes::MotionSensor;
    }
     else
    {
        m_strFldName= unitiUs.strName();
        m_strFldName.Append(L"_State");

        eSemType = tCQCKit::EFldSTypes::Generic;
    }

    //
    //  OK, based on the representation type, we gen up the correct impl objects and
    //  set them on our parent.
    //
    //  These are all assumed to be read only since they are sensors, so the default
    //  access set by the base CC impl class is fine for us.
    //
    //  If notification, he will look for the notification oriented extra info that
    //  is required.
    //
    m_pzwcciVal = nullptr;
    if (m_eHandlerClass == tZWaveUSB3Sh::ECClasses::Basic)
    {
        m_pzwcciVal = new TZWCCImplBasicSwitchF(&unitiUs, m_strFldName, eSemType);
    }
     else if (m_eHandlerClass == tZWaveUSB3Sh::ECClasses::BinSensor)
    {
        m_pzwcciVal = new TZWCCImplBinSensorF(&unitiUs, m_strFldName, eSemType);
    }
     else if (m_eHandlerClass == tZWaveUSB3Sh::ECClasses::Notification)
    {
        m_pzwcciVal = new TZWCCImplNotifyStateF(&unitiUs, m_strFldName, eSemType);
    }
     else
    {
        facZWaveUSB3Sh().LogTraceErr
        (
            L"Unknown handler class (%(1)) for unit %(2)"
            , tZWaveUSB3Sh::strXlatECClasses(m_eHandlerClass)
            , unitiUs.strName()
        );
        return kCIDLib::False;
    }

    m_pzwcciVal->strDesc(tZWaveUSB3Sh::strXlatEBinSensorTypes(m_eSensorType));

    //
    //  Add it to our parent. He will be given a chance to do any prep work and find
    //  any extra info from the device info file. If he fails, we get back false and
    //  return that and this unit will be failed.
    //
    tCIDLib::TCard4 c4Id;
    return bAddImplObject(m_pzwcciVal, c4Id);
}


//
//  If we get a value change from our impl and we are a motion sensor, and we are
//  configured to send triggers, then we need to do that.
//
tCIDLib::TVoid
TBinNotSensorUnit::ImplValueChanged(const   tCIDLib::TCard4         c4ImplId
                                    , const tZWaveUSB3Sh::EValSrcs  eSource
                                    , const tCIDLib::TBoolean       bWasInErr
                                    , const tCIDLib::TInt4          )
{
    // If not a programmatic change, and it's for our value CC
    if ((c4ImplId == m_pzwcciVal->c4ImplId()) && (eSource != tZWaveUSB3Sh::EValSrcs::Program))
    {
        TString strId;
        strId.AppendFormatted(c1Id());

        if (m_eSensorType == tZWaveUSB3Sh::EBinSensorTypes::Motion)
        {
            // Motion events are optiinal
            if (bQueryOptVal(kZWaveUSB3Sh::strUOpt_SendTrigger, kCIDLib::True))
            {
                unitiOwner().QueueEventTrig
                (
                    tCQCKit::EStdDrvEvs::Motion
                    , m_strFldName
                    , facCQCKit().strBoolStartEnd(m_pzwcciVal->bState())
                    , strId
                    , strName()
                    , TString::strEmpty()
                );
            }
        }
    }
}


//
//  We override this and see if we need to report an optional triggers option. If our
//  semantic type is motion, then we created a V2 motion field, and motion triggers
//  are optional.
//
tCIDLib::TVoid
TBinNotSensorUnit::QueryUnitAttrs(  tCIDMData::TAttrList&   colToFill
                                    , TAttrData&            adatTmp) const
{
    TParent::QueryUnitAttrs(colToFill, adatTmp);

    // If a motion sensor, then triggers are optional
    if (m_eSensorType == tZWaveUSB3Sh::EBinSensorTypes::Motion)
    {
        adatTmp.Set(L"Motion Options:", TString::strEmpty(), tCIDMData::EAttrTypes::Separator);
        colToFill.objAdd(adatTmp);

        const TZWUnitInfo& unitiUs = unitiOwner();
        adatTmp.Set
        (
            L"Send Triggers"
            , kZWaveUSB3Sh::strUOpt_SendTrigger
            , tCIDMData::EAttrTypes::Bool
        );

        // If no current value, we'll set true as the default
        adatTmp.SetBool(unitiUs.bQueryOptVal(kZWaveUSB3Sh::strUOpt_SendTrigger, kCIDLib::True));
        colToFill.objAdd(adatTmp);
    }
}

