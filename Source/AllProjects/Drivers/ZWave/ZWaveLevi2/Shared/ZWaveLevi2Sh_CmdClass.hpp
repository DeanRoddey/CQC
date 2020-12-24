//
// FILE NAME: ZWaveLevi2Sh_CmdClass.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/28/2014
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
//  This is the header file for the ZWaveLevi2Sh_CmdClass.cpp file, which implements
//  the TZWCmdClass class. This is the abstract base class from which all of the
//  various command classes derived from.
//
//  We also define some of the simplest ones here.
//
//  NOTE that Basic is not implemented as a cmd class. The basic reports are always
//  mapped to the command class indicated in the device info as the base class.
//
// CAVEATS/GOTCHAS:
//
//  1)  NO PERSISTENT data is stored in these guys. All such data is stored at the
//      unit level. So these guys don't implement any streaming. All the info
//      required to re-generate these guys is the command class list in the device
//      info. Generally it would be either stuff just always stored by the unit, or
//      in the options values.
//
//  2)  We can access any basic driver functionality here via the base driver class.
//      But, we can't see our actual server side driver class, because it exists
//      above us. So, in order to access driver specific functionality, we have to
//      define a simple mixin class here that the server side driver can implement.
//      He can then pass himself to use via that. In order to be consistent, and also
//      to have strict control over what driver functionality is accessed, we make
//      all driver access so through it.
//
//      This also simplfies things on the client side, since it brings in this facility
//      as well. We can just create a dummy implementation that does nothing, whereas
//      the actual base driver class would attempt to do things if called and cause
//      problems since it wouldn't be really in a state to react.
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)

class TZWaveUnit;
class TZWCmdClass;


// ---------------------------------------------------------------------------
//   CLASS: MZWLeviSrvFuncs
//  PREFIX: mzwsf
// ---------------------------------------------------------------------------
class ZWLEVI2SHEXPORT MZWLeviSrvFuncs
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        MZWLeviSrvFuncs();

        MZWLeviSrvFuncs(const MZWLeviSrvFuncs&) = delete;

        ~MZWLeviSrvFuncs();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        MZWLeviSrvFuncs& operator=(const MZWLeviSrvFuncs&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        virtual tCIDLib::TBoolean bLeviFldExists
        (
            const   TString&                strName
        )   const = 0;

        virtual tCIDLib::TBoolean bLeviQueryFldName
        (
            const   tCIDLib::TCard4         c4FldId
            ,       TString&                strToFill
        )   const = 0;

        virtual tCIDLib::TBoolean bLeviStoreBoolFld
        (
            const   tCIDLib::TCard4         c4FldId
            , const tCIDLib::TBoolean       bToStore
        ) = 0;

        virtual tCIDLib::TBoolean bLeviStoreCardFld
        (
            const   tCIDLib::TCard4         c4FldId
            , const tCIDLib::TCard4         c4ToStore
        ) = 0;

        virtual tCIDLib::TBoolean bLeviStoreFloatFld
        (
            const   tCIDLib::TCard4         c4FldId
            , const tCIDLib::TFloat8        f8ToStore
        ) = 0;

        virtual tCIDLib::TBoolean bLeviStoreIntFld
        (
            const   tCIDLib::TCard4         c4FldId
            , const tCIDLib::TInt4          i4ToStore
        ) = 0;

        virtual tCIDLib::TBoolean bLeviStoreStrFld
        (
            const   tCIDLib::TCard4         c4FldId
            , const TString&                strToStore
        ) = 0;

        virtual tCIDLib::TBoolean bLeviTraceEnabled() const = 0;

        virtual tCIDLib::TBoolean bLeviWaitUnitMsg
        (
                    TZWaveUnit&             unitSrc
            , const TZWCmdClass&            zwccSrc
            , const tCIDLib::TCard4         c4CmdClass
            , const tCIDLib::TCard4         c4CmdId
            ,       tCIDLib::TCardList&     fcolVals
        ) = 0;

        virtual tCIDLib::TCard4 c4LeviFldIdFromName
        (
            const   TString&                strName
        )   const = 0;

        virtual tCIDLib::TCard4 c4LeviVRCOPId() const = 0;

        virtual tCQCKit::EVerboseLvls eLeviVerboseLevel() const = 0;

        virtual tCIDLib::TVoid LeviQEventTrig
        (
            const   tCQCKit::EStdDrvEvs     eEvent
            , const TString&                strFld
            , const TString&                strVal1
            , const TString&                strVal2
            , const TString&                strVal3
            , const TString&                strVal4
        ) = 0;

        virtual tCIDLib::TVoid LeviSendMsg
        (
            const   TZWaveUnit&             unitTar
            , const tCIDLib::TCardList&     fcolVals
        ) = 0;

        virtual tCIDLib::TVoid LeviSendUnitMsg
        (
            const   TZWaveUnit&             unitTar
            , const TZWCmdClass&            zwccSrc
            , const tCIDLib::TCard4         c4CmdClass
            , const tCIDLib::TCard4         c4Cmd
        ) = 0;

        virtual tCIDLib::TVoid LeviSendUnitMsg
        (
            const   TZWaveUnit&             unitTar
            , const TZWCmdClass&            zwccSrc
            , const tCIDLib::TCardList&     fcolVals
        ) = 0;

        virtual tCIDLib::TVoid LeviSetFldErr
        (
            const   tCIDLib::TCard4         c4FldId
        ) = 0;

        virtual const TCQCFldLimit* pfldlLeviLimsFor
        (
            const   tCIDLib::TCard4         c4FldId
            , const TClass&                 clsType
        )   const = 0;

        virtual const TString& strLeviMoniker() const = 0;

        virtual TTextOutStream& strmLeviTrace() = 0;
};



// ---------------------------------------------------------------------------
//   CLASS: TZWCmdClass
//  PREFIX: zwcc
// ---------------------------------------------------------------------------
class ZWLEVI2SHEXPORT TZWCmdClass : public TObject
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TZWCmdClass(const TZWCmdClass&) = delete;

        ~TZWCmdClass();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TZWCmdClass& operator=(const TZWCmdClass&) = delete;


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        virtual tCIDLib::TBoolean bHandleGenReport
        (
            const   tCIDLib::TCardList&     fcolVals
            , const TZWaveUnit&             unitMe
            ,       MZWLeviSrvFuncs&        mzwsfLevi
            , const tCIDLib::TCard4         c4InstId
        );

        virtual tCIDLib::TBoolean bOwnsFld
        (
            const   tCIDLib::TCard4         c4FldId
        )   const;

        virtual tCIDLib::TBoolean bQueryFldVals
        (
                    TZWaveUnit&             unitMe
            ,       MZWLeviSrvFuncs&        mzwsfLevi
        );

        virtual tCQCKit::ECommResults eWriteBoolFld
        (
            const   tCIDLib::TCard4         c4FldId
            , const tCIDLib::TBoolean       bValue
            ,       TZWaveUnit&             unitMe
            ,       MZWLeviSrvFuncs&        mzwsfLevi
        );

        virtual tCQCKit::ECommResults eWriteCardFld
        (
            const   tCIDLib::TCard4         c4FldId
            , const tCIDLib::TCard4         c4Value
            ,       TZWaveUnit&             unitMe
            ,       MZWLeviSrvFuncs&        mzwsfLevi
        );

        virtual tCQCKit::ECommResults eWriteIntFld
        (
            const   tCIDLib::TCard4         c4FldId
            , const tCIDLib::TInt4          i4Value
            ,       TZWaveUnit&             unitMe
            ,       MZWLeviSrvFuncs&        mzwsfLevi
        );

        virtual tCQCKit::ECommResults eWriteFloatFld
        (
            const   tCIDLib::TCard4         c4FldId
            , const tCIDLib::TFloat8        f8Value
            ,       TZWaveUnit&             unitMe
            ,       MZWLeviSrvFuncs&        mzwsfLevi
        );

        virtual tCQCKit::ECommResults eWriteStringFld
        (
            const   tCIDLib::TCard4         c4FldId
            , const TString&                strValue
            ,       TZWaveUnit&             unitMe
            ,       MZWLeviSrvFuncs&        mzwsfLevi
        );

        virtual tCIDLib::TVoid InitVals
        (
            const   TZWaveUnit&             unitMe
            ,       MZWLeviSrvFuncs&        mzwsfLevi
        ) = 0;

        virtual tCIDLib::TVoid LoadOptVals
        (
            const   TZWDevClass&            zwdcInst
            ,       tZWaveLevi2Sh::TOptValList& colToFill
            ,       TZWOptionVal&           zwoptvTmp
        )   const;

        virtual tCIDLib::TVoid LoadOptList
        (
                    tZWaveLevi2Sh::TOptList& colToFill
        )   const;

        virtual tCIDLib::TVoid QueryFldDefs
        (
                    tCQCKit::TFldDefList&   colAddTo
            ,       TCQCFldDef&             flddTmp
            , const TZWaveUnit&             unitMe
            ,       TString&                strTmp1
            ,       TString&                strTmp2
        )   const;

        virtual tCIDLib::TVoid StoreFldIds
        (
            const   TZWaveUnit&             unitMe
            , const MZWLeviSrvFuncs&        mzwsfLevi
            ,       TString&                strTmp1
            ,       TString&                strTmp2
        );


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bAllZeroInst() const;

        tCIDLib::TBoolean bCanRead() const;

        tCIDLib::TBoolean bCanWrite() const;

        tCIDLib::TBoolean bCheckPollReady();

        tCIDLib::TBoolean bIsThisObject
        (
            const   tZWaveLevi2Sh::ECClasses eClass
            , const tCIDLib::TCard4         c4InstId
        )   const;

        tCIDLib::TBoolean bSecure() const;

        tCIDLib::TBoolean bWaitInitVals() const;

        tCIDLib::TBoolean bWaitInitVals
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TCard4 c4Flags() const;

        tCIDLib::TCard4 c4InstId() const;

        tCIDLib::TCard4 c4ZWClassId() const;

        tCIDLib::TEncodedTime enctLastMsg() const;

        tZWaveLevi2Sh::ECClasses eCmdClass() const;

        tCQCKit::EFldAccess eFldAccess() const;

        tZWaveLevi2Sh::EMultiTypes eMultiType() const;

        tCIDLib::TVoid ResetTimeStamp();

        const TString& strInstDescr() const;

        const TString& strInstName() const;


    protected :
        // -------------------------------------------------------------------
        //  Hidden constructors and operators
        // -------------------------------------------------------------------
        TZWCmdClass
        (
            const   tZWaveLevi2Sh::ECClasses eCCmdClass
            , const TZWDevClass&            zwdcInst
            , const tCIDLib::TCard4         c4ZWClassId
        );


        // -------------------------------------------------------------------
        //  Protected, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid AppendInstName
        (
            const   TZWaveUnit&             unitMe
            , const tCIDLib::TCh* const     pszDefInst
            ,       TString&                strTar
        )   const;

        tCIDLib::TBoolean bCheckFldId
        (
            const   tCIDLib::TCard4         c4ToCheck
            , const tCIDLib::TCh* const     pszFldName
            , const TZWaveUnit&             unitMe
        )   const;

        tCIDLib::TBoolean bDecodeValue
        (
            const   tCIDLib::TCard4         ac4Vals[]
            , const tCIDLib::TCard4         c4ValCnt
            , const tCIDLib::TBoolean       bExpectInt
            ,       tCIDLib::TInt4&         i4Result
            ,       tCIDLib::TFloat8&       f8Result
            , const tCIDLib::TCh* const     pszFld
            , const MZWLeviSrvFuncs&        mzwsfLevi
        )   const;

        tCIDLib::TVoid BuildInstName
        (
            const   TZWaveUnit&             unitMe
            , const tCIDLib::TCh* const     pszDefInst
            ,       TString&                strToFill
        )   const;

        tCIDLib::TVoid BuildOptKey
        (
            const   tCIDLib::TCh* const     pszKeyBase
            ,       TString&                strKey
        )   const;

        tCIDLib::TCard4 c4PollMins() const;

        tCIDLib::TCard4 c4PollMins
        (
            const   tCIDLib::TCard4         c4ToSet
        );

        tCIDLib::TCard4 c4SetRespCls() const;

        tCIDLib::TCard4 c4SetRespCmd() const;

        tCIDLib::TVoid SetAccess
        (
            const   tCIDLib::TBoolean       bCanRead
            , const tCIDLib::TBoolean       bCanWrite
        );


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bAllZeroInst
        //      In some cases, devices have multiple instances of a class, but don't
        //      report them via multi inst/channel. So this is set to force the driver
        //      to pass all zero instance general reports to all instances. They can
        //      then use some other means to decide whether to store the data.
        //
        //  m_bCanRead
        //  m_bCanWrite
        //      These are only paid attention to when the command class is normally
        //      readable or writeable, so tha they can be used to override that default
        //      assumption. Some devices may not support reading, so we depend on async
        //      reports. And some might not allow writing, for security/safety reasons.
        //      For a class that is already not readable and/or writable, it will ignore
        //      these.
        //
        //  m_bSecure
        //      Messages this class sends must be secure. This is driven by the device
        //      info file.
        //
        //  m_bWaitInitVals
        //      For units that implement Wakeup, we assume they cannot be initially
        //      queried, so we have to wait until we hear something from them before
        //      we can do anything. This flag is initailized true and can be used by
        //      derived classes to remember if they have made an attempt yet to get
        //      initial field data.
        //
        //  m_c4Flags
        //      The general flags (32 bit) value from the device info file, which
        //      allows for ad hoc flags for each command class.
        //
        //  m_c4InstId
        //      The instance id (and the class) represent the unique key for a given
        //      command class object. Some devices support more than one instance of
        //      a given command class, though generally there is only one.
        //
        //  m_c4PollMins
        //      The polling interval for this class, if it is needed. It can be set
        //      in the device info file. If not it will be zero. The derived class
        //      can override if it wants. If not, it stays zero and we know that we
        //      should use a reasonable (slow) default. If non-zero, it will be forced
        //      to be a minimum of 5 minutes, since that's as fast as we will allow
        //      active polling to be done.
        //
        //  m_c4SetRespCls
        //  m_c4SetRespCmd
        //      If non-zero, this is the msg to expect to be generated when you send
        //      a set type command. This let's the class use the generated async
        //      response as a confirmation response.
        //
        //  m_c4ZWClassId
        //      The derived class tells us the Z-Wave level command class id for it's
        //      class. We expose this since the driver sometimes needs to find command
        //      class objects this way.
        //
        //  m_enctNextPoll
        //  m_enctNextPing
        //      When ResetTimeStamp() is called by derived classes, which they do when
        //      they store new data, we update these to the next time that this guy
        //      could be polled or pinged. They may not get used (if m_bPoll is not
        //      set) but we keep them up to date.
        //
        //  m_eClass
        //      The command class id (and instance id) represent the unique key for a
        //      given command class object. Some devices support more than one instance
        //      of a given command class, though generally there is only one.
        //
        //  m_eMultiType
        //      If the unit has more than one instance of a given class, they must be
        //      marked as to how to interact with them, since there are two different
        //      ways. If only one instance, then this shouls be set to Single.
        //
        //  m_strInstDescr
        //      From the configured device class info that was used to create us.
        //
        //  m_strInstName
        //      The instance name, which will be combined with the overall unit name
        //      to create a unit-per-instance name (and hence field names.)
        // -------------------------------------------------------------------
        tCIDLib::TBoolean           m_bAllZeroInst;
        tCIDLib::TBoolean           m_bCanRead;
        tCIDLib::TBoolean           m_bCanWrite;
        tCIDLib::TBoolean           m_bPoll;
        tCIDLib::TBoolean           m_bSecure;
        tCIDLib::TBoolean           m_bWaitInitVals;
        tCIDLib::TCard4             m_c4Flags;
        tCIDLib::TCard4             m_c4InstId;
        tCIDLib::TCard4             m_c4PollMins;
        tCIDLib::TCard4             m_c4SetRespCls;
        tCIDLib::TCard4             m_c4SetRespCmd;
        tCIDLib::TCard4             m_c4ZWClassId;
        tZWaveLevi2Sh::ECClasses    m_eClass;
        tZWaveLevi2Sh::EMultiTypes  m_eMultiType;
        tCIDLib::TEncodedTime       m_enctNextPing;
        tCIDLib::TEncodedTime       m_enctNextPoll;
        TString                     m_strInstDescr;
        TString                     m_strInstName;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWCmdClass,TObject)
};

#pragma CIDLIB_POPPACK


