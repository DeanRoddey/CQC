//
// FILE NAME: ZWaveLevi2Sh_EntryCtrl.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/01/2014
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
//  This file provides the implementation for the entry control command
//  class derivative.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "ZWaveLevi2Sh_.hpp"



// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TZWCCEntryCtrl, TZWCmdClass)




// ---------------------------------------------------------------------------
//   CLASS: TZWCCEntryCtrl
//  PREFIX: zwcc
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWCCEntryCtrl: Constructors Destructor
// ---------------------------------------------------------------------------
TZWCCEntryCtrl::TZWCCEntryCtrl(const TZWDevClass& zwdcInst) :

    TZWCmdClass(tZWaveLevi2Sh::ECClasses::DoorLock, zwdcInst, COMMAND_CLASS_DOOR_LOCK)
    , m_c4FldId_State(kCIDLib::c4MaxCard)
{
}

TZWCCEntryCtrl::~TZWCCEntryCtrl()
{
}


// ---------------------------------------------------------------------------
//  TZWCCEntryCtrl: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  Upon receipt of a general report, we see if it is something that is reporting
//  our lock state. If so, we store that new state. If this is a different state
//  than our field currently contains that will cause a trigger to be sent.
//
tCIDLib::TBoolean
TZWCCEntryCtrl::bHandleGenReport(const  tCIDLib::TCardList& fcolVals
                                , const TZWaveUnit&         unitMe
                                ,       MZWLeviSrvFuncs&    mzwsfLevi
                                , const tCIDLib::TCard4     c4FromInst)
{
    // Shouldn't get anything less than 3, but don't tempt fate
    const tCIDLib::TCard4 c4ValCnt = fcolVals.c4ElemCount();
    if (c4ValCnt < 3)
        return kCIDLib::False;

    if (c4FromInst != c4InstId())
        return kCIDLib::False;

    tCIDLib::TBoolean bRes = kCIDLib::False;
    switch (fcolVals[0])
    {
        case COMMAND_CLASS_BASIC :
            if (fcolVals[1] == BASIC_REPORT)
            {
                bRes = kCIDLib::True;
                StoreLockState
                (
                    unitMe
                    , mzwsfLevi
                    , fcolVals[2] != 0
                    , 0
                    , TCQCEvent::strVal_LockType_Other
                );
            }
            break;

        case COMMAND_CLASS_DOOR_LOCK :
            //
            //  If we have enough data, and it's a lock report, process it.
            //  Note that we don't send any code info here, and we set the
            //  lock type to 'other' so that the handlers if the trigger
            //  don't try to process the code.
            //
            if ((c4ValCnt >= 3)
            &&  ((fcolVals[1] == DOOR_LOCK_OPERATION_REPORT)
            ||   (fcolVals[1] == DOOR_LOCK_OPERATION_SET)))
            {
                bRes = kCIDLib::True;
                StoreLockState
                (
                    unitMe
                    , mzwsfLevi
                    , fcolVals[2] != 0
                    , 0
                    , TCQCEvent::strVal_LockType_Other
                );
            }
            break;

        case COMMAND_CLASS_ALARM :
            //
            //  We can also get the status indirectly via an alarm message,
            //  by looking at the alarm code. If it's an alarm get, then it
            //  only has the alarm type. Else it also has a level.
            //
            if ((fcolVals[1] == ALARM_GET) || (fcolVals[1] == ALARM_REPORT))
            {
                bRes = kCIDLib::True;

                if (fcolVals.c4ElemCount() == 9)
                {
                    // Some newer version I'm not sure of, worked out emperically
                    StoreLockState
                    (
                        unitMe, mzwsfLevi
                        , fcolVals[7] == 1
                        , 0
                        , TCQCEvent::strVal_LockType_Manual
                    );
                }
                 else if (fcolVals.c4ElemCount() == 10)
                {
                    //
                    //  Some newer version I'm not sure of, worked out emperically.
                    //  In this one we get the code as well. It's a code unlock, so the
                    //  state isn't sent. last byte is the code number.
                    //
                    StoreLockState
                    (
                        unitMe, mzwsfLevi
                        , kCIDLib::False
                        , fcolVals[9]
                        , TCQCEvent::strVal_LockType_Pad
                    );
                }
                 else if (fcolVals.c4ElemCount() == 4)
                {
                    // This is the original alarm report
                    tCIDLib::TCard4 c4Type = 0;
                    tCIDLib::TCard4 c4Level = 0;
                    if (c4ValCnt >= 3)
                        c4Type = fcolVals[2];

                    if (c4ValCnt >= 4)
                        c4Level = fcolVals[3];

                    tCIDLib::TBoolean bLock = kCIDLib::False;
                    switch(c4Type)
                    {
                        // These indicate a lock
                        case 0x12 :
                        case 0x15 :
                        case 0x18 :
                            // Fall through after setting this
                            bLock = kCIDLib::True;

                        // These indicate unlock
                        case 0x13 :
                        case 0x16 :
                        case 0x19 :
                            // We get here bLock set correctly
                            StoreLockState
                            (
                                unitMe
                                , mzwsfLevi
                                , bLock
                                , c4Level
                                , strType(c4Type)
                            );
                            break;

                        default :
                            break;
                    };
                }
            }
            break;


        default :
            break;
    };
    return bRes;
}


tCIDLib::TBoolean TZWCCEntryCtrl::bOwnsFld(const tCIDLib::TCard4 c4FldId) const
{
    return (c4FldId == m_c4FldId_State);
}


//
//  This is called when the driver wants us to actively query our field values and
//  store them away.
//
tCIDLib::TBoolean
TZWCCEntryCtrl::bQueryFldVals(TZWaveUnit& unitMe, MZWLeviSrvFuncs& mzwsfLevi)
{
    //
    //  If our basic class is us, then we have to send a basic get and will
    //  get back a basic report. Else do a lock get/report.
    //
    tCIDLib::TCard4 c4CmdClass;
    tCIDLib::TCard4 c4Get;
    tCIDLib::TCard4 c4Report;

    if (unitMe.eBaseClass() == eCmdClass())
    {
        c4CmdClass = COMMAND_CLASS_BASIC;
        c4Get      = BASIC_GET;
        c4Report   = BASIC_REPORT;
    }
     else
    {
        c4CmdClass = COMMAND_CLASS_DOOR_LOCK;
        c4Get      = DOOR_LOCK_OPERATION_GET;
        c4Report   = DOOR_LOCK_OPERATION_REPORT;
    }

    tCIDLib::TCardList fcolVals(16);
    mzwsfLevi.LeviSendUnitMsg(unitMe, *this, c4CmdClass, c4Get);
    if (mzwsfLevi.bLeviWaitUnitMsg(unitMe, *this, c4CmdClass, c4Report, fcolVals))
        bHandleGenReport(fcolVals, unitMe, mzwsfLevi, c4InstId());
    else
        mzwsfLevi.LeviSetFldErr(m_c4FldId_State);

    return kCIDLib::True;
}


//
//  Handle writes to our state field, to lock/unlock the door.
//
//  Even if this guy provides async reports, they are often so delayed that we
//  can't wait for them, and attempting to read back the current value will just
//  get the old value since it won't have completed the operation yet.
//
//  So we just store and send a trigger now assuming that transmission of the
//  msg indicates success. Not much else we can do. If the report later comes
//  back, it'll be the same state as what we stored and will not cause another
//  trigger.
//
tCQCKit::ECommResults
TZWCCEntryCtrl::eWriteBoolFld(  const   tCIDLib::TCard4     c4FldId
                                , const tCIDLib::TBoolean   bValue
                                ,       TZWaveUnit&         unitMe
                                ,       MZWLeviSrvFuncs&    mzwsfLevi)
{
    if (c4FldId != m_c4FldId_State)
        return tCQCKit::ECommResults::FieldNotFound;

    // Not likely, but check it
    if (!bCanWrite())
        return tCQCKit::ECommResults::Access;

    // We never send a basic set, since that is disallowed for locks
    tCIDLib::TCardList fcolVals(6);
    fcolVals.c4AddElement(COMMAND_CLASS_DOOR_LOCK);
    fcolVals.c4AddElement(DOOR_LOCK_OPERATION_SET);
    if (bValue)
        fcolVals.c4AddElement(255);
    else
        fcolVals.c4AddElement(0);

    mzwsfLevi.LeviSendUnitMsg(unitMe, *this, fcolVals);

    StoreLockState
    (
        unitMe
        , mzwsfLevi
        , bValue
        , 0
        , TCQCEvent::strVal_LockType_Other
    );

    return tCQCKit::ECommResults::Success;
}


// Make sure our fields are set to error state until good values are stored
tCIDLib::TVoid
TZWCCEntryCtrl::InitVals(const TZWaveUnit& unitMe,  MZWLeviSrvFuncs& mzwsfLevi)
{
    mzwsfLevi.LeviSetFldErr(m_c4FldId_State);
}


//
//
//  Add our fields to the list. We either add just a generic multi-level value, or
//  a standard V2 dimmer field, depending on configuration.
//
tCIDLib::TVoid
TZWCCEntryCtrl::QueryFldDefs(       tCQCKit::TFldDefList& colAddTo
                            ,       TCQCFldDef&           flddTmp
                            , const TZWaveUnit&           unitMe
                            ,       TString&              strTmp1
                            ,       TString&              strTmp2) const
{
    strTmp1 = L"LOCK#";
    AppendInstName(unitMe, kCIDLib::pszEmptyZStr, strTmp1);

    // We just add our one boolean field
    flddTmp.Set
    (
        strTmp1
        , tCQCKit::EFldTypes::Boolean
        , tCQCKit::EFldAccess::ReadWrite
        , tCQCKit::EFldSTypes::LockState
    );
    colAddTo.objAdd(flddTmp);
}



// We look up our field and store it away
tCIDLib::TVoid
TZWCCEntryCtrl::StoreFldIds(const   TZWaveUnit&         unitMe
                            , const MZWLeviSrvFuncs&    mzwsfLevi
                            ,       TString&            strTmp1
                            ,       TString&            strTmp2)
{
    strTmp1 = L"LOCK#";
    AppendInstName(unitMe, kCIDLib::pszEmptyZStr, strTmp1);
    m_c4FldId_State = mzwsfLevi.c4LeviFldIdFromName(strTmp1);
}


// ---------------------------------------------------------------------------
//  TZWCCEntryCtrl: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  Return a standard lock status trigger type string, based on the alarm we
//  got.
//
const TString& TZWCCEntryCtrl::strType(const tCIDLib::TCard4 c4AlarmType) const
{
    if ((c4AlarmType == 0x16) || (c4AlarmType == 0x15))
        return TCQCEvent::strVal_LockType_Manual;
    else if ((c4AlarmType == 0x19) || (c4AlarmType == 0x18))
        return TCQCEvent::strVal_LockType_Remote;
    else if ((c4AlarmType == 0x12) || (c4AlarmType == 0x13))
        return TCQCEvent::strVal_LockType_Pad;

    return TCQCEvent::strVal_LockType_Other;
};


//
//  Called to do the work to store a new lock state. If it's a different state
//  than what we already have, we send out a trigger.
//
tCIDLib::TVoid
TZWCCEntryCtrl::StoreLockState( const   TZWaveUnit&         unitMe
                                ,       MZWLeviSrvFuncs&    mzwsfLevi
                                , const tCIDLib::TBoolean   bState
                                , const tCIDLib::TCard4     c4Level
                                , const TString&            strType)
{
    if (!bCheckFldId(m_c4FldId_State, L"state", unitMe))
        return;

    // Store the new lock state value. Remember if it's a new state
    tCIDLib::TBoolean bChanged = mzwsfLevi.bLeviStoreBoolFld(m_c4FldId_State, bState);

    // Make sure we are now marked as having valid info
    bWaitInitVals(kCIDLib::False);

    // If the state chaned, then send a trigger
    if (bChanged)
    {
        TString strCode;
        strCode.AppendFormatted(c4Level);
        mzwsfLevi.LeviQEventTrig
        (
            tCQCKit::EStdDrvEvs::LockStatus
            , TString::strEmpty()
            , bState ? TCQCEvent::strVal_LockState_Locked
                     : TCQCEvent::strVal_LockState_Unlocked
            , unitMe.strName()
            , strCode
            , strType
        );
    }
}

