//
// FILE NAME: R2DIS_DriverImpl.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/08/2005
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
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: TR2DIDriver
//  PREFIX: orbs
// ---------------------------------------------------------------------------
class TR2DIDriver : public TBaseIRSrvDriver
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TR2DIDriver() = delete;

        TR2DIDriver
        (
            const   TCQCDriverObjCfg&       cqcdcToLoad
        );

        TR2DIDriver(const TR2DIDriver&) = delete;
        TR2DIDriver(TR2DIDriver&&) = delete;

        ~TR2DIDriver();


        // -------------------------------------------------------------------
        //  Protected operator
        // -------------------------------------------------------------------
        TR2DIDriver& operator=(const TR2DIDriver&) = delete;
        TR2DIDriver& operator=(TR2DIDriver&&) = delete;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCheckRecTrainingData
        (
                    TString&                strKeyToFill
        )   final;

        tCIDLib::TBoolean bCvtManualBlastData
        (
            const   TString&                strText
            ,       tCIDLib::TCard4&        c4DataBytes
            ,       THeapBuf&               mbufToFill
            ,       TString&                strError
        )   final;

        tCIDLib::TBoolean bGetCommResource
        (
                    TThread&                thrThis
        )   final;

        tCIDLib::TBoolean bRecTrainingMode() const final;

        tCIDLib::TBoolean bResetConnection() final;

        tCIDLib::TCard4 c4ZoneCount() const final;

        tCIDLib::TCard4 c4InvokeFldId() const;

        tCIDLib::TVoid ClearRecTrainingData() final;

        tCQCKit::ECommResults eBoolFldValChanged
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4FldId
            , const tCIDLib::TBoolean       bNewValue
        )   final;

        tCQCKit::ECommResults eConnectToDevice
        (
                    TThread&                thrThis
        )   final;

        tCQCKit::EDrvInitRes eInitializeImpl() final;

        tCQCKit::ECommResults ePollDevice
        (
                    TThread&                thrThis
        )   final;

        tCIDLib::TVoid EnterRecTrainingMode() final;

        tCIDLib::TVoid ExitRecTrainingMode() final;

        tCIDLib::TVoid InvokeBlastCmd
        (
            const   TString&                strDevice
            , const TString&                strCmd
            , const tCIDLib::TCard4         c4ZoneNum
        )   final;

        tCIDLib::TVoid SendBlasterData
        (
            const   tCIDLib::TCard4         c4DataBytes
            , const TMemBuf&                mbufToSend
            , const tCIDLib::TCard4         c4ZoneNum
            , const tCIDLib::TCard4         c4RepeatCount
        )   final;

        tCIDLib::TVoid ReleaseCommResource() final;

        tCIDLib::TVoid TerminateImpl() final;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bGetMessage
        (
                    TString&                strToFill
            , const tCIDLib::TCard4         c4WaitFor
            , const tCIDLib::TBoolean       bThrowIfNot
        );

        tCIDLib::TVoid PollSensors
        (
                    TString&                strTmp
        );

        tCIDLib::TVoid ProcessMsg
        (
                    TString&                strMsg
        );

        tCIDLib::TVoid SendAndWaitForAck
        (
            const   TString&                strToSend
            , const tCIDLib::TCard4         c4WaitFor
        );

        tCIDLib::TVoid SendIRCmd
        (
            const   tCIDLib::TCard1* const  pc1Data
            , const tCIDLib::TCard4         c4Bytes
            , const tCIDLib::TCard4         c4ZoneNum
            , const tCIDLib::TCard4         c4RepeatCount
        );

        tCIDLib::TVoid SendMsg
        (
            const   TString&                strToSend
        );

        tCIDLib::TVoid WaitForAckOrError
        (
            const   TString&                strResponseTo
            ,       TString&                strTmp
            , const tCIDLib::TCard4         c4WaitFor
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_abZoneTypes
        //      An array of 4 zones that indicates whether each of the available
        //      zones is a blaster or sensor. If false it's a blaster, if true
        //      it's a sensor. This is set from driver config prompts we get
        //      in our driver config.
        //
        //  m_ac4FldIdSensors
        //      Some zones are blasters and some are sensors. But we assign
        //      1 based, contiguous sensor numbers in the field names (sensors
        //      have their own fields.) In order to be able to map between
        //      field ids and zone numbers for sensors, we use this array. we
        //      set blaster zones to c4MaxCard and set the sensor zones with
        //      their field ids.
        //
        //  m_bRecTrainingMode
        //      Indicates whether we are currently in receive training mode
        //      or not.
        //
        //  m_bGotRecTrainingData
        //      When the training data has been seen by the polling loop, it
        //      will store it in m_strRecTrainData and set this flag to
        //      let the client IR driver know that the data is ready to be
        //      picked up.
        //
        //  m_c4FldIdXXX
        //      The ids for our fields.
        //
        //  m_cpcfgPort
        //      The port configuration that we were given upon creation.
        //
        //  m_enctNextActivePoll
        //      Each time we get some response back from the device, we set
        //      this to 5 seconds out from then. The poll loop will watch
        //      this and if nothing has come in since then, it will do an
        //      active poll to insure the device is still there.
        //
        //  m_pcommR2DI
        //      The com port we use to talk to the device.
        //
        //  m_strMsgXXX
        //      Some pre-fab reply strings that we will look for incoming or
        //      use in outgoing, just set up as members for efficiency.
        //
        //  m_strPort
        //      The com port we were set up to use by our driver config.
        //
        //  m_strRecTrainData
        //      When we are in receiver training mode, the poll loop will be
        //      looking for data to arrive. It's stored here until the client
        //      comes to get it and takes us out of training mode.
        //
        //  m_tcvtIO
        //      A text converter for transcoding between our Unicode and the
        //      device's ASCII formats.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_abZoneTypes[kR2DIS::c4ZoneCount];
        tCIDLib::TCard4         m_ac4FldIdSensors[kR2DIS::c4ZoneCount];
        tCIDLib::TBoolean       m_bRecTrainingMode;
        tCIDLib::TBoolean       m_bGotRecTrainingData;
        tCIDLib::TCard4         m_c4BlasterZoneCnt;
        tCIDLib::TCard4         m_c4FldIdFirmwareVer;
        tCIDLib::TCard4         m_c4FldIdInvoke;
        tCIDLib::TCard4         m_c4FldIdTrainingMode;
        tCIDLib::TCard4         m_c4SensorZoneCnt;
        tCIDLib::TEncodedTime   m_enctNextActivePoll;
        TCommPortCfg            m_cpcfgSerial;
        TCommPortBase*          m_pcommR2DI;
        TString                 m_strMsgAckReply;
        TString                 m_strMsgBlastPref;
        TString                 m_strMsgErrPref;
        TString                 m_strMsgPref;
        TString                 m_strMsgSensorStatePref;
        TString                 m_strMsgTrainDataPref;
        TString                 m_strPort;
        TString                 m_strRecTrainData;
        TUSASCIIConverter       m_tcvtIO;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TR2DIDriver,TBaseIRSrvDriver)
};

#pragma CIDLIB_POPPACK


