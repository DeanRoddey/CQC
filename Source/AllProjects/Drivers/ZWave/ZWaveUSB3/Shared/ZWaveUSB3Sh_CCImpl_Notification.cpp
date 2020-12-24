//
// FILE NAME: ZWaveUSB3Sh_CCImpl_Notification.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/13/2018
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
//  This is the implementation classes for the Z-Wave notification command class.
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
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TZWCCImplNotifyState, TZWCCImplBin)
RTTIDecls(TZWCCImplNotifyStateF, TZWCCImplNotifyState)




// ---------------------------------------------------------------------------
//   CLASS: TZWCCImplNotifyState
//  PREFIX: zwcci
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWCCImplNotifyState: Constructors and Destructor
// ---------------------------------------------------------------------------
TZWCCImplNotifyState::
TZWCCImplNotifyState(       TZWUnitInfo* const  punitiThis
                    , const tCIDLib::TCard1     c1EPId) :

    TZWCCImplBin(punitiThis, tZWaveUSB3Sh::ECClasses::Notification, c1EPId)
    , m_bForceIds(kCIDLib::False)
    , m_bForceTypes(kCIDLib::False)
    , m_c2NotId(0)
    , m_c4LastEventParam(0)
    , m_c4Version(1)
    , m_eEvType(tZWaveUSB3Sh::ENotEvTypes::NotEvent)
    , m_eIdType(tZWaveUSB3Sh::ENotIdTypes::NotType)
{
    //
    //  For convenience, store the version of our command class. Tell it not to
    //  throw if not found, just return 1. That would be awfully weird since we
    //  would be getting created otherwise. But we don't want to fall over. It will
    //  just return 1.
    //
    m_c4Version = punitiThis->deviUnit().c4ClassVersion
    (
        tZWaveUSB3Sh::ECClasses::Notification, kCIDLib::False, strUnitName()
    );

    //
    //  Make sure our access is always none, overriding the default set by the base,
    //  which would have gotten set to readable if our unit is always on style.
    //
    SetAccess(tZWaveUSB3Sh::EUnitAcc::None, tZWaveUSB3Sh::EUnitAcc::AllBits);
}

TZWCCImplNotifyState::~TZWCCImplNotifyState()
{
}


// ---------------------------------------------------------------------------
//  TZWCCImplNotifyState: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TZWCCImplNotifyState::bPrepare()
{
    if (!TParent::bPrepare())
        return kCIDLib::False;

    const TZWUnitInfo& unitiUs = unitiOwner();

    //
    //  If we have ids forced on us in the ctor or via SetNotIds, don't try to load
    //  them.
    //
    TString strVal;
    if (!m_bForceIds)
    {
        //
        //  This could be zero if we are going to get V1 (ALARM) type msgs. But
        //  log a trace error if our CC version is beyond 2.
        //
        if (!bFindExtraVal(L"NotId", strVal)
        ||  !strVal.bToCard2(m_c2NotId, tCIDLib::ERadices::Auto))
        {
            return kCIDLib::False;
        }

        if (!m_c2NotId && (m_c4Version > 2))
        {
            facZWaveUSB3Sh().LogTraceErr
            (
                L"Unit %(1), got 0 notification id but CC version is %(2)"
                , strUnitName()
                , TCardinal(m_c4Version)
            );
        }

        // We have to break out the off/on events since there can be more than one
        tCIDLib::TCard1 c1Tmp;
        TString strEv;
        {
            if (!bFindExtraVal(L"OffEvs", strVal))
            {
                facZWaveUSB3Sh().LogTraceErr
                (
                    L"Unit %(1), no notification Off events found in extra info"
                    , strUnitName()
                );
                return kCIDLib::False;
            }

            TStringTokenizer stokOffEvs(&strVal, L" ");
            m_fcolOffEvs.RemoveAll();
            while (stokOffEvs.bGetNextToken(strEv))
            {
                if (!strEv.bToCard1(c1Tmp, tCIDLib::ERadices::Auto))
                {
                    facZWaveUSB3Sh().LogTraceErr
                    (
                        L"Unit %(1), invalid notification Off event found in extra info (%(2))"
                        , strUnitName()
                        , strEv
                    );
                    return kCIDLib::False;
                }
                m_fcolOffEvs.c4AddElement(c1Tmp);
            }
        }

        {
            if (!bFindExtraVal(L"OnEvs", strVal))
            {
                facZWaveUSB3Sh().LogTraceErr
                (
                    L"Unit %(1), no notification On events found in extra info"
                    , strUnitName()
                );
                return kCIDLib::False;
            }

            TStringTokenizer stokOnEvs(&strVal, L" ");
            m_fcolOnEvs.RemoveAll();
            while (stokOnEvs.bGetNextToken(strEv))
            {
                if (!strEv.bToCard1(c1Tmp, tCIDLib::ERadices::Auto))
                {
                    facZWaveUSB3Sh().LogTraceErr
                    (
                        L"Unit %(1), invalid notification On event found in extra info (%(2))"
                        , strUnitName()
                        , strEv
                    );
                    return kCIDLib::False;
                }
                m_fcolOnEvs.c4AddElement(c1Tmp);
            }
        }
    }

    //
    //  If not forced, get the type info. We don't want the defaults to be taken
    //  either so we have to prevent all of this.
    //
    if (!m_bForceTypes)
    {
        // Get the notification id type
        if (bFindExtraVal(L"NIdType", strVal))
        {
            if (strVal.bCompareI(L"NotType"))
                m_eIdType = tZWaveUSB3Sh::ENotIdTypes::NotType;
            else if (strVal.bCompareI(L"AlarmType"))
                m_eIdType = tZWaveUSB3Sh::ENotIdTypes::AlarmType;
            else if (strVal.bCompareI(L"BothNew"))
                m_eIdType = tZWaveUSB3Sh::ENotIdTypes::BothNew;
            else
            {
                facZWaveUSB3Sh().LogTraceErr
                (
                    L"Unit %(1), invalid notification id type in extra info (%(2))"
                    , strUnitName()
                    , strVal
                );
                return kCIDLib::False;
            }
        }
         else
        {
            // Take the default
            m_eIdType = tZWaveUSB3Sh::ENotIdTypes::NotType;
        }

        // Get the event type if set
        if (bFindExtraVal(L"EvType", strVal))
        {
            if (strVal.bCompareI(L"NotEvent"))
                m_eEvType = tZWaveUSB3Sh::ENotEvTypes::NotEvent;
            else if (strVal.bCompareI(L"AlarmLevel"))
                m_eEvType = tZWaveUSB3Sh::ENotEvTypes::AlarmLevel;
            else
            {
                facZWaveUSB3Sh().LogTraceErr
                (
                    L"Unit %(1), invalid notification event type in extra info (%(2))"
                    , strUnitName()
                    , strVal
                );
                return kCIDLib::False;
            }
        }
         else
        {
            // Take the default
            m_eEvType = tZWaveUSB3Sh::ENotEvTypes::NotEvent;
        }
    }

    return kCIDLib::True;
}


// ---------------------------------------------------------------------------
//  TZWCCImplNotifyState: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Given one of our standard binary sensor types, set up our ids to the most
//  likely scenarios for that type, assuming beyond V1 ALARM is supported.
//
tCIDLib::TBoolean
TZWCCImplNotifyState::bSetNotIds(const tZWaveUSB3Sh::EBinSensorTypes eType)
{
    switch(eType)
    {
        case tZWaveUSB3Sh::EBinSensorTypes::CO :
            m_c2NotId = 0x2;
            m_fcolOffEvs.c4AddElement(0);
            m_fcolOnEvs.c4AddElement(1);
            m_fcolOnEvs.c4AddElement(2);
            break;

        case tZWaveUSB3Sh::EBinSensorTypes::CO2 :
            m_c2NotId = 0x3;
            m_fcolOffEvs.c4AddElement(0);
            m_fcolOnEvs.c4AddElement(1);
            m_fcolOnEvs.c4AddElement(2);
            break;

        case tZWaveUSB3Sh::EBinSensorTypes::Door :
        case tZWaveUSB3Sh::EBinSensorTypes::Tilt :
            m_c2NotId = 0x6;
            m_fcolOffEvs.c4AddElement(0);
            m_fcolOnEvs.c4AddElement(16);
            m_fcolOnEvs.c4AddElement(17);
            break;

        case tZWaveUSB3Sh::EBinSensorTypes::Glass :
            m_c2NotId = 0x7;
            m_fcolOffEvs.c4AddElement(0);
            m_fcolOnEvs.c4AddElement(5);
            m_fcolOnEvs.c4AddElement(6);
            break;

        case tZWaveUSB3Sh::EBinSensorTypes::Heat :
            m_c2NotId = 0x4;
            m_fcolOffEvs.c4AddElement(0);
            m_fcolOnEvs.c4AddElement(1);
            m_fcolOnEvs.c4AddElement(2);
            break;

        case tZWaveUSB3Sh::EBinSensorTypes::Intrusion :
            m_c2NotId = 0x7;
            m_fcolOffEvs.c4AddElement(0);
            m_fcolOnEvs.c4AddElement(1);
            m_fcolOnEvs.c4AddElement(2);
            break;

        case tZWaveUSB3Sh::EBinSensorTypes::Leak :
            m_c2NotId = 0x5;
            m_fcolOffEvs.c4AddElement(0);
            m_fcolOnEvs.c4AddElement(1);
            m_fcolOnEvs.c4AddElement(2);
            break;

        case tZWaveUSB3Sh::EBinSensorTypes::Motion :
            m_c2NotId = 0x7;
            m_fcolOffEvs.c4AddElement(0);
            m_fcolOnEvs.c4AddElement(7);
            m_fcolOnEvs.c4AddElement(8);
            break;

        case tZWaveUSB3Sh::EBinSensorTypes::Smoke :
            m_c2NotId = 0x1;
            m_fcolOffEvs.c4AddElement(0);
            m_fcolOnEvs.c4AddElement(1);
            m_fcolOnEvs.c4AddElement(2);
            break;

        default :
            facZWaveUSB3Sh().LogTraceErr
            (
                L"Unit %(1), unknown notification sensor type, can't set ids", strUnitName()
            );
            return kCIDLib::False;
    };

    // Insure we are looking for the right info to match what we just set
    m_eIdType = tZWaveUSB3Sh::ENotIdTypes::NotType;
    m_eEvType = tZWaveUSB3Sh::ENotEvTypes::NotEvent;

    // And set both force flags
    m_bForceIds = kCIDLib::True;
    m_bForceTypes = kCIDLib::True;

    return kCIDLib::True;
}


tCIDLib::TCard4 TZWCCImplNotifyState::c4LastEventParam() const
{
    return m_c4LastEventParam;
}


//
//  Let the unit handler set arbitrary notification ids on us. We trust here that
//  they have set the correct type values to match what they are setting here.
//
tCIDLib::TVoid
TZWCCImplNotifyState::SetNotIds(const   tCIDLib::TCard2     c2NotId
                                , const tCIDLib::TByteList  fcolOffEvs
                                , const tCIDLib::TByteList  fcolOnEvs)
{
    m_c2NotId = c2NotId;
    m_fcolOffEvs = fcolOffEvs;
    m_fcolOnEvs = fcolOnEvs;

    m_bForceIds = kCIDLib::True;
}


tCIDLib::TVoid
TZWCCImplNotifyState::SetNotIds(const   tZWaveUSB3Sh::ENotIdTypes   eIdType
                                , const tZWaveUSB3Sh::ENotEvTypes   eEvType
                                , const tCIDLib::TCard2             c2NotId
                                , const tCIDLib::TByteList          fcolOffEvs
                                , const tCIDLib::TByteList          fcolOnEvs)
{
    m_eIdType = eIdType;
    m_eEvType = eEvType;
    m_c2NotId = c2NotId;
    m_fcolOffEvs = fcolOffEvs;
    m_fcolOnEvs = fcolOnEvs;

    m_bForceIds = kCIDLib::True;
    m_bForceTypes = kCIDLib::True;
}


// ---------------------------------------------------------------------------
//  TZWCCImplNotifyState: Protected, inherited methods
// ---------------------------------------------------------------------------

tZWaveUSB3Sh::ECCMsgRes
TZWCCImplNotifyState::eHandleCCMsg( const   tCIDLib::TCard1 c1Class
                                    , const tCIDLib::TCard1 c1Cmd
                                    , const TZWInMsg&       zwimIn)
{
    //
    //  The V3 doesn't matter, it's the same value for alarm and notification CCs.
    //  Since we can't be sure we got correct CC version info, we'll go by the length
    //  of the CC bytes to decide what version of report it is.
    //
    if ((c1Class == COMMAND_CLASS_NOTIFICATION_V3)
    &&  (c1Cmd == NOTIFICATION_REPORT_V3))
    {
        const tCIDLib::TCard4 c4CCBytes = zwimIn.c4CCBytes();
        if (c4CCBytes == 4)
        {
            //
            //  We get just a type and a level. We store the level as the event
            //  parameter, which may or may not be useful. Only the unit handler
            //  knows.
            //
            const tCIDLib::TCard1 c1Ev = zwimIn.c1CCByteAt(2);

            // It's our guy so check if it's an event we understand
            tCIDLib::TBoolean bMatched = kCIDLib::False;
            tCIDLib::TBoolean bNewState;
            if (m_fcolOffEvs.bElementPresent(c1Ev))
            {
                bMatched = kCIDLib::True;
                bNewState = kCIDLib::False;
            }
             else if (m_fcolOnEvs.bElementPresent(c1Ev))
            {
                bMatched = kCIDLib::True;
                bNewState = kCIDLib::True;
            }

            // If we got a match, then let's process it
            if (bMatched)
            {
                //
                //  Store the last event param before we set the value, since that
                //  is going to cause the notifications to occur immediately. So we
                //  need this stored.
                //
                m_c4LastEventParam = zwimIn.c1CCByteAt(3);
                bSetBinState(bNewState, tZWaveUSB3Sh::EValSrcs::Unit);
            }
            return tZWaveUSB3Sh::ECCMsgRes_Value;
        }
         else if (c4CCBytes >= 9)
        {
            //
            //  Get out the notification id and the event id. These can be defined in
            //  various ways so we have to pull out the right info.
            //
            tCIDLib::TCard2 c2NotId;
            tCIDLib::TCard1 c1EventId;
            switch(m_eIdType)
            {
                case tZWaveUSB3Sh::ENotIdTypes::NotType :
                    // The new style notification type
                    c2NotId = zwimIn.c1CCByteAt(6);
                    break;

                case tZWaveUSB3Sh::ENotIdTypes::AlarmType :
                    // The old style alarm type
                    c2NotId = zwimIn.c1CCByteAt(2);
                    break;

                case tZWaveUSB3Sh::ENotIdTypes::BothNew :
                    // Both the notification type and event mixed
                    c2NotId = zwimIn.c1CCByteAt(6);
                    c2NotId <<= 8;
                    c2NotId |= zwimIn.c1CCByteAt(7);
                    break;

                default :
                    facZWaveUSB3Sh().LogTraceErr
                    (
                        L"%(1)/%(2) - Invalid notification id type %(3)"
                        , strUnitName()
                        , strDesc()
                        , tZWaveUSB3Sh::strXlatENotIdTypes(m_eIdType)
                    );
                    SetErrState();
                    return tZWaveUSB3Sh::ECCMsgRes_Handled;
            };

            switch(m_eEvType)
            {
                case tZWaveUSB3Sh::ENotEvTypes::NotEvent :
                    // The standard new style event
                    c1EventId = zwimIn.c1CCByteAt(7);
                    break;

                case tZWaveUSB3Sh::ENotEvTypes::AlarmLevel :
                    c1EventId = zwimIn.c1CCByteAt(3);
                    break;

                default :
                    facZWaveUSB3Sh().LogTraceErr
                    (
                        L"%(1)/%(2) - Invalid notification event type %(3)"
                        , strUnitName()
                        , strDesc()
                        , tZWaveUSB3Sh::strXlatENotEvTypes(m_eEvType)
                    );
                    SetErrState();
                    return tZWaveUSB3Sh::ECCMsgRes_Handled;
            };


            // If it's our notification,  then check the event
            if (c2NotId == m_c2NotId)
            {
                // It's our guy so check if it's an event we understand
                tCIDLib::TBoolean bMatched = kCIDLib::False;
                tCIDLib::TBoolean bNewState;
                if (m_fcolOffEvs.bElementPresent(c1EventId))
                {
                    bMatched = kCIDLib::True;
                    bNewState = kCIDLib::False;
                }
                 else if (m_fcolOnEvs.bElementPresent(c1EventId))
                {
                    bMatched = kCIDLib::True;
                    bNewState = kCIDLib::True;
                }

                if (bMatched)
                {
                    // Store the event parameter if appropriate
                    m_c4LastEventParam = 0;
                    const tCIDLib::TCard4 c4ParmCnt = zwimIn.c1CCByteAt(8) & 0x1F;
                    if (c4ParmCnt > 2)
                    {
                        if (c2NotId == 0x6)
                        {
                            // Store the user code for a pad lock/unlock
                            if ((c1EventId == 5) || (c1EventId == 6))
                                m_c4LastEventParam = zwimIn.c1CCByteAt(9 + 2);
                        }
                    }

                    bSetBinState(bNewState, tZWaveUSB3Sh::EValSrcs::Unit);
                }
            }
            return tZWaveUSB3Sh::ECCMsgRes_Value;
        }

    }
    return tZWaveUSB3Sh::ECCMsgRes_Unhandled;
}





// ---------------------------------------------------------------------------
//   CLASS: TZWCCImplNotifyStateF
//  PREFIX: zwcci
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWCCImplNotifyStateF: Constructors and Destructor
// ---------------------------------------------------------------------------
TZWCCImplNotifyStateF::
TZWCCImplNotifyStateF(          TZWUnitInfo* const  punitiThis
                        , const TString&            strFldName
                        , const tCQCKit::EFldSTypes eSemType
                        , const tCIDLib::TCard1     c1EPId) :

    TZWCCImplNotifyState(punitiThis, c1EPId)
    , m_c4FldId_State(kCIDLib::c4MaxCard)
    , m_eSemType(eSemType)
    , m_strFldName(strFldName)
{
}

TZWCCImplNotifyStateF::~TZWCCImplNotifyStateF()
{
}


// ---------------------------------------------------------------------------
//  TZWCCImplNotifyStateF: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TZWCCImplNotifyStateF::bOwnsFld(const tCIDLib::TCard4 c4FldId) const
{
    return (c4FldId == m_c4FldId_State);
}


tCIDLib::TVoid
TZWCCImplNotifyStateF::QueryFldDefs(tCQCKit::TFldDefList&   colAddTo
                                    , TCQCFldDef&           flddTmp) const
{
    //
    //  NOTE THAT our base class set the base impl class' access to none, because the
    //  CC itself is neither readable nor writeable. But the field itself of course is
    //  obviously readable, we just get the values unsolicted always.
    //
    flddTmp.Set
    (
        m_strFldName
        , tCQCKit::EFldTypes::Boolean
        , tCQCKit::EFldAccess::Read
        , m_eSemType
    );
    colAddTo.objAdd(flddTmp);
}


tCIDLib::TVoid TZWCCImplNotifyStateF::StoreFldIds()
{
    // Just look up our one field using the name we were given
    m_c4FldId_State = c4LookupFldId(m_strFldName);

    // Until we get a value, it should be in error state
    SetFieldErr(m_c4FldId_State);
}


// ---------------------------------------------------------------------------
//  TZWCCImplNotifyStateF: Protected, inherited methods
// ---------------------------------------------------------------------------

// On value or error state change, just call our helper to store the new state
tCIDLib::TVoid TZWCCImplNotifyStateF::EnteredErrState()
{
    SetFieldErr(m_c4FldId_State);
    TParent::EnteredErrState();
}


tCIDLib::TVoid
TZWCCImplNotifyStateF::ValueChanged(const   tZWaveUSB3Sh::EValSrcs  eSource
                                    , const tCIDLib::TBoolean       bWasInErr
                                    , const tCIDLib::TInt4          i4ExtraInfo)
{
    bWriteFld(m_c4FldId_State, bState());
    TParent::ValueChanged(eSource, bWasInErr, i4ExtraInfo);
}

