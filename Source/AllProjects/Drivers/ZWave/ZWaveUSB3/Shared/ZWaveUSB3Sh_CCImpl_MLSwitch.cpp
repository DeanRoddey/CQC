//
// FILE NAME: ZWaveUSB3Sh_CCMLSwitch.cpp
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
RTTIDecls(TZWCCImplMLLevel, TZWCCImplLevel)
RTTIDecls(TZWCCImplMLLevelF, TZWCCImplMLLevel)
RTTIDecls(TZWCCImplMLSwitch, TZWCCImplBin)
RTTIDecls(TZWCCImplMLSwitchF, TZWCCImplMLSwitch)


// ---------------------------------------------------------------------------
//   CLASS: TZWCCImplMLLevel
//  PREFIX: zwcci
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWCCImplMLLevel: Constructors and Destructor
// ---------------------------------------------------------------------------

// If always on, we enable write, in addition to whatever the base class set
TZWCCImplMLLevel::
TZWCCImplMLLevel(       TZWUnitInfo* const  punitiThis
                , const tCIDLib::TCard1     c1EPId) :

    TZWCCImplLevel
    (
        punitiThis, tZWaveUSB3Sh::ECClasses::MLSwitch, c1EPId
    )
{
    SetWriteAccess(punitiThis->bAlwaysOn());
}

TZWCCImplMLLevel::~TZWCCImplMLLevel()
{
}


// ---------------------------------------------------------------------------
//  TZWCCImplMLLevel: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid TZWCCImplMLLevel::SendValueQuery()
{
    // Sanity check
    if (!bIsReadable())
    {
        facZWaveUSB3Sh().LogTraceErr
        (
            L"SendValueQuery() called for non-readable ML switch level %(1)/%(2)"
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
            , COMMAND_CLASS_SWITCH_MULTILEVEL
            , SWITCH_MULTILEVEL_GET
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
//  TZWCCImplMLLevel: Public, non-virtual methods
// ---------------------------------------------------------------------------

tCQCKit::ECommResults TZWCCImplMLLevel::eSendValue(const tCIDLib::TCard4 c4ToSend)
{
    // Sanity check
    if (!bIsWriteable())
    {
        facZWaveUSB3Sh().LogTraceErr
        (
            L"SendValue() called for non-writeable ML switch level %(1)/%(2)"
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
            , COMMAND_CLASS_SWITCH_MULTILEVEL
            , SWITCH_MULTILEVEL_SET
            , 0
            , facZWaveUSB3Sh().c1PercentToDim(c4ToSend)
            , tZWaveUSB3Sh::EMsgPrios::Command
            , kCIDLib::True
        );
        SendWaitAck(zwomOut);

        // Assume for now it worked and set our state to match
        bSetLevel(c4ToSend, tZWaveUSB3Sh::EValSrcs::Driver);

        // Send a followup read if needed
        if (bSendReadAfterWrite())
        {
            facZWaveUSB3Sh().BuildCCMsg
            (
                zwomOut
                , unitiOwner()
                , COMMAND_CLASS_SWITCH_MULTILEVEL
                , SWITCH_MULTILEVEL_GET
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
//  TZWCCImplMLLevel: Protected, inherited methods
// ---------------------------------------------------------------------------

// We just watch for a multi-level switch report or set
tZWaveUSB3Sh::ECCMsgRes
TZWCCImplMLLevel::eHandleCCMsg( const   tCIDLib::TCard1 c1Class
                                , const tCIDLib::TCard1 c1Cmd
                                , const TZWInMsg&       zwimIn)
{
    if ((c1Class == COMMAND_CLASS_SWITCH_MULTILEVEL)
    &&  ((c1Cmd == SWITCH_MULTILEVEL_REPORT) || (c1Cmd == SWITCH_MULTILEVEL_SET)))
    {
        //
        //  Convert the percentage to a Z-Wave dim level and store it. No triggers
        //  involved for the level field.
        //
        const tCIDLib::TCard1 c1Val = zwimIn.c1CCByteAt(2);
        if ((c1Val > 0x63) && (c1Val < 0xFF))
            SetErrState();
        else
            bSetLevel(c1Val, tZWaveUSB3Sh::EValSrcs::Unit);

        return tZWaveUSB3Sh::ECCMsgRes_Value;
    }
    return tZWaveUSB3Sh::ECCMsgRes_Unhandled;
}





// ---------------------------------------------------------------------------
//   CLASS: TZWCCImplMLLevelF
//  PREFIX: zwcci
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWCCImplMLLevelF: Constructors and Destructor
// ---------------------------------------------------------------------------
TZWCCImplMLLevelF::
TZWCCImplMLLevelF(          TZWUnitInfo* const      punitiThis
                    , const TString&                strFldName
                    , const tCQCKit::EFldSTypes     eSemType
                    , const tCIDLib::TCard1         c1EPId) :

    TZWCCImplMLLevel(punitiThis, c1EPId)
    , m_bRangeForced(kCIDLib::False)
    , m_c4FldId_Level(kCIDLib::c4MaxCard)
    , m_c4RangeMin(0)
    , m_c4RangeMax(100)
    , m_eSemType(eSemType)
    , m_strFldName(strFldName)
{
}

TZWCCImplMLLevelF::~TZWCCImplMLLevelF()
{
}


// ---------------------------------------------------------------------------
//  TZWCCImplMLLevelF: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TZWCCImplMLLevelF::bCardFldChanged(  const   TString&               strFldName
                                    , const tCIDLib::TCard4         c4FldId
                                    , const tCIDLib::TCard4         c4Value
                                    ,       tCQCKit::ECommResults&  eRes)
{
    tCIDLib::TBoolean bRet = kCIDLib::True;
    try
    {
        if (c4FldId == m_c4FldId_Level)
            eRes = eSendValue(c4Value);
        else
            bRet = kCIDLib::False;
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }

    return bRet;
}


tCIDLib::TBoolean TZWCCImplMLLevelF::bOwnsFld(const tCIDLib::TCard4 c4FldId) const
{
    return (c4FldId == m_c4FldId_Level);
}


// If not already forced, look for some extra info and store it away
tCIDLib::TBoolean TZWCCImplMLLevelF::bParseExtraInfo()
{
    if (!TParent::bParseExtraInfo())
        return kCIDLib::False;

    if (!m_bRangeForced)
    {
        TString strMin;
        if (bFindExtraVal(L"Range", strMin))
        {
            TString strMax;
            if (!strMin.bSplit(strMax, kCIDLib::chSpace)
            ||  !strMin.bToCard4(m_c4RangeMin, tCIDLib::ERadices::Auto)
            ||  !strMax.bToCard4(m_c4RangeMax, tCIDLib::ERadices::Auto)
            ||  (m_c4RangeMin >= m_c4RangeMax))
            {
                facZWaveUSB3Sh().LogTraceErr
                (
                    L"Invalid Range info on ML switch, %(1)/%(2)"
                    , strUnitName()
                    , strDesc()
                );
                return kCIDLib::False;
            }
        }
    }

    return kCIDLib::True;
}

tCIDLib::TVoid
TZWCCImplMLLevelF::QueryFldDefs( tCQCKit::TFldDefList& colAddTo
                                , TCQCFldDef&         flddTmp) const
{
    // We just have our one field
    TString strLims;
    if ((m_c4RangeMin > 0) || (m_c4RangeMax < kCIDLib::c4MaxCard))
    {
        strLims = L"Range: ";
        strLims.AppendFormatted(m_c4RangeMin);
        strLims.Append(kCIDLib::chComma);
        strLims.AppendFormatted(m_c4RangeMax);
    }
    flddTmp.Set
    (
        m_strFldName, tCQCKit::EFldTypes::Card, eToFldAccess(), m_eSemType, strLims
    );
    colAddTo.objAdd(flddTmp);
}


tCIDLib::TVoid TZWCCImplMLLevelF::StoreFldIds()
{
    // Just look up our one field using the name we were given
    m_c4FldId_Level = c4LookupFldId(m_strFldName);

    // If readable set it initially to error state
    if (bIsReadable())
        SetFieldErr(m_c4FldId_Level);
}


// ---------------------------------------------------------------------------
//  TZWCCImplMLLevelF: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TZWCCImplMLLevelF::EnteredErrState()
{
    SetFieldErr(m_c4FldId_Level);
    TParent::EnteredErrState();
}


tCIDLib::TVoid
TZWCCImplMLLevelF::ValueChanged(const   tZWaveUSB3Sh::EValSrcs  eSource
                                , const tCIDLib::TBoolean       bWasInErr
                                , const tCIDLib::TInt4          i4ExtraInfo)
{
    bWriteFld(m_c4FldId_Level, c4Level());
    TParent::ValueChanged(eSource, bWasInErr, i4ExtraInfo);
}


// ---------------------------------------------------------------------------
//  TZWCCImplMLLevelF: Protected, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TZWCCImplMLLevelF::SetRange(const tCIDLib::TCard4 c4Min, const tCIDLib::TCard4 c4Max)
{
    m_bRangeForced = kCIDLib::True;
    m_c4RangeMin = c4Min;
    m_c4RangeMax = c4Max;
}



// ---------------------------------------------------------------------------
//   CLASS: TZWCCImplMLSwitch
//  PREFIX: zwcci
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWCCImplMLSwitch: Constructors and Destructor
// ---------------------------------------------------------------------------
TZWCCImplMLSwitch::
TZWCCImplMLSwitch(          TZWUnitInfo* const  punitiThis
                    , const tCIDLib::TCard1     c1EPId) :

    TZWCCImplBin(punitiThis, tZWaveUSB3Sh::ECClasses::MLSwitch, c1EPId)
{
}

TZWCCImplMLSwitch::~TZWCCImplMLSwitch()
{
}


// ---------------------------------------------------------------------------
//  TZWCCImplMLSwitch: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TZWCCImplMLSwitch::SendValueQuery()
{
    // Sanity check
    if (!bIsReadable())
    {
        facZWaveUSB3Sh().LogTraceErr
        (
            L"SendValueQuery() called for non-readable ML switch state %(1)/%(2)"
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
            , COMMAND_CLASS_SWITCH_MULTILEVEL
            , SWITCH_MULTILEVEL_GET
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
//  TZWCCImplMLSwitch: Public, inherited methods
// ---------------------------------------------------------------------------
tCQCKit::ECommResults TZWCCImplMLSwitch::eSendValue(const tCIDLib::TBoolean bToSend)
{
    // Sanity check
    if (!bIsWriteable())
    {
        facZWaveUSB3Sh().LogTraceErr
        (
            L"SendValue() called for non-writeable ML switch state %(1)/%(2)"
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
            , COMMAND_CLASS_SWITCH_MULTILEVEL
            , SWITCH_MULTILEVEL_SET
            , 0
            , tCIDLib::TCard1(bToSend ? 0xFF : 0)
            , tZWaveUSB3Sh::EMsgPrios::Command
            , kCIDLib::True
        );
        SendWaitAck(zwomOut);

        // Assume it worked for now and set our state to match
        bSetBinState(bToSend, tZWaveUSB3Sh::EValSrcs::Driver);

        // Send a followup read if needed
        if (bSendReadAfterWrite())
        {
            facZWaveUSB3Sh().BuildCCMsg
            (
                zwomOut
                , unitiOwner()
                , COMMAND_CLASS_SWITCH_MULTILEVEL
                , SWITCH_MULTILEVEL_GET
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
//  TZWCCImplMLSwitch: Protected, inherited methods
// ---------------------------------------------------------------------------

tZWaveUSB3Sh::ECCMsgRes
TZWCCImplMLSwitch::eHandleCCMsg(const   tCIDLib::TCard1 c1Class
                                , const tCIDLib::TCard1 c1Cmd
                                , const TZWInMsg&       zwimIn)
{
    if ((c1Class == COMMAND_CLASS_SWITCH_MULTILEVEL)
    &&  ((c1Cmd == SWITCH_MULTILEVEL_REPORT) || (c1Cmd == SWITCH_MULTILEVEL_SET)))
    {
        // Watch for the special unknown value
        const tCIDLib::TCard1 c1Val = zwimIn.c1CCByteAt(2);
        if (c1Val == 0xFE)
            SetErrState();
        else
            bSetBinState(c1Val != 0, tZWaveUSB3Sh::EValSrcs::Unit);

        return tZWaveUSB3Sh::ECCMsgRes_Value;
    }
    return tZWaveUSB3Sh::ECCMsgRes_Unhandled;
}




// ---------------------------------------------------------------------------
//   CLASS: TZWCCImplMLSwitchF
//  PREFIX: zwcci
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWCCImplMLSwitchF: Constructors and Destructor
// ---------------------------------------------------------------------------
TZWCCImplMLSwitchF::
TZWCCImplMLSwitchF(         TZWUnitInfo* const      punitiThis
                    , const TString&                strFldName
                    , const tCQCKit::EFldSTypes     eSemType
                    , const tCIDLib::TCard1         c1EPId) :

    TZWCCImplMLSwitch(punitiThis, c1EPId)
    , m_c4FldId_State(kCIDLib::c4MaxCard)
    , m_eSemType(eSemType)
    , m_strFldName(strFldName)
{
}

TZWCCImplMLSwitchF::~TZWCCImplMLSwitchF()
{
}


// ---------------------------------------------------------------------------
//  TZWCCImplMLSwitchF: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TZWCCImplMLSwitchF::bBoolFldChanged(const   TString&                strFldName
                                    , const tCIDLib::TCard4         c4FldId
                                    , const tCIDLib::TBoolean       bValue
                                    ,       tCQCKit::ECommResults&  eRes)
{
    tCIDLib::TBoolean bRet = kCIDLib::True;
    try
    {
        if (c4FldId == m_c4FldId_State)
            eRes = eSendValue(bValue);
        else
            bRet = kCIDLib::False;
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }

    return bRet;
}


tCIDLib::TBoolean TZWCCImplMLSwitchF::bOwnsFld(const tCIDLib::TCard4 c4FldId) const
{
    return (c4FldId == m_c4FldId_State);
}


tCIDLib::TVoid
TZWCCImplMLSwitchF::QueryFldDefs(tCQCKit::TFldDefList&   colAddTo
                                , TCQCFldDef&           flddTmp) const
{
    // We just have our one boolean state field
    flddTmp.Set(m_strFldName, tCQCKit::EFldTypes::Boolean, eToFldAccess(), m_eSemType);
    colAddTo.objAdd(flddTmp);
}


tCIDLib::TVoid TZWCCImplMLSwitchF::StoreFldIds()
{
    // Just look up our one field using the name we were given
    m_c4FldId_State = c4LookupFldId(m_strFldName);

    // If readable set it initially to error state
    if (bIsReadable())
        SetFieldErr(m_c4FldId_State);
}


// ---------------------------------------------------------------------------
//  TZWCCImplMLSwitchF: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TZWCCImplMLSwitchF::EnteredErrState()
{
    SetFieldErr(m_c4FldId_State);
    TParent::EnteredErrState();
}


tCIDLib::TVoid
TZWCCImplMLSwitchF::ValueChanged(const  tZWaveUSB3Sh::EValSrcs  eSource
                                , const tCIDLib::TBoolean       bWasInErr
                                , const tCIDLib::TInt4          i4ExtraInfo)
{
    bWriteFld(m_c4FldId_State, bState());
    TParent::ValueChanged(eSource, bWasInErr, i4ExtraInfo);
}

