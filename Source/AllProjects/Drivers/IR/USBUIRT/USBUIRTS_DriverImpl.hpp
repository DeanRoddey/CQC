//
// FILE NAME: USBUIRTS_DriverImpl.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/12/2003
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
//  This is the header file for the actual driver class. We inherit from the
//  IDL generated base server class, and provide implementations for the
//  stubbed out virtual methods it created for us.
//
//  Note that zone support was added to this driver after the fact. Because
//  of the fact that the front blaster is Zone 3 for some reason, instead
//  of one, we have to swizzle the zone numbers so that the default is three
//  instead of one. So we assign them as (with our zero based zones)
//
//  Zone 0 : Z3
//  Zone 1 : Z1
//  Zone 2 : Z2
//
// CAVEATS/GOTCHAS:
//
//  1)  This driver is unusual in that it can receive callbacks from the
//      UIRT. Normally drivers only run on their own thread, and the only
//      way data would be updated is because the driver thread itself was
//      updating it.
//
//      To avoid issues, we just post ourself an async command, and don't
//      wait for it, then we'll receive the command back again on our own
//      driver thread. This avoids all locking problems.
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: TUSBUIRTSDriver
//  PREFIX: orbs
// ---------------------------------------------------------------------------
class TUSBUIRTSDriver : public TBaseIRSrvDriver
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TUSBUIRTSDriver
        (
            const   TCQCDriverObjCfg&       cqcdcToLoad
        );

        ~TUSBUIRTSDriver();


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TCard4 c4SendCmd
        (
            const   TString&                strCmdId
            , const TString&                strParms
        );


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TCard4 c4NextEventTime() const;

        tCIDLib::TCard4 c4NextEventTime
        (
            const   tCIDLib::TCard4         c4ToSet
        );

        tCIDLib::TVoid HandleRecEvent
        (
            const   TString&                strEventStr
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bBlastTrainingMode();

        tCIDLib::TBoolean bRecTrainingMode();

        tCIDLib::TBoolean bCheckBlastTrainingData
        (
                    tCIDLib::TCard4&        c4DataBytes
            ,       THeapBuf&               mbufToFill
        );

        tCIDLib::TBoolean bCheckRecTrainingData
        (
                    TString&                strKeyToFill
        );

        tCIDLib::TBoolean bCvtManualBlastData
        (
            const   TString&                strText
            ,       tCIDLib::TCard4&        c4DataBytes
            ,       THeapBuf&               mbufToFill
            ,       TString&                strError
        );

        tCIDLib::TBoolean bGetCommResource
        (
                    TThread&                thrThis
        );

        tCIDLib::TBoolean bResetConnection();

        tCIDLib::TCard4 c4InvokeFldId() const;

        tCIDLib::TCard4 c4ZoneCount() const;

        tCIDLib::TVoid ClearBlastTrainingData();

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

        tCIDLib::TVoid EnterBlastTrainingMode();

        tCIDLib::TVoid EnterRecTrainingMode();

        tCIDLib::TVoid ExitBlastTrainingMode();

        tCIDLib::TVoid ExitRecTrainingMode();

        tCIDLib::TVoid FormatBlastData
        (
            const   TIRBlasterCmd&          irbcFmt
            ,       TString&                strToFill
        );

        tCIDLib::TVoid InvokeBlastCmd
        (
            const   TString&                strDevice
            , const TString&                strCmd
            , const tCIDLib::TCard4         c4ZoneNum
        );

        tCIDLib::TVoid ReleaseCommResource();

        tCIDLib::TVoid SendBlasterData
        (
            const   tCIDLib::TCard4         c4DataBytes
            , const TMemBuf&                mbufToSend
            , const tCIDLib::TCard4         c4ZoneNum
            , const tCIDLib::TCard4         c4RepeatCount
        );

        tCIDLib::TVoid TerminateImpl();


    private :
        // -------------------------------------------------------------------
        //  Unimplemented constructors and operators
        // -------------------------------------------------------------------
        TUSBUIRTSDriver(const TUSBUIRTSDriver&);
        tCIDLib::TVoid operator=(const TUSBUIRTSDriver&);


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::EExitCodes eLearnThread
        (
                    TThread&                thrThis
            ,       tCIDLib::TVoid*         pData
        );

        const TString& strUUIRTErr
        (
            const   tCIDLib::TOSErrCode     errcToLoad
        );

        tCIDLib::TVoid SendIRData
        (
            const   TMemBuf&                mbufData
            , const tCIDLib::TCard4         c4Bytes
            , const tCIDLib::TCard4         c4RepeatCount
            , const tCIDLib::TCard4         c4ZoneNum
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bGotBlastTrainingData
        //  m_bGotRecTrainingData
        //      This when we have either blaster or reciever training data
        //      that someone can pick up.
        //
        //      These should be protected by a driver lock, see the file hdr
        //      comments above.
        //
        //  m_bBlastTrainingMode
        //  m_bRecTrainingMode
        //      Indicates whether we are currently in blaster or receiver
        //      training mode or not.
        //
        //      These should be protected by a driver lock, see the file hdr
        //      comments above.
        //
        //  m_c4CancelLearn
        //      The learn method is blocking, and we do it on a separate
        //      thread. To cancel it, we have to pass it a flag which we
        //      can set.
        //
        //  m_c4FldIdXXX
        //      The field ids of our fields, which we look up after we
        //      register them, so that we can do 'by id' field access.
        //
        //  m_c4NextRebindTime
        //      Used to periodically cycle the rebinder. We want to do it
        //      every five seconds, or thereabouts, since the rebinder
        //      is set up for that cycle time.
        //
        //  m_c4PollCount
        //      We want to periodically poll the device to make sure it's
        //      there. So we use this to count up a number of times to the
        //      poll call, and when we hit the desired count, we do the call.
        //
        //  m_colRecKeyQ
        //      We get called back on a UIRT thread, not on one of our
        //      threads. So the callback method cannot directly call into
        //      our parent class to queue up the incoming IR key. Instead
        //      it queues up the keys here and our poll method will see
        //      them and queue them up.
        //
        //  m_hUIRT
        //      Our handle to the UIRT. It will be c4MaxCard if not open.
        //
        //  m_pchBlastTrainData
        //      Used during training, this is where we store the data that we
        //      receive. This is the format it gives it back to us, which is
        //      the reason for the use of the raw buffer. It is a null
        //      terminated ASCII string, so we figure out the length after
        //      the fact.
        //
        //  m_pfnXXXX
        //      We load the USB-UIRT DLL dynamically, and look up the needed
        //      APIs, storing pointers to the for later call. These are used
        //      to hold the function pointers.
        //
        //  n_pmodSupport
        //      We need to load the USB-UIRT DLL so that we can call it's
        //      APIs, and we need to keep it loaded so we need to keep
        //      around a module object. It has to be a pointer since we
        //      can't know the DLL is there, so we cannot just blindly
        //      try to load it during the driver ctor.
        //
        //  m_strRecTrainData
        //      When we are in training mode, the callback will drop any
        //      incoming events into here and set the 'got rec data' flag.
        //
        //      This should be protected by a driver lock, see the file hdr
        //      comments above.
        //
        //  m_thrLearn
        //      The USB-UIRT's learning call is blocking, so we have to do
        //      it via a separate thread. We only allow one outstanding
        //      learning operation at once, so we only need this one thread
        //      to support this functionality.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bGotBlastTrainingData;
        tCIDLib::TBoolean       m_bGotRecTrainingData;
        tCIDLib::TBoolean       m_bBlastTrainingMode;
        tCIDLib::TBoolean       m_bRecTrainingMode;
        tCIDLib::TCard4         m_c4CancelLearn;
        tCIDLib::TCard4         m_c4NextEventTime;
        tCIDLib::TCard4         m_c4FldIdFirmwareVer;
        tCIDLib::TCard4         m_c4FldIdInvoke;
        tCIDLib::TCard4         m_c4FldIdTrainingMode;
        tCIDLib::TCard4         m_c4NextRebindTime;
        tCIDLib::TCard4         m_c4PollCount;
        tCIDLib::TStringQ       m_colRecKeyQ;
        TUUHandle               m_hUIRT;
        tCIDLib::TSCh           m_pchBlastTrainData[8192];
        TUUIRTClose             m_pfnCloseProc;
        TUUIRTGetDrvInfo        m_pfnGetInfoProc;
        TUUIRTGetUUIRTInfo      m_pfnGetUIRTInfoProc;
        TUUIRTLearnIR           m_pfnLearnProc;
        TUUIRTOpen              m_pfnOpenProc;
        TUUIRTSetUUIRTConfig    m_pfnSetCfgProc;
        TUUIRTSetRecCB          m_pfnSetRecCBProc;
        TUUIRTTransmitIR        m_pfnTransmitProc;
        TModule*                m_pmodSupport;
        TString                 m_strErr_NoDevice;
        TString                 m_strErr_NoResp;
        TString                 m_strErr_NoDLL;
        TString                 m_strErr_Version;
        TString                 m_strErr_Unknown;
        TString                 m_strRecTrainData;
        TThread                 m_thrLearn;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TUSBUIRTSDriver,TBaseIRSrvDriver)
};

#pragma CIDLIB_POPPACK



