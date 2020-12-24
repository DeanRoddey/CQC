//
// FILE NAME: ZWaveUSB3Sh_DualBinSensor.cpp
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
//  This is the implementation for a semi-generic, non-multi-channel compliant, dual
//  binary sensor, see the header class comments.
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
RTTIDecls(TDualBinSensorUnit, TZWaveUnit)




// ---------------------------------------------------------------------------
//   CLASS: TDualBinSensorUnit
//  PREFIX: unit
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TDualBinSensorUnit: Constructors and Destructor
// ---------------------------------------------------------------------------
TDualBinSensorUnit::TDualBinSensorUnit(TZWUnitInfo* const punitiOwner) :

    TZWaveUnit(punitiOwner)
    , m_eHandlerClass1(tZWaveUSB3Sh::ECClasses::None)
    , m_eHandlerClass2(tZWaveUSB3Sh::ECClasses::None)
    , m_eSensorType1(tZWaveUSB3Sh::EBinSensorTypes::Count)
    , m_eSensorType2(tZWaveUSB3Sh::EBinSensorTypes::Count)
    , m_pzwcciVal1(nullptr)
    , m_pzwcciVal2(nullptr)
{
}

TDualBinSensorUnit::~TDualBinSensorUnit()
{
}


// ---------------------------------------------------------------------------
//  TDualBinSensorUnit: Public, inherited methods
// ---------------------------------------------------------------------------

// Just pass it on to our CC impls
tCIDLib::TBoolean TDualBinSensorUnit::bOwnsFld(const tCIDLib::TCard4 c4FldId) const
{
    return m_pzwcciVal1->bOwnsFld(c4FldId) || m_pzwcciVal2->bOwnsFld(c4FldId);
}


// We parse our extra info and store the data away for later use
tCIDLib::TBoolean TDualBinSensorUnit::bParseExtraInfo()
{
    if (!TParent::bParseExtraInfo())
        return kCIDLib::False;

    TZWUnitInfo& unitiUs = unitiOwner();

    // Figure out what CCs to use
    m_eHandlerClass1 = tZWaveUSB3Sh::ECClasses::None;
    m_eHandlerClass2 = tZWaveUSB3Sh::ECClasses::None;
    m_eSensorType1 = tZWaveUSB3Sh::EBinSensorTypes::Count;
    m_eSensorType2 = tZWaveUSB3Sh::EBinSensorTypes::Count;

    TString strHandlerClass1;
    TString strHandlerClass2;
    if (!bFindExtraVal(L"Class1", strHandlerClass1)
    ||  !bFindExtraVal(L"Class2", strHandlerClass2))
    {
        if (facZWaveUSB3Sh().bLowTrace())
        {
            facZWaveUSB3Sh().LogTraceErr
            (
                L"Missing handler class info for unit %(1)", unitiUs.strName()
            );
        }
        return kCIDLib::False;
    }

    m_eHandlerClass1 = tZWaveUSB3Sh::eXlatECClasses(strHandlerClass1);
    m_eHandlerClass2 = tZWaveUSB3Sh::eXlatECClasses(strHandlerClass2);

    // These cannot be the same since we can only distinguish them via CC
    if (m_eHandlerClass1 == m_eHandlerClass2)
    {
        if (facZWaveUSB3Sh().bLowTrace())
        {
            facZWaveUSB3Sh().LogTraceErr
            (
                L"Both handler classes we the same for dual sensors unit: %(1)", unitiUs.strName()
            );
        }
        return kCIDLib::False;
    }

    //
    //  Get the type of the sensor, which isn't a CQC driver semantic type but types
    //  of sensors we can understand.
    //
    TString strType1;
    TString strType2;
    if (!bFindExtraVal(L"Type1", strType1) || !bFindExtraVal(L"Type2", strType2))
    {
        if (facZWaveUSB3Sh().bLowTrace())
            facZWaveUSB3Sh().LogTraceErr(L"Missing sensor type for unit %(1)", unitiUs.strName());
        return kCIDLib::False;
    }

    m_eSensorType1 = tZWaveUSB3Sh::eXlatEBinSensorTypes(strType1);
    m_eSensorType2 = tZWaveUSB3Sh::eXlatEBinSensorTypes(strType2);
    if ((m_eSensorType1 == tZWaveUSB3Sh::EBinSensorTypes::Count)
    ||  (m_eSensorType2 == tZWaveUSB3Sh::EBinSensorTypes::Count))
    {
        if (facZWaveUSB3Sh().bLowTrace())
        {
            facZWaveUSB3Sh().LogTraceErr
            (
                L"Bad sensor type for unit %(1), value=", unitiUs.strName()
                , (m_eSensorType1 == tZWaveUSB3Sh::EBinSensorTypes::Count) ? strType1 : strType2
            );
        }
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


// We need to create our impl objects
tCIDLib::TBoolean TDualBinSensorUnit::bPrepare()
{
    if (!TParent::bPrepare())
        return kCIDLib::False;

    TZWUnitInfo& unitiUs = unitiOwner();

    // This is repetitive so we call a helper
    if (!bMakeSensorCC(m_eHandlerClass1, m_eSensorType1, m_strFldName1, m_pzwcciVal1, kCIDLib::True))
        return kCIDLib::False;
    if (!bMakeSensorCC(m_eHandlerClass2, m_eSensorType2, m_strFldName2, m_pzwcciVal2, kCIDLib::False))
        return kCIDLib::False;

    bInitCCImpl(*m_pzwcciVal1);
    bInitCCImpl(*m_pzwcciVal2);
    return kCIDLib::True;
}


// We manage our impls ourself, so we have to override this
tCIDLib::TVoid
TDualBinSensorUnit::FormatCCsInfo(TTextOutStream& strmTar) const
{
    m_pzwcciVal1->FormatReport(strmTar);
    m_pzwcciVal2->FormatReport(strmTar);
}


// We have to manually handle distributing values to our CC impls
tCIDLib::TVoid
TDualBinSensorUnit::HandleCCMsg(const   tCIDLib::TCard1 c1Class
                                , const tCIDLib::TCard1 c1Cmd
                                , const TZWInMsg&       zwimIn)
{
    TParent::HandleCCMsg(c1Class, c1Cmd, zwimIn);

    if (c1Class == COMMAND_CLASS_BASIC)
    {
        if (m_eHandlerClass1 == tZWaveUSB3Sh::ECClasses::Basic)
            m_pzwcciVal1->ProcessCCMsg(c1Class, c1Cmd, zwimIn);
        else if (m_eHandlerClass2 == tZWaveUSB3Sh::ECClasses::Basic)
            m_pzwcciVal2->ProcessCCMsg(c1Class, c1Cmd, zwimIn);
    }
     else if (c1Class == COMMAND_CLASS_SENSOR_BINARY)
    {
        if (m_eHandlerClass1 == tZWaveUSB3Sh::ECClasses::BinSensor)
            m_pzwcciVal1->ProcessCCMsg(c1Class, c1Cmd, zwimIn);
        else if (m_eHandlerClass2 == tZWaveUSB3Sh::ECClasses::BinSensor)
            m_pzwcciVal2->ProcessCCMsg(c1Class, c1Cmd, zwimIn);
    }
     else if (c1Class == COMMAND_CLASS_NOTIFICATION_V3)
    {
        //
        //  If it's a report and has enough bytes to not be a V1 or V2 report, then
        //  it's one of the standardized ones as of V3.
        //
        if ((c1Cmd == NOTIFICATION_REPORT_V3) && (zwimIn.c4CCBytes() >= 8))
        {
            if (m_eHandlerClass1 == tZWaveUSB3Sh::ECClasses::Notification)
                m_pzwcciVal1->ProcessCCMsg(c1Class, c1Cmd, zwimIn);
            else if (m_eHandlerClass2 == tZWaveUSB3Sh::ECClasses::Notification)
                m_pzwcciVal2->ProcessCCMsg(c1Class, c1Cmd, zwimIn);
        }
    }
}



//
//  If we get a value change from our impl and we are a motion sensor, and we are
//  configured to send triggers, then we need to do that.
//
tCIDLib::TVoid
TDualBinSensorUnit::ImplValueChanged(const  tCIDLib::TCard4         c4ImplId
                                    , const tZWaveUSB3Sh::EValSrcs  eSource
                                    , const tCIDLib::TBoolean       bWasInErr
                                    , const tCIDLib::TInt4          )
{
    //
    //  If not a programmatic change, and for one of our value CCs, see if we need to
    //  send a motion event
    //
    if ((eSource != tZWaveUSB3Sh::EValSrcs::Program)
    &&  ((c4ImplId == m_pzwcciVal1->c4ImplId()) || (c4ImplId == m_pzwcciVal2->c4ImplId())))
    {
        // Figure out which one changed
        tCIDLib::TCard4 c4WhichSensor = kCIDLib::c4MaxCard;
        tZWaveUSB3Sh::EBinSensorTypes   eSensorType;
        TString                         strFldName;
        TZWCCImplBin*                   pzwcciVal = nullptr;
        if (m_pzwcciVal1->c4ImplId() == c4ImplId)
        {
            c4WhichSensor = 1;
            eSensorType = m_eSensorType1;
            strFldName = m_strFldName1;
            pzwcciVal = m_pzwcciVal1;
        }
         else if (m_pzwcciVal2->c4ImplId() == c4ImplId)
        {
            c4WhichSensor = 2;
            eSensorType = m_eSensorType2;
            strFldName = m_strFldName2;
            pzwcciVal = m_pzwcciVal2;
        }

        if (c4WhichSensor != kCIDLib::c4MaxCard)
        {
            TString strId;
            strId.AppendFormatted(c1Id());

            if (eSensorType == tZWaveUSB3Sh::EBinSensorTypes::Motion)
            {
                // Motion events are optional
                if (bQueryOptVal(kZWaveUSB3Sh::strUOpt_SendTrigger, kCIDLib::True))
                {
                    unitiOwner().QueueEventTrig
                    (
                        tCQCKit::EStdDrvEvs::Motion
                        , strFldName
                        , facCQCKit().strBoolStartEnd(pzwcciVal->bState())
                        , strId
                        , strName()
                        , TString::strEmpty()
                    );
                }
            }
        }
    }
}


// Just pass it on to our CC impls
tCIDLib::TVoid
TDualBinSensorUnit::QueryFldDefs(tCQCKit::TFldDefList& colAddTo, TCQCFldDef& flddTmp) const
{
    TParent::QueryFldDefs(colAddTo, flddTmp);

    m_pzwcciVal1->QueryFldDefs(colAddTo, flddTmp);
    m_pzwcciVal2->QueryFldDefs(colAddTo, flddTmp);
}


//
//  We override this and see if we need to report an optional triggers option. If our
//  semantic type is motion, then we created a V2 motion field, and motion triggers
//  are optional.
//
tCIDLib::TVoid
TDualBinSensorUnit::QueryUnitAttrs( tCIDMData::TAttrList&   colToFill
                                    , TAttrData&            adatTmp) const
{
    TParent::QueryUnitAttrs(colToFill, adatTmp);

    // If a motion sensor, then triggers are optional
    if ((m_eSensorType1 == tZWaveUSB3Sh::EBinSensorTypes::Motion)
    ||  (m_eSensorType2 == tZWaveUSB3Sh::EBinSensorTypes::Motion))
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


// We have to override this since we manage our own cc impls
tCIDLib::TVoid TDualBinSensorUnit::Process()
{
    m_pzwcciVal1->Process();
    m_pzwcciVal2->Process();
}


// Just pass on to our CC impls
tCIDLib::TVoid TDualBinSensorUnit::StoreFldIds()
{
    TParent::StoreFldIds();

    m_pzwcciVal1->StoreFldIds();
    m_pzwcciVal2->StoreFldIds();
}


// Just pass on to our CC impls
tCIDLib::TVoid TDualBinSensorUnit::UnitIsOnline()
{
    TParent::UnitIsOnline();

    m_pzwcciVal1->UnitIsOnline();
    m_pzwcciVal2->UnitIsOnline();
}



// ---------------------------------------------------------------------------
//  TDualBinSensorUnit: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  We have some redundant setup work to do, so this is called to do the work. We use
//  the info gotten during the extra info parse, which is passed to us. We get the
//  values for one of the sensors and pass back the info we created for it.
//
tCIDLib::TBoolean
TDualBinSensorUnit::bMakeSensorCC(  const   tZWaveUSB3Sh::ECClasses         eHandlerClass
                                    , const tZWaveUSB3Sh::EBinSensorTypes&  eSensorType
                                    ,       TString&                        strFldName
                                    ,       TZWCCImplBin*&                  pzwcciVal
                                    , const tCIDLib::TBoolean               bFirst)
{
    TZWUnitInfo& unitiUs = unitiOwner();

    if (eSensorType == tZWaveUSB3Sh::EBinSensorTypes::Motion)
    {
        strFldName = L"MOT#";
        strFldName.Append(unitiUs.strName());
    }
     else
    {
        strFldName = unitiUs.strName();
        strFldName.Append(L"_State");
        strFldName.Append(bFirst ? L"1": L"2");
    }

    // Set the semantic type based on the sensor type
    const tCQCKit::EFldSTypes eSemType
    (
        (eSensorType == tZWaveUSB3Sh::EBinSensorTypes::Motion) ? tCQCKit::EFldSTypes::MotionSensor
                                                             : tCQCKit::EFldSTypes::Generic
    );

    //
    //  OK, based on the representation type, we gen up the correct impl objects
    //
    //  These are all assumed to be read only since they are sensors. So any CCs that
    //  can be read/write, we set them to read only.
    //
    //  If notification, he will look for the notification oriented extra info that
    //  is required.
    //
    pzwcciVal = nullptr;
    if (eHandlerClass == tZWaveUSB3Sh::ECClasses::Basic)
    {
        pzwcciVal = new TZWCCImplBasicSwitchF(&unitiUs, strFldName, eSemType);
    }
     else if (eHandlerClass == tZWaveUSB3Sh::ECClasses::BinSensor)
    {
        pzwcciVal = new TZWCCImplBinSensorF(&unitiUs, strFldName, eSemType);
    }
     else if (eHandlerClass == tZWaveUSB3Sh::ECClasses::Notification)
    {
        pzwcciVal = new TZWCCImplNotifyStateF(&unitiUs, strFldName, eSemType);
    }
     else
    {
        facZWaveUSB3Sh().LogTraceErr
        (
            L"Unknown handler class (%(1)) for unit %(2)"
            , tZWaveUSB3Sh::strXlatECClasses(eHandlerClass)
            , unitiUs.strName()
        );
        return kCIDLib::False;
    }

    pzwcciVal->strDesc(tZWaveUSB3Sh::strXlatEBinSensorTypes(eSensorType));
    return kCIDLib::True;
}
