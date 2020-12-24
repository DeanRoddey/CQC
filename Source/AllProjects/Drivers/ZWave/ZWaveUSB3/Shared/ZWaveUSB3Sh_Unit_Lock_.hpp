//
// FILE NAME: ZWaveUSB3Sh_Unit_Lock_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/1/2017
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
//  There are a lot of locks that we can deal with generically. Our basic handler,
//  TLockUnit just works in terms of the DOOR_LOCK command class, which typically
//  would handle the needs of a modern lock.
//
//  We also have one that deals with less consistent implementations where the door
//  lock class is used for get and set, but notifications/alarms are used for
//  async notifications. One advantage of that is we get the user code used to lock
//  or unlock via the pad.
//
//  Lock event triggers are always sent, so we don't have any configurable option
//  for whether to send them or not.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once

#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TLockUnit
//  PREFIX: unit
// ---------------------------------------------------------------------------
class TLockUnit : public TZWaveUnit
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TLockUnit() = delete;

        TLockUnit
        (
                TZWUnitInfo* const          punitiOwner
        );

        TLockUnit(const TLockUnit&) = delete;

        ~TLockUnit();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TLockUnit& operator=(const TLockUnit&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bPrepare() override;

        tCIDLib::TVoid ImplValueChanged
        (
            const   tCIDLib::TCard4         c4ImplId
            , const tZWaveUSB3Sh::EValSrcs  eSource
            , const tCIDLib::TBoolean       bWasInErr
            , const tCIDLib::TInt4          i4ExtraInfo
        )   override;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_pzwcciLock
        //      We keep a typed pointer to our lock CC impl for convenience, though
        //      the base class owns it.
        // -------------------------------------------------------------------
        TZWCCImplDLockStateF*   m_pzwcciLock;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TLockUnit, TZWaveUnit)
};


// ---------------------------------------------------------------------------
//   CLASS: TLockNotUnit
//  PREFIX: unit
// ---------------------------------------------------------------------------
class ZWUSB3SHEXPORT TLockNotUnit : public TZWaveUnit
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TLockNotUnit() = delete;

        TLockNotUnit
        (
                TZWUnitInfo* const      punitiOwner
        );

        TLockNotUnit(const TLockNotUnit&) = delete;

        ~TLockNotUnit();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TLockNotUnit& operator=(const TLockNotUnit&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bPrepare() override;

        tCIDLib::TVoid ImplValueChanged
        (
            const   tCIDLib::TCard4         c4ImplId
            , const tZWaveUSB3Sh::EValSrcs  eSource
            , const tCIDLib::TBoolean       bWasInErr
            , const tCIDLib::TInt4          i4ExtraInfo
        )   override;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_pzwcciLock
        //  m_pzwcciNot
        //      We keep typed pointers to these to make things easier, but they are
        //      set on the parent class who manages in them in the usual way.
        //      One handles the door lock class stuff, and one handles incoming
        //      notification msgs.
        //
        //      We override ImplValueChanged() to watch for changes in the state of
        //      either, so that we can arrange to send out the appropriate lock
        //      event triggers. When we get a change in the notification one, we push
        //      that change to the lock one.
        // -------------------------------------------------------------------
        TZWCCImplDLockStateF*   m_pzwcciLock;
        TZWCCImplNotifyState*   m_pzwcciNot;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TLockNotUnit, TZWaveUnit)
};

#pragma CIDLIB_POPPACK
