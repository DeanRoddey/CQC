//
// FILE NAME: ZWaveUSB3Sh_NMCMSensor.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/06/2018
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
//  This is the implementation for a family of an annoyingly common type of device
//  with multiple-sensors without using the multi-channel system.
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
RTTIDecls(TNonMCMultiSensorUnit, TZWaveUnit)


// ---------------------------------------------------------------------------
//   CLASS: TNonMCMultiSensorUnit::TSensorInfo
//  PREFIX: sensi
// ---------------------------------------------------------------------------

TNonMCMultiSensorUnit::TSensorInfo::TSensorInfo() :

    m_bIsBinary(kCIDLib::False)
    , m_c2NotId(0)
    , m_eAccess(tZWaveUSB3Sh::EUnitAcc::None)
    , m_eNotEvType(tZWaveUSB3Sh::ENotEvTypes::Count)
    , m_eNotCC(tZWaveUSB3Sh::ECClasses::None)
    , m_eNotIdType(tZWaveUSB3Sh::ENotIdTypes::Count)
    , m_ePrimCC(tZWaveUSB3Sh::ECClasses::None)
    , m_eType(tZWaveUSB3Sh::EGenMSTypes::Count)
    , m_pzwcciNot(nullptr)
    , m_pzwcciPrim(nullptr)
{
}

//
//  The CCs are not copied, they will be created later, and the source wouldn't have them
//  set anyway at the point one was being copied.

TNonMCMultiSensorUnit::TSensorInfo::
TSensorInfo(const TNonMCMultiSensorUnit::TSensorInfo& sensiSrc) :

    m_bIsBinary(sensiSrc.m_bIsBinary)
    , m_c2NotId(sensiSrc.m_c2NotId)
    , m_eAccess(tZWaveUSB3Sh::EUnitAcc::None)
    , m_eNotEvType(sensiSrc.m_eNotEvType)
    , m_eNotCC(sensiSrc.m_eNotCC)
    , m_eNotIdType(sensiSrc.m_eNotIdType)
    , m_ePrimCC(sensiSrc.m_ePrimCC)
    , m_eType(sensiSrc.m_eType)
    , m_pzwcciNot(nullptr)
    , m_pzwcciPrim(nullptr)
{
}

TNonMCMultiSensorUnit::TSensorInfo::~TSensorInfo()
{
    // Clean up our CCs if created
    if (m_pzwcciNot)
    {
        try
        {
            delete m_pzwcciNot;
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        catch(...)
        {
        }
    }

    if (m_pzwcciPrim)
    {
        try
        {
            delete m_pzwcciPrim;
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        catch(...)
        {
        }
    }
}


tCIDLib::TBoolean
TNonMCMultiSensorUnit::TSensorInfo::bCreateCCs(         TZWUnitInfo&        unitiSrc
                                                , const tCIDLib::TCard4     c4Index
                                                ,       tCIDLib::TCard4&    c4FauxId
                                                ,       TString&            strErr)
{
    // Based on sensor type, set up some stuff we'll use below
    tCQCKit::EFldTypes  eType;
    tCQCKit::EFldSTypes eSemType = tCQCKit::EFldSTypes::Generic;
    TString             strDesc;
    tCIDLib::TFloat8    f8Min = 0;
    tCIDLib::TFloat8    f8Max = tCIDLib::TFloat8(kCIDLib::c4MaxCard);

    switch(m_eType)
    {
        case tZWaveUSB3Sh::EGenMSTypes::DigInput :
            m_bIsBinary = kCIDLib::True;
            eType = tCQCKit::EFldTypes::Boolean;
            m_strFldName = unitiSrc.strName();
            m_strFldName.Append(L"_DigInput");
            strDesc = L"DigInput";
            break;

        case tZWaveUSB3Sh::EGenMSTypes::DoorWnd :
            m_bIsBinary = kCIDLib::True;
            eType = tCQCKit::EFldTypes::Boolean;
            m_strFldName = unitiSrc.strName();
            m_strFldName.Append(L"_State");
            strDesc = L"Door/Wnd";
            break;

        case tZWaveUSB3Sh::EGenMSTypes::Motion :
            m_bIsBinary = kCIDLib::True;
            eType = tCQCKit::EFldTypes::Boolean;
            m_strFldName = L"MOT#";
            m_strFldName.Append(unitiSrc.strName());
            strDesc = L"Motion";
            eSemType = tCQCKit::EFldSTypes::MotionSensor;
            break;

        case tZWaveUSB3Sh::EGenMSTypes::Humidity :
            m_bIsBinary = kCIDLib::False;
            eType = tCQCKit::EFldTypes::Card;
            m_strFldName = unitiSrc.strName();
            m_strFldName.Append(L"_Humidity");
            strDesc = L"Humidity";
            f8Max = 100;
            eSemType = tCQCKit::EFldSTypes::HumSensor;
            break;

        case tZWaveUSB3Sh::EGenMSTypes::Lux :
            m_bIsBinary = kCIDLib::False;
            eType = tCQCKit::EFldTypes::Card;
            m_strFldName = unitiSrc.strName();
            m_strFldName.Append(L"_Luminance");
            strDesc = L"Lux";
            break;

        case tZWaveUSB3Sh::EGenMSTypes::Switch :
            m_bIsBinary = kCIDLib::True;
            eType = tCQCKit::EFldTypes::Boolean;
            m_strFldName = unitiSrc.strName();
            m_strFldName.Append(L"_Switch");
            strDesc = L"Switch";
            break;

        case tZWaveUSB3Sh::EGenMSTypes::Temp :
            m_bIsBinary = kCIDLib::False;
            eType = tCQCKit::EFldTypes::Int;
            eSemType = tCQCKit::EFldSTypes::CurTemp;
            m_strFldName = unitiSrc.strName();
            m_strFldName.Append(L"_Temp");
            strDesc = L"Temperature";

            // We will special case the min/max below
            break;

        case tZWaveUSB3Sh::EGenMSTypes::UV :
            m_bIsBinary = kCIDLib::False;
            eType = tCQCKit::EFldTypes::Card;
            m_strFldName = unitiSrc.strName();
            m_strFldName.Append(L"_UV");
            strDesc = L"UV";
            f8Min = 1;
            f8Max = 20;
            break;

        case tZWaveUSB3Sh::EGenMSTypes::Voltage :
            m_bIsBinary = kCIDLib::False;
            eType = tCQCKit::EFldTypes::Card;
            m_strFldName = unitiSrc.strName();
            m_strFldName.Append(L"_Voltage");
            strDesc = L"Voltage";

            // The voltage will have no limits set
            break;

        default :
            strErr = L"Unknown NMCh sensor type";
            return kCIDLib::False;
    };


    // If a temp sensor, we have to get the scale and min/max from options
    tCIDLib::TBoolean bFScale = kCIDLib::False;
    if (m_eType == tZWaveUSB3Sh::EGenMSTypes::Temp)
    {
        // Get the temp range option
        bFScale = unitiSrc.strQueryOptVal(kZWaveUSB3Sh::strUOpt_TempScale, L"F") == L"F";

        f8Min = unitiSrc.i4QueryOptVal
        (
            kZWaveUSB3Sh::strUOpt_CurTempMin, bFScale ? -20 : -30
        );
        f8Max = unitiSrc.i4QueryOptVal
        (
            kZWaveUSB3Sh::strUOpt_CurTempMax, bFScale ? 120 : 55
        );
    }

    // Create our primary CC, which we always have
    if (m_ePrimCC == tZWaveUSB3Sh::ECClasses::Basic)
    {
        // We create either a switch or level type as required
        if (m_bIsBinary)
            m_pzwcciPrim = new TZWCCImplBasicSwitchF(&unitiSrc, m_strFldName, eSemType);
        else
            m_pzwcciPrim = new TZWCCImplBasicLevelF(&unitiSrc, m_strFldName, eSemType);
    }
     else if (m_ePrimCC == tZWaveUSB3Sh::ECClasses::BinSensor)
    {
        if (!m_bIsBinary)
        {
            strErr = L"BinSensor must be a binary type";
            return kCIDLib::False;
        }
        m_pzwcciPrim = new TZWCCImplBinSensorF(&unitiSrc, m_strFldName, eSemType);
    }
     else if (m_ePrimCC == tZWaveUSB3Sh::ECClasses::BinSwitch)
    {
        if (!m_bIsBinary)
        {
            strErr = L"BinSwitch must be a binary type";
            return kCIDLib::False;
        }
        m_pzwcciPrim = new TZWCCImplBinSwitchF(&unitiSrc, m_strFldName, eSemType);
    }
     else if (m_ePrimCC == tZWaveUSB3Sh::ECClasses::MLSensor)
    {
        if (m_bIsBinary)
        {
            strErr = L"MLSensor is not a binary type";
            return kCIDLib::False;
        }

        TZWCCImplMLSensor* pzwcciNew = new TZWCCImplMLSensorF
        (
            &unitiSrc, m_strFldName, eSemType
        );

        pzwcciNew->SetFormat
        (
            eType
            , f8Min
            , f8Max
            , bFScale ? tZWaveUSB3Sh::ETScales::F : tZWaveUSB3Sh::ETScales::C
        );
        m_pzwcciPrim = pzwcciNew;
    }
     else if (m_ePrimCC == tZWaveUSB3Sh::ECClasses::Notification)
    {
        if (!m_bIsBinary)
        {
            strErr = L"Notification must be a binary type";
            return kCIDLib::False;
        }

        TZWCCImplNotifyState* pzwcciNew = new TZWCCImplNotifyStateF
        (
            &unitiSrc, m_strFldName, eSemType
        );
        m_pzwcciPrim = pzwcciNew;

        // We have to fully set it up ourself in our case
        pzwcciNew->SetNotIds
        (
            m_eNotIdType, m_eNotEvType, m_c2NotId, m_fcolOffEvs, m_fcolOnEvs
        );
    }
     else
    {
        strErr = L"Unknown primary CC type";
        return kCIDLib::False;
    }

    // Set up some common stuff on the primary
    m_pzwcciPrim->SetAccess(m_eAccess, tZWaveUSB3Sh::EUnitAcc::AllBits);
    m_pzwcciPrim->c4ImplId(c4FauxId++);
    m_pzwcciPrim->strDesc(strDesc);


    // If we have a notification CC, create that
    if (m_eNotCC == tZWaveUSB3Sh::ECClasses::Basic)
    {
        if (m_bIsBinary)
            m_pzwcciNot = new TZWCCImplBasicSwitch(&unitiSrc);
        else
            m_pzwcciNot = new TZWCCImplBasicLevel(&unitiSrc);
    }
     else if (m_eNotCC == tZWaveUSB3Sh::ECClasses::BinSensor)
    {
        m_pzwcciNot = new TZWCCImplBinSensor(&unitiSrc);
    }
     else if (m_eNotCC == tZWaveUSB3Sh::ECClasses::Notification)
    {
        TZWCCImplNotifyState* pzwcciNew = new TZWCCImplNotifyState(&unitiSrc);
        m_pzwcciNot = pzwcciNew;

        // We have to fully set it up ourself in our case
        pzwcciNew->SetNotIds
        (
            m_eNotIdType, m_eNotEvType, m_c2NotId, m_fcolOffEvs, m_fcolOnEvs
        );
    }
     else if (m_eNotCC != tZWaveUSB3Sh::ECClasses::None)
    {
        strErr = L"Unknown notification CC type";
        return kCIDLib::False;
    }

    //
    //  If we got one, do the common init on it as well. Make sure access is set
    //  to none in this case, since this is a notification class.
    //
    if (m_pzwcciNot)
    {
        m_pzwcciNot->SetAccess
        (
            tZWaveUSB3Sh::EUnitAcc::None, tZWaveUSB3Sh::EUnitAcc::AllBits
        );
        m_pzwcciNot->c4ImplId(c4FauxId++);
        m_pzwcciNot->strDesc(strDesc + TString(L" Notify"));
    }

    return kCIDLib::True;
}


//
//  Parse out the extra info and set ourself up. We look for sensor info until we
//  don't find one, then we know we are done.
//
tCIDLib::TBoolean
TNonMCMultiSensorUnit::TSensorInfo::bParseInfo( const   TString&        strInfo
                                                , const TZWUnitInfo&    unitiSrc
                                                ,       TString&        strErr)
{
    TString strToken;
    tCIDLib::TStrList colTokens;

    //
    //  Parse this guy out into the separate tokens, though some may still
    //  have sub-content we need to break out.
    //
    TStringTokenizer stokTok(&strInfo, L";");
    while (stokTok.bGetNextToken(strToken))
    {
        strToken.StripWhitespace();
        colTokens.objAdd(strToken);
    }

    // We have to have at least 4 tokens minimum
    const tCIDLib::TCard4 c4TokenCnt = colTokens.c4ElemCount();
    if (c4TokenCnt < 4)
    {
        strErr = L"Too few sensor info values";
        return kCIDLib::False;
    }

    //
    //  Get the fixed bits.
    //
    m_eType = tZWaveUSB3Sh::eXlatEGenMSTypes(colTokens[0]);
    m_ePrimCC = tZWaveUSB3Sh::eXlatECClasses(colTokens[1]);
    m_eNotCC =
    (
        colTokens[2].bIsEmpty() ? tZWaveUSB3Sh::ECClasses::None
                                : tZWaveUSB3Sh::eXlatECClasses(colTokens[2])
    );

    // If we failed to get a primary, obviously bad
    if (m_ePrimCC == tZWaveUSB3Sh::ECClasses::None)
    {
        strErr = L"Invalid primary CC was found";
        return kCIDLib::False;
    }

    // If notification is the primary, then we can't have any notification
    if ((m_ePrimCC == tZWaveUSB3Sh::ECClasses::Notification)
    &&  (m_eNotCC != tZWaveUSB3Sh::ECClasses::None))
    {
        strErr = L"If primary is Notification, there cannot be a notification CC";
        return kCIDLib::False;
    }

    //
    //  Parse out the access flags, if any. Unlike the base CC impl we don't have to
    //  deal with the old style for these flags (via the RWAccess CC extra info), since
    //  this handler was added along with the changes to conslidate these values across
    //  all units and CCs.
    //
    //  Though we are a multi-'sensor', we do support some writable things like switches
    //  since it's not worth having another complex handler like this just to deal with
    //  that.
    //
    m_eAccess = tZWaveUSB3Sh::EUnitAcc::None;
    if (!colTokens[3].bIsEmpty())
    {
        stokTok.Reset(&colTokens[3], L" ");
        while (stokTok.bGetNextToken(strToken))
        {
            if (strToken.bCompareI(L"Read"))
                m_eAccess |= tZWaveUSB3Sh::EUnitAcc::Read;
            else if (strToken.bCompareI(L"ReadOnWake"))
                m_eAccess |= tZWaveUSB3Sh::EUnitAcc::ReadOnWake;
            else if (strToken.bCompareI(L"ReadAfterWrite"))
                m_eAccess |= tZWaveUSB3Sh::EUnitAcc::ReadAfterWrite;
            else if (strToken.bCompareI(L"Write"))
                m_eAccess |= tZWaveUSB3Sh::EUnitAcc::Write;
        }
    }

    if (tCIDLib::bAnyBitsOn(m_eAccess, tZWaveUSB3Sh::EUnitAcc::OptReadFlags)
    &&  !tCIDLib::bAllBitsOn(m_eAccess, tZWaveUSB3Sh::EUnitAcc::Read))
    {
        strErr = L"Read options enabled but not readable";
        return kCIDLib::False;
    }

    // Based on the sensor type, remember if this is a binary or multi-level sensor
    switch(m_eType)
    {
        case tZWaveUSB3Sh::EGenMSTypes::DigInput :
        case tZWaveUSB3Sh::EGenMSTypes::DoorWnd :
        case tZWaveUSB3Sh::EGenMSTypes::Motion :
        case tZWaveUSB3Sh::EGenMSTypes::Switch :
            m_bIsBinary = kCIDLib::True;
            break;

        case tZWaveUSB3Sh::EGenMSTypes::Humidity :
        case tZWaveUSB3Sh::EGenMSTypes::Lux :
        case tZWaveUSB3Sh::EGenMSTypes::Temp :
        case tZWaveUSB3Sh::EGenMSTypes::UV :
        case tZWaveUSB3Sh::EGenMSTypes::Voltage :
            m_bIsBinary = kCIDLib::False;
            break;

        default :
            strErr = L"Unknown general multi-sensor type";
            return kCIDLib::False;
    };

    // If notification is the primary CC, then it has to be binary
    if ((m_ePrimCC == tZWaveUSB3Sh::ECClasses::Notification) && !m_bIsBinary)
    {
        strErr = L"Notification class requires a binary type";
        return kCIDLib::False;
    }

    // If there's a notification class used, we have to have the notification info.
    if ((m_ePrimCC == tZWaveUSB3Sh::ECClasses::Notification)
    ||  (m_eNotCC == tZWaveUSB3Sh::ECClasses::Notification))
    {
        // We don't support a notification for multi-level sensor primary
        if (m_ePrimCC == tZWaveUSB3Sh::ECClasses::MLSensor)
        {
            strErr = L"Notification CC not supported for multi-level sensors";
            return kCIDLib::False;
        }

        // We have to have a minimum of 9 tokens in this case
        if (c4TokenCnt != 9)
        {
            strErr = L"Too few sensor info values for notification CC";
            return kCIDLib::False;
        }

        m_eNotIdType = tZWaveUSB3Sh::eXlatENotIdTypes(colTokens[4]);
        if (m_eNotIdType == tZWaveUSB3Sh::ENotIdTypes::Count)
        {
            strErr = L"Invalid notification id type";
            return kCIDLib::False;
        }

        m_eNotEvType = tZWaveUSB3Sh::eXlatENotEvTypes(colTokens[5]);
        if (m_eNotEvType == tZWaveUSB3Sh::ENotEvTypes::Count)
        {
            strErr = L"Invalid notification event type";
            return kCIDLib::False;
        }

        if (!colTokens[6].bToCard2(m_c2NotId, tCIDLib::ERadices::Auto))
        {
            strErr = L"Invalid notification id";
            return kCIDLib::False;
        }

        tCIDLib::TCard1 c1Id;
        stokTok.Reset(&colTokens[7], L" ");
        while (stokTok.bGetNextToken(strToken))
        {
            if (strToken.bToCard1(c1Id, tCIDLib::ERadices::Auto))
                m_fcolOffEvs.c4AddElement(c1Id);
        }

        stokTok.Reset(&colTokens[8], L" ");
        while (stokTok.bGetNextToken(strToken))
        {
            if (strToken.bToCard1(c1Id, tCIDLib::ERadices::Auto))
                m_fcolOnEvs.c4AddElement(c1Id);
        }

        if (m_fcolOffEvs.bIsEmpty() || m_fcolOnEvs.bIsEmpty())
        {
            strErr = L"Missing off or on notification event ids";
            return kCIDLib::False;
        }
    }

    return kCIDLib::True;
}


tCIDLib::TBoolean TNonMCMultiSensorUnit::TSensorInfo::
bHandleImplChange(TZWUnitInfo& unitiUs, const tCIDLib::TCard4 c4ImplId)
{
    tCIDLib::TBoolean bRet = kCIDLib::False;

    // Push the change one way or another
    if (m_pzwcciPrim->c4ImplId() == c4ImplId)
    {
        // There may not be a notification class
        if (m_pzwcciNot)
            bRet = bPushChange(*m_pzwcciPrim, *m_pzwcciNot);
    }
    else if (m_pzwcciNot &&  (m_pzwcciNot->c4ImplId() == c4ImplId))
    {
        // There has to be a primary, so we don't need to check that
        bRet = bPushChange(*m_pzwcciNot, *m_pzwcciPrim);
    }

    return bRet;
}


tCIDLib::TBoolean
TNonMCMultiSensorUnit::TSensorInfo::bPushChange(TZWCCImpl& zwcciSrc, TZWCCImpl& zwcciTar)
{
    tCIDLib::TBoolean bRet = kCIDLib::False;
    if (m_bIsBinary)
    {
        //
        //  It'll be one of the base binary impl derivatives so we can cast them both
        //  and do it easily via the base binary CC class they share.
        //
        CIDAssert
        (
            zwcciTar.bIsDescendantOf(TZWCCImplBin::clsThis())
            , L"The target is not a binary type but the source is"
        );

        bRet = static_cast<TZWCCImplBin&>(zwcciTar).bSetBinState
        (
            static_cast<TZWCCImplBin&>(zwcciSrc).bState()
            , tZWaveUSB3Sh::EValSrcs::Program
        );
    }
     else
    {
        if (m_ePrimCC == tZWaveUSB3Sh::ECClasses::Basic)
        {
            //
            //  These are things that implement the basic level type,
            //  so we can handle them the same. Currently only non-binary
            //  basic.
            //
            bRet = static_cast<TZWCCImplLevel&>(zwcciTar).bSetLevel
            (
                static_cast<TZWCCImplLevel&>(zwcciSrc).c4Level()
                , tZWaveUSB3Sh::EValSrcs::Program
            );
        }
         else if (m_ePrimCC == tZWaveUSB3Sh::ECClasses::MLSensor)
        {
            // This is not supported so shouldn't happen
            CIDAssert2(L"Should not have notification CC for ML sensor");
        }
    }
    return bRet;
}



// ---------------------------------------------------------------------------
//   CLASS: TNonMCMultiSensorUnit
//  PREFIX: unit
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TNonMCMultiSensorUnit: Constructors and Destructor
// ---------------------------------------------------------------------------
TNonMCMultiSensorUnit::TNonMCMultiSensorUnit(TZWUnitInfo* const punitiOwner) :

    TZWaveUnit(punitiOwner)

{
}

TNonMCMultiSensorUnit::~TNonMCMultiSensorUnit()
{
    // Clean up our CC impls that we created
    const tCIDLib::TCard4 c4Count = m_colSensors.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        try
        {
            m_colSensors.RemoveAt(0);
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        catch(...)
        {
        }
    }
}


// ---------------------------------------------------------------------------
//  TNonMCMultiSensorUnit: Public, inherited methods
// ---------------------------------------------------------------------------

// If we have any writable CCs, we need to handle this
tCIDLib::TBoolean
TNonMCMultiSensorUnit::bBoolFldChanged( const   TString&                strFldName
                                        , const tCIDLib::TCard4         c4FldId
                                        , const tCIDLib::TBoolean       bNewValue
                                        ,       tCQCKit::ECommResults&  eRes)
{
    tCIDLib::TBoolean bRet = kCIDLib::False;
    try
    {
        const tCIDLib::TCard4 c4Count = m_colSensors.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            TSensorInfo& sensiCur = m_colSensors[c4Index];
            if (!sensiCur.m_pzwcciPrim || !sensiCur.m_pzwcciPrim->bIsReadable())
                continue;

            if (sensiCur.m_pzwcciPrim->bBoolFldChanged(strFldName, c4FldId, bNewValue, eRes))
            {
                bRet = kCIDLib::True;
                break;
            }
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
    return bRet;
}


tCIDLib::TBoolean
TNonMCMultiSensorUnit::bCardFldChanged( const   TString&                strFldName
                                        , const tCIDLib::TCard4         c4FldId
                                        , const tCIDLib::TCard4         c4NewValue
                                        ,       tCQCKit::ECommResults&  eRes)
{
    tCIDLib::TBoolean bRet = kCIDLib::False;
    try
    {
        const tCIDLib::TCard4 c4Count = m_colSensors.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            TSensorInfo& sensiCur = m_colSensors[c4Index];
            if (!sensiCur.m_pzwcciPrim || !sensiCur.m_pzwcciPrim->bIsReadable())
                continue;

            if (sensiCur.m_pzwcciPrim->bCardFldChanged(strFldName, c4FldId, c4NewValue, eRes))
            {
                bRet = kCIDLib::True;
                break;
            }
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
    return bRet;
}


// Just pass it on to our CC impls
tCIDLib::TBoolean TNonMCMultiSensorUnit::bOwnsFld(const tCIDLib::TCard4 c4FldId) const
{
    //
    //  We return false to break out of the iteration if we find a hit. So we
    //  negate the return to get the right value.
    //
    return !m_colSensors.bForEach
    (
        [this, c4FldId](const TSensorInfo& sensiCur)
        {
            if (sensiCur.m_pzwcciPrim)
            {
                if (sensiCur.m_pzwcciPrim->bOwnsFld(c4FldId))
                    return kCIDLib::False;

                if (sensiCur.m_pzwcciNot && sensiCur.m_pzwcciNot->bOwnsFld(c4FldId))
                    return kCIDLib::False;
            }
            return kCIDLib::True;
        }
    );
}


// We get told by the device info file what sensors we support
tCIDLib::TBoolean TNonMCMultiSensorUnit::bParseExtraInfo()
{
    if (!TParent::bParseExtraInfo())
        return kCIDLib::False;

    TZWUnitInfo& unitiUs = unitiOwner();

    // We run up through the sensors and store the info away
    m_colSensors.RemoveAll();
    tCIDLib::TCard4 c4SensorNum = 1;
    TString strSensorInfo;
    TString strSensorKey;
    TString strErr;
    const TString strErrMsg(L"%(1), Unit=%(2), Sensor #=%(3)");
    while (kCIDLib::True)
    {
        // Break out if we don't find this one
        strSensorKey = L"Sensor";
        strSensorKey.AppendFormatted(c4SensorNum);
        if (!bFindExtraVal(strSensorKey, strSensorInfo))
            break;

        // Add an entry to the list and let it parse its content out
        TSensorInfo& sensiNew = m_colSensors.objAdd(TSensorInfo());
        if (!sensiNew.bParseInfo(strSensorInfo, unitiUs, strErr))
        {
            if (facZWaveUSB3Sh().bLowTrace())
            {
                facZWaveUSB3Sh().LogTraceErr
                (
                    strErrMsg, strErr, unitiUs.strName(), TCardinal(c4SensorNum)
                );
            }
            return kCIDLib::False;
        }

        // Move up to the next one
        c4SensorNum++;
    }

    // Make sure no clashing info was provided
    tCIDLib::TBoolean bBasic = kCIDLib::False;
    tCIDLib::TBoolean bBinSensor = kCIDLib::False;
    tCIDLib::TBoolean bHumidity = kCIDLib::False;
    tCIDLib::TBoolean bLux = kCIDLib::False;
    tCIDLib::TBoolean bMotion = kCIDLib::False;
    tCIDLib::TBoolean bNotify = kCIDLib::False;
    tCIDLib::TBoolean bVoltage = kCIDLib::False;
    tCIDLib::TBoolean bTemp = kCIDLib::False;
    const tCIDLib::TCard4 c4Count = m_colSensors.c4ElemCount();
    for (tCIDLib::TCard4 c4Index= 0; c4Index < c4Count; c4Index++)
    {
        TSensorInfo& sensiCur = m_colSensors[c4Index];
        if ((sensiCur.m_ePrimCC == tZWaveUSB3Sh::ECClasses::Basic)
        ||  (sensiCur.m_eNotCC == tZWaveUSB3Sh::ECClasses::Basic))
        {
            if (bBasic)
            {
                facZWaveUSB3Sh().LogTraceErr(L"Dup Basic CC in unit %(1)", unitiUs.strName());
                return kCIDLib::False;
            }
            bBasic = kCIDLib::True;
        }
         else if ((sensiCur.m_ePrimCC == tZWaveUSB3Sh::ECClasses::BinSensor)
              ||  (sensiCur.m_eNotCC == tZWaveUSB3Sh::ECClasses::BinSensor))
        {
            if (bBinSensor)
            {
                facZWaveUSB3Sh().LogTraceErr(L"Dup BinSensor CC in unit %(1)", unitiUs.strName());
                return kCIDLib::False;
            }
            bBinSensor = kCIDLib::True;
        }
         else if (sensiCur.m_ePrimCC == tZWaveUSB3Sh::ECClasses::MLSensor)
        {
            if (sensiCur.m_eType == tZWaveUSB3Sh::EGenMSTypes::Humidity)
            {
                if (bHumidity)
                {
                    facZWaveUSB3Sh().LogTraceErr(L"Dup humidity in unit %(1)", unitiUs.strName());
                    return kCIDLib::False;
                }
                bHumidity = kCIDLib::True;
            }
             else if (sensiCur.m_eType == tZWaveUSB3Sh::EGenMSTypes::Lux)
            {
                if (bLux)
                {
                    facZWaveUSB3Sh().LogTraceErr(L"Dup lux in unit %(1)", unitiUs.strName());
                    return kCIDLib::False;
                }
                bLux = kCIDLib::True;
            }
             else if (sensiCur.m_eType == tZWaveUSB3Sh::EGenMSTypes::Motion)
            {
                if (bMotion)
                {
                    facZWaveUSB3Sh().LogTraceErr(L"Dup motion in unit %(1)", unitiUs.strName());
                    return kCIDLib::False;
                }
                bMotion = kCIDLib::True;
            }
             else if (sensiCur.m_eType == tZWaveUSB3Sh::EGenMSTypes::Temp)
            {
                if (bTemp)
                {
                    facZWaveUSB3Sh().LogTraceErr(L"Dup temp in unit %(1)", unitiUs.strName());
                    return kCIDLib::False;
                }
                bTemp = kCIDLib::True;
            }
             else if (sensiCur.m_eType == tZWaveUSB3Sh::EGenMSTypes::Voltage)
            {
                if (bTemp)
                {
                    facZWaveUSB3Sh().LogTraceErr(L"Dup voltage in unit %(1)", unitiUs.strName());
                    return kCIDLib::False;
                }
                bTemp = kCIDLib::True;
            }
             else
            {
               facZWaveUSB3Sh().LogTraceErr(L"Bad ML sensor type in unit %(1)", unitiUs.strName());
                return kCIDLib::False;
            }
        }
         else
        {
            facZWaveUSB3Sh().LogTraceErr(L"Unknown sensor type in unit %(1)", unitiUs.strName());
            return kCIDLib::False;
        }
    }

    return kCIDLib::True;
}


//
//  We need to create our impl objects and prep them. We have to assign faux ids to them
//  so that we can match them up to impl change callbacks.
//
tCIDLib::TBoolean TNonMCMultiSensorUnit::bPrepare()
{
    if (!TParent::bPrepare())
        return kCIDLib::False;

    TZWUnitInfo& unitiUs = unitiOwner();
    tCIDLib::TCard4 c4FauxId = 127;

    //
    //  Create and manually set up our CC impls. NOTE WE HAVE TO manually set any
    //  stuff they would otherwise get from extra info, since that stuff isn't
    //  there in this case.
    //
    try
    {
        TString strErr;
        const TString strErrMsg(L"%(1), Unit=%(2), Sensor #=%(3)");
        tCIDLib::TCard4 c4FauxId = 128;
        const tCIDLib::TCard4 c4Count = m_colSensors.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            TSensorInfo& sensiCur = m_colSensors[c4Index];
            if (!sensiCur.bCreateCCs(unitiUs, c4Index, c4FauxId, strErr))
            {
                if (facZWaveUSB3Sh().bLowTrace())
                {
                    facZWaveUSB3Sh().LogTraceErr
                    (
                        strErrMsg, strErr, unitiUs.strName(), TCardinal(c4Index + 1)
                    );
                }
                return kCIDLib::False;
            }

            // Initialize his CCs
            if (!bInitCCImpl(*sensiCur.m_pzwcciPrim))
                return kCIDLib::False;

            if (sensiCur.m_pzwcciNot)
            {
                if (!bInitCCImpl(*sensiCur.m_pzwcciNot))
                    return kCIDLib::False;
            }
        }
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
tCIDLib::TVoid
TNonMCMultiSensorUnit::FormatCCsInfo(TTextOutStream& strmTar) const
{
    m_colSensors.bForEach
    (
        [this, &strmTar](const TSensorInfo& sensiCur)
        {
            if (sensiCur.m_pzwcciPrim)
            {
                sensiCur.m_pzwcciPrim->FormatReport(strmTar);
                if (sensiCur.m_pzwcciNot)
                    sensiCur.m_pzwcciNot->FormatReport(strmTar);
            }
            return kCIDLib::True;
        }
    );
}


tCIDLib::TVoid TNonMCMultiSensorUnit::GetInitVals()
{
    m_colSensors.bForEachNC
    (
        [](TSensorInfo& sensiCur)
        {
            if (sensiCur.m_pzwcciPrim)
            {
                if (sensiCur.m_pzwcciPrim->bReadAlways())
                    sensiCur.m_pzwcciPrim->SendInitValueQuery();
            }
            return kCIDLib::True;
        }
    );
}


// We have to manually handle distributing values to our CC impls
tCIDLib::TVoid
TNonMCMultiSensorUnit::HandleCCMsg( const   tCIDLib::TCard1 c1Class
                                    , const tCIDLib::TCard1 c1Cmd
                                    , const TZWInMsg&       zwimIn)
{
    TParent::HandleCCMsg(c1Class, c1Cmd, zwimIn);

    // Let all of our CC impls see it
    m_colSensors.bForEachNC
    (
        [c1Class, c1Cmd, &zwimIn](TSensorInfo& sensiCur)
        {
            if (sensiCur.m_pzwcciPrim)
            {
                sensiCur.m_pzwcciPrim->ProcessCCMsg(c1Class, c1Cmd, zwimIn);
                if (sensiCur.m_pzwcciNot)
                    sensiCur.m_pzwcciNot->ProcessCCMsg(c1Class, c1Cmd, zwimIn);
            }
            return kCIDLib::True;
        }
    );
}


//
//  WE have to watch for changes from either of the motion impls. We have to keep the
//  two of them in sync, and we have to send a motion event if we are configured to.
//
tCIDLib::TVoid
TNonMCMultiSensorUnit::ImplValueChanged(const   tCIDLib::TCard4         c4ImplId
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

    //
    //  Go through and find which, if any, of our CC impls this is from. We need to
    //  push values between the notifications and fields to keep them in sync. This
    //  will only happen for binary types, since the multi-levels don't have a separate
    //  notification CC.
    //
    TZWUnitInfo& unitiUs = unitiOwner();
    const tCIDLib::TCard4 c4Count = m_colSensors.c4ElemCount();
    for (tCIDLib::TCard4 c4Index= 0; c4Index < c4Count; c4Index++)
    {
        TSensorInfo& sensiCur = m_colSensors[c4Index];

        // Don't do it if we were in error state, only from one godo state to another
        if (sensiCur.bHandleImplChange(unitiUs, c4ImplId) && !bWasInErr)
        {
            //
            //  If a change we care about, we are configured to send triggers, then
            //  we need to do that.
            //
            if (sensiCur.m_eType == tZWaveUSB3Sh::EGenMSTypes::Motion)
            {
                TString strId;
                strId.AppendFormatted(c1Id());

                unitiOwner().QueueEventTrig
                (
                    tCQCKit::EStdDrvEvs::Motion
                    , sensiCur.m_strFldName
                    , facCQCKit().strBoolStartEnd
                      (
                        static_cast<TZWCCImplBin*>(sensiCur.m_pzwcciPrim)->bState()
                      )
                    , strId
                    , strName()
                    , TString::strEmpty()
                );
            }
        }
    }
}



// Just pass it on to our CC impls
tCIDLib::TVoid
TNonMCMultiSensorUnit::QueryFldDefs(tCQCKit::TFldDefList&   colAddTo
                                    , TCQCFldDef&           flddTmp) const
{
    TParent::QueryFldDefs(colAddTo, flddTmp);

    m_colSensors.bForEach
    (
        [&colAddTo, &flddTmp](const TSensorInfo& sensiCur)
        {
            if (sensiCur.m_pzwcciPrim)
            {
                if (sensiCur.m_pzwcciPrim)
                    sensiCur.m_pzwcciPrim->QueryFldDefs(colAddTo, flddTmp);
            }
            return kCIDLib::True;
        }
    );
}


tCIDLib::TVoid
TNonMCMultiSensorUnit::QueryUnitAttrs(  tCIDMData::TAttrList&   colToFill
                                        , TAttrData&            adatTmp) const
{
    TParent::QueryUnitAttrs(colToFill, adatTmp);

    const TZWUnitInfo& unitiUs = unitiOwner();

    // Loop through and remember if we have a motion and/or a temp
    tCIDLib::TBoolean bHaveMotion = kCIDLib::False;
    tCIDLib::TBoolean bHaveTemp   = kCIDLib::False;

    const tCIDLib::TCard4 c4Count = m_colSensors.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TSensorInfo& sensiCur = m_colSensors[c4Index];
        if (sensiCur.m_eType == tZWaveUSB3Sh::EGenMSTypes::Motion)
            bHaveMotion = kCIDLib::True;
        else if (sensiCur.m_eType == tZWaveUSB3Sh::EGenMSTypes::Temp)
            bHaveTemp = kCIDLib::True;
    }

    //
    //  Motion triggers are optional If no current value, we'll set true as the
    //  default.
    //
    if (bHaveMotion)
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
    if (bHaveTemp)
    {
        //
        //  The thermostat CC helper class has a method to do the standard temp scale and
        //  min/max temp range.
        //
        adatTmp.Set(L"Temp Options:", TString::strEmpty(), tCIDMData::EAttrTypes::Separator);
        colToFill.objAdd(adatTmp);
        TZWUSB3CCThermo::QueryTempAttrs(unitiUs, colToFill, adatTmp);
    }
}


// We have to override this since we manage our own impls
tCIDLib::TVoid TNonMCMultiSensorUnit::Process()
{
    m_colSensors.bForEach
    (
        [this](const TSensorInfo& sensiCur)
        {
            if (sensiCur.m_pzwcciPrim)
                sensiCur.m_pzwcciPrim->Process();
            return kCIDLib::True;
        }
    );
}


// Just pass on to our CC impls
tCIDLib::TVoid TNonMCMultiSensorUnit::StoreFldIds()
{
    TParent::StoreFldIds();

    m_colSensors.bForEachNC
    (
        [](TSensorInfo& sensiCur)
        {
            if (sensiCur.m_pzwcciPrim)
            {
                if (sensiCur.m_pzwcciPrim)
                    sensiCur.m_pzwcciPrim->StoreFldIds();
            }
            return kCIDLib::True;
        }
    );
}


// Just pass on to our CC impls
tCIDLib::TVoid TNonMCMultiSensorUnit::UnitIsOnline()
{
    TParent::UnitIsOnline();

    m_colSensors.bForEachNC
    (
        [](TSensorInfo& sensiCur)
        {
            if (sensiCur.m_pzwcciPrim)
            {
                if (sensiCur.m_pzwcciPrim)
                    sensiCur.m_pzwcciPrim->UnitIsOnline();
            }
            return kCIDLib::True;
        }
    );
}

