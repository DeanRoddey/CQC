//
// FILE NAME: GenTrigS_DriverImpl.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/16/2007
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
//  This is the header file for the actual driver class. We derive from a
//  base IR server side driver class, and implement that interface. We override
//  a couple things from it's base class, the general server side driver base
//  class, mostly wrt to getting/releasing the comm resource, which only we
//  know about. We also override the poll callback to watch for incoming
//  training/triggering data.
//
//  We only support receiving, not blasting. So we can either be in receiver
//  training mode (in which case we just store incoming info as training data
//  for the parent class to get ahold of), or we are in regular receiving mode,
//  in which case we queue the incoming msgs up on the parent class to be
//  processed. It does that on a background thread that pulls out the value
//  from the queue and invokes the associated action if any.
//
//  In our case, nothing ever happens async. We see new info coming in during
//  our poll callback, and our parent class calls us to get training info
//  as it is called via regular driver callbacks it overrides, and these are
//  all serialized. So we don't really have any locking concerns here. The
//  queue we call to queue up incoming commands on (in the parent class) is
//  thread safe.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)



// ---------------------------------------------------------------------------
//   CLASS: TGenTrigSDriver
//  PREFIX: orbs
// ---------------------------------------------------------------------------
class TGenTrigSDriver : public TBaseIRSrvDriver
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TGenTrigSDriver
        (
            const   TCQCDriverObjCfg&       cqcdcToLoad
        );

        ~TGenTrigSDriver();


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bRecTrainingMode();

        tCIDLib::TBoolean bCheckRecTrainingData
        (
                    TString&                strKeyToFill
        );

        tCIDLib::TBoolean bGetCommResource
        (
                    TThread&                thrThis
        );

        tCIDLib::TBoolean bResetConnection();

        tCIDLib::TCard4 c4InvokeFldId() const;

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
        TGenTrigSDriver(const TGenTrigSDriver&);
        tCIDLib::TVoid operator=(const TGenTrigSDriver&);


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bGotRecTrainingData
        //      This is set when we have reciever training data that someone
        //      can pick up.
        //
        //  m_bRecTrainingMode
        //      Indicates whether we are currently in receiver training mode
        //      or not.
        //
        //  m_bSocket
        //      Indicates whether we are doing a socket or serial type
        //      connection.
        //
        //  m_c4FldIdXXX
        //      The field ids of our fields, which we look up after we
        //      register them, so that we can do 'by id' field access.
        //
        //  m_cpcfgSerial
        //  m_cpcfgSock
        //      The comm port or socket configuration that we were given
        //      upon creation. We use this to configure our connection.
        //
        //  m_eTermType
        //      The user tells us what type of line termination to expect,
        //      via a driver prompt during install.
        //
        //  m_pcommTrig
        //  m_psockTrig
        //      The comm port or socket that we use to accept incoming
        //      messages.
        //
        //  m_strPollStr
        //      A string to use in the Poll() method, to avoid creating and
        //      destroy one every time through.
        //
        //  m_strPort
        //      The serial port we were told to configuration ourself on, if
        //      in serial port mode. In socket mode this is empty.
        //
        //  m_strRecTrainVal
        //      When we are in training mode, we store the training data
        //      here until it's picked up.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bGotRecTrainingData;
        tCIDLib::TBoolean       m_bRecTrainingMode;
        tCIDLib::TBoolean       m_bSocket;
        tCIDLib::TCard4         m_c4FldIdInvoke;
        tCIDLib::TCard4         m_c4FldIdTrainingMode;
        TCommPortCfg            m_cpcfgSerial;
        TCQCIPConnCfg           m_conncfgSock;
        tGenTrigS::ETermTypes   m_eTermType;
        TCommPortBase*          m_pcommTrig;
        TStreamSocket*          m_psockTrig;
        TString                 m_strPollStr;
        TString                 m_strPort;
        TString                 m_strRecTrainData;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TGenTrigSDriver,TBaseIRSrvDriver)
};

#pragma CIDLIB_POPPACK



