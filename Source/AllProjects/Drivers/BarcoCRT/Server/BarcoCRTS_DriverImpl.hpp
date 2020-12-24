//
// FILE NAME: BarcoCRTS_DriverImpl.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/13/2002
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
//  This is the header file for the actual driver class. We just inherit from
//  the IDL generated base server class, and provide implementations for the
//  virtual methods it created for us.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TBarcoCRTS
//  PREFIX: orbs
// ---------------------------------------------------------------------------
class TBarcoCRTSDriver : public TCQCServerBase
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TBarcoCRTSDriver
        (
            const   TCQCDriverObjCfg&       cqcdcToLoad
        );

        ~TBarcoCRTSDriver();


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bGetCommResource
        (
                    TThread&                thrThis
        );

        tCIDLib::TBoolean bWaitConfig
        (
                    TThread&                thrThis
        );

        tCQCKit::ECommResults eBoolFldValChanged
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4FldId
            , const tCIDLib::TBoolean       bNewValue
        );

        tCQCKit::ECommResults eCardFldValChanged
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4FldId
            , const tCIDLib::TCard4         c4NewValue
        );

        tCQCKit::ECommResults eConnectToDevice
        (
                    TThread&                thrThis
        );

        tCQCKit::EDrvInitRes eInitializeImpl();

        tCQCKit::ECommResults ePollDevice
        (
                    TThread&                thrThis
        );

        tCIDLib::TVoid ReleaseCommResource();

        tCIDLib::TVoid TerminateImpl();


    private :
        // -------------------------------------------------------------------
        //  Unimplemented constructors and operators
        // -------------------------------------------------------------------
        TBarcoCRTSDriver();
        TBarcoCRTSDriver(const TBarcoCRTSDriver&);
        tCIDLib::TVoid operator=(const TBarcoCRTSDriver&);


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tBarcoCRTS::EMsgResults eDoStatusPoll
        (
                    TThread&                thrThis
        );

        const tCIDLib::TCh* pszXlatSigType
        (
            const   tCIDLib::TCard1         c1RawValue
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c1ProjAddr
        //      The address of the projector we are communicating with. This
        //      is queried from the user on projector install. It goes into
        //      each message, to address one of a set of daisy chained units.
        //
        //  m_c4FldIdXXX
        //      We look up the ids of our fields after registering them.
        //
        //  m_c4ProjBaseType
        //      The user tells us the base type of the projector, which let's
        //      us deal with small differences in the protocol ovr the range
        //      of projectors. Its a number like 700, 1200, etc...
        //
        //  m_c4Timeout
        //      Because the Barco can go off into the ozone for long periods
        //      of time, we allow a number of timeouts before we give up on
        //      it and consider it having gone bye bye. This counter is used
        //      to keep up with the count of successive poll failures.
        //
        //  m_cpcfgSerial
        //      The comm port configuration that we were given upon creation.
        //      We use this to configure our serial connection.
        //
        //  m_pcommBarco
        //      The comm port via which we talk to the projector.
        //
        //  m_strPort
        //      The serial port we were told to configuration ourself on.
        //
        //  m_msgPollReply
        //      A reply object for the poll thread to use to read replies
        //      back from the projector.
        //
        //  m_msgQuerySrc
        //  m_msgQueryStatus
        //      We send these messages to the projector constantly, so in order
        //      to avoid rebuilding them over and over again, we pre-build them
        //      and just reuse them.
        // -------------------------------------------------------------------
        tCIDLib::TCard1     m_c1ProjAddr;
        tCIDLib::TCard4     m_c4FldIdHorzPeriod;
        tCIDLib::TCard4     m_c4FldIdInput;
        tCIDLib::TCard4     m_c4FldIdPower;
        tCIDLib::TCard4     m_c4FldIdSigType;
        tCIDLib::TCard4     m_c4FldIdVertRate;
        tCIDLib::TCard4     m_c4ProjBaseType;
        tCIDLib::TCard4     m_c4Timeout;
        TCommPortCfg        m_cpcfgSerial;
        TCommPortBase*      m_pcommBarco;
        TString             m_strPort;
        TBarcoMsg           m_msgPollReply;
        TBarcoMsg           m_msgQuerySrc;
        TBarcoMsg           m_msgQueryStatus;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TBarcoCRTSDriver,TCQCServerBase)
};

#pragma CIDLIB_POPPACK


