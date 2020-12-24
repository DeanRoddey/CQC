//
// FILE NAME: ZWaveUSB3Sh_CCImpl_MLSensor.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/15/2018
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
//  This is the implementation for the multi-level switch command class namespace.
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
//  Magic maros
// ---------------------------------------------------------------------------
RTTIDecls(TZWCCImplMLSensor, TZWCCImpl)
RTTIDecls(TZWCCImplMLSensorF, TZWCCImplMLSensor)



// ---------------------------------------------------------------------------
//   CLASS: TZWCCImplMLSensor
//  PREFIX: zwcci
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWCCImplMLSensor: Public, static methods
// ---------------------------------------------------------------------------

//
//  Annoyingly, Z-Wave uses the same format as the ML sensor for other CCs, instead
//  of using using the ML sensor. It's way too much trickery to want to replicate so
//  we provide this helper to extract the information from an incoming msg.
//
//  The other CC may not place the values as the same offset, so we get an offset
//  value to expect the encoded precision/scale/size byte to be at. Then we get size
//  number of value bytes.
//
//  We return the raw unsigned value. The caller can cast it if required to signed.
//
tCIDLib::TBoolean
TZWCCImplMLSensor::bExtractInfo(const   TZWInMsg&           zwimSrc
                                , const tCIDLib::TCard4     c4Ofs
                                ,       tCIDLib::TCard4&    c4Precision
                                ,       tCIDLib::TCard4&    c4Scale
                                ,       tCIDLib::TCard4&    c4Bytes
                                ,       tCIDLib::TCard4&    c4RawVal)
{
    //
    //  We just care bout the precision and number of bytes. That's enough to get
    //  the value out.
    //
    c4Precision = (zwimSrc.c1CCByteAt(c4Ofs) >> 5) & 0x7;
    c4Bytes = zwimSrc.c1CCByteAt(c4Ofs) & 0x7;
    c4Scale = (zwimSrc.c1CCByteAt(c4Ofs) >> 3) & 0x3;

    if (c4Bytes == 1)
    {
        c4RawVal = zwimSrc.c1CCByteAt(c4Ofs + 1);
    }
     else if (c4Bytes == 2)
    {
        c4RawVal = zwimSrc.c1CCByteAt(c4Ofs + 1);
        c4RawVal <<= 8;
        c4RawVal |= zwimSrc.c1CCByteAt(c4Ofs + 2);
    }
     else if (c4Bytes == 4)
    {
        c4RawVal = zwimSrc.c1CCByteAt(c4Ofs + 1);
        c4RawVal <<= 8;
        c4RawVal |= zwimSrc.c1CCByteAt(c4Ofs + 2);
        c4RawVal <<= 8;
        c4RawVal |= zwimSrc.c1CCByteAt(c4Ofs + 3);
        c4RawVal <<= 8;
        c4RawVal |= zwimSrc.c1CCByteAt(c4Ofs + 4);
    }
     else
    {
        // Shouldn't happen, but just in case
        return kCIDLib::True;
    }
    return kCIDLib::True;
}


// ---------------------------------------------------------------------------
//  TZWCCImplMLSensor: Constructors and Destructor
// ---------------------------------------------------------------------------

// Take the default access bits set by the base class
TZWCCImplMLSensor::
TZWCCImplMLSensor(          TZWUnitInfo* const  punitiThis
                    , const tCIDLib::TCard1     c1EPId) :

    TZWCCImpl(punitiThis, tZWaveUSB3Sh::ECClasses::MLSensor, c1EPId)
    , m_bError(kCIDLib::True)
    , m_bFmtForced(kCIDLib::False)
    , m_c4Level(0)
    , m_eTempScale(tZWaveUSB3Sh::ETScales::Actual)
    , m_eValType(tCQCKit::EFldTypes::Card)
    , m_f8Level(0)
    , m_f8MinRange(0)
    , m_f8MaxRange(100)
{
}

TZWCCImplMLSensor::~TZWCCImplMLSensor()
{
}


// ---------------------------------------------------------------------------
//  TZWCCImplMLSensor: Public, inherited methods
// ---------------------------------------------------------------------------

// If we haven't had the form forced on us, we get it from the extra info
tCIDLib::TBoolean TZWCCImplMLSensor::bParseExtraInfo()
{
    if (!TParent::bParseExtraInfo())
        return kCIDLib::False;

    if (!m_bFmtForced)
    {
        const TZWUnitInfo& unitiUs = unitiOwner();

        TString strType;
        TString strMin;
        TString strMax;
        if (!bFindExtraVal(L"Type", strType)
        ||  !bFindExtraVal(L"RangeMin", strMin)
        ||  !bFindExtraVal(L"RangeMax", strMax))
        {
            return kCIDLib::False;
        }

        m_eValType = tCQCKit::eXlatEFldTypes(strType);
        if (m_eValType == tCQCKit::EFldTypes::Count)
            return kCIDLib::False;

        if (!strMin.bToFloat8(m_f8MinRange) || !strMin.bToFloat8(m_f8MaxRange))
            return kCIDLib::False;
    }
    return kCIDLib::True;
}


// Queues up an async query for the sensor's value
tCIDLib::TVoid TZWCCImplMLSensor::SendValueQuery()
{
    // Sanity check
    if (!bIsReadable())
    {
        facZWaveUSB3Sh().LogTraceErr
        (
            L"SendValueQuery() called for non-readable ML sensor %(1)/%(2)"
            , strUnitName()
            , strDesc()
        );
        return;
    }

    try
    {
        TZWOutMsg zwomOut;
        facZWaveUSB3Sh().BuildCCMsg
        (
            zwomOut
            , unitiOwner()
            , COMMAND_CLASS_SENSOR_MULTILEVEL
            , SENSOR_MULTILEVEL_GET
            , 0
            , tZWaveUSB3Sh::EMsgPrios::Async
            , kCIDLib::False
        );
        SendMsgAsync(zwomOut);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}


// ---------------------------------------------------------------------------
//  TZWCCImplMLSensor: Public, non-virtual methods
// ---------------------------------------------------------------------------

// The containing unit can force our format, in which case we don't parse extra info
tCIDLib::TVoid
TZWCCImplMLSensor::SetFormat(const  tCQCKit::EFldTypes      eValType
                            , const tCIDLib::TFloat8        f8MinRange
                            , const tCIDLib::TFloat8        f8MaxRange
                            , const tZWaveUSB3Sh::ETScales  eScale)
{
    m_eTempScale = eScale;
    m_eValType = eValType;
    m_f8MinRange = f8MinRange;
    m_f8MaxRange = f8MaxRange;

    m_bFmtForced = kCIDLib::True;
}


// ---------------------------------------------------------------------------
//  TZWCCImplMLSensor: Protected, inherited methods
// ---------------------------------------------------------------------------

//  We just watch for a multi-level sensor report and store the value
tZWaveUSB3Sh::ECCMsgRes
TZWCCImplMLSensor::eHandleCCMsg(const   tCIDLib::TCard1 c1Class
                                , const tCIDLib::TCard1 c1Cmd
                                , const TZWInMsg&       zwimIn)
{
    if ((c1Class == COMMAND_CLASS_SENSOR_MULTILEVEL) && (c1Cmd == SENSOR_MULTILEVEL_REPORT))
    {
        const TZWUnitInfo& unitiUs = unitiOwner();

        tCIDLib::TCard4 c4Precision;
        tCIDLib::TCard4 c4Scale;
        tCIDLib::TCard4 c4Bytes;
        tCIDLib::TCard4 c4RawVal;
        const tCIDLib::TBoolean bRes = bExtractInfo
        (
            zwimIn
            , 3
            , c4Precision
            , c4Scale
            , c4Bytes
            , c4RawVal
        );

        if (!bRes)
        {
            SetErrState();
            return tZWaveUSB3Sh::ECCMsgRes_Value;
        }

        const tCIDLib::TBoolean bWasInError = bError();
        tCIDLib::TFloat8 f8Val(c4RawVal);

        //
        //  Now deal with any decimal digits. This gets us the actual value.
        //  Be careful of divide by zero if c1Precision is zero!
        //
        if (c4Precision)
            f8Val /= TMathLib::f8Power(10.0, tCIDLib::TFloat4(c4Precision));

        //
        //  If this is a value that is in F or C scale, then see if we need to convert
        //  to another scale.
        //
        const tCIDLib::TCard1 c1ValType = zwimIn.c1CCByteAt(2);
        if ((c1ValType == 0x1)
        ||  (c1ValType == 0xB)
        ||  (c1ValType == 0x17)
        ||  (c1ValType == 0x18)
        ||  (c1ValType == 0x22))
        {
            if ((m_eTempScale == tZWaveUSB3Sh::ETScales::F) && (c4Scale == 0))
            {
                // Convert from C to F
                f8Val *= 9.0 / 5.0;
                f8Val += 32;
            }
             else if ((m_eTempScale == tZWaveUSB3Sh::ETScales::C) && (c4Scale == 1))
            {
                 // Convert from F to C
                 f8Val -= 32;
                 f8Val *= 5.0 / 9.0;
            }
        }

        //
        //  And we can now check the range. If out of range, then we are in error,
        //  else let's see if we can convert it to the target type.
        //
        //  We pass the scale as the extra info to the value change report method.
        //  The derived class and owning unit may need this.
        //
        if ((f8Val < m_f8MinRange) || (f8Val > m_f8MaxRange))
        {
            if (facZWaveUSB3Sh().bLowTrace())
            {
                facZWaveUSB3Sh().LogTraceErr
                (
                    L"%(1)/%(2)- Received value '%(3)' is out of field range"
                    , strUnitName()
                    , strDesc()
                    , TFloat(f8Val));
            }

            SetErrState();
        }
         else if (m_eValType == tCQCKit::EFldTypes::Card)
        {
            TMathLib::Round(f8Val);
            const tCIDLib::TCard4 c4NewVal = tCIDLib::TCard4(f8Val);
            if (f8Val < 0)
            {
                // Just in case
                SetErrState();
            }
             else if (bWasInError || (m_c4Level != c4NewVal))
            {
                m_c4Level = c4NewVal;
                ReportValChange(tZWaveUSB3Sh::EValSrcs::Unit, tCIDLib::TInt4(c4Scale));
            }
        }
         else if (m_eValType == tCQCKit::EFldTypes::Float)
        {
            if (bWasInError || (m_f8Level != f8Val))
            {
                m_f8Level = f8Val;
                ReportValChange(tZWaveUSB3Sh::EValSrcs::Unit, tCIDLib::TInt4(c4Scale));
            }
        }
         else if (m_eValType == tCQCKit::EFldTypes::Int)
        {
            TMathLib::Round(f8Val);
            const tCIDLib::TInt4 i4NewVal = tCIDLib::TInt4(f8Val);
            if ((f8Val < kCIDLib::i4MinInt) || (f8Val > kCIDLib::i4MaxInt))
            {
                // Just in case
                SetErrState();
            }
             else if (bWasInError || (m_i4Level != i4NewVal))
            {
                m_i4Level = i4NewVal;
                ReportValChange(tZWaveUSB3Sh::EValSrcs::Unit, tCIDLib::TInt4(c4Scale));
            }
        }
         else
        {
            facZWaveUSB3Sh().LogTraceErr
            (
                L"%(1)/%(2)- Unknown ML sensor value type '%(3)'"
                , strUnitName()
                , strDesc()
                , TInteger(tCIDLib::i4EnumOrd(m_eValType))
            );

            // Shouldn't happen but just in case
            SetErrState();
        }
        return tZWaveUSB3Sh::ECCMsgRes_Value;
    }
    return tZWaveUSB3Sh::ECCMsgRes_Unhandled;
}




// ---------------------------------------------------------------------------
//   CLASS: TZWCCImplMLSensorF
//  PREFIX: zwcci
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWCCImplMLSensorF: Constructors and Destructor
// ---------------------------------------------------------------------------
TZWCCImplMLSensorF::
TZWCCImplMLSensorF(         TZWUnitInfo* const  punitiThis
                    , const TString&            strFldName
                    , const tCQCKit::EFldSTypes eSemType
                    , const tCIDLib::TCard1     c1EPId) :

    TZWCCImplMLSensor(punitiThis, c1EPId)
    , m_c4FldId_Level(kCIDLib::c4MaxCard)
    , m_eSemType(eSemType)
    , m_strFldName(strFldName)
{
}

TZWCCImplMLSensorF::~TZWCCImplMLSensorF()
{
}


// ---------------------------------------------------------------------------
//  TZWCCImplMLSensorF: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TZWCCImplMLSensorF::bOwnsFld(const tCIDLib::TCard4 c4FldId) const
{
    return (c4FldId == m_c4FldId_Level);
}


tCIDLib::TVoid
TZWCCImplMLSensorF::QueryFldDefs(tCQCKit::TFldDefList&  colAddTo
                                , TCQCFldDef&           flddTmp) const
{
    // Format out the range
    TString strLims(L"Range: ", 32UL);

    const tCQCKit::EFldTypes eType = eValType();
    if (eType == tCQCKit::EFldTypes::Card)
    {
        strLims.AppendFormatted(tCIDLib::TCard4(f8MinRange()));
        strLims.Append(kCIDLib::chComma);
        strLims.AppendFormatted(tCIDLib::TCard4(f8MaxRange()));
    }
     else if (eType == tCQCKit::EFldTypes::Float)
    {
        strLims.AppendFormatted(f8MinRange(), 4);
        strLims.Append(kCIDLib::chComma);
        strLims.AppendFormatted(f8MaxRange(), 4);
    }
     else if (eType == tCQCKit::EFldTypes::Int)
    {
        strLims.AppendFormatted(tCIDLib::TInt4(f8MinRange()));
        strLims.Append(kCIDLib::chComma);
        strLims.AppendFormatted(tCIDLib::TInt4(f8MaxRange()));
    }
     else
    {
        CIDAssert2(L"Unknown multi-level sensor field type");
    }

    flddTmp.Set(m_strFldName, eType, tCQCKit::EFldAccess::Read, m_eSemType, strLims);
    colAddTo.objAdd(flddTmp);
}


tCIDLib::TVoid TZWCCImplMLSensorF::StoreFldIds()
{
    // Just look up our one field using the name we were given
    m_c4FldId_Level = c4LookupFldId(m_strFldName);
    SetFieldErr(m_c4FldId_Level);
}


// ---------------------------------------------------------------------------
//  TZWCCImplMLSensorF: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TZWCCImplMLSensorF::EnteredErrState()
{
    SetFieldErr(m_c4FldId_Level);
    TParent::SetErrState();
}


tCIDLib::TVoid
TZWCCImplMLSensorF::ValueChanged(const  tZWaveUSB3Sh::EValSrcs  eSource
                                , const tCIDLib::TBoolean       bWasInErr
                                , const tCIDLib::TInt4          i4ExtraInfo)
{
    switch(eValType())
    {
        case tCQCKit::EFldTypes::Card :
            bWriteFld(m_c4FldId_Level, c4Level());
            break;

        case tCQCKit::EFldTypes::Float :
            bWriteFld(m_c4FldId_Level, f8Level());
            break;

        case tCQCKit::EFldTypes::Int :
            bWriteFld(m_c4FldId_Level, i4Level());
            break;

        default :
            break;
    };

    TParent::ValueChanged(eSource, bWasInErr, i4ExtraInfo);
}

