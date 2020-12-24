//
// FILE NAME: ZWaveUSB3Sh_UnitInfo.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/24/2017
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
//  We need to track information about each known unit. This class provides that
//  functionality, along with some other info. Mostly this class deals with info that
//  is figured out ourself (via dialog with the unit), some is told to us by the user.
//  And some of it is just for runtime purposes. It might be reasonable to move some of
//  that to the unit base class instead, I'm not sure. At least the unit object pointer
//  itself is still runtime stuff and either stays here or we keep a separate list of
//  them.
//
//  We keep track of the unit name and its id at the most fundamental level. Even if
//  we know nothing else in terms of configuration we need to know those things. We
//  also track a state that tells us how far along the process of getting what we
//  need this unit is.
//
//  Much of what we do here is to try to move the unit along to the Ready state where
//  it is set up and ready to use.
//
//  Unit States
//
//  These are documented in in the IDL file, so I won't repeat that here. Basically we
//  start off getting basic info from our local Z-Stick. If that indicates it's something
//  we want to talk to, then we start trying to move up through the states, gathering
//  more info as we go.
//
//  Unlike some Z-Wave libraries we don't try to get all of the info from the units. That
//  is a long process and puts a big load on the Z-Wave network. Instead we get the bare
//  minimum required to try to see if we have built in support for the unit type. If
//  so we can load the device info file, which has all of the detailed information. Or,
//  the user can manually tell us which device info file is appropriate.
//
//  The only bit of non-essential info we really get is, if the unit supports naming, we
//  see if it has been given a name. If so, we grab that as the default initial name.
//
//  Even if the unit is a non-listener, we'll attempt to talk to it at least once, on
//  the chance it's awake. If not, that will fail and we will move to WaitWakeup state.
//  At some point we hope to see a node info frame or wakeup that will let us move forward
//  and start talking to the unit and get the info we need.
//
//  If we get the info (manufacturer/product ids) and we don't have support, then we move
//  to NoAutoMatch and the user has to provide that info. If the unit doesn't support
//  man ids, then we go to WaitDevInfo for the same reason.
//
//  If we get a match, then we can store the device info on this unit info object and we
//  can create the actual unit object that handles talking to the unit. The device info
//  file indicates what type to create. This is done even on the client side because of
//  user settable options, which is discussed below, but the unit object is not otherwise
//  used by the client.
//
//  Ongoing Processing
//
//  The driver will periodically call our Process() method. We look at the current state
//  and do a query that would get us to the next one, then set a time stamp by which time
//  we hope to have gotten that reply. If not, we try again and set a new time stamp,
//  though longer this time. We keep trying and increasing the time out period until we
//  get to a max, then we just stay there. We use a timeout counter to count consequtive
//  timeouts, and that is used to keep increasing the timeout period. Once we successfully
//  take a step, it gets zeroed again.
//
//  Once we get to ready state, then we start passing on the process call to the unit
//  handler object (which is created once we get get the device info.) It can then do
//  any ongoing processing it needs to do.
//
//  Wakeup Queue
//
//  We maintain a wakeup queue. Any messages sent by the unit or its CC impl objects
//  will be added to this queue if the unit is a non-listener, and is not currently known
//  to be awake. These messages will be sent when we get a wakeup or node info frame.
//
//  To avoid the necessity of multiple wakes ups, if we manage to get the man ids and
//  auto-id the unit, we will go ahead and queue up auto-config messages immediately so
//  that they will get sent. Otherwise, if we waited until the user approved the newly
//  id'd unit, they would just get queued up and have to wait for another wakeup cycle.
//
//  There is a catch-22 in that we may not see wakeups if the unit doesn't broadcast them,
//  since otherwise it will only send to associated units and we won't get associated
//  until we can send the auto-config msgs. That's why we treat node info frames like
//  wakeups. They are broadcast typically and usually also imply that the unit is going
//  to be awake for a little bit (usually sent when the batteries are taken out and put
//  back in.)
//
//  Device Info
//
//  We cache the device info set on us and that is persisted. This way we don't have
//  to get go through this process but once to get the unit identified. After that,
//  we assume nothing has changed (until proven otherwise by the user) and just reload
//  that information. That way we can get started immediately for these already id'd
//  units.
//
//  User Settable Options
//
//  The unit handler objects are the ones that define the available user settable options.
//  That is something specific to the unit type. However, we provide the persisted config
//  info. So, he provides the definitions of the options and we store the actual selected
//  values. They are just key/value pairs. So we store the key and the value. That lets us
//  correlate our stored values with the reported options from the unit object.
//
//  When the unit object is created we call a method that will get the options and will
//  add default values to our list for any that we don't current have, and remove any
//  values that are no longer reported by the unit object.
//
//  The unit object will ask us for these values whenever he needs to know what his
//  option values are.
//
//  Re-Auto-Config
//
//  We send auto-config msgs (as defined in the device info file) once the unit is either
//  automatically or manually id'd. However, there's always the chance that that fails
//  or something else interferes with the configuration. So we randomly schedule a time
//  once a day to send out auto-config msgs, and re-send on a daily basis. This just
//  helps insure that damage is repaired.
//
// CAVEATS/GOTCHAS:
//
//
// LOG:
//
#pragma once

#pragma CIDLIB_PACK(CIDLIBPACK)

class TZWaveUnit;

// ---------------------------------------------------------------------------
//   CLASS: TZWUnitInfo
//  PREFIX: uniti
// ---------------------------------------------------------------------------
class ZWUSB3SHEXPORT TZWUnitInfo :  public TObject
                                    , public MStreamable
{
    public :
        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static tCIDLib::TCard1 c1Key
        (
            const   TZWUnitInfo&            zwndiSrc
        );

        static tCIDLib::ESortComps eCompById
        (
            const   TZWUnitInfo&            uniti1
            , const TZWUnitInfo&            uniti2
        );

        static tCIDLib::ESortComps eCompByManIds
        (
            const   TZWUnitInfo&            uniti1
            , const TZWUnitInfo&            uniti2
        );

        static tCIDLib::ESortComps eCompByName
        (
            const   TZWUnitInfo&            uniti1
            , const TZWUnitInfo&            uniti2
        );

        static const TString& strUIKey
        (
            const   TZWUnitInfo&            unitiSrc
        );


        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TZWUnitInfo();

        TZWUnitInfo
        (
            const   tCIDLib::TCard1         c1UnitiD
        );

        TZWUnitInfo
        (
            const   TZWUnitInfo&            unitiSrc
        );

        ~TZWUnitInfo();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TZWUnitInfo& operator=
        (
            const   TZWUnitInfo&            unitiSrc
        );

        tCIDLib::TBoolean operator==
        (
            const   TZWUnitInfo&            unitiSrc
        )   const;

        tCIDLib::TBoolean operator!=
        (
            const   TZWUnitInfo&            unitiSrc
        )   const;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid ApproveForUse();

        tCIDLib::TBoolean bAlwaysOn() const
        {
            return m_bListener || m_bFreqListener;
        }

        tCIDLib::TBoolean bDoAutoCfg
        (
            const   tCIDLib::TBoolean       bIsAwake
            ,       TString&                strStatus
        );

        tCIDLib::TBoolean bFindOptVal
        (
            const   TString&                strKey
            ,       TString&                strToFill
        )   const;

        tCIDLib::TBoolean bForceManIds
        (
            const   tCIDLib::TCard8         c8ToSet
            ,       TString&                strErr
        );

        tCIDLib::TBoolean bForceRescan
        (
            const   tCIDLib::TBoolean       bKnownAwake
        );

        tCIDLib::TBoolean bFreqListener() const
        {
            return m_bFreqListener;
        }

        tCIDLib::TBoolean bHasDevInfo() const
        {
            return m_deviUnit.bIsValid();
        }

        tCIDLib::TBoolean bHasUnitObject() const
        {
            return (m_punitThis != nullptr);
        }

        tCIDLib::TBoolean bIsCtrl() const
        {
            return m_bIsCtrl;
        }

        tCIDLib::TBoolean bIsViable() const
        {
            return (m_eState >= tZWaveUSB3Sh::EUnitStates::FirstViableState);
        }

        tCIDLib::TBoolean bListener() const
        {
            return m_bListener;
        }

        tCIDLib::TBoolean bQueryOptVal
        (
            const   TString&                strKey
            , const tCIDLib::TBoolean       bDefault
        )   const;

        tCIDLib::TBoolean bOptExists
        (
            const   TString&                strKey
        )   const;

        tCIDLib::TBoolean bSupportsClass(const tZWaveUSB3Sh::ECClasses eClass) const
        {
            return m_deviUnit.bSupportsClass(eClass);
        }

        tCIDLib::TCard1 c1Id() const
        {
            return m_c1Id;
        }

        tCIDLib::TCard1 c1Id
        (
            const   tCIDLib::TCard1         c1ToSet
        );

        tCIDLib::TCard4 c4QueryOptVal
        (
            const   TString&                strKey
            , const tCIDLib::TCard4         c4Default
        )   const;

        tCIDLib::TCard4 c4SerialNum() const
        {
            return m_c4SerialNum;
        }

        tCIDLib::TCard8 c8ManIds() const
        {
            return m_deviUnit.c8ManIds();
        }

        const TZWDevInfo& deviUnit() const
        {
            return m_deviUnit;
        }

        tCIDLib::EFindRes2 eCheckCCAccess
        (
            const   tZWaveUSB3Sh::ECClasses eClass
            , const tZWaveUSB3Sh::EUnitAcc  eCanHave
            , const tZWaveUSB3Sh::EUnitAcc  eMustHave
            , const tCIDLib::TCard1         c1EndPoint = 0xFF
        )   const;

        tZWaveUSB3Sh::EUnitNameSrcs eNameSrc() const
        {
            return m_eNameSrc;
        }

        tCIDLib::EFindRes2 eProbeCCAccess
        (
            const   tZWaveUSB3Sh::ECClasses eClass
            ,       tZWaveUSB3Sh::EUnitAcc& eToSet
            , const tCIDLib::TCard1         c1EndPoint = 0xFF
        )   const;

        tZWaveUSB3Sh::EUnitStates eState() const;

        tZWaveUSB3Sh::EUnitStates eState
        (
            const   tZWaveUSB3Sh::EUnitStates eToSet
        );

        tCIDLib::TVoid FormatAutoCfg
        (
                    TTextOutStream&         strmTar
        )   const;

        tCIDLib::TVoid FormatReport
        (
                    TTextOutStream&         strmTar
        )   const;

        tCIDLib::TVoid HandleAppUpdate
        (
            const   TZWInMsg&               zwimUpdate
        );

        tCIDLib::TVoid HandleCCMsg
        (
            const   tCIDLib::TCard1         c1Class
            , const tCIDLib::TCard1         c1Cmd
            , const TZWInMsg&               zwimCC
        );

        tCIDLib::TInt4 i4QueryOptVal
        (
            const   TString&                strKey
            , const tCIDLib::TInt4          i4Default
        )   const;

        tCIDLib::TVoid MakeUnitObject
        (
            const   tCIDLib::TBoolean       bClientSide
        );

        const TZWaveUnit* punitThis() const
        {
            return m_punitThis;
        }

        TZWaveUnit* punitThis()
        {
            return m_punitThis;
        }

        tCIDLib::TVoid ParseDevInfo
        (
            const TXMLTreeElement&          xtnodeRoot
        );

        tCIDLib::TVoid PrepForUse
        (
            const   tCIDLib::TEncodedTime   enctNow
            ,       MZWaveCtrlIntf* const   pmzwciOwner
        );

        tCIDLib::TVoid Process();

        tCIDLib::TVoid QueueEventTrig
        (
            const   tCQCKit::EStdDrvEvs     eEvent
            , const TString&                strFld
            , const TString&                strVal1
            , const TString&                strVal2
            , const TString&                strVal3
            , const TString&                strVal4
        );

        tCIDLib::TVoid QueryUnitAttrs
        (
                    tCIDMData::TAttrList&   colToFill
        );

        tCIDLib::TVoid ResetDevInfo
        (
            const   tCIDLib::TBoolean       bRescan
        );

        const TString& strMake() const
        {
            return m_deviUnit.strMake();
        }

        const TString& strModel() const
        {
            return m_deviUnit.strModel();
        }

        const TString& strName() const
        {
            return m_strName;
        }

        const TString& strName
        (
            const   TString&                strToSet
        );

        const TString& strQueryOptVal
        (
            const   TString&                strKey
            , const TString&                strDefault
        )   const;

        tCIDLib::TVoid SendMsgAsync
        (
            const   TZWOutMsg&              zwomToSend
            , const tCIDLib::TBoolean       bKnownAwake = kCIDLib::False
        );

        tCIDLib::TVoid SendWaitAck
        (
            const   TZWOutMsg&              zwomToSend
            , const tCIDLib::TBoolean       bKnownAwake = kCIDLib::False
        );

        tCIDLib::TVoid SendWaitReply
        (
            const   TZWOutMsg&              zwomToSend
            ,       TZWInMsg&               zwimReply
            , const tCIDLib::TBoolean       bKnownAwake = kCIDLib::False
        );

        tCIDLib::TVoid SetOption
        (
            const   TString&                strKey
            , const TString&                strValue
        );

        tCIDLib::TVoid StoreUnitAttr
        (
            const   TAttrData&              adatToStore
        );

        const TZWaveUnit& unitThis() const;

        TZWaveUnit& unitThis();


    protected :
        // -------------------------------------------------------------------
        //  The base CC impl and unit classes friends so they can access some methods
        //  below that we make available for them (so that we can limit access to the
        //  driver and require it go through us.) The configuration class is also so
        //  that he can play a couple tricks.
        // -------------------------------------------------------------------
        friend class TZWCCImpl;
        friend class TZWaveUnit;
        friend class TZWaveDrvCfg;


        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        )   override;

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const   override;


        // -------------------------------------------------------------------
        //  Protected, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bQueryFldName
        (
            const   tCIDLib::TCard4         c4FldId
            ,       TString&                strToFill
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

        tCIDLib::TCard4 c4FindFldId
        (
            const   TString&                strFldName
        )   const;

        const TString& strDrvMoniker() const;

        tCIDLib::TVoid SetFldError
        (
            const   tCIDLib::TCard4         c4FldId
        );


    private :

        // A queue for msgs we can't send until we get a wakeup
        typedef TVector<TZWOutMsg>          TWakeupQ;

        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bProbeUnit
        (
                    MZWaveCtrlIntf* const   pmzwciOwner
        );

        tCIDLib::TEncodedTime enctCalcNextProc
        (
            const   tCIDLib::TCard4         c4BaseSeconds
        );

        tCIDLib::TVoid IncSerialNum();

        tCIDLib::TVoid Reset
        (
            const   tCIDLib::TCard1         c1UnitId
        );

        tCIDLib::TVoid ResetRuntime();

        tCIDLib::TVoid SendWakeupQ();

        tCIDLib::TVoid StoreDevInfo
        (
            const   TZWDevInfo&             deviOther
            , const tCIDLib::TBoolean       bKnownAwake
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bIsCtrl
        //  m_bListener
        //  m_bFreqListener
        //  m_bHasManSpec
        //  m_bIsSecure
        //  m_bManSpecSecure
        //      We get some basic info on new units during the InitUnit and GetUnitInfo
        //      states. Some of this is just used to know if we can get man ids, and
        //      will be ignored later once we get device info set. But the listener
        //      stuff is used in a lot of places. If it's a controller we mark it as
        //      disabled so it doesn't get used.
        //
        //  m_c1Id
        //      The network id for this module. This is persisted since we want to
        //      compare it upon subsequent network scan so we can compare it to what
        //      is at that id now.
        //
        //  m_c4SerialNum
        //      Any time we are modified we bump this. This lets the clients know if
        //      any unit has changed. Note this is about *configuration, not any current
        //      field values and such.
        //
        //  m_c4TryCount
        //      When we are moving through the unit states, trying to get to Ready state,
        //      this is bumped each time we timeout waiting for a response for the
        //      current state.
        //
        //      THIS IS ONLY bumped by enctCalcNextProc(). The other code zeros it after
        //      a successful move to a new state, or successful Process pass().
        //
        //  m_colOptVals
        //      See the class comments above. A list of key=value pairs that are used to
        //      hold the values of unit options. Only the units (and their CC impls) know
        //      the options, and we just store the values. The attribute paths are used
        //      to connect the two.
        //
        //  m_colWakeupQ
        //      Our unit handlers send msgs by calling us. If our unit is not a live
        //      listener, we just queue it up and send it later when we get a wakeup.
        //
        //  m_deviUnit
        //      The device info we load once we have either auto-identified the unit
        //      or the user tells us what it is.
        //
        //  m_enctNextAutoCfg
        //      Just to help insure things stay healthy, we will once every day send
        //      out auto-config msgs to the unit. They may get messed with by some
        //      other program, or whatever. This just insures that they get back happy
        //      again. This is runtime only and is initialized when the unit is about
        //      to start being used (i.e. it becomes part of the active driver config.)
        //
        //  m_enctNextProc
        //      This is used to throttle our activity in Process(). If we aren't making
        //      forward state progress, we don't want to continually bang away, or we
        //      need to know when to time out on any async operations.
        //
        //  m_eState
        //      The current overall state of this unit. This is purely a runtime thing.
        //      It has to be here because we don't even create the actual unit object
        //      until we get to a state where it's possible.
        //
        //  m_eNameSrc
        //      Tells us how we got our current unit name, so that we can only override
        //      with defaults or unit provided names if appropriate.
        //
        //  m_punitThis
        //      The actual unit object for this unit. This is not created until we have
        //      the appropriate make/model/C++ class info.
        //
        //      THIS IS NOT copied or assigned. It is assumed that these will only exist
        //      it the working config data of the driver. After any assignment of new
        //      data new objects may need to be created.
        //
        //  m_pmzwciOwner
        //      A pointer to our owning driver, via our mixin interface that he
        //      implements. This lets us provide helpers that we, our unit object and
        //      his CC impl objects can use to access driver functionality, and so
        //      that we can do magic stuff on their behalf.
        //
        //  m_strName
        //      The name of the unit, which is defaulted by us and then configured by
        //      the user. It must be unique in the list of units, and is used to create
        //      field names and in logged messages and generic commands that target
        //      modules by name. We will get any Z-Wave level assigned name as the
        //      initial default, if the unit supports naming.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean           m_bIsCtrl;
        tCIDLib::TBoolean           m_bListener;
        tCIDLib::TBoolean           m_bFreqListener;
        tCIDLib::TBoolean           m_bHasManSpec;
        tCIDLib::TBoolean           m_bIsSecure;
        tCIDLib::TBoolean           m_bManSpecSecure;
        tCIDLib::TCard1             m_c1Id;
        tCIDLib::TCard4             m_c4SerialNum;
        tCIDLib::TCard4             m_c4TryCount;
        tCIDLib::TKVHashSet         m_colOptVals;
        TWakeupQ                    m_colWakeupQ;
        TZWDevInfo                  m_deviUnit;
        tCIDLib::TEncodedTime       m_enctNextAutoCfg;
        tCIDLib::TEncodedTime       m_enctNextProc;
        tZWaveUSB3Sh::EUnitStates   m_eState;
        tZWaveUSB3Sh::EUnitNameSrcs  m_eNameSrc;
        TZWaveUnit*                 m_punitThis;
        MZWaveCtrlIntf*             m_pmzwciOwner;
        TString                     m_strName;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWUnitInfo, TObject)
};

#pragma CIDLIB_POPPACK
