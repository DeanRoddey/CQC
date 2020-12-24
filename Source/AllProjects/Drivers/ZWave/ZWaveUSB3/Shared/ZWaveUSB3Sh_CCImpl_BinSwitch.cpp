//
// FILE NAME: ZWaveUSB3Sh_CCImpl_BinSwitch.cpp
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
//  This is the implementation classs for the Z-Wave Binary Switch command class
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
RTTIDecls(TZWCCImplBinSwitch, TZWCCImplBin)
RTTIDecls(TZWCCImplBinSwitchF, TZWCCImplBinSwitch)



// ---------------------------------------------------------------------------
//   CLASS: TZWCCImplBinSwitch
//  PREFIX: zwcci
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWCCImplBinSwitch: Constructors and Destructor
// ---------------------------------------------------------------------------
TZWCCImplBinSwitch::
TZWCCImplBinSwitch(         TZWUnitInfo* const      punitiThis
                    , const tCIDLib::TCard1         c1EPId) :

    TZWCCImplBin(punitiThis, tZWaveUSB3Sh::ECClasses::BinSwitch, c1EPId)
{
    //
    //  Set our default access to read/write. We won't undo any optional read
    //  flags if they are set, though that's not likely at this point.
    //
    SetAccess(tZWaveUSB3Sh::EUnitAcc::ReadWrite, tZWaveUSB3Sh::EUnitAcc::ReadWrite);
}

TZWCCImplBinSwitch::~TZWCCImplBinSwitch()
{
}


// ---------------------------------------------------------------------------
//  TZWCCImplBinSwitch: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid TZWCCImplBinSwitch::SendValueQuery()
{
    // Sanity check
    if (!bIsReadable())
    {
        facZWaveUSB3Sh().LogTraceErr
        (
            L"SendValueQuery() called for non-readable bin switch %(1)/%(2)"
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
            , COMMAND_CLASS_SWITCH_BINARY
            , SWITCH_BINARY_GET
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
//  TZWCCImplBinSwitch: Public, non-virtual methods
// ---------------------------------------------------------------------------

tCQCKit::ECommResults TZWCCImplBinSwitch::eSendValue(const tCIDLib::TBoolean bToSend)
{
    // Sanity check
    if (!bIsWriteable())
    {
        facZWaveUSB3Sh().LogTraceErr
        (
            L"bSendValue() called for non-writeable bin switch %(1)/%(2)"
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
            , COMMAND_CLASS_SWITCH_BINARY
            , SWITCH_BINARY_SET
            , 0
            , bToSend ? 0xFF : 0
            , tZWaveUSB3Sh::EMsgPrios::Command
            , kCIDLib::True
        );
        SendWaitAck(zwomOut);

        // For now, assume it worked and store the new state
        bSetBinState(bToSend, tZWaveUSB3Sh::EValSrcs::Driver);

        // Send a followup read if needed
        if (bSendReadAfterWrite())
        {
            facZWaveUSB3Sh().BuildCCMsg
            (
                zwomOut
                , unitiOwner()
                , COMMAND_CLASS_SWITCH_BINARY
                , SWITCH_BINARY_GET
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
//  TZWCCImplBinSwitch: Protected, inherited methods
// ---------------------------------------------------------------------------

// We just watch for a basic set or basic report operation
tZWaveUSB3Sh::ECCMsgRes
TZWCCImplBinSwitch::eHandleCCMsg(const  tCIDLib::TCard1 c1Class
                                , const tCIDLib::TCard1 c1Cmd
                                , const TZWInMsg&       zwimIn)
{
    if ((c1Class == COMMAND_CLASS_SWITCH_BINARY)
    &&  ((c1Cmd == SWITCH_BINARY_REPORT) || (c1Cmd == SWITCH_BINARY_SET)))
    {
        //  Get the state and write it, seeing if this is a change.
        const tCIDLib::TCard1 c1Val = zwimIn.c1CCByteAt(2);
        if ((c1Val > 1) && (c1Val < 0xFE))
            SetErrState();
        else
            bSetBinState(c1Val != 0, tZWaveUSB3Sh::EValSrcs::Unit);

        return tZWaveUSB3Sh::ECCMsgRes_Value;
    }
    return tZWaveUSB3Sh::ECCMsgRes_Unhandled;
}





// ---------------------------------------------------------------------------
//   CLASS: TZWCCImplBinSwitchF
//  PREFIX: zwcci
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWCCImplBinSwitchF: Constructors and Destructor
// ---------------------------------------------------------------------------
TZWCCImplBinSwitchF::
TZWCCImplBinSwitchF(        TZWUnitInfo* const  punitiThis
                    , const TString&            strFldName
                    , const tCQCKit::EFldSTypes eSemType
                    , const tCIDLib::TCard1     c1EPId) :

    TZWCCImplBinSwitch(punitiThis, c1EPId)
    , m_c4FldId_State(kCIDLib::c4MaxCard)
    , m_eSemType(eSemType)
    , m_strFldName(strFldName)
{
}

TZWCCImplBinSwitchF::~TZWCCImplBinSwitchF()
{
}


// ---------------------------------------------------------------------------
//  TZWCCImplBinSwitchF: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TZWCCImplBinSwitchF::bBoolFldChanged(const  TString&                strFldName
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


tCIDLib::TBoolean TZWCCImplBinSwitchF::bOwnsFld(const tCIDLib::TCard4 c4FldId) const
{
    return (c4FldId == m_c4FldId_State);
}


tCIDLib::TVoid
TZWCCImplBinSwitchF::QueryFldDefs(  tCQCKit::TFldDefList&   colAddTo
                                    , TCQCFldDef&           flddTmp) const
{
    // We just have our one boolean state field
    flddTmp.Set(m_strFldName, tCQCKit::EFldTypes::Boolean, eToFldAccess(), m_eSemType);
    colAddTo.objAdd(flddTmp);
}


tCIDLib::TVoid TZWCCImplBinSwitchF::StoreFldIds()
{
    // Just look up our one field using the name we were given
    m_c4FldId_State = c4LookupFldId(m_strFldName);

    // If readable set it initially to error state
    if (bIsReadable())
        SetFieldErr(m_c4FldId_State);
}


// ---------------------------------------------------------------------------
//  TZWCCImplBinSwitchF: Protected, inherited methods
// ---------------------------------------------------------------------------

// On value or error state change, just call our helper to store the new state
tCIDLib::TVoid TZWCCImplBinSwitchF::EnteredErrState()
{
    SetFieldErr(m_c4FldId_State);
    TParent::EnteredErrState();
}

tCIDLib::TVoid
TZWCCImplBinSwitchF::ValueChanged(  const   tZWaveUSB3Sh::EValSrcs  eSource
                                    , const tCIDLib::TBoolean       bWasInErr
                                    , const tCIDLib::TInt4          i4ExtraInfo)
{
    bWriteFld(m_c4FldId_State, bState());
    TParent::ValueChanged(eSource, bWasInErr, i4ExtraInfo);
}

