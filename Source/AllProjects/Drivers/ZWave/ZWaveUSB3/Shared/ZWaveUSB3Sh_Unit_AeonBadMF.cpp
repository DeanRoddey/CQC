//
// FILE NAME: ZWaveUSB3Sh_AeonBadMF.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/14/2018
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
//  This is the implementation for a family of very non-standard Aeon Labs multi-
//  sensor.
//
//
//  THIS HANDLER IS obsolete and the new NMCMSensor handler should be used for
//  such things. At some point we'd like to migrate the few things that use this
//  handler to the new one.
//
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
RTTIDecls(TAeonBadMFUnit, TZWaveUnit)



// ---------------------------------------------------------------------------
//   CLASS: TAeonBadMFUnit
//  PREFIX: unit
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TAeonBadMFUnit: Constructors and Destructor
// ---------------------------------------------------------------------------
TAeonBadMFUnit::TAeonBadMFUnit(TZWUnitInfo* const punitiOwner) :

    TZWaveUnit(punitiOwner)
    , m_abSensorFlags(kCIDLib::False)
    , m_azwcciSensors(static_cast<TZWCCImpl*>(nullptr))
    , m_eMotionCC(tZWaveUSB3Sh::ECClasses::Count)
    , m_eMotion2CC(tZWaveUSB3Sh::ECClasses::Count)
    , m_pzwcciMotion(nullptr)
    , m_pzwcciMotion2(nullptr)
{
}

TAeonBadMFUnit::~TAeonBadMFUnit()
{
    // Clean up our CC impls that we created
    tCIDLib::ForEachE<tZWaveUSB3Sh::EAeonMSTypes>
    (
        [this](const auto eIndex)
        {
            try { delete m_azwcciSensors[eIndex]; } catch(...) { }
            m_azwcciSensors[eIndex] = nullptr;
        }
    );

    // Do the motion 2 one as well if needed
    try
    {
        delete m_pzwcciMotion2;
    }

    catch(...)
    {
    }

    m_pzwcciMotion2 = nullptr;
}


// ---------------------------------------------------------------------------
//  TAeonBadMFUnit: Public, inherited methods
// ---------------------------------------------------------------------------

// Just pass it on to our CC impls
tCIDLib::TBoolean TAeonBadMFUnit::bOwnsFld(const tCIDLib::TCard4 c4FldId) const
{
    const tZWaveUSB3Sh::EAeonMSTypes eLast = tCIDLib::eForEachE<tZWaveUSB3Sh::EAeonMSTypes>
    (
        [this, c4FldId](const auto eIndex)
        {
            // If not used, or doesn't own, keep going
            return
            (
                !m_azwcciSensors[eIndex] || !m_azwcciSensors[eIndex]->bOwnsFld(c4FldId)
            );
        }
    );
    return (eLast < tZWaveUSB3Sh::EAeonMSTypes::Count);
}


// We get told by the device info file what sensors we support
tCIDLib::TBoolean TAeonBadMFUnit::bParseExtraInfo()
{
    if (!TParent::bParseExtraInfo())
        return kCIDLib::False;

    // We have to have a list of supported sensors. We might
    TString strTypes;
    if (!bFindExtraVal(L"SensTypes", strTypes))
        return kCIDLib::False;

    // Let's break out the values
    TStringTokenizer stokTypes(&strTypes, L" ");
    tCIDLib::TCard4 c4Count = 0;
    TString strTok;
    while (stokTypes.bGetNextToken(strTok))
    {
        c4Count++;
        if (strTok.bCompareI(L"Humidity"))
            m_abSensorFlags[tZWaveUSB3Sh::EAeonMSTypes::Humidity] = kCIDLib::True;
        else if (strTok.bCompareI(L"Lux"))
            m_abSensorFlags[tZWaveUSB3Sh::EAeonMSTypes::Lux] = kCIDLib::True;
        else if (strTok.bCompareI(L"Motion"))
            m_abSensorFlags[tZWaveUSB3Sh::EAeonMSTypes::Motion] = kCIDLib::True;
        else if (strTok.bCompareI(L"Temp"))
            m_abSensorFlags[tZWaveUSB3Sh::EAeonMSTypes::Temp] = kCIDLib::True;
        else if (strTok.bCompareI(L"UV"))
            m_abSensorFlags[tZWaveUSB3Sh::EAeonMSTypes::UV] = kCIDLib::True;
        else
            return kCIDLib::False;
    }

    if (!c4Count)
        return kCIDLib::False;

    // If motion is supported find out what CC
    if (m_abSensorFlags[tZWaveUSB3Sh::EAeonMSTypes::Motion])
    {
        if (!bFindExtraVal(L"Motion", strTok))
            return kCIDLib::False;

        m_eMotionCC = tZWaveUSB3Sh::eXlatECClasses(strTok);
        if ((m_eMotionCC != tZWaveUSB3Sh::ECClasses::Basic)
        &&  (m_eMotionCC != tZWaveUSB3Sh::ECClasses::BinSensor)
        &&  (m_eMotionCC != tZWaveUSB3Sh::ECClasses::Notification))
        {
            return kCIDLib::False;
        }

        // And do the same for motion 2 if it is set
        if (bFindExtraVal(L"Motion2", strTok))
        {
            m_eMotion2CC = tZWaveUSB3Sh::eXlatECClasses(strTok);
            if ((m_eMotion2CC != tZWaveUSB3Sh::ECClasses::Basic)
            &&  (m_eMotion2CC != tZWaveUSB3Sh::ECClasses::BinSensor)
            &&  (m_eMotion2CC != tZWaveUSB3Sh::ECClasses::Notification))
            {
                return kCIDLib::False;
            }

            // It can't be the same as the main motion
            if (m_eMotion2CC == m_eMotionCC)
                return kCIDLib::False;
        }
    }
    return kCIDLib::True;
}


//  We need to create our impl objects and prep them
tCIDLib::TBoolean TAeonBadMFUnit::bPrepare()
{
    if (!TParent::bPrepare())
        return kCIDLib::False;

    TZWUnitInfo& unitiUs = unitiOwner();
    TString strFldName;

    //
    //  Create and manually set up our CC impls. NOTE WE HAVE TO manually set the
    //  data format stuff before we initialize, so they won't go looking for
    //  extra info that is not there in our case.
    //
    try
    {
        if (m_abSensorFlags[tZWaveUSB3Sh::EAeonMSTypes::Humidity])
        {
            strFldName = unitiUs.strName();
            strFldName.Append(L"_Humidity");
            TZWCCImplMLSensor* pzwcciNew = new TZWCCImplMLSensorF
            (
                &unitiUs, strFldName, tCQCKit::EFldSTypes::HumSensor
            );
            pzwcciNew->strDesc(L"Humidity");
            m_azwcciSensors[tZWaveUSB3Sh::EAeonMSTypes::Humidity] = pzwcciNew;

            // Decide what the range should be
            tCIDLib::TCard4 c4Min = 0;
            tCIDLib::TCard4 c4Max = kCIDLib::c4MaxCard;
            bCheckRange(tZWaveUSB3Sh::EAeonMSTypes::Humidity, c4Min, c4Max);

            pzwcciNew->SetFormat(tCQCKit::EFldTypes::Card, c4Min, c4Max);
            if (!bInitCCImpl(*pzwcciNew))
                return kCIDLib::False;
        }

        if (m_abSensorFlags[tZWaveUSB3Sh::EAeonMSTypes::Lux])
        {
            strFldName = unitiUs.strName();
            strFldName.Append(L"_Luminance");
            TZWCCImplMLSensor* pzwcciNew = new TZWCCImplMLSensorF
            (
                &unitiUs, strFldName, tCQCKit::EFldSTypes::Generic
            );
            pzwcciNew->strDesc(L"Lux");
            m_azwcciSensors[tZWaveUSB3Sh::EAeonMSTypes::Lux] = pzwcciNew;

            // Decide what the range should be
            tCIDLib::TCard4 c4Min = 0;
            tCIDLib::TCard4 c4Max = kCIDLib::c4MaxCard;
            bCheckRange(tZWaveUSB3Sh::EAeonMSTypes::Lux, c4Min, c4Max);

            pzwcciNew->SetFormat(tCQCKit::EFldTypes::Card, c4Min, c4Max);
            if (!bInitCCImpl(*pzwcciNew))
                return kCIDLib::False;
        }

        if (m_abSensorFlags[tZWaveUSB3Sh::EAeonMSTypes::Motion])
        {
            //
            //  Note that we store a second (more typed) pointer to the motion handler
            //  though it is in the list as well.
            //
            m_strMotFldName = L"MOT#";
            m_strMotFldName.Append(unitiUs.strName());
            if (m_eMotionCC == tZWaveUSB3Sh::ECClasses::Basic)
            {
                TZWCCImplBasicSwitchF* pzwcciNew = new TZWCCImplBasicSwitchF
                (
                    &unitiUs, m_strMotFldName, tCQCKit::EFldSTypes::MotionSensor
                );
                m_azwcciSensors[tZWaveUSB3Sh::EAeonMSTypes::Motion] = pzwcciNew;
                m_pzwcciMotion = pzwcciNew;
            }
             else if (m_eMotionCC == tZWaveUSB3Sh::ECClasses::BinSensor)
            {
                TZWCCImplBinSensorF* pzwcciNew = new TZWCCImplBinSensorF
                (
                    &unitiUs, m_strMotFldName, tCQCKit::EFldSTypes::MotionSensor
                );
                m_azwcciSensors[tZWaveUSB3Sh::EAeonMSTypes::Motion] = pzwcciNew;
                m_pzwcciMotion = pzwcciNew;
            }
             else if (m_eMotionCC == tZWaveUSB3Sh::ECClasses::Notification)
            {
                TZWCCImplNotifyStateF* pzwcciNew = new TZWCCImplNotifyStateF
                (
                    &unitiUs, m_strMotFldName, tCQCKit::EFldSTypes::MotionSensor
                );
                m_azwcciSensors[tZWaveUSB3Sh::EAeonMSTypes::Motion] = pzwcciNew;
                m_pzwcciMotion = pzwcciNew;
            }
             else
            {
                CIDAssert2(L"Unknown Aeon multi-sensor motion CC");
            }

            m_azwcciSensors[tZWaveUSB3Sh::EAeonMSTypes::Motion]->strDesc(L"Motion");
            if (!bInitCCImpl(*m_azwcciSensors[tZWaveUSB3Sh::EAeonMSTypes::Motion]))
                return kCIDLib::False;
        }

        if (m_abSensorFlags[tZWaveUSB3Sh::EAeonMSTypes::Temp])
        {
            // Get the temp range option and use that to set the range
            const TString& strScale = unitiUs.strQueryOptVal
            (
                kZWaveUSB3Sh::strUOpt_TempScale, L"F"
            );
            const tCIDLib::TBoolean bFScale(strScale == L"F");

            tCIDLib::TInt4 i4LowTemp, i4HighTemp;
            if (bFScale)
            {
                i4LowTemp = -25;
                i4HighTemp = 100;
            }
             else
            {
                i4LowTemp = -28;
                i4HighTemp = 50;
            }


            strFldName = unitiUs.strName();
            strFldName.Append(L"_Temp");
            TZWCCImplMLSensor* pzwcciNew = new TZWCCImplMLSensorF
            (
                &unitiUs, strFldName, tCQCKit::EFldSTypes::CurTemp
            );
            m_azwcciSensors[tZWaveUSB3Sh::EAeonMSTypes::Temp] = pzwcciNew;
            pzwcciNew->SetFormat
            (
                tCQCKit::EFldTypes::Int
                , i4LowTemp
                , i4HighTemp
                , bFScale ? tZWaveUSB3Sh::ETScales::F : tZWaveUSB3Sh::ETScales::C
            );
            pzwcciNew->strDesc(L"Temp");
            if (!bInitCCImpl(*pzwcciNew))
                return kCIDLib::False;
        }

        if (m_abSensorFlags[tZWaveUSB3Sh::EAeonMSTypes::UV])
        {
            strFldName = unitiUs.strName();
            strFldName.Append(L"_UV");
            TZWCCImplMLSensor* pzwcciNew = new TZWCCImplMLSensorF
            (
                &unitiUs, strFldName, tCQCKit::EFldSTypes::Generic
            );
            pzwcciNew->strDesc(L"UV");
            m_azwcciSensors[tZWaveUSB3Sh::EAeonMSTypes::Lux] = pzwcciNew;

            //
            //  Decide what the range should be. 20 is a monster level, so we do that
            //  as a worst case.
            //
            tCIDLib::TCard4 c4Min = 1;
            tCIDLib::TCard4 c4Max = 20;
            bCheckRange(tZWaveUSB3Sh::EAeonMSTypes::UV, c4Min, c4Max);

            pzwcciNew->SetFormat(tCQCKit::EFldTypes::Card, c4Min, c4Max);
            if (!bInitCCImpl(*pzwcciNew))
                return kCIDLib::False;
        }

        // If there's a motion2, deal with that.
        if (m_eMotion2CC != tZWaveUSB3Sh::ECClasses::Count)
        {
            if (m_eMotion2CC == tZWaveUSB3Sh::ECClasses::Basic)
            {
                m_pzwcciMotion2 = new TZWCCImplBasicSwitch(&unitiUs);
            }
             else if (m_eMotion2CC == tZWaveUSB3Sh::ECClasses::BinSensor)
            {
                m_pzwcciMotion2 = new TZWCCImplBinSensor(&unitiUs);
            }
             else if (m_eMotion2CC == tZWaveUSB3Sh::ECClasses::Notification)
            {
                //
                //  We don't force any event ids here. We let this be set in the
                //  device info class. The CC impl will parse that stuff out.
                //
                TZWCCImplNotifyState* pzwcciNew = new TZWCCImplNotifyState(&unitiUs);
                m_pzwcciMotion2 = pzwcciNew;
            }

            if (m_pzwcciMotion2)
            {
                m_pzwcciMotion2->strDesc(L"Motion2");
                if (!bInitCCImpl(*m_pzwcciMotion2))
                    return kCIDLib::False;
            }
        }


        //
        //  WE ALSO have to set our own pseudo impl ids since we don't add these to our
        //  parent class, we manage them ourself. There could be some set on our parent
        //  class for standard stuff like battery and such. So we take a fairly large
        //  value as our starting point.
        //
        tZWaveUSB3Sh::EAeonMSTypes eType = tZWaveUSB3Sh::EAeonMSTypes::Min;
        tCIDLib::TCard4 c4FauxId = 128;
        while (eType <= tZWaveUSB3Sh::EAeonMSTypes::Max)
        {
            if (m_azwcciSensors[eType])
                m_azwcciSensors[eType]->c4ImplId(c4FauxId++);
            eType++;
        }

        // Do the alt motion one as well if we have it
        if (m_pzwcciMotion2)
            m_pzwcciMotion2->c4ImplId(c4FauxId++);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);
        return kCIDLib::False;
    }

    catch(...)
    {
        return kCIDLib::False;
    }

    return kCIDLib::True;
}


// Since we manage some of our own CCs, we have to override this
tCIDLib::TVoid TAeonBadMFUnit::FormatCCsInfo(TTextOutStream& strmTar) const
{
    tZWaveUSB3Sh::EAeonMSTypes eType = tZWaveUSB3Sh::EAeonMSTypes::Min;
    while (eType <= tZWaveUSB3Sh::EAeonMSTypes::Max)
    {
        if (m_azwcciSensors[eType])
            m_azwcciSensors[eType]->FormatReport(strmTar);
        eType++;
    }

    // Do the alt motion one as well if we have it
    if (m_pzwcciMotion2)
        m_pzwcciMotion2->FormatReport(strmTar);
}


// We have to manually handle distributing values to our CC impls
tCIDLib::TVoid
TAeonBadMFUnit::HandleCCMsg(const   tCIDLib::TCard1 c1Class
                            , const tCIDLib::TCard1 c1Cmd
                            , const TZWInMsg&       zwimIn)
{
    TParent::HandleCCMsg(c1Class, c1Cmd, zwimIn);

    if (c1Class == COMMAND_CLASS_BASIC)
    {
        if (m_abSensorFlags[tZWaveUSB3Sh::EAeonMSTypes::Motion])
        {
            if (m_eMotionCC == tZWaveUSB3Sh::ECClasses::Basic)
            {
                m_azwcciSensors[tZWaveUSB3Sh::EAeonMSTypes::Motion]->ProcessCCMsg(c1Class, c1Cmd, zwimIn);
            }
            else if (m_eMotion2CC == tZWaveUSB3Sh::ECClasses::Basic)
            {
                CIDAssert(m_pzwcciMotion2 != nullptr, L"Motion2 CC is set but no basic impl object");
                m_pzwcciMotion2->ProcessCCMsg(c1Class, c1Cmd, zwimIn);
            }
        }
    }
     else if (c1Class == COMMAND_CLASS_SENSOR_BINARY)
    {
        if (m_abSensorFlags[tZWaveUSB3Sh::EAeonMSTypes::Motion])
        {
            if (m_eMotionCC == tZWaveUSB3Sh::ECClasses::BinSensor)
            {
                m_azwcciSensors[tZWaveUSB3Sh::EAeonMSTypes::Motion]->ProcessCCMsg(c1Class, c1Cmd, zwimIn);
            }
             else if (m_eMotion2CC == tZWaveUSB3Sh::ECClasses::BinSensor)
            {
                CIDAssert(m_pzwcciMotion2 != nullptr, L"Motion2 CC is set but no binsensor impl object");
                m_pzwcciMotion2->ProcessCCMsg(c1Class, c1Cmd, zwimIn);
            }
        }
    }
    else if (c1Class == COMMAND_CLASS_NOTIFICATION_V3)
    {
        //
        //  If it's a report and has enough bytes to not be a V1 or V2 report, then
        //  it's one of the standardized ones as of V3.
        //
        if ((c1Cmd == NOTIFICATION_REPORT_V3)
        &&  (zwimIn.c4CCBytes() >= 8))
        {
            if (m_eMotionCC == tZWaveUSB3Sh::ECClasses::Notification)
            {
                m_azwcciSensors[tZWaveUSB3Sh::EAeonMSTypes::Motion]->ProcessCCMsg(c1Class, c1Cmd, zwimIn);
            }
             else if (m_eMotion2CC == tZWaveUSB3Sh::ECClasses::Notification)
            {
                CIDAssert(m_pzwcciMotion2 != nullptr, L"Motion2 CC is set but no notification impl object");
                m_pzwcciMotion2->ProcessCCMsg(c1Class, c1Cmd, zwimIn);
            }
        }
    }
     else if (c1Class == COMMAND_CLASS_SENSOR_MULTILEVEL)
    {
        // These are special case and have to be routed based on the sensor type
        const tCIDLib::TCard1 c1Type = zwimIn.c1CCByteAt(2);
        if (c1Type == 1)
        {
            m_azwcciSensors[tZWaveUSB3Sh::EAeonMSTypes::Temp]->ProcessCCMsg
            (
                c1Class, c1Cmd, zwimIn
            );
        }
         else if (c1Type == 3)
        {
            m_azwcciSensors[tZWaveUSB3Sh::EAeonMSTypes::Lux]->ProcessCCMsg
            (
                c1Class, c1Cmd, zwimIn
            );
        }
         else if (c1Type == 5)
        {
            m_azwcciSensors[tZWaveUSB3Sh::EAeonMSTypes::Humidity]->ProcessCCMsg
            (
                c1Class, c1Cmd, zwimIn
            );
        }
         else if (c1Type == 0x1B)
        {
            m_azwcciSensors[tZWaveUSB3Sh::EAeonMSTypes::UV]->ProcessCCMsg
            (
                c1Class, c1Cmd, zwimIn
            );
        }
         else if (c1Type == 7)
        {
            if (m_abSensorFlags[tZWaveUSB3Sh::EAeonMSTypes::Motion])
            {
                if (m_eMotionCC == tZWaveUSB3Sh::ECClasses::Notification)
                {
                    m_azwcciSensors[tZWaveUSB3Sh::EAeonMSTypes::Motion]->ProcessCCMsg
                    (
                        c1Class, c1Cmd, zwimIn
                     );
                }
                 else if (m_eMotion2CC == tZWaveUSB3Sh::ECClasses::Notification)
                {
                    CIDAssert(m_pzwcciMotion2 != nullptr, L"Motion2 CC is set but no notification impl object");
                    m_pzwcciMotion2->ProcessCCMsg(c1Class, c1Cmd, zwimIn);
                }
            }
        }
    }
}


//
//  WE have to watch for changes from either of the motion impls. We have to keep the
//  two of them in sync, and we have to send a motion event if we are configured to.
//
tCIDLib::TVoid
TAeonBadMFUnit::ImplValueChanged(const   tCIDLib::TCard4        c4ImplId
                                , const tZWaveUSB3Sh::EValSrcs  eSource
                                , const tCIDLib::TBoolean       bWasInErr
                                , const tCIDLib::TInt4          )
{
    //
    //  Ignore program source changes, we caused that ourself. When we force one
    //  motion impl with the new value from the other one that will call us back here
    //  but this will prevent actual recursion.
    //
    if (eSource == tZWaveUSB3Sh::EValSrcs::Program)
        return;

    tCIDLib::TBoolean bChange = kCIDLib::False;
    if (m_pzwcciMotion && (c4ImplId == m_pzwcciMotion->c4ImplId()))
    {
        // This one is always a change, and we update 2 to match us if we have one
        bChange = kCIDLib::True;
        if (m_pzwcciMotion2)
        {
            m_pzwcciMotion2->bSetBinState
            (
                m_pzwcciMotion->bState(), tZWaveUSB3Sh::EValSrcs::Program
            );
        }
    }
     else if (m_pzwcciMotion2 && (c4ImplId == m_pzwcciMotion2->c4ImplId()))
    {
        // If this changes our main motion guy's state, it's a change
        bChange = m_pzwcciMotion->bSetBinState
        (
            m_pzwcciMotion2->bState(), tZWaveUSB3Sh::EValSrcs::Program
        );
    }
     else
    {
        // Not one we care about here
        return;
    }

    //
    //  If a change we care about, we are configured to send triggers, then send a
    //  motion trigger. We know we created a V2 motion field.
    //
    if (bChange && bQueryOptVal(kZWaveUSB3Sh::strUOpt_SendTrigger, kCIDLib::True))
    {
        TString strId;
        strId.AppendFormatted(c1Id());

        unitiOwner().QueueEventTrig
        (
            tCQCKit::EStdDrvEvs::Motion
            , m_strMotFldName
            , facCQCKit().strBoolStartEnd(m_pzwcciMotion->bState())
            , strId
            , strName()
            , TString::strEmpty()
        );
    }
}



// Just pass it on to our CC impls
tCIDLib::TVoid
TAeonBadMFUnit::QueryFldDefs(tCQCKit::TFldDefList& colAddTo, TCQCFldDef& flddTmp) const
{
    TParent::QueryFldDefs(colAddTo, flddTmp);

    tCIDLib::ForEachE<tZWaveUSB3Sh::EAeonMSTypes>
    (
        [this, &colAddTo, &flddTmp](const auto eIndex)
        {
            if (m_azwcciSensors[eIndex])
                m_azwcciSensors[eIndex]->QueryFldDefs(colAddTo, flddTmp);
            return kCIDLib::True;
        }
    );
}


tCIDLib::TVoid
TAeonBadMFUnit::QueryUnitAttrs( tCIDMData::TAttrList&   colToFill
                                , TAttrData&            adatTmp) const
{
    TParent::QueryUnitAttrs(colToFill, adatTmp);

    const TZWUnitInfo& unitiUs = unitiOwner();

    //
    //  Motion triggers are optional If no current value, we'll set true as the
    //  default.
    //
    if (m_abSensorFlags[tZWaveUSB3Sh::EAeonMSTypes::Motion])
    {
        adatTmp.Set(L"Motion Options:", TString::strEmpty(), tCIDMData::EAttrTypes::Separator);
        colToFill.objAdd(adatTmp);

        adatTmp.Set
        (
            L"Send Triggers"
            , kZWaveUSB3Sh::strUOpt_SendTrigger
            , tCIDMData::EAttrTypes::Bool
        );
        adatTmp.SetBool(unitiUs.bQueryOptVal(kZWaveUSB3Sh::strUOpt_SendTrigger, kCIDLib::True));
        colToFill.objAdd(adatTmp);
    }

    // We need to know what temp scale they want us to use if temps are supported
    if (m_abSensorFlags[tZWaveUSB3Sh::EAeonMSTypes::Temp])
    {
        adatTmp.Set(L"Temp Options:", TString::strEmpty(), tCIDMData::EAttrTypes::Separator);
        colToFill.objAdd(adatTmp);

        adatTmp.Set
        (
            L"Temp Scale"
            , kZWaveUSB3Sh::strUOpt_TempScale
            , L"Enum: F, C"
            , tCIDMData::EAttrTypes::String
        );
        adatTmp.SetString
        (
            unitiUs.strQueryOptVal(kZWaveUSB3Sh::strUOpt_TempScale, L"F")
        );
        colToFill.objAdd(adatTmp);
    }
}


// We have to override this since we manage our own impls
tCIDLib::TVoid TAeonBadMFUnit::Process()
{
tCIDLib::ForEachE<tZWaveUSB3Sh::EAeonMSTypes>
    (
        [this](const auto eIndex)
        {
            if (m_azwcciSensors[eIndex])
                m_azwcciSensors[eIndex]->Process();
            return kCIDLib::True;
        }
    );
}

// Just pass on to our CC impls
tCIDLib::TVoid TAeonBadMFUnit::StoreFldIds()
{
    TParent::StoreFldIds();

    tCIDLib::ForEachE<tZWaveUSB3Sh::EAeonMSTypes>
    (
        [this](const auto eIndex)
        {
            if (m_azwcciSensors[eIndex])
                m_azwcciSensors[eIndex]->StoreFldIds();
            return kCIDLib::True;
        }
    );
}


// Just pass on to our CC impls
tCIDLib::TVoid TAeonBadMFUnit::UnitIsOnline()
{
    TParent::UnitIsOnline();

    tCIDLib::ForEachE<tZWaveUSB3Sh::EAeonMSTypes>
    (
        [this](const auto eIndex)
        {
            if (m_azwcciSensors[eIndex])
                m_azwcciSensors[eIndex]->UnitIsOnline();
            return kCIDLib::True;
        }
    );
}



// ---------------------------------------------------------------------------
//  TAeonBadMFUnit: Private, non-virtaal methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TAeonBadMFUnit::bCheckRange(const   tZWaveUSB3Sh::EAeonMSTypes  eType
                            ,       tCIDLib::TCard4&            c4Min
                            ,       tCIDLib::TCard4&            c4Max)
{
    TString strKey(L"Range_", 16UL);
    strKey.Append(tZWaveUSB3Sh::strXlatEAeonMSTypes(eType));

    TString strMin;
    if (!bFindExtraVal(strKey, strMin))
        return kCIDLib::False;

    TString strMax;
    if (!strMin.bSplit(strMax, kCIDLib::chSpace))
        return kCIDLib::False;

    if (!strMin.bToCard4(c4Min, tCIDLib::ERadices::Auto)
    ||  !strMax.bToCard4(c4Max, tCIDLib::ERadices::Auto))
    {
        return kCIDLib::False;
    }
    return kCIDLib::True;
}
