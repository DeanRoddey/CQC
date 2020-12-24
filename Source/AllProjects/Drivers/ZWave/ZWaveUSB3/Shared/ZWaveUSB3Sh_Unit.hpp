//
// FILE NAME: ZWaveUSB3Sh_Unit.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/31/2017
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
//  This is the header for the base unit class. All of the various specific unit classes
//  derive from this. It defines the core virtual interface that all units must
//  implement. Each unit is given the unit info object that drove its creation, which it
//  can use to set itself up as required. So unit info is config and this unit class is
//  functionality.
//
//  Basically we need a derivative for each class of unit, and sometimes for particularly
//  ill designed ones that require specific knowledge. Mostly though we can have a unit
//  type handler for all reasonably well formed locks, dimmers, sensors, etc... Extra
//  info the device info file can parameterize them in various ways for more flexibility.
//  We want to avoid very specific unit handlers as much as possible, since they could
//  grow out of hand.
//
//  Responsibilities
//
//  The primary things any unit has to do is to process incoming messages addressed
//  to it, to provide info on the fields it created, and handle writes to its
//  fields (sending out appropriate messages). And it needs to tell the driver what
//  fields it needs. So we provide virtual interfaces for that stuff.
//
//  Mostly, a lot of this is just distributing these calls to any CC impl objects it
//  creates. A lot of them will be little more than reading config from the device info
//  file, creating CC impls based on that, and setting them on us. We then distribute
//  calls to them as appropriate.
//
//  So we keep a by ref vector of CC impl classes and the derived class can give
//  us instances to manage for them. Most of the derivates really just create CC impls
//  and give them to us, and mostly stay out of the way.
//
//  But, they can do a lot more if they need to. They can manage the CC impls themselves
//  if required, or manage some of them while giving us others to manage. This just means
//  that they have to replicate the work we do of distributing calls to the ones that
//  they manage. We try to avoid this as much as possible, but ultimately that gives us
//  the flexibility to deal with stupider unit types.
//
//  Init Order
//
//  The unit handler is created when sufficient info is available. After it is created
//  the following calls are made:
//
//      1. bParseExtraInfo()
//      2. QueryUnitAttrs()
//      3. bPrepare()
//
//  These give the unit a chance to bootstrap itself up appropriately. Options sometimes
//  depend on extra info. And preparation (creating CC impls and such) will often depend
//  on both extra info and query options.
//
//  Generlly bParseExtraInfo() should parse and store away in members the unit level
//  extra data it reads from the device info file. Subsequent steps can then make use
//  of it. That data cannot change for any given field registration cycle, so it's fine
//  to store it away in a more convenient form.
//
//  But, if it only needs the info in bPrepare() it can wait until then parse it and
//  not handle bParseExtraInfo().
//
//  QueryUnitAttrs() is called on the server side so that any options will get initial
//  default values faulted in and any that are not valid anymore can be removed.
//  Otherwise QueryUnitAttrs() is used by the client side to get the editable options
//  to present ot the user.
//
//  User Configurable Options
//
//  The client side interface uses the standard attribute editor to let the user edit
//  options. The client will call QueryUnitAttrs() to get the list of values to load into
//  a standard attribute editor window. The unit handlers and any CC impls it creates are
//  given a chance to report any editable values as TAttrData objects. At this level they
//  all MUST be driven by unit options and the attribute id must be the option key. The
//  same for the CC Impls if they add any.
//
//  When the value is stored the unit info object will update the actual option value,
//  which he keeps up with. These changes are not reacted to until the user saves and
//  the unit resets, since that could change the fields that should be created or the
//  specifics of the field. So there is no need for the unit info object to inform
//  us of these changes, he just stores them.
//
//  Ongoing Work
//
//  Once this above is done and the CC impls are created, then the driver will asks us
//  for our fields. He'll register everyone's fields, then let us look up our fields.
//  Both of these things are primarily handled by the CC impls to which these calls are
//  passed on.
//
//  Then it's a matter of handling incoming CC msgs, and sending them out in response to
//  field writes.
//
//  We also get called regularly on our Process() method. This is just passed on to any
//  CC impls to let them do ongoing processing as required.
//
//  The unit is informed any time any of its CC impls enter/exit error state, or when
//  any of their fields change value, so that they can do anything necessary in response.
//  Mostly they don't care, but if they are managing their own CC impls or managing
//  their own fields, they may need to do things in response to this.
//
// CAVEATS/GOTCHAS:
//
//  1)  If a CC Impl object has an end point id, we will only dispatch incoming msgs
//      to it if the msg is for the same id, though they can mark themselves as
//      'see all EPs' in which case we'll dispatch all end point msgs for his owning
//      unit to him.
//
//  2)  Any derived classes that DON'T add their CC impls to our list MUST override
//      methods and pass them on to those CC impls since we don't know about them. As
//      much as possible units should just set his CC impls on us so everything gets
//      passed on automagically. But sometimes units have to manage CC impls itself.
//      This of course adds complexity since any changes to these methods now must
//      be dealt with in more places. But most of the unit derivatives just set their
//      CCs on us.
//
// LOG:
//
#pragma once

#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TZWaveUnit
//  PREFIX: unit
// ---------------------------------------------------------------------------
class ZWUSB3SHEXPORT TZWaveUnit : public TObject
{
    public :
        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static tCIDLib::ESortComps eCompById
        (
            const   TZWaveUnit&             unit1
            , const TZWaveUnit&             unit2
            , const tCIDLib::TCard4         c4UserData
        );

        static tCIDLib::ESortComps eCompByNextTime
        (
            const   TZWaveUnit&             unit1
            , const TZWaveUnit&             unit2
            , const tCIDLib::TCard4         c4UserData
        );


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TZWaveUnit(const TZWaveUnit&) = delete;

        ~TZWaveUnit();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TZWaveUnit& operator=(const TZWaveUnit&) = delete;


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        virtual tCIDLib::TBoolean bBoolFldChanged
        (
            const   TString&                strFldName
            , const tCIDLib::TCard4         c4FldId
            , const tCIDLib::TBoolean       bValue
            ,       tCQCKit::ECommResults&  eRes
        );

        virtual tCIDLib::TBoolean bCardFldChanged
        (
            const   TString&                strFldName
            , const tCIDLib::TCard4         c4FldId
            , const tCIDLib::TCard4         c4Value
            ,       tCQCKit::ECommResults&  eRes
        );

        virtual tCIDLib::TBoolean bFloatFldChanged
        (
            const   TString&                strFldName
            , const tCIDLib::TCard4         c4FldId
            , const tCIDLib::TFloat8        f8Value
            ,       tCQCKit::ECommResults&  eRes
        );

        virtual tCIDLib::TBoolean bIntFldChanged
        (
            const   TString&                strFldName
            , const tCIDLib::TCard4         c4FldId
            , const tCIDLib::TInt4          i4Value
            ,       tCQCKit::ECommResults&  eRes
        );

        virtual tCIDLib::TBoolean bOwnsFld
        (
            const   tCIDLib::TCard4         c4FldId
        )   const;

        virtual tCIDLib::TBoolean bParseExtraInfo();

        virtual tCIDLib::TBoolean bPrepare();

        virtual tCIDLib::TBoolean bStringFldChanged
        (
            const   TString&                strFldName
            , const tCIDLib::TCard4         c4FldId
            , const TString&                strValue
            ,       tCQCKit::ECommResults&  eRes
        );

        virtual tCIDLib::TVoid GetInitVals();

        virtual tCIDLib::TVoid ImplInErrState
        (
            const   tCIDLib::TCard4         c4ImplId
        );

        virtual tCIDLib::TVoid ImplValueChanged
        (
            const   tCIDLib::TCard4         c4ImplId
            , const tZWaveUSB3Sh::EValSrcs  eSource
            , const tCIDLib::TBoolean       bWasInErr
            , const tCIDLib::TInt4          i4ExtraInfo
        );

        virtual tCIDLib::TVoid HandleCCMsg
        (
            const   tCIDLib::TCard1         c1Class
            , const tCIDLib::TCard1         c1Cmd
            , const TZWInMsg&               zwimToHandle
        );

        virtual tCIDLib::TVoid Process();

        virtual tCIDLib::TVoid QueryFldDefs
        (
                    tCQCKit::TFldDefList&   colAddTo
            ,       TCQCFldDef&             flddTmp
        )   const;

        virtual tCIDLib::TVoid QueryUnitAttrs
        (
                    tCIDMData::TAttrList&   colToFill
            ,       TAttrData&              adatTmp
        )   const;

        virtual tCIDLib::TVoid StoreFldIds();

        virtual tCIDLib::TVoid UnitIsOnline();


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bAlwaysOn() const
        {
            return m_punitiOwner->bAlwaysOn();
        }

        tCIDLib::TBoolean bFreqListener() const
        {
            return m_punitiOwner->bFreqListener();
        }

        tCIDLib::TBoolean bSupportsClass
        (
            const   tZWaveUSB3Sh::ECClasses eClass
        )   const;

        tCIDLib::TCard1 c1Id() const
        {
            return m_punitiOwner->c1Id();
        }

        tZWaveUSB3Sh::EUnitStates eState()
        {
            return m_punitiOwner->eState();
        }

        tCIDLib::TVoid FormatReport
        (
                    TTextOutStream&         strmTar
        )   const;

        const TString& strName() const
        {
            return m_punitiOwner->strName();
        }

        const TZWUnitInfo& unitiOwner() const
        {
                return *m_punitiOwner;
        }

        TZWUnitInfo& unitiOwner()
        {
                return *m_punitiOwner;
        }


    protected :
        // -------------------------------------------------------------------
        //  Our unit info class is our friend
        // -------------------------------------------------------------------
        friend class TZWUnitInfo;


        // -------------------------------------------------------------------
        //  Hidden constructors and destructor
        // -------------------------------------------------------------------
        TZWaveUnit
        (
                    TZWUnitInfo* const      punitiOwner
        );


        // -------------------------------------------------------------------
        //  Protected, virtual methods
        // -------------------------------------------------------------------
        virtual tCIDLib::TVoid FormatCCsInfo
        (
                    TTextOutStream&         strmTar
        )   const;

        virtual tCIDLib::TVoid FormatUnitInfo
        (
                    TTextOutStream&         strmTar
        )   const;


        // -------------------------------------------------------------------
        //  Protected, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bAddImplObject
        (
                    TZWCCImpl* const        pzwcciToAdopt
            ,       tCIDLib::TCard4&        c4ImplId
        );

        tCIDLib::TBoolean bFindExtraVal
        (
            const   TString&                strKey
            ,       TString&                strToFill
        )   const;

        tCIDLib::TBoolean bFindOptVal
        (
            const   TString&                strKey
            ,       TString&                strToFill
        )   const;

        tCIDLib::TBoolean bInitCCImpl
        (
                    TZWCCImpl&              zwcciTar
        );

        tCIDLib::TBoolean bQueryFldName
        (
            const   tCIDLib::TCard4         c4FldId
            ,       TString&                strToFill
        )   const;

        tCIDLib::TBoolean bQueryOptVal
        (
            const   TString&                strKey
            , const tCIDLib::TBoolean       bDefault
        )   const;

        tCIDLib::TBoolean bWriteFld
        (
            const   tCIDLib::TCard4         c4FldId
            , const tCIDLib::TBoolean       bToStore
        );

        tCIDLib::TBoolean bWriteFld
        (
            const   tCIDLib::TCard4         c4FldId
            , const tCIDLib::TCard4         c4ToStore
        );

        tCIDLib::TBoolean bWriteFld
        (
            const   tCIDLib::TCard4         c4FldId
            , const tCIDLib::TFloat8        f8ToStore
        );

        tCIDLib::TBoolean bWriteFld
        (
            const   tCIDLib::TCard4         c4FldId
            , const tCIDLib::TInt4          i4ToStore
        );

        tCIDLib::TBoolean bWriteFld
        (
            const   tCIDLib::TCard4         c4FldId
            , const TString&                strToStore
        );

        tCIDLib::TCard4 c4LookupFldId
        (
            const   TString&                strFldName
        )   const;

        const TString& strDrvMoniker() const;

        tCIDLib::TVoid SetFldError
        (
            const   tCIDLib::TCard4         c4FldId
        );


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_colImpls
        //      See the class comments above. This may or may not be used by any
        //      particular derivative, but most probably will. It lets us handle a
        //      lot of grunt work on their behalf.
        //
        //      The index into this list is an id we give back to the derived class
        //      when he adds an impl object, so that he can get back to a specific
        //      one as needed. We own and clean up these objects.
        //
        //  m_punitiOwner
        //      A pointer to the unit info object that owns us. This lets us have quick
        //      access to our own config. There's no sync issues. We and it run in the
        //      same thread always. So we can update it as appropriate, mostly just our
        //      current state. We don't own it, we just reference it.
        // -------------------------------------------------------------------
        tZWaveUSB3Sh::TCCImplList   m_colImpls;
        TZWUnitInfo*                m_punitiOwner;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWaveUnit,TObject)
};

#pragma CIDLIB_POPPACK
