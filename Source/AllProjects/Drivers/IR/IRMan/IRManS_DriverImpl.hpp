//
// FILE NAME: IRManS_DriverImpl.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/28/2002
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
//   CLASS: TIRManSDriver
//  PREFIX: sdrv
// ---------------------------------------------------------------------------
class TIRManSDriver : public TBaseIRSrvDriver
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TIRManSDriver() = delete;

        TIRManSDriver
        (
            const   TCQCDriverObjCfg&       cqcdcToLoad
        );

        TIRManSDriver(const TIRManSDriver&) = delete;
        TIRManSDriver(TIRManSDriver&&) = delete;

        ~TIRManSDriver();


        // -------------------------------------------------------------------
        //  Protected operator
        // -------------------------------------------------------------------
        TIRManSDriver& operator=(const TIRManSDriver&) = delete;
        TIRManSDriver& operator=(TIRManSDriver&&) = delete;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCheckRecTrainingData
        (
                    TString&                strKeyToFill
        )   final;

        tCIDLib::TBoolean bGetCommResource
        (
                    TThread&                thrThis
        )   final;

        tCIDLib::TBoolean bResetConnection() final;

        tCIDLib::TVoid ClearRecTrainingData() final;

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

        tCIDLib::TVoid ReleaseCommResource() final;

        tCIDLib::TVoid TerminateImpl() final;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCycleConn();

        tCIDLib::TBoolean bGetMsg
        (
                    TString&                strToFill
            , const tCIDLib::TCard4         c4ToWait
        );

        tCIDLib::TVoid FormatKey
        (
            const   tCIDLib::TCard1* const  pac1Buf
            ,       TString&                strToFill
        )   const;


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
        //  m_c4NextEventTime
        //      In order to supress multiple events from the IRMan, we don't
        //      accept another event for a given period of time after we
        //      accept one. Many remotes will cause 2 or 3 events, and we
        //      only want to accept one.
        //
        //  m_c4ResetCount
        //      The IRMan protocol is woeful, and we have no way to know if
        //      it has been unplugged or died or whatever. The only thing we
        //      can do is recycle the connection. So, if we've not seen any
        //      activity in a minute, we'll recycle it.
        //
        //  m_cpcfgSerial
        //      The comm port configuration that we were given upon creation.
        //      We use this to configure our serial connection.
        //
        //  m_pcommIRMan
        //      The comm port via which we talk to the IRMan.
        //
        //  m_strPort
        //      The serial port we were told to configuration ourself on.
        //
        //  m_strTrainVal
        //      When we are in training mode, the poll thread will drop any
        //      incoming events into here.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bTrainingMode;
        tCIDLib::TCard4     m_c4FldIdFirmwareVer;
        tCIDLib::TCard4     m_c4FldIdTrainingMode;
        tCIDLib::TCard4     m_c4NextEventTime;
        tCIDLib::TCard4     m_c4ResetCount;
        TCommPortCfg        m_cpcfgSerial;
        TCommPortBase*      m_pcommIRMan;
        TString             m_strPort;
        TString             m_strTrainVal;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TIRManSDriver,TBaseIRSrvDriver)
};

#pragma CIDLIB_POPPACK


