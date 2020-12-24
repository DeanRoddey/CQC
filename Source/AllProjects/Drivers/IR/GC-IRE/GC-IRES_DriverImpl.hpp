//
// FILE NAME: GC-IRES_DriverImpl.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/29/2008
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
//  This is the header file for the actual driver class.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: TGCIRESDriver
//  PREFIX: sdrv
// ---------------------------------------------------------------------------
class TGCIRESDriver : public TBaseIRSrvDriver
{
    public :
        // -------------------------------------------------------------------
        //  A simple class that we use to hold on/off transition values from
        //  the incoming IR data.
        // -------------------------------------------------------------------
        class TOOPair
        {
            public :
                TOOPair();

                TOOPair
                (
                    const   tCIDLib::TCard4 c4On
                    , const tCIDLib::TCard4 c4Off
                );

                ~TOOPair();

                tCIDLib::TVoid Set
                (
                    const   tCIDLib::TCard4 c4On
                    , const tCIDLib::TCard4 c4Off
                );

                tCIDLib::TCard4 m_c4Off;
                tCIDLib::TCard4 m_c4On;
        };


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TGCIRESDriver
        (
            const   TCQCDriverObjCfg&       cqcdcToLoad
        );

        ~TGCIRESDriver();



    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCheckRecTrainingData
        (
                    TString&                strKeyToFill
        );

        tCIDLib::TBoolean bGetCommResource
        (
                    TThread&                thrThis
        );

        tCIDLib::TBoolean bResetConnection();

        tCIDLib::TVoid ClearRecTrainingData();

        tCQCKit::ECommResults eConnectToDevice
        (
                    TThread&                thrThis
        );

        tCQCKit::EDrvInitRes eInitializeImpl();

        tCQCKit::ECommResults ePollDevice
        (
                    TThread&                thrThis
        );

        tCIDLib::TVoid EnterRecTrainingMode();

        tCIDLib::TVoid ExitRecTrainingMode();

        tCIDLib::TVoid ReleaseCommResource();

        tCIDLib::TVoid TerminateImpl();


    private :
        // -------------------------------------------------------------------
        //  Unimplemented constructors and operators
        // -------------------------------------------------------------------
        TGCIRESDriver(const TGCIRESDriver&);
        tCIDLib::TVoid operator=(const TGCIRESDriver&);


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bEncodeSignal
        (
            const   TString&                strSignal
            ,       TString&                strKeyToFill
        );

        tCIDLib::TBoolean bGetMsg
        (
                    TString&                strToFill
            , const tCIDLib::TCard4         c4ToWait
            , const tCIDLib::TBoolean       bThrowIfNot
        );

		tCIDLib::TCard4 c4Compare
        (
			const   tCIDLib::TCard4			c4First
			, const	tCIDLib::TCard4			c4Second
        )   const;

        tCQCKit::ECommResults ePing
        (
            const   tCIDLib::TBoolean       bStore
        );

        tCIDLib::TVoid ProcessSignal
        (
            const   TString&                strFirstMsg
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bTrainingMode
        //      When a client puts the driver into training mode, we will
        //      turn off the polling thread, so that the device is completely
        //      at the disposal of the client doing the training. This flag
        //      will be set to warn off other clients, and the training mode
        //      field will be set to reflect the trainging state.
        //
        //  m_c4FldIdXXX
        //      The field ids of our fields, which we look up after we
        //      register them, so that we can do 'by id' field access.
        //
        //  m_enctLastMsg
        //      If we don't hear anything from the device for a while, i.e.
        //      no user activity, then we will actively poll it. We set this
        //      every time we get a message.
        //
        //  m_enctNextEventTime
        //      In order to supress multiple events from the GC-IRE, we don't
        //      accept another event for a given period of time after we
        //      accept one. Many remotes will cause 2 or 3 events, and we
        //      only want to accept one.
        //
        //  m_colPairs
        //      This is for use during decoding, to hold the pairs as we
        //      parse them out.
        //
        //  m_cpcfgSerial
        //      The comm port configuration that we were given upon creation.
        //      We use this to configure our serial connection.
        //
        //  m_pcommGC-IRE
        //      The comm port via which we talk to the GC-IRE.
        //
        //  m_strCmdPrefix
        //      This is a prefix that's on all the incoming IR strings, we
        //      we have to check for it every time. So we just set up one
        //      for efficient use.
        //
        //  m_strPollMsg
        //      Used by the polling callback to read message into.
        //
        //  m_strPort
        //      The serial port we were told to configuration ourself on.
        //
        //  m_strTmpX
        //      Just some string for temp use within callbacks
        //
        //  m_strTrainVal
        //      When we are in training mode, the poll thread will drop any
        //      incoming events into here.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bTrainingMode;
        tCIDLib::TCard4         m_c4FldIdFirmwareVer;
        tCIDLib::TCard4         m_c4FldIdTrainingMode;
        tCIDLib::TEncodedTime   m_enctLastMsg;
        tCIDLib::TEncodedTime   m_enctNextEvent;
        TVector<TOOPair>        m_colPairs;
        TCommPortCfg            m_cpcfgSerial;
        TCommPortBase*          m_pcommGCIRE;
        const TString           m_strCmdPrefix;
        TString                 m_strPollMsg;
        TString                 m_strPort;
        TString                 m_strTmp;
        TString                 m_strTmp2;
        TString                 m_strTrainVal;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TGCIRESDriver,TBaseIRSrvDriver)
};

#pragma CIDLIB_POPPACK


