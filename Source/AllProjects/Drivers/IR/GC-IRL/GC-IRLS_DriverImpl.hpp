//
// FILE NAME: GC-IRLS_DriverImpl.hpp
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
//   CLASS: TGCIRLSDriver
//  PREFIX: sdrv
// ---------------------------------------------------------------------------
class TGCIRLSDriver : public TBaseIRSrvDriver
{
    public :
        // -------------------------------------------------------------------
        //  A simple structure to store on/off pair values as we expand the
        //  compressed data.
        // -------------------------------------------------------------------
        struct TOOPair
        {
            tCIDLib::TCard4 c4On;
            tCIDLib::TCard4 c4Off;
        };


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TGCIRLSDriver
        (
            const   TCQCDriverObjCfg&       cqcdcToLoad
        );

        ~TGCIRLSDriver();



    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bBlastTrainingMode() const;

        tCIDLib::TBoolean bCheckBlastTrainingData
        (
                    tCIDLib::TCard4&        c4DataBytes
            ,       THeapBuf&               mbufToFill
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

        tCIDLib::TVoid ClearBlastTrainingData();

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

        tCIDLib::TVoid ExitBlastTrainingMode();

        tCIDLib::TVoid FormatBlastData
        (
            const   TIRBlasterCmd&          irbcFmt
            ,       TString&                strToFill
        );

        tCIDLib::TVoid ReleaseCommResource();

        tCIDLib::TVoid TerminateImpl();


    private :
        // -------------------------------------------------------------------
        //  Unimplemented constructors and operators
        // -------------------------------------------------------------------
        TGCIRLSDriver(const TGCIRLSDriver&);
        tCIDLib::TVoid operator=(const TGCIRLSDriver&);


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bDecodeSignal
        (
            const   TString&                strSignal
            ,       TString&                strToFill
        );

        tCIDLib::TBoolean bGetMsg
        (
                    TString&                strToFill
            , const tCIDLib::TEncodedTime   enctEnd
        );

        tCIDLib::TBoolean bWaitReply
        (
            const   TString&                strExpPref
            ,       TString&                strToFill
            , const tCIDLib::TCard4         c4WaitMS
        );

        tCQCKit::ECommResults ePing
        (
            const   tCIDLib::TBoolean       bStore
        );

        tCIDLib::TVoid ProcessSignal
        (
            const   TString&                strData
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bGotTrainingData
        //      This when we have blaster training data that the client can
        //      pick up.
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
        //  m_cpcfgSerial
        //      The comm port configuration that we were given upon creation.
        //      We use this to configure our serial connection.
        //
        //  m_pcommGC-IRE
        //      The comm port via which we talk to the GC-IRE.
        //
        //  m_strPrefixXXX
        //      Some prefixes we use to recognize specific responses or
        //      asyncs.
        //
        //  m_strPollMsg
        //      Used by the polling callback to read message into.
        //
        //  m_strPort
        //      The serial port we were told to configuration ourself on.
        //
        //  m_strTrainVal
        //      When we are in training mode, the poll thread will drop any
        //      incoming events into here.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bGotTrainingData;
        tCIDLib::TBoolean       m_bTrainingMode;
        tCIDLib::TCard4         m_c4FldIdFirmwareVer;
        tCIDLib::TCard4         m_c4FldIdTrainingMode;
        tCIDLib::TEncodedTime   m_enctLastMsg;
        TCommPortCfg            m_cpcfgSerial;
        TCommPortBase*          m_pcommGCIRL;
        const TString           m_strPrefixIRData;
        const TString           m_strPrefixIREnd;
        const TString           m_strPrefixVer;
        TString                 m_strPollMsg;
        TString                 m_strPort;
        TString                 m_strTrainVal;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TGCIRLSDriver,TBaseIRSrvDriver)
};

#pragma CIDLIB_POPPACK


