//
// FILE NAME: ZWaveUSB3Sh_CCImpl_DoorLock.cpp
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
//  The implementation of the door lock oriented CC Impl classes.
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
RTTIDecls(TZWCCImplDLockState, TZWCCImplBin)
RTTIDecls(TZWCCImplDLockStateF, TZWCCImplDLockState)


// ---------------------------------------------------------------------------
//   CLASS: TZWCCImplDLockState
//  PREFIX: zwcci
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWCCImplDLockState: Constructors and Destructor
// ---------------------------------------------------------------------------
TZWCCImplDLockState::
TZWCCImplDLockState(        TZWUnitInfo* const  punitiThis
                    , const tCIDLib::TCard1     c1EPId) :

    TZWCCImplBin(punitiThis, tZWaveUSB3Sh::ECClasses::DoorLock, c1EPId)
{
    strDesc(L"LockState");
    SetAccess(tZWaveUSB3Sh::EUnitAcc::RWWithFollowup, tZWaveUSB3Sh::EUnitAcc::RWWithFollowup);
}

TZWCCImplDLockState::~TZWCCImplDLockState()
{
}

// ---------------------------------------------------------------------------
//  TZWCCImplDLockState: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid TZWCCImplDLockState::SendValueQuery()
{
    // Sanity check
    if (!bIsReadable())
    {
        facZWaveUSB3Sh().LogTraceErr
        (
            L"SendValueQuery() called for non-readable lock %(1)", strUnitName()
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
            , COMMAND_CLASS_DOOR_LOCK
            , DOOR_LOCK_OPERATION_GET
            , DOOR_LOCK_OPERATION_REPORT
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
//  TZWCCImplDLockState: Public, non-virtual methods
// ---------------------------------------------------------------------------

tCQCKit::ECommResults TZWCCImplDLockState::eSendValue(const tCIDLib::TBoolean bToSend)
{
    // Sanity check
    if (!bIsWriteable())
    {
        facZWaveUSB3Sh().LogTraceErr
        (
            L"SendValue() called for non-writeable lock %(1)", strUnitName()
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
            , COMMAND_CLASS_DOOR_LOCK
            , DOOR_LOCK_OPERATION_SET
            , 0
            , bToSend ? 0xFF : 0
            , tZWaveUSB3Sh::EMsgPrios::Command
            , kCIDLib::True
        );
        SendWaitAck(zwomOut);

        // Assume for now it worked and set our state to match
        bSetBinState(bToSend, tZWaveUSB3Sh::EValSrcs::Driver);

        // Send a followup read if needed
        if (bSendReadAfterWrite())
        {
            facZWaveUSB3Sh().BuildCCMsg
            (
                zwomOut
                , unitiOwner()
                , COMMAND_CLASS_DOOR_LOCK
                , DOOR_LOCK_OPERATION_GET
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
//  TZWCCImplDLockState: Protected, inherited methods
// ---------------------------------------------------------------------------

tZWaveUSB3Sh::ECCMsgRes
TZWCCImplDLockState::eHandleCCMsg(  const   tCIDLib::TCard1 c1Class
                                    , const tCIDLib::TCard1 c1Cmd
                                    , const TZWInMsg&       zwimIn)
{
    if ((c1Class == COMMAND_CLASS_DOOR_LOCK)
    &&  (c1Cmd == DOOR_LOCK_OPERATION_REPORT))
    {
        //
        //  Get the lock state out and store it, seeing if this is a change. Anything
        //  other than 0xFF means unsecured in some way or another. 0xFE means unknown
        //  in which case we ignore it.
        //
        const TZWUnitInfo& unitiUs = unitiOwner();
        const tCIDLib::TCard1 c1StateVal = zwimIn.c1CCByteAt(2);

        // 0xFE means unknown. Anything else besides 0xFF means unlocked in some way
        if (c1StateVal == 0xFE)
        {
            if (facZWaveUSB3Sh().bLowTrace())
                facZWaveUSB3Sh().LogTraceMsg(L"Lock %(1) reported unknown state", strUnitName());
            SetErrState();
        }
        else
            bSetBinState(c1StateVal == 0xFF, tZWaveUSB3Sh::EValSrcs::Unit);

        return tZWaveUSB3Sh::ECCMsgRes_Value;
    }
    return tZWaveUSB3Sh::ECCMsgRes_Unhandled;
}




// ---------------------------------------------------------------------------
//   CLASS: TZWCCImplDLockStateF
//  PREFIX: zwcci
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWCCImplDLockStateF: Constructors and Destructor
// ---------------------------------------------------------------------------
TZWCCImplDLockStateF::
TZWCCImplDLockStateF(       TZWUnitInfo* const  punitiThis
                    ,const  TString&            strSubUnit
                    , const tCIDLib::TCard1     c1EPId) :

    TZWCCImplDLockState(punitiThis, c1EPId)
    , m_c4FldId_State(kCIDLib::c4MaxCard)
    , m_strSubUnit(strSubUnit)
{
    SetAccess(tZWaveUSB3Sh::EUnitAcc::Read, tZWaveUSB3Sh::EUnitAcc::Read);
}

TZWCCImplDLockStateF::~TZWCCImplDLockStateF()
{
}


// ---------------------------------------------------------------------------
//  TZWCCImplDLockStateF: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TZWCCImplDLockStateF::bBoolFldChanged(  const   TString&                strFldName
                                        , const tCIDLib::TCard4         c4FldId
                                        , const tCIDLib::TBoolean       bValue
                                        ,       tCQCKit::ECommResults&  eRes)
{
    tCIDLib::TBoolean bRes = kCIDLib::True;
    try
    {
        if (c4FldId == m_c4FldId_State)
        {
            eRes = eSendValue(bValue);
        }
         else
        {
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


tCIDLib::TBoolean TZWCCImplDLockStateF::bOwnsFld(const tCIDLib::TCard4 c4FldId) const
{
    return (c4FldId == m_c4FldId_State);
}


// We build our field name for subsequent use
tCIDLib::TBoolean TZWCCImplDLockStateF::bPrepare()
{
    if (!TParent::bPrepare())
        return kCIDLib::False;

    // Build up our field name and store it away
    m_strFldName = L"LOCK#";
    if (!m_strSubUnit.bIsEmpty())
    {
        m_strFldName.Append(m_strSubUnit);
        m_strFldName.Append(kCIDLib::chTilde);
    }
    m_strFldName.Append(strUnitName());

    return kCIDLib::True;
}


tCIDLib::TVoid
TZWCCImplDLockStateF::QueryFldDefs( tCQCKit::TFldDefList&   colAddTo
                                    , TCQCFldDef&           flddTmp) const
{
    flddTmp.Set
    (
        m_strFldName
        , tCQCKit::EFldTypes::Boolean
        , tCQCKit::EFldAccess::ReadWrite
        , tCQCKit::EFldSTypes::LockState
    );
    colAddTo.objAdd(flddTmp);
}


tCIDLib::TVoid TZWCCImplDLockStateF::StoreFldIds()
{
    m_c4FldId_State = c4LookupFldId(m_strFldName);

    // If readable, in error until we get a value
    if (bIsReadable())
        SetFieldErr(m_c4FldId_State);
}


// ---------------------------------------------------------------------------
//  TZWCCImplDLockStateF: Protected, inherited methods
// ---------------------------------------------------------------------------

// On value or error state change, just call our helper to store the new state
tCIDLib::TVoid TZWCCImplDLockStateF::EnteredErrState()
{
    SetFieldErr(m_c4FldId_State);
    TParent::EnteredErrState();
}


tCIDLib::TVoid
TZWCCImplDLockStateF::ValueChanged( const   tZWaveUSB3Sh::EValSrcs  eSource
                                    , const tCIDLib::TBoolean       bWasInErr
                                    , const tCIDLib::TInt4          i4ExtraInfo)
{
    //
    //  Update the field. If the value is the same as currently stored, nothing will
    //  happen.
    //
    bWriteFld(m_c4FldId_State, bState());
    TParent::ValueChanged(eSource, bWasInErr, i4ExtraInfo);
}

