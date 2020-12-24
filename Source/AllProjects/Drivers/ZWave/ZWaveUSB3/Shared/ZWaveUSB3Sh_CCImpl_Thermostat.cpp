//
// FILE NAME: ZWaveUSB3Sh_CCImpl_Thermostat.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/03/2018
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
//  The implementation of the thermostat oriented CC Impl classes.
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
RTTIDecls(TZWCCImplThermoMode, TZWCCImpl)
RTTIDecls(TZWCCImplThermoFanMode, TZWCCImpl)
RTTIDecls(TZWCCImplThermoFanState, TZWCCImpl)
RTTIDecls(TZWCCImplThermoState, TZWCCImpl)
RTTIDecls(TZWCCImplThermoSP, TZWCCImpl)



// ---------------------------------------------------------------------------
//   CLASS: TZWCCImplThermoMode
//  PREFIX: zwcci
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWCCImplThermoMode: Constructors and Destructor
// ---------------------------------------------------------------------------
TZWCCImplThermoMode::
TZWCCImplThermoMode(TZWUnitInfo* const  punitiThis, const TString& strFldName) :

    TZWCCImpl(punitiThis, tZWaveUSB3Sh::ECClasses::ThermoMode)
    , m_c4FldId_Mode(kCIDLib::c4MaxCard)
    , m_c4ModeMask(0)
    , m_strFldName(strFldName)
{
    strDesc(L"ThermoMode");

    //
    //  As a default add write access to whatever the base class set, if we are
    //  always on.
    //
    SetWriteAccess(punitiThis->bAlwaysOn());
}

TZWCCImplThermoMode::~TZWCCImplThermoMode()
{
}


// ---------------------------------------------------------------------------
//  TZWCCImplThermoMode: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TZWCCImplThermoMode::bOwnsFld(const tCIDLib::TCard4 c4FldId) const
{
    return (c4FldId == m_c4FldId_Mode);
}


//
//  We pull out and store any extra info from our class in the device info.
//
tCIDLib::TBoolean TZWCCImplThermoMode::bParseExtraInfo()
{
    if (!TParent::bParseExtraInfo())
        return kCIDLib::False;

    // Look up our modes list
    TString strModes;
    if (!bFindExtraVal(L"Modes", strModes)
    ||  !strModes.bToCard4(m_c4ModeMask, tCIDLib::ERadices::Auto))
    {
        facZWaveUSB3Sh().LogTraceErr
        (
            L"%(1)/%(2)- No/bad thermo mode list in extra info", strUnitName(), strDesc()
        );
        return kCIDLib::False;
    }

    //
    //  Let's convert this into a limit string. We just iterate the mode enum, and
    //  use its value as a shifted bit to test against the mask.
    //
    m_strLimits.Clear();
    if (m_c4ModeMask)
    {
        TString strLims(L"Enum: Unknown", 128UL);
        tZWaveUSB3Sh::EThermoModes eMode = tZWaveUSB3Sh::EThermoModes::Min;
        while (eMode <= tZWaveUSB3Sh::EThermoModes::Max)
        {
            tCIDLib::TCard4 c4Bit = 0x1;
            c4Bit <<= tCIDLib::TCard4(eMode);
            if (c4Bit & m_c4ModeMask)
            {
                strLims.Append(L", ");
                strLims.Append(tZWaveUSB3Sh::strXlatEThermoModes(eMode));
            }
            eMode++;
        }
        m_strLimits = strLims;
    }

    return kCIDLib::True;
}


tCIDLib::TBoolean
TZWCCImplThermoMode::bStringFldChanged( const   TString&                strFldName
                                        , const tCIDLib::TCard4         c4FldId
                                        , const TString&                strValue
                                        ,       tCQCKit::ECommResults&  eRes)
{
    tCIDLib::TBoolean bRes = kCIDLib::True;
    try
    {
        if (c4FldId == m_c4FldId_Mode)
        {
            //
            //  Convert the value to the appropriate Z-Wave value. We first convert
            //  it to our thermo mode enum, then get the alt value.
            //
            tZWaveUSB3Sh::EThermoModes eMode = tZWaveUSB3Sh::eXlatEThermoModes(strValue);
            if (eMode == tZWaveUSB3Sh::EThermoModes::Count)
            {
                // Shouldn't happen since we have a limit on the field but just in case
                eRes = tCQCKit::ECommResults::ValueRejected;
            }
             else
            {
                // This should never fail but it will throw if it does
                tCIDLib::TCard1 c1Val = tCIDLib::TCard1
                (
                    tZWaveUSB3Sh::i4AltNumEThermoModes(eMode)
                );

                try
                {
                    // Ask for an ack since this is a command and there won't be a reply
                    TZWOutMsg zwomOut;
                    facZWaveUSB3Sh().BuildCCMsg
                    (
                        zwomOut
                        , unitiOwner()
                        , COMMAND_CLASS_THERMOSTAT_MODE
                        , THERMOSTAT_MODE_SET
                        , 0
                        , c1Val
                        , tZWaveUSB3Sh::EMsgPrios::Command
                        , kCIDLib::True
                    );
                    SendWaitAck(zwomOut);

                    // If configured to do so, then do a read back of the value
                    if (bSendReadAfterWrite())
                        SendValueQuery();
                }

                catch(TError& errToCatch)
                {
                    errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                    throw;
                }

                eRes = tCQCKit::ECommResults::Success;
            }
        }
         else
        {
            // Not one of ours
            bRes = kCIDLib::False;
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
    return bRes;
}


tCIDLib::TVoid
TZWCCImplThermoMode::QueryFldDefs(  tCQCKit::TFldDefList&   colAddTo
                                    , TCQCFldDef&           flddTmp) const
{
    flddTmp.Set
    (
        m_strFldName
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::ReadWrite
        , m_strLimits
    );
    colAddTo.objAdd(flddTmp);
}


tCIDLib::TVoid TZWCCImplThermoMode::SendValueQuery()
{
    TZWOutMsg zwomOut;
    facZWaveUSB3Sh().BuildCCMsg
    (
        zwomOut
        , unitiOwner()
        , COMMAND_CLASS_THERMOSTAT_MODE
        , THERMOSTAT_MODE_GET
        , 0
        , tZWaveUSB3Sh::EMsgPrios::Async
        , kCIDLib::False
    );
    SendWaitAck(zwomOut);
}


tCIDLib::TVoid TZWCCImplThermoMode::StoreFldIds()
{
    m_c4FldId_Mode = c4LookupFldId(m_strFldName);
}


// ---------------------------------------------------------------------------
//  TZWCCImplThermoMode: Protected, inherited methods
// ---------------------------------------------------------------------------

tZWaveUSB3Sh::ECCMsgRes
TZWCCImplThermoMode::eHandleCCMsg(  const   tCIDLib::TCard1 c1Class
                                    , const tCIDLib::TCard1 c1Cmd
                                    , const TZWInMsg&       zwimIn)
{
    if ((c1Class == COMMAND_CLASS_THERMOSTAT_MODE) && (c1Cmd == THERMOSTAT_MODE_REPORT))
    {
        // Get the mode out, mask off reserved bits
        tCIDLib::TCard4 c4Mode = zwimIn.c1CCByteAt(2) & 0x1F;

        // Convert to our enum value. If not valid, set it to unknown
        tZWaveUSB3Sh::EThermoModes eMode = tZWaveUSB3Sh::eAltNumEThermoModes(c4Mode);
        if ((eMode != tZWaveUSB3Sh::EThermoModes::Count)
        &&  ((0x1UL << tCIDLib::c4EnumOrd(eMode)) & m_c4ModeMask))
            bWriteFld(m_c4FldId_Mode, tZWaveUSB3Sh::strXlatEThermoModes(eMode));
        else
            bWriteFld(m_c4FldId_Mode, TString(L"Unknown"));
        return tZWaveUSB3Sh::ECCMsgRes_Value;
    }
    return tZWaveUSB3Sh::ECCMsgRes_Unhandled;
}



// On value or error state change, just call our helper to store the new state
tCIDLib::TVoid TZWCCImplThermoMode::EnteredErrState()
{
    SetFieldErr(m_c4FldId_Mode);
    TParent::EnteredErrState();
}




// ---------------------------------------------------------------------------
//   CLASS: TZWCCImplThermoFanMode
//  PREFIX: zwcci
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWCCImplThermoFanMode: Constructors and Destructor
// ---------------------------------------------------------------------------
TZWCCImplThermoFanMode::
TZWCCImplThermoFanMode(TZWUnitInfo* const  punitiThis, const TString& strFldName) :

    TZWCCImpl(punitiThis, tZWaveUSB3Sh::ECClasses::ThermoFanMode)
    , m_c4FldId_Mode(kCIDLib::c4MaxCard)
    , m_c4ModeMask(0)
    , m_strFldName(strFldName)
{
    strDesc(L"ThermoFanMode");

    //
    //  As a default add write access to whatever the base class set, if we are
    //  always on.
    //
    SetWriteAccess(punitiThis->bAlwaysOn());
}

TZWCCImplThermoFanMode::~TZWCCImplThermoFanMode()
{
}


// ---------------------------------------------------------------------------
//  TZWCCImplThermoFanMode: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TZWCCImplThermoFanMode::bOwnsFld(const tCIDLib::TCard4 c4FldId) const
{
    return (c4FldId == m_c4FldId_Mode);
}


//
//  We pull out and store any extra info from our class in the device info.
//
tCIDLib::TBoolean TZWCCImplThermoFanMode::bParseExtraInfo()
{
    if (!TParent::bParseExtraInfo())
        return kCIDLib::False;

    // Look up our modes list
    TString strModes;
    if (!bFindExtraVal(L"Modes", strModes)
    ||  !strModes.bToCard4(m_c4ModeMask, tCIDLib::ERadices::Auto))
    {
        facZWaveUSB3Sh().LogTraceErr
        (
            L"%(1)/%(2)- No/bad thermo fan mode list in extra info"
            , strUnitName()
            , strDesc()
        );
        return kCIDLib::False;
    }

    //
    //  Let's convert this into a limit string. We iterate the thermo modes and use
    //  the ordinal as a bit shifter to test against the above mask.
    //
    m_strLimits.Clear();
    if (m_c4ModeMask)
    {
        TString strLims(L"Enum: Unknown", 128UL);
        tZWaveUSB3Sh::EThermoFModes eMode = tZWaveUSB3Sh::EThermoFModes::Min;
        while (eMode <= tZWaveUSB3Sh::EThermoFModes::Max)
        {
            tCIDLib::TCard4 c4Bit = 0x1;
            c4Bit <<= tCIDLib::TCard4(eMode);
            if (c4Bit & m_c4ModeMask)
            {
                strLims.Append(L", ");
                strLims.Append(tZWaveUSB3Sh::strXlatEThermoFModes(eMode));
            }
            eMode++;
        }
        m_strLimits = strLims;
    }

    return kCIDLib::True;
}


tCIDLib::TBoolean
TZWCCImplThermoFanMode::bStringFldChanged(  const   TString&                strFldName
                                            , const tCIDLib::TCard4         c4FldId
                                            , const TString&                strValue
                                            ,       tCQCKit::ECommResults&  eRes)
{
    tCIDLib::TBoolean bRes = kCIDLib::True;
    try
    {
        if (c4FldId == m_c4FldId_Mode)
        {
            //
            //  Convert the value to the appropriate Z-Wave value. We first convert
            //  it to our thermo mode enum, then get the alt value.
            //
            tZWaveUSB3Sh::EThermoFModes eMode = tZWaveUSB3Sh::eXlatEThermoFModes(strValue);
            if (eMode == tZWaveUSB3Sh::EThermoFModes::Count)
            {
                // Shouldn't happen since we have a limit on the field but just in case
                eRes = tCQCKit::ECommResults::ValueRejected;
            }
             else
            {
                // This should never fail but it will throw if it does
                tCIDLib::TCard1 c1Val = tCIDLib::TCard1
                (
                    tZWaveUSB3Sh::i4AltNumEThermoFModes(eMode)
                );

                try
                {
                    // Ask for an ack since this is a command and there won't be a reply
                    TZWOutMsg zwomOut;
                    facZWaveUSB3Sh().BuildCCMsg
                    (
                        zwomOut
                        , unitiOwner()
                        , COMMAND_CLASS_THERMOSTAT_FAN_MODE
                        , THERMOSTAT_FAN_MODE_SET
                        , 0
                        , c1Val
                        , tZWaveUSB3Sh::EMsgPrios::Command
                        , kCIDLib::True
                    );
                    SendWaitAck(zwomOut);

                    // If configured to do so, then do a read back of the value
                    if (bSendReadAfterWrite())
                        SendValueQuery();
                }

                catch(TError& errToCatch)
                {
                    errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                    throw;
                }

                eRes = tCQCKit::ECommResults::Success;
            }
        }
         else
        {
            // Not one of ours
            bRes = kCIDLib::False;
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
    return bRes;
}


tCIDLib::TVoid
TZWCCImplThermoFanMode::QueryFldDefs(  tCQCKit::TFldDefList&   colAddTo
                                    , TCQCFldDef&           flddTmp) const
{
    flddTmp.Set
    (
        m_strFldName
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::ReadWrite
        , m_strLimits
    );
    colAddTo.objAdd(flddTmp);
}


tCIDLib::TVoid TZWCCImplThermoFanMode::SendValueQuery()
{
    TZWOutMsg zwomOut;
    facZWaveUSB3Sh().BuildCCMsg
    (
        zwomOut
        , unitiOwner()
        , COMMAND_CLASS_THERMOSTAT_FAN_MODE
        , THERMOSTAT_FAN_MODE_GET
        , 0
        , tZWaveUSB3Sh::EMsgPrios::Async
        , kCIDLib::False
    );
    SendWaitAck(zwomOut);
}


tCIDLib::TVoid TZWCCImplThermoFanMode::StoreFldIds()
{
    m_c4FldId_Mode = c4LookupFldId(m_strFldName);
}


// ---------------------------------------------------------------------------
//  TZWCCImplThermoFanMode: Protected, inherited methods
// ---------------------------------------------------------------------------

tZWaveUSB3Sh::ECCMsgRes
TZWCCImplThermoFanMode::eHandleCCMsg(const  tCIDLib::TCard1 c1Class
                                    , const tCIDLib::TCard1 c1Cmd
                                    , const TZWInMsg&       zwimIn)
{
    if ((c1Class == COMMAND_CLASS_THERMOSTAT_FAN_MODE) && (c1Cmd == THERMOSTAT_FAN_MODE_REPORT))
    {
        // Get the mode out, mask off reserved bits
        tCIDLib::TCard4 c4Mode = zwimIn.c1CCByteAt(2) & 0xF;

        // Convert to our enum value. If not valid, set it to unknown
        tZWaveUSB3Sh::EThermoFModes eMode = tZWaveUSB3Sh::eAltNumEThermoFModes(c4Mode);
        if ((eMode != tZWaveUSB3Sh::EThermoFModes::Count)
        &&  ((0x1UL << tCIDLib::c4EnumOrd(eMode)) & m_c4ModeMask))
            bWriteFld(m_c4FldId_Mode, tZWaveUSB3Sh::strXlatEThermoFModes(eMode));
        else
            bWriteFld(m_c4FldId_Mode, TString(L"Unknown"));
        return tZWaveUSB3Sh::ECCMsgRes_Value;
    }
    return tZWaveUSB3Sh::ECCMsgRes_Unhandled;
}


// On value or error state change, just call our helper to store the new state
tCIDLib::TVoid TZWCCImplThermoFanMode::EnteredErrState()
{
    SetFieldErr(m_c4FldId_Mode);
    TParent::EnteredErrState();
}



// ---------------------------------------------------------------------------
//   CLASS: TZWCCImplThermoFanState
//  PREFIX: zwcci
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWCCImplThermoFanState: Constructors and Destructor
// ---------------------------------------------------------------------------

// We keep the defaults set by the base class
TZWCCImplThermoFanState::
TZWCCImplThermoFanState(TZWUnitInfo* const  punitiThis, const TString& strFldName) :

    TZWCCImpl(punitiThis, tZWaveUSB3Sh::ECClasses::ThermoFanState)
    , m_c4FldId_State(kCIDLib::c4MaxCard)
    , m_strFldName(strFldName)
{
    strDesc(L"ThermoFanState");
}

TZWCCImplThermoFanState::~TZWCCImplThermoFanState()
{
}


// ---------------------------------------------------------------------------
//  TZWCCImplThermoFanState: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TZWCCImplThermoFanState::bOwnsFld(const tCIDLib::TCard4 c4FldId) const
{
    return (c4FldId == m_c4FldId_State);
}




tCIDLib::TVoid
TZWCCImplThermoFanState::QueryFldDefs(  tCQCKit::TFldDefList&   colAddTo
                                        , TCQCFldDef&           flddTmp) const
{
    flddTmp.Set(m_strFldName, tCQCKit::EFldTypes::String, tCQCKit::EFldAccess::Read);
    colAddTo.objAdd(flddTmp);
}


tCIDLib::TVoid TZWCCImplThermoFanState::SendValueQuery()
{
    TZWOutMsg zwomOut;
    facZWaveUSB3Sh().BuildCCMsg
    (
        zwomOut
        , unitiOwner()
        , COMMAND_CLASS_THERMOSTAT_FAN_STATE
        , THERMOSTAT_FAN_STATE_GET
        , 0
        , tZWaveUSB3Sh::EMsgPrios::Async
        , kCIDLib::False
    );
    SendWaitAck(zwomOut);
}


tCIDLib::TVoid TZWCCImplThermoFanState::StoreFldIds()
{
    m_c4FldId_State = c4LookupFldId(m_strFldName);
    SetFieldErr(m_c4FldId_State);
}


// ---------------------------------------------------------------------------
//  TZWCCImplThermoFanState: Protected, inherited methods
// ---------------------------------------------------------------------------

tZWaveUSB3Sh::ECCMsgRes
TZWCCImplThermoFanState::eHandleCCMsg(  const   tCIDLib::TCard1 c1Class
                                        , const tCIDLib::TCard1 c1Cmd
                                        , const TZWInMsg&       zwimIn)
{
    if ((c1Class == COMMAND_CLASS_THERMOSTAT_FAN_STATE)
    &&  (c1Cmd == THERMOSTAT_FAN_STATE_REPORT))
    {
        tCIDLib::TCard4 c4State = zwimIn.c1CCByteAt(2) & 0xF;

        // Convert to our enum value
        tZWaveUSB3Sh::EThermoFStates eState = tZWaveUSB3Sh::eAltNumEThermoFStates(c4State);
        if (eState == tZWaveUSB3Sh::EThermoFStates::Count)
            SetErrState();
        else
            bWriteFld(m_c4FldId_State, tZWaveUSB3Sh::strXlatEThermoFStates(eState));

        return tZWaveUSB3Sh::ECCMsgRes_Value;
    }
    return tZWaveUSB3Sh::ECCMsgRes_Unhandled;
}


// On value or error state change, just call our helper to store the new state
tCIDLib::TVoid TZWCCImplThermoFanState::EnteredErrState()
{
    SetFieldErr(m_c4FldId_State);
    TParent::EnteredErrState();
}





// ---------------------------------------------------------------------------
//   CLASS: TZWCCImplThermoState
//  PREFIX: zwcci
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWCCImplThermoState: Constructors and Destructor
// ---------------------------------------------------------------------------

// We keep the defaults set by the base class
TZWCCImplThermoState::
TZWCCImplThermoState(TZWUnitInfo* const  punitiThis, const TString& strFldName) :

    TZWCCImpl(punitiThis, tZWaveUSB3Sh::ECClasses::ThermoState)
    , m_c4FldId_State(kCIDLib::c4MaxCard)
    , m_strFldName(strFldName)
{
    strDesc(L"ThermoState");
}

TZWCCImplThermoState::~TZWCCImplThermoState()
{
}


// ---------------------------------------------------------------------------
//  TZWCCImplThermoState: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TZWCCImplThermoState::bOwnsFld(const tCIDLib::TCard4 c4FldId) const
{
    return (c4FldId == m_c4FldId_State);
}


tCIDLib::TVoid
TZWCCImplThermoState::QueryFldDefs( tCQCKit::TFldDefList&   colAddTo
                                    , TCQCFldDef&           flddTmp) const
{
    flddTmp.Set(m_strFldName, tCQCKit::EFldTypes::String, tCQCKit::EFldAccess::Read);
    colAddTo.objAdd(flddTmp);
}


tCIDLib::TVoid TZWCCImplThermoState::SendValueQuery()
{
    TZWOutMsg zwomOut;
    facZWaveUSB3Sh().BuildCCMsg
    (
        zwomOut
        , unitiOwner()
        , COMMAND_CLASS_THERMOSTAT_OPERATING_STATE
        , THERMOSTAT_OPERATING_STATE_GET
        , 0
        , tZWaveUSB3Sh::EMsgPrios::Async
        , kCIDLib::False
    );
    SendWaitAck(zwomOut);
}


tCIDLib::TVoid TZWCCImplThermoState::StoreFldIds()
{
    m_c4FldId_State = c4LookupFldId(m_strFldName);
    SetFieldErr(m_c4FldId_State);
}


// ---------------------------------------------------------------------------
//  TZWCCImplThermoState: Protected, inherited methods
// ---------------------------------------------------------------------------

tZWaveUSB3Sh::ECCMsgRes
TZWCCImplThermoState::eHandleCCMsg( const   tCIDLib::TCard1 c1Class
                                    , const tCIDLib::TCard1 c1Cmd
                                    , const TZWInMsg&       zwimIn)
{
    if ((c1Class == COMMAND_CLASS_THERMOSTAT_OPERATING_STATE)
    &&  (c1Cmd == THERMOSTAT_OPERATING_STATE_REPORT))
    {
        tCIDLib::TCard4 c4State = zwimIn.c1CCByteAt(2);

        // Convert to our enum value
        tZWaveUSB3Sh::EThermoStates eState = tZWaveUSB3Sh::eAltNumEThermoStates(c4State);
        if (eState == tZWaveUSB3Sh::EThermoStates::Count)
            SetErrState();
        else
            bWriteFld(m_c4FldId_State, tZWaveUSB3Sh::strXlatEThermoStates(eState));

        return tZWaveUSB3Sh::ECCMsgRes_Value;
    }
    return tZWaveUSB3Sh::ECCMsgRes_Unhandled;
}


// On value or error state change, just call our helper to store the new state
tCIDLib::TVoid TZWCCImplThermoState::EnteredErrState()
{
    SetFieldErr(m_c4FldId_State);
    TParent::EnteredErrState();
}



// ---------------------------------------------------------------------------
//   CLASS: TZWCCImplThermoSP
//  PREFIX: zwcci
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWCCImplThermoSP: Constructors and Destructor
// ---------------------------------------------------------------------------
TZWCCImplThermoSP::
TZWCCImplThermoSP(          TZWUnitInfo* const  punitiThis
                    , const TString&            strFldName
                    , const tCIDLib::TBoolean   bHighSP
                    , const tCIDLib::TBoolean   bFScale) :

    TZWCCImpl(punitiThis, tZWaveUSB3Sh::ECClasses::ThermoSetPnt)
    , m_bFScale(bFScale)
    , m_bHighSP(bHighSP)
    , m_c4FldId_SP(kCIDLib::c4MaxCard)
    , m_i4MaxSPValF(0)
    , m_i4MinSPValF(0)
    , m_strFldName(strFldName)
{
    if (m_bHighSP)
        strDesc(L"ThermoHSP");
    else
        strDesc(L"ThermoLSP");

    // Based on scale set reasonable min/max default values
    if (m_bFScale)
    {
        if (m_bHighSP)
        {
            m_i4MinSPValF = 65;
            m_i4MaxSPValF = 100;
        }
         else
        {
            m_i4MinSPValF = 30;
            m_i4MaxSPValF = 80;
        }
    }
     else
    {
        if (m_bHighSP)
        {
            m_i4MinSPValF = 20;
            m_i4MaxSPValF = 40;
        }
         else
        {
            m_i4MinSPValF = 0;
            m_i4MaxSPValF = 28;
        }
    }


    //
    //  As a default add write access to whatever the base class set, if we are
    //  always on, else disable it until such time as the owning unit might choose
    //  to enable writes.
    //
    SetWriteAccess(punitiThis->bAlwaysOn());
}

TZWCCImplThermoSP::~TZWCCImplThermoSP()
{
}


// ---------------------------------------------------------------------------
//  TZWCCImplThermoSP: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TZWCCImplThermoSP::bIntFldChanged(  const   TString&                strFldName
                                    , const tCIDLib::TCard4         c4FldId
                                    , const tCIDLib::TInt4          i4Val
                                    ,       tCQCKit::ECommResults&  eRes)
{
    tCIDLib::TBoolean bRes = kCIDLib::True;
    try
    {
        if (c4FldId == m_c4FldId_SP)
        {
            try
            {
                //
                //  To be safe we use two byte format. Our format is either 0xA
                //  which is C scale 2 bytes, or 2, which is F scale 2 bytes.
                //
                TZWOutMsg zwomOut;
                facZWaveUSB3Sh().BuildCCMsg
                (
                    zwomOut
                    , unitiOwner()
                    , COMMAND_CLASS_THERMOSTAT_SETPOINT
                    , THERMOSTAT_SETPOINT_SET
                    , 0
                    , m_bHighSP ? 2 : 1
                    , m_bFScale ? 0xA : 0x2
                    , tCIDLib::TInt1(i4Val >> 8)
                    , tCIDLib::TInt1(i4Val)
                    , tZWaveUSB3Sh::EMsgPrios::Command
                    , kCIDLib::False
                );
                SendWaitAck(zwomOut);

                // If configured to do so, then do a read back of the value
                if (bSendReadAfterWrite())
                    SendValueQuery();
            }

            catch(TError& errToCatch)
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                throw;
            }

            eRes = tCQCKit::ECommResults::Success;
        }
         else
        {
            // Not one of ours
            bRes = kCIDLib::False;
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
    return bRes;
}


tCIDLib::TBoolean TZWCCImplThermoSP::bOwnsFld(const tCIDLib::TCard4 c4FldId) const
{
    return (c4FldId == m_c4FldId_SP);
}


//
//  We pull out and store any extra info from our class in the device info. In addition
//  to whatever our parent classes support, we can get set point min/max range info.
//
tCIDLib::TBoolean TZWCCImplThermoSP::bParseExtraInfo()
{
    if (!TParent::bParseExtraInfo())
        return kCIDLib::False;

    // We need to get the value appropriate for our set point type
    TString strMin;
    if (bFindExtraVal(m_bHighSP ? L"HighRange" : L"LowRange", strMin))
    {
        TString strMax;
        if (!strMin.bSplit(strMax, kCIDLib::chSpace))
        {
            facZWaveUSB3Sh().LogTraceErr
            (
                L"%(1)/%(2)- Bad thermo SP range in extra info", strUnitName(), strDesc()
            );
            return kCIDLib::False;
        }

        strMin.StripWhitespace();
        strMax.StripWhitespace();
        tCIDLib::TInt4 i4Min, i4Max;

        if (!strMin.bToInt4(i4Min, tCIDLib::ERadices::Dec)
        ||  !strMax.bToInt4(i4Max, tCIDLib::ERadices::Dec))
        {
            facZWaveUSB3Sh().LogTraceErr
            (
                L"%(1)/%(2)- Bad thermo SP range in extra info", strUnitName(), strDesc()
            );
            return kCIDLib::False;
        }

        if (i4Max < i4Min)
        {
            facZWaveUSB3Sh().LogTraceErr
            (
                L"%(1)/%(2)- Bad (reversed) thermo SP range in extra info"
                , strUnitName()
                , strDesc()
            );
            return kCIDLib::False;
        }

        // Looks good so store them
        m_i4MinSPValF = i4Min;
        m_i4MaxSPValF = i4Max;
    }


    return kCIDLib::True;
}


tCIDLib::TVoid
TZWCCImplThermoSP::QueryFldDefs(tCQCKit::TFldDefList&   colAddTo
                                , TCQCFldDef&           flddTmp) const
{
    //
    //  Set up our limits range. If we are not in F scale, we need to convert the
    //  values, which are always in F scale.
    //
    tCIDLib::TInt4 i4Min;
    tCIDLib::TInt4 i4Max;
    if (m_bFScale)
    {
        i4Min = m_i4MinSPValF;
        i4Max = m_i4MaxSPValF;
    }
     else
    {
        tCIDLib::TFloat8 f8Val;

        f8Val = m_i4MinSPValF;
        f8Val -= 32;
        f8Val *= 5.0 / 9.0;
        i4Min = TMathLib::i4Round(f8Val);

        f8Val = m_i4MaxSPValF;
        f8Val -= 32;
        f8Val *= 5.0 / 9.0;
        i4Max = TMathLib::i4Round(f8Val);
    }

    TString strLimits(L"Range: ", 16UL);
    strLimits.AppendFormatted(i4Min);
    strLimits.Append(kCIDLib::chComma);
    strLimits.AppendFormatted(i4Max);

    flddTmp.Set
    (
        m_strFldName
        , tCQCKit::EFldTypes::Int
        , tCQCKit::EFldAccess::ReadWrite
        , m_bHighSP ? tCQCKit::EFldSTypes::HighSetPnt : tCQCKit::EFldSTypes::LowSetPnt
        , strLimits
    );
    colAddTo.objAdd(flddTmp);
}


tCIDLib::TVoid TZWCCImplThermoSP::StoreFldIds()
{
    m_c4FldId_SP = c4LookupFldId(m_strFldName);
    SetFieldErr(m_c4FldId_SP);
}


// ---------------------------------------------------------------------------
//  TZWCCImplThermoSP: Public, non-virtual methods
// ---------------------------------------------------------------------------


// The owning uniit can force our range if he knows what they are
tCIDLib::TVoid
TZWCCImplThermoSP::SetSPRange(  const   tCIDLib::TInt4  i4MinF
                                , const tCIDLib::TInt4  i4MaxF)
{
    m_i4MinSPValF = i4MinF;
    m_i4MaxSPValF = i4MaxF;
}


// ---------------------------------------------------------------------------
//  TZWCCImplThermoSP: Protected, inherited methods
// ---------------------------------------------------------------------------

tZWaveUSB3Sh::ECCMsgRes
TZWCCImplThermoSP::eHandleCCMsg(const   tCIDLib::TCard1 c1Class
                                , const tCIDLib::TCard1 c1Cmd
                                , const TZWInMsg&       zwimIn)
{
    //
    //  We have to make sure we are seeing the right set point since we will see
    //  both.
    //
    if ((c1Class == COMMAND_CLASS_THERMOSTAT_SETPOINT)
    &&  (c1Cmd == THERMOSTAT_SETPOINT_REPORT))
    {
        const tCIDLib::TCard1 c1WhichSP((zwimIn.c1CCByteAt(2) & 0xF));

        // If not low or high, we don't handle that
        if ((c1WhichSP < 1) || (c1WhichSP > 2))
            return tZWaveUSB3Sh::ECCMsgRes_Unhandled;

        // If it's the low and we are high, or vice versa, ignore it
        if ((c1WhichSP == 1) && m_bHighSP)
            return tZWaveUSB3Sh::ECCMsgRes_Unhandled;
        if ((c1WhichSP == 2) && !m_bHighSP)
            return tZWaveUSB3Sh::ECCMsgRes_Unhandled;

        //
        //  This uses the same format as the multi-level sensor, so that CC provides
        //  a static to extract the info.
        //
        tCIDLib::TCard4 c4Precision;
        tCIDLib::TCard4 c4Scale;
        tCIDLib::TCard4 c4Bytes;
        tCIDLib::TCard4 c4RawVal;
        const tCIDLib::TBoolean bRes = TZWCCImplMLSensor::bExtractInfo
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
            if (facZWaveUSB3Sh().bTraceEnabled())
            {
                facZWaveUSB3Sh().LogTraceErr
                (
                    L"%(1)/%(2)- Failed to extract ML info for SP", strUnitName(), strDesc()
                );
            }
            SetErrState();
            return tZWaveUSB3Sh::ECCMsgRes_Value;
        }

        // We first convert to float for conversions below
        tCIDLib::TFloat8 f8Val = tCIDLib::TFloat8(c4RawVal);

        //
        //  If we have decimal digits, divide them out. We only show integral
        //  temps.
        //
        if (c4Precision)
            f8Val /= TMathLib::f8Power(10.0, tCIDLib::TFloat4(c4Precision));

        // If this is not in our desired temp scale, we need to convert
        if (m_bFScale && (c4Scale == 0))
        {
            // Convert from C to F
            f8Val *= 9.0 / 5.0;
            f8Val += 32;
        }
         else if (!m_bFScale && (c4Scale == 1))
        {
            // Convert from F to C
            f8Val -= 32;
            f8Val *= 5.0 / 9.0;
        }

        // Now round to the nearest int and store
        tCIDLib::TInt4 i4Val = TMathLib::i4Round(f8Val);
        bWriteFld(m_c4FldId_SP, i4Val);

        return tZWaveUSB3Sh::ECCMsgRes_Value;
    }
    return tZWaveUSB3Sh::ECCMsgRes_Unhandled;
}


// On value or error state change, just call our helper to store the new state
tCIDLib::TVoid TZWCCImplThermoSP::EnteredErrState()
{
    SetFieldErr(m_c4FldId_SP);
    TParent::EnteredErrState();
}


tCIDLib::TVoid TZWCCImplThermoSP::SendValueQuery()
{
    TZWOutMsg zwomOut;
    facZWaveUSB3Sh().BuildCCMsg
    (
        zwomOut
        , unitiOwner()
        , COMMAND_CLASS_THERMOSTAT_SETPOINT
        , THERMOSTAT_SETPOINT_GET
        , 0
        , m_bHighSP ? 2 : 1
        , tZWaveUSB3Sh::EMsgPrios::Async
        , kCIDLib::False
    );
    SendWaitAck(zwomOut);
}

