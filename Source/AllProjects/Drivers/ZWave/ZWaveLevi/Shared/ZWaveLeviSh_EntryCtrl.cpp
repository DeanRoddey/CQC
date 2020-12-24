//
// FILE NAME: ZWaveLeviSh_EntryCtrl.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/12/2012
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
//  This file provides the implementation for the entry control oriented
//  unit types.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "ZWaveLeviSh_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
AdvRTTIDecls(TZWEntryCtrl,TZWaveUnit)


// ---------------------------------------------------------------------------
//  Local types and constants
// ---------------------------------------------------------------------------
namespace ZWaveLeviSh_EntryCtrl
{
    // -----------------------------------------------------------------------
    //  Persistent format versions
    // -----------------------------------------------------------------------
    const tCIDLib::TCard2   c2FmtEntryCtrlVer   = 1;
}



// ---------------------------------------------------------------------------
//   CLASS: TZWEntryCtrl
//  PREFIX: unit
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWEntryCtrl: Constructors and Destructor
// ---------------------------------------------------------------------------
TZWEntryCtrl::TZWEntryCtrl() :

    TZWaveUnit
    (
        GENERIC_TYPE_ENTRY_CONTROL
        , SPECIFIC_TYPE_DOOR_LOCK
        , L"EntryCtrl"
        , L"Entry Control"
        , tZWaveLeviSh::EUnitCaps
          (
            tZWaveLeviSh::EUnitCap_Readable
            | tZWaveLeviSh::EUnitCap_Writeable
            | tZWaveLeviSh::EUnitCap_Secure
            | tZWaveLeviSh::EUnitCap_Battery
          )
        , tZWaveLeviSh::EUnitCap_None
    )
    , m_c4FldId_State(kCIDLib::c4MaxCard)
{
}

TZWEntryCtrl::TZWEntryCtrl(const tCIDLib::TCard4 c4UnitId) :

    TZWaveUnit
    (
        GENERIC_TYPE_ENTRY_CONTROL
        , SPECIFIC_TYPE_DOOR_LOCK
        , L"EntryCtrl"
        , L"Entry Control"
        , tZWaveLeviSh::EUnitCaps
          (
            tZWaveLeviSh::EUnitCap_Readable
            | tZWaveLeviSh::EUnitCap_Writeable
            | tZWaveLeviSh::EUnitCap_Secure
            | tZWaveLeviSh::EUnitCap_Battery
          )
        , tZWaveLeviSh::EUnitCap_None
        , c4UnitId
    )
    , m_c4FldId_State(kCIDLib::c4MaxCard)
{
}

TZWEntryCtrl::TZWEntryCtrl(const TZWEntryCtrl& unitToCopy) :

    TZWaveUnit(unitToCopy)
    , m_c4FldId_State(unitToCopy.m_c4FldId_State)
{
}

TZWEntryCtrl::~TZWEntryCtrl()
{
}


// ---------------------------------------------------------------------------
//  TZWEntryCtrl: Public operators
// ---------------------------------------------------------------------------
TZWEntryCtrl& TZWEntryCtrl::operator=(const TZWEntryCtrl& unitSrc)
{
    if (this != &unitSrc)
    {
        TParent::operator=(unitSrc);
        m_c4FldId_State = unitSrc.m_c4FldId_State;
    }
    return *this;
}


// We don't have any of our own persisted members, so just call the parent for now
tCIDLib::TBoolean TZWEntryCtrl::operator==(const TZWEntryCtrl& unitSrc) const
{
    return TParent::operator==(unitSrc);
}


tCIDLib::TBoolean TZWEntryCtrl::operator!=(const TZWEntryCtrl& unitSrc) const
{
    return !operator==(unitSrc);
}


// ---------------------------------------------------------------------------
//  TZWEntryCtrl: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  If we are secure, then the driver cannot poll us using the basic get
//  command, which it does in a more efficient multi-cast way. In that case
//  we have to be queried separately. All locks are going to provide async
//  notifications, but they'll still be polled periodically to insure that
//  we stay in sync.
//
//  If not secure, this won't get called.
//
tCIDLib::TBoolean
TZWEntryCtrl::bBuildGetMsg(TString& strToFill, const tCIDLib::TCard4 c4Index)
{
    // We reported one get message, so anything but zero is invalid
    if (c4Index != 0)
        return kCIDLib::False;

    strToFill = L"N";
    strToFill.AppendFormatted(c4Id());
    strToFill.Append(kCIDLib::chComma);
    strToFill.Append(L"SS,");
    strToFill.AppendFormatted(tCIDLib::TCard4(COMMAND_CLASS_DOOR_LOCK));
    strToFill.Append(L",2");

    return kCIDLib::True;
}

//
//  We have to override this since we have to do secure transmission for
//  this type. If not secure, we pass it along to our parent class, else we
//  handle it here.
//
tCIDLib::TBoolean
TZWEntryCtrl::bBuildSetMsg( const   tCIDLib::TCard4 c4FldId
                            , const TString&        strValue
                            ,       TString&        strToFill
                            ,       TCQCServerBase& sdrvLevi)
{
    // If it's for our state value
    if (c4FldId == m_c4FldId_State)
    {
        // We only respond to on/off messages, which are open and close
        if (strValue.bCompareI(kZWaveLeviSh::strGetMsg_Off)
        ||  strValue.bCompareI(kZWaveLeviSh::strGetMsg_On))
        {
            const tCIDLib::TBoolean bOn = strValue.bCompareI(kZWaveLeviSh::strGetMsg_On);

            if (bHasCap(tZWaveLeviSh::EUnitCap_Secure))
            {
                // Gotta build a secure message
                strToFill = L"N";
                strToFill.AppendFormatted(c4Id());
                strToFill.Append(kCIDLib::chComma);
                strToFill.Append(L"SS,");
                strToFill.AppendFormatted(tCIDLib::TCard4(COMMAND_CLASS_DOOR_LOCK));
                strToFill.Append(kCIDLib::chComma);
                strToFill.AppendFormatted(tCIDLib::TCard4(DOOR_LOCK_OPERATION_SET));
                if (bOn)
                    strToFill.Append(L",255");
                else
                    strToFill.Append(L",0");
            }
             else
            {
                // We can just do a standard set
                strToFill = L"N";
                strToFill.AppendFormatted(c4Id());
                if (bOn)
                    strToFill.Append(L"ON");
                else
                    strToFill.Append(L"OF");
            }
        }
    }
     else
    {
        return kCIDLib::False;
    }

    return kCIDLib::True;
}


tCIDLib::TBoolean TZWEntryCtrl::bOwnsFld(const tCIDLib::TCard4 c4FldId) const
{
    return (m_c4FldId_State == c4FldId);
}


//
//  If not a secure lock, then we can get basic reports. Otherwise we'll
//  get general reports. We just update our boolean state field.
//
tCIDLib::TBoolean
TZWEntryCtrl::bStoreBasicReport(const   tCIDLib::TCard4 c4Value
                                ,       TCQCServerBase& sdrvLevi)
{
    //
    //  Update our last poll time to now since we got a new value. Tell it
    //  to update the last value stamp as well.
    //
    ResetPollTime(kCIDLib::True);

    // Just store the boolean state
    if (bCheckFldId(m_c4FldId_State, L"state"))
        sdrvLevi.bStoreBoolFld(m_c4FldId_State, c4Value != 0, kCIDLib::True);

    return kCIDLib::True;
}


tCIDLib::TBoolean
TZWEntryCtrl::bStoreFldIds(  const  TVector<TCQCFldDef>&    colFlds
                            , const tZWaveLeviSh::TValList& fcolIds)
{
    // Find our field and store the id
    const TString& strOurFld = strName();
    const tCIDLib::TCard4 c4Count = colFlds.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TCQCFldDef& flddCur = colFlds[c4Index];
        if (flddCur.strName() == strName())
        {
            m_c4FldId_State = fcolIds[c4Index];
            return kCIDLib::True;
        }
    }
    return kCIDLib::False;
}


//
//  Handles non-basic type reports, which we'll get even for the basic
//  state if a secure lock.
//
tCIDLib::TBoolean
TZWEntryCtrl::bStoreGenReport(  const   tCIDLib::TCard4 ac4Values[]
                                , const tCIDLib::TCard4 c4ValCnt
                                ,       TCQCServerBase& sdrvLevi)
{
    // Shouldn't get anything less than 2, but don't tempt fate
    if (c4ValCnt < 2)
        return kCIDLib::False;

    tCIDLib::TBoolean bRes = kCIDLib::True;
    switch (ac4Values[0])
    {
        case COMMAND_CLASS_DOOR_LOCK :
            //
            //  If we have enough data, and it's a lock report, process it.
            //  Note that we don't send any code info here, and we set the
            //  lock type to 'other' so that the handlers if the trigger
            //  don't try to process the code.
            //
            if ((c4ValCnt >= 3)
            &&  ((ac4Values[1] == DOOR_LOCK_OPERATION_REPORT)
            ||   (ac4Values[1] == DOOR_LOCK_OPERATION_SET)))
            {
                StoreLockState
                (
                    sdrvLevi
                    , ac4Values[2] != 0
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
            if ((ac4Values[1] == ALARM_GET) || (ac4Values[1] == ALARM_REPORT))
            {
                tCIDLib::TCard4 c4Type = 0;
                tCIDLib::TCard4 c4Level = 0;
                if (c4ValCnt >= 3)
                    c4Type = ac4Values[2];

                if (c4ValCnt >= 4)
                    c4Level = ac4Values[3];

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
                        StoreLockState(sdrvLevi, bLock, c4Level, strType(c4Type));
                        break;

                    default :
                        break;
                };
            }
            break;


        default :
            // Nothing we specifically handle, so pass to the parent
            bRes = TParent::bStoreGenReport(ac4Values, c4ValCnt, sdrvLevi);
            break;
    };
    return bRes;
}



tCIDLib::TCard4
TZWEntryCtrl::c4QueryFldDefs(TVector<TCQCFldDef>&   colAddTo
                            , TCQCFldDef&           flddTmp) const
{
    // We just add our one boolean field
    flddTmp.Set
    (
        strName(), tCQCKit::EFldTypes::Boolean, eDefFldAccess()
    );
    colAddTo.objAdd(flddTmp);

    return 1;
}


// Call our parent and zero out our field id
tCIDLib::TVoid TZWEntryCtrl::PrepForUse()
{
    TParent::PrepForUse();
    m_c4FldId_State = kCIDLib::c4MaxCard;
}


// Set all of our unit status oriented fields to error state
tCIDLib::TVoid TZWEntryCtrl::SetFieldErrs(TCQCServerBase& sdrvLevi)
{
    if (m_c4FldId_State != kCIDLib::c4MaxCard)
    {
        // If it's readable, mark it
        if (sdrvLevi.flddFromId(m_c4FldId_State).bIsReadable())
            sdrvLevi.SetFieldErr(m_c4FldId_State, kCIDLib::True);
    }
}




// ---------------------------------------------------------------------------
//  TZWEntryCtrl: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Return a standard lock status trigger type string, based on the alarm we
//  got.
//
const TString& TZWEntryCtrl::strType(const tCIDLib::TCard4 c4AlarmType) const
{
    if ((c4AlarmType == 0x16) || (c4AlarmType == 0x15))
        return TCQCEvent::strVal_LockType_Manual;
    else if ((c4AlarmType == 0x19) || (c4AlarmType == 0x18))
        return TCQCEvent::strVal_LockType_Remote;
    else if ((c4AlarmType == 0x12) || (c4AlarmType == 0x13))
        return TCQCEvent::strVal_LockType_Pad;

    return TCQCEvent::strVal_LockType_Other;
};


// Called to do the work to store a new lock state
tCIDLib::TVoid
TZWEntryCtrl::StoreLockState(       TCQCServerBase&     sdrvLevi
                            , const tCIDLib::TBoolean   bState
                            , const tCIDLib::TCard4     c4Level
                            , const TString&            strType)
{
    //
    //  Update our last poll time to now since we got a new
    //  value. Tell it to update the last value stamp as well.
    //
    ResetPollTime(kCIDLib::True);

    if (!bCheckFldId(m_c4FldId_State, L"state"))
        return;

    // Store the new lock state value. Remember if it's a new state
    tCIDLib::TBoolean bChanged = sdrvLevi.bStoreBoolFld
    (
        m_c4FldId_State, bState, kCIDLib::True
    );

    // Queue a lock status event if the state changes
    if (bChanged)
    {
        TString strCode;
        strCode.AppendFormatted(c4Level);
        sdrvLevi.QueueEventTrig
        (
            tCQCKit::EStdDrvEvs::LockStatus
            , TString::strEmpty()
            , bState ? TCQCEvent::strVal_LockState_Locked
                     : TCQCEvent::strVal_LockState_Unlocked
            , strName()
            , strCode
            , strType
        );
    }
}

