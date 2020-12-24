//
// FILE NAME: ZWaveUSB3Sh_CCImpl_Basic.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/21/2017
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
//  This is the implementation classs for the Z-Wave Basic command class
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
RTTIDecls(TZWCCImplBasicLevel, TZWCCImplLevel)
RTTIDecls(TZWCCImplBasicLevelF, TZWCCImplBasicLevel)
RTTIDecls(TZWCCImplBasicSwitch, TZWCCImplBin)
RTTIDecls(TZWCCImplBasicSwitchF, TZWCCImplBasicSwitch)



// ---------------------------------------------------------------------------
//   CLASS: TZWCCImplBasicLevel
//  PREFIX: zwcci
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWCCImplBasicLevel: Constructors and Destructor
// ---------------------------------------------------------------------------

//
//  It's hard to say what the right default access should be here, so we just keep
//  what the base CC impl class set. It should be updated by the extra info or the
//  owning unit to what is appropriate. This is often used as a notification class
//  or as a regular class.
//
TZWCCImplBasicLevel::
TZWCCImplBasicLevel(        TZWUnitInfo* const  punitiThis
                    , const tCIDLib::TCard1     c1EPId) :

    TZWCCImplLevel(punitiThis, tZWaveUSB3Sh::ECClasses::Basic, c1EPId)
{
}

TZWCCImplBasicLevel::~TZWCCImplBasicLevel()
{
}


// ---------------------------------------------------------------------------
//  TZWCCImplBasicLevel: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TZWCCImplBasicLevel::SendValueQuery()
{
    // Sanity check
    if (!bIsReadable())
    {
        facZWaveUSB3Sh().LogTraceErr
        (
            L"SendValueQuery() called for non-readable basic level %(1)/%(2)"
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
            , COMMAND_CLASS_BASIC
            , BASIC_GET
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
//  TZWCCImplBasicLevel: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCQCKit::ECommResults TZWCCImplBasicLevel::eSendValue(const tCIDLib::TCard4 c4ToSend)
{
    // Sanity check
    if (!bIsWriteable())
    {
        facZWaveUSB3Sh().LogTraceErr
        (
            L"bSendValue() called for non-writeable basic level %(1)/%(2)"
            , strUnitName()
            , strDesc()
        );
        return tCQCKit::ECommResults::Access;
    }

    try
    {
        //
        //  This is a set, so ask for a trans ack and do it sync, with Command
        //  priority. We get a percent, which we have to convert to a Z-Wave
        //  dim level.
        //
        TZWOutMsg zwomOut;
        facZWaveUSB3Sh().BuildCCMsg
        (
            zwomOut
            , unitiOwner()
            , COMMAND_CLASS_BASIC
            , BASIC_SET
            , 0
            , facZWaveUSB3Sh().c1PercentToDim(c4ToSend)
            , tZWaveUSB3Sh::EMsgPrios::Command
            , kCIDLib::True
        );
        SendWaitAck(zwomOut);

        // Assume it worked for now and set our value to this, indicate not from unit
        bSetLevel(c4ToSend, tZWaveUSB3Sh::EValSrcs::Driver);

        // Send a followup if needed
        if (bSendReadAfterWrite())
        {
            facZWaveUSB3Sh().BuildCCMsg
            (
                zwomOut
                , unitiOwner()
                , COMMAND_CLASS_BASIC
                , BASIC_GET
                , 0
                , tZWaveUSB3Sh::EMsgPrios::Async
                , kCIDLib::False
            );
            SendMsgAsync(zwomOut);
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
    return tCQCKit::ECommResults::Success;
}


// ---------------------------------------------------------------------------
//  TZWCCImplBasicLevel: Protected, inherited methods
// ---------------------------------------------------------------------------

// We just watch for a basic set or basic report operation
tZWaveUSB3Sh::ECCMsgRes
TZWCCImplBasicLevel::eHandleCCMsg(  const   tCIDLib::TCard1 c1Class
                                    , const tCIDLib::TCard1 c1Cmd
                                    , const TZWInMsg&       zwimIn)
{
    if ((c1Class == COMMAND_CLASS_BASIC)
    &&  ((c1Cmd == BASIC_REPORT) || (c1Cmd == BASIC_SET)))
    {
        // Convert the percentage to a Z-Wave dim level and store it
        const tCIDLib::TCard4 c4Per = facZWaveUSB3Sh().c4DimToPercent(zwimIn.c1CCByteAt(2));

        if (c4Per == kCIDLib::c1MaxCard)
            SetErrState();
        else
            bSetLevel(c4Per, tZWaveUSB3Sh::EValSrcs::Unit);

        return tZWaveUSB3Sh::ECCMsgRes_Value;
    }
    return tZWaveUSB3Sh::ECCMsgRes_Unhandled;
}





// ---------------------------------------------------------------------------
//   CLASS: TZWCCImplBasicLevelF
//  PREFIX: zwcci
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWCCImplBasicLevelF: Constructors and Destructor
// ---------------------------------------------------------------------------
TZWCCImplBasicLevelF::
TZWCCImplBasicLevelF(       TZWUnitInfo* const  punitiThis
                    , const TString&            strFldName
                    , const tCQCKit::EFldSTypes eSemType
                    , const tCIDLib::TCard1     c1EPId) :

    TZWCCImplBasicLevel(punitiThis, c1EPId)
    , m_c4FldId_Level(kCIDLib::c4MaxCard)
    , m_eSemType(eSemType)
    , m_strFldName(strFldName)
{
}

TZWCCImplBasicLevelF::~TZWCCImplBasicLevelF()
{
}


// ---------------------------------------------------------------------------
//  TZWCCImplBasicLevel: Public, inherited methods
// ---------------------------------------------------------------------------

// A helper to react to a write to our fields
tCIDLib::TBoolean
TZWCCImplBasicLevelF::bCardFldChanged(  const   TString&                strFldName
                                        , const tCIDLib::TCard4         c4FldId
                                        , const tCIDLib::TCard4         c4Value
                                        ,       tCQCKit::ECommResults&  eRes)
{
    tCIDLib::TBoolean bRes = kCIDLib::True;
    try
    {
        if (c4FldId == m_c4FldId_Level)
            eRes = eSendValue(c4Value);
        else
            bRes = kCIDLib::False;
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }

    return bRes;
}


tCIDLib::TBoolean TZWCCImplBasicLevelF::bOwnsFld(const tCIDLib::TCard4 c4FldId) const
{
    return (c4FldId == m_c4FldId_Level);
}


tCIDLib::TVoid
TZWCCImplBasicLevelF::QueryFldDefs( tCQCKit::TFldDefList&   colAddTo
                                    ,       TCQCFldDef&     flddTmp) const
{
    // We just have our one boolean state field
    flddTmp.Set
    (
        m_strFldName
        , tCQCKit::EFldTypes::Card
        , eToFldAccess()
        , m_eSemType
        , L"Range: 0, 100"
    );
    colAddTo.objAdd(flddTmp);
}


tCIDLib::TVoid
TZWCCImplBasicLevelF::StoreFldIds()
{
    // Just look up our one field using the name we were given
    m_c4FldId_Level = c4LookupFldId(m_strFldName);

    // If readable, initially set it to error state
    if (bIsReadable())
        SetFieldErr(m_c4FldId_Level);
}


// ---------------------------------------------------------------------------
//  TZWCCImplBasicLevelF: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TZWCCImplBasicLevelF::EnteredErrState()
{
    SetFieldErr(m_c4FldId_Level);
    TParent::EnteredErrState();
}


tCIDLib::TVoid
TZWCCImplBasicLevelF::ValueChanged( const   tZWaveUSB3Sh::EValSrcs  eSource
                                    , const tCIDLib::TBoolean       bWasInErr
                                    , const tCIDLib::TInt4          i4ExtraInfo)
{
    bWriteFld(m_c4FldId_Level, c4Level());
    TParent::ValueChanged(eSource, bWasInErr, i4ExtraInfo);
}



// ---------------------------------------------------------------------------
//   CLASS: TZWCCImplBasicSwitch
//  PREFIX: zwcci
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWCCImplBasicSwitch: Constructors and Destructor
// ---------------------------------------------------------------------------

//
//  It's hard to say what the right default access should be here, so we just keep
//  what the base CC impl class set. It should be updated by the extra info or the
//  owning unit to what is appropriate. This is often used as a notification class
//  and as a regular class.
//
TZWCCImplBasicSwitch::
TZWCCImplBasicSwitch(       TZWUnitInfo* const  punitiThis
                    , const tCIDLib::TCard1     c1EPId) :

    TZWCCImplBin(punitiThis, tZWaveUSB3Sh::ECClasses::Basic, c1EPId)
{
}

TZWCCImplBasicSwitch::~TZWCCImplBasicSwitch()
{
}


// ---------------------------------------------------------------------------
//  TZWCCImplBasicSwitch: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid TZWCCImplBasicSwitch::SendValueQuery()
{
    // Sanity check
    if (!bIsReadable())
    {
        facZWaveUSB3Sh().LogTraceErr
        (
            L"SendValueQuery() called for non-readable switch %(1)/%(2)"
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
            , COMMAND_CLASS_BASIC
            , BASIC_GET
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
//  TZWCCImplBasicSwitch: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCQCKit::ECommResults TZWCCImplBasicSwitch::eSendValue(const tCIDLib::TBoolean bToSend)
{
    // Sanity check
    if (!bIsWriteable())
    {
        facZWaveUSB3Sh().LogTraceErr
        (
            L"SendValue() called for non-writeable basic switch %(1)/%(2)"
            , strUnitName()
            , strDesc()
        );
        return tCQCKit::ECommResults::Access;
    }

    try
    {
        //
        //  This is a set, so ask for a trans ack and do it sync, with Command
        //  priority.
        //
        TZWOutMsg zwomOut;
        facZWaveUSB3Sh().BuildCCMsg
        (
            zwomOut
            , unitiOwner()
            , COMMAND_CLASS_BASIC
            , BASIC_SET
            , 0
            , bToSend ? 0xFF : 0
            , tZWaveUSB3Sh::EMsgPrios::Command
            , kCIDLib::True
        );
        SendWaitAck(zwomOut);

        // Assume it worked for now and set our state to this, indicate not from unit
        bSetBinState(bToSend, tZWaveUSB3Sh::EValSrcs::Driver);

        // Send a followup read if needed
        if (bSendReadAfterWrite())
        {
            facZWaveUSB3Sh().BuildCCMsg
            (
                zwomOut
                , unitiOwner()
                , COMMAND_CLASS_BASIC
                , BASIC_GET
                , 0
                , tZWaveUSB3Sh::EMsgPrios::Async
                , kCIDLib::False
            );
            SendMsgAsync(zwomOut);
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
    return tCQCKit::ECommResults::Success;
}


// ---------------------------------------------------------------------------
//  TZWCCImplBasicSwitch: Protected, inherited methods
// ---------------------------------------------------------------------------

//  We just watch for a basic set or basic report operation
tZWaveUSB3Sh::ECCMsgRes
TZWCCImplBasicSwitch::eHandleCCMsg( const   tCIDLib::TCard1 c1Class
                                    , const tCIDLib::TCard1 c1Cmd
                                    , const TZWInMsg&       zwimIn)
{
    if ((c1Class == COMMAND_CLASS_BASIC)
    &&  ((c1Cmd == BASIC_REPORT) || (c1Cmd == BASIC_SET)))
    {
        // Get the state and write it, seeing if this is a change
        const tCIDLib::TCard1 c1Val = zwimIn.c1CCByteAt(2);
        if ((c1Val > 0x63) && (c1Val < 0xFF))
            SetErrState();
        else
            bSetBinState(c1Val != 0, tZWaveUSB3Sh::EValSrcs::Unit);

        // We handled this one
        return tZWaveUSB3Sh::ECCMsgRes_Value;
    }
    return tZWaveUSB3Sh::ECCMsgRes_Unhandled;
}




// ---------------------------------------------------------------------------
//   CLASS: TZWCCImplBasicSwitchF
//  PREFIX: zwcci
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWCCImplBasicSwitchF: Constructors and Destructor
// ---------------------------------------------------------------------------
TZWCCImplBasicSwitchF::
TZWCCImplBasicSwitchF(          TZWUnitInfo* const      punitiThis
                        , const TString&                strFldName
                        , const tCQCKit::EFldSTypes     eSemType
                        , const tCIDLib::TCard1         c1EPId) :

    TZWCCImplBasicSwitch(punitiThis, c1EPId)
    , m_c4FldId_State(kCIDLib::c4MaxCard)
    , m_eSemType(eSemType)
    , m_strFldName(strFldName)
{
}

TZWCCImplBasicSwitchF::~TZWCCImplBasicSwitchF()
{
}


// ---------------------------------------------------------------------------
//  TZWCCImplBasicSwitchF: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TZWCCImplBasicSwitchF::bBoolFldChanged( const   TString&                strFldName
                                        , const tCIDLib::TCard4         c4FldId
                                        , const tCIDLib::TBoolean       bValue
                                        ,       tCQCKit::ECommResults&  eRes)
{
    tCIDLib::TBoolean bRes = kCIDLib::True;
    try
    {
        if (c4FldId == m_c4FldId_State)
            eRes = eSendValue(bValue);
        else
            bRes = kCIDLib::False;
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
    return bRes;
}


tCIDLib::TBoolean TZWCCImplBasicSwitchF::bOwnsFld(const tCIDLib::TCard4 c4FldId) const
{
    return (c4FldId == m_c4FldId_State);
}


tCIDLib::TVoid
TZWCCImplBasicSwitchF::QueryFldDefs(tCQCKit::TFldDefList&   colAddTo
                                    , TCQCFldDef&           flddTmp) const
{
    // We just have our one boolean state field
    flddTmp.Set(m_strFldName, tCQCKit::EFldTypes::Boolean, eToFldAccess(), m_eSemType);
    colAddTo.objAdd(flddTmp);
}


tCIDLib::TVoid TZWCCImplBasicSwitchF::StoreFldIds()
{
    // Just look up our one field using the name we were given
    m_c4FldId_State = c4LookupFldId(m_strFldName);

    // If readable set it initially to error state
    if (bIsReadable())
        SetFieldErr(m_c4FldId_State);
}


// ---------------------------------------------------------------------------
//  TZWCCImplBasicSwitchF: Protected, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid TZWCCImplBasicSwitchF::EnteredErrState()
{
    SetFieldErr(m_c4FldId_State);
    TParent::SetErrState();
}


tCIDLib::TVoid
TZWCCImplBasicSwitchF::ValueChanged(const   tZWaveUSB3Sh::EValSrcs  eSource
                                    , const tCIDLib::TBoolean       bWasInErr
                                    , const tCIDLib::TInt4          i4ExtraInfo)
{
    bWriteFld(m_c4FldId_State, bState());
    TParent::ValueChanged(eSource, bWasInErr, i4ExtraInfo);
}
