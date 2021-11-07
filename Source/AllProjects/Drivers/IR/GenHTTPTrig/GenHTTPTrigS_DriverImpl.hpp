//
// FILE NAME: GenHTTPTrigS_DriverImpl.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/24/2013
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
//  This driver, unlike almost any others, is a listening socket server. we
//  listen for connections instead of making a connection. This is generally
//  considered bad form and a security risk, but some folks want to have such
//  a thing so that they can trigger CQC to do things from various programs or
//  devices that are implemented as clients, not servers.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)



// ---------------------------------------------------------------------------
//   CLASS: TGenHTTPTrigSDriver
//  PREFIX: orbs
// ---------------------------------------------------------------------------
class TGenHTTPTrigSDriver : public TBaseIRSrvDriver
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TGenHTTPTrigSDriver() = delete;

        TGenHTTPTrigSDriver
        (
            const   TCQCDriverObjCfg&       cqcdcToLoad
        );

        TGenHTTPTrigSDriver(const TGenHTTPTrigSDriver&) = delete;
        TGenHTTPTrigSDriver(TGenHTTPTrigSDriver&&) = delete;

        ~TGenHTTPTrigSDriver();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TGenHTTPTrigSDriver& operator=(const TGenHTTPTrigSDriver&) = delete;
        TGenHTTPTrigSDriver& operator=(TGenHTTPTrigSDriver&&) = delete;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bRecTrainingMode() const final;

        tCIDLib::TBoolean bCheckRecTrainingData
        (
                    TString&                strKeyToFill
        )   final;

        tCIDLib::TBoolean bGetCommResource
        (
                    TThread&                thrThis
        )   final;

        tCIDLib::TBoolean bResetConnection() final;

        tCIDLib::TCard4 c4InvokeFldId() const final;

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
        tCIDLib::TBoolean bRewriteURL
        (
            const   TURL&                   urlReq
            ,       TString&                strToFill
        );


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
        //  m_c4FldIdXXX
        //      The field ids of our fields, which we look up after we
        //      register them, so that we can do 'by id' field access.
        //
        //  m_httpcSrv
        //      The HTTP object we use to handle incoming requests and format
        //      out replies.
        //
        //  m_ipepClient
        //      When we get a connection we get back the end point.
        //
        //  m_ippnListen
        //      The port we were told to listen on. Since we aren't a normal
        //      IP based driver, we don't use the IP comm config stuff in the
        //      manifest. We just use a user prompt to let the user tell us
        //      the port.
        //
        //  m_mbufData
        //      A buffer for reading in the HTTP data.
        //
        //  m_psocklTrig
        //      The socket listener we create to listen for connections. We
        //      set it up for just one connection at a time. We treat our
        //      get comm resource as creating this guy, so we won't come
        //      online until we get it created. We clean it up during driver
        //      termination.
        //
        //  m_strCmdStr
        //      A string to format the incoming URL into in order to queue
        //      it up.
        //
        //  m_strRecTrainVal
        //      When we are in training mode, we store the training data
        //      here until it's picked up.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bGotRecTrainingData;
        tCIDLib::TBoolean       m_bRecTrainingMode;
        tCIDLib::TCard4         m_c4FldIdInvoke;
        tCIDLib::TCard4         m_c4FldIdTrainingMode;
        tCIDLib::TKVPList       m_colInHdrLines;
        THTTPClient             m_httpcSrv;
        TIPEndPoint             m_ipepClient;
        tCIDLib::TIPPortNum     m_ippnListen;
        THeapBuf                m_mbufData;
        TSocketListener*        m_psocklTrig;
        TString                 m_strCmdStr;
        TString                 m_strRecTrainData;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TGenHTTPTrigSDriver,TBaseIRSrvDriver)
};

#pragma CIDLIB_POPPACK



