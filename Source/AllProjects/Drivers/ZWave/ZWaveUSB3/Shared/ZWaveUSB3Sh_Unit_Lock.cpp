//
// FILE NAME: ZWaveUSB3Sh_Lock.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/01/2017
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
//  This is the implementation for a generic lock.
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
RTTIDecls(TLockUnit, TZWaveUnit)
RTTIDecls(TLockNotUnit, TZWaveUnit)



// ---------------------------------------------------------------------------
//   CLASS: TLockUnit
//  PREFIX: unit
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TLockUnit: Constructors and Destructor
// ---------------------------------------------------------------------------
TLockUnit::TLockUnit(TZWUnitInfo* const punitiOwner) :

    TZWaveUnit(punitiOwner)
    , m_pzwcciLock(nullptr)
{
}

TLockUnit::~TLockUnit()
{
}


// ---------------------------------------------------------------------------
//  TLockUnit: Public, inherited methods
// ---------------------------------------------------------------------------

// We create and store our CC impl objects
tCIDLib::TBoolean TLockUnit::bPrepare()
{
    if (!TParent::bPrepare())
        return kCIDLib::False;

    // We just need to gen up a lock CC impl object
    m_pzwcciLock = new TZWCCImplDLockStateF(&unitiOwner(), TString::strEmpty());
    tCIDLib::TCard4 c4ImplId;
    return bAddImplObject(m_pzwcciLock, c4ImplId);
}


//
//  We need to watch for value changes and send out an event trigger.
//
tCIDLib::TVoid
TLockUnit::ImplValueChanged(const   tCIDLib::TCard4         c4ImplId
                            , const tZWaveUSB3Sh::EValSrcs  eSource
                            , const tCIDLib::TBoolean       bWasInErr
                            , const tCIDLib::TInt4          )
{
    // If not programmatic change and is from our lock CC
    if ((eSource != tZWaveUSB3Sh::EValSrcs::Program)
    &&  (m_pzwcciLock->c4ImplId() == c4ImplId))
    {
        //
        //  We don't send a user code in this case, since the door lock class
        //  doesn't provide one.
        //
        unitiOwner().QueueEventTrig
        (
            tCQCKit::EStdDrvEvs::LockStatus
            , m_pzwcciLock->strFldName()
            , m_pzwcciLock->bState() ? TCQCEvent::strVal_LockState_Locked
                                     : TCQCEvent::strVal_LockState_Unlocked
            , strName()
            , TString::strEmpty()
            , TString::strEmpty()
        );
    }
}






// ---------------------------------------------------------------------------
//   CLASS: TLockNotUnit
//  PREFIX: unit
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TLockNotUnit: Constructors and Destructor
// ---------------------------------------------------------------------------
TLockNotUnit::TLockNotUnit(TZWUnitInfo* const punitiOwner) :

    TZWaveUnit(punitiOwner)
    , m_pzwcciLock(nullptr)
    , m_pzwcciNot(nullptr)
{
}

TLockNotUnit::~TLockNotUnit()
{
}


// ---------------------------------------------------------------------------
//  TLockNotUnit: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  We create our CC impl objects. In our case we don't store them on the base class
//  since we need more control.
//
//  So we just handle it ourself.
//
tCIDLib::TBoolean TLockNotUnit::bPrepare()
{
    if (!TParent::bPrepare())
        return kCIDLib::False;

    // Create our notification handler and add it
    tCIDLib::TCard4 c4ImplId;
    m_pzwcciNot = new TZWCCImplNotifyState(&unitiOwner());
    if (!bAddImplObject(m_pzwcciNot, c4ImplId))
        return kCIDLib::False;

    // Add a lock field handler for our main lock functionality
    m_pzwcciLock = new TZWCCImplDLockStateF(&unitiOwner(), TString::strEmpty());
    if (!bAddImplObject(m_pzwcciLock, c4ImplId))
        return kCIDLib::False;

    return kCIDLib::True;
}


//
//  We need to watch for value changes. Note that our calls to sync up one impl with
//  another will cause us to be called back into here recursively. But we pass program
//  as the source, and we ignore programmatic changes, so that won't cause any sort
//  of freak out.
//
tCIDLib::TVoid
TLockNotUnit::ImplValueChanged( const   tCIDLib::TCard4         c4ImplId
                                , const tZWaveUSB3Sh::EValSrcs  eSource
                                , const tCIDLib::TBoolean       bWasInErr
                                , const tCIDLib::TInt4          )
{
    // If not a programmatic change and for our CC impl id
    if ((eSource != tZWaveUSB3Sh::EValSrcs::Program)
    &&  ((m_pzwcciLock->c4ImplId() == c4ImplId) || (m_pzwcciNot->c4ImplId() == c4ImplId)))
    {
        //
        //  If the notification guy's value changes, we need to pass it to the lock
        //  impl. If we get the change via notification, we have a user code.
        //
        tCIDLib::TBoolean bChange = kCIDLib::False;
        TString strCode;
        if (c4ImplId == m_pzwcciNot->c4ImplId())
        {
            //
            //  The notification guy's state changed, so set that on the lock and see
            //  if that's a change for him.
            //
            bChange = m_pzwcciLock->bSetBinState
            (
                m_pzwcciNot->bState(), tZWaveUSB3Sh::EValSrcs::Program
            );

            //
            //  If a change, format out the last notification event parameter we got
            //  That should be the code of it's a lock.
            //
            if (bChange)
                strCode.SetFormatted(m_pzwcciNot->c4LastEventParam());
        }
         else if (c4ImplId == m_pzwcciLock->c4ImplId())
        {
            //
            //  The lock impl itself changed, so we consider that a change. We push
            //  it into the notification guy so that they are always in sync.
            //
            bChange = kCIDLib::True;
            m_pzwcciNot->bSetBinState
            (
                m_pzwcciLock->bState(), tZWaveUSB3Sh::EValSrcs::Program
            );
        }

        // Now let's send a notification if we got a useful change
        if (bChange)
        {
            unitiOwner().QueueEventTrig
            (
                tCQCKit::EStdDrvEvs::LockStatus
                , m_pzwcciLock->strFldName()
                , m_pzwcciLock->bState() ? TCQCEvent::strVal_LockState_Locked
                                         : TCQCEvent::strVal_LockState_Unlocked
                , strName()
                , strCode
                , TString::strEmpty()
            );
        }
    }
}


